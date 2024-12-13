/******************************************************************************/
/*	program			: fn_scklib_tap.h																							*/
/*																																					  */
/*	Input				:																														  */
/*																																						*/
/*	Output			:																													    */
/*																																					  */
/*	Description	:	Header file for the Socket Library     										  */
/*                                                                            */
/*  Log         :                                                             */
/******************************************************************************/
/*  Shailesh Hinge						15-Apr-2008							ver 1.0									*/
/******************************************************************************/
/* VER TOL : TUX on LINUX */
#pragma once
#include <fo_exch_comnN.h>


# define TM_0 0 
# define TM_30 30 

#	define  DATA_RCVD  2
# define  MAX_SCK_MSG	1024	
#	define	SET_KEEPALIVE	1				
# define	SET_SELECT_SIZE	1
# define 	SET_TCP_NODELAY	1
# define 	SET_TCP_NODELAY_ACK	1

#define   TEN_YRS_IN_SEC  315513000

/** Added by Indrajit for ORS2 Broadcast START **/
#define  START 				1
#define  STOP       	0
#define  STATUS     	2 
#define  CHILD     		0
#define  TM_30     		30
#define  BOTH_DWN		  0
#define  BOTH_UP		  1
#define  BRD_SRVR_UP  2 
#define  BRD_PRCS_UP  3 
/** Added by Indrajit for ORS2 Broadcast END **/

#define  CLIENT 1



int fn_socket(void);

struct st_frs_hdr
{
  long int li_msg_typ;
  long int li_msg_len;
};

typedef struct st_frs_hdr msg_frs_hdr;

int fn_crt_clnt_sck(char *ptr_c_ip_addr,
                    int i_port_no,
                    int *ptr_i_sck_id,
										char *c_ServiceName,
										char *c_err_msg);

int fn_close_sck(int i_sck_id,
							   char *c_ServiceName,
								 char *c_err_msg);

int fn_readn(int i_sck_id,
             void *ptr_v_data,
             long int li_len,
             long int li_timeout,
						 char *c_ServiceName,
						 char *c_err_msg);

int fn_writen(int i_sck_id,
              void *ptr_v_data,
              long int li_len,
							char *c_ServiceName,
							char *c_err_msg);

/** Added by Indrajit for ORS2 Broadcast START **/
int fn_read_sck(		int i_sck_id,
                    void *ptr_v_data,
                    msg_frs_hdr *st_msg_hdr,
                    long int li_timeout,
                    char *c_ServiceName,
                    char *c_err_msg);

int fn_write_sck(int i_sck_id,
                 void *ptr_v_data,
                 msg_frs_hdr st_msg_frs_hdr,
                 char *c_ServiceName,
                 char *c_err_msg);
/** Added by Indrajit for ORS2 Broadcast END **/


int fn_write_msg_sck(int i_sck_id,
                     void *ptr_v_data,
                     struct st_int_header st_msg_frs_hdr1,
										 char *c_ServiceName,
										 char *c_err_msg);

int fn_read_xchng_sck(int i_sck_id,
                    void *ptr_v_data,
                    long int li_timeout,
										char *c_ServiceName,
										char *c_err_msg);


void fn_alrm_intr(int signo);

int fn_set_tmr_sigdisp(long int li_time,
											 char *c_ServiceName,
											 char *c_err_msg);
	
void fn_set_break_sck(void);

int fn_crt_srvr_sck(int i_port_no,
                    int *ptr_i_sck_id,
                    char *c_ServiceName,
                    char *c_err_msg);

int fn_accept_clnt (int i_sck_id,
                    int *ptr_i_clnt_sckid,
                    char *c_ServiceName,
                    char *c_err_msg);


/*inline*/ void fn_orstonse_char ( char *ptr_c_dest,
                               long int li_len_dest,
                               const char *ptr_c_src,
                               long int li_len_src );

/*inline*/ void fn_nsetoors_char ( char *ptr_c_dest,
                               long int li_len_dest,
                               char *ptr_c_src,
                               long int li_len_src );

/*inline*/ void fn_orstonse_long ( long int *ptr_li_dest,
                               const long int li_src );

/*inline*/ void fn_nsetoors_long ( long int *ptr_li_dest,
                               const long int li_src );

/*inline*/ void fn_orstonse_short ( short int *ptr_si_dest,
                                const short int si_src );

/*inline*/ void fn_nsetoors_short ( short int *ptr_si_dest,
                                const short int si_src );

/*inline*/ void fn_orstonse_tm ( long int *ptr_li_dest,
                             char *c_tm_stmp );

/*inline*/ void fn_nsetoors_tm ( char *c_tm_stmp,
                             long int li_src );

void fn_sck_init(void);

int fn_crt_mcast_sck(char *c_hst_addrss,
                     int  i_port_no,
                     char *c_dest_addrss,
                     int  *ptr_i_sck_id,
                     char *c_ServiceName,
                     char *c_err_msg);

int fn_close_mcast_sck(int  i_sck_id,
                       char *c_hst_addrss,
                       char *c_dest_addrss,
                       char *c_ServiceName,
                       char *c_err_msg);


int fn_recv_udp_msg(int  i_sck_id,
                    void *ptr_v_data,
                    long int li_len,
                    long int li_timeout,
                    char *c_ServiceName,
                    char *c_err_msg);

unsigned short fn_twiddle(unsigned short int i_input);

void binary(int num ,int size);

void fn_timearr_to_long(char *c_tm_stmp, long int *ptr_li_dest);

void fn_long_to_timearr(char *c_tm_stmp,long int li_src);

int fn_sendto ( int  i_fd,
                int  i_family,
                char *ptr_c_stm,
                int  i_length,
                char *ptr_c_ip_addr,
                int  i_prt_no,
                char *c_ServiceName,
                char *c_errmsg);

/** Added by Shamili **/
int fn_snd_rcv_msg_sck(int i_sck_id,
                       void *ptr_v_ip_data,
                       msg_frs_hdr st_ip_hdr,
                       void *ptr_v_op_data,
                       msg_frs_hdr st_op_hdr,
                       char *c_ServiceName,
                       char *c_err_msg );


int fn_udp_socket (void);


int fn_crt_foudp_sck ( int   *ptr_i_sck_id,
                       char  *c_brd_port,
											 char  *c_ServiceName
                     );
