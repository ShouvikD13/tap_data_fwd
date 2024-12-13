/*** ver 1.1 IBM Migration Changes sandeep ***/
/*** ver 1.2 FNO PIPO Run No Passing (Sachin Birje) ***/

#include <atmi.h>
#include <fml32.h>
#include <fo.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fn_battmpt.h>
#include <fml_def.h>
#include <fo_fml_def.h>
#include <string.h>			/*** Ver 1.1 * **/
#include <fn_read_debug_lvl.h>      /*** Ver 1.1 ***/

void BAT_BILL_DTLS ( int argc, char *argv[] ) 
{ 
	long int li_tot_rcrds;
  int i_ch_val;
  long int li_cnt;
  long l_run_no=0;   /** Ver 1.2 **/

  char c_msg[256];
	FBFR32 *ptr_fml_buf;
  char c_ServiceName [ 33 ];
  char c_err_msg [ 256 ];

	char c_xchng_cd [ 4 ];
	char c_trd_dt [ LEN_DATE ];
	char c_bnk_accnt [ 12 ];
  char c_bill_no [ 50 ];
  double d_pipo_amt;
  double d_int_adj_amt;
  char c_intadj_rfrnc [ 50 ];
  char c_pipo_rfrnc [ 50 ];

  strcpy ( c_ServiceName, argv[0] );

	INITBATDBGLVL(argv[0]);     /*** Ver 1.1 ***/

	sprintf ( c_msg, "Batch process %s started", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

	strcpy ( c_msg, "Creating data. Please wait ... " );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

	if( argc < 5 )
	{
   sprintf ( c_msg, "syntax: %s <ini> <NFO> <NA> <trd dt>", argv[0] );
   printf("%s\n\n",c_msg);
   fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
   fn_errlog( c_ServiceName, "S31005", LIBMSG, c_err_msg );
   return;
  }

	strcpy( c_xchng_cd, argv[2] );
	strcpy( c_trd_dt, argv[4] );

	ptr_fml_buf = fn_create_rs ( c_ServiceName,
                               c_err_msg,
                               "SFO_GT_BILLDTLS",
                               TPNOTIME,
															 2,
															 FFO_XCHNG_CD, ( char * )c_xchng_cd, 
                               FFO_TRD_DT, (char * )c_trd_dt );
	if ( ptr_fml_buf == NULL )
	{
		fn_errlog( c_ServiceName, "S31010", LIBMSG, c_err_msg );
		return;
	}

	li_tot_rcrds = fn_reccnt_rs ( ptr_fml_buf,
                                c_ServiceName,
                                c_err_msg,
																FFO_BILL_NO );
	if ( li_tot_rcrds == -1 )
	{
		fn_errlog( c_ServiceName, "S31015", LIBMSG, c_err_msg );
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
															9,                /** Ver 1.2, changed 8 to 9 **/
															FFO_XCHNG_CD, ( char * )c_xchng_cd, NULL,
                              FFO_TRD_DT, (char * )c_trd_dt, NULL, 
															FFO_BILL_NO,(char *)c_bill_no, NULL,
                              FFO_PI_PO_AMT, ( char * )&d_pipo_amt, NULL,
                              FFO_INT_ADJ_AMT, ( char * )&d_int_adj_amt, NULL,
															FFO_INTADJ_RFRNC, ( char * )c_intadj_rfrnc, NULL,
															FFO_BNK_ACT_NO, ( char * )c_bnk_accnt, NULL,
                              FFO_PIPO_RFRNC, ( char * )c_pipo_rfrnc, NULL, 
                              FFO_PRCS_NO, ( char * )&l_run_no, NULL );  /*** Ver 1.2 **/
		if ( i_ch_val != 0 )
		{
			fn_errlog( c_ServiceName, "S31020", LIBMSG, c_err_msg );
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
                                  "sfr_beod_gtdt",
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

    i_ch_val = fn_acall_svc_fml ( c_ServiceName,
                                  c_err_msg,
                                  "SFO_PRCS_BILL",
                                  0,
																	11,     /** Ver 1.2, Changed from 10 to 11 **/
																	FFO_XCHNG_CD, ( char * )c_xchng_cd, 
                              		FFO_TRD_DT, (char * )c_trd_dt, 
                              		FFO_BILL_NO,(char *)c_bill_no,
                              		FFO_PI_PO_AMT, ( char * )&d_pipo_amt,
                              		FFO_INT_ADJ_AMT, ( char * )&d_int_adj_amt,
                              		FFO_INTADJ_RFRNC, ( char * )c_intadj_rfrnc,
                              		FFO_BNK_ACT_NO, ( char * )c_bnk_accnt,
                              		FFO_PIPO_RFRNC, ( char * )c_pipo_rfrnc,
																	FFO_BAT_TAG, ( char * )c_tag,
																	FFO_REC_NO, ( char * )&li_cnt, 
																	FFO_PRCS_NO, ( char * )&l_run_no ); /*** Ver 1.2 ** l_run_no Added **/
																	
    if ( i_ch_val == -1 )
    {
			fn_errlog( c_ServiceName, "S31030", LIBMSG, c_err_msg );
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
	fn_var = BAT_BILL_DTLS;
	return 0;
}

void fn_bat_term ( int argc, char *argv[] )
{
	return;
}

int fn_prcs_trg ( char *c_pgm_nm, int i_trg_typ )
{
	return 0;
}
