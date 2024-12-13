#pragma once                /* VER TOL : tux on linux */

struct vw_contract {
	long	l_eba_cntrct_id;		/* null=-1 */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	long	l_ca_lvl;		/* null=-1 */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_cntrct_qt {
	long	l_eba_cntrct_id;		/* null=-1 */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	long	l_ca_lvl;		/* null=-1 */
	long	l_lst_trdd_prc;		/* null=-1 */
	char	c_lst_trdd_time[23];		/* null="*" */
	long	l_lst_trdd_qty;		/* null=-1 */
	long	l_bst_bid_prc;		/* null=-1 */
	long	l_bst_bid_qty;		/* null=-1 */
	long	l_bst_offr_prc;		/* null=-1 */
	long	l_bst_offr_qty;		/* null=-1 */
	long	l_close_prc;		/* null=-1 */
	long	l_open_prc;		/* null=-1 */
	long	l_high_prc;		/* null=-1 */
	long	l_low_prc;		/* null=-1 */
	long	l_prev_close_prc;		/* null=-1 */
	long	l_chng_prvs_close_prc;		/* null=-1 */
	char	c_indctr;		/* null='*' */
	long	l_high_prc_rng;		/* null=-1 */
	long	l_low_prc_rng;		/* null=-1 */
	long	l_avg_trdd_prc;		/* null=-1 */
	long	l_tot_qty_trdd;		/* null=-1 */
	long	l_tot_val_trdd;		/* null=-1 */
	long	l_base_prc;		/* null=-1 */
	double	d_crnt_oi;		/* null=-1.000000 */
	double	d_chng_oi;		/* null=-1.000000 */
	double	d_oi_high;		/* null=-1.000000 */
	double	d_oi_low;		/* null=-1.000000 */
	char	c_oi_indctr[5];		/* null="*" */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
	long	l_min_lot_qty;		/* null=-1 */
	double	d_tot_val_trdd;		/* null=-1.000000 */
};


struct vw_cntrctlong_qt {
	long	l_eba_cntrct_id;		/* null=-1 */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	long	l_ca_lvl;		/* null=-1 */
	long	l_lst_trdd_prc;		/* null=-1 */
	char	c_lst_trdd_time[23];		/* null="*" */
	long	l_lst_trdd_qty;		/* null=-1 */
	long	l_tot_trdd_qty;		/* null=-1 */
	long	l_avg_trdd_prc;		/* null=-1 */
	long	l_bst_bid_prc[5];		/* null=-1 */
	long	l_bst_bid_qty[5];		/* null=-1 */
	long	l_bst_offr_prc[5];		/* null=-1 */
	long	l_bst_offr_qty[5];		/* null=-1 */
	long	l_tot_qty_trdd;		/* null=-1 */
	long	l_tot_val_trdd;		/* null=-1 */
	double	d_tot_bid_qty;		/* null=-1.000000 */
	double	d_tot_offr_qty;		/* null=-1.000000 */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
	double	d_tot_val_trdd;		/* null=-1.000000 */
};


struct vw_cntrt_gen_inf {
	long	l_eba_cntrct_id;		/* null=-1 */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	long	l_ca_lvl;		/* null=-1 */
	long	l_prmtd_to_trd;		/* null=-1 */
	long	l_stts;		/* null=-1 */
	long	l_min_lot_qty;		/* null=-1 */
	long	l_board_lot_qty;		/* null=-1 */
	long	l_tick_sz;		/* null=-1 */
	char	c_record_dt[23];		/* null="*" */
	char	c_nd_strt_dt[23];		/* null="*" */
	char	c_nd_end_dt[23];		/* null="*" */
	char	c_bk_cls_strt_dt[23];		/* null="*" */
	char	c_bk_cls_end_dt[23];		/* null="*" */
	long	l_setl_stlmnt_no;		/* null=-1 */
	long	l_multplr;		/* null=-1 */
	char	c_rqst_typ;		/* null='*' */
	long	l_hgh_prc_rng;		/* null=-1 */
	long	l_low_prc_rng;		/* null=-1 */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_indx_qt {
	char	c_index_cd[7];		/* null="*" */
	char	c_indx_tm_stmp[23];		/* null="*" */
	long	l_indx_val;		/* null=-1 */
	long	l_high_indx_val;		/* null=-1 */
	long	l_open_indx_val;		/* null=-1 */
	long	l_low_indx_val;		/* null=-1 */
	long	l_close_indx_val;		/* null=-1 */
	long	l_prcntg_chng_indx;		/* null=-1 */
	long	l_indx_yrly_high_val;		/* null=-1 */
	long	l_indx_yrly_low_val;		/* null=-1 */
	char	c_indx_indctr;		/* null='*' */
	double	d_mkt_cptltn;		/* null=-1.000000 */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_undrlyng {
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_ctgry_indstk;		/* null='*' */
	char	c_stlmnt_typ;		/* null='*' */
	char	c_dlvry_allwd;		/* null='*' */
	char	c_qt_trdng_flg;		/* null='*' */
	long	l_close_prc;		/* null=-1 */
	char	c_cls_prc_tm[23];		/* null="*" */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_nse_cntrct {
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	char	c_symbol[11];		/* null="*" */
	char	c_series[3];		/* null="*" */
	long	l_ca_lvl;		/* null=-1 */
	long	l_token_id;		/* null=-1 */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_mtch_cntrct {
	char	c_xchng_cd[4];		/* null="*" */
	char	c_instrmnt_typ[11];		/* null="*" */
	char	c_symbol[21];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_opt_typ[3];		/* null="\0" */
	double	d_strike_prc;		/* null=-1.000000 */
	long	l_ca_lvl;		/* null=-1 */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_err_msg {
	char	c_err_no[7];		/* null="*" */
	char	c_err_msg[256];		/* null="*" */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_usr_prfl {
	char	c_user_id[9];		/* null="*" */
	long	l_session_id;		/* null=-1 */
	char	c_pipe_id[3];		/* null="*" */
	char	c_cln_mtch_accnt[11];		/* null="*" */
	long	C_c_dp_id;	/* count of c_dp_id */
	char	c_dp_id[5][9];		/* null="*" */
	char	c_dp_clnt_id[5][9];		/* null="*" */
	long	C_c_bnk_accnt_nmbr;	/* count of c_bnk_accnt_nmbr */
	char	c_bnk_accnt_nmbr[5][21];		/* null="*" */
	long	l_clnt_ctgry;		/* null=-1 */
	long	l_usr_flg;		/* null=0 */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_order {
	char	c_ordr_rfrnc[19];		/* null="*" */
	char	c_ordr_flw;		/* null='*' */
	char	c_slm_flg;		/* null='*' */
	char	c_ord_typ;		/* null='*' */
	long	l_dsclsd_qty;		/* null=-1 */
	long	l_stp_lss_tgr;		/* null=-1 */
	long	l_ord_tot_qty;		/* null=-1 */
	long	l_ord_lmt_rt;		/* null=-1 */
	char	c_valid_dt[23];		/* null="*" */
	char	c_trd_dt[23];		/* null="*" */
	char	c_ordr_stts;		/* null='*' */
	long	l_exctd_qty;		/* null=-1 */
	long	l_can_qty;		/* null=-1 */
	long	l_exprd_qty;		/* null=-1 */
	char	c_settlor[13];		/* null="*" */
	char	c_sprd_ord_rfrnc[19];		/* null="*" */
	long	l_mdfctn_cntr;		/* null=-1 */
	char	c_xchng_ack[20];		/* null="*" */
	char	c_req_typ;		/* null='*' */
	char	c_ack_tm[23];		/* null="*" */
	char	c_prev_ack_tm[23];		/* null="*" */
	char	c_oprn_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_orderbook {
	char	c_user_id[9];		/* null="*" */
	long	l_session_id;		/* null=-1 */
	char	c_pipe_id[3];		/* null="*" */
	char	c_cln_mtch_accnt[11];		/* null="*" */
	long	C_c_dp_id;	/* count of c_dp_id */
	char	c_dp_id[5][9];		/* null="*" */
	char	c_dp_clnt_id[5][9];		/* null="*" */
	long	C_c_bnk_accnt_nmbr;	/* count of c_bnk_accnt_nmbr */
	char	c_bnk_accnt_nmbr[5][21];		/* null="*" */
	long	l_clnt_ctgry;		/* null=-1 */
	long	l_usr_flg;		/* null=0 */
	long	l_eba_cntrct_id;		/* null=-1 */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	long	l_ca_lvl;		/* null=-1 */
	char	c_ordr_rfrnc[19];		/* null="*" */
	char	c_ordr_flw;		/* null='*' */
	char	c_slm_flg;		/* null='*' */
	char	c_ord_typ;		/* null='*' */
	long	l_dsclsd_qty;		/* null=-1 */
	long	l_stp_lss_tgr;		/* null=-1 */
	long	l_ord_tot_qty;		/* null=-1 */
	long	l_ord_lmt_rt;		/* null=-1 */
	char	c_valid_dt[23];		/* null="*" */
	char	c_trd_dt[23];		/* null="*" */
	char	c_ordr_stts;		/* null='*' */
	long	l_exctd_qty;		/* null=-1 */
	long	l_exctd_qty_day;		/* null=-1 */
	long	l_can_qty;		/* null=-1 */
	long	l_exprd_qty;		/* null=-1 */
	char	c_settlor[13];		/* null="*" */
	char	c_sprd_ord_rfrnc[19];		/* null="*" */
	long	l_mdfctn_cntr;		/* null=-1 */
	char	c_xchng_ack[20];		/* null="*" */
	char	c_req_typ;		/* null='*' */
	char	c_spl_flg;		/* null='*' */
	char	c_ack_tm[23];		/* null="*" */
	char	c_prev_ack_tm[23];		/* null="*" */
	char	c_pro_cli_ind;		/* null='*' */
	char	c_oprn_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
	char	c_channel[4];		/* null="*" */
	char	c_bp_id[9];		/* null="*" */
	char	c_ctcl_id[16];		/* null="*" */
	long	l_cse_id;		/* null=-1 */
	char	c_mrkt_typ;		/* null='\0' */
};


struct vw_xchngbook {
	char	c_xchng_cd[4];		/* null="*" */
	char	c_ordr_rfrnc[19];		/* null="*" */
	char	c_pipe_id[3];		/* null="*" */
	char	c_mod_trd_dt[23];		/* null="*" */
	long	l_ord_seq;		/* null=-1 */
	char	c_slm_flg;		/* null='*' */
	long	l_dsclsd_qty;		/* null=-1 */
	long	l_ord_tot_qty;		/* null=-1 */
	long	l_ord_lmt_rt;		/* null=-1 */
	long	l_stp_lss_tgr;		/* null=-1 */
	long	l_mdfctn_cntr;		/* null=-1 */
	char	c_valid_dt[23];		/* null="*" */
	char	c_ord_typ;		/* null='*' */
	char	c_sprd_ord_ind;		/* null='*' */
	char	c_req_typ;		/* null='*' */
	long	l_quote;		/* null=-1 */
	char	c_qt_tm[23];		/* null="*" */
	char	c_rqst_tm[23];		/* null="*" */
	char	c_frwrd_tm[23];		/* null="*" */
	char	c_plcd_stts;		/* null='*' */
	char	c_rms_prcsd_flg;		/* null='*' */
	long	l_ors_msg_typ;		/* null=-1 */
	char	c_ack_tm[23];		/* null="*" */
	char	c_xchng_rmrks[257];		/* null="*" */
	char	c_ex_ordr_typ;		/* null='*' */
	long	l_xchng_can_qty;		/* null=-1 */
	long	l_bit_flg;		/* null=-1 */
	char	c_spl_flg;		/* null='*' */
	char	c_ordr_flw;		/* null='*' */
	char	c_oprn_typ;		/* null='*' */
	double	d_jiffy;		/* null=-1.000000 */
	long	l_dwnld_flg;		/* null=-1 */
	char	c_entry_dt_tm[23];		/* null="*" */
	char	c_rout_crt[4];		/* null="*" */
	char	c_mkrt_typ;		/* null='N' */
};


struct vw_exrc_req {
	char	c_exrc_ordr_rfrnc[19];		/* null="*" */
	char	c_exrc_rqst_typ;		/* null='*' */
	long	l_exrc_qty;		/* null=-1 */
	char	c_exrc_stts;		/* null='*' */
	char	c_trd_dt[23];		/* null="*" */
	long	l_mdfctn_cntr;		/* null=-1 */
	char	c_settlor[13];		/* null="*" */
	char	c_xchng_ack[20];		/* null="*" */
	char	c_oprn_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_exrcbook {
	char	c_user_id[9];		/* null="*" */
	long	l_session_id;		/* null=-1 */
	char	c_pipe_id[3];		/* null="*" */
	char	c_cln_mtch_accnt[11];		/* null="*" */
	long	C_c_dp_id;	/* count of c_dp_id */
	char	c_dp_id[5][9];		/* null="*" */
	char	c_dp_clnt_id[5][9];		/* null="*" */
	long	C_c_bnk_accnt_nmbr;	/* count of c_bnk_accnt_nmbr */
	char	c_bnk_accnt_nmbr[5][21];		/* null="*" */
	long	l_clnt_ctgry;		/* null=-1 */
	long	l_usr_flg;		/* null=0 */
	long	l_eba_cntrct_id;		/* null=-1 */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	long	l_ca_lvl;		/* null=-1 */
	char	c_exrc_ordr_rfrnc[19];		/* null="*" */
	char	c_exrc_rqst_typ;		/* null='*' */
	long	l_exrc_qty;		/* null=-1 */
	char	c_exrc_stts;		/* null='*' */
	char	c_trd_dt[23];		/* null="*" */
	long	l_mdfctn_cntr;		/* null=-1 */
	char	c_settlor[13];		/* null="*" */
	char	c_xchng_ack[20];		/* null="*" */
	long	l_opnpstn_qty;		/* null=-1 */
	char	c_oprn_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
	char	c_ack_tm[23];		/* null="*" */
	char	c_pro_cli_ind;		/* null='*' */
	long	l_cur_mkt_prc;		/* null=-1 */
	char	c_channel[4];		/* null="*" */
	char	c_bp_id[9];		/* null="*" */
	long	l_cse_id;		/* null=-1 */
	long	l_brkg_val;		/* null=-1 */
	long	l_stt;		/* null=-1 */
	long	l_src_tx;		/* null=-1 */
	long	l_sebi_tt;		/* null=-1 */
	long	l_trn_chrg;		/* null=-1 */
	long	l_stmp_dty;		/* null=-1 */
	long	l_oth_chrg;		/* null=-1 */
};


struct vw_tradebook {
	char	c_user_id[9];		/* null="*" */
	long	l_session_id;		/* null=-1 */
	char	c_pipe_id[3];		/* null="*" */
	char	c_cln_mtch_accnt[11];		/* null="*" */
	long	C_c_dp_id;	/* count of c_dp_id */
	char	c_dp_id[5][9];		/* null="*" */
	char	c_dp_clnt_id[5][9];		/* null="*" */
	long	C_c_bnk_accnt_nmbr;	/* count of c_bnk_accnt_nmbr */
	char	c_bnk_accnt_nmbr[5][21];		/* null="*" */
	long	l_clnt_ctgry;		/* null=-1 */
	long	l_usr_flg;		/* null=0 */
	long	l_eba_cntrct_id;		/* null=-1 */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	long	l_ca_lvl;		/* null=-1 */
	char	c_trd_rfrnc[19];		/* null="*" */
	char	c_ordr_rfrnc[19];		/* null="*" */
	char	c_xchng_ack[20];		/* null="*" */
	long	l_xchng_trd_no;		/* null=-1 */
	char	c_trd_dt[23];		/* null="*" */
	char	c_trnsctn_typ[4];		/* null="*" */
	char	c_trd_flw;		/* null='*' */
	long	l_exctd_qty;		/* null=-1 */
	long	l_exctd_rt;		/* null=-1 */
	long	l_frs_brkg_val;		/* null=-1 */
	long	l_cvr_brkg_val;		/* null=-1 */
	char	c_upld_mtch_flg;		/* null='*' */
	char	c_rms_prcsd_flg;		/* null='*' */
	long	l_fresh_qty;		/* null=-1 */
	long	l_cover_qty;		/* null=-1 */
	char	c_oprn_typ;		/* null='*' */
	double	d_jiffy;		/* null=-1.000000 */
	long	l_dwnld_flg;		/* null=-1 */
	char	c_rout_crt[4];		/* null="*" */
	long	l_brkg_val;		/* null=-1 */
	long	l_net_trd_val;		/* null=-1 */
	char	c_fut_conv_flg;		/* null='\0' */
	long	l_stt;		/* null=-1 */
	long	l_src_tx;		/* null=-1 */
	long	l_sebi_tt;		/* null=-1 */
	long	l_trn_chrg;		/* null=-1 */
	long	l_stmp_dty;		/* null=-1 */
};


struct vw_sequence {
	long	l_seq_num;		/* null=-1 */
	char	c_pipe_id[3];		/* null="*" */
	char	c_trd_dt[23];		/* null="*" */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_asgnmt {
	char	c_user_id[9];		/* null="*" */
	long	l_session_id;		/* null=-1 */
	char	c_pipe_id[3];		/* null="*" */
	char	c_cln_mtch_accnt[11];		/* null="*" */
	long	C_c_dp_id;	/* count of c_dp_id */
	char	c_dp_id[5][9];		/* null="*" */
	char	c_dp_clnt_id[5][9];		/* null="*" */
	long	C_c_bnk_accnt_nmbr;	/* count of c_bnk_accnt_nmbr */
	char	c_bnk_accnt_nmbr[5][21];		/* null="*" */
	long	l_clnt_ctgry;		/* null=-1 */
	long	l_usr_flg;		/* null=0 */
	long	l_eba_cntrct_id;		/* null=-1 */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	char	c_asgn_rfrnc[19];		/* null="*" */
	char	c_asgn_dt[23];		/* null="*" */
	long	l_asgnd_qty;		/* null=-1 */
	long	l_opnpstn_qty;		/* null=-1 */
	long	l_sttlmnt_prc;		/* null=-1 */
	long	l_cur_mkt_prc;		/* null=-1 */
	char	c_oprn_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_usr_dtls {
	char	c_user_id[9];		/* null="*" */
	long	l_session_id;		/* null=-1 */
	char	c_cln_mtch_accnt[10][11];		/* null="*" */
	char	c_dp_id[10][9];		/* null="*" */
	char	c_dp_clnt_id[10][9];		/* null="*" */
	char	c_bnk_accnt_nmbr[10][21];		/* null="*" */
	long	l_clnt_ctgry;		/* null=-1 */
	long	l_passwd_lmt;		/* null=-1 */
	char	c_clnt_typ;		/* null='*' */
	char	c_xchng_cd[5][4];		/* null="*" */
	char	c_prd_desc[5][11];		/* null="*" */
	char	c_prd_typ[5];		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_pstn_actn {
	char	c_user_id[9];		/* null="*" */
	long	l_session_id;		/* null=-1 */
	char	c_cln_mtch_accnt[11];		/* null="*" */
	long	l_eba_cntrct_id;		/* null=-1 */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	long	l_ca_lvl;		/* null=-1 */
	char	c_cntrct_tag;		/* null='*' */
	long	l_actn_typ;		/* null=-1 */
	char	c_trnsctn_flw;		/* null='*' */
	long	l_orgnl_qty;		/* null=-1 */
	long	l_orgnl_rt;		/* null=-1 */
	long	l_new_qty;		/* null=-1 */
	long	l_new_rt;		/* null=-1 */
	long	l_exec_qty;		/* null=-1 */
	long	l_exec_rt;		/* null=-1 */
	long	l_fresh_qty;		/* null=-1 */
	long	l_cover_qty;		/* null=-1 */
	double	d_add_mrgn_amt;		/* null=-1.000000 */
	double	d_und_quote;		/* null=-1.000000 */
	long	l_curr_rt;		/* null=-1 */
	char	c_ref_rmrks[133];		/* null="\0" */
	char	c_mtm_flag;		/* null='*' */
	char	c_nkd_blkd_flg;		/* null='*' */
	char	c_new_cntrct_tag;		/* null='*' */
	long	l_report_no;		/* null=-1 */
	char	c_imtm_rmrks[257];		/* null="*" */
	char	c_rout_crt[4];		/* null="*" */
	char	c_channel[4];		/* null="*" */
	char	c_bp_id[9];		/* null="*" */
	char	c_alias[9];		/* null="*" */
};


struct vw_xchngstts {
	char	c_xchng_cd[4];		/* null="*" */
	char	c_nxt_trd_dt[23];		/* null="*" */
	long	l_opn_tm;		/* null=-1 */
	long	l_cls_tm;		/* null=-1 */
	char	c_crrnt_stts;		/* null='*' */
	long	l_exrc_rq_opn_tm;		/* null=-1 */
	long	l_exrc_rq_cls_tm;		/* null=-1 */
	char	c_exrc_mkt_stts;		/* null='*' */
	char	c_mkt_typ;		/* null='*' */
	char	c_brkr_id[6];		/* null="*" */
	char	c_brkr_stts;		/* null='*' */
	char	c_settlor[6];		/* null="*" */
	char	c_settlor_stts;		/* null='*' */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_upd_limits {
	char	c_xchng_cd[4];		/* null="*" */
	char	c_cln_mtch_accnt[11];		/* null="*" */
	char	c_trd_dt[23];		/* null="\0" */
	char	c_narration_id[4];		/* null="*" */
	char	c_remarks[133];		/* null="\0" */
	char	c_dr_cr_flg;		/* null='-' */
	double	d_amount;		/* null=0.000000 */
	double	d_bal_amt;		/* null=0.000000 */
	char	c_dr_without_lmt_flg;		/* null='-' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_gt_lmt_dtls {
	char	c_xchng_cd[4];		/* null="*" */
	char	c_cln_mtch_accnt[11];		/* null="*" */
	char	c_trd_dt[23];		/* null="\0" */
	long	l_gid;		/* null=-1 */
	double	d_amt;		/* null=0.000000 */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_mkt_msg {
	char	c_xchng_cd[4];		/* null="*" */
	char	c_brkr_id[6];		/* null="*" */
	char	c_msg_id;		/* null='-' */
	char	c_msg[256];		/* null="\0" */
	char	c_tm_stmp[23];		/* null="*" */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_fut_pos {
	char	c_cln_mtch_accnt[11];		/* null="*" */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	long	l_ibuy_qty;		/* null=-1 */
	long	l_isell_qty;		/* null=-1 */
	long	l_ibuy_ord_rt;		/* null=-1 */
	long	l_isell_ord_rt;		/* null=-1 */
	double	d_ibuy_ord_vl;		/* null=-1.000000 */
	double	d_isell_ord_vl;		/* null=-1.000000 */
	long	l_exbuy_qty;		/* null=-1 */
	long	l_exsell_qty;		/* null=-1 */
	long	l_exbuy_ord_rt;		/* null=-1 */
	long	l_exsell_ord_rt;		/* null=-1 */
	double	d_exbuy_ord_vl;		/* null=-1.000000 */
	double	d_exsell_ord_vl;		/* null=-1.000000 */
	long	l_buy_exctd_qty;		/* null=-1 */
	long	l_sell_exctd_qty;		/* null=-1 */
	char	c_opnpstn_flw;		/* null='*' */
	long	l_opnpstn_qty;		/* null=-1 */
	long	l_opnpstn_rt;		/* null=-1 */
	double	d_opnpstn_val;		/* null=-1.000000 */
	long	l_base_ord_rt;		/* null=-1 */
	double	d_base_ord_val;		/* null=-1.000000 */
	double	d_ord_amt_blkd;		/* null=-1.000000 */
	double	d_pstn_amt_blkd;		/* null=-1.000000 */
	long	l_mtm_trg_prc;		/* null=-1 */
	char	c_oprn_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_cntfut_pos {
	char	c_cln_mtch_accnt[11];		/* null="*" */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	char	c_cntrct_tag;		/* null='*' */
	long	l_ibuy_qty;		/* null=-1 */
	long	l_isell_qty;		/* null=-1 */
	double	d_ibuy_ord_vl;		/* null=-1.000000 */
	double	d_isell_ord_vl;		/* null=-1.000000 */
	long	l_exbuy_qty;		/* null=-1 */
	long	l_exsell_qty;		/* null=-1 */
	double	d_exbuy_ord_vl;		/* null=-1.000000 */
	double	d_exsell_ord_vl;		/* null=-1.000000 */
	long	l_buy_exctd_qty;		/* null=-1 */
	long	l_sell_exctd_qty;		/* null=-1 */
	char	c_opnpstn_flw;		/* null='*' */
	long	l_opnpstn_qty;		/* null=-1 */
	double	d_org_opn_val;		/* null=-1.000000 */
	double	d_mtm_opn_val;		/* null=-1.000000 */
	double	d_imtm_opn_val;		/* null=-1.000000 */
	long	l_und_opn_qty;		/* null=-1 */
	double	d_und_opn_val;		/* null=-1.000000 */
	double	d_ibuy_mrgn;		/* null=-1.000000 */
	double	d_isell_mrgn;		/* null=-1.000000 */
	double	d_exbuy_mrgn;		/* null=-1.000000 */
	double	d_exsell_mrgn;		/* null=-1.000000 */
	double	d_ordr_mrgn;		/* null=-1.000000 */
	char	c_oprn_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_undfut_pos {
	char	c_cln_mtch_accnt[11];		/* null="*" */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_ctgry_indstk;		/* null='*' */
	char	c_cntrct_tag;		/* null='*' */
	long	l_ibuy_qty;		/* null=-1 */
	long	l_isell_qty;		/* null=-1 */
	double	d_ibuy_ord_vl;		/* null=-1.000000 */
	double	d_isell_ord_vl;		/* null=-1.000000 */
	long	l_exbuy_qty;		/* null=-1 */
	long	l_exsell_qty;		/* null=-1 */
	double	d_exbuy_ord_vl;		/* null=-1.000000 */
	double	d_exsell_ord_vl;		/* null=-1.000000 */
	long	l_buy_exctd_qty;		/* null=-1 */
	long	l_sell_exctd_qty;		/* null=-1 */
	char	c_opnpstn_flw;		/* null='*' */
	long	l_opnpstn_qty;		/* null=-1 */
	double	d_org_opn_val;		/* null=-1.000000 */
	double	d_mtm_opn_val;		/* null=-1.000000 */
	double	d_imtm_opn_val;		/* null=-1.000000 */
	double	d_add_mrgn_val;		/* null=-1.000000 */
	long	l_opn_buyqty;		/* null=-1 */
	long	l_opn_sellqty;		/* null=-1 */
	double	d_ibuy_mrgn;		/* null=-1.000000 */
	double	d_isell_mrgn;		/* null=-1.000000 */
	double	d_exbuy_mrgn;		/* null=-1.000000 */
	double	d_exsell_mrgn;		/* null=-1.000000 */
	double	d_ordr_mrgn;		/* null=-1.000000 */
	double	d_exctd_mrgn;		/* null=-1.000000 */
	double	d_sprd_mrgn;		/* null=-1.000000 */
	double	d_sprd_pl;		/* null=-1.000000 */
	double	d_trd_mrgn;		/* null=-1.000000 */
	double	d_imtm_exctd_mrgn;		/* null=-1.000000 */
	double	d_imtm_sprd_mrgn;		/* null=-1.000000 */
	double	d_imtm_sprd_pl;		/* null=-1.000000 */
	double	d_imtm_trd_mrgn;		/* null=-1.000000 */
	double	d_mm_exctd_mrgn;		/* null=-1.000000 */
	double	d_mm_sprd_mrgn;		/* null=-1.000000 */
	double	d_mm_sprd_pl;		/* null=-1.000000 */
	double	d_mm_trd_mrgn;		/* null=-1.000000 */
	char	c_oprn_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_cntopt_pos {
	char	c_cln_mtch_accnt[11];		/* null="*" */
	char	c_xchng_cd[4];		/* null="*" */
	char	c_prd_typ;		/* null='*' */
	char	c_undrlyng[7];		/* null="*" */
	char	c_expry_dt[23];		/* null="*" */
	char	c_exrc_typ;		/* null='*' */
	char	c_opt_typ;		/* null='\0' */
	long	l_strike_prc;		/* null=-1 */
	char	c_ctgry_indstk;		/* null='*' */
	long	l_ibuy_qty;		/* null=-1 */
	long	l_isell_qty;		/* null=-1 */
	long	l_iblkd_qty;		/* null=-1 */
	double	d_ibuy_ord_vl;		/* null=-1.000000 */
	double	d_isell_ord_vl;		/* null=-1.000000 */
	long	l_exbuy_qty;		/* null=-1 */
	long	l_exsell_qty;		/* null=-1 */
	long	l_exblkd_qty;		/* null=-1 */
	double	d_exbuy_ord_vl;		/* null=-1.000000 */
	double	d_exsell_ord_vl;		/* null=-1.000000 */
	long	l_buy_exctd_qty;		/* null=-1 */
	long	l_sell_exctd_qty;		/* null=-1 */
	long	l_opnpstn_qty;		/* null=-1 */
	long	l_blkd_opnpstn_qty;		/* null=-1 */
	double	d_org_opn_val;		/* null=-1.000000 */
	long	l_iexec_qty;		/* null=-1 */
	double	d_iord_vl;		/* null=-1.000000 */
	long	l_ex_exctd_qty;		/* null=-1 */
	double	d_exord_vl;		/* null=-1.000000 */
	double	d_ibuy_mrgn;		/* null=-1.000000 */
	double	d_isell_mrgn;		/* null=-1.000000 */
	double	d_exbuy_mrgn;		/* null=-1.000000 */
	double	d_exsell_mrgn;		/* null=-1.000000 */
	double	d_ordr_mrgn;		/* null=-1.000000 */
	long	l_blkd_ordr_qty;		/* null=-1 */
	double	d_trd_mrgn;		/* null=-1.000000 */
	long	l_blkd_trd_qty;		/* null=-1 */
	double	d_blkd_trd_mrgn;		/* null=-1.000000 */
	long	l_exrc_qty;		/* null=-1 */
	long	l_iexrc_qty;		/* null=-1 */
	long	l_exexrc_qty;		/* null=-1 */
	long	l_buy_exrc_qty;		/* null=-1 */
	long	l_sell_exrc_qty;		/* null=-1 */
	long	l_idont_exrc_qty;		/* null=-1 */
	long	l_exdont_exrc_qty;		/* null=-1 */
	long	l_asgnd_qty;		/* null=-1 */
	long	l_buy_asgn_qty;		/* null=-1 */
	long	l_sell_asgn_qty;		/* null=-1 */
	char	c_opnpstn_flw;		/* null='*' */
	long	l_ord_dp_qty;		/* null=-1 */
	char	c_dp_id[9];		/* null="*" */
	char	c_dp_clnt_id[9];		/* null="*" */
	char	c_isin_nmbr[13];		/* null="*" */
	char	c_bkubk_flg;		/* null='*' */
	long	l_mtm_trg_prc;		/* null=-1 */
	char	c_oprn_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_spdordbk {
	long	C_c_sprd_ord_rfrnc;	/* count of c_sprd_ord_rfrnc */
	char	c_sprd_ord_rfrnc[3][19];		/* null="*" */
	char	c_ordr_rfrnc[3][19];		/* null="*" */
	char	c_ack_tm[3][23];		/* null="*" */
	char	c_lst_md_tm[3][23];		/* null="*" */
	char	c_xchng_ack[3][20];		/* null="*" */
	char	c_sprd_ord_ind[3];		/* null='*' */
	char	c_pipe_id[3][3];		/* null="*" */
	long	l_mdfctn_cntr[3];		/* null=-1 */
	long	l_ors_msg_typ[3];		/* null=-1 */
	long	l_ord_tot_qty[3];		/* null=-1 */
	long	l_exctd_qty[3];		/* null=-1 */
	long	l_can_qty[3];		/* null=-1 */
	char	c_rqst_typ[3];		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};


struct vw_spdxchbk {
	char	c_xchng_cd[3][4];		/* null="*" */
	long	C_c_ordr_rfrnc;	/* count of c_ordr_rfrnc */
	char	c_ordr_rfrnc[3][19];		/* null="*" */
	char	c_pipe_id[3][3];		/* null="*" */
	char	c_mod_trd_dt[3][23];		/* null="*" */
	long	l_ord_seq[3];		/* null=-1 */
	char	c_slm_flg[3];		/* null='*' */
	long	l_dsclsd_qty[3];		/* null=-1 */
	long	l_ord_tot_qty[3];		/* null=-1 */
	long	l_ord_lmt_rt[3];		/* null=-1 */
	long	l_stp_lss_tgr[3];		/* null=-1 */
	long	l_mdfctn_cntr[3];		/* null=-1 */
	char	c_valid_dt[3][23];		/* null="*" */
	char	c_ord_typ[3];		/* null='*' */
	char	c_sprd_ord_ind[3];		/* null='*' */
	char	c_req_typ[3];		/* null='*' */
	long	l_quote[3];		/* null=-1 */
	char	c_qt_tm[3][23];		/* null="*" */
	char	c_rqst_tm[3][23];		/* null="*" */
	char	c_frwrd_tm[3][23];		/* null="*" */
	char	c_plcd_stts[3];		/* null='*' */
	char	c_rms_prcsd_flg[3];		/* null='*' */
	long	l_ors_msg_typ[3];		/* null=-1 */
	char	c_ack_tm[3][23];		/* null="*" */
	char	c_xchng_rmrks[3][257];		/* null="*" */
	char	c_ex_ordr_typ[3];		/* null='*' */
	long	l_xchng_can_qty[3];		/* null=-1 */
	long	l_bit_flg[3];		/* null=-1 */
	char	c_spl_flg[3];		/* null='*' */
	char	c_ordr_flw[3];		/* null='*' */
	char	c_oprn_typ[3];		/* null='*' */
	double	d_jiffy[3];		/* null=-1.000000 */
	long	l_dwnld_flg[3];		/* null=-1 */
	char	c_entry_dt_tm[3][23];		/* null="*" */
	char	c_rout_crt[4];		/* null="*" */
};

