/**************************************************************************/
/* File 			 : fn_nw_to_host_format_nse.c																*/
/* Description : network to host conversion Function Definitions 					*/ 
/*                                                                        */
/* Ver 1.0		 : 13-Mar-2019	Suchita Dabir.															*/
/**************************************************************************/



/**** C header ****/
#include <string.h>
#include <stdlib.h>
#include <fn_log.h>
#include <netinet/in.h>
#include <ctype.h>
#include <fn_host_to_nw_format_nse.h>
#include<fo_exch_comnN.h>
#include <endian.h>  /* VER TOL : TUX on LINUX */

/* VER TOL : TUX on LINUX -- Replaced htonll with be64toh (Ravindra) */


/*** The logon process SIGN_ON_REQUEST_IN ****/
void fn_cnvt_htn_int_header ( struct st_int_header *ptr_st_int_header )
{
  ptr_st_int_header->si_transaction_code = htons(ptr_st_int_header->si_transaction_code);
	ptr_st_int_header->li_log_time         = htonl(ptr_st_int_header->li_log_time);
  ptr_st_int_header->li_trader_id        = htonl(ptr_st_int_header->li_trader_id);
  ptr_st_int_header->si_error_code       = htons(ptr_st_int_header->si_error_code);
  ptr_st_int_header->si_message_length   = htons(ptr_st_int_header->si_message_length);
}

void fn_cnvt_htn_net_hdr ( struct st_net_hdr *ptr_net_hdr)
{
	ptr_net_hdr->si_message_length = htons(ptr_net_hdr->si_message_length);
	ptr_net_hdr->i_seq_num = htonl(ptr_net_hdr->i_seq_num);
}

/*** The logon process SIGN_ON_REQUEST_IN ****/

void fn_cnvt_htn_sign_on_req_in ( struct st_sign_on_req *ptr_st_sign_on_req )
{
	fn_cnvt_htn_int_header(&ptr_st_sign_on_req->st_hdr);
  ptr_st_sign_on_req->li_user_id = htonl(ptr_st_sign_on_req->li_user_id );
  ptr_st_sign_on_req->li_last_password_change_date = htonl(ptr_st_sign_on_req->li_last_password_change_date);
  ptr_st_sign_on_req->si_branch_id = htons(ptr_st_sign_on_req->si_branch_id );
	ptr_st_sign_on_req->li_version_number   = htonl(ptr_st_sign_on_req->li_version_number );
	ptr_st_sign_on_req->li_batch_2_start_time = htonl(ptr_st_sign_on_req->li_batch_2_start_time);
	ptr_st_sign_on_req->si_user_type = htons(ptr_st_sign_on_req->si_user_type);
  ptr_st_sign_on_req->d_sequence_number = be64toh(ptr_st_sign_on_req->d_sequence_number ); /** double find **/
  /** ptr_st_sign_on_req->st_mkt_allwd_lst.flg_normal_mkt
  ptr_st_sign_on_req->st_mkt_allwd_lst.flg_oddlot_mkt
  ptr_st_sign_on_req->st_mkt_allwd_lst.flg_spot_mkt
  ptr_st_sign_on_req->st_mkt_allwd_lst.flg_auction_mkt
  ptr_st_sign_on_req->st_mkt_allwd_lst.flg_filler1
  ptr_st_sign_on_req->st_mkt_allwd_lst.flg_filler2 ** bits format ***/
	ptr_st_sign_on_req->si_member_type = htons(ptr_st_sign_on_req->si_member_type );
}


/***** The logon process SYSTEM_INFORMATION_IN ********/

void fn_cnvt_htn_sys_info_req ( struct st_system_info_req *ptr_st_sys_inf_req)
{
	 fn_cnvt_htn_int_header( &ptr_st_sys_inf_req->st_hdr);
  ptr_st_sys_inf_req->li_last_update_protfolio_time = htonl(ptr_st_sys_inf_req->li_last_update_protfolio_time);
}


/*****  The logon process UPDATE_LOCALDB_IN  *********/
void fn_cnvt_to_htn_upd_local_db ( struct st_update_local_database *ptr_st_upd_local_db)
{
   fn_cnvt_htn_int_header( &ptr_st_upd_local_db->st_hdr );
  ptr_st_upd_local_db->li_last_update_security_time    = htonl( ptr_st_upd_local_db->li_last_update_security_time);
  ptr_st_upd_local_db->li_last_update_participant_time = htonl(	ptr_st_upd_local_db->li_last_update_participant_time);
	ptr_st_upd_local_db->li_last_update_instrument_time = htonl(ptr_st_upd_local_db->li_last_update_instrument_time );
	ptr_st_upd_local_db->li_last_update_index_time      = htonl(ptr_st_upd_local_db->li_last_update_index_time);
  ptr_st_upd_local_db->st_mkt_stts.si_normal    = htons(ptr_st_upd_local_db->st_mkt_stts.si_normal);
	ptr_st_upd_local_db->st_mkt_stts.si_oddlot    = htons(ptr_st_upd_local_db->st_mkt_stts.si_oddlot);
	ptr_st_upd_local_db->st_mkt_stts.si_spot      = htons(ptr_st_upd_local_db->st_mkt_stts.si_spot);
	ptr_st_upd_local_db->st_mkt_stts.si_auction   = htons(ptr_st_upd_local_db->st_mkt_stts.si_auction);
  ptr_st_upd_local_db->st_ex_mkt_stts.si_normal  = htons(ptr_st_upd_local_db->st_ex_mkt_stts.si_normal);
	ptr_st_upd_local_db->st_ex_mkt_stts.si_oddlot  = htons (ptr_st_upd_local_db->st_ex_mkt_stts.si_oddlot);
	ptr_st_upd_local_db->st_ex_mkt_stts.si_spot     = htons(ptr_st_upd_local_db->st_ex_mkt_stts.si_spot);
	ptr_st_upd_local_db->st_ex_mkt_stts.si_auction  =htons (ptr_st_upd_local_db->st_ex_mkt_stts.si_auction);
  ptr_st_upd_local_db->st_pl_mkt_stts.si_normal = htons( ptr_st_upd_local_db->st_pl_mkt_stts.si_normal );    
	ptr_st_upd_local_db->st_pl_mkt_stts.si_oddlot	= htons (ptr_st_upd_local_db->st_pl_mkt_stts.si_oddlot);    
	ptr_st_upd_local_db->st_pl_mkt_stts.si_spot		 = htons(ptr_st_upd_local_db->st_pl_mkt_stts.si_spot);    
	ptr_st_upd_local_db->st_pl_mkt_stts.si_auction =htons (ptr_st_upd_local_db->st_pl_mkt_stts.si_auction);     
}


/***** SIGN_OFF_REQUEST_IN *****
void fn_cnvt_htn_sign_off_req( ){} **/

/***** DOWNLOAD_REQUEST 7000 ***/
void fn_cnvt_htn_dwld_req( struct st_message_download *ptr_st_message_download )
{
     fn_cnvt_htn_int_header( &ptr_st_message_download->st_hdr );
		 ptr_st_message_download->d_sequence_number = be64toh( ptr_st_message_download->d_sequence_number ); /** find double **/
}

/*** contract desc structure non trim ***/
void fn_cnvt_htn_cntrct_desc( struct  st_contract_desc *ptr_st_contract_desc )
{
     ptr_st_contract_desc->li_expiry_date = htonl( ptr_st_contract_desc->li_expiry_date );
     ptr_st_contract_desc->li_strike_price = htonl( ptr_st_contract_desc->li_strike_price );
		 ptr_st_contract_desc->si_ca_level     = htons (ptr_st_contract_desc->si_ca_level);
}


/** order flags structure ***/
void fn_cnvt_htn_ordr_flg_struct( struct st_order_flags *ptr_st_order_flags )
{ /** find usigned int **
    ptr_st_order_flags->flg_ato         
  	ptr_st_order_flags->flg_market		
		ptr_st_order_flags->flg_sl			
		ptr_st_order_flags->flg_mit		
		ptr_st_order_flags->flg_day			
		ptr_st_order_flags->flg_gtc		
		ptr_st_order_flags->flg_ioc		
		ptr_st_order_flags->flg_aon			
		ptr_st_order_flags->flg_mf					
		ptr_st_order_flags->flg_matched_ind	
		ptr_st_order_flags->flg_traded		
		ptr_st_order_flags->flg_modified
		ptr_st_order_flags->flg_frozen	
		ptr_st_order_flags->flg_filler1 ***/
}

/*** BOARD_LOT_IN:20000 ***/
void fn_cnvt_htn_new_ordr_req_tr( struct st_oe_reqres_tr *ptr_st_oe_reqres_tr )
{
		ptr_st_oe_reqres_tr->si_transaction_code  = htons(ptr_st_oe_reqres_tr->si_transaction_code);
	  ptr_st_oe_reqres_tr->li_userid            = htonl(ptr_st_oe_reqres_tr->li_userid);          
	  ptr_st_oe_reqres_tr->si_reason_code       = htons(ptr_st_oe_reqres_tr->si_reason_code);     
	  ptr_st_oe_reqres_tr->l_token_no           = htonl(ptr_st_oe_reqres_tr->l_token_no );           
	  ptr_st_oe_reqres_tr->st_con_desc_tr.li_expiry_date = htonl(ptr_st_oe_reqres_tr->st_con_desc_tr.li_expiry_date);
    ptr_st_oe_reqres_tr->st_con_desc_tr.li_strike_price =  htonl(ptr_st_oe_reqres_tr->st_con_desc_tr.li_strike_price);
	  ptr_st_oe_reqres_tr->si_book_type          = htons (ptr_st_oe_reqres_tr->si_book_type);
	  ptr_st_oe_reqres_tr->si_buy_sell_indicator = htons (ptr_st_oe_reqres_tr->si_buy_sell_indicator);
		ptr_st_oe_reqres_tr->li_disclosed_volume   = htonl (ptr_st_oe_reqres_tr->li_disclosed_volume);
		ptr_st_oe_reqres_tr->li_volume             = htonl (ptr_st_oe_reqres_tr->li_volume);
    ptr_st_oe_reqres_tr->li_price              = htonl (ptr_st_oe_reqres_tr->li_price);
		ptr_st_oe_reqres_tr->li_good_till_date     = htonl (ptr_st_oe_reqres_tr->li_good_till_date);
		fn_cnvt_htn_ordr_flg_struct( &ptr_st_oe_reqres_tr->st_ord_flg );
    ptr_st_oe_reqres_tr->si_branch_id= htons (  ptr_st_oe_reqres_tr->si_branch_id);    
 		ptr_st_oe_reqres_tr->li_trader_id= htonl (	ptr_st_oe_reqres_tr->li_trader_id);
		ptr_st_oe_reqres_tr->si_pro_client_indicator  = htons (	ptr_st_oe_reqres_tr->si_pro_client_indicator);
		ptr_st_oe_reqres_tr->li_ordr_sqnc= htonl (	ptr_st_oe_reqres_tr->li_ordr_sqnc);
		ptr_st_oe_reqres_tr->d_nnf_field = be64toh (	ptr_st_oe_reqres_tr->d_nnf_field); /** find double **/
		ptr_st_oe_reqres_tr->l_algo_id = htonl ( ptr_st_oe_reqres_tr->l_algo_id);
		ptr_st_oe_reqres_tr->si_algo_category = htons (	ptr_st_oe_reqres_tr->si_algo_category);
}


/*** ORDER_MOD_IN_TR (20040),ORDER_CANCEL_IN_TR (20070),ORDER_QUICK_CANCEL_IN_TR (20060) ***/
void fn_cnvt_htn_modcan_req_tr( struct st_om_rqst_tr *ptr_st_om_rqst_tr )
{
	  ptr_st_om_rqst_tr->si_transaction_code = htons(ptr_st_om_rqst_tr->si_transaction_code );
		ptr_st_om_rqst_tr->li_userid = htonl ( ptr_st_om_rqst_tr->li_userid);
		ptr_st_om_rqst_tr->l_token_no = htonl ( ptr_st_om_rqst_tr->l_token_no);
	  ptr_st_om_rqst_tr->st_con_desc_tr.li_expiry_date = htonl(ptr_st_om_rqst_tr->st_con_desc_tr.li_expiry_date);
    ptr_st_om_rqst_tr->st_con_desc_tr.li_strike_price = htonl	(ptr_st_om_rqst_tr->st_con_desc_tr.li_strike_price);
    ptr_st_om_rqst_tr->st_con_desc_tr.li_expiry_date = htonl(ptr_st_om_rqst_tr->st_con_desc_tr.li_expiry_date);
    ptr_st_om_rqst_tr->st_con_desc_tr.li_strike_price=htonl(ptr_st_om_rqst_tr->st_con_desc_tr.li_strike_price);
		ptr_st_om_rqst_tr->d_order_number = be64toh(ptr_st_om_rqst_tr->d_order_number);     /** find double **/
		ptr_st_om_rqst_tr->si_book_type = htons(ptr_st_om_rqst_tr->si_book_type);
		ptr_st_om_rqst_tr->si_buy_sell_indicator = htons (ptr_st_om_rqst_tr->si_buy_sell_indicator);
		ptr_st_om_rqst_tr->li_disclosed_volume = htonl (ptr_st_om_rqst_tr->li_disclosed_volume);
    ptr_st_om_rqst_tr->li_disclosed_volume_remaining = htonl(ptr_st_om_rqst_tr->li_disclosed_volume_remaining);
		ptr_st_om_rqst_tr->li_total_volume_remaining = htonl (	ptr_st_om_rqst_tr->li_total_volume_remaining);
		ptr_st_om_rqst_tr->li_volume = htonl (ptr_st_om_rqst_tr->li_volume);
		ptr_st_om_rqst_tr->li_volume_filled_today = htonl (	ptr_st_om_rqst_tr->li_volume_filled_today);
		ptr_st_om_rqst_tr->li_price = htonl (	ptr_st_om_rqst_tr->li_price);
		ptr_st_om_rqst_tr->li_good_till_date = htonl(ptr_st_om_rqst_tr->li_good_till_date);
		ptr_st_om_rqst_tr->li_entry_date_time = htonl(ptr_st_om_rqst_tr->li_entry_date_time);
		ptr_st_om_rqst_tr->li_last_modified = htonl (ptr_st_om_rqst_tr->li_last_modified);
		fn_cnvt_htn_ordr_flg_struct( &ptr_st_om_rqst_tr->st_ord_flg );
		ptr_st_om_rqst_tr->si_branch_id = htons (ptr_st_om_rqst_tr->si_branch_id);
		ptr_st_om_rqst_tr->li_trader_id = htonl (ptr_st_om_rqst_tr->li_trader_id);
		ptr_st_om_rqst_tr->si_pro_client_indicator	 = htons (ptr_st_om_rqst_tr->si_pro_client_indicator);
    ptr_st_om_rqst_tr->li_ordr_sqnc = htonl (ptr_st_om_rqst_tr->li_ordr_sqnc);
		ptr_st_om_rqst_tr->d_nnf_field = be64toh (ptr_st_om_rqst_tr->d_nnf_field); /** find double **/
    ptr_st_om_rqst_tr->ll_lastactivityref = be64toh(ptr_st_om_rqst_tr->ll_lastactivityref);
		ptr_st_om_rqst_tr->l_algo_id = htonl (ptr_st_om_rqst_tr->l_algo_id);
		ptr_st_om_rqst_tr->si_algo_category = htons (ptr_st_om_rqst_tr->si_algo_category);
}

/*** SP_BOARD_LOT_IN,TWOL_BOARD_LOT_IN,THRL_BOARD_LOT_IN ***/
void fn_cnvt_htn_spd2L3L_req ( struct st_spd_oe_reqres *ptr_st_spd_oe_reqres )
{
    fn_cnvt_htn_int_header( &ptr_st_spd_oe_reqres->st_hdr );
		ptr_st_spd_oe_reqres->si_competitor_period = htons( ptr_st_spd_oe_reqres->si_competitor_period);
    ptr_st_spd_oe_reqres->si_solicitor_period  = htons(	ptr_st_spd_oe_reqres->si_solicitor_period );
		ptr_st_spd_oe_reqres->si_reason_code= htons(	ptr_st_spd_oe_reqres->si_reason_code );
		ptr_st_spd_oe_reqres->l_token= htonl(	ptr_st_spd_oe_reqres->l_token);
		fn_cnvt_htn_cntrct_desc( &ptr_st_spd_oe_reqres->st_cntrct_desc );
		ptr_st_spd_oe_reqres->si_order_type= htons(ptr_st_spd_oe_reqres->si_order_type );
		ptr_st_spd_oe_reqres->d_order_number= be64toh(ptr_st_spd_oe_reqres->d_order_number );/** find double **/
		ptr_st_spd_oe_reqres->si_book_type= htons(ptr_st_spd_oe_reqres->si_book_type );
		ptr_st_spd_oe_reqres->si_buy_sell= htons(ptr_st_spd_oe_reqres->si_buy_sell );
		ptr_st_spd_oe_reqres->li_disclosed_vol  = htonl(ptr_st_spd_oe_reqres->li_disclosed_vol );
		ptr_st_spd_oe_reqres->li_disclosed_vol_remaining =htonl(ptr_st_spd_oe_reqres->li_disclosed_vol_remaining );
		ptr_st_spd_oe_reqres->li_total_vol_remaining= htonl (ptr_st_spd_oe_reqres->li_total_vol_remaining );
		ptr_st_spd_oe_reqres->li_volume= htonl (ptr_st_spd_oe_reqres->li_volume );
		ptr_st_spd_oe_reqres->li_volume_filled_today= htonl (ptr_st_spd_oe_reqres->li_volume_filled_today );
		ptr_st_spd_oe_reqres->li_price= htonl (ptr_st_spd_oe_reqres->li_price);
		ptr_st_spd_oe_reqres->li_trigger_price= htonl (ptr_st_spd_oe_reqres->li_trigger_price);
		ptr_st_spd_oe_reqres->li_good_till_date= htonl (ptr_st_spd_oe_reqres->li_good_till_date	);
		ptr_st_spd_oe_reqres->li_entry_date_time= htonl (ptr_st_spd_oe_reqres->li_entry_date_time);
		ptr_st_spd_oe_reqres->li_min_fill_aon = htonl (ptr_st_spd_oe_reqres->li_min_fill_aon	);
		ptr_st_spd_oe_reqres->li_last_modified = htonl (ptr_st_spd_oe_reqres->li_last_modified);
		fn_cnvt_htn_ordr_flg_struct( &ptr_st_spd_oe_reqres->st_order_flgs );
		ptr_st_spd_oe_reqres->si_branch_id = htons (ptr_st_spd_oe_reqres->si_branch_id);
		ptr_st_spd_oe_reqres->li_trader_id = htonl (ptr_st_spd_oe_reqres->li_trader_id);
		ptr_st_spd_oe_reqres->si_pro_client = htons (ptr_st_spd_oe_reqres->si_pro_client);
		ptr_st_spd_oe_reqres->si_settlement_period = htons (ptr_st_spd_oe_reqres->si_settlement_period);
		ptr_st_spd_oe_reqres->i_sprd_seq_no = htonl (ptr_st_spd_oe_reqres->i_sprd_seq_no	);
    ptr_st_spd_oe_reqres->d_nnf_field = be64toh (ptr_st_spd_oe_reqres->d_nnf_field	);/** find double **/
		ptr_st_spd_oe_reqres->d_filler19 = be64toh (ptr_st_spd_oe_reqres->d_filler19	);/** find double **/
    ptr_st_spd_oe_reqres->ll_lastactivityref = be64toh(ptr_st_spd_oe_reqres->ll_lastactivityref);
		ptr_st_spd_oe_reqres->l_algo_id = htonl (ptr_st_spd_oe_reqres->l_algo_id);
		ptr_st_spd_oe_reqres->si_algo_category = htons (ptr_st_spd_oe_reqres->si_algo_category  );
		ptr_st_spd_oe_reqres->li_spd_price_diff = htonl (ptr_st_spd_oe_reqres->li_spd_price_diff);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[0].l_token = htonl( ptr_st_spd_oe_reqres->st_spd_lg_inf[0].l_token );
		fn_cnvt_htn_cntrct_desc( &ptr_st_spd_oe_reqres->st_spd_lg_inf[0].st_cntrct_desc );
    ptr_st_spd_oe_reqres->st_spd_lg_inf[0].si_order_type = htons (ptr_st_spd_oe_reqres->st_spd_lg_inf[0].si_order_type);                                 
		ptr_st_spd_oe_reqres->st_spd_lg_inf[0].si_buy_sell = htons (ptr_st_spd_oe_reqres->st_spd_lg_inf[0].si_buy_sell);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_disclosed_vol = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_disclosed_vol);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_disclosed_vol_remaining	= htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_disclosed_vol_remaining);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_total_vol_remaining = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_total_vol_remaining);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_volume = htonl(ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_volume);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_volume_filled_today = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_volume_filled_today);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_price = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_price);
    ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_trigger_price = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_trigger_price);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_min_fill_aon = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[0].li_min_fill_aon);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].l_token = htonl( ptr_st_spd_oe_reqres->st_spd_lg_inf[1].l_token );
		fn_cnvt_htn_cntrct_desc( &ptr_st_spd_oe_reqres->st_spd_lg_inf[1].st_cntrct_desc );
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].si_order_type = htons (ptr_st_spd_oe_reqres->st_spd_lg_inf[1].si_order_type);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].si_buy_sell=htons(ptr_st_spd_oe_reqres->st_spd_lg_inf[1].si_buy_sell);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_disclosed_vol = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_disclosed_vol);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_disclosed_vol_remaining	= htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_disclosed_vol_remaining);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_total_vol_remaining = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_total_vol_remaining);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_volume =htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_volume);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_volume_filled_today = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_volume_filled_today);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_price = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_price);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_trigger_price = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_trigger_price);
		ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_min_fill_aon = htonl (ptr_st_spd_oe_reqres->st_spd_lg_inf[1].li_min_fill_aon);
		
}


/*** EX_PL_ENTRY_IN,EX_PL_MOD_IN,EX_PL_CXL_IN ***/
void fn_cnvt_htn_new_exer_req ( struct st_ex_pl_reqres *ptr_st_ex_pl_reqres )
{
    fn_cnvt_htn_int_header( &ptr_st_ex_pl_reqres->st_hdr );
		ptr_st_ex_pl_reqres->si_reason_code = htons ( ptr_st_ex_pl_reqres->si_reason_code );
    ptr_st_ex_pl_reqres->st_ex_pl_data.l_token = htonl ( ptr_st_ex_pl_reqres->st_ex_pl_data.l_token);
		fn_cnvt_htn_cntrct_desc(&ptr_st_ex_pl_reqres->st_ex_pl_data.st_cntrct_desc);
		ptr_st_ex_pl_reqres->st_ex_pl_data.si_expl_flag= htons( ptr_st_ex_pl_reqres->st_ex_pl_data.si_expl_flag);
		ptr_st_ex_pl_reqres->st_ex_pl_data.d_expl_number= htonl( ptr_st_ex_pl_reqres->st_ex_pl_data.d_expl_number);
		ptr_st_ex_pl_reqres->st_ex_pl_data.si_market_type=htons(ptr_st_ex_pl_reqres->st_ex_pl_data.si_market_type);
		ptr_st_ex_pl_reqres->st_ex_pl_data.li_quantity= htonl( ptr_st_ex_pl_reqres->st_ex_pl_data.li_quantity);
		ptr_st_ex_pl_reqres->st_ex_pl_data.si_pro_cli= htons( ptr_st_ex_pl_reqres->st_ex_pl_data.si_pro_cli);
		ptr_st_ex_pl_reqres->st_ex_pl_data.si_exercise_type	= htons( ptr_st_ex_pl_reqres->st_ex_pl_data.si_exercise_type);
		ptr_st_ex_pl_reqres->st_ex_pl_data.li_entry_date_tm	= htonl( ptr_st_ex_pl_reqres->st_ex_pl_data.li_entry_date_tm);
		ptr_st_ex_pl_reqres->st_ex_pl_data.si_branch_id= htons( ptr_st_ex_pl_reqres->st_ex_pl_data.si_branch_id);
		ptr_st_ex_pl_reqres->st_ex_pl_data.li_trader_id= htonl( ptr_st_ex_pl_reqres->st_ex_pl_data.li_trader_id);
}

void fn_cnvt_htn_oe_reqres( struct st_oe_reqres *ptr_oe_reqres )
{
  fn_cnvt_htn_int_header( &ptr_oe_reqres->st_hdr);
  ptr_oe_reqres->si_competitor_period                 = htons(ptr_oe_reqres->si_competitor_period);
  ptr_oe_reqres->si_solicitor_period                  = htons(ptr_oe_reqres->si_solicitor_period);
  ptr_oe_reqres->si_reason_code                       = htons(ptr_oe_reqres->si_reason_code);
  ptr_oe_reqres->l_token_no                           = htonl(ptr_oe_reqres->l_token_no);
  fn_cnvt_htn_cntrct_desc(&ptr_oe_reqres->st_con_desc);
  ptr_oe_reqres->si_order_type                        = htons(ptr_oe_reqres->si_order_type);
  ptr_oe_reqres->d_order_number						  = be64toh(ptr_oe_reqres->d_order_number);  /*doubt*/
  ptr_oe_reqres->si_book_type                         = htons(ptr_oe_reqres->si_book_type);
  ptr_oe_reqres->si_buy_sell_indicator                = htons(ptr_oe_reqres->si_buy_sell_indicator);
  ptr_oe_reqres->li_disclosed_volume                  = htonl(ptr_oe_reqres->li_disclosed_volume);
  ptr_oe_reqres->li_disclosed_volume_remaining        = htonl(ptr_oe_reqres->li_disclosed_volume_remaining);
  ptr_oe_reqres->li_total_volume_remaining            = htonl(ptr_oe_reqres->li_total_volume_remaining);
  ptr_oe_reqres->li_volume                            = htonl(ptr_oe_reqres->li_volume);
  ptr_oe_reqres->li_volume_filled_today               = htonl(ptr_oe_reqres->li_volume_filled_today);
  ptr_oe_reqres->li_price                             = htonl(ptr_oe_reqres->li_price);
  ptr_oe_reqres->li_trigger_price                     = htonl(ptr_oe_reqres->li_trigger_price);
  ptr_oe_reqres->li_good_till_date                    = htonl(ptr_oe_reqres->li_good_till_date);
  ptr_oe_reqres->li_entry_date_time                   = htonl(ptr_oe_reqres->li_entry_date_time);
  ptr_oe_reqres->li_minimum_fill_aon_volume           = htonl(ptr_oe_reqres->li_minimum_fill_aon_volume);
  ptr_oe_reqres->li_last_modified                     = htonl(ptr_oe_reqres->li_last_modified);
  ptr_oe_reqres->si_branch_id                         = htons(ptr_oe_reqres->si_branch_id);
  ptr_oe_reqres->li_trader_id                         = htonl(ptr_oe_reqres->li_trader_id);
  ptr_oe_reqres->si_pro_client_indicator              = htons(ptr_oe_reqres->si_pro_client_indicator);
  ptr_oe_reqres->si_settlement_period                 = htons(ptr_oe_reqres->si_settlement_period);
  ptr_oe_reqres->i_ordr_sqnc                          = htonl(ptr_oe_reqres->i_ordr_sqnc);  /*int doubt*/
  ptr_oe_reqres->d_nnf_field                          = be64toh(ptr_oe_reqres->d_nnf_field); /*double doubt*/
  ptr_oe_reqres->d_filler19                           = be64toh(ptr_oe_reqres->d_filler19);  /*double doubt*/
  ptr_oe_reqres->l_algo_id                            = htonl(ptr_oe_reqres->l_algo_id);
  ptr_oe_reqres->si_algo_category                     = htons(ptr_oe_reqres->si_algo_category);
  ptr_oe_reqres->ll_lastactivityref                   = be64toh(ptr_oe_reqres->ll_lastactivityref);
}
