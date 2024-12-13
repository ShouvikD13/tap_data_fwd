/*** ver 1.1 IBM Migration Changes sandeep ***/
/*** ver 1.2 spread benefit for selective scripts ****By Shamili ***/
/*** ver 1.4 Automation of contract split process ****By Mrinal Kishore(27-Dec-2018) ***/
#include <string.h>			/*** Ver 1.1 ***/			 
#include <stdlib.h>	    /*** Ver 1.1 ***/
#include <atmi.h>
#include <fml32.h>
#include <fo.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fn_battmpt.h>
#include <fo_fml_def.h>
#include <fn_ddr.h>
#include <fml_rout.h>   /*** Ver 1.1 ***/				
#include <fn_read_debug_lvl.h>    /*** Ver 1.1  ***/


void BAT_CNT_SPLT ( int argc, char *argv[] )
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
  char c_trd_dt [ LEN_DATE ];
  char c_cln_mtch_accnt [ 11 ];
  char c_undrlyng [ 7 ];
  char c_rout_crt [ 4 ];
  char c_prd_typ;
  char c_exrc_typ;
  char c_cntrct_tag;
  char c_ind_stk;
  char c_new_cntrct_tag;
  char c_old_cntrct_tag;

	/*** Ver 1.4 Starts ***/
  char c_sprd_allwd_flg = '\0';
  char c_sprd_chg_dt [ LEN_DATE ] = "NA";
	int i_cd = -1;
	FBFR32 *ptr_fml_tobuf;
	long int li_len_tobuf;
	long l_ret_val;
	/*** Ver 1.4 Ends ***/

	char c_und_str[2000]; /*** Ver 1.2 ***/

  strcpy ( c_ServiceName, argv[0] );

	INITBATDBGLVL(argv[0]);     /*** Ver 1.1 ***/

	sprintf ( c_msg, "Batch process %s started", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

	strcpy ( c_msg, "Creating data. Please wait ... " );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

	/*************** Ver 1.2 ***********
  if( argc < 7 )
  {
    fn_bat_pst_msg ( c_ServiceName, "Invalid Number of Arguments", c_tag );
    sprintf(c_msg,"syntax: %s <ini><Exg><NA><Exp dt><old tag><new tag>",argv[0]);
 	  fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
    exit(0);
  }

	**************************************/


	/*************** Ver 1.2 starts ********************/

  if( argc < 8 )
  {
    fn_bat_pst_msg ( c_ServiceName, "Invalid Number of Arguments", c_tag );
  	sprintf(c_msg,"syntax: %s <ini><Exg><NA><Exp dt><old tag><new tag><undlyng str/NONE>",argv[0]);
    fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
    exit(0);
  }

  /******************Ver 1.2 Ends ********************/	


 	strcpy( c_xchng_cd, argv[2] );
  strcpy( c_exp_dt, argv[4] );

	c_old_cntrct_tag = argv[5][0];	
	c_new_cntrct_tag = argv[6][0];	
	strcpy(c_und_str,argv[7]);           /*** Ver 1.2 ***/
	rtrim(c_sprd_chg_dt);							 /*** Ver 1.4 ***/

  fn_bat_pst_msg ( c_ServiceName, "Passed Arguments", c_tag );

	fn_userlog(c_ServiceName,"c_und_str:%s:",c_und_str);            /*** Ver 1.2 ***/
	fn_userlog(c_ServiceName,"c_sprd_allwd_flg:%c:",c_sprd_allwd_flg);      /*** Ver 1.4 ***/
	fn_userlog(c_ServiceName,"c_sprd_chg_dt:%s:",c_sprd_chg_dt);            /*** Ver 1.4 ***/

	if ( c_old_cntrct_tag == c_new_cntrct_tag )
	{
    fn_bat_pst_msg ( c_ServiceName, "Old contract tag and new contract tag cannot be same", c_tag );
    exit(0);
	}

	fn_userlog(c_ServiceName,"s1111");

	fn_init_ddr_pop ( "NA", "NA", "NA" );

	if( strcmp(c_und_str,"AUT_SPLT") != 0 )	/*** ver 1.4 ***/
	{
		ptr_fml_buf = fn_create_rs ( c_ServiceName,
                               c_err_msg,
                               "SFO_GT_CS_REC",
                               TPNOTIME,
															 7,                       /*** Ver 1.2 ,4 changes to 5*5->7 in ver 1.4**/
                               FFO_XCHNG_CD, ( char * )c_xchng_cd,
                               FFO_EXPRY_DT, ( char *) c_exp_dt,
                               FFO_CNTRCT_TAG, (char *)&c_old_cntrct_tag,
                               FFO_NEW_CNTRCT_TAG, (char *)&c_new_cntrct_tag, 
															 FFO_STCK_CD ,(char *)c_und_str,                 /*** Ver 1.2 ***/
                               FFO_SOURCE_FLG, (char *)&c_sprd_allwd_flg, 	/*** ver 1.4 ***/
                               FFO_RCRD_DT, ( char *)c_sprd_chg_dt);								/*** Ver 1.4 ***/
		if ( ptr_fml_buf == NULL )
		{

			fn_userlog(c_ServiceName,"Error while calling SFO_GT_CS_REC");
			fn_errlog( c_ServiceName, "S31005", LIBMSG, c_err_msg );
			return;
		}
	}								/*** Ver 1.4 ***/
	/*** Ver 1.4 Starts ***/
	else if(strcmp(c_und_str,"AUT_SPLT") == 0)
	{
		c_sprd_allwd_flg = argv[8][0];
  	strcpy( c_sprd_chg_dt, argv[9] );
		rtrim(c_sprd_chg_dt);
		fn_userlog( c_ServiceName, "c_sprd_allwd_flg:%c:", c_sprd_allwd_flg);   /*** Ver 1.4 ***/
		fn_userlog( c_ServiceName, "c_sprd_chg_dt:%s:", c_sprd_chg_dt);   /*** Ver 1.4 ***/
		ptr_fml_buf = fn_create_rs ( c_ServiceName,
                               c_err_msg,
                               "SFO_GT_CS_REC",
                               TPNOTIME,
                               7,                       
                               FFO_XCHNG_CD, ( char * )c_xchng_cd,
                               FFO_EXPRY_DT, ( char *) c_exp_dt,
                               FFO_CNTRCT_TAG, (char *)&c_old_cntrct_tag,
                               FFO_NEW_CNTRCT_TAG, (char *)&c_new_cntrct_tag,
                               FFO_STCK_CD ,(char *)c_und_str,
                               FFO_SOURCE_FLG, (char *)&c_sprd_allwd_flg,
                               FFO_RCRD_DT, ( char *)c_sprd_chg_dt);
    if ( ptr_fml_buf == NULL )
    {

      fn_userlog(c_ServiceName,"Error while calling SFO_GT_CS_REC");
      fn_errlog( c_ServiceName, "S31010", LIBMSG, c_err_msg );
      return;
    }
	}
	else
	{
		fn_bat_pst_msg ( c_ServiceName, "Invalid Number of Arguments", c_tag );
    sprintf(c_msg,"syntax for manual: %s <ini><Exg><NA><Exp dt><old tag><new tag><undlyng str/NONE>",argv[0]);
    sprintf(c_msg,"syntax for Automation: %s <ini><Exg><NA><Exp dt><old tag><new tag><undlyng str/NONE><sprd_allwd_flg><sprd_chg_dt>",argv[0]);
    fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
    exit(0);
	}
	/*** Ver 1.4 Ends ***/
		fn_userlog(c_ServiceName,"s2222");

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

	fn_userlog(c_ServiceName,"s3333");

	strcpy ( c_msg, "Data created." );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
  sprintf ( c_msg, "Total records = %ld", li_tot_rcrds );
  fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );


	fn_userlog(c_ServiceName,"s4444");

  for ( li_cnt=1; li_cnt<=li_tot_rcrds; li_cnt++ )
  {
		i_ch_val = fn_getnxt_rs( ptr_fml_buf,
                             c_ServiceName,
                             c_err_msg,
                             8,
                             FFO_EBA_MTCH_ACT_NO,( char *)c_cln_mtch_accnt,NULL,
                             FFO_XCHNG_CD,( char * )c_xchng_cd, NULL,
                             FFO_PRDCT_TYP,( char * )&c_prd_typ, NULL,
                             FFO_UNDRLYNG,( char * )c_undrlyng, NULL,
                             FFO_EXPRY_DT,( char * )c_exp_dt, NULL,
                             FFO_EXER_TYP,( char * )&c_exrc_typ, NULL,
                             FFO_CNTRCT_TAG,( char * )&c_cntrct_tag, NULL,
                             FFO_CTGRY_INDSTK,( char * )&c_ind_stk, NULL );
		if ( i_ch_val != 0 )
		{
			fn_errlog( c_ServiceName, "S31020", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
		}

		fn_userlog(c_ServiceName,"s5555");

    if(li_cnt % 100 == 1)
    {
      sprintf ( c_msg, "Processing record %ld of %ld", li_cnt, li_tot_rcrds );
      fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
    }

		fn_userlog(c_ServiceName,"s6666");

    i_ch_val = fn_pre_prcs_rcrd ( c_ServiceName,
                                  c_err_msg,
                                  "sfr_beos_gtdt",
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

		fn_userlog(c_ServiceName,"s7777");

		fn_cpy_ddr ( c_rout_crt ) ;

    i_ch_val = fn_acall_svc_fml ( c_ServiceName,
                                  c_err_msg,
                                  "SFO_PRCS_CS",
                                  0,
                                  12,
                                  FFO_EBA_MTCH_ACT_NO,( char *)c_cln_mtch_accnt,
                                  FFO_XCHNG_CD,( char * )c_xchng_cd,
                                  FFO_PRDCT_TYP,( char * )&c_prd_typ,
                                  FFO_UNDRLYNG,( char * )c_undrlyng,
                                  FFO_EXPRY_DT,( char * )c_exp_dt,
                                  FFO_EXER_TYP,( char * )&c_exrc_typ,
                                  FFO_CNTRCT_TAG,( char * )&c_cntrct_tag,
                                  FFO_CTGRY_INDSTK,( char * )&c_ind_stk,
                                  FFO_NEW_CNTRCT_TAG,( char * )
																														&c_new_cntrct_tag,
                                  FFO_BAT_TAG, ( char * )c_tag,
                                  FFO_ROUT_CRT, ( char * )c_rout_crt,
                                  FFO_REC_NO, ( char * )&li_cnt );
    if ( i_ch_val == -1 )
    {
			fn_errlog( c_ServiceName, "S31030", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
		}
  }

  /*** Ver 1.4 Starts ***/
	if(li_tot_rcrds > 0)
	{
    fn_userlog(c_ServiceName,"TPGETRPLY 111 for SFO_PRCS_CS");
    fn_userlog(c_ServiceName,"***** i_cd = :%d:",i_cd);
    ptr_fml_tobuf = (FBFR32 *)tpalloc("FML32",NULL,MIN_FML_BUF_LEN);
    if (ptr_fml_tobuf == NULL)
    {
      fn_errlog(c_ServiceName, "S31035",FMLMSG, c_err_msg);
      return;
    }

    fn_userlog(c_ServiceName,"TPGETRPLY 222 for SFO_PRCS_CS");
    /* i_ch_val = tpgetrply( &i_cd, (char **)&ptr_fml_tobuf, &li_len_tobuf, TPNOTIME|TPGETANY); */
    i_ch_val = tpgetrply( &i_cd, (char **)&ptr_fml_tobuf, &li_len_tobuf, TPGETANY);
    if (i_ch_val == -1)
    {
      fn_userlog(c_ServiceName,"Error received while getting response from SFO_PRCS_CS");
      sprintf ( c_msg, "Error received while getting response for Contract Split Process...");
      fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
      if ( tperrno != TPESVCFAIL )
      {
        fn_userlog(c_ServiceName,"Error received 111 while getting response from SFO_PRCS_CS");
        fn_errlog(c_ServiceName, "S31040",TPMSG, c_err_msg);
        l_ret_val = SYSTEM_ERROR;
      }
      else
      {
        fn_userlog(c_ServiceName,"Error received 222 while getting response from SFO_PRCS_CS");
        l_ret_val  = tpurcode;
      }
      tpfree ( ( char * ) ptr_fml_tobuf );
      return ;
    }
    fn_userlog(c_ServiceName,"TPGETRPLY 333 for SFO_PRCS_CS");
    sprintf ( c_msg, "After response received for Contract Split Process...");
    fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
	}
  /*** Ver 1.4 Ends ***/

	fn_dstry_rs ( ptr_fml_buf );

	sprintf ( c_msg, "Batch process %s ended", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

  return;
}

int fn_bat_init ( int argc, char *argv[] ) 
{
	fn_var = BAT_CNT_SPLT;
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
