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
	"bytes"
	"encoding/binary"

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

// func (MQM *MessageQueueManager) pack(message WriteQueueMessage) ([]byte, error) {
// 	var buf bytes.Buffer

// 	if err := binary.Write(&buf, binary.LittleEndian, message.GetMsgType()); err != nil {
// 		return nil, err
// 	}

// 	// Packind Second value of the structure
// 	switch data := message.(type) {
// 	case *models.St_req_q_data:
// 		if err := binary.Write(&buf, binary.LittleEndian, data.St_exch_msg_data); err != nil {
// 			return nil, err
// 		}
// 	case *models.St_req_q_data_Log_On:
// 		if err := binary.Write(&buf, binary.LittleEndian, data.St_exch_msg_Log_On); err != nil {
// 			return nil, err
// 		}
// 	case *models.St_req_q_data_Log_Off:
// 		if err := binary.Write(&buf, binary.LittleEndian, data.St_exch_msg_Log_Off); err != nil {
// 			return nil, err
// 		}
// 	default:
// 		return nil, fmt.Errorf("unsupported message type")
// 	}

// 	return buf.Bytes(), nil
// }


func (MQM *MessageQueueManager) CreateQueue(key int) int {
	// Initialize MessageStatManager instance and assign it to MQM.MSM
	// This sets up the MessageStatManager for managing message queue statistics
	mq, err := sysv_mq.NewMessageQueue(&sysv_mq.QueueConfig{
		Key:     key,
		MaxSize: 1024 * 3,
		Mode:    (sysv_mq.IPC_CREAT | 0777),
	})
	if err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[CreateQueue] [Error: failed to Create Message Queue: %v", err)
		return -1
	}
	MQM.mq = mq
	return 0

	// this funtion should call the function 'int create_message_queue(int *queue_id) ' of MessageQueue.c
}

func (MQM *MessageQueueManager) WriteToQueue(MqKey int, message WriteQueueMessage) int {

	switch msg := message.(type) {

	case *models.St_req_q_data:

		var boardLotMsg C.message_board_lot_in
		boardLotMsg.li_msg = C.int64_t(msg.L_msg_type)

		for i := 0; i < len(msg.St_exch_msg_data); i++ {
			boardLotMsg.board_lot_in[i] = C.char(msg.St_exch_msg_data[i])
		}

		C.fn_write_msg_q(C.int(MqKey), &boardLotMsg, C.long(C.sizeof_message_board_lot_in))

	case *models.St_req_q_data_Log_On:

		var logOnMsg C.message_log_on
		logOnMsg.li_msg = C.int64_t(msg.L_msg_type)

		for i := 0; i < len(msg.St_exch_msg_Log_On); i++ {
			logOnMsg.log_on[i] = C.char(msg.St_exch_msg_Log_On[i])
		}

		C.fn_write_msg_q(C.int(MqKey), &logOnMsg, C.long(C.sizeof_message_log_on))

	case *models.St_req_q_data_Log_Off:

		var logOffMsg C.message_log_off
		logOffMsg.li_msg = C.int64_t(msg.L_msg_type)

		for i := 0; i < len(msg.St_exch_msg_Log_Off); i++ {
			logOffMsg.log_off[i] = C.char(msg.St_exch_msg_Log_Off[i])
		}

		C.fn_write_msg_q(C.int(MqKey), &logOffMsg, C.long(C.sizeof_message_log_off))

	default:
		MQM.LoggerManager.LogError(MQM.ServiceName, "[WriteToQueue] Unknown Structure Type to be a message")
		return -1
	}

	return 0

	// packedData, err := MQM.pack(message)
	// if err != nil {
	// 	MQM.LoggerManager.LogError(MQM.ServiceName, "[writeToQueue] [Error: failed to pack data in Message Queue: %v", err)
	// 	return -1
	// }

	// MQM.LoggerManager.LogInfo(MQM.ServiceName, "[writeToQueue] Packed data size: %d bytes", len(packedData))

	// err = MQM.mq.SendBytes(packedData, mId, 0)
	// if err != nil {
	// 	MQM.LoggerManager.LogError(MQM.ServiceName, "[writeToQueue] [Error: failed to send message in Message Queue: %v", err)
	// 	return -1
	// }
	// return 0

	// This function should call the 'int send_message_to_queue(int queue_id, void *message, long message_length)'
}

func (MQM *MessageQueueManager) ReadFromQueue(mId int) (int64, []byte, int) {

	receivedData, receivedType, err := MQM.mq.ReceiveBytes(mId, 0)
	if err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[readFromQueue] [Error: failed to receive message from Message Queue: %v", err)
		return 0, nil, -1
	}

	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[readFromQueue] Received message with type: %d", receivedType)
	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[readFromQueue] Length of Received Data: %d", len(receivedData))

	var Li_msg_type int64
	buf := bytes.NewReader(receivedData)

	if err := binary.Read(buf, binary.LittleEndian, &Li_msg_type); err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[readFromQueue] [Error: failed to decode Li_msg_type: %v", err)
		return 0, nil, -1
	}

	byteArray := make([]byte, buf.Len())
	if err := binary.Read(buf, binary.LittleEndian, &byteArray); err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[readFromQueue] [Error: failed to decode byte array: %v", err)
		return 0, nil, -1
	}

	return Li_msg_type, byteArray, 0

	// this function should call 'receive_message_from_queue(int queue_id, void *buffer, long *buffer_length)'
}

func (MQM *MessageQueueManager) DestroyQueue() int {
	err := MQM.mq.Destroy()
	if err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[destroyQueue] [Error:  failed to destroy message queue: %v", err)
		return -1
	}
	return 0
	// this function should call 'void destroy_message_queue(int queue_id)'
}

func (MQM *MessageQueueManager) FnCanWriteToQueue() int {

	key := util.ORDINARY_ORDER_QUEUE_ID

	// Call GetMsgQueueStats to get the message queue stats
	stats, err := MQM.MSM.GetMsgQueueStats(key)
	if err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[canWriteToQueue] [Error: failed to get message queue stats: %v", err)
		return -1
	}

	if stats.MsgQnum < 10 { //Here, I am setting the maximum number of messages that can be present in the queue. Based on my system, this number is 10.
		return 0
	}

	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[canWriteToQueue] Queue is full, current message count: %d", stats.MsgQnum)
	return -1

	// This function should call 'get_queue_message_count(int queue_id, unsigned int *message_count)'
}

func (MQM *MessageQueueManager) FnFlushQueue() int {
	return 0

	// This Function should call 'int flush_message_queue(int queue_id) '
}

// gcc -c -fPIC message_queue.c -o message_queue.o

// gcc -shared -o libmylib.so mylib.o
