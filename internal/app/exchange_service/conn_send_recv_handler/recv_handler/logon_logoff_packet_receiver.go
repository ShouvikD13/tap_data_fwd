package recvhandler

import (
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"DATA_FWD_TAP/util/OrderConversion"
	typeconversionutil "DATA_FWD_TAP/util/TypeConversionUtil"
	"bytes"
	"crypto/md5"
	"encoding/binary"
	"fmt"
)

type RecvManager struct {
	NetHDR            models.St_net_hdr
	St_sign_on_res    models.St_sign_on_res
	St_Error_Response *models.St_Error_Response
	Vw_mkt_msg        *models.Vw_mkt_msg
	LM                *util.LoggerManager
	OCM               *OrderConversion.OrderConversionManager
	TCUM              *typeconversionutil.TypeConversionUtilManager
	ServiceName       string
}

func (RM *RecvManager) FnValidateTap(exgSeq *int32, wholePtrData []byte) int {

	// Step 1: Validate the received buffer size
	headerSize := binary.Size(RM.NetHDR)
	if len(wholePtrData) < headerSize {
		RM.LM.LogError(RM.ServiceName, "[FnValidateTap] Error: Packet data is too small to contain header")
		return -1
	}

	// Step 2: Copy data to the NetHdr structure and Perform Order Conversion
	reader := bytes.NewReader(wholePtrData[:headerSize])
	err := binary.Read(reader, binary.BigEndian, &RM.NetHDR)
	if err != nil {
		RM.LM.LogError(RM.ServiceName, "[FnValidateTap] Error reading net header: %v", err)
		return -1
	}
	// order conversion
	RM.NetHDR = RM.OCM.ConvertNetHeaderToHostOrder(RM.NetHDR)

	RM.LM.LogInfo(RM.ServiceName, "[FnValidateTap] Sequence number of tap packet: %d", RM.NetHDR.I_seq_num)
	RM.LM.LogInfo(RM.ServiceName, "[FnValidateTap] Incoming sequence number: %d", *exgSeq)
	RM.LM.LogInfo(RM.ServiceName, "[FnValidateTap] Sequence difference: %d", *exgSeq-RM.NetHDR.I_seq_num)

	*exgSeq++

	// Calculate the expected packet length
	packetLength := int(RM.NetHDR.S_message_length) - headerSize
	if packetLength < 0 || len(wholePtrData) < packetLength+headerSize {
		RM.LM.LogError(RM.ServiceName, "[FnValidateTap] Error: Invalid packet length")
		return -1
	}

	// Step 3: Calculate MD5 digest on packet data
	dataToDigest := wholePtrData[headerSize : headerSize+packetLength]
	digest := md5.Sum(dataToDigest)

	// Step 4: Compare checksums
	if !bytes.Equal(RM.NetHDR.C_checksum[:], digest[:]) {
		RM.LM.LogError(RM.ServiceName, "[FnValidateTap] Error: Checksum validation failed")
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, "[FnValidateTap] Checksum validation successful")
	return 0
}

func (RM *RecvManager) FnSignOnRequestOut(St_sign_on_res *models.St_sign_on_res) int {

	RM.LM.LogInfo(RM.ServiceName, "[FnSignOnRequestOut] Inside Log On Response Handling Function")

	err, TempTimeStamp := RM.TCUM.LongToTimeArr(string(St_sign_on_res.St_hdr.Li_log_time))
	if err != 0 {
		RM.LM.LogError(RM.ServiceName, "Error recieved while parsing the time ")
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("The ERROR CODE IS : %d", St_sign_on_res.St_hdr.Si_error_code))

	RM.LM.LogInfo(RM.ServiceName, "Sign On Successful : Checking Other Details")
	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Broker Status Is : %c", St_sign_on_res.C_broker_status))
	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Clearing Status Is : %c", St_sign_on_res.C_clearing_status))

	// Initialize message
	c_message := "Logon Successful"

	if St_sign_on_res.C_broker_status == util.CLOSE_OUT {
		c_msg := fmt.Sprintf("|%s| - Logon successful, Broker ClosedOut|", TempTimeStamp)
		c_message = "Logon Successful, Broker ClosedOut"

		RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Msg Is : %s", c_msg))

	} else if St_sign_on_res.C_broker_status != util.ACTIVE {
		c_msg := fmt.Sprintf("|%s| - Logon successful, Broker Suspended|", TempTimeStamp)
		c_message = "Logon Successful, Broker Suspended"

		RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Msg Is : %s", c_msg))

	}

	if St_sign_on_res.C_clearing_status != util.ACTIVE {
		c_msg := fmt.Sprintf("|%s| - Logon successful, Clearing member suspended|", TempTimeStamp)
		c_message = "Logon Successful, Clearing member suspended"

		RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Msg Is : %s", c_msg))

	}

	RM.LM.LogInfo(RM.ServiceName, "Inside Success Condition While Log In")
	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("c_message Is : %s", c_message))

	// Exchange Code and Broker ID fetching (pseudo-code as placeholder for SQL queries)
	exchangeCode := "FetchedExchangeCode" // Replace with actual DB call
	brokerID := "FetchedBrokerID"         // Replace with actual DB call

	c_msg := fmt.Sprintf("|%s|%d- %s|", TempTimeStamp, St_sign_on_res.St_hdr.Si_error_code, c_message)
	st_msg := &models.Vw_mkt_msg{
		C_xchngCd: exchangeCode,
		C_Msg:     c_msg,
		C_MsgId:   TRADER_MSG,
		C_BrkrId:  brokerID,
	}

	RM.LM.LogInfo(RM.ServiceName, "Exchange Code Is : %s", st_msg.C_xchng_cd)
	RM.LM.LogInfo(RM.ServiceName, "Broker ID Is : %s", st_msg.C_brkr_id)
	RM.LM.LogInfo(RM.ServiceName, "Message Is : %s", st_msg.C_msg)
	RM.LM.LogInfo(RM.ServiceName, "Time Stamp Is : %s", TempTimeStamp)
	RM.LM.LogInfo(RM.ServiceName, "Message ID Is : %c", st_msg.C_msg_id)
	RM.LM.LogInfo(RM.ServiceName, "Before Call to SFO_UPD_MSG")

	// Service Call (placeholder as a mock call)
	serviceError := RM.ServiceCall("SFO_UPD_MSG", st_msg)
	if serviceError != nil {
		RM.LM.LogError(RM.ServiceName, "Service call to SFO_UPD_MSG failed")
		return -1
	}

	// Additional time conversions (using example values)
	pwdLastChangeTime, _ := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", St_sign_on_res.Li_last_password_change_date))
	marketCloseDate, _ := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", St_sign_on_res.St_hdr.Li_log_time))
	exchangeTime, _ := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", St_sign_on_res.Li_end_time))

	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Password Last Change Date Is : %s", pwdLastChangeTime))
	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Market Closing Date Is : %s", marketCloseDate))
	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Exchange Time Is : %s", exchangeTime))

	RM.LM.LogInfo(RM.ServiceName, "[FnSignOnRequestOut] Exiting Log On Response Handling Function")

	return 0
}

/*
	RM.St_Error_Response.St_hdr = St_sign_on_res.St_hdr
		copy(RM.St_Error_Response.CKey[:], St_sign_on_res.St_hdr.C_key[:])
		copy(RM.St_Error_Response.CErrorMessage[:], St_sign_on_res.St_hdr.C_error_message[:])

		there is a problem in this code . both these structures we get in a response from the NSE
		if there errorcode in the header is 0 then we recieve the actual structure otherwise we are getting the error structure .
		// i wanted here , if we recieve error than St_sign_on_res convert into St_Error_Response
*/

func (RM *RecvManager) FnSignOnRequestOutErrorResponse(St_Error_Response *models.St_Error_Response) int {

	RM.LM.LogInfo(RM.ServiceName, "[FnSignOnRequestOutErrorResponse] Inside Log On Error Response Handling Function")

	err, TempTimeStamp := RM.TCUM.LongToTimeArr(string(St_Error_Response.St_Hdr.Li_log_time))
	if err != 0 {
		RM.LM.LogError(RM.ServiceName, "Error recieved while parsing the time ")
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, "[FnSignOnRequestOutErrorResponse] Exiting Log On Error Response Handling Function")
	return 0
}
