/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   signal_handlers.h
 * Author: 33866
 *
 * Created on March 7, 2019, 5:27 PM
 */

#ifndef SIGNAL_HANDLERS_H
#define SIGNAL_HANDLERS_H


#include <csignal>
    extern volatile sig_atomic_t gi_signal_recvd;
    
    void sig_handler(int i_sig);
    void set_sig_handler();


#endif /* SIGNAL_HANDLERS_H */

