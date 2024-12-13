/******************************************************************************/
/*  Description : Get the access of Unix Queues                               */ 
/*  version 1.0 - Sachin Birje                                                */
/*  version 1.1 - Varadraj G                                                  */ 
/*  version 1.2 - Sachin Birje, Added 3 more msg queues                       */ 
/******************************************************************************/
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
#include <fo_exg_brd_lib.h>
#include <fo_shm.h>

int i_nrml_qid;
int i_fi_qid;
int i_fs_qid;
int i_oi_qid;
int i_os_qid;
int i_othrs_qid;  /*  Added in ver 1.1 */
int i_os_qid_q; /* Added in ver 1.1 */
int i_othrs_qid1;  /** Ver 1.2 **/
int i_othrs_qid2;  /** Ver 1.2 **/
int i_othrs_qid3;  /** Ver 1.2 **/
char sql_c_pipe_id [ 3 ];

int fn_read_debug_flg(char * cptr_prcs_name); /* Debug Level */

struct st_tkn_val *st_tkn;

int tpsvrinit ( int argc, char * argv[] )
{

		int   i_ch_val;
    int   i_msgq_typ;
    int   i_ret_cd;

    long l_temp_tot_count;

		char  *ptr_c_tmp;
		char  ptr_c_fileName [512];
		char 	c_ServiceName [ 33 ];
  	char 	c_err_msg [ 256 ];	
    char  c_ini_name[4];

		strcpy(c_ServiceName, "SFO_BRDLZO_ROUT" );

    userlog("Inside fo_brd_server");
		i_ch_val = 0;
	
    strcpy(c_ini_name,"BRD");
 
		sprintf(ptr_c_fileName, "%s/%s.ini", getenv("BIN"), c_ini_name);

    userlog(" ptr_c_fileName :%s:",ptr_c_fileName);
		i_ch_val = fn_create_pipe ( );

		if ( i_ch_val == -1 )
  	{
    	userlog ( "EBA : Error creating pipe" );
    	return ( -1 );
  	}	


		if ( tpopen( ) == -1 )
  	{
    	userlog ( "EBA : RM failed in opening connection |%s|", TPMSG );
    	fn_destroy_pipe();
    	return ( -1 );
  	}


		/*** Initialize Process Space  ***/
	
	 	i_ch_val = fn_init_prcs_spc( "SFO_BRDLZO_ROUT",
                                 ptr_c_fileName,
                                 "SFO_BRDLZO_ROUT" );
  	if (i_ch_val == -1)
  	{
    	userlog ( "EBA : Initializing Process space is failed " );
    	fn_destroy_pipe();
    	return ( -1 );
  	}
	
		/***** Taking access of Transmit queque. *****/	

		ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"NORMAL_QUEUE");
    i_msgq_typ = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName," NORMAL_QUEUE :%d:",i_msgq_typ);
    userlog(" ptr_c_fileName :%s:",ptr_c_fileName);

		if(fn_crt_msg_q(&i_nrml_qid, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
		{
			userlog("EBA : Failed to take access of Transmit queque.");		
    	fn_destroy_pipe();
    	return ( -1 );
		}
   /********** Ver 1.1 starts **********/

    ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OTHRS_QUEUE");
    i_msgq_typ = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName," OTHRS_QUEUE :%d:",i_msgq_typ);
    userlog(" ptr_c_fileName :%s:",ptr_c_fileName);

    if(fn_crt_msg_q(&i_othrs_qid, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
    {
     userlog("EBA : Failed to take access of Transmit queque.");
     fn_destroy_pipe();
     return ( -1 );
    }

   /********** Ver 1.1 Ends ************/

   /**** Ver 1.2 Starts here ***/
    ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OTHRS_QUEUE1");
    i_msgq_typ = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName," OTHRS_QUEUE :%d:",i_msgq_typ);
    userlog(" ptr_c_fileName :%s:",ptr_c_fileName);

    if(fn_crt_msg_q(&i_othrs_qid1, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
    {
     userlog("EBA : Failed to take access of Transmit queque.");
     fn_destroy_pipe();
     return ( -1 );
    }

    ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OTHRS_QUEUE2");
    i_msgq_typ = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName," OTHRS_QUEUE :%d:",i_msgq_typ);
    userlog(" ptr_c_fileName :%s:",ptr_c_fileName);

    if(fn_crt_msg_q(&i_othrs_qid2, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
    {
     userlog("EBA : Failed to take access of Transmit queque.");
     fn_destroy_pipe();
     return ( -1 );
    }

    ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OTHRS_QUEUE3");
    i_msgq_typ = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName," OTHRS_QUEUE :%d:",i_msgq_typ);
    userlog(" ptr_c_fileName :%s:",ptr_c_fileName);

    if(fn_crt_msg_q(&i_othrs_qid3, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
    {
     userlog("EBA : Failed to take access of Transmit queque.");
     fn_destroy_pipe();
     return ( -1 );
    }


   /**** Ver 1.2 Ends Here ***/

    ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"FUTIDX_QUEUE");
    i_msgq_typ = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName," FUTIDX_QUEUE :%d:",i_msgq_typ);
    if(fn_crt_msg_q(&i_fi_qid, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
    {
      userlog("EBA : Failed to take access of Transmit queque.");
      fn_destroy_pipe();
      return ( -1 );
    }

    ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"FUTSTK_QUEUE");
    i_msgq_typ = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName," FUTSTK_QUEUE :%d:",i_msgq_typ);
    if(fn_crt_msg_q(&i_fs_qid, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
    {
      userlog("EBA : Failed to take access of Transmit queque.");
      fn_destroy_pipe();
      return ( -1 );
    }

    ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OPTIDX_QUEUE");
    i_msgq_typ = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName," OPTIDX_QUEUE :%d:",i_msgq_typ);
    if(fn_crt_msg_q(&i_oi_qid, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
    {
      userlog("EBA : Failed to take access of Transmit queque.");
      fn_destroy_pipe();
      return ( -1 );
    }

    ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OPTSTK_QUEUE"); 
    i_msgq_typ = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName," OPTSTK_QUEUE :%d:",i_msgq_typ);
    if(fn_crt_msg_q(&i_os_qid, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
    {
      userlog("EBA : Failed to take access of Transmit queque.");
      fn_destroy_pipe();
      return ( -1 );
    }
    /*********************** Added in Ver 1.1 ************************/
    ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OPTSTK_QUEUE_Q");
    i_msgq_typ = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName," OPTSTK_QUEU_Q :%d:",i_msgq_typ);
    if(fn_crt_msg_q(&i_os_qid_q, i_msgq_typ, CLIENT, c_ServiceName, c_err_msg) == -1)
    {
      userlog("EBA : Failed to take access of Transmit queque.");
      fn_destroy_pipe();
      return ( -1 );
    }
    /************************ Ver 1.1 Ends **************************/
    fn_userlog(c_ServiceName," before get shmid");

    i_ret_cd = fn_get_shm_id(CLIENT,c_ServiceName,0,c_err_msg);

    if( i_ret_cd == -1)
    {
      fn_errlog( c_ServiceName, "S31030","", c_err_msg );
      return -1;
    }

    i_ret_cd = fn_attach_shm((void *)&st_tkn,c_ServiceName,c_err_msg);
    if( i_ret_cd == -1)
    {
     fn_errlog( c_ServiceName, "S31030","", c_err_msg );
     return -1;
    }

		fn_read_debug_flg(argv[0]); /* Debug Level */

		return ( 0 );

}

void tpsvrdone ( )
{

		char c_err_msg [ 256 ];
		if ( tpclose ( ) == -1 )
		{
			userlog ( "EBA : RM failed in closing connection |%s|", TPMSG );
		}
		fn_destroy_pipe ( );

}
