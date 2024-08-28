package structures

import "DATA_FWD_TAP/models"

type Vw_xchngbook struct {
	C_ordr_rfrnc  string // this valiable we are getting from the table
	C_pipe_id     string // this variable we are getting from the args[3]
	C_mod_trd_dt  string // checkout this one, we are getting from table "exg_xchng_mstr"
	L_ord_seq     int32  // this variable we are gettign from the table
	C_slm_flg     string // byte   // this variable we are gettign from the table
	L_dsclsd_qty  int32  // this variable we are gettign from the table
	L_ord_tot_qty int32  // this variable we are gettign from the table
	L_ord_lmt_rt  int32  // this variable we are gettign from the table
	L_stp_lss_tgr int32  // this variable we are gettign from the table
	C_ord_typ     string // byte   // this variable we are gettign from the table
	C_req_typ     string // byte   // this variable we are gettign from the table
	C_valid_dt    string // this variable we are gettign from the table
	C_xchng_cd    string // this we are getting from "opm_ord_pipe_mstr"

	//-----------------------------------------------------------------------------
	C_ex_ordr_type  string  // this one we have to use to use for set we need this for ordinary order type.
	C_oprn_typ      string  // this is we are setting       // this column is not in the 'fxb' table.
	C_plcd_stts     string  // this field is used in "fnRefToOrd" to change the status in "UPDATION_ON_ORDER_FORWARDING"
	L_mdfctn_cntr   int32   // used in same querry as above
	C_frwrd_tm      string  // used in same querry as above
	D_jiffy         float64 //used in "fnRefToOrd" to change the status in "UPDATION_ON_EXCHANGE_RESPONSE"
	L_dwnld_flg     int32   // used in same querry as above // this column is not in the 'fxb' table.
	C_xchng_rmrks   string  // used in last query of "fnRefToOrd"
	C_rms_prcsd_flg string  // used in same query as above
	L_ors_msg_typ   int32   // used in same query as above
	C_ack_tm        string  // used in same querry as above

	//c_ip_addrs, FXB_IP, 'NA'
	//c_prcimpv_flg, FXB_PRCIMPV_FLG 'N'
	// these values are also to be added in the table

}

/*
	value of this structure "Vw_xchngbook" we are extracting from "FXB_FO_XCHNG_BOOK". and all this is happening in "fn_seq_to_omd" in "cln_pack_clnt".
*/

type Vw_orderbook struct {
	C_ordr_rfrnc     string
	C_cln_mtch_accnt string
	C_ordr_flw       string //byte
	L_ord_tot_qty    int32
	L_exctd_qty      int32
	L_exctd_qty_day  int32
	C_settlor        string
	C_spl_flg        string //byte
	C_ack_tm         string
	C_prev_ack_tm    string
	C_pro_cli_ind    string //byte
	C_ctcl_id        string
	C_xchng_ack      string //getting from querry of fetching the order details
	//-----------------------------------------------------

	L_mdfctn_cntr int32  // this is we are changing after we fetch the data from both the tables. (fod and fxb)
	C_ordr_stts   string // it is used in updating the status after the fetcing the orders from the table
	// C_oprn_typ    string // it is used in updating the status after the fetcing the orders from the table

	C_xchng_cd     string
	C_prd_typ      string
	C_undrlyng     string
	C_expry_dt     string
	C_exrc_typ     string
	C_opt_typ      string
	L_strike_prc   int64
	C_ctgry_indstk string
	// L_ca_lvl       int64
}

/*
	values of this structure  "Vw_orderbook". we are extracting from "fod_fo_ordr_dtls" . all this is happening in "fn_ref_to_ord" in "cln_pack_clnt"
*/

type Vw_contract struct {
	L_eba_cntrct_id int64  // null = -1
	C_xchng_cd      string // null = "*"
	C_prd_typ       string // null = '*'
	C_undrlyng      string // null = "*"
	C_expry_dt      string // null = "*"
	C_exrc_typ      string // null = '*'
	C_opt_typ       string // null = '\0'
	L_strike_prc    int64  // null = -1
	C_ctgry_indstk  string // null = '*'
	// L_ca_lvl        int64  // null = -1
	C_rqst_typ string // null = '*'
	C_rout_crt string // null = "*"
}

type Vw_nse_cntrct struct {
	C_prd_typ      string //byte
	C_ctgry_indstk string //byte
	C_symbol       string // this field we are initialising from first query in "fn_get_ext_cnt"
	L_ca_lvl       int32  //this field we are initialising from  query in "fn_get_ext_cnt"
	L_token_id     int32  //this field we are initialising from  query in "fn_get_ext_cnt"

	//----------------------------
	// these all fields are initialising from "fn_get_ext_cnt"
	C_xchng_cd   string
	C_expry_dt   string
	C_exrc_typ   string
	C_opt_typ    string
	L_strike_prc int64
	C_rqst_typ   string
	C_series     string
}

/*
	this value of this structure we are getting from "Vw_contract". which is in "fn_get_ext_cnt" in "cln_pack_clnt"

	 **** problem here is i am not able find the source like from where we are getting the value of the structure "Vw_contract".
*/

type St_opm_pipe_mstr struct {
	/*
		long li_opm_brnch_id;	 // --------------------- (this one is used) from (OPM_ORD_PIPE_MSTR)
		char c_xchng_brkr_id[6]; //--------------------- (this one is used)	 from (exg_xchng_mstr)
		char c_opm_trdr_id[6];	 //--------------------- (this one is used) from (OPM_ORD_PIPE_MSTR)
		int si_user_typ_glb;	 //----------------------(this one is used) "this one we are getting from "cofiguration file" "GetProcessSpaceValue()""

	*/
	L_opm_brnch_id  int64  // null=-1
	C_xchng_brkr_id string // null="*"
	C_opm_trdr_id   string // null="*"
	S_user_typ_glb  int    // null=0   (i think , it can be 0 (for trader) , 4 (for CORPORATE_MANAGER) , 5 (for BRANCH_MANAGER) )

	/* Table name "OPM_ORD_PIPE_MSTR"
	opm_xchng_cd,
	opm_max_pnd_ord,
	opm_stream_no
	OPM_TRDR_ID,
	OPM_BRNCH_ID
	OPM_XCHNG_CD
	OPM_PIPE_ID
	*/

	/* Table Name "exg_xchng_mstr"
	exg_nxt_trd_dt,
	exg_brkr_id,
	exg_ctcl_id
	exg_xchng_cd

	*/

}

/*
	fields of this structure we are getting from different sources.
*/

// ================================ till now all the strctures we are getting as parameters ========================

type St_req_q_data struct {
	L_msg_type int64
	/* we are changeing this field multiple times.
	1. it is changing, where we are packing the structure which is in "fn_pack_ordnry_ord_to_nse" here we are setting the the variable based on the request type like (if request is "new" then "li_msg_type = BOARD_LOT_IN")

	*/
	St_exch_msg_data St_exch_msg // in the original structure here a union is used . but i am directly using the structure.
}

type St_exch_msg struct {
	St_net_header St_net_hdr
	St_oe_res     St_oe_reqres
}

type St_net_hdr struct {
	S_message_length int16
	I_seq_num        int32
	C_checksum       string //[16]byte
}

type St_oe_reqres struct {
	St_hdr                        *St_int_header
	C_participant_type            string // byte
	C_filler_1                    string //byte
	Si_competitor_period          int16
	Si_solicitor_period           int16
	C_modified_cancelled_by       string //byte
	C_filler_2                    string // byte
	Si_reason_code                int16
	C_filler_3                    string
	L_token_no                    int32
	St_con_desc                   *St_contract_desc
	C_counter_party_broker_id     [models.LEN_BROKER_ID]byte
	C_filler_4                    string //byte
	C_filler_5                    string
	C_closeout_flg                string //byte
	C_filler_6                    string //byte
	Si_order_type                 int16
	D_order_number                float64
	C_account_number              [models.LEN_ACCOUNT_NUMBER]byte
	Si_book_type                  int16
	Si_buy_sell_indicator         int16
	Li_disclosed_volume           int32
	Li_disclosed_volume_remaining int32
	Li_total_volume_remaining     int32
	Li_volume                     int32
	Li_volume_filled_today        int32
	Li_price                      int32
	Li_trigger_price              int32
	Li_good_till_date             int32
	Li_entry_date_time            int32
	Li_minimum_fill_aon_volume    int32
	Li_last_modified              int32
	St_ord_flg                    *St_order_flags
	Si_branch_id                  int16
	Li_trader_id                  int32 // Changed from int16 to int32 in Ver 1.8
	C_broker_id                   [models.LEN_BROKER_ID]byte
	C_remarks                     [models.LEN_REMARKS]byte
	C_open_close                  string // byte
	C_settlor                     [models.LEN_SETTLOR]byte
	Si_pro_client_indicator       int16
	Si_settlement_period          int16
	C_cover_uncover               string // byte
	C_giveup_flag                 string // byte
	I_order_seq                   int32  // Changed from i_ordr_rfrnc to i_ordr_sqnc in Ver 1.7
	D_nnf_field                   float64
	D_filler19                    float64
	C_pan                         string // Added in Ver 2.7
	L_algo_id                     int32  // Added in Ver 2.7
	Si_algo_category              int16  // Added in Ver 2.7
	Ll_lastactivityref            int64  // Added in Ver 2.9
	C_reserved                    string // Updated in Ver 2.9
}

type St_int_header struct {
	Si_transaction_code int16
	Li_log_time         int32

	C_alpha_char      [models.LEN_ALPHA_CHAR]byte // this one is changed <---------------------
	Li_trader_id      int32
	Si_error_code     int16
	C_filler_2        string
	C_time_stamp_1    [models.LEN_TIME_STAMP]byte // this one is changed <---------------------
	C_time_stamp_2    [models.LEN_TIME_STAMP]byte // this one is changed <---------------------
	Si_message_length int32
}

type St_contract_desc struct {
	C_instrument_name [models.LEN_INSTRUMENT_NAME]byte
	C_symbol          [models.LEN_SYMBOL_NSE]byte
	Li_expiry_date    int32
	Li_strike_price   int64
	C_option_type     [models.LEN_OPTION_TYPE]byte
	Si_ca_level       int16
}

type St_order_flags struct {
	Flg_ato         uint32 // 1 bit
	Flg_market      uint32 // 1 bit
	Flg_sl          uint32 // 1 bit
	Flg_mit         uint32 // 1 bit
	Flg_day         uint32 // 1 bit
	Flg_gtc         uint32 // 1 bit
	Flg_ioc         uint32 // 1 bit
	Flg_aon         uint32 // 1 bit
	Flg_mf          uint32 // 1 bit
	Flg_matched_ind uint32 // 1 bit
	Flg_traded      uint32 // 1 bit
	Flg_modified    uint32 // 1 bit
	Flg_frozen      uint32 // 1 bit
	Flg_filler1     uint32 // 3 bits
}

//-------------------------------------------------------------------------------------------------------------------

// type St_pk_sequence struct { Not Used
// 	C_pipe_id  string
// 	C_trd_dt   string
// 	C_rqst_typ string
// 	C_seq_num  int32
// }

// type St_addtnal_order_flags struct {  Not Used
// 	C_cover_uncover string //byte
// }
