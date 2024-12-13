/******************************************************************************/
/*  Program       : bat_fo_errqres.c                                          */
/*                                                                            */
/*  Description   : This program dequeue responses from error queue and call 	*/
/*									Receive Client to process the same,	must be run on Tux3		*/
/*                                                                            */
/*  Input         : <configuration file> <Exchange code> <pipe_id>            */ 
/*                                                                            */
/*  Output        : Success/Failure                                           */
/*                                                                            */
/*  Log           :                                                           */
/*                                                                            */
/*  Ver 1.0   26-Jun-2009   Prakash       Initial Release CRCSN31735 ORS2     */
/*  Ver 1.1   07-Jul-2009   Prakash       Adding selective run option         */
/*  Ver 1.3   16-Dec-2016   Tanmay W      Syncing ESR_CLNT with the batch			*/
/*  Ver 1.4   22-Feb-2019   Parag Kanojia FO Direct Connectivity              */
/*  Ver 1.5   17-JUL-2020   Suchita Dabir CR_139156 changes                   */
/******************************************************************************/

/* C header files */
#include <string.h>
/* Tuxedo header files */
#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>
/* Application header files */
#include <fo.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fn_battmpt.h>
#include <fo_fml_def.h>
#include <fn_read_debug_lvl.h>

/* Ver 1.1 Starts */
#include <stdlib.h>
#include <eba_to_ors.h>
#include <fo_exch_comnN.h>
#include <fo_view_def.h>
#include <fn_ddr.h>
#include <fml_rout.h>
#include <fn_scklib_tap.h>
#define ALL_RUN 				'A'
#define SELECTIVE_RUN 	'S'
/* Ver 1.1 Ends */

/* Global variables */
char c_ServiceName[33];
char c_err_msg[256];
int i_trnsctn;
int i_ch_val;

void fn_re_enqueue( FBFR32 *, char *, char *);

int fn_selective_actn(  FBFR32 *ptr_fml_tempbuf,	/* Ver 1.1 */
                        char c_res_typ[],
                        char c_ordref_ack[],
                        long l_mod_cntr,
                        char *c_call_rcv );

void BAT_FO_ERRQRES( int argc, char *argv[])
{
  FBFR32 *ptr_fml_ibuf;
  char *ptr_car_buf;      /** Ver 1.3 **/
  TPQCTL st_dqctl;
  FLDLEN32 ud_len;					/** ver 1.3 **/

	char c_pipe_id[3];
  char c_qspacename[32];
  char c_err_qn[32];
	char c_run_opt;										/* Ver 1.1 */
  char c_call_rcv;									/* Ver 1.1 */
  char c_ordref_ack[LEN_ORDR_RFRNC];/* Ver 1.1 */
  char c_res_typ[6+1];							/* Ver 1.1 */

	int i_dwnld_flg;
	int i_deferred_dwnld_flg;
	long int li_len;
  long l_mod_cntr;									/* Ver 1.1 */
	long int li_sbuf_len;            /* Ver 1.3 */
  struct st_oe_reqres st_oe_reqres_data;    /* Ver 1.3 */
  struct st_oe_rspn_tr st_oe_rspn_tr_data;    /* Ver 1.4 */
	struct st_cmn_header_tr st_hdr;							/* Ver 1.4 */

  /*** ver 1.5 starts ***/
  struct vw_xchngbook st_i_xchngbk;
  int i_cnt=0;
  int i_ferr[3];
  int i_err[3]; 
  char c_ord_ack_nse[20]="\0";
  /*** ver 1.5 ends ***/

	INITBATDBGLVL( argv[0] );

	strcpy( c_ServiceName, argv[0] );

	strcpy( c_pipe_id, argv[3] );

	/* Ver 1.1 Starts */

  if( argc < 5 )
  {
    fn_userlog( c_ServiceName, "Invalid number of arguments Usage - %s <configuration file> <Exchange code> <pipe_id> \
                                                                      <run opt>", c_ServiceName );
    return;
  }

  c_run_opt = argv[4][0];

  if(DEBUG_MSG_LVL_3){
    fn_userlog( c_ServiceName, "c_pipe_id   |%s|", c_pipe_id );
    fn_userlog( c_ServiceName, "c_run_opt   |%c|", c_run_opt );
  }

  if( c_run_opt == SELECTIVE_RUN )
  {
    strcpy( c_res_typ, argv[5] );
    fn_userlog( c_ServiceName, "c_res_typ   |%s|", c_res_typ );

    if( strcasecmp(c_res_typ, "ordcon") == 0 )
    {
      if (argc != 8)
      {
        fn_userlog( c_ServiceName, "Invalid number of arguments Usage - %s <configuration file> <Exchange code> <pipe_id> \
                                          <run opt> <res typ> <ordref/ack> <mod cntr>", c_ServiceName );
        return;
      }
      l_mod_cntr = atol( argv[7] );
      fn_userlog( c_ServiceName, "l_mod_cntr   |%ld|", l_mod_cntr );
    }
    else
    {
      if (argc != 7)
      {
        fn_userlog( c_ServiceName, "Invalid number of arguments Usage - %s <configuration file> <Exchange code> <pipe_id> \
                                          <run opt> <res typ (trdcon)> <ordref/ack>", c_ServiceName );
        return;
      }
    }

    strcpy( c_ordref_ack, argv[6] );
    fn_userlog( c_ServiceName, "c_ordref_ack   |%s|", c_ordref_ack );
  }
  /* Ver 1.1 Ends */

	/* error qname, qspace name */
  sprintf(c_err_qn,"ERR_%s_Q",c_pipe_id);
  sprintf(c_qspacename,"%s_QSPACE",c_pipe_id);

	if(DEBUG_MSG_LVL_3){
    fn_userlog( c_ServiceName, "c_err_qn   |%s|", c_err_qn );
    fn_userlog( c_ServiceName, "c_qspacename   |%s|", c_qspacename );
  }

	/* allocating input/output fml buffers */
  ptr_fml_ibuf = (FBFR32 *)tpalloc( "FML32", NULL, 15*MIN_FML_BUF_LEN );
  if (ptr_fml_ibuf == NULL)
  {
    fn_errlog( c_ServiceName, "S31005", TPMSG, c_err_msg );
  	return;
  }

 	ptr_car_buf =(char *)tpalloc("CARRAY", NULL, MIN_FML_BUF_LEN);   /** Ver 1.3 **/
  if (ptr_car_buf == NULL)
  {
    fn_errlog( c_ServiceName, "S31010", TPMSG, c_err_msg  );
    tpfree ( (char *) ptr_fml_ibuf );
    return;
  }

	/* set control parameter to dequeue */ 
  st_dqctl.flags = TPNOFLAGS;

	/* while( 1 )
	{								commented in Ver 1.1 looping handled in shell utility */

		/* dequeueing a message from errq in transaction */
  	i_trnsctn = fn_begintran( c_ServiceName, c_err_msg );
  	if( i_trnsctn == -1 )
  	{
    	fn_errlog( c_ServiceName, "S31015", LIBMSG, c_err_msg );
    	tpfree ( (char *) ptr_fml_ibuf );
    	tpfree ( (char *) ptr_car_buf );    /** Ver 1.3 **/
    	return;
  	}

  	i_ch_val = tpdequeue ( c_qspacename, c_err_qn, &st_dqctl,
  		                     (char **)&ptr_fml_ibuf, &li_len, 0 );
  	if ( i_ch_val == -1 )
  	{
			if(DEBUG_MSG_LVL_3) {
      	fn_userlog( c_ServiceName, "ERROR IS %s", tpstrerror(tperrno) );
        fn_userlog( c_ServiceName, "%d", tperrno );
        fn_userlog( c_ServiceName, "VALUE OF DIAG is %ld", st_dqctl.diagnostic );
      }
      if(st_dqctl.diagnostic == QMENOMSG)
      {
				fn_userlog( c_ServiceName, "No message in the queue, BAT_FO_ERRQRES leaving...");
        /* break; commented in Ver 1.1 */
      }
			else 								/* else clause provision added in Ver 1.1 */
			{
  			fn_errlog( c_ServiceName, "S31020", TPMSG, c_err_msg );
			}

    	tpfree ( (char *) ptr_fml_ibuf );
    	fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
      tpfree ( (char *) ptr_car_buf );    /** Ver 1.3 **/
    	return;
  	}

  	if( fn_committran( c_ServiceName, i_trnsctn, c_err_msg ) == -1 )
  	{
    	fn_errlog( c_ServiceName, "S31025", LIBMSG, c_err_msg );
    	tpfree ( (char *) ptr_fml_ibuf );
      tpfree ( (char *) ptr_car_buf );    /** Ver 1.3 **/
    	fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
    	return;
  	}

		/* Ver 1.1 Starts */
    c_call_rcv = 'Y';

    if( c_run_opt == SELECTIVE_RUN )
    {
      i_ch_val = fn_selective_actn( ptr_fml_ibuf,
                                    c_res_typ,
                                    c_ordref_ack,
                                    l_mod_cntr,
                                    &c_call_rcv );
			if( i_ch_val == -1 )
      {
        fn_re_enqueue( ptr_fml_ibuf, c_err_qn, c_qspacename);
        fn_errlog( c_ServiceName, "S31030", DEFMSG, c_err_msg );
        tpfree ( (char *) ptr_fml_ibuf );
      	tpfree ( (char *) ptr_car_buf );    /** Ver 1.3 **/
        return;
      }
		}

		if( c_call_rcv == 'N' )
    {
      fn_userlog( c_ServiceName, "Skipping Record" );
      fn_re_enqueue( ptr_fml_ibuf, c_err_qn, c_qspacename);
      tpfree ( (char *) ptr_fml_ibuf );
      tpfree ( (char *) ptr_car_buf );    /** Ver 1.3 **/
      return;
    }
    /* Ver 1.1 Ends */

		/* hardcode dwnld_flg and deferred_dwnld_flg */
		i_dwnld_flg = DOWNLOAD;
  	if( Fchg32( ptr_fml_ibuf, FFO_BIT_FLG, 0, (char *)&i_dwnld_flg, 0 ) == -1 )
  	{
			fn_re_enqueue( ptr_fml_ibuf, c_err_qn, c_qspacename);
			fn_errlog( c_ServiceName, "S31035", FMLMSG, c_err_msg );
    	tpfree ( (char *) ptr_fml_ibuf );
      tpfree ( (char *) ptr_car_buf );    /** Ver 1.3 **/
			return;
		}

		i_deferred_dwnld_flg = 0;
  	if( Fchg32( ptr_fml_ibuf, FFO_DWNLD_FLG, 0, (char *)&i_deferred_dwnld_flg, 0 ) == -1 )
  	{
			fn_re_enqueue( ptr_fml_ibuf, c_err_qn, c_qspacename);
			fn_errlog( c_ServiceName, "S31040", FMLMSG, c_err_msg );
    	tpfree ( (char *) ptr_fml_ibuf );
      tpfree ( (char *) ptr_car_buf );    /** Ver 1.3 **/
			return;
		}
	
		/*** Ver 1.3  Starts ***/

		ud_len = ( FLDLEN32 )sizeof( li_sbuf_len );                   

  	i_ch_val = Fget32( ptr_fml_ibuf, FFO_TMPLT, 0, ( char * )&li_sbuf_len, &ud_len );
  	if( i_ch_val == -1 )
  	{
  	 fn_errlog( c_ServiceName, "S31045", FMLMSG, c_err_msg );
 	   tpfree ( ( char * )ptr_fml_ibuf );
     tpfree ( (char *) ptr_car_buf );   
 	   return;
 	  }

		ud_len = ( FLDLEN32 )li_sbuf_len;

  	i_ch_val = Fget32( ptr_fml_ibuf, FFO_CBUF, 0, ( char * )ptr_car_buf, &ud_len );
  	if( i_ch_val == -1)
  	{
  	  fn_errlog( c_ServiceName, "S31050", FMLMSG, c_err_msg );
  	  tpfree ( ( char * )ptr_fml_ibuf );
      tpfree ( (char *) ptr_car_buf ); 
  	  return;
  	}

  	/** MEMSET( st_hdr );
  	memcpy( &st_hdr, ptr_car_buf, sizeof( struct st_cmn_header ) );   ** Ver 1.3 ** to Be removed **/

		/*** Ver 1.4 Starts ***/

		MEMSET(st_oe_rspn_tr_data);
		MEMSET(st_oe_reqres_data);

		memcpy( &st_hdr, ptr_car_buf, sizeof( struct st_cmn_header_tr ) );

		if( st_hdr.si_transaction_code == ORDER_CONFIRMATION_OUT_TR || st_hdr.si_transaction_code == ORDER_MOD_CONFIRM_OUT_TR ||st_hdr.si_transaction_code == ORDER_CXL_CONFIRMATION_TR ||st_hdr.si_transaction_code == TRADE_CONFIRMATION_TR )
		{
			memcpy( &st_oe_rspn_tr_data, ptr_car_buf, sizeof( struct st_oe_rspn_tr ) );
		}
		else
		{
		/*** Ver 1.4 Ends ***/
/***     MEMSET( st_oe_reqres_data );	*** Commented in Ver 1.4 ***/
       memcpy( &st_oe_reqres_data, ptr_car_buf, sizeof( struct st_oe_reqres ) );

      if(DEBUG_MSG_LVL_3) {
		  	fn_userlog ( c_ServiceName, "st_oe_reqres_data.st_hdr.si_transaction_code is :%d:", st_oe_reqres_data.st_hdr.si_transaction_code ) ;
		  	fn_userlog ( c_ServiceName, "st_oe_reqres_data.st_ord_flg.flg_ioc is :%c", st_oe_reqres_data.st_ord_flg.flg_ioc );
		  	fn_userlog ( c_ServiceName, "st_oe_reqres_data.st_hdr.si_error_code is :%d:", st_oe_reqres_data.st_hdr.si_error_code ); 
		  	fn_userlog ( c_ServiceName, "st_oe_reqres_data.i_ordr_sqnc is :%d:", st_oe_reqres_data.i_ordr_sqnc ) ;
			}
		}			/*** Ver 1.4 ***/

   /*** ver 1.5 starts ****/
   st_i_xchngbk.c_req_typ = '\0';
   fn_userlog ( c_ServiceName, "CR_139156 : st_oe_rspn_tr_data.li_ordr_sqnc :%ld:",st_oe_rspn_tr_data.li_ordr_sqnc);
  fn_userlog ( c_ServiceName, "CR_139156 : st_oe_reqres_data.i_ordr_sqnc :%ld:",st_oe_reqres_data.i_ordr_sqnc);

   if(
   ( st_oe_reqres_data.st_hdr.si_transaction_code == ORDER_CANCEL_CONFIRM_OUT &&  (st_oe_reqres_data.st_ord_flg.flg_ioc !=  1 && st_oe_reqres_data.st_hdr.si_error_code != 17070 && st_oe_reqres_data.st_hdr.si_error_code !=17071 && st_oe_reqres_data.st_hdr.si_error_code != 16388 && st_oe_reqres_data.i_ordr_sqnc != 0 ) ) || ( st_oe_rspn_tr_data.si_transaction_code == ORDER_CXL_CONFIRMATION_TR &&  (st_oe_rspn_tr_data.st_ord_flg.flg_ioc !=  1 && st_oe_rspn_tr_data.si_error_code != 17070 && st_oe_rspn_tr_data.si_error_code != 17071 && st_oe_rspn_tr_data.si_error_code != 16388 && st_oe_rspn_tr_data.li_ordr_sqnc != 0 ))
   )
  {
   i_err [0 ] = Fget32(ptr_fml_ibuf,FFO_NXT_TRD_DT,0,(char *)st_i_xchngbk.c_mod_trd_dt,0);
   i_ferr [ 0 ] = Ferror32;
   i_err [ 1 ] = Fget32(ptr_fml_ibuf,FFO_XCHNG_CD,0,(char *)st_i_xchngbk.c_xchng_cd,0);
   i_ferr [ 1 ] = Ferror32;
   for ( i_cnt=0; i_cnt<=2; i_cnt++ )
   {
    if (i_err[i_cnt] == -1)
    {
        fn_errlog( c_ServiceName, "S31055", TPMSG, c_err_msg );
        tpfree ( (char *) ptr_fml_ibuf );
        tpfree ( (char *) ptr_car_buf ); 
        return;
    }
   }

      strcpy(st_i_xchngbk.c_pipe_id,c_pipe_id);
      st_i_xchngbk.c_oprn_typ = FOR_ERR_Q;

      fn_userlog ( c_ServiceName, "CR_139156 : pipe :%s: date :%s: exchng cd :%s: order seq :%ld:",st_i_xchngbk.c_pipe_id,st_i_xchngbk.c_mod_trd_dt,st_i_xchngbk.c_xchng_cd,st_i_xchngbk.l_ord_seq);

      memset(c_ord_ack_nse,'\0',sizeof(c_ord_ack_nse));

      if( st_oe_reqres_data.st_hdr.si_transaction_code == ORDER_CANCEL_CONFIRM_OUT )
      { 
           sprintf(c_ord_ack_nse,"%16.0lf",st_oe_reqres_data.d_order_number);
           st_i_xchngbk.l_ord_seq = st_oe_reqres_data.i_ordr_sqnc;
      }
      else
      {
           sprintf(c_ord_ack_nse,"%16.0lf",st_oe_rspn_tr_data.d_order_number);
           st_i_xchngbk.l_ord_seq = st_oe_rspn_tr_data.li_ordr_sqnc;
      }
      fn_userlog(c_ServiceName,"CR_139156 c_ord_ack_nse:%s:",c_ord_ack_nse);
      fn_userlog(c_ServiceName,"CR_139156 st_i_xchngbk.l_ord_seq :%ld:",st_i_xchngbk.l_ord_seq);

      strcpy(st_i_xchngbk.c_xchng_rmrks, c_ord_ack_nse);

      i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_i_xchngbk,
                           &st_i_xchngbk,
                           "vw_xchngbook",
                           "vw_xchngbook",
                           sizeof ( st_i_xchngbk ),
                           sizeof ( st_i_xchngbk ),
                           0,
                           "SFO_SEQ_TO_OMD");

      if ( i_ch_val != 0 )
      {
            fn_userlog ( c_ServiceName, "CR_139156 SFO_SEQ_TO_OMD ERROR" );
            fn_errlog( c_ServiceName, "S31060", TPMSG, c_err_msg );
            tpfree ( (char *) ptr_fml_ibuf );
            tpfree ( (char *) ptr_car_buf ); 
            return;
      }
      fn_userlog ( c_ServiceName, "CR_139156 : st_i_xchngbk.c_req_typ :%c: c_ordr_rfrnc :%s:",st_i_xchngbk.c_req_typ,st_i_xchngbk.c_ordr_rfrnc);
    }
    /*** ver 1.5 ends ***/

		/**** Ver 1.3 Start ****/

  	if(( st_oe_reqres_data.st_hdr.si_transaction_code == ORDER_CANCEL_CONFIRM_OUT &&  (st_oe_reqres_data.st_ord_flg.flg_ioc ==  1
  	|| st_oe_reqres_data.st_hdr.si_error_code == 17070 || st_oe_reqres_data.st_hdr.si_error_code == 17071  
  	|| st_oe_reqres_data.st_hdr.si_error_code == 16388 || st_oe_reqres_data.i_ordr_sqnc == 0 ||  (st_i_xchngbk.c_req_typ != 'C') ) ) || st_oe_reqres_data.st_hdr.si_transaction_code == ON_STOP_NOTIFICATION ) /** ver 1.5 ***/      
  	{
			if ( tpacall("SFO_PRCS_SLTP", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
			{
				fn_errlog( c_ServiceName, "S31065", TPMSG, c_err_msg );
				tpfree ( (char *) ptr_fml_ibuf );
      	tpfree ( (char *) ptr_car_buf );    /** Ver 1.3 **/
				return;
			}
    }
		/*** Ver 1.4 Starts ***/
		else if (( st_oe_rspn_tr_data.si_transaction_code == ORDER_CXL_CONFIRMATION_TR &&  (st_oe_rspn_tr_data.st_ord_flg.flg_ioc ==  1 || st_oe_rspn_tr_data.si_error_code == 17070 || st_oe_rspn_tr_data.si_error_code == 17071 || st_oe_rspn_tr_data.si_error_code == 16388 || st_oe_rspn_tr_data.li_ordr_sqnc == 0 ||  (st_i_xchngbk.c_req_typ != 'C') )) || st_oe_rspn_tr_data.si_transaction_code == ON_STOP_NOTIFICATION )  /** ver 1.5 ***/
		{
      if ( tpacall("SFO_PRCS_SLTP", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
      {
        fn_errlog( c_ServiceName, "S31070", TPMSG, c_err_msg );
        tpfree ( (char *) ptr_fml_ibuf );
        tpfree ( (char *) ptr_car_buf );
        return;
      }
    }
		/*** Ver 1.4 Ends ***/
		else 
		{

			/* async call to service rcv client */
			if ( tpacall("SFO_RCV_CLNT", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
  		{
				/* fn_re_enqueue( ptr_fml_ibuf, c_err_qn, c_qspacename); 
										not needed here as must be enqueued by the service itself */
				fn_errlog( c_ServiceName, "S31075", TPMSG, c_err_msg );
    		tpfree ( (char *) ptr_fml_ibuf );
    	  tpfree ( (char *) ptr_car_buf );    /** Ver 1.3 **/
				return;
 			}
		 }
	/* } End while( 1 ) loop  commented in Ver 1.1 */

 	tpfree ( (char *) ptr_fml_ibuf );
  tpfree ( (char *) ptr_car_buf );    /** Ver 1.3 **/
 	return;
}

void fn_re_enqueue( FBFR32 *ptr_fml_ibuf, char c_err_qn[], char c_qspacename[])
{
  TPQCTL st_eqctl;

	/* set control parameter to enqueue */ 
  st_eqctl.flags = TPNOFLAGS;

  /* enqueueing a message into errq in transaction */
  i_trnsctn = fn_begintran( c_ServiceName, c_err_msg );
  if( i_trnsctn == -1 )
  {
    fn_errlog( c_ServiceName, "S31080", LIBMSG, c_err_msg );
    tpfree ( (char *) ptr_fml_ibuf );
    return;
  }

  i_ch_val = tpenqueue ( c_qspacename, c_err_qn, &st_eqctl,
                                    (char *)ptr_fml_ibuf, 0, 0 );
	if( i_ch_val == -1 )                  													/* bug corrected in Ver 1.1 */
	{
    fn_errlog( c_ServiceName, "S31085", TPMSG, c_err_msg );
    tpfree ( (char *) ptr_fml_ibuf );
    fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
    return;
  }

  if( fn_committran( c_ServiceName, i_trnsctn, c_err_msg ) == -1 )
  {
    fn_errlog( c_ServiceName, "S31090", LIBMSG, c_err_msg );
    tpfree ( (char *) ptr_fml_ibuf );
    fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
    return;
  }
}

/* Ver 1.1 Starts */
int fn_selective_actn(  FBFR32 *ptr_fml_tempbuf,
                        char c_res_typ[],
                        char c_ordref_ack[],
                        long l_mod_cntr,
                        char *c_call_rcv )
{
	FLDLEN32 ud_len;
/***	struct st_cmn_header st_hdr;	*** Commented in Ver 1.4 ***/
	struct st_cmn_header_tr st_hdr;				/*** Ver 1.4 ***/
	struct st_oe_reqres st_oe_reqres_data;
	struct st_oe_rspn_tr st_oe_res_data_tr;		/*** Ver 1.4 ***/
	struct st_spd_oe_reqres st_spd_reqres_data;
	struct st_trade_confirm st_trd_conf;
	struct st_trade_confirm_tr st_trd_conf_tr;	/*** Ver 1.4 ***/
	struct st_ex_pl_reqres st_expl_reqres_data;
	struct vw_xchngbook st_i_xchngbk;
	char c_temp_ordref_ack[LEN_ORDR_RFRNC];

	char *ptr_car_buf;
	long int li_sbuf_len;

	MEMSET( c_temp_ordref_ack );

	ptr_car_buf =(char *)tpalloc("CARRAY", NULL, MIN_FML_BUF_LEN);
  if (ptr_car_buf == NULL)
  {
    fn_errlog( c_ServiceName, "S31095", TPMSG, c_err_msg  );
    return -1;
  }

	ud_len = ( FLDLEN32 )sizeof( li_sbuf_len );

  i_ch_val = Fget32( ptr_fml_tempbuf, FFO_TMPLT, 0, ( char * )&li_sbuf_len, &ud_len );
  if( i_ch_val == -1 )
  {
    fn_errlog( c_ServiceName, "S31100", FMLMSG, c_err_msg );
		tpfree ( ( char * )ptr_car_buf );
    return -1;
  }

  ud_len = ( FLDLEN32 )li_sbuf_len;

  i_ch_val = Fget32( ptr_fml_tempbuf, FFO_CBUF, 0, ( char * )ptr_car_buf, &ud_len );
  if( i_ch_val == -1)
  {
    fn_errlog( c_ServiceName, "S31105", FMLMSG, c_err_msg );
		tpfree ( ( char * )ptr_car_buf );
    return -1;
  }

	MEMSET( st_hdr );
/***  memcpy( &st_hdr, ptr_car_buf, sizeof( struct st_cmn_header ) );	*** Commented in Ver 1.4 ***/
  memcpy( &st_hdr, ptr_car_buf, sizeof( struct st_cmn_header_tr ) );	/*** Ver 1.4 ***/

  if( strcasecmp( c_res_typ, "ordcon" ) == 0 )   /* Ordinary order response */
  {
    switch( st_hdr.si_transaction_code )
    {
      case ORDER_CONFIRMATION_OUT :
      case ORDER_MOD_CONFIRM_OUT :
      case ORDER_CANCEL_CONFIRM_OUT :
      case ORDER_ERROR_OUT :
      case ORDER_MOD_REJ_OUT :
      case ORDER_CXL_REJ_OUT :
      case FREEZE_TO_CONTROL :
      case BATCH_ORDER_CXL_OUT :

        strcpy( st_i_xchngbk.c_ordr_rfrnc, c_ordref_ack );
        st_i_xchngbk.l_mdfctn_cntr = l_mod_cntr;
        st_i_xchngbk.c_oprn_typ = WITHOUT_ORS_MSG_TYP;

        fn_cpy_ddr ( st_i_xchngbk.c_rout_crt );
        i_ch_val = fn_call_svc( c_ServiceName,
                                    c_err_msg,
                                    &st_i_xchngbk,
                                    &st_i_xchngbk,
                                    "vw_xchngbook",
                                    "vw_xchngbook",
                                    sizeof ( struct vw_xchngbook ),
                                    sizeof ( struct vw_xchngbook ),
                                    0,
                                    "SFO_REF_TO_OMD" );
        if( i_ch_val != SUCC_BFR )
        {
          fn_errlog( c_ServiceName, "S31110", LIBMSG, c_err_msg );
					tpfree ( ( char * )ptr_car_buf );
          return -1;
        }

        MEMSET( st_oe_reqres_data );
        memcpy( &st_oe_reqres_data, ptr_car_buf, sizeof( struct st_oe_reqres ) );

        if( (int) st_i_xchngbk.l_ord_seq != st_oe_reqres_data.i_ordr_sqnc )
        {
          *c_call_rcv = 'N';
        }

        break;

			/*** Ver 1.4 Starts ***/
      case ORDER_CONFIRMATION_OUT_TR :
      case ORDER_MOD_CONFIRM_OUT_TR :
      case ORDER_CXL_CONFIRMATION_TR :
/***      case ORDER_ERROR_OUT :	*** Check and remove these cases a sthey brlong to non trim ***
      case ORDER_MOD_REJ_OUT :
      case ORDER_CXL_REJ_OUT :
      case FREEZE_TO_CONTROL :
      case BATCH_ORDER_CXL_OUT :	***/

        strcpy( st_i_xchngbk.c_ordr_rfrnc, c_ordref_ack );
        st_i_xchngbk.l_mdfctn_cntr = l_mod_cntr;
        st_i_xchngbk.c_oprn_typ = WITHOUT_ORS_MSG_TYP;

        fn_cpy_ddr ( st_i_xchngbk.c_rout_crt );
        i_ch_val = fn_call_svc( c_ServiceName,
                                    c_err_msg,
                                    &st_i_xchngbk,
                                    &st_i_xchngbk,
                                    "vw_xchngbook",
                                    "vw_xchngbook",
                                    sizeof ( struct vw_xchngbook ),
                                    sizeof ( struct vw_xchngbook ),
                                    0,
                                    "SFO_REF_TO_OMD" );
        if( i_ch_val != SUCC_BFR )
        {
          fn_errlog( c_ServiceName, "S31115", LIBMSG, c_err_msg );
          tpfree ( ( char * )ptr_car_buf );
          return -1;
        }

        MEMSET( st_oe_res_data_tr );
        memcpy( &st_oe_res_data_tr, ptr_car_buf, sizeof( struct st_oe_rspn_tr ) );

        if( (int) st_i_xchngbk.l_ord_seq != st_oe_res_data_tr.li_ordr_sqnc )
				{
          *c_call_rcv = 'N';
        }

        break;
      /*** Ver 1.4 Ends ***/

      default :
              *c_call_rcv = 'N';
              break;
    }
  }

  else if( strcasecmp( c_res_typ, "spdcon" ) == 0 )   /* Spread order response */
  {
    switch( st_hdr.si_transaction_code )
    {
      case SP_ORDER_CONFIRMATION :
      case SP_ORDER_ERROR :
      case TWOL_ORDER_CONFIRMATION :
      case TWOL_ORDER_ERROR :
      case THRL_ORDER_CONFIRMATION :
      case THRL_ORDER_ERROR :

        MEMSET( st_spd_reqres_data );
        memcpy( &st_spd_reqres_data, ptr_car_buf, sizeof( struct st_spd_oe_reqres ) );

        fn_nsetoors_char ( c_temp_ordref_ack,
                         LEN_ORDR_RFRNC,
                         st_spd_reqres_data.c_oe_remarks,
                         LEN_REMARKS );

        if( strncmp( c_temp_ordref_ack, c_ordref_ack, LEN_ORDR_RFRNC ) != 0 )
        {
          *c_call_rcv = 'N';
        }

        break;

      default :
              *c_call_rcv = 'N';
              break;
    }
  }

  else if( strcasecmp( c_res_typ, "spdcan" ) == 0 )   /* Spread order IOC */
  {
    switch( st_hdr.si_transaction_code )
    {
      case SP_ORDER_CXL_CONFIRM_OUT :
      case TWOL_ORDER_CXL_CONFIRMATION :
      case THRL_ORDER_CXL_CONFIRMATION :

        MEMSET( st_spd_reqres_data );
        memcpy( &st_spd_reqres_data, ptr_car_buf, sizeof( struct st_spd_oe_reqres ) );

        sprintf( c_temp_ordref_ack, "%16.0lf", st_spd_reqres_data.d_order_number );

				fn_userlog( c_ServiceName, "%s", c_temp_ordref_ack );

        if( strncmp( c_temp_ordref_ack, c_ordref_ack, LEN_XCHNG_ACK ) != 0 )
        {
					fn_userlog( c_ServiceName, "DBG: In no call" );
          *c_call_rcv = 'N';
        }

        break;

      default :
              *c_call_rcv = 'N';
              break;
    }
  }

  else if( strcasecmp( c_res_typ, "slptrg" ) == 0 )   /* SLTP trigger */
  {
    switch( st_hdr.si_transaction_code )
    {
      case ON_STOP_NOTIFICATION :

        MEMSET( st_trd_conf );
        memcpy( &st_trd_conf, ptr_car_buf, sizeof (struct st_trade_confirm ) );

        sprintf( c_temp_ordref_ack, "%16.0lf", st_trd_conf.d_response_order_number );

        if( strncmp( c_temp_ordref_ack, c_ordref_ack, LEN_XCHNG_ACK ) != 0 )
        {
          *c_call_rcv = 'N';
        }

        break;

      default :
              *c_call_rcv = 'N';
              break;
    }
  }

  else if( strcasecmp( c_res_typ, "trdcon" ) == 0 )   /* Trade Confirmation */
  {
    switch( st_hdr.si_transaction_code )
    {
      case TRADE_CONFIRMATION :

        MEMSET( st_trd_conf );
        memcpy( &st_trd_conf, ptr_car_buf, sizeof (struct st_trade_confirm ));

        sprintf( c_temp_ordref_ack, "%16.0lf", st_trd_conf.d_response_order_number );

        if( strncmp( c_temp_ordref_ack, c_ordref_ack, LEN_XCHNG_ACK ) != 0 )
        {
          *c_call_rcv = 'N';
        }

        break;

			/*** Ver 1.4 Starts ***/
       case TRADE_CONFIRMATION_TR :

        MEMSET( st_trd_conf_tr );
        memcpy( &st_trd_conf_tr, ptr_car_buf, sizeof (struct st_trade_confirm_tr ));

        sprintf( c_temp_ordref_ack, "%16.0lf", st_trd_conf_tr.d_response_order_number );

        if( strncmp( c_temp_ordref_ack, c_ordref_ack, LEN_XCHNG_ACK ) != 0 )
        {
          *c_call_rcv = 'N';
        }

        break;
      /*** Ver 1.4 Ends ***/

      default :
              *c_call_rcv = 'N';
              break;
    }
	}

	  else if( strcasecmp( c_res_typ, "exrres" ) == 0 )   /* exrecise order response */
  {
    switch( st_hdr.si_transaction_code )
    {
      case EX_PL_ENTRY_OUT :
      case EX_PL_MOD_OUT :
      case EX_PL_CXL_OUT :

      MEMSET( st_expl_reqres_data );
      memcpy( &st_expl_reqres_data, ptr_car_buf, sizeof (struct st_ex_pl_reqres ));

      fn_nsetoors_char ( c_temp_ordref_ack,
                         LEN_ORDR_RFRNC,
                         st_expl_reqres_data.st_ex_pl_data.c_remarks,
                         LEN_REMARKS_1 );
	
			fn_userlog( c_ServiceName, "c_temp_ordref_ack  :%s", c_temp_ordref_ack );

      if( strncmp( c_temp_ordref_ack, c_ordref_ack, LEN_ORDR_RFRNC ) != 0 )
      {
        *c_call_rcv = 'N';
      }

      break;

      default :
              *c_call_rcv = 'N';
              break;
    }
  }

  else if( strcasecmp( c_res_typ, "exrcon" ) == 0 )   /* exrecise order confirmation */
  {
    switch( st_hdr.si_transaction_code )
    {
      case EX_PL_CXL_CONFIRMATION :
      case EX_PL_MOD_CONFIRMATION :
      case EX_PL_CONFIRMATION :
      case EX_PL_ENTRY_IN :

        MEMSET( st_expl_reqres_data );
        memcpy( &st_expl_reqres_data,ptr_car_buf,sizeof (struct st_ex_pl_reqres ));

        sprintf( c_temp_ordref_ack, "%16.0lf", st_expl_reqres_data.st_ex_pl_data.d_expl_number );
				fn_userlog( c_ServiceName, "c_temp_ordref_ack  :%s", c_temp_ordref_ack );

        if( strncmp( c_temp_ordref_ack, c_ordref_ack, LEN_XCHNG_ACK ) != 0 )
        {
          *c_call_rcv = 'N';
        }

        break;

      default :
              *c_call_rcv = 'N';
              break;
    }
  }

  else   /* others skipping */
  {
    *c_call_rcv = 'N';
  }

  return 0;
}
/* Ver 1.1 Ends */

int fn_bat_init ( int argc, char *argv[] )
{
  /* if (argc != 4)
  {
    fn_userlog( argv[0],
      					"Invalid number of arguments Usage - %s <configuration file> <Exchange code> <pipe_id>",argv[0] );
    return -1;
  }		commented in Ver 1.1 */

  fn_var = BAT_FO_ERRQRES;
  return 0;
}

void fn_bat_term ( int argc, char *argv[] )
{
  fn_userlog( argv[0], "Batch process %s ended", argv[0] );
  return;
}

int fn_prcs_trg ( char *c_pgm_nm,
                  int i_trg_typ )
{
  return 0;
}
