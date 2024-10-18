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
}

func NewMessageQueueManager(ServiceName string, LoggerManager *util.LoggerManager) *MessageQueueManager {
	return &MessageQueueManager{
		ServiceName:   ServiceName,
		LoggerManager: LoggerManager,
	}
}

type WriteQueueMessage interface {
	GetMessageData() (int64, []byte)
}

func (MQM *MessageQueueManager) CreateQueue(initial_QId *int) int {

	initialQId := *initial_QId
	cInitialQId := C.int(initialQId)
	ptr := (*C.int)(C.malloc(C.size_t(unsafe.Sizeof(cInitialQId))))
	if ptr == nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[CreateQueue] Failed to allocate memory for initialQueueId")
		return -1
	}
	defer C.free(unsafe.Pointer(ptr))

	*ptr = cInitialQId

	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[CreateQueue] Initial queue ID (constant): %d", initialQId)
	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[CreateQueue] Address of Initial queue ID (from Go): %p, Address passed to C: %p", &cInitialQId, ptr)

	initResult := C.create_message_queue(ptr)

	if initResult != -1 {
		MQM.LoggerManager.LogInfo(MQM.ServiceName, "[CreateQueue] Created the Message Queue Successfully With QueueId Provided by system : %d ", int(initResult))
		return int(initResult)
	} else {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[CreateQueue] Failed to create the message Queue. Returned with error code : %d ", int(initResult))
		return -1
	}
}

func (MQM *MessageQueueManager) WriteToQueue(GlobalQId int, message WriteQueueMessage) int {
	switch msg := message.(type) {
	case *models.St_req_q_data:
		var boardLotMsg C.struct_message_board_lot_in
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
			return int(initResult)
		} else {
			MQM.LoggerManager.LogInfo(MQM.ServiceName, "[WriteToQueue] Message sent successfully")
		}

	case *models.St_req_q_data_Log_On:
		var logOnMsg C.struct_message_log_on
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
			return int(initResult)
		} else {
			MQM.LoggerManager.LogInfo(MQM.ServiceName, "[WriteToQueue] Log On message sent successfully")
		}

	case *models.St_req_q_data_Log_Off:
		var logOffMsg C.struct_message_log_off
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
			return int(initResult)
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
	size := C.long(400)
	buffer := make([]byte, size)

	result := C.receive_message_from_queue(C.int(GlobalQId), unsafe.Pointer(&buffer[0]), &size)

	if result != 0 {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[Error Received from `receive_message_from_queue`, Exiting ]")
		return 0, nil, -1
	}

	bufferSize := len(buffer)
	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[ReadFromQueue] Buffer size after receiving data: %d", bufferSize)

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
		copiedSize := copy(reqData.St_exch_msg_data[:], buffer[8:346]) // 8 + 338 = 346
		MQM.LoggerManager.LogInfo(MQM.ServiceName, "[ReadFromQueue] Copied %d bytes from buffer[8:346]", copiedSize)
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
		copiedSize := copy(logOnData.St_exch_msg_Log_On[:], buffer[8:308]) // 8 + 300 = 308
		MQM.LoggerManager.LogInfo(MQM.ServiceName, "[ReadFromQueue] Copied %d bytes from buffer[8:308]", copiedSize)

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
		copiedSize := copy(logOffData.St_exch_msg_Log_Off[:], buffer[8:70]) // 8 + 62 = 70
		MQM.LoggerManager.LogInfo(MQM.ServiceName, "[ReadFromQueue] Copied %d bytes from buffer[8:70]", copiedSize)

		MQM.LoggerManager.LogInfo(MQM.ServiceName, "[ReadFromQueue] log_off message received successfully")
		return logOffData.L_msg_type, logOffData.St_exch_msg_Log_Off[:], 0

	default:
		MQM.LoggerManager.LogError(MQM.ServiceName, "Received message of unknown type or incorrect size")
		return 0, nil, -1
	}

	// This function should call the 'int send_message_to_queue(int queue_id, void *message, long message_length)'
}

func (MQM *MessageQueueManager) DestroyQueue(GlobalQId int) int {

	initResult := C.destroy_message_queue(C.int(GlobalQId))
	if initResult != 0 {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[destroyQueue] [Error:  failed to destroy message queue: ")
		return -1
	}

	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[destroyQueue] Queue Destroyed successFully ")
	return 0

}

func (MQM *MessageQueueManager) FnCanWriteToQueue(GlobalQId int) int {
	var messageNumber C.ulong

	initResult := C.get_queue_message_count(C.int(GlobalQId), &messageNumber)
	if initResult != 0 {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[canWriteToQueue] [Error: failed to get message queue stats]")
		return -1
	}

	if uint(messageNumber) < 10 { // Here, I am setting the maximum number of messages that can be present in the queue.
		return 0
	}

	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[canWriteToQueue] Queue is full, current message count: %d", uint(messageNumber))
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

// # Compile Log.c into an object file
// gcc -c -fPIC Log.c -o Log.o

// # Compile message_queue.c into an object file
// gcc -c -fPIC message_queue.c -o message_queue.o

// # Create the shared library (libmessagequeue.so)
// gcc -shared -o libmessagequeue.so Log.o message_queue.o
