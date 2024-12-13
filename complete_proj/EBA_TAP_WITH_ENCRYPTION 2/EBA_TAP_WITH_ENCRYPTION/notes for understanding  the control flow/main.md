Component Flow (Low level):

* inside the main() function:-
1. 
    bool eba_tap_globals::b_app_shutdown_flg = false;

    :- using as a flag to shutDown 

2. then we are declaring some variable and setting intial value as 0.
    std::string s_temp1, s_temp2;
    short si_tmp;
    short si_box_id;
    string s_broker_id;
    int i_user_id;
    short si_max_ords_in_window;

3. then we are setting same flag again as false

    eba_tap_globals::b_app_shutdown_flg = false;

4. then after we are defining some thread_group

    boost::thread_group exg_thr_grp;
    boost::thread_group pipe_thr_grp;
    boost::thread_group pipe_resp_hdlr_thr_grp;

5. then we are defining some objects of io_service

    boost::asio::io_service gateway_io_service;
    boost::asio::io_service exgobj_io_service;
    boost::asio::io_service pipehandlr_exgresp_io_service;
    boost::asio::io_service pipes_io_service;

6. Setting io_service objects to 'work object'. which will prevent run() from returning prematurely.

    boost::asio::io_service::work work(exgobj_io_service);
    boost::asio::io_service::work work1(pipehandlr_exgresp_io_service);
    boost::asio::io_service::work work2(pipes_io_service);

7. then we are starting try block

8. inside the try block first thing is we are reading the "Tap_config.ini" file. inside the file
    there are values that we will 
    be stored in the variable we are declaring in the 2. point.

    Config tap_config_obj("TAP_Config.ini"); //constructor of Config class
    tap_config_obj.read_config(); 

    // read_config() function is declared insde the Config.h file and define in Config.cpp file
    //void read_config(); // function of Config class

9. then we are setting the threshold for logging

    /* Setting threshold for logging */ (** i don''t know yet why we have to set the threshold for loging)
        SET_LOG_SEVERITY_THRESHOLD((tap_config_obj.get_logging_lvl("LOGGING_LEVEL")));

    //** tap_config_obj.get_logging_lvl("LOGGING_LEVEL"). it is declared inside the Config class in Config.h file 
    // and define inside the file Config.cpp

10. then we are setting the values in the variable which are defined in 2. point

    tap_config_obj.get_string_val("EXG_CONNECT_IP", s_temp1);                    // Gateway IP
    tap_config_obj.get_short_val("EXG_CONNECT_PORT", si_tmp);                    // Gateway port
    tap_config_obj.get_short_val("BOX_ID", si_box_id);                           // Box id
    tap_config_obj.get_string_val("BROKER_ID", s_broker_id);                     // Broker Id
    tap_config_obj.get_int_val("USER_ID", i_user_id);                            // User id of any one pipe mapped to this box
    tap_config_obj.get_short_val("MAX_ORDERS_IN_WINDOW", si_max_ords_in_window); // Max order can be send per window

    // these function are declared in Config class inside the Config.h file. and Define inside the Config.cpp file.

11. then after we are executing the loging statements 

    LOG_INFO << LOGPATH << "Gateway IP: " << s_temp1 << ":";
    LOG_INFO << LOGPATH << "Gateway Port: " << si_tmp << ":";
    LOG_INFO << LOGPATH << "BOX_ID:" << si_box_id << ":";
    LOG_INFO << LOGPATH << "BROKER_ID:" << s_broker_id << ":";
    LOG_INFO << LOGPATH << "USER_ID:" << i_user_id << ":";
    LOG_INFO << LOGPATH << "MAX_ORDERS_IN_WINDOW:" << si_max_ords_in_window << ":";

12. then after we are communicating with the gateway /**Commmunication starts with gateway**/

    1. we are creating the context obj 'ctx' with the version 'tls_client'

    boost::asio::ssl::context ctx(boost::asio::ssl::context::tls_client);

    2. then we are setting the verify mode 

    ctx.set_verify_mode(boost::asio::ssl::context::verify_peer);

    3. then we are starting the try block and inside the try block we are verifying the certificate.

        try
        {
            ctx.load_verify_file("gr_cafo_cert3.pem"); // Here we are varifying the certificate provided by server
        }
        catch (const boost::system::system_error &e)
        {
            std::cerr << "Error loading certificate file: " << e.what() << "\n";
            return 1;
        }
    4. after the varification we are creating the ssl::stream

        boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket(gateway_io_service, ctx);

        *ssl takes the socket as a template parameter. it wraps the socket to provide the SSL/TSL support.

    5. After that we are creating the object of class gateway_comm  which is in gateway_comm.h file.
        
        gateway_comm gateway_comm_obj(gateway_io_service, i_user_id, si_box_id, s_broker_id.c_str(), s_temp1.c_str(), (long)si_tmp, ssl_socket);
        //in constructor we are pasing 7 variables. NOw let see from where they are getting there value.

        1. gateway_io_service.  // service object
        2. i_user_id.           //we are getting the value of this function from Config.cpp
        3. si_box_id.            //we are getting the value of this function from Config.cpp
        4. s_broker_id.c_str()   //we are getting the value of this function from Config.cpp
        5. s_temp1.c_str()       //we are getting the value of this function from Config.cpp
        6. (long)si_tmp          //we are getting the value of this function from Config.cpp
        7. ssl_socket           //SSL::stream object.


        *inside this (gateway_comm) class . there are 7 function declared.

    6. After that we are calling the function "gateway_comm_obj.get_xchng_ip_port_ssn();" 

        this function will starts the communication with the gateway and stores the recieved data (from requst and response ) and stores it into the structures.
    
    7. after that we are calling "gateway_comm_obj.get_gatway_status()" inside the if condition which is to tell if communication happend successfully or not.
        if it happend success fully then we are taking log of  Exg_ip , Exg_port , Session_key , inCryptographic IV , Cryptographic Key.

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
    /**Communication Ends with Gateway**/

13. we create  the endpoint for communication with 'Exchange'
    boost::asio::ip::tcp::endpoint exg_endpoint(boost::asio::ip::address::from_string(gateway_comm_obj.getExgIp()), (short)gateway_comm_obj.getExgPort());

14. then we are creating the context of encryption and intialise it.

        creation of context :
        ---------------------
        EVP_CIPHER_CTX *encrypt_ctx; // here we are creating the object pointer.

        if (!(encrypt_ctx = EVP_CIPHER_CTX_new())) {
            LOG_ERROR << LOGPATH << "Unable to create a new context. Exiting...";
            return 1;
        }

        initialising the context:
        -------------------------

        if (EVP_EncryptInit_ex(encrypt_ctx,
                       EVP_aes_256_gcm(),
                       NULL,
                       (unsigned char *)gateway_comm_obj.get_cryptographic_Key(),
                       (unsigned char *)gateway_comm_obj.get_cryptographic_IV()) != 1)
                        {
                            LOG_ERROR << LOGPATH << "Unable to initialize the encryption using AES 256 GCM cipher. Exiting...";
                            return 1;
                        }

        the function (EVP_EncryptInit_ex()) takes 5 parameters.

        encrypt_ctx  -----> context you want to initialise
        EVP_aes_256_gcm() -----> specific algorithm .
        NULL ---> specific engine (in this case there is none)
        (unsigned char *)gateway_comm_obj.get_cryptographic_Key() ----> encryption key
        (unsigned char *)gateway_comm_obj.get_cryptographic_IV() -----> initialisation vector

        /**this function (EVP_EncryptInit_ex)  returns 1 it initialisation is successful**/

        If initialization fails, an error message is logged, and the function returns 1 and program terminates.


15. the we are creating the context of decryption and initialise it. (same as step 14)

16. 








Question / Doubts :
-------------------
1. if we are passing 'gateway_io_service' in ssl_socket. then why we are passing it in gateway_comm_obj().

2. why we are sending the ssl_socket in the boost::asio::read() and boost::asio::write()

//and we are doing handshake with ssl_socket.


