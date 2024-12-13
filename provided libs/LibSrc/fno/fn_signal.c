/******************************************************************************/
/*  Program           : fn_signal.c                                           */
/*                                                                            */
/*  Input             :  -                                                    */
/*                                                                            */
/*  Output            :  -                                                    */
/*                                                                            */
/*  Description       :  Library functions related to Signals 		            */
/*                                                                            */
/*  Log               :                                                       */
/*                                                                            */
/******************************************************************************/
/* Sridhar.T.N   version1.0   06-Nov-01                                       */
/******************************************************************************/

#include <signal.h>
#include <fo.h>
#include <errno.h>
/*#include <eba.h>	* Ver 1.1 AIX Migration Rohit */  
#include <fn_log.h> /* Ver 1.1 AIX Migration Rohit */
#include <string.h>	/* ver 1.1 AIX Migration Rohit */

/******************************************************************************/
/*  To set a signal handler function for a signal                             */
/*  INPUT PARAMETERS                                                          */
/*    i_signo             -   signal for which handler is to be assigned      */
/*    sig_hdlr            -   pointer to a handler function                   */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_set_sgn_hnd( int i_sig_no,
                    void (*sig_hdlr) (int),
                    char *ptr_c_serviceName,
                    char *ptr_c_errmsg)
{
  struct sigaction st_act;
  struct sigaction st_oact;

	/*    Set the signal handler function   */
  st_act.sa_handler = sig_hdlr;
  sigemptyset ( &st_act.sa_mask );
  st_act.sa_flags = SA_ONSTACK | SA_SIGINFO;  

  if (sigaction ( i_sig_no, &st_act, &st_oact) == -1)
  {
    fn_errlog(ptr_c_serviceName, "L31005", UNXMSG, ptr_c_errmsg);
    return -1;
  }
  return 0;
}
