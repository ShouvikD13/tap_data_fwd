package app

import (
	"DATA_FWD_TAP/models"
	"DATA_FWD_TAP/models/structures"
	"log"
	"strings"

	"gorm.io/gorm"
)

type ClnPackClntManager struct {
	serviceName  string                    // Name of the service being managed
	xchngbook    *structures.Vw_xchngbook  // Pointer to the Vw_xchngbook structure
	orderbook    *structures.Vw_orderbook  // Pointer to the Vw_orderbook structure
	contract     *structures.Vw_contract   // we are updating it's value from orderbook
	nse_contract *structures.Vw_nse_cntrct //we are updating it in 'fn_get_ext_cnt'
	cPanNo       string                    // Pan number, initialized in the 'fnRefToOrd' method
	cLstActRef   string                    // Last activity reference, initialized in the 'fnRefToOrd' method
	cEspID       string                    // ESP ID, initialized in the 'fnRefToOrd' method
	cAlgoID      string                    // Algorithm ID, initialized in the 'fnRefToOrd' method
	cSourceFlg   string                    // Source flag, initialized in the 'fnRefToOrd' method
}

func (cpcm *ClnPackClntManager) Fn_bat_init(args []string, Db *gorm.DB) int {
	var temp_str string
	var resultTmp int
	cpcm.serviceName = "cln_pack_clnt"
	cpcm.xchngbook = &structures.Vw_xchngbook{}
	cpcm.orderbook = &structures.Vw_orderbook{}
	log.Printf("[%s] Entering Fn_bat_init", cpcm.serviceName)

	// we are getting the 7 args
	if len(args) < 7 {
		log.Printf("[%s] Error: insufficient arguments provided", cpcm.serviceName)
		return -1
	}

	cpcm.xchngbook.C_pipe_id = args[3]

	log.Printf("[%s] Copied pipe ID from args[3]: %s", cpcm.serviceName, cpcm.xchngbook.C_pipe_id)

	// Fetch exchange code
	queryForOpm_Xchng_Cd := `SELECT opm_xchng_cd
				FROM opm_ord_pipe_mstr
				WHERE opm_pipe_id = ?`

	log.Printf("[%s] Executing query to fetch exchange code with pipe ID: %s", cpcm.serviceName, cpcm.xchngbook.C_pipe_id)

	row := Db.Raw(queryForOpm_Xchng_Cd, cpcm.xchngbook.C_pipe_id).Row()
	temp_str = ""
	err := row.Scan(&temp_str)
	if err != nil {
		log.Printf("[%s] Error scanning row for exchange code: %v", cpcm.xchngbook.C_xchng_cd, err)
		return -1
	}

	cpcm.xchngbook.C_xchng_cd = temp_str
	temp_str = ""
	log.Printf("[%s] Exchange code fetched: %s", cpcm.serviceName, cpcm.xchngbook.C_xchng_cd)

	// Fetch modification trade date
	queryFor_exg_nxt_trd_dt := `SELECT exg_nxt_trd_dt
				FROM exg_xchng_mstr
				WHERE exg_xchng_cd = ?`

	log.Printf("[%s] Executing query to fetch modification trade date with exchange code: %s", cpcm.serviceName, cpcm.xchngbook.C_xchng_cd)
	row2 := Db.Raw(queryFor_exg_nxt_trd_dt, cpcm.xchngbook.C_xchng_cd).Row()

	err2 := row2.Scan(&temp_str)
	if err2 != nil {
		log.Printf("[%s] Error scanning row for modification trade date: %v", cpcm.serviceName, err2)
		return -1
	}

	// TmpByteArr := []byte(temp_str) // these i have to look again
	// TmpArrLen := len(TmpByteArr)   // this i have to look again

	cpcm.xchngbook.C_mod_trd_dt = temp_str

	cpcm.serviceName = cpcm.xchngbook.C_mod_trd_dt

	log.Printf("[%s] Modification trade date fetched and set in C_mod_trd_dt: %s", cpcm.serviceName, cpcm.xchngbook.C_mod_trd_dt)

	cpcm.xchngbook.L_ord_seq = 0 // I am initially setting it to '0' because it was set that way in 'fn_bat_init' and I have not seen it getting changed anywhere. If I find it being changed somewhere, I will update it accordingly.

	log.Printf("[%s] L_ord_seq initialized to %d", cpcm.serviceName, cpcm.xchngbook.L_ord_seq)

	resultTmp = cpcm.CLN_PACK_CLNT(args, Db)
	if resultTmp != 0 {
		log.Printf("[%s] CLN_PACK_CLNT failed with result code: %d", cpcm.serviceName, resultTmp)
		log.Printf("[%s] Returning to main from fn_bat_init", cpcm.serviceName)
		return -1
	}
	log.Printf("[%s] Exiting Fn_bat_init", cpcm.serviceName)
	return 0
}

func (cpcm *ClnPackClntManager) CLN_PACK_CLNT(args []string, Db *gorm.DB) int {
	var resultTmp int

	log.Printf("[%s] Entering CLN_PACK_CLNT", cpcm.serviceName)
	resultTmp = cpcm.fnGetNxtRec(Db)
	if resultTmp != 0 {
		log.Printf("[%s] failed in getting the next record returning with result code: %d", cpcm.serviceName, resultTmp)
		log.Printf("[%s] Exiting CLN_PACK_CLNT with error", cpcm.serviceName)
		return -1
	}
	log.Printf("[%s] Exiting CLN_PACK_CLNT", cpcm.serviceName)
	return 0
}

func (cpcm *ClnPackClntManager) fnGetNxtRec(Db *gorm.DB) int {
	log.Printf("[%s] Entering fnGetNxtRec", cpcm.serviceName)

	resultTmp := cpcm.fnSeqToOmd(Db)
	if resultTmp != 0 {
		log.Printf("[%s] Failed to fetch data into exchngbook structure", cpcm.serviceName)
		return -1
	}

	if cpcm.xchngbook == nil {
		log.Printf("[%s] Error: xchngbook is nil", cpcm.serviceName)
		return -1
	}

	if cpcm.xchngbook.C_rms_prcsd_flg == "P" {
		log.Printf("[%s] Order ref / Mod number = |%s| / |%d| Already Processed", cpcm.serviceName, cpcm.xchngbook.C_ordr_rfrnc, cpcm.xchngbook.L_mdfctn_cntr)

		log.Printf("[%s] Exiting from from 'fnGetNxtRec'", cpcm.serviceName)

		return 0
	}

	if cpcm.xchngbook.C_plcd_stts != models.REQUESTED {
		log.Printf("[%s] the Placed status is not requested . so we are we are not getting the record ", cpcm.serviceName)
		return -1
	}

	if cpcm.xchngbook.C_ex_ordr_type == models.ORDINARY_ORDER {

		log.Printf("[%s] Assigning C_ordr_rfrnc from exchngbook to orderbook", cpcm.serviceName)
		cpcm.orderbook.C_ordr_rfrnc = cpcm.xchngbook.C_ordr_rfrnc

		resultTmp = cpcm.fnRefToOrd(Db)
		if resultTmp != 0 {
			log.Printf("[%s] Failed to fetch data into orderbook structure", cpcm.serviceName)
			return -1
		}

		if cpcm.xchngbook.L_mdfctn_cntr != cpcm.orderbook.L_mdfctn_cntr {
			log.Printf("[%s] L_mdfctn_cntr of both xchngbook and order are not same", cpcm.serviceName)
			log.Printf("Exiting fnGetNxtRec")
			return -1

		}

		cpcm.xchngbook.C_plcd_stts = models.QUEUED
		cpcm.xchngbook.C_oprn_typ = models.UPDATION_ON_ORDER_FORWARDING

		resultTmp = cpcm.fnUpdXchngbk(Db)

		if resultTmp != 0 {
			log.Printf("[%s] failed to update the status in Xchngbook", cpcm.serviceName)
			return -1
		}

		cpcm.orderbook.C_plcd_stts = models.QUEUED
		cpcm.orderbook.C_oprn_typ = models.UPDATE_ORDER_STATUS

		resultTmp = cpcm.fnUpdOrdrbk(Db)

		if resultTmp != 0 {
			log.Printf("[%s] failed to update the status in Order book", cpcm.serviceName)
			return -1
		}

		// here we are setting the contract data from orderbook structure
		log.Printf("[%s] Setting contract data from orderbook structure", cpcm.serviceName)
		cpcm.contract.C_xchng_cd = cpcm.orderbook.C_xchng_cd
		cpcm.contract.C_prd_typ = cpcm.orderbook.C_prd_typ
		cpcm.contract.C_undrlyng = cpcm.orderbook.C_undrlyng
		cpcm.contract.C_expry_dt = cpcm.orderbook.C_expry_dt
		cpcm.contract.C_exrc_typ = cpcm.orderbook.C_exrc_typ
		cpcm.contract.C_opt_typ = cpcm.orderbook.C_opt_typ
		cpcm.contract.L_strike_prc = cpcm.orderbook.L_strike_prc
		cpcm.contract.C_ctgry_indstk = cpcm.orderbook.C_ctgry_indstk
		cpcm.contract.L_ca_lvl = cpcm.orderbook.L_ca_lvl

		log.Printf("[%s] contract.C_xchng_cd: |%s|", cpcm.serviceName, cpcm.contract.C_xchng_cd)
		log.Printf("[%s] contract.C_prd_typ: |%s|", cpcm.serviceName, cpcm.contract.C_prd_typ)
		log.Printf("[%s] contract.C_undrlyng: |%s|", cpcm.serviceName, cpcm.contract.C_undrlyng)
		log.Printf("[%s] contract.C_expry_dt: |%s|", cpcm.serviceName, cpcm.contract.C_expry_dt)
		log.Printf("[%s] contract.C_exrc_typ: |%s|", cpcm.serviceName, cpcm.contract.C_exrc_typ)
		log.Printf("[%s] contract.C_opt_typ: |%s|", cpcm.serviceName, cpcm.contract.C_opt_typ)
		log.Printf("[%s] contract.L_strike_prc: |%d|", cpcm.serviceName, cpcm.contract.L_strike_prc)
		log.Printf("[%s] contract.C_ctgry_indstk: |%s|", cpcm.serviceName, cpcm.contract.C_ctgry_indstk)
		log.Printf("[%s] contract.L_ca_lvl: |%d|", cpcm.serviceName, cpcm.contract.L_ca_lvl)

		if cpcm.xchngbook.C_xchng_cd == "NFO" {

			cpcm.contract.C_rqst_typ = models.CONTRACT_TO_NSE_ID

			resultTmp = cpcm.fnGetExtCnt(Db)

			if resultTmp != 0 {
				log.Printf("[%s] failed to load data in NSE CNT ", cpcm.serviceName)
			}
		}
	}
	log.Printf("[%s] Exiting fnGetNxtRec", cpcm.serviceName)
	return 0
}

func (cpcm *ClnPackClntManager) fnSeqToOmd(db *gorm.DB) int {
	log.Printf("[%s] Entering fnSeqToOmd", cpcm.serviceName)
	log.Printf("[%s] Before extracting the data from the 'fxb_ordr_rfrnc' and storing it in the 'xchngbook' structure", cpcm.serviceName)

	query := `
		SELECT fxb_ordr_rfrnc,
		fxb_lmt_mrkt_sl_flg,
		fxb_dsclsd_qty,
		fxb_ordr_tot_qty,
		fxb_lmt_rt,
		fxb_stp_lss_tgr,
		TO_CHAR(TO_DATE(fxb_ordr_valid_dt, 'YYYY-MM-DD'), 'DD-Mon-YYYY') AS valid_dt,
		CASE
			WHEN fxb_ordr_type = 'V' THEN 'T'
			ELSE fxb_ordr_type
		END AS ord_typ,
		fxb_rqst_typ,
		fxb_ordr_sqnc
	FROM FXB_FO_XCHNG_BOOK
	WHERE fxb_xchng_cd =?
	AND fxb_pipe_id = ?
	AND TO_DATE(fxb_mod_trd_dt, 'YYYY-MM-DD') = TO_DATE(?, 'YYYY-MM-DD')
	AND fxb_ordr_sqnc = (
		SELECT MIN(fxb_b.fxb_ordr_sqnc)
		FROM FXB_FO_XCHNG_BOOK fxb_b
		WHERE fxb_b.fxb_xchng_cd = ?
			AND TO_DATE(fxb_b.fxb_mod_trd_dt, 'YYYY-MM-DD') = TO_DATE(?, 'YYYY-MM-DD')
			AND fxb_b.fxb_pipe_id = ?
        AND fxb_b.fxb_plcd_stts = 'R'
  	)
	`

	log.Printf("[%s] Executing query to fetch order details", cpcm.serviceName)

	log.Printf("[%s] C_xchng_cd: %s", cpcm.serviceName, cpcm.xchngbook.C_xchng_cd)
	log.Printf("[%s] C_pipe_id: %s", cpcm.serviceName, cpcm.xchngbook.C_pipe_id)
	log.Printf("[%s] C_mod_trd_dt: %s", cpcm.serviceName, cpcm.xchngbook.C_mod_trd_dt)

	row := db.Raw(query,
		cpcm.xchngbook.C_xchng_cd,
		cpcm.xchngbook.C_pipe_id,
		cpcm.xchngbook.C_mod_trd_dt,
		cpcm.xchngbook.C_xchng_cd,
		cpcm.xchngbook.C_mod_trd_dt,
		cpcm.xchngbook.C_pipe_id).Row()

	err := row.Scan(
		&cpcm.xchngbook.C_ordr_rfrnc,
		&cpcm.xchngbook.C_slm_flg,
		&cpcm.xchngbook.L_dsclsd_qty,
		&cpcm.xchngbook.L_ord_tot_qty,
		&cpcm.xchngbook.L_ord_lmt_rt,
		&cpcm.xchngbook.L_stp_lss_tgr,
		&cpcm.xchngbook.C_valid_dt,
		&cpcm.xchngbook.C_ord_typ,
		&cpcm.xchngbook.C_req_typ,
		&cpcm.xchngbook.L_ord_seq,
	)

	if err != nil {
		log.Printf("[%s] Error scanning row: %v", cpcm.serviceName, err)
		log.Printf("[%s] Exiting fnSeqToOmd with error", cpcm.serviceName)
		return -1
	}

	log.Printf("[%s] Data extracted and stored in the 'xchngbook' structure:", cpcm.serviceName)
	log.Printf("[%s]   C_ordr_rfrnc: 	%s", cpcm.serviceName, cpcm.xchngbook.C_ordr_rfrnc)
	log.Printf("[%s]   C_slm_flg: 		%s", cpcm.serviceName, cpcm.xchngbook.C_slm_flg)
	log.Printf("[%s]   L_dsclsd_qty: 	%d", cpcm.serviceName, cpcm.xchngbook.L_dsclsd_qty)
	log.Printf("[%s]   L_ord_tot_qty: 	%d", cpcm.serviceName, cpcm.xchngbook.L_ord_tot_qty)
	log.Printf("[%s]   L_ord_lmt_rt: 	%d", cpcm.serviceName, cpcm.xchngbook.L_ord_lmt_rt)
	log.Printf("[%s]   L_stp_lss_tgr: 	%d", cpcm.serviceName, cpcm.xchngbook.L_stp_lss_tgr)
	log.Printf("[%s]   C_valid_dt: 		%s", cpcm.serviceName, cpcm.xchngbook.C_valid_dt)
	log.Printf("[%s]   C_ord_typ: 		%s", cpcm.serviceName, cpcm.xchngbook.C_ord_typ)
	log.Printf("[%s]   C_req_typ: 		%s", cpcm.serviceName, cpcm.xchngbook.C_req_typ)
	log.Printf("[%s]   L_ord_seq: 		%d", cpcm.serviceName, cpcm.xchngbook.L_ord_seq)

	log.Printf("[%s] Data extracted and stored in the 'xchngbook' structure successfully", cpcm.serviceName)

	log.Printf("[%s] Exiting fnSeqToOmd", cpcm.serviceName)
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

	log.Printf("[%s] Entering fnFetchOrderDetails", cpcm.serviceName)
	log.Printf("[%s] Before extracting the data from the 'fod_fo_ordr_dtls' and storing it in the 'orderbook' structure", cpcm.serviceName)

	query := `
    SELECT
    fod_clm_mtch_accnt,
    fod_ordr_flw,
    fod_ordr_tot_qty,
    fod_exec_qty,
    COALESCE(fod_exec_qty_day, 0),
    fod_settlor,
    fod_spl_flag,
    TO_CHAR(fod_ord_ack_tm, 'YYYY-MM-DD HH24:MI:SS') AS fod_ord_ack_tm,
    TO_CHAR(fod_lst_rqst_ack_tm, 'YYYY-MM-DD HH24:MI:SS') AS fod_lst_rqst_ack_tm,
    fod_pro_cli_ind,
    COALESCE(fod_ctcl_id, ' '),
    COALESCE(fod_pan_no, '*'),
    COALESCE(fod_lst_act_ref, '0'),
    COALESCE(FOD_ESP_ID, '*'),
    COALESCE(FOD_ALGO_ID, '*'),
		COALESCE(FOD_SOURCE_FLG, '*')
	FROM
		fod_fo_ordr_dtls
	WHERE
		fod_ordr_rfrnc = ?
	FOR UPDATE NOWAIT;

    `

	log.Printf("[%s] Executing query to fetch order details", cpcm.serviceName)

	log.Printf("[%s] Order Reference: %s", cpcm.serviceName, cpcm.orderbook.C_ordr_rfrnc)

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
	)

	if err != nil {
		log.Printf("[%s] Error scanning row: %v", cpcm.serviceName, err)
		log.Printf("[%s] Exiting fnFetchOrderDetails with error", cpcm.serviceName)
		return -1
	}

	cpcm.cPanNo = strings.TrimSpace(cpcm.cPanNo)
	cpcm.cLstActRef = strings.TrimSpace(cpcm.cLstActRef)
	cpcm.cEspID = strings.TrimSpace(cpcm.cEspID)
	cpcm.cAlgoID = strings.TrimSpace(cpcm.cAlgoID)
	cpcm.cSourceFlg = strings.TrimSpace(cpcm.cSourceFlg)

	log.Printf("[%s] Data extracted and stored in the 'orderbook' structure:", cpcm.serviceName)
	log.Printf("[%s]   C_cln_mtch_accnt:   %s", cpcm.serviceName, cpcm.orderbook.C_cln_mtch_accnt)
	log.Printf("[%s]   C_ordr_flw:        %s", cpcm.serviceName, cpcm.orderbook.C_ordr_flw)
	log.Printf("[%s]   L_ord_tot_qty:     %d", cpcm.serviceName, cpcm.orderbook.L_ord_tot_qty)
	log.Printf("[%s]   L_exctd_qty:       %d", cpcm.serviceName, cpcm.orderbook.L_exctd_qty)
	log.Printf("[%s]   L_exctd_qty_day:   %d", cpcm.serviceName, cpcm.orderbook.L_exctd_qty_day)
	log.Printf("[%s]   C_settlor:         %s", cpcm.serviceName, cpcm.orderbook.C_settlor)
	log.Printf("[%s]   C_spl_flg:         %s", cpcm.serviceName, cpcm.orderbook.C_spl_flg)
	log.Printf("[%s]   C_ack_tm:          %s", cpcm.serviceName, cpcm.orderbook.C_ack_tm)
	log.Printf("[%s]   C_prev_ack_tm:     %s", cpcm.serviceName, cpcm.orderbook.C_prev_ack_tm)
	log.Printf("[%s]   C_pro_cli_ind:     %s", cpcm.serviceName, cpcm.orderbook.C_pro_cli_ind)
	log.Printf("[%s]   C_ctcl_id:         %s", cpcm.serviceName, cpcm.orderbook.C_ctcl_id)
	log.Printf("[%s]   C_pan_no:          %s", cpcm.serviceName, cpcm.cPanNo)
	log.Printf("[%s]   C_lst_act_ref_tmp: %s", cpcm.serviceName, cpcm.cLstActRef)
	log.Printf("[%s]   C_esp_id:          %s", cpcm.serviceName, cpcm.cEspID)
	log.Printf("[%s]   C_algo_id:         %s", cpcm.serviceName, cpcm.cAlgoID)
	log.Printf("[%s]   C_source_flg_tmp:  %s", cpcm.serviceName, cpcm.cSourceFlg)

	log.Printf("[%s] Data extracted and stored in the 'orderbook' structure successfully", cpcm.serviceName)

	clmQuery := `
    SELECT
    COALESCE(TRIM(CLM_CP_CD), ' '),
    COALESCE(TRIM(CLM_CLNT_CD), CLM_MTCH_ACCNT)
	FROM
    CLM_CLNT_MSTR
	WHERE
    CLM_MTCH_ACCNT = ?
    `
	log.Printf("[%s] Executing query to fetch data from 'CLM_CLNT_MSTR' ", cpcm.serviceName)

	row = db.Raw(clmQuery, strings.TrimSpace(cpcm.orderbook.C_cln_mtch_accnt)).Row()

	var cCpCode, cUccCd string
	err = row.Scan(&cCpCode, &cUccCd)

	if err != nil {
		log.Printf("[%s] Error scanning client details: %v", cpcm.serviceName, err)
		log.Printf("[%s] Exiting fnRefToOrd with error", cpcm.serviceName)
		return -1
	}

	log.Printf("[%s] cCpCode : %s", cpcm.serviceName, cCpCode)
	log.Printf("[%s] cUccCd : %s", cpcm.serviceName, cUccCd)

	log.Printf("[%s] Updating orderbook with client details", cpcm.serviceName)

	cpcm.orderbook.C_cln_mtch_accnt = strings.TrimSpace(cUccCd)
	cpcm.orderbook.C_settlor = strings.TrimSpace(cCpCode)
	cpcm.orderbook.C_ctcl_id = strings.TrimSpace(cpcm.orderbook.C_ctcl_id)

	log.Printf("[%s] Exiting fnFetchOrderDetails", cpcm.serviceName)
	return 0
}

func (cpcm *ClnPackClntManager) fnUpdXchngbk(db *gorm.DB) int {

	var iRecExists int64

	log.Printf("[%s] Entering fnUpdXchngbk", cpcm.serviceName)
	log.Printf("[%s]: %s", cpcm.serviceName, cpcm.xchngbook.C_xchng_cd)
	log.Printf("[%s]: %s", cpcm.serviceName, cpcm.xchngbook.C_ordr_rfrnc)
	log.Printf("[%s]: %s", cpcm.serviceName, cpcm.xchngbook.C_pipe_id)
	log.Printf("[%s]: %s", cpcm.serviceName, cpcm.xchngbook.C_mod_trd_dt)
	log.Printf("[%s]: %d", cpcm.serviceName, cpcm.xchngbook.L_ord_seq)
	log.Printf("[%s]: %s", cpcm.serviceName, cpcm.xchngbook.C_slm_flg)
	log.Printf("[%s]: %d", cpcm.serviceName, cpcm.xchngbook.L_dsclsd_qty)
	log.Printf("[%s]: %d", cpcm.serviceName, cpcm.xchngbook.L_ord_tot_qty)
	log.Printf("[%s]: %d", cpcm.serviceName, cpcm.xchngbook.L_ord_lmt_rt)
	log.Printf("[%s]: %d", cpcm.serviceName, cpcm.xchngbook.L_stp_lss_tgr)
	log.Printf("[%s]: %d", cpcm.serviceName, cpcm.xchngbook.L_mdfctn_cntr)
	log.Printf("[%s]: %s", cpcm.serviceName, cpcm.xchngbook.C_valid_dt)
	log.Printf("[%s]: %s", cpcm.serviceName, cpcm.xchngbook.C_ord_typ)
	log.Printf("[%s]: %s", cpcm.serviceName, cpcm.xchngbook.C_req_typ)
	log.Printf("[%s]: %s", cpcm.serviceName, cpcm.xchngbook.C_plcd_stts)

	switch cpcm.xchngbook.C_oprn_typ {
	case models.UPDATION_ON_ORDER_FORWARDING:

		query1 := `UPDATE fxb_fo_xchng_book 
				SET fxb_plcd_stts = ?, fxb_frwd_tm = SYSDATE
				WHERE fxb_ordr_rfrnc = ? 
				AND fxb_mdfctn_cntr = ?`

		result := db.Exec(query1, cpcm.xchngbook.C_plcd_stts, cpcm.xchngbook.C_ordr_rfrnc, cpcm.xchngbook.L_mdfctn_cntr)
		if result.Error != nil {
			log.Printf("[%s] Error updating order forwarding: %v", cpcm.serviceName, result.Error)
			return -1
		}

	case models.UPDATION_ON_EXCHANGE_RESPONSE:
		if cpcm.xchngbook.L_dwnld_flg == models.DOWNLOAD {

			result := db.Raw(
				`SELECT COUNT(*) 
					FROM fxb_fo_xchng_book 
					WHERE fxb_jiffy = ? 
					AND fxb_xchng_cd = ? 
					AND fxb_pipe_id = ?`,
				cpcm.xchngbook.D_jiffy, cpcm.xchngbook.C_xchng_cd, cpcm.xchngbook.C_pipe_id,
			).Scan(&iRecExists)
			if result.Error != nil {
				log.Printf("[%s] SQL error: %v", cpcm.serviceName, result.Error)
				return -1
			}

			if iRecExists > 0 {
				log.Printf("[%s] Record already processed", cpcm.serviceName)
				return -1
			}
		}

		c_xchng_rmrks := strings.TrimSpace(cpcm.xchngbook.C_xchng_rmrks)

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
			cpcm.xchngbook.C_plcd_stts,
			cpcm.xchngbook.C_rms_prcsd_flg,
			cpcm.xchngbook.L_ors_msg_typ,
			cpcm.xchngbook.C_ack_tm,
			c_xchng_rmrks,
			cpcm.xchngbook.D_jiffy,
			cpcm.xchngbook.C_ordr_rfrnc,
			cpcm.xchngbook.L_mdfctn_cntr,
		)
		if result.Error != nil {
			log.Printf("[%s] Error updating exchange response: %v", cpcm.serviceName, result.Error)
			return -1
		}

	default:
		log.Printf("[%s] Invalid Operation Type", cpcm.serviceName)
		return -1
	}

	log.Printf("[%s] Exiting fnUpdXchngbk", cpcm.serviceName)

	return 0
}

func (cpcm *ClnPackClntManager) fnUpdOrdrbk(db *gorm.DB) int {
	return 0
}

func (cpcm *ClnPackClntManager) fnGetExtCnt(db *gorm.DB) int {
	var i_sem_entity int
	var c_stck_cd, c_symbl string

	if cpcm.contract.C_xchng_cd == "NFO" {
		i_sem_entity = models.NFO_ENTTY
	} else {
		i_sem_entity = models.BFO_ENTTY
	}

	c_stck_cd = cpcm.contract.C_undrlyng

	c_stck_cd = strings.ToUpper(c_stck_cd)

	// log

	query := `
			SELECT TRIM(sem_map_vl)
			INTO ?
			FROM sem_stck_map
			WHERE sem_entty = ?
			AND sem_stck_cd = ?;
			`
	db.Raw(query, &c_symbl, i_sem_entity, c_stck_cd)

	// log

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

	return 0
}
