
1. fn_pack_sprdord_to_nse
-------------------------
    first function (this is the only function i need) in the file. this function takes 6 pointers of strtures 
                                                                         1. (struct vw_xchngbook *ptr_st_rqst)
                                                                         2. (struct vw_orderbook *ptr_st_ord)
                                                                         3. (struct vw_nse_cntrct *ptr_st_nse_cnt)
                                                                         4. (struct vw_spdordbk *ptr_st_spd_ord_bk)
                                                                         5. (struct st_opm_pipe_mstr *ptr_st_opm_pipe_mst)


                            //  ABOVE FIVE FUNCTION WORKS GIVES INPUT WHICH THAN PACKED INTO 6 STRUCTURE 

                                                                         6. (struct st_req_q_data *ptr_st_q_packet)

*Working:
---------


we get 6 structures mentioned above as the input and inside the function we have used three structures. 

1. struct st_spd_oe_reqres st_spdord_req;
2. struct st_exch_msg st_exch_message;
3. struct st_pk_sequence st_s_sequence;

so we extract the data from those five structures which are in the parameters and store them into 1. structure (st_spd_oe_reqres) defined inside the function, then we copy "st_spd_oe_reqres" into "st_exch_snd_msg" and in "st_net_hdr" we set the 'total message Length' , checksum and sequence_number.
then in the end copy both structure "st_exch_snd_msg" and "st_net_hdr" into struct "st_req_q_data" .

Detailed explanition:
--------------------

step1:
------

    we are setting null to all the fields of 

            1. st_spdord_req
            2. st_exch_message
            3. st_s_sequence // only used to extract the sequence nuber

step2:
------
        in this step we are taking the log based on the "debug message level" .

step3:
------
        then we are populating the header(st_hdr) which is inside the "st_spdord_req".

step4:
------
        then we populate other fields of "st_spdord_req".

step 5:
-------
        After Populating the structure we are doing the conversion.

        "fn_cnvt_htn_spd2L3L_req(&st_spdord_req)"

step 6:
-------
        After conversion we copy it into the "st_exch_msg"

step 7:
-------
    we populate the st_s_sequence,
    Then after we calculate the sequence_number by calling " n_get_fno_pack_seq(c_ServiceName,&st_s_sequence,c_err_msg); "

step8:
------ 
    then we populate the "st_net_hdr" which has the essential fields.
    1. si_message_length  (sizeof(st_exch_message.st_net_header) + sizeof(st_spdord_req))
	2. i_seq_num         (this value comes from previous request)
	3. c_checksum        (we calculate this variable by calling "fn_cal_md5digest" function)

step 9:
-------
    then we conversion of "st_net_hdr".
    "fn_cnvt_htn_net_hdr(&st_exch_message.st_net_header)"

step10:
-------
    At the end we copy both structure "st_exch_snd_msg" and "st_net_hdr" into struct "st_req_q_data".

