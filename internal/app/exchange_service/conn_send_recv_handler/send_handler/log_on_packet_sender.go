package sendhandler

import (
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	socket "DATA_FWD_TAP/util/TapSocket"
	"fmt"

	"gorm.io/gorm"
)

type SendManager struct {
	ServiceName             string
	St_system_info_req      models.St_system_info_req
	St_exch_msg_system_info models.St_exch_msg_system_info_Req
	SM                      *socket.SocketManager
	LM                      *util.LoggerManager
	Db                      *gorm.DB
	SendRecvTrigger         int // this is set globally
	CPipeId                 string
	XchngCd                 string
	C_opm_trdr_id           string
}

func (SM *SendManager) FnDoXchngLogOn(Data []byte) error {
	// Send the data packet to the socket
	if err := SM.SM.WriteOnTapSocket(Data); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnDoXchngLogOn] Error from 'WriteOnTapSocket': %v", err)
		return err
	}
	SM.LM.LogInfo(SM.ServiceName, "[FnDoXchngLogOn] Successfully sent data packet on socket")

	// Wait until response is received or error is triggered
	for SM.SendRecvTrigger != util.LOGON_RESP_RCVD && SM.SendRecvTrigger != util.RCV_ERR {
		// Here, add the logic to wait until a response is received from the socket
	}

	if SM.SendRecvTrigger == util.RCV_ERR {
		errMsg := fmt.Sprintf("%s [FnDoXchngLogOn] Received error in receive thread.", SM.ServiceName)
		SM.LM.LogError(SM.ServiceName, errMsg)
		SM.LM.LogInfo(SM.ServiceName, "[FnDoXchngLogOn] Value of 'SendRecvTrigger' received from 'fnrecievethread': %d", SM.SendRecvTrigger)
		return fmt.Errorf(errMsg)
	}

	// Query to fetch trader ID
	queryToFetchTraderID := `SELECT OPM_TRDR_ID FROM OPM_ORD_PIPE_MSTR WHERE OPM_PIPE_ID = ? AND OPM_XCHNG_CD = ?`
	SM.LM.LogInfo(SM.ServiceName, "[FnDoXchngLogOn] Executing query to fetch trader ID with exchange code: %s and pipe ID: %s", SM.XchngCd, SM.CPipeId)

	row := SM.Db.Raw(queryToFetchTraderID, SM.CPipeId, SM.XchngCd).Row()
	if err := row.Scan(&SM.C_opm_trdr_id); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnDoXchngLogOn] Error scanning row for trader ID: %v", err)
		return err
	}

	return nil
}

func (SM *SendManager) FnSystemInfoReq(Data []byte) error {
	// Send system info request over the socket
	if err := SM.SM.WriteOnTapSocket(Data); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnSystemInfoReq] Error from 'WriteOnTapSocket': %v", err)
		return err
	}
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] Successfully sent system info request on the socket")

	return nil
}
