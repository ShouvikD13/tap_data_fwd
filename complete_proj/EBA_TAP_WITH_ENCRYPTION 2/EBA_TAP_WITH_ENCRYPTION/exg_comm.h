/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   exg_comm.h
 * Author: 33866
 *
 * Created on 5 February, 2019, 6:22 PM
 */

#ifndef EXG_COMM_H
#define EXG_COMM_H

#include <atomic>
#include <mutex>
#include <deque>
#include <thread>
#include <boost/asio.hpp>
#include <exception>
#include <openssl/evp.h>

#include "exg_data.h"
#include "gateway_comm.h"
#include "to_exg_intf.h"
#include "slidingWindow.h"

#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::asio::ip::tcp;

class exg_comm
    : public snd_to_exg_inf
{
public:
  exg_comm(boost::asio::io_service &io_service, tcp::endpoint endpoint_exg, gateway_comm &gateway_comm_obj, short si_max_ords_in_window, EVP_CIPHER_CTX *encrypt_ctx, EVP_CIPHER_CTX *decrypt_ctx)
      : io_service_(io_service),
        socket_(io_service),
        i_curr_wrt_msg_q(0),
        b_set_pipe_handler(false), gateway_comm_obj(gateway_comm_obj),
        slidingWindow_(si_max_ords_in_window),
        tap_deadline_tmr(io_service, boost::posix_time::seconds(30)),
        encrypt_ctx(encrypt_ctx),
        decrypt_ctx(decrypt_ctx)
  {
    // slidingWindow_(si_window_sz_milliseconds, si_max_ords_in_window);
    b_box_logged_in = false;
    box_reg_res_flag = false;
    b_q_processing = false;
    do_connect(endpoint_exg);
  }
  exg_comm() = delete;
  exg_comm(const exg_comm &orig) = delete;
  virtual ~exg_comm();
  virtual void deliver_to_exg(const exg_data &msg) override;
  virtual void set_pipe_handler(snd_to_usr_pipe_inf *pipe_hdlr_ptr) override
  {
    pipe_handler_ptr = pipe_hdlr_ptr;
    b_set_pipe_handler = true;
  };
  int get_sndque_count() const override { return (msg_data_to_exg[0].size() + msg_data_to_exg[1].size()); }
  bool get_box_signon_stts() { return b_box_logged_in; }

  void create_signon_request(unsigned char *c_gatway_request);
  bool check_signon_response(unsigned char *c_sign_on_response, short *s_error_id);
  void craete_box_reg_rqst(unsigned char *box_reg_rqst);
  void check_box_reg_response(unsigned char *c_box_reg_response);

private:
  void do_connect(tcp::endpoint endpoint_exg);
  // bool do_signin();
  void do_read_header();
  void do_read_body();
  void do_write();
  void toggle_wrt_que() { i_curr_wrt_msg_q = (i_curr_wrt_msg_q == 0) ? 1 : 0; }
  const int curr_exg_que_to_read() { return ((i_curr_wrt_msg_q == 0) ? 1 : 0); }
  void send_reg_rqst();
  void do_box_signin();
  bool check_box_signin(unsigned char *c_box_sign_in_rspns);
  bool write_queues_empty() const { return (msg_data_to_exg[0].empty() && msg_data_to_exg[1].empty()); }
  void check_idle(const boost::system::error_code &ec);
  void create_heartbeat(unsigned char *c_heartbeat);

  boost::asio::io_service &io_service_; 

  tcp::socket socket_;

  std::mutex exg_msg_que_mtx[2]; // two mutex are created one for each operation. read and write


  volatile int i_curr_wrt_msg_q;

  std::atomic<bool> b_q_processing; 

  exg_data_queue_t msg_data_to_exg[2];

  exg_data data_to_exg;

  exg_data data_to_usr;

  bool b_set_pipe_handler;

  snd_to_usr_pipe_inf *pipe_handler_ptr;

  static unsigned long l_packet_seq;

  // static unsigned long l_packet_seq_check;
  // unsigned char c_to_usr_msg_body[1024];

  gateway_comm &gateway_comm_obj;

  bool b_box_logged_in;

  bool box_reg_res_flag;

  slidingWindow slidingWindow_;

  boost::asio::deadline_timer tap_deadline_tmr;

  // Ver 1.1
  EVP_CIPHER_CTX *encrypt_ctx;
  EVP_CIPHER_CTX *decrypt_ctx;
};

class Box_Signin_Fail : public std::exception
{
  const char *what() const throw()
  {
    return "Box Sign-in Fail";
  }
};

#endif /* EXG_COMM_H */
