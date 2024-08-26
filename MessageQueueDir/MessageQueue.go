package MessageQueueDir

import (
	"bytes"
	"encoding/gob"

	"log"

	"DATA_FWD_TAP/models/structures"

	"github.com/Shopify/sysv_mq"
)

type MessageQueueManager struct {
	ServiceName string
	Req_q_data  *structures.St_req_q_data
	// Result_req_q_data *structures.St_req_q_data
	mq *sysv_mq.MessageQueue
}

func (MQM *MessageQueueManager) serialize(data *structures.St_req_q_data) ([]byte, error) {
	var buf bytes.Buffer
	enc := gob.NewEncoder(&buf)
	if err := enc.Encode(data); err != nil {
		log.Printf("[%s] [deserialize] [Error] failed to deserialize data: %v", MQM.ServiceName, err)
		return nil, err
	}
	return buf.Bytes(), nil
}

func (MQM *MessageQueueManager) deserialize(data []byte) (*structures.St_req_q_data, error) {
	var result structures.St_req_q_data
	buf := bytes.NewBuffer(data)
	dec := gob.NewDecoder(buf)
	if err := dec.Decode(&result); err != nil {
		log.Printf("[%s] [deserialize] [Error] failed to deserialize data: %v", MQM.ServiceName, err)
		return nil, err
	}
	return &result, nil
}

func (MQM *MessageQueueManager) CreateQueue(key int) int {
	mq, err := sysv_mq.NewMessageQueue(&sysv_mq.QueueConfig{
		Key:     key,                        // SysV IPC key
		MaxSize: 1024,                       // Max size of a message
		Mode:    (sysv_mq.IPC_CREAT | 0777), //| (sysv_mq.IPC_EXCL | 0777), // Create queue if not exists with 0777 permissions
	})
	if err != nil {
		log.Printf("[%s] [createQueue] [Error] failed to Create Message Queue: %v", MQM.ServiceName, err)
		return -1
	}
	MQM.mq = mq
	return 0
}

func (MQM *MessageQueueManager) WriteToQueue(mtype int) int {
	serializedData, err := MQM.serialize(MQM.Req_q_data)
	if err != nil {
		log.Printf("[%s] [writeToQueue] [Error] failed to serialize data: %v", MQM.ServiceName, err)
		return -1
	}

	err = MQM.mq.SendBytes(serializedData[:512], mtype, 0)
	if err != nil {
		log.Printf("[%s] [writeToQueue] [Error] failed to send message: %v", MQM.ServiceName, err)
		return -1
	}
	return 0
}

func (MQM *MessageQueueManager) ReadFromQueue(mtype int) (*structures.St_req_q_data, int64, int) {
	receivedData, receivedType, err := MQM.mq.ReceiveBytes(mtype, 0) // Added 0 for flags
	if err != nil {
		log.Printf("[%s] [readFromQueue] [Error] failed to receive message: %v", MQM.ServiceName, err)
		return nil, 0, -1
	}

	deserializedData, err := MQM.deserialize(receivedData)
	if err != nil {
		log.Printf("[%s] [readFromQueue] [Error] failed to deserialize data: %v", MQM.ServiceName, err)
		return nil, 0, -1
	}

	// Convert receivedType to int64 for return
	return deserializedData, int64(receivedType), 0
}

func (MQM *MessageQueueManager) DestroyQueue() int {
	err := MQM.mq.Destroy()
	if err != nil {
		log.Printf("[%s] [destroyQueue] [Error] failed to destroy message queue: %v", MQM.ServiceName, err)
		return -1
	}
	return 0
}
