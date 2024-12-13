/******************************************************************************/
/*  Program           : CLN_BRD_PRCSQ		                                      */
/*                                                                            */
/*  Input             : C_PIPE_ID                                             */
/*                                                                            */
/*  Description       : This program reads the broadcast message from socket  */
/*                      and enqueue the message in message queue              */
/******************************************************************************/
/*Ver 1.0             : New Release																						*/
/*Ver 1.1             : View to FML Changes ( Sachin Birje ) 									*/
/*Ver TOL             : Tux on Linux ( Agam )    	         	      */
/******************************************************************************/

/**** C header ****/
#include <stdio.h>
#include <sys/socket.h>

/**** Tuxedo header ****/
#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>

/** #include <fo_view_def.h> ** ver 1.1 **/

#include <fo_fml_def.h>
#include <fml_def.h>
#include <fo.h>
#include <fn_tuxlib.h>
#include <fn_battmpt.h>
#include <fn_scklib_tap.h>

/* Process specific header  */
#include <fn_env.h>
#include <fn_log.h>
#include <brd_cln_srvr.h>
#include <fn_fil_log.h>
#include <fn_ddr.h>
#include <fn_msgq.h>

#include <string.h>
#include <stdlib.h>
#include <fml_rout.h>
#include <fn_signal.h>
#include <fn_read_debug_lvl.h>
#include <signal.h>                     /* Added in Ver TOL */

#define MAX_BCAST_PACKET_LEN 516

void fn_sgnl_usr2(int i_sig_no);

char c_pipe_id [ 5+1 ];
char c_brd_port[10];
char c_qspace_name[10];
char c_q_name[10];

int  i_dmp_val;
int  i_sck_id;
int i_exit_stts;

int i_send_qid;

long int li_port;

struct st_q_message
{
long l_msg_typ;
char c_buffer_msg[516];
};


void CLN_BRD_PRCSQ ( int argc, char *argv[] )
{

  long int li_buf_len ;
  int  i_ch_val;
  int  i_ret_val;
  int  i_err;
  int  i_ferr;

	char c_ServiceName [33];
	char c_err_msg [256];
	char c_run_mod;
	char *ptr_c_tmp;
  char c_bcast_data [MAX_BCAST_PACKET_LEN];
  struct st_q_message st_q_msg;

  FBFR32 *Ibuffer;

	strcpy(c_ServiceName, argv[0]);

	INITBATDBGLVL(argv[0]);

	fn_userlog(c_ServiceName,"BEFORE Port No. Is :%s:",c_brd_port);

	if( fn_crt_foudp_sck (&i_sck_id,c_brd_port,c_ServiceName) == -1 )
  {
    fn_errlog( c_ServiceName, "L31005", LIBMSG, c_err_msg );
		exit(0);
  }
  i_exit_stts = DONT_EXIT;

	while ( i_exit_stts == DONT_EXIT )
  {
    memset(&st_q_msg,'\0',sizeof(st_q_msg));

		li_buf_len = sizeof( c_bcast_data );

		memset ( c_bcast_data, '\0', sizeof ( c_bcast_data ) );

    i_ch_val = fn_recv_udp_msg ( i_sck_id,
																 c_bcast_data,
																 li_buf_len,
																 0,
																 c_ServiceName,
																 c_err_msg );
    if ( i_ch_val == -1 )
    {
      fn_errlog ( c_ServiceName, "L31015", LIBMSG, c_err_msg );
      continue;
    }

    if(DEBUG_MSG_LVL_3)
    {
     fn_userlog(c_ServiceName , " Enquing the Broadcast Data ");
    }

    st_q_msg.l_msg_typ =121;
    memcpy(st_q_msg.c_buffer_msg,c_bcast_data,sizeof(c_bcast_data));

    if(fn_write_msg_q(i_send_qid,
                    (void *)&st_q_msg,
                    sizeof(st_q_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
    {
     fn_errlog(c_ServiceName, "L31130", LIBMSG, c_err_msg);
    }
 
  } 

}


int fn_bat_init(int argc, char *argv[])
{
  int  i_ch_val ;
	int  i_loop;
  int  i_msgq_typ;

  char c_ServiceName [ 33 ];
	char c_pipe_id [ 5+1 ];
  char c_err_msg[256];
	char *ptr_c_tmp;
  char c_filename[256];

  strcpy( c_ServiceName, "cln_brd_prcsQ" );

  if( argc != 4)
  {
    fn_userlog(c_ServiceName,
    "Usage - cln_brd_prcsQ <Exchange qualifier> <Exchange code> <pipe_id>");
    return -1; 
  }
 
  strcpy( c_pipe_id, argv[3] );

  ptr_c_tmp = ( char * ) fn_get_prcs_spc ( c_ServiceName, "ORS_SRVR_PORT" );
  if ( ptr_c_tmp == NULL )
  {
    fn_errlog( c_ServiceName, "L31035", LIBMSG, c_err_msg );
    return -1;
  }

	strcpy (c_brd_port,ptr_c_tmp);

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Port No. Is :%s:",c_brd_port);
	}

 
  ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"SEND_QUEUE");
  i_msgq_typ = atoi(ptr_c_tmp);
  fn_userlog(c_ServiceName, " SEND_QUEUE  :%s:",ptr_c_tmp);

  if(fn_crt_msg_q(&i_send_qid, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
  {
     fn_userlog(c_ServiceName,"EBA : Failed to take access of Transmit queque.");
     return ( -1 );
  }

	fn_sck_init();

	if ( fn_set_sgn_hnd ( SIGUSR2, fn_sgnl_usr2, c_ServiceName, c_err_msg ) == -1 )
	{
  	fn_errlog( c_ServiceName, "S31005", LIBMSG, c_err_msg );
  	return -1;
	}

  fn_var = CLN_BRD_PRCSQ;

  return 0;
}

void fn_bat_term(int argc,
                 char *argv[] )
{
  /*  Close socket  */
  char c_err_msg[256];
	char c_ServiceName[256];
	int i_ret_val;

	strcpy(c_ServiceName,argv[0]);

	i_ret_val = fn_close_sck( i_sck_id,
                            c_ServiceName,
                            c_err_msg );

  if ( i_ret_val == -1 )
  {
    fn_userlog ( c_ServiceName, "Error while closing socket." );
  }
  return;
}

int fn_prcs_trg(char *c_ServiceName,
                 int i_trg_typ)
{
  return 0;
}
void fn_sgnl_usr2(int i_sig_no)
{
	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog("cln_brd_clnt","CLN_BRD_PRCSQ SIGUSER2 exit");
	}
  i_exit_stts = EXIT;
  return;
}
