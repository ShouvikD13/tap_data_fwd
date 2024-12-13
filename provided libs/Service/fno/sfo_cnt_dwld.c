/******************************************************************************/
/*	Program	    			:	SFO_CNT_DWLD                                            */
/*                                                                            */
/*  Input             :                                                       */
/*                                                                            */
/*                                                                            */
/*  Output            :                                                       */
/*                                                                            */
/*  Description       :                                                       */
/*                                                                            */
/*  Log               :                                                       */
/*                                                                            */
/******************************************************************************/
/*     24-Nov-2001       Ver 1.0        S.Balamurugan      Release 1.0        */
/*		 03-Mar-2008			 Ver 1.1        Sandeep Patil			 IBM CHANGES				*/
/*     23-Feb-2012       Ver 1.2        Sahcin Birje       FNO BSE Changes    */
/*     16-Aug-2018       Ver 1.4        Suchita Dabir      Physical settlement*/ 
/*                                                         of stock derivative*/
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atmi.h>
#include <fml32.h>
/*Add header files here */
#include <fo_fml_def.h>
#include <fo.h>
#include <fn_log.h>
#include <fo_view_def.h>
#include <fn_tuxlib.h>
#include <fn_ddr.h>
#include <fn_read_debug_lvl.h>			/*** Ver 1.1 ***/

void SFO_CNT_DWLD( TPSVCINFO *rqst )
{

	FBFR32 *ptr_fml_Ibuf;

	char c_ServiceName [33];
  char c_err_msg [256];
	int i_returncode;
	int i_ip_len;
	int i_op_len;
	char c_tag [ 256 ];
	char c_msg [ 256 ];
	long int li_rec_no;

	struct vw_mtch_cntrct st_mtch_cntrct;
	struct vw_cntrt_gen_inf st_cntrt_gen_inf;
	struct vw_contract st_contract;

	ptr_fml_Ibuf = ( FBFR32 * )rqst->data;
	strcpy( c_ServiceName, rqst->name );

	INITDBGLVL(c_ServiceName);			 /*** Ver 1.1 ***/

	i_returncode = Fvftos32( ptr_fml_Ibuf,
													 ( char * )&st_mtch_cntrct,
                           "vw_mtch_cntrct" );
	if( i_returncode == -1 )
	{
    fn_errlog( c_ServiceName, "S31005", FMLMSG, c_err_msg );
		Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn( TPFAIL, ERR_BFR, ( char * )ptr_fml_Ibuf, 0, 0 );		
	}

  /*** Added for Order Routing ***/
  fn_init_ddr_val ( st_mtch_cntrct.c_rout_crt );
   
  if( strcmp(st_mtch_cntrct.c_xchng_cd,"NFO") == 0 ) /** Ver 1.2 ***/
  {	
   i_returncode = fn_unpack_fmltovar ( c_ServiceName,
                                      c_err_msg,
																		  ptr_fml_Ibuf,
																		  6, /** In ver 1.4,5 changed to 6 ***/
					 FFO_MIN_LOT_QTY, ( char * )&st_cntrt_gen_inf.l_min_lot_qty, NULL,
					 FFO_TCK_SZ, (char * )&st_cntrt_gen_inf.l_tick_sz, NULL,
					 FFO_MULTPLR, ( char * )&st_cntrt_gen_inf.l_multplr, NULL,
					 FFO_BAT_TAG, (char * )c_tag, NULL,
					 FFO_REC_NO, (char * )&li_rec_no, NULL,
           FFO_ND_STRT_DT,(char * )st_cntrt_gen_inf.c_nd_strt_dt,NULL /** added in ver 1.4 ***/ 
           );
  }
  else if( strcmp(st_mtch_cntrct.c_xchng_cd,"BFO") == 0 ) /*** Ver 1.2 starts here ***/
  {
   i_returncode = fn_unpack_fmltovar ( c_ServiceName,
                                      c_err_msg,
                                      ptr_fml_Ibuf,
                                      7,
           FFO_MIN_LOT_QTY, ( char * )&st_cntrt_gen_inf.l_min_lot_qty, NULL,
           FFO_TCK_SZ, (char * )&st_cntrt_gen_inf.l_tick_sz, NULL,
           FFO_MULTPLR, ( char * )&st_cntrt_gen_inf.l_multplr, NULL,
           FFO_BAT_TAG, (char * )c_tag, NULL,
           FFO_REC_NO, (char * )&li_rec_no, NULL,
           FFO_EBA_CNTRCT_ID, (char * )&st_cntrt_gen_inf.l_setl_stlmnt_no, NULL,
           FFO_IMTM_RMRKS, (char * )st_cntrt_gen_inf.c_bk_cls_strt_dt,NULL 
           );
     

  
  } /*** Ver 1.2 Ends Here ***/
	if( i_returncode != 0 )
	{
		fn_errlog( c_ServiceName, "S31010", LIBMSG , c_err_msg );
		Fadd32(ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn( TPFAIL, ERR_BFR, ( char * )ptr_fml_Ibuf, 0, 0 );
	}
																
	i_ip_len = sizeof( struct vw_mtch_cntrct );
	i_op_len = sizeof( struct vw_contract );

	fn_cpy_ddr(st_mtch_cntrct.c_rout_crt);
	st_mtch_cntrct.c_rqst_typ = MATCH_ID_TO_CONTRACT;

	if (DEBUG_MSG_LVL_3)			 /*** Ver 1.1 ***/ 
	{
		fn_userlog( c_ServiceName, "c_xchng_cd [%s]",st_mtch_cntrct.c_xchng_cd );
		fn_userlog( c_ServiceName, "c_instrmnt_typ [%s]",st_mtch_cntrct.c_instrmnt_typ );
		fn_userlog( c_ServiceName, "d_strike_prc:%lf", st_mtch_cntrct.d_strike_prc );
		fn_userlog( c_ServiceName, "c_symbol [%s]",st_mtch_cntrct.c_symbol );
		fn_userlog( c_ServiceName, "c_expry_dt [%s]",st_mtch_cntrct.c_expry_dt );
		fn_userlog( c_ServiceName, "c_opt_typ [%s]",st_mtch_cntrct.c_opt_typ );
		fn_userlog( c_ServiceName, "l_ca_lvl [%ld]",st_mtch_cntrct.l_ca_lvl );  
		fn_userlog( c_ServiceName, "c_rqst_typ [%c]",st_mtch_cntrct.c_rqst_typ );
    fn_userlog( c_ServiceName, " l_scrip_id [%ld]",st_cntrt_gen_inf.l_setl_stlmnt_no);
    fn_userlog( c_ServiceName,"st_cntrt_gen_inf.c_nd_strt_dt [%s]",st_cntrt_gen_inf.c_nd_strt_dt); /** added in ver 1.4 ***/
  }

	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              &st_mtch_cntrct,
                              &st_contract,
                              "vw_mtch_cntrct",
                              "vw_contract",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_CNT_FOR_ENT" );
	if( i_returncode != SUCC_BFR )
	{
		fn_errlog( c_ServiceName, "S31015", LIBMSG, c_err_msg );
		Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
    sprintf ( c_msg, "%ld - %s", li_rec_no, c_err_msg );
    fn_bat_pst_msg_fno ( c_ServiceName, c_msg, c_tag );
		tpreturn( TPFAIL, ERR_BFR, ( char * )ptr_fml_Ibuf, 0, 0 );
	}

	strcpy(st_cntrt_gen_inf.c_xchng_cd, st_contract.c_xchng_cd );
	strcpy(st_cntrt_gen_inf.c_undrlyng, st_contract.c_undrlyng );
	strcpy(st_cntrt_gen_inf.c_expry_dt, st_contract.c_expry_dt );
	st_cntrt_gen_inf.l_eba_cntrct_id = st_contract.l_eba_cntrct_id;
	st_cntrt_gen_inf.c_prd_typ = st_contract.c_prd_typ;
	st_cntrt_gen_inf.c_opt_typ = st_contract.c_opt_typ;
	st_cntrt_gen_inf.l_strike_prc = st_contract.l_strike_prc;
	st_cntrt_gen_inf.c_ctgry_indstk = st_contract.c_ctgry_indstk;
	st_cntrt_gen_inf.l_ca_lvl = st_contract.l_ca_lvl;
	st_cntrt_gen_inf.c_exrc_typ = st_contract.c_exrc_typ;
	fn_cpy_ddr(st_cntrt_gen_inf.c_rout_crt);
	st_cntrt_gen_inf.c_rqst_typ = UPDATE_CONTRACT_DOWNLOAD;

	if (DEBUG_MSG_LVL_3)			 /*** Ver 1.1 ***/ 
	{
		fn_userlog( c_ServiceName, "st_cntrt_gen_inf.l_strike_prc:%ld",
																					st_cntrt_gen_inf.l_strike_prc );  
		fn_userlog( c_ServiceName, "exercise type %c",st_cntrt_gen_inf.c_exrc_typ );
	}


	i_ip_len = sizeof( struct vw_cntrt_gen_inf );

	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
   														&st_cntrt_gen_inf,
   														&st_cntrt_gen_inf,
															"vw_cntrt_gen_inf",
															"vw_cntrt_gen_inf",
 															i_ip_len,
 															i_ip_len,
															0,
															"SFO_UPD_CNTRCT" );

	if( i_returncode != SUCC_BFR )
	{
		fn_errlog( c_ServiceName, "S31020", LIBMSG , c_err_msg );
		Fadd32(ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
    sprintf ( c_msg, "%ld - %s", li_rec_no, c_err_msg );
    fn_bat_pst_msg_fno ( c_ServiceName, c_msg, c_tag );
		tpreturn( TPFAIL, ERR_BFR, ( char * )ptr_fml_Ibuf, 0, 0 );
	}

	tpreturn(TPSUCCESS, SUCC_BFR, NULL, 0, 0 );

}

