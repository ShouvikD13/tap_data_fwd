/*Ver 1.2 02-Jan-2009 -  Extended Market definition - Shailesh */
/*Ver 1.3 14-Jan-2009 -  Trade Reconcilation - Shailesh */
/*Ver 1.4 25-Jun-2009 -  ORS2 - Sandeep */
/*Ver 1.5 25-Jun-2009 -  PartConvetrt to Future Plus-Vidya ***/
/*Ver 1.6 23-Dec-2009 -  Pack client excercise mkt provision - Shailesh */
/*Ver 1.7 21-Jul-2010 -  New Requset Type Introduced For Close Out Expiry -   Sandeep Patil ***/
/*Ver	1.8	27-Oct-2010	-	 CRCON40299 Additional Margin Sandeep Patil ***/
/*Ver 1.9 23-Nov-2010 -  New Operation Type Introduced For Silver Light -     Sandeep Patil ***/
/*Ver	2.0	21-Jan-2011	-	 To add ENABLED and DISABLED for get contracts	 - sangeet	***/
/*Ver 2.1	05-Aug-2011	-  New Operation Type Introduced For Mobile App in Order book - Navina **/ 
/*Ver 2.2 27-Apr-2011	-	 SPAN	Changes	-		Mahesh Shinde	**/
/*Ver 2.3 10-Feb-2012 -  Limit Negativity - Mahesh  ***/
/*Ver 2.4 20-Jan02012 -  CR-ISEC10-18963 Convert From Future to Future Plus - Mahesh */
/*Ver 2.5 19-Jan-2012 -  FNO BSE Changes Sachin Birje*/
/*Ver 2.6 08-Jun-2012 -  CR-ISEC14-26850 Convert from Future plus to Future - Mahesh */
/*Ver 2.7 01-Nov-2012	-	 ROLLOVER	- Mahesh */	 
/*Ver 2.8 29-Nov-2012	-	 Span-IOC/2L/3L order Handling - Swati A.B */ 
/*Ver 2.9 29-Nov-2012 -  SLTP FuturePLUS Handling - Navina ***/
/*Ver 3.0 05-Feb-2013 -  SPAN Future Plus - Mahesh ***/
/*Ver 3.1 05-Feb-2013 -  Broker closeout RRM changes - Mahesh ***/
/*Ver 3.2 23-Sep-2013 -  OPTIONPLUS Handling - Navina ***/
/*Ver 3.3 02-Dec-2013 -  VTC Order (Sachin Birje) ***/
/*Ver 3.4 20-Jan-2014 -  Contract Note Changes ( Sachin Birje ) ***/
/*Ver 3.6 05-Nov-2014 -  NNF Trade execution range broadcast change (Navina D.) */
/*Ver 3.7 21-Oct-2015 -  Conversion from fpsl to fut and opls to opt changes (Navina D.) */
/*Ver 3.8 16-Oct-2015 -  Cloud order Changes (Kishor B.)*/
/*Ver 3.9 29-Nov-2017 -  EOD MTM & Contract split changes (Sachin Birje)*/
/*Ver 4.0 07-Mar-2018 -  Rollover With Spread changes (Sachin Birje)*/
/*Ver 4.1 07-Mar-2018 -  Physical Delivery Margin Changes (Sachin Birje)*/
/*Ver 4.2 25-Nov-2019 -  Sensibull (Tanmay Patel)*/
/*Ver 4.3 25-Sep-2020 -  Unsolicited cancellation without reason code( Suchita )*/
/*Ver 4.5 29-Jul-2021 -  CR-ISEC10-156987 MarginShotfall_Communication ( Tanmay Patel ) */ 
/*Ver 4.6 01-Sep-2021 -  CR-ISEC14-152298 changes ( Suchita Dabir ) */
/*Ver 4.7 16-Jun-2021 -  CR_ISEC04_132410_delivery_settlement ( Abhinav Kumbhar )*/
/*Ver 4.8 16-Nov-2021 -  CR_ISEC04_132410_delivery_settlement live issue ( Suchita Dabir )*/
/*Ver 4.9 26-Nov-2021 -  FPSL add margin ( Suchita Dabir )*/
/* Ver 5.0 20-Jun-2022 - Option buy Premium ( Suchita ) */
/*#include <debug.h>*//*****Commented for IBM****/

#include <string.h>  /* VER TOL : TUX ON LINUX -- Added for strerror definition */

/****************************************************************************/
/**** GLOBAL APPLICATION SYSTEM VALUES                                   ****/
/****************************************************************************/

/**** Error string formatting macros ****/
#define SQLCODE sqlca.sqlcode
#define SQLMSG  sqlca.sqlerrm.sqlerrmc
#define TPCODE tperrno
#define TPMSG  tpstrerror(tperrno)
#define FMLMSG Fstrerror32(Ferror32)
#define UNXMSG strerror(errno)  
#define ODBC_MSG ODBC_error(hstmt)
#define DEFMSG "Null message"
#define LIBMSG "Error in library function"

/* added MEMSET ver 1.1 (DSK) */
#define MEMSET(A) memset(&A,'\0',sizeof(A))

/**** Macros for handling varchars ****/
#define SETLEN(a)  a.len = strlen((char *)a.arr)
#define SETNULL(a) a.arr[a.len]='\0'

/**** Oracle standard error codes ****/
#define NO_DATA_FOUND 1403
#define TOO_MANY_ROWS_FETCHED -2112
#define NULL_RETURN  -1405

/**** Minimum buffer length ****/
#define MIN_FML_BUF_LEN  1024

/**** Default session and user id ****/
#define DEF_SSSN 0L
#define DEF_USR  "system"

/**** General values ****/
#define SUCCESS    0
#define FAILURE   -1
#define YES        'Y'
#define NO         'N'
#define TRUE       1
#define FALSE      0
#define EXIT       0
#define DONT_EXIT  1

/**** Standard values for tpurcode ****/
#define	NO_BFR		-2
#define ERR_BFR		-1
#define	SUCC_BFR	 0
#define	RECORD_ALREADY_PRCSD	 1
#define	INSUFFICIENT_LIMITS	 -3

/**** Flag for registering and unregistering batch programs ****/
#define REGISTER   'R'
#define UNREGISTER 'U'

/**** Length of qspace and qname variables in tuxedo ****/
#define QSPACE_LEN 16
#define QNAME_LEN  16

/**** Flags for DDR ****/
/*******************************************************
For NFO_O_PRD_ID, DP_O_PRD_ID, MTCH_PRD_ID, MAIL_PRD_ID
*******************************************************/
#define BACKOFFICE          0
#define PORTFOLIO           1
#define FUTURE              2
#define OPTION              3


/*******************************************************
For EBA_O_PIP_ID, ODIN_PIP_ID
********************************************************/
#define STR_11              "11"
#define STR_21              "21"

/****************************************************************************/
/**** LENGTH OF DATA TYPE                                                ****/
/****************************************************************************/
#define LEN_DATE	             21
#define LEN_USR_TRDNG_PSSWRD   51

/****************************************************************************/
/**** GLOBAL APPLICATION BUSINESS VALUES                                 ****/
/****************************************************************************/

/**** Product types ****/
#define FUTURES 'F'
#define OPTIONS 'O'
#define FUTURE_PLUS 'P'
#define SPAN		'S'			/***	Ver	2.2	***/
#define SLTP_FUTUREPLUS 'U' /*** Ver 2.9 ***/
#define OPTIONPLUS 'I'			/*** Ver 3.2 ***/

/**** Option types ****/
#define	CALL	'C'
#define	PUT		'P'

/**** Tradable entity type ****/
#define INDEX 'I'
#define STOCK 'S'

/**** Order flow ****/
#define BUY  'B'
#define SELL 'S'
#define NEUTRAL 'N'

/**** Order type classifications ****/
#define LIMIT     'L'
#define MARKET    'M'
#define STOP_LOSS 'S'
#define VTC_ORDER 'V'  /** Ver 3.3 **/

#define NORMAL_ORDER 			'N'
#define SPREAD_ORDER 			'P'
#define L2_ORDER 					'2'
#define L3_ORDER 					'3'
#define ARBITRAGE     		'A'
#define CLIENT_SQROFF 		'C'
#define	SYSTEM_SQUAREOFF	'S' 
#define	JOINT_SQUAREOFF		'J' 
#define	ROLLOVER					'R'     /*** Ver 2.7 ***/ 
#define	ROLLOVER_WITH_SPREAD					'O'     /*** Ver 4.0 ***/ 
/** Added for Temporary order on 02/06/02 **/
#define	TEMPORARY_ORDER		'T'
#define LMTNEG_SYS_SQUAREOFF 'L'  /** Ver 3.4 **/
#define IMTM_SYS_SQUAREOFF 'M' 		/** Ver 3.4 **/
#define PHYSICAL_DLVRY_SQOFF 'D'    /** Ver 4.1 **/

/**Added by Shailesh for futureplus **/
/**Conversion from futureplus to future: SFO_CONV_TO_FUT **/
#define SINGLE_CONVERSION 'S'		/**FuturePlus**/
#define JOINT_CONVERSION  'J'  /**FuturePlus**/ 

/**Conversion from futureplus to future: SFO_GT_FUTPLUS ****/
#define GET_ALL_PSTN                  'A'
#define GET_UNDRLYNG_PSTN							'B'
#define GET_MATCH_PSTN								'C'
#define GET_MATCH_UNDRLYNG_PSTN				'D'
#define GET_TAG_PSTN									'E'
#define GET_TAG_UNDRLYNG_PSTN					'F'
#define GET_TAG_MATCH_PSTN						'G'
#define GET_TAG_MATCH_UNDRLYNG_PSTN		'H'

#define GOOD_TILL_TODAY  	  'T'
#define GOOD_TILL_DATE 	    'D'
#define IMMEDIATE_OR_CANCEL 'I'

/**** Order, exercise / request status ****/
#define REQUESTED 				  'R'
#define QUEUED 					    'Q'
#define CANCELLED					  'C'

#define PLACED              'E'
#define ACCEPT              'A'
#define REJECT              'J'
#define FREEZED             'F'
#define TRIGGERED           'T'

#define ORDERED 					  							'O'
#define PARTIALLY_EXECUTED								'P'
#define EXECUTED 				    							'E'
#define REJECTED 				    							'J'
#define EXPIRED 					  							'X'
#define PARTIALLY_EXECUTED_AND_EXPIRED		'B'
#define PARTIALLY_EXECUTED_AND_CANCELLED	'D'
#define EXPIRED 					  							'X'
#define UNDER_PROCESS 					  				'U'

/**** Exercise status ****/
#define EXERCISED           		'E'

/**** Order request type ****/
#define NEW    'N'
#define MODIFY 'M'
#define CANCEL 'C'

/* Added for Future Plus */
#define FPEOS_UNDRLYNG_PIPE               'B'
#define FPEOS_ALL_UND_ALL_PIPE            'A'
#define FPEOS_UND_ALL_PIPE                'C'
#define FPEOS_ALL_UND_PIPE                'D'

/**** Modification counter for expiry record ****/
#define EXPIRY_MDFCTN_CNTR 99

/**** Select all option ****/
#define ALL 'A'

/**** Exchange book order type ****/
#define ORDINARY_ORDER 	    'O'
#define EXERCISE 		        'E'
#define DONT_EXERCISE       'D'

/**** Exchange status ****/
#define EXCHANGE_UNDER_EXPIRY 	'X'
#define EXCHANGE_OPEN 	        'O'
#define EXCHANGE_CLOSE 	        'C'
#define EXCHANGE_SUSPEND        'S'
 

/***  Narration id for limits ***/

    #define ON_ORDER_PLACEMENT          "A"
    #define ON_ORDER_ACCEPTANCE         "B"
    #define ON_ORDER_REJECTION          "C"
    #define ON_MODIFICATION_PLACEMENT   "D"
    #define ON_MODIFICATION_ACCEPTANCE  "E"
    #define ON_MODIFICATION_REJECTION   "F"
    #define ON_CANCELLATION_PLACEMENT   "G"
    #define ON_CANCELLATION_ACCEPTANCE  "H"
    #define ON_CANCELLATION_REJECT      "I"
    #define ON_TRADE_EXECUTION          "J"
    #define ON_EXPIRY          					"K"
    #define ON_SYSTEM_ADD_MARGIN        "L"
    #define ADJUSTMENT_ON_EODMTM        "M"
    #define PROFIT_LOSS_ON_EOD_MTM      "N"
    #define RELEASE_BLKD_PROFIT_LOSS_ON_EOD_MTM "PL"    /*** Ver 2.2 ***/
    #define MARGIN_RLSD_FOR_COVER_LEG   "O"
    #define MARGIN_BLKD_FOR_FRESH_LEG   "P"
    #define PROFIT_LOSS_ON_COVER_LEG    "Q"
    #define SPREAD_MARGIN_ADJ						"R"
    #define SPREAD_PROFIT_LOSS_ADJ 			"S"
    #define MARGIN_ADJ_ON_TRD_EXEC      "T"
    #define RELEASE_OF_ADD_MARGIN       "U"
    #define RELEASE_OF_ADDMARGIN_ON_EODMTM       "V"
    #define CLOSING_OF_UNCRYSTALIZED_BAL       "W"
    #define UNCRYSTALIZED_BAL_BROUGHT_FORWARD       "X"
    #define RELEASE_OF_TRD_MRGN_ON_EODMTM       "Y"
    #define MARGIN_BLOCKED_ON_EODMTM       "Z"
    #define ORD_MRGN_ADJ_ON_CONTRACT_SPLIT      "A1"
    #define TRD_MRGN_ADJ_ON_CONTRACT_SPLIT      "A2"
    #define RELEASE_OF_MRGN_ON_CONTRACT_SPLIT      "A3"
    #define MARGIN_ADJ_ON_CLOSE_OUT      "C1"
    #define PREMIUM_ADJUSTMENTS      "PA"
    #define PROFIT_ON_EXER      "AA"
    #define LOSS_ON_ASGMNT      "AB"
    #define ON_USER_ADD_MARGIN  "AC"
    #define MARGIN_ADJ_ON_EXER_CONF  "AD"
    #define MARGIN_ADJ_ON_ASGMNT  "AE"
    #define MARGIN_ADJ_ON_CHG_OF_MRGN_PRCNT  "AF"
    #define ADDHOC_ADD_MRGN   "AM"                    /***  Ver 1.8 ***/
    #define MANUAL_ADJ_LMT_G1 "Z1"
    #define MANUAL_ADJ_PRJ_G1 "Z2"
    #define MANUAL_ADJ_LMT_G2 "Y1"
    #define MANUAL_ADJ_PRJ_G2 "Y2"
    #define ON_EXERCISE_PLACEMENT                 "E1"
    #define ON_EXERCISE_ACCEPTANCE                "E2"
    #define ON_EXERCISE_REJECTION                 "E3"
    #define ON_EXERCISE_MODIFICATION_PLACEMENT    "E4"
    #define ON_EXERCISE_MODIFICATION_ACCEPTANCE   "E5"
    #define ON_EXERCISE_MODIFICATION_REJECTION    "E6"
    #define ON_EXERCISE_CANCELLATION_PLACEMENT    "E7"
    #define ON_EXERCISE_CANCELLATION_ACCEPTANCE   "E8"
    #define ON_EXERCISE_CANCELLATION_REJECT   		"E9"
    #define ON_EXERCISE_EXPIRY   									"EE"
    #define RELEASE_OF_TRADE_MRGN       					"A4"
    #define TDS_NRI       												"TD"
    #define CLOSING_OF_UNCRYSTALIZED_TDS		       "TD1"
    #define UNCRYSTALIZED_TDS_BROUGHT_FORWARD      "TD3"
    #define ORDER_MARGIN_ADJ_ON_FUT_CONV           "FPO"                 /**FuturePlus**/
    #define TRD_MARGIN_ADJ_ON_FUT_CONV             "FPT"                 /**FuturePlus**/
    #define DEFICIT_PROFIT_LOSS										 "N1"									/***	Ver	2.2	SPAN	***/
    #define REL_TRDMRGN_CONV_SPAN_TO_NORM					 "L1"
    #define REL_BLKDLOSS_CONV_SPAN_TO_NORM					 "L2"
    #define BLK_TRDMRGN_CONV_SPAN_TO_NORM					 "L3"
    #define REL_TRDMRGN_CONV_NORM_TO_SPAN					 "L4"
    #define BLK_TRDMRGN_CONV_NORM_TO_SPAN					 "L5"
    #define BLK_NOTIONAL_LOSS_CONV_NORM_TO_SPAN					 "L6"
    #define BLK_DEFICIT_PNL_IMTM										 "L7"
    #define REL_TRDMRGN_ORDER_EXPIRY	"L8"
    #define REL_BLKDLOSS_ORDER_EXPIRY	"L9"
    #define BLK_SPAN_MRGN_ORDER_EXPIRY	"L10"
    #define BLK_EXPR_MRGN_ORDER_EXPIRY	"L11"
    #define BLK_NOTIONAL_LOSS_ORDER_EXPIRY	"L12"
    #define REL_TRDMRGN_ON_ASSIGNMENT	"L13"
    #define REL_BLKDLOSS_ON_ASSIGNMENT	"L14"
    #define BLK_SPAN_MRGN_ON_ASSIGNMENT	"L15"
    #define BLK_EXPR_MRGN_ON_ASSIGNMENT	"L16"
    #define BLK_NOTIONAL_LOSS_ON_ASSIGNMENT	"L17"
    #define REL_TRDMRGN_ON_IMTM	"L18"
    #define REL_BLKDLOSS_ON_IMTM	"L19"
    #define BLK_SPAN_MRGN_ON_IMTM	"L20"
    #define BLK_EXPR_MRGN_ON_IMTM	"L21"
    #define BLK_NOTIONAL_LOSS_ON_IMTM	"L22"
    #define BLK_TRD_MRGN_ON_IMTM	"L23"
    #define REL_TRDMRGN_ON_SYS_CANCELLATION	"L24"
    #define REL_BLKDLOSS_ON_SYS_CANCELLATION	"L25"
    #define BLK_SPAN_MRGN_ON_SYS_CANCELLATION "L26"
    #define BLK_EXPR_MRGN_ON_SYS_CANCELLATION "L27"
    #define BLK_NOTIONAL_ON_SYS_CANCELLATION "L28"
    #define REL_TRDMRGN_ON_SQR_ORDR_CANCELLATION "LA5"
    #define REL_BLKDLOSS_ON_SQR_ORDR_CANCELLATION  "LA6"
    #define BLK_SPAN_MRGN_ON_SQR_ORDR_CANCELLATION "LA7"
    #define BLK_EXPR_MRGN_ON_SQR_ORDR_CANCELLATION "LA8"
    #define BLK_NOTIONAL_ON_SQR_ORDR_CANCELLATION "LA9"
    #define REL_TRDMRGN_ON_RESPONSE "L29"
    #define REL_BLKDLOSS_ON_RESPONSE	"L30"
    #define BLK_SPAN_MRGN_ON_RESPONSE "L31"
    #define BLK_EXPR_MRGN_ON_RESPONSE	"L32"
    #define BLK_NOTIONAL_LOSS_ON_RESPONSE "L33"
    #define REL_TRDMRGN_SYS_ORDER_PLACEMENT	"L34"
    #define REL_BLKDLOSS_SYS_ORDER_PLACEMENT "L35"
    #define BLK_SPAN_MRGN_SYS_ORDER_PLACEMENT "L36"
    #define BLK_EXPR_MRGN_SYS_ORDER_PLACEMENT "L37"
    #define BLK_NOTIONAL_LOSS_SYS_ORDER_PLACEMENT "L38"
    #define REL_TRDMRGN_SQR_ORDER_MODIFICATION	"L39"
    #define REL_BLKDLOSS_SQR_ORDER_MODIFICATION "L40"
    #define BLK_SPAN_MRGN_SQR_ORDER_MODIFICATION "L41"
    #define BLK_EXPR_MRGN_SQR_ORDER_MODIFICATION "L42"
    #define BLK_NOTIONAL_LOSS_SQR_ORDER_MODIFICATION "L43"
    #define REL_TRDMRGN_TRADE_CONFIRMATION	"L44"
    #define REL_BLKDLOSS_TRADE_CONFIRMATION "L45"
    #define BLK_SPAN_MRGN_TRADE_CONFIRMATION "L46"
    #define BLK_EXPR_MRGN_TRADE_CONFIRMATION "L47"
    #define BLK_NOTIONAL_LOSS_TRADE_CONFIRMATION "L48"
    #define REL_TRDMRGN_ON_CLOSEOUT	"L49"
    #define REL_BLKDLOSS_ON_CLOSEOUT "L50"
    #define BLK_SPAN_MRGN_ON_CLOSEOUT "L51"
    #define BLK_EXPR_MRGN_ON_CLOSEOUT "L52"
    #define BLK_NOTIONAL_LOSS_ON_CLOSEOUT	"L53"
    #define REL_TRDMRGN_ON_EXCERCISE_CONF	"L54"
    #define REL_BLKDLOSS_ON_EXCERCISE_CONF "L55"
    #define BLK_SPAN_MRGN_ON_EXCERCISE_CONF "L56"
    #define BLK_EXPR_MRGN_ON_EXCERCISE_CONF "L57"
    #define BLK_NOTIONAL_LOSS_ON_EXCERCISE_CONF "L58"
    #define REL_TRDMRGN_ON_EXCERCISE_RJCT	"L59"
    #define REL_BLKDLOSS_ON_EXCERCISE_RJCT "L60"
    #define BLK_SPAN_MRGN_ON_EXCERCISE_RJCT "L61"
    #define BLK_EXPR_MRGN_ON_EXCERCISE_RJCT "L62"
    #define BLK_NOTIONAL_LOSS_ON_EXCERCISE_RJCT "L63"
    #define REL_TRDMRGN_ON_NEWORD_RJCT	"L64"
    #define REL_BLKDLOSS_ON_NEWORD_RJCT "L65"
    #define BLK_SPAN_MRGN_ON_NEWORD_RJCT "L66"
    #define BLK_EXPR_MRGN_ON_NEWORD_RJCT "L67"
    #define BLK_NOTIONAL_LOSS_ON_NEWORD_RJCT "L68"
    #define REL_TRDMRGN_ON_MODORD_RJCT	"L69"
    #define REL_BLKDLOSS_ON_MODORD_RJCT "L70"
    #define BLK_SPAN_MRGN_ON_MODORD_RJCT "L71"
    #define BLK_EXPR_MRGN_ON_MODORD_RJCT "L72"
    #define BLK_NOTIONAL_LOSS_ON_MODORD_RJCT "L73"
    #define REL_TRDMRGN_ON_CANORD_RJCT	"L74"
    #define REL_BLKDLOSS_ON_CANORD_RJCT "L75"
    #define BLK_SPAN_MRGN_ON_CANORD_RJCT "L76"
    #define BLK_EXPR_MRGN_ON_CANORD_RJCT "L77"
    #define BLK_NOTIONAL_LOSS_ON_CANORD_RJCT "L78"
    #define REL_TRDMRGN_ON_NEWORD_ACPT	"L79"
    #define REL_BLKDLOSS_ON_NEWORD_ACPT "L80"
    #define BLK_SPAN_MRGN_ON_NEWORD_ACPT "L81"
    #define BLK_EXPR_MRGN_ON_NEWORD_ACPT "L82"
    #define BLK_NOTIONAL_LOSS_ON_NEWORD_ACPT "L83"
    #define REL_TRDMRGN_ON_MODORD_ACPT "L84"
    #define REL_BLKDLOSS_ON_MODORD_ACPT "L85"
    #define BLK_SPAN_MRGN_ON_MODORD_ACPT "L86"
    #define BLK_EXPR_MRGN_ON_MODORD_ACPT "L87"
    #define BLK_NOTIONAL_LOSS_ON_MODORD_ACPT "L88"
    #define REL_TRDMRGN_ON_CANORD_ACPT "L89"
    #define REL_BLKDLOSS_ON_CANORD_ACPT "L90"
    #define BLK_SPAN_MRGN_ON_CANORD_ACPT "L91"
    #define BLK_EXPR_MRGN_ON_CANORD_ACPT "L92"
    #define BLK_NOTIONAL_LOSS_ON_CANORD_ACPT	"L93"
    #define BLK_SPAN_MRGN_ON_EODMTM "L94"
    #define BLK_EXPR_MRGN_ON_EODMTM "L95"
    #define REL_TRDMRGN_SQR_ORDER_PLACEMENT "L96"
    #define REL_BLKDLOSS_SQR_ORDER_PLACEMENT "L97"
    #define BLK_SPAN_MRGN_SQR_ORDER_PLACEMENT "L98"
    #define BLK_EXPR_MRGN_SQR_ORDER_PLACEMENT "L99"
    #define BLK_NOTIONAL_LOSS_SQR_ORDER_PLACEMENT  "LA1"
    #define REL_BLKDLOSS_CONV_NORM_TO_SPAN         "LA2"
    #define BLK_EXPR_MRGN_CONV_NORM_TO_SPAN        "LA3"
    #define BLK_OPTN_PRMUM_CONV_NORM_TO_SPAN       "LA4"
    #define REL_TRDMRGN_ON_CHG_OF_MRGN_PRCNT			 "LB1"
    #define	SPAN_MRGN_ADJ_ON_CHG_OF_MRGN_PRCNT     "LB2"
    #define EXPR_MRGN_ADJ_ON_CHG_OF_MRGN_PRCNT     "LB3" 
    #define NOTIONAL_LOSS_ADJ_ON_CHG_OF_PNL				 "LB4"
    #define TRDMRGN_Adj_ON_CHG_OF_PNL							 "LB5"
    #define NOTIONAL_LOSS_ADJ_ON_CHG_OF_MRGN_PRCNT_AND_PNL      "LB6"
    #define MRGN_ADJ_ON_CHG_OF_MRGN_PRCNT_AND_PNL		"LB7" 
    #define RELEASE_OF_ORDER_MARGIN_ON_EOD_MTM  "VO"   /** Ver 3.9 **/
    #define BLOCK_OF_ORDER_MARGIN_ON_EOD_MTM    "VB"   /** Ver 3.9 **/ 
    #define BLOCK_ON_PHYDLVRY_OPT_EOS            "DM1" /*** Ver 4.1 ***/
    #define RELEASE_ON_PHYDLVRY_OPT_EOS          "DM2" /*** Ver 4.1 ***/
    #define REL_PHYSICAL_DELIVERY_MARGIN    "PD1" /*** Ver 4.7 ***/
    #define BLOCK_PHYSICAL_DELIVERY_MARGIN  "PD2" /*** Ver 4.7 ***/
    #define BLOCK_PHYSICAL_DELIVERY_MARGIN_FOBG  "PD4"  /*** Ver 4.7 ***/
    #define REL_PHYSICAL_DELIVERY_MARGIN_REM_POSITION "PD3" /*** Ver 4.7 ***/
    #define REL_PHYSICAL_DELIVERY_MARGIN_SHARES "PD6" /*** ver 4.8 ***/
    #define BLK_DLVRY_OBLIGATION           "DM3" /*** Ver 4.1 ***/
    #define ON_SUBSCRIPTION  "DM4" /** Ver 4.2 **/ 


/**** Debit type for updating limits ****/
    #define DEBIT_WHEN_LIMIT    'N'
    #define DEBIT_WITHOUT_LIMIT 'Y'
    #define DEBIT_TILL_LIMIT    'E'
    #define BLOCK_ADD_OPLS_MRGN "OA1" /** ver 4.6 ***/
    #define BLOCK_ADD_FPSL_MRGN "OA3" /** ver 4.9 ***/
    #define BLOCK_ADD_FP_MRGN "OA2"   /** ver 4.9 ***/

/**** Debit/credit flag and flags used in bank transactions****/
#define DEBIT  'D'
#define CREDIT 'C'
#define BLOCK  'B'
#define UNBLOCK_DEBIT  'U'

/*****  RMS flag value  ***/
#define NOT_PROCESSED 'N'
#define PROCESSED     'P'

/*****  Download flag value ***/
# define DOWNLOAD     10
# define NOT_DOWNLOAD     20

/****************************************************************************/
/**** OPERATION TYPES                                                    ****/
/****************************************************************************/

/**** SFO_REF_TO_OMD ****/
#define FOR_SPRD  'S'
#define FOR_NORM  'N'
#define FOR_EXTEND 'X'			/*Ver 1.2 */
#define FOR_RCV		 'R'			/*Ver 1.4 */
#define FROM_STOCKLST_ENABLED     'A' /* Ver 2.0*/
#define FROM_STOCKLST_DISABLED    'B' /* Ver 2.0*/
#define FOR_EXER   'E'      			/* Ver 1.6 */
#define FOR_ERR_Q  'Q' /*** ver 4.3  ***/

/**** SFO_FUTPOS_BOOK, SFO_OPTPOS_BOOK ****/
#define WITH_UNDERLYING    'U'
#define WITHOUT_UNDERLYING 'W'
#define WITH_UNDERLYING_TAG 'J'

/**** SFO_ASGN_BOOK ****/
#define WITH_DATE_IP    									'W'
#define WITH_DATE_AND_UNDRLYNG_IP    			'U'
#define WITHOUT_DATE_IP_AND_UNDRLYNG    	'I'
#define WITHOUT_DATE_IP_WITH_UNDRLYNG_INP 'O'
#define WITHOUT_DATE_IP 									'D'

/**** SFO_GT_EXT_CNT ****/
#define NFO_ENTTY	1
#define BFO_ENTTY	2

/**** SFO_UPDGEN_INF ****/
#define UPDATE_STATUS		'S'
#define UPDATE_GEN_INFO	'G'

/**** SFO_UPD_CNTRCT, SFO_USR_FVRTS ****/
#define ADD			'+'
#define DELETE	'-'
#define UPDATE_CONTRACT_DOWNLOAD  'M'

/**** SFO_GET_CNTRCT, SFO_WTCH_LST ****/
#define FAVOURITE_CONTRACTS		'F'
#define MOST_ACTIVE_CONTRACT	'M'
#define FOR_QUOTES_ORDERS			'O'
#define FROM_UNDERLYING				'U'
#define CLOUD_ORDERS          'C'                   /**** Ver 3.8 ****/

/**** SFO_UPD_LONG_QT ****/
#define	UPDATE_OI				'O'
#define	UPDATE_PRICE_QT	'P' 

/**** SFO_GET_ENTTY, SFO_GT_EXT_CNT ****/
#define	CONTRACT_TO_MATCH_ID 	'M' 
#define	CONTRACT_TO_NSE_ID  	'N' 
#define	CONTRACT_TO_EBA_ID   	'E' 

/**** SFO_CNT_FOR_ENT ****/
#define	MATCH_ID_TO_CONTRACT 	 'M' 
#define	NSE_ID_TO_CONTRACT  	 'N' 
#define	EBA_ID_TO_CONTRACT   	 'E' 
#define NSE_TOKEN_TO_CONTRACT  'T'

/**** SFO_EXRC_BOOK, SFO_ORDR_BOOK ****/
#define	CONTRACT_IP   								'A' 
#define	CONTRACT_AND_DATE_IP   				'B' 
#define	UNDERLYING_IP   							'C' 
#define	UNDERLYING_AND_DATE_IP   			'D' 
#define	STATUS_PRODUCT_IP   					'E' 
#define	STATUS_PRODUCT_AND_DATE_IP   	'F' 
#define	EXCHNG_ACK_AND_PIPE_ID_IP			'G' 
#define ORDER_RFRNC_IP                'H'     /***  Ver 1.9 ***/
#define ORDER_REF_IP									'I'			/***  Ver 2.1 ***/

/**** SFO_UPD_XCHNGBK ****/
#define	INSERTION_ON_ORDER_MODIFICATION	'M' 
#define	INSERTION_ON_EXREQ_MODIFICATION	'E' 
#define	UPDATION_ON_ORDER_FORWARDING   	'P' 
#define	UPDATION_ON_EXCHANGE_RESPONSE  	'R' 
#define	INSERTION_ON_ORDER_FTE       		'F' 
#define	UPDATION_IF_ORDER_NOT_PLACED   	'N' 
#define	UPDATE_PLACED_STTS             	'U' 
#define	UPDATE_RMS_PRCSD_FLG           	'A' 
#define UPDATION_ON_FUT_CONVERSION      'L'
#define UPDATION_ON_MRKTTOLMT           'G'     /***  Ver 2.5 ***/

/**** SFO_UPD_ORDRBK ****/
#define INSERT_ON_ORDER_PLACEMENT	      'N'
#define UPDATE_ORDER_STATUS				      'S'
#define UPDATE_ORDER_MODIFICATION	      'M'
#define UPDATE_XCHNG_RESPONSE			      'X'
#define UPDATE_TRADE_CONFIRMATION	      'T'
#define UPDATE_XCHNG_RESPONSE_NEW_ORDER 'F'
#define UPDATE_MOD_CNTR                 'C'
#define	UPDATE_STATUS_ACK_TM						'A'
#define	UPDATE_XCHNG_RESP_DTLS					'Z'
#define	UPDATE_XCHNG_RESP_DTLS_CA				'B'
#define	UPDATE_XCHNG_RESP_DTLS_OT				'D'
#define	UPDATE_ORDER_CANCELATION				'L'
#define UPDATE_FUT_CONVERSION           'P'		/** FuturePlus **/

/**** SFO_UPD_EXBK ****/
#define INSERT_ON_NEW_EXERCISE_REQUEST	'N'
#define UPDATE_EXERCISE_REQUEST_STATUS	'S'
#define UPDATE_QTY_STATUS_ON_MODIFICATION	'M'
#define UPDATE_STATUS_ACK_TM	'A'
#define UPDATE_QTY_STATUS_ACKTM_ON_MOD	'B'
#define UPDATE_MOD_CNTR_EXBK 'C'
#define UPDATE_STATUS_ON_XCHNG_RESP 'D'
#define UPD_EXER_STTS_CLS_PRC 'E'

/**** SFO_GET_SEQ ****/
# define GET_XCHNG_SEQ 	'X'
# define GET_ORDER_SEQ 	'O'
# define GET_PLACED_SEQ	'P'
# define UPD_PLACED_SEQ	'U'
# define GET_TRD_SEQ	  'T'
# define GET_SPRDORD_SEQ 'S'
# define GET_ASGNMT_SEQ	 'A'
/** Added for Temp order on 03/06/02 **/
# define GET_TEMPORD_SEQ	 'M'
# define RESET_PLACED_SEQ 'R'

/*** For service SFO_UPD_CNTRCT	***/
#define  UPDATE_GENERAL_INFO 'U'

/*** For service SFO_UPD_QT	***/
#define  UPDATE_SHORT_QT_1    '1'
#define  UPDATE_SHORT_QT_2    '2'
#define  UPDATE_SHORT_QT_3    '3'
#define  UPDATE_CLOSE_PRC			'4'
#define  UPDATE_TRD_EXEC_RNG	'5'  /*** Ver 3.6 ***/

/*** For service SFO_UPD_XSTTS	***/
#define  UPD_PARTICIPANT_STTS  'P'
#define  UPD_BRK_LMT_EXCD      'B'
#define  UPD_NORMAL_MKT_STTS   'N'
#define  UPD_EXER_MKT_STTS     'E'
#define  UPD_EXTND_MKT_STTS    'X'		/*Ver 1.2 */

/*** For service SFO_UPD_INDX_INFO	***/
#define  UPDATE_INDX_QT_1	'I'
#define  UPDATE_INDX_QT_2	'X'


/***	For service SFO_UPD_TRDBK	***/
#define INSERT_TRADE_CONF  'C'
#define UPDATE_TRADE_CONF  'U'

/***	For service SFO_GET_LIMITS	***/
#define	LIMIT_X								'A'
#define LIMIT_X_GID						'B'
#define LIMIT_X_GID_OPTB			'D' /** ver 5.0 **/
#define MAX_UNBLOCK_AMT				'C'

/**** SFO_REF_TO_ORD, SFO_EXRQ_DTLS ****/
#define FOR_VIEW  'V'
#define FOR_UPDATE 'U'
#define FOR_SNDCLNT 'S'
#define FOR_TRDRECO 'T'					/*Ver 1.3 */

/**** SFO_REF_TO_OMD ****/
#define WITH_ORS_MSG_TYP			'M'
#define WITHOUT_ORS_MSG_TYP		'W'

/**** SFO_FUT_ACK ****/
#define	ORDER_EXPIRY	'E'

/**** SFO_EXREQ_ACK ****/
#define	NEW_EXREQ_ACCEPTANCE						'N'
#define	NEW_EXREQ_REJECTION							'J'
#define	MODIFICATION_EXREQ_ACCEPTANCE		'M'
#define	MODIFICATION_EXREQ_REJECTION		'R'
#define	CANCELLATION_EXREQ_ACCEPTANCE		'C'
#define	CANCELLATION_EXREQ_REJECTION		'E'


/**** BAT_HOUSE_KEEP ****/
#define HOUSE_KEEP_ORDER		'O'
#define HOUSE_KEEP_CNTRCT		'C'

/**** BAT_FO_EXP ****/
#define BATCH_EXPIRY							'X'
#define ORDER_SPECIFIC_EXPIRY			'O'
#define UNDRLYNG_EXPRY						'U'
#define GTD_EXPRY									'G'
#define CLSOUT_EXPIRY             'C'     /***  Ver 1.7 ***/

/**** FFO_BRKR_STTS ****/
#define SUSPEND				'S'
#define ACTIVE				'A'	
#define CLOSE_OUT			'C'	
#define RRM_MODE			'R'	               /***  Ver 3.1 ***/

/**** Daily Obligation from Match ****/
#define	MATCH_OPTION_PREMIUM								"M1"
#define	MATCH_STOCK_OPTION_EXERCISE					"M2"
#define	MATCH_INDEX_OPTION_EXERCISE					"M3"
#define	MATCH_STOCK_OPTION_ASSIGNMENT				"M4"
#define	MATCH_INDEX_OPTION_ASSIGNMENT				"M5"
#define	MATCH_EOD_MTM_FUTURES								"M6"
#define	MATCH_BROKERAGE											"M7"
#define	MATCH_CASH_SETTLEMENT_FOR_FRACTIONS	"M8"

/********************* Ver 4.7 ***********************/
/****     Daily Physical Obligation from Match    ****/
#define MATCH_OPTION_PREMIUM_PHY                  "PH1"
#define MATCH_STOCK_OPTION_EXERCISE_PHY           "PH2"
#define MATCH_INDEX_OPTION_EXERCISE_PHY           "PH3"
#define MATCH_STOCK_OPTION_ASSIGNMENT_PHY         "PH4"
#define MATCH_INDEX_OPTION_ASSIGNMENT_PHY         "PH5"
#define MATCH_EOD_MTM_FUTURES_PHY                 "PH6"
#define MATCH_BROKERAGE_PHY                       "PH7"
#define MATCH_CASH_SETTLEMENT_FOR_FRACTIONS_PHY   "PH8"
#define MATCH_DLY_OBLIGATION                      "PH9"
/******************* Ver 4.7 ******************/

/**** Back office *****/
#define UPLD_FOR_MTCH_ACCNT 'M'
#define MTCH_UPLD_FOR_XCHNG 'X'
#define CLS_FOR_INT_ADJ 'A'
#define CLS_FOR_PIO_ADJ 'B'

/**** BAT_FO_IMTM ****/
#define MTM_ALL_PSTN									'A'
#define MTM_FOR_UNDRLYNG			    		'U'
#define MTM_FOR_MATCH_ALL_UNDRLYNG	  'M'
#define MTM_FOR_MATCH_UNDRLYNG			  'S'
#define NORMAL_IMTM                   'N'     /*** Margin statement ***/
#define MTM_MRGN_STMNT                'M'     /*** Margin statement ***/

/**** SFO_GT_SIMTM ****/
#define MARK_ALL_PSTN					'P'				/***	Ver	2.2	***/
#define MARK_FOR_UNDRLYNG		  'F'				/***	Ver	2.2	***/

#define RUN_INTRADAY_MTM			    1
#define CREATE_MTM_REPORT_DATA    2
#define RUN_SPAN_IMTM_IM	1										/**Ver 2.2 for SPAN**/
#define RUN_SPAN_IMTM_PNL	2										/**Ver 2.2 for SPAN**/
#define RUN_SPAN_IMTM_IMPNL	3									/**Ver 2.2 for SPAN**/

/*****  MTM flag value  ***/
#define UNDER_MTM 						'M'
#define NO_OPEN_ORDERS 				'P'
#define NO_OPEN_POSITION     	'X'
#define ORD_ACCPT_FOR_SYSTEM_SQ_OFF     	'A'
#define FOR_REMARKS_ENTRY 						'R'
#define UNDER_PHYDLVRY_MTM            'D'  /** Ver 4.1 **/
#define NO_PHYDLVRY_OPEN_ORDERS 'D'   /** Ver 4.1 **/

/****** SFO_CANSQ_FIMTM  ***/
#define CANCEL_ORDERS					'C'
#define SQUARE_OFF						'S'

/****** SFO_GT_BULKFUT  ***/
#define BULK_FOR_MTCH_ACCNT					'M'
#define BULK_ORDR_FOR_XCHNG					'E'

/****** SFO_FUT_PLC_ORD ****/
#define SYSTEM_PLACE_ORDER   9999
#define JOINT_SQUARE_OFF_ORDER   9998

/****** SFO_UPD_OPT_POS / SFO_UPD_FUT_POS ****/
#define ADD_MARGIN	          	1
#define EOD_MTM	              	2
#define INTRADAY_MTM          	3
#define CONTRACT_SPLIT        	4
#define CLOSE_OUT_DEALS       	5
#define INTRADAY_MTM_REPORT   	6
#define CALCULATE_MARGIN   			7
#define EXER_CONF         			8
#define EXER_RJCT		          	9
#define ASGMNT			          	10
#define CANCEL_AFTER_ASSIGNMENT 11
#define CONV_TO_FUT             12
#define CLOSE_FUTPLUS           13                        /**FuturePlus**/
#define CREATE_FUT              14                        /**FuturePlus**/
#define MRGN_STMNT              15                        /**Margin Statement**/
#define ADHOC_ADD_MRGN_RQST     18                        /***  Ver 1.8 ***/
#define EODMTM_SPLIT            25                        /***  Ver 3.9 ***/
#define EOD_MRGN_REP            26                        /*** Ver 4.5 ***/ 

/****** SFO_QRY_SPDBK ******/
#define ORDREF_TO_SPDREF	'A'
#define ACK_TO_ORD        'B'
#define SPDREF_TO_ORDREF  'C'

/****** SFO_UPD_SPDBK ******/
#define UPDATE_XCHNG_RESPONSE_RC 'A'

/****** SFO_ABT_PLC_ORD ******/
#define CASH 'C'
#define CASH_BUY 1
#define CASH_SELL 2 
/****** BP RELATED ******/
# define	BPID '#'
# define      LEN_DT_MTCH_COUNT		100
/******IMTM Change*******/
static double d_limit_MTM ;

/****** SFO_GT_CS_REC ******/
#define CNTRCT_SPLT                   'S'
#define CNTRCT_CNSLDTN        'C'

/***Ver 1.2 For Extended Market Hours***/
#define NORMAL_MRKT						'N'
#define EXTND_MRKT						'X'

#define CONV_TO_PART_FUT			16                        /**Ver 1.5 PartConvert**/
#define CLOSE_PART_FUTPLUS		17                        /**Ver 1.5 PartConvert**/

/** Limit Negativity for SPAN Ver 2.2 **/

#define REL_TRDMRGN_ON_LIMIT_NEG       "LC5"
#define REL_BLKDLOSS_ON_LIMIT_NEG      "LC6"
#define BLK_SPAN_MRGN_ON_LIMIT_NEG     "LC7"
#define BLK_EXPR_MRGN_ON_LIMIT_NEG     "LC8"
#define BLK_NOTIONAL_LOSS_ON_LIMIT_NEG "LC9"

/*** Ver 2.3 Limit Negativity ***/
#define ON_LIMIT_NEG_ADJ "LC4"

/*** Ver 2.4 Convert to Future PLUS ***/
#define CONV_TO_FUTPLS        19
#define CONV_TO_PART_FUTPLS   20
#define CREATE_FUTPLS         21
#define CLOSE_FUT             22
#define CLOSE_PART_FUT        23
#define ORDER_MARGIN_ADJ_ON_FUTPLUS_CONV      "LB8"
#define RELEASE_OF_ADD_MARGIN_ON_FUTPLUS_CONV "LB9"
#define TRD_MARGIN_ADJ_ON_FUTPLUS_CONV        "LC1"
#define PROFIT_LOSS_ON_COVER_LEG_FUTPLUS_CONV "LC2"
#define BLK_NOTIONAL_LOSS_ON_FUTPLUS_CONV     "LC3"

/*** Ver 3.7 ***/
#define TRD_MARGIN_ADJ_ON_FPSL_CONV        "LF1"
#define TRD_MARGIN_ADJ_ON_OPLS_CONV        "LF2"

/*** Ver 2.6 Convert from Future Plus to Future ***/
#define BLK_NOTIONAL_LOSS_ON_FUT_CONV     "LD1"

/*** Ver 2.7 ***/
#define ON_ROLLOVER_ORDER_PLACEMENT          "LD7"
#define BLOCK_PNL_ON_ROLLOVER_ORDER_PLACEMENT          "LD8"
/** ver 4.0 starts ***/
#define REL_ROMARG_ORD_ACCPT  "RL1"
#define REL_ROPNL_ORD_ACCPT		"RL2"
#define BLK_ROMARG_ORD_ACCPT			 "RL3"
#define BLK_ROPNL_ORD_ACCPT				 "RL4"
#define REL_ROMARG_ORDMOD_ACCPT		 "RL5"
#define REL_ROPNL_ORDMOD_ACCPT		 "RL6"
#define BLK_ROMARG_ORDMOD_ACCPT		 "RL7"
#define BLK_ROPNL_ORDMOD_ACCPT		 "RL8"
#define REL_ROMARG_ORDCAN_ACCPT		 "RC1"
#define REL_ROPNL_ORDCAN_ACCPT		 "RC2"
#define REL_ROMARG_ORDTRD_ACCPT		 "RT1"
#define REL_ROPNL_ORDTRD_ACCPT		 "RT2"
#define ON_ROMOD_REJ   "RMJ"
#define ON_ROORD_REJ   "ROJ"
#define ON_ROCAN_REJ	"RCJ"
#define ON_ROMOD_PL_REJ  "RLM"
#define ON_ROCAN_PL_REJ  "RLC"
#define BLK_ROPNL_ORDMOD_RJCT "BLM"
#define BLK_ROPNL_ORDCAN_RJCT "BLC"
#define BLK_ROMARG_ORDMOD_RJCT "BMM"
#define BLK_ROMARG_ORDCAN_RJCT "BMC"
#define ON_RWS_ORDER_PLACEMENT        "RWS"
#define BLOCK_PNL_ON_RWS_ORDER_PLACEMENT         "RSL" 
/** ver 4.0 ends ***/

/**** Ver 2.8 SPAN -IOC/2L/3L Order Handling ****/

#define REL_TRDMRGN_ON_IOC_2L_3L_CANORD_ACCPT  "LD2"
#define REL_BLKDLOSS_ON_IOC_2L_3L_CANORD_ACCPT "LD3"
#define BLK_SPAN_MRGN_ON_IOC_2L_3L_CANORD_ACCPT "LD4"
#define BLK_EXPR_MRGN_ON_IOC_2L_3L_CANORD_ACPT "LD5"
#define BLK_NOTIONAL_LOSS_ON_IOC_2L_3L_CANORD_ACPT "LD6"

/*** Ver 2.9 SLTP FUTUREPLUS Handling ***/
#define FRESH	'F'
#define COVER	'C'

/*** Ver 3.0 SPAN Future Plus ***/
#define REL_TRDMRGN_ON_CONV_FUT_TO_FUTPLS 		  "LD9"
#define BLK_SPAN_MRGN_ON_CONV_FUT_TO_FUTPLS 	  "LE1"
#define BLK_EXPR_MRGN_ON_CONV_FUT_TO_FUTPLS 	  "LE2"
#define BLK_NOTIONAL_LOSS_ON_CONV_FUT_TO_FUTPLS "LE3"
#define REL_TRDMRGN_ON_CONV_FUTPLS_TO_FUT 		  "LE4"
#define BLK_SPAN_MRGN_ON_CONV_FUTPLS_TO_FUT 	  "LE5"
#define BLK_EXPR_MRGN_ON_CONV_FUTPLS_TO_FUT 	  "LE6"
#define BLK_NOTIONAL_LOSS_ON_CONV_FUTPLS_TO_FUT "LE7"
