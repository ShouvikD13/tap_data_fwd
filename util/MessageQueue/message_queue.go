package MessageQueue

import (
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"bytes"
	"encoding/binary"
	"fmt"
	"reflect"
	"unsafe"

	"github.com/Shopify/sysv_mq"
)

type MessageQueueManager struct {
	ServiceName   string
	Req_q_data    models.St_req_q_data
	mq            *sysv_mq.MessageQueue
	LoggerManager *util.LoggerManager
}

func (MQM *MessageQueueManager) pack(data models.St_req_q_data) ([]byte, error) {
	var buf bytes.Buffer

	if err := binary.Write(&buf, binary.LittleEndian, data.L_msg_type); err != nil {
		return nil, err
	}
	if err := binary.Write(&buf, binary.LittleEndian, data.St_exch_msg_data); err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

// New unpack method using reflection
func (MQM *MessageQueueManager) unpack(data []byte) (models.St_req_q_data, error) {
	var result models.St_req_q_data
	err := MQM.UnpackStruct(data, &result)
	if err != nil {
		return result, err
	}
	return result, nil
}

// UnpackStruct unpacks a byte slice into the struct v, including nested structs.
func (MQM *MessageQueueManager) UnpackStruct(data []byte, v interface{}) error {
	buf := bytes.NewReader(data)
	val := reflect.ValueOf(v).Elem()

	if val.Kind() != reflect.Struct {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[Message_queue] [UnpackStruct] Error: expected a pointer to a struct, got %v", val.Kind())
		return fmt.Errorf("[Message_queue] [UnpackStruct] Error: expected a pointer to a struct, got %v", val.Kind())
	}

	for i := 0; i < val.NumField(); i++ {
		field := val.Field(i)

		if field.Kind() == reflect.Ptr {
			// Allocate memory for the pointed-to struct and unpack
			subStruct := reflect.New(field.Type().Elem()).Elem()
			if err := UnpackPointerField(buf, subStruct.Addr().Interface()); err != nil {
				return err
			}
			field.Set(subStruct.Addr())
		} else {
			if err := binary.Read(buf, binary.LittleEndian, field.Addr().Interface()); err != nil {
				return err
			}
		}
	}

	return nil
}

// UnpackPointerField is used to unpack a struct pointed to by a field.
func UnpackPointerField(buf *bytes.Reader, field interface{}) error {
	val := reflect.ValueOf(field).Elem()
	for i := 0; i < val.NumField(); i++ {
		subField := val.Field(i)
		if err := binary.Read(buf, binary.LittleEndian, subField.Addr().Interface()); err != nil {
			return err
		}
	}
	return nil
}

func (MQM *MessageQueueManager) CreateQueue(key int) int {
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
}

func (MQM *MessageQueueManager) WriteToQueue(mtype int) int {
	packedData, err := MQM.pack(MQM.Req_q_data)
	if err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[writeToQueue] [Error: failed to pack data in Message Queue: %v", err)

		return -1
	}

	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[writeToQueue] Packed data size: %d bytes", len(packedData))

	err = MQM.mq.SendBytes(packedData, mtype, 0)
	if err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[writeToQueue] [Error: failed to send message in Message Queue: %v", err)
		return -1
	}
	return 0
}

func (MQM *MessageQueueManager) ReadFromQueue(mtype int) (models.St_req_q_data, int64, int) {
	receivedData, receivedType, err := MQM.mq.ReceiveBytes(mtype, 0)
	if err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[readFromQueue] [Error:  failed to receive message from Message Queue: %v", err)

		return models.St_req_q_data{}, 0, -1
	}

	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[readFromQueue] size of data before unpacking: %d bytes", unsafe.Sizeof(receivedData))
	unpackedData, err := MQM.unpack(receivedData)
	if err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[readFromQueue] [Error:  failed to unpack data from Message Queue: %v", err)
		return models.St_req_q_data{}, 0, -1
	}
	MQM.LoggerManager.LogInfo(MQM.ServiceName, "[readFromQueue] Size of data after unpacking: %d bytes", unsafe.Sizeof(unpackedData))

	return unpackedData, int64(receivedType), 0
}

func (MQM *MessageQueueManager) DestroyQueue() int {
	err := MQM.mq.Destroy()
	if err != nil {
		MQM.LoggerManager.LogError(MQM.ServiceName, "[destroyQueue] [Error:  failed to destroy message queue: %v", err)
		return -1
	}
	return 0
}