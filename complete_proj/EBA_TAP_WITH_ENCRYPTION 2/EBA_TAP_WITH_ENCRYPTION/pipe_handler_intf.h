/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   pipe_handler_intf.h
 * Author: 33866
 *
 * Created on 8 February, 2019, 8:59 PM
 */

#ifndef PIPE_HANDLER_INTF_H
#define PIPE_HANDLER_INTF_H

#include "exg_data.h"
#include "to_user_pipe_inf.h"


/* this interface is for accessing Pipe handler from Pipe session */
class add_to_exg_que_inf
{
public:
  virtual ~add_to_exg_que_inf() {}
  virtual void add_to_exg_que(const exg_data& msg) = 0;
  virtual void add_pipe(dlvr_to_usr_ptr pipe_ptr) = 0;
  virtual void add_pipe_mapping(dlvr_to_usr_ptr pipe_ptr,long trader_id) = 0;
  virtual void leave_pipehandler(dlvr_to_usr_ptr pipe_ptr) = 0;
};



#endif /* PIPE_HANDLER_INTF_H */

