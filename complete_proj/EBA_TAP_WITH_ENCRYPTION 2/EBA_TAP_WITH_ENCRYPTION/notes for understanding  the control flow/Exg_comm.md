Exchange_communication (exg_comm.h and exg_comm.cpp)

variables:
---------

 boost::asio::io_service &io_service_; 

  tcp::socket socket_;

  std::mutex exg_msg_que_mtx[2];

  volatile int i_curr_wrt_msg_q;

  std::atomic<bool> b_q_processing; 

  exg_data_queue_t msg_data_to_exg[2];

  exg_data data_to_exg;

  exg_data data_to_usr;

  bool b_set_pipe_handler;

  snd_to_usr_pipe_inf *pipe_handler_ptr;

  static unsigned long l_packet_seq;

  // static unsigned long l_packet_seq_check;
  // unsigned char c_to_usr_msg_body[1024];

  gateway_comm &gateway_comm_obj;

  bool b_box_logged_in;

  bool box_reg_res_flag;

  slidingWindow slidingWindow_;
  
  boost::asio::deadline_timer tap_deadline_tmr;

  // Ver 1.1
  EVP_CIPHER_CTX *encrypt_ctx;
  EVP_CIPHER_CTX *decrypt_ctx;
  ------------------------------

 ** we are recieving 6 parameter inthe costructor
 1. exgobj_io_service //created in the main

 2. exg_endpoint //created in the main

 3. gateway_comm_obj --------> // created in the main

 4. si_max_ords_in_window // retrieved from Configration class

 5. encrypt_ctx  // created in the main

 6. decrypt_ctx  // created in the main


----------------------------------------------
 /**so there are total 13 function in exg_comm**/

1. do_connect()
2. do_read_header()
3. do_read_body()
4. do_write()
5. deliver_to_exg()
6. do_box_signin()
7. send_reg_request()
8. create_box_reg_rqst()
9. create_signon_rqst()
10. check_box_reg_response()
11. check_box_signin()
12. check_idle()
13. create_heartbeat()

--------------------------------------------------------------------------------------------------------

* from the constructor we are calling the function do_connect(endpoint_exg) with the parameter "endpoint_exg" which is created in the main.

     boost::asio::ip::tcp::endpoint exg_endpoint(boost::asio::ip::address::from_string(gateway_comm_obj.getExgIp()), (short)gateway_comm_obj.getExgPort()); 

     // this end ppoint is reated in the main. and ip and port we are getting from the response of gateway_comm.

 ** if we are passing the socket to the socket which is totally normal. 
    if we initialize a tcp::socket with an io_service, you essentially tell the socket which io_service instance will handle its asynchronous operations.

    By associating the tcp::socket with io_service, you ensure that the asynchronous operations for the socket (like connect, read, write) will be managed by that io_service instance.

1. Connection with the exchange server:
 ---------------------------------------

    void exg_comm::do_connect(boost::asio::ip::tcp::endpoint exg_end_point)
    ----------------------------------------------
   1. 
        this function is called from the constructor of the exg_comm class with the 'endpoint of exg' as parameter.

   2. 
        then we connect with the server by pasisng the "endpoint of exg"  and the lambda function. inside the lambda function we are creating  object of error_code (because if there is any error happend during the connection , will be stored in the object of error_code).
        
        1. inside the lambda function we are checking if there is any error occured during the connection. 
        if connection is happend without any error.

           1. first we are taking the log. "Connected to exchange"

           2. then after the connection we are setting the nodelay
                1. what is nodelay?
                    boost::asio::ip::tcp::no_delay is a socket option that corresponds to the TCP_NODELAY option at the system level.

                    by doing so we can control the use of "Nagle's algorithm".
                     which is a method to improve the efficiency of TCP/IP networks by combining a number of small outgoing messages and sending them all at once.

                2. opt1(true)   

                    by passing this to the constructor of nodelay . we are diabling the Nagle's algorithm. now small outgoing message will be immediatly sent over the network.

                3. socket_.set_option(opt1) 
                this line sets the socket for no delay option.

           3. then we are calling the 7. send_reg_rqst();
2. 

4. do_write():
--------------

    this function callde from 5th. deliver_to_exg() function. 

    this function is resonsible for processsing the messages from the queue and sending them asyncronously to EXG via socket.

    1. variables that needs to be address:
    -------------------------------------
    b_read_q_empty : is initialized to false
    b_q_processing : is set to true to indicate that a write operation is in progress.
    i_wrt_q_to_read : this will tell which queue should be accessed. and this is determined by the function "curr_exg_que_to_read()" it is inside exg_comm.h .
    



5. deliver_to_exg():
-------------------
    this function is called from 7.th send_reg_rqst() function with the "exg_data object" as parameter  "box_reg_rqst_obj".

    this function is responsible for adding a message to one of two exg queues.  

    1. i_wrt_msg_q is set to the value of i_curr_wrt_msg_q, which is initially set to 0. This variable will indicate which queue (out of the two) the message will be added to.

    ```CPP
    int i_wrt_msg_q = i_curr_wrt_msg_q;
    ```

    2. then after we are taking log to tell we are inside the function.

    ```java
        LOG_DEBUG << LOGPATH << "Adding packet to exchange Q.";
    ```

    3. 
    
    A scoped block is created to lock the mutex associated with current write queue. this ensures the thread safety.

    Add Message: The message (msg) is added to the queue.

    ```c++
    {
        std::lock_guard<std::mutex> lock(exg_msg_que_mtx[i_wrt_msg_q]);
        msg_data_to_exg[i_wrt_msg_q].emplace_back(msg);
    }
    ```
    4.
        A trace log is created to indicate the addition of the packet to the queue and the current state of b_q_processing.

    ```java
        LOG_TRACE << LOGPATH << "Adding packet to exchange Q.:" << b_q_processing;
    ```

    5.
      then we are checking, if there is any write operation is currently is in the process. if not than we are calling the "do_write()" function.

    /**END**/

7. send_reg_rqst():
-------------------
this function is called from 1. do_connect() function and here we emptying the Exh_data buffer and paasing it to 8th function create_box_reg_rqst(unsigned char *box_reg_rqst)

   1. Logging the Function Entry :

        LOG_DEBUG << LOGPATH << "Inside send_reg_rqst:";

    2. Creating an exg_data Object:

        exg_data box_reg_rqst_obj;

    3. Making Buffer Empty

        box_reg_rqst_obj.reset_msg(); // here we are making char array empty of exg_data.

    4. Adding values to the buffer

        we are passing the buffere to the 8. function 
        craete_box_reg_rqst(unsigned char *box_reg_rqst)

        in this function we are filling the structure "SECURE_BOX_REGISTRATION_REQUEST" with the values of response of gatewau_comm . and then we are copying the structure to the buffer. 
        
    5. Here we are calling the deliver_to_exg(box_reg_rqst_obj); . in the argument we are passing the object f the "Exg_data" . and we are adding the object in the QUEUE.

    /**END**/

8. craete_box_reg_rqst(unsigned char *box_reg_rqst) :
-----------------------------------------------------
    this function is called from 7th send_reg_rqst() function. with parameter of a buffer of exg_data.

    in this function we are setting the values in to "SECURE_BOX_REGISTRATION_REQUEST" structure.
    then copying that data to the buffer of exg_data.

    1. Logging the function entry:

            LOG_DEBUG << LOGPATH << "Inside craete_box_reg_rqst";

    2. Creating the instance of the SECURE_BOX_REGISTRATION_REQUEST structure.

        SECURE_BOX_REGISTRATION_REQUEST st_box_reg;

    3. Setting Null to all it's field.

        memset(&st_box_reg, '\0', exg_data::SECURE_BOX_REGISTRATION_REQUEST_SZ);

    4. Setting all the field of Structure with the values of gateway_comm which was provided from the response.

    5. After setting the values to the structure we are copying it into the Buffer of exg_data.

    memcpy(box_reg_rqst, &st_box_reg, exg_data::SECURE_BOX_REGISTRATION_REQUEST_SZ);




-------------------------------------------------------------------------------------------------------------
what is the use of "b_not_processing_que = msg_data_to_exg[i_wrt_msg_q].empty();" in "deliver_to_exg(const exg_data &msg)" ?






        


