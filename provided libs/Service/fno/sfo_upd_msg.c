/******************************************************************************/
/*  Program           : SFO_UPD_MSG.PC                                        */
/*                                                                            */
/*  Input             : ptr_st_mkt_msg_i                                      */
/*                                                                            */
/*  Output            : ptr_st_mkt_msg_i                                      */
/*                                                                            */
/*  Description       : It is a Simulator process, which reads the broadcasted*/
/*											message and updates the database after assigning the  */
/*											current system date as acknowledgment time and returns*/
/*										  the packet with acknowledgement time.									*/
/*                                                                            */
/*  Log               :                                                       */
/*                                                                            */
/******************************************************************************/
/*     Swamy Boggarapu				16-Nov-01						Ver 1.0                     */
/*     Vidyayini Krish        18-Dec-2007         Ver 1.1                     */
/******************************************************************************//*																																						*/
/***	C Header  ***/
#include <stdio.h>

/*** To be added for pro*c code ***/
#include <sqlca.h>

/*** Tuxedo Headers ***/
#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>

/*** Application Headers ***/
#include <fo_view_def.h>
#include <fo.h>
#include <fn_tuxlib.h>
#include <fn_ddr.h>
#include <fn_log.h> /* Ver 1.1 */
#include <string.h> /* Ver 1.1 */
#include <fn_read_debug_lvl.h> /* Ver 1.1 */


void SFO_UPD_MSG (TPSVCINFO *ptr_st_ti_rqst)
{
	char c_ServiceName[33];
	char c_err_msg[256];
	int i_trn_val;
	
	EXEC SQL BEGIN DECLARE SECTION;
 		struct vw_mkt_msg *ptr_st_mkt_msg_i;
		varchar sql_vc_msg[255];
	EXEC SQL END DECLARE SECTION;
	
  /*** Input  and  Output views for the Service.  ***/

	/*** Error Structure   ***/
	struct vw_err_msg  *ptr_st_err_msg; 

	strcpy(c_ServiceName, "SFO_UPD_MSG");
	INITDBGLVL(c_ServiceName); 

	/*** Reading Input Buffer into the input structure ***/
	ptr_st_mkt_msg_i = (struct vw_mkt_msg *)ptr_st_ti_rqst->data;

	/*** Allocate the error Structure.  ***/
	ptr_st_err_msg = (struct vw_err_msg *)tpalloc("VIEW32","vw_err_msg",
																								 sizeof(struct vw_err_msg));
	if(ptr_st_err_msg == NULL)
	{
		fn_errlog (c_ServiceName, "S31005", TPMSG, c_err_msg);
		tpreturn( TPFAIL, NO_BFR, NULL, 0,0 );
	} 						  

  /*** Added for Order Routing ***/
  fn_init_ddr_val ( ptr_st_mkt_msg_i->c_rout_crt );
	 
  strcpy( (char *)sql_vc_msg.arr, ptr_st_mkt_msg_i->c_msg);
	SETLEN(sql_vc_msg);

	i_trn_val = fn_begintran ( c_ServiceName, ptr_st_err_msg->c_err_msg );
	if ( i_trn_val == -1 )
	{
		fn_errlog(c_ServiceName, "S31010", LIBMSG, ptr_st_err_msg->c_err_msg);
		tpreturn( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0,0 );
	}

	/*** Inserting a record into the FTM_FO_TRD_MSG table with the time stamp ***/
	EXEC SQL
		INSERT INTO FTM_FO_TRD_MSG 
                ( FTM_XCHNG_CD,
									FTM_BRKR_CD,
									FTM_MSG_ID,
									FTM_MSG,
									FTM_TM )
		VALUES (:ptr_st_mkt_msg_i->c_xchng_cd,
						:ptr_st_mkt_msg_i->c_brkr_id,
						:ptr_st_mkt_msg_i->c_msg_id,
						:sql_vc_msg,
						to_date ( :ptr_st_mkt_msg_i->c_tm_stmp, 'dd-Mon-yyyy hh24:mi:ss') );
	if (SQLCODE != 0 )
	{
		fn_errlog(c_ServiceName, "S31015", SQLMSG, ptr_st_err_msg->c_err_msg);
		fn_aborttran ( c_ServiceName, i_trn_val, ptr_st_err_msg->c_err_msg );
		tpreturn( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0,0 );
	}
 	
	i_trn_val = fn_committran ( c_ServiceName, i_trn_val, 
                              ptr_st_err_msg->c_err_msg );
	if ( i_trn_val ==  -1 )
	{
		tpreturn( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0,0 );
	}

	/*** Free the error structure ***/
	tpfree((char *)ptr_st_err_msg);	

	/*** Returning the output View ***/
	tpreturn(TPSUCCESS,SUCC_BFR,(char *)ptr_st_mkt_msg_i,0,0);
}
