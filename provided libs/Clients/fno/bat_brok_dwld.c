/*** ver 1.1	 IBM Migration Changes	 sandeep ***/
/*** Ver 1.2   Code Optimization change | Sachin Birje ***/
/*** Ver 1.3	 Optimization	(05-Nov-2018) | Bhushan Harekar ***/

#include <atmi.h>
#include <fml32.h>
#include <fo.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fn_battmpt.h>
#include <fml_def.h>
#include <fo_fml_def.h>
#include <fn_ddr.h>
#include <string.h>			/*** Ver 1.1 ***/			
#include <fml_rout.h>		/*** Ver 1.1 ***/
#include <fn_read_debug_lvl.h>			/*** Ver 1.1 ***/			

/*** Ver 1.3 Starts ***/

#define TOTAL_FML	3			
#define MAX_FETCH 1000

struct fno_class_brkg
{
  char   c_client_code[21];
  char   c_order_number[21];
  double  d_brokerage;
  double  d_total_brokerage;
};

/*** Ver 1.3 Ends ***/

void BAT_BROK_DWLD ( int argc, char *argv[] )
{
	/*** long int li_tot_rcrds; *** Commented in Ver 1.3 ***/
  int i_ch_val;
	int i_err[TOTAL_FML]; 			/*** Ver 1.3 ***/
  int i_ferr[TOTAL_FML];  		/*** Ver 1.3 ***/
	int i_fml_occur = 0;				/*** Ver 1.3 ***/
	int i_occur_loop = 0;				/*** Ver 1.3 ***/
	int i_recds_per_occur = 0;  /*** Ver 1.3 ***/
	int	i_fget_loop = 0;				/*** Ver 1.3 ***/
	int	i_rec = 0;							/*** Ver 1.3 ***/
  long	l_carr_buf_len = 0;		/*** Ver 1.3 ***/
	int i_tot_rcrds = 0;				/*** Ver 1.3 ***/
	long	l_buf_len = 0;				/*** Ver 1.3 ***/
	long l_brokerage = 0;				/*** Ver 1.3 ***/
  long int li_cnt;
  char c_msg[256];
	FBFR32 *ptr_fml_buf;
	char   *ptr_car_rbuf;				/*** Ver 1.3 ***/
  char c_ServiceName [ 33 ];
  char c_err_msg [ 256 ];

  char  c_usr_id[8];
  long  l_session_id;
  char  c_client_code[21];
  char  c_order_number[21];
  char  c_ixchng_cd[21];
  char  c_xchng_cd[21];
  char  c_date[12];
  char  c_rout_crt[4];
	/** double  l_brokerage; ** Commented in Ver 1.3 ***/
	double  d_total_brokerage;

	struct fno_class_brkg st_brkg_arr[MAX_FETCH];	/*** Ver 1.3 ***/

  strcpy ( c_ServiceName, argv[0] );

	INITBATDBGLVL(argv[0]);				 /*** Ver 1.1 ***/

	sprintf ( c_msg, "Batch process %s started", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

	strcpy ( c_msg, "Creating data. Please wait ... " );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
  strcpy(c_usr_id,"system");
  l_session_id = 0;
  strcpy( c_ixchng_cd, argv[2] );
  strcpy(c_date, argv[4] );
  strcpy(c_client_code, argv[5] );
	
  fn_init_ddr_pop ( "NA", "NA", "NA" );

	/*** Ver 1.3 Start ***/

	/* Calculate carray buffer len */
	l_carr_buf_len	=  sizeof ( struct fno_class_brkg) * MAX_FETCH;

	/* Allocate space to the CARRAY buffer pointer based on buffer length received. */
  ptr_car_rbuf = tpalloc ( "CARRAY", NULL, l_carr_buf_len );
  if ( ptr_car_rbuf == NULL )
  {
		strcpy ( c_msg, "System error. Contact system support" );
    fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
		fn_errlog( c_ServiceName, "S31005", TPMSG, c_err_msg );
		return;
  }

	/*** Ver 1.3 Ends ***/

	ptr_fml_buf = fn_create_rs ( c_ServiceName,
                               c_err_msg,
                               "SFO_GETBROK",
                               TPNOTIME,
                               5,
                               FFO_USR_ID, ( char * )c_usr_id,
                               FFO_SSSN_ID, ( char * )&l_session_id,
                               FFO_XCHNG_CD, ( char * )c_ixchng_cd,
                               FFO_TRD_DT, ( char * )c_date,
                               FFO_EBA_MTCH_ACT_NO, (char *)c_client_code );                               
	if ( ptr_fml_buf == NULL )
	{
		fn_errlog( c_ServiceName, "S31010", LIBMSG, c_err_msg );
		strcpy ( c_msg, "System error. Contact system support" );  /*** Ver 1.3 ***/
		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );						 /*** Ver 1.3 ***/
		tpfree( ptr_car_rbuf );		/*** Ver 1.3 ***/
		return;
	}

	/*** li_tot_rcrds = fn_reccnt_rs ( ptr_fml_buf,  *** Commented in Ver 1.3 ***/
			 i_fml_occur	=	fn_reccnt_rs ( ptr_fml_buf,	 /*** added in Ver 1.3 ***/
                                c_ServiceName,
                                c_err_msg,
                              	/*** FFO_EBA_MTCH_ACT_NO ** Commented in Ver 1.3 ***/
																FML_NO_OF_MSG); /*** added in Ver 1.3 ***/

	/*** if ( li_tot_rcrds == SYSTEM_ERROR ) *** Commented in Ver 1.3 ***/
	if ( i_fml_occur == SYSTEM_ERROR ) 
	{
		strcpy ( c_msg, "System error. Contact system support" );  /** Ver 1.3 **/
    fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );						 /** Ver 1.3 **/					
		fn_errlog( c_ServiceName, "S31015", LIBMSG, c_err_msg );
		fn_dstry_rs ( ptr_fml_buf );
    tpfree (ptr_car_rbuf);							/** Ver 1.3 **/
		return;
	}

  /*** Ver 1.3 Started ***/

  fn_userlog( c_ServiceName," Total fetch :%d:",i_fml_occur);

  if(i_fml_occur <= 0)
  {
    if(i_fml_occur == 0)
    {
      sprintf (c_msg ,"No data received.");
      fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
    }
    else
    {
      fn_errlog( c_ServiceName, "S61015", LIBMSG, c_err_msg );
      strcpy ( c_msg, "System error. Contact system support" );
      fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
      fn_dstry_rs ( ptr_fml_buf );
      tpfree (ptr_car_rbuf);
      return;
    }
  }

	for ( i_occur_loop = 0; i_occur_loop < i_fml_occur; i_occur_loop++ )
	{
		i_err[0] = Fget32 ( ptr_fml_buf, FML_NO_OF_MSG, i_occur_loop,(char*) &i_recds_per_occur, 0 );
		i_ferr[0]= Ferror32;

		if( i_err[0] == -1 )
		{
			strcpy ( c_msg, "System error. Contact system support" );
			fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			fn_errlog( c_ServiceName,"S31020",Fstrerror32(i_ferr[0]), c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
			tpfree ( ptr_car_rbuf );
		}	

		i_tot_rcrds += i_recds_per_occur;	

	}

	strcpy(c_xchng_cd,c_ixchng_cd);

  /*** Ver 1.3 Ends ***/

	strcpy ( c_msg, "Data created." );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
  sprintf ( c_msg, "Total records = %d", i_tot_rcrds );/*** li_tot_rcrds replced with i_tot_rcrds in ver 1.3 **/
  fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

  /*** for ( li_cnt=1; li_cnt<=i_tot_rcrds; li_cnt++ ) *** Commented in Ver 1.3 ***/ 
  for ( i_occur_loop = 0; i_occur_loop < i_fml_occur; i_occur_loop++ ) /*** for loop added in Ver 1.3 ***/
  {

		/*** Commented in Ver 1.3 ***
		i_ch_val = fn_getnxt_rs ( ptr_fml_buf,
                              c_ServiceName,
                              c_err_msg,
															6,
                              FFO_XCHNG_CD , ( char * )c_xchng_cd, NULL,
                              FFO_TRD_DT , ( char * )c_date, NULL,
                              FFO_EBA_MTCH_ACT_NO , ( char * )c_client_code, NULL,
															FFO_ACK_NMBR, (char *)c_order_number, NULL,
															FFO_BRKRG_VL, (char *)&l_brokerage, NULL,
															FFO_AMOUNT, (char *)&d_total_brokerage, NULL);
		if ( i_ch_val != 0 )
		{
			fn_errlog( c_ServiceName, "S31025", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
		}

		****/

		/**** Ver 1.3 Starts ****/

		i_err[0] = Fget32 ( ptr_fml_buf, FML_NO_OF_MSG, i_occur_loop, (char *)&i_recds_per_occur, 0 );
    i_ferr[0]= Ferror32;
    i_err[1] = Fget32 ( ptr_fml_buf, FML_RBUF_LEN, i_occur_loop, (char *)&l_buf_len, 0 );
    i_ferr[1]= Ferror32;
    i_err[2] = Fget32 ( ptr_fml_buf, FML_RBUF, i_occur_loop, ptr_car_rbuf, (FLDLEN32 *)&l_buf_len );
    i_ferr[2]= Ferror32;

		for( i_fget_loop = 0; i_fget_loop < TOTAL_FML; i_fget_loop++ )
		{
			if ( i_err[i_fget_loop] == -1 )
			{
				strcpy ( c_msg, "System error. Contact system support" );
				fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
				fn_userlog( c_ServiceName,"Error while getting data at position :%d:", i_fget_loop );	
				fn_errlog( c_ServiceName,"S31030",Fstrerror32(i_ferr[i_fget_loop]), c_err_msg );
				fn_dstry_rs ( ptr_fml_buf );
    		tpfree ( ptr_car_rbuf );          
				return;
			}
		}	

		MEMSET(st_brkg_arr);
		memcpy ( ( char * )&st_brkg_arr, ptr_car_rbuf, l_buf_len );
		
		for( i_rec = 0; i_rec < i_recds_per_occur; i_rec++)
		{
			strcpy(c_client_code,st_brkg_arr[i_rec].c_client_code);	
			strcpy(c_order_number,st_brkg_arr[i_rec].c_order_number);
			st_brkg_arr[i_rec].d_brokerage *= 100;
			st_brkg_arr[i_rec].d_total_brokerage *= 100;
			l_brokerage = (long)st_brkg_arr[i_rec].d_brokerage;
			d_total_brokerage = st_brkg_arr[i_rec].d_total_brokerage;

			li_cnt++;

		/*** Ver 1.3 Ends ***/

    if(li_cnt % 50 == 1)
    {
      sprintf ( c_msg, "Processing record %ld of %d", li_cnt, i_tot_rcrds );/** li_tot_rcrds replace by i_tot_rcrds in ver 1.3 ***/
      fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
    }	

    i_ch_val = fn_pre_prcs_rcrd ( c_ServiceName,
                                  c_err_msg,
                                /***  "sfr_beod_mtchgt", *** Ver 1.2 *** Wrong server Name passed ***/
                                  "sfr_beod_prcs",       /*** Ver 1.2 ***/
                                  li_cnt,
                                  i_tot_rcrds); /*** li_tot_rcrds replace by i_tot_rcrds in ver 1.3 ***/
    if ( i_ch_val == -1 )
    {
			fn_errlog( c_ServiceName, "S31035", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
      tpfree ( ptr_car_rbuf );				 /*** Ver 1.3 ***/
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
    }

    fn_cpy_ddr ( c_rout_crt ) ;

		if (DEBUG_MSG_LVL_3)			 /*** Ver 1.1 ***/
		{
			fn_userlog(c_ServiceName,"The l_brokerage is :%ld:",l_brokerage); /** '%lf' replaced by '%ld' in Ver 1.3 ***/
			fn_userlog(c_ServiceName,"The d_total_brokerage is :%lf:",d_total_brokerage);
  	}	

    i_ch_val = fn_acall_svc_fml ( c_ServiceName,
                                  c_err_msg,
                                  "SFO_UPD_BRKG",
                                  0,
																	9,
                              		FFO_XCHNG_CD , ( char * )c_xchng_cd,
                              		FFO_TRD_DT , ( char * )c_date,
                              		FFO_EBA_MTCH_ACT_NO , ( char * )c_client_code,
																	FFO_ACK_NMBR, (char *)c_order_number,
																	FFO_BRKRG_VL, (char *)&l_brokerage,
																	FFO_AMOUNT, (char *)&d_total_brokerage,
																	FFO_ROUT_CRT, (char *)c_rout_crt,
                                  FFO_BAT_TAG, ( char * )c_tag,
                                  FFO_REC_NO, ( char * )&li_cnt );
    if ( i_ch_val == -1 )
    {
			fn_errlog( c_ServiceName, "S31040", LIBMSG, c_err_msg );
			fn_dstry_rs ( ptr_fml_buf );
      tpfree ( ptr_car_rbuf );         /*** Ver 1.3 ***/
  		strcpy ( c_msg, "System error. Contact system support" );
  		fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );
			return;
		}
  }
	}

	fn_dstry_rs ( ptr_fml_buf );
  tpfree ( ptr_car_rbuf );         /*** Ver 1.3 ***/

	sprintf ( c_msg, "Batch process %s ended", argv[0] );
 	fn_bat_pst_msg ( c_ServiceName, c_msg, c_tag );

  return;
}

int fn_bat_init ( int argc, char *argv[] ) 
{
	fn_var = BAT_BROK_DWLD;
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
