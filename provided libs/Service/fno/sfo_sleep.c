/***********************************************************************************************
* Service name : SFO_SLEEP
*
* Description  : Service to sleep for a given time(in sec) and then 
                 forward the request to the given service.
*
* Input        : 
*                FML_ORD_DT1               Sleep till time in sec
*                FML_TRD_DT1               Sleep till time in nano sec
*                FML_USR_SNGL_US_PSSWRD    Caller Reference
*                FML_CALLED_SVC   		     Service to be called after sleep
*                FML_O_MSGTAG              Call No
*                INPUT_BUFFER        		   To be passed to the called service
*
*	Log          : 28-Sep-2021  1.0  New Release (Shlok Ghan)
**********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atmi.h>
#include <fml32.h>
#include <sqlca.h>
#include <time.h>
#include <fo_fml_def.h>
#include <fn_tuxlib.h>
#include <fn_log.h>
#include <fn_ddr.h>
#include <fn_session.h>
#include <fn_read_debug_lvl.h>
#include <math.h>

#include <errno.h>

#include <atmi.h>
#include <userlog.h>
#include <fml_def.h>
#include <eba.h>

#define TOTAL_FML 		          5
#define	MAX_ALLWD_SLEEP_TM_SEC	61000
#define SEC_TO_NANOSEC          1000000000
#define CALLER_RFRNC_MAX_LEN    30

void SFO_SLEEP(TPSVCINFO *rqst)
{
	FBFR32  *ptr_fml_Ibuffer;
	char    c_ServiceName[33];
	int			i_fml_loop = 0;
	int			i_errno = 0;
	int			i_call_no = 0;
	int			i_ret_val;
	int			i_err[TOTAL_FML];
	int			i_ferr[TOTAL_FML];
	struct timespec st_sleep_end_tm={0,0}, st_crrnt_tm={0,0}, st_calc_sleep_tm={0,0};

	char    c_call_to_srvc_nm[20+1];
	char    c_caller_rfrnc[CALLER_RFRNC_MAX_LEN+1];

	ptr_fml_Ibuffer = (FBFR32 *)rqst->data;

	strcpy(c_ServiceName, rqst->name);
	INITDBGLVL(c_ServiceName);
	

	if(DEBUG_MSG_LVL_5)
  {
		Fprint32 ( ptr_fml_Ibuffer );
	}

  MEMSET(c_call_to_srvc_nm);
  MEMSET(c_caller_rfrnc);

	INIT(i_err, TOTAL_FML);
	INIT(i_ferr, TOTAL_FML);
	
	i_err[0]  = Fget32(ptr_fml_Ibuffer, FFO_OLD_RT, 0, (char *)&st_sleep_end_tm.tv_sec, 0);
	i_ferr[0] = Ferror32;
	i_err[1]  = Fget32(ptr_fml_Ibuffer, FFO_NEW_RT, 0, (char *)&st_sleep_end_tm.tv_nsec, 0);
	i_ferr[1] = Ferror32;
	i_err[2]  = Fget32(ptr_fml_Ibuffer, FFO_CLNT_DPID, 0, (char *)c_caller_rfrnc, 0);
	i_ferr[2] = Ferror32;
	i_err[3]  = Fget32(ptr_fml_Ibuffer, FFO_SVC_NAME, 0, (char *)c_call_to_srvc_nm, 0);
	i_ferr[3] = Ferror32;
	i_err[4]  = Fget32(ptr_fml_Ibuffer, FFO_SETLMNT_NO, 0, (char *)&i_call_no, 0);
	i_ferr[4] = Ferror32;

	for(i_fml_loop = 0; i_fml_loop < TOTAL_FML ; i_fml_loop++)
	{
  	if(i_err[i_fml_loop] == -1)
  	{
    	fn_userlog(c_ServiceName,"The field number is %d", i_fml_loop);
    	fn_errlog(c_ServiceName, "S31005", Fstrerror32(i_ferr[i_fml_loop]), c_err_msg);
    	Fadd32(ptr_fml_Ibuffer, FML_ERR_MSG, c_err_msg, 0);
    	tpreturn(TPFAIL, 0, (char *)ptr_fml_Ibuffer, 0, 0);
  	}
	}
	

	rtrim(c_call_to_srvc_nm);
	rtrim(c_caller_rfrnc);


	if(DEBUG_MSG_LVL_3)
  {
		fn_userlog(c_ServiceName,"Inputs to SFO_SLEEP are ");
		fn_userlog(c_ServiceName,"Sleep End time :%ld: sec, :%ld: nano sec", st_sleep_end_tm.tv_sec, st_sleep_end_tm.tv_nsec);
		fn_userlog(c_ServiceName,"Calling service name :%s: for caller rfrnc :%s: call no :%d:", 
             c_call_to_srvc_nm, c_caller_rfrnc, i_call_no);
	}

	if( strcmp(c_call_to_srvc_nm, c_ServiceName) == 0 )
	{
		fn_userlog(c_ServiceName,"Recursive Calls Not allowed");
   	fn_errlog(c_ServiceName, "S31010", DEFMSG, c_err_msg);
   	Fadd32(ptr_fml_Ibuffer, FML_ERR_MSG, c_err_msg, 0);
   	tpreturn(TPFAIL, 0, (char *)ptr_fml_Ibuffer, 0, 0);
	}

	/* Get current time */
	clock_gettime(CLOCK_MONOTONIC, &st_crrnt_tm);

  fn_userlog(c_ServiceName,"Current time :%ld: sec, :%ld: nano sec", st_crrnt_tm.tv_sec, st_crrnt_tm.tv_nsec);

	/* Calc the diff between End tm & current tm */
	st_calc_sleep_tm.tv_sec = st_sleep_end_tm.tv_sec - st_crrnt_tm.tv_sec;
	
	if(DEBUG_MSG_LVL_3)
  {
		fn_userlog(c_ServiceName,"Current time :%ld: sec", st_calc_sleep_tm.tv_sec);
	}

	if( ( st_calc_sleep_tm.tv_sec > 0 || ( st_calc_sleep_tm.tv_sec == 0 && st_sleep_end_tm.tv_nsec >= st_crrnt_tm.tv_nsec )) && st_calc_sleep_tm.tv_sec <= MAX_ALLWD_SLEEP_TM_SEC )
	{
		if( st_sleep_end_tm.tv_nsec < st_crrnt_tm.tv_nsec )
		{
			st_calc_sleep_tm.tv_sec -= 1;
			st_calc_sleep_tm.tv_nsec = SEC_TO_NANOSEC + st_sleep_end_tm.tv_nsec - st_crrnt_tm.tv_nsec;
		}
		else
		{
			st_calc_sleep_tm.tv_nsec = st_sleep_end_tm.tv_nsec - st_crrnt_tm.tv_nsec;
		}

  	if(DEBUG_MSG_LVL_3)
    {
		fn_userlog(c_ServiceName, "Sleeping for :%ld: sec :%ld: nano sec for caller rfrnc :%s:", 
 	           st_calc_sleep_tm.tv_sec, st_crrnt_tm.tv_nsec, c_caller_rfrnc );
		}

		i_ret_val = nanosleep(&st_calc_sleep_tm, NULL);
		if( i_ret_val == -1 )
		{
			i_errno = errno;
			fn_userlog(c_ServiceName, "Error while sleeping. errno is :%d:", errno );
 	   	fn_errlog(c_ServiceName, "S31015", UNXMSG , c_err_msg);
			if(i_errno == EINTR)
				fn_userlog(c_ServiceName,"Interruption Received" );
			if(i_errno == EINVAL)
				fn_userlog(c_ServiceName,"Invalid nano second value");
 	   	Fadd32(ptr_fml_Ibuffer, FML_ERR_MSG, c_err_msg, 0);
 	   	tpreturn(TPFAIL, 0, (char *)ptr_fml_Ibuffer, 0, 0);
		}
	
		if(DEBUG_MSG_LVL_3)
  	{
			fn_userlog(c_ServiceName, "Sleeping Over for caller rfrnc :%s: i_ret_val :%d: Call No :%d:", 
   	         c_caller_rfrnc, i_ret_val, ++i_call_no );
		}

	}
	else if( st_calc_sleep_tm.tv_sec > (long)MAX_ALLWD_SLEEP_TM_SEC )
	{
    fn_userlog(c_ServiceName,"Received Sleep Time :%ld: exceed max allowed sleep time :%d: for :%s:",
            st_calc_sleep_tm.tv_sec, MAX_ALLWD_SLEEP_TM_SEC, c_caller_rfrnc);
   	fn_errlog(c_ServiceName, "S31020", DEFMSG, c_err_msg);
   	Fadd32(ptr_fml_Ibuffer, FML_ERR_MSG, c_err_msg, 0);
   	tpreturn(TPFAIL, 0, (char *)ptr_fml_Ibuffer, 0, 0);
	}
	else
	{
		fn_userlog(c_ServiceName, "Sleeping Not Required at call no :%d: for caller rfrnc :%s:", ++i_call_no, c_caller_rfrnc );
	}

	INIT(i_err, TOTAL_FML);
	INIT(i_ferr, TOTAL_FML);
	
	i_err[0]  = Fdel32(ptr_fml_Ibuffer, FFO_OLD_RT, 0);
	i_ferr[0] = Ferror32;
	i_err[1]  = Fdel32(ptr_fml_Ibuffer, FFO_NEW_RT, 0);
	i_ferr[1] = Ferror32;
	i_err[2]  = Fdel32(ptr_fml_Ibuffer, FFO_CLNT_DPID, 0);
	i_ferr[2] = Ferror32;
	i_err[3]  = Fdel32(ptr_fml_Ibuffer, FFO_SVC_NAME, 0);
	i_ferr[3] = Ferror32;
	i_err[4]  = Fchg32(ptr_fml_Ibuffer, FFO_SETLMNT_NO, 0, (char *)&i_call_no, 0);
	i_ferr[4] = Ferror32;

	for(i_fml_loop = 0; i_fml_loop < TOTAL_FML ; i_fml_loop++)
	{
  	if(i_err[i_fml_loop] == -1)
  	{
    	fn_userlog(c_ServiceName,"Error while updating FML. The field number is %d", i_fml_loop);
    	fn_errlog(c_ServiceName, "S31025", Fstrerror32(i_ferr[i_fml_loop]), c_err_msg);
    	Fadd32(ptr_fml_Ibuffer, FML_ERR_MSG, c_err_msg, 0);
    	tpreturn(TPFAIL, 0, (char *)ptr_fml_Ibuffer, 0, 0);
  	}
	}

	fn_userlog(c_ServiceName, "Calling :%s: for :%s:", c_call_to_srvc_nm, c_caller_rfrnc);

  /*	tpforward(c_call_to_srvc_nm, (char *)ptr_fml_Ibuffer, 0, 0);	*/
	if( tpacall( c_call_to_srvc_nm, (char *)ptr_fml_Ibuffer,  0, TPNOTRAN|TPNOREPLY ) == -1 )
	{
		fn_userlog(c_ServiceName,"Error in acall to :%s:", c_call_to_srvc_nm);
   	fn_errlog(c_ServiceName, "S31030", TPMSG, c_err_msg);
   	Fadd32(ptr_fml_Ibuffer, FML_ERR_MSG, c_err_msg, 0);
   	tpreturn(TPFAIL, 0, (char *)ptr_fml_Ibuffer, 0, 0);
	}

	fn_userlog(c_ServiceName, "Called :%s: for :%s:", c_call_to_srvc_nm, c_caller_rfrnc);

	tpreturn(TPSUCCESS, 0, (char *)NULL, 0, 0);
}

