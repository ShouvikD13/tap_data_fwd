/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   gateway_comm.h
 * Author: C706822
 *
 * Created on February 25, 2019, 11:42 AM
 */

#ifndef GATEWAY_COMM_H
#define GATEWAY_COMM_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <cstring>
#include "md5.h"
#include "exg_data.h"
// #include <exception>

#pragma pack(push, 2)

#pragma pack(pop)

class gateway_comm
{
public:
    gateway_comm() = delete;
    gateway_comm(const gateway_comm &orig) = delete;

    gateway_comm(boost::asio::io_service &gateway_io_service, long l_user_id, short s_box_id, const char *c_broker_id, const char *c_gatway_ip_address, long l_gatway_port, boost::asio::ssl::stream<boost::asio::ip::tcp::socket> &ssl_socket);
    bool get_xchng_ip_port_ssn();

    bool get_gatway_status() { return b_have_xchng_data; }
    char *getExgIp() { return c_ip_address; }
    long getExgPort() { return l_port; }
    short get_box_id() { return s_box_id; }
    long get_box_user_id() { return l_user_id; }
    char *get_session_key() { return c_session_key; }
    char *get_broker_id() { return c_broker_id; }
    char *get_cryptographic_Key() { return c_cryptographic_Key; }
    char *get_cryptographic_IV() { return c_cryptographic_IV; }
    virtual ~gateway_comm();
    void create_box_hdr(TAP_HDR &st_msg_hdr, short s_msg_trn_cd, unsigned char *c_msg_body, unsigned short s_msg_body_sz, long l_packet_seq);

private:
    void create_gateway_request(char *c_gatway_request);
    void check_gateway_response(char *c_gateway_response, short *s_error_id);
    void get_checksum(unsigned char *c_msg_str, unsigned int i_length, unsigned char *c_checksum);
    void handle_gateway_connect(boost::system::error_code ec);
    bool handle_handshake();
    long l_user_id;
    short s_box_id;
    char c_broker_id[5 + 1];
    char c_gatway_ip_address[16 + 1];
    long l_gatway_port;
    bool b_have_xchng_data;
    char c_ip_address[16 + 1];
    long l_port;
    char c_session_key[8];
    char c_cryptographic_Key[32];
    char c_cryptographic_IV[16];
    boost::asio::io_service &gateway_io_service;
    // OLD
    // boost::asio::ip::tcp::socket socket_;
    // NEW
    boost::asio::ip::tcp::socket::lowest_layer_type *socket_gateway; // socket_gateway points to the lowest layer of the SSL socket, which is typically a TCP socket.
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> &ssl_socket_;  // here we are creating the reference of ssl_socket which is created in the main.cpp file.
};

#endif /* GATEWAY_COMM_H */
