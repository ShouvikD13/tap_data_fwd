package app

import (
	"DATA_FWD_TAP/models"
	"DATA_FWD_TAP/models/structures"
	"log"
	"reflect"
	"strconv"
	"strings"
	"time"
)

type ExchngPackLibMaster struct {
	serviceName  string
	requestQueue *structures.St_req_q_data
	xchngbook    *structures.Vw_xchngbook
	orderbook    *structures.Vw_orderbook
	pipe_mstr    *structures.St_opm_pipe_mstr
	nse_contract *structures.Vw_nse_cntrct
	oe_reqres    *structures.St_oe_reqres
	exch_msg     *structures.St_exch_msg
	St_hdr       *structures.St_int_header
	St_con_desc  *structures.St_contract_desc
	St_ord_flg   *structures.St_order_flags
	cPanNo       string
	cLstActRef   string
	cEspID       string
	cAlgoID      string
	cSourceFlg   string
	cPrgmFlg     string
	cUserTypGlb  string
}

// constructor function
func NewExchngPackLibMaster(serviceName string, reqQueue *structures.St_req_q_data, xchngbook *structures.Vw_xchngbook, orderbook *structures.Vw_orderbook, pipe_mstr *structures.St_opm_pipe_mstr, nseContract *structures.Vw_nse_cntrct, oe_req *structures.St_oe_reqres, exch_msg *structures.St_exch_msg, St_hdr *structures.St_int_header, St_contract_desc *structures.St_contract_desc, St_ord_flg *structures.St_order_flags, cPanNo, cLstActRef, cEspID, cAlgoID, cSourceFlg, cPrgmFlg string) *ExchngPackLibMaster {
	return &ExchngPackLibMaster{
		serviceName:  serviceName,
		requestQueue: reqQueue,
		xchngbook:    xchngbook,
		orderbook:    orderbook,
		pipe_mstr:    pipe_mstr,
		nse_contract: nseContract,
		oe_reqres:    oe_req,
		exch_msg:     exch_msg,
		St_hdr:       St_hdr,
		St_con_desc:  St_contract_desc,
		St_ord_flg:   St_ord_flg,
		cPanNo:       cPanNo,
		cLstActRef:   cLstActRef,
		cEspID:       cEspID,
		cAlgoID:      cAlgoID,
		cSourceFlg:   cSourceFlg,
		cPrgmFlg:     cPrgmFlg,
	}
}
func (eplm *ExchngPackLibMaster) fnPackOrdnryOrdToNse() int {

	log.Printf("[%s] Inside 'fnPackOrdnryOrdToNse' ", eplm.serviceName)

	eplm.cEspID = strings.TrimSpace(eplm.cEspID)
	eplm.cAlgoID = strings.TrimSpace(eplm.cAlgoID)
	eplm.cSourceFlg = strings.TrimSpace(eplm.cSourceFlg)

	log.Printf("[%s] Trimmed cEspID: '%s'", eplm.serviceName, eplm.cEspID)
	log.Printf("[%s] Trimmed cAlgoID: '%s'", eplm.serviceName, eplm.cAlgoID)
	log.Printf("[%s] Trimmed cSourceFlg: '%s'", eplm.serviceName, eplm.cSourceFlg)

	if eplm.orderbook != nil {
		log.Printf("[%s] Order book data...", eplm.serviceName)
		log.Printf("[%s] Order book c_pro_cli_ind received: '%s'", eplm.serviceName, eplm.orderbook.C_pro_cli_ind)
		log.Printf("[%s] Order book c_cln_mtch_accnt received: '%s'", eplm.serviceName, eplm.orderbook.C_cln_mtch_accnt)
		log.Printf("[%s] Order book c_ordr_flw received: '%s'", eplm.serviceName, eplm.orderbook.C_ordr_flw)
		log.Printf("[%s] Order book l_ord_tot_qty received: '%d'", eplm.serviceName, eplm.orderbook.L_ord_tot_qty)
		log.Printf("[%s] Order book l_exctd_qty received: '%d'", eplm.serviceName, eplm.orderbook.L_exctd_qty)
		log.Printf("[%s] Order book l_exctd_qty_day received: '%d'", eplm.serviceName, eplm.orderbook.L_exctd_qty_day)
		log.Printf("[%s] Order book c_settlor received: '%s'", eplm.serviceName, eplm.orderbook.C_settlor)
		log.Printf("[%s] Order book c_ctcl_id received: '%s'", eplm.serviceName, eplm.orderbook.C_ctcl_id)
		log.Printf("[%s] Order book c_ack_tm received: '%s'", eplm.serviceName, eplm.orderbook.C_ack_tm)
		log.Printf("[%s] Order book c_prev_ack_tm received: '%s'", eplm.serviceName, eplm.orderbook.C_prev_ack_tm)
	} else {
		log.Printf("[%s] No data received in orderbook in 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		log.Printf("[%s] Exiting from 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		return -1
	}

	if eplm.xchngbook != nil {
		log.Printf("[%s] Exchange book data...", eplm.serviceName)
		log.Printf("[%s] Exchange book c_req_typ received: '%s'", eplm.serviceName, eplm.xchngbook.C_req_typ)
		log.Printf("[%s] Exchange book l_dsclsd_qty received: '%d'", eplm.serviceName, eplm.xchngbook.L_dsclsd_qty)
		log.Printf("[%s] Exchange book c_slm_flg received: '%s'", eplm.serviceName, eplm.xchngbook.C_slm_flg)
		log.Printf("[%s] Exchange book l_ord_lmt_rt received: '%d'", eplm.serviceName, eplm.xchngbook.L_ord_lmt_rt)
		log.Printf("[%s] Exchange book l_stp_lss_tgr received: '%d'", eplm.serviceName, eplm.xchngbook.L_stp_lss_tgr)
		log.Printf("[%s] Exchange book c_ord_typ received: '%s'", eplm.serviceName, eplm.xchngbook.C_ord_typ)
		log.Printf("[%s] Exchange book l_ord_tot_qty received: '%d'", eplm.serviceName, eplm.xchngbook.L_ord_tot_qty)
		log.Printf("[%s] Exchange book c_valid_dt received: '%s'", eplm.serviceName, eplm.xchngbook.C_valid_dt)
		log.Printf("[%s] Exchange book l_ord_seq received: '%d'", eplm.serviceName, eplm.xchngbook.L_ord_seq)

	} else {
		log.Printf("[%s] No data received in xchngbook in 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		log.Printf("[%s] Exiting from 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		return -1
	}

	if eplm.pipe_mstr != nil {
		log.Printf("[%s] OPM data...", eplm.serviceName)
		log.Printf("[%s] OPM li_opm_brnch_id received: '%d'", eplm.serviceName, eplm.pipe_mstr.L_opm_brnch_id)
		log.Printf("[%s] OPM c_opm_trdr_id received: '%s'", eplm.serviceName, eplm.pipe_mstr.C_opm_trdr_id)
		log.Printf("[%s] OPM c_xchng_brkr_id received: '%s'", eplm.serviceName, eplm.pipe_mstr.C_xchng_brkr_id)
	} else {
		log.Printf("[%s] No data received in 'OPM_PIPE_MSTR' Struct in 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		log.Printf("[%s] Exiting from 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		return -1
	}

	if eplm.nse_contract != nil {
		log.Printf("[%s] Exchange message structure data...", eplm.serviceName)
		log.Printf("[%s] NSE CONTRACT c_prd_typ received: '%s'", eplm.serviceName, eplm.nse_contract.C_prd_typ)
		log.Printf("[%s] NSE CONTRACT c_ctgry_indstk received: '%s'", eplm.serviceName, eplm.nse_contract.C_ctgry_indstk)
		log.Printf("[%s] NSE CONTRACT c_symbol received: '%s'", eplm.serviceName, eplm.nse_contract.C_symbol)
		log.Printf("[%s] NSE CONTRACT l_ca_lvl received: '%d'", eplm.serviceName, eplm.nse_contract.L_ca_lvl)
		log.Printf("[%s] NSE CONTRACT l_token_id received: '%d'", eplm.serviceName, eplm.nse_contract.L_token_id)

	} else {
		log.Printf("[%s] No data received in 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		log.Printf("[%s] Exiting from 'fnPackOrdnryOrdToNse'", eplm.serviceName)
		return -1
	}

	// Starting thr intialization of Header

	traderID, err := ConvertToInt32(eplm.pipe_mstr.C_opm_trdr_id)
	if err != nil {
		log.Printf("[%s] Conversion failed. Trader ID: %d", eplm.serviceName, traderID)
	} else {
		eplm.St_hdr.Li_trader_id = traderID //1  HDR
	}
	eplm.St_hdr.Li_log_time = 0                                        // 2 HDR
	eplm.St_hdr.C_alpha_char = strings.TrimSpace(strings.ToUpper(" ")) // 3 HDR 'orstonse'
	eplm.St_hdr.Si_transaction_code = models.BOARD_LOT_IN              // 4 HDR
	eplm.St_hdr.C_filler_2 = strings.TrimSpace(strings.ToUpper(" "))   // 5 HDR
	eplm.St_hdr.Si_error_code = 0                                      // 6 HDR
	/*
		for i := range eplm.oe_reqres.St_hdr.C_time_stamp_1 {
			eplm.oe_reqres.St_hdr.C_time_stamp_1[i] = 0 //7 HDR
		}
		for i := range eplm.oe_reqres.St_hdr.C_time_stamp_2 {
			eplm.oe_reqres.St_hdr.C_time_stamp_2[i] = 0 //8 HDR
	}*/
	eplm.St_hdr.C_time_stamp_1 = "0"                                             //7 HDR
	eplm.St_hdr.C_time_stamp_2 = "0"                                             //8 HDR
	eplm.St_hdr.Si_message_length = int32(reflect.TypeOf(eplm.oe_reqres).Size()) //9 HDR

	eplm.oe_reqres.St_hdr = eplm.St_hdr

	// Header structure Done Till here

	switch eplm.pipe_mstr.S_user_typ_glb {
	case 0:
		eplm.cUserTypGlb = models.TRADER
	case 4:
		eplm.cUserTypGlb = models.CORPORATE_MANAGER
	case 5:
		eplm.cUserTypGlb = models.BRANCH_MANAGER
	default:
		log.Printf("[%s] Invalid user type, returning from 'fo_exg_pack_lib' with an error..", eplm.serviceName)
		return -1
	}

	eplm.oe_reqres.Ll_lastactivityref = 0                               //1 BDY
	eplm.oe_reqres.C_participant_type = "C"                             //2 BDY
	eplm.oe_reqres.C_filler_1 = " "                                     //3 BDY
	eplm.oe_reqres.Si_competitor_period = 0                             //4 BDY
	eplm.oe_reqres.Si_solicitor_period = 0                              //5 BDY
	eplm.oe_reqres.C_modified_cancelled_by = eplm.cUserTypGlb           //6 BDY
	eplm.oe_reqres.C_filler_2 = " "                                     //7 BDY
	eplm.oe_reqres.Si_reason_code = 0                                   //8 BDY
	eplm.oe_reqres.C_filler_3 = strings.TrimSpace(strings.ToUpper(" ")) //9 BDY
	eplm.oe_reqres.L_token_no = eplm.nse_contract.L_token_id            //10 BDY

	eplm.oe_reqres.St_con_desc = eplm.fn_orstonse_cntrct_desc()

	log.Printf("[%s] Printing header structure for Ordinary Order....", eplm.serviceName)
	log.Printf("[%s] li_log_time is :	%d ", eplm.serviceName, eplm.oe_reqres.St_hdr.Li_log_time)
	log.Printf("[%s] c_alpha_char is :	%s ", eplm.serviceName, eplm.oe_reqres.St_hdr.C_alpha_char)
	log.Printf("[%s] si_transaction_code is :	%d ", eplm.serviceName, eplm.oe_reqres.St_hdr.Si_transaction_code)
	log.Printf("[%s] si_error_code is :	%d ", eplm.serviceName, eplm.oe_reqres.St_hdr.Si_error_code)
	log.Printf("[%s] c_filler_2 is :	%s ", eplm.serviceName, eplm.oe_reqres.St_hdr.C_filler_2)
	log.Printf("[%s] c_time_stamp_1 is :	%s ", eplm.serviceName, eplm.oe_reqres.St_hdr.C_time_stamp_1)
	log.Printf("[%s] c_time_stamp_2 is :	%s ", eplm.serviceName, eplm.oe_reqres.St_hdr.C_time_stamp_2)
	log.Printf("[%s] si_message_length is :	%d ", eplm.serviceName, eplm.oe_reqres.St_hdr.Si_message_length)

	log.Printf("[%s] c_participant_type is :	%s ", eplm.serviceName, eplm.oe_reqres.C_participant_type)
	log.Printf("[%s] c_filler_1 is :	%s ", eplm.serviceName, eplm.oe_reqres.C_filler_1)
	log.Printf("[%s] si_competitor_period is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_competitor_period)
	log.Printf("[%s] si_solicitor_period is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_solicitor_period)
	log.Printf("[%s] c_modified_cancelled_by is :	%s ", eplm.serviceName, eplm.oe_reqres.C_modified_cancelled_by)
	log.Printf("[%s] c_filler_2 is :	%s ", eplm.serviceName, eplm.oe_reqres.C_filler_2)
	log.Printf("[%s] si_reason_code is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_reason_code)
	log.Printf("[%s] c_filler_3 is :	%s ", eplm.serviceName, eplm.oe_reqres.C_filler_3)
	log.Printf("[%s] l_token_no is :	%d ", eplm.serviceName, eplm.oe_reqres.L_token_no)

	log.Printf("[%s] Printing contract_desc structure for Ordinary Order....", eplm.serviceName)
	log.Printf("[%s] c_instrument_name is :	%s ", eplm.serviceName, eplm.oe_reqres.St_con_desc.C_instrument_name)
	log.Printf("[%s] c_symbol is :	%s ", eplm.serviceName, eplm.oe_reqres.St_con_desc.C_symbol)
	log.Printf("[%s] li_expiry_date is :	%d ", eplm.serviceName, eplm.oe_reqres.St_con_desc.Li_expiry_date)
	log.Printf("[%s] li_strike_price is :	%d ", eplm.serviceName, eplm.oe_reqres.St_con_desc.Li_strike_price)
	log.Printf("[%s] c_option_type is :	%s ", eplm.serviceName, eplm.oe_reqres.St_con_desc.C_option_type)
	log.Printf("[%s] si_ca_level is :	%d ", eplm.serviceName, eplm.oe_reqres.St_con_desc.Si_ca_level)

	log.Printf("[%s] c_counter_party_broker_id is :	%s ", eplm.serviceName, eplm.oe_reqres.C_counter_party_broker_id)
	log.Printf("[%s] c_filler_4 is :	%s ", eplm.serviceName, eplm.oe_reqres.C_filler_4)
	log.Printf("[%s] c_filler_5 is :	%s ", eplm.serviceName, eplm.oe_reqres.C_filler_5)
	log.Printf("[%s] c_closeout_flg is :	%s ", eplm.serviceName, eplm.oe_reqres.C_closeout_flg)
	log.Printf("[%s] c_filler_6 is :	%s ", eplm.serviceName, eplm.oe_reqres.C_filler_6)
	log.Printf("[%s] si_order_type is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_order_type)
	log.Printf("[%s] d_order_number is :	%f ", eplm.serviceName, eplm.oe_reqres.D_order_number)
	log.Printf("[%s] c_account_number is :	%s ", eplm.serviceName, eplm.oe_reqres.C_account_number)
	log.Printf("[%s] si_book_type is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_book_type)
	log.Printf("[%s] si_buy_sell_indicator is :	%d ", eplm.serviceName, eplm.oe_reqres.Si_buy_sell_indicator)
	log.Printf("[%s] li_disclosed_volume is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_disclosed_volume)
	log.Printf("[%s] li_disclosed_volume_remaining is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_disclosed_volume_remaining)
	log.Printf("[%s] li_total_volume_remaining is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_total_volume_remaining)
	log.Printf("[%s] li_volume is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_volume)
	log.Printf("[%s] li_volume_filled_today is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_volume_filled_today)
	log.Printf("[%s] li_price is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_price)
	log.Printf("[%s] li_trigger_price is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_trigger_price)
	log.Printf("[%s] li_good_till_date is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_good_till_date)
	log.Printf("[%s] li_entry_date_time is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_entry_date_time)
	log.Printf("[%s] li_minimum_fill_aon_volume is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_minimum_fill_aon_volume)
	log.Printf("[%s] li_last_modified is :	%d ", eplm.serviceName, eplm.oe_reqres.Li_last_modified)

	log.Printf("[%s] Printing order flags structure for Ordinary Order....", eplm.serviceName)
	log.Printf("[%s] flg_ato is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_ato)
	log.Printf("[%s] flg_market is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_market)
	log.Printf("[%s] flg_sl is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_sl)
	log.Printf("[%s] flg_mit is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_mit)
	log.Printf("[%s] flg_day is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_day)
	log.Printf("[%s] flg_gtc is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_gtc)
	log.Printf("[%s] flg_ioc is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_ioc)
	log.Printf("[%s] flg_aon is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_aon)
	log.Printf("[%s] flg_mf is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_mf)
	log.Printf("[%s] flg_matched_ind is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_matched_ind)
	log.Printf("[%s] flg_traded is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_traded)
	log.Printf("[%s] flg_modified is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_modified)
	// log.Printf("[%s] flg_cancelled is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_cancelled)
	// log.Printf("[%s] flg_cancel_pending is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_cancel_pending)
	// log.Printf("[%s] flg_closed is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_closed)
	// log.Printf("[%s] flg_fill_and_kill is :	%d ", eplm.serviceName, eplm.oe_reqres.St_ord_flg.Flg_fill_and_kill)

	return 0
}

func (eplm *ExchngPackLibMaster) fn_orstonse_cntrct_desc() *structures.St_contract_desc {
	log.Printf("[%s] Entered in the function 'fn_orstonse_cntrct_desc'", eplm.serviceName)

	var instrumentName [6]byte

	switch eplm.nse_contract.C_prd_typ {
	case "F":
		copy(instrumentName[:], []byte{'F', 'U', 'T'})
	case "O":
		copy(instrumentName[:], []byte{'O', 'P', 'T'})
	default:
		log.Printf("[%s] Invalid product type: %s", eplm.serviceName, eplm.nse_contract.C_prd_typ)
		return nil
	}

	switch eplm.nse_contract.C_ctgry_indstk {
	case "I":
		instrumentName[3] = 'I'
		if strings.HasPrefix(string(eplm.nse_contract.C_symbol[:]), "INDIAVIX") {
			instrumentName[4] = 'V'
		} else {
			instrumentName[4] = 'D'
		}
		instrumentName[5] = 'X'
	case "S":
		copy(instrumentName[:], []byte{'S', 'T', 'K'})
	default:
		log.Printf("[%s] Should be Index/stock, got: %s", eplm.serviceName, eplm.nse_contract.C_ctgry_indstk)
		return nil
	}

	copyAndFormatSymbol(eplm.St_con_desc.C_symbol[:], len(eplm.St_con_desc.C_symbol), eplm.nse_contract.C_symbol)

	expiryDateStr := eplm.nse_contract.C_expry_dt + " 14:30:00"
	expiryDate, err := time.Parse("2006-01-02 15:04:05", expiryDateStr)
	if err != nil {
		log.Printf("[%s] Error parsing expiry date: %v", eplm.serviceName, err)
		return nil
	}
	eplm.St_con_desc.Li_expiry_date = int32(expiryDate.Unix())

	if eplm.nse_contract.C_prd_typ == "F" {
		eplm.St_con_desc.Li_strike_price = -1
	} else {
		eplm.St_con_desc.Li_strike_price = eplm.nse_contract.L_strike_prc
	}

	if eplm.nse_contract.C_prd_typ == "O" {
		if len(eplm.nse_contract.C_opt_typ) > 0 {
			eplm.St_con_desc.C_option_type[0] = eplm.nse_contract.C_opt_typ[0] //here i have to assume the the string will be of size 1. This problems happens because i replace all the byte array with string
		} else {
			eplm.St_con_desc.C_option_type[0] = 'X'
		}

		if len(eplm.nse_contract.C_exrc_typ) > 0 {
			eplm.St_con_desc.C_option_type[1] = eplm.nse_contract.C_exrc_typ[0] //here i have to assume the the string will be of size 1. This problems happens because i replace all the byte array with string
		} else {
			eplm.St_con_desc.C_option_type[1] = 'X'
		}
	} else {
		eplm.St_con_desc.C_option_type[0] = 'X'
		eplm.St_con_desc.C_option_type[1] = 'X'
	}

	eplm.St_con_desc.Si_ca_level = int16(eplm.nse_contract.L_ca_lvl)

	copy(eplm.St_con_desc.C_instrument_name[:], instrumentName[:])

	log.Printf("[%s] Exiting from 'fn_orstonse_cntrct_desc' after successfully packing the 'St_contract_desc'", eplm.serviceName)

	return eplm.St_con_desc
}

func ConvertToInt32(s string) (int32, error) {
	trimmed := strings.TrimSpace(s)
	value, err := strconv.Atoi(trimmed)
	if err != nil {
		log.Printf("Error converting string to int: %v", err)
		return -1, err
	}
	return int32(value), nil
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
