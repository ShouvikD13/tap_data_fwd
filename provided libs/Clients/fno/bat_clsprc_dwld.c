/*** ver 1.1 IBM Migration Changes sandeep ***/
/*** Ver 1.2 FNO BSE Changes Sachin Birje********/
/*** Ver 1.3 Code Optimization | Sachin Birje ***/

#include <string.h>			/*** Ver 1.1 ***/	
#include <atmi.h>
#include <fml32.h>
#include <fo.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fn_battmpt.h>
#include <fml_def.h>
#include <fo_fml_def.h>
#include <fn_ddr.h>
#include <fml_rout.h>			/*** Ver 1.1 ***/	
#include <fn_read_debug_lvl.h>      /*** Ver 1.1 ***/

void BAT_CLSPRC_DWLD ( int argc, char *argv[] )
{
	long 		int li_tot_rcrds;
  int 		i_ch_val;
  long int li_cnt;
  char 		c_msg[256];
	FBFR32 *ptr_fml_buf;
  char 		c_ServiceName [ 33 ];
  char 		c_err_msg [ 256 ];

	char  	c_usr_id[8];
  long  	l_session_id;
	char  	c_ixchng_cd [21];	
  char  	c_trd_dt[12];
  char  	c_trade_dt[12];
	char		c_dwld_typ[2];
  long 		l_script_id;
  char 		c_instrmnt_typ[9];
  char 		c_symbol[12];
  char 		c_expry_dt[12];
  char 		c_opt_typ[3];
  double 	d_strike_prc = 0.00;
  long 		l_strike_prc = 0;
  long    l_token_no=0;       /*** Ver 1.2 ***/
/*char 		c_ca_lvl[3];	*/			/*** Ver 1.1 ***/
	long    l_ca_lvl;							/*** Ver 1.1 ***/
	long		l_close_price;		
	char 		c_xchng_cd[4];
	char 		c_rout_crt[4];

  strcpy ( c_ServiceName, argv[0] );

	INITBATDBGLVL(argv[0]);     /*** Ver 1.1 ***/

	sprintf ( c_msg, "Batch process %s started", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

	strcpy(c_usr_id,"system"); /**/
	strcpy ( c_msg, "Creating data. Please wait ... " );

 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
  l_session_id = 0; 
	strcpy( c_ixchng_cd, argv[2] ); 
	strcpy(c_trd_dt, argv[4] ); 
	strcpy(c_dwld_typ ,argv[5] ); 

  fn_init_ddr_pop ( "NA", "NA", "NA" );

	if(strcmp(c_dwld_typ,"O")==0)
	{
	ptr_fml_buf = fn_create_rs ( c_ServiceName,
                               c_err_msg,
                               "SFO_GETOPTMTM",
                               TPNOTIME,
															 4,
                               FFO_USR_ID, ( char * )c_usr_id,
                               FFO_SSSN_ID, ( char * )&l_session_id,
                               FFO_XCHNG_CD, ( char * )c_ixchng_cd,
                               FFO_TRD_DT, ( char * )c_trd_dt );
	}
	else if(strcmp(c_dwld_typ,"F")	==	0)
	{
	ptr_fml_buf = fn_create_rs ( c_ServiceName,
                               c_err_msg,
                               "SFO_GETFUTMTM",
                               TPNOTIME,
															 4,
                               FFO_USR_ID, ( char * )c_usr_id,
                               FFO_SSSN_ID, ( char * )&l_session_id,
                               FFO_XCHNG_CD, ( char * )c_ixchng_cd,
                               FFO_TRD_DT, ( char * )c_trd_dt );
	}
	

	if ( ptr_fml_buf == NULL )
	{
		fn_errlog( c_ServiceName, "S31005", LIBMSG, c_err_msg );
  	strcpy ( c_msg, "Failed to Create the records");
  	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
		return;
	}

	li_tot_rcrds = fn_reccnt_rs ( ptr_fml_buf,
                                c_ServiceName,
                                c_err_msg,
                                FFO_SYMBOL);
	if ( li_tot_rcrds == SYSTEM_ERROR )
	{
		fn_errlog( c_ServiceName, "S31010", LIBMSG, c_err_msg );
  	strcpy ( c_msg, "Failed to get the count of records");
  	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
		fn_dstry_rs ( ptr_fml_buf );
		return;
	}

	strcpy ( c_msg, "Data created." );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
  sprintf ( c_msg, "Total records = %ld", li_tot_rcrds );
  fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
  fn_userlog(c_ServiceName,"  c_ixchng_cd :%s:",c_ixchng_cd); 
  for ( li_cnt=1; li_cnt<=li_tot_rcrds; li_cnt++ )
  {
   if( strcmp(c_ixchng_cd, "NFO") == 0 )  /*** Ver 1.2 ***/
   {
		i_ch_val = fn_getnxt_rs ( ptr_fml_buf,	/*Gets the next record*/
                              c_ServiceName,
                              c_err_msg,
                              9,
															FFO_INSTRMNT_TYP , ( char * )c_instrmnt_typ, NULL,
															FFO_SYMBOL, ( char * )c_symbol, NULL,
															FFO_HGH_PRC, ( char * )&l_strike_prc, NULL,
															FFO_EXPRY_DT, ( char * )c_expry_dt, NULL,
															FFO_OPTION_TYP,( char * )c_opt_typ, NULL,
															FFO_CA_LVL, ( char * )&l_ca_lvl, NULL,			/*** Ver 1.1 ***/
															FFO_XCHNG_CD, ( char * )c_xchng_cd, NULL,
															FFO_CLS_PRC, (char *)&l_close_price,NULL,
															FFO_TRD_DT, (char *)c_trade_dt,NULL );
   }
   else if( strcmp(c_ixchng_cd, "BFO") == 0) /*** Ver 1.2 starts here ***/
   {                                         /*** In case of BSE, store proc is returning extra output l_token_no ****/
     fn_userlog(c_ServiceName," Inside 11 ");
     i_ch_val = fn_getnxt_rs ( ptr_fml_buf,  /*Gets the next record*/
                              c_ServiceName,
                              c_err_msg,
                              10,
                              FFO_INSTRMNT_TYP , ( char * )c_instrmnt_typ, NULL,
                              FFO_SYMBOL, ( char * )c_symbol, NULL,
                              FFO_HGH_PRC, ( char * )&l_strike_prc, NULL,
                              FFO_EXPRY_DT, ( char * )c_expry_dt, NULL,
                              FFO_OPTION_TYP,( char * )c_opt_typ, NULL,
                              FFO_CA_LVL, ( char * )&l_ca_lvl, NULL,    
                              FFO_XCHNG_CD, ( char * )c_xchng_cd, NULL,
                              FFO_CLS_PRC, (char *)&l_close_price,NULL,
                              FFO_TRD_DT, (char *)c_trade_dt,NULL,
                              FFO_TOKEN_ID, (char *)&l_token_no,NULL );  
 
   }  /*** Ver 1.2 Ends here ****/
    fn_userlog(c_ServiceName," Inside 12 ");
		if ( i_ch_val != 0 )
		{
			fn_errlog( c_ServiceName, "S31015", LIBMSG, c_err_msg );
  		strcpy ( c_msg, "System error. Contact system support ");
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			fn_dstry_rs ( ptr_fml_buf );
			return;
		}
   fn_userlog(c_ServiceName," Inside 13 ");	
	 /* Fget32(ptr_fml_buf,FFO_HGH_PRC,0, (char *)&l_strike_prc,0); */
			d_strike_prc = ((double)(l_strike_prc ))/ 100 ;
    fn_userlog(c_ServiceName,"THE STRIKE PRICE_LONG IS :%ld:",l_strike_prc);
    fn_userlog(c_ServiceName,"THE STRIKE PRICE_DOUBLE IS :%lf:",d_strike_prc);
    if(li_cnt % 150 == 1)
    {
      sprintf ( c_msg, "Processing record %ld of %ld", li_cnt, li_tot_rcrds );
      fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
    }

    i_ch_val = fn_pre_prcs_rcrd ( c_ServiceName,
                                  c_err_msg,
                               /**   "sfr_beod_mtchgt", *** Ver 1.3 *** Wrong server Name passed ***/
                                  "sfr_beod_prcs",      /*** Ver 1.3 ***/
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
		rtrim(c_trade_dt);
    fn_userlog(c_ServiceName," Inside 14 ");
    fn_cpy_ddr ( c_rout_crt ) ;
   if( strcmp(c_ixchng_cd, "NFO") == 0 )  /*** Ver 1.2 ***/
   {  
    i_ch_val = fn_acall_svc_fml ( c_ServiceName,
                                  c_err_msg,
                                  "SFO_CLSPRC_DWLD",
                                  0,
																	12,
                              		FFO_INSTRMNT_TYP , ( char * )c_instrmnt_typ,
                              		FFO_SYMBOL, ( char * )c_symbol,
                              		FFO_EXPRY_DT, ( char * )c_expry_dt,
                              		FFO_OPTION_TYP,( char * )c_opt_typ,
                              		FFO_STRIKE_PRC, ( char * )&d_strike_prc,
                              		FFO_CA_LVL, ( char * )&l_ca_lvl,				/*** Ver 1.1 ***/
                              		FFO_CLS_PRC, (char *)&l_close_price,
                              		FFO_XCHNG_CD, (char *)c_xchng_cd,
                              		/** FFO_XCHNG_CD, (char *)c_ixchng_cd,	Ver	1.2	***/
																	FFO_TRD_DT, (char *)c_trade_dt,
                              		FFO_ROUT_CRT, (char *)c_rout_crt,
																	FFO_BAT_TAG, ( char * )c_tag,
                                  FFO_REC_NO, ( char * )&li_cnt);
   }
   /***Ver 1.2** In case of BSE, For stock code MSX & BSX, class is returning the same c_symbol value
                 which is 1,So distinguish between them, class is sending the new field token no in 
                 case of BSE only ****/ 
   else if( strcmp(c_ixchng_cd, "BFO") == 0 )  /*** Ver 1.2, starts here ****/
   {                                           /*** In case of BSE, we are sending extra input l_token_no to service ****/
    fn_userlog(c_ServiceName," Token no in batch prm :%ld:",l_token_no);
    i_ch_val = fn_acall_svc_fml ( c_ServiceName,
                                  c_err_msg,
                                  "SFO_CLSPRC_DWLD",
                                  0,
                                  13,
                                  FFO_INSTRMNT_TYP , ( char * )c_instrmnt_typ,
                                  FFO_SYMBOL, ( char * )c_symbol,
                                  FFO_EXPRY_DT, ( char * )c_expry_dt,
                                  FFO_OPTION_TYP,( char * )c_opt_typ,
                                  FFO_STRIKE_PRC, ( char * )&d_strike_prc,
                                  FFO_CA_LVL, ( char * )&l_ca_lvl,        
                                  FFO_CLS_PRC, (char *)&l_close_price,
                                  FFO_XCHNG_CD, (char *)c_xchng_cd,
                                  FFO_TRD_DT, (char *)c_trade_dt,
                                  FFO_TOKEN_ID, (char *)&l_token_no,
                                  FFO_ROUT_CRT, (char *)c_rout_crt,
                                  FFO_BAT_TAG, ( char * )c_tag,
                                  FFO_REC_NO, ( char * )&li_cnt); 

   }
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
	fn_var = BAT_CLSPRC_DWLD;
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
