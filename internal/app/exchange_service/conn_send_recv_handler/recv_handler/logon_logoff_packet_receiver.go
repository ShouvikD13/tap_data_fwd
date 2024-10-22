package recvhandler

import (
	"DATA_FWD_TAP/util"
	socket "DATA_FWD_TAP/util/TapSocket"
	"bytes"
	"encoding/binary"
	"fmt"
	"os"
)

type RecieverManager struct {
	ServiceName string
	SM          *socket.SocketManager
	LM          *util.LoggerManager
}

func (RM *RecieverManager) FnDoXchngLogOn(Data []byte) int {

	if err := RM.SM.WriteOnTapSocket(Data); err != nil {

		RM.LM.LogInfo("")

	}

	return 0
}

// Go equivalent of C function
func fnDoXchngLogon(ptrStExchMsg *StExchMsg, ptrSndMsgsFile *os.File, cServiceName string, cErrMsg *string) int {
	var iChVal int
	var liBusinessDataSize, liSendTapMsgSize int64
	var liOpmTrdrId int64
	var cTapHeader byte
	var stExchMessage StExchMsg
	var digest [16]byte
	var stSysInfReq StSystemInfoReq

	// Set initial condition control
	iCondSndRcvCntrl = LOGON_REQ_SENT

	// Calculate business data size
	liBusinessDataSize = int64(binary.Size(stSysInfReq))
	liSendTapMsgSize = int64(binary.Size(ptrStExchMsg.StNetHeader)) + liBusinessDataSize

	// Increment sequence number and log
	iNtwkHdrSeqNo++
	ptrStExchMsg.StNetHeader.ISeqNum = iNtwkHdrSeqNo
	fnUserlog(cServiceName, "st_net_header.i_seq_num :%d:", ptrStExchMsg.StNetHeader.ISeqNum)

	// Send data over socket
	msgBuffer := new(bytes.Buffer)
	binary.Write(msgBuffer, binary.LittleEndian, ptrStExchMsg)
	err := fnWriten(iTapSckId, msgBuffer.Bytes(), cServiceName, cErrMsg)
	if err != nil {
		fnErrlog(cServiceName, "L31005", "Error sending message", cErrMsg)
		return -1
	}

	// Log message dump if needed
	if iMsgDumpFlg == LOG_DUMP {
		cTapHeader = 'Y'
		fnFprintLog(ptrSndMsgsFile, cTapHeader, msgBuffer.Bytes(), cServiceName, cErrMsg)
	}

	// Wait for condition to be met
	for iCondSndRcvCntrl != LOGON_RESP_RCVD && iCondSndRcvCntrl != RCV_ERR {
		fnUserlog(cServiceName, "Inside while loop waiting for LOGON_RESP_RCVD or RCV_ERR")
		err = fnThrdCondWait(condSndRcvCntrl, &mutSndRcvCntrl, cServiceName, cErrMsg)
		if err != nil {
			fnErrlog(cServiceName, "L31010", "Error in thread wait", cErrMsg)
			return -1
		}
	}

	// Check if an error occurred
	if iCondSndRcvCntrl == RCV_ERR {
		fnErrlog(cServiceName, "L31015", "Error in receive thread", cErrMsg)
		return -1
	}

	// Populate System information request structure
	liOpmTrdrId, err = fetchTraderID("sqlCPipeID", "sqlCXchngCD")
	if err != nil {
		fnErrlog(cServiceName, "L31020", "Error fetching trader ID", cErrMsg)
		return -1
	}

	stSysInfReq.StHdr.LiTraderId = liOpmTrdrId
	stSysInfReq.StHdr.LiLogTime = 0
	fnOrstonseChar(stSysInfReq.StHdr.CAlphaChar[:], LEN_ALPHA_CHAR, " ", 1)
	stSysInfReq.StHdr.SiTransactionCode = SYSTEM_INFORMATION_IN
	stSysInfReq.StHdr.SiErrorCode = 0
	fnOrstonseChar(stSysInfReq.StHdr.CFiller2[:], 8, " ", 1)

	return 0
}

func main() {
	// Example usage of fnDoXchngLogon
	var exchMsg StExchMsg
	var errMsg string
	f, _ := os.Create("log.txt")
	defer f.Close()

	result := fnDoXchngLogon(&exchMsg, f, "ServiceName", &errMsg)
	fmt.Println("Result:", result)
	if errMsg != "" {
		fmt.Println("Error:", errMsg)
	}
}
