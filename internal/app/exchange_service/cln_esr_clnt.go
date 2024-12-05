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
	"sync"

	"fmt"
	"time"

	"gorm.io/gorm"
)

type ESRManager struct {
	Req_q_data                          *models.St_req_q_data
	ServiceName                         string
	St_exch_msg                         *models.St_exch_msg
	St_int_header                       *models.St_int_header
	St_net_hdr                          *models.St_net_hdr
	St_oe_reqres                        *models.St_oe_reqres
	St_sign_on_req                      *models.St_sign_on_req
	St_sign_on_res                      *models.St_sign_on_res
	St_Error_Response                   *models.St_Error_Response
	StSystemInfoData                    *models.StSystemInfoData
	StUpdateLocalDBData                 *models.StUpdateLocalDBData
	St_exch_msg_Log_on                  *models.St_exch_msg_Log_On
	St_exch_msg_resp                    *models.St_exch_msg_resp
	St_system_info_req                  *models.St_system_info_req
	St_exch_msg_system_info_Req         *models.St_exch_msg_system_info_Req
	St_req_q_data_system_info_Req       *models.St_req_q_data_system_info_Req
	StUpdateLocalDatabase               *models.StUpdateLocalDatabase
	St_Exch_Msg_UpdateLocalDatabase     *models.St_Exch_Msg_UpdateLocalDatabase
	St_req_q_data_StUpdateLocalDatabase *models.St_req_q_data_StUpdateLocalDatabase
	//------------  All the Managers ----------------------
	ENVM                  *util.EnvironmentManager
	PUM                   *util.PasswordUtilManger
	Message_queue_manager *MessageQueue.MessageQueueManager
	TCUM                  *typeconversionutil.TypeConversionUtilManager
	LoggerManager         *util.LoggerManager
	TransactionManager    *util.TransactionManager
	SCM                   *socket.SocketManager
	OCM                   *OrderConversion.OrderConversionManager
	Max_Pack_Val          int
	SendManager           *sendhandler.SendManager
	RecieveManager        *recvhandler.RecvManager
	//------------- Variables from Factory ------------
	DB         *gorm.DB
	Args       []string
	InitialQId *int
	GlobalQId  *int
	IP         string
	Port       string
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
	ActualResponseTrigger *chan int
	ErrorResponseTrigger  *chan int

	ChanValue        int
	ThreadExitStatus int
	RecievedStatus   string
}

var Sequence_number int32 = 1

func (ESRM *ESRManager) FnBatInit() error {

	if len(ESRM.Args) < 7 {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] [Error] Insufficient arguments provided. Expected at least 7, but got %d", len(ESRM.Args))
		return fmt.Errorf("[FnBatInit] Insufficient arguments provided. Expected at least 7, but got %d", len(ESRM.Args))
	}

	ESRM.OPM_PipeID = ESRM.Args[3]

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
		TO_CHAR(EXG_NXT_TRD_DT::DATE, 'dd-mon-yyyy') AS nxt_trd_dt_fmt,
		TO_CHAR(EXG_NXT_TRD_DT::DATE, 'yyyymmdd') AS nxt_trd_dt_yyyymmdd,
		TO_CHAR(EXG_SEC_TM::DATE, 'dd-mon-yyyy') AS sec_tm_fmt,
		TO_CHAR(EXG_PART_TM::DATE, 'dd-mon-yyyy') AS part_tm_fmt,
		TO_CHAR(EXG_INST_TM::DATE, 'dd-mon-yyyy') AS inst_tm_fmt,
		TO_CHAR(EXG_IDX_TM::DATE, 'dd-mon-yyyy') AS idx_tm_fmt
		FROM EXG_XCHNG_MSTR 
		WHERE EXG_XCHNG_CD = ?;
	`

	ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnBatInit] Fetching trade details for exchange code: %s", ESRM.OPM_ExchangeCode)

	row = ESRM.DB.Debug().Raw(queryToFetchTradeDetails, ESRM.OPM_ExchangeCode).Row()
	if err := row.Scan(&ESRM.EXG_NextTradeDate, &ESRM.EXG_TradeRef, &ESRM.EXG_SecurityTime, &ESRM.EXG_ParticipantTime, &ESRM.EXG_InstrumentTime, &ESRM.EXG_IndexTime); err != nil {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] Error fetching trade details: %v", err)
		return err
	}

	// Begin transaction
	transactionStatus := ESRM.TransactionManager.FnBeginTran()
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

	if result := ESRM.DB.Debug().Exec(queryToUpdateOrderPipeMaster, ESRM.IP, ESRM.OPM_PipeID); result.Error != nil {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] Error updating OPM_ORD_PIPE_MSTR: %v", result.Error)
		if abortResult := ESRM.TransactionManager.FnAbortTran(); abortResult == -1 {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] Failed to abort transaction")
		}
		return result.Error
	}

	// Commit transaction
	commitStatus := ESRM.TransactionManager.FnCommitTran()
	if commitStatus != 0 {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnBatInit] Transaction commit failed")
		return fmt.Errorf("transaction commit failed")
	}

	/***********************SEND HANDLER INITIALIZATIO***********************/

	ESRM.SendManager = sendhandler.NewSendManager(
		ESRM.ServiceName,
		ESRM.St_system_info_req,
		ESRM.St_net_hdr,
		ESRM.St_exch_msg_system_info_Req,
		ESRM.St_req_q_data_system_info_Req,
		ESRM.StUpdateLocalDatabase,
		ESRM.St_Exch_Msg_UpdateLocalDatabase,
		ESRM.St_req_q_data_StUpdateLocalDatabase,
		ESRM.SCM,
		ESRM.LoggerManager,
		ESRM.OCM,
		ESRM.TCUM,
		ESRM.DB,
		ESRM.OPM_PipeID,
		ESRM.OPM_ExchangeCode,
		ESRM.EXG_NextTradeDate,
		ESRM.EXG_TradeRef,
		ESRM.EXG_SecurityTime,
		ESRM.EXG_ParticipantTime,
		ESRM.EXG_InstrumentTime,
		ESRM.EXG_IndexTime,
		ESRM.ActualResponseTrigger,
		ESRM.ErrorResponseTrigger,
	)

	/*
			return &SendManager{
			ServiceName:                         serviceName,
			St_system_info_req:                  stSystemInfoReq,
			St_net_hdr:                          stNetHdr,
			St_exch_msg_system_info:             stExchMsgSystemInfoReq,
			St_req_q_data_system_info_Req:       stReqQDataSystemInfoReq,
			St_upd_local_db:                     stUpdLocalDb,
			St_Exch_Msg_UpdateLocalDatabase:     stExchMsgUpdLocalDb,
			St_req_q_data_StUpdateLocalDatabase: stReqQDataUpdLocalDb,
			SCM:                                 scm,
			LM:                                  lm,
			OCM:                                 ocm,
			TCUM:                                tcum,
			Db:                                  db,
			CPipeId:                             cPipeId,
			XchngCd:                             xchngCd,
			EXG_NextTradeDate:                   exgNextTradeDate,
			EXG_TradeRef:                        exgTradeRef,
			EXG_SecurityTime:                    exgSecurityTime,
			EXG_ParticipantTime:                 exgParticipantTime,
			EXG_InstrumentTime:                  exgInstrumentTime,
			EXG_IndexTime:                       exgIndexTime,
			ActualResponseTrigger:               actualResponseTrigger,
			ErrorResponseTrigger:                errorResponseTrigger,
		}

	*/

	/***********************RECIEVE HANDLER INITIALIZATION***********************/

	ESRM.RecieveManager = recvhandler.NewRecvManager(
		ESRM.ServiceName,
		ESRM.DB,
		ESRM.TransactionManager,
		ESRM.PUM,
		ESRM.LoggerManager,
		ESRM.OCM,
		ESRM.TCUM,
		ESRM.St_int_header,
		ESRM.St_net_hdr,
		ESRM.St_sign_on_res,
		ESRM.St_Error_Response,
		ESRM.StSystemInfoData,
		ESRM.StUpdateLocalDBData,
		ESRM.OPM_ExchangeCode,
		ESRM.OPM_PipeID,
	)

	/*
			recvManager := recvhandler.NewRecvManager(
			"ServiceName",                  // Replace with your service name
			dbInstance,                     // Instance of *gorm.DB
			transactionManagerInstance,     // Instance of *util.TransactionManager
			passwordUtilManagerInstance,    // Instance of *util.PasswordUtilManger
			loggerManagerInstance,          // Instance of *util.LoggerManager
			orderConversionManagerInstance, // Instance of *OrderConversion.OrderConversionManager
			typeConversionManagerInstance,  // Instance of *typeconversionutil.TypeConversionUtilManager
			intHeaderInstance,              // Instance of *models.St_int_header
			netHeaderInstance,              // Instance of *models.St_net_hdr
			signOnResponseInstance,         // Instance of *models.St_sign_on_res
			errorResponseInstance,          // Instance of *models.St_Error_Response
			sysInfoDataInstance,            // Instance of *models.StSystemInfoData
			ldbDataInstance,                // Instance of *models.StUpdateLocalDBData
			"ExchangeCode",                 // Replace with your exchange code (string)
			"PipeId",                       // Replace with your pipe ID (string)
		)


	*/

	ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnBatInit] Initialization successful Now we are going to call the go routines ")

	var wg sync.WaitGroup

	wg.Add(2)

	go func() {
		defer wg.Done()

		ESRM.FnSendThread()
	}()
	go func() {
		defer wg.Done()

		ESRM.FnRecieveThread()
	}()

	wg.Wait()

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
	ESRM.ThreadExitStatus = util.NOT_EXIT

	for ESRM.ThreadExitStatus != util.EXIT {

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
				if Err := ESRM.RecieveManager.FnErrorResponse(); Err != 0 {
					initResult := ESRM.HandleErrorFromSignInOutAndErrorResponse()
					if initResult != nil {
						ESRM.LoggerManager.LogError(ESRM.ServiceName, "[initResult] Failed to handle error response: %v Exiting from 'FnRecieveThread' Loop", initResult)
						ESRM.ThreadExitStatus = util.EXIT
						*ESRM.ErrorResponseTrigger <- util.RCV_ERR
					}
				} else {

					*ESRM.ActualResponseTrigger <- util.LOGON_RESP_RCVD
				}
			} else {
				err = binary.Read(bytes.NewReader(ActualData), binary.BigEndian, ESRM.St_sign_on_res)
				if err != nil {
					ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Error parsing sign_on_res: %v", err)
					break
				}
				ESRM.OCM.ConvertSignOnResToHostOrder(ESRM.St_sign_on_res, intHdr)

				if initResult := ESRM.RecieveManager.FnSignOnRequestOut(); initResult != 0 {
					initResult := ESRM.HandleErrorFromSignInOutAndErrorResponse()
					if initResult != nil {
						ESRM.LoggerManager.LogError(ESRM.ServiceName, "[initResult] Failed to handle error response: %v Exiting from 'FnRecieveThread' Loop", initResult)
						ESRM.ThreadExitStatus = util.EXIT
						*ESRM.ErrorResponseTrigger <- util.RCV_ERR
					}
				} else {
					*ESRM.ActualResponseTrigger <- util.LOGON_RESP_RCVD
				}
			}
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] SIGN_ON_REQUEST_OUT ENDED")

		case util.SIGN_OFF_REQUEST_OUT:

		case util.SYSTEM_INFORMATION_OUT:

			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] SYSTEM_INFORMATION_OUT STARTED")
			err := ESRM.RecieveManager.FnSystemInformationOut()
			if err != 0 {
				ESRM.ThreadExitStatus = util.EXIT
				*ESRM.ErrorResponseTrigger <- util.RCV_ERR
			} else {
				*ESRM.ActualResponseTrigger <- util.LOGON_RESP_RCVD
			}
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] SYSTEM_INFORMATION_OUT ENDED")

		case util.PARTIAL_SYSTEM_INFORMATION:
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] PARTIAL_SYSTEM_INFORMATION STARTED ")
			err := ESRM.RecieveManager.FnPartialLogonRes()
			if err != 0 {
				ESRM.LoggerManager.LogError(ESRM.ServiceName, "Failed In Function 'FnPartialLogonRes' ")
			}
			ESRM.ThreadExitStatus = util.EXIT
			*ESRM.ErrorResponseTrigger <- util.RCV_ERR
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] PARTIAL_SYSTEM_INFORMATION ENDED ")

		case util.UPDATE_LOCALDB_HEADER:
			ESRM.RecievedStatus = "NO"
		case util.UPDATE_LOCALDB_TRAILER:
			*ESRM.ActualResponseTrigger <- util.LDB_RESP_RCVD
		case util.UPDATE_LOCALDB_DATA:

		default:
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Unknown transaction code received: %d", intHdr.Si_transaction_code)
		}
	}
}

func (ESRM *ESRManager) HandleErrorFromSignInOutAndErrorResponse() error {

	ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "Recived error from 'fnSignOnIN' or 'FnErrorResponse' ")

	newExgPwdQuery := `
    SELECT 
        COALESCE(OPM_NEW_EXG_PWD, 'NA') AS new_exg_pwd
    FROM OPM_ORD_PIPE_MSTR
    WHERE OPM_XCHNG_CD = ? 
      AND OPM_PIPE_ID = ?
`

	ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[HandleError] Starting error handling logic for exchange code: %s and pipe ID: %s", ESRM.OPM_ExchangeCode, ESRM.OPM_PipeID)

	var cNewExgPwd string
	row := ESRM.DB.Raw(newExgPwdQuery, ESRM.OPM_ExchangeCode, ESRM.OPM_PipeID).Row()
	if err := row.Scan(&cNewExgPwd); err != nil {
		ESRM.ChanValue = util.RCV_ERR
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[HandleError] Error fetching new exchange password: %v", err)
		return fmt.Errorf("failed to fetch new exchange password: %w", err)
	}
	if cNewExgPwd == "NA" {
		ESRM.LoggerManager.LogError(ESRM.ServiceName, "[HandleError] No new exchange password available for exchange code: %s and pipe ID: %s", ESRM.OPM_ExchangeCode, ESRM.OPM_PipeID)

		beginTranResult := ESRM.TransactionManager.FnBeginTran()

		if beginTranResult != 0 {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[HandleError] Failed to begin transaction. FnBeginTran returned: %d", beginTranResult)
			ESRM.ChanValue = util.RCV_ERR
			return fmt.Errorf("failed to begin transaction: FnBeginTran returned %d", beginTranResult)
		}
		updateNewExgPwdQuery := `
		UPDATE OPM_ORD_PIPE_MSTR
		SET OPM_NEW_EXG_PWD = NULL
		WHERE OPM_XCHNG_CD = ? 
		AND OPM_PIPE_ID = ?
		`
		result := ESRM.DB.Exec(updateNewExgPwdQuery, ESRM.OPM_ExchangeCode, ESRM.OPM_PipeID)
		if result.Error != nil {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[HandleError] Error resetting exchange password: %v", result.Error)
			abortTranResult := ESRM.TransactionManager.FnAbortTran()
			if abortTranResult != 0 {
				ESRM.LoggerManager.LogError(ESRM.ServiceName, "[HandleError] Failed to abort transaction : %v", result.Error)
				ESRM.ChanValue = util.RCV_ERR
				return fmt.Errorf("[HandleError] Error: Unable to abort transaction , FnAbortTran returned: %d", abortTranResult)
			}
			ESRM.ChanValue = util.RCV_ERR
			return fmt.Errorf("failed to reset exchange password: %w", result.Error)
		}

		commitTranResult := ESRM.TransactionManager.FnCommitTran()
		if commitTranResult != 0 {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[HandleError] Failed to commit transaction. FnCommitTran returned: %d", commitTranResult)
			abortTranResult := ESRM.TransactionManager.FnAbortTran()
			if abortTranResult != 0 {
				ESRM.LoggerManager.LogError(ESRM.ServiceName, "[HandleError] Failed to abort transaction after commit failure. FnAbortTran returned: %d", abortTranResult)
				ESRM.ChanValue = util.RCV_ERR
				return fmt.Errorf("[HandleError] Error: Unable to abort transaction after commit failure, FnAbortTran returned: %d", abortTranResult)

			}
			ESRM.ChanValue = util.RCV_ERR
			return fmt.Errorf("[HandleError] Failed to commit transaction , Transaction RollBack ")
		}
		ESRM.ChanValue = util.RCV_ERR
		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[HandleError] Transaction committed successfully")

	} else {
		ESRM.ChanValue = util.RCV_ERR
		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[HandleError] Fetched new exchange password successfully: %s", cNewExgPwd)
	}

	return nil
}
