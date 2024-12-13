/******************************************************************************/
/*  Program           : SFO_FOS_TRG			 			                                */
/*                                                                            */
/*  Output            :                     		 							      					*/
/*                                                                            */
/*  Description       : This Service Posts a Trigger								          */
/*                                                                            */
/*  Log               : 19-Sep-2003  1.0  Sangeet 														*/
/*										:	20-Dec-2007	 1.1  Ranjit	IBM Migration 							*/
/******************************************************************************/

#include <stdio.h>      /* C Header File                */
#include <stdlib.h>     /* C Header File                */
#include <string.h>			/* C Header File								*/
#include <atmi.h>       /* TUXEDO Header File           */
#include <userlog.h>    /* TUXEDO Header File           */
#include <fml32.h>      /* TUXEDO Header File           */
#include <Usysflds.h>   /* TUXEDO Field Definitions     */
#include <fml_def.h> /* created for FML handling     */
#include <fo_fml_def.h> /* created for FML handling     */
#include <fo.h>
#include <fn_tuxlib.h>
#include <fn_log.h>     /* ver 1.1 */
#include <fml_rout.h>		/* ver 1.1 */
#include <fn_read_debug_lvl.h>	/* ver 1.1 */

char c_ServiceName[33];

void SFO_FOS_TRG( TPSVCINFO *rqst )
{

  /* Tuxedo specific variables                        */
  FBFR32  *ptr_fml_Ibuf;     /* Pointer to FML Input buffer    */

  /* Application specific variables                   */
	char c_trg_nm[256];
	char c_trg_dat[256];
	char c_filter[256];
	char c_dmn_nm[33];
	char c_err_msg[256];
  int i_returncode;

  char c_ServiceName [ 33 ];

	/* Initialisation of Variables */

  ptr_fml_Ibuf = (FBFR32 *)rqst->data;
  strcpy(c_ServiceName, rqst->name);

	INITDBGLVL(c_ServiceName);	/* ver 1.1 */	

  i_returncode = fn_unpack_fmltovar ( c_ServiceName,
                                      c_err_msg,
                                      (FBFR32 *)ptr_fml_Ibuf,
                                      4,
                                      FFO_DMN_NM, (char *)c_dmn_nm,NULL,
                                      FFO_TRG_NM, (char *)c_trg_nm,NULL,
                                      FFO_ABT_TRG_DATA, (char *)c_trg_dat,NULL,
                                      FFO_ABT_FILTER, (char *)c_filter,NULL);
	if ( i_returncode != 0 )
	{
	  fn_errlog( c_ServiceName, "S31005", LIBMSG, c_err_msg  );
    Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, c_err_msg, 0 );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
	}

	/* Post the Trigger */
	if(strcmp(c_dmn_nm, "FNO") == 0)
	{
		fn_pst_trg ( (char *)c_ServiceName, (char *)c_trg_nm, 
								(char *)c_trg_dat, (char *)c_filter );
	}
	else
	{
	  fn_errlog( c_ServiceName, "S31010", "Invalid domain Name", c_err_msg  );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
	}

  tpreturn( TPSUCCESS, SUCC_BFR, (char *)ptr_fml_Ibuf, 0 , 0 );
}
