/******************************************************************************/
/*	Program									: CLN_BSE_ESR_CLNT																*/
/*																																						*/
/*  Input										: C_PIPE_ID																				*/
/*																																						*/
/*  Output									:																									*/
/*																																						*/
/*	Description							: BSE-Exchange send/receive client reads message  */
/*														from send message queue, sends to IML server    */
/*														through TCP/IP socket.  												*/
/*														This also resposible for receiving message from */
/*														IML and call receive client service for further */
/*														processing																			*/
/*																																						*/
/******************************************************************************/
/*	Ver 1.0		New Realease																										*/
/*	Ver 1.1		Migration Changes																								*/
/*	Ver 1.2		Keep Alive Message Changes | Sachin Birje. 											*/                    
/*	Ver 1.3	: 22-Jun-2012	Broker Suspension Changes [Sandip Tambe]            */
/*	Ver 1.4	: 28-Dec-2012	CAP Utilization UMS 1922  Changes [Sandip Tambe]    */
/*	Ver 1.5	: 06-Feb-2014 New IML Changes                   [Sandip Tambe]    */
/*	Ver 1.6	: 04-Aug-2023 New IML Changes                   [Ravi Malla]  	  */
/*  Ver 1.7 : 26-Dec-2023 Download changes in BSE           [Prajakta Sutar]  */
/*  Ver 1.8 : 03-Jan-2024 Core Issue in BSE       			    [Ravi Malla] 		  */
/******************************************************************************/
/**** C header ****/
#include <stdio.h>

#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>    
#include <pthread.h>    

/**** To be added for Pro c codes ****/
#include <sqlca.h>

/**** Tuxedo header ****/
#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>
#include <ETILayouts.h>   /* Ver 1.6 */

#include <netinet/in.h>

/**** Application header ****/
#include <fn_battmpt.h>
#include <fo_fml_def.h>
#include <fo.h>					  
#include <fn_env.h>					  
#include <fn_tuxlib.h>
#include <fn_scklib_iml.h>
#include <fn_pthread.h>
#include <fn_signal.h>
#include <fn_msgq.h>
#include <fn_fil_log.h>
#include <fn_ddr.h>
#include <fn_log.h>
#include <userlog.h>
#include <fml_rout.h>   
#include <fn_read_debug_lvl.h>  
#include <fo_bse_exg_esr_lib.h>					  
#include <fn_fo_bse_fprint.h>		
#include <fn_fo_bse_convrtn.h>
#include <fn_fobse_bcast.h>											/*** Ver 1.3 ***/ 
#include <fn_cnvt_to_hfmtB.h>				/** Ver 1.6 **/

int Z;
#define INIT(x,y)  for(Z=0; Z< y; Z++) x[Z] =0

#define TOTAL_FML 9 
#define ACK		'A'
#define	TRD		'T'
#define SLTP	'S'
#define ACKTRD 'Z'             /* Ver 1.6 */
#define ORDR_DWNLD 1           /* Ver 1.7 */
#define TRD_DWNLD 2            /* Ver 1.7 */

/**** Global Variable Declaration ****/

char c_thd_typ;
int  i_deferred_dwnld_flg;
char c_xchng_cd[3 + 1];
char c_trd_dt[20 + 1];
char c_skip_ordref[LEN_ORD_REFACK];
unsigned int ui_keep_alive_interval; 
char c_ServiceName [ 33 ];
char c_errmsg [ 256 ];
char c_fprint_msg[ 1024 ];

int i_snd_thrd_exit_stts;
int i_rcv_thrd_exit_stts;
int i_keep_alive_thrd_exit_stts;  /*** Ver 1.2 ***/
int i_partition_id = 0;  /** Ver 1.7 **/
int i_msg_dump_flg;
int i_snd_qid;
int i_iml_sck_id;
char c_delete_dwld_flg = 'N';

char c_ord_dw_cmplt = 'N';
char c_trd_dw_cmplt = 'N';
char c_sltp_dw_cmplt = 'N';
long l_tot_recrd_dwnlded_ord = 0;
long l_tot_recrd_dwnlded_trd = 0;
long l_tot_recrd_dwnlded_sltp = 0;
long l_tot_recrd_insert_ord = 0;
long l_tot_recrd_insert_sltp = 0;
long l_tot_recrd_insert_trd = 0;
char c_breco_start_flg = 'N';
long l_dwld_prcs_cnt = 0;
long l_seq_no = 0;  
long l_init_slot_no = 0;  
long l_init_msg_length = 0;  

char  c_snd_msg_data[MAX_SCK_MSG];

FILE *ptr_rcv_msgs_file;
FILE *ptr_snd_msgs_file;

EXEC SQL BEGIN DECLARE SECTION;
	char sql_c_xchng_cd[3+1];
	varchar sql_c_nxt_trd_date[LEN_DATE];	
	char sql_c_pipe_id [ 2+1 ];
	char sql_c_trd_ref[ 8+1 ];
	varchar sql_sec_tm [LEN_DATE];
	varchar sql_part_tm [LEN_DATE];
	varchar sql_inst_tm [LEN_DATE];
	varchar sql_idx_tm [LEN_DATE];
EXEC SQL END DECLARE SECTION;

int fn_crt_n_flushq ( int *ptr_i_qid,
                      char *ptr_c_qname,
                      char *c_ServiceName,
                      char *c_errmsg);

int fn_crt_iml_sck ( int *ptr_i_sck_id,
                      char *ptr_c_iml_addr_nm,
                      char *ptr_c_iml_port_nm,
                      char *c_ServiceName,
                      char *c_errmsg);

int fn_write_msgq_file(int *ptr_i_qid,
                       char *c_ServiceName,
                       char *c_errmsg);

void fn_sigusr2_hnd( int i_signo);
void fn_sigusr1_hnd( int i_signo);

/*** void fn_signalalarm_hnd(int signo); **** Ver 1.2 ***/

int fn_send_keepalive(void);

void* fn_bse_snd_thrd( );
void* fn_bse_rcv_thrd( );
void* fn_keep_alive_thrd( );  /*** Ver 1.2 ***/

/* ####################################################################################   */

void CLN_BSE_ESR_CLNT ( int argc, char *argv[] )
{
  char c_ServiceName[33];
  char c_errmsg[256];

  int i_ch_val;

  pthread_t snd_thrd;
  pthread_t rcv_thrd;
  pthread_t keep_alive_thrd;  /*** Ver 1.2 ***/

  pthread_attr_t snd_thrd_attr;
  pthread_attr_t rcv_thrd_attr;	
  pthread_attr_t keep_alive_thrd_attr;  /*** Ver 1.2 ***/

  strcpy(c_ServiceName,"cln_bse_esr_clnt");

  /*** Initialization of Send/Receive threads  ***/
  pthread_attr_init(&snd_thrd_attr);
  pthread_attr_init(&rcv_thrd_attr);
  pthread_attr_init(&keep_alive_thrd_attr);  /*** Ver 1.2 ***/

  /*** Initialize conditionla variables ***/
  pthread_mutex_init(&mut_thrd_cntrl, NULL);
  pthread_mutex_init(&mut_snd_rcv_cntrl, NULL);

  pthread_cond_init(&cond_thrd_cntrl, NULL);
  pthread_cond_init(&cond_snd_rcv_cntrl, NULL);

  pthread_attr_setscope(&snd_thrd_attr, PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setscope(&rcv_thrd_attr, PTHREAD_SCOPE_SYSTEM);
  pthread_attr_setscope(&keep_alive_thrd_attr, PTHREAD_SCOPE_SYSTEM);   /*** Ver 1.2 ***/

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"Before starting threads");
  }

  pthread_create(&snd_thrd, &snd_thrd_attr, fn_bse_snd_thrd , NULL);

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"After creating snd threads and Before starting rcv threads");
  }

  pthread_create(&rcv_thrd, &rcv_thrd_attr, fn_bse_rcv_thrd , NULL);  

  pthread_create(&keep_alive_thrd, &keep_alive_thrd_attr, fn_keep_alive_thrd , NULL); /** Ver 1.2 ***/

  pthread_mutex_lock(&mut_thrd_cntrl);
  i_cond_thrd_cntrl = 0;
  i_ch_val = fn_thrd_cond_wait ( &cond_thrd_cntrl,
                                 &mut_thrd_cntrl,
                                 c_ServiceName,
                                 c_errmsg);

	if(DEBUG_MSG_LVL_3)  
	{
   	fn_userlog(c_ServiceName,"val of i_cond_thrd_cntrl initially <%d>",i_cond_thrd_cntrl);
	}

  switch(i_cond_thrd_cntrl)
  {
    case EXIT_BY_SIGUSR2:

      fn_userlog(c_ServiceName,"EXIT_BY_SIGUSR2");
      pthread_mutex_unlock(&mut_thrd_cntrl);
      c_thd_typ = 'S';
      pthread_kill(snd_thrd, SIGUSR1);
      pthread_join(snd_thrd,NULL);
      c_thd_typ = 'R';
      pthread_kill(rcv_thrd, SIGUSR1);
      pthread_join(rcv_thrd, NULL);

     	pthread_kill(keep_alive_thrd,SIGUSR1);   /** Ver 1.2 ***/
      pthread_join(keep_alive_thrd,NULL);     /** Ver 1.2 ***/

      break;

    case EXIT_BY_RCV_THRD:

      fn_userlog(c_ServiceName,"EXIT_BY_RCV_THRD");
      pthread_mutex_unlock(&mut_thrd_cntrl);
      pthread_join(rcv_thrd, NULL);
      c_thd_typ = 'S';
      pthread_kill(snd_thrd, SIGUSR1);
      pthread_join(snd_thrd, NULL);

      pthread_kill(keep_alive_thrd,SIGUSR1);   /** Ver 1.2 ***/
      pthread_join(keep_alive_thrd,NULL);     /** Ver 1.2 ***/

      break;

    case EXIT_BY_SND_THRD:

      fn_userlog(c_ServiceName,"EXIT_BY_SND_THRD");
      pthread_mutex_unlock(&mut_thrd_cntrl);
      pthread_join(snd_thrd, NULL);
      c_thd_typ = 'R';
      pthread_kill(rcv_thrd, SIGUSR1);
      pthread_join(rcv_thrd, NULL);
 
      pthread_kill(keep_alive_thrd,SIGUSR1);   /** Ver 1.2 ***/
      pthread_join(keep_alive_thrd,NULL);     /** Ver 1.2 ***/

      break;

    default:

      pthread_mutex_unlock(&mut_thrd_cntrl);
      fn_errlog(c_ServiceName,"S31005", "Invalid exit type", c_errmsg);
      break;
  }

  pthread_attr_destroy(&snd_thrd_attr);
  pthread_attr_destroy(&rcv_thrd_attr);

  pthread_attr_destroy(&keep_alive_thrd_attr);  /*** Ver 1.2 ***/

	return; 
}
/* ###########################################################################  */

void fn_bat_term (  int argc, char *argv[] )
{
  char c_errmsg [ 256 ];
  char c_filter [ 256 ];

  strcpy(c_ServiceName, argv[0]);
  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"Inside fn_bat_term for pipe id :%s:",argv[3]);
  }

 	fn_close_sck ( i_iml_sck_id,
                 argv[0],
                 c_errmsg);

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"After fn_close_sck-Val of i_iml_sck_id<%d> and i_msg_dump_flg<%d>",i_iml_sck_id,i_msg_dump_flg);
  }

  if ( i_msg_dump_flg == 1 )
  {
    fn_destroy_file(ptr_rcv_msgs_file);
    fn_destroy_file(ptr_snd_msgs_file);
    fn_userlog(c_ServiceName,"Destroy file When i_msg_dump_flg is LOG_DUMP");
  }

  /** Imp Note: Here pack client is stopped using trigger when esr is stopped **/
  sprintf ( c_filter, "cln_bse_pack_clnt_%s", argv[3] );
  fn_pst_trg ( argv[0], "TRG_STP_BAT", "TRG_STP_BAT", c_filter );
  fn_userlog(c_ServiceName,"After posting trigger TRG_STP_BAT in fn_bat_term <%s>", c_filter);
  return;
}

/** #############################################################  **/

int fn_bat_init ( int argc, char *argv[] )
{
	char c_ServiceName [ 33 ];
	char c_fileName[ 200 ];
	char c_errmsg [ 256 ];
	char c_dump_msg_flg_nm[30];

	int  i_ch_val;
	int  i_trnsctn;
	int  i_snd_seq;
  char *ptr_c_tmp;
   
  
  INITBATDBGLVL(argv[0]);    

  strcpy(c_ServiceName, argv[0]);
  strcpy(sql_c_pipe_id, argv[3]);
  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName, "Start of fn_bat_init of cln_bse_esr_clnt- sql_c_pipe_id:%s:",sql_c_pipe_id);
  }

	if ( argc < 4 )
  {
    fn_userlog ( c_ServiceName, "Usage - cln_bse_esr_clnt <tag qualifier> <Exchange cd> <Pipe id>" ); 
    return (-1);
  }
  /*** Initialize the Routing string ***/
  fn_init_ddr_pop ( argv[3], TRADING_SECTION, COMMON);

	/**** Get exchange code from the database ****/
  EXEC SQL
           SELECT     OPM_XCHNG_CD 
           INTO      :sql_c_xchng_cd
           FROM      OPM_ORD_PIPE_MSTR
           WHERE     OPM_PIPE_ID = :sql_c_pipe_id;
  if ( SQLCODE != 0 )
  {
    fn_errlog(c_ServiceName,"S31010", SQLMSG, c_errmsg);
    return ( -1 );
  }
	if(DEBUG_MSG_LVL_3)
	{
	 fn_userlog(c_ServiceName,"In fn_bat_init -sql_c_xchng_cd:%s:",sql_c_xchng_cd);
	}

  /**** Get exchange next trade date from the database ****/

  EXEC SQL
					 SELECT to_char ( EXG_NXT_TRD_DT, 'dd-mon-yyyy' ),
					 				to_char ( EXG_NXT_TRD_DT, 'yyyymmdd' ),
									to_char ( EXG_SEC_TM, 'dd-mon-yyyy' ),
									to_char ( EXG_PART_TM, 'dd-mon-yyyy' ),
									to_char ( EXG_INST_TM, 'dd-mon-yyyy' ),
									to_char ( EXG_IDX_TM, 'dd-mon-yyyy' )
					 into :sql_c_nxt_trd_date,
								:sql_c_trd_ref,
								:sql_sec_tm,
								:sql_part_tm,
								:sql_inst_tm,
								:sql_idx_tm
					 FROM EXG_XCHNG_MSTR
					 WHERE EXG_XCHNG_CD = :sql_c_xchng_cd;
  if ( SQLCODE != 0 )
  {
    fn_errlog(c_ServiceName,"S31015", SQLMSG, c_errmsg);
    return ( -1 );
  }


/***  Get Send Message queue id and flush all messages   ***/

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog ( c_ServiceName, "Before fn_crt_n_flushq in fn_bat_init " ); 
  }

  i_ch_val = fn_crt_n_flushq(&i_snd_qid,
                             "SEND_QUEUE",
                             c_ServiceName,
                             c_errmsg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"S31020", LIBMSG, c_errmsg);
    fn_userlog ( c_ServiceName, "Error in fn_crt_n_flushq in fn_bat_init " ); 

		return ( -1 );
  }
  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog ( c_ServiceName, "Val of i_snd_qid -After fn_crt_n_flushq in fn_bat_init <%d>", i_snd_qid ); 
  }

	/**** Create socket with IML machine ****/
  i_ch_val = fn_crt_iml_sck(&i_iml_sck_id,
                       		  "IML_IP_ADDRESS",
 		 		 									  "IML_PORT",
                          	c_ServiceName,
                          	c_errmsg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"S31025", LIBMSG, c_errmsg);
    fn_userlog ( c_ServiceName, "Error in fn_crt_iml_sck in fn_bat_init " ); 
		return ( -1 );
  }
  if(DEBUG_MSG_LVL_3)
  {
  fn_userlog ( c_ServiceName, "Val of i_iml_sck_id -After fn_crt_iml_sck in fn_bat_init <%d>", i_iml_sck_id ); 
  }

/**** Create send/receive message dump files	****/
	strcpy(c_dump_msg_flg_nm,"DUMP_MSG_FLG");

	ptr_c_tmp = (char *)fn_get_prcs_spc ( c_ServiceName,
                                        c_dump_msg_flg_nm);
  if (ptr_c_tmp == NULL )
  {
    fn_errlog(c_ServiceName,"S31030", LIBMSG, c_errmsg);
    return ( -1 );
  }

  i_msg_dump_flg = atoi(ptr_c_tmp);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"The Dump Log Flg is:%d:%s:",i_msg_dump_flg,ptr_c_tmp);
	}

  if ( i_msg_dump_flg == LOG_DUMP )
  {
    sprintf(c_fileName, "trade/%s/%s/intmsgs_frombse.txt",sql_c_xchng_cd, sql_c_pipe_id);

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"path name for file of msg from bse is :%s:",c_fileName);
		}

    /*** To Write a dump of messages got from Exchange ***/
    i_ch_val = fn_create_file ( &ptr_rcv_msgs_file,
                                c_fileName,
                                "a+",
                                RELATIVE,
                                c_ServiceName,
                                c_errmsg);
    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S31035", (char *)UNXMSG, c_errmsg);
      fn_userlog(c_ServiceName,"Error in fn_create_file for msgs from bse");
      return ( -1 );
    }

    sprintf(c_fileName, "trade/%s/%s/intmsgs_tobse.txt",sql_c_xchng_cd, sql_c_pipe_id);

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"path name for file of msg to bse is :%s:",c_fileName);
		}

    /*** To Write a dump of messages sent to Exchange ***/
    i_ch_val = fn_create_file ( &ptr_snd_msgs_file,
                                c_fileName,
                                "a+",
                                RELATIVE,
                                c_ServiceName,
                                c_errmsg);
    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName, "S31040", (char *)UNXMSG, c_errmsg);
      fn_userlog(c_ServiceName,"Error in fn_create_file for msgs to bse");
      return ( -1 );
    }
  }
  if(DEBUG_MSG_LVL_3)
  {
  fn_userlog(c_ServiceName,"After fn_create_file in fn_bat_init ");
  }



	ptr_c_tmp = (char *)fn_get_prcs_spc ( c_ServiceName, "SKIP_ORD_REFSEQ" );
  if (ptr_c_tmp == NULL )
  {
    fn_errlog(c_ServiceName,"S31045", LIBMSG, c_errmsg);
    return ( -1 );
  }

	strcpy(c_skip_ordref,ptr_c_tmp); 

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Order Ref mentioned to skip in fn_bat_init :%s:",c_skip_ordref);
	}


  /** Kept belwo part of DEFERRED_DOWNLOAD_FLG for future use in trade processing ***/
   ptr_c_tmp = (char *)fn_get_prcs_spc ( c_ServiceName,"DEFERRED_DOWNLOAD_FLG" );
  if (ptr_c_tmp == NULL )
  {
    fn_errlog(c_ServiceName,"S31050", LIBMSG, c_errmsg);
    return ( -1 );
  }

	i_deferred_dwnld_flg = atoi(ptr_c_tmp);
	
	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Deffered Download Flg:%d:",i_deferred_dwnld_flg);
	}

	ptr_c_tmp = (char *)fn_get_prcs_spc ( c_ServiceName, "KEEPALIVE_INTRVL" );
	if (ptr_c_tmp == NULL )
	{
 	 fn_errlog(c_ServiceName,"S31055", LIBMSG, c_errmsg);
	 return ( -1 );
	}
	ui_keep_alive_interval  = atol(ptr_c_tmp);
	if(DEBUG_MSG_LVL_3)
	{
	 fn_userlog(c_ServiceName,"Keep Alive Interval in fn_bat_init is <%d>",ui_keep_alive_interval);
	}

	/**** Initialise socket API parameters ****/
	fn_sck_init();

	/**** Set signal handler for SIGUSR2  ****/

  if ( fn_set_sgn_hnd ( SIGUSR2,
                        fn_sigusr2_hnd,
                        c_ServiceName,
                        c_errmsg   ) == -1 )
  {
    fn_errlog(c_ServiceName,"S31060", LIBMSG, c_errmsg);
    return ( -1 );
  }
  if(DEBUG_MSG_LVL_5)
  {
   fn_userlog(c_ServiceName,"After fn_set_sgn_hnd for sign-2 in bat_init");
  }


	/**** Set signal handler for SIGUSR1  ****/
  if ( fn_set_sgn_hnd ( SIGUSR1,
                        fn_sigusr1_hnd,
                        c_ServiceName,
                        c_errmsg        ) == -1 )
  {
    fn_errlog(c_ServiceName,"S31065", LIBMSG, c_errmsg);
    return ( -1 );
  }
  if(DEBUG_MSG_LVL_5)
  {
   fn_userlog(c_ServiceName,"After fn_set_sgn_hnd for sign-1 in bat_init");
  }


  /**** Set signal handler for SIGALRM  ****/
  /***** Ver 1.2 Starts Here ******************

  if ( fn_set_sgn_hnd ( SIGALRM,
                        fn_signalalarm_hnd,
                        c_ServiceName,
                        c_errmsg        ) == -1 )
  {
    fn_errlog(c_ServiceName,"S31070", LIBMSG, c_errmsg);
    return ( -1 );
  }
  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"After fn_set_sgn_hnd for SIGALRM in bat_init");
  }

  ****** Ver 1.2 Ends Here ********************/
   
  i_trnsctn = fn_begintran( c_ServiceName, c_errmsg );
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"S31075", LIBMSG, c_errmsg);
    return ( -1 );
  }

  EXEC SQL
    UPDATE opm_ord_pipe_mstr
    SET opm_login_stts = 0
    WHERE opm_pipe_id = :sql_c_pipe_id;
  if ( SQLCODE != 0 )
  {
    fn_errlog(c_ServiceName,"S31080", SQLMSG, c_errmsg);
    fn_userlog(c_ServiceName,"Error in updating OPM status to 0 ");
    fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
    return ( -1 );
  }

	i_ch_val = fn_get_reset_seq(sql_c_pipe_id,
					sql_c_nxt_trd_date.arr,
					RESET_PLACED_SEQ,
					&i_snd_seq,
					c_ServiceName,
					c_errmsg);

	
	if ( i_ch_val == -1 )
	{
		fn_errlog(c_ServiceName,"L31020", LIBMSG, c_errmsg);
		fn_userlog(c_ServiceName,"Error in reset sequence ");
		return -1;
	}

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"After update opm status and reset seq :%d: for pipe id %s:",i_snd_seq, sql_c_pipe_id);
	}


  i_ch_val = fn_committran( c_ServiceName, i_trnsctn, c_errmsg );
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"S31085", LIBMSG, c_errmsg);
    return ( -1 );
  }

	fn_var = CLN_BSE_ESR_CLNT;

  return ( 0 );
}

/** #############################################################  **/

int fn_crt_n_flushq ( int *ptr_i_qid,
                      char *ptr_c_qname,
                      char *c_ServiceName,
                      char *c_errmsg)
{
  char *ptr_c_tmp;
  int i_ch_val;
  int i_msgq_typ;

  /***  Get IPC key  ***/
  ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName, ptr_c_qname);
  if ( ptr_c_tmp == NULL )
  {
    fn_errlog(c_ServiceName,"S31090", LIBMSG, c_errmsg);
    fn_userlog(c_ServiceName,"Error in fn_get_prcs_spc for fn_crt_n_flushq to get queue id");
    return -1;
  }
  i_msgq_typ = atoi(ptr_c_tmp);

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"Val of i_msgq_typ -Before calling fn_crt_n_flushq <%d>", i_msgq_typ);
  }

  /***  Create a message queue  ***/

  i_ch_val = fn_crt_msg_q ( ptr_i_qid,
                            i_msgq_typ,
                            CLIENT,
                            c_ServiceName,
                            c_errmsg);
  if ( i_ch_val == -1)
  {
    fn_errlog(c_ServiceName,"S31095", LIBMSG, c_errmsg);
    return -1;
  }
  if(DEBUG_MSG_LVL_3)
  { 
   fn_userlog(c_ServiceName,"Val of ptr_i_qid -After calling  fn_crt_msg_q <%d>", ptr_i_qid);
  } 

  /***  Get Send Message queue id and write all messages to file before flushing it***/
  i_ch_val = fn_write_msgq_file(ptr_i_qid,c_ServiceName,c_errmsg);

  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"S31100", LIBMSG, c_errmsg);
    fn_userlog(c_ServiceName,"Error in write msg to file from queue before flush");
    return ( -1 );
  }

  /*** Flush all messages in the queue  ***/
  i_ch_val = fn_flush_q( *ptr_i_qid,
                          c_ServiceName,
                          c_errmsg);
  if ( i_ch_val == -1)
  {
    fn_errlog(c_ServiceName,"S31105", LIBMSG, c_errmsg);
    fn_userlog(c_ServiceName,"Error in flush queue");
    return -1;
  }

  return 0;
}

/** #############################################################  **/

int fn_write_msgq_file(int *ptr_i_qid,
		       						 char *c_ServiceName,
					 					   char *c_errmsg)
{
 FILE *ptr_queue_msgs_file;

 unsigned int i_queue_cnt = 0;
 int i_ch_val;
 int i_cnt;
 long li_queue_msg_size;

 char c_iml_header;
 char c_fileName[ 200 ];


 /*struct st_req_q_data st_req_data; */
 struct st_send_rqst_data st_send_rq_data; 

 li_queue_msg_size = sizeof(struct st_send_rqst_data);
 c_iml_header = 'Y';

 sprintf(c_fileName, "trade/%s/%s/msgs_sendq_before_flush.txt",sql_c_xchng_cd, sql_c_pipe_id);

 if(DEBUG_MSG_LVL_3)
 {
	fn_userlog(c_ServiceName,"path name of file before flush is :%s:",c_fileName);
 }

	/*** To Write a dump of messages got from Exchange ***/
	i_ch_val = fn_create_file ( &ptr_queue_msgs_file,
				    c_fileName,
				    "a+",
				    RELATIVE,
				    c_ServiceName,
				    c_errmsg);
	if ( i_ch_val == -1 )
	{
		fn_errlog(c_ServiceName, "S31110", (char *)UNXMSG, c_errmsg);
		return ( -1 );
	}

	i_ch_val = fn_get_no_of_msg (*ptr_i_qid, &i_queue_cnt,c_ServiceName,c_errmsg);
	if (i_ch_val == -1)
	{
  	fn_userlog(c_ServiceName,"Error in fn_get_no_of_msg ");
   	fn_errlog(c_ServiceName,"S31115", LIBMSG, c_errmsg);
    return -1;
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"The number of message in Queue before flush and writing to Queue:%d:",i_queue_cnt);
	}

	for(i_cnt=1; i_cnt <= i_queue_cnt; i_cnt++)
	{
    MEMSET(st_send_rq_data);
		if ( fn_read_msg_q (  *ptr_i_qid,&st_send_rq_data,&li_queue_msg_size,c_ServiceName,c_errmsg       ) == -1 )
    {
      fn_errlog(c_ServiceName,"L31300", LIBMSG, c_errmsg);
      fn_userlog(c_ServiceName,"Error in fn_read_msg_q...1 ");

			return -1;
    }

		fn_bfprint_log(  ptr_queue_msgs_file,
									   c_iml_header,
										 (void *)(&st_send_rq_data.st_bsnd_rqst_data),
									 	 c_ServiceName,
									   c_errmsg);
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"After Sucessfully written data from Queue to file:%d:",i_queue_cnt);
	}


  fn_destroy_file(ptr_queue_msgs_file);

	return 0;
}


/** #############################################################  **/

int fn_crt_iml_sck ( int *ptr_i_sck_id, char *ptr_c_iml_addr_nm, char *ptr_c_iml_port_nm, char *c_ServiceName, char *c_errmsg)
{
  char *ptr_c_tmp;
  char c_iml_ip_addr[30];          
  int  i_iml_port_no;                 
	int  i_ch_val;

	ptr_c_tmp = (char *)fn_get_prcs_spc ( c_ServiceName, ptr_c_iml_addr_nm );
  if (ptr_c_tmp == NULL )
  {
    fn_errlog(c_ServiceName,"S31120", LIBMSG, c_errmsg);
    return ( -1 );
  }

	strcpy(c_iml_ip_addr,ptr_c_tmp);


	ptr_c_tmp = (char *)fn_get_prcs_spc ( c_ServiceName, ptr_c_iml_port_nm );
  if (ptr_c_tmp == NULL )
  {
    fn_errlog(c_ServiceName,"S31125", LIBMSG, c_errmsg);
    return ( -1 );
  }

	i_iml_port_no = atoi(ptr_c_tmp);

	/**** Connect to IML Machine****/
  i_ch_val = fn_crt_clnt_sck ( c_iml_ip_addr, 
															 i_iml_port_no, 
														   ptr_i_sck_id, 
													     c_ServiceName, 
														   c_errmsg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"S31130", LIBMSG, c_errmsg);
		return -1;
  }

	if(DEBUG_MSG_LVL_3)
	{
 	 fn_userlog(c_ServiceName,"socket connected in fn_crt_iml_sck with i_iml_port_no <%d>", i_iml_port_no);
	}

	return 0;
}

int fn_prcs_trg ( char *c_pgm_nm, int i_trg_typ )
{
  fn_userlog(c_ServiceName,"In fn_prcs_trg with val of i_trg_typ = <%d>", i_trg_typ);
  return 0;
}


void fn_sigusr2_hnd( int i_signo)
{
  fn_set_break_thrd();
  fn_userlog(c_ServiceName,"In fn_sigusr2_hnd with val of i_signo = <%d>", i_signo);
 	fn_lock_sig_unlock (&cond_thrd_cntrl,
                       &mut_thrd_cntrl,
                       &i_cond_thrd_cntrl,
                       EXIT_BY_SIGUSR2);
  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"In fn_sigusr2_hnd aftr fn_lock_sig_unlock-Val of i_cond_thrd_cntrl:<%d>",i_cond_thrd_cntrl);
  }
}


void fn_sigusr1_hnd( int i_signo)
{

  if(DEBUG_MSG_LVL_3)
  {
  fn_userlog(c_ServiceName,"In fn_sigusr1_hnd with val of i_signo =<%d> and c_thd_typ =<%c>", i_signo, c_thd_typ);
  }
  switch(c_thd_typ)
  {
    case 'R':
      i_rcv_thrd_exit_stts = EXIT;
      i_keep_alive_thrd_exit_stts = EXIT;  /*** Ver 1.2 ***/
      alarm(1);   /** Ver 1.2 ***/
      fn_set_break_sck();  
      fn_userlog(c_ServiceName,"In fn_sigusr1_hnd with casr R");
      break;

    case 'S':
      i_snd_thrd_exit_stts = EXIT;
      i_keep_alive_thrd_exit_stts = EXIT; /** Ver 1.2 ***/
      alarm(1);     /*** Ver 1.2 ***/
      fn_set_break_q();
      fn_set_break_thrd();
      break;

    default:
			if(DEBUG_MSG_LVL_0)
			{
      	fn_userlog("cln_esr_srvr", "Invalid thrd type");
			}
      break;
  }
}

/* ###############################################################################  */

/*************** Ver 1.2 Start Here *********************

void fn_signalalarm_hnd(int signo)
{
	int i_ret_val = 0;
	i_ret_val = fn_send_keepalive();
	if( i_ret_val == -1)
  {
    fn_errlog(c_ServiceName, "S31135", UNXMSG, c_errmsg);
    fn_userlog(c_ServiceName, " Error while sending keep alive packet to iml");
    return;
  }
  
  alarm(ui_keep_alive_interval);
  if(DEBUG_MSG_LVL_3)
  {
  fn_userlog(c_ServiceName, " Alarm is reset after sending keep alive message with i_ret_val :%d:",i_ret_val);
  }

}
************* Ver 1.2 ends Here ***********************/


/* ###############################################################################  */

int fn_send_keepalive()
{
	char c_msg_cbuf[MAX_SCK_MSG];
  /*commented in ver 1.6 struct st_keep_alive st_var_keep_alive; ***/
	HeartbeatT st_var_keep_alive;  /** Ver 1.6 **/
  long l_keep_alive;
  int i_ret_val = 0;
  int i_pckt_len;
  int i_ch_value = 0;

	MEMSET(st_var_keep_alive);

 	/*** ver1.6  commented start
	st_var_keep_alive.l_msg_typ = htonl(KEEP_ALIVE);

  st_var_keep_alive.st_hdr.l_slot_no = htonl(-1);
  st_var_keep_alive.st_hdr.l_message_length = htonl(sizeof(st_var_keep_alive) - sizeof(st_var_keep_alive.st_hdr));

  st_var_keep_alive.l_msg_typ = fn_swap_long(st_var_keep_alive.l_msg_typ);
  st_var_keep_alive.st_hdr.l_slot_no = fn_swap_long(st_var_keep_alive.st_hdr.l_slot_no);
  st_var_keep_alive.st_hdr.l_message_length = fn_swap_long(st_var_keep_alive.st_hdr.l_message_length);

	MEMSET(c_msg_cbuf);
  memcpy((char *)c_msg_cbuf, &st_var_keep_alive, sizeof(HeartbeatT));
  i_pckt_len = sizeof(struct st_var_keep_alive);	
	ver 1.6 comment end ***/

	/* Added in ver1.6 -Starts */
  st_var_keep_alive.MessageHeaderIn.BodyLen = htonl(sizeof(HeartbeatT));
  st_var_keep_alive.MessageHeaderIn.TemplateID = htonl(TID_HEARTBEAT);
  strcpy(st_var_keep_alive.MessageHeaderIn.NetworkMsgID,NO_VALUE_STR);
  strcpy(st_var_keep_alive.MessageHeaderIn.Pad2,NO_VALUE_STR);

  st_var_keep_alive.MessageHeaderIn.BodyLen = fn_swap_ulong(st_var_keep_alive.MessageHeaderIn.BodyLen);
  st_var_keep_alive.MessageHeaderIn.TemplateID = fn_swap_ushort(st_var_keep_alive.MessageHeaderIn.TemplateID);

  MEMSET(c_msg_cbuf);
  memcpy((char *)c_msg_cbuf, &st_var_keep_alive, sizeof(HeartbeatT));
  i_pckt_len = sizeof(HeartbeatT);
  /* Added in ver 1.6 -End */

	/*** pthread_mutex_lock ( &mut_snd_rcv_cntrl);     ***  Ver 1.1 *** Commented in Ver 1.2 ***/

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"Val of i_pckt_len before fn_send for keep_alive <%d>",i_pckt_len);
  }

  i_ch_value = fn_writen(i_iml_sck_id,
		      						   (char *)c_msg_cbuf,
		      						   i_pckt_len,
		      							 c_ServiceName,
		      							 c_errmsg);
 
  
  if( i_ch_value == -1) 
  {
    fn_userlog(c_ServiceName,"Error while sending keep alive packet to iml");
    fn_errlog(c_ServiceName, "L31070", UNXMSG, c_errmsg);
    return -1;
  }

  /******** Ver 1.2  Starts Here ****************
  pthread_mutex_unlock (&mut_snd_rcv_cntrl );     ***  Ver 1.1 ***

  fn_lock_sig_unlock ( &cond_thrd_cntrl,          ***  Ver 1.1 ***
                       &mut_thrd_cntrl,
                       &i_cond_thrd_cntrl,
                       EXIT_BY_SND_THRD );
  ********* Ver 1.2 Ends Here ******************/

  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"After success of fn_send for keep_alive ");
  }	
  return 0;
}

/* ###############################################################################  */


/******************************************************************************/
/*  Description   :  The send thread waits on the message queue.If any message*/
/*                   is received,it sends the message on IML socket. 					*/
/*								   In case of error while processing its normal       			*/
/*                   operation, it will set the conditional variable of the   */
/*                   thread control to EXIT_BY_SND_THRD and returns to parent */
/*                   thread function                                          */
/******************************************************************************/
void* fn_bse_snd_thrd( void )
{
  int i_ch_val;
  char c_ServiceName[33];
  char c_errmsg[256];
	char c_iml_header;

	long li_queue_msg_size;
	long li_business_data_size;
	long li_send_iml_msg_size;
  long l_snd_msg_type;
  int  i_trnsctn = 0; 
  char c_ordr_dwnld_snd_flg;

	struct st_bfo_usr_rgstrn_req  st_bfo_rgstrn_req; /* Added to send only regstr struct in send thrd   */

	struct st_bfo_logon_req st_blogon_req;       /* Added for log purpose -   */
	struct st_bfo_upd_passwd_req st_bupd_passwd_req; /* Added for log purpose -   */
	struct st_ord_rqst st_lmt_mrkt_ord_rqst;  /* Added for log purpose -   */
	struct st_stoploss_ord_rqst st_sltp_ord_rqst;  /* Added for log purpose -   */
	struct st_ioc_ordr_rqst st_ioc_order_rqst;     /* Added for log purpose -   */
	struct st_prsnl_dwnld st_prsnl_dwnld_req;        /* Added for log purpose -   */

	struct st_send_rqst_data st_send_rq_data;
	union st_snd_rqst_data st_bsnd_rqst_data;

	sigset_t new;

  sigemptyset(&new);
  sigaddset(&new, SIGUSR2);
  pthread_sigmask(SIG_BLOCK, &new, NULL);

  /********** Ver 1.2 ************
  sigemptyset(&new);
  sigaddset(&new, SIGALRM);
  pthread_sigmask(SIG_BLOCK, &new, NULL);
  ********** Ver 1.2 *************/

  strcpy(c_ServiceName, "fn_bse_snd_thrd");

	/** setting the thread status to don't exit **/
  i_snd_thrd_exit_stts = DONT_EXIT;

	if(DEBUG_MSG_LVL_5)
	{
  	fn_userlog ( c_ServiceName, "Before Entering while") ;	
	}

  while ( i_snd_thrd_exit_stts == DONT_EXIT)
  {     /** SND_THRD is in DONT_EXIT state loop start **/

		MEMSET(st_send_rq_data);
		MEMSET(st_bsnd_rqst_data);

    li_queue_msg_size = sizeof(struct st_send_rqst_data);

    if(DEBUG_MSG_LVL_5)
    {
    fn_userlog(c_ServiceName,"Before reading from Send Queue in bse_esr with i_snd_qid as : <%d>", i_snd_qid);
    }
   
	/** COmmented in Ver 1.8  
    if ( fn_read_msg_q ( i_snd_qid,
                         (void *)c_snd_msg_data,
                         &li_queue_msg_size,
                         c_ServiceName,
                         c_errmsg       ) == -1 )
    {
      fn_errlog(c_ServiceName,"L31300", LIBMSG, c_errmsg);
      fn_userlog(c_ServiceName,"Error in reading Msg Q in send thrd initially ---1");
      pthread_mutex_unlock ( &mut_snd_rcv_cntrl);
			i_snd_thrd_exit_stts = EXIT;
			continue;	
    }	COmmented in Ver 1.8 **/

		 if ( fn_read_msg_q ( i_snd_qid,
                         &st_send_rq_data,
                         &li_queue_msg_size,
                         c_ServiceName,
                         c_errmsg       ) == -1 )
    {
      fn_errlog(c_ServiceName,"L31300", LIBMSG, c_errmsg);
      fn_userlog(c_ServiceName,"Error in reading Msg Q in send thrd initially ---1");
      pthread_mutex_unlock ( &mut_snd_rcv_cntrl);
      i_snd_thrd_exit_stts = EXIT;
      continue;
    }

    /*memcpy(&l_snd_msg_type, c_snd_msg_data, sizeof(long)); COmmented in Ver 1.8 **/
			/* l_snd_msg_type=st_send_rq_data.li_msg_type;i */	/** Ver 1.8 **/
			memcpy(&l_snd_msg_type, &st_send_rq_data.li_msg_type, sizeof(long));	
    
    if(DEBUG_MSG_LVL_3)
    {
    fn_userlog ( c_ServiceName, "Message Type Received from MSG Q in send thrd initially:%ld:",l_snd_msg_type);
    }

		pthread_mutex_lock ( &mut_snd_rcv_cntrl);

		i_cond_snd_rcv_cntrl = 0;
		
		memcpy(&st_bsnd_rqst_data,&st_send_rq_data.st_bsnd_rqst_data,sizeof(st_bsnd_rqst_data));


		if(l_snd_msg_type == USR_RGSTRN )
		{
				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"In if condition of USR_RGSTRN");
				}

			/* Added below part to send only registration req part */		

          memset(&st_bfo_rgstrn_req, 0, sizeof(st_bfo_rgstrn_req));
					/* memcpy(&st_bfo_rgstrn_req,c_snd_msg_data + sizeof(long) , sizeof(st_bfo_rgstrn_req)); COmmented in Ver 1.8 **/
					memcpy(&st_bfo_rgstrn_req,&st_send_rq_data.st_bsnd_rqst_data, sizeof(st_bfo_rgstrn_req));	/** Ver 1.8 **/

          /**  Ver 1.6 Start 
				if(DEBUG_MSG_LVL_5)
				{
				fn_userlog(c_ServiceName,"BSE ORS l_message_length -Before Sending to fn_do_bse_logon IS :%ld:",fn_swap_long(st_bfo_rgstrn_req.st_hdr.l_message_length));
				fn_userlog(c_ServiceName,"BSE ORS l_slot_no -Before Sending to fn_do_bse_logon IS :%ld:",fn_swap_long(st_bfo_rgstrn_req.st_hdr.l_slot_no));

				fn_userlog(c_ServiceName,"BSE ORS l_msg_typ -Before Sending to fn_do_bse_logon IS :%ld:",fn_swap_long(st_bfo_rgstrn_req.l_msg_typ));  
				fn_userlog(c_ServiceName,"BSE ORS l_slot_no -Before Sending to fn_do_bse_logon IS :%ld:",       fn_swap_long(st_bfo_rgstrn_req.l_slot_no));    
				fn_userlog(c_ServiceName,"BSE ORS l_member_id -Before Sending to fn_do_bse_logon IS :%ld:",fn_swap_long(st_bfo_rgstrn_req.l_member_id));  
				fn_userlog(c_ServiceName,"BSE ORS l_trader_id -Before Sending to fn_do_bse_logon IS :%ld:",       fn_swap_long(st_bfo_rgstrn_req.l_trader_id));  
				}
				Ver 1.6 End ***/

				/** Ver 1.6 Start debug **/
				fn_userlog(c_ServiceName,"User registration Request TemplateId:%d: BodyLen:%ld: l_trader_id :%ld: l_member_id :%ld:",st_bfo_rgstrn_req.TemplateID,st_bfo_rgstrn_req.BodyLen,st_bfo_rgstrn_req.l_trader_id,st_bfo_rgstrn_req.l_member_id);
				/** Ver 1.6 End debug **/

				i_ch_val = fn_do_bse_logon( &(st_bfo_rgstrn_req),
																			ptr_snd_msgs_file,
																			c_ServiceName,
																			c_errmsg       );

			  if(DEBUG_MSG_LVL_3)
			  {
			  fn_userlog(c_ServiceName,"After fn_do_bse_logon in USR_RGSTRN of snd_thrd - ERROR_MSG=<%s>", c_errmsg);
			  }
				if ( i_ch_val == -1 )
				{
					fn_errlog(c_ServiceName,"L31300", LIBMSG, c_errmsg);
					pthread_mutex_unlock ( &mut_snd_rcv_cntrl);
					i_snd_thrd_exit_stts = EXIT;
					continue;
				}
		}

/*******************************************************************************/
/***    User Registration Check Complete   *************************************/
/********************************************************************************/

  /************* Generalised Message writting on socket for all message types start  **************/	
		else 
		{          /*** Else started for All types other than Register **/
		switch(l_snd_msg_type)
		{  /** Switch case Starts in send thread for all other types than Register */
		  
		  case LOGON_REQ:  /** 1131 **/


			    if(DEBUG_MSG_LVL_3)
			    {
			    fn_userlog(c_ServiceName,"In switch condition of LOGON_REQ in Send thread");
			    }
			  li_send_iml_msg_size =  sizeof(UserLoginRequestT);		/* Ver 1.6 */

				if(DEBUG_MSG_LVL_5)
				{
					 fn_userlog(c_ServiceName," LOGON_REQ BodyLen:%d: TemplateID:%d: NetworkMsgID:%s: c_password:%s: SenderSubID:%d: MsgSeqNum :%d:",st_bsnd_rqst_data.st_bsigon.MessageHeaderIn.BodyLen,st_bsnd_rqst_data.st_bsigon.MessageHeaderIn.TemplateID,st_bsnd_rqst_data.st_bsigon.MessageHeaderIn.NetworkMsgID,st_bsnd_rqst_data.st_bsigon.Password,st_bsnd_rqst_data.st_bsigon.RequestHeader.SenderSubID,st_bsnd_rqst_data.st_bsigon.RequestHeader.MsgSeqNum);

				}
				
		  break;

		  case LOGOFF_REQ:   /* 1132 **/ 

			    if(DEBUG_MSG_LVL_3)
			    {
			      fn_userlog(c_ServiceName,"In if condition of LOGOFF_REQ in Send thread");
			    }
		            li_send_iml_msg_size = sizeof(struct st_bfo_logon_req);

			    if(DEBUG_MSG_LVL_3)
			    {
			     fn_userlog(c_ServiceName,"Val of li_business_data_size for LOGOFF_REQ before writting to socket: <%ld>", li_business_data_size);
					
			    } 
	
		  break;

		  case MANDATORY_PASSWD_CHNG:  /** 1133 **/
		  case OPTNL_PASSWD_CHNG:      /** 1134 **/

			    if(DEBUG_MSG_LVL_3)
			    {
			      fn_userlog(c_ServiceName,"In if condition of PASSWD_CHNG in Send thread");
			    }
			    li_send_iml_msg_size = sizeof(struct st_bfo_upd_passwd_req );

			    if(DEBUG_MSG_LVL_3)
			    {
			     fn_userlog(c_ServiceName,"Val of li_business_data_size for PASSWD_CHNG before writting to socket: <%ld>", li_business_data_size);
			      
			      fn_userlog(c_ServiceName,"val of l_slot_no in MANDATORY_PASSWD_CHNG =<%ld>", st_bupd_passwd_req.st_hdr.l_slot_no);
			      fn_userlog(c_ServiceName,"val of l_message_length in MANDATORY_PASSWD_CHNG =<%ld>", st_bupd_passwd_req.st_hdr.l_message_length);
			      fn_userlog(c_ServiceName,"val of l_msg_typ in MANDATORY_PASSWD_CHNG =<%ld>", st_bupd_passwd_req.l_msg_typ);
			      fn_userlog(c_ServiceName,"val of c_old_pwd in MANDATORY_PASSWD_CHNG =<%s>", st_bupd_passwd_req.c_old_pwd);
			      fn_userlog(c_ServiceName,"val of c_new_pwd in MANDATORY_PASSWD_CHNG =<%s>", st_bupd_passwd_req.c_new_pwd);
			      fn_userlog(c_ServiceName,"val of l_msg_tag in MANDATORY_PASSWD_CHNG =<%ld>", st_bupd_passwd_req.l_msg_tag);
			      
			    }
		  break;

		  case PRSNL_ORDR_DWNLD:  /** 1092 **/
		  case PRSNL_TRD_DWNLD:   /** 1095 **/
		  case PRSN_SLTP_ORDR_DWNLD: /** 1097 **/
			    if(DEBUG_MSG_LVL_3)
			    {
			      fn_userlog(c_ServiceName,"In if condition of ORDER/TRADE/SLTP DWNLD in Send thread");
			    }

			    if (l_snd_msg_type == PRSNL_ORDR_DWNLD)
			    {
			      c_ordr_dwnld_snd_flg ='Y';
			      c_delete_dwld_flg == 'N';
			      fn_userlog(c_ServiceName,"After setting c_ordr_dwnld_snd_flg to 'Y' for ORD_DWLD");

		/***********************************************************************************/
			/** Added delete part for download table Start- 14-Dec-2011 */
		/***********************************************************************************/

			c_ord_dw_cmplt =  'N';
			c_trd_dw_cmplt =  'N';
			c_sltp_dw_cmplt = 'N';

			if(DEBUG_MSG_LVL_3)
			{
			fn_userlog(c_ServiceName,"Initialising dw_cmplt flag to 'N' when download req sent ");
			}

			 if(c_delete_dwld_flg == 'N')
			 {
			  i_trnsctn = fn_begintran( c_ServiceName, c_errmsg );
			  if ( i_trnsctn == -1)
			  {
			    fn_errlog(c_ServiceName,"S31140", LIBMSG, c_errmsg);
    			    fn_userlog(c_ServiceName,"Error in transaction begin-ESR clnt for dwnld table -delete");
			    break;
			  }
				EXEC SQL
					DELETE FROM FXD_FO_XCHNG_DWNLD
					WHERE FXD_PIPE_ID = :sql_c_pipe_id;
				if ( SQLCODE != 0  && SQLCODE != NO_DATA_FOUND)
			{
			fn_errlog(c_ServiceName,"S31145", SQLMSG, c_errmsg);
			fn_userlog(c_ServiceName,"Error -Deleting data in FXD table for trade dwld for :%s:",sql_c_pipe_id);
			break;
			}
			fn_userlog(c_ServiceName,"After deleting from FXD for pipe id<%s>", sql_c_pipe_id);

				EXEC SQL
					DELETE FROM FXD_FO_XCHNG_TRD_DWNLD
					WHERE FXD_PIPE_ID = :sql_c_pipe_id;
				if ( SQLCODE != 0  && SQLCODE != NO_DATA_FOUND)
			{
			fn_errlog(c_ServiceName,"S31150", SQLMSG, c_errmsg);
			fn_userlog(c_ServiceName,"Error -Deleting data in FXD_TRD table for trade dwld for :%s:",sql_c_pipe_id);
			break;
			}
			fn_userlog(c_ServiceName,"After deleting from FXD_TRD for pipe id<%s>", sql_c_pipe_id);

			  i_ch_val = fn_committran( c_ServiceName, i_trnsctn, c_errmsg );
			  if ( i_ch_val == -1 )
			  {
			    fn_errlog(c_ServiceName,"S31155", LIBMSG, c_errmsg);
			    fn_userlog(c_ServiceName,"Error in commit transaction begin-ESR clnt for dwnld table -delete");
			    break;
			  }

			  c_delete_dwld_flg = 'Y';
			  fn_userlog(c_ServiceName,"Val of c_delete_dwld_flg set to 'Y' for dwnld table -delete");
			  }
		/***********************************************************************************/
			/** Added delete part for download table End- 14-Dec-2011 */
		/***********************************************************************************/

			    }   
			    
			    /* commented in Ver 1.6 li_send_iml_msg_size = sizeof(struct st_prsnl_dwnld ); */

          /* Ver 1.6 starts */
					 if(l_snd_msg_type == PRSNL_ORDR_DWNLD)
           {
			    		li_send_iml_msg_size = sizeof(RetransmitMEMessageRequestT);  /* changed to RetransmitMEMessageRequestT in Ver 1.7 */ 
              i_partition_id = fn_swap_short(st_send_rq_data.st_bsnd_rqst_data.st_dwnld.PartitionID); /** ver 1.7 **/
 					 }
           if(l_snd_msg_type == PRSNL_TRD_DWNLD)
			     {
          		li_send_iml_msg_size = sizeof(RetransmitRequestT);   /* chnaged to RetransmitRequestT in Ver 1.7 */
              i_partition_id = fn_swap_short(st_send_rq_data.st_bsnd_rqst_data.st_trddwnld.PartitionID);  /** Ver 1.7 **/
				   }
          /* Ver 1.6 ends */

			    if(DEBUG_MSG_LVL_5)
			    {
			     fn_userlog(c_ServiceName,"Val of li_business_data_size for PRSNL_ORDR_DWNLD/PRSNL_TRD_DWNLD before writting to socket: <%ld>", li_business_data_size);

				  fn_userlog(c_ServiceName,"val of li_send_iml_msg_size of DOWNLOAD RQST in snd_thrd =<%ld>", li_send_iml_msg_size);
				  /* commented in Ver 1.6 fn_userlog(c_ServiceName,"Val of st_prsnl_dwnld_req.l_msg_typ in snd_thrd:%ld:",st_prsnl_dwnld_req.l_msg_typ);
				  fn_userlog(c_ServiceName,"Val of st_prsnl_dwnld_req.l_msg_tag in snd_thrd:%ld:",st_prsnl_dwnld_req.l_msg_tag); */
			    }
		  break;

		  case LIMIT_MARKET_ORD:  /** 1025 **/

			    if(DEBUG_MSG_LVL_3)
			    {
			      fn_userlog(c_ServiceName,"In if condition of LIMIT_MARKET_ORD in Send thread");
			    }
			    /* li_send_iml_msg_size = sizeof(struct st_ord_rqst ); ver 1.6 **/
    
          /* Ver 1.6 starts */
					if(fn_swap_short(st_bsnd_rqst_data.st_eti_order_req.MessageHeaderIn.TemplateID) == TID_NEW_ORDER_SINGLE_REQUEST)
						li_send_iml_msg_size=sizeof(NewOrderSingleRequestT);
					else if(fn_swap_short(st_bsnd_rqst_data.st_eti_order_req.MessageHeaderIn.TemplateID)==TID_MODIFY_ORDER_SINGLE_REQUEST)
						li_send_iml_msg_size=sizeof(ModifyOrderSingleRequestT);
					else if(fn_swap_short(st_bsnd_rqst_data.st_eti_order_req.MessageHeaderIn.TemplateID)==TID_DELETE_ORDER_SINGLE_REQUEST)
						li_send_iml_msg_size=sizeof(DeleteOrderSingleRequestT);
		      /* Ver 1.6 ends */

			    if(DEBUG_MSG_LVL_5)
			    {
			     fn_userlog(c_ServiceName,"Val of li_business_data_size for LIMIT_MARKET_ORD before writting to socket: <%ld>", li_business_data_size);
          /* Ver 1.6 starts */
				  fn_userlog(c_ServiceName,"Val of TemplateID for Order Data before writting to socket: <%ld>", st_bsnd_rqst_data.st_eti_order_req.MessageHeaderIn.TemplateID);
				  fn_userlog(c_ServiceName,"Val of SimpleSecurityID before writting to socket: <%ld>", st_bsnd_rqst_data.st_eti_order_req.SimpleSecurityID);
				  fn_userlog(c_ServiceName,"Val of FreeText1  before writting to socket: <%ld>", st_bsnd_rqst_data.st_eti_order_req.FreeText1);
				  fn_userlog(c_ServiceName,"Val of ClOrdID    before writting to socket: <%ld>", st_bsnd_rqst_data.st_eti_order_req.ClOrdID);
				  fn_userlog(c_ServiceName,"Val of FreeText3  before writting to socket: <%ld>", st_bsnd_rqst_data.st_eti_order_req.FreeText3);
          /* Ver 1.6 ends */
					
			    }
		break;	
		/*******************************************************/


		  case	ADD_STOPLOSS_BUY_ORD:   /** 2501 **/
		  case 	UPD_STOPLOSS_BUY_ORD:   /** 2502 **/
		  case 	ADD_STOPLOSS_SELL_ORD:  /** 2504 **/
		  case	UPD_STOPLOSS_SELL_ORD:  /** 2505 **/

			    if(DEBUG_MSG_LVL_3)
			    {
			      fn_userlog(c_ServiceName,"In if condition of STOPLOSS Order in Send thread");
			    }
			    li_send_iml_msg_size = sizeof(struct st_stoploss_ord_rqst );

			    if(DEBUG_MSG_LVL_3)
			    {
			     fn_userlog(c_ServiceName,"Val of li_business_data_size for STOPLOSS before writting to socket: <%ld>", li_business_data_size);
				  fn_userlog(c_ServiceName,"Val of l_msg_typ for for STOPLOSS Order before writting to socket: <%ld>", st_sltp_ord_rqst.l_msg_typ);
				  fn_userlog(c_ServiceName,"Val of l_scrip_cd for for STOPLOSS Order before writting to socket: <%ld>", st_sltp_ord_rqst.l_scrip_cd);
				  fn_userlog(c_ServiceName,"Val of l_qty for for STOPLOSS Order before writting to socket: <%ld>", st_sltp_ord_rqst.l_qty);
				  fn_userlog(c_ServiceName,"Val of l_avaqty for for STOPLOSS Order before writting to socket: <%ld>", st_sltp_ord_rqst.l_avaqty);
				  fn_userlog(c_ServiceName,"Val of l_sltp_rate for for STOPLOSS Order before writting to socket: <%ld>", st_sltp_ord_rqst.l_sltp_rate);
				  fn_userlog(c_ServiceName,"Val of l_limit_rate for for STOPLOSS Order before writting to socket: <%ld>", st_sltp_ord_rqst.l_limit_rate);
				  fn_userlog(c_ServiceName,"Val of ll_ordr_id for for STOPLOSS Order before writting to socket: <%lld>", st_sltp_ord_rqst.ll_ordr_id); 
			    }
		break;
    case UPD_SLTP_BUY_AFTER_TRG:
    case UPD_SLTP_SELL_AFTER_TRG:
         li_send_iml_msg_size = sizeof(struct st_upd_sltp_rqst);
    
     break;
		/**************************************************************/
		  case ADD_IOC_BUY_ORDR:  /** 1051 **/
		  case ADD_IOC_SELL_ORDR: /** 1061 **/

			    if(DEBUG_MSG_LVL_3)
			    {
			      fn_userlog(c_ServiceName,"In if condition of IOC_REQ in Send thread");
			    }
			    li_send_iml_msg_size = sizeof(struct st_ioc_ordr_rqst );

			    if(DEBUG_MSG_LVL_5)
			    {
			     fn_userlog(c_ServiceName,"Val of li_business_data_size for ADD_IOC before writting to socket: <%ld>", li_business_data_size);
				  fn_userlog(c_ServiceName,"Val of l_msg_typ for Order Data before writting to socket: <%ld>", st_ioc_order_rqst.l_msg_typ);
				  fn_userlog(c_ServiceName,"Val of l_scrip_cd for Order Data before writting to socket: <%ld>", st_ioc_order_rqst.l_scrip_cd);
				  fn_userlog(c_ServiceName,"Val of l_qty for Order Data before writting to socket: <%ld>", st_ioc_order_rqst.l_qty);
				  fn_userlog(c_ServiceName,"Val of l_avaqty for Order Data before writting to socket: <%ld>", st_ioc_order_rqst.l_avaqty);
				  fn_userlog(c_ServiceName,"Val of l_rate for Order Data before writting to socket: <%ld>", st_ioc_order_rqst.l_rate);
				  fn_userlog(c_ServiceName,"Val of ll_ordr_id for Order Data before writting to socket: <%ld>", st_ioc_order_rqst.ll_ordr_id); 
			    }
		break;
		/****************************************************************/	
    
    case OPN_INTRST_RQST : /** 1149 **/
      if(DEBUG_MSG_LVL_3)
          {
            fn_userlog(c_ServiceName,"In if condition of OPN_INTRST_RQST in Send thread");
          }
          li_send_iml_msg_size = sizeof(struct st_opn_intrst_rqst );
    
      break;

     case DEL_STOPLOSS_BUY_ORD:
     case DEL_STOPLOSS_SELL_ORD:
     case DEL_SLTP_BUY_AFTER_TRIGGER:
     case DEL_SLTP_SELL_AFTER_TRIGGER:
         li_send_iml_msg_size = sizeof(struct st_del_ord_rqst);
           
     break;  
    	/***********************************************************/

		}   /** Switch case ends in send thread for all other types than Register */

		 if(i_snd_thrd_exit_stts == EXIT)			/**This is for Invalid message error handling **/
		 {
		   continue;
		 }

		
		 if(DEBUG_MSG_LVL_5)   /** Ver 1.6 **/
		 {
		 fn_userlog(c_ServiceName,"Before call to fn_writen Val of i_iml_sck_id <%d> and li_send_iml_msg_size <%ld> ", i_iml_sck_id, li_send_iml_msg_size);
		 }

		 /******Write order packet to IML socket **/

		 i_ch_val = fn_writen(i_iml_sck_id,
				     (void *)(&st_bsnd_rqst_data),
				      li_send_iml_msg_size,
				      c_ServiceName,
				      c_errmsg);
		 if ( i_ch_val == -1 )
		 {
			fn_errlog(c_ServiceName,"L31020", LIBMSG, c_errmsg);
			pthread_mutex_unlock (&mut_snd_rcv_cntrl );
			i_snd_thrd_exit_stts = EXIT;
			continue;
		 }	

		 if(DEBUG_MSG_LVL_5)
		 {
		 fn_userlog(c_ServiceName,"After call to fn_writen Val of i_iml_sck_id <%d> and c_errmsg <%s> ", i_iml_sck_id, c_errmsg);
		 }

		  if ( i_msg_dump_flg == LOG_DUMP )
		  {
		    /**call function to write data to file **/
				c_iml_header = 'Y';

				fn_bfprint_log(	ptr_snd_msgs_file,
						c_iml_header,
						(void *)(&st_bsnd_rqst_data),
						c_ServiceName,
						c_errmsg);
						/* 				(void *)(&st_send_rq_data.st_bsnd_rqst_data), */

		  if(DEBUG_MSG_LVL_5)
		  {
		  fn_userlog(c_ServiceName,"After call to fn_bfprint_log in  .>Val of c_err_msg <%s> ",  c_errmsg);
		  }

		  }
     }	/*** Else ended for All other types than Register**/
/***************************************************************************************/
/***    User Login Check Complete   *************************************/
/********************************************************************************/


		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName, " Message - |%ld| Successfully sent to IML machine",st_send_rq_data.li_msg_type);
		}

		pthread_mutex_unlock (&mut_snd_rcv_cntrl );
				
	}   /** SND_THRD is in DONT_EXIT state loop End **/

		fn_lock_sig_unlock ( &cond_thrd_cntrl,
                       	 &mut_thrd_cntrl,
                       	 &i_cond_thrd_cntrl,
                       	 EXIT_BY_SND_THRD );

		if(DEBUG_MSG_LVL_5)
		{
		fn_userlog(c_ServiceName, "After fn_lock_sig_unlock in snd_thrd-Val of i_cond_thrd_cntrl <%d> ", i_cond_thrd_cntrl);
		}

}

/* ###############################################################################  */

/******************************************************************************/
/*  Description   :  The rcv thread waits on the IML socket.If any message is */
/*                   received,it processes resp by calling sfo_rcv_clnt.	 */ 
/*								   In case of error while processing its normal       			*/
/*                   operation, it will set the conditional variable of the   */
/*                   thread control to EXIT_BY_RCV_THRD and returns to parent */
/*                   thread function                                          */
/******************************************************************************/


void* fn_bse_rcv_thrd( void )
{
  int i_ch_val;
	int i_break_stts;
	int i_err[TOTAL_FML];
	int i_ferr[TOTAL_FML];
	int i_call_rcv = YES;
	int i_read_size;
	int i_ptr_size=0;        /* Ver 1.6 */
	int i_loop;
	int i_dwnld_flg;
	int i_brk_stts = DONT_EXIT;

	long int li_len_buf;
	long int li_stream_cnt	=	0;
	long l_msg_type;
	long 

	long l_dw_msg_typ; 
	long l_dw_slot_no;
	
	char c_pgm_name[33];
	char c_cmd_str[100];
	int  i_trnsctn = 0; 

  char c_ServiceName[33];
  char c_errmsg[256];
  char c_err_msg[256];
	char c_iml_header;
	/*char c_ptr_data[MAX_SCK_MSG];       */
	char c_ptr_data[2048];      
	char c_data[100]="===================================================";   /** Ver 1.6 **/
	char c_bph_rgstrn_lgn_flg;
	char c_pgm_nm [ 256 ];

	FBFR32    *ptr_fml_Sbuf;
	sigset_t new;

	struct st_comm_header    st_comm_hdr;   		/* Ver 1.6 */
	char arr[16] = {0x00};                   /* Ver 1.6 */
	short s_trd_partition_id = 1;            /* Ver 1.6 */
	short s_ord_partition_id = 1;            /* Ver 1.6 */
	short s_max_partition_id = 0;         	 /* Ver 1.6 */
	LONG  l_login_reject = -1;               /* Ver 1.6 */
  int i_dwnld_flag = 0;                    /* Ver 1.7 */
  int i_orddwnld_cntr = 0;                    /* Ver 1.7 */
  int i_trddwnld_cntr = 0;                    /* Ver 1.7 */
  int i_total_msgs = 0;                    /* Ver 1.7 */
  int i = 0;                               /* Ver 1.7 */
  char c_RefApplLastSeqNum_hex[32+1] ="\0";/* Ver 1.7 */
  char c_EndApplSeqNum_hex[32+1]  = "\0";  /* Ver 1.7 */
  char *c_data1;                           /* Ver 1.7 */


  uint64_t ll_RefApplLastSeqNum=0; /** Ver 1.7 **/
  uint64_t ll_append_seq_num=0;    /** Ver 1.7 **/
  char c_RefApplLastSeqNum[16]="\0";         /** ver 1.7 **/
  char c_EndApplSeqNum[16]="\0";             /** ver 1.7 **/
  char c_EndAppllastbgn[16]="\0";             /** ver 1.7 **/

	memset(c_EndAppllastbgn,0,16);							/** ver 1.7 **/

	union st_rcv_reply_data *st_buf_dat;  
	union st_rcv_reply_data tmp_buf_dat;

	struct st_bfo_rgstrn_reply  st_bse_rgstrn_reply;
	struct st_ord_rqst_rply st_ord_rqst_rply_data;

	/* Allocate the buffers */

  ptr_fml_Sbuf  = (FBFR32 *)tpalloc("FML32", NULL, 25*MAX_SCK_MSG );	/** changed to 5 from 2 in ver 1.6 **/
  if ( ptr_fml_Sbuf == NULL )
  {
    fn_errlog(c_ServiceName, "S31160", UNXMSG, c_errmsg);
		i_cond_thrd_cntrl = EXIT_BY_RCV_THRD;
		pthread_cond_signal(&cond_thrd_cntrl);
  	fn_userlog ( c_ServiceName, "Val of i_cond_thrd_cntrl in error: <%d> ", i_cond_thrd_cntrl);
  }

  sigemptyset(&new);
  sigaddset(&new, SIGUSR2);
  pthread_sigmask(SIG_BLOCK, &new, NULL);

  /****** Ver 1.2 Starte Here ************
  sigemptyset(&new);
  sigaddset(&new, SIGALRM);
  pthread_sigmask(SIG_BLOCK, &new, NULL);
  ******* Ver 1.2 Ends Here *************/

 	strcpy(c_ServiceName, "fn_bse_rcv_thrd");

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog ( c_ServiceName, "Receive thread started ");
	}

  /** set the receive thread's exit status to DONT_EXIT **/
  i_rcv_thrd_exit_stts = DONT_EXIT;

 	if(DEBUG_MSG_LVL_5)
	{
	fn_userlog ( c_ServiceName, "Val of i_rcv_thrd_exit_stts in rcv thrd <%d> ", i_rcv_thrd_exit_stts);
	}
	while (i_rcv_thrd_exit_stts == DONT_EXIT)
  {  /** loop for i_rcv_thrd_exit_stts == DONT_EXIT starts **/

  	fn_userlog ( c_ServiceName, "in DONT_EXIT state of rcv thrd ");
		i_dwnld_flg = NOT_DOWNLOAD;

	  li_len_buf = sizeof(union st_rcv_reply_data); 

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Before calling fn_read_bse_sck: Val of i_iml_sck_id=<%d>", i_iml_sck_id);
		}

		memset ( &tmp_buf_dat, '\0', sizeof(tmp_buf_dat) );
		/*memset( c_ptr_data, '\0', MAX_SCK_MSG );*/
		memset( c_ptr_data, '\0', 2048 );
		MEMSET(st_comm_hdr);        /* Ver 1.6 */

    if (fn_read_bse_sck(i_iml_sck_id, c_ptr_data, 0, c_ServiceName, c_errmsg) != 0)
    {
      fn_errlog(c_ServiceName,"S31165", LIBMSG, c_errmsg);
      i_rcv_thrd_exit_stts = EXIT;
      fn_userlog(c_ServiceName,"ERROR in calling fn_read_bse_sck: Val of c_errmsg=<%s>", c_errmsg);
      continue;
    }

    if(DEBUG_MSG_LVL_3)
    {
    fn_userlog(c_ServiceName,"After calling fn_read_bse_sck: Val of c_errmsg=<%s>", c_errmsg);
    }
		pthread_mutex_lock (&mut_snd_rcv_cntrl );

		/**call service for writing log **/
		
		if ( i_msg_dump_flg == LOG_DUMP )
		{
			c_iml_header = 'Y';

			fn_bfprint_rcvlog( 	ptr_rcv_msgs_file,
													c_iml_header,
													(void *)c_ptr_data,
													c_ServiceName,
													c_errmsg);

	   if(DEBUG_MSG_LVL_5)
		 {
		 fn_userlog(c_ServiceName,"After calling fn_bfprint_log in rcv thrd : Val of c_errmsg=<%s>", c_errmsg);
		 }
		}

		/*** Commented in ver 1.6
	  memcpy(&l_msg_type, c_ptr_data + sizeof(struct st_bfo_header), sizeof(long));  
    fn_userlog ( c_ServiceName, " Before Conversion MSG Typ :%ld:",l_msg_type); 
    l_msg_type = fn_swap_long(l_msg_type);
    fn_userlog ( c_ServiceName, "Message recieved from exchange-after convrsn -li_msg_type  |%ld|", l_msg_type);
			Commented in ver 1.6 ***/

		/* Ver 1.6 */
    memset ( &st_comm_hdr, 0, sizeof ( struct st_comm_header ) );
    memcpy(&st_comm_hdr,c_ptr_data, sizeof( st_comm_hdr) );
    fn_cnvt_to_hfmt_comm_hdr(&st_comm_hdr);
    /* Ver 1.6 */
    fn_userlog ( c_ServiceName, "Message recieved from exchange-after convrsn -TemplateID  |%ld|", st_comm_hdr.TemplateID);


			
				/** ver 1.6 Start **/
			  if(st_comm_hdr.TemplateID == TID_REJECT && l_login_reject == -1)
    		{
							RejectT st_var_reject;
							memset ( &st_var_reject, 0, sizeof ( RejectT ) );
							memcpy ( ( char * )&st_var_reject, c_ptr_data, sizeof ( RejectT ) );
							fn_cnvt_to_hfmt_reject_rply(&st_var_reject);

      				l_login_reject = st_var_reject.NRResponseHeaderME.MsgSeqNum;      /* set to 0 when receive reject message for login response */
					 		fn_userlog ( c_ServiceName, " st_var_reject.VarText :%s" ,st_var_reject.VarText);  
				}
				/* Added in ver 1.6 -Ends */
				
			/*********************************************************************************/
			/** Added part on 12-Apr-2012 for SLOT NOT FREE ISSUE - END- 		        **/
			/*********************************************************************************/			

        memset ( &tmp_buf_dat, '\0', sizeof(tmp_buf_dat) );
  			memcpy(&tmp_buf_dat, c_ptr_data  , sizeof(tmp_buf_dat)); 

				i_ptr_size=st_comm_hdr.BodyLen; /* Ver 1.6 */
    switch (st_comm_hdr.TemplateID)	/** change l_msg_type to st_comm_hdr.TemplateID Ver 1.6 **/
    {   /*** Switch condition for l_msg_type start **/
		    /** Add condition for success of USR_RGSTRN_REQ_OUT --error code: */

 			case USR_RGSTRN_REQ_OUT:  /** 0 **/

					 /** Call i_ch_val = fn_bse_rgstrn_req_out  */

  				if(DEBUG_MSG_LVL_3)
				{
				fn_userlog ( c_ServiceName, "Val of state of rcv thrd b4 fn_bse_rgstrn_req_out <%d> ", i_rcv_thrd_exit_stts);
				}

	 				i_ch_val = fn_bse_rgstrn_req_out(& (tmp_buf_dat.st_busr_rgstrn_reply),
																				   c_ServiceName,            
					  															 c_errmsg );               

        	if ( i_ch_val == -1 )
        	{
          	fn_errlog(c_ServiceName, "S31170", LIBMSG, c_errmsg);
			 			i_cond_snd_rcv_cntrl = RCV_ERR;	
						pthread_cond_signal(&cond_snd_rcv_cntrl);
          	i_rcv_thrd_exit_stts = EXIT;
        	}

					if(DEBUG_MSG_LVL_0)
					{
        		fn_userlog(c_ServiceName,"RGSTRN_REQ_OUT ENDED ");
   				fn_userlog(c_ServiceName, "Val of sql_c_pipe_id to be used for fn_logon_to_bse <%s>", sql_c_pipe_id);
   				fn_userlog(c_ServiceName, "Before call to fn_logon_to_bse after reading registration reply");
					}

     			/*** Added BPH selecet part start for AUTO login when registration success ****/	
    			EXEC SQL 
    					SELECT BPH_RGSTRN_LGN_FLG 
    					INTO :c_bph_rgstrn_lgn_flg
    					FROM BPH_BFO_PSSWD_HNDLG
    					WHERE BPH_PIPE_ID = :sql_c_pipe_id;


    				if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
    				{
      				fn_userlog(c_ServiceName, "ERROR IN BPH select for regstr flag");
      				fn_errlog(c_ServiceName,"S31175", LIBMSG, c_err_msg);
      				i_rcv_thrd_exit_stts = EXIT;
      				continue;
    				}

						/*** Added BPH selecet part end for AUTO login when registration success ****/	

  			  if( (tmp_buf_dat.st_busr_rgstrn_reply.l_error_no == 0)  && (c_bph_rgstrn_lgn_flg == 'A') )
   				{
     				i_ch_val = fn_logon_to_bse(c_ServiceName, sql_c_pipe_id, c_err_msg);

    				if(i_ch_val == -1)
    				{
      				fn_userlog(c_ServiceName, "ERROR IN call to fn_logon_to_bse");
      				fn_errlog(c_ServiceName,"S31180", LIBMSG, c_err_msg);
      				i_rcv_thrd_exit_stts = EXIT;
      				continue;
    				}
   
   					/** Added on 28-May-2011 for signal handelling  Starts */
   					else
   					{
	  					fn_userlog(c_ServiceName, "Val of state of rcv thrd After fn_bse_rgstrn_req_out <%d> ", i_rcv_thrd_exit_stts);
			 		 		i_cond_snd_rcv_cntrl = RGSTRN_RESP_RCVD;	 
							pthread_cond_signal(&cond_snd_rcv_cntrl);
   					}
   					/** Added on 28-May-2011 for signal handelling  End */
    				fn_userlog(c_ServiceName, "After successful call to fn_logon_to_bse");
  				}

					fn_userlog(c_ServiceName, "Before:%d:",i_cond_snd_rcv_cntrl);
   			  i_cond_snd_rcv_cntrl = RGSTRN_RESP_RCVD;
					fn_userlog(c_ServiceName, "After :%d:",i_cond_snd_rcv_cntrl);
          pthread_cond_signal(&cond_snd_rcv_cntrl);

  				i_call_rcv = NO;
  	  break;
/** Added LOGON_REQ_OUT case for login response handelling Start **/
      case LOGON_REQ_OUT :   /* 1131 */
			case TID_USER_LOGIN_RESPONSE :			/** Ver 1.6 **/

					st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat; 

					if(DEBUG_MSG_LVL_3)
					{
					fn_userlog ( c_ServiceName, "After memcpy of reveive structure ");
          fn_userlog ( c_ServiceName, "INSIDE LOGON_REQ_OUT ");
          /* fn_userlog ( c_ServiceName, "Val of st_buf_dat.st_blogon_reply.s_reply_code <%d> l_msg_tag <%ld> l_trader_id <%ld>",st_buf_dat->st_blogon_reply.s_reply_code,st_buf_dat->st_blogon_reply.l_msg_tag,st_buf_dat->st_blogon_reply.l_trader_id); Ver 1.6 **/
           fn_userlog ( c_ServiceName, "Val of st_buf_dat.MessageHeaderOut.TemplateID <%d> ",st_buf_dat->st_sinonreply.MessageHeaderOut.TemplateID); /** Ver 1.6 **/
	  			}

				  i_ch_val = fn_bse_logon_req_out ( & (st_buf_dat->st_sinonreply), c_ServiceName, c_errmsg ); /* changed to st_sinonreply from st_blogon_reply in Ver 1.6 */ 

          if ( i_ch_val == -1 )
          {
           fn_errlog(c_ServiceName, "S31185", LIBMSG, c_errmsg);
           i_cond_snd_rcv_cntrl = RCV_ERR;	
 	  			 pthread_cond_signal(&cond_snd_rcv_cntrl);
           i_rcv_thrd_exit_stts = EXIT;
          }
          else
          {
	  		   fn_userlog ( c_ServiceName, "inside Login i_ch_val == 0 of fn_bse_logon_req_out" );
	    		 i_cond_snd_rcv_cntrl = LOGON_RES_RCVD;	 
	     		 pthread_cond_signal(&cond_snd_rcv_cntrl);
			 		 /*** alarm(ui_keep_alive_interval); *** Ver 1.2 ***/ /*** Alarm send after login Success ***/ 
          }
 
         if(DEBUG_MSG_LVL_0)
				 {
          fn_userlog(c_ServiceName,"Logon Reply Processed ");
				 }

         i_call_rcv = NO;
      break; 
			/** Added LOGON_REQ_OUT case for login response handelling End **/
      case OPN_INTRST_RQST :  /** 1149 **/
           st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;
           memcpy(&l_dw_slot_no, c_ptr_data , sizeof(long)); 
           if(DEBUG_MSG_LVL_3)
	   		 	 {
						 fn_userlog(c_ServiceName,"Call OPN_INTRST_RQST to update FUM ");
					 }
           l_dw_slot_no = fn_swap_long(l_dw_slot_no);
           
           if(l_dw_slot_no == 1)
           { 
            fn_userlog(c_ServiceName," l_dw_slot_no :%ld:",l_dw_slot_no);
            fn_userlog(c_ServiceName," Msg Typ  :%ld:",st_buf_dat->st_opn_intrst_data.l_msg_typ);
            fn_userlog(c_ServiceName," Reply Code :%d:",st_buf_dat->st_opn_intrst_data.s_rply_cd);
            fn_userlog(c_ServiceName," Msg Tag   :%ld:",st_buf_dat->st_opn_intrst_data.l_msg_tag); 
            fn_userlog(c_ServiceName," Filler     :%d:",st_buf_dat->st_opn_intrst_data.s_filler);
           }
           if( l_dw_slot_no == 0 )
           {                                     
            i_ch_val = fn_upd_opn_inrst(&tmp_buf_dat.st_ums_opn_intrst_data, c_ServiceName, c_errmsg );

            if ( i_ch_val == -1 )
            {
             fn_errlog(c_ServiceName, "S31190", LIBMSG, c_errmsg);
             i_cond_snd_rcv_cntrl = RCV_ERR;
             pthread_cond_signal(&cond_snd_rcv_cntrl);
             i_rcv_thrd_exit_stts = EXIT;
            } 
           }
          i_call_rcv = NO;
       break;
      case END_OF_UMS_OPN_INTRST : /*** 1150 ***/

         st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;
         fn_userlog(c_ServiceName," ######################################## ");
	 fn_userlog(c_ServiceName," Received END of OPEN INTEREST DOWNLOAD ");
	 fn_userlog(c_ServiceName," ######################################## ");
         st_buf_dat->st_end_of_oi_ums_data.l_tot_records = fn_swap_long(st_buf_dat->st_end_of_oi_ums_data.l_tot_records);
         fn_userlog(c_ServiceName," Total no of Open Interest Records Downloaded :%ld:",st_buf_dat->st_end_of_oi_ums_data.l_tot_records);            
         i_call_rcv = NO; 
        break;
      case LOGOFF_REQ_OUT :  /** 1132 **/
      case TID_USER_LOGOUT_RESPONSE :  /** 10024 **/	/** Ver 1.6 **/
            
     				fn_userlog ( c_ServiceName, "INSIDE LOGOFF_REQ_OUT ");
     				/* i_ch_val = fn_bse_logoff_req_out( &(tmp_buf_dat.st_blogoff_reply), c_ServiceName, c_errmsg); commented in Ver 1.6  **/
     				i_ch_val = fn_bse_logoff_req_out( &(tmp_buf_dat.st_signoffreply), c_ServiceName, c_errmsg); /** Ver 1.6 **/ 

    				if ( i_ch_val == -1 )
     				{
       			 fn_errlog(c_ServiceName, "S31195", LIBMSG, c_errmsg);
       			 i_cond_snd_rcv_cntrl = RCV_ERR;
       			 pthread_cond_signal(&cond_snd_rcv_cntrl);
       			 i_rcv_thrd_exit_stts = EXIT;
     				}
     				else
     				{
      			  fn_userlog (c_ServiceName,"Val of state of rcv thrd After fn_bse_rgstrn_req_out -1<%d>", i_rcv_thrd_exit_stts);
      				i_cond_snd_rcv_cntrl = LOGOFF_RES_RCVD;
      				pthread_cond_signal(&cond_snd_rcv_cntrl);
     
     					/** Imp Note: Here esr client is stopped using trigger when logoff reply is processed **/
          		i_cond_snd_rcv_cntrl = LOGOFF_RES_RCVD;	
 	  					pthread_cond_signal(&cond_snd_rcv_cntrl);
          		i_rcv_thrd_exit_stts = EXIT;
							/*************************************************/
	  					fn_userlog(c_ServiceName,"Val of sql_c_pipe_id before stop bse_esr <%s>", sql_c_pipe_id);
       				sprintf ( c_pgm_nm, "cln_bse_esr_clnt_%s",sql_c_pipe_id );
	  					fn_pst_trg ( c_pgm_nm, "TRG_STP_BAT", "TRG_STP_BAT", c_pgm_nm );
	  					fn_userlog(c_ServiceName,"After posting trigger TRG_STP_BAT in fn_bat_term for esr <%s>", c_pgm_nm);
     				}
     				if(DEBUG_MSG_LVL_0)
     				{
      				fn_userlog(c_ServiceName,"Logoff Replay Processed ");
     				}
     				i_call_rcv = NO;
      break; 
		  /****** Read password change response ***/
   	  case MANDATORY_PASSWD_CHNG : /** 1133 **/
      case OPTNL_PASSWD_CHNG :   /** 1134 **/
			case TID_USER_PASSWORD_CHANGE_RESPONSE :		/** 10043 **/ /** Ver 1.6 **/

      			st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat; 
						if(DEBUG_MSG_LVL_3)
						{
      			fn_userlog ( c_ServiceName, "After memcpy of specific structure for passwd chng ");
          	fn_userlog ( c_ServiceName, "INSIDE MANDATORY_PASSWD_CHNG in rcbv_thrd");
          	/* commented in Ver 1.6 fn_userlog ( c_ServiceName, "Val of st_buf_dat.st_blogon_reply.s_reply_code <%d> l_msg_tag<%ld> l_trader_id<%ld>",st_buf_dat->st_blogon_reply.s_reply_code,st_buf_dat->st_blogon_reply.l_msg_tag,st_buf_dat->st_blogon_reply.l_trader_id); */
						fn_userlog ( c_ServiceName, "Val of MsgSeqNum :%ld: TemplateID :%d:,BodyLen :ld:", fn_swap_long(st_buf_dat->st_pswdchngreply.ResponseHeader.MsgSeqNum),fn_swap_short(st_buf_dat->st_pswdchngreply.MessageHeaderOut.TemplateID),fn_swap_long(st_buf_dat->st_pswdchngreply.MessageHeaderOut.BodyLen));  /* Ver 1.6 */
						}
		/* if (st_buf_dat->st_blogon_reply.s_reply_code == 0) commented in ver 1.6  */
			 {
        if(DEBUG_MSG_LVL_3)
				{
				fn_userlog(c_ServiceName,"Call fn_upd_chngd_passwd to update PW");
				}
							i_ch_val = fn_upd_chngd_passwd ( c_ServiceName, c_errmsg ); 

          		if ( i_ch_val == -1 )
          		{
            		fn_errlog(c_ServiceName, "S31200", LIBMSG, c_errmsg);
            		i_cond_snd_rcv_cntrl = RCV_ERR;	
 	    					pthread_cond_signal(&cond_snd_rcv_cntrl);
            		i_rcv_thrd_exit_stts = EXIT;
          		}
	  					else
          		{
	   						fn_userlog ( c_ServiceName, "inside Login i_ch_val == 0 " );
	   						i_cond_snd_rcv_cntrl = PASSWD_RES_RCVD;	 
	   						pthread_cond_signal(&cond_snd_rcv_cntrl);
          		}        
	 		}
		/* commented in Ver 1.6 	else
     	{
	  					fn_userlog ( c_ServiceName, "inside change password request fail" );
	  					i_cond_snd_rcv_cntrl = PASSWD_RES_RCVD;	 
	  					pthread_cond_signal(&cond_snd_rcv_cntrl);
     	}         */

        		if(DEBUG_MSG_LVL_0)
						{
         			fn_userlog(c_ServiceName,"Logon Reply Processed ");
						}

        		i_call_rcv = NO;
      break; 
      /* ############################################################################### */ 
      case PRSNL_ORDR_DWNLD : /** 1092 **/
         
						st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;  
					  memcpy(&l_dw_slot_no, c_ptr_data , sizeof(long));  

         	  if(DEBUG_MSG_LVL_3)
						{
							fn_userlog ( c_ServiceName, "INSIDE ORD DOWNLOAD RESPONSE RECVD in rcv_thrd- type-1092 ");
						}
						if(DEBUG_MSG_LVL_5)
	 	        {
						fn_userlog ( c_ServiceName, "Val After memcpy of l_dw_slot_no <%ld>",l_dw_slot_no);
		        fn_userlog ( c_ServiceName, "Val of st_buf_dat.st_prsnl_dwnld_reply.s_rply_cd <%d>",st_buf_dat->st_prsnl_dwnld_reply.s_rply_cd);
			  		fn_userlog ( c_ServiceName, "Val of st_buf_dat.st_prsnl_dwnld_reply.l_msg_typ <%ld>",st_buf_dat->st_prsnl_dwnld_reply.l_msg_typ);
          	fn_userlog ( c_ServiceName, "Val of st_buf_dat->st_prsnl_dwnld_reply.st_hdr.l_slot_no <%ld>",st_buf_dat->st_prsnl_dwnld_reply.st_hdr.l_slot_no);
						}
     
						l_dw_slot_no = fn_swap_long(l_dw_slot_no);
						if(DEBUG_MSG_LVL_3)
						{
						fn_userlog ( c_ServiceName, "Val After SWAP of l_dw_slot_no for type-1092<%ld>",l_dw_slot_no);
						}

					  i_call_rcv = NO;  /** Moved here on 17-May-2012 POST UAT and changed it to NO **/
					  if (l_dw_slot_no == 0)
					  {
		   				if(DEBUG_MSG_LVL_3)
						{
						fn_userlog ( c_ServiceName, "In case of actual order download--Call BRCV_CLNT ");
						}
						i_call_rcv = ACK;  /** Changed POST UAT to avoid duplicate entries in FXD **/
		  				/*****************************************************************************/
		   				i_read_size = sizeof(struct st_prsnl_ordr_ums); 
		  				/*****************************************************************************/

        	 		if(DEBUG_MSG_LVL_3)
							{
								fn_userlog ( c_ServiceName, "After copy of st_prsnl_ordr_ums_data");
							}
						}
          	fn_userlog(c_ServiceName,"Download  Reply Processed ");        		
						i_dwnld_flg = DOWNLOAD;

     /** Ver 1.6 starts**/
      break;
			case TID_RETRANSMIT_RESPONSE : /* 10009 */
				st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;

        fn_userlog ( c_ServiceName," 10009 TID_RETRANSMIT_RESPONSE  ");

					i_call_rcv = NO;   /* changed to NO in Ver 1.7 */
					/** commented in Ver 1.7 i_read_size = sizeof(RetransmitResponseT);
					i_dwnld_flg = DOWNLOAD; **/
      
          /** Ver 1.7 starts **/
          i_total_msgs = fn_swap_ushort(st_buf_dat->st_var_trade_dwnld.ApplTotalMessageCount) + 1;
          fn_userlog(c_ServiceName,"10009 i_partition_id |%d| st_buf_dat->st_var_trade_dwnld.ApplTotalMessageCount:%d:",i_partition_id, i_total_msgs);

          ll_append_seq_num = fn_swap_ulnglng(st_buf_dat->st_var_trade_dwnld.ApplEndSeqNum);
          ll_RefApplLastSeqNum = fn_swap_ulnglng(st_buf_dat->st_var_trade_dwnld.RefApplLastSeqNum);

          i_call_rcv = NO;
          i_dwnld_flag = TRD_DWNLD;        /**  Flag Set for Trade Download  **/
          i_trddwnld_cntr = 0;

          if( DEBUG_MSG_LVL_0 )
          {
            fn_userlog(c_ServiceName,"st_buf_dat->st_var_trade_dwnld.ApplTotalMessageCount:%d:", i_total_msgs);
            fn_userlog(c_ServiceName,"ll_append_seq_num :%lld:",ll_append_seq_num);
            fn_userlog(c_ServiceName,"ll_RefApplLastSeqNum :%lld:",ll_RefApplLastSeqNum);
          }     
         /** Ver 1.7 ends **/

				break;
			 case TID_RETRANSMIT_ME_MESSAGE_RESPONSE : /* 10027 */
        st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;

        fn_userlog ( c_ServiceName,"TID_RETRANSMIT_RESPONSE  ");

         /** commented in Ver 1.7 i_call_rcv = ACK;
          i_read_size = sizeof(RetransmitMEMessageResponseT);
          i_dwnld_flg = DOWNLOAD; **/

         /** Ver 1.7 starts **/
        i_total_msgs = fn_swap_ushort(st_buf_dat->st_var_ordr_dwnld.ApplTotalMessageCount) + 1;
        fn_userlog(c_ServiceName,"10027 i_partition_id |%d|st_buf_dat->st_var_ordr_dwnld.ApplTotalMessageCount:%d:",i_partition_id, i_total_msgs);

         i_call_rcv = NO;
         i_dwnld_flag = ORDR_DWNLD;        /**  Flag Set for Order Download  **/
         i_orddwnld_cntr = 0;
  
         memset(c_RefApplLastSeqNum,0,16);
         memset(c_EndApplSeqNum,0,16);
         memcpy(c_RefApplLastSeqNum,st_buf_dat->st_var_ordr_dwnld.RefApplLastMsgID,sizeof(c_RefApplLastSeqNum));
         memcpy(c_EndApplSeqNum,st_buf_dat->st_var_ordr_dwnld.ApplEndMsgID,sizeof(c_EndApplSeqNum));

         if(DEBUG_MSG_LVL_5)
         {
          c_data1 = c_RefApplLastSeqNum_hex;
          for(i=0;i<16;i++)
          {
            c_data1 += sprintf(c_data1, "%.2X", c_RefApplLastSeqNum[i]);
          }
          fn_userlog(c_ServiceName,"c_RefApplLastSeqNum_hex:%s:",c_RefApplLastSeqNum_hex);  

          c_data1 = c_EndApplSeqNum_hex;
          for(i=0;i<16;i++)
          {
            c_data1 += sprintf(c_data1, "%.2X", c_EndApplSeqNum[i]);
          }
          fn_userlog(c_ServiceName,"c_EndApplSeqNum_hex :%s:",c_EndApplSeqNum_hex);
		     }		 
         /** Ver 1.7 ends **/
        break;
      /** Ver 1.6 ends **/
 
      /* ############################################################################### */
      case PRSNL_TRD_DWNLD : /** 1095 **/
	 					 st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;  
					   memcpy(&l_dw_slot_no, c_ptr_data , sizeof(long));  

         		 if(DEBUG_MSG_LVL_3)
						 {
							 fn_userlog ( c_ServiceName, "INSIDE TRD DOWNLOAD RESPONSE RECVD in rcbv_thrd-Type- 1095");
						 }

						 if(DEBUG_MSG_LVL_5)
						 {
						 fn_userlog ( c_ServiceName, "Val After memcpy of l_dw_slot_no in TRD_DWLD <%ld>",l_dw_slot_no);
						 fn_userlog ( c_ServiceName, "Val of st_buf_dat.st_prsnl_dwnld_reply.s_rply_cd in TRD_DWLD<%d>",st_buf_dat->st_prsnl_dwnld_reply.s_rply_cd);
						 fn_userlog ( c_ServiceName, "Val of st_buf_dat.st_prsnl_dwnld_reply.l_msg_typ in TRD_DWLD<%ld>",st_buf_dat->st_prsnl_dwnld_reply.l_msg_typ);
          	 fn_userlog ( c_ServiceName, "Val of st_buf_dat->st_prsnl_dwnld_reply.st_hdr.l_slot_no in TRD_DWLD<%ld>",st_buf_dat->st_prsnl_dwnld_reply.st_hdr.l_slot_no);
		 				 }
     
						 l_dw_slot_no = fn_swap_long(l_dw_slot_no);
						 if(DEBUG_MSG_LVL_3)
						 {
						 fn_userlog ( c_ServiceName, "Val After SWAP of l_dw_slot_no in TRD_DWLD <%ld>",l_dw_slot_no);
						 }

					 	 i_call_rcv = NO;   /** Moved here on 17-May-2012 POST UAT and changed it to NO **/
						 if (l_dw_slot_no == 0)
						 {
		   				 if(DEBUG_MSG_LVL_3)
						 {
						 fn_userlog ( c_ServiceName, "In case of actual TRD download--Call BTRD_CLNT ");
						 }
						 i_call_rcv = TRD;  /** Changed POST UAT to avoid duplicate entries in FXD_TRD **/
		  			 i_read_size = sizeof(struct st_prsnl_trade_ums); 

       	 	 	 if(DEBUG_MSG_LVL_5)
						 {
								 fn_userlog ( c_ServiceName, "Val of No of Records Downloaded in TRD_DWLD without conv:%ld:",st_buf_dat->st_prsnl_trade_ums_data.l_no_of_records);
								 fn_userlog ( c_ServiceName, "After copy of st_prsnl_trade_ums_data");
						 }
				  	 }
          	 fn_userlog(c_ServiceName,"Download  Reply Processed for TRD_DWLD");  		 
			       i_dwnld_flg = DOWNLOAD;
      break;
      /* ############################################################################### */

      /* ############################################################################### */
      /*******************  case to handle sltp dwnld in rcv thrd Starts- 28-Nov-2011  *****************/
      case PRSN_SLTP_ORDR_DWNLD : /** 1097 **/
	 					 st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;  
					   memcpy(&l_dw_slot_no, c_ptr_data , sizeof(long));  

         		if(DEBUG_MSG_LVL_3)
						{
						 fn_userlog ( c_ServiceName, "INSIDE PRSN_SLTP_ORDR_DWNLD RESPONSE RECVD in rcbv_thrd");
						 fn_userlog ( c_ServiceName, "Val After memcpy of l_dw_slot_no in TRD_DWLD <%ld>",l_dw_slot_no);
						 fn_userlog ( c_ServiceName, "Val of st_buf_dat.st_prsnl_dwnld_reply.s_rply_cd in TRD_DWLD<%d>",st_buf_dat->st_prsnl_dwnld_reply.s_rply_cd);
						 fn_userlog ( c_ServiceName, "Val of st_buf_dat.st_prsnl_dwnld_reply.l_msg_typ in TRD_DWLD<%ld>",st_buf_dat->st_prsnl_dwnld_reply.l_msg_typ);
          	 fn_userlog ( c_ServiceName, "Val of st_buf_dat->st_prsnl_dwnld_reply.st_hdr.l_slot_no in TRD_DWLD<%ld>",st_buf_dat->st_prsnl_dwnld_reply.st_hdr.l_slot_no);
		 				}     
						 l_dw_slot_no = fn_swap_long(l_dw_slot_no);
						 if(DEBUG_MSG_LVL_5)
						 {
						 fn_userlog ( c_ServiceName, "Val After SWAP of l_dw_slot_no in TRD_DWLD <%ld>",l_dw_slot_no);
						 }

					 	 i_call_rcv = NO;   /** Moved here on 17-May-2012 POST UAT and changed it to NO **/
						 if (l_dw_slot_no == 0)
						 {
		   				 fn_userlog ( c_ServiceName, "In case of actual PRSN_SLTP_ORDR_DWNLD download--Call BRCV_CLNT ");
							 i_call_rcv = ACK;  /** Changed POST UAT to avoid duplicate entries in FXD_TRD **/
		  				 /*****************************************************************************/
		  				 i_read_size = sizeof(struct st_prsnl_sltp_ums); 
		 				   /*****************************************************************************/

		 					 if(DEBUG_MSG_LVL_3)
							 {
							 fn_userlog ( c_ServiceName, "After copy of st_prsnl_sltp_ums_data");
       	 	 		 fn_userlog ( c_ServiceName, "Val of No of Records Downloaded in SLTP_DWLD:%ld:",st_buf_dat->st_prsnl_sltp_ums_data.l_no_of_records);
				 				}
				  	 }
          	 fn_userlog(c_ServiceName,"Download  Reply Processed for SLTP_DWLD");        		 
			       i_dwnld_flg = DOWNLOAD;
      break;
      /*******************  case to handle sltp dwnld in rcv thrd Ends- 28-Nov-2011  *****************/
      /* ############################################################################### */
   		case PRSNL_DWNLD_CMPLT: /** 1520 **/
       
					 st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;
        	 if(DEBUG_MSG_LVL_3)
		 			 {
		 					fn_userlog ( c_ServiceName, "INSIDE DOWNLOAD COMPLETE ");
					 }
        	 st_buf_dat->st_end_of_ums_dwnld_reply.l_no_of_recrd_dwnlded = ntohl(st_buf_dat->st_end_of_ums_dwnld_reply.l_no_of_recrd_dwnlded);
        	 st_buf_dat->st_end_of_ums_dwnld_reply.l_no_of_recrd_dwnlded=fn_swap_long(st_buf_dat->st_end_of_ums_dwnld_reply.l_no_of_recrd_dwnlded);
       	 	 fn_userlog ( c_ServiceName, " No of Records Downloaded :%ld:",st_buf_dat->st_end_of_ums_dwnld_reply.l_no_of_recrd_dwnlded);
        	 st_buf_dat->st_end_of_ums_dwnld_reply.l_msg_tag = fn_swap_long(st_buf_dat->st_end_of_ums_dwnld_reply.l_msg_tag);
        	 if (st_buf_dat->st_end_of_ums_dwnld_reply.l_msg_tag == PRSNL_ORDR_DWNLD_TAG)
        	 {
						 c_ord_dw_cmplt = 'Y';
        		 fn_userlog ( c_ServiceName, " PERSONAL ORDER DOWNLOAD COMPLETE - <%c>", c_ord_dw_cmplt);
			 			 l_tot_recrd_dwnlded_ord = st_buf_dat->st_end_of_ums_dwnld_reply.l_no_of_recrd_dwnlded;
			 			 fn_userlog ( c_ServiceName, " PERSONAL ORDER DOWNLOAD COUNT - <%ld>", l_tot_recrd_dwnlded_ord);
        	 }
        	 if (st_buf_dat->st_end_of_ums_dwnld_reply.l_msg_tag ==PRSNL_TRD_DWNLD_TAG)
        	 {
						c_trd_dw_cmplt = 'Y';
						l_tot_recrd_dwnlded_trd = st_buf_dat->st_end_of_ums_dwnld_reply.l_no_of_recrd_dwnlded;
         		fn_userlog ( c_ServiceName, " PERSONAL TRADE DOWNLOAD COMPLETE- <%c>", c_trd_dw_cmplt); 
					  fn_userlog ( c_ServiceName, " PERSONAL TRADE DOWNLOAD COUNT- <%ld>", l_tot_recrd_dwnlded_trd); 

        	 } 
        	 if (st_buf_dat->st_end_of_ums_dwnld_reply.l_msg_tag == PRSN_SLTP_ORDR_DWNLD_TAG)
        	 {
        		 c_sltp_dw_cmplt = 'Y';
			 			 l_tot_recrd_dwnlded_sltp = st_buf_dat->st_end_of_ums_dwnld_reply.l_no_of_recrd_dwnlded;
			 			 fn_userlog ( c_ServiceName, " PERSONAL SLTP ORDER DOWNLOAD COMPLETE- <%c>", c_sltp_dw_cmplt);
			 			 fn_userlog ( c_ServiceName, " PERSONAL SLTP ORDER DOWNLOAD COUNT- <%ld>", l_tot_recrd_dwnlded_sltp);
        	 }

		 			if (c_ord_dw_cmplt == 'Y' && c_trd_dw_cmplt == 'Y' && c_sltp_dw_cmplt == 'Y')
		 			{
						/* Start of Added  part for logic to start Reco pgm after complete inserted into download tables */

						while (1)
						{
			 				if (l_tot_recrd_insert_ord < l_tot_recrd_dwnlded_ord)
			 				{
								EXEC SQL 
								SELECT COUNT(1) 
								into :l_tot_recrd_insert_ord
								FROM FXD_FO_XCHNG_DWNLD
								WHERE FXD_PIPE_ID = :sql_c_pipe_id
								/*and trunc(FXD_LST_MOD_TM) = :sql_c_nxt_trd_date */
								and trunc(FXD_LST_MOD_TM) = trunc(sysdate)
								and FXD_MSG_TYP = '1092';

							 if ( SQLCODE != 0  && SQLCODE != NO_DATA_FOUND)
							 {
				 				fn_errlog(c_ServiceName,"S31205", SQLMSG, c_errmsg);
				 				fn_userlog(c_ServiceName,"Error -selecting count in FXD table for ord dwld for :%s:",sql_c_pipe_id);
				 				break;
							 }

							 if(DEBUG_MSG_LVL_5)
							 {
								fn_userlog ( c_ServiceName, " PERSONAL ORDER INSERTED COUNT IN LOOP- <%ld>", l_tot_recrd_insert_ord);
							 }

			 			  }
			 			  if (l_tot_recrd_insert_trd < l_tot_recrd_dwnlded_trd)
			 			  {
								EXEC SQL 
								SELECT COUNT(*) 
								into :l_tot_recrd_insert_trd
								FROM FXD_FO_XCHNG_TRD_DWNLD
								WHERE FXD_PIPE_ID = :sql_c_pipe_id
								/* and trunc(FXD_TRD_DT) = :sql_c_nxt_trd_date */
								and trunc(FXD_TRD_DT) = trunc(sysdate)
								and FXD_MSG_TYP = '1095';

								if ( SQLCODE != 0  && SQLCODE != NO_DATA_FOUND)
								{
				 					fn_errlog(c_ServiceName,"S31210", SQLMSG, c_errmsg);
				 					fn_userlog(c_ServiceName,"Error -selecting count in FXD_TRD table for trd dwld for :%s:",sql_c_pipe_id);
				 					break;
								}
				
								if(DEBUG_MSG_LVL_5)
								{
									fn_userlog ( c_ServiceName, " PERSONAL TRADE INSERTED COUNT IN LOOP- <%ld>", l_tot_recrd_insert_trd); 
								}
			 				}
						  if (l_tot_recrd_insert_sltp < l_tot_recrd_dwnlded_sltp)
			 				{
								EXEC SQL 
								SELECT COUNT(1) 
								into :l_tot_recrd_insert_sltp
								FROM FXD_FO_XCHNG_DWNLD
								WHERE FXD_PIPE_ID = :sql_c_pipe_id
								and trunc(FXD_LST_MOD_TM) = trunc(sysdate)
								and FXD_MSG_TYP = '1097';

								if ( SQLCODE != 0  && SQLCODE != NO_DATA_FOUND)
								{
				 					fn_errlog(c_ServiceName,"S31215", SQLMSG, c_errmsg);
				 					fn_userlog(c_ServiceName,"Error -selecting count in FXD table for sltp dwld for :%s:",sql_c_pipe_id);
				 					break;
								}

								if(DEBUG_MSG_LVL_5)
								{
									fn_userlog ( c_ServiceName, " PERSONAL SLTP ORDER INSERTED COUNT IN LOOP- <%ld>", l_tot_recrd_insert_sltp);
								}
			 				}

							if ( (l_tot_recrd_dwnlded_ord == l_tot_recrd_insert_ord) && (l_tot_recrd_dwnlded_sltp == l_tot_recrd_insert_sltp) && (l_tot_recrd_dwnlded_trd == l_tot_recrd_insert_trd) )
							{
			 					fn_userlog(c_ServiceName,"All inserted count matches with downloaded count - break loop");
			 					break;
							}

							l_dwld_prcs_cnt ++;
							fn_userlog ( c_ServiceName, "Val of l_dwld_prcs_cnt in loop - SLEEP for 1 sec - <%ld>", l_dwld_prcs_cnt );
							sleep(1);	
						}

						if ( (l_tot_recrd_dwnlded_ord != 0) || (l_tot_recrd_dwnlded_sltp != 0) || (l_tot_recrd_dwnlded_trd != 0)) 
			  		{
			  			c_breco_start_flg = 'Y';	

							if(DEBUG_MSG_LVL_3)
							{
								fn_userlog ( c_ServiceName, " PERSONAL ORDER DOWNLOADED COUNT- <%ld>", l_tot_recrd_dwnlded_ord);
								fn_userlog ( c_ServiceName, " PERSONAL ORDER INSERTED COUNT- <%ld>", l_tot_recrd_insert_ord);
								fn_userlog ( c_ServiceName, " PERSONAL TRADE DOWNLOADED COUNT- <%ld>", l_tot_recrd_dwnlded_trd);
								fn_userlog ( c_ServiceName, " PERSONAL TRADE INSERTED COUNT- <%ld>", l_tot_recrd_insert_trd);
								fn_userlog ( c_ServiceName, " PERSONAL SLTP DOWNLOADED COUNT- <%ld>", l_tot_recrd_dwnlded_sltp);
								fn_userlog ( c_ServiceName, " PERSONAL SLTP INSERTED COUNT- <%ld>", l_tot_recrd_insert_sltp);
								fn_userlog(c_ServiceName,"Val of c_breco_start_flg is set to 'Y'-After complete insert");
							}  			   			   
			 		  }
			 	 	  /* End of Added  part for logic to start Reco pgm after complete inserted into download tables */

	 	  		  fn_pst_trg ( c_ServiceName, "TRG_RECO_OVER", "TRG_RECO_OVER", sql_c_pipe_id );
		   		  fn_userlog(c_ServiceName,"After posting trigger: TRG_RECO_OVER");
		   		  fn_userlog(c_ServiceName,"END OF DOWNLOAD");

		 		 	  fn_userlog ( c_ServiceName, "#######################################################");
			      fn_userlog ( c_ServiceName, "---------- All THREE-TYPES DOWNLOAD COMPLETE- ");
				 	  fn_userlog ( c_ServiceName, "#######################################################");

            /** Added POST UAT for FTM insert Starts **/

			  	  i_trnsctn = fn_begintran( c_ServiceName, c_errmsg );
			  	  if ( i_trnsctn == -1)
			  	  {
			    		fn_errlog(c_ServiceName,"S31220", LIBMSG, c_errmsg);
    			    fn_userlog(c_ServiceName,"Error in transaction begin-ESR clnt for FTM insert");
			    		break;
			  	  }

					  EXEC SQL
					  INSERT INTO ftm_fo_trd_msg
											(	
												ftm_xchng_cd,
												ftm_msg,
												ftm_msg_id,
												ftm_tm
											)
											VALUES	
											(	
											 :sql_c_xchng_cd,
											 'Down Load Completed:'||  :sql_c_pipe_id,
											 'S',			
											 sysdate
										  );
				    if ( SQLCODE != 0 )
    		    {
    					fn_errlog(c_ServiceName,"S31225", SQLMSG, c_err_msg);
      				fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
      				fn_userlog(c_ServiceName,"Error in Insert for FTM ");
      				break;
    				}					

      		  i_ch_val = fn_committran( c_ServiceName, i_trnsctn, c_errmsg );
  					if ( i_ch_val == -1 )
  					{
    					fn_errlog(c_ServiceName,"S31230", LIBMSG, c_errmsg);
							fn_userlog(c_ServiceName,"Error in transaction Commit-ESR clnt for FTM insert");
							break;
  					}
						fn_userlog(c_ServiceName,"After insert into FTM for download msg");

			  		/** Added POST UAT for FTM insert Ends **/

						if (c_breco_start_flg == 'Y')
						{
							strcpy( c_pgm_name,"bat_fo_Brecon" );
							sprintf( c_cmd_str, "nohup %s %s %s %s &", c_pgm_name, sql_c_xchng_cd, sql_c_xchng_cd, sql_c_pipe_id );
		  				fn_userlog( c_ServiceName,"All three types-Download Completed. Now Process Queued/Lmt mismatcg records via RECO Pgm :%s:", c_cmd_str );
						 /** system( c_cmd_str ); **/  /** Commented for initial  LIVE version-- Will be uncommented later **/ 
			 			}

		 			 }	
        	 i_cond_snd_rcv_cntrl = DWNLD_RES_RCVD;
           pthread_cond_signal(&cond_snd_rcv_cntrl);
     
           i_call_rcv = NO;
      break;
    /************************************************************************************/
			/** Ver 1.6 Start **/
      case TID_NEW_ORDER_RESPONSE: 			/** 10101 **/
      case TID_MODIFY_ORDER_RESPONSE: 	/** 10107 **/
      case TID_DELETE_ORDER_RESPONSE: 	/** 10110 **/
      case TID_DELETE_ORDER_BROADCAST: 	/** 10112 **/
			case TID_ORDER_EXEC_REPORT_BROADCAST: /** 10117 **/
			case TID_REJECT: /** 10010 **/
					 
					 st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;

           if(DEBUG_MSG_LVL_3)
           {
            fn_userlog ( c_ServiceName, " Inside TID_ORDER Case in receive thread");
            fn_userlog ( c_ServiceName, " Template ID:%ld:",st_comm_hdr.TemplateID);
           }

						if(st_comm_hdr.TemplateID==TID_REJECT)
						{
						fn_userlog(c_ServiceName," Inside Reject");
						}

           i_read_size = sizeof(union st_rcv_reply_data);
           i_call_rcv = ACK;

           if(DEBUG_MSG_LVL_3)
           {
            fn_userlog ( c_ServiceName, "Val of i_read_size in ORDER_RESPONSE <%d>", i_read_size);
           }

      break;
      case TID_ORDER_EXEC_NOTIFICATION: /** 10104 **/
			case TID_TRADE_BROADCAST: /** 10500 **/

					st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;

           if(DEBUG_MSG_LVL_3)
           {
            fn_userlog ( c_ServiceName, " Inside BSE_ORD_TRADE Case in receive thread");
           }
							i_read_size = sizeof(union st_rcv_reply_data);

           i_call_rcv = TRD;

          if(DEBUG_MSG_LVL_3)
          {
           fn_userlog ( c_ServiceName, "Val of i_read_size in BSE_ORD_TRADE <%d>", i_read_size);
          }

      	break;
     case TID_ORDER_EXEC_RESPONSE: /** 10103 **/
  
         st_buf_dat = (union st_rcv_reply_data *)&tmp_buf_dat;

           if(DEBUG_MSG_LVL_0)
           {
            fn_userlog ( c_ServiceName, " Inside ACKTRD case in receive thread");
           }
              i_read_size = sizeof(union st_rcv_reply_data);

           i_call_rcv = ACKTRD;

          if(DEBUG_MSG_LVL_0)
          {
           fn_userlog ( c_ServiceName, "Val of i_read_size in ACKTRD case <%d>", i_read_size);
          }

        break;
    /**** Case to process Order request from Exchange -  Start on 20-Oct-2011 ***/

  		case LIMIT_MARKET_ORD: /** 1025 **/

	  		 	 if(DEBUG_MSG_LVL_3)
				 	 {
				  	fn_userlog ( c_ServiceName, " Inside LIMIT_MARKET_ORD Case in receive thread"); 
				 	 }

	 				 i_read_size = sizeof(struct st_ord_rqst_rply); 
	  			 i_call_rcv = ACK; 

					 if(DEBUG_MSG_LVL_3)
					 {
	  			  fn_userlog ( c_ServiceName, "Val of i_read_size in LIMIT_MARKET_ORD <%d>", i_read_size); 
        	 }

      break;	

    /**** Case to process Order request from Exchange -  End   on 20-Oct-2011 ***/

    /************************************************************************************/
    /**** Case to process Converted Market to Limit Order from Exchange -  Start on 14-Nov-2011 ***/

  		case CONVRTED_MKT_TO_LMT: /** 1530 **/

	  			 if(DEBUG_MSG_LVL_3)
				   {
				    fn_userlog ( c_ServiceName, " Inside CONVRTED_MKT_TO_LMT Case in receive thread"); 
				   }

	 				 i_read_size = sizeof(struct st_mkrt_to_lmt_ord); 
	  			 i_call_rcv = ACK; 

					if(DEBUG_MSG_LVL_3)
					{
	  			 fn_userlog ( c_ServiceName, "Val of i_read_size in CONVRTED_MKT_TO_LMT <%d>", i_read_size); 
        	}
      break;
    /**** Case to process Trade Confirmation from Exchange -  Start on 03-Nov-2011 ***/

  		case BSE_TRADE_CONFIRMATION: /** 1521 **/

	  			 if(DEBUG_MSG_LVL_3)
				   {
				    fn_userlog ( c_ServiceName, " Inside BSE_TRADE_CONFIRMATION Case in receive thread"); 
				   }

	 				 i_read_size = sizeof(struct st_trd_cnfrmtn); 
	  			 i_call_rcv = TRD; 

					if(DEBUG_MSG_LVL_3)
					{
	  			 fn_userlog ( c_ServiceName, "Val of i_read_size in BSE_TRADE_CONFIRMATION <%d>", i_read_size); 
        	}

      break;	
    /**** Case to process Trade Confirmation from Exchange - End on 03-Nov-2011 ***/
    /************************************************************************************/
    /**** Case to process Stop-loss Order request from Exchange - Start on 01-Nov-2011 ***/

  		case ADD_STOPLOSS_BUY_ORD: /** 2501 **/
		case UPD_STOPLOSS_BUY_ORD: /** 2502 **/
		case ADD_STOPLOSS_SELL_ORD: /** 2504 **/
		case UPD_STOPLOSS_SELL_ORD: /** 2505 **/
    case DEL_SLTP_BUY_AFTER_TRIGGER: /** 2509 **/ 
    case DEL_SLTP_SELL_AFTER_TRIGGER: /** 2511 **/ 

	  			 if(DEBUG_MSG_LVL_3)
				 	 {
				 		fn_userlog ( c_ServiceName, " Inside Stop-Loss-Add/Upd Case in receive thread"); 
				 	 }

	 				 i_read_size = sizeof(struct st_stoploss_ord_rply); 
	  			 i_call_rcv = ACK; 

					 if(DEBUG_MSG_LVL_3)
					 {
	  			  fn_userlog ( c_ServiceName, "Val of i_read_size in STOPLOSS_ORD <%d>", i_read_size); 
        	 }

      break;	
    /**** Case to process Stop-loss Order request from Exchange - End   on 01-Nov-2011 ***/

    /************************************************************************************/
    /**** Case to process Delete Order request from Exchange - Start on 01-Nov-2011 ***/

  		
		case DEL_STOPLOSS_BUY_ORD: /** 2503 **/
		case DEL_STOPLOSS_SELL_ORD: /** 2506 **/
		case ADD_IOC_BUY_ORDR: /** 1051 **/
		case ADD_IOC_SELL_ORDR: /** 1061 **/
    case UPD_SLTP_BUY_AFTER_TRG:
    case UPD_SLTP_SELL_AFTER_TRG:

	  			 if(DEBUG_MSG_LVL_3)
				   {
				 		fn_userlog ( c_ServiceName, " Inside Delete order Case in receive thread"); 
				   }

	 				 i_read_size = sizeof(struct st_del_ord_rply); 
	  			 i_call_rcv = ACK; 

					 if(DEBUG_MSG_LVL_3)
					 {
	  			  fn_userlog ( c_ServiceName, "Val of i_read_size in Delete_ORD <%d>", i_read_size); 
        	 }

      break;	

    /**** Case to process Delete Order request from Exchange - End   on 01-Nov-2011 ***/


    /****************************************************************************************************/
    /**** Case to process response KILL_MIN_FILL_ORDR -IOC from Exchange - Start on 16-Nov-2011 ***/

  		
		case KILL_MIN_FILL_ORDR:

	  			 
				 	 if(DEBUG_MSG_LVL_3)
				 	 {
				 		fn_userlog ( c_ServiceName, " Inside KILL_MIN_FILL_ORDR in receive thread"); 
				 	 }

	 				 i_read_size = sizeof(struct st_kill_min_fill_ordr); 
	  			 i_call_rcv = ACK; 

					if(DEBUG_MSG_LVL_3)
					{
	  			 fn_userlog ( c_ServiceName, "Val of i_read_size in KILL_MIN_FILL_ORDR <%d>", i_read_size); 
        	}

      break;	

    /**** Case to process Delete Order request from Exchange -  End   on 01-Nov-2011 ***/
    /************************************************************************************/

    /**** Case to process STOP LOSS Trigger from Exchange -  Start on 25-Nov-2011 ***/

  		case STOPLOSS_ORDR_UMS:

	  			 if(DEBUG_MSG_LVL_3)
				   {
				    fn_userlog ( c_ServiceName, " Inside STOPLOSS_ORDR_UMS Case in receive thread"); 
				   }

	 				 i_read_size = sizeof(struct st_sltp_ord_ums); 
	  			 i_call_rcv = ACK;   

					if(DEBUG_MSG_LVL_3)
					{
	  			 fn_userlog ( c_ServiceName, "Val of i_read_size in STOPLOSS_ORDR_UMS in RCV thread<%d>", i_read_size); 
        	}

      break;	
    /**** Case to process STOP LOSS Trigger from Exchange -  End on 25-Nov-2011 ***/
    /************************************************************************************/

    /** Case to process SLOT_NOT_FREE - Start on 12-Apr-2012  **/	

    case SLOT_NOT_FREE:
      
       if(DEBUG_MSG_LVL_3)
       {
       fn_userlog ( c_ServiceName, " Inside SLOT_NOT_FREE Case in receive thread"); 
       }
       i_call_rcv = NO;   
			 if(DEBUG_MSG_LVL_3)
			 {
				fn_userlog ( c_ServiceName, "Val of l_seq_no in SLOT_NOT_FREE in RCV thread<%ld>", l_seq_no); 
			 }


  		i_trnsctn = fn_begintran( c_ServiceName, c_errmsg );
  		if ( i_trnsctn == -1)
  		{
    		fn_errlog(c_ServiceName,"S31235", LIBMSG, c_errmsg);
  		}

  		EXEC SQL
    		UPDATE fxb_fo_xchng_book
    		SET FXB_PLCD_STTS = 'R'
    		WHERE FXB_ORDR_SQNC = :l_seq_no
    		AND FXB_PIPE_ID = :sql_c_pipe_id
    		AND FXB_XCHNG_CD = :sql_c_xchng_cd
    		AND trunc(FXB_MOD_TRD_DT) = :sql_c_nxt_trd_date;

  		if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
  		{
    		fn_errlog(c_ServiceName,"S31240", SQLMSG, c_errmsg);
    		fn_userlog(c_ServiceName,"Error in updating FXB status to 'R' ");
    		fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
    		i_rcv_thrd_exit_stts = EXIT;
    		continue;
  		}

			if(DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName,"After update FXB placed status to 'R' for :%ld: for pipe id %s: with l_seq_no<%ld>",l_seq_no, sql_c_pipe_id, l_seq_no);
			}


  		i_ch_val = fn_committran( c_ServiceName, i_trnsctn, c_errmsg );
  		if ( i_ch_val == -1 )
  		{
    		fn_errlog(c_ServiceName,"S31245", LIBMSG, c_errmsg);
  		}

			break;

    	/** Case to process SLOT_NOT_PRESENT - End on 12-Apr-2012  **/	

    /**************************************************************************************************/
		 /***** Ver 1.3 Starts *********/

  		case BCAST_TRDR_LIMIT_EXCEED: /** 25100 **/

	  			 if(DEBUG_MSG_LVL_3)
				   {
				    fn_userlog ( c_ServiceName, " Inside BCAST_TRDR_LIMIT_EXCEED Case in receive thread"); 
				   }

	 				 i_read_size = sizeof(struct st_trd_lmt_exceed); 
	  			 i_call_rcv = ACK;   

					if(DEBUG_MSG_LVL_3)
					{
	  			 fn_userlog ( c_ServiceName, "Val of i_read_size in BCAST_TRDR_LIMIT_EXCEED in RCV thread<%d>", i_read_size); 
        	}
      break;	
			case POS_ALERT_UMS:						/** 1921  **/
	  			 if(DEBUG_MSG_LVL_3)
				   {
				    fn_userlog ( c_ServiceName, " Inside POS_ALERT_UMS Case in receive thread"); 
				   }

	 				 i_read_size = sizeof(struct st_pos_alrt_ums); 
	  			 i_call_rcv = ACK;   

					if(DEBUG_MSG_LVL_3)
					{
	  			 fn_userlog ( c_ServiceName, "Val of i_read_size in POS_ALERT_UMS in RCV thread<%d>", i_read_size); 
        	}
      break;	

  		case BROKER_SUSPENDED_UMS: 		/** 24004 **/

	  			 if(DEBUG_MSG_LVL_3)
				   {
				    fn_userlog ( c_ServiceName, " Inside BROKER_SUSPENDED_UMS Case in receive thread"); 
				   }

	 				 i_read_size = sizeof(struct st_broker_susp); 
	  			 i_call_rcv = ACK;   

					if(DEBUG_MSG_LVL_3)
					{
	  			 fn_userlog ( c_ServiceName, "Val of i_read_size in BROKER_SUSPENDED_UMS in RCV thread<%d>", i_read_size); 
        	}
      break;	

  		case TRADER_SUSPENDED_UMS:					/*** 1524 ***/

	  			 if(DEBUG_MSG_LVL_3)
				   {
				    fn_userlog ( c_ServiceName, " Inside TRADER_SUSPENDED_UMS Case in receive thread"); 
				   }

	 				 i_read_size = sizeof(struct st_trader_susp); 
	  			 i_call_rcv = ACK;   

					if(DEBUG_MSG_LVL_3)
					{
	  			 fn_userlog ( c_ServiceName, "Val of i_read_size in TRADER_SUSPENDED_UMS in RCV thread<%d>", i_read_size); 
        	}
      break;	

  		case MEMBER_REACTIVATED_UMS:			/*** 1528 ***/

	  			 if(DEBUG_MSG_LVL_0)
				   {
				    fn_userlog ( c_ServiceName, " Inside MEMBER_REACTIVATED_UMS Case in receive thread"); 
				   }

	 				 i_read_size = sizeof(struct st_mbr_reactv); 
	  			 i_call_rcv = ACK;   

					if(DEBUG_MSG_LVL_0)
					{
	  			 fn_userlog ( c_ServiceName, "Val of i_read_size in MEMBER_REACTIVATED_UMS in RCV thread<%d>", i_read_size); 
        	}
      break;	

  		case RRM_UMS:				/** 1920 **/

	  		 if(DEBUG_MSG_LVL_0)
				 {
				   fn_userlog ( c_ServiceName, " Inside RRM_UMS Case in receive thread"); 
				 }
	 			 i_read_size = sizeof(struct st_rrm_alrt_ums); 
	  		 i_call_rcv = ACK;   

				 if(DEBUG_MSG_LVL_0)
				 {
	  			 fn_userlog(c_ServiceName,"Val of i_read_size in RRM_UMS in RCV thread<%d>", i_read_size);
        }
      break;	
  		case CAP_ADEQ_UMS_70: /** 24001 **/
  		case CAP_ADEQ_UMS_80: /** 24002 **/
  		case CAP_ADEQ_UMS_85: /** 24003 **/

	  		 if(DEBUG_MSG_LVL_0)
				 {
				   fn_userlog ( c_ServiceName, " Inside CAP_ADEQ_UMS Case in receive thread"); 
				 }
	 			 i_read_size = sizeof(struct st_cap_adeq_ums); 
	  		 i_call_rcv = ACK;   

				 if(DEBUG_MSG_LVL_0)
				 {
	  			 fn_userlog(c_ServiceName,"Val of i_read_size in CAP_ADEQ_UMS in RCV thread<%d>", i_read_size);
        }
      break;	
  		case CANCELLED_ORDR_UMS: /** 3233 **/

	  		 if(DEBUG_MSG_LVL_0)
				 {
				   fn_userlog ( c_ServiceName, " Inside CANCELLED_ORDR_UMS Case in receive thread"); 
				 }
	 			 i_read_size = sizeof(struct st_cancl_ordr_ums); 
	  		 i_call_rcv = ACK;   

				 if(DEBUG_MSG_LVL_0)
				 {
	  			 fn_userlog(c_ServiceName,"Val of i_read_size in CAP_ADEQ_UMS in RCV thread<%d>", i_read_size);
        }
      break;
			case PROTOCOL_MSG_CONN_REF :/** 800 **/

				 fn_userlog ( c_ServiceName, " Inside PROTOCOL_MSG_CONN_REF Case in receive thread"); 
	 			 i_read_size = sizeof(struct st_conn_refused_reply); 
	  		 i_call_rcv = ACK;   
	  		 fn_userlog(c_ServiceName,"Val of i_read_size in PROTOCOL_MSG_CONN_REF in RCV thread<%d>", i_read_size);
					 break;
	
		/**** Ver 1.3 Ends ***************/
		/**** Ver 1.4 Starts ****/
			case CAP_UTLZTN_UMS :					/** 1922 **/
				 fn_userlog(c_ServiceName," Inside CAP_UTLZTN_UMS case in receive thread");
				 i_read_size = sizeof(struct st_cap_utlztn);
				 i_call_rcv = ACK;
				 fn_userlog(c_ServiceName,"Val of i_read_size in CAP_UTLZTN_UMS in RCV thread<%d>",i_read_size);
				 break;
		/**** Ver 1.4 Ends ****/
    /*******************************************************************************/
			/** Ver 1.5 starts **/
			case ORDR_RATE_UMS :
      case RESUME_ORDR_REQ_UMS:

				fn_userlog(c_ServiceName," Inside Order Rate UMS ");
				i_read_size = sizeof(struct st_ordr_rate_ums);
				i_call_rcv = ACK;
				fn_userlog(c_ServiceName,"Val of i_read_size in Order Rate UMS<%d>",i_read_size);
				break;
			/** Ver 1.5 ends ***/
      default :
          fn_userlog ( c_ServiceName, "Warning -Invalid message type in rcv thrd-<%ld>", st_comm_hdr.TemplateID ); /* Ver 1.6 */
	 			  i_call_rcv = NO;
      break;

		  fn_userlog(c_ServiceName, "After successful call to fn_logon_to_bse- Errr Msg <%s>", c_err_msg);
    }  /** Switch condition for l_msg_type ends **/

    /** Ver 1.7 starts **/
    if (i_dwnld_flag == ORDR_DWNLD)
    {
      i_orddwnld_cntr++;
      fn_userlog(c_ServiceName,"i_dwnld_cntr in ORDR_DWNLD is :%d:",i_orddwnld_cntr);

			/*
			if (DEBUG_MSG_LVL_5)
			{
      	fn_userlog(c_ServiceName,"c_RefApplLastSeqNum is %s:",c_RefApplLastSeqNum);
      	fn_userlog(c_ServiceName,"c_EndApplSeqNum is :%s:",c_EndApplSeqNum);
      	fn_userlog(c_ServiceName,"c_EndAppllastbgn is :%s:",c_EndAppllastbgn);
			}*/

      if (i_orddwnld_cntr >= i_total_msgs )
      {
				if ( i_total_msgs - 1 != 0 )
				{
        	i_dwnld_flag = 0; 
        	/*if( (memcmp(st_buf_dat->st_var_ordr_dwnld.ApplEndMsgID,st_buf_dat->st_var_ordr_dwnld.RefApplLastMsgID,16) != 0) &&
							(memcmp(c_EndAppllastbgn,st_buf_dat->st_var_ordr_dwnld.ApplEndMsgID,16) != 0))*/

					 if( (strcmp(c_RefApplLastSeqNum,c_EndApplSeqNum) != 0) &&
              (strcmp(c_EndAppllastbgn,c_EndApplSeqNum) != 0))
        	{
						memset(c_EndAppllastbgn,0,16);
						memcpy(c_EndAppllastbgn,st_buf_dat->st_var_ordr_dwnld.ApplEndMsgID,sizeof(c_EndAppllastbgn));

          	fn_retrnsmt_rqst(c_EndApplSeqNum,
                           i_partition_id,
                           c_ServiceName,
                           c_errmsg);

             if ( i_ch_val == -1 )
             {
              fn_userlog(c_ServiceName,"Error in Error fn_retrnsmt_rqst");
             }
        	}
        	else
        	{
						memset(c_EndAppllastbgn,0,16);
          	fn_userlog(c_ServiceName,"Order Download Completed and trigger posted i_partition_id :%d:",i_partition_id);
          	fn_pst_trg ( c_ServiceName, "TRG_RECO_OVER", "TRG_RECO_OVER", sql_c_pipe_id );
        	}
				}
				else
				{
					memset(c_EndAppllastbgn,0,16);
					fn_userlog(c_ServiceName,"Order Download Completed and trigger posted 2 i_partition_id :%d: i_total_msgs |%d|",i_partition_id,
i_total_msgs);
          fn_pst_trg ( c_ServiceName, "TRG_RECO_OVER", "TRG_RECO_OVER", sql_c_pipe_id );
				}
      } 
    }
    else if (i_dwnld_flag == TRD_DWNLD)
    {
      i_trddwnld_cntr++;
      fn_userlog(c_ServiceName,"i_dwnld_cntr in TRD_DWNLD is :%d:",i_trddwnld_cntr);
      fn_userlog(c_ServiceName,"ll_append_seq_num is %lld:",ll_append_seq_num);
      fn_userlog(c_ServiceName,"ll_RefApplLastSeqNum is :%lld:",ll_RefApplLastSeqNum);

      if (i_trddwnld_cntr >= i_total_msgs)
      { 
        if( ( i_total_msgs - 1) != 0)
        { 
           i_dwnld_flag = 0; 
           fn_userlog(c_ServiceName,"i_partition_id :%d: ll_append_seq_num :%lld:",i_partition_id,ll_append_seq_num);

           if((ll_append_seq_num > 0) && (ll_append_seq_num != ll_RefApplLastSeqNum))
           {
             i_ch_val= fn_retrnsmt_trd_rqst( ll_append_seq_num ,
                                             i_partition_id,
                                             c_ServiceName,
                                             c_errmsg);
             if ( i_ch_val == -1 )
             {
              fn_userlog(c_ServiceName,"Error in Error fn_retrnsmt_trd_rqst");
             }
           }
           else
           {
             fn_userlog(c_ServiceName,"Trade Download Completed and trigger posted :%d: i_partition_id :%d:",i_total_msgs,i_partition_id);
             fn_pst_trg ( c_ServiceName, "TRG_RECO_OVER_TRD", "TRG_RECO_OVER_TRD", sql_c_pipe_id );
           }
         }
         else
         {
           fn_userlog(c_ServiceName,"Trade Download Completed and trigger posted 2 :%d: i_partition_id :%d:",i_total_msgs,i_partition_id);
           fn_pst_trg ( c_ServiceName, "TRG_RECO_OVER_TRD", "TRG_RECO_OVER_TRD", sql_c_pipe_id );
         } 
      } 
    }
    /**  Ver 1.7 Ends  **/

	  /*************************************************/

    if( i_call_rcv == ACK || i_call_rcv == TRD || i_call_rcv == SLTP || i_call_rcv == ACKTRD)  /** ACKTRD added in ver 1.6 **/ 
		{
     INIT( i_err,  TOTAL_FML );
   	 INIT( i_ferr, TOTAL_FML );

		 if(DEBUG_MSG_LVL_5)
		 {
	  	fn_userlog(c_ServiceName,"i_read_size in rcv_thrd :%d:",i_read_size);
	  	fn_userlog(c_ServiceName,"i_dwnld_flg in rcv_thrd :%d:",i_dwnld_flg);
	  	fn_userlog(c_ServiceName,"sql_c_nxt_trd_date in rcv_thrd :%s:",sql_c_nxt_trd_date.arr);
	  	fn_userlog(c_ServiceName,"sql_c_pipe_id in rcv_thrd  :%s:",sql_c_pipe_id);
	  	fn_userlog(c_ServiceName,"sql_c_xchng_cd in rcv_thrd  :%s:",sql_c_xchng_cd);
	  	fn_userlog(c_ServiceName,"sql_c_trd_ref in rcv_thrd  :%s:",sql_c_trd_ref);
	  	fn_userlog(c_ServiceName,"i_ptr_size Size :%d:",i_ptr_size);  /* Ver 1.6 */
		 }

			 fn_userlog(c_ServiceName," Fsizeof32:%ld: Fused32:%ld: Funused32 :%ld: Fneeded32 :%ld:",Fsizeof32(ptr_fml_Sbuf),Fused32(ptr_fml_Sbuf),Funused32(ptr_fml_Sbuf),Fneeded32(1,i_read_size));   /** Ver 1.6 **/

		 i_err[0] = Fchg32( ptr_fml_Sbuf, FFO_CBUF, 0,(char *)st_buf_dat, i_ptr_size );	/** Changed i_ptr_size from i_read_size in ver 1.6 **/
		 i_ferr[0] = Ferror32;
		 i_err[1] =  Fchg32( ptr_fml_Sbuf, FFO_TMPLT, 0, (char *)&i_ptr_size, 0 );	/** Changed i_ptr_size from i_read_size in ver 1.6 **/
		 i_ferr[1] = Ferror32;
		 i_err[2] = Fchg32( ptr_fml_Sbuf, FFO_BIT_FLG, 0, (char *)&i_dwnld_flg, 0 );
		 i_ferr[2] = Ferror32;
		 i_err[3] = Fchg32( ptr_fml_Sbuf, FFO_NXT_TRD_DT, 0 , (char *)sql_c_nxt_trd_date.arr, 0 );
		 i_ferr[3] = Ferror32;
		 i_err[4] = Fchg32( ptr_fml_Sbuf, FFO_PIPE_ID,0, (char *)sql_c_pipe_id, 0 );
		 i_ferr[4] = Ferror32;
 	   i_err[5] = Fchg32( ptr_fml_Sbuf, FFO_XCHNG_CD,0, (char *)sql_c_xchng_cd, 0 );
		 i_ferr[5] = Ferror32;
		 i_err[6] = Fchg32( ptr_fml_Sbuf, FFO_REF_NO, 0,(char *)sql_c_trd_ref, 0 );
		 i_ferr[6] = Ferror32;
		 i_err[7] = Fchg32( ptr_fml_Sbuf, FFO_ORDR_RFRNC, 0,(char *)c_skip_ordref, 0 );
		 i_ferr[7] = Ferror32;
		 i_err[8] = Fchg32( ptr_fml_Sbuf, FFO_DWNLD_FLG, 0,(char *)&i_deferred_dwnld_flg, 0 );
		 i_ferr[8] = Ferror32;

	   for( i_loop=0; i_loop<=8; i_loop++ )
	   {
		  if(i_err[i_loop] == -1)
		  {
			 fn_userlog(c_ServiceName,"Error [%d] in Fchange at [%d], for pipe [%s] ", i_ferr[i_loop],i_loop, sql_c_pipe_id );
			 fn_userlog(c_ServiceName,"Error Msg [%s]" , Fstrerror32(i_ferr[i_loop])); /* Ver 1.6 */
			 i_brk_stts = EXIT;
			 break;
		  }
	   }

		 if(i_brk_stts == EXIT)
		 {
			i_rcv_thrd_exit_stts = EXIT;
			break;
		 }


		 if( i_call_rcv == ACK ) 
		 {
			 
			 if(DEBUG_MSG_LVL_3)
			 {
			 fn_userlog ( c_ServiceName, "Before calling SFO_BRCV_CLNT for |%ld|", l_msg_type);
			 }
			 i_ch_val = tpacall("SFO_BRCV_CLNT", (char *)ptr_fml_Sbuf, 0, TPNOREPLY);
			 if( i_ch_val == -1 )
			 {
		   	fn_userlog(c_ServiceName,"Error in call sfo_bse_rcv_clnt for pipe :%s:",sql_c_pipe_id);
		   	fn_errlog(c_ServiceName, "S31250", LIBMSG, c_errmsg);
		   	i_rcv_thrd_exit_stts = EXIT;
		   	continue;
			 }
		   if(i_ch_val == 0)
		   {
		    fn_userlog ( c_ServiceName, "Called SFO_BRCV_CLNT Successfully for |%ld|", l_msg_type);
		   }
	   }
	   else if ( i_call_rcv == TRD ) 
		 {
			 

			 if(DEBUG_MSG_LVL_3)
			 {
			 fn_userlog ( c_ServiceName, "Before calling SFO_BTRD_CLNT for |%ld|", l_msg_type);
			 }
			 i_ch_val = tpacall("SFO_BTRD_CLNT", (char *)ptr_fml_Sbuf, 0, TPNOREPLY);
			 if( i_ch_val == -1 )
			 {
		   	fn_userlog(c_ServiceName,"Error in call sfo_btrd_clnt for pipe :%s:",sql_c_pipe_id);
		   	fn_errlog(c_ServiceName, "S31255", LIBMSG, c_errmsg);
		   	i_rcv_thrd_exit_stts = EXIT;
		   	continue;
			 }
		   if(DEBUG_MSG_LVL_3)
		   {
		    fn_userlog ( c_ServiceName, "Called SFO_BTRD_CLNT Successfully for |%ld|", l_msg_type);
		   }
	   }
    /** Ver 1.6 start **/
     else if ( i_call_rcv == ACKTRD )
     {

       if(DEBUG_MSG_LVL_0)
       {
        fn_userlog ( c_ServiceName, " Template ID:%ld:",st_comm_hdr.TemplateID);
        fn_userlog ( c_ServiceName, "Before calling SFO_BRCV_CLNT for ACKTRD |%ld|", l_msg_type);
       }
       i_ch_val = tpacall("SFO_BRCV_CLNT", (char *)ptr_fml_Sbuf, 0, TPNOREPLY);
       if( i_ch_val == -1 )
       {
        fn_userlog(c_ServiceName,"Error in call sfo_bse_rcv_clnt for pipe :%s:",sql_c_pipe_id);
        fn_errlog(c_ServiceName, "S31260", LIBMSG, c_errmsg);
        i_rcv_thrd_exit_stts = EXIT;
        continue;
       }
       if(i_ch_val == 0)
       {
        fn_userlog ( c_ServiceName, "Called SFO_BRCV_CLNT Successfully for ACKTRD|%ld|", l_msg_type);
       }

       if(DEBUG_MSG_LVL_0)
       {
        fn_userlog ( c_ServiceName, " Template ID:%ld:",st_comm_hdr.TemplateID);
       fn_userlog ( c_ServiceName, "Before calling SFO_BTRD_CLNT for |%ld|", l_msg_type);
       }
       i_ch_val = tpacall("SFO_BTRD_CLNT", (char *)ptr_fml_Sbuf, 0, TPNOREPLY);
       if( i_ch_val == -1 )
       {
        fn_userlog(c_ServiceName,"Error in call sfo_btrd_clnt for pipe :%s:",sql_c_pipe_id);
        fn_errlog(c_ServiceName, "S31265", LIBMSG, c_errmsg);
        i_rcv_thrd_exit_stts = EXIT;
        continue;
       }
       if(DEBUG_MSG_LVL_0)
       {
        fn_userlog ( c_ServiceName, "Called SFO_BTRD_CLNT Successfully for ACKTRD |%ld|", l_msg_type);
       }
     }
    /** Ver 1.6 ends **/

    }


		pthread_mutex_unlock (&mut_snd_rcv_cntrl );

	}  /** loop for i_rcv_thrd_exit_stts == DONT_EXIT Ends **/

		tpfree( ( char *) ptr_fml_Sbuf );
		fn_lock_sig_unlock ( &cond_thrd_cntrl,
                         &mut_thrd_cntrl,
                         &i_cond_thrd_cntrl,
                         EXIT_BY_RCV_THRD );
		fn_userlog(c_ServiceName,"After successful call to fn_lock_sig_unlock-Val of i_cond_thrd_cntrl <%d>",i_cond_thrd_cntrl);

}


/**** Ver 1.2 Starts Here *****/

/*********************************************************************************/
/*  Description   :  The Keep Alive thread waits for SIGALRM to raise.           */
/*                   Once sigwait receives the SIGALRM signal, it writes the     */
/*                   Keep alive message on socket. sigwait blocks the Keep Alive */
/*                   thread untill the SIGALRM signal is raised.                 */
/*********************************************************************************/

void* fn_keep_alive_thrd( void )
{
  int i_ret_val;

  char c_ServiceName[33];
  char c_errmsg[256];
  char c_iml_header;
  char c_err_msg[256];
  sigset_t set;
  sigset_t new;

  sigemptyset(&set);
  sigaddset(&set, SIGALRM);
  pthread_sigmask(SIG_BLOCK, &set, 0);

  sigemptyset(&new);
  sigaddset(&new, SIGUSR2);
  pthread_sigmask(SIG_BLOCK, &new, NULL);

  int sig;
  int i_err;
  sigset_t sigs;
  sigset_t oldSigSet;
  sigfillset(&sigs);
  sigthreadmask(SIG_BLOCK, &sigs, &oldSigSet);

  strcpy(c_ServiceName, "fn_keep_alive_thrd");

  fn_userlog ( c_ServiceName, "inside fn_keep_alive_thrd ") ;

  i_keep_alive_thrd_exit_stts = DONT_EXIT;
  alarm(ui_keep_alive_interval);

  while ( i_keep_alive_thrd_exit_stts == DONT_EXIT)
  {
    fn_userlog ( c_ServiceName, "Inside Keep Alive DONT_EXIT state ");

    i_err = sigwait(&set, &sig);

    if(i_err)
    {
      fn_userlog ( c_ServiceName, "Inside Error Condition, Error Cd :%d:",i_err);
      i_keep_alive_thrd_exit_stts=EXIT;
      continue;
    }
    else
    {
     fn_userlog(c_ServiceName," Signal caught ");
     if( i_keep_alive_thrd_exit_stts == DONT_EXIT )
     {
       pthread_mutex_lock(&mut_thrd_cntrl);

       i_ret_val = fn_send_keepalive();

       if( i_ret_val == -1)
       {
        fn_errlog(c_ServiceName, "S31270", UNXMSG, c_errmsg);
        fn_userlog(c_ServiceName, " Error while sending keep alive packet to iml");
        /* commented in ver 1.6 pthread_mutex_unlock ( &mut_snd_rcv_cntrl); */
        pthread_mutex_unlock ( &mut_thrd_cntrl);  /** Ver 1.6 **/
        i_keep_alive_thrd_exit_stts = EXIT;
        continue;
       }
       fn_userlog(c_ServiceName,"KEEP ALIVE Message written on socket");
       alarm(ui_keep_alive_interval);
       pthread_mutex_unlock(&mut_thrd_cntrl);
     }
     else
     {
      fn_userlog(c_ServiceName,"Inside Keep Alive Exit Status ");
     }
    }
  }
}

/**** Ver 1.2 *****/
