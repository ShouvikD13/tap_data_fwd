/******************************************************************************/
/*  version 1.0 - 13-Jul-2022 New Release   Sachin Birje                      */
/*  version TOL - 02-Aug-2023 Tux on Linux  Agam                              */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <userlog.h>
#include <atmi.h>
#include <time.h>
#include <string.h>
#include <fn_env.h>
#include <fn_log.h>
#include <fo.h>
#include <ud_sck.h>

#define  SET_KEEPALIVE   1

int i_span_sck_id;
int i_sck_indctr;					
char sql_c_pipe_id [ 3 ];
char c_primary_ip[20];	  
char c_backup_ip[20];		
char c_third_ip[20];	
char c_span_srvr_id[20];	
long li_port;						

int fn_read_debug_flg(char * cptr_prcs_name); /* Debug Level */

int fn_crt_clnt_sck(char *c_ip_addr, int li_port, int *i_span_sck_id, char *c_ServiceName, char *c_err_msg);
/* Ver TOL tux on linux long li_port changed to int*/
int fn_crt_sck(char *c_ServiceName, char *c_err_msg);		

int fn_close_sck(int i_sck_id,char *c_ServiceName,char *c_err_msg);


int tpsvrinit ( int argc, char * argv[] )
{

		int   i_ch_val;
		int   i_primary_fallback_indctr;    
		char  *ptr_c_tmp;
		char  ptr_c_fileName [512];
		char 	c_ServiceName [ 33 ];
  	char 	c_err_msg [ 256 ];	
		char *ptr_c_ip_addr;
		char c_ip_addr[20];
		int i_tmp;
		int i_exit_status;

		i_ch_val = 0;
	
	/*	while ( strcmp ( argv[i_ch_val], "--" ) != 0 )
  	{
    	i_ch_val++;
    	if ( i_ch_val == argc )
    	{
      	userlog ( "CLOPT option not specified properly" );
      	return ( -1 );
    	}
  	}

		if ( i_ch_val+1+1 != argc )
  	{
    	userlog ( "CLOPT option not specified properly" );
    	return ( -1 );
  	}

		i_ch_val++;
  	strcpy ( sql_c_pipe_id, argv[i_ch_val] );		
		sprintf(ptr_c_fileName, "%s/%s.ini", getenv("BIN"), argv[i_ch_val]);
		**/

		strcpy(c_span_srvr_id,argv[6]);     /*** Added in Ver 1.2 ***/

		sprintf(ptr_c_fileName, "%s/SPAN.ini", getenv("BIN"));

		i_ch_val = fn_create_pipe ( );

		if ( i_ch_val == -1 )
  	{
    	userlog ( "EBA : Error creating pipe" );
    	return ( -1 );
  	}	

		strcpy(c_ServiceName, "SFO_SPAN_BMRGN");

		if ( tpopen( ) == -1 )
  	{
    	userlog ( "EBA : RM failed in opening connection |%s|", TPMSG );
    	fn_destroy_pipe();
    	return ( -1 );
  	}
	

		/*** Initialize Process Space  ***/
	
	 	i_ch_val = fn_init_prcs_spc( "SFO_SPAN_BMRGN",
                                 ptr_c_fileName,
                                 "SFO_SPAN_BMRGN" );
  	if (i_ch_val == -1)
  	{
    	userlog ( "EBA : Initializing Process space is failed " );
    	fn_destroy_pipe();
    	return ( -1 );
  	}

		/*** Getting IP address and Port No from the process Space. ***/
		ptr_c_ip_addr = (char *)fn_get_prcs_spc(c_ServiceName, "SPAN_PRIMARY_ADDR");
		if(ptr_c_ip_addr == NULL)
		{
			fn_errlog(c_ServiceName, "L31005", LIBMSG, c_err_msg);
			return -1;
		}
		strcpy(c_primary_ip, ptr_c_ip_addr);							

		/*** Getting IP address and Port No from the process Space. ***/
		ptr_c_ip_addr = (char *)fn_get_prcs_spc(c_ServiceName, "SPAN_BACKUP_ADDR");
		if(ptr_c_ip_addr == NULL)
		{
			fn_errlog(c_ServiceName, "L31010", LIBMSG, c_err_msg);
			return -1;
		}
		strcpy(c_backup_ip, ptr_c_ip_addr);							

	  /******* Added in ver 1.2 ********/	

		ptr_c_ip_addr = (char *)fn_get_prcs_spc(c_ServiceName, "SPAN_THIRD_ADDR");
    if(ptr_c_ip_addr == NULL)
    {
      fn_errlog(c_ServiceName, "L31015", LIBMSG, c_err_msg);
      return -1;
    }
    strcpy(c_third_ip, ptr_c_ip_addr);

		fn_userlog(c_ServiceName,"The Primary ip is |%s|",c_primary_ip);
		fn_userlog(c_ServiceName,"The back up ip is |%s|",c_backup_ip);
		fn_userlog(c_ServiceName,"The Third ip is   |%s|",c_third_ip);
	  
		/******* ver 1.2 Ended********/	

		ptr_c_tmp = (char*)fn_get_prcs_spc(c_ServiceName, "SPAN_SRVR_PORT");
		if(ptr_c_tmp == NULL)
		{
			fn_errlog(c_ServiceName, "L31020", LIBMSG, c_err_msg);
			return -1;
		}
		li_port = atoi(ptr_c_tmp);

		/*** creating client socket. ***/

		i_sck_indctr = 1;  		/**Primary Socket **/
		if(fn_crt_sck	( c_ServiceName,
									 c_err_msg) == -1)
		{
			fn_errlog(c_ServiceName, "L31025", LIBMSG, c_err_msg);
			return -1;
		}

    /**Insert the details like USER,SID,IP and Port in LOG table **/

		fn_userlog(c_ServiceName,"Client Socket created");

		fn_read_debug_flg(argv[0]); /* Debug Level */

		fn_userlog(c_ServiceName,"socket ids: send:%d:",i_span_sck_id);
		fn_userlog(c_ServiceName,"Finished tpsvrinit");

		return ( 0 );
}

void tpsvrdone ( )
{

		char c_err_msg [ 256 ];
		char c_ServiceName[ 256 ];
		strcpy(c_ServiceName,"SFO_SPAN_BMRGN");

    if( fn_close_sck(i_span_sck_id,c_ServiceName,c_err_msg) == -1 )
		{
			fn_userlog(c_ServiceName,"Error in closing send socket");
		}

		if ( tpclose ( ) == -1 )
		{
			userlog ( "EBA : RM failed in closing connection |%s|", TPMSG );
		}

		fn_destroy_pipe ( );
}


int fn_crt_sck ( char *c_ServiceName,
								 char *c_err_msg)
{

	if(i_sck_indctr == 1 )
	{
		if( fn_crt_clnt_sck(c_primary_ip, 
												li_port, 
												&i_span_sck_id, 
												c_ServiceName, 
												c_err_msg) == -1 )
		{
			fn_errlog(c_ServiceName, "L31030", LIBMSG, c_err_msg);
			i_sck_indctr = 2; /*** Added in ver 1.2 Reason :- In case exe of server not available ***/
			return -1;
		}

		i_sck_indctr = 2; 

	}else if(i_sck_indctr == 2 )
	{

		if( fn_crt_clnt_sck(c_backup_ip, 
												li_port, 
												&i_span_sck_id, 
												c_ServiceName, 
												c_err_msg) == -1 )
		{
			fn_errlog(c_ServiceName, "L31035", LIBMSG, c_err_msg);
			i_sck_indctr = 3; /*** Added in ver 1.2 Reason :- In case exe of server not available ***/
			return -1;
		}

		/*** i_sck_indctr = 1; Commented in Ver 1.2 ***/
		i_sck_indctr = 3;  /*** Added in ver 1.2 ***/

	}else if(i_sck_indctr == 3 ) /*** Added in ver 1.2 ***/
  {

    if( fn_crt_clnt_sck(c_third_ip,
                        li_port,
                        &i_span_sck_id,
                        c_ServiceName,
                        c_err_msg) == -1 )
    {
      fn_errlog(c_ServiceName, "L31040", LIBMSG, c_err_msg);
      i_sck_indctr = 1; /*** Added in ver 1.2 Reason :- In case exe of server not available ***/
      return -1;
    }
    
		i_sck_indctr = 1;

  } /*** Ver 1.2 Ends ***/
	
	return 0;
}

/*****************************************************************************/
/*                                                                           */
/*  Description       :  Function to create the socket by reading he desired */
/*                       Inputs from the process space.                      */
/*                                                                           */
/*  Input Parameters  :  socket id.                                          */
/*                                                                           */
/*  Output Parameters :  Returns 0 upon creating the socket and -1 on failure*/
/*                                                                           */
/*****************************************************************************/
int fn_crt_clnt_sck(char *ptr_c_ip_addr,
                    int  i_port_no,
                    int  *ptr_i_sck_id,
                    char *c_ServiceName,
                    char *c_err_msg)
{
  int i_tmp_sck_id;
  int i_setval;

  socklen_t ud_optlen;

  struct sockaddr_in sck_in;

  if(  fn_socket ( &i_tmp_sck_id, AF_INET, SOCK_STREAM, 0 ) == -1 )
  {
    fn_errlog(c_ServiceName, "L31045", UNXMSG, c_err_msg);
    return -1;
  }

  i_setval = SET_KEEPALIVE;
  ud_optlen = (socklen_t)sizeof(int);

  if( fn_setsockopt ( i_tmp_sck_id, SOL_SOCKET, SO_KEEPALIVE, &i_setval, ud_optlen ) == -1 )
  {
    fn_errlog(c_ServiceName, "L31050", UNXMSG, c_err_msg);
    return -1;
  }



  i_setval = TCP_NODELAY;
  ud_optlen = (socklen_t)sizeof(int);


  if( fn_setsockopt ( i_tmp_sck_id, IPPROTO_TCP, TCP_NODELAY, &i_setval, ud_optlen ) == -1 )
  {
    errlog(c_ServiceName, "S31125", UNXMSG, (char *)DEF_USR, DEF_SSSN, c_err_msg);
    return -1;
  }

  i_setval = 20;
  if( fn_setsockopt ( i_tmp_sck_id, IPPROTO_TCP, TCP_KEEPIDLE, &i_setval, ud_optlen ) == -1 )
  {
    fn_errlog(c_ServiceName, "L31055", UNXMSG, c_err_msg);
    return -1;
  }

  i_setval = 20;
  if( fn_setsockopt ( i_tmp_sck_id, IPPROTO_TCP, TCP_KEEPINTVL, &i_setval, ud_optlen ) == -1 )
  {
    fn_errlog(c_ServiceName, "L31060", UNXMSG, c_err_msg);
    return -1;
  }

  i_setval = 600;
  if( fn_setsockopt ( i_tmp_sck_id, IPPROTO_TCP, TCP_KEEPCNT, &i_setval, ud_optlen ) == -1 )
  {
    fn_errlog(c_ServiceName, "L31065", UNXMSG, c_err_msg);
    return -1;
  }

  i_setval = 1;
  if( fn_setsockopt ( i_tmp_sck_id, IPPROTO_TCP, TCP_NODELAY, &i_setval, ud_optlen ) == -1 ) /* tux on linux TCP_NODELAYACK to TCP_NODELAY*/
  {
    fn_errlog(c_ServiceName, "L31070", UNXMSG, c_err_msg);
    return -1;
  }



  if( fn_connect ( i_tmp_sck_id, AF_INET, ptr_c_ip_addr, i_port_no ) == -1 )
  {
    fn_errlog(c_ServiceName, "L31075", UNXMSG, c_err_msg);
    return -1;
  }

 *ptr_i_sck_id = i_tmp_sck_id;

  return 0;

}

int fn_close_sck(int i_sck_id,char *c_ServiceName,char *c_err_msg)
{
  if(close(i_sck_id) == -1)
  {
    fn_errlog(c_ServiceName, "L31080", UNXMSG,c_err_msg);
    return -1;
  }
  return 0;
}
