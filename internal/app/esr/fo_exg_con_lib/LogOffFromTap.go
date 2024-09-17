package foexgconlib

import (
	"DATA_FWD_TAP/util"
	"crypto/md5"
	"encoding/json"
	"fmt"
	"io"
	"reflect"
	"strconv"
	"strings"
	"unsafe"
)

func (FECLM *FoExgConLibManager) Fn_logoff_from_TAP() int {
	var sql_c_opm_xchng_cd, sql_c_exg_trdr_id string
	var sql_c_nxt_trd_date string

	// Fetch opm_xchng_cd and opm_trdr_id
	query1 := `
		SELECT opm_xchng_cd, opm_trdr_id
		FROM opm_ord_pipe_mstr
		WHERE opm_pipe_id = ?`
	err1 := FECLM.DB.Raw(query1, FECLM.C_pipe_id).Row().Scan(&sql_c_opm_xchng_cd, &sql_c_exg_trdr_id).Error
	if err1 != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "[Fn_logoff_from_TAP] Error fetching opm_xchng_cd: %v", err1)
		return -1
	}

	// Fetch exg_nxt_trd_date
	query2 := `
		SELECT exg_nxt_trd_dt
		FROM exg_xchng_mstr
		WHERE exg_xchng_cd = ?`
	err2 := FECLM.DB.Raw(query2, sql_c_opm_xchng_cd).Scan(&sql_c_nxt_trd_date).Error
	if err2 != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "[Fn_logoff_from_TAP] Error fetching exg_nxt_trd_dt: %v", err2)
		return -1
	}

	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "[Fn_logoff_from_TAP] The Trade date is: %s", sql_c_nxt_trd_date)

	value, err := strconv.Atoi(strings.TrimSpace(sql_c_exg_trdr_id))
	if err != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "[fFn_logoff_from_TAP] ERROR: Failed to convert Trader ID to int: %v", err)
		return -1
	}

	FECLM.int_header.Li_trader_id = int32(value)                                        // 1 HDR
	FECLM.int_header.Li_log_time = 0                                                    // 2 HDR
	CopyAndFormatSymbol(FECLM.int_header.C_alpha_char[:], util.LEN_ALPHA_CHAR, " ")     // 3 HDR
	FECLM.int_header.Si_transaction_code = util.SIGN_OFF_REQUEST_IN                     // 4 HDR
	FECLM.int_header.Si_error_code = 0                                                  // 5 HDR
	copy(FECLM.int_header.C_filler_2[:], "        ")                                    // 6 HDR
	copy(FECLM.int_header.C_time_stamp_1[:], defaultTimeStamp)                          // 7 HDR
	copy(FECLM.int_header.C_time_stamp_2[:], defaultTimeStamp)                          // 8 HDR
	FECLM.int_header.Si_message_length = int16(reflect.TypeOf(FECLM.int_header).Size()) // 9 HDR

	// Generate the sequence number
	FECLM.Exg_NxtTrdDate = strings.TrimSpace(sql_c_nxt_trd_date)

	FECLM.St_net_hdr.S_message_length = int16(unsafe.Sizeof(FECLM.St_net_hdr) + unsafe.Sizeof(FECLM.int_header)) // 1 NET_FDR
	FECLM.St_net_hdr.I_seq_num = FECLM.GetResetSequence()                                                        // 2 NET_HDR

	hasher := md5.New()
	int_headerString, err := json.Marshal(FECLM.int_header) // to convert the structure in string
	if err != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, " [ Error: failed to convert int_header to string: %v", err)
	}
	io.WriteString(hasher, string(int_headerString))

	checksum := hasher.Sum(nil)
	copy(FECLM.St_net_hdr.C_checksum[:], fmt.Sprintf("%x", checksum)) // 3 NET_HDR

	// Fill request queue data
	FECLM.St_req_q_data.L_msg_type = util.SIGN_OFF_REQUEST_IN

	// Here I have to write Another field on St_Req_data for Log_Off

	// Write to message queue
	if FECLM.MQM.CreateQueue(util.LOGOFF_QUEUE_ID) != 0 {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, " [Fn_logoff_from_TAP] [Error: Returning from 'CreateQueue' with an Error... %s")
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, " [Fn_logoff_from_TAP]  Exiting from function")
	} else {
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, " [Fn_logoff_from_TAP] Created Message Queue SuccessFully")
	}

	if FECLM.MQM.WriteToQueue(mtype) != 0 {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, " [CLN_PACK_CLNT] [Error:  Failed to write to queue with message type %d", mtype)
		return -1
	}

	// err = FECLM.fn_write_msg_q(i_send_qid, FECLM.St_req_q_data)
	// if err != nil {
	// 	FECLM.LoggerManager.LogError(FECLM.ServiceName, "[Fn_logoff_from_TAP] Failed to write in Transmit queue: %v", err)
	// 	*c_err_msg = fmt.Sprintf("Failed to write in Transmit queue: %v", err)
	// 	return -1
	// }

	return 0
}
