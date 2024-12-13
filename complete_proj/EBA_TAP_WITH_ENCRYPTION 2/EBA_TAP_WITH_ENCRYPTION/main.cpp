/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   main.cpp
 * Author: 33866
 *
 * Created on 1 February, 2019, 12:25 PM
 */

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <thread>
#include <string>
#include <exception>
#include "gateway_comm.h"
#include "exg_data.h"
#include "genLogger.h"
#include "Config.h"
#include "Tap_interface.h"
#include "exg_comm.h"
#include "signal_handlers.h"
#include "eba_tap_hdr.h"
#include "openssl/evp.h"

using namespace std;

/*
 *
 */
//flag
bool eba_tap_globals::b_app_shutdown_flg = false;

int main(int argc, char **argv)
{

    std::string s_temp1, s_temp2;
    short si_tmp; 
    short si_box_id;
    string s_broker_id;
    int i_user_id;
    short si_max_ords_in_window;

    set_sig_handler();
    s_temp1.clear();
    s_temp2.clear();
    s_broker_id.clear();
    i_user_id = si_max_ords_in_window = si_box_id = si_tmp = 0;
    //flag
    eba_tap_globals::b_app_shutdown_flg = false;

    boost::thread_group exg_thr_grp;
    boost::thread_group pipe_thr_grp;
    boost::thread_group pipe_resp_hdlr_thr_grp;

    boost::asio::io_service gateway_io_service;
    boost::asio::io_service exgobj_io_service;
    boost::asio::io_service pipehandlr_exgresp_io_service;
    boost::asio::io_service pipes_io_service;

    boost::asio::io_service::work work(exgobj_io_service);
    boost::asio::io_service::work work1(pipehandlr_exgresp_io_service);
    boost::asio::io_service::work work2(pipes_io_service);
    try
    {
        /* Reading configuration file from Application Server*/
        Config tap_config_obj("TAP_Config.ini"); // constructor of Config class
        tap_config_obj.read_config();

        /* Setting threshold for logging */
        SET_LOG_SEVERITY_THRESHOLD((tap_config_obj.get_logging_lvl("LOGGING_LEVEL")));

        tap_config_obj.get_string_val("EXG_CONNECT_IP", s_temp1);                    // Gateway IP
        tap_config_obj.get_short_val("EXG_CONNECT_PORT", si_tmp);                    // Gateway port
        tap_config_obj.get_short_val("BOX_ID", si_box_id);                           // Box id
        tap_config_obj.get_string_val("BROKER_ID", s_broker_id);                     // Broker Id
        tap_config_obj.get_int_val("USER_ID", i_user_id);                            // User id of any one pipe mapped to this box
        tap_config_obj.get_short_val("MAX_ORDERS_IN_WINDOW", si_max_ords_in_window); // Max order can be send per window

        LOG_INFO << LOGPATH << "Gateway IP: " << s_temp1 << ":";
        LOG_INFO << LOGPATH << "Gateway Port: " << si_tmp << ":";
        LOG_INFO << LOGPATH << "BOX_ID:" << si_box_id << ":";
        LOG_INFO << LOGPATH << "BROKER_ID:" << s_broker_id << ":";
        LOG_INFO << LOGPATH << "USER_ID:" << i_user_id << ":";
        LOG_INFO << LOGPATH << "MAX_ORDERS_IN_WINDOW:" << si_max_ords_in_window << ":";

        /* gateway communication -Start */
        /* Creating gateway object */
        boost::asio::ssl::context ctx(boost::asio::ssl::context::tls_client);
        ctx.set_verify_mode(boost::asio::ssl::context::verify_peer);

        try
        {
            ctx.load_verify_file("gr_cafo_cert3.pem"); // Here we are varifing the certificate provided by server
        }
        catch (const boost::system::system_error &e)
        {
            std::cerr << "Error loading certificate file: " << e.what() << "\n";
            return 1;
        }

        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket(gateway_io_service, ctx);
        gateway_comm gateway_comm_obj(gateway_io_service, i_user_id, si_box_id, s_broker_id.c_str(), s_temp1.c_str(), (long)si_tmp, ssl_socket);
        
        //******  check above file how it is working.


        // get exchange ip, port and session
        long l_xchng_prt; // unused variable
        unsigned short si_xchng_prt; // unused variable

        /* get_xchng_ip_port_ssn connect Gateway and get xchnag IP,Port & session */
        gateway_comm_obj.get_xchng_ip_port_ssn();

        if (gateway_comm_obj.get_gatway_status())
        {
            LOG_INFO << LOGPATH << "Gateway connect successful.";
            LOG_INFO << LOGPATH << "Xchnge IP:" << gateway_comm_obj.getExgIp() << ":Port:" << gateway_comm_obj.getExgPort() << ":";
            LOG_INFO << LOGPATH << "Session Key:" << gateway_comm_obj.get_session_key() << ":"; //Session key
            LOG_INFO << LOGPATH << "Cryptographic IV:" << gateway_comm_obj.get_cryptographic_IV() << ":";   // part of Encryption changes
            LOG_INFO << LOGPATH << "Cryptographic Key:" << gateway_comm_obj.get_cryptographic_Key() << ":"; // part of Encryption changes
        }
        else
        {
            LOG_ERROR << LOGPATH << "Gateway connect Failed. Exiting...";
            return 0;
        }
        /* gatway communication -End */

        si_xchng_prt = static_cast<unsigned short>(l_xchng_prt); // unused variable


        /*the ip and port we are using from gateway_comm, and these ip and port are updated from the response of the gatway   */
        boost::asio::ip::tcp::endpoint exg_endpoint(boost::asio::ip::address::from_string(gateway_comm_obj.getExgIp()), (short)gateway_comm_obj.getExgPort());
 
        // Ver 1.1
        // Creating Context for Encryption
        EVP_CIPHER_CTX *encrypt_ctx;
        if (!(encrypt_ctx = EVP_CIPHER_CTX_new()))
        {
            LOG_ERROR << LOGPATH << "Unable to create a new context. Exiting...";
            return 1; // Terminating program due to error in creating encryption context
        }
        }
        if (EVP_EncryptInit_ex(encrypt_ctx,
                              EVP_aes_256_gcm(),
                               NULL,
                               (unsigned char *)gateway_comm_obj.get_cryptographic_Key(),
                               (unsigned char *)gateway_comm_obj.get_cryptographic_IV()) != 1)
        {
            LOG_ERROR << LOGPATH << "Unable to initialize the encryption using AES 256 GCM cipher. Exiting...";
            return 1;   // Terminating program due to error in initializing encryption context
        }
        // Creating Context for Decryption
        EVP_CIPHER_CTX *decrypt_ctx;
        if (!(decrypt_ctx = EVP_CIPHER_CTX_new()))
        {
            LOG_ERROR << LOGPATH << "Unable to create a new context. Exiting...";
            return 1;  // Terminating program due to error in creating decryption context
        }
        if (EVP_DecryptInit_ex(decrypt_ctx,
                               EVP_aes_256_gcm(),
                               NULL,
                               (unsigned char *)gateway_comm_obj.get_cryptographic_Key(),
                               (unsigned char *)gateway_comm_obj.get_cryptographic_IV()) != 1)
        {
            LOG_ERROR << LOGPATH << "Unable to initialize the decryption using AES 256 GCM cipher. Exiting...";
            return 1;  // Terminating program due to error in initializing decryption context
        }

        // Ver 1.0
        // exg_comm exg_comm_obj(exgobj_io_service, exg_endpoint, gateway_comm_obj, si_max_ords_in_window);

        // Ver 1.1
        exg_comm exg_comm_obj(exgobj_io_service, exg_endpoint, gateway_comm_obj, si_max_ords_in_window, encrypt_ctx, decrypt_ctx);

        exg_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &exgobj_io_service));
        exg_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &exgobj_io_service));

        int i_max_box_login_wait = 0;
        do
        {
            this_thread::sleep_for(std::chrono::seconds(2));
            i_max_box_login_wait++;
            if (i_max_box_login_wait > 5)
            {
                exgobj_io_service.stop();
                LOG_ERROR << LOGPATH << "BOX SIGN ON Failed. Exitting...";
                exg_thr_grp.join_all();
                return 0;
            }
        } while (!exg_comm_obj.get_box_signon_stts());

        s_temp1.clear();
        si_tmp = 0;
        tap_config_obj.get_string_val("PIPE_LISTEN_IP", s_temp1);
        tap_config_obj.get_short_val("PIPE_LISTEN_PORT", si_tmp);

        LOG_INFO << LOGPATH << "PIPE_LISTEN_IP: " << s_temp1;
        LOG_INFO << LOGPATH << "PIPE_LISTEN_PORT: " << si_tmp;

        tcp::resolver resolver2(pipehandlr_exgresp_io_service);
        tcp::resolver::iterator endpoint_itr_pipe = resolver2.resolve({boost::asio::ip::address::from_string(s_temp1), si_tmp});

        pipe_handler pipe_handler_obj(pipes_io_service, endpoint_itr_pipe, exg_comm_obj, pipehandlr_exgresp_io_service);

        pipe_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &pipes_io_service));
        pipe_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &pipes_io_service));
        pipe_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &pipes_io_service));

        pipe_resp_hdlr_thr_grp.create_thread(boost::bind(&boost::asio::io_service::run, &pipehandlr_exgresp_io_service));

        s_temp1.clear();
        si_tmp = 0;
        tap_config_obj.get_string_val("ADM_LISTEN_IP", s_temp1);
        tap_config_obj.get_short_val("ADM_LISTEN_PORT", si_tmp);

        LOG_INFO << LOGPATH << "ADM_LISTEN_IP: " << s_temp1;
        LOG_INFO << LOGPATH << "ADM_LISTEN_PORT: " << si_tmp;

        boost::asio::io_service TAP_adm_io_service;

        tcp::resolver resolver3(TAP_adm_io_service);
        tcp::endpoint endpoint_adm = (resolver3.resolve({boost::asio::ip::address::from_string(s_temp1), si_tmp}))->endpoint();

        Tap_interface TAP_adm(TAP_adm_io_service, endpoint_adm, pipe_handler_obj);
        TAP_adm.start();
    }
    catch (std::exception &e)
    {
        cout << e.what() << '\n';
        LOG_ERROR << LOGPATH << "Exception occurred :" << e.what();
    }
    catch (...)
    {
        cout << "Unknown exception occurred." << '\n';
        LOG_ERROR << LOGPATH << "Unknown Exception occurred :";
    }

    LOG_INFO << LOGPATH << "Exiting TAP Application";

    gateway_io_service.stop();
    exgobj_io_service.stop();
    pipehandlr_exgresp_io_service.stop();
    pipes_io_service.stop();

    exg_thr_grp.join_all();
    if (pipe_thr_grp.size() > 0)
    {
        pipe_thr_grp.join_all();
    }
    pipe_resp_hdlr_thr_grp.join_all();
    return 0;
}