/******************************************************************************/
/*  Program           : 		SFO_CON_CLNT                                      */
/*                                                                            */
/*  Input             : 		FFO_EXCHNG_USR_ID                                 */
/*                      		FFO_RQST_TYP                                      */
/*                          FFO_PIPE_ID                                       */
/*																																						*/
/*  Output            :     FFO_REMARK                                        */
/*                          FFO_ERR_MSG                                       */
/*                                                                            */
/*	Description				:			This process acts as a server process to all the  */
/*													admin consoles. it accepts various requests such  */
/*													broadcast clients, starting up ESR and EBRD 		  */
/*													servers,logon and logoff process with TAP.        */
/*																																			      */
/*                                                                            */
/*  Log 1.0           :  Initial Release  : Shamili Dalvi.                    */
/*  Log 1.1						:  Brodcast         : Shamili Dalvi.                    */	
/*  Log	1.2           :	 Single Console		:	Shamili Dalvi.                    */
/*  Log	1.3           :	 Single Console		:	Shamili Dalvi.                    */
/*  Log 1.4           :  Re-Forwarding    : Navina D.                         */
/*  Log 1.5           :  Consolidated FNO Console	NFO/BFO 	: Bineet S.       */
/*  Log 1.6						:  Skip admin status check for connect All and          */
/*                       Forward All options	: Navina D (17-Oct-2013)        */  
/*  Log 1.7(24-Dec-2014): Exchange login Password generation Change :Sachin B  */
/*  Log 1.8(23-Jan-2015): Auto Reconnect changes                   :Sachin B  */
/*  Log 2.1           :  Broadcast Automation : Tanmay W.(04-Feb-2015)        */
/*  Log 2.2           :  Authentication of FNO Console :Kishor B(21-Oct-2016) */
/*	Log 2.4           :  Flow of Exchange login Password generation Changes : Vishal Borhade (11-Mar-2017) */
/*  Log 2.5           :  Added one seperate instance for broadcast process    */
/*                    :  Varadraj G                                           */
/*  Log 2.6 					:  sleep introduced in auto-reconnect case to wait until*/ 
/*											 cln_esr_clnt removes from process status | Prachi B 	*/ 
/*  Log 2.7           :  Changes in the query which updates the placed status */
/*                       to 'M' | Suchita Dabir(19-Nov-2018)                  */ 
/*	Log 2.8						:	 Handling added to process 'M' status orders using		*/
/*											 script : Bhushan Harekar (25-Sep-2018)								*/
/*	Log 2.9						:	 DC Adapter Console Changes (28-Aug-2019)							*/
/*	Log 3.0						:	 Increased in broadcast instances (02-Apr-2020,Sachin Birje)*/
/*  Log TOL           :  Tux on Linux (07-Aug-2023, Agam)                     */
/******************************************************************************/
/**** C header ****/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**** To be added for Pro c codes ****/
#include <sqlca.h>

/**** Tuxedo header ****/
#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>
#include <fcntl.h>

/**** Application header ****/
#include <fo_fml_def.h>
#include <fo_view_def.h>
#include <fo.h>
#include <fn_env.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fn_scklib_tap.h>				/** 1.1 **/
#include <fn_read_debug_lvl.h>      
#include <fn_msgq.h>
#include <fml_rout.h>
#include <fn_ddr.h>
#include <fo_exg_con_lib.h>
#include <fn_rndm_passwd_genrn.h>  /** Ver 1.7 **/
#include <signal.h>                /* Added in TOL */
#include <unistd.h>                /* Added in TOL */

#define	MAX 50										/*1.2*/
#define	PIPE	15									/*1.2*/
#define RUNNING 1                 /*1.8*/
#define NOT_RUNNING 0            /*1.8*/
extern int i_send_qid;            /* int added in Ver TOL */

int fn_strt_esr_clnt ( char	*c_pipe_id,
                       char c_auto_reconnect,  /** Ver 1.8 **/
                       char c_server_flag_new,      /** Ver 1.8 **/ 
											 char *c_ServiceName, 
  	                   char *c_err_msg);

int fn_strt_pack_clnt ( char *c_pipe_id,
                       FBFR32 *ptr_fml_ibuf,
											 char *c_ServiceName, 
                    	 char *c_err_msg);

int fn_stop_pack_clnt( char *c_pipe_id,
  									  char *c_ServiceName, 
                      char *c_err_msg);

int fn_stop_esr_clnt( char *c_pipe_id, 
											char *c_ServiceName, 
                     	char *c_err_msg);

int fn_mkt_stts_chng( char *c_trg_name,
                      int i_mkt_type,
                      int i_mkt_stts,
                      char *c_pipe_id,
                      char *c_ServiceName,
                      char *c_err_msg);
/** 1.1 starts**/
int	fn_req_brd_chnl(char *c_pipe_id,
										 char *c_ServiceName,
										 char *c_err_msg,
                     char c_flag);                  /**** Added in Ver 2.1 *****/

int fn_strt_bcast_clnt ( char *c_pipe_id,
                         char *c_ServiceName,
                         char *c_err_msg,
                     char c_flag);                  /**** Added in Ver 2.1 *****/

int	fn_stop_brd_clnt (char	*c_pipe_id,
                      char  *c_ServiceName,
                      char  *c_err_msg,
                     char c_flag);                  /**** Added in Ver 2.1 *****/


/** 1.1 Ends**/

	int fn_stop_new_brd_clnt (char *c_prcs_nm,          /**** Added in Ver 2.1 *****/
                          char *c_xchng_cd,
                          char *c_pipe_id,
                          char *c_ServiceName,
                          char *c_err_msg);


int fn_get_prcs_id(char *c_prcs_nm, 
									 char *c_xchng_cd, 
								   char *c_pipe_id, 
									 long *li_pro_id,
								   char *c_ServiceName, 
									 char *c_err_msg);


int fn_get_new_prcs_id(char *c_prcs_nm,           /*** Added in Ver 2.1 ***/
                   char *c_xchng_cd,
                   char *c_pipe_id,
                   long *li_pro_id,
                   char *c_ServiceName,
                   char *c_err_msg);


int fn_stop_prcs(char *c_name ,
								long li_pro_id,
								char *c_ServiceName,
							  char *c_err_msg);

/** 1.2 Starts **/

int fn_all_status(char   *c_pipe_id,
									char 	 *c_all_status,
                  FBFR32 *ptr_fml_ibuf,
                  char   *c_ServiceName,
                  char   *c_err_msg);

int fn_chk_n_strt_frwrdng (char *c_pipe_id,
                        	 char *c_ServiceName,
                        	 char *c_err_msg);

int fn_connect_all( FBFR32 *ptr_fml_ibuf,
                    char *c_ServiceName,
                    char *c_err_msg);

int fn_fwdng_all (FBFR32 *ptr_fml_ibuf,
                  char *c_ServiceName,
                  char *c_err_msg);
/** 1.2 Ends **/

int fn_prcs_m_stts ( FBFR32 *ptr_fml_ibuf,
										 char *c_pipe_id,
                     char *c_ServiceName,
                     char *c_err_msg);			/*** Ver 1.4 ***/

int fn_check_status ( char *c_prcs_nm,
                      char *c_xchng_cd,
                      long *li_pro_id,
                      char *c_serviceName,
                      char *c_errmsg);   /** Ver 1.8 **/

/*** Ver 1.8 Starts Here ***/
int fo_upd_opm_reconct_dtls( char *c_ServiceName,
                             char *c_errmsg,
                             char *c_pipe_id,
                             char *c_xchng_cd,
                             char *c_server_flg,
                             int  *i_contn_cntr
                           );
/*** Ver 1.8 Ends Here ***/

/*** Ver 2.9 Starts ***/
int fn_checkNstart_adap (	char *c_ip_address,
													char *c_return_str,
													char *c_return_str_msg,
                      		char *c_ServiceName,
                      		char *c_err_msg);

int fn_checkNstop_adap ( char *c_ip_address , char *c_return_str , char *c_return_str_msg , char *c_ServiceName , char *c_err_msg );

int fn_checkStatus_adap	(	char *c_ip_address,
													char *c_return_str,
                          char *c_ServiceName,
                          char *c_err_msg);

int fn_get_adap ( char *c_return_str ,
									char *c_return_str1 ,
                  char *c_ServiceName ,
                  char *c_err_msg );
/*** Ver 2.9 Ends ***/

void SFO_CON_CLNT ( TPSVCINFO *ptr_st_ti_rqst )
{
	int i_ch_val;
	int i_errno;
  int i_tmp;
	int i_cnt;
	int i_ferr [ 4 ];		/*** Ver 1.5 changed from 3 to 4 ***/
	int i_err [ 4 ];		/*** Ver 1.5 changed from 3 to 4 ***/
	int i_tran_id;
	int i_adm_stts;
	
	char c_rqst_typ;
	char c_rqst_typ1 = '\0';		/*** Ver 2.9 ***/
	char c_ip_address[20];			/*** Ver 2.9 ***/
	char c_return_str[256];			/*** Ver 2.9 ***/
	char c_return_str_msg[256];			/*** Ver 2.9 ***/
	char c_return_str1[256];		/*** Ver 2.9 ***/
	char c_return_str_add[256];     /*** Ver 2.9 ***/
  char c_return_str_add1[256];    /*** Ver 2.9 ***/
	char c_prcs_nm[15];
	char c_ServiceName [ 33 ];
	char c_err_msg [ 256 ];
  char c_flag = '\0';                                 /**** Ver 2.1 ****/ 
	char c_usr_id [ 9 ];
	char c_all_status[15];
	char c_pipe_id [ LEN_PIPE_ID ];
	char c_pipe_id1 [ LEN_PIPE_ID ];
	char c_pipe_ctgry[2];							/** 1.3 **/
	char c_pipe_catg_lst[100];			  /** 1.3 **/
	char c_pipe_prmy_lst[100];			  /** 1.3 **/
	char c_pipe_secdry_lst[100];			/** 1.3 **/
	char c_new_pass_wd[LEN_PASS_WD];  /** 1.5 **/
  char c_msg[256];									/** 1.5 **/
  char c_crnt_passwd[10];           /** Ver 1.7 **/
  char c_command[100];              /** Ver 1.7 **/ 
  char c_char;                      /** Ver 1.7 **/ 
  char c_auto_reconnect='\0';       /** Ver 1.8 **/
  char c_server_flag_new='\0'; /** Ver 1.8 **/
  char c_actv_mode='\0'; /** Ver 2.2 **/

	int   i_login_stts;								/** 1.5 **/

	struct vw_mkt_msg st_msg;					/** 1.5 **/
	struct st_req_q_data st_q_con_clnt;    /** Ver 2.4 **/
	
	TPINIT *ptr_st_tpinfo;
	
	/**** Decalre input and output buffer ****/
	FBFR32 *ptr_fml_ibuf;
	FBFR32 *ptr_fml_obuf;

	strcpy ( c_ServiceName, "SFO_CON_CLNT" );

	INITDBGLVL(c_ServiceName);			

	/**** Read the input into a fml buffer ****/
	ptr_fml_ibuf = (FBFR32 *)ptr_st_ti_rqst->data;
	ptr_fml_obuf = (FBFR32*)NULL ;  

	/*Fprint32(ptr_fml_ibuf); */       /** 1.3 **/

	MEMSET(c_all_status);	
	MEMSET(st_q_con_clnt);            /*** Ver 2.4 ***/
	MEMSET(c_return_str);							/*** Ver 2.9 ***/
	MEMSET(c_return_str_msg);					/*** Ver 2.9 ***/
	MEMSET(c_return_str1);						/*** Ver 2.9 ***/
	MEMSET(c_return_str_add);					/*** Ver 2.9 ***/
	MEMSET(c_return_str_add1);				/*** Ver 2.9 ***/

	/**** Read the input buffer into input structure ****/
  i_err [ 0 ] = Fget32(ptr_fml_ibuf, FFO_RQST_TYP, 0, (char *)&c_rqst_typ, 0);
	i_ferr [ 0 ] = Ferror32;
	i_err [ 1 ] = Fget32(ptr_fml_ibuf,FFO_PIPE_ID,0,(char *)c_pipe_id, 0);
  i_ferr [ 1 ] = Ferror32;
	i_err [ 2 ] = Fget32(ptr_fml_ibuf, FFO_USR_ID, 0, (char *)c_usr_id, 0);
	i_ferr [ 2 ] = Ferror32;
/***	i_err [ 3 ] = Fget32(ptr_fml_ibuf,FFO_NEW_PASS_WD,0,(char *)c_new_pass_wd,0);		** Ver 1.5 **
  i_ferr [ 3 ] = Ferror32;																												** Ver 1.5 **
  ************** Ver 1.7 ****************/
	for ( i_cnt=0; i_cnt<=2; i_cnt++ )
	{
  	if (i_err[i_cnt] == -1)
		{
    	fn_userlog(c_ServiceName, "Error in field no. %d",i_cnt);	
			fn_errlog(c_ServiceName, "S31005", Fstrerror32(i_ferr[i_cnt]),c_err_msg );
			Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
			tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
		}
	}

  /**** Ver 2.2 Starts Here ****/

    if(DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "c_rqst_typ is....:%c:",c_rqst_typ);
    }

  if( c_rqst_typ == USR_AUTHN )
  {
    if(DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "Inside User Authentication");
    }

    EXEC SQL
    SELECT NVL(ACTIVE,'N')
      INTO :c_actv_mode
      FROM FNO_Console_Users
     WHERE UserId=:c_usr_id;

    if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
    {
      fn_errlog(c_ServiceName, "S31010", SQLMSG, c_err_msg );
      strcpy(c_msg,"Login is not Successful");
      Fadd32( ptr_fml_ibuf, FFO_REMARKS, c_msg, 0 );
      Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
    }

		if(SQLCODE == NO_DATA_FOUND)
		{
			c_actv_mode='N';
		}

    if(DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "c_actv_mode....:%c:",c_actv_mode);
    }

    if(c_actv_mode == 'Y')
    {
      strcpy(c_msg,"Login Successful");
      Fadd32( ptr_fml_ibuf, FFO_REMARKS, c_msg, 0 );
      tpreturn( TPSUCCESS, SUCC_BFR, (char *)ptr_fml_ibuf, 0 , 0 );
    }
    else if(c_actv_mode == 'N')
    {
      fn_errlog(c_ServiceName, "S31015", "Login Unsuccessful", c_err_msg );
      strcpy(c_err_msg,"Login Unsuccessful");
      Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
    }
  }

  /**** Ver 2.2 Ends Here ****/

  /**** Ver 1.8 Starts Here *******/
  i_ch_val = Fget32(ptr_fml_ibuf,FFO_OPERATION_TYP,0,(char *)&c_auto_reconnect,0);
  i_ferr[0] = Ferror32;
  if( i_ch_val == -1 )
  {
    if( i_ferr[0] == FNOTPRES )
    {
      c_auto_reconnect = 'N';
    }
    else
    {
      fn_errlog( c_ServiceName, "S31020", FMLMSG, c_err_msg );
      Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 ); 
    }
  }

  i_ch_val = Fget32(ptr_fml_ibuf,FFO_STATUS_FLG,0,(char *)&c_server_flag_new,0);
  i_ferr[0] = Ferror32;
  if( i_ch_val == -1 )
  {
     if( i_ferr[0] == FNOTPRES )
     {
       c_server_flag_new = 'P';
     }
     else
     {
        fn_errlog( c_ServiceName, "S31025", FMLMSG, c_err_msg );
      Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
     }
  }
 
  /**** Ver 1.8 Ends Here ********/

	if(i_err[3] == -1) /** Ver 1.5 **/
  {
    strcpy(c_new_pass_wd, "\0");
  }

	fn_userlog ( c_ServiceName, "|%s,%s| Service requested - |%c|", c_usr_id, c_pipe_id, c_rqst_typ );

	if ( strcmp ( c_usr_id, "system" ) != 0 )
	{
		fn_errlog(c_ServiceName, "B28501", DEFMSG, c_err_msg );
		Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
	}

	/*** if( c_rqst_typ != RESET_PWD) * Ver 1.5 * Commented in Ver 1.6 ***/
	if( c_rqst_typ != RESET_PWD && c_rqst_typ != CONNECT_ALL && c_rqst_typ != FRWD_ALL && c_auto_reconnect !='Y'  && c_rqst_typ != PRCS_M_STTS_SCRPT && c_rqst_typ != MANUAL_DC_ADAP )		/*** Ver 1.6 ***/  /** ver 1.8 , c_auto_reconnect added **/	/*** Ver 2.9 MANUAL_DC_ADAP Added ***/
	/*** PRCS_M_STTS_SCRPT added in Ver 2.8 ***/
	{
	EXEC SQL
		SELECT opm_adm_stts
		INTO   :i_adm_stts
		FROM   opm_ord_pipe_mstr
		WHERE  opm_pipe_id = :c_pipe_id;

	if ( SQLCODE != 0 )
	{
		fn_errlog(c_ServiceName, "S31030", SQLMSG, c_err_msg );
		Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
	}		


	if ( c_rqst_typ != LOG_ON_REQ )
	{
		if ( i_adm_stts != 1 )
		{
			fn_errlog(c_ServiceName, "B28503", DEFMSG, c_err_msg );
			Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
			tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
		}
	}
	else
	{
		if ( i_adm_stts == 1 )
		{
			fn_errlog(c_ServiceName, "B28505", DEFMSG, c_err_msg );
			Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
			tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
		}
	}
  } /* Ver 1.5 */
	/**** Main routine ****/

	i_ch_val =0;

  switch (c_rqst_typ)
	{
		
		/** Ver 1.5 Starts **/

		case RESET_PWD :

			/**** Get current exchange password  ****/
			EXEC SQL
				SELECT  OPM_LOGIN_STTS
					INTO  :i_login_stts
					FROM  OPM_ORD_PIPE_MSTR
					WHERE OPM_PIPE_ID = :c_pipe_id
					AND   OPM_XCHNG_CD = 'NFO';
			
			if ( SQLCODE != 0 )
			{
			 fn_userlog(c_ServiceName," Error in getting Login stts");
			 fn_errlog(c_ServiceName, "S31035", SQLMSG, c_err_msg );
       tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
			}

			if( i_login_stts == 0)
			{
				fn_userlog(c_ServiceName," User has not logged in for pipe id :%s:",c_pipe_id);
				fn_errlog(c_ServiceName, "S31040", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
			}
       /******** Comented in Ver 1.7 *********************************
   		 if (strlen(c_new_pass_wd) < 8 )
   		 {
          sprintf(c_err_msg," NEW PASSWORD LENGTH CAN'T BE LESS THAN 8 CHARACTERS");
          fn_userlog(c_ServiceName, "%s", c_err_msg );
          fn_errlog(c_ServiceName, "S31045", LIBMSG, c_err_msg );
          Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
   		 }
   		 if (strlen(c_new_pass_wd) > 8 )
   		 {
					sprintf(c_err_msg,"PLEASE ENTER THE PASSWORD OF 8 CHARACTER LENGTH [eg. abcdef11 ]");
     			fn_userlog(c_ServiceName, "%s", c_err_msg );
        	fn_errlog(c_ServiceName, "S31050", LIBMSG, c_err_msg );
        	Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );

   		 }
       *********** Ver 1.7 Ends Here ********************************/

        /**** Ver 1.7 Starts Here  ***/
        MEMSET(c_crnt_passwd);
        MEMSET(c_new_pass_wd);

        EXEC SQL
          SELECT OPM_EXG_PWD
            INTO :c_crnt_passwd
            FROM OPM_ORD_PIPE_MSTR
           WHERE OPM_PIPE_ID=:c_pipe_id
             AND OPM_XCHNG_CD='NFO';
        if ( SQLCODE != 0 )
        {
          fn_userlog(c_ServiceName," Error in getting Login stts");
          fn_errlog(c_ServiceName, "S31055", SQLMSG, c_err_msg );
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
        }
      
        rtrim(c_crnt_passwd);

        fn_decrypt(c_crnt_passwd,c_crnt_passwd);

        fn_userlog(c_ServiceName,"Existingg password :%s:",c_crnt_passwd);

        fn_get_passwd(c_crnt_passwd,c_new_pass_wd,CRNT_PSSWD_LEN);

        fn_userlog(c_ServiceName,"New password Generated :%s:",c_new_pass_wd); 
 
        rtrim(c_new_pass_wd); 

        fn_encrypt(c_new_pass_wd,c_new_pass_wd);

        fn_userlog(c_ServiceName,"Encrypted password :%s:",c_new_pass_wd); 

        /**** Ver 1.7 Ends Here ***/

	     i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
 	     if ( i_tran_id == -1 )
       {
      	  fn_errlog(c_ServiceName, "S31060", LIBMSG, c_err_msg );
       	  Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
       	  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       }
	
				EXEC SQL UPDATE OPM_ORD_PIPE_MSTR
								 SET OPM_NEW_EXG_PWD=:c_new_pass_wd
								 WHERE OPM_PIPE_ID = :c_pipe_id				/*** LIKE DECODE(:c_pipe_id,'*','%',:c_pipe_id) ***/
								 AND OPM_XCHNG_CD = 'NFO';
				
				if ( SQLCODE != 0 )
        {
            fn_errlog(c_ServiceName, "S31065", SQLMSG, c_err_msg );
						fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
            tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
        }	
 
				i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
        if ( i_ch_val == -1 )
        {
						fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
	       		fn_errlog(c_ServiceName, "S31070", LIBMSG, c_err_msg );
        		Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      	}

        /*** Ver 1.7 Starts Here ****/
        MEMSET(c_command);
        fn_decrypt(c_new_pass_wd,c_new_pass_wd);
        fn_userlog(c_ServiceName,"Decrypted password :%s:",c_new_pass_wd);
        c_char='"';
        
        sprintf(c_command,"ksh snd_passwd_chng_mail.sh %s '%c%s%c'", c_pipe_id,c_char,c_new_pass_wd,c_char);
        system(c_command);  
        /*** Ver 1.7 Ends Here ***/
        
				EXEC SQL
					SELECT to_char(sysdate, 'dd-Mon-yyyy hh24:mi:ss') INTO :st_msg.c_tm_stmp FROM dual;


			  sprintf(c_msg,"|%s- %s|",st_msg.c_tm_stmp,"Password changed successfully");

			  strcpy( st_msg.c_xchng_cd,"NFO");
			  strcpy(st_msg.c_msg,c_msg);
			  st_msg.c_msg_id = TRADER_MSG;


				MEMSET(st_msg.c_brkr_id);

				EXEC SQL
						SELECT  exg_brkr_id
						INTO    :st_msg.c_brkr_id
						FROM    exg_xchng_mstr
						WHERE   exg_xchng_cd = :st_msg.c_xchng_cd;

				if ( SQLCODE != 0 )
				{
					fn_errlog(c_ServiceName,"S31075", SQLMSG, c_err_msg);
					Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
				}
			
				fn_cpy_ddr(st_msg.c_rout_crt);
				strcpy(st_msg.c_tm_stmp,"06-Mar-2013 16:01:38");
				if(DEBUG_MSG_LVL_0){
				 fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_msg.c_xchng_cd);
				 fn_userlog(c_ServiceName,"Broker ID Is :%s:",st_msg.c_brkr_id);
				 fn_userlog(c_ServiceName,"Message Is :%s:",st_msg.c_msg);
				 fn_userlog(c_ServiceName,"Time Stamp  Is :%s:",st_msg.c_tm_stmp);
				 fn_userlog(c_ServiceName,"Message ID Is :%c:",st_msg.c_msg_id);
 				 fn_userlog(c_ServiceName,"Rout Crt Is :%s:",st_msg.c_rout_crt);
				 fn_userlog(c_ServiceName,"Before Call to SFO_UPD_MSG");
				 }

				i_ch_val = fn_acall_svc ( c_ServiceName,
																	c_err_msg,
																	&st_msg,
																	"vw_mkt_msg",
																	sizeof (struct vw_mkt_msg ),
																	TPNOREPLY,
																	"SFO_UPD_MSG" );

				if ( i_ch_val == SYSTEM_ERROR )
				{
						fn_errlog ( c_ServiceName, "S31080", LIBMSG, c_err_msg );
						Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
						tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
				}
									
		
			  sprintf(c_err_msg,"Password Changed Successfully");
				fn_userlog(c_ServiceName, "New Password is :: %s", c_new_pass_wd );
        
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
				tpreturn( TPSUCCESS, SUCC_BFR, (char *)ptr_fml_ibuf, 0 , 0 );
				break;  
				
				/** Ver 1.5 Ends **/

		case LOG_ON_REQ :


			/******************* Ver 1.3 Starts ******************************/

				EXEC SQL
						 DECLARE cur_pipe_catg_list CURSOR FOR
						 SELECT  opm_pipe_id,
										 opm_pipe_categary 
						 FROM		 opm_ord_pipe_mstr
						 WHERE   opm_xchng_cd = 'NFO';

				if ( SQLCODE != 0 )
  			{
						fn_errlog(c_ServiceName, "S31085", SQLMSG, c_err_msg );
						EXEC SQL
							CLOSE cur_pipe_catg_list;
						tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
  			}


				EXEC SQL
    				 OPEN  cur_pipe_catg_list;

			  MEMSET(c_pipe_catg_lst);
				MEMSET(c_pipe_id1);
				MEMSET(c_pipe_ctgry); 
				MEMSET(c_pipe_prmy_lst); 
				MEMSET(c_pipe_secdry_lst); 

			  strcpy(c_pipe_prmy_lst,"P-");
			  strcpy(c_pipe_secdry_lst,"F-");

				while (1)
				{

						EXEC SQL
								 FETCH cur_pipe_catg_list
						INTO	:c_pipe_id1,
									:c_pipe_ctgry;

						if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
						{
								fn_errlog(c_ServiceName, "S31090", SQLMSG, c_err_msg );
            		EXEC SQL
              		CLOSE cur_pipe_catg_list;
            			tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
						}

						if ( SQLCODE == NO_DATA_FOUND )
						{
							break;
						}

						rtrim(c_pipe_id1);
						rtrim(c_pipe_ctgry);

						if( strcmp(c_pipe_ctgry,"P") == 0 )
						{
							strcat(c_pipe_prmy_lst,(char *)c_pipe_id1);
							strcat(c_pipe_prmy_lst,":");
						}
						else if( strcmp(c_pipe_ctgry,"F") == 0 ) 
						{

							strcat(c_pipe_secdry_lst,(char *)c_pipe_id1);
              strcat(c_pipe_secdry_lst,":");

						}

				}

				strcpy(c_pipe_catg_lst,(char *)c_pipe_prmy_lst);
				strcat(c_pipe_catg_lst,",");
        strcat(c_pipe_catg_lst,(char *)c_pipe_secdry_lst);

				fn_userlog(c_ServiceName,"========== c_pipe_catg_lst [%s]",c_pipe_catg_lst);

				i_ch_val = Fadd32(ptr_fml_ibuf,FFO_PSTN_FLG,(char *)c_pipe_catg_lst,0);
        if( i_ch_val == -1 )
        {
             fn_errlog( c_ServiceName, "S31095", FMLMSG, c_err_msg );
						 EXEC SQL
              CLOSE cur_pipe_catg_list;
             tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
        }

				EXEC SQL
						 CLOSE cur_pipe_catg_list;
				
			/******************* Ver 1.3 Ends ******************************/

			i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
			if ( i_tran_id == -1 )
			{
				fn_errlog(c_ServiceName, "S31100", LIBMSG, c_err_msg );
				Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
				tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
			}

			EXEC SQL
				UPDATE opm_ord_pipe_mstr
				SET    opm_adm_stts = 1
				WHERE  opm_pipe_id  = :c_pipe_id;

			if ( SQLCODE != 0 )
			{
				fn_errlog(c_ServiceName, "S31105", SQLMSG, c_err_msg );
				Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
				fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
				tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
			}
		
			i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
			if ( i_ch_val == -1 )
			{
				fn_errlog(c_ServiceName, "S31110", LIBMSG, c_err_msg );
				Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
				tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
			}


       tpreturn( TPSUCCESS, SUCC_BFR, (char *)ptr_fml_ibuf, 0 , 0 );    /** ver 1.3 **/

			break;		
			
		case LOG_OFF_REQ :

        i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
        if ( i_tran_id == -1 )
        {
          fn_errlog(c_ServiceName, "S31115", LIBMSG, c_err_msg );
          Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
        }

        EXEC SQL
          UPDATE opm_ord_pipe_mstr
          SET    opm_adm_stts = 0
          WHERE  opm_pipe_id = :c_pipe_id;

        if ( SQLCODE != 0 )
        {
          fn_errlog(c_ServiceName, "S31120", SQLMSG, c_err_msg );
          Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
          fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
        }

        i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
        if ( i_ch_val == -1 )
        {
          fn_errlog(c_ServiceName, "S31125", LIBMSG, c_err_msg );
          Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
        }
        break;		
        
		case STRT_ESR_CLNT :
			i_ch_val = fn_strt_esr_clnt ( c_pipe_id,
                                    c_auto_reconnect,            /*** Ver 1.8 , N- Normal Connect **/
                                    c_server_flag_new,           /*** Ver 1.8, P -Primary TAP connect **/ 
																		c_ServiceName,
   																  c_err_msg); 

			if ( i_ch_val == -1 )
      {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31130", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }

			break;

		case	STRT_PACK_CLNT:
			i_ch_val = fn_strt_pack_clnt (c_pipe_id,
																		ptr_fml_ibuf,
																		c_ServiceName,
                                    c_err_msg);	

			if ( i_ch_val == -1 )
      {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31135", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
			break;

		case LOG_ON_TO_TAP :

			i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
			if ( i_tran_id == -1 )
			{
				fn_errlog(c_ServiceName, "S31140", LIBMSG, c_err_msg );
				Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
				tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
			}

			i_ch_val = fn_logon_to_TAP( ptr_fml_ibuf,
																	  c_ServiceName,
   																  c_err_msg,
																		&st_q_con_clnt    /** Ver 2.4 **/
																 );

			if ( i_ch_val == -1 )
      {
				strcat(c_err_msg ," **** Login failed ****");
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31145", LIBMSG, c_err_msg );
				fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );	
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }

			i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
			if ( i_ch_val == -1 )
			{
				fn_errlog(c_ServiceName, "S31150", LIBMSG, c_err_msg );
				Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
				tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
			}

			/** Ver 2.4 starts here **/

      if(DEBUG_MSG_LVL_0)
      {
        fn_userlog(c_ServiceName,"i_send_qid :%d:",i_send_qid);
      }

      if( fn_write_msg_q  ( i_send_qid,
                            (void *)&st_q_con_clnt,
                            sizeof(st_q_con_clnt),
                            c_ServiceName,
                            c_err_msg)    == -1 )

      {
        fn_errlog(c_ServiceName, "S31155", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }

      if(DEBUG_MSG_LVL_0)
      {
        fn_userlog(c_ServiceName,"Ends fn_write_msg_q-fn_logon_to_TAP");
      }

			/** Ver 2.4 Ends here ***/

			break;

		case STRT_FWD_ORDS :
		/**------------------- 1.2 Commented  ----------------	
			fn_pst_trg( c_ServiceName,
								 "TRG_STRT_PMP",
							   "TRG_STRT_PMP",
							   c_pipe_id);

		----------------------- 1.2 Commented ---------------**/	
		
		/** 1.2 Starts **/		

		i_ch_val =  fn_chk_n_strt_frwrdng(c_pipe_id,
                                      c_ServiceName,
                                      c_err_msg);

			
		break;

		/** 1.2 Ends **/		

		/** 1.1 Starts **/

		case STRT_BRD_CLNT :

			c_flag = 'E';                           /*** Added in Ver 2.1 ***/
			
			i_ch_val = fn_req_brd_chnl(c_pipe_id,
                                 c_ServiceName,
                                 c_err_msg,
                                 c_flag);     /*** Added in Ver 2.1 ***/

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31160", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;
		/** 1.1 Ends **/

		      /***** Added in Ver 2.1 starts *****/

    case STRT_NEW_BRD_CLNT :

    c_flag = 'N';
    i_ch_val = fn_req_brd_chnl(c_pipe_id,
                               c_ServiceName,
                               c_err_msg,
                               c_flag);

     if ( i_ch_val == -1 )
     {
       fn_userlog(c_ServiceName,"%s",c_err_msg);
       fn_errlog(c_ServiceName, "S31165", LIBMSG, c_err_msg );
       Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
       tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
     }
     break;
     /***** Added in Ver 2.1 ends *****/

		case STOP_FWD_ORDS :
			fn_pst_trg( c_ServiceName, 
								 "TRG_STOP_PMP", 
								 "TRG_STOP_PMP", 
								 c_pipe_id);
			break;

		case SEND_RECO_OVER :
       fn_pst_trg( c_ServiceName,
                 "TRG_RECO_OVER",
                 "TRG_RECO_OVER",
                 c_pipe_id);
      break;

		case STOP_PACK_CLNT	:
		  i_ch_val =	fn_stop_pack_clnt(c_pipe_id,
																	  c_ServiceName,
																    c_err_msg);	

			if ( i_ch_val == -1 )
      {
				fn_userlog(c_ServiceName,"%s",c_err_msg);	
        fn_errlog(c_ServiceName, "S31170", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
			break;

		case LOGOFF_FROM_TAP :
			i_ch_val = fn_logoff_from_TAP( c_pipe_id,
																		 c_ServiceName,
																		 c_err_msg);
			if ( i_ch_val == -1 )
      {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31175", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
			break;

		case STOP_ESR_CLNT :	
			i_ch_val = fn_stop_esr_clnt( c_pipe_id,
																	 c_ServiceName,	
																   c_err_msg);
			 if ( i_ch_val == -1 )
       {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31180", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       }
			break;

		case SET_ORD_MKT_OPN:
			i_ch_val = fn_mkt_stts_chng( "TRG_ORD_OPN",
																	  ORDER_MKT,
																		OPEN,
																		c_pipe_id,
								 		  							c_ServiceName, 
                      							c_err_msg);
			 if ( i_ch_val == -1 )
       {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31185", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       }
			break;

		case STOP_BCAST_CLNT :
	
			c_flag = 'E';																		/*** Added in Ver 2.1 ***/

			i_ch_val = fn_stop_brd_clnt (c_pipe_id,
                                   c_ServiceName,
                                   c_err_msg,
                                    c_flag);          /*** Added in Ver 2.1 ***/

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31190", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;

			       /*** Added in Ver 2.1 starts ***/

      case STOP_NEW_BCAST_CLNT :

       c_flag = 'N';
       i_ch_val = fn_stop_brd_clnt (c_pipe_id,
                                    c_ServiceName,
                                    c_err_msg,
                                    c_flag);

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31195", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;

       /*** Added in Ver 2.1 ends ***/


			/**** Ver 1.2 starts *********/
			
		 case STATUS_ALL_PROCSS:
					i_ch_val = fn_all_status (c_pipe_id,
																	  c_all_status,
																	  ptr_fml_ibuf,
                                		c_ServiceName,
                                		c_err_msg); 	 

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31200", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
			}

		    break;

			/**** Ver 1.2 Ends *********/

		case SET_ORD_MKT_CLS:
			i_ch_val = fn_mkt_stts_chng( "TRG_ORD_CLS",
																	  ORDER_MKT,
																		CLOSED,
																		c_pipe_id,
								 		  							c_ServiceName, 
                      							c_err_msg);
			 if ( i_ch_val == -1 )
      {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31205", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
			break;

		case SET_EXR_MKT_OPN:
			i_ch_val = fn_mkt_stts_chng( "TRG_EXR_OPN",
																	  EXERCISE_MKT,
																		OPEN,
																		c_pipe_id,
								 		  							c_ServiceName, 
                      							c_err_msg);
			 if ( i_ch_val == -1 )
       {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31210", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       }
			break;

		case SET_EXR_MKT_CLS:
			i_ch_val = fn_mkt_stts_chng( "TRG_EXR_CLS",
																	  EXERCISE_MKT,
																		CLOSED,
																		c_pipe_id,
								 		  							c_ServiceName, 
                      							c_err_msg);
			 if ( i_ch_val == -1 )
       {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31215", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       }
			break;

		case SET_EXT_MKT_OPN:
			i_ch_val = fn_mkt_stts_chng( "TRG_EXT_OPN",
																	  EXTND_MKT,
																		OPEN,
																		c_pipe_id,
								 		  							c_ServiceName, 
                      							c_err_msg);
			 if ( i_ch_val == -1 )
       {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31220", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       }
			break;

		case SET_EXT_MKT_CLS:
			i_ch_val = fn_mkt_stts_chng( "TRG_EXT_CLS",
																	  EXTND_MKT,
																		CLOSED,
																		c_pipe_id,
								 		  							c_ServiceName, 
                      							c_err_msg);
			 if ( i_ch_val == -1 )
       {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31225", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       }
			break;			

		
    /** Ver 1.2 Starts **/

    case CONNECT_ALL:

      fn_userlog(c_ServiceName,"------------------ CONNECT_ALL -------------");
      i_ch_val = fn_connect_all(ptr_fml_ibuf,
                                c_ServiceName,
                                c_err_msg);

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31230", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;

     case FRWD_ALL:

     fn_userlog(c_ServiceName,"------------------ FWD_ALL -----------------");
     i_ch_val = fn_fwdng_all(ptr_fml_ibuf,
                                c_ServiceName,
                                c_err_msg);

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31235", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;

    /** Ver 1.2 Ends **/	

		/*** Ver 1.4 starts ***/
		case	PRCS_M_STTS:
		case	PRCS_M_STTS_SCRPT:		/*** added in Ver 2.8 ***/
	    fn_userlog(c_ServiceName,"------------------ PRCS_M_STTS -----------------");
			i_ch_val = fn_prcs_m_stts ( ptr_fml_ibuf,
																	c_pipe_id,
																	c_ServiceName,
																	c_err_msg);	

			if ( i_ch_val == -1 )
      {
				fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31240", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
			break;
		/*** Ver 1.4 Ends ***/

		/*** Ver 2.9 Starts ***/

		case	MANUAL_DC_ADAP:

		i_err [ 0 ] = Fget32(ptr_fml_ibuf, FFO_REQ_TYP, 0, (char *)&c_rqst_typ1, 0);
		i_ferr[0] = Ferror32;
  	if( i_ferr[0] == -1 )
  	{
			fn_errlog( c_ServiceName, "S31245", FMLMSG, c_err_msg );
      Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
		}

		i_err [ 1 ] = Fget32(ptr_fml_ibuf, FFO_SETTLOR, 0, (char *)&c_ip_address, 0);
    i_ferr[1] = Ferror32;
    if( i_ferr[1] == -1 )
    {
      fn_errlog( c_ServiceName, "S31250", FMLMSG, c_err_msg );
      Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
    }

		if( c_rqst_typ1 == START_DC_ADAPTER )
		{
      fn_userlog(c_ServiceName,"------------------ START_DC_ADAPTER -----------------");

      i_ch_val = fn_checkNstart_adap ( c_ip_address,
																			 c_return_str,
																			 c_return_str_msg,
                                       c_ServiceName,
                                       c_err_msg );
      if ( i_ch_val != 0 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31255", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
		}
		else if ( c_rqst_typ1 == STOP_DC_ADAPTER )
		{
			fn_userlog(c_ServiceName,"------------------ STOP_DC_ADAPTER -----------------");

			i_ch_val = fn_checkNstop_adap ( c_ip_address,
																			c_return_str,
																			c_return_str_msg,
																			c_ServiceName,
																			c_err_msg );
			if ( i_ch_val != 0 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31260", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
		}
		else if ( c_rqst_typ1 == STATUS_DC_ADAPTER )
		{
			fn_userlog(c_ServiceName,"------------------ STATUS_DC_ADAPTER -----------------");

			i_ch_val = fn_checkStatus_adap ( c_ip_address,
																			 c_return_str,
                                       c_ServiceName,
                                       c_err_msg );
			if ( i_ch_val != 0 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31265", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
		}
		else if ( c_rqst_typ1 == GET_DC_ADAPTER_IP )
		{
			fn_userlog(c_ServiceName,"------------------ GET_DC_ADAPTER_IP -----------------");

			i_ch_val = fn_get_adap ( c_return_str,
															 c_return_str1,
                               c_ServiceName,
                               c_err_msg );
			if ( i_ch_val != 0 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31270", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }			
		}

		break;
		/*** Ver 2.9 Ends ***/

		default:
			i_ch_val = -1;
	    fn_errlog( c_ServiceName, "S31275", "Protocol error", c_err_msg );
			break;

	}	/*** switch ***/

	/*** common error checking and error logging of all above cases	***/

	if ( i_ch_val == -1 )
	{
		Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn(TPFAIL, ERR_BFR, ( char * )ptr_fml_ibuf, 0, 0 );
	}


	/**  status ***/
	if ( i_ch_val != -1 && c_rqst_typ == STATUS_ALL_PROCSS )
	{

			    /**** Allocating output buffer ****/

          ptr_fml_obuf = (FBFR32 *)tpalloc("FML32",NULL,MIN_FML_BUF_LEN);
          if (ptr_fml_obuf == NULL)
          {
              fn_errlog( c_ServiceName, "S31280", TPMSG, c_err_msg );
              return ;
          }

          /**** Adding value in Output buffer ****/

          i_ch_val = Fadd32(ptr_fml_obuf,FFO_REMARKS,(char *)c_all_status,0);

          /***** Returning Value in output buffer **/

				tpreturn( TPSUCCESS, SUCC_BFR, (char *)ptr_fml_obuf, 0 , 0 );

	} 
	/*** Ver 2.9 Starts ***/
	if ( i_ch_val == 0 && c_rqst_typ == MANUAL_DC_ADAP )
	{
		/**** Allocating output buffer ****/

          ptr_fml_obuf = (FBFR32 *)tpalloc("FML32",NULL,MIN_FML_BUF_LEN);
          if (ptr_fml_obuf == NULL)
          {
              fn_errlog( c_ServiceName, "S31285", TPMSG, c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, ( char * )ptr_fml_ibuf, 0, 0 );
          }

          /**** Adding value in Output buffer ****/

					fn_userlog ( c_ServiceName , "Before Adding in FFO_REMARKS c_return_str is :%s:",c_return_str );

					strcpy(c_return_str_add,c_return_str);

					char* add_token = strtok(c_return_str_add, ",");
					while (add_token != NULL)
					{
						fn_userlog ( c_ServiceName , " add_token before Fadd is :%s:",add_token);
						i_ch_val = Fadd32(ptr_fml_obuf,FFO_REMARKS,(char *)add_token,0);
          	if( i_ch_val == -1 )
          	{
            	fn_errlog( c_ServiceName, "S31290", FMLMSG, c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, ( char * )ptr_fml_ibuf, 0, 0 );
          	}
						add_token = strtok(NULL, ",");
					}

					if( c_rqst_typ1 == GET_DC_ADAPTER_IP )
					{
						fn_userlog ( c_ServiceName , "Before Adding in FFO_REF_NO c_return_str1 is :%s:",c_return_str1 );

						strcpy(c_return_str_add1,c_return_str1);

						char* add_token1 = strtok(c_return_str_add1, ",");

          	while (add_token1 != NULL)
          	{
            	fn_userlog ( c_ServiceName , " add_token1 before Fadd is :%s:",add_token1);
            	i_ch_val = Fadd32(ptr_fml_obuf,FFO_REF_NO,(char *)add_token1,0);
            	if( i_ch_val == -1 )
            	{
              	fn_errlog( c_ServiceName, "S31295", FMLMSG, c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, ( char * )ptr_fml_ibuf, 0, 0 );
            	}
            	add_token1 = strtok(NULL, ",");
          	}
					}

					if ( c_rqst_typ1 == START_DC_ADAPTER || c_rqst_typ1 == STOP_DC_ADAPTER )
					{
						i_ch_val = Fadd32(ptr_fml_obuf,FFO_SERIES,(char *)c_return_str_msg,0);
						if( i_ch_val == -1 )
            {
							fn_errlog( c_ServiceName, "S31300", FMLMSG, c_err_msg );
              tpreturn(TPFAIL, ERR_BFR, ( char * )ptr_fml_ibuf, 0, 0 );
						}
					}

          /***** Returning Value in output buffer **/

        tpreturn( TPSUCCESS, SUCC_BFR, (char *)ptr_fml_obuf, 0 , 0 );
	}
	/*** Ver 2.9 Ends ***/

		tpreturn(TPSUCCESS, SUCC_BFR, NULL, 0, 0 );
}

/*****************************************************************************/
/*																																					 */
/*	Description 	    :		Sending the request to start the ESR server process*/
/* 																																					 */
/*	Input Parameters	:		Pipe id,Error Message,Service Name                 */
/* 																																					 */
/*	Output Parameters	:		Returns 0 on successful start up and -1 on failure.*/
/*																																					 */
/*****************************************************************************/	
int fn_strt_esr_clnt ( char *c_pipe_id, char c_auto_reconnect,  /** Ver 1.8 **/  char c_server_flag_new,      /** Ver 1.8 **/  char *c_ServiceName,  char *c_err_msg)
{

	int i_ch_val;
  int i_run_counter=0; /** Ver 1.8 **/
  char sql_c_clnt_stts='N'; /** Ver 1.8 **/
	char c_cmd_ln[256];
  char c_msg[200];  /** Ver 1.8 **/
  long  li_prcs_id; /** Ver 1.8 **/
  char  c_command[150]; /** Ver 1.8 **/

	fn_userlog(c_ServiceName,"Log1");
	fn_userlog(c_ServiceName,"Log2");

  if( c_auto_reconnect == 'Y' )
  {
					/**** Ver 2.6 starts ****/
					i_ch_val = fn_check_status ("cln_esr_clnt",
                                       c_pipe_id,
                                       &li_prcs_id,
                                       c_ServiceName,
                                       c_err_msg);
					if( i_ch_val == RUNNING)
					{
						fn_userlog(c_ServiceName, "cln_esr_clnt for pipe id :%s: and pid :%ld:",c_pipe_id,li_prcs_id);
            fn_userlog(c_ServiceName, "Before sleep in RUNNING status");
            sleep(2);
            fn_userlog(c_ServiceName, "After sleep in RUNNING status");
					}		
					/**** Ver 2.6 ends ****/
          i_ch_val = fn_check_status ("cln_esr_clnt",
                                       c_pipe_id,
                                       &li_prcs_id,
                                       c_ServiceName,
                                       c_err_msg);
          if( i_ch_val == RUNNING)
          {
            fn_userlog(c_ServiceName, "cln_esr_clnt for pipe id :%s: and pid :%ld:",c_pipe_id,li_prcs_id);
            MEMSET(c_command);
            strcpy(c_msg,"Unable to reconnect as cln_esr_clnt is Running in status, Pls check.");
            sprintf(c_command,"fo_xchng_relogin_mail.sh NFO %s '%s'",c_pipe_id,c_msg);
            system(c_command);
            fn_errlog(c_ServiceName, "S31305", LIBMSG, c_err_msg );
            return -1;
          }
          else
          {
            i_ch_val = fo_upd_opm_reconct_dtls(c_ServiceName,
                                               c_err_msg,
                                               c_pipe_id,
                                               "NFO",
                                               &c_server_flag_new,
                                               &i_run_counter
                                              );
            if( i_ch_val == -1 )
            {
              fn_errlog(c_ServiceName, "S31310", LIBMSG, c_err_msg );
              return -1;
            }

            fn_userlog(c_ServiceName,"c_server_flag_new :%c:",c_server_flag_new);
            fn_userlog(c_ServiceName,"i_run_counter :%d:",i_run_counter);

            /******************************************************************************/
            /** If c_server_flag_new = C then Do not reconnect                           **/
            /** If c_server_flag_new = N then Do reconnect to MAIN TAP/IML server        **/
            /** If c_server_flag_new = F then Do reconnect to FALLBACK TAP/IML server    **/
            /******************************************************************************/

            if(c_server_flag_new == 'C' || i_run_counter > 4)
            {
              fn_userlog(c_ServiceName,"Do not run :%s:", c_err_msg);
              MEMSET(c_command);
              sprintf(c_msg,"%s",c_err_msg);
              sprintf(c_command,"fo_xchng_relogin_mail.sh NFO %s '%s'",c_pipe_id,c_msg);
              system(c_command);
              return -1;
            }

            if(DEBUG_MSG_LVL_3)
            {
              fn_userlog(c_ServiceName,"Before sleep");
            }

            if(i_run_counter == 1)
            {
              sleep(1);
            }
            else
            {
              sleep(5);
            }

            if(DEBUG_MSG_LVL_3)
            {
              fn_userlog(c_ServiceName,"After sleep");
            }

            fn_userlog(c_ServiceName, "Starting the cln_esr_clnt for pipe id :%s: ",c_pipe_id);
          }
  }

	sprintf(c_cmd_ln, "nohup cln_esr_clnt %s NA %s %c %c &", c_pipe_id, c_pipe_id,c_auto_reconnect,c_server_flag_new); 
	i_ch_val = system(c_cmd_ln);	
	if (i_ch_val == -1 )
  {
    fn_errlog( c_ServiceName, "S31315", UNXMSG, c_err_msg );
    return -1;
  }

	return 0;
}

/*****************************************************************************/
/*																																					 */
/*  Description       :   For starting the Pack client process.              */
/*                                                                           */
/*  Input Parameters  :   Input Buffer, Pipe Id, Error Message, Service Name */
/*                                                                           */
/*  Output Parameters :   Returns 0 on successful start up and -1 on failure */
/*   																																				 */	
/*****************************************************************************/

int fn_strt_pack_clnt ( char *c_pipe_id, FBFR32 *ptr_fml_ibuf, char *c_ServiceName, char *c_err_msg)
{
	  int 	i_ch_val;
		int   i_login_stts;
  	int 	i_no_of_ordrs = 0;
		int		i_tran_id;					/*** Ver 1.4 ***/		
    int   i_ferr[2];          /*** Ver 1.8 ***/
		char  c_cmd_ln[256];
  	char 	c_run_mode;
  	char 	c_tm_stmp [ LEN_DT ];	
    char  sql_c_clnt_nm[40];
    char  sql_c_clnt_stts;
    char  c_auto_reconnect='N';   /*** Ver 1.8 ***/
    char *ptr_c_time;           /** Ver 1.8 **/
    long  l_from_time=0;     /** Ver 1.8 **/

	strcpy(sql_c_clnt_nm,"cln_esr_clnt");
	
  /**** Checking if ESR is running or not ******/

  EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
			AND			bps_xchng_cd = 'NA'
			AND 		bps_pipe_id = :c_pipe_id;

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31320", SQLMSG, c_err_msg);
		return -1;
  }

	if(sql_c_clnt_stts == 'N' ) 
  {
		sprintf(c_err_msg,"ESR client is not running for pipe id :%s:,so pack client can not be started",c_pipe_id);
		return -1;
  }

  /**** Checking if user is already logged on or not ****/

   EXEC SQL
      SELECT opm_login_stts
      INTO   :i_login_stts
      FROM   opm_ord_pipe_mstr
      WHERE  opm_pipe_id = :c_pipe_id;

    if ( SQLCODE != 0 )
    {
			fn_errlog(c_ServiceName, "S31325", SQLMSG, c_err_msg);
			return -1;
    }

    if(i_login_stts == 0)
    {
			fn_userlog(c_ServiceName,"User is not logged in for pipe id :%s:",c_pipe_id);
			sprintf ( c_err_msg, "User is not logged in for pipe id :%s:",c_pipe_id);
			return -1;
    }	

		
		i_ch_val = fn_unpack_fmltovar ( c_ServiceName,
                                    c_err_msg,
                                    ptr_fml_ibuf,	
                                    2,
                                    FFO_SPL_FLG, (char *)&c_run_mode, NULL,
                                    FFO_ORD_TOT_QTY, (char*)&i_no_of_ordrs, NULL);
  	if ( i_ch_val == -1 )
  	{
    	fn_errlog( c_ServiceName, "S31330", FMLMSG, c_err_msg );
    	return -1;
  	}
  
    /**** Ver 1.8 Starts Here *******/
    i_ch_val = Fget32(ptr_fml_ibuf,FFO_OPERATION_TYP,0,(char *)&c_auto_reconnect,0);
    i_ferr[0] = Ferror32;
    if( i_ch_val == -1 )
    {
       if( i_ferr[0] == FNOTPRES )
       {
         c_auto_reconnect = 'N';
       }
       else
       {
         fn_errlog( c_ServiceName, "S31335", FMLMSG, c_err_msg );
         return -1;
       }
    }
    /**** Ver 1.8 Ends Here ********/


		/*** Ver 1.4 starts ***/
		i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
		if ( i_tran_id == -1 )
		{
			fn_errlog(c_ServiceName, "S31340", LIBMSG, c_err_msg );
			return -1;
		}
		
   	fn_userlog( c_ServiceName, "Update FXB status from Q to M" );

		EXEC SQL	
			UPDATE fxb_fo_xchng_book 
			SET		 fxb_plcd_stts ='M' 
			WHERE  fxb_xchng_cd = 'NFO'
/** 	AND    trunc(fxb_rqst_tm) = trunc(sysdate) commented in ver 2.7 as the condition doesn't cater to overnight orders placed on previous date. ***/
      AND    trunc(fxb_mod_trd_dt) = ( select EXG_NXT_TRD_DT from EXG_XCHNG_MSTR where EXG_XCHNG_CD='NFO') /*** added in ver 2.7 ***/ 
			AND    fxb_plcd_stts ='Q'
		  AND    fxb_pipe_id = :c_pipe_id;

		if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND)
    {
      fn_errlog(c_ServiceName, "S31345", SQLMSG, c_err_msg );
		  fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
      return -1;
    }

		i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
		if ( i_ch_val == -1 )
		{
			fn_errlog(c_ServiceName, "S31350", LIBMSG, c_err_msg );
		  fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
			return -1;
		}
		/*** Ver 1.4 Ends ***/

		if ( c_run_mode == 'B' )
  	{
     if( c_auto_reconnect != 'Y' )
     {
    	i_ch_val = Fget32(ptr_fml_ibuf,FFO_TM_STMP,0,(char *)c_tm_stmp,0);

    	if ( i_ch_val == -1 )
    	{
     		 fn_errlog( c_ServiceName, "S31355", FMLMSG, c_err_msg );
      	 return -1;
      }
     }
     else  /** Ver 1.8 Starts here ***/
     {
        ptr_c_time = (char *)fn_get_prcs_spc ( c_ServiceName, "DOWNLOAD_TIME" );
        if ( ptr_c_time == NULL  )
        {
          fn_errlog(c_ServiceName, "S31360", LIBMSG, c_err_msg);
          return -1;
        }

        l_from_time = atol(ptr_c_time);
   
        fn_userlog(c_ServiceName,"Download to be taken from last :%ld: mins",l_from_time);

        EXEC SQL
         SELECT to_char((SYSDATE - (:l_from_time * 60)/86400),'dd-Mon-YYYY-HH24:Mi:ss')
          INTO :c_tm_stmp
          FROM DUAL;
     }    /** Ver 1.8 Ends here ***/

    	sprintf ( c_cmd_ln, "nohup cln_pack_clnt %s NA %s %d %c %s %c &",
              c_pipe_id, c_pipe_id, i_no_of_ordrs, c_run_mode, c_tm_stmp, c_auto_reconnect ); /** Ver 1.8, c_auto_reconnect Added */
    }
  	else
  	{
    	sprintf ( c_cmd_ln, "nohup cln_pack_clnt %s NA %s %d %c &",
              c_pipe_id, c_pipe_id, i_no_of_ordrs, c_run_mode );
  	}

		i_ch_val = system(c_cmd_ln);

  	if (i_ch_val == -1 )
  	{
    	fn_errlog(c_ServiceName, "L31005", LIBMSG, c_err_msg);
    	return -1;
  	}

  	return 0;
}

/*****************************************************************************/
/***	  Raising a trigger to stop the Pack client.		      		           ***/ 
/*****************************************************************************/

int fn_stop_pack_clnt (  char	*c_pipe_id, char *c_ServiceName, char *c_err_msg)
{ 
	char c_filter [256];

  sprintf (c_filter, "cln_pack_clnt_%s",c_pipe_id);

  fn_pst_trg( c_ServiceName,"TRG_STP_BAT","TRG_STP_BAT",c_filter);

	return 0;
}

/*****************************************************************************/
/*** 		 Raising a trigger to stop the ESR client. 												 ***/
/*****************************************************************************/
int fn_stop_esr_clnt (	char *c_pipe_id, char *c_ServiceName, char *c_err_msg)
{
	int i_ch_val;
	long li_pro_id;
	char c_filter [256];
	char c_xchng_cd [3+1];
	char c_prcs_nm[20];

	/**The following logic is commented since OPM has exchange code as NFO 
	EXEC SQL
		SELECT opm_xchng_cd
		INTO   :c_xchng_cd
		FROM   opm_ord_pipe_mstr
		WHERE  opm_pipe_id = :c_pipe_id;
	if ( SQLCODE != 0 )
	{
		fn_errlog(c_ServiceName, "S31365", SQLMSG, c_err_msg );
		return -1;
	}*/

	strcpy(c_xchng_cd,"NA");

	strcpy(c_prcs_nm,"cln_esr_clnt");

	i_ch_val = fn_get_prcs_id(c_prcs_nm,
														 c_xchng_cd,
														 c_pipe_id,
														 &li_pro_id,
														 c_ServiceName,
														 c_err_msg);
	if( i_ch_val == 0 )
	{
	  fn_userlog(c_ServiceName,"Process Id to be stopped:%d",li_pro_id);
	}
	else
	{
		fn_userlog(c_ServiceName,"Process ID not found for cln_esr_clnt for pipe id:%s:Exchange code:%s:",c_pipe_id,c_xchng_cd);
		sprintf(c_err_msg,"Process ID not found for cln_esr_clnt for pipe id:%s:Exchange code:%s:",c_pipe_id,c_xchng_cd);
		return -1;
	}

	if ( li_pro_id != 0 )
	{
		i_ch_val = fn_stop_prcs(c_prcs_nm,li_pro_id,c_ServiceName,c_err_msg);
		if(i_ch_val == 0)
		{
			fn_userlog(c_ServiceName,"ESR CLIENT STOPPED");
		}
		else
		{
			fn_userlog(c_ServiceName,"ESR CLIENT COULD NOT BE STOPPED");
			sprintf(c_err_msg,"ESR CLIENT COULD NOT BE STOPPED");
			return -1;
		}
	}

	return 0;
} 
							
/*****************************************************************************/
/*** Checking and applying Market status 	 	   															 */
/*****************************************************************************/
int fn_mkt_stts_chng( char *c_trg_name, int i_mkt_type, int i_mkt_stts, char *c_pipe_id, char *c_ServiceName, char *c_err_msg)
{

  int i_ch_val;

	EXEC SQL BEGIN DECLARE SECTION;
    struct vw_xchngstts st_stts;      
    char sql_c_ip_pipe_id[2+1];        
  EXEC SQL END DECLARE SECTION;

	fn_pst_trg( c_ServiceName,
              c_trg_name,
              c_trg_name,
              "NFO");

	strcpy ( sql_c_ip_pipe_id, c_pipe_id);

	EXEC SQL	
		SELECT opm_xchng_cd
		INTO   :st_stts.c_xchng_cd
		FROM   opm_ord_pipe_mstr
		WHERE  opm_pipe_id = :c_pipe_id;

		if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "S31370", SQLMSG, c_err_msg );
      return -1;
    }

	  if( i_mkt_type == ORDER_MKT )
		{	
			if( i_mkt_stts == OPEN )
			{
					st_stts.c_crrnt_stts = EXCHANGE_OPEN ;
			}
			else
			{
					st_stts.c_crrnt_stts = EXCHANGE_UNDER_EXPIRY ;
			}

			st_stts.c_rqst_typ = UPD_NORMAL_MKT_STTS ;	
		}
		else if( i_mkt_type == EXERCISE_MKT )
		{
			if( i_mkt_stts == OPEN )
			{
			    st_stts.c_exrc_mkt_stts = EXCHANGE_OPEN ; 	
			}
			else
			{
					st_stts.c_exrc_mkt_stts = EXCHANGE_CLOSE;	
			}
			st_stts.c_rqst_typ = UPD_EXER_MKT_STTS;
		}
		else if ( i_mkt_type == EXTND_MKT )
		{
			
			if( i_mkt_stts == OPEN )
			{
				st_stts.c_crrnt_stts = EXCHANGE_OPEN ;
			}
			else
			{
				st_stts.c_crrnt_stts = EXCHANGE_UNDER_EXPIRY ;
			}
  	  st_stts.c_rqst_typ = UPD_EXTND_MKT_STTS;	
		}

	  fn_cpy_ddr(st_stts.c_rout_crt);

		i_ch_val = fn_acall_svc ( "sfo_con_clnt",
                              c_err_msg,
                              &st_stts,
                              "vw_xchngstts",
                              sizeof (st_stts ),
                              TPNOREPLY,
                              "SFO_UPD_XSTTS" );	

	  if ( i_ch_val == SYSTEM_ERROR )
    {
      fn_errlog ( c_ServiceName, "S31375", LIBMSG, c_err_msg );
      return -1;
    }

	 return 0;
} 


/** 1.1 Starts **/
int fn_req_brd_chnl( char *c_pipe_id,  char *c_ServiceName, char *c_err_msg, char c_flag)   /** Added in Ver 2.1 **/

{

		msg_frs_hdr   st_op_hdr;
		msg_frs_hdr   st_ip_hdr;
		msg_prcs_req	st_prcs_req;
		msg_prcs_res	st_prcs_res;

		int 			i_ch_val;
		int 			i_sck_id;
		char 			*ptr_c_port;
		char 			*ptr_c_add;
		char 			c_ors_addrss[16];
		long int	li_ors_port_no;
	
		/* Read IP Address from ini file*/
   	ptr_c_add = (char *) fn_get_prcs_spc( c_ServiceName, "ORS_SRVR_ADDR");
   	if( ptr_c_add == NULL )
   	{
      fn_errlog(c_ServiceName,"S31380", LIBMSG, c_err_msg);
      return -1;
   	}
   	strcpy ( c_ors_addrss, ptr_c_add );
		fn_userlog(c_ServiceName," ORS_SRVR_ADDR is :%s:",c_ors_addrss);	

		/**** Read the ors server port ****/	
	  ptr_c_port = (char *)fn_get_prcs_spc ( c_ServiceName, "ORS_SRVR_PORT" );
    if ( ptr_c_port == NULL )
    {
    	fn_errlog(c_ServiceName, "S31385", LIBMSG, c_err_msg);
    	return -1;
  	}
  	li_ors_port_no = atol(ptr_c_port);
		fn_userlog(c_ServiceName," ORS_SRVR_PORT is :%ld:",li_ors_port_no);	

		/**** Create a server socket ****/
  	i_ch_val = fn_crt_clnt_sck( c_ors_addrss,
															 li_ors_port_no,	
                               &i_sck_id,
                               c_ServiceName,
                               c_err_msg);


		if ( i_ch_val == -1 )
  	{
    	fn_errlog(c_ServiceName, "S31390", LIBMSG, c_err_msg);
			return -1;
  	}

		fn_userlog(c_ServiceName," ********** Sending Request to start  ORS SERVER *************");

		st_ip_hdr.li_msg_typ = ORS_SRVR_REQ;
    st_ip_hdr.li_msg_len = sizeof(st_prcs_req);
		st_prcs_req.li_prcs_no = START ;

  	st_op_hdr.li_msg_len = sizeof(st_prcs_res);
		st_op_hdr.li_msg_typ = ORS_SRVR_RES;

    i_ch_val = fn_snd_rcv_msg_sck ( i_sck_id,
                                 		&st_prcs_req, 
                                  	st_ip_hdr,
                                  	&st_prcs_res,
                                  	st_op_hdr,
                                  	c_ServiceName,
                                  	c_err_msg );		

		
		if (i_ch_val == -1)
  	{

    	i_ch_val = fn_close_sck(i_sck_id, c_ServiceName, c_err_msg);
    	if (i_ch_val == -1)
    	{
      	fn_errlog(c_ServiceName, "S31395", LIBMSG, c_err_msg);
    	}
    	return -1;
  	} 	


		if(st_prcs_res.li_stts == 0  &&  st_op_hdr.li_msg_typ == ORS_SRVR_RES )	
		{

			 /* i_ch_val = fn_strt_bcast_clnt (c_pipe_id,c_ServiceName,c_err_msg); ** Commented in 2.1 **/


       i_ch_val = fn_strt_bcast_clnt (c_pipe_id,c_ServiceName,c_err_msg,c_flag); /** Added in ver 2.1 **/

				if ( i_ch_val == -1 )
      	{
        	fn_userlog(c_ServiceName,"%s",c_err_msg);
        	fn_errlog(c_ServiceName, "S31400", LIBMSG, c_err_msg );

					i_ch_val = fn_close_sck(i_sck_id, c_ServiceName, c_err_msg);
      		if (i_ch_val == -1)
      		{
        			fn_errlog(c_ServiceName, "S31405", LIBMSG, c_err_msg);
      		}	
					return -1;
      	}

		}
  	else if (st_prcs_res.li_stts == -1)
  	{
			
			strcpy(c_err_msg,st_prcs_res.c_msg);
    	fn_errlog(c_ServiceName, "S31410", LIBMSG, c_err_msg);

			i_ch_val = fn_close_sck(i_sck_id, c_ServiceName, c_err_msg);
      if (i_ch_val == -1)
      {
            fn_errlog(c_ServiceName, "S31415", LIBMSG, c_err_msg);
      }	
    	return -1;
  	}

		i_ch_val = fn_close_sck(i_sck_id, c_ServiceName, c_err_msg);
    if (i_ch_val == -1)
    {
            fn_errlog(c_ServiceName, "S31420", LIBMSG, c_err_msg);
            return -1;
    } 		

  return 0;
  
}

int fn_strt_bcast_clnt ( char *c_pipe_id, char *c_ServiceName, char *c_err_msg, char c_flag)   /* Added in Ver 2.1 */
{
  char c_cmd_ln[256];
  int  i_ch_val;
  int  i_flag = 0;								 /**** Ver 2.1 ****/
  char c_command[100];           /** Ver 2.1 **/
  char c_path[100] = "\0";     /** Ver 2.1 **/
  char  c_file_nm[100];		  /**Added**/
  FILE *fptr;				  /**Added**/
  char c_msg[100]; /** Ver 2.1 **/

	FBFR32 *ptr_fml_ibuf;
  FBFR32 *ptr_fml_obuf;

	/******* Ver 2.1  starts *******/

  if(c_flag == 'N')
  {
    strcpy(c_path, tuxgetenv("HOME"));
    strcat(c_path, "/sh_scripts");

    MEMSET(c_command);
   	sprintf(c_command,"cln_shm_tokn BRD NA 11 ");
  	system(c_command);

 	 	EXEC SQL
  	SELECT 1
  	INTO   :i_flag
  	FROM   FSP_FO_SYSTM_PRMTR
  	WHERE  trunc( FSP_BRD_RUN_DATE ) = trunc( sysdate );

 	 	fn_userlog(c_ServiceName , " the flag is :%d:", i_flag);

  	if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
  	{
    	fn_errlog( c_ServiceName, "S31425", SQLMSG, c_err_msg  );
    	Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
    	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
  	}

  	if(i_flag != 1)
  	{
      MEMSET(c_msg);
      strcpy(c_msg,"cln_shm_tokn process is filed, pls check");
     	sprintf(c_command,"ksh %s/fn_conclnt_mail.sh '%s'",c_path,c_msg);
     	system(c_command);

     	fn_errlog(c_ServiceName, "S31430", LIBMSG, c_err_msg);
     	return -1;
  	}

  	MEMSET(c_command);
  	sprintf(c_command,"tmshutdown -s sfr_brdlzo_rout; tmboot -s sfr_brdlzo_rout;");
  	system(c_command);

	  i_flag = 0;			 /***Added***/
	  MEMSET(c_file_nm);
		strcpy(c_file_nm, getenv("STATUS_FILE"));
		remove(c_file_nm);
	
		sprintf(c_command,"ps -efa | grep -v grep | grep -i sfr_brdlzo_rout | wc -l > %s",c_file_nm);

		fn_userlog(c_ServiceName, " c_file_nm name is :%s:",c_file_nm);
    fn_userlog(c_ServiceName, "c_command is :%s:", c_command);

		system(c_command);

		fptr = (FILE *)fopen(c_file_nm, "r");
		if(fptr == NULL)
		{
			fn_errlog(c_ServiceName,"S31435", LIBMSG, c_err_msg);
			return -1;
		}
		
		fscanf(fptr,"%d",&i_flag);
	 
		fn_userlog(c_ServiceName,"sfr_brdlzo_rout : %d",i_flag);

		fclose(fptr);
	
		if(i_flag == 0)
		{
			MEMSET(c_command);
    	MEMSET(c_msg);
    	strcpy(c_msg,"sfr_brdlzo_rout server did not start, pls check"); 
			sprintf(c_command,"ksh %s/fn_conclnt_mail.sh '%s'",c_path,c_msg);
			system(c_command);
			return -1;
		}

    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_brd_enq BRD NA 11 &");
    system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_brd_enq 11 started ");

    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_brdqt_prcs BRD NA 12 FUTIDX_QUEUE &");
    i_ch_val = system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_brdqt_prcs 12");

    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_brdqt_prcs BRD NA 11 FUTSTK_QUEUE &");
    i_ch_val = system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_brdqt_prcs 11 ");

    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_brdqt_prcs BRD NA 13 OPTIDX_QUEUE &");
    i_ch_val = system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_brdqt_prcs 13 ");

    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_brdqt_prcs BRD NA 14 OPTSTK_QUEUE &");
    system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_brdqt_prcs 14 ");
    /******** Added in Ver 2.5 *******************/
    
    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_brdqt_prcs NFO NA 15 OPTSTK_QUEUE &");
    system(c_cmd_ln);   
    fn_userlog(c_ServiceName,"the cln_brdqt_prcs 15 "); 

    /*********** Ver 2.5 Ends ********************/

    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_othrbrd_prcs BRD NA 11 &");
    system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_othrbrd_prcs 11 ");
    /*********** Added in Ver 2.5 ********************/
    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_othrbrd_prcs NFO NA 13 &");
    system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_othrbrd_prcs 13 ");
    /*********** Ver 2.5 Ends ************************/
    
    /**** Ver 3.0 Starts here ****/
     MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_othrbrd_prcs 12 NA 12 &");
    system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_othrbrd_prcs 12 ");


    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_othrbrd_prcs 14 NA 14 &");
    system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_othrbrd_prcs 14 ");

    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_othrbrd_prcs 15 NA 15 &");
    system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_othrbrd_prcs 15 ");

    /**** Ver 3.0 Ends Here ****/

    MEMSET(c_cmd_ln);
    strcpy(c_cmd_ln,"nohup cln_brd_deq BRD NA 11 &");
    system(c_cmd_ln);

    fn_userlog(c_ServiceName,"the cln_brd_deq 11 ");

   	sleep(3);

		i_flag = 0;		     /***Added***/
		MEMSET(c_file_nm);
		strcpy(c_file_nm, getenv("STATUS_FILE"));
		remove(c_file_nm);

		sprintf(c_cmd_ln,"status | grep -i brd | wc -l > %s",c_file_nm);
/***		sprintf(c_cmd_ln,"ps -efa | grep -v grep | grep -i cln_ | grep BRD | wc -l > %s",c_file_nm);*** commented in ver 2.5 ****/
        sprintf(c_cmd_ln,"ps -efa | grep -v grep | grep -i cln_ | grep -i BRD | wc -l > %s",c_file_nm); /*** Added in ver 2.5 ****/
	
		fn_userlog(c_ServiceName, " c_file_nm name is :%s:",c_file_nm);
  	fn_userlog(c_ServiceName, "c_command is :%s:",c_cmd_ln);

		system(c_cmd_ln);

		fptr = (FILE *)fopen(c_file_nm,"r");
		if(fptr == NULL)
		{
			fn_errlog(c_ServiceName,"S31440", LIBMSG, c_err_msg);
			return -1;
		}

		fscanf(fptr,"%d",&i_flag);
	
		fn_userlog(c_ServiceName,"cln_brd : %d",i_flag);

		fclose(fptr);

	/*	if(i_flag != 7)  ** commented in Ver 2.5 **********/
    if(i_flag != 12)  /**** condition is changed from 7 to 9 in Ver 2.5 ***/ /** Ver 3.0 changed from 9 to 12 **/
		{
			MEMSET(c_command);
    	MEMSET(c_msg);
    	strcpy(c_msg,"Starting of broadcast clients failed, pls check");
    	sprintf(c_command,"ksh %s/fn_conclnt_mail.sh '%s'",c_path,c_msg);
			system(c_command);
			return -1;
		}

  }
  else
  {
    /************************* Ver 2.1 ends *************************/

	fn_userlog(c_ServiceName,"******** Calling cln_brd_clnt ********");
  sprintf(c_cmd_ln, "nohup cln_brd_clnt %s NA %s &", c_pipe_id, c_pipe_id);
  i_ch_val = system(c_cmd_ln);
  if (i_ch_val == -1 )
  {
    fn_errlog( c_ServiceName, "S31445", UNXMSG, c_err_msg );
    return -1;
  }

 }   /**** Ver 2.1 ****/
 return 0;
}

int fn_stop_brd_clnt( char *c_pipe_id, char *c_ServiceName, char *c_err_msg, char c_flag)             /** Ver 2.1 **/
{

	  msg_frs_hdr   st_op_hdr;
    msg_frs_hdr   st_ip_hdr;
    msg_prcs_req  st_prcs_req;
    msg_prcs_res  st_prcs_res;
		char*					ptr_c_add;
		char					c_ors_addrss[16];
	  char*         ptr_c_port;
  	char 					c_xchng_cd [3+1];
  	char 					c_prcs_nm[20];
		long int      li_ors_port_no;
		long 					li_pro_id;
		int 					i_ch_val;
		int 					i_sck_id;


		/* Read IP Address from ini file*/
    ptr_c_add = (char *) fn_get_prcs_spc( c_ServiceName, "ORS_SRVR_ADDR");
    if( ptr_c_add == NULL )
    {
      fn_errlog(c_ServiceName,"S31450", LIBMSG, c_err_msg);
      return -1;
    }
    strcpy ( c_ors_addrss, ptr_c_add );


    /**** Read the ors server port ****/
    ptr_c_port = (char *)fn_get_prcs_spc ( c_ServiceName, "ORS_SRVR_PORT" );
    if ( ptr_c_port == NULL )
    {
      fn_errlog(c_ServiceName, "S31455", LIBMSG, c_err_msg);
      return -1;
    }
    li_ors_port_no = atol(ptr_c_port);

		 /**** Create a server socket ****/
    i_ch_val = fn_crt_clnt_sck( c_ors_addrss,
                               li_ors_port_no,
                               &i_sck_id,
                               c_ServiceName,
                               c_err_msg);


    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S31460", LIBMSG, c_err_msg);
      return -1;
    }

		fn_userlog(c_ServiceName,"*********** Stopping Brodcast client *******"); 

	 	st_ip_hdr.li_msg_typ = ORS_SRVR_REQ;
    st_ip_hdr.li_msg_len = sizeof(st_prcs_req);
    st_prcs_req.li_prcs_no = STOP ;

    st_op_hdr.li_msg_len = sizeof(st_prcs_res);
		st_op_hdr.li_msg_typ = ORS_SRVR_RES;

		i_ch_val = fn_snd_rcv_msg_sck ( i_sck_id,
                                    &st_prcs_req,
                                    st_ip_hdr,
                                    &st_prcs_res,
                                    st_op_hdr,
                                    c_ServiceName,
                                    c_err_msg );

		

	 	if ( i_ch_val == -1 )
    {

				i_ch_val = fn_close_sck(i_sck_id, c_ServiceName, c_err_msg);
      	if (i_ch_val == -1)
      	{
        	fn_errlog(c_ServiceName, "S31465", LIBMSG, c_err_msg);
      	}

        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31470", LIBMSG, c_err_msg );
        return -1;
    }

	fn_userlog(c_ServiceName," c_flag is  :%c:", c_flag);               /*** Ver 2.1 ***/

	if(st_prcs_res.li_stts == 0  &&  st_op_hdr.li_msg_typ == ORS_SRVR_RES )
	{
		strcpy(c_xchng_cd,"NA");

		if(c_flag =='E') 													/**** Ver 2.1 ****/
    {

  		strcpy(c_prcs_nm,"cln_brd_clnt");

  		i_ch_val = fn_get_prcs_id(c_prcs_nm,
                             		c_xchng_cd,
                             		c_pipe_id,
                             		&li_pro_id,
                             		c_ServiceName,
                             		c_err_msg);
  		if( i_ch_val == 0 )
  		{
    		fn_userlog(c_ServiceName,"Process Id to be stopped:%d",li_pro_id);
  		}
			else
  		{
    		fn_userlog(c_ServiceName,"Process ID not found for cln_brd_clnt for pipe id:%s:Exchange code:%s:",c_pipe_id,c_xchng_cd)
;
    		sprintf(c_err_msg,"Process ID not found for cln_brd_clnt for pipe id:%s:Exchange code:%s:",c_pipe_id,c_xchng_cd);
    		return -1;
  		}

			if ( li_pro_id != 0 )
  		{
    			i_ch_val = fn_stop_prcs(c_prcs_nm,li_pro_id,c_ServiceName,c_err_msg);
    			if(i_ch_val == 0)
    			{
      				fn_userlog(c_ServiceName,"BRODCAST CLIENT STOPPED");
    			}
   				 else
    			{
      				fn_userlog(c_ServiceName,"BRODCAST CLIENT COULD NOT BE STOPPED");
      				sprintf(c_err_msg,"BRODCAST CLIENT COULD NOT BE STOPPED");
      				return -1;
    			}
  		}
		 }	
			/*** Ver 2.1 Starts ***/
		 else if(c_flag == 'N')
     {

      fn_userlog(c_ServiceName,"im in new code ");

      MEMSET(c_prcs_nm);
      strcpy(c_prcs_nm,"cln_brd_enq");

      fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );

      strcpy(c_pipe_id,"11");

      if( DEBUG_MSG_LVL_3 )
      {
        fn_userlog(c_ServiceName,"c_xchng_cd is :%s:",c_xchng_cd);
        fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
      }

      i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                               c_xchng_cd,
                               c_pipe_id,
                               c_ServiceName,
                               c_err_msg);

      if ( i_ch_val == -1 )
      {

        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31475", LIBMSG, c_err_msg );
      }

      fn_userlog(c_ServiceName," 1st clnt stopped");

      strcpy(c_prcs_nm,"cln_brdqt_prcs");

      fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );

      strcpy(c_pipe_id,"12");

      if( DEBUG_MSG_LVL_3 )
      {
 	  	  fn_userlog(c_ServiceName,"c_xchng_cd is :%s:",c_xchng_cd);
      	fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
      }

      i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
      					                     c_xchng_cd,
              			                 c_pipe_id,
                  			            c_err_msg);
        if ( i_ch_val == -1 )
        {
           fn_userlog(c_ServiceName,"%s",c_err_msg);
           fn_errlog(c_ServiceName, "S31480", LIBMSG, c_err_msg );
        }

        fn_userlog(c_ServiceName," 2nd  clnt stopped");

        strcpy(c_prcs_nm,"cln_brdqt_prcs");
        fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );
			
				
        strcpy(c_pipe_id,"11");

        if( DEBUG_MSG_LVL_3 )
        {
          fn_userlog(c_ServiceName,"c_xchng_cd is :%s:",c_xchng_cd);
          fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
        }

        i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                               c_xchng_cd,
                               c_pipe_id,
                               c_ServiceName,
                               c_err_msg);
        if ( i_ch_val == -1 )
        {
           fn_userlog(c_ServiceName,"%s",c_err_msg);
           fn_errlog(c_ServiceName, "S31485", LIBMSG, c_err_msg );
        }
        fn_userlog(c_ServiceName," 3rd  clnt stopped");

        strcpy(c_prcs_nm,"cln_brdqt_prcs");

        strcpy(c_pipe_id,"13");

        if( DEBUG_MSG_LVL_3 )
        {
          fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );
        	fn_userlog(c_ServiceName,"c_xchng_cd is :%s:",c_xchng_cd);
        	fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
        }

        i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                               c_xchng_cd,
                               c_pipe_id,
                               c_ServiceName,
                               c_err_msg);

        if ( i_ch_val == -1 )
        {
           fn_userlog(c_ServiceName,"%s",c_err_msg);
           fn_errlog(c_ServiceName, "S31490", LIBMSG, c_err_msg );
        }

        strcpy(c_prcs_nm,"cln_brdqt_prcs");

        strcpy(c_pipe_id,"14");
       
        if( DEBUG_MSG_LVL_3 )
        {
        	fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );
        	fn_userlog(c_ServiceName,"c_xchng_cd is :%s:",c_xchng_cd);
        	fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
        }

        i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                               c_xchng_cd,
                               c_pipe_id,
                               c_ServiceName,
                               c_err_msg);

				        if ( i_ch_val == -1 )
        {
           fn_userlog(c_ServiceName,"%s",c_err_msg);
           fn_errlog(c_ServiceName, "S31495", LIBMSG, c_err_msg );
        }
        /*** Added in ver 2.5 ***/
        
        strcpy(c_prcs_nm,"cln_brdqt_prcs");

        strcpy(c_pipe_id,"15");

        if( DEBUG_MSG_LVL_3 )
        {
          fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );
          fn_userlog(c_ServiceName,"c_xchng_cd is :%s:",c_xchng_cd);
          fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
        }

        i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                               c_xchng_cd,
                               c_pipe_id,
                               c_ServiceName,
                               c_err_msg);

                if ( i_ch_val == -1 )
        {
           fn_userlog(c_ServiceName,"%s",c_err_msg);
           fn_errlog(c_ServiceName, "S31500", LIBMSG, c_err_msg );
        }

        /*** Ver 2.5 Ends ***/

        strcpy(c_prcs_nm,"cln_othrbrd_prcs");

        strcpy(c_pipe_id,"11");
        
        if( DEBUG_MSG_LVL_3 )
        {
        	fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );
        	fn_userlog(c_ServiceName,"c_xchng_cd is :%s:",c_xchng_cd);
        	fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
        }

        i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                               c_xchng_cd,
                               c_pipe_id,
                               c_ServiceName,
                               c_err_msg);

        if ( i_ch_val == -1 )
        {
           fn_userlog(c_ServiceName,"%s",c_err_msg);
           fn_errlog(c_ServiceName, "S31505", LIBMSG, c_err_msg );
        }
        /******************  Added in Ver 2.5 *******************/

        strcpy(c_pipe_id,"13");
 
           if( DEBUG_MSG_LVL_3 )
           {
             fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );
             fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
           }
 
           i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                                  c_xchng_cd,
                                  c_pipe_id,
                                  c_ServiceName,
                                  c_err_msg);
 
           if ( i_ch_val == -1 )
           {
              fn_userlog(c_ServiceName,"%s",c_err_msg);
              fn_errlog(c_ServiceName, "S31510", LIBMSG, c_err_msg );
           }
         /******************* Ver 2.5 Ends ************************/

        /*** Ver 3.0 Starts Here ****/ 
        strcpy(c_pipe_id,"12");

           if( DEBUG_MSG_LVL_3 )
           {
             fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );
             fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
           }

           i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                                  c_xchng_cd,
                                  c_pipe_id,
                                  c_ServiceName,
                                  c_err_msg);

           if ( i_ch_val == -1 )
           {
              fn_userlog(c_ServiceName,"%s",c_err_msg);
              fn_errlog(c_ServiceName, "S31515", LIBMSG, c_err_msg );
           }

           strcpy(c_pipe_id,"14");

           if( DEBUG_MSG_LVL_3 )
           {
             fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );
             fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
           }

           i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                                  c_xchng_cd,
                                  c_pipe_id,
                                  c_ServiceName,
                                  c_err_msg);

           if ( i_ch_val == -1 )
           {
              fn_userlog(c_ServiceName,"%s",c_err_msg);
              fn_errlog(c_ServiceName, "S31520", LIBMSG, c_err_msg );
           }
           
           strcpy(c_pipe_id,"15");

           if( DEBUG_MSG_LVL_3 )
           {
             fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );
             fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
           }

           i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                                  c_xchng_cd,
                                  c_pipe_id,
                                  c_ServiceName,
                                  c_err_msg);

           if ( i_ch_val == -1 )
           {
              fn_userlog(c_ServiceName,"%s",c_err_msg);
              fn_errlog(c_ServiceName, "S31525", LIBMSG, c_err_msg );
           }
        /*** Ver 3.0 Ends Here ***/
        strcpy(c_prcs_nm,"cln_brd_deq");

        strcpy(c_pipe_id,"11");

        if( DEBUG_MSG_LVL_3 )
        { 
        	fn_userlog(c_ServiceName,"c_prcs_nm is :%s:",c_prcs_nm );
        	fn_userlog(c_ServiceName,"c_xchng_cd is :%s:",c_xchng_cd);
        	fn_userlog(c_ServiceName,"c_pipe_id is :%s:", c_pipe_id );
        }

        i_ch_val = fn_stop_new_brd_clnt(c_prcs_nm,
                               c_xchng_cd,
                               c_pipe_id,
                               c_ServiceName,
                               c_err_msg);

        if ( i_ch_val == -1 )
        {
           fn_userlog(c_ServiceName,"%s",c_err_msg);
           fn_errlog(c_ServiceName, "S31530", LIBMSG, c_err_msg );
        }

      }
			      i_ch_val = fn_close_sck(i_sck_id, c_ServiceName, c_err_msg);
      if (i_ch_val == -1)
      {
        fn_errlog(c_ServiceName, "S31535", LIBMSG, c_err_msg);
        return -1;
      }
	
			/**** Ver 2.1 Ends ****/
	}
	else if (st_prcs_res.li_stts == -1)
	{
			fn_userlog(c_ServiceName," Error in stopping brodcast ");
			fn_errlog(c_ServiceName, "S31540", st_prcs_res.c_msg, c_err_msg);

			i_ch_val = fn_close_sck(i_sck_id, c_ServiceName, c_err_msg);
  		if (i_ch_val == -1)
   		{
       		fn_errlog(c_ServiceName, "S31545", LIBMSG, c_err_msg);
   		}	
		return -1;
	}
 
  i_ch_val = fn_close_sck(i_sck_id, c_ServiceName, c_err_msg);
  if (i_ch_val == -1)
   {
       fn_errlog(c_ServiceName, "S31550", LIBMSG, c_err_msg);
       return -1;
   } 

 return 0;
}


int fn_stop_new_brd_clnt(char *c_prcs_nm,	char *c_xchng_cd, char *c_pipe_id, char *c_ServiceName, char *c_err_msg)
{


      int i_ch_val = 0;
      long li_pro_id=0;
      
      i_ch_val = fn_get_new_prcs_id(c_prcs_nm,
                                  c_xchng_cd,
                                  c_pipe_id,
                                  &li_pro_id,
                                  c_ServiceName,
                                  c_err_msg);


        fn_userlog(c_ServiceName," li_pro_id is :%ld",li_pro_id);

        if( i_ch_val == 0 )
        {
          fn_userlog(c_ServiceName,"Process Id to be stopped:%d",li_pro_id);
        }
        else
        {
          fn_userlog(c_ServiceName,"Process ID not found for cln_brd_clnt for pipe id:%s:Exchange code:%s:",c_pipe_id,c_xchng_cd);
          sprintf(c_err_msg,"Process ID not found for cln_brd_clnt for pipe id:%s:Exchange code:%s:",c_pipe_id,c_xchng_cd);
          return -1;
        }

        if ( li_pro_id != 0 )
        {
          i_ch_val = fn_stop_prcs(c_prcs_nm,li_pro_id,c_ServiceName,c_err_msg);
          if(i_ch_val == 0)
          {
              fn_userlog(c_ServiceName,"BRODCAST CLIENT STOPPED");
          }
           else
          {
              fn_userlog(c_ServiceName,"BRODCAST CLIENT COULD NOT BE STOPPED");
              sprintf(c_err_msg,"BRODCAST CLIENT COULD NOT BE STOPPED");
              return -1;
          }
        }

}
	

int fn_all_status( char *c_pipe_id, char	*c_all_status, FBFR32 *ptr_fml_ibuf, char *c_ServiceName, char *c_err_msg)
{

    msg_frs_hdr   st_op_hdr;
    msg_frs_hdr   st_ip_hdr;
    msg_prcs_req  st_prcs_req;
    msg_prcs_res  st_prcs_res;
    int       i_ch_val;
		int 			i_sck_id;
		char			*ptr_c_port;
		char			*ptr_c_add;
		char      c_ors_addrss[16];
		char      *c_status;					
		char  		sql_c_clnt_nm[40];
		char  		sql_c_esr_clnt_stts;
		char  		sql_c_pack_clnt_stts;
		char  		sql_c_brd_clnt_stts;
		char  		sql_c_login_stts;
		char  		sql_c_fwd_stts;
		char      sql_c_mrkt_opn_stts;	/*** Ver 1.5 ***/
		long int  li_ors_port_no;
		int       i_mcount = 0;

		/* Read IP Address from ini file*/
    ptr_c_add = (char *) fn_get_prcs_spc( c_ServiceName, "ORS_SRVR_ADDR");
	
    if( ptr_c_add == NULL )
    {
      fn_errlog(c_ServiceName,"S31555", LIBMSG, c_err_msg);
      return -1;
    }
    strcpy ( c_ors_addrss, ptr_c_add );
    fn_userlog(c_ServiceName," ORS_SRVR_ADDR is :%s:",c_ors_addrss);

    /**** Read the ors server port ****/
    ptr_c_port = (char *)fn_get_prcs_spc ( c_ServiceName, "ORS_SRVR_PORT" );
    if ( ptr_c_port == NULL )
    {
      fn_errlog(c_ServiceName, "S31560", LIBMSG, c_err_msg);
      return -1;
    }
    li_ors_port_no = atol(ptr_c_port);
    fn_userlog(c_ServiceName," ORS_SRVR_PORT is :%ld:",li_ors_port_no);

    i_ch_val = fn_crt_clnt_sck( c_ors_addrss,
                               li_ors_port_no,
                               &i_sck_id,
                               c_ServiceName,
                               c_err_msg);


    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S31565", LIBMSG, c_err_msg);
      return -1;
    }

		fn_userlog(c_ServiceName,"************* Checking Status of Brodcast Client ********");

    st_ip_hdr.li_msg_typ = ORS_SRVR_REQ;
    st_ip_hdr.li_msg_len = sizeof(st_prcs_req);
    st_prcs_req.li_prcs_no = STATUS ;

    st_op_hdr.li_msg_len = sizeof(st_prcs_res);
	  st_op_hdr.li_msg_typ = ORS_SRVR_RES;

    i_ch_val = fn_snd_rcv_msg_sck ( i_sck_id,
                                    &st_prcs_req,
                                    st_ip_hdr,
                                    &st_prcs_res,
                                    st_op_hdr,
                                    c_ServiceName,
                                    c_err_msg );

   if ( i_ch_val == -1 )
    {

				i_ch_val = fn_close_sck(i_sck_id, c_ServiceName, c_err_msg);
				if (i_ch_val == -1)
				{
 					 fn_errlog(c_ServiceName, "S31570", LIBMSG, c_err_msg);
				}

				fn_userlog(c_ServiceName,"%s",c_err_msg);
				fn_errlog(c_ServiceName, "S31575", LIBMSG, c_err_msg );
				return -1;

    }

	fn_userlog(c_ServiceName,"Allocating memory for output buffer" );

  strcpy(sql_c_clnt_nm,"cln_esr_clnt");

    EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_esr_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
      AND     bps_xchng_cd = 'NA'
      AND     bps_pipe_id = :c_pipe_id;

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31580", SQLMSG, c_err_msg);
    return -1;
  }

	 fn_userlog(c_ServiceName,"************* Checking Status of ESR Client ********");
	 fn_userlog(c_ServiceName,"sql_c_clnt_stts - ESR :%c:",sql_c_esr_clnt_stts);

   strcpy(sql_c_clnt_nm,"cln_pack_clnt");

    EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_pack_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
      AND     bps_xchng_cd = 'NA'
      AND     bps_pipe_id = :c_pipe_id;

	 if(SQLCODE != 0)
   {
    	fn_errlog(c_ServiceName, "S31585", SQLMSG, c_err_msg);
    	return -1;
   }


	 fn_userlog(c_ServiceName,"************* Checking Status of PACK Client ********");
	 fn_userlog(c_ServiceName,"sql_c_clnt_stts - PACK :%c:",sql_c_pack_clnt_stts);

   strcpy(sql_c_clnt_nm,"cln_brd_clnt");

    EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_brd_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
      AND     bps_xchng_cd = 'NA'
      AND     bps_pipe_id = :c_pipe_id;

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31590", SQLMSG, c_err_msg);
    return -1;
  }

	fn_userlog(c_ServiceName,"************* Checking Status of BRD Client ********");
	fn_userlog(c_ServiceName,"sql_c_clnt_stts - BRD :%c:",sql_c_brd_clnt_stts);

	fn_userlog(c_ServiceName,"************* Checking Status of BRD Server-Pocess ********");

	if (st_op_hdr.li_msg_typ == ORS_SRVR_RES)
	{

			if(st_prcs_res.li_stts == 1 )
  		{
					fn_userlog(c_ServiceName," BOTH_UP ");
  		}
  		else if (st_prcs_res.li_stts == 2 )
  		{
      		fn_userlog(c_ServiceName," BRD_SRVR_UP ");
					fn_userlog(c_ServiceName,"******** STATUS :%s:",st_prcs_res.c_msg);
  		}
			else if (st_prcs_res.li_stts == 3 )
			{
			
      		fn_userlog(c_ServiceName," BRD_PRCS_UP ");
					fn_userlog(c_ServiceName,"******** STATUS :%s:",st_prcs_res.c_msg);
  		}
			else if (st_prcs_res.li_stts == 0 )
  		{
      		fn_userlog(c_ServiceName," BOTH_DWN ");
					fn_userlog(c_ServiceName,"******** STATUS :%s:",st_prcs_res.c_msg);
  		}
			else
			{
					fn_userlog(c_ServiceName," Invalid Message ");
      		fn_errlog(c_ServiceName, "S31595", st_prcs_res.c_msg, c_err_msg);
				  return -1;   	
			}
	}


	EXEC SQL
			 SELECT opm_login_stts,
							opm_pmp_stts 
			 INTO		:sql_c_login_stts,
			 				:sql_c_fwd_stts
			 FROM   opm_ord_pipe_mstr 
			 WHERE  opm_pipe_id = :c_pipe_id
			 AND		opm_xchng_cd = 'NFO';

	if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31600", SQLMSG, c_err_msg);
    return -1;
  }

	/*** Ver 1.5 starts ***/
  EXEC SQL
    SELECT exg_crrnt_stts
    INTO   :sql_c_mrkt_opn_stts
    FROM   exg_xchng_mstr
    WHERE  exg_xchng_cd='NFO';

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31605", SQLMSG, c_err_msg);
    return -1;
  }	
	fn_userlog(c_ServiceName,"sql_c_mrkt_opn_stts :%c:",sql_c_mrkt_opn_stts);
	/** Ver 1.5 ends ***/
	
	fn_userlog(c_ServiceName,"sql_c_login_stts:%c:",sql_c_login_stts);
	fn_userlog(c_ServiceName,"sql_c_fwd_stts :%c:",sql_c_fwd_stts);

	/*** Commented in Ver 1.4 ***
  fn_userlog(c_ServiceName,"Final Remark:");
	fn_userlog(c_ServiceName,"Status : ESR CLNT - LOGIN - PACK CLNT - FWD - BRD CLNT - BRD SRVR n PRCS");
	sprintf(c_all_status,"%c-%c-%c-%c-%c-%ld",sql_c_esr_clnt_stts,sql_c_login_stts,sql_c_pack_clnt_stts,sql_c_fwd_stts,sql_c_brd_clnt_stts,st_prcs_res.li_stts); 
	***/

  /*** Ver 1.4 starts here ***/

  EXEC SQL
    SELECT count(0)
    INTO   :i_mcount
		FROM	 fxb_fo_xchng_book
    WHERE  fxb_xchng_cd = 'NFO'
		/*** AND    trunc(fxb_rqst_tm) = trunc(sysdate) commented in ver 2.7 ***/
    AND    trunc(fxb_mod_trd_dt) = ( select EXG_NXT_TRD_DT from EXG_XCHNG_MSTR where EXG_XCHNG_CD='NFO') /*** added in ver 2.7 ***/
    AND    fxb_plcd_stts ='M'
    AND    fxb_pipe_id = :c_pipe_id;

  if(SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
  {
    fn_errlog(c_ServiceName, "S31610", SQLMSG, c_err_msg);
    return -1;
  }

	fn_userlog(c_ServiceName,"Final Remark:");
	fn_userlog(c_ServiceName,"Status : ESR CLNT - LOGIN - PACK CLNT - FWD - BRD CLNT - BRD SRVR n PRCS - 'M' STAT - MRKT_STTS");  /*** MRKT_STTS added in Ver 1.5 ***/
  sprintf(c_all_status,"%c-%c-%c-%c-%c-%ld-%d-%c",sql_c_esr_clnt_stts,sql_c_login_stts,sql_c_pack_clnt_stts,sql_c_fwd_stts,sql_c_brd_clnt_stts,st_prcs_res.li_stts,i_mcount,sql_c_mrkt_opn_stts); /*** Market status added in Ver 1.5 ***/

	/*** Ver 1.4 Ends here ***/
	
	fn_userlog(c_ServiceName,"c_all_status FINAL:%s:",c_all_status);

	i_ch_val = fn_close_sck(i_sck_id, c_ServiceName, c_err_msg);
  if (i_ch_val == -1)
   {
       fn_errlog(c_ServiceName, "S31615", LIBMSG, c_err_msg);
       return -1;
   }

	return 0;
}

/** 1.1 Ends **/

int fn_get_prcs_id(char *c_prcs_nm, char *c_xchng_cd, char *c_pipe_id, long *li_pro_id,char *c_ServiceName, char *c_err_msg)
{

  char  c_command[150];
  char  c_file_nm[100];
  char  c_user[20];
  char  c_process[20];
  char  c_exchange[13+1];
  FILE  *fptr;
  long   li_tmp_prc_id;

  MEMSET(c_file_nm);
  MEMSET(c_command);

  strcpy(c_file_nm, getenv("STATUS_FILE"));

  fn_userlog(c_ServiceName,"IN the process get process id");
  fn_userlog(c_ServiceName,"process Name:%s:",c_prcs_nm);
  fn_userlog(c_ServiceName,"Exchange Code:%s:",c_xchng_cd);
  fn_userlog(c_ServiceName,"Pipe Id:%s:",c_pipe_id);


  remove(c_file_nm);

  sprintf(c_command, "ps -efa -o pid,user,comm,args | grep -v 'grep' |grep $USER | grep %s | grep \"%s %s\" > %s",c_xchng_cd,c_prcs_nm,c_pipe_id,c_file_nm);


  system(c_command);

  fptr = (FILE *)fopen(c_file_nm, "r");
  if(fptr == NULL)
  {
    fn_errlog(c_ServiceName,"S31620", LIBMSG, c_err_msg);
    return -1;
  }

  MEMSET(c_process);

  fscanf(fptr, "%d %s",&li_tmp_prc_id,c_process);
  fn_userlog(c_ServiceName, "Process :%s: is running and process id is :%d",c_process,li_tmp_prc_id);

  *li_pro_id = li_tmp_prc_id;

  fclose(fptr);
  return 0;
}


  /*** Added in Ver 2.1 starts ****/
int fn_get_new_prcs_id(char *c_prcs_nm, char *c_xchng_cd, char *c_pipe_id, long *li_pro_id,char *c_ServiceName, char *c_err_msg)
{

  char  c_command[150];
  char  c_file_nm[100];
  char  c_process[20]="\0";
  char  c_exchange[13+1];
  FILE  *fptr;
  long  li_tmp_prc_id=0;

  MEMSET(c_file_nm);
  MEMSET(c_command);

  strcpy(c_file_nm, getenv("STATUS_FILE"));
  
  if( DEBUG_MSG_LVL_0 )
  {
  	fn_userlog(c_ServiceName,"IN the process get process id");
  	fn_userlog(c_ServiceName,"process Name:%s:",c_prcs_nm);
  	fn_userlog(c_ServiceName,"Exchange Code:%s:",c_xchng_cd);
  	fn_userlog(c_ServiceName,"Pipe Id:%s:",c_pipe_id);
  }

  remove(c_file_nm);
sprintf(c_command, "ps -efa -o pid,user,comm,args | grep -v 'grep' |grep $USER | grep %s | grep %s| grep %s > %s",c_xchng_cd,
 c_prcs_nm, c_pipe_id, c_file_nm);

  if( DEBUG_MSG_LVL_1)
  {
   fn_userlog(c_ServiceName, " c_file_nm name is :%s:",c_file_nm);
   fn_userlog(c_ServiceName, "c_command is :%s:", c_command);
  }

  system(c_command);

  fptr = (FILE *)fopen(c_file_nm, "r");
  if(fptr == NULL)
  {
    fn_errlog(c_ServiceName,"S31625", LIBMSG, c_err_msg);
    return -1;
  }

  MEMSET(c_process);

  fscanf(fptr, "%ld %s",&li_tmp_prc_id,c_process);
  fn_userlog(c_ServiceName, "Process :%s: is running and process id is :%ld:",c_process,li_tmp_prc_id);

  *li_pro_id = li_tmp_prc_id;

  fclose(fptr);
  return 0;
} 
/******************** Ver 2.1 Ends **********************/

int fn_stop_prcs(char *c_name ,long li_pro_id, char* c_ServiceName,char *c_err_msg)
{

  union sigval  st_data;
  st_data.sival_int = SIGUSR2;
  fn_userlog(c_ServiceName,"Inside stop process for %s:",c_name);

  /*Raise a signal*/
    if(sigqueue(li_pro_id,SIGUSR2, st_data) == -1)
    {
      fn_errlog(c_ServiceName, "31010", UNXMSG,c_err_msg);
      return -1;
    }
    fn_userlog(c_ServiceName, "Signal(%d) raised to %d", SIGUSR2, li_pro_id);
  return 0;
}

/*** ------------------------1.2 Starts ---------------------------------***/

int fn_chk_n_strt_frwrdng (char *c_pipe_id, char *c_ServiceName, char *c_err_msg)
{

	 EXEC SQL BEGIN DECLARE SECTION;
      char c_exg_nxt_trd_dt[15];
      char c_exg_crrnt_stts;
      char c_exg_extnd_mrkt_stts;
      char c_exg_exrc_mkt_stts;
    EXEC SQL END DECLARE SECTION;

		EXEC SQL
      SELECT exg_nxt_trd_dt,
             exg_crrnt_stts,
             exg_exrc_mkt_stts,
             exg_extnd_mrkt_stts
      INTO   :c_exg_nxt_trd_dt,
             :c_exg_crrnt_stts,
             :c_exg_exrc_mkt_stts,
             :c_exg_extnd_mrkt_stts
      FROM   exg_xchng_mstr
      WHERE exg_xchng_cd = 'NFO';

		if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "S31630", SQLMSG, c_err_msg );
      return -1;
    }

		fn_userlog (c_ServiceName, "Trade date |%s|", c_exg_nxt_trd_dt );
    fn_userlog (c_ServiceName, "Current status |%c|", c_exg_crrnt_stts );
    fn_userlog (c_ServiceName, "Exercise Market status |%c|", c_exg_exrc_mkt_stts );
    fn_userlog (c_ServiceName, "Extnd Market Status |%c|", c_exg_extnd_mrkt_stts );

		if (( c_exg_crrnt_stts      == 'O' ) || ( c_exg_exrc_mkt_stts   == 'O' ) || ( c_exg_extnd_mrkt_stts == 'O' ))
		{
      fn_userlog(c_ServiceName, "Posting trigger as Market Status is OPEN" );
      fn_pst_trg( c_ServiceName,
                 "TRG_STRT_PMP",
                 "TRG_STRT_PMP",
                 c_pipe_id);
      sprintf(c_err_msg,"Posting trigger as Market Status is OPEN");
    }
    else
    {
      fn_userlog(c_ServiceName, "Not posting trigger as Market Status is NOT OPEN" );
      sprintf(c_err_msg,"Not posting trigger as Market Status is NOT OPEN");
      return (-1);
    }

		return (0);	

}


int fn_connect_all(FBFR32 *ptr_fml_ibuf, char *c_ServiceName, char *c_err_msg)
{
    int i;
    int i_len;
    int i_ch_val;
    int i_tran_id;
		int li_tot_qty;
		int	count=0;
    int i_ferr [ 1 ];
    int i_err [ 1 ];
    int i_run_counter=0;
		int i_msgq_typ = 0;       /** Ver 2.4 **/
    int i_send_queue_new = 0; /** Ver 2.4 **/
 
		char  c_run_mode;
		char	c_order_dtls;
    char 	c_rqst_typ;
    char 	c_dwn_flg;
    char 	c_pipe_id[3];
    char 	c_user_id[6];
    long 	li_user_id;
		char	c_pipe_list1[MAX];
		char	c_pipe_list[PIPE][3];
		char 	*null_ptr = 0;
		char  *ptr_c_flg;
		char  *ptr_c_tmp;       /** Ver 2.4 **/
    char  c_fileName [512]; /** Ver 2.4 **/
    char  c_auto_reconnect='\0';  /** Ver 1.8 **/
    char  c_server_flag_new='\0'; /** Ver 1.8 **/

		 char  c_tm_stmp [ LEN_DT ];

    char c_passwd[LEN_PASS_WD];

		MEMSET(c_pipe_list1);
		MEMSET(c_run_mode);

		struct st_req_q_data st_q_con_clnt;    /** Ver 2.4 **/
		MEMSET(st_q_con_clnt);    						 /** Ver 2.4 **/
	
		fn_userlog(c_ServiceName,"connect all loop");	

    i_ch_val = fn_unpack_fmltovar ( c_ServiceName,
                                    c_err_msg,
                                    ptr_fml_ibuf,
                                    2,																			 /*** Changed from 3 to 2 Ver 1.5 ***/
                                    FFO_QUEUE_NAME, (char *)c_pipe_list1, NULL,
                                    /* FFO_PASS_WD, (char*)c_passwd, NULL,*/ /*** Commented in Ver 1.5 ***/
																		FFO_SPL_FLG,(char *)&c_run_mode,0);
    if ( i_ch_val == -1 )
    {
      fn_errlog( c_ServiceName, "S31635", FMLMSG, c_err_msg );
			return -1;
    }

		fn_userlog(c_ServiceName," --------------------------step 1  --------------------------------------");

		fn_userlog(c_ServiceName,"c_pipe_list1 :%s:",c_pipe_list1);
    fn_userlog(c_ServiceName,"c_passwd :%s:",c_passwd);
    fn_userlog(c_ServiceName,"c_run_mode:%c:",c_run_mode);
    fn_userlog(c_ServiceName,"Auto Reconnect Flag :%c:",c_auto_reconnect);

	  if ( c_run_mode == 'B' )
    {
       i_ch_val = Fget32(ptr_fml_ibuf,FFO_TM_STMP,0,(char *)c_tm_stmp,0);

       if ( i_ch_val == -1 )
       {
            fn_errlog( c_ServiceName, "S31640", FMLMSG, c_err_msg );
            return -1;
       }

    }
  
    fn_userlog(c_ServiceName,"c_tm_stmp:%s:",c_tm_stmp);

		i_len = strlen(c_pipe_list1);
		fn_userlog(c_ServiceName,"i_len = :%d:",i_len);

		for(i = 0;i< i_len; i++ )
		{
				if(c_pipe_list1[i]  == '-')
				{
				count++;					
				}
		}

		fn_userlog(c_ServiceName,"count :%d:",count);

    for(i=0;i<count +1 ;i++)
    {
				fn_userlog(c_ServiceName," -------------------------- step 2 --------------------------------------");

        MEMSET(c_pipe_id);
				MEMSET(c_fileName);   /*** Ver 2.4 ***/

				if(i == 0)
				{
				strcpy(c_pipe_list[i],strtok(c_pipe_list1,"-"));
				}
				else
				{
				strcpy(c_pipe_list[i],strtok(null_ptr, "-" ));

				}

				strcpy(c_pipe_id,c_pipe_list[i]);

        fn_userlog(c_ServiceName,"c_pipe_id :%s:",c_pipe_id);

        EXEC SQL
             SELECT opm_user_id
             INTO 	:c_user_id
             FROM 	opm_ord_pipe_mstr
             WHERE  opm_pipe_id = :c_pipe_id
             AND    opm_xchng_cd = 'NFO';

			 	if ( SQLCODE != 0 )
       	{
            fn_errlog(c_ServiceName, "S31645", SQLMSG, c_err_msg );
						return -1;
        }

        fn_userlog(c_ServiceName,"c_user_id :%s:",c_user_id);

        fn_userlog(c_ServiceName," *********** Starting ESR Client ******************");
        c_auto_reconnect='N';
        c_server_flag_new='P';
 
        i_ch_val = fn_strt_esr_clnt ( c_pipe_id,
                                      c_auto_reconnect,   /** Ver 1.8 **/
                                      c_server_flag_new,  /** Ver 1.8 **/
                                    	c_ServiceName,
                                    	c_err_msg);

        if ( i_ch_val == -1 )
        {
             fn_userlog(c_ServiceName,"%s",c_err_msg);
             fn_errlog(c_ServiceName, "S31650", LIBMSG, c_err_msg );
						 return -1;
        }

				sleep(1); 
        
        fn_userlog(c_ServiceName," *********** Logon to TAP ******************");

				Finit32(ptr_fml_ibuf, Fsizeof32(ptr_fml_ibuf));

        fn_userlog(c_ServiceName," c_passwd:%s:",c_passwd);

				li_user_id = atol(c_user_id);

				fn_userlog(c_ServiceName," li_user_id:%ld:",li_user_id);

				/*** For connect all , open order details is taken as LOGIN_WITH_OPEN_ORDR_DTLS i.e 'G' ***/

				c_order_dtls = 'G';

				fn_userlog(c_ServiceName,"c_order_dtls :%c:",c_order_dtls);

				i_ch_val = fn_pack_vartofml( c_ServiceName,
                                          c_err_msg,
                                          &ptr_fml_ibuf,
                                          3,																			/*** Changed from 4 to 3 Ver 1.5 ***/
                                          FFO_PIPE_ID, ( char * )c_pipe_id,
                                          FFO_EXCHNG_USR_ID,(char *)&li_user_id,
                                          FFO_SPL_FLG, ( char * )&c_order_dtls);	
                                          /* FFO_PASS_WD, ( char * )c_passwd); */  /*** Commented in Ver 1.5 ***/


        if( i_ch_val == -1 )
        {
          fn_errlog( c_ServiceName, "S31655", FMLMSG, c_err_msg );
					return -1;
        }

        i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
        if ( i_tran_id == -1 )
        {
          fn_errlog(c_ServiceName, "S31660", LIBMSG, c_err_msg );
					return -1;
        }

				i_ch_val = fn_connect_all_logon_to_TAP( ptr_fml_ibuf,
                                    						c_ServiceName,
                                    						c_err_msg,
																								&st_q_con_clnt    /** Ver 2.4 **/
																							);

        if ( i_ch_val == -1 )
        {
          strcat(c_err_msg ," **** Login failed ****");
          fn_userlog(c_ServiceName,"%s",c_err_msg);
          fn_errlog(c_ServiceName, "S31665", LIBMSG, c_err_msg );
          fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
					return -1;
        }

        i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
        if ( i_ch_val == -1 )
        {
          fn_errlog(c_ServiceName, "S31670", LIBMSG, c_err_msg );
					return -1;
        }

				/*** Ver 2.4 Starts here ***/

        sprintf(c_fileName, "%s/%s.ini", getenv("BIN"), c_pipe_id);

        i_ch_val = fn_init_prcs_spc( "SFO_CON_CLNT",
                                      c_fileName,
                                      "SFO_CON_CLNT"
                                   );

        if (i_ch_val == -1)
        {
          fn_userlog (c_ServiceName,"Initializing Process space is failed " );
          fn_errlog(c_ServiceName, "S41580", LIBMSG, c_err_msg );
          return -1;
        }

        /***** Taking access of Transmit queque. *****/

        ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"SEND_QUEUE");
        i_msgq_typ = atoi(ptr_c_tmp);

        if(fn_crt_msg_q(&i_send_queue_new, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
        {
          fn_userlog (c_ServiceName,"Failed to take access of Transmit queque");
          fn_errlog(c_ServiceName, "S41580", LIBMSG, c_err_msg );
          return -1;
        }


        if(DEBUG_MSG_LVL_0)
        {
          fn_userlog(c_ServiceName,"i_msgq_typ :%d:",i_msgq_typ);
          fn_userlog(c_ServiceName,"i_send_queue_new :%d:", i_send_queue_new);
        }

        if(fn_write_msg_q(i_send_queue_new,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
          fn_errlog(c_ServiceName, "S31675", LIBMSG, c_err_msg );
          return -1;
        }

        if(DEBUG_MSG_LVL_0)
        {
          fn_userlog(c_ServiceName,"Done fn_write_msg_q-fn_connect_all");
        }


        /*** Ver 2.4 Ends here ***/

 			  sleep(5);

        fn_userlog(c_ServiceName," *********** Starting Pack Client ******************");

			  i_ch_val =  Fchg32(ptr_fml_ibuf,FFO_SPL_FLG,0,(char *)&c_run_mode,0);
			  if( i_ch_val == -1 )
        {
          fn_errlog( c_ServiceName, "S31680", FMLMSG, c_err_msg );
        }

				/** Total Initial Qty is taken as '0' **/

				li_tot_qty = 0; 

				i_ch_val = Fadd32(ptr_fml_ibuf,FFO_ORD_TOT_QTY,(char *)&li_tot_qty,0);
				if( i_ch_val == -1 )
        {
            fn_errlog( c_ServiceName, "S31685", FMLMSG, c_err_msg );
        }

				if ( c_run_mode == 'B' )
    		{
						i_ch_val = Fadd32(ptr_fml_ibuf,FFO_TM_STMP,(char *)c_tm_stmp,0);
						if( i_ch_val == -1 )
        		{
          		 fn_errlog( c_ServiceName, "S31690", FMLMSG, c_err_msg );
        		}
			  }	

				fn_userlog(c_ServiceName," li_tot_qty:%d:",li_tot_qty);
				fn_userlog(c_ServiceName," c_run_mode :%c:",c_run_mode);


        i_ch_val = fn_strt_pack_clnt (c_pipe_id,
                                    	ptr_fml_ibuf,
                                    	c_ServiceName,
                                    	c_err_msg);

        if ( i_ch_val == -1 )
        {
            fn_userlog(c_ServiceName,"%s",c_err_msg);
            fn_errlog(c_ServiceName, "S31695", LIBMSG, c_err_msg );
						return -1;
        }
 
				sleep(1);

		 fn_userlog(c_ServiceName," ===================END OF PIPE :%s:===========================\n\n ",c_pipe_id); 
  }

  return 0;
}


int fn_fwdng_all(FBFR32 *ptr_fml_ibuf, char *c_ServiceName, char *c_err_msg)
{

    int i;
    int i_ch_val;
    int i_len;
		int count = 0 ;
		char *null_ptr = 0;
		char c_pipe_list1[MAX];
		char  c_pipe_list[PIPE][3];
    char c_pipe_id[3];

		i_ch_val = fn_unpack_fmltovar ( c_ServiceName,
                                    c_err_msg,
                                    ptr_fml_ibuf,
                                    1,
                                    FFO_QUEUE_NAME, (char *)c_pipe_list1, NULL);
    if ( i_ch_val == -1 )
    {
      fn_errlog( c_ServiceName, "S31700", FMLMSG, c_err_msg );
      return -1;
    }

		fn_userlog(c_ServiceName,"c_pipe_list1 :%s:",c_pipe_list1);

		i_len = strlen(c_pipe_list1);

    for(i = 0;i< i_len; i++ )
    {
        if(c_pipe_list1[i]  == '-')
        {
        count++;
        }
    }

    fn_userlog(c_ServiceName,"count :%d:",count);

    for(i=0;i<count + 1;i++)
    {

        MEMSET(c_pipe_id);

        if(i == 0)
        {
        strcpy(c_pipe_list[i],strtok(c_pipe_list1,"-"));
        }
        else
        {
        strcpy(c_pipe_list[i],strtok(null_ptr, "-" ));

        }

        strcpy(c_pipe_id,c_pipe_list[i]);

        fn_userlog(c_ServiceName,"c_pipe_id :%s:",c_pipe_id);


        i_ch_val =  fn_chk_n_strt_frwrdng(c_pipe_id,
                                      		c_ServiceName,
                                      		c_err_msg);


        if ( i_ch_val == -1 )
        {
            fn_userlog(c_ServiceName,"%s",c_err_msg);
            fn_errlog(c_ServiceName, "S31705", LIBMSG, c_err_msg );
						return -1;
        }

    }
return 0;
}				 

/*** ------------------------1.2 Ends ---------------------------------***/

/*** Ver 1.4 starts ***/
int fn_prcs_m_stts ( FBFR32 *ptr_fml_ibuf, char *c_pipe_id, char *c_ServiceName, char *c_err_msg)
{
	int 	i_ch_val;
	int		i_tran_id;	
	char  c_action;
	char c_exg_nxt_trd_dt[15];
	char c_cmd_ln[256];

	i_ch_val = Fget32(ptr_fml_ibuf,FFO_REQ_TYP,0,(char *)&c_action,0);

 	fn_userlog( c_ServiceName, "c_action = :%c:",c_action );


	if ( i_ch_val == -1 )
	{
		fn_errlog( c_ServiceName, "S31710", FMLMSG, c_err_msg );
		return -1;
	}

	if (c_action == 'R')		/*** Re-forward Queued orders ***/
	{
   	fn_userlog( c_ServiceName, "Re-forward Queued orders" );

		i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
	
		if ( i_tran_id == -1 )
		{
			fn_errlog(c_ServiceName, "S31715", LIBMSG, c_err_msg );
			return -1;
		}
		
   	fn_userlog( c_ServiceName, "Update FXB status from M to R" );

		EXEC SQL	
			UPDATE fxb_fo_xchng_book 
			SET		 fxb_plcd_stts ='R' 
			WHERE  fxb_xchng_cd = 'NFO'
	/**	AND    trunc(fxb_rqst_tm) = trunc(sysdate) commented in ver 2.7 ***/
      AND    trunc(fxb_mod_trd_dt) = ( select EXG_NXT_TRD_DT from EXG_XCHNG_MSTR where EXG_XCHNG_CD='NFO') /*** added in ver 2.7 ***/ 
			AND    fxb_plcd_stts ='M'
		  AND    fxb_pipe_id = :c_pipe_id;

		if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND)
    {
      fn_errlog(c_ServiceName, "S31720", SQLMSG, c_err_msg );
		  fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
      return -1;
    }

		i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
		if ( i_ch_val == -1 )
		{
			fn_errlog(c_ServiceName, "S31725", LIBMSG, c_err_msg );
		  fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
			return -1;
		}
	}
	else
	if (c_action == 'J')		/*** Reject Queued orders ***/
	{
   	fn_userlog( c_ServiceName, "Reject Queued orders" );

		EXEC SQL
			SELECT exg_nxt_trd_dt
			INTO   :c_exg_nxt_trd_dt
			FROM   exg_xchng_mstr
      WHERE exg_xchng_cd = 'NFO';

		if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "S31730", SQLMSG, c_err_msg );
      return -1;
    }
		sprintf ( c_cmd_ln, "bat_fo_rej NFO NFO %s %s M N",	c_pipe_id, c_exg_nxt_trd_dt );

		i_ch_val = system(c_cmd_ln);
		if (i_ch_val == -1 )
		{
			fn_errlog( c_ServiceName, "S31735", UNXMSG, c_err_msg );
			return -1;
		}
	}
 	return 0;
}
/*** Ver 1.4 Ends ***/

/*** Ver 1.8 Starts Here **/
int fn_check_status ( char *c_prcs_nm, char *c_pipe_id, long *li_pro_id, char *c_serviceName, char *c_errmsg)
{
  char  c_command[150];
  char  c_file_nm[100];
  char  c_process[20];
  FILE  *fptr;
  long  li_tmp_prc_id = 0;
  int i_retval = 0;
  sigset_t newset;
  sigset_t oldset;
  sigset_t *newset_p;
  newset_p = &newset;

  MEMSET(c_file_nm);
  MEMSET(c_command);

  strcpy(c_file_nm, getenv("STATUS_FILE"));

  fn_userlog(c_serviceName,"Checking status of %s.....",c_prcs_nm);


  remove(c_file_nm);

  sprintf(c_command, "ps -efa -o pid,comm,user,args | grep -v 'grep' |grep $USER | grep \"%s %s\" > %s",
          c_prcs_nm,c_pipe_id,c_file_nm);

  system(c_command);

  fptr = (FILE *)fopen(c_file_nm, "r");
  if(fptr == NULL)
  {
    fn_errlog(c_serviceName,"S31740", LIBMSG, c_errmsg);
    return -1;
  }
  MEMSET(c_process);

  fscanf(fptr, "%d %s",&li_tmp_prc_id,c_process);

  if(li_tmp_prc_id == 0) /* process does not exist*/
  {
    fclose(fptr);
    return NOT_RUNNING;
  }
  else /* Process exists */
  {
    fn_userlog(c_serviceName, "Process %s is running with pid :%d",c_process,li_tmp_prc_id);
    *li_pro_id = li_tmp_prc_id;
    fclose(fptr);
    return RUNNING;
  }
}

int fo_upd_opm_reconct_dtls( char *c_ServiceName, char *c_errmsg, char *c_pipe_id, char *c_xchng_cd, char *c_server_flg, int  *i_contn_cntr )
{
  int 	i_counter;
  int   i_tran_id;
  int   i_ch_val;

  char c_reconnection_flg='\0';
 
  fn_userlog(c_ServiceName,"c_pipe_id :%s:",c_pipe_id);
  fn_userlog(c_ServiceName,"c_xchng_cd :%s:",c_xchng_cd);

  EXEC SQL
        SELECT NVL(opm_rcnnct_count, 0),
          CASE
               WHEN (SYSDATE - NVL(opm_rcnnct_tm, SYSDATE)) >= ((1.0*PAR_MAX_DECISION_TIME_SEC)/(24*60*60))  THEN 'S'
                                                            /** Start fresh **/
               WHEN (SYSDATE - NVL(opm_rcnnct_tm, SYSDATE)) < ((1.0*PAR_MAX_DISCONNECT_TIME_SEC)/(24*60*60)) THEN 'R'
                                          /** Check counter stop or restart(SAME/SWITCH server) **/
               WHEN (SYSDATE - NVL(opm_rcnnct_tm, SYSDATE)) >= ((1.0*PAR_MAX_DISCONNECT_TIME_SEC)/(24*60*60)) Then 'C'
                                          /** Fluctuation restart with switch server **/
          END
        INTO  :i_counter,
              :c_reconnection_flg
        FROM  opm_ord_pipe_mstr, PAR_SYSTM_PRMTR
        WHERE opm_xchng_cd = :c_xchng_cd
        AND   opm_pipe_id  = :c_pipe_id;
   if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "S31745", SQLMSG, c_errmsg );
      return -1;
    } 
  fn_userlog(c_ServiceName,"Call from recconection :%d: Flag :%c:", i_counter, c_reconnection_flg);
 
  if(c_reconnection_flg == 'S' || c_reconnection_flg == 'C')
  {
    i_counter = 0;
  }

  if(c_reconnection_flg == 'R')
  {
    if(i_counter < 3)
    {
      strcpy(c_errmsg, "Indicates try again on same setup.");
    }
    else if(i_counter == 3)
    {
      if(*c_server_flg == 'F')
       *c_server_flg = 'P';       /** Re-Connect On main server **/
      else
       *c_server_flg = 'F';       /** Re-Connect on fallback server **/

      strcpy(c_errmsg, "Indicates that its time to try diff TAP as 3 times retry has done.");
    }
    else if(i_counter >= 4)
    {
      *c_server_flg = 'C';         /** Do not reconnect **/
      fn_userlog(c_ServiceName,"%s: Error : Max reconnection attempts exceeded :%s: counter :%d:",c_ServiceName,c_pipe_id, i_counter);
      strcpy(c_errmsg, "Max reconnection attempts exceeded.");
    }
  }
  else if(c_reconnection_flg == 'C')
  {
   if(*c_server_flg == 'F')
    *c_server_flg = 'P';       /** Re-Connect On main server **/
   else
    *c_server_flg = 'F';       /** Re-Connect on fallback server **/

   strcpy(c_errmsg, "Line fluctuation : its time to try diff TAP as 3 times retry has done.");
  }

  if(i_counter < 4)
  {
    i_tran_id = fn_begintran ( c_ServiceName, c_errmsg );
    if ( i_tran_id == -1 )
    {
     fn_errlog(c_ServiceName, "S31750", LIBMSG, c_errmsg );
     return -1;
    }


    EXEC SQL
      UPDATE  opm_ord_pipe_mstr
      SET     opm_dwnld_tm = sysdate-(30/(24*60)),
              opm_rcnnct_count = :i_counter + 1,
              opm_rcnnct_tm = DECODE(:c_reconnection_flg, 'R', opm_rcnnct_tm, SYSDATE),
              opm_remark  =  to_char(sysdate,'dd-Mon-yyyy hh24:mi:ss:')|| ':' || 'Reconnecting',
              opm_conncted_on_server = decode(:c_server_flg, 'C', opm_conncted_on_server, :c_server_flg)
       WHERE  opm_xchng_cd = :c_xchng_cd
         AND  opm_pipe_id  = :c_pipe_id;

   if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "S31755", SQLMSG, c_errmsg );
      fn_aborttran( c_ServiceName, i_tran_id, c_errmsg );
      return -1;
    }

    i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_errmsg );
    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S31760", LIBMSG, c_errmsg );
      fn_aborttran( c_ServiceName, i_tran_id, c_errmsg ); 
      return -1;
     }

  }

  *i_contn_cntr = i_counter + 1;
   
return 0;
} 

/*** Ver 1.8 Ends here ***/

/*** Ver 2.9 Starts ***/
int fn_snd_rcv_msg_adap_sck ( int i_sck_id, void *ptr_v_ip_data, void *ptr_v_op_data, char *c_ServiceName, char *c_err_msg )
{
  long int li_len_write = 10;
  long int li_len_read = 10;
	int i_written;
	int i_read;

  if ( fn_set_tmr_sigdisp ( TM_30,
                            c_ServiceName,
                            c_err_msg       ) == -1 )
  {
    return -1;
  }

	i_written = write( i_sck_id, ptr_v_ip_data, li_len_write );
	fn_userlog(c_ServiceName,"After write1 :%d:",i_written);
	if ( i_written == -1 )
	{
		return -1;
	}

	fn_userlog(c_ServiceName,"After write");

	i_read = read( i_sck_id, ptr_v_op_data, li_len_read );
	if ( i_read == -1 )
	{
		return -1;
	}

	fn_userlog(c_ServiceName,"After read :%s:",ptr_v_op_data);

  fn_set_tmr_sigdisp ( TM_0,
                       c_ServiceName,
                       c_err_msg       );
  return 0;
}


int fn_checkNstart_adap (	char *c_ip_address, char *c_return_str, char *c_return_str_msg, char *c_ServiceName, char *c_err_msg )
{

	char  *ptr_c_add = '\0';
	char  *ptr_c_port = '\0';
	char  *ptr_c_sleep = '\0';
	int   i_sck_id = 0;
	int   i_ch_val = 0;
	char  c_adap_con_addrss[16];
	long int li_adap_con_port_no = 0; 
	int i_sleep = 0;
	char c_inp_data [10];
	char c_out_data [10];
	int i_out;
	int i_out2;
	char c_out [10];
	MEMSET(c_adap_con_addrss);
	MEMSET(c_inp_data);
	MEMSET(c_out_data);

	/* Read IP Address */

	strcpy(c_adap_con_addrss,c_ip_address);

	/**** Read the ors server port ****/
    ptr_c_port = (char *)fn_get_prcs_spc ( c_ServiceName, "ADAP_CON_PORT" );
    if ( ptr_c_port == NULL )
    {
      fn_errlog(c_ServiceName, "S31765", LIBMSG, c_err_msg);
      return -1;
    }
    li_adap_con_port_no = atol(ptr_c_port);
    fn_userlog(c_ServiceName," ORS_SRVR_PORT is :%ld:",li_adap_con_port_no);

	/*** Read the parameterized Sleep for ADAPTER START Check ***/
		ptr_c_sleep = (char *)fn_get_prcs_spc ( c_ServiceName, "ADAP_SLP_CHK" );
		if ( ptr_c_sleep == NULL )
    {
      fn_errlog(c_ServiceName, "S31770", LIBMSG, c_err_msg);
      return -1;
    }
		i_sleep = atoi(ptr_c_sleep);
		fn_userlog(c_ServiceName," Parameterized Sleep for ADAPTER START Check is :%d:",i_sleep);

    /**** Create a server socket ****/
    i_ch_val = fn_crt_clnt_sck( c_adap_con_addrss,
                               li_adap_con_port_no,
                               &i_sck_id,
                               c_ServiceName,
                               c_err_msg);


    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S12121", LIBMSG, c_err_msg);
      return -1;
    }

		/*** Check current status of adapter ***/

		strcpy( c_inp_data , "FO~E");
		i_ch_val = fn_snd_rcv_msg_adap_sck ( i_sck_id,
					            									 (void *)c_inp_data,
											           				 (void *)c_out_data,
														          	 c_ServiceName,
														           	 c_err_msg );	
		if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S21212", LIBMSG, c_err_msg);
			fn_close_sck(i_sck_id,c_ServiceName,c_err_msg);
      return -1;
    }

	fn_userlog(c_ServiceName,"Output is :%s:",c_out_data);
	rtrim(c_out_data);
	fn_userlog(c_ServiceName,"Output2 is :%s:",c_out_data);
	memcpy(&c_out,c_out_data,10);
	fn_userlog(c_ServiceName,"Output4 is :%s:",c_out);
	i_out = atoi(c_out);
	fn_userlog(c_ServiceName,"Output5 is :%d:",i_out);
	fn_close_sck(i_sck_id,c_ServiceName,c_err_msg);	/*** Confirm ***/

	if ( i_out == -1 )	/*** i_out is "-1" when DC_ADAPTER application is down ***/
	{
		MEMSET(c_inp_data);
		MEMSET(c_out_data);
		MEMSET(c_out);
		i_out = 0;
		i_sck_id = 0;

		 i_ch_val = fn_crt_clnt_sck( c_adap_con_addrss,
                               li_adap_con_port_no,
                               &i_sck_id,
                               c_ServiceName,
                               c_err_msg);


    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S12121", LIBMSG, c_err_msg);
      return -1;
    }

		strcpy( c_inp_data , "FO~D");
		i_ch_val = fn_snd_rcv_msg_adap_sck ( i_sck_id,
                                         (void *)c_inp_data,
                                         (void *)c_out_data,
                                         c_ServiceName,
                                         c_err_msg );
    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S21212", LIBMSG, c_err_msg);
			fn_userlog(c_ServiceName,"i_ch_val for Start is :%d:",i_ch_val);
      fn_close_sck(i_sck_id,c_ServiceName,c_err_msg);
      return -1;
    }
		fn_userlog(c_ServiceName,"Output is :%s:",c_out_data);
  	rtrim(c_out_data);
  	fn_userlog(c_ServiceName,"Output2 is :%s:",c_out_data);
  	memcpy(&c_out,c_out_data,10);
  	fn_userlog(c_ServiceName,"Output4 is :%s:",c_out);
  	i_out = atoi(c_out);
  	fn_userlog(c_ServiceName,"Output5 is :%d:",i_out);
		fn_close_sck(i_sck_id,c_ServiceName,c_err_msg);

		/*** Again Send the command for status to check wherher status of ADAPTER ***/

		sleep(i_sleep);		/*** Sleep Added as it takes time to bring Adapter down ***/

		MEMSET(c_inp_data);
    MEMSET(c_out_data);
    MEMSET(c_out);
    i_out = 0;
    i_sck_id = 0;

     i_ch_val = fn_crt_clnt_sck( c_adap_con_addrss,
                               li_adap_con_port_no,
                               &i_sck_id,
                               c_ServiceName,
                               c_err_msg);


    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S12121", LIBMSG, c_err_msg);
      return -1;
    }

    strcpy( c_inp_data , "FO~E");
    i_ch_val = fn_snd_rcv_msg_adap_sck ( i_sck_id,
                                         (void *)c_inp_data,
                                         (void *)c_out_data,
                                         c_ServiceName,
                                         c_err_msg );
    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S21212", LIBMSG, c_err_msg);
      fn_close_sck(i_sck_id,c_ServiceName,c_err_msg);
      return -1;
    }
    fn_userlog(c_ServiceName,"Output is :%s:",c_out_data);
    rtrim(c_out_data);
    fn_userlog(c_ServiceName,"Output2 is :%s:",c_out_data);
		 memcpy(&c_out,c_out_data,10);
    fn_userlog(c_ServiceName,"Output4 is :%s:",c_out);
    i_out = atoi(c_out);
    fn_userlog(c_ServiceName,"Output5 is :%d:",i_out);
    fn_close_sck(i_sck_id,c_ServiceName,c_err_msg);

		if ( i_out != 0 )
		{
			fn_userlog(c_ServiceName,"ADAPTER Not Started and i_out is :%d:",i_out);
			strcpy(c_return_str_msg,"ADAPTER Not Started");
      strcpy(c_return_str,"Not Running");
			return 0;
		}
		else if ( i_out == 0 )
		{
			fn_userlog(c_ServiceName,"ADAPTER Started Successfully and i_out is :%d:",i_out);
			strcpy(c_return_str_msg,"ADAPTER Started Successfully");
			strcpy(c_return_str,"Running");
			return 0;
		}
		else
		{
			fn_userlog(c_ServiceName,"Undefined Status Received");
			return -1;
		}
	}
	else if ( i_out == 0 )
	{
		fn_userlog(c_ServiceName,"ADAPTER Already Running");
		strcpy(c_return_str_msg,"ADAPTER Already Running");
		strcpy(c_return_str,"Running");
		return 0;
	}
	else
	{
		fn_userlog(c_ServiceName,"Undefined Status Received");
    return -1;
	}

}

int fn_checkNstop_adap( char *c_ip_address , char *c_return_str , char *c_return_str_msg , char *c_ServiceName , char *c_err_msg )
{
	char  *ptr_c_add = '\0';
  char  *ptr_c_port = '\0';
  int   i_sck_id = 0;
  int   i_ch_val = 0;
  char  c_adap_con_addrss[16];
  long int li_adap_con_port_no = 0;
	char c_prog_name [] = "cln_esr_clnt";
  char c_inp_data [10];
  char c_out_data [10];
  int i_out;
  int i_out2;
  char c_out [10];
	int d_esr_runcount = 0;
  MEMSET(c_adap_con_addrss);
  MEMSET(c_inp_data);
  MEMSET(c_out_data);

	/* Read IP Address */

	strcpy ( c_adap_con_addrss,c_ip_address );

  /**** Read the ors server port ****/
    ptr_c_port = (char *)fn_get_prcs_spc ( c_ServiceName, "ADAP_CON_PORT" );
    if ( ptr_c_port == NULL )
    {
      fn_errlog(c_ServiceName, "S31775", LIBMSG, c_err_msg);
      return -1;
    }
    li_adap_con_port_no = atol(ptr_c_port);
    fn_userlog(c_ServiceName," ORS_SRVR_PORT is :%ld:",li_adap_con_port_no);

	EXEC SQL
	SELECT count(*)
	INTO	 :d_esr_runcount
	FROM 	 OPM_ORD_PIPE_MSTR ,
				 BPS_BTCH_PGM_STTS
	WHERE  OPM_PIPE_ID 		 = BPS_PIPE_ID
	AND		 OPM_IPO_SSSN_ID = :c_adap_con_addrss
	AND		 BPS_PGM_NAME		 = :c_prog_name
	AND		 BPS_STTS				 = 'R';

	if ( SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31780", SQLMSG, c_err_msg );
		return -1;
  }

	fn_userlog(c_ServiceName," d_esr_runcount is :%d:",d_esr_runcount);

	if ( d_esr_runcount == 0 )
	{
		/**** Create a server socket ****/
    i_ch_val = fn_crt_clnt_sck( c_adap_con_addrss,
                               li_adap_con_port_no,
                               &i_sck_id,
                               c_ServiceName,
                               c_err_msg);


    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S12121", LIBMSG, c_err_msg);
      return -1;
    }

    /*** Check current status of adapter ***/

    strcpy( c_inp_data , "FO~E");
    i_ch_val = fn_snd_rcv_msg_adap_sck ( i_sck_id,
                                         (void *)c_inp_data,
                                         (void *)c_out_data,
                                         c_ServiceName,
                                         c_err_msg );
    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S21212", LIBMSG, c_err_msg);
      fn_close_sck(i_sck_id,c_ServiceName,c_err_msg);
      return -1;
    }

		fn_userlog(c_ServiceName,"Output is :%s:",c_out_data);
  	rtrim(c_out_data);
  	fn_userlog(c_ServiceName,"Output2 is :%s:",c_out_data);
  	memcpy(&c_out,c_out_data,10);
  	fn_userlog(c_ServiceName,"Output4 is :%s:",c_out);
  	i_out = atoi(c_out);
  	fn_userlog(c_ServiceName,"Output5 is :%d:",i_out);
  	fn_close_sck(i_sck_id,c_ServiceName,c_err_msg); /*** Confirm ***/

  	if ( i_out == -1 )  /*** i_out is "-1" when DC_ADAPTER application is down ***/
  	{
			fn_userlog(c_ServiceName, "Adapter Already down");
			strcpy(c_return_str_msg,"Adapter Already down");
			strcpy(c_return_str,"Not Running");
			return 0;
		}
		else if ( i_out == 0 )
		{
			MEMSET(c_inp_data);
    	MEMSET(c_out_data);
    	MEMSET(c_out);
    	i_out = 0;
    	i_sck_id = 0;

     	i_ch_val = fn_crt_clnt_sck( c_adap_con_addrss,
      	                          li_adap_con_port_no,
        	                        &i_sck_id,
          	                      c_ServiceName,
            	                    c_err_msg);


    	if ( i_ch_val == -1 )
    	{
      	fn_errlog(c_ServiceName, "S12121", LIBMSG, c_err_msg);
      	return -1;
    	}

    	strcpy( c_inp_data , "FO~s");
    	i_ch_val = fn_snd_rcv_msg_adap_sck ( i_sck_id,
     	                                     (void *)c_inp_data,
       	                                   (void *)c_out_data,
         	                                 c_ServiceName,
           	                               c_err_msg );
    	if ( i_ch_val == -1 )
    	{
        fn_errlog(c_ServiceName, "S21212", LIBMSG, c_err_msg);
      	fn_userlog(c_ServiceName,"i_ch_val for Start is :%d:",i_ch_val);
        fn_close_sck(i_sck_id,c_ServiceName,c_err_msg);
        return -1;
    	}
    	fn_userlog(c_ServiceName,"Output is :%s:",c_out_data);
    	rtrim(c_out_data);
    	fn_userlog(c_ServiceName,"Output2 is :%s:",c_out_data);
			memcpy(&c_out,c_out_data,10);
    	fn_userlog(c_ServiceName,"Output4 is :%s:",c_out);
    	i_out = atoi(c_out);
    	fn_userlog(c_ServiceName,"Output5 is :%d:",i_out);
    	fn_close_sck(i_sck_id,c_ServiceName,c_err_msg);

			if( i_out == 0 )
			{
				fn_userlog(c_ServiceName,"Adapter Successfully Stopped");
				strcpy(c_return_str_msg,"Adapter Successfully Stopped");
				strcpy(c_return_str,"Not Running");
				return 0;
			}
			else
			{
				fn_userlog(c_ServiceName,"Error occurred while Stopping Adapter");
				return -1;
			}

		}
		else
		{
			fn_userlog(c_ServiceName, "Invalid Status returned from Console");
			return -1;
		}

	}
	else
	{
		fn_userlog(c_ServiceName, "Adapter NOT Stopped because Still Mapped ESR clients are running");
		strcpy(c_return_str_msg,"Adapter NOT Stopped  because Still Mapped ESR clients are running");
		strcpy(c_return_str,"Running");
		return 0;
	}
}

int fn_checkStatus_adap (	char *c_ip_address, char *c_return_str, char *c_ServiceName, char *c_err_msg )
{

  char  *ptr_c_add = '\0';
  char  *ptr_c_port = '\0';
  int   i_sck_id = 0;
  int   i_ch_val = 0;
  char  c_adap_con_addrss[16];
  long int li_adap_con_port_no = 0;
  char c_inp_data [10];
  char c_out_data [10];
  int i_out;
  int i_out2;
  char c_out [10];
  MEMSET(c_adap_con_addrss);
  MEMSET(c_inp_data);
  MEMSET(c_out_data);

  /* Read IP Address */

	strcpy ( c_adap_con_addrss,c_ip_address );

	/**** Read the ors server port ****/
    ptr_c_port = (char *)fn_get_prcs_spc ( c_ServiceName, "ADAP_CON_PORT" );
    if ( ptr_c_port == NULL )
    {
      fn_errlog(c_ServiceName, "S31785", LIBMSG, c_err_msg);
      return -1;
    }
    li_adap_con_port_no = atol(ptr_c_port);
    fn_userlog(c_ServiceName," ORS_SRVR_PORT is :%ld:",li_adap_con_port_no);

    /**** Create a server socket ****/
    i_ch_val = fn_crt_clnt_sck( c_adap_con_addrss,
                               li_adap_con_port_no,
                               &i_sck_id,
                               c_ServiceName,
                               c_err_msg);


    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S12121", LIBMSG, c_err_msg);
      return -1;
		}

    /*** Check current status of adapter ***/

    strcpy( c_inp_data , "FO~E");
    i_ch_val = fn_snd_rcv_msg_adap_sck ( i_sck_id,
                                         (void *)c_inp_data,
                                         (void *)c_out_data,
                                         c_ServiceName,
                                         c_err_msg );
    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S21212", LIBMSG, c_err_msg);
      fn_close_sck(i_sck_id,c_ServiceName,c_err_msg);
      return -1;
    }

  fn_userlog(c_ServiceName,"Output is :%s:",c_out_data);
  rtrim(c_out_data);
  fn_userlog(c_ServiceName,"Output2 is :%s:",c_out_data);
  memcpy(&c_out,c_out_data,10);
  fn_userlog(c_ServiceName,"Output4 is :%s:",c_out);
  i_out = atoi(c_out);
  fn_userlog(c_ServiceName,"Output5 is :%d:",i_out);
  fn_close_sck(i_sck_id,c_ServiceName,c_err_msg); /*** Confirm ***/

	if ( i_out == 0 )
	{
		fn_userlog(c_ServiceName,"Adapter Running");
		strcpy(c_return_str,"Running");
		return 0;
	}
	else if ( i_out == -1 )
	{
		fn_userlog(c_ServiceName,"Adapter Not Running");
		strcpy(c_return_str,"Not Running");
		return 0;
	}
	else
	{
		fn_userlog(c_ServiceName,"Undefined Status Received");
		return -1;
	}
}

int fn_get_adap ( char *c_return_str , char *c_return_str1 , char *c_ServiceName , char *c_err_msg )
{
	char  *ptr_c_add = '\0';
	char	c_adap_con_addrss [256];
	char	c_ip_address_temp [20];
	int i_ch_val = 0;
	char c_return_str_temp[50];
	int i_val_check = 0;

	/* Read IP Address from ini file*/

  ptr_c_add = (char *) fn_get_prcs_spc( c_ServiceName, "ADAP_CON_IP_ADDR");

  if( ptr_c_add == NULL )
  {
    fn_errlog(c_ServiceName,"S12212", LIBMSG, c_err_msg);
    return -1;
  }
  strcpy ( c_adap_con_addrss , ptr_c_add );
  fn_userlog(c_ServiceName," fn_get_adap ORS_SRVR_ADDR is :%s:",c_adap_con_addrss);

	strcpy( c_return_str1 , c_adap_con_addrss );
	fn_userlog(c_ServiceName," fn_get_adap c_return_str1 is :%s:",c_return_str1);

	char* token = strtok(c_adap_con_addrss, ",");

	while (token != NULL)
	{ 
		MEMSET(c_return_str_temp);
    printf("%s\n", token);

		strcpy ( c_ip_address_temp , token );

		fn_userlog(c_ServiceName," Before call to fn_checkStatus_adap for is :%s:",c_ip_address_temp);

		i_ch_val = fn_checkStatus_adap ( c_ip_address_temp,
                                     c_return_str_temp,
                                     c_ServiceName,
                                     c_err_msg );
    if ( i_ch_val != 0 )
    {
			fn_errlog(c_ServiceName,"S12212", LIBMSG, c_err_msg);
    	return -1;
		}

		fn_userlog(c_ServiceName," c_return_str  is :%s:",c_return_str);
		fn_userlog(c_ServiceName," c_return_str_temp is :%s:",c_return_str_temp);
		if ( i_val_check == 0 )
		{
			strcpy(c_return_str,c_return_str_temp);
			i_val_check = 1;
		}
		else
		{
			strcat(c_return_str,",");
			strcat(c_return_str,c_return_str_temp);
		}
		fn_userlog(c_ServiceName," c_return_str after fn_checkStatus_adap is :%s:",c_return_str);
    token = strtok(NULL, ","); 
  }

	
	return 0;
}
/*** Ver 2.9 Ends ***/
