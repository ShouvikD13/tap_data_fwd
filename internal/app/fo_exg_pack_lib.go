package app

import (
	"DATA_FWD_TAP/models/structures"
	"log"
	"strings"
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
	cPanNo       string
	cLstActRef   string
	cEspID       string
	cAlgoID      string
	cSourceFlg   string
	cPrgmFlg     string
}

// constructor function
func NewExchngPackLibMaster(serviceName string, reqQueue *structures.St_req_q_data, xchngbook *structures.Vw_xchngbook, orderbook *structures.Vw_orderbook, pipe_mstr *structures.St_opm_pipe_mstr, nseContract *structures.Vw_nse_cntrct, cPanNo, cLstActRef, cEspID, cAlgoID, cSourceFlg, cPrgmFlg string) *ExchngPackLibMaster {
	return &ExchngPackLibMaster{
		serviceName:  serviceName,
		requestQueue: reqQueue,
		xchngbook:    xchngbook,
		orderbook:    orderbook,
		pipe_mstr:    pipe_mstr,
		nse_contract: nseContract,
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

	return 0
}
