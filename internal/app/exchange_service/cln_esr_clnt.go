package esr

import (
	//"bytes"
	//"encoding/binary"

	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"DATA_FWD_TAP/util/MessageQueue"
	socket "DATA_FWD_TAP/util/TapSocket"
	typeconversionutil "DATA_FWD_TAP/util/TypeConversionUtil"
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"log"
	"net"

	"time"

	"unsafe"

	"gorm.io/gorm"
)

type ESRManager struct {
	Req_q_data             *models.St_req_q_data
	ServiceName            string
	St_exch_msg            *models.St_exch_msg
	St_net_hdr             *models.St_net_hdr
	St_oe_reqres           *models.St_oe_reqres
	St_sign_on_req         *models.St_sign_on_req
	St_exch_msg_Log_on     *models.St_exch_msg_Log_On
	St_exch_msg_Log_on_use *models.St_exch_msg_Log_On_use
	St_exch_msg_resp       *models.St_exch_msg_resp

	ENVM                  *util.EnvironmentManager
	Req_q_data1           models.St_req_q_data
	PUM                   *util.PasswordUtilManger
	Message_queue_manager *MessageQueue.MessageQueueManager
	TCUM                  *typeconversionutil.TypeConversionUtilManager
	LoggerManager         *util.LoggerManager
	TM                    *util.TransactionManager
	SCM                   *socket.SocketManager
	Max_Pack_Val          int
	Db                    *gorm.DB
	InitialQId            *int
	GlobalQId             *int
	IP                    string
	Port                  string
	AutoReconnect         *bool
}

// func (ESRM *ESRManager) Fn_crt_tap_con() (net.Conn, error) {

// 	// address := net.JoinHostPort(config.IP, config.Port)
// 	// log.Printf("After join host port")
// 	// conn, err := net.Dial("tcp", address)
// 	// log.Printf("after dialing to")
// 	// if err != nil {
// 	// 	log.Printf("Error connecting:", err)
// 	// 	if config.AutoReconnect {
// 	// 		log.Printf("Disconnected. Auto Reconnecting...")
// 	// 		return nil, nil
// 	// 	} else {
// 	// 		log.Printf("Auto Reconnect is disabled.")
// 	// 		return nil, err
// 	// 	}
// 	// }

// 	// log.Printf("Connected to TAP at", address)
// 	// return conn, nil
// }

func (ESRM *ESRManager) FnSendThread(conn net.Conn) {

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

			if err := ESRM.Do_xchng_logon(receivedexchngMsg); err != nil {
				ESRM.LoggerManager.LogError(ESRM.ServiceName, fmt.Sprintf("[%s] Failed to send exch_msg_data: %v", ESRM.ServiceName, err))
				return
			}

		} else if R_L_msg_type == util.BOARD_LOT_IN {
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "[send_thrd] Message type is BOARD LOT IN")

			li_business_data_size := int64(unsafe.Sizeof(models.St_oe_reqres{}))
			li_send_tap_msg_size := int64(len(receivedexchngMsg))
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, fmt.Sprintf("[send_thrd] Business Data Size: %v", li_business_data_size))
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, fmt.Sprintf("[send_thrd] TAP message size: %v", li_send_tap_msg_size))

			// Call fn_writen to write exch_msg_data to TCP connection
			if err := ESRM.fn_writen(conn, receivedexchngMsg, li_send_tap_msg_size); err != nil {
				ESRM.LoggerManager.LogError(ESRM.ServiceName, fmt.Sprintf("[%s] [send_thrd] Failed to send exch_msg_data: %v", ESRM.ServiceName, err))
				return
			}

		} else {
			ESRM.LoggerManager.LogInfo(ESRM.ServiceName, fmt.Sprintf("[%s] [send_thrd] Skipped message with message type: %d", ESRM.ServiceName, R_L_msg_type))
		}

		ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "Inside SEND Routine ENDS HERE")
	}

	ESRM.LoggerManager.LogInfo(ESRM.ServiceName, "Message queue closed, send thread exiting")
	exitcount += 1
}

func (ESRM *ESRManager) fn_writen(conn net.Conn, msg interface{}, msg_size int64) error {
	var buf bytes.Buffer

	// Get the size of the message
	msgSize := int(unsafe.Sizeof(msg))

	log.Printf("Mesage Size %d ", msgSize)

	// Use a switch to handle different message sizes
	switch msgSize {
	case 24:
		// Serialize St_exch_msg
		if err := binary.Write(&buf, binary.BigEndian, msg); err != nil {
			return err
		}
	case 314:
		// Serialize St_exch_msg_Log_On
		if err := binary.Write(&buf, binary.BigEndian, msg); err != nil {
			return err
		}
	default:
		//return errors.New("unsupported message type")
		if err := binary.Write(&buf, binary.BigEndian, msg); err != nil {
			return err
		}
	}
	dsize := buf.Len()

	log.Printf("Size of binary data: %v", dsize)

	data := buf.Bytes()

	// Restrict the message size if it exceeds the given msg_size
	if int64(len(data)) > msg_size {
		data = data[:msg_size]
	}

	totalWritten := 0

	for totalWritten < len(data) {
		log.Printf("[%s] [fn_writen]Attempting to write: totalWritten=%d, data_len=%d", totalWritten, len(data))
		written, err := conn.Write(data[totalWritten:])
		if err != nil {
			if ne, ok := err.(net.Error); ok && ne.Temporary() {
				log.Printf("Temporary network error, retrying...")
				time.Sleep(100 * time.Millisecond)
				continue
			}
			log.Printf("[%s] []Write failed: %v\n", err)
			return err
		}

		if written == 0 {
			// If no bytes are written, the connection might be stalled or closed.
			log.Printf("No bytes written; connection may be stalled or closed.")
			return net.ErrWriteToConnected
		}

		totalWritten += written

		log.Printf("[%s] [fn_writen]Written: %d bytes, Total Written: %d bytes", written, totalWritten)

		if totalWritten < len(data) {
			// Small delay to help with buffer recovery
			time.Sleep(50 * time.Millisecond)
		}
	}

	//log.Printf("[%s] []Successfully sent exch_msg: %+v", msg)
	log.Printf("[%s]Successfully sent exch_msg")
	return nil
}

func (ESRM *ESRManager) rcv_thrd(conn net.Conn) {
	log.Printf("Inside Rcv Routine STARTS HERE")
	defer wg.Done()

	for {
		// Read the message from the socket using readXchngSocket function
		msgData, err := ESRM.readXchngSocket(conn, 0) // Pass appropriate timeout here
		if err != nil {
			log.Printf("[rcv_thrd] Error reading from socket: %v", err)
			return
		}

		// Log the length of the received message
		log.Printf("[rcv_thrd] Received message of length: %d bytes", len(msgData))

		// Dynamically determine the structure based on the message length
		switch len(msgData) {
		case int(unsafe.Sizeof(models.St_exch_msg_Log_On{})):
			var tmp_buf_dat models.St_exch_msg_Log_On_use
			buf := bytes.NewBuffer(msgData)
			if err := binary.Read(buf, binary.BigEndian, &tmp_buf_dat); err != nil {
				log.Printf("[rcv_thrd] Failed to decode exch_msg_data: %v", err)
				return
			}
			log.Println("[rcv_thrd] Sign On response received")

			//Call_Do_xchng_logon to start the sign on process
			/* switch tmp_buf_dat.St_sign_on_req.St_hdr.Si_transaction_code {

				case util.SIGN_ON_REQUEST_OUT {
					_,err := ESRM.Fn_sign_on_request_out(tmp_buf_dat.St_sign_on_req); err != nil {
						log.Printf("[rcv_thrd] Failed to send exch_msg_data: %v", err)
						return nil
					}
				}

			 } */

		case int(unsafe.Sizeof(models.St_exch_msg{})):
			var tmp_buf_dat models.St_exch_msg_resp
			buf := bytes.NewBuffer(msgData)
			if err := binary.Read(buf, binary.BigEndian, &tmp_buf_dat); err != nil {
				log.Printf("[rcv_thrd] Failed to decode exch_msg_so_data: %v", err)
				return
			}

			log.Printf("[rcv_thrd] Message type: %d", tmp_buf_dat.St_oe_res.St_hdr.Si_transaction_code)

			switch tmp_buf_dat.St_oe_res.St_hdr.Si_transaction_code {
			case util.PRICE_CONFIRMATION:
				log.Println("[rcv_thrd] PRICE RESPONSE")

			case util.ORDER_CONFIRMATION_OUT:
				log.Println("[rcv_thrd] CONFIRM RESPONSE")
				log.Println("[rcv_thrd] Exchange Response Received:")
				log.Printf("[rcv_thrd] Transaction Id: %d", tmp_buf_dat.St_oe_res.St_hdr.Si_transaction_code)
				log.Printf("[rcv_thrd] IOC Flag Value: %d", ESRM.TCUM.BoolToInt(tmp_buf_dat.St_oe_res.St_ord_flg.GetFlagValue(models.Flg_IOC)))
				log.Printf("[rcv_thrd] Error Code: %d", tmp_buf_dat.St_oe_res.St_hdr.Si_error_code)

			default:
				log.Printf("[rcv_thrd] Unexpected transaction code: %d", tmp_buf_dat.St_oe_res.St_hdr.Si_transaction_code)
			}

		default:
			log.Printf("[rcv_thrd] Unexpected message size: %v bytes", len(msgData))
			return
		}

		if exitcount == 1 {
			log.Printf("[rcv_thrd] Exit condition met, breaking the loop.")
			break
		}
	}

	log.Printf("Inside Rcv Routine ENDS HERE")
}

func (ESRM *ESRManager) readXchngSocket(conn net.Conn, timeout time.Duration) ([]byte, error) {
	// Set the header size based on the size of the NetHdr struct
	headerSize := int(unsafe.Sizeof(models.St_net_hdr{}))
	headerData, err := ESRM.readNBytes(conn, headerSize, timeout)
	if err != nil {
		return nil, err
	}

	var header models.St_net_hdr
	buf := bytes.NewBuffer(headerData)
	if err := binary.Read(buf, binary.BigEndian, &header); err != nil {
		return nil, err
	}

	// Calculate the length of the remainder of the message
	remainingLength := int(header.S_message_length) - headerSize

	// Read the remaining message
	var messageData []byte
	if remainingLength > 0 {
		messageData, err = ESRM.readNBytes(conn, remainingLength, timeout)
		if err != nil {
			return nil, err
		}
	}

	// Combine header and message into one slice
	fullMessage := append(headerData, messageData...)

	return fullMessage, nil
}

func (ESRM *ESRManager) readNBytes(conn net.Conn, li_len int, timeout time.Duration) ([]byte, error) {
	buffer := make([]byte, li_len)
	read := 0

	// Set the read deadline based on the timeout value
	if timeout > 0 {
		conn.SetReadDeadline(time.Now().Add(timeout))
	} else {
		conn.SetReadDeadline(time.Time{}) // No timeout
	}

	for read < li_len {
		n, err := conn.Read(buffer[read:])
		if err != nil {
			if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
				// Handle timeout
				return nil, errors.New("read timeout")
			}
			return nil, err
		}

		if n == 0 {
			// Socket closed by the other end
			return nil, errors.New("socket closed by peer")
		}

		read += n
	}

	// Clear the deadline
	if timeout > 0 {
		conn.SetReadDeadline(time.Time{}) // Remove timeout after successful read
	}

	return buffer, nil
}

func (ESRM *ESRManager) ClnEsrClnt() int {

	ESRM.Req_q_data = &models.St_req_q_data{}
	ESRM.St_exch_msg = &models.St_exch_msg{}
	ESRM.St_net_hdr = &models.St_net_hdr{}
	ESRM.St_oe_reqres = &models.St_oe_reqres{}
	ESRM.St_sign_on_req = &models.St_sign_on_req{}
	ESRM.PUM = &util.PasswordUtilManger{}

	ESRM.TM = &util.TransactionManager{}

	conn, err := ESRM.Fn_crt_tap_con()
	if err != nil {
		log.Fatalf("Error in crt_tap_con: %v", err)
	}
	defer conn.Close()

	// time.Sleep(100 * time.Millisecond)

	wg.Add(1)
	go ESRM.send_thrd(conn)
	log.Printf("Send Routine Initialized")
	go ESRM.rcv_thrd(conn)
	log.Printf("Recieve Routine Initialized")

	wg.Wait()
	return 0
}

func (ESRM *ESRManager) Do_xchng_logon(Data []byte) error {

	if len(Data) < 22 {
		return fmt.Errorf(" [Do_xchng_logon] Message size is less than size of NET_HDR")
	}

	NET_HDR := Data[:22]

	message := Data[22:]

	if err := ESRM.SCM.WriteOnTapSocket(NET_HDR, message); err != nil {
		log.Printf("[%s] [Do_xchng_logon] Failed to send exch_msg_data: %v", err)
		return err
	}

	log.Printf("Waiting for logon response...")

	logonResponse := <-logonResponseChan // Block until logon response is received
	log.Printf("Logon response received, processing...")

	// Process the logon response (example)
	log.Printf("Transaction Code: %v", logonResponse.St_sign_on_req.StHdr.Si_transaction_code)
	log.Printf("Error Code: %v", logonResponse.St_sign_on_req.StHdr.Si_error_code)

	// Return if error code in logon response indicates failure
	if logonResponse.St_sign_on_req.StHdr.Si_error_code != 0 {
		return fmt.Errorf("logon failed with error code: %v", logonResponse.St_sign_on_req.StHdr.Si_error_code)
	}

	// Logon success
	log.Printf("Logon successful, continuing...")

	return nil
}
