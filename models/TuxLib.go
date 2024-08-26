package models

import (
	"DATA_FWD_TAP/common"
	"fmt"
	"log"

	"gorm.io/gorm"
)

/**********************************************************************************/
/*                                                                                */
/*  Constants:                                                                    */
/*    - TRAN_TIMEOUT: Placeholder for transaction timeout.                        */
/*    - REMOTE_TRNSCTN: Constant for remote transaction type.                     */
/*    - LOCAL_TRNSCTN: Constant for local transaction type.                       */
/*                                                                                */
/*  Structs:                                                                      */
/*    - TransactionManager: Manages database transactions using GORM.             */
/*                                                                                */
/*  Functions:                                                                    */
/*    - NewTransactionManager: Creates and returns a new TransactionManager      */
/*      instance with the specified service name and database accessor.           */
/*                                                                                */
/*    - FnBeginTran: Begins a new transaction. Determines if the transaction is   */
/*      local or remote based on the database connection status.                   */
/*                                                                                */
/*    - PingDatabase: Checks if the database connection is alive by pinging it.   */
/*                                                                                */
/*    - FnCommitTran: Commits the transaction if it is local, or skips commit     */
/*      for remote transactions.                                                  */
/*                                                                                */
/*    - FnAbortTran: Rolls back the transaction if it is local, or skips rollback */
/*      for remote transactions.                                                  */
/*                                                                                */
/**********************************************************************************/

const (
	TRAN_TIMEOUT   = 300 // placeholder for transaction timeout . i have to use this .
	REMOTE_TRNSCTN = 2
	LOCAL_TRNSCTN  = 1
)

type TransactionManager struct {
	ServiceName string
	TranType    int
	Tran        *gorm.DB
	DbAccessor  common.DBAccessor
}

func NewTransactionManager(ServiceName string, dbAccessor common.DBAccessor) *TransactionManager {
	return &TransactionManager{
		ServiceName: ServiceName,
		DbAccessor:  dbAccessor,
	}
}

func (tm *TransactionManager) FnBeginTran() int {

	// Check if a transaction is already active (GORM does not provide tpgetlev equivalent, manage this externally if needed)

	db := tm.DbAccessor.GetDB() // Get the database instance on this instance we will check if there is any active transaction.
	fmt.Println(tm.DbAccessor)
	fmt.Println(db)
	err := tm.PingDatabase(db) // Use PingDatabase to check if there is any active transaction.

	if err == nil {
		tm.Tran = db.Begin() // Begin the transaction here. This line represents something unique. When we call db.Begin(), it returns a new *gorm.DB instance, and we are supposed to use this instance throughout the transaction. This is because we have to maintain the 'isolation'.
		if tm.Tran.Error != nil {
			errMsg := fmt.Sprintf("[%s] tpbegin error: %v", tm.ServiceName, tm.Tran.Error)
			log.Println("L31200", errMsg)
			return -1
		}
		tm.TranType = LOCAL_TRNSCTN
		log.Printf("[%s] EBA : tm.tranType:%d", tm.ServiceName, tm.TranType)

	} else {
		tm.TranType = REMOTE_TRNSCTN
		log.Printf("[%s] EBA : tm.tranType:%d", tm.ServiceName, tm.TranType)

	}
	return 0
}

func (tm *TransactionManager) PingDatabase(db *gorm.DB) error {
	sqlDB, err := db.DB() // Get the underlying *sql.DB
	if err != nil {
		log.Printf("[%s] Failed to get underlying *sql.DB: %v", tm.ServiceName, err)
		return err
	}
	if err := sqlDB.Ping(); err != nil {
		log.Printf("[%s] Failed to ping database: %v", tm.ServiceName, err)
		return err
	}
	return nil
}

func (tm *TransactionManager) FnCommitTran() int {
	switch tm.TranType {
	case LOCAL_TRNSCTN:
		if err := tm.Tran.Commit().Error; err != nil {
			log.Printf("[%s] tpcommit error: %v", tm.ServiceName, err)
			tm.Tran.Rollback() // Ensure rollback on commit error
			return -1
		}
		log.Printf("[%s] Transaction committed", tm.ServiceName)
	case REMOTE_TRNSCTN:
		log.Printf("[%s] Skipping commit for remote transaction", tm.ServiceName)
	default:
		log.Printf("[%s] Invalid Transaction Number |%d|", tm.ServiceName, tm.TranType)
		return -1
	}
	return 0
}

func (tm *TransactionManager) FnAbortTran() int {
	switch tm.TranType {
	case LOCAL_TRNSCTN:
		if err := tm.Tran.Rollback().Error; err != nil {
			log.Printf("[%s] tpabort error: %v", tm.ServiceName, err)
			return -1
		}
		log.Printf("[%s] Transaction aborted", tm.ServiceName)
	case REMOTE_TRNSCTN:
		log.Printf("[%s] Skipping abort for remote transaction", tm.ServiceName)
	default:
		log.Printf("[%s] Invalid Transaction Number |%d|", tm.ServiceName, tm.TranType)
		return -1
	}
	return 0
}
