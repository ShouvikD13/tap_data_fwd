package util

import "log"

const (
	/*upd xchng book*/
	UPDATION_ON_ORDER_FORWARDING  = 'P'
	UPDATION_ON_EXCHANGE_RESPONSE = 'R'

	/*upd ordr book*/
	UPDATE_ORDER_STATUS = 'S'

	/**/
	ORDINARY_ORDER = 'O'

	FOR_NORM = 'N'

	/*placed status*/
	REQUESTED = 'R'
	QUEUED    = 'Q'
	CANCELLED = 'C'

	DOWNLOAD     = 10
	NOT_DOWNLOAD = 20

	CONTRACT_TO_NSE_ID = 'N'

	/**** SFO_GT_EXT_CNT ****/
	NFO_ENTTY = 1
	BFO_ENTTY = 2

	/* product type */
	FUTURES = 'F'

	// on reject order
	ORS_NEW_ORD_RJCT = 5304

	//length
	LEN_ALPHA_CHAR      = 2
	LEN_SETTLOR         = 12
	LEN_BROKER_ID       = 5
	LEN_FILLER_OPTIONS  = 3
	LEN_ACCOUNT_NUMBER  = 10
	LEN_REMARKS         = 24
	LEN_TIME_STAMP      = 8
	LEN_INSTRUMENT_NAME = 6
	LEN_SYMBOL_NSE      = 10
	LEN_OPTION_TYPE     = 2
	LEN_PAN             = 10 // this value i have deduce from original C code "char c_pan[10];				  /*** Ver 2.7 ***/"
	// Order & Trade management
	BOARD_LOT_IN = 2000

	TRADER            = 'T'
	CORPORATE_MANAGER = 'M'
	BRANCH_MANAGER    = 'B'

	//for Expiry DT
	TEN_YRS_IN_SEC = 315513000

	/**** Order request type ****/
	NEW = 'N'
	// MODIFY = "M"
	// CANCEL = "C"

	//Book type

	REGULAR_LOT_ORDER   = 1
	STOP_LOSS_MIT_ORDER = 3

	//Buy Sell indicator
	NSE_BUY  = 1
	NSE_SELL = 2

	// Order placed by Broker / Customer Indicator

	BRKR_PLCD  = 'P'
	CSTMR_PLCD = 'C'

	//
	NSE_CLIENT = 1
	NSE_PRO    = 2

	FO_AUTO_MTM_ALG_ID      = 24264
	FO_AUTO_MTM_ALG_CAT_ID  = 99
	FO_PRICE_IMP_ALG_ID     = 37383
	FO_PRICE_IMP_ALG_CAT_ID = 1
	FO_PRFT_ORD_ALG_CAT_ID  = 1
	FO_PRFT_ORD_ALG_ID      = 82686
	FO_FLASH_TRD_ALG_CAT_ID = 1
	FO_FLASH_TRD_ALG_ID     = 89385
	FO_NON_ALG_ID           = 0
	FO_NON_ALG_CAT_ID       = 0
	FO_FOGTT_ALG_ID         = 98816
	FO_FOGTT_ALG_CAT_ID     = 1

	MAX_SEQ_NUM = 2147483647
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
