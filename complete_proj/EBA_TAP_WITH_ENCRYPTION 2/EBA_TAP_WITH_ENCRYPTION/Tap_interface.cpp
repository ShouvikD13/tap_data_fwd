/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Tap_interface.cpp
 * Author: 33866
 *
 * Created on 14 February, 2019, 8:20 PM
 */

#include "Tap_interface.h"
#include "genLogger.h"
#include "signal_handlers.h"

Tap_interface::Tap_interface(boost::asio::io_service &io_service,
                             const tcp::endpoint &endpoint, pipe_handler &pipe_handler_obj)
    : acceptor_(io_service, endpoint),
      socket_(io_service),
      tap_deadline_tmr(io_service, boost::posix_time::seconds(2)),
      adm_io_service(io_service),
      pipe_handler_obj_ref(pipe_handler_obj),
      // b_app_shutdown_flg(false),
      b_app_close_flg(false)
{
}

/*
Tap_interface::Tap_interface(const Tap_interface& orig) {
}
 * */

Tap_interface::~Tap_interface()
{
  if (acceptor_.is_open())
  {
    acceptor_.cancel();
    acceptor_.close();
  }
  if (socket_.is_open())
  {
    socket_.close();
  }
}

void Tap_interface::start()
{
  do_accept();
  tap_deadline_tmr.expires_from_now(boost::posix_time::seconds(2));
  LOG_DEBUG << LOGPATH << "TAP_INT:Before async wait";
  tap_deadline_tmr.async_wait([this](const boost::system::error_code &e_c)
                              {
        if(e_c == boost::asio::error::operation_aborted) return;
        check_signal(e_c); });
  LOG_DEBUG << LOGPATH << "TAP_INT:Before adm_io_service run";
  adm_io_service.run();

  LOG_DEBUG << LOGPATH << "TAP_INT:After adm_io_service run";
}

void Tap_interface::do_accept()
{
  acceptor_.async_accept(socket_,
                         [this](boost::system::error_code ec)
                         {
                           if (!ec)
                           {
                             b_app_close_flg = false;
                             handle_admin_session();
                           }
                           else
                           {
                             eba_tap_globals::b_app_shutdown_flg = true;
                           }
                         });
}

void Tap_interface::handle_admin_session()
{
  memset(static_cast<void *>(&adm_msg_obj), '\0', sizeof(adm_msg_obj));
  boost::asio::async_read(socket_,
                          boost::asio::buffer(adm_msg_obj.c_reqst, sizeof(adm_msg_obj.c_reqst)),
                          [this](boost::system::error_code ec, std::size_t /*length*/)
                          {
                            if (!ec)
                            {
                              handle_request();
                            }
                            else
                            {
                              socket_.close();
                              b_app_close_flg = true;
                            }
                            if (b_app_close_flg && !eba_tap_globals::b_app_shutdown_flg)
                            {
                              do_accept();
                            }
                          });
}

void Tap_interface::handle_request()
{

  switch (adm_msg_obj.c_reqst[0])
  {
  case 'a':
  { /* request for Exg send queue count. */
    sprintf(adm_msg_obj.c_response, "%d", pipe_handler_obj_ref.get_exg_sndque_count());
    break;
  }
  case 'b':
  { /* request for Exg recv queue count. */
    sprintf(adm_msg_obj.c_response, "%d", pipe_handler_obj_ref.get_exg_rcvque_count());
    break;
  }
  case 'c':
  { /* request for pipe count. */
    sprintf(adm_msg_obj.c_response, "%d", pipe_handler_obj_ref.get_pipe_count());
    break;
  }
  case 'q':
  { /* request for closing app interface */
    b_app_close_flg = true;
    socket_.close();
    return;
  }
  case 's':
  { /* request for shutdown of TAP */
    eba_tap_globals::b_app_shutdown_flg = true;
    socket_.close();
    return;
  }
  default:
  {
    strcpy(adm_msg_obj.c_response, "Unknown request.");
  }
  }

  LOG_DEBUG << LOGPATH << "TAP_INTERFACE response :" << adm_msg_obj.c_response;

  boost::asio::async_write(socket_,
                           boost::asio::buffer(adm_msg_obj.c_response, sizeof(adm_msg_obj.c_response)),
                           [this](boost::system::error_code ec, std::size_t /*length*/)
                           {
                             if (!ec)
                             {
                               handle_admin_session();
                             }
                             else
                             {
                               socket_.close();
                             }
                           });
}
void Tap_interface::check_signal(const boost::system::error_code &ec)
{
  if (ec == boost::asio::error::operation_aborted)
  {
    return;
  }
  if (gi_signal_recvd > 0)
  {
    eba_tap_globals::b_app_shutdown_flg = true;
    LOG_INFO << LOGPATH << "TAP_INT:signal_recvd:" << gi_signal_recvd;
  }
  // LOG_DEBUG << "TAP_INT:Inside check sig:" << eba_tap_globals::b_app_shutdown_flg;

  if (!eba_tap_globals::b_app_shutdown_flg)
  {
    tap_deadline_tmr.expires_from_now(boost::posix_time::seconds(2));
    tap_deadline_tmr.async_wait([this](const boost::system::error_code &e_c)
                                {
        if(e_c == boost::asio::error::operation_aborted) return;
      
        check_signal(e_c); });
  }
  else
  {
    adm_io_service.stop();
  }
  // LOG_DEBUG << "TAP_INT:Before return.";
  return;
}