/******************************************************************************/
/*  Program           :  cln_ors_svr.c                                       	*/
/*                                                                            */
/*  Input             :  pipe id									                           	*/
/*                                                                            */
/*  Output            :  -                                                    */
/*                                                                            */
/*  Description       :  ORS server manages the broadcast server and the      */
/*                       broadcast process on request from console client.    */
/*                       It also manages the broadcast queue.                 */
/*  Log               :                                                       */
/*                                                                            */
/******************************************************************************/
/*	Indrajit Bhadange version 1.0   27-Jul-2009                                */
/******************************************************************************/

/* Header file */
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/wait.h>
#include<signal.h>

/* Application headers */
#include <fn_env.h>
#include <fn_ors_log.h>
#include <fn_signal.h>
#include <fo.h>
#include <fn_msgq.h>
#include <eba_to_ors.h>
#include <fn_scklib_tap.h>

#define	NOT_RUNNING 0
#define	RUNNING 1
#define	STARTED 2

/*  Global variables  */
int i_exit_status;
char c_xchng_cd[3+1];	
long li_prcs_id;

long li_brdsrvr_id;
long li_brdprcs_id;


void fn_sigusr2_hnd( int signo);

void fn_sigchld_hnd( int signo);

void fn_dmn_func(				 	int i_sck_id,
												 	char *c_xchng_cd,
												 	char *c_serviceName,
													char *c_errmsg );

void fn_stp_chld (  		 	char *c_prcs_name,
												 	char *c_xchng_cd,
												 	char *c_serviceName,
												 	char *c_errmsg);

int fn_start_brdcast_chnl(int i_sck_id,
													int i_chld_sck,
                      		char *c_xchng_cd,
                      		char *c_serviceName,
                      		char *c_errmsg);

void fn_res_to_con_clnt(  int i_status,
                          int i_chld_sck,
                          char *c_status_mesg,
													char *c_serviceName,
													char *c_errmsg);

int fn_stop_prcs(					long li_pro_id, 
													char *c_name, 
													char *c_serviceName,
													char *c_errmsg);

int fn_check_status ( 		char *c_prcs_nm,
                      		char *c_xchng_cd,
                      		long *li_pro_id,
                      		char *c_serviceName,
                      		char *c_errmsg);

void fn_get_status( 			int i_chld_sck,
                    			char  *c_xchng_cd,
                    			char  *c_serviceName,
                    			char  *c_errmsg);

void main(int argc, char *argv [])
{
	char *ptr_c_tmp;
  char c_serviceName[33];
  char c_errmsg[256];
  char c_fileName[256];

  int i_ch_val;
  int i_brdqid;
  int i_sck_id;
	int i_msgq_key;
  long int li_port;

  strcpy(c_serviceName, "cln_ors_srvr");

 	/**** Create Log pipe ****/
 	i_ch_val = fn_create_pipe();
 	if( i_ch_val == -1 )
 	{
		fn_errlog(c_serviceName, "S31005", LIBMSG, c_errmsg);	
   	exit(0);
 	}

	fn_userlog(c_serviceName,"Starting ors server.....");

  /**** check for command line arguments  ****/
  if(argc != 2)
  {
    fn_userlog(c_serviceName,"Usage - cln_ors_srvr <Exchange code>");
    exit(0);
  }
  strcpy(c_xchng_cd , argv[1]);
  sprintf(c_fileName, "%s/%s.ini", getenv("BIN"), c_xchng_cd );


 	/**** Initialize process variables  ****/
 	if ( fn_init_prcs_spc ( c_serviceName,
                         	c_fileName,
                         	"cln_ors_srvr") == -1)
 	{
		fn_errlog(c_serviceName, "S31010", LIBMSG, c_errmsg);	
    fn_destroy_pipe();
    exit(0);
 	}

	/**** Create a boardcast message queue ****/
	ptr_c_tmp = (char *)fn_get_prcs_spc ( c_serviceName, "BRD_Q" );
	if ( ptr_c_tmp == NULL )
	{
  	fn_errlog(c_serviceName, "S31015", LIBMSG, c_errmsg);
  	fn_destroy_pipe();
  	exit(0);
	}
	i_msgq_key = atoi(ptr_c_tmp);

	if ( fn_crt_msg_q( &i_brdqid,
                     i_msgq_key,
                     OWNER,
                     c_serviceName,
                     c_errmsg) == -1)
  {
    fn_errlog(c_serviceName, "S31020", LIBMSG, c_errmsg);
    fn_destroy_pipe();
    exit(0);
  }
	fn_userlog(c_serviceName,"Broadcast queue id is :%d:",i_brdqid);

 	/**** Set signal handlers ****/

	if ( fn_set_sgn_hnd ( SIGUSR2,
                        fn_sigusr2_hnd,
                        c_serviceName,
                        c_errmsg) == -1)
  {
    fn_errlog(c_serviceName, "S31025", LIBMSG, c_errmsg);
		fn_dstry_msg_q ( i_brdqid,
                     c_serviceName,
                     c_errmsg     );
    fn_destroy_pipe();
    exit(0);
  }

 	/**** Set signal handler for SIGCHLD  ****/
  if ( fn_set_sgn_hnd ( SIGCHLD,
                        fn_sigchld_hnd,
                        c_serviceName,
                        c_errmsg) == -1)
  {
    fn_errlog(c_serviceName, "S31030", LIBMSG, c_errmsg);
    fn_dstry_msg_q ( i_brdqid,
                     c_serviceName,
                     c_errmsg     );
    fn_destroy_pipe();
    exit(0);
  }

	/**** Read the ors server port ****/

  ptr_c_tmp = (char *)fn_get_prcs_spc ( c_serviceName, "ORS_SRVR_PORT" );
  if ( ptr_c_tmp == NULL )
  {
    fn_errlog(c_serviceName, "S31035", LIBMSG, c_errmsg);
    fn_dstry_msg_q ( i_brdqid,
                     c_serviceName,
                     c_errmsg     );
    fn_destroy_pipe();
    exit(0);
  }
  li_port = atol(ptr_c_tmp);
	
	fn_userlog(c_serviceName,"ors server port is :%ld:",li_port);

	/**** Create a server socket ****/
  i_ch_val = fn_crt_srvr_sck ( li_port,
                               &i_sck_id,
                               c_serviceName,
                               c_errmsg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_serviceName, "S31040", LIBMSG, c_errmsg);
    fn_dstry_msg_q ( i_brdqid,
                     c_serviceName,
                     c_errmsg     );
    fn_destroy_pipe();
    exit(0);
  }

	fn_userlog(c_serviceName,"ors server socket is  :%d:",i_sck_id);

	/**** Call deamon function ****/
  fn_dmn_func(i_sck_id,
              c_xchng_cd,
              c_serviceName,
              c_errmsg );

	fn_userlog(c_serviceName,"ors server shutting down.....");

	/**** Stop the broadcast server and broadcast process  ****/

	if ( li_brdprcs_id > 0) 
	{
		fn_userlog(c_serviceName,"Stopping cln_brd_prcs :%ld:.....",li_brdprcs_id);
		i_ch_val = fn_stop_prcs(li_brdprcs_id,"cln_brd_prcs",c_serviceName,c_errmsg);

		if (i_ch_val == -1)
		{
			fn_errlog(c_serviceName, "S31045", UNXMSG,c_errmsg);
		}
	}

	if ( li_brdsrvr_id > 0 )
	{
		fn_userlog(c_serviceName,"Stopping cln_brd_srvr :%ld:.....",li_brdsrvr_id);
		i_ch_val = fn_stop_prcs(li_brdsrvr_id,"cln_brd_srvr",c_serviceName,c_errmsg);

		if (i_ch_val == -1)
		{
			fn_errlog(c_serviceName, "S31050", UNXMSG,c_errmsg);
		}
	}

	/**** Close server socket ****/
	fn_userlog(c_serviceName,"Closing parent socket :%d:.....",i_sck_id);
  i_ch_val = fn_close_sck (i_sck_id,
            						   c_serviceName,
                					 c_errmsg);

	/**** Destroy broadcast queue	****/
	fn_userlog(c_serviceName,"Destroying broadcast queue :%d:.....",i_brdqid);
  fn_dstry_msg_q ( i_brdqid,
                   c_serviceName,
                   c_errmsg     );

  /**** wait for all child processes to close ****/
  while (1)
  {
    i_ch_val = waitpid ( -1, NULL, 0 );
    if ( (i_ch_val == -1 ) && ( errno == ECHILD ) )
    {
      break;
    }
  }
	fn_userlog(c_serviceName,"ORS SERVER TERMINATED");
	fn_destroy_pipe();
  exit(0);
}

void fn_dmn_func(int i_sck_id,
                 char *c_xchng_cd,
                 char *c_serviceName,
                 char *c_errmsg )
{
  msg_prcs_req prcs_req;
  msg_frs_hdr frs_hdr;

  int i_chld_sck;
  int i_ch_val;


  /**** Set exit status to DONT_EXIT ****/
  i_exit_status = DONT_EXIT;

	fn_userlog(c_serviceName,"ORS SERVER STARTED");

  while (i_exit_status == DONT_EXIT)
  {
		fn_userlog(c_serviceName,"Waiting for client connection request....");
    /**** Wait for a client to request for connection ****/
    if (fn_accept_clnt ( i_sck_id,
                         &i_chld_sck ,
                         c_serviceName,
                         c_errmsg ) == -1 )
    {
      fn_errlog(c_serviceName, "S31055", LIBMSG, c_errmsg);
			fn_userlog(c_serviceName,"ERROR in socket operation: Returning from daemon function");
			return;
    }

		fn_userlog(c_serviceName,"Child socket is :%d:",i_chld_sck);

		/**** Read the message from socket ****/
    if (fn_read_sck ( i_chld_sck,
                      &prcs_req,
                      &frs_hdr,
                      TM_30,
                      c_serviceName,
                      c_errmsg ) == -1)
    {
      fn_errlog(c_serviceName, "S31060", LIBMSG, c_errmsg);
      i_ch_val = fn_close_sck ( i_chld_sck,
                                c_serviceName,
                                c_errmsg);
      continue;
    }

		fn_userlog(c_serviceName,"Message Type is :%ld:",frs_hdr.li_msg_typ);


		if (frs_hdr.li_msg_typ != ORS_SRVR_REQ )
		{
    	fn_errlog( c_serviceName, "S31065", "Protocol Error", c_errmsg);

    	i_ch_val = fn_close_sck ( i_chld_sck,
                              	c_serviceName,
                              	c_errmsg);
    	continue;
   	}

		fn_userlog(c_serviceName,"Message request is :%ld:",prcs_req.li_prcs_no);

		switch  ( prcs_req.li_prcs_no )
    {
      case START:
				if( li_brdsrvr_id > 0 || li_brdprcs_id > 0 )
				{
					fn_userlog(c_serviceName, "Cannot start broadcast channel !");

					if(li_brdsrvr_id > 0 && li_brdprcs_id > 0)
					{
						fn_userlog(c_serviceName, "Broadcast channel is already running");
						fn_res_to_con_clnt(1,i_chld_sck,"Broadcast channel is RUNNING",c_serviceName,c_errmsg);
					}
					else if ( li_brdsrvr_id > 0 )
					{
						fn_userlog(c_serviceName, "cln_brd_srvr is running with pid :%ld:",li_brdsrvr_id);
						fn_res_to_con_clnt(	1,i_chld_sck,"CLN_BRD_SRVR IS RUNNING..PlEASE STOP CLN_BRD_SRVR..",
																c_serviceName,c_errmsg);
					}
					else if ( li_brdprcs_id > 0 )
					{
						fn_userlog(c_serviceName, "cln_brd_prcs is running with pid :%ld:",li_brdprcs_id);
						fn_res_to_con_clnt(	1,i_chld_sck,"CLN_BRD_PRCS IS RUNNING..PlEASE STOP CLN_BRD_PRCS..",
																c_serviceName,c_errmsg);
					} 
					break;
				}
				i_ch_val 	= fn_start_brdcast_chnl(	i_sck_id,
																						i_chld_sck,
																						c_xchng_cd,
																						c_serviceName,
                           									c_errmsg);

				if (i_ch_val == STARTED)
				{
					fn_res_to_con_clnt(0,i_chld_sck,"Broadcast channel STARTED succesfully",c_serviceName,c_errmsg);
				}
				else
				{
					fn_res_to_con_clnt(-1,i_chld_sck,"ERROR IN STARTING broadcast channel",c_serviceName,c_errmsg);
				}

				break;

				
      case STOP:
				if ( li_brdprcs_id >  0)
				{
					fn_userlog(c_serviceName, "STOPPING cln_brd_prcs :%ld:",li_brdprcs_id);
					i_ch_val = fn_stop_prcs(li_brdprcs_id,"cln_brd_prcs",c_serviceName,c_errmsg);
				
					if (i_ch_val == -1)
					{
						fn_errlog(c_serviceName, "S31070", UNXMSG,c_errmsg);
					}
					fn_userlog(c_serviceName, "child process cln_brd_prcs stopped");
				}
				
				
				if ( li_brdsrvr_id > 0 )
				{
					fn_userlog(c_serviceName, "STOPPING cln_brd_srvr :%ld:",li_brdsrvr_id);
					i_ch_val = fn_stop_prcs(li_brdsrvr_id,"cln_brd_srvr",c_serviceName,c_errmsg);
				
					if (i_ch_val == -1)
					{
						fn_errlog(c_serviceName, "S31075", UNXMSG,c_errmsg);
					}
					fn_userlog(c_serviceName, "child process cln_brd_srvr stopped");
				}

				li_brdsrvr_id = 0;
				li_brdprcs_id = 0;

				fn_res_to_con_clnt(0,i_chld_sck,"broadcast channel STOPPED",c_serviceName,c_errmsg);

        break;

			case STATUS:
			
				fn_get_status(i_chld_sck,
                      c_xchng_cd,
                      c_serviceName,
                      c_errmsg);
				
        break;

      default:
        fn_errlog ( c_serviceName, "S31080", "Protocol error: INVALID SERVICE REQUEST", c_errmsg);
				fn_res_to_con_clnt(-1,i_chld_sck,"INVALID SERVICE REQUEST",c_serviceName,c_errmsg);
        break;
     }  /**** End of switch loop  ****/
		 /**** Close child socket ****/
  	 i_ch_val = fn_close_sck(	i_chld_sck,
       				                c_serviceName,
               			          c_errmsg);
	} /***** End of while loop ****/
	return;
}
		
		
void fn_sigchld_hnd (int i_signo)
{
	char c_errmsg[256];
  char c_serviceName[33];
	int i_ch_val;

  strcpy( c_serviceName, "cln_ors_srvr");
	
  fn_userlog(c_serviceName, "Trigger got is SIGCHLD");

	i_ch_val =  waitpid(-1,NULL , 0); 
  fn_userlog(c_serviceName, "waitpid return values is :%d:",i_ch_val);
	if ( i_ch_val == -1 )
	{
		return;
	}

	i_ch_val = fn_check_status ("cln_brd_prcs",
                              c_xchng_cd,
                              &li_prcs_id,
                              c_serviceName,
                              c_errmsg);
  if( i_ch_val == RUNNING)
  {
		fn_userlog(c_serviceName, "Attempting to terminate cln_brd_prcs pid :%ld:",li_prcs_id);
    i_ch_val = fn_stop_prcs(li_prcs_id,"cln_brd_prcs",c_serviceName,c_errmsg);

    if (i_ch_val == -1)
    {
      fn_errlog(c_serviceName, "S31085", UNXMSG,c_errmsg);
      return;
    }
	}
	else if ( i_ch_val == NOT_RUNNING)
	{
		fn_userlog(c_serviceName, "Process cln_brd_prcs does not exist");
		i_ch_val = fn_check_status ("cln_brd_srvr",
                              	c_xchng_cd,
                              	&li_prcs_id,
                              	c_serviceName,
                              	c_errmsg);
		if( i_ch_val == RUNNING)
  	{
			fn_userlog(c_serviceName, "Attempting to terminate cln_brd_srvr pid :%ld:",li_prcs_id);
    	i_ch_val = fn_stop_prcs(li_prcs_id,"cln_brd_srvr",c_serviceName,c_errmsg);

    	if (i_ch_val == -1)
    	{
      	fn_errlog(c_serviceName, "S31090", UNXMSG,c_errmsg);
      	return;
    	}
  	}
		else if ( i_ch_val == NOT_RUNNING)
		{	
			fn_userlog(c_serviceName, "Process cln_brd_srvr does not exist");
			fn_userlog(c_serviceName, "Both child processes exited");
		}
		else
		{
			fn_userlog(c_serviceName, "FILE OPENING ERROR");
			return;
		}

	}
	else
	{
		fn_userlog(c_serviceName, "FILE OPENING ERROR");
		return;
	}
	li_brdsrvr_id = 0;
	li_brdprcs_id = 0;
	fn_userlog(c_serviceName, "Returning from SIGCHLD handler");
	return;
}

void fn_sigusr2_hnd( int i_signo)
{
  fn_userlog ( "cln_ors_srvr", "Trigger got is SIGUSR2");

  /**** set exit status to EXIT ****/
  i_exit_status = EXIT;

  /**** To inform signal caught to socket operation ****/ 
	fn_set_break_sck();
	
	fn_userlog("cln_ors_srvr", "Returning from SIGUSR2 handler");
	return;
	
}

int fn_check_status (	char *c_prcs_nm,
										 	char *c_xchng_cd,
											long *li_pro_id,
											char *c_serviceName, 
											char *c_errmsg)
{
  char  c_command[150];
  char  c_file_nm[100];
  char  c_process[20];
  FILE  *fptr;
  long  li_tmp_prc_id = 0;
	int i_retval = 0;
  sigset_t newset;
	sigset_t oldset;
  sigset_t *newset_p;
	newset_p = &newset;

  MEMSET(c_file_nm);
  MEMSET(c_command);

  strcpy(c_file_nm, getenv("STATUS_FILE"));

  fn_userlog(c_serviceName,"Checking status of %s.....",c_prcs_nm);


  remove(c_file_nm);
	
  sprintf(c_command, "ps -efa -o pid,comm,user,args | grep -v 'grep' |grep $USER | grep \"%s %s\" > %s",
					c_prcs_nm,c_xchng_cd,c_file_nm);

  system(c_command);

  fptr = (FILE *)fopen(c_file_nm, "r");
  if(fptr == NULL)
  {
    fn_errlog(c_serviceName,"S31095", LIBMSG, c_errmsg);
    return -1;
	}
	MEMSET(c_process);

  fscanf(fptr, "%d %s",&li_tmp_prc_id,c_process);

	if(li_tmp_prc_id == 0) /* process does not exist*/
	{
  	fclose(fptr);
		return NOT_RUNNING;
	}
	else /* Process exists */
	{
  	fn_userlog(c_serviceName, "Process %s is running with pid :%d",c_process,li_tmp_prc_id);
  	*li_pro_id = li_tmp_prc_id;
  	fclose(fptr);
  	return RUNNING;
	}	
}	



int fn_stop_prcs(long li_pro_id, char *c_name, char* c_serviceName,char *c_errmsg)
{

  union sigval st_data;
  st_data.sival_int = SIGUSR2;

  /*Raise a signal*/
  if(sigqueue(li_pro_id,SIGUSR2, st_data) == 0)
  {
   	fn_userlog(c_serviceName, "Signal(%d) raised to %s :%d:", SIGUSR2,c_name,li_pro_id);
  }
	else
	{
		if(sigqueue(li_pro_id,SIGKILL, st_data) == -1)
		{
  		fn_errlog(c_serviceName, "S31100", UNXMSG,c_errmsg);
  		return -1;
		}
   	fn_userlog(c_serviceName, "Signal(%d) raised to %s :%d;", SIGKILL,c_name,li_pro_id);
	}
	return 0;
}

int fn_start_brdcast_chnl(  int	i_sck_id,
														int i_chld_sck,
														char	*c_xchng_cd,
														char 	*c_serviceName,
														char 	*c_errmsg)
{
  char c_path[512];
  char c_exer_str[20];
  char c_sck[10];
	int i_ch_val;
	int i_ch_val_srvr;
	int i_ch_val_prcs;
	int i_brd_srvr_stts;
	int i_brd_prcs_stts;
	
	
	fn_userlog(c_serviceName, "Starting Broadcast channel.....");	
	/**** Fork cln_brd_srvr ****/
 	i_ch_val_srvr = fork();
	fn_userlog(c_serviceName, "i_ch_val_srvr :%d:",i_ch_val_srvr);

	if( i_ch_val_srvr == 0)
	{
		/*******  This is Child process ********/
		/**** Close parent socket ****/
		fn_userlog(c_serviceName, "Closing parent socket :%d: of child process cln_brd_srvr",i_sck_id);
	  i_ch_val =fn_close_sck (i_sck_id,
														c_serviceName,
														c_errmsg);

	  /**** Close child socket ****/
		fn_userlog(c_serviceName, "Closing child socket of :%d: of child process cln_brd_srvr",i_chld_sck);
 		i_ch_val = fn_close_sck(i_chld_sck,
             			          c_serviceName,
                   			    c_errmsg);

		/**** Execute the child process space *****/
		strcpy ( c_exer_str, "cln_brd_srvr" );
		sprintf(c_path, "%s/%s", getenv("BIN"), c_exer_str);
		fn_userlog(c_serviceName, "Executing child %s %s ",c_path,c_xchng_cd);

		i_ch_val = execl(c_path, c_exer_str, c_xchng_cd, NULL);

		if(i_ch_val == -1)
		{
			fn_userlog(c_serviceName, "error in executing cln_brd_srvr");
			fn_errlog(c_serviceName, "S31105", UNXMSG, c_errmsg);
			exit(0);	
		}
	}
	else if( i_ch_val_srvr < 0)
	{
		fn_userlog(c_serviceName, "ERROR in forking cln_brd_srvr ");    
		return -1;	
	}
	else
	{		
		/*******  This is Parent process ********/
		li_brdsrvr_id = i_ch_val_srvr;
		fn_userlog(c_serviceName, "CLN_BRD_SRVR FORK SUCCESFULL PID :%ld:",li_brdsrvr_id);

		/**** Fork cln_brd_prcs ****/
  	i_ch_val_prcs = fork();
		fn_userlog(c_serviceName, "i_ch_val_prcs :%d:",i_ch_val_prcs);

  	if ( i_ch_val_prcs == 0)
 		{
			/*******  This is Child process ********/
	 		/**** Close parent socket ****/

	 		fn_userlog(c_serviceName, "Closing parent socket :%d: of child process cln_brd_prcs",i_sck_id);
	 		i_ch_val =fn_close_sck (	i_sck_id,
																c_serviceName,
																c_errmsg);

			/**** Close child socket ****/
			fn_userlog(c_serviceName, "Closing child socket :%d:of child process cln_brd_prcs",i_chld_sck);
			i_ch_val = fn_close_sck(i_chld_sck,
															c_serviceName,
															c_errmsg);
	
			strcpy ( c_exer_str, "cln_brd_prcs" );
   		sprintf(c_path, "%s/%s", getenv("BIN"), c_exer_str);
			fn_userlog(c_serviceName, "Executing child %s %s",c_path,c_xchng_cd);
   		i_ch_val = execl(c_path, c_exer_str, c_xchng_cd, NULL);

   		if (i_ch_val == -1)
   		{
				fn_userlog(c_serviceName, "error in executing cln_brd_prcs");
     		fn_errlog(c_serviceName, "S31110", UNXMSG, c_errmsg);
     		exit(0);
   		}
		}
		else if(i_ch_val_prcs < 0)
		{
			fn_userlog(c_serviceName, "ERROR in forking cln_brd_prcs ");    
    	return -1;
		}
		else
		{
			/*******  This is Parent process ********/
			li_brdprcs_id = i_ch_val_prcs;
			fn_userlog(c_serviceName, "CLN_BRD_PRCS FORK SUCCESFULL PID :%ld:",li_brdprcs_id);
		}
	}
	return STARTED;
}

void fn_res_to_con_clnt( 	int i_status,
													int i_chld_sck,
													char *c_status_mesg,										
													char *c_serviceName,
													char *c_errmsg 
												)
{
  msg_prcs_res prcs_res;
  msg_frs_hdr frs_hdr;
	int i_ch_val;

	fn_userlog(c_serviceName, "In function fn_res_to_con_clnt");
	/**** Send message broadcast server created ****/
            
	/**** Header ****/
  frs_hdr.li_msg_typ = ORS_SRVR_RES ;
  frs_hdr.li_msg_len = sizeof(msg_prcs_res);

  /**** Message ****/
  prcs_res.li_stts = i_status;
  strcpy ( prcs_res.c_msg, c_status_mesg);
  prcs_res.li_flags = 0;

  /**** Write a message in the child socket ****/
	fn_userlog(	c_serviceName, "STATUS is :%ld: MESSAGE is :%s: on socket id :%d:",
							prcs_res.li_stts,prcs_res.c_msg,i_chld_sck);

	if ( fn_write_sck ( i_chld_sck,
											&prcs_res,
											frs_hdr,
											c_serviceName,
											c_errmsg) == -1)
	{
		fn_errlog(c_serviceName, "S31115", LIBMSG, c_errmsg);
	}

	fn_userlog(c_serviceName, "Message sent on child socket :%d:",i_chld_sck);
	return;
}

void fn_get_status(	int i_chld_sck,
										char *c_xchng_cd,
										char *c_serviceName,
										char *c_errmsg)
{
  int i_ch_val;

	if(li_brdsrvr_id > 0 &&  li_brdprcs_id > 0)
	{
    fn_userlog(c_serviceName, "cln_brd_srvr is RUNNING with pid :%ld:",li_brdsrvr_id);
    fn_userlog(c_serviceName, "cln_brd_prcs is RUNNING with pid :%ld:",li_brdprcs_id);
		fn_res_to_con_clnt(BOTH_UP,i_chld_sck,"Both servers are running ",c_serviceName,c_errmsg);
	}
	else if( li_brdsrvr_id == 0 && li_brdprcs_id == 0 )
	{
    fn_userlog(c_serviceName, " No server is running ");
		fn_res_to_con_clnt(BOTH_DWN,i_chld_sck,"No server is running ",c_serviceName,c_errmsg);	
	}
	else if( li_brdsrvr_id > 0 && li_brdprcs_id == 0)
	{
    fn_userlog(c_serviceName, "cln_brd_srvr is RUNNING with pid :%ld:",li_brdsrvr_id);
		fn_res_to_con_clnt(BRD_SRVR_UP,i_chld_sck,"cln_brd_srvr is running",c_serviceName,c_errmsg);	
	}
	else if( li_brdsrvr_id == 0 && li_brdprcs_id > 0)
	{
    fn_userlog(c_serviceName, "cln_brd_prcs is RUNNING with pid :%ld:",li_brdprcs_id);
		fn_res_to_con_clnt(BRD_PRCS_UP,i_chld_sck,"cln_brd_prcs is running",c_serviceName,c_errmsg);	
	}
	return;
}		
