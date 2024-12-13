#include <stdio.h>
#include <stdlib.h>
#include <userlog.h>
#include <atmi.h>
#include <time.h>
#include <string.h>
#include <fn_env.h>
#include <fn_msgq.h>
#include <fn_log.h>
#include <fo.h>

int i_send_qid;
char sql_c_pipe_id [ 3 ];

int fn_read_debug_flg(char * cptr_prcs_name); /* Debug Level */

int tpsvrinit ( int argc, char * argv[] )
{

    int   i_ch_val;
    int   i_msgq_typ;
    char  *ptr_c_tmp;
    char  ptr_c_fileName [512];
    char  c_ServiceName [ 33 ];
    char  c_err_msg [ 256 ];

    strcpy(c_ServiceName, "SFO_BCON_CLNT" );
    
    i_ch_val = 0;

    while ( strcmp ( argv[i_ch_val], "--" ) != 0 )
    {
      i_ch_val++;
      if ( i_ch_val == argc )
      {
        /*fn_userlog (c_ServiceName, "CLOPT option not specified properly" );  */
        userlog ("CLOPT option not specified properly" ); 
        return ( -1 );
      }
    }

    if ( i_ch_val+1+1 != argc )
    {
      /* fn_userlog (c_ServiceName, "CLOPT option not specified properly" );  */
			userlog ("CLOPT option not specified properly" );  
      return ( -1 );
    }

    i_ch_val++;
    strcpy ( sql_c_pipe_id, argv[i_ch_val] );
    sprintf(ptr_c_fileName, "%s/%s.ini", getenv("BIN"), argv[i_ch_val]);
    
    /* fn_userlog(c_ServiceName," ptr_c_fileName :%s:",ptr_c_fileName);  */
    userlog(" ptr_c_fileName :%s:",ptr_c_fileName);  
  
    i_ch_val = fn_create_pipe ( );

		if ( i_ch_val == -1 )
    {
      /* fn_userlog (c_ServiceName, "EBA : Error creating pipe" );  */
			userlog ("EBA : Error creating pipe" ); 
      return ( -1 );
    }

    /* fn_userlog(c_ServiceName," Inside fn_bconclnt_server after pipe creation");  */
		userlog(" Inside fn_bconclnt_server after pipe creation"); 

    if ( tpopen( ) == -1 )
    {
      /* fn_userlog (c_ServiceName, "EBA : RM failed in opening connection |%s|", TPMSG ); */
			userlog ("EBA : RM failed in opening connection |%s|", TPMSG ); 
      fn_destroy_pipe();
      return ( -1 );
    }


    /*** Initialize Process Space  ***/

    i_ch_val = fn_init_prcs_spc( "SFO_BCON_CLNT",
                                 ptr_c_fileName,
                                 "SFO_BCON_CLNT" );
  if (i_ch_val == -1)
    {
      /* fn_userlog (c_ServiceName, "EBA : Initializing Process space is failed " );  */
			 userlog ("EBA : Initializing Process space is failed " ); 
      fn_destroy_pipe();
      return ( -1 );
    }

    /***** Taking access of Transmit queque. *****/

    ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"SEND_QUEUE");
    i_msgq_typ = atoi(ptr_c_tmp);
    /* fn_userlog(c_ServiceName, " SEND_QUEUE  :%s:",ptr_c_tmp); */
		userlog(" In BCON_CLNT- SEND_QUEUE  :%s:",ptr_c_tmp); 

    if(fn_crt_msg_q(&i_send_qid, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
    {
      /* fn_userlog(c_ServiceName,"EBA : Failed to take access of Transmit queque."); */
			userlog("EBA : Failed to take access of Transmit queque in BCON CLNT."); 
      fn_destroy_pipe();
      return ( -1 );
    }
    /* fn_userlog(c_ServiceName," i_send_qid  :%d:", i_send_qid);  */
		 userlog(" In BCON_CLNT i_send_qid  :%d:", i_send_qid); 
 
    fn_read_debug_flg(argv[0]); /* Debug Level */

    return ( 0 );

}

void tpsvrdone ( )
{

    char c_err_msg [ 256 ];
    if ( tpclose ( ) == -1 )
    {
      userlog ("EBA : RM failed in closing connection |%s|", TPMSG );
    }
    fn_destroy_pipe ( );

}
