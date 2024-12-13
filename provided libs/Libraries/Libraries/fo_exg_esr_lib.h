/*** Ver 1.1 Stream No. Exchange Exception Handling Changes  | Parag Kanojia (30-Jul-2018) ***/

#include <fo_exg_msgs.h>

# define RESP_RCVD 0
# define UNSOL_RESP_RCVD 1

# define DOWNLOAD     10
# define NOT_DOWNLOAD     20

# define NO_MISMATCH  1
# define MISMATCH     2

# define LEN_PIPE_ID  3

# define EXIT_BY_SIGUSR2 1
# define EXIT_BY_RCV_THRD 2
# define EXIT_BY_SND_THRD 3

# define REQ_SENT       1
# define LOGON_REQ_SENT 2
# define LOGON_RESP_RCVD 3
# define SID_REQ_SENT 4
# define SID_RESP_RCVD 5
# define LDB_REQ_SENT 6
# define LDB_RESP_RCVD 7
# define DWNLD_REQ_SENT 8
# define DWNLD_OVER 9
# define ORD_REQ_SENT 10
# define ORD_RESP_RCVD 11
# define SPD_ORD_REQ_SENT 12
# define SPD_ORD_RESP_RCVD 13
# define EXER_REQ_SENT 14
# define EXER_RESP_RCVD 15
# define SEND_MAX_REACH 16
# define SEND_UNBLOCK 17
# define WAITING_FOR_INVT_PCKT	18
# define INVT_PCKT_RCVD 19

# define RCV_ERR -1

# define FROM_INT 1
# define FROM_BRD 2

# define LIMIT_ABOUT_TO_EXCEED  1
# define LIMIT_EXCEEDED         2

# define NSE_BUY  1
# define NSE_SELL 2

# define NSE_CLIENT  1
# define NSE_PRO     2

# define TRADER             'T'
# define CORPORATE_MANAGER  'M'
# define BRANCH_MANAGER     'B'

# define LOG_DUMP      1
# define DONT_LOG_DUMP 0


int fn_system_information_out(struct st_system_info_data *st_sys_info_dat,
                              char *c_ServiceName,
                              char *c_err_msg);

int fn_do_xchng_logon( struct st_exch_msg *ptr_st_exch_msg,
											 FILE *ptr_snd_msgs_file, 
                       char *c_ServiceName,
                       char *c_errmsg );

int fn_sign_on_request_out( struct st_sign_on_res *ptr_st_logon_res ,
                      char *c_ServiceName,
                      char *c_err_msg );

int fn_logout_res ( char *c_xchng_cd,
										char *c_pipe_id,
										char *c_ServiceName,
										char *c_err_msg
                  );

int fn_partial_logon_res ( struct st_system_info_data *ptr_st_sysinf_dat ,
                           char *c_pipe_id,
                           char *c_ServiceName,
                           char *c_err_msg );

int fn_localdb_res ( struct st_local_database_data *ptr_st_ldb_data ,
                     char *c_pipe_id,
                     char *c_ServiceName,
                     char *c_err_msg );

int fn_validate_tap( int * i_exg_seq, char * c_wholeptr_data,char *c_ServiceName);

int fn_exch_exception_dwld( struct  st_brd_exch_exception *st_brd_exch_exception_msg,	/*** Function Declaration added in Ver 1.1 ***/
                            char *c_xchng_cd,
                            char *c_pipe_id,
                            char *c_ServiceName,
                            char *c_errmsg );
