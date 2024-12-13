/**************************************************************************************/
/*  Library Name :fn_fobse_socket.c																										*/
/*	LOG :																																							*/
/*	Ver 1.0 : Initial Release																													*/
/**************************************************************************************/
/*Header Files*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <fn_mcast_log.h>
#include <ud_fobse_socket.h>
#include <eba.h>
#include <fn_mcast_env.h>
int i_break_status_sck;

/******************************************************************************/
/* This function sets the break_status_sck variable to indicate that the user */
/* signal is caught.                                                          */
/*  INPUT PARAMETERS                                                          */
/*    None                                                                    */
/*  OUTPUT PARAMETERS                                                         */
/*    None                                                                    */
/******************************************************************************/
void fn_set_break_sck()
{
  i_break_status_sck = EXIT;
}


int fn_crt_clnt_sck ( char  *c_ServiceName,
                      int   *ptr_i_udp_sck,
                      char  *c_brd_udp_port,
                      char  *c_errmsg )
{
  int   i_ch_val;
  int   i_brd_udp_port;

  /*  Create a UDP socket */
  i_ch_val = fn_socket ( ptr_i_udp_sck,
                         PF_INET,
                         SOCK_DGRAM,
                         0 );
  if ( i_ch_val != NOERROR )  /* Error in udp socket creation  */
  {
    fn_errlog ( c_ServiceName, "S31025", UNXMSG, c_errmsg );
    return ( i_ch_val );
  }

  i_brd_udp_port = atoi ( c_brd_udp_port );

  fn_userlog (c_ServiceName,"Socket Created is :%d: Now Binding it to port no :%d:", *ptr_i_udp_sck, i_brd_udp_port );

  /*  Bind the UDP socket with the given server addr & port no. */
  i_ch_val = fn_bind ( *ptr_i_udp_sck,
                        AF_INET,
                        INADDR_ANY,
                        0,
                        i_brd_udp_port,
                        CLIENT );

  if ( i_ch_val != NOERROR )  /*  Error while binding socket */
  {
    fn_errlog ( c_ServiceName, "S31030", UNXMSG,c_errmsg );
    return ( i_ch_val );
  }

  return 0;
}

/******************************************************************************/
/*  This function is for closing the socket                                   */
/*  INPUT PARAMETERS                                                          */
/*      i_sck_id      - The socket which has to be closed.                    */
/*			c_ServiceName - Service Name                                          */ 
/*      c_err_msg     - The error message to be displayed on screen           */
/*  OUTPUT PARAMETERS                                                         */
/*        0 - Success                                                         */
/*       -1 - Failure                                                         */
/******************************************************************************/
int fn_close_sck(int  i_sck_id ,char *c_ServiceName, char *c_err_msg)
{
  /*Close the socket*/
  if(close(i_sck_id) == -1)
  {
    fn_errlog(c_ServiceName,"L31005", UNXMSG, c_err_msg);
    return -1;
  }
  return 0;
}

