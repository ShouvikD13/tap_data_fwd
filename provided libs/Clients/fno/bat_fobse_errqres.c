/******************************************************************************/
/*  Program       : bat_fobse_errqres.c                                       */
/*                                                                            */
/*  Description   : This program dequeue responses from error queue and call 	*/
/*									Receive Client to process the same,	must be run on Tux3		*/
/*                                                                            */
/*  Input         : <configuration file> <Exchange code> <pipe_id>            */ 
/*                                                                            */
/*  Output        : Success/Failure                                           */
/*                                                                            */
/*  Log           : New Version : Sachin Birje                                */
/*                  Ver 1.1			:	Samip M																			*/
/*                                                                            */
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

#include <stdlib.h>
#include <eba_to_ors.h>
#include <fo_bse_exch_comnN.h>
/*** #include <fo_view_def.h>	**/	/** Ver 1.1	**/
#include <fn_ddr.h>
#include <fml_rout.h>
#include <fn_scklib_iml.h>
#include <fn_fo_bse_convrtn.h>
#define ALL_RUN 				'A'

/* Global variables */
char c_ServiceName[33];
char c_err_msg[256];
int i_trnsctn;
int i_ch_val;

void fn_re_enqueue( FBFR32 *, char *, char *);

void BAT_FOBSE_ERRQRES( int argc, char *argv[])
{
  FBFR32 *ptr_fml_ibuf;
  FLDLEN32 ud_len;

  TPQCTL st_dqctl;

	char c_pipe_id[3];
  char c_qspacename[32];
  char c_err_qn[32];
	char c_run_opt;										
  char c_ordref_ack[LEN_ORDR_RFRNC];
  char c_res_typ[6+1];							
  char *ptr_car_sbuf;

	int i_dwnld_flg;
	int i_deferred_dwnld_flg;
	long int li_len;
  long l_mod_cntr;								
  long l_msg_type;
  long int li_sbuf_len;

	INITBATDBGLVL( argv[0] );

	strcpy( c_ServiceName, argv[0] );

	strcpy( c_pipe_id, argv[3] );

  if( argc < 5 )
  {
    fn_userlog( c_ServiceName, "Invalid number of arguments Usage - %s <configuration file> <Exchange code> <pipe_id> \
                                                                      <run opt>", c_ServiceName );
    return;
  }

  c_run_opt = argv[4][0];

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog( c_ServiceName, "c_pipe_id   |%s|", c_pipe_id );
    fn_userlog( c_ServiceName, "c_run_opt   |%c|", c_run_opt );
  }

	/* error qname, qspace name */
  sprintf(c_err_qn,"ERR_%s_Q",c_pipe_id);
  sprintf(c_qspacename,"%s_QSPACE",c_pipe_id);

	if(DEBUG_MSG_LVL_3)
  {
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

	/* set control parameter to dequeue */ 
  st_dqctl.flags = TPNOFLAGS;


 /* dequeueing a message from errq in transaction */
	i_trnsctn = fn_begintran( c_ServiceName, c_err_msg );
  if( i_trnsctn == -1 )
 	{
  	fn_errlog( c_ServiceName, "S31010", LIBMSG, c_err_msg );
   	tpfree ( (char *) ptr_fml_ibuf );
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
			fn_userlog( c_ServiceName, "No message in the queue, BAT_FOBSE_ERRQRES leaving...");
     }
		else 							
		{
 			fn_errlog( c_ServiceName, "S31015", TPMSG, c_err_msg );
		}

   	tpfree ( (char *) ptr_fml_ibuf );
   	fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
   	return;
 	}

 	if( fn_committran( c_ServiceName, i_trnsctn, c_err_msg ) == -1 )
 	{
   	fn_errlog( c_ServiceName, "S31020", LIBMSG, c_err_msg );
   	tpfree ( (char *) ptr_fml_ibuf );
   	fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
   	return;
 	}

  ptr_car_sbuf =(char *)tpalloc("CARRAY", NULL, MIN_FML_BUF_LEN);
  if (ptr_car_sbuf == NULL)
  {
    fn_errlog( c_ServiceName, "S31025", TPMSG, c_err_msg  );
    return ;
  }

  ud_len = ( FLDLEN32 )sizeof( li_sbuf_len );

  i_ch_val = Fget32( ptr_fml_ibuf, FFO_TMPLT, 0, ( char * )&li_sbuf_len, &ud_len );
  if( i_ch_val == -1 )
  {
    fn_errlog( c_ServiceName, "S31030", FMLMSG, c_err_msg );
    tpfree ( ( char * )ptr_car_sbuf );
    return ;
  }

  ud_len = ( FLDLEN32 )li_sbuf_len;

  i_ch_val = Fget32(ptr_fml_ibuf , FFO_CBUF, 0, ( char * )ptr_car_sbuf, &ud_len );
  if( i_ch_val == -1)
  {
    fn_errlog( c_ServiceName, "S31035", FMLMSG, c_err_msg );
    tpfree ( ( char * )ptr_car_sbuf );
    return ;
  }

  memcpy(&l_msg_type, ptr_car_sbuf + sizeof(struct st_bfo_header), sizeof(long));

  l_msg_type = fn_swap_long(l_msg_type);

	/* hardcode dwnld_flg and deferred_dwnld_flg */
	i_dwnld_flg = DOWNLOAD;
 	if( Fchg32( ptr_fml_ibuf, FFO_BIT_FLG, 0, (char *)&i_dwnld_flg, 0 ) == -1 )
 	{
		fn_re_enqueue( ptr_fml_ibuf, c_err_qn, c_qspacename);
		fn_errlog( c_ServiceName, "S31040", FMLMSG, c_err_msg );
   	tpfree ( (char *) ptr_fml_ibuf );
		return;
	}

	i_deferred_dwnld_flg = 0;
 	if( Fchg32( ptr_fml_ibuf, FFO_DWNLD_FLG, 0, (char *)&i_deferred_dwnld_flg, 0 ) == -1 )
 	{
		fn_re_enqueue( ptr_fml_ibuf, c_err_qn, c_qspacename);
		fn_errlog( c_ServiceName, "S31045", FMLMSG, c_err_msg );
   	tpfree ( (char *) ptr_fml_ibuf );
		return;
	}

	/* async call to service rcv client */

  if ( l_msg_type == BSE_TRADE_CONFIRMATION  || l_msg_type == PRSNL_TRD_DWNLD )
  { 
	  if ( tpacall("SFO_BTRD_CLNT", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
 	  {
	 	 fn_errlog( c_ServiceName, "S31050", TPMSG, c_err_msg );
   	 tpfree ( (char *) ptr_fml_ibuf );
		 return;
 	  }
  }
  else 
  {
    if ( tpacall("SFO_BRCV_CLNT", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
    {
     fn_errlog( c_ServiceName, "S31055", TPMSG, c_err_msg );
     tpfree ( (char *) ptr_fml_ibuf );
     return;
    }  
  }
 	tpfree ( (char *) ptr_fml_ibuf );
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
    fn_errlog( c_ServiceName, "S31060", LIBMSG, c_err_msg );
    tpfree ( (char *) ptr_fml_ibuf );
    return;
  }

  i_ch_val = tpenqueue ( c_qspacename, c_err_qn, &st_eqctl,
                                    (char *)ptr_fml_ibuf, 0, 0 );
	if( i_ch_val == -1 )                  									
	{
    fn_errlog( c_ServiceName, "S31065", TPMSG, c_err_msg );
    tpfree ( (char *) ptr_fml_ibuf );
    fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
    return;
  }

  if( fn_committran( c_ServiceName, i_trnsctn, c_err_msg ) == -1 )
  {
    fn_errlog( c_ServiceName, "S31070", LIBMSG, c_err_msg );
    tpfree ( (char *) ptr_fml_ibuf );
    fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
    return;
  }
}

int fn_bat_init ( int argc, char *argv[] )
{
  fn_var = BAT_FOBSE_ERRQRES;
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
