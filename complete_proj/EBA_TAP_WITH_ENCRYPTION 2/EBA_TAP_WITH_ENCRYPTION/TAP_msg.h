/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TAP_msg.h
 * Author: 33866
 *
 * Created on 15 February, 2019, 4:46 PM
 */

#ifndef TAP_MSG_H
#define TAP_MSG_H

typedef struct adm_msg{
    char c_reqst[4];
    char c_response[124];
} adm_msg_t;


#endif /* TAP_MSG_H */

