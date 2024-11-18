package main

import (
	"DATA_FWD_TAP/internal/initializers"
	"DATA_FWD_TAP/util"
	"log"
)

func main() {
	// These are Example arguments . Actual ARguments will be provided from Command line.
	args := []string{"main", "arg1", "arg2", "arg3", "P1", "arg5", "arg6", "arg7"}
	serviceName := args[0]
	var mtypewrite, mtypeRead, InitialQId, GlobalQId int
	mtypewrite = 1
	mtypeRead = 1

	InitialQId = util.INITIAL_QUEUE_ID

	log.Printf("[%s] Program %s starts", serviceName, args[0])

	mainContainer := initializers.NewMainContainer(args[0], args[1:], &mtypeRead, &mtypewrite, &InitialQId, &GlobalQId)

	serviceInitManager := &initializers.ServiceInitializationManager{
		MainContainer: mainContainer,
	}

	logger := mainContainer.UtilContainer.LoggerManager.GetLogger()

	// Normal Order Packing
	if initResult := serviceInitManager.ClnPackClntInitialization(); initResult != 0 {
		logger.Fatalf("[Fatal: Shutting down due to error in %s: ClnPackClntInitialization failed with result code %d", serviceName, initResult)
	}

	// LogOn Packing
	if initResult := serviceInitManager.LogOnToTapInitialization(); initResult != 0 {
		logger.Fatalf("Fatal: Shutting down due to error in %s: LogOnToTapInitialization failed with result code %d", serviceName, initResult)
	}

	//LOgOff Packing
	if initResult := serviceInitManager.LogOffFromTapInitialization(); initResult != 0 {
		logger.Fatalf("Fatal: Shutting down due to error in %s: LogOffFromTapInitialization failed with result code %d", serviceName, initResult)
	}

	mainContainer.UtilContainer.LoggerManager.LogInfo(serviceName, "Main ended successfully.")
}
