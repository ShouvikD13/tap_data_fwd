/************************ ver 1.1 To display errors coming from exchange .Shamili ***********/

/************************ ver 1.2 NNF-Message Download changes.						Shailesh ***********/
/************************ ver 1.3	Consolidated FNO Console NFO/BFO        Bineet   ***********/
/************************ ver 1.4	Random Password generation Change | Sachin Birje   ***********/
/************************ ver 1.5	Stream No. handelling changes | Tanmay Warkhade ************/
/************************ ver 1.6 Stream No. Exchange Exception Handling Changes  | Parag Kanojia ************/
/************************ ver 1.7 Trimmed Protocol for Non-NEAT Front End | Suchita Dabir ************/
/************************ ver 1.8 Stream Number Changes  | Sachin Birje ************/
/************************ ver 1.9 RQ6892 Network Header Seq No changes  | Sachin Birje ************/

#include <stdlib.h>
#include <stdio.h>

#include <sqlca.h>
#include <atmi.h>
#include <fn_tuxlib.h>

#include <fml32.h>
#include <Usysflds.h>
#include <pthread.h>
#include <memory.h>
#include <fo_exg_esr_lib.h>
#include <fo.h>
#include <fn_ddr.h>      /*** 1.1 ****/
#include <fo_view_def.h> /*** 1.1 ****/
#include <fn_log.h>
#include <fn_pthread.h>
#include <fn_scklib_tap.h>
#include <fn_md5.h>
#include <fn_fo_fprint.h>
#include <fn_read_debug_lvl.h>
#include <fn_rndm_passwd_genrn.h>
#include <netinet/in.h>               /*** Ver 1.7 ***/
#include <fn_host_to_nw_format_nse.h> /*** Ver 1.7 ***/
#include <fn_nw_to_host_format_nse.h> /*** Ver 1.7 ***/

EXEC SQL BEGIN DECLARE SECTION;
extern char sql_c_xchng_cd[3 + 1];
extern varchar sql_c_nxt_trd_date[LEN_DATE];
extern char sql_c_pipe_id[2 + 1];
extern varchar sql_sec_tm[LEN_DT];
extern varchar sql_part_tm[LEN_DT];
extern varchar sql_inst_tm[LEN_DT];
extern varchar sql_idx_tm[LEN_DT];
extern i_ntwk_hdr_seqno; /* Vre 1.9 **/
EXEC SQL END DECLARE SECTION;
Checking if user is already logged on in console or not extern char c_rqst_for_open_ordrs;
extern int i_tap_sck_id;
/***	extern int i_invt_cnt;	*** Commented in Ver 1.7 ***/
extern int i_msg_dump_flg;
extern int i_cond_snd_rcv_cntrl;
extern pthread_cond_t cond_snd_rcv_cntrl;
extern pthread_mutex_t mut_snd_rcv_cntrl;

static short int si_normal_mkt_stts;
static short int si_exr_mkt_stts;
static short int si_pl_mkt_stts;

static short int si_oddmkt_stts;
static short int si_spotmkt_stts;
static short int si_auctionmkt_stts;

static short int si_exr_oddmkt_stts;
static short int si_exr_spotmkt_stts;
static short int si_exr_auctionmkt_stts;

static short int si_pl_oddmkt_stts;
static short int si_pl_spotmkt_stts;
static short int si_pl_auctionmkt_stts;

int fn_chng_exp_passwd_req(char *c_pipe_id, char *c_new_gen_passwd, char *c_ServiceName, char *c_err_msg); /* ver 1.3 */

int fn_do_xchng_logon(struct st_exch_msg *ptr_st_exch_msg, FILE *ptr_snd_msgs_file, char *c_ServiceName, char *c_err_msg)
{
  int i_ch_val;
  int i_snd_seq;
  long li_business_data_size;
  long li_send_tap_msg_size;
  long int li_opm_trdr_id = 0;

  char c_tap_header;

  struct st_exch_msg st_exch_message;
  unsigned char digest[16];

  struct st_system_info_req st_sys_inf_req;
  struct st_update_local_database st_upd_local_db;

  i_cond_snd_rcv_cntrl = LOGON_REQ_SENT;

  li_business_data_size = sizeof(struct st_sign_on_req);
  li_send_tap_msg_size = sizeof(struct st_net_hdr) + li_business_data_size;

  /** Ver 1.9 Starts Here **/
  i_ntwk_hdr_seqno = i_ntwk_hdr_seqno + 1;
  ptr_st_exch_msg->st_net_header.i_seq_num = i_ntwk_hdr_seqno;
  fn_userlog(c_ServiceName, "st_net_header.i_seq_num :%d:", ptr_st_exch_msg->st_net_header.i_seq_num);
  /** Ver 1.9 Ends Here **/

  i_ch_val = fn_writen(i_tap_sck_id,
                       (void *)ptr_st_exch_msg,
                       li_send_tap_msg_size,
                       c_ServiceName,
                       c_err_msg);
  if (i_ch_val == -1)
  {
    fn_errlog(c_ServiceName, "L31005", LIBMSG, c_err_msg);
    return -1;
  }

  /***	i_invt_cnt = i_invt_cnt - 1;	*** Commented in Ver 1.7 ***/

  if (i_msg_dump_flg == LOG_DUMP)
  {
    /**call function to write data to file **/
    c_tap_header = 'Y';

    fn_fprint_log(ptr_snd_msgs_file,
                  c_tap_header,
                  (void *)ptr_st_exch_msg,
                  c_ServiceName,
                  c_err_msg);
  }

  while ((i_cond_snd_rcv_cntrl != LOGON_RESP_RCVD) &&
         (i_cond_snd_rcv_cntrl != RCV_ERR))
  {
    if (DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "Inside while of not equat to LOGON_RESP_RCVD and RCV_ERR");
    }

    i_ch_val = fn_thrd_cond_wait(&cond_snd_rcv_cntrl,
                                 &mut_snd_rcv_cntrl,
                                 c_ServiceName,
                                 c_err_msg);
    if (i_ch_val == -1)
    {
      fn_errlog(c_ServiceName, "L31010", LIBMSG, c_err_msg);
      return -1;
    }
  }

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "The value of i_cond_snd_rcv_cntrl II is :%d:", i_cond_snd_rcv_cntrl);
  }

  if (i_cond_snd_rcv_cntrl == RCV_ERR)
  {
    fn_errlog(c_ServiceName, "L31015", "Error in recieve thread", c_err_msg);
    pthread_mutex_unlock(&mut_snd_rcv_cntrl);
    return -1;
  }

  pthread_mutex_unlock(&mut_snd_rcv_cntrl);

  /*** Populate System information request structure ***/

  memset(&st_sys_inf_req, '\0', sizeof(st_sys_inf_req));
  memset(&st_exch_message, '\0', sizeof(st_exch_message));
  memset(&digest, '\0', sizeof(digest));

  /***** commented in ver 1.7 ******
   fn_orstonse_char ( st_sys_inf_req.st_hdr.c_filler_1,
                      2,
                      " ",
                      1); ******/
  /*** ver 1.7 started ***/
  EXEC SQL
      SELECT OPM_TRDR_ID
          INTO : li_opm_trdr_id
                     FROM OPM_ORD_PIPE_MSTR
                         WHERE OPM_PIPE_ID = : sql_c_pipe_id
                                                   AND OPM_XCHNG_CD = : sql_c_xchng_cd;

  if (SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31020", SQLMSG, c_err_msg);
    return -1;
  }

  st_sys_inf_req.st_hdr.li_trader_id = li_opm_trdr_id;
  /*** ver 1.7 ended *****/

  st_sys_inf_req.st_hdr.li_log_time = 0;
  fn_orstonse_char(st_sys_inf_req.st_hdr.c_alpha_char,
                   LEN_ALPHA_CHAR,
                   " ",
                   1);
  st_sys_inf_req.st_hdr.si_transaction_code = SYSTEM_INFORMATION_IN;
  st_sys_inf_req.st_hdr.si_error_code = 0;

  fn_orstonse_char(st_sys_inf_req.st_hdr.c_filler_2,
                   8,
                   " ",
                   1);

  memset(st_sys_inf_req.st_hdr.c_time_stamp_1, 0, LEN_TIME_STAMP);
  memset(st_sys_inf_req.st_hdr.c_time_stamp_2, 0, LEN_TIME_STAMP);
  st_sys_inf_req.st_hdr.si_message_length = sizeof(st_sys_inf_req);
  st_sys_inf_req.li_last_update_protfolio_time = 0;

  if (DEBUG_MSG_LVL_3)
  {
    /****** fn_userlog(c_ServiceName,"st_sys_inf_req.st_hdr.c_filler_1 : %s ",st_sys_inf_req.st_hdr.c_filler_1);*** commented in ver 1.7 ****/
    fn_userlog(c_ServiceName, "st_sys_inf_req.st_hdr.li_trader_id is :%ld:", st_sys_inf_req.st_hdr.li_trader_id); /*** ver 1.7 ***/
    fn_userlog(c_ServiceName, "st_sys_inf_req.st_hdr.li_log_time : %d ", st_sys_inf_req.st_hdr.li_log_time);
    fn_userlog(c_ServiceName, "st_sys_inf_req.st_hdr.c_alpha_char : %s ", st_sys_inf_req.st_hdr.c_alpha_char);
    fn_userlog(c_ServiceName, "st_sys_inf_req.st_hdr.si_transaction_code : %d ", st_sys_inf_req.st_hdr.si_transaction_code);
    fn_userlog(c_ServiceName, "st_sys_inf_req.st_hdr.si_error_code : %d ", st_sys_inf_req.st_hdr.si_error_code);
    /*** fn_userlog(c_ServiceName,"st_sys_inf_req.st_hdr.c_filler_2 : %s ",st_sys_inf_req.st_hdr.c_filler_2 );  commented in ver 1.7 ***/
    fn_userlog(c_ServiceName, "st_sys_inf_req.st_hdr.si_message_length : %d ", st_sys_inf_req.st_hdr.si_message_length);
    fn_userlog(c_ServiceName, "st_sys_inf_req.li_last_update_protfolio_time : %d ", st_sys_inf_req.li_last_update_protfolio_time);
  }

  pthread_mutex_lock(&mut_snd_rcv_cntrl);

  i_cond_snd_rcv_cntrl = SID_REQ_SENT;

  fn_cnvt_htn_sys_info_req(&st_sys_inf_req); /*** Ver 1.7 ***/

  memcpy(&(st_exch_message.st_exch_sndmsg), &st_sys_inf_req, sizeof(st_sys_inf_req));

  /****call routine for get place sequence ****/

  i_ch_val = fn_get_reset_seq(sql_c_pipe_id,
                              sql_c_nxt_trd_date.arr,
                              GET_PLACED_SEQ,
                              &i_snd_seq,
                              c_ServiceName,
                              c_err_msg);
  if (i_ch_val == -1)
  {
    fn_errlog(c_ServiceName, "L31025", LIBMSG, c_err_msg);
    return -1;
  }

  st_exch_message.st_net_header.i_seq_num = i_snd_seq;

  if (DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName, "The sequence number got is:%d:", i_snd_seq);
  }

  fn_cal_md5digest((void *)&st_sys_inf_req, sizeof(st_sys_inf_req), digest);

  memcpy(st_exch_message.st_net_header.c_checksum, digest, sizeof(digest));

  st_exch_message.st_net_header.si_message_length = sizeof(st_exch_message.st_net_header) + sizeof(st_sys_inf_req);

  /* Ver 1.9 Starts here **/
  i_ntwk_hdr_seqno = i_ntwk_hdr_seqno + 1;
  st_exch_message.st_net_header.i_seq_num = i_ntwk_hdr_seqno;
  fn_userlog(c_ServiceName, "st_net_header.i_seq_num :%d:", st_exch_message.st_net_header.i_seq_num);
  /** Ver 1.9 Ends Here ***/

  li_business_data_size = sizeof(st_sys_inf_req);
  li_send_tap_msg_size = sizeof(st_exch_message.st_net_header) + li_business_data_size;

  /***  if( i_invt_cnt >= 1)	*** Commented in Ver 1.7 ***
    {	*** Ver 1.7 ***/
  i_ch_val = fn_writen(i_tap_sck_id,
                       (void *)&st_exch_message,
                       li_send_tap_msg_size,
                       c_ServiceName,
                       c_err_msg);
  if (i_ch_val == -1)
  {
    fn_errlog(c_ServiceName, "L31030", LIBMSG, c_err_msg);
    return -1;
  }

  /***		i_invt_cnt = i_invt_cnt - 1;		*** Commented in Ver 1.7 ***
    }
    else
    {
      fn_userlog(c_ServiceName,"The SID Request cannot be sent,please wait for the invitation count to update");
      return -1;
    }	*** Ver 1.7 ***/
  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "After calling fn_set_xchg_logon_stts for SID_RQST_SENT");
  }

  if (i_msg_dump_flg == LOG_DUMP)
  {
    /**call function to write data to file **/
    c_tap_header = 'Y';

    fn_fprint_log(ptr_snd_msgs_file,
                  c_tap_header,
                  (void *)&st_exch_message,
                  c_ServiceName,
                  c_err_msg);
  }

  while ((i_cond_snd_rcv_cntrl != SID_RESP_RCVD) &&
         (i_cond_snd_rcv_cntrl != RCV_ERR))
  {
    i_ch_val = fn_thrd_cond_wait(&cond_snd_rcv_cntrl,
                                 &mut_snd_rcv_cntrl,
                                 c_ServiceName,
                                 c_err_msg);
    if (i_ch_val == -1)
    {
      fn_errlog(c_ServiceName, "L31035", LIBMSG, c_err_msg);
      return -1;
    }
  }

  if (i_cond_snd_rcv_cntrl == RCV_ERR)
  {
    fn_errlog(c_ServiceName, "L31040", "Error in recieve thread", c_err_msg);
    return -1;
  }

  pthread_mutex_unlock(&mut_snd_rcv_cntrl);

  /*** Populate Update Local database request structure ***/
  memset(&st_upd_local_db, '\0', sizeof(st_upd_local_db));
  memset(&st_exch_message, '\0', sizeof(st_exch_message));

  /******* commented in ver 1.7 *****
    fn_orstonse_char ( st_upd_local_db.st_hdr.c_filler_1,
                       2,
                       " ",
                       1); ******/

  st_upd_local_db.st_hdr.li_trader_id = li_opm_trdr_id; /*** ver 1.7 ***/
  st_upd_local_db.st_hdr.li_log_time = 0;

  fn_orstonse_char(st_upd_local_db.st_hdr.c_alpha_char,
                   LEN_ALPHA_CHAR,
                   " ",
                   1);

  st_upd_local_db.st_hdr.si_transaction_code = UPDATE_LOCALDB_IN;

  st_upd_local_db.st_hdr.si_error_code = 0;

  fn_orstonse_char(st_upd_local_db.st_hdr.c_filler_2,
                   8,
                   " ",
                   1);

  memset(st_upd_local_db.st_hdr.c_time_stamp_1, 0, LEN_TIME_STAMP);

  memset(st_upd_local_db.st_hdr.c_time_stamp_2, 0, LEN_TIME_STAMP);

  st_upd_local_db.st_hdr.si_message_length = sizeof(st_upd_local_db);

  fn_timearr_to_long(sql_sec_tm.arr, &st_upd_local_db.li_last_update_security_time);

  fn_timearr_to_long(sql_part_tm.arr, &st_upd_local_db.li_last_update_participant_time);

  fn_timearr_to_long(sql_inst_tm.arr, &st_upd_local_db.li_last_update_instrument_time);

  fn_timearr_to_long(sql_idx_tm.arr, &st_upd_local_db.li_last_update_index_time);

  st_upd_local_db.c_request_for_open_orders = c_rqst_for_open_ordrs;

  st_upd_local_db.c_filler_1 = ' ';

  st_upd_local_db.st_mkt_stts.si_normal = si_normal_mkt_stts;

  st_upd_local_db.st_mkt_stts.si_oddlot = si_oddmkt_stts;

  st_upd_local_db.st_mkt_stts.si_spot = si_spotmkt_stts;

  st_upd_local_db.st_mkt_stts.si_auction = si_auctionmkt_stts;

  st_upd_local_db.st_ex_mkt_stts.si_normal = si_exr_mkt_stts;

  st_upd_local_db.st_ex_mkt_stts.si_oddlot = si_exr_oddmkt_stts;

  st_upd_local_db.st_ex_mkt_stts.si_spot = si_exr_spotmkt_stts;

  st_upd_local_db.st_ex_mkt_stts.si_auction = si_exr_auctionmkt_stts;

  st_upd_local_db.st_pl_mkt_stts.si_normal = si_pl_mkt_stts;

  st_upd_local_db.st_pl_mkt_stts.si_oddlot = si_pl_oddmkt_stts;

  st_upd_local_db.st_pl_mkt_stts.si_spot = si_pl_spotmkt_stts;

  st_upd_local_db.st_pl_mkt_stts.si_auction = si_pl_auctionmkt_stts;

  if (DEBUG_MSG_LVL_3)
  {
    /*** fn_userlog(c_ServiceName,"st_upd_local_db.st_hdr.c_filler_1 is : %s", st_upd_local_db.st_hdr.c_filler_1); commented in ver 1.7 *******/
    fn_userlog(c_ServiceName, "st_upd_local_db.st_hdr.li_trader_id is :%ld:", st_upd_local_db.st_hdr.li_trader_id); /*** ver 1.7 ***/
    fn_userlog(c_ServiceName, "st_upd_local_db.st_hdr.li_log_time : %ld", st_upd_local_db.st_hdr.li_log_time);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_hdr.c_alpha_char : %s", st_upd_local_db.st_hdr.c_alpha_char);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_hdr.si_transaction_code : %ld", st_upd_local_db.st_hdr.si_transaction_code);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_hdr.si_error_code : %ld", st_upd_local_db.st_hdr.si_error_code);
    /*** commented in ver 1.7 fn_userlog(c_ServiceName,"st_upd_local_db.st_hdr.c_filler_2 : %s", st_upd_local_db.st_hdr.c_filler_2); *****/
    fn_userlog(c_ServiceName, "st_upd_local_db.st_hdr.c_time_stamp_1 : %s", st_upd_local_db.st_hdr.c_time_stamp_1);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_hdr.c_time_stamp_2 : %s", st_upd_local_db.st_hdr.c_time_stamp_2);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_hdr.si_message_length : %ld", st_upd_local_db.st_hdr.si_message_length);
    fn_userlog(c_ServiceName, "st_upd_local_db.li_last_update_security_time :%ld", st_upd_local_db.li_last_update_security_time);
    fn_userlog(c_ServiceName, "st_upd_local_db.li_last_update_participant_time :%ld", st_upd_local_db.li_last_update_participant_time);
    fn_userlog(c_ServiceName, "st_upd_local_db.li_last_update_instrument_time :%ld", st_upd_local_db.li_last_update_instrument_time);
    fn_userlog(c_ServiceName, "st_upd_local_db.li_last_update_index_time :%ld", st_upd_local_db.li_last_update_index_time);
    fn_userlog(c_ServiceName, "st_upd_local_db.c_request_for_open_orders :%c", st_upd_local_db.c_request_for_open_orders);
    fn_userlog(c_ServiceName, "st_upd_local_db.c_filler_1 :%c", st_upd_local_db.c_filler_1);
    fn_userlog(c_ServiceName, "NORMAL :%d:", si_normal_mkt_stts);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_mkt_stts.si_normal :%d", st_upd_local_db.st_mkt_stts.si_normal);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_mkt_stts.si_oddlot :%d", st_upd_local_db.st_mkt_stts.si_oddlot);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_mkt_stts.si_spot :%d", st_upd_local_db.st_mkt_stts.si_spot);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_mkt_stts.si_auction :%d", st_upd_local_db.st_mkt_stts.si_auction);
    fn_userlog(c_ServiceName, "EXER :%d:", si_exr_mkt_stts);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_ex_mkt_stts.si_normal :%d", st_upd_local_db.st_ex_mkt_stts.si_normal);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_ex_mkt_stts.si_oddlot :%d", st_upd_local_db.st_ex_mkt_stts.si_oddlot);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_ex_mkt_stts.si_spot :%d", st_upd_local_db.st_ex_mkt_stts.si_spot);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_ex_mkt_stts.si_auction :%d", st_upd_local_db.st_ex_mkt_stts.si_auction);
    fn_userlog(c_ServiceName, "PL :%d:", si_pl_mkt_stts);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_pl_mkt_stts.si_normal :%d", st_upd_local_db.st_pl_mkt_stts.si_normal);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_pl_mkt_stts.si_oddlot :%d", st_upd_local_db.st_pl_mkt_stts.si_oddlot);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_pl_mkt_stts.si_spot :%d", st_upd_local_db.st_pl_mkt_stts.si_spot);
    fn_userlog(c_ServiceName, "st_upd_local_db.st_pl_mkt_stts.si_auction :%d", st_upd_local_db.st_pl_mkt_stts.si_auction);
  }

  pthread_mutex_lock(&mut_snd_rcv_cntrl);

  i_cond_snd_rcv_cntrl = LDB_REQ_SENT;

  fn_cnvt_to_htn_upd_local_db(&st_upd_local_db); /*** Ver 1.7 ***/

  memcpy(&(st_exch_message.st_exch_sndmsg), &st_upd_local_db, sizeof(st_upd_local_db));

  i_ch_val = fn_get_reset_seq(sql_c_pipe_id,
                              sql_c_nxt_trd_date.arr,
                              GET_PLACED_SEQ,
                              &i_snd_seq,
                              c_ServiceName,
                              c_err_msg);
  if (i_ch_val == -1)
  {
    fn_errlog(c_ServiceName, "L31045", LIBMSG, c_err_msg);
    return -1;
  }

  st_exch_message.st_net_header.i_seq_num = i_snd_seq;

  if (DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName, "The sequence number got is:%d:", i_snd_seq);
  }

  fn_cal_md5digest((void *)&st_upd_local_db, sizeof(st_upd_local_db), digest);

  memcpy(st_exch_message.st_net_header.c_checksum, digest, sizeof(digest));

  st_exch_message.st_net_header.si_message_length = sizeof(st_exch_message.st_net_header) + sizeof(st_upd_local_db);

  fn_cnvt_htn_net_hdr(&st_exch_message.st_net_header); /*** Ver 1.7 ***/

  li_business_data_size = sizeof(st_upd_local_db);
  li_send_tap_msg_size = sizeof(st_exch_message.st_net_header) + li_business_data_size;

  /* Ver 1.9 Starts here **/
  i_ntwk_hdr_seqno = i_ntwk_hdr_seqno + 1;
  st_exch_message.st_net_header.i_seq_num = i_ntwk_hdr_seqno;
  fn_userlog(c_ServiceName, "st_net_header.i_seq_num :%d:", st_exch_message.st_net_header.i_seq_num);
  /** Ver 1.9 Ends Here ***/

  /***  if( i_invt_cnt >= 1)	*** Commented in Ver 1.7 ***
    {	*** Ver 1.7 ***/
  i_ch_val = fn_writen(i_tap_sck_id,
                       (void *)&st_exch_message,
                       li_send_tap_msg_size,
                       c_ServiceName,
                       c_err_msg);
  if (i_ch_val == -1)
  {
    fn_errlog(c_ServiceName, "L31050", LIBMSG, c_err_msg);
    return -1;
  }

  /***		i_invt_cnt = i_invt_cnt - 1;	*** Commented in Ver 1.7 ***
    }
    else
    {
      fn_userlog(c_ServiceName,"The LDB Request cannot be sent,please wait for the invitation count to update");
      return -1;
    }	*** Ver 1.7 ***/

  if (i_msg_dump_flg == LOG_DUMP)
  {
    /**call function to write data to file **/
    c_tap_header = 'Y';

    fn_fprint_log(ptr_snd_msgs_file,
                  c_tap_header,
                  (void *)&st_exch_message,
                  c_ServiceName,
                  c_err_msg);
  }

  while ((i_cond_snd_rcv_cntrl != LDB_RESP_RCVD) &&
         (i_cond_snd_rcv_cntrl != RCV_ERR))
  {
    if (DEBUG_MSG_LVL_3)
    {
      fn_userlog(c_ServiceName, "Inside while Before fn_thrd_cond_wait ");
    }
    i_ch_val = fn_thrd_cond_wait(&cond_snd_rcv_cntrl,
                                 &mut_snd_rcv_cntrl,
                                 c_ServiceName,
                                 c_err_msg);
    if (i_ch_val == -1)
    {
      fn_errlog(c_ServiceName, "L31055", LIBMSG, c_err_msg);
      pthread_mutex_unlock(&mut_snd_rcv_cntrl);
      return -1;
    }

    if (DEBUG_MSG_LVL_3)
    {
      fn_userlog(c_ServiceName, "Inside while After fn_thrd_cond_wait ");
    }
  }

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "The i_cond_snd_rcv_cntrl II is :%d:", i_cond_snd_rcv_cntrl);
  }

  if (i_cond_snd_rcv_cntrl == RCV_ERR)
  {
    fn_errlog(c_ServiceName, "L31060", "Error in recieve thread", c_err_msg);
    return -1;
  }

  return 0;
}

int fn_system_information_out(struct st_system_info_data *st_sys_info_dat, char *c_ServiceName, char *c_err_msg)
{
  long int li_mkt_type;
  long int li_stream_no;
  int i_ch_val;
  int i_trnsctn;

  char c_stream_no[2];

  si_normal_mkt_stts = st_sys_info_dat->st_mkt_stts.si_normal;
  si_oddmkt_stts = st_sys_info_dat->st_mkt_stts.si_oddlot;
  si_spotmkt_stts = st_sys_info_dat->st_mkt_stts.si_spot;
  si_auctionmkt_stts = st_sys_info_dat->st_mkt_stts.si_auction;

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "The Normal  si_mkt_stts - Normal is :%ld:", si_normal_mkt_stts);
    fn_userlog(c_ServiceName, "The Normal  si_mkt_stts - oddlot is :%ld, %ld:", st_sys_info_dat->st_mkt_stts.si_oddlot, si_oddmkt_stts);
    fn_userlog(c_ServiceName, "The Normal  si_mkt_stts - spot is :%ld, %ld:", st_sys_info_dat->st_mkt_stts.si_spot, si_spotmkt_stts);
    fn_userlog(c_ServiceName, "The Normal  si_mkt_stts - auction is :%ld, %ld:", st_sys_info_dat->st_mkt_stts.si_auction, si_auctionmkt_stts);
  }

  si_exr_mkt_stts = st_sys_info_dat->st_ex_mkt_stts.si_normal;
  si_exr_oddmkt_stts = st_sys_info_dat->st_ex_mkt_stts.si_oddlot;
  si_exr_spotmkt_stts = st_sys_info_dat->st_ex_mkt_stts.si_spot;
  si_exr_auctionmkt_stts = st_sys_info_dat->st_ex_mkt_stts.si_auction;

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "The Exer  si_mkt_stts is :%ld:", si_exr_mkt_stts);
    fn_userlog(c_ServiceName, "The Exer  si_mkt_stts is :%ld, %ld:", st_sys_info_dat->st_ex_mkt_stts.si_oddlot, si_exr_oddmkt_stts);
    fn_userlog(c_ServiceName, "The Exer  si_mkt_stts is :%ld, %ld:", st_sys_info_dat->st_ex_mkt_stts.si_spot, si_exr_spotmkt_stts);
    fn_userlog(c_ServiceName, "The Exer  si_mkt_stts is :%ld, %ld:", st_sys_info_dat->st_ex_mkt_stts.si_auction, si_exr_auctionmkt_stts);
  }

  li_mkt_type = EXERCISE_MKT;

  i_ch_val = fn_mrkt_stts_hndlr(si_exr_mkt_stts,
                                li_mkt_type,
                                sql_c_xchng_cd,
                                c_ServiceName,
                                c_err_msg);

  if (i_ch_val == -1)
  {
    fn_userlog(c_ServiceName, "Failed While Calling Function fn_mrkt_stts_hndlr");
    return -1;
  }

  si_pl_mkt_stts = st_sys_info_dat->st_pl_mkt_stts.si_normal;
  si_pl_oddmkt_stts = st_sys_info_dat->st_pl_mkt_stts.si_oddlot;
  si_pl_spotmkt_stts = st_sys_info_dat->st_pl_mkt_stts.si_spot;
  si_pl_auctionmkt_stts = st_sys_info_dat->st_pl_mkt_stts.si_auction;

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "The PL  si_mkt_stts is :%ld:", si_pl_mkt_stts);
    fn_userlog(c_ServiceName, "The PL  si_mkt_stts is :%ld, %ld:", st_sys_info_dat->st_pl_mkt_stts.si_oddlot, si_pl_oddmkt_stts);
    fn_userlog(c_ServiceName, "The PL  si_mkt_stts is :%ld, %ld:", st_sys_info_dat->st_pl_mkt_stts.si_spot, si_pl_spotmkt_stts);
    fn_userlog(c_ServiceName, "The PL  si_mkt_stts is :%ld, %ld:", st_sys_info_dat->st_pl_mkt_stts.si_auction, si_pl_auctionmkt_stts);
  }

  li_mkt_type = PL_MKT;

  /**Ver 1.2 starts**/

  li_stream_no = st_sys_info_dat->st_hdr.c_alpha_char[0]; /** Ver 1.5 ***/

  fn_userlog(c_ServiceName, " st_sys_info_dat->st_hdr.c_alpha_char[0] Is :%c:", st_sys_info_dat->st_hdr.c_alpha_char[0]);
  fn_userlog(c_ServiceName, " st_sys_info_dat->st_hdr.c_alpha_char[0] Is :%d:", st_sys_info_dat->st_hdr.c_alpha_char[0]);
  fn_userlog(c_ServiceName, " before li_stream_no Is :%ld:", li_stream_no);
  /**if(li_stream_no == 32 ) *** Ver 1.8 ***
  {
    li_stream_no = 0;
  }
  fn_userlog(c_ServiceName," after li_stream_no Is :%ld:",li_stream_no);
  ***/

  fn_userlog(c_ServiceName, " st_hdr.c_alpha_char is :%s:", st_sys_info_dat->st_hdr.c_alpha_char);

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "The number of streams received is:%ld:", li_stream_no);
  }

  i_trnsctn = fn_begintran(c_ServiceName, c_err_msg);
  if (i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName, "L31065", LIBMSG, c_err_msg);
    return -1;
  }

  EXEC SQL
      UPDATE exg_xchng_mstr
          SET exg_def_stlmnt_pd = : st_sys_info_dat->si_default_sttlmnt_period_nm,
              exg_wrn_pcnt = : st_sys_info_dat->si_warning_percent,
              exg_vol_frz_pcnt = : st_sys_info_dat->si_volume_freeze_percent,
              exg_brd_lot_qty = : st_sys_info_dat->li_board_lot_quantity,
              exg_tck_sz = : st_sys_info_dat->li_tick_size,
              exg_gtc_dys = : st_sys_info_dat->si_maximum_gtc_days,
              exg_dsclsd_qty_pcnt = : st_sys_info_dat->si_disclosed_quantity_percent_allowed
                                          WHERE exg_xchng_cd = : sql_c_xchng_cd;

  if (SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31070", SQLMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }

  EXEC SQL
      UPDATE OPM_ORD_PIPE_MSTR
          SET OPM_STREAM_NO = : li_stream_no
                                    WHERE OPM_PIPE_ID = : sql_c_pipe_id
                                                              AND OPM_XCHNG_CD = : sql_c_xchng_cd;

  if (SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31075", SQLMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }

  /**Ver 1.2 ends**/

  if (fn_committran(c_ServiceName, i_trnsctn, c_err_msg) == -1)
  {
    fn_errlog(c_ServiceName, "L31080", LIBMSG, c_err_msg);
    return -1;
  }
  return 0;
}

/******************************************************************************/
/*  Following function handels Log In Response Received From Exchange         */
/*  INPUT PARAMETERS                                                          */
/*      ptr_st_logon_res - Pointer to a message structure got from Exchange   */
/*      c_ServiceName - Service Name currently running                        */
/*      c_err_msg     - Error message to be returned                          */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/

int fn_sign_on_request_out(struct st_sign_on_res *ptr_st_logon_res, char *c_ServiceName, char *c_err_msg)
{

  struct vw_mkt_msg st_msg; /** 1.1 **/

  char c_msg[256];
  char c_message[256]; /*** Ver 1.3 ***/
  char c_brkr_stts;
  char c_tm_stmp[30];
  char c_clr_mem_stts;
  char c_new_gen_passwd[LEN_PASS_WD]; /*** Ver 1.3 ***/

  int i_ch_val;
  int i_trnsctn;

  struct st_error_response *ptr_st_err_res;

  EXEC SQL BEGIN DECLARE SECTION;
  char sql_passwd_lst_updt_dt[LEN_DT];
  char sql_lst_mkt_cls_dt[LEN_DT];
  char sql_exch_tm[LEN_DT];
  EXEC SQL END DECLARE SECTION;

  MEMSET(st_msg);           /*** 1.1 ***/
  MEMSET(c_new_gen_passwd); /*** Ver 1.3 ***/
  // Step1 : take log indicating control is inside the function
  if (DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName, "Inside Log On Response Handling Function");
  }

  // call this function

  fn_long_to_timearr(c_tm_stmp, ptr_st_logon_res->st_hdr.li_log_time);

  if (DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName, "The ERROR CODE IS :%d:", ptr_st_logon_res->st_hdr.si_error_code);
  }
  // step 2 : check the recieved error code from TAP . if we are getting some error than control will goes in to the if condition
  if (ptr_st_logon_res->st_hdr.si_error_code != 0)
  {
    // step 2.1: first we are taking the log which is indicating the we are getting the in response
    if (DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "Inside Error Condition Of Error Code Check While Log In ");
    }

    // step 2.2: assigning the logon_response in error_response structure
    ptr_st_err_res = (struct st_error_response *)ptr_st_logon_res;

    fn_nsetoors_char(c_err_msg,
                     256,
                     ptr_st_err_res->c_error_message,
                     LEN_ERROR_MESSAGE);

    /*sprintf(c_msg,"|%s|%d- %s|",c_tm_stmp,ptr_st_logon_res->st_hdr.si_error_code,c_err_msg); */ /** 1.1 **/

    if (DEBUG_MSG_LVL_3)
    {
      fn_userlog(c_ServiceName, "Msg Is :%s:", c_msg);
    }

    /**** Ver 1.1 Starts ******/

    EXEC SQL
        SELECT to_char(sysdate, 'dd-Mon-yyyy hh24:mi:ss') INTO : st_msg.c_tm_stmp FROM dual;

    sprintf(c_msg, "|%s|%d- %s|", st_msg.c_tm_stmp, ptr_st_logon_res->st_hdr.si_error_code, c_err_msg);

    strcpy(st_msg.c_xchng_cd, sql_c_xchng_cd);
    strcpy(st_msg.c_msg, c_msg);
    st_msg.c_msg_id = TRADER_MSG;

    MEMSET(st_msg.c_brkr_id);

    EXEC SQL
        SELECT exg_brkr_id
            INTO : st_msg.c_brkr_id
                       FROM exg_xchng_mstr
                           WHERE exg_xchng_cd = : st_msg.c_xchng_cd;

    if (SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "S31190", SQLMSG, c_err_msg);
      return (-1);
    }
    /* sprintf(st_msg.c_brkr_id,"%s",ptr_st_logon_res->c_broker_id);*/

    if (DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "Exchange Code Is :%s:", st_msg.c_xchng_cd);
      fn_userlog(c_ServiceName, "Broker ID Is :%s:", st_msg.c_brkr_id);
      fn_userlog(c_ServiceName, "Message Is :%s:", st_msg.c_msg);
      fn_userlog(c_ServiceName, "Time Stamp  Is :%s:", st_msg.c_tm_stmp);
      fn_userlog(c_ServiceName, "Message ID Is :%c:", st_msg.c_msg_id);
      fn_userlog(c_ServiceName, "Before Call to SFO_UPD_MSG");
    }

    fn_cpy_ddr(st_msg.c_rout_crt);

    i_ch_val = fn_acall_svc(c_ServiceName,
                            c_err_msg,
                            &st_msg,
                            "vw_mkt_msg",
                            sizeof(struct vw_mkt_msg),
                            TPNOREPLY,
                            "SFO_UPD_MSG");

    if (i_ch_val == SYSTEM_ERROR)
    {
      fn_errlog(c_ServiceName, "S31005", LIBMSG, c_err_msg);
      return -1;
    }

    /**** Ver 1.1 Ends ******/

    /* Ver 1.3 Starts */
    if (DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "ptr_st_logon_res->st_hdr.si_error_code:: %d", ptr_st_logon_res->st_hdr.si_error_code);
    }

    if (ptr_st_logon_res->st_hdr.si_error_code == 16053)
    {
      /*****  Auto password change functionality  *****/

      i_ch_val = fn_chng_exp_passwd_req(sql_c_pipe_id, c_new_gen_passwd, c_ServiceName, c_err_msg);

      if (i_ch_val == -1)
      {
        fn_errlog(c_ServiceName, "L31085", LIBMSG, c_err_msg);
        fn_userlog(c_ServiceName, "###########################################");
        fn_userlog(c_ServiceName, " Error in request of change password ");
        fn_userlog(c_ServiceName, "###########################################");
        return -1;
      }

      MEMSET(c_msg);

      sprintf(c_msg, "|%s- %s %s|", st_msg.c_tm_stmp, "Your password has been regenerated, Please relogin. New pwd is", c_new_gen_passwd);
      strcpy(st_msg.c_msg, c_msg);

      i_ch_val = fn_acall_svc(c_ServiceName,
                              c_err_msg,
                              &st_msg,
                              "vw_mkt_msg",
                              sizeof(struct vw_mkt_msg),
                              TPNOREPLY,
                              "SFO_UPD_MSG");

      if (i_ch_val == SYSTEM_ERROR)
      {
        fn_errlog(c_ServiceName, "S31005", LIBMSG, c_err_msg);
        return -1;
      }
    }
    /* Ver 1.3 Ends */

    fn_pst_trg(c_ServiceName, "TRG_ORS_CON", c_msg, sql_c_pipe_id);
    return -1;
  }
  else
  {
    if (DEBUG_MSG_LVL_3)
    {
      fn_userlog(c_ServiceName, "Sign On Successful : Checking Other Details ");
      fn_userlog(c_ServiceName, "Broker Status Is :%c:", ptr_st_logon_res->c_broker_status);
      fn_userlog(c_ServiceName, "Clearing Status Is :%c:", ptr_st_logon_res->c_clearing_status);
    }

    /*** Ver 1.3  start ***/
    MEMSET(c_message);
    strcpy(c_message, "Logon Successful");
    /*** Ver 1.3  ends ***/

    if (ptr_st_logon_res->c_broker_status == CLOSE_OUT)
    {
      sprintf(c_msg, "|%s| - Logon successful, Broker ClosedOut|", c_tm_stmp);
      strcpy(c_message, "Logon Successful, Broker ClosedOut"); /*** Ver 1.3  ***/

      if (DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName, "Msg Is :%s;", c_msg);
      }
    }

    if (ptr_st_logon_res->c_broker_status != ACTIVE && ptr_st_logon_res->c_broker_status != CLOSE_OUT)
    {
      sprintf(c_msg, "|%s| - Logon successful, Broker suspended|", c_tm_stmp);
      strcpy(c_message, "Logon Successful, Broker Suspended"); /*** Ver 1.3  ***/

      if (DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName, "Msg Is :%s;", c_msg);
      }
    }

    if (ptr_st_logon_res->c_clearing_status != ACTIVE)
    {
      strcpy(c_msg, "Logon successful,Clearing member suspended");
      sprintf(c_msg, "|%s| - Logon successful, Clearing member suspended|", c_tm_stmp);
      strcpy(c_message, "Logon Successful, Clearing member suspended"); /*** Ver 1.3  ***/

      if (DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName, "Msg Is :%s;", c_msg);
      }
    }

    /*** Ver 1.3  starts ***/

    if (DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "Inside Success Condition While Log In ");
      fn_userlog(c_ServiceName, "c_message Is :%s:", c_message);
    }

    if (DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "ptr_st_logon_res->st_hdr.si_error_code:: %d", ptr_st_logon_res->st_hdr.si_error_code);
    }

    EXEC SQL
        SELECT to_char(sysdate, 'dd-Mon-yyyy hh24:mi:ss') INTO : st_msg.c_tm_stmp FROM dual;

    sprintf(c_msg, "|%s|%d- %s|", st_msg.c_tm_stmp, ptr_st_logon_res->st_hdr.si_error_code, c_message);

    strcpy(st_msg.c_xchng_cd, sql_c_xchng_cd);
    strcpy(st_msg.c_msg, c_msg);
    st_msg.c_msg_id = TRADER_MSG; // 'T'

    MEMSET(st_msg.c_brkr_id);

    EXEC SQL
        SELECT exg_brkr_id
            INTO : st_msg.c_brkr_id
                       FROM exg_xchng_mstr
                           WHERE exg_xchng_cd = : st_msg.c_xchng_cd;

    if (SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "S31190", SQLMSG, c_err_msg);
      return (-1);
    }
    if (DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "Exchange Code Is :%s:", st_msg.c_xchng_cd);
      fn_userlog(c_ServiceName, "Broker ID Is :%s:", st_msg.c_brkr_id);
      fn_userlog(c_ServiceName, "Message Is :%s:", st_msg.c_msg);
      fn_userlog(c_ServiceName, "Time Stamp  Is :%s:", st_msg.c_tm_stmp);
      fn_userlog(c_ServiceName, "Message ID Is :%c:", st_msg.c_msg_id);
      fn_userlog(c_ServiceName, "Before Call to SFO_UPD_MSG");
    }

    fn_cpy_ddr(st_msg.c_rout_crt);

    i_ch_val = fn_acall_svc(c_ServiceName,
                            c_err_msg,
                            &st_msg,
                            "vw_mkt_msg",
                            sizeof(struct vw_mkt_msg),
                            TPNOREPLY,
                            "SFO_UPD_MSG");

    if (i_ch_val == SYSTEM_ERROR)
    {
      fn_errlog(c_ServiceName, "S31005", LIBMSG, c_err_msg);
      return -1;
    }
    /*** Ver 1.3  ends ***/

    fn_pst_trg(c_ServiceName, "TRG_ORS_CON", c_msg, sql_c_pipe_id);

    fn_long_to_timearr(sql_passwd_lst_updt_dt, ptr_st_logon_res->li_last_password_change_date);
    fn_long_to_timearr(sql_lst_mkt_cls_dt, ptr_st_logon_res->st_hdr.li_log_time);
    fn_long_to_timearr(sql_exch_tm, ptr_st_logon_res->li_end_time);

    if (DEBUG_MSG_LVL_3)
    {
      fn_userlog(c_ServiceName, "Password Last Change Date Is ;%s;", sql_passwd_lst_updt_dt);
      fn_userlog(c_ServiceName, "Market Closing Date Is :%s:", sql_lst_mkt_cls_dt);
      fn_userlog(c_ServiceName, "Exchange Time Is :%s:", sql_exch_tm);
    }
  }

  i_trnsctn = fn_begintran(c_ServiceName, c_err_msg);
  if (i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName, "L31090", LIBMSG, c_err_msg);
    return -1;
  }

  /* Ver 1.3 Starts */
  EXEC SQL UPDATE OPM_ORD_PIPE_MSTR
      SET OPM_EXG_PWD = OPM_NEW_EXG_PWD,
          OPM_NEW_EXG_PWD = null
              WHERE OPM_PIPE_ID = : sql_c_pipe_id
                                        AND OPM_XCHNG_CD = : sql_c_xchng_cd
                                                                 AND OPM_NEW_EXG_PWD is not null;

  if (SQLCODE != 0 && SQLCODE != 1403)
  {
    fn_errlog(c_ServiceName, "L31095", SQLMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return (-1);
  }
  /* Ver 1.3 Ends */

  if (ptr_st_logon_res->c_broker_status == ACTIVE)
  {
    EXEC SQL
        UPDATE opm_ord_pipe_mstr
            SET opm_lst_pswd_chg_dt = to_date( : sql_passwd_lst_updt_dt, 'dd-Mon-yyyy hh24:mi:ss'),
                opm_login_stts = 1 WHERE opm_pipe_id = : sql_c_pipe_id;

    if (SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "L31100", SQLMSG, c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return -1;
    }
  }

  if ((ptr_st_logon_res->c_broker_status == ACTIVE) || (ptr_st_logon_res->c_broker_status == CLOSE_OUT) || (ptr_st_logon_res->c_broker_status == SUSPEND))
  {
    EXEC SQL
        UPDATE exg_xchng_mstr
            set exg_brkr_stts = : ptr_st_logon_res->c_broker_status
                                      where EXG_XCHNG_CD = : sql_c_xchng_cd;

    if (SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "L31105", SQLMSG, c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return -1;
    }
  }

  i_ch_val = fn_committran(c_ServiceName, i_trnsctn, c_err_msg);
  if (i_ch_val == -1)
  {
    fn_errlog(c_ServiceName, "L31110", LIBMSG, c_err_msg);
    return -1;
  }

  if (DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName, "Sign On Successful : Checking Other Details Finished");
  }
  return 0;
}

int fn_logout_res(char *c_xchng_cd, char *c_pipe_id, char *c_ServiceName, char *c_err_msg)
{

  int i_trnsctn;

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "Exchange Code Is ;%s:", c_xchng_cd);
    fn_userlog(c_ServiceName, "Pipe ID Is :%s:", c_pipe_id);
  }

  i_trnsctn = fn_begintran(c_ServiceName, c_err_msg);
  if (i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName, "L31115", LIBMSG, c_err_msg);
    return -1;
  }

  EXEC SQL
      UPDATE opm_ord_pipe_mstr
          SET opm_login_stts = 0 WHERE opm_pipe_id = : c_pipe_id
                                                           AND opm_xchng_cd = : c_xchng_cd;

  if (SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "L31120", SQLMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }

  i_trnsctn = fn_committran(c_ServiceName, i_trnsctn, c_err_msg);
  if (i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName, "L31125", LIBMSG, c_err_msg);
    return -1;
  }

  if (DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName, "Log Out response Finished");
  }
  return 0;
}

int fn_partial_logon_res(struct st_system_info_data *ptr_st_sysinf_dat, char *c_pipe_id, char *c_ServiceName, char *c_err_msg)
{
  char c_tm_stmp[30];
  char c_msg[256];
  char c_trd_msg[256];

  if (DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName, "Inside Function fn_partial_logon_res");
  }

  strcpy(c_msg, "SID information download failed");

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "MSG Is :%s:", c_msg);
  }

  fn_long_to_timearr(c_tm_stmp, ptr_st_sysinf_dat->st_hdr.li_log_time);

  sprintf(c_trd_msg, "|%s| %s|", c_tm_stmp, c_msg);

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "Trade Msg Is :%s:", c_trd_msg);
  }

  fn_pst_trg(c_ServiceName, "TRG_ORS_CON", c_trd_msg, c_pipe_id);

  return 0;
}

int fn_localdb_res(struct st_local_database_data *ptr_st_ldb_data, char *c_pipe_id, char *c_ServiceName, char *c_err_msg)
{
  char c_tm_stmp[30];
  char c_msg[256];
  char c_trd_msg[256];

  if (DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName, "Inside Function fn_localdb_res");
  }

  strcpy(c_msg, "LDB information downloaded");

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "MSG Is :%s:", c_msg);
  }

  ptr_st_ldb_data->st_hdr.li_log_time = ntohl(ptr_st_ldb_data->st_hdr.li_log_time); /*** Ver 1.7 ***/
  fn_long_to_timearr(c_tm_stmp, ptr_st_ldb_data->st_hdr.li_log_time);

  sprintf(c_trd_msg, "|%s| %s|", c_tm_stmp, c_msg);

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "Trade Msg Is :%s:", c_trd_msg);
  }

  fn_pst_trg(c_ServiceName, "TRG_ORS_CON", c_trd_msg, c_pipe_id);

  return 0;
}

int fn_validate_tap(int *i_exg_seq, char *c_wholeptr_data, char *c_ServiceName)
{
  struct st_net_hdr st_net_header;
  int i_len_of_packet = 0;
  int i_size_of_tap_hdr = 0;
  int i_chk = 0;
  unsigned char c_digest[16];

  i_size_of_tap_hdr = sizeof(struct st_net_hdr);

  MEMSET(c_digest);
  MEMSET(st_net_header);
  memcpy(&st_net_header, c_wholeptr_data, i_size_of_tap_hdr);

  fn_cnvt_nth_net_hdr(&st_net_header); /*** Ver 1.7 ***/

  if (DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName, "sequence number of tap packet :%d:", st_net_header.i_seq_num);
    fn_userlog(c_ServiceName, "sequence number incoming      :%d:", *i_exg_seq);
  }

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "difference :%d:", *i_exg_seq - st_net_header.i_seq_num);
  }

  *i_exg_seq += 1;

  i_len_of_packet = st_net_header.si_message_length - i_size_of_tap_hdr;

  fn_cal_md5digest((void *)c_wholeptr_data + i_size_of_tap_hdr, i_len_of_packet, c_digest);

  if (memcmp(st_net_header.c_checksum, c_digest, 16))
  {
    fn_userlog(c_ServiceName, "Error in Validating checksum");
    return -1;
  }

  return 0;
}

/* Ver 1.3 Starts */
int fn_chng_exp_passwd_req(char *c_pipe_id, char *c_new_gen_passwd, char *c_ServiceName, char *c_err_msg)
{
  int i_cnt;
  int i_login_stts;
  int i_snd_seq;
  int i_ch_val;
  int i_tran_id;
  int i_seq;
  int i_nxt_seq;
  int i_ret_val;
  int i_num1;
  int i_num2;

  char c_curnt_passwd[LEN_PASS_WD];
  char c_new_passwd[LEN_PASS_WD];
  char c_passwd_flg;
  char c_num[2];
  char c_command[100];
  char c_char;

  MEMSET(c_curnt_passwd);
  MEMSET(c_new_passwd);

  EXEC SQL
      SELECT OPM_EXG_PWD
          INTO : c_curnt_passwd
                     FROM OPM_ORD_PIPE_MSTR
                         WHERE OPM_PIPE_ID = : c_pipe_id;
  if (SQLCODE != 0)
  {
    fn_userlog(c_ServiceName, " Error in getting current password");
    fn_errlog(c_ServiceName, "L31130", SQLMSG, c_err_msg);
    return (-1);
  }

  rtrim(c_curnt_passwd);

  fn_userlog(c_ServiceName, " The decripted current password :%s:", c_curnt_passwd);

  fn_decrypt(c_curnt_passwd, c_curnt_passwd);
  i_ret_val = fn_get_passwd(c_curnt_passwd, c_new_passwd, CRNT_PSSWD_LEN);
  rtrim(c_new_passwd);
  fn_userlog(c_ServiceName, "New Password Generated on old Password Expiry :%s:", c_new_passwd);
  fn_encrypt(c_new_passwd, c_new_passwd);
  /*** Ver 1.4 Ends Here ***/

  rtrim(c_new_passwd);

  fn_userlog(c_ServiceName, "New Password  :%s:", c_new_passwd);

  /**** Update new password requested in BPH table ***/

  i_tran_id = fn_begintran(c_ServiceName, c_err_msg);
  if (i_tran_id == -1)
  {
    fn_errlog(c_ServiceName, "L31135", LIBMSG, c_err_msg);
    return -1;
  }

  EXEC SQL
      UPDATE OPM_ORD_PIPE_MSTR
          SET OPM_NEW_EXG_PWD = : c_new_passwd
                                      WHERE OPM_PIPE_ID = : c_pipe_id;

  if (SQLCODE != 0)
  {
    fn_userlog(c_ServiceName, " Error in Update of new password");
    fn_aborttran(c_ServiceName, i_tran_id, c_err_msg);
    fn_errlog(c_ServiceName, "L31140", SQLMSG, c_err_msg);
    return (-1);
  }

  i_ch_val = fn_committran(c_ServiceName, i_tran_id, c_err_msg);
  if (i_ch_val == -1)
  {
    fn_errlog(c_ServiceName, "L31145", LIBMSG, c_err_msg);
    return -1;
  }

  /** Ver 1.4 Starts Here ***/
  fn_decrypt(c_new_passwd, c_new_passwd);
  MEMSET(c_command);
  c_char = '"';
  sprintf(c_command, "ksh snd_passwd_chng_mail.sh %s '%c%s%c'", c_pipe_id, c_char, c_new_passwd, c_char);
  system(c_command);
  /*** Ver 1.4 Ends here ***/

  strcpy(c_new_gen_passwd, c_new_passwd);

  return 0;
}
/* Ver 1.3 Ends */

/*** Ver 1.6 Starts ***/

int fn_exch_exception_dwld(struct st_brd_exch_exception *st_brd_exch_exception_msg, char *c_xchng_cd, char *c_pipe_id, char *c_ServiceName, char *c_errmsg)
{
  int i_ret_val;

  struct st_bcast_message st_bcast_msg;

  if (DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName, "Inside Function fn_exch_exception_dwld");
    fn_userlog(c_ServiceName, "Exchange code is :%s:", c_xchng_cd);
  }

  if (st_brd_exch_exception_msg->status == 1)
  {
    if (DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "Inside Condition BCAST_CONT_MSG(Exch Exception) in Download");
    }
    strcpy(st_bcast_msg.c_broadcast_message, "Exception Handling Start of Outage in Download,");
  }
  else if (st_brd_exch_exception_msg->status == 0)
  {
    if (DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName, "Inside Condition BCAST_CONT_MSG(Exch Exception) in Download");
    }
    strcpy(st_bcast_msg.c_broadcast_message, "Exception Handling End of Outage in Download,");
  }
  else
  {
    fn_userlog(c_ServiceName, "Inside Condition BCAST_CONT_MSG(Exch Exception) INVALID STATUS in Download");
    return -1;
  }

  if ((st_brd_exch_exception_msg->streamnumber == 0))
  {
    fn_userlog(c_ServiceName, "Inside Condition All Streams Impacted ");
    strcat(st_bcast_msg.c_broadcast_message, "All Streams Imapcted");
  }
  else if ((st_brd_exch_exception_msg->streamnumber))
  {
    fn_userlog(c_ServiceName, "Inside Condition Specific Stream No:%d: Impacted ", st_brd_exch_exception_msg->streamnumber);
    sprintf(st_bcast_msg.c_broadcast_message, "%s Specific Stream No:%d: Impacted ", st_bcast_msg.c_broadcast_message, st_brd_exch_exception_msg->streamnumber);
  }
  st_bcast_msg.st_hdr.li_log_time = st_brd_exch_exception_msg->st_hdr.li_log_time;
  st_bcast_msg.st_bcast_dest.flg_cntrl_ws = 1;

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "Exchange Msg Is :%s:", st_bcast_msg.c_broadcast_message);
    fn_userlog(c_ServiceName, "Exchange Log Time Is :%ld:", st_bcast_msg.st_hdr.li_log_time);
    fn_userlog(c_ServiceName, "Before Call to Function fn_gnrltrd_msg");
  }

  i_ret_val = fn_gnrltrd_msg(&st_bcast_msg,
                             c_xchng_cd,
                             c_pipe_id,
                             c_ServiceName,
                             c_errmsg);

  if (i_ret_val == -1)
  {
    fn_errlog(c_ServiceName, "L31150", LIBMSG, c_errmsg);
    return -1;
  }

  return 0;
}

/*** Ver 1.6 Ends ***/

int fn_opn_ord_dtls(struct st_oe_reqres *ptr_oe_reqres,
                    char *c_pipe_id,
                    char *c_rout_str,
                    char *c_ServiceName,
                    char *c_err_msg)
{
  int i_ch_val;

  struct vw_orderbook st_i_ordbk;

  if (DEBUG_MSG_LVL_3) /*** Ver 4.9 ** debug changed form 0 to 3 **/
  {
    fn_userlog(c_ServiceName, "Inside Function fn_opn_ord_dtls");
  }

  sprintf(st_i_ordbk.c_xchng_ack, "%16.0lf", ptr_oe_reqres->d_order_number);

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "Exchange Ack Is :%s:", st_i_ordbk.c_xchng_ack);
    fn_userlog(c_ServiceName, "Total Volume :%ld:", ptr_oe_reqres->li_volume);
    fn_userlog(c_ServiceName, "Total Volume Remaining :%ld:", ptr_oe_reqres->li_total_volume_remaining);
    fn_userlog(c_ServiceName, "Rout String Is :%s:", c_rout_str);
  }

  st_i_ordbk.l_exctd_qty = ptr_oe_reqres->li_volume - ptr_oe_reqres->li_total_volume_remaining;
  st_i_ordbk.l_ord_tot_qty = ptr_oe_reqres->li_volume;
  strcpy(st_i_ordbk.c_pipe_id, c_pipe_id);

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "Pipe ID is :%s:", c_pipe_id);
    fn_userlog(c_ServiceName, "Executed Qty Is :%ld:", st_i_ordbk.l_exctd_qty);
    fn_userlog(c_ServiceName, "Total Order Qty Is :%ld:", st_i_ordbk.l_ord_tot_qty);
  }

  /*fn_cpy_ddr(st_i_ordbk.c_rout_crt); */ /*** Ver 1.1 ***/

  strcpy(st_i_ordbk.c_rout_crt, c_rout_str); /*** Ver 1.1 ***/

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "Before Call To SFO_OPN_ORD");
  }

  i_ch_val = fn_acall_svc(c_ServiceName,
                          c_err_msg,
                          &st_i_ordbk,
                          "vw_orderbook",
                          sizeof(st_i_ordbk),
                          TPNOREPLY,
                          "SFO_OPN_ORD");
  if (i_ch_val != 0)
  {
    fn_errlog(c_ServiceName, "L31005", LIBMSG, c_err_msg);
    return -1;
  }

  if (DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName, "After Complition Of fn_opn_ord_dtls");
  }

  return 0;
}