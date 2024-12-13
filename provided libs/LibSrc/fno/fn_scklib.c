/******************************************************************************/
/*	Program					:	fn_scklib.c   									  									    */
/*																																				    */
/*	Input						:																											    */
/*																																				    */
/*  Output					:																											    */
/*																																				    */
/*	Description			:	Library File for Socket Handling										    */
/*																																				    */
/*	Log							:																											    */
/*																																						*/
/******************************************************************************/
/*	Swamy Boggarapu						23-Oct-2001					ver 1.0									    */
/******************************************************************************/
/*	Ver 1.1		20-Mar-2008		Changes to implement keep alive on client sockets.*/
/*													(SKS)																			    		*/
/*  Ver 1.2   03-Apr-2008 AIX Migration		ROhit																*/
/******************************************************************************/
/****	 C header   ********/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#include <fn_scklib.h>
#include <fn_signal.h>
#include <fo.h>
#include <sys/time.h>   /* Ver 1.2 */
#include <strings.h>    /* Ver 1.2 */
#include <unistd.h>			/* Ver 1.2 */	
#include <fn_log.h>				/* Ver 1.2 */
#include <arpa/inet.h>  /* Ver 1.2 */


/******************************************************************************/
/*Global variables 																													  */
/******************************************************************************/
/******** to trap timer signal interrupt *********/
int i_sig_intrpt_trp = 0;

/******** to trap user signal caught   ***********/
int i_break_status_sck = DONT_EXIT;

int fn_socket(void);			/* Ver 1.2 */

/*****************************************************************************/
/* Function to create a socket having protocol family as AF_INET and socket  */
/* type as SOCK_STREAM 																											 */
/*****************************************************************************/
int fn_socket()
{
  int i_tmp_sckid;

  i_tmp_sckid = socket(AF_INET,SOCK_STREAM,0);
  if (i_tmp_sckid == -1)
  {
    return -1;
  }

  return i_tmp_sckid;
}
 
/*****************************************************************************/
/*  Function to create a socket and connects it to known port of a specified */
/*  machine. Before connecting to the machine, the internet socket address   */
/*  structure is to be filled with server machine's IP address and port      */
/*  number.																																	 */ 
/*																																 					 */
/*  INPUT PARAMETERS																												 */
/*				ptr_c_ip_addr		IP address of the destination machine.						 */
/*				i_port_no				port number to which connection is to be made.		 */
/*				c_ServiceName		The Service which is calling this function.			   */
/* 				c_err_msg				The error message which has to be displayed.			 */
/*																																					 */
/*	OUTPUT PARAMETERS																											   */
/*				ptr_i_sck_id		The id of the socket created.											 */
/*****************************************************************************/
int fn_crt_clnt_sck(char *ptr_c_ip_addr, 
                    int i_port_no,
                    int *ptr_i_sck_id,
										char *c_ServiceName,
										char *c_err_msg)
{
    int i_tmp_sck_id;
		struct sockaddr_in sck_in;

		  int i_setval; 				/*	Ver 1.1	*/
  		socklen_t ud_optlen;  /*	Ver 1.1	*/

    if ((i_tmp_sck_id = fn_socket()) == -1)
		{
			fn_errlog(c_ServiceName, "L31005", UNXMSG, c_err_msg);
	  	return -1;    	
		}

		/*	Ver 1.1 : Addition Begins	*/
  i_setval  = SET_KEEPALIVE;
  ud_optlen = (socklen_t) sizeof(int);

  if(setsockopt(i_tmp_sck_id, SOL_SOCKET, SO_KEEPALIVE, &i_setval, ud_optlen) == -1)
  {
    fn_errlog(c_ServiceName, "L31010", UNXMSG, c_err_msg);
    fn_close_sck(i_tmp_sck_id, c_ServiceName, c_err_msg);
    return -1;
  }
  /*	Ver 1.1 : Addition Ends	*/

		bzero(&sck_in,sizeof(struct sockaddr_in)); 
		sck_in.sin_family = AF_INET;
		sck_in.sin_port = htons(i_port_no);
		sck_in.sin_addr.s_addr = inet_addr(ptr_c_ip_addr);

		if ( connect ( i_tmp_sck_id,
								   (struct sockaddr *)&sck_in,
								   sizeof(struct sockaddr_in)  ) == -1 ) 
		{
			fn_errlog(c_ServiceName, "L31015", UNXMSG, c_err_msg);
			fn_close_sck(i_tmp_sck_id, c_ServiceName, c_err_msg);		/*	Ver 1.1 : Added	*/
			return -1;
		}

		*ptr_i_sck_id = i_tmp_sck_id;
		return 0;
}

/*****************************************************************************/
/*    This function creates a socket having the protocol family AF_INET and  */
/* of socket type SOCK_STREAM. the server's well known port number is bound  */
/* to the socket by filling internet socket address structure with port      */
/* number. this socket waits on listening the requests from the client.      */
/*																																					 */
/*	INPUT PARAMETERS																												 */
/*		i_port_no				The port which is bound to the socket for connection	 */
/*		c_ServiceName		The Service requested the service socket creation.		 */
/*		c_err_msg				The error message which needs to be displayed on screen*/
/*																																					 */
/*	OUTPUT PARAMETERS																												 */
/*		ptr_i_sck_id		The id of the socket got created.											 */
/*																																					 */	
/*****************************************************************************/
int fn_crt_srvr_sck (int i_port_no,
										 int *ptr_i_sck_id,
										 char *c_ServiceName,
										 char *c_err_msg)
{
	int i_tmp_sckid;
  struct sockaddr_in sck_in;

	if ((i_tmp_sckid = fn_socket())==-1)
	{
		fn_errlog(c_ServiceName, "L31020", UNXMSG, c_err_msg);
		return -1;
	}


	bzero(&sck_in, sizeof(struct sockaddr_in));

	sck_in.sin_family = AF_INET;
	sck_in.sin_port = htons(i_port_no);
	sck_in.sin_addr.s_addr = htonl(INADDR_ANY);
	if ( bind ( i_tmp_sckid, 
              (struct sockaddr *)&sck_in, 
              sizeof(struct sockaddr_in)  ) == -1 )
	{
		fn_errlog(c_ServiceName, "L31025", UNXMSG, c_err_msg);
		return -1;
	}

	if(listen(i_tmp_sckid,0)==-1)
	{
		fn_errlog(c_ServiceName, "L31030", UNXMSG, c_err_msg);
		return -1;
	}

	*ptr_i_sck_id = i_tmp_sckid;
	return 0; 
}

/*****************************************************************************/
/* This function puts the server process to sleep mode, waiting for a client */
/* connection to arrive. when connect request comes, server accepts taht and */
/* assigns to a new socket. if it is interrupted by the user signal, it will */
/* come out of the loop. otherwise it will wait for client connection.       */
/*																																					 */
/*	INPUT PARAMETERS																												 */
/*			i_sck_id			socket id.																						 */
/*			c_ServiceName	The service requested this function.									 */
/*			c_err_msg			The error message needs to be displayed on screen.		 */
/*																																					 */
/*	OUTPUT PARAMETERS																												 */	
/*			ptr_i_clnt_sckid  The client socket id.													     */
/*																																					 */	
/*****************************************************************************/
int fn_accept_clnt(int i_sck_id, 
									 int *ptr_i_clnt_sckid,
									 char *c_ServiceName,
									 char *c_err_msg)
{
	i_break_status_sck = DONT_EXIT;

	while (i_break_status_sck == DONT_EXIT)
	{
		if ((*ptr_i_clnt_sckid = accept(i_sck_id,NULL,NULL)) == -1)
		{
			if (errno != EINTR)
			{
				fn_errlog(c_ServiceName, "L31035", UNXMSG, c_err_msg);
				return -1;
			}
		}
		else
		{
			return 0;
		}
	}

	fn_errlog(c_ServiceName, "L31040", "Interrupt caught", c_err_msg);
	return -1;  
}

/******************************************************************************/
/*  This function is to close the socket. on success, it returns 0 and on     */
/* failure, it logs the error and returns -1																	*/
/* 																																						*/
/*	INPUT PARAMETERS																													*/
/*			i_sck_id			The socket which has to be closed.  										*/
/*			c_ServiceName	The Service that wants to close the socket.							*/
/*			c_err_msg			The error message that wants to be dispalyed on screen. */
/******************************************************************************/
int fn_close_sck(int i_sck_id,
								 char *c_ServiceName,
								 char *c_err_msg)
{
	if (close(i_sck_id)==-1)
	{
		fn_errlog(c_ServiceName, "L31045", UNXMSG, c_err_msg);
		return -1;
	}

	return 0;
}

/******************************************************************************//*  This function reads N bytes from socket within a specified time. First it */
/* sets the timer for specified time(li_timeout). There is no time limit if   */
/* li_timeout parameter is specified as 0. it reads socket buffer until it    */
/* reads li_len bytes. in between,if it is interrupted by user signal or time */
/* out signal, it will come out of the loop. otherwise it continues in its    */
/* loop. 																																			*/
/*																																						*/
/*	INPUT PARAMETERS																													*/
/*		i_sck_id			Id of the socket for reading.															*/
/*		ptr_v_data  	Pointer to store the read data.														*/
/*		li_len				Length of the data to be read.														*/
/*		li_timeout		The timeout limit for the operation.											*/
/*		c_ServicName  The service that requested the read operation.						*/
/*		c_err_msg			The error message that has to be displayed on screen.			*/
/*																																						*/
/******************************************************************************/int fn_readn(int i_sck_id,
						 void *ptr_v_data,
						 long int li_len,
						 long int li_timeout,
						 char *c_ServiceName,
					   char *c_err_msg)
{
	long int li_left;
  int i_read;
	char *ptr_v_tmp_data;
	i_break_status_sck= DONT_EXIT;

	li_left = li_len;
	i_read = 0;
	ptr_v_tmp_data = (char *)ptr_v_data;
	if(li_timeout > 0)
	{
		if ( fn_set_tmr_sigdisp ( li_timeout,
													    c_ServiceName,
													    c_err_msg       ) == -1 )
		{
			return -1;
		}
	}

	while ( (li_left > 0) && (i_break_status_sck == DONT_EXIT) )
	{
		if ( ( i_read = read ( i_sck_id,
											     ptr_v_tmp_data,
											     li_left         ) ) == -1 )	
		{
			if (errno == EINTR)
			{
				if (i_sig_intrpt_trp == 1) /*** Timer is over ***/
			  {
					i_sig_intrpt_trp=0;
					fn_errlog(c_ServiceName, "L31050", "Timer expired", c_err_msg);
					if (li_timeout >0)
					{
						fn_set_tmr_sigdisp ( TM_0, c_ServiceName, c_err_msg );
					}
					return -1;
				}
				else
				{
					i_read = 0;
        }
			}
			else 
			{
				fn_errlog(c_ServiceName, "L31055", UNXMSG, c_err_msg);
				if (li_timeout >0)
				{
					fn_set_tmr_sigdisp ( TM_0, c_ServiceName, c_err_msg );
				}
			  return -1;	
			}
		}
		else if ( i_read == 0 )
		{
			fn_errlog(c_ServiceName,"L31060","Socket in other end closed",c_err_msg);
			if (li_timeout >0)
			{
				fn_set_tmr_sigdisp ( TM_0, c_ServiceName, c_err_msg );
			}
		  return -1;	
		}

		li_left = li_left - i_read;
		ptr_v_tmp_data += i_read; 
	}

	if (i_break_status_sck == EXIT) /*** User signal is caught  ***/
	{
		fn_errlog(  c_ServiceName, "L31065", "Signal caught", c_err_msg);
		if (li_timeout >0)
		{
			fn_set_tmr_sigdisp ( TM_0, c_ServiceName, c_err_msg );
		}
		return -1;
	}

	if (li_timeout >0)
	{
		fn_set_tmr_sigdisp ( TM_0, c_ServiceName, c_err_msg );
	}

  return 0;
}

/******************************************************************************/
/* This function writes li_len byets to the socket completely. it it is       */
/* interrupted by user signal, it logs an error and come out. otherwise it    */
/* completes its operation. on success, it returns 0. on failure it logs an   */
/* error and comes out. otherwise, it completes it's operation.               */
/*																																						*/
/*	INPUT PARAMETERS																													*/
/*		i_sck_id			socket to which writing has to be made.										*/
/*		ptr_v_data		pointer to data stored which has to be writen to socket   */
/*		li_len				length of the data that has to be written.								*/
/*		c_ServiceName	the service which requested the write operation.					*/
/*		c_err_msg			the error message that has to be displayed on screen.			*/
/*																																						*/
/******************************************************************************/
int fn_writen(int i_sck_id, 
						 void *ptr_v_data,
						 long int li_len,
						 char *c_ServiceName,
						 char *c_err_msg )
{
	long int li_left;
	long int li_written;
	char * ptr_v_tmp_data;

	li_left = li_len;
	li_written = 0;
	ptr_v_tmp_data = (char *)ptr_v_data;
	i_break_status_sck = DONT_EXIT;

	while ( (li_left >0) && (i_break_status_sck == DONT_EXIT) )
	{
		if ( (li_written = write ( i_sck_id,
                               ptr_v_tmp_data,
                               li_left         ) ) == -1 )
		{
			if (errno == EINTR)
			{
				li_written = 0;
			}
			else
			{
				fn_errlog(c_ServiceName, "L31070", UNXMSG, c_err_msg);
				return -1;
			} 
		}

		li_left = li_left - li_written;
		ptr_v_tmp_data += li_written; 
	}
			
	if (i_break_status_sck == EXIT)
	{
		fn_errlog(c_ServiceName, "L31075", " User Signal is caught", c_err_msg);
		return -1;
	}

	return 0; 
}

/******************************************************************************/
/*	This function writes a message header  and the corresponding message to   */
/* socket. first it writes header completely and checks for any message. if   */
/* message is there, it will write m3ssage omplet3ely. in between it will come*/
/* out only if the user signal is caught. 																		*/
/*																																						*/
/*	INPUT PARAMETERS																													*/
/*		i_sck_id				socket to which the data has to be written.							*/
/*		ptr_v_data		  pointer to the data which has to be written to socket.	*/
/*		st_msg_frs_hdr	header packet which has to be written into socket.			*/
/*		c_ServiceName		Service that requested this service.										*/
/*		c_err_msg				Error message that has to be displayed on the screen.		*/
/*																																						*/
/******************************************************************************/

int fn_write_msg_sck ( int i_sck_id,
											 void *ptr_v_data,
											 msg_frs_hdr st_msg_frs_hdr, 
											 char *c_ServiceName,
										   char *c_err_msg )
{
	if(st_msg_frs_hdr.li_msg_typ != 0)
	{
		if ( fn_writen ( i_sck_id,
									   &st_msg_frs_hdr,
									   sizeof(msg_frs_hdr),
									   c_ServiceName,
									   c_err_msg            ) == -1 )
		{
			return -1;
		}

		if (st_msg_frs_hdr.li_msg_len >0)
		{
			if (fn_writen(i_sck_id,
										ptr_v_data,
										st_msg_frs_hdr.li_msg_len,
										c_ServiceName,
										c_err_msg                  ) == -1 )
			{
				return -1;
			} 
		}
	}

	return 0;	
}

/******************************************************************************/
/*   This function reads a message header and the corresponding message from  */
/* socket within a specified time. first it sets the timer and reads the      */
/* header from socket. then it checks  whether there is any message to be     */
/* recieved. if any message is there, it reads whole message completely and   */
/* put into a given pinter. the complete operation can be interrupted either  */
/* by user signal or time out. there is no time limit if the li_timeout       */
/* parameter is specified as 0. on success, it returns 0. 										*/
/*																																						*/
/*	INPUT PARAMETERS																													*/
/*		i_sck_id			socket id from where data has to be read.									*/
/*		ptr_v_data		pointer to where the data read has to be stored.					*/
/*		st_msg_hdr		pointer to the heder message reading from the socket.			*/
/*		li_timeout		time out limit for the read operation.										*/
/*		c_ServiceName	The Service that requested the read operation.						*/
/*		c_err_msg			The error message that wants to be displayed.							*/
/*																																						*/
/******************************************************************************/
int fn_read_msg_sck(int i_sck_id,
										void *ptr_v_data,
									  msg_frs_hdr *st_msg_hdr,
										long int li_timeout,	
										char *c_ServiceName,
										char *c_err_msg )
{
	int i_ch_val;

	if(li_timeout > 0)
	{
		if( fn_set_tmr_sigdisp ( li_timeout,
													   c_ServiceName,
													   c_err_msg      ) == -1 )
		{
			return -1;
		}
	}

	i_ch_val = fn_readn ( i_sck_id,
						      			st_msg_hdr,
					     					sizeof(msg_frs_hdr),
							    			0,
							    			c_ServiceName,
							    			c_err_msg            );
	if ( i_ch_val == -1 )
	{
		if (li_timeout >0)
		{
			fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
		}
		return -1;
	}

	if (st_msg_hdr->li_msg_len >0)
	{
		i_ch_val = fn_readn ( i_sck_id,
								    			ptr_v_data,
								    			st_msg_hdr->li_msg_len,
								    			0,
								    			c_ServiceName,
								    			c_err_msg                );
		if ( i_ch_val == -1 )
		{
			if (li_timeout >0)
			{
				fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
			}
			return -1;
		}
	}

	if (li_timeout >0)
	{
		fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
	}

	return 0;
}

/******************************************************************************/
/* This function send one type of message and receives a known type of messge.*/
/* the message structures to be send and receive are given as parameters.     */
/*																																						*/
/*	INPUT PARAMETERS																													*/
/*		i_sck_id 			The socket with which read and write operations are made  */
/*		ptr_v_ip_data	pointer for the input data.																*/
/*		st_ip_hdr			input header.																							*/
/*		ptr_v_op_data	pointer for the output data.															*/
/*		st_op_hdr			output header.																					  */
/*		c_ServiceName	The service that has requested the service.								*/
/*		c_err_msg			The error messge that has to be displayed on the screen.  */
/*																																						*/
/******************************************************************************/
int fn_snd_rcv_msg_sck(int i_sck_id,
											 void *ptr_v_ip_data,
											 msg_frs_hdr st_ip_hdr,
											 void *ptr_v_op_data,
											 msg_frs_hdr st_op_hdr, 
                       char *c_ServiceName,
                       char *c_err_msg )
{
	int li_tmp_type;

	if ( fn_set_tmr_sigdisp ( TM_30,
												    c_ServiceName,
												    c_err_msg       ) == -1 )
	{
		return -1;
	}

	if (fn_write_msg_sck(i_sck_id,
									 ptr_v_ip_data,
									 st_ip_hdr,
								   c_ServiceName,
									 c_err_msg       ) == -1)
	{
		fn_set_tmr_sigdisp ( TM_0,
										     c_ServiceName,
										     c_err_msg       );
		return -1;
	}

	if (st_op_hdr.li_msg_typ != 0)
	{
		li_tmp_type = st_op_hdr.li_msg_typ;

		if (fn_readn(i_sck_id,
								 &st_op_hdr,
								 sizeof ( st_op_hdr ),
								 0,
								 c_ServiceName,
								 c_err_msg              ) == -1)
		{
			fn_set_tmr_sigdisp ( TM_0,
										     		c_ServiceName,
											     	c_err_msg       );
			return -1;
		}

		if (st_op_hdr.li_msg_typ == li_tmp_type)
		{
			if(st_op_hdr.li_msg_len >0)
			{
				if (fn_readn(i_sck_id,
										 ptr_v_op_data,
										 st_op_hdr.li_msg_len,
										 0,
										 c_ServiceName,
										 c_err_msg             ) == -1)
				{
					fn_set_tmr_sigdisp ( TM_0,
										     			c_ServiceName,
										     			c_err_msg       );
					return -1;
				}
			}
		} 
		else
		{
			fn_errlog(c_ServiceName, "L31080", "Message mismatch", c_err_msg);
			fn_set_tmr_sigdisp ( TM_0,
										     	c_ServiceName,
										     	c_err_msg       );
			return -1;
		}	
	}

	fn_set_tmr_sigdisp ( TM_0,
									     c_ServiceName,
									     c_err_msg       );
	return 0;
}

/******************************************************************************/
/*	This function is a signal handler for SIGALRM. This is executed when the  */
/* timer expires. It just sets i_sig_intpt_trp to 1 to indicate SIGALRM is    */
/* caught.																																		*/
/*																																						*/
/*	INPUT PARAMETER																														*/
/*		signo			s																															*/
/******************************************************************************/

void fn_alrm_intr(int signo)
{
	i_sig_intrpt_trp = 1;
  return;
} 

/******************************************************************************/
/*   This function sets the timer for specified time and also set the signal  */
/* disposition for SIGALRM. on success, it returns o. on failure, it logs an  */
/* error and returns -1.																											*/
/******************************************************************************/
int fn_set_tmr_sigdisp(long int li_time,
											 char *c_ServiceName,
											 char *c_err_msg)
{
	alarm(li_time);

	if (fn_set_sgn_hnd(SIGALRM, fn_alrm_intr, c_ServiceName, c_err_msg)== -1)
	{
		return -1;
	}

	return 0;
}

/******************************************************************************/
/* This function sets the break_status_sck variable to indicate that the user */
/* signal is caught.																													*/
/******************************************************************************/
void fn_set_break_sck()
{
	i_break_status_sck = EXIT;
}
 
