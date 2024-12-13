/**********************************************************************/
/* Service name : SFO_NEW_ROUT                                    */
/*                                                                    */
/* Description  : This service receives input as target service name, */
/*                Type of i/p buffer content(FML/STRUCT) and routes   */
/*                the service call on tux3                            */
/*                                                                    */
/* Input        : FFO_SVC_NAME                                        */
/*                FFO_QUEUE_NAME                                      */
/*                                                                    */
/* Log          : 31-Mar-2009  1.0  New Release                       */
/* Log          : 07-Sep-2014  1.1  View to FML Changes (Sachin Birje)*/
/*                                                                    */
/**********************************************************************/

#include <string.h>
#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>
#include <fo_fml_def.h>
/** #include <fo_view_def.h> ** Ver 1.1 **/
#include <fn_log.h>
#include <fn_read_debug_lvl.h>

void SFO_NEW_ROUT( TPSVCINFO *rqst )
{
    int i_err[2];
    int i_ferr[2];
    int i;
    char c_ServiceName[33];
    char c_rqst_service[33];
    char c_buffer_typ[10];
    char c_err_msg[256];

    FBFR32  *ptr_fml_Ibuffer;

    strcpy( c_ServiceName, rqst->name );
    INITDBGLVL(c_ServiceName);

    ptr_fml_Ibuffer = ( FBFR32 * )rqst->data;

    i_err[0] = Fget32 ( ptr_fml_Ibuffer, FFO_SVC_NAME, 0, (char *) c_rqst_service, 0 );
    i_ferr[0]= Ferror32;

    i_err[1] = Fget32 ( ptr_fml_Ibuffer, FFO_QUEUE_NAME, 0, (char *) c_buffer_typ, 0 );
    i_ferr[1]= Ferror32;

    for ( i = 0; i < 2; i++ )
    {
      if ( i_err[i] == -1 )
      {
        fn_userlog( c_ServiceName, "Error while getting the input id :%d:",i);
        fn_errlog( c_ServiceName, "S31005", Fstrerror32(i_ferr[i]),c_err_msg);
        Fadd32( ptr_fml_Ibuffer, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn( TPFAIL, 0L, ( char * )ptr_fml_Ibuffer, 0L, 0 );
      } 
    }

    if( strcmp( c_buffer_typ, "FMLBUFFER") == 0 )
    {
			fn_userlog(c_ServiceName,"In FMLBUFFER");
			Fprint32(ptr_fml_Ibuffer);      /*** Temporarily added  ***/
			fn_userlog(c_ServiceName,"Calling the service :%s:",c_rqst_service);
			tpforward( c_rqst_service, ( char * )ptr_fml_Ibuffer, 0, 0 );
    }
    else if( strcmp( c_buffer_typ, "STRUCTURE") == 0 )
    {
			fn_userlog(c_ServiceName,"In STRUCTURE");
			Fprint32(ptr_fml_Ibuffer);      /*** Temporarily added  ***/
			fn_userlog(c_ServiceName,"Calling the service :%s:",c_rqst_service);
			tpforward( "SFO_FTOV", ( char * )ptr_fml_Ibuffer, 0, 0 );
    }
}
