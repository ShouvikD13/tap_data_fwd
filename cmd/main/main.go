package main

import (
	"DATA_FWD_TAP/config"
	"DATA_FWD_TAP/internal/app"
	"DATA_FWD_TAP/models"
	"log"
)

func main() {
	// args := os.Args
	args := []string{"main", "arg1", "arg2", "arg3", "P1", "arg5", "arg6", "arg7"}
	serviceName := args[0]
	var resultTmp int

	log.Printf("[%s] Program %s starts", serviceName, args[0])

	/* testing Eviroment.go
	configFilePath := filepath.Join("config", "config.ini")

	result := models.InitProcessSpace(configFilePath, "Server") // here
	if result != 0 {
		log.Fatalf("[%s] Failed to initialize process space: %d", serviceName, result)
	}

	// fmt.Println(result)

	tokenValue := models.GetProcessSpaceValue("Host")
	if tokenValue == "" {
		log.Printf("[%s] Token not found", serviceName)
	} else {
		fmt.Printf("[%s] %s\n", serviceName, tokenValue)
	}
	*/
	//----------------------------------------------

	environmentManager := models.NewEnvironmentManager(serviceName, "/mnt/c/Users/devdu/go-workspace/data_fwd_tap/config/EnvConfig.ini")

	environmentManager.LoadIniFile()

	configManager := config.NewConfigManager(serviceName, environmentManager)

	if configManager.LoadPostgreSQLConfig() != 0 {
		log.Fatalf("[%s] Failed to load PostgreSQL configuration: ", serviceName)
	}

	if configManager.GetDatabaseConnection() != 0 {
		log.Fatalf("[%s] Failed to connect to database", serviceName)
	}

	DB := configManager.GetDB()
	if DB == nil {
		log.Fatalf("[%s] Database connection is nil. Failed to get the database instance.", serviceName)
	}

	//=======================================================================================================

	// /*Tuxlib.go Testing

	// txManager := &models.TransactionManager{DbAccessor: cfgManager} // This creates a variable of the structure defined in Tuxlib.go. The field in this structure is an instance of the database. Throughout the transaction, we have to use the same database instance.

	// tranType := txManager.FnBeginTran(serviceName) // transaction begins here. Here we are calling the "FnBeginTran()" of "Tuxlib.go".

	// if tranType == -1 {
	// 	log.Fatalf("[%s] Failed to begin transaction", serviceName)
	// }

	// user := structures.User{
	// 	Username: "new_user",
	// 	Email:    "new_user@example.com",
	// }
	// if err := txManager.Tran.Create(&user).Error; err != nil {
	// 	log.Fatalf("[%s] Failed to insert new user: %v", serviceName, err)
	// }

	// product := structures.Product{
	// 	Name:          "New Product",
	// 	Description:   "Description for New Product",
	// 	Price:         39.99,
	// 	StockQuantity: 200,
	// }
	// if err := txManager.Tran.Create(&product).Error; err != nil {
	// 	log.Fatalf("[%s] Failed to insert new product: %v", serviceName, err)
	// }

	// order := structures.Order{
	// 	UserID:    user.ID,
	// 	OrderDate: time.Now(),
	// 	Amount:    39.99,
	// 	Status:    "Pending",
	// }
	// if err := txManager.Tran.Create(&order).Error; err != nil {
	// 	log.Fatalf("[%s] Failed to insert new order: %v", serviceName, err)
	// }

	// orderItem := structures.OrderItem{
	// 	OrderID:   order.ID,
	// 	ProductID: product.ID,
	// 	Quantity:  1,
	// 	Price:     39.99,
	// }
	// if err := txManager.Tran.Create(&orderItem).Error; err != nil {
	// 	log.Fatalf("[%s] Failed to insert new order item: %v", serviceName, err)
	// }

	// // Sample Read Operation
	// var fetchedUser structures.User
	// if err := txManager.Tran.First(&fetchedUser, "username = ?", "new_user").Error; err != nil {
	// 	log.Fatalf("[%s] Failed to fetch user: %v", serviceName, err)
	// }
	// fmt.Printf("[%s] Fetched User: %+v\n", serviceName, fetchedUser)

	// // Sample Update Operation
	// if err := txManager.Tran.Model(&fetchedUser).Update("Email", "updated_user@example.com").Error; err != nil {
	// 	log.Fatalf("[%s] Failed to update user email: %v", serviceName, err)
	// }

	// // Sample Delete Operation
	// if err := txManager.Tran.Delete(&fetchedUser).Error; err != nil {
	// 	log.Fatalf("[%s] Failed to delete user: %v", serviceName, err)
	// }

	// if txManager.FnCommitTran(serviceName, tranType) != 0 {
	// 	log.Fatalf("[%s] Failed to commit transaction", serviceName)
	// }

	// fmt.Printf("[%s] Transaction successfully committed\n", serviceName)
	// */
	//=======================================================================================================

	//=======================================================================================================
	// // Testing "cln_pack_clnt.go"

	VarClnPack := &app.ClnPackClntManager{
		Enviroment_manager: environmentManager,
		Config_manager:     configManager,
	}
	resultTmp = VarClnPack.Fn_bat_init(args[1:], DB)

	if resultTmp != 0 {
		log.Printf("[%s] Fn_bat_init failed with result code: %d", serviceName, resultTmp)
		log.Fatal("Shutting down due to error") // log.Fatal logs the message and exits with status 1
	}

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

	log.Printf("[%s] Main Ended Here...", serviceName)

}
