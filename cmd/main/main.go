package main

import (
	"DATA_FWD_TAP/internal/initializers"
	"log"
	"sync"
)

func main() {
	// These are Example arguments . Actual ARguments will be provided from Command line.
	args := []string{"main", "arg1", "arg2", "arg3", "P1", "arg5", "arg6", "arg7"}
	serviceName := args[0]
	var mtypewrite, mtypeRead int
	mtypewrite = 1
	mtypeRead = 1

	log.Printf("[%s] Program %s starts", serviceName, args[0])

	// Initialization of MainContainer
	mainContainer := initializers.NewMainContainer(args[0], args[1:], &mtypeRead, &mtypewrite)

	serviceInitManager := &initializers.ServiceInitializationManager{
		MainContainer: mainContainer,
	}

	logger := mainContainer.UtilContainer.LoggerManager.GetLogger()

	// // ClnPackClnt initialization
	// if initResult := serviceInitManager.ClnPackClntInitialization(); initResult != 0 {
	// 	logger.Fatalf("[Fatal: Shutting down due to error in %s: ClnPackClntInitialization failed with result code %d", serviceName, initResult)
	// }

	// LogOnToTap initialization
	var wg sync.WaitGroup // WaitGroup to wait for the Go routines

	// ClnPackClnt initialization using a Go routine
	wg.Add(1)
	go func() {
		defer wg.Done()
		if initResult := serviceInitManager.ClnPackClntInitialization(); initResult != 0 {
			logger.Fatalf("[Fatal: Shutting down due to error in %s: ClnPackClntInitialization failed with result code %d", serviceName, initResult)
		}
	}()

	// ESR function initialization using a Go routine
	wg.Add(1)
	go func() {
		defer wg.Done()
		if initResult := serviceInitManager.ESRInitialization(); initResult != 0 {
			logger.Fatalf("[Fatal: Shutting down due to error in %s: ESRInitialization failed with result code %d", serviceName, initResult)
		}
	}()

	// Wait for both Go routines to finish
	wg.Wait()

	// if initResult := serviceInitManager.ESRInitialization(); initResult != 0 {
	// 	logger.Fatalf("[Fatal: Shutting down due to error in %s: ESRInitialization failed with result code %d", serviceName, initResult)
	// }

	mainContainer.UtilContainer.LoggerManager.LogInfo(serviceName, "Main ended successfully.")
}
