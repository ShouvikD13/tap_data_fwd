package app

import (
	"DATA_FWD_TAP/models"
	"DATA_FWD_TAP/models/structures"
	"log"
	"strconv"
	"strings"
	"time"

	"gorm.io/gorm"
)

type ClnPackClntManager struct {
	ServiceName  string                    // Name of the service being managed
	Xchngbook    *structures.Vw_xchngbook  // Pointer to the Vw_Xchngbook structure
	orderbook    *structures.Vw_orderbook  // Pointer to the Vw_orderbook structure
	contract     *structures.Vw_contract   // we are updating it's value from orderbook
	nse_contract *structures.Vw_nse_cntrct // we are updating it in 'fn_get_ext_cnt'
	requestQueue *structures.St_req_q_data // this is used in 'fnGetNxtRec'
	pipe_mstr    *structures.St_opm_pipe_mstr
	oe_reqres    *structures.St_oe_reqres
	exch_msg     *structures.St_exch_msg
	OCM          *models.OrderConversionManager
	cPanNo       string // Pan number, initialized in the 'fnRefToOrd' method
	cLstActRef   string // Last activity reference, initialized in the 'fnRefToOrd' method
	cEspID       string // ESP ID, initialized in the 'fnRefToOrd' method
	cAlgoID      string // Algorithm ID, initialized in the 'fnRefToOrd' method
	cSourceFlg   string // Source flag, initialized in the 'fnRefToOrd' method
	cPrgmFlg     string
	Envm         *models.EnvironmentManager
}

func (cpcm *ClnPackClntManager) Fn_bat_init(args []string, Db *gorm.DB) int {

	var temp_str string
	var resultTmp int
	cpcm.ServiceName = "cln_pack_clnt"
	cpcm.Xchngbook = &structures.Vw_xchngbook{}
	cpcm.orderbook = &structures.Vw_orderbook{}
	cpcm.contract = &structures.Vw_contract{}
	cpcm.nse_contract = &structures.Vw_nse_cntrct{}
	cpcm.requestQueue = &structures.St_req_q_data{}
	cpcm.pipe_mstr = &structures.St_opm_pipe_mstr{}
	cpcm.oe_reqres = &structures.St_oe_reqres{}
	cpcm.exch_msg = &structures.St_exch_msg{}
	cpcm.oe_reqres.St_hdr = &structures.St_int_header{}
	cpcm.oe_reqres.St_con_desc = &structures.St_contract_desc{}
	cpcm.oe_reqres.St_ord_flg = &structures.St_order_flags{}
	cpcm.OCM = &models.OrderConversionManager{}
	log.Printf("[%s]  [Fn_bat_init] Entering Fn_bat_init", cpcm.ServiceName)

	// we are getting the 7 args
	if len(args) < 7 {
		log.Printf("[%s] [Fn_bat_init] Error: insufficient arguments provided", cpcm.ServiceName)
		return -1
	}

	cpcm.Xchngbook.C_pipe_id = args[3]

	log.Printf("[%s] [Fn_bat_init] Copied pipe ID from args[3]: %s", cpcm.ServiceName, cpcm.Xchngbook.C_pipe_id)

	// Fetch exchange code
	queryForOpm_Xchng_Cd := `SELECT opm_xchng_cd
				FROM opm_ord_pipe_mstr
				WHERE opm_pipe_id = ?`

	log.Printf("[%s] [Fn_bat_init] Executing query to fetch exchange code with pipe ID: %s", cpcm.ServiceName, cpcm.Xchngbook.C_pipe_id)

	row := Db.Raw(queryForOpm_Xchng_Cd, cpcm.Xchngbook.C_pipe_id).Row()
	temp_str = ""
	err := row.Scan(&temp_str)
	if err != nil {
		log.Printf("[%s] [Fn_bat_init] Error scanning row for exchange code: %v", cpcm.Xchngbook.C_xchng_cd, err)
		return -1
	}

	cpcm.Xchngbook.C_xchng_cd = temp_str
	temp_str = ""
	log.Printf("[%s] [Fn_bat_init] Exchange code fetched: %s", cpcm.ServiceName, cpcm.Xchngbook.C_xchng_cd)

	//

	queryForTraderAndBranchID := `SELECT OPM_TRDR_ID, OPM_BRNCH_ID
				FROM OPM_ORD_PIPE_MSTR
				WHERE OPM_XCHNG_CD = ? AND OPM_PIPE_ID = ?`

	log.Printf("[%s] [Fn_bat_init] Executing query to fetch trader and branch ID with exchange code: %s and pipe ID: %s", cpcm.ServiceName, cpcm.Xchngbook.C_xchng_cd, cpcm.Xchngbook.C_pipe_id)

	rowQueryForTraderAndBranchID := Db.Raw(queryForTraderAndBranchID, cpcm.Xchngbook.C_xchng_cd, cpcm.Xchngbook.C_pipe_id).Row()

	errQueryForTraderAndBranchID := rowQueryForTraderAndBranchID.Scan(&cpcm.pipe_mstr.C_opm_trdr_id, &cpcm.pipe_mstr.L_opm_brnch_id)
	if errQueryForTraderAndBranchID != nil {
		log.Printf("[%s] [Fn_bat_init] Error scanning row for trader and branch ID: %v", cpcm.ServiceName, errQueryForTraderAndBranchID)
		return -1
	}

	log.Printf("[%s] [Fn_bat_init] fetched trader ID: %s and branch ID : %d", cpcm.ServiceName, cpcm.pipe_mstr.C_opm_trdr_id, cpcm.pipe_mstr.L_opm_brnch_id)
	// Fetch modification trade date
	queryFor_exg_nxt_trd_dt := `SELECT exg_nxt_trd_dt,
				exg_brkr_id
				FROM exg_xchng_mstr
				WHERE exg_xchng_cd = ?`

	log.Printf("[%s] [Fn_bat_init] Fetching trade date and broker ID with exchange code: %s", cpcm.ServiceName, cpcm.Xchngbook.C_xchng_cd)
	row2 := Db.Raw(queryFor_exg_nxt_trd_dt, cpcm.Xchngbook.C_xchng_cd).Row()

	err2 := row2.Scan(&temp_str, &cpcm.pipe_mstr.C_xchng_brkr_id)
	if err2 != nil {
		log.Printf("[%s] [Fn_bat_init] Error scanning trade date and broker ID: %v", cpcm.ServiceName, err2)
		return -1
	}

	cpcm.Xchngbook.C_mod_trd_dt = temp_str

	log.Printf("[%s] [Fn_bat_init] Fetched trade date : %s and broker ID : %s", cpcm.ServiceName, cpcm.Xchngbook.C_mod_trd_dt, cpcm.pipe_mstr.C_xchng_brkr_id)

	cpcm.Xchngbook.L_ord_seq = 0 // I am initially setting it to '0' because it was set that way in 'fn_bat_init' and I have not seen it getting changed anywhere. If I find it being changed somewhere, I will update it accordingly.

	log.Printf("[%s] [Fn_bat_init] L_ord_seq initialized to %d", cpcm.ServiceName, cpcm.Xchngbook.L_ord_seq)

	//here we are setting the S_user_typ_glb of 'St_opm_pipe_mstr' from the configuration file
	userTypeStr := cpcm.Envm.GetProcessSpaceValue("UserSettings", "UserType")
	userType, err := strconv.Atoi(userTypeStr)
	if err != nil {
		log.Printf("[%s] [Fn_bat_init] Failed to convert UserType '%s' to integer: %v", cpcm.ServiceName, userTypeStr, err)

	}
	cpcm.pipe_mstr.S_user_typ_glb = userType

	resultTmp = cpcm.CLN_PACK_CLNT(args, Db)
	if resultTmp != 0 {
		log.Printf("[%s] [Fn_bat_init] CLN_PACK_CLNT failed with result code: %d", cpcm.ServiceName, resultTmp)
		log.Printf("[%s] [Fn_bat_init] Returning to main from fn_bat_init", cpcm.ServiceName)
		return -1
	}
	log.Printf("[%s] [Fn_bat_init] Exiting Fn_bat_init", cpcm.ServiceName)
	return 0
}

func (cpcm *ClnPackClntManager) CLN_PACK_CLNT(args []string, Db *gorm.DB) int {
	var resultTmp int

	log.Printf("[%s] [CLN_PACK_CLNT] Entering CLN_PACK_CLNT", cpcm.ServiceName)
	resultTmp = cpcm.fnGetNxtRec(Db)
	if resultTmp != 0 {
		log.Printf("[%s] [CLN_PACK_CLNT] failed in getting the next record returning with result code: %d", cpcm.ServiceName, resultTmp)
		log.Printf("[%s] [CLN_PACK_CLNT] Exiting CLN_PACK_CLNT with error", cpcm.ServiceName)
		return -1
	}
	log.Printf("[%s] [CLN_PACK_CLNT] Exiting CLN_PACK_CLNT", cpcm.ServiceName)
	return 0
}

func (cpcm *ClnPackClntManager) fnGetNxtRec(Db *gorm.DB) int {
	log.Printf("[%s] [fnGetNxtRec] Entering fnGetNxtRec", cpcm.ServiceName)

	cpcm.cPrgmFlg = "0"

	resultTmp := cpcm.fnSeqToOmd(Db)
	if resultTmp != 0 {
		log.Printf("[%s] [fnGetNxtRec] Failed to fetch data into eXchngbook structure", cpcm.ServiceName)
		return -1
	}

	if cpcm.Xchngbook == nil {
		log.Printf("[%s] [fnGetNxtRec] Error: Xchngbook is nil", cpcm.ServiceName)
		return -1
	}

	if cpcm.Xchngbook.C_rms_prcsd_flg == "P" {
		log.Printf("[%s] [fnGetNxtRec] Order ref / Mod number = |%s| / |%d| Already Processed", cpcm.ServiceName, cpcm.Xchngbook.C_ordr_rfrnc, cpcm.Xchngbook.L_mdfctn_cntr)

		log.Printf("[%s] [fnGetNxtRec] Exiting from from 'fnGetNxtRec'", cpcm.ServiceName)

		return 0
	}

	if cpcm.Xchngbook.C_plcd_stts != models.REQUESTED {
		log.Printf("[%s] [fnGetNxtRec] the Placed status is not requested . so we are we are not getting the record ", cpcm.ServiceName)
		return -1
	}

	if cpcm.Xchngbook.C_ex_ordr_type == models.ORDINARY_ORDER {

		log.Printf("[%s] [fnGetNxtRec] Assigning C_ordr_rfrnc from eXchngbook to orderbook", cpcm.ServiceName)
		cpcm.orderbook.C_ordr_rfrnc = cpcm.Xchngbook.C_ordr_rfrnc

		resultTmp = cpcm.fnRefToOrd(Db)
		if resultTmp != 0 {
			log.Printf("[%s] [fnGetNxtRec] Failed to fetch data into orderbook structure", cpcm.ServiceName)
			return -1
		}

		if cpcm.Xchngbook.L_mdfctn_cntr != cpcm.orderbook.L_mdfctn_cntr {
			log.Printf("[%s] [fnGetNxtRec] L_mdfctn_cntr of both Xchngbook and order are not same", cpcm.ServiceName)
			log.Printf(" [fnGetNxtRec]  Exiting fnGetNxtRec")
			return -1

		}

		cpcm.Xchngbook.C_plcd_stts = models.QUEUED
		cpcm.Xchngbook.C_oprn_typ = models.UPDATION_ON_ORDER_FORWARDING

		resultTmp = cpcm.fnUpdXchngbk(Db)

		if resultTmp != 0 {
			log.Printf("[%s] [fnGetNxtRec] failed to update the status in Xchngbook", cpcm.ServiceName)
			return -1
		}

		cpcm.orderbook.C_ordr_stts = models.QUEUED

		resultTmp = cpcm.fnUpdOrdrbk(Db)

		if resultTmp != 0 {
			log.Printf("[%s] failed to update the status in Order book", cpcm.ServiceName)
			return -1
		}

		// here we are setting the contract data from orderbook structure
		log.Printf("[%s]  [fnGetNxtRec] Setting contract data from orderbook structure", cpcm.ServiceName)
		cpcm.contract.C_xchng_cd = cpcm.orderbook.C_xchng_cd
		cpcm.contract.C_prd_typ = cpcm.orderbook.C_prd_typ
		cpcm.contract.C_undrlyng = cpcm.orderbook.C_undrlyng
		cpcm.contract.C_expry_dt = cpcm.orderbook.C_expry_dt
		cpcm.contract.C_exrc_typ = cpcm.orderbook.C_exrc_typ
		cpcm.contract.C_opt_typ = cpcm.orderbook.C_opt_typ
		cpcm.contract.L_strike_prc = cpcm.orderbook.L_strike_prc
		cpcm.contract.C_ctgry_indstk = cpcm.orderbook.C_ctgry_indstk
		// cpcm.contract.L_ca_lvl = cpcm.orderbook.L_ca_lvl

		log.Printf("[%s] [fnGetNxtRec] contract.C_xchng_cd: %s ", cpcm.ServiceName, cpcm.contract.C_xchng_cd)
		log.Printf("[%s] [fnGetNxtRec] contract.C_prd_typ: %s ", cpcm.ServiceName, cpcm.contract.C_prd_typ)
		log.Printf("[%s] [fnGetNxtRec] contract.C_undrlyng: %s ", cpcm.ServiceName, cpcm.contract.C_undrlyng)
		log.Printf("[%s] [fnGetNxtRec] contract.C_expry_dt: %s ", cpcm.ServiceName, cpcm.contract.C_expry_dt)
		log.Printf("[%s] [fnGetNxtRec] contract.C_exrc_typ: %s ", cpcm.ServiceName, cpcm.contract.C_exrc_typ)
		log.Printf("[%s] [fnGetNxtRec] contract.C_opt_typ: %s ", cpcm.ServiceName, cpcm.contract.C_opt_typ)
		log.Printf("[%s] [fnGetNxtRec] contract.L_strike_prc: %d ", cpcm.ServiceName, cpcm.contract.L_strike_prc)
		log.Printf("[%s] [fnGetNxtRec] contract.C_ctgry_indstk: %s ", cpcm.ServiceName, cpcm.contract.C_ctgry_indstk)
		//log.Printf("[%s] contract.L_ca_lvl: |%d|", cpcm.ServiceName, cpcm.contract.L_ca_lvl)

		if strings.TrimSpace(cpcm.Xchngbook.C_xchng_cd) == "NFO" {

			cpcm.contract.C_rqst_typ = models.CONTRACT_TO_NSE_ID

			resultTmp = cpcm.fnGetExtCnt(Db)

			if resultTmp != 0 {
				log.Printf("[%s]  [fnGetNxtRec] failed to load data in NSE CNT ", cpcm.ServiceName)
				return -1
			}
		} else {
			log.Printf("[%s] [fnGetNxtRec] returning from 'fnGetNxtRec' because 'Xchngbook.C_xchng_cd' is not 'NFO' --> %s ", cpcm.ServiceName, cpcm.Xchngbook.C_xchng_cd)
			log.Printf("[%s] [fnGetNxtRec] returning with Error", cpcm.ServiceName)
			return -1
		}

		log.Printf("[%s] [fnGetNxtRec] Packing Ordinary Order STARTS ", cpcm.ServiceName)

		if cpcm.nse_contract.L_token_id == 0 {
			log.Printf("[%s] [fnGetNxtRec] Token id for ordinary order is: %d", cpcm.ServiceName, cpcm.nse_contract.L_token_id)

			log.Printf("[%s] [fnGetNxtRec] token id is not avialable so we are calling 'fn_rjct_rcrd' ", cpcm.ServiceName)

			resultTmp = cpcm.fnRjctRcrd(Db)

			if resultTmp != 0 {
				log.Printf("[%s] [fnGetNxtRec] returned from 'fnRjctRcrd' with Error", cpcm.ServiceName)
				log.Printf("[%s] [fnGetNxtRec] Exiting from 'fnGetNxtRec' ", cpcm.ServiceName)
				return -1
			}

		}

		// here we are initialising the "ExchngPackLibMaster"
		eplm := NewExchngPackLibMaster(
			cpcm.ServiceName,
			cpcm.requestQueue,
			cpcm.Xchngbook,
			cpcm.orderbook,
			cpcm.pipe_mstr,
			cpcm.nse_contract,
			cpcm.oe_reqres,
			cpcm.exch_msg,
			cpcm.OCM,
			cpcm.cPanNo,
			cpcm.cLstActRef,
			cpcm.cEspID,
			cpcm.cAlgoID,
			cpcm.cSourceFlg,
			cpcm.cPrgmFlg,
		)

		resultTmp = eplm.fnPackOrdnryOrdToNse(Db)

		if resultTmp != 0 {
			log.Printf("[%s]  [fnGetNxtRec] 'fnPackOrdnryOrdToNse' returned an error.", cpcm.ServiceName)
			log.Printf("[%s] [fnGetNxtRec] Exiting 'fnGetNxtRec'.", cpcm.ServiceName)
		} else {
			log.Printf("[%s] [fnGetNxtRec] Data successfully packed.", cpcm.ServiceName)
		}

	}

	log.Printf("[%s] [fnGetNxtRec] Exiting fnGetNxtRec", cpcm.ServiceName)
	return 0
}

func (cpcm *ClnPackClntManager) fnSeqToOmd(db *gorm.DB) int {
	var c_ip_addrs string
	var c_prcimpv_flg string

	log.Printf("[%s][fnSeqToOmd] Entering fnSeqToOmd", cpcm.ServiceName)
	log.Printf("[%s][fnSeqToOmd] Before extracting the data from the 'fxb_ordr_rfrnc' and storing it in the 'Xchngbook' structure", cpcm.ServiceName)

	query := `
		SELECT fxb_ordr_rfrnc,
		fxb_lmt_mrkt_sl_flg AS C_slm_flg,
		fxb_dsclsd_qty AS L_dsclsd_qty,
		fxb_ordr_tot_qty AS L_ord_tot_qty,
		fxb_lmt_rt AS L_ord_lmt_rt,
		fxb_stp_lss_tgr AS L_stp_lss_tgr,
		TO_CHAR(TO_DATE(fxb_ordr_valid_dt, 'YYYY-MM-DD'), 'DD-Mon-YYYY') AS C_valid_dt,
		CASE
			WHEN fxb_ordr_type = 'V' THEN 'T'
			ELSE fxb_ordr_type
		END AS C_ord_typ,
		fxb_rqst_typ AS C_req_typ,
		fxb_ordr_sqnc AS L_ord_seq,
		COALESCE(fxb_ip, 'NA') AS C_ip_addrs,
		COALESCE(fxb_prcimpv_flg, 'N') AS C_prcimpv_flg,
		fxb_ex_ordr_typ AS C_ex_ordr_type,
		fxb_plcd_stts AS C_plcd_stts,
		fxb_mdfctn_cntr AS L_mdfctn_cntr,
		fxb_frwd_tm AS C_frwrd_tm,
		fxb_jiffy AS D_jiffy,
		fxb_xchng_rmrks AS C_xchng_rmrks,
		fxb_rms_prcsd_flg AS C_rms_prcsd_flg,
		fxb_ors_msg_typ AS L_ors_msg_typ,
		fxb_ack_tm AS C_ack_tm
		FROM FXB_FO_XCHNG_BOOK
		WHERE fxb_xchng_cd = ?
		AND fxb_pipe_id = ?
		AND TO_DATE(fxb_mod_trd_dt, 'YYYY-MM-DD') = TO_DATE(?, 'YYYY-MM-DD')
		AND fxb_ordr_sqnc = (
		SELECT MIN(fxb_b.fxb_ordr_sqnc)
		FROM FXB_FO_XCHNG_BOOK fxb_b
		WHERE fxb_b.fxb_xchng_cd = ?
         AND TO_DATE(fxb_b.fxb_mod_trd_dt, 'YYYY-MM-DD') = TO_DATE(?, 'YYYY-MM-DD')
         AND fxb_b.fxb_pipe_id = ?
         AND fxb_b.fxb_plcd_stts = 'R'
);


	`

	log.Printf("[%s][fnSeqToOmd] Executing query to fetch order details", cpcm.ServiceName)

	log.Printf("[%s][fnSeqToOmd] C_xchng_cd: %s", cpcm.ServiceName, cpcm.Xchngbook.C_xchng_cd)
	log.Printf("[%s][fnSeqToOmd] C_pipe_id: %s", cpcm.ServiceName, cpcm.Xchngbook.C_pipe_id)
	log.Printf("[%s][fnSeqToOmd] C_mod_trd_dt: %s", cpcm.ServiceName, cpcm.Xchngbook.C_mod_trd_dt)

	row := db.Raw(query,
		cpcm.Xchngbook.C_xchng_cd,
		cpcm.Xchngbook.C_pipe_id,
		cpcm.Xchngbook.C_mod_trd_dt,
		cpcm.Xchngbook.C_xchng_cd,
		cpcm.Xchngbook.C_mod_trd_dt,
		cpcm.Xchngbook.C_pipe_id).Row()

	err := row.Scan(
		&cpcm.Xchngbook.C_ordr_rfrnc,    // fxb_ordr_rfrnc
		&cpcm.Xchngbook.C_slm_flg,       // fxb_lmt_mrkt_sl_flg
		&cpcm.Xchngbook.L_dsclsd_qty,    // fxb_dsclsd_qty
		&cpcm.Xchngbook.L_ord_tot_qty,   // fxb_ordr_tot_qty
		&cpcm.Xchngbook.L_ord_lmt_rt,    // fxb_lmt_rt
		&cpcm.Xchngbook.L_stp_lss_tgr,   // fxb_stp_lss_tgr
		&cpcm.Xchngbook.C_valid_dt,      // C_valid_dt (formatted as 'DD-Mon-YYYY')
		&cpcm.Xchngbook.C_ord_typ,       // C_ord_typ (transformed 'V' to 'T')
		&cpcm.Xchngbook.C_req_typ,       // C_req_typ
		&cpcm.Xchngbook.L_ord_seq,       // L_ord_seq
		&c_ip_addrs,                     // COALESCE(fxb_ip, 'NA')
		&c_prcimpv_flg,                  // COALESCE(fxb_prcimpv_flg, 'N')
		&cpcm.Xchngbook.C_ex_ordr_type,  // C_ex_ordr_type
		&cpcm.Xchngbook.C_plcd_stts,     // C_plcd_stts
		&cpcm.Xchngbook.L_mdfctn_cntr,   // L_mdfctn_cntr
		&cpcm.Xchngbook.C_frwrd_tm,      // C_frwrd_tm
		&cpcm.Xchngbook.D_jiffy,         // D_jiffy
		&cpcm.Xchngbook.C_xchng_rmrks,   // C_xchng_rmrks
		&cpcm.Xchngbook.C_rms_prcsd_flg, // C_rms_prcsd_flg
		&cpcm.Xchngbook.L_ors_msg_typ,   // L_ors_msg_typ
		&cpcm.Xchngbook.C_ack_tm,        // C_ack_tm
	)

	if err != nil {
		log.Printf("[%s][fnSeqToOmd] Error scanning row: %v", cpcm.ServiceName, err)
		log.Printf("[%s][fnSeqToOmd] Exiting fnSeqToOmd with error", cpcm.ServiceName)
		return -1
	}

	if c_prcimpv_flg == "Y" {
		cpcm.cPrgmFlg = "T"
	} else {
		cpcm.cPrgmFlg = c_ip_addrs
	}

	log.Printf("[%s][fnSeqToOmd] Data extracted and stored in the 'Xchngbook' structure:", cpcm.ServiceName)
	log.Printf("[%s][fnSeqToOmd]   C_ordr_rfrnc:     %s", cpcm.ServiceName, cpcm.Xchngbook.C_ordr_rfrnc)
	log.Printf("[%s][fnSeqToOmd]   C_slm_flg:        %s", cpcm.ServiceName, cpcm.Xchngbook.C_slm_flg)
	log.Printf("[%s][fnSeqToOmd]   L_dsclsd_qty:     %d", cpcm.ServiceName, cpcm.Xchngbook.L_dsclsd_qty)
	log.Printf("[%s][fnSeqToOmd]   L_ord_tot_qty:    %d", cpcm.ServiceName, cpcm.Xchngbook.L_ord_tot_qty)
	log.Printf("[%s][fnSeqToOmd]   L_ord_lmt_rt:     %d", cpcm.ServiceName, cpcm.Xchngbook.L_ord_lmt_rt)
	log.Printf("[%s][fnSeqToOmd]   L_stp_lss_tgr:    %d", cpcm.ServiceName, cpcm.Xchngbook.L_stp_lss_tgr)
	log.Printf("[%s][fnSeqToOmd]   C_valid_dt:       %s", cpcm.ServiceName, cpcm.Xchngbook.C_valid_dt)
	log.Printf("[%s][fnSeqToOmd]   C_ord_typ:        %s", cpcm.ServiceName, cpcm.Xchngbook.C_ord_typ)
	log.Printf("[%s][fnSeqToOmd]   C_req_typ:        %s", cpcm.ServiceName, cpcm.Xchngbook.C_req_typ)
	log.Printf("[%s][fnSeqToOmd]   L_ord_seq:        %d", cpcm.ServiceName, cpcm.Xchngbook.L_ord_seq)
	log.Printf("[%s][fnSeqToOmd]   IpAddrs:          %s", cpcm.ServiceName, c_ip_addrs)
	log.Printf("[%s][fnSeqToOmd]   PrcimpvFlg:       %s", cpcm.ServiceName, c_prcimpv_flg)
	log.Printf("[%s][fnSeqToOmd]   C_ex_ordr_type:   %s", cpcm.ServiceName, cpcm.Xchngbook.C_ex_ordr_type)
	log.Printf("[%s][fnSeqToOmd]   C_plcd_stts:      %s", cpcm.ServiceName, cpcm.Xchngbook.C_plcd_stts)
	log.Printf("[%s][fnSeqToOmd]   L_mdfctn_cntr:    %d", cpcm.ServiceName, cpcm.Xchngbook.L_mdfctn_cntr)
	log.Printf("[%s][fnSeqToOmd]   C_frwrd_tm:       %s", cpcm.ServiceName, cpcm.Xchngbook.C_frwrd_tm)
	log.Printf("[%s][fnSeqToOmd]   D_jiffy:          %f", cpcm.ServiceName, cpcm.Xchngbook.D_jiffy)
	log.Printf("[%s][fnSeqToOmd]   C_xchng_rmrks:    %s", cpcm.ServiceName, cpcm.Xchngbook.C_xchng_rmrks)
	log.Printf("[%s][fnSeqToOmd]   C_rms_prcsd_flg:  %s", cpcm.ServiceName, cpcm.Xchngbook.C_rms_prcsd_flg)
	log.Printf("[%s][fnSeqToOmd]   L_ors_msg_typ:    %d", cpcm.ServiceName, cpcm.Xchngbook.L_ors_msg_typ)
	log.Printf("[%s][fnSeqToOmd]   C_ack_tm:         %s", cpcm.ServiceName, cpcm.Xchngbook.C_ack_tm)

	log.Printf("[%s][fnSeqToOmd] Data extracted and stored in the 'Xchngbook' structure successfully", cpcm.ServiceName)

	log.Printf("[%s][fnSeqToOmd] Exiting fnSeqToOmd", cpcm.ServiceName)
	return 0
}

func (cpcm *ClnPackClntManager) fnRefToOrd(db *gorm.DB) int {
	/*
		c_pan_no --> we are simply setting it to 0 using memset in the
		MEMSET(c_ordr_rfrnc);
		MEMSET(c_pan_no);
		MEMSET(c_lst_act_ref);
		MEMSET(c_esp_id);
		MEMSET(c_algo_id);
		char c_source_flg = '\0';
	*/

	log.Printf("[%s] [fnRefToOrd]  Entering fnFetchOrderDetails", cpcm.ServiceName)
	log.Printf("[%s] [fnRefToOrd] Before extracting the data from the 'fod_fo_ordr_dtls' and storing it in the 'orderbook' structure", cpcm.ServiceName)

	query := `
    SELECT
    fod_clm_mtch_accnt AS C_cln_mtch_accnt,
    fod_ordr_flw AS C_ordr_flw,
    fod_ordr_tot_qty AS L_ord_tot_qty,
    fod_exec_qty AS L_exctd_qty,
    COALESCE(fod_exec_qty_day, 0) AS L_exctd_qty_day,
    fod_settlor AS C_settlor,
    fod_spl_flag AS C_spl_flg,
    TO_CHAR(fod_ord_ack_tm, 'YYYY-MM-DD HH24:MI:SS') AS C_ack_tm,
    TO_CHAR(fod_lst_rqst_ack_tm, 'YYYY-MM-DD HH24:MI:SS') AS C_prev_ack_tm,
    fod_pro_cli_ind AS C_pro_cli_ind,
    COALESCE(fod_ctcl_id, ' ') AS C_ctcl_id,
    COALESCE(fod_pan_no, '*') AS C_pan_no,
    COALESCE(fod_lst_act_ref, '0') AS C_lst_act_ref,
    COALESCE(fod_esp_id, '*') AS C_esp_id,
    COALESCE(fod_algo_id, '*') AS C_algo_id,
    COALESCE(fod_source_flg, '*') AS C_source_flg,
    fod_mdfctn_cntr AS L_mdfctn_cntr,
    fod_ordr_stts AS C_ordr_stts,
    fod_xchng_cd AS C_xchng_cd,
    fod_prdct_typ AS C_prd_typ,
    fod_undrlying AS C_undrlyng,
    fod_expry_dt AS C_expry_dt,
    fod_exer_typ AS C_exrc_typ,
    fod_opt_typ AS C_opt_typ,
    fod_strk_prc AS L_strike_prc,
    fod_indstk AS C_ctgry_indstk,
	fod_ack_nmbr As C_xchng_ack
	FROM
		fod_fo_ordr_dtls
	WHERE
		fod_ordr_rfrnc =  ?
	FOR UPDATE NOWAIT;

    `

	log.Printf("[%s] [fnRefToOrd] Executing query to fetch order details", cpcm.ServiceName)

	log.Printf("[%s] [fnRefToOrd] Order Reference: %s", cpcm.ServiceName, cpcm.orderbook.C_ordr_rfrnc)

	row := db.Raw(query, strings.TrimSpace(cpcm.orderbook.C_ordr_rfrnc)).Row()

	err := row.Scan(
		&cpcm.orderbook.C_cln_mtch_accnt,
		&cpcm.orderbook.C_ordr_flw,
		&cpcm.orderbook.L_ord_tot_qty,
		&cpcm.orderbook.L_exctd_qty,
		&cpcm.orderbook.L_exctd_qty_day,
		&cpcm.orderbook.C_settlor,
		&cpcm.orderbook.C_spl_flg,
		&cpcm.orderbook.C_ack_tm,
		&cpcm.orderbook.C_prev_ack_tm,
		&cpcm.orderbook.C_pro_cli_ind,
		&cpcm.orderbook.C_ctcl_id,
		&cpcm.cPanNo,
		&cpcm.cLstActRef,
		&cpcm.cEspID,
		&cpcm.cAlgoID,
		&cpcm.cSourceFlg,
		&cpcm.orderbook.L_mdfctn_cntr,
		&cpcm.orderbook.C_ordr_stts,
		&cpcm.orderbook.C_xchng_cd,
		&cpcm.orderbook.C_prd_typ,
		&cpcm.orderbook.C_undrlyng,
		&cpcm.orderbook.C_expry_dt,
		&cpcm.orderbook.C_exrc_typ,
		&cpcm.orderbook.C_opt_typ,
		&cpcm.orderbook.L_strike_prc,
		&cpcm.orderbook.C_ctgry_indstk,
		&cpcm.orderbook.C_xchng_ack,
	)

	if err != nil {
		log.Printf("[%s] [fnRefToOrd] Error scanning row: %v", cpcm.ServiceName, err)
		log.Printf("[%s] [fnRefToOrd] Exiting fnFetchOrderDetails with error", cpcm.ServiceName)
		return -1
	}

	cpcm.cPanNo = strings.TrimSpace(cpcm.cPanNo)
	cpcm.cLstActRef = strings.TrimSpace(cpcm.cLstActRef)
	cpcm.cEspID = strings.TrimSpace(cpcm.cEspID)
	cpcm.cAlgoID = strings.TrimSpace(cpcm.cAlgoID)
	cpcm.cSourceFlg = strings.TrimSpace(cpcm.cSourceFlg)

	log.Printf("[%s] [fnRefToOrd] Data extracted and stored in the 'orderbook' structure:", cpcm.ServiceName)

	log.Printf("[%s] [fnRefToOrd]   C_cln_mtch_accnt:   %s", cpcm.ServiceName, cpcm.orderbook.C_cln_mtch_accnt)
	log.Printf("[%s] [fnRefToOrd]   C_ordr_flw:        %s", cpcm.ServiceName, cpcm.orderbook.C_ordr_flw)
	log.Printf("[%s] [fnRefToOrd]   L_ord_tot_qty:     %d", cpcm.ServiceName, cpcm.orderbook.L_ord_tot_qty)
	log.Printf("[%s] [fnRefToOrd]   L_exctd_qty:       %d", cpcm.ServiceName, cpcm.orderbook.L_exctd_qty)
	log.Printf("[%s] [fnRefToOrd]   L_exctd_qty_day:   %d", cpcm.ServiceName, cpcm.orderbook.L_exctd_qty_day)
	log.Printf("[%s] [fnRefToOrd]   C_settlor:         %s", cpcm.ServiceName, cpcm.orderbook.C_settlor)
	log.Printf("[%s] [fnRefToOrd]   C_spl_flg:         %s", cpcm.ServiceName, cpcm.orderbook.C_spl_flg)
	log.Printf("[%s] [fnRefToOrd]   C_ack_tm:          %s", cpcm.ServiceName, cpcm.orderbook.C_ack_tm)
	log.Printf("[%s] [fnRefToOrd]   C_prev_ack_tm:     %s", cpcm.ServiceName, cpcm.orderbook.C_prev_ack_tm)
	log.Printf("[%s] [fnRefToOrd]   C_pro_cli_ind:     %s", cpcm.ServiceName, cpcm.orderbook.C_pro_cli_ind)
	log.Printf("[%s] [fnRefToOrd]   C_ctcl_id:         %s", cpcm.ServiceName, cpcm.orderbook.C_ctcl_id)
	log.Printf("[%s] [fnRefToOrd]   C_pan_no:          %s", cpcm.ServiceName, cpcm.cPanNo)
	log.Printf("[%s] [fnRefToOrd]   C_lst_act_ref_tmp: %s", cpcm.ServiceName, cpcm.cLstActRef)
	log.Printf("[%s] [fnRefToOrd]   C_esp_id:          %s", cpcm.ServiceName, cpcm.cEspID)
	log.Printf("[%s] [fnRefToOrd]   C_algo_id:         %s", cpcm.ServiceName, cpcm.cAlgoID)
	log.Printf("[%s] [fnRefToOrd]   C_source_flg_tmp:  %s", cpcm.ServiceName, cpcm.cSourceFlg)
	log.Printf("[%s] [fnRefToOrd]   L_mdfctn_cntr:     %d", cpcm.ServiceName, cpcm.orderbook.L_mdfctn_cntr)
	log.Printf("[%s] [fnRefToOrd]   C_ordr_stts:       %s", cpcm.ServiceName, cpcm.orderbook.C_ordr_stts)
	log.Printf("[%s] [fnRefToOrd]   C_xchng_cd:        %s", cpcm.ServiceName, cpcm.orderbook.C_xchng_cd)
	log.Printf("[%s] [fnRefToOrd]   C_prd_typ:         %s", cpcm.ServiceName, cpcm.orderbook.C_prd_typ)
	log.Printf("[%s] [fnRefToOrd]   C_undrlyng:        %s", cpcm.ServiceName, cpcm.orderbook.C_undrlyng)
	log.Printf("[%s] [fnRefToOrd]   C_expry_dt:        %s", cpcm.ServiceName, cpcm.orderbook.C_expry_dt)
	log.Printf("[%s] [fnRefToOrd]   C_exrc_typ:        %s", cpcm.ServiceName, cpcm.orderbook.C_exrc_typ)
	log.Printf("[%s] [fnRefToOrd]   C_opt_typ:         %s", cpcm.ServiceName, cpcm.orderbook.C_opt_typ)
	log.Printf("[%s] [fnRefToOrd]   L_strike_prc:      %d", cpcm.ServiceName, cpcm.orderbook.L_strike_prc)
	log.Printf("[%s] [fnRefToOrd]   C_ctgry_indstk:    %s", cpcm.ServiceName, cpcm.orderbook.C_ctgry_indstk)
	log.Printf("[%s] [fnRefToOrd]   C_xchng_ack	: 	   %s", cpcm.ServiceName, cpcm.orderbook.C_xchng_ack)

	log.Printf("[%s] [fnRefToOrd] Data extracted and stored in the 'orderbook' structure successfully", cpcm.ServiceName)

	clmQuery := `
    SELECT
    COALESCE(TRIM(CLM_CP_CD), ' '),
    COALESCE(TRIM(CLM_CLNT_CD), CLM_MTCH_ACCNT)
	FROM
    CLM_CLNT_MSTR
	WHERE
    CLM_MTCH_ACCNT = ?
    `
	log.Printf("[%s] [fnRefToOrd] Executing query to fetch data from 'CLM_CLNT_MSTR' ", cpcm.ServiceName)

	row = db.Raw(clmQuery, strings.TrimSpace(cpcm.orderbook.C_cln_mtch_accnt)).Row()

	var cCpCode, cUccCd string
	err = row.Scan(&cCpCode, &cUccCd)

	if err != nil {
		log.Printf("[%s] [fnRefToOrd] Error scanning client details: %v", cpcm.ServiceName, err)
		log.Printf("[%s] [fnRefToOrd] Exiting fnRefToOrd with error", cpcm.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnRefToOrd] cCpCode : %s", cpcm.ServiceName, cCpCode)
	log.Printf("[%s] [fnRefToOrd] cUccCd : %s", cpcm.ServiceName, cUccCd)

	log.Printf("[%s] [fnRefToOrd] Updating orderbook with client details", cpcm.ServiceName)

	cpcm.orderbook.C_cln_mtch_accnt = strings.TrimSpace(cUccCd)
	cpcm.orderbook.C_settlor = strings.TrimSpace(cCpCode)
	cpcm.orderbook.C_ctcl_id = strings.TrimSpace(cpcm.orderbook.C_ctcl_id)

	log.Printf("[%s] [fnRefToOrd] Exiting fnFetchOrderDetails", cpcm.ServiceName)
	return 0
}

func (cpcm *ClnPackClntManager) fnUpdXchngbk(db *gorm.DB) int {

	var iRecExists int64

	log.Printf("[%s] [fnUpdXchngbk] Entering fnUpdXchngbk", cpcm.ServiceName)
	log.Printf("[%s] [fnUpdXchngbk]: %s", cpcm.ServiceName, cpcm.Xchngbook.C_xchng_cd)
	log.Printf("[%s] [fnUpdXchngbk]: %s", cpcm.ServiceName, cpcm.Xchngbook.C_ordr_rfrnc)
	log.Printf("[%s] [fnUpdXchngbk]: %s", cpcm.ServiceName, cpcm.Xchngbook.C_pipe_id)
	log.Printf("[%s] [fnUpdXchngbk]: %s", cpcm.ServiceName, cpcm.Xchngbook.C_mod_trd_dt)
	log.Printf("[%s] [fnUpdXchngbk]: %d", cpcm.ServiceName, cpcm.Xchngbook.L_ord_seq)
	log.Printf("[%s] [fnUpdXchngbk]: %s", cpcm.ServiceName, cpcm.Xchngbook.C_slm_flg)
	log.Printf("[%s] [fnUpdXchngbk]: %d", cpcm.ServiceName, cpcm.Xchngbook.L_dsclsd_qty)
	log.Printf("[%s] [fnUpdXchngbk]: %d", cpcm.ServiceName, cpcm.Xchngbook.L_ord_tot_qty)
	log.Printf("[%s] [fnUpdXchngbk]: %d", cpcm.ServiceName, cpcm.Xchngbook.L_ord_lmt_rt)
	log.Printf("[%s] [fnUpdXchngbk]: %d", cpcm.ServiceName, cpcm.Xchngbook.L_stp_lss_tgr)
	log.Printf("[%s] [fnUpdXchngbk]: %d", cpcm.ServiceName, cpcm.Xchngbook.L_mdfctn_cntr)
	log.Printf("[%s] [fnUpdXchngbk]: %s", cpcm.ServiceName, cpcm.Xchngbook.C_valid_dt)
	log.Printf("[%s] [fnUpdXchngbk]: %s", cpcm.ServiceName, cpcm.Xchngbook.C_ord_typ)
	log.Printf("[%s] [fnUpdXchngbk]: %s", cpcm.ServiceName, cpcm.Xchngbook.C_req_typ)
	log.Printf("[%s] [fnUpdXchngbk]: %s", cpcm.ServiceName, cpcm.Xchngbook.C_plcd_stts)

	switch cpcm.Xchngbook.C_oprn_typ {
	case models.UPDATION_ON_ORDER_FORWARDING:

		query1 := `UPDATE fxb_fo_xchng_book 
				SET fxb_plcd_stts = ?, fxb_frwd_tm = CURRENT_TIMESTAMP
				WHERE fxb_ordr_rfrnc = ? 
				AND fxb_mdfctn_cntr = ?`

		result := db.Exec(query1, cpcm.Xchngbook.C_plcd_stts, cpcm.Xchngbook.C_ordr_rfrnc, cpcm.Xchngbook.L_mdfctn_cntr)
		if result.Error != nil {
			log.Printf("[%s] [fnUpdXchngbk] Error updating order forwarding: %v", cpcm.ServiceName, result.Error)
			return -1
		}

	case models.UPDATION_ON_EXCHANGE_RESPONSE:
		if cpcm.Xchngbook.L_dwnld_flg == models.DOWNLOAD {

			result := db.Raw(
				`SELECT COUNT(*) 
					FROM fxb_fo_xchng_book 
					WHERE fxb_jiffy = ? 
					AND fxb_xchng_cd = ? 
					AND fxb_pipe_id = ?`,
				cpcm.Xchngbook.D_jiffy, cpcm.Xchngbook.C_xchng_cd, cpcm.Xchngbook.C_pipe_id,
			).Scan(&iRecExists)
			if result.Error != nil {
				log.Printf("[%s] [fnUpdXchngbk] SQL error: %v", cpcm.ServiceName, result.Error)
				return -1
			}

			if iRecExists > 0 {
				log.Printf("[%s] [fnUpdXchngbk] Record already processed", cpcm.ServiceName)
				return -1
			}
		}

		c_xchng_rmrks := strings.TrimSpace(cpcm.Xchngbook.C_xchng_rmrks)

		result := db.Exec(
			`UPDATE fxb_fo_xchng_book 
				SET fxb_plcd_stts = ?, 
					fxb_rms_prcsd_flg = ?, 
					fxb_ors_msg_typ = ?, 
					fxb_ack_tm = TO_DATE(?, 'DD-Mon-yyyy hh24:mi:ss'), 
					fxb_xchng_rmrks = RTRIM(fxb_xchng_rmrks) || ?, 
					fxb_jiffy = ? 
				WHERE fxb_ordr_rfrnc = ? 
				AND fxb_mdfctn_cntr = ?`,
			cpcm.Xchngbook.C_plcd_stts,
			cpcm.Xchngbook.C_rms_prcsd_flg,
			cpcm.Xchngbook.L_ors_msg_typ,
			cpcm.Xchngbook.C_ack_tm,
			c_xchng_rmrks,
			cpcm.Xchngbook.D_jiffy,
			cpcm.Xchngbook.C_ordr_rfrnc,
			cpcm.Xchngbook.L_mdfctn_cntr,
		)
		if result.Error != nil {
			log.Printf("[%s] [fnUpdXchngbk] Error updating exchange response: %v", cpcm.ServiceName, result.Error)
			return -1
		}

	default:
		log.Printf("[%s] [fnUpdXchngbk] Invalid Operation Type", cpcm.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnUpdXchngbk] Exiting fnUpdXchngbk", cpcm.ServiceName)

	return 0
}

func (cpcm *ClnPackClntManager) fnUpdOrdrbk(db *gorm.DB) int {

	log.Printf("[%s] [fnUpdOrdrbk] Starting 'fnUpdOrdbk' function", cpcm.ServiceName)

	query := `
    UPDATE fod_fo_ordr_dtls
    SET fod_ordr_stts = ?
    WHERE fod_ordr_rfrnc = ?;
	`

	log.Printf("[%s] [fnUpdOrdrbk] Executing query to update order status", cpcm.ServiceName)

	log.Printf("[%s] [fnUpdOrdrbk] Order Reference: %s", cpcm.ServiceName, cpcm.orderbook.C_ordr_rfrnc)
	log.Printf("[%s] [fnUpdOrdrbk]  Order Status: %s", cpcm.ServiceName, cpcm.orderbook.C_ordr_stts)

	result := db.Exec(query, cpcm.orderbook.C_ordr_stts, strings.TrimSpace(cpcm.orderbook.C_ordr_rfrnc))

	if result.Error != nil {
		log.Printf("[%s] [fnUpdOrdrbk] Error executing update query: %v", cpcm.ServiceName, result.Error)
		log.Printf("[%s] [fnUpdOrdrbk] Exiting 'fnUpdOrdbk' with error", cpcm.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnUpdOrdrbk] Order status updated successfully", cpcm.ServiceName)

	log.Printf("[%s] [fnUpdOrdrbk] returning from 'fnUpdOrdbk' function", cpcm.ServiceName)

	return 0
}

func (cpcm *ClnPackClntManager) fnGetExtCnt(db *gorm.DB) int {
	var i_sem_entity int
	var c_stck_cd, c_symbl, c_exg_cd string

	log.Printf("[%s] [fnGetExtCnt] Entering 'fnGetExtCnt' ", cpcm.ServiceName)

	if cpcm.contract.C_xchng_cd == "NFO" {
		i_sem_entity = models.NFO_ENTTY
	} else {
		i_sem_entity = models.BFO_ENTTY
	}

	c_stck_cd = cpcm.contract.C_undrlyng

	c_stck_cd = strings.ToUpper(c_stck_cd)
	log.Printf("[%s] [fnGetExtCnt] Converted 'c_stck_cd' to uppercase: %s", cpcm.ServiceName, c_stck_cd)

	query1 := `
		SELECT TRIM(sem_map_vl)
		FROM sem_stck_map
		WHERE sem_entty = ?
		AND sem_stck_cd = ?;
	`
	log.Printf("[%s] [fnGetExtCnt] Executing query to fetch 'C_symbol'", cpcm.ServiceName)

	row1 := db.Raw(query1, i_sem_entity, c_stck_cd).Row()

	err := row1.Scan(&c_symbl)

	if err != nil {
		log.Printf("[%s] [fnGetExtCnt] Error scanning row: %v", cpcm.ServiceName, err)
		log.Printf("[%s] [fnGetExtCnt] Exiting 'fnGetExtCnt' due to error", cpcm.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnGetExtCnt] Value successfully fetched from the table 'sem_stck_map'", cpcm.ServiceName)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.C_symbol is: %s", cpcm.ServiceName, c_symbl)

	// Assign values to the NSE contract structure
	cpcm.nse_contract.C_xchng_cd = cpcm.contract.C_xchng_cd
	cpcm.nse_contract.C_prd_typ = cpcm.contract.C_prd_typ
	cpcm.nse_contract.C_expry_dt = cpcm.contract.C_expry_dt
	cpcm.nse_contract.C_exrc_typ = cpcm.contract.C_exrc_typ
	cpcm.nse_contract.C_opt_typ = cpcm.contract.C_opt_typ
	cpcm.nse_contract.L_strike_prc = cpcm.contract.L_strike_prc
	cpcm.nse_contract.C_symbol = c_symbl
	cpcm.nse_contract.C_rqst_typ = cpcm.contract.C_rqst_typ
	cpcm.nse_contract.C_ctgry_indstk = cpcm.contract.C_ctgry_indstk

	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.C_xchng_cd is: %s", cpcm.ServiceName, cpcm.nse_contract.C_xchng_cd)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.C_prd_typ is: %s", cpcm.ServiceName, cpcm.nse_contract.C_prd_typ)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.C_expry_dt is: %s", cpcm.ServiceName, cpcm.nse_contract.C_expry_dt)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.C_exrc_typ is: %s", cpcm.ServiceName, cpcm.nse_contract.C_exrc_typ)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.C_opt_typ is: %s", cpcm.ServiceName, cpcm.nse_contract.C_opt_typ)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.L_strike_prc is: %d", cpcm.ServiceName, cpcm.nse_contract.L_strike_prc)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.C_symbol is: %s", cpcm.ServiceName, cpcm.nse_contract.C_symbol)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.C_ctgry_indstk is: %s", cpcm.ServiceName, cpcm.nse_contract.C_ctgry_indstk)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.C_rqst_typ is: %s", cpcm.ServiceName, cpcm.nse_contract.C_rqst_typ)

	if cpcm.contract.C_xchng_cd == "NFO" {
		c_exg_cd = "NSE"
	} else if cpcm.contract.C_xchng_cd == "BFO" {
		c_exg_cd = "BSE"
	} else {
		log.Printf("[%s] [fnGetExtCnt] Invalid option '%s' for 'cpcm.contract.C_xchng_cd'. Exiting 'fnGetExtCnt'", cpcm.ServiceName, cpcm.contract.C_xchng_cd)
		return -1
	}

	query2 := `
		SELECT COALESCE(ESS_XCHNG_SUB_SERIES, ' ')
		FROM ESS_SGMNT_STCK
		WHERE ess_stck_cd = ?
		AND ess_xchng_cd = ?;
	`

	log.Printf("[%s] [fnGetExtCnt] Executing query to fetch 'C_series'", cpcm.ServiceName)

	row2 := db.Raw(query2, cpcm.contract.C_undrlyng, c_exg_cd).Row()

	err2 := row2.Scan(&cpcm.nse_contract.C_series)

	if err2 != nil {
		log.Printf("[%s] [fnGetExtCnt] Error scanning row: %v", cpcm.ServiceName, err2)
		log.Printf("[%s] [fnGetExtCnt] Exiting 'fnGetExtCnt' due to error", cpcm.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnGetExtCnt] Value successfully fetched from the table 'ESS_SGMNT_STCK'", cpcm.ServiceName)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.C_series is: %s", cpcm.ServiceName, cpcm.nse_contract.C_series)

	query3 := `
		SELECT COALESCE(ftq_token_no, 0),
		       COALESCE(ftq_ca_lvl, 0)
		FROM ftq_fo_trd_qt
		WHERE ftq_xchng_cd = ?
		  AND ftq_prdct_typ = ?
		  AND ftq_undrlyng = ?
		  AND ftq_expry_dt = TO_DATE(?, 'dd-Mon-yyyy')
		  AND ftq_exer_typ = ?
		  AND ftq_opt_typ = ?
		  AND ftq_strk_prc = ?;
	`

	log.Printf("[%s] [fnGetExtCnt] Executing query to fetch 'L_token_id' and 'L_ca_lvl'", cpcm.ServiceName)

	log.Printf("[%s] [fnGetExtCnt] C_xchng_cd: %s", cpcm.ServiceName, cpcm.contract.C_xchng_cd)
	log.Printf("[%s] [fnGetExtCnt] C_prd_typ: %s", cpcm.ServiceName, cpcm.contract.C_prd_typ)
	log.Printf("[%s] [fnGetExtCnt] C_undrlyng: %s", cpcm.ServiceName, cpcm.contract.C_undrlyng)
	log.Printf("[%s] [fnGetExtCnt] C_expry_dt: %s", cpcm.ServiceName, cpcm.contract.C_expry_dt)
	log.Printf("[%s] [fnGetExtCnt] C_exrc_typ: %s", cpcm.ServiceName, cpcm.contract.C_exrc_typ)
	log.Printf("[%s] [fnGetExtCnt] C_opt_typ: %s", cpcm.ServiceName, cpcm.contract.C_opt_typ)
	log.Printf("[%s] [fnGetExtCnt] L_strike_prc: %d", cpcm.ServiceName, cpcm.contract.L_strike_prc)

	parsedDate, err := time.Parse(time.RFC3339, cpcm.contract.C_expry_dt)
	if err != nil {
		log.Printf("[%s] [fnGetExtCnt] Error parsing date: %v", cpcm.ServiceName, err)
		log.Printf("[%s] [fnGetExtCnt] Exiting 'fnGetExtCnt' due to error", cpcm.ServiceName)
		return -1
	}

	// Format the parsed date as 'dd-Mon-yyyy'
	formattedDate := parsedDate.Format("02-Jan-2006")

	row3 := db.Raw(query3,
		cpcm.contract.C_xchng_cd,
		cpcm.contract.C_prd_typ,
		cpcm.contract.C_undrlyng,
		formattedDate,
		cpcm.contract.C_exrc_typ,
		cpcm.contract.C_opt_typ,
		cpcm.contract.L_strike_prc,
	).Row()

	err3 := row3.Scan(&cpcm.nse_contract.L_token_id, &cpcm.nse_contract.L_ca_lvl)

	if err3 != nil {
		log.Printf("[%s] [fnGetExtCnt] Error scanning row: %v", cpcm.ServiceName, err3)
		log.Printf("[%s] [fnGetExtCnt] Exiting 'fnGetExtCnt' due to error", cpcm.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnGetExtCnt] Values successfully fetched from the table 'ftq_fo_trd_qt'", cpcm.ServiceName)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.L_token_id is: %d", cpcm.ServiceName, cpcm.nse_contract.L_token_id)
	log.Printf("[%s] [fnGetExtCnt] cpcm.nse_contract.L_ca_lvl is: %d", cpcm.ServiceName, cpcm.nse_contract.L_ca_lvl)

	return 0
}

func (cpcm *ClnPackClntManager) fnRjctRcrd(db *gorm.DB) int {

	var resultTmp int
	var c_tm_stmp string

	log.Printf("[%s] [fnRjctRcrd] Function 'fnRjctRcrd' starts", cpcm.ServiceName)

	/*
		// log.Printf("[%s] [fnRjctRcrd] Product type is : %s", cpcm.ServiceName, cpcm.orderbook.C_prd_typ)

		// if cpcm.orderbook.C_prd_typ == models.FUTURES {
		// 	svc = "SFO_FUT_ACK"
		// } else {
		// 	svc = "SFO_OPT_ACK"
		// }
	*/

	query := `SELECT TO_CHAR(NOW(), 'DD-Mon-YYYY HH24:MI:SS') AS c_tm_stmp`

	row := db.Raw(query).Row()
	err := row.Scan(&c_tm_stmp)

	if err != nil {
		log.Printf("[%s] [fnRjctRcrd] Error getting the system time: %v", cpcm.ServiceName, err)
		return -1
	}

	c_tm_stmp = strings.TrimSpace(c_tm_stmp)

	log.Printf("[%s] [fnRjctRcrd] Current timestamp: %s", cpcm.ServiceName, c_tm_stmp)

	cpcm.Xchngbook.C_plcd_stts = "REJECT"
	cpcm.Xchngbook.C_rms_prcsd_flg = "NOT_PROCESSED"
	cpcm.Xchngbook.L_ors_msg_typ = models.ORS_NEW_ORD_RJCT
	cpcm.Xchngbook.C_ack_tm = c_tm_stmp
	cpcm.Xchngbook.C_xchng_rmrks = "Token id not available"
	cpcm.Xchngbook.D_jiffy = 0
	cpcm.Xchngbook.C_oprn_typ = "UPDATION_ON_EXCHANGE_RESPONSE"

	log.Printf("[%s] [fnRjctRcrd] Before calling 'fnUpdXchngbk' on 'Reject Record' ", cpcm.ServiceName)

	resultTmp = cpcm.fnUpdXchngbk(db)

	if resultTmp != 0 {
		log.Printf("[%s] [fnRjctRcrd] returned from 'fnUpdXchngbk' with an Error", cpcm.ServiceName)
		log.Printf("[%s] [fnRjctRcrd] exiting from 'fnRjctRcrd'", cpcm.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnRjctRcrd] Time Before calling 'fnUpdXchngbk' : %s ", cpcm.ServiceName, c_tm_stmp)
	log.Printf("[%s] [fnRjctRcrd] Function 'fnRjctRcrd' ends successfully", cpcm.ServiceName)
	return 0
}
