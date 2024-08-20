package config

import (
	"DATA_FWD_TAP/common"
	"DATA_FWD_TAP/models"

	"fmt"
	"log"

	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

/**********************************************************************************/
/*                                                                                 */
/*  Description       : This package defines a configuration structure for         */
/*                      connecting to a PostgreSQL database. The `PostgreSQLConfig`*/
/*                      struct holds essential details required for establishing   */
/*                      a connection, including username, password, database name, */
/*                      host, port, and SSL mode.                                  */
/*                                                                                 */
/*                      The `GetDatabaseConnection` function formats these values  */
/*                      into a Data Source Name (DSN) string, which is used to     */
/*                      open a connection to the PostgreSQL database using GORM.   */
/*                                                                                 */
/*  Functions         :                                                            */
/*                        - LoadPostgreSQLConfig: Reads the configuration from     */
/*                          an INI file and returns a `PostgreSQLConfig` instance. */
/*                                                                                 */
/*                        - GetDatabaseConnection: Constructs a DSN string from    */
/*                          the provided `PostgreSQLConfig` struct, establishes a  */
/*                          connection to the PostgreSQL database using GORM, and  */
/*                          returns an int status code.                            */
/*                                                                                 */
/*                        - GetDB: Returns the currently established database      */
/*                          connection as a `*gorm.DB` instance.                   */
/*                                                                                 */
/**********************************************************************************/

// ConfigManager implements the DBConfigLoader, DBConnector, and DBAccessor interfaces.
type ConfigManager struct {
	database struct {
		Db *gorm.DB // GORM database connection instance
	}
	environmentManager *models.EnvironmentManager
	resultTmp          int
	FileName           string
	cfg                *common.PostgreSQLConfig
	serviceName        string
}

func NewConfigManager(serviceName, fileName string, envManager *models.EnvironmentManager) *ConfigManager {
	return &ConfigManager{
		environmentManager: envManager,
		FileName:           fileName,
		serviceName:        serviceName,
	}
}

// LoadPostgreSQLConfig reads the configuration from an INI file and returns a PostgreSQLConfig instance.
// It takes the file name of the INI file as an argument.
func (cm *ConfigManager) LoadPostgreSQLConfig() int {

	cm.resultTmp = cm.environmentManager.InitProcessSpace("database")
	if cm.resultTmp != 0 {
		log.Printf("[%s] Failed to read config file: %v", cm.serviceName, cm.resultTmp)
		return -1
	}

	cm.cfg = &common.PostgreSQLConfig{
		Host:     cm.environmentManager.GetProcessSpaceValue("host"),
		Port:     cm.environmentManager.GetProcessSpaceValueAsInt("port"),
		Username: cm.environmentManager.GetProcessSpaceValue("username"),
		Password: cm.environmentManager.GetProcessSpaceValue("password"),
		DBName:   cm.environmentManager.GetProcessSpaceValue("dbname"),
		SSLMode:  cm.environmentManager.GetProcessSpaceValue("sslmode"),
	}

	return 0
}

// GetDatabaseConnection constructs a DSN string from the provided PostgreSQLConfig struct,
// establishes a connection to the PostgreSQL database using GORM, and returns an int status code.
// It returns 0 on success and -1 on failure.
func (cm *ConfigManager) GetDatabaseConnection() int {

	log.Printf("[%s] Setting up the database connection", cm.serviceName)
	dsn := fmt.Sprintf("user=%s password=%s dbname=%s host=%s port=%d sslmode=%s",
		cm.cfg.Username, cm.cfg.Password, cm.cfg.DBName, cm.cfg.Host, cm.cfg.Port, cm.cfg.SSLMode)

	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		log.Printf("[%s] failed to connect database: %v", cm.serviceName, err)
		return -1
	}
	cm.database.Db = db
	return 0
}

// GetDB returns the currently established database connection as a *gorm.DB instance.

func (cm *ConfigManager) GetDB() *gorm.DB {
	log.Printf("[%s] GetDb is called and Returning the Database instance ", cm.serviceName)
	return cm.database.Db // Return the GORM database connection instance
}
