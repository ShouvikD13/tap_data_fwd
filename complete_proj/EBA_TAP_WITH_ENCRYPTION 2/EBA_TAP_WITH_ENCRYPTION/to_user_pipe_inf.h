/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   to_user_pipe_inf.h
 * Author: 33866
 *
 * Created on 4 February, 2019, 5:50 PM
 */

#ifndef TO_USER_PIPE_INF_H
#define TO_USER_PIPE_INF_H

#include "exg_data.h"

/* this interface is used to access pipe obj from pipe handler */
 class deliver_to_usr_inf
{
public:
  virtual ~deliver_to_usr_inf() {}
  virtual void start() = 0;
  virtual void deliver_to_usr(const exg_data& msg) = 0;
  virtual const long get_trader_id() = 0;
};

typedef std::shared_ptr<deliver_to_usr_inf>  dlvr_to_usr_ptr;


#endif /* TO_USER_PIPE_INF_H */

