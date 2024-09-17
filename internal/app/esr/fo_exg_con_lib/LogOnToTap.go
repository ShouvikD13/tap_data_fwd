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

	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"DATA_FWD_TAP/util/MessageQueue"

	"gorm.io/gorm"
)

const defaultTimeStamp = "0"

type FoExgConLibManager struct {
	ServiceName                string
	St_sign_on_req             *models.St_sign_on_req
	St_req_q_data              *models.St_req_q_data
	St_exch_msg                *models.St_exch_msg
	int_header                 *models.St_int_header
	St_net_hdr                 *models.St_net_hdr
	St_BrokerEligibilityPerMkt *models.St_broker_eligibility_per_mkt
	MQM                        *MessageQueue.MessageQueueManager
	EM                         *util.EnvironmentManager
	PUM                        *util.PasswordUtilManger // initialize it where ever you are initializing 'FECLM'
	DB                         *gorm.DB
	LoggerManager              *util.LoggerManager
	OCM                        *util.OrderConversionManager
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

func (FECLM *FoExgConLibManager) LogOnToTap() int {
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
	result := FECLM.DB.Raw(queryForBPS_Stts, "cln_esr_clnt", FECLM.C_pipe_id).Scan(&bpsStts)

	if result.Error != nil {
		errorMsg := fmt.Sprintf("Error executing query: %v", result.Error)
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "[LogOnToTap] %s", errorMsg)
		return -1
	}

	if bpsStts == "N" {
		errorMsg := fmt.Sprintf("ESR client is not running for Pipe ID: %s and User ID: %d", FECLM.C_pipe_id, FECLM.UserID)
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "[LogOnToTap] %s", errorMsg)
		return -1
	}

	successMsg := fmt.Sprintf("ESR client is running for Pipe ID: %s and User ID: %d", FECLM.C_pipe_id, FECLM.UserID)
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
	errForExgMaxPswdVldDays := FECLM.DB.Raw(queryForExgMaxPswdVldDays, FECLM.C_pipe_id).Scan(&exgMaxPswdVldDays).Error

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

			errFor_opm_exg_pwd := FECLM.DB.Raw(queryFor_opm_exg_pwd, FECLM.C_pipe_id).Scan(&currentPassword).Error

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

			errforSET_opm_new_exg_pwd := tx.Exec(queryforSET_opm_new_exg_pwd, encryptedPassword, FECLM.C_pipe_id).Error

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
			errForPasswordChange := FECLM.PUM.FnwritePasswordChangeToFile(FECLM.C_pipe_id, newPassword)
			if errForPasswordChange != nil {
				FECLM.LoggerManager.LogError("PasswordChange", "Failed to log password change: %v", errForPasswordChange)
			}

			successMsg := fmt.Sprintf("Password changed successfully for Pipe ID: %s", FECLM.C_pipe_id)
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
	row := FECLM.DB.Raw(queryFor_opm_ord_pipe_mstr_Vars, FECLM.C_pipe_id).Row()

	errFor_opm_ord_pipe_mstr_Vars := row.Scan(&FECLM.Opm_loginStatus, &FECLM.Opm_userID, &FECLM.Opm_existingPasswd, &FECLM.Opm_newPasswd)

	if errFor_opm_ord_pipe_mstr_Vars != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "SQL Error: %v", errFor_opm_ord_pipe_mstr_Vars)
		return -1
	}

	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Fetched new password: %s", FECLM.Opm_newPasswd.String)

	if FECLM.UserID != FECLM.Opm_userID {
		errorMsg := fmt.Sprintf("Incorrect User ID for Pipe ID: %s, expected: %d, got: %d", FECLM.C_pipe_id, FECLM.Opm_userID, FECLM.UserID)
		FECLM.LoggerManager.LogError(FECLM.ServiceName, errorMsg)
		return -1
	}

	if FECLM.Opm_loginStatus == 1 {
		errorMsg := fmt.Sprintf("User is already logged in for Pipe ID: %s, User ID: %d", FECLM.C_pipe_id, FECLM.Opm_userID)
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
	rowForOpmOrdPipeMstr := FECLM.DB.Raw(queryForOpmOrdPipeMstr, FECLM.C_pipe_id).Row()

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
	/********************** Body Starts ********************/
	/********************** Header Starts ********************/
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

	/********************** Header Done ********************/

	FECLM.St_sign_on_req.Li_user_id = FECLM.Opm_userID                                                           // 1 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_reserved_1[:], 8, " ")                                            // 2 BDY
	CopyAndFormatPassword(FECLM.St_sign_on_req.C_password[:], util.LEN_PASSWORD, FECLM.Opm_existingPasswd)       // 3 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_reserved_2[:], 8, " ")                                            // 4 BDY
	CopyAndFormatPassword(FECLM.St_sign_on_req.C_new_password[:], util.LEN_PASSWORD, FECLM.Opm_newPasswd.String) // 5 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_trader_name[:], util.LEN_TRADER_NAME, FECLM.Opm_TrdrID)           // 6 BDY
	FECLM.St_sign_on_req.Li_last_password_change_date = 0                                                        // 7 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_broker_id[:], util.LEN_BROKER_ID, FECLM.Exg_BrkrID)               // 8 BDY
	FECLM.St_sign_on_req.C_filler_1 = ' '                                                                        // 9 BDY
	FECLM.St_sign_on_req.Si_branch_id = int16(FECLM.Opm_BrnchID)                                                 // 10 BDY

	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Sign-on request message length: %d", FECLM.St_sign_on_req.St_hdr.Si_message_length)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "User ID: %d", FECLM.St_sign_on_req.Li_user_id)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Password: %s", FECLM.St_sign_on_req.C_password)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "New Password: %s", FECLM.St_sign_on_req.C_new_password)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Trader Name: %s", FECLM.St_sign_on_req.C_trader_name)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Last Password Change Date: %d", FECLM.St_sign_on_req.Li_last_password_change_date)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Broker ID: %s", FECLM.St_sign_on_req.C_broker_id)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Filler 1: %c", FECLM.St_sign_on_req.C_filler_1)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "Branch ID: %d", FECLM.St_sign_on_req.Si_branch_id)

	versionStr := FECLM.EM.GetProcessSpaceValue("version", "VERSION_NUMBER")
	if versionStr == "" {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "Failed to retrieve VERSION_NUMBER")
		return -1
	}
	FECLM.St_sign_on_req.Li_version_number, _ = strconv.ParseInt(versionStr, 10, 64) // 11 BDY
	FECLM.St_sign_on_req.Li_batch_2_start_time = 0                                   // 12 BDY
	FECLM.St_sign_on_req.C_host_switch_context = ' '                                 // 13 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_colour[:], util.LEN_COLOUR, " ")      // 14 BDY
	FECLM.St_sign_on_req.C_filler_2 = ' '                                            // 15 BDY

	userTypeStr := FECLM.EM.GetProcessSpaceValue("UserSettings", "USER_TYPE")
	userType, err := strconv.Atoi(userTypeStr)
	if err != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "Failed to convert USER_TYPE to int: %v", err)
		return -1
	}

	FECLM.St_sign_on_req.Si_user_type = int16(userType) // 16 BDY
	FECLM.St_sign_on_req.D_sequence_number = 0          // 17 BDY

	wsClassName := FECLM.EM.GetProcessSpaceValue("Service", "WSC_NAME")
	if wsClassName == "" {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "Failed to retrieve WSC_NAME")
		return -1
	}

	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_ws_class_name[:], util.LEN_WS_CLASS_NAME, wsClassName) // 18 BDY
	FECLM.St_sign_on_req.C_broker_status = ' '                                                        // 19 BDY
	FECLM.St_sign_on_req.C_show_index = 'T'                                                           // 20 BDY

	/***************************** St_broker_eligibility_per_mkt Configuration ***************************************/

	FECLM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_NormalMkt)
	FECLM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_OddlotMkt)
	FECLM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_SpotMkt)
	FECLM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_AuctionMkt)
	FECLM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_BrokerFiller1)
	FECLM.St_BrokerEligibilityPerMkt.ClearFlag(models.Flg_BrokerFiller2)

	/***************************** End of St_broker_eligibility_per_mkt Configuration *********************************/

	FECLM.St_sign_on_req.Si_member_type = 0      // 21 BDY
	FECLM.St_sign_on_req.C_clearing_status = ' ' // 22 BDY

	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_broker_name[:], util.LEN_BROKER_NAME, FECLM.Exg_BrkrName) // 23 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_reserved_3[:], 16, " ")                                   // 24 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_reserved_4[:], 16, " ")                                   // 25 BDY
	CopyAndFormatSymbol(FECLM.St_sign_on_req.C_reserved_5[:], 16, " ")                                   // 26 BDY

	FECLM.ConvertSignOnReqToNetworkOrder() // Here we are converting all the numbers to Network Order

	FECLM.St_net_hdr.S_message_length = int16(unsafe.Sizeof(FECLM.St_sign_on_req) + unsafe.Sizeof(FECLM.St_net_hdr)) // 1 NET_FDR
	FECLM.St_net_hdr.I_seq_num = FECLM.GetResetSequence()                                                            // 2 NET_HDR

	hasher := md5.New()

	oeReqResString, err := json.Marshal(FECLM.St_sign_on_req) // to convert the structure in string
	if err != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, " [ Error: failed to convert St_sign_on_req to string: %v", err)
	}
	io.WriteString(hasher, string(oeReqResString))

	checksum := hasher.Sum(nil)
	copy(FECLM.St_net_hdr.C_checksum[:], fmt.Sprintf("%x", checksum)) // 3 NET_FDR

	FECLM.St_req_q_data.L_msg_type = util.LOGIN_WITHOUT_OPEN_ORDR_DTLS // 1 St_req_q

	return 0
}

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

	err = FECLM.fn_write_msg_q(i_send_qid, FECLM.St_req_q_data)
	if err != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "[Fn_logoff_from_TAP] Failed to write in Transmit queue: %v", err)
		*c_err_msg = fmt.Sprintf("Failed to write in Transmit queue: %v", err)
		return -1
	}

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

func (FECLM *FoExgConLibManager) ConvertSignOnReqToNetworkOrder() {

	FECLM.Convertint_headerToNetworkOrder()
	FECLM.St_sign_on_req.Li_user_id = FECLM.OCM.ConvertInt64ToNetworkOrder(FECLM.St_sign_on_req.Li_user_id)
	FECLM.St_sign_on_req.Li_last_password_change_date = FECLM.OCM.ConvertInt64ToNetworkOrder(FECLM.St_sign_on_req.Li_last_password_change_date)
	FECLM.St_sign_on_req.Si_branch_id = FECLM.OCM.ConvertInt16ToNetworkOrder(FECLM.St_sign_on_req.Si_branch_id)
	FECLM.St_sign_on_req.Li_version_number = FECLM.OCM.ConvertInt64ToNetworkOrder(FECLM.St_sign_on_req.Li_version_number)
	FECLM.St_sign_on_req.Li_batch_2_start_time = FECLM.OCM.ConvertInt64ToNetworkOrder(FECLM.St_sign_on_req.Li_batch_2_start_time)
	FECLM.St_sign_on_req.Si_user_type = FECLM.OCM.ConvertInt16ToNetworkOrder(FECLM.St_sign_on_req.Si_user_type)
	FECLM.St_sign_on_req.D_sequence_number = FECLM.OCM.ConvertFloat64ToNetworkOrder(FECLM.St_sign_on_req.D_sequence_number)
	FECLM.St_sign_on_req.Si_member_type = FECLM.OCM.ConvertInt16ToNetworkOrder(FECLM.St_sign_on_req.Si_member_type)
}

func (FECLM *FoExgConLibManager) Convertint_headerToNetworkOrder() {
	FECLM.int_header.Si_transaction_code = FECLM.OCM.ConvertInt16ToNetworkOrder(FECLM.int_header.Si_transaction_code)
	FECLM.int_header.Li_log_time = FECLM.OCM.ConvertInt32ToNetworkOrder(FECLM.int_header.Li_log_time)
	FECLM.int_header.Li_trader_id = FECLM.OCM.ConvertInt32ToNetworkOrder(FECLM.int_header.Li_trader_id)
	FECLM.int_header.Si_error_code = FECLM.OCM.ConvertInt16ToNetworkOrder(FECLM.int_header.Si_error_code)
	FECLM.int_header.Si_message_length = FECLM.OCM.ConvertInt16ToNetworkOrder(FECLM.int_header.Si_message_length)
}

func (FECLM *FoExgConLibManager) GetResetSequence() int32 {
	var seqNum int32
	var query string

	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, " [GetResetSequence] [Executing increment sequence query for pipe ID: %s, trade date: %s]", FECLM.C_pipe_id, FECLM.Exg_NxtTrdDate)

	query = `
		UPDATE fsp_fo_seq_plcd
		SET fsp_seq_num = fsp_seq_num + 1
		WHERE fsp_pipe_id = ? AND fsp_trd_dt = TO_DATE(?, 'YYYY-MM-DD')
		RETURNING fsp_seq_num;
	`
	result1 := FECLM.DB.Raw(query, FECLM.C_pipe_id, FECLM.Exg_NxtTrdDate).Scan(&seqNum)

	if result1.Error != nil {
		FECLM.LoggerManager.LogError(FECLM.ServiceName, " [GetResetSequence] [Error: executing increment sequence query: %v]", result1.Error)
		return -1
	}
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, " [GetResetSequence] [Incremented sequence number: %d]", seqNum)

	if seqNum == util.MAX_SEQ_NUM {
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, " [GetResetSequence] [Sequence number reached MAX_SEQ_NUM, resetting...]")

		query = `
			UPDATE fsp_fo_seq_plcd
			SET fsp_seq_num = 0
			WHERE fsp_pipe_id = ? AND fsp_trd_dt = TO_DATE(?, 'YYYY-MM-DD')
			RETURNING fsp_seq_num;
		`
		result2 := FECLM.DB.Raw(query, FECLM.C_pipe_id, FECLM.Exg_NxtTrdDate).Scan(&seqNum)

		if result2.Error != nil {
			FECLM.LoggerManager.LogError(FECLM.ServiceName, " [GetResetSequence] [Error: executing reset sequence query: %v]", result2.Error)
			return -1
		}
		FECLM.LoggerManager.LogInfo(FECLM.ServiceName, " [GetResetSequence] [Sequence number reset to: %d]", seqNum)
	}
	return seqNum
}
