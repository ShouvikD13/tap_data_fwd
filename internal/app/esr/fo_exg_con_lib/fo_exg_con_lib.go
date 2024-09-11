package foexgconlib

import (
	"fmt"

	"DATA_FWD_TAP/internal/models"
	"DATA_FWD_TAP/util"

	"gorm.io/gorm"
)

type FoExgConLibManager struct {
	St_sign_on_req *models.St_sign_on_req
	St_req_q_data  *models.St_req_q_data
	St_exch_msg    *models.St_exch_msg
	DB             *gorm.DB
	LoggerManager  *util.LoggerManager
	ServiceName    string
	IpPipeID       string
	UserID         int64
}

func (FECLM *FoExgConLibManager) LogOnToTap() int {
	// Query for checking if ESR is running
	query := `
		SELECT bps_stts
		FROM bps_btch_pgm_stts
		WHERE bps_pgm_name = ?
		AND bps_xchng_cd = 'NA'
		AND bps_pipe_id = ?
	`

	var bpsStts string
	result := FECLM.DB.Raw(query, "cln_esr_clnt", FECLM.IpPipeID).Scan(&bpsStts)

	if result.Error != nil {
		errorMsg := fmt.Sprintf("Error executing query: %v", result.Error)
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "[LogOnToTap] %s", errorMsg)
		return -1
	}

	if bpsStts == "N" {
		errorMsg := fmt.Sprintf("ESR client is not running for Pipe ID: %s and User ID: %d", FECLM.IpPipeID, FECLM.UserID)
		FECLM.LoggerManager.LogError(FECLM.ServiceName, "[LogOnToTap] %s", errorMsg)
		return -1
	}

	successMsg := fmt.Sprintf("ESR client is running for Pipe ID: %s and User ID: %d", FECLM.IpPipeID, FECLM.UserID)
	FECLM.LoggerManager.LogInfo(FECLM.ServiceName, "[LogOnToTap] %s", successMsg)

	return 0
}
