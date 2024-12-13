/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Tap_interface.h
 * Author: 33866
 *
 * Created on 14 February, 2019, 8:20 PM
 */

#ifndef TAP_INTERFACE_H
#define TAP_INTERFACE_H

#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "pipe_handler.h"
#include "TAP_msg.h"
#include "eba_tap_hdr.h"

class Tap_interface {
public:
    Tap_interface(boost::asio::io_service& io_service,
      const tcp::endpoint& endpoint,pipe_handler& pipe_handler_obj);
    Tap_interface(const Tap_interface& orig) = delete;
    void start();
    virtual ~Tap_interface();
private:
    pipe_handler& pipe_handler_obj_ref;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
    void do_accept();
    void handle_admin_session();
    void handle_request();
    void check_signal(const boost::system::error_code& ec);
    //bool b_app_shutdown_flg;
    bool b_app_close_flg;
    adm_msg_t adm_msg_obj;
    boost::asio::deadline_timer tap_deadline_tmr;
    boost::asio::io_service& adm_io_service;
};

#endif /* TAP_INTERFACE_H */

