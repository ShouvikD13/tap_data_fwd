/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   UserPipe.cpp
 * Author: 33866
 *
 * Created on 1 February, 2019, 5:38 PM
 */

#include<iostream>
#include "UserPipe.h"
#include "genLogger.h"

/*
UserPipe::UserPipe() {
}

UserPipe::UserPipe(const UserPipe& orig) {
}
 */
UserPipe::~UserPipe()
{
    LOG_DEBUG << LOGPATH << "Trader id :" << l_trader_id << " destructor called";
    write_msgs_.clear();
    std::cout << "In userpipe destructor position a" << std::endl;
    if (socket_.is_open())
    {
        boost::system::error_code ec;
        socket_.close(ec);
    }
}

void UserPipe::do_write()
{
    auto self(shared_from_this());
    std::lock_guard<std::mutex> lock(to_usr_pipe_msg_que_mtx);
    if (write_msgs_.empty() == true)
    {
        return;
    }
    exg_data &snd_data = write_msgs_.front();
    int i_pkt_len = snd_data.get_pkt_len();
    LOG_DEBUG << LOGPATH << "Pipe 4 rspns :Deliver message to Pipe:Msg CODE:" << snd_data.get_trans_cd() << ":trdId:" << snd_data.get_trd_id() << ":Length send:" << i_pkt_len;
    boost::asio::async_write(socket_,
                             boost::asio::buffer(snd_data.getdata(), i_pkt_len),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/)
                             {
                                 if (!ec)
                                 {
                                     // write_msgs_.pop_front();
                                     LOG_TRACE << LOGPATH << "in async_write.";
                                     bool b_no_msgs_to_snd = false;
                                     {
                                         std::lock_guard<std::mutex> lock(to_usr_pipe_msg_que_mtx);
                                         write_msgs_.pop_front();
                                         b_no_msgs_to_snd = write_msgs_.empty();
                                     }
                                     if (!b_no_msgs_to_snd)
                                     {
                                         LOG_DEBUG << LOGPATH << "in:do_write:before:do_write: sizeof queue" << write_msgs_.size();
                                         do_write();
                                     }
                                 }
                                 else
                                 {
                                     LOG_ERROR << LOGPATH << "Error while writing to pipe.";
                                     usr_pipe_hldr->leave_pipehandler(shared_from_this());
                                 }
                             });
    // write_msgs_.pop_front();
}

void UserPipe::start()
{
    usr_pipe_hldr->add_pipe(shared_from_this());
    do_read_login_header();
    LOG_INFO << LOGPATH << "Pipe connection request received.";
}

void UserPipe::deliver_to_usr(const exg_data &msg)
{
    LOG_TRACE << LOGPATH << "in:deliver_to_usr:before:lock: sizeof queue" << write_msgs_.size();
    bool write_not_in_progress = false;
    { /* block start for limiting scope of lock */
        std::lock_guard<std::mutex> lock(to_usr_pipe_msg_que_mtx);
        write_not_in_progress = write_msgs_.empty();
        write_msgs_.emplace_back(msg);
        LOG_TRACE << LOGPATH << "in:deliver_to_usr:after:lock: sizeof queue" << write_msgs_.size();
    } /* block end for limiting scope of lock */
    if (write_not_in_progress)
    {
        do_write();
    }
}

void UserPipe::do_read_login_header()
{
    boost::asio::async_read(socket_,
                            boost::asio::buffer(recv_data_to_exg.data(), recv_data_to_exg.MSG_HDR_LEN),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec /* && recv_data_to_exg.decode_header()*/ &&
                                    recv_data_to_exg.get_msg_body_len() > 0)
                                {
                                    do_read_login_msg_body();
                                }
                                else
                                {
                                    LOG_ERROR << LOGPATH << "Error while reading header.: packet body length:" << recv_data_to_exg.get_msg_body_len();
                                    usr_pipe_hldr->leave_pipehandler(shared_from_this());
                                    // socket_.close();
                                }
                            });
}
void UserPipe::do_read_login_msg_body()
{
    // LOG_DEBUG << LOGPATH << "Message body length :" << recv_data_to_exg.get_msg_body_len();
    boost::asio::async_read(socket_,
                            boost::asio::buffer(recv_data_to_exg.msg_body_data(), recv_data_to_exg.get_msg_body_len()),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec /* && read_msg_.decode_message()*/)
                                {
                                    // LOG_DEBUG << LOGPATH << "Pipe 1 :add_to_exg_que:Msg CODE:" << recv_data_to_exg.get_trans_cd()<< ":trdId:" << recv_data_to_exg.get_trd_id() ;
                                    l_trader_id = recv_data_to_exg.get_trd_id();
                                    usr_pipe_hldr->add_pipe_mapping(shared_from_this(), l_trader_id);
                                    LOG_INFO << LOGPATH << "Pipe added with trade id:" << l_trader_id << ":Received Login request TransCd:" << recv_data_to_exg.get_trans_cd();
                                    usr_pipe_hldr->add_to_exg_que(recv_data_to_exg);
                                    do_read_msg_hdr();
                                }
                                else
                                {
                                    LOG_ERROR << LOGPATH << "Error while reading Pipe login request. Trader id:" << l_trader_id;
                                    socket_.close();
                                }
                            });
}

void UserPipe::do_read_msg_hdr()
{
    recv_data_to_exg.reset_msg();
    boost::asio::async_read(socket_,
                            boost::asio::buffer(recv_data_to_exg.data(), recv_data_to_exg.MSG_HDR_LEN),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec /* && recv_data_to_exg.decode_header()*/)
                                {
                                    do_read_msg_body();
                                }
                                else
                                {
                                    socket_.close();
                                }
                            });
}

void UserPipe::do_read_msg_body()
{
    // LOG_DEBUG << LOGPATH << "Message body length :" << recv_data_to_exg.get_msg_body_len();
    boost::asio::async_read(socket_,
                            boost::asio::buffer(recv_data_to_exg.msg_body_data(), recv_data_to_exg.get_msg_body_len()),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                 /* && read_msg_.decode_message()*/
                                if (!ec)
                                {
                                    // LOG_DEBUG << LOGPATH << "Pipe 1 :add_to_exg_que:Msg CODE:" << recv_data_to_exg.get_trans_cd()<< ":trdId:" << recv_data_to_exg.get_trd_id() ;
                                    usr_pipe_hldr->add_to_exg_que(recv_data_to_exg);
                                    do_read_msg_hdr();
                                }
                                else
                                {
                                    socket_.close();
                                }
                            }
    );
}