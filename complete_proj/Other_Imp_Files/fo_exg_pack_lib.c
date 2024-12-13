/**************************************************************************************************/
/*Ver 1.1  FOR NNF User Id and Trader Id changes																									*/
/*Ver 1.2 For 2L Order replacing OEREMARK1 by integer 																						*/
/*Ver 1.3 FO Auto MTM Changes  | Sachin Birje.        																						*/
/*Ver 1.7 Migration of RI to NRI ( Bhupendra Malik 03-Feb-2016)                                   */
/*Ver 1.8 Ctcl id changes                                                                         */
/*Ver 2.0	NNF_7.21_CHANGES(PAN CHANGES)	|	Parag Kanojia	(04-May-2018)															*/
/*Ver 2.1 rollover with spread chenges                |Mrinal Kishore               |06-Mar-2018  */
/*Ver 2.2 FO Direct Connectivity				| Bhushan Harekar (14-Feb-2019)														*/
/*Ver 2.3 Profut order changes  (14-Feb-2019)														*/
/*Ver 2.4 CR-ISEC14-169739 -Introduction of Algorithm Trading Strategies Sachin Birje   (04-Feb-2022)*/
/*Ver 2.5 Flash Trade Changes (26-Jul-2022)																												*/
/*Ver 2.6 Blueshift Changes(Sachin Birje) (01-Aug-2022)             															*/
/*Ver 2.7 Logs commented Mahesh 																																	*/
/*Ver 2.8 spira 1968 using fn_get_fno_pack_seq funtion to get sequence (Devesh Chawda)            */
/*Ver 2.9 FO GTT Changes (05-May-2023)                         (Edwin)                            */
/*Ver TOL tux on Linux   (03-Aug-2023)                         (Agam)                             */
/**************************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fn_log.h>
#include <fn_md5.h>
#include <fn_msgq.h>
#include <fo.h>
#include <atmi.h>
#include <fn_tuxlib.h>
#include <fn_ddr.h>
#include <fn_scklib_tap.h>
#include <fo_view_def.h>
#include <fn_read_debug_lvl.h>
#include <fo_exg_pack_lib.h>
#include <sqlca.h>
#include <netinet/in.h>				  /*** Ver 2.2 ***/
#include <fn_host_to_nw_format_nse.h> /*** Ver 2.2 ***/
#include <fo_exg_esr_lib.h>			  /*  Added in Ver TOL tux on Linux */
/* #include <fn_ofs_exgcomnN.h>  Added in Ver TOL tux on Linux */

#define FO_FOGTT_ALG_ID 98816 /** Ver 2.9 **/
#define FO_FOGTT_ALG_CAT_ID 1 /** Ver 2.9 **/

EXEC SQL INCLUDE "table/icd_info_client_dtls.h"; /*** Ver 1.7 ***/

int fn_get_fno_pack_seq(char *, struct st_pk_sequence *ptr_st_i_seq, char *); /** added in Ver 2.8 **/
/***********************************************************************************/
/*	Function :fn_sub_seq(Ver1.2)																									 */
/*	iDescription: will return the sequence number from spread order ref no				 */
/***********************************************************************************/
char *fn_sub_seq(char *c_sprd_ord_ref)
{
	char *temp;
	temp = c_sprd_ord_ref + 10;
	return temp;
}
/************************************************************************************************************/
/* Function      : fn_pack_sprdord_to_nse                                                                   */
/*                                                                                                          */
/* Description   : This function picks up data from exchange book, order book, spread book, pipe master,    */
/*                 and NSE contract and fills the relevant information in the exchange spread message       */
/*                 structure st_spd_oe_reqres. This structure is then packed into the st_exch_msg           */
/*                 structure with the sequence number from the FSB table and MD5 digest and is packed       */
/*                 in q_packet.                                                                             */
/*                                                                                                          */
/* INPUT         :                                                                                          */
/*                 - vw_xchngbook                                                                           */
/*                 - vw_orderbook                                                                           */
/*                 - vw_nse_cntrct                                                                          */
/*                 - vw_spdordbk                                                                            */
/*                 - st_opm_pipe_mstr                                                                       */
/*                 - c_ServiceName                                                                          */
/*                                                                                                          */
/* OUTPUT        :                                                                                          */
/*                 - st_req_q_data                                                                          */
/*                                                                                                          */
/* _________________________________________________________________________________________               */
/* | message type | exchange message data                                              		|               */
/* |______________|_________________________________________________________________________|               */
/* | Length       | Sequence Number  | Checksum (MD5) for business data | Business Data  	|               */
/* | (2 Bytes)    | (4 Bytes)        | (16 Bytes)                       | (Variable Length) |               */
/* |______________|__________________|__________________________________|___________________|               */
/*                                                                                                          */
/************************************************************************************************************/


int fn_pack_sprdord_to_nse(struct vw_xchngbook *ptr_st_rqst,			  // input
						   struct vw_orderbook *ptr_st_ord,				  // input
						   struct vw_nse_cntrct *ptr_st_nse_cnt,		  // input
						   struct vw_spdordbk *ptr_st_spd_ord_bk,		  // input
						   struct st_opm_pipe_mstr *ptr_st_opm_pipe_mstr, // input
						   struct st_req_q_data *ptr_st_q_packet,		  // ouput will be stored here
						   char *c_pan_no,								  /*** Added In ver 2.0 ***/
						   char *c_lst_act_ref,							  /*** Ver 2.1 ***/
						   char *c_esp_id,								  /*** Ver 2.4 ***/
						   char *c_algo_id,								  /*** Ver 2.4 ***/
						   char c_source_flg,							  /*** Ver 2.6 ***/
						   char *c_ServiceName)
{
	struct st_spd_oe_reqres st_spdord_req;
	struct st_exch_msg st_exch_message;
	struct st_pk_sequence st_s_sequence; /*** Added in ver 2.8 ***/

	int i_ch_val;
	int i_sprd_flg;
	int i_tmp;
	int i_rec_cnt;
	int i_cnt;
	int i_tmp_cntr;
	int i_snd_seq;
	char c_err_msg[256];
	unsigned char digest[16];
	char c_ordr_ref_no[19]; /*** Ver 1.2 ****/
	char c_user_typ_glb;	/*** Ver 2.1 ***/
	/***************Memset the structures ******************************/

	memset(&st_spdord_req, '\0', sizeof(struct st_spd_oe_reqres));
	memset(&st_exch_message, '\0', sizeof(struct st_exch_msg));
	MEMSET(st_s_sequence); /*** Added in ver 2.8 ***/

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Inside function fn_pack_sprdord_to_nse");
	}

	if (DEBUG_MSG_LVL_3)
	{

		fn_userlog(c_ServiceName, "Order book data");

		fn_userlog(c_ServiceName, "Order book c_pro_cli_ind recieved is :%c:", ptr_st_ord[0].c_pro_cli_ind);
		fn_userlog(c_ServiceName, "Order book c_cln_mtch_accnt recieved is :%s:", ptr_st_ord[0].c_cln_mtch_accnt);
		fn_userlog(c_ServiceName, "Order book c_ordr_flw recieved is :%c:", ptr_st_ord[0].c_ordr_flw);
		fn_userlog(c_ServiceName, "Order book l_ord_tot_qty recieved is :%ld:", ptr_st_ord[0].l_ord_tot_qty);
		fn_userlog(c_ServiceName, "Order book l_exctd_qty recieved is :%ld:", ptr_st_ord[0].l_exctd_qty);
		fn_userlog(c_ServiceName, "Order book l_exctd_qty_day recieved is :%ld:", ptr_st_ord[0].l_exctd_qty_day);
		fn_userlog(c_ServiceName, "Order book c_settlor recieved is :%s:", ptr_st_ord[0].c_settlor);
		fn_userlog(c_ServiceName, "Order book c_ctcl_id recieved is :%s:", ptr_st_ord[0].c_ctcl_id);
		fn_userlog(c_ServiceName, "Order book c_spl_flg recieved is :%c:", ptr_st_ord[0].c_spl_flg); /** Ver 2.1 **/

		fn_userlog(c_ServiceName, "Exchange book data");

		fn_userlog(c_ServiceName, "Exchange book l_dsclsd_qty recieved is :%ld:", ptr_st_rqst[0].l_dsclsd_qty);
		fn_userlog(c_ServiceName, "Exchange book c_slm_flg recieved is :%c:", ptr_st_rqst[0].c_slm_flg);
		fn_userlog(c_ServiceName, "Exchange book l_ord_lmt_rt recieved is :%ld:", ptr_st_rqst[0].l_ord_lmt_rt);
		fn_userlog(c_ServiceName, "Exchange book l_stp_lss_tgr recieved is :%ld:", ptr_st_rqst[0].l_stp_lss_tgr);
		fn_userlog(c_ServiceName, "Exchange book c_ord_typ recieved is :%c:", ptr_st_rqst[0].c_ord_typ);
		fn_userlog(c_ServiceName, "Exchange book l_ord_tot_qty recieved is :%ld:", ptr_st_rqst[0].l_ord_tot_qty);
		fn_userlog(c_ServiceName, "Exchange book c_sprd_ord_ind recieved is :%c:", ptr_st_rqst[0].c_sprd_ord_ind);

		fn_userlog(c_ServiceName, "NSE CONTRACT data");

		fn_userlog(c_ServiceName, "NSE CONTRACT c_prd_typ is :%c:", ptr_st_nse_cnt[0].c_prd_typ);
		fn_userlog(c_ServiceName, "NSE CONTRACT c_ctgry_indstk is :%c:", ptr_st_nse_cnt[0].c_ctgry_indstk);
		fn_userlog(c_ServiceName, "NSE CONTRACT c_symbol is :%s:", ptr_st_nse_cnt[0].c_symbol);
		fn_userlog(c_ServiceName, "NSE CONTRACT l_ca_lvl is :%ld:", ptr_st_nse_cnt[0].l_ca_lvl);
		fn_userlog(c_ServiceName, "NSE CONTRACT l_token_id is :%ld:", ptr_st_nse_cnt[0].l_token_id);

		fn_userlog(c_ServiceName, "OPM data");

		fn_userlog(c_ServiceName, "OPM li_opm_brnch_id recieved is :%ld:", ptr_st_opm_pipe_mstr->li_opm_brnch_id);
		fn_userlog(c_ServiceName, "OPM c_opm_trdr_id recieved is :%s:", ptr_st_opm_pipe_mstr->c_opm_trdr_id);
		fn_userlog(c_ServiceName, "OPM c_xchng_brkr_id recieved is :%s:", ptr_st_opm_pipe_mstr->c_xchng_brkr_id);

		fn_userlog(c_ServiceName, "Spread book  data");
	}
	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Spread book c_sprd_ord_rfrnc recieved is :%s:",
				   ptr_st_spd_ord_bk->c_sprd_ord_rfrnc);
	}

	/********Populate the header of the spread order structure ******************/

	/************ header details START ******************************************/
	/*** fn_orstonse_char ( st_spdord_req.st_hdr.c_filler_1,
						4,
						" ",
						1 ); ** 2nd parameter of function changed from 2 to 4 in Ver 1.1* Commented in Ver 2.2 **/

	st_spdord_req.st_hdr.li_trader_id = (long int)atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id); /*** Ver 2.2 ***/

	st_spdord_req.st_hdr.li_log_time = 0;

	fn_orstonse_char(st_spdord_req.st_hdr.c_alpha_char, LEN_ALPHA_CHAR, " ", );

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "c_sprd_ord_ind recieved is : %s", ptr_st_rqst[0].c_sprd_ord_ind);
	}

	switch (ptr_st_rqst[0].c_sprd_ord_ind)
	{
	case SPREAD_ORDER:
		/*** ver 2.1 Starts Here ***/
		if (ptr_st_rqst[0].c_req_typ == NEW)
		{
			st_spdord_req.st_hdr.si_transaction_code = SP_BOARD_LOT_IN;
		}
		else if (ptr_st_rqst[0].c_req_typ == MODIFY)
		{
			st_spdord_req.st_hdr.si_transaction_code = SP_ORDER_MOD_IN;
		}
		else if (ptr_st_rqst[0].c_req_typ == CANCEL)
		{
			st_spdord_req.st_hdr.si_transaction_code = SP_ORDER_CANCEL_IN;
		}
		else
		{
			fn_errlog(c_ServiceName, "L31005", " Invalid Request Type ", c_err_msg);
			return -1;
		}
		/**** Ver 2.1 Ends Here ***/
		/* st_spdord_req.st_hdr.si_transaction_code = SP_BOARD_LOT_IN; ** Commented in Ver 2.1 ***/
		i_sprd_flg = 2;
		break;

	case L2_ORDER:
		st_spdord_req.st_hdr.si_transaction_code = TWOL_BOARD_LOT_IN;
		i_sprd_flg = 2;
		break;

	case L3_ORDER:
		st_spdord_req.st_hdr.si_transaction_code = THRL_BOARD_LOT_IN;
		i_sprd_flg = 3;
		break;

	default:
		fn_errlog(c_ServiceName, "L31010", " Invalid Message Type ", c_err_msg);
		return -1;
		break;
	}

	st_spdord_req.st_hdr.si_error_code = 0;

	fn_orstonse_char(st_spdord_req.st_hdr.c_filler_2, 8, " ", 1);

	memset(st_spdord_req.st_hdr.c_time_stamp_1, 0, LEN_TIME_STAMP);

	memset(st_spdord_req.st_hdr.c_time_stamp_2, 0, LEN_TIME_STAMP);

	st_spdord_req.st_hdr.si_message_length = sizeof(struct st_spd_oe_reqres);

	/************ header details END ******************************************/

	st_spdord_req.c_participant_type = ' ';

	st_spdord_req.c_filler1 = ' ';

	st_spdord_req.si_competitor_period = 0;

	st_spdord_req.si_solicitor_period = 0;

	/**** st_spdord_req.c_mod_cxl_by = ' '; **** Comented in Ver 2.1 ***/
	/*** Ver 2.1 Starts here ****/
	switch (ptr_st_opm_pipe_mstr->si_user_typ_glb)
	{
	case 0:
		c_user_typ_glb = TRADER; // c_user_typ_glb is a local variable
		break;

	case 4:
		c_user_typ_glb = CORPORATE_MANAGER;
		break;

	case 5:
		c_user_typ_glb = BRANCH_MANAGER;
		break;

	default:
		fn_errlog(c_ServiceName, "L31015", " Invalid USER Type ", c_err_msg);
		return -1;
		break;
	}

	if (ptr_st_rqst[0].c_req_typ == NEW)
	{
		st_spdord_req.c_mod_cxl_by = ' ';
	}
	else
	{
		st_spdord_req.c_mod_cxl_by = c_user_typ_glb;
	}

	/**** Ver 2.1 ends Here ****/

	st_spdord_req.c_filler2 = ' ';

	st_spdord_req.si_reason_code = 0;

	fn_orstonse_char(st_spdord_req.c_start_alpha,
					 LEN_ALPHA_CHAR,
					 " ",
					 1);

	fn_orstonse_char(st_spdord_req.c_end_alpha,
					 LEN_ALPHA_CHAR,
					 " ",
					 1);

	st_spdord_req.l_token = ptr_st_nse_cnt[0].l_token_id;

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "TOKEN in Spread order placement is :%ld: :%ld:",
				   st_spdord_req.l_token, ptr_st_nse_cnt[0].l_token_id);
	}

	/******* Convert NSE contract structure to NSE contract description *******/

	fn_orstonse_cntrct_desc((struct vw_nse_cntrct *)&ptr_st_nse_cnt[0],
							&st_spdord_req.st_cntrct_desc,
							c_ServiceName,
							c_err_msg);

	fn_orstonse_char(st_spdord_req.c_op_broker_id,
					 LEN_BROKER_ID,
					 " ",
					 1);

	st_spdord_req.c_filler3 = ' ';

	fn_orstonse_char(st_spdord_req.c_filler_options,
					 LEN_FILLER_OPTIONS,
					 " ",
					 1);

	st_spdord_req.c_filler4 = ' ';

	st_spdord_req.si_order_type = 0;

	st_spdord_req.d_order_number = 0;
	/*** Ver 2.1 Starts Here ****/
	if (ptr_st_rqst[0].c_req_typ == NEW)
	{
		st_spdord_req.d_order_number = 0;
	}
	else
	{
		st_spdord_req.d_order_number = (double)atoll(ptr_st_ord[0].c_xchng_ack);
	}

	/*** Ver 2.1 Ends Here ****/

	if (ptr_st_ord[0].c_pro_cli_ind == BRKR_PLCD)
	{
		fn_orstonse_char(st_spdord_req.c_account_number,
						 LEN_ACCOUNT_NUMBER,
						 " ",
						 1);
	}
	else
	{
		fn_orstonse_char(st_spdord_req.c_account_number,
						 LEN_ACCOUNT_NUMBER,
						 ptr_st_ord[0].c_cln_mtch_accnt,
						 strlen(ptr_st_ord[0].c_cln_mtch_accnt));
	}

	switch (ptr_st_rqst[0].c_slm_flg)
	{
	case 'S':
		st_spdord_req.si_book_type = STOP_LOSS_MIT_ORDER;
		break;

	case 'L':
	case 'M':
		st_spdord_req.si_book_type = REGULAR_LOT_ORDER;
		break;

	default:
		fn_errlog(c_ServiceName, "L31020", " Invalid Slm Flag ", c_err_msg);
		return -1;
		break;
	}

	switch (ptr_st_ord[0].c_ordr_flw)
	{
	case 'B':
		st_spdord_req.si_buy_sell = NSE_BUY;
		break;

	case 'S':
		st_spdord_req.si_buy_sell = NSE_SELL;
		break;

	default:
		fn_errlog(c_ServiceName, "L31025", " Invalid Order Flag ", c_err_msg);
		return -1;
		break;
	}

	st_spdord_req.li_disclosed_vol = ptr_st_rqst[0].l_dsclsd_qty;

	st_spdord_req.li_disclosed_vol_remaining = st_spdord_req.li_disclosed_vol;

	if (ptr_st_rqst[0].l_ord_tot_qty > ptr_st_ord[0].l_ord_tot_qty)
	{
		st_spdord_req.li_total_vol_remaining =
			(ptr_st_ord[0].l_ord_tot_qty - ptr_st_ord[0].l_exctd_qty);
	}
	else
	{
		st_spdord_req.li_total_vol_remaining =
			(ptr_st_rqst[0].l_ord_tot_qty - ptr_st_ord[0].l_exctd_qty);
	}

	st_spdord_req.li_volume =
		ptr_st_rqst[0].l_ord_tot_qty - ptr_st_ord[0].l_exctd_qty;

	st_spdord_req.li_volume_filled_today = ptr_st_ord[0].l_exctd_qty_day;

	switch (ptr_st_rqst[0].c_slm_flg)
	{
	case 'M':
		st_spdord_req.li_price = 0;
		break;

	case 'L':
	case 'S':
		/** Added in Ver 2.1 **/
		if (ptr_st_ord[0].c_spl_flg == 'O')
		{
			if (DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName, "Inside rollover with spread c_spl_flg is :%c:", ptr_st_ord[0].c_spl_flg);
			}
			st_spdord_req.li_price = 0;
		}
		else
		{
			/** Ver 2.1 Ends **/
			st_spdord_req.li_price = ptr_st_rqst[0].l_ord_lmt_rt;
		} /*** Ver 2.1 ***/
		break;

	default:
		fn_errlog(c_ServiceName, "L31030", " Invalid Slm Flag ", c_err_msg);
		return -1;
		break;
	}

	st_spdord_req.li_trigger_price = ptr_st_rqst[0].l_stp_lss_tgr;

	st_spdord_req.li_good_till_date = 0;

	/***  st_spdord_req.li_entry_date_time = 0; *** ver 2.1 ***/
	/*** Ver 2.1 Starts Here ****/
	if (ptr_st_rqst[0].c_req_typ == NEW)
	{
		st_spdord_req.li_entry_date_time = 0;
	}
	else
	{
		fn_timearr_to_long(ptr_st_ord[0].c_ack_tm, &(st_spdord_req.li_entry_date_time));
	}
	/**** Ver 2.1 Ends Here ****/

	st_spdord_req.li_min_fill_aon = 0;

	/*** st_spdord_req.li_last_modified = 0;  *** Ver 2.1 ***/
	/*** Ver 2.1 Starts Here ****/
	if (ptr_st_rqst[0].c_req_typ == NEW)
	{
		st_spdord_req.li_last_modified = 0;
	}
	else
	{
		fn_timearr_to_long(ptr_st_ord[0].c_prev_ack_tm, &(st_spdord_req.li_last_modified));
	}
	/**** Ver 2.1 Ends Here ***/

	st_spdord_req.st_order_flgs.flg_ato = 0;

	st_spdord_req.st_order_flgs.flg_market = 0;

	st_spdord_req.st_order_flgs.flg_sl = 0;

	st_spdord_req.st_order_flgs.flg_mit = 0;

	if (ptr_st_rqst[0].c_ord_typ == 'T')
	{
		st_spdord_req.st_order_flgs.flg_day = 1;
	}
	else
	{
		st_spdord_req.st_order_flgs.flg_day = 0;
	}

	st_spdord_req.st_order_flgs.flg_gtc = 0;

	if (ptr_st_rqst[0].c_ord_typ == 'I')
	{
		st_spdord_req.st_order_flgs.flg_ioc = 1;
	}
	else
	{
		st_spdord_req.st_order_flgs.flg_ioc = 0;
	}

	st_spdord_req.st_order_flgs.flg_aon = 0;

	st_spdord_req.st_order_flgs.flg_mf = 0;

	st_spdord_req.st_order_flgs.flg_matched_ind = 0;

	st_spdord_req.st_order_flgs.flg_traded = 0;

	/*** st_spdord_req.st_order_flgs.flg_modified  = 0; *** ver 2.1 ***/
	/*** Ver 2.1 Starts Here ****/
	if (ptr_st_rqst[0].c_req_typ == MODIFY)
	{
		st_spdord_req.st_order_flgs.flg_modified = 1;
	}
	else
	{
		st_spdord_req.st_order_flgs.flg_modified = 0;
	}

	if (ptr_st_rqst[0].c_req_typ == CANCEL || ptr_st_rqst[0].c_req_typ == MODIFY)
	{
		st_spdord_req.ll_lastactivityref = atoll(c_lst_act_ref);
		fn_userlog(c_ServiceName, " Testing LAST ACT REF PACK LIB :%s: :%lld:", c_lst_act_ref, st_spdord_req.ll_lastactivityref);
	}
	else
	{
		st_spdord_req.ll_lastactivityref = 0;
	}
	/**** Ver 2.1 Ends Here ***/

	st_spdord_req.st_order_flgs.flg_frozen = 0;

	st_spdord_req.st_order_flgs.flg_filler1 = 0;

	st_spdord_req.si_branch_id = ptr_st_opm_pipe_mstr->li_opm_brnch_id;

	/***   Ver 1.1  **/
	/**	st_spdord_req.si_trader_id =(short) atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id); **/
	st_spdord_req.li_trader_id = (long)atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id);
	/*** Ver 1.1 ends ***/

	fn_orstonse_char(st_spdord_req.c_broker_id,
					 LEN_BROKER_ID,
					 ptr_st_opm_pipe_mstr->c_xchng_brkr_id,
					 strlen(ptr_st_opm_pipe_mstr->c_xchng_brkr_id));
	/****** Ver 1.2 Comment to replace OEREMARKS by integer ***
	fn_orstonse_char (st_spdord_req.c_oe_remarks,
										LEN_REMARKS,
										(char *)ptr_st_spd_ord_bk->c_sprd_ord_rfrnc,
										LEN_ORDR_RFRNC );
	****** Ver 1.2 Comment Ends ********/

	/*** Ver 1.2 New Code Starts ***/

	MEMSET(c_ordr_ref_no);
	strcpy(c_ordr_ref_no, (char *)ptr_st_spd_ord_bk->c_sprd_ord_rfrnc);

	st_spdord_req.i_sprd_seq_no = atoi(fn_sub_seq(c_ordr_ref_no));

	fn_userlog(c_ServiceName, " seq no to nse i_sprd_seq_no=:%d:", st_spdord_req.i_sprd_seq_no);

	/*** Ver 1.2 New Code Ends ****/

	st_spdord_req.c_open_close = 'O';

	memset(st_spdord_req.c_settlor, 0, LEN_SETTLOR);
	/** Commented in Ver 1.7 ***
	if (strncmp(ptr_st_ord[0].c_cln_mtch_accnt,"751",3)!=0 &&
			strncmp(ptr_st_ord[0].c_cln_mtch_accnt,"651",3)!=0 &&
			strncmp(ptr_st_ord[0].c_cln_mtch_accnt,"750",3)!=0 &&
			strncmp(ptr_st_ord[0].c_cln_mtch_accnt,"650",3)!=0 ) ***/
	/*** Ver 1.7 Starts ***/
	MEMSET(sql_icd_cust_type);
	EXEC SQL
		SELECT ICD_CUST_TYPE
			INTO : sql_icd_cust_type
					   FROM ICD_INFO_CLIENT_DTLS,
				   IAI_INFO_ACCOUNT_INFO
					   WHERE ICD_SERIAL_NO = IAI_SERIAL_NO
						   AND IAI_MATCH_ACCOUNT_NO = : ptr_st_ord[0].c_cln_mtch_accnt;

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "L31035", SQLMSG, c_err_msg);
		return -1;
	}
	SETNULL(sql_icd_cust_type);
	/*********  Ver 1.7 Ends ************/
	if (strcmp(sql_icd_cust_type.arr, "NRI") != 0) /****** Check added in Ver 1.7 *******/
	{

		if (ptr_st_ord[0].c_pro_cli_ind == BRKR_PLCD)
		{
			fn_orstonse_char(st_spdord_req.c_settlor,
							 LEN_SETTLOR,
							 " ",
							 1);

			st_spdord_req.si_pro_client = NSE_PRO;
		}
		else
		{
			fn_orstonse_char(st_spdord_req.c_settlor,
							 LEN_SETTLOR,
							 ptr_st_opm_pipe_mstr->c_xchng_brkr_id,
							 strlen(ptr_st_opm_pipe_mstr->c_xchng_brkr_id));

			st_spdord_req.si_pro_client = NSE_CLIENT;
		}
	}
	else
	{
		fn_orstonse_char(st_spdord_req.c_settlor,
						 LEN_SETTLOR,
						 ptr_st_ord[0].c_settlor,
						 strlen(ptr_st_ord[0].c_settlor));

		st_spdord_req.si_pro_client = NSE_CLIENT;
	}

	/*** Ver 2.0 Starts ***/
	if (strcmp(c_pan_no, "*") != 0)
	{
		fn_orstonse_char(st_spdord_req.c_pan,
						 LEN_PAN,
						 c_pan_no,
						 strlen(c_pan_no));
	}
	else
	{
		fn_orstonse_char(st_spdord_req.c_pan,
						 LEN_PAN,
						 " ",
						 1);
	}
	/*** Ver 2.0 Ends ***/

	/***** Since there is no provision for settlement period in EBA we set this to 0 .
				   In future if need arises, this variable will be set as per the requirements**/
	st_spdord_req.si_settlement_period = 0;

	st_spdord_req.c_cover_uncover = 'V';

	st_spdord_req.c_give_up_flag = ' ';

	/********* Ver 1.2 Comment starts ***
	st_spdord_req.us_filler1 = 0;

	st_spdord_req.us_filler2 = 0;

	st_spdord_req.us_filler3 = 0;

	st_spdord_req.us_filler4 = 0;

	st_spdord_req.us_filler5 = 0;

	st_spdord_req.us_filler6 = 0;

	st_spdord_req.us_filler7 = 0;

	st_spdord_req.us_filler8 = 0;

	st_spdord_req.us_filler9 = 0;

	st_spdord_req.us_filler10 = 0;

	st_spdord_req.us_filler11 = 0;

	st_spdord_req.us_filler12 = 0;

	st_spdord_req.us_filler13 = 0;

	st_spdord_req.us_filler14 = 0;

	st_spdord_req.us_filler15 = 0;

	st_spdord_req.us_filler16 = 0;

	st_spdord_req.c_filler17 = ' ';

	st_spdord_req.c_filler18 = ' ';

	***** Ber 1.2 Comment Ends ****/
	/**** 1.9 DBC Complaince ******/

	/*** st_spdord_req.d_nnf_field = 111111111111100; ***/

	strcat(ptr_st_ord[0].c_ctcl_id, "100");
	st_spdord_req.d_nnf_field = (double)atoll(ptr_st_ord[0].c_ctcl_id);

	/**** 1.9 DBC Complaince END******/

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "The d_nnf_field set is [%lf]", st_spdord_req.d_nnf_field);
	}

	st_spdord_req.d_filler19 = 0.0;

	/*** Added in Ver 2.1 ***/
	if (ptr_st_ord[0].c_spl_flg == 'O')
	{
		if (DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName, "Inside rollover with spread c_spl_flg is :%c:", ptr_st_ord[0].c_spl_flg);
		}
		st_spdord_req.li_spd_price_diff = ptr_st_rqst[0].l_ord_lmt_rt;
	}
	else
	{
		/**** Ver 2.1 Ends Here ***/
		st_spdord_req.li_spd_price_diff = 0;
	} /*** Ver 2.1 ***/

	/*** Ver 2.0 Starts ***/

	st_spdord_req.l_algo_id = FO_NON_ALG_ID;
	st_spdord_req.si_algo_category = FO_NON_ALG_CAT_ID;

	/*** st_spdord_req.ll_lastactivityref = 0;	*** Ver 2.2 *** Commented in Ver 2.1 **/

	fn_orstonse_char(st_spdord_req.c_reserved,
					 52, /*** Changes from 60 to 52 in Ver 2.2 ***/
					 " ",
					 1);

	/*** Ver 2.0 Ends ***/

	if (ptr_st_rqst[0].c_sprd_ord_ind == L3_ORDER)
	{
		i_rec_cnt = 2;
	}
	else
	{
		i_rec_cnt = 1;
	}

	for (i_cnt = 1; i_cnt <= i_rec_cnt; i_cnt++)
	{

		if (DEBUG_MSG_LVL_3)
		{

			fn_userlog(c_ServiceName, "Order book data element %d", i_cnt);

			fn_userlog(c_ServiceName, "Order book c_pro_cli_ind recieved is :%c:", ptr_st_ord[i_cnt].c_pro_cli_ind);
			fn_userlog(c_ServiceName, "Order book c_cln_mtch_accnt recieved is :%s:",
					   ptr_st_ord[i_cnt].c_cln_mtch_accnt);
			fn_userlog(c_ServiceName, "Order book c_ordr_flw recieved is :%c:", ptr_st_ord[i_cnt].c_ordr_flw);
			fn_userlog(c_ServiceName, "Order book l_ord_tot_qty recieved is :%ld:", ptr_st_ord[i_cnt].l_ord_tot_qty);
			fn_userlog(c_ServiceName, "Order book l_exctd_qty recieved is :%ld:", ptr_st_ord[i_cnt].l_exctd_qty);
			fn_userlog(c_ServiceName, "Order book l_exctd_qty_day recieved is :%ld:",
					   ptr_st_ord[i_cnt].l_exctd_qty_day);
			fn_userlog(c_ServiceName, "Order book c_settlor recieved is :%s:", ptr_st_ord[i_cnt].c_settlor);
			fn_userlog(c_ServiceName, "Order book c_ctcl_id recieved is :%s:", ptr_st_ord[i_cnt].c_ctcl_id);
			fn_userlog(c_ServiceName, "Order book c_spl_flg rcvd is :%c:", ptr_st_ord[i_cnt].c_spl_flg); /* Ver 2.1 */

			fn_userlog(c_ServiceName, "Exchange book element %d", i_cnt);

			fn_userlog(c_ServiceName, "Exchange book l_dsclsd_qty recieved is :%ld:", ptr_st_rqst[i_cnt].l_dsclsd_qty);
			fn_userlog(c_ServiceName, "Exchange book c_slm_flg recieved is :%c:", ptr_st_rqst[i_cnt].c_slm_flg);
			fn_userlog(c_ServiceName, "Exchange book l_ord_lmt_rt recieved is :%ld:", ptr_st_rqst[i_cnt].l_ord_lmt_rt);
			fn_userlog(c_ServiceName, "Exchange book l_stp_lss_tgr recieved is :%ld:",
					   ptr_st_rqst[i_cnt].l_stp_lss_tgr);
			fn_userlog(c_ServiceName, "Exchange book c_ord_typ recieved is :%s:", ptr_st_rqst[i_cnt].c_ord_typ);
			fn_userlog(c_ServiceName, "Exchange book l_ord_tot_qty recieved is :%ld:",
					   ptr_st_rqst[i_cnt].l_ord_tot_qty);
			fn_userlog(c_ServiceName, "Exchange book c_sprd_ord_ind recieved is :%c:",
					   ptr_st_rqst[i_cnt].c_sprd_ord_ind);

			fn_userlog(c_ServiceName, "Printing exchange message structure element %d", i_cnt);

			fn_userlog(c_ServiceName, "NSE CONTRACT c_prd_typ is :%c:", ptr_st_nse_cnt[i_cnt].c_prd_typ);
			fn_userlog(c_ServiceName, "NSE CONTRACT c_ctgry_indstk is :%c:", ptr_st_nse_cnt[i_cnt].c_ctgry_indstk);
			fn_userlog(c_ServiceName, "NSE CONTRACT c_symbol is :%s:", ptr_st_nse_cnt[i_cnt].c_symbol);
			fn_userlog(c_ServiceName, "NSE CONTRACT l_ca_lvl is :%ld:", ptr_st_nse_cnt[i_cnt].l_ca_lvl);
		}

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].l_token = ptr_st_nse_cnt[i_cnt].l_token_id;

		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "NSE CONTRACT l_token_id recieved is : %ld", ptr_st_nse_cnt[i_cnt].l_token_id);
		}

		/* Convert ORS contract to NSE contract description */
		fn_orstonse_cntrct_desc((struct vw_nse_cntrct *)&ptr_st_nse_cnt[i_cnt],
								&st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_cntrct_desc,
								c_ServiceName,
								c_err_msg);

		fn_orstonse_char(st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_op_broker_id,
						 LEN_BROKER_ID,
						 " ",
						 1);

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_filler1 = ' ';

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].si_order_type = 0;

		switch (ptr_st_ord[i_cnt].c_ordr_flw)
		{
		case 'B':
			st_spdord_req.st_spd_lg_inf[i_cnt - 1].si_buy_sell = NSE_BUY;
			break;

		case 'S':
			st_spdord_req.st_spd_lg_inf[i_cnt - 1].si_buy_sell = NSE_SELL;
			break;

		default:
			fn_errlog(c_ServiceName, "L31040", " Invalid Order Flag ", c_err_msg);
			return -1;
			break;
		}

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_disclosed_vol = ptr_st_rqst[i_cnt].l_dsclsd_qty;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_disclosed_vol_remaining =
			st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_disclosed_vol;

		if (ptr_st_rqst[i_cnt].l_ord_tot_qty > ptr_st_ord[i_cnt].l_ord_tot_qty)
		{
			st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_total_vol_remaining =
				(ptr_st_ord[i_cnt].l_ord_tot_qty - ptr_st_ord[i_cnt].l_exctd_qty);
		}
		else
		{
			st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_total_vol_remaining =
				(ptr_st_rqst[i_cnt].l_ord_tot_qty - ptr_st_ord[i_cnt].l_exctd_qty);
		}

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_volume =
			(ptr_st_rqst[i_cnt].l_ord_tot_qty - ptr_st_ord[i_cnt].l_exctd_qty);

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_volume_filled_today = ptr_st_ord[i_cnt].l_exctd_qty_day;

		switch (ptr_st_rqst[i_cnt].c_slm_flg)
		{
		case 'M':
			st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_price = 0;
			break;

		case 'L':
		case 'S':
			/** Added in Ver 2.1 **/
			if (ptr_st_ord[i_cnt].c_spl_flg == 'O')
			{
				if (DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName, "Inside rollover with spread c_spl_flg is :%c:", ptr_st_ord[i_cnt].c_spl_flg);
				}
				st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_price = 0;
			}
			else
			{
				/** Ver 2.1 Add Ends **/
				st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_price =
					ptr_st_rqst[i_cnt].l_ord_lmt_rt;
			} /*** Ver 2.1 ***/
			break;

		default:
			fn_errlog(c_ServiceName, "L31045", " Invalid Slm Flag ", c_err_msg);
			return -1;
			break;
		}

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_trigger_price = ptr_st_rqst[i_cnt].l_stp_lss_tgr;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_min_fill_aon = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_ato = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_market = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_sl = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_mit = 0;

		if (ptr_st_rqst[i_cnt].c_ord_typ == 'T')
		{
			st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_day = 1;
		}
		else
		{
			st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_day = 0;
		}

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_gtc = 0;

		if (ptr_st_rqst[i_cnt].c_ord_typ == 'I')
		{
			st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_ioc = 1;
		}
		else
		{
			st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_ioc = 0;
		}

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_aon = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_mf = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_matched_ind = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_traded = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_modified = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_frozen = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_filler1 = 0;

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_open_close = 'O';

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_cover_uncover = 'V';

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_giveup_flag = ' ';

		st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_filler2 = ' ';
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "**********Printing exchange message structure**********");

		fn_userlog(c_ServiceName, "**Printing the header structure st_spdord_req. st_hdr**");
		/**fn_userlog(c_ServiceName,"c_filler_1 is :%s:",st_spdord_req.st_hdr.c_filler_1);**Commented in Ver 2.2 **/
		fn_userlog(c_ServiceName, "li_trader_id :%ld:", st_spdord_req.st_hdr.li_trader_id); /** Ver 2.2 **/
		fn_userlog(c_ServiceName, " li_log_time is :%ld:", st_spdord_req.st_hdr.li_log_time);
		fn_userlog(c_ServiceName, "c_alpha_char is :%s:", st_spdord_req.st_hdr.c_alpha_char);
		fn_userlog(c_ServiceName, "si_transaction_code is :%d:", st_spdord_req.st_hdr.si_transaction_code);
		fn_userlog(c_ServiceName, "si_error_code is :%d:", st_spdord_req.st_hdr.si_error_code);
		fn_userlog(c_ServiceName, "c_filler_2 is :%s:", st_spdord_req.st_hdr.c_filler_2);
		fn_userlog(c_ServiceName, "c_time_stamp_1 is :%s:", st_spdord_req.st_hdr.c_time_stamp_1);
		fn_userlog(c_ServiceName, "c_time_stamp_2 is :%s:", st_spdord_req.st_hdr.c_time_stamp_2);
		fn_userlog(c_ServiceName, "si_message_length is :%d:", st_spdord_req.st_hdr.si_message_length);
		fn_userlog(c_ServiceName, "**Printing the header structure st_spdord_req. st_hdr ENDS**");

		fn_userlog(c_ServiceName, "c_participant_type is :%c:", st_spdord_req.c_participant_type);
		fn_userlog(c_ServiceName, "c_filler1 is :%c:", st_spdord_req.c_filler1);
		fn_userlog(c_ServiceName, "si_competitor_period is :%d:", st_spdord_req.si_competitor_period);
		fn_userlog(c_ServiceName, "si_solicitor_period is :%d:", st_spdord_req.si_solicitor_period);
		fn_userlog(c_ServiceName, "c_mod_cxl_by is :%c:", st_spdord_req.c_mod_cxl_by);
		fn_userlog(c_ServiceName, "c_filler2 is :%c:", st_spdord_req.c_filler2);
		fn_userlog(c_ServiceName, "si_reason_code is :%d:", st_spdord_req.si_reason_code);
		fn_userlog(c_ServiceName, "c_start_alpha is :%s:", st_spdord_req.c_start_alpha);
		fn_userlog(c_ServiceName, "c_end_alpha is :%s:", st_spdord_req.c_end_alpha);
		fn_userlog(c_ServiceName, "l_token is :%ld:", st_spdord_req.l_token);

		fn_userlog(c_ServiceName, "*************Printing contract_desc structure*********");
		fn_userlog(c_ServiceName, "c_instrument_name is :%s:", st_spdord_req.st_cntrct_desc.c_instrument_name);
		fn_userlog(c_ServiceName, "c_symbol is :%s:", st_spdord_req.st_cntrct_desc.c_symbol);
		fn_userlog(c_ServiceName, "li_expiry_date is :%ld:", st_spdord_req.st_cntrct_desc.li_expiry_date);
		fn_userlog(c_ServiceName, "li_strike_price is :%ld:", st_spdord_req.st_cntrct_desc.li_strike_price);
		fn_userlog(c_ServiceName, "c_option_type is :%s:", st_spdord_req.st_cntrct_desc.c_option_type);
		fn_userlog(c_ServiceName, "si_ca_level is :%d:", st_spdord_req.st_cntrct_desc.si_ca_level);
		fn_userlog(c_ServiceName, "*************Printing contract_desc structure ENDS*********");

		fn_userlog(c_ServiceName, "c_op_broker_id is :%s:", st_spdord_req.c_op_broker_id);
		fn_userlog(c_ServiceName, "c_filler3 is :%c:", st_spdord_req.c_filler3);
		fn_userlog(c_ServiceName, "c_filler_options is :%s:", st_spdord_req.c_filler_options);
		fn_userlog(c_ServiceName, "c_filler4 is :%c:", st_spdord_req.c_filler4);
		fn_userlog(c_ServiceName, "si_order_type is :%d:", st_spdord_req.si_order_type);
		fn_userlog(c_ServiceName, "d_order_number is :%ld:", st_spdord_req.d_order_number);
		fn_userlog(c_ServiceName, "c_account_number is :%s:", st_spdord_req.c_account_number);
		fn_userlog(c_ServiceName, "si_book_type is :%d:", st_spdord_req.si_book_type);
		fn_userlog(c_ServiceName, "si_buy_sell is :%d:", st_spdord_req.si_buy_sell);
		fn_userlog(c_ServiceName, "li_disclosed_vol is :%ld:", st_spdord_req.li_disclosed_vol);
		fn_userlog(c_ServiceName, "li_disclosed_vol_remaining is :%ld:", st_spdord_req.li_disclosed_vol_remaining);
		fn_userlog(c_ServiceName, "li_total_vol_remaining is :%ld:", st_spdord_req.li_disclosed_vol_remaining);
		fn_userlog(c_ServiceName, "li_volume is :%ld:", st_spdord_req.li_volume);
		fn_userlog(c_ServiceName, "li_volume_filled_today is :%ld:", st_spdord_req.li_volume_filled_today);
		fn_userlog(c_ServiceName, "li_price is :%ld:", st_spdord_req.li_price);
		fn_userlog(c_ServiceName, "li_trigger_price is :%ld:", st_spdord_req.li_trigger_price);
		fn_userlog(c_ServiceName, "li_good_till_date is :%ld:", st_spdord_req.li_good_till_date);
		fn_userlog(c_ServiceName, "li_entry_date_time is :%ld:", st_spdord_req.li_entry_date_time);
		fn_userlog(c_ServiceName, "li_min_fill_aon is :%ld:", st_spdord_req.li_min_fill_aon);
		fn_userlog(c_ServiceName, "li_last_modified is :%ld:", st_spdord_req.li_last_modified);

		fn_userlog(c_ServiceName, "**********Printing order flags structure**************");
		fn_userlog(c_ServiceName, "flg_ato is :%d:", st_spdord_req.st_order_flgs.flg_ato);
		fn_userlog(c_ServiceName, "flg_market is :%d:", st_spdord_req.st_order_flgs.flg_market);
		fn_userlog(c_ServiceName, "flg_sl is :%d:", st_spdord_req.st_order_flgs.flg_sl);
		fn_userlog(c_ServiceName, "flg_mit is :%d:", st_spdord_req.st_order_flgs.flg_mit);
		fn_userlog(c_ServiceName, "flg_day is :%d:", st_spdord_req.st_order_flgs.flg_day);
		fn_userlog(c_ServiceName, "flg_gtc is :%d:", st_spdord_req.st_order_flgs.flg_gtc);
		fn_userlog(c_ServiceName, "flg_ioc is :%d:", st_spdord_req.st_order_flgs.flg_ioc);
		fn_userlog(c_ServiceName, "flg_aon is :%d:", st_spdord_req.st_order_flgs.flg_aon);
		fn_userlog(c_ServiceName, "flg_mf is :%d:", st_spdord_req.st_order_flgs.flg_mf);
		fn_userlog(c_ServiceName, "flg_matched_ind is :%d:", st_spdord_req.st_order_flgs.flg_matched_ind);
		fn_userlog(c_ServiceName, "flg_traded is :%d:", st_spdord_req.st_order_flgs.flg_traded);
		fn_userlog(c_ServiceName, "flg_modified is :%d:", st_spdord_req.st_order_flgs.flg_modified);
		fn_userlog(c_ServiceName, "flg_frozen is :%d:", st_spdord_req.st_order_flgs.flg_frozen);
		fn_userlog(c_ServiceName, "flg_filler1 is :%d:", st_spdord_req.st_order_flgs.flg_filler1);
		fn_userlog(c_ServiceName, "**********Printing order flags structure ENDS**************");

		fn_userlog(c_ServiceName, "si_branch_id is :%d:", st_spdord_req.si_branch_id);
		fn_userlog(c_ServiceName, "li_trader_id is :%ld:", st_spdord_req.li_trader_id);
		fn_userlog(c_ServiceName, "c_broker_id is :%s:", st_spdord_req.c_broker_id);
		fn_userlog(c_ServiceName, "c_oe_remarks is :%s:", st_spdord_req.c_oe_remarks);
		fn_userlog(c_ServiceName, "c_open_close is :%c:", st_spdord_req.c_open_close);
		fn_userlog(c_ServiceName, "c_settlor is :%s:", st_spdord_req.c_settlor);
		fn_userlog(c_ServiceName, "si_pro_client is :%d:", st_spdord_req.si_pro_client);
		fn_userlog(c_ServiceName, "si_settlement_period is :%d:", st_spdord_req.si_settlement_period);
		fn_userlog(c_ServiceName, "c_cover_uncover is :%c:", st_spdord_req.c_cover_uncover);
		fn_userlog(c_ServiceName, "c_give_up_flag is :%c:", st_spdord_req.c_give_up_flag);
		/*fn_userlog(c_ServiceName,"c_filler17 is :%c:",st_spdord_req.c_filler17);*** Ver 1.2 **/
		/*fn_userlog(c_ServiceName,"c_filler18 is :%c:",st_spdord_req.c_filler18);*** Ver 1.2 **/
		fn_userlog(c_ServiceName, "d_nnf_field is :%ld:", st_spdord_req.d_nnf_field);
		fn_userlog(c_ServiceName, "d_filler19 is :%ld:", st_spdord_req.d_filler19);
		fn_userlog(c_ServiceName, "li_spd_price_diff is :%ld:", st_spdord_req.li_spd_price_diff);
		/*** Ver 2.0 Starts ***/
		fn_userlog(c_ServiceName, "c_pan is :%s:", st_spdord_req.c_pan);
		fn_userlog(c_ServiceName, "l_algo_id is :%ld:", st_spdord_req.l_algo_id);
		fn_userlog(c_ServiceName, "si_algo_category is :%d:", st_spdord_req.si_algo_category);
		fn_userlog(c_ServiceName, "c_reserved is :%s:", st_spdord_req.c_reserved);
		/*** Ver 2.0 Ends ***/

		for (i_cnt = 1; i_cnt <= i_rec_cnt; i_cnt++)
		{
			fn_userlog(c_ServiceName, "*****Printing spread leg info structure element ***** %d", i_cnt - 1);
			fn_userlog(c_ServiceName, "l_token is %ld", st_spdord_req.st_spd_lg_inf[i_cnt - 1].l_token);

			fn_userlog(c_ServiceName, "*****Printing contract_desc structure element ***** %d", i_cnt - 1);
			fn_userlog(c_ServiceName, "c_instrument_name is :%s:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_cntrct_desc.c_instrument_name);
			fn_userlog(c_ServiceName, "c_symbol is :%s:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_cntrct_desc.c_symbol);
			fn_userlog(c_ServiceName, "li_expiry_date is :%ld:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_cntrct_desc.li_expiry_date);
			fn_userlog(c_ServiceName, "li_strike_price is :%ld:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_cntrct_desc.li_strike_price);
			fn_userlog(c_ServiceName, "c_option_type is :%s:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_cntrct_desc.c_option_type);
			fn_userlog(c_ServiceName, "si_ca_level is :%d:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_cntrct_desc.si_ca_level);
			fn_userlog(c_ServiceName, "*****Printing contract_desc  structure element ***** %d  ENDS", i_cnt - 1);

			fn_userlog(c_ServiceName, "c_op_broker_id is :%s:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_op_broker_id);
			fn_userlog(c_ServiceName, "c_filler1 is :%c:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_filler1);
			fn_userlog(c_ServiceName, "si_order_type is :%d:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].si_order_type);
			fn_userlog(c_ServiceName, "si_buy_sell is :%d:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].si_buy_sell);
			fn_userlog(c_ServiceName, "li_disclosed_vol is :%ld:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_disclosed_vol);
			fn_userlog(c_ServiceName, "li_disclosed_vol_remaining is :%ld:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_disclosed_vol_remaining);
			fn_userlog(c_ServiceName, "li_total_vol_remaining is :%ld:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_total_vol_remaining);
			fn_userlog(c_ServiceName, "li_volume is :%ld:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_volume);
			fn_userlog(c_ServiceName, "li_volume_filled_today is :%ld:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_volume_filled_today);
			fn_userlog(c_ServiceName, "li_price is :%ld:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_price);
			fn_userlog(c_ServiceName, "li_trigger_price is :%ld:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_trigger_price);
			fn_userlog(c_ServiceName, "li_min_fill_aon is :%ld:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].li_min_fill_aon);

			fn_userlog(c_ServiceName, "*****Printing order flags structure element ***** %d", i_cnt - 1);
			fn_userlog(c_ServiceName, "flg_ato is :%d:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_ato);
			fn_userlog(c_ServiceName, "flg_market is :%d:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_market);
			fn_userlog(c_ServiceName, "flg_sl is :%d:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_sl);
			fn_userlog(c_ServiceName, "flg_mit is :%d:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_mit);
			fn_userlog(c_ServiceName, "flg_day is :%d:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_day);
			fn_userlog(c_ServiceName, "flg_gtc is :%d:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_gtc);
			fn_userlog(c_ServiceName, "flg_ioc is :%d:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_ioc);
			fn_userlog(c_ServiceName, "flg_aon is :%d:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_aon);
			fn_userlog(c_ServiceName, "flg_mf is :%d:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_mf);
			fn_userlog(c_ServiceName, "flg_matched_ind is :%d:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_matched_ind);
			fn_userlog(c_ServiceName, "flg_traded is :%d:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_traded);
			fn_userlog(c_ServiceName, "flg_modified is :%d:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_modified);
			fn_userlog(c_ServiceName, "flg_frozen is :%d:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_frozen);
			fn_userlog(c_ServiceName, "flg_filler1 is :%d:",
					   st_spdord_req.st_spd_lg_inf[i_cnt - 1].st_order_flgs.flg_filler1);
			fn_userlog(c_ServiceName, "*****Printing order flags structure element ***** %d  ENDS", i_cnt - 1);

			fn_userlog(c_ServiceName, "c_open_close is :%c:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_open_close);
			fn_userlog(c_ServiceName, "c_cover_uncover is :%c:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_cover_uncover);
			fn_userlog(c_ServiceName, "c_giveup_flag is :%c:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_giveup_flag);
			fn_userlog(c_ServiceName, "c_filler2 is :%c:", st_spdord_req.st_spd_lg_inf[i_cnt - 1].c_filler2);
			fn_userlog(c_ServiceName, "*****Printing  spread leg info structure element ***** %d  ENDS", i_cnt - 1);
		}
		fn_userlog(c_ServiceName, "**********Printing exchange message structure  ENDS**********");
	}

	/***Packing the structure to put in Queue **/

	fn_userlog(c_ServiceName, "B. LAST ACT REF PACK LIB :%s: :%lld:", c_lst_act_ref, st_spdord_req.ll_lastactivityref);

	fn_cnvt_htn_spd2L3L_req(&st_spdord_req); /*** Ver 2.2 ***/

	fn_userlog(c_ServiceName, "Q. LAST ACT REF PACK LIB :%s: :%lld:", c_lst_act_ref, st_spdord_req.ll_lastactivityref);

	memcpy(&(st_exch_message.st_exch_sndmsg), &st_spdord_req, sizeof(st_spdord_req));

	/*** Commmented in ver 2.8
	i_ch_val = fn_get_reset_seq( 	ptr_st_rqst[0].c_pipe_id,
									  ptr_st_rqst[0].c_mod_trd_dt,
									GET_PLACED_SEQ,
																&i_snd_seq,
									c_ServiceName,
									c_err_msg);
 ***/
	/*** Added in Ver 2.8 ***/
	strcpy(st_s_sequence.c_pipe_id, ptr_st_rqst[0].c_pipe_id);
	strcpy(st_s_sequence.c_trd_dt, ptr_st_rqst[0].c_mod_trd_dt);
	st_s_sequence.c_rqst_typ = GET_PLACED_SEQ;

	i_ch_val = fn_get_fno_pack_seq(c_ServiceName,
								   &st_s_sequence,
								   c_err_msg);
	/*** Ver 2.8 Ends ***/

	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S00005", LIBMSG, c_err_msg);
		return -1;
	}

	/** st_exch_message.st_net_header.i_seq_num = i_snd_seq;   Commented in Ver 2.8    **/
	st_exch_message.st_net_header.i_seq_num = st_s_sequence.i_seq_num; /** using st_s_sequence.i_seq_num in ver 2.8 **/

	if (DEBUG_MSG_LVL_1)
	{
		/** fn_userlog(c_ServiceName,"The sequence number got is:%d:",i_snd_seq);    Commented in Ver 2.8 **/
		fn_userlog(c_ServiceName, "The sequence number got is:%d:", st_s_sequence.i_seq_num); /** using st_s_sequence.i_seq_num in Ver 2.8 **/
	}

	/****call routine for md5 ******/

	fn_cal_md5digest((void *)&st_spdord_req, sizeof(st_spdord_req), digest);

	/*****Copy the checksum in network header *******/

	memcpy(st_exch_message.st_net_header.c_checksum, digest, sizeof(digest));

	/******Set length of the network header *******/
	st_exch_message.st_net_header.si_message_length =
		sizeof(st_exch_message.st_net_header) + sizeof(st_spdord_req);

	fn_cnvt_htn_net_hdr(&st_exch_message.st_net_header); /** Ver 2.2 **/

	/***Packing the structure to in Queue datatype  **/

	switch (ptr_st_rqst[0].c_sprd_ord_ind)
	{
	case SPREAD_ORDER:
		/*** ptr_st_q_packet->li_msg_type = SP_BOARD_LOT_IN;  *** ver 2.1 ***/
		/** Added in Ver 2.1 **/
		if (ptr_st_rqst[0].c_req_typ == NEW)
		{
			ptr_st_q_packet->li_msg_type = SP_BOARD_LOT_IN;
		}
		else if (ptr_st_rqst[0].c_req_typ == MODIFY)
		{
			ptr_st_q_packet->li_msg_type = SP_ORDER_MOD_IN;
		}
		else if (ptr_st_rqst[0].c_req_typ == CANCEL)
		{
			ptr_st_q_packet->li_msg_type = SP_ORDER_CANCEL_IN;
		}
		/** Ver 2.1 Add Ends **/
		break;

	case L2_ORDER:
		ptr_st_q_packet->li_msg_type = TWOL_BOARD_LOT_IN;
		i_sprd_flg = 2;
		break;

	case L3_ORDER:
		ptr_st_q_packet->li_msg_type = THRL_BOARD_LOT_IN;
		i_sprd_flg = 3;
		break;

	default:
		fn_errlog(c_ServiceName, "L31050", " Invalid Message Type ", c_err_msg);
		return -1;
		break;
	}
	fn_userlog(c_ServiceName, "JUST TESTING st_exch_message.st_exch_sndmsg.st_spdoe_reqres.ll_lastactivityref :%lld:", st_exch_message.st_exch_sndmsg.st_spdoe_reqres.ll_lastactivityref);

	memcpy(&(ptr_st_q_packet->st_exch_msg_data), &st_exch_message, sizeof(st_exch_message));

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "SPREAD ORDER PACKING COMPLETE ");
	}

	return 0;
}

/****************************************************************************************************/
/* Function     : fn_pack_ordnry_ord_to_nse																													*/
/*                                                                                                  */
/* Description  : This function picks up data from exchange book and order book and fills the 			*/
/*								relevant information in exchange message structure st_oe_reqrest.This structure 	*/
/*								is then packed into st_exch_msg structure with the sequence number from FSB table */
/*								and MD5 digest and is packed in q_packet  																				*/
/*                                                                                                  */
/* INPUT        : vw_xchngbook                                                                      */
/*                vw_orderbook                                                                      */
/*                vw_nse_cntrct                                                                     */
/*                st_opm_pipe_mstr                                                                  */
/*                c_ServiceName                                                                     */
/*                                                                                                  */
/* OUTPUT       : st_req_q_data                                                                     */
/*                                                                                                  */
/*__________________________________________________________________________________________        */
/*|message type| exchange message data                                                      |       */
/*|____________|____________________________________________________________________________|       */
/*|Length      | Sequence Number | Checksum(MD5) for business data |Business Data           |       */
/*|(2 Bytes)   | (4 Bytes)       | (16 Bytes)                      | (Variable Length)      |       */
/*|____________|_________________|_________________________________|________________________|       */
/*                                                                                                  */
/****************************************************************************************************/

int fn_pack_ordnry_ord_to_nse(struct vw_xchngbook *ptr_st_rqst1,
							  struct vw_orderbook *ptr_st_ord1,
							  struct vw_nse_cntrct *ptr_st_nse_cnt1,
							  struct st_opm_pipe_mstr *ptr_st_opm_pipe_mstr,
							  struct st_req_q_data *ptr_st_q_packet,
							  char c_prgm_flg,	   /** Ver 1.3 **/
							  char *c_pan_no,	   /*** Added In ver 2.0 ***/
							  char *c_lst_act_ref, /*** Ver 2.2 ***/
							  char *c_esp_id,	   /*** Ver 2.4 ***/
							  char *c_algo_id,	   /*** Ver 2.4 ***/
							  char c_source_flg,   /*** Ver 2.6 ***/
							  char *c_ServiceName)
{
	struct st_oe_reqres st_ord_ent;
	struct st_exch_msg st_exch_message;

	int i_ch_val;
	int i_tmp_cntr;
	char c_refack_tmp[LEN_ORD_REFACK];
	time_t ud_tm_stmp;
	int i_no;
	int i_snd_seq;
	char c_err_msg[256];
	char c_user_typ_glb;

	unsigned char digest[16];

	/***************Memset the structures ******************************/

	memset(&st_ord_ent, '\0', sizeof(struct st_oe_reqres));
	memset(&st_exch_message, '\0', sizeof(struct st_exch_msg));

	rtrim(c_algo_id); /** Ver 2.4 **/
	rtrim(c_esp_id);  /* Ver 2.6 **/

	if (DEBUG_MSG_LVL_4) /** Ver 2.4 **/
	{
		fn_userlog(c_ServiceName, "esp_id:%s: algo_id:%s: c_source_flg :%c:", c_esp_id, c_algo_id, c_source_flg); /** Ver 2.4 **/ /* Vre 2.6*/
	}
	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Inside function fn_pack_ordnry_ord_to_nse");
	}

	if (DEBUG_MSG_LVL_3)
	{

		fn_userlog(c_ServiceName, "Order book data");

		fn_userlog(c_ServiceName, "Order book c_pro_cli_ind recieved is :%c", ptr_st_ord1->c_pro_cli_ind);
		fn_userlog(c_ServiceName, "Order book c_cln_mtch_accnt recieved is :%s:", ptr_st_ord1->c_cln_mtch_accnt);
		fn_userlog(c_ServiceName, "Order book c_ordr_flw recieved is :%c:", ptr_st_ord1->c_ordr_flw);
		fn_userlog(c_ServiceName, "Order book l_ord_tot_qty recieved is :%ld:", ptr_st_ord1->l_ord_tot_qty);
		fn_userlog(c_ServiceName, "Order book l_exctd_qty recieved is :%ld:", ptr_st_ord1->l_exctd_qty);
		fn_userlog(c_ServiceName, "Order book l_exctd_qty_day recieved is :%ld:", ptr_st_ord1->l_exctd_qty_day);
		fn_userlog(c_ServiceName, "Order book c_settlor recieved is :%s:", ptr_st_ord1->c_settlor);
		fn_userlog(c_ServiceName, "Order book c_ctcl_id recieved is :%s:", ptr_st_ord1->c_ctcl_id);
		fn_userlog(c_ServiceName, "Order book c_ack_tm recieved is :%s:", ptr_st_ord1->c_ack_tm);
		fn_userlog(c_ServiceName, "Order book c_prev_ack_tm recieved is :%s:", ptr_st_ord1->c_prev_ack_tm);

		fn_userlog(c_ServiceName, "Exchange book data");

		fn_userlog(c_ServiceName, "Exchange book c_req_typ recieved is :%c:",
				   ptr_st_rqst1->c_req_typ);
		fn_userlog(c_ServiceName, "Exchange book l_dsclsd_qty recieved is :%ld:", ptr_st_rqst1->l_dsclsd_qty);
		fn_userlog(c_ServiceName, "Exchange book c_slm_flg recieved is :%c:", ptr_st_rqst1->c_slm_flg);
		fn_userlog(c_ServiceName, "Exchange book l_ord_lmt_rt recieved is :%ld:", ptr_st_rqst1->l_ord_lmt_rt);
		fn_userlog(c_ServiceName, "Exchange book l_stp_lss_tgr recieved is :%ld:", ptr_st_rqst1->l_stp_lss_tgr);
		fn_userlog(c_ServiceName, "Exchange book c_ord_typ recieved is :%c:", ptr_st_rqst1->c_ord_typ);
		fn_userlog(c_ServiceName, "Exchange book l_ord_tot_qty recieved is :%ld:", ptr_st_rqst1->l_ord_tot_qty);
		fn_userlog(c_ServiceName, "Exchange book c_valid_dt recieved is :%s:", ptr_st_rqst1->c_valid_dt);
		fn_userlog(c_ServiceName, "Exchange book l_ord_seq recieved is :%ld:", ptr_st_rqst1->l_ord_seq);

		fn_userlog(c_ServiceName, "OPM data");

		fn_userlog(c_ServiceName, "OPM li_opm_brnch_id recieved is :%ld:", ptr_st_opm_pipe_mstr->li_opm_brnch_id);
		fn_userlog(c_ServiceName, "OPM c_opm_trdr_id recieved is :%s:", ptr_st_opm_pipe_mstr->c_opm_trdr_id);
		fn_userlog(c_ServiceName, "OPM c_xchng_brkr_id recieved is :%s:", ptr_st_opm_pipe_mstr->c_xchng_brkr_id);

		fn_userlog(c_ServiceName, "exchange message structure data");

		fn_userlog(c_ServiceName, "NSE CONTRACT c_prd_typ is :%c:", ptr_st_nse_cnt1->c_prd_typ);
		fn_userlog(c_ServiceName, "NSE CONTRACT c_ctgry_indstk is :%c:", ptr_st_nse_cnt1->c_ctgry_indstk);
		fn_userlog(c_ServiceName, "NSE CONTRACT c_symbol is :%s:", ptr_st_nse_cnt1->c_symbol);
		fn_userlog(c_ServiceName, "NSE CONTRACT l_ca_lvl is :%ld:", ptr_st_nse_cnt1->l_ca_lvl);
	}

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "NSE CONTRACT l_token_id is :%ld:", ptr_st_nse_cnt1->l_token_id);
	}

	/********Populate the header of the ordinary order structure ******************/

	/************ header details START ******************************************/

	/**  fn_orstonse_char ( st_ord_ent.st_hdr.c_filler_1,
								  4,
								  " ",
								  1 );  ** 2nd parameter of function changed from 2 to 4 in Ver 1.1 **
	*** Commented in Ver 2.2 **/

	st_ord_ent.st_hdr.li_trader_id = (long int)atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id); /*** Ver 2.2 ***/

	st_ord_ent.st_hdr.li_log_time = 0;

	fn_orstonse_char(st_ord_ent.st_hdr.c_alpha_char,
					 LEN_ALPHA_CHAR,
					 " ",
					 1);

	switch (ptr_st_rqst1->c_req_typ)
	{
	case NEW:
		st_ord_ent.st_hdr.si_transaction_code = BOARD_LOT_IN;
		st_ord_ent.ll_lastactivityref = 0; /*** Ver 2.2 ***/
		break;

	case MODIFY:
		st_ord_ent.st_hdr.si_transaction_code = ORDER_MOD_IN;
		st_ord_ent.ll_lastactivityref = atoll(c_lst_act_ref); /*** Ver 2.2 ***/
		break;

	case CANCEL:
		st_ord_ent.st_hdr.si_transaction_code = ORDER_CANCEL_IN;
		st_ord_ent.ll_lastactivityref = atoll(c_lst_act_ref); /*** Ver 2.2 ***/
		break;

	default:
		fn_errlog(c_ServiceName, "L31055", " Invalid Request Type ", c_err_msg);
		return -1;
		break;
	}

	switch (ptr_st_opm_pipe_mstr->si_user_typ_glb)
	{
	case 0:
		c_user_typ_glb = TRADER;
		break;

	case 4:
		c_user_typ_glb = CORPORATE_MANAGER;
		break;

	case 5:
		c_user_typ_glb = BRANCH_MANAGER;
		break;

	default:
		fn_errlog(c_ServiceName, "L31060", " Invalid USER Type ", c_err_msg);
		return -1;
		break;
	}

	st_ord_ent.st_hdr.si_error_code = 0;

	fn_orstonse_char(st_ord_ent.st_hdr.c_filler_2, 8, " ", 1);

	/*
	 void fn_orstonse_char(char *ptr_c_dest, long int li_len_dest, const char *ptr_c_src,  long int li_len_src)
		{
		long int li_cnt;

		for (li_cnt = 0; li_cnt < li_len_src; li_cnt++)
		{
			*(ptr_c_dest + li_cnt) = toupper(*(ptr_c_src + li_cnt));
		}
		memset((ptr_c_dest + li_len_src), ' ', (li_len_dest - li_len_src));
		}
	*/

	memset(st_ord_ent.st_hdr.c_time_stamp_1, 0, LEN_TIME_STAMP);

	memset(st_ord_ent.st_hdr.c_time_stamp_2, 0, LEN_TIME_STAMP);

	st_ord_ent.st_hdr.si_message_length = sizeof(st_ord_ent);

	/************ header details END ******************************************/

	st_ord_ent.c_participant_type = 'C';

	st_ord_ent.c_filler_1 = ' ';

	st_ord_ent.si_competitor_period = 0;

	st_ord_ent.si_solicitor_period = 0;

	st_ord_ent.c_modified_cancelled_by = c_user_typ_glb;

	st_ord_ent.c_filler_2 = ' ';

	st_ord_ent.si_reason_code = 0;

	fn_orstonse_char(st_ord_ent.c_filler_3,
					 4,
					 " ",
					 1);

	st_ord_ent.l_token_no = ptr_st_nse_cnt1->l_token_id;

	fn_orstonse_cntrct_desc((struct vw_nse_cntrct *)ptr_st_nse_cnt1,
							(struct st_contract_desc *)&(st_ord_ent.st_con_desc),
							c_ServiceName,
							c_err_msg);

	fn_orstonse_char(st_ord_ent.c_counter_party_broker_id,
					 LEN_BROKER_ID,
					 " ",
					 1);

	st_ord_ent.c_filler_4 = ' ';

	fn_orstonse_char(st_ord_ent.c_filler_5,
					 2,
					 " ",
					 1);

	st_ord_ent.c_closeout_flg = ' ';

	st_ord_ent.c_filler_6 = ' ';

	st_ord_ent.si_order_type = 0;

	if (ptr_st_rqst1->c_req_typ == NEW)
	{
		st_ord_ent.d_order_number = 0;
	}
	else
	{
		st_ord_ent.d_order_number = (double)atoll(ptr_st_ord1->c_xchng_ack);
	}

	if (ptr_st_ord1->c_pro_cli_ind == BRKR_PLCD)
	{
		fn_orstonse_char(st_ord_ent.c_account_number,
						 LEN_ACCOUNT_NUMBER,
						 " ",
						 1);
	}
	else
	{
		fn_orstonse_char(st_ord_ent.c_account_number,
						 LEN_ACCOUNT_NUMBER,
						 ptr_st_ord1->c_cln_mtch_accnt,
						 strlen(ptr_st_ord1->c_cln_mtch_accnt));
	}

	switch (ptr_st_rqst1->c_slm_flg)
	{
	case 'S':
		st_ord_ent.si_book_type = STOP_LOSS_MIT_ORDER;
		break;

	case 'L':
	case 'M':
		st_ord_ent.si_book_type = REGULAR_LOT_ORDER;
		break;

	default:
		fn_errlog(c_ServiceName, "L31065", " Invalid slm flag ", c_err_msg);
		return -1;
		break;
	}

	switch (ptr_st_ord1->c_ordr_flw)
	{
	case 'B':
		st_ord_ent.si_buy_sell_indicator = NSE_BUY;
		break;
	case 'S':
		st_ord_ent.si_buy_sell_indicator = NSE_SELL;
		break;

	default:
		fn_errlog(c_ServiceName, "L31070", " Invalid order flow flag ", c_err_msg);
		return -1;
		break;
	}

	st_ord_ent.li_disclosed_volume = ptr_st_rqst1->l_dsclsd_qty;

	st_ord_ent.li_disclosed_volume_remaining = st_ord_ent.li_disclosed_volume;

	st_ord_ent.li_volume = (ptr_st_rqst1->l_ord_tot_qty - ptr_st_ord1->l_exctd_qty);

	if (ptr_st_rqst1->l_ord_tot_qty > ptr_st_ord1->l_ord_tot_qty)
	{
		st_ord_ent.li_total_volume_remaining = (ptr_st_ord1->l_ord_tot_qty - ptr_st_ord1->l_exctd_qty);
	}
	else
	{
		st_ord_ent.li_total_volume_remaining = (ptr_st_rqst1->l_ord_tot_qty - ptr_st_ord1->l_exctd_qty);
	}

	st_ord_ent.li_volume_filled_today = ptr_st_ord1->l_exctd_qty_day;

	switch (ptr_st_rqst1->c_slm_flg)
	{
	case 'M':
		st_ord_ent.li_price = 0;
		break;

	case 'L':
	case 'S':
		st_ord_ent.li_price = ptr_st_rqst1->l_ord_lmt_rt;
		break;
	default:
		fn_errlog(c_ServiceName, "L31075", " Invalid slm flag ", c_err_msg);
		return -1;
		break;
	}
	st_ord_ent.li_trigger_price = ptr_st_rqst1->l_stp_lss_tgr;

	switch (ptr_st_rqst1->c_ord_typ)
	{
	case 'T':
	case 'I':
		st_ord_ent.li_good_till_date = 0;
		break;

	case 'D':
		/*fn_orstonse_tm( &st_ord_ent.li_good_till_date, ptr_st_rqst1->c_valid_dt);*/

		fn_timearr_to_long(ptr_st_rqst1->c_valid_dt, &st_ord_ent.li_good_till_date);
		break;
	default:
		fn_errlog(c_ServiceName, "L31080", " Invalid order type", c_err_msg);
		return -1;
		break;
	}

	if (ptr_st_rqst1->c_req_typ == NEW)
	{
		st_ord_ent.li_entry_date_time = 0;
	}
	else
	{
		/*fn_orstonse_tm ( &(st_ord_ent.li_entry_date_time),ptr_st_ord1->c_ack_tm); */

		fn_timearr_to_long(ptr_st_ord1->c_ack_tm, &(st_ord_ent.li_entry_date_time));
	}

	st_ord_ent.li_minimum_fill_aon_volume = 0;

	if (ptr_st_rqst1->c_req_typ == NEW)
	{
		st_ord_ent.li_last_modified = 0;
	}
	else
	{
		/*fn_orstonse_tm ( &(st_ord_ent.li_last_modified),ptr_st_ord1->c_prev_ack_tm);    */

		fn_timearr_to_long(ptr_st_ord1->c_prev_ack_tm, &(st_ord_ent.li_last_modified));
	}

	st_ord_ent.st_ord_flg.flg_ato = 0;

	st_ord_ent.st_ord_flg.flg_market = 0;

	switch (ptr_st_rqst1->c_slm_flg)
	{
	case 'M':
	case 'L':
		st_ord_ent.st_ord_flg.flg_sl = 0;
		break;

	case 'S':
		st_ord_ent.st_ord_flg.flg_sl = 1;
		break;
	default:
		fn_errlog(c_ServiceName, "L31085", " Invalid slm flag ", c_err_msg);
		return -1;
		break;
	}

	st_ord_ent.st_ord_flg.flg_mit = 0;

	if (ptr_st_rqst1->c_ord_typ == 'T')
	{
		st_ord_ent.st_ord_flg.flg_day = 1;
	}
	else
	{
		st_ord_ent.st_ord_flg.flg_day = 0;
	}

	st_ord_ent.st_ord_flg.flg_gtc = 0;

	if (ptr_st_rqst1->c_ord_typ == 'I')
	{
		st_ord_ent.st_ord_flg.flg_ioc = 1;
	}
	else
	{
		st_ord_ent.st_ord_flg.flg_ioc = 0;
	}

	st_ord_ent.st_ord_flg.flg_aon = 0;

	st_ord_ent.st_ord_flg.flg_mf = 0;

	st_ord_ent.st_ord_flg.flg_matched_ind = 0;

	st_ord_ent.st_ord_flg.flg_traded = 0;

	if (ptr_st_rqst1->c_req_typ == MODIFY)
	{
		st_ord_ent.st_ord_flg.flg_modified = 1;
	}
	else
	{
		st_ord_ent.st_ord_flg.flg_modified = 0;
	}

	st_ord_ent.st_ord_flg.flg_frozen = 0;

	st_ord_ent.st_ord_flg.flg_filler1 = 0;

	st_ord_ent.si_branch_id = ptr_st_opm_pipe_mstr->li_opm_brnch_id;

	/**** Ver 1.1 ****/
	/**	st_ord_ent.si_trader_id =(short int) atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id); **/
	st_ord_ent.li_trader_id = (long int)atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id);
	/**** Ver 1.1 ends ****/

	fn_orstonse_char(st_ord_ent.c_broker_id,
					 LEN_BROKER_ID,
					 ptr_st_opm_pipe_mstr->c_xchng_brkr_id,
					 strlen(ptr_st_opm_pipe_mstr->c_xchng_brkr_id));

	st_ord_ent.i_ordr_sqnc = ptr_st_rqst1->l_ord_seq;

	st_ord_ent.c_open_close = 'O';

	memset(st_ord_ent.c_settlor, 0, LEN_SETTLOR);

	/**** Commented in Ver 1.7 ***
	if (strncmp(ptr_st_ord1->c_cln_mtch_accnt,"751",3)!=0 &&
			strncmp(ptr_st_ord1->c_cln_mtch_accnt,"651",3)!=0 &&
			strncmp(ptr_st_ord1->c_cln_mtch_accnt,"750",3)!=0 &&
			strncmp(ptr_st_ord1->c_cln_mtch_accnt,"650",3)!=0 ) ***/
	/*** Ver 1.7 Starts ***/
	MEMSET(sql_icd_cust_type);
	EXEC SQL
		SELECT ICD_CUST_TYPE
			INTO : sql_icd_cust_type
					   FROM ICD_INFO_CLIENT_DTLS,
				   IAI_INFO_ACCOUNT_INFO
					   WHERE ICD_SERIAL_NO = IAI_SERIAL_NO
						   AND IAI_MATCH_ACCOUNT_NO = : ptr_st_ord1->c_cln_mtch_accnt;

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "L31090", SQLMSG, c_err_msg);
		return -1;
	}
	SETNULL(sql_icd_cust_type);
	/*********  Ver 1.7 Ends ************/
	if (strcmp(sql_icd_cust_type.arr, "NRI") != 0) /****** Check added in Ver 1.7 *******/
	{
		if (ptr_st_ord1->c_pro_cli_ind == BRKR_PLCD)
		{
			fn_orstonse_char(st_ord_ent.c_settlor,
							 LEN_SETTLOR,
							 " ",
							 1);

			st_ord_ent.si_pro_client_indicator = NSE_PRO;
		}
		else
		{
			fn_orstonse_char(st_ord_ent.c_settlor,
							 LEN_SETTLOR,
							 ptr_st_opm_pipe_mstr->c_xchng_brkr_id,
							 strlen(ptr_st_opm_pipe_mstr->c_xchng_brkr_id));

			st_ord_ent.si_pro_client_indicator = NSE_CLIENT;
		}
	}
	else
	{
		fn_orstonse_char(st_ord_ent.c_settlor,
						 LEN_SETTLOR,
						 ptr_st_ord1->c_settlor,
						 strlen(ptr_st_ord1->c_settlor));

		st_ord_ent.si_pro_client_indicator = NSE_CLIENT;
	}

	/*** Ver 2.0 Starts ***/
	if (strcmp(c_pan_no, "*") != 0)
	{
		fn_orstonse_char(st_ord_ent.c_pan,
						 LEN_PAN,
						 c_pan_no,
						 strlen(c_pan_no));
	}
	else
	{
		fn_orstonse_char(st_ord_ent.c_pan,
						 LEN_PAN,
						 " ",
						 1);
	}
	/*** Ver 2.0 Ends ***/

	st_ord_ent.si_settlement_period = 0; /***** Since there is no provision for
																				 settlement period in EBA we set this to  **/

	st_ord_ent.c_cover_uncover = 'V';

	st_ord_ent.c_giveup_flag = 'P';

	if (DEBUG_MSG_LVL_2)
	{
		fn_userlog(c_ServiceName, "c_prgm_flg :%c:", c_prgm_flg); /** Ver 1.3 **/
	}

	/*** Ver 2.0 Starts ***/
	if (c_prgm_flg == 'A')
	{
		st_ord_ent.l_algo_id = FO_AUTO_MTM_ALG_ID;
		st_ord_ent.si_algo_category = FO_AUTO_MTM_ALG_CAT_ID;
	}
	else if (c_prgm_flg == 'T')
	{
		st_ord_ent.l_algo_id = FO_PRICE_IMP_ALG_ID;
		st_ord_ent.si_algo_category = FO_PRICE_IMP_ALG_CAT_ID;
	}
	else if (c_prgm_flg == 'Z') /*** ver 2.3 starts **/
	{
		st_ord_ent.l_algo_id = FO_PRFT_ORD_ALG_ID;
		st_ord_ent.si_algo_category = FO_PRFT_ORD_ALG_CAT_ID;
	} /** ver 2.3 ends **/
	else if (c_prgm_flg == 'G') /*** ver 2.5 starts **/
	{
		st_ord_ent.l_algo_id = FO_FLASH_TRD_ALG_ID;
		st_ord_ent.si_algo_category = FO_FLASH_TRD_ALG_CAT_ID;
	} /** ver 2.5 ends **/
	else if (c_source_flg == 'G') /*** Ver 2.9 ***/
	{
		st_ord_ent.l_algo_id = FO_FOGTT_ALG_ID;
		st_ord_ent.si_algo_category = FO_FOGTT_ALG_CAT_ID;
	}
	else if (strcmp(c_algo_id, "*") != 0) /** Ver 2.4 **/
	{
		st_ord_ent.l_algo_id = atol(c_algo_id);
		st_ord_ent.si_algo_category = 0;
	}
	else
	{
		st_ord_ent.l_algo_id = FO_NON_ALG_ID;
		st_ord_ent.si_algo_category = FO_NON_ALG_CAT_ID;
	}

	fn_orstonse_char(st_ord_ent.c_reserved,
					 52, /*** Changes from 60 to 52 in Ver 2.2 ***/
					 " ",
					 1);

	/*** Ver 2.0 Ends ***/

	/*** Ver 1.3 starts here **/
	if (c_prgm_flg == 'A' || c_prgm_flg == 'T' || c_prgm_flg == 'Z' || c_prgm_flg == 'G' || c_source_flg == 'G') /*** c_prgm_flg == 'T' added in Ver 1.8 ***/ /** ver 2.3 Z added ***/ /** Ver 2.9 c_source_flg added **/
	{
		strcat(ptr_st_ord1->c_ctcl_id, "000");
	} /*** Ver 1.3 ends here ***/
	else if (strcmp(c_algo_id, "*") != 0 && c_source_flg == 'M') /** Ver 2.6 starts here **/ /* Mobile Algo */
	{
		MEMSET(ptr_st_ord1->c_ctcl_id);
		strcpy(ptr_st_ord1->c_ctcl_id, "333333333333");
		strcat(ptr_st_ord1->c_ctcl_id, "000");
	}
	else if (strcmp(c_algo_id, "*") != 0 && (c_source_flg == 'W' || c_source_flg == 'E')) /* Web and Exe algo */
	{
		MEMSET(ptr_st_ord1->c_ctcl_id);
		strcpy(ptr_st_ord1->c_ctcl_id, "111111111111");
		strcat(ptr_st_ord1->c_ctcl_id, "000");
	} /** Ver 2.6 ends here **/
	/** else if( strcmp(c_algo_id,"*") != 0 ) ** Ver 2.4 ** ** Ver 2.6 **/
	else if ((strcmp(c_algo_id, "*") != 0) && (strcmp(c_esp_id, "4124") == 0)) /** Ver 2.6 **/ /** Intellect Old APK**/
	{
		MEMSET(ptr_st_ord1->c_ctcl_id);
		strcpy(ptr_st_ord1->c_ctcl_id, "333333333333");
		strcat(ptr_st_ord1->c_ctcl_id, "000");
	}
	else if (strcmp(c_algo_id, "*") != 0) /** Ver 2.6 **/ /**BlueShift and other **/
	{
		MEMSET(ptr_st_ord1->c_ctcl_id);
		strcpy(ptr_st_ord1->c_ctcl_id, "111111111111");
		strcat(ptr_st_ord1->c_ctcl_id, "000");
	}
	else
	{
		strcat(ptr_st_ord1->c_ctcl_id, "100");
	}

	st_ord_ent.d_nnf_field = (double)atoll(ptr_st_ord1->c_ctcl_id);

	st_ord_ent.d_filler19 = 0.0;

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "**Printing exchange message structure for Ordinary Order**");
		fn_userlog(c_ServiceName, "**Printing header structure for Ordinary Order**");
		/***fn_userlog(c_ServiceName,"c_filler_1 is :%s:",st_ord_ent.st_hdr.c_filler_1);** Commented in Ver 2.2 **/
		fn_userlog(c_ServiceName, " li_log_time is :%ld:", st_ord_ent.st_hdr.li_log_time);
		fn_userlog(c_ServiceName, "c_alpha_char is :%s:", st_ord_ent.st_hdr.c_alpha_char);
		fn_userlog(c_ServiceName, "si_transaction_code is :%d:", st_ord_ent.st_hdr.si_transaction_code);
		fn_userlog(c_ServiceName, "si_error_code is :%d:", st_ord_ent.st_hdr.si_error_code);
		fn_userlog(c_ServiceName, "c_filler_2 is :%s:", st_ord_ent.st_hdr.c_filler_2);
		fn_userlog(c_ServiceName, "c_time_stamp_1 is :%s:", st_ord_ent.st_hdr.c_time_stamp_1);
		fn_userlog(c_ServiceName, "c_time_stamp_2 is :%s:", st_ord_ent.st_hdr.c_time_stamp_2);
		fn_userlog(c_ServiceName, "si_message_length is :%d:", st_ord_ent.st_hdr.si_message_length);
		fn_userlog(c_ServiceName, "**Printing header structure for Ordinary Order ends**");

		fn_userlog(c_ServiceName, "c_participant_type is :%c:", st_ord_ent.c_participant_type);
		fn_userlog(c_ServiceName, "c_filler_1 is :%c:", st_ord_ent.c_filler_1);
		fn_userlog(c_ServiceName, "si_competitor_period is :%d:", st_ord_ent.si_competitor_period);
		fn_userlog(c_ServiceName, "si_solicitor_period is :%d:", st_ord_ent.si_solicitor_period);
		fn_userlog(c_ServiceName, "c_modified_cancelled_by is :%c:", st_ord_ent.c_modified_cancelled_by);
		fn_userlog(c_ServiceName, "c_filler_2 is :%c:", st_ord_ent.c_filler_2);
		fn_userlog(c_ServiceName, "si_reason_code is :%d:", st_ord_ent.si_reason_code);
		fn_userlog(c_ServiceName, "c_filler_3 is :%s:", st_ord_ent.c_filler_3);
		fn_userlog(c_ServiceName, "l_token_no is :%ld:", st_ord_ent.l_token_no);

		fn_userlog(c_ServiceName, "**Printing cntract_desc structure for Ordinary Order**");
		fn_userlog(c_ServiceName, "c_instrument_name is :%s:", st_ord_ent.st_con_desc.c_instrument_name);
		fn_userlog(c_ServiceName, "c_symbol is :%s:", st_ord_ent.st_con_desc.c_symbol);
		fn_userlog(c_ServiceName, "li_expiry_date is :%ld:", st_ord_ent.st_con_desc.li_expiry_date);
		fn_userlog(c_ServiceName, "li_strike_price is :%ld:", st_ord_ent.st_con_desc.li_strike_price);
		fn_userlog(c_ServiceName, "c_option_type is :%s:", st_ord_ent.st_con_desc.c_option_type);
		fn_userlog(c_ServiceName, "si_ca_level is :%d:", st_ord_ent.st_con_desc.si_ca_level);
		fn_userlog(c_ServiceName, "**Printing cntract_desc structure for Ordinary Order ends**");

		fn_userlog(c_ServiceName, "c_counter_party_broker_id  is :%s :", st_ord_ent.c_counter_party_broker_id);
		fn_userlog(c_ServiceName, "c_filler_4  is :%c:", st_ord_ent.c_filler_4);
		fn_userlog(c_ServiceName, "c_filler_5  is :%s:", st_ord_ent.c_filler_5);
		fn_userlog(c_ServiceName, "c_closeout_flg is :%c:", st_ord_ent.c_closeout_flg);
		fn_userlog(c_ServiceName, "c_filler_6 is :%c:", st_ord_ent.c_filler_6);
		fn_userlog(c_ServiceName, "si_order_type is :%d:", st_ord_ent.si_order_type);
		fn_userlog(c_ServiceName, "d_order_number is :%ld:", st_ord_ent.d_order_number);
		fn_userlog(c_ServiceName, "c_account_number  is :%s:", st_ord_ent.c_account_number);
		fn_userlog(c_ServiceName, "si_book_type is :%d:", st_ord_ent.si_book_type);
		fn_userlog(c_ServiceName, "si_buy_sell_indicator is :%d:", st_ord_ent.si_buy_sell_indicator);
		fn_userlog(c_ServiceName, "li_disclosed_volume is :%ld:", st_ord_ent.li_disclosed_volume);
		fn_userlog(c_ServiceName, "li_disclosed_volume_remaining is :%ld:", st_ord_ent.li_disclosed_volume_remaining);
		fn_userlog(c_ServiceName, "li_total_volume_remaining is :%ld:", st_ord_ent.li_total_volume_remaining);
		fn_userlog(c_ServiceName, "li_volume is :%ld:", st_ord_ent.li_volume);
		fn_userlog(c_ServiceName, "li_volume_filled_today is :%ld:", st_ord_ent.li_volume_filled_today);
		fn_userlog(c_ServiceName, "li_price is :%ld:", st_ord_ent.li_price);
		fn_userlog(c_ServiceName, "li_trigger_price is :%ld:", st_ord_ent.li_trigger_price);
		fn_userlog(c_ServiceName, "li_good_till_date is :%ld:", st_ord_ent.li_good_till_date);
		fn_userlog(c_ServiceName, "li_entry_date_time is :%ld:", st_ord_ent.li_entry_date_time);
		fn_userlog(c_ServiceName, "li_minimum_fill_aon_volume is :%ld:", st_ord_ent.li_minimum_fill_aon_volume);
		fn_userlog(c_ServiceName, "li_last_modified is :%ld:", st_ord_ent.li_last_modified);
		fn_userlog(c_ServiceName, "**Printing order flags structure for Ordinary Order**");
		fn_userlog(c_ServiceName, "flg_ato is :%d:", st_ord_ent.st_ord_flg.flg_ato);
		fn_userlog(c_ServiceName, "flg_market is :%d:", st_ord_ent.st_ord_flg.flg_market);
		fn_userlog(c_ServiceName, "flg_sl is :%d:", st_ord_ent.st_ord_flg.flg_sl);
		fn_userlog(c_ServiceName, "flg_mit is :%d:", st_ord_ent.st_ord_flg.flg_mit);
		fn_userlog(c_ServiceName, "flg_day is :%d:", st_ord_ent.st_ord_flg.flg_day);
		fn_userlog(c_ServiceName, "flg_gtc is :%d:", st_ord_ent.st_ord_flg.flg_gtc);
		fn_userlog(c_ServiceName, "flg_ioc is :%d:", st_ord_ent.st_ord_flg.flg_ioc);
		fn_userlog(c_ServiceName, "flg_aon is :%d:", st_ord_ent.st_ord_flg.flg_aon);
		fn_userlog(c_ServiceName, "flg_mf is :%d:", st_ord_ent.st_ord_flg.flg_mf);
		fn_userlog(c_ServiceName, "flg_matched_ind is :%d:", st_ord_ent.st_ord_flg.flg_matched_ind);
		fn_userlog(c_ServiceName, "flg_traded is :%d:", st_ord_ent.st_ord_flg.flg_traded);
		fn_userlog(c_ServiceName, "flg_modified is :%d:", st_ord_ent.st_ord_flg.flg_modified);
		fn_userlog(c_ServiceName, "flg_frozen is :%d:", st_ord_ent.st_ord_flg.flg_frozen);
		fn_userlog(c_ServiceName, "flg_filler1 is :%d:", st_ord_ent.st_ord_flg.flg_filler1);
		fn_userlog(c_ServiceName, "**Printing order flags structure for Ordinary Order ends**");
		fn_userlog(c_ServiceName, "si_branch_id is :%d:", st_ord_ent.si_branch_id);
		fn_userlog(c_ServiceName, "li_trader_id is :%ld:", st_ord_ent.li_trader_id);
		fn_userlog(c_ServiceName, "c_broker_id  is :%s:", st_ord_ent.c_broker_id);
		fn_userlog(c_ServiceName, "c_remarks is :%s:", st_ord_ent.c_remarks);
		fn_userlog(c_ServiceName, "c_open_close is :%c:", st_ord_ent.c_open_close);
		fn_userlog(c_ServiceName, "c_settlor is :%s:", st_ord_ent.c_settlor);
		fn_userlog(c_ServiceName, "si_pro_client_indicator is :%d:", st_ord_ent.si_pro_client_indicator);
		fn_userlog(c_ServiceName, "si_settlement_period is :%d:", st_ord_ent.si_settlement_period);
		fn_userlog(c_ServiceName, "c_cover_uncover is :%c:", st_ord_ent.c_cover_uncover);
		fn_userlog(c_ServiceName, "c_giveup_flag is :%c:", st_ord_ent.c_giveup_flag);
		fn_userlog(c_ServiceName, "i_ordr_sqnc is :%d:", st_ord_ent.i_ordr_sqnc);
		fn_userlog(c_ServiceName, "d_nnf_field is :%ld:", st_ord_ent.d_nnf_field);
		fn_userlog(c_ServiceName, "d_filler19 is :%ld:", st_ord_ent.d_filler19);
		/*** Ver 2.0 Starts ***/
		fn_userlog(c_ServiceName, "c_pan is :%s:", st_ord_ent.c_pan);
		fn_userlog(c_ServiceName, "l_algo_id is :%ld:", st_ord_ent.l_algo_id);
		fn_userlog(c_ServiceName, "si_algo_category is :%d:", st_ord_ent.si_algo_category);
		fn_userlog(c_ServiceName, "c_reserved is :%s:", st_ord_ent.c_reserved);
		fn_userlog(c_ServiceName, "ll_lastactivityref :%ld:", st_ord_ent.ll_lastactivityref); /** Ver 2.2 **/
		/*** Ver 2.0 Ends ***/
		fn_userlog(c_ServiceName, "**Printing exchange message structure for Ordinary Order ends**");
	}

	/***Packing the structure to put in Queue **/

	fn_cnvt_htn_oe_reqres(&st_ord_ent); /*** ver 2.2 ***/

	memcpy(&(st_exch_message.st_exch_sndmsg), &st_ord_ent, sizeof(st_ord_ent));

	i_ch_val = fn_get_reset_seq(ptr_st_rqst1->c_pipe_id,
								ptr_st_rqst1->c_mod_trd_dt,
								GET_PLACED_SEQ,
								&i_snd_seq,
								c_ServiceName,
								c_err_msg);

	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S00010", LIBMSG, c_err_msg);
		return -1;
	}

	st_exch_message.st_net_header.i_seq_num = i_snd_seq;

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "The sequence number got is:%d:", i_snd_seq);
	}

	/****call routine for md5 ******/

	fn_cal_md5digest((void *)&st_ord_ent, sizeof(st_ord_ent), digest);

	/*****Copy the checksum in network header *******/

	memcpy(st_exch_message.st_net_header.c_checksum, digest, sizeof(digest));

	/******Set length of the network header *******/

	st_exch_message.st_net_header.si_message_length =
		sizeof(st_exch_message.st_net_header) + sizeof(st_ord_ent);

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "The message length is:%d:", st_exch_message.st_net_header.si_message_length);
	}

	fn_cnvt_htn_net_hdr(&st_exch_message.st_net_header); /*** ver 2.2 ***/

	/***Packing the structure to in Queue datatype  **/

	switch (ptr_st_rqst1->c_req_typ)
	{
	case NEW:
		ptr_st_q_packet->li_msg_type = BOARD_LOT_IN;
		break;

	case MODIFY:
		ptr_st_q_packet->li_msg_type = ORDER_MOD_IN;
		break;

	case CANCEL:
		ptr_st_q_packet->li_msg_type = ORDER_CANCEL_IN;
		break;
	}

	memcpy(&(ptr_st_q_packet->st_exch_msg_data), &st_exch_message, sizeof(st_exch_message));
	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, " Data copied in queue packet");
	}

	return 0;
}

/****************************************************************************************************/
/* Function     : fn_pack_exer_ord_to_nse																														*/
/*                                                                                                  */
/* Description  : This function picks up data from exchange book and Exercise book and fills the    */
/*                relevant information in exchange message structure st_ex_pl_reqres.This structure */
/*                is then packed into st_exch_msg structure with the sequence number from FSB table */
/*                and MD5 digest and is packed in q_packet                                          */
/*                                                                                                  */
/* INPUT        : vw_xchngbook                                                                      */
/*                vw_exrcbook                                                                       */
/*                vw_nse_cntrct                                                                     */
/*                st_opm_pipe_mstr                                                                  */
/*                c_ServiceName                                                                     */
/*                                                                                                  */
/* OUTPUT       : st_req_q_data                                                                     */
/*                                                                                                  */
/*__________________________________________________________________________________________        */
/*|message type| exchange message data                                                      |       */
/*|____________|____________________________________________________________________________|       */
/*       |Length    | Sequence Number | Checksum(MD5) for business data |Business Data      |       */
/*       |(2 Bytes) | (4 Bytes)       | (16 Bytes)                      | (Variable Length) |       */
/*       |__________|_________________|_________________________________|___________________|       */
/*                                                                                                  */
/****************************************************************************************************/

int fn_pack_exer_ord_to_nse(struct vw_xchngbook *ptr_st_rqst1,
							struct vw_exrcbook *ptr_st_exr,
							struct vw_nse_cntrct *ptr_st_nse_cnt1,
							struct st_opm_pipe_mstr *ptr_st_opm_pipe_mstr,
							struct st_req_q_data *ptr_st_q_packet,
							char *c_ServiceName)
{
	struct st_ex_pl_reqres st_ex_pl_ent_req;
	struct st_exch_msg st_exch_message;

	unsigned char digest[16];
	unsigned char c_tmp;
	char c_err_msg[256];

	int i_snd_seq;
	int i_ch_val;

	/***************Memset the structures ******************************/

	memset(&st_ex_pl_ent_req, '\0', sizeof(struct st_ex_pl_reqres));
	memset(&st_exch_message, '\0', sizeof(struct st_exch_msg));

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Inside function fn_pack_exer_ord_to_nse");
	}

	if (DEBUG_MSG_LVL_3)
	{

		fn_userlog(c_ServiceName, "Exercise book data");

		fn_userlog(c_ServiceName, "Exercise book c_pro_cli_ind recieved is :%c:", ptr_st_exr->c_pro_cli_ind);
		fn_userlog(c_ServiceName, "Exercise book c_cln_mtch_accnt recieved is :%s:", ptr_st_exr->c_cln_mtch_accnt);
		fn_userlog(c_ServiceName, "Exercise book c_xchng_ack recieved is :%s:", ptr_st_exr->c_xchng_ack);
		fn_userlog(c_ServiceName, "Exercise book c_ack_tm recieved is :%s:", ptr_st_exr->c_ack_tm);
		fn_userlog(c_ServiceName, "Exercise book c_settlor recieved is :%s:", ptr_st_exr->c_settlor);

		fn_userlog(c_ServiceName, "Exchange book data");

		fn_userlog(c_ServiceName, "Exchange book c_req_typ recieved is :%c:",
				   ptr_st_rqst1->c_req_typ);
		fn_userlog(c_ServiceName, "Exchange book l_dsclsd_qty recieved is :%ld:", ptr_st_rqst1->l_dsclsd_qty);
		fn_userlog(c_ServiceName, "Exchange book c_slm_flg recieved is :%c:", ptr_st_rqst1->c_slm_flg);
		fn_userlog(c_ServiceName, "Exchange book l_ord_lmt_rt recieved is :%ld:", ptr_st_rqst1->l_ord_lmt_rt);
		fn_userlog(c_ServiceName, "Exchange book l_stp_lss_tgr recieved is :%ld:", ptr_st_rqst1->l_stp_lss_tgr);
		fn_userlog(c_ServiceName, "Exchange book c_ord_typ recieved is :%c:", ptr_st_rqst1->c_ord_typ);
		fn_userlog(c_ServiceName, "Exchange book c_ordr_rfrnc recieved is :%s:", ptr_st_rqst1->c_ordr_rfrnc);

		fn_userlog(c_ServiceName, "OPM data");

		fn_userlog(c_ServiceName, "OPM li_opm_brnch_id recieved is :%ld:", ptr_st_opm_pipe_mstr->li_opm_brnch_id);
		fn_userlog(c_ServiceName, "OPM c_opm_trdr_id recieved is :%s:", ptr_st_opm_pipe_mstr->c_opm_trdr_id);
		fn_userlog(c_ServiceName, "OPM c_xchng_brkr_id recieved is :%s:", ptr_st_opm_pipe_mstr->c_xchng_brkr_id);

		fn_userlog(c_ServiceName, "exchange message structure data");

		fn_userlog(c_ServiceName, "NSE CONTRACT c_prd_typ is :%c:", ptr_st_nse_cnt1->c_prd_typ);
		fn_userlog(c_ServiceName, "NSE CONTRACT c_ctgry_indstk is :%c:", ptr_st_nse_cnt1->c_ctgry_indstk);
		fn_userlog(c_ServiceName, "NSE CONTRACT c_symbol is :%s:", ptr_st_nse_cnt1->c_symbol);
		fn_userlog(c_ServiceName, "NSE CONTRACT l_ca_lvl is :%ld:", ptr_st_nse_cnt1->l_ca_lvl);
	}
	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "NSE CONTRACT l_token_id is :%ld:", ptr_st_nse_cnt1->l_token_id);
	}

	/********Populate the header of the exercise order structure ******************/

	/************ header details START ******************************************/

	/*** fn_orstonse_char ( st_ex_pl_ent_req.st_hdr.c_filler_1,
								  4,
								  " ",
								  1 ); ** 2nd parameter of function changed from 2 to 4 in Ver 1.1 **
	   *** Commented in Ver 2.2 ***/

	st_ex_pl_ent_req.st_hdr.li_trader_id = (long int)atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id); /*** Ver 2.2 **/

	st_ex_pl_ent_req.st_hdr.li_log_time = 0;

	fn_orstonse_char(st_ex_pl_ent_req.st_hdr.c_alpha_char,
					 LEN_ALPHA_CHAR,
					 " ",
					 1);

	switch (ptr_st_rqst1->c_req_typ)
	{
	case NEW:
		st_ex_pl_ent_req.st_hdr.si_transaction_code = EX_PL_ENTRY_IN;
		break;

	case MODIFY:
		st_ex_pl_ent_req.st_hdr.si_transaction_code = EX_PL_MOD_IN;
		break;

	case CANCEL:
		st_ex_pl_ent_req.st_hdr.si_transaction_code = EX_PL_CXL_IN;
		break;

	default:
		fn_errlog(c_ServiceName, "L31095", "Invalid Order Type", c_err_msg);
		return -1;
		break;
	}

	st_ex_pl_ent_req.st_hdr.si_error_code = 0;

	fn_orstonse_char(st_ex_pl_ent_req.st_hdr.c_filler_2,
					 8,
					 " ",
					 1);

	memset(st_ex_pl_ent_req.st_hdr.c_time_stamp_1, 0, LEN_TIME_STAMP);

	memset(st_ex_pl_ent_req.st_hdr.c_time_stamp_2, 0, LEN_TIME_STAMP);

	st_ex_pl_ent_req.st_hdr.si_message_length = sizeof(st_ex_pl_ent_req);

	/************ header details END ******************************************/

	st_ex_pl_ent_req.si_reason_code = 0;

	st_ex_pl_ent_req.st_ex_pl_data.l_token = ptr_st_nse_cnt1->l_token_id;

	fn_orstonse_cntrct_desc((struct vw_nse_cntrct *)ptr_st_nse_cnt1,
							&st_ex_pl_ent_req.st_ex_pl_data.st_cntrct_desc,
							c_ServiceName,
							c_err_msg);

	st_ex_pl_ent_req.st_ex_pl_data.si_expl_flag = 1;

	if (ptr_st_rqst1->c_req_typ == NEW)
	{
		st_ex_pl_ent_req.st_ex_pl_data.d_expl_number = 0;
	}
	else
	{
		st_ex_pl_ent_req.st_ex_pl_data.d_expl_number = (double)atoll(ptr_st_exr->c_xchng_ack);
	}

	st_ex_pl_ent_req.st_ex_pl_data.si_market_type = NORMAL_MARKET;

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Exrc mtch accnt before conversion:%s:", ptr_st_exr->c_cln_mtch_accnt);
	}

	if (ptr_st_exr->c_pro_cli_ind == BRKR_PLCD)
	{
		fn_orstonse_char(st_ex_pl_ent_req.st_ex_pl_data.c_account_number,
						 LEN_ACCOUNT_NUMBER,
						 " ",
						 1);
	}
	else
	{
		fn_orstonse_char(st_ex_pl_ent_req.st_ex_pl_data.c_account_number,
						 LEN_ACCOUNT_NUMBER,
						 ptr_st_exr->c_cln_mtch_accnt,
						 strlen(ptr_st_exr->c_cln_mtch_accnt));
	}

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Exrc mtch accnt after conversion:%s:", st_ex_pl_ent_req.st_ex_pl_data.c_account_number);
	}

	st_ex_pl_ent_req.st_ex_pl_data.li_quantity = ptr_st_rqst1->l_ord_tot_qty;

	if (ptr_st_exr->c_pro_cli_ind == BRKR_PLCD)
	{
		st_ex_pl_ent_req.st_ex_pl_data.si_pro_cli = NSE_PRO;
	}
	else
	{
		st_ex_pl_ent_req.st_ex_pl_data.si_pro_cli = NSE_CLIENT;
	}

	if (ptr_st_rqst1->c_ex_ordr_typ == 'E')
	{
		st_ex_pl_ent_req.st_ex_pl_data.si_exercise_type = 1;
	}
	else
	{
		st_ex_pl_ent_req.st_ex_pl_data.si_exercise_type = 0;
	}

	if (ptr_st_rqst1->c_req_typ == NEW)
	{
		st_ex_pl_ent_req.st_ex_pl_data.li_entry_date_tm = 0;
	}
	else
	{
		/*fn_orstonse_tm ( &st_ex_pl_ent_req.st_ex_pl_data.li_entry_date_tm, ptr_st_exr->c_ack_tm);*/

		fn_timearr_to_long(ptr_st_exr->c_ack_tm, &(st_ex_pl_ent_req.st_ex_pl_data.li_entry_date_tm));
	}

	st_ex_pl_ent_req.st_ex_pl_data.si_branch_id = ptr_st_opm_pipe_mstr->li_opm_brnch_id;
	/*** Ver 1.1 **/
	/** st_ex_pl_ent_req.st_ex_pl_data.si_trader_id =(short) atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id); **/
	st_ex_pl_ent_req.st_ex_pl_data.li_trader_id = (long)atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id);
	/** Ver 1.1 ends **/

	fn_orstonse_char(st_ex_pl_ent_req.st_ex_pl_data.c_broker_id,
					 LEN_BROKER_ID,
					 ptr_st_opm_pipe_mstr->c_xchng_brkr_id,
					 strlen(ptr_st_opm_pipe_mstr->c_xchng_brkr_id));

	fn_orstonse_char(st_ex_pl_ent_req.st_ex_pl_data.c_remarks,
					 LEN_REMARKS_1,
					 ptr_st_rqst1->c_ordr_rfrnc,
					 strlen(ptr_st_rqst1->c_ordr_rfrnc));

	if (ptr_st_exr->c_pro_cli_ind == BRKR_PLCD)
	{
		fn_orstonse_char(st_ex_pl_ent_req.st_ex_pl_data.c_participant,
						 LEN_SETTLOR,
						 " ",
						 1);
	}
	else
	{
		fn_orstonse_char(st_ex_pl_ent_req.st_ex_pl_data.c_participant,
						 LEN_SETTLOR,
						 ptr_st_exr->c_settlor,
						 strlen(ptr_st_exr->c_settlor));
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "**Printing cntract_desc structure for Exercise Order**");
		fn_userlog(c_ServiceName, "**Printing header structure for Exercise Order**");
		/*fn_userlog(c_ServiceName,"c_filler_1 is :%s:",st_ex_pl_ent_req.st_hdr.c_filler_1); * Commented in Ver 2.2 **/
		fn_userlog(c_ServiceName, "li_trader_id :%ld:", st_ex_pl_ent_req.st_hdr.li_trader_id); /** Ver 2.2 **/
		fn_userlog(c_ServiceName, " li_log_time is :%ld:", st_ex_pl_ent_req.st_hdr.li_log_time);
		fn_userlog(c_ServiceName, "c_alpha_char is :%s:", st_ex_pl_ent_req.st_hdr.c_alpha_char);
		fn_userlog(c_ServiceName, "si_transaction_code is :%d:", st_ex_pl_ent_req.st_hdr.si_transaction_code);
		fn_userlog(c_ServiceName, "si_error_code is :%d:", st_ex_pl_ent_req.st_hdr.si_error_code);
		fn_userlog(c_ServiceName, "c_filler_2 is :%s:", st_ex_pl_ent_req.st_hdr.c_filler_2);
		fn_userlog(c_ServiceName, "c_time_stamp_1 is :%s:", st_ex_pl_ent_req.st_hdr.c_time_stamp_1);
		fn_userlog(c_ServiceName, "c_time_stamp_2 is :%s:", st_ex_pl_ent_req.st_hdr.c_time_stamp_2);
		fn_userlog(c_ServiceName, "si_message_length is :%d:", st_ex_pl_ent_req.st_hdr.si_message_length);
		fn_userlog(c_ServiceName, "**Printing header structure for Exercise Order ENDS**");

		fn_userlog(c_ServiceName, "si_reason_code is :%d:", st_ex_pl_ent_req.si_reason_code);
		fn_userlog(c_ServiceName, "**Printing st_ex_pl_data structure for Exercise Order**");
		fn_userlog(c_ServiceName, "l_token is :%ld:", st_ex_pl_ent_req.st_ex_pl_data.l_token);
		fn_userlog(c_ServiceName, "**Printing contract desc structure for Exercise Order**");
		fn_userlog(c_ServiceName, "c_instrument_name is :%s:",
				   st_ex_pl_ent_req.st_ex_pl_data.st_cntrct_desc.c_instrument_name);
		fn_userlog(c_ServiceName, "c_symbol is :%s:", st_ex_pl_ent_req.st_ex_pl_data.st_cntrct_desc.c_symbol);
		fn_userlog(c_ServiceName, "li_expiry_date is :%ld:",
				   st_ex_pl_ent_req.st_ex_pl_data.st_cntrct_desc.li_expiry_date);
		fn_userlog(c_ServiceName, "li_strike_price is :%ld:",
				   st_ex_pl_ent_req.st_ex_pl_data.st_cntrct_desc.li_strike_price);
		fn_userlog(c_ServiceName, "c_option_type is :%s:",
				   st_ex_pl_ent_req.st_ex_pl_data.st_cntrct_desc.c_option_type);
		fn_userlog(c_ServiceName, "si_ca_level is :%d:",
				   st_ex_pl_ent_req.st_ex_pl_data.st_cntrct_desc.si_ca_level);
		fn_userlog(c_ServiceName, "**Printing contract desc structure for Exercise Order ENDS**");

		fn_userlog(c_ServiceName, "si_expl_flag is :%d:", st_ex_pl_ent_req.st_ex_pl_data.si_expl_flag);
		fn_userlog(c_ServiceName, "d_expl_number is :%ld:", st_ex_pl_ent_req.st_ex_pl_data.d_expl_number);
		fn_userlog(c_ServiceName, "si_market_type is :%d:", st_ex_pl_ent_req.st_ex_pl_data.si_market_type);
		fn_userlog(c_ServiceName, "c_account_number is :%s:", st_ex_pl_ent_req.st_ex_pl_data.c_account_number);
		fn_userlog(c_ServiceName, "li_quantity is :%ld:", st_ex_pl_ent_req.st_ex_pl_data.li_quantity);
		fn_userlog(c_ServiceName, "si_pro_cli is :%d:", st_ex_pl_ent_req.st_ex_pl_data.si_pro_cli);
		fn_userlog(c_ServiceName, "si_exercise_type is :%d: ", st_ex_pl_ent_req.st_ex_pl_data.si_exercise_type);
		fn_userlog(c_ServiceName, "li_entry_date_tm is :%ld:", st_ex_pl_ent_req.st_ex_pl_data.li_entry_date_tm);
		fn_userlog(c_ServiceName, "si_branch_id is :%d:", st_ex_pl_ent_req.st_ex_pl_data.si_branch_id);
		fn_userlog(c_ServiceName, "li_trader_id is :%ld:", st_ex_pl_ent_req.st_ex_pl_data.li_trader_id);
		fn_userlog(c_ServiceName, "c_broker_id is :%s:", st_ex_pl_ent_req.st_ex_pl_data.c_broker_id);
		fn_userlog(c_ServiceName, "c_remarks is :%s:", st_ex_pl_ent_req.st_ex_pl_data.c_remarks);
		fn_userlog(c_ServiceName, "c_participant is :%s:", st_ex_pl_ent_req.st_ex_pl_data.c_participant);
		fn_userlog(c_ServiceName, "**Printing st_ex_pl_data structure for Exercise Order ENDS**");
		fn_userlog(c_ServiceName, "**Printing cntract_desc structure for Exercise Order**");
	}

	/***Packing the structure to put in Queue **/

	fn_cnvt_htn_new_exer_req(&st_ex_pl_ent_req); /*** Ver 2.2 ***/

	memcpy(&(st_exch_message.st_exch_sndmsg), &st_ex_pl_ent_req, sizeof(st_ex_pl_ent_req));

	i_ch_val = fn_get_reset_seq(ptr_st_rqst1->c_pipe_id,
								ptr_st_rqst1->c_mod_trd_dt,
								GET_PLACED_SEQ,
								&i_snd_seq,
								c_ServiceName,
								c_err_msg);

	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S00005", LIBMSG, c_err_msg);
		return -1;
	}

	st_exch_message.st_net_header.i_seq_num = i_snd_seq;

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "The sequence number got is:%d:", i_snd_seq);
	}

	/****call routine for md5 ******/
	fn_cal_md5digest((void *)&st_ex_pl_ent_req, sizeof(st_ex_pl_ent_req), digest);

	/*****Copy the checksum in network header *******/

	memcpy(st_exch_message.st_net_header.c_checksum, digest, sizeof(digest));

	/******Set length of the network header *******/
	st_exch_message.st_net_header.si_message_length =
		sizeof(st_exch_message.st_net_header) + sizeof(st_ex_pl_ent_req);
	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "The message length is:%d:", st_exch_message.st_net_header.si_message_length);
	}

	fn_cnvt_htn_net_hdr(&st_exch_message.st_net_header); /** Ver 2.2 **/

	/***Packing the structure to in Queue datatype  **/
	switch (ptr_st_rqst1->c_req_typ)
	{
	case NEW:
		ptr_st_q_packet->li_msg_type = EX_PL_ENTRY_IN;
		break;

	case MODIFY:
		ptr_st_q_packet->li_msg_type = EX_PL_MOD_IN;
		break;

	case CANCEL:
		ptr_st_q_packet->li_msg_type = EX_PL_CXL_IN;
		break;
	}
	memcpy(&(ptr_st_q_packet->st_exch_msg_data), &st_exch_message, sizeof(st_exch_message));
	return 0;
}

/*** Ver 2.2 Started ***/

int fn_pack_ordnry_ord_to_nse_tr(struct vw_xchngbook *ptr_st_rqst1,
								 struct vw_orderbook *ptr_st_ord1,
								 struct vw_nse_cntrct *ptr_st_nse_cnt1,
								 struct st_opm_pipe_mstr *ptr_st_opm_pipe_mstr,
								 struct st_req_q_data *ptr_st_q_packet,
								 char c_prgm_flg,
								 char *c_pan_no,
								 char *c_lst_act_ref,
								 char *c_esp_id,	/*** Ver 2.4 ***/
								 char *c_algo_id,	/*** Ver 2.4 ***/
								 char c_source_flg, /*** Ver 2.6 ***/
								 char *c_ServiceName)
{

	struct st_oe_reqres_tr st_ord_ent_tr;
	struct st_om_rqst_tr st_ord_mod_tr;
	struct st_exch_msg st_exch_message;
	struct st_pk_sequence st_s_sequence; /*** Added in ver 2.8 ***/

	int i_ch_val;
	int i_tmp_cntr;
	int i_no;
	int i_snd_seq;
	time_t ud_tm_stmp;

	char c_refack_tmp[LEN_ORD_REFACK];
	char c_err_msg[256];
	char c_user_typ_glb;
	unsigned char digest[16];

	/***************Memset the structures ******************************/
	memset(&st_ord_ent_tr, '\0', sizeof(struct st_oe_reqres_tr));
	memset(&st_ord_mod_tr, '\0', sizeof(struct st_om_rqst_tr));
	memset(&st_exch_message, '\0', sizeof(struct st_exch_msg));
	MEMSET(st_s_sequence); /*** Added in ver 2.8 ***/

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Inside function fn_pack_ordnry_ord_to_nse_trm");
	}

	if (DEBUG_MSG_LVL_3)
	{

		fn_userlog(c_ServiceName, "Order book data");

		fn_userlog(c_ServiceName, "Order book c_pro_cli_ind recieved is :%c", ptr_st_ord1->c_pro_cli_ind);
		fn_userlog(c_ServiceName, "Order book c_cln_mtch_accnt recieved is :%s:", ptr_st_ord1->c_cln_mtch_accnt);
		fn_userlog(c_ServiceName, "Order book c_ordr_flw recieved is :%c:", ptr_st_ord1->c_ordr_flw);
		fn_userlog(c_ServiceName, "Order book l_ord_tot_qty recieved is :%ld:", ptr_st_ord1->l_ord_tot_qty);
		fn_userlog(c_ServiceName, "Order book l_exctd_qty recieved is :%ld:", ptr_st_ord1->l_exctd_qty);
		fn_userlog(c_ServiceName, "Order book l_exctd_qty_day recieved is :%ld:", ptr_st_ord1->l_exctd_qty_day);
		fn_userlog(c_ServiceName, "Order book c_settlor recieved is :%s:", ptr_st_ord1->c_settlor);
		fn_userlog(c_ServiceName, "Order book c_ctcl_id recieved is :%s:", ptr_st_ord1->c_ctcl_id);
		fn_userlog(c_ServiceName, "Order book c_ack_tm recieved is :%s:", ptr_st_ord1->c_ack_tm);
		fn_userlog(c_ServiceName, "Order book c_prev_ack_tm recieved is :%s:", ptr_st_ord1->c_prev_ack_tm);

		fn_userlog(c_ServiceName, "Exchange book data");

		fn_userlog(c_ServiceName, "Exchange book c_req_typ recieved is :%c:",
				   ptr_st_rqst1->c_req_typ);
		fn_userlog(c_ServiceName, "Exchange book l_dsclsd_qty recieved is :%ld:", ptr_st_rqst1->l_dsclsd_qty);
		fn_userlog(c_ServiceName, "Exchange book c_slm_flg recieved is :%c:", ptr_st_rqst1->c_slm_flg);
		fn_userlog(c_ServiceName, "Exchange book l_ord_lmt_rt recieved is :%ld:", ptr_st_rqst1->l_ord_lmt_rt);
		fn_userlog(c_ServiceName, "Exchange book l_stp_lss_tgr recieved is :%ld:", ptr_st_rqst1->l_stp_lss_tgr);
		fn_userlog(c_ServiceName, "Exchange book c_ord_typ recieved is :%c:", ptr_st_rqst1->c_ord_typ);
		fn_userlog(c_ServiceName, "Exchange book l_ord_tot_qty recieved is :%ld:", ptr_st_rqst1->l_ord_tot_qty);
		fn_userlog(c_ServiceName, "Exchange book c_valid_dt recieved is :%s:", ptr_st_rqst1->c_valid_dt);
		fn_userlog(c_ServiceName, "Exchange book l_ord_seq recieved is :%ld:", ptr_st_rqst1->l_ord_seq);

		fn_userlog(c_ServiceName, "OPM data");

		fn_userlog(c_ServiceName, "OPM li_opm_brnch_id recieved is :%ld:", ptr_st_opm_pipe_mstr->li_opm_brnch_id);
		fn_userlog(c_ServiceName, "OPM c_opm_trdr_id recieved is :%s:", ptr_st_opm_pipe_mstr->c_opm_trdr_id);
		fn_userlog(c_ServiceName, "OPM c_xchng_brkr_id recieved is :%s:", ptr_st_opm_pipe_mstr->c_xchng_brkr_id);

		fn_userlog(c_ServiceName, "exchange message structure data");

		fn_userlog(c_ServiceName, "NSE CONTRACT c_prd_typ is :%c:", ptr_st_nse_cnt1->c_prd_typ);
		fn_userlog(c_ServiceName, "NSE CONTRACT c_ctgry_indstk is :%c:", ptr_st_nse_cnt1->c_ctgry_indstk);
		fn_userlog(c_ServiceName, "NSE CONTRACT c_symbol is :%s:", ptr_st_nse_cnt1->c_symbol);
		fn_userlog(c_ServiceName, "NSE CONTRACT l_ca_lvl is :%ld:", ptr_st_nse_cnt1->l_ca_lvl);
		fn_userlog(c_ServiceName, "esp_id:%s: algo_id:%s:", c_esp_id, c_algo_id); /** Ver 2.4 **/
	}

	rtrim(c_algo_id); /** Ver 2.4 **/
	rtrim(c_esp_id);  /** Ver 2.6 **/

	if (DEBUG_MSG_LVL_4) /** Ver 2.4 **/
	{
		fn_userlog(c_ServiceName, "esp_id:%s: algo_id:%s:", c_esp_id, c_algo_id); /** Ver 2.4 **/
	}
	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "NSE CONTRACT l_token_id is :%ld:", ptr_st_nse_cnt1->l_token_id);
	}

	switch (ptr_st_rqst1->c_req_typ)
	{
	case NEW:
		st_ord_ent_tr.si_transaction_code = BOARD_LOT_IN_TR;
		st_ord_ent_tr.li_userid = (long int)atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id);
		st_ord_ent_tr.si_reason_code = 0;
		st_ord_ent_tr.l_token_no = ptr_st_nse_cnt1->l_token_id;

		fn_orstonse_cntrct_desc_tr((struct vw_nse_cntrct *)ptr_st_nse_cnt1,
								   (struct st_contract_desc_tr *)&(st_ord_ent_tr.st_con_desc_tr),
								   c_ServiceName,
								   c_err_msg);

		if (ptr_st_ord1->c_pro_cli_ind == BRKR_PLCD)
		{
			fn_orstonse_char(st_ord_ent_tr.c_account_number,
							 LEN_ACCOUNT_NUMBER,
							 " ",
							 1);
		}
		else
		{
			fn_orstonse_char(st_ord_ent_tr.c_account_number,
							 LEN_ACCOUNT_NUMBER,
							 ptr_st_ord1->c_cln_mtch_accnt,
							 strlen(ptr_st_ord1->c_cln_mtch_accnt));
		}

		switch (ptr_st_rqst1->c_slm_flg)
		{
		case 'S':
			st_ord_ent_tr.si_book_type = STOP_LOSS_MIT_ORDER;
			break;

		case 'L':
		case 'M':
			st_ord_ent_tr.si_book_type = REGULAR_LOT_ORDER;
			break;

		default:
			fn_errlog(c_ServiceName, "L31100", " Invalid slm flag ", c_err_msg);
			return -1;
			break;
		}

		switch (ptr_st_ord1->c_ordr_flw)
		{
		case 'B':
			st_ord_ent_tr.si_buy_sell_indicator = NSE_BUY;
			break;
		case 'S':
			st_ord_ent_tr.si_buy_sell_indicator = NSE_SELL;
			break;
		default:
			fn_errlog(c_ServiceName, "L31105", " Invalid order flow flag ", c_err_msg);
			return -1;
			break;
		}

		st_ord_ent_tr.li_disclosed_volume = ptr_st_rqst1->l_dsclsd_qty;
		st_ord_ent_tr.li_volume = (ptr_st_rqst1->l_ord_tot_qty - ptr_st_ord1->l_exctd_qty);

		switch (ptr_st_rqst1->c_slm_flg)
		{
		case 'M':
			st_ord_ent_tr.li_price = 0;
			break;

		case 'L':
		case 'S':
			st_ord_ent_tr.li_price = ptr_st_rqst1->l_ord_lmt_rt;
			break;
		default:
			fn_errlog(c_ServiceName, "L31110", " Invalid slm flag ", c_err_msg);
			return -1;
			break;
		}

		switch (ptr_st_rqst1->c_ord_typ)
		{
		case 'T':
		case 'I':
			st_ord_ent_tr.li_good_till_date = 0;
			break;

		case 'D':
			fn_timearr_to_long(ptr_st_rqst1->c_valid_dt, &st_ord_ent_tr.li_good_till_date);
			break;
		default:
			fn_errlog(c_ServiceName, "L31115", " Invalid order type", c_err_msg);
			return -1;
			break;
		}

		st_ord_ent_tr.st_ord_flg.flg_ato = 0;

		st_ord_ent_tr.st_ord_flg.flg_market = 0;

		switch (ptr_st_rqst1->c_slm_flg)
		{
		case 'M':
		case 'L':
			st_ord_ent_tr.st_ord_flg.flg_sl = 0;
			break;

		case 'S':
			st_ord_ent_tr.st_ord_flg.flg_sl = 1;
			break;
		default:
			fn_errlog(c_ServiceName, "L31120", " Invalid slm flag ", c_err_msg);
			return -1;
			break;
		}

		st_ord_ent_tr.st_ord_flg.flg_mit = 0;

		if (ptr_st_rqst1->c_ord_typ == 'T')
		{
			st_ord_ent_tr.st_ord_flg.flg_day = 1;
		}
		else
		{
			st_ord_ent_tr.st_ord_flg.flg_day = 0;
		}

		st_ord_ent_tr.st_ord_flg.flg_gtc = 0;

		if (ptr_st_rqst1->c_ord_typ == 'I')
		{
			st_ord_ent_tr.st_ord_flg.flg_ioc = 1;
		}
		else
		{
			st_ord_ent_tr.st_ord_flg.flg_ioc = 0;
		}

		st_ord_ent_tr.st_ord_flg.flg_aon = 0;
		st_ord_ent_tr.st_ord_flg.flg_mf = 0;
		st_ord_ent_tr.st_ord_flg.flg_matched_ind = 0;
		st_ord_ent_tr.st_ord_flg.flg_traded = 0;
		st_ord_ent_tr.st_ord_flg.flg_modified = 0;
		st_ord_ent_tr.st_ord_flg.flg_frozen = 0;
		st_ord_ent_tr.st_ord_flg.flg_filler1 = 0;

		st_ord_ent_tr.si_branch_id = ptr_st_opm_pipe_mstr->li_opm_brnch_id;
		st_ord_ent_tr.li_trader_id = (long int)atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id);

		fn_orstonse_char(st_ord_ent_tr.c_broker_id,
						 LEN_BROKER_ID,
						 ptr_st_opm_pipe_mstr->c_xchng_brkr_id,
						 strlen(ptr_st_opm_pipe_mstr->c_xchng_brkr_id));

		st_ord_ent_tr.c_open_close = 'O';

		memset(st_ord_ent_tr.c_settlor, 0, LEN_SETTLOR);

		MEMSET(sql_icd_cust_type);

		EXEC SQL
			SELECT ICD_CUST_TYPE
				INTO : sql_icd_cust_type
						   FROM ICD_INFO_CLIENT_DTLS,
					   IAI_INFO_ACCOUNT_INFO
						   WHERE ICD_SERIAL_NO = IAI_SERIAL_NO
							   AND IAI_MATCH_ACCOUNT_NO = : ptr_st_ord1->c_cln_mtch_accnt;

		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "L31125", SQLMSG, c_err_msg);
			return -1;
		}

		SETNULL(sql_icd_cust_type);

		if (strcmp(sql_icd_cust_type.arr, "NRI") != 0)
		{
			if (ptr_st_ord1->c_pro_cli_ind == BRKR_PLCD)
			{
				fn_orstonse_char(st_ord_ent_tr.c_settlor,
								 LEN_SETTLOR,
								 " ",
								 1);

				st_ord_ent_tr.si_pro_client_indicator = NSE_PRO;
			}
			else
			{
				fn_orstonse_char(st_ord_ent_tr.c_settlor,
								 LEN_SETTLOR,
								 ptr_st_opm_pipe_mstr->c_xchng_brkr_id,
								 strlen(ptr_st_opm_pipe_mstr->c_xchng_brkr_id));

				st_ord_ent_tr.si_pro_client_indicator = NSE_CLIENT;
			}
		}
		else
		{
			fn_orstonse_char(st_ord_ent_tr.c_settlor,
							 LEN_SETTLOR,
							 ptr_st_ord1->c_settlor,
							 strlen(ptr_st_ord1->c_settlor));

			st_ord_ent_tr.si_pro_client_indicator = NSE_CLIENT;
		}

		st_ord_ent_tr.st_add_ord_flg.c_cover_uncover = 'V';

		st_ord_ent_tr.li_ordr_sqnc = ptr_st_rqst1->l_ord_seq;

		if (DEBUG_MSG_LVL_2)
		{
			fn_userlog(c_ServiceName, "c_prgm_flg :%c:", c_prgm_flg);
		}

		if (c_prgm_flg == 'A' || c_prgm_flg == 'T' || c_prgm_flg == 'Z' || c_prgm_flg == 'G' || c_source_flg == 'G') /*Ver 2.9 c_source_flg **/
		{
			strcat(ptr_st_ord1->c_ctcl_id, "000");
		}
		else if (strcmp(c_algo_id, "*") != 0 && c_source_flg == 'M') /** Ver 2.6 Starts here Mobile**/
		{
			MEMSET(ptr_st_ord1->c_ctcl_id);
			strcpy(ptr_st_ord1->c_ctcl_id, "333333333333");
			strcat(ptr_st_ord1->c_ctcl_id, "000");
		}
		else if (strcmp(c_algo_id, "*") != 0 && (c_source_flg == 'W' || c_source_flg == 'E')) /** Web and Exe **/
		{
			MEMSET(ptr_st_ord1->c_ctcl_id);
			strcpy(ptr_st_ord1->c_ctcl_id, "111111111111");
			strcat(ptr_st_ord1->c_ctcl_id, "000");
		} /*** Ver 2.6 Ends Here ***/
		/** else if( strcmp(c_algo_id,"*") != 0 ) ** Ver 2.4 ** *Ver 2.6 **/
		else if ((strcmp(c_algo_id, "*") != 0) && (strcmp(c_esp_id, "4124") == 0)) /** Ver 2.6 **/ /** Intellect Old APK**/
		{
			MEMSET(ptr_st_ord1->c_ctcl_id);
			strcpy(ptr_st_ord1->c_ctcl_id, "333333333333");
			strcat(ptr_st_ord1->c_ctcl_id, "000");
		}
		else if (strcmp(c_algo_id, "*") != 0) /** Ver 2.6 **/ /**BlueShift and other **/
		{
			MEMSET(ptr_st_ord1->c_ctcl_id);
			strcpy(ptr_st_ord1->c_ctcl_id, "111111111111");
			strcat(ptr_st_ord1->c_ctcl_id, "000");
		}
		else
		{
			strcat(ptr_st_ord1->c_ctcl_id, "100");
		}

		st_ord_ent_tr.d_nnf_field = (double)atoll(ptr_st_ord1->c_ctcl_id);

		if (strcmp(c_pan_no, "*") != 0)
		{
			fn_orstonse_char(st_ord_ent_tr.c_pan,
							 LEN_PAN,
							 c_pan_no,
							 strlen(c_pan_no));
		}
		else
		{
			fn_orstonse_char(st_ord_ent_tr.c_pan,
							 LEN_PAN,
							 " ",
							 1);
		}

		if (c_prgm_flg == 'A')
		{
			st_ord_ent_tr.l_algo_id = FO_AUTO_MTM_ALG_ID;
			st_ord_ent_tr.si_algo_category = FO_AUTO_MTM_ALG_CAT_ID;
		}
		else if (c_prgm_flg == 'T')
		{
			st_ord_ent_tr.l_algo_id = FO_PRICE_IMP_ALG_ID;
			st_ord_ent_tr.si_algo_category = FO_PRICE_IMP_ALG_CAT_ID;
		}
		else if (c_prgm_flg == 'Z') /*** ver 2.3 starts **/
		{
			st_ord_ent_tr.l_algo_id = FO_PRFT_ORD_ALG_ID;
			st_ord_ent_tr.si_algo_category = FO_PRFT_ORD_ALG_CAT_ID;
		} /** ver 2.3 ends **/
		else if (c_prgm_flg == 'G') /** Ver 2.4 **/
		{
			st_ord_ent_tr.l_algo_id = FO_FLASH_TRD_ALG_ID;
			st_ord_ent_tr.si_algo_category = FO_FLASH_TRD_ALG_CAT_ID;
		} /** Ver 2.4 **/
		else if (c_source_flg == 'G') /** System Placed Gtt Order ** Ver 2.9 **/
		{
			st_ord_ent_tr.l_algo_id = FO_FOGTT_ALG_ID;
			st_ord_ent_tr.si_algo_category = FO_FOGTT_ALG_CAT_ID;
		}
		else if (strcmp(c_algo_id, "*") != 0) /** Ver 2.4 **/
		{
			/* fn_userlog(c_ServiceName,"Inside c_esp_id "); * Ver 2.6 */
			st_ord_ent_tr.l_algo_id = atol(c_algo_id);
			st_ord_ent_tr.si_algo_category = 0;
		}
		else
		{
			st_ord_ent_tr.l_algo_id = FO_NON_ALG_ID;
			st_ord_ent_tr.si_algo_category = FO_NON_ALG_CAT_ID;
		}
		/*** Commented in Ver 2.7 *****
  fn_userlog(c_ServiceName,"st_ord_ent_tr.l_algo_id :%ld:",st_ord_ent_tr.l_algo_id);
  fn_userlog(c_ServiceName,"st_ord_ent_tr.si_algo_category :%d:",st_ord_ent_tr.si_algo_category);
		************************************/

		fn_orstonse_char(st_ord_ent_tr.c_reserved,
						 60,
						 " ",
						 1);

		break;
	case MODIFY:
	case CANCEL:

		if (ptr_st_rqst1->c_req_typ == MODIFY)
		{
			st_ord_mod_tr.si_transaction_code = ORDER_MOD_IN_TR;
		}
		else if (ptr_st_rqst1->c_req_typ == CANCEL)
		{
			st_ord_mod_tr.si_transaction_code = ORDER_CANCEL_IN_TR;
		}
		else
		{
			fn_errlog(c_ServiceName, "L31130", " Invalid Request Type ", c_err_msg);
			return -1;
		}

		st_ord_mod_tr.li_userid = (long int)atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id);

		switch (ptr_st_opm_pipe_mstr->si_user_typ_glb)
		{
		case 0:
			c_user_typ_glb = TRADER;
			break;

		case 4:
			c_user_typ_glb = CORPORATE_MANAGER;
			break;

		case 5:
			c_user_typ_glb = BRANCH_MANAGER;
			break;

		default:
			fn_errlog(c_ServiceName, "L31135", " Invalid USER Type ", c_err_msg);
			return -1;
			break;
		}

		st_ord_mod_tr.c_modified_cancelled_by = c_user_typ_glb;

		st_ord_mod_tr.l_token_no = ptr_st_nse_cnt1->l_token_id;

		fn_orstonse_cntrct_desc_tr((struct vw_nse_cntrct *)ptr_st_nse_cnt1,
								   (struct st_contract_desc_tr *)&(st_ord_mod_tr.st_con_desc_tr),
								   c_ServiceName,
								   c_err_msg);

		st_ord_mod_tr.d_order_number = (double)atoll(ptr_st_ord1->c_xchng_ack);

		if (ptr_st_ord1->c_pro_cli_ind == BRKR_PLCD)
		{
			fn_orstonse_char(st_ord_mod_tr.c_account_number,
							 LEN_ACCOUNT_NUMBER,
							 " ",
							 1);
		}
		else
		{
			fn_orstonse_char(st_ord_mod_tr.c_account_number,
							 LEN_ACCOUNT_NUMBER,
							 ptr_st_ord1->c_cln_mtch_accnt,
							 strlen(ptr_st_ord1->c_cln_mtch_accnt));
		}

		switch (ptr_st_rqst1->c_slm_flg)
		{
		case 'S':
			st_ord_mod_tr.si_book_type = STOP_LOSS_MIT_ORDER;
			break;

		case 'L':
		case 'M':
			st_ord_mod_tr.si_book_type = REGULAR_LOT_ORDER;
			break;

		default:
			fn_errlog(c_ServiceName, "L31140", " Invalid slm flag ", c_err_msg);
			return -1;
			break;
		}

		switch (ptr_st_ord1->c_ordr_flw)
		{
		case 'B':
			st_ord_mod_tr.si_buy_sell_indicator = NSE_BUY;
			break;
		case 'S':
			st_ord_mod_tr.si_buy_sell_indicator = NSE_SELL;
			break;
		default:
			fn_errlog(c_ServiceName, "L31145", " Invalid order flow flag ", c_err_msg);
			return -1;
			break;
		}

		st_ord_mod_tr.li_disclosed_volume = ptr_st_rqst1->l_dsclsd_qty;

		st_ord_mod_tr.li_disclosed_volume_remaining = st_ord_mod_tr.li_disclosed_volume;

		if (ptr_st_rqst1->l_ord_tot_qty > ptr_st_ord1->l_ord_tot_qty)
		{
			st_ord_mod_tr.li_total_volume_remaining = (ptr_st_ord1->l_ord_tot_qty - ptr_st_ord1->l_exctd_qty);
		}
		else
		{
			st_ord_mod_tr.li_total_volume_remaining = (ptr_st_rqst1->l_ord_tot_qty - ptr_st_ord1->l_exctd_qty);
		}

		st_ord_mod_tr.li_volume = (ptr_st_rqst1->l_ord_tot_qty - ptr_st_ord1->l_exctd_qty);

		st_ord_mod_tr.li_volume_filled_today = ptr_st_ord1->l_exctd_qty_day;

		switch (ptr_st_rqst1->c_slm_flg)
		{
		case 'M':
			st_ord_mod_tr.li_price = 0;
			break;

		case 'L':
		case 'S':
			st_ord_mod_tr.li_price = ptr_st_rqst1->l_ord_lmt_rt;
			break;
		default:
			fn_errlog(c_ServiceName, "L31150", " Invalid slm flag ", c_err_msg);
			return -1;
			break;
		}

		switch (ptr_st_rqst1->c_ord_typ)
		{
		case 'T':
		case 'I':
			st_ord_mod_tr.li_good_till_date = 0;
			break;

		case 'D':
			fn_timearr_to_long(ptr_st_rqst1->c_valid_dt, &st_ord_mod_tr.li_good_till_date);
			break;
		default:
			fn_errlog(c_ServiceName, "L31155", " Invalid order type", c_err_msg);
			return -1;
			break;
		}

		fn_timearr_to_long(ptr_st_ord1->c_ack_tm, &(st_ord_mod_tr.li_entry_date_time));

		fn_timearr_to_long(ptr_st_ord1->c_prev_ack_tm, &(st_ord_mod_tr.li_last_modified));

		st_ord_mod_tr.st_ord_flg.flg_ato = 0;

		st_ord_mod_tr.st_ord_flg.flg_market = 0;

		switch (ptr_st_rqst1->c_slm_flg)
		{
		case 'M':
		case 'L':
			st_ord_mod_tr.st_ord_flg.flg_sl = 0;
			break;

		case 'S':
			st_ord_mod_tr.st_ord_flg.flg_sl = 1;
			break;
		default:
			fn_errlog(c_ServiceName, "L31160", " Invalid slm flag ", c_err_msg);
			return -1;
			break;
		}

		st_ord_mod_tr.st_ord_flg.flg_mit = 0;

		if (ptr_st_rqst1->c_ord_typ == 'T')
		{
			st_ord_mod_tr.st_ord_flg.flg_day = 1;
		}
		else
		{
			st_ord_mod_tr.st_ord_flg.flg_day = 0;
		}

		st_ord_mod_tr.st_ord_flg.flg_gtc = 0;

		if (ptr_st_rqst1->c_ord_typ == 'I')
		{
			st_ord_mod_tr.st_ord_flg.flg_ioc = 1;
		}
		else
		{
			st_ord_ent_tr.st_ord_flg.flg_ioc = 0;
		}

		st_ord_mod_tr.st_ord_flg.flg_aon = 0;
		st_ord_mod_tr.st_ord_flg.flg_mf = 0;
		st_ord_mod_tr.st_ord_flg.flg_matched_ind = 0;
		st_ord_mod_tr.st_ord_flg.flg_traded = 0;

		if (ptr_st_rqst1->c_req_typ == MODIFY)
		{
			st_ord_mod_tr.st_ord_flg.flg_modified = 1;
		}
		else
		{
			st_ord_mod_tr.st_ord_flg.flg_modified = 0;
		}

		st_ord_mod_tr.st_ord_flg.flg_frozen = 0;
		st_ord_mod_tr.st_ord_flg.flg_filler1 = 0;

		st_ord_mod_tr.si_branch_id = ptr_st_opm_pipe_mstr->li_opm_brnch_id;
		st_ord_mod_tr.li_trader_id = (long int)atoi(ptr_st_opm_pipe_mstr->c_opm_trdr_id);

		fn_orstonse_char(st_ord_mod_tr.c_broker_id,
						 LEN_BROKER_ID,
						 ptr_st_opm_pipe_mstr->c_xchng_brkr_id,
						 strlen(ptr_st_opm_pipe_mstr->c_xchng_brkr_id));

		st_ord_mod_tr.c_open_close = 'O';

		memset(st_ord_mod_tr.c_settlor, 0, LEN_SETTLOR);

		MEMSET(sql_icd_cust_type);

		EXEC SQL
			SELECT ICD_CUST_TYPE
				INTO : sql_icd_cust_type
						   FROM ICD_INFO_CLIENT_DTLS,
					   IAI_INFO_ACCOUNT_INFO
						   WHERE ICD_SERIAL_NO = IAI_SERIAL_NO
							   AND IAI_MATCH_ACCOUNT_NO = : ptr_st_ord1->c_cln_mtch_accnt;

		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "L31165", SQLMSG, c_err_msg);
			return -1;
		}

		SETNULL(sql_icd_cust_type);

		if (strcmp(sql_icd_cust_type.arr, "NRI") != 0)
		{
			if (ptr_st_ord1->c_pro_cli_ind == BRKR_PLCD)
			{
				fn_orstonse_char(st_ord_mod_tr.c_settlor,
								 LEN_SETTLOR,
								 " ",
								 1);

				st_ord_mod_tr.si_pro_client_indicator = NSE_PRO;
			}
			else
			{
				fn_orstonse_char(st_ord_mod_tr.c_settlor,
								 LEN_SETTLOR,
								 ptr_st_opm_pipe_mstr->c_xchng_brkr_id,
								 strlen(ptr_st_opm_pipe_mstr->c_xchng_brkr_id));

				st_ord_mod_tr.si_pro_client_indicator = NSE_CLIENT;
			}
		}
		else
		{
			fn_orstonse_char(st_ord_mod_tr.c_settlor,
							 LEN_SETTLOR,
							 ptr_st_ord1->c_settlor,
							 strlen(ptr_st_ord1->c_settlor));

			st_ord_mod_tr.si_pro_client_indicator = NSE_CLIENT;
		}

		st_ord_mod_tr.st_add_ord_flg.c_cover_uncover = 'V';

		st_ord_mod_tr.li_ordr_sqnc = ptr_st_rqst1->l_ord_seq;

		if (DEBUG_MSG_LVL_2)
		{
			fn_userlog(c_ServiceName, "c_prgm_flg :%c:", c_prgm_flg);
		}

		if (c_prgm_flg == 'A' || c_prgm_flg == 'T' || c_prgm_flg == 'Z' || c_prgm_flg == 'G')
		{
			strcat(ptr_st_ord1->c_ctcl_id, "000");
		}
		else
		{
			strcat(ptr_st_ord1->c_ctcl_id, "100");
		}

		st_ord_mod_tr.d_nnf_field = (double)atoll(ptr_st_ord1->c_ctcl_id);

		if (strcmp(c_pan_no, "*") != 0)
		{
			fn_orstonse_char(st_ord_mod_tr.c_pan,
							 LEN_PAN,
							 c_pan_no,
							 strlen(c_pan_no));
		}
		else
		{
			fn_orstonse_char(st_ord_mod_tr.c_pan,
							 LEN_PAN,
							 " ",
							 1);
		}

		if (c_prgm_flg == 'A')
		{
			st_ord_mod_tr.l_algo_id = FO_AUTO_MTM_ALG_ID;
			st_ord_mod_tr.si_algo_category = FO_AUTO_MTM_ALG_CAT_ID;
		}
		else if (c_prgm_flg == 'T')
		{
			st_ord_mod_tr.l_algo_id = FO_PRICE_IMP_ALG_ID;
			st_ord_mod_tr.si_algo_category = FO_PRICE_IMP_ALG_CAT_ID;
		}
		else if (c_prgm_flg == 'Z')
		{
			st_ord_mod_tr.l_algo_id = FO_PRFT_ORD_ALG_ID;
			st_ord_mod_tr.si_algo_category = FO_PRFT_ORD_ALG_CAT_ID;
		}
		else if (c_prgm_flg == 'G')
		{
			st_ord_mod_tr.l_algo_id = FO_FLASH_TRD_ALG_ID;
			st_ord_mod_tr.si_algo_category = FO_FLASH_TRD_ALG_CAT_ID;
		}
		else
		{
			st_ord_mod_tr.l_algo_id = FO_NON_ALG_ID;
			st_ord_mod_tr.si_algo_category = FO_NON_ALG_CAT_ID;
		}

		st_ord_mod_tr.ll_lastactivityref = atoll(c_lst_act_ref);

		fn_orstonse_char(st_ord_mod_tr.c_reserved,
						 52,
						 " ",
						 1);
		break;

	default:
		fn_errlog(c_ServiceName, "L31170", " Invalid Request Type ", c_err_msg);
		return -1;
		break;
	}

	/***Packing the structure to put in Queue **/

	if (ptr_st_rqst1->c_req_typ == NEW)
	{
		fn_cnvt_htn_new_ordr_req_tr(&st_ord_ent_tr);
		memcpy(&(st_exch_message.st_exch_sndmsg), &st_ord_ent_tr, sizeof(st_ord_ent_tr));
	}
	else
	{
		fn_cnvt_htn_modcan_req_tr(&st_ord_mod_tr);
		memcpy(&(st_exch_message.st_exch_sndmsg), &st_ord_mod_tr, sizeof(st_ord_mod_tr));
	}

	/*** Commmented in ver 2.8
   i_ch_val =  fn_get_reset_seq( ptr_st_rqst1->c_pipe_id,
							   ptr_st_rqst1->c_mod_trd_dt,
							   GET_PLACED_SEQ,
							   &i_snd_seq,
							   c_ServiceName,
							   c_err_msg);

***/
	/*** Added in Ver 2.8 ***/
	strcpy(st_s_sequence.c_pipe_id, ptr_st_rqst1->c_pipe_id);
	strcpy(st_s_sequence.c_trd_dt, ptr_st_rqst1->c_mod_trd_dt);
	st_s_sequence.c_rqst_typ = GET_PLACED_SEQ;

	i_ch_val = fn_get_fno_pack_seq(c_ServiceName,
								   &st_s_sequence,
								   c_err_msg);
	/*** Ver 2.8 Ends ***/

	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S00010", LIBMSG, c_err_msg);
		return -1;
	}
	/**   st_exch_message.st_net_header.i_seq_num = i_snd_seq;     Commented in version 2.8 **/
	st_exch_message.st_net_header.i_seq_num = st_s_sequence.i_seq_num; /** using st_s_sequence.i_seq_num in ver 2.8 **/

	if (DEBUG_MSG_LVL_1)
	{
		/**     fn_userlog(c_ServiceName,"The sequence number got is:%d:",i_snd_seq);      Commented in version 2.8  **/
		fn_userlog(c_ServiceName, "The sequence number got is:%d:", st_s_sequence.i_seq_num); /** using st_s_sequence.i_seq_num in ver 2.8 **/
	}

	/****call routine for md5 ******/

	if (ptr_st_rqst1->c_req_typ == NEW)
	{
		fn_cal_md5digest((void *)&st_ord_ent_tr, sizeof(st_ord_ent_tr), digest);
	}
	else
	{
		fn_cal_md5digest((void *)&st_ord_mod_tr, sizeof(st_ord_mod_tr), digest);
	}

	/*****Copy the checksum in network header *******/

	memcpy(st_exch_message.st_net_header.c_checksum, digest, sizeof(digest));

	/******Set length of the network header *******/

	if (ptr_st_rqst1->c_req_typ == NEW)
	{
		st_exch_message.st_net_header.si_message_length = sizeof(st_exch_message.st_net_header) + sizeof(st_ord_ent_tr);
	}
	else
	{
		st_exch_message.st_net_header.si_message_length = sizeof(st_exch_message.st_net_header) + sizeof(st_ord_mod_tr);
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "The message length is:%d:", st_exch_message.st_net_header.si_message_length);
	}

	/***Packing the structure to in Queue datatype  **/

	fn_cnvt_htn_net_hdr(&st_exch_message.st_net_header);

	switch (ptr_st_rqst1->c_req_typ)
	{
	case NEW:
		ptr_st_q_packet->li_msg_type = BOARD_LOT_IN_TR;
		break;

	case MODIFY:
		ptr_st_q_packet->li_msg_type = ORDER_MOD_IN_TR;
		break;

	case CANCEL:
		ptr_st_q_packet->li_msg_type = ORDER_CANCEL_IN_TR;
		break;
	}

	/**** Commented in Ver 2.7 ******
	fn_userlog(c_ServiceName,"**Bhushan-li_msg_type :%ld:",ptr_st_q_packet->li_msg_type);
	***************************/

	memcpy(&(ptr_st_q_packet->st_exch_msg_data), &st_exch_message, sizeof(st_exch_message));

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, " Data copied in queue packet");
	}
}
/*** Ver 2.2 Ends ***/

/***** Ver 2.8 Starts *****/

int fn_get_fno_pack_seq(char *c_ServiceName, struct st_pk_sequence *ptr_st_i_seq, char *c_errmsg)
{

	char c_err_msg[256];

	varchar c_trd_dt[23];

	int i_trnsctn;

	if (DEBUG_MSG_LVL_5)
	{
		fn_userlog(c_ServiceName, "SPAN  Inside Function fn_get_fno_pack_seq.");
	}

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "fn_get_fno_pack_seq : c_rqst_typ :%c:", ptr_st_i_seq->c_rqst_typ);
	}

	strcpy((char *)c_trd_dt.arr, ptr_st_i_seq->c_trd_dt);
	SETLEN(c_trd_dt);

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "c_pipe_id  :%s:", ptr_st_i_seq->c_pipe_id);
		fn_userlog(c_ServiceName, "c_trd_dt  :%s:", c_trd_dt.arr);
		fn_userlog(c_ServiceName, "c_rqst_typ  :%c:", ptr_st_i_seq->c_rqst_typ);
	}

	switch (ptr_st_i_seq->c_rqst_typ)
	{
	case GET_PLACED_SEQ:
		EXEC SQL
			UPDATE fsp_fo_seq_plcd
				SET fsp_seq_num = fsp_seq_num + 1 WHERE fsp_pipe_id = : ptr_st_i_seq->c_pipe_id
																			AND fsp_trd_dt = to_date( : c_trd_dt, 'dd-Mon-yyyy')
																								 RETURNING fsp_seq_num INTO : ptr_st_i_seq->i_seq_num;

		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "L31175", SQLMSG, c_err_msg);
			return -1;
		}
		break;

	default:

		strcpy(c_errmsg, "Invalid Operation Type");
		fn_errlog(c_ServiceName, "L31180", DEFMSG, c_errmsg);
		return -1;
	}

	return 0;
}
/***** Ver 2.8 Ends *****/
