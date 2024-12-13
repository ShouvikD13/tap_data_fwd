/* Ver 1.1 20-Dec-2007 IBM Migration Ranjit */
/* Ver 1.2 13-Jul-2009 CRCON31510; All option Vidyayini Krish */


#include <atmi.h>
#include <fml32.h>
#include <fo.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fn_battmpt.h>
#include <fo_fml_def.h>
#include <fn_ddr.h>
#include <string.h>		/* ver 1.1 */
#include <stdlib.h>		/* ver 1.1 */
#include <fn_read_debug_lvl.h>	/* ver 1.1 */

void BAT_RECAL_OPTTP ( int argc, char *argv[] )
{
	long int li_tot_rcrds;
  int i_ch_val;
  long int li_cnt;
  char c_msg[256];
	FBFR32 *ptr_fml_buf;
  char c_ServiceName [ 33 ];
  char c_err_msg [ 256 ];

  char c_xchng_cd [ 4 ];
  char c_exp_dt [ LEN_DATE ];
  char c_cln_mtch_accnt [ 11 ];
  char c_undrlyng [ 256 ];

  char c_prd_typ;
	char c_rqst_typ[2];   /* 1.2 */
	char c_request_typ;   /* 1.2 */
  char c_exrc_typ;
  char c_ind_stk;
	char c_opt_typ;
	long l_strk_prc;

  strcpy ( c_ServiceName, argv[0] );
	
	INITBATDBGLVL(argv[0]);	/* ver 1.1 */

	sprintf ( c_msg, "Batch process %s started", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

	strcpy ( c_msg, "Creating data. Please wait ... " );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

  if( argc < 5 ) 
  {
    fn_bat_pst_msg ( c_ServiceName, "Invalid Number of Arguments", c_tag );
    exit(0);
  }

  strcpy( c_xchng_cd, argv[2] );

  /* strcpy( c_undrlyng, argv[4] ); */

	/* 1.2 Starts */

	memset( c_rqst_typ,'\0',sizeof(c_rqst_typ) );
	strcpy( c_rqst_typ, argv[4] );

  if ( strcmp(c_rqst_typ,"A") == 0 ) /* All underlying */
	{
	  strcpy( c_undrlyng,"%" );	
		c_request_typ = 'A';
	}
	else if ( strcmp(c_rqst_typ,"S") == 0 ) /* Single */
	{
	   if( argc < 6 )
		 {
			 fn_bat_pst_msg ( c_ServiceName, "Invalid Number of Arguments", c_tag );
       return;	
		 }	
     c_request_typ = 'S';
		 strcpy( c_undrlyng, argv[5] );
	}
	else if ( strcmp(c_rqst_typ,"M") == 0 ) /* Multiple */
  {
     if( argc < 6 )
     {
       fn_bat_pst_msg ( c_ServiceName, "Invalid Number of Arguments", c_tag );
       return;
     }
     c_request_typ = 'M';
     strcpy( c_undrlyng, argv[5] );
  }
	else
	{
		fn_bat_pst_msg ( c_ServiceName, "Invalid Request type", c_tag );
		return;
	}

	/* 1.2 Ends   */

	/* 1.2 In the below call the number of FMLs increased from 2 to 3 and the new FML was added */

	ptr_fml_buf = fn_create_rs ( c_ServiceName,
                               c_err_msg,
                               "SFO_GT_OTPCALRC",
                               TPNOTIME,
															 3,                                  
                               FFO_XCHNG_CD, ( char * )c_xchng_cd,
                               FFO_UNDRLYNG, ( char * )c_undrlyng,
															 FFO_RQST_TYP, (char *)&c_request_typ
														 );

	if ( ptr_fml_buf == NULL )
	{
		fn_errlog( c_ServiceName, "S31005", LIBMSG, c_err_msg );
		return;
	}

	li_tot_rcrds = fn_reccnt_rs ( ptr_fml_buf,
                                c_ServiceName,
                                c_err_msg,
                                FFO_EBA_MTCH_ACT_NO );
	if ( li_tot_rcrds == -1 )
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
															9,
														FFO_EBA_MTCH_ACT_NO,( char *)c_cln_mtch_accnt,NULL,
														FFO_XCHNG_CD,( char * )c_xchng_cd, NULL,
														FFO_PRDCT_TYP,( char * )&c_prd_typ, NULL,
														FFO_CTGRY_INDSTK,( char * )&c_ind_stk, NULL,
														FFO_UNDRLYNG,( char * )c_undrlyng, NULL,
														FFO_EXPRY_DT,( char * )c_exp_dt, NULL,
														FFO_EXER_TYP,( char * )&c_exrc_typ, NULL,
														FFO_OPT_TYP,( char * )&c_opt_typ, NULL,
                            FFO_STRK_PRC, ( char * )&l_strk_prc,NULL );

		if ( i_ch_val != 0 )
		{
			fn_errlog( c_ServiceName, "S31015", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
		}

		sprintf ( c_msg, "Processing record %ld of %ld", li_cnt, li_tot_rcrds );
 		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

    i_ch_val = fn_pre_prcs_rcrd ( c_ServiceName,
                                  c_err_msg,
                                  "sfr_beod_gtdt",
                                  li_cnt,
                                  li_tot_rcrds );
    if ( i_ch_val == -1 )
    {
			fn_errlog( c_ServiceName, "S31020", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
    }

    i_ch_val = fn_acall_svc_fml ( c_ServiceName,
                                  c_err_msg,
                                  "SFO_PRCS_OTPCAL",
                                  0,
                                  11,
																	FFO_EBA_MTCH_ACT_NO,( char *)c_cln_mtch_accnt,
																	FFO_XCHNG_CD,( char * )c_xchng_cd,
																	FFO_PRDCT_TYP,( char * )&c_prd_typ,
																	FFO_CTGRY_INDSTK,( char * )&c_ind_stk, 
																	FFO_UNDRLYNG,( char * )c_undrlyng,
																	FFO_EXPRY_DT,( char * )c_exp_dt,
																	FFO_EXER_TYP,( char * )&c_exrc_typ,
																	FFO_OPT_TYP,( char * )&c_opt_typ,
                            			FFO_STRK_PRC, ( char * )&l_strk_prc,
                                  FFO_BAT_TAG, ( char * )c_tag,
                                  FFO_REC_NO, ( char * )&li_cnt );

    if ( i_ch_val == -1 )
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
	fn_var = BAT_RECAL_OPTTP;
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
