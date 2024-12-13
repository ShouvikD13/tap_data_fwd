/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   pipe_handler.h
 * Author: 33866
 *
 * Created on 4 February, 2019, 6:58 PM
 */

#ifndef PIPE_HANDLER_H
#define PIPE_HANDLER_H

#include <set>
#include <unordered_map>
#include <mutex>
#include <boost/asio.hpp>
#include "to_user_pipe_inf.h"
#include "pipe_handler_intf.h"
#include "exg_data.h"
#include "to_exg_intf.h"
#include "UserPipe.h"


using boost::asio::ip::tcp;

class pipe_handler
:public add_to_exg_que_inf,public snd_to_usr_pipe_inf
{
public:
    pipe_handler(boost::asio::io_service& io_service,
      const tcp::resolver::iterator& endpoint_itr,snd_to_exg_inf& exg_obj,
            boost::asio::io_service& to_usr_pipe_io_service)
    :acceptor_(io_service,endpoint_itr->endpoint()),
            socket_(io_service),
            exg_ref(exg_obj),
            msg_to_usr_io_service(to_usr_pipe_io_service)
    {
        exg_ref.set_pipe_handler(this);
        do_accept();
    }
    pipe_handler(const pipe_handler& orig)= delete;
    virtual ~pipe_handler();
    void add_to_exg_que(const exg_data& msg) override;
    void deliver_to_handlr(const exg_data& msg) override;
    void add_pipe(dlvr_to_usr_ptr pipe_ptr) override;
    void add_pipe_mapping(dlvr_to_usr_ptr pipe_ptr,long trader_id) override;
    void leave_pipehandler(dlvr_to_usr_ptr pipe_ptr) override;
    int get_exg_sndque_count()const{return exg_ref.get_sndque_count();}
    int get_exg_rcvque_count() const {return data_to_usr_msgs_que_.size();}
    int get_pipe_count() const { return userid_pipeptr_map.size();}
private:
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    std::mutex pipe_mtx;
    std::set<dlvr_to_usr_ptr> user_pipes_;
    std::unordered_map<long,dlvr_to_usr_ptr> userid_pipeptr_map;
    exg_data_queue_t data_to_usr_msgs_que_;
    std::mutex to_usr_msg_que_mtx;
    std::mutex to_exg_msg_que_mtx;
    boost::asio::io_service& msg_to_usr_io_service;
    snd_to_exg_inf& exg_ref;
    void deliver_to_usr_pipe();
    void do_accept();
};



#endif /* PIPE_HANDLER_H */

