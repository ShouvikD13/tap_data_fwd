/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   exg_comm.cpp
 * Author: 33866
 *
 * Created on 5 February, 2019, 6:22 PM
 */

#include <chrono>
#include <exception>
#include "exg_comm.h"
#include "to_user_pipe_inf.h"
#include "genLogger.h"
#include "eba_tap_hdr.h"
#include "aes256.h"

#include "nse_xchng_mgs.h"
using namespace nse_xchng_msg_namespase;
using namespace std;

/*
exg_comm::exg_comm() {
}

exg_comm::exg_comm(const exg_comm& orig) {
}
 */

unsigned long exg_comm::l_packet_seq = 2;
exg_comm::~exg_comm()
{

    LOG_DEBUG << LOGPATH << "In exg_comm destructor" << std::endl;
    if (socket_.is_open())
    {
        // socket_.shutdown(SHUT_RDWR);
        boost::system::error_code ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }
}

void exg_comm::do_connect(boost::asio::ip::tcp::endpoint exg_end_point)
{

    socket_.async_connect(exg_end_point,
                          [this](boost::system::error_code ec /*, tcp::endpoint exg_end_point*/)
                          {
                              if (!ec)
                              {
                                  LOG_INFO << LOGPATH << "Connected to exchange";
                                  boost::asio::ip::tcp::no_delay opt1(true);
                                  socket_.set_option(opt1);
                                  send_reg_rqst();
                                  do_box_signin();
                                  do_read_header();
                              }
                              else
                              {
                                  // socket_.shutdown(SHUT_RDWR);
                                  socket_.close();
                                  LOG_ERROR << LOGPATH << "Not connected to exchange:ErrorMsg:" << ec.message();
                                  eba_tap_globals::b_app_shutdown_flg = true;
                              }
                          });
}

void exg_comm::do_read_header()
{
    boost::asio::async_read(socket_,
                            boost::asio::buffer(data_to_usr.data(), exg_data::MSG_HDR_LEN),
                            [this](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec /*&& data_to_usr.decode_header()*/)
                                {
                                    LOG_INFO << LOGPATH << "Msg from exchange: TraderId:" << data_to_usr.get_trd_id() << ":TransCd:" << data_to_usr.get_trans_cd();
                                    LOG_DEBUG << LOGPATH << "do_read_header: Msg_CODE:" << data_to_usr.get_trans_cd() << ":BodyLen:" << data_to_usr.get_msg_body_len() << ":PktLen:" << data_to_usr.get_pkt_len();
                                    do_read_body();
                                }
                                else
                                {
                                    // socket_.shutdown(SHUT_RDWR);
                                    LOG_ERROR << LOGPATH << "Msg from exchange: Error:" << ec.message();
                                    // boost::system::error_code sk_ec;
                                    // socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,sk_ec);
                                    // socket_.close(sk_ec);
                                    eba_tap_globals::b_app_shutdown_flg = true;
                                }
                            });
}

void exg_comm::do_read_body()
{
    unsigned char encrypted_text[1024]; // local buffer
    boost::asio::async_read(socket_,
                            boost::asio::buffer(encrypted_text, data_to_usr.get_msg_body_len()),
                            [this, encrypted_text](boost::system::error_code ec, std::size_t /*length*/)
                            {
                                if (!ec)
                                {
                                    if (!box_reg_res_flag)
                                    {
                                        check_box_reg_response((unsigned char *)encrypted_text);
                                    }
                                    else
                                    {
                                        // Ver 1.1
                                        // Decrypting the data_to_usr.msg_body_data() of length data_to_usr.get_msg_body_len()
                                        int received_bytes = data_to_usr.get_msg_body_len();
                                        unsigned char decrypted_text[received_bytes];
                                        int decrypted_len;

                                        if (EVP_DecryptUpdate(decrypt_ctx, decrypted_text, &decrypted_len, encrypted_text, received_bytes) != 1)
                                        {
                                            LOG_ERROR << LOGPATH << "Unable to decrypt the message.";
                                            b_box_logged_in = false;
                                            throw Box_Signin_Fail();
                                        }
                                        else
                                        {
                                            memcpy(data_to_usr.msg_body_data(), decrypted_text, decrypted_len);
                                            data_to_usr.change_msg_body_len(decrypted_len);
                                        }

                                        if (b_box_logged_in == true)
                                        {
                                            LOG_DEBUG << LOGPATH << "Exchg 3 rspns :Deliver message to message pipe handler:Msg_CODE:" << data_to_usr.get_trans_cd() << ":trdId:" << data_to_usr.get_trd_id() << ":Ful_len" << data_to_usr.get_pkt_len();

                                            pipe_handler_ptr->deliver_to_handlr(data_to_usr);
                                        }
                                        else
                                        {
                                            if (!check_box_signin(data_to_usr.data()))
                                            {
                                                b_box_logged_in = false;
                                                throw Box_Signin_Fail();
                                            }
                                            else
                                            {
                                                b_box_logged_in = true;
                                                tap_deadline_tmr.expires_from_now(boost::posix_time::seconds(30));
                                                tap_deadline_tmr.async_wait([this](const boost::system::error_code &e_c)
                                                                            {
                                if (e_c == boost::asio::error::operation_aborted) return;

                                check_idle(e_c); });
                                            }
                                        }
                                    }

                                    if (!eba_tap_globals::b_app_shutdown_flg)
                                    {
                                        do_read_header();
                                    }
                                }
                                else
                                {
                                    ////socket_.shutdown(SHUT_RDWR);
                                    socket_.close();
                                    LOG_ERROR << LOGPATH << "Msg from exchange: Error:" << ec.message();
                                    eba_tap_globals::b_app_shutdown_flg = true;
                                }
                            });
}

// void exg_comm::do_write() {
//     int i_wrt_q_to_read = curr_exg_que_to_read();
//     std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_wrt_q_to_read]);
//     if (!msg_data_to_exg[i_wrt_q_to_read].empty()) {
//         exg_data& msg_data = msg_data_to_exg[i_wrt_q_to_read].front();
//         reinterpret_cast<EXG_MSG_HDR*>(msg_data.data())->l_packet_seq = fn_swap_long(exg_comm::l_packet_seq++);
//         LOG_INFO << LOGPATH << "Msg to Exchg 2 : TraderId:" << msg_data.get_trd_id() << "Msg_CODE:" << msg_data.get_trans_cd() << ":PktSeq:" << msg_data.get_packet_seq();
//         //LOG_INFO << LOGPATH << "PKT to Exchg 2 :Send message to exchange: Msg_CODE:" << msg_data.get_trans_cd() << ":TOG:" << i_wrt_q_to_read << ":trdId:" << msg_data.get_trd_id();
//         LOG_DEBUG << LOGPATH << "Exchg 2 :Pkt_len:" << msg_data.get_pkt_len() << ":Seq:" << reinterpret_cast<EXG_MSG_HDR*>(msg_data.data())->l_packet_seq << ":";
//         LOG_DEBUG << LOGPATH << "Exchg 2 :User_id:" << reinterpret_cast<EXG_MSG_HDR*>(msg_data.data())->l_trader_id << ":";
//         slidingWindow_.validate_slidingwindow();
//
//         boost::asio::async_write(socket_, boost::asio::buffer(msg_data.getdata(),
//                 msg_data.get_pkt_len()),
//                 [this](boost::system::error_code ec, std::size_t /*length*/) {
//                     if (!ec) {
//                         LOG_TRACE << LOGPATH << "Inside async_write 1";
//                         int i_wrt_q_to_read = curr_exg_que_to_read();
//                         bool b_q_empty = false;
//                         {
//                             std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_wrt_q_to_read]);
//                             msg_data_to_exg[i_wrt_q_to_read].pop_front();
//                             b_q_empty = write_queues_empty();
//                         }
//                         LOG_TRACE << LOGPATH << "Inside async_write 2: qempty:" << b_q_empty;
//                         if (!b_q_empty) {
//                             if(!eba_tap_globals::b_app_shutdown_flg)
//                             {
//                                 do_write();
//                             }
//                         }
//                     } else {
//                         //socket_.shutdown(SHUT_RDWR);
//                         socket_.close();
//                         LOG_ERROR << LOGPATH << "Error while sending packet to exchange :error:" << ec.message();
//                         eba_tap_globals::b_app_shutdown_flg = true;
//                     }
//                 });
//         //msg_data_to_exg[i_wrt_q_to_read].pop_front();
//     } else {
//         LOG_TRACE << LOGPATH << "check for toggole" ;
//         if (!msg_data_to_exg[i_curr_wrt_msg_q].empty()) {
//             LOG_TRACE << LOGPATH << "Exchg Toggle :";
//             toggle_wrt_que();
//             do_write();
//         }
//     }
// }
//
// void exg_comm::deliver_to_exg(const exg_data& msg) {
//     try {
//         int i_wrt_msg_q = i_curr_wrt_msg_q;
//         LOG_DEBUG << LOGPATH << "Adding packet to exchange Q.";
//         bool b_not_processing_que = false;
//         { /* block created for limiting mutex lock scope */
//             std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_wrt_msg_q]);
//             b_not_processing_que = write_queues_empty();
//             msg_data_to_exg[i_wrt_msg_q].emplace_back(msg);
//         }
//         LOG_TRACE << LOGPATH << "Adding packet to exchange Q.:" << b_not_processing_que;
//         if (b_not_processing_que) {
//             do_write();
//         }
//     } catch (exception& e) {
//         LOG_ERROR << LOGPATH << "Exception in exg_comm::deliver_to_exg: " << e.what();
//         eba_tap_globals::b_app_shutdown_flg = true;
//     }
// }

void exg_comm::do_write()
{
    bool b_read_q_empty = false;
    b_q_processing = true;
    int i_wrt_q_to_read = curr_exg_que_to_read(); // here for first time the value will be returned 1.
    { /* read que block start */
        std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_wrt_q_to_read]);
        b_read_q_empty = msg_data_to_exg[i_wrt_q_to_read].empty();
        if (!b_read_q_empty) // if queue is not empty 
        {
            exg_data &msg_data = msg_data_to_exg[i_wrt_q_to_read].front();
            LOG_TRACE << LOGPATH << "For testing check seqNo before change:" << msg_data.get_packet_seq() << ":";

            reinterpret_cast<EXG_MSG_HDR *>(msg_data.data())->l_packet_seq = fn_swap_long(exg_comm::l_packet_seq++);

            /*
            RHS :
                    here we are casting the msg_data.data() in to the "EXG_MSG_HDR"

            LHS :
                    here we are incrementing the packet sequence then changing it's byte order (which will be accepted by EXG).          
                    
            Assignment :
                    and then we are assigning increamented and swaped value to the EXG_MSG_HDR's l_packet_seq.
            */

            LOG_INFO << LOGPATH << "Msg to Exchg 2 : TraderId:" << msg_data.get_trd_id() << "Msg_CODE:" << msg_data.get_trans_cd() << ":PktSeq:" << msg_data.get_packet_seq();
            // LOG_INFO << LOGPATH << "PKT to Exchg 2 :Send message to exchange: Msg_CODE:" << msg_data.get_trans_cd() << ":TOG:" << i_wrt_q_to_read << ":trdId:" << msg_data.get_trd_id();
            LOG_DEBUG << LOGPATH << "Exchg 2 :Pkt_len:" << msg_data.get_pkt_len() << ":Seq:" << reinterpret_cast<EXG_MSG_HDR *>(msg_data.data())->l_packet_seq << ":";
            LOG_DEBUG << LOGPATH << "Exchg 2 :User_id:" << reinterpret_cast<EXG_MSG_HDR *>(msg_data.data())->l_trader_id << ":";
            exg_data &msg_data_temp = msg_data_to_exg[i_wrt_q_to_read].front();
            LOG_TRACE << LOGPATH << "For testing check seqNo after change:" << msg_data_temp.get_packet_seq() << ":";
            slidingWindow_.validate_slidingwindow();


        

            // Ver 1.1
            // Encryption of msg_data
            if (msg_data.get_trans_cd() != msg_data.SECURE_BOX_REGISTRATION_REQUEST_CD)
            {
                int len = sizeof(msg_data.msg_body_data()) + EVP_MAX_BLOCK_LENGTH;
                unsigned char c_msg_encrypted[len];
                int encrypted_len;

                if (EVP_EncryptUpdate(encrypt_ctx, c_msg_encrypted, &encrypted_len, msg_data.msg_body_data(), len) != 1)
                {
                    LOG_DEBUG << LOGPATH << "Unable to encrypt the message.";
                }
                else
                {
                    LOG_DEBUG << LOGPATH << "Changing message length in header after encryption" << encrypted_len;
                    TAP_HDR st_hdr;
                    memcpy(&st_hdr, msg_data.data(), msg_data.MSG_HDR_LEN);

                    st_hdr.s_packet_len = (short)encrypted_len + msg_data.MSG_HDR_LEN;
                    LOG_DEBUG << LOGPATH << "Packet Len after encryption: " << st_hdr.s_packet_len;

                    memcpy(msg_data.data(), &st_hdr, msg_data.MSG_HDR_LEN);
                    memcpy(msg_data.msg_body_data(), c_msg_encrypted, encrypted_len);
                }
            }

            boost::asio::async_write(socket_, boost::asio::buffer(msg_data.getdata(), msg_data.get_pkt_len()),
                                     [this](boost::system::error_code ec, std::size_t /*length*/)
                                     {
                                         if (!ec)
                                         {
                                             LOG_TRACE << LOGPATH << "Inside async_write 1";
                                             int i_wrt_q_to_read = curr_exg_que_to_read();
                                             bool b_q_empty = false;
                                             {
                                                 std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_wrt_q_to_read]);
                                                 msg_data_to_exg[i_wrt_q_to_read].pop_front();
                                                 b_q_empty = msg_data_to_exg[i_wrt_q_to_read].empty();
                                                 // b_q_empty = write_queues_empty();
                                             }
                                             LOG_TRACE << LOGPATH << "Inside async_write 2: qempty:" << b_q_empty;
                                             if (b_q_empty)
                                             {
                                                 int i_wrt_msg_q = i_curr_wrt_msg_q;
                                                 //                            std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_curr_wrt_msg_q]);
                                                 //                            b_q_empty = msg_data_to_exg[i_curr_wrt_msg_q].empty();
                                                 std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_wrt_msg_q]);
                                                 b_q_empty = msg_data_to_exg[i_wrt_msg_q].empty();
                                             }
                                             LOG_TRACE << LOGPATH << "Inside async_write 3: qempty:" << b_q_empty;
                                             if (!b_q_empty)
                                             {
                                                 if (!eba_tap_globals::b_app_shutdown_flg)
                                                 {
                                                     do_write();
                                                 }
                                             }
                                             else
                                             {
                                                 b_q_processing = false;
                                             }
                                         }
                                         else
                                         {
                                             // socket_.shutdown(SHUT_RDWR);
                                             socket_.close();
                                             LOG_ERROR << LOGPATH << "Error while sending packet to exchange :error:" << ec.message();
                                             eba_tap_globals::b_app_shutdown_flg = true;
                                         }
                                     });
            // msg_data_to_exg[i_wrt_q_to_read].pop_front();
        } /* if(!b_read_q_empty) block end */

    } /* read que block end */
    if (b_read_q_empty)
    {
        LOG_TRACE << LOGPATH << "check for toggole";
        bool b_wrt_q_empty = false;
        {
            int i_wrt_msg_q = i_curr_wrt_msg_q;
            //            std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_curr_wrt_msg_q]);
            //            b_wrt_q_empty = msg_data_to_exg[i_curr_wrt_msg_q].empty();
            std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_wrt_msg_q]);
            b_wrt_q_empty = msg_data_to_exg[i_wrt_msg_q].empty();

            if (!b_wrt_q_empty)
            {
                LOG_TRACE << LOGPATH << "Exchg Toggle :";
                toggle_wrt_que();
            }
        }
        if (!b_wrt_q_empty)
        {
            LOG_TRACE << LOGPATH << "Call do_write after toggle :";
            do_write();
        }
        else
        {
            b_q_processing = false;
        }
    }
}

void exg_comm::deliver_to_exg(const exg_data &msg)
{
    try
    {
        int i_wrt_msg_q = i_curr_wrt_msg_q; //intially it is set to 0 . (in the constructor od exg_data)
        /*i_curr_wrt_msg_q is a toggle initially set to 0 */ 
        LOG_DEBUG << LOGPATH << "Adding packet to exchange Q.";
        bool b_not_processing_que = false;
        { /* block created for limiting mutex lock scope */
            //in below line we are creating two mutex one for each queue . because we have two queues
            std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_wrt_msg_q]); 
            b_not_processing_que = msg_data_to_exg[i_wrt_msg_q].empty(); // what is the use of this var
            // b_not_processing_que = write_queues_empty();
            msg_data_to_exg[i_wrt_msg_q].emplace_back(msg); // here we are setting data to queue which is present at 0th position 
        }
        LOG_TRACE << LOGPATH << "Adding packet to exchange Q.:" << b_q_processing;
        //        if(b_not_processing_que){
        //            int i_read_q = curr_exg_que_to_read();
        //            std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_read_q]);
        //            b_not_processing_que = msg_data_to_exg[i_read_q].empty();
        //        }
        //        LOG_TRACE << LOGPATH << "Adding packet to exchange Q.:" << b_not_processing_que;
        // if (b_not_processing_que) {

        if (!b_q_processing) // "b_q_processing" is false meaning no write operation is in progress.
        {
            LOG_TRACE << LOGPATH << "Calling do_write.";
            do_write();
        } 

        /*b_q_processing , b_q_processing , i_wrt_msg_q . "what are the roll of these three"*/


    }
    catch (exception &e)
    {
        LOG_ERROR << LOGPATH << "Exception in exg_comm::deliver_to_exg: " << e.what();
        eba_tap_globals::b_app_shutdown_flg = true;
    }
}

void exg_comm::do_box_signin() /* add request in exchange queue */
{
    LOG_DEBUG << LOGPATH << "Inside do_box_signin:";
    exg_data box_login_obj;
    box_login_obj.reset_msg();
    create_signon_request(box_login_obj.data());
    deliver_to_exg(box_login_obj);
}

void exg_comm::send_reg_rqst() /* add request in exchange queue */
{
    LOG_DEBUG << LOGPATH << "Inside send_reg_rqst:";
    exg_data box_reg_rqst_obj;
    box_reg_rqst_obj.reset_msg(); // here we are making Buffer of exg_data empty 
    craete_box_reg_rqst(box_reg_rqst_obj.data()); // and passing same buffer here 
    deliver_to_exg(box_reg_rqst_obj); // passing the object of the exg_data. and adding this object in the queue. 
}

/*new request for encryption and decryption*/
void exg_comm::craete_box_reg_rqst(unsigned char *box_reg_rqst) 
{
    LOG_DEBUG << LOGPATH << "Inside craete_box_reg_rqst";
    SECURE_BOX_REGISTRATION_REQUEST st_box_reg;
    memset(&st_box_reg, '\0', exg_data::SECURE_BOX_REGISTRATION_REQUEST_SZ);
    LOG_DEBUG << LOGPATH << "craete_box_reg_rqst:BOX_ID:" << gateway_comm_obj.get_box_id();
    st_box_reg.msg_hdr.l_user_id = fn_swap_long(gateway_comm_obj.get_box_user_id());
    st_box_reg.msg_hdr.s_msg_len = fn_swap_short(exg_data::SECURE_BOX_REGISTRATION_REQUEST_SZ);
    st_box_reg.msg_hdr.s_tran_cd = fn_swap_short(exg_data::SECURE_BOX_REGISTRATION_REQUEST_CD);
    st_box_reg.msg_hdr.l_log_tm = 0;
    fn_char_to_string(st_box_reg.msg_hdr.c_alphachar, sizeof(st_box_reg.msg_hdr.c_alphachar), " ", 1);
    st_box_reg.msg_hdr.s_error_cd = 0;
    st_box_reg.msg_hdr.ll_tm_stmp = 0;
    memset(st_box_reg.msg_hdr.c_tm_stmp1, '0', sizeof(st_box_reg.msg_hdr.c_tm_stmp1));
    memset(st_box_reg.msg_hdr.c_tm_stmp2, '0', sizeof(st_box_reg.msg_hdr.c_tm_stmp2));
    st_box_reg.s_box_id = fn_swap_short((short)gateway_comm_obj.get_box_id());

    memcpy(box_reg_rqst, &st_box_reg, exg_data::SECURE_BOX_REGISTRATION_REQUEST_SZ); // setting the values of structure to the Buffer
    // box_reg_rqst is char array (Buffer) from exg_data
}
void exg_comm::create_signon_request(unsigned char *c_sign_on_request)
{
    BOX_SIGN_ON_RQST st_gr;
    TAP_HDR st_hdr;
    unsigned char c_msg[exg_data::BOX_SIGN_ON_RQST_SZ];
    memset(&st_gr, '\0', exg_data::BOX_SIGN_ON_RQST_SZ);
    memset(c_msg, '\0', exg_data::BOX_SIGN_ON_RQST_SZ);

    LOG_DEBUG << LOGPATH << "BOX_SIGNIN:USER_ID:" << gateway_comm_obj.get_box_user_id();
    st_gr.msg_hdr.l_user_id = fn_swap_long(gateway_comm_obj.get_box_user_id());
    st_gr.msg_hdr.s_msg_len = fn_swap_short(exg_data::BOX_SIGN_ON_RQST_SZ);
    st_gr.msg_hdr.s_tran_cd = fn_swap_short(exg_data::BOX_SIGN_ON_RQST_CD);
    LOG_DEBUG << LOGPATH << "BOX_SIGNIN:UserId:" << st_gr.msg_hdr.l_user_id << ":msg_len:" << st_gr.msg_hdr.s_msg_len << ":TrnCD:" << st_gr.msg_hdr.s_tran_cd << ":";

    st_gr.msg_hdr.l_log_tm = 0;

    fn_char_to_string(st_gr.msg_hdr.c_alphachar, sizeof(st_gr.msg_hdr.c_alphachar), " ", 1);

    st_gr.msg_hdr.s_error_cd = 0;
    st_gr.msg_hdr.ll_tm_stmp = 0;
    memset(st_gr.msg_hdr.c_tm_stmp1, '0', sizeof(st_gr.msg_hdr.c_tm_stmp1));
    memset(st_gr.msg_hdr.c_tm_stmp2, '0', sizeof(st_gr.msg_hdr.c_tm_stmp2));

    st_gr.s_box_id = fn_swap_short((short)gateway_comm_obj.get_box_id());
    fn_char_to_string(st_gr.c_broker_id, sizeof(st_gr.c_broker_id), gateway_comm_obj.get_broker_id(), sizeof(st_gr.c_broker_id));
    // memcpy(st_gr.c_reserved,' ',5); exchange need '\0' in reserve field
    // fn_char_to_string(st_gr.c_session_key, sizeof(st_gr.c_session_key), gateway_comm_obj.get_session_key(), sizeof(st_gr.c_session_key));
    memcpy(st_gr.c_session_key, gateway_comm_obj.get_session_key(), sizeof(st_gr.c_session_key));

    memcpy(c_msg, &st_gr, exg_data::BOX_SIGN_ON_RQST_SZ);
    gateway_comm_obj.create_box_hdr(st_hdr, exg_data::BOX_SIGN_ON_RQST_CD, c_msg, exg_data::BOX_SIGN_ON_RQST_SZ, 2);

    memcpy(c_sign_on_request, &st_hdr, TAP_HDR_SZ);
    memcpy(c_sign_on_request + TAP_HDR_SZ, c_msg, data_to_usr.get_msg_body_len());

    LOG_DEBUG << LOGPATH << ":BoxId:" << (reinterpret_cast<BOX_SIGN_ON_RQST *>(c_msg)->s_box_id) << ":";
    LOG_DEBUG << LOGPATH << ":BrokId:" << (reinterpret_cast<BOX_SIGN_ON_RQST *>(c_msg)->c_broker_id) << ":";
    LOG_DEBUG << LOGPATH << ":SessionId:" << (reinterpret_cast<BOX_SIGN_ON_RQST *>(c_msg)->c_session_key) << ":";
    LOG_DEBUG << LOGPATH << "BOX_SIGNIN:PKTSeq:" << st_hdr.l_packet_seq << ":PktLen:" << st_hdr.s_packet_len << ":";
}

void exg_comm::check_box_reg_response(unsigned char *c_box_reg_response)
{
    short si_error_cd;
    short si_tran_cd;

    LOG_DEBUG << LOGPATH << "Checking box registration response: ";

    SECURE_BOX_REGISTRATION_RESPONSE st_box_reg_res;
    // TAP_HDR st_hdr;

    memcpy(&st_box_reg_res, c_box_reg_response, sizeof(SECURE_BOX_REGISTRATION_RESPONSE));

    si_tran_cd = fn_swap_short(st_box_reg_res.msg_hdr.s_tran_cd);
    si_error_cd = fn_swap_short(st_box_reg_res.msg_hdr.s_error_cd);

    LOG_DEBUG << LOGPATH << "check_box_reg_response: RSPNS_CD:" << si_tran_cd << ":error:" << si_error_cd;

    if (si_tran_cd == exg_data::SECURE_BOX_REGISTRATION_RESPONSE_CD && si_error_cd == 0)
    {
        LOG_INFO << "Secure Box Registration is successful:" << si_error_cd;
        box_reg_res_flag = true;
    }
    else
    {
        LOG_INFO << "Secure Box Registration failed:" << si_error_cd;
        box_reg_res_flag = false;
    }
}

bool exg_comm::check_box_signin(unsigned char *c_box_sign_in_rspns)
{
    short si_error_cd;
    short si_tran_cd;
    LOG_DEBUG << LOGPATH << "Checking box signin: ";
    BOX_SIGN_ON_RSPNS st_sn_rspns;
    TAP_HDR st_hdr;

    memcpy(&st_hdr, c_box_sign_in_rspns, TAP_HDR_SZ);
    memcpy(&st_sn_rspns, c_box_sign_in_rspns + TAP_HDR_SZ, exg_data::BOX_SIGN_ON_RSPNS_SZ);
    LOG_DEBUG << LOGPATH << "CHK_BOX_SIGNIN:PktLen:" << st_hdr.s_packet_len << ":CD:" << st_sn_rspns.msg_hdr.s_tran_cd << ":Err:" << st_sn_rspns.msg_hdr.s_error_cd << ":";
    si_tran_cd = fn_swap_short(st_sn_rspns.msg_hdr.s_tran_cd);
    si_error_cd = fn_swap_short(st_sn_rspns.msg_hdr.s_error_cd);
    LOG_DEBUG << LOGPATH << "check_box_signin: RSPNS_CD:" << si_tran_cd << ":error:" << si_error_cd;
    if (si_tran_cd == exg_data::BOX_SIGN_ON_RSPNS_CD && si_error_cd == 0)
    {
        LOG_INFO << "Box login is successful:" << si_error_cd;
        b_box_logged_in = true;
        return true;
    }
    else
    {
        LOG_ERROR << "Box login is failed:" << si_error_cd << ":TrnCd:" << si_tran_cd;
        b_box_logged_in = false;
        return false;
    }
}

void exg_comm::check_idle(const boost::system::error_code &ec)
{
    static bool b_heart_beat = false;
    static exg_data heart_beat_obj;
    static long l_packet_seq_check;
    if (ec == boost::asio::error::operation_aborted)
    {
        return;
    }
    if (b_heart_beat)
    {
        // LOG_DEBUG << LOGPATH << "l_packet_seq_check:" << exg_comm::l_packet_seq_check << ":l_packet_seq:" << exg_comm::l_packet_seq;
        if (l_packet_seq_check >= exg_comm::l_packet_seq)
        {
            // LOG_DEBUG << LOGPATH << "send heartbeat as system is ideal for 30 sec";
            deliver_to_exg(heart_beat_obj);
        }
    }
    else
    {
        heart_beat_obj.reset_msg();
        create_heartbeat(heart_beat_obj.data());
        b_heart_beat = true;
    }
    l_packet_seq_check = exg_comm::l_packet_seq;
    tap_deadline_tmr.expires_from_now(boost::posix_time::seconds(30));
    tap_deadline_tmr.async_wait([this](const boost::system::error_code &e_c)
                                {
        if (e_c == boost::asio::error::operation_aborted) return;
        check_idle(e_c); });
}

void exg_comm::create_heartbeat(unsigned char *c_heartbeat)
{
    BOX_MSG_HDR st_heartbeat;
    TAP_HDR st_hdr;
    unsigned char c_msg[BOX_MSG_HDR_SZ];
    memset(c_msg, '\0', BOX_MSG_HDR_SZ);
    // LOG_DEBUG << LOGPATH << "Heartbeat:USER_ID:" << gateway_comm_obj.get_box_user_id();
    st_heartbeat.l_user_id = fn_swap_long(gateway_comm_obj.get_box_user_id());
    // LOG_DEBUG << LOGPATH << "Heartbeat:UserId:" << st_heartbeat.l_user_id << ":msg_len:" << st_heartbeat.s_msg_len << ":TrnCD:" << st_heartbeat.s_tran_cd << ":";

    st_heartbeat.s_msg_len = fn_swap_short(BOX_MSG_HDR_SZ);
    st_heartbeat.s_tran_cd = fn_swap_short(BOX_HEART_BEAT_CD);
    st_heartbeat.l_log_tm = 0;

    fn_char_to_string(st_heartbeat.c_alphachar, sizeof(st_heartbeat.c_alphachar), " ", 1);

    st_heartbeat.s_error_cd = 0;
    st_heartbeat.ll_tm_stmp = 0;
    memset(st_heartbeat.c_tm_stmp1, '0', sizeof(st_heartbeat.c_tm_stmp1));
    memset(st_heartbeat.c_tm_stmp2, '0', sizeof(st_heartbeat.c_tm_stmp2));

    memcpy(c_msg, &st_heartbeat, BOX_MSG_HDR_SZ);
    gateway_comm_obj.create_box_hdr(st_hdr, BOX_HEART_BEAT_CD, c_msg, BOX_MSG_HDR_SZ, 0);

    memcpy(c_heartbeat, &st_hdr, TAP_HDR_SZ);
    memcpy(c_heartbeat + TAP_HDR_SZ, c_msg, BOX_MSG_HDR_SZ);
    // LOG_DEBUG << LOGPATH << "BOX_SIGNIN:PKTSeq:" << st_hdr.l_packet_seq << ":PktLen:" << st_hdr.s_packet_len << ":";
}
