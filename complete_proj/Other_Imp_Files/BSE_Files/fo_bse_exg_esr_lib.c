/**********************************************************************************/
/*  Library           : fo_bse_exg_esr_lib.pc                                     */ 
/*                                                                                */
/*  Description       : Library function to handle the Exchange Login logoff      */
/*                      responses.                                                */
/*                                                                                */
/* Log                :New Release : Nitin Save,Sachin Birje.                     */
/*                    :Ver 1.1 | Reduction in Database transactions | Sachin      */
/*                    :Ver 1.2 | Bug Fixing                         | Sachin      */
/*                    :Ver 1.3 | Avoiding the updaate of FUm Volume | Sachin      */
/*                    :Ver 1.4 | Auto password generation logic chng| Sandip      */
/*                    :Ver 1.6 | Contract master & Trade Quote merger Changes     */ 
/*																																	| Ritesh D    */
/*                    :Ver 1.7 | Bse Re-deployment		(Ravi Malla)					      */ 
/*                    :Ver 1.8 | BSE Derivatives Download Changes   | Prajakta S  */
/**********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sqlca.h>
#include <atmi.h>
#include <fn_tuxlib.h>
#include <fml32.h>
#include <Usysflds.h>
#include <memory.h>
#include <fo.h>
#include <fn_ddr.h>      
#include <fo_view_def.h>
#include <fn_log.h>
#include <fn_read_debug_lvl.h>
#include <pthread.h>
#include <memory.h>
#include <fo_bse_exg_esr_lib.h>
#include <fn_pthread.h>
#include <fn_scklib_iml.h>
#include <fn_fo_bse_fprint.h>					  
#include <fn_msgq.h>
#include<fn_fo_bse_convrtn.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <ETILayouts.h>   /* Ver 1.7 */

int fn_snd_opn_intrst_rqst(char *c_ServiceName, char *c_err_msg);

EXEC SQL BEGIN DECLARE SECTION;
 extern  char sql_c_xchng_cd[3+1];
 extern  varchar sql_c_nxt_trd_date[ LEN_DATE ];
 extern  char sql_c_pipe_id [ 2+1 ];
EXEC SQL END DECLARE SECTION;

extern int i_iml_sck_id;
extern int i_msg_dump_flg;
extern int i_cond_snd_rcv_cntrl;
extern int i_rcv_thrd_exit_stts; 
extern pthread_cond_t cond_snd_rcv_cntrl;
extern pthread_mutex_t mut_snd_rcv_cntrl;
extern int  i_snd_qid;


/*commented in ver 1.7 int fn_bse_logon_req_out(struct st_bfo_logon_reply *ptr_st_logon_rply, char *c_ServiceName, char *c_err_msg )*/
int fn_bse_logon_req_out(UserLoginResponseT *ptr_st_logon_rply, char *c_ServiceName, char *c_err_msg )  /* Ver 1.7 */
{
  char c_msg[256];
  char c_brkr_id[5];  
  char c_rply_txt[50];

  int i_ch_val;
  int i_rtn_cd;
  int i_trnsctn;
  long l_user_id=0;  
  long l_msg_typ;
  long l_rply_cd;

  /**  commented in ver 1.7
  ptr_st_logon_rply->s_reply_code = ntohs(ptr_st_logon_rply->s_reply_code);
  ptr_st_logon_rply->s_reply_code = fn_swap_short(ptr_st_logon_rply->s_reply_code);  ver 1.7 **/
  if(DEBUG_MSG_LVL_3)
  {
   /* fn_userlog(c_ServiceName," Login Reply Code in fn_bse_logon_req_out :%d;",ptr_st_logon_rply->s_reply_code); commented in ver 1.7 */ 
   fn_userlog(c_ServiceName," Login Reply Code in fn_bse_logon_req_out "); /* Ver 1.7 */
	}

  EXEC  SQL
  		SELECT  EXG_BRKR_ID
  		INTO  :c_brkr_id
  		FROM  exg_xchng_mstr
  		WHERE exg_xchng_cd = 'BFO';
  
	if(SQLCODE != 0)
  {
   fn_errlog(c_ServiceName, "L31005", SQLMSG, c_err_msg);
   return -1;
  } 
  rtrim(c_brkr_id);

  EXEC SQL
  		SELECT OPM_USER_ID
  		INTO   :l_user_id
  		FROM   OPM_ORD_PIPE_MSTR
  		WHERE  OPM_PIPE_ID =:sql_c_pipe_id;
 	if(SQLCODE != 0)
  {
   fn_errlog(c_ServiceName, "L31010", SQLMSG, c_err_msg);
   return -1;
  }  
	/**  commented in ver 1.7
	l_msg_typ = fn_swap_long(ptr_st_logon_rply->l_msg_tag);
  strcpy(c_rply_txt,ptr_st_logon_rply->c_reply_txt);
  rtrim(c_rply_txt); 
  sprintf(c_msg,"User id-%ld,MSG_TYP-%ld,|%d-|%s|",l_user_id,l_msg_typ,ptr_st_logon_rply->s_reply_code,c_rply_txt);
	ver 1.7 **/

	sprintf(c_msg,"User id-%ld,MSG_TYP-%ld,|%s-|%s|",l_user_id,TID_USER_LOGIN_RESPONSE,sql_c_pipe_id,"ETI login successful."); /* Ver 1.7 */

  i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn == -1)
  {
   fn_errlog(c_ServiceName,"L31015", LIBMSG, c_err_msg);
   return -1;
  }
 
  EXEC SQL
  		INSERT INTO FTM_FO_TRD_MSG
      		        (FTM_XCHNG_CD,
          		     FTM_BRKR_CD,
              		 FTM_MSG_ID,
                	 FTM_MSG,
     		           FTM_TM
        		      )
    		          values
       		        (
  		             'BFO',
     		           :c_brkr_id,
        		       'T',
            		   :c_msg,
                	 sysdate
                  );
  if(SQLCODE != 0)
  {
   fn_errlog(c_ServiceName, "L31020", SQLMSG, c_err_msg);
   fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
   return -1;
  }
  i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_ch_val == -1 )
  {
   fn_errlog(c_ServiceName,"L31025", LIBMSG, c_err_msg);
   return -1;
  } 

	/*** commmented in ver 1.7 
  if(ptr_st_logon_rply->s_reply_code == LOGIN_SUCCESS )
  { ver 1.7 ***/
    if( DEBUG_MSG_LVL_0 )
    {
     fn_userlog(c_ServiceName,"###########################################");     
     fn_userlog(c_ServiceName," FINAL BSE LOGON SUCCESSFUL ");
     fn_userlog(c_ServiceName,"###########################################");
    }
    i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
    if ( i_trnsctn == -1)
    {
      fn_errlog(c_ServiceName,"L31030", LIBMSG, c_err_msg);
      return -1;
    }
    
    EXEC SQL
      UPDATE OPM_ORD_PIPE_MSTR
      SET    OPM_LOGIN_STTS = 1
      WHERE  OPM_PIPE_ID = :sql_c_pipe_id;
 
    if(SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "L31035", SQLMSG, c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return -1;
    } 
    i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
   
    if ( i_ch_val == -1 )
    {
     fn_errlog(c_ServiceName,"L31040", LIBMSG, c_err_msg);
     return -1;
    }
    fn_pst_trg ( c_ServiceName, "TRG_ORS_CON",c_msg, sql_c_pipe_id);
    return 0;   
     
	/* Ver 1.7 commented   
  } */
  /*else if( ptr_st_logon_rply->s_reply_code == PASSWORD_EXPIRED ) ver 1.7 commented*/
	if(ptr_st_logon_rply->DaysLeftForPasswdExpiry <= 3)   /* Added in Ver 1.7 */
  {
    if( DEBUG_MSG_LVL_0 )
    { 
     fn_userlog(c_ServiceName,"###########################################");
     fn_userlog(c_ServiceName," BSE LOGIN PASSWORD GETS EXPIRED ");
     fn_userlog(c_ServiceName,"###########################################");
    }
  
		/** Function to be called to send password change request mandatory **/
    if( DEBUG_MSG_LVL_0 )
    {
     fn_userlog(c_ServiceName," Raising request to change the password ");
    }
 
		
    /** Added  for signal handelling  Starts */
		i_cond_snd_rcv_cntrl = PASSWD_RES_RCVD;	 
		pthread_cond_signal(&cond_snd_rcv_cntrl);

    /** Added  for signal handelling  End */

    /*****  Added password change request part here  *****/
   
	  i_ch_val = fn_chng_exp_passwd_req( sql_c_pipe_id, MANDATORY_PASSWD_CHNG, c_ServiceName, c_err_msg );

    if ( i_ch_val == -1 )
	  {
	   fn_errlog(c_ServiceName,"L31045", LIBMSG, c_err_msg);
     fn_userlog(c_ServiceName,"###########################################");
	   fn_userlog(c_ServiceName," Error in request of change password ");
	   fn_userlog(c_ServiceName,"###########################################"); 
     return -1;
	  }

 	  fn_userlog(c_ServiceName," After successfully raising request to change the password ");
	  return 0;
  }
	/** commented in ver 1.7  
  else
  {
   fn_userlog(c_ServiceName,"###########################################");
   fn_userlog(c_ServiceName," BSE Login Failed ");
   fn_userlog(c_ServiceName,"###########################################"); 
   return 0; 
  } commented in ver 1.7 **/
    
}

/*****************************************************************************/
									/** Sending Logon Request Start  ***/
/*****************************************************************************/

int fn_logon_to_bse( char *c_ServiceName, char *c_pipe_id, char *c_err_msg  )
{
  int   i_cnt;
  int   i_login_stts;
  int   i_snd_seq;
  int   i_ch_val;
  int   i_ret_val;
  int   idx;
  char  c_tmp;
  long int li_ip_usr_id;
  long int li_user_id;
  
  char c_curnt_passwd[LEN_PASS_WD];
  char c_ip_new_pass_wd[LEN_PASS_WD];
  char c_new_pass_wd_entrd;
  char c_opn_ordr_dtls_flg;
   
  EXEC SQL BEGIN DECLARE SECTION;
    long int  sql_li_ip_usr_id;
    char c_sql_pipe_id[2+1];
  EXEC SQL END DECLARE SECTION; 
 
  MEMSET(c_sql_pipe_id); 
  strcpy(c_sql_pipe_id , c_pipe_id);

  /* struct st_bfo_logon_req st_signon_req; commented in Ver 1.7 */
	UserLoginRequestT st_signon_req;            /* Ver 1.7 */
  struct st_send_rqst_data st_q_con_clnt;
 
  MEMSET(st_signon_req);
  MEMSET(st_q_con_clnt);
  MEMSET(c_curnt_passwd);

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"Val of c_pipe_id :%s:",c_sql_pipe_id);
  }
  
  EXEC SQL
    SELECT BPH_CURNT_PASSWD ,
					 TO_NUMBER(TRIM(EXG_BRKR_ID)||'00'|| LPAD(BPH_USER_ID,3,'0')),   /* <Brokerd id>00<userid> */	/* Ver 1.7 */
					 TO_NUMBER(TRIM(EXG_BRKR_ID)||'00'|| LPAD(BPH_USER_ID,3,'0')),  /* <Brokerd id>00<userid> */ /* Ver 1.7 */
					 (SUBSTR(BPH_PIPE_ID,2,1) || '0') /* <pipe id[1] || '0' <Sequence> */ /* ver 1.7 */
    INTO   :c_curnt_passwd,
					 :st_signon_req.Username,	 /* Ver 1.7 */
					 :st_signon_req.RequestHeader.SenderSubID,	 /* Ver 1.7 */
					 :st_signon_req.RequestHeader.MsgSeqNum	 /* Ver 1.7 */
    FROM   BPH_BFO_PSSWD_HNDLG,
					 EXG_XCHNG_MSTR  /* ver 1.7 */
    WHERE  BPH_PIPE_ID = :c_sql_pipe_id	
					 AND EXG_XCHNG_CD='BFO';  /* ver 1.7 */ 
  
  if ( SQLCODE != 0)
  {
   fn_userlog(c_ServiceName,"Error while selecting Pipe specific and Exchange specific details.");
   fn_errlog(c_ServiceName, "L31050", SQLMSG, c_err_msg);
   return ( -1 );
  }
  
  rtrim(c_curnt_passwd);
   
  fn_userlog(c_ServiceName," Login Password  :%s:",c_curnt_passwd);
 
  /**** Coping the login Data in exchange structure *****/


	/************* Commented in Ver 1.7 
  st_signon_req.st_hdr.l_slot_no = htonl(REQ_SLOT_NO);
  st_signon_req.l_msg_typ = htonl(LOGON_REQ);
  strcpy((char *)st_signon_req.c_password,(char *)c_curnt_passwd); 
  st_signon_req.c_filler = ' ';
  st_signon_req.l_msg_tag = htonl(LOGIN_MSG_TAG);
  st_signon_req.st_hdr.l_message_length = htonl(sizeof(st_signon_req) - sizeof(st_signon_req.st_hdr));
  st_q_con_clnt.li_msg_type = LOGON_REQ;
  st_signon_req.st_hdr.l_slot_no = fn_swap_long(st_signon_req.st_hdr.l_slot_no);
  st_signon_req.l_msg_typ = fn_swap_long(st_signon_req.l_msg_typ);
  st_signon_req.l_msg_tag = fn_swap_long(st_signon_req.l_msg_tag);
  st_signon_req.st_hdr.l_message_length = fn_swap_long(st_signon_req.st_hdr.l_message_length);

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_signon_req,sizeof(st_signon_req));

  **** End of Coping login Data in exchange structure ****
  if ( DEBUG_MSG_LVL_4 )
  {  
	 fn_userlog(c_ServiceName," Message Length in fn_logon_to_bse :%d:",sizeof(st_signon_req));
   fn_userlog(c_ServiceName,"l_slot_no  in fn_logon_to_bse :%ld:",st_signon_req.st_hdr.l_slot_no);
   fn_userlog(c_ServiceName,"l_msg_typ in fn_logon_to_bse  :%ld:",st_signon_req.l_msg_typ);
   fn_userlog(c_ServiceName,"l_msg_tag in fn_logon_to_bse  :%ld:",st_signon_req.l_msg_tag);
   fn_userlog(c_ServiceName,"l_message_length in fn_logon_to_bse :%ld:",st_signon_req.st_hdr.l_message_length);
   fn_userlog(c_ServiceName,"li_msg_type in fn_logon_to_bse  :%ld:",st_q_con_clnt.li_msg_type); 
   fn_userlog(c_ServiceName,"Before call to fn_write_msg_q --Val of i_snd_qid <%d>",i_snd_qid); 
   fn_userlog(c_ServiceName,"Before call to fn_write_msg_q --Val of c_ServiceName <%s>",c_ServiceName); 
  }
	commenting in ver 1.7 ended **/

	/* Ver 1.7 Start */

  st_signon_req.MessageHeaderIn.BodyLen = htonl(sizeof(UserLoginRequestT));
  st_signon_req.MessageHeaderIn.TemplateID = htons(TID_USER_LOGIN_REQUEST);
  strcpy(st_signon_req.MessageHeaderIn.NetworkMsgID,"");
  strcpy(st_signon_req.MessageHeaderIn.Pad2,"");
  st_signon_req.RequestHeader.MsgSeqNum = 0;
  strcpy((char *)st_signon_req.Password,(char *)c_curnt_passwd);


	 fn_userlog(c_ServiceName,"User Login Request TemplateID :%d: BodyLen :%ld:  Username :%ld: Psswd :%s: SenderSubID :%ld:",st_signon_req.MessageHeaderIn.TemplateID,st_signon_req.MessageHeaderIn.BodyLen,st_signon_req.Username,st_signon_req.Password,st_signon_req.RequestHeader.SenderSubID);

  st_signon_req.MessageHeaderIn.BodyLen = fn_swap_ulong(st_signon_req.MessageHeaderIn.BodyLen);
  st_signon_req.MessageHeaderIn.TemplateID = fn_swap_ushort(st_signon_req.MessageHeaderIn.TemplateID);
  st_signon_req.RequestHeader.MsgSeqNum = fn_swap_ulong(st_signon_req.RequestHeader.MsgSeqNum);
  st_signon_req.RequestHeader.SenderSubID = fn_swap_ulong(st_signon_req.RequestHeader.SenderSubID);
  st_signon_req.Username = fn_swap_ulong(st_signon_req.Username);

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_signon_req,sizeof(st_signon_req));
  /* Ver 1.2 End */

  if(fn_write_msg_q(i_snd_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)

  {
   fn_errlog(c_ServiceName, "L31055", LIBMSG, c_err_msg);
   fn_userlog(c_ServiceName,"ERROR- fn_write_msg_q in fn_logon_to_bse ");
   return -1;
  }
     
  fn_userlog(c_ServiceName,"After fn_write_msg_q in fn_logon_to_bse "); 

  return 0;
}

/* ########################################################################################  */

/* int fn_bse_logoff_req_out(struct st_bfo_logoff_rply *ptr_st_logoff_rply, char *c_ServiceName, char *c_err_msg) commented in ver 1.7 */
int fn_bse_logoff_req_out(UserLogoutRequestT *ptr_st_logoff_rply, char *c_ServiceName, char *c_err_msg) /* Added in Ver 1.7 */ 
{
  char c_msg[256];
  char c_pipe_id[3];

  long l_msg_typ;
  long l_user_id;
  char c_brkr_id[6];

  char c_rply_msg[50];
  int i_ch_val;
  int i_trnsctn;
  
  strcpy(c_pipe_id,sql_c_pipe_id);

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"c_pipe_id in fn_bse_logoff_req_out :%s:", c_pipe_id);
  }
  EXEC SQL
  		SELECT EXG_BRKR_ID
 		  INTO  :c_brkr_id
  		FROM  exg_xchng_mstr
  		WHERE exg_xchng_cd = 'BFO';
  if(SQLCODE != 0)
  {
   fn_errlog(c_ServiceName, "L31060", SQLMSG, c_err_msg);
   return -1;
  }
  rtrim(c_brkr_id);

  EXEC SQL
  		SELECT OPM_USER_ID
  		INTO   :l_user_id
  		FROM   OPM_ORD_PIPE_MSTR
  		WHERE  OPM_PIPE_ID =:sql_c_pipe_id;
  if(SQLCODE != 0)
  {
   fn_errlog(c_ServiceName, "L31065", SQLMSG, c_err_msg);
   return -1;
  }

	/* commented in ver 1.7 
  l_msg_typ = fn_swap_long(ptr_st_logoff_rply->l_msg_typ);
  ptr_st_logoff_rply->s_reply_cd = fn_swap_long(ptr_st_logoff_rply->s_reply_cd);
  strcpy(c_rply_msg,ptr_st_logoff_rply->c_reply_msg);
  rtrim(c_rply_msg);
 
  sprintf(c_msg,"User Id:%ld,Msg Typ: %ld|%d-%s",l_user_id,l_msg_typ,ptr_st_logoff_rply->s_reply_cd, ptr_st_logoff_rply->c_reply_msg);
	commented in ver 1.7 */

	sprintf(c_msg,"User Id:%l, Successfully Logged in pipe ",ptr_st_logoff_rply->Username,c_pipe_id);  /* Ver 1.7 */
  i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
   
  if ( i_trnsctn == -1)
  {
   fn_errlog(c_ServiceName,"L31070", LIBMSG, c_err_msg);
   return -1;
  }

  EXEC SQL
    INSERT INTO FTM_FO_TRD_MSG
                ( FTM_XCHNG_CD,
                  FTM_BRKR_CD,
                  FTM_MSG_ID,
                  FTM_MSG,
                  FTM_TM
                 )
                 values
                 (
                  'BFO',
                  :c_brkr_id,
                  'T',
                  :c_msg,
                  sysdate
                 );
  if(SQLCODE != 0)
  {
   fn_errlog(c_ServiceName, "L31075", SQLMSG, c_err_msg);
   fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
   return -1;
  } 
  i_trnsctn = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_trnsctn == -1 )
  {
   fn_errlog(c_ServiceName,"L31080", LIBMSG, c_err_msg);
   fn_userlog(c_ServiceName,"Error -in commitran---1  ");
   return -1;
  } 

  /* if(ptr_st_logoff_rply->s_reply_cd == 0 ) commented in ver 1.7 
  { */ 
    if( DEBUG_MSG_LVL_0 )
    { 
     fn_userlog(c_ServiceName,"######################################"); 
     fn_userlog(c_ServiceName," BSE LOGOFF SUCCESSFUL ");
     fn_userlog(c_ServiceName,"######################################");
    }

    if(DEBUG_MSG_LVL_3)
    {
    fn_userlog(c_ServiceName," Message :%s:",c_msg);
    }

    i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
    if ( i_trnsctn == -1)
    {
     fn_errlog(c_ServiceName,"L31085", LIBMSG, c_err_msg);
     return -1;
    }

    EXEC SQL
    UPDATE  opm_ord_pipe_mstr
    SET     opm_login_stts = 0
    WHERE   opm_pipe_id  = :c_pipe_id
    AND     opm_xchng_cd = 'BFO';

    if(SQLCODE != 0)
    {
     fn_errlog(c_ServiceName, "L31090", SQLMSG, c_err_msg);
     fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
     return -1;
    }

    EXEC SQL
    UPDATE BPH_BFO_PSSWD_HNDLG
    SET    BPH_USR_RGSTRN_FLG ='N'
    WHERE  BPH_PIPE_ID = :c_pipe_id;

    if(SQLCODE != 0)
    {
     fn_errlog(c_ServiceName, "L31095", SQLMSG, c_err_msg);
     fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
     return -1;
    }
   
    i_trnsctn = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
    if ( i_trnsctn == -1 )
    {
     fn_errlog(c_ServiceName,"L31100", LIBMSG, c_err_msg);
     return -1;
    }
 
    return 0;
  /*
  }
	 commented in ver 1.7  
  else
  {
   if( DEBUG_MSG_LVL_0 )
   {
    fn_userlog(c_ServiceName,"##########################################");
    fn_userlog(c_ServiceName," BSE LOGOFF UNSUCCESSFUL");
    fn_userlog(c_ServiceName,"##########################################");
   }   
   return 0; 
  }	***/
}


int fn_bse_rgstrn_req_out( struct st_bfo_rgstrn_reply *ptr_st_rgstrn_rply, 
                           char *c_ServiceName,
                           char *c_err_msg )
{
  char c_msg[256];
  char c_rply_txt[50];

  int i_ch_val;
  int i_trnsctn;
  long l_user_id;
  long l_error_no;
  long l_msg_typ;

  char c_tm_stmp[30]; 
  char c_brkr_id[5+1];
  char c_rgstrn_msg[100] ={'\0'}; 
   
  EXEC SQL
  SELECT to_char(sysdate, 'dd-Mon-yyyy hh24:mi:ss')
  INTO   :c_tm_stmp
  FROM   dual;
  
  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31105", SQLMSG, c_err_msg);
    return -1;
  }
  EXEC SQL
  SELECT  EXG_BRKR_ID
  INTO  :c_brkr_id
  FROM  exg_xchng_mstr
  WHERE exg_xchng_cd = 'BFO';
  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31110", SQLMSG, c_err_msg);
    return -1;
  }
  rtrim(c_brkr_id);
  
  EXEC SQL
  SELECT OPM_USER_ID
  INTO   :l_user_id
  FROM   OPM_ORD_PIPE_MSTR
  WHERE  OPM_PIPE_ID =:sql_c_pipe_id;
  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31115", SQLMSG, c_err_msg);
    return -1;
  }
  i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"L31120", LIBMSG, c_err_msg);
    return -1;
  }
  strcpy(c_rply_txt,ptr_st_rgstrn_rply->c_error_text);
  rtrim(c_rply_txt); 
  l_error_no = fn_swap_long(ptr_st_rgstrn_rply->l_error_no);
  l_msg_typ =  fn_swap_long(ptr_st_rgstrn_rply->l_msg_type);

  if(ptr_st_rgstrn_rply-> l_error_no != 0 )
  {
    if( DEBUG_MSG_LVL_0 ) 
    {
     fn_userlog(c_ServiceName,"#############################################");
     fn_userlog(c_ServiceName," REGISTRATION FAILED ");
     fn_userlog(c_ServiceName,"#############################################");
    }
    sprintf(c_msg,"User Id:%ld,MSG_TYP:%ld|%ld-%s",l_user_id,l_msg_typ,l_error_no,c_rply_txt);
  }
  else if(ptr_st_rgstrn_rply-> l_error_no == 0)
  { 
    if( DEBUG_MSG_LVL_0 ) 
    {
     fn_userlog(c_ServiceName,"#############################################");
     fn_userlog(c_ServiceName," REGISTRATION SUCCESSFUL");
     fn_userlog(c_ServiceName,"#############################################");
    }
    sprintf(c_rgstrn_msg," Registration SuccessFul for pipe id :%s",sql_c_pipe_id);
    rtrim(c_rgstrn_msg);
    sprintf(c_msg,"User id:%ld,Msg_Typ:%ld|%ld-%s",l_user_id,l_msg_typ,l_error_no,c_rgstrn_msg);

    EXEC SQL
    UPDATE   BPH_BFO_PSSWD_HNDLG
    SET      BPH_USR_RGSTRN_FLG ='Y'
    WHERE    BPH_PIPE_ID = :sql_c_pipe_id;
    if(SQLCODE != 0)
    {
     fn_errlog(c_ServiceName, "L31125", SQLMSG, c_err_msg);
     fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
     return -1;
    } 
  }
  if( DEBUG_MSG_LVL_3 )
  { 
    fn_userlog(c_ServiceName," Val of c_brkr_id in fn_bse_rgstrn_req_out: <%s>", c_brkr_id);
    fn_userlog(c_ServiceName," Val of c_msg in fn_bse_rgstrn_req_out: <%s>", c_msg);
    fn_userlog(c_ServiceName," Val of c_tm_stmp in fn_bse_rgstrn_req_out: <%s>", c_tm_stmp);
  }
  EXEC SQL
    INSERT INTO FTM_FO_TRD_MSG
                ( FTM_XCHNG_CD,
                  FTM_BRKR_CD,
                  FTM_MSG_ID,
                  FTM_MSG,
                  FTM_TM
                 )
                 values
                 (
                  'BFO',
                  :c_brkr_id,
                  'T',
                  :c_msg,
                  sysdate
                 );
   
  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31130", SQLMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }      
     
  i_trnsctn = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_trnsctn == -1 )
  {
    fn_errlog(c_ServiceName,"L31135", LIBMSG, c_err_msg);
    return -1;
  }
   return 0;
 }

int fn_do_bse_logon( struct st_bfo_usr_rgstrn_req *ptr_st_exch_msg,
			FILE *ptr_snd_msgs_file,
                       char *c_ServiceName,
                       char *c_err_msg )
{
  int i_ch_val;
	int i_snd_seq;
  long li_business_data_size;
  long li_send_iml_msg_size;

	char c_iml_header;

  struct st_bfo_usr_rgstrn_req st_exch_message;               

  unsigned char digest[16];                         

  i_cond_snd_rcv_cntrl = RGSTRN_REQ_SENT;
  li_business_data_size = sizeof(struct st_bfo_usr_rgstrn_req);
  li_send_iml_msg_size =  li_business_data_size;  

  if(DEBUG_MSG_LVL_5)
  {
	/** Commented in ver 1.7 
  fn_userlog(c_ServiceName,"Before call to fn_writen in snd_thrd .>Val of ptr_st_exch_msg->st_hdr.l_slot_no <%ld>", ptr_st_exch_msg->st_hdr.l_slot_no);
  fn_userlog(c_ServiceName,"Before call to fn_writen in snd_thrd .>Val of ptr_st_exch_msg->st_hdr.l_message_length <%ld>", ptr_st_exch_msg->st_hdr.l_message_length);
ver 1.7  comment end	**/
	fn_userlog(c_ServiceName,"Before call to fn_writen in snd_thrd ptr_st_exch_msg->l_trader_id <%ld> l_member_id <%ld> MsgSeqNum <%d>",ptr_st_exch_msg->l_trader_id,ptr_st_exch_msg->l_member_id,ptr_st_exch_msg->MsgSeqNum);   /* Ver 1.7 */
  }

  i_ch_val = fn_writen (i_iml_sck_id,
                       (void *)ptr_st_exch_msg,
                        li_send_iml_msg_size,
                        c_ServiceName,
                        c_err_msg);

  if ( i_ch_val == -1 )
  {
     fn_errlog(c_ServiceName,"L31140", LIBMSG, c_err_msg);
     return -1;
  }

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"After call to fn_writen in fn_do_bse_logon .>Val of i_iml_sck_id <%d> and c_err_msg <%s> ", i_iml_sck_id, c_err_msg);
  }
	
  if ( i_msg_dump_flg == LOG_DUMP )
  {
    /**call function to write data to file **/
		c_iml_header = 'Y';

		fn_bfprint_log(	ptr_snd_msgs_file,
										c_iml_header,
										(void *)ptr_st_exch_msg,
										c_ServiceName,
										c_err_msg);

  if(DEBUG_MSG_LVL_5)
  {
   fn_userlog(c_ServiceName,"After call to fn_bfprint_log in fn_do_bse_logon .>Val of c_err_msg <%s> ",  c_err_msg);
  }

  }

	/** Newly added for receive thread continuity --start */


  while ( ( i_cond_snd_rcv_cntrl != RGSTRN_RESP_RCVD) &&
          ( i_cond_snd_rcv_cntrl != RCV_ERR ) )
  {
		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Inside while of not equat to RGSTRN_RESP_RCVD and RCV_ERR");
		}

    i_ch_val = fn_thrd_cond_wait ( &cond_snd_rcv_cntrl,
                                   &mut_snd_rcv_cntrl,
                                   c_ServiceName,
                                   c_err_msg);
    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName,"L31145", LIBMSG, c_err_msg);
      return -1;
    }
  }

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"The value of i_cond_snd_rcv_cntrl II is :%d:",i_cond_snd_rcv_cntrl);
	}

  if ( i_cond_snd_rcv_cntrl == RCV_ERR )
  {
    fn_errlog(c_ServiceName,"L31150", "Error in recieve thread", c_err_msg);
    pthread_mutex_unlock (&mut_snd_rcv_cntrl );
    return -1;
  }

  pthread_mutex_unlock (&mut_snd_rcv_cntrl );

	/** Newly added for receive thgread continuity --End */
  /*************************************************/

  i_ch_val = fn_get_reset_seq(sql_c_pipe_id,
						    sql_c_nxt_trd_date.arr, 
								GET_PLACED_SEQ,
								&i_snd_seq,
				 			c_ServiceName,
                         			c_err_msg);
		if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName,"L31155", LIBMSG, c_err_msg);
      return -1;
    }

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"The sequence number got in fn_do_bse_logon is:%d:",i_snd_seq);
	}

  if ( i_cond_snd_rcv_cntrl == RCV_ERR )
  {
    fn_errlog(c_ServiceName,"L31160", "Error in recieve thread", c_err_msg);
    return -1;
  }
  return 0;
}

int fn_upd_chngd_passwd( char *c_ServiceName,
                         char *c_err_msg )
{

  int i_ch_val;
  int i_tran_id;
  int i_ret_val = 0;

  char c_curnt_passwd[LEN_PASS_WD];
  char c_new_passwd[LEN_PASS_WD];
  char c_pipe_id[2+1];
  char c_command[250]; 

  MEMSET(c_curnt_passwd);
  MEMSET(c_new_passwd);
  MEMSET(c_pipe_id);
  MEMSET(c_command);

  EXEC SQL
    SELECT BPH_NEW_PASSWD
    INTO   :c_new_passwd
    FROM   BPH_BFO_PSSWD_HNDLG
    WHERE  BPH_PIPE_ID =:sql_c_pipe_id;
  if ( SQLCODE != 0 )
  {
   fn_userlog(c_ServiceName," Error in getting new passwd");
   fn_errlog(c_ServiceName, "L31165", SQLMSG, c_err_msg);
   return(-1);
  }

  rtrim(c_new_passwd);
  fn_userlog(c_ServiceName,"Changed password is :%s:",c_new_passwd);

  i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
  if ( i_tran_id == -1 )
  {
   fn_userlog(c_ServiceName,"Failed in tpcommit ");
   fn_errlog(c_ServiceName, "L31170", LIBMSG, c_err_msg );
   return(-1); 
  } 

	if(DEBUG_MSG_LVL_3)
	{
	 fn_userlog(c_ServiceName,"sql_c_pipe_id Is :%s:",sql_c_pipe_id);
	}
  
  EXEC SQL
     UPDATE BPH_BFO_PSSWD_HNDLG
     SET    BPH_CURNT_PASSWD = :c_new_passwd,
            BPH_LST_PSWD_CHG_DT = SYSDATE
     WHERE  BPH_PIPE_ID    = :sql_c_pipe_id;

  if ( SQLCODE != 0 )
  {
   fn_userlog(c_ServiceName," Error in update current passwd <%ld>", SQLCODE);
   fn_aborttran(c_ServiceName, i_tran_id, c_err_msg);
   fn_errlog(c_ServiceName, "L31175", SQLMSG, c_err_msg);
   return(-1);
  }

  i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
  if ( i_ch_val == -1 )
  {
   fn_userlog(c_ServiceName,"Failed in tpcommit ");
   fn_errlog(c_ServiceName, "L31180", LIBMSG, c_err_msg );
   return(-1);
  }
  if( DEBUG_MSG_LVL_0 )
  {
   fn_userlog(c_ServiceName,"###############################################");
   fn_userlog(c_ServiceName," NEW PASSWORD SUCCESSFULLY UPDATED ");
   fn_userlog(c_ServiceName,"###############################################");
  }
  /***************************************************************************/

  fn_userlog(c_ServiceName,"Val of sql_c_pipe_id in sys command Is :%s:",sql_c_pipe_id);
  sprintf( c_command, "ksh send_passwd_chng_mail.sh %s %s",sql_c_pipe_id,c_new_passwd);
  fn_userlog(c_ServiceName,"Mail send command for password change:%s:", c_command );
  system(c_command);

  /***************************************************************************/

  return(0);

}
/***********************************************************************************/

 int fn_chng_exp_passwd_req(char * c_pipe_id,long l_rqst_typ,char *c_ServiceName,char *c_err_msg  )
 {
  int   i_cnt;
  int   i_login_stts;
  int   i_snd_seq;
  int   i_ch_val;
  int   i_tran_id;
  int   i_seq;
  int   i_nxt_seq;
  int   i_ret_val;
  int i_num1;
  int i_num2;

  int i_err[10];
  int i_ferr[10];
  
  char c_passwd_flg;
  char  c_tmp;
  
  long int li_ip_usr_id;
  long int li_user_id;
  
  char c_encrptd_passwd[LEN_PASS_WD]={'\0'};			/** Ver 1.4 Initialization done **/
  char c_decrptd_passwd[LEN_PASS_WD]={'\0'};      /** Ver 1.4 Initialization done **/
  char c_curnt_passwd[LEN_PASS_WD]={'\0'};      	/** Ver 1.4 Initialization done **/
  char c_new_passwd[LEN_PASS_WD]={'\0'};      		/** Ver 1.4 Initialization done **/
  char c_num[2]={'\0'};      											/** Ver 1.4 Initialization done **/
  char c_nxt_seq[3]={'\0'};												/** Ver 1.4 **/
  char c_new_pass_wd_entrd;
  char c_opn_ordr_dtls_flg;

  /* struct st_bfo_upd_passwd_req st_upd_passwd_req; commented in Ver 1.7 */
	UserPasswordChangeRequestT st_upd_passwd_req;       /* Ver 1.7 */
  struct st_send_rqst_data st_q_esr_clnt;

  MEMSET(st_upd_passwd_req);
  MEMSET(st_q_esr_clnt);

  MEMSET(c_curnt_passwd);
  MEMSET(c_new_passwd); 
  MEMSET(c_encrptd_passwd);


  /**** Get current exchange password  ****/
  EXEC SQL
    SELECT  OPM_LOGIN_STTS
      INTO  :i_login_stts
      FROM  OPM_ORD_PIPE_MSTR
      WHERE OPM_PIPE_ID = :c_pipe_id
      AND   OPM_XCHNG_CD = 'BFO';
  if ( SQLCODE != 0 )
  {
   fn_userlog(c_ServiceName," Error in getting Login stts");
   fn_errlog(c_ServiceName, "L31185", SQLMSG, c_err_msg);
   return(-1);

  }
  
  EXEC SQL
    SELECT BPH_CURNT_PASSWD 
    INTO   :c_curnt_passwd
    FROM   BPH_BFO_PSSWD_HNDLG
    WHERE  BPH_PIPE_ID = :c_pipe_id;
  if ( SQLCODE != 0 )
  {
   fn_userlog(c_ServiceName," Error in getting current password");
   fn_errlog(c_ServiceName, "L31190", SQLMSG, c_err_msg);
   return(-1);
  }
  
  rtrim(c_curnt_passwd);

  fn_userlog(c_ServiceName," The decripted current password :%s:",c_curnt_passwd);

  if( l_rqst_typ == MANDATORY_PASSWD_CHNG )
  {
    /****** Auto Creation of next new password  *****/
       c_passwd_flg = 'M'; 
       c_num[0] = c_curnt_passwd[6];
       c_num[1] = c_curnt_passwd[7];
       for(i_cnt=0;i_cnt<6;i_cnt++)
       {
        c_new_passwd[i_cnt] = c_curnt_passwd[i_cnt];
       }

       i_seq = atoi(c_num);
       if(i_seq < 10 )
       {
        i_num1 = c_num[0]-48;
        i_num2 = c_num[1]-48;
        i_num2 = i_num2+1;
        if(i_num2 == 10 )
        {
         i_num2 = 0;
         i_num1 = i_num1+1;
        }
        sprintf(c_new_passwd,"%s%d%d",c_new_passwd,i_num1,i_num2);       
 
       }
       else if( i_seq == 99 )
       {
        i_nxt_seq = 00;
				strcpy(c_nxt_seq,"00");									/** Ver 1.4 **/
        /*sprintf(c_new_passwd,"%s%d",c_new_passwd,i_nxt_seq); ***Ver 1.4 comment **/ 
        sprintf(c_new_passwd,"%s%s",c_new_passwd,c_nxt_seq);	 /**Ver 1.4 **/ 
       }
       else
       {
        i_nxt_seq = i_seq + 1;
        sprintf(c_new_passwd,"%s%d",c_new_passwd,i_nxt_seq);
       }
        
       rtrim(c_new_passwd);
       fn_userlog(c_ServiceName," Next new Password is :%s:",c_new_passwd);
     /******************************************/        
  }

  rtrim(c_new_passwd);

  fn_userlog(c_ServiceName,"New Password  :%s:",c_new_passwd);

  /**** Update new password requested in BPH table ***/
  
  i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
  if ( i_tran_id == -1 )
  {
    fn_errlog(c_ServiceName, "L31195", LIBMSG, c_err_msg );
    return -1;
  }

  EXEC SQL 
     UPDATE BPH_BFO_PSSWD_HNDLG
     SET    BPH_NEW_PASSWD = :c_new_passwd,
            BPH_PASSWD_FLG = :c_passwd_flg 
     WHERE  BPH_PIPE_ID    = :c_pipe_id;

  if ( SQLCODE != 0 )
  {
    fn_userlog(c_ServiceName," Error in Update of new password");
    fn_aborttran(c_ServiceName, i_tran_id, c_err_msg);
    fn_errlog(c_ServiceName, "L31200", SQLMSG, c_err_msg);
    return(-1);
  } 

  i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName, "L31205", LIBMSG, c_err_msg );
    return -1;
  }   

  /*** Coping the required data for change password ***/

	/** Commented in ver 1.7 **

  if(l_rqst_typ == MANDATORY_PASSWD_CHNG )
  {
   fn_userlog(c_ServiceName," Inside Mandatory Password Change ");
 
   st_upd_passwd_req.l_msg_typ = htonl(MANDATORY_PASSWD_CHNG);
 
   st_q_esr_clnt.li_msg_type = MANDATORY_PASSWD_CHNG;
  }
  
  strcpy((char *)st_upd_passwd_req.c_old_pwd,(char *)c_curnt_passwd); 
  strcpy((char *)st_upd_passwd_req.c_new_pwd,(char *)c_new_passwd); 
  st_upd_passwd_req.l_msg_tag = htonl(UPD_PASSWD_TAG); 
  st_upd_passwd_req.st_hdr.l_slot_no = htonl(REQ_SLOT_NO); 
  st_upd_passwd_req.st_hdr.l_message_length=htonl(sizeof(st_upd_passwd_req) - sizeof(st_upd_passwd_req.st_hdr)); 
  
  st_upd_passwd_req.l_msg_typ = fn_swap_long(st_upd_passwd_req.l_msg_typ);
  st_upd_passwd_req.l_msg_tag = fn_swap_long( st_upd_passwd_req.l_msg_tag );
  st_upd_passwd_req.st_hdr.l_slot_no = fn_swap_long(st_upd_passwd_req.st_hdr.l_slot_no);
  st_upd_passwd_req.st_hdr.l_message_length = fn_swap_long(st_upd_passwd_req.st_hdr.l_message_length);

  memcpy(&(st_q_esr_clnt.st_bsnd_rqst_data),&st_upd_passwd_req,sizeof(st_upd_passwd_req));
  
  **** End of Coping data ****

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"Password Change Data copied in structure");
   fn_userlog(c_ServiceName,"Old Password :%s:",c_curnt_passwd);
   fn_userlog(c_ServiceName,"New Requested Passwd :%s:",c_new_passwd);
   fn_userlog(c_ServiceName,"Message tag :%ld:",st_upd_passwd_req.l_msg_tag);
   fn_userlog(c_ServiceName,"Slot No :%ld:",st_upd_passwd_req.st_hdr.l_slot_no);
   fn_userlog(c_ServiceName,"Message Lenngth :%ld:",st_upd_passwd_req.st_hdr.l_message_length);
  } 
	** Commented in ver 1.7 **/

	/* Added in ver 1.7 -Starts */
  st_upd_passwd_req.MessageHeaderIn.BodyLen = htonl(sizeof(UserPasswordChangeRequestT));
  st_upd_passwd_req.MessageHeaderIn.TemplateID = htons(TID_USER_PASSWORD_CHANGE_REQUEST);
  strcpy(st_upd_passwd_req.MessageHeaderIn.NetworkMsgID,"");
  strcpy(st_upd_passwd_req.MessageHeaderIn.Pad2,"");

  strcpy((char *)st_upd_passwd_req.Password,(char *)c_curnt_passwd);
  strcpy((char *)st_upd_passwd_req.NewPassword,(char *)c_new_passwd);
  strcpy((char *)st_upd_passwd_req.Pad4,NO_VALUE_STR);

  st_upd_passwd_req.MessageHeaderIn.BodyLen = fn_swap_ulong(st_upd_passwd_req.MessageHeaderIn.BodyLen);
  st_upd_passwd_req.MessageHeaderIn.TemplateID = fn_swap_ushort(st_upd_passwd_req.MessageHeaderIn.TemplateID);
  st_upd_passwd_req.RequestHeader.MsgSeqNum = fn_swap_ulong(st_upd_passwd_req.RequestHeader.MsgSeqNum);
  st_upd_passwd_req.RequestHeader.SenderSubID = fn_swap_ulong(st_upd_passwd_req.RequestHeader.SenderSubID);
  st_upd_passwd_req.Username = fn_swap_ulong(st_upd_passwd_req.Username);

	 if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"Password Change Data copied in structure");
   fn_userlog(c_ServiceName,"Username :%s:",st_upd_passwd_req.Username);
   fn_userlog(c_ServiceName,"Old Password :%s:",st_upd_passwd_req.Password);
   fn_userlog(c_ServiceName,"New Requested Passwd :%s:",st_upd_passwd_req.NewPassword);
   fn_userlog(c_ServiceName,"Message TemplateID :%ld:",st_upd_passwd_req.MessageHeaderIn.TemplateID);
   fn_userlog(c_ServiceName,"SenderSubID :%ld:",st_upd_passwd_req.RequestHeader.SenderSubID);
   fn_userlog(c_ServiceName,"Message Lenngth :%ld:",st_upd_passwd_req.MessageHeaderIn.BodyLen);
  }

  /* ver 1.7 End */

 
  if(DEBUG_MSG_LVL_5)
  {
   fn_userlog(c_ServiceName," Calling fn_write_msg_q");
  }
 
  if(fn_write_msg_q(i_snd_qid,
                    (void *)&st_q_esr_clnt,
                    sizeof(st_q_esr_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)

  {
    fn_errlog(c_ServiceName, "L31210", LIBMSG, c_err_msg);
    return -1;
  }  

  fn_userlog(c_ServiceName," After call to fn_write_msg_q");
  return 0;

 }

/***********************************************************************************/

int fn_snd_opn_intrst_rqst(char *c_ServiceName, char *c_err_msg)
{  
 struct st_opn_intrst_rqst st_opn_intrst_rqst_data;
 struct st_send_rqst_data st_q_con_clnt;
 MEMSET(st_q_con_clnt);
 MEMSET(st_opn_intrst_rqst_data);

 st_opn_intrst_rqst_data.l_msg_typ = OPN_INTRST_RQST;
 st_opn_intrst_rqst_data.l_msg_tag = 1;
 st_opn_intrst_rqst_data.st_hdr.l_slot_no = REQ_SLOT_NO;
 st_opn_intrst_rqst_data.st_hdr.l_message_length = sizeof(st_opn_intrst_rqst_data) - sizeof(st_opn_intrst_rqst_data.st_hdr);
 
 st_opn_intrst_rqst_data.l_msg_typ = htonl(st_opn_intrst_rqst_data.l_msg_typ);
 st_opn_intrst_rqst_data.l_msg_tag = htonl(st_opn_intrst_rqst_data.l_msg_tag);
 st_opn_intrst_rqst_data.st_hdr.l_slot_no = htonl(st_opn_intrst_rqst_data.st_hdr.l_slot_no);
 st_opn_intrst_rqst_data.st_hdr.l_message_length = htonl(st_opn_intrst_rqst_data.st_hdr.l_message_length);

 st_opn_intrst_rqst_data.l_msg_typ = fn_swap_long(st_opn_intrst_rqst_data.l_msg_typ);
 st_opn_intrst_rqst_data.l_msg_tag = fn_swap_long(st_opn_intrst_rqst_data.l_msg_tag);
 st_opn_intrst_rqst_data.l_rsrvd_fld	=	0;
 st_opn_intrst_rqst_data.st_hdr.l_slot_no = fn_swap_long(st_opn_intrst_rqst_data.st_hdr.l_slot_no); 
 st_opn_intrst_rqst_data.st_hdr.l_message_length = fn_swap_long(st_opn_intrst_rqst_data.st_hdr.l_message_length);  
 
 st_q_con_clnt.li_msg_type = OPN_INTRST_RQST;
 memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_opn_intrst_rqst_data,sizeof(st_opn_intrst_rqst_data));
 
 if(fn_write_msg_q(i_snd_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)

  {
   fn_errlog(c_ServiceName, "L31215", LIBMSG, c_err_msg);
   fn_userlog(c_ServiceName,"ERROR- fn_write_msg_q in fn_snd_opn_intrst_rqst");
   return -1;
  }
  if( DEBUG_MSG_LVL_5)
  {
   fn_userlog(c_ServiceName," After Convertion "); 
   fn_userlog(c_ServiceName," Msg Typ :%ld:",st_opn_intrst_rqst_data.l_msg_typ);
   fn_userlog(c_ServiceName," Msg Tag :%ld:",st_opn_intrst_rqst_data.l_msg_tag);
   fn_userlog(c_ServiceName," REserved Field :%ld:",st_opn_intrst_rqst_data.l_rsrvd_fld);
   fn_userlog(c_ServiceName," Slot Number    :%ld:",st_opn_intrst_rqst_data.st_hdr.l_slot_no);
   fn_userlog(c_ServiceName," Msg Length  :%ld:",st_opn_intrst_rqst_data.st_hdr.l_message_length);
  }

  fn_userlog(c_ServiceName,"After fn_write_msg_q in fn_snd_opn_intrst_rqst");

  return 0;
}


int fn_upd_opn_inrst(struct st_ums_opn_intrst *ptr_st_opn_instr_dtls,
                         char *c_ServiceName,
                         char *c_errmsg)
{
  int i_ch_val;
  int i_trnsctn;
  long l_count;
  int i_hrs,i_min,i_secnds,i_mlscnds;
  char c_stock[21];
  char c_time[20];
  char c_mltime[20];
   
  long l_token_id=0;
  long l_oi_val; 
  double d_oi =0.0;
  /* commented in ver 1.7 EXEC SQL INCLUDE  "table/fum_fo_undrlyng_mstr.h";
  EXEC SQL INCLUDE  "table/ftq_fo_trd_qt.h";   */  
  EXEC SQL INCLUDE  "table/BUM_BFO_UNDRLYNG_MSTR.h";  /* Ver 1.7 */
  EXEC SQL INCLUDE  "table/btq_bfo_trd_qt.h";         /* Ver 1.7 */
 

  ptr_st_opn_instr_dtls->l_no_of_recrd = ntohl(ptr_st_opn_instr_dtls->l_no_of_recrd);
  ptr_st_opn_instr_dtls->l_no_of_recrd = fn_swap_long(ptr_st_opn_instr_dtls->l_no_of_recrd);

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"Inside Function fn_upd_opn_interest ");
   fn_userlog(c_ServiceName,"No Of records in message :%ld:", ptr_st_opn_instr_dtls->l_no_of_recrd); 
  }
  
  /******* Ver 1.1 Starts Here *******/

  i_trnsctn = fn_begintran( c_ServiceName, c_errmsg );
  if ( i_trnsctn == -1 )
  {
   fn_errlog(c_ServiceName,"L31220",SQLMSG ,c_errmsg);
   return -1;
  }

  /******* Ver 1.1 Ends here ********/   
  
  for(l_count =0 ; l_count <ptr_st_opn_instr_dtls->l_no_of_recrd ; l_count++)
  {
    ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_contrct_id = ntohl(ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_contrct_id);
    ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_contrct_id = fn_swap_long(ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_contrct_id);
    ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_qty = ntohl(ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_qty);  
    ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_qty = fn_swap_long(ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_qty);
    ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_val = fn_swap_ulnglng(ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_val);
    ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_chng = ntohl(ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_chng);
    ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_chng = fn_swap_long(ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_chng);

   if(DEBUG_MSG_LVL_3)
   {
      fn_userlog(c_ServiceName,"Processing record %ld of %ld ",l_count,ptr_st_opn_instr_dtls->l_no_of_recrd);
      fn_userlog(c_ServiceName,"Instrument id :%ld: ",ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_contrct_id);
      fn_userlog(c_ServiceName,"Open Intrest qty :%ld: ",ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_qty);
      fn_userlog(c_ServiceName,"Open Intrest VAL :%lld: ",ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_val);
      fn_userlog(c_ServiceName,"Open Intrest Change :%ld: ",ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_chng);
   } 

   l_token_id = ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_contrct_id;

   if(DEBUG_MSG_LVL_3)
   { 
    /* commented in ver 1.7 fn_userlog (c_ServiceName,"Before FUM Selection"); */
    fn_userlog (c_ServiceName,"Before BUM Selection");      /** Ver 1.7 **/
   }
  /* commented in ver 1.7 MEMSET(sql_fum_undrlyng); ** Ver 1.2 **
  MEMSET(sql_ftq_expry_dt); ** Ver 1.2 **
  sql_ftq_strk_prc = 0;     ** Ver 1.2 **/

  MEMSET(sql_bum_undrlyng); /** Ver 1.7 **/
  MEMSET(sql_btq_expry_dt); /** Ver 1.7 **/
  sql_btq_strk_prc = 0;     /** Ver 1.7 **/

   /*** Commented In Ver 1.6  ***	

   EXEC SQL
      SELECT FCM_UNDRLYNG,
             FCM_PRDCT_TYP,
						 FCM_OPT_TYP,
						 FCM_EXPRY_DT,
						 FCM_STRK_PRC
      INTO :sql_bum_undrlyng,
           :sql_bum_prdct_typ,
					 :sql_btq_opt_typ,
					 :sql_btq_expry_dt,
					 :sql_btq_strk_prc
      FROM FCM_FO_CNTRCT_MSTR
      WHERE FCM_XCHNG_CD ='BFO'
      AND FCM_TOKEN_NO =:l_token_id;
	
		*** Ver 1.6 Comment Ends ***/

		/*** Ver 1.6 Starts ***/

		 EXEC SQL
      SELECT BTQ_UNDRLYNG,
             BTQ_PRDCT_TYP,
             BTQ_OPT_TYP,
             BTQ_EXPRY_DT,
             BTQ_STRK_PRC
      INTO :sql_bum_undrlyng,
           :sql_bum_prdct_typ,
           :sql_btq_opt_typ,
           :sql_btq_expry_dt,
           :sql_btq_strk_prc
      FROM BTQ_BFO_TRD_QT             /* changed FTQ_FO_TRD_QT to BTQ_BFO_TRD_QT in ver 1.7 */
      WHERE BTQ_XCHNG_CD ='BFO'
      AND BTQ_TOKEN_NO =:l_token_id;

    /*** Ver 1.6 Ends	***/

   if(SQLCODE != 0 && SQLCODE != NO_DATA_FOUND)
   {
    fn_errlog(c_ServiceName,"S31005",SQLMSG,c_errmsg);
    continue;
   }
   else if(SQLCODE == NO_DATA_FOUND)
   {
    fn_userlog(c_ServiceName,"inside no data found case for FUM in OI");
    continue;
   }

   if(DEBUG_MSG_LVL_3 )
   {
    fn_userlog (c_ServiceName,"After BUM Selection");       /* changed FUM to BUM and ftq to btq in ver 1.7 */
    fn_userlog (c_ServiceName,"Underlying  :%s:",sql_bum_undrlyng);
    fn_userlog (c_ServiceName,"Prdct type  :%c:",sql_bum_prdct_typ);  /** Ver 1.2 ** changed from %s to %c */
    fn_userlog (c_ServiceName,"Option type  :%c:",sql_btq_opt_typ);   /** Ver 1.2 ** changed from %s to %c */
    fn_userlog (c_ServiceName,"Expry Date  :%s:",sql_btq_expry_dt.arr); /** Ver 1.2 ** changed to .arr */
    fn_userlog (c_ServiceName,"Strike Price  :%ld:",sql_btq_strk_prc);  /** Ver 1.2 ** changed from %s to %ld */
   }
   
   /**** Ver 1.1 , Commented  ****************    
   i_trnsctn = fn_begintran( c_ServiceName, c_errmsg );
   if ( i_trnsctn == -1 )
   {
    fn_errlog(c_ServiceName,"L31225",SQLMSG ,c_errmsg);
    return -1;
   }
   ************* Ver 1.1 *********************/
   /*** Ver 1.2 starts ***/ /* changed FUM to BUM and ftq to btq in ver 1.7 */
   sql_bum_oi =0;          
   sql_bum_volume =0;  
   sql_btq_crnt_oi =0;
   sql_btq_chng_oi =0;
   /*** Ver 1.2 Ends here **/
   /*************************** Ver 1.3 *********************
   sql_bum_oi = (double)ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_val;
   sql_bum_volume =(double)ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_qty;
   sql_btq_crnt_oi=(double)ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_val;
   ****************** Ver 1.3 ******************************/

   /* changed FUM to BUM and ftq to btq in ver 1.7 */
   sql_btq_chng_oi=(double)ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_chng;  
   sql_bum_oi = (double)ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_qty;   /** Ver 1.3**/
   sql_btq_crnt_oi=(double)ptr_st_opn_instr_dtls->st_ums_opn_intrst_data[l_count].l_opn_intrst_qty;   /** Ver 1.3**/

   if( DEBUG_MSG_LVL_0 )
   {
    fn_userlog(c_ServiceName,"OI Before Updating FUM:%f:",sql_bum_oi);
   }

   EXEC SQL
      UPDATE BTQ_BFO_TRD_QT           /* changed FTQ_FO_TRD_QT to BTQ_BFO_TRD_QT in ver 1.7 */
      SET BTQ_CRNT_OI= round(:sql_btq_crnt_oi,2),
          BTQ_CHNG_OI= round(:sql_btq_chng_oi,2)
      WHERE BTQ_UNDRLYNG = :sql_bum_undrlyng 
      AND BTQ_XCHNG_CD   = 'BFO'
      AND BTQ_EXPRY_DT   = :sql_btq_expry_dt
      AND BTQ_PRDCT_TYP  = :sql_bum_prdct_typ
      AND BTQ_OPT_TYP    = :sql_btq_opt_typ
      AND BTQ_STRK_PRC   = :sql_btq_strk_prc;

    if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
    {
      fn_errlog ( c_ServiceName, "S31010", SQLMSG,c_errmsg );
     /** fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg ); **** Ver 1.1 ****/
      continue;
    }

    if(SQLCODE == NO_DATA_FOUND )
    {
      fn_userlog(c_ServiceName ,"No data found for stock code :%ld:",l_token_id);
    }
 
   EXEC SQL
     UPDATE fum_fo_undrlyng_mstr 
     SET fum_oi = round(:sql_bum_oi,2)
			/**	 fum_volume = round(:sql_bum_volume,2) *** Ver 1.3 ***/
     WHERE fum_xchng_cd = 'BFO' 
     AND fum_undrlyng = :sql_bum_undrlyng 
     AND  fum_PRDCT_TYP = :sql_bum_prdct_typ;


   if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
   {
    fn_errlog ( c_ServiceName, "S31015", SQLMSG,c_errmsg );
    /**** fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg ); ****** Ver 1.1 ******/
    continue;
   }

   if(SQLCODE == NO_DATA_FOUND )
   {
    fn_userlog(c_ServiceName ,"No data found for stock code :%ld:",l_token_id);
    /*** fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg ); ***** Ver 1.1 ******/
    continue;
   }
   /***** Ver 1.1, commented *************
   if( fn_committran( c_ServiceName, i_trnsctn, c_errmsg ) == -1 )
   {
    fn_errlog ( c_ServiceName, "S31020", SQLMSG,c_errmsg );
    return -1;
   }
   ***************************************/

   if( DEBUG_MSG_LVL_3 )
   {
    fn_userlog(c_ServiceName,"After Updating FUM and FTQ ");
   }

  }
  
  /********* Ver 1.1 starts here **********/  
  if( fn_committran( c_ServiceName, i_trnsctn, c_errmsg ) == -1 )
  {
    fn_errlog ( c_ServiceName, "S31020", SQLMSG,c_errmsg );
    return -1;
  }
  /********* Ver 1.1 Ends here ***********/
  return 0;
}  

/** Ver 1.8 starts **/

int fn_retrnsmt_rqst ( char *c_ord_lst_seq,int i_partition_id, char *c_ServiceName, char *c_err_msg  )
{
 char c_ord_lst_seq_hexa[32+1] ="\0";
 char *c_data1;
 int i;

 RetransmitMEMessageRequestT st_dwnld;
 struct st_send_rqst_data st_q_con_clnt;

 MEMSET(st_dwnld);
 MEMSET(st_q_con_clnt);

 EXEC SQL
    SELECT (SUBSTR(OPM_PIPE_ID,2,1) || '0'),                                              /* <pipe id[1] || '0' <Sequence> */
           TO_NUMBER(TRIM(EXG_BRKR_ID)||'00'|| LPAD(OPM_USER_ID,3,'0'))                   /* <Brokerd id>00<userid> */
    INTO   :st_dwnld.RequestHeader.MsgSeqNum,
           :st_dwnld.SubscriptionScope                                                    /* Not used */
    FROM   OPM_ORD_PIPE_MSTR,
           EXG_XCHNG_MSTR
    WHERE  OPM_XCHNG_CD = :sql_c_xchng_cd
    AND    OPM_XCHNG_CD = EXG_XCHNG_CD
    AND    OPM_PIPE_ID  = :sql_c_pipe_id;

  if ( SQLCODE != 0)
  {
    fn_userlog(c_ServiceName,"Error while selecting Pipe specific and Exchange specific details.");
    fn_errlog(c_ServiceName, "L31230", LIBMSG, c_err_msg);
    return ( -1 );
  }

 if( DEBUG_MSG_LVL_0 )
 {
  fn_userlog(c_ServiceName," Inside ORD fn_retrnsmt_rqst");
 }

  fn_userlog(c_ServiceName,"i_partition_id :%d:",i_partition_id);

  st_dwnld.MessageHeaderIn.BodyLen = htonl(sizeof(RetransmitMEMessageRequestT));
  st_dwnld.MessageHeaderIn.TemplateID = htons(TID_RETRANSMIT_ME_MESSAGE_REQUEST);
  strcpy(st_dwnld.MessageHeaderIn.NetworkMsgID,"");
  strcpy(st_dwnld.MessageHeaderIn.Pad2,"");
  st_dwnld.RequestHeader.SenderSubID = NO_VALUE_UINT;
  st_dwnld.SubscriptionScope = NO_VALUE_UINT;
  st_dwnld.RefApplID = 4;               /* 4 : Session data */
  st_dwnld.PartitionID = i_partition_id;

	if(DEBUG_MSG_LVL_5)
  {
     c_data1 = c_ord_lst_seq_hexa;
     for(i=0;i<16;i++)
     {
        c_data1 += sprintf(c_data1, "%.2X", c_ord_lst_seq[i]);
     }
     fn_userlog(c_ServiceName,"c_ord_lst_seq_hexa:%s:",c_ord_lst_seq_hexa);
	}

  /**memcpy(st_dwnld.ApplBegMsgID,ptr_dwnld_resp->ApplEndMsgID,LEN_APPL_END_MSGID);*******/
  memcpy(st_dwnld.ApplBegMsgID, c_ord_lst_seq,sizeof(st_dwnld.ApplBegMsgID));
  memset(st_dwnld.ApplEndMsgID,0,16);
  st_dwnld.Pad1[0] = '\0';

  st_dwnld.MessageHeaderIn.BodyLen = fn_swap_ulong(st_dwnld.MessageHeaderIn.BodyLen);
  st_dwnld.MessageHeaderIn.TemplateID = fn_swap_ushort(st_dwnld.MessageHeaderIn.TemplateID);
  st_dwnld.RequestHeader.MsgSeqNum = fn_swap_ulong(st_dwnld.RequestHeader.MsgSeqNum);
  st_dwnld.RequestHeader.SenderSubID = fn_swap_ulong(st_dwnld.RequestHeader.SenderSubID);
  st_dwnld.SubscriptionScope = fn_swap_ulong(st_dwnld.SubscriptionScope);
  st_dwnld.PartitionID = fn_swap_ushort(st_dwnld.PartitionID);
  
  st_q_con_clnt.li_msg_type = PRSNL_ORDR_DWNLD;

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_dwnld,sizeof(st_dwnld));

  if(DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName,"Before call to fn_write_msg_q --Val of i_snd_qid <%d>",i_snd_qid);
  }

  if(fn_write_msg_q(i_snd_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)
  {
    fn_errlog(c_ServiceName, "L31235", LIBMSG, c_err_msg);
    return -1;
  }

  fn_userlog(c_ServiceName," After ORD fn_write_msg_q");
return 0;
}

int fn_retrnsmt_trd_rqst ( uint64_t ll_ApplLastSeqNum, int i_partition_id, char *c_ServiceName, char *c_err_msg  )
{
 uint64_t ll_trd_lst_seq;

 RetransmitRequestT st_trddwnld;
 struct st_send_rqst_data st_q_con_clnt;

 MEMSET(st_trddwnld);
 MEMSET(st_q_con_clnt);

 EXEC SQL
    SELECT (SUBSTR(OPM_PIPE_ID,2,1) || '0'),                                              /* <pipe id[1] || '0' <Sequence> */
           TO_NUMBER(TRIM(EXG_BRKR_ID)||'00'|| LPAD(OPM_USER_ID,3,'0'))                   /* <Brokerd id>00<userid> */
    INTO   :st_trddwnld.RequestHeader.MsgSeqNum,
           :st_trddwnld.SubscriptionScope                                                    /* Not used */
    FROM   OPM_ORD_PIPE_MSTR,
           EXG_XCHNG_MSTR
    WHERE  OPM_XCHNG_CD = :sql_c_xchng_cd
    AND    OPM_XCHNG_CD = EXG_XCHNG_CD
    AND    OPM_PIPE_ID  = :sql_c_pipe_id;

  if ( SQLCODE != 0)
  {
    fn_userlog(c_ServiceName,"Error while selecting Pipe specific and Exchange specific details.");
    fn_errlog(c_ServiceName, "L31240", LIBMSG, c_err_msg);
    return ( -1 );
  }

  ll_trd_lst_seq = 0;

 if( DEBUG_MSG_LVL_0 )
 {
  fn_userlog(c_ServiceName," Inside fn_retrnsmt_rqst");
 }

  st_trddwnld.MessageHeaderIn.BodyLen = htonl(sizeof(RetransmitRequestT));
  st_trddwnld.MessageHeaderIn.TemplateID = htons(TID_RETRANSMIT_REQUEST);
  strcpy(st_trddwnld.MessageHeaderIn.NetworkMsgID,"");
  strcpy(st_trddwnld.MessageHeaderIn.Pad2,"");
  st_trddwnld.RequestHeader.SenderSubID = NO_VALUE_UINT;
  st_trddwnld.SubscriptionScope = NO_VALUE_UINT;
  st_trddwnld.RefApplID = 1;               /* 1 : News Broadcast  */
  st_trddwnld.PartitionID = i_partition_id;
  /*ll_trd_lst_seq = ptr_trd_dwnld_resp->ApplEndSeqNum;*/
  if( DEBUG_MSG_LVL_5 )
  {
   fn_userlog(c_ServiceName," Inside fn_retrnsmt_rqst ll_ApplLastSeqNum |%lld|",ll_ApplLastSeqNum);
  }
  /*st_trddwnld.ApplBegSeqNum = htonll(ll_trd_lst_seq);*/
  st_trddwnld.ApplBegSeqNum = htonll(ll_ApplLastSeqNum);
  st_trddwnld.ApplEndSeqNum = NO_VALUE_ULONG;
  st_trddwnld.Pad1[0] = '\0';

  st_trddwnld.ApplBegSeqNum = fn_swap_ulnglng(st_trddwnld.ApplBegSeqNum);
  st_trddwnld.ApplEndSeqNum = fn_swap_ulnglng(st_trddwnld.ApplEndSeqNum);

  st_trddwnld.MessageHeaderIn.BodyLen = fn_swap_ulong(st_trddwnld.MessageHeaderIn.BodyLen);
  st_trddwnld.MessageHeaderIn.TemplateID = fn_swap_ushort(st_trddwnld.MessageHeaderIn.TemplateID);
  st_trddwnld.RequestHeader.MsgSeqNum = fn_swap_ulong(st_trddwnld.RequestHeader.MsgSeqNum);
  st_trddwnld.RequestHeader.SenderSubID = fn_swap_ulong(st_trddwnld.RequestHeader.SenderSubID);
  st_trddwnld.SubscriptionScope = fn_swap_ulong(st_trddwnld.SubscriptionScope);
  st_trddwnld.PartitionID = fn_swap_ushort(st_trddwnld.PartitionID);
 
 st_q_con_clnt.li_msg_type = PRSNL_TRD_DWNLD;

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_trddwnld,sizeof(st_trddwnld));

  if(DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName,"Before call to fn_write_msg_q --Val of i_snd_qid <%d>",i_snd_qid);
  }

  if(fn_write_msg_q(i_snd_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)
  {
    fn_errlog(c_ServiceName, "L31245", LIBMSG, c_err_msg);
    return -1;
  }

  fn_userlog(c_ServiceName," After fn_write_msg_q");
return 0;
}
/** Ver 1.8 ends **/ 
