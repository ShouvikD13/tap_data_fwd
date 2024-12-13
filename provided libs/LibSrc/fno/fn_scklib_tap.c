/******************************************************************************/
/*  Program         : fn_scklib_tap.c                                         */
/*                                                                            */
/*  Description     : Library functions for Socket handling operations        */
/*                                                                            */
/*  Log             :                                                         */
/*                                                                            */
/*  Ver 1.0   15-Apr-2008  Initial Release (Shailesh Hinge)		                */
/*  Ver 1.1   20-Apr-2009  Set socket options(Shailesh Hinge)		              */
/*	Ver 1.2		31-aug-2009	 Broadcast libraries (Shailesh Hinge)								*/
/*	Ver 1.3		05-Jul-2016	 Debug level changes (Sachin Birje)  								*/
/******************************************************************************/
/****  C header   ********/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h> /* For non-blocking */
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <aio.h>
#include <netinet/tcp.h>

#include <fo_exch_comnN.h>
#include <fn_scklib_tap.h>
#include <fn_signal.h>
#include <fo.h>
#include <fn_log.h>
#include <fn_read_debug_lvl.h> /*** ver 1.3 ***/

/*Global variables*/

/*To trap timer signal interrupt*/
int i_sig_intrpt_trp = 0;

/*To trap user signal caught*/
int i_break_status_sck = DONT_EXIT;

long int li_utc_offset;
long int li_ten_yrs;

/************************************************************************************************/
/** Function to create a socket having protocol family PF_INET and socket type as SOCK_STREAM  **/
/**	Usage:																																										 **/
/**				int fn_socket(PF_INET,SOCK_STREAM,0)																								 **/
/**	Input Parameters :																																				 **/
/** None																																											 **/
/** Return Values:																																						 **/
/**	If > 0 returned then it will be socket identifier																					 **/
/**     -1  returned then error																																 **/
/************************************************************************************************/

int fn_socket()
{
  int i_tmp_sckid;
  i_tmp_sckid = socket(PF_INET, SOCK_STREAM, 0);
  if (i_tmp_sckid == -1)
  {
    return -1;
  }
  return i_tmp_sckid;
}

/***************************************************************************************************/
/** Function to create a UDP socket having protocol family PF_INET and socket type as SOCK_DGRAM	**/
/** Usage:                                  	                                                    **/
/**       int fn_socket(PF_INET,SOCK_DGRAM,0)   		                                              **/
/** Input Parameters :                              	                                            **/
/** None                                                	                                        **/
/** Return Values:                                         	                                      **/
/** If > 0 returned then it will be socket identifier         		                                **/
/**     -1  returned then error                                   	                              **/
/***************************************************************************************************/

int fn_udp_socket() /* Ver 1.2 */
{

  int i_ch_val;

  i_ch_val = socket(PF_INET, SOCK_DGRAM, 0);

  if (i_ch_val == -1)
  {
    return (-1);
  }

  return (i_ch_val);
}

/************************************************************************************************/
/** Function to create the client socket and connect it to the known port of the TAP machine.	 **/
/** Before connecting to the machine, the internet socket address structure is to be filled 	 **/
/**	with server machine's I/P address and port number.																				 **/
/** Usage:																																										 **/
/**				fn_crt_clnt_sck(char *ptr_c_ip_addr,int i_port_no,int *ptr_i_sck_id,								 **/
/**										  	char *c_ServiceName, char *c_err_msg);															 **/
/**	Input Parameters:																																					 **/
/**		ptr_c_ip_addr	-	The ip address of the destination machine 															 **/
/**		i_port_no			-	Known port of the destination machine.																	 **/
/**		ptr_i_sck_id	-	Pointer to the id of the socket created.																 **/
/**		c_Servicename	- Name of the service which called this functional												 **/
/**		c_err_msg			-	The error message which is to be displayed.															 **/
/**																																														 **/
/**	Output Parameters:																																				 **/
/**		0	-	Success																																							 **/
/**	 -1	- 	Failure.																																				 	 **/
/************************************************************************************************/

int fn_crt_clnt_sck(char *ptr_c_ip_addr, int i_port_no, int *ptr_i_sck_id,
                    char *c_ServiceName, char *c_err_msg)
{
  int i_tmp_sck_id;
  struct sockaddr_in sck_in;
  int i_setval;
  socklen_t ud_optlen;

  if ((i_tmp_sck_id = fn_socket()) == -1)
  {
    fn_errlog(c_ServiceName, "L31005", UNXMSG, c_err_msg);
    return -1;
  }

  i_setval = SET_TCP_NODELAY;
  ud_optlen = (socklen_t)sizeof(int);

  if (setsockopt(i_tmp_sck_id, IPPROTO_TCP, TCP_NODELAY, &i_setval, ud_optlen) == -1)
  {
    fn_errlog(c_ServiceName, "L31010", UNXMSG, c_err_msg);
    fn_close_sck(i_tmp_sck_id, c_ServiceName, c_err_msg);
    return -1;
  }

  /**Ver 1.1, added to avoid delayed acknowledgements by TCP **/
  i_setval = SET_TCP_NODELAY_ACK;
  ud_optlen = (socklen_t)sizeof(int);

  /* VER TOL : TUX on LINUX -- Changed TCP_NODELAYACK to TCP_NODELAY */
  if (setsockopt(i_tmp_sck_id, IPPROTO_TCP, TCP_NODELAY, &i_setval, ud_optlen) == -1)
  {
    fn_errlog(c_ServiceName, "S31225", UNXMSG, c_err_msg);
    fn_close_sck(i_tmp_sck_id, c_ServiceName, c_err_msg);
    return -1;
  }

  i_setval = 20;
  if (setsockopt(i_tmp_sck_id, IPPROTO_TCP, TCP_KEEPIDLE, &i_setval, ud_optlen) == -1)
  {
    fn_errlog(c_ServiceName, "S31230", UNXMSG, c_err_msg);
    return -1;
  }

  i_setval = 20;
  if (setsockopt(i_tmp_sck_id, IPPROTO_TCP, TCP_KEEPINTVL, &i_setval, ud_optlen) == -1)
  {
    fn_errlog(c_ServiceName, "S31235", UNXMSG, c_err_msg);
    return -1;
  }

  i_setval = 600;
  if (setsockopt(i_tmp_sck_id, IPPROTO_TCP, TCP_KEEPCNT, &i_setval, ud_optlen) == -1)
  {
    fn_errlog(c_ServiceName, "S31240", UNXMSG, c_err_msg);
    return -1;
  }

  i_setval = SET_KEEPALIVE;
  ud_optlen = (socklen_t)sizeof(int);

  if (setsockopt(i_tmp_sck_id, SOL_SOCKET, SO_KEEPALIVE, &i_setval, ud_optlen) == -1)
  {
    fn_errlog(c_ServiceName, "L31015", UNXMSG, c_err_msg);
    fn_close_sck(i_tmp_sck_id, c_ServiceName, c_err_msg);
    return -1;
  }

  bzero(&sck_in, sizeof(struct sockaddr_in));
  sck_in.sin_family = AF_INET;
  sck_in.sin_port = htons(i_port_no);
  sck_in.sin_addr.s_addr = inet_addr(ptr_c_ip_addr);

  if (connect(i_tmp_sck_id,
              (struct sockaddr *)&sck_in,
              sizeof(struct sockaddr_in)) == -1)
  {
    fn_errlog(c_ServiceName, "L31020", UNXMSG, c_err_msg);
    fn_close_sck(i_tmp_sck_id, c_ServiceName, c_err_msg);
    return -1;
  }

  *ptr_i_sck_id = i_tmp_sck_id;
  return 0;
}

/******************************************************************************/
/*  This function is to close the socket. on success, it returns 0 and on     */
/* failure, it logs the error and returns -1                                  */
/*                                                                            */
/*  INPUT PARAMETERS                                                          */
/*      i_sck_id      The socket which has to be closed.                      */
/*      c_ServiceName The Service that wants to close the socket.             */
/*      c_err_msg     The error message that wants to be dispalyed on screen. */
/******************************************************************************/
int fn_close_sck(int i_sck_id,
                 char *c_ServiceName,
                 char *c_err_msg)
{
  if (close(i_sck_id) == -1)
  {
    fn_errlog(c_ServiceName, "L31025", UNXMSG, c_err_msg);
    return -1;
  }

  return 0;
}
/******************************************************************************/
/* This function reads exchange interactive message header and corresponding  */
/* message from socket based on the length in the header within a specified   */
/* time. First it sets the timer and then reads the header from socket. Then  */
/* it reads whole message completely and returns it. There is no time limit if*/
/* the timeout parameter is specified as 0                                    */
/*                                                                            */
/*  INPUT PARAMETERS                                                          */
/*    i_sck_id      - Socket id from where data has to be read                */
/*    ptr_v_data    - Pointer to where the data read has to be stored         */
/*    li_timeout    - Time out limit for the read operation                   */
/*    c_ServiceName - The Service Name                                        */
/*    c_err_msg     - The error message to be displayed.                      */
/*  OUTPUT PARAMETERS                                                         */
/*        0 - Success                                                         */
/*       -1 - Failure                                                         */
/******************************************************************************/
int fn_read_xchng_sck(int i_sck_id,
                      void *ptr_v_data,
                      long int li_timeout,
                      char *c_ServiceName,
                      char *c_err_msg)
{
  int i_ch_val;
  long int li_read_len;
  char c_buffer[MAX_SCK_MSG];
  struct st_net_hdr st_msg_hdr;
  FILE *i_fd;

  /*Set the timer if the timeout value is specified*/
  if (li_timeout > 0)
  {
    if (fn_set_tmr_sigdisp(li_timeout, c_ServiceName, c_err_msg) == -1)
    {
      return -1;
    }
  }

  memset(&st_msg_hdr, 0, sizeof(st_msg_hdr));

  /*First read the header*/
  i_ch_val = fn_readn(i_sck_id, &st_msg_hdr, sizeof(st_msg_hdr), 0, c_ServiceName, c_err_msg);
  if (i_ch_val == -1)
  {
    if (li_timeout > 0)
    {
      fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
    }
    return -1;
  }

  /*Copy the header first into the return buffer*/
  memcpy(ptr_v_data, &st_msg_hdr, sizeof(st_msg_hdr));
  memset(c_buffer, 0, MAX_SCK_MSG);
  if (DEBUG_MSG_LVL_3) /** ver 1.3 **/
  {
    fn_userlog(c_ServiceName, "taplib:After reading the header");
  }

  /*Read the remainder packet*/
  li_read_len = st_msg_hdr.si_message_length - sizeof(st_msg_hdr);

  /*If the message length is greater than zero, read the message*/
  if (li_read_len > 0)
  {
    if (li_read_len > (MAX_SCK_MSG - sizeof(st_msg_hdr)))
    {
      li_read_len = MAX_SCK_MSG - sizeof(st_msg_hdr);
    }

    i_ch_val = fn_readn(i_sck_id, c_buffer, li_read_len, 0, c_ServiceName, c_err_msg);
    if (i_ch_val == -1)
    {
      if (li_timeout > 0)
      {
        fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
      }
      return -1;
    }

    /*Copy the message data into the return buffer*/
    memcpy((char *)ptr_v_data + sizeof(st_msg_hdr), c_buffer, li_read_len);
  }

  if (li_timeout > 0)
  {
    fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
  }

  return 0;
}

/******************************************************************************/
/* This function sets the timer for specified time and also sets the signal   */
/* handler for SIGALRM.                                                       */
/*  INPUT PARAMETERS                                                          */
/*    li_time       - Timer value                                             */
/*    c_ServiceName - The Service Name                                        */
/*    c_err_msg     - The error messge to be displayed on the screen          */
/*  OUTPUT PARAMETERS                                                         */
/*        0 - Success                                                         */
/*       -1 - Failure                                                         */
/******************************************************************************/
int fn_set_tmr_sigdisp(long int li_time,
                       char *c_ServiceName,
                       char *c_err_msg)
{
  /*Set the Alarm*/
  alarm(li_time);

  /*Set the interrupt handler*/
  if (fn_set_sgn_hnd(SIGALRM, fn_alrm_intr, c_ServiceName, c_err_msg) == -1)
  {
    fn_errlog(c_ServiceName, "L31030", LIBMSG, c_err_msg);
    return -1;
  }

  return 0;
}

/******************************************************************************/
/* This function is a signal handler for SIGALRM. This is executed when the   */
/* timer expires. It just sets i_sig_intpt_trp to 1 to indicate SIGALRM is    */
/* caught.                                                                    */
/*  INPUT PARAMETER                                                           */
/*    signo - Signal No                                                       */
/*  OUTPUT PARAMETER                                                          */
/*    None                                                                    */
/******************************************************************************/
void fn_alrm_intr(int signo)
{
  i_sig_intrpt_trp = 1;
  return;
}

/******************************************************************************/
/* This function reads N bytes from socket within a specified time. First it  */
/* sets the timer for specified time(li_timeout). There is no time limit if   */
/* the timeout parameter is specified as 0. It reads the socket until it has  */
/* read li_len bytes. In between, if it is interrupted by user signal or time */
/* out signal, it will come out of the loop. Otherwise it continues in its    */
/* loop.                                                                      */
/*  INPUT PARAMETERS                                                          */
/*    i_sck_id     - Id of the socket for reading.                            */
/*    ptr_v_data   - Variable to store the read data.                         */
/*    li_len       - Length of the data to be read.                           */
/*    li_timeout   - The timeout limit for the operation.                     */
/*    c_ServiceName- The service that requested the read operation.           */
/*    c_err_msg    - The error message that has to be displayed on screen.    */
/*  OUTPUT PARAMETERS                                                         */
/*        0 - Success                                                         */
/*       -1 - Failure                                                         */
/******************************************************************************/
int fn_readn(int i_sck_id,
             void *ptr_v_data,
             long int li_len,
             long int li_timeout,
             char *c_ServiceName,
             char *c_err_msg)
{
  long int li_left;
  int i_read;
  char *ptr_v_tmp_data;

  i_break_status_sck = DONT_EXIT;

  li_left = li_len;
  i_read = 0;
  ptr_v_tmp_data = (char *)ptr_v_data;

  /*Set the timer*/
  if (li_timeout > 0)
  {
    /*Set the timer alarm to the timeout value given*/
    if (fn_set_tmr_sigdisp(li_timeout, c_ServiceName, c_err_msg) == -1)
    {
      fn_errlog(c_ServiceName, "L31035", LIBMSG, c_err_msg);
      return -1;
    }
  }

  /*While more data is to be read and there is no interrupt*/
  while ((li_left > 0) && (i_break_status_sck == DONT_EXIT))
  {
    if ((i_read = read(i_sck_id, ptr_v_tmp_data, li_left)) == -1)
    {
      fn_userlog(c_ServiceName, "i_read = :%d:, li_left = :%ld:", i_read, li_left);
      if (errno == EINTR)
      {
        if (i_sig_intrpt_trp == 1) /*** Timer is over ***/
        {
          i_sig_intrpt_trp = 0;
          fn_errlog(c_ServiceName, "L31040", "Timer expired", c_err_msg);
          if (li_timeout > 0)
          {
            fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
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
        fn_errlog(c_ServiceName, "L31045", UNXMSG, c_err_msg);
        if (li_timeout > 0)
        {
          fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
        }
        return -1;
      }
    }
    else if (i_read == 0)
    {
      fn_errlog(c_ServiceName, "L31050", "Socket in other end closed", c_err_msg);
      if (li_timeout > 0)
      {
        fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
      }
      return -1;
    }

    if (DEBUG_MSG_LVL_3) /** ver 1.3 **/
    {
      fn_userlog(c_ServiceName, "SHAILEH:bytes read from socket = :%d:", i_read);
    }

    li_left = li_left - i_read;
    ptr_v_tmp_data += i_read;
  }
  if (i_break_status_sck == EXIT) /*User signal is caught*/
  {
    fn_errlog(c_ServiceName, "L31055", "Signal caught", c_err_msg);
    if (li_timeout > 0)
    {
      fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
    }
    return -1;
  }

  /*Unset the timer*/
  if (li_timeout > 0)
  {
    fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
  }

  return 0;
}

/******************************************************************************/
/* This function writes the message header and the corresponding message to   */
/* the socket. First it writes the header completely and follow it up with the*/
/* message. In between it will come out only if a user signal is caught       */
/*  INPUT PARAMETERS                                                          */
/*    i_sck_id       - Socket to which the data has to be written.            */
/*    ptr_v_data     - Pointer to the data which has to be written to socket. */
/*    st_msg_frs_hdr - Header packet which has to be written into socket.     */
/*    c_ServiceName  - Service Name                                           */
/*    c_err_msg      - Error message that has to be displayed on the screen.  */
/*  OUTPUT PARAMETERS                                                         */
/*        0 - Success                                                         */
/*       -1 - Failure                                                         */
/******************************************************************************/
int fn_write_msg_sck(int i_sck_id,
                     void *ptr_v_data,
                     struct st_int_header st_msg_frs_hdr,
                     char *c_ServiceName,
                     char *c_err_msg)
{
  if (st_msg_frs_hdr.si_transaction_code != 0)
  {
    /*Write the header*/
    if (fn_writen(i_sck_id, &st_msg_frs_hdr, sizeof(struct st_int_header), c_ServiceName,
                  c_err_msg) == -1)
    {
      fn_errlog(c_ServiceName, "L31060", LIBMSG, c_err_msg);
      return -1;
    }

    /*Write the message*/
    if (st_msg_frs_hdr.si_message_length > 0)
    {
      if (fn_writen(i_sck_id, ptr_v_data, st_msg_frs_hdr.si_message_length, c_ServiceName,
                    c_err_msg) == -1)
      {
        fn_errlog(c_ServiceName, "L31065", LIBMSG, c_err_msg);
        return -1;
      }
    }
  }

  return 0;
}

/******************************************************************************/
/* This function writes li_len byets to the socket completely. If it is       */
/* interrupted by user signal, it logs an error and comes out. Otherwise it   */
/* completes its operation                                                    */
/*  INPUT PARAMETERS                                                          */
/*    i_sck_id      - Socket to which writing has to be made                  */
/*    ptr_v_data    - Pointer to data which has to be writen to the socket    */
/*    li_len        - Length of the data that has to be written               */
/*    c_ServiceName - The Service Name                                        */
/*    c_err_msg     - The error message that has to be displayed on screen    */
/*  OUTPUT PARAMETERS                                                         */
/*        0 - Success                                                         */
/*       -1 - Failure                                                         */
/******************************************************************************/
int fn_writen(int i_sck_id,
              void *ptr_v_data,
              long int li_len,
              char *c_ServiceName,
              char *c_err_msg)
{
  long int li_left;
  long int li_written;
  char *ptr_v_tmp_data;

  li_left = li_len;
  li_written = 0;
  ptr_v_tmp_data = (char *)ptr_v_data;
  i_break_status_sck = DONT_EXIT;

  /*While there is more data to write and the break condition is not encountered*/
  while ((li_left > 0) && (i_break_status_sck == DONT_EXIT))
  {
    if ((li_written = write(i_sck_id, ptr_v_tmp_data, li_left)) == -1)
    {
      if (errno == EINTR)
      {
        li_written = 0;
        fn_userlog(c_ServiceName, "Process has been interrupted");
      }
      else
      {
        fn_errlog(c_ServiceName, "L31070", UNXMSG, c_err_msg);
        fn_userlog(c_ServiceName, " fn_writen errno is [%ld]", errno);
        fn_userlog(c_ServiceName, " fn_writen socket id is [%d]", i_sck_id);
        fn_userlog(c_ServiceName, " fn_writen li_left is [%ld]", li_left);
        return -1;
      }
    }

    fn_userlog(c_ServiceName, "Written :%ld: bytes on socket", li_written);

    li_left = li_left - li_written;
    ptr_v_tmp_data += li_written;
  }

  if (i_break_status_sck == EXIT)
  {
    fn_errlog(c_ServiceName, "L31075", "User Signal is caught", c_err_msg);
    return -1;
  }

  return 0;
}

/*inline*/ void fn_orstonse_char(char *ptr_c_dest, long int li_len_dest, const char *ptr_c_src,  long int li_len_src)
{
  long int li_cnt;

  for (li_cnt = 0; li_cnt < li_len_src; li_cnt++)
  {
    *(ptr_c_dest + li_cnt) = toupper(*(ptr_c_src + li_cnt));
  }
  memset((ptr_c_dest + li_len_src), ' ', (li_len_dest - li_len_src));
}

/*inline*/ void fn_nsetoors_char(char *ptr_c_dest,
                                 long int li_len_dest,
                                 char *ptr_c_src,
                                 long int li_len_src)
{
  long int li_cnt;
  int i_tmp;

  for (i_tmp = li_len_src - 1; *(ptr_c_src + i_tmp) == ' '; i_tmp--)
    ;
  i_tmp++;

  for (li_cnt = 0; ((li_cnt < i_tmp) && (li_cnt < li_len_dest)); li_cnt++)
  {
    *(ptr_c_dest + li_cnt) = *(ptr_c_src + li_cnt);
  }

  *(ptr_c_dest + li_cnt) = '\0';
}

/*inline*/ void fn_orstonse_tm(long int *ptr_li_dest,
                               char *c_tm_stmp)
{
  struct tm temp_tm;
  char c_tmp;
  int i;
  int j;

  char c_day[3];
  char c_mon[4];
  char c_yr[5];
  char c_hr[3];
  char c_min[3];
  char c_sec[3];

  for (j = 0, i = 0; i < 2; i++, j++)
  {
    *(c_day + j) = *(c_tm_stmp + i);
  }
  c_day[2] = '\0';

  for (j = 0, i = 3; i < 6; i++, j++)
  {
    *(c_mon + j) = *(c_tm_stmp + i);
  }
  c_mon[3] = '\0';

  for (j = 0, i = 7; i < 11; i++, j++)
  {
    *(c_yr + j) = *(c_tm_stmp + i);
  }
  c_yr[4] = '\0';

  for (j = 0, i = 12; i < 14; i++, j++)
  {
    *(c_hr + j) = *(c_tm_stmp + i);
  }
  c_hr[2] = '\0';

  for (j = 0, i = 15; i < 17; i++, j++)
  {
    *(c_min + j) = *(c_tm_stmp + i);
  }
  c_min[2] = '\0';

  for (j = 0, i = 18; i < 20; i++, j++)
  {
    *(c_sec + j) = *(c_tm_stmp + i);
  }
  c_sec[2] = '\0';

  temp_tm.tm_mday = atoi(c_day);
  switch (toupper(c_mon[0]))
  {
  case 'J':
    if (strcasecmp(c_mon, "JAN") == 0)
    {
      temp_tm.tm_mon = 0;
    }
    else if (strcasecmp(c_mon, "JUN") == 0)
    {
      temp_tm.tm_mon = 5;
    }
    else if (strcasecmp(c_mon, "JUL") == 0)
    {
      temp_tm.tm_mon = 6;
    }
    break;

  case 'A':
    if (strcasecmp(c_mon, "APR") == 0)
    {
      temp_tm.tm_mon = 3;
    }
    else if (strcasecmp(c_mon, "AUG") == 0)
    {
      temp_tm.tm_mon = 7;
    }
    break;

  case 'M':

    if (strcasecmp(c_mon, "MAR") == 0)
    {
      temp_tm.tm_mon = 2;
    }
    else if (strcasecmp(c_mon, "MAY") == 0)
    {
      temp_tm.tm_mon = 4;
    }
    break;

  case 'F':
    temp_tm.tm_mon = 1;
    break;

  case 'S':
    temp_tm.tm_mon = 8;
    break;

  case 'O':
    temp_tm.tm_mon = 9;
    break;

  case 'N':
    temp_tm.tm_mon = 10;
    break;

  case 'D':
    temp_tm.tm_mon = 11;
    break;
  }
  temp_tm.tm_year = atoi(c_yr) - 1900;
  temp_tm.tm_hour = atoi(c_hr);
  temp_tm.tm_min = atoi(c_min);
  temp_tm.tm_sec = atoi(c_sec);
  temp_tm.tm_wday = 0;
  temp_tm.tm_yday = 0;
  temp_tm.tm_isdst = 0;

  *ptr_li_dest = mktime(&temp_tm) + li_utc_offset - li_ten_yrs;
}

/*inline*/ void fn_nsetoors_tm(char *c_tm_stmp,
                               long int li_src)
{
  char *c_tm;
  int i;
  int j;
  time_t now;

  li_src += li_ten_yrs;
  strftime(c_tm_stmp, 21, "%d-%b-%Y %H:%M:%S", gmtime(&li_src));
}

void fn_sck_init(void)
{
  struct tm tenyrs_tm;

  tenyrs_tm.tm_sec = 0;
  tenyrs_tm.tm_min = 0;
  tenyrs_tm.tm_hour = 0;
  tenyrs_tm.tm_mday = 1;
  tenyrs_tm.tm_mon = 0;
  tenyrs_tm.tm_year = 70;
  tenyrs_tm.tm_wday = 0;
  tenyrs_tm.tm_yday = 0;
  tenyrs_tm.tm_isdst = 0;

  li_utc_offset = mktime(&tenyrs_tm) * (-1);

  tenyrs_tm.tm_sec = 0;
  tenyrs_tm.tm_min = 0;
  tenyrs_tm.tm_hour = 0;
  tenyrs_tm.tm_mday = 1;
  tenyrs_tm.tm_mon = 0;
  tenyrs_tm.tm_year = 80;
  tenyrs_tm.tm_wday = 0;
  tenyrs_tm.tm_yday = 0;
  tenyrs_tm.tm_isdst = 0;

  li_ten_yrs = mktime(&tenyrs_tm) + li_utc_offset;
}

/******************************************************************************/
/* This function sets the break_status_sck variable to indicate that the user */
/* signal is caught.                                                          */
/******************************************************************************/
void fn_set_break_sck()
{
  i_break_status_sck = EXIT;
}

/** Added in Ver 1.2 for ORS2 Broadcast STARTS **/
/******************************************************************************/
/* This function creates a client socket, subscribes to the multicast group   */
/* specified as the destination address with the host address and binds it to */
/* the port no given as input.                                                */
/*  INPUT PARAMETERS                                                          */
/*    c_hst_addrss    - Host Address                                          */
/*    i_port_no       - Port No                                               */
/*    c_dest_adrss    - Destination Address                                   */
/*    ptr_i_sck_id    - Socket Identifier to be returned                      */
/*    c_ServiceName   - The Service Name                                      */
/*    c_err_msg       - The error messge to be displayed on the screen        */
/*  OUTPUT PARAMETERS                                                         */
/*        0 - Success                                                         */
/*       -1 - Failure                                                         */
/******************************************************************************/
int fn_crt_mcast_sck(char *c_hst_addrss,
                     int i_port_no,
                     char *c_dest_addrss,
                     int *ptr_i_sck_id,
                     char *c_ServiceName,
                     char *c_err_msg)
{
  int i_tmp_sckid;
  struct ip_mreq st_mreq;
  struct sockaddr_in sck_in;

  /*Create a UDP socket*/
  i_tmp_sckid = socket(AF_INET, SOCK_DGRAM, 0);
  if (i_tmp_sckid == -1)
  {
    fn_errlog(c_ServiceName, "L31080", UNXMSG, c_err_msg);
    return -1;
  }

  bzero(&sck_in, sizeof(struct sockaddr_in));

  /*Bind the socket to the known IP Address and Port No*/
  sck_in.sin_family = AF_INET;
  sck_in.sin_port = htons(i_port_no);

  /*sck_in.sin_addr.s_addr = inet_addr(c_hst_addrss);*/
  sck_in.sin_addr.s_addr = INADDR_ANY;

  if (bind(i_tmp_sckid, (struct sockaddr *)&sck_in, sizeof(struct sockaddr_in)) == -1)
  {
    fn_errlog(c_ServiceName, "L31085", UNXMSG, c_err_msg);
    return -1;
  }

  /*Set the multicast group address as the destination address*/
  st_mreq.imr_multiaddr.s_addr = inet_addr(c_dest_addrss);
  /*Set the host address*/
  st_mreq.imr_interface.s_addr = inet_addr(c_hst_addrss);

  /*Subscribe to multicast*/
  if (setsockopt(i_tmp_sckid, IPPROTO_IP, IP_ADD_MEMBERSHIP, &st_mreq, sizeof(st_mreq)) == -1)
  {
    fn_errlog(c_ServiceName, "L31090", UNXMSG, c_err_msg);
    return -1;
  }

  *ptr_i_sck_id = i_tmp_sckid;
  return 0;
}

/******************************************************************************/
/* This function requests the multicast group identified with the destination */
/* address to remove the host address from its multicast users list and then  */
/* closes the client socket.                                                  */
/*  INPUT PARAMETERS                                                          */
/*    i_sck_id        - Socket Identifier                                     */
/*    c_hst_addrss    - Host Address                                          */
/*    c_dest_adrss    - Destination Address                                   */
/*    c_ServiceName   - The Service Name                                      */
/*    c_err_msg       - The error messge to be displayed on the screen        */
/*  OUTPUT PARAMETERS                                                         */
/*        0 - Success                                                         */
/*       -1 - Failure                                                         */
/******************************************************************************/
int fn_close_mcast_sck(int i_sck_id,
                       char *c_hst_addrss,
                       char *c_dest_addrss,
                       char *c_ServiceName,
                       char *c_err_msg)
{
  struct ip_mreq st_mreq;

  /*Set the multicast group address as the destination address*/
  st_mreq.imr_multiaddr.s_addr = inet_addr(c_dest_addrss);
  /*Set the host address*/
  st_mreq.imr_interface.s_addr = inet_addr(c_hst_addrss);

  if (setsockopt(i_sck_id, IPPROTO_IP, IP_DROP_MEMBERSHIP, &st_mreq, sizeof(st_mreq)) == -1)
  {
    fn_errlog(c_ServiceName, "L31095", UNXMSG, c_err_msg);
    return -1;
  }

  if (fn_close_sck(i_sck_id, c_ServiceName, c_err_msg) == -1)
  {
    fn_errlog(c_ServiceName, "L31100", LIBMSG, c_err_msg);
    return -1;
  }
  return 0;
}

/******************************************************************************/
/* This function reads UDP packets from a socket within a specified time. It  */
/* first sets the timer for the input time(li_timeout). There is no time limit*/
/* if the timeout parameter is given  as 0. In between, if it is interrupted  */
/* by a user signal or timeout signal, it will come out of the loop. Otherwise*/
/* it continues in its loop.                                                  */
/*                                                                            */
/*  INPUT PARAMETERS                                                          */
/*    i_sck_id     - Id of the socket for reading.                            */
/*    ptr_v_data   - Variable to store the read data.                         */
/*    li_len       - Length of the data to be read.                           */
/*    li_timeout   - The timeout limit for the operation.                     */
/*    c_ServiceName- The service that requested the read operation.           */
/*    c_err_msg    - The error message that has to be displayed on screen.    */
/*  OUTPUT PARAMETERS                                                         */
/*        0 - Success                                                         */
/*       -1 - Failure                                                         */
/******************************************************************************/
int fn_recv_udp_msg(int i_sck_id,
                    void *ptr_v_data,
                    long int li_len,
                    long int li_timeout,
                    char *c_ServiceName,
                    char *c_err_msg)
{
  int i_read;
  socklen_t li_addrss_len; /* Added in Ver TOL : TUX on LINUX */
  /* COMMENETED IN VER TOL * unsigned long li_addrss_len; */
  char *ptr_v_tmp_data;
  struct sockaddr st_frm_addrss;

  i_break_status_sck = DONT_EXIT;

  i_read = 0;
  ptr_v_tmp_data = (char *)ptr_v_data;
  if (li_timeout > 0)
  {
    /*Set the timer alarm to the timeout value given*/
    if (fn_set_tmr_sigdisp(li_timeout, c_ServiceName, c_err_msg) == -1)
    {
      fn_errlog(c_ServiceName, "L31105", LIBMSG, c_err_msg);
      return -1;
    }
  }

  /*Read the UDP message*/
  li_addrss_len = (socklen_t)sizeof(st_frm_addrss);
  if ((i_read = recvfrom(i_sck_id, ptr_v_tmp_data, li_len, 0, (struct sockaddr *)&st_frm_addrss,
                         &li_addrss_len)) == -1)
  {
    if (errno == EINTR)
    {
      if (i_sig_intrpt_trp == 1) /*** Timer is over ***/
      {
        i_sig_intrpt_trp = 0;
        fn_errlog(c_ServiceName, "L31110", "Timer expired", c_err_msg);
      }
      else if (i_break_status_sck == EXIT)
      {
        fn_errlog(c_ServiceName, "L31115", "Signal caught", c_err_msg);
      }
      else
      {
        fn_errlog(c_ServiceName, "L31120", UNXMSG, c_err_msg);
      }

      if (li_timeout > 0)
      {
        fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
      }
      return -1;
    }
    else
    {
      fn_errlog(c_ServiceName, "L31125", UNXMSG, c_err_msg);
      if (li_timeout > 0)
      {
        fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
      }
      return -1;
    }
  }
  else if (i_read == 0)
  {
    fn_errlog(c_ServiceName, "L31130", "Socket in other end closed", c_err_msg);
    if (li_timeout > 0)
    {
      fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
    }
    return -1;
  }

  /*Unset the timer*/
  if (li_timeout > 0)
  {
    fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
  }

  return 0;
}

/******************************************************************************/
/* This function is used to twiddle the integer bytes receieved from exchange */
/* subnet 																																		*/
/*                                                                            */
/*  INPUT PARAMETERS                                                          */
/*       i_input - Unsigned Integer data input																*/
/*  OUTPUT PARAMETERS                                                         */
/*			Twiddeled Unsigned Integer data																				*/
/******************************************************************************/
unsigned short fn_twiddle(unsigned short int i_input)
{
  unsigned short int i, j, n, x, i_output;

  i = 0;
  j = 8;
  n = 8;

  /*binary(i_input,sizeof(i_input)); */

  x = ((i_input >> i) ^ (i_input >> j)) & ((1 << n) - 1);

  /*binary(x,sizeof(x)); */

  i_output = i_input ^ ((x << i) | (x << j));

  /*binary(r,sizeof(i_output)); */

  return i_output;
}

void binary(int num, int size)
{
  int andmask, k, j, i;
  i = (8 * size) - 1;
  for (; i >= 0; i--)
  {
    k = 1 << i;

    j = num & k;

    if (j == 0)
    {
      printf("%d", 0);
    }
    else
    {
      printf("%d", 1);
    }
  }
}

/*** COMMENTED IN VER TOL : TUX on LINUX -- Dfinition moved to eba_common.c (Ravindra) ***
void fn_timearr_to_long(char *c_tm_stmp,
                        long int *ptr_li_dest)
{
  int  i_srcindex;
  int  i_arrayindex;
  char c_tmp;
  char c_day[3];
  char c_mon[4];
  char c_yr[5];
  char c_hr[3];
  char c_min[3];
  char c_sec[3];

  struct tm temp_tm;

  if(strcmp(c_tm_stmp, "0") == 0)
  {
    *ptr_li_dest = 0;
    return;
  }

  for(i_arrayindex = 0, i_srcindex = 0; i_srcindex < 2; i_srcindex++, i_arrayindex++)
  {
    *(c_day + i_arrayindex) = *(c_tm_stmp + i_srcindex);
  }
  c_day[2] = '\0';

  for(i_arrayindex = 0, i_srcindex = 3; i_srcindex < 6; i_srcindex++, i_arrayindex++)
  {
    *(c_mon + i_arrayindex) = *(c_tm_stmp + i_srcindex);
  }
  c_mon[3] = '\0';

  for(i_arrayindex = 0, i_srcindex = 7; i_srcindex < 11; i_srcindex++, i_arrayindex++)
  {
    *(c_yr + i_arrayindex) = *(c_tm_stmp + i_srcindex);
  }
  c_yr[4] = '\0';

  for(i_arrayindex = 0, i_srcindex = 12; i_srcindex < 14; i_srcindex++, i_arrayindex++)
  {
    *(c_hr + i_arrayindex) = *(c_tm_stmp + i_srcindex);
  }
  c_hr[2] = '\0';
  for(i_arrayindex = 0, i_srcindex = 15; i_srcindex < 17; i_srcindex++, i_arrayindex++)
  {
    *(c_min + i_arrayindex) = *(c_tm_stmp + i_srcindex);
  }
  c_min[2] = '\0';

  for(i_arrayindex = 0, i_srcindex = 18; i_srcindex < 20; i_srcindex++, i_arrayindex++)
  {
    *(c_sec + i_arrayindex) = *(c_tm_stmp + i_srcindex);
  }
  c_sec[2] = '\0';

  *Get the month number for the month string name*
  temp_tm.tm_mday = atoi(c_day);
  switch(toupper(c_mon[0]))
  {
    case 'J' :
    {
      if(strcasecmp(c_mon, "JAN") == 0)
      {
        temp_tm.tm_mon = 0;
      }
      else if(strcasecmp(c_mon, "JUN") == 0)
      {
        temp_tm.tm_mon = 5;
      }
      else if(strcasecmp(c_mon, "JUL") == 0)
      {
        temp_tm.tm_mon = 6;
      }
      break;
    }
   case 'A' :
    {
      if(strcasecmp(c_mon, "APR") == 0)
      {
        temp_tm.tm_mon = 3;
      }
      else if(strcasecmp(c_mon, "AUG") == 0)
      {
        temp_tm.tm_mon = 7;
      }
      break;
    }
    case 'M' :
    {
      if(strcasecmp(c_mon, "MAR") == 0)
      {
        temp_tm.tm_mon = 2;
      }
      else if(strcasecmp(c_mon, "MAY") == 0)
      {
        temp_tm.tm_mon = 4;
      }
      break;
    }
    case 'F' :
    {
      temp_tm.tm_mon = 1;
      break;
    }
    case 'S' :
    {
      temp_tm.tm_mon = 8;
      break;
    }
    case 'O' :
    {
      temp_tm.tm_mon = 9;
      break;
    }
    case 'N' :
    {
      temp_tm.tm_mon = 10;
      break;
    }
    case 'D' :
    {
      temp_tm.tm_mon = 11;
      break;
    }
  }
  temp_tm.tm_year = atoi(c_yr) - 1900;
  temp_tm.tm_hour = atoi(c_hr);
  temp_tm.tm_min  = atoi(c_min);
  temp_tm.tm_sec  = atoi(c_sec);
  temp_tm.tm_wday = 0;
  temp_tm.tm_yday = 0;
  temp_tm.tm_isdst = 0;

  *ptr_li_dest = mktime(&temp_tm) - TEN_YRS_IN_SEC; *Exchange time is since 1980*

  return;
}
*** COMMENT OF VER TOL : TUX on LINUX ends here ****/

void fn_long_to_timearr(char *c_tm_stmp, long int li_src)
{
  li_src += TEN_YRS_IN_SEC;
  strftime(c_tm_stmp, 21, "%d-%b-%Y %H:%M:%S", localtime(&li_src));
}

/******************************************************************************/
/* This function send one type of message and receives a known type of messge.*/
/* the message structures to be send and receive are given as parameters.     */
/*                                                                            */
/*  INPUT PARAMETERS                                                          */
/*    i_sck_id      The socket with which read and write operations are made  */
/*    ptr_v_ip_data pointer for the input data.                               */
/*    st_ip_hdr     input header.                                             */
/*    ptr_v_op_data pointer for the output data.                              */
/*    st_op_hdr     output header.                                            */
/*    c_ServiceName The service that has requested the service.               */
/*    c_err_msg     The error messge that has to be displayed on the screen.  */
/*                                                                            */
/******************************************************************************/

int fn_snd_rcv_msg_sck(int i_sck_id,
                       void *ptr_v_ip_data,
                       msg_frs_hdr st_ip_hdr,
                       void *ptr_v_op_data,
                       msg_frs_hdr st_op_hdr,
                       char *c_ServiceName,
                       char *c_err_msg)
{
  long int li_tmp_type;

  if (fn_set_tmr_sigdisp(TM_30,
                         c_ServiceName,
                         c_err_msg) == -1)
  {
    return -1;
  }

  if (fn_write_sck(i_sck_id,
                   ptr_v_ip_data,
                   st_ip_hdr,
                   c_ServiceName,
                   c_err_msg) == -1)
  {
    fn_set_tmr_sigdisp(TM_0,
                       c_ServiceName,
                       c_err_msg);
    return -1;
  }

  if (st_op_hdr.li_msg_typ != 0)
  {
    li_tmp_type = st_op_hdr.li_msg_typ;

    if (fn_readn(i_sck_id,
                 &st_op_hdr,
                 sizeof(st_op_hdr),
                 0,
                 c_ServiceName,
                 c_err_msg) == -1)
    {
      fn_set_tmr_sigdisp(TM_0,
                         c_ServiceName,
                         c_err_msg);
      return -1;
    }

    if (st_op_hdr.li_msg_typ == li_tmp_type)
    {
      if (st_op_hdr.li_msg_len > 0)
      {
        if (fn_readn(i_sck_id,
                     ptr_v_op_data,
                     st_op_hdr.li_msg_len,
                     0,
                     c_ServiceName,
                     c_err_msg) == -1)
        {
          fn_set_tmr_sigdisp(TM_0,
                             c_ServiceName,
                             c_err_msg);
          return -1;
        }
      }
    }
    else
    {
      fn_errlog(c_ServiceName, "L31135", "Message mismatch", c_err_msg);
      fn_set_tmr_sigdisp(TM_0,
                         c_ServiceName,
                         c_err_msg);
      return -1;
    }
  }

  fn_set_tmr_sigdisp(TM_0,
                     c_ServiceName,
                     c_err_msg);
  return 0;
}

/******************************************************************************/
/*  This function writes a message header  and the corresponding message to   */
/* socket. first it writes header completely and checks for any message. if   */
/* message is there, it will write m3ssage omplet3ely. in between it will come*/
/* out only if the user signal is caught.                                     */
/*                                                                            */
/*  INPUT PARAMETERS                                                          */
/*    i_sck_id        socket to which the data has to be written.             */
/*    ptr_v_data      pointer to the data which has to be written to socket.  */
/*    st_msg_frs_hdr  header packet which has to be written into socket.      */
/*    c_ServiceName   Service that requested this service.                    */
/*    c_err_msg       Error message that has to be displayed on the screen.   */
/*                                                                            */
/******************************************************************************/
int fn_write_sck(int i_sck_id,
                 void *ptr_v_data,
                 msg_frs_hdr st_msg_frs_hdr,
                 char *c_ServiceName,
                 char *c_err_msg)
{
  if (st_msg_frs_hdr.li_msg_typ != 0)
  {
    if (fn_writen(i_sck_id,
                  &st_msg_frs_hdr,
                  sizeof(msg_frs_hdr),
                  c_ServiceName,
                  c_err_msg) == -1)
    {
      return -1;
    }

    if (st_msg_frs_hdr.li_msg_len > 0)
    {
      if (fn_writen(i_sck_id,
                    ptr_v_data,
                    st_msg_frs_hdr.li_msg_len,
                    c_ServiceName,
                    c_err_msg) == -1)
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
/* parameter is specified as 0. on success, it returns 0.                     */
/*                                                                            */
/*  INPUT PARAMETERS                                                          */
/*    i_sck_id      socket id from where data has to be read.                 */
/*    ptr_v_data    pointer to where the data read has to be stored.          */
/*    st_msg_hdr    pointer to the heder message reading from the socket.     */
/*    li_timeout    time out limit for the read operation.                    */
/*    c_ServiceName The Service that requested the read operation.            */
/*    c_err_msg     The error message that wants to be displayed.             */
/*                                                                            */
/******************************************************************************/
int fn_read_sck(int i_sck_id,
                void *ptr_v_data,
                msg_frs_hdr *st_msg_hdr,
                long int li_timeout,
                char *c_ServiceName,
                char *c_err_msg)
{
  int i_ch_val;

  if (li_timeout > 0)
  {
    if (fn_set_tmr_sigdisp(li_timeout,
                           c_ServiceName,
                           c_err_msg) == -1)
    {
      return -1;
    }
  }

  i_ch_val = fn_readn(i_sck_id,
                      st_msg_hdr,
                      sizeof(msg_frs_hdr),
                      0,
                      c_ServiceName,
                      c_err_msg);
  if (i_ch_val == -1)
  {
    if (li_timeout > 0)
    {
      fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
    }
    return -1;
  }
  if (st_msg_hdr->li_msg_len > 0)
  {
    i_ch_val = fn_readn(i_sck_id,
                        ptr_v_data,
                        st_msg_hdr->li_msg_len,
                        0,
                        c_ServiceName,
                        c_err_msg);
    if (i_ch_val == -1)
    {
      if (li_timeout > 0)
      {
        fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
      }
      return -1;
    }
  }

  if (li_timeout > 0)
  {
    fn_set_tmr_sigdisp(TM_0, c_ServiceName, c_err_msg);
  }

  return 0;
}

/*****************************************************************************/
/*    This function creates a socket having the protocol family AF_INET and  */
/* of socket type SOCK_STREAM. the server's well known port number is bound  */
/* to the socket by filling internet socket address structure with port      */
/* number. this socket waits on listening the requests from the client.      */
/*                                                                           */
/*  INPUT PARAMETERS                                                         */
/*    i_port_no       The port which is bound to the socket for connection   */
/*    c_ServiceName   The Service requested the service socket creation.     */
/*    c_err_msg       The error message which needs to be displayed on screen*/
/*                                                                           */
/*  OUTPUT PARAMETERS                                                        */
/*    ptr_i_sck_id    The id of the socket got created.                      */
/*                                                                           */
/*****************************************************************************/
int fn_crt_srvr_sck(int i_port_no,
                    int *ptr_i_sck_id,
                    char *c_ServiceName,
                    char *c_err_msg)
{
  int i_tmp_sckid;
  struct sockaddr_in sck_in;

  if ((i_tmp_sckid = fn_socket()) == -1)
  {
    fn_errlog(c_ServiceName, "L31140", UNXMSG, c_err_msg);
    return -1;
  }

  bzero(&sck_in, sizeof(struct sockaddr_in));

  sck_in.sin_family = AF_INET;
  sck_in.sin_port = htons(i_port_no);
  sck_in.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(i_tmp_sckid,
           (struct sockaddr *)&sck_in,
           sizeof(struct sockaddr_in)) == -1)
  {
    fn_errlog(c_ServiceName, "L31145", UNXMSG, c_err_msg);
    return -1;
  }

  if (listen(i_tmp_sckid, 0) == -1)
  {
    fn_errlog(c_ServiceName, "L31150", UNXMSG, c_err_msg);
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
/*                                                                           */
/*  INPUT PARAMETERS                                                         */
/*      i_sck_id      socket id.                                             */
/*      c_ServiceName The service requested this function.                   */
/*      c_err_msg     The error message needs to be displayed on screen.     */
/*                                                                           */
/*  OUTPUT PARAMETERS                                                        */
/*      ptr_i_clnt_sckid  The client socket id.                              */
/*                                                                           */
/*****************************************************************************/
int fn_accept_clnt(int i_sck_id,
                   int *ptr_i_clnt_sckid,
                   char *c_ServiceName,
                   char *c_err_msg)
{
  i_break_status_sck = DONT_EXIT;

  while (i_break_status_sck == DONT_EXIT)
  {
    if ((*ptr_i_clnt_sckid = accept(i_sck_id, NULL, NULL)) == -1)
    {
      if (errno != EINTR)
      {
        fn_errlog(c_ServiceName, "L31155", UNXMSG, c_err_msg);
        return -1;
      }
    }
    else
    {
      return 0;
    }
  }

  fn_errlog(c_ServiceName, "L31160", "Interrupt caught", c_err_msg);
  return -1;
}
int fn_sendto(int i_fd,
              int i_family,
              char *ptr_c_stm,
              int i_length,
              char *ptr_c_ip_addr,
              int i_prt_no,
              char *c_ServiceName,
              char *c_errmsg)
{
  int i_res = 0;
  struct sockaddr_in serv_addr;

  bzero(&serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = i_family;
  serv_addr.sin_addr.s_addr = inet_addr(ptr_c_ip_addr);
  serv_addr.sin_port = htons(i_prt_no);

  i_res = sendto(i_fd, ptr_c_stm, i_length, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (i_res == -1)
  {
    fn_errlog(c_ServiceName, "L31165", UNXMSG, c_errmsg);
    return -1;
  }

  return 0;
}

int fn_crt_foudp_sck(int *ptr_i_sck_id,
                     char *c_brd_port,
                     char *c_ServiceName)
{
  int i_ch_val;
  int i_brd_udp_port;
  int i_sck_id;

  struct sockaddr_in st_machn_addr;

  i_ch_val = fn_udp_socket();

  if (i_ch_val == -1)
  {
    return (-1);
  }

  *ptr_i_sck_id = i_ch_val;

  i_brd_udp_port = atoi(c_brd_port);

  bzero(&st_machn_addr, sizeof(st_machn_addr));

  st_machn_addr.sin_family = AF_INET;
  st_machn_addr.sin_port = htons(i_brd_udp_port);
  st_machn_addr.sin_addr.s_addr = INADDR_ANY;

  fn_userlog(c_ServiceName, "Before Binding");

  i_ch_val = bind(i_ch_val, (struct sockaddr *)&st_machn_addr, sizeof(struct sockaddr_in));

  if (i_ch_val == -1)
  {
    return -1;
  }

  fn_userlog(c_ServiceName, "After Binding");
  return (0);
}
/** Added in Ver 1.2 for ORS2 Broadcast ENDS **/
