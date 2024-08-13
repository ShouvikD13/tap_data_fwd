package app

import "DATA_FWD_TAP/models/structures"

type ExchngPackLibMaster struct {
	serviceName  string
	requestQueue *structures.St_req_q_data
	xchngbook    *structures.Vw_xchngbook
	orderbook    *structures.Vw_orderbook
	contract     *structures.Vw_contract
	nse_contract *structures.Vw_nse_cntrct
	cPanNo       string
	cLstActRef   string
	cEspID       string
	cAlgoID      string
	cSourceFlg   string
}

func (eplm *ExchngPackLibMaster) fnPackOrdnryOrdToNse() int {
	return 0
}
