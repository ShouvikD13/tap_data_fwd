/********************************************************************************
 * Program Name : fo_bse_exg_con_lib.pc                                         *
 *                                                                              * 
 * Versioni - Name         -  Description                                       *
 * 1.0      - Sachin Birje -  New Release                                       *   
 * 1.1      - Shashi Kumar -  Handling Connect all issue in BFO                 *   
 * 1.2      - Ravi Kumar   -  BSE New changes										                *   
*********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sqlca.h>
#include <atmi.h>
#include <Usysflds.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <fo.h>
#include <fml_rout.h>
#include <fn_log.h>
#include <fn_env.h>
#include <fml32.h>
#include <fo_fml_def.h>
#include <fn_tuxlib.h>

#include <fn_msgq.h>
#include <fn_read_debug_lvl.h>

#include <fo_bse_exg_con_lib.h>
#include <fo_bse_exch_comnN.h>
#include<fn_fo_bse_convrtn.h>
#include <ETILayouts.h>		/* Ver 1.2 */

EXEC SQL INCLUDE "table/bph_bfo_psswd_hndlg.h";


long l_msg_typ;
extern int  i_send_qid;
void  trim(char *str);

/*********************************************************************************/
/** Sending Registration Request                                                **/
/*********************************************************************************/

 int  fn_rgstrn_to_bse(FBFR32 *ptr_fml_ibuf, 
                       char *c_ServiceName, 
                       char *c_err_msg)
 {
  int   i_err[5];
  int   i_ferr[5];
  int   i_cnt;
  int   i_login_stts;
  int i_tran_id;
  int i_ch_val;
 
  long int li_user_id;
  long int li_ip_usr_id; 
  char c_rgstrn_lgn_flg;

/*******Start of Ver 1.1 **************************/

  int   i_send_qid_new;       
	int   i_msgq_typ;           
  char  ptr_c_fileName [512]; 
  char  *ptr_c_tmp;        		

/*******End of Ver 1.1  **************************/

  struct st_bfo_usr_rgstrn_req st_rgstrn_req;
  struct st_send_rqst_data st_q_con_clnt;

  MEMSET(st_rgstrn_req);
  MEMSET(st_q_con_clnt);
  

  EXEC SQL BEGIN DECLARE SECTION;
   long int  sql_li_ip_usr_id;
   char      sql_c_ip_pipe_id[2+1];
   char      sql_c_clnt_nm[40];
   char      sql_c_clnt_stts;
  EXEC SQL END DECLARE SECTION;    


  i_err[0] = Fget32(ptr_fml_ibuf,FFO_SPL_FLG,0,(char *)&c_rgstrn_lgn_flg,0);
  i_ferr[0] = Ferror32;
  i_err[1] = Fget32(ptr_fml_ibuf,FFO_PIPE_ID,0,(char *)sql_c_ip_pipe_id,0);
  i_ferr[1] = Ferror32;
   
  for (i_cnt=0; i_cnt < 2; i_cnt++)
  {
      if (i_err[i_cnt] == -1)
      {
        fn_userlog(c_ServiceName, "Error in field %d", i_cnt);
        fn_errlog(c_ServiceName,"L31005",Fstrerror32(i_ferr[i_cnt]),c_err_msg);
        return -1;
      }
  }
  if(DEBUG_MSG_LVL_0)
  { 
   fn_userlog(c_ServiceName,"Inside Registration To BSE");
   fn_userlog(c_ServiceName,"c_rgstrn_lgn_flg :%c:",c_rgstrn_lgn_flg);
   fn_userlog(c_ServiceName,"sql_c_ip_pipe_id :%s:",sql_c_ip_pipe_id);
  }
   
  strcpy(sql_c_clnt_nm,"cln_bse_esr_clnt");

/*********Start  Ver 1.1 ****************************************/

  sprintf(ptr_c_fileName, "%s/%s.ini", getenv("BIN"), sql_c_ip_pipe_id);

  fn_userlog(c_ServiceName,"sh file name  :%s:",ptr_c_fileName);


  i_ch_val = fn_init_prcs_spc( "SFO_BCON_CLNT",
                                ptr_c_fileName,
                                "SFO_BCON_CLNT" );
  if (i_ch_val == -1)
  {
    fn_userlog (c_ServiceName,"Initializing Process space is failed " );
    return ( -1 );
  }

  ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"SEND_QUEUE");
  i_msgq_typ = atoi(ptr_c_tmp);

  fn_userlog(c_ServiceName,"SEND_QUEUE ID :%d:",i_msgq_typ);


  if(fn_crt_msg_q(&i_send_qid_new, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
  {
    fn_userlog (c_ServiceName,"Failed to take access of Transmit queque");
    return ( -1 );
  }

/***********End Ver 1.1 *********************************************/
   
  EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
      AND     bps_xchng_cd = 'NA'
      AND     bps_pipe_id = :sql_c_ip_pipe_id;

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31010", SQLMSG, c_err_msg);
    return -1;
  }
 
  if(sql_c_clnt_stts == 'N')
  {
    sprintf(c_err_msg,"ESR client is not running for Pipe id :%s: and  user id :%ld:",sql_c_ip_pipe_id,li_ip_usr_id);
    return -1;
  }
    
  /**** Checking if user is already logged on in console or not ****/

  EXEC SQL
    SELECT opm_login_stts
    INTO   :i_login_stts
    FROM   opm_ord_pipe_mstr
    WHERE  opm_pipe_id = :sql_c_ip_pipe_id;

  if ( SQLCODE != 0 )
  {
    fn_errlog(c_ServiceName, "L31015", SQLMSG, c_err_msg );
    return -1;
  }

  if(i_login_stts == 1)
  {
    sprintf ( c_err_msg, "User is already logged in for pipe id :%s:,user id :%ld:",sql_c_ip_pipe_id,li_ip_usr_id);
    return -1;
  }
  
  /**** Coping the Registration Data in exchange structure *****/ 
 
  EXEC SQL
    SELECT TO_NUMBER(TRIM(OPM_TRDR_ID))
    INTO   :st_rgstrn_req.l_trader_id
    FROM   OPM_ORD_PIPE_MSTR
    WHERE  OPM_XCHNG_CD ='BFO' 
    AND    OPM_PIPE_ID  = :sql_c_ip_pipe_id;

  if ( SQLCODE != 0)
  {
    fn_userlog(c_ServiceName,"Error while selecting Pipe specific and Exchange specific details.");
    fn_errlog(c_ServiceName, "L31020", SQLMSG, c_err_msg);
    return ( -1 );
  }     
 
  EXEC SQL
    SELECT TO_NUMBER(TRIM(EXG_BRKR_ID))
    into   :st_rgstrn_req.l_member_id
    FROM   EXG_XCHNG_MSTR
    WHERE  EXG_XCHNG_CD ='BFO'; 

  if( SQLCODE != 0)
  {
    fn_userlog(c_ServiceName,"Error while selecting Broker id.");
    fn_errlog(c_ServiceName, "L31025", SQLMSG, c_err_msg);
    return(-1);
  }
  if(DEBUG_MSG_LVL_3 )
  {
   fn_userlog(c_ServiceName,"----Before converation----");
   fn_userlog(c_ServiceName,"User id :%ld:",li_ip_usr_id);
   fn_userlog(c_ServiceName,"Trader Id :%ld:",st_rgstrn_req.l_trader_id);
   fn_userlog(c_ServiceName,"Member ID :%ld:",st_rgstrn_req.l_member_id); 
  }
 
  /**** Copy the required data for registration *****/


	/*** COmmenting in Ver 1.2

  st_rgstrn_req.l_msg_typ = htonl(USR_RGSTRN_REQ);
  
  st_rgstrn_req.l_slot_no = htonl(-1);
  
  st_rgstrn_req.st_hdr.l_slot_no = htonl(-1);

  st_rgstrn_req.st_hdr.l_message_length = htonl(sizeof(st_rgstrn_req) - sizeof(st_rgstrn_req.st_hdr));

  st_q_con_clnt.li_msg_type = USR_RGSTRN;
  st_rgstrn_req.l_msg_typ = fn_swap_long(st_rgstrn_req.l_msg_typ);
  st_rgstrn_req.l_slot_no = fn_swap_long(st_rgstrn_req.l_slot_no);
  st_rgstrn_req.st_hdr.l_slot_no = fn_swap_long(st_rgstrn_req.st_hdr.l_slot_no);
  st_rgstrn_req.st_hdr.l_message_length = fn_swap_long(st_rgstrn_req.st_hdr.l_message_length);
  st_rgstrn_req.l_member_id = fn_swap_long(st_rgstrn_req.l_member_id);
  st_rgstrn_req.l_trader_id = fn_swap_long(st_rgstrn_req.l_trader_id);
   
  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_rgstrn_req,sizeof(st_rgstrn_req)); 
  if ( DEBUG_MSG_LVL_3 )
  {
	 fn_userlog(c_ServiceName,"BSE ORS st_q_con_clnt.st_bsnd_rqst_data Is :%ld:",sizeof(st_q_con_clnt.st_bsnd_rqst_data));
  }

  **** End of coping data ****
  
  if( DEBUG_MSG_LVL_3 ) 
  { 
   fn_userlog(c_ServiceName,"---- After convertion---- ");
   fn_userlog(c_ServiceName,"l_msg_typ :%ld",st_rgstrn_req.l_msg_typ);
   fn_userlog(c_ServiceName,"l_slot_no :%ld",st_rgstrn_req.l_slot_no);
   fn_userlog(c_ServiceName,"l_message_length :%ld:",st_rgstrn_req.st_hdr.l_message_length);
   fn_userlog(c_ServiceName,"l_member_id :%ld:",st_rgstrn_req.l_member_id);
   fn_userlog(c_ServiceName,"l_trader_id :%ld:",st_rgstrn_req.l_trader_id);
   fn_userlog(c_ServiceName," size of st_rgstrn_req :%d:",sizeof(st_rgstrn_req));
  }
	commented in Ver 1.2 ***/

 /** Ver 1.2 STart **/
	st_q_con_clnt.li_msg_type = USR_RGSTRN;
  st_rgstrn_req.BodyLen = sizeof(st_rgstrn_req);
  st_rgstrn_req.TemplateID = USR_RGSTRN_REQ;
  strcpy(st_rgstrn_req.NetworkMsgID,"");
  strcpy(st_rgstrn_req.Pad2,"");
  st_rgstrn_req.MsgSeqNum = 0;
  st_rgstrn_req.SenderSubID = 0;

  st_rgstrn_req.BodyLen = fn_swap_long(st_rgstrn_req.BodyLen);
  st_rgstrn_req.l_trader_id = fn_swap_long(st_rgstrn_req.l_trader_id);
  st_rgstrn_req.l_member_id = fn_swap_long(st_rgstrn_req.l_member_id);

   fn_userlog(c_ServiceName,"---- After convertion---- ");
   fn_userlog(c_ServiceName,"BodyLen :%ld",st_rgstrn_req.BodyLen);
   fn_userlog(c_ServiceName,"TemplateId :%ld",st_rgstrn_req.TemplateID);
   fn_userlog(c_ServiceName,"l_member_id :%ld:",st_rgstrn_req.l_member_id);
   fn_userlog(c_ServiceName,"l_trader_id :%ld:",st_rgstrn_req.l_trader_id);
   fn_userlog(c_ServiceName," size of st_rgstrn_req :%d:",sizeof(st_rgstrn_req));

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_rgstrn_req,sizeof(st_rgstrn_req)); 

   fn_userlog(c_ServiceName,"---- After copy---- ");

   fn_userlog(c_ServiceName,"con_BodyLen :%ld",st_q_con_clnt.st_bsnd_rqst_data.st_busr_rgstrn_req.BodyLen);
   fn_userlog(c_ServiceName,"con_TemplateId :%ld",st_q_con_clnt.st_bsnd_rqst_data.st_busr_rgstrn_req.TemplateID);
   fn_userlog(c_ServiceName,"con_l_member_id :%ld:",st_q_con_clnt.st_bsnd_rqst_data.st_busr_rgstrn_req.l_member_id);
   fn_userlog(c_ServiceName,"con_l_trader_id :%ld:",st_q_con_clnt.st_bsnd_rqst_data.st_busr_rgstrn_req.l_trader_id);
   fn_userlog(c_ServiceName," size of st_q_con_clnt :%d:",sizeof(st_q_con_clnt));

  /** Ver 1.2 End **/


/**** Commented in 1.1 ******
		if(fn_write_msg_q(i_send_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)

  {
    fn_errlog(c_ServiceName, "L31030", LIBMSG, c_err_msg);
    return -1;
  } 
*****************/

/*** Start Ver 1.1 ********************************/
    if(fn_write_msg_q(i_send_qid_new,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)
  {
    fn_errlog(c_ServiceName, "L31035", LIBMSG, c_err_msg);
    return -1;
  }

/*** Endded Ver 1.1 ********************************/

  i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
  if ( i_tran_id == -1 )
  {
    fn_errlog(c_ServiceName, "L31040", LIBMSG, c_err_msg );
    return(-1); 
  }

  EXEC SQL
     UPDATE BPH_BFO_PSSWD_HNDLG
     SET    BPH_RGSTRN_LGN_FLG = :c_rgstrn_lgn_flg
     WHERE  BPH_PIPE_ID    = :sql_c_ip_pipe_id;

  if ( SQLCODE != 0 )
  {
    fn_userlog(c_ServiceName," Error in Update of new password");
    fn_aborttran(c_ServiceName, i_tran_id, c_err_msg);
    fn_errlog(c_ServiceName, "L31045", SQLMSG, c_err_msg);
    return(-1);

  }

  i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName, "L31050", LIBMSG, c_err_msg );
    fn_aborttran(c_ServiceName, i_tran_id, c_err_msg); 
    return(-1);
  } 
   

  return 0;

 } /*** End of registration ****/



/**************************************************************************/
/** Sending Logoff Request                                               **/
/**************************************************************************/


 int fn_logoff_from_bse(char *c_pipe_id,
                        char *c_ServiceName,
                        char *c_err_msg)
 {
  
  char c_curnt_passwd[LEN_PASS_WD];
  int  i_ret_val;
 
  /** struct st_bfo_logon_req st_signoff_req; commented in Ver 1.2 **/
	UserLogoutRequestT   st_signoff_req;	/** Ver 1.2 **/
  struct st_send_rqst_data st_q_con_clnt;
  
   
  MEMSET(st_signoff_req);
  MEMSET(st_q_con_clnt);
  /**
  MEMSET(c_curnt_passwd);
  **/

  MEMSET(sql_bph_curnt_passwd);

  EXEC SQL
    SELECT BPH_CURNT_PASSWD,
					 (TRIM(EXG_BRKR_ID)||'00'|| LPAD(BPH_USER_ID,3,'0')),  /* <Brokerd id>00<userid> */ /* Ver 1.2*/
					 (SUBSTR(BPH_PIPE_ID,2,1) || '0')	 		 /* Ver 1.2*/
    INTO   :sql_bph_curnt_passwd,
					 :st_signoff_req.Username,						 /* Ver 1.2*/
					 :st_signoff_req.RequestHeader.SenderSubID		 /* Ver 1.2*/
    FROM   BPH_BFO_PSSWD_HNDLG,
					 EXG_XCHNG_MSTR	/* Ver 1.2*/
    WHERE  BPH_PIPE_ID = :c_pipe_id 
					 AND EXG_XCHNG_CD='BFO'; 	/* Ver 1.2*/
  
  if ( SQLCODE != 0)
  {
    fn_userlog(c_ServiceName,"Error while selecting Pipe specific and Exchange specific details.");
    fn_errlog(c_ServiceName, "L31055", SQLMSG, c_err_msg);
    return ( -1 );
  }
  SETNULL(sql_bph_curnt_passwd);
  if ( DEBUG_MSG_LVL_0 )
  {
   fn_userlog(c_ServiceName," sql_bph_curnt_passwd :%s:",sql_bph_curnt_passwd.arr);
  }
  /**** Coping the logoff Data in exchange structure *****/

	/******** Commenting in Ver 1.2 Start 

  st_signoff_req.st_hdr.l_slot_no = htonl(REQ_SLOT_NO);
 
  st_signoff_req.l_msg_typ = htonl(LOGOFF_REQ);
 
  strcpy((char *)st_signoff_req.c_password,(char *)sql_bph_curnt_passwd.arr);
 
  st_signoff_req.c_filler = ' ';
 
  st_signoff_req.l_msg_tag = htonl(LOGOFF_MSG_TAG);
 
  st_signoff_req.st_hdr.l_message_length = htonl(sizeof(st_signoff_req) - sizeof(st_signoff_req.st_hdr));
  
  st_q_con_clnt.li_msg_type = LOGOFF_REQ;

  st_signoff_req.st_hdr.l_slot_no = fn_swap_long(st_signoff_req.st_hdr.l_slot_no);
  st_signoff_req.l_msg_typ = fn_swap_long(st_signoff_req.l_msg_typ);
  st_signoff_req.l_msg_tag = fn_swap_long(st_signoff_req.l_msg_tag);
  st_signoff_req.st_hdr.l_message_length = fn_swap_long(st_signoff_req.st_hdr.l_message_length);

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_signoff_req,sizeof(st_signoff_req));

  *** End of coping data ***

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"Inside LogOff function");
   fn_userlog(c_ServiceName,"l_msg_typ :%ld:", st_signoff_req.l_msg_typ);
   fn_userlog(c_ServiceName,"l_slot_no :%ld:",st_signoff_req.st_hdr.l_slot_no);
   fn_userlog(c_ServiceName,"c_password :%s:",st_signoff_req.c_password);
   fn_userlog(c_ServiceName,"l_msg_tag :%ld:",st_signoff_req.l_msg_tag);
   fn_userlog(c_ServiceName,"l_message_length :%ld:",st_signoff_req.st_hdr.l_message_length);
  }
	commenting end in ver 1.2 ***************/


	/** Ver 1.2 start **/

  st_q_con_clnt.li_msg_type = LOGOFF_REQ;

	st_signoff_req.MessageHeaderIn.BodyLen = htonl(sizeof(UserLogoutRequestT));
  st_signoff_req.MessageHeaderIn.TemplateID = htons(TID_USER_LOGOUT_REQUEST);
  strcpy(st_signoff_req.MessageHeaderIn.NetworkMsgID,"");
  strcpy(st_signoff_req.MessageHeaderIn.Pad2,"");
  st_signoff_req.RequestHeader.MsgSeqNum = 0;

  fn_userlog(c_ServiceName,"User Logoff Request TemplateID :%d: BodyLen :%ld:  Username :%ld: SenderSubID :%ld:",st_signoff_req.MessageHeaderIn.TemplateID,st_signoff_req.MessageHeaderIn.BodyLen,st_signoff_req.Username,st_signoff_req.RequestHeader.SenderSubID);

  st_signoff_req.MessageHeaderIn.BodyLen = fn_swap_ulong(st_signoff_req.MessageHeaderIn.BodyLen);
  st_signoff_req.MessageHeaderIn.TemplateID = fn_swap_ushort(st_signoff_req.MessageHeaderIn.TemplateID);
  st_signoff_req.RequestHeader.MsgSeqNum = fn_swap_ulong(st_signoff_req.RequestHeader.MsgSeqNum);
  st_signoff_req.RequestHeader.SenderSubID = fn_swap_ulong(st_signoff_req.RequestHeader.SenderSubID);
  st_signoff_req.Username = fn_swap_ulong(st_signoff_req.Username);

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_signoff_req,sizeof(st_signoff_req));
	/** Ver 1.2 End **/

  if(fn_write_msg_q(i_send_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)

  {
    fn_errlog(c_ServiceName, "L31060", LIBMSG, c_err_msg);
    return -1;
   }  

  return 0; 

 }

 int fn_chng_optnl_passwd_req(FBFR32 *ptr_fml_ibuf,long l_rqst_typ,char *c_ServiceName,char *c_err_msg  )
 {
  int   i_err[5];
  int   i_ferr[5];
  int   i_cnt;
  int   i_login_stts;
  int   i_snd_seq;
  int   i_ch_val;
  int   i_tran_id;
  int   i_seq;
  int   i_nxt_seq;
  int   i_ret_val;
  
  char c_passwd_flg;
  char  c_tmp;
  
  long int li_ip_usr_id;
  long int li_user_id;
  
  char c_pipe_id[2+1];  
  char c_encrptd_passwd[LEN_PASS_WD];
  char c_decrptd_passwd[LEN_PASS_WD];
  char c_curnt_passwd[LEN_PASS_WD];
  char c_new_passwd[LEN_PASS_WD];
  char c_num[2];
  char c_new_pass_wd_entrd;
  char c_opn_ordr_dtls_flg;

  char sql_c_ip_pipe_id[2+1];
 
  /* struct st_bfo_upd_passwd_req st_upd_passwd_req; commented in ver 1.2	*/
	UserPasswordChangeRequestT st_upd_passwd_req;       /* Ver 1.2 */
  struct st_send_rqst_data st_q_con_clnt;

  
  i_err[0] = Fget32(ptr_fml_ibuf,FFO_PIPE_ID,0,(char *)c_pipe_id,0);
  i_ferr[0] = Ferror32;
   
  for (i_cnt=0; i_cnt < 1; i_cnt++)
  {
    if (i_err[i_cnt] == -1)
    {
      fn_userlog(c_ServiceName, "Error in field %d", i_cnt);
      fn_errlog(c_ServiceName,"L31065",Fstrerror32(i_ferr[i_cnt]),c_err_msg);
      return -1;
    }
  }

  MEMSET(st_upd_passwd_req);
  MEMSET(st_q_con_clnt);

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
   fn_errlog(c_ServiceName, "L31070", SQLMSG, c_err_msg);
   return(-1);

  }
  if( i_login_stts == 0)
  {
    fn_userlog(c_ServiceName," User has not logged in for pipe id :%s:",c_pipe_id);
    fn_errlog(c_ServiceName, "L31075", LIBMSG, c_err_msg );
    return (-1);
  }  
  
  EXEC SQL
    SELECT BPH_CURNT_PASSWD,
					 TO_NUMBER(TRIM(EXG_BRKR_ID)||'00'|| LPAD(BPH_USER_ID,3,'0')),   /* <Brokerd id>00<userid> *//* ver 1.2 */
					 TO_NUMBER(TRIM(EXG_BRKR_ID)||'00'|| LPAD(BPH_USER_ID,3,'0')),  /* <Brokerd id>00<userid> */ /* ver 1.2 */
					 (SUBSTR(BPH_PIPE_ID,2,1) || '0') /* <pipe id[1] || '0' <Sequence> */ /* ver 1.2 */
    INTO   :c_curnt_passwd,
					 :st_upd_passwd_req.Username,     /* ver 1.2 */
           :st_upd_passwd_req.RequestHeader.SenderSubID, /* ver 1.2 */
           :st_upd_passwd_req.RequestHeader.MsgSeqNum  /* ver 1.2 */
    FROM   BPH_BFO_PSSWD_HNDLG,
						EXG_XCHNG_MSTR  /* ver 1.2 */
    WHERE  BPH_PIPE_ID = :c_pipe_id
				   AND EXG_XCHNG_CD='BFO';	/* ver 1.2 */
  if ( SQLCODE != 0 )
  {
   fn_userlog(c_ServiceName," Error in getting current password");
   fn_errlog(c_ServiceName, "L31080", SQLMSG, c_err_msg);
   return(-1);

  }
  trim(c_curnt_passwd);
 
  if( l_rqst_typ == OPTNL_PASSWD_CHNG )
  {
   fn_userlog(c_ServiceName," Inside OPTNL_PASSWD_CHNG ");
 
   c_passwd_flg = 'O';

   i_err[0] = Fget32(ptr_fml_ibuf,FFO_NEW_PASS_WD,0,(char *)c_new_passwd,0);
   i_ferr[0] = Ferror32;
   for (i_cnt=0; i_cnt < 1; i_cnt++)
   {
    if (i_err[i_cnt] == -1)
    {
      fn_userlog(c_ServiceName, "Error in field %d", i_cnt);
      fn_errlog(c_ServiceName,"L31085",Fstrerror32(i_ferr[i_cnt]),c_err_msg);
      return -1;
    }
   }
   trim(c_new_passwd);
   if (strlen(c_new_passwd) < 8 )
   {
     fn_userlog(c_ServiceName, " NEW PASSWORD LENGTH CAN'T BE LESS THAN 8 CHARACTERS");
     fn_errlog(c_ServiceName, "L31090", LIBMSG, c_err_msg );
     return -1;
   }
   if (strlen(c_new_passwd) > 8 )
   {
     fn_userlog(c_ServiceName, " PLEASE ENTER THE PASSWORD OF 8 CHARACTER LENGTH [eg. abcdef11 ]");
     fn_errlog(c_ServiceName, "L31095", LIBMSG, c_err_msg );
     return -1;
   }
   if ( DEBUG_MSG_LVL_0 )
   { 
    fn_userlog(c_ServiceName," New password got is :%s:",c_new_passwd);
   }
  }

  /**** Update new password requested in BPH table ***/
  
  i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
  if ( i_tran_id == -1 )
  {
    fn_errlog(c_ServiceName, "L31100", LIBMSG, c_err_msg );
    return(-1);
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
    fn_errlog(c_ServiceName, "L31105", SQLMSG, c_err_msg);
    return(-1);

  } 

  i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName, "L31110", LIBMSG, c_err_msg );
    fn_aborttran(c_ServiceName, i_tran_id, c_err_msg);
    return(-1);
  }   

  /*** Coping the required data for change password ***/

	/** Commented in Ver 1.2  
  if( l_rqst_typ == OPTNL_PASSWD_CHNG)
  {
   fn_userlog(c_ServiceName," Inside Optional ");
 
   st_upd_passwd_req.l_msg_typ = htonl(OPTNL_PASSWD_CHNG); 
 
   st_q_con_clnt.li_msg_type = OPTNL_PASSWD_CHNG;
  }
  *****
  else if(l_rqst_typ == MANDATORY_PASSWD_CHNG )
  {
   fn_userlog(c_ServiceName," Inside Mandatory Password Change ");
 
   st_upd_passwd_req.l_msg_typ = htonl(MANDATORY_PASSWD_CHNG);
 
   st_q_con_clnt.li_msg_type = MANDATORY_PASSWD_CHNG;
  }
 
  *****
 
  strcpy((char *)st_upd_passwd_req.c_old_pwd,(char *)c_curnt_passwd);
 
  strcpy((char *)st_upd_passwd_req.c_new_pwd,(char *)c_new_passwd);
 
  st_upd_passwd_req.l_msg_tag = htonl(UPD_PASSWD_TAG);
 
  st_upd_passwd_req.st_hdr.l_slot_no = htonl(REQ_SLOT_NO);
 
  st_upd_passwd_req.st_hdr.l_message_length=htonl(sizeof(st_upd_passwd_req) - sizeof(st_upd_passwd_req.st_hdr));
 
  
  st_upd_passwd_req.l_msg_typ = fn_swap_long(st_upd_passwd_req.l_msg_typ);
  st_upd_passwd_req.l_msg_tag = fn_swap_long( st_upd_passwd_req.l_msg_tag );
  st_upd_passwd_req.st_hdr.l_slot_no = fn_swap_long(st_upd_passwd_req.st_hdr.l_slot_no);
  st_upd_passwd_req.st_hdr.l_message_length = fn_swap_long(st_upd_passwd_req.st_hdr.l_message_length);

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_upd_passwd_req,sizeof(st_upd_passwd_req));
  
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
	commented in Ver 1.2 **/

	/* Added in ver 1.2 -Starts */
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

	fn_userlog(c_ServiceName,"Password Change Data copied in structure");
	fn_userlog(c_ServiceName,"Old Password :%s: New Passwd :%s: ",st_upd_passwd_req.Password,st_upd_passwd_req.NewPassword);

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_upd_passwd_req,sizeof(st_upd_passwd_req));

  /* Added in ver 1.2 -End */
 
  fn_userlog(c_ServiceName," Calling fn_write_msg_q");
 
  if(fn_write_msg_q(i_send_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)
  {
    fn_errlog(c_ServiceName, "L31115", LIBMSG, c_err_msg);
    return -1;
  }  
  if( DEBUG_MSG_LVL_0 )
  {
   fn_userlog(c_ServiceName," After call to fn_write_msg_q");
  }
  return 0;

 }

int fn_logon_bse_rqst( FBFR32 *ptr_fml_ibuf,char *c_ServiceName, char *c_err_msg  )
{

  int   i_cnt;
  int   i_login_stts;
  int   i_ch_val;
  int   i_ret_val;
  int   idx;
  int   i_err[10];
  int   i_ferr[10];
   
  char  c_tmp;
  long int li_ip_usr_id;
  long int li_user_id;

  char c_curnt_passwd[LEN_PASS_WD];
  char c_ip_new_pass_wd[LEN_PASS_WD];
  char c_ip_pass_wd[LEN_PASS_WD];
  char c_new_pass_wd_entrd;
  char c_opn_ordr_dtls_flg;
  char c_pipe_id[2+1];

  EXEC SQL BEGIN DECLARE SECTION;
    long int  sql_li_ip_usr_id;
    char c_sql_pipe_id[2+1];
    char sql_c_ip_pipe_id[2+1];
    char sql_c_clnt_nm[40];  
    char sql_c_clnt_stts; 
  EXEC SQL END DECLARE SECTION;

  MEMSET(c_sql_pipe_id);

  strcpy(c_sql_pipe_id , c_pipe_id);

  /* struct st_bfo_logon_req st_signon_req;	 ver 1.2 **/
	UserLoginRequestT st_signon_req;            /* Ver 1.2 */
  struct st_send_rqst_data st_q_con_clnt;


  MEMSET(st_signon_req);
  MEMSET(st_q_con_clnt);

  MEMSET(c_curnt_passwd);
 
  i_err[0] = Fget32(ptr_fml_ibuf,FFO_EXCHNG_USR_ID,0,(char *)&li_ip_usr_id,0);
  i_ferr[0] = Ferror32;
  i_err[1] = Fget32(ptr_fml_ibuf,FFO_PIPE_ID,0,(char *)sql_c_ip_pipe_id,0);
  i_ferr[1] = Ferror32;
  i_err[2] = Fget32(ptr_fml_ibuf,FFO_PASS_WD,0,(char *)c_ip_pass_wd,0);
  i_ferr[2] = Ferror32;

  if( DEBUG_MSG_LVL_3 )
  {
   fn_userlog(c_ServiceName,"li_ip_usr_id :%ld:",li_ip_usr_id);       
   fn_userlog(c_ServiceName,"sql_c_ip_pipe_id :%s:",sql_c_ip_pipe_id);
  }


  for (i_cnt=0; i_cnt <= 2; i_cnt++)
  {
      if (i_err[i_cnt] == -1 && i_cnt==1)	/** added i_cnt==1 in Ver 1.2 **/
      {
        fn_userlog(c_ServiceName, "Error in field %d", i_cnt);
        fn_errlog(c_ServiceName,"L31120",Fstrerror32(i_ferr[i_cnt]),c_err_msg);
        return -1; 
      }
  }

  /**** Checking if ESR is running or not ******/
  strcpy(sql_c_clnt_nm,"cln_bse_esr_clnt");

  EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
      AND     bps_xchng_cd = 'NA'
      AND     bps_pipe_id = :sql_c_ip_pipe_id;

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31125", SQLMSG, c_err_msg);
    return -1;
  }

  if(sql_c_clnt_stts == 'N')
  {
      sprintf(c_err_msg,"ESR client is not running for Pipe id :%s: and  user id :%ld:",sql_c_ip_pipe_id,li_ip_usr_id);
      return -1;
  }
  /**** Checking if user is already logged on in console or not ****/
	MEMSET(c_ip_pass_wd);  /* Ver 1.2 */
  EXEC SQL
    SELECT opm_login_stts,
           /* opm_user_id    commented in ver 1.2 */
					 TO_NUMBER(TRIM(EXG_BRKR_ID)||'00'|| LPAD(OPM_USER_ID,3,'0')),	/** Ver 1.2 **/
					 (SUBSTR(OPM_PIPE_ID,2,1) || '0'),
						BPH_CURNT_PASSWD	
    INTO   :i_login_stts,
           /* :li_user_id commented in ver 1.2 */
					 :st_signon_req.Username,
					 :st_signon_req.RequestHeader.SenderSubID,
					 :c_ip_pass_wd
    FROM   opm_ord_pipe_mstr,
						BPH_BFO_PSSWD_HNDLG,/* 1.2 */
						EXG_XCHNG_MSTR /* 1.2 */
    WHERE  opm_pipe_id = :sql_c_ip_pipe_id 
					 AND BPH_PIPE_ID=:sql_c_ip_pipe_id AND EXG_XCHNG_CD='BFO'; /* 1.2 */

  if ( SQLCODE != 0 )
  {
    fn_errlog(c_ServiceName, "L31130", SQLMSG, c_err_msg );
    return -1;
  }

  /* commented in Ver 1.2 
	 if( li_user_id  != li_ip_usr_id)      
  {
    fn_userlog(c_ServiceName,"Inside User ID Check");
    fn_userlog(c_ServiceName,"li_user_id Is :%ld:",li_user_id);
    fn_userlog(c_ServiceName,"li_ip_usr_id Is :%ld:",li_ip_usr_id);
    sprintf ( c_err_msg, "Please Enter Correct User ID For Pipe Id :%s:,user id :%ld:",sql_c_ip_pipe_id,li_user_id);
    return -1;
  } **/
  if(i_login_stts == 1)
  {
    sprintf ( c_err_msg, "User is already logged in for pipe id :%s:,user id :%ld:",sql_c_ip_pipe_id,li_ip_usr_id);
    return -1;
  }
  strcpy(c_curnt_passwd,c_ip_pass_wd);
  rtrim(c_curnt_passwd);

  fn_userlog(c_ServiceName," Login Password :%s:",c_curnt_passwd);

  /**** Coping the login Data in exchange structure *****/

	/*** Ver 1.2 Comment start

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

  **** End of Coping data ****
  if ( DEBUG_MSG_LVL_3 )
  {
   fn_userlog(c_ServiceName," Message Length in fn_logon_to_bse :%d:",sizeof(st_signon_req));
   fn_userlog(c_ServiceName,"l_slot_no  in fn_logon_to_bse :%ld:",st_signon_req.st_hdr.l_slot_no);
   fn_userlog(c_ServiceName,"l_msg_typ in fn_logon_to_bse  :%ld:",st_signon_req.l_msg_typ);
   fn_userlog(c_ServiceName,"l_msg_tag in fn_logon_to_bse  :%ld:",st_signon_req.l_msg_tag);
   fn_userlog(c_ServiceName,"l_message_length in fn_logon_to_bse :%ld:",st_signon_req.st_hdr.l_message_length);
   fn_userlog(c_ServiceName,"li_msg_type in fn_logon_to_bse  :%ld:",st_q_con_clnt.li_msg_type);
  }
	ver 1.2 comment end ***/

	/* Ver 1.2 Start */

  st_q_con_clnt.li_msg_type = LOGON_REQ;

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

	fn_userlog(c_ServiceName,"1User Login Request TemplateID :%d: BodyLen :%ld:  Username :%ld: Psswd :%s: SenderSubID :%ld:",st_signon_req.MessageHeaderIn.TemplateID,st_signon_req.MessageHeaderIn.BodyLen,st_signon_req.Username,st_signon_req.Password,st_signon_req.RequestHeader.SenderSubID);

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_signon_req,sizeof(st_signon_req));
	/* Ver 1.2 End */

  if(fn_write_msg_q(i_send_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)

  {
    fn_errlog(c_ServiceName, "L31135", LIBMSG, c_err_msg);
    return -1;
  }

   fn_userlog(c_ServiceName,"After fn_write_msg_q in exg_con.fn_logon_to_bse "); /* Ver 1.2 */

   return 0;

}
 
int fn_snd_opn_intrst_rqst	(	char *c_ServiceName,
															char *c_err_msg
														)
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
 	st_opn_intrst_rqst_data.l_rsrvd_fld  = 0;
 	st_opn_intrst_rqst_data.st_hdr.l_slot_no = fn_swap_long(st_opn_intrst_rqst_data.st_hdr.l_slot_no);
 	st_opn_intrst_rqst_data.st_hdr.l_message_length = fn_swap_long(st_opn_intrst_rqst_data.st_hdr.l_message_length);
 	st_q_con_clnt.li_msg_type = OPN_INTRST_RQST;

 	memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_opn_intrst_rqst_data,sizeof(st_opn_intrst_rqst_data));

 	if(	fn_write_msg_q	(	i_send_qid,
                    		(void *)&st_q_con_clnt,
                    		sizeof(st_q_con_clnt),
                    		c_ServiceName,
                    		c_err_msg) == -1)

  {
  	fn_errlog(c_ServiceName, "L31140", LIBMSG, c_err_msg);
   	fn_userlog(c_ServiceName,"ERROR- fn_write_msg_q in fn_snd_opn_intrst_rqst");
   	return -1;
  }

	if( DEBUG_MSG_LVL_3)
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



void trim(char *str)
{
  int i=0;
  char *ptr;
  ptr = str;
  while ( *str != '\0' )
  {
    if (*str != ' ' && *str != '\t')
    {
      i=i+1;
      *ptr++=*str;
    }
    str++;
  }
  *ptr='\0';
  while( i != 0 )
  {
    ptr--;
    i=i-1;
  }
  str = ptr;
}



/**************************************************/


