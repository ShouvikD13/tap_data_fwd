/******************************************************************************/
/*	Program	    			:	SFO_UPD_XSTTS                                         */
/*                                                                            */
/*  Input             : vw_xchngstts                                          */
/*                                                                            */
/*  Output            : vw_xchngstts                                          */
/*  										vw_err_msg                                            */
/*                                                                            */
/*  Description       : This service updates the status of a exchange master  */
/*                      based on the type                                     */
/*                      1.Update status of participant                        */
/*                      2.Update status of broker                             */
/*                      3.Update status of exercise market                    */
/*                      4.Update status of normal market                      */
/*                                                                            */
/*  Log               : 1.0 	21-Nov-2001		A.Satheesh Kumar Reddy            */
/*                    : 1.1 	19-Dec-2007   Sushil Sharma - IBM Migration     */
/*										:	1.2		02-Jan-2009		Sandeep Patil											*/
/*										:	1.3		17-Jan-2013		Swati A.B												  */
/*                                                                            */
/******************************************************************************/
/*	1.0		-		New Release                                                     */
/*	1.1   -   IBM Migration																										*/
/*	1.2   - 	NNF Changes For Extended Market Hours														*/
/*	1.3   - 	CR-Overnight Orders at 3:30																			*/
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atmi.h>
#include <fml32.h>
#include <sqlca.h>
#include <fo.h>
#include <fo_view_def.h>
#include <fn_tuxlib.h>
#include <fn_ddr.h>
#include <fn_log.h>             /*  Ver 1.1 : Added */
#include <fn_read_debug_lvl.h>  /*  Ver 1.1 : Added */

void SFO_UPD_XSTTS( TPSVCINFO *rqst )
{
  char  c_ServiceName[33];
  char  c_errmsg[256];
	int		i_trnsctn;
  char  c_raise_trigger_type [30];
	char  c_filter[32];

  EXEC SQL BEGIN DECLARE SECTION;
    struct vw_xchngstts *ptr_st_xchngstts;
		int 	 i_record_exists;
	  char 	 c_opm_pipe_id[3];
  EXEC SQL END DECLARE SECTION;

  struct vw_err_msg *ptr_st_err_msg;

  ptr_st_xchngstts = ( struct vw_xchngstts * )rqst->data;
  strcpy( c_ServiceName, "SFO_UPD_XSTTS" );
	INITDBGLVL(c_ServiceName);            /*  Ver 1.1 : Added   */
	strcpy(c_raise_trigger_type, "\0");

  ptr_st_err_msg = ( struct vw_err_msg * ) tpalloc ( "VIEW32",
                                                     "vw_err_msg",
                                                sizeof ( struct vw_err_msg ) );
  if ( ptr_st_err_msg ==  NULL )
  {
    fn_errlog ( c_ServiceName, "S31005", TPMSG, c_errmsg );
    tpreturn ( TPFAIL, NO_BFR, NULL, 0, 0 );
  }

  memset ( ptr_st_err_msg, 0, sizeof ( struct vw_err_msg ) );

  /*** Added for Order Routing ***/
  fn_init_ddr_val ( ptr_st_xchngstts->c_rout_crt );

	i_trnsctn = fn_begintran( c_ServiceName, c_errmsg );
	if ( i_trnsctn == -1 )
	{
		strcpy ( ptr_st_err_msg->c_err_msg, c_errmsg );
		tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
	}

	switch( ptr_st_xchngstts->c_rqst_typ ) 
	{
		case UPD_PARTICIPANT_STTS:	
			 i_record_exists = 0;
			 EXEC SQL
					  SELECT	1 
					  INTO		:i_record_exists
						FROM		EXG_XCHNG_MSTR
						WHERE	  EXG_XCHNG_CD = :ptr_st_xchngstts->c_xchng_cd
						AND			EXG_SETTLOR_ID  = :ptr_st_xchngstts->c_settlor;
			 if ( ( SQLCODE != 0 ) &&
            ( SQLCODE != NO_DATA_FOUND ) )
			 {
					fn_errlog ( c_ServiceName, "S31010", 
																		SQLMSG, ptr_st_err_msg->c_err_msg );
					fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
    			tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
			 }

			 if ( i_record_exists == 1 )
			 {
					EXEC SQL
       			 UPDATE  EXG_XCHNG_MSTR
           	 SET  	 EXG_SETTLOR_STTS  = :ptr_st_xchngstts->c_settlor_stts
			 	 		 WHERE   EXG_XCHNG_CD      = :ptr_st_xchngstts->c_xchng_cd
           	 AND  	 EXG_SETTLOR_ID	     = :ptr_st_xchngstts->c_settlor;
					if ( SQLCODE != 0 )
      		{
        		fn_errlog ( c_ServiceName, "S31015",
                                    SQLMSG, ptr_st_err_msg->c_err_msg );
        		fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
        		tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
      		}

					if ( ptr_st_xchngstts->c_settlor_stts == SUSPEND )
					{
						strcpy ( c_raise_trigger_type , "TRG_PART_SUS");
					}
					else
					{
						strcpy ( c_raise_trigger_type , "TRG_PART_ACT");
					}
				}
				break;
			
		case UPD_BRK_LMT_EXCD:	 
			 i_record_exists = 0;
				EXEC SQL
            SELECT  1
            INTO    :i_record_exists
            FROM    EXG_XCHNG_MSTR
            WHERE   EXG_XCHNG_CD = :ptr_st_xchngstts->c_xchng_cd
            AND     EXG_BRKR_ID  = :ptr_st_xchngstts->c_brkr_id;	
			 if ( ( SQLCODE != 0 ) &&
            ( SQLCODE != NO_DATA_FOUND ) )
       	{
          fn_errlog ( c_ServiceName, "S31020",
                                    SQLMSG, ptr_st_err_msg->c_err_msg );
          fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
          tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
       	}	
				
				if ( i_record_exists == 1 )
       	{
          EXEC SQL
             UPDATE  EXG_XCHNG_MSTR
             SET     EXG_BRKR_STTS    = :ptr_st_xchngstts->c_brkr_stts
             WHERE   EXG_XCHNG_CD     = :ptr_st_xchngstts->c_xchng_cd
             AND     EXG_BRKR_ID      = :ptr_st_xchngstts->c_brkr_id;

          if ( SQLCODE != 0 )
          {
            fn_errlog ( c_ServiceName, "S31025",
                                    SQLMSG, ptr_st_err_msg->c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
            tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
          }
				  if ( ptr_st_xchngstts->c_brkr_stts == SUSPEND )
          {
            strcpy ( c_raise_trigger_type , "TRG_BRKR_SUS");
          }
          else
          {
						strcpy ( c_raise_trigger_type , "TRG_BRKR_ACT");
          }
         }
         break;

			case UPD_EXER_MKT_STTS:	
					EXEC SQL
             UPDATE  EXG_XCHNG_MSTR
             SET   	 EXG_EXRC_MKT_STTS = :ptr_st_xchngstts->c_exrc_mkt_stts
             WHERE   EXG_XCHNG_CD      = :ptr_st_xchngstts->c_xchng_cd;

          if ( SQLCODE != 0 )
          {
            fn_errlog ( c_ServiceName, "S31030",
                                    SQLMSG, ptr_st_err_msg->c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
            tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
          }
					
					if ( ptr_st_xchngstts->c_exrc_mkt_stts == EXCHANGE_OPEN )
          {
						strcpy ( c_raise_trigger_type , "TRG_EXR_OPN");
          }
          else
          {
						strcpy ( c_raise_trigger_type , "TRG_EXR_CLS");
          }
			
					break;

			case UPD_NORMAL_MKT_STTS:
					EXEC SQL
             UPDATE  EXG_XCHNG_MSTR
             SET     EXG_CRRNT_STTS		 = :ptr_st_xchngstts->c_crrnt_stts
             WHERE   EXG_XCHNG_CD      = :ptr_st_xchngstts->c_xchng_cd;
					if ( SQLCODE != 0 )
          {
            fn_errlog ( c_ServiceName, "S31035",
                                    SQLMSG, ptr_st_err_msg->c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
            tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
          }

					/**** Ver 1.3 Starts ***/

					if ( ptr_st_xchngstts->c_crrnt_stts == 'X')
					{
						EXEC SQL
             UPDATE  EXG_XCHNG_MSTR
             SET     EXG_TMP_MKT_STTS	 = :ptr_st_xchngstts->c_crrnt_stts
             WHERE   EXG_XCHNG_CD      = :ptr_st_xchngstts->c_xchng_cd;

          	if ( SQLCODE != 0 )
          	{
            	fn_errlog ( c_ServiceName, "S31040", SQLMSG, ptr_st_err_msg->c_err_msg );
            	fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
            	tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
          	}
					}	
					/**** Ver 1.3 Ends ***/

					if ( ptr_st_xchngstts->c_crrnt_stts == EXCHANGE_OPEN )
          {
						strcpy ( c_raise_trigger_type , "TRG_ORD_OPN");
          }
          else
          {
						strcpy ( c_raise_trigger_type , "TRG_ORD_CLS");
          }

         break;

			/****	Ver 1.2 Added For Handling Of Extended Market	****/

			case UPD_EXTND_MKT_STTS:
					EXEC SQL
						 UPDATE	EXG_XCHNG_MSTR
					   SET		EXG_EXTND_MRKT_STTS = :ptr_st_xchngstts->c_crrnt_stts
						 WHERE  EXG_XCHNG_CD      = :ptr_st_xchngstts->c_xchng_cd;

					if ( SQLCODE != 0 )
          {
            fn_errlog ( c_ServiceName, "S31045",
                                    SQLMSG, ptr_st_err_msg->c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
            tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
          }
					
					if ( ptr_st_xchngstts->c_crrnt_stts == EXCHANGE_OPEN )
          {
            strcpy ( c_raise_trigger_type , "TRG_EXT_OPN");
          }
          else
          {
            strcpy ( c_raise_trigger_type , "TRG_EXT_CLS");
          }

         break;   	
						

			default:
					fn_errlog ( c_ServiceName, "S31050",
                                    SQLMSG, ptr_st_err_msg->c_err_msg );
          fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
          tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
	}	
		
	if ( fn_committran( c_ServiceName, i_trnsctn, c_errmsg ) == -1 )
  {
    strcpy ( ptr_st_err_msg->c_err_msg, c_errmsg );
  	tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
  }

	if ( c_raise_trigger_type[0] != '\0' )  /* VER TOL : TUX on LINUX -- Changed NULL to '\0' (Ravindra) */
	{
			fn_pst_trg ( c_ServiceName,
									 c_raise_trigger_type,
									 c_raise_trigger_type,
									 ptr_st_xchngstts->c_xchng_cd);
	}
	
	tpfree ( ( char * ) ptr_st_err_msg );
	tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_xchngstts, 0, 0 );
}

