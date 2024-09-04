package app

import (
	"DATA_FWD_TAP/MessageQueueDir"
	"DATA_FWD_TAP/config"
	"DATA_FWD_TAP/models"
	"DATA_FWD_TAP/models/structures"
	"fmt"
	"log"
	"strconv"
	"strings"
	"time"
	"unsafe"

	"gorm.io/gorm"
)

type ClnPackClntManager struct {
	ServiceName              string                    // Name of the service being managed
	Xchngbook                *structures.Vw_xchngbook  // Pointer to the Vw_Xchngbook structure
	orderbook                *structures.Vw_orderbook  // Pointer to the Vw_orderbook structure
	contract                 *structures.Vw_contract   // we are updating it's value from orderbook
	nse_contract             *structures.Vw_nse_cntrct // we are updating it in 'fn_get_ext_cnt'
	pipe_mstr                *structures.St_opm_pipe_mstr
	oe_reqres                *structures.St_oe_reqres
	exch_msg                 *structures.St_exch_msg
	net_hdr                  *structures.St_net_hdr
	q_packet                 *structures.St_req_q_data
	int_header               *structures.St_int_header
	contract_desc            *structures.St_contract_desc
	order_flag               *structures.St_order_flags
	Order_conversion_manager *models.OrderConversionManager
	cPanNo                   string // Pan number, initialized in the 'fnRefToOrd' method
	cLstActRef               string // Last activity reference, initialized in the 'fnRefToOrd' method
	cEspID                   string // ESP ID, initialized in the 'fnRefToOrd' method
	cAlgoID                  string // Algorithm ID, initialized in the 'fnRefToOrd' method
	cSourceFlg               string // Source flag, initialized in the 'fnRefToOrd' method
	cPrgmFlg                 string
	Enviroment_manager       *models.EnvironmentManager
	Message_queue_manager    *MessageQueueDir.MessageQueueManager
	Transaction_manager      *models.TransactionManager
	Max_Pack_Val             int
	Config_manager           *config.ConfigManager
}

/***************************************************************************************
 * Fn_bat_init initializes the client package manager by setting up data structures,
 * fetching configuration values, and preparing the environment for processing.
 * It ensures all prerequisites are met before the service starts.
 *
 * INPUT PARAMETERS:
 *    args[] - Command-line arguments, which must include at least 7 elements.
 *             The fourth argument (args[3]) specifies the pipe ID.
 *    Db     - Pointer to a GORM database connection object used for SQL queries.
 *
 * OUTPUT PARAMETERS:
 *    int    - Returns 0 for success, or -1 if an error occurs.
 *
 * FUNCTIONAL FLOW:
 * 1. Initializes required structures and managers for the service.
 * 2. Validates command-line arguments to ensure all necessary parameters are present.
 * 3. Sets the pipe ID from the command-line argument and creates a message queue.
 * 4. Executes SQL queries to fetch and store necessary data such as exchange code,
 *    trader ID, branch ID, trade date, and broker ID.
 * 5. Reads configuration values from a file, including user type and message limit.
 * 6. Invokes the `CLN_PACK_CLNT` function to start the main service logic.
 * 7. Returns a status code indicating the success or failure of the initialization.
 *
 ***********************************************************************************************/
func (client_pack_manager *ClnPackClntManager) Fn_bat_init(args []string, Db *gorm.DB) int {

	var temp_str string
	var resultTmp int
	client_pack_manager.ServiceName = "cln_pack_clnt"
	client_pack_manager.Xchngbook = &structures.Vw_xchngbook{}
	client_pack_manager.orderbook = &structures.Vw_orderbook{}
	client_pack_manager.contract = &structures.Vw_contract{}
	client_pack_manager.nse_contract = &structures.Vw_nse_cntrct{}
	client_pack_manager.pipe_mstr = &structures.St_opm_pipe_mstr{}
	client_pack_manager.oe_reqres = &structures.St_oe_reqres{}
	client_pack_manager.exch_msg = &structures.St_exch_msg{}
	client_pack_manager.Order_conversion_manager = &models.OrderConversionManager{}
	client_pack_manager.net_hdr = &structures.St_net_hdr{}
	client_pack_manager.q_packet = &structures.St_req_q_data{}
	client_pack_manager.Message_queue_manager = &MessageQueueDir.MessageQueueManager{}
	client_pack_manager.int_header = &structures.St_int_header{}
	client_pack_manager.contract_desc = &structures.St_contract_desc{}
	client_pack_manager.order_flag = &structures.St_order_flags{}
	client_pack_manager.Transaction_manager = models.NewTransactionManager(client_pack_manager.ServiceName, client_pack_manager.Config_manager)

	client_pack_manager.Message_queue_manager.Req_q_data = client_pack_manager.q_packet
	client_pack_manager.Message_queue_manager.ServiceName = client_pack_manager.ServiceName

	log.Printf("[%s]  [Fn_bat_init] Entering Fn_bat_init", client_pack_manager.ServiceName)

	// we are getting the 7 args
	if len(args) < 7 {
		log.Printf("[%s] [Fn_bat_init] Error: insufficient arguments provided", client_pack_manager.ServiceName)
		return -1
	}

	// setting pipe_id with command line argument (command line argument recieved from main)
	// using Pipe_Id to fetch xchng_code
	client_pack_manager.Xchngbook.C_pipe_id = args[3]

	/* Create a message queue using the CreateQueue function from MessageQueue.go file ().
	This function is responsible for creating a system-level queue on Linux. */
	if client_pack_manager.Message_queue_manager.CreateQueue(3) != 0 {
		log.Printf("[%s] [Fn_bat_init] Returning from 'CreateQueue' with an Error... %s", client_pack_manager.ServiceName)
		log.Printf("[%s] [Fn_bat_init] Exiting from function", client_pack_manager.ServiceName)
	} else {
		log.Printf("[%s] [Fn_bat_init] Created Message Queue SuccessFully", client_pack_manager.ServiceName)
	}

	log.Printf("[%s] [Fn_bat_init] Copied pipe ID from args[3]: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_pipe_id)

	// Fetch exchange code from 'opm_ord_pipe_mstr'
	queryForOpm_Xchng_Cd := `SELECT opm_xchng_cd
				FROM opm_ord_pipe_mstr
				WHERE opm_pipe_id = ?`

	log.Printf("[%s] [Fn_bat_init] Executing query to fetch exchange code with pipe ID: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_pipe_id)

	row := Db.Raw(queryForOpm_Xchng_Cd, client_pack_manager.Xchngbook.C_pipe_id).Row()
	temp_str = ""
	err := row.Scan(&temp_str)
	if err != nil {
		log.Printf("[%s] [Fn_bat_init] Error scanning row for exchange code: %v", client_pack_manager.Xchngbook.C_xchng_cd, err)
		return -1
	}

	client_pack_manager.Xchngbook.C_xchng_cd = temp_str
	temp_str = ""
	log.Printf("[%s] [Fn_bat_init] Exchange code fetched: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_xchng_cd)

	/*here we are fetching the 'OPM_TRDR_ID' and 'OPM_BRNCH_ID' by using 'OPM_XCHNG_CD' and 'OPM_PIPE_ID'
	which is fatched earlier */

	queryForTraderAndBranchID := `SELECT OPM_TRDR_ID, OPM_BRNCH_ID
				FROM OPM_ORD_PIPE_MSTR
				WHERE OPM_XCHNG_CD = ? AND OPM_PIPE_ID = ?`

	log.Printf("[%s] [Fn_bat_init] Executing query to fetch trader and branch ID with exchange code: %s and pipe ID: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_xchng_cd, client_pack_manager.Xchngbook.C_pipe_id)

	rowQueryForTraderAndBranchID := Db.Raw(queryForTraderAndBranchID, client_pack_manager.Xchngbook.C_xchng_cd, client_pack_manager.Xchngbook.C_pipe_id).Row()

	errQueryForTraderAndBranchID := rowQueryForTraderAndBranchID.Scan(&client_pack_manager.pipe_mstr.C_opm_trdr_id, &client_pack_manager.pipe_mstr.L_opm_brnch_id)
	if errQueryForTraderAndBranchID != nil {
		log.Printf("[%s] [Fn_bat_init] Error scanning row for trader and branch ID: %v", client_pack_manager.ServiceName, errQueryForTraderAndBranchID)
		return -1
	}

	log.Printf("[%s] [Fn_bat_init] fetched trader ID: %s and branch ID : %d", client_pack_manager.ServiceName, client_pack_manager.pipe_mstr.C_opm_trdr_id, client_pack_manager.pipe_mstr.L_opm_brnch_id)

	// Fetch modification trade date from 'exg_xchng_mstr'

	queryFor_exg_nxt_trd_dt := `SELECT exg_nxt_trd_dt,
				exg_brkr_id
				FROM exg_xchng_mstr
				WHERE exg_xchng_cd = ?`

	log.Printf("[%s] [Fn_bat_init] Fetching trade date and broker ID with exchange code: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_xchng_cd)
	row2 := Db.Raw(queryFor_exg_nxt_trd_dt, client_pack_manager.Xchngbook.C_xchng_cd).Row()

	err2 := row2.Scan(&temp_str, &client_pack_manager.pipe_mstr.C_xchng_brkr_id)
	if err2 != nil {
		log.Printf("[%s] [Fn_bat_init] Error scanning trade date and broker ID: %v", client_pack_manager.ServiceName, err2)
		return -1
	}

	client_pack_manager.Xchngbook.C_mod_trd_dt = temp_str

	log.Printf("[%s] [Fn_bat_init] Fetched trade date : %s and broker ID : %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_mod_trd_dt, client_pack_manager.pipe_mstr.C_xchng_brkr_id)

	client_pack_manager.Xchngbook.L_ord_seq = 0 // I am initially setting it to '0' because it was set that way in 'fn_bat_init' and I have not seen it getting changed anywhere. If I find it being changed somewhere, I will update it accordingly.

	log.Printf("[%s] [Fn_bat_init] L_ord_seq initialized to %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_ord_seq)

	//here we are setting the S_user_typ_glb of 'St_opm_pipe_mstr' from the configuration file
	userTypeStr := client_pack_manager.Enviroment_manager.GetProcessSpaceValue("UserSettings", "UserType")
	userType, err := strconv.Atoi(userTypeStr)
	if err != nil {
		log.Printf("[%s] [Fn_bat_init] Failed to convert UserType '%s' to integer: %v", client_pack_manager.ServiceName, userTypeStr, err)

	}
	client_pack_manager.pipe_mstr.S_user_typ_glb = userType

	//this value also we are reding from configuration file "I have set the temp value in the configuration file for now"
	maxPackValStr := client_pack_manager.Enviroment_manager.GetProcessSpaceValue("PackingLimit", "PACK_VAL")
	if maxPackValStr == "" {
		log.Printf("[%s] [Fn_bat_init] 'PACK_VAL' not found in the configuration under 'PackingLimit'", client_pack_manager.ServiceName)
	} else {
		maxPackVal, err := strconv.Atoi(maxPackValStr)
		if err != nil {
			log.Printf("[%s] [Fn_bat_init] Failed to convert 'PACK_VAL' '%s' to integer: %v", client_pack_manager.ServiceName, maxPackValStr, err)
			maxPackVal = 0
		} else {
			log.Printf("[%s] [Fn_bat_init] Fetched and converted 'PACK_VAL' from configuration: %d", client_pack_manager.ServiceName, maxPackVal)
		}
		client_pack_manager.Max_Pack_Val = maxPackVal
	}

	// finally we are calling the CLN_PACK_CLNT function (service)
	resultTmp = client_pack_manager.CLN_PACK_CLNT(args, Db)
	if resultTmp != 0 {
		log.Printf("[%s] [Fn_bat_init] CLN_PACK_CLNT failed with result code: %d", client_pack_manager.ServiceName, resultTmp)
		log.Printf("[%s] [Fn_bat_init] Returning to main from fn_bat_init", client_pack_manager.ServiceName)
		return -1
	}
	log.Printf("[%s] [Fn_bat_init] Exiting Fn_bat_init", client_pack_manager.ServiceName)
	return 0
}

/***************************************************************************************
 * CLN_PACK_CLNT handles the core operations of the client package manager service.
 * It continuously retrieves newly created orders from the database, manages database
 * transactions, and writes the data to a system queue.
 *
 * INPUT PARAMETERS:
 *    args[] - Command-line arguments used for service configuration and control.
 *    Db     - Pointer to a GORM database connection object for SQL operations.
 *
 * OUTPUT PARAMETERS:
 *    int    - Returns 0 for success, or -1 if an error occurs.
 *
 * FUNCTIONAL FLOW:
 * 1. Enters an infinite loop to continuously process orders.
 * 2. Initiates a local transaction with `FnBeginTran()`.
 * 3. Retrieves the next order record using `fnGetNxtRec()`.
 * 4. Commits the transaction with `FnCommitTran()`.
 * 5. Writes the fetched data to the system queue using `WriteToQueue()`.
 * 6. Increments the message type and reset if it exceeds the maximum value.
 *
 ***********************************************************************************************/

func (client_pack_manager *ClnPackClntManager) CLN_PACK_CLNT(args []string, Db *gorm.DB) int {
	var resultTmp int
	mtype := 1

	log.Printf("[%s] [CLN_PACK_CLNT] Entering CLN_PACK_CLNT", client_pack_manager.ServiceName)

	// Setting up an infinite loop to keep the service running and continuously fetch newly created orders.
	for {
		// Beginning a local transaction for the service to perform fetch and update operations on the database.
		resultTmp = client_pack_manager.Transaction_manager.FnBeginTran()
		if resultTmp == -1 {
			log.Printf("[%s] [CLN_PACK_CLNT] Transaction begin failed", client_pack_manager.ServiceName)
			return -1
		}
		log.Printf("[%s] [CLN_PACK_CLNT] Transaction started with type: %d", client_pack_manager.ServiceName, client_pack_manager.Transaction_manager.TranType)

		// Calling the function to fetch the next order record.
		resultTmp = client_pack_manager.fnGetNxtRec(Db)
		if resultTmp != 0 {
			log.Printf("[%s] [CLN_PACK_CLNT] failed in getting the next record returning with result code: %d", client_pack_manager.ServiceName, resultTmp)
			if client_pack_manager.Transaction_manager.FnAbortTran() == -1 {
				log.Printf("[%s] [CLN_PACK_CLNT] Transaction abort failed", client_pack_manager.ServiceName)
				return -1
			}
			log.Printf("[%s] [CLN_PACK_CLNT] Exiting CLN_PACK_CLNT with error", client_pack_manager.ServiceName)
			return -1
		}

		// Committing the transaction if it is a local transaction.
		if client_pack_manager.Transaction_manager.FnCommitTran() == -1 {
			log.Printf("[%s] [CLN_PACK_CLNT] Transaction commit failed", client_pack_manager.ServiceName)
			return -1
		}

		log.Printf("[%s] [CLN_PACK_CLNT] Transaction committed successfully", client_pack_manager.ServiceName)

		// Writing the data fetched from 'fnGetNxtRec' to the system queue (Linux).
		if client_pack_manager.Message_queue_manager.WriteToQueue(mtype) != 0 {
			log.Printf("[%s] [CLN_PACK_CLNT] Error writing to queue with message type %d", client_pack_manager.ServiceName, mtype)
			return -1
		}

		log.Printf("[%s] [CLN_PACK_CLNT] Successfully wrote to queue with message type %d", client_pack_manager.ServiceName, mtype)

		// testing purposes (this will not be the part of actuall code)
		receivedData, receivedType, readErr := client_pack_manager.Message_queue_manager.ReadFromQueue(mtype)
		if readErr != 0 {
			log.Printf("[%s] [CLN_PACK_CLNT] Error reading from queue with message type %d: %d", client_pack_manager.ServiceName, mtype, readErr)
			return -1
		}
		log.Printf("[%s] [CLN_PACK_CLNT] Successfully read from queue with message type %d, received type: %d", client_pack_manager.ServiceName, mtype, receivedType)

		fmt.Println("Message Type:", receivedData.L_msg_type)
		fmt.Println("Checksum:", receivedData.St_exch_msg_data.St_net_header.C_checksum)
		fmt.Println("Sequence Number:", receivedData.St_exch_msg_data.St_net_header.I_seq_num)
		fmt.Println("Message Length:", receivedData.St_exch_msg_data.St_net_header.S_message_length)
		fmt.Println("Account Number:", receivedData.St_exch_msg_data.St_oe_res.C_account_number)
		fmt.Println("Exchange Message Data:", receivedData.St_exch_msg_data.St_oe_res)
		fmt.Printf("Size of receivedData: %d bytes\n", unsafe.Sizeof(receivedData))

		mtype++

		if mtype > client_pack_manager.Max_Pack_Val {
			mtype = 1
		}

		TemporaryQueryForTesting := `UPDATE FXB_FO_XCHNG_BOOK
                        SET fxb_plcd_stts = 'R'
                        WHERE fxb_plcd_stts = 'Q'`

		log.Printf("[%s] [CLN_PACK_CLNT] Executing update query to change status from 'Q' to 'R' in FXB_FO_XCHNG_BOOK", client_pack_manager.ServiceName)

		result := Db.Exec(TemporaryQueryForTesting)
		if result.Error != nil {
			log.Printf("[%s] [CLN_PACK_CLNT] Error executing update query: %v", client_pack_manager.ServiceName, result.Error)
			return -1
		}
		rowsAffected := result.RowsAffected
		log.Printf("[%s] [CLN_PACK_CLNT] Update query executed successfully, %d rows affected", client_pack_manager.ServiceName, rowsAffected)

	}

	// log.Printf("[%s] [CLN_PACK_CLNT] Exiting CLN_PACK_CLNT", client_pack_manager.ServiceName)
	//return 0
}

/***************************************************************************************
 * fnGetNxtRec retrieves and processes the next record from the database based on its
 * status and type, then updates relevant structures and statuses. It handles extraction
 * of order data, updates database statuses, and packs the data for further processing.
 *
 * INPUT PARAMETERS:
 *    Db - Pointer to a GORM database connection object for SQL operations.
 *
 * OUTPUT PARAMETERS:
 *    int - Returns 0 for successful execution, or -1 if an error occurs.
 *
 * FUNCTIONAL FLOW:
 * 1. Calls 'fnSeqToOmd' to fetch records from the 'FXB_FO_XCHNG_BOOK' table.
 * 2. Ensures 'C_plcd_stts' is 'REQUESTED'.
 * 3. Processes the order if its type is 'ORDINARY_ORDER':
 *    a. Transfers 'C_ordr_rfrnc' from 'Xchngbook' to 'orderbook'.
 *    b. Calls 'fnRefToOrd' to fetch order details from 'fod_fo_ordr_dtls'.
 *    c. Compares 'L_mdfctn_cntr' between 'Xchngbook' and 'orderbook'.
 *    d. Updates 'Xchngbook' and 'orderbook' statuses to 'QUEUED':
 *       - Calls 'fnUpdXchngbk' and 'fnUpdOrdrbk' to update the database.
 *    e. Sets the contract data using values from 'orderbook'.
 *    f. Checks if 'C_xchng_cd' is "NFO":
 *       - If "NFO", sets the request type and calls 'fnGetExtCnt' to load additional data.
 *    g. Calls 'fnPackOrdnryOrdToNse' to pack data into the final structure 'St_req_q_data'.
 *
 ***********************************************************************************************/

func (client_pack_manager *ClnPackClntManager) fnGetNxtRec(Db *gorm.DB) int {
	log.Printf("[%s] [fnGetNxtRec] Entering fnGetNxtRec", client_pack_manager.ServiceName)

	client_pack_manager.cPrgmFlg = "0"

	// Calling the function 'fnSeqToOmd' to fetched the Records from the 'FXB_FO_XCHNG_BOOK'
	resultTmp := client_pack_manager.fnSeqToOmd(Db)
	if resultTmp != 0 {
		log.Printf("[%s] [fnGetNxtRec] Failed to fetch data into eXchngbook structure", client_pack_manager.ServiceName)
		return -1
	}

	if client_pack_manager.Xchngbook == nil {
		log.Printf("[%s] [fnGetNxtRec] Error: Xchngbook is nil", client_pack_manager.ServiceName)
		return -1
	}

	if client_pack_manager.Xchngbook.C_rms_prcsd_flg == "P" {
		log.Printf("[%s] [fnGetNxtRec] Order ref / Mod number = |%s| / |%d| Already Processed", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_ordr_rfrnc, client_pack_manager.Xchngbook.L_mdfctn_cntr)

		log.Printf("[%s] [fnGetNxtRec] Exiting from from 'fnGetNxtRec'", client_pack_manager.ServiceName)

		return 0
	}

	if client_pack_manager.Xchngbook.C_plcd_stts != string(models.REQUESTED) {
		log.Printf("[%s] [fnGetNxtRec] the Placed status is not requested . so we are we are not getting the record ", client_pack_manager.ServiceName)
		return -1
	}

	// Checking the order type. Only fetch the order if it is of type 'Ordinary_Order'; otherwise, return without fetching.
	if client_pack_manager.Xchngbook.C_ex_ordr_type == string(models.ORDINARY_ORDER) {

		log.Printf("[%s] [fnGetNxtRec] Assigning C_ordr_rfrnc from eXchngbook to orderbook", client_pack_manager.ServiceName)
		client_pack_manager.orderbook.C_ordr_rfrnc = client_pack_manager.Xchngbook.C_ordr_rfrnc

		// Calling the function 'fnRefToOrd' to fetch orders from the 'fod_fo_ordr_dtls' table.

		resultTmp = client_pack_manager.fnRefToOrd(Db)
		if resultTmp != 0 {
			log.Printf("[%s] [fnGetNxtRec] Failed to fetch data into orderbook structure", client_pack_manager.ServiceName)
			return -1
		}

		// After fetching the records from 'order_book' and 'xchng_book', we check whether the values of 'L_mdfctn_cntr' from both tables are equal.
		if client_pack_manager.Xchngbook.L_mdfctn_cntr != client_pack_manager.orderbook.L_mdfctn_cntr {
			log.Printf("[%s] [fnGetNxtRec] L_mdfctn_cntr of both Xchngbook and order are not same", client_pack_manager.ServiceName)
			log.Printf(" [fnGetNxtRec]  Exiting fnGetNxtRec")
			return -1

		}

		// Here, we are updating the status of 'xchngbook' to reflect the change in 'FXB_FO_XCHNG_BOOK', indicating that the record has been read.
		client_pack_manager.Xchngbook.C_plcd_stts = string(models.QUEUED)
		client_pack_manager.Xchngbook.C_oprn_typ = string(models.UPDATION_ON_ORDER_FORWARDING)

		resultTmp = client_pack_manager.fnUpdXchngbk(Db)

		if resultTmp != 0 {
			log.Printf("[%s] [fnGetNxtRec] failed to update the status in Xchngbook", client_pack_manager.ServiceName)
			return -1
		}

		client_pack_manager.orderbook.C_ordr_stts = string(models.QUEUED)
		// Here, we are updating the status of the 'fod_fo_ordr_dtls' table to indicate that this record has been read and queued.
		resultTmp = client_pack_manager.fnUpdOrdrbk(Db)

		if resultTmp != 0 {
			log.Printf("[%s] failed to update the status in Order book", client_pack_manager.ServiceName)
			return -1
		}

		// here we are setting the contract data from orderbook structure
		log.Printf("[%s]  [fnGetNxtRec] Setting contract data from orderbook structure", client_pack_manager.ServiceName)
		client_pack_manager.contract.C_xchng_cd = client_pack_manager.orderbook.C_xchng_cd
		client_pack_manager.contract.C_prd_typ = client_pack_manager.orderbook.C_prd_typ
		client_pack_manager.contract.C_undrlyng = client_pack_manager.orderbook.C_undrlyng
		client_pack_manager.contract.C_expry_dt = client_pack_manager.orderbook.C_expry_dt
		client_pack_manager.contract.C_exrc_typ = client_pack_manager.orderbook.C_exrc_typ
		client_pack_manager.contract.C_opt_typ = client_pack_manager.orderbook.C_opt_typ
		client_pack_manager.contract.L_strike_prc = client_pack_manager.orderbook.L_strike_prc
		client_pack_manager.contract.C_ctgry_indstk = client_pack_manager.orderbook.C_ctgry_indstk
		// client_pack_manager.contract.L_ca_lvl = client_pack_manager.orderbook.L_ca_lvl

		log.Printf("[%s] [fnGetNxtRec] contract.C_xchng_cd: %s ", client_pack_manager.ServiceName, client_pack_manager.contract.C_xchng_cd)
		log.Printf("[%s] [fnGetNxtRec] contract.C_prd_typ: %s ", client_pack_manager.ServiceName, client_pack_manager.contract.C_prd_typ)
		log.Printf("[%s] [fnGetNxtRec] contract.C_undrlyng: %s ", client_pack_manager.ServiceName, client_pack_manager.contract.C_undrlyng)
		log.Printf("[%s] [fnGetNxtRec] contract.C_expry_dt: %s ", client_pack_manager.ServiceName, client_pack_manager.contract.C_expry_dt)
		log.Printf("[%s] [fnGetNxtRec] contract.C_exrc_typ: %s ", client_pack_manager.ServiceName, client_pack_manager.contract.C_exrc_typ)
		log.Printf("[%s] [fnGetNxtRec] contract.C_opt_typ: %s ", client_pack_manager.ServiceName, client_pack_manager.contract.C_opt_typ)
		log.Printf("[%s] [fnGetNxtRec] contract.L_strike_prc: %d ", client_pack_manager.ServiceName, client_pack_manager.contract.L_strike_prc)
		log.Printf("[%s] [fnGetNxtRec] contract.C_ctgry_indstk: %s ", client_pack_manager.ServiceName, client_pack_manager.contract.C_ctgry_indstk)
		//log.Printf("[%s] contract.L_ca_lvl: |%d|", client_pack_manager.ServiceName, client_pack_manager.contract.L_ca_lvl)

		if strings.TrimSpace(client_pack_manager.Xchngbook.C_xchng_cd) == "NFO" {

			client_pack_manager.contract.C_rqst_typ = string(models.CONTRACT_TO_NSE_ID)

			// Calling this function to extract data from various tables and store it in the contract structure.
			resultTmp = client_pack_manager.fnGetExtCnt(Db)

			if resultTmp != 0 {
				log.Printf("[%s]  [fnGetNxtRec] failed to load data in NSE CNT ", client_pack_manager.ServiceName)
				return -1
			}
		} else {
			log.Printf("[%s] [fnGetNxtRec] returning from 'fnGetNxtRec' because 'Xchngbook.C_xchng_cd' is not 'NFO' --> %s ", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_xchng_cd)
			log.Printf("[%s] [fnGetNxtRec] returning with Error", client_pack_manager.ServiceName)
			return -1
		}

		log.Printf("[%s] [fnGetNxtRec] Packing Ordinary Order STARTS ", client_pack_manager.ServiceName)

		// Checking if 'Token_id' is received from 'fnGetExtCnt'. If not, the record will be rejected.

		if client_pack_manager.nse_contract.L_token_id == 0 {
			log.Printf("[%s] [fnGetNxtRec] Token id for ordinary order is: %d", client_pack_manager.ServiceName, client_pack_manager.nse_contract.L_token_id)

			log.Printf("[%s] [fnGetNxtRec] token id is not avialable so we are calling 'fn_rjct_rcrd' ", client_pack_manager.ServiceName)

			// Calling 'fnRjctRcrd' to reject the record.
			resultTmp = client_pack_manager.fnRjctRcrd(Db)

			if resultTmp != 0 {
				log.Printf("[%s] [fnGetNxtRec] returned from 'fnRjctRcrd' with Error", client_pack_manager.ServiceName)
				log.Printf("[%s] [fnGetNxtRec] Exiting from 'fnGetNxtRec' ", client_pack_manager.ServiceName)
				return -1
			}

		}

		// here we are initialising the "ExchngPackLibMaster"
		eplm := NewExchngPackLibMaster(
			client_pack_manager.ServiceName,
			client_pack_manager.Xchngbook,
			client_pack_manager.orderbook,
			client_pack_manager.pipe_mstr,
			client_pack_manager.nse_contract,
			client_pack_manager.oe_reqres,
			client_pack_manager.exch_msg,
			client_pack_manager.net_hdr,
			client_pack_manager.q_packet,
			client_pack_manager.int_header,
			client_pack_manager.contract_desc,
			client_pack_manager.order_flag,
			client_pack_manager.Order_conversion_manager,
			client_pack_manager.cPanNo,
			client_pack_manager.cLstActRef,
			client_pack_manager.cEspID,
			client_pack_manager.cAlgoID,
			client_pack_manager.cSourceFlg,
			client_pack_manager.cPrgmFlg,
		)

		// Calling the function 'fnPackOrdnryOrdToNse' to pack the entire data into the final structure 'St_req_q_data'.
		resultTmp = eplm.fnPackOrdnryOrdToNse(Db)

		if resultTmp != 0 {
			log.Printf("[%s]  [fnGetNxtRec] 'fnPackOrdnryOrdToNse' returned an error.", client_pack_manager.ServiceName)
			log.Printf("[%s] [fnGetNxtRec] Exiting 'fnGetNxtRec'.", client_pack_manager.ServiceName)
		} else {
			log.Printf("[%s] [fnGetNxtRec] Data successfully packed.", client_pack_manager.ServiceName)
		}

	}

	log.Printf("[%s] [fnGetNxtRec] Exiting fnGetNxtRec", client_pack_manager.ServiceName)
	return 0
}

/***************************************************************************************
 * fnSeqToOmd fetches the next record from the 'FXB_FO_XCHNG_BOOK' table in the database
 * based on specified criteria (exchange code, pipe ID, trade date, and order sequence).
 * It retrieves order information and stores it in the 'Xchngbook' structure
 * within the 'ClnPackClntManager' instance.
 *
 * INPUT PARAMETERS:
 *    db - Pointer to a GORM database connection object for SQL operations.
 *
 * OUTPUT PARAMETERS:
 *    int - Returns 0 for successful data retrieval, or -1 if an error occurs.
 *
 * FUNCTIONAL FLOW:
 * 1. Constructs and executes a SQL query using GORM's 'Raw' method to fetch order details
 *    from the 'FXB_FO_XCHNG_BOOK' table:
 *    a. The query selects various columns, including order reference, quantity, order type,
 *       sequence number, etc., based on specified conditions.
 *    b. Utilizes 'TO_DATE' to format date fields and 'COALESCE' to handle null values for
 *       specific fields.
 * 2. Scans the retrieved row into the fields of the 'Xchngbook' structure and additional
 *    local variables ('c_ip_addrs', 'c_prcimpv_flg').
 *
 ***********************************************************************************************/

func (client_pack_manager *ClnPackClntManager) fnSeqToOmd(db *gorm.DB) int {
	var c_ip_addrs string
	var c_prcimpv_flg string

	log.Printf("[%s][fnSeqToOmd] Entering fnSeqToOmd", client_pack_manager.ServiceName)
	log.Printf("[%s][fnSeqToOmd] Before extracting the data from the 'fxb_ordr_rfrnc' and storing it in the 'Xchngbook' structure", client_pack_manager.ServiceName)

	query := `
		SELECT fxb_ordr_rfrnc,
		fxb_lmt_mrkt_sl_flg AS C_slm_flg,
		fxb_dsclsd_qty AS L_dsclsd_qty,
		fxb_ordr_tot_qty AS L_ord_tot_qty,
		fxb_lmt_rt AS L_ord_lmt_rt,
		fxb_stp_lss_tgr AS L_stp_lss_tgr,
		TO_CHAR(TO_DATE(fxb_ordr_valid_dt, 'YYYY-MM-DD'), 'DD-Mon-YYYY') AS C_valid_dt,
		CASE
			WHEN fxb_ordr_type = 'V' THEN 'T'
			ELSE fxb_ordr_type
		END AS C_ord_typ,
		fxb_rqst_typ AS C_req_typ,
		fxb_ordr_sqnc AS L_ord_seq,
		COALESCE(fxb_ip, 'NA') AS C_ip_addrs,
		COALESCE(fxb_prcimpv_flg, 'N') AS C_prcimpv_flg,
		fxb_ex_ordr_typ AS C_ex_ordr_type,
		fxb_plcd_stts AS C_plcd_stts,
		fxb_mdfctn_cntr AS L_mdfctn_cntr,
		fxb_frwd_tm AS C_frwrd_tm,
		fxb_jiffy AS D_jiffy,
		fxb_xchng_rmrks AS C_xchng_rmrks,
		fxb_rms_prcsd_flg AS C_rms_prcsd_flg,
		fxb_ors_msg_typ AS L_ors_msg_typ,
		fxb_ack_tm AS C_ack_tm
		FROM FXB_FO_XCHNG_BOOK
		WHERE fxb_xchng_cd = ?
		AND fxb_pipe_id = ?
		AND TO_DATE(fxb_mod_trd_dt, 'YYYY-MM-DD') = TO_DATE(?, 'YYYY-MM-DD')
		AND fxb_ordr_sqnc = (
		SELECT MIN(fxb_b.fxb_ordr_sqnc)
		FROM FXB_FO_XCHNG_BOOK fxb_b
		WHERE fxb_b.fxb_xchng_cd = ?
         AND TO_DATE(fxb_b.fxb_mod_trd_dt, 'YYYY-MM-DD') = TO_DATE(?, 'YYYY-MM-DD')
         AND fxb_b.fxb_pipe_id = ?
         AND fxb_b.fxb_plcd_stts = 'R'
		);


	`

	log.Printf("[%s][fnSeqToOmd] Executing query to fetch order details", client_pack_manager.ServiceName)

	log.Printf("[%s][fnSeqToOmd] C_xchng_cd: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_xchng_cd)
	log.Printf("[%s][fnSeqToOmd] C_pipe_id: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_pipe_id)
	log.Printf("[%s][fnSeqToOmd] C_mod_trd_dt: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_mod_trd_dt)

	row := db.Raw(query,
		client_pack_manager.Xchngbook.C_xchng_cd,
		client_pack_manager.Xchngbook.C_pipe_id,
		client_pack_manager.Xchngbook.C_mod_trd_dt,
		client_pack_manager.Xchngbook.C_xchng_cd,
		client_pack_manager.Xchngbook.C_mod_trd_dt,
		client_pack_manager.Xchngbook.C_pipe_id).Row()

	err := row.Scan(
		&client_pack_manager.Xchngbook.C_ordr_rfrnc,    // fxb_ordr_rfrnc
		&client_pack_manager.Xchngbook.C_slm_flg,       // fxb_lmt_mrkt_sl_flg
		&client_pack_manager.Xchngbook.L_dsclsd_qty,    // fxb_dsclsd_qty
		&client_pack_manager.Xchngbook.L_ord_tot_qty,   // fxb_ordr_tot_qty
		&client_pack_manager.Xchngbook.L_ord_lmt_rt,    // fxb_lmt_rt
		&client_pack_manager.Xchngbook.L_stp_lss_tgr,   // fxb_stp_lss_tgr
		&client_pack_manager.Xchngbook.C_valid_dt,      // C_valid_dt (formatted as 'DD-Mon-YYYY')
		&client_pack_manager.Xchngbook.C_ord_typ,       // C_ord_typ (transformed 'V' to 'T')
		&client_pack_manager.Xchngbook.C_req_typ,       // C_req_typ
		&client_pack_manager.Xchngbook.L_ord_seq,       // L_ord_seq
		&c_ip_addrs,                                    // COALESCE(fxb_ip, 'NA')
		&c_prcimpv_flg,                                 // COALESCE(fxb_prcimpv_flg, 'N')
		&client_pack_manager.Xchngbook.C_ex_ordr_type,  // C_ex_ordr_type
		&client_pack_manager.Xchngbook.C_plcd_stts,     // C_plcd_stts
		&client_pack_manager.Xchngbook.L_mdfctn_cntr,   // L_mdfctn_cntr
		&client_pack_manager.Xchngbook.C_frwrd_tm,      // C_frwrd_tm
		&client_pack_manager.Xchngbook.D_jiffy,         // D_jiffy
		&client_pack_manager.Xchngbook.C_xchng_rmrks,   // C_xchng_rmrks
		&client_pack_manager.Xchngbook.C_rms_prcsd_flg, // C_rms_prcsd_flg
		&client_pack_manager.Xchngbook.L_ors_msg_typ,   // L_ors_msg_typ
		&client_pack_manager.Xchngbook.C_ack_tm,        // C_ack_tm
	)

	if err != nil {
		log.Printf("[%s][fnSeqToOmd] Error scanning row: %v", client_pack_manager.ServiceName, err)
		log.Printf("[%s][fnSeqToOmd] Exiting fnSeqToOmd with error", client_pack_manager.ServiceName)
		return -1
	}

	if c_prcimpv_flg == "Y" {
		client_pack_manager.cPrgmFlg = "T"
	} else {
		client_pack_manager.cPrgmFlg = c_ip_addrs
	}

	log.Printf("[%s][fnSeqToOmd] Data extracted and stored in the 'Xchngbook' structure:", client_pack_manager.ServiceName)
	log.Printf("[%s][fnSeqToOmd]   C_ordr_rfrnc:     %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_ordr_rfrnc)
	log.Printf("[%s][fnSeqToOmd]   C_slm_flg:        %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_slm_flg)
	log.Printf("[%s][fnSeqToOmd]   L_dsclsd_qty:     %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_dsclsd_qty)
	log.Printf("[%s][fnSeqToOmd]   L_ord_tot_qty:    %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_ord_tot_qty)
	log.Printf("[%s][fnSeqToOmd]   L_ord_lmt_rt:     %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_ord_lmt_rt)
	log.Printf("[%s][fnSeqToOmd]   L_stp_lss_tgr:    %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_stp_lss_tgr)
	log.Printf("[%s][fnSeqToOmd]   C_valid_dt:       %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_valid_dt)
	log.Printf("[%s][fnSeqToOmd]   C_ord_typ:        %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_ord_typ)
	log.Printf("[%s][fnSeqToOmd]   C_req_typ:        %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_req_typ)
	log.Printf("[%s][fnSeqToOmd]   L_ord_seq:        %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_ord_seq)
	log.Printf("[%s][fnSeqToOmd]   IpAddrs:          %s", client_pack_manager.ServiceName, c_ip_addrs)
	log.Printf("[%s][fnSeqToOmd]   PrcimpvFlg:       %s", client_pack_manager.ServiceName, c_prcimpv_flg)
	log.Printf("[%s][fnSeqToOmd]   C_ex_ordr_type:   %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_ex_ordr_type)
	log.Printf("[%s][fnSeqToOmd]   C_plcd_stts:      %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_plcd_stts)
	log.Printf("[%s][fnSeqToOmd]   L_mdfctn_cntr:    %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_mdfctn_cntr)
	log.Printf("[%s][fnSeqToOmd]   C_frwrd_tm:       %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_frwrd_tm)
	log.Printf("[%s][fnSeqToOmd]   D_jiffy:          %f", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.D_jiffy)
	log.Printf("[%s][fnSeqToOmd]   C_xchng_rmrks:    %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_xchng_rmrks)
	log.Printf("[%s][fnSeqToOmd]   C_rms_prcsd_flg:  %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_rms_prcsd_flg)
	log.Printf("[%s][fnSeqToOmd]   L_ors_msg_typ:    %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_ors_msg_typ)
	log.Printf("[%s][fnSeqToOmd]   C_ack_tm:         %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_ack_tm)

	log.Printf("[%s][fnSeqToOmd] Data extracted and stored in the 'Xchngbook' structure successfully", client_pack_manager.ServiceName)

	log.Printf("[%s][fnSeqToOmd] Exiting fnSeqToOmd", client_pack_manager.ServiceName)
	return 0
}

/***********************************************************************************************
 * fnRefToOrd retrieves and processes order data from the 'fod_fo_ordr_dtls' and 'CLM_CLNT_MSTR'
 * tables in the database, and stores it in the `orderbook` structure within the
 * 'ClnPackClntManager' instance.
 *
 * INPUT PARAMETERS:
 *    None
 *
 * OUTPUT PARAMETERS:
 *    int - Returns 0 for success, or -1 if an error occurs.
 *
 * FUNCTIONAL FLOW:
 * 1. Constructs and executes a SQL query using GORM's 'Raw' method to fetch order details
 *    from the 'fod_fo_ordr_dtls' table:
 *    a. Selects various columns, including client match account, order flow, total order quantity,
 *       executed quantity, etc., based on specified conditions.
 *    b. Uses 'FOR UPDATE NOWAIT' to lock selected rows for update and prevent other transactions
 *       from waiting for the lock.
 * 2. Scans the retrieved row into the fields of the `orderbook` structure within the
 *    'ClnPackClntManager' instance.
 * 3. Constructs and executes a second SQL query to fetch client details from the 'CLM_CLNT_MSTR' table
 *    where the client match account matches the previously fetched value.
 *
 ***********************************************************************************************/

func (client_pack_manager *ClnPackClntManager) fnRefToOrd(db *gorm.DB) int {
	/*
		c_pan_no --> we are simply setting it to 0 using memset in the
		MEMSET(c_ordr_rfrnc);
		MEMSET(c_pan_no);
		MEMSET(c_lst_act_ref);
		MEMSET(c_esp_id);
		MEMSET(c_algo_id);
		char c_source_flg = '\0';
	*/

	log.Printf("[%s] [fnRefToOrd]  Entering fnFetchOrderDetails", client_pack_manager.ServiceName)
	log.Printf("[%s] [fnRefToOrd] Before extracting the data from the 'fod_fo_ordr_dtls' and storing it in the 'orderbook' structure", client_pack_manager.ServiceName)

	query := `
    SELECT
    fod_clm_mtch_accnt AS C_cln_mtch_accnt,
    fod_ordr_flw AS C_ordr_flw,
    fod_ordr_tot_qty AS L_ord_tot_qty,
    fod_exec_qty AS L_exctd_qty,
    COALESCE(fod_exec_qty_day, 0) AS L_exctd_qty_day,
    fod_settlor AS C_settlor,
    fod_spl_flag AS C_spl_flg,
    TO_CHAR(fod_ord_ack_tm, 'YYYY-MM-DD HH24:MI:SS') AS C_ack_tm,
    TO_CHAR(fod_lst_rqst_ack_tm, 'YYYY-MM-DD HH24:MI:SS') AS C_prev_ack_tm,
    fod_pro_cli_ind AS C_pro_cli_ind,
    COALESCE(fod_ctcl_id, ' ') AS C_ctcl_id,
    COALESCE(fod_pan_no, '*') AS C_pan_no,
    COALESCE(fod_lst_act_ref, '0') AS C_lst_act_ref,
    COALESCE(fod_esp_id, '*') AS C_esp_id,
    COALESCE(fod_algo_id, '*') AS C_algo_id,
    COALESCE(fod_source_flg, '*') AS C_source_flg,
    fod_mdfctn_cntr AS L_mdfctn_cntr,
    fod_ordr_stts AS C_ordr_stts,
    fod_xchng_cd AS C_xchng_cd,
    fod_prdct_typ AS C_prd_typ,
    fod_undrlying AS C_undrlyng,
    fod_expry_dt AS C_expry_dt,
    fod_exer_typ AS C_exrc_typ,
    fod_opt_typ AS C_opt_typ,
    fod_strk_prc AS L_strike_prc,
    fod_indstk AS C_ctgry_indstk,
	fod_ack_nmbr As C_xchng_ack
	FROM
		fod_fo_ordr_dtls
	WHERE
		fod_ordr_rfrnc =  ?
	FOR UPDATE NOWAIT;

    `

	log.Printf("[%s] [fnRefToOrd] Executing query to fetch order details", client_pack_manager.ServiceName)

	log.Printf("[%s] [fnRefToOrd] Order Reference: %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_ordr_rfrnc)

	row := db.Raw(query, strings.TrimSpace(client_pack_manager.orderbook.C_ordr_rfrnc)).Row()

	err := row.Scan(
		&client_pack_manager.orderbook.C_cln_mtch_accnt,
		&client_pack_manager.orderbook.C_ordr_flw,
		&client_pack_manager.orderbook.L_ord_tot_qty,
		&client_pack_manager.orderbook.L_exctd_qty,
		&client_pack_manager.orderbook.L_exctd_qty_day,
		&client_pack_manager.orderbook.C_settlor,
		&client_pack_manager.orderbook.C_spl_flg,
		&client_pack_manager.orderbook.C_ack_tm,
		&client_pack_manager.orderbook.C_prev_ack_tm,
		&client_pack_manager.orderbook.C_pro_cli_ind,
		&client_pack_manager.orderbook.C_ctcl_id,
		&client_pack_manager.cPanNo,
		&client_pack_manager.cLstActRef,
		&client_pack_manager.cEspID,
		&client_pack_manager.cAlgoID,
		&client_pack_manager.cSourceFlg,
		&client_pack_manager.orderbook.L_mdfctn_cntr,
		&client_pack_manager.orderbook.C_ordr_stts,
		&client_pack_manager.orderbook.C_xchng_cd,
		&client_pack_manager.orderbook.C_prd_typ,
		&client_pack_manager.orderbook.C_undrlyng,
		&client_pack_manager.orderbook.C_expry_dt,
		&client_pack_manager.orderbook.C_exrc_typ,
		&client_pack_manager.orderbook.C_opt_typ,
		&client_pack_manager.orderbook.L_strike_prc,
		&client_pack_manager.orderbook.C_ctgry_indstk,
		&client_pack_manager.orderbook.C_xchng_ack,
	)

	if err != nil {
		log.Printf("[%s] [fnRefToOrd] Error scanning row: %v", client_pack_manager.ServiceName, err)
		log.Printf("[%s] [fnRefToOrd] Exiting fnFetchOrderDetails with error", client_pack_manager.ServiceName)
		return -1
	}

	client_pack_manager.cPanNo = strings.TrimSpace(client_pack_manager.cPanNo)
	client_pack_manager.cLstActRef = strings.TrimSpace(client_pack_manager.cLstActRef)
	client_pack_manager.cEspID = strings.TrimSpace(client_pack_manager.cEspID)
	client_pack_manager.cAlgoID = strings.TrimSpace(client_pack_manager.cAlgoID)
	client_pack_manager.cSourceFlg = strings.TrimSpace(client_pack_manager.cSourceFlg)

	log.Printf("[%s] [fnRefToOrd] Data extracted and stored in the 'orderbook' structure:", client_pack_manager.ServiceName)

	log.Printf("[%s] [fnRefToOrd]   C_cln_mtch_accnt:   %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_cln_mtch_accnt)
	log.Printf("[%s] [fnRefToOrd]   C_ordr_flw:        %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_ordr_flw)
	log.Printf("[%s] [fnRefToOrd]   L_ord_tot_qty:     %d", client_pack_manager.ServiceName, client_pack_manager.orderbook.L_ord_tot_qty)
	log.Printf("[%s] [fnRefToOrd]   L_exctd_qty:       %d", client_pack_manager.ServiceName, client_pack_manager.orderbook.L_exctd_qty)
	log.Printf("[%s] [fnRefToOrd]   L_exctd_qty_day:   %d", client_pack_manager.ServiceName, client_pack_manager.orderbook.L_exctd_qty_day)
	log.Printf("[%s] [fnRefToOrd]   C_settlor:         %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_settlor)
	log.Printf("[%s] [fnRefToOrd]   C_spl_flg:         %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_spl_flg)
	log.Printf("[%s] [fnRefToOrd]   C_ack_tm:          %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_ack_tm)
	log.Printf("[%s] [fnRefToOrd]   C_prev_ack_tm:     %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_prev_ack_tm)
	log.Printf("[%s] [fnRefToOrd]   C_pro_cli_ind:     %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_pro_cli_ind)
	log.Printf("[%s] [fnRefToOrd]   C_ctcl_id:         %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_ctcl_id)
	log.Printf("[%s] [fnRefToOrd]   C_pan_no:          %s", client_pack_manager.ServiceName, client_pack_manager.cPanNo)
	log.Printf("[%s] [fnRefToOrd]   C_lst_act_ref_tmp: %s", client_pack_manager.ServiceName, client_pack_manager.cLstActRef)
	log.Printf("[%s] [fnRefToOrd]   C_esp_id:          %s", client_pack_manager.ServiceName, client_pack_manager.cEspID)
	log.Printf("[%s] [fnRefToOrd]   C_algo_id:         %s", client_pack_manager.ServiceName, client_pack_manager.cAlgoID)
	log.Printf("[%s] [fnRefToOrd]   C_source_flg_tmp:  %s", client_pack_manager.ServiceName, client_pack_manager.cSourceFlg)
	log.Printf("[%s] [fnRefToOrd]   L_mdfctn_cntr:     %d", client_pack_manager.ServiceName, client_pack_manager.orderbook.L_mdfctn_cntr)
	log.Printf("[%s] [fnRefToOrd]   C_ordr_stts:       %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_ordr_stts)
	log.Printf("[%s] [fnRefToOrd]   C_xchng_cd:        %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_xchng_cd)
	log.Printf("[%s] [fnRefToOrd]   C_prd_typ:         %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_prd_typ)
	log.Printf("[%s] [fnRefToOrd]   C_undrlyng:        %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_undrlyng)
	log.Printf("[%s] [fnRefToOrd]   C_expry_dt:        %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_expry_dt)
	log.Printf("[%s] [fnRefToOrd]   C_exrc_typ:        %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_exrc_typ)
	log.Printf("[%s] [fnRefToOrd]   C_opt_typ:         %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_opt_typ)
	log.Printf("[%s] [fnRefToOrd]   L_strike_prc:      %d", client_pack_manager.ServiceName, client_pack_manager.orderbook.L_strike_prc)
	log.Printf("[%s] [fnRefToOrd]   C_ctgry_indstk:    %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_ctgry_indstk)
	log.Printf("[%s] [fnRefToOrd]   C_xchng_ack	: 	   %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_xchng_ack)

	log.Printf("[%s] [fnRefToOrd] Data extracted and stored in the 'orderbook' structure successfully", client_pack_manager.ServiceName)

	clmQuery := `
    SELECT
    COALESCE(TRIM(CLM_CP_CD), ' '),
    COALESCE(TRIM(CLM_CLNT_CD), CLM_MTCH_ACCNT)
	FROM
    CLM_CLNT_MSTR
	WHERE
    CLM_MTCH_ACCNT = ?
    `
	log.Printf("[%s] [fnRefToOrd] Executing query to fetch data from 'CLM_CLNT_MSTR' ", client_pack_manager.ServiceName)

	row = db.Raw(clmQuery, strings.TrimSpace(client_pack_manager.orderbook.C_cln_mtch_accnt)).Row()

	var cCpCode, cUccCd string
	err = row.Scan(&cCpCode, &cUccCd)

	if err != nil {
		log.Printf("[%s] [fnRefToOrd] Error scanning client details: %v", client_pack_manager.ServiceName, err)
		log.Printf("[%s] [fnRefToOrd] Exiting fnRefToOrd with error", client_pack_manager.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnRefToOrd] cCpCode : %s", client_pack_manager.ServiceName, cCpCode)
	log.Printf("[%s] [fnRefToOrd] cUccCd : %s", client_pack_manager.ServiceName, cUccCd)

	log.Printf("[%s] [fnRefToOrd] Updating orderbook with client details", client_pack_manager.ServiceName)

	client_pack_manager.orderbook.C_cln_mtch_accnt = strings.TrimSpace(cUccCd)
	client_pack_manager.orderbook.C_settlor = strings.TrimSpace(cCpCode)
	client_pack_manager.orderbook.C_ctcl_id = strings.TrimSpace(client_pack_manager.orderbook.C_ctcl_id)

	log.Printf("[%s] [fnRefToOrd] Exiting fnFetchOrderDetails", client_pack_manager.ServiceName)
	return 0
}

/***********************************************************************************************
 * fnUpdXchngbk updates the status of a record in the 'fxb_fo_xchng_book' table based on the
 * operation type specified in the `Xchngbook` structure within the 'ClnPackClntManager' instance.
 * which are set in the 'fnGetNxtRec' .
 * The function handles two main operations:
 * 1. **Order Forwarding Update**: Updates the 'fxb_plcd_stts' and 'fxb_frwd_tm' fields for a record
 *    where 'fxb_ordr_rfrnc' and 'fxb_mdfctn_cntr' match the values in the `Xchngbook` structure.
 * 2. **Exchange Response Update**: If the download flag indicates the record should be processed, it
 *    checks if the record exists using the provided 'fxb_jiffy', 'fxb_xchng_cd', and 'fxb_pipe_id'.
 *    If the record exists, it logs that it has already been processed. Otherwise, it updates the record
 *    with the provided status, remarks, and other details.
 *
 * INPUT PARAMETERS:
 *    - db *gorm.DB: The database connection instance used for executing SQL queries.
 *
 * OUTPUT PARAMETERS:
 *    int - Returns 0 for success or -1 if an error occurs during the update operations.
 *
 * FUNCTIONAL FLOW:

 * 1. Based on the `C_oprn_typ` value in the `Xchngbook` structure:
 *    a. For **Order Forwarding** (`models.UPDATION_ON_ORDER_FORWARDING`):
 *       - Constructs and executes a SQL query to update 'fxb_plcd_stts' and 'fxb_frwd_tm' fields
 *         for the record that matches 'fxb_ordr_rfrnc' and 'fxb_mdfctn_cntr'.
 *    b. For **Exchange Response** (`models.UPDATION_ON_EXCHANGE_RESPONSE`):
 *       - Checks if the record should be processed based on the `L_dwnld_flg` value.
 *       - If processing is required, it verifies if a record already exists using the provided
 *         'fxb_jiffy', 'fxb_xchng_cd', and 'fxb_pipe_id'.
 *       - If the record is not found, updates the 'fxb_fo_xchng_book' table with new values including
 *         'fxb_plcd_stts', 'fxb_rms_prcsd_flg', 'fxb_ors_msg_typ', and 'fxb_ack_tm'.
 ***********************************************************************************************/

func (client_pack_manager *ClnPackClntManager) fnUpdXchngbk(db *gorm.DB) int {

	var iRecExists int64

	log.Printf("[%s] [fnUpdXchngbk] Entering fnUpdXchngbk", client_pack_manager.ServiceName)
	log.Printf("[%s] [fnUpdXchngbk]: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_xchng_cd)
	log.Printf("[%s] [fnUpdXchngbk]: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_ordr_rfrnc)
	log.Printf("[%s] [fnUpdXchngbk]: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_pipe_id)
	log.Printf("[%s] [fnUpdXchngbk]: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_mod_trd_dt)
	log.Printf("[%s] [fnUpdXchngbk]: %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_ord_seq)
	log.Printf("[%s] [fnUpdXchngbk]: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_slm_flg)
	log.Printf("[%s] [fnUpdXchngbk]: %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_dsclsd_qty)
	log.Printf("[%s] [fnUpdXchngbk]: %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_ord_tot_qty)
	log.Printf("[%s] [fnUpdXchngbk]: %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_ord_lmt_rt)
	log.Printf("[%s] [fnUpdXchngbk]: %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_stp_lss_tgr)
	log.Printf("[%s] [fnUpdXchngbk]: %d", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.L_mdfctn_cntr)
	log.Printf("[%s] [fnUpdXchngbk]: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_valid_dt)
	log.Printf("[%s] [fnUpdXchngbk]: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_ord_typ)
	log.Printf("[%s] [fnUpdXchngbk]: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_req_typ)
	log.Printf("[%s] [fnUpdXchngbk]: %s", client_pack_manager.ServiceName, client_pack_manager.Xchngbook.C_plcd_stts)

	switch client_pack_manager.Xchngbook.C_oprn_typ {
	case string(models.UPDATION_ON_ORDER_FORWARDING):

		query1 := `UPDATE fxb_fo_xchng_book 
				SET fxb_plcd_stts = ?, fxb_frwd_tm = CURRENT_TIMESTAMP
				WHERE fxb_ordr_rfrnc = ? 
				AND fxb_mdfctn_cntr = ?`

		result := db.Exec(query1, client_pack_manager.Xchngbook.C_plcd_stts, client_pack_manager.Xchngbook.C_ordr_rfrnc, client_pack_manager.Xchngbook.L_mdfctn_cntr)
		if result.Error != nil {
			log.Printf("[%s] [fnUpdXchngbk] Error updating order forwarding: %v", client_pack_manager.ServiceName, result.Error)
			return -1
		}

	case string(models.UPDATION_ON_EXCHANGE_RESPONSE):
		if client_pack_manager.Xchngbook.L_dwnld_flg == models.DOWNLOAD {

			result := db.Raw(
				`SELECT COUNT(*) 
					FROM fxb_fo_xchng_book 
					WHERE fxb_jiffy = ? 
					AND fxb_xchng_cd = ? 
					AND fxb_pipe_id = ?`,
				client_pack_manager.Xchngbook.D_jiffy, client_pack_manager.Xchngbook.C_xchng_cd, client_pack_manager.Xchngbook.C_pipe_id,
			).Scan(&iRecExists)
			if result.Error != nil {
				log.Printf("[%s] [fnUpdXchngbk] SQL error: %v", client_pack_manager.ServiceName, result.Error)
				return -1
			}

			if iRecExists > 0 {
				log.Printf("[%s] [fnUpdXchngbk] Record already processed", client_pack_manager.ServiceName)
				return -1
			}
		}

		c_xchng_rmrks := strings.TrimSpace(client_pack_manager.Xchngbook.C_xchng_rmrks)

		result := db.Exec(
			`UPDATE fxb_fo_xchng_book 
				SET fxb_plcd_stts = ?, 
					fxb_rms_prcsd_flg = ?, 
					fxb_ors_msg_typ = ?, 
					fxb_ack_tm = TO_DATE(?, 'DD-Mon-yyyy hh24:mi:ss'), 
					fxb_xchng_rmrks = RTRIM(fxb_xchng_rmrks) || ?, 
					fxb_jiffy = ? 
				WHERE fxb_ordr_rfrnc = ? 
				AND fxb_mdfctn_cntr = ?`,
			client_pack_manager.Xchngbook.C_plcd_stts,
			client_pack_manager.Xchngbook.C_rms_prcsd_flg,
			client_pack_manager.Xchngbook.L_ors_msg_typ,
			client_pack_manager.Xchngbook.C_ack_tm,
			c_xchng_rmrks,
			client_pack_manager.Xchngbook.D_jiffy,
			client_pack_manager.Xchngbook.C_ordr_rfrnc,
			client_pack_manager.Xchngbook.L_mdfctn_cntr,
		)
		if result.Error != nil {
			log.Printf("[%s] [fnUpdXchngbk] Error updating exchange response: %v", client_pack_manager.ServiceName, result.Error)
			return -1
		}

	default:
		log.Printf("[%s] [fnUpdXchngbk] Invalid Operation Type", client_pack_manager.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnUpdXchngbk] Exiting fnUpdXchngbk", client_pack_manager.ServiceName)

	return 0
}

/***********************************************************************************************
 * fnUpdOrdrbk updates the 'order status' of a record in the 'fod_fo_ordr_dtls' table based on
 * the details provided in the `orderbook` structure within the 'ClnPackClntManager' instance.
 * The function performs the following operations:
 * 1. **Order Status Update**: Updates the 'fod_ordr_stts' field for a record where the
 *    'fod_ordr_rfrnc' matches the value specified in the `orderbook` structure.
 *
 * INPUT PARAMETERS:
 *    - db *gorm.DB: The database connection instance used for executing SQL queries.
 *
 * OUTPUT PARAMETERS:
 *    int - Returns 0 for success or -1 if an error occurs during the update operation.
 *
 * FUNCTIONAL FLOW:
 * 1. Constructs and executes a SQL query to update the 'fod_ordr_stts' field in the
 *    'fod_fo_ordr_dtls' table where 'fod_ordr_rfrnc' matches the value from the `orderbook` structure.
 ***********************************************************************************************/

func (client_pack_manager *ClnPackClntManager) fnUpdOrdrbk(db *gorm.DB) int {

	log.Printf("[%s] [fnUpdOrdrbk] Starting 'fnUpdOrdbk' function", client_pack_manager.ServiceName)

	query := `
    UPDATE fod_fo_ordr_dtls
    SET fod_ordr_stts = ?
    WHERE fod_ordr_rfrnc = ?;
	`

	log.Printf("[%s] [fnUpdOrdrbk] Executing query to update order status", client_pack_manager.ServiceName)

	log.Printf("[%s] [fnUpdOrdrbk] Order Reference: %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_ordr_rfrnc)
	log.Printf("[%s] [fnUpdOrdrbk]  Order Status: %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_ordr_stts)

	result := db.Exec(query, client_pack_manager.orderbook.C_ordr_stts, strings.TrimSpace(client_pack_manager.orderbook.C_ordr_rfrnc))

	if result.Error != nil {
		log.Printf("[%s] [fnUpdOrdrbk] Error executing update query: %v", client_pack_manager.ServiceName, result.Error)
		log.Printf("[%s] [fnUpdOrdrbk] Exiting 'fnUpdOrdbk' with error", client_pack_manager.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnUpdOrdrbk] Order status updated successfully", client_pack_manager.ServiceName)

	log.Printf("[%s] [fnUpdOrdrbk] returning from 'fnUpdOrdbk' function", client_pack_manager.ServiceName)

	return 0
}

/***********************************************************************************************
 * fnGetExtCnt retrieves and sets external contract details based on the values provided
 * in the `contract` structure within the 'ClnPackClntManager' instance. This function performs
 * multiple operations to populate the `nse_contract` structure with relevant data.
 *
 * The function performs the following operations:
 * 1. **Determine Entity Type**: Based on the value of `C_xchng_cd` in the `contract` structure,
 *    sets the `i_sem_entity` variable to either `models.NFO_ENTTY` or `models.BFO_ENTTY`.
 *
 * 2. **Fetch Symbol**: Executes a query to fetch the stock symbol (`C_symbol`) from the
 *    'sem_stck_map' table using the stock code (`C_undrlyng`) and entity type.
 *
 * 3. **Determine Exchange Code**: Sets the exchange code (`c_exg_cd`) based on the value of
 *    `C_xchng_cd` in the `contract` structure.
 *
 * 4. **Fetch Series**: Executes a query to fetch the series (`C_series`) from the 'ESS_SGMNT_STCK'
 *    table using the underlying stock code (`C_undrlyng`) and exchange code (`c_exg_cd`).
 *
 * 5. **Fetch Token ID and CA Level**: Executes a query to fetch token ID (`L_token_id`) and
 *    CA level (`L_ca_lvl`) from the 'ftq_fo_trd_qt' table using various contract details including
 *    exchange code, product type, underlying stock code, expiry date, exercise type, option type,
 *    and strike price. The expiry date is formatted to 'dd-Mon-yyyy'.
 *
 * INPUT PARAMETERS:
 *    - db *gorm.DB: The database connection instance used for executing SQL queries.
 *
 * OUTPUT PARAMETERS:
 *    int - Returns 0 for success or -1 if an error occurs .
 *
 * FUNCTIONAL FLOW:
 * 1. Determines the entity type and fetches the symbol from 'sem_stck_map'.
 * 2. Based on the `C_xchng_cd`, sets the appropriate exchange code and fetches the series from
 *    'ESS_SGMNT_STCK'.
 * 3. Retrieves token ID and CA level from 'ftq_fo_trd_qt' using the formatted expiry date and
 *    other contract details.
 * 4. Updates the `nse_contract` structure with the fetched values.
 ***********************************************************************************************/

func (client_pack_manager *ClnPackClntManager) fnGetExtCnt(db *gorm.DB) int {
	var i_sem_entity int
	var c_stck_cd, c_symbl, c_exg_cd string

	log.Printf("[%s] [fnGetExtCnt] Entering 'fnGetExtCnt' ", client_pack_manager.ServiceName)

	if client_pack_manager.contract.C_xchng_cd == "NFO" {
		i_sem_entity = models.NFO_ENTTY
	} else {
		i_sem_entity = models.BFO_ENTTY
	}

	c_stck_cd = client_pack_manager.contract.C_undrlyng

	c_stck_cd = strings.ToUpper(c_stck_cd)
	log.Printf("[%s] [fnGetExtCnt] Converted 'c_stck_cd' to uppercase: %s", client_pack_manager.ServiceName, c_stck_cd)

	query1 := `
		SELECT TRIM(sem_map_vl)
		FROM sem_stck_map
		WHERE sem_entty = ?
		AND sem_stck_cd = ?;
	`
	log.Printf("[%s] [fnGetExtCnt] Executing query to fetch 'C_symbol'", client_pack_manager.ServiceName)

	row1 := db.Raw(query1, i_sem_entity, c_stck_cd).Row()

	err := row1.Scan(&c_symbl)

	if err != nil {
		log.Printf("[%s] [fnGetExtCnt] Error scanning row: %v", client_pack_manager.ServiceName, err)
		log.Printf("[%s] [fnGetExtCnt] Exiting 'fnGetExtCnt' due to error", client_pack_manager.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnGetExtCnt] Value successfully fetched from the table 'sem_stck_map'", client_pack_manager.ServiceName)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.C_symbol is: %s", client_pack_manager.ServiceName, c_symbl)

	// Assign values to the NSE contract structure
	client_pack_manager.nse_contract.C_xchng_cd = client_pack_manager.contract.C_xchng_cd
	client_pack_manager.nse_contract.C_prd_typ = client_pack_manager.contract.C_prd_typ
	client_pack_manager.nse_contract.C_expry_dt = client_pack_manager.contract.C_expry_dt
	client_pack_manager.nse_contract.C_exrc_typ = client_pack_manager.contract.C_exrc_typ
	client_pack_manager.nse_contract.C_opt_typ = client_pack_manager.contract.C_opt_typ
	client_pack_manager.nse_contract.L_strike_prc = client_pack_manager.contract.L_strike_prc
	client_pack_manager.nse_contract.C_symbol = c_symbl
	client_pack_manager.nse_contract.C_rqst_typ = client_pack_manager.contract.C_rqst_typ
	client_pack_manager.nse_contract.C_ctgry_indstk = client_pack_manager.contract.C_ctgry_indstk

	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.C_xchng_cd is: %s", client_pack_manager.ServiceName, client_pack_manager.nse_contract.C_xchng_cd)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.C_prd_typ is: %s", client_pack_manager.ServiceName, client_pack_manager.nse_contract.C_prd_typ)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.C_expry_dt is: %s", client_pack_manager.ServiceName, client_pack_manager.nse_contract.C_expry_dt)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.C_exrc_typ is: %s", client_pack_manager.ServiceName, client_pack_manager.nse_contract.C_exrc_typ)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.C_opt_typ is: %s", client_pack_manager.ServiceName, client_pack_manager.nse_contract.C_opt_typ)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.L_strike_prc is: %d", client_pack_manager.ServiceName, client_pack_manager.nse_contract.L_strike_prc)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.C_symbol is: %s", client_pack_manager.ServiceName, client_pack_manager.nse_contract.C_symbol)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.C_ctgry_indstk is: %s", client_pack_manager.ServiceName, client_pack_manager.nse_contract.C_ctgry_indstk)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.C_rqst_typ is: %s", client_pack_manager.ServiceName, client_pack_manager.nse_contract.C_rqst_typ)

	if client_pack_manager.contract.C_xchng_cd == "NFO" {
		c_exg_cd = "NSE"
	} else if client_pack_manager.contract.C_xchng_cd == "BFO" {
		c_exg_cd = "BSE"
	} else {
		log.Printf("[%s] [fnGetExtCnt] Invalid option '%s' for 'client_pack_manager.contract.C_xchng_cd'. Exiting 'fnGetExtCnt'", client_pack_manager.ServiceName, client_pack_manager.contract.C_xchng_cd)
		return -1
	}

	query2 := `
		SELECT COALESCE(ESS_XCHNG_SUB_SERIES, ' ')
		FROM ESS_SGMNT_STCK
		WHERE ess_stck_cd = ?
		AND ess_xchng_cd = ?;
	`

	log.Printf("[%s] [fnGetExtCnt] Executing query to fetch 'C_series'", client_pack_manager.ServiceName)

	row2 := db.Raw(query2, client_pack_manager.contract.C_undrlyng, c_exg_cd).Row()

	err2 := row2.Scan(&client_pack_manager.nse_contract.C_series)

	if err2 != nil {
		log.Printf("[%s] [fnGetExtCnt] Error scanning row: %v", client_pack_manager.ServiceName, err2)
		log.Printf("[%s] [fnGetExtCnt] Exiting 'fnGetExtCnt' due to error", client_pack_manager.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnGetExtCnt] Value successfully fetched from the table 'ESS_SGMNT_STCK'", client_pack_manager.ServiceName)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.C_series is: %s", client_pack_manager.ServiceName, client_pack_manager.nse_contract.C_series)

	query3 := `
		SELECT COALESCE(ftq_token_no, 0),
		       COALESCE(ftq_ca_lvl, 0)
		FROM ftq_fo_trd_qt
		WHERE ftq_xchng_cd = ?
		  AND ftq_prdct_typ = ?
		  AND ftq_undrlyng = ?
		  AND ftq_expry_dt = TO_DATE(?, 'dd-Mon-yyyy')
		  AND ftq_exer_typ = ?
		  AND ftq_opt_typ = ?
		  AND ftq_strk_prc = ?;
	`

	log.Printf("[%s] [fnGetExtCnt] Executing query to fetch 'L_token_id' and 'L_ca_lvl'", client_pack_manager.ServiceName)

	log.Printf("[%s] [fnGetExtCnt] C_xchng_cd: %s", client_pack_manager.ServiceName, client_pack_manager.contract.C_xchng_cd)
	log.Printf("[%s] [fnGetExtCnt] C_prd_typ: %s", client_pack_manager.ServiceName, client_pack_manager.contract.C_prd_typ)
	log.Printf("[%s] [fnGetExtCnt] C_undrlyng: %s", client_pack_manager.ServiceName, client_pack_manager.contract.C_undrlyng)
	log.Printf("[%s] [fnGetExtCnt] C_expry_dt: %s", client_pack_manager.ServiceName, client_pack_manager.contract.C_expry_dt)
	log.Printf("[%s] [fnGetExtCnt] C_exrc_typ: %s", client_pack_manager.ServiceName, client_pack_manager.contract.C_exrc_typ)
	log.Printf("[%s] [fnGetExtCnt] C_opt_typ: %s", client_pack_manager.ServiceName, client_pack_manager.contract.C_opt_typ)
	log.Printf("[%s] [fnGetExtCnt] L_strike_prc: %d", client_pack_manager.ServiceName, client_pack_manager.contract.L_strike_prc)

	parsedDate, err := time.Parse(time.RFC3339, client_pack_manager.contract.C_expry_dt)
	if err != nil {
		log.Printf("[%s] [fnGetExtCnt] Error parsing date: %v", client_pack_manager.ServiceName, err)
		log.Printf("[%s] [fnGetExtCnt] Exiting 'fnGetExtCnt' due to error", client_pack_manager.ServiceName)
		return -1
	}

	// Format the parsed date as 'dd-Mon-yyyy'
	formattedDate := parsedDate.Format("02-Jan-2006")

	row3 := db.Raw(query3,
		client_pack_manager.contract.C_xchng_cd,
		client_pack_manager.contract.C_prd_typ,
		client_pack_manager.contract.C_undrlyng,
		formattedDate,
		client_pack_manager.contract.C_exrc_typ,
		client_pack_manager.contract.C_opt_typ,
		client_pack_manager.contract.L_strike_prc,
	).Row()

	err3 := row3.Scan(&client_pack_manager.nse_contract.L_token_id, &client_pack_manager.nse_contract.L_ca_lvl)

	if err3 != nil {
		log.Printf("[%s] [fnGetExtCnt] Error scanning row: %v", client_pack_manager.ServiceName, err3)
		log.Printf("[%s] [fnGetExtCnt] Exiting 'fnGetExtCnt' due to error", client_pack_manager.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnGetExtCnt] Values successfully fetched from the table 'ftq_fo_trd_qt'", client_pack_manager.ServiceName)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.L_token_id is: %d", client_pack_manager.ServiceName, client_pack_manager.nse_contract.L_token_id)
	log.Printf("[%s] [fnGetExtCnt] client_pack_manager.nse_contract.L_ca_lvl is: %d", client_pack_manager.ServiceName, client_pack_manager.nse_contract.L_ca_lvl)

	return 0
}

func (client_pack_manager *ClnPackClntManager) fnRjctRcrd(db *gorm.DB) int {

	var resultTmp int
	var c_tm_stmp string

	log.Printf("[%s] [fnRjctRcrd] Function 'fnRjctRcrd' starts", client_pack_manager.ServiceName)

	/*
		// log.Printf("[%s] [fnRjctRcrd] Product type is : %s", client_pack_manager.ServiceName, client_pack_manager.orderbook.C_prd_typ)

		// if client_pack_manager.orderbook.C_prd_typ == models.FUTURES {
		// 	svc = "SFO_FUT_ACK"
		// } else {
		// 	svc = "SFO_OPT_ACK"
		// }
	*/

	query := `SELECT TO_CHAR(NOW(), 'DD-Mon-YYYY HH24:MI:SS') AS c_tm_stmp`

	row := db.Raw(query).Row()
	err := row.Scan(&c_tm_stmp)

	if err != nil {
		log.Printf("[%s] [fnRjctRcrd] Error getting the system time: %v", client_pack_manager.ServiceName, err)
		return -1
	}

	c_tm_stmp = strings.TrimSpace(c_tm_stmp)

	log.Printf("[%s] [fnRjctRcrd] Current timestamp: %s", client_pack_manager.ServiceName, c_tm_stmp)

	client_pack_manager.Xchngbook.C_plcd_stts = "REJECT"
	client_pack_manager.Xchngbook.C_rms_prcsd_flg = "NOT_PROCESSED"
	client_pack_manager.Xchngbook.L_ors_msg_typ = models.ORS_NEW_ORD_RJCT
	client_pack_manager.Xchngbook.C_ack_tm = c_tm_stmp
	client_pack_manager.Xchngbook.C_xchng_rmrks = "Token id not available"
	client_pack_manager.Xchngbook.D_jiffy = 0
	client_pack_manager.Xchngbook.C_oprn_typ = "UPDATION_ON_EXCHANGE_RESPONSE"

	log.Printf("[%s] [fnRjctRcrd] Before calling 'fnUpdXchngbk' on 'Reject Record' ", client_pack_manager.ServiceName)

	resultTmp = client_pack_manager.fnUpdXchngbk(db)

	if resultTmp != 0 {
		log.Printf("[%s] [fnRjctRcrd] returned from 'fnUpdXchngbk' with an Error", client_pack_manager.ServiceName)
		log.Printf("[%s] [fnRjctRcrd] exiting from 'fnRjctRcrd'", client_pack_manager.ServiceName)
		return -1
	}

	log.Printf("[%s] [fnRjctRcrd] Time Before calling 'fnUpdXchngbk' : %s ", client_pack_manager.ServiceName, c_tm_stmp)
	log.Printf("[%s] [fnRjctRcrd] Function 'fnRjctRcrd' ends successfully", client_pack_manager.ServiceName)
	return 0
}
