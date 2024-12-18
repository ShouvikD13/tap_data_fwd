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
	"strings"
	"time"

	"gorm.io/gorm"
)

type RecvManager struct {
	ServiceName string
	//------ All the utility managers
	Db   *gorm.DB
	TCM  *util.TransactionManager
	PUM  *util.PasswordUtilManger
	LM   *util.LoggerManager
	OCM  *OrderConversion.OrderConversionManager
	TCUM *typeconversionutil.TypeConversionUtilManager
	//------ all the needed structures
	St_int_header     *models.St_int_header
	NetHDR            *models.St_net_hdr
	St_sign_on_res    *models.St_sign_on_res
	St_Error_Response *models.St_Error_Response
	stSysInfoDat      *models.StSystemInfoData
	stLdbData         *models.StUpdateLocalDBData
	//-------- All needed variable
	C_xchng_cd string
	PipeId     string
}

func NewRecvManager(
	serviceName string,
	db *gorm.DB,
	tcm *util.TransactionManager,
	pum *util.PasswordUtilManger,
	lm *util.LoggerManager,
	ocm *OrderConversion.OrderConversionManager,
	tcum *typeconversionutil.TypeConversionUtilManager,
	stIntHeader *models.St_int_header,
	netHDR *models.St_net_hdr,
	stSignOnRes *models.St_sign_on_res,
	stErrorRes *models.St_Error_Response,
	stSysInfoDat *models.StSystemInfoData,
	stLdbData *models.StUpdateLocalDBData,
	cXchngCd string,
	pipeId string,
) *RecvManager {
	return &RecvManager{
		ServiceName:       serviceName,
		Db:                db,
		TCM:               tcm,
		PUM:               pum,
		LM:                lm,
		OCM:               ocm,
		TCUM:              tcum,
		St_int_header:     stIntHeader,
		NetHDR:            netHDR,
		St_sign_on_res:    stSignOnRes,
		St_Error_Response: stErrorRes,
		stSysInfoDat:      stSysInfoDat,
		stLdbData:         stLdbData,
		C_xchng_cd:        cXchngCd,
		PipeId:            pipeId,
	}
}

/*
	recvMgr := NewRecvManager(
	"RecvService",
	db,
	tcm,
	pum,
	lm,
	ocm,
	tcum,
	stIntHeader,
	netHDR,
	stSignOnRes,
	stErrorRes,
	stSysInfoDat,
	stLdbData,
	"XCHNG_CD",
	"PipeID_123",
)

*/

func (RM *RecvManager) FnValidateTap(exgSeq *int32, NetHDR *models.St_net_hdr, DataRecievedFromTap []byte) error {

	headerSize := binary.Size(NetHDR)
	RM.LM.LogInfo(RM.ServiceName, "[FnValidateTap] calculated size for Net_HDR: %d", headerSize)
	if len(DataRecievedFromTap) < headerSize {
		RM.LM.LogError(RM.ServiceName, "[FnValidateTap] Error: Packet data is too small to contain header")
		return fmt.Errorf("packet data too small; expected at least %d bytes, got %d bytes", headerSize, len(DataRecievedFromTap))
	}

	RM.LM.LogInfo(RM.ServiceName, "[FnValidateTap] Sequence number of tap packet: %d", NetHDR.I_seq_num)
	RM.LM.LogInfo(RM.ServiceName, "[FnValidateTap] Incoming sequence number: %d", *exgSeq)
	RM.LM.LogInfo(RM.ServiceName, "[FnValidateTap] Sequence difference: %d", *exgSeq-NetHDR.I_seq_num)

	*exgSeq++

	packetLength := int(NetHDR.S_message_length) - headerSize
	RM.LM.LogInfo(RM.ServiceName, "[FnValidateTap] calculated size for 'NetHDR.S_message_length' message Length : %d", NetHDR.S_message_length)

	if packetLength < 0 {
		RM.LM.LogError(RM.ServiceName, "[ValidateTap] Error: Packet length is negative (invalid header length: %d)", packetLength)
		return fmt.Errorf("invalid packet length: %d", packetLength)
	}

	if len(DataRecievedFromTap) < packetLength {
		RM.LM.LogError(RM.ServiceName, "[ValidateTap] Error: Packet data is smaller than expected size (expected: %d, got: %d)", headerSize+packetLength, len(DataRecievedFromTap))
		return fmt.Errorf("packet data too small; expected %d bytes, got %d bytes", headerSize+packetLength, len(DataRecievedFromTap))
	}

	// dataToDigest := DataRecievedFromTap[headerSize : headerSize+packetLength]
	dataToDigest := DataRecievedFromTap
	digest := md5.Sum(dataToDigest)

	if !bytes.Equal(NetHDR.C_checksum[:], digest[:]) {
		RM.LM.LogError(RM.ServiceName, "[ValidateTap] Error: Checksum validation failed. Expected: %x, Received: %x", NetHDR.C_checksum, digest)
		return fmt.Errorf("checksum validation failed; expected %x, got %x", NetHDR.C_checksum, digest)
	}

	RM.LM.LogInfo(RM.ServiceName, "[FnValidateTap] Checksum validation successful")
	return nil
}

func (RM *RecvManager) FnSignOnRequestOut() int {

	RM.LM.LogInfo(RM.ServiceName, "[FnSignOnRequestOut] Inside Log On Response Handling Function")

	err, LogTime := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", RM.St_sign_on_res.St_hdr.Li_log_time))
	if err != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Error recieved while parsing the time ")
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, "Sign On Successful : Checking Other Details")
	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Broker Status Is : %c", RM.St_sign_on_res.C_broker_status))
	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Clearing Status Is : %c", RM.St_sign_on_res.C_clearing_status))

	c_message := "Logon Successful"

	TempTimeStamp := time.Unix(int64(LogTime), 0).Format("02-Jan-2006 15:04:05")

	if RM.St_sign_on_res.C_broker_status == util.CLOSE_OUT {
		c_msg := fmt.Sprintf("|%s| - Logon successful, Broker ClosedOut|", TempTimeStamp)
		c_message = "Logon Successful, Broker ClosedOut"

		RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Msg Is : %s", c_msg))

	} else if RM.St_sign_on_res.C_broker_status != util.ACTIVE {
		c_msg := fmt.Sprintf("|%s| - Logon successful, Broker Suspended|", TempTimeStamp)
		c_message = "Logon Successful, Broker Suspended"

		RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Msg Is : %s", c_msg))
	}

	if RM.St_sign_on_res.C_clearing_status != util.ACTIVE {
		c_msg := fmt.Sprintf("|%s| - Logon successful, Clearing member suspended|", TempTimeStamp)
		c_message = "Logon Successful, Clearing member suspended"

		RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Msg Is : %s", c_msg))
	}

	RM.LM.LogInfo(RM.ServiceName, "Inside Success Condition While Log In")
	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("c_message Is : %s", c_message))

	query := `
	SELECT exg_brkr_id
	FROM exg_xchng_mstr
	WHERE exg_xchng_cd = ?
	`
	var brkrID string
	result := RM.Db.Raw(query, RM.C_xchng_cd).Scan(&brkrID)
	if result.Error != nil {
		RM.LM.LogError(RM.ServiceName, " [FnSignOnRequestOut] [Error executing query: ... %v", result.Error)
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, " [FnSignOnRequestOut] exg_brkr_id: %s", brkrID)

	trader_msg := string(util.TRADER_MSG)
	if initResult := RM.FnInsertTradeMessage(RM.C_xchng_cd, brkrID, trader_msg, c_message, TempTimeStamp); initResult != nil {
		RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Error: %v", initResult)
		return -1
	}

	errPwd, pwdLastChangeDate := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", RM.St_sign_on_res.Li_last_password_change_date))
	if errPwd != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Error parsing Password Last Change Date")
		return -1
	}

	errMarketClose, marketCloseDate := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", RM.St_sign_on_res.St_hdr.Li_log_time))
	if errMarketClose != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Error parsing Market Closing Date")
		return -1
	}

	errExchangeTime, exchangeTime := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", RM.St_sign_on_res.Li_end_time))
	if errExchangeTime != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Error parsing Exchange Time")
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Password Last Change Date Is : %s", pwdLastChangeDate)
	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Market Closing Date Is : %s", marketCloseDate)
	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Exchange Time Is : %s", exchangeTime)

	tranStatus := RM.TCM.FnBeginTran()
	if tranStatus != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Failed to begin transaction")
		return -1
	}

	updateQuery1 := `
    UPDATE OPM_ORD_PIPE_MSTR
    SET OPM_EXG_PWD = OPM_NEW_EXG_PWD,
        OPM_NEW_EXG_PWD = NULL
    WHERE OPM_PIPE_ID = ? AND OPM_XCHNG_CD = ? AND OPM_NEW_EXG_PWD IS NOT NULL`
	if result := RM.Db.Exec(updateQuery1, RM.PipeId, RM.C_xchng_cd); result.Error != nil {
		RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Error updating OPM_ORD_PIPE_MSTR: %v", result.Error)
		if abortResult := RM.TCM.FnAbortTran(); abortResult == -1 {
			RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Failed to abort transaction")
		}
		return -1
	}

	if RM.St_sign_on_res.C_broker_status == util.ACTIVE {
		updateQuery2 := `
        UPDATE opm_ord_pipe_mstr
        SET opm_lst_pswd_chg_dt = TO_DATE(?, 'dd-Mon-yyyy hh24:mi:ss'),
            opm_login_stts = 1
        WHERE opm_pipe_id = ?`
		if result := RM.Db.Exec(updateQuery2, pwdLastChangeDate, RM.PipeId); result.Error != nil {
			RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Error updating opm_ord_pipe_mstr: %v", result.Error)
			if abortResult := RM.TCM.FnAbortTran(); abortResult == -1 {
				RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Failed to abort transaction")
			}
			return -1
		}
	}

	if RM.St_sign_on_res.C_broker_status == util.ACTIVE || RM.St_sign_on_res.C_broker_status == util.CLOSE_OUT || RM.St_sign_on_res.C_broker_status == util.SUSPEND {
		updateQuery3 := `
        UPDATE exg_xchng_mstr
        SET exg_brkr_stts = ?
        WHERE EXG_XCHNG_CD = ?`
		if result := RM.Db.Exec(updateQuery3, RM.St_sign_on_res.C_broker_status, RM.C_xchng_cd); result.Error != nil {
			RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Error updating exg_xchng_mstr: %v", result.Error)
			if abortResult := RM.TCM.FnAbortTran(); abortResult == -1 {
				RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Failed to abort transaction")
			}
			return -1
		}
	}

	commitStatus := RM.TCM.FnCommitTran()
	if commitStatus != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Transaction commit failed")
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, "[FnSignOnRequestOut] Exiting Log On Response Handling Function")

	return 0
}

func (RM *RecvManager) FnErrorResponse() int {
	RM.LM.LogInfo(RM.ServiceName, "[FnErrorResponse] Inside Log On Error Response Handling Function")

	ErrMsg := strings.TrimSpace(string(RM.St_Error_Response.C_ErrorMessage[:]))

	timestampQuery := `
		SELECT to_char(CURRENT_TIMESTAMP, 'DD-Mon-YYYY HH24:MI:SS')
	`
	var timestamp string
	result := RM.Db.Raw(timestampQuery).Scan(&timestamp)
	if result.Error != nil {
		RM.LM.LogError(RM.ServiceName, "[FnErrorResponse] Error executing timestamp query: %v", result.Error)
		return -1
	}

	c_msg := fmt.Sprintf("|%s|%d- %s|", timestamp, RM.St_Error_Response.St_Hdr.Si_error_code, ErrMsg)

	brkrIDQuery := `
		SELECT exg_brkr_id
		FROM exg_xchng_mstr
		WHERE exg_xchng_cd = ?
	`
	var brkrID string
	result = RM.Db.Raw(brkrIDQuery, RM.C_xchng_cd).Scan(&brkrID)
	if result.Error != nil {
		RM.LM.LogError(RM.ServiceName, "[FnErrorResponse] Error executing broker ID query: %v", result.Error)
		return -1
	}

	trader_msg := string(util.TRADER_MSG)

	if initResult := RM.FnInsertTradeMessage(RM.C_xchng_cd, brkrID, trader_msg, c_msg, timestamp); initResult != nil {
		RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Failed to insert trade message : %v", initResult)
		return -1
	}

	if RM.St_Error_Response.St_Hdr.Si_error_code == 16053 {
		RM.LM.LogInfo(RM.ServiceName, "[FnErrorResponse] Expired password detected, initiating password regeneration process")

		cNewGenPasswd, iChVal := RM.FnChngExpPasswdReq()
		if iChVal != nil {
			RM.LM.LogError(RM.ServiceName, "[FnErrorResponse] Failed to change expired password: %v", iChVal)
			RM.LM.LogInfo(RM.ServiceName, "Error encountered during password change request")
			return -1
		}

		c_msg = fmt.Sprintf("|%s - Password has been regenerated. Please re-login. New password is: %s|", timestamp, cNewGenPasswd)

		if initResult := RM.FnInsertTradeMessage(RM.C_xchng_cd, brkrID, trader_msg, c_msg, timestamp); initResult != nil {
			RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Failed to insert trade message for password change : %v", initResult)
			return -1
		}

		RM.LM.LogInfo(RM.ServiceName, "[FnErrorResponse] Password successfully regenerated and trade message inserted")

	}

	RM.LM.LogInfo(RM.ServiceName, "[FnErrorResponse] Exiting Log On Error Response Handling Function")
	return -1
}

func (RM *RecvManager) FnInsertTradeMessage(exchangeCode, brokerID, messageID, message, timestamp string) error {

	cMsg := fmt.Sprintf("|%s|%s- %s|", timestamp, messageID, message)

	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Exchange Code: %s", exchangeCode)
	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Broker ID: %s", brokerID)
	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Message ID: %s", messageID)
	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Message: %s", cMsg)
	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Timestamp: %s", timestamp)

	tranStatus := RM.TCM.FnBeginTran()
	if tranStatus != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Failed to begin transaction")
		return fmt.Errorf("failed to begin transaction")
	}

	query := `
        INSERT INTO ftm_fo_trd_msg (ftm_xchng_cd, ftm_brkr_cd, ftm_msg_id, ftm_msg, ftm_tm)
        VALUES (?, ?, ?, ?, to_timestamp(?, 'DD-Mon-YYYY HH24:MI:SS'))
    `

	result := RM.Db.Exec(query, exchangeCode, brokerID, messageID, cMsg, timestamp)
	if result.Error != nil {
		RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Error executing insert: %v", result.Error)
		if abortResult := RM.TCM.FnAbortTran(); abortResult == -1 {
			RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Failed to abort transaction")
		}
		return result.Error
	}

	commitStatus := RM.TCM.FnCommitTran()
	if commitStatus != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Transaction commit failed")
		return fmt.Errorf("transaction commit failed")
	}

	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Inserted message successfully")
	return nil
}

func (RM *RecvManager) FnChngExpPasswdReq() (string, error) {
	var currentPassword string

	tranStatus := RM.TCM.FnBeginTran()
	if tranStatus != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Failed to begin transaction")
		return "", fmt.Errorf("failed to begin transaction")
	}

	query := `SELECT OPM_EXG_PWD FROM OPM_ORD_PIPE_MSTR WHERE OPM_PIPE_ID = ?`
	if err := RM.Db.Raw(query, RM.PipeId).Scan(&currentPassword).Error; err != nil {
		RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Error fetching current password: %v", err)
		if abortResult := RM.TCM.FnAbortTran(); abortResult == -1 {
			RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Failed to abort transaction")
		}
		return "", fmt.Errorf("error fetching current password: %v", err)
	}

	decryptedCurrentPassword := RM.PUM.Fndecrypt(currentPassword)
	RM.LM.LogInfo(RM.ServiceName, "[FnChngExpPasswdReq] Decrypted current password: %s", decryptedCurrentPassword)

	newPassword, err := RM.PUM.FngenerateNewPassword(decryptedCurrentPassword)
	if err != nil {
		RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Error generating new password: %v", err)
		if abortResult := RM.TCM.FnAbortTran(); abortResult == -1 {
			RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Failed to abort transaction")
		}
		return "", err
	}

	encryptedNewPassword := RM.PUM.Fnencrypt(newPassword)
	RM.LM.LogInfo(RM.ServiceName, "[FnChngExpPasswdReq] Generated new password (encrypted): %s", encryptedNewPassword)

	updateQuery := `UPDATE OPM_ORD_PIPE_MSTR SET OPM_NEW_EXG_PWD = ? WHERE OPM_PIPE_ID = ?`
	if err := RM.Db.Exec(updateQuery, encryptedNewPassword, RM.PipeId).Error; err != nil {
		RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Error updating new password: %v", err)
		if abortResult := RM.TCM.FnAbortTran(); abortResult == -1 {
			RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Failed to abort transaction")
		}
		return "", fmt.Errorf("error updating new password: %v", err)
	}

	if commitStatus := RM.TCM.FnCommitTran(); commitStatus != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Error committing transaction")
		return "", fmt.Errorf("error committing transaction")
	}

	decryptedNewPassword := RM.PUM.Fndecrypt(encryptedNewPassword)

	errForPasswordChange := RM.PUM.FnwritePasswordChangeToFile(RM.PipeId, decryptedNewPassword)
	if errForPasswordChange != nil {
		RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Failed to log password change: %v", errForPasswordChange)
	}

	return newPassword, nil
}

func (RM *RecvManager) FnSystemInformationOut() int {

	// Step 1: Extract Normal Market Status
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Step 1: Extracting Normal Market Status")
	siNormalMktStts := RM.stSysInfoDat.StMktStts.SiNormal
	siOddMktStts := RM.stSysInfoDat.StMktStts.SiOddlot
	siSpotMktStts := RM.stSysInfoDat.StMktStts.SiSpot
	siAuctionMktStts := RM.stSysInfoDat.StMktStts.SiAuction

	// Log Market Status for Debugging
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Normal Market Status - Normal: %d", siNormalMktStts)
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Normal Market Status - Oddlot: %d", siOddMktStts)
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Normal Market Status - Spot: %d", siSpotMktStts)
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Normal Market Status - Auction: %d", siAuctionMktStts)

	/*// Exercise Market Status
	siExrMktStts := stSysInfoDat.StExMktStts.SiNormal
	siExrOddMktStts := stSysInfoDat.StExMktStts.SiOddlot
	siExrSpotMktStts := stSysInfoDat.StExMktStts.SiSpot
	siExrAuctionMktStts := stSysInfoDat.StExMktStts.SiAuction

	if DebugMsgLvl3 {
		logMessage(RM.ServiceName, "The Exer si_mkt_stts is :%d:", siExrMktStts)
		logMessage(RM.ServiceName, "The Exer si_mkt_stts is :%d, %d:", stSysInfoDat.StExMktStts.SiOddlot, siExrOddMktStts)
		logMessage(RM.ServiceName, "The Exer si_mkt_stts is :%d, %d:", stSysInfoDat.StExMktStts.SiSpot, siExrSpotMktStts)
		logMessage(RM.ServiceName, "The Exer si_mkt_stts is :%d, %d:", stSysInfoDat.StExMktStts.SiAuction, siExrAuctionMktStts)
	}

	liMktType = ExerciseMkt

	iChVal = FnMrktSttsHndlr(siExrMktStts, liMktType, sqlCXchngCd, RM.ServiceName, errMsg)
	if iChVal == -1 {
		logMessage(RM.ServiceName, "Failed While Calling Function FnMrktSttsHndlr")
		return -1
	}

	// Portfolio Market Status
	siPlMktStts := stSysInfoDat.StPlMktStts.SiNormal
	siPlOddMktStts := stSysInfoDat.StPlMktStts.SiOddlot
	siPlSpotMktStts := stSysInfoDat.StPlMktStts.SiSpot
	siPlAuctionMktStts := stSysInfoDat.StPlMktStts.SiAuction

	if DebugMsgLvl3 {
		logMessage(RM.ServiceName, "The PL si_mkt_stts is :%d:", siPlMktStts)
		logMessage(RM.ServiceName, "The PL si_mkt_stts is :%d, %d:", stSysInfoDat.StPlMktStts.SiOddlot, siPlOddMktStts)
		logMessage(RM.ServiceName, "The PL si_mkt_stts is :%d, %d:", stSysInfoDat.StPlMktStts.SiSpot, siPlSpotMktStts)
		logMessage(RM.ServiceName, "The PL si_mkt_stts is :%d, %d:", stSysInfoDat.StPlMktStts.SiAuction, siPlAuctionMktStts)
	}

	liMktType = PlMkt
	*/

	// Step 2: Extract Stream Number
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Step 2: Handling Stream Number")
	liStreamNo := int64(RM.stSysInfoDat.StHdr.C_alpha_char[0])
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Header Stream Char[0]: %c", RM.stSysInfoDat.StHdr.C_alpha_char[0])
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Converted Stream Number: %d", liStreamNo)
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Header Stream: %s", string(RM.stSysInfoDat.StHdr.C_alpha_char[:]))

	// Step 3: Begin Transaction
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Step 3: Beginning Transaction")
	iTransaction := RM.TCM.FnBeginTran()
	if iTransaction == -1 {
		RM.LM.LogError(RM.ServiceName, "[FnSystemInformationOut] Failed to begin transaction")
		return -1
	}

	// Step 4: Update Exchange Master Table
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Step 4: Updating Exchange Master Table")
	updateExchangeMasterQuery := `
    UPDATE exg_xchng_mstr 
    SET exg_def_stlmnt_pd = ?, 
        exg_wrn_pcnt = ?, 
        exg_vol_frz_pcnt = ?, 
        exg_brd_lot_qty = ?, 
        exg_tck_sz = ?, 
        exg_gtc_dys = ?, 
        exg_dsclsd_qty_pcnt = ?
    WHERE exg_xchng_cd = ?`

	if err := RM.Db.Exec(updateExchangeMasterQuery,
		RM.stSysInfoDat.SiDefaultSttlmntPeriodNm,
		RM.stSysInfoDat.SiWarningPercent,
		RM.stSysInfoDat.SiVolumeFreezePercent,
		RM.stSysInfoDat.LiBoardLotQuantity,
		RM.stSysInfoDat.LiTickSize,
		RM.stSysInfoDat.SiMaximumGtcDays,
		RM.stSysInfoDat.SiDisclosedQuantityPercentAllowed,
		RM.C_xchng_cd).Error; err != nil {
		RM.LM.LogError(RM.ServiceName, "[FnSystemInformationOut] Error updating exchange master: %v", err)
		RM.TCM.FnAbortTran()
		return -1
	}

	// Step 5: Update Order Pipe Master Table
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Step 5: Updating Order Pipe Master Table")
	updateOrderPipeMasterQuery := `
    UPDATE OPM_ORD_PIPE_MSTR 
    SET OPM_STREAM_NO = ? 
    WHERE OPM_PIPE_ID = ? 
      AND OPM_XCHNG_CD = ?`

	if err := RM.Db.Exec(updateOrderPipeMasterQuery,
		liStreamNo,
		RM.PipeId,
		RM.C_xchng_cd).Error; err != nil {
		RM.LM.LogError(RM.ServiceName, "[FnSystemInformationOut] Error updating order pipe master: %v", err)
		RM.TCM.FnAbortTran()
		return -1
	}

	// Step 6: Commit Transaction
	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Step 6: Committing Transaction")
	if RM.TCM.FnCommitTran() == -1 {
		RM.LM.LogError(RM.ServiceName, "[FnSystemInformationOut] Failed to commit transaction")
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, "[FnSystemInformationOut] Transaction completed successfully")
	return 0
}

func (RM *RecvManager) FnPartialLogonRes() int {
	var msg string
	var tradeMsg string

	RM.LM.LogInfo(RM.ServiceName, "[FnPartialLogonRes] Inside Function FnPartialLogonRes")

	msg = "SID information download failed"

	RM.LM.LogInfo(RM.ServiceName, "[FnPartialLogonRes] MSG Is : %s", msg)
	RM.OCM.ConvertIntHeaderToHostOrder(&RM.stSysInfoDat.StHdr)

	err, timestampStr := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", RM.stSysInfoDat.StHdr.Li_log_time))
	if err != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnPartialLogonRes] Error received while parsing the log time")
		return -1
	}

	tradeMsg = fmt.Sprintf("|%d| |%s|", timestampStr, msg)

	RM.LM.LogInfo(RM.ServiceName, "[FnPartialLogonRes] Trade Msg Is : %s", tradeMsg)

	return 0
}

func (RM *RecvManager) FnLocalDBRes() int {
	var msg string
	var tradeMsg string

	RM.LM.LogInfo(RM.ServiceName, "[FnLocalDBRes] Inside Function FnLocalDBRes")

	msg = "LDB information downloaded"

	RM.LM.LogInfo(RM.ServiceName, "[FnLocalDBRes] MSG Is: %s", msg)

	RM.OCM.ConvertIntHeaderToHostOrder(&RM.stLdbData.MessageHeader)

	err, timestampStr := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", RM.stLdbData.MessageHeader.Li_log_time))
	if err != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnLocalDBRes] Error received while parsing the log time")
		return -1
	}

	tradeMsg = fmt.Sprintf("|%d| %s|", timestampStr, msg)

	RM.LM.LogInfo(RM.ServiceName, "[FnLocalDBRes] Trade Msg Is: %s", tradeMsg)

	return 0
}
