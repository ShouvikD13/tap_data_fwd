/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   pipe_handler.cpp
 * Author: 33866
 *
 * Created on 4 February, 2019, 6:58 PM
 */

#include "pipe_handler.h"
#include "genLogger.h"
#include "eba_tap_hdr.h"

/*pipe_handler::pipe_handler() {
}

pipe_handler::pipe_handler(const pipe_handler& orig) {
}
*/
pipe_handler::~pipe_handler()
{
    if (acceptor_.is_open())
    {
        acceptor_.cancel();
        acceptor_.close();
    }
    if (socket_.is_open())
    {
        boost::system::error_code ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }
    userid_pipeptr_map.clear();
    user_pipes_.clear();
}

void pipe_handler::add_pipe(dlvr_to_usr_ptr pipe_ptr)
{
    std::lock_guard<std::mutex> lock(pipe_mtx);
    if (user_pipes_.count(pipe_ptr) == 0)
    {
        user_pipes_.insert(pipe_ptr);
    }
}

void pipe_handler::add_pipe_mapping(dlvr_to_usr_ptr pipe_ptr, long trader_id)
{
    LOG_INFO << LOGPATH << "Add pipe for trader id:" << trader_id;
    std::lock_guard<std::mutex> lock(pipe_mtx);
    if (/*(userid_pipeptr_map.count(trader_id) == 0) && */ (user_pipes_.count(pipe_ptr) == 1))
    {
        userid_pipeptr_map[trader_id] = pipe_ptr;
        user_pipes_.erase(pipe_ptr);
    }
}

void pipe_handler::deliver_to_handlr(const exg_data &msg)
{
    exg_data exg_msg_tmp = msg;
    LOG_DEBUG << LOGPATH << ":trn_cd:" << exg_msg_tmp.get_trans_cd() << ":Total_len:" << exg_msg_tmp.get_pkt_len() << "Error code:" << exg_msg_tmp.get_error_cd();
    std::lock_guard<std::mutex> lock(to_usr_msg_que_mtx);
    bool b_not_processing_que = data_to_usr_msgs_que_.empty();
    data_to_usr_msgs_que_.emplace_back(msg);
    if (b_not_processing_que)
    {
        msg_to_usr_io_service.post([this]()
                                   { deliver_to_usr_pipe(); });
    }
}

void pipe_handler::deliver_to_usr_pipe()
{
    LOG_DEBUG << LOGPATH << ":inside deliver to usr pipe:";
    bool b_data_to_usr_que_empty = data_to_usr_msgs_que_.empty();
    while (!b_data_to_usr_que_empty)
    {
        exg_data msg_data;
        /* below block is separate block and not else of above if */
        { /* separate block to reduce lock impact */
            std::lock_guard<std::mutex> lock(to_usr_msg_que_mtx);
            msg_data = data_to_usr_msgs_que_.front();
            data_to_usr_msgs_que_.pop_front();
            b_data_to_usr_que_empty = data_to_usr_msgs_que_.empty();
        }

        long l_trd_id = msg_data.get_trd_id();
        LOG_DEBUG << LOGPATH << ":before sending to pipe:" << l_trd_id << ":_ptr trn_cd:" << msg_data.get_trans_cd() << ":Total_len:" << msg_data.get_pkt_len() << ":count in map:" << userid_pipeptr_map.count(l_trd_id);
        if (userid_pipeptr_map.count(l_trd_id) > 0)
        {
            dlvr_to_usr_ptr pipe_ptr = userid_pipeptr_map[l_trd_id];
            pipe_ptr->deliver_to_usr(msg_data);
        }
    }
}

void pipe_handler::leave_pipehandler(dlvr_to_usr_ptr pipe_ptr)
{
    std::lock_guard<std::mutex> lock(pipe_mtx);
    if (user_pipes_.count(pipe_ptr) == 1)
    {
        user_pipes_.erase(pipe_ptr);
    }
    const long l_trd_id = pipe_ptr->get_trader_id();

    if (userid_pipeptr_map.count(l_trd_id) == 1)
    {
        userid_pipeptr_map.erase(l_trd_id);
    }
    LOG_DEBUG << LOGPATH << "In Leave_pipehandler for trader id:" << l_trd_id;
}

void pipe_handler::do_accept()
{
    boost::asio::socket_base::reuse_address opt1(true);
    acceptor_.set_option(opt1);
    acceptor_.async_accept(socket_,
                           [this](boost::system::error_code ec)
                           {
                               if (!ec)
                               {
                                   std::make_shared<UserPipe>(std::move(socket_), this)->start();
                               }
                               if (!eba_tap_globals::b_app_shutdown_flg)
                               {
                                   do_accept();
                               }
                           });
}

void pipe_handler::add_to_exg_que(const exg_data &msg)
{
    std::lock_guard<std::mutex> lock(to_exg_msg_que_mtx);
    exg_ref.deliver_to_exg(msg);
}