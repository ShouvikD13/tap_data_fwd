package main

import (
	foexgconlib "DATA_FWD_TAP/internal/app/esr/fo_exg_con_lib"
	"DATA_FWD_TAP/internal/database"
	"DATA_FWD_TAP/util"
	typeconversionutil "DATA_FWD_TAP/util/TypeConversionUtil"
	"log"
)

func main() {
	// args := os.Args
	args := []string{"main", "arg1", "arg2", "arg3", "P1", "arg5", "arg6", "arg7"}
	serviceName := args[0]
	var mtype int
	mtype = 1

	var resultTmp int

	log.Printf("[%s] Program %s starts", serviceName, args[0])

	/* testing Eviroment.go
	configFilePath := filepath.Join("config", "config.ini")

	result := util.InitProcessSpace(configFilePath, "Server") // here
	if result != 0 {
		log.Fatalf("[%s] Failed to initialize process space: %d", serviceName, result)
	}

	// fmt.Println(result)

	tokenValue := util.GetProcessSpaceValue("Host")
	if tokenValue == "" {
		log.Printf("[%s] Token not found", serviceName)
	} else {
		fmt.Printf("[%s] %s\n", serviceName, tokenValue)
	}
	*/
	//----------------------------------------------

	environmentManager := util.NewEnvironmentManager(serviceName, "/mnt/c/Users/devdu/go-workspace/data_fwd_tap/internal/config/env_config.ini")

	environmentManager.LoadIniFile()

	//Logger file

	loggerManager := &util.LoggerManager{}
	loggerManager.InitLogger(environmentManager)

	// Get the logger instance
	logger := loggerManager.GetLogger()

	loggerManager.LogInfo(serviceName, "Program %s starts", args[0])

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

	//=======================================================================================================

	//=======================================================================================================
	//
	TCUM := &typeconversionutil.TypeConversionUtilManager{
		LoggerManager: loggerManager,
	}

	// Testing "cln_pack_clnt.go"

	// VarClnPack := &pack_clnt.ClnPackClntManager{
	// 	Enviroment_manager: environmentManager,
	// 	Config_manager:     configManager,
	// 	LoggerManager:      loggerManager,
	// 	TCUM:               TCUM,
	// 	Mtype:              &mtype,
	// }
	// resultTmp = VarClnPack.Fn_bat_init(args[1:], DB)

	// if resultTmp != 0 {
	// 	loggerManager.LogInfo(serviceName, " Fn_bat_init failed with result code: %d", resultTmp)
	// 	logger.Fatalf("[Fatal: Shutting down due to error in %s: result code %d", serviceName, resultTmp) // log.Fatal logs the message and exits with status 1
	// }

	//=======================================================================================================

	//=======================================================================================================
	// Test ESRManger

	// log.Println("Starting ESR Service")

	// VarESR := &app.ESRManger{
	// 	ServiceName: serviceName,
	// 	ENVM:        environmentManager,
	// 	// Initialize other fields if necessary
	// }

	// // Start the ESR client (similarly to Fn_bat_init in ClnPackClntManager)
	// VarESR.ClnEsrClnt()

	// log.Println("ESR Service Ended Successfully")

	//=======================================================================================================

	//=======================================================================================================

	VarClnPack := &foexgconlib.LogOnToTapManager{
		Enviroment_manager: environmentManager,
		Config_manager:     configManager,
		LoggerManager:      loggerManager,
		TCUM:               TCUM,
		Mtype:              &mtype,
	}
	resultTmp = VarClnPack.Fn_bat_init(args[1:], DB)

	loggerManager.LogInfo(serviceName, " Main Ended Here...")

}
