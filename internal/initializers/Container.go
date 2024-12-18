package initializers

import (
	"DATA_FWD_TAP/internal/database"
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"DATA_FWD_TAP/util/MessageQueue"
	"DATA_FWD_TAP/util/OrderConversion"
	socket "DATA_FWD_TAP/util/TapSocket"
	typeconversionutil "DATA_FWD_TAP/util/TypeConversionUtil"
	"database/sql"
	"net"

	"gorm.io/gorm"
)

type UtilContainer struct {
	ServiceName               string
	OrderConversionManager    *OrderConversion.OrderConversionManager
	EnvironmentManager        *util.EnvironmentManager
	MessageQueueManager       *MessageQueue.MessageQueueManager
	MaxPackVal                int
	ConfigManager             *database.ConfigManager
	LoggerManager             *util.LoggerManager
	TypeConversionUtilManager *typeconversionutil.TypeConversionUtilManager
	TransactionManager        *util.TransactionManager
	PasswordUtilManager       *util.PasswordUtilManger
	SocketManager             *socket.SocketManager
	DB                        *gorm.DB
	InitialQId                *int
	GlobalQId                 *int
	IP                        string
	PORT                      string
	ActualResponseTrigger     *chan int
	ErrorResponseTrigger      *chan int
}

type ClientContainer struct {
	Xchngbook    *models.Vw_xchngbook
	Orderbook    *models.Vw_orderbook
	Contract     *models.Vw_contract
	NseContract  *models.Vw_nse_cntrct
	PipeMaster   *models.St_opm_pipe_mstr
	OeReqres     *models.St_oe_reqres
	ExchMsg      *models.St_exch_msg
	NetHdr       *models.St_net_hdr
	QPacket      *models.St_req_q_data
	IntHeader    *models.St_int_header
	ContractDesc *models.St_contract_desc
	OrderFlag    *models.St_order_flags
}

type ClientGlobalValueContainer struct {
	InitialQueueId *int
	GlobalQueueId  *int
	CPanNo         string
	CLastActRef    string
	CEspID         string
	CAlgoID        string
	CSourceFlg     string
	CPrgmFlg       string
	Args           []string
}

type LogOnContainer struct {
	StSignOnReq               *models.St_sign_on_req
	StReqQData                *models.St_req_q_data_Log_On
	StExchMsgLogOn            *models.St_exch_msg_Log_On
	IntHeader                 *models.St_int_header
	StNetHdr                  *models.St_net_hdr
	StBrokerEligibilityPerMkt *models.St_broker_eligibility_per_mkt
}

type LogOnGlobalValueContainer struct {
	CPipeID           string
	UserID            int32
	MaxPackVal        int
	OpmLoginStatus    int
	OpmUserID         int32
	OpmExistingPasswd string
	OpmNewPasswd      sql.NullString
	OpmLstPswdChgDt   string
	OpmXchngCd        string
	OpmTrdrID         string
	OpmBrnchID        int64
	ExgBrkrID         string
	ExgNxtTrdDate     string
	ExgBrkrName       string
	ExgBrkrStts       string
	Args              []string
	InitialQueueId    *int
	GlobalQueueId     *int
}

type LogOffContainer struct {
	IntHeader       *models.St_int_header
	StNetHdr        *models.St_net_hdr
	StExchMsgLogOff *models.St_exch_msg_Log_Off
	StReqQData      *models.St_req_q_data_Log_Off
}

type LogOffGlobalValueContainer struct {
	OpmXchngCd     string
	OpmTrdrID      string
	ExgNxtTrdDate  string
	C_pipe_id      string
	Args           []string
	InitialQueueId *int
	GlobalQueueId  *int
}

type ESRContainer struct {
	Req_q_data                          *models.St_req_q_data
	St_exch_msg                         *models.St_exch_msg
	St_int_header                       *models.St_int_header // Added
	St_net_hdr                          *models.St_net_hdr
	St_oe_reqres                        *models.St_oe_reqres
	St_sign_on_req                      *models.St_sign_on_req
	St_sign_on_res                      *models.St_sign_on_res      // Added
	St_Error_Response                   *models.St_Error_Response   // Added
	StSystemInfoData                    *models.StSystemInfoData    // Added
	StUpdateLocalDBData                 *models.StUpdateLocalDBData // Added
	St_exch_msg_Log_on                  *models.St_exch_msg_Log_On
	St_exch_msg_resp                    *models.St_exch_msg_resp
	St_system_info_req                  *models.St_system_info_req
	St_exch_msg_system_info_Req         *models.St_exch_msg_system_info_Req         // Added
	St_req_q_data_system_info_Req       *models.St_req_q_data_system_info_Req       // Added
	StUpdateLocalDatabase               *models.StUpdateLocalDatabase               // Added
	St_Exch_Msg_UpdateLocalDatabase     *models.St_Exch_Msg_UpdateLocalDatabase     // Added
	St_req_q_data_StUpdateLocalDatabase *models.St_req_q_data_StUpdateLocalDatabase // Added
}

type ESRGlobalValueContainer struct {
	Max_Pack_Val    int
	InitialQueueId  *int
	GlobalQueueId   *int
	ResponseTrigger *int
	SocConnection   *net.Conn
	IP              string
	Port            string
	AutoReconnect   *bool
	Args            []string
}
