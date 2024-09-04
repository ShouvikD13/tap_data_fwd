package app

import (
	"DATA_FWD_TAP/models"
	"DATA_FWD_TAP/models/structures"
	"log"
	"reflect"
	"strconv"
	"strings"
	"time"
	"unsafe"

	"gorm.io/gorm"
)

const defaultTimeStamp = "0"

type ExchngPackLibMaster struct {
	serviceName   string
	xchngbook     *structures.Vw_xchngbook
	orderbook     *structures.Vw_orderbook
	pipe_mstr     *structures.St_opm_pipe_mstr
	nse_contract  *structures.Vw_nse_cntrct
	oe_reqres     *structures.St_oe_reqres
	exch_msg      *structures.St_exch_msg
	net_hdr       *structures.St_net_hdr
	q_packet      *structures.St_req_q_data
	int_header    *structures.St_int_header
	contract_desc *structures.St_contract_desc
	order_flag    *structures.St_order_flags
	OCM           *models.OrderConversionManager
	cPanNo        [models.LEN_PAN]byte
	cLstActRef    [22]byte
	cEspID        [51]byte
	cAlgoID       [51]byte
	cSourceFlg    byte
	cPrgmFlg      byte
	cUserTypGlb   byte
}

// constructor function
func NewExchngPackLibMaster(serviceName string,
	xchngbook *structures.Vw_xchngbook,
	orderbook *structures.Vw_orderbook,
	pipe_mstr *structures.St_opm_pipe_mstr,
	nseContract *structures.Vw_nse_cntrct,
	oe_req *structures.St_oe_reqres,
	exch_msg *structures.St_exch_msg,
	net_hdr *structures.St_net_hdr,
	q_packet *structures.St_req_q_data,
	int_header *structures.St_int_header,
	contract_desc *structures.St_contract_desc,
	order_flag *structures.St_order_flags,
	OCM *models.OrderConversionManager,
	cPanNo, cLstActRef, cEspID, cAlgoID, cSourceFlg, cPrgmFlg string) *ExchngPackLibMaster {

	var (
		bPanNo     [10]byte
		bLstActRef [22]byte
		bEspID     [51]byte
		bAlgoID    [51]byte
	)

	copy(bPanNo[:], []byte(cPanNo))
	copy(bLstActRef[:], []byte(cLstActRef))
	copy(bEspID[:], []byte(cEspID))
	copy(bAlgoID[:], []byte(cAlgoID))
	bSourceFlg := cSourceFlg[0]
	bPrgmFlg := cPrgmFlg[0]

	return &ExchngPackLibMaster{
		serviceName:   serviceName,
		xchngbook:     xchngbook,
		orderbook:     orderbook,
		pipe_mstr:     pipe_mstr,
		nse_contract:  nseContract,
		oe_reqres:     oe_req,
		exch_msg:      exch_msg,
		net_hdr:       net_hdr,
		q_packet:      q_packet,
		int_header:    int_header,
		contract_desc: contract_desc,
		order_flag:    order_flag,
		OCM:           OCM,
		cPanNo:        bPanNo,
		cLstActRef:    bLstActRef,
		cEspID:        bEspID,
		cAlgoID:       bAlgoID,
		cSourceFlg:    bSourceFlg,
		cPrgmFlg:      bPrgmFlg,
	}
}

// There is an issue with the field `c_remark`. it is not being initialized anywhere in the original code.
// In the NNF, `c_remark` is mentioned as 'cOrdFiller CHAR 24', but there is no description provided for it.

func (eplm *ExchngPackLibMaster) fnPackOrdnryOrdToNse(db *gorm.DB) int {

	log.Printf("[%s] [fnPackOrdnryOrdToNse] Inside 'fnPackOrdnryOrdToNse' ", eplm.serviceName)

	trimmedEspID := strings.TrimSpace(string(eplm.cEspID[:]))
	copy(eplm.cEspID[:], trimmedEspID)
	for i := len(trimmedEspID); i < len(eplm.cEspID); i++ {
		eplm.cEspID[i] = 0 // null in golnag
	}

	trimmedAlgoID := strings.TrimSpace(string(eplm.cAlgoID[:]))
	copy(eplm.cAlgoID[:], trimmedAlgoID)
	for i := len(trimmedAlgoID); i < len(eplm.cAlgoID); i++ {
		eplm.cAlgoID[i] = 0
	}

	log.Printf("[%s] [fnPackOrdnryOrdToNse] Trimmed cEspID: '%s'", eplm.serviceName, eplm.cEspID)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] Trimmed cAlgoID: '%s'", eplm.serviceName, eplm.cAlgoID)

	if eplm.orderbook != nil {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book data...", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book c_pro_cli_ind received: '%s'", eplm.serviceName, eplm.orderbook.C_pro_cli_ind)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book c_cln_mtch_accnt received: '%s'", eplm.serviceName, eplm.orderbook.C_cln_mtch_accnt)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book c_ordr_flw received: '%s'", eplm.serviceName, eplm.orderbook.C_ordr_flw)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book l_ord_tot_qty received: '%d'", eplm.serviceName, eplm.orderbook.L_ord_tot_qty)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book l_exctd_qty received: '%d'", eplm.serviceName, eplm.orderbook.L_exctd_qty)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book l_exctd_qty_day received: '%d'", eplm.serviceName, eplm.orderbook.L_exctd_qty_day)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book c_settlor received: '%s'", eplm.serviceName, eplm.orderbook.C_settlor)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book c_ctcl_id received: '%s'", eplm.serviceName, eplm.orderbook.C_ctcl_id)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book c_ack_tm received: '%s'", eplm.serviceName, eplm.orderbook.C_ack_tm)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Order book c_prev_ack_tm received: '%s'", eplm.serviceName, eplm.orderbook.C_prev_ack_tm)
	} else {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] No data received in orderbook in 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exiting from 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		return -1
	}

	if eplm.xchngbook != nil {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange book data...", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange book c_req_typ received: '%s'", eplm.serviceName, eplm.xchngbook.C_req_typ)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange book l_dsclsd_qty received: '%d'", eplm.serviceName, eplm.xchngbook.L_dsclsd_qty)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange book c_slm_flg received: '%s'", eplm.serviceName, eplm.xchngbook.C_slm_flg)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange book l_ord_lmt_rt received: '%d'", eplm.serviceName, eplm.xchngbook.L_ord_lmt_rt)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange book l_stp_lss_tgr received: '%d'", eplm.serviceName, eplm.xchngbook.L_stp_lss_tgr)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange book c_ord_typ received: '%s'", eplm.serviceName, eplm.xchngbook.C_ord_typ)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange book l_ord_tot_qty received: '%d'", eplm.serviceName, eplm.xchngbook.L_ord_tot_qty)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange book c_valid_dt received: '%s'", eplm.serviceName, eplm.xchngbook.C_valid_dt)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange book l_ord_seq received: '%d'", eplm.serviceName, eplm.xchngbook.L_ord_seq)

	} else {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] No data received in xchngbook in 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exiting from 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		return -1
	}

	if eplm.pipe_mstr != nil {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] OPM data...", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] OPM li_opm_brnch_id received: '%d'", eplm.serviceName, eplm.pipe_mstr.L_opm_brnch_id)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] OPM c_opm_trdr_id received: '%s'", eplm.serviceName, eplm.pipe_mstr.C_opm_trdr_id)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] OPM c_xchng_brkr_id received: '%s'", eplm.serviceName, eplm.pipe_mstr.C_xchng_brkr_id)
	} else {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] No data received in 'OPM_PIPE_MSTR' Struct in 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exiting from 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		return -1
	}

	if eplm.nse_contract != nil {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exchange message structure data...", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] NSE CONTRACT c_prd_typ received: '%s'", eplm.serviceName, eplm.nse_contract.C_prd_typ)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] NSE CONTRACT c_ctgry_indstk received: '%s'", eplm.serviceName, eplm.nse_contract.C_ctgry_indstk)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] NSE CONTRACT c_symbol received: '%s'", eplm.serviceName, eplm.nse_contract.C_symbol)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] NSE CONTRACT l_ca_lvl received: '%d'", eplm.serviceName, eplm.nse_contract.L_ca_lvl)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] NSE CONTRACT l_token_id received: '%d'", eplm.serviceName, eplm.nse_contract.L_token_id)

	} else {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] No data received in 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exiting from 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		return -1
	}

	// Starting thr intialization of Header

	value, err := strconv.Atoi(strings.TrimSpace(eplm.pipe_mstr.C_opm_trdr_id))
	if err != nil {
		log.Printf("[%s] ERROR: Failed to convert Trader ID to int: %v", eplm.serviceName, err)
		// return -1
	}

	eplm.oe_reqres.St_hdr.Li_trader_id = int32(value)

	eplm.oe_reqres.St_hdr.Li_log_time = 0                                                  // 2 HDR
	copyAndFormatSymbol(eplm.oe_reqres.St_hdr.C_alpha_char[:], models.LEN_ALPHA_CHAR, " ") // 3 HDR 'orstonse'
	eplm.oe_reqres.St_hdr.Si_transaction_code = models.BOARD_LOT_IN                        // 4 HDR
	eplm.oe_reqres.St_hdr.C_filler_2 = ' '                                                 // 5 HDR

	eplm.oe_reqres.St_hdr.Si_error_code = 0 // 6 HDR
	/*
		for i := range eplm.oe_reqres.St_hdr.C_time_stamp_1 {
			eplm.oe_reqres.St_hdr.C_time_stamp_1[i] = 0 //7 HDR
		}
		for i := range eplm.oe_reqres.St_hdr.C_time_stamp_2 {
			eplm.oe_reqres.St_hdr.C_time_stamp_2[i] = 0 //8 HDR
	}*/

	copy(eplm.oe_reqres.St_hdr.C_time_stamp_1[:], defaultTimeStamp)                        //7 HDR
	copy(eplm.oe_reqres.St_hdr.C_time_stamp_2[:], defaultTimeStamp)                        //8 HDR
	eplm.oe_reqres.St_hdr.Si_message_length = int16(reflect.TypeOf(eplm.oe_reqres).Size()) //9 HDR

	// Header structure Done Till here

	switch eplm.pipe_mstr.S_user_typ_glb {
	case 0:
		eplm.cUserTypGlb = models.TRADER
	case 4:
		eplm.cUserTypGlb = models.CORPORATE_MANAGER
	case 5:
		eplm.cUserTypGlb = models.BRANCH_MANAGER
	default:
		log.Printf("[%s] [fnPackOrdnryOrdToNse] ERROR: Invalid user type, returning from 'fo_exg_pack_lib' with an error..", eplm.serviceName)
		return -1
	}

	eplm.oe_reqres.Ll_lastactivityref = 0                     //1 BDY
	eplm.oe_reqres.C_participant_type = 'C'                   //2 BDY
	eplm.oe_reqres.C_filler_1 = ' '                           //3 BDY
	eplm.oe_reqres.Si_competitor_period = 0                   //4 BDY
	eplm.oe_reqres.Si_solicitor_period = 0                    //5 BDY
	eplm.oe_reqres.C_modified_cancelled_by = eplm.cUserTypGlb //6 BDY
	eplm.oe_reqres.C_filler_2 = ' '                           //7 BDY
	eplm.oe_reqres.Si_reason_code = 0                         //8 BDY
	eplm.oe_reqres.C_filler_3 = [4]byte{' ', ' ', ' ', ' '}   //9 BDY
	eplm.oe_reqres.L_token_no = eplm.nse_contract.L_token_id  //10 BDY

	if eplm.fn_orstonse_cntrct_desc() != 0 { // 11 BDY
		log.Printf("[%s] [fnPackOrdnryOrdToNse] returned from 'fn_orstonse_cntrct_desc()' with error ", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exiting from 'fnPackOrdnryOrdToNse' ", eplm.serviceName)
		return -1
	}

	copyAndFormatSymbol(eplm.oe_reqres.C_counter_party_broker_id[:], models.LEN_BROKER_ID, " ") // 12 BDY
	eplm.oe_reqres.C_filler_4 = ' '                                                             // 13 BDY
	eplm.oe_reqres.C_filler_5 = [2]byte{' ', ' '}                                               // 14 BDY
	eplm.oe_reqres.C_closeout_flg = ' '                                                         // 15 BDY
	eplm.oe_reqres.C_filler_6 = ' '                                                             // 16 BDY
	eplm.oe_reqres.Si_order_type = 0                                                            // 17 BDY

	if eplm.xchngbook.C_req_typ == string(models.NEW) { // 18 BDY
		eplm.oe_reqres.D_order_number = 0
	} else {
		eplm.oe_reqres.D_order_number, err = strconv.ParseFloat(eplm.orderbook.C_xchng_ack, 64)
		if err != nil {
			log.Printf("[%s] [fnPackOrdnryOrdToNse] [ERROR: Error parsing order number: %v", eplm.serviceName, err)
			log.Printf("[%s] [fnPackOrdnryOrdToNse] Returning from 'fnPackOrdnryOrdToNse' with an error ", eplm.serviceName)
			return -1
		}
	}

	copyAndFormatSymbol(eplm.oe_reqres.C_account_number[:], len(eplm.oe_reqres.C_account_number), eplm.orderbook.C_cln_mtch_accnt) //19 BDY

	if eplm.xchngbook.C_slm_flg == "M" {
		eplm.oe_reqres.Si_book_type = models.REGULAR_LOT_ORDER //20 BDY
		eplm.oe_reqres.Li_price = 0                            //21 BDY
	} else {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] [ERROR: Invalid slm flag ...", eplm.serviceName)
		return -1
	}

	switch eplm.orderbook.C_ordr_flw { //22 BDY
	case "B":
		eplm.oe_reqres.Si_buy_sell_indicator = models.NSE_BUY
	case "S":
		eplm.oe_reqres.Si_buy_sell_indicator = models.NSE_SELL
	default:
		log.Printf("[%s] [fnPackOrdnryOrdToNse] [ERROR: Invalid order flow flag ...", eplm.serviceName)
		return -1
	}

	eplm.oe_reqres.Li_disclosed_volume = eplm.xchngbook.L_dsclsd_qty                     // 23 BDY
	eplm.oe_reqres.Li_disclosed_volume_remaining = eplm.oe_reqres.Li_disclosed_volume    //24 BDY
	eplm.oe_reqres.Li_volume = eplm.xchngbook.L_ord_tot_qty - eplm.orderbook.L_exctd_qty //25 BDY

	if eplm.xchngbook.L_ord_tot_qty > eplm.orderbook.L_ord_tot_qty {
		eplm.oe_reqres.Li_total_volume_remaining = eplm.orderbook.L_ord_tot_qty - eplm.orderbook.L_exctd_qty //26 BDY
	} else {
		eplm.oe_reqres.Li_total_volume_remaining = eplm.xchngbook.L_ord_tot_qty - eplm.orderbook.L_exctd_qty
	}

	eplm.oe_reqres.Li_volume_filled_today = eplm.orderbook.L_exctd_qty_day //27 BDY
	eplm.oe_reqres.Li_trigger_price = eplm.xchngbook.L_stp_lss_tgr         //28 BDY

	switch eplm.xchngbook.C_ord_typ { // 29 BDY
	case "T", "I":
		eplm.oe_reqres.Li_good_till_date = 0

	case "D":
		eplm.TimeArrToLong(eplm.xchngbook.C_valid_dt, &eplm.oe_reqres.Li_good_till_date)

	default:
		log.Printf(" [%s] [fnPackOrdnryOrdToNse] [ERROR: Invalid order type ...", eplm.serviceName)
		return -1
	}

	if eplm.xchngbook.C_req_typ == string(models.NEW) {
		eplm.oe_reqres.Li_entry_date_time = 0 // 30 BDY
		eplm.oe_reqres.Li_last_modified = 0   //31 BDY
	} else {
		log.Printf(" [%s] [fnPackOrdnryOrdToNse] [ERROR: Invalid Request type for setting 'Entry Date Time'  ...", eplm.serviceName)
		log.Printf(" [%s] [fnPackOrdnryOrdToNse] [ERROR: Invalid Request type for setting 'Last Modified'  ...", eplm.serviceName)
		return -1
	}

	eplm.oe_reqres.Li_minimum_fill_aon_volume = 0 // 32 BDY

	eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_ATO) // 1 FLG

	eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_Market) // 2 FLG

	if eplm.xchngbook.C_slm_flg == "M" { // 3 FLG
		eplm.oe_reqres.St_ord_flg.SetFlag(structures.Flg_SL) // Set flag
	} else {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] [ERROR: Invalid slm flag for setting 'Flg_SL'  ...", eplm.serviceName)
		return -1
	}
	eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_MIT) // 4 FLG

	if eplm.xchngbook.C_ord_typ == "T" {
		eplm.oe_reqres.St_ord_flg.SetFlag(structures.Flg_Day) // 5 FLG
	} else {
		eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_Day) // 6 FLG
	}

	eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_GTC) // 7 FLG

	if eplm.xchngbook.C_ord_typ == "I" {
		eplm.oe_reqres.St_ord_flg.SetFlag(structures.Flg_IOC) // 8 FLG
	} else {
		eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_IOC)
	}

	eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_AON)        // 9 FLG
	eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_MF)         // 10 FLG
	eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_MatchedInd) // 11 FLG
	eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_Traded)     // 12 FLG
	eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_Modified)   // 13 FLG
	eplm.oe_reqres.St_ord_flg.ClearFlag(structures.Flg_Frozen)     // 14 FLG

	// Clear bits 13, 14, and 15 in Flags using Flg_Filler1 bitmask.
	eplm.oe_reqres.St_ord_flg.Flags &^= structures.Flg_Filler1 // 15 FLG

	eplm.oe_reqres.Si_branch_id = int16(eplm.pipe_mstr.L_opm_brnch_id) //33 BDY

	value1, err := strconv.Atoi(eplm.pipe_mstr.C_opm_trdr_id)
	if err != nil {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] [Error converting C_opm_trdr_id to int:  ... %v", eplm.serviceName, err)
		return -1
	}
	eplm.oe_reqres.Li_trader_id = int32(value1) // 34 BDY

	copyAndFormatSymbol(eplm.oe_reqres.C_broker_id[:], len(eplm.oe_reqres.C_broker_id), eplm.pipe_mstr.C_xchng_brkr_id) //35 BDY

	eplm.oe_reqres.I_order_seq = eplm.xchngbook.L_ord_seq // 36 BDY
	eplm.oe_reqres.C_open_close = 'O'                     // 37 BDY

	for i := range eplm.oe_reqres.C_settlor {
		eplm.oe_reqres.C_settlor[i] = 0 // 38 BDY
	}

	query := `
		SELECT ICD_CUST_TYPE
		FROM ICD_INFO_CLIENT_DTLS
		JOIN IAI_INFO_ACCOUNT_INFO ON ICD_SERIAL_NO = IAI_SERIAL_NO
		WHERE IAI_MATCH_ACCOUNT_NO = ?
	`
	var icdCustType string
	result := db.Raw(query, eplm.orderbook.C_cln_mtch_accnt).Scan(&icdCustType)
	if result.Error != nil {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] [Error executing query: ... %v", eplm.serviceName, result.Error)
		return -1
	}

	log.Printf("[%s] [fnPackOrdnryOrdToNse] ICD_CUST_TYPE: %s", eplm.serviceName, icdCustType)
	if icdCustType != "NRI" {
		if eplm.orderbook.C_pro_cli_ind == string(models.BRKR_PLCD) {
			copyAndFormatSymbol(eplm.oe_reqres.C_settlor[:], models.LEN_SETTLOR, " ") // already set to zero now setting value
			eplm.oe_reqres.Si_pro_client_indicator = models.NSE_PRO                   // 39 BDY
		} else {
			copyAndFormatSymbol(eplm.oe_reqres.C_settlor[:], models.LEN_SETTLOR, eplm.pipe_mstr.C_xchng_brkr_id)
			eplm.oe_reqres.Si_pro_client_indicator = models.NSE_CLIENT
		}
	} else {
		copyAndFormatSymbol(eplm.oe_reqres.C_settlor[:], models.LEN_SETTLOR, eplm.orderbook.C_settlor)
		eplm.oe_reqres.Si_pro_client_indicator = models.NSE_CLIENT
	}

	switch eplm.cPrgmFlg {
	case 'A':
		eplm.oe_reqres.L_algo_id = models.FO_AUTO_MTM_ALG_ID            //40 BDY
		eplm.oe_reqres.Si_algo_category = models.FO_AUTO_MTM_ALG_CAT_ID //41 BDY

	case 'T':
		eplm.oe_reqres.L_algo_id = models.FO_PRICE_IMP_ALG_ID
		eplm.oe_reqres.Si_algo_category = models.FO_PRICE_IMP_ALG_CAT_ID

	case 'Z':
		eplm.oe_reqres.L_algo_id = models.FO_PRFT_ORD_ALG_ID
		eplm.oe_reqres.Si_algo_category = models.FO_PRFT_ORD_ALG_CAT_ID

	case 'G':
		eplm.oe_reqres.L_algo_id = models.FO_FLASH_TRD_ALG_ID
		eplm.oe_reqres.Si_algo_category = models.FO_FLASH_TRD_ALG_CAT_ID

	default:
		cAlgoIDStr1 := strings.TrimSpace(string(eplm.cAlgoID[:]))
		if cAlgoIDStr1 != "*" {
			algoID, err := strconv.Atoi(cAlgoIDStr1)
			if err != nil {
				log.Printf("[%s] [ProcessAlgorithmFlags] [Error converting cAlgoID ...  %v", eplm.cUserTypGlb, err)
				return -1
			}
			eplm.oe_reqres.L_algo_id = int32(algoID)
			eplm.oe_reqres.Si_algo_category = 0
		} else {
			eplm.oe_reqres.L_algo_id = models.FO_NON_ALG_ID
			eplm.oe_reqres.Si_algo_category = models.FO_NON_ALG_CAT_ID
		}
	}

	if eplm.cSourceFlg == 'G' {
		eplm.oe_reqres.L_algo_id = models.FO_FOGTT_ALG_ID
		eplm.oe_reqres.Si_algo_category = models.FO_FOGTT_ALG_CAT_ID
	}

	eplm.orderbook.C_ctcl_id = strings.TrimSpace(eplm.orderbook.C_ctcl_id)

	cAlgoIDStr := strings.TrimSpace(string(eplm.cAlgoID[:]))
	cSourceFlgStr := strings.TrimSpace(string(eplm.cSourceFlg))
	cEspIDStr := strings.TrimSpace(string(eplm.cEspID[:]))

	if eplm.cPrgmFlg == 'A' || eplm.cPrgmFlg == 'T' || eplm.cPrgmFlg == 'Z' || eplm.cPrgmFlg == 'G' || cSourceFlgStr == "G" {
		eplm.orderbook.C_ctcl_id = eplm.orderbook.C_ctcl_id + "000"
	} else if cAlgoIDStr != "*" && cSourceFlgStr == "M" {
		eplm.orderbook.C_ctcl_id = "333333333333" + "000"
	} else if cAlgoIDStr != "*" && (cSourceFlgStr == "W" || cSourceFlgStr == "E") {
		eplm.orderbook.C_ctcl_id = "111111111111" + "000"
	} else if cAlgoIDStr != "*" && cEspIDStr == "4124" {
		eplm.orderbook.C_ctcl_id = "333333333333" + "000"
	} else if cAlgoIDStr != "*" {
		eplm.orderbook.C_ctcl_id = "111111111111" + "000"
	} else {
		eplm.orderbook.C_ctcl_id = eplm.orderbook.C_ctcl_id + "100"
	}

	eplm.oe_reqres.D_nnf_field, err = strconv.ParseFloat(eplm.orderbook.C_ctcl_id, 64)
	if err != nil {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] [Error in converting the 'C_ctcl_id' in float ... ", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] Exiting from the function ", eplm.serviceName)
	}

	cPanNoStr := strings.TrimSpace(strings.ToUpper(string(eplm.cPanNo[:])))

	if cPanNoStr != "*" {
		copy(eplm.oe_reqres.C_pan[:], cPanNoStr)
	} else {
		copy(eplm.oe_reqres.C_pan[:], "          ")
	}

	/*
		cPanNo
		C_cover_uncover
		C_giveup_flag
		C_reserved
	*/

	/***** Since there is no provision for
	settlement period in EBA we set this to  **/
	eplm.oe_reqres.Si_settlement_period = 0

	eplm.oe_reqres.C_cover_uncover = 'V'

	eplm.oe_reqres.C_giveup_flag = 'P'

	eplm.oe_reqres.D_filler19 = 0.0

	var reserved [52]byte
	for i := range reserved {
		reserved[i] = ' '
	}
	eplm.oe_reqres.C_reserved = reserved

	log.Printf("[%s] [fnPackOrdnryOrdToNse]  Printing header structure for Ordinary Order....", eplm.serviceName)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'Int Header' li_log_time is :	%d ", eplm.serviceName, eplm.oe_reqres.St_hdr.Li_log_time)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'Int Header' c_alpha_char is :	%c ", eplm.serviceName, eplm.oe_reqres.St_hdr.C_alpha_char)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'Int Header' si_transaction_code is :	%d ", eplm.serviceName, eplm.oe_reqres.St_hdr.Si_transaction_code)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'Int Header' si_error_code is :	%d ", eplm.serviceName, eplm.oe_reqres.St_hdr.Si_error_code)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'Int Header' c_filler_2 is :	%c ", eplm.serviceName, eplm.oe_reqres.St_hdr.C_filler_2)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'Int Header' c_time_stamp_1 is :	%c ", eplm.serviceName, eplm.oe_reqres.St_hdr.C_time_stamp_1)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'Int Header' c_time_stamp_2 is :	%c ", eplm.serviceName, eplm.oe_reqres.St_hdr.C_time_stamp_2)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'Int Header' si_message_length is :	%d ", eplm.serviceName, eplm.oe_reqres.St_hdr.Si_message_length)

	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_participant_type is :	%c ", eplm.serviceName, eplm.oe_reqres.C_participant_type)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_filler_1 is :	%c ", eplm.serviceName, eplm.oe_reqres.C_filler_1)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' si_competitor_period is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_competitor_period)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' si_solicitor_period is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_solicitor_period)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_modified_cancelled_by is :	%c ", eplm.serviceName, eplm.oe_reqres.C_modified_cancelled_by)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_filler_2 is :	%c ", eplm.serviceName, eplm.oe_reqres.C_filler_2)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' si_reason_code is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_reason_code)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_filler_3 is :	%c ", eplm.serviceName, eplm.oe_reqres.C_filler_3)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' l_token_no is :	%d ", eplm.serviceName, eplm.oe_reqres.L_token_no)

	log.Printf("[%s] [fnPackOrdnryOrdToNse] Printing contract_desc structure for Ordinary Order....", eplm.serviceName)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'contract_desc' c_instrument_name is :	%c ", eplm.serviceName, eplm.oe_reqres.St_con_desc.C_instrument_name)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'contract_desc' c_symbol is :	%c ", eplm.serviceName, eplm.oe_reqres.St_con_desc.C_symbol)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'contract_desc' li_expiry_date is :	%d ", eplm.serviceName, eplm.oe_reqres.St_con_desc.Li_expiry_date)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'contract_desc' li_strike_price is :	%d ", eplm.serviceName, eplm.oe_reqres.St_con_desc.Li_strike_price)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'contract_desc' c_option_type is :	%c ", eplm.serviceName, eplm.oe_reqres.St_con_desc.C_option_type)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'contract_desc' si_ca_level is :	%d ", eplm.serviceName, eplm.oe_reqres.St_con_desc.Si_ca_level)

	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_counter_party_broker_id is :	%c ", eplm.serviceName, eplm.oe_reqres.C_counter_party_broker_id)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_filler_4 is :	%c ", eplm.serviceName, eplm.oe_reqres.C_filler_4)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_filler_5 is :	%c ", eplm.serviceName, eplm.oe_reqres.C_filler_5)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_closeout_flg is :	%c ", eplm.serviceName, eplm.oe_reqres.C_closeout_flg)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_filler_6 is :	%c ", eplm.serviceName, eplm.oe_reqres.C_filler_6)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' si_order_type is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_order_type)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' d_order_number is :	%f ", eplm.serviceName, eplm.oe_reqres.D_order_number)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_account_number is :	%c ", eplm.serviceName, eplm.oe_reqres.C_account_number)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' si_book_type is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_book_type)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' si_buy_sell_indicator is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_buy_sell_indicator)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_disclosed_volume is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_disclosed_volume)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_disclosed_volume_remaining is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_disclosed_volume_remaining)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_total_volume_remaining is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_total_volume_remaining)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_volume is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_volume)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_volume_filled_today is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_volume_filled_today)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_price is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_price)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_trigger_price is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_trigger_price)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_good_till_date is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_good_till_date)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_entry_date_time is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_entry_date_time)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_minimum_fill_aon_volume is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_minimum_fill_aon_volume)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' li_last_modified is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_last_modified)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' c_PanNo is :	%c ", eplm.serviceName, eplm.oe_reqres.C_pan)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' C_cover_uncover is :	%c ", eplm.serviceName, eplm.oe_reqres.C_cover_uncover)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' C_giveup_flag is :	%c ", eplm.serviceName, eplm.oe_reqres.C_giveup_flag)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'St_oe_reqres' C_reserved is :	%c ", eplm.serviceName, eplm.oe_reqres.C_reserved)

	// Log the values of different flags in the 'order flags' structure
	// Log the values of different flags in the 'order flags' structure
	log.Printf("[%s] [fnPackOrdnryOrdToNse] Printing order flags structure for Ordinary Order....", eplm.serviceName)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_ato is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_ATO)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_market is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_Market)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_sl is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_SL)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_mit is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_MIT)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_day is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_Day)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_gtc is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_GTC)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_ioc is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_IOC)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_aon is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_AON)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_mf is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_MF)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_matched_ind is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_MatchedInd)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_traded is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_Traded)))
	log.Printf("[%s] [fnPackOrdnryOrdToNse] 'order flags' flg_modified is : %d", eplm.serviceName, boolToInt(eplm.oe_reqres.St_ord_flg.GetFlagValue(structures.Flg_Modified)))
	// log.Printf("[%s] [fnPackOrdnryOrdToNse] flg_cancelled is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_cancelled)
	// log.Printf("[%s] [fnPackOrdnryOrdToNse] flg_cancel_pending is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_cancel_pending)
	// log.Printf("[%s] [fnPackOrdnryOrdToNse] flg_closed is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_closed)
	// log.Printf("[%s] [fnPackOrdnryOrdToNse] flg_fill_and_kill is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_fill_and_kill)

	eplm.ConvertOrderReqResToNetworkOrder()

	eplm.exch_msg.St_oe_res = *eplm.oe_reqres
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Assigned oe_reqres to St_oe_res]", eplm.serviceName)

	copy(eplm.net_hdr.C_checksum[:], "                ") // 16 spaces

	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Set C_checksum to a blank space]", eplm.serviceName)

	tmpVar := eplm.GetResetSequence(db)
	if tmpVar == -1 {
		log.Printf("[%s] [fnPackOrdnryOrdToNse] [Error: Failed to retrieve sequence number...]", eplm.serviceName)
		log.Printf("[%s] [fnPackOrdnryOrdToNse] [Returning from this function with error]", eplm.serviceName)
		return -1
	} else {
		eplm.net_hdr.I_seq_num = tmpVar
		log.Printf("[%s] [fnPackOrdnryOrdToNse] [Set I_seq_num to: %d]", eplm.serviceName, eplm.net_hdr.I_seq_num)
	}

	sizeNetHeader := unsafe.Sizeof(eplm.net_hdr)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Size of St_net_hdr with Data : %d]", eplm.serviceName, sizeNetHeader)

	sizeOrdEnt := unsafe.Sizeof(eplm.oe_reqres)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Size of St_oe_reqres with Data : %d]", eplm.serviceName, sizeOrdEnt)

	sizeHdr := unsafe.Sizeof(eplm.oe_reqres.St_hdr)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Size of St_HDR with Data : %d]", eplm.serviceName, sizeHdr)

	sizeContractDesc := unsafe.Sizeof(eplm.oe_reqres.St_con_desc)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Size of St_con_desc with Data : %d]", eplm.serviceName, sizeContractDesc)

	sizeOrderFlag := unsafe.Sizeof(eplm.oe_reqres.St_ord_flg)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Size of St_ord_flg with Data : %d]", eplm.serviceName, sizeOrderFlag)

	eplm.net_hdr.S_message_length = int16(sizeOrdEnt + sizeNetHeader)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Set S_message_length with Data to: %d]", eplm.serviceName, eplm.net_hdr.S_message_length)

	sizeNetHeader1 := unsafe.Sizeof(structures.St_net_hdr{})
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Size of St_net_hdr without_data : %d]", eplm.serviceName, sizeNetHeader1)

	sizeOrdEnt1 := unsafe.Sizeof(structures.St_oe_reqres{})
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Size of St_oe_reqres without_data : %d]", eplm.serviceName, sizeOrdEnt1)

	sizeHdr1 := unsafe.Sizeof(structures.St_int_header{})
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Size of St_HDR without_data : %d]", eplm.serviceName, sizeHdr1)

	sizeContractDesc1 := unsafe.Sizeof(structures.St_contract_desc{})
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Size of St_con_desc without_data : %d]", eplm.serviceName, sizeContractDesc1)

	sizeOrderFlag1 := unsafe.Sizeof(structures.St_order_flags{})
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Size of St_ord_flg without_data : %d]", eplm.serviceName, sizeOrderFlag1)

	message_length := int16(sizeOrdEnt1 + sizeNetHeader1)
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Set S_message_length without_data to: %d]", eplm.serviceName, message_length)

	eplm.exch_msg.St_net_header = *eplm.net_hdr

	eplm.ConvertNetHeaderToNetworkOrder()
	log.Printf("[%s] [fnPackOrdnryOrdToNse] [Converted net header to network order]", eplm.serviceName)

	eplm.q_packet.L_msg_type = int64(models.BOARD_LOT_IN)

	eplm.q_packet.St_exch_msg_data = *eplm.exch_msg

	log.Printf("[%s] [fnPackOrdnryOrdToNse] Data Copied into Queue Packet", eplm.serviceName)

	return 0
}

func (eplm *ExchngPackLibMaster) fn_orstonse_cntrct_desc() int {
	log.Printf("[%s] Entered in the function 'fn_orstonse_cntrct_desc'", eplm.serviceName)

	var instrumentName [6]byte

	switch eplm.nse_contract.C_prd_typ {
	case "F":
		copy(instrumentName[:], []byte{'F', 'U', 'T'})
	case "O":
		copy(instrumentName[:], []byte{'O', 'P', 'T'})
	default:
		log.Printf("[%s] Invalid product type: %s", eplm.serviceName, eplm.nse_contract.C_prd_typ)
		return -1
	}

	switch eplm.nse_contract.C_ctgry_indstk {
	case "I":
		instrumentName[3] = 'I'
		if strings.HasPrefix(eplm.nse_contract.C_symbol, "INDIAVIX") {
			instrumentName[4] = 'V'
		} else {
			instrumentName[4] = 'D'
		}
		instrumentName[5] = 'X'
	case "S":
		copy(instrumentName[:], []byte{'S', 'T', 'K'})
	default:
		log.Printf("[%s] Should be Index/stock, got: %s", eplm.serviceName, eplm.nse_contract.C_ctgry_indstk)
		return -1
	}

	copyAndFormatSymbol(eplm.oe_reqres.St_con_desc.C_symbol[:], len(eplm.oe_reqres.St_con_desc.C_symbol), eplm.nse_contract.C_symbol)

	if eplm.longToTimeArr() != 0 {
		log.Printf("[%s] Returned from 'longToTimeArr()' with an Error", eplm.serviceName)
		log.Printf("[%s] Exiting  from 'fn_orstone_cntrct_desc' ", eplm.serviceName)
		return -1
	}

	if eplm.nse_contract.C_prd_typ == "F" {
		eplm.oe_reqres.St_con_desc.Li_strike_price = -1
	} else {
		eplm.oe_reqres.St_con_desc.Li_strike_price = eplm.nse_contract.L_strike_prc
	}

	if eplm.nse_contract.C_prd_typ == "O" {
		if len(eplm.nse_contract.C_opt_typ) > 0 {
			eplm.oe_reqres.St_con_desc.C_option_type[0] = eplm.nse_contract.C_opt_typ[0] //here i have to assume the the string will be of size 1. This problems happens because i replace all the byte array with string
		} else {
			eplm.oe_reqres.St_con_desc.C_option_type[0] = 'X'
		}

		if len(eplm.nse_contract.C_exrc_typ) > 0 {
			eplm.oe_reqres.St_con_desc.C_option_type[1] = eplm.nse_contract.C_exrc_typ[0]
		} else {
			eplm.oe_reqres.St_con_desc.C_option_type[1] = 'X'
		}
	} else {
		eplm.oe_reqres.St_con_desc.C_option_type[0] = 'X'
		eplm.oe_reqres.St_con_desc.C_option_type[1] = 'X'
	}

	eplm.oe_reqres.St_con_desc.Si_ca_level = int16(eplm.nse_contract.L_ca_lvl)

	copy(eplm.oe_reqres.St_con_desc.C_instrument_name[:], instrumentName[:])

	log.Printf("[%s] Exiting from 'fn_orstonse_cntrct_desc' after successfully packing the 'St_contract_desc'", eplm.serviceName)

	return 0
}

func copyAndFormatSymbol(dest []byte, destLen int, src string) { //'fn_orstonse_char'

	srcUpper := strings.ToUpper(src)

	copyLen := len(srcUpper)
	if copyLen > destLen {
		copyLen = destLen
	}

	copy(dest, srcUpper[:copyLen])

	for i := copyLen; i < destLen; i++ {
		dest[i] = ' '
	}
}

func (eplm *ExchngPackLibMaster) longToTimeArr() int {
	// Parse the ISO 8601 date-time string to a time.Time object
	layout := time.RFC3339
	t, err := time.Parse(layout, eplm.nse_contract.C_expry_dt)
	if err != nil {
		log.Printf("[%s] [longToTimeArr] ERROR: error in parsing expiry date: %v", eplm.serviceName, err)
		return -1
	}

	liSrc := t.Unix() + models.TEN_YRS_IN_SEC

	eplm.oe_reqres.St_con_desc.Li_expiry_date = int32(liSrc)

	return 0
}
func (eplm *ExchngPackLibMaster) TimeArrToLong(C_valid_dt string, date *int32) int {

	layout := time.RFC3339

	t, err := time.Parse(layout, C_valid_dt)
	if err != nil {
		log.Printf("[%s] [TimeArrToLong] ERROR: error in parsing valid date: %v", eplm.serviceName, err)
		return -1
	}

	*date = int32(t.Unix())

	return 0
}

func (eplm *ExchngPackLibMaster) ConvertOrderReqResToNetworkOrder() {
	eplm.ConvertIntHeaderToNetworkOrder() //&eplm.oe_reqres.St_hdr
	eplm.oe_reqres.Si_competitor_period = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.Si_competitor_period)
	eplm.oe_reqres.Si_solicitor_period = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.Si_solicitor_period)
	eplm.oe_reqres.Si_reason_code = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.Si_reason_code)
	eplm.oe_reqres.L_token_no = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.L_token_no)
	eplm.ConvertContractDescToNetworkOrder() //&eplm.oe_reqres.St_con_desc
	eplm.oe_reqres.Si_order_type = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.Si_order_type)
	eplm.oe_reqres.D_order_number = eplm.OCM.ConvertFloat64ToNetworkOrder(eplm.oe_reqres.D_order_number)
	eplm.oe_reqres.Si_book_type = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.Si_book_type)
	eplm.oe_reqres.Si_buy_sell_indicator = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.Si_buy_sell_indicator)
	eplm.oe_reqres.Li_disclosed_volume = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_disclosed_volume)
	eplm.oe_reqres.Li_disclosed_volume_remaining = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_disclosed_volume_remaining)
	eplm.oe_reqres.Li_total_volume_remaining = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_total_volume_remaining)
	eplm.oe_reqres.Li_volume = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_volume)
	eplm.oe_reqres.Li_volume_filled_today = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_volume_filled_today)
	eplm.oe_reqres.Li_price = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_price)
	eplm.oe_reqres.Li_trigger_price = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_trigger_price)
	eplm.oe_reqres.Li_good_till_date = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_good_till_date)
	eplm.oe_reqres.Li_entry_date_time = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_entry_date_time)
	eplm.oe_reqres.Li_minimum_fill_aon_volume = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_minimum_fill_aon_volume)
	eplm.oe_reqres.Li_last_modified = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_last_modified)
	eplm.oe_reqres.Si_branch_id = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.Si_branch_id)
	eplm.oe_reqres.Li_trader_id = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.Li_trader_id)
	eplm.oe_reqres.Si_pro_client_indicator = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.Si_pro_client_indicator)
	eplm.oe_reqres.Si_settlement_period = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.Si_settlement_period)
	eplm.oe_reqres.I_order_seq = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.I_order_seq)
	eplm.oe_reqres.D_nnf_field = eplm.OCM.ConvertFloat64ToNetworkOrder(eplm.oe_reqres.D_nnf_field)
	eplm.oe_reqres.D_filler19 = eplm.OCM.ConvertFloat64ToNetworkOrder(eplm.oe_reqres.D_filler19)
	eplm.oe_reqres.L_algo_id = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.L_algo_id)
	eplm.oe_reqres.Si_algo_category = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.Si_algo_category)
	eplm.oe_reqres.Ll_lastactivityref = eplm.OCM.ConvertInt64ToNetworkOrder(eplm.oe_reqres.Ll_lastactivityref)
}

func (eplm *ExchngPackLibMaster) ConvertIntHeaderToNetworkOrder() {
	eplm.oe_reqres.St_hdr.Si_transaction_code = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.St_hdr.Si_transaction_code)
	eplm.oe_reqres.St_hdr.Li_log_time = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.St_hdr.Li_log_time)
	eplm.oe_reqres.St_hdr.Li_trader_id = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.St_hdr.Li_trader_id)
	eplm.oe_reqres.St_hdr.Si_error_code = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.St_hdr.Si_error_code)
	eplm.oe_reqres.St_hdr.Si_message_length = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.St_hdr.Si_message_length)
}

func (eplm *ExchngPackLibMaster) ConvertContractDescToNetworkOrder() {
	eplm.oe_reqres.St_con_desc.Li_expiry_date = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.St_con_desc.Li_expiry_date)
	eplm.oe_reqres.St_con_desc.Li_strike_price = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.oe_reqres.St_con_desc.Li_strike_price)
	eplm.oe_reqres.St_con_desc.Si_ca_level = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.oe_reqres.St_con_desc.Si_ca_level)
}

func (eplm *ExchngPackLibMaster) ConvertNetHeaderToNetworkOrder() {

	eplm.net_hdr.I_seq_num = eplm.OCM.ConvertInt32ToNetworkOrder(eplm.net_hdr.I_seq_num)
	eplm.net_hdr.S_message_length = eplm.OCM.ConvertInt16ToNetworkOrder(eplm.net_hdr.S_message_length)
}

func (eplm *ExchngPackLibMaster) GetResetSequence(db *gorm.DB) int32 {
	var seqNum int32
	var query string

	log.Printf("[%s] [GetResetSequence] [Executing increment sequence query for pipe ID: %s, trade date: %s]", eplm.serviceName, eplm.xchngbook.C_pipe_id, eplm.xchngbook.C_mod_trd_dt)

	query = `
		UPDATE fsp_fo_seq_plcd
		SET fsp_seq_num = fsp_seq_num + 1
		WHERE fsp_pipe_id = ? AND fsp_trd_dt = TO_DATE(?, 'YYYY-MM-DD')
		RETURNING fsp_seq_num;
	`
	result1 := db.Raw(query, eplm.xchngbook.C_pipe_id, eplm.xchngbook.C_mod_trd_dt).Scan(&seqNum)

	if result1.Error != nil {
		log.Printf("[%s] [GetResetSequence] [Error executing increment sequence query: %v]", eplm.serviceName, result1.Error)
		return -1
	}
	log.Printf("[%s] [GetResetSequence] [Incremented sequence number: %d]", eplm.serviceName, seqNum)

	if seqNum == models.MAX_SEQ_NUM {
		log.Printf("[%s] [GetResetSequence] [Sequence number reached MAX_SEQ_NUM, resetting...]", eplm.serviceName)

		query = `
			UPDATE fsp_fo_seq_plcd
			SET fsp_seq_num = 0
			WHERE fsp_pipe_id = ? AND fsp_trd_dt = TO_DATE(?, 'YYYY-MM-DD')
			RETURNING fsp_seq_num;
		`
		result2 := db.Raw(query, eplm.xchngbook.C_pipe_id, eplm.xchngbook.C_mod_trd_dt).Scan(&seqNum)

		if result2.Error != nil {
			log.Printf("[%s] [GetResetSequence] [Error executing reset sequence query: %v]", eplm.serviceName, result2.Error)
			return -1
		}
		log.Printf("[%s] [GetResetSequence] [Sequence number reset to: %d]", eplm.serviceName, seqNum)
	}
	return seqNum
}

func boolToInt(value bool) int {
	if value {
		return 1
	}
	return 0
}
