/******************************************************************************/
/*	Program									: CLN_ESR_CLNT																		*/
/*																																						*/
/*  Input										: C_PIPE_ID																				*/
/*																																						*/
/*  Output									:																									*/
/*																																						*/
/*	Description							: Exchange send/receive client reads message from */
/*														send message queue, sends to TAP server through */
/*														TCP/IP socket.  																*/
/*														This also resposible for receiving message from */
/*														TAP and call receive client service for further */
/*														processing																			*/
/*																																						*/
/******************************************************************************/
/*	Ver 1.0		New Realease									Shailesh Hinge		23-Jun-2009 		*/
/*	Ver 1.1		Trade Confirmation Handling	  Sandeep Patil			06-Oct-2009			*/
/*	Ver 1.2		Intractive Messages handling  Sandeep Patil     07-Oct-2009     */
/*	Ver 1.3		SLTP Handeling								Sandeep Patil			15-Dec-2009			*/
/*	Ver 1.4		IOC & Exercise Handling				Sandeep Patil			05-Mar-2010			*/
/*	Ver 1.5		NNF Download Changes					Sandeep Patil			15-Mar-2010			*/
/*	Ver 1.6		Time Function Added						Sandeep Patil			03-Dec-2013			*/
/*  Ver 1.7   Exchange initiated expiry     Navina D.					18-Feb-2013     */
/*  Ver 1.8   Processing unsoli cancelation Navina D.         11-Jul-2014     */
/*            through PRCS SLTP to introduce delay 			                      */
/*  Ver 2.0   Auto Reconnect Changes  Sachin Birje            22-Jan-2015     */
/*  Ver 2.1   Login with old password.      Kishor Borkar     30-Jul-2015     */
/*  Ver 2.2   Cancellation due to SELF Trade Navina D.        01-Sep-2015     */
/*            through PRCS SLTP to introduce delay 			                      */
/*  Ver 2.3   Debug level Changes  - Sachin Birje             05-Jul-2016     */
/*  Ver 2.4   SEQ Zero and 16388 Handelling - Tanmay W.				15-Dec-2016			*/
/*  Ver 2.5   AutoReconnect Changes - Suchita D.              19-Jun-2018     */
/*	Ver 2.6		Stream No. Exchange Exception Handling Changes - Parag K.	30-Jul-2018			*/
/*	Ver 2.7	  Migration to Direct Connectivity - Parag K. 22-Feb-2019 	*/
/*  Ver 2.8   The response message length struct changed - Suchita D. 30-Aug-2019 */
/*	Ver 2.9		Reconnection & Invitation Packet Changes - Parag Kanojia	12-Sep-2019	*/
/*  Ver 3.0   Rollover with Spread Changes - Sachin Birje.    (28-Aug-2018)   */
/******************************************************************************/
/**** C header ****/
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h> /***	Ver 1.6	***/

#include <sys/types.h>				  /** Ver 2.7 **/
#include <netinet/in.h>				  /** Ver 2.7 **/
#include <fn_nw_to_host_format_nse.h> /** Ver 2.7 **/

/**** To be added for Pro c codes ****/
#include <sqlca.h>

/**** Tuxedo header ****/
#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>

/**** Application header ****/
#include <fo_fml_def.h>
#include <fo.h>
#include <fn_env.h>
#include <fn_battmpt.h>
#include <fn_tuxlib.h>
#include <fn_scklib_tap.h>
#include <fn_pthread.h>
#include <fn_signal.h>
#include <fn_msgq.h>
#include <fn_fil_log.h>
#include <fn_ddr.h>
#include <fn_log.h>
#include <userlog.h>
#include <fml_rout.h>
#include <fn_read_debug_lvl.h>
#include <fo_exg_esr_lib.h>
#include <fn_fo_fprint.h>
#include <sys/time.h>	   /* Added in Ver TOL */
#include <fo_exch_comnN.h> /* Added in Ver TOL */
#include <errno.h>
#include <signal.h>

int Z;
#define INIT(x, y)          \
	for (Z = 0; Z < y; Z++) \
	x[Z] = 0

/**** Global Variable Declaration ****/
#define TOTAL_FML 9

/*** Added In Ver 1.1	***/
#define ACK 'A'
#define TRD 'T'
#define SLTP 'S' /***	Added In Ver 1.3	***/

char c_thd_typ;
char c_rqst_for_open_ordrs;
int i_deferred_dwnld_flg;
char c_xchng_cd[3 + 1];
char c_trd_dt[20 + 1];
char c_skip_ordref[LEN_ORD_REFACK];
char c_ServiceName[33];
char c_errmsg[256];
char c_fprint_msg[1024];
char c_time[12];		/***	Ver	1.6	***/
char c_new_exg_pwd[12]; /*** Ver 2.1 ***/

int i_snd_thrd_exit_stts;
int i_rcv_thrd_exit_stts;
int i_msg_dump_flg;
int i_snd_qid;
int i_tap_sck_id;
int i_invt_cnt;
int i_exg_seq = 1;

long li_total_stream = 0; /***	Ver 1.5	***/

int i_logoff_resp = 0;		/** Ver 2.0 **/
int i_signout_err = 0;		/** Ver 2.0 **/
char c_auto_connect = '\0'; /** Ver 2.0 **/
char c_server_flg = '\0';	/** Ver 2.0 **/

FILE *ptr_rcv_msgs_file;
FILE *ptr_snd_msgs_file;

EXEC SQL BEGIN DECLARE SECTION;
char sql_c_xchng_cd[3 + 1];
varchar sql_c_nxt_trd_date[LEN_DATE];
char sql_c_pipe_id[2 + 1];
char sql_c_trd_ref[8 + 1];
varchar sql_sec_tm[LEN_DATE];
varchar sql_part_tm[LEN_DATE];
varchar sql_inst_tm[LEN_DATE];
varchar sql_idx_tm[LEN_DATE];
EXEC SQL END DECLARE SECTION;

int fn_crt_n_flushq(int *ptr_i_qid,
					char *ptr_c_qname,
					char *c_ServiceName,
					char *c_errmsg);

int fn_crt_tap_sck(int *ptr_i_sck_id,
				   char *ptr_c_tap_addr_nm,
				   char *ptr_c_tap_port_nm,
				   char *ptr_c_tap_address, /*** Added in Ver 2.9 ***/
				   char *c_ServiceName,
				   char *c_errmsg);

int fn_write_msgq_file(int *ptr_i_qid,
					   char *c_ServiceName,
					   char *c_errmsg);

void fo_call_conclnt(char c_rqst_typ,
					 char c_spl_flg,
					 char *c_ServiceName,
					 char *c_err_msg); /** Ver 2.0 **/

void fn_sigusr2_hnd(int i_signo);
void fn_sigusr1_hnd(int i_signo);

void *fn_snd_thrd();
void *fn_rcv_thrd();
void fn_gettime(void); /***	Ver	1.6	***/

void CLN_ESR_CLNT(int argc, char *argv[])
{
	char c_ServiceName[33];
	char c_errmsg[256];

	int i_ch_val;

	pthread_t snd_thrd;
	pthread_t rcv_thrd;

	pthread_attr_t snd_thrd_attr;
	pthread_attr_t rcv_thrd_attr;

	strcpy(c_ServiceName, "cln_esr_clnt");

	/*** Initialization of Send/Receive threads  ***/
	pthread_attr_init(&snd_thrd_attr);
	pthread_attr_init(&rcv_thrd_attr);

	/*** Initialize conditionla variables ***/

	pthread_mutex_init(&mut_thrd_cntrl, NULL);
	pthread_mutex_init(&mut_snd_rcv_cntrl, NULL);

	pthread_cond_init(&cond_thrd_cntrl, NULL);
	pthread_cond_init(&cond_snd_rcv_cntrl, NULL);

	pthread_attr_setscope(&snd_thrd_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setscope(&rcv_thrd_attr, PTHREAD_SCOPE_SYSTEM);

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Before starting threads");
	}

	pthread_create(&snd_thrd, &snd_thrd_attr, fn_snd_thrd, NULL);
	pthread_create(&rcv_thrd, &rcv_thrd_attr, fn_rcv_thrd, NULL);

	pthread_mutex_lock(&mut_thrd_cntrl);
	i_cond_thrd_cntrl = 0;
	i_ch_val = fn_thrd_cond_wait(&cond_thrd_cntrl,
								 &mut_thrd_cntrl,
								 c_ServiceName,
								 c_errmsg);

	switch (i_cond_thrd_cntrl)
	{
	case EXIT_BY_SIGUSR2:
		pthread_mutex_unlock(&mut_thrd_cntrl);
		c_thd_typ = 'S';
		pthread_kill(snd_thrd, SIGUSR1);
		pthread_join(snd_thrd, NULL);
		c_thd_typ = 'R';
		pthread_kill(rcv_thrd, SIGUSR1);
		pthread_join(rcv_thrd, NULL);
		break;

	case EXIT_BY_RCV_THRD:

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "EXIT_BY_RCV_THRD");
		}
		pthread_mutex_unlock(&mut_thrd_cntrl);
		pthread_join(rcv_thrd, NULL);
		c_thd_typ = 'S';
		pthread_kill(snd_thrd, SIGUSR1);
		pthread_join(snd_thrd, NULL);
		break;

	case EXIT_BY_SND_THRD:

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "EXIT_BY_SND_THRD");
		}
		pthread_mutex_unlock(&mut_thrd_cntrl);
		pthread_join(snd_thrd, NULL);
		c_thd_typ = 'R';
		pthread_kill(rcv_thrd, SIGUSR1);
		pthread_join(rcv_thrd, NULL);
		break;

	default:
		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "val of i_cond_thrd_cntrl:%d:", i_cond_thrd_cntrl);
		}
		pthread_mutex_unlock(&mut_thrd_cntrl);
		fn_errlog(c_ServiceName, "S31005", "Invalid exit type", c_errmsg);
		break;
	}

	pthread_attr_destroy(&snd_thrd_attr);
	pthread_attr_destroy(&rcv_thrd_attr);

	return;
}

void fn_bat_term(int argc, char *argv[])
{
	char c_errmsg[256];
	char c_filter[256];
	char c_pipe_id[3];	  /** Ver 2.0 **/
	char c_time_stmp[22]; /** Ver 2.0 **/
	char c_spl_flg;
	int i_err[8];
	int i_retcd;
	char c_rqst_typ;
	char c_usr_id[10];

	/** Ver 2.0 Starts Here **/
	char c_command[200];
	char c_msg[100];
	/** Ver 2.0 Ends Here **/

	strcpy(c_pipe_id, argv[3]);

	userlog("Inside fn_bat_term for pipe id :%s:", argv[3]);

	fn_close_sck(i_tap_sck_id,
				 argv[0],
				 c_errmsg);

	if (i_msg_dump_flg == LOG_DUMP)
	{
		fn_destroy_file(ptr_rcv_msgs_file);
		fn_destroy_file(ptr_snd_msgs_file);
	}

	sprintf(c_filter, "cln_pack_clnt_%s", argv[3]);
	fn_pst_trg(argv[0], "TRG_STP_BAT", "TRG_STP_BAT", c_filter);

	/**** Ver 2.0 Starst Here ***/
	userlog("i_logoff_resp :%d:", i_logoff_resp);
	userlog("i_signout_err :%d:", i_signout_err);

	if (i_logoff_resp != 1 && i_signout_err != 1)
	{

		MEMSET(c_command);
		strcpy(c_msg, "Disconected, Auto Reconnecting ....");
		sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", c_pipe_id, c_msg);
		system(c_command);

		c_spl_flg = 'N';
		c_rqst_typ = 'B';
		fo_call_conclnt(c_rqst_typ, c_spl_flg, "cln_esr_clnt", c_errmsg);
	}
	/**** Ver 2.0 Ends Here ****/

	return;
}

int fn_bat_init(int argc, char *argv[])
{
	char c_ServiceName[33];
	char c_fileName[200];
	char c_errmsg[256];
	char c_dump_msg_flg_nm[30];
	/** Ver 2.0 Starts Here **/
	i_logoff_resp = 0;
	i_signout_err = 0;
	c_auto_connect = '\0';
	c_server_flg = '\0';
	char c_spl_flg;
	char c_rqst_typ;
	char c_command[200];
	char c_msg[100];
	/** Ver 2.0 Ends Here **/
	char c_tap_address[31]; /*** Added in Ver 2.9 ***/
	MEMSET(c_tap_address);	/*** Added in Ver 2.9 ***/

	int i_ch_val;
	int i_trnsctn;
	int i_snd_seq;
	char *ptr_c_tmp;

	if (argc < 4)
	{
		fn_userlog(c_ServiceName, "Usage - cln_esr_clnt <tag qualifier> <Exchange cd> <Pipe id>");
		return (-1);
	}

	strcpy(c_ServiceName, argv[0]);
	strcpy(sql_c_pipe_id, argv[3]);
	c_auto_connect = argv[4][0]; /** Ver 2.0 **/
	c_server_flg = argv[5][0];	 /** Ver 2.0 **/

	/*** Initialize the Routing string ***/
	fn_init_ddr_pop(argv[3], TRADING_SECTION, COMMON);

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "sql_c_pipe_id  :%s:", sql_c_pipe_id);
		fn_userlog(c_ServiceName, "c_auto_connect :%c:", c_auto_connect);
		fn_userlog(c_ServiceName, "c_server_flg   :%c:", c_server_flg);
	}

	/**** Get exchange code from the database ****/
	EXEC SQL
		SELECT OPM_XCHNG_CD
			INTO : sql_c_xchng_cd
					   FROM OPM_ORD_PIPE_MSTR
						   WHERE OPM_PIPE_ID = : sql_c_pipe_id;
	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31010", SQLMSG, c_errmsg);
		return (-1);
	}

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "sql_c_xchng_cd:%s:", sql_c_xchng_cd);
	}
	/**** Get exchange next trade date from the database ****/
	EXEC SQL
		SELECT to_char(EXG_NXT_TRD_DT, 'dd-mon-yyyy'),
		to_char(EXG_NXT_TRD_DT, 'yyyymmdd'),
		to_char(EXG_SEC_TM, 'dd-mon-yyyy'),
		to_char(EXG_PART_TM, 'dd-mon-yyyy'),
		to_char(EXG_INST_TM, 'dd-mon-yyyy'),
		to_char(EXG_IDX_TM, 'dd-mon-yyyy')
			into : sql_c_nxt_trd_date,
		: sql_c_trd_ref,
		: sql_sec_tm,
		: sql_part_tm,
		: sql_inst_tm,
		: sql_idx_tm
			  FROM EXG_XCHNG_MSTR
				  WHERE EXG_XCHNG_CD = : sql_c_xchng_cd;
	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31015", SQLMSG, c_errmsg);

		return (-1);
	}

	/***  Get Send Message queue id and flush all messages   ***/
	i_ch_val = fn_crt_n_flushq(&i_snd_qid,
							   "SEND_QUEUE",
							   c_ServiceName,
							   c_errmsg);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S31020", LIBMSG, c_errmsg);
		return (-1);
	}

	/**** Create socket with TAP machine ****/
	if (c_server_flg == 'P') /** Ver 2.0 *** if condition added **/
	{
		i_ch_val = fn_crt_tap_sck(&i_tap_sck_id,
								  "TAP_IP_ADDRESS",
								  "TAP_PORT",
								  c_tap_address, /*** Added in Ver 2.9 ***/
								  c_ServiceName,
								  c_errmsg);
		if (i_ch_val == -1)
		{
			fn_errlog(c_ServiceName, "S31025", LIBMSG, c_errmsg);
			if (c_auto_connect != 'Y')
			{
				return (-1);
			}
			else /** ver 2.0 Starts here **/
			{
				MEMSET(c_command);
				strcpy(c_msg, "Disconected, Auto Reconnecting ....");
				sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
				system(c_command);

				c_spl_flg = 'N';
				c_rqst_typ = 'B';
				fo_call_conclnt(c_rqst_typ, c_spl_flg, "cln_esr_clnt", c_errmsg);
				return (-1);
			} /** ver 2.0 Ends here **/
		}
	}
	else /** Ver 2.0 Starts Here **/
	{
		i_ch_val = fn_crt_tap_sck(&i_tap_sck_id,
								  "TAP_IP_ADDRESS_FB",
								  "TAP_PORT",
								  c_tap_address, /*** Added in Ver 2.9 ***/
								  c_ServiceName,
								  c_errmsg);
		if (i_ch_val == -1)
		{
			fn_errlog(c_ServiceName, "S31030", LIBMSG, c_errmsg);
			if (c_auto_connect != 'Y')
			{
				return (-1);
			}
			else /** ver 2.0 Starts here **/
			{
				MEMSET(c_command);
				strcpy(c_msg, "Disconected, Auto Reconnecting ....");
				sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
				system(c_command);

				c_spl_flg = 'N';
				c_rqst_typ = 'B';
				fo_call_conclnt(c_rqst_typ, c_spl_flg, "cln_esr_clnt", c_errmsg);
				return (-1);
			} /** ver 2.0 Ends here **/
		}
	} /** Ver 2.0 Ends Here **/

	/**** Create send/receive message dump files	****/
	strcpy(c_dump_msg_flg_nm, "DUMP_MSG_FLG");

	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,
										c_dump_msg_flg_nm);
	if (ptr_c_tmp == NULL)
	{
		fn_errlog(c_ServiceName, "S31035", LIBMSG, c_errmsg);
		return (-1);
	}

	i_msg_dump_flg = atoi(ptr_c_tmp);

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "The Dump Log Flg is:%d:%s:", i_msg_dump_flg, ptr_c_tmp);
	}

	if (i_msg_dump_flg == LOG_DUMP)
	{
		sprintf(c_fileName, "trade/%s/%s/intmsgs_fromnse.txt",
				sql_c_xchng_cd, sql_c_pipe_id);

		if (DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName, "path name is :%s:", c_fileName);
		}

		/*** To Write a dump of messages got from Exchange ***/
		i_ch_val = fn_create_file(&ptr_rcv_msgs_file,
								  c_fileName,
								  "a+",
								  RELATIVE,
								  c_ServiceName,
								  c_errmsg);
		if (i_ch_val == -1)
		{
			fn_errlog(c_ServiceName, "S31040", (char *)UNXMSG, c_errmsg);
			return (-1);
		}

		sprintf(c_fileName, "trade/%s/%s/intmsgs_tonse.txt",
				sql_c_xchng_cd, sql_c_pipe_id);

		if (DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName, "path name is :%s:", c_fileName);
		}

		/*** To Write a dump of messages sent to Exchange ***/
		i_ch_val = fn_create_file(&ptr_snd_msgs_file,
								  c_fileName,
								  "a+",
								  RELATIVE,
								  c_ServiceName,
								  c_errmsg);
		if (i_ch_val == -1)
		{
			fn_errlog(c_ServiceName, "S31045", (char *)UNXMSG, c_errmsg);
			return (-1);
		}
	}

	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,
										"SKIP_ORD_REFSEQ");
	if (ptr_c_tmp == NULL)
	{
		fn_errlog(c_ServiceName, "S31050", LIBMSG, c_errmsg);
		return (-1);
	}

	strcpy(c_skip_ordref, ptr_c_tmp);

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Order Ref mentioned to skip:%s:", c_skip_ordref);
	}

	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,
										"DEFERRED_DOWNLOAD_FLG");
	if (ptr_c_tmp == NULL)
	{
		fn_errlog(c_ServiceName, "S31055", LIBMSG, c_errmsg);
		return (-1);
	}

	i_deferred_dwnld_flg = atoi(ptr_c_tmp);

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "Deffered Download Flg:%d:", i_deferred_dwnld_flg);
	}

	/**** Initialise socket API parameters ****/
	fn_sck_init();

	/**** Set signal handler for SIGUSR2  ****/
	if (fn_set_sgn_hnd(SIGUSR2,
					   fn_sigusr2_hnd,
					   c_ServiceName,
					   c_errmsg) == -1)
	{
		fn_errlog(c_ServiceName, "S31060", LIBMSG, c_errmsg);
		return (-1);
	}

	/**** Set signal handler for SIGUSR1  ****/
	if (fn_set_sgn_hnd(SIGUSR1,
					   fn_sigusr1_hnd,
					   c_ServiceName,
					   c_errmsg) == -1)
	{
		fn_errlog(c_ServiceName, "S31065", LIBMSG, c_errmsg);
		return (-1);
	}

	i_trnsctn = fn_begintran(c_ServiceName, c_errmsg);
	if (i_trnsctn == -1)
	{
		fn_errlog(c_ServiceName, "S31070", LIBMSG, c_errmsg);
		return (-1);
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "Connected Tap IP is :%s:", c_tap_address); /*** Added in Ver 2.9 ***/
	}

	EXEC SQL
		UPDATE opm_ord_pipe_mstr
			SET opm_login_stts = 0,
				OPM_IPO_SSSN_ID = : c_tap_address /*** Added in Ver 2.9 ***/
										WHERE opm_pipe_id = : sql_c_pipe_id;
	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31075", SQLMSG, c_errmsg);
		return (-1);
	}

	i_ch_val = fn_get_reset_seq(sql_c_pipe_id,
								sql_c_nxt_trd_date.arr,
								RESET_PLACED_SEQ,
								&i_snd_seq,
								c_ServiceName,
								c_errmsg);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "L31020", LIBMSG, c_errmsg);
		return -1;
	}

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "The TAP send sequence for pipe id :%s: after RESET:%d:", sql_c_pipe_id, i_snd_seq);
	}

	i_ch_val = fn_committran(c_ServiceName, i_trnsctn, c_errmsg);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S31080", LIBMSG, c_errmsg);
		return (-1);
	}

	fn_var = CLN_ESR_CLNT;

	return (0);
}

int fn_crt_n_flushq(int *ptr_i_qid,
					char *ptr_c_qname,
					char *c_ServiceName,
					char *c_errmsg)
{
	char *ptr_c_tmp;
	int i_ch_val;
	int i_msgq_typ;

	/***  Get IPC key  ***/
	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName, ptr_c_qname);
	if (ptr_c_tmp == NULL)
	{
		fn_errlog(c_ServiceName, "S31085", LIBMSG, c_errmsg);
		return -1;
	}
	i_msgq_typ = atoi(ptr_c_tmp);

	/***  Create a message queue  ***/
	i_ch_val = fn_crt_msg_q(ptr_i_qid,
							i_msgq_typ,
							CLIENT,
							c_ServiceName,
							c_errmsg);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S31090", LIBMSG, c_errmsg);
		return -1;
	}
	/***  Get Send Message queue id and write all messages to file before flushing it***/
	i_ch_val = fn_write_msgq_file(ptr_i_qid,
								  c_ServiceName,
								  c_errmsg);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S31095", LIBMSG, c_errmsg);
		return (-1);
	}

	/*** Flush all messages in the queue  ***/
	i_ch_val = fn_flush_q(*ptr_i_qid,
						  c_ServiceName,
						  c_errmsg);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S31100", LIBMSG, c_errmsg);
		return -1;
	}
	return 0;
}

int fn_crt_tap_sck(int *ptr_i_sck_id, char *ptr_c_tap_addr_nm, char *ptr_c_tap_port_nm, char *ptr_c_tap_address, /*** Added in Ver 2.9 ***/ char *c_ServiceName, char *c_errmsg)
{
	char *ptr_c_tmp;
	char c_tap_ip_addr[30];
	int i_tap_port_no;
	int i_ch_val;

	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,
										ptr_c_tap_addr_nm);
	if (ptr_c_tmp == NULL)
	{
		fn_errlog(c_ServiceName, "S31105", LIBMSG, c_errmsg);
		return (-1);
	}

	strcpy(c_tap_ip_addr, ptr_c_tmp);
	strcpy(ptr_c_tap_address, c_tap_ip_addr); /*** Added in Ver 2.9 ***/

	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,
										ptr_c_tap_port_nm);
	if (ptr_c_tmp == NULL)
	{
		fn_errlog(c_ServiceName, "S31110", LIBMSG, c_errmsg);
		return (-1);
	}

	i_tap_port_no = atoi(ptr_c_tmp);

	/**** Connect to TAP Machine****/
	i_ch_val = fn_crt_clnt_sck(c_tap_ip_addr,
							   i_tap_port_no,
							   ptr_i_sck_id,
							   c_ServiceName,
							   c_errmsg);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S31115", LIBMSG, c_errmsg);
		return -1;
	}

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "socket connected");
	}

	return 0;
}

void fn_sigusr2_hnd(int i_signo)
{
	fn_set_break_thrd();
	fn_lock_sig_unlock(&cond_thrd_cntrl,
					   &mut_thrd_cntrl,
					   &i_cond_thrd_cntrl,
					   EXIT_BY_SIGUSR2);
}

void fn_sigusr1_hnd(int i_signo)
{
	switch (c_thd_typ)
	{
	case 'R':
		i_rcv_thrd_exit_stts = EXIT;
		fn_set_break_sck();
		break;

	case 'S':
		i_snd_thrd_exit_stts = EXIT;
		fn_set_break_q();
		fn_set_break_thrd();
		break;

	default:
		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog("cln_esr_srvr", "Invalid thrd type");
		}
		break;
	}
}

int fn_prcs_trg(char *c_pgm_nm, int i_trg_typ)
{
	return 0;
}

/******************************************************************************/
/*  Description   :  The send thread waits on the message queue.If any message*/
/*                   is received,it sends the message on tap socket. 					*/
/*								   In case of error while processing its normal       			*/
/*                   operation, it will set the conditional variable of the   */
/*                   thread control to EXIT_BY_SND_THRD and returns to parent */
/*                   thread function                                          */
/******************************************************************************/
void *fn_snd_thrd(void)
{
	int i_ch_val;

	char c_ServiceName[33];
	char c_errmsg[256];
	char c_tap_header;

	long li_queue_msg_size;
	long li_business_data_size;
	long li_send_tap_msg_size;

	struct st_req_q_data st_req_data;
	struct st_exch_msg st_exch_msg_data;

	sigset_t new;

	sigemptyset(&new);
	sigaddset(&new, SIGUSR2);
	pthread_sigmask(SIG_BLOCK, &new, NULL);

	strcpy(c_ServiceName, "fn_snd_thrd");

	/** setting the thread status to don't exit **/
	i_snd_thrd_exit_stts = DONT_EXIT;

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "Before Entering while");
	}

	while (i_snd_thrd_exit_stts == DONT_EXIT)
	{

		MEMSET(st_req_data);
		MEMSET(st_exch_msg_data);

		li_queue_msg_size = sizeof(struct st_req_q_data);

		/** if(DEBUG_MSG_LVL_0) *** ver 2.3 **/
		if (DEBUG_MSG_LVL_3) /** Ver 2.3 **/
		{
			fn_userlog(c_ServiceName, "Before reading from Send Queue");
		}

		if (fn_read_msg_q(i_snd_qid,
						  &st_req_data,
						  &li_queue_msg_size,
						  c_ServiceName,
						  c_errmsg) == -1)
		{
			fn_errlog(c_ServiceName, "L31300", LIBMSG, c_errmsg);
			pthread_mutex_unlock(&mut_snd_rcv_cntrl);
			i_snd_thrd_exit_stts = EXIT;
			continue;
		}

		pthread_mutex_lock(&mut_snd_rcv_cntrl);

		i_cond_snd_rcv_cntrl = 0;

		memcpy(&st_exch_msg_data, &st_req_data + sizeof(long int), sizeof(st_exch_msg_data));

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "Message recieved - |%ld|", st_req_data.li_msg_type);
		}

		if (st_req_data.li_msg_type == LOGIN_WITH_OPEN_ORDR_DTLS || st_req_data.li_msg_type == LOGIN_WITHOUT_OPEN_ORDR_DTLS)
		{
			if (st_req_data.li_msg_type == LOGIN_WITH_OPEN_ORDR_DTLS)
			{
				c_rqst_for_open_ordrs = 'G';
			}
			else
			{
				c_rqst_for_open_ordrs = 'N';
			}

			if (DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName, "c_rqst_for_open_ordrs:%c:", c_rqst_for_open_ordrs);
			}

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "SIGN_ON_REQUEST_IN");
			}
			i_ch_val = fn_do_xchng_logon(&(st_req_data.st_exch_msg_data),
										 ptr_snd_msgs_file,
										 c_ServiceName,
										 c_errmsg);
			if (i_ch_val == -1)
			{
				fn_errlog(c_ServiceName, "L31300", LIBMSG, c_errmsg);
				pthread_mutex_unlock(&mut_snd_rcv_cntrl);
				i_snd_thrd_exit_stts = EXIT;
				continue;
			}
		}
		else
		{
			switch (st_req_data.li_msg_type)
			{

			case SIGN_OFF_REQUEST_IN:

				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "ORS_LOGOFF_REQ ");
				}
				i_logoff_resp = 1; /** Ver 2.0 **/

				li_business_data_size = sizeof(struct st_int_header);

				/** The opm login status is updated to NOT_LOGGED in advance becoz TAP is disconnecting
						cln_esr_clnt sometimes before giving SIGN_OFF_REQ_OUT
				***/

				i_ch_val = fn_logout_res(sql_c_xchng_cd,
										 sql_c_pipe_id,
										 c_ServiceName,
										 c_errmsg);

				if (i_ch_val == -1)
				{
					fn_userlog(c_ServiceName, "Failed While Processing Log Out Response");
					fn_errlog(c_ServiceName, "S31120", LIBMSG, c_errmsg);
				}
				break;

			case DOWNLOAD_REQUEST:

				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "ORS_DWNLD_REQ ");
				}
				li_business_data_size = sizeof(struct st_message_download);
				break;

			case BOARD_LOT_IN:
			case ORDER_MOD_IN:
			case ORDER_CANCEL_IN:

				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "ORS_NEW_ORD_REQ ");
				}
				li_business_data_size = sizeof(struct st_oe_reqres);
				break;

			/*** Ver 2.7 Starts ***/
			case BOARD_LOT_IN_TR:

				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "ORS_NEW_ORD_REQ_TRIM ");
				}
				li_business_data_size = sizeof(struct st_oe_reqres_tr);
				break;

			case ORDER_MOD_IN_TR:
			case ORDER_CANCEL_IN_TR:

				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "ORS_NEW_ORD_REQ_TRIM ");
				}
				li_business_data_size = sizeof(struct st_om_rqst_tr);
				break;
				/*** Ver 2.7 Ends ***/

			case SP_BOARD_LOT_IN:
			case TWOL_BOARD_LOT_IN:
			case THRL_BOARD_LOT_IN:
			case SP_ORDER_MOD_IN:	 /*** Ver 3.0 ***/
			case SP_ORDER_CANCEL_IN: /*** Ver 3.0 ***/

				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "ORS_SPRD_ORD_REQ ");
				}
				li_business_data_size = sizeof(struct st_spd_oe_reqres);
				break;

			case EX_PL_ENTRY_IN:
			case EX_PL_MOD_IN:
			case EX_PL_CXL_IN:

				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "ORS_NEW_EXER_REQ ");
				}

				li_business_data_size = sizeof(struct st_ex_pl_reqres);
				break;

			default:

				fn_userlog(c_ServiceName, "Fatal error - Invalid message type - %ld",
						   st_req_data.li_msg_type);
				pthread_mutex_unlock(&mut_snd_rcv_cntrl);
				i_snd_thrd_exit_stts = EXIT;
				break;
			}

			if (i_snd_thrd_exit_stts == EXIT) /**This is for Invalid message error handling **/
			{
				continue;
			}

			li_send_tap_msg_size = sizeof(st_exch_msg_data.st_net_header) + li_business_data_size;

			/******Write order packet to TAP socket **/

			if (DEBUG_MSG_LVL_2) /*** Ver 2.3 **/
			{
				fn_gettime();																	   /***  Ver 1.6 ***/
				fn_userlog(c_ServiceName, "ESR TIME_STATS Before Writing On Socket :%s:", c_time); /***  Ver 1.6 ***/
			}

			i_ch_val = fn_writen(i_tap_sck_id,
								 (void *)(&st_req_data.st_exch_msg_data),
								 li_send_tap_msg_size,
								 c_ServiceName,
								 c_errmsg);
			if (i_ch_val == -1)
			{
				fn_errlog(c_ServiceName, "L31020", LIBMSG, c_errmsg);
				pthread_mutex_unlock(&mut_snd_rcv_cntrl);
				i_snd_thrd_exit_stts = EXIT;
				continue;
			}

			if (DEBUG_MSG_LVL_2) /*** Ver 2.3 **/
			{
				fn_gettime();																	  /***  Ver 1.6 ***/
				fn_userlog(c_ServiceName, "ESR TIME_STATS After Writing On Socket :%s:", c_time); /***  Ver 1.6 ***/
			}

			/**call service for writing log **/

			if (i_msg_dump_flg == LOG_DUMP)
			{
				c_tap_header = 'Y';

				fn_fprint_log(ptr_snd_msgs_file,
							  c_tap_header,
							  (void *)(&st_req_data.st_exch_msg_data),
							  c_ServiceName,
							  c_errmsg);
			}

		} /**sign_on_req else ends **/

		if (DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName, " Message - |%ld| Successfully sent to TAP machine", st_req_data.li_msg_type);
		}

		pthread_mutex_unlock(&mut_snd_rcv_cntrl);
	}

	fn_lock_sig_unlock(&cond_thrd_cntrl,
					   &mut_thrd_cntrl,
					   &i_cond_thrd_cntrl,
					   EXIT_BY_SND_THRD);
}

/******************************************************************************/
/*  Description   :  The rcv thread waits on the tap socket.If any message is */
/*                   received,it processes resp by calling sfo_rcv_clnt.	  */
/*								   In case of error while processing its normal       			*/
/*                   operation, it will set the conditional variable of the   */
/*                   thread control to EXIT_BY_RCV_THRD and returns to parent */
/*                   thread function                                          */
/******************************************************************************/
void *fn_rcv_thrd(void)
{
	int i_ch_val;
	int i_break_stts;
	int i_err[TOTAL_FML];
	int i_ferr[TOTAL_FML];
	int i_call_rcv = YES;
	int i_read_size;
	int i_loop;
	int i_dwnld_flg;
	int i_brk_stts = DONT_EXIT;
	/** int i_signout_err=0;  ** Ver 2.0 **   commented in Ver 2.1**/
	int i_invitation_cnt = 0; /** Ver 2.0 **/
	int i_trnsctn = 0;		  /** Ver 2.1 **/

	long int li_len_buf;
	long int li_stream_cnt = 0; /***	Ver 1.5	***/

	char c_ServiceName[33];
	char c_errmsg[256];
	char c_tap_header;
	char c_ptr_data[MAX_SCK_MSG]; // size 1024
	char c_msg[256];			  /** Ver 2.0 **/
	char c_command[100];		  /** Ver 2.0 **/

	/** Ver 2.0 Starts Here **/
	int i_retcd = 0;
	char c_auto_reconnect = '\0';
	char c_rqst_typ;
	char c_spl_flg;
	char c_usr_id[10];

	/** Ver 2.0 Ends Here **/
	int i_send_login = 0; /*** Ver 2.9 ***/

	FBFR32 *ptr_fml_Sbuf;
	sigset_t new;

	union st_exch_rcv_msg *st_buf_dat;
	union st_exch_rcv_msg tmp_buf_dat;

	MEMSET(c_msg);	   /** Ver 2.0 **/
	MEMSET(c_command); /** Ver 2.0 **/

	/* Allocate the buffers */

	ptr_fml_Sbuf = (FBFR32 *)tpalloc("FML32", NULL, 2 * MAX_SCK_MSG);
	if (ptr_fml_Sbuf == NULL)
	{
		fn_errlog(c_ServiceName, "S31125", UNXMSG, c_errmsg);
		i_cond_thrd_cntrl = EXIT_BY_RCV_THRD;
		pthread_cond_signal(&cond_thrd_cntrl);
	}

	sigemptyset(&new);
	sigaddset(&new, SIGUSR2);
	pthread_sigmask(SIG_BLOCK, &new, NULL);

	strcpy(c_ServiceName, "fn_rcv_thrd");

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Receive thread started ");
	}

	/** set the receive thread's exit status to DONT_EXIT **/
	i_rcv_thrd_exit_stts = DONT_EXIT;

	/*** Ver 2.9 Starts ***/
	if (i_send_login == 0 && c_auto_connect == 'Y')
	{
		c_spl_flg = 'G';
		c_rqst_typ = 'C';
		fo_call_conclnt(c_rqst_typ, c_spl_flg, "cln_esr_clnt", c_errmsg);
		fn_userlog(c_ServiceName, "After fo_call_conclnt in i_send_login = 0 ");
		i_send_login = i_send_login + 1;
	}

	/*** Ver 2.9 Ends ***/

	while (i_rcv_thrd_exit_stts == DONT_EXIT)
	{

		i_dwnld_flg = NOT_DOWNLOAD;

		li_len_buf = sizeof(union st_exch_rcv_msg);

		if (DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName, "Before calling fn_read_xchng_sck");
		}
		// Step 1: we are setting null to union and buffer
		memset(&tmp_buf_dat, '\0', sizeof(tmp_buf_dat));
		memset(c_ptr_data, '\0', MAX_SCK_MSG);
		// Step 2: we are calcuating the current time
		if (DEBUG_MSG_LVL_2) /*** Ver 2.3 **/
		{
			fn_gettime();																		 /***  Ver 1.6 ***/
			fn_userlog(c_ServiceName, "ESR TIME_STATS Before Reading From Socket :%s:", c_time); /***  Ver 1.6 ***/
		}
		// Step3 :  we are reading from TAP socket
		if (fn_read_xchng_sck(i_tap_sck_id, c_ptr_data, 0, c_ServiceName, c_errmsg) != 0)
		{
			fn_errlog(c_ServiceName, "S31130", LIBMSG, c_errmsg);
			i_rcv_thrd_exit_stts = EXIT;
			continue;
		}
		// Step 4 : we are fetching the current time.
		if (DEBUG_MSG_LVL_2) /*** Ver 2.3 **/
		{
			fn_gettime();																		/***  Ver 1.6 ***/
			fn_userlog(c_ServiceName, "ESR TIME_STATS After Reading From Socket :%s:", c_time); /***  Ver 1.6 ***/
		}

		pthread_mutex_lock(&mut_snd_rcv_cntrl);

		/**** Validate the TAP message ****/
		// Step 5: here we are validating the NetHDR. meaning valiating the checksum and sequence number
		i_ch_val = fn_validate_tap(&i_exg_seq, c_ptr_data, c_ServiceName);
		if (i_ch_val == -1)
		{
			fn_errlog(c_ServiceName, "S31135", LIBMSG, c_errmsg);
			i_rcv_thrd_exit_stts = EXIT;
			pthread_mutex_unlock(&mut_snd_rcv_cntrl);
			continue;
		}

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "Validation Successful");
		}
		// step 6 : we recieves data into the buffer (c_ptr_data) , and we are storing it into the '&tmp_buf_dat'which is an union . i don't know how ??
		memcpy(&tmp_buf_dat, c_ptr_data + sizeof(struct st_net_hdr), sizeof(tmp_buf_dat));

		tmp_buf_dat.st_hdr.si_transaction_code = ntohs(tmp_buf_dat.st_hdr.si_transaction_code);

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "Message recieved from exchange - 1 |%d|",
					   tmp_buf_dat.st_hdr.si_transaction_code);
		}

		fn_userlog(c_ServiceName, " tmp_buf_dat.st_hdr.si_transaction_code :%d:", tmp_buf_dat.st_hdr.si_transaction_code);

		switch (tmp_buf_dat.st_hdr.si_transaction_code)
		{

		case MESSAGE_RECORD: // 7021

			st_buf_dat = (union st_exch_rcv_msg *)&(tmp_buf_dat.st_dwnldmsg.un_dw_msg);

			st_buf_dat->st_hdr.si_transaction_code = ntohs(st_buf_dat->st_hdr.si_transaction_code); /*** Ver 2.7 ***/

			i_dwnld_flg = DOWNLOAD;

			if (DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName, "Message recieved from exchange - 2 |%d|", st_buf_dat->st_hdr.si_transaction_code);
			}

			break;

		case UPDATE_LOCALDB_DATA:

			st_buf_dat = (union st_exch_rcv_msg *)&(tmp_buf_dat.st_ldb_data.u_ldbmsg);
			st_buf_dat->st_hdr.si_transaction_code = ntohs(st_buf_dat->st_hdr.si_transaction_code);			  /*** Ver 2.7 ***/
			st_buf_dat->st_innr_hdr.si_transaction_code = ntohs(st_buf_dat->st_innr_hdr.si_transaction_code); /*** Ver 2.7 ***/
			if (st_buf_dat->st_hdr.si_transaction_code == ORDER_CONFIRMATION_OUT)
			{
				st_buf_dat->st_hdr.si_transaction_code = ORS_OPN_ORD_DTLS;
			}
			/*** Ver 2.7 Starts ***/
			else
			{
				st_buf_dat->st_hdr.si_transaction_code = st_buf_dat->st_innr_hdr.si_transaction_code;
			}
			/*** Ver 2.7 Ends ***/

			if (DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName, "Message recieved from exchange - 2 |%d|", st_buf_dat->st_hdr.si_transaction_code);
			}

			fn_userlog(c_ServiceName, "Message recieved from exchange - 2 |%d|", st_buf_dat->st_innr_hdr.si_transaction_code);

			break;

		default:
			st_buf_dat = (union st_exch_rcv_msg *)&tmp_buf_dat;
			st_buf_dat->st_hdr.si_transaction_code = ntohs(st_buf_dat->st_hdr.si_transaction_code); /*** Ver 2.7 ***/
			break;
		}

		switch (st_buf_dat->st_hdr.si_transaction_code)
		{
		case INVITATION_PACKET:

			if (DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName, "INVITATION PACKET PROCESSING STARTED");
				fn_userlog(c_ServiceName, "INVITATION COUNT is:%d:", st_buf_dat->st_inv_pck.si_inv_cnt);
				fn_userlog(c_ServiceName, "Current Invitation count :%d:", i_invt_cnt);
			}

			/***			 i_invt_cnt = st_buf_dat->st_inv_pck.si_inv_cnt;	*** Commented in Ver 2.7 ***/
			i_invt_cnt = ntohs(st_buf_dat->st_inv_pck.si_inv_cnt); /*** Ver 2.7 ***/

			if (i_cond_snd_rcv_cntrl == WAITING_FOR_INVT_PCKT)
			{
				i_cond_snd_rcv_cntrl = INVT_PCKT_RCVD;
				pthread_cond_signal(&cond_snd_rcv_cntrl);
			}

			i_call_rcv = NO;

			if (DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName, "INVITATION PACKET PROCESSING ENDED");
			}

			break;

		case SIGN_ON_REQUEST_OUT:
			/*
				step 1: first we taking log
				step 2: we are going to extract the data from the actual_message
				step 3: after extracting the data we are going to convert the byte orders
				step 4: Then we are calling fn_sign_on_request_out with the extracted structure.
				step 5: After we recieve the response successfully , we are going to trigger the channel to send tthread
				step 6: after that we are going take log which will indicate that this section is done successfully .
			*/
			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "SIGN_ON_REQUEST_OUT STARTED ");
			}

			fn_cnvt_nth_sign_on_res(&st_buf_dat->st_lgn_res); /*** Ver 2.7 ***/

			i_ch_val = fn_sign_on_request_out(&(st_buf_dat->st_lgn_res),
											  c_ServiceName,
											  c_errmsg);
			if (i_ch_val == -1)
			{
				/** Ver 2.1 Starts Here **/
				i_signout_err = 1;
				MEMSET(c_new_exg_pwd);

				EXEC SQL
					SELECT nvl(OPM_NEW_EXG_PWD, 'NA')
						INTO : c_new_exg_pwd
								   FROM OPM_ORD_PIPE_MSTR
									   WHERE OPM_XCHNG_CD = : sql_c_xchng_cd
																  AND OPM_PIPE_ID = : sql_c_pipe_id;

				if (SQLCODE != 0)
				{
					fn_errlog(c_ServiceName, "S31140", LIBMSG, c_errmsg);

					if (c_auto_connect == 'Y')
					{
						MEMSET(c_command);
						strcpy(c_msg, "Reconnect to exchange not Successful,Pls check.");
						sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
						system(c_command);
					}

					i_cond_snd_rcv_cntrl = RCV_ERR;
					pthread_cond_signal(&cond_snd_rcv_cntrl);
					i_rcv_thrd_exit_stts = EXIT;
				}

				rtrim(c_new_exg_pwd);
				if (strcmp(c_new_exg_pwd, "NA") != 0)
				{
					MEMSET(c_command);
					strcpy(c_msg, "Password change request has been failed,login with old password.");
					sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
					system(c_command);

					i_trnsctn = fn_begintran(c_ServiceName, c_errmsg);
					if (i_trnsctn == -1)
					{
						fn_errlog(c_ServiceName, "S31145", LIBMSG, c_errmsg);

						if (c_auto_connect == 'Y')
						{
							MEMSET(c_command);
							strcpy(c_msg, "Reconnect to exchange not Successful,Pls check.");
							sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
							system(c_command);
						}

						i_cond_snd_rcv_cntrl = RCV_ERR;
						pthread_cond_signal(&cond_snd_rcv_cntrl);
						i_rcv_thrd_exit_stts = EXIT;
					}

					EXEC SQL
						UPDATE OPM_ORD_PIPE_MSTR
							SET OPM_NEW_EXG_PWD = NULL
								WHERE OPM_XCHNG_CD = : sql_c_xchng_cd
														   AND OPM_PIPE_ID = : sql_c_pipe_id;

					if (SQLCODE != 0)
					{
						fn_errlog(c_ServiceName, "S31150", LIBMSG, c_errmsg);
						fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
						if (c_auto_connect == 'Y')
						{
							MEMSET(c_command);
							strcpy(c_msg, "Reconnect to exchange not Successful,Pls check.");
							sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
							system(c_command);
						}

						i_cond_snd_rcv_cntrl = RCV_ERR;
						pthread_cond_signal(&cond_snd_rcv_cntrl);
						i_rcv_thrd_exit_stts = EXIT;
					}

					i_ch_val = fn_committran(c_ServiceName, i_trnsctn, c_errmsg);
					if (i_ch_val == -1)
					{
						fn_errlog(c_ServiceName, "S31155", LIBMSG, c_errmsg);
						fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
						if (c_auto_connect == 'Y')
						{
							MEMSET(c_command);
							strcpy(c_msg, "Reconnect to exchange not Successful,Pls check.");
							sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
							system(c_command);
						}

						i_cond_snd_rcv_cntrl = RCV_ERR;
						pthread_cond_signal(&cond_snd_rcv_cntrl);
						i_rcv_thrd_exit_stts = EXIT;
					}

					fn_userlog(c_ServiceName, "Password Change required failed, Login with old password.");

					c_spl_flg = 'N';
					c_rqst_typ = 'B';
					fo_call_conclnt(c_rqst_typ, c_spl_flg, "cln_esr_clnt", c_errmsg);

					i_cond_snd_rcv_cntrl = RCV_ERR;
					pthread_cond_signal(&cond_snd_rcv_cntrl);
					i_rcv_thrd_exit_stts = EXIT;

				} /** Ver 2.1 Ends Here **/
				else
				{
					fn_userlog(c_ServiceName, "in else condition");
					i_signout_err = 1; /** Ver 2.0 **/
					fn_errlog(c_ServiceName, "S31160", LIBMSG, c_errmsg);
					/** Ver 2.0 Starts Here ***/
					if (c_auto_connect == 'Y')
					{
						MEMSET(c_command);
						strcpy(c_msg, "Reconnect to exchange not Successful,Pls check.");
						sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
						system(c_command);
					}
					/** Ver 2.0 Ends Here ***/

					i_cond_snd_rcv_cntrl = RCV_ERR;
					pthread_cond_signal(&cond_snd_rcv_cntrl);
					i_rcv_thrd_exit_stts = EXIT;
				}
			}
			else
			{
				i_cond_snd_rcv_cntrl = LOGON_RESP_RCVD;
				pthread_cond_signal(&cond_snd_rcv_cntrl);
			}

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "SIGN_ON_REQUEST_OUT ENDED ");
			}

			i_call_rcv = NO;

			break;

		case SYSTEM_INFORMATION_OUT:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "SYSTEM_INFORMATION_OUT STARTED ");
			}

			fn_cnvt_nth_system_info_data(&st_buf_dat->st_sys_inf_dat); /*** Ver 2.7 ***/

			i_ch_val = fn_system_information_out(&(st_buf_dat->st_sys_inf_dat),
												 c_ServiceName,
												 c_errmsg);
			if (i_ch_val == -1)
			{
				fn_errlog(c_ServiceName, "S31165", LIBMSG, c_errmsg);
				i_cond_snd_rcv_cntrl = RCV_ERR;
				pthread_cond_signal(&cond_snd_rcv_cntrl);
				i_rcv_thrd_exit_stts = EXIT;
			}
			else
			{
				i_cond_snd_rcv_cntrl = SID_RESP_RCVD;
				pthread_cond_signal(&cond_snd_rcv_cntrl);
			}

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "SYSTEM_INFORMATION_OUT ENDED ");
			}

			i_call_rcv = NO;

			break;

		case PARTIAL_SYSTEM_INFORMATION:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "PARTIAL_SYSTEM_INFORMATION STARTED ");
			}

			fn_cnvt_nth_system_info_data(&st_buf_dat->st_sys_inf_dat); /*** Ver 2.7 ***/

			i_ch_val = fn_partial_logon_res(&(st_buf_dat->st_sys_inf_dat),
											sql_c_pipe_id,
											c_ServiceName,
											c_errmsg);

			if (i_ch_val == -1)
			{
				fn_userlog(c_ServiceName, "Failed In Function fn_partial_logon_res");
				fn_errlog(c_ServiceName, "S31170", LIBMSG, c_errmsg);
			}

			fn_errlog(c_ServiceName, "S31175", LIBMSG, c_errmsg);

			i_rcv_thrd_exit_stts = EXIT;

			i_cond_snd_rcv_cntrl = RCV_ERR;

			pthread_cond_signal(&cond_snd_rcv_cntrl);

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "PARTIAL_SYSTEM_INFORMATION ENDED ");
			}

			i_call_rcv = NO;

			break;

		case UPDATE_LOCALDB_HEADER:
			/*** ignored ***/
			i_call_rcv = NO;
			break;

		case ORS_OPN_ORD_DTLS:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "ORS_OPN_ORD_DTLS ");
			}

			i_read_size = sizeof(struct st_oe_reqres);
			/*i_call_rcv = YES;	Ver 1.1 ***/

			i_call_rcv = ACK; /*** Ver 1.1 ***/

			break;

		case UPDATE_LOCALDB_TRAILER:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "CASE UPDATE_LOCALDB_TRAILER STARTED ");
			}

			i_ch_val = fn_localdb_res(&(st_buf_dat->st_ldb_data),
									  sql_c_pipe_id,
									  c_ServiceName,
									  c_errmsg);

			if (i_ch_val == -1)
			{
				fn_userlog(c_ServiceName, "Failed In Function fn_localdb_res");
				fn_errlog(c_ServiceName, "S31180", LIBMSG, c_errmsg);
			}

			i_cond_snd_rcv_cntrl = LDB_RESP_RCVD;
			pthread_cond_signal(&cond_snd_rcv_cntrl);

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "CASE UPDATE_LOCALDB_TRAILER ENDED ");
			}
			/*** ver 2.5 started ***/
			if (c_auto_connect == 'Y' && i_dwnld_flg == NOT_DOWNLOAD)
			{
				MEMSET(c_command);
				strcpy(c_msg, "Reconnect to exchange successful.");
				sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
				system(c_command);
				fn_userlog(c_ServiceName, "Sign On Successful, Starting Pack Client");
				c_spl_flg = 'B';
				c_rqst_typ = 'D';
				fo_call_conclnt(c_rqst_typ, c_spl_flg, "cln_esr_clnt", c_errmsg);
			}
			/*** version 2.5 ended ***/
			i_call_rcv = NO;

			break;

		case SIGN_OFF_REQUEST_OUT:

			/**The OPM login status update handeled in SIGN_OFF_REQ_IN **/

			i_call_rcv = NO;
			break;

		case HEADER_RECORD:
			/*** ignored ***/
			i_call_rcv = NO;
			break;

		case PRICE_CONFIRMATION:
			/*** ignored ***/
			i_call_rcv = NO;
			break;

		case TRAILER_RECORD:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "TRAILER_RECORD ");
				fn_userlog(c_ServiceName, "TRAILER_RECORD li_stream_cnt Is :%ld:", li_stream_cnt);
			}

			/***	Ver 1.5 Starts	***/

			if (li_stream_cnt == 0)
			{
				EXEC SQL
					SELECT OPM_STREAM_NO
						INTO : li_total_stream
								   FROM OPM_ORD_PIPE_MSTR
									   WHERE OPM_XCHNG_CD = : sql_c_xchng_cd
																  AND OPM_PIPE_ID = : sql_c_pipe_id;

				if (SQLCODE != 0)
				{
					fn_errlog(c_ServiceName, "S31185", SQLMSG, c_errmsg);
				}

				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "TRAILER_RECORD li_total_stream	Is :%ld:", li_total_stream);
				}
			}

			if (li_total_stream != 0 && li_stream_cnt == 0)
			{
				li_stream_cnt = 1;
			}

			if (li_total_stream == li_stream_cnt)
			{

				fn_pst_trg(c_ServiceName, "TRG_RECO_OVER", "TRG_RECO_OVER", sql_c_pipe_id);

				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "TRAILER_RECORD ENDS");
				}

				li_stream_cnt = 0;
			}
			else
			{
				li_stream_cnt++;

				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "ELSE TRAILER_RECORD li_stream_cnt Is :%ld:", li_stream_cnt);
				}
			}

			i_call_rcv = NO;

			break;

		case ORDER_CONFIRMATION_OUT:
		case ORDER_MOD_CONFIRM_OUT:
		case ORDER_CANCEL_CONFIRM_OUT:
		case ORDER_ERROR_OUT:
		case ORDER_MOD_REJ_OUT:
		case ORDER_CXL_REJ_OUT:
		case FREEZE_TO_CONTROL:
		case BATCH_ORDER_CXL_OUT:

			/******** Commented in Ver 2.3 ********************
					if(DEBUG_MSG_LVL_0)
					{
				fn_userlog(c_ServiceName,"NORMAL_ORDER_CONFIRMATION");
				fn_userlog(c_ServiceName,"Exchange Response Got :%d:",st_buf_dat->st_hdr.si_transaction_code);
				fn_userlog(c_ServiceName,"IOC flag Got :%d:",st_buf_dat->st_oe_res.st_ord_flg.flg_ioc);	*** Ver 1.8 ***
				fn_userlog(c_ServiceName,"Error Code Got :%d:",st_buf_dat->st_oe_res.st_hdr.si_error_code);	*** Ver 1.8 ***
					}
			********** Ver  2.3 Ends Here **********************/

			if (DEBUG_MSG_LVL_0) /*** Ver 2.3 ***/
			{
				fn_userlog(c_ServiceName, "Exchange Response Got :%d:,IOC flag Got :%d:,Error Code Got :%d:", st_buf_dat->st_hdr.si_transaction_code, st_buf_dat->st_oe_res.st_ord_flg.flg_ioc, st_buf_dat->st_oe_res.st_hdr.si_error_code);
			}

			i_read_size = sizeof(struct st_oe_reqres);

			i_call_rcv = ACK; /*** Ver 1.1 ***/

			/*** if(st_buf_dat->st_hdr.si_transaction_code == ORDER_CANCEL_CONFIRM_OUT &&	st_buf_dat->st_oe_res.st_ord_flg.flg_ioc	==	1	)			***	Ver 1.4	*** Commented in Ver 1.8 ***/
			if (st_buf_dat->st_hdr.si_transaction_code == ORDER_CANCEL_CONFIRM_OUT && (st_buf_dat->st_oe_res.st_ord_flg.flg_ioc == 1 || st_buf_dat->st_oe_res.st_hdr.si_error_code == 17070 || st_buf_dat->st_oe_res.st_hdr.si_error_code == 17071 || st_buf_dat->st_oe_res.st_hdr.si_error_code == 16388 || st_buf_dat->st_oe_res.i_ordr_sqnc == 0)) /*** 17070 error code added in Ver 1.8 ***/ /*** 17071 error code added in Ver 2.2 ***/ /** Ver 2.4 16388 , sqnc is 0 **/
			{
				i_call_rcv = SLTP;
			}

			break;

			/*** Ver 2.7 Starts ***/

		case ORDER_CONFIRMATION_OUT_TR:
		case ORDER_MOD_CONFIRM_OUT_TR:
		case ORDER_CXL_CONFIRMATION_TR:

			/** i_read_size = sizeof(struct st_oe_reqres_tr); commented in ver 2.8 ***/
			i_read_size = sizeof(struct st_oe_rspn_tr); /**** ver 2.8 ****/

			fn_userlog(c_ServiceName, "Suchita :: st_buf_dat->st_oe_res_tr.ll_lastactivityref :%lld:", st_buf_dat->st_oe_res_tr.ll_lastactivityref); /*** ver 2.8 ***/

			i_call_rcv = ACK;

			if (st_buf_dat->st_hdr.si_transaction_code == ORDER_CXL_CONFIRMATION_TR && (st_buf_dat->st_oe_res_tr.st_ord_flg.flg_ioc == 1 || st_buf_dat->st_oe_res_tr.si_error_code == 17070 || st_buf_dat->st_oe_res_tr.si_error_code == 17071 || st_buf_dat->st_oe_res_tr.si_error_code == 16388 || st_buf_dat->st_oe_res_tr.li_ordr_sqnc == 0))
			{
				i_call_rcv = SLTP;
			}

			break;

			/*** Ver 2.7 Ends ***/

		case SP_ORDER_CONFIRMATION:
		case SP_ORDER_ERROR:
		case TWOL_ORDER_CONFIRMATION:
		case TWOL_ORDER_ERROR:
		case THRL_ORDER_CONFIRMATION:
		case THRL_ORDER_ERROR:
		case SP_ORDER_CXL_CONFIRM_OUT:
		case TWOL_ORDER_CXL_CONFIRMATION:
		case THRL_ORDER_CXL_CONFIRMATION:
		case SP_ORDER_MOD_CON_OUT:	  /*** Ver 3.0 ***/
		case SP_ORDER_CANCEL_REJ_OUT: /*** Ver 3.0 ***/
		case SP_ORDER_MOD_REJ_OUT:	  /*** Ver 3.0 ***/
		case BATCH_SPREAD_CXL_OUT:	  /*** Ver 3.0 ***/
			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "SP_ORDER_CONFIRMATION ");
				fn_userlog(c_ServiceName, "Exchange Response Got :%d:", st_buf_dat->st_hdr.si_transaction_code);
			}

			i_read_size = sizeof(struct st_spd_oe_reqres);

			/*i_call_rcv = YES; Ver 1.1	***/

			i_call_rcv = ACK; /*** Ver 1.1 ***/

			break;

		case ON_STOP_NOTIFICATION:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "ON_STOP_NOTIFICATION ");
			}

			i_read_size = sizeof(struct st_trade_confirm);

			/*i_call_rcv = YES;	Ver 1.1	***/

			/*i_call_rcv = ACK; *** Ver 1.1 Commented in Ver 1.3***/

			i_call_rcv = SLTP; /***	Ver 1.3	***/

			break;

		case TRADE_CONFIRMATION:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "TRADE_CONFIRMATION");
			}
			i_read_size = sizeof(struct st_trade_confirm);

			/*i_call_rcv = YES;	Ver 1.1	***/

			i_call_rcv = TRD; /*** Ver 1.1 ***/

			break;

			/*** Ver 2.7 Starts ***/

		case TRADE_CONFIRMATION_TR:

			i_read_size = sizeof(struct st_trade_confirm_tr);
			i_call_rcv = TRD;
			break;

			/*** Ver 2.7 Ends ***/

		case EX_PL_ENTRY_OUT:
		case EX_PL_MOD_OUT:
		case EX_PL_CXL_OUT:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "EX_PL_OUT");
			}

			i_read_size = sizeof(struct st_ex_pl_reqres);

			/*i_call_rcv = YES;	Ver 1.1	***/

			i_call_rcv = ACK; /*** Ver 1.1 ***/

			break;

		case EX_PL_CXL_CONFIRMATION:
		case EX_PL_MOD_CONFIRMATION:
		case EX_PL_CONFIRMATION:
		case EX_PL_ENTRY_IN:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "EX_PL_CONFIRMATION");
			}

			i_read_size = sizeof(struct st_ex_pl_reqres);

			/*i_call_rcv = YES;	Ver 1.1	***/

			/****	i_call_rcv = ACK; *** Ver 1.1 ***/
			i_call_rcv = SLTP; /*** Ver 1.4 ***/

			break;

			/***	Ver 1.2 Starts	***/

		case BCAST_SECURITY_MSTR_CHG:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "BCAST_SECURITY_MSTR_CHG");
			}

			i_read_size = sizeof(struct st_security_update_info);
			i_call_rcv = ACK;
			break;

		case BCAST_STOCK_STATUS_CHG:
		case BCAST_STOCK_STATUS_CHG_PREOPEN:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "STOCK_STATUS_CHG/STOCK_STATUS_CHG_PREOPEN");
			}
			i_read_size = sizeof(struct st_security_status_update_info);
			i_call_rcv = ACK;
			break;

		case BCAST_PART_MSTR_CHG:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "BCAST_PART_MSTR_CHG");
			}
			i_read_size = sizeof(struct st_participant_update_info);
			i_call_rcv = ACK;
			break;

		case BCAST_INSTR_MSTR_CHG:

			/***  Not Handeled  ***/

			i_call_rcv = NO;
			break;

		case BCAST_INDEX_MSTR_CHG:

			/***  Not Handeled  ***/

			i_call_rcv = NO;
			break;

		case BCAST_BASE_PRICE:

			/***  Not Handeled  ***/

			i_call_rcv = NO;
			break;

		case BCAST_INDEX_MAP_TABLE:

			/***  Not Handeled  ***/

			i_call_rcv = NO;
			break;

		case BCAST_JRNL_VCT_MSG:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "BCAST_JRNL_VCT_MSG");
			}

			i_read_size = sizeof(struct st_bcast_message);
			i_call_rcv = ACK;

			break;

		case BCAST_TURNOVER_EXCEEDED:
		case BROADCAST_BROKER_REACTIVATED:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "TURNOVER EXCEEDED/BROKER REACTIVATED");
			}

			i_read_size = sizeof(struct st_broadcast_tlimit_exceeded);
			i_call_rcv = ACK;

			break;

		case BC_OPEN_MSG:
		case BC_CLOSE_MSG:
		case BC_PRE_OR_POST_DAY_MSG:
		case BC_PRE_OPEN_ENDED:
		case EQUAL_BC_POSTCLOSE_MSG:

			/*** Handeled In Broadcast Client	***/

			i_call_rcv = NO;
			break;

		case CTRL_MSG_TO_TRADER:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "CTRL_MSG_TO_TRADER ");
			}
			i_read_size = sizeof(struct st_trader_int_msg);
			i_call_rcv = ACK;

			break;

		/*** Ver 1.2 Ends ***/

		/*** Ver 2.6 Starts ***/
		case BCAST_CONT_MSG:

			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "Inside Case BCAST_CONT_MESSAGE(EXCEPTION) in Download");
			}

			i_ch_val = fn_exch_exception_dwld(&(st_buf_dat->st_brd_exch_exception_msg),
											  sql_c_xchng_cd,
											  sql_c_pipe_id,
											  c_ServiceName,
											  c_errmsg);
			if (i_ch_val == -1)
			{
				fn_userlog(c_ServiceName, "Failed In Function fn_exch_exception_dwld");
				fn_errlog(c_ServiceName, "S31190", LIBMSG, c_errmsg);
			}

			i_call_rcv = NO;
			break;

			/*** Ver 2.6 Ends ***/

		default:
			fn_userlog(c_ServiceName, "Warning - Invalid message type - %d",
					   st_buf_dat->st_hdr.si_transaction_code);

			i_call_rcv = NO;

			break;
		}

		if (i_call_rcv == ACK || i_call_rcv == TRD || i_call_rcv == SLTP) /*** Ver 1.1 ***/ /*** SLTP Check Added In Ver 1.3	***/
		{
			INIT(i_err, TOTAL_FML);
			INIT(i_ferr, TOTAL_FML);

			/**** i_read_size will be the size of individual structure determined in above switch ****/

			if (DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName, "i_read_size:%d:", i_read_size);
				fn_userlog(c_ServiceName, "i_dwnld_flg:%d:", i_dwnld_flg);
				fn_userlog(c_ServiceName, "sql_c_nxt_trd_date:%s:", sql_c_nxt_trd_date.arr);
				fn_userlog(c_ServiceName, "sql_c_pipe_id:%s:", sql_c_pipe_id);
				fn_userlog(c_ServiceName, "sql_c_xchng_cd:%s:", sql_c_xchng_cd);
				fn_userlog(c_ServiceName, "sql_c_trd_ref:%s:", sql_c_trd_ref);
			}

			i_err[0] = Fchg32(ptr_fml_Sbuf, FFO_CBUF, 0, (char *)st_buf_dat, i_read_size);
			i_ferr[0] = Ferror32;
			i_err[1] = Fchg32(ptr_fml_Sbuf, FFO_TMPLT, 0, (char *)&i_read_size, 0);
			i_ferr[1] = Ferror32;
			i_err[2] = Fchg32(ptr_fml_Sbuf, FFO_BIT_FLG, 0, (char *)&i_dwnld_flg, 0);
			i_ferr[2] = Ferror32;
			i_err[3] = Fchg32(ptr_fml_Sbuf, FFO_NXT_TRD_DT, 0, (char *)sql_c_nxt_trd_date.arr, 0);
			i_ferr[3] = Ferror32;
			i_err[4] = Fchg32(ptr_fml_Sbuf, FFO_PIPE_ID, 0, (char *)sql_c_pipe_id, 0);
			i_ferr[4] = Ferror32;
			i_err[5] = Fchg32(ptr_fml_Sbuf, FFO_XCHNG_CD, 0, (char *)sql_c_xchng_cd, 0);
			i_ferr[5] = Ferror32;
			i_err[6] = Fchg32(ptr_fml_Sbuf, FFO_REF_NO, 0, (char *)sql_c_trd_ref, 0);
			i_ferr[6] = Ferror32;
			i_err[7] = Fchg32(ptr_fml_Sbuf, FFO_ORDR_RFRNC, 0, (char *)c_skip_ordref, 0);
			i_ferr[7] = Ferror32;
			i_err[8] = Fchg32(ptr_fml_Sbuf, FFO_DWNLD_FLG, 0, (char *)&i_deferred_dwnld_flg, 0);
			i_ferr[8] = Ferror32;

			for (i_loop = 0; i_loop <= 8; i_loop++)
			{
				if (i_err[i_loop] == -1)
				{
					fn_userlog(c_ServiceName, "Error [%d] in Fchange at [%d], for pipe [%s] ", i_ferr[i_loop], i_loop, sql_c_pipe_id);
					i_brk_stts = EXIT;
					break;
				}
			}

			if (i_brk_stts == EXIT)
			{
				i_rcv_thrd_exit_stts = EXIT;
				break;
			}

			if (i_call_rcv == ACK) /***	Ver 1.1	***/
			{
				i_ch_val = tpacall("SFO_RCV_CLNT", (char *)ptr_fml_Sbuf, 0, TPNOREPLY);
				if (i_ch_val == -1)
				{
					fn_userlog(c_ServiceName, " Error in call sfo_rcv_clnt for pipe :%s:", sql_c_pipe_id);
					fn_errlog(c_ServiceName, "S31195", LIBMSG, c_errmsg);
					i_rcv_thrd_exit_stts = EXIT;
					continue;
				}

				if (DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName, "Called SFO_RCV_CLNT Successfully for |%d|", st_buf_dat->st_hdr.si_transaction_code);
				}
			}
			else if (i_call_rcv == TRD) /***  Ver 1.1 ***/
			{
				i_ch_val = tpacall("SFO_TRD_CLNT", (char *)ptr_fml_Sbuf, 0, TPNOREPLY);
				if (i_ch_val == -1)
				{
					fn_userlog(c_ServiceName, " Error in call sfo_trd_clnt for pipe :%s:", sql_c_pipe_id);
					fn_errlog(c_ServiceName, "S31200", LIBMSG, c_errmsg);
					i_rcv_thrd_exit_stts = EXIT;
					continue;
				}

				if (DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName, "Called SFO_TRD_CLNT Successfully for |%d|", st_buf_dat->st_hdr.si_transaction_code);
				}
			}
			else if (i_call_rcv == SLTP) /***  Ver 1.3 ***/
			{
				i_ch_val = tpacall("SFO_PRCS_SLTP", (char *)ptr_fml_Sbuf, 0, TPNOREPLY);
				if (i_ch_val == -1)
				{
					fn_userlog(c_ServiceName, " Error in call sfo_prcs_sltp for pipe :%s:", sql_c_pipe_id);
					fn_errlog(c_ServiceName, "S31205", LIBMSG, c_errmsg);
					i_rcv_thrd_exit_stts = EXIT;
					continue;
				}

				if (DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName, "Called SFO_PRCS_SLTP Successfully for |%d|", st_buf_dat->st_hdr.si_transaction_code);
				}
			}
		}

		pthread_mutex_unlock(&mut_snd_rcv_cntrl);
	}

	tpfree((char *)ptr_fml_Sbuf);
	fn_lock_sig_unlock(&cond_thrd_cntrl,
					   &mut_thrd_cntrl,
					   &i_cond_thrd_cntrl,
					   EXIT_BY_RCV_THRD);
}

int fn_write_msgq_file(int *ptr_i_qid, char *c_ServiceName, char *c_errmsg)
{
	FILE *ptr_queue_msgs_file;

	unsigned int i_queue_cnt = 0;
	int i_ch_val;
	int i_cnt;
	long li_queue_msg_size;

	char c_tap_header;
	char c_fileName[200];

	struct st_req_q_data st_req_data;

	li_queue_msg_size = sizeof(struct st_req_q_data);
	c_tap_header = 'Y';

	sprintf(c_fileName, "trade/%s/%s/msgs_sendq_before_flush.txt",
			sql_c_xchng_cd, sql_c_pipe_id);

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "path name is :%s:", c_fileName);
	}

	/*** To Write a dump of messages got from Exchange ***/
	i_ch_val = fn_create_file(&ptr_queue_msgs_file,
							  c_fileName,
							  "a+",
							  RELATIVE,
							  c_ServiceName,
							  c_errmsg);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S31210", (char *)UNXMSG, c_errmsg);
		return (-1);
	}

	i_ch_val = fn_get_no_of_msg(*ptr_i_qid, &i_queue_cnt, c_ServiceName, c_errmsg);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S31215", LIBMSG, c_errmsg);
		return -1;
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "The number of message in Queue before flush and writing to Queue:%d:", i_queue_cnt);
	}

	for (i_cnt = 1; i_cnt <= i_queue_cnt; i_cnt++)
	{
		MEMSET(st_req_data);
		if (fn_read_msg_q(*ptr_i_qid,
						  &st_req_data,
						  &li_queue_msg_size,
						  c_ServiceName,
						  c_errmsg) == -1)
		{
			fn_errlog(c_ServiceName, "L31300", LIBMSG, c_errmsg);
			return -1;
		}

		fn_fprint_log(ptr_queue_msgs_file,
					  c_tap_header,
					  (void *)(&st_req_data.st_exch_msg_data),
					  c_ServiceName,
					  c_errmsg);
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "After Sucessfully written data from Queue to file:%d:", i_queue_cnt);
	}

	fn_destroy_file(ptr_queue_msgs_file);

	return 0;
}

/****************** Ver 1.6 Starts ********************************/
void fn_gettime()
{
	int i_mili_sec = 0;
	struct timeval tv;
	struct timezone tz;
	struct tm *tm;

	gettimeofday(&tv, &tz);
	tm = localtime(&tv.tv_sec);
	i_mili_sec = tv.tv_usec / 1000;
	sprintf(c_time, "%d:%d:%d:%d", tm->tm_hour, tm->tm_min, tm->tm_sec, i_mili_sec);
	return;
}

/****************** Ver 1.6 Ends ********************************/

/** Ver 2.0 Starts Here ***/

void fo_call_conclnt(char c_rqst_typ, char c_spl_flg, char *c_ServiceName, char *c_err_msg)
{
	int i_retcd = 0;
	int i_err[8];
	long l_initial_ord = 0;

	char c_auto_reconnect = '\0';
	char c_usr_id[10];
	char c_msg[256];
	char c_command[100];

	FBFR32 *ptr_fml_Ibuf;

	fn_userlog(c_ServiceName, "inside fo_call_conclnt %s", sql_c_pipe_id);

	ptr_fml_Ibuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);

	c_auto_reconnect = 'Y';
	strcpy(c_usr_id, "system");
	l_initial_ord = 0;

	i_err[0] = Fadd32(ptr_fml_Ibuf, FFO_QUEUE_NAME, (char *)sql_c_pipe_id, 0);
	i_err[1] = Fadd32(ptr_fml_Ibuf, FFO_SPL_FLG, (char *)&c_spl_flg, 0);
	i_err[2] = Fadd32(ptr_fml_Ibuf, FFO_PIPE_ID, (char *)sql_c_pipe_id, 0);
	i_err[3] = Fadd32(ptr_fml_Ibuf, FFO_RQST_TYP, (char *)&c_rqst_typ, 0);
	i_err[4] = Fadd32(ptr_fml_Ibuf, FFO_USR_ID, (char *)c_usr_id, 0);
	i_err[5] = Fadd32(ptr_fml_Ibuf, FFO_OPERATION_TYP, (char *)&c_auto_reconnect, 0);
	i_err[6] = Fadd32(ptr_fml_Ibuf, FFO_STATUS_FLG, (char *)&c_server_flg, 0);
	i_err[7] = Fadd32(ptr_fml_Ibuf, FFO_ORD_TOT_QTY, (char *)&l_initial_ord, 0);

	i_retcd = tpacall("SFO_CON_CLNT", (char *)ptr_fml_Ibuf, 0, TPNOTRAN | TPNOREPLY);
	if (i_retcd != 0)
	{
		fn_errlog(c_ServiceName, "S31220", TPMSG, c_errmsg);
		MEMSET(c_command);
		strcpy(c_msg, "Service Failed to call Auto Reconnect, Pls check.");
		sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
		system(c_command);
	}

	tpfree((char *)ptr_fml_Ibuf);
}
/** ver 2.0 Ends Here ***/
