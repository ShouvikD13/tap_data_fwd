# Exg_data.h
---------------

this file contains various structures and 'exg_data class' . which appears to handle and processign and manipulation of the exchange data messages.

* structres
------------

1. EXG_MSG_HDR : represents the header of exchange messages
' short s_packet_len: Length of the packet'
' long l_packet_seq: Packate sequence number'
char c_checksum[16] : checksum of packets
short s_tran_cd: transaction code
long l_log_tm : Log time 
char c_alphachar[2] : Alphabatic character
long l_trader_id : Trader Id
short s_error_cd: Error code.
long long ll_tm_stmp: Timestamp.
char c_tm_stmp1[8]: Timestamp part 1.
char c_tm_stmp2[8]: Timestamp part 2.
short s_msg_len: Message length.

2. EXG_MSG_HDR_TR : I think it is a simplified version of EXG_MSG_HDR. because it containes most variables of EXG_MSG_HDR.
    short s_packet_len;
    long l_packet_seq;
    char c_checksum[16];
    short s_tran_cd;
    long l_log_tm;
    long l_trader_id;

3. TAP_HDR : represents the header of TAP 

short s_packet_len: Length of the packet.
long int l_packet_seq: Packet sequence number.
char c_checksum[16]: Checksum of the packet.

4. BOX_MSG_HDR: represents header of Box Messages
All the field are same here as EXG_MSG_HDR . but with l_user_id instead of l_trader_id.
and it didn't contains 's_packet_len' , 'l_packet_seq' , 'c_checksum[16]'.

5. BOX_GATWAY_RQST: Represents a BOX gateway request.

BOX_MSG_HDR msg_hdr: Message header.
short s_box_id: Box ID.
char c_broker_id[5]: Broker ID.
char c_filler: Filler character.

6. BOX_GATWAY_RSPNS: Represents a BOX gateway response.

BOX_MSG_HDR msg_hdr: Message header.
unsigned short s_box_id: Box ID.
char c_broker_id[5]: Broker ID.
char c_filler: Filler character.
char c_ip_address[16]: IP address.
unsigned long l_port: Port number.
char c_session_key[8]: Session key.
char Cryptographic_Key[32]: Cryptographic key.
char Cryptographic_IV[16]: Cryptographic initialization vector.

7. SECURE_BOX_REGISTRATION_REQUEST: Represents a secure box registration request.

BOX_MSG_HDR msg_hdr: Message header.
short s_box_id: Box ID.

8. SECURE_BOX_REGISTRATION_RESPONSE: Represents a secure box registration response.

BOX_MSG_HDR msg_hdr: Message header.

9. BOX_SIGN_ON_RQST: Represents a box sign-on request.

BOX_MSG_HDR msg_hdr: Message header.
short s_box_id: Box ID.
char c_broker_id[5]: Broker ID.
char c_reserved[5]: Reserved characters.
char c_session_key[8]: Session key.

10. BOX_SIGN_ON_RSPNS: Represents a box sign-on response.

BOX_MSG_HDR msg_hdr: Message header.
short s_box_id: Box ID.
char c_reserved[10]: Reserved characters.
char c_session_key[8]: Session key.

* Macros:
---------
BOX_MSG_HDR_SZ:  sizeof(BOX_MSG_HDR) ----> Size of BOX_MSG_HDR.

TAP_HDR_SZ:sizeof(TAP_HDR) ----> Size of TAP_HDR.

MAX_BOX_MSG_LEN: 1024 ----> Maximum length of a BOX message.

BOX_GATWAY_RQST_CD: 2400 -----> Code for BOX gateway request.

BOX_GATWAY_RQST_SZ: sizeof(BOX_GATWAY_RQST) ----> Size of BOX_GATWAY_RQST.

BOX_GATWAY_RSPNS_SZ: sizeof(BOX_GATWAY_RSPNS) ----> Size of BOX_GATWAY_RSPNS.

BOX_GATWAY_RSPNS_CD: 2401 --->  Code for BOX gateway response.

BOX_HEART_BEAT_CD: 23506 ----> Code for BOX heartbeat message.

class:
----------

Enumerations:
-------------
1. MAX_BUF_LEN: 
---------------
Defines the maximum length of the buffer used to store exchange data (1024 bytes).

2. MSG_HDR_LEN:
---------------
Length of the message header, consisting of TAP header (22 bytes) and message header (40 bytes).

3. BOX_SIGN_ON_RQST_CD:
-----------------------
Code for BOX sign-on request messages.

4. BOX_SIGN_ON_RQST_SZ:
-----------------------
Size of the BOX sign-on request message structure.

5. BOX_SIGN_ON_RSPNS_CD:
-------------------------
Code for BOX sign-on response messages.

6. BOX_SIGN_ON_RSPNS_SZ: 
------------------------
Size of the BOX sign-on response message structure.

7. EXG_MSG_HDR_SZ: 
------------------
Size of the exchange message header structure.

8. SECURE_BOX_REGISTRATION_REQUEST_CD: 
--------------------------------------
Code for secure box registration request messages.

9. SECURE_BOX_REGISTRATION_REQUEST_SZ: 
--------------------------------------
Size of the secure box registration request message structure.

10. SECURE_BOX_REGISTRATION_RESPONSE_CD: 
----------------------------------------
Code for secure box registration response messages.

11. SECURE_BOX_REGISTRATION_RESPONSE_SZ:
----------------------------------------
Size of the secure box registration response message structure.
--------------------------------------------------------------------------------------------------------------
Variables: 
----------

c_exg_data[1024]

------------------------

constructor() : in constructor we are setting the '\0' to all the indexes.
Copy Constructor: Copies data from another exg_data object to initialize a new object.
~Destructor: Cleans up any resources when an exg_data object is destroyed (default behavior).

-------------------------
methods:
--------
    Assignment Operator (operator overloading ):
    --------------------------------------------
    same working as copy constructor. "exg_data &operator=(const exg_data &orig)"
    Copies data from another exg_data object.

    Accessors:
    ----------
    const unsigned char *getdata() { return c_exg_data; } ----> Returns the data buffer as a constant pointer.

    unsigned char *data() { return c_exg_data; }----> Returns the data buffer as a mutable pointer.

    unsigned char *msg_body_data() { return (c_exg_data + MSG_HDR_LEN); } ----> Returns a pointer to the message body data (skipping the header).

    Getter:
    -------
    const short get_trans_cd(): Returns the transaction code (byte-swapped).
    
    const std::size_t get_pkt_len(): Returns the packet length (byte-swapped).
    
    const std::size_t get_msg_len(): Returns the message length.
    
    const std::size_t get_msg_body_len(): Returns the length of the message body.
    
    const long get_trd_id(): Returns the trader ID (byte-swapped) based on the transaction code.
    
    const short get_error_cd(): Returns the error code (byte-swapped).
    
    const short get_packet_seq(): Returns the packet sequence number (byte-swapped).

    Modifiers:
    ---------
    short change_msg_body_len(int decrypted_text_len): Changes the message body length.

    void reset_msg(): Resets the message buffer to zero.

    std::string print_hex(int i_buf_len): Returns a hex string representation of the buffer.


--------------------
exg_data.cpp
------------
This file implements the methods declared in exg_data.h.

print_hex:
----------
Converts the buffer data to a hexadecimal string for a given length.
Uses a stringstream to format the data into a hex string.



---------------------------------------------------------------------------------------------------------------
Question :
what is packet .
what are all these variables and where will these used ?
where we will use EXG_MSG_HDR_TR ?
what is BOX , BOx mesages?
what is BOX_SIGN_ON_RQST and BOX_SIGN_ON_RSPNS ?
what is reserved characters and session key ?

if we already have the Macros then what is the need of defining the Enumeration ?

why we have used typedef ?

