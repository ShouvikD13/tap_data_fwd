/******************************************************************************/
/*  version 1.0 - 27-Jun-2009 New Release   Shamili Dalvi.                    */
/*  version 1.1 -	Single Console																							*/
/*  version 1.2 - 07-Apr-2010 User ID Check. 										Sandeep Patil */
/*	version	1.3	-	30-Sep-2010	NNF Changes Data Type Of User			Sandeep Patil */
/*														ID Changed From Short Int To Long.							*/
/*  version 1.4 - 17-Oct-2012 Consolidated FNO Console NFO/BFO  Bineet S.			*/
/*  version 1.5 - 24-Dec-2014 Random Login Password generation changei | Sachin B*/
/*  version 1.6 - 14-May-2015 Auto Login on Password Expiry         | Sachin B*/
/*	version 1.8	- 01-Nov-2016	Live Connect All Issue 	|	Ritesh D.							*/
/*  version 1.9 - 11-Mar-2017 Flow of Exchange login Password generation Changes | Vishal B */	
/*	version 2.0 - 07-Mar-2018	NNF_7.21_CHANGES(PAN CHANGES)	| Parag Kanojia		*/
/*  version 2.1 - 11-Mar-2019  Trimmed Protocol for Non-NEAT Front End | Suchita Dabir */
/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sqlca.h>
#include <atmi.h>
#include <Usysflds.h>

#include <fo.h>
#include <fn_md5.h>
#include <fml_rout.h>
#include <fn_log.h>
#include <fn_env.h>
#include <fml32.h>
#include <fo_fml_def.h>
#include <fo_view_def.h>  /** Ver 1.6 **/
#include <fn_tuxlib.h>   /** Ver 1.6 **/
#include <fn_ddr.h>       /** Ver 1.6 **/

#include <fn_msgq.h>
#include <fn_scklib_tap.h>
#include <fn_read_debug_lvl.h>

#include <fo_exg_con_lib.h>
#include <fn_rndm_passwd_genrn.h>
#include <netinet/in.h>		/*** Ver 2.1 ***/
#include <fn_host_to_nw_format_nse.h>		/*** Ver 2.1 ***/

extern int  i_send_qid;

void fn_orstonse_char_pswd ( char *ptr_c_dest,
                        long int li_len_dest,
                        const char *ptr_c_src,
                        long int li_len_src );

int fn_logon_to_TAP ( FBFR32 *ptr_fml_ibuf, char *c_ServiceName, char *c_err_msg, struct st_req_q_data *st_q_con_clnt   /** Ver 1.9 **/)
{
  int   i_err[5];
  int   i_ferr[5];
  int   i_cnt;
  int   i_login_stts;
  int   i_snd_seq;
	int   i_ch_val;
	int   idx;
  int   i_tran_id;  /** Ver 1.6 **/

	char  c_tmp;
	char  *ptr_c_tmp;
  char  c_trd_dt[23];
  char  c_brkr_nm[LEN_BRKR_NAME]; 
  unsigned char digest[16];

  long int li_ip_usr_id;
  long int li_user_id;      /***  1.2 ***/

  char c_ip_pass_wd[LEN_PASS_WD];
  char c_ip_new_pass_wd[LEN_PASS_WD];
  char c_new_pass_wd_entrd;
  char c_opn_ordr_dtls_flg;
  char c_new_pass_wd[LEN_PASS_WD];  /** 1.6 **/
  char c_msg[256];                  /** 1.6 **/
  char c_crnt_passwd[10];           /** Ver 1.6 **/
  char c_command[100];              /** Ver 1.6 **/
  char c_char;                      /** Ver 1.6 **/

  struct st_sign_on_req st_exch_signon_req;
  struct st_exch_msg st_exch_message;

  /*** Commented in Ver 1.9 ***
	struct st_req_q_data st_q_con_clnt; ***/

  struct vw_mkt_msg st_msg;         /** 1.6 **/

  EXEC SQL BEGIN DECLARE SECTION;
    long int  sql_li_ip_usr_id;
    long int  sql_li_exg_brnch_id;
    long int  li_exg_max_pswd_vld_days;
		char      sql_c_ip_pipe_id[2+1];
    char      sql_c_clnt_nm[40];
    char      sql_c_clnt_stts;
    char      sql_c_opm_xchng_cd[3+1];
    char      sql_c_exg_trdr_id[5+1];
    char      sql_c_exg_brkr_id[5+1];
    char      sql_c_exg_brkr_stts;
    varchar   sql_vc_opm_lst_pswd_chg_dt[LEN_DT];
		varchar   sql_c_nxt_trd_date[ LEN_DATE ];
    varchar   sql_c_exg_brkr_name[LEN_BRKR_NAME];
		short int si_ip_new_pass_wd=0;		/* Ver 1.4 */
  EXEC SQL END DECLARE SECTION;

	MEMSET(sql_c_nxt_trd_date);
	MEMSET(sql_vc_opm_lst_pswd_chg_dt);
	MEMSET(sql_c_exg_brkr_name);
//-----------------------------------------------------------------------------
  /**** Taking input *****/
  MEMSET(c_ip_pass_wd); /** Ver 1.5 **/
  MEMSET(c_ip_new_pass_wd); /** Ver 1.5 **/
	/* Ver 1.4 starts here */
	i_err[0] = Fget32(ptr_fml_ibuf,FFO_PIPE_ID,0,(char *)sql_c_ip_pipe_id,0);
  i_ferr[0] = Ferror32;
  i_err[1] = Fget32(ptr_fml_ibuf,FFO_SPL_FLG,0,(char *)&c_opn_ordr_dtls_flg,0);
  i_ferr[1] = Ferror32;
  c_new_pass_wd_entrd = 'Y';
	/* Ver 1.4 ends here */
//-------------------------------------------------------------------------------
  for (i_cnt=0; i_cnt <= 1; i_cnt++)	/* ver 1.4 i_cnt changed from 3 to 1 */
  {
      if (i_err[i_cnt] == -1)
      {
        fn_userlog(c_ServiceName, "Error in field %d", i_cnt);
        fn_errlog(c_ServiceName,"L31005",Fstrerror32(i_ferr[i_cnt]),c_err_msg);
        return -1;
      }
  }

	
  strcpy(sql_c_clnt_nm,"cln_esr_clnt");

  /**** Checking if ESR is running or not ******/

  EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
			AND			bps_xchng_cd = 'NA'
			AND			bps_pipe_id = :sql_c_ip_pipe_id; 

	if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31015", SQLMSG, c_err_msg);
		return -1;
  }

  if(sql_c_clnt_stts == 'N')
  {
			sprintf(c_err_msg,"ESR client is not running for Pipe id :%s: and  user id :%ld:",sql_c_ip_pipe_id,li_ip_usr_id);
			return -1;
  }
	fn_userlog(c_ServiceName,"Shailesh 1");

  /**** Checking if user is already logged on in console or not ****/
  
  /******** Ver 1.6 Starts Here ****/
  EXEC SQL
  SELECT exg_max_pswd_vld_days
  INTO   :li_exg_max_pswd_vld_days
  FROM   opm_ord_pipe_mstr,
         exg_xchng_mstr
 WHERE  opm_pipe_id  = :sql_c_ip_pipe_id
   AND    opm_xchng_cd = exg_xchng_cd
   AND    exg_max_pswd_vld_days > (trunc(sysdate)- opm_lst_pswd_chg_dt);

 if(SQLCODE != 0)
 {
  if (SQLCODE == NO_DATA_FOUND)
  {
    /*** Change the Password ***/
    MEMSET(c_crnt_passwd);
    MEMSET(c_new_pass_wd);

    EXEC SQL
    SELECT OPM_EXG_PWD
      INTO :c_crnt_passwd
      FROM OPM_ORD_PIPE_MSTR
     WHERE OPM_PIPE_ID=:sql_c_ip_pipe_id
      AND OPM_XCHNG_CD='NFO';
    if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "L31020", SQLMSG, c_err_msg);
      return -1;
    }  
    
    rtrim(c_crnt_passwd);
  
    fn_decrypt(c_crnt_passwd,c_crnt_passwd);
    fn_userlog(c_ServiceName,"Existingg password :%s:",c_crnt_passwd);
    fn_get_passwd(c_crnt_passwd,c_new_pass_wd,CRNT_PSSWD_LEN);
    fn_userlog(c_ServiceName,"New password Generated :%s:",c_new_pass_wd);
    rtrim(c_new_pass_wd);
    fn_encrypt(c_new_pass_wd,c_new_pass_wd);
    fn_userlog(c_ServiceName,"Encrypted password :%s:",c_new_pass_wd);
    
    i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
    if ( i_tran_id == -1 )
    {
      fn_errlog(c_ServiceName, "L31025", TPMSG, c_err_msg);
      return -1;
    }

    EXEC SQL UPDATE OPM_ORD_PIPE_MSTR
                 SET OPM_NEW_EXG_PWD=:c_new_pass_wd
                 WHERE OPM_PIPE_ID = :sql_c_ip_pipe_id /*** LIKE DECODE(:c_pipe_id,'*','%',:c_pipe_id) ***/
                 AND OPM_XCHNG_CD = 'NFO';
    if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "S31005", SQLMSG, c_err_msg );
      fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
      return -1;
    }

    i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
    if ( i_ch_val == -1 )
    {
      fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
      fn_errlog(c_ServiceName, "S31010", TPMSG, c_err_msg );
      return -1;
    }

    MEMSET(c_command);
    fn_decrypt(c_new_pass_wd,c_new_pass_wd);
    fn_userlog(c_ServiceName,"Decrypted password :%s:",c_new_pass_wd);
    c_char='"';

    sprintf(c_command,"ksh snd_passwd_chng_mail.sh %s '%c%s%c'", sql_c_ip_pipe_id,c_char,c_new_pass_wd,c_char);
    system(c_command);

    EXEC SQL
     SELECT to_char(sysdate, 'dd-Mon-yyyy hh24:mi:ss') INTO :st_msg.c_tm_stmp FROM dual;


    sprintf(c_msg,"|%s- %s| %s",st_msg.c_tm_stmp,"Password changed successfully",c_new_pass_wd);

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
      fn_errlog(c_ServiceName,"S31015", SQLMSG, c_err_msg);
      return -1; 
    }

    fn_cpy_ddr(st_msg.c_rout_crt);

    i_ch_val = fn_acall_svc ( c_ServiceName,
                                  c_err_msg,
                                  &st_msg,
                                  "vw_mkt_msg",
                                  sizeof (struct vw_mkt_msg ),
                                  TPNOREPLY|TPNOTRAN,
                                  "SFO_UPD_MSG" );

    if ( i_ch_val == SYSTEM_ERROR )
    {
      fn_errlog ( c_ServiceName, "S31020", LIBMSG, c_err_msg );
      return -1;  
  
    }

  }
  else
  {
    fn_errlog(c_ServiceName, "L31030", SQLMSG, c_err_msg);
    return -1;
  }
 }
  /*** Ver 1.6 Ends Here *****/

	EXEC SQL
		SELECT opm_login_stts,
           opm_user_id,      /***  1.2 ***/
					 opm_exg_pwd,													/* Ver 1.4 */
					 opm_new_exg_pwd						/* Ver 1.4 */
		INTO   :i_login_stts,
           :li_user_id,
					 :c_ip_pass_wd,												/* Ver 1.4 */	 
					 :c_ip_new_pass_wd:si_ip_new_pass_wd	/* Ver 1.4 */
		FROM   opm_ord_pipe_mstr
		WHERE  opm_pipe_id = :sql_c_ip_pipe_id;

	if ( SQLCODE != 0 )
	{
		fn_errlog(c_ServiceName, "L31035", SQLMSG, c_err_msg );
		return -1;
	}

  rtrim(c_ip_new_pass_wd); /** Ver 1.5 **/
  rtrim(c_ip_pass_wd);  /** Ver 1.5 **/

	/* Ver 1.4 Starts */
  if(si_ip_new_pass_wd == -1 )
  {
    strcpy(c_ip_new_pass_wd, "\0");
    c_new_pass_wd_entrd = 'N';
  }else
	{
		c_new_pass_wd_entrd = 'Y';
	}

  /*** Ver 1.5 Starts Here  *****/
  fn_decrypt(c_ip_pass_wd,c_ip_pass_wd);

  rtrim(c_ip_pass_wd);

  if( c_new_pass_wd_entrd == 'Y' )
  {
    fn_decrypt(c_ip_new_pass_wd,c_ip_new_pass_wd);
  }
  /*** Ver 1.5 Ends Here ****/
  rtrim(c_ip_new_pass_wd);

   li_ip_usr_id =(long int) li_user_id;	
   fn_userlog(c_ServiceName,"c_ip_new_pass_wd :%s:",c_ip_new_pass_wd); 
   fn_userlog(c_ServiceName,"li_ip_usr_id :%ld:",li_ip_usr_id);   
   fn_userlog(c_ServiceName,"sql_c_ip_pipe_id :%s:",sql_c_ip_pipe_id); 
	/* Ver 1.4 Ends */

	fn_userlog(c_ServiceName,"Shailesh 2");
	if( li_user_id  != li_ip_usr_id)      /***  Ver 1.2 ***/
  {
    fn_userlog(c_ServiceName,"Inside User ID Check");
    fn_userlog(c_ServiceName,"li_user_id Is :%ld:",li_user_id);
    fn_userlog(c_ServiceName,"li_ip_usr_id Is :%ld:",li_ip_usr_id);
    sprintf ( c_err_msg, "Please Enter Correct User ID For Pipe Id :%s:,user id :%ld:",sql_c_ip_pipe_id,li_user_id);
    return -1;

  }

	fn_userlog(c_ServiceName,"Shailesh 3");
	if(i_login_stts == 1)
	{
		sprintf ( c_err_msg, "User is already logged in for pipe id :%s:,user id :%ld:",sql_c_ip_pipe_id,li_ip_usr_id);
		return -1;
	}

	fn_userlog(c_ServiceName,"Shailesh 4");

  
	fn_userlog(c_ServiceName,"Shailesh 5");

	EXEC SQL
    SELECT opm_xchng_cd,
           to_char ( opm_lst_pswd_chg_dt, 'dd-mon-yyyy' ),
           opm_trdr_id,
           opm_brnch_id
    INTO   :sql_c_opm_xchng_cd,
           :sql_vc_opm_lst_pswd_chg_dt,
           :sql_c_exg_trdr_id,
           :sql_li_exg_brnch_id
    FROM   opm_ord_pipe_mstr
    WHERE  opm_pipe_id = :sql_c_ip_pipe_id;

  	if (SQLCODE != 0)
  	{
     fn_errlog(c_ServiceName, "L31045", SQLMSG, c_err_msg);
     return -1;
    }

	EXEC SQL
    SELECT exg_brkr_id,
           exg_nxt_trd_dt,
           nvl(exg_brkr_name,' '),
           exg_brkr_stts
    INTO  :sql_c_exg_brkr_id,
          :sql_c_nxt_trd_date,
          :sql_c_exg_brkr_name,
          :sql_c_exg_brkr_stts
    FROM  exg_xchng_mstr
    WHERE exg_xchng_cd = :sql_c_opm_xchng_cd;

  	if(SQLCODE != 0)
  	{
     fn_errlog(c_ServiceName, "L31050", SQLMSG, c_err_msg);
     return -1;
    }

  SETNULL ( sql_c_exg_brkr_name );
	fn_userlog(c_ServiceName,"Shailesh 6");

  /** Initialization of TAP logon packet **/

   memset(&st_exch_signon_req, '\0', sizeof(st_exch_signon_req));
   memset(&st_exch_message, '\0', sizeof(st_exch_message));

/******* commented in ver 2.1  *****
	fn_orstonse_char( st_exch_signon_req.st_hdr.c_filler_1,
                    2,
                    " ",
                    1); *******/
 
  st_exch_signon_req.st_hdr.li_trader_id   = atol(sql_c_exg_trdr_id); /*** ver 2.1 ***/


  st_exch_signon_req.st_hdr.li_log_time = 0;

  fn_orstonse_char ( st_exch_signon_req.st_hdr.c_alpha_char,
                     LEN_ALPHA_CHAR,
                     " ",
                     1);


  st_exch_signon_req.st_hdr.si_transaction_code = SIGN_ON_REQUEST_IN;

  st_exch_signon_req.st_hdr.si_error_code = 0;
  fn_orstonse_char ( st_exch_signon_req.st_hdr.c_filler_2,
                     8,
                     " ",
                     1);

  memset( st_exch_signon_req.st_hdr.c_time_stamp_1, 0, LEN_TIME_STAMP);
  memset( st_exch_signon_req.st_hdr.c_time_stamp_2, 0, LEN_TIME_STAMP);


  st_exch_signon_req.st_hdr.si_message_length = sizeof ( st_exch_signon_req );

	/***	Commented In Ver 1.3
  st_exch_signon_req.si_user_id =  li_ip_usr_id;
	*************************/

	st_exch_signon_req.li_user_id =  li_ip_usr_id;			/***	Ver	1.3 Data Type Changed From Short Int To Long	***/
	fn_userlog(c_ServiceName,"Shailesh 7");
  
  	fn_orstonse_char ( st_exch_signon_req.c_reserved_1,	/*** Added in Ver 2.0 ***/
										 8,
										 " ",
										 1);
	
  /******* Ver 1.5 Starts here *******/
	fn_orstonse_char_pswd ( st_exch_signon_req.c_password,
                     LEN_PASSWORD,
                     c_ip_pass_wd,
                     strlen (c_ip_pass_wd));
	fn_userlog(c_ServiceName,"Shailesh 71");

	fn_orstonse_char ( st_exch_signon_req.c_reserved_2, /*** Added in Ver 2.0 ***/
                     8,
                     " ",
                     1);

  fn_orstonse_char_pswd ( st_exch_signon_req.c_new_password,
                     LEN_PASSWORD,
                     c_ip_new_pass_wd,
                     strlen (c_ip_new_pass_wd ));
	fn_userlog(c_ServiceName,"Shailesh 72");
  /***** Ver 1.5 Ends here *********/
 
  fn_orstonse_char ( st_exch_signon_req.c_trader_name,
                     LEN_TRADER_NAME,
                     sql_c_exg_trdr_id,
                     strlen (sql_c_exg_trdr_id ));
	fn_userlog(c_ServiceName,"Shailesh 73");

  st_exch_signon_req.li_last_password_change_date = 0;
	fn_userlog(c_ServiceName,"Shailesh 74");

  fn_orstonse_char ( st_exch_signon_req.c_broker_id,
                     LEN_BROKER_ID,
                     sql_c_exg_brkr_id,
                     strlen (sql_c_exg_brkr_id ));
	fn_userlog(c_ServiceName,"Shailesh 75");

   st_exch_signon_req.c_filler_1 = ' ';
	fn_userlog(c_ServiceName,"Shailesh 76");

   st_exch_signon_req.si_branch_id = sql_li_exg_brnch_id;
	fn_userlog(c_ServiceName,"Shailesh 77");
	
	if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_hdr.si_message_length : %d ", st_exch_signon_req.st_hdr.si_message_length
);
	fn_userlog(c_ServiceName,"Shailesh 78");
  fn_userlog(c_ServiceName,"st_exch_signon_req.li_user_id : %ld ", st_exch_signon_req.li_user_id);			/***	Ver	1.3	***/
	fn_userlog(c_ServiceName,"Shailesh 8");
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_password : %s ", st_exch_signon_req.c_password);
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_new_password : %s ", st_exch_signon_req.c_new_password);
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_trader_name : %s ", st_exch_signon_req.c_trader_name);
  fn_userlog(c_ServiceName,"st_exch_signon_req.li_last_password_change_date : %ld ", st_exch_signon_req.li_last_password_change_date);
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_broker_id : %s ", st_exch_signon_req.c_broker_id);
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_filler_1 : %c ", st_exch_signon_req.c_filler_1);
  fn_userlog(c_ServiceName,"st_exch_signon_req.si_branch_id : %d ", st_exch_signon_req.si_branch_id);
	}

  ptr_c_tmp = (char*) fn_get_prcs_spc(c_ServiceName, "VERSION_NUMBER");

  if(ptr_c_tmp == NULL)
  {
    fn_errlog(c_ServiceName,"L31055", LIBMSG, c_err_msg);
    return -1;
  }			

	st_exch_signon_req.li_version_number = atol(ptr_c_tmp);

  st_exch_signon_req.li_batch_2_start_time = 0;

  st_exch_signon_req.c_host_switch_context = ' ';

  fn_orstonse_char ( st_exch_signon_req.c_colour,
                     LEN_COLOUR,
                     " ",
                     1);

   st_exch_signon_req.c_filler_2 = ' ';

  ptr_c_tmp = (char*) fn_get_prcs_spc(c_ServiceName, "USER_TYPE");

  if(ptr_c_tmp == NULL)
  {
    fn_errlog(c_ServiceName,"L31060", LIBMSG, c_err_msg);
    return -1;
  }
  st_exch_signon_req.si_user_type = atoi(ptr_c_tmp);

  st_exch_signon_req.d_sequence_number = 0;

  ptr_c_tmp = (char*) fn_get_prcs_spc(c_ServiceName, "WSC_NAME");

  if(ptr_c_tmp == NULL)
  {
    fn_errlog(c_ServiceName,"L31065", LIBMSG, c_err_msg);
    return -1;
  }

  fn_orstonse_char ( st_exch_signon_req.c_ws_class_name,
                     LEN_WS_CLASS_NAME,
                     ptr_c_tmp,
                     strlen(ptr_c_tmp) );

  st_exch_signon_req.c_broker_status = ' ';
/***  st_exch_signon_req.c_show_index = ' ';		*** Commented in Ver 2.1 ***/
	st_exch_signon_req.c_show_index = 'T'; 		/*** Added in ver 2.1 ***/
  st_exch_signon_req.st_mkt_allwd_lst.flg_normal_mkt  = 0;
  st_exch_signon_req.st_mkt_allwd_lst.flg_oddlot_mkt  = 0;
  st_exch_signon_req.st_mkt_allwd_lst.flg_spot_mkt    = 0;
  st_exch_signon_req.st_mkt_allwd_lst.flg_auction_mkt = 0;
  st_exch_signon_req.st_mkt_allwd_lst.flg_filler1     = 0;
  st_exch_signon_req.st_mkt_allwd_lst.flg_filler2     = 0;
  st_exch_signon_req.si_member_type = 0;
  st_exch_signon_req.c_clearing_status = ' ';

  strcpy (c_brkr_nm, (char*) sql_c_exg_brkr_name.arr);


	fn_orstonse_char ( st_exch_signon_req.c_broker_name,
                     LEN_BROKER_NAME,
                     c_brkr_nm,
                     strlen(c_brkr_nm));

	/*** Ver 2.0 Starts ***/
	fn_orstonse_char ( st_exch_signon_req.c_reserved_3,
										 16,
										 " ",
										 1);

	fn_orstonse_char ( st_exch_signon_req.c_reserved_4,
                     16,
                     " ",
                     1);

	fn_orstonse_char ( st_exch_signon_req.c_reserved_5,
                     16,
                     " ",
                     1);
	/*** Ver 2.0 Ends ***/

  if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"st_exch_signon_req.li_version_number : %ld ", st_exch_signon_req.li_version_number);

  fn_userlog(c_ServiceName,"st_exch_signon_req.li_batch_2_start_time : %ld ", st_exch_signon_req.li_batch_2_start_time);

  fn_userlog(c_ServiceName,"st_exch_signon_req.c_host_switch_context : %c ", st_exch_signon_req.c_host_switch_context);

  fn_userlog(c_ServiceName,"st_exch_signon_req.c_colour : %s ", st_exch_signon_req.c_colour);

	fn_userlog(c_ServiceName,"st_exch_signon_req.c_filler_2 : %c ", st_exch_signon_req.c_filler_2);

	fn_userlog(c_ServiceName,"st_exch_signon_req.si_user_type : %d ", st_exch_signon_req.si_user_type);

	fn_userlog(c_ServiceName,"st_exch_signon_req.d_sequence_number : %d ", st_exch_signon_req.d_sequence_number);

	fn_userlog(c_ServiceName,"st_exch_signon_req.c_ws_class_name : %s ", st_exch_signon_req.c_ws_class_name);

  fn_userlog(c_ServiceName,"st_exch_signon_req.c_broker_status : %c ", st_exch_signon_req.c_broker_status);

  fn_userlog(c_ServiceName,"st_exch_signon_req.c_show_index : %c ", st_exch_signon_req.c_show_index);

	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_normal_mkt:%d", st_exch_signon_req.st_mkt_allwd_lst.flg_normal_mkt); 
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_oddlot_mkt:%d", st_exch_signon_req.st_mkt_allwd_lst.flg_oddlot_mkt);
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_spot_mkt:%d", st_exch_signon_req.st_mkt_allwd_lst.flg_spot_mkt);
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_auction_mkt:%d",st_exch_signon_req.st_mkt_allwd_lst.flg_auction_mkt);
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_filler1:%d",st_exch_signon_req.st_mkt_allwd_lst.flg_filler1);
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_filler2:%d", st_exch_signon_req.st_mkt_allwd_lst.flg_filler2);
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_clearing_status : %c ", st_exch_signon_req.c_clearing_status);

  fn_userlog(c_ServiceName,"st_exch_signon_req.c_broker_name : %s ", st_exch_signon_req.c_broker_name);
	/*** Ver 2.0 Starts ***/
	fn_userlog(c_ServiceName,"st_exch_signon_req.c_reserved_1 is :%s:",st_exch_signon_req.c_reserved_1);
	fn_userlog(c_ServiceName,"st_exch_signon_req.c_reserved_2 is :%s:",st_exch_signon_req.c_reserved_2);
	fn_userlog(c_ServiceName,"st_exch_signon_req.c_reserved_3 is :%s:",st_exch_signon_req.c_reserved_3);
	fn_userlog(c_ServiceName,"st_exch_signon_req.c_reserved_4 is :%s:",st_exch_signon_req.c_reserved_4);
	fn_userlog(c_ServiceName,"st_exch_signon_req.c_reserved_5 is :%s:",st_exch_signon_req.c_reserved_5);
	/*** Ver 2.0 Ends ***/
	}

	fn_cnvt_htn_sign_on_req_in(&st_exch_signon_req);		/*** Ver 2.1 ***/

  memcpy(&(st_exch_message.st_exch_sndmsg),&st_exch_signon_req ,sizeof ( st_exch_signon_req ));

  strcpy (c_trd_dt, (char*) sql_c_nxt_trd_date.arr);

  i_ch_val = fn_get_reset_seq( sql_c_ip_pipe_id,
															 c_trd_dt,
															 GET_PLACED_SEQ,
															 &i_snd_seq,
															 c_ServiceName,
															 c_err_msg);
	if ( i_ch_val == -1 )
	{
    fn_errlog(c_ServiceName,"L31070", LIBMSG, c_err_msg);
    return -1;
	}

  st_exch_message.st_net_header.i_seq_num = i_snd_seq;

	if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"The sequence number got is:%d:",i_snd_seq);
	}


  /****call routine for md5 ******/

  fn_cal_md5digest((void *)&st_exch_signon_req,sizeof(st_exch_signon_req),digest);

  /*****Copy the checksum in network header *******/

   memcpy(st_exch_message.st_net_header.c_checksum, digest,sizeof(digest));


  /******Set length of the network header *******/

   st_exch_message.st_net_header.si_message_length = sizeof(st_exch_message.st_net_header) + sizeof(st_exch_signon_req);
	
	if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"c_opn_ordr_dtls_flg :%c:",c_opn_ordr_dtls_flg);
	}

	if (c_opn_ordr_dtls_flg == 'G')
	{
		/** Commented in Ver 1.9 **
  	st_q_con_clnt.li_msg_type = LOGIN_WITH_OPEN_ORDR_DTLS;  ** 1.9 **/

		st_q_con_clnt->li_msg_type = LOGIN_WITH_OPEN_ORDR_DTLS;   /** Ver 1.9 **/
  }	
  else
	{
		/** Commented in Ver 1.9 **
  	st_q_con_clnt.li_msg_type = LOGIN_WITHOUT_OPEN_ORDR_DTLS; ** 1.9 **/
		
		st_q_con_clnt->li_msg_type = LOGIN_WITHOUT_OPEN_ORDR_DTLS;   /** Ver 1.9 **/
	}

	/** Commented in Ver 1.9 **
  memcpy(&(st_q_con_clnt.st_exch_msg_data),&st_exch_message,sizeof (st_exch_message));  ** 1.9 **/

	fn_cnvt_htn_net_hdr(&st_exch_message.st_net_header);		/*** Ver 2.1 ***/

	memcpy(&(st_q_con_clnt->st_exch_msg_data),&st_exch_message,sizeof (st_exch_message));  /** Ver 1.9 **/

	if(DEBUG_MSG_LVL_0)  /** Ver 1.9 **/
  {
		fn_userlog(c_ServiceName,"i_send_qid  :%d:",i_send_qid);
  }



	fn_userlog(c_ServiceName,"Shailesh 9");

  return 0;
}

int fn_logoff_from_TAP ( char *sql_c_ip_pipe_id, char *c_ServiceName, char *c_err_msg)
{
    int i_snd_seq;
    int i_ch_val;
    char c_trd_dt[23];
    unsigned char digest[16];

    struct st_int_header st_hdr;
    struct st_exch_msg st_exch_message;
		struct st_req_q_data st_q_con_clnt;	

		EXEC SQL BEGIN DECLARE SECTION;
			char     sql_c_opm_xchng_cd[3+1];
			varchar  sql_c_nxt_trd_date[ LEN_DATE ];
			char		 sql_c_exg_trdr_id[5+1];			/*** Added in Ver 2.1 ***/
		EXEC SQL END DECLARE SECTION;

		MEMSET(sql_c_nxt_trd_date);

		EXEC SQL
			SELECT opm_xchng_cd,
						 opm_trdr_id					/*** Added in Ver 2.1 ***/
			INTO   :sql_c_opm_xchng_cd,
						 :sql_c_exg_trdr_id		/*** Added in Ver 2.1 ***/
			FROM   opm_ord_pipe_mstr
			WHERE  opm_pipe_id = :sql_c_ip_pipe_id;

			if (SQLCODE != 0)
			{
			 fn_errlog(c_ServiceName, "L31080", SQLMSG, c_err_msg);
			 return -1;
			}

		EXEC SQL
			SELECT exg_nxt_trd_dt
			INTO  :sql_c_nxt_trd_date
			FROM  exg_xchng_mstr
			WHERE exg_xchng_cd = :sql_c_opm_xchng_cd;

			if(SQLCODE != 0)
			{
			 fn_errlog(c_ServiceName, "L31085", SQLMSG, c_err_msg);
			 return -1;
			}

		fn_userlog(c_ServiceName,"The Trade date is :%s:",sql_c_nxt_trd_date.arr);

    memset(&st_exch_message, '\0', sizeof(st_exch_message));

/***		fn_orstonse_char ( st_hdr.c_filler_1,	*** Commented in Ver 2.1 ***
                     2,
                     " ",
                     1);	Ver 2.1 ***/

		st_hdr.li_trader_id   = atol(sql_c_exg_trdr_id); 	/*** ver 2.1 ***/

    st_hdr.li_log_time = 0;

    fn_orstonse_char ( st_hdr.c_alpha_char,
                     LEN_ALPHA_CHAR,
                     " ",
                     1);

    st_hdr.si_transaction_code = SIGN_OFF_REQUEST_IN;

    st_hdr.si_error_code = 0;

    fn_orstonse_char ( st_hdr.c_filler_2,
                     8,
                     " ",
                     1);

    memset( st_hdr.c_time_stamp_1, 0, LEN_TIME_STAMP);

    memset( st_hdr.c_time_stamp_2, 0, LEN_TIME_STAMP);

    st_hdr.si_message_length = sizeof ( st_hdr );


    memcpy(&(st_exch_message.st_exch_sndmsg),&st_hdr,sizeof (st_hdr));

    strcpy (c_trd_dt, (char*) sql_c_nxt_trd_date.arr);

    i_ch_val = fn_get_reset_seq( sql_c_ip_pipe_id,
																 c_trd_dt,
																 GET_PLACED_SEQ,
																 &i_snd_seq,
																 c_ServiceName,
																 c_err_msg);
		if ( i_ch_val == -1 )
		{
			fn_errlog(c_ServiceName,"L31090", LIBMSG, c_err_msg);
			return -1;
		}

    st_exch_message.st_net_header.i_seq_num = i_snd_seq;
    fn_userlog(c_ServiceName,"The sequence number got is:%d:",i_snd_seq);

    /****call routine for md5 ******/

	  fn_cal_md5digest((void *)&st_hdr,sizeof(st_hdr),digest);

    /*****Copy the checksum in network header *******/

		memcpy(st_exch_message.st_net_header.c_checksum, digest,sizeof(digest));

		/******Set length of the network header *******/

		st_exch_message.st_net_header.si_message_length = sizeof(st_exch_message.st_net_header) + sizeof(st_hdr);

		st_q_con_clnt.li_msg_type = SIGN_OFF_REQUEST_IN;
		memcpy(&(st_q_con_clnt.st_exch_msg_data),&st_exch_message,sizeof (st_exch_message));

		if(fn_write_msg_q(i_send_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)
		{
			fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
    	fn_errlog(c_ServiceName, "L31095", LIBMSG, c_err_msg);
			return -1;
  	}

  return 0;
}

/** 1.1 **/

int fn_connect_all_logon_to_TAP ( FBFR32 *ptr_fml_ibuf, char *c_ServiceName, char *c_err_msg, struct st_req_q_data *st_q_con_clnt  /** Ver 1.9 **/)
{
  int   i_err[5];
  int   i_ferr[5];
  int   i_cnt;
  int   i_login_stts;
  int   i_snd_seq;
	int   i_ch_val;
	int   idx;
	/** int   i_send_queue_new;  ** Commented in Ver 1.9 **/
  int   i_msgq_typ;
  int   i_tran_id;   						/** Ver 1.6 **/

	char  c_tmp;
	char  *ptr_c_tmp;
  char  c_trd_dt[23];
  char  c_brkr_nm[LEN_BRKR_NAME]; 
  char  ptr_c_fileName [512];
  unsigned char digest[16];

  long int li_ip_usr_id;
	long int li_user_id;      /***  1.4 ***/

  char c_ip_pass_wd[LEN_PASS_WD];
  char c_ip_new_pass_wd[LEN_PASS_WD];
  char c_new_pass_wd_entrd;
  char c_opn_ordr_dtls_flg;
  char c_new_pass_wd[LEN_PASS_WD];  /** 1.6 **/
  char c_msg[256];                  /** 1.6 **/
  char c_crnt_passwd[10];           /** Ver 1.6 **/
  char c_command[100];              /** Ver 1.6 **/
  char c_char;                      /** Ver 1.6 **/

  struct st_sign_on_req st_exch_signon_req;
  struct st_exch_msg st_exch_message;

/** Commented in Ver 1.9 ** 
	struct st_req_q_data st_q_con_clnt; ** 1.9 **/

  struct vw_mkt_msg st_msg;         /** 1.6 **/

  EXEC SQL BEGIN DECLARE SECTION;
    long int  sql_li_ip_usr_id;
    long int  sql_li_exg_brnch_id;
    long int  li_exg_max_pswd_vld_days;
		char      sql_c_ip_pipe_id[2+1];
    char      sql_c_clnt_nm[40];
    char      sql_c_clnt_stts;
    char      sql_c_opm_xchng_cd[3+1];
    char      sql_c_exg_trdr_id[5+1];
    char      sql_c_exg_brkr_id[5+1];
    char      sql_c_exg_brkr_stts;
    varchar   sql_vc_opm_lst_pswd_chg_dt[LEN_DT];
		varchar   sql_c_nxt_trd_date[ LEN_DATE ];
    varchar   sql_c_exg_brkr_name[LEN_BRKR_NAME];
		short int si_ip_new_pass_wd=0;/* Ver 1.4 */
  EXEC SQL END DECLARE SECTION;

	MEMSET(sql_c_nxt_trd_date);
	MEMSET(sql_vc_opm_lst_pswd_chg_dt);
	MEMSET(sql_c_exg_brkr_name);

  /**** Taking input *****/

	/* Commented in  ver 1.4 *
  i_err[0] = Fget32(ptr_fml_ibuf,FFO_EXCHNG_USR_ID,0,(char *)&li_ip_usr_id,0);
  i_ferr[0] = Ferror32;
  i_err[1] = Fget32(ptr_fml_ibuf,FFO_PIPE_ID,0,(char *)sql_c_ip_pipe_id,0);
  i_ferr[1] = Ferror32;
  i_err[2] = Fget32(ptr_fml_ibuf,FFO_SPL_FLG,0,(char *)&c_opn_ordr_dtls_flg,0);
  i_ferr[2] = Ferror32;
  i_err[3] = Fget32(ptr_fml_ibuf,FFO_PASS_WD,0,(char *)c_ip_pass_wd,0);
  i_ferr[3] = Ferror32;
  c_new_pass_wd_entrd = 'Y';
  i_err[4] = Fget32(ptr_fml_ibuf,FFO_NEW_PASS_WD,0,(char *)c_ip_new_pass_wd,0);
  i_ferr[4] = Ferror32;
		
		fn_userlog(c_ServiceName,"c_ip_new_pass_wd :%s:",c_ip_new_pass_wd);
	*/

	/* Ver 1.4 starts here */
	i_err[1] = Fget32(ptr_fml_ibuf,FFO_PIPE_ID,0,(char *)sql_c_ip_pipe_id,0);
  i_ferr[1] = Ferror32;
  i_err[2] = Fget32(ptr_fml_ibuf,FFO_SPL_FLG,0,(char *)&c_opn_ordr_dtls_flg,0);
  i_ferr[2] = Ferror32;

  c_new_pass_wd_entrd = 'Y';
  /* Ver 1.4 ends here */

	if(DEBUG_MSG_LVL_0)			/*** Ver 1.8 ***/
  {
    fn_userlog(c_ServiceName,"sql_c_ip_pipe_id 		:%s:",sql_c_ip_pipe_id);
    fn_userlog(c_ServiceName,"c_opn_ordr_dtls_flg :%c:",c_opn_ordr_dtls_flg);
	}

	/*  for (i_cnt=0; i_cnt <= 1; i_cnt++)	* ver 1.4 i_cnt changed from 3 to 1 */ /*** Commented In ver 2.0 ***/
	for (i_cnt=1; i_cnt <= 2; i_cnt++)	/*** Added In Ver 2.0 ***/
  {
		if(DEBUG_MSG_LVL_0)     /*** Ver 1.8 ***/
  	{
      fn_userlog(c_ServiceName,"i_cnt 				:%d:",i_cnt);
      fn_userlog(c_ServiceName,"i_err[i_cnt] 	:%d:",i_err[i_cnt]);
		}

      if (i_err[i_cnt] == -1)
      {
        fn_userlog(c_ServiceName, "Error in field %d", i_cnt);
        fn_errlog(c_ServiceName,"L31100",Fstrerror32(i_ferr[i_cnt]),c_err_msg);
        return -1;
      }
  }

	/* commented in ver 1.4 
  if((i_err[4] == -1) && (i_ferr[4] != FNOTPRES))
  {
    fn_errlog(c_ServiceName, "L31105",Fstrerror32(i_ferr[4]), c_err_msg);
    return -1;
  }
  else if(i_err[4] == -1)
  {
    c_new_pass_wd_entrd = 'N';
    strcpy(c_ip_new_pass_wd, "\0");
  }
	*/

  strcpy(sql_c_clnt_nm,"cln_esr_clnt");

	/*** Commented in Ver 1.9 ***

  sprintf(ptr_c_fileName, "%s/%s.ini", getenv("BIN"), sql_c_ip_pipe_id);

	i_ch_val = fn_init_prcs_spc( "SFO_CON_CLNT",
                                ptr_c_fileName,
                                "SFO_CON_CLNT" );
	if (i_ch_val == -1)
	{
		fn_userlog (c_ServiceName,"Initializing Process space is failed " );
		return ( -1 );
	}

***** Taking access of Transmit queque. *****

	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"SEND_QUEUE");
	i_msgq_typ = atoi(ptr_c_tmp);

	if(fn_crt_msg_q(&i_send_queue_new, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
	{
		fn_userlog (c_ServiceName,"Failed to take access of Transmit queque");
		return ( -1 );
  }

	*** Ver 1.9 Comment Ends ***/
	
  /**** Checking if ESR is running or not ******/

  EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
			AND			bps_xchng_cd = 'NA'
			AND			bps_pipe_id = :sql_c_ip_pipe_id; 

	if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31110", SQLMSG, c_err_msg);
		return -1;
  }

	 /* Ver 1.4 same check is performed after collecting data from opm_ord_pipe_mstr 
  if(sql_c_clnt_stts == 'N')
  {
			sprintf(c_err_msg,"ESR client is not running for Pipe id :%s: and  user id :%ld:",sql_c_ip_pipe_id,li_ip_usr_id);
			return -1;
  }
	*/

  /******** Ver 1.6 Starts Here ****/
  EXEC SQL
  SELECT exg_max_pswd_vld_days
  INTO   :li_exg_max_pswd_vld_days
  FROM   opm_ord_pipe_mstr,
         exg_xchng_mstr
 WHERE  opm_pipe_id  = :sql_c_ip_pipe_id
   AND    opm_xchng_cd = exg_xchng_cd
   AND    exg_max_pswd_vld_days > (trunc(sysdate)- opm_lst_pswd_chg_dt);

 if(SQLCODE != 0)
 {
  if (SQLCODE == NO_DATA_FOUND)
  {
    /*** Change the Password ***/
    MEMSET(c_crnt_passwd);
    MEMSET(c_new_pass_wd);

    EXEC SQL
    SELECT OPM_EXG_PWD
      INTO :c_crnt_passwd
      FROM OPM_ORD_PIPE_MSTR
     WHERE OPM_PIPE_ID=:sql_c_ip_pipe_id
      AND OPM_XCHNG_CD='NFO';
    if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "L31115", SQLMSG, c_err_msg);
      return -1;
    }

    rtrim(c_crnt_passwd);

    fn_decrypt(c_crnt_passwd,c_crnt_passwd);
    fn_userlog(c_ServiceName,"Existingg password :%s:",c_crnt_passwd);
    fn_get_passwd(c_crnt_passwd,c_new_pass_wd,CRNT_PSSWD_LEN);
    fn_userlog(c_ServiceName,"New password Generated :%s:",c_new_pass_wd);
    rtrim(c_new_pass_wd);
    fn_encrypt(c_new_pass_wd,c_new_pass_wd);
    fn_userlog(c_ServiceName,"Encrypted password :%s:",c_new_pass_wd);

    i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
    if ( i_tran_id == -1 )
    {
      fn_errlog(c_ServiceName, "L31120", TPMSG, c_err_msg);
      return -1;
    }
  
    EXEC SQL UPDATE OPM_ORD_PIPE_MSTR
                 SET OPM_NEW_EXG_PWD=:c_new_pass_wd
                 WHERE OPM_PIPE_ID = :sql_c_ip_pipe_id /*** LIKE DECODE(:c_pipe_id,'*','%',:c_pipe_id) ***/
                 AND OPM_XCHNG_CD = 'NFO';
    if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "S31025", SQLMSG, c_err_msg );
      fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
      return -1;
    }

    i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
    if ( i_ch_val == -1 )
    {
      fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
      fn_errlog(c_ServiceName, "S31030", TPMSG, c_err_msg );
      return -1;
    }

    MEMSET(c_command);
    fn_decrypt(c_new_pass_wd,c_new_pass_wd);
    fn_userlog(c_ServiceName,"Decrypted password :%s:",c_new_pass_wd);
    c_char='"';

    sprintf(c_command,"ksh snd_passwd_chng_mail.sh %s '%c%s%c'", sql_c_ip_pipe_id,c_char,c_new_pass_wd,c_char);
    system(c_command);

    EXEC SQL
     SELECT to_char(sysdate, 'dd-Mon-yyyy hh24:mi:ss') INTO :st_msg.c_tm_stmp FROM dual;


    sprintf(c_msg,"|%s- %s| %s",st_msg.c_tm_stmp,"Password changed successfully",c_new_pass_wd);

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
      fn_errlog(c_ServiceName,"S31035", SQLMSG, c_err_msg);
      return -1;
    }

    fn_cpy_ddr(st_msg.c_rout_crt);
    
    i_ch_val = fn_acall_svc ( c_ServiceName,
                                  c_err_msg,
                                  &st_msg,
                                  "vw_mkt_msg",
                                  sizeof (struct vw_mkt_msg ),
                                  TPNOREPLY|TPNOTRAN,
                                  "SFO_UPD_MSG" );

    if ( i_ch_val == SYSTEM_ERROR )
    {
      fn_errlog ( c_ServiceName, "S31040", LIBMSG, c_err_msg );
      return -1;

    }
   
  }
  else
  {
    fn_errlog(c_ServiceName, "L31125", SQLMSG, c_err_msg);
    return -1;
  }
 }
 /**** Ver 1.6 Ends here ****/

  /**** Checking if user is already logged on in console or not ****/

	EXEC SQL
		SELECT opm_login_stts,
					 opm_user_id,     /* Ver 1.4 */
           opm_exg_pwd,     /* Ver 1.4 */
           opm_new_exg_pwd  /* Ver 1.4 */
		INTO   :i_login_stts,
					 :li_user_id,													/* Ver 1.4 */
           :c_ip_pass_wd,												/* Ver 1.4 */
           :c_ip_new_pass_wd:si_ip_new_pass_wd	/* Ver 1.4 */
		FROM   opm_ord_pipe_mstr
		WHERE  opm_pipe_id = :sql_c_ip_pipe_id;

	if ( SQLCODE != 0 )
	{
		fn_errlog(c_ServiceName, "L31130", SQLMSG, c_err_msg );
		return -1;
	}

  /* Ver 1.4 Starts */
  if(si_ip_new_pass_wd == -1 )
  {
    strcpy(c_ip_new_pass_wd, "\0");
    c_new_pass_wd_entrd = 'N';
  }else
  {
    c_new_pass_wd_entrd = 'Y';
  }

  /*** Ver 1.5 Starts Here *****/
   rtrim(c_ip_new_pass_wd); 
   rtrim(c_ip_pass_wd); 

   fn_decrypt(c_ip_pass_wd,c_ip_pass_wd);
   rtrim(c_ip_pass_wd);

   if( c_new_pass_wd_entrd == 'Y' )
   {
    fn_decrypt(c_ip_new_pass_wd,c_ip_new_pass_wd);
    rtrim(c_ip_new_pass_wd);
   }
  /*** Ver 1.5 Ends Here ****/


  li_ip_usr_id=(long int) li_user_id;


   fn_userlog(c_ServiceName,"c_ip_pass_wd :%s:",c_ip_pass_wd);
   fn_userlog(c_ServiceName,"c_ip_new_pass_wd :%s:",c_ip_new_pass_wd);
   fn_userlog(c_ServiceName,"li_ip_usr_id :%ld:",li_ip_usr_id);
   fn_userlog(c_ServiceName,"sql_c_ip_pipe_id :%s:",sql_c_ip_pipe_id);


  if(sql_c_clnt_stts == 'N')
  {
      sprintf(c_err_msg,"ESR client is not running for Pipe id :%s: and  user id :%ld:",sql_c_ip_pipe_id,li_ip_usr_id);
      return -1;
  }
  /* Ver 1.4 Ends */

	if(i_login_stts == 1)
	{
		sprintf ( c_err_msg, "User is already logged in for pipe id :%s:,user id :%ld:",sql_c_ip_pipe_id,li_ip_usr_id);
		return -1;
	}

	if (c_new_pass_wd_entrd == 'N')
  {
    EXEC SQL
      SELECT exg_max_pswd_vld_days
      INTO   :li_exg_max_pswd_vld_days
      FROM   opm_ord_pipe_mstr,
             exg_xchng_mstr
      WHERE  opm_pipe_id  = :sql_c_ip_pipe_id
      AND    opm_xchng_cd = exg_xchng_cd
      AND    exg_max_pswd_vld_days > (trunc(sysdate)- opm_lst_pswd_chg_dt);

      if(SQLCODE != 0)
      {
        if (SQLCODE == NO_DATA_FOUND)
        {
          fn_errlog(c_ServiceName, "B28508", DEFMSG, c_err_msg);
          return -1;
        }
        else
        {
          fn_errlog(c_ServiceName, "L31135", SQLMSG, c_err_msg);
          return -1;
        }
      }
  }

	EXEC SQL
    SELECT opm_xchng_cd,
           to_char ( opm_lst_pswd_chg_dt, 'dd-mon-yyyy' ),
           opm_trdr_id,
           opm_brnch_id
    INTO   :sql_c_opm_xchng_cd,
           :sql_vc_opm_lst_pswd_chg_dt,
           :sql_c_exg_trdr_id,
           :sql_li_exg_brnch_id
    FROM   opm_ord_pipe_mstr
    WHERE  opm_pipe_id = :sql_c_ip_pipe_id;

  	if (SQLCODE != 0)
  	{
     fn_errlog(c_ServiceName, "L31140", SQLMSG, c_err_msg);
     return -1;
    }

	EXEC SQL
    SELECT exg_brkr_id,
           exg_nxt_trd_dt,
           nvl(exg_brkr_name,' '),
           exg_brkr_stts
    INTO  :sql_c_exg_brkr_id,
          :sql_c_nxt_trd_date,
          :sql_c_exg_brkr_name,
          :sql_c_exg_brkr_stts
    FROM  exg_xchng_mstr
    WHERE exg_xchng_cd = :sql_c_opm_xchng_cd;

  	if(SQLCODE != 0)
  	{
     fn_errlog(c_ServiceName, "L31145", SQLMSG, c_err_msg);
     return -1;
    }

  SETNULL ( sql_c_exg_brkr_name );

  /** Initialization of TAP logon packet **/

   memset(&st_exch_signon_req, '\0', sizeof(st_exch_signon_req));
   memset(&st_exch_message, '\0', sizeof(st_exch_message));

  /***** commented in ver 2.1 *****
	fn_orstonse_char( st_exch_signon_req.st_hdr.c_filler_1,
                    2,
                    " ",
                    1); ********/
  st_exch_signon_req.st_hdr.li_trader_id   = atol(sql_c_exg_trdr_id);  /***** ver 2.1 ***/


  st_exch_signon_req.st_hdr.li_log_time = 0;

  fn_orstonse_char ( st_exch_signon_req.st_hdr.c_alpha_char,
                     LEN_ALPHA_CHAR,
                     " ",
                     1);


  st_exch_signon_req.st_hdr.si_transaction_code = SIGN_ON_REQUEST_IN;

  st_exch_signon_req.st_hdr.si_error_code = 0;
  fn_orstonse_char ( st_exch_signon_req.st_hdr.c_filler_2,
                     8,
                     " ",
                     1);

  memset( st_exch_signon_req.st_hdr.c_time_stamp_1, 0, LEN_TIME_STAMP);
  memset( st_exch_signon_req.st_hdr.c_time_stamp_2, 0, LEN_TIME_STAMP);


  st_exch_signon_req.st_hdr.si_message_length = sizeof ( st_exch_signon_req );

	/***	Commented In Ver 1.3
  st_exch_signon_req.si_user_id =  li_ip_usr_id;
	***********************/

  st_exch_signon_req.li_user_id =  li_ip_usr_id;			/***	Ver	1.3	Datat Type Changed From Short Int To Long	***/
  
  /****** Commented in Ver 1.5 ****************************
	fn_orstonse_char ( st_exch_signon_req.c_password,
                     LEN_PASSWORD,
                     c_ip_pass_wd,
                     strlen (c_ip_pass_wd));

  fn_orstonse_char ( st_exch_signon_req.c_new_password,
                     LEN_PASSWORD,
                     c_ip_new_pass_wd,
                     strlen (c_ip_new_pass_wd ));
  ************ Ver 1.5 **********************************/

	fn_orstonse_char ( st_exch_signon_req.c_reserved_1, /*** Added in Ver 2.0 ***/
                     8,
                     " ",
                     1);

  /*********** Ver 1.5 Starts Here *********************/
  fn_orstonse_char_pswd ( st_exch_signon_req.c_password,
                     LEN_PASSWORD,
                     c_ip_pass_wd,
                     strlen (c_ip_pass_wd));

	fn_orstonse_char ( st_exch_signon_req.c_reserved_2, /*** Added in Ver 2.0 ***/
                     8,
                     " ",
                     1);

  fn_orstonse_char_pswd ( st_exch_signon_req.c_new_password,
                     LEN_PASSWORD,
                     c_ip_new_pass_wd,
                     strlen (c_ip_new_pass_wd ));
  /*********** Ver 1.5 Ends Here ***********************/
  fn_orstonse_char ( st_exch_signon_req.c_trader_name,
                     LEN_TRADER_NAME,
                     sql_c_exg_trdr_id,
                     strlen (sql_c_exg_trdr_id ));

  st_exch_signon_req.li_last_password_change_date = 0;

  fn_orstonse_char ( st_exch_signon_req.c_broker_id,
                     LEN_BROKER_ID,
                     sql_c_exg_brkr_id,
                     strlen (sql_c_exg_brkr_id ));

   st_exch_signon_req.c_filler_1 = ' ';

   st_exch_signon_req.si_branch_id = sql_li_exg_brnch_id;
	
	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"st_exch_signon_req.st_hdr.si_message_length : %d ", st_exch_signon_req.st_hdr.si_message_length);
  	fn_userlog(c_ServiceName,"st_exch_signon_req.li_user_id : %ld ", st_exch_signon_req.li_user_id);			/***	Ver	1.3	***/
  	fn_userlog(c_ServiceName,"st_exch_signon_req.c_password : %s ", st_exch_signon_req.c_password);
  	fn_userlog(c_ServiceName,"st_exch_signon_req.c_new_password : %s ", st_exch_signon_req.c_new_password);
  	fn_userlog(c_ServiceName,"st_exch_signon_req.c_trader_name : %s ", st_exch_signon_req.c_trader_name);
  	fn_userlog(c_ServiceName,"st_exch_signon_req.li_last_password_change_date : %ld ", st_exch_signon_req.li_last_password_change_date);
  	fn_userlog(c_ServiceName,"st_exch_signon_req.c_broker_id : %s ", st_exch_signon_req.c_broker_id);
  	fn_userlog(c_ServiceName,"st_exch_signon_req.c_filler_1 : %c ", st_exch_signon_req.c_filler_1);
  	fn_userlog(c_ServiceName,"st_exch_signon_req.si_branch_id : %d ", st_exch_signon_req.si_branch_id);
	}

  ptr_c_tmp = (char*) fn_get_prcs_spc(c_ServiceName, "VERSION_NUMBER");

  if(ptr_c_tmp == NULL)
  {
    fn_errlog(c_ServiceName,"L31150", LIBMSG, c_err_msg);
    return -1;
  }			

	st_exch_signon_req.li_version_number = atol(ptr_c_tmp);

  st_exch_signon_req.li_batch_2_start_time = 0;

  st_exch_signon_req.c_host_switch_context = ' ';

  fn_orstonse_char ( st_exch_signon_req.c_colour,
                     LEN_COLOUR,
                     " ",
                     1);

   st_exch_signon_req.c_filler_2 = ' ';

  ptr_c_tmp = (char*) fn_get_prcs_spc(c_ServiceName, "USER_TYPE");

  if(ptr_c_tmp == NULL)
  {
    fn_errlog(c_ServiceName,"L31155", LIBMSG, c_err_msg);
    return -1;
  }
  st_exch_signon_req.si_user_type = atoi(ptr_c_tmp);

  st_exch_signon_req.d_sequence_number = 0;

  ptr_c_tmp = (char*) fn_get_prcs_spc(c_ServiceName, "WSC_NAME");

  if(ptr_c_tmp == NULL)
  {
    fn_errlog(c_ServiceName,"L31160", LIBMSG, c_err_msg);
    return -1;
  }

  fn_orstonse_char ( st_exch_signon_req.c_ws_class_name,
                     LEN_WS_CLASS_NAME,
                     ptr_c_tmp,
                     strlen(ptr_c_tmp) );

  st_exch_signon_req.c_broker_status = ' ';
  /** st_exch_signon_req.c_show_index = ' '; commented in ver 2.1 **/
  st_exch_signon_req.c_show_index = 'T'; /*** added in ver 2.1 ***/
  st_exch_signon_req.st_mkt_allwd_lst.flg_normal_mkt  = 0;
  st_exch_signon_req.st_mkt_allwd_lst.flg_oddlot_mkt  = 0;
  st_exch_signon_req.st_mkt_allwd_lst.flg_spot_mkt    = 0;
  st_exch_signon_req.st_mkt_allwd_lst.flg_auction_mkt = 0;
  st_exch_signon_req.st_mkt_allwd_lst.flg_filler1     = 0;
  st_exch_signon_req.st_mkt_allwd_lst.flg_filler2     = 0;
  st_exch_signon_req.si_member_type = 0;
  st_exch_signon_req.c_clearing_status = ' ';

  strcpy (c_brkr_nm, (char*) sql_c_exg_brkr_name.arr);


	fn_orstonse_char ( st_exch_signon_req.c_broker_name,
                     LEN_BROKER_NAME,
                     c_brkr_nm,
                     strlen(c_brkr_nm));

	/*** Ver 2.0 Starts ***/
  fn_orstonse_char ( st_exch_signon_req.c_reserved_3,
                     16,
                     " ",
                     1);

  fn_orstonse_char ( st_exch_signon_req.c_reserved_4,
                     16,
                     " ",
                     1);

  fn_orstonse_char ( st_exch_signon_req.c_reserved_5,
                     16,
                     " ",
                     1);
  /*** Ver 2.0 Ends ***/

  if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"st_exch_signon_req.li_version_number : %ld ", st_exch_signon_req.li_version_number);

  fn_userlog(c_ServiceName,"st_exch_signon_req.li_batch_2_start_time : %ld ", st_exch_signon_req.li_batch_2_start_time);

  fn_userlog(c_ServiceName,"st_exch_signon_req.c_host_switch_context : %c ", st_exch_signon_req.c_host_switch_context);

  fn_userlog(c_ServiceName,"st_exch_signon_req.c_colour : %s ", st_exch_signon_req.c_colour);

	fn_userlog(c_ServiceName,"st_exch_signon_req.c_filler_2 : %c ", st_exch_signon_req.c_filler_2);

	fn_userlog(c_ServiceName,"st_exch_signon_req.si_user_type : %d ", st_exch_signon_req.si_user_type);

	fn_userlog(c_ServiceName,"st_exch_signon_req.d_sequence_number : %d ", st_exch_signon_req.d_sequence_number);

	fn_userlog(c_ServiceName,"st_exch_signon_req.c_ws_class_name : %s ", st_exch_signon_req.c_ws_class_name);

  fn_userlog(c_ServiceName,"st_exch_signon_req.c_broker_status : %c ", st_exch_signon_req.c_broker_status);

  fn_userlog(c_ServiceName,"st_exch_signon_req.c_show_index : %c ", st_exch_signon_req.c_show_index);

	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_normal_mkt:%d", st_exch_signon_req.st_mkt_allwd_lst.flg_normal_mkt); 
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_oddlot_mkt:%d", st_exch_signon_req.st_mkt_allwd_lst.flg_oddlot_mkt);
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_spot_mkt:%d", st_exch_signon_req.st_mkt_allwd_lst.flg_spot_mkt);
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_auction_mkt:%d",st_exch_signon_req.st_mkt_allwd_lst.flg_auction_mkt);
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_filler1:%d",st_exch_signon_req.st_mkt_allwd_lst.flg_filler1);
	fn_userlog(c_ServiceName,"st_exch_signon_req.st_mkt_allwd_lst.flg_filler2:%d", st_exch_signon_req.st_mkt_allwd_lst.flg_filler2);
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_clearing_status : %c ", st_exch_signon_req.c_clearing_status);

  fn_userlog(c_ServiceName,"st_exch_signon_req.c_broker_name : %s ", st_exch_signon_req.c_broker_name);

	/*** Ver 2.0 Starts ***/
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_reserved_1 is :%s:",st_exch_signon_req.c_reserved_1
);
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_reserved_2 is :%s:",st_exch_signon_req.c_reserved_2
);
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_reserved_3 is :%s:",st_exch_signon_req.c_reserved_3
);
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_reserved_4 is :%s:",st_exch_signon_req.c_reserved_4
);
  fn_userlog(c_ServiceName,"st_exch_signon_req.c_reserved_5 is :%s:",st_exch_signon_req.c_reserved_5
);
  /*** Ver 2.0 Ends ***/
	}

  memcpy(&(st_exch_message.st_exch_sndmsg),&st_exch_signon_req ,sizeof ( st_exch_signon_req ));

  strcpy (c_trd_dt, (char*) sql_c_nxt_trd_date.arr);

  i_ch_val = fn_get_reset_seq( sql_c_ip_pipe_id,
															 c_trd_dt,
															 GET_PLACED_SEQ,
															 &i_snd_seq,
															 c_ServiceName,
															 c_err_msg);
	if ( i_ch_val == -1 )
	{
    fn_errlog(c_ServiceName,"L31165", LIBMSG, c_err_msg);
    return -1;
	}

  st_exch_message.st_net_header.i_seq_num = i_snd_seq;

	if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"The sequence number got is:%d:",i_snd_seq);
	}


  /****call routine for md5 ******/

  fn_cal_md5digest((void *)&st_exch_signon_req,sizeof(st_exch_signon_req),digest);

  /*****Copy the checksum in network header *******/

   memcpy(st_exch_message.st_net_header.c_checksum, digest,sizeof(digest));


  /******Set length of the network header *******/

   st_exch_message.st_net_header.si_message_length = sizeof(st_exch_message.st_net_header) + sizeof(st_exch_signon_req);
	
	if(DEBUG_MSG_LVL_3){
	fn_userlog(c_ServiceName,"c_opn_ordr_dtls_flg :%c:",c_opn_ordr_dtls_flg);
	}

	if (c_opn_ordr_dtls_flg == 'G')
	{
		/** Commented in Ver 1.9 **
  	st_q_con_clnt.li_msg_type = LOGIN_WITH_OPEN_ORDR_DTLS;  ** 1.9 **/
	
		st_q_con_clnt->li_msg_type = LOGIN_WITH_OPEN_ORDR_DTLS;  /** Ver 1.9 **/
  }	
  else
	{
		/** Commented in Ver 1.9 **
  	st_q_con_clnt.li_msg_type = LOGIN_WITHOUT_OPEN_ORDR_DTLS;  ** 1.9 **/

		st_q_con_clnt->li_msg_type = LOGIN_WITHOUT_OPEN_ORDR_DTLS;   /** Ver 1.9 **/
	}

	/** Commented in Ver 1.9 **
  memcpy(&(st_q_con_clnt.st_exch_msg_data),&st_exch_message,sizeof (st_exch_message));  ** 1.9 **/

	memcpy(&(st_q_con_clnt->st_exch_msg_data),&st_exch_message,sizeof (st_exch_message));  /** Ver 1.9 **/

/*** Commented in Ver 1.9 ***

  if(fn_write_msg_q(i_send_queue_new, 
									  (void *)&st_q_con_clnt, 
										sizeof(st_q_con_clnt), 
										c_ServiceName, 
										c_err_msg) == -1)

  {
    fn_errlog(c_ServiceName, "L31170", LIBMSG, c_err_msg);
    return -1;
  }

	*** Comments ends Ver 1.9 ***/

  return 0;
 }

void fn_orstonse_char_pswd ( char *ptr_c_dest, long int li_len_dest, const char *ptr_c_src, long int li_len_src )
{
  long int li_cnt;

  for ( li_cnt=0; li_cnt<li_len_src; li_cnt++ )
  {
    *(ptr_c_dest + li_cnt) =  *(ptr_c_src + li_cnt) ;
  }
  memset ( ( ptr_c_dest + li_len_src ), ' ', ( li_len_dest - li_len_src ) );
}

