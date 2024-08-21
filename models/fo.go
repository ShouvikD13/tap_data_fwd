package models

import "log"

const (
	/*upd xchng book*/
	UPDATION_ON_ORDER_FORWARDING  = "P"
	UPDATION_ON_EXCHANGE_RESPONSE = "R"

	/*upd ordr book*/
	UPDATE_ORDER_STATUS = "S"

	/**/
	ORDINARY_ORDER = "O"

	FOR_NORM = "N"

	/*placed status*/
	REQUESTED = "R"
	QUEUED    = "Q"
	CANCELLED = "C"

	DOWNLOAD     = 10
	NOT_DOWNLOAD = 20

	CONTRACT_TO_NSE_ID = "N"

	/**** SFO_GT_EXT_CNT ****/
	NFO_ENTTY = 1
	BFO_ENTTY = 2

	/* product type */
	FUTURES = "F"

	// on reject order
	ORS_NEW_ORD_RJCT = 5304

	//length
	LEN_ALPHA_CHAR = 2

	// Order & Trade management
	BOARD_LOT_IN = 2000

	TRADER            = "T"
	CORPORATE_MANAGER = "M"
	BRANCH_MANAGER    = "B"

	//for Expiry DT
	TEN_YRS_IN_SEC = 315513000

	/**** Order request type ****/
	NEW    = "N"
	MODIFY = "M"
	CANCEL = "C"

	//Book type

	REGULAR_LOT_ORDER   = 1
	STOP_LOSS_MIT_ORDER = 3

	//Buy Sell indicator
	NSE_BUY  = 1
	NSE_SELL = 2
)

/*********************************************************************/
/*                                                                   */
/*  Description : Sets a null character ('\0') at a specified        */
/*                position in a byte slice, marking the end of the   */
/*                string in the slice.                               */
/*                                                                   */
/*********************************************************************/

type FO_Manager struct {
}

func (FOM *FO_Manager) SETNULL(serviceName string, a []byte, length int) {
	if length < len(a) {
		a[length] = 0
		log.Printf("[%s] Set null character at position %d in byte slice", serviceName, length)
	} else {
		log.Printf("[%s] Length %d exceeds byte slice size %d. No action taken.", serviceName, length, len(a))
	}
}
