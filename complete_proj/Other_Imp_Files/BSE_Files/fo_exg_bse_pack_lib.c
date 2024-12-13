/**********************************************************************************/
/*  Program           : fo_exg_bse_pack_lib.pc                                    */
/*                                                                                */
/*  Input             :                                                           */
/*                                                                                */
/*  Output            :                                                           */
/*                                                                                */
/*  Description       :                                                           */
/*                                                                                */
/* Log                :New Release with modified SLTP structure : Sachin Birje    */
/*                     (28-Jun-2012)                                              */  
/*                    :Ver 1.1 :Mobile CTCL ID Changes [Sachin Birje]             */
/*										:Ver 1.2 :SLTP To Normal Handling Changes ( Sandeep Patil ) */
/*										:Ver 1.3 :Order Ack changes               ( Sandip Tambe )  */
/*										:Ver 1.5 :Migration of RI to NRI    (Bhupendra Malik)       */
/*										:Ver 1.6 :BSe re-deployment (Ravi Malla)						        */
/*                    :Ver 1.7 :SLTP Order issue fix (Prajakta Sutar)             */
/*                    :Ver 1.8 :BSE Market order rejection fix (Prajakta Sutar)   */
/*                    :Ver 1.9 :RQ_13247_Order_Validity_Change_from_EOD_to_EOS (Kunwar Prashant) */
/**********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fn_log.h>
#include <fn_msgq.h>
#include <fo.h>
#include <atmi.h>
#include <fn_tuxlib.h>
#include <fn_ddr.h>
#include <fo_view_def.h>
#include <fn_read_debug_lvl.h>
#include <fo_exg_bse_pack_lib.h>
#include <fn_fo_bse_convrtn.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <sqlca.h>

EXEC SQL INCLUDE "table/icd_info_client_dtls.h";  /*** Ver 1.5 **/

int fn_pack_ordnry_ord_to_bse(struct vw_xchngbook *ptr_st_rqst1,
                              struct vw_orderbook *ptr_st_ord1,
                              struct vw_nse_cntrct *ptr_st_nse_cnt1,
                              struct st_opm_pipe_mstr *ptr_st_opm_pipe_mstr,
                              struct st_send_rqst_data *ptr_st_q_packet,
                              char *c_ServiceName
                              )
{
  struct st_ord_rqst st_ord_snd_rqst;
  struct st_del_ord_rqst st_del_ord_request;
  struct st_stoploss_ord_rqst st_sltp_snd_rqst;
  struct st_upd_sltp_rqst st_upd_sltp_request;
 
  int i_ch_val;
  int i_tmp_cntr;
  time_t ud_tm_stmp;
  int i_no;
  int i_snd_seq;
  int i_day=0;
  int i_month=0;
  int i_year=0;
  int i_trg_cnt=0;
	char c_lmt_mrkt_sl_flg;			/***	Ver	1.2	***/
  char c_err_msg[256];
  char c_user_typ_glb;
  long l_msg_typ;
  long l_previous_qty=0;
  long l_exctd_qty=0;
  long l_modified_qty=0;
  long l_mdfctn_cnt=0;
  char v_account_number[16];
  char c_clm_mtch_accnt[10];
  char c_location_id[17];
  long l_vendor_cd=0;
  long l_prgrm_trdng_flg=0;
	long l_stp_loss_trgr	=	0;				/***  Ver 1.2 ***/
	long l_market_identifier;         /** Ver 1.6 **/
	char c_algo_id[LEN_ALGO_ID + 1]; 	/** Ver 1.6 **/
	SHORT s_retention;								/** Ver 1.6 **/
	SHORT s_ord_prtctn_rt=0;					/** Ver 1.6 **/
	char c_new_msgtag[10 + 1];	 			/** Ver 1.6 **/
	char c_eti_user_ssn_id[9 + 1];    /** Ver 1.6 **/
	long int li_last_trd_time = 0;	  /** Ver 1.6 **/

  /* Ver 1.6 starts */
	EXEC SQL BEGIN DECLARE SECTION;
	varchar v_ack_number[23];
	EXEC SQL END DECLARE SECTION;
  /* Ver 1.6 Ends */

	MEMSET(c_algo_id); /** Ver 1.6 **/

  unsigned char digest[16];

  /***************Memset the structures ******************************/

  memset(&st_ord_snd_rqst, '\0', sizeof(struct st_ord_rqst));
  memset(&st_sltp_snd_rqst, '\0',sizeof(struct st_stoploss_ord_rqst));
  memset(&st_del_ord_request,'\0',sizeof(struct st_del_ord_rqst));
  memset(&st_upd_sltp_request,'\0',sizeof(struct st_upd_sltp_rqst));
 
  memset(c_location_id,'\0',sizeof(c_location_id));
 
  l_vendor_cd = VENDOR_CODE;
  /*** l_prgrm_trdng_flg = PRGRM_TRNDNG_FLG; *** Ver 1.1 ***/

  if(DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName,"Inside function fn_pack_ordnry_ord_to_bse");
  }
  
  /* Ver 1.6 starts */ 
    fn_userlog(c_ServiceName,"li_opm_user_id :%ld:",ptr_st_opm_pipe_mstr->li_opm_user_id);
    fn_userlog(c_ServiceName,"c_xchng_brkr_id :%s:",ptr_st_opm_pipe_mstr->c_xchng_brkr_id);

	MEMSET(c_eti_user_ssn_id);

	EXEC SQL SELECT 
							TO_NUMBER(TRIM(:ptr_st_opm_pipe_mstr->c_xchng_brkr_id)||'00'|| LPAD(:ptr_st_opm_pipe_mstr->li_opm_user_id,3,'0')) 						INTO 
							:c_eti_user_ssn_id
						FROM DUAL;
  /* Ver 1.6 Ends */	

    if(DEBUG_MSG_LVL_0)
  {

    fn_userlog(c_ServiceName,"Order book data");
		fn_userlog(c_ServiceName," Exchange User id :%s",c_eti_user_ssn_id);   /* ver 1.6 */

    fn_userlog(c_ServiceName,"Order book c_cln_mtch_accnt recieved is :%s:",ptr_st_ord1->c_cln_mtch_accnt);
    fn_userlog(c_ServiceName,"Order book c_ordr_flw recieved is :%c:",ptr_st_ord1->c_ordr_flw);
    fn_userlog(c_ServiceName,"Order book l_ord_tot_qty recieved is :%ld:",ptr_st_ord1->l_ord_tot_qty);
    fn_userlog(c_ServiceName,"Order book l_exctd_qty recieved is :%ld:",ptr_st_ord1->l_exctd_qty);
    fn_userlog(c_ServiceName,"Order book l_exctd_qty_day recieved is :%ld:",ptr_st_ord1->l_exctd_qty_day);
    fn_userlog(c_ServiceName,"Order book c_ctcl_id recieved is :%s:",ptr_st_ord1->c_ctcl_id);
    fn_userlog(c_ServiceName,"Order book c_ack_tm recieved is :%s:",ptr_st_ord1->c_ack_tm);

    fn_userlog(c_ServiceName,"Exchange book data");

    fn_userlog(c_ServiceName,"Exchange book c_req_typ recieved is :%c:",
                              ptr_st_rqst1->c_req_typ);
    fn_userlog(c_ServiceName,"Exchange book c_slm_flg recieved is :%c:",ptr_st_rqst1->c_slm_flg);
    fn_userlog(c_ServiceName,"Exchange book l_ord_lmt_rt recieved is :%ld:",ptr_st_rqst1->l_ord_lmt_rt);
    fn_userlog(c_ServiceName,"Exchange book l_stp_lss_tgr recieved is :%ld:",ptr_st_rqst1->l_stp_lss_tgr);
    fn_userlog(c_ServiceName,"Exchange book c_ord_typ recieved is :%c:",ptr_st_rqst1->c_ord_typ);
    fn_userlog(c_ServiceName,"Exchange book l_ord_tot_qty recieved is :%ld:",ptr_st_rqst1->l_ord_tot_qty);
    fn_userlog(c_ServiceName,"Exchange book c_valid_dt recieved is :%s:",ptr_st_rqst1->c_valid_dt);
    fn_userlog(c_ServiceName,"Exchange book l_ord_seq recieved is :%ld:",ptr_st_rqst1->l_ord_seq);
  }

  /*** Ver 1.1 Starts Here ***/

  if(strcmp(ptr_st_ord1->c_ctcl_id,"5555555555555")==0 ) /*** For Mobile Application ***/ 
  {
   l_prgrm_trdng_flg = PRGRM_MOBL_TRNDNG_FLG;
  }
  else 
  {
   l_prgrm_trdng_flg = PRGRM_TRNDNG_FLG;
  }
  /*** Ver 1.1 Ends Here ****/

  sprintf(c_location_id,"%s%ld%ld",ptr_st_ord1->c_ctcl_id,l_prgrm_trdng_flg,l_vendor_cd);
  fn_userlog(c_ServiceName," Location ID :%s:",c_location_id);

  if(	ptr_st_rqst1->c_slm_flg == LIMIT_ORDR_RQST 	|| 
			ptr_st_rqst1->c_slm_flg == MARKET_ORDR_RQST ||
			ptr_st_rqst1->c_slm_flg == SLTP_ORDR_RQST
		) 
  {

    /**st_ord_snd_rqst.l_msg_typ = LIMIT_MARKET_ORD; commented in Ver 1.6 ** 1025 **/ 
    st_ord_snd_rqst.l_msg_typ = TID_NEW_ORDER_SINGLE_REQUEST; /** 10100 **/  /* Ver 1.6 */ 
    st_ord_snd_rqst.l_scrip_cd = ptr_st_nse_cnt1->l_token_id; 
    st_ord_snd_rqst.l_msg_tag  = ptr_st_rqst1->l_ord_seq;	/* Ver 1.6 changed msg_tag1 to msg_tag */
   
   if(ptr_st_rqst1->c_req_typ == NEW )
   {
    st_ord_snd_rqst.l_qty = abs(ptr_st_rqst1->l_ord_tot_qty);  
   }
   else if(ptr_st_rqst1->c_req_typ == MODIFY )
   {
     l_mdfctn_cnt = ptr_st_rqst1->l_mdfctn_cntr;
     c_lmt_mrkt_sl_flg = ptr_st_rqst1->c_slm_flg;   /* Ver 1.7 */
 
     EXEC SQL 
     SELECT FXB_ORDR_TOT_QTY
						/* commented in Ver 1.7 FXB_LMT_MRKT_SL_FLG		***	Ver	1.2	***/
     INTO   :l_previous_qty
						/* commented in Ver 1.7 :c_lmt_mrkt_sl_flg 			***	Ver	1.2	***/
     FROM   FXB_FO_XCHNG_BOOK
     WHERE  FXB_ORDR_RFRNC = :ptr_st_rqst1->c_ordr_rfrnc
     AND    fxb_mdfctn_cntr = 
            ( SELECT MAX(fxb_mdfctn_cntr)
              FROM   FXB_FO_XCHNG_BOOK
              WHERE  FXB_PLCD_STTS = 'A'
              AND    FXB_ORDR_RFRNC = :ptr_st_rqst1->c_ordr_rfrnc
              AND    fxb_mdfctn_cntr < :l_mdfctn_cnt
            );
  
     if(SQLCODE != 0)
     {      
      fn_errlog( c_ServiceName, "L31005",SQLMSG, c_err_msg );
      return (-1); 
     }
	
		 EXEC SQL
			 SELECT NVL(SUM(FTD_EXCTD_QTY),0)
			 INTO	  :l_exctd_qty
			 FROM	  FTD_FO_TRD_DTLS
			 WHERE  FTD_ORDR_RFRNC	=	:ptr_st_rqst1->c_ordr_rfrnc;

		 if(SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
     {
      fn_errlog( c_ServiceName, "L31010",SQLMSG, c_err_msg );
      return (-1);
     }

     fn_userlog(c_ServiceName," l_previous_qty :%ld:",l_previous_qty);
     fn_userlog(c_ServiceName," l_exctd_qty :%ld:",l_exctd_qty);
     fn_userlog(c_ServiceName," l_ord_tot_qty  :%ld:", ptr_st_rqst1->l_ord_tot_qty);
     fn_userlog(c_ServiceName," c_lmt_mrkt_sl_flg :%c:", c_lmt_mrkt_sl_flg);			/***	Ver	1.2	***/

     l_modified_qty = ptr_st_rqst1->l_ord_tot_qty - l_previous_qty	;
     fn_userlog(c_ServiceName," l_modified_qty  :%ld:",l_modified_qty);
 
     st_ord_snd_rqst.l_qty = l_modified_qty;

		 /***	Ver	1.2	Starts	***/
		 if ( c_lmt_mrkt_sl_flg	==	'S'	)
		 {
				fn_userlog(c_ServiceName,"Inside Stop Loss Order Condition.");

				EXEC SQL
					SELECT	FOD_STP_LSS_TGR
					INTO		:l_stp_loss_trgr
					FROM		FOD_FO_ORDR_DTLS
					WHERE		FOD_ORDR_RFRNC	=	:ptr_st_rqst1->c_ordr_rfrnc;
		
     	if(SQLCODE != 0)
     	{
      	fn_errlog( c_ServiceName, "L31015",SQLMSG, c_err_msg );
      	return (-1);
     	}
	
			fn_userlog(c_ServiceName,"Stop Loss Trigger Price Is :%ld:",l_stp_loss_trgr);

		} /*** Ver 1.2 Ends ***/
   }
   else if(ptr_st_rqst1->c_req_typ == CANCEL )
   {
     st_ord_snd_rqst.l_qty = 0;
   }

   /* Ver 1.6 Statrs */ 
	 if(ptr_st_rqst1->c_req_typ == CANCEL || ptr_st_rqst1->c_req_typ == MODIFY )
	 {

		MEMSET(v_ack_number);

		/** 
			EXEC SQL SELECT  GREATEST(FXB_RQST_TM,FTD_TRD_DT)
			INTO :v_ack_number 
			FROM FTD_FO_TRD_DTLS,FXB_FO_XCHNG_BOOK 
			WHERE FXB_ORDR_RFRNC=FTD_ORDR_RFRNC
			AND FXB_ORDR_RFRNC=:ptr_st_rqst1->c_ordr_rfrnc;	**/

			EXEC SQL SELECT FOD_LST_ACT_REF INTO :v_ack_number
			FROM FOD_FO_ORDR_DTLS
			WHERE FOD_ORDR_RFRNC=:ptr_st_rqst1->c_ordr_rfrnc;

		if(SQLCODE != 0)
      {
        fn_errlog( c_ServiceName, "L31020",SQLMSG, c_err_msg );
        return (-1);
      }

			fn_timearr_to_long((char*)v_ack_number.arr,&li_last_trd_time);

	 }
   /* Ver 1.6 Ends */

	 fn_userlog(c_ServiceName," l_exctd_qty :%ld:",l_exctd_qty);
   fn_userlog(c_ServiceName," l_ord_tot_qty  :%ld:", ptr_st_rqst1->l_ord_tot_qty);

	 st_ord_snd_rqst.l_revealed_qty= ptr_st_rqst1->l_ord_tot_qty - l_exctd_qty ; 
	 if(ptr_st_rqst1->c_slm_flg == MARKET_ORDR_RQST )	/** Ver 1.3 if added to send rate as 0 for market order**/
	 {
   	st_ord_snd_rqst.l_rate = 0;
	 }
	 else
	 {
   	st_ord_snd_rqst.l_rate =  ptr_st_rqst1->l_ord_lmt_rt;
	 }

   st_ord_snd_rqst.ll_filler_1 = 0;
   st_ord_snd_rqst.l_filler_2 = 0;
   st_ord_snd_rqst.l_filler_3 = 0;
	 st_ord_snd_rqst.ll_reserved_1 =0;
  
   if(ptr_st_rqst1->c_req_typ == NEW )
   {
    st_ord_snd_rqst.ll_ordr_id = 0;  
   } 
   else 
   {
    fn_userlog(c_ServiceName," Inside ELSS part of rqst Typ");
    st_ord_snd_rqst.ll_ordr_id = atoll(ptr_st_ord1->c_xchng_ack);  
   }
   fn_userlog(c_ServiceName," Order id after ATOLL() :%lld:",st_ord_snd_rqst.ll_ordr_id);

   st_ord_snd_rqst.ll_location_id =atoll(c_location_id); 

   strcpy(st_ord_snd_rqst.c_client_id,ptr_st_ord1->c_cln_mtch_accnt);   

		st_ord_snd_rqst.l_msg_typ = -2;	 				 /** Ver 1.6 **/

   if( ptr_st_rqst1->c_req_typ == NEW )
   {
    /* st_ord_snd_rqst.c_ordr_actn_cd = ADD_LMT_MRKT_ORDR; commented in ver 1.6 **/
		st_ord_snd_rqst.l_msg_typ       = TID_NEW_ORDER_SINGLE_REQUEST;	/** Ver 1.6 **/
   }
   else if(ptr_st_rqst1->c_req_typ == MODIFY )
   {
    /* st_ord_snd_rqst.c_ordr_actn_cd = UPD_LMT_MRKT_ORDR; commented in ver 1.6 **/
		st_ord_snd_rqst.l_msg_typ       = TID_MODIFY_ORDER_SINGLE_REQUEST;	/** Ver 1.6 **/
   }
   else if(ptr_st_rqst1->c_req_typ == CANCEL )
   {
    /* st_ord_snd_rqst.c_ordr_actn_cd = DEL_LMT_MRKT_ORD;	commented in ver 1.6 **/
		st_ord_snd_rqst.l_msg_typ       = TID_DELETE_ORDER_SINGLE_REQUEST;	/** Ver 1.6 **/
   }
     
	
 
	 MEMSET(sql_icd_cust_type);
   
   EXEC SQL
	 SELECT  ICD_CUST_TYPE
   INTO    :sql_icd_cust_type
   FROM    ICD_INFO_CLIENT_DTLS,
           IAI_INFO_ACCOUNT_INFO
   WHERE   ICD_SERIAL_NO = IAI_SERIAL_NO
   AND     IAI_MATCH_ACCOUNT_NO  = :ptr_st_ord1->c_cln_mtch_accnt; 

   if ( SQLCODE != 0 )
   {
     fn_errlog( c_ServiceName, "L31025", SQLMSG, c_err_msg );
		 return -1;
 	 }
  
	 SETNULL(sql_icd_cust_type);
  
   
   if( ptr_st_rqst1->c_ord_typ == 'I' )  
   {
			st_ord_snd_rqst.s_retention  = IMMEDIATE;				/**** IOC Order Validity ****/
 	 }
	 else
	 {
		st_ord_snd_rqst.s_retention  = END_OF_SSSN;
	 }
     
   st_ord_snd_rqst.st_hdr.l_slot_no = REQ_SLOT_NO;
   st_ord_snd_rqst.st_hdr.l_message_length = sizeof(st_ord_snd_rqst) - sizeof(st_ord_snd_rqst.st_hdr);
   if(DEBUG_MSG_LVL_3){
   fn_userlog(c_ServiceName," **** Before Converation ");
   fn_userlog(c_ServiceName," l_msg_typ  :%ld:",st_ord_snd_rqst.l_msg_typ);
   fn_userlog(c_ServiceName," l_scrip_cd :%ld:",st_ord_snd_rqst.l_scrip_cd);
   fn_userlog(c_ServiceName," l_msg_tag :%ld:", st_ord_snd_rqst.l_msg_tag);
   fn_userlog(c_ServiceName," l_qty :%ld:", st_ord_snd_rqst.l_qty);
   fn_userlog(c_ServiceName," l_revealed_qty :%ld:",st_ord_snd_rqst.l_revealed_qty);
   fn_userlog(c_ServiceName," l_rate :%ld:",st_ord_snd_rqst.l_rate);
   fn_userlog(c_ServiceName," ll_ordr_id :%ld:",st_ord_snd_rqst.ll_ordr_id);
   fn_userlog(c_ServiceName," s_retention :%d:",st_ord_snd_rqst.s_retention);
   fn_userlog(c_ServiceName," l_slot_no :%ld:",st_ord_snd_rqst.st_hdr.l_slot_no);
   fn_userlog(c_ServiceName," l_message_length :%ld:",st_ord_snd_rqst.st_hdr.l_message_length);
	 
  
   }
   /*** Converting from host byte to Network byte ***/
   st_ord_snd_rqst.l_msg_typ   = htonl(st_ord_snd_rqst.l_msg_typ);
   st_ord_snd_rqst.l_scrip_cd  = htonl(st_ord_snd_rqst.l_scrip_cd);
   st_ord_snd_rqst.l_msg_tag  = htonl(st_ord_snd_rqst.l_msg_tag);   /* changed l_msg_tag1 to l_msg_tag in ver 1.6 */
   st_ord_snd_rqst.l_qty       = htonl(st_ord_snd_rqst.l_qty);
   st_ord_snd_rqst.l_revealed_qty   = htonl(st_ord_snd_rqst.l_revealed_qty);
   st_ord_snd_rqst.l_rate = htonl(st_ord_snd_rqst.l_rate);
   st_ord_snd_rqst.s_retention        = htons(st_ord_snd_rqst.s_retention);
   st_ord_snd_rqst.ll_filler_1  = htonl(st_ord_snd_rqst.ll_filler_1);
   st_ord_snd_rqst.l_filler_2  = htonl(st_ord_snd_rqst.l_filler_2);
   st_ord_snd_rqst.l_filler_3  = htonl(st_ord_snd_rqst.l_filler_3);
	

   st_ord_snd_rqst.st_hdr.l_slot_no  = htonl(st_ord_snd_rqst.st_hdr.l_slot_no);
   st_ord_snd_rqst.st_hdr.l_message_length = htonl(st_ord_snd_rqst.st_hdr.l_message_length);

   /*** Converation of values in exchange specific format ****/

   st_ord_snd_rqst.l_msg_typ  = fn_swap_long(st_ord_snd_rqst.l_msg_typ);
   st_ord_snd_rqst.l_scrip_cd = fn_swap_long(st_ord_snd_rqst.l_scrip_cd);
   st_ord_snd_rqst.l_msg_tag  = fn_swap_long(st_ord_snd_rqst.l_msg_tag); /* changed l_msg_tag1 to l_msg_tag in ver 1.6 */
   st_ord_snd_rqst.l_qty      = fn_swap_long(st_ord_snd_rqst.l_qty);
   st_ord_snd_rqst.l_revealed_qty   = fn_swap_long(st_ord_snd_rqst.l_revealed_qty);
  
   st_ord_snd_rqst.l_rate = fn_swap_long(st_ord_snd_rqst.l_rate);
   st_ord_snd_rqst.ll_ordr_id = fn_swap_ulnglng(st_ord_snd_rqst.ll_ordr_id);
   st_ord_snd_rqst.s_retention = fn_swap_short(st_ord_snd_rqst.s_retention);
   st_ord_snd_rqst.ll_location_id = fn_swap_ulnglng(st_ord_snd_rqst.ll_location_id);
   st_ord_snd_rqst.ll_filler_1 = fn_swap_ulnglng(st_ord_snd_rqst.ll_filler_1);
   st_ord_snd_rqst.l_filler_2 = fn_swap_long(st_ord_snd_rqst.l_filler_2);
   st_ord_snd_rqst.l_filler_3 = fn_swap_long(st_ord_snd_rqst.l_filler_3);
	
    
   st_ord_snd_rqst.st_hdr.l_slot_no = fn_swap_long(st_ord_snd_rqst.st_hdr.l_slot_no);
	 strcpy(st_ord_snd_rqst.c_algo_id,c_algo_id);	/** Ver 1.6 **/
   st_ord_snd_rqst.st_hdr.l_message_length = fn_swap_long(st_ord_snd_rqst.st_hdr.l_message_length);
  	
			l_market_identifier=0;

		  EXEC SQL  SELECT BTQ_PRD_ID 
			INTO :l_market_identifier
      FROM   BTQ_BFO_TRD_QT
      WHERE   BTQ_TOKEN_NO=:ptr_st_nse_cnt1->l_token_id;
		 if ( SQLCODE != 0 )
   	 {
    	 fn_errlog( c_ServiceName, "L31030", SQLMSG, c_err_msg );
     	return -1;
   	 } 

   	ptr_st_q_packet->li_msg_type =  LIMIT_MARKET_ORD;
		if(ptr_st_rqst1->c_req_typ == NEW)
		{
			NewOrderSingleRequestT st_eti_order_req;	
  		memset(&st_eti_order_req, '\0', sizeof(st_eti_order_req));

			fn_userlog(c_ServiceName,"NEW packet");

	 fn_userlog(c_ServiceName,"c_eti_user_ssn_id :%s: ordr_type :%c: lmt_rt :%ld: stp_lss_tgr :%ld: s_ord_prtctn_rt :%d:",c_eti_user_ssn_id,ptr_st_rqst1->c_slm_flg,ptr_st_rqst1->l_ord_lmt_rt,ptr_st_rqst1->l_stp_lss_tgr,s_ord_prtctn_rt );
		MEMSET(st_eti_order_req);
		
		st_eti_order_req.MessageHeaderIn.BodyLen = htonl(sizeof(NewOrderSingleRequestT));
    st_eti_order_req.MessageHeaderIn.TemplateID = htons(TID_NEWORDERSINGLEREQUEST);
    strcpy(st_eti_order_req.MessageHeaderIn.NetworkMsgID,NO_VALUE_STR);
    strcpy(st_eti_order_req.MessageHeaderIn.Pad2,NO_VALUE_STR);

		fn_userlog(c_ServiceName," 1TemplateID:%ld",st_eti_order_req.MessageHeaderIn.TemplateID);

		st_eti_order_req.RequestHeader.MsgSeqNum = htonl(0);              /* set on ETI */
		st_eti_order_req.RequestHeader.SenderSubID = htonl(atol(c_eti_user_ssn_id));


		if(ptr_st_rqst1->c_slm_flg == LIMIT_ORDR || ptr_st_rqst1->c_slm_flg == SLTP_ORDR
				||ptr_st_rqst1->c_slm_flg == SLTP_ORDR_RQST) /* LIMIT or STOP LIMIT */
    {
       st_eti_order_req.Price = htonll((long long)(ptr_st_rqst1->l_ord_lmt_rt * 1000000.00));          /* rate */
    }
    else
    {
       st_eti_order_req.Price = NO_VALUE_SLONG;
    }

		if(ptr_st_rqst1->c_slm_flg == SLTP_ORDR || (ptr_st_rqst1->c_slm_flg == 'Q' ||ptr_st_rqst1->c_slm_flg == SLTP_ORDR_RQST))              /* STOP LIMIT or STOP MARKET */
    {
       st_eti_order_req.StopPx = htonll((long long)(ptr_st_rqst1->l_stp_lss_tgr * 1000000.00));               /* stop loss */
    }
    else
    {
       st_eti_order_req.StopPx = NO_VALUE_SLONG;
    }

		 if(DEBUG_MSG_LVL_3)
      {
				 fn_userlog(c_ServiceName,"ptr_st_opm_pipe_mstr.c_opm_pipe_id :%c: st_eti_order_req.Price :%lld: st_eti_order_req.StopPx :%lld:",ptr_st_opm_pipe_mstr->c_opm_pipe_id[1],st_eti_order_req.Price,st_eti_order_req.StopPx );
      }		

			/** commeneted in Ver 1.8 st_eti_order_req.MaxPricePercentage = htonll((long long)(s_ord_prtctn_rt * 1000000.00));           * Protection%  */
      st_eti_order_req.MaxPricePercentage = NO_VALUE_SLONG;  /** Added in Ver 1.8 **/
      st_eti_order_req.SenderLocationID = atoll(c_location_id);                     /* Location ID */
      sprintf(c_new_msgtag,"%c%ld",ptr_st_opm_pipe_mstr->c_opm_pipe_id[1],ptr_st_rqst1->l_ord_seq);
			fn_userlog(c_ServiceName," c_new_msgtag :%s:",c_new_msgtag);
      st_eti_order_req.ClOrdID = atoll(c_new_msgtag);                              /* pipe[1]||OMD sequence number*/
      st_eti_order_req.Filler1 = NO_VALUE_ULONG;                                           /* Not used */
      st_eti_order_req.Filler2 = NO_VALUE_UINT;                                           /* Not used */
      st_eti_order_req.MessageTag = htonl(atol(c_new_msgtag));                    /* pipe[1]||OMD sequence number */

			/* Order Qty */
      st_eti_order_req.OrderQty  =  htonl(abs(ptr_st_rqst1->l_ord_tot_qty));

      /* Reveal qty */
      st_eti_order_req.MaxShow = htonl(ptr_st_rqst1->l_ord_tot_qty - l_exctd_qty);

      st_eti_order_req.ExpireDate = NO_VALUE_UINT;                                        /* Not used */
      st_eti_order_req.MarketSegmentID = htonl(l_market_identifier);                      /* Product Identifier */
      st_eti_order_req.SimpleSecurityID = htonl(ptr_st_nse_cnt1->l_token_id);             /* Scrip code */
      st_eti_order_req.RegulatoryID = NO_VALUE_UINT;                                      /* Not used */
      st_eti_order_req.Filler3 = NO_VALUE_USHORT;                                         /* Not used */
      strcpy(st_eti_order_req.PartyIDTakeUpTradingFirm,NO_VALUE_STR);                     /* Not used */
      strcpy(st_eti_order_req.PartyIDOrderOriginationFirm,NO_VALUE_STR);                  /* Not used */
      strcpy(st_eti_order_req.PartyIDBeneficiary,NO_VALUE_STR);                           /* Not used */
      st_eti_order_req.AccountType = htons(CA_CLASS_CLIENT);                  						/* account type 30 */
      st_eti_order_req.ApplSeqIndicator = 1;                                  						/* 0-lean order 1 - standard order */
      st_eti_order_req.Side = ((ptr_st_ord1->c_ordr_flw == 'B') ? 1 : 2);            		  /* Buy/sell */

      fn_userlog(c_ServiceName,"***Inside NEW order OrdType :%ld: st_eti_order_req.Price :%lld: st_eti_order_req.StopPx :%lld: ",st_eti_order_req.OrdType,st_eti_order_req.Price,st_eti_order_req.StopPx);   /* added by roshni */
      fn_userlog(c_ServiceName,"***Inside NEW order st_eti_order_req.ClOrdID :%lld:",st_eti_order_req.ClOrdID);
      fn_userlog(c_ServiceName,"***Inside NEW order Template ID :%ld:",st_eti_order_req.MessageHeaderIn.TemplateID); 

			/* order type */
      if(ptr_st_rqst1->c_slm_flg == LIMIT_ORDR && l_stp_loss_trgr!=0)
      {
        st_eti_order_req.OrdType = 4;           /* stop limit */
      }
      else if(ptr_st_rqst1->c_slm_flg == LIMIT_ORDR)
      {
        st_eti_order_req.OrdType = 2;           /* limit */
      }
      else if(ptr_st_rqst1->c_slm_flg ==MARKET_ORDR_RQST  && l_stp_loss_trgr!=0)
      {
        st_eti_order_req.OrdType = 3;           /* stop market */
      }
      else if(ptr_st_rqst1->c_slm_flg == MARKET_ORDR_RQST)
      {
        st_eti_order_req.OrdType = 5;           /* Market */
      }
      else if(ptr_st_rqst1->c_slm_flg == SLTP_ORDR_RQST && ptr_st_rqst1->l_ord_lmt_rt != 0)
      {
        st_eti_order_req.OrdType = 4;           /* STOP LOSS LIMIT */
      }
      else if(ptr_st_rqst1->c_slm_flg == SLTP_ORDR_RQST && ptr_st_rqst1->l_ord_lmt_rt == 0)
      {
        st_eti_order_req.OrdType = 3;           /* STOP LOSS Market */
      }

			st_eti_order_req.PriceValidityCheckType = 0;                            /* Not used */
      st_eti_order_req.TimeInForce = htons((ptr_st_rqst1->c_ord_typ == 'I') ? 3 : 7);       /* 0 - DAY,3 IOC,7-EOS */ /* Ver 1.9 3 IOC changed to 7 GFS */
      st_eti_order_req.ExecInst = 1;                                          /* 1-persistent order */
      st_eti_order_req.STPCFlag = 1;                                          /* self trade flag 1- active */
      st_eti_order_req.Filler4 = NO_VALUE_UCHAR;                              /* Not Used */
      st_eti_order_req.TradingSessionSubID = NO_VALUE_UCHAR;                  /* not used */
      st_eti_order_req.TradingCapacity = 1;                                   /* must be sent as 1 */
      strcpy(st_eti_order_req.Account,"A1");                                  /* must be sent as A1 */
      st_eti_order_req.PositionEffect[0] = 'C';                               /* must be sent as C */
      strcpy(st_eti_order_req.PartyIDLocationID,NO_VALUE_STR);                /* Not used */
      strcpy(st_eti_order_req.CustOrderHandlingInst,NO_VALUE_STR);            /* Not used */
      strcpy(st_eti_order_req.RegulatoryText,NO_VALUE_STR);                   /* Regulatory text */
      strcpy(st_eti_order_req.AlgoID,c_algo_id);                              /* Algo id */
      strcpy(st_eti_order_req.FreeText1,(char *)ptr_st_ord1->c_cln_mtch_accnt);/* client UCC */
      strcpy(st_eti_order_req.CPCode,NO_VALUE_STR);                           /* CP code */
      strcpy(st_eti_order_req.FreeText3,ptr_st_rqst1->c_ordr_rfrnc+8);          /* Order reference */

			 /* Swap values */
      st_eti_order_req.MessageHeaderIn.BodyLen = fn_swap_ulong(st_eti_order_req.MessageHeaderIn.BodyLen);
      st_eti_order_req.MessageHeaderIn.TemplateID = fn_swap_ushort(st_eti_order_req.MessageHeaderIn.TemplateID);
      st_eti_order_req.RequestHeader.MsgSeqNum = fn_swap_ulong(st_eti_order_req.RequestHeader.MsgSeqNum);
      st_eti_order_req.RequestHeader.SenderSubID = fn_swap_ulong(st_eti_order_req.RequestHeader.SenderSubID);
      st_eti_order_req.Price = fn_swap_ulnglng(st_eti_order_req.Price);
      st_eti_order_req.StopPx = fn_swap_ulnglng(st_eti_order_req.StopPx);   
      st_eti_order_req.MaxPricePercentage = fn_swap_ulnglng(st_eti_order_req.MaxPricePercentage);
      st_eti_order_req.SenderLocationID = fn_swap_ulnglng(st_eti_order_req.SenderLocationID);
      st_eti_order_req.ClOrdID = fn_swap_ulnglng(st_eti_order_req.ClOrdID);
      st_eti_order_req.Filler1 = fn_swap_ulnglng(st_eti_order_req.Filler1);
      st_eti_order_req.Filler2 = fn_swap_ulong(st_eti_order_req.Filler2);
      st_eti_order_req.MessageTag = fn_swap_long(st_eti_order_req.MessageTag);
      st_eti_order_req.OrderQty =  fn_swap_long(st_eti_order_req.OrderQty);
      st_eti_order_req.MaxShow = fn_swap_long(st_eti_order_req.MaxShow);
      st_eti_order_req.ExpireDate = fn_swap_ulong(st_eti_order_req.ExpireDate);
      st_eti_order_req.MarketSegmentID = fn_swap_long(st_eti_order_req.MarketSegmentID);
      st_eti_order_req.SimpleSecurityID = fn_swap_ulong(st_eti_order_req.SimpleSecurityID);
      st_eti_order_req.RegulatoryID = fn_swap_ulong(st_eti_order_req.RegulatoryID);
      st_eti_order_req.Filler3 = fn_swap_ushort(st_eti_order_req.Filler3);
		fn_userlog(c_ServiceName,"msg_type:%ld  1TemplateID:%ld",ptr_st_q_packet->li_msg_type,st_eti_order_req.MessageHeaderIn.TemplateID);
    fn_userlog(c_ServiceName,"###Inside NEW order OrdType :%ld: st_eti_order_req.Price :%lld: st_eti_order_req.StopPx :%lld: ",st_eti_order_req.OrdType,st_eti_order_req.Price,st_eti_order_req.StopPx);   /* added by roshni */
    fn_userlog(c_ServiceName,"***Inside NEW order st_eti_order_req.ClOrdID :%lld:",st_eti_order_req.ClOrdID);
      fn_userlog(c_ServiceName,"***Inside NEW order Template ID :%ld:",st_eti_order_req.MessageHeaderIn.TemplateID); 
   	memcpy(&(ptr_st_q_packet->st_bsnd_rqst_data),&st_eti_order_req,sizeof(st_eti_order_req));
		}
		else if(ptr_st_rqst1->c_req_typ == MODIFY)
		{
			ModifyOrderSingleRequestT st_eti_order_req;	
  		memset(&st_eti_order_req, '\0', sizeof(st_eti_order_req));

      fn_userlog(c_ServiceName,"inside MODIFY......");
      MEMSET(st_eti_order_req);

      st_eti_order_req.MessageHeaderIn.BodyLen = htonl(sizeof(ModifyOrderSingleRequestT));
      st_eti_order_req.MessageHeaderIn.TemplateID = htons(TID_MODIFY_ORDER_SINGLE_REQUEST);
      strcpy(st_eti_order_req.MessageHeaderIn.NetworkMsgID,NO_VALUE_STR);
      strcpy(st_eti_order_req.MessageHeaderIn.Pad2,NO_VALUE_STR);

      st_eti_order_req.RequestHeader.MsgSeqNum = htonl(0);          /* set on ETI */
      st_eti_order_req.RequestHeader.SenderSubID = htonl(atol(c_eti_user_ssn_id));


      st_eti_order_req.OrderID = atoll(ptr_st_ord1->c_xchng_ack);    /* Exchnage ack */
      sprintf(c_new_msgtag,"%c%ld",ptr_st_opm_pipe_mstr->c_opm_pipe_id[1],ptr_st_rqst1->l_ord_seq);
      st_eti_order_req.ClOrdID = atoll(c_new_msgtag);               /* pipeid[1]||OMD sequence number */
      st_eti_order_req.OrigClOrdID = NO_VALUE_ULONG;                /* last sucessfully processed OMD sequence number */

      if(ptr_st_rqst1->c_slm_flg == LIMIT_ORDR || ptr_st_rqst1->c_slm_flg == SLTP_ORDR || ptr_st_rqst1->c_slm_flg == SLTP_ORDR_RQST)       /* LIMIT or STOP LIMIT */
      {
        st_eti_order_req.Price = htonll((long long)(ptr_st_rqst1->l_ord_lmt_rt * 1000000.00));                       /* rate */
      }
      else
      {
        st_eti_order_req.Price = NO_VALUE_SLONG;
      }

      if(ptr_st_rqst1->c_slm_flg == SLTP_ORDR || ptr_st_rqst1->c_slm_flg == 'Q' || ptr_st_rqst1->c_slm_flg == SLTP_ORDR_RQST)  /* STOP LIMIT or STOP MARKET */
      {
        st_eti_order_req.StopPx = htonll((long long)(ptr_st_rqst1->l_stp_lss_tgr * 1000000.00));       /* stop loss */
      }
			else
      {
        st_eti_order_req.StopPx = NO_VALUE_SLONG;
      }
      /** commented in Ver 1.8 st_eti_order_req.MaxPricePercentage = htonll((long long)(s_ord_prtctn_rt * 1000000.00));        * Protection%  */
      st_eti_order_req.MaxPricePercentage = NO_VALUE_SLONG;   /** Added in Ver 1.8 **/
      st_eti_order_req.SenderLocationID = atoll(c_location_id);           /* Location ID */
      st_eti_order_req.ActivityTime = htonll(atoll(v_ack_number.arr));        /* Last activity reference */
      st_eti_order_req.Filler1 = NO_VALUE_ULONG ;                   /* Not used */
      st_eti_order_req.Filler2 = NO_VALUE_UINT;                     /* Not Used */
      st_eti_order_req.MessageTag = htonl(atol(c_new_msgtag));      /* pipeid[1]||OMD sequence number */

      /* Order Qty */
      st_eti_order_req.OrderQty            =  htonl(abs(ptr_st_rqst1->l_ord_tot_qty));
			/* Reveal qty */
			st_eti_order_req.MaxShow = htonl(ptr_st_rqst1->l_ord_tot_qty - l_exctd_qty);
			fn_userlog(c_ServiceName,"Qty to be modified:%ld: Open:%ld: disclosed:%ld:",st_eti_order_req.OrderQty,ptr_st_rqst1->l_ord_tot_qty, ptr_st_rqst1->l_ord_tot_qty - l_exctd_qty);
			

      st_eti_order_req.ExpireDate = NO_VALUE_UINT;
      st_eti_order_req.MarketSegmentID = htonl(l_market_identifier);            /* Product Identifier */
      st_eti_order_req.SimpleSecurityID = htonl(ptr_st_nse_cnt1->l_token_id);   /* Scrip code */
      st_eti_order_req.TargetPartyIDSessionID = NO_VALUE_UINT;                  /* partyidSession id set on ETI */
      st_eti_order_req.RegulatoryID = NO_VALUE_UINT;                            /* Not used */
      strcpy(st_eti_order_req.PartyIDTakeUpTradingFirm,NO_VALUE_STR);           /* Not used */
      strcpy(st_eti_order_req.PartyIDOrderOriginationFirm,NO_VALUE_STR);        /* Not used */
      strcpy(st_eti_order_req.PartyIDBeneficiary,NO_VALUE_STR);                 /* Not used */
      st_eti_order_req.AccountType = htons(CA_CLASS_CLIENT);                    /* account type 30 */
      st_eti_order_req.ApplSeqIndicator = 1;                                    /* 0 - lean order 1 - standard order */
      st_eti_order_req.Side = ((ptr_st_ord1->c_ordr_flw == 'B') ? 1 : 2);       /* Buy/sell */
  
      fn_userlog(c_ServiceName,"***Inside MODIFY order OrdType :%ld: st_eti_order_req.Price :%lld: st_eti_order_req.StopPx :%lld: ",st_eti_order_req.OrdType,st_eti_order_req.Price,st_eti_order_req.StopPx);   /* added by roshni */

      /* order type */
			if(ptr_st_rqst1->c_slm_flg == LIMIT_ORDR && l_stp_loss_trgr!=0)
      {
        st_eti_order_req.OrdType = 4;           /* stop limit */
      }
      else if(ptr_st_rqst1->c_slm_flg == LIMIT_ORDR)
      {
        st_eti_order_req.OrdType = 2;           /* limit */
      }
      else if(ptr_st_rqst1->c_slm_flg ==MARKET_ORDR_RQST  && l_stp_loss_trgr!=0)
      {
        st_eti_order_req.OrdType = 3;           /* stop market */
      }
      else if(ptr_st_rqst1->c_slm_flg == MARKET_ORDR_RQST)
      {
        st_eti_order_req.OrdType = 5;           /* Market */
      }
			else if(ptr_st_rqst1->c_slm_flg == SLTP_ORDR_RQST && ptr_st_rqst1->l_ord_lmt_rt != 0)
      {
        st_eti_order_req.OrdType = 4;           /* STOP LOSS LIMIT */
      }
      else if(ptr_st_rqst1->c_slm_flg == SLTP_ORDR_RQST && ptr_st_rqst1->l_ord_lmt_rt == 0)
      {
        st_eti_order_req.OrdType = 3;           /* STOP LOSS Market */
      }

			st_eti_order_req.PriceValidityCheckType = 0;                            /* Not used */
      st_eti_order_req.TimeInForce = htons((ptr_st_rqst1->c_ord_typ == 'I') ? 3 : 7);       /* 0 - DAY,3 IOC ,7-EOS*/ /* Ver 1.9 3 IOC changed to 7 GFS */
      st_eti_order_req.ExecInst = 1;                                          /* 1-persistent order */
      st_eti_order_req.Filler4 = NO_VALUE_UCHAR;                              /* Not used */
      st_eti_order_req.TradingSessionSubID = NO_VALUE_UCHAR;                  /* not used */
      st_eti_order_req.TradingCapacity = 1;                                   /* must be sent as 1 */
      st_eti_order_req.DeltaQtyFlag[0] = NO_VALUE_STR;                            /* Not used */
      strcpy(st_eti_order_req.Account,"A1");                                  /* must be sent as A1 */
      st_eti_order_req.PositionEffect[0] = 'C';                               /* must be sent as C */
      strcpy(st_eti_order_req.PartyIDLocationID,NO_VALUE_STR);                /* Not used */
      strcpy(st_eti_order_req.CustOrderHandlingInst,NO_VALUE_STR);            /* Not used */
      strcpy(st_eti_order_req.RegulatoryText,NO_VALUE_STR);                   /* Regulatory text */
      strcpy(st_eti_order_req.AlgoID,c_algo_id);                              /* Algo id */
      strcpy(st_eti_order_req.FreeText1,(char *)ptr_st_ord1->c_cln_mtch_accnt);        /* client UCC */
      strcpy(st_eti_order_req.CPCode,NO_VALUE_STR);                           /* CP code */
      strcpy(st_eti_order_req.FreeText3,ptr_st_rqst1->c_ordr_rfrnc + 8);          /* Order reference */
      strcpy(st_eti_order_req.Pad3,NO_VALUE_STR);                             /* Not used */
 
			/* Swap values */
      st_eti_order_req.MessageHeaderIn.BodyLen = fn_swap_ulong(st_eti_order_req.MessageHeaderIn.BodyLen);
      st_eti_order_req.MessageHeaderIn.TemplateID = fn_swap_ushort(st_eti_order_req.MessageHeaderIn.TemplateID);
      st_eti_order_req.RequestHeader.MsgSeqNum = fn_swap_ulong(st_eti_order_req.RequestHeader.MsgSeqNum);
      st_eti_order_req.RequestHeader.SenderSubID = fn_swap_ulong(st_eti_order_req.RequestHeader.SenderSubID);
      st_eti_order_req.OrderID = fn_swap_ulnglng(st_eti_order_req.OrderID);
      st_eti_order_req.ClOrdID = fn_swap_ulnglng(st_eti_order_req.ClOrdID);
      st_eti_order_req.OrigClOrdID = fn_swap_ulnglng(st_eti_order_req.OrigClOrdID);
      st_eti_order_req.Price = fn_swap_ulnglng(st_eti_order_req.Price);
      st_eti_order_req.StopPx = fn_swap_ulnglng(st_eti_order_req.StopPx);
      st_eti_order_req.MaxPricePercentage = fn_swap_ulnglng(st_eti_order_req.MaxPricePercentage);
      st_eti_order_req.SenderLocationID = fn_swap_ulnglng(st_eti_order_req.SenderLocationID);
      st_eti_order_req.ActivityTime = fn_swap_ulnglng(st_eti_order_req.ActivityTime);
      st_eti_order_req.Filler1 = fn_swap_ulnglng(st_eti_order_req.Filler1);
      st_eti_order_req.Filler2 = fn_swap_ulong(st_eti_order_req.Filler2);
      st_eti_order_req.MessageTag = fn_swap_long(st_eti_order_req.MessageTag);
      st_eti_order_req.OrderQty =  fn_swap_long(st_eti_order_req.OrderQty);
      st_eti_order_req.MaxShow = fn_swap_long(st_eti_order_req.MaxShow);
      st_eti_order_req.ExpireDate = fn_swap_ulong(st_eti_order_req.ExpireDate);
      st_eti_order_req.MarketSegmentID = fn_swap_long(st_eti_order_req.MarketSegmentID);
      st_eti_order_req.SimpleSecurityID = fn_swap_ulong(st_eti_order_req.SimpleSecurityID);
      st_eti_order_req.TargetPartyIDSessionID = fn_swap_ulong(st_eti_order_req.TargetPartyIDSessionID);
      st_eti_order_req.RegulatoryID = fn_swap_ulong(st_eti_order_req.RegulatoryID);
      st_eti_order_req.Filler3 = fn_swap_ushort(st_eti_order_req.Filler3);
		fn_userlog(c_ServiceName,"msg_type:%ld  1TemplateID:%ld",ptr_st_q_packet->li_msg_type,st_eti_order_req.MessageHeaderIn.TemplateID);
    fn_userlog(c_ServiceName,"###Inside MODIFY order OrdType :%ld: st_eti_order_req.Price :%lld: st_eti_order_req.StopPx :%lld: ",st_eti_order_req.OrdType,st_eti_order_req.Price,st_eti_order_req.StopPx);   /* added by roshni */
   	memcpy(&(ptr_st_q_packet->st_bsnd_rqst_data),&st_eti_order_req,sizeof(st_eti_order_req));
		}
		else if(ptr_st_rqst1->c_req_typ == CANCEL)
		{
			DeleteOrderSingleRequestT st_eti_order_req;
  		memset(&st_eti_order_req, '\0', sizeof(st_eti_order_req));

			fn_userlog(c_ServiceName,"inside CANCEL......");
      MEMSET(st_eti_order_req);

      st_eti_order_req.MessageHeaderIn.BodyLen = htonl(sizeof(DeleteOrderSingleRequestT));
      st_eti_order_req.MessageHeaderIn.TemplateID = htons(TID_DELETE_ORDER_SINGLE_REQUEST);
      strcpy(st_eti_order_req.MessageHeaderIn.NetworkMsgID,NO_VALUE_STR);
      strcpy(st_eti_order_req.MessageHeaderIn.Pad2,NO_VALUE_STR);

      st_eti_order_req.RequestHeader.MsgSeqNum = htonl(0);          /* set on ETI */
      st_eti_order_req.RequestHeader.SenderSubID = htonl(atol(c_eti_user_ssn_id));

      st_eti_order_req.OrderID = atoll(ptr_st_ord1->c_xchng_ack);             /* Exchnage ack */
      sprintf(c_new_msgtag,"%c%ld",ptr_st_opm_pipe_mstr->c_opm_pipe_id[1],ptr_st_rqst1->l_ord_seq);
      st_eti_order_req.ClOrdID = atoll(c_new_msgtag);               /* pipeid[1]||OMD sequence number */
      st_eti_order_req.OrigClOrdID = NO_VALUE_ULONG;                /* last sucessfully processed OMD sequence number */
      st_eti_order_req.ActivityTime = htonll(atoll( v_ack_number.arr) );
      st_eti_order_req.MessageTag = htonl(atol(c_new_msgtag));      /* pipeid[1]||OMD sequence number */
      st_eti_order_req.MarketSegmentID = htonl(l_market_identifier);            /* Product Identifier */
			st_eti_order_req.SimpleSecurityID = htonl(ptr_st_nse_cnt1->l_token_id);                    /* Scrip code */
      st_eti_order_req.TargetPartyIDSessionID = NO_VALUE_UINT;                  /* partyidSession id set on ETI */
      st_eti_order_req.RegulatoryID = NO_VALUE_UINT;                            /* Not used */
      strcpy(st_eti_order_req.AlgoID,c_algo_id);                              /* Algo id */
      strcpy(st_eti_order_req.Pad4,NO_VALUE_STR);                              /* not Used */

      /* Swap values */
      st_eti_order_req.MessageHeaderIn.BodyLen = fn_swap_ulong(st_eti_order_req.MessageHeaderIn.BodyLen);
      st_eti_order_req.MessageHeaderIn.TemplateID = fn_swap_ushort(st_eti_order_req.MessageHeaderIn.TemplateID);
      st_eti_order_req.RequestHeader.MsgSeqNum = fn_swap_ulong(st_eti_order_req.RequestHeader.MsgSeqNum);
      st_eti_order_req.RequestHeader.SenderSubID = fn_swap_ulong(st_eti_order_req.RequestHeader.SenderSubID);
      st_eti_order_req.OrderID = fn_swap_ulnglng(st_eti_order_req.OrderID);
      st_eti_order_req.ClOrdID = fn_swap_ulnglng(st_eti_order_req.ClOrdID);
      st_eti_order_req.OrigClOrdID = fn_swap_ulnglng(st_eti_order_req.OrigClOrdID);
      st_eti_order_req.ActivityTime = fn_swap_ulnglng(st_eti_order_req.ActivityTime);
      st_eti_order_req.MessageTag = fn_swap_long(st_eti_order_req.MessageTag);
      st_eti_order_req.MarketSegmentID = fn_swap_long(st_eti_order_req.MarketSegmentID);
      st_eti_order_req.SimpleSecurityID = fn_swap_ulong(st_eti_order_req.SimpleSecurityID);
      st_eti_order_req.TargetPartyIDSessionID = fn_swap_ulong(st_eti_order_req.TargetPartyIDSessionID);
      st_eti_order_req.RegulatoryID = fn_swap_ulong(st_eti_order_req.RegulatoryID);
		fn_userlog(c_ServiceName,"msg_type:%ld  1TemplateID:%ld",ptr_st_q_packet->li_msg_type,st_eti_order_req.MessageHeaderIn.TemplateID);
   	memcpy(&(ptr_st_q_packet->st_bsnd_rqst_data),&st_eti_order_req,sizeof(st_eti_order_req));
		}
    /* Ver 1.6 Ends */  
   /* commented in Ver 1.6 memcpy(&(ptr_st_q_packet->st_bsnd_rqst_data),&st_ord_snd_rqst,sizeof(struct st_ord_rqst)); */
 }
  return 0;
}
