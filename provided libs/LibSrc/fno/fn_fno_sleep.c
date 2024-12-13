/***********************************************************************************************
* Function name: fn_sleep_nd_call
*
* Description  : Library function to sleep for a given time(in millisec) and then
                 call the given service.
*
* Input        :
*                INPUT_BUFFER             To be passed to the called service
*                d_sleep_tm_msec          Time to sleep in milli-sec
*                i_max_call_allwd         Max no of times this fn call can be allowed
*                c_caller_rfrnc           Caller Reference
*                c_fwd_to_srvc            Service to be called after sleep
*
* Log          : 07-May-2013  1.0  New Release (Shlok Ghan)
*              : 01-Aug-2023  TOL  Tux on linux (Agam)
**********************************************************************************************/

#include <atmi.h>
#include <eba.h>
#include <userlog.h>
#include <fml_def.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <fo_fml_def.h>
#include <fn_tuxlib.h>
#include <fn_log.h>
#include <fn_ddr.h>
#include <fn_session.h>
#include <fn_read_debug_lvl.h>

/*int Z; ver TOL Tux on linux */
#define INIT(x,y)  for(int Z=0; Z< y; Z++) x[Z] =0 /* ver TOL tux on linux */
#define FML_CNT_CHK	4
#define MAX_ALLWD_SLEEP_TM_MSEC 300000 
#define MIN_ALLWD_SLEEP_TM_MSEC 0.000001
#define	SEC_TO_NANOSEC					1000000000
#define CALLER_RFRNC_MAX_LEN    30

int fn_fno_sleep( char     *c_ServiceName, 
                      FBFR32   **ptr_fml_Ibuffer, 
                      double   d_sleep_tm_msec, 
                      int      i_max_call_allwd, 
                      char     *c_caller_rfrnc,
                      char     *c_fwd_to_srvc, 
                      char     *c_errmsg)
{
	struct timespec   st_sleep_strt_tm={0,0}, st_sleep_end_tm={0,0}, st_sleep_tm={0,0};

	/* Get current time */
  clock_gettime(CLOCK_MONOTONIC, &st_sleep_strt_tm);

	int 	i_fml_cntr[FML_CNT_CHK];
	int 	i_ferr_cntr[FML_CNT_CHK];
	int		i_fml_loop = 0;
	int		i_call_no = 0;
	int		i_add_call_no = 0;
	double	d_sleep_tm_sec = 0.0;
	double	d_sleep_tm_nsec = 0.0;

	INIT(i_fml_cntr, FML_CNT_CHK);

 	if(DEBUG_MSG_LVL_3)  
  {	
		fn_userlog(c_ServiceName,"Inside fn_fno_sleep");
	}

	i_fml_cntr[0] = Foccur32((FBFR32*)(*ptr_fml_Ibuffer), FFO_OLD_RT);
	i_fml_cntr[1] = Foccur32((FBFR32*)(*ptr_fml_Ibuffer), FFO_NEW_RT);
	i_fml_cntr[2] = Foccur32((FBFR32*)(*ptr_fml_Ibuffer), FFO_CLNT_DPID);
	i_fml_cntr[3] = Foccur32((FBFR32*)(*ptr_fml_Ibuffer), FFO_SVC_NAME);
	
	fn_userlog(c_ServiceName,"Check pt  fn_fno_sleep");
	for(i_fml_loop = 0; i_fml_loop < FML_CNT_CHK ; i_fml_loop++)
  {
    if(i_fml_cntr[i_fml_loop] != 0)
    {
      fn_userlog(c_ServiceName,"FML found. The field counter is %d", i_fml_cntr);
      fn_errlog(c_ServiceName, "L31005", DEFMSG,c_err_msg);
			return -1;
		}
	}

	i_add_call_no = 0;

	if(Fget32(*ptr_fml_Ibuffer, FFO_SETLMNT_NO , 0, (char *)&i_call_no, 0) == -1)
	{
		if( Ferror32 == FNOTPRES )
		{
			fn_userlog(c_ServiceName, "FNOTPRES : This is new call" );
			i_call_no = 1;
			i_add_call_no = 1;
		}
		else
		{
    	fn_userlog(c_ServiceName,"FML for call no not found");
    	fn_errlog(c_ServiceName, "L31010", LIBMSG, c_err_msg);
	  	return -1;
		}
	}

	if( i_call_no > i_max_call_allwd )
	{
    fn_userlog(c_ServiceName,"Exceeding max call allowed for :%s: Max :%d: Call No :%d:", c_caller_rfrnc, i_max_call_allwd, i_call_no);
		fn_errlog(c_ServiceName, "L31015", LIBMSG, c_err_msg);
	  return -1;
	}

  if(d_sleep_tm_msec > (double)MAX_ALLWD_SLEEP_TM_MSEC || d_sleep_tm_msec < MIN_ALLWD_SLEEP_TM_MSEC )
  {
    fn_userlog(c_ServiceName,"Received Sleep Time :%lf: max allowed sleep time :%d: Min allwd :%lf:",
             d_sleep_tm_msec, MAX_ALLWD_SLEEP_TM_MSEC, MIN_ALLWD_SLEEP_TM_MSEC );
    fn_errlog(c_ServiceName, "L31020", LIBMSG,c_err_msg);
		return -1;
  }

	if(strlen(c_fwd_to_srvc) > CALLER_RFRNC_MAX_LEN )
	{
    fn_userlog(c_ServiceName,"Caller Reference :%s: length :%ld: exceeeds max allowed :%d:", c_fwd_to_srvc, strlen(c_fwd_to_srvc), CALLER_RFRNC_MAX_LEN );
    fn_errlog(c_ServiceName, "L31025", LIBMSG, c_err_msg);
		return -1;
	}

  /* Convert Sleep time into proper format */
  d_sleep_tm_sec = d_sleep_tm_msec/1000;                  /* Get the seconds part */
  d_sleep_tm_nsec = modf(d_sleep_tm_sec, &d_sleep_tm_sec);   /* Get the nano-sec part */
  st_sleep_tm.tv_sec = (long)d_sleep_tm_sec;
  st_sleep_tm.tv_nsec = (long)(d_sleep_tm_nsec * SEC_TO_NANOSEC);

	/* Calculate Sleep end time */
	st_sleep_end_tm.tv_sec = st_sleep_tm.tv_sec + st_sleep_strt_tm.tv_sec;
	
	fn_userlog(c_ServiceName,"Check point 2...");

	if( st_sleep_tm.tv_nsec + st_sleep_strt_tm.tv_nsec >= SEC_TO_NANOSEC )
	{
		st_sleep_end_tm.tv_sec++;
		st_sleep_end_tm.tv_nsec = (st_sleep_tm.tv_nsec + st_sleep_strt_tm.tv_nsec) - SEC_TO_NANOSEC;
	}
	else
	{
		st_sleep_end_tm.tv_nsec = st_sleep_tm.tv_nsec + st_sleep_strt_tm.tv_nsec;
	}


	INIT(i_fml_cntr, FML_CNT_CHK);
	INIT(i_ferr_cntr, FML_CNT_CHK);

  i_fml_cntr[0]  = Fadd32(*ptr_fml_Ibuffer, FFO_OLD_RT, (char *)&st_sleep_end_tm.tv_sec, 0);
  i_ferr_cntr[0] = Ferror32;
  i_fml_cntr[1]  = Fadd32(*ptr_fml_Ibuffer, FFO_NEW_RT, (char *)&st_sleep_end_tm.tv_nsec, 0);
  i_ferr_cntr[1] = Ferror32;
	i_fml_cntr[2]  = Fadd32(*ptr_fml_Ibuffer, FFO_CLNT_DPID, (char *)c_caller_rfrnc, 0);
	i_ferr_cntr[2] = Ferror32;
  i_fml_cntr[3]  = Fadd32(*ptr_fml_Ibuffer, FFO_SVC_NAME, (char *)c_fwd_to_srvc, 0);
  i_ferr_cntr[3] = Ferror32;
	
  for(i_fml_loop = 0; i_fml_loop < FML_CNT_CHK ; i_fml_loop++)
  {
    if(i_fml_cntr[i_fml_loop] == -1)
    {
      fn_userlog(c_ServiceName,"Error while Adding FML. The field counter is %d", i_fml_cntr);
      fn_errlog(c_ServiceName, "L31030", Fstrerror32(i_ferr_cntr[i_fml_loop]), c_err_msg);
      return -1;
    }
  }

	if( i_add_call_no == 1 )
	{
		if( Fadd32(*ptr_fml_Ibuffer, FFO_SETLMNT_NO , (char *)&i_call_no, 0) == -1 )
		{
      fn_userlog(c_ServiceName,"Error while Adding FML FFO_SETLMNT_NO");
      fn_errlog(c_ServiceName, "L31035", LIBMSG, c_err_msg);
      return -1;
		}
	}
	if(DEBUG_MSG_LVL_0) 
  {
		fn_userlog(c_ServiceName, "Calling Sleep service for :%s:", c_caller_rfrnc );
	}

	if ( tpacall("SFO_SLEEP", (char *)(*ptr_fml_Ibuffer),  0, TPNOTRAN|TPNOREPLY) == -1)
	{
    fn_userlog(c_ServiceName,"Error while calling service SFO_SLEEP");
    fn_errlog(c_ServiceName, "L31040", TPMSG, c_err_msg);
    return -1;
	}
	

	return 0;
}
