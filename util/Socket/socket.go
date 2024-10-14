package socket

import (
	"DATA_FWD_TAP/util"
	"fmt"
	"net"
	"sync"
)

const (
	DefaultBufferSize = 400
)

type SocketManager struct {
	LM            *util.LoggerManager
	SocConnection net.Conn
	ServiceName   string
	mu            sync.Mutex
}

func NewSocketManager(lm *util.LoggerManager, serviceName string) *SocketManager {
	return &SocketManager{
		LM:          lm,
		ServiceName: serviceName,
	}
}

func (sm *SocketManager) Connect(ip, port string) error {
	address := fmt.Sprintf("%s:%s", ip, port)
	connection, err := net.Dial("tcp", address)
	if err != nil {
		sm.LM.LogError(sm.ServiceName, "Connect: Failed to connect to socket %s: %v", address, err)
		return fmt.Errorf("failed to connect to socket %s: %w", address, err)
	}

	sm.mu.Lock()
	sm.SocConnection = connection // setting the connection instance globally
	sm.mu.Unlock()

	sm.LM.LogInfo(sm.ServiceName, "Connect: Successfully connected to socket %s.", address)
	return nil
}

func (sm *SocketManager) Write(data []byte) error {
	sm.mu.Lock()
	defer sm.mu.Unlock()

	if sm.SocConnection == nil {
		sm.LM.LogError(sm.ServiceName, "Write: Socket connection is not established.")
		return fmt.Errorf("socket connection is not established")
	}

	n, err := sm.SocConnection.Write(data)
	if err != nil {
		sm.LM.LogError(sm.ServiceName, "Write: Error writing to socket: %v", err)
		return fmt.Errorf("error writing to socket: %w", err)
	}

	sm.LM.LogInfo(sm.ServiceName, "Write: Successfully wrote %d bytes to socket.", n)
	return nil
}

func (sm *SocketManager) Read(bufferSize int) ([]byte, error) {
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
