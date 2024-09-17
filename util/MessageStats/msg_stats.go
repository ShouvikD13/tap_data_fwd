package MessageStat

/*
#cgo CFLAGS: -I.
#cgo LDFLAGS: -L. -lmsgqueue
#include "msg_queue_wrapper.h"
*/
import "C"
import (
	"fmt"
	"time"
)

// MsgQueueStats wraps the C MsgQueueStats struct
type MsgQueueStats struct {
	MsgStime  time.Time
	MsgRtime  time.Time
	MsgCtime  time.Time
	MsgCbytes uint64
	MsgQnum   uint64
	MsgQbytes uint64
	MsgLspid  int32
	MsgLrpid  int32
}

// GetMsgQueueStats gets the message queue statistics
func GetMsgQueueStats(key int) (*MsgQueueStats, error) {
	var stats C.MsgQueueStats

	// Call the C function to fetch message queue stats
	res := C.get_msg_queue_stats(C.key_t(key), &stats)
	if res != 0 {
		return nil, fmt.Errorf("failed to get message queue stats")
	}

	return &MsgQueueStats{
		MsgStime:  time.Unix(int64(stats.msg_stime), 0),
		MsgRtime:  time.Unix(int64(stats.msg_rtime), 0),
		MsgCtime:  time.Unix(int64(stats.msg_ctime), 0),
		MsgCbytes: uint64(stats.__msg_cbytes),
		MsgQnum:   uint64(stats.msg_qnum),
		MsgQbytes: uint64(stats.msg_qbytes),
		MsgLspid:  int32(stats.msg_lspid),
		MsgLrpid:  int32(stats.msg_lrpid),
	}, nil
}

// func main() {
// 	key := 1
// 	stats, err := GetMsgQueueStats(key)
// 	if err != nil {
// 		fmt.Println("Error:", err)
// 		return
// 	}

// 	fmt.Println("Message Queue Statistics:")
// 	fmt.Printf("  Last msgsnd time: %s\n", stats.MsgStime.Format("2006-01-02 15:04:05 MST"))
// 	fmt.Printf("  Last msgrcv time: %s\n", stats.MsgRtime.Format("2006-01-02 15:04:05 MST"))
// 	fmt.Printf("  Last change time: %s\n", stats.MsgCtime.Format("2006-01-02 15:04:05 MST"))
// 	fmt.Printf("  Current number of bytes on queue: %d\n", stats.MsgCbytes)
// 	fmt.Printf("  Number of messages currently on queue: %d\n", stats.MsgQnum)
// 	fmt.Printf("  Max number of bytes allowed on queue: %d\n", stats.MsgQbytes)
// 	fmt.Printf("  PID of last msgsnd: %d\n", stats.MsgLspid)
// 	fmt.Printf("  PID of last msgrcv: %d\n", stats.MsgLrpid)
// }
