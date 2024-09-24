package initializers

import (
	"DATA_FWD_TAP/internal/app/exchange_connection"
	"DATA_FWD_TAP/internal/app/pack_clnt"
)

type ServiceInitializationManager struct {
	MainContainer *MainContainer
}

func (SIM *ServiceInitializationManager) ClnPackClntInitialization() int {
	VarClnPack := &pack_clnt.ClnPackClntManager{
		ServiceName:              SIM.MainContainer.UtilContainer.ServiceName,
		Xchngbook:                SIM.MainContainer.ClientContainer.Xchngbook,
		Orderbook:                SIM.MainContainer.ClientContainer.Orderbook,
		Contract:                 SIM.MainContainer.ClientContainer.Contract,
		Nse_contract:             SIM.MainContainer.ClientContainer.NseContract,
		Pipe_mstr:                SIM.MainContainer.ClientContainer.PipeMaster,
		Oe_reqres:                SIM.MainContainer.ClientContainer.OeReqres,
		Exch_msg:                 SIM.MainContainer.ClientContainer.ExchMsg,
		Net_hdr:                  SIM.MainContainer.ClientContainer.NetHdr,
		Q_packet:                 SIM.MainContainer.ClientContainer.QPacket,
		Int_header:               SIM.MainContainer.ClientContainer.IntHeader,
		Contract_desc:            SIM.MainContainer.ClientContainer.ContractDesc,
		Order_flag:               SIM.MainContainer.ClientContainer.OrderFlag,
		Order_conversion_manager: SIM.MainContainer.UtilContainer.OrderConversionManager,
		CPanNo:                   SIM.MainContainer.ClientGlobalValueContainer.CPanNo,
		CLstActRef:               SIM.MainContainer.ClientGlobalValueContainer.CLastActRef,
		CEspID:                   SIM.MainContainer.ClientGlobalValueContainer.CEspID,
		CAlgoID:                  SIM.MainContainer.ClientGlobalValueContainer.CAlgoID,
		CSourceFlg:               SIM.MainContainer.ClientGlobalValueContainer.CSourceFlg,
		CPrgmFlg:                 SIM.MainContainer.ClientGlobalValueContainer.CPrgmFlg,
		Enviroment_manager:       SIM.MainContainer.UtilContainer.EnvironmentManager,
		Message_queue_manager:    SIM.MainContainer.UtilContainer.MessageQueueManager,
		Transaction_manager:      SIM.MainContainer.UtilContainer.TransactionManager,
		Max_Pack_Val:             SIM.MainContainer.UtilContainer.MaxPackVal,
		Config_manager:           SIM.MainContainer.UtilContainer.ConfigManager,
		LoggerManager:            SIM.MainContainer.UtilContainer.LoggerManager,
		TCUM:                     SIM.MainContainer.UtilContainer.TypeConversionUtilManager,
		MtypeWrite:               SIM.MainContainer.UtilContainer.MTypeWrite,
		Args:                     SIM.MainContainer.ClientGlobalValueContainer.Args,
		Db:                       SIM.MainContainer.UtilContainer.DB,
	}

	initResult := VarClnPack.Fn_bat_init()

	if initResult != 0 {
		SIM.MainContainer.UtilContainer.LoggerManager.LogInfo(SIM.MainContainer.UtilContainer.ServiceName, " Fn_bat_init failed with result code: %d", initResult)
		return -1
	}
	return 0
}

func (SIM *ServiceInitializationManager) LogOnToTapInitialization() int {

	VarLogOn := &exchange_connection.LogOnToTapManager{
		ServiceName:                SIM.MainContainer.UtilContainer.ServiceName,
		St_sign_on_req:             SIM.MainContainer.LogOnContainer.StSignOnReq,
		St_req_q_data:              SIM.MainContainer.LogOnContainer.StReqQData,
		St_exch_msg_Log_On:         SIM.MainContainer.LogOnContainer.StExchMsgLogOn,
		Int_header:                 SIM.MainContainer.LogOnContainer.IntHeader,
		St_net_hdr:                 SIM.MainContainer.LogOnContainer.StNetHdr,
		St_BrokerEligibilityPerMkt: SIM.MainContainer.LogOnContainer.StBrokerEligibilityPerMkt,
		EM:                         SIM.MainContainer.UtilContainer.EnvironmentManager,
		PUM:                        SIM.MainContainer.UtilContainer.PasswordUtilManager,
		DB:                         SIM.MainContainer.UtilContainer.DB,
		LoggerManager:              SIM.MainContainer.UtilContainer.LoggerManager,
		OCM:                        SIM.MainContainer.UtilContainer.OrderConversionManager,
		TCUM:                       SIM.MainContainer.UtilContainer.TypeConversionUtilManager,
		Message_queue_manager:      SIM.MainContainer.UtilContainer.MessageQueueManager,
		C_pipe_id:                  SIM.MainContainer.LogOnGlobalValueContainer.CPipeID,
		UserID:                     SIM.MainContainer.LogOnGlobalValueContainer.UserID,
		MtypeWrite:                 SIM.MainContainer.UtilContainer.MTypeWrite,
		Max_Pack_Val:               SIM.MainContainer.UtilContainer.MaxPackVal,
		Opm_loginStatus:            SIM.MainContainer.LogOnGlobalValueContainer.OpmLoginStatus,
		Opm_userID:                 SIM.MainContainer.LogOnGlobalValueContainer.OpmUserID,
		Opm_existingPasswd:         SIM.MainContainer.LogOnGlobalValueContainer.OpmExistingPasswd,
		Opm_newPasswd:              SIM.MainContainer.LogOnGlobalValueContainer.OpmNewPasswd,
		Opm_LstPswdChgDt:           SIM.MainContainer.LogOnGlobalValueContainer.OpmLstPswdChgDt,
		Opm_XchngCd:                SIM.MainContainer.LogOnGlobalValueContainer.OpmXchngCd,
		Opm_TrdrID:                 SIM.MainContainer.LogOnGlobalValueContainer.OpmTrdrID,
		Opm_BrnchID:                SIM.MainContainer.LogOnGlobalValueContainer.OpmBrnchID,
		Exg_BrkrID:                 SIM.MainContainer.LogOnGlobalValueContainer.ExgBrkrID,
		Exg_NxtTrdDate:             SIM.MainContainer.LogOnGlobalValueContainer.ExgNxtTrdDate,
		Exg_BrkrName:               SIM.MainContainer.LogOnGlobalValueContainer.ExgBrkrName,
		Exg_BrkrStts:               SIM.MainContainer.LogOnGlobalValueContainer.ExgBrkrStts,
		Args:                       SIM.MainContainer.LogOnGlobalValueContainer.Args,
	}

	initResult := VarLogOn.LogOnToTap()

	if initResult != 0 {
		SIM.MainContainer.UtilContainer.LoggerManager.LogInfo(SIM.MainContainer.UtilContainer.ServiceName, " LogOnToTap failed with result code: %d", initResult)
		return -1
	}

	return 0
}

func (SIM *ServiceInitializationManager) LogOffFromTapInitialization() int {

	VarLogOff := &exchange_connection.LogOffFromTapManager{
		DB:                    SIM.MainContainer.UtilContainer.DB,
		LoggerManager:         SIM.MainContainer.UtilContainer.LoggerManager,
		ServiceName:           SIM.MainContainer.UtilContainer.ServiceName,
		MtypeWrite:            SIM.MainContainer.UtilContainer.MTypeWrite,
		Int_header:            SIM.MainContainer.LogOffContainer.IntHeader,
		St_net_hdr:            SIM.MainContainer.LogOffContainer.StNetHdr,
		Exch_msg_Log_Off:      SIM.MainContainer.LogOffContainer.StExchMsgLogOff,
		St_req_q_data:         SIM.MainContainer.LogOffContainer.StReqQData,
		TCUM:                  SIM.MainContainer.UtilContainer.TypeConversionUtilManager,
		Message_queue_manager: SIM.MainContainer.UtilContainer.MessageQueueManager,
		Opm_XchngCd:           SIM.MainContainer.LogOffGlobalValueContainer.OpmXchngCd,
		Opm_TrdrID:            SIM.MainContainer.LogOffGlobalValueContainer.OpmTrdrID,
		Exg_NxtTrdDate:        SIM.MainContainer.LogOffGlobalValueContainer.ExgNxtTrdDate,
		C_pipe_id:             SIM.MainContainer.LogOffGlobalValueContainer.C_pipe_id,
		Args:                  SIM.MainContainer.LogOffGlobalValueContainer.Args,
		Max_Pack_Val:          SIM.MainContainer.UtilContainer.MaxPackVal,
	}

	initResult := VarLogOff.Fn_logoff_from_TAP()

	if initResult != 0 {
		SIM.MainContainer.UtilContainer.LoggerManager.LogInfo(SIM.MainContainer.UtilContainer.ServiceName, " Fn_logoff_from_TAP() failed with result code: %d", initResult)
		return -1
	}

	return 0

}
