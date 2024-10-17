package socket

import (
	"DATA_FWD_TAP/util"
	"fmt"
	"net"
	"sync"
)

const (
	DefaultBufferSize = 400 // i have create this buffer because maximum size of all structure is 400
)

type SocketManager struct {
	LM            *util.LoggerManager
	SocConnection *net.Conn
	ServiceName   string
	mu            sync.Mutex
}

func NewSocketManager(lm *util.LoggerManager, serviceName string) *SocketManager {
	return &SocketManager{
		LM:          lm,
		ServiceName: serviceName,
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

func (sm *SocketManager) WriteOnTapSocket(net_header []byte, message []byte) error {
	sm.mu.Lock()
	defer sm.mu.Unlock()

	if sm.SocConnection == nil {
		sm.LM.LogError(sm.ServiceName, "Write: Socket connection is not established.")
		return fmt.Errorf("socket connection is not established")
	}

	// Step 1: Send HDR (the 22-byte net_header)
	if len(net_header) != 22 {
		return fmt.Errorf("invalid net_header size, expected 22 bytes, got %d", len(net_header))
	}

	n, err := sm.SocConnection.Write(net_header)
	if err != nil {
		sm.LM.LogError(sm.ServiceName, "Write: Error writing net_header to socket: %v", err)
		return fmt.Errorf("error writing net_header to socket: %w", err)
	}
	sm.LM.LogInfo(sm.ServiceName, "Write: Successfully wrote %d bytes of net_header to socket.", n)

	// Step 2: Send Actual Message (For ordinay order , LogOn , LogOff)
	n, err = sm.SocConnection.Write(message)
	if err != nil {
		sm.LM.LogError(sm.ServiceName, "Write: Error writing message to socket: %v", err)
		return fmt.Errorf("error writing message to socket: %w", err)
	}
	sm.LM.LogInfo(sm.ServiceName, "Write: Successfully wrote %d bytes of message to socket.", n)

	return nil
}

func (sm *SocketManager) ReadFromTapSocket(bufferSize int) ([]byte, error) {
	sm.mu.Lock()
	defer sm.mu.Unlock()

	if sm.SocConnection == nil {
		sm.LM.LogError(sm.ServiceName, "Read: Socket connection is not established.")
		return nil, fmt.Errorf("socket connection is not established")
	}

	buffer := make([]byte, bufferSize)
	n, err := sm.SocConnection.Read(buffer)
	if err != nil {
		sm.LM.LogError(sm.ServiceName, "Read: Error reading from socket: %v", err)
		return nil, fmt.Errorf("error reading from socket: %w", err)
	}

	sm.LM.LogInfo(sm.ServiceName, "Read: Successfully read %d bytes from socket.", n)
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
