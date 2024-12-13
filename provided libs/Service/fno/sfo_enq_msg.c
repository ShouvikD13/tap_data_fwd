/* Service name : SFO_ENQ_MSG                                     		*/
/*                                                                		*/
/* Description  : This service write the packed data into a queue 		*/
/*                                                                		*/
/* Input        : FFO_QSPACE_NAME                                 		*/
/*							: FFO_QUEUE_NAME																			*/
/*                                                                		*/
/* Output       : NONE                                            		*/
/*                                                                		*/
/* Log          : 1.0   04/06/2012   Release                      		*/
/*							:	1.1		06/12/2012	 Fprint32 Commented Sandeep P.		*/
/**********************************************************************/

#include <atmi.h>   
#include <userlog.h>
#include <fml32.h> 
#include <fo_fml_def.h> 
#include <odin_comn.h>
#include <eba.h> 
#include <fn_log.h> 
#include <fn_read_debug_lvl.h> 

void SFO_ENQ_MSG ( TPSVCINFO *rqst )
{
	FBFR32 *ptr_fml_ibuf;
  TPQCTL st_eqctl;
  char   c_ServiceName [ 33 ]; 
  FILE   *ptr_F_mktevt;
  time_t ud_tm_stmp;
  char   c_dname[10];
  char   c_pname[50];
  char   c_tm_stmp[30];
  char   *c_temp;
	int		 i_err;
  char 	 c_oreqQName[32];
  char 	 c_qspacename[32]; 

	ptr_fml_ibuf = ( FBFR32 * ) rqst->data;
	strcpy ( c_ServiceName,"SFO_ENQ_MSG" );
	INITDBGLVL(c_ServiceName);

	/**Fprint32(ptr_fml_ibuf);			Commented In Ver 1.1	***/

	fn_userlog(c_ServiceName,"Inside ENQ MSG Service.");

  i_err = Fget32(ptr_fml_ibuf,FFO_QSPACE_NAME,0,(char *)c_qspacename,0);

	if(i_err == -1)
  {
			errlog ( c_ServiceName, "S31005", FMLMSG, DEF_USR, DEF_SSSN, c_err_msg );
			tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  }

	fn_userlog(c_ServiceName,"After Fget 1.");
	fn_userlog(c_ServiceName,"c_qspacename Is :%s:.",c_qspacename);

  i_err = Fget32(ptr_fml_ibuf, FFO_QUEUE_NAME,0,(char *)c_oreqQName,0);

	if(i_err == -1)
  {
			errlog ( c_ServiceName, "S31010", FMLMSG, DEF_USR, DEF_SSSN, c_err_msg );
			tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  }

	fn_userlog(c_ServiceName,"After Fget 2.");
	fn_userlog(c_ServiceName,"c_oreqQName Is :%s:.",c_oreqQName);

	
	st_eqctl.flags = TPNOFLAGS;

	fn_userlog(c_ServiceName," BEfore Enqueue.");

	if ( tpenqueue ( c_qspacename, c_oreqQName, ( TPQCTL * )&st_eqctl, 
                   ( char * )ptr_fml_ibuf, 0, 0 ) == -1 )
	{
		errlog ( c_ServiceName, "S31015", TPMSG, DEF_USR, DEF_SSSN, c_err_msg );
			tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
	}

  tpreturn ( TPSUCCESS, 0, (char *)NULL , 0L, 0 );
}
