/******************************************************************************/
/*  Program       : crt_exg_rcs.c                                             */
/*                                                                            */
/*  Description   : This utility create transmit queue (SND_Q)                */
/*                  the same will be used by Packing and ESR client           */
/*                                                                            */
/*  Input         : <list of pipe IDs>																				*/
/*                                                                            */
/*  Output        : Success/Failure                                           */
/*                                                                            */
/*  Log           :                                                           */
/*                                                                            */
/*  Ver 1.0   26-Jun-2009   Prakash       Initial Release CRCSN31735 ORS2     */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fn_env.h>
#include <fn_log.h>
#include <fn_msgq.h>
#include <string.h>

void fn_err_dstry_liveq( int i_qlist[], int i_qcnt );
char c_serviceName[33];

int main( int argc, char *argv[] )
{
  char *ptr_c_tmp;
  char c_errmsg[256];
  char c_fileName[256];	
	char c_qname[8];

	int i_qid;
	int i_msgq_typ;
	int i_ch_val;
	int i_cnt;
	int i_alive_qid[10];
	int i_qcnt=0;
	
	/* check for command line arguments  */
  if(argc < 2)
  {
    perror("Usage - crt_exg_rcs <pipe id1> <pipe id2> <pipe id3> ...");
    exit(-1);
  }

	strcpy(c_serviceName, argv[0]);
  sprintf(c_fileName, "%s/exg_rcs.ini", getenv("BIN") );

  /* Create Log pipe */
  i_ch_val = fn_create_pipe();
  if( i_ch_val == -1 )
  {
    exit(-1);
  }
	fn_userlog( c_serviceName, "Utility Started ");

  /* Initialize process variables  */
  if ( fn_init_prcs_spc ( c_serviceName,
                          c_fileName,
                          "CRT_EXG_RCS") == -1)
  {
    fn_destroy_pipe();
    exit(-1);
  }

	for( i_cnt=1; i_cnt<argc; i_cnt++ )
	{
		/* Create a send message queue */
		strcpy( c_qname, "SND_Q");
		strcat( c_qname, argv[i_cnt] );
		fn_userlog( c_serviceName, "Char SND qname  :%s:", c_qname);

		ptr_c_tmp = (char *)fn_get_prcs_spc ( c_serviceName, c_qname );
  	if ( ptr_c_tmp == NULL )
  	{
    	perror("Error - in reading send qname from ini file ...");
			fn_err_dstry_liveq( i_alive_qid, i_qcnt );
    	exit(-1);
  	}
  	i_msgq_typ = atoi(ptr_c_tmp);
		fn_userlog( c_serviceName, "Int SND qname  :%s:", c_qname);

  	if ( fn_crt_msg_q ( &i_qid,
                      i_msgq_typ,
                      OWNER,
                      c_serviceName,
                      c_errmsg) == -1)
  	{
    	perror("Error - in creating creating sendq ...");
			fn_err_dstry_liveq( i_alive_qid, i_qcnt );
    	exit(-1);
  	}
		i_alive_qid[i_qcnt] = i_qid;
		i_qcnt++;

	}
	fn_userlog( c_serviceName, "Utility Ended ");
}

void fn_err_dstry_liveq( int i_qlist[], int i_qcnt )
{
	int i_cnt; 
  char c_errmsg[256];
	
	/* destroy all the alive message queues */
	for( i_cnt=0; i_cnt<i_qcnt; i_cnt++ )
	{
		fn_dstry_msg_q ( i_qlist[i_cnt],
                     c_serviceName,
                     c_errmsg     );
	}

	fn_destroy_pipe();
}
