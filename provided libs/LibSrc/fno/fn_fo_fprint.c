/******************************************************************************/
/*  Program       : fn_fo_fprint.c                                            */
/*                                                                            */
/*  Description   : This program writes the input packet to a given file.     */
/*                                                                            */
/*  Input         : <File pointer> <message type> <message data>              */
/*                                                                            */
/*  Output        : Success/Failure                                           */
/*                                                                            */
/*  Log           :                                                           */
/*                                                                            */
/*  Ver 1.0   24-Jun-2009   Prakash       Initial Release CRCSN31735 ORS2     */
/*	Ver	1.1		04-Oct-2010		Sandeep				Data Type Changed For User ID				*/
/*  Ver 1.2   06-Feb-2012   Sandip 				2L OEREMARK Changes									*/
/*	Ver 1.3 	07-May-2018		Parag				  NNF_CHANGES_7.21(PAN CHANGES)				*/
/*  Ver 1.4   13-Mar-2019   Kiran         Trim Protocol implementation changes*/
/******************************************************************************/

#include <stdio.h>
#include <time.h>
#include <memory.h>
#include <fn_fo_fprint.h>
#include <fo_exch_comnN.h>
#include <fn_log.h>
#include <fn_host_to_nw_format_nse.h>
#include <fn_nw_to_host_format_nse.h>

#define LEN_CHECKSUM     16
#define MAX_SCK_MSG      1024


/***********************************************************************************/
/* This function writes a blank padded string to file terminating it with new line */
/* INPUT PARAMETERS                                                                */
/*    fptr      - File handler to which data is to be written                      */
/*    c_const   - Field description                                                */
/*    c_ptr     - Blank padded string                                              */
/*    i_len     - Actual data length in the blank padded string                    */
/* OUTPUT PARAMETERS                                                               */
/*          None                                                                   */
/***********************************************************************************/
void fn_fprint_string(FILE *fptr,
                      char *c_const,
                      char *c_ptr,
                      int  i_len)
{
  int i_loop;

  fprintf(fptr, "%s:", c_const);
  for(i_loop = 0; i_loop < i_len; i_loop++)
  {
    fprintf(fptr, "%c", c_ptr[i_loop]);
  }
  fprintf(fptr, ":\n");
}



/******************************************************************************/
/* This function takes char buffer as input and depending on the message type */
/* maps them to respective structures and writes the data to the file pointed */
/* to by the file pointer.                                                    */
/* INPUT PARAMETERS                                                           */
/*      fptr         - File handler                                           */
/*      li_msg_typ   - Message Type                                           */
/*      c_msg_data   - Structure to be printed                                */
/*      c_ServiceName- Name of the service                                    */
/*      c_errmsg     - Error String                                           */
/* OUTPUT PARAMETERS                                                          */
/*          0 - Success                                                       */
/******************************************************************************/
int fn_fprint_log(FILE *fptr,
                  char c_tap_header,
                  void *c_msg_data,
                  char *c_ServiceName,
                  char *c_errmsg)
{
  char      c_dname[22];
  char      c_start[20];
  char      c_end[15];
  time_t    tm_stmp;
  short int si_err_code;
  int       i_loop = 0;
  long int  li_msg_typ;
  long long ll_timestamp1;
  long long ll_timestamp2;
  long long ll_jiffy;
  struct st_int_header st_ihdr;
  struct st_net_hdr st_nhdr;

  tm_stmp = time(NULL);
  strftime(c_dname, 21, "%d-%b-%Y %H:%M:%S", localtime(&tm_stmp));
  fprintf(fptr,"Timestamp :%s:\n", c_dname);
	
  strcpy(c_start, "Printing New packet");
  strcpy(c_end, "Packet Printed");

  fprintf(fptr, "%s\n", c_start);

  if ( c_tap_header == 'Y' )
  {
    /* Printing net header of packet format*/
		
    memset(&st_nhdr, 0, sizeof(st_nhdr));
    memcpy(&st_nhdr , c_msg_data, sizeof(st_nhdr));
    c_msg_data += sizeof(struct st_net_hdr);

    fprintf(fptr, "%-35s=:%d:\n", "Message length", st_nhdr.si_message_length);
    fprintf(fptr, "%-35s=:%d:\n", "Sequence number", st_nhdr.i_seq_num);
    fn_fprint_string(fptr,"Checksum                           =",st_nhdr.c_checksum,LEN_CHECKSUM);
    fprintf(fptr, "Checksum in hex                    =");
    for( i_loop = 0 ; i_loop < LEN_CHECKSUM ; i_loop ++ )
    {
      fprintf(fptr, "%02X\t", st_nhdr.c_checksum[i_loop]);
    }
    fprintf(fptr, "\n");
  }

	/* Printing interactive header */

	memset(&st_ihdr, 0, sizeof(st_ihdr));
  memcpy(&st_ihdr, c_msg_data, sizeof(st_ihdr));

	fprintf(fptr, "%-35s=:%d:\n", "Transaction Code", st_ihdr.si_transaction_code);
	li_msg_typ = st_ihdr.si_transaction_code;

	if((li_msg_typ !=  BOARD_LOT_IN_TR) && (li_msg_typ != ORDER_MOD_IN_TR) && (li_msg_typ != ORDER_CANCEL_IN_TR) && (li_msg_typ != ORDER_CONFIRMATION_OUT_TR) && (li_msg_typ != ORDER_MOD_CONFIRM_OUT_TR) && (li_msg_typ != ORDER_CXL_CONFIRMATION_TR) && (li_msg_typ != TRADE_CONFIRMATION_TR ))	/* Ver 1.4  */ 
	{ 
 		 fprintf(fptr, "%-35s=:%ld:\n", "Log Time", st_ihdr.li_log_time);
 		 fn_fprint_string(fptr,"Alpha char                         =",
                    st_ihdr.c_alpha_char,LEN_ALPHA_CHAR);
 		 fprintf(fptr, "%-35s=:%ld:\n", "Trader id", st_ihdr.li_trader_id);
 		 fprintf(fptr, "%-35s=:%d:\n", "Error Code", st_ihdr.si_error_code);
			fn_fprint_string(fptr,"c_filler_2                         =",
                    st_ihdr.c_filler_2,8);
 		 memcpy(&ll_timestamp1, st_ihdr.c_time_stamp_1, LEN_TIME_STAMP);
 		 fprintf(fptr, "%-35s=:%lld:\n", "Timestamp1", ll_timestamp1);
 		 memcpy(&ll_timestamp2, st_ihdr.c_time_stamp_2, LEN_TIME_STAMP);
 		 fprintf(fptr, "%-35s=:%lld:\n", "Timestamp2", ll_timestamp2);
 		 fprintf(fptr, "%-35s=:%d:\n", "Message Length", st_ihdr.si_message_length);

			si_err_code = st_ihdr.si_error_code;
 }

	switch(li_msg_typ)
  {
    case INVITATION_PACKET:                 /* 15000 */
    {
			struct st_invt_pckt st_inv_pack;
      memset(&st_inv_pack, 0, sizeof(st_inv_pack));
      memcpy(&st_inv_pack, (struct st_invt_pckt *)c_msg_data, sizeof(st_inv_pack));
			fn_cnvt_nth_invt_pckt(&st_inv_pack);													/* Ver 1.4  */

      fprintf(fptr,"%-35s=:%d:\n", "Invitation count", st_inv_pack.si_inv_cnt);
      fprintf(fptr, "\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

		/* The logon process */
		case SIGN_ON_REQUEST_IN:                  /*2300*/
    {
      struct st_sign_on_req st_login_req;
      memset(&st_login_req, 0, sizeof(st_login_req));
      memcpy(&st_login_req, (struct st_sign_on_req *)c_msg_data, sizeof(st_login_req));
			fn_cnvt_htn_sign_on_req_in(&st_login_req);							/* Ver 1.4  */

			/***	Commented In Ver 1.1
      fprintf(fptr,"%-35s=:%d:\n", "User Id", st_login_req.si_user_id);
			***************************************/
			fprintf(fptr,"%-35s=:%ld:\n", "User Id", st_login_req.li_user_id);			/***	Ver	1.1	***/

			fn_fprint_string(fptr,"Reserved1													=",
												st_login_req.c_reserved_1,8);		/*** Ver 1.3 ***/
      fn_fprint_string(fptr,"Password                           =",
                        st_login_req.c_password,LEN_PASSWORD);
			fn_fprint_string(fptr,"Reserved2													=",
												st_login_req.c_reserved_2,8);   /*** Ver 1.3 ***/
      fn_fprint_string(fptr,"New Password                       =",
                        st_login_req.c_new_password, LEN_PASSWORD);
      fn_fprint_string(fptr,"Trader Name                        =",
                        st_login_req.c_trader_name,LEN_TRADER_NAME);

      fprintf(fptr,"%-35s=:%ld:\n", "Last Passwd Chng Date",
              st_login_req.li_last_password_change_date);

      fn_fprint_string(fptr,"Broker Id                          =",
                        st_login_req.c_broker_id,LEN_BROKER_ID);
      fprintf(fptr,"%-35s=:%c:\n", "Filler1", st_login_req.c_filler_1);
			
      fprintf(fptr,"%-35s=:%d:\n", "Branch Id", st_login_req.si_branch_id);
      fprintf(fptr,"%-35s=:%ld:\n", "Version No", st_login_req.li_version_number);

      fprintf(fptr,"%-35s=:%ld:\n", "Batch 2 start time", st_login_req.li_batch_2_start_time);
      fn_fprint_string(fptr,"COLOUR                             =",
                        st_login_req.c_colour, LEN_COLOUR);
      fprintf(fptr,"%-35s=:%c:\n", "Filler2", st_login_req.c_filler_2);

      fprintf(fptr, "%-35s=:%d:\n", "User Type", st_login_req.si_user_type);

      fprintf(fptr, "%-35s=:%lf:\n", "Sequence No", st_login_req.d_sequence_number);
      fn_fprint_string(fptr,"Workstation Number                 =",
                        st_login_req.c_ws_class_name,LEN_WS_CLASS_NAME);
			fprintf(fptr, "%-35s=:%c:\n", "Broker Status", st_login_req.c_broker_status);
      fprintf(fptr, "%-35s=:%c:\n", "Show Index", st_login_req.c_show_index);
      fprintf(fptr, "%-35s=:%d:\n", "Eligibility Normal Mkt",
                    st_login_req.st_mkt_allwd_lst.flg_normal_mkt);
      fprintf(fptr, "%-35s=:%d:\n", "Eligibility Oddlot Mkt",
                    st_login_req.st_mkt_allwd_lst.flg_oddlot_mkt);
      fprintf(fptr, "%-35s=:%d:\n", "Eligibility Spot Mkt",
                    st_login_req.st_mkt_allwd_lst.flg_spot_mkt);
      fprintf(fptr, "%-35s=:%d:\n", "Eligibility Auction Mkt",
                    st_login_req.st_mkt_allwd_lst.flg_auction_mkt);
      fprintf(fptr, "%-35s=:%d:\n", "Eligibility Filler1",
                    st_login_req.st_mkt_allwd_lst.flg_filler1);
      fprintf(fptr, "%-35s=:%d:\n", "Eligibility Filler2",
                    st_login_req.st_mkt_allwd_lst.flg_filler2);
      fn_fprint_string(fptr,"Broker Name                        =",
                        st_login_req.c_broker_name,LEN_BROKER_NAME);
			/*** Ver 1.3 Starts ***/
			fn_fprint_string(fptr,"Reserved3													=",
												st_login_req.c_reserved_3,16);
			fn_fprint_string(fptr,"Reserved4													=",
												st_login_req.c_reserved_4,16);
			fn_fprint_string(fptr,"Reserved5													=",
												st_login_req.c_reserved_5,16);
			/*** Ver 1.3 Ends ***/
      fprintf(fptr, "\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

		case SIGN_ON_REQUEST_OUT:               /*2301*/
    {
      if(si_err_code == 0)
      {
        struct st_sign_on_res st_login_req_out;
        memset(&st_login_req_out, 0, sizeof(st_login_req_out));
        memcpy(&st_login_req_out, (struct st_signon_res *)c_msg_data,
               sizeof(st_login_req_out));
				
				/***	Commented In Ver 1.1
        fprintf(fptr,"%-35s=:%d:\n", "User Id", st_login_req_out.si_user_id);
				**************************************/

				fn_cnvt_nth_sign_on_res(&st_login_req_out);
				fprintf(fptr,"%-35s=:%ld:\n", "User Id", st_login_req_out.li_user_id);			/***	Ver	1.1	***/
				fn_fprint_string(fptr,"Reserved1                          =",
                        st_login_req_out.c_reserved_1,8);   /*** Ver 1.3 ***/	
        fn_fprint_string(fptr,"Password                           =",
                          st_login_req_out.c_password,LEN_PASSWORD);
				fn_fprint_string(fptr,"Reserved2                          =",
                        st_login_req_out.c_reserved_2,8);   /*** Ver 1.3 ***/	
        fn_fprint_string(fptr,"New Password                       =",
                          st_login_req_out.c_new_password,LEN_PASSWORD);
        fn_fprint_string(fptr,"Trader Name                        =",
                          st_login_req_out.c_trader_name, LEN_TRADER_NAME);
				
        fprintf(fptr,"%-35s=:%ld:\n", "Last psswd chng date",
                st_login_req_out.li_last_password_change_date);
        fn_fprint_string(fptr,"Broker Id                          =",
                          st_login_req_out.c_broker_id,LEN_BROKER_ID);
        fprintf(fptr,"%-35s=:%c:\n", "Filler1", st_login_req_out.c_filler_1);
        fprintf(fptr,"%-35s=:%d:\n", "Branch Id", st_login_req_out.si_branch_id);
        fprintf(fptr,"%-35s=:%ld:\n", "Version No", st_login_req_out.li_version_number);
        fprintf(fptr,"%-35s=:%ld:\n", "End Time", st_login_req_out.li_end_time);
        fn_fprint_string(fptr,"Filler2                            =",
                          st_login_req_out.c_filler_2,52);
        fprintf(fptr,"%-35s=:%d:\n", "User Type", st_login_req_out.si_user_type);
        fprintf(fptr,"%-35s=:%lf:\n", "Sequence No", st_login_req_out.d_sequence_number);
        fn_fprint_string(fptr,"Filler3                            =",
                          st_login_req_out.c_filler_3,14);
        fprintf(fptr,"%-35s=:%c:\n", "Broker Status", st_login_req_out.c_broker_status);
        fprintf(fptr,"%-35s=:%c:\n", "Show Index", st_login_req_out.c_show_index);
        fprintf(fptr,"%-35s=:%d:\n", "Eligibility Normal Mkt",
                st_login_req_out.st_mkt_allwd_lst.flg_normal_mkt);
        fprintf(fptr,"%-35s=:%d:\n", "Eligibility Oddlot Mkt",
                st_login_req_out.st_mkt_allwd_lst.flg_oddlot_mkt);
        fprintf(fptr,"%-35s=:%d:\n", "Eligibility Spot Mkt",
                st_login_req_out.st_mkt_allwd_lst.flg_spot_mkt);
        fprintf(fptr,"%-35s=:%d:\n", "Eligibility Auction Mkt",
                st_login_req_out.st_mkt_allwd_lst.flg_auction_mkt);
        fprintf(fptr,"%-35s=:%d:\n", "Eligibility Filler1",
                st_login_req_out.st_mkt_allwd_lst.flg_filler1);
        fprintf(fptr,"%-35s=:%d:\n", "Eligibility Filler2",
                st_login_req_out.st_mkt_allwd_lst.flg_filler2);
        fn_fprint_string(fptr,"Broker Name                        =",
                        st_login_req_out.c_broker_name,LEN_BROKER_NAME);
				/*** Ver 1.3 Starts ***/
      	fn_fprint_string(fptr,"Reserved3                          =",
                        st_login_req_out.c_reserved_3,16);
      	fn_fprint_string(fptr,"Reserved4                          =",
                        st_login_req_out.c_reserved_4,16);
      	fn_fprint_string(fptr,"Reserved5                          =",
                        st_login_req_out.c_reserved_5,16);
      /*** Ver 1.3 Ends ***/
      }
      else
      {
        struct st_error_response st_err_res;
        memset(&st_err_res, 0, sizeof(st_err_res));
        memcpy(&st_err_res, (struct st_error_response *)c_msg_data, sizeof(st_err_res));

        fn_fprint_string(fptr,"Key                                =",
                          st_err_res.c_key,LEN_ERROR_KEY);
        fn_fprint_string(fptr,"Error message                      =",
                          st_err_res.c_error_message, LEN_ERROR_MESSAGE);
      }
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

		case SIGN_OFF_REQUEST_IN:                       /*2320*/
    {
      /*only header section is there*/
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

		case SIGN_OFF_REQUEST_OUT:                       /*2321 not in EQ tap newly added here*/
    {
      struct st_sign_off_res st_signoff_resp;
      memset(&st_signoff_resp, 0, sizeof(st_signoff_resp));
      memcpy(&st_signoff_resp, (struct st_sign_off_res *)c_msg_data, sizeof(st_signoff_resp));

      fn_fprint_string(fptr,"Fileer1                              =",
                          st_signoff_resp.c_filler_1,145);
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

		case SYSTEM_INFORMATION_IN:                     /*1600*/
		{
			struct st_system_info_req st_sys_info_req ;
      memset(&st_sys_info_req, 0, sizeof(st_sys_info_req));
      memcpy(&st_sys_info_req, (struct st_system_info_req *)c_msg_data, sizeof(st_sys_info_req));
			fn_cnvt_htn_sys_info_req(&st_sys_info_req); 						/* Ver 1.4  */
	
      fprintf(fptr,"%-35s=:%ld:\n", "Last update protfolio time", st_sys_info_req.li_last_update_protfolio_time);
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

		case SYSTEM_INFORMATION_OUT:                    /*1601*/
    case PARTIAL_SYSTEM_INFORMATION:                /*7321*/
		{
      struct st_system_info_data st_sysinfo_out;
      memset(&st_sysinfo_out, 0, sizeof(st_sysinfo_out));
      memcpy(&st_sysinfo_out,(struct st_system_info_data *)c_msg_data,
             sizeof(st_sysinfo_out));
			fn_cnvt_nth_system_info_data(&st_sysinfo_out);				/* Ver 1.4 */

      fprintf(fptr,"%-35s=:%d:\n", "Mkt status Normal", st_sysinfo_out.st_mkt_stts.si_normal);
      fprintf(fptr,"%-35s=:%d:\n", "Mkt status Oddlot", st_sysinfo_out.st_mkt_stts.si_oddlot);
      fprintf(fptr,"%-35s=:%d:\n", "Mkt status Spot", st_sysinfo_out.st_mkt_stts.si_spot);
      fprintf(fptr,"%-35s=:%d:\n", "Mkt status Auction", st_sysinfo_out.st_mkt_stts.si_auction);

      fprintf(fptr,"%-35s=:%d:\n", "Ex Mkt status Normal", st_sysinfo_out.st_ex_mkt_stts.si_normal);
      fprintf(fptr,"%-35s=:%d:\n", "Ex Mkt status Oddlot", st_sysinfo_out.st_ex_mkt_stts.si_oddlot);
      fprintf(fptr,"%-35s=:%d:\n", "Ex Mkt status Spot", st_sysinfo_out.st_ex_mkt_stts.si_spot);
      fprintf(fptr,"%-35s=:%d:\n", "Ex Mkt status Auction", st_sysinfo_out.st_ex_mkt_stts.si_auction);

      fprintf(fptr,"%-35s=:%d:\n", "Pl Mkt status Normal", st_sysinfo_out.st_pl_mkt_stts.si_normal);
      fprintf(fptr,"%-35s=:%d:\n", "Pl Mkt status Oddlot", st_sysinfo_out.st_pl_mkt_stts.si_oddlot);
      fprintf(fptr,"%-35s=:%d:\n", "Pl Mkt status Spot", st_sysinfo_out.st_pl_mkt_stts.si_spot);
      fprintf(fptr,"%-35s=:%d:\n", "Pl Mkt status Auction", st_sysinfo_out.st_pl_mkt_stts.si_auction);

      fprintf(fptr,"%-35s=:%c:\n", "Update Portfolio", st_sysinfo_out.c_update_portfolio);
      fprintf(fptr,"%-35s=:%ld:\n", "Market index", st_sysinfo_out.li_market_index);
      fprintf(fptr,"%-35s=:%d:\n", "Def stlmt period nm",
              st_sysinfo_out.si_default_sttlmnt_period_nm);
      fprintf(fptr,"%-35s=:%d:\n", "Def stlmt period sp",
              st_sysinfo_out.si_default_sttlmnt_period_sp);
      fprintf(fptr,"%-35s=:%d:\n", "Def stlmt period au",
              st_sysinfo_out.si_default_sttlmnt_period_au);
      fprintf(fptr,"%-35s=:%d:\n", "Competitor Period", st_sysinfo_out.si_competitor_period);
      fprintf(fptr,"%-35s=:%d:\n", "Solicitor Period", st_sysinfo_out.si_solicitor_period);
      fprintf(fptr,"%-35s=:%d:\n", "Warning Percent", st_sysinfo_out.si_warning_percent);
      fprintf(fptr,"%-35s=:%d:\n", "Vol freeze percent", st_sysinfo_out.si_volume_freeze_percent);
      fn_fprint_string(fptr, "Filler1                            =",
                        st_sysinfo_out.c_filler_1, 4);
      fprintf(fptr,"%-35s=:%ld:\n", "Board lot quantity", st_sysinfo_out.li_board_lot_quantity);
      fprintf(fptr,"%-35s=:%ld:\n", "Tick size", st_sysinfo_out.li_tick_size);
      fprintf(fptr,"%-35s=:%d:\n", "Maximum gtc days", st_sysinfo_out.si_maximum_gtc_days);

      fprintf(fptr,"%-35s=:%d:\n", "Elgble indctr aon", st_sysinfo_out.st_stk_elg_ind.flg_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Elgble indctr min fill",
              st_sysinfo_out.st_stk_elg_ind.flg_minimum_fill);
      fprintf(fptr,"%-35s=:%d:\n", "Elgble indctr books merged",
              st_sysinfo_out.st_stk_elg_ind.flg_books_merged);
      fprintf(fptr,"%-35s=:%d:\n", "Elgble indctr filler1",
              st_sysinfo_out.st_stk_elg_ind.flg_filler1);
      fprintf(fptr,"%-35s=:%c:\n", "Elgble indctr filler2",
              st_sysinfo_out.st_stk_elg_ind.flg_filler2);
      fprintf(fptr,"%-35s=:%d:\n", "Disclosed qty perc allwd",
              st_sysinfo_out.si_disclosed_quantity_percent_allowed);
      fprintf(fptr,"%-35s=:%ld:\n", "Risk free interest rate", st_sysinfo_out.li_risk_free_interest_rate);
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

		case UPDATE_LOCALDB_IN:                     /*7300*/
    {
      struct st_update_local_database st_upd_lcl_db;
      memset(&st_upd_lcl_db, 0, sizeof(st_upd_lcl_db));
      memcpy(&st_upd_lcl_db,(struct st_update_local_database *)c_msg_data,
             sizeof(st_upd_lcl_db));
			fn_cnvt_to_htn_upd_local_db(&st_upd_lcl_db);				/* Ver 1.4 */

      fprintf(fptr,"%-35s=:%ld:\n", "Last upd sec time",
              st_upd_lcl_db.li_last_update_security_time);
      fprintf(fptr,"%-35s=:%ld:\n", "Last upd prtcpnt time",
              st_upd_lcl_db.li_last_update_participant_time);
      fprintf(fptr,"%-35s=:%ld:\n", "Last upd instrument time",
              st_upd_lcl_db.li_last_update_instrument_time);
      fprintf(fptr,"%-35s=:%ld:\n", "Last upd index time",
              st_upd_lcl_db.li_last_update_index_time);
      fprintf(fptr,"%-35s=:%c:\n", "Req for open ordrs",
              st_upd_lcl_db.c_request_for_open_orders);
      fprintf(fptr,"%-35s=:%c:\n", "Filler1", st_upd_lcl_db.c_filler_1);

      fprintf(fptr,"%-35s=:%d:\n", "Market Status Normal",
              st_upd_lcl_db.st_mkt_stts.si_normal);
      fprintf(fptr,"%-35s=:%d:\n", "Market Status Oddlot",
              st_upd_lcl_db.st_mkt_stts.si_oddlot);
      fprintf(fptr,"%-35s=:%d:\n", "Market Status Spot",
              st_upd_lcl_db.st_mkt_stts.si_spot);
      fprintf(fptr,"%-35s=:%d:\n", "Market Status Auction",
              st_upd_lcl_db.st_mkt_stts.si_auction);

      fprintf(fptr,"%-35s=:%d:\n", "Ex Market Status Normal",
              st_upd_lcl_db.st_ex_mkt_stts.si_normal);
      fprintf(fptr,"%-35s=:%d:\n", "Ex Market Status Oddlot",
              st_upd_lcl_db.st_ex_mkt_stts.si_oddlot);
      fprintf(fptr,"%-35s=:%d:\n", "Ex Market Status Spot",
              st_upd_lcl_db.st_ex_mkt_stts.si_spot);
      fprintf(fptr,"%-35s=:%d:\n", "Ex Market Status Auction",
              st_upd_lcl_db.st_ex_mkt_stts.si_auction);

      fprintf(fptr,"%-35s=:%d:\n", "Pl Market Status Normal",
              st_upd_lcl_db.st_pl_mkt_stts.si_normal);
      fprintf(fptr,"%-35s=:%d:\n", "Pl Market Status Oddlot",
              st_upd_lcl_db.st_pl_mkt_stts.si_oddlot);
      fprintf(fptr,"%-35s=:%d:\n", "Pl Market Status Spot",
              st_upd_lcl_db.st_pl_mkt_stts.si_spot);
      fprintf(fptr,"%-35s=:%d:\n", "Pl Market Status Auction",
              st_upd_lcl_db.st_pl_mkt_stts.si_auction);

      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

    case UPDATE_LOCALDB_HEADER:                     /*7307*/
    case UPDATE_LOCALDB_TRAILER:                    /*7308*/
    {
      struct st_update_ldb_header st_upd_ldb_hdr;
      memset(&st_upd_ldb_hdr, 0, sizeof(st_upd_ldb_hdr));
      memcpy(&st_upd_ldb_hdr,(struct st_update_ldb_header *)c_msg_data,sizeof(st_upd_ldb_hdr));

      fn_fprint_string(fptr,"Filler1                            =",
                        st_upd_ldb_hdr.c_filler_1,2);
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

		case UPDATE_LOCALDB_DATA:                       /*7304*/
    {
				/* Wont get */
		}

		case DOWNLOAD_REQUEST:                        /*7000*/
    {
      struct st_message_download st_msg_dwnld;
      memset(&st_msg_dwnld, 0, sizeof(st_msg_dwnld));
      memcpy(&st_msg_dwnld,(struct st_message_download *)c_msg_data, sizeof(st_msg_dwnld));
			fn_cnvt_htn_dwld_req(&st_msg_dwnld);					/* Ver 1.4 */

      memcpy(&ll_jiffy,&st_msg_dwnld.d_sequence_number,sizeof(ll_jiffy));
      fprintf(fptr,"%-35s=:%lld:\n", "Sequence No", ll_jiffy);

      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

    case HEADER_RECORD:                           /*7011*/
    {
      /*only header section is there*/
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

    case MESSAGE_RECORD:                          /*7021*/
    {
			/* wont get */
    }

    case TRAILER_RECORD:                            /*7031*/
    {
      /*only header section is there*/
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }


		/* Normal Order */
		/* Order placement */
    case BOARD_LOT_IN:                              /*2000*/
		case PRICE_CONFIRMATION:                        /*2012* newly added not in EQ TAP */
    case ORDER_CONFIRMATION_OUT:                    /*2073*/
    case FREEZE_TO_CONTROL:                         /*2170*/
    case ORDER_ERROR_OUT:                           /*2231*/
		/* Order Modification */
    case ORDER_MOD_IN:                              /*2040*/
    case ORDER_MOD_OUT:                             /*2041*/
    case ORDER_MOD_CONFIRM_OUT:                     /*2074*/
    case ORDER_MOD_REJ_OUT:                         /*2042*/
		/* Order cancellation */
    case ORDER_CANCEL_IN:                           /*2070*/
    case ORDER_CANCEL_OUT:                          /*2071*/
    case ORDER_CANCEL_CONFIRM_OUT:                  /*2075*/
    case ORDER_CXL_REJ_OUT:                         /*2072*/
    case BATCH_ORDER_CXL_OUT:                       /*9002*/
    {
      struct st_oe_reqres st_oe_req;
      memset(&st_oe_req, 0, sizeof(st_oe_req));
      memcpy(&st_oe_req,(struct st_oe_reqres *)c_msg_data, sizeof(st_oe_req));
			fn_cnvt_nth_oe_reqres(&st_oe_req);

      fprintf(fptr,"%-35s=:%c:\n", "Partcpnt Type", st_oe_req.c_participant_type);
      fprintf(fptr,"%-35s=:%c:\n", "Filler1", st_oe_req.c_filler_1);
      fprintf(fptr,"%-35s=:%d:\n", "Compititor period", st_oe_req.si_competitor_period);
      fprintf(fptr,"%-35s=:%d:\n", "Solicitor period", st_oe_req.si_solicitor_period);
      fprintf(fptr,"%-35s=:%c:\n", "Modfd cncld by", st_oe_req.c_modified_cancelled_by);
      fprintf(fptr,"%-35s=:%c:\n", "Filler2", st_oe_req.c_filler_2);
      fprintf(fptr,"%-35s=:%d:\n", "Reason code", st_oe_req.si_reason_code);
      fn_fprint_string(fptr,"Filler3                            =",st_oe_req.c_filler_3,4);
      fprintf(fptr,"%-35s=:%ld:\n", "Token No", st_oe_req.l_token_no);

      fn_fprint_string(fptr,"Instrument Name                    =",
                          st_oe_req.st_con_desc.c_instrument_name,LEN_INSTRUMENT_NAME);
      fn_fprint_string(fptr,"Symbol                             =",
                          st_oe_req.st_con_desc.c_symbol,LEN_SYMBOL_NSE);
      fprintf(fptr,"%-35s=:%ld:\n", "Expiry Date", st_oe_req.st_con_desc.li_expiry_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Strike Prc", st_oe_req.st_con_desc.li_strike_price);
      fn_fprint_string(fptr,"Option Type                        =",
                          st_oe_req.st_con_desc.c_option_type,LEN_OPTION_TYPE);
      fprintf(fptr,"%-35s=:%d:\n", "CA level", st_oe_req.st_con_desc.si_ca_level);

      fn_fprint_string(fptr,"Counter party brkr id              =",
                        st_oe_req.c_counter_party_broker_id, LEN_BROKER_ID);
      fprintf(fptr,"%-35s=:%c:\n", "Filler4", st_oe_req.c_filler_4);
      fn_fprint_string(fptr,"Filler5                            =",
                        st_oe_req.c_filler_5, 2);
      fprintf(fptr,"%-35s=:%c:\n", "Close Out Flg", st_oe_req.c_closeout_flg);
      fprintf(fptr,"%-35s=:%c:\n", "Filler6", st_oe_req.c_filler_6);
      fprintf(fptr,"%-35s=:%d:\n", "Order Type", st_oe_req.si_order_type);
      fprintf(fptr,"%-35s=:%lf:\n", "Order Number", st_oe_req.d_order_number);
      fn_fprint_string(fptr,"Account Number                     =",
                        st_oe_req.c_account_number, LEN_ACCOUNT_NUMBER);
      fprintf(fptr,"%-35s=:%d:\n", "Book Type", st_oe_req.si_book_type);
      fprintf(fptr,"%-35s=:%d:\n", "Bye Sell indicator", st_oe_req.si_buy_sell_indicator);
      fprintf(fptr,"%-35s=:%ld:\n", "Disclosed volume", st_oe_req.li_disclosed_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "Disclosed vol remain", st_oe_req.li_disclosed_volume_remaining);
      fprintf(fptr,"%-35s=:%ld:\n", "Total vol remain", st_oe_req.li_total_volume_remaining);
      fprintf(fptr,"%-35s=:%ld:\n", "Volume", st_oe_req.li_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "Volume filled today", st_oe_req.li_volume_filled_today);
      fprintf(fptr,"%-35s=:%ld:\n", "Price", st_oe_req.li_price);
      fprintf(fptr,"%-35s=:%ld:\n", "Trigger prc", st_oe_req.li_trigger_price);
      fprintf(fptr,"%-35s=:%ld:\n", "Good till date", st_oe_req.li_good_till_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Entry dt time", st_oe_req.li_entry_date_time);
      fprintf(fptr,"%-35s=:%ld:\n", "Min fill AON volume", st_oe_req.li_minimum_fill_aon_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "Last Modified", st_oe_req.li_last_modified);

      fprintf(fptr,"%-35s=:%d:\n", "Order flag ato",  st_oe_req.st_ord_flg.flg_ato);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mkt", st_oe_req.st_ord_flg.flg_market);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag sl",  st_oe_req.st_ord_flg.flg_sl);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag day",  st_oe_req.st_ord_flg.flg_day);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag gtc",  st_oe_req.st_ord_flg.flg_gtc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ioc",  st_oe_req.st_ord_flg.flg_ioc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag aon",  st_oe_req.st_ord_flg.flg_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mf",  st_oe_req.st_ord_flg.flg_mf);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mtchd ind", st_oe_req.st_ord_flg.flg_matched_ind);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag traded", st_oe_req.st_ord_flg.flg_traded);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag modified", st_oe_req.st_ord_flg.flg_modified);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag frozen", st_oe_req.st_ord_flg.flg_frozen);
      fprintf(fptr,"%-35s=:%d:\n", "Filler1",  st_oe_req.st_ord_flg.flg_filler1);

      fprintf(fptr,"%-35s=:%d:\n", "Branch id", st_oe_req.si_branch_id);

			/***	Commented In Ver 1.1
      fprintf(fptr,"%-35s=:%d:\n", "Trader id", st_oe_req.si_trader_id);
			*************************************/

			fprintf(fptr,"%-35s=:%ld:\n", "Trader id", st_oe_req.li_trader_id);			/***	Ver	1.1	***/

      fn_fprint_string(fptr,"Broker id                          =",
                        st_oe_req.c_broker_id,LEN_BROKER_ID);
      fn_fprint_string(fptr,"Remarks                            =",
                        st_oe_req.c_remarks,LEN_REMARKS);
      fprintf(fptr,"%-35s=:%c:\n", "Open Close", st_oe_req.c_open_close);
      fn_fprint_string(fptr,"Settlor                            =",
                        st_oe_req.c_settlor,LEN_SETTLOR);
      fprintf(fptr,"%-35s=:%d:\n", "Pro client indctr", st_oe_req.si_pro_client_indicator);
      fprintf(fptr,"%-35s=:%d:\n", "Settlement period", st_oe_req.si_settlement_period);
      fprintf(fptr,"%-35s=:%c:\n", "Cover uncover", st_oe_req.c_cover_uncover);
      fprintf(fptr,"%-35s=:%c:\n", "Giveup flg", st_oe_req.c_giveup_flag);
      fprintf(fptr,"%-35s=:%d:\n", "Order sequence",st_oe_req.i_ordr_sqnc);
      fprintf(fptr,"%-35s=:%lf:\n", "NNF Field", st_oe_req.d_nnf_field);
      fprintf(fptr,"%-35s=:%lf:\n", "Filler19",st_oe_req.d_filler19);
			/*** Ver 1.3 Starts ***/
			fn_fprint_string(fptr,"PAN NO															=",
												st_oe_req.c_pan,10);
			fprintf(fptr,"%-35s=:%ld:\n", "Algo Id", st_oe_req.l_algo_id);
			fprintf(fptr,"%-35s=:%d:\n", "Algo Category", st_oe_req.si_algo_category);
			fprintf(fptr,"%-35s=:%lld:\n", "Last activity reference",st_oe_req.ll_lastactivityref); /* Ver 1.4 */
			fn_fprint_string(fptr,"Reserved														=",
												st_oe_req.c_reserved,52);															/* Ver 1.4 */
			/*** Ver 1.3 Ends ***/
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

		/* Spread order newly added*/
		case SP_BOARD_LOT_IN:													/*2100*/
		case SP_BOARD_LOT_OUT:                         /*2101*/
    case SP_ORDER_CONFIRMATION:										/*2124*/
		case SP_ORDER_ERROR: 													/*2154*/
		/* case SP_ORDER_CXL_CONFIRM:											*2130*/
		case SP_ORDER_MOD_IN: 													/*2118*/
		/* case SP_ORDER_CANCEL_IN: 											*2106*/
		case SP_ORDER_MOD_OUT:													/*2119*/
		/* case SP_ORDER_CANCEL_OUT:											*2107*/
		case SP_ORDER_MOD_CON_OUT: 										/*2136*/
		/* case SP_ORDER_CXL_CONFIRMATION: 								*2132*/
		case SP_ORDER_MOD_REJ_OUT: 										/*2133*/
		/* case SP_ORDER_CXL_REJ_OUT: 										*2127*/
		/* 2L and 3L order */
		case TWOL_BOARD_LOT_IN: 														/*2102*/
		case THRL_BOARD_LOT_IN: 														/*2104*/
		case TWOL_BOARD_LOT_OUT: 														/*2103*/
		case THRL_BOARD_LOT_OUT: 														/*2105*/
		case TWOL_ORDER_CONFIRMATION: 											/*2125*/
		case THRL_ORDER_CONFIRMATION: 											/*2126*/
		case TWOL_ORDER_ERROR:  														/*2155*/
		case THRL_ORDER_ERROR: 															/*2156*/
		case TWOL_ORDER_CXL_CONFIRMATION: 									/*2131*/
		case THRL_ORDER_CXL_CONFIRMATION: 									/*2132*/
		{
      struct st_spd_oe_reqres st_spd_ord;
      memset(&st_spd_ord, 0, sizeof(st_spd_ord));
      memcpy(&st_spd_ord,(struct st_spd_oe_reqres *)c_msg_data,sizeof(st_spd_ord));
			fn_cnvt_htn_spd2L3L_req(&st_spd_ord);				/* Ver 1.4 */

			fprintf(fptr,"%-35s=:%c:\n", "participant_type", st_spd_ord.c_participant_type);
			fprintf(fptr,"%-35s=:%c:\n", "filler1", st_spd_ord.c_filler1);
			fprintf(fptr,"%-35s=:%d:\n", "competitor_period", st_spd_ord.si_competitor_period);
			fprintf(fptr,"%-35s=:%d:\n", "solicitor_period", st_spd_ord.si_solicitor_period);
			fprintf(fptr,"%-35s=:%c:\n", "mod_cxl_by", st_spd_ord.c_mod_cxl_by);
			fprintf(fptr,"%-35s=:%c:\n", "st_spd_ord.c_filler2", st_spd_ord.c_filler2);
			fprintf(fptr,"%-35s=:%d:\n", "st_spd_ord.si_reason_code", st_spd_ord.si_reason_code);
      fn_fprint_string(fptr,"start_alpha                        =",
                        st_spd_ord.c_start_alpha,2);
      fn_fprint_string(fptr,"end_alpha                          =",
                        st_spd_ord.c_end_alpha,2);
			fprintf(fptr,"%-35s=:%ld:\n", "token", st_spd_ord.l_token);
      fn_fprint_string(fptr,"Instrument Name                    =",
                          st_spd_ord.st_cntrct_desc.c_instrument_name,LEN_INSTRUMENT_NAME);
      fn_fprint_string(fptr,"Symbol                             =",
                          st_spd_ord.st_cntrct_desc.c_symbol,LEN_SYMBOL_NSE);
      fprintf(fptr,"%-35s=:%ld:\n", "Expiry Date", st_spd_ord.st_cntrct_desc.li_expiry_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Strike Prc", st_spd_ord.st_cntrct_desc.li_strike_price);
      fn_fprint_string(fptr,"Option Type                        =",
                          st_spd_ord.st_cntrct_desc.c_option_type,LEN_OPTION_TYPE);
      fprintf(fptr,"%-35s=:%d:\n", "CA level", st_spd_ord.st_cntrct_desc.si_ca_level);
      fn_fprint_string(fptr,"op_broker_id                       =",
                        st_spd_ord.c_op_broker_id,LEN_BROKER_ID);
			fprintf(fptr,"%-35s=:%c:\n", "filler3", st_spd_ord.c_filler3);
      fn_fprint_string(fptr,"filler_options                     =",
                        st_spd_ord.c_filler_options,LEN_FILLER_OPTIONS);
			fprintf(fptr,"%-35s=:%c:\n", "filler4", st_spd_ord.c_filler4);
			fprintf(fptr,"%-35s=:%d:\n", "order_type", st_spd_ord.si_order_type);
			fprintf(fptr,"%-35s=:%ld:\n", "d_order_number", st_spd_ord.d_order_number);
      fn_fprint_string(fptr,"account_number                     =",
                        st_spd_ord.c_account_number,LEN_ACCOUNT_NUMBER);
			fprintf(fptr,"%-35s=:%d:\n", "book_type", st_spd_ord.si_book_type);
			fprintf(fptr,"%-35s=:%d:\n", "buy_sell", st_spd_ord.si_buy_sell);
			fprintf(fptr,"%-35s=:%ld:\n", "disclosed_vol", st_spd_ord.li_disclosed_vol);
			fprintf(fptr,"%-35s=:%ld:\n", "closed_vol_remaining", st_spd_ord.li_disclosed_vol_remaining);
			fprintf(fptr,"%-35s=:%ld:\n", "total_vol_remaining", st_spd_ord.li_total_vol_remaining);
			fprintf(fptr,"%-35s=:%ld:\n", "volume", st_spd_ord.li_volume);
			fprintf(fptr,"%-35s=:%ld:\n", "volume_filled_today", st_spd_ord.li_volume_filled_today);
			fprintf(fptr,"%-35s=:%ld:\n", "price", st_spd_ord.li_price);
			fprintf(fptr,"%-35s=:%ld:\n", "trigger_price", st_spd_ord.li_trigger_price);
			fprintf(fptr,"%-35s=:%ld:\n", "good_till_date", st_spd_ord.li_good_till_date);
			fprintf(fptr,"%-35s=:%ld:\n", "entry_date_time", st_spd_ord.li_entry_date_time);
			fprintf(fptr,"%-35s=:%ld:\n", "min_fill_aon", st_spd_ord.li_min_fill_aon);
			fprintf(fptr,"%-35s=:%ld:\n", "last_modified", st_spd_ord.li_last_modified);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ato",  st_spd_ord.st_order_flgs.flg_ato);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mkt", st_spd_ord.st_order_flgs.flg_market);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag sl",  st_spd_ord.st_order_flgs.flg_sl);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag day",  st_spd_ord.st_order_flgs.flg_day);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag gtc",  st_spd_ord.st_order_flgs.flg_gtc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ioc",  st_spd_ord.st_order_flgs.flg_ioc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag aon",  st_spd_ord.st_order_flgs.flg_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mf",  st_spd_ord.st_order_flgs.flg_mf);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mtchd ind", st_spd_ord.st_order_flgs.flg_matched_ind);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag traded", st_spd_ord.st_order_flgs.flg_traded);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag modified", st_spd_ord.st_order_flgs.flg_modified);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag frozen", st_spd_ord.st_order_flgs.flg_frozen);
      fprintf(fptr,"%-35s=:%d:\n", "Filler1",  st_spd_ord.st_order_flgs.flg_filler1);
			fprintf(fptr,"%-35s=:%d:\n", "branch_id", st_spd_ord.si_branch_id);

			/***	Commented In Ver 1.1
			fprintf(fptr,"%-35s=:%d:\n", "trader_id", st_spd_ord.si_trader_id);
			******************************************/

			fprintf(fptr,"%-35s=:%ld:\n", "trader_id", st_spd_ord.li_trader_id);			/***	Ver	1.1	***/
      fn_fprint_string(fptr,"broker_id                          =",
                        st_spd_ord.c_broker_id,LEN_BROKER_ID);
     /* fn_fprint_string(fptr,"oe_remarks                         =",
                        st_spd_ord.c_oe_remarks,LEN_REMARKS);								*** Ver 1.2 comment**/
			fprintf(fptr,"%-35s=:%c:\n", "open_close", st_spd_ord.c_open_close);
      fn_fprint_string(fptr,"Settlor                            =",
                        st_spd_ord.c_settlor,LEN_SETTLOR);
			fprintf(fptr,"%-35s=:%d:\n", "pro_client", st_spd_ord.si_pro_client);
			fprintf(fptr,"%-35s=:%d:\n", "settlement_period", st_spd_ord.si_settlement_period);
			fprintf(fptr,"%-35s=:%c:\n", "cover_uncover", st_spd_ord.c_cover_uncover);
			fprintf(fptr,"%-35s=:%c:\n", "give_up_flag", st_spd_ord.c_give_up_flag);
			/***** Ver 1.2 Comment	Starts ****************************
			fprintf(fptr,"%-35s=:%d:\n", "us_filler1", st_spd_ord.us_filler1);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler2", st_spd_ord.us_filler2);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler3", st_spd_ord.us_filler3);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler4", st_spd_ord.us_filler4);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler5", st_spd_ord.us_filler5);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler6", st_spd_ord.us_filler6);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler7", st_spd_ord.us_filler7);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler8", st_spd_ord.us_filler8);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler9", st_spd_ord.us_filler9);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler10", st_spd_ord.us_filler10);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler11", st_spd_ord.us_filler11);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler12", st_spd_ord.us_filler12);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler13", st_spd_ord.us_filler13);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler14", st_spd_ord.us_filler14);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler15", st_spd_ord.us_filler15);
			fprintf(fptr,"%-35s=:%d:\n", "us_filler16", st_spd_ord.us_filler16);
			fprintf(fptr,"%-35s=:%c:\n", "c_filler17", st_spd_ord.c_filler17);
			fprintf(fptr,"%-35s=:%c:\n", "c_filler18", st_spd_ord.c_filler18);
				********* Ver 1.2 Comment Ends ****************/
			fprintf(fptr,"%-35s=:%d:\n","i_sprd_seq_no",st_spd_ord.i_sprd_seq_no);		/*** Ver 1.2 ***/
			fprintf(fptr,"%-35s=:%lf:\n", "nnf_field", st_spd_ord.d_nnf_field);
			fprintf(fptr,"%-35s=:%lf:\n", "filler19", st_spd_ord.d_filler19);
			/*** Ver 1.3 Starts ***/
      fn_fprint_string(fptr,"PAN NO                             =",
                        st_spd_ord.c_pan,10);
      fprintf(fptr,"%-35s=:%ld:\n", "Algo Id", st_spd_ord.l_algo_id);
      fprintf(fptr,"%-35s=:%d:\n", "Algo Category", st_spd_ord.si_algo_category);
			fprintf(fptr,"%-35s=:%lld:\n", "Last activity reference",st_spd_ord.ll_lastactivityref); /* Ver 1.4 */
      fn_fprint_string(fptr,"Reserved                           =",
                        st_spd_ord.c_reserved,52);													/* Ver 1.4 */
      /*** Ver 1.3 Ends ***/
			fprintf(fptr,"%-35s=:%ld:\n", "spd_price_diff", st_spd_ord.li_spd_price_diff);

			fprintf(fptr,"%-35s=:%ld:\n", "Second Leg ---- token", st_spd_ord.st_spd_lg_inf[0].l_token);
      fn_fprint_string(fptr,"Instrument Name                    =",
                          st_spd_ord.st_spd_lg_inf[0].st_cntrct_desc.c_instrument_name,LEN_INSTRUMENT_NAME);
      fn_fprint_string(fptr,"Symbol                             =",
                          st_spd_ord.st_spd_lg_inf[0].st_cntrct_desc.c_symbol,LEN_SYMBOL_NSE);
      fprintf(fptr,"%-35s=:%ld:\n", "Expiry Date", st_spd_ord.st_spd_lg_inf[0].st_cntrct_desc.li_expiry_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Strike Prc", st_spd_ord.st_spd_lg_inf[0].st_cntrct_desc.li_strike_price);
      fn_fprint_string(fptr,"Option Type                        =",
                          st_spd_ord.st_spd_lg_inf[0].st_cntrct_desc.c_option_type,LEN_OPTION_TYPE);
      fprintf(fptr,"%-35s=:%d:\n", "CA level", st_spd_ord.st_spd_lg_inf[0].st_cntrct_desc.si_ca_level);
      fn_fprint_string(fptr,"Option Type                        =",
                          st_spd_ord.st_spd_lg_inf[0].c_op_broker_id,LEN_BROKER_ID);
			fprintf(fptr,"%-35s=:%c:\n", "filler1", st_spd_ord.st_spd_lg_inf[0].c_filler1);
      fprintf(fptr,"%-35s=:%d:\n", "order_typ", st_spd_ord.st_spd_lg_inf[0].si_order_type);
      fprintf(fptr,"%-35s=:%d:\n", "buy_sell", st_spd_ord.st_spd_lg_inf[0].si_buy_sell);
      fprintf(fptr,"%-35s=:%ld:\n", "disclosed_vol", st_spd_ord.st_spd_lg_inf[0].li_disclosed_vol);
      fprintf(fptr,"%-35s=:%ld:\n", "disclosed_vol_remaining", st_spd_ord.st_spd_lg_inf[0].li_disclosed_vol_remaining);
      fprintf(fptr,"%-35s=:%ld:\n", "total_vol_remaining", st_spd_ord.st_spd_lg_inf[0].li_total_vol_remaining);
      fprintf(fptr,"%-35s=:%ld:\n", "volume", st_spd_ord.st_spd_lg_inf[0].li_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "volume_filled_today", st_spd_ord.st_spd_lg_inf[0].li_volume_filled_today);
      fprintf(fptr,"%-35s=:%ld:\n", "price", st_spd_ord.st_spd_lg_inf[0].li_price);
      fprintf(fptr,"%-35s=:%ld:\n", "trigger_price", st_spd_ord.st_spd_lg_inf[0].li_trigger_price);
      fprintf(fptr,"%-35s=:%ld:\n", "min_fill_aon", st_spd_ord.st_spd_lg_inf[0].li_min_fill_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ato",  st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_ato);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mkt", st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_market);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag sl",  st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_sl);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag day",  st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_day);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag gtc",  st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_gtc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ioc",  st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_ioc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag aon",  st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mf",  st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_mf);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mtchd ind", st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_matched_ind);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag traded", st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_traded);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag modified", st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_modified);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag frozen", st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_frozen);
      fprintf(fptr,"%-35s=:%d:\n", "Filler1",  st_spd_ord.st_spd_lg_inf[0].st_order_flgs.flg_filler1);
			fprintf(fptr,"%-35s=:%c:\n", "open_close", st_spd_ord.st_spd_lg_inf[0].c_open_close);
			fprintf(fptr,"%-35s=:%c:\n", "cover_uncover", st_spd_ord.st_spd_lg_inf[0].c_cover_uncover);
			fprintf(fptr,"%-35s=:%c:\n", "c_giveup_flag", st_spd_ord.st_spd_lg_inf[0].c_giveup_flag);
			fprintf(fptr,"%-35s=:%c:\n", "filler2", st_spd_ord.st_spd_lg_inf[0].c_filler2);

      fprintf(fptr,"%-35s=:%ld:\n", "Third Leg ---- token", st_spd_ord.st_spd_lg_inf[1].l_token);
      fn_fprint_string(fptr,"Instrument Name                    =",
                          st_spd_ord.st_spd_lg_inf[1].st_cntrct_desc.c_instrument_name,LEN_INSTRUMENT_NAME);
      fn_fprint_string(fptr,"Symbol                             =",
                          st_spd_ord.st_spd_lg_inf[1].st_cntrct_desc.c_symbol,LEN_SYMBOL_NSE);
      fprintf(fptr,"%-35s=:%ld:\n", "Expiry Date", st_spd_ord.st_spd_lg_inf[1].st_cntrct_desc.li_expiry_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Strike Prc", st_spd_ord.st_spd_lg_inf[1].st_cntrct_desc.li_strike_price);
      fn_fprint_string(fptr,"Option Type                        =",
                          st_spd_ord.st_spd_lg_inf[1].st_cntrct_desc.c_option_type,LEN_OPTION_TYPE);
      fprintf(fptr,"%-35s=:%d:\n", "CA level", st_spd_ord.st_spd_lg_inf[1].st_cntrct_desc.si_ca_level);
      fn_fprint_string(fptr,"Option Type                        =",
                          st_spd_ord.st_spd_lg_inf[1].c_op_broker_id,LEN_BROKER_ID);
      fprintf(fptr,"%-35s=:%c:\n", "filler1", st_spd_ord.st_spd_lg_inf[1].c_filler1);
      fprintf(fptr,"%-35s=:%d:\n", "order_typ", st_spd_ord.st_spd_lg_inf[1].si_order_type);
      fprintf(fptr,"%-35s=:%d:\n", "buy_sell", st_spd_ord.st_spd_lg_inf[1].si_buy_sell);
      fprintf(fptr,"%-35s=:%ld:\n", "disclosed_vol", st_spd_ord.st_spd_lg_inf[1].li_disclosed_vol);
      fprintf(fptr,"%-35s=:%ld:\n", "disclosed_vol_remaining", st_spd_ord.st_spd_lg_inf[1].li_disclosed_vol_remaining);
      fprintf(fptr,"%-35s=:%ld:\n", "total_vol_remaining", st_spd_ord.st_spd_lg_inf[1].li_total_vol_remaining);
      fprintf(fptr,"%-35s=:%ld:\n", "volume", st_spd_ord.st_spd_lg_inf[1].li_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "volume_filled_today", st_spd_ord.st_spd_lg_inf[1].li_volume_filled_today);
      fprintf(fptr,"%-35s=:%ld:\n", "price", st_spd_ord.st_spd_lg_inf[1].li_price);
      fprintf(fptr,"%-35s=:%ld:\n", "trigger_price", st_spd_ord.st_spd_lg_inf[1].li_trigger_price);
      fprintf(fptr,"%-35s=:%ld:\n", "min_fill_aon", st_spd_ord.st_spd_lg_inf[1].li_min_fill_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ato",  st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_ato);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mkt", st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_market);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag sl",  st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_sl);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag day",  st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_day);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag gtc",  st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_gtc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ioc",  st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_ioc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag aon",  st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mf",  st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_mf);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mtchd ind", st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_matched_ind);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag traded", st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_traded);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag modified", st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_modified);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag frozen", st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_frozen);
      fprintf(fptr,"%-35s=:%d:\n", "Filler1",  st_spd_ord.st_spd_lg_inf[1].st_order_flgs.flg_filler1);
      fprintf(fptr,"%-35s=:%c:\n", "open_close", st_spd_ord.st_spd_lg_inf[1].c_open_close);
      fprintf(fptr,"%-35s=:%c:\n", "cover_uncover", st_spd_ord.st_spd_lg_inf[1].c_cover_uncover);
      fprintf(fptr,"%-35s=:%c:\n", "c_giveup_flag", st_spd_ord.st_spd_lg_inf[1].c_giveup_flag);
      fprintf(fptr,"%-35s=:%c:\n", "filler2", st_spd_ord.st_spd_lg_inf[1].c_filler2);
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
		}

		/* Exercise and position liquidation */
		case EX_PL_ENTRY_IN:                 /*4000*/
		case EX_PL_ENTRY_OUT:                /*4001*/
		case EX_PL_CONFIRMATION:             /*4002*/
		case EX_PL_MOD_IN:                   /*4005*/
		case EX_PL_MOD_OUT:                  /*4006*/
		case EX_PL_MOD_CONFIRMATION:         /*4007*/
		case EX_PL_CXL_IN:                   /*4008*/
		case EX_PL_CXL_OUT:                  /*4009*/
		case EX_PL_CXL_CONFIRMATION:         /*4010*/
		{
			struct st_ex_pl_reqres st_ex_pl;
			memset( &st_ex_pl, 0, sizeof(st_ex_pl) );
			memcpy( &st_ex_pl, (struct st_ex_pl_reqres *)c_msg_data, sizeof(st_ex_pl) );
			fn_cnvt_htn_new_exer_req(&st_ex_pl); 			/* Ver 1.4 */

			fprintf(fptr,"%-35s=:%d:\n", "Reason Code", st_ex_pl.si_reason_code);
			fprintf(fptr,"%-35s=:%ld:\n", "Token", st_ex_pl.st_ex_pl_data.l_token);

			fn_fprint_string(fptr,"Instrument Name                    =",
                          st_ex_pl.st_ex_pl_data.st_cntrct_desc.c_instrument_name,LEN_INSTRUMENT_NAME);
      fn_fprint_string(fptr,"Symbol                             =",
                          st_ex_pl.st_ex_pl_data.st_cntrct_desc.c_symbol,LEN_SYMBOL_NSE);
      fprintf(fptr,"%-35s=:%ld:\n", "Expiry Date", st_ex_pl.st_ex_pl_data.st_cntrct_desc.li_expiry_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Strike Prc", st_ex_pl.st_ex_pl_data.st_cntrct_desc.li_strike_price);
      fn_fprint_string(fptr,"Option Type                        =",
                          st_ex_pl.st_ex_pl_data.st_cntrct_desc.c_option_type,LEN_OPTION_TYPE);
      fprintf(fptr,"%-35s=:%d:\n", "CA level", st_ex_pl.st_ex_pl_data.st_cntrct_desc.si_ca_level);

			fprintf(fptr,"%-35s=:%d:\n", "expl_flag", st_ex_pl.st_ex_pl_data.si_expl_flag);
			fprintf(fptr,"%-35s=:%lf:\n", "expl_number", st_ex_pl.st_ex_pl_data.d_expl_number);
			fprintf(fptr,"%-35s=:%d:\n", "market_type", st_ex_pl.st_ex_pl_data.si_market_type);
			fn_fprint_string(fptr,"Account No                         =",
                            st_ex_pl.st_ex_pl_data.c_account_number,LEN_ACCOUNT_NUMBER);
			fprintf(fptr,"%-35s=:%ld:\n", "quantity", st_ex_pl.st_ex_pl_data.li_quantity);
			fprintf(fptr,"%-35s=:%d:\n", "pro_cli", st_ex_pl.st_ex_pl_data.si_pro_cli);
			fprintf(fptr,"%-35s=:%d:\n", "exercise_type", st_ex_pl.st_ex_pl_data.si_exercise_type);
			fprintf(fptr,"%-35s=:%ld:\n", "entry_date_tm", st_ex_pl.st_ex_pl_data.li_entry_date_tm);
			fprintf(fptr,"%-35s=:%d:\n", "branch_id", st_ex_pl.st_ex_pl_data.si_branch_id);
			/***	Commented In Ver 1.1
			fprintf(fptr,"%-35s=:%d:\n", "trader_id", st_ex_pl.st_ex_pl_data.si_trader_id);
			***********************************/

			fprintf(fptr,"%-35s=:%ld:\n", "trader_id", st_ex_pl.st_ex_pl_data.li_trader_id);			/***	Ver	1.1	***/
			fn_fprint_string(fptr,"Broker id                          =",
                            st_ex_pl.st_ex_pl_data.c_broker_id,LEN_BROKER_ID);
			fn_fprint_string(fptr,"Remark 1                           =",
                            st_ex_pl.st_ex_pl_data.c_remarks,LEN_REMARKS_1);
			fn_fprint_string(fptr,"Participant                        =",
                            st_ex_pl.st_ex_pl_data.c_participant,LEN_SETTLOR);
			fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
		}

    /* Unsolicited Messages */
    case ON_STOP_NOTIFICATION:                    /*2212*/
    case TRADE_CONFIRMATION:                      /*2222*/
    {
      struct st_trade_confirm st_trd_cnfrm;
      memset(&st_trd_cnfrm, 0, sizeof(st_trd_cnfrm));
      memcpy(&st_trd_cnfrm,(struct st_trade_confirm *)c_msg_data,sizeof(st_trd_cnfrm));
			fn_cnvt_nth_trade_confirm(&st_trd_cnfrm);				/* Ver 1.4 */

      fprintf(fptr,"%-35s=:%lf:\n", "Response order no",st_trd_cnfrm.d_response_order_number);
      fn_fprint_string(fptr,"Broker id                          =",
                            st_trd_cnfrm.c_broker_id,LEN_BROKER_ID);
      fprintf(fptr,"%-35s=:%c:\n", "Filler1", st_trd_cnfrm.c_filler_1);

			/***	Commented In Ver 1.1
      fprintf(fptr,"%-35s=:%d:\n", "Trader no", st_trd_cnfrm.si_trader_number);
			********************************************/

			fprintf(fptr,"%-35s=:%ld:\n", "Trader no", st_trd_cnfrm.li_trader_number);			/***	Ver	1.1	***/

      fn_fprint_string(fptr,"Account no                         =",
                        st_trd_cnfrm.c_account_number, LEN_ACCOUNT_NUMBER);
      fprintf(fptr,"%-35s=:%d:\n", "Buy sell indctr", st_trd_cnfrm.si_buy_sell_indicator);
      fprintf(fptr,"%-35s=:%ld:\n", "Original volume",  st_trd_cnfrm.li_original_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "Disclosed volume", st_trd_cnfrm.li_disclosed_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "Remaining volume", st_trd_cnfrm.li_remaining_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "Disclosed vol remng",
              st_trd_cnfrm.li_disclosed_volume_remaining);
      fprintf(fptr,"%-35s=:%ld:\n", "Price",  st_trd_cnfrm.li_price);

      fprintf(fptr,"%-35s=:%d:\n", "Order flag ato",  st_trd_cnfrm.st_ord_flg.flg_ato);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mkt", st_trd_cnfrm.st_ord_flg.flg_market);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag sl",  st_trd_cnfrm.st_ord_flg.flg_sl);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag lit",  st_trd_cnfrm.st_ord_flg.flg_mit);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag day",  st_trd_cnfrm.st_ord_flg.flg_day);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag gtc",  st_trd_cnfrm.st_ord_flg.flg_gtc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ioc",  st_trd_cnfrm.st_ord_flg.flg_ioc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag aon",  st_trd_cnfrm.st_ord_flg.flg_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mf",  st_trd_cnfrm.st_ord_flg.flg_mf);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mtchd ind",
              st_trd_cnfrm.st_ord_flg.flg_matched_ind);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag traded", st_trd_cnfrm.st_ord_flg.flg_traded);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag modified", st_trd_cnfrm.st_ord_flg.flg_modified);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag frozen", st_trd_cnfrm.st_ord_flg.flg_frozen);
      fprintf(fptr,"%-35s=:%d:\n", "Filler1", st_trd_cnfrm.st_ord_flg.flg_filler1);

      fprintf(fptr,"%-35s=:%ld:\n", "Good till date", st_trd_cnfrm.li_good_till_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Fill no", st_trd_cnfrm.li_fill_number);
      fprintf(fptr,"%-35s=:%ld:\n", "Fill qty", st_trd_cnfrm.li_fill_quantity);
      fprintf(fptr,"%-35s=:%ld:\n", "Fill price", st_trd_cnfrm.li_fill_price);
      fprintf(fptr,"%-35s=:%ld:\n", "Vol filled today", st_trd_cnfrm.li_volume_filled_today);
      fn_fprint_string(fptr,"Activity type                      =",
                        st_trd_cnfrm.c_activity_type, LEN_ACTIVITY_TYPE);
      fprintf(fptr,"%-35s=:%ld:\n", "Activity time", st_trd_cnfrm.li_activity_time);
      fprintf(fptr,"%-35s=:%lf:\n", "Counter trd ord no",
              st_trd_cnfrm.d_counter_trade_order_number);
      fn_fprint_string(fptr,"Counter broker id                  =",
                        st_trd_cnfrm.c_counter_broker_id, LEN_BROKER_ID);
      fprintf(fptr,"%-35s=:%ld:\n", "Token no", st_trd_cnfrm.l_token);

      fn_fprint_string(fptr,"Instrument Name                    =",
                          st_trd_cnfrm.st_con_desc.c_instrument_name,LEN_INSTRUMENT_NAME);
      fn_fprint_string(fptr,"Symbol                             =",
                          st_trd_cnfrm.st_con_desc.c_symbol,LEN_SYMBOL_NSE);
      fprintf(fptr,"%-35s=:%ld:\n", "Expiry Date", st_trd_cnfrm.st_con_desc.li_expiry_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Strike Prc", st_trd_cnfrm.st_con_desc.li_strike_price);
      fn_fprint_string(fptr,"Option Type                        =",
                          st_trd_cnfrm.st_con_desc.c_option_type,LEN_OPTION_TYPE);
      fprintf(fptr,"%-35s=:%d:\n", "CA level", st_trd_cnfrm.st_con_desc.si_ca_level);

      fprintf(fptr,"%-35s=:%c:\n", "Open Close", st_trd_cnfrm.c_open_close);
      fprintf(fptr,"%-35s=:%c:\n", "Old Open Close", st_trd_cnfrm.c_old_open_close);
      fprintf(fptr,"%-35s=:%c:\n", "book Type", st_trd_cnfrm.c_book_type);
      fprintf(fptr,"%-35s=:%ld:\n", "New Volume", st_trd_cnfrm.li_new_volume);
			fn_fprint_string(fptr,"Old account number                 =",
                          st_trd_cnfrm.c_old_account_number,LEN_ACCOUNT_NUMBER);
			fn_fprint_string(fptr,"Participant                        =",
                          st_trd_cnfrm.c_participant,LEN_PARTICIPANT_NAME);
			fn_fprint_string(fptr,"Old participant                    =",
                          st_trd_cnfrm.c_old_participant,LEN_PARTICIPANT_NAME);
      fprintf(fptr,"%-35s=:%c:\n", "Cover uncover", st_trd_cnfrm.c_cover_uncover);
      fprintf(fptr,"%-35s=:%c:\n", "Old cover uncover", st_trd_cnfrm.c_old_cover_uncover);
      fprintf(fptr,"%-35s=:%c:\n", "Give up trade", st_trd_cnfrm.c_giveup_trade);
			/*** Ver 1.3 Starts ***/
			fprintf(fptr,"%-35s=:%c:\n", "Reserved Filler2", st_trd_cnfrm.c_reserved_filler2);
			fn_fprint_string(fptr,"PAN NO															=",
													st_trd_cnfrm.c_pan,10);
			fn_fprint_string(fptr,"OLD PAN NO                             =",
													st_trd_cnfrm.c_old_pan,10);
			fprintf(fptr,"%-35s=:%ld:\n", "Algo Id",st_trd_cnfrm.l_algo_id);
			fprintf(fptr,"%-35s=:%d:\n", "Algo Catedory Id",st_trd_cnfrm.si_algo_category);
			fprintf(fptr,"%-35s=:%lld:\n", "Last activity reference",st_trd_cnfrm.ll_lastactivityref); /* Ver 1.4 */
			fn_fprint_string(fptr,"RESERVED                           =",
													st_trd_cnfrm.c_reserved,52); 												/* Ver 1.4 */
			/*** Ver 1.3 Ends ***/
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }
		case CTRL_MSG_TO_TRADER:                      /*5295*/
    {
      /*Not being sent*/
      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;
    }

/* ver 1.4 started */

    case BOARD_LOT_IN_TR:													/*20000*/
    {
      struct st_oe_reqres_tr st_oe_req_tr;
      memset(&st_oe_req_tr, 0, sizeof(st_oe_req_tr));
      memcpy(&st_oe_req_tr,(struct st_oe_reqres_tr *)c_msg_data,sizeof(st_oe_req_tr));
			fn_cnvt_htn_new_ordr_req_tr(&st_oe_req_tr);				/* Ver 1.4 */

      fprintf(fptr,"%-35s=:%d:\n", "Transaction Code", st_oe_req_tr.si_transaction_code);
      fprintf(fptr,"%-35s=:%ld:\n", "User Id", st_oe_req_tr.li_userid);
      fprintf(fptr,"%-35s=:%d:\n", "Reason code", st_oe_req_tr.si_reason_code);
      fprintf(fptr,"%-35s=:%ld:\n", "Token No", st_oe_req_tr.l_token_no);

      fn_fprint_string(fptr,"Instrument Name                    =",
                          st_oe_req_tr.st_con_desc_tr.c_instrument_name,LEN_INSTRUMENT_NAME);
      fn_fprint_string(fptr,"Symbol                             =",
                          st_oe_req_tr.st_con_desc_tr.c_symbol,LEN_SYMBOL_NSE);
      fprintf(fptr,"%-35s=:%ld:\n", "Expiry Date", st_oe_req_tr.st_con_desc_tr.li_expiry_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Strike Prc", st_oe_req_tr.st_con_desc_tr.li_strike_price);
      fn_fprint_string(fptr,"Option Type                        =",
                          st_oe_req_tr.st_con_desc_tr.c_option_type,LEN_OPTION_TYPE);
      fn_fprint_string(fptr,"Account Number                     =",
                        st_oe_req_tr.c_account_number, LEN_ACCOUNT_NUMBER);
      fprintf(fptr,"%-35s=:%d:\n", "Book Type", st_oe_req_tr.si_book_type);
      fprintf(fptr,"%-35s=:%d:\n", "Bye Sell indicator", st_oe_req_tr.si_buy_sell_indicator);
      fprintf(fptr,"%-35s=:%ld:\n", "Disclosed volume", st_oe_req_tr.li_disclosed_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "Volume", st_oe_req_tr.li_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "Price", st_oe_req_tr.li_price);
      fprintf(fptr,"%-35s=:%ld:\n", "Good till date", st_oe_req_tr.li_good_till_date);

      fprintf(fptr,"%-35s=:%d:\n", "Order flag ato",  st_oe_req_tr.st_ord_flg.flg_ato);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mkt", st_oe_req_tr.st_ord_flg.flg_market);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag sl",  st_oe_req_tr.st_ord_flg.flg_sl);
			fprintf(fptr,"%-35s=:%d:\n", "Order flag mit",  st_oe_req_tr.st_ord_flg.flg_mit);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag day",  st_oe_req_tr.st_ord_flg.flg_day);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag gtc",  st_oe_req_tr.st_ord_flg.flg_gtc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ioc",  st_oe_req_tr.st_ord_flg.flg_ioc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag aon",  st_oe_req_tr.st_ord_flg.flg_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mf",  st_oe_req_tr.st_ord_flg.flg_mf);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mtchd ind", st_oe_req_tr.st_ord_flg.flg_matched_ind);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag traded", st_oe_req_tr.st_ord_flg.flg_traded);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag modified", st_oe_req_tr.st_ord_flg.flg_modified);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag frozen", st_oe_req_tr.st_ord_flg.flg_frozen);
      fprintf(fptr,"%-35s=:%d:\n", "Filler1",  st_oe_req_tr.st_ord_flg.flg_filler1);
      fprintf(fptr,"%-35s=:%d:\n", "Branch id", st_oe_req_tr.si_branch_id);
      fprintf(fptr,"%-35s=:%ld:\n", "Trader id", st_oe_req_tr.li_trader_id);
      fn_fprint_string(fptr,"Broker id                          =",
                        st_oe_req_tr.c_broker_id,LEN_BROKER_ID);
      fprintf(fptr,"%-35s=:%c:\n", "Open Close", st_oe_req_tr.c_open_close);
      fn_fprint_string(fptr,"Settlor                            =",
                        st_oe_req_tr.c_settlor,LEN_SETTLOR);
      fprintf(fptr,"%-35s=:%d:\n", "Pro client indctr", st_oe_req_tr.si_pro_client_indicator);
      fprintf(fptr,"%-35s=:%c:\n", "Cover uncover flag",  st_oe_req_tr.st_add_ord_flg.c_cover_uncover);
      fprintf(fptr,"%-35s=:%ld:\n", "Order sequence",  st_oe_req_tr.li_ordr_sqnc);
      fprintf(fptr,"%-35s=:%lf:\n", "NNF Field", st_oe_req_tr.d_nnf_field);
      fn_fprint_string(fptr,"PAN NO                             =",
                        st_oe_req_tr.c_pan,10);
      fprintf(fptr,"%-35s=:%ld:\n", "Algo Id", st_oe_req_tr.l_algo_id);
      fprintf(fptr,"%-35s=:%d:\n", "Algo Category", st_oe_req_tr.si_algo_category);
      fn_fprint_string(fptr,"Reserved                           =",
                        st_oe_req_tr.c_reserved,32);


      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;

    }

    case ORDER_MOD_IN_TR:																/*20040*/
    case ORDER_CANCEL_IN_TR:														/*20070*/
    {
      struct st_om_rqst_tr st_om_rqst;
      memset(&st_om_rqst, 0, sizeof(st_om_rqst));
      memcpy(&st_om_rqst,(struct st_om_rqst_tr *)c_msg_data,sizeof(st_om_rqst));
			fn_cnvt_htn_modcan_req_tr(&st_om_rqst);							/* Ver 1.4 */
			
			fprintf(fptr,"%-35s=:%d:\n", "Transaction Code", st_om_rqst.si_transaction_code);
      fprintf(fptr,"%-35s=:%ld:\n", "User Id", st_om_rqst.li_userid);
			fprintf(fptr,"%-35s=:%c:\n", "c_modified_cancelled_by" , st_om_rqst.c_modified_cancelled_by);
			fprintf(fptr,"%-35s=:%ld:\n", "Token number" , st_om_rqst.l_token_no);
		  fn_fprint_string(fptr,"Instrument Name                    =",
                          st_om_rqst.st_con_desc_tr.c_instrument_name,LEN_INSTRUMENT_NAME);
			fn_fprint_string(fptr,"Symbol                             =",
                        st_om_rqst.st_con_desc_tr.c_symbol,LEN_SYMBOL_NSE);
      fprintf(fptr,"%-35s=:%ld:\n", "Expiry date",st_om_rqst.st_con_desc_tr.li_expiry_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Strike prize",st_om_rqst.st_con_desc_tr.li_strike_price);
      fn_fprint_string(fptr,"Option type                        =",
                        st_om_rqst.st_con_desc_tr.c_option_type,LEN_OPTION_TYPE);
	    fprintf(fptr,"%-35s=:%lf:\n", "Order number" , st_om_rqst.d_order_number);
			fn_fprint_string(fptr,"Account Number                     =",
                        st_om_rqst.c_account_number, LEN_ACCOUNT_NUMBER);
			fprintf(fptr,"%-35s=:%d:\n", "Book type", st_om_rqst.si_book_type);
			fprintf(fptr,"%-35s=:%d:\n", "buy_sell_indicator", st_om_rqst.si_buy_sell_indicator);
			fprintf(fptr,"%-35s=:%ld:\n", "Disclosed volume", st_om_rqst.li_disclosed_volume);
			fprintf(fptr,"%-35s=:%ld:\n", "Disclosed volume remaining", st_om_rqst.li_disclosed_volume_remaining);
			fprintf(fptr,"%-35s=:%ld:\n", "Total volume remaining", st_om_rqst.li_total_volume_remaining);
			fprintf(fptr,"%-35s=:%ld:\n", "Volume", st_om_rqst.li_volume);
			fprintf(fptr,"%-35s=:%ld:\n", "Volume filled today", st_om_rqst.li_volume_filled_today);
			fprintf(fptr,"%-35s=:%ld:\n", "Prize", st_om_rqst.li_price);
			fprintf(fptr,"%-35s=:%ld:\n", "Good till date", st_om_rqst.li_good_till_date);
			fprintf(fptr,"%-35s=:%ld:\n", "Entry date time", st_om_rqst.li_entry_date_time);
			fprintf(fptr,"%-35s=:%ld:\n", "Last modified", st_om_rqst.li_last_modified);

			fprintf(fptr,"%-35s=:%d:\n", "Order flag ato",  st_om_rqst.st_ord_flg.flg_ato);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mkt", st_om_rqst.st_ord_flg.flg_market);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag sl",  st_om_rqst.st_ord_flg.flg_sl);
			fprintf(fptr,"%-35s=:%d:\n", "Order flag mit", st_om_rqst.st_ord_flg.flg_mit);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag day",  st_om_rqst.st_ord_flg.flg_day);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag gtc",  st_om_rqst.st_ord_flg.flg_gtc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ioc",  st_om_rqst.st_ord_flg.flg_ioc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag aon",  st_om_rqst.st_ord_flg.flg_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mf",  st_om_rqst.st_ord_flg.flg_mf);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mtchd ind", st_om_rqst.st_ord_flg.flg_matched_ind);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag traded", st_om_rqst.st_ord_flg.flg_traded);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag modified", st_om_rqst.st_ord_flg.flg_modified);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag frozen", st_om_rqst.st_ord_flg.flg_frozen);
      fprintf(fptr,"%-35s=:%d:\n", "Filler1",  st_om_rqst.st_ord_flg.flg_filler1);
			
			fprintf(fptr,"%-35s=:%d:\n", "Branch id", st_om_rqst.si_branch_id);
			fprintf(fptr,"%-35s=:%ld:\n", "Trader id", st_om_rqst.li_trader_id);
			fn_fprint_string(fptr,"Broker id                          =",
                        st_om_rqst.c_broker_id,LEN_BROKER_ID);
			fprintf(fptr,"%-35s=:%c:\n", "Open close", st_om_rqst.c_open_close);
			fn_fprint_string(fptr,"Settler                            =",
												st_om_rqst.c_settlor,LEN_SETTLOR);
			fprintf(fptr,"%-35s=:%d:\n", "Pro client indctr", st_om_rqst.si_pro_client_indicator);
			fprintf(fptr,"%-35s=:%c:\n", "Cover uncover flag",  st_om_rqst.st_add_ord_flg.c_cover_uncover);	
			fprintf(fptr,"%-35s=:%ld:\n", "Order Sequence",  st_om_rqst.li_ordr_sqnc);
			fprintf(fptr,"%-35s=:%lf:\n", "NNF Field", st_om_rqst.d_nnf_field);
		  fn_fprint_string(fptr,"PAN NO                             =",
                        st_om_rqst.c_pan,10);
			fprintf(fptr,"%-35s=:%ld:\n", "Algo Id", st_om_rqst.l_algo_id);
	 		fprintf(fptr,"%-35s=:%d:\n", "Algo Category", st_om_rqst.si_algo_category);
			fprintf(fptr,"%-35s=:%lld:\n", "Last activity reference",st_om_rqst.ll_lastactivityref);
			fn_fprint_string(fptr,"Reserved                           =",
                        st_om_rqst.c_reserved,24);
				
			fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;	
						
    }

    case ORDER_CONFIRMATION_OUT_TR:											/*20073*/
		case ORDER_MOD_CONFIRM_OUT_TR:											/*20074*/
		case ORDER_CXL_CONFIRMATION_TR:											/*20075*/
		{
		  struct st_oe_rspn_tr st_oe_res;
      memset(&st_oe_res, 0, sizeof(st_oe_res));
      memcpy(&st_oe_res,(struct st_oe_rspn_tr *)c_msg_data, sizeof(st_oe_res));
      fn_cnvt_nth_oe_rspn_tr(&st_oe_res); /*** ver 1.4 ***/
	
			fprintf(fptr,"%-35s=:%d:\n", "Transaction Code", st_oe_res.si_transaction_code);
			fprintf(fptr,"%-35s=:%ld:\n", "Log time", st_oe_res.li_log_time);
      fprintf(fptr,"%-35s=:%ld:\n", "User id", st_oe_res.li_userid);
			fprintf(fptr,"%-35s=:%d:\n", "Error code", st_oe_res.si_error_code);

			memcpy(&ll_timestamp1, st_oe_res.c_time_stamp_1,LEN_TIME_STAMP);
			fprintf(fptr, "%-35s=:%lld:\n", "c_timestamp1",ll_timestamp1);

			fprintf(fptr,"%-35s=:%c:\n", "c_time_stamp_2", st_oe_res.c_time_stamp_2);
			fprintf(fptr,"%-35s=:%c:\n", "c_modified_cancelled_by" , st_oe_res.c_modified_cancelled_by);	
			fprintf(fptr,"%-35s=:%d:\n", "Reason code", st_oe_res.si_reason_code);
			fprintf(fptr,"%-35s=:%ld:\n", "Token number" ,st_oe_res.l_token_no);
			fn_fprint_string(fptr,"Instrument Name                    =",
												st_oe_res.st_con_desc_tr.c_instrument_name,LEN_INSTRUMENT_NAME);
			fn_fprint_string(fptr,"Symbol      			                  =",
                        st_oe_res.st_con_desc_tr.c_symbol,LEN_SYMBOL_NSE);
			fprintf(fptr,"%-35s=:%ld:\n", "Expiry date",st_oe_res.st_con_desc_tr.li_expiry_date);
			fprintf(fptr,"%-35s=:%ld:\n", "Strike prize",st_oe_res.st_con_desc_tr.li_strike_price);
			fn_fprint_string(fptr,"Option type                        =",
                        st_oe_res.st_con_desc_tr.c_option_type,LEN_OPTION_TYPE);															
			fprintf(fptr,"%-35s=:%c:\n", "Closeout flag",st_oe_res.c_closeout_flg);
			fprintf(fptr,"%-35s=:%lf:\n", "Order number",st_oe_res.d_order_number);
			fn_fprint_string(fptr,"Account Number                     =",
											  st_oe_res.c_account_number, LEN_ACCOUNT_NUMBER);
			fprintf(fptr,"%-35s=:%d:\n", "Book type",st_oe_res.si_book_type);
			fprintf(fptr,"%-35s=:%d:\n", "buy_sell_indicator",st_oe_res.si_buy_sell_indicator);
			fprintf(fptr,"%-35s=:%ld:\n", "Disclosed volume",st_oe_res.li_disclosed_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "Disclosed volume remaining",st_oe_res.li_disclosed_volume_remaining);
      fprintf(fptr,"%-35s=:%ld:\n", "Total volume remaining",st_oe_res.li_total_volume_remaining);
      fprintf(fptr,"%-35s=:%ld:\n", "Volume", st_oe_res.li_volume);
      fprintf(fptr,"%-35s=:%ld:\n", "Volume filled today",st_oe_res.li_volume_filled_today);
      fprintf(fptr,"%-35s=:%ld:\n", "Prize", st_oe_res.li_price);
      fprintf(fptr,"%-35s=:%ld:\n", "Good till date", st_oe_res.li_good_till_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Entry date time",st_oe_res.li_entry_date_time);
      fprintf(fptr,"%-35s=:%ld:\n", "Last modified", st_oe_res.li_last_modified);
	    fprintf(fptr,"%-35s=:%d:\n", "Order flag ato", st_oe_res.st_ord_flg.flg_ato);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mkt",st_oe_res.st_ord_flg.flg_market);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag sl",st_oe_res.st_ord_flg.flg_sl);
			fprintf(fptr,"%-35s=:%d:\n", "Order flag mit",st_oe_res.st_ord_flg.flg_mit);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag day",st_oe_res.st_ord_flg.flg_day);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag gtc",st_oe_res.st_ord_flg.flg_gtc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ioc",st_oe_res.st_ord_flg.flg_ioc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag aon",st_oe_res.st_ord_flg.flg_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mf",st_oe_res.st_ord_flg.flg_mf);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mtchd ind",st_oe_res.st_ord_flg.flg_matched_ind);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag traded",st_oe_res.st_ord_flg.flg_traded);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag modified",st_oe_res.st_ord_flg.flg_modified);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag frozen",st_oe_res.st_ord_flg.flg_frozen);
      fprintf(fptr,"%-35s=:%d:\n", "Filler1",st_oe_res.st_ord_flg.flg_filler1);
			fprintf(fptr,"%-35s=:%:d\n", "Branch id",st_oe_res.si_branch_id);
			fprintf(fptr,"%-35s=:%ld:\n", "Trader id",st_oe_res.li_trader_id);
			fn_fprint_string(fptr,"Broker id                          =",
												st_oe_res.c_broker_id,LEN_BROKER_ID);
			fprintf(fptr,"%-35s=:%c:\n", "Open Close",st_oe_res.c_open_close);
			fn_fprint_string(fptr,"Settlor                            =",
                        st_oe_res.c_settlor,LEN_SETTLOR);
			fprintf(fptr,"%-35s=:%d:\n", "Pro client indctr",st_oe_res.si_pro_client_indicator);
			fprintf(fptr,"%-35s=:%c:\n", "Cover uncover flag",  st_oe_res.st_add_ord_flg.c_cover_uncover);
			fprintf(fptr,"%-35s=:%ld:\n", "Order Sequence",st_oe_res.li_ordr_sqnc);
			fprintf(fptr,"%-35s=:%lf:\n", "NNF Field",st_oe_res.d_nnf_field);
			fprintf(fptr,"%-35s=:%lld:\n", "Timestamp",st_oe_res.l_timestamp);
			fn_fprint_string(fptr,"PAN NO                             =",
												st_oe_res.c_pan,10);
			fprintf(fptr,"%-35s=:%ld:\n", "Algo Id",st_oe_res.l_algo_id);
			fprintf(fptr,"%-35s=:%d:\n", "Algo Category",st_oe_res.si_algo_category);
			fprintf(fptr,"%-35s=:%lld:\n", "Last activity reference",st_oe_res.ll_lastactivityref);
			fn_fprint_string(fptr,"Reserved                           =",
                        st_oe_res.c_reserved,52);
			fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;								
	
		}

		case TRADE_CONFIRMATION_TR:											/*20222*/
		{
			struct st_trade_confirm_tr st_trd_cnfrm_tr;
      memset(&st_trd_cnfrm_tr, 0, sizeof(st_trd_cnfrm_tr));
      memcpy(&st_trd_cnfrm_tr,(struct st_trade_confirm_tr *)c_msg_data, sizeof(st_trd_cnfrm_tr));
      fn_cnvt_nth_trade_confirm_tr(&st_trd_cnfrm_tr);

			fprintf(fptr,"%-35s=:%d:\n", "Transaction Code",st_trd_cnfrm_tr.si_transaction_code);
			fprintf(fptr,"%-35s=:%ld:\n", "Log time",st_trd_cnfrm_tr.li_log_time);
			fprintf(fptr,"%-35s=:%ld:\n", "Trader number",st_trd_cnfrm_tr.li_trader_number);
			fprintf(fptr,"%-35s=:%lld:\n", "Timestamp",st_trd_cnfrm_tr.ll_timestamp);

			memcpy(&ll_timestamp1, st_trd_cnfrm_tr.c_timestamp1,LEN_TIME_STAMP);
      fprintf(fptr, "%-35s=:%lld:\n", "c_timestamp1",ll_timestamp1);
			
      fprintf(fptr,"%-35s=:%lf:\n", "d_time_stamp2",st_trd_cnfrm_tr.d_timestamp2);
			fprintf(fptr,"%-35s=:%lf:\n", "Response order number",st_trd_cnfrm_tr.d_response_order_number);
			fn_fprint_string(fptr,"Broker id                          =",
												st_trd_cnfrm_tr.c_broker_id,LEN_BROKER_ID);
			fprintf(fptr,"%-35s=:%c:\n", "c_reserved1",st_trd_cnfrm_tr.c_reserved1);
			fn_fprint_string(fptr,"Account Number                     =",
												st_trd_cnfrm_tr.c_account_number,LEN_ACCOUNT_NUMBER);
			fprintf(fptr,"%-35s=:%d:\n","buy_sell_indicator",st_trd_cnfrm_tr.si_buy_sell_indicator);
			fprintf(fptr,"%-35s=:%ld:\n","Original volume",st_trd_cnfrm_tr.li_original_volume);
			fprintf(fptr,"%-35s=:%ld:\n","Disclosed volume",st_trd_cnfrm_tr.li_disclosed_volume);
			fprintf(fptr,"%-35s=:%ld:\n","Remaining volume",st_trd_cnfrm_tr.li_remaining_volume);
			fprintf(fptr,"%-35s=:%ld:\n","Disclosed volume remaining",st_trd_cnfrm_tr.li_disclosed_volume_remaining);
			fprintf(fptr,"%-35s=:%ld:\n", "Prize",st_trd_cnfrm_tr.li_price);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ato",st_trd_cnfrm_tr.st_ord_flg.flg_ato); 
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mkt",st_trd_cnfrm_tr.st_ord_flg.flg_market);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag sl",st_trd_cnfrm_tr.st_ord_flg.flg_sl);
			fprintf(fptr,"%-35s=:%d:\n", "Order flag mit",st_trd_cnfrm_tr.st_ord_flg.flg_mit);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag day",st_trd_cnfrm_tr.st_ord_flg.flg_day);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag gtc",st_trd_cnfrm_tr.st_ord_flg.flg_gtc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag ioc",st_trd_cnfrm_tr.st_ord_flg.flg_ioc);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag aon",st_trd_cnfrm_tr.st_ord_flg.flg_aon);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mf",st_trd_cnfrm_tr.st_ord_flg.flg_mf);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag mtchd ind",st_trd_cnfrm_tr.st_ord_flg.flg_matched_ind);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag traded",st_trd_cnfrm_tr.st_ord_flg.flg_traded);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag modified",st_trd_cnfrm_tr.st_ord_flg.flg_modified);
      fprintf(fptr,"%-35s=:%d:\n", "Order flag frozen",st_trd_cnfrm_tr.st_ord_flg.flg_frozen);
      fprintf(fptr,"%-35s=:%d:\n", "Filler1",st_trd_cnfrm_tr.st_ord_flg.flg_filler1);

			fprintf(fptr,"%-35s=:%ld:\n", "Good till date",st_trd_cnfrm_tr.li_good_till_date);
			fprintf(fptr,"%-35s=:%ld:\n","Fill number",st_trd_cnfrm_tr.li_fill_number);
			fprintf(fptr,"%-35s=:%ld:\n","Fill quantity",st_trd_cnfrm_tr.li_fill_quantity);
			fprintf(fptr,"%-35s=:%ld:\n","Fill prize",st_trd_cnfrm_tr.li_fill_price);
			fprintf(fptr,"%-35s=:%ld:\n","Volume filled today",st_trd_cnfrm_tr.li_volume_filled_today);
				
			fn_fprint_string(fptr,"Activity type                      =",
                        st_trd_cnfrm_tr.c_activity_type,LEN_ACTIVITY_TYPE);
			fprintf(fptr,"%-35s=:%ld:\n","Activity time",st_trd_cnfrm_tr.li_activity_time);
			fprintf(fptr,"%-35s=:%ld:\n","Token",st_trd_cnfrm_tr.l_token);
			fn_fprint_string(fptr,"Instrument Name                    =",
                        st_trd_cnfrm_tr.st_con_desc_tr.c_instrument_name,LEN_INSTRUMENT_NAME);	
			fn_fprint_string(fptr,"Symbol                             =",
                          st_trd_cnfrm_tr.st_con_desc_tr.c_symbol,LEN_SYMBOL_NSE);
      fprintf(fptr,"%-35s=:%ld:\n", "Expiry Date", st_trd_cnfrm_tr.st_con_desc_tr.li_expiry_date);
      fprintf(fptr,"%-35s=:%ld:\n", "Strike Prc", st_trd_cnfrm_tr.st_con_desc_tr.li_strike_price);
      fn_fprint_string(fptr,"Option Type                        =",
                          st_trd_cnfrm_tr.st_con_desc_tr.c_option_type,LEN_OPTION_TYPE);
			fprintf(fptr,"%-35s=:%c:\n", "Open close",st_trd_cnfrm_tr.c_open_close);
			fprintf(fptr,"%-35s=:%c:\n", "Book type",st_trd_cnfrm_tr.c_book_type);
			fn_fprint_string(fptr,"Participant name                   =",		
												st_trd_cnfrm_tr.c_participant,LEN_PARTICIPANT_NAME);
			fprintf(fptr,"%-35s=:%c:\n", "Cover uncover flag",  st_trd_cnfrm_tr.st_add_ord_flg.c_cover_uncover);	
			fn_fprint_string(fptr,"PAN NO                             =",
												st_trd_cnfrm_tr.c_pan,10);
			fprintf(fptr,"%-35s=:%ld:\n", "Algo Id",st_trd_cnfrm_tr.l_algo_id);
      fprintf(fptr,"%-35s=:%d:\n", "Algo Category",st_trd_cnfrm_tr.si_algo_category);
			fprintf(fptr,"%-35s=:%lld:\n", "Last activity reference",st_trd_cnfrm_tr.ll_lastactivityref);		
      fn_fprint_string(fptr,"Reserved2                          =",
                        st_trd_cnfrm_tr.c_reserved2,52);

      fprintf(fptr,"\n%s\n\n", c_end);
      fflush(fptr);
      break;					

		}


    /* ver 1.4 end */


    default:
    {
      fprintf(fptr,"Inside default -- Message code =:%d:\n", li_msg_typ);
      fn_fprint_asc( fptr, c_msg_data, c_ServiceName, c_errmsg );
      fflush(fptr);
      break;
    }
  } /*End of switch(li_msg_type)*/

  return 0;
}

/******************************************************************************/
/* This function takes a char buffer as input and prints every byte of the    */
/* buffer to the file in ascii mode                                           */
/* INPUT PARAMETERS                                                           */
/*      fptr         - File handler                                           */
/*      c_msg_data   - Structure to be printed                                */
/*      c_ServiceName- Name of the service                                    */
/*      c_errmsg     - Error String                                           */
/* OUTPUT PARAMETERS                                                          */
/*          0 - Success                                                       */
/******************************************************************************/
int fn_fprint_asc(FILE *fptr,
                  void *c_msg_data,
                  char *c_ServiceName,
                  char *c_errmsg)
{
  int     i_loop;
  char    c_dname[22];
  char    c_start[20];
  char    c_end[15];
  time_t  tm_stmp;

  tm_stmp = time(NULL);
  strftime(c_dname, 21, "%d-%b-%Y %H:%M:%S", localtime(&tm_stmp));
  fprintf(fptr,"Timestamp :%s:\n", c_dname);

  strcpy(c_start, "Printing New packet");
  strcpy(c_end, "Packet Printed");
  fprintf(fptr,"%s\n", c_start);

  for(i_loop = 0; i_loop < MAX_SCK_MSG; i_loop++)
  {
    fprintf(fptr, ":%d: ", (char *)c_msg_data + i_loop);
    if(i_loop != 0 && ((i_loop + 1) % 16) == 0)
    {
      fprintf(fptr, "\n");
    }
  }
  fprintf(fptr,"\n%s\n\n", c_end);
  fflush(fptr);
  return 0;
}
