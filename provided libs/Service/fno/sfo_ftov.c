/*****************************************************************/
/*	Program	    			:	SFO_FTOV                               	 */
/*                                                               */
/*  Input             : FFO_SVC_NAME                             */
/*                      FFO_IP_VIEW_NM                           */
/*                      FFO_OP_VIEW_NM                           */
/*                                                               */
/*  Output            :                                          */
/*                                                               */
/*  Description       :                                          */
/*                                                               */
/*  History           : ver 1.1 12-Jan-2007	AIX Migration  (SKS) */
/*                    : Ver 1.2 17-Jan-2015 VIew Changes (Sachin)*/
/*                    : Ver 1.3 03-Sep-2015 VIew Changes (Sachin)*/
/*										: Ver 1.6 17-Sep-2017 Introduced log to 	 */	
/*															check calling service.(Swapnil)	 */
/*										:	Ver 1.7 29-Nov-2018	Changes to call 		 */
/*															service SFO_UPD_XSTTS (Parag)		 */
/*                                                               */
/*****************************************************************/
#include <stdio.h>
#include <atmi.h>
#include <fml32.h>
#include <stdlib.h>
#include <string.h>
#include <fn_log.h>
#include <fo.h>
#include <fo_fml_def.h>
#include <fn_env.h>
#include <fn_tuxlib.h>
#include <fn_read_debug_lvl.h>  /*  Ver 1.1 : Added */
#include <fml_rout.h>           /*  Ver 1.2 : Added   */

void SFO_FTOV( TPSVCINFO *rqst )
{
	int i_cnt;
	int i_returncode;
	int i_err[5];
	int i_ferr[5];
	int i_ip_len;
	int i_op_len;
	char c_ServiceName[33];
	char c_svcnm[33];
	char c_inp_view[32];
	char c_out_view[32];
	char c_err_msg[256];
	char *c_tmp_ptr;
  char c_rout_crt[4]; /** Ver 1.2 **/

  long li_recvbuf=0;  /** Ver 1.2 **/

	FBFR32 *ptr_fml_Ibuf;
	FBFR32 *ptr_fml_Obuf;
	void *ptr_i;
	void *ptr_o;

	ptr_fml_Ibuf  = (FBFR32 *)rqst->data;
	ptr_fml_Obuf  = (FBFR32 *)NULL;

	strcpy( c_ServiceName, rqst->name );
	INITDBGLVL(c_ServiceName);            /*  Ver 1.1 : Added   */

	i_err[0] = Fget32( ptr_fml_Ibuf, FFO_SVC_NAME, 0, (char *)c_svcnm, 0 );
	i_ferr[0] = Ferror32;

	i_err[1] = Fget32( ptr_fml_Ibuf, FFO_IP_VIEW_NM, 0, (char *)c_inp_view, 0 );
	i_ferr[1] = Ferror32;

	i_err[2] = Fget32( ptr_fml_Ibuf, FFO_OP_VIEW_NM, 0, (char *)c_out_view, 0 );
	i_ferr[2] = Ferror32;

	for ( i_cnt=0; i_cnt <= 2; i_cnt++ )
	{
		if ( i_err[ i_cnt ] == -1 )
		{
			fn_errlog( c_ServiceName,"S31005", Fstrerror32(i_ferr[i_cnt]), c_err_msg);
/*		userlog("S31005");	**	Ver 1.1 : Commented	*/
			Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
			tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
		}
	}

  /*** Ver 1.2 Starts Here *****/
  i_err[0]  = Fget32( ptr_fml_Ibuf, FFO_ROUT_CRT, 0, (char *)c_rout_crt, 0 );
  i_ferr [0] = Ferror32;
  if ( i_err[0] == -1  && Ferror32 != FNOTPRES)
  {
    fn_errlog( c_ServiceName,"S31010", Fstrerror32(i_ferr[i_cnt]), c_err_msg);
    Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
  }
  else if( Ferror32 == FNOTPRES )
  {
    fn_userlog(c_ServiceName,"Insider FNOTPRES");
    strcpy(c_rout_crt,"*");
    Fadd32( ptr_fml_Ibuf, FFO_ROUT_CRT, c_rout_crt, 0 );
  }
  /*** Ver 1.2 Ends Here ******/

	if(DEBUG_MSG_LVL_1)  /*** Intrdoduced log in Ver 1.6 ***/		
	{
 		fn_userlog(c_ServiceName, "Service Name:%s:", c_svcnm); 
	}

 if( ( strcmp(c_svcnm,"SFO_FUT_TCONF") == 0 ) || ( strcmp(c_svcnm,"SFO_OPT_TCONF") == 0 ) || 
     ( strcmp(c_svcnm,"SFO_FUT_ACK") == 0 ) || ( strcmp(c_svcnm,"SFO_OPT_ACK") == 0 ) ||
		 ( strcmp(c_svcnm,"SFO_UPD_XSTTS") == 0 )
   ) /** Added in Ver 1.2 **/ /** ver 1.3 ** SFO_FUT_ACK , SFO_OPT_ACK **/	/** ver 1.7 ** SFO_UPD_XSTTS added **/
 {
  c_tmp_ptr = ( char * ) fn_get_prcs_spc ( c_ServiceName, c_inp_view );
  if ( c_tmp_ptr == NULL )
  {
		fn_errlog( c_ServiceName, "S31015", DEFMSG, c_err_msg);
		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
  }
  else
  {
		i_ip_len = atoi ( c_tmp_ptr );
  }

  c_tmp_ptr = ( char * ) fn_get_prcs_spc ( c_ServiceName, c_out_view );
  if ( c_tmp_ptr == NULL )
  {
		fn_errlog( c_ServiceName, "S31020", DEFMSG, c_err_msg);
		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
  }
  else
  {
		i_op_len = atoi ( c_tmp_ptr );
  }

	ptr_i = ( char * ) tpalloc ( "VIEW32",
                                c_inp_view,
                                i_ip_len );
	if ( ptr_i == NULL )
	{
		fn_errlog( c_ServiceName, "S31025", TPMSG, c_err_msg  );
		Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
	}

	ptr_o = ( char  * ) tpalloc ( "VIEW32",
                                 c_out_view,
                                 i_op_len );
	if ( ptr_o == NULL )
	{
		fn_errlog( c_ServiceName, "S31030", TPMSG, c_err_msg  );
		tpfree( (char *)ptr_i );
		Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
	}

	i_returncode = Fvftos32( ptr_fml_Ibuf, ptr_i, c_inp_view );
	if ( i_returncode == -1 )
	{
		fn_errlog( c_ServiceName, "S31035", FMLMSG, c_err_msg  );
		tpfree( (char *)ptr_i );
		tpfree( (char *)ptr_o );
		Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
	}

	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              ptr_i,
                              ptr_o,
                              c_inp_view,
                              c_out_view,
                              i_ip_len,
                              i_op_len,
                              0,
                              c_svcnm );
	if ( i_returncode != SUCC_BFR )
	{
		fn_errlog( c_ServiceName, "S31040", LIBMSG, c_err_msg  );
		tpfree( (char *)ptr_i );
		tpfree( (char *)ptr_o );
		Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
	}
                                 
	ptr_fml_Obuf = ( FBFR32 * ) tpalloc ( "FML32",
                                        0,
                                        MIN_FML_BUF_LEN  );
	if ( ptr_fml_Obuf == NULL )
	{
		fn_errlog( c_ServiceName, "S31045", TPMSG, c_err_msg  );
		tpfree( (char *)ptr_i );
		tpfree( (char *)ptr_o );
		Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
	}

	i_returncode = Fvstof32( ptr_fml_Obuf, ptr_o, FCONCAT, c_out_view );
	if ( i_returncode == -1 )
	{
		fn_errlog( c_ServiceName, "S31050", FMLMSG, c_err_msg  );
		tpfree( (char *)ptr_i );
		tpfree( (char *)ptr_o );
		tpfree( (char *)ptr_fml_Ibuf );
		Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
	}

	tpfree( (char *)ptr_i );
	tpfree( (char *)ptr_o );
 }
 else /** Ver 1.2 Starts Here **/
 {

  ptr_fml_Obuf = ( FBFR32 * ) tpalloc ( "FML32",
                                        0,
                                        MIN_FML_BUF_LEN  );
  if ( ptr_fml_Obuf == NULL )
  {
    fn_errlog( c_ServiceName, "S31055", TPMSG, c_err_msg  );
    tpfree( (char *)ptr_i );
    tpfree( (char *)ptr_o );
    Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
  }

  i_returncode = tpcall ( c_svcnm,
                       (char *)ptr_fml_Ibuf,
                       0,
                       (char **)&ptr_fml_Obuf,
                       &li_recvbuf,
                       0
                      );
   if(i_returncode == -1)
   {
    fn_errlog( c_ServiceName, "S31060", TPMSG, c_err_msg  );
    tpfree( (char *)ptr_i );
    tpfree( (char *)ptr_o );
    Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
   }
 }   /** Ver 1.2 Ends Here **/

	tpreturn(TPSUCCESS, SUCC_BFR, (char *)ptr_fml_Obuf, 0, 0 );
}

