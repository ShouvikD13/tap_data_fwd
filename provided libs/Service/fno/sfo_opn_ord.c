/******************************************************************************/
/*	Program	    			:	SFO_OPN_ORD                                           */
/*                                                                            */
/*  Input             : vw_orderbook                                          */
/*                                                                            */
/*  Output            :                                                       */
/*                                                                            */
/*  Description       : To update the open order details in database          */
/*                                                                            */
/*  Log               : 1.0   16-Jan-2002   Sridhar.T.N                       */
/*                                                                            */
/******************************************************************************/
/*	1.0 16-Jan-2002			New Release																						*/
/*  1.1	22-Jan-2004			Changes to pick up trader id from opm_ord_pipe_mstr		*/
/*                      instead of exg_xchng_mstr															*/
/*	1.2	19-Dec-2007			IBM Migration	(SKS)																		*/
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <atmi.h>
#include <sqlca.h>
#include <fml32.h>
#include <fo.h>
#include <fo_fml_def.h>
#include <fo_view_def.h>
#include <fn_tuxlib.h>
#include <fn_ddr.h>
#include <fn_log.h>             /*  Ver 1.2 : Added */
#include <fn_read_debug_lvl.h>  /*  Ver 1.2 : Added */

void SFO_OPN_ORD( TPSVCINFO *rqst )
{
	char c_ServiceName[ 33];
	char c_err_msg[256];
	char c_msg[256];

	struct vw_orderbook st_i_ordrbk;

	int i_ch_val;
	time_t ud_tm_stmp;

	EXEC SQL BEGIN DECLARE SECTION;
		struct vw_orderbook *ptr_st_orderbook;
		struct vw_mkt_msg st_mktmsg;
	EXEC SQL END DECLARE SECTION;

	struct vw_err_msg *ptr_st_err_msg;

	ptr_st_orderbook = ( struct vw_orderbook *)rqst->data;
	strcpy( c_ServiceName, rqst->name );
	INITDBGLVL(c_ServiceName);            /*  Ver 1.2 : Added   */

	ptr_st_err_msg = ( struct vw_err_msg * ) tpalloc ( "VIEW32",
                                                     "vw_err_msg",
                                                sizeof ( struct vw_err_msg ) );
	if ( ptr_st_err_msg ==  NULL )
	{
		fn_errlog ( c_ServiceName, "S31005", TPMSG, c_err_msg );
		tpreturn ( TPFAIL, NO_BFR, NULL, 0, 0 );
	}

	memset ( ptr_st_err_msg, 0, sizeof ( struct vw_err_msg ) );

  /*** Added for Order Routing ***/
  fn_init_ddr_val ( ptr_st_orderbook->c_rout_crt );

  strcpy ( st_i_ordrbk.c_xchng_ack , ptr_st_orderbook->c_xchng_ack );
  strcpy ( st_i_ordrbk.c_pipe_id , ptr_st_orderbook->c_pipe_id );
  st_i_ordrbk.c_oprn_typ = FOR_VIEW;

  /*** Added for Order Routing ***/
	fn_cpy_ddr ( st_i_ordrbk.c_rout_crt );

  i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_i_ordrbk,
                           &st_i_ordrbk,
                           "vw_orderbook",
                           "vw_orderbook",
                           sizeof ( st_i_ordrbk ),
                           sizeof ( st_i_ordrbk ),
                           0,
                           "SFO_ACK_TO_ORD" );
  if ( i_ch_val != SUCC_BFR )
  {
		strcpy ( st_mktmsg.c_xchng_cd, " " );
		strcpy ( st_mktmsg.c_brkr_id, " " );
		sprintf(st_mktmsg.c_msg, "%s - EBA: NA NA NSE:%ld %ld",
								ptr_st_orderbook->c_xchng_ack,
								ptr_st_orderbook->l_ord_tot_qty, ptr_st_orderbook->l_exctd_qty);
  }
	else
	{
		fn_userlog(c_ServiceName, "|%ld|%ld|%ld|",st_i_ordrbk.l_exctd_qty,
								st_i_ordrbk.l_exctd_qty_day,ptr_st_orderbook->l_exctd_qty);
 	  if( (st_i_ordrbk.l_exctd_qty - st_i_ordrbk.l_exctd_qty_day) 
												== ptr_st_orderbook->l_exctd_qty        ) 
		{
			fn_userlog(c_ServiceName, "Order open - |%s|", st_i_ordrbk.c_ordr_rfrnc);
			tpfree ( ( char * ) ptr_st_err_msg );
/* 		tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )&st_i_ordrbk, 
                                  		sizeof( struct vw_orderbook ), 0 );	*	Ver 1.2 : Commented	*/
			/* Ver 1.2 : Addition Begins -- to avoid "Invalid data pointer given to tpreturn()" ERROR */
			memset ( ptr_st_orderbook, 0, sizeof ( struct vw_orderbook ) );
			memcpy ( ptr_st_orderbook, &st_i_ordrbk, sizeof ( struct vw_orderbook ) );
			tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_orderbook, 0, 0 );
			/* Ver 1.2 : Addition Ends	*/
		}
		strcpy ( st_mktmsg.c_xchng_cd, st_i_ordrbk.c_xchng_cd );

   	/*** Commented ver - 1.1  ***
    EXEC SQL
      SELECT EXG_TRDR_ID
      INTO :st_mktmsg.c_brkr_id
      FROM EXG_XCHNG_MSTR
      WHERE EXG_XCHNG_CD = :st_mktmsg.c_xchng_cd;
   	*** Commented ver - 1.1  ***/

   	/*** Added ver - 1.1  ***/
		EXEC SQL
				 SELECT		opm_trdr_id
				 INTO 		:st_mktmsg.c_brkr_id
				 FROM 		opm_ord_pipe_mstr
				 WHERE 		opm_xchng_cd = :st_mktmsg.c_xchng_cd
				 AND			opm_pipe_id = :ptr_st_orderbook->c_pipe_id;
   	/*** Added ver - 1.1  ***/

		if ( SQLCODE != 0 )
  	{
    	fn_errlog( c_ServiceName, "S31010", SQLMSG, c_err_msg );
    	tpreturn( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0,0 );
  	}

		sprintf(st_mktmsg.c_msg, "%s - EBA:%ld %ld NSE:%ld %ld",
								st_i_ordrbk.c_ordr_rfrnc,
								st_i_ordrbk.l_ord_tot_qty, st_i_ordrbk.l_exctd_qty,
								ptr_st_orderbook->l_ord_tot_qty, ptr_st_orderbook->l_exctd_qty);
	}

	st_mktmsg.c_msg_id = 'T'; /* Trader message */
	ud_tm_stmp = time (NULL);
	strftime(st_mktmsg.c_tm_stmp, 21, "%d-%b-%Y %H:%M:%S",localtime(&ud_tm_stmp));

  /*** Added for Order Routing ***/
	fn_cpy_ddr ( st_mktmsg.c_rout_crt );

  i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_err_msg,
                            &st_mktmsg,
                            "vw_mkt_msg",
                            sizeof (struct vw_mkt_msg ),
                            TPNOREPLY,
                            "SFO_UPD_MSG" );
  if ( i_ch_val == -1 )
  {
    fn_errlog ( c_ServiceName, "S31015", LIBMSG, c_err_msg );
    tpreturn( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0,0 );
  }

	tpfree ( ( char * ) ptr_st_err_msg );
/*tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )&st_i_ordrbk,
                                  sizeof( struct vw_orderbook ), 0 );	*	Ver 1.2 : Commented	*/
  /* Ver 1.2 : Addition Begins -- to avoid "Invalid data pointer given to tpreturn()" ERROR */
	memset ( ptr_st_orderbook, 0, sizeof ( struct vw_orderbook ) );
  memcpy ( ptr_st_orderbook, &st_i_ordrbk, sizeof ( struct vw_orderbook ) );
  tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_orderbook, 0, 0 );
  /* Ver 1.2 : Addition Ends  */
}
