package sendhandler

import (
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"DATA_FWD_TAP/util/OrderConversion"
	socket "DATA_FWD_TAP/util/TapSocket"
	typeconversionutil "DATA_FWD_TAP/util/TypeConversionUtil"
	"bytes"
	"crypto/md5"
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"strconv"

	"gorm.io/gorm"
)

type SendManager struct {
	ServiceName string
	//---------- Required models ---------------------
	St_system_info_req            models.St_system_info_req
	St_net_hdr                    models.St_net_hdr
	St_exch_msg_system_info       models.St_exch_msg_system_info_Req
	St_req_q_data_system_info_Req models.St_req_q_data_system_info_Req

	// 'stSysInfoDat' used here to update the market status of  'St_upd_local_db'
	stSysInfoDat                        *models.StSystemInfoData
	St_upd_local_db                     models.StUpdateLocalDatabase
	St_Exch_Msg_UpdateLocalDatabase     models.St_Exch_Msg_UpdateLocalDatabase
	St_req_q_data_StUpdateLocalDatabase models.St_req_q_data_StUpdateLocalDatabase

	SCM             *socket.SocketManager
	LM              *util.LoggerManager
	OCM             *OrderConversion.OrderConversionManager
	TCUM            *typeconversionutil.TypeConversionUtilManager
	Db              *gorm.DB
	SendRecvTrigger int // this is set globally
	CPipeId         string
	XchngCd         string
	C_mod_trd_dt    string
	C_opm_trdr_id   string
	// ---------------- Value s Recieved from 'cln_esr_clnt' --------
	EXG_NextTradeDate   string
	EXG_TradeRef        string
	EXG_SecurityTime    string
	EXG_ParticipantTime string
	EXG_InstrumentTime  string
	EXG_IndexTime       string

	// -------- Trigger to communicate between Send and recieve ----------
	ResponseTrigger *int
}

func NewSendManager(
	serviceName string,
	stSystemInfoReq models.St_system_info_req,
	stNetHdr models.St_net_hdr,
	stExchMsgSystemInfoReq models.St_exch_msg_system_info_Req,
	stReqQDataSystemInfoReq models.St_req_q_data_system_info_Req,
	stUpdLocalDb models.StUpdateLocalDatabase,
	scm *socket.SocketManager,
	lm *util.LoggerManager,
	ocm *OrderConversion.OrderConversionManager,
	tcum *typeconversionutil.TypeConversionUtilManager,
	db *gorm.DB,
	sendRecvTrigger int,
	cPipeId, xchngCd, cModTrdDt, cOpmTrdrId string,
) *SendManager {
	return &SendManager{
		ServiceName:                   serviceName,
		St_system_info_req:            stSystemInfoReq,
		St_net_hdr:                    stNetHdr,
		St_exch_msg_system_info:       stExchMsgSystemInfoReq,
		St_req_q_data_system_info_Req: stReqQDataSystemInfoReq,
		St_upd_local_db:               stUpdLocalDb,

		SCM:             scm,
		LM:              lm,
		OCM:             ocm,
		TCUM:            tcum,
		Db:              db,
		SendRecvTrigger: sendRecvTrigger,
		CPipeId:         cPipeId,
		XchngCd:         xchngCd,
		C_mod_trd_dt:    cModTrdDt,
		C_opm_trdr_id:   cOpmTrdrId,
	}
}

func (SM *SendManager) FnDoXchngLogOn(Data []byte) error {

	if err := SM.SCM.WriteOnTapSocket(Data); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnDoXchngLogOn] Error writing to socket: %v", err)
		return fmt.Errorf("error writing to socket: %w", err)
	}

	*SM.ResponseTrigger = util.LOGON_REQ_SENT

	// Wait until response is received or error is triggered
	for {
		if *SM.ResponseTrigger != util.LOGON_RESP_RCVD && *SM.ResponseTrigger != util.RCV_ERR {
			break
		}
	}

	if SM.SendRecvTrigger == util.RCV_ERR {
		errMsg := fmt.Sprintf("%s [FnDoXchngLogOn] Received error in receive thread.", SM.ServiceName)
		SM.LM.LogError(SM.ServiceName, errMsg)
		SM.LM.LogInfo(SM.ServiceName, "[FnDoXchngLogOn] Value of 'SendRecvTrigger' received from 'fnrecievethread': %d", SM.SendRecvTrigger)
		return errors.New(errMsg)
	}

	// Query to fetch trader ID
	queryToFetchTraderID := `SELECT OPM_TRDR_ID FROM OPM_ORD_PIPE_MSTR WHERE OPM_PIPE_ID = ? AND OPM_XCHNG_CD = ?`
	SM.LM.LogInfo(SM.ServiceName, "[FnDoXchngLogOn] Executing query to fetch trader ID with exchange code: %s and pipe ID: %s", SM.XchngCd, SM.CPipeId)

	row := SM.Db.Raw(queryToFetchTraderID, SM.CPipeId, SM.XchngCd).Row()
	if err := row.Scan(&SM.C_opm_trdr_id); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnDoXchngLogOn] Error scanning row for trader ID: %v", err)
		return err
	}

	SysInfoErr := SM.FnSystemInfoReq()
	if SysInfoErr != nil {
		SM.LM.LogError(SM.ServiceName, "[FnDoXchngLogOn] FnSystemInfoReq encountered an error: %v", SysInfoErr)
		return SysInfoErr
	}

	for SM.SendRecvTrigger != util.LOGON_RESP_RCVD && SM.SendRecvTrigger != util.RCV_ERR {
		// Here, add the logic to wait until a response is received from the socket
	}

	LocalDbErr := SM.FnLocalDBReq()
	if LocalDbErr != nil {
		SM.LM.LogError(SM.ServiceName, "[FnDoXchngLogOn] FnLocalDBReq encountered an error: %v", LocalDbErr)
		return LocalDbErr
	}

	for SM.SendRecvTrigger != util.LOGON_RESP_RCVD && SM.SendRecvTrigger != util.RCV_ERR {
		// Here, add the logic to wait until a response is received from the socket
	}
	return nil
}

func (SM *SendManager) FnSystemInfoReq() error {

	//----------- Header Packing Starts Here --------------------

	traderID, err := strconv.Atoi(SM.C_opm_trdr_id)
	if err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnSystemInfoReq] Invalid trader ID", err)
		return err
	}
	SM.St_system_info_req.St_Hdr.Li_trader_id = int32(traderID)                                // 1 HDR
	SM.St_system_info_req.St_Hdr.Li_log_time = 0                                               // 2 HDR
	copy(SM.St_system_info_req.St_Hdr.C_alpha_char[:], " ")                                    // 3 HDR
	SM.St_system_info_req.St_Hdr.Si_transaction_code = util.SYSTEM_INFORMATION_IN              // 4 HDR
	SM.St_system_info_req.St_Hdr.Si_error_code = 0                                             // 5 HDR
	copy(SM.St_system_info_req.St_Hdr.C_filler_2[:], " ")                                      // 6 HDR
	SM.St_system_info_req.St_Hdr.C_time_stamp_1 = [8]byte{}                                    // 7 HDR
	SM.St_system_info_req.St_Hdr.C_time_stamp_2 = [8]byte{}                                    // 8 HDR
	SM.St_system_info_req.St_Hdr.Si_message_length = int16(binary.Size(SM.St_system_info_req)) // 9 HDR

	//-------------------- Header packing Done Here ---------------------------------

	//-------------------- Started Packing for Body ---------------------------------

	SM.St_system_info_req.Li_last_update_protfolio_time = 0 // 1 BDY

	//-------------------- Body packing Done Here ------------------------------------

	// Logging the header structure
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] Printing Header structure...")
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] 'Header' Li_trader_id is: %d", SM.St_system_info_req.St_Hdr.Li_trader_id)               // 1 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] 'Header' Li_log_time is: %d", SM.St_system_info_req.St_Hdr.Li_log_time)                 // 2 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] 'Header' C_alpha_char is: %s", string(SM.St_system_info_req.St_Hdr.C_alpha_char[:]))    // 3 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] 'Header' Si_transaction_code is: %d", SM.St_system_info_req.St_Hdr.Si_transaction_code) // 4 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] 'Header' Si_error_code is: %d", SM.St_system_info_req.St_Hdr.Si_error_code)             // 5 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] 'Header' C_filler_2 is: %s", string(SM.St_system_info_req.St_Hdr.C_filler_2[:]))        // 6 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] 'Header' C_time_stamp_1 is: %v", SM.St_system_info_req.St_Hdr.C_time_stamp_1)           // 7 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] 'Header' C_time_stamp_2 is: %v", SM.St_system_info_req.St_Hdr.C_time_stamp_2)           // 8 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] 'Header' Si_message_length is: %d", SM.St_system_info_req.St_Hdr.Si_message_length)     // 9 HDR

	// Logging the body structure
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] Printing Body structure...")
	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] 'Body' Li_last_update_protfolio_time is: %d", SM.St_system_info_req.Li_last_update_protfolio_time) // 1 BDY

	//ByteOrder Conversion of Header
	SM.OCM.ConvertIntHeaderToNetworkOrder(&SM.St_system_info_req.St_Hdr)
	//ByteOrder Conversion of Body
	SM.St_system_info_req.Li_last_update_protfolio_time = SM.OCM.ConvertInt32ToNetworkOrder(SM.St_system_info_req.Li_last_update_protfolio_time)

	//-------------------- Net Hdr Packing Starts Here--------------------------------

	tmpVar, er1 := SM.TCUM.GetResetSequence(SM.Db, SM.CPipeId, SM.C_mod_trd_dt)
	if er1 != 0 {
		SM.LM.LogError(SM.ServiceName, "[FnLocalDBReq] Error retrieving sequence number: %v", err)
		return fmt.Errorf("error retrieving sequence number: %w", err)
	}

	SM.St_net_hdr.I_seq_num = tmpVar // 1 NET_HDR

	hasher := md5.New()
	data, err := json.Marshal(SM.St_system_info_req)
	if err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnSystemInfoReq] Error marshaling St_system_info_req: %v", err)
		return fmt.Errorf("failed to marshal St_system_info_req: %w", err)
	}
	if _, err := hasher.Write(data); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnSystemInfoReq] Error hashing data: %v", err)
		return fmt.Errorf("failed to hash data: %w", err)
	}
	copy(SM.St_net_hdr.C_checksum[:], fmt.Sprintf("%x", hasher.Sum(nil)))                                          // 2 NET_HDR
	SM.St_net_hdr.S_message_length = int16(binary.Size(SM.St_net_hdr)) + int16(binary.Size(SM.St_system_info_req)) // 3 NET_HDR

	// Byte Order Conversion for Net HDR
	SM.OCM.ConvertNetHeaderToNetworkOrder(&SM.St_net_hdr)

	buf := new(bytes.Buffer)

	if err := SM.TCUM.WriteAndCopy(buf, SM.St_system_info_req, SM.St_exch_msg_system_info.St_system_info_req[:]); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnSystemInfoReq] Error: Failed to write St_system_info_req: %v", err)
		return fmt.Errorf("failed to write St_system_info_req: %w", err)
	}

	if err := SM.TCUM.WriteAndCopy(buf, SM.St_net_hdr, SM.St_exch_msg_system_info.St_net_header[:]); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnSystemInfoReq] [Error: Failed to write St_net_hdr: %v", err)
		return fmt.Errorf("failed to write St_net_hdr: %w", err)
	}

	if err := SM.TCUM.WriteAndCopy(buf, SM.St_exch_msg_system_info, SM.St_req_q_data_system_info_Req.St_exch_msg_system_info_Req[:]); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnSystemInfoReq] [Error: Failed to write St_exch_msg_system_info: %v", err)
		return fmt.Errorf("failed to write St_exch_msg_system_info: %w", err)
	}

	if err := SM.SCM.WriteOnTapSocket(SM.St_req_q_data_system_info_Req.St_exch_msg_system_info_Req[:]); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnSystemInfoReq] Error writing to socket: %v", err)
		return fmt.Errorf("error writing to socket: %w", err)
	}

	SM.LM.LogInfo(SM.ServiceName, "[FnSystemInfoReq] Successfully sent System_Info_Request on the socket")

	return nil
}

func (SM *SendManager) FnLocalDBReq() error {

	//------------------------------------------ Header Packing Starts from here -------------------------------------
	trdrId, err := strconv.Atoi(SM.C_opm_trdr_id)
	if err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnLocalDBReq] Invalid trader ID", err)
		return err
	}
	SM.St_upd_local_db.St_Hdr.Li_trader_id = int32(trdrId)                               // 1 HDR
	SM.St_upd_local_db.St_Hdr.Li_log_time = 0                                            // 2 HDR
	copy(SM.St_upd_local_db.St_Hdr.C_alpha_char[:], " ")                                 // 3 HDR
	SM.St_upd_local_db.St_Hdr.Si_transaction_code = util.UPDATE_LOCALDB_IN               // 4 HDR
	SM.St_upd_local_db.St_Hdr.Si_error_code = 0                                          // 5 HDR
	copy(SM.St_upd_local_db.St_Hdr.C_filler_2[:], " ")                                   // 6 HDR
	SM.St_upd_local_db.St_Hdr.C_time_stamp_1 = [8]byte{}                                 // 7 HDR
	SM.St_upd_local_db.St_Hdr.C_time_stamp_2 = [8]byte{}                                 // 8 HDR
	SM.St_upd_local_db.St_Hdr.Si_message_length = int16(binary.Size(SM.St_upd_local_db)) // 9 HDR

	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] Printing Header structure...")
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Header' Li_trader_id is: %d", SM.St_upd_local_db.St_Hdr.Li_trader_id)               // 1 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Header' Li_log_time is: %d", SM.St_upd_local_db.St_Hdr.Li_log_time)                 // 2 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Header' C_alpha_char is: %s", string(SM.St_upd_local_db.St_Hdr.C_alpha_char[:]))    // 3 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Header' Si_transaction_code is: %d", SM.St_upd_local_db.St_Hdr.Si_transaction_code) // 4 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Header' Si_error_code is: %d", SM.St_upd_local_db.St_Hdr.Si_error_code)             // 5 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Header' C_filler_2 is: %s", string(SM.St_upd_local_db.St_Hdr.C_filler_2[:]))        // 6 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Header' C_time_stamp_1 is: %v", SM.St_upd_local_db.St_Hdr.C_time_stamp_1)           // 7 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Header' C_time_stamp_2 is: %v", SM.St_upd_local_db.St_Hdr.C_time_stamp_2)           // 8 HDR
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Header' Si_message_length is: %d", SM.St_upd_local_db.St_Hdr.Si_message_length)     // 9 HDR

	//------------------------ Header Pacing Done Here ----------------------------------------------
	//------------------------- Body Packig Starts from here ---------------------------------------

	initErr, initResult := SM.TCUM.LongToTimeArr(SM.EXG_SecurityTime)
	if initErr != 0 {
		errMsg := fmt.Errorf("[FnLocalDBReq] Error received from 'LongToTimeArr' for EXG_SecurityTime: %d", initErr)
		SM.LM.LogError(SM.ServiceName, errMsg.Error())
		return errMsg
	}
	SM.St_upd_local_db.Li_LastUpdateSecurityTime = initResult // 1 BDY

	partErr, partResult := SM.TCUM.LongToTimeArr(SM.EXG_ParticipantTime)
	if partErr != 0 {
		errMsg := fmt.Errorf("[FnLocalDBReq] Error received from 'LongToTimeArr' for Sql_part_tm.Arr: %d", partErr)
		SM.LM.LogError(SM.ServiceName, errMsg.Error())
		return errMsg
	}
	SM.St_upd_local_db.Li_LastUpdateParticipantTime = partResult // 2 BDY

	instErr, instResult := SM.TCUM.LongToTimeArr(SM.EXG_InstrumentTime)
	if instErr != 0 {
		errMsg := fmt.Errorf("[FnLocalDBReq] Error received from 'LongToTimeArr' for Sql_inst_tm.Arr: %d", instErr)
		SM.LM.LogError(SM.ServiceName, errMsg.Error())
		return errMsg
	}
	SM.St_upd_local_db.Li_LastUpdateInstrumentTime = instResult // 3 BDY

	idxErr, idxResult := SM.TCUM.LongToTimeArr(SM.EXG_IndexTime)
	if idxErr != 0 {
		errMsg := fmt.Errorf("[FnLocalDBReq] Error received from 'LongToTimeArr' for Sql_idx_tm.Arr: %d", idxErr)
		SM.LM.LogError(SM.ServiceName, errMsg.Error())
		return errMsg
	}
	SM.St_upd_local_db.Li_LastUpdateIndexTime = idxResult // 4 BDY

	// Set remaining fields
	SM.St_upd_local_db.C_RequestForOpenOrders = 'G' // 5 BDY // For now i have set this field sttically . check point no. 16 in 'RemainingTasks.txt'
	SM.St_upd_local_db.C_Filler1 = ' '              // 6 BDY

	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq]  Li_LastUpdateSecurityTime is: %d", SM.St_upd_local_db.Li_LastUpdateSecurityTime)       // 1 BDY
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq]  Li_LastUpdateParticipantTime is: %d", SM.St_upd_local_db.Li_LastUpdateParticipantTime) // 2 BDY
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq]  Li_LastUpdateInstrumentTime is: %d", SM.St_upd_local_db.Li_LastUpdateInstrumentTime)   // 3 BDY
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq]  Li_LastUpdateIndexTime is: %d", SM.St_upd_local_db.Li_LastUpdateIndexTime)             // 4 BDY
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq]  C_RequestForOpenOrders is: %c", SM.St_upd_local_db.C_RequestForOpenOrders)             // 5 BDY
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq]  C_Filler1 is: %c", SM.St_upd_local_db.C_Filler1)                                       // 6 BDY

	// ------------------------- Body Packing done Here ------------------------

	//-------------------------- Market Status Pakcking starts here -------------
	SM.St_upd_local_db.St_MktStts.SiNormal = SM.stSysInfoDat.StMktStts.SiNormal
	SM.St_upd_local_db.St_MktStts.SiOddlot = SM.stSysInfoDat.StMktStts.SiOddlot
	SM.St_upd_local_db.St_MktStts.SiSpot = SM.stSysInfoDat.StMktStts.SiSpot
	SM.St_upd_local_db.St_MktStts.SiAuction = SM.stSysInfoDat.StMktStts.SiAuction

	SM.St_upd_local_db.St_ExMktStts.SiNormal = SM.stSysInfoDat.StExMktStts.SiNormal
	SM.St_upd_local_db.St_ExMktStts.SiOddlot = SM.stSysInfoDat.StExMktStts.SiOddlot
	SM.St_upd_local_db.St_ExMktStts.SiSpot = SM.stSysInfoDat.StExMktStts.SiSpot
	SM.St_upd_local_db.St_ExMktStts.SiAuction = SM.stSysInfoDat.StExMktStts.SiAuction

	SM.St_upd_local_db.St_PlMktStts.SiNormal = SM.stSysInfoDat.StPlMktStts.SiNormal
	SM.St_upd_local_db.St_PlMktStts.SiOddlot = SM.stSysInfoDat.StPlMktStts.SiOddlot
	SM.St_upd_local_db.St_PlMktStts.SiSpot = SM.stSysInfoDat.StPlMktStts.SiSpot
	SM.St_upd_local_db.St_PlMktStts.SiAuction = SM.stSysInfoDat.StPlMktStts.SiAuction

	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Market Status' St_MktStts.SiNormal is: %d", SM.St_upd_local_db.St_MktStts.SiNormal)   // 1 MKT
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Market Status' St_MktStts.SiOddlot is: %d", SM.St_upd_local_db.St_MktStts.SiOddlot)   // 2 MKT
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Market Status' St_MktStts.SiSpot is: %d", SM.St_upd_local_db.St_MktStts.SiSpot)       // 3 MKT
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Market Status' St_MktStts.SiAuction is: %d", SM.St_upd_local_db.St_MktStts.SiAuction) // 4 MKT

	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Extended Market Status' St_ExMktStts.SiNormal is: %d", SM.St_upd_local_db.St_ExMktStts.SiNormal)   // 5 MKT
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Extended Market Status' St_ExMktStts.SiOddlot is: %d", SM.St_upd_local_db.St_ExMktStts.SiOddlot)   // 6 MKT
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Extended Market Status' St_ExMktStts.SiSpot is: %d", SM.St_upd_local_db.St_ExMktStts.SiSpot)       // 7 MKT
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Extended Market Status' St_ExMktStts.SiAuction is: %d", SM.St_upd_local_db.St_ExMktStts.SiAuction) // 8 MKT

	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Planned Market Status' St_PlMktStts.SiNormal is: %d", SM.St_upd_local_db.St_PlMktStts.SiNormal)   // 9 MKT
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Planned Market Status' St_PlMktStts.SiOddlot is: %d", SM.St_upd_local_db.St_PlMktStts.SiOddlot)   // 10 MKT
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Planned Market Status' St_PlMktStts.SiSpot is: %d", SM.St_upd_local_db.St_PlMktStts.SiSpot)       // 11 MKT
	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] 'Planned Market Status' St_PlMktStts.SiAuction is: %d", SM.St_upd_local_db.St_PlMktStts.SiAuction) // 12 MKT

	//------------------------------------ Market Status Pakcking Done here --------------------

	//------------------------------------ Net_HDR packing starts here -------------------------

	SM.OCM.ConvertIntHeaderToNetworkOrder(&SM.St_upd_local_db.St_Hdr)

	tmpVar, er1 := SM.TCUM.GetResetSequence(SM.Db, SM.CPipeId, SM.C_mod_trd_dt)
	if er1 != 0 {
		SM.LM.LogError(SM.ServiceName, "[FnLocalDBReq] Error retrieving sequence number: %v", err)
		return fmt.Errorf("error retrieving sequence number: %w", err)
	}
	SM.St_net_hdr.I_seq_num = tmpVar // 1 NET_HDR

	hasher := md5.New()
	data, err := json.Marshal(SM.St_upd_local_db)
	if err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnLocalDBReq] Error marshaling FnLocalDBReq : %v", err)
		return fmt.Errorf("failed to marshal St_system_info_req: %w", err)
	}
	if _, err := hasher.Write(data); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnLocalDBReq] Error hashing data: %v", err)
		return fmt.Errorf("failed to hash data: %w", err)
	}
	copy(SM.St_net_hdr.C_checksum[:], fmt.Sprintf("%x", hasher.Sum(nil)))                                       // 2 NET_HDR
	SM.St_net_hdr.S_message_length = int16(binary.Size(SM.St_net_hdr)) + int16(binary.Size(SM.St_upd_local_db)) // 3 NET_HDR

	SM.OCM.ConvertNetHeaderToNetworkOrder(&SM.St_net_hdr)

	buf := new(bytes.Buffer)

	if err := SM.TCUM.WriteAndCopy(buf, SM.St_upd_local_db, SM.St_Exch_Msg_UpdateLocalDatabase.StUpdateLocalDatabase[:]); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnLocalDBReq] Error: Failed to write St_system_info_req: %v", err)
		return fmt.Errorf("failed to write St_system_info_req: %w", err)
	}

	if err := SM.TCUM.WriteAndCopy(buf, SM.St_net_hdr, SM.St_Exch_Msg_UpdateLocalDatabase.St_net_header[:]); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnLocalDBReq] [Error: Failed to write St_net_hdr: %v", err)
		return fmt.Errorf("failed to write St_net_hdr: %w", err)
	}

	if err := SM.TCUM.WriteAndCopy(buf, SM.St_Exch_Msg_UpdateLocalDatabase, SM.St_req_q_data_StUpdateLocalDatabase.St_Exch_Msg_UpdateLocalDatabase[:]); err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnLocalDBReq] [Error: Failed to write St_exch_msg_system_info: %v", err)
		return fmt.Errorf("failed to write St_exch_msg_system_info: %w", err)
	}

	// Write to Socket
	err = SM.SCM.WriteOnTapSocket(SM.St_req_q_data_StUpdateLocalDatabase.St_Exch_Msg_UpdateLocalDatabase[:])
	if err != nil {
		SM.LM.LogError(SM.ServiceName, "[FnLocalDBReq] Error writing to socket: %v", err)
		return fmt.Errorf("error writing to socket: %w", err)
	}

	SM.LM.LogInfo(SM.ServiceName, "[FnLocalDBReq] Successfully sent LocalDB Request.")
	return nil
}
