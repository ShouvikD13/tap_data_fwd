struct vw_xchngbook
{
	char c_xchng_cd[4];	   /* null="*" */
	char c_ordr_rfrnc[19]; /* null="*" */

	char c_pipe_id[3]; /* null="*" */ //<-------------------------------(this one is used) (FXB)

	char c_mod_trd_dt[23]; /* null="*" */ //<-------------------------------(this one is used) (FXB)

	long l_ord_seq; /* null=-1 */  //<-------------------------------(this one is used) (FXB)

	char c_slm_flg; /* null='*' */ //<-------------------------------(this one is used) (FXB)

	long l_dsclsd_qty; /* null=-1 */ // <-------------------------------(this one is used) (FXB)

	long l_ord_tot_qty; /* null=-1 */ //<-------------------------------(this one is used) (FXB)

	long l_ord_lmt_rt; /* null=-1 */ //<-------------------------------(this one is used) (FXB)

	long l_stp_lss_tgr; /* null=-1 */ //<-------------------------------(this one is used) (FXB)

	char c_ord_typ; /* null='*' */ //<-------------------------------(this one is used) (FXB)

	char c_sprd_ord_ind; /* null='*' */ 

	char c_req_typ; /* null='*' */ //<-------------------------------(this one is used) (FXB)

	long l_mdfctn_cntr;		 /* null=-1 */
	char c_valid_dt[23];	 /* null="*" */ // <-------------------------------(this one is used) (FXB)
	long l_quote;			 /* null=-1 */
	char c_qt_tm[23];		 /* null="*" */
	char c_rqst_tm[23];		 /* null="*" */
	char c_frwrd_tm[23];	 /* null="*" */
	char c_plcd_stts;		 /* null='*' */
	char c_rms_prcsd_flg;	 /* null='*' */
	long l_ors_msg_typ;		 /* null=-1 */
	char c_ack_tm[23];		 /* null="*" */
	char c_xchng_rmrks[257]; /* null="*" */
	char c_ex_ordr_typ;		 /* null='*' */
	long l_xchng_can_qty;	 /* null=-1 */
	long l_bit_flg;			 /* null=-1 */
	char c_spl_flg;			 /* null='*' */
	char c_ordr_flw;		 /* null='*' */
	char c_oprn_typ;		 /* null='*' */
	double d_jiffy;			 /* null=-1.000000 */
	long l_dwnld_flg;		 /* null=-1 */
	char c_entry_dt_tm[23];	 /* null="*" */
	char c_rout_crt[4];		 /* null="*" */
	char c_mkrt_typ;		 /* null='N' */
};

//---------------------------------------------------

struct vw_orderbook
{
	char c_user_id[9]; /* null="*" */
	long l_session_id; /* null=-1 */
	char c_pipe_id[3]; /* null="*" */

	char c_cln_mtch_accnt[11]; /* null="*" */ //<-------------------------------(this one is used) (FOD)

	long C_c_dp_id;				  /* count of c_dp_id */
	char c_dp_id[5][9];			  /* null="*" */
	char c_dp_clnt_id[5][9];	  /* null="*" */
	long C_c_bnk_accnt_nmbr;	  /* count of c_bnk_accnt_nmbr */
	char c_bnk_accnt_nmbr[5][21]; /* null="*" */
	long l_clnt_ctgry;			  /* null=-1 */
	long l_usr_flg;				  /* null=0 */
	long l_eba_cntrct_id;		  /* null=-1 */
	char c_xchng_cd[4];			  /* null="*" */
	char c_prd_typ;				  /* null='*' */
	char c_undrlyng[7];			  /* null="*" */
	char c_expry_dt[23];		  /* null="*" */
	char c_exrc_typ;			  /* null='*' */
	char c_opt_typ;				  /* null='\0' */
	long l_strike_prc;			  /* null=-1 */
	char c_ctgry_indstk;		  /* null='*' */
	long l_ca_lvl;				  /* null=-1 */
	char c_ordr_rfrnc[19];		  /* null="*" */

	char c_ordr_flw; /* null='*' */ //<-------------------------------(this one is used) (FOD)

	char c_slm_flg;		/* null='*' */
	char c_ord_typ;		/* null='*' */
	long l_dsclsd_qty;	/* null=-1 */
	long l_stp_lss_tgr; /* null=-1 */

	long l_ord_tot_qty; /* null=-1 */ //<-------------------------------(this one is used) (FOD)

	long l_ord_lmt_rt;	 /* null=-1 */
	char c_valid_dt[23]; /* null="*" */
	char c_trd_dt[23];	 /* null="*" */
	char c_ordr_stts;	 /* null='*' */

	long l_exctd_qty; /* null=-1 */ //<-------------------------------(this one is used) (FOD)

	long l_exctd_qty_day; /* null=-1 */ //<-------------------------------(this one is used) (FOD)

	long l_can_qty;	  /* null=-1 */
	long l_exprd_qty; /* null=-1 */

	char c_settlor[13]; /* null="*" */ //<-------------------------------(this one is used) (FOD)

	char c_sprd_ord_rfrnc[19]; /* null="*" */
	long l_mdfctn_cntr;		   /* null=-1 */

	char c_xchng_ack[20]; /* null="*" */ //<-------------------------------(this one is used) (FOD) 

	char c_req_typ; /* null='*' */

	char c_spl_flg; /* null='*' */ //<-------------------------------(this one is used) (FOD)

	char c_ack_tm[23]; /* null="*" */ //<-------------------------------(this one is used) (FOD)

	char c_prev_ack_tm[23]; /* null="*" */ //<-------------------------------(this one is used) (FOD)

	char c_pro_cli_ind; /* null='*' */ //<-------------------------------(this one is used) (FOD)

	char c_oprn_typ;	/* null='*' */
	char c_rout_crt[4]; /* null="*" */
	char c_channel[4];	/* null="*" */
	char c_bp_id[9];	/* null="*" */

	char c_ctcl_id[16]; /* null="*" */ //<-------------------------------(this one is used) (FOD)

	long l_cse_id;	 /* null=-1 */
	char c_mrkt_typ; /* null='\0' */
};

//-----------------------------------------------

struct vw_nse_cntrct
{
	char c_xchng_cd[4]; /* null="*" */ 

	char c_prd_typ;		 /* null='*' */	//<-------------------------------(this one is used) (fod)

	char c_expry_dt[23]; /* null="*" */
	char c_exrc_typ;	 /* null='*' */
	char c_opt_typ;		 /* null='\0' */
	long l_strike_prc;	 /* null=-1 */

	char c_ctgry_indstk; /* null='*' */ //<-------------------------------(this one is used) (fod)

	char c_symbol[11]; /* null="*" */ //<-------------------------------(this one is used) (fod)

	char c_series[3]; /* null="*" */

	long l_ca_lvl; /* null=-1 */ //<-------------------------------(this one is used) (ftq_fo_trd_qt)

	long l_token_id; /* null=-1 */ //<-------------------------------(this one is used) (ftq_fo_trd_qt)

	char c_rqst_typ;	/* null='*' */
	char c_rout_crt[4]; /* null="*" */
};

//--------------------------------------------

struct vw_spdordbk
{
	long C_c_sprd_ord_rfrnc; /* count of c_sprd_ord_rfrnc */

	char c_sprd_ord_rfrnc[3][19]; /* null="*" */ //<-------------------------------(this one is used)

	char c_ordr_rfrnc[3][19]; /* null="*" */
	char c_ack_tm[3][23];	  /* null="*" */
	char c_lst_md_tm[3][23];  /* null="*" */
	char c_xchng_ack[3][20];  /* null="*" */
	char c_sprd_ord_ind[3];	  /* null='*' */
	char c_pipe_id[3][3];	  /* null="*" */
	long l_mdfctn_cntr[3];	  /* null=-1 */
	long l_ors_msg_typ[3];	  /* null=-1 */
	long l_ord_tot_qty[3];	  /* null=-1 */
	long l_exctd_qty[3];	  /* null=-1 */
	long l_can_qty[3];		  /* null=-1 */
	char c_rqst_typ[3];		  /* null='*' */
	char c_rout_crt[4];		  /* null="*" */
};
//--------------------------------------------

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

	long li_opm_brnch_id;	 // --------------------- (this one is used) (OPM_ORD_PIPE_MSTR)
	char c_xchng_brkr_id[6]; //--------------------- (this one is used)	 (exg_xchng_mstr)
	char c_opm_trdr_id[6];	 //--------------------- (this one is used) (OPM_ORD_PIPE_MSTR)
	int si_user_typ_glb;	 //----------------------(this one is used) "this one we are getting from "cofiguration file""
};

//--------------------------------------------

struct st_req_q_data
{
	long int li_msg_type;				 //<-------------------------------(this one is used)
	struct st_exch_msg st_exch_msg_data; // <-------------------------------(this one is used)
};
	/* All Fields are used*/
//-------------------------------------------
struct st_exch_msg // (used in above struture)
{
	struct st_net_hdr st_net_header; // <-------------------------------(this one is used)
	union st_exch_snd_msg st_exch_sndmsg; // <-------------------------------(this one is used)
};

//-------------------------------------------
struct st_net_hdr // (used in above structure st_exch_msg) (Not used in the function)
{
	short si_message_length;
	int i_seq_num;
	unsigned char c_checksum[16];
};
	//(Not used in the function)
//--------------------------------------------
union st_exch_snd_msg //(This union is used in st_exch_msg )
{
	struct st_sign_on_req st_signon_req;
	struct st_system_info_req st_sysinf_req;
	struct st_update_local_database st_updlcdb_req;
	struct st_message_download st_msg_dwld;
	struct st_oe_reqres st_oe_res;	//<-------------------------------(this one is used)
	struct st_oe_reqres_tr st_oe_req_tr; /*** Added in Ver 2.9 ***/
	struct st_om_rqst_tr st_om_req_tr;	 /*** Added in Ver 2.9 ***/

	struct st_spd_oe_reqres st_spdoe_reqres; 

	struct st_ex_pl_reqres st_expl_reqres;
};

//--------------------------------------------

struct st_spd_oe_reqres
{
	struct st_int_header st_hdr; /* in header (st_hdr)
											1. c_filler_1
											2. li_trader_id
											3. c_alpha_char
											4. si_transaction_code
											5. c_filler_2
											6. c_time_stamp_1
											7. c_time_stamp_2
											8. si_message_length
											9. li_log_time */

	char c_participant_type;

	char c_filler1;

	short int si_competitor_period;

	short int si_solicitor_period;
 
	char c_mod_cxl_by;

	char c_filler2;

	short int si_reason_code;

	char c_start_alpha[2];

	char c_end_alpha[2];

	long l_token; /**V1.4**/

	struct st_contract_desc st_cntrct_desc; /* (Structure)
											1. c_instrument_name
											2. c_symbol
											3. li_expiry_date
											4. li_strike_price
											5. c_option_type
											6. si_ca_level */

	char c_op_broker_id[LEN_BROKER_ID];

	char c_filler3;

	char c_filler_options[LEN_FILLER_OPTIONS];

	char c_filler4;

	short int si_order_type;

	double d_order_number;

	char c_account_number[LEN_ACCOUNT_NUMBER];

	short int si_book_type;

	short int si_buy_sell;

	long int li_disclosed_vol;

	long int li_disclosed_vol_remaining;

	long int li_total_vol_remaining;

	long int li_volume;

	long int li_volume_filled_today;

	long int li_price;

	long int li_trigger_price;

	long int li_good_till_date;

	long int li_entry_date_time;

	long int li_min_fill_aon;

	long int li_last_modified;

	struct st_order_flags st_order_flgs; /* (Structure)
										 1. flg_ato
										 2. flg_market
										 3. flg_sl
										 4. flg_mit
										 5. flg_day
										 6. flg_gtc
										 7. flg_ioc
										 8. flg_aon
										 9. flg_mf
										 10. flg_matched_ind
										 11. flg_traded
										 12. flg_modified
										 13. flg_frozen
										 14. flg_filler1	*/

	short int si_branch_id;

	/*short int si_trader_id;			Commented In Ver 1.8	***/

	long int li_trader_id; /***	Ver 1.8 Data Type Changed From Short Int To Long	***/

	char c_broker_id[LEN_BROKER_ID];

	char c_oe_remarks[LEN_REMARKS];

	char c_open_close;

	char c_settlor[LEN_SETTLOR];

	short int si_pro_client;

	short int si_settlement_period;

	char c_cover_uncover;

	char c_give_up_flag;

	/* Ver 1.3 */
	/*** Ver 2.0 Coment Starts ****
	unsigned int us_filler1     :1;
	unsigned int us_filler2     :1;
	unsigned int us_filler3     :1;
	unsigned int us_filler4     :1;
	unsigned int us_filler5     :1;
	unsigned int us_filler6     :1;
	unsigned int us_filler7     :1;
	unsigned int us_filler8     :1;
	unsigned int us_filler9     :1;
	unsigned int us_filler10    :1;
	unsigned int us_filler11    :1;
	unsigned int us_filler12    :1;
	unsigned int us_filler13    :1;
	unsigned int us_filler14    :1;
	unsigned int us_filler15    :1;
	unsigned int us_filler16    :1;
	char c_filler17;
	char c_filler18;

	*** Ver 2.0 Comment Ends **/

	/*** Ver 2.0 ***/
	int i_sprd_seq_no;

	double d_nnf_field;

	double d_filler19;

	/* till here ...ver 1.3 */
	char c_pan[10]; /*** Ver 2.7 ***/

	long l_algo_id; /*** Ver 2.7 ***/

	short si_algo_category; /*** Ver 2.7 ***/

	/***	char c_reserved [ 60 ];   *** Ver 2.7 ***	*** Commented in Ver 2.9 ***/

	long long ll_lastactivityref; /*** Ver 2.9 ***/

	char c_reserved[52]; /*** Ver 2.9 ***/ // NOt Used

	long int li_spd_price_diff;

	struct st_spd_leg_info st_spd_lg_inf[2];
	//(Structure) //** All Fields are used
};
//**  ALL Fields are Used
//----------------------------------------------------------------------------------------------------
struct st_int_header
{
	short int si_transaction_code;
	long int li_log_time;

	char c_alpha_char[LEN_ALPHA_CHAR];

	long int li_trader_id;
	short int si_error_code;
	char c_filler_2[8];
	char c_time_stamp_1[LEN_TIME_STAMP];
	char c_time_stamp_2[LEN_TIME_STAMP];
	short int si_message_length;
};
//** ALL Fields are Used
//----------------------------------------------------------------------------------------------------
struct st_spd_leg_info
{
	long l_token; /**V1.4**/
	struct st_contract_desc st_cntrct_desc;
	char c_op_broker_id[LEN_BROKER_ID];
	char c_filler1;
	short int si_order_type;

	short int si_buy_sell;
	long int li_disclosed_vol;
	long int li_disclosed_vol_remaining;
	long int li_total_vol_remaining;
	long int li_volume;
	long int li_volume_filled_today;
	long int li_price;
	long int li_trigger_price;
	long int li_min_fill_aon;
	struct st_order_flags st_order_flgs; // all field used
	char c_open_close;
	char c_cover_uncover;
	char c_giveup_flag;
	char c_filler2;
};
//** All Fields are used
//--------------------------------------------------------------------------------------------------

struct st_contract_desc
{
	char c_instrument_name[LEN_INSTRUMENT_NAME];  //<-------------------------------(this one is used) (FOD)
	char c_symbol[LEN_SYMBOL_NSE];				  //<-------------------------------(this one is used) (FOD)
	long int li_expiry_date;					  //<-------------------------------(this one is used) (not sure)
	long int li_strike_price;					  //<-------------------------------(this one is used) (not sure)
	char c_option_type[LEN_OPTION_TYPE];		  //<-------------------------------(this one is used) (not sure)
	short int si_ca_level;						  ////<-------------------------------(this one is used) (ftq_fo_trd_qt)
};

//** ALL Fields are Used
//--------------------------------------------------------------------------------------------------

struct st_order_flags
{
	unsigned int flg_ato : 1;
	unsigned int flg_market : 1;
	unsigned int flg_sl : 1;
	unsigned int flg_mit : 1;
	unsigned int flg_day : 1;
	unsigned int flg_gtc : 1;
	unsigned int flg_ioc : 1;
	unsigned int flg_aon : 1;
	unsigned int flg_mf : 1;
	unsigned int flg_matched_ind : 1;
	unsigned int flg_traded : 1;
	unsigned int flg_modified : 1;
	unsigned int flg_frozen : 1;
	unsigned int flg_filler1 : 3;
};
//** ALL Fields are Used

//---------------------------------------------------------------------------------------------------

struct st_pk_sequence
{
	char c_pipe_id[3];
	char c_trd_dt[23];
	char c_rqst_typ[3];
	int i_seq_num;
};
// not given
//--------------------------------------------------------------------------------------------------

struct st_message_download
{
	struct st_int_header st_hdr;
	double d_sequence_number;
};


struct st_exch_msg
{
	struct st_net_hdr st_net_header;
	union st_exch_snd_msg st_exch_sndmsg;
};

struct vw_sequence {   // this is usd in "fo_exg_msgs" in function "fn_get_reset_seq" which is called from "fn_prcs_reco" in "cln_pack_plnt".
	long	l_seq_num;		/* null=-1 */
	char	c_pipe_id[3];		/* null="*" */
	char	c_trd_dt[23];		/* null="*" */
	char	c_rqst_typ;		/* null='*' */
	char	c_rout_crt[4];		/* null="*" */
};

struct st_oe_reqres
{
    struct st_int_header st_hdr;
    char c_participant_type;
    char c_filler_1;
    short int si_competitor_period;
    short int si_solicitor_period;
    char c_modified_cancelled_by;
    char c_filler_2;
    short int si_reason_code;
    char c_filler_3[4];
    long l_token_no;
    struct st_contract_desc st_con_desc;
    char c_counter_party_broker_id[LEN_BROKER_ID];
    char c_filler_4;
    char c_filler_5[2];
    char c_closeout_flg;
    char c_filler_6;
    short int si_order_type;
    double d_order_number;
    char c_account_number[LEN_ACCOUNT_NUMBER];
    short int si_book_type;
    short int si_buy_sell_indicator;
    long int li_disclosed_volume;
    long int li_disclosed_volume_remaining;
    long int li_total_volume_remaining;
    long int li_volume;
    long int li_volume_filled_today;
    long int li_price;
    long int li_trigger_price;
    long int li_good_till_date;
    long int li_entry_date_time;
    long int li_minimum_fill_aon_volume;
    long int li_last_modified;
    struct st_order_flags st_ord_flg;
    short int si_branch_id;
    long int li_trader_id;
    char c_broker_id[LEN_BROKER_ID];
    char c_remarks[LEN_REMARKS];
    char c_open_close;
    char c_settlor[LEN_SETTLOR];
    short int si_pro_client_indicator;
    short int si_settlement_period;
    char c_cover_uncover;
    char c_giveup_flag;
    int i_ordr_sqnc;
    double d_nnf_field;
    double d_filler19;
    char c_pan[10];
    long l_algo_id;
    short si_algo_category;
    long long ll_lastactivityref;
    char c_reserved[52];
};

//---------------------------------------------------

/* these are all used structures so . 
	vw_xchngbook
	vw_orderbook
	st_opm_pipe_mstr
	vw_nse_cntrct

	struct st_oe_reqres // (we are using this) (all the fiels of above four structures are used here so indirectly ther are part of this structure. )
		struct st_int_header st_hdr;
		struct st_contract_desc st_con_desc;
		struct st_order_flags st_ord_flg;

	struct st_exch_msg // (used in above struture)
	{
		struct st_net_hdr st_net_header; // <-------------------------------(this one is used)
		struct st_oe_reqres  st_oe_res; // <-------------------------------(this one is used)
	};

	struct st_net_hdr // (used in above structure st_exch_msg) (Not used in the function)
	{
		short si_message_length;
		int i_seq_num;
		unsigned char c_checksum[16];
	};

	struct st_req_q_data
	{
		long int li_msg_type;				 //<-------------------------------(this one is used)
		struct st_exch_msg st_exch_msg_data; // <-------------------------------(this one is used)
	};
*/

/*
struct st_req_q_data
├── long int li_msg_type
└── struct st_exch_msg st_exch_msg_data
    ├── struct st_net_hdr st_net_header
    │   ├── short si_message_length
    │   ├── int i_seq_num
    │   └── unsigned char c_checksum[16]
    └── struct st_oe_reqres st_oe_res
        ├── struct st_int_header st_hdr
        ├── char c_participant_type
        ├── char c_filler_1
        ├── short int si_competitor_period
        ├── short int si_solicitor_period
        ├── char c_modified_cancelled_by
        ├── char c_filler_2
        ├── short int si_reason_code
        ├── char c_filler_3[4]
        ├── long l_token_no
        ├── struct st_contract_desc st_con_desc
        │   ├── // Fields of st_contract_desc
        ├── char c_counter_party_broker_id[LEN_BROKER_ID]
        ├── char c_filler_4
        ├── char c_filler_5[2]
        ├── char c_closeout_flg
        ├── char c_filler_6
        ├── short int si_order_type
        ├── double d_order_number
        ├── char c_account_number[LEN_ACCOUNT_NUMBER]
        ├── short int si_book_type
        ├── short int si_buy_sell_indicator
        ├── long int li_disclosed_volume
        ├── long int li_disclosed_volume_remaining
        ├── long int li_total_volume_remaining
        ├── long int li_volume
        ├── long int li_volume_filled_today
        ├── long int li_price
        ├── long int li_trigger_price
        ├── long int li_good_till_date
        ├── long int li_entry_date_time
        ├── long int li_minimum_fill_aon_volume
        ├── long int li_last_modified
        ├── struct st_order_flags st_ord_flg
        │   ├── // Fields of st_order_flags
        ├── short int si_branch_id
        ├── long int li_trader_id
        ├── char c_broker_id[LEN_BROKER_ID]
        ├── char c_remarks[LEN_REMARKS]
        ├── char c_open_close
        ├── char c_settlor[LEN_SETTLOR]
        ├── short int si_pro_client_indicator
        ├── short int si_settlement_period
        ├── char c_cover_uncover
        ├── char c_giveup_flag
        ├── int i_ordr_sqnc
        ├── double d_nnf_field
        ├── double d_filler19
        ├── char c_pan[10]
        ├── long l_algo_id
        ├── short si_algo_category
        ├── long long ll_lastactivityref
        └── char c_reserved[52]

*/


struct vw_contract // this is updating in "fn_get_nxt_rec" in "cln_pack_clnt" and we are setting the field of this struct from "vw_orderbook"
{
	long l_eba_cntrct_id; 
	char c_xchng_cd[4];	// <------------- "this one is used "
	char c_prd_typ;		// <------------- "this one is used "
	char c_undrlyng[7];	// <------------- "this one is used "
	char c_expry_dt[23];// <------------- "this one is used "  
	char c_exrc_typ;	   
	char c_opt_typ;		  
	long l_strike_prc;	  
	char c_ctgry_indstk;  
	long l_ca_lvl;		  
	char c_rqst_typ;	  
	char c_rout_crt[4];	 
};


