/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   UserPipe.h
 * Author: 33866
 *
 * Created on 1 February, 2019, 5:38 PM
 */

#ifndef USERPIPE_H
#define USERPIPE_H

#include <mutex>
#include <memory>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "exg_data.h"
#include "pipe_handler_intf.h"
#include "to_user_pipe_inf.h"
//#include "to_exg_intf.h"

using boost::asio::ip::tcp;

class UserPipe    
    : public deliver_to_usr_inf ,
      public std::enable_shared_from_this<UserPipe>
{
public:
    UserPipe(tcp::socket socket, add_to_exg_que_inf* pipe_hdlr)
            : socket_(std::move(socket)),
      usr_pipe_hldr(pipe_hdlr),
      l_trader_id(0)
  {
                b_pipe_handler_map = false;
  }
    UserPipe() = delete;
    UserPipe(const UserPipe& orig)= delete;
    virtual ~UserPipe();
    void start() override;
    void deliver_to_usr(const exg_data& c_data) override;
    const long get_trader_id() override { return l_trader_id; }
private:
    
    tcp::socket socket_;
    add_to_exg_que_inf* usr_pipe_hldr;
    std::mutex to_usr_pipe_msg_que_mtx;
    exg_data_queue_t write_msgs_;
    exg_data recv_data_to_exg;
    exg_data snd_data_to_usr;
    size_t i_recv_msg_len;
    size_t i_snd_msg_len;
    bool b_pipe_handler_map;
    long l_trader_id;
    void do_write();
    void do_read_login_header();
    void do_read_login_msg_body();
    void do_read_msg_hdr();
    void do_read_msg_body();
};

typedef std::shared_ptr<UserPipe> user_pipe_ptr;

#endif /* USERPIPE_H */

