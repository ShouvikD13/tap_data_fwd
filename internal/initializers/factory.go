package initializers

import (
	"DATA_FWD_TAP/internal/database"
	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"
	"DATA_FWD_TAP/util/MessageQueue"
	"DATA_FWD_TAP/util/OrderConversion"
	socket "DATA_FWD_TAP/util/TapSocket"
	typeconversionutil "DATA_FWD_TAP/util/TypeConversionUtil"
	"log"
	"strconv"
)

func NewMainContainer(serviceName string, args []string, mTypeRead *int, mTypeWrite *int, InitialQId *int, GlobalQId *int) *MainContainer {

	log.Printf("[%s] Program %s starts", serviceName, args[0])

	utilContainer := NewUtilContainer(serviceName, args, mTypeRead, mTypeWrite, InitialQId, GlobalQId)
	clientContainer := NewClientContainer()
	logOnContainer := NewLogOnContainer()
	logOffContainer := NewLogOffContainer()
	clientGlobalValueContainer := NewClientGlobalValueContainer(args)
	logOnGlobalValueContainer := NewLogOnGlobalValueContainer(args)
	logOffGlobalValueContainer := NewLogOffGlobalValueContainer(args)
	ESRGlobalValueContainer := NewESRGlobalValueContainer()
	ESRContainer := NewESRContainer()

	return &MainContainer{
		UtilContainer:              utilContainer,
		ClientContainer:            clientContainer,
		LogOnContainer:             logOnContainer,
		LogOffContainer:            logOffContainer,
		ClientGlobalValueContainer: clientGlobalValueContainer,
		LogOnGlobalValueContainer:  logOnGlobalValueContainer,
		LogOffGlobalValueContainer: logOffGlobalValueContainer,
		ESRContainer:               ESRContainer,
		ESRGlobalValueContainer:    ESRGlobalValueContainer,
	}
}

func NewUtilContainer(serviceName string, args []string, mTypeRead *int, mTypeWrite *int, InitialQId *int, GlobalQId *int) *UtilContainer {

	log.Printf("[%s] Program %s starts", serviceName, args[0])

	/*
	 Step 1: Setting up the Environment Manager, which is responsible for reading the configuration file.
	 The Environment Manager requires two inputs:
	 1. Service Name
	 2. Path to the configuration file.
	*/

	environmentManager := util.NewEnvironmentManager(serviceName, "/home/devasheesh/Desktop/go-workspace/data_fwd_tap/internal/config/env_config.ini")

	environmentManager.LoadIniFile()

	/*
	 Step 2: Initializing the LoggerManager, which handles logging to both the terminal and log files.
	*/

	loggerManager := &util.LoggerManager{}
	loggerManager.InitLogger(environmentManager)

	// Get the logger instance
	logger := loggerManager.GetLogger()

	loggerManager.LogInfo(serviceName, "Program %s starts", args[0])

	/*
		Step 3: Here we are setting up the configmanager Which is used for DataBase operations.
	*/

	configManager := database.NewConfigManager(serviceName, environmentManager, loggerManager)

	if configManager.LoadPostgreSQLConfig() != 0 {
		loggerManager.LogError(serviceName, "Failed to load PostgreSQL configuration")
		logger.Fatalf("[%s] Failed to load PostgreSQL configuration", serviceName)
	}

	if configManager.GetDatabaseConnection() != 0 {
		loggerManager.LogError(serviceName, "Failed to connect to the database")
		logger.Fatalf("[%s] Failed to connect to database", serviceName)
	}

	DB := configManager.GetDB()
	if DB == nil {
		loggerManager.LogError(serviceName, "Database connection is nil. Failed to get the database instance.")
		logger.Fatalf("[%s] Database connection is nil.", serviceName)
	}
	/*
	 Step 4: Initializing the TransactionManager, which handles transaction operations for update and insert queries.
	*/
	transactionManager := util.NewTransactionManager(serviceName, configManager, loggerManager)

	/*
		Step 5: Initializing the TypeConversionUtilManager, which is generic Manager used for various types of `type conversions
	*/
	typeConversionManager := typeconversionutil.NewTypeConversionUtilManager(loggerManager, serviceName)

	/*
		Step 6: Initializing the PasswordUtilManager, which Used in LogOn service to hanhandle the password conversion
	*/
	passwordManager := util.NewPasswordUtilManager(loggerManager)

	/*
	 Step 7: Initializing the MessageQueueManager, which handles system queue operations (create, write, read, flush, destroy).
	*/

	messageQueueManager := MessageQueue.NewMessageQueueManager(serviceName, loggerManager)

	//this value also we are reding from configuration file "I have set the temp value in the configuration file for now"

	maxPackValStr := environmentManager.GetProcessSpaceValue("PackingLimit", "PACK_VAL")
	var maxPackVal int
	if maxPackValStr == "" {
		loggerManager.LogError(serviceName, " [Factory] [Error: 'PACK_VAL' not found in the configuration under 'PackingLimit'")
	} else {
		maxPackVal, err := strconv.Atoi(maxPackValStr)
		if err != nil {
			loggerManager.LogError(serviceName, " [Factory] [Error: Failed to convert 'PACK_VAL' '%s' to integer: %v", maxPackValStr, err)
			maxPackVal = 0
		} else {
			loggerManager.LogInfo(serviceName, " [Factory] Fetched and converted 'PACK_VAL' from configuration: %d", maxPackVal)
		}

	}

	/*
	 Step 8: Initializing the SocketManager, which manages operations on the TAP socket (creation, write, read).
	*/

	Ip := environmentManager.GetProcessSpaceValue("server", "ip")
	Port := environmentManager.GetProcessSpaceValue("server", "port")

	socketManager := socket.NewSocketManager(loggerManager, serviceName, typeConversionManager)
	Socket, err := socketManager.ConnectToTAP(Ip, Port)

	if err != nil {
		loggerManager.LogError(serviceName, "[NewUtilContainer] Initial connection to TAP failed: %v", err)
		Socket, err = socketManager.AutoReconnection(Ip, Port)
		if err != nil {
			loggerManager.LogError(serviceName, "[NewUtilContainer] Auto reconnection failed: %v", err)
			// If reconnection fails, the program will terminate.
		} else {
			socketManager.SocConnection = Socket
			loggerManager.LogInfo(serviceName, "[NewUtilContainer] Successfully reconnected to TAP.")
		}
	} else {
		socketManager.SocConnection = Socket
		loggerManager.LogInfo(serviceName, "[NewUtilContainer] Connected to TAP successfully.")
	}
	//

	return &UtilContainer{
		ServiceName:               serviceName,
		OrderConversionManager:    &OrderConversion.OrderConversionManager{},
		EnvironmentManager:        environmentManager,
		MaxPackVal:                maxPackVal,
		ConfigManager:             configManager,
		LoggerManager:             loggerManager,
		TypeConversionUtilManager: typeConversionManager,
		TransactionManager:        transactionManager,
		PasswordUtilManager:       passwordManager,
		MessageQueueManager:       messageQueueManager,
		SocketManager:             socketManager,
		DB:                        DB,
		InitialQId:                InitialQId,
		GlobalQId:                 GlobalQId,
	}

}

func NewClientContainer() *ClientContainer {
	return &ClientContainer{
		Xchngbook:    &models.Vw_xchngbook{},
		Orderbook:    &models.Vw_orderbook{},
		Contract:     &models.Vw_contract{},
		NseContract:  &models.Vw_nse_cntrct{},
		PipeMaster:   &models.St_opm_pipe_mstr{},
		OeReqres:     &models.St_oe_reqres{},
		ExchMsg:      &models.St_exch_msg{},
		NetHdr:       &models.St_net_hdr{},
		QPacket:      &models.St_req_q_data{},
		IntHeader:    &models.St_int_header{},
		ContractDesc: &models.St_contract_desc{},
		OrderFlag:    &models.St_order_flags{},
	}
}

func NewClientGlobalValueContainer(args []string) *ClientGlobalValueContainer {

	return &ClientGlobalValueContainer{
		Args: args,
	}
}

func NewLogOnContainer() *LogOnContainer {
	return &LogOnContainer{
		StSignOnReq:               &models.St_sign_on_req{},
		StReqQData:                &models.St_req_q_data_Log_On{},
		StExchMsgLogOn:            &models.St_exch_msg_Log_On{},
		IntHeader:                 &models.St_int_header{},
		StNetHdr:                  &models.St_net_hdr{},
		StBrokerEligibilityPerMkt: &models.St_broker_eligibility_per_mkt{},
	}
}

func NewLogOnGlobalValueContainer(args []string) *LogOnGlobalValueContainer {
	return &LogOnGlobalValueContainer{
		Args: args,
	}
}

func NewLogOffContainer() *LogOffContainer {
	return &LogOffContainer{
		IntHeader:       &models.St_int_header{},
		StNetHdr:        &models.St_net_hdr{},
		StExchMsgLogOff: &models.St_exch_msg_Log_Off{},
		StReqQData:      &models.St_req_q_data_Log_Off{},
	}
}

func NewLogOffGlobalValueContainer(args []string) *LogOffGlobalValueContainer {
	return &LogOffGlobalValueContainer{
		Args: args,
	}
}

func NewESRContainer() *ESRContainer {
	return &ESRContainer{
		Req_q_data:         &models.St_req_q_data{},
		St_exch_msg:        &models.St_exch_msg{},
		St_net_hdr:         &models.St_net_hdr{},
		St_oe_reqres:       &models.St_oe_reqres{},
		St_sign_on_req:     &models.St_sign_on_req{},
		St_exch_msg_Log_on: &models.St_exch_msg_Log_On{},
		St_exch_msg_resp:   &models.St_exch_msg_resp{},
	}
}

func NewESRGlobalValueContainer() *ESRGlobalValueContainer {

	return &ESRGlobalValueContainer{}
}
