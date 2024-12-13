/******************************************************************************/
/*	Program						:	fn_msgq.c                                             */
/*                                                                            */
/*	Input							:			                                                  */
/*                                                                            */
/* 	Output						:	                                                      */
/*                                                                            */
/* 	Description				:	Library file for Message Queue.                       */
/*                                                                            */
/*	Log               :                                                       */
/*                                                                            */
/******************************************************************************/
/*				23/10/2001				S.Balamurugan				v1.0				Release1.0        */
/*				13/12/2007				ROhit								v1.1				AIX Migration			*/
/******************************************************************************/
/***  C Header file  ***/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <userlog.h> /* Ver 1.1 */
#include <fn_log.h>	 /* Ver 1.1 */

/***  Application Header File  ***/
#include <fo.h>
#include <fn_msgq.h>

int i_break_status_q;

/******************************************************************************/
/*	Function Name			:	fn_crt_msg_q                                  */
/*                                                                            */
/*  Input							:	int *, char *, char *, char *         */
/*          																  */
/*  Output						:																											  */
/*                                                                            */
/*	Description				:	This function gets IPC key from ini file and creates  */
/*                      a message queue of specified type (send/Receive/      */
/*                      Broadcast). Queue identifier is returned as parameter */
/*                      (ptr_i_qid). On success, it return 0. On failure, it  */
/*                      logs an error and returns -1.                         */
/*                                                                            */
/*	Log								:                                                       */
/*                                                                            */
/******************************************************************************/
int fn_crt_msg_q(int *ptr_i_qid,
				 int i_msgq_typ,
				 int i_proc_typ,
				 char *ptr_c_ServiceName,
				 char *ptr_c_err_msg)
{
	int i_ch_val;

	/***   To get message queue identifier   ***/
	if (i_proc_typ == OWNER)
	{
		// Obtain a message queue identifier
		i_ch_val = msgget(
			i_msgq_typ,					  // 'i_msgq_typ' is the key that identifies the message queue
			(IPC_CREAT | IPC_EXCL | 0777) // Flags for creating a new queue:
			// IPC_CREAT: Create a new message queue if it doesn't exist.
			// IPC_EXCL: Ensure a new queue is created; fail if it already exists.
			// 0777: Set full read, write, and execute permissions for user, group, and others.
		);

		if (i_ch_val == -1)
		{
			fn_errlog(ptr_c_ServiceName, "L31005", UNXMSG, ptr_c_err_msg);
			return -1;
		}
	}
	else if (i_proc_typ == CLIENT)
	{
		i_ch_val = msgget(i_msgq_typ, 0);
		if (i_ch_val == -1)
		{
			fn_errlog(ptr_c_ServiceName, "L31010", UNXMSG, ptr_c_err_msg);
			return -1;
		}
	}
	else
	{
		fn_errlog(ptr_c_ServiceName, "L31015", "Protocol Error", ptr_c_err_msg);
		return -1;
	}
	*ptr_i_qid = i_ch_val;
	return 0;
}

/******************************************************************************/
/*	Function Name			:	fn_dstry_msg_q                                        */
/*    																																				*/
/*	Input							:	int, char *, char *                                   */
/*                                                                            */
/*  Output						:																												*/
/*																																						*/
/*	Description				:	This function removes the message queue identifier,   */
/*                      which is given as parameter. 	                        */
/*                      If success, it returns 0 else -1.                     */
/*                                                                            */
/*	Log								:																												*/
/* 																																						*/
/******************************************************************************/
void fn_dstry_msg_q(int i_qid,
					char *ptr_c_ServiceName,
					char *ptr_c_err_msg)
{

	if (msgctl(i_qid, IPC_RMID, NULL) == -1)
	{
		fn_errlog(ptr_c_ServiceName, "L31020", UNXMSG, ptr_c_err_msg);
	}
}

/******************************************************************************/
/*	Function Name			:	fn_write_msg_q																				*/
/*    																																				*/
/*	Input							:	int, void *, long int, char *, char *                 */
/*                                                                            */
/*  Output						:																												*/
/*																																						*/
/*	Description				:	This function sends a message to a specified message  */
/*                      queue. If it is interrupted by any signal except user */
/*                      signal, it won't come out of the function until it    */
/*                      completes. Function will come out at any one of the   */
/*                      following situation:                                  */
/*                      a) Success                                            */
/*                      b) Message Queue is full                              */
/*                      c) Message Queue id is removed.                       */
/*                      If success, it returns 0 else -1.                     */
/*                                                                            */
/*	Log								:																												*/
/* 																																						*/
/******************************************************************************/
int fn_write_msg_q(int i_qid,
				   void *ptr_v_msg,
				   long int li_len,
				   char *ptr_c_ServiceName,
				   char *ptr_c_err_msg)
{
	if ((msgsnd(i_qid, ptr_v_msg, li_len, IPC_NOWAIT)) == -1)
	{
		fn_errlog(ptr_c_ServiceName, "L31025", UNXMSG, ptr_c_err_msg);
		return -1;
	}

	return 0;
}

/******************************************************************************/
/*	Function Name			:	fn_read_spcd_msg_q																		*/
/*    																																				*/
/*	Input							:	int, long int, void *, long int, char *, char *       */
/*                                                                            */
/*  Output						:																												*/
/*																																						*/
/*	Description				:	This function reads a specified type of message from  */
/*                      the message queue. If there is no message found in    */
/*                      queue of specified type, it will return NO_DATA.      */
/*                      If success, it stores the message in a given pointer, */
/*                      gives a number of bytes of message has read and       */
/*                      return 0. On failure, it logs an error and            */
/*                      return -1.                                            */
/*                                                                            */
/*	Log								:																												*/
/* 																																						*/
/******************************************************************************/
int fn_read_spcd_msg_q(int i_qid,
					   long int li_type,
					   void *ptr_v_msg,
					   long int *ptr_li_len,
					   char *ptr_c_ServiceName,
					   char *ptr_c_err_msg)
{

	long int i_received;
	if ((i_received = msgrcv(i_qid, ptr_v_msg, *ptr_li_len, li_type, IPC_NOWAIT)) == -1)
	{
		if (errno == ENOMSG)
		{
			return NO_DATA;
		}
		else
		{
			fn_errlog(ptr_c_ServiceName, "L31030", UNXMSG, ptr_c_err_msg);
			return -1;
		}
	}
	else
	{
		/***  Assigning size of message  ***/
		*ptr_li_len = i_received;
		return 0;
	}
}

/******************************************************************************/
/*	Function Name			:	fn_read_msg_q     																		*/
/*    																																				*/
/*	Input							:	int, void *, long int, char *, char *                 */
/*                                                                            */
/*  Output						:																												*/
/*																																						*/
/*	Description				:	This function reads a message from queue in FIFO basis*/
/*                      If there is no message in queue, it will wait till the*/
/*                      message comes. If it is interrupted, it will receive  */
/*                      the message again. But it will come out if the user   */
/*                      signal stating an exit is caught.                     */
/*                      If success, it return size of message received else   */
/*                      return -1.                                            */
/*                                                                            */
/*	Log								:																												*/
/* 																																						*/
/******************************************************************************/
int fn_read_msg_q(int i_qid,
				  void *ptr_q_data,
				  long int *ptr_li_len,
				  char *ptr_c_ServiceName,
				  char *ptr_c_err_msg)
{
	long int i_received;
	i_break_status_q = DONT_EXIT;

	while (i_break_status_q == DONT_EXIT)
	{
		if ((i_received = msgrcv(i_qid, ptr_q_data, *ptr_li_len, 0, 0)) == -1)
		{
			if (errno != EINTR)
			{
				fn_errlog(ptr_c_ServiceName, "L31035", UNXMSG, ptr_c_err_msg);
				return -1;
			}
		}
		else
		{
			/***  Storing no., of bytes received  ***/
			*ptr_li_len = i_received;
			return 0;
		}
	}

	fn_errlog(ptr_c_ServiceName, "L31040", "SIGUSR2 signal is caught", ptr_c_err_msg);
	return -1;
}

/******************************************************************************/
/*	Function Name			:	fn_get_no_of_msg  																		*/
/*    																																				*/
/*	Input							:	int, long int, char *, char *                         */
/*                                                                            */
/*  Output						:																												*/
/*																																						*/
/*	Description				:	This function gives total number of messages in queue.*/
/*                      The status structure (msqid_ds) of a queue is attached*/
/*                      with each queue. The status structure of the queue    */
/*                      can be obtained by msgctl function. From the status   */
/*                      structure we can get the total no., of messages in    */
/*                      queue.                                                */
/*                      If success, it return 0 else -1                       */
/*                                                                            */
/*	Log								: ver 1.1 19-sep-2007 IBM migration	rohit								*/
/* 																																						*/
/******************************************************************************/
int fn_get_no_of_msg(int i_qid,
					 unsigned int *ptr_li_no_msg, /* Ver 1.1 long int changed to unsigned int **/
					 char *ptr_c_ServiceName,
					 char *ptr_c_err_msg)
{
	/***  Get the status structure of a queue  ***/
	struct msqid_ds st_temp_status;
	if ((msgctl(i_qid, IPC_STAT, &st_temp_status)) == -1)
	{
		fn_errlog(ptr_c_ServiceName, "L31045", UNXMSG, ptr_c_err_msg);
		return -1;
	}
	else
	{
		/***  Storing no., of messages  ***/
		*ptr_li_no_msg = st_temp_status.msg_qnum;
		return 0;
	}
}

/******************************************************************************/
/*	Function Name			:	fn_flush_q        																		*/
/*    																																				*/
/*	Input							:	int, char *, char *                                   */
/*                                                                            */
/*  Output						:																												*/
/*																																						*/
/*	Description				:	This function flushes all messages in the queue.      */
/*                      Flushing is nothing but reading all the messages in   */
/*                      a queue and discards it.                              */
/*                      If success, it return 0 else logs an error and        */
/*                      return -1                                             */
/*                                                                            */
/*	Log								:																												*/
/* 																																						*/
/******************************************************************************/
int fn_flush_q(int i_qid,
			   char *ptr_c_ServiceName,
			   char *ptr_c_err_msg)
{
	char tmp_buffer[3072];
	long int li_len;

	i_break_status_q = DONT_EXIT;
	li_len = sizeof(tmp_buffer);
	while (i_break_status_q == DONT_EXIT)
	{
		if (msgrcv(i_qid, &tmp_buffer, li_len, 0, IPC_NOWAIT) == -1)
		{
			if (errno == ENOMSG)
			{
				return 0;
			}
			else if (errno != EINTR)
			{
				fn_errlog(ptr_c_ServiceName, "L31050", UNXMSG, ptr_c_err_msg);
				return -1;
			}
		}
	}
	fn_errlog(ptr_c_ServiceName, "L31055", UNXMSG, ptr_c_err_msg);
	return -1;
}

/******************************************************************************/
/*	Function Name			:	fn_set_break_q    																		*/
/*    																																				*/
/*	Input							:	                                                      */
/*                                                                            */
/*  Output						:																												*/
/*																																						*/
/*	Description				:	This function sets the break_status_q variable to     */
/*                      indicate that the user signal is caught.              */
/*                                                                            */
/*	Log								:																												*/
/* 																																						*/
/******************************************************************************/
int fn_set_break_q()
{
	i_break_status_q = EXIT;
}
