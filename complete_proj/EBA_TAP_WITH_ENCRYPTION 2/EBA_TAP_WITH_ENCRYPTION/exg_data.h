/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   exg_data.h
 * Author: 33866
 *
 * Created on 4 February, 2019, 12:53 PM
 */

#ifndef EXG_DATA_H
#define EXG_DATA_H

#include <cassert>
#include <cstring>
#include <deque>
#include <boost/lexical_cast.hpp>
// #include "gateway_comm.h"
#include "nse_xchng_mgs.h"

using namespace nse_xchng_msg_namespase;

#pragma pack(push, 2)
typedef struct st_ndx_msg_hdr
{
    short s_packet_len;
    long l_packet_seq;
    char c_checksum[16];
    short s_tran_cd;
    long l_log_tm;
    char c_alphachar[2];
    long l_trader_id;
    short s_error_cd;
    long long ll_tm_stmp;
    char c_tm_stmp1[8];
    char c_tm_stmp2[8];
    short s_msg_len;
} EXG_MSG_HDR;

typedef struct st_ndx_msg_hdr_tr
{
    short s_packet_len;
    long l_packet_seq;
    char c_checksum[16];
    short s_tran_cd;
    long l_log_tm;
    long l_trader_id;
} EXG_MSG_HDR_TR;

// BOX_SIGN_ON_REQUEST_IN - 23000
typedef struct st_tap_hdr
{
    short s_packet_len;
    long int l_packet_seq;
    char c_checksum[16];
} TAP_HDR;

typedef struct st_box_msg_hdr
{
    short s_tran_cd;
    long l_log_tm;
    char c_alphachar[2];
    long l_user_id;
    short s_error_cd;
    long long ll_tm_stmp;
    char c_tm_stmp1[8];
    char c_tm_stmp2[8];
    short s_msg_len;
} BOX_MSG_HDR;

// GR_REQUEST - 2400

typedef struct st_box_gatway_rqst
{
    BOX_MSG_HDR msg_hdr;
    short s_box_id;
    char c_broker_id[5];
    char c_filler;
} BOX_GATWAY_RQST;

// GR_RESPONSE - 2401

typedef struct st_box_gatway_rspns
{
    BOX_MSG_HDR msg_hdr;
    unsigned short s_box_id;
    char c_broker_id[5];
    char c_filler;
    char c_ip_address[16];
    unsigned long l_port;
    char c_session_key[8];
    char Cryptographic_Key[32];
    char Cryptographic_IV[16];
} BOX_GATWAY_RSPNS;

typedef struct st_box_reg_rqst
{
    BOX_MSG_HDR msg_hdr;
    short s_box_id;
} SECURE_BOX_REGISTRATION_REQUEST;

typedef struct st_box_reg_rspns
{
    BOX_MSG_HDR msg_hdr;
} SECURE_BOX_REGISTRATION_RESPONSE;

typedef struct st_box_sign_on_rqst
{
    BOX_MSG_HDR msg_hdr;
    short s_box_id;
    char c_broker_id[5];
    char c_reserved[5];
    char c_session_key[8];
} BOX_SIGN_ON_RQST;

// BOX_SIGN_ON_REQUEST_OUT - 23001

typedef struct st_box_sign_on_rspns
{
    BOX_MSG_HDR msg_hdr;
    short s_box_id;
    char c_reserved[10];
    char c_session_key[8];
} BOX_SIGN_ON_RSPNS;
#pragma pack(pop)

#define BOX_MSG_HDR_SZ sizeof(BOX_MSG_HDR)
#define TAP_HDR_SZ sizeof(TAP_HDR)
#define MAX_BOX_MSG_LEN 1024
#define BOX_GATWAY_RQST_CD 2400
#define BOX_GATWAY_RQST_SZ sizeof(BOX_GATWAY_RQST)
#define BOX_GATWAY_RSPNS_SZ sizeof(BOX_GATWAY_RSPNS)
#define BOX_GATWAY_RSPNS_CD 2401
#define BOX_HEART_BEAT_CD 23506
/*
typedef struct st_ndx_msg_hdr {
    char s_tran_cd[8];
    char l_trader_id[4];
    char s_msg_len[4];
} EXG_MSG_HDR;
*/
class exg_data
{
public:
    enum
    {
        MAX_BUF_LEN = 1024,
        MSG_HDR_LEN = 22, /* Tap header : 22 + Msg hdr : 40 */
        BOX_SIGN_ON_RQST_CD = 23000,
        BOX_SIGN_ON_RQST_SZ = sizeof(BOX_SIGN_ON_RQST),
        BOX_SIGN_ON_RSPNS_CD = 23001,
        BOX_SIGN_ON_RSPNS_SZ = sizeof(BOX_SIGN_ON_RSPNS),
        EXG_MSG_HDR_SZ = sizeof(EXG_MSG_HDR),
        SECURE_BOX_REGISTRATION_REQUEST_CD = 23008,
        SECURE_BOX_REGISTRATION_REQUEST_SZ = sizeof(SECURE_BOX_REGISTRATION_REQUEST),
        SECURE_BOX_REGISTRATION_RESPONSE_CD = 23009,
        SECURE_BOX_REGISTRATION_RESPONSE_SZ = sizeof(SECURE_BOX_REGISTRATION_RESPONSE)
    };

    exg_data() { memset(c_exg_data, '\0', sizeof(c_exg_data)); }
    exg_data(const exg_data &orig)
    {
        // memset(c_exg_data,'\0',sizeof(c_exg_data));
        memcpy(c_exg_data, orig.c_exg_data, sizeof(c_exg_data));
    }
    /*    exg_data(const unsigned char* c_buff){
            assert(sizeof(c_buff) == MAX_BUF_LEN);
            memset(c_exg_data,'\0',sizeof(c_exg_data));
            memcpy(c_exg_data,c_buff,((sizeof(c_buff) < MAX_BUF_LEN)?sizeof(c_buff):sizeof(c_exg_data)));
        }
     * */
    const unsigned char *getdata() { return c_exg_data; }
    unsigned char *data() { return c_exg_data; }
    unsigned char *msg_body_data() { return (c_exg_data + MSG_HDR_LEN); }

    virtual ~exg_data() {}
    exg_data &operator=(const exg_data &orig)
    {
        memset(c_exg_data, '\0', sizeof(c_exg_data));
        memcpy(c_exg_data, orig.c_exg_data, sizeof(c_exg_data));
        return *this;
    }
    const short get_trans_cd() { return fn_swap_short(reinterpret_cast<EXG_MSG_HDR *>(c_exg_data)->s_tran_cd); }
    const std::size_t get_pkt_len() { return fn_swap_short((reinterpret_cast<EXG_MSG_HDR *>(c_exg_data)->s_packet_len)); }
    const std::size_t get_msg_len() { return get_pkt_len() - MSG_HDR_LEN; }
    const std::size_t get_msg_body_len()
    {
        return ((get_pkt_len() > MSG_HDR_LEN && get_pkt_len() <= MAX_BOX_MSG_LEN) ? (get_pkt_len() - MSG_HDR_LEN) : 0);
    }

    const long get_trd_id()
    {
        return fn_swap_long(get_trans_cd() >= 20000 ? (reinterpret_cast<EXG_MSG_HDR_TR *>(c_exg_data)->l_trader_id) : (reinterpret_cast<EXG_MSG_HDR *>(c_exg_data)->l_trader_id));
    }
    const short get_error_cd() { return fn_swap_short(reinterpret_cast<EXG_MSG_HDR *>(c_exg_data)->s_error_cd); }

    // const short get_error_cd(){ return  (reinterpret_cast<EXG_MSG_HDR*>(c_exg_data)->s_error_cd) ; }

    const short get_packet_seq() { return fn_swap_long(reinterpret_cast<EXG_MSG_HDR *>(c_exg_data)->l_packet_seq); }

    // Ver 1.1
    short change_msg_body_len(int decrypted_text_len)
    {
        // reset_msg();
        // memcpy(msg_body_data(), decrypted_text, decrypted_text_len);
        reinterpret_cast<EXG_MSG_HDR *>(c_exg_data)->s_packet_len = decrypted_text_len;
    }

    void reset_msg() { memset(c_exg_data, '\0', sizeof(c_exg_data)); }
    std::string print_hex(int i_buf_len);

private:
    unsigned char c_exg_data[1024]; //buffer
};

typedef std::deque<exg_data> exg_data_queue_t;

#endif /* EXG_DATA_H */
