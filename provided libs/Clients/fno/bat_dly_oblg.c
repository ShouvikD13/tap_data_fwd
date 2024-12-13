/*** ver 1.1 IBM Migration Changes sandeep ***/
/*** Ver 1.2 Code Optimization | Sachin Birje ***/

#include <atmi.h>
#include <string.h>				/*** Ver 1.1 ***/
#include <fml32.h>
#include <fo.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fn_battmpt.h>
#include <fml_def.h>
#include <fo_fml_def.h>
#include <fn_read_debug_lvl.h>			/*** Ver 1.1 ***/


void BAT_DLY_OBLG ( int argc, char *argv[] )
{
	long int li_tot_rcrds;
  int i_ch_val;
  long int li_cnt;
  char c_msg[256];
	FBFR32 *ptr_fml_buf;
  char c_ServiceName [ 33 ];
  char c_err_msg [ 256 ];
  char c_xchng_cd[21];
  char c_trd_dt[12];
  char c_eba_mtch_act_no[11];
  char c_client_cd[11];
  char c_bill[51];
  double d_option_premium;
  double d_stock_option_ex;
  double d_index_option_ex;
  double d_stock_option_asmnt;
  double d_index_option_asmnt;
  double d_eod_mtm;
  long	 l_brokerage;
  double d_sttlmnt_frn;

  strcpy ( c_ServiceName, argv[0] );

	INITBATDBGLVL(argv[0]);			/*** Ver 1.1 ***/

	sprintf ( c_msg, "Batch process %s started", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

  strcpy ( c_xchng_cd, argv[2] );
  strcpy ( c_trd_dt, argv[4] );
  strcpy ( c_eba_mtch_act_no, argv[5] );


	strcpy ( c_msg, "Creating data. Please wait ... " );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

	ptr_fml_buf = fn_create_rs ( c_ServiceName,
                               c_err_msg,
                               "SFO_GETDLYOBL",
                               TPNOTIME,
															 3,
															 FFO_XCHNG_CD,(char *)c_xchng_cd ,
															 FFO_TRD_DT,(char *)c_trd_dt ,
															 FFO_EBA_MTCH_ACT_NO,(char *)c_eba_mtch_act_no
                               );
	if ( ptr_fml_buf == NULL )
	{
		fn_errlog( c_ServiceName, "S31005", LIBMSG, c_err_msg );
		return;
	}

	li_tot_rcrds = fn_reccnt_rs ( ptr_fml_buf,
                                c_ServiceName,
                                c_err_msg,
																FFO_BILL_NO);
	if ( li_tot_rcrds == SYSTEM_ERROR )
	{
		fn_errlog( c_ServiceName, "S31010", LIBMSG, c_err_msg );
		fn_dstry_rs ( ptr_fml_buf );
		return;
	}

	strcpy ( c_msg, "Data created." );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
  sprintf ( c_msg, "Total records = %ld", li_tot_rcrds );
  fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

  for ( li_cnt=1; li_cnt<=li_tot_rcrds; li_cnt++ )
  {
		i_ch_val = fn_getnxt_rs ( ptr_fml_buf,
                              c_ServiceName,
                              c_err_msg,
															10,
 															FFO_BILL_NO,(char *)c_bill,NULL,
 															FFO_OPT_PREMIUM,(char *)&d_option_premium,NULL,
 															FFO_STKOPT_EXRC,(char *)&d_stock_option_ex,NULL,
 															FFO_INDOPT_EXRC,(char *)&d_index_option_ex,NULL,
 															FFO_STKOPT_ASGN,(char *)&d_stock_option_asmnt,NULL,
 															FFO_INDOPT_ASGN,(char *)&d_index_option_asmnt,NULL,
 															FFO_EODMTM_FUT,(char *)&d_eod_mtm,NULL,
 															FFO_BRKRG_VL,(char *)&l_brokerage,NULL,
 															FFO_CSHSTTL_FRCTN,(char *)&d_sttlmnt_frn,NULL,
 															FFO_EBA_MTCH_ACT_NO,(char *)c_client_cd,NULL
                              );
		if ( i_ch_val != 0 )
		{
			fn_errlog( c_ServiceName, "S31015", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
		}
		if(li_cnt % 50 == 1)
		{
			sprintf ( c_msg, "Processing record %ld of %ld", li_cnt, li_tot_rcrds );
 			fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
		}

    i_ch_val = fn_pre_prcs_rcrd ( c_ServiceName,
                                  c_err_msg,
                                /***  "sfr_beod_mtchgt",** Ver 1.2 Wrong server name passed ***/
                                  "sfr_beod_prcs",      /*** Ver 1.2 ***/
                                  li_cnt,
                                  li_tot_rcrds );
    if ( i_ch_val == SYSTEM_ERROR )
    {
			fn_errlog( c_ServiceName, "S31020", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
    }

		if (DEBUG_MSG_LVL_3)			/*** Ver 1.1 ***/
		{
			fn_userlog(c_ServiceName,"FFO_BRKRG_VL is :%ld:",l_brokerage);
			fn_userlog(c_ServiceName," INPUT FFO_EBA_MTCH_ACT_NO is :%s:",c_eba_mtch_act_no);
			fn_userlog(c_ServiceName,"FFO_OPT_PREMIUM is :%lf:",d_option_premium);
			fn_userlog(c_ServiceName,"FFO_STKOPT_EXRC is :%lf:",d_stock_option_ex);
			fn_userlog(c_ServiceName,"FFO_INDOPT_EXRC is :%lf:",d_index_option_ex);
			fn_userlog(c_ServiceName,"FFO_STKOPT_ASGN is :%lf:",d_stock_option_asmnt);
			fn_userlog(c_ServiceName,"FFO_INDOPT_ASGN is :%lf:",d_index_option_asmnt);
			fn_userlog(c_ServiceName,"FFO_EODMTM_FUT is :%lf:",d_eod_mtm);
			fn_userlog(c_ServiceName,"FFO_CSHSTTL_FRCTN is :%lf:",d_sttlmnt_frn);
			fn_userlog(c_ServiceName," OUTPUT FFO_EBA_MTCH_ACT_NO is :%s:",c_client_cd);
		}

    i_ch_val = fn_acall_svc_fml ( c_ServiceName,
                                  c_err_msg,
                                  "SFO_DLY_OBLG",
                                  0,
																	14,
																	FFO_XCHNG_CD,(char *)c_xchng_cd ,
																	FFO_TRD_DT,(char *)c_trd_dt ,
 																	FFO_BILL_NO,(char *)c_bill,
 																	FFO_OPT_PREMIUM,(char *)&d_option_premium,
 																	FFO_STKOPT_EXRC,(char *)&d_stock_option_ex,
 																	FFO_INDOPT_EXRC,(char *)&d_index_option_ex,
 																	FFO_STKOPT_ASGN,(char *)&d_stock_option_asmnt,
 																	FFO_INDOPT_ASGN,(char *)&d_index_option_asmnt,
 																	FFO_EODMTM_FUT,(char *)&d_eod_mtm,
 																	FFO_BRKRG_VL,(char *)&l_brokerage,
 																	FFO_CSHSTTL_FRCTN,(char *)&d_sttlmnt_frn,
 																	FFO_EBA_MTCH_ACT_NO,(char *)c_client_cd,
                                  FFO_BAT_TAG, ( char * )c_tag,
                                  FFO_REC_NO, ( char * )&li_cnt );
    if ( i_ch_val == SYSTEM_ERROR )
    {
			fn_errlog( c_ServiceName, "S31025", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
		}
  }

	fn_dstry_rs ( ptr_fml_buf );

	sprintf ( c_msg, "Batch process %s ended", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

  return;
}

int fn_bat_init ( int argc, char *argv[] ) 
{
	fn_var = BAT_DLY_OBLG;
	return 0;
}

void fn_bat_term ( int argc , char *argv[])
{
	return;
}

int fn_prcs_trg ( char *c_pgm_nm,
                  int i_trg_typ )
{
	return 0;
}
