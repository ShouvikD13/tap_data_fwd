/*************************************************************************************/
/*    Program               : BAT_CONV_TO_FUT                                        */
/*                                                                                   */
/*    Input                 : Exchange_code                                          */
/*                          : Trade_date                                             */
/*													: Match_Acc																							 */
/*													: Underlying																						 */
/*													: Contract_Tag																					 */
/*                                                                                   */
/*    Output                :                                                        */
/*                                                                                   */
/*    Description           : This batch program picks all open futureplus positions */
/*                            and converts it to the future position one bye one.    */
/*                                                                                   */
/*    Log                   : 1.0   12-Mar-2008 Shailesh Hinge | Kunal Jadav         */
/*													: 1.1   25-Apr-2008 Sandeep Patil                        */
/*													: 1.2   09-Sep-2014 Samip M															 */
/*************************************************************************************/
/*    1.0 - New release                                                              */
/*    1.1 - IBM Migration Changes																										 */
/*    1.2 - View to FML changes																											 */
/*************************************************************************************/
#include <sqlca.h>
#include <string.h>
#include <atmi.h>
#include <fml32.h>
#include <fo.h>
#include <fn_tuxlib.h> 
#include <fn_battmpt.h>
#include <fml_def.h>
#include <fo_fml_def.h>
/** #include <fo_view_def.h> **/	/** Ver 1.2 **/
#include <fn_ddr.h>
#include <fn_log.h>
#include <fn_read_debug_lvl.h>			/*** Ver 1.1 ***/

void BAT_CONV_TO_FUT( int argc, char* argv[])
{
  char c_ServiceName[33];
  char c_err_msg[256];	
	char c_msg[256];
	char c_xchng_cd[4];
	char c_trd_dt[ LEN_DATE ];
	char c_prd_typ;
	char c_clm_mtch_acnt [ 11 ];
	char c_undrlyng [ 7 ];
	char c_indstk;
	char c_cntrct_tag;
	char c_spl_flg='S';
	char c_rout_str [ 4 ];
	char c_svc_name [ 33 ];
	char c_user_id[9];
  char c_channel[4];

	int i_trnsctn;
	int i_ip_len;
	int i_op_len;
	int i_returncode;
	int i_ch_val;
	long int li_tot_rcrds;
	long int li_cnt;
  long l_sssn_id;

	FBFR32 *ptr_fml_buf;

	strcpy( c_ServiceName, argv[0] );

  INITBATDBGLVL(argv[0]);     /*** Ver 1.1 ***/

  if( argc < 5 )
  {
    fn_userlog( "Usage: %s <ini> <Exchange_code> <NA> <Mtch_Acc> <Undrlyng> <ContractTag>", argv[0] );
    return;
  }

	sprintf ( c_msg, "Batch process %s started", argv[0] );
  fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
	
  strcpy ( c_msg, "Creating data. Please wait ... " );
  fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
	
	strcpy( c_xchng_cd, argv[2] );
	strcpy( c_clm_mtch_acnt, argv[4] );
	strcpy( c_undrlyng, argv[5] );
	c_prd_typ = 'P';
	strcpy(c_user_id,"system");
	strcpy(c_channel,"SYS");
	l_sssn_id = 0;

	fn_userlog(c_ServiceName,"c_xchng_cd:%s:",c_xchng_cd);
	fn_userlog(c_ServiceName,"c_clm_mtch_acnt:%s:",c_clm_mtch_acnt);
	fn_userlog(c_ServiceName,"c_undrlyng:%s:",c_undrlyng);

	if( argc > 6 )
	{
		c_cntrct_tag =argv[6][0];
		fn_userlog(c_ServiceName,"c_cntrct_tag:%c:",c_cntrct_tag);
		
		ptr_fml_buf = fn_create_rs (  c_ServiceName,
																	c_err_msg,
																	"SFO_GT_FUTPLUS",
																	TPNOTIME,
																	5,
																	FFO_XCHNG_CD, ( char *)c_xchng_cd,
																	FFO_PRDCT_TYP, (char *)&c_prd_typ,
																	FFO_EBA_MTCH_ACT_NO,( char *)c_clm_mtch_acnt,
																	FFO_UNDRLYNG, (char *)c_undrlyng,
																	FFO_CNTRCT_TAG, (char *)&c_cntrct_tag);

		if ( ptr_fml_buf == NULL )
		{
			fn_errlog( c_ServiceName, "S31005", LIBMSG, c_err_msg );
			return;
  	}
	}
	else
	{
		c_cntrct_tag = '*';
		fn_userlog(c_ServiceName,"c_cntrct_tag:%c:",c_cntrct_tag);
		ptr_fml_buf = fn_create_rs (  c_ServiceName,
																	c_err_msg,
																	"SFO_GT_FUTPLUS",
																	TPNOTIME,
																	5,
																	FFO_XCHNG_CD, ( char *)c_xchng_cd,
																	FFO_PRDCT_TYP, (char *)&c_prd_typ,
																	FFO_EBA_MTCH_ACT_NO,( char *)c_clm_mtch_acnt,
																	FFO_UNDRLYNG, (char *)c_undrlyng,
																	FFO_CNTRCT_TAG, (char *)&c_cntrct_tag);	

		if ( ptr_fml_buf == NULL )
		{
			fn_errlog( c_ServiceName, "S31010", LIBMSG, c_err_msg );
			return;
  	}
	}

	li_tot_rcrds = fn_reccnt_rs ( ptr_fml_buf,
                                c_ServiceName,
                                c_err_msg,
                                FFO_EBA_MTCH_ACT_NO);
  if ( li_tot_rcrds == -1 )
  {
    fn_errlog( c_ServiceName, "S31015", LIBMSG, c_err_msg );
    fn_dstry_rs ( ptr_fml_buf );
    return;
  }

	if( DEBUG_MSG_LVL_3 ) 
	{
		fn_userlog( c_ServiceName, "Data created");
		fn_userlog( c_ServiceName, "Total records = %ld", li_tot_rcrds );
	}

	for ( li_cnt=1; li_cnt<=li_tot_rcrds; li_cnt++ )
  {
    i_returncode = fn_getnxt_rs( ptr_fml_buf,
                              	 c_ServiceName,
                              	 c_err_msg,
                              	 6,
                              	 FFO_XCHNG_CD, ( char * )c_xchng_cd, NULL,
                              	 FFO_EBA_MTCH_ACT_NO, ( char * )c_clm_mtch_acnt, NULL,
                              	 FFO_PRDCT_TYP, ( char * )&c_prd_typ, NULL,
                              	 FFO_UNDRLYNG, ( char * )c_undrlyng, NULL,
                              	 FFO_CTGRY_INDSTK, ( char * )&c_indstk, NULL,
                              	 FFO_CNTRCT_TAG, ( char * )&c_cntrct_tag, NULL);
    if ( i_returncode != 0 )
    {
      fn_errlog( c_ServiceName, "S31020", LIBMSG, c_err_msg );
      fn_dstry_rs ( ptr_fml_buf );
			strcpy ( c_msg, "System error. Contact system support" );
      fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
      return;
    }

		sprintf ( c_msg, "Processing record %ld of %ld", li_cnt, li_tot_rcrds );
		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

    i_ch_val = fn_pre_prcs_rcrd ( c_ServiceName,
                                  c_err_msg,
                                  "sfr_fpl_pos",
                                  li_cnt,
                                  li_tot_rcrds );
    if ( i_ch_val == -1 )
    {
      fn_errlog( c_ServiceName, "S31025", LIBMSG, c_err_msg );
      fn_dstry_rs ( ptr_fml_buf );
      strcpy ( c_msg, "System error. Contact system support" );
      fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
      return;
    }

    strcpy( c_svc_name,"SFO_CONV_TO_FUT");

		i_returncode = fn_acall_svc_fml( c_ServiceName,
                                  	 c_err_msg,
                                  	 "SFO_ORD_ROUT",
                                  	 0,
                                  	 13,
                                  	 FFO_XCHNG_CD, ( char * )c_xchng_cd,
                                  	 FFO_EBA_MTCH_ACT_NO,(char *)c_clm_mtch_acnt,
                                  	 FFO_PRDCT_TYP, ( char * )&c_prd_typ,
                                  	 FFO_UNDRLYNG, ( char * )c_undrlyng,
                                  	 FFO_CTGRY_INDSTK, ( char * )&c_indstk,
                                  	 FFO_CNTRCT_TAG, ( char * )&c_cntrct_tag,
																		 FFO_SPL_FLG,(char *)&c_spl_flg,
																		 FFO_USR_ID, (char *) c_user_id,
																		 FFO_SSSN_ID, (char *)&l_sssn_id,
																		 FFO_CHANNEL, (char *) c_channel,
																		 FFO_SVC_NAME, (char *)c_svc_name,
																		 FFO_BAT_TAG,(char *)c_tag,
                                 		 FFO_REC_NO, (char *)&li_cnt);

    if( i_returncode == -1 )
    {
      fn_errlog( c_ServiceName, "S31030", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );	
      return;
    }	
	}
	fn_dstry_rs ( ptr_fml_buf );
	
	if( DEBUG_MSG_LVL_3 ) 
	{	
		fn_userlog(c_ServiceName, "Batch conv_to_fut process Ended");
	}
	
	sprintf ( c_msg, "Batch process %s ended", argv[0] );
  fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
	
	return;
}
int fn_bat_init ( int argc, char *argv[] )
{
  fn_var = BAT_CONV_TO_FUT;
  return 0;
}

void fn_bat_term ( int argc, char *argv[] )
{
  return;
}

int fn_prcs_trg ( char *c_pgm_nm,
                  int i_trg_typ )
{
  return 0;
}
