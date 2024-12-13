/******************************************************************************/
/*  Program						:	<fn_msgq.h>                                           */
/*                                                                            */
/*  Input 						:																												*/
/*                                                                            */
/*  Output						:                                                       */
/*                                                                            */
/*	Description				:	Header file for Message Queue.                        */
/*                                                                            */
/*	Log								:	ver 1.1 19-Sep-2007 IBM migration rohit               */
/*	Log								:	ver 1.2 30-Jun-2009 IBM migration Shailesh            */
/*                                                                            */
/******************************************************************************/
/* 23/10/2001					S.Balamurugan						v1.0					Release 1.0       */
/******************************************************************************/

#define					DONT_EXIT				1
#define					EXIT						0
#define					NO_DATA					-3
#define					OWNER						2
#define					CLIENT					1

/*		Funtion Protytype			*/
int fn_crt_msg_q(int *ptr_i_qid,
                int i_msgq_typ,
								int i_proc_typ,
                char *ptr_c_ServiceName,
                char *ptr_c_err_msg);

void fn_dstry_msg_q(int i_qid,
                   char *ptr_c_ServiceName,
                   char *ptr_c_err_msg);

int fn_write_msg_q(int i_qid,
                   void *ptr_v_msg,
                   long int li_len,
                   char *ptr_c_ServiceName,
                   char *ptr_c_err_msg);  

int fn_read_spcd_msg_q(int l_qid,
                      long int li_type,
                      void *ptr_v_msg,
                      long int *ptr_li_len,
                      char *ptr_c_ServiceName,
                      char *ptr_c_err_msg);

int fn_read_msg_q(int i_qid,
                  void *ptr_q_data,
                  long int *ptr_li_len,
                  char *ptr_c_ServiceName,
                  char *ptr_c_err_msg);

int fn_get_no_of_msg(int i_qid,
                     unsigned int *ptr_li_no_msg,			/** ver 1.1 long int changed to unsigned int **/
                     char *ptr_c_ServiceName,
                     char *ptr_c_err_msg);

int fn_flush_q(int i_qid,
               char *ptr_c_ServiceName,
               char *ptr_c_err_msg);

int fn_set_break_q(void);															/** Ver 1.2 **/

