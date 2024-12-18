/******************************************************************************/
/*	Program	    			:	SFO_ACK_TO_ORD                                        */
/*                                                                            */
/*  Input             : vw_orderbook                                          */
/*                                                                            */
/*  Output            : vw_orderbook                                          */
/*                      vw_err_msg                                            */
/*                                                                            */
/*  Description       : To get the details of the Order for the given         */
/*                      input of Exchange Acknowledgement and Pipe ID         */
/*                                                                            */
/*  Log               : 1.0   03-Oct-2001   S. Swaminathan                    */
/*                      1.1		22-Feb-2007		Sushil Sharma                     */
/*											1.2		03-Mar-2008		Sandeep Patil											*/
/*											1.3		15-Oct-2008		Shailesh Hinge										*/
/*                                                                            */
/******************************************************************************/
/*  1.0  -  New release                                                       */
/*  1.1	 -	Inserting user_id into vw_orderbook                               */
/*	1.2	 -  IBM MIGRATION CHANGES																							*/	
/*	1.3	 -  Trade Reconcilation change																				*/	
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atmi.h>
#include <sqlca.h>
#include <fml32.h>
#include <fo.h>
#include <fo_fml_def.h>
#include <fo_view_def.h>
#include <fn_tuxlib.h>
#include <fn_ddr.h>
#include <fn_log.h>
#include <fn_read_debug_lvl.h>			/*** Ver 1.2 ***/

void SFO_ACK_TO_ORD( TPSVCINFO *rqst )
{
	char c_ServiceName[ 33];
	char c_errmsg[256];
	char c_msg[256];

	EXEC SQL BEGIN DECLARE SECTION;
		struct vw_orderbook *ptr_st_orderbook;
		short	 i_settlor;
		short	 i_sprd_ord_ref;
		short	 i_ack_tm;
		short	 i_prev_ack_tm;
	EXEC SQL END DECLARE SECTION;

	struct vw_err_msg *ptr_st_err_msg;

	ptr_st_orderbook = ( struct vw_orderbook *)rqst->data;
	strcpy( c_ServiceName, rqst->name );

	INITDBGLVL(c_ServiceName);			/*** Ver 1.2 ***/

	ptr_st_err_msg = ( struct vw_err_msg * ) tpalloc ( "VIEW32",
                                                     "vw_err_msg",
                                                sizeof ( struct vw_err_msg ) );
	if ( ptr_st_err_msg ==  NULL )
	{
		fn_errlog ( c_ServiceName, "S31005", TPMSG, c_errmsg );
		tpreturn ( TPFAIL, NO_BFR, NULL, 0, 0 );
	}

	memset ( ptr_st_err_msg, 0, sizeof ( struct vw_err_msg ) );

	if (DEBUG_MSG_LVL_3)			/*** Ver 1.2 ***/ 
	{
  	fn_userlog ( c_ServiceName,"c_xchng_ack:%s:", ptr_st_orderbook->c_xchng_ack );
  	fn_userlog ( c_ServiceName,"c_pipe_id:%s:", ptr_st_orderbook->c_pipe_id );
	}

  /*** Added for Order Routing ***/
  fn_init_ddr_val ( ptr_st_orderbook->c_rout_crt );

	if ( ptr_st_orderbook->c_oprn_typ == FOR_VIEW || ptr_st_orderbook->c_oprn_typ == FOR_TRDRECO )		/**Ver 1.3 **/
	{
		if (DEBUG_MSG_LVL_3)			/*** Ver 1.2 ***/
		{
			fn_userlog(c_ServiceName,"INSIDE FOR_VIEW");		/*****	ADDED IN VER 1.1   *****/
	  }

		EXEC SQL
			SELECT  fod_clm_mtch_accnt,
							fod_clnt_ctgry,
							fod_ordr_rfrnc,
							fod_xchng_cd,
							fod_prdct_typ,
							fod_undrlyng,
							to_char ( fod_expry_dt, 'dd-Mon-yyyy' ),
							fod_exer_typ,
							fod_opt_typ,
							fod_strk_prc,
							fod_ordr_flw,
							fod_lmt_mrkt_sl_flg,
							NVL( fod_dsclsd_qty, 0 ),
							NVL ( fod_ordr_tot_qty, 0 ),
							NVL( fod_lmt_rt, 0 ),
							fod_stp_lss_tgr,
							fod_ordr_type,
							to_char ( fod_ordr_valid_dt, 'dd-Mon-yyyy' ),
							to_char ( fod_trd_dt, 'dd-Mon-yyyy' ),
							fod_ordr_stts,
							NVL( fod_exec_qty, 0 ),
							NVL( fod_exec_qty_day, 0 ),
							NVL( fod_cncl_qty, 0 ),
							NVL( fod_exprd_qty, 0 ),
							fod_sprd_ordr_ref,
							fod_mdfctn_cntr,
							fod_settlor, 
							fod_spl_flag,
							fod_indstk,
							to_char ( fod_ord_ack_tm, 'dd-Mon-yyyy hh24:mi:ss' ),
							to_char ( fod_lst_rqst_ack_tm, 'dd-Mon-yyyy hh24:mi:ss' ),
							NVL( trim(fod_usr_id), '*' )                	      /*****    COLUMN ADDED IN VER 1.1   *****/
			INTO    :ptr_st_orderbook->c_cln_mtch_accnt,
              :ptr_st_orderbook->l_clnt_ctgry,
							:ptr_st_orderbook->c_ordr_rfrnc,
							:ptr_st_orderbook->c_xchng_cd,
              :ptr_st_orderbook->c_prd_typ,
              :ptr_st_orderbook->c_undrlyng,
              :ptr_st_orderbook->c_expry_dt,
              :ptr_st_orderbook->c_exrc_typ,
              :ptr_st_orderbook->c_opt_typ,
              :ptr_st_orderbook->l_strike_prc,
              :ptr_st_orderbook->c_ordr_flw,
              :ptr_st_orderbook->c_slm_flg,
              :ptr_st_orderbook->l_dsclsd_qty,
              :ptr_st_orderbook->l_ord_tot_qty,
              :ptr_st_orderbook->l_ord_lmt_rt,
              :ptr_st_orderbook->l_stp_lss_tgr,
              :ptr_st_orderbook->c_ord_typ,
              :ptr_st_orderbook->c_valid_dt,
              :ptr_st_orderbook->c_trd_dt,
              :ptr_st_orderbook->c_ordr_stts,
              :ptr_st_orderbook->l_exctd_qty,
              :ptr_st_orderbook->l_exctd_qty_day,
              :ptr_st_orderbook->l_can_qty,
              :ptr_st_orderbook->l_exprd_qty,
              :ptr_st_orderbook->c_sprd_ord_rfrnc :i_sprd_ord_ref,
              :ptr_st_orderbook->l_mdfctn_cntr,
              :ptr_st_orderbook->c_settlor :i_settlor,
              :ptr_st_orderbook->c_req_typ,
							:ptr_st_orderbook->c_ctgry_indstk,
							:ptr_st_orderbook->c_ack_tm :i_ack_tm,
							:ptr_st_orderbook->c_prev_ack_tm :i_prev_ack_tm,
							:ptr_st_orderbook->c_user_id                         /*****    FIELD ADDED IN VER 1.1   *****/
			FROM	 	fod_fo_ordr_dtls
			WHERE 	fod_ack_nmbr = :ptr_st_orderbook->c_xchng_ack
			AND     fod_pipe_id  = :ptr_st_orderbook->c_pipe_id;
	}
	else if ( ptr_st_orderbook->c_oprn_typ == FOR_UPDATE )
	{
		if (DEBUG_MSG_LVL_3)			/*** Ver 1.2 ***/
		{
			fn_userlog(c_ServiceName,"INSIDE FOR_UPDATE");    /*****  ADDED IN VER 1.1   *****/
		}

		EXEC SQL
			SELECT 	fod_clm_mtch_accnt,
							fod_clnt_ctgry,
							fod_ordr_rfrnc,
							fod_xchng_cd,
							fod_prdct_typ,
							fod_undrlyng,
							to_char ( fod_expry_dt, 'dd-Mon-yyyy' ),
							fod_exer_typ,
							fod_opt_typ,
							fod_strk_prc,
							fod_ordr_flw,
							fod_lmt_mrkt_sl_flg,
							NVL( fod_dsclsd_qty, 0 ),
							NVL ( fod_ordr_tot_qty, 0 ),
							NVL( fod_lmt_rt, 0 ),
							fod_stp_lss_tgr,
							fod_ordr_type,
							to_char ( fod_ordr_valid_dt, 'dd-Mon-yyyy' ),
							to_char ( fod_trd_dt, 'dd-Mon-yyyy' ),
							fod_ordr_stts,
							NVL( fod_exec_qty, 0 ),
							NVL( fod_exec_qty_day, 0 ),
							NVL( fod_cncl_qty, 0 ),
							NVL( fod_exprd_qty, 0 ),
							fod_sprd_ordr_ref,
							fod_mdfctn_cntr,
							fod_settlor, 
							fod_spl_flag,
							fod_indstk,
							to_char ( fod_ord_ack_tm, 'dd-Mon-yyyy hh24:mi:ss' ),
							to_char ( fod_lst_rqst_ack_tm, 'dd-Mon-yyyy hh24:mi:ss' )
			INTO    :ptr_st_orderbook->c_cln_mtch_accnt,
              :ptr_st_orderbook->l_clnt_ctgry,
							:ptr_st_orderbook->c_ordr_rfrnc,
							:ptr_st_orderbook->c_xchng_cd,
              :ptr_st_orderbook->c_prd_typ,
              :ptr_st_orderbook->c_undrlyng,
              :ptr_st_orderbook->c_expry_dt,
              :ptr_st_orderbook->c_exrc_typ,
              :ptr_st_orderbook->c_opt_typ,
              :ptr_st_orderbook->l_strike_prc,
              :ptr_st_orderbook->c_ordr_flw,
              :ptr_st_orderbook->c_slm_flg,
              :ptr_st_orderbook->l_dsclsd_qty,
              :ptr_st_orderbook->l_ord_tot_qty,
              :ptr_st_orderbook->l_ord_lmt_rt,
              :ptr_st_orderbook->l_stp_lss_tgr,
              :ptr_st_orderbook->c_ord_typ,
              :ptr_st_orderbook->c_valid_dt,
              :ptr_st_orderbook->c_trd_dt,
              :ptr_st_orderbook->c_ordr_stts,
              :ptr_st_orderbook->l_exctd_qty,
              :ptr_st_orderbook->l_exctd_qty_day,
              :ptr_st_orderbook->l_can_qty,
              :ptr_st_orderbook->l_exprd_qty,
              :ptr_st_orderbook->c_sprd_ord_rfrnc :i_sprd_ord_ref,
              :ptr_st_orderbook->l_mdfctn_cntr,
              :ptr_st_orderbook->c_settlor :i_settlor,
              :ptr_st_orderbook->c_req_typ,
							:ptr_st_orderbook->c_ctgry_indstk,
							:ptr_st_orderbook->c_ack_tm :i_ack_tm,
							:ptr_st_orderbook->c_prev_ack_tm :i_prev_ack_tm
			FROM	 	fod_fo_ordr_dtls
			WHERE 	fod_ack_nmbr = :ptr_st_orderbook->c_xchng_ack
			AND     fod_pipe_id  = :ptr_st_orderbook->c_pipe_id
			FOR UPDATE OF fod_ordr_rfrnc;
	}
	else 
	{
    fn_errlog ( c_ServiceName, "S31010", "Invalid option", 
                ptr_st_err_msg->c_err_msg );
    tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
	}

	if ( ( SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
  {
		if ( SQLCODE == TOO_MANY_ROWS_FETCHED )
		{
    	tpreturn ( TPSUCCESS, TOO_MANY_ROWS_FETCHED,
                 ( char * )ptr_st_err_msg, 0, 0 );
		}
    fn_errlog ( c_ServiceName, "S31015", SQLMSG, ptr_st_err_msg->c_err_msg );
    tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
  }

	/*** Commented by sangeet for performance
	if ( ( SQLCODE == NO_DATA_FOUND ) &&
       ( ptr_st_orderbook->c_oprn_typ == FOR_VIEW ) )
	{
		EXEC SQL
			SELECT 	fod_clm_mtch_accnt,
							fod_clnt_ctgry,
							fod_ordr_rfrnc,
							fod_xchng_cd,
							fod_prdct_typ,
							fod_undrlyng,
							to_char ( fod_expry_dt, 'dd-Mon-yyyy' ),
							fod_exer_typ,
							fod_opt_typ,
							fod_strk_prc,
							fod_ordr_flw,
							fod_lmt_mrkt_sl_flg,
							NVL( fod_dsclsd_qty, 0 ),
							NVL ( fod_ordr_tot_qty, 0 ),
							NVL( fod_lmt_rt, 0 ),
							fod_stp_lss_tgr,
							fod_ordr_type,
							to_char ( fod_ordr_valid_dt, 'dd-Mon-yyyy' ),
							to_char ( fod_trd_dt, 'dd-Mon-yyyy' ),
							fod_ordr_stts,
							NVL( fod_exec_qty, 0 ),
							NVL( fod_cncl_qty, 0 ),
							NVL( fod_exprd_qty, 0 ),
							NVL( fod_sprd_ordr_ref, '*' ),
							fod_mdfctn_cntr,
							NVL( fod_settlor, '*' ),
							NVL( fod_spl_flag, ' ' ),
							NVL( fod_indstk, '*' ),
							to_char ( fod_ord_ack_tm, 'dd-Mon-yyyy hh24:mi:ss' ),
							to_char ( fod_lst_rqst_ack_tm, 'dd-Mon-yyyy hh24:mi:ss' ),
							NVL( trim(fod_usr_id), '*' )        
			INTO    :ptr_st_orderbook->c_cln_mtch_accnt,
              :ptr_st_orderbook->l_clnt_ctgry,
							:ptr_st_orderbook->c_ordr_rfrnc,
							:ptr_st_orderbook->c_xchng_cd,
              :ptr_st_orderbook->c_prd_typ,
              :ptr_st_orderbook->c_undrlyng,
              :ptr_st_orderbook->c_expry_dt,
              :ptr_st_orderbook->c_exrc_typ,
              :ptr_st_orderbook->c_opt_typ,
              :ptr_st_orderbook->l_strike_prc,
              :ptr_st_orderbook->c_ordr_flw,
              :ptr_st_orderbook->c_slm_flg,
              :ptr_st_orderbook->l_dsclsd_qty,
              :ptr_st_orderbook->l_ord_tot_qty,
              :ptr_st_orderbook->l_ord_lmt_rt,
              :ptr_st_orderbook->l_stp_lss_tgr,
              :ptr_st_orderbook->c_ord_typ,
              :ptr_st_orderbook->c_valid_dt,
              :ptr_st_orderbook->c_trd_dt,
              :ptr_st_orderbook->c_ordr_stts,
              :ptr_st_orderbook->l_exctd_qty,
              :ptr_st_orderbook->l_can_qty,
              :ptr_st_orderbook->l_exprd_qty,
              :ptr_st_orderbook->c_sprd_ord_rfrnc,
              :ptr_st_orderbook->l_mdfctn_cntr,
              :ptr_st_orderbook->c_settlor,
              :ptr_st_orderbook->c_req_typ,
							:ptr_st_orderbook->c_ctgry_indstk,
							:ptr_st_orderbook->c_ack_tm,
							:ptr_st_orderbook->c_prev_ack_tm,
							:ptr_st_orderbook->c_user_id         
			FROM	 	fod_fo_ordr_dtls_hstry
			WHERE 	fod_ack_nmbr = :ptr_st_orderbook->c_xchng_ack
			AND     fod_pipe_id  = :ptr_st_orderbook->c_pipe_id;

			if ( SQLCODE != 0 )
			{
    		fn_errlog ( c_ServiceName,"S31020", SQLMSG, ptr_st_err_msg->c_err_msg );
    		tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
			}
	}   ***/

  if ( SQLCODE == NO_DATA_FOUND )
  {
		/**Ver 1.3 starts **/
		if( ptr_st_orderbook->c_oprn_typ == FOR_TRDRECO)
		{
			fn_userlog(c_ServiceName,"Inside FOR_TRDRECO");
    	tpreturn ( TPSUCCESS, NO_DATA_FOUND, ( char * )ptr_st_err_msg, 0, 0 );

		} 
		/**Ver 1.3 ends **/

    fn_errlog ( c_ServiceName,"S31025", SQLMSG, ptr_st_err_msg->c_err_msg );
    tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
  }
  /** ver 1.1 added **/
  rtrim(ptr_st_orderbook->c_user_id); 
  if (DEBUG_MSG_LVL_3)				/*** Ver 1.2 ***/
	{
  	fn_userlog(c_ServiceName,"User id right trimmed to :%s:",ptr_st_orderbook->c_user_id);
  }
  /** ver 1.1 additon ends **/

	rtrim ( ptr_st_orderbook->c_expry_dt );

	/*****************	ADDED IN VER 1.1	****************/
	if (DEBUG_MSG_LVL_3)			/*** Ver 1.2 ***/
	{
  	fn_userlog ( c_ServiceName, "OUTSIDE c_user_id:%s:", ptr_st_orderbook->c_user_id );
	}

	if ( ptr_st_orderbook->c_user_id[0]=='*' )
	{
		strcpy( ptr_st_orderbook->c_user_id, "" );
		if (DEBUG_MSG_LVL_3)			/*** Ver 1.2 ***/
		{
    	fn_userlog ( c_ServiceName, "INSIDE c_user_id:%s:", ptr_st_orderbook->c_user_id );
  	}
	}

	tpfree ( ( char * ) ptr_st_err_msg );
  tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_orderbook, 
                                  sizeof( struct vw_orderbook ), 0 );
}
