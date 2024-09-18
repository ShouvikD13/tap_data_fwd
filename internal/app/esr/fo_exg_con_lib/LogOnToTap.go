package foexgconlib

import (
	"crypto/md5"
	"database/sql"
	"encoding/json"
	"fmt"
	"io"
	"reflect"
	"strconv"
	"strings"
	"unsafe"

	ordercinversion "DATA_FWD_TAP/internal/OrderConversion"
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"DATA_FWD_TAP/util/MessageQueue"

	"gorm.io/gorm"
)

const defaultTimeStamp = "0"

type LogOnToTapManager struct {
	ServiceName                string
	St_sign_on_req             *models.St_sign_on_req
	St_req_q_data              *models.St_req_q_data
	St_exch_msg                *models.St_exch_msg
	int_header                 *models.St_int_header
	St_net_hdr                 *models.St_net_hdr
	St_BrokerEligibilityPerMkt *models.St_broker_eligibility_per_mkt
	MQM                        *MessageQueue.MessageQueueManager
	EM                         *util.EnvironmentManager
	PUM                        *util.PasswordUtilManger // initialize it where ever you are initializing 'LOTTM'
	DB                         *gorm.DB
	LoggerManager              *util.LoggerManager
	OCM                        *ordercinversion.OrderConversionManager
	Ser                        string
	C_pipe_id                  string
	UserID                     int64
	mtype                      *int64
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
	Exg_NxtTrdDate string // <--
	Exg_BrkrName   string
	Exg_BrkrStts   string
}

func (LOTTM *LogOnToTapManager) LogOnToTap() int {
	/*
		what variables i have to find 'from where they are getting value'
		 1. C_pipe_id (in c code it is 'ql_c_ip_pipe_id'. in con_clnt we are setting it from the `args`)
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
	result := LOTTM.DB.Raw(queryForBPS_Stts, "cln_esr_clnt", LOTTM.C_pipe_id).Scan(&bpsStts)

	if result.Error != nil {
		errorMsg := fmt.Sprintf("Error executing query: %v", result.Error)
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "[LogOnToTap] %s", errorMsg)
		return -1
	}

	if bpsStts == "N" {
		errorMsg := fmt.Sprintf("ESR client is not running for Pipe ID: %s and User ID: %d", LOTTM.C_pipe_id, LOTTM.UserID)
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "[LogOnToTap] %s", errorMsg)
		return -1
	}

	successMsg := fmt.Sprintf("ESR client is running for Pipe ID: %s and User ID: %d", LOTTM.C_pipe_id, LOTTM.UserID)
	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "[LogOnToTap] %s", successMsg)

	/**** Step2: Checking if user is already logged on in console or not ****/

	var exgMaxPswdVldDays int
	queryForExgMaxPswdVldDays := `
		SELECT exg_max_pswd_vld_days
		FROM opm_ord_pipe_mstr, exg_xchng_mstr
		WHERE opm_pipe_id = ?
		AND opm_xchng_cd = exg_xchng_cd
		AND exg_max_pswd_vld_days > (date_trunc('day', current_date) - opm_lst_pswd_chg_dt)
	`
	errForExgMaxPswdVldDays := LOTTM.DB.Raw(queryForExgMaxPswdVldDays, LOTTM.C_pipe_id).Scan(&exgMaxPswdVldDays).Error

	if errForExgMaxPswdVldDays != nil {
		/**** There is certain time limit of Days for the password after that password will be updated ****/
		if errForExgMaxPswdVldDays == gorm.ErrRecordNotFound {

			LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "[LogOnToTap] No record found, changing password")

			var currentPassword string
			queryFor_opm_exg_pwd :=
				`SELECT opm_exg_pwd
				FROM opm_ord_pipe_mstr
				WHERE opm_pipe_id = ?
				AND opm_xchng_cd = 'NFO' `

			errFor_opm_exg_pwd := LOTTM.DB.Raw(queryFor_opm_exg_pwd, LOTTM.C_pipe_id).Scan(&currentPassword).Error

			if errFor_opm_exg_pwd != nil {
				LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "[LogOnToTap] Error fetching current password: %v", errFor_opm_exg_pwd)
				return -1
			}

			LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Existing password: %s", currentPassword)

			decryptedPassword := LOTTM.PUM.Fndecrypt(currentPassword)

			newPassword, errFor_generateNewPassword := LOTTM.PUM.FngenerateNewPassword(decryptedPassword)
			if errFor_generateNewPassword != nil {

				LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "Failed to generate new password: %v", errFor_generateNewPassword)
				return -1
			}

			LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "New password generated: %s", newPassword)

			encryptedPassword := LOTTM.PUM.Fnencrypt(newPassword)
			LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Encrypted new password: %s", encryptedPassword)

			tx := LOTTM.DB.Begin()

			queryforSET_opm_new_exg_pwd :=
				`	UPDATE opm_ord_pipe_mstr
					SET opm_new_exg_pwd = ?
					WHERE opm_pipe_id = ?
					AND opm_xchng_cd = 'NFO'
				`

			errforSET_opm_new_exg_pwd := tx.Exec(queryforSET_opm_new_exg_pwd, encryptedPassword, LOTTM.C_pipe_id).Error

			if errforSET_opm_new_exg_pwd != nil {
				LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "[LogOnToTap] Error updating new password: %v", errforSET_opm_new_exg_pwd)
				tx.Rollback()
				return -1
			}

			if err := tx.Commit().Error; err != nil {
				LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "[LogOnToTap] Transaction commit failed: %v", err)
				return -1
			}

			LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Decrypted new password: %s", LOTTM.PUM.Fndecrypt(encryptedPassword))
			errForPasswordChange := LOTTM.PUM.FnwritePasswordChangeToFile(LOTTM.C_pipe_id, newPassword)
			if errForPasswordChange != nil {
				LOTTM.LoggerManager.LogError("PasswordChange", "Failed to log password change: %v", errForPasswordChange)
			}

			successMsg := fmt.Sprintf("Password changed successfully for Pipe ID: %s", LOTTM.C_pipe_id)
			LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "[LogOnToTap] %s", successMsg)

		} else {
			LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "[LogOnToTap] SQL error: %v", errForExgMaxPswdVldDays)
			return -1
		}
	}

	// step 3: Fetch login status, user ID, and passwords from the database and validating these values
	queryFor_opm_ord_pipe_mstr_Vars :=
		`SELECT opm_login_stts, opm_user_id, opm_exg_pwd, opm_new_exg_pwd
												FROM opm_ord_pipe_mstr
												WHERE opm_pipe_id = ?
											`
	row := LOTTM.DB.Raw(queryFor_opm_ord_pipe_mstr_Vars, LOTTM.C_pipe_id).Row()

	errFor_opm_ord_pipe_mstr_Vars := row.Scan(&LOTTM.Opm_loginStatus, &LOTTM.Opm_userID, &LOTTM.Opm_existingPasswd, &LOTTM.Opm_newPasswd)

	if errFor_opm_ord_pipe_mstr_Vars != nil {
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "SQL Error: %v", errFor_opm_ord_pipe_mstr_Vars)
		return -1
	}

	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Fetched new password: %s", LOTTM.Opm_newPasswd.String)

	if LOTTM.UserID != LOTTM.Opm_userID {
		errorMsg := fmt.Sprintf("Incorrect User ID for Pipe ID: %s, expected: %d, got: %d", LOTTM.C_pipe_id, LOTTM.Opm_userID, LOTTM.UserID)
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, errorMsg)
		return -1
	}

	if LOTTM.Opm_loginStatus == 1 {
		errorMsg := fmt.Sprintf("User is already logged in for Pipe ID: %s, User ID: %d", LOTTM.C_pipe_id, LOTTM.Opm_userID)
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, errorMsg)
		return -1
	}

	LOTTM.Opm_existingPasswd = strings.TrimSpace(LOTTM.Opm_existingPasswd)
	if LOTTM.Opm_newPasswd.Valid {
		LOTTM.Opm_newPasswd.String = strings.TrimSpace(LOTTM.Opm_newPasswd.String)
	}

	decryptedExistingPasswd := LOTTM.PUM.Fndecrypt(LOTTM.Opm_existingPasswd)
	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Decrypted existing password: %s", decryptedExistingPasswd)

	if LOTTM.Opm_newPasswd.Valid {
		decryptedNewPasswd := LOTTM.PUM.Fndecrypt(LOTTM.Opm_newPasswd.String)
		LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Decrypted new password: %s", decryptedNewPasswd)
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
	rowForOpmOrdPipeMstr := LOTTM.DB.Raw(queryForOpmOrdPipeMstr, LOTTM.C_pipe_id).Row()

	errForOpmOrdPipeMstr := rowForOpmOrdPipeMstr.Scan(&LOTTM.Opm_XchngCd, &LOTTM.Opm_LstPswdChgDt, &LOTTM.Opm_TrdrID, &LOTTM.Opm_BrnchID)
	if errForOpmOrdPipeMstr != nil {
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "SQL Error: %v", errForOpmOrdPipeMstr)
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
	rowExg := LOTTM.DB.Raw(queryForExgXchngMstr, LOTTM.Opm_XchngCd).Row()

	errForExgXchngMstr := rowExg.Scan(LOTTM.Exg_BrkrID, LOTTM.Exg_NxtTrdDate, LOTTM.Exg_BrkrName, LOTTM.Exg_BrkrStts)
	if errForExgXchngMstr != nil {
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "SQL Error: %v", errForExgXchngMstr)
		return -1
	}

	if LOTTM.Exg_BrkrName == "" {
		LOTTM.Exg_BrkrName = " "
	}

	// Step 5 : assigning the values to the header (int_header)
	/********************** Body Starts ********************/
	/********************** Header Starts ********************/
	traderID, err := strconv.ParseInt(LOTTM.Opm_TrdrID, 10, 32)
	if err != nil {
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "Failed to convert Opm_TrdrID to int32: %v", err)
		return -1
	}
	LOTTM.int_header.Li_trader_id = int32(traderID)                                         // 1 HDR
	LOTTM.int_header.Li_log_time = 0                                                        // 2 HDR
	CopyAndFormatSymbol(LOTTM.int_header.C_alpha_char[:], util.LEN_ALPHA_CHAR, " ")         // 3 HDR
	LOTTM.int_header.Si_transaction_code = util.SIGN_ON_REQUEST_IN                          // 4 HDR
	LOTTM.int_header.Si_error_code = 0                                                      // 5 HDR
	copy(LOTTM.int_header.C_filler_2[:], "        ")                                        // 6 HDR
	copy(LOTTM.int_header.C_time_stamp_1[:], defaultTimeStamp)                              // 7 HDR
	copy(LOTTM.int_header.C_time_stamp_2[:], defaultTimeStamp)                              // 8 HDR
	LOTTM.int_header.Si_message_length = int16(reflect.TypeOf(LOTTM.St_sign_on_req).Size()) // 9 HDR

	/********************** Header Done ********************/

	LOTTM.St_sign_on_req.Li_user_id = LOTTM.Opm_userID                                                           // 1 BDY
	CopyAndFormatSymbol(LOTTM.St_sign_on_req.C_reserved_1[:], 8, " ")                                            // 2 BDY
	CopyAndFormatPassword(LOTTM.St_sign_on_req.C_password[:], util.LEN_PASSWORD, LOTTM.Opm_existingPasswd)       // 3 BDY
	CopyAndFormatSymbol(LOTTM.St_sign_on_req.C_reserved_2[:], 8, " ")                                            // 4 BDY
	CopyAndFormatPassword(LOTTM.St_sign_on_req.C_new_password[:], util.LEN_PASSWORD, LOTTM.Opm_newPasswd.String) // 5 BDY
	CopyAndFormatSymbol(LOTTM.St_sign_on_req.C_trader_name[:], util.LEN_TRADER_NAME, LOTTM.Opm_TrdrID)           // 6 BDY
	LOTTM.St_sign_on_req.Li_last_password_change_date = 0                                                        // 7 BDY
	CopyAndFormatSymbol(LOTTM.St_sign_on_req.C_broker_id[:], util.LEN_BROKER_ID, LOTTM.Exg_BrkrID)               // 8 BDY
	LOTTM.St_sign_on_req.C_filler_1 = ' '                                                                        // 9 BDY
	LOTTM.St_sign_on_req.Si_branch_id = int16(LOTTM.Opm_BrnchID)                                                 // 10 BDY

	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Sign-on request message length: %d", LOTTM.St_sign_on_req.St_hdr.Si_message_length)
	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "User ID: %d", LOTTM.St_sign_on_req.Li_user_id)
	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Password: %s", LOTTM.St_sign_on_req.C_password)
	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "New Password: %s", LOTTM.St_sign_on_req.C_new_password)
	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Trader Name: %s", LOTTM.St_sign_on_req.C_trader_name)
	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Last Password Change Date: %d", LOTTM.St_sign_on_req.Li_last_password_change_date)
	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Broker ID: %s", LOTTM.St_sign_on_req.C_broker_id)
	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Filler 1: %c", LOTTM.St_sign_on_req.C_filler_1)
	LOTTM.LoggerManager.LogInfo(LOTTM.ServiceName, "Branch ID: %d", LOTTM.St_sign_on_req.Si_branch_id)

	versionStr := LOTTM.EM.GetProcessSpaceValue("version", "VERSION_NUMBER")
	if versionStr == "" {
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "Failed to retrieve VERSION_NUMBER")
		return -1
	}
	LOTTM.St_sign_on_req.Li_version_number, _ = strconv.ParseInt(versionStr, 10, 64) // 11 BDY
	LOTTM.St_sign_on_req.Li_batch_2_start_time = 0                                   // 12 BDY
	LOTTM.St_sign_on_req.C_host_switch_context = ' '                                 // 13 BDY
	CopyAndFormatSymbol(LOTTM.St_sign_on_req.C_colour[:], util.LEN_COLOUR, " ")      // 14 BDY
	LOTTM.St_sign_on_req.C_filler_2 = ' '                                            // 15 BDY

	userTypeStr := LOTTM.EM.GetProcessSpaceValue("UserSettings", "USER_TYPE")
	userType, err := strconv.Atoi(userTypeStr)
	if err != nil {
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "Failed to convert USER_TYPE to int: %v", err)
		return -1
	}

	LOTTM.St_sign_on_req.Si_user_type = int16(userType) // 16 BDY
	LOTTM.St_sign_on_req.D_sequence_number = 0          // 17 BDY

	wsClassName := LOTTM.EM.GetProcessSpaceValue("Service", "WSC_NAME")
	if wsClassName == "" {
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, "Failed to retrieve WSC_NAME")
		return -1
	}

	CopyAndFormatSymbol(LOTTM.St_sign_on_req.C_ws_class_name[:], util.LEN_WS_CLASS_NAME, wsClassName) // 18 BDY
	LOTTM.St_sign_on_req.C_broker_status = ' '                                                        // 19 BDY
	LOTTM.St_sign_on_req.C_show_index = 'T'                                                           // 20 BDY

	/***************************** St_broker_eligibility_per_mkt Configuration ***************************************/

	LOTTM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_NormalMkt)
	LOTTM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_OddlotMkt)
	LOTTM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_SpotMkt)
	LOTTM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_AuctionMkt)
	LOTTM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_BrokerFiller1)
	LOTTM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_BrokerFiller2)

	/***************************** End of St_broker_eligibility_per_mkt Configuration *********************************/

	LOTTM.St_sign_on_req.Si_member_type = 0      // 21 BDY
	LOTTM.St_sign_on_req.C_clearing_status = ' ' // 22 BDY

	CopyAndFormatSymbol(LOTTM.St_sign_on_req.C_broker_name[:], util.LEN_BROKER_NAME, LOTTM.Exg_BrkrName) // 23 BDY
	CopyAndFormatSymbol(LOTTM.St_sign_on_req.C_reserved_3[:], 16, " ")                                   // 24 BDY
	CopyAndFormatSymbol(LOTTM.St_sign_on_req.C_reserved_4[:], 16, " ")                                   // 25 BDY
	CopyAndFormatSymbol(LOTTM.St_sign_on_req.C_reserved_5[:], 16, " ")                                   // 26 BDY

	LOTTM.ConvertSignOnReqToNetworkOrder() // Here we are converting all the numbers to Network Order

	LOTTM.St_net_hdr.S_message_length = int16(unsafe.Sizeof(LOTTM.St_sign_on_req) + unsafe.Sizeof(LOTTM.St_net_hdr)) // 1 NET_FDR
	LOTTM.St_net_hdr.I_seq_num = LOTTM.GetResetSequence()                                                            // 2 NET_HDR

	hasher := md5.New()

	oeReqResString, err := json.Marshal(LOTTM.St_sign_on_req) // to convert the structure in string
	if err != nil {
		LOTTM.LoggerManager.LogError(LOTTM.ServiceName, " [ Error: failed to convert St_sign_on_req to string: %v", err)
	}
	io.WriteString(hasher, string(oeReqResString))

	checksum := hasher.Sum(nil)
	copy(LOTTM.St_net_hdr.C_checksum[:], fmt.Sprintf("%x", checksum)) // 3 NET_FDR

	LOTTM.St_req_q_data.L_msg_type = util.LOGIN_WITHOUT_OPEN_ORDR_DTLS // 1 St_req_q

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

func CopyAndFormatPassword(dest []byte, destLen int, src string) {

	for i := 0; i < len(src) && i < destLen; i++ {
		dest[i] = src[i]
	}

	for i := len(src); i < destLen; i++ {
		dest[i] = ' '
	}
}
