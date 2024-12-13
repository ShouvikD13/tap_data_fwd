#include <atmi.h>
#include <fml32.h>
#include <tpadm.h>
#include <fo.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fml_def.h>
#include <fo_fml_def.h>
#include <fn_env.h>			/*** IBM migration Ver 1.1**/
#include <string.h>			/*** IBM migration Ver 1.1**/
#include <unistd.h>			/*** IBM migration Ver 1.1**/
#include <userlog.h>		/*** IBM migration Ver 1.1**/
#include <stdlib.h>     /*** IBM migration32 Ver 1.1**/
#include <fml_rout.h>		/*** IBM migration32 Ver 1.1**/

int fn_rgstr_batc ( char c_flag, 
                   char *c_pgm_name,
                   char *c_xchng_cd, 
                   char *c_pipe_id );  /* fn_rgstr_bat to fn_rgstr_batc in Ver TOL */

int fn_get_tot_prcs ( char *c_ServiceName,
                      char *c_err_msg,
                      char *c_srvr_nm );

int fn_bat_init ( int argc, char *argv[] );

void fn_bat_term ( int argc, char *argv[] );

int fn_chk_stts ( char *c_pgm_nm );

int fn_pre_prcs_rcrd ( char *c_ServiceName,
                       char *c_err_msg,
                       char *c_srvr_nm,
                       long int li_rec_cnt,
                       long int li_tot_rec );

extern int fn_prcs_trg ( char *c_ServiceName, 
                         int i_trg_typ );

void (*fn_var)( int argc, char *argv[] );
char c_tag [ 256 ];
#define BSNS_TRG 1
#define SYSTM_TRG 2

void main ( int argc, char *argv [ ] )
{
  TPINIT *ptr_st_tpinfo;
  int i_ch_val ;
	char c_ServiceName [ 33 ];
	char c_filenm [ 512 ];
	char c_err_msg [ 256 ];
	char *ptr_c_tmp;
	char c_filter [ 256 ];
	long int li_stp_trg_hnd;
	long int li_pse_trg_hnd;
	long int li_res_trg_hnd;

	printf("LIBPATH=:%s:", getenv("LIBPATH"));
	
  i_ch_val=fn_create_pipe();
  if ( i_ch_val == -1 )
  {
    userlog ( "Unable to create pipe" );
    exit ( 0 );
  }

	strcpy ( c_ServiceName, argv[0] );

	ptr_c_tmp = tuxgetenv ( "BIN" );
	if ( ptr_c_tmp == NULL )
	{
   	fn_errlog( c_ServiceName, "L31005", TPMSG, c_err_msg );
   	fn_destroy_pipe();
   	exit ( 0 );
	}
	sprintf ( c_filenm, "%s/%s.ini", ptr_c_tmp, argv [ 1 ] );

	i_ch_val = fn_init_prcs_spc ( c_ServiceName, 
                                c_filenm, 
                                c_ServiceName );
	if ( i_ch_val == -1 )
	{
   	fn_errlog( c_ServiceName, "L31010", LIBMSG, c_err_msg );
   	fn_destroy_pipe();
   	exit ( 0 );
	}

  ptr_st_tpinfo = (TPINIT *) tpalloc ( "TPINIT",
                                        NULL,
                                        MIN_FML_BUF_LEN );

  if ( ptr_st_tpinfo == NULL )
  {
    fn_errlog( c_ServiceName, "L31015", TPMSG, c_err_msg );
    fn_destroy_pipe();
    exit ( 0 );
  }

  strcpy ( ptr_st_tpinfo->passwd, "\0" );
  strcpy ( ptr_st_tpinfo->usrname, "\0" );
  strcpy ( ptr_st_tpinfo->cltname, c_ServiceName );
  ptr_st_tpinfo->flags = 0;
	ptr_c_tmp = ( char * ) fn_get_prcs_spc ( c_ServiceName, "CONN_GRP" );
	if ( ptr_c_tmp == NULL )
	{
    fn_errlog( c_ServiceName, "L31020", TPMSG, c_err_msg );
    tpfree ( (char *) ptr_st_tpinfo );
    fn_destroy_pipe();
    exit ( 0 );
	}
  strcpy ( ptr_st_tpinfo->grpname, ptr_c_tmp );

  i_ch_val = tpinit ( ptr_st_tpinfo );

  if ( i_ch_val == -1 )
  {
    fn_errlog( c_ServiceName, "L31025", TPMSG, c_err_msg );
    tpfree ( (char *) ptr_st_tpinfo );
    fn_destroy_pipe();
    exit ( 0 );
  }
  tpfree ( (char *) ptr_st_tpinfo );

  i_ch_val = tpopen ( );
  if ( i_ch_val == -1 )
  {
    fn_errlog( c_ServiceName, "L31030", TPMSG, c_err_msg );
    tpterm ( );
    fn_destroy_pipe();
    exit ( 0 );
  }

	i_ch_val = fn_rgstr_batc ( 'R', argv[0], argv[2], argv[3] ); 
	if ( i_ch_val == -2 )
	{
		fn_userlog ( c_ServiceName, "%s - Batch process already running", argv[0] );
		tpclose ( );
    tpterm ( );
    fn_destroy_pipe();
    exit ( 0 );
	}
	else if ( i_ch_val == -1 )
	{
		tpclose ( );
    tpterm ( );
    fn_destroy_pipe();
    exit ( 0 );
	}

	fn_set_msghnd ( );

	sprintf ( c_tag, "%s_%s", argv[0], argv[1] );
	sprintf ( c_filter, "FFO_FILTER=='%s'", c_tag );

  li_stp_trg_hnd = tpsubscribe ( "TRG_STP_BAT",
                                 ( char * ) c_filter,
                                 ( TPEVCTL * ) NULL,
                                 0 );
  if ( li_stp_trg_hnd == -1 )
  {
    fn_errlog( c_ServiceName, "L31035", TPMSG, c_err_msg );
		fn_rgstr_batc ( 'U', argv[0], argv[2], argv[3] ); 
		tpclose ( );
    tpterm ( );
    fn_destroy_pipe();
    exit ( 0 );
  }

  li_pse_trg_hnd = tpsubscribe ( "TRG_PSE_BAT",
                                 ( char * ) c_filter,
                                 ( TPEVCTL * ) NULL,
                                 0 );
  if ( li_pse_trg_hnd == -1 )
  {
    fn_errlog( c_ServiceName, "L31040", TPMSG, c_err_msg );
		tpunsubscribe ( -1, 0 );
		fn_rgstr_batc ( 'U', argv[0], argv[2], argv[3] ); 
		tpclose ( );
    tpterm ( );
    fn_destroy_pipe();
    exit ( 0 );
  }

  li_res_trg_hnd = tpsubscribe ( "TRG_RES_BAT",
                                 ( char * ) c_filter,
                                 ( TPEVCTL * ) NULL,
                                 0 );
  if ( li_res_trg_hnd == -1 )
  {
    fn_errlog( c_ServiceName, "L31045", TPMSG, c_err_msg );
		tpunsubscribe ( -1, 0 );
		fn_rgstr_batc ( 'U', argv[0], argv[2], argv[3] ); 
		tpclose ( );
    tpterm ( );
    fn_destroy_pipe();
    exit ( 0 );
  }

	i_ch_val = fn_bat_init ( argc, argv );
	if ( i_ch_val == -1 )
	{
    fn_errlog( c_ServiceName, "L31050", LIBMSG, c_err_msg );
		tpunsubscribe ( -1, 0 );
		fn_rgstr_batc ( 'U', argv[0], argv[2], argv[3] ); 
		tpclose ( );
    tpterm ( );
    fn_destroy_pipe();
    exit ( 0 );
	}

	if ( i_ch_val == 0 )
	{
		(*fn_var)( argc, argv );
	}

	fn_bat_term ( argc, argv );

	tpunsubscribe ( -1, 0 );

	fn_rgstr_batc ( 'U', argv[0], argv[2], argv[3] ); 

  tpclose ( );

  tpterm ( );

  fn_destroy_pipe();

  exit ( 0 );
}

int fn_rgstr_batc ( char c_flag, 
                   char *c_pgm_name,
                   char *c_xchng_cd, 
                   char *c_pipe_id )
{
  int     i_ch_val;
  char    c_retmsg[256];

	if ( ( strcmp ( c_xchng_cd, "NA" ) == 0 ) &&
       ( strcmp ( c_pipe_id, "NA"  ) == 0 )    )
	{
		return 0;
	}

	i_ch_val = fn_call_svc_fml ( c_pgm_name,
                               c_retmsg,
                               "SVC_RGSTR_BAT",
                               TPNOTRAN,
                               4,
                               0,
                               FML_GAME_REG_FLG, ( char * )&c_flag,
                               FML_GMS_NM, ( char * )c_pgm_name,
                               FML_ORD_XCHNG_CD, ( char * )c_xchng_cd,
                               FML_USR_ZIP_CD, ( char * )c_pipe_id );
	if ( i_ch_val != SUCC_BFR )
	{
    fn_errlog ( c_pgm_name, "L31055", LIBMSG, c_retmsg );
		return i_ch_val;
	}
 
  return 0;
}

int fn_get_tot_prcs ( char *c_ServiceName,
                      char *c_err_msg,
                      char *c_srvr_nm )
{
  FBFR32 *ptr_fml_buf;
  int i_cnt;
  int i_val[4];
  int i_ch_val;
  int i_no_of_prcs;
  char c_path[256];
  char *ptr_c_tmp;

  ptr_fml_buf = ( FBFR32 * ) tpalloc ( "FML32", NULL, MIN_FML_BUF_LEN );
  if ( ptr_fml_buf == NULL )
  {
    fn_errlog ( c_ServiceName, "L31060", TPMSG, c_err_msg );
    return ( -1 );
  }

  ptr_c_tmp = tuxgetenv ( "APPDIR" );
  if ( ptr_c_tmp == NULL )
  {
    fn_userlog ( c_ServiceName, 
                 "Unable to get APPDIR from environment settings" );
    tpfree ( ( char * ) ptr_fml_buf );
    return ( -1 );
  }

	sprintf ( c_path, "%s/%s", ptr_c_tmp, c_srvr_nm );

  i_val[0] = Fadd32 ( ptr_fml_buf, TA_OPERATION, "GET", 0 );
  i_val[1] = Fadd32 ( ptr_fml_buf, TA_CLASS, "T_SERVER", 0 );
  i_val[2] = Fadd32 ( ptr_fml_buf, TA_STATE, "ACT", 0 );
  i_val[3] = Fadd32 ( ptr_fml_buf, TA_SERVERNAME, c_path, 0 );
  for ( i_cnt=0; i_cnt<=3; i_cnt++ )
  {
    if ( i_val[i_cnt] == -1 )
    {
      fn_userlog ( c_ServiceName, "Error in field %d", i_cnt );
      fn_errlog ( c_ServiceName, "L31065", FMLMSG, c_err_msg);
      tpfree ( ( char * ) ptr_fml_buf );
      return ( -1 );
    }
  }

  i_ch_val = tpadmcall ( ptr_fml_buf, &ptr_fml_buf, 0 );
  if ( i_ch_val == -1 )
  {
    fn_errlog ( c_ServiceName, "L31070", TPMSG, c_err_msg);
    tpfree ( ( char * ) ptr_fml_buf );
    return ( -1 );
  }

  i_ch_val = Fget32 ( ptr_fml_buf, TA_MIN, 0, ( char * )&i_no_of_prcs,
                      NULL );
  if ( i_ch_val == -1 )
  {
    fn_errlog ( c_ServiceName, "L31075", FMLMSG, c_err_msg);
    tpfree ( ( char * ) ptr_fml_buf );
    return ( -1 );
  }

  tpfree ( ( char * ) ptr_fml_buf );
  return ( i_no_of_prcs );
}

int fn_chk_stts ( char *c_pgm_nm )
{
	int i_ch_val;

	i_ch_val = fn_chk_trg ( );
 
	if ( i_ch_val == 0 )
	{
		return 0;
	}



  // from where we are getting the "c_trg_msg" . this variable is not defined .

	fn_userlog ( c_pgm_nm, "%s trigger being processed", c_trg_msg );

	if ( strcmp ( c_trg_msg, "TRG_STP_BAT" ) == 0 )     
	{
		fn_bat_pst_msg_fno ( c_pgm_nm, "Stop trigger got", c_tag );

      /*  this function "fn_bat_pst_msg_fno " is inside the "fn_tuxlib.c)"
        void fn_bat_pst_msg_fno(char *c_pgm_nm,
						char *c_msg,
						char *c_tag)
        {
          int i_ch_val;
          char c_retmsg[256];

          fn_userlog(c_pgm_nm, "%s - %s", c_tag, c_msg);

          i_ch_val = fn_acall_svc_fml(c_pgm_nm,
                        c_retmsg,
                        "SVC_BAT_MSG",
                        TPNOTRAN | TPNOREPLY,
                        2,
                        FML_EVENT, (char *)c_msg,
                        FML_FML_NAME, (char *)c_tag);
          if (i_ch_val == -1)
          {
            fn_errlog(c_pgm_nm, "L31195", LIBMSG, c_retmsg);
          }

          return;
        }


      */

            /*   this function "fn_acall_svc_fml" is used inside the "fn_bat_pst_msg_fno " and it is in "fn_tuxlib"

                        int fn_acall_svc_fml(char *c_ServiceName,
                          char *c_err_msg,
                          char *c_svc_nam,
                          long int li_flag,
                          int i_tot_ifields,
                          ...)
                        {
                          va_list c_arg_lst;
                          int i_cnt;
                          int i_ch_val;
                          //unsigned long l_ip_fml; 
                          unsigned int i_ip_fml;  //Ver 1.3 64 bit portability 
                          char *c_ip_dat;
                          FBFR32 *ptr_fml_tibuf;
                          int i_ret_val;

                          va_start(c_arg_lst, i_tot_ifields);

                          // Allocat input and output buffer 
                          ptr_fml_tibuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);
                          if (ptr_fml_tibuf == NULL)
                          {
                            fn_errlog(c_ServiceName, "L31065", TPMSG, c_err_msg);
                            return SYSTEM_ERROR;
                          }

                          for (i_cnt = 1; i_cnt <= i_tot_ifields; i_cnt++)
                          {
                            i_ip_fml = va_arg(c_arg_lst, unsigned int); // Ver  1.3 64 bit portability l_op_fml changed to i_op_fml 
                                                  // ver 1.3 2nd parameter type unsigned long changed to unsigned int 
                            c_ip_dat = va_arg(c_arg_lst, char *);
                            i_ch_val = Fadd32(ptr_fml_tibuf, i_ip_fml, (char *)c_ip_dat, 0);
                            //  ver 1.3 2nd parameter l_op_fml changed to i_op_fml 
                            if (i_ch_val == -1)
                            {
                              fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
                              fn_errlog(c_ServiceName, "L31070", FMLMSG, c_err_msg);
                              tpfree((char *)ptr_fml_tibuf);
                              return SYSTEM_ERROR;
                            }
                          }

                          i_ch_val = tpacall(c_svc_nam, (char *)ptr_fml_tibuf, 0, li_flag);
                          if (i_ch_val == -1)
                          {
                            fn_errlog(c_ServiceName, "L31075", TPMSG, c_err_msg);
                            tpfree((char *)ptr_fml_tibuf);
                            return SYSTEM_ERROR;
                          }

                          i_ret_val = 0;

                          tpfree((char *)ptr_fml_tibuf);

                          return (i_ret_val);
                        }

            */
		
           
    fn_prcs_trg ( c_pgm_nm, SYSTM_TRG );

        /* this function "fn_prcs_trg" is inside the "bat_cnt_splt.c" and don't know what it is doing 

            int fn_prcs_trg(char *c_ServiceName, int i_trg_typ)
      {
        int i_ch_val;
        char c_err_msg[256];

        char sql_xchng_crrnt_stts;	
        char sql_xchng_exrc_mkt_stts;
        char sql_xchng_extnd_mrkt_stts; 

        if (i_trg_typ == SYSTM_TRG)
        {
          i_exit_stts = EXIT;
          return (-1);
        }


        if (strcmp(c_trg_msg, "TRG_ORD_OPN") == 0)
        {
          i_ord_mkt_stts = TRUE;
        }
        else if (strcmp(c_trg_msg, "TRG_ORD_CLS") == 0)
        {
          i_ord_mkt_stts = FALSE;
        }
        else if (strcmp(c_trg_msg, "TRG_EXT_OPN") == 0)
        {
          i_ext_mkt_stts = TRUE;
        }
        else if (strcmp(c_trg_msg, "TRG_EXT_CLS") == 0)
        {
          i_ext_mkt_stts = FALSE;
        }
        else if (strcmp(c_trg_msg, "TRG_EXR_OPN") == 0)
        {
          i_exr_mkt_stts = TRUE;
        }
        else if (strcmp(c_trg_msg, "TRG_EXR_CLS") == 0)
        {
          i_exr_mkt_stts = FALSE;
        }
        else if (strcmp(c_trg_msg, "TRG_STRT_PMP") == 0)
        {
          

          EXEC SQL
            SELECT exg_crrnt_stts,
            exg_exrc_mkt_stts,
            exg_extnd_mrkt_stts
              INTO : sql_xchng_crrnt_stts,
            : sql_xchng_exrc_mkt_stts,
            : sql_xchng_extnd_mrkt_stts
                FROM exg_xchng_mstr
                  WHERE exg_xchng_cd = : sql_c_xchng_cd;

          if (SQLCODE != 0)
          {
            fn_errlog(c_ServiceName, "S31230", SQLMSG, c_err_msg);
            return (-1);
          }

          if (DEBUG_MSG_LVL_0)
          {
            fn_userlog(c_ServiceName, "Before Forwardnig normal market status is :%c:", sql_xchng_crrnt_stts);
            fn_userlog(c_ServiceName, "Before Forwardnig extended market status is :%c:", sql_xchng_extnd_mrkt_stts);
            fn_userlog(c_ServiceName, "Before Forwardnig exercise market status is :%c:", sql_xchng_exrc_mkt_stts);
            fn_userlog(c_ServiceName, "Before Forwarding Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:i_ext_mkt_stts:%d:", i_ord_mkt_stts, i_exr_mkt_stts, i_ext_mkt_stts);
          }

          if (sql_xchng_crrnt_stts == EXCHANGE_OPEN)
          {
            i_ord_mkt_stts = TRUE;
          }
          if (sql_xchng_exrc_mkt_stts == EXCHANGE_OPEN)
          {
            i_exr_mkt_stts = TRUE;
          }
          if (sql_xchng_extnd_mrkt_stts == EXCHANGE_OPEN)
          {
            i_ext_mkt_stts = TRUE;
          }

          if (DEBUG_MSG_LVL_0)
          {
            fn_userlog(c_ServiceName, "After Forwarding Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:i_ext_mkt_stts:%d:", i_ord_mkt_stts, i_exr_mkt_stts, i_ext_mkt_stts);
          }

          

          i_pmp_stts = TRUE;
          i_upd_stts = TRUE;
        }
        else if (strcmp(c_trg_msg, "TRG_STOP_PMP") == 0)
        {
          i_pmp_stts = FALSE;
          i_upd_stts = TRUE;
        }
        else if (strcmp(c_trg_msg, "TRG_RECO_OVER") == 0)
        {
          i_reco_stts = 0;
          fn_userlog(c_ServiceName, "After i_reco_stts = 0");
        }
        else if (strcmp(c_trg_msg, "TRG_PART_SUS") == 0)
        {
          i_part_stts = FALSE;
        }
        else if (strcmp(c_trg_msg, "TRG_PART_ACT") == 0)
        {
          i_part_stts = TRUE;
        }
        else if (strcmp(c_trg_msg, "TRG_BRKR_SUS") == 0)
        {
          i_brkr_stts = FALSE;
        }
        else if (strcmp(c_trg_msg, "TRG_BRKR_ACT") == 0)
        {
          i_brkr_stts = TRUE;
        }

        return (0);
      }



              */

		return -1;
	}
	else if ( strcmp ( c_trg_msg, "TRG_PSE_BAT" ) == 0 )
	{
		fn_bat_pst_msg_fno ( c_pgm_nm, "Pause trigger got", c_tag );
		while ( strcmp ( c_trg_msg, "TRG_PSE_BAT" ) == 0 )
		{
			sleep ( 5 );
			fn_chk_trg ( );
		}
		if ( strcmp ( c_trg_msg, "TRG_STP_BAT" ) == 0 )
		{
			fn_bat_pst_msg_fno ( c_pgm_nm, "Stop trigger got", c_tag );
			fn_prcs_trg ( c_pgm_nm, SYSTM_TRG );
			return -1;
		}
		fn_bat_pst_msg_fno ( c_pgm_nm, "Resume trigger got", c_tag );
	}

	i_ch_val = fn_prcs_trg ( c_pgm_nm, BSNS_TRG ); // defined in this file "BSNS_TRG"

	strcpy ( c_trg_msg, "" );

	return ( i_ch_val );
}

int fn_pre_prcs_rcrd ( char *c_ServiceName, char *c_err_msg, char *c_srvr_nm, long int li_rec_cnt, long int li_tot_rec )
{
  static long int li_tot_prcs = 0;
  static long int li_prcs_used = 0;
  int i_ch_val;
  int i_cd;
  char c_msg[256];
	int i_ret_val;

	i_cd = -1;

	if ( li_tot_prcs == 0 )
	{
  	li_tot_prcs = fn_get_tot_prcs ( c_ServiceName,
                                    c_err_msg,
                                    c_srvr_nm );
  	if ( li_tot_prcs == -1 )
  	{
    	return -1;
  	}
	}

  i_ch_val = fn_chk_stts ( c_ServiceName );
  if ( i_ch_val == -1 )
  {
		while ( li_prcs_used > 0 )
		{
    	i_ch_val = fn_agetrep_svc_fml ( c_ServiceName,
                                      c_err_msg,
                                      &i_cd,
                                      TPGETANY,
                                      0 );
    	if ( i_ch_val == SYSTEM_ERROR ) 
    	{
      	fn_errlog( c_ServiceName, "L31080", LIBMSG, c_err_msg );
      	strcpy ( c_msg, "System error. Contact system support" );
      	fn_bat_pst_msg_fno ( c_ServiceName, c_msg, c_tag );
				return -1;
    	}

    	li_prcs_used--;
		}

    return -1;
  }
	i_ret_val = i_ch_val;

	if ( li_prcs_used >= li_tot_prcs )
	{
    i_ch_val = fn_agetrep_svc_fml ( c_ServiceName,
                                    c_err_msg,
                                    &i_cd,
                                    TPGETANY,
                                    0 );
    if ( i_ch_val == SYSTEM_ERROR )
    {
      fn_errlog( c_ServiceName, "L31085", LIBMSG, c_err_msg );
      strcpy ( c_msg, "System error. Contact system support" );
      fn_bat_pst_msg_fno ( c_ServiceName, c_msg, c_tag );
			return -1;
    }

    li_prcs_used--;
	}

	if ( li_rec_cnt == li_tot_rec )
	{
		while ( li_prcs_used > 0 )
		{
    	i_ch_val = fn_agetrep_svc_fml ( c_ServiceName,
                                      c_err_msg,
                                      &i_cd,
                                      TPGETANY,
                                      0 );
    	if ( i_ch_val == SYSTEM_ERROR )
    	{
      	fn_errlog( c_ServiceName, "L31090", LIBMSG, c_err_msg );
      	strcpy ( c_msg, "System error. Contact system support" );
      	fn_bat_pst_msg_fno ( c_ServiceName, c_msg, c_tag );
				return -1;
    	}

    	li_prcs_used--;
		}

		return 0;
	}

  li_prcs_used++;
	return ( i_ret_val );
}
