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
	ServiceName       string
	Db                *gorm.DB
	TCM               *util.TransactionManager
	PUM               *util.PasswordUtilManger
	NetHDR            models.St_net_hdr
	St_sign_on_res    models.St_sign_on_res
	St_Error_Response *models.St_Error_Response
	Vw_mkt_msg        *models.Vw_mkt_msg
	LM                *util.LoggerManager
	OCM               *OrderConversion.OrderConversionManager
	TCUM              *typeconversionutil.TypeConversionUtilManager
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

func (RM *RecvManager) FnSignOnRequestOut(St_sign_on_res *models.St_sign_on_res, C_xchng_cd string, PipeId string) int {

	RM.LM.LogInfo(RM.ServiceName, "[FnSignOnRequestOut] Inside Log On Response Handling Function")

	err, LogTime := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", St_sign_on_res.St_hdr.Li_log_time))
	if err != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Error recieved while parsing the time ")
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, "Sign On Successful : Checking Other Details")
	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Broker Status Is : %c", St_sign_on_res.C_broker_status))
	RM.LM.LogInfo(RM.ServiceName, fmt.Sprintf("Clearing Status Is : %c", St_sign_on_res.C_clearing_status))

	c_message := "Logon Successful"

	TempTimeStamp := time.Unix(int64(LogTime), 0).Format("02-Jan-2006 15:04:05")

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

	query := `
	SELECT exg_brkr_id
	FROM exg_xchng_mstr
	WHERE exg_xchng_cd = ?
	`
	var brkrID string
	result := RM.Db.Raw(query, C_xchng_cd).Scan(&brkrID)
	if result.Error != nil {
		RM.LM.LogError(RM.ServiceName, " [FnSignOnRequestOut] [Error executing query: ... %v", result.Error)
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, " [FnSignOnRequestOut] exg_brkr_id: %s", brkrID)

	trader_msg := string(util.TRADER_MSG) // converting to string
	if initResult := RM.FnInsertTradeMessage(C_xchng_cd, brkrID, trader_msg, c_message, TempTimeStamp); initResult != nil {
		RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Error: %v", initResult)
		return -1
	}

	pwdLastChangeDate, _ := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", St_sign_on_res.Li_last_password_change_date))
	marketCloseDate, _ := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", St_sign_on_res.St_hdr.Li_log_time))
	exchangeTime, _ := RM.TCUM.LongToTimeArr(fmt.Sprintf("%d", St_sign_on_res.Li_end_time))

	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Password Last Change Date Is : %s", pwdLastChangeDate)
	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Market Closing Date Is : %s", marketCloseDate)
	RM.LM.LogInfo(RM.ServiceName, "[FnInsertTradeMessage] Exchange Time Is : %s", exchangeTime)

	tranStatus := RM.TCM.FnBeginTran()
	if tranStatus != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Failed to begin transaction")
		return -1
	}

	// Update OPM_ORD_PIPE_MSTR table
	updateQuery1 := `
    UPDATE OPM_ORD_PIPE_MSTR
    SET OPM_EXG_PWD = OPM_NEW_EXG_PWD,
        OPM_NEW_EXG_PWD = NULL
    WHERE OPM_PIPE_ID = ? AND OPM_XCHNG_CD = ? AND OPM_NEW_EXG_PWD IS NOT NULL`
	if result := RM.Db.Exec(updateQuery1, PipeId, C_xchng_cd); result.Error != nil {
		RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Error updating OPM_ORD_PIPE_MSTR: %v", result.Error)
		if abortResult := RM.TCM.FnAbortTran(); abortResult == -1 {
			RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Failed to abort transaction")
		}
		return -1
	}

	// Update if broker status is ACTIVE
	if St_sign_on_res.C_broker_status == util.ACTIVE {
		updateQuery2 := `
        UPDATE opm_ord_pipe_mstr
        SET opm_lst_pswd_chg_dt = TO_DATE(?, 'dd-Mon-yyyy hh24:mi:ss'),
            opm_login_stts = 1
        WHERE opm_pipe_id = ?`
		if result := RM.Db.Exec(updateQuery2, pwdLastChangeDate, PipeId); result.Error != nil {
			RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Error updating opm_ord_pipe_mstr: %v", result.Error)
			if abortResult := RM.TCM.FnAbortTran(); abortResult == -1 {
				RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Failed to abort transaction")
			}
			return -1
		}
	}

	// Update Broker Status in exg_xchng_mstr table
	if St_sign_on_res.C_broker_status == util.ACTIVE || St_sign_on_res.C_broker_status == util.CLOSE_OUT || St_sign_on_res.C_broker_status == util.SUSPEND {
		updateQuery3 := `
        UPDATE exg_xchng_mstr
        SET exg_brkr_stts = ?
        WHERE EXG_XCHNG_CD = ?`
		if result := RM.Db.Exec(updateQuery3, St_sign_on_res.C_broker_status, C_xchng_cd); result.Error != nil {
			RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Error updating exg_xchng_mstr: %v", result.Error)
			if abortResult := RM.TCM.FnAbortTran(); abortResult == -1 {
				RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Failed to abort transaction")
			}
			return -1
		}
	}

	// Commit transaction
	commitStatus := RM.TCM.FnCommitTran()
	if commitStatus != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnSignOnRequestOut] Transaction commit failed")
		return -1
	}

	RM.LM.LogInfo(RM.ServiceName, "[FnSignOnRequestOut] Exiting Log On Response Handling Function")

	return 0
}

func (RM *RecvManager) FnErrorResponse(St_Error_Response *models.St_Error_Response, C_xchng_cd string, PipeId string) int {
	RM.LM.LogInfo(RM.ServiceName, "[FnErrorResponse] Inside Log On Error Response Handling Function")

	ErrMsg := strings.TrimSpace(string(St_Error_Response.C_ErrorMessage[:]))

	timestampQuery := `
		SELECT to_char(CURRENT_TIMESTAMP, 'DD-Mon-YYYY HH24:MI:SS')
	`
	var timestamp string
	result := RM.Db.Raw(timestampQuery).Scan(&timestamp)
	if result.Error != nil {
		RM.LM.LogError(RM.ServiceName, "[FnErrorResponse] Error executing timestamp query: %v", result.Error)
		return -1
	}

	c_msg := fmt.Sprintf("|%s|%d- %s|", timestamp, St_Error_Response.St_Hdr.Si_error_code, ErrMsg)

	brkrIDQuery := `
		SELECT exg_brkr_id
		FROM exg_xchng_mstr
		WHERE exg_xchng_cd = ?
	`
	var brkrID string
	result = RM.Db.Raw(brkrIDQuery, C_xchng_cd).Scan(&brkrID)
	if result.Error != nil {
		RM.LM.LogError(RM.ServiceName, "[FnErrorResponse] Error executing broker ID query: %v", result.Error)
		return -1
	}

	trader_msg := string(util.TRADER_MSG)

	if initResult := RM.FnInsertTradeMessage(C_xchng_cd, brkrID, trader_msg, c_msg, timestamp); initResult != nil {
		RM.LM.LogError(RM.ServiceName, "[FnInsertTradeMessage] Failed to insert trade message : %v", initResult)
		return -1
	}

	if St_Error_Response.St_Hdr.Si_error_code == 16053 {
		RM.LM.LogInfo(RM.ServiceName, "[FnErrorResponse] Expired password detected, initiating password regeneration process")

		cNewGenPasswd, iChVal := RM.FnChngExpPasswdReq(PipeId)
		if iChVal != nil {
			RM.LM.LogError(RM.ServiceName, "[FnErrorResponse] Failed to change expired password: %v", iChVal)
			RM.LM.LogInfo(RM.ServiceName, "Error encountered during password change request")
			return -1
		}

		c_msg = fmt.Sprintf("|%s - Password has been regenerated. Please re-login. New password is: %s|", timestamp, cNewGenPasswd)

		if initResult := RM.FnInsertTradeMessage(C_xchng_cd, brkrID, trader_msg, c_msg, timestamp); initResult != nil {
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

func (RM *RecvManager) FnChngExpPasswdReq(pipeID string) (string, error) {
	var currentPassword string

	tranStatus := RM.TCM.FnBeginTran()
	if tranStatus != 0 {
		RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Failed to begin transaction")
		return "", fmt.Errorf("failed to begin transaction")
	}

	query := `SELECT OPM_EXG_PWD FROM OPM_ORD_PIPE_MSTR WHERE OPM_PIPE_ID = ?`
	if err := RM.Db.Raw(query, pipeID).Scan(&currentPassword).Error; err != nil {
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
	if err := RM.Db.Exec(updateQuery, encryptedNewPassword, pipeID).Error; err != nil {
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

	errForPasswordChange := RM.PUM.FnwritePasswordChangeToFile(pipeID, decryptedNewPassword)
	if errForPasswordChange != nil {
		RM.LM.LogError(RM.ServiceName, "[FnChngExpPasswdReq] Failed to log password change: %v", errForPasswordChange)
	}

	return newPassword, nil
}
