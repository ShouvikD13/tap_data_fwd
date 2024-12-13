/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   to_exg_intf.h
 * Author: 33866
 *
 * Created on 1 February, 2019, 5:40 PM
 */

#ifndef TO_EXG_INTF_H
#define TO_EXG_INTF_H

#include "to_pipe_handler_from_exgobj_intf.h"


/* This interface is for accessing Exg obj from Pipe handler */
class snd_to_exg_inf
{
public:
  virtual ~snd_to_exg_inf() {}
  virtual void deliver_to_exg(const exg_data& msg) = 0;
  virtual void set_pipe_handler(snd_to_usr_pipe_inf * pipe_hdlr_ptr) = 0;
  virtual int get_sndque_count()const = 0; 
 
};

typedef snd_to_exg_inf* snd_to_exg_ptr;

/* this interface is for accessing Pipe handler from Pipe session */
/* class add_to_exg_que_inf
{
public:
  virtual ~add_to_exg_que_inf() {}
  virtual void add_to_exg_que(const exg_data& msg) = 0;
  virtual void add_pipe(user_pipe_ptr pipe_ptr) = 0;
  virtual void add_pipe_mapping(user_pipe_ptr pipe_ptr,long trader_id) = 0;
  virtual void leave_pipehandler(user_pipe_ptr pipe_ptr) = 0;
};
*/
//typedef std::shared_ptr<add_to_exg_que_inf> add_to_exg_que_ptr;

#endif /* TO_EXG_INTF_H */

