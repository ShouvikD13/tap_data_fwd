package recvhandler

import (
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"DATA_FWD_TAP/util/OrderConversion"
	"bytes"
	"crypto/md5"
	"encoding/binary"
)

type RecvManager struct {
	NetHDR      models.St_net_hdr
	LM          *util.LoggerManager
	OCM         *OrderConversion.OrderConversionManager
	ServiceName string
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
