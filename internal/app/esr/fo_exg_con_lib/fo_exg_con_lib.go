package foexgconlib

import (
	"database/sql"
	"fmt"
	"reflect"
	"strconv"
	"strings"

	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"

	"gorm.io/gorm"
)

const defaultTimeStamp = "0"

type FoExgConLibManager struct {
	St_sign_on_req *models.St_sign_on_req
	St_req_q_data  *models.St_req_q_data
	St_exch_msg    *models.St_exch_msg
	int_header     *models.St_int_header

	PUM           *util.PasswordUtilManger // initialize it where ever you are initializing 'FECLM'
	DB            *gorm.DB
	LoggerManager *util.LoggerManager
	ServiceName   string
	IpPipeID      string
	UserID        int64
	//----------------------------------
	Opm_loginStatus    int
	Opm_userID         int64
	Opm_existingPasswd string
	Opm_newPasswd      sql.NullString
	Opm_LstPswdChgDt   string
	Opm_XchngCd        string
	Opm_TrdrID         string
	Opm_BrnchID        int64
	//---------------------------------------
	Exg_BrkrID     string
	Exg_NxtTrdDate string
	Exg_BrkrName   string
	Exg_BrkrStts   string
}

func (FECLM *FoExgConLibManager) LogOnToTap() int {
	/*
		what variables i have to find 'from where they are getting value'
		 1. IpPipeID (in c code it is 'ql_c_ip_pipe_id'. in con_clnt we are setting it from the `args`)
		 2. UserID

		 ** If these values are not found the refer to the Service .
	*/

	// Step 1. we are checking the ESR Status
	// Step 2: Check password expiration and prompt for password change if required
	// Step 3: Fetch login status, user ID, and password and NewPassword from the database and validate them.
	// Step4: we are fetching 4 feilds from `opm_ord_pipe_mstr` (XchngCd, LstPswdChgDt, TrdrID, BrnchID)
	//	and 4 fields from `exg_xchng_mstr` (exg_brkr_id, exg_nxt_trd_dt, exg_brkr_name, exg_brkr_stts)

	// Step 1:  Query for checking if ESR is running
	queryForBPS_Stts := `
		SELECT bps_stts
		FROM bps_btch_pgm_stts
		WHERE bps_pgm_name = ?
		AND bps_xchng_cd = 'NA'
		AND bps_pipe_id = ?
	`

	var bpsStts string
	result := FECLM.DB.Raw(queryForBPS_Stts, "cln_esr_clnt", FECLM.IpPipeID).Scan(&bpsStts)

	if result.Error != nil {
		errorMsg := fmt.Sprintf("Error executing query: %v", result.Error)
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "[LogOnToTap] %s", errorMsg)
		return -1
	}

	if bpsStts == "N" {
		errorMsg := fmt.Sprintf("ESR client is not running for Pipe ID: %s and User ID: %d", FECLM.IpPipeID, FECLM.UserID)
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "[LogOnToTap] %s", errorMsg)
		return -1
	}

	successMsg := fmt.Sprintf("ESR client is running for Pipe ID: %s and User ID: %d", FECLM.IpPipeID, FECLM.UserID)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "[LogOnToTap] %s", successMsg)

	/**** Step2: Checking if user is already logged on in console or not ****/

	var exgMaxPswdVldDays int
	queryForExgMaxPswdVldDays := `
		SELECT exg_max_pswd_vld_days
		FROM opm_ord_pipe_mstr, exg_xchng_mstr
		WHERE opm_pipe_id = ?
		AND opm_xchng_cd = exg_xchng_cd
		AND exg_max_pswd_vld_days > (date_trunc('day', current_date) - opm_lst_pswd_chg_dt)
	`
	errForExgMaxPswdVldDays := FECLM.DB.Raw(queryForExgMaxPswdVldDays, FECLM.IpPipeID).Scan(&exgMaxPswdVldDays).Error

	if errForExgMaxPswdVldDays != nil {
		/**** There is certain time limit of Days for the password after that password will be updated ****/
		if errForExgMaxPswdVldDays == gorm.ErrRecordNotFound {

			FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "[LogOnToTap] No record found, changing password")

			var currentPassword string
			queryFor_opm_exg_pwd :=
				`SELECT opm_exg_pwd
				FROM opm_ord_pipe_mstr
				WHERE opm_pipe_id = ?
				AND opm_xchng_cd = 'NFO' `

			errFor_opm_exg_pwd := FECLM.DB.Raw(queryFor_opm_exg_pwd, FECLM.IpPipeID).Scan(&currentPassword).Error

			if errFor_opm_exg_pwd != nil {
				FECLM.LoggerManager.LogError(FECLM.ServiceName, "[LogOnToTap] Error fetching current password: %v", errFor_opm_exg_pwd)
				return -1
			}

			FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Existing password: %s", currentPassword)

			decryptedPassword := FECLM.PUM.Fndecrypt(currentPassword)

			newPassword, errFor_generateNewPassword := FECLM.PUM.FngenerateNewPassword(decryptedPassword)
			if errFor_generateNewPassword != nil {

				FECLM.LoggerManager.LogError(FECLM.ServiceName, "Failed to generate new password: %v", errFor_generateNewPassword)
				return -1
			}

			FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "New password generated: %s", newPassword)

			encryptedPassword := FECLM.PUM.Fnencrypt(newPassword)
			FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Encrypted new password: %s", encryptedPassword)

			tx := FECLM.DB.Begin()

			queryforSET_opm_new_exg_pwd :=
				`	UPDATE opm_ord_pipe_mstr
					SET opm_new_exg_pwd = ?
					WHERE opm_pipe_id = ?
					AND opm_xchng_cd = 'NFO'
				`

			errforSET_opm_new_exg_pwd := tx.Exec(queryforSET_opm_new_exg_pwd, encryptedPassword, FECLM.IpPipeID).Error

			if errforSET_opm_new_exg_pwd != nil {
				FECLM.LoggerManager.LogError(FECLM.ServiceName, "[LogOnToTap] Error updating new password: %v", errforSET_opm_new_exg_pwd)
				tx.Rollback()
				return -1
			}

			if err := tx.Commit().Error; err != nil {
				FECLM.LoggerManager.LogError(FECLM.ServiceName, "[LogOnToTap] Transaction commit failed: %v", err)
				return -1
			}

			FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Decrypted new password: %s", FECLM.PUM.Fndecrypt(encryptedPassword))
			errForPasswordChange := FECLM.PUM.FnwritePasswordChangeToFile(FECLM.IpPipeID, newPassword)
			if errForPasswordChange != nil {
				FECLM.LoggerManager.LogError("PasswordChange", "Failed to log password change: %v", errForPasswordChange)
			}

			successMsg := fmt.Sprintf("Password changed successfully for Pipe ID: %s", FECLM.IpPipeID)
			FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "[LogOnToTap] %s", successMsg)

		} else {
			FECLM.LoggerManager.LogError(FECLM.ServiceName, "[LogOnToTap] SQL error: %v", errForExgMaxPswdVldDays)
			return -1
		}
	}

	// step 3: Fetch login status, user ID, and passwords from the database and validating these values
	queryFor_opm_ord_pipe_mstr_Vars :=
		`SELECT opm_login_stts, opm_user_id, opm_exg_pwd, opm_new_exg_pwd
												FROM opm_ord_pipe_mstr
												WHERE opm_pipe_id = ?
											`
	row := FECLM.DB.Raw(queryFor_opm_ord_pipe_mstr_Vars, FECLM.IpPipeID).Row()

	errFor_opm_ord_pipe_mstr_Vars := row.Scan(&FECLM.Opm_loginStatus, &FECLM.Opm_userID, &FECLM.Opm_existingPasswd, &FECLM.Opm_newPasswd)

	if errFor_opm_ord_pipe_mstr_Vars != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "SQL Error: %v", errFor_opm_ord_pipe_mstr_Vars)
		return -1
	}

	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Fetched new password: %s", FECLM.Opm_newPasswd.String)

	if FECLM.UserID != FECLM.Opm_userID {
		errorMsg := fmt.Sprintf("Incorrect User ID for Pipe ID: %s, expected: %d, got: %d", FECLM.IpPipeID, FECLM.Opm_userID, FECLM.UserID)
		FECLM.LoggerManager.LogError(FECLM.ServiceName, errorMsg)
		return -1
	}

	if FECLM.Opm_loginStatus == 1 {
		errorMsg := fmt.Sprintf("User is already logged in for Pipe ID: %s, User ID: %d", FECLM.IpPipeID, FECLM.Opm_userID)
		FECLM.LoggerManager.LogError(FECLM.ServiceName, errorMsg)
		return -1
	}

	FECLM.Opm_existingPasswd = strings.TrimSpace(FECLM.Opm_existingPasswd)
	if FECLM.Opm_newPasswd.Valid {
		FECLM.Opm_newPasswd.String = strings.TrimSpace(FECLM.Opm_newPasswd.String)
	}

	decryptedExistingPasswd := FECLM.PUM.Fndecrypt(FECLM.Opm_existingPasswd)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Decrypted existing password: %s", decryptedExistingPasswd)

	if FECLM.Opm_newPasswd.Valid {
		decryptedNewPasswd := FECLM.PUM.Fndecrypt(FECLM.Opm_newPasswd.String)
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Decrypted new password: %s", decryptedNewPasswd)
	}

	// Step4: we are fetching 4 feilds from `opm_ord_pipe_mstr` (XchngCd, LstPswdChgDt, TrdrID, BrnchID)
	//	and 4 fields from `exg_xchng_mstr` (exg_brkr_id, exg_nxt_trd_dt, exg_brkr_name, exg_brkr_stts)

	queryForOpmOrdPipeMstr := `
		SELECT opm_xchng_cd, 
		       to_char(opm_lst_pswd_chg_dt, 'dd-mon-yyyy'),
		       opm_trdr_id,
		       opm_brnch_id
		FROM opm_ord_pipe_mstr
		WHERE opm_pipe_id = ?
	`
	rowForOpmOrdPipeMstr := FECLM.DB.Raw(queryForOpmOrdPipeMstr, FECLM.IpPipeID).Row()

	errForOpmOrdPipeMstr := rowForOpmOrdPipeMstr.Scan(&FECLM.Opm_XchngCd, &FECLM.Opm_LstPswdChgDt, &FECLM.Opm_TrdrID, &FECLM.Opm_BrnchID)
	if errForOpmOrdPipeMstr != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "SQL Error: %v", errForOpmOrdPipeMstr)
		return -1
	}

	queryForExgXchngMstr := `
		SELECT exg_brkr_id, 
		       exg_nxt_trd_dt, 
		       nvl(exg_brkr_name, ' '), 
		       exg_brkr_stts 
		FROM exg_xchng_mstr 
		WHERE exg_xchng_cd = ?
	`
	rowExg := FECLM.DB.Raw(queryForExgXchngMstr, FECLM.Opm_XchngCd).Row()

	errForExgXchngMstr := rowExg.Scan(FECLM.Exg_BrkrID, FECLM.Exg_NxtTrdDate, FECLM.Exg_BrkrName, FECLM.Exg_BrkrStts)
	if errForExgXchngMstr != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "SQL Error: %v", errForExgXchngMstr)
		return -1
	}

	if FECLM.Exg_BrkrName == "" {
		FECLM.Exg_BrkrName = " "
	}

	// Step 5 : assigning the values to the header (int_header)

	traderID, err := strconv.ParseInt(FECLM.Opm_TrdrID, 10, 32)
	if err != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "Failed to convert Opm_TrdrID to int32: %v", err)
		return -1
	}
	FECLM.int_header.Li_trader_id = int32(traderID)                                         // 1 HDR
	FECLM.int_header.Li_log_time = 0                                                        // 2 HDR
	CopyAndFormatSymbol(FECLM.int_header.C_alpha_char[:], util.LEN_ALPHA_CHAR, " ")         // 3 HDR
	FECLM.int_header.Si_transaction_code = util.SIGN_ON_REQUEST_IN                          // 4 HDR
	FECLM.int_header.Si_error_code = 0                                                      // 5 HDR
	copy(FECLM.int_header.C_filler_2[:], "        ")                                        // 6 HDR
	copy(FECLM.int_header.C_time_stamp_1[:], defaultTimeStamp)                              // 7 HDR
	copy(FECLM.int_header.C_time_stamp_2[:], defaultTimeStamp)                              // 8 HDR
	FECLM.int_header.Si_message_length = int16(reflect.TypeOf(FECLM.St_sign_on_req).Size()) // 9 HDR

	//********************** Header Done Till Here ********************/

	FECLM.St_sign_on_req.Li_user_id = FECLM.Opm_userID                                                     // 1 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_reserved_1[:], 8, " ")                                      // 2 BDY
	CopyAndFormatPassword(FECLM.St_sign_on_req.C_password[:], util.LEN_PASSWORD, FECLM.Opm_existingPasswd) // 3 BDY
	CopyAndFormatPassword(FECLM.St_sign_on_req.C_new_password[:], util.LEN_PASSWORD, FECLM.Opm_newPasswd)  // 4 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_trader_name[:], util.LEN_TRADER_NAME, FECLM.Opm_TrdrID)     // 5 BDY
	FECLM.St_sign_on_req.Li_last_password_change_date = 0                                                  // 6 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_broker_id[:], util.LEN_BROKER_ID, FECLM.Exg_BrkrID)         // 7 BDY
	FECLM.St_sign_on_req.C_filler_1 = ' '                                                                  // 8 BDY
	FECLM.St_sign_on_req.Si_branch_id = int16(FECLM.Opm_BrnchID)                                           // 9 BDY

	// Logging for debug
	if FECLM.DebugMsgLevel >= 3 {
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Sign-on request message length: %d", FECLM.St_sign_on_req.St_hdr.Si_message_length)
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "User ID: %d", FECLM.St_sign_on_req.Li_user_id)
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Password: %s", FECLM.St_sign_on_req.C_password)
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "New Password: %s", FECLM.St_sign_on_req.C_new_password)
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Trader Name: %s", FECLM.St_sign_on_req.C_trader_name)
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Last Password Change Date: %d", FECLM.St_sign_on_req.Li_last_password_change_date)
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Broker ID: %s", FECLM.St_sign_on_req.C_broker_id)
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Filler 1: %c", FECLM.St_sign_on_req.C_filler_1)
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Branch ID: %d", FECLM.St_sign_on_req.Si_branch_id)
	}

	// Fetch version number
	versionStr := FECLM.FetchProcessSpace(FECLM.ServiceName, "VERSION_NUMBER")
	if versionStr == "" {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "Failed to retrieve VERSION_NUMBER")
		return -1
	}
	FECLM.St_sign_on_req.Li_version_number, _ = strconv.ParseInt(versionStr, 10, 64)

	// Set default values
	FECLM.St_sign_on_req.Li_batch_2_start_time = 0
	FECLM.St_sign_on_req.C_host_switch_context = ' '

	// Format color field
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_colour[:], LEN_COLOUR, " ")

	// Set second filler
	FECLM.St_sign_on_req.C_filler_2 = ' '

	// Fetch and set user type
	userTypeStr := FECLM.FetchProcessSpace(FECLM.ServiceName, "USER_TYPE")
	if userTypeStr == "" {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "Failed to retrieve USER_TYPE")
		return -1
	}
	FECLM.St_sign_on_req.Si_user_type, _ = strconv.Atoi(userTypeStr)

	// Set sequence number
	FECLM.St_sign_on_req.D_sequence_number = 0

	// Fetch and set workstation class name
	wsClassName := FECLM.FetchProcessSpace(FECLM.ServiceName, "WSC_NAME")
	if wsClassName == "" {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "Failed to retrieve WSC_NAME")
		return -1
	}
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_ws_class_name[:], LEN_WS_CLASS_NAME, wsClassName)

	// Set broker status
	FECLM.St_sign_on_req.C_broker_status = ' '

	return 0
}

func CopyAndFormatSymbol(dest []byte, destLen int, src string) { //'fn_orstonse_char'

	srcUpper := strings.ToUpper(src)

	copyLen := len(srcUpper)
	if copyLen > destLen {
		copyLen = destLen
	}

	copy(dest, srcUpper[:copyLen])

	for i := copyLen; i < destLen; i++ {
		dest[i] = ' '
	}
}
