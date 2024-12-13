so in gateway_comm file we have 14 variables.

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
    boost::asio::ip::tcp::socket::lowest_layer_type *socket_gateway; // socket_gateway points to the lowest layer of the SSL socket, which is typically a TCP socket.
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> &ssl_socket_;  // here we are creating the reference of ssl_socket which is created in the main.cpp file.
---------------------------------------------------------------------------
* where are these variables are used

long l_user_id
--------------
create_gateway_request(): Used to set msg_hdr.l_user_id.

short s_box_id
--------------
create_gateway_request(): Used to set s_box_id.

char c_broker_id[5 + 1]
------------------------
create_gateway_request(): Used in fn_char_to_string() function call.

char c_gatway_ip_address[16 + 1]
--------------------------------
get_xchng_ip_port_ssn(): Used to create gateway_endpoint.

long l_gatway_port
------------------
get_xchng_ip_port_ssn(): Used to create gateway_endpoint.

bool b_have_xchng_data
----------------------
get_xchng_ip_port_ssn(): Used to return the status.
check_gateway_response(): Set to true or false based on response.

char c_ip_address[16 + 1]
-------------------------
check_gateway_response(): Set using fn_string_to_char().

long l_port
-----------
check_gateway_response(): Set with fn_swap_long().

char c_session_key[8]
---------------------
check_gateway_response(): Set with memcpy().

char c_cryptographic_Key[32]
----------------------------
check_gateway_response(): Set with memcpy().

char c_cryptographic_IV[16]
---------------------------
check_gateway_response(): Set with memcpy().

boost::asio::io_service &gateway_io_service
--------------------------------------------
get_xchng_ip_port_ssn(): Used to run asynchronous operations with gateway_io_service.run().

boost::asio::ip::tcp::socket::lowest_layer_type *socket_gateway
----------------------------------------------------------------
get_xchng_ip_port_ssn(): Used to initiate asynchronous connection.
~gateway_comm(): Used to shut down and close the socket.

boost::asio::ssl::stream<boost::asio::ip::tcp::socket> &ssl_socket_
------------------------------------------------------------------- 
handle_handshake(): Used to perform SSL handshake.
get_xchng_ip_port_ssn(): Used to write the gateway request.

----------------------------------------------------------------------------------------------------------------
now the functions with flow:

Name of tthe functions:
-----------------------
1. bool gateway_comm::get_xchng_ip_port_ssn()
2. bool gateway_comm::handle_handshake()
3. void gateway_comm::create_gateway_request(char *c_gatway_request)
4. void gateway_comm::check_gateway_response(char *c_gateway_response, short *s_error_id)
5. void gateway_comm::create_box_hdr(TAP_HDR &st_msg_hdr, short s_msg_trn_cd, unsigned char *c_msg_body, unsigned short s_msg_body_sz, long l_packet_seq)
6. void gateway_comm::get_checksum(unsigned char *c_msg_str, unsigned int i_length, unsigned char *c_checksum)
7. void gateway_comm::handle_gateway_connect(boost::system::error_code ec)

** All the variables are initialise with Config Class (Config.cpp)  functions.

Execution of these functions start when in the main.cpp, we call first function (gateway_comm_obj.get_xchng_ip_port_ssn();).
1. get_xchng_ip_port_ssn() is the 1st funtion in above list. 

--> so in this function what we do is, we create the an endpoint to connect with the gateway.
--> how do we create the endpoint is, we create the object of endpoint (boost::asio::ip::tcp::endpoint) and in the constructor we will pass the (ip_address and port )
---> the values of (c_ip_address) and (l_gateway_port) are actually provided by the config.cpp 
---> After that we start the connection by using the async_connect() in the function we pass two things 
    1. gateway_endpoint and 2. a Lambda function in this function we are performing the handshake by calling handle_handshake().
    if handshake is successfull then it will call handle_gateway_connect(ec). 
    which will do following things
    1. Logging Connection Status
    2. Handling Successful Connection
    3. Preparing and Sending the Request
    4. Handling Request Sending Result
    5. Receiving and Processing the Response
    6. Handling Response Reception Result

2. handle_handshake() this was called from get_xchng_ip_port_ssn() function. in this function we are just 
--> performing the handshake by calling .handshake(boost::asio::ssl::stream_base::client) and passing boost::asio::ssl::stream_base::client . 
--> boost::asio::ssl::stream_base::client, this is passed to specify that this is a client-side.
--> if handshake is successfuly happened without any error then this function  'handle_handshake()' will return 'true'.
--> otherwise it will return  'false'.  

3. create_gateway_request() 
---------------------------
    1. this function is called from 7. handle_gateway_connect() with the parameter of a character array nameed 'c_gatway_request'.

    2. then we take the log "create_gateway_request".
        LOG_TRACE << LOGPATH << "create_gateway_request";

    3.  Creates an object st_gr of the structure BOX_GATWAY_RQST.
        Creates an object st_hdr of the structure TAP_HDR.
        Declares a buffer c_msg of size BOX_GATWAY_RQST_SZ to hold the gateway request message.

    4. 
        Initializes the c_msg buffer with spaces.
        memset(c_msg, ' ', BOX_GATWAY_RQST_SZ);

    5. 
        Logs the initial state of the request, including user ID, message length, transaction code, box ID, and broker ID.

        LOG_TRACE << LOGPATH << "Print Rqst Before Swap:";
        LOG_TRACE << LOGPATH << "user id:" << this->l_user_id << ":MSG_LEN:" << BOX_GATWAY_RQST_SZ << ":s_msg_trn_cd:"<< BOX_GATWAY_RQST_CD << ":";
        LOG_TRACE << LOGPATH << "BoxId:" << s_box_id << ":BrokerId:" << c_broker_id << ":";

    6.  Sets the fields of the st_gr.msg_hdr (message header), swapping the byte order where necessary:
        Swaps the byte order of this->l_user_id and assigns it to l_user_id.
        Swaps the byte order of BOX_GATWAY_RQST_SZ and assigns it to s_msg_len.
        Swaps the byte order of BOX_GATWAY_RQST_CD and assigns it to s_tran_cd.
        Initializes other fields like l_log_tm, c_alphachar, s_error_cd, ll_tm_stmp, c_tm_stmp1, and c_tm_stmp2.

        st_gr.msg_hdr.l_user_id = fn_swap_long((long)this->l_user_id);
        st_gr.msg_hdr.s_msg_len = fn_swap_short(BOX_GATWAY_RQST_SZ);
        st_gr.msg_hdr.s_tran_cd = fn_swap_short(BOX_GATWAY_RQST_CD);
        st_gr.msg_hdr.l_log_tm = 0;
        memset(st_gr.msg_hdr.c_alphachar, ' ', sizeof(st_gr.msg_hdr.c_alphachar));
        st_gr.msg_hdr.s_error_cd = 0;
        st_gr.msg_hdr.ll_tm_stmp = 0;
        memset(st_gr.msg_hdr.c_tm_stmp1, '0', sizeof(st_gr.msg_hdr.c_tm_stmp1));
        memset(st_gr.msg_hdr.c_tm_stmp2, '0', sizeof(st_gr.msg_hdr.c_tm_stmp2));

    7.  
        After setting those values in the 'msg_hdr' Structure and this structure is also a structure of       BOX_GATWAY_RQST Structer. so after 'msg_hdr' we fill remaning value of Box_gateway_RQST.
        whixh are:
        box_id , broker_id, filler.

    8.  
        After structer completly filled we copy it into the message buffer.
        memcpy(c_msg, &st_gr, BOX_GATWAY_RQST_SZ);

    9. 
        After that we create the header by calling the function (create_box_hdr 5th function)
         create_box_hdr(st_hdr, BOX_GATWAY_RQST_CD, c_msg, BOX_GATWAY_RQST_SZ, 1);
         here :
         ------
                st_hdr (is object of TAP_HDR structure)
                BOX_GATWAY_RQST_CD (macro of size 2400)
                c_msg (message buffer defined in this class)
                BOX_GATWAY_RQST_SZ (macro which gives the size of the BOX_GATWAY_RQST Structure)
                1
    
    10. 
        after this we add the header to the st_hdr to the 'c_gatway_request' which was passed to the function
        and then after we copy message buffer to 'c_gatway_request'.

        create_box_hdr(st_hdr, BOX_GATWAY_RQST_CD, c_msg, BOX_GATWAY_RQST_SZ, 1);
        memcpy(c_gatway_request, &st_hdr, TAP_HDR_SZ);
        memcpy(c_gatway_request + TAP_HDR_SZ, c_msg, BOX_GATWAY_RQST_SZ);

    11. 
        Logs the details of the request after the byte swapping operations, such as user ID, message length,    transaction code, box ID, and broker ID.

4. check_gateway_response(char *c_gateway_response, short *s_error_id):
-----------------------------------------------------------------------

    this function is cllaed from 7.  handle_gateway_connect() function.

   1. this function accepts 2 arguments.
        1. *c_gateway_response --> it is a response buffer declared in handle gateway connect
        2. *s_error_id --->  it will store any error code found in the response. so after the response function we can check the error_code and detect that response successfully recieved or not .

    2. then we are declaring 2 structers 

         BOX_GATWAY_RSPNS st_gatway_rspns; // for response body
        TAP_HDR st_hdr; // for response header

        here we will store the response which is stored in the response buffer.

    3. then we are copying the response header and body.

        memcpy(&st_hdr, c_gateway_response, TAP_HDR_SZ); // TAP_HDR_SZ (size of header . defined in exg_data.h)
        memcpy(&st_gatway_rspns, c_gateway_response + TAP_HDR_SZ, BOX_GATWAY_RSPNS_SZ);

    4. then we are taking the log 

            LOG_DEBUG << LOGPATH << "Checking Gateway response";
            LOG_DEBUG << LOGPATH << "Before swap-tran_cd:" << st_gatway_rspns.msg_hdr.s_tran_cd << ":error_cd:" << st_gatway_rspns.msg_hdr.s_error_cd << ":";
    
    5. Check for Errors in Response:

        if (si_tran_cd != BOX_GATWAY_RSPNS_CD || si_error_cd != 0)
    
        Checks if the transaction code is not equal to "BOX_GATWAY_RSPNS_CD" or if there is a non-zero error code.

    6. Handle Error (if we recieve error in the response) 
        if above condition matched (means there is an error in the response) then we store the recieved error code from response (st_gatway_rspns.msg_hdr.s_error_cd) into 
        s_error_id. 

        *s_error_id = si_error_cd;
        b_have_xchng_data = false; 

        LOG_ERROR << LOGPATH << "Error in Gateway response:" << si_error_cd << ":Trans_cd:" << si_tran_cd;
    
    7. Handle successful response (if we successfully recieved the response from gateway)

        1.
            we set *s_error_id to '0'
            and b_have_xchng_data to true.

            *s_error_id = 0;
            b_have_xchng_data = true;

        2. then we take Logs trace information about the received IP address, port, and session key of EXG.

            LOG_TRACE << LOGPATH << "Exchange IP:" << st_gatway_rspns.c_ip_address << ":Port:" << st_gatway_rspns.l_port << ":Session:" << st_gatway_rspns.c_session_key << ":";

        3. Here we are updating the IP address.(we are getting the IP address of EXG from the Response of gatway)

            fn_string_to_char(c_ip_address, sizeof(c_ip_address), st_gatway_rspns.c_ip_address, sizeof(st_gatway_rspns.c_ip_address));
        
        4. Here we are updating the port with the port of EXG.
            
            l_port = fn_swap_long(st_gatway_rspns.l_port);

        5. Copies the session key, cryptographic key, and initialization vector from st_gatway_rspns to corresponding member variables of gateway_comm class.

            memcpy(c_session_key, st_gatway_rspns.c_session_key, sizeof(c_session_key));
            memcpy(c_cryptographic_Key, st_gatway_rspns.Cryptographic_Key, sizeof(c_cryptographic_Key));
            memcpy(c_cryptographic_IV, st_gatway_rspns.Cryptographic_IV, sizeof(c_cryptographic_IV));



5. create_box_hdr(TAP_HDR &st_msg_hdr, short s_msg_trn_cd, unsigned char *c_msg_body, unsigned short s_msg_body_sz, long l_packet_seq) :   --------------------------------------------------------------
-----------------------------------
    This function is called from 3. create_gateway_request() function. there are 5 parameters in this function . 
        1. TAP_HDR object reference.
        2. BOX_GATEWAY_RQST_CD (macro of size 2400) (it is in the exg_data.h)
        3. Buffer (buffer is defined in the 3. function)
        4. BOX_GATEWAY_RQST_SZ (macro which calculate the size of BOX_GATWAY_RQST) (it is in the exg_data.h)
        5. 1 (literal value) (from 3rd function)

    1. 
        first in this function we are creating local character array called 'c_checksum' of size "MD5_SIZE" this size is defined in the MD5.h file. which is "16".

    2.
        then we log 

        LOG_DEBUG << LOGPATH << "Print header Before Swap:sizeofLONG:" << sizeof(long);
        LOG_DEBUG << LOGPATH << ":Seq:" << l_packet_seq << ":packet_len:" << TAP_HDR_SZ + s_msg_body_sz << ":";
    
    3.
        then we set the "Length of the packet" and "Packet sequence number".
        st_msg_hdr.s_packet_len = fn_swap_short(TAP_HDR_SZ + s_msg_body_sz);
        st_msg_hdr.l_packet_seq = fn_swap_long(l_packet_seq);

        both these values (above) are fields of TAP_HDR. and we are creating the packet here.

    4.
        And after making the packet. we are calculating the checksome value by calling the 6.th function get_checksum.

        get_checksum(c_msg_body, s_msg_body_sz, c_checksum); // here we calculate checksum value and store it into the c_checksum.

        here:
        c_msg_body -> buffer
        s_msg_body ->  BOX_GATEWAY_RQST_SZ (macro which calculate the size of BOX_GATWAY_RQST) (it is in the exg_data.h)
        c_checksum -> local character array.
    
    5. after loading the checksum value in the local array . we copy same value in the c_checksum of the TAP_HDR.

    memcpy(st_msg_hdr.c_checksum, c_checksum, MD5_SIZE); // here we copy the c_checksum (defined here) into the c_checksum of the TAP_HDR

    6. then After we take the logs.

        LOG_DEBUG << LOGPATH << ":Seq:" << st_msg_hdr.l_packet_seq << ":packet_len:" << st_msg_hdr.s_packet_len << ":";

    LOG_DEBUG << LOGPATH << "Create header End:";


6. void gateway_comm::get_checksum(unsigned char *c_msg_str, unsigned int i_length, unsigned char *c_checksum):
---------------------------------------------------------------------------------------------------------------
    this function is called from 5th create_box_hdr () function.
    1. in this function first we are creating the object of MD5.
    2. then after we call two function to calculate the checksum
        1.process()
        2.finish()

7. handle_gateway_connect()
---------------------------
 this funtion is called from 1st get_xchng_ip_port_ssn() function.

   1.  
        This function is a member of the gateway_comm class and takes a boost::system::error_code as a parameter, which indicates whether there was an error in the connection process.

        void gateway_comm::handle_gateway_connect(boost::system::error_code ec)

    
    2. Logging Connection Status:

        Logs the status of the connection attempt. If ec is empty, it means the connection was successful

        LOG_TRACE << LOGPATH << "Connected to gateway server handle_gateway_connect:" << ec.message();
    
    3. Check for Errors:

            if (!ec)
        Proceeds only if there were no errors during the connection.


    4.  in this function we are declaring 2 character array.
        1. 
            request buffer // char c_gatway_response[MAX_BOX_MSG_LEN]; // Macro MAX_BOX_MSG_LEN is exg_data.h with size 1024 
        2. 
            response buffer //char c_gatway_request[MAX_BOX_MSG_LEN];

    5. then we are setting null to all elements of request array 
        memset(c_gatway_request, '\0', MAX_BOX_MSG_LEN);
    
    6. 
        then we are creating gateway request by calling the function 
        create_gateway_request(c_gatway_request);  and by passing the the request buffer as the arguent. so created request (Header + Body (content will be stored here) ).
    
    7. Log Sending Request:

        LOG_INFO << LOGPATH << "Sending Gateway Request to Gateway Server";

    8. Send Gateway Request:

        boost::asio::write(ssl_socket_, boost::asio::buffer(c_gatway_request),
                   boost::asio::transfer_exactly(TAP_HDR_SZ + BOX_GATWAY_RQST_SZ), ignored_error);

        Uses boost::asio::write to send the request buffer over the ssl_socket_. The exact number of bytes to transfer is the sum of TAP_HDR_SZ and BOX_GATWAY_RQST_SZ.
    
    9. Check for Sending Errors:

        if (!ignored_error)
        {
            LOG_DEBUG << LOGPATH << "Successfully send Gateway Request to Gateway Server";
        }
        else
        {
            LOG_ERROR << LOGPATH << "Error in sending GR:" << ignored_error.message();
        }

    10. then after we Initialize Response Buffer:

        memset(c_gatway_response, '\0', sizeof(c_gatway_response));

        Sets all elements of the c_gatway_response buffer to null.

    11. Log waiting for response

        LOG_TRACE << LOGPATH << "Waiting for Gateway Response to Gateway Server";

    12. Receive Gateway Response:

        size_t len = boost::asio::read(ssl_socket_, boost::asio::buffer(c_gatway_response),
                               boost::asio::transfer_exactly(TAP_HDR_SZ + BOX_GATWAY_RSPNS_SZ), ignored_error);
    
        Uses boost::asio::read to receive the response from the gateway server into the c_gatway_response buffer. The expected size is the sum of TAP_HDR_SZ and BOX_GATWAY_RSPNS_SZ.

    13. Check for Receiving Errors:

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

    If there are no errors, logs the received response and its length, closes the socket, and checks the response using check_gateway_response.
    If there are errors, logs the error message and closes the socket.

    14. Handle Connection Failure (first if which was trying ti see if connection is successful)

        else
        {
            LOG_ERROR << LOGPATH << "Gateway connect failed.";
        }



---------------------------------------------------------------------------------------------------------------
ques:
-----
1. what is len ? 

    

----------------------------------------
1.
in get_xchng_ip_port_ssn(). what are the last two lines ??

gateway_io_service.run();
return b_have_xchng_data;

2. b_have_xchng_data what is this?



---------------------------------------------------------------------------------------------------
Question:
---------

in third function why we are accessing the elements of msg_hdr through BOX_GATWAY_RQST st_gr ? (why not directly ?)

in the 4.  check_gatway_response() what are we checking in the if condition (below)?
if (si_tran_cd != BOX_GATWAY_RSPNS_CD || si_error_cd != 0).

what is actually BOX_GATWAY_RSPNS_CD ?
ans:
----
        this variable is used for telling if the communication (rqst/rspns) happening successfully .

        this variable is declared in the gateway_comm class and modified in check_gateway_response() of gateway_comm.cpp.
        so if response is recieved without any error then it is set to true otherwise false.

    