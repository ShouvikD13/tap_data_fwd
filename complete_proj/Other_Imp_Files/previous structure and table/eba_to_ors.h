/******************************************************************************/
/*  Program         : eba_to_ors.h                                            */
/*                                                                            */
/*  Input           :                                                         */
/*                                                                            */
/*  Output          :                                                         */
/*                                                                            */
/*  Description     : The API header file for EBA to ORS communication        */
/*                                                                            */
/*  Log             :                                                         */
/*                                                                            */
/******************************************************************************/
/*  Swamy Boggarapu           24-Oct-2001         ver 1.0                     */
/*	ICICI Infotech(SR)				10-Feb-2004					ver 1.1 Pragma introduced		*/
/*	ICICI Infotech(MM)				17-Mar-2004					ver 1.2 Broker's name added */
/*																												in Login packet.		*/
/*	ICICI Infotech(MM)				16-Apr-2004					ver 1.3 CTCL ID added in 		*/
/*																												Order packet.				*/
/*	ICICI Infotech(MM)				17-Oct-2005					ver 1.4 CTCL ID For DBC 		*/
/*  ICICI Infotech(VK)        11-Oct-2007         ver 1.5 For increased size  */
/*                                                        Order and Trade no. */
/*                                                        by 1 char (NNF 808) */
/*  ICICI Infotech(SH)        02-Jan-2009         ver 1.6 Extended Market			*/
/*																												Provision					  */
/*																																						*/
/*	ICICI Infotech						27-Jun-2009					Ver 1.7 ORS2								*/
/*	ICICI Infotech						31-Jan-2014					Ver 1.8 Order Ack Changes		*/
/*	ICICI Infotech						14-Mar-2018					Ver 1.9 Rollover with spread*/
/*                                                        changes (Suchita D.)*/
/*	ICICI Infotech						27-Jun-2019					Ver 2.0 MCX Changes(Suchita)*/
/*	ICICI Infotech						27-FEB-2022         Ver 2.1 CR-ISEC14-161249 changes ( Suchita )***/
/******************************************************************************/

#pragma pack(4) /*Packing of 4 bytes added in Ver 1.1*/

/******  Message types  *********/
/*******************************************************************/
/* 1. ORS server service requests & responses                      */
/*******************************************************************/
#define ORS_SRVR_REQ 5001
#define ORS_SRVR_RES 5002
#define ORS_PRCS_STTS_REQ 5003
#define ORS_PRCS_STTS_RES 5004
#define ORS_MTRCS_DTLS_REQ 5005
#define ORS_MTRCS_DTLS_RES 5006
#define ORS_RESET_MTRCS_DTLS_REQ 5007
#define ORS_STP_SRVR_REQ 5008
#define ORS_REFACK_DTLS_REQ 5009
#define ORS_REFACK_DTLS_RES 5010
#define ORS_KILL_SRVR_REQ 5011
#define ORS_MKT_STTS_CHNG 5012
#define ORS_FRZ_ORDR_LST 5013
#define ORS_FRZ_ORDR_LST_END 5014

/*******************************************************************/
/* 2. Exchange connectivity requests & responses                   */
/*******************************************************************/
#define ORS_LOGON_REQ 5101
#define ORS_LOGON_RES 5102
#define ORS_LOGOFF_REQ 5103
#define LOGIN_WITH_OPEN_ORDR_DTLS 5104    /** 1.7 ORS2 by shamili **/
#define LOGIN_WITHOUT_OPEN_ORDR_DTLS 5105 /** 1.7 ORS2 by shamili **/

/*******************************************************************/
/* 3. Exchange special data requests & responses                   */
/*******************************************************************/
#define ORS_DWNLD_REQ 5201
#define ORS_RETRNS_END 5202

/***********************************************************************/
/* 4. Message types for exchange order management requests & responses */
/***********************************************************************/
#define ORS_NEW_ORD_REQ 5301
#define ORS_NEW_ORD_RES 5302
#define ORS_NEW_ORD_ACPT 5303
#define ORS_NEW_ORD_RJCT 5304

#define ORS_MOD_ORD_REQ 5305
#define ORS_MOD_ORD_ACPT 5306
#define ORS_MOD_ORD_RJCT 5307

#define ORS_CAN_ORD_REQ 5308
#define ORS_CAN_ORD_ACPT 5309
#define ORS_CAN_ORD_RJCT 5310

#define ORS_TRD_CONF 5312

#define ORS_ORD_FRZ 5313
#define ORS_ORD_TRG 5314
#define ORS_OPN_ORD_DTLS 5326
#define ORS_ORD_EXP 5315

#define ORS_NEW_EXER_REQ 5316
#define ORS_NEW_EXER_RES 5317
#define ORS_NEW_EXER_ACPT 5318
#define ORS_NEW_EXER_RJCT 5319

#define ORS_MOD_EXER_REQ 5320
#define ORS_MOD_EXER_ACPT 5321
#define ORS_MOD_EXER_RJCT 5322

#define ORS_CAN_EXER_REQ 5323
#define ORS_CAN_EXER_ACPT 5324
#define ORS_CAN_EXER_RJCT 5325

#define ORS_SPRD_ORD_REQ 5311
#define ORS_2L_ORD_REQ 5327
#define ORS_3L_ORD_REQ 5328

#define ORS_SPN_ORD_RES 5329
#define ORS_SPN_ORD_ACPT 5330
#define ORS_SPN_ORD_RJCT 5331
#define ORS_2LN_ORD_RES 5332
#define ORS_2LN_ORD_ACPT 5333
#define ORS_2LN_ORD_RJCT 5334
#define ORS_3LN_ORD_RES 5335
#define ORS_3LN_ORD_ACPT 5336
#define ORS_3LN_ORD_RJCT 5337
#define ORS_SPD_ORD_CNCL 5338
#define ORS_2L_ORD_CNCL 5339
#define ORS_3L_ORD_CNCL 5340

#define ORS_IOC_CAN_ACPT 5341

#define ORS_ORD_FRZ_CNCL 5342

#define EBA_FUT_CONVERSION 5343 /** Added For FuturePlus **/
#define ORS_SPRD_MOD_ACPT 5417  /** Ver 1.9 **/
#define ORS_SPRD_MOD_RJCT 5418  /** Ver 1.9 **/
#define ORS_SPRD_CAN_RJCT 5419  /** Ver 1.9 **/
#define ORS_SPRD_CAN_ACPT 5420  /** Ver 1.9 **/

/** Number used upto 5342 *****/

/*******************************************************************/
/* 5. Broadcast message                                            */
/*******************************************************************/
#define ORS_TRD_MSG 5401
#define ORS_GNR_MSG 5402
#define ORS_EXCH_CON_MSG 5003

#define ORS_CNTRCT_INFO 5404
#define ORS_CNTRCT_STTS 5405
#define ORS_EOD_QT 5406
#define ORS_LONG_QT 5407
#define ORS_SHORT_QT 5408
#define ORS_INDX_QT 5409

#define ORS_PARTI_STTS 5410
#define ORS_BRK_LMT_EXCD 5411
#define ORS_MKT_STTS 5412

#define ORS_SYS_INFO 5413

#define ORS_INDX_SHORT_QT 5414

#define ORS_OI_QT 5415
#define ORS_EQ_OI_QT 5416

/*******************************************************************/
/* Length of character strings                                     */
/*******************************************************************/
#define LEN_MSG 256

#define LEN_USR_ID 8 + 1
#define LEN_PASS_WD 8 + 1

#define LEN_DT 20 + 1
#define LEN_TM 20 + 1
#define LEN_LOG_TM 8 + 1

#define LEN_TRDR_ID 5 + 1
#define LEN_BRKR_ID 5 + 1
#define LEN_BRKR_NAME 25 + 1 /* 1.2 */
#define LEN_CTCL_ID 15 + 1   /* 1.3 */
#define LEN_CLNT_ID 10 + 1
#define LEN_PARTI_ID 12 + 1

#define LEN_XCHNG_CD 3 + 1

#define LEN_SYMBOL 10 + 1
#define LEN_SERIES 3 + 1
#define LEN_INDEX_NAM 21 + 1

#define LEN_ORDR_RFRNC 18 + 1
#define LEN_XCHNG_ACK 19 + 1 /** Added in ver 1.5 **/ /***	Ver 1.8	***/

/*******************************************************************/
/* Max array size                                                  */
/*******************************************************************/
#define MAX_BOFR_BBID 5
#define MAX_RECS 20

/*******************************************************************/
/* Request type for download data                                  */
/*******************************************************************/
#define CNTRCT_INFO 1
#define OPEN_ORDR_INFO 2
#define RETRANSMIT_DATA 3

/*******************************************************************/
/* Server identity numbers                                         */
/*******************************************************************/
#define CON_SRVR 1
#define SND_SRVR 2
#define RCV_SRVR 3
#define BRD_SRVR 4
#define ESR_SRVR 5
#define EBRD_SRVR 6
#define ORS_SRVR 7
#define EXCH_SMLTR 8

/*******************************************************************/
/* Market type for market status                                   */
/*******************************************************************/
#define ORDER_MARKET 'N'
#define EXER_MARKET 'E'
#define PL_MARKET 'P'
#define EXTND_MARKET 'X' /*Ver 1.6 */

/*******************************************************************/
/* Order placed by Broker / Customer Indicator                     */
/*******************************************************************/
#define BRKR_PLCD 'P'
#define CSTMR_PLCD 'C'

/*Ver 1.7 starts */
#define EXERCISE_SEGMENT 'E'
#define NORMAL_SEGMENT 'N'
#define EXTENDED_SEGMENT 'X'

#define SUSPEND 'S'
/*Ver 1.7 ends */

/*******************************************************************/
/* Common structures                                               */
/*******************************************************************/
struct st_nfo_cntrct
{
  char c_xchng_cd[LEN_XCHNG_CD];
  char c_prd_typ;
  char c_expry_dt[LEN_DT];
  char c_exrc_typ;
  char c_opt_typ;
  long l_strike_prc;
  char c_ctgry_indstk;
  char c_symbol[LEN_SYMBOL];
  char c_series[LEN_SERIES];
  long l_ca_lvl;
  long l_token_id;
};
typedef struct st_nfo_cntrct nfo_cntrct;

struct st_bfo_cntrct
{
  char c_xchng_cd[LEN_XCHNG_CD];
  char c_prd_typ;
  char c_expry_dt[LEN_DT];
  char c_exrc_typ;
  char c_opt_typ;
  long l_strike_prc;
  char c_ctgry_indstk;
  char c_symbol[LEN_SYMBOL];
  char c_series[LEN_SERIES];
  long l_ca_lvl;
  long l_token_id;
};
typedef struct st_bfo_cntrct bfo_cntrct;

union st_cntrct
{
  nfo_cntrct st_nfo_cnt;
  bfo_cntrct st_bfo_cnt;
};
typedef union st_cntrct cntrct;

struct st_ordr_dtls
{
  char c_cln_mtch_accnt[LEN_CLNT_ID];
  cntrct st_cntrct_info;
  char c_ordr_rfrnc[LEN_ORDR_RFRNC];
  char c_ordr_flw;
  char c_slm_flg;
  char c_ord_typ;
  long int l_dsclsd_qty;
  long int l_stp_lss_tgr;
  long int l_ord_tot_qty;
  long int l_ord_lmt_rt;
  char c_valid_dt[LEN_DT];
  char c_trd_dt[LEN_DT];
  char c_ordr_stts;
  long int l_exctd_qty;
  long int l_exctd_qty_day;
  long int l_can_qty;
  long int l_exprd_qty;
  char c_settlor[LEN_PARTI_ID];
  long int l_mdfctn_cntr;
  char c_ord_ack[LEN_XCHNG_ACK];
  char c_ack_tm[LEN_TM];
  char c_prev_ack_tm[LEN_TM];
  char c_ordr_exer_plcd_by;
  short int si_stlmnt_period;
  char c_ctcl_id[LEN_CTCL_ID]; /* 1.4 */
};
typedef struct st_ordr_dtls ordr_dtls;

struct st_exer_dtls
{
  char c_cln_mtch_accnt[LEN_CLNT_ID];
  cntrct st_cntrct_info;
  char c_exer_rfrnc[LEN_ORDR_RFRNC];
  long int l_exrc_qty;
  char c_exrc_stts;
  char c_trd_dt[LEN_DT];
  char c_settlor[LEN_PARTI_ID];
  long int l_mdfctn_cntr;
  char c_ord_ack[LEN_XCHNG_ACK];
  char c_ack_tm[LEN_TM];
  char c_ordr_exer_plcd_by;
};
typedef struct st_exer_dtls exer_dtls;

union st_book_dtls
{
  struct st_ordr_dtls st_ordr;
  struct st_exer_dtls st_exer;
};
typedef union st_book_dtls book_dtls;

struct st_nfo_exch_dtls
{
  char c_trdr_id[LEN_TRDR_ID];
  char c_brkr_id[LEN_BRKR_ID];
  long int li_brnch_id;
  char c_ctcl_id[LEN_CTCL_ID]; /* Ver 1.3 */
};
typedef struct st_nfo_exch_dtls nfo_exch_dtls;

struct st_bfo_exch_dtls
{
  char c_trdr_id[LEN_TRDR_ID];
  char c_brkr_id[LEN_BRKR_ID];
  long int li_brnch_id;
};
typedef struct st_bfo_exch_dtls bfo_exch_dtls;

union st_xchng_dtls
{
  nfo_exch_dtls st_nfo_xchng;
  bfo_exch_dtls st_bfo_xchng;
};
typedef union st_xchng_dtls xchng_dtls;

/*******************************************************************/
/* 1. ORS server service requests & responses                      */
/*******************************************************************/
struct st_prcs_req
{
  long int li_prcs_no;
};
typedef struct st_prcs_req msg_prcs_req;

struct st_prcs_res
{
  long int li_stts;
  char c_msg[LEN_MSG];
  long int li_flags;
};
typedef struct st_prcs_res msg_prcs_res;

struct st_prcs_stts
{
  int i_prcs_no[MAX_RECS];
  int i_prcs_stts[MAX_RECS];
};
typedef struct st_prcs_stts msg_prcs_stts;

/*******************************************************************/
/* 2. Exchange connectivity requests & responses                   */
/*******************************************************************/
struct st_exch_logon
{
  long int li_user_id;
  char c_pass_wd[LEN_PASS_WD];
  char c_new_pass_wd[LEN_PASS_WD];
  char c_lst_pass_wd_ch_dt[LEN_DT];
  char c_trdr_id[LEN_TRDR_ID];
  char c_brkr_id[LEN_BRKR_ID];
  long int li_brnch_id;
  char c_brkr_stts;
  char c_clr_mem_stts;
  long int li_stts;
  char c_msg[LEN_MSG];
  char c_lst_mkt_cls_dt[LEN_DT];
  char c_exch_tm[LEN_TM];
  char c_nm_mkt_stts;
  char c_ex_mkt_stts;
  char c_lst_upd_security_tm[LEN_TM];
  char c_lst_upd_participant_tm[LEN_TM];
  char c_lst_upd_instrument_tm[LEN_TM];
  char c_lst_upd_index_tm[LEN_TM];
  char c_rqst_for_open_ordrs;
  char c_brkr_name[LEN_BRKR_NAME]; /* ver 1.2 */
};
typedef struct st_exch_logon msg_exch_logon;

/*******************************************************************/
/* 3. Exchange special data requests & responses                   */
/*******************************************************************/
struct st_dwnld_req
{
  long int li_data_typ;
  long long ll_strt_tm;
};
typedef struct st_dwnld_req msg_dwnld_req;

struct st_opn_ord_dtls
{
  char c_ord_ack[LEN_XCHNG_ACK];
  long int li_tot_qty;
  long int li_un_exctd_qty;
};
typedef struct st_opn_ord_dtls msg_opn_ord_dtls;

/***********************************************************************/
/* 4. Message types for exchange order management requests & responses */
/***********************************************************************/
struct st_reqres_dtls
{
  char c_ordr_rfrnc[LEN_ORDR_RFRNC];
  char c_trd_dt[LEN_DT];
  long int l_ord_seq;
  char c_ex_ordr_typ;
  char c_req_typ;
  char c_slm_flg;
  long int l_dsclsd_qty;
  long int l_ord_tot_qty;
  long int l_ord_lmt_rt;
  long int l_stp_lss_tgr;
  long int l_mdfctn_cntr;
  char c_ord_typ;
  char c_valid_dt[LEN_DT];
  long int li_ors_msg_typ;
  long int l_xchng_can_qty;
  char c_ack_tm[LEN_TM];
  char c_xchng_rmrks[LEN_MSG];
  char c_xchng_ack[LEN_XCHNG_ACK];
  char c_entry_dt_tm[LEN_TM];
  char c_lst_ack_tm[LEN_TM];
  int i_retrans_flag;
  long long ll_log_tm_stmp;
};
typedef struct st_reqres_dtls msg_reqres_dtls;

struct st_ioc_can_dtls
{
  msg_reqres_dtls st_res_dtls;
  long int l_token_id;
};
typedef struct st_ioc_can_dtls msg_ioc_can_dtls;

struct st_ord_exer_req
{
  book_dtls st_bk_dtls;
  msg_reqres_dtls st_req_dtls;
  xchng_dtls st_exg_dtls;
};
typedef struct st_ord_exer_req msg_ord_exer_req;

struct st_sprd_ord_req
{
  book_dtls st_bk_dtls[3];
  msg_reqres_dtls st_req_dtls[3];
  xchng_dtls st_exg_dtls;
  char c_spd_ordr_rfrnc[LEN_ORDR_RFRNC];
};
typedef struct st_sprd_ord_req msg_sprd_ord_req;

struct st_trd_conf
{
  char c_ord_ack[LEN_XCHNG_ACK];
  long int li_exctd_qty;
  long int li_exctd_rt;
  double d_trd_ack;
  char c_exctd_tm[LEN_TM];
  int i_retrans_flag;
  long long ll_log_tm_stmp;
  long int l_token_id;
};
typedef struct st_trd_conf msg_trd_conf;

struct st_sprd_ord_res
{
  msg_reqres_dtls st_res_dtls[3];
  char c_spd_ordr_rfrnc[LEN_ORDR_RFRNC];
};
typedef struct st_sprd_ord_res msg_sprd_ord_res;
/*******************************************************************/
/* 5. Broadcast message                                            */
/*******************************************************************/
struct st_trd_msg
{
  char c_trdr_id[LEN_TRDR_ID];
  char c_msg[LEN_MSG];
  char c_xchng_cd[LEN_XCHNG_CD];
  char c_tm_stmp[LEN_TM];
};
typedef struct st_trd_msg msg_trd_msg;

struct st_cntrct_info
{
  cntrct st_cntrt_dtls;
  long int li_perm_trd;
  int i_cntrct_stts;
  char c_adc_flg;
  char c_corp_action;
  long int li_issue_rt;
  char c_issue_strt_dt[LEN_DT];
  char c_issue_int_pymt_dt[LEN_DT];
  char c_issue_mat_dt[LEN_DT];
  char c_list_dt[LEN_DT];
  char c_exp_dt[LEN_DT];
  char c_read_dt[LEN_DT];
  char c_rec_dt[LEN_DT];
  char c_nd_strt_dt[LEN_DT];
  char c_nd_end_dt[LEN_DT];
  char c_bc_strt_dt[LEN_DT];
  char c_bc_end_dt[LEN_DT];
  char c_ex_strt_dt[LEN_DT];
  char c_ex_end_dt[LEN_DT];
  long int li_min_lot_qty;
  long int li_brd_lot_qty;
  long int li_tck_sz;
  long int li_high_prc_rng;
  long int li_low_prc_rng;
  char c_tm[LEN_TM];
  char c_normal_ext_mkttyp; /**Ver 1.6 Added for Extended Market Hours provision in bat_cnt_part **/
};
typedef struct st_cntrct_info msg_cntrct_info;

struct st_cntrct_stts
{
  cntrct st_cntrt_dtls;
  int i_cntrct_stts;
};
typedef struct st_cntrct_stts msg_cntrct_stts;

struct st_eod_qt
{
  cntrct st_cntrt_dtls;
  long int li_open_prc;
  long int li_close_prc;
  long int li_high_prc;
  long int li_low_prc;
  long int li_prev_close_prc;
  long int li_tot_qty_trd;
  long int li_tot_val_trd;
  long int li_open_interest;
  long int li_chg_open_interest;
  long int li_open_interest_high;
  long int li_open_interest_low;
  char c_oi_indicator;
  char c_tm[LEN_TM];
};
typedef struct st_eod_qt msg_eod_qt;

struct st_long_qt
{
  cntrct st_cntrt_dtls;
  long int li_lst_trd_prc;
  long int li_lst_trd_qty;
  long int li_tot_trd_qty;
  char c_lst_trd_tm[LEN_TM];
  long int li_chg_prev_close_prc;
  char c_prc_indicator;
  long int li_avg_trd_prc;
  double d_tot_buy_qty;
  double d_tot_sell_qty;
  long int li_open_prc;
  long int li_close_prc;
  long int li_high_prc;
  long int li_low_prc;
  long int li_bofr_qty[MAX_BOFR_BBID];
  long int li_bofr_prc[MAX_BOFR_BBID];
  long int li_bbid_qty[MAX_BOFR_BBID];
  long int li_bbid_prc[MAX_BOFR_BBID];
};
typedef struct st_long_qt msg_long_qt;

struct st_short_qt
{
  cntrct st_cntrt_dtls;
  long int li_lst_trd_prc;
  char c_lst_trd_tm[LEN_TM];
  long int li_bofr_qty;
  long int li_bofr_prc;
  long int li_bbid_qty;
  long int li_bbid_prc;
  long int li_open_interest;
};
typedef struct st_short_qt msg_short_qt;

struct st_indx_qt
{
  char c_index_nam[LEN_INDEX_NAM];
  long int li_index_val;
  long int li_open_val;
  long int li_close_val;
  long int li_high_val;
  long int li_low_val;
  long int li_chng_prev_close;
  long int li_yrly_high;
  long int li_yrly_low;
  long int li_mkt_cap;
  char c_indx_indicator;
  char c_tm[LEN_TM];
  char c_filter; /*****changed by sangeet FOR NNF******/
};
typedef struct st_indx_qt msg_indx_qt;

struct st_parti_stts
{
  char c_parti_id[LEN_PARTI_ID];
  char c_parti_stts;
  char c_xchng_cd[LEN_XCHNG_CD];
};
typedef struct st_parti_stts msg_parti_stts;

struct st_brkr_stts
{
  char c_brkr_id[LEN_BRKR_ID];
  char c_brkr_stts;
  char c_xchng_cd[LEN_XCHNG_CD];
};
typedef struct st_brkr_stts msg_brkr_stts;

struct st_mkt_stts
{
  char c_mkt_typ;
  char c_mkt_stts;
  char c_xchng_cd[LEN_XCHNG_CD];
};
typedef struct st_mkt_stts msg_mkt_stts;

struct st_sys_info_data
{
  long int li_def_settle_pd_n;
  long int li_warning_percent;
  long int li_vol_frz_percent;
  long int li_brd_lot_qty;
  long int li_tick_size;
  long int li_max_gtc_days;
  long int li_dscld_qty_percent_alld;
};
typedef struct st_sys_info_data msg_sys_info_data;

struct st_eq_oi_qt
{
  long int li_tkn_no;
  char c_exch_cd[4];
  long int li_oi;
};
typedef struct st_eq_oi_qt msg_eq_oi_qt;

/******************************************************************************/
/*   Union of all messages to be forwarded to ORS system through send path    */
/******************************************************************************/
union un_req_msg
{
  msg_exch_logon st_lgn_req;
  msg_ord_exer_req st_oe_req;
  msg_sprd_ord_req st_sp_req;
  msg_dwnld_req st_dw_req;
  msg_reqres_dtls st_res_dtls;
};

/******************************************************************************/
/*   Union of all messages to be received by ORS system through receive path  */
/******************************************************************************/
union un_res_msg
{
  msg_exch_logon exch_logon;
  msg_reqres_dtls st_res_dtls;
  msg_trd_conf st_tconf;
  msg_opn_ord_dtls st_opn_ord;
  msg_sprd_ord_res st_spd_ord_res;
  msg_ioc_can_dtls st_ioccan_dtls;
};

/******************************************************************************/
/* Union for Broadcast message.	  																		    		*/
/******************************************************************************/
union un_brd_msg
{
  msg_trd_msg st_trdr_msg;
  msg_cntrct_info st_contract_info;
  msg_cntrct_stts st_contract_stts;
  msg_eod_qt st_end_of_day_qt;
  msg_long_qt st_lng_qt;
  msg_short_qt st_sht_qt;
  msg_indx_qt st_idx_qt;
  msg_parti_stts st_participant_stts;
  msg_brkr_stts st_broker_stts;
  msg_mkt_stts st_mrkt_stts;
  msg_sys_info_data st_sid_msg;
  msg_eq_oi_qt st_equity_oi;
};

#pragma pack() /*Ver 1.1*/

/*********************************************************************/
/* Ver 1.7: Structure for opm_ord_pipe_mstr table details 					 */
/*********************************************************************/

struct st_opm_pipe_mstr
{
  char c_opm_pipe_id[3];
  char c_opm_pipe_desc[257];
  char c_opm_xchng_cd[4];
  char c_opm_pipe_ctgry_id[3];
  char c_opm_mkt_allwd[2];
  long li_opm_user_id;
  char c_opm_lst_pswd_chg_dt[20];
  int i_opm_max_pnd_ord;
  int i_opm_pmp_stts;
  int i_opm_adm_stts;
  char c_opm_ctcl_product[21];
  char c_opm_exg_pwd[13];
  char c_opm_new_exg_pwd[13];
  char c_opm_remark[513];
  char c_opm_ws_no[11];
  char c_opm_dwnld_tm[20];
  char c_opm_usr_typ[2];
  long li_opm_brnch_id;
  char c_xchng_brkr_id[6];
  char c_opm_trdr_id[6];
  int si_user_typ_glb;
};

/*** ver 2.0 starts ***/
struct st_mco_cntrct_info
{
  char c_xchng_cd[LEN_XCHNG_CD];
  char c_prd_typ;
  char c_expry_dt[LEN_DT];
  char c_exrc_typ;
  char c_opt_typ;
  long l_strike_prc;
  char c_ctgry_indstk;
  char c_symbol[LEN_SYMBOL];
  char c_series[LEN_SERIES];
  long l_ca_lvl;
  long l_token_id;
  long l_quotation_qty;
  char c_quotation_unit[30];
  long l_uid_auc_buy;
  long l_uid_auc_sell;
  char c_t2t_allwd[5];
  long l_tradble_lot;
  double d_prc_tick;
  char c_near_mon_sym[10];
  char c_far_mon_sym[10];
  char c_prd_strt_dt[30];
  char c_prd_end_dt[30];
  char c_tender_strt_dt[30];
  char c_tender_end_dt[30];
  char c_dlvry_strt_dt[30];
  char c_dlvry_end_dt[30];
  char c_expry_prcs_dt[30];
  char c_mrgn_indctr;
  double d_reg_buy_mrgn;
  double d_reg_sell_mrgn;
  double d_spl_buy_mrgn;
  double d_spl_sell_mrgn;
  double d_tender_b_mrgn;
  double d_tender_s_mrgn;
  double d_dlvry_b_mrgn;
  double d_dlvry_s_mrgn;
  double d_lim_for_all_clnts;
  double d_lim_only_all_clnts;
  double d_lim_only_all_own;
  double d_lim_per_clnt_acc;
  double d_lim_per_own_acc;
  char c_sprd_ben_allwd;
  char c_rec_del;
  char c_rmrks[30];
  double d_prc_nr;
  double d_prc_dr;
  double d_gen_nr;
  double d_gen_dr;
  double d_lot_nr;
  double d_lot_dr;
  long l_dec_locator;
  long l_block_deal;
  char c_curr_code[4];
  double d_dlvry_wt;
  char c_dlvry_unit[6];
  char c_prdct_mon[8];
  long l_trd_grp_id;
  long l_mtching_num;
  long l_pre_opn_sessn;
  long l_sprd_typ;
  double d_extrm_lss_buy_mrgn;
  double d_extrm_lss_sell_mrgn;
  char c_optn_prc_method[4];
  double d_threshold_lim;
  long l_dlvry_mode;
  char c_trdng_unit[6]; /*** ver 2.1 **/
};
typedef struct st_mco_cntrct_info msg_mco_cntrct_info;
/*** ver 2.0 ends ***/
