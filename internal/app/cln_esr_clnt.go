package app

import (
	//"bytes"
	//"encoding/binary"
	"DATA_FWD_TAP/models"
	"DATA_FWD_TAP/models/structures"
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	"net"
	"sync"
	"time"

	"unsafe"
)

type ESRManger struct {
	req_q_data   *structures.St_req_q_data
	ServiceName  string
	st_exch_msg  *structures.St_exch_msg
	st_net_hdr   *structures.St_net_hdr
	st_oe_reqres *structures.St_oe_reqres
	ENVM         *models.EnvironmentManager
}

var li_send_tap_msg_size int64
var li_business_data_size int64

var (
	messageQueue = make(chan structures.St_req_q_data, 20)
	wg           sync.WaitGroup
)

type Config struct {
	IP            string
	Port          string
	AutoReconnect bool
}

/*
type exch_msg struct {
	TestData1 int64
	TestData2 byte
	TestData3 [20]byte // Fixed-size byte array for the string
}

type req_q_data struct {
	li_msg_type   int64
	exch_msg_data exch_msg
} */

func (ESRM *ESRManger) loadConfig() (Config, error) {
	// cfg, err := ini.Load("EnvConfig.ini")
	// if err != nil {
	// 	return Config{}, fmt.Errorf("failed to read config file: %v", err)
	// }

	autoReconnect := false //ESRM.ENVM.GetProcessSpaceValue("server", "auto_reconnect")
	// if err != nil {
	// 	return Config{}, fmt.Errorf("failed to read auto_reconnect config: %v", err)
	// }

	config := Config{
		IP:            ESRM.ENVM.GetProcessSpaceValue("server", "ip"),
		Port:          ESRM.ENVM.GetProcessSpaceValue("server", "port"),
		AutoReconnect: autoReconnect, // strconv.ParseBool(autoReconnect)
	}
	return config, nil
}

func (ESRM *ESRManger) crt_tap_con() (net.Conn, error) {
	config, err := ESRM.loadConfig()
	if err != nil {
		return nil, fmt.Errorf("error loading config: %v", err)
	}
	// Ensure the format specifier matches the type of the values being logged
	log.Printf("[%s] [crt_tap_con] IP for TAP socket: %s", ESRM.ServiceName, config.IP)
	log.Printf("[%s] [crt_tap_con] Port for TAP socket: %s", ESRM.ServiceName, config.Port)
	log.Printf("[%s] [crt_tap_con] Auto Reconnect status: %v", ESRM.ServiceName, config.AutoReconnect)

	log.Println("Starting Connection to TAP")

	address := net.JoinHostPort(config.IP, config.Port)
	conn, err := net.Dial("tcp", address)
	if err != nil {
		log.Println("Error connecting:", err)
		if config.AutoReconnect {
			log.Println("Disconnected. Auto Reconnecting...")
			return nil, nil
		} else {
			log.Println("Auto Reconnect is disabled.")
			return nil, err
		}
	}

	log.Println("Connected to TAP at", address)
	return conn, nil
}

func (ESRM *ESRManger) send_thrd(conn net.Conn) {
	defer wg.Done()

	for msg := range messageQueue {
		// Check if li_msg_type is "2000"
		if msg.L_msg_type == models.BOARD_LOT_IN {
			log.Println("Message type is c0rrect")

			li_business_data_size = int64(unsafe.Sizeof(structures.St_oe_reqres{}))                      // will use st_oe_reqres in actrual implementation.
			li_send_tap_msg_size = int64(unsafe.Sizeof(structures.St_net_hdr{})) + li_business_data_size // will use size of net header here in implementation.
			log.Println("TAP message size:", li_send_tap_msg_size)
			log.Println("Busniness Data Size:", li_business_data_size)
			// Call fn_writen to write exch_msg_data to TCP connection
			if err := ESRM.fn_writen(conn, msg.St_exch_msg_data, li_send_tap_msg_size); err != nil {
				log.Printf("[%s] []Failed to send exch_msg_data: %v", err)
				return
			}

		} else {
			log.Printf("[%s] [send_thrd] Skipped message with li_msg_type: %d", ESRM.ServiceName, msg.L_msg_type)
		}
	}

	log.Println("Message queue closed, send thread exiting")

}

func (ESRM *ESRManger) fn_writen(conn net.Conn, msg structures.St_exch_msg, msg_size int64) error {
	var buf bytes.Buffer

	// Serialize the exch_msg structure into the buffer
	if err := binary.Write(&buf, binary.BigEndian, msg); err != nil {
		return err
	}

	dsize := buf.Len()

	log.Println("Size of binary data:", dsize)

	data := buf.Bytes()

	// Restrict the message size if it exceeds the given msg_size
	if int64(len(data)) > msg_size {
		data = data[:msg_size]
	}

	totalWritten := 0

	for totalWritten < len(data) {
		log.Printf("[%s] []Attempting to write: totalWritten=%d, data_len=%d", totalWritten, len(data))
		written, err := conn.Write(data[totalWritten:])
		if err != nil {
			if ne, ok := err.(net.Error); ok && ne.Temporary() {
				log.Println("Temporary network error, retrying...")
				time.Sleep(100 * time.Millisecond)
				continue
			}
			log.Printf("[%s] []Write failed: %v\n", err)
			return err
		}

		if written == 0 {
			// If no bytes are written, the connection might be stalled or closed.
			log.Println("No bytes written; connection may be stalled or closed.")
			return net.ErrWriteToConnected
		}

		totalWritten += written

		log.Printf("[%s] []Written: %d bytes, Total Written: %d bytes", written, totalWritten)

		if totalWritten < len(data) {
			// Small delay to help with buffer recovery
			time.Sleep(50 * time.Millisecond)
		}
	}

	log.Printf("[%s] []Successfully sent exch_msg: %+v", msg)
	return nil
}

/* func (ESRM *ESRManger) rcv_thrd(conn net.Conn) {
	defer wg.Done()

	for {
		var tmp_buf_dat exch_msg

		li_len_buf := int(unsafe.Sizeof(tmp_buf_dat))
		c_ptr_data := make([]byte, li_len_buf) //Will use MAX_SCK_MSG in actual implementation

		// Read data from the socket
		if _, err := conn.Read(c_ptr_data); err != nil {
			log.Printf("[%s] []Error reading from socket: %v", err)
			return
		}

		// Unmarshal the data into the tmp_buf_dat struct
		buf := bytes.NewBuffer(c_ptr_data)
		if err := binary.Read(buf, binary.BigEndian, &tmp_buf_dat); err != nil {
			log.Printf("[%s] []Failed to decode exch_msg_data: %v", err)
			return
		}

		// Here you would typically perform your validation and processing
		// For example, validating the transaction code:
		// tmp_buf_dat.st_hdr.si_transaction_code = ntohs(tmp_buf_dat.st_hdr.si_transaction_code)

		log.Printf("[%s] []Message received from exchange: %+v", tmp_buf_dat)
	}
} */

/* func (ESRM *ESRManger) ClnEsrClnt() {
	conn, err := ESRM.crt_tap_con()
	if err != nil {
		log.Fatalf("Error in crt_tap_con: %v", err)
	}
	defer conn.Close()

	wg.Add(1)
	go ESRM.send_thrd(conn)
	//go ESRM.rcv_thrd(conn)

	time.Sleep(100 * time.Millisecond)

	// Enqueue messages
	for i := 1; i <= 10; i++ {
		ex_msg := exch_msg{
			TestData1: int64(i),
			TestData2: byte(i + 1),
		}
		copy(ex_msg.TestData3[:], "Message "+string(rune('A'+i-1))) // Copy string into fixed-size array

		msg := req_q_data{
			li_msg_type:   1, // li_msg_type is set to 1 for all messages
			exch_msg_data: ex_msg,
		}

		messageQueue <- msg
		log.Printf("[%s] []Queued message: %+v", msg)
		time.Sleep(10 * time.Millisecond)
	}

	// Optionally close the queue when you're done
	close(messageQueue)

	wg.Wait()
} */

func (ESRM *ESRManger) ClnEsrClnt() {
	conn, err := ESRM.crt_tap_con()
	if err != nil {
		log.Fatalf("Error in crt_tap_con: %v", err)
	}
	defer conn.Close()

	wg.Add(1)
	go ESRM.send_thrd(conn)
	// go ESRM.rcv_thrd(conn)

	time.Sleep(100 * time.Millisecond)

	// Create three distinct messages
	for i := 1; i <= 3; i++ {
		// Initialize the St_req_q_data structure
		reqData := structures.St_req_q_data{
			L_msg_type: 2000, // All messages have the same L_msg_type
			St_exch_msg_data: structures.St_exch_msg{
				St_net_header: structures.St_net_hdr{
					S_message_length: int16(i * 10),
					I_seq_num:        int32(i),
					C_checksum:       [16]byte{}, // Fixed-size byte array
				},
				St_oe_res: structures.St_oe_reqres{
					St_hdr: &structures.St_int_header{
						Si_transaction_code: int16(i),
						Li_log_time:         int32(i * 1000),
						C_alpha_char:        [models.LEN_ALPHA_CHAR]byte{'A' + byte(i), 'B' + byte(i)},
						Li_trader_id:        int32(i * 200),
						Si_error_code:       int16(i),

						C_filler_2:        0,
						C_time_stamp_1:    [models.LEN_TIME_STAMP]byte{'T', 'S', '1', byte(i + '0'), ' ', ' ', ' ', ' '},
						C_time_stamp_2:    [models.LEN_TIME_STAMP]byte{'T', 'S', '2', byte(i + '0'), ' ', ' ', ' ', ' '},
						Si_message_length: int16(i * 100),
					},
					C_participant_type:      byte('A' + i),
					Si_competitor_period:    int16(i),
					Si_solicitor_period:     int16(i),
					C_modified_cancelled_by: byte(0),
					C_filler_2:              byte(0),
					Si_reason_code:          int16(i * 2),
					L_token_no:              int32(i * 3000),
					St_con_desc: &structures.St_contract_desc{
						C_instrument_name: [models.LEN_INSTRUMENT_NAME]byte{'I', 'N', 'S', byte(i + '0')},
						C_symbol:          [models.LEN_SYMBOL_NSE]byte{'S', 'Y', 'M', byte(i + '0')},
						Li_expiry_date:    int32(i * 10000),
						Li_strike_price:   int32(i * 50000),

						C_option_type: [models.LEN_OPTION_TYPE]byte{'O', 'T'},
						Si_ca_level:   int16(i * 10),
					},
					St_ord_flg: &structures.St_order_flags{

						Flg_ato:         uint16(1),
						Flg_market:      uint16(0),
						Flg_sl:          uint16(1),
						Flg_mit:         uint16(0),
						Flg_day:         uint16(1),
						Flg_gtc:         uint16(0),
						Flg_ioc:         uint16(1),
						Flg_aon:         uint16(0),
						Flg_mf:          uint16(1),
						Flg_matched_ind: uint16(0),
						Flg_traded:      uint16(1),
						Flg_modified:    uint16(0),
						Flg_frozen:      uint16(1),
						Flg_filler1:     uint16(0),
					},
					Si_order_type:      int16(i),
					D_order_number:     float64(i) * 1.1,
					Li_volume:          int32(i * 400),
					Si_branch_id:       int16(i * 5),
					Li_trader_id:       int32(i * 500),
					C_broker_id:        [models.LEN_BROKER_ID]byte{'B', 'R', 'K', byte('0' + i)},
					C_account_number:   [models.LEN_ACCOUNT_NUMBER]byte{'A', 'C', 'C', byte('0' + i)},
					L_algo_id:          int32(i * 6000),
					Ll_lastactivityref: int64(i * 7000),

					C_reserved: byte(0),
				},
			},
		}

		// Enqueue the message
		messageQueue <- reqData
		log.Printf("[Message %d] Queued message: %+v", i, reqData)
		time.Sleep(10 * time.Millisecond)
	}

	// Optionally close the queue when done
	close(messageQueue)

	wg.Wait()
}
