package util

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/sirupsen/logrus"
	"gopkg.in/natefinch/lumberjack.v2"
)

type LoggerManager struct {
	log        *logrus.Logger
	envManager *EnvironmentManager
}

func (LM *LoggerManager) InitLogger(envManager *EnvironmentManager) {
	LM.log = logrus.New()
	LM.envManager = envManager

	logDir := filepath.Join("/home/devasheesh/Desktop/go-workspace/data_fwd_tap", "logs")

	if _, err := os.Stat(logDir); os.IsNotExist(err) {
		err := os.MkdirAll(logDir, os.ModePerm)
		if err != nil {
			LM.log.Fatalf("Failed to create log directory: %v", err)
		}
	}

	logFilename := filepath.Join(logDir, "ULOG."+time.Now().Format("2006-01-02")+".log")

	lumberjackLogger := &lumberjack.Logger{
		Filename:   logFilename,
		MaxSize:    50,
		MaxBackups: 5,
		MaxAge:     30,
		Compress:   true,
	}

	logOutput := io.MultiWriter(lumberjackLogger, os.Stdout)
	LM.log.SetOutput(logOutput)

	LM.log.SetFormatter(&CustomFormatter{})
	LM.log.SetReportCaller(false)

	LM.log.SetLevel(logrus.InfoLevel)
}

func (LM *LoggerManager) GetLogger() *logrus.Logger {
	return LM.log
}

func (LM *LoggerManager) LogInfo(serviceName string, msg string, args ...interface{}) {
	LM.log.Infof("[%s] "+msg, append([]interface{}{serviceName}, args...)...)
}

func (LM *LoggerManager) LogError(serviceName string, msg string, args ...interface{}) {
	LM.log.Errorf("[%s] "+msg, append([]interface{}{serviceName}, args...)...)
}

/*--------------------------------------- Below is Custome Formater ---------------------------------------*/
type CustomFormatter struct{}

// Format formats the log entry according to the desired format
func (f *CustomFormatter) Format(entry *logrus.Entry) ([]byte, error) {
	// Format the timestamp
	timestamp := entry.Time.Format("2006-01-02 15:04:05")

	// Convert the log level to uppercase
	level := strings.ToUpper(entry.Level.String())

	// Construct the final log message
	// Expected format: LEVEL [timestamp] [serviceName] [messageName] message
	logMessage := fmt.Sprintf("%s [%s] %s\n",
		level,
		timestamp,
		entry.Message,
	)

	return []byte(logMessage), nil
}
