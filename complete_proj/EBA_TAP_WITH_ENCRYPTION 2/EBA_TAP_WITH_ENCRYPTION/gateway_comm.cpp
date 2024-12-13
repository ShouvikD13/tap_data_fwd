/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   gateway_comm.cpp
 * Author: C706822
 *
 * Created on February 25, 2019, 11:42 AM
 */

#include <stdbool.h>
#include <iostream>
#include <cstring>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "gateway_comm.h"
#include "nse_xchng_mgs.h"
#include "genLogger.h"

using namespace nse_xchng_msg_namespase;

gateway_comm::gateway_comm(boost::asio::io_service &gateway_io_service,
                           long l_user_id,
                           short s_box_id,
                           const char *c_broker_id,
                           const char *c_gatway_ip_address,
                           long l_gatway_port,
                           boost::asio::ssl::stream<boost::asio::ip::tcp::socket> &ssl_socket)
    : // socket_(gateway_io_service),
      gateway_io_service(gateway_io_service),
      l_user_id(l_user_id),
      s_box_id(s_box_id),
      ssl_socket_(ssl_socket)
{
    this->socket_gateway = &ssl_socket_.lowest_layer(); //Sets socket_gateway to point to the lowest layer of the SSL socket, which is typically a TCP socket.

    strncpy(this->c_broker_id, c_broker_id, strlen(c_broker_id)); //Copies the broker ID from c_broker_id to the member c_broker_id using strncpy.

    strncpy(this->c_gatway_ip_address, c_gatway_ip_address, strlen(c_gatway_ip_address)); //Copies the gateway IP address from c_gatway_ip_address to the member 
    
    this->l_gatway_port = l_gatway_port; //Sets the member l_gatway_port to the passed l_gatway_port. 
    
    this->b_have_xchng_data = false; //Initializes b_have_xchng_data to false. This likely indicates that no exchange data is present initially.
    
    memset(this->c_ip_address, '\0', sizeof(this->c_ip_address));//Initializes the c_ip_address array to all null characters, effectively clearing it.
    
    this->l_port = 0; //Initializes l_port to 0.
    
    memset(this->c_session_key, '\0', sizeof(this->c_session_key)); // Initializes the c_session_key array to all null characters, effectively clearing it.
    
    LOG_TRACE << LOGPATH << "Gateway Constructor"; //Logs the message "Gateway Constructor" using a logging mechanism (presumably a macro or function).
}

bool gateway_comm::get_xchng_ip_port_ssn()
{

    using boost::asio::ip::tcp;
    boost::asio::ip::tcp::endpoint gateway_endpoint(boost::asio::ip::address::from_string(c_gatway_ip_address), l_gatway_port);
    /* Get gatway request -start*/ 
    /*and this socket_gateway is present in gateway_comm.h file and it is a TCP socket (lowest layer of ssl)*/ 
    socket_gateway->async_connect(gateway_endpoint, [this](boost::system::error_code ec)
                                  {
                            if (!handle_handshake()) {
                                return b_have_xchng_data;
                            }
                           handle_gateway_connect(ec); });
    gateway_io_service.run();
    return b_have_xchng_data;
}

bool gateway_comm::handle_handshake()
{
    // ssl_socket_.async_handshake(boost::asio::ssl::stream_base::client, [&](const boost::system::error_code& handshake_error) {
    //                                 if (!handshake_error) {
    //                                     std::cout << "SSL Handshake successful\n";
    //                                     // success_handshake = true;
    //                                     // Now you can perform secure communication using ssl_socket
    //                                     // boost::asio::write(ssl_socket, boost::asio::buffer(c_gatway_request));
    //                                 } else {
    //                                     std::cout << "SSL Handshake failed: " << handshake_error.message() << "\n";
    //                                 }
    //                             });
    try
    {
        ssl_socket_.handshake(boost::asio::ssl::stream_base::client); // (boost::asio::ssl::stream_base::client) this is to specify that we are from client side
        LOG_TRACE << LOGPATH << "Gateway SSL Handshake successful";
        return true;
    }
    catch (const boost::system::system_error &e)
    {
        LOG_TRACE << LOGPATH << "Gateway SSL Handshake failed" << e.what();
        return false;
    }
    return false;
}

void gateway_comm::create_gateway_request(char *c_gatway_request)
{
    LOG_TRACE << LOGPATH << "create_gateway_request";
    BOX_GATWAY_RQST st_gr;
    TAP_HDR st_hdr;
    unsigned char c_msg[BOX_GATWAY_RQST_SZ];
    // unsigned char c_checksum[MD5_SIZE];
    memset(c_msg, ' ', BOX_GATWAY_RQST_SZ);

    LOG_TRACE << LOGPATH << "Print Rqst Before Swap:";
    LOG_TRACE << LOGPATH << "user id:" << this->l_user_id << ":MSG_LEN:" << BOX_GATWAY_RQST_SZ << ":s_msg_trn_cd:" << BOX_GATWAY_RQST_CD << ":";
    LOG_TRACE << LOGPATH << "BoxId:" << s_box_id << ":BrokerId:" << c_broker_id << ":";

    st_gr.msg_hdr.l_user_id = fn_swap_long((long)this->l_user_id);
    st_gr.msg_hdr.s_msg_len = fn_swap_short(BOX_GATWAY_RQST_SZ);
    st_gr.msg_hdr.s_tran_cd = fn_swap_short(BOX_GATWAY_RQST_CD);
    st_gr.msg_hdr.l_log_tm = 0;
    memset(st_gr.msg_hdr.c_alphachar, ' ', sizeof(st_gr.msg_hdr.c_alphachar));
    st_gr.msg_hdr.s_error_cd = 0;
    st_gr.msg_hdr.ll_tm_stmp = 0;
    memset(st_gr.msg_hdr.c_tm_stmp1, '0', sizeof(st_gr.msg_hdr.c_tm_stmp1));
    memset(st_gr.msg_hdr.c_tm_stmp2, '0', sizeof(st_gr.msg_hdr.c_tm_stmp2));

    st_gr.s_box_id = fn_swap_short(s_box_id);
    fn_char_to_string(st_gr.c_broker_id, sizeof(st_gr.c_broker_id), c_broker_id, strlen(c_broker_id));
    st_gr.c_filler = ' ';

    memcpy(c_msg, &st_gr, BOX_GATWAY_RQST_SZ); // copying every thing from BOX_GATWAY_RQST (this the body of request) to the local buffer 

    create_box_hdr(st_hdr, BOX_GATWAY_RQST_CD, c_msg, BOX_GATWAY_RQST_SZ, 1);
    memcpy(c_gatway_request, &st_hdr, TAP_HDR_SZ); // here c_gatway_request is pointer to char array from handle_gateway_connect
    memcpy(c_gatway_request + TAP_HDR_SZ, c_msg, BOX_GATWAY_RQST_SZ);
    LOG_TRACE << LOGPATH << "End create_gateway_request: After Swap";
    LOG_TRACE << LOGPATH << "user id:" << st_gr.msg_hdr.l_user_id << ":MSG_LEN:" << st_gr.msg_hdr.s_msg_len << ":s_msg_trn_cd:" << st_gr.msg_hdr.s_tran_cd << ":";
    LOG_TRACE << LOGPATH << "BoxId:" << st_gr.s_box_id << ":BrokerId:" << st_gr.c_broker_id << ":";
}

void gateway_comm::check_gateway_response(char *c_gateway_response, short *s_error_id)
{
    BOX_GATWAY_RSPNS st_gatway_rspns;
    TAP_HDR st_hdr;

    memcpy(&st_hdr, c_gateway_response, TAP_HDR_SZ);
    memcpy(&st_gatway_rspns, c_gateway_response + TAP_HDR_SZ, BOX_GATWAY_RSPNS_SZ);

    LOG_DEBUG << LOGPATH << "Checking Gateway response";
    LOG_DEBUG << LOGPATH << "Before swap-tran_cd:" << st_gatway_rspns.msg_hdr.s_tran_cd << ":error_cd:" << st_gatway_rspns.msg_hdr.s_error_cd << ":";

    short si_tran_cd = fn_swap_short(st_gatway_rspns.msg_hdr.s_tran_cd);
    short si_error_cd = fn_swap_short(st_gatway_rspns.msg_hdr.s_error_cd);

    LOG_INFO << LOGPATH << "After Swap-tran_cd:" << si_tran_cd << ":error_cd:" << si_error_cd << ":";

    if (si_tran_cd != BOX_GATWAY_RSPNS_CD || si_error_cd != 0)
    {
        *s_error_id = si_error_cd;
        b_have_xchng_data = false;

        LOG_ERROR << LOGPATH << "Error in Gateway response:" << si_error_cd << ":Trans_cd:" << si_tran_cd;
    }
    else
    {
        *s_error_id = 0;
        b_have_xchng_data = true;

        LOG_TRACE << LOGPATH << "Exchange IP:" << st_gatway_rspns.c_ip_address << ":Port:" << st_gatway_rspns.l_port << ":Session:" << st_gatway_rspns.c_session_key << ":";

        fn_string_to_char(c_ip_address, sizeof(c_ip_address), st_gatway_rspns.c_ip_address, sizeof(st_gatway_rspns.c_ip_address));
        l_port = fn_swap_long(st_gatway_rspns.l_port);
        // fn_string_to_char(c_session_key, sizeof (c_session_key), st_gatway_rspns.c_session_key, sizeof (st_gatway_rspns.c_session_key));

        memcpy(c_session_key, st_gatway_rspns.c_session_key, sizeof(c_session_key));
        memcpy(c_cryptographic_Key, st_gatway_rspns.Cryptographic_Key, sizeof(c_cryptographic_Key));
        memcpy(c_cryptographic_IV, st_gatway_rspns.Cryptographic_IV, sizeof(c_cryptographic_IV));
        
        // LOG_DEBUG << LOGPATH << "End check_gateway_response: After Swap";
        LOG_DEBUG << LOGPATH << "Exchange IP:" << c_ip_address << ":Port:" << l_port << ":Session:" << c_session_key << ":";
    }
}

void gateway_comm::create_box_hdr(TAP_HDR &st_msg_hdr, short s_msg_trn_cd, unsigned char *c_msg_body, unsigned short s_msg_body_sz, long l_packet_seq)
{
    unsigned char c_checksum[MD5_SIZE]; //MD5_SIZE = (4 * sizeof(unsigned int)); /* 16 */

    LOG_DEBUG << LOGPATH << "Print header Before Swap:sizeofLONG:" << sizeof(long);
    LOG_DEBUG << LOGPATH << ":Seq:" << l_packet_seq << ":packet_len:" << TAP_HDR_SZ + s_msg_body_sz << ":";

    st_msg_hdr.s_packet_len = fn_swap_short(TAP_HDR_SZ + s_msg_body_sz);
    st_msg_hdr.l_packet_seq = fn_swap_long(l_packet_seq);

    // in above two lines we are creating the packet 
    
    get_checksum(c_msg_body, s_msg_body_sz, c_checksum); // here we calculate checksum value and store it into the c_checksum 
    memcpy(st_msg_hdr.c_checksum, c_checksum, MD5_SIZE); // here we copy the c_checksum (defined here) into the c_checksum of the TAP_HDR
    LOG_DEBUG << LOGPATH << ":Seq:" << st_msg_hdr.l_packet_seq << ":packet_len:" << st_msg_hdr.s_packet_len << ":";

    LOG_DEBUG << LOGPATH << "Create header End:";
}

void gateway_comm::get_checksum(unsigned char *c_msg_str, unsigned int i_length, unsigned char *c_checksum)
{
    md5::md5_t md5;
    md5.process(c_msg_str, i_length);
    md5.finish(c_checksum);
}

gateway_comm::~gateway_comm()
{
    std::cout << "In gateway_comm destructor" << std::endl;
    if (socket_gateway->is_open())
    {
        // socket_gateway.shutdown(SHUT_RDWR);
        boost::system::error_code ec;
        socket_gateway->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_gateway->close(ec);
    }
}

void gateway_comm::handle_gateway_connect(boost::system::error_code ec)
{
    LOG_TRACE << LOGPATH << "Connected to gateway server handle_gateway_connect:" << ec.message();
    if (!ec)
    {
        char c_gatway_response[MAX_BOX_MSG_LEN]; // Macro MAX_BOX_MSG_LEN defined in exg_data.h with size 1024 
        char c_gatway_request[MAX_BOX_MSG_LEN];
        memset(c_gatway_request, '\0', MAX_BOX_MSG_LEN); // setting null to all the indexes of array

        create_gateway_request(c_gatway_request); //preparing the request message

        ////        char c_tmp_rqst[1024];
        ////        memcpy(c_tmp_rqst, c_gatway_request + TAP_HDR_SZ, BOX_GATWAY_RQST_SZ);
        ////        BOX_GATWAY_RQST* st_gatway_rqst = reinterpret_cast<BOX_GATWAY_RQST*> (c_tmp_rqst);
        ////        TAP_HDR* st_hdr = reinterpret_cast<TAP_HDR*> (c_gatway_request);
        ////
        ////
        ////        LOG_DEBUG << "user id:" << st_gatway_rqst->msg_hdr.l_user_id << ":Seq.:" << st_hdr->l_packet_seq << ":packet_len:" << st_hdr->s_packet_len;
        ////        LOG_DEBUG << "MsgLen:" << st_gatway_rqst->msg_hdr.s_msg_len << ":s_msg_trn_cd.:" << st_gatway_rqst->msg_hdr.s_tran_cd;
        ////        LOG_DEBUG << "brkrId:" << st_gatway_rqst->c_broker_id << ":Box Id:" << st_gatway_rqst->s_box_id << ":Send data 70";

        boost::system::error_code ignored_error;
        short s_error_id;
        LOG_INFO << LOGPATH << "Sending Gateway Request to Gateway Server";

        //In below line we are sending the message 
        boost::asio::write(ssl_socket_, boost::asio::buffer(c_gatway_request),
                           boost::asio::transfer_exactly(TAP_HDR_SZ + BOX_GATWAY_RQST_SZ), ignored_error);
        std::cout << "Size of request: " << TAP_HDR_SZ + BOX_GATWAY_RQST_SZ << std::endl;
        if (!ignored_error)
        {
            LOG_DEBUG << LOGPATH << "Successfully send Gateway Request to Gateway Server";
        }
        else
        {
            LOG_ERROR << LOGPATH << "Error in sending GR:" << ignored_error.message();
        }

        memset(c_gatway_response, '\0', sizeof(c_gatway_response));
        LOG_TRACE << LOGPATH << "Waiting for Gateway Response to Gateway Server";
        size_t len = boost::asio::read(ssl_socket_, boost::asio::buffer(c_gatway_response),
                                       boost::asio::transfer_exactly(TAP_HDR_SZ + BOX_GATWAY_RSPNS_SZ), ignored_error);
        std::cout << "Length Read: " << len << std::endl;
        std::cout << "Trying to read header" << TAP_HDR_SZ << std::endl;
        std::cout << "Trying to read BOX_GATWAY_RSPNS_SZ " << BOX_GATWAY_RSPNS_SZ << std::endl;
        // std::string temp_response (c_gatway_response);
        // std::cout << "Response Received Len: " << temp_response.length() << std::endl;
        // std::cout << "Response Received: " << temp_response << std::endl;
        // for (int i=0; i<98; i++) {
        //     std::cout << c_gatway_response[i];
        // }
        // std::cout << std::endl;
        if (!ignored_error)
        {
            LOG_DEBUG << LOGPATH << "Received Gateway Response: Length of Msg Received:" << len << ":PacketLen:" << (reinterpret_cast<TAP_HDR *>(c_gatway_response)->s_packet_len);
            socket_gateway->close();
            check_gateway_response(c_gatway_response, &s_error_id);
        }
        else
        {
            socket_gateway->close();
            LOG_ERROR << LOGPATH << "Error in Received Gateway Response:" << ignored_error.message();
        }
    }
    else
    {
        LOG_ERROR << LOGPATH << "Gateway connect failed.";
    }
    // gatway_io_service.stop();
}