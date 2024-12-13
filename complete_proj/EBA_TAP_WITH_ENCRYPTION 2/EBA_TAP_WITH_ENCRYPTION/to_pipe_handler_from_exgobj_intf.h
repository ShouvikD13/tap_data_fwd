/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   to_pipe_handler_from_exgobj_intf.h
 * Author: 33866
 *
 * Created on 11 February, 2019, 7:29 PM
 */

#ifndef TO_PIPE_HANDLER_FROM_EXGOBJ_INTF_H
#define TO_PIPE_HANDLER_FROM_EXGOBJ_INTF_H

#include "exg_data.h"

/* this interface is used to access pipe handler from exg_obj */
class snd_to_usr_pipe_inf
{
public:
  virtual ~snd_to_usr_pipe_inf() {}
  virtual void deliver_to_handlr(const exg_data& msg) = 0;
  //virtual void set_pipe_handler()=0;
};

//typedef std::shared_ptr<snd_to_usr_pipe_inf> snd_to_usr_pipe_ptr;



#endif /* TO_PIPE_HANDLER_FROM_EXGOBJ_INTF_H */

