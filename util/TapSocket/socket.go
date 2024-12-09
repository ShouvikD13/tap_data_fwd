package socket

import (
	"DATA_FWD_TAP/util"
	typeconversionutil "DATA_FWD_TAP/util/TypeConversionUtil"
	"fmt"
	"net"
	"os"
	"sync"
)

const (
	DefaultBufferSize = 400 // i have create this buffer because maximum size of all structure is 400
)

type SocketManager struct {
	LM            *util.LoggerManager
	SocConnection net.Conn
	ServiceName   string
	mu            sync.Mutex
	TCUM          *typeconversionutil.TypeConversionUtilManager
}

func NewSocketManager(lm *util.LoggerManager, serviceName string, tcum *typeconversionutil.TypeConversionUtilManager) *SocketManager {
	return &SocketManager{
		LM:          lm,
		ServiceName: serviceName,
		TCUM:        tcum,
	}
}

func (sm *SocketManager) ConnectToTAP(ip, port string) (net.Conn, error) {
	address := fmt.Sprintf("%s:%s", ip, port)
	connection, err := net.Dial("tcp", address)
	if err != nil {
		sm.LM.LogError(sm.ServiceName, "Connect: Failed to connect to socket %s: %v", address, err)
		return nil, fmt.Errorf("failed to connect to socket %s: %w", address, err)
	}

	sm.LM.LogInfo(sm.ServiceName, "Connect: Successfully connected to socket %s.", address)
	return connection, nil
}

func (sm *SocketManager) WriteOnTapSocket(buffer []byte) error {
	sm.LM.LogInfo(sm.ServiceName, "Eneterd in 'WriteOnTapSocket' ")
	// sm.mu.Lock()
	// sm.LM.LogInfo(sm.ServiceName, "Eneterd in 'WriteOnTapSocket' ")
	// defer sm.mu.Unlock()
	sm.LM.LogInfo(sm.ServiceName, "Eneterd in 'WriteOnTapSocket' ")

	net_header, message, err := sm.TCUM.ExtractNetHdrAndMessage(buffer)
	if err != nil {
		sm.LM.LogError(sm.ServiceName, "Error extracting net header and message: %v", err)
		return fmt.Errorf("error extracting net header and message: %w", err)
	}

	if sm.SocConnection == nil {
		sm.LM.LogError(sm.ServiceName, "Write: Socket connection is not established.")
		return fmt.Errorf("socket connection is not established")
	}

	if len(net_header) != 22 {
		sm.LM.LogError(sm.ServiceName, "Invalid net_header size, expected 22 bytes, got %d", len(net_header))
		return fmt.Errorf("invalid net_header size, expected 22 bytes, got %d", len(net_header))
	}

	n, err := sm.SocConnection.Write(net_header)
	if err != nil {
		sm.LM.LogError(sm.ServiceName, "Write: Error writing net_header to socket: %v", err)
		return fmt.Errorf("error writing net_header to socket: %w", err)
	}
	sm.LM.LogInfo(sm.ServiceName, "Write: Successfully wrote %d bytes of net_header to socket.", n)

	// Step 2: Send the actual message
	n, err = sm.SocConnection.Write(message)
	if err != nil {
		sm.LM.LogError(sm.ServiceName, "Write: Error writing message to socket: %v", err)
		return fmt.Errorf("error writing message to socket: %w", err)
	}
	sm.LM.LogInfo(sm.ServiceName, "Write: Successfully wrote %d bytes of message to socket.", n)

	return nil
}

func (sm *SocketManager) ReadFromTapSocket(bufferSize int16) ([]byte, error) {
	sm.mu.Lock()
	defer sm.mu.Unlock()

	if sm.SocConnection == nil {
		sm.LM.LogError(sm.ServiceName, "[ReadFromTapSocket] Socket connection is not established.")
		return nil, fmt.Errorf("socket connection is not established")
	}

	buffer := make([]byte, bufferSize) // on this line the error is occuring 'panic: runtime error: makeslice: len out of range'
	n, err := sm.SocConnection.Read(buffer)
	if err != nil {
		sm.LM.LogError(sm.ServiceName, "[ReadFromTapSocket] Error reading from socket: %v", err)
		return nil, fmt.Errorf("error reading from socket: %w", err)
	}
	return buffer[:n], nil
}

func (sm *SocketManager) Close() error {
	sm.mu.Lock()
	defer sm.mu.Unlock()

	if sm.SocConnection != nil {
		if err := sm.SocConnection.Close(); err != nil {
			sm.LM.LogError(sm.ServiceName, "Close: Error closing socket connection: %v", err)
			return fmt.Errorf("error closing socket connection: %w", err)
		}
		sm.SocConnection = nil
		sm.LM.LogInfo(sm.ServiceName, "Close: Socket connection closed successfully.")
	}
	return nil
}

func (sm *SocketManager) AutoReconnection(ip, port string) (net.Conn, error) {
	var Conn net.Conn
	var err error

	sm.LM.LogError(sm.ServiceName, "[AutoReconnection] Disconnected... Initiating automatic reconnection.")

	for i := 0; i < 5; i++ {
		Conn, err = sm.ConnectToTAP(ip, port)
		if err != nil {
			sm.LM.LogError(sm.ServiceName, "Reconnection attempt %d failed: %v", i+1, err)
		} else {
			sm.LM.LogInfo(sm.ServiceName, "Connection re-established successfully.")
			return Conn, nil
		}
	}

	sm.LM.LogError(sm.ServiceName, "[AutoReconnection] Reconnection attempts exceeded the limit. IP: %s, Port: %s", ip, port)
	fmt.Println("Program terminated: Unable to establish a connection to the socket. Please try again with a different IP or port.")
	os.Exit(1)

	return nil, err
}
