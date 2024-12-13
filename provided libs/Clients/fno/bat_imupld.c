/* Ver 1.1 ranjit 19-Dec-2007 IBM Migration */
/* Ver 1.2 Code Optimization | Sachin Birje */
/* Ver 1.3 Error Handling 12-Dec-2016 | Samip M */
/* Ver 1.4 Console Message 05-Sep-2016 | Tanmay Patel */

#include <atmi.h>
#include <fml32.h>
#include <fo.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fn_battmpt.h>
#include <fo_fml_def.h>
#include <string.h>		/* ver 1.1  */
#include <fn_read_debug_lvl.h>	/* ver 1.1 */
int Z ;
#define INIT(x,y) for(Z=0;Z<y;Z++)x[Z]=0

void BAT_IMUPLD ( int argc, char *argv[] )
{
	long int li_tot_rcrds;
  int i_ch_val;
  long int li_cnt;
  char c_msg[256];
	FBFR32 *ptr_fml_buf;
  char c_ServiceName [ 33 ];
  char c_err_msg [ 256 ];

	char 		c_xchng_cd[4];
	char 		c_trd_dt[12]; 
	char 		c_clm_mtch_accnt[11];
	double 		d_amount = 0;
  double	d_bal_amt = 0;

  strcpy ( c_ServiceName, argv[0] );
	INITBATDBGLVL(argv[0]);		/* ver 1.1 */
  strcpy ( c_xchng_cd, argv[2] );
  strcpy ( c_trd_dt, argv[4] );

	sprintf ( c_msg, "Batch process %s started", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

	strcpy ( c_msg, "Creating data. Please wait ... " );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
	INIT(c_clm_mtch_accnt ,11);

	ptr_fml_buf = fn_create_rs ( c_ServiceName,
                               c_err_msg,
                               "SFO_GT_IM_DTLS",
                               TPNOTIME,
                               3,                    /*** Change from 2 to 3 in Ver 1.4 ***/
															 FFO_XCHNG_CD, (char *)c_xchng_cd ,
															 FFO_TRD_DT , (char *)c_trd_dt ,
                               FFO_BAT_TAG , (char *)c_tag);    /***Added in  Ver 1.4 ***/
	if ( ptr_fml_buf == NULL )
	{
		fn_errlog( c_ServiceName, "S31005", LIBMSG, c_err_msg );
 		strcpy ( c_msg, "System error. Contact system support" );		/** Ver 1.3 **/
 		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );							/** Ver 1.3 **/
		return;
	}


	li_tot_rcrds = fn_reccnt_rs ( ptr_fml_buf,
                                c_ServiceName,
                                c_err_msg,
                                FFO_XCHNG_CD);
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
															5,
                              FFO_XCHNG_CD, (char *)c_xchng_cd ,NULL ,
															FFO_TRD_DT, (char *)c_trd_dt ,NULL ,
															FFO_EBA_MTCH_ACT_NO , (char *)c_clm_mtch_accnt,NULL ,
															FFO_AMOUNT,(char *)&d_amount ,NULL ,
															FFO_BAL_AMT, (char *)&d_bal_amt ,NULL
 															);
		if ( i_ch_val == SYSTEM_ERROR )
		{
			fn_errlog( c_ServiceName, "S31015", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
		}
	
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"FFO_XCHNG_CD is :%s:",c_xchng_cd);
			fn_userlog(c_ServiceName,"FFO_TRD_DT is :%s:",c_trd_dt);
			fn_userlog(c_ServiceName,"FFO_EBA_MTCH_ACT_NO is :%s:",c_clm_mtch_accnt);
			fn_userlog(c_ServiceName,"FFO_AMOUNT is :%lf:",d_amount);
			fn_userlog(c_ServiceName,"FFO_BAL_AMT is :%lf:",d_bal_amt);
		}

    if(li_cnt % 10 == 1)
    {
      sprintf ( c_msg, "Processing record %ld of %ld", li_cnt, li_tot_rcrds );
      fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
    }

    i_ch_val = fn_pre_prcs_rcrd ( c_ServiceName,
                                  c_err_msg,
                             /***     "sfr_beod_gtdt", *** Ver 1.2 ** Wrong server name passed ***/
                                  "sfr_beod_ldprc",   /*** Ver 1.2 ***/
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
	fn_userlog(c_ServiceName,"8888");

    i_ch_val = fn_acall_svc_fml ( c_ServiceName,
                                  c_err_msg,
                                  "SFO_GETIMDTLS",
                                  0,
																	8,
                              		FFO_XCHNG_CD, (char *)c_xchng_cd ,
																	FFO_TRD_DT, (char *)c_trd_dt ,
																	FFO_EBA_MTCH_ACT_NO , (char *)c_clm_mtch_accnt,
																	FFO_AMOUNT,(char *)&d_amount ,
																	FFO_BAL_AMT, (char *)&d_bal_amt,
                                  FFO_BAT_TAG, ( char * )c_tag,
                                  FFO_REC_NO, ( char * )&li_cnt ,
                                  FFO_BAT_TAG, ( char * )c_tag );
    if ( i_ch_val == SYSTEM_ERROR )
    {
			fn_errlog( c_ServiceName, "S31025", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
		}
	fn_userlog(c_ServiceName,"9999");
  }

	fn_dstry_rs ( ptr_fml_buf );

	sprintf ( c_msg, "Batch process %s ended", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

  return;
}

int fn_bat_init ( int argc, char *argv[] ) 
{
	fn_var = BAT_IMUPLD;
	return 0;
}

void fn_bat_term ( int argc,char *argv[] )
{
	return;
}

int fn_prcs_trg ( char *c_pgm_nm,
                  int i_trg_typ )
{
	return 0;
}
