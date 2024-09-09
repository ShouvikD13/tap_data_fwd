package util

import (
	"io"
	"os"
	"path/filepath"
	"time"

	"github.com/sirupsen/logrus"
	"gopkg.in/natefinch/lumberjack.v2"
)

type LoggerManager struct {
	log *logrus.Logger
}

func (LM *LoggerManager) InitLogger() {
	LM.log = logrus.New()

	// Set up log directory
	logDir := filepath.Join("/mnt/c/Users/devdu/go-workspace/data_fwd_tap", "logs")

	// Ensure the logs directory exists
	if _, err := os.Stat(logDir); os.IsNotExist(err) {
		err := os.MkdirAll(logDir, os.ModePerm)
		if err != nil {
			LM.log.Fatalf("Failed to create log directory: %v", err)
		}
	}

	logFilename := filepath.Join(logDir, "ULOG."+time.Now().Format("2006-01-02")+".log")

	// Configure the lumberjack logger for log rotation
	lumberjackLogger := &lumberjack.Logger{
		Filename:   logFilename,
		MaxSize:    50,   // megabytes
		MaxBackups: 5,    // number of backups to keep
		MaxAge:     30,   // days
		Compress:   true, // compress old log files
	}

	// Log to file and stdout depending on environment
	LM.log.SetOutput(LM.getLogOutput(lumberjackLogger))

	// Set custom log formatter for professional logs
	LM.log.SetFormatter(&logrus.TextFormatter{
		TimestampFormat: "2006-01-02 15:04:05",
		FullTimestamp:   true,
		ForceColors:     true,  // for colorful output on terminals
		DisableColors:   false, // change this if running in production (optional)
	})

	// Set the log level based on the environment
	LM.log.SetLevel(LM.getLogLevel())
}

// GetLogger returns the logrus logger instance
func (LM *LoggerManager) GetLogger() *logrus.Logger {
	return LM.log
}

// getLogOutput determines whether to log to a file, stdout, or both
func (LM *LoggerManager) getLogOutput(lumberjackLogger *lumberjack.Logger) io.Writer {
	if os.Getenv("ENV") == "production" {
		// Only log to file in production
		return lumberjackLogger
	}
	// Log to both file and stdout in non-production environments
	return io.MultiWriter(lumberjackLogger, os.Stdout)
}

// getLogLevel sets the logging level based on the environment
func (LM *LoggerManager) getLogLevel() logrus.Level {
	switch os.Getenv("ENV") {
	case "production":
		return logrus.WarnLevel
	case "staging":
		return logrus.InfoLevel
	default:
		return logrus.DebugLevel
	}
}

// Professional structured logging
func (LM *LoggerManager) LogInfo(serviceName string, msg string, args ...interface{}) {
	LM.log.Infof("[%s] "+msg, append([]interface{}{serviceName}, args...)...)
}

func (LM *LoggerManager) LogError(serviceName string, msg string, args ...interface{}) {
	LM.log.Errorf("[%s] "+msg, append([]interface{}{serviceName}, args...)...)
}
