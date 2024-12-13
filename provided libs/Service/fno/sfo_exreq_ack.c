/******************************************************************************/
/*	Program	    			:	SFO_EXREQ_ACK                                         */
/*                                                                            */
/*  Input             : vw_xchngbook                                          */
/*                                                                            */
/*  Output            : vw_xchngbook                                          */
/*                                                                            */
/*  Description       : This service updates the Exercise book, exchange book,*/
/*                      positions and the limits on getting Response from the */
/*                      Exchange for Exercise requests. The various response  */
/*                      got from the Exchange are:                            */
/*                      ORS_NEW_EXER_ACPT                                     */
/*                      ORS_NEW_EXER_RJCT                                     */
/*                      ORS_MOD_EXER_ACPT                                     */
/*                      ORS_MOD_EXER_RJCT                                     */
/*                      ORS_CAN_EXER_ACPT                                     */
/*                      ORS_CAN_EXER_RJCT                                     */
/*                                                                            */
/*  Log               : 1.0   13-Nov-2001  S. Swaminathan                     */
/*										:	1.1		24-Dec-2007	 Ranjit IBM Migration								*/	
/*										:	1.2w	23-Jul-2008	 Wipro|Vaishali											*/	
/*                                                                            */
/******************************************************************************/
/*  1.0  - New Release                                                        */
/*  1.2w - CRCSN13948-Order Log strengthening																	*/
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atmi.h>
#include <fml32.h>
#include <sqlca.h>
#include <fo.h>
#include <eba_to_ors.h>
#include <fo_view_def.h>
#include <fn_tuxlib.h>
#include <fn_ddr.h>
#include <fn_log.h>				/* ver 1.1 */
#include <fn_session.h>		/* ver 1.1 */
#include <fn_read_debug_lvl.h>	/* ver 1.1 */

void SFO_EXREQ_ACK( TPSVCINFO *rqst )
{
	char	c_ServiceName[33];
	char	c_err_msg[256];
	int		i_trnsctn;
	int		i_returncode;
	int		i_ip_len;
	int		i_op_len;

	struct vw_xchngbook *ptr_st_xchngbook;
	struct vw_xchngbook st_xchngbook;
	struct vw_exrcbook st_exrcbook;
	struct vw_pstn_actn st_pstn_actn;
	struct vw_err_msg *ptr_st_err_msg;

	ptr_st_xchngbook = ( struct vw_xchngbook *)rqst->data;
	strcpy( c_ServiceName, rqst->name );
	INITDBGLVL(c_ServiceName);	/* ver 1.1 */

	ptr_st_err_msg = ( struct vw_err_msg * ) tpalloc ( "VIEW32",
                                                     "vw_err_msg",
                                                sizeof ( struct vw_err_msg ) );
	if ( ptr_st_err_msg ==  NULL )
	{
		fn_errlog ( c_ServiceName, "S31005", TPMSG, c_err_msg );
		tpreturn ( TPFAIL, NO_BFR, NULL, 0, 0 );
	}
	memset ( ptr_st_err_msg, 0, sizeof ( struct vw_err_msg ) );

  /*** Added for Order Routing ***/
  fn_init_ddr_val ( ptr_st_xchngbook->c_rout_crt );

	i_trnsctn = fn_begintran( c_ServiceName, c_err_msg );
	if ( i_trnsctn == -1 )
	{
		fn_errlog ( c_ServiceName, "S31010", LIBMSG, c_err_msg );
		strcpy ( ptr_st_err_msg->c_err_msg, c_err_msg );
		tpfree ( ( char * ) ptr_st_xchngbook );
		tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
	}

	if(DEBUG_MSG_LVL_3)
	{
	  fn_userlog( c_ServiceName, "i_trnsctn:%d:", i_trnsctn );
		fn_userlog( c_ServiceName, "ptr_st_xchngbook->c_oprn_typ:%c:",
                                             ptr_st_xchngbook->c_oprn_typ );
  }

	strcpy( st_exrcbook.c_exrc_ordr_rfrnc, ptr_st_xchngbook->c_ordr_rfrnc );
	st_exrcbook.c_oprn_typ = FOR_UPDATE;
	
	i_ip_len = sizeof ( struct vw_exrcbook );
	i_op_len = sizeof ( struct vw_exrcbook );

  /*** Added for Order Routing ***/
	fn_cpy_ddr ( st_exrcbook.c_rout_crt );

	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              &st_exrcbook,
                              &st_exrcbook,
                              "vw_exrcbook",
                              "vw_exrcbook",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_EXRQ_DTLS" );

	if ( i_returncode == -1 )
	{
		fn_errlog ( c_ServiceName, "S31015", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

	ptr_st_xchngbook->c_oprn_typ = WITH_ORS_MSG_TYP;

	i_ip_len = sizeof ( struct vw_xchngbook );
	i_op_len = sizeof ( struct vw_xchngbook );

  /*** Added for Order Routing ***/
	fn_cpy_ddr ( ptr_st_xchngbook->c_rout_crt );

	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              ptr_st_xchngbook,
                              &st_xchngbook,
                              "vw_xchngbook",
                              "vw_xchngbook",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_REF_TO_OMD" );

	if ( i_returncode == -1 )
	{
		fn_errlog ( c_ServiceName, "S31020", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

	fn_userlog ( c_ServiceName, "match account  :%s:", st_exrcbook.c_cln_mtch_accnt );	


	i_returncode = fn_lock_usr( c_ServiceName,  st_exrcbook.c_cln_mtch_accnt );
	if ( i_returncode != 0 )
	{
		fn_errlog ( c_ServiceName, "S31025", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

	if ( st_xchngbook.c_rms_prcsd_flg == 'P' )
	{
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_xchngbook, 0, 0 );
	}

  strcpy(st_pstn_actn.c_user_id,"system");
  st_pstn_actn.l_session_id  = 0;

	strcpy( st_pstn_actn.c_cln_mtch_accnt , st_exrcbook.c_cln_mtch_accnt );
	st_pstn_actn.l_eba_cntrct_id = st_exrcbook.l_eba_cntrct_id;
	strcpy( st_pstn_actn.c_xchng_cd , st_exrcbook.c_xchng_cd );
	st_pstn_actn.c_prd_typ = st_exrcbook.c_prd_typ;
	strcpy( st_pstn_actn.c_undrlyng , st_exrcbook.c_undrlyng );
	strcpy( st_pstn_actn.c_expry_dt , st_exrcbook.c_expry_dt );
	st_pstn_actn.c_exrc_typ  = st_exrcbook.c_exrc_typ;
	st_pstn_actn.c_opt_typ = st_exrcbook.c_opt_typ;
	st_pstn_actn.l_strike_prc = st_exrcbook.l_strike_prc;
	st_pstn_actn.c_ctgry_indstk = st_exrcbook.c_ctgry_indstk;
	st_pstn_actn.l_ca_lvl  = st_exrcbook.l_ca_lvl;
	st_pstn_actn.c_cntrct_tag  = '*';
	st_pstn_actn.l_actn_typ = ptr_st_xchngbook->l_ors_msg_typ;
	st_pstn_actn.d_add_mrgn_amt  = 0;

	switch ( ptr_st_xchngbook->l_ors_msg_typ )
	{
		case	ORS_NEW_EXER_ACPT:
		case	ORS_NEW_EXER_RJCT:
					st_pstn_actn.l_orgnl_qty = 0;
					st_pstn_actn.l_orgnl_rt  = 0;
					st_pstn_actn.l_new_qty = st_xchngbook.l_ord_tot_qty;
					st_pstn_actn.l_new_rt  = 0;
					st_pstn_actn.l_exec_qty  = 0;
					st_pstn_actn.l_exec_rt  = 0;
					break;

		case	ORS_MOD_EXER_ACPT:
		case	ORS_MOD_EXER_RJCT:
					st_pstn_actn.l_orgnl_qty = st_exrcbook.l_exrc_qty;
					st_pstn_actn.l_orgnl_rt  = 0;
					st_pstn_actn.l_new_qty = st_xchngbook.l_ord_tot_qty;
					st_pstn_actn.l_new_rt  = 0;
					st_pstn_actn.l_exec_qty  = 0;
					st_pstn_actn.l_exec_rt  = 0;
					break;

		case	ORS_CAN_EXER_ACPT:
		case	ORS_CAN_EXER_RJCT:
					st_pstn_actn.l_orgnl_qty = st_exrcbook.l_exrc_qty;
					st_pstn_actn.l_orgnl_rt  = 0;
					st_pstn_actn.l_new_qty = 0;
					st_pstn_actn.l_new_rt  = 0;
					st_pstn_actn.l_exec_qty  = 0;
					st_pstn_actn.l_exec_rt  = 0;
					break;

		default:
					strcpy( ptr_st_err_msg->c_err_msg, "Invalid Operation Type" );
          fn_userlog ( c_ServiceName, "ERROR :%s:", ptr_st_err_msg->c_err_msg );
          tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
					break;
	} 

	st_pstn_actn.c_trnsctn_flw = st_exrcbook.c_exrc_rqst_typ;

	i_ip_len = sizeof ( struct vw_pstn_actn );
	i_op_len = sizeof ( struct vw_pstn_actn );

  /*** Added for Order Routing ***/
	fn_cpy_ddr ( st_pstn_actn.c_rout_crt );

	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              &st_pstn_actn,
                              &st_pstn_actn,
                              "vw_pstn_actn",
                              "vw_pstn_actn",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_UPD_OPT_POS" );

	if ( i_returncode == -1 )
	{
		fn_errlog ( c_ServiceName, "S31030", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

	switch ( ptr_st_xchngbook->l_ors_msg_typ )
	{
		case	ORS_NEW_EXER_ACPT:
					if( st_exrcbook.c_exrc_stts == REQUESTED )
					{
						st_exrcbook.c_exrc_stts = REQUESTED;
					}
					else
					{
						st_exrcbook.c_exrc_stts = ORDERED;
					}
					strcpy( st_exrcbook.c_ack_tm,
                  ptr_st_xchngbook->c_entry_dt_tm );
					st_exrcbook.c_oprn_typ = UPDATE_STATUS_ACK_TM;
					break;

		case	ORS_NEW_EXER_RJCT:
					st_exrcbook.c_exrc_stts = REJECTED;
					strcpy( st_exrcbook.c_ack_tm,
                  ptr_st_xchngbook->c_entry_dt_tm );
					st_exrcbook.c_oprn_typ = UPDATE_STATUS_ACK_TM;
					break;

		case	ORS_MOD_EXER_ACPT:
					st_exrcbook.c_exrc_stts = ORDERED;
					st_exrcbook.l_exrc_qty = st_xchngbook.l_ord_tot_qty;
					strcpy( st_exrcbook.c_ack_tm,
                  ptr_st_xchngbook->c_ack_tm );
					st_exrcbook.c_oprn_typ = UPDATE_QTY_STATUS_ACKTM_ON_MOD;
					
					/*Ver 1.2w Order Log CR*/
					strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Exercise Modification Accepted");
					break;

		case	ORS_MOD_EXER_RJCT:
					if( st_exrcbook.c_exrc_stts == REQUESTED )
					{
						st_exrcbook.c_exrc_stts = REQUESTED;
					}
					else
					{
						st_exrcbook.c_exrc_stts = ORDERED;
					}
					st_exrcbook.c_oprn_typ = UPDATE_STATUS_ON_XCHNG_RESP;

					/*Ver 1.2w Order Log CR*/
					strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Exercise Modification Rejected");
					break;

		case	ORS_CAN_EXER_ACPT:
					st_exrcbook.c_exrc_stts = CANCELLED;
					strcpy( st_exrcbook.c_ack_tm,
                  ptr_st_xchngbook->c_ack_tm );
					st_exrcbook.c_oprn_typ = UPDATE_STATUS_ACK_TM;

					/*Ver 1.2w Order Log CR*/
					strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Exercise Cancellation Accepted");
					break;

		case	ORS_CAN_EXER_RJCT:
					st_exrcbook.c_exrc_stts = ORDERED;
					st_exrcbook.c_oprn_typ = UPDATE_STATUS_ON_XCHNG_RESP;

					/*Ver 1.2w Order Log CR*/
					strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Exercise Cancellation Rejected");
					break;

		default:
					break;
	}

	i_ip_len = sizeof ( struct vw_exrcbook );
	i_op_len = sizeof ( struct vw_exrcbook );
				
  /*** Added for Order Routing ***/
	fn_cpy_ddr ( st_exrcbook.c_rout_crt );

	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              &st_exrcbook,
                              &st_exrcbook,
                              "vw_exrcbook",
                              "vw_exrcbook",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_UPD_EXBK" );
				
	if ( i_returncode == -1 )
	{
		fn_errlog ( c_ServiceName, "S31035", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}


  ptr_st_xchngbook->c_rms_prcsd_flg = 'P';
	ptr_st_xchngbook->c_oprn_typ = UPDATE_RMS_PRCSD_FLG;

	i_ip_len = sizeof ( struct vw_xchngbook );
	i_op_len = sizeof ( struct vw_xchngbook );
				
  /*** Added for Order Routing ***/
	fn_cpy_ddr ( ptr_st_xchngbook->c_rout_crt );

	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              ptr_st_xchngbook,
                              ptr_st_xchngbook,
                              "vw_xchngbook",
                              "vw_xchngbook",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_UPD_XCHNGBK" );
				
	if ( i_returncode == -1 )
	{
		fn_errlog ( c_ServiceName, "S31040", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

	if ( fn_committran( c_ServiceName, i_trnsctn, 
                                     ptr_st_err_msg->c_err_msg ) == -1 )
	{
		fn_errlog ( c_ServiceName, "S31045", LIBMSG, c_err_msg );
  	fn_errlog ( c_ServiceName, "S31050", TPMSG, ptr_st_err_msg->c_err_msg );
  	tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
	}

  if ( i_trnsctn == LOCAL_TRNSCTN )
  {
    /*** Added for Order Routing ***/
    fn_cpy_ddr ( st_exrcbook.c_rout_crt );

/* ORS_NEW_EXER_REQ, ORS_NEW_EXER_ACPT, ORS_NEW_EXER_RJCT, 
   ORS_MOD_EXER_REQ, ORS_MOD_EXER_ACPT, ORS_MOD_EXER_RJCT,
   ORS_CAN_EXER_REQ, ORS_CAN_EXER_ACPT, ORS_CAN_EXER_RJCT,
   EXER_CONF, EXER_RJCT */

   i_returncode = fn_acall_svc( c_ServiceName,
                                ptr_st_err_msg->c_err_msg,
                                &st_exrcbook,
                                "vw_exrcbook",
                                sizeof( struct vw_exrcbook),
                                TPNOREPLY,
                                "SFO_ML_FRMTTR" );
    if ( i_returncode != SUCC_BFR )
    {
      fn_errlog ( c_ServiceName, "S31055", LIBMSG, c_err_msg );
/************
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
*********/
    }
  }

	tpfree ( ( char * ) ptr_st_err_msg );
  tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_xchngbook, 0, 0 );
} 
