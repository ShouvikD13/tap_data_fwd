/******************************************************************************/
/*  version 1.0 - 02-Jan-2012 New Release   Navina D.			                    */
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
/*char sql_c_pipe_id [ 3 ];*/

int fn_read_debug_flg(char * cptr_prcs_name); /* Debug Level */


long li_port;
char c_ip_addr[20];

int tpsvrinit ( int argc, char * argv[] )
{

		int   i_ch_val;
		char  *ptr_c_tmp;
		char  ptr_c_fileName [512];
		char 	c_ServiceName [ 33 ];
  	char 	c_err_msg [ 256 ];	
		char *ptr_c_ip_addr;
		int i_tmp;
		int i_exit_status;

		i_ch_val = 0;
	
		/*while ( strcmp ( argv[i_ch_val], "--" ) != 0 )
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
		sprintf(ptr_c_fileName, "%s/SPAN.ini", getenv("BIN"), argv[i_ch_val]);
		*/

		sprintf(ptr_c_fileName, "%s/SPAN.ini", getenv("BIN"));
		i_ch_val = fn_create_pipe ( );

		if ( i_ch_val == -1 )
  	{
    	userlog ( "EBA : Error creating pipe" );
    	return ( -1 );
  	}	

		strcpy(c_ServiceName, "SFO_CLNT_DUMP");

		if ( tpopen( ) == -1 )
  	{
    	userlog ( "EBA : RM failed in opening connection |%s|", TPMSG );
    	fn_destroy_pipe();
    	return ( -1 );
  	}
	

		/*** Initialize Process Space  ***/
	
	 	i_ch_val = fn_init_prcs_spc( "SFO_CLNT_DUMP",
                                 ptr_c_fileName,
                                 "SFO_CLNT_DUMP" );
  	if (i_ch_val == -1)
  	{
    	userlog ( "EBA : Initializing Process space is failed " );
    	fn_destroy_pipe();
    	return ( -1 );
  	}

		/*** Getting IP address and Port No from the process Space. ***/
		ptr_c_ip_addr = (char *)fn_get_prcs_spc(c_ServiceName, "POPUP_SRVR_ADDR");
		if(ptr_c_ip_addr == NULL)
		{
			fn_errlog(c_ServiceName, "L31005", LIBMSG, c_err_msg);
			return -1;
		}
		strcpy(c_ip_addr, ptr_c_ip_addr);
    
		userlog ( "c_ip_addr = :%s: ",c_ip_addr );
	
		ptr_c_tmp = (char*)fn_get_prcs_spc(c_ServiceName, "POPUP_SRVR_PORT");
		if(ptr_c_tmp == NULL)
		{
			fn_errlog(c_ServiceName, "L31010", LIBMSG, c_err_msg);
			return -1;
		}
		li_port = atol(ptr_c_tmp);

		userlog ( "li_port = :%ld: ",li_port );

		fn_read_debug_flg(argv[0]); /* Debug Level */
		fn_userlog(c_ServiceName,"Finished tpsvrinit");
		return ( 0 );
}

void tpsvrdone ( )
{

		char c_err_msg [ 256 ];
		char c_ServiceName[ 256 ];
		strcpy(c_ServiceName,"SFO_CLNT_DUMP");

		if ( tpclose ( ) == -1 )
		{
			userlog ( "EBA : RM failed in closing connection |%s|", TPMSG );
		}

		fn_destroy_pipe ( );
}

