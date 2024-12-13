/** Ver 1.1 : Auto MTM Changes | Sachin Birje ***/
/** Ver 1.2 : Pack Client Error Handling Changes | Vardraj ***/
/** Ver 1.3 : NNF_CHANGES_7.21(PAN CHANGES)	|	Parag Kanojia ***/
/** Ver 1.5 : FO Direct Connvectivity	| Bhushan Harekar (14-Feb-2019) **/
/** Ver 1.6 : Last Activity Changes Rollover With Spread ( 09-Sep-2019 ) **/
/** Ver 1.6 : CR-ISEC14-169739 Markets App Algo Order Changes  Sachin Birje ( 04-Feb-2022 ) **/
/** Ver 1.7 : Blueshift algo changes  Sachin Birje ( 01-Sep-2022 ) **/
/** Ver 1.8 : Spira 1968 Optimization in sequence generation Devesh Chawda (29-May-2023) **/
/** Ver TOL : Tux on Linux Agam (04-Aug-2023)  **/
					/***** Ver 1.8 Starts *****/

struct st_opm_pipe_mstr; /* Added in Ver TOL */

struct st_pk_sequence
{
  int   i_seq_num;
  char  c_pipe_id[3];
  char  c_trd_dt[23];
  char  c_rqst_typ;
  char  c_rout_crt[4];
};

 				/***** Ver 1.8 Edns *****/

int fn_pack_exer_ord_to_nse(	struct vw_xchngbook *ptr_st_rqst1,
                          		struct vw_exrcbook   *ptr_st_exr,
                          		struct vw_nse_cntrct *ptr_st_nse_cnt1,
			  											struct st_opm_pipe_mstr *ptr_st_opm_pipe_mstr,
															struct st_req_q_data *ptr_st_q_packet,	
															char *c_ServiceName
                          	);

int fn_pack_ordnry_ord_to_nse(struct vw_xchngbook *ptr_st_rqst1,
                        			struct vw_orderbook *ptr_st_ord1,
                        			struct vw_nse_cntrct *ptr_st_nse_cnt1,
															struct st_opm_pipe_mstr *ptr_st_opm_pipe_mstr,
															struct st_req_q_data *ptr_st_q_packet,
                              char   c_prgm_flg,  /* Ver 1.1 */
															char *c_pan_no,     /*** Added In ver 1.3 ***/
															char *c_lst_act_ref, /*** Ver 1.5 ***/
															char *c_esp_id, /*** Ver 1.7 ***/
															char *c_algo_id, /*** Ver 1.7 ***/
															char c_source_flg, /*** Ver 1.7 ***/
 															char *c_ServiceName                       			
														);

int fn_pack_sprdord_to_nse( 	struct vw_xchngbook *ptr_st_rqst,
                        			struct vw_orderbook *ptr_st_ord,
                        			struct vw_nse_cntrct *ptr_st_nse_cnt,
                        			struct vw_spdordbk *ptr_st_spd_ord_bk,
															struct st_opm_pipe_mstr *ptr_st_opm_pipe_mstr,
															struct st_req_q_data *ptr_st_q_packet,
															char *c_pan_no,     /*** Added In ver 1.3 ***/
                              char *c_lst_act_ref, /*** Ver 1.6 ***/
                              char *c_esp_id, /*** Ver 1.7 ***/
                              char *c_algo_id, /*** Ver 1.7 ***/
                              char c_source_flg, /*** Ver 1.7 ***/
															char *c_ServiceName
													);

/*** Ver 1.5 Started ***/
int fn_pack_ordnry_ord_to_nse_tr(	struct vw_xchngbook *ptr_st_rqst1,
                              		struct vw_orderbook *ptr_st_ord1,
                              		struct vw_nse_cntrct *ptr_st_nse_cnt1,
                              		struct st_opm_pipe_mstr *ptr_st_opm_pipe_mstr,
                              		struct st_req_q_data *ptr_st_q_packet,
                              		char   c_prgm_flg,
                              		char *c_pan_no,    
																	char *c_lst_act_ref,
																	char *c_esp_id,  /** 1.7 **/
																	char *c_algo_id, /** 1.7 **/
																	char c_source_flg, /** 1.7 **/
                              		char *c_ServiceName
                            		);

/*** Ver 1.5 Ended ***/

