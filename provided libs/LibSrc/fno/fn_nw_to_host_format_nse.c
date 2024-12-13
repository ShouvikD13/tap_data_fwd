/**************************************************************************/
/* File        : fn_nw_to_host_format.c ( Parag K. )                      */
/* Description : network to host conversion Function Definitions          */
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

/* VER TOL : TUX on LINUX -- Changed be64toh with be64toh (Ravindra) */

void fn_cnvt_nth_int_header(struct st_int_header *ptr_int_header)
{
	ptr_int_header->si_transaction_code   =	ntohs(ptr_int_header->si_transaction_code);
	ptr_int_header->li_log_time           =	ntohl(ptr_int_header->li_log_time);
	ptr_int_header->li_trader_id          =	ntohl(ptr_int_header->li_trader_id);
	ptr_int_header->si_error_code         = ntohs(ptr_int_header->si_error_code);
	ptr_int_header->si_message_length     =	ntohs(ptr_int_header->si_message_length);
}

void fn_cnvt_nth_net_hdr ( struct st_net_hdr *ptr_net_hdr)
{
  ptr_net_hdr->si_message_length = ntohs(ptr_net_hdr->si_message_length);
  ptr_net_hdr->i_seq_num = ntohl(ptr_net_hdr->i_seq_num);
}

void fn_cnvt_nth_inner_header(struct st_inner_header *ptr_inner_header)
{
	ptr_inner_header->li_log_time         = ntohl(ptr_inner_header->li_log_time);
	ptr_inner_header->si_transaction_code = ntohs(ptr_inner_header->si_transaction_code);
	ptr_inner_header->si_error_code       = ntohs(ptr_inner_header->si_error_code);
	ptr_inner_header->si_message_length   = ntohs(ptr_inner_header->si_message_length);
}

void fn_cnvt_nth_cntrct_desc( struct  st_contract_desc *ptr_contract_desc )
{
  ptr_contract_desc->li_expiry_date  = ntohl( ptr_contract_desc->li_expiry_date );
  ptr_contract_desc->li_strike_price = ntohl( ptr_contract_desc->li_strike_price );
  ptr_contract_desc->si_ca_level     = ntohs (ptr_contract_desc->si_ca_level);
}

void fn_cnvt_nth_invt_pckt(struct	st_invt_pckt *ptr_invt_pckt)
{
	fn_cnvt_nth_int_header(&ptr_invt_pckt->st_hdr);
	ptr_invt_pckt->si_inv_cnt             = ntohs(ptr_invt_pckt->si_inv_cnt);
}

void fn_cnvt_nth_sign_on_res(struct	st_sign_on_res *ptr_sign_on_res)
{
	fn_cnvt_nth_int_header(&ptr_sign_on_res->st_hdr);
	ptr_sign_on_res->li_user_id           = ntohl(ptr_sign_on_res->li_user_id);
	ptr_sign_on_res->li_last_password_change_date  = ntohl(ptr_sign_on_res->li_last_password_change_date);
	ptr_sign_on_res->si_branch_id         = ntohs(ptr_sign_on_res->si_branch_id);
	ptr_sign_on_res->li_version_number    = ntohl(ptr_sign_on_res->li_version_number);
	ptr_sign_on_res->li_end_time          = ntohl(ptr_sign_on_res->li_end_time);
	ptr_sign_on_res->si_user_type         = ntohs(ptr_sign_on_res->si_user_type);
	ptr_sign_on_res->d_sequence_number    = ntohl(ptr_sign_on_res->d_sequence_number);	/*** double doubt***/
	// COMMENTED IN VER  commented VER TOL : TUX ON LINUX (Ravindra) ptr_sign_on_res->st_mkt_allwd_lst     = ntohl(ptr_sign_on_res->st_mkt_allwd_lst);		/* st_broker_eligibility_per_mkt doubt */	
	ptr_sign_on_res->si_member_type       = ntohs(ptr_sign_on_res->si_member_type);
}

void fn_cnvt_nth_system_info_data(struct st_system_info_data *ptr_system_info_data)
{
	fn_cnvt_nth_int_header(&ptr_system_info_data->st_hdr);
	ptr_system_info_data->st_mkt_stts.si_normal  = ntohs(ptr_system_info_data->st_mkt_stts.si_normal);
  ptr_system_info_data->st_mkt_stts.si_oddlot  = ntohs(ptr_system_info_data->st_mkt_stts.si_oddlot);
  ptr_system_info_data->st_mkt_stts.si_spot    = ntohs(ptr_system_info_data->st_ex_mkt_stts.si_spot);
  ptr_system_info_data->st_mkt_stts.si_auction = ntohs(ptr_system_info_data->st_ex_mkt_stts.si_auction);
  ptr_system_info_data->st_pl_mkt_stts.si_normal  = ntohs(ptr_system_info_data->st_pl_mkt_stts.si_normal);
	ptr_system_info_data->st_ex_mkt_stts.si_oddlot  = ntohs(ptr_system_info_data->st_ex_mkt_stts.si_oddlot);
	ptr_system_info_data->st_ex_mkt_stts.si_oddlot  = ntohs(ptr_system_info_data->st_ex_mkt_stts.si_normal);
	ptr_system_info_data->st_ex_mkt_stts.si_spot    = ntohs(ptr_system_info_data->st_ex_mkt_stts.si_spot);
	ptr_system_info_data->st_ex_mkt_stts.si_auction = ntohs(ptr_system_info_data->st_ex_mkt_stts.si_auction);
	ptr_system_info_data->st_pl_mkt_stts.si_normal  = ntohs(ptr_system_info_data->st_pl_mkt_stts.si_normal);
  ptr_system_info_data->st_pl_mkt_stts.si_oddlot  = ntohs(ptr_system_info_data->st_pl_mkt_stts.si_oddlot);
  ptr_system_info_data->st_pl_mkt_stts.si_spot    = ntohs(ptr_system_info_data->st_pl_mkt_stts.si_spot);
  ptr_system_info_data->st_pl_mkt_stts.si_auction = ntohs(ptr_system_info_data->st_pl_mkt_stts.si_auction);
	ptr_system_info_data->li_market_index           = ntohl(ptr_system_info_data->li_market_index);
	ptr_system_info_data->si_default_sttlmnt_period_nm  = ntohs(ptr_system_info_data->si_default_sttlmnt_period_nm);
	ptr_system_info_data->si_default_sttlmnt_period_sp  = ntohs(ptr_system_info_data->si_default_sttlmnt_period_sp);
	ptr_system_info_data->si_default_sttlmnt_period_au  = ntohs(ptr_system_info_data->si_default_sttlmnt_period_au);
	ptr_system_info_data->si_competitor_period          = ntohs(ptr_system_info_data->si_competitor_period);
	ptr_system_info_data->si_solicitor_period           = ntohs(ptr_system_info_data->si_solicitor_period);
	ptr_system_info_data->si_warning_percent            = ntohs(ptr_system_info_data->si_warning_percent);
	ptr_system_info_data->si_volume_freeze_percent      = ntohs(ptr_system_info_data->si_volume_freeze_percent);
	ptr_system_info_data->li_board_lot_quantity         = ntohl(ptr_system_info_data->li_board_lot_quantity);
	ptr_system_info_data->li_tick_size                  = ntohl(ptr_system_info_data->li_tick_size);
	ptr_system_info_data->si_maximum_gtc_days						= ntohs(ptr_system_info_data->si_maximum_gtc_days);
	ptr_system_info_data->st_stk_elg_ind = ptr_system_info_data->st_stk_elg_ind;		/*doubt st_stock_eligible_indicators */
	ptr_system_info_data->si_disclosed_quantity_percent_allowed  = ntohs(ptr_system_info_data->si_disclosed_quantity_percent_allowed);
	ptr_system_info_data->li_risk_free_interest_rate    = ntohl(ptr_system_info_data->li_risk_free_interest_rate);
}

void fn_cnvt_nth_oe_reqres( struct st_oe_reqres *ptr_oe_reqres)
{
	fn_cnvt_nth_int_header(&ptr_oe_reqres->st_hdr);	
	ptr_oe_reqres->si_competitor_period                 = ntohs(ptr_oe_reqres->si_competitor_period);
	ptr_oe_reqres->si_solicitor_period                  = ntohs(ptr_oe_reqres->si_solicitor_period);
	ptr_oe_reqres->si_reason_code                       = ntohs(ptr_oe_reqres->si_reason_code);
	ptr_oe_reqres->l_token_no                           = ntohl(ptr_oe_reqres->l_token_no);
	fn_cnvt_nth_cntrct_desc(&ptr_oe_reqres->st_con_desc);
	ptr_oe_reqres->si_order_type												= ntohs(ptr_oe_reqres->si_order_type);
	ptr_oe_reqres->d_order_number = be64toh(ptr_oe_reqres->d_order_number);	/*doubt*/
	ptr_oe_reqres->si_book_type                         = ntohs(ptr_oe_reqres->si_book_type);
	ptr_oe_reqres->si_buy_sell_indicator                = ntohs(ptr_oe_reqres->si_buy_sell_indicator);
	ptr_oe_reqres->li_disclosed_volume									= ntohl(ptr_oe_reqres->li_disclosed_volume);
	ptr_oe_reqres->li_disclosed_volume_remaining        = ntohl(ptr_oe_reqres->li_disclosed_volume_remaining);
	ptr_oe_reqres->li_total_volume_remaining            = ntohl(ptr_oe_reqres->li_total_volume_remaining);
	ptr_oe_reqres->li_volume                            = ntohl(ptr_oe_reqres->li_volume);
	ptr_oe_reqres->li_volume_filled_today               = ntohl(ptr_oe_reqres->li_volume_filled_today);
	ptr_oe_reqres->li_price                             = ntohl(ptr_oe_reqres->li_price);
	ptr_oe_reqres->li_trigger_price											= ntohl(ptr_oe_reqres->li_trigger_price);
	ptr_oe_reqres->li_good_till_date                    = ntohl(ptr_oe_reqres->li_good_till_date);
	ptr_oe_reqres->li_entry_date_time										= ntohl(ptr_oe_reqres->li_entry_date_time);
	ptr_oe_reqres->li_minimum_fill_aon_volume						= ntohl(ptr_oe_reqres->li_minimum_fill_aon_volume);
	ptr_oe_reqres->li_last_modified                     = ntohl(ptr_oe_reqres->li_last_modified);
	ptr_oe_reqres->si_branch_id                         = ntohs(ptr_oe_reqres->si_branch_id);
	ptr_oe_reqres->li_trader_id                         = ntohl(ptr_oe_reqres->li_trader_id);
	ptr_oe_reqres->si_pro_client_indicator							= ntohs(ptr_oe_reqres->si_pro_client_indicator);
	ptr_oe_reqres->si_settlement_period								  = ntohs(ptr_oe_reqres->si_settlement_period);
	ptr_oe_reqres->i_ordr_sqnc                          = ntohl(ptr_oe_reqres->i_ordr_sqnc);	/*int doubt*/
	ptr_oe_reqres->d_nnf_field                          = be64toh(ptr_oe_reqres->d_nnf_field);	/*double doubt*/
	ptr_oe_reqres->d_filler19                           = be64toh(ptr_oe_reqres->d_filler19);	/*double doubt*/
	ptr_oe_reqres->l_algo_id                            = ntohl(ptr_oe_reqres->l_algo_id);
	ptr_oe_reqres->si_algo_category                     = ntohs(ptr_oe_reqres->si_algo_category);
	ptr_oe_reqres->ll_lastactivityref                   = be64toh(ptr_oe_reqres->ll_lastactivityref);
}

void fn_cnvt_nth_local_database_data(struct st_local_database_data *ptr_local_database_data)
{
	fn_cnvt_nth_int_header(&ptr_local_database_data->st_hdr);
	/*** doubt ***/
}

void fn_cnvt_nth_spd_oe_reqres(struct st_spd_oe_reqres *ptr_spd_oe_reqres)
{
	int i=0;
	fn_cnvt_nth_int_header(&ptr_spd_oe_reqres->st_hdr);
	ptr_spd_oe_reqres->si_competitor_period             = ntohs(ptr_spd_oe_reqres->si_competitor_period);
	ptr_spd_oe_reqres->si_solicitor_period              = ntohs(ptr_spd_oe_reqres->si_solicitor_period);
	ptr_spd_oe_reqres->si_reason_code                   = ntohs(ptr_spd_oe_reqres->si_reason_code);
	ptr_spd_oe_reqres->l_token                          = ntohl(ptr_spd_oe_reqres->l_token);
	fn_cnvt_nth_cntrct_desc(&ptr_spd_oe_reqres->st_cntrct_desc);
	ptr_spd_oe_reqres->si_order_type                    = ntohs(ptr_spd_oe_reqres->si_order_type);
	ptr_spd_oe_reqres->d_order_number = be64toh(ptr_spd_oe_reqres->d_order_number);	/*double doubt*/
	ptr_spd_oe_reqres->si_book_type											= ntohs(ptr_spd_oe_reqres->si_book_type);
	ptr_spd_oe_reqres->si_buy_sell                      = ntohs(ptr_spd_oe_reqres->si_buy_sell);
	ptr_spd_oe_reqres->li_disclosed_vol                 = ntohl(ptr_spd_oe_reqres->li_disclosed_vol);
	ptr_spd_oe_reqres->li_disclosed_vol_remaining       = ntohl(ptr_spd_oe_reqres->li_disclosed_vol_remaining);
	ptr_spd_oe_reqres->li_total_vol_remaining           = ntohl(ptr_spd_oe_reqres->li_total_vol_remaining);
	ptr_spd_oe_reqres->li_volume                        = ntohl(ptr_spd_oe_reqres->li_volume);
	ptr_spd_oe_reqres->li_volume_filled_today           = ntohl(ptr_spd_oe_reqres->li_volume_filled_today);
	ptr_spd_oe_reqres->li_price													= ntohl(ptr_spd_oe_reqres->li_price);
	ptr_spd_oe_reqres->li_trigger_price                 = ntohl(ptr_spd_oe_reqres->li_trigger_price);
	ptr_spd_oe_reqres->li_good_till_date                = ntohl(ptr_spd_oe_reqres->li_good_till_date);
	ptr_spd_oe_reqres->li_entry_date_time               = ntohl(ptr_spd_oe_reqres->li_entry_date_time);
	ptr_spd_oe_reqres->li_min_fill_aon                  = ntohl(ptr_spd_oe_reqres->li_min_fill_aon);
	ptr_spd_oe_reqres->li_last_modified                 = ntohl(ptr_spd_oe_reqres->li_last_modified);
	ptr_spd_oe_reqres->si_branch_id                     = ntohs(ptr_spd_oe_reqres->si_branch_id);
	ptr_spd_oe_reqres->li_trader_id											= ntohl(ptr_spd_oe_reqres->li_trader_id);
	ptr_spd_oe_reqres->si_pro_client                    = ntohs(ptr_spd_oe_reqres->si_pro_client);
	ptr_spd_oe_reqres->si_settlement_period             = ntohs(ptr_spd_oe_reqres->si_settlement_period);
	ptr_spd_oe_reqres->i_sprd_seq_no = ntohl(ptr_spd_oe_reqres->i_sprd_seq_no);	/*int doubt */
	ptr_spd_oe_reqres->d_nnf_field                      = ntohl(ptr_spd_oe_reqres->d_nnf_field);/*doible doubt*/
	ptr_spd_oe_reqres->d_filler19                       = ntohl(ptr_spd_oe_reqres->d_filler19);/*doible doubt */
	ptr_spd_oe_reqres->l_algo_id                        = ntohl(ptr_spd_oe_reqres->l_algo_id);
	ptr_spd_oe_reqres->si_algo_category                 = ntohs(ptr_spd_oe_reqres->si_algo_category);
	ptr_spd_oe_reqres->ll_lastactivityref = be64toh(ptr_spd_oe_reqres->ll_lastactivityref);
	ptr_spd_oe_reqres->li_spd_price_diff                = ntohl(ptr_spd_oe_reqres->li_spd_price_diff);
	for(i=0;i<2;i++)
  {	
	ptr_spd_oe_reqres->st_spd_lg_inf[i].l_token            = ntohl(ptr_spd_oe_reqres->st_spd_lg_inf[i].l_token);
	fn_cnvt_nth_cntrct_desc(&ptr_spd_oe_reqres->st_spd_lg_inf[i].st_cntrct_desc);
	ptr_spd_oe_reqres->st_spd_lg_inf[i].si_order_type      = ntohs(ptr_spd_oe_reqres->st_spd_lg_inf[i].si_order_type);
	ptr_spd_oe_reqres->st_spd_lg_inf[i].si_buy_sell        = ntohs(ptr_spd_oe_reqres->st_spd_lg_inf[i].si_buy_sell);
	ptr_spd_oe_reqres->st_spd_lg_inf[i].li_disclosed_vol   = ntohl(ptr_spd_oe_reqres->st_spd_lg_inf[i].li_disclosed_vol);
	ptr_spd_oe_reqres->st_spd_lg_inf[i].li_disclosed_vol_remaining  = ntohl(ptr_spd_oe_reqres->st_spd_lg_inf[i].li_disclosed_vol_remaining);
	ptr_spd_oe_reqres->st_spd_lg_inf[i].li_total_vol_remaining      = ntohl(ptr_spd_oe_reqres->st_spd_lg_inf[i].li_total_vol_remaining);
	ptr_spd_oe_reqres->st_spd_lg_inf[i].li_volume                   = ntohl(ptr_spd_oe_reqres->st_spd_lg_inf[i].li_volume);
	ptr_spd_oe_reqres->st_spd_lg_inf[i].li_volume_filled_today      = ntohl(ptr_spd_oe_reqres->st_spd_lg_inf[i].li_volume_filled_today);
	ptr_spd_oe_reqres->st_spd_lg_inf[i].li_price                    = ntohl(ptr_spd_oe_reqres->st_spd_lg_inf[i].li_price);
	ptr_spd_oe_reqres->st_spd_lg_inf[i].li_trigger_price            = ntohl(ptr_spd_oe_reqres->st_spd_lg_inf[i].li_trigger_price);
	ptr_spd_oe_reqres->st_spd_lg_inf[i].li_min_fill_aon             = ntohl(ptr_spd_oe_reqres->st_spd_lg_inf[i].li_min_fill_aon);
	}
}

void fn_cnvt_nth_trade_confirm(struct st_trade_confirm *ptr_trade_confirm)
{
	fn_cnvt_nth_int_header(&ptr_trade_confirm->st_hdr);
	ptr_trade_confirm->d_response_order_number=ntohl(ptr_trade_confirm->d_response_order_number);	/*double doubt*/
	ptr_trade_confirm->li_trader_number       = ntohl(ptr_trade_confirm->li_trader_number);
	ptr_trade_confirm->si_buy_sell_indicator  = ntohs(ptr_trade_confirm->si_buy_sell_indicator);
	ptr_trade_confirm->li_original_volume     = ntohl(ptr_trade_confirm->li_original_volume);
	ptr_trade_confirm->li_disclosed_volume    = ntohl(ptr_trade_confirm->li_disclosed_volume);
	ptr_trade_confirm->li_remaining_volume    = ntohl(ptr_trade_confirm->li_remaining_volume);
	ptr_trade_confirm->li_disclosed_volume_remaining  = ntohl(ptr_trade_confirm->li_disclosed_volume_remaining);
	ptr_trade_confirm->li_price                       = ntohl(ptr_trade_confirm->li_price);
	ptr_trade_confirm->li_good_till_date              = ntohl(ptr_trade_confirm->li_good_till_date);
	ptr_trade_confirm->li_fill_number                 = ntohl(ptr_trade_confirm->li_fill_number);
	ptr_trade_confirm->li_fill_quantity               = ntohl(ptr_trade_confirm->li_fill_quantity);
	ptr_trade_confirm->li_fill_price                  = ntohl(ptr_trade_confirm->li_fill_price);
	ptr_trade_confirm->li_volume_filled_today         = ntohl(ptr_trade_confirm->li_volume_filled_today);
	ptr_trade_confirm->li_activity_time               = ntohl(ptr_trade_confirm->li_activity_time);
	ptr_trade_confirm->d_counter_trade_order_number   = be64toh(ptr_trade_confirm->d_counter_trade_order_number);	/*double doubt*/
	ptr_trade_confirm->l_token                        = ntohl(ptr_trade_confirm->l_token);
	fn_cnvt_nth_cntrct_desc(&ptr_trade_confirm->st_con_desc);
	ptr_trade_confirm->li_new_volume                  = ntohl(ptr_trade_confirm->li_new_volume);
	ptr_trade_confirm->l_algo_id                      = ntohl(ptr_trade_confirm->l_algo_id);
	ptr_trade_confirm->si_algo_category               = ntohs(ptr_trade_confirm->si_algo_category);
	ptr_trade_confirm->ll_lastactivityref             = be64toh(ptr_trade_confirm->ll_lastactivityref);
}

void fn_cnvt_nth_ex_pl_reqres(struct st_ex_pl_reqres *ptr_ex_pl_reqres)
{
	fn_cnvt_nth_int_header(&ptr_ex_pl_reqres->st_hdr);
	ptr_ex_pl_reqres->si_reason_code            = ntohs(ptr_ex_pl_reqres->si_reason_code);
	ptr_ex_pl_reqres->st_ex_pl_data.l_token     = ntohl(ptr_ex_pl_reqres->st_ex_pl_data.l_token);
	fn_cnvt_nth_cntrct_desc(&ptr_ex_pl_reqres->st_ex_pl_data.st_cntrct_desc);
	ptr_ex_pl_reqres->st_ex_pl_data.si_expl_flag  = ntohs(ptr_ex_pl_reqres->st_ex_pl_data.si_expl_flag);
	ptr_ex_pl_reqres->st_ex_pl_data.d_expl_number = be64toh(ptr_ex_pl_reqres->st_ex_pl_data.d_expl_number);	/*double doubt*/
	ptr_ex_pl_reqres->st_ex_pl_data.si_market_type = ntohs(ptr_ex_pl_reqres->st_ex_pl_data.si_market_type);
	ptr_ex_pl_reqres->st_ex_pl_data.li_quantity    = ntohl(ptr_ex_pl_reqres->st_ex_pl_data.li_quantity);	/*double doubt*/
	ptr_ex_pl_reqres->st_ex_pl_data.si_pro_cli     = ntohs(ptr_ex_pl_reqres->st_ex_pl_data.si_pro_cli);
	ptr_ex_pl_reqres->st_ex_pl_data.si_exercise_type  = ntohs(ptr_ex_pl_reqres->st_ex_pl_data.si_exercise_type);
	ptr_ex_pl_reqres->st_ex_pl_data.li_entry_date_tm  = ntohl(ptr_ex_pl_reqres->st_ex_pl_data.li_entry_date_tm);
	ptr_ex_pl_reqres->st_ex_pl_data.si_branch_id      = ntohs(ptr_ex_pl_reqres->st_ex_pl_data.si_branch_id);
	ptr_ex_pl_reqres->st_ex_pl_data.li_trader_id      = ntohl(ptr_ex_pl_reqres->st_ex_pl_data.li_trader_id);
}

void fn_cnvt_nth_security_update_info(struct st_security_update_info *ptr_security_update_info)
{
  fn_cnvt_nth_inner_header(&ptr_security_update_info->st_hdr);
	ptr_security_update_info->l_token           = ntohl(ptr_security_update_info->l_token);
	ptr_security_update_info->st_sc_info.li_expiry_date  = ntohl(ptr_security_update_info->st_sc_info.li_expiry_date);
	ptr_security_update_info->st_sc_info.li_strike_price = ntohl(ptr_security_update_info->st_sc_info.li_strike_price);
	ptr_security_update_info->st_sc_info.si_ca_level     = ntohs(ptr_security_update_info->st_sc_info.si_ca_level);
	ptr_security_update_info->si_permited_trade          = ntohs(ptr_security_update_info->si_permited_trade);
	ptr_security_update_info->d_issued_capital           = be64toh(ptr_security_update_info->d_issued_capital);
	ptr_security_update_info->li_warning_quantity        = ntohl(ptr_security_update_info->li_warning_quantity);
	ptr_security_update_info->li_freeze_quantity         = ntohl(ptr_security_update_info->li_freeze_quantity);
	ptr_security_update_info->si_issue_rate              = ntohs(ptr_security_update_info->si_issue_rate);
	ptr_security_update_info->li_issue_start_date        = ntohl(ptr_security_update_info->li_issue_start_date);
	ptr_security_update_info->li_interest_payment_date   = ntohl(ptr_security_update_info->li_interest_payment_date);
	ptr_security_update_info->li_issue_maturity_date     = ntohl(ptr_security_update_info->li_issue_maturity_date);
	ptr_security_update_info->li_margin_percentage       = ntohl(ptr_security_update_info->li_margin_percentage);
	ptr_security_update_info->li_minimum_lot_quantity    = ntohl(ptr_security_update_info->li_minimum_lot_quantity);
	ptr_security_update_info->li_board_lot_quantity      = ntohl(ptr_security_update_info->li_board_lot_quantity);
	ptr_security_update_info->li_tick_size               = ntohl(ptr_security_update_info->li_tick_size);
	ptr_security_update_info->li_listing_date            = ntohl(ptr_security_update_info->li_listing_date);
	ptr_security_update_info->li_expulsion_date          = ntohl(ptr_security_update_info->li_expulsion_date);
	ptr_security_update_info->li_readmission_date        = ntohl(ptr_security_update_info->li_readmission_date);
	ptr_security_update_info->li_record_date             = ntohl(ptr_security_update_info->li_record_date);
	ptr_security_update_info->li_low_price_range         = ntohl(ptr_security_update_info->li_low_price_range);
	ptr_security_update_info->li_high_price_range        = ntohl(ptr_security_update_info->li_high_price_range);
	ptr_security_update_info->li_expiry_date             = ntohl(ptr_security_update_info->li_expiry_date);
	ptr_security_update_info->li_nd_start_date           = ntohl(ptr_security_update_info->li_nd_start_date);
	ptr_security_update_info->li_nd_end_date             = ntohl(ptr_security_update_info->li_nd_end_date);
	ptr_security_update_info->st_elg_ind = ptr_security_update_info->st_elg_ind;	/*st_elg_ind doubt bit */
	ptr_security_update_info->li_book_closure_start_date = ntohl(ptr_security_update_info->li_book_closure_start_date);
	ptr_security_update_info->li_book_closure_end_date   = ntohl(ptr_security_update_info->li_book_closure_end_date);
	ptr_security_update_info->li_exercise_start_date     = ntohl(ptr_security_update_info->li_exercise_start_date);
	ptr_security_update_info->li_exercise_end_date       = ntohl(ptr_security_update_info->li_exercise_end_date);
	ptr_security_update_info->l_old_token                = ntohl(ptr_security_update_info->l_old_token);
	ptr_security_update_info->l_asset_token              = ntohl(ptr_security_update_info->l_asset_token);
	ptr_security_update_info->li_intrinsic_value         = ntohl(ptr_security_update_info->li_intrinsic_value);
	ptr_security_update_info->li_extrinsic_value         = ntohl(ptr_security_update_info->li_extrinsic_value);
	ptr_security_update_info->st_pps = ptr_security_update_info->st_pps;	/*st_pps doubt bit */
	ptr_security_update_info->li_local_update_date_time  = ntohl(ptr_security_update_info->li_local_update_date_time);
}

/*(void fn_cnvt_nth_security_status_update_info(struct st_security_status_update_info *ptr_security_status_update_info)
{
  fn_cnvt_nth_inner_header(&ptr_security_status_update_info->st_hdr);
	ptr_security_status_update_info->si_number_of_records  = ntohs(ptr_security_status_update_info->si_number_of_records);
	ptr_security_status_update_info->st_t_and_e[1].l_token    = ntohl(ptr_security_status_update_info->st_t_and_e[1].l_token);		*** doubt [?] ***
	ptr_security_status_update_info->st_t_and_e.l_token.st_sec_stts[1].si_stts = ntohs(ptr_security_status_update_info->st_t_and_e.l_token.st_sec_stts[1].si_stts);		 *** doubt [?] ***
}*/

void fn_cnvt_nth_participant_update_info(struct st_participant_update_info *ptr_participant_update_info)
{
  fn_cnvt_nth_inner_header(&ptr_participant_update_info->st_hdr);
	ptr_participant_update_info->li_participant_update_date_time = ntohl(ptr_participant_update_info->li_participant_update_date_time);
}

void fn_cnvt_nth_trader_int_msg(struct st_trader_int_msg *ptr_trader_int_msg)
{
	fn_cnvt_nth_int_header(&ptr_trader_int_msg->st_hdr);
	ptr_trader_int_msg->li_trader_id 								= ntohl(ptr_trader_int_msg->li_trader_id);
	ptr_trader_int_msg->si_broadcast_message_length = ntohs(ptr_trader_int_msg->si_broadcast_message_length);
}

/*void fn_cnvt_nth_brd_exch_exception(struct st_brd_exch_exception *ptr_brd_exch_exception)
{
	fn_cnvt_nth_int_header(&ptr_brd_exch_exception->st_hdr);
	ptr_brd_exch_exception->streamnumber = ntohs(ptr_brd_exch_exception->streamnumber);
	ptr_brd_exch_exception->status = ntohs(ptr_brd_exch_exception->status);
}*/

void fn_cnvt_nth_cntrct_desc_tr( struct  st_contract_desc_tr *ptr_contract_desc_tr )
{
  ptr_contract_desc_tr->li_expiry_date  = ntohl( ptr_contract_desc_tr->li_expiry_date );
  ptr_contract_desc_tr->li_strike_price = ntohl( ptr_contract_desc_tr->li_strike_price );
}

void fn_cnvt_nth_oe_reqres_tr(struct st_oe_reqres_tr *ptr_oe_reqres_tr)
{
	ptr_oe_reqres_tr->si_transaction_code = ntohs(ptr_oe_reqres_tr->si_transaction_code);
	ptr_oe_reqres_tr->li_userid = ntohl(ptr_oe_reqres_tr->li_userid);
	ptr_oe_reqres_tr->si_reason_code = ntohs(ptr_oe_reqres_tr->si_reason_code);
	ptr_oe_reqres_tr->l_token_no = ntohl(ptr_oe_reqres_tr->l_token_no);
	fn_cnvt_nth_cntrct_desc_tr(&ptr_oe_reqres_tr->st_con_desc_tr);
	ptr_oe_reqres_tr->si_book_type = ntohs(ptr_oe_reqres_tr->si_book_type);
	ptr_oe_reqres_tr->si_buy_sell_indicator = ntohs(ptr_oe_reqres_tr->si_buy_sell_indicator);
	ptr_oe_reqres_tr->li_disclosed_volume = ntohl(ptr_oe_reqres_tr->li_disclosed_volume);
	ptr_oe_reqres_tr->li_volume = ntohl(ptr_oe_reqres_tr->li_volume);
	ptr_oe_reqres_tr->li_price = ntohl(ptr_oe_reqres_tr->li_price);
	ptr_oe_reqres_tr->li_good_till_date = ntohl(ptr_oe_reqres_tr->li_good_till_date);
	ptr_oe_reqres_tr->si_branch_id = ntohs(ptr_oe_reqres_tr->si_branch_id);
	ptr_oe_reqres_tr->li_trader_id = ntohl(ptr_oe_reqres_tr->li_trader_id);
	ptr_oe_reqres_tr->si_pro_client_indicator = ntohs(ptr_oe_reqres_tr->si_pro_client_indicator);
	ptr_oe_reqres_tr->li_ordr_sqnc = ntohl(ptr_oe_reqres_tr->li_ordr_sqnc);
	ptr_oe_reqres_tr->d_nnf_field = be64toh(ptr_oe_reqres_tr->d_nnf_field);
	ptr_oe_reqres_tr->l_algo_id = ntohl(ptr_oe_reqres_tr->l_algo_id);
	ptr_oe_reqres_tr->si_algo_category = ntohs(ptr_oe_reqres_tr->si_algo_category);
}

void fn_cnvt_nth_om_rqst_tr(struct st_om_rqst_tr *ptr_om_rqst_tr)
{
	ptr_om_rqst_tr->si_transaction_code = ntohs(ptr_om_rqst_tr->si_transaction_code);
	ptr_om_rqst_tr->li_userid = ntohl(ptr_om_rqst_tr->li_userid);
	ptr_om_rqst_tr->l_token_no = ntohl(ptr_om_rqst_tr->l_token_no);
	fn_cnvt_nth_cntrct_desc_tr(&ptr_om_rqst_tr->st_con_desc_tr);
	ptr_om_rqst_tr->d_order_number = be64toh(ptr_om_rqst_tr->d_order_number);
	ptr_om_rqst_tr->si_book_type = ntohs(ptr_om_rqst_tr->si_book_type);
	ptr_om_rqst_tr->si_buy_sell_indicator = ntohs(ptr_om_rqst_tr->si_buy_sell_indicator);
	ptr_om_rqst_tr->li_disclosed_volume = ntohl(ptr_om_rqst_tr->li_disclosed_volume);
	ptr_om_rqst_tr->li_disclosed_volume_remaining = ntohl(ptr_om_rqst_tr->li_disclosed_volume_remaining);
	ptr_om_rqst_tr->li_total_volume_remaining = ntohl(ptr_om_rqst_tr->li_total_volume_remaining);
	ptr_om_rqst_tr->li_volume = ntohl(ptr_om_rqst_tr->li_volume);
	ptr_om_rqst_tr->li_volume_filled_today = ntohl(ptr_om_rqst_tr->li_volume_filled_today);
	ptr_om_rqst_tr->li_price = ntohl(ptr_om_rqst_tr->li_price);
	ptr_om_rqst_tr->li_good_till_date = ntohl(ptr_om_rqst_tr->li_good_till_date);
	ptr_om_rqst_tr->li_entry_date_time = ntohl(ptr_om_rqst_tr->li_entry_date_time);
	ptr_om_rqst_tr->li_last_modified = ntohl(ptr_om_rqst_tr->li_last_modified);
	ptr_om_rqst_tr->si_branch_id = ntohs(ptr_om_rqst_tr->si_branch_id);
	ptr_om_rqst_tr->li_trader_id = ntohl(ptr_om_rqst_tr->li_trader_id);
	ptr_om_rqst_tr->si_pro_client_indicator = ntohs(ptr_om_rqst_tr->si_pro_client_indicator);
	ptr_om_rqst_tr->li_ordr_sqnc = ntohl(ptr_om_rqst_tr->li_ordr_sqnc);
	ptr_om_rqst_tr->d_nnf_field = be64toh(ptr_om_rqst_tr->d_nnf_field);
	ptr_om_rqst_tr->l_algo_id = ntohl(ptr_om_rqst_tr->l_algo_id);
	ptr_om_rqst_tr->si_algo_category = ntohs(ptr_om_rqst_tr->si_algo_category);
	ptr_om_rqst_tr->ll_lastactivityref = be64toh(ptr_om_rqst_tr->ll_lastactivityref);
}

void fn_cnvt_nth_oe_rspn_tr(struct st_oe_rspn_tr *ptr_oe_rspn_tr)
{
	ptr_oe_rspn_tr->si_transaction_code = ntohs(ptr_oe_rspn_tr->si_transaction_code);
	ptr_oe_rspn_tr->li_log_time = ntohl(ptr_oe_rspn_tr->li_log_time);
	ptr_oe_rspn_tr->li_userid = ntohl(ptr_oe_rspn_tr->li_userid);
	ptr_oe_rspn_tr->si_error_code = ntohs(ptr_oe_rspn_tr->si_error_code);
	ptr_oe_rspn_tr->si_reason_code = ntohs(ptr_oe_rspn_tr->si_reason_code);
	ptr_oe_rspn_tr->l_token_no = ntohl(ptr_oe_rspn_tr->l_token_no);
	fn_cnvt_nth_cntrct_desc_tr(&ptr_oe_rspn_tr->st_con_desc_tr);
	ptr_oe_rspn_tr->d_order_number = be64toh(ptr_oe_rspn_tr->d_order_number);
	ptr_oe_rspn_tr->si_book_type = ntohs(ptr_oe_rspn_tr->si_book_type);
	ptr_oe_rspn_tr->si_buy_sell_indicator = ntohs(ptr_oe_rspn_tr->si_buy_sell_indicator);
	ptr_oe_rspn_tr->li_disclosed_volume = ntohl(ptr_oe_rspn_tr->li_disclosed_volume);
	ptr_oe_rspn_tr->li_disclosed_volume_remaining = ntohl(ptr_oe_rspn_tr->li_disclosed_volume_remaining);
	ptr_oe_rspn_tr->li_total_volume_remaining = ntohl(ptr_oe_rspn_tr->li_total_volume_remaining);
	ptr_oe_rspn_tr->li_volume = ntohl(ptr_oe_rspn_tr->li_volume);
	ptr_oe_rspn_tr->li_volume_filled_today = ntohl(ptr_oe_rspn_tr->li_volume_filled_today);
	ptr_oe_rspn_tr->li_price = ntohl(ptr_oe_rspn_tr->li_price);
	ptr_oe_rspn_tr->li_good_till_date = ntohl(ptr_oe_rspn_tr->li_good_till_date);
	ptr_oe_rspn_tr->li_entry_date_time = ntohl(ptr_oe_rspn_tr->li_entry_date_time);
	ptr_oe_rspn_tr->li_last_modified = ntohl(ptr_oe_rspn_tr->li_last_modified);
	ptr_oe_rspn_tr->si_branch_id = ntohs(ptr_oe_rspn_tr->si_branch_id);
	ptr_oe_rspn_tr->li_trader_id = ntohl(ptr_oe_rspn_tr->li_trader_id);
	ptr_oe_rspn_tr->si_pro_client_indicator = ntohs(ptr_oe_rspn_tr->si_pro_client_indicator);
	ptr_oe_rspn_tr->li_ordr_sqnc = ntohl(ptr_oe_rspn_tr->li_ordr_sqnc);
	ptr_oe_rspn_tr->d_nnf_field = be64toh(ptr_oe_rspn_tr->d_nnf_field);
	ptr_oe_rspn_tr->l_timestamp = ntohl(ptr_oe_rspn_tr->l_timestamp);
	ptr_oe_rspn_tr->l_algo_id = ntohl(ptr_oe_rspn_tr->l_algo_id);
	ptr_oe_rspn_tr->si_algo_category = ntohs(ptr_oe_rspn_tr->si_algo_category);
	ptr_oe_rspn_tr->ll_lastactivityref = be64toh(ptr_oe_rspn_tr->ll_lastactivityref);
}

void fn_cnvt_nth_trade_confirm_tr(struct st_trade_confirm_tr *ptr_trade_confirm_tr)
{
	ptr_trade_confirm_tr->si_transaction_code = ntohs(ptr_trade_confirm_tr->si_transaction_code);
	ptr_trade_confirm_tr->li_log_time = ntohl(ptr_trade_confirm_tr->li_log_time);
	ptr_trade_confirm_tr->li_trader_number = ntohl(ptr_trade_confirm_tr->li_trader_number);
	ptr_trade_confirm_tr->ll_timestamp = be64toh(ptr_trade_confirm_tr->ll_timestamp);
	ptr_trade_confirm_tr->d_timestamp2 = be64toh(ptr_trade_confirm_tr->d_timestamp2);
	ptr_trade_confirm_tr->d_response_order_number = be64toh(ptr_trade_confirm_tr->d_response_order_number);
	ptr_trade_confirm_tr->si_buy_sell_indicator = ntohs(ptr_trade_confirm_tr->si_buy_sell_indicator);
	ptr_trade_confirm_tr->li_original_volume = ntohl(ptr_trade_confirm_tr->li_original_volume);
	ptr_trade_confirm_tr->li_disclosed_volume = ntohl(ptr_trade_confirm_tr->li_disclosed_volume);
	ptr_trade_confirm_tr->li_remaining_volume = ntohl(ptr_trade_confirm_tr->li_remaining_volume);
	ptr_trade_confirm_tr->li_disclosed_volume_remaining = ntohl(ptr_trade_confirm_tr->li_disclosed_volume_remaining);
	ptr_trade_confirm_tr->li_price = ntohl(ptr_trade_confirm_tr->li_price);
	ptr_trade_confirm_tr->li_good_till_date = ntohl(ptr_trade_confirm_tr->li_good_till_date);
	ptr_trade_confirm_tr->li_fill_number = ntohl(ptr_trade_confirm_tr->li_fill_number);
	ptr_trade_confirm_tr->li_fill_quantity = ntohl(ptr_trade_confirm_tr->li_fill_quantity);
	ptr_trade_confirm_tr->li_fill_price = ntohl(ptr_trade_confirm_tr->li_fill_price);
	ptr_trade_confirm_tr->li_volume_filled_today = ntohl(ptr_trade_confirm_tr->li_volume_filled_today);
	ptr_trade_confirm_tr->li_activity_time = ntohl(ptr_trade_confirm_tr->li_activity_time);
	ptr_trade_confirm_tr->l_token = ntohl(ptr_trade_confirm_tr->l_token);
	fn_cnvt_nth_cntrct_desc_tr(&ptr_trade_confirm_tr->st_con_desc_tr);
	ptr_trade_confirm_tr->l_algo_id = ntohl(ptr_trade_confirm_tr->l_algo_id);
	ptr_trade_confirm_tr->si_algo_category = ntohs(ptr_trade_confirm_tr->si_algo_category);
	ptr_trade_confirm_tr->ll_lastactivityref = be64toh(ptr_trade_confirm_tr->ll_lastactivityref);
}

