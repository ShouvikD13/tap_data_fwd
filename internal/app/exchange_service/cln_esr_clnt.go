package esr

import (
	//"bytes"
	//"encoding/binary"

	recvhandler "DATA_FWD_TAP/internal/app/exchange_service/conn_send_recv_handler/recv_handler"
	sendhandler "DATA_FWD_TAP/internal/app/exchange_service/conn_send_recv_handler/send_handler"
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"DATA_FWD_TAP/util/MessageQueue"
	"DATA_FWD_TAP/util/OrderConversion"
	socket "DATA_FWD_TAP/util/TapSocket"
	typeconversionutil "DATA_FWD_TAP/util/TypeConversionUtil"
	"bytes"
	"encoding/binary"

	"fmt"
	"time"

	"gorm.io/gorm"
)

type ESRManager struct {
	Req_q_data         *models.St_req_q_data
	ServiceName        string
	St_exch_msg        *models.St_exch_msg
	St_int_header      *models.St_int_header
	St_net_hdr         *models.St_net_hdr
	St_oe_reqres       *models.St_oe_reqres
	St_sign_on_req     *models.St_sign_on_req
	St_sign_on_res     *models.St_sign_on_res
	St_Error_Response  *models.St_Error_Response
	St_exch_msg_Log_on *models.St_exch_msg_Log_On
	St_exch_msg_resp   *models.St_exch_msg_resp
	//------------  All the Managers ----------------------
	ENVM                  *util.EnvironmentManager
	Req_q_data1           models.St_req_q_data
	PUM                   *util.PasswordUtilManger
	Message_queue_manager *MessageQueue.MessageQueueManager
	TCUM                  *typeconversionutil.TypeConversionUtilManager
	LoggerManager         *util.LoggerManager
	TM                    *util.TransactionManager
	SCM                   *socket.SocketManager
	OCM                   *OrderConversion.OrderConversionManager
	Max_Pack_Val          int
	SendManager           *sendhandler.SendManager
	RecieveManager        *recvhandler.RecvManager
	//------------- Variables from Factory ------------
	DB            *gorm.DB
	InitialQId    *int
	GlobalQId     *int
	IP            string
	Port          string
	AutoReconnect *bool
	//--------- Values from OPM table------
	OPM_ExchangeCode string
	OPM_PipeID       string
	//--------- Values from Exg table
	EXG_NextTradeDate   string
	EXG_TradeRef        string
	EXG_SecurityTime    string
	EXG_ParticipantTime string
	EXG_InstrumentTime  string
	EXG_IndexTime       string

	// -------- Trigger to communicate between Send and recieve ----------
	ActualResponseTrigger chan *int
	ErrorResponseTrigger  chan *int
}

var Sequence_number int32 = 1

func (ESRM *ESRManager) FnBatInit() error {
	// Query to fetch exchange code
	queryToFetchExchangeCode := `
        SELECT OPM_XCHNG_CD 
        FROM OPM_ORD_PIPE_MSTR 
        WHERE OPM_PIPE_ID = ?`

	ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnBatInit] Fetching exchange code for pipe ID: %s", ESRM.OPM_PipeID)

	row := ESRM.DB.Raw(queryToFetchExchangeCode, ESRM.OPM_PipeID).Row()
	if err := row.Scan(&ESRM.OPM_ExchangeCode); err != nil {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] Error fetching exchange code: %v", err)
		return err
	}

	// Query to fetch trade details
	queryToFetchTradeDetails := `
        SELECT 
            TO_CHAR(EXG_NXT_TRD_DT, 'dd-mon-yyyy'), 
            TO_CHAR(EXG_NXT_TRD_DT, 'yyyymmdd'), 
            TO_CHAR(EXG_SEC_TM, 'dd-mon-yyyy'), 
            TO_CHAR(EXG_PART_TM, 'dd-mon-yyyy'), 
            TO_CHAR(EXG_INST_TM, 'dd-mon-yyyy'), 
            TO_CHAR(EXG_IDX_TM, 'dd-mon-yyyy')
        FROM EXG_XCHNG_MSTR 
        WHERE EXG_XCHNG_CD = ?`

	ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnBatInit] Fetching trade details for exchange code: %s", ESRM.OPM_ExchangeCode)

	row = ESRM.DB.Raw(queryToFetchTradeDetails, ESRM.OPM_ExchangeCode).Row()
	if err := row.Scan(&ESRM.EXG_NextTradeDate, &ESRM.EXG_TradeRef, &ESRM.EXG_SecurityTime, &ESRM.EXG_ParticipantTime, &ESRM.EXG_InstrumentTime, &ESRM.EXG_IndexTime); err != nil {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] Error fetching trade details: %v", err)
		return err
	}

	// Begin transaction
	transactionStatus := ESRM.TM.FnBeginTran()
	if transactionStatus != 0 {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] Failed to begin transaction")
		return fmt.Errorf("transaction begin failed")
	}

	// Query to update order pipe master
	queryToUpdateOrderPipeMaster := `
        UPDATE OPM_ORD_PIPE_MSTR
        SET OPM_LOGIN_STTS = 0, OPM_IPO_SSSN_ID = ?
        WHERE OPM_PIPE_ID = ?`

	ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnBatInit] Updating login status for pipe ID: %s", ESRM.OPM_PipeID)

	if result := ESRM.DB.Exec(queryToUpdateOrderPipeMaster, ESRM.IP, ESRM.OPM_PipeID); result.Error != nil {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] Error updating OPM_ORD_PIPE_MSTR: %v", result.Error)
		if abortResult := ESRM.TM.FnAbortTran(); abortResult == -1 {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] Failed to abort transaction")
		}
		return result.Error
	}

	// Commit transaction
	commitStatus := ESRM.TM.FnCommitTran()
	if commitStatus != 0 {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] Transaction commit failed")
		return fmt.Errorf("transaction commit failed")
	}

	/***********************SEND HANDLER INITIALIZATIO***********************/
	/*
		sendManager := NewSendManager(
			"SendManagerService",
			models.St_system_info_req{},
			models.St_net_hdr{},
			models.St_exch_msg_system_info_Req{},
			models.St_req_q_data_system_info_Req{},
			models.StUpdateLocalDatabase{},
			scm,
			lm,
			ocm,
			tcum,
			db,
			SendRecvTrigger,
			CPipeId,
			XchngCd,
			C_mod_trd_dt,
			C_opm_trdr_id,
		)
	*/

	/***********************RECIEVE HANDLER INITIALIZATION***********************/
	/*
			recvMgr := NewRecvManager(
			"RecvService",
			db,
			tcm,
			pum,
			lm,
			ocm,
			tcum,
			stIntHeader,
			netHDR,
			stSignOnRes,
			stErrorRes,
			stSysInfoDat,
			stLdbData,
			"XCHNG_CD",
			"PipeID_123",
		)

	*/

	ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnBatInit] Initialization successful")
	return nil
}

func (ESRM *ESRManager) FnSendThread() {

	ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "Inside Send Routine STARTS HERE")

	for {

		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, fmt.Sprintf("Attempting to read from queue with Global queue Id: %d", *ESRM.GlobalQId))

		R_L_msg_type, receivedexchngMsg, readErr := ESRM.Message_queue_manager.ReadFromQueue(*ESRM.GlobalQId)
		if readErr != 0 {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, fmt.Sprintf("[send_thrd] [Error: Failed to read from queue with GlobalQueueId... %d]", *ESRM.GlobalQId))
			continue
		}

		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, fmt.Sprintf("[send_thrd] Successfully read from queue with GlobalQueueId: %d, received data: %v", *ESRM.GlobalQId, receivedexchngMsg))

		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, fmt.Sprintf("Message Type: %d", R_L_msg_type))

		// Check if R_L_msg_type is "2300"
		if R_L_msg_type == util.SIGN_ON_REQUEST_IN {
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[send_thrd] Message Type is LOGIN")

			if err := ESRM.SendManager.FnDoXchngLogOn(receivedexchngMsg); err != nil {
				ESRM.LoggerManager.LogError(ESRM.ServiceName, fmt.Sprintf("[send_thrd] Failed to send exch_msg_data: %v", err))
				return
			}

		} else if R_L_msg_type == util.BOARD_LOT_IN {
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[send_thrd] Message type is BOARD LOT IN")

			li_send_tap_msg_size := int64(len(receivedexchngMsg))
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, fmt.Sprintf("[send_thrd] TAP message size: %v", li_send_tap_msg_size))

			// Call fn_writen to write exch_msg_data to TCP connection
			if err := ESRM.SCM.WriteOnTapSocket(receivedexchngMsg); err != nil {
				ESRM.LoggerManager.LogError(ESRM.ServiceName, fmt.Sprintf("[%s] [send_thrd] Failed to send exch_msg_data: %v", ESRM.ServiceName, err))
				return
			}

		} else if R_L_msg_type == util.SIGN_OFF_REQUEST_IN {

		} else {
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, fmt.Sprintf("[%s] [send_thrd] Skipped message with message type: %d", ESRM.ServiceName, R_L_msg_type))
		}

		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "Inside SEND Routine ENDS HERE")
	}
}

func (ESRM *ESRManager) FnRecieveThread() {
	for {

		// Step 1: Fetching the current time before reading from the socket
		formattedTime := time.Now().Format("2006-01-02 15:04:05")
		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] ESR TIME_STATS Before Reading From Socket %s", formattedTime)

		// Step 2: Reading from the socket using the updated ReadFromTapSocket function
		receivedResult, err := ESRM.SCM.ReadFromTapSocket(1024) // 1024 buffer
		if err != nil {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Error while reading from socket: %v", err)
			continue
		}

		// Step 3: Log after reading from the socket
		formattedTime = time.Now().Format("2006-01-02 15:04:05")
		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] ESR TIME_STATS After Reading From Socket %s", formattedTime)

		//Step 4: TAP validation ,  meaning valiating the checksum and sequence number
		initResult := ESRM.RecieveManager.FnValidateTap(&Sequence_number, receivedResult)
		if initResult != 0 {
			continue // from for loop
		}

		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] Validation Successfull")

		// After the Sucessfull validation we need to identify the what we have recieved
		// this will be in following steps
		// step 1: First i have to extract the net_hdr than after extract Int_hdr_then after actual message
		// step 2: we will extract the int header and based on the tranactionCode we are going to extract actual message.

		netHdrSize := binary.Size(ESRM.St_net_hdr)
		intHdrSize := binary.Size(ESRM.St_int_header)

		if len(receivedResult) < netHdrSize+intHdrSize {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Received data is too small to contain headers.")
			break
		}

		netHdrData := receivedResult[:netHdrSize]
		netHdr := ESRM.St_net_hdr

		err = binary.Read(bytes.NewReader(netHdrData), binary.BigEndian, netHdr)
		if err != nil {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Error parsing net_hdr: %v", err)
			break
		}

		*netHdr = ESRM.OCM.ConvertNetHeaderToHostOrder(*netHdr)

		intHdrData := receivedResult[netHdrSize : netHdrSize+intHdrSize]
		intHdr := ESRM.St_int_header

		err = binary.Read(bytes.NewReader(intHdrData), binary.BigEndian, intHdr)
		if err != nil {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Error parsing int_hdr: %v", err)
			break
		}

		ESRM.OCM.ConvertIntHeaderToHostOrder(intHdr)

		// Step 5: Extract actual message
		ActualMessageSize := netHdr.S_message_length - int16(netHdrSize)
		actualDataEnd := int16(netHdrSize+intHdrSize) + ActualMessageSize
		if int(actualDataEnd) > len(receivedResult) {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Actual message size exceeds received data length.")
			break
		}
		ActualData := receivedResult[netHdrSize+intHdrSize : actualDataEnd]

		// Step 6: Process based on transaction code
		switch intHdr.Si_transaction_code {
		case util.SIGN_ON_REQUEST_OUT:
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] SIGN_ON_REQUEST_OUT STARTED")

			if intHdr.Si_error_code != 0 {

				err = binary.Read(bytes.NewReader(ActualData), binary.BigEndian, ESRM.St_Error_Response)
				if err != nil {
					ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Error parsing St_Error_Response: %v", err)
					break
				}

				if Err := ESRM.RecieveManager.FnSignOnRequestOut(); Err != 0 {
					// calling a function
				}

			} else {
				err = binary.Read(bytes.NewReader(ActualData), binary.BigEndian, ESRM.St_sign_on_res)
				if err != nil {
					ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Error parsing sign_on_res: %v", err)
					break
				}

				ESRM.OCM.ConvertSignOnResToHostOrder(ESRM.St_sign_on_res, intHdr)

				initResult := ESRM.RecieveManager.FnSignOnRequestOut()
				if initResult != 0 {
					// calling a function
				}
			}

			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] SIGN_ON_REQUEST_OUT ENDED")

		case util.SIGN_OFF_REQUEST_OUT:

		case util.SYSTEM_INFORMATION_OUT:

		case util.PARTIAL_SYSTEM_INFORMATION:
			// Handle PARTIAL_SYSTEM_INFORMATION case

		case util.UPDATE_LOCALDB_HEADER:
			// Handle UPDATE_LOCALDB_HEADER case

		case util.UPDATE_LOCALDB_TRAILER:
			// Handle UPDATE_LOCALDB_TRAILER case

		case util.UPDATE_LOCALDB_DATA:
			// Handle UPDATE_LOCALDB_DATA case

		default:
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Unknown transaction code received: %d", intHdr.Si_transaction_code)
		}
	}
}

func (ESRM *ESRManager) FnClnEsrClnt() int {
	return 0
}

func (ESRM *ESRManager) functionThatWillBeCalledAfterRecievingErrorfromSignOnoutfunctionAndErrorFUnction() int {
	newExgPwdQuery := `
	SELECT COALESCE(OPM_NEW_EXG_PWD, 'NA') AS new_exg_pwd
	FROM OPM_ORD_PIPE_MSTR
	WHERE OPM_XCHNG_CD = ? AND OPM_PIPE_ID = ?
`

	var cNewExgPwd string
	result := ESRM.DB.Raw(newExgPwdQuery, sqlCXchngCd, sqlCPipeId).Scan(&cNewExgPwd)
	if result.Error != nil {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnGetNewExgPwd] Error executing query: %v", result.Error)
		return "", result.Error
	}

	updateNewExgPwdQuery := `
	UPDATE OPM_ORD_PIPE_MSTR
	SET OPM_NEW_EXG_PWD = NULL
	WHERE OPM_XCHNG_CD = ? AND OPM_PIPE_ID = ?
`

	result = RM.Db.Exec(updateNewExgPwdQuery, sqlCXchngCd, sqlCPipeId)
	if result.Error != nil {
		RM.LM.LogError(RM.ServiceName, "[FnUpdateNewExgPwd] Error executing update query: %v", result.Error)
		return result.Error
	}
	// This function will be called when we are going to return -1 from FnSignInOut and FnErrorResponse. here i have to implement all the logic used in the if condion in recieve thread in C code .
	return 0
}
