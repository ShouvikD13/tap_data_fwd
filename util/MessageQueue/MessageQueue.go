package MessageQueue

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -lmessagequeue
#include "message_queue.h"
*/
import "C"
import (
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	MessageStat "DATA_FWD_TAP/util/MessageStats"
	"unsafe"

	"github.com/Shopify/sysv_mq"
)

type MessageQueueManager struct {
	ServiceName        string
	Req_q_data         models.St_req_q_data
	Req_q_data_Log_On  models.St_req_q_data_Log_On
	Req_q_data_Log_Off models.St_req_q_data_Log_Off
	mq                 *sysv_mq.MessageQueue
	LoggerManager      *util.LoggerManager
	MSM                *MessageStat.MessageStatManager
}

func NewMessageQueueManager(ServiceName string, mq *sysv_mq.MessageQueue) {

}

type WriteQueueMessage interface {
	GetMessageData() (int64, []byte)
}

func (MQM *MessageQueueManager) CreateQueue(Initial_Q_Id int) int {

	if initResult := C.create_message_queue(&Initial_Q_Id); initResult != -1 {
		MQM.LoggerManager.LogInfo(MQM.ServiceName, "[CreateQueue] Created the Message Queue SuccessFully With QueueId Provided by system : %d ", initResult)
		return initResult
	} else {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[CreateQueue] Failed to create the message Queue. Returned with error code : %d ", initResult)
		return initResult
	}

	// this funtion should call the function 'int create_message_queue(int *queue_id) ' of MessageQueue.c
}

func (MQM *MessageQueueManager) WriteToQueue(GlobalQId int, message WriteQueueMessage) int {
	switch msg := message.(type) {
	case *models.St_req_q_data:
		var boardLotMsg C.message_board_lot_in
		boardLotMsg.li_msg = C.int64_t(msg.L_msg_type)

		if len(msg.St_exch_msg_data) > len(boardLotMsg.board_lot_in) {
			MQM.LoggerManager.LogError(MQM.ServiceName, "[WriteToQueue] St_exch_msg_data exceeds buffer size")
			return -1
		}

		for i := 0; i < len(msg.St_exch_msg_data); i++ {
			boardLotMsg.board_lot_in[i] = C.char(msg.St_exch_msg_data[i])
		}

		initResult := C.send_board_lot_message(C.int(GlobalQId), &boardLotMsg)
		if initResult != 0 {
			MQM.LoggerManager.LogError(MQM.ServiceName, "[WriteToQueue] Failed to Write to Queue. Received error from `send_board_lot_message()`")
			return initResult
		} else {
			MQM.LoggerManager.LogInfo(MQM.ServiceName, "[WriteToQueue] Message sent successfully")
		}

	case *models.St_req_q_data_Log_On:
		var logOnMsg C.message_log_on
		logOnMsg.li_msg = C.int64_t(msg.L_msg_type)

		if len(msg.St_exch_msg_Log_On) > len(logOnMsg.log_on) {
			MQM.LoggerManager.LogError(MQM.ServiceName, "[WriteToQueue] St_exch_msg_Log_On exceeds buffer size")
			return -1
		}

		for i := 0; i < len(msg.St_exch_msg_Log_On); i++ {
			logOnMsg.log_on[i] = C.char(msg.St_exch_msg_Log_On[i])
		}

		initResult := C.send_log_on_message(C.int(GlobalQId), &logOnMsg)
		if initResult != 0 {
			MQM.LoggerManager.LogError(MQM.ServiceName, "[WriteToQueue] Failed to Write to Queue. Received error from `send_log_on_message()`")
			return initResult
		} else {
			MQM.LoggerManager.LogInfo(MQM.ServiceName, "[WriteToQueue] Log On message sent successfully")
		}

	case *models.St_req_q_data_Log_Off:
		var logOffMsg C.message_log_off
		logOffMsg.li_msg = C.int64_t(msg.L_msg_type)

		if len(msg.St_exch_msg_Log_Off) > len(logOffMsg.log_off) {
			MQM.LoggerManager.LogError(MQM.ServiceName, "[WriteToQueue] St_exch_msg_Log_Off exceeds buffer size")
			return -1
		}

		for i := 0; i < len(msg.St_exch_msg_Log_Off); i++ {
			logOffMsg.log_off[i] = C.char(msg.St_exch_msg_Log_Off[i])
		}

		initResult := C.send_log_off_message(C.int(GlobalQId), &logOffMsg)
		if initResult != 0 {
			MQM.LoggerManager.LogError(MQM.ServiceName, "[WriteToQueue] Failed to Write to Queue. Received error from `send_log_off_message()`")
			return initResult
		} else {
			MQM.LoggerManager.LogInfo(MQM.ServiceName, "[WriteToQueue] Log Off message sent successfully")
		}

	default:
		MQM.LoggerManager.LogError(MQM.ServiceName, "[WriteToQueue] Unknown Structure Type to be a message")
		return -1
	}

	return 0

	// This function should call the 'int send_message_to_queue(int queue_id, void *message, long message_length)'
}

func (MQM *MessageQueueManager) ReadFromQueue(GlobalQId int) (int64, []byte, int) {
	size := 400
	buffer := make([]byte, size)

	result := C.receive_message_from_queue(C.int(GlobalQId), unsafe.Pointer(&buffer[0]), C.int(size))

	if result != 0 {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[Error Received from `receive_message_from_queue`, Exiting ]")
		return 0, nil, -1
	}

	if len(buffer) < 8 {
		MQM.LoggerManager.LogError(MQM.ServiceName, "Received buffer is too small to contain li_msg")
		return 0, nil, -1
	}
	liMsg := *(*int64)(unsafe.Pointer(&buffer[0]))

	switch liMsg {

	case util.BOARD_LOT_IN:
		expectedSize := 8 + 338
		if len(buffer) < expectedSize {
			MQM.LoggerManager.LogError(MQM.ServiceName, "[ReadFromQueue] Buffer size is smaller than expected for board_lot_in")
			return 0, nil, -1
		}

		var reqData models.St_req_q_data
		reqData.L_msg_type = liMsg
		copy(reqData.St_exch_msg_data[:], buffer[8:346]) // 8 + 338 = 346

		MQM.LoggerManager.LogInfo(MQM.ServiceName, "[ReadFromQueue] board_lot_in message received successfully")
		return reqData.L_msg_type, reqData.St_exch_msg_data[:], 0

	case util.LOGIN_WITHOUT_OPEN_ORDR_DTLS:
		expectedSize := 8 + 300
		if len(buffer) < expectedSize {
			MQM.LoggerManager.LogError(MQM.ServiceName, "[ReadFromQueue] Buffer size is smaller than expected for log_on")
			return 0, nil, -1
		}

		var logOnData models.St_req_q_data_Log_On
		logOnData.L_msg_type = liMsg
		copy(logOnData.St_exch_msg_Log_On[:], buffer[8:308]) // 8 + 300 = 308

		MQM.LoggerManager.LogInfo(MQM.ServiceName, "[ReadFromQueue] log_on message received successfully")
		return logOnData.L_msg_type, logOnData.St_exch_msg_Log_On[:], 0

	case util.SIGN_OFF_REQUEST_IN:
		expectedSize := 8 + 62
		if len(buffer) < expectedSize {
			MQM.LoggerManager.LogError(MQM.ServiceName, "[ReadFromQueue] Buffer size is smaller than expected for log_off")
			return 0, nil, -1
		}

		var logOffData models.St_req_q_data_Log_Off
		logOffData.L_msg_type = liMsg
		copy(logOffData.St_exch_msg_Log_Off[:], buffer[8:70]) // 8 + 62 = 70

		MQM.LoggerManager.LogInfo(MQM.ServiceName, "[ReadFromQueue] log_off message received successfully")
		return logOffData.L_msg_type, logOffData.St_exch_msg_Log_Off[:], 0

	default:
		MQM.LoggerManager.LogError(MQM.ServiceName, "Received message of unknown type or incorrect size")
		return 0, nil, -1
	}

	// This function should call the 'int send_message_to_queue(int queue_id, void *message, long message_length)'
}

func (MQM *MessageQueueManager) DestroyQueue(GlobalQId int) int {

	initResult := destroy_message_queue(C.int(GlobalQId))
	if initResult != 0 {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[destroyQueue] [Error:  failed to destroy message queue: ")
		return -1
	}

	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[destroyQueue] Queue Destroyed successFully ")
	return 0

}

func (MQM *MessageQueueManager) FnCanWriteToQueue(GlobalQId int) int {
	var MessageNumber C.unsigned_int

	initResult := C.get_queue_message_count(C.int(GlobalQId), &MessageNumber)
	if initResult != 0 {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[canWriteToQueue] [Error: failed to get message queue stats]")
		return -1
	}

	if MessageNumber < 10 { // Here, I am setting the maximum number of messages that can be present in the queue.
		return 0
	}

	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[canWriteToQueue] Queue is full, current message count: %d", MessageNumber)
	return -1
}

func (MQM *MessageQueueManager) FnFlushQueue(GlobalQId int) int {
	if initResult := C.flush_message_queue(C.int(GlobalQId)); initResult != 0 {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[FnFlushQueue] [Error: failed to flush the queue]")
		return -1
	}

	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[FnFlushQueue] Successfully flushed the Message Queue...")
	return 0
}

// gcc -c -fPIC message_queue.c -o message_queue.o

// gcc -shared -o libmylib.so mylib.o