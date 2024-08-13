package common

import (
	"gorm.io/gorm"
)

type DBConfigLoader interface {
	LoadPostgreSQLConfig(serviceName string, fileName string) (*PostgreSQLConfig, error)
}

type DBConnector interface {
	GetDatabaseConnection(serviceName string, cfg PostgreSQLConfig) int
}

type DBAccessor interface {
	GetDB(serviceName string) *gorm.DB
}

type PostgreSQLConfig struct {
	Username string
	Password string
	DBName   string
	Host     string
	Port     int
	SSLMode  string
}
