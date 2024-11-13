package esr

import (
	//"bytes"
	//"encoding/binary"

	recvhandler "DATA_FWD_TAP/internal/app/exchange_service/conn_send_recv_handler/recv_handler"
	sendhandler "DATA_FWD_TAP/internal/app/exchange_service/conn_send_recv_handler/send_handler"
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"DATA_FWD_TAP/util/MessageQueue"
	socket "DATA_FWD_TAP/util/TapSocket"
	typeconversionutil "DATA_FWD_TAP/util/TypeConversionUtil"
	"fmt"
	"time"

	"unsafe"

	"gorm.io/gorm"
)

type ESRManager struct {
	Req_q_data         *models.St_req_q_data
	ServiceName        string
	St_exch_msg        *models.St_exch_msg
	St_net_hdr         *models.St_net_hdr
	St_oe_reqres       *models.St_oe_reqres
	St_sign_on_req     *models.St_sign_on_req
	St_exch_msg_Log_on *models.St_exch_msg_Log_On
	St_exch_msg_resp   *models.St_exch_msg_resp

	ENVM                  *util.EnvironmentManager
	Req_q_data1           models.St_req_q_data
	PUM                   *util.PasswordUtilManger
	Message_queue_manager *MessageQueue.MessageQueueManager
	TCUM                  *typeconversionutil.TypeConversionUtilManager
	LoggerManager         *util.LoggerManager
	TM                    *util.TransactionManager
	SCM                   *socket.SocketManager
	RM                    *recvhandler.RecvManager
	Max_Pack_Val          int
	SHM                   *sendhandler.SendManager
	Db                    *gorm.DB
	InitialQId            *int
	GlobalQId             *int
	IP                    string
	Port                  string
	AutoReconnect         *bool
}

var Sequence_number int32 = 1
var RecieveTrigger = make(chan int)

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

		// Check if R_L_msg_type is "2000"
		if R_L_msg_type == util.LOGIN_WITHOUT_OPEN_ORDR_DTLS {
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[send_thrd] Message Type is LOGIN")

			if err := ESRM.SHM.FnDoXchngLogOn(receivedexchngMsg); err != nil {
				ESRM.LoggerManager.LogError(ESRM.ServiceName, fmt.Sprintf("[send_thrd] Failed to send exch_msg_data: %v", err))
				return
			}

		} else if R_L_msg_type == util.BOARD_LOT_IN {
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[send_thrd] Message type is BOARD LOT IN")

			li_business_data_size := int64(unsafe.Sizeof(models.St_oe_reqres{}))
			li_send_tap_msg_size := int64(len(receivedexchngMsg))
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, fmt.Sprintf("[send_thrd] Business Data Size: %v", li_business_data_size))
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, fmt.Sprintf("[send_thrd] TAP message size: %v", li_send_tap_msg_size))

			// Call fn_writen to write exch_msg_data to TCP connection
			if err := ESRM.SCM.WriteOnTapSocket(receivedexchngMsg); err != nil {
				ESRM.LoggerManager.LogError(ESRM.ServiceName, fmt.Sprintf("[%s] [send_thrd] Failed to send exch_msg_data: %v", ESRM.ServiceName, err))
				return
			}

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
		receivedResult, err := ESRM.SCM.ReadFromTapSocket(1024)
		if err != nil {
			ESRM.LoggerManager.LogError(ESRM.ServiceName, "[FnRecieveThread] Error while reading from socket: %v", err)
			break
		}

		// Step 3: Log after reading from the socket
		formattedTime = time.Now().Format("2006-01-02 15:04:05")
		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[FnRecieveThread] ESR TIME_STATS After Reading From Socket %s", formattedTime)

		//Step 4: TAP validation ,  meaning valiating the checksum and sequence number
		initResult := ESRM.RM.FnValidateTap(&Sequence_number, receivedResult)
		if initResult != 0 {
			break // from for loop
		}

		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "Validation Successfull")

		// After the Sucessfull validation we need to identify the what we have recieved
		// this will be in following steps
		// step 1: Extract the header from the buffer
		// step 2: Based on the tranactionCode we are going to extract .

	}
}
