/************************************************************************************************/
/*			File :fn_host_to_nw_format_nse.h	  ( Suchita D. )            													*/
/*			Description :Host To Network conversion headers								 													*/ 
/*																																		 													*/
/*																																		 													*/
/*																																		 													*/
/*																																							 								*/
/*																																		 													*/
/*																																															*/
/************************************************************************************************/

/*VER TOL : TUX on LINUX */
#pragma once
#include <fo_exch_comnN.h>

void fn_cnvt_htn_net_hdr ( struct st_net_hdr *ptr_net_hdr);
void fn_cnvt_htn_int_header ( struct st_int_header *ptr_st_int_header );
void fn_cnvt_htn_sign_on_req_in ( struct st_sign_on_req *ptr_st_sign_on_req );
void fn_cnvt_htn_sys_info_req ( struct st_system_info_req *ptr_st_sys_inf_req);
void fn_cnvt_to_htn_upd_local_db ( struct st_update_local_database *ptr_st_upd_local_db);
void fn_cnvt_htn_dwld_req( struct st_message_download *ptr_st_message_download );
void fn_cnvt_htn_cntrct_desc( struct  st_contract_desc *ptr_st_contract_desc );
void fn_cnvt_htn_ordr_flg_struct( struct st_order_flags *ptr_st_order_flags );
void fn_cnvt_htn_new_ordr_req_tr( struct st_oe_reqres_tr *ptr_st_oe_reqres_tr );
void fn_cnvt_htn_modcan_req_tr( struct st_om_rqst_tr *ptr_st_om_rqst_tr );
void fn_cnvt_htn_spd2L3L_req ( struct st_spd_oe_reqres *ptr_st_spd_oe_reqres );
void fn_cnvt_htn_new_exer_req ( struct st_ex_pl_reqres *ptr_st_ex_pl_reqres );
void fn_cnvt_htn_oe_reqres ( struct st_oe_reqres *ptr_oe_reqres );
