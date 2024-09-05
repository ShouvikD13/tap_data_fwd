package MessageQueueDir

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"reflect"
	"unsafe"

	"DATA_FWD_TAP/models/structures"

	"github.com/Shopify/sysv_mq"
)

type MessageQueueManager struct {
	ServiceName string
	Req_q_data  structures.St_req_q_data
	mq          *sysv_mq.MessageQueue
}

func (MQM *MessageQueueManager) pack(data structures.St_req_q_data) ([]byte, error) {
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
func (MQM *MessageQueueManager) unpack(data []byte) (structures.St_req_q_data, error) {
	var result structures.St_req_q_data
	err := UnpackStruct(data, &result)
	if err != nil {
		return result, err
	}
	return result, nil
}

// UnpackStruct unpacks a byte slice into the struct v, including nested structs.
func UnpackStruct(data []byte, v interface{}) error {
	buf := bytes.NewReader(data)
	val := reflect.ValueOf(v).Elem()

	if val.Kind() != reflect.Struct {
		return fmt.Errorf("expected a pointer to a struct, got %s", val.Kind())
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
		log.Printf("[%s] [createQueue] [Error] failed to Create Message Queue: %v", MQM.ServiceName, err)
		return -1
	}
	MQM.mq = mq
	return 0
}

func (MQM *MessageQueueManager) WriteToQueue(mtype int) int {
	// log.Printf(" Data Before packing: %v", MQM.Req_q_data)
	log.Printf("[%s] [writeToQueue] size of 'MQM.Req_q_data': %d bytes", MQM.ServiceName, unsafe.Sizeof(MQM.Req_q_data))
	packedData, err := MQM.pack(MQM.Req_q_data)
	if err != nil {
		log.Printf("[%s] [writeToQueue] [Error] failed to pack data: %v", MQM.ServiceName, err)
		return -1
	}
	// log.Printf("Packed Data: %v", packedData) // <---------------------------------------
	log.Printf("[%s] [writeToQueue] Packed data size: %d bytes", MQM.ServiceName, len(packedData))

	err = MQM.mq.SendBytes(packedData, mtype, 0)
	if err != nil {
		log.Printf("[%s] [writeToQueue] [Error] failed to send message: %v", MQM.ServiceName, err)
		return -1
	}
	return 0
}

func (MQM *MessageQueueManager) ReadFromQueue(mtype int) (structures.St_req_q_data, int64, int) {
	receivedData, receivedType, err := MQM.mq.ReceiveBytes(mtype, 0)
	if err != nil {
		log.Printf("[%s] [readFromQueue] [Error] failed to receive message: %v", MQM.ServiceName, err)
		return structures.St_req_q_data{}, 0, -1
	}
	log.Printf("Received Data: %v", receivedData) // <---------------------------------------
	log.Printf("[%s] [readFromQueue] size of data before unpacking: %d bytes", MQM.ServiceName, len(receivedData))
	unpackedData, err := MQM.unpack(receivedData)
	if err != nil {
		log.Printf("[%s] [readFromQueue] [Error] failed to unpack data: %v", MQM.ServiceName, err)
		return structures.St_req_q_data{}, 0, -1
	}

	log.Printf("[%s] [readFromQueue] Size of data after unpacking: %d bytes", MQM.ServiceName, unsafe.Sizeof(unpackedData))
	return unpackedData, int64(receivedType), 0
}

func (MQM *MessageQueueManager) DestroyQueue() int {
	err := MQM.mq.Destroy()
	if err != nil {
		log.Printf("[%s] [destroyQueue] [Error] failed to destroy message queue: %v", MQM.ServiceName, err)
		return -1
	}
	return 0
}
