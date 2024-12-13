/******************************************************************************/
/*  Program           : SFO_RCV_CLNT                                          */
/*                                                                            */
/*  Input             : C_PIPE_ID                                             */
/*                                                                            */
/*  Output            :                                                       */
/*                                                                            */
/*  Description       :																												*/
/*																																						*/
/*  Log               : 1.0   22-Jun-2009     Sandeep Patil                   */
/*	Log								:	1.1		13-Aug-2009			Sandeep	Patil										*/
/*	Log								:	1.2		18-Aug-2009			Sandeep	Patil										*/
/*	Log								:	1.3		30-Oct-2009			Sangeet	Sinha										*/
/*	log								:	1.4		17-Jun-2011			Sandeep Patil										*/
/*	log								:	1.5		12-Mar-2012			Shailesh Hinge									*/
/*	log								:	1.6		12-Mar-2012			SH															*/
/*  log               : 1.7   17-Apr-2014     Vishnu Nair                     */
/*  log               : 1.9   09-Sep-2014     Samip M                         */
/*  log               : 2.0   07-Jul-2016     Sachin Birje                    */
/*  log               : 2.1   10-Nov-2016     Tanmay W.		                    */
/*  log               : 2.2   10-Oct-2017     Sachin Birje                    */
/*  log               : 2.3   14-Feb-2019     Suchita Dabir                   */
/*  log               : 2.4   14-Mar-2018     Sachin Birje                    */
/*  log               : 2.5   17-NOV-2020     Suchita Dabir                   */
/******************************************************************************/
/*  Version 1.0 -	New Release															                    */
/*	Version 1.1 - Routing Added																								*/
/*	Version	1.2	-	Sleep Commented In case Of Trade Confirmation	and handeled	*/
/*								in fn_trd_confirmation Function															*/
/*	Version	1.3	-	Removing all sleep conditions to Fix Scheduling issue		    */
/*	Version	1.4	-	SPAN	Changes																								*/
/*	Version	1.5	-	Call from bat_fo_prcs_trd check of SQL rownum handeled		  */
/*	Version	1.6	-	Broker closeout handling was missed out in previous version */
/*								1.3,introduced it again																			*/ 
/*  Version 1.7 - Exchange ack number formatted as number rcvd by exchange    */
/*                contained decimal which resulted in to NO_DATA_FOUND        */
/*  Version 1.9 - View to FML changes                                         */
/*  Version 2.0 - debug level changes                                         */
/*  Version 2.1 - Response in trans code 2075 for part executed cancel 				*/
/*								spread order in two packets and cancellation with sequence 	*/
/*								zero & Core Fix  (Tanmay W.)																*/
/* Version 2.2 	- Userlog changes ( Sachin Birje ) 														*/
/* Version 2.3  - Trimmed Protocol for Non-NEAT Front End                     */
/* Version 2.4  - FO Rollover with Spread Changes ( Sachin Birje )            */
/* Version 2.5  - Unsolicited cancellation in rcv ( Suchita)                  */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <brd_cln_srvr.h>
#include <fo_exg_rcv_lib.h>
#include <fo_spn_rcv_lib.h>

#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>

#include <sqlca.h>

#include <fo_fml_def.h>
/** #include <fo_view_def.h>  **/ /** Ver 1.9 **/
#include <fo.h>
#include <fn_env.h>
#include <fn_tuxlib.h>
#include <fn_fil_log.h>
#include <fn_ddr.h>
#include <fn_log.h>
#include <fml_rout.h>
#include <fml_def.h>
#include <userlog.h>
#include <fn_session.h>
#include <fn_read_debug_lvl.h>
#include <fn_span_lib.h>
#include <netinet/in.h>									/*** Ver 2.3 ***/
#include <fn_nw_to_host_format_nse.h>		/*** Ver 2.3 ***/


void SFO_RCV_CLNT ( TPSVCINFO *rqst )
{
	FBFR32 *ptr_fml_ibuf;
	
	FLDLEN32  ud_len;
	char c_ServiceName[33];
	char *ptr_car_sbuf;
	char c_pipe_id[ 2 + 1 ];
	char c_qspacename[32];
	char c_err_qn[32];
	char c_err_msg[256];
	char c_xchng_cd [ 3+1 ];
	varchar c_trd_dt [ 20 + 1 ];
	varchar c_trd_ref [8 + 1];
	char c_skpord_ref[18 + 1];
	char c_rout_str [ 4 ];
	char c_spn_flg;								/***	Ver	1.4	***/
	char c_mod_trd_dt[23];			 /***  Ver 1.4 ***/
	char c_spd_ord_seq[10 +1 ];     /** 2L Order OEREMARK Change **/
  char c_xchng_ack[17]; 	/***  Ver 1.4 ***/
  char c_ord_ack_nse[20] ;    /** 2.1 **/


	int i_rtrn_cd ;
	int i_ret_val ;
	int i_dwnld_flg ;
	int i_errno;
	int i_err [ 7 ];
	int i_ferr [ 7 ];
	int i_cnt;
	int li_sbuf_len;
	int i_deferred_dwnld_flg;


	/** struct st_cmn_header st_hdr; commented in ver 2.3 ***/
  struct st_cmn_header_tr st_hdr; /*** ver 2.3 ***/
	struct st_oe_reqres st_oe_reqres_data;
	struct st_oe_rspn_tr st_oe_rspn_tr_data; /*** ver 2.3 ***/
	struct st_trade_confirm_tr st_trd_conf_tr_data; /*** ver 2.3 ***/
	struct st_spd_oe_reqres st_spd_reqres_data;
	struct st_trade_confirm st_trd_conf; 
	struct st_ex_pl_reqres st_expl_reqres_data;
	struct st_spdordbk st_spd_ordbk;             /** 2L Order OEREMARK Change **/
  struct st_trader_int_msg st_trd_msg;				 /**Ver 1.6 **/
	MEMSET(st_trd_msg);													/**ver 1.6 **/	
	MEMSET(st_spd_ordbk);											 /** 2L Order OEREMARK Change **/
  MEMSET(st_oe_rspn_tr_data); /*** ver 2.3 ***/
	MEMSET(st_trd_conf_tr_data); /*** ver 2.3 ***/
	strcpy(c_ServiceName, "SFO_RCV_CLNT" );

	INITDBGLVL(c_ServiceName);


	ptr_fml_ibuf = (FBFR32 *)rqst->data;

	MEMSET (c_pipe_id);

	i_err [ 0 ] = Fget32(ptr_fml_ibuf,FFO_PIPE_ID,0,(char *)c_pipe_id, 0);
	i_ferr [ 0 ] = Ferror32;
	i_err [ 1 ] = Fget32(ptr_fml_ibuf,FFO_BIT_FLG,0,(char *)&i_dwnld_flg,0);
	i_ferr [ 1 ] = Ferror32;
	i_err [ 2 ] = Fget32(ptr_fml_ibuf,FFO_NXT_TRD_DT,0,(char *)c_trd_dt.arr,0);
	i_ferr [ 2 ] = Ferror32;
	i_err [ 3 ] = Fget32(ptr_fml_ibuf,FFO_XCHNG_CD,0,(char *)c_xchng_cd,0);
  i_ferr [ 3 ] = Ferror32;
	i_err [ 4 ] = Fget32(ptr_fml_ibuf,FFO_REF_NO,0,(char *)c_trd_ref.arr,0);
  i_ferr [ 4 ] = Ferror32;
	i_err [ 5 ] = Fget32(ptr_fml_ibuf,FFO_ORDR_RFRNC,0,(char *)c_skpord_ref,0);
  i_ferr [ 5 ] = Ferror32;
	i_err [ 6 ] = Fget32(ptr_fml_ibuf,FFO_DWNLD_FLG,0,(char *)&i_deferred_dwnld_flg,0);
	i_ferr [ 6 ] = Ferror32;

	for ( i_cnt=0; i_cnt<=6; i_cnt++ )
	{
		if (i_err[i_cnt] == -1)
		{
			fn_userlog(c_ServiceName, "Error in field no. %d",i_cnt);
      fn_errlog(c_ServiceName, "S31005", Fstrerror32(i_ferr[i_cnt]),c_err_msg );
			tpfree ( ( char * )ptr_fml_ibuf);
      tpreturn(TPFAIL,0 , (char *)NULL, 0, 0 );
    }
  }

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"I/P Pipe ID Is :%s:",c_pipe_id);
		fn_userlog(c_ServiceName,"I/P Down Load Flag Is :%d:",i_dwnld_flg);
		fn_userlog(c_ServiceName,"I/P Exchange Code Is :%s:",c_xchng_cd);
		fn_userlog(c_ServiceName,"I/P Next Trade Date :%s:",c_trd_dt.arr);
		fn_userlog(c_ServiceName,"I/P Trade Ref Is :%s:",c_trd_ref.arr);
		fn_userlog(c_ServiceName,"I/P Skip Order Ref Is :%s:",c_skpord_ref);
		fn_userlog(c_ServiceName,"I/P Deffered DownLoad Flag Is :%d:",i_deferred_dwnld_flg);
		fn_userlog(c_ServiceName,"Step 1 CARRAY");
	}

	ptr_car_sbuf =(char *)tpalloc("CARRAY", NULL, MIN_FML_BUF_LEN);

	if (ptr_car_sbuf == NULL)
	{
		fn_errlog( c_ServiceName, "S31010", TPMSG, c_err_msg  );
		tpfree ( ( char * )ptr_fml_ibuf);
		tpreturn(TPFAIL, 0, (char *)NULL, 0, 0 );
	}
	
	MEMSET(c_qspacename);
	MEMSET(c_err_qn);

	sprintf(c_err_qn,"ERR_%s_Q",c_pipe_id);
	sprintf(c_qspacename,"%s_QSPACE",c_pipe_id);

	if( Fadd32(ptr_fml_ibuf,FML_FILE_NAME, (char *)c_err_qn, 0) == -1)
	{
		fn_errlog( c_ServiceName, "S31015", UNXMSG, c_err_msg  );
		tpfree ( ( char * )ptr_car_sbuf );

		if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
		{
			fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
		}
		tpfree ( ( char * )ptr_fml_ibuf);
		tpreturn(TPFAIL, 0, (char *)NULL, 0, 0 );
	}

	if ( Fadd32(ptr_fml_ibuf,FML_GMS_NM, (char *)c_qspacename, 0) == -1 )
	{
		fn_errlog( c_ServiceName, "S31020", UNXMSG, c_err_msg  );

		tpfree ( ( char * )ptr_car_sbuf );

		if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
		{
			 fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
		}
		tpfree ( ( char * )ptr_fml_ibuf);
		tpreturn(TPFAIL, 0, (char *)NULL, 0, 0 );
	}

	ud_len = (FLDLEN32)sizeof(li_sbuf_len);
	i_ret_val = Fget32(ptr_fml_ibuf, FFO_TMPLT, 0,(char *)&li_sbuf_len, &ud_len);

	if(i_ret_val == -1 )
	{
		fn_errlog(c_ServiceName,"S31025", DEF_USR,c_err_msg);
		tpfree ( ( char * )ptr_car_sbuf );
    if( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
		{
			 fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
		}
		tpfree ( ( char * )ptr_fml_ibuf);
    tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  }

	ud_len = (FLDLEN32)li_sbuf_len;
	i_ret_val = Fget32(ptr_fml_ibuf, FFO_CBUF, 0,(char *)ptr_car_sbuf, &ud_len);

  if (i_ret_val == -1)
  {
    fn_errlog(c_ServiceName,"S31030",DEF_USR,c_err_msg);
		tpfree ( ( char * )ptr_car_sbuf );
    if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
		{
			 fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
		}
		tpfree ( ( char * )ptr_fml_ibuf);
    tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  }

	/***	Ver 1.1 ***/
	if(c_pipe_id[0] == '1')
	{
		c_rout_str[0] = c_pipe_id[1];
  	c_rout_str[1] = c_pipe_id[1];
  	c_rout_str[2] = c_pipe_id[1];
  	c_rout_str[3] = '\0';
	}
	else
	{
		strcpy(c_rout_str,"000");
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"c_rout_str is :%s:",c_rout_str);
	}

	MEMSET(st_hdr);

	/** memcpy (&st_hdr,ptr_car_sbuf,sizeof (struct st_cmn_header)); *** commented in ver 2.3 ***/
  memcpy (&st_hdr,ptr_car_sbuf,sizeof (struct st_cmn_header_tr)); /*** ver 2.3 ***/
	st_hdr.si_transaction_code = ntohs(st_hdr.si_transaction_code);	/*** ver 2.3 ***/

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"Transaction Code Received From Exchange Is :%d:",st_hdr.si_transaction_code);
	}

	switch (st_hdr.si_transaction_code)
	{
		case BCAST_SECURITY_MSTR_CHG :

			break;

		case BCAST_STOCK_STATUS_CHG :
		case BCAST_STOCK_STATUS_CHG_PREOPEN : 

			break;

		case BCAST_PART_MSTR_CHG:

			break;

		case BCAST_INSTR_MSTR_CHG:

			break;

		case BCAST_INDEX_MSTR_CHG :

			break;

		case BCAST_BASE_PRICE :

			break;

		case BCAST_INDEX_MAP_TABLE :

			break;

		case ORS_OPN_ORD_DTLS :

			if(DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName,"Inside ORS_OPN_ORD_DTLS Case ");
			}

			MEMSET(st_oe_reqres_data);
			memcpy (&st_oe_reqres_data,ptr_car_sbuf,sizeof (struct st_oe_reqres ));

			fn_cnvt_nth_oe_reqres(&st_oe_reqres_data);    /*** Ver 2.3 ***/

			i_ret_val = fn_opn_ord_dtls ( &st_oe_reqres_data,
																		c_pipe_id,
																		c_rout_str,					/*** Ver 1.1 Routing Added ***/
			                         			c_ServiceName ,
                              			c_err_msg );

      if(i_ret_val == -1)
      {
        if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
      }														 

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"After Successful Call To fn_opn_ord_dtls ");
			}

			break;

		case EXCH_PORTF_OUT :

			break;

		case HEADER_RECORD :

			break;

		case BCAST_JRNL_VCT_MSG :

			break;

		case BCAST_TURNOVER_EXCEEDED :
		case BROADCAST_BROKER_REACTIVATED :

			break;

		case BC_OPEN_MSG :
		case BC_CLOSE_MSG :
 		case BC_PRE_OR_POST_DAY_MSG :
 		case BC_PRE_OPEN_ENDED :
 		case EQUAL_BC_POSTCLOSE_MSG :

			break;

		case BOARD_LOT_OUT :
  	case ORDER_MOD_OUT :
  	case ORDER_CANCEL_OUT :

			break;

		case PRICE_CONFIRMATION :

			break;		

		case ORDER_CONFIRMATION_OUT :
		case ORDER_MOD_CONFIRM_OUT :
		case ORDER_CANCEL_CONFIRM_OUT :
		case ORDER_ERROR_OUT :
		case ORDER_MOD_REJ_OUT :
		case ORDER_CXL_REJ_OUT :
		case FREEZE_TO_CONTROL :
		case BATCH_ORDER_CXL_OUT :

			if(DEBUG_MSG_LVL_3)  /*** ver 2.0 **, debug level changed from 0 to 3 **/
			{
				fn_userlog(c_ServiceName,"Inside ORDER CONFERMATION/REJECTION Case ");
			}

			MEMSET(st_oe_reqres_data);
			memcpy (&st_oe_reqres_data,ptr_car_sbuf,sizeof (struct st_oe_reqres ));

			fn_cnvt_nth_oe_reqres(&st_oe_reqres_data);		/*** Ver 2.3 ***/

			if(DEBUG_MSG_LVL_5)  /*** Ver 2.0, debug level changed from 0 to 5 ***/
			{
				fn_userlog(c_ServiceName,"Sequence Number Is :%d:",st_oe_reqres_data.i_ordr_sqnc);
			}

			if(i_dwnld_flg == DOWNLOAD && i_deferred_dwnld_flg == 1 )
  		{
    		fn_userlog(c_ServiceName,"Inside Deffered Download Condition");
    		if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
    		{
      		fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
    		}
    		tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
    		tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  		}
   
      /***** Commented in Ver 2.0 *********
			fn_userlog(c_ServiceName,"c_xchng_cd :%s:",c_xchng_cd);			***	Ver	1.4	***
			fn_userlog(c_ServiceName,"c_pipe_id :%s:",c_pipe_id);			***	Ver	1.4	***
      ******* Ver 2.0 Ends here ***********/

			strcpy(c_mod_trd_dt,c_trd_dt.arr);			/***	Ver	1.4	***/

      /**** Commented in Ver 2.0 *********
			fn_userlog(c_ServiceName,"c_mod_trd_dt :%s:",c_mod_trd_dt);			***	Ver	1.4	***
      ******** Ver 2.0 Ends here ***********/
 
      if(DEBUG_MSG_LVL_0)  /*** Ver 2.0 ***/
      {
        fn_userlog(c_ServiceName,"Sequence Number Is :%d:,c_xchng_cd :%s:,c_pipe_id :%s:,c_mod_trd_dt :%s:",st_oe_reqres_data.i_ordr_sqnc,c_xchng_cd,c_pipe_id,c_mod_trd_dt);  
      }
     
      /****** commented in ver 2.5 ********* 
       if(st_hdr.si_transaction_code  == ORDER_CANCEL_CONFIRM_OUT && (st_oe_reqres_data.st_hdr.si_error_code == 16388 || st_oe_reqres_data.i_ordr_sqnc == 0))   ** ver 2.1 **/
      if(st_hdr.si_transaction_code  == ORDER_CANCEL_CONFIRM_OUT ) /** ver 2.5 ***/
			{

        fn_userlog(c_ServiceName,"CANCELLATION HANDLED BY ACK NUMBER");
        memset(c_ord_ack_nse,'\0',sizeof(c_ord_ack_nse));
        sprintf(c_ord_ack_nse,"%16.0lf",st_oe_reqres_data.d_order_number);
        fn_userlog(c_ServiceName,"c_ord_ack_nse:%s:",c_ord_ack_nse);

        EXEC SQL
				SELECT  NVL(FOD_SPN_FLG,'N')
        INTO    :c_spn_flg
        FROM    FOD_FO_ORDR_DTLS
        WHERE   FOD_ACK_NMBR  = :c_ord_ack_nse
        AND     ROWNUM < 2;

			}
			else
			{
				EXEC SQL										/***	Ver	1.4	***/
				SELECT	NVL(FXB_SPN_FLG,'N')
				INTO		:c_spn_flg
				FROM		FXB_FO_XCHNG_BOOK
				WHERE		FXB_XCHNG_CD		=	:c_xchng_cd
				AND			FXB_PIPE_ID			=	:c_pipe_id
				AND			FXB_ORDR_SQNC		=	:st_oe_reqres_data.i_ordr_sqnc
				AND			FXB_MOD_TRD_DT	=	:c_mod_trd_dt;
			}

			if ( SQLCODE != 0 )
			{
				fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");
    		fn_errlog ( c_ServiceName, "S31035", SQLMSG, c_err_msg );

        if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
        tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );

			}
      /**** Commented in Ver 2.0 *******************************
			fn_userlog(c_ServiceName,"c_xchng_cd Is :%s:",c_xchng_cd);
			fn_userlog(c_ServiceName,"c_pipe_id Is :%s:",c_pipe_id);
			fn_userlog(c_ServiceName,"st_oe_reqres_data.i_ordr_sqnc Is :%d:",st_oe_reqres_data.i_ordr_sqnc);
			fn_userlog(c_ServiceName,"c_mod_trd_dt Is :%s:",c_mod_trd_dt);
			fn_userlog(c_ServiceName,"c_spn_flg Is :%c:",c_spn_flg);
      ************ Ver 2.0 Ends here ***************************/

      if(DEBUG_MSG_LVL_0)  /*** Ver 2.0 ***/
      {
        fn_userlog(c_ServiceName,"c_spn_flg Is :%c:",c_spn_flg);  
      }

			if(	c_spn_flg	!=	'S' )	
			{
				i_ret_val = fn_ord_confirmation( &st_oe_reqres_data,	
																				 c_xchng_cd,
																				 c_pipe_id,
																				 c_rout_str,					/*** Ver 1.1 Routing Added ***/
																				 c_trd_dt.arr,
																				 c_skpord_ref,
																				 i_dwnld_flg,
																				 c_ServiceName,
																	  		 c_err_msg );

      	if(i_ret_val == -1)
      	{
        	if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        	{
          	fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        	}
      	}
			}
			else
			{
        if(DEBUG_MSG_LVL_3)  /*** Ver 2.0 ***/
        {
				  fn_userlog(c_ServiceName,"SPAN ORDER Reponse Processing.");
		    }

				i_ret_val = fn_spnord_confirmation( &st_oe_reqres_data,
                                         		c_xchng_cd,
                                         		c_pipe_id,
                                        	 	c_rout_str,      
                                         		c_trd_dt.arr,
                                         		c_skpord_ref,
                                         		i_dwnld_flg,
                                         		c_ServiceName,
                                         		c_err_msg );

        if(i_ret_val == -1)
        {
          if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
          {
            fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
          }
        }

			}
			
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"After Successful Call To fn_ord_cnfrmtn");
			}

			break;
		/*** ver 2.3 starts **/
		case ORDER_CONFIRMATION_OUT_TR:
		case ORDER_MOD_CONFIRM_OUT_TR:
		case ORDER_CXL_CONFIRMATION_TR:
			if(DEBUG_MSG_LVL_3) 
			{
				fn_userlog(c_ServiceName,"Inside ORDER CONFERMATION/REJECTION Case For Trimmed Protocol ");
			}

			MEMSET(st_oe_rspn_tr_data);
			memcpy (&st_oe_rspn_tr_data,ptr_car_sbuf,sizeof (struct st_oe_rspn_tr ));

			fn_cnvt_nth_oe_rspn_tr(&st_oe_rspn_tr_data);

			if(DEBUG_MSG_LVL_5) 
			{
				fn_userlog(c_ServiceName,"Sequence Number Is :%ld:",st_oe_rspn_tr_data.li_ordr_sqnc);
			}

			if(i_dwnld_flg == DOWNLOAD && i_deferred_dwnld_flg == 1 )
  		{
    		fn_userlog(c_ServiceName,"Inside Deffered Download Condition");
    		if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
    		{
      		fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
    		}
    		tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   
    		tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  		}
   
			strcpy(c_mod_trd_dt,c_trd_dt.arr);		

 
      if(DEBUG_MSG_LVL_0)  
      {
        fn_userlog(c_ServiceName,"Sequence Number Is :%ld:,c_xchng_cd :%s:,c_pipe_id :%s:,c_mod_trd_dt :%s:",st_oe_rspn_tr_data.li_ordr_sqnc,c_xchng_cd,c_pipe_id,c_mod_trd_dt);  
      }
      
      /** if(st_hdr.si_transaction_code  == ORDER_CXL_CONFIRMATION_TR && (st_oe_rspn_tr_data.si_error_code == 16388 || st_oe_rspn_tr_data.li_ordr_sqnc == 0))    commentedin ver 2.5 ***/
      if(st_hdr.si_transaction_code  == ORDER_CXL_CONFIRMATION_TR) /** ver 2.5 **/
			{

        fn_userlog(c_ServiceName,"CANCELLATION HANDLED BY ACK NUMBER :%lf:",st_oe_rspn_tr_data.d_order_number);
        memset(c_ord_ack_nse,'\0',sizeof(c_ord_ack_nse));
        sprintf(c_ord_ack_nse,"%16.0lf",st_oe_rspn_tr_data.d_order_number);
        fn_userlog(c_ServiceName,"c_ord_ack_nse:%s:",c_ord_ack_nse);

        EXEC SQL
				SELECT  NVL(FOD_SPN_FLG,'N')
        INTO    :c_spn_flg
        FROM    FOD_FO_ORDR_DTLS
        WHERE   FOD_ACK_NMBR  = :c_ord_ack_nse
        AND     ROWNUM < 2;

			}
			else
			{
				EXEC SQL										
				SELECT	NVL(FXB_SPN_FLG,'N')
				INTO		:c_spn_flg
				FROM		FXB_FO_XCHNG_BOOK
				WHERE		FXB_XCHNG_CD		=	:c_xchng_cd
				AND			FXB_PIPE_ID			=	:c_pipe_id
				AND			FXB_ORDR_SQNC		=	:st_oe_rspn_tr_data.li_ordr_sqnc
				AND			FXB_MOD_TRD_DT	=	:c_mod_trd_dt;
			}

			if ( SQLCODE != 0 )
			{
				fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");
    		fn_errlog ( c_ServiceName, "S31040", SQLMSG, c_err_msg );

        if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
        tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );

			}


      if(DEBUG_MSG_LVL_0)  
      {
        fn_userlog(c_ServiceName,"c_spn_flg in TRIM Is :%c:",c_spn_flg);  
      }

			if(	c_spn_flg	!=	'S' )	
			{
				i_ret_val = fn_ord_confirmation_trim( &st_oe_rspn_tr_data,	
																				 c_xchng_cd,
																				 c_pipe_id,
																				 c_rout_str,					
																				 c_trd_dt.arr,
																				 c_skpord_ref,
																				 i_dwnld_flg,
																				 c_ServiceName,
																	  		 c_err_msg );

      	if(i_ret_val == -1)
      	{
        	if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        	{
          	fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        	}
      	}
			}
			else
			{
        if(DEBUG_MSG_LVL_3)  
        {
				  fn_userlog(c_ServiceName,"SPAN ORDER Reponse Processing.");
		    }

				i_ret_val = fn_spnord_confirmation_trim( &st_oe_rspn_tr_data,
                                         		c_xchng_cd,
                                         		c_pipe_id,
                                        	 	c_rout_str,      
                                         		c_trd_dt.arr,
                                         		c_skpord_ref,
                                         		i_dwnld_flg,
                                         		c_ServiceName,
                                         		c_err_msg );

        if(i_ret_val == -1)
        {
          if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
          {
            fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
          }
        }

			}
			
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"After Successful Call To fn_ord_cnfrmtn");
			}

			break;

		
		case TRADE_CONFIRMATION_TR :

			if(DEBUG_MSG_LVL_3)  
			{
				fn_userlog(c_ServiceName,"Inside TRADE_CONFIRMATION Case TRIMMED PROTOCOL");
			}

			MEMSET(st_trd_conf_tr_data);
			memcpy (&st_trd_conf_tr_data,ptr_car_sbuf,sizeof (struct st_trade_confirm_tr ));

			fn_cnvt_nth_trade_confirm_tr(&st_trd_conf_tr_data);

			if(DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName,"Exchange Ack Is :%lf:",st_trd_conf_tr_data.d_response_order_number);  
			}

			if(i_dwnld_flg == DOWNLOAD && i_deferred_dwnld_flg == 1 )
 			{
				if(DEBUG_MSG_LVL_0)
				{
    			fn_userlog(c_ServiceName,"Inside Deffered Download Condition");
				}

    		if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
    		{
      		fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
    		}
    		tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf ); 
    		tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  		}
	

			sprintf(c_xchng_ack,"%16.0lf",st_trd_conf_tr_data.d_response_order_number);		

  		rtrim(c_xchng_ack);			

      if( DEBUG_MSG_LVL_0)  
      {
  		  fn_userlog(c_ServiceName,"Exchange Ack Is :%s:",c_xchng_ack);		
      }
			
  		EXEC SQL                                  
    		SELECT  NVL(FOD_SPN_FLG,'N')
    		INTO    :c_spn_flg
   	 		FROM    FOD_FO_ORDR_DTLS
    		WHERE   FOD_ACK_NMBR  = :c_xchng_ack
				AND     ROWNUM < 2;											

			if ( SQLCODE != 0 && SQLCODE!= NO_DATA_FOUND)
      {
        fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");

        if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
        tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );  
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );

      }

      if ( c_spn_flg != 'S' )
      {
				i_ret_val = fn_trd_confirmation_trim ( &st_trd_conf_tr_data,
																					c_xchng_cd,
          	           							      c_pipe_id,
																					c_rout_str,			
																				  c_trd_dt.arr,
																					c_trd_ref.arr,
																		      i_dwnld_flg,
                    								      c_ServiceName,
                    								      c_err_msg );
			}
			else
			{
				i_ret_val = fn_spntrd_cnfrmtn_trim	( &st_trd_conf_tr_data,
        	                             	c_xchng_cd,
                                       	c_pipe_id,
                                       	c_rout_str,   
                                       	c_trd_dt.arr,
                                       	c_trd_ref.arr,
                                       	i_dwnld_flg,
                                       	c_ServiceName,
                                       	c_err_msg );


			}
	
			if(i_ret_val == -1)
      {
        if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
      }
			
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"After Successful Call To fn_trd_cnfrmtn");
			}
			break;
    /*** ver 2.3 ends ***/
		case TRADE_MOD_OUT :
		case TRADE_CANCEL_OUT :
		case TRADE_ERROR :

			break;


		case SP_BOARD_LOT_OUT :
		case SP_ORDER_MOD_OUT :
		case TWOL_BOARD_LOT_OUT :
		case THRL_BOARD_LOT_OUT :

			break;

		
		case SP_ORDER_CONFIRMATION :
		case SP_ORDER_ERROR :
		case TWOL_ORDER_CONFIRMATION :
		case TWOL_ORDER_ERROR :
		case THRL_ORDER_CONFIRMATION :
		case THRL_ORDER_ERROR :
		case SP_ORDER_CXL_CONFIRM_OUT :
		case TWOL_ORDER_CXL_CONFIRMATION :
		case THRL_ORDER_CXL_CONFIRMATION :
		case SP_ORDER_MOD_CON_OUT:    /*** Ver 2.4 ***/
    case SP_ORDER_MOD_REJ_OUT:       /*** Ver 2.4 ***/
    case SP_ORDER_CANCEL_REJ_OUT: /*** Ver 2.4 ***/
    case BATCH_SPREAD_CXL_OUT: /*** Ver 2.4 suchita ***/
			if(DEBUG_MSG_LVL_3)  /*** Ver 2.0, debug level changed from 0 to 3 ***/
			{
				fn_userlog(c_ServiceName,"Inside SPREAD ORDER CONFERMATION/REJECTION Case ");
			}

			MEMSET(st_spd_reqres_data);
			
			memcpy (&st_spd_reqres_data,ptr_car_sbuf,sizeof (struct st_spd_oe_reqres ));

			fn_cnvt_nth_spd_oe_reqres(&st_spd_reqres_data);		/*** Ver 2.3 ***/

			if(DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName,"Spread Order Reference Is :%s:",st_spd_reqres_data.c_oe_remarks);
			}

			if(i_dwnld_flg == DOWNLOAD && i_deferred_dwnld_flg == 1 )
  		{
    		fn_userlog(c_ServiceName,"Inside Deffered Download Condition");
    		if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
    		{
      		fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
    		}
    		tpfree ( ( char * )ptr_fml_ibuf);
	    	tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
    		tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  		}

			if(i_dwnld_flg == DOWNLOAD && (strcmp(c_skpord_ref,st_spd_reqres_data.c_oe_remarks)==0))
			{
				if(DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName,"Inside Skip Order Reference Case");
					/*** fn_userlog(c_ServiceName,"DownLoad Flag Is :%ld:",i_dwnld_flg); *** Ver 2.2 ***/
					fn_userlog(c_ServiceName,"DownLoad Flag Is :%d:",i_dwnld_flg);  /*** Ver 2.2 ***/
					fn_userlog(c_ServiceName,"Order Reference To Be Skipped Is :%s:",c_skpord_ref);   
				}
				break;

			}
			
			/******** 2L Order OEREMARK Changes Starts ************/
			
			MEMSET(c_spd_ord_seq);
			
		  EXEC SQL
      	SELECT to_char(EXG_NXT_TRD_DT,'yyyymmdd') ||:c_pipe_id
      	INTO   :c_spd_ord_seq
      	FROM   EXG_XCHNG_MSTR
     		WHERE  EXG_XCHNG_CD   = :c_xchng_cd;

  		if(SQLCODE != 0)
  		{
				tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  		}

  		if(DEBUG_MSG_LVL_0)
  		{
    		fn_userlog(c_ServiceName,"spread order seq before sprintf=:%s:",st_spd_ordbk.c_sprd_ord_rfrnc);
  		}
  		memset ( st_spd_ordbk.c_sprd_ord_rfrnc, 0,sizeof (st_spd_ordbk.c_sprd_ord_rfrnc) );
  		sprintf(st_spd_ordbk.c_sprd_ord_rfrnc[0],"%s%08d",c_spd_ord_seq,st_spd_reqres_data.i_sprd_seq_no);
  		if(DEBUG_MSG_LVL_0)
 		 	{
    		fn_userlog(c_ServiceName,"spread order seq after sprintf=:%s:",st_spd_ordbk.c_sprd_ord_rfrnc);
  	 	}

  		/*** 2L Order OEREMARK Changes Ends ***/

	
 			EXEC SQL                    /***  Ver 1.4 ***/
        SELECT  NVL(FSD_SPN_FLG,'N')
        INTO    :c_spn_flg
        FROM    FSD_FO_SPRD_DTLS
        /** WHERE  FSD_SPRD_RFRNC	=	:st_spd_reqres_data.c_oe_remarks commented For 2L Order OEREMARK Change*/ 
				WHERE   FSD_SPRD_RFRNC	=	:st_spd_ordbk.c_sprd_ord_rfrnc[0]   /*** 2L Order OEREMARK Change ***/
				AND			rownum <2;

		 	if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
      {
        fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");

        if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
        tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );

      }

      if(DEBUG_MSG_LVL_0)  /*** Ver 2.0 ***/ 
      {
			  fn_userlog(c_ServiceName,"SPREAD SPAN FLAG IS :%c:",c_spn_flg);
	    }

			if ( c_spn_flg != 'S' )
			{
				i_ret_val = fn_sprd_ord_confirmation ( &st_spd_reqres_data,
																							 c_xchng_cd,
																							 c_pipe_id,
																							 c_rout_str,					/*** Ver 1.1 Routing Added ***/
																							 c_trd_dt.arr,
                	          					         i_dwnld_flg,
                  	       								     c_ServiceName,
                    	     								     c_err_msg );

			}
			else
			{
				i_ret_val = fn_spnsprd_ord_confirmation ( 	&st_spd_reqres_data,
                                               			c_xchng_cd,
                                               			c_pipe_id,
                                               			c_rout_str, 
                                               			c_trd_dt.arr,
                                               			i_dwnld_flg,
                                               			c_ServiceName,
                                               			c_err_msg );
			}

			if(i_ret_val == -1)
      {
        if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
      }

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"After Successful Call To fn_sprd_ord_cnfrmtn ");
			}
			break;
			
		case ON_STOP_NOTIFICATION :

			if(DEBUG_MSG_LVL_3) /*** Ver 2.0, debug level changed from 0 to 3 ***/ 
			{
				fn_userlog(c_ServiceName,"Inside ON_STOP_NOTIFICATION Case ");
			}

			MEMSET(st_trd_conf);
			memcpy (&st_trd_conf,ptr_car_sbuf,sizeof (struct st_trade_confirm ));

			fn_cnvt_nth_trade_confirm(&st_trd_conf);		/*** Ver 2.3 ***/
	
			if(DEBUG_MSG_LVL_0)
			{		
				/*** fn_userlog(c_ServiceName,"Exchange Ack Is :%d:",st_trd_conf.d_response_order_number); *** Ver 2.2 **/
				fn_userlog(c_ServiceName,"Exchange Ack Is :%lf:",st_trd_conf.d_response_order_number);  /*** Ver 2.2 ***/
			}
			
			if(i_dwnld_flg == DOWNLOAD && i_deferred_dwnld_flg == 1 )
  		{
				if(DEBUG_MSG_LVL_0)
				{
    			fn_userlog(c_ServiceName,"Inside Deffered Download Condition");
				}

    		if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
    		{
      		fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
    		}
    		tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
    		tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  		}


			/** Version 1.3**/
      /**sleep(3);       **Added to differ execution till order confirmation is processed **/

			MEMSET(c_xchng_ack);																								    /***  Ver 1.7 **/
			sprintf(c_xchng_ack,"%16.0lf",st_trd_conf.d_response_order_number);     /***  Ver 1.7 **/
			rtrim(c_xchng_ack);  																									  /***  Ver 1.7 **/

			 EXEC SQL                    /***  Ver 1.4 ***/
        SELECT  NVL(FOD_SPN_FLG,'N')
        INTO    :c_spn_flg
        FROM    FOD_FO_ORDR_DTLS
        WHERE   FOD_XCHNG_CD  = :c_xchng_cd
        AND     FOD_ACK_NMBR	= :c_xchng_ack; /* Ver 1.7  variable st_trd_conf.d_response_order_number replaced by c_xchng_ack **/

      if ( SQLCODE != 0 )
      {
        fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");

        if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
        tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );

      }
			/** i_dwnld_flg = NOT_DOWNLOAD; ** Ver 1.7 ** Commented in Ver 1.9 **/

      if ( c_spn_flg != 'S' )
      {

				i_ret_val =  fn_stpls_trg ( &st_trd_conf,
																		c_xchng_cd,
																		c_pipe_id,
																		c_rout_str,			/*** Ver 1.1 Routing Added ***/
																		c_trd_dt.arr,
																		TRIGGERED,
                  	 								c_ServiceName,
																		i_dwnld_flg,
                   									c_err_msg ); 
			}
			else
			{
				 i_ret_val =  fn_spnstpls_trg ( &st_trd_conf,
                                    		c_xchng_cd,
                                    		c_pipe_id,
                                    		c_rout_str,
                                    		c_trd_dt.arr,
                                    		TRIGGERED,
                                    		c_ServiceName,
                                    		i_dwnld_flg,
                                    		c_err_msg );

			}

			if(i_ret_val == -1)
      {
        if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
      }
			
			if(DEBUG_MSG_LVL_3)
			{
      	fn_userlog(c_ServiceName,"After Successful Call To fn_stpls_trg");
			}

			break;

		case TRADE_CONFIRMATION :

			if(DEBUG_MSG_LVL_3)  /*** Ver 2.0, debug level changed from 0 to 3 ***/
			{
				fn_userlog(c_ServiceName,"Inside TRADE_CONFIRMATION Case");
			}

			MEMSET(st_trd_conf);
			memcpy (&st_trd_conf,ptr_car_sbuf,sizeof (struct st_trade_confirm ));

			fn_cnvt_nth_trade_confirm(&st_trd_conf);		/*** Ver 2.3 ***/

			if(DEBUG_MSG_LVL_0)
			{
				/*** fn_userlog(c_ServiceName,"Exchange Ack Is :%d:",st_trd_conf.d_response_order_number); *** Ver 2.2 ***/
				fn_userlog(c_ServiceName,"Exchange Ack Is :%lf:",st_trd_conf.d_response_order_number);  /*** Ver 2.2 ***/
			}

			if(i_dwnld_flg == DOWNLOAD && i_deferred_dwnld_flg == 1 )
 			{
				if(DEBUG_MSG_LVL_0)
				{
    			fn_userlog(c_ServiceName,"Inside Deffered Download Condition");
				}

    		if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
    		{
      		fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
    		}
    		tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
    		tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  		}

			/** Version 1.3**/
      /**sleep(1); **Introduced to avoid bulk trade sudden choking case **/						/*** Ver 1.2 ***/

			

			sprintf(c_xchng_ack,"%16.0lf",st_trd_conf.d_response_order_number);			/***  Ver 1.4 ***/

  		rtrim(c_xchng_ack);			/***  Ver 1.4 ***/

      if( DEBUG_MSG_LVL_0)  /** Ver 2.0 **/
      {
  		  fn_userlog(c_ServiceName,"Exchange Ack Is :%s:",c_xchng_ack);			/***  Ver 1.4 ***/
      }
			
  		EXEC SQL                                  /***  Ver 1.4 ***/
    		SELECT  NVL(FOD_SPN_FLG,'N')
    		INTO    :c_spn_flg
   	 		FROM    FOD_FO_ORDR_DTLS
    		WHERE   FOD_ACK_NMBR  = :c_xchng_ack
				AND     ROWNUM < 2;												/**Ver 1.5 **/

			if ( SQLCODE != 0 && SQLCODE!= NO_DATA_FOUND)
      {
        fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");

        if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
        tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );

      }

      if ( c_spn_flg != 'S' )
      {
				i_ret_val = fn_trd_confirmation ( &st_trd_conf,
																					c_xchng_cd,
          	           							      c_pipe_id,
																					c_rout_str,			/*** Ver 1.1 Routing Added ***/
																				  c_trd_dt.arr,
																					c_trd_ref.arr,
																		      i_dwnld_flg,
                    								      c_ServiceName,
                    								      c_err_msg );
			}
			else
			{
				i_ret_val = fn_spntrd_cnfrmtn	( &st_trd_conf,
        	                             	c_xchng_cd,
                                       	c_pipe_id,
                                       	c_rout_str,   
                                       	c_trd_dt.arr,
                                       	c_trd_ref.arr,
                                       	i_dwnld_flg,
                                       	c_ServiceName,
                                       	c_err_msg );


			}
	
			if(i_ret_val == -1)
      {
        if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
      }
			
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"After Successful Call To fn_trd_cnfrmtn");
			}
			break;

		case CTRL_MSG_TO_TRADER :

			/** Ver 1.6 starts **/

			if(DEBUG_MSG_LVL_3)
			{
      	fn_userlog(c_ServiceName,"Inside Case CTRL_MSG_TO_TRADER");
			}

    	fn_userlog(c_ServiceName,"Calling function fn_intgnrltrd_msg ");

      memcpy (&st_trd_msg,ptr_car_sbuf,sizeof (struct st_trader_int_msg));

			fn_cnvt_nth_trader_int_msg(&st_trd_msg);		/*** Ver 2.3 ***/

      i_ret_val = fn_intgnrltrd_msg( &st_trd_msg,
                                     c_xchng_cd,
                                     c_pipe_id,
                                     c_ServiceName,
                                     c_err_msg);

      if ( i_ret_val == -1 )
      {
        fn_userlog(c_ServiceName,"Failed While Calling fn_gnrltrd_msg");
        fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
      }

			/** Ver 1.6 ends **/

			break;

		case EX_PL_ENTRY_OUT :
		case EX_PL_MOD_OUT :
		case EX_PL_CXL_OUT :

			if(DEBUG_MSG_LVL_3)  /** ver 2.0, debug level changed from 0 to 3 **/
			{
				fn_userlog(c_ServiceName,"Inside Exercise Response Case");
			}

			MEMSET(st_expl_reqres_data );

			memcpy (&st_expl_reqres_data,ptr_car_sbuf,sizeof (struct st_ex_pl_reqres ));

			fn_cnvt_nth_ex_pl_reqres(&st_expl_reqres_data);		/*** Ver 2.3 ***/

			if(i_dwnld_flg == DOWNLOAD && i_deferred_dwnld_flg == 1 )
      {
				if(DEBUG_MSG_LVL_0)
				{
        	fn_userlog(c_ServiceName,"Inside Deffered Download Condition");
				}

        if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
        tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
      }


			if(i_dwnld_flg == DOWNLOAD && (strcmp(c_skpord_ref,st_expl_reqres_data.st_ex_pl_data.c_remarks)==0))
      {
				if(DEBUG_MSG_LVL_0)
				{
        	fn_userlog(c_ServiceName,"Inside Skip Order Reference Case");
        	/** fn_userlog(c_ServiceName,"DownLoad Flag Is :%ld:",i_dwnld_flg); *** Ver 2.2 ***/
        	fn_userlog(c_ServiceName,"DownLoad Flag Is :%d:",i_dwnld_flg);     /*** Ver 2.2 ***/
        	fn_userlog(c_ServiceName,"Order Reference To Be Skipped Is :%s:",c_skpord_ref);
				}
        break;

      }

			EXEC SQL                    /***  Ver 1.4 ***/
        SELECT  NVL(FXB_SPN_FLG,'N')
        INTO    :c_spn_flg
        FROM    FXB_FO_XCHNG_BOOK
        WHERE   FXB_XCHNG_CD    = :c_xchng_cd
        AND     FXB_PIPE_ID     = :c_pipe_id
        AND     FXB_ORDR_SQNC		= :st_expl_reqres_data.st_ex_pl_data.d_expl_number
        AND     FXB_MOD_TRD_DT  = :c_mod_trd_dt;

			if ( SQLCODE != 0 )
      {
        fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");

        if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
        tpfree ( ( char * )ptr_fml_ibuf);
    		tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );

      }

			if ( c_spn_flg != 'S' )
			{
				i_ret_val =  fn_expl_reqres ( &st_expl_reqres_data,
																			c_xchng_cd,
          	           								c_pipe_id,
																			c_rout_str,			/*** Ver 1.1 Routing Added ***/
																			c_trd_dt.arr,
                	     								i_dwnld_flg,
                  	  							  c_ServiceName,
                    									c_err_msg );
			}

			/**********************************
			else
			{
				i_ret_val =  fn_spnexpl_reqres ( &st_expl_reqres_data,
                                      c_xchng_cd,
                                      c_pipe_id,
                                      c_rout_str, 
                                      c_trd_dt.arr,
                                      i_dwnld_flg,
                                      c_ServiceName,
                                      c_err_msg );


			}

			*********************************************/

			if(i_ret_val == -1)
      {
        if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
      }

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"After Successful Call To fn_expl_reqres");
			}

			break;

		case EX_PL_CXL_CONFIRMATION :
		case EX_PL_MOD_CONFIRMATION :
		case EX_PL_CONFIRMATION :
		case EX_PL_ENTRY_IN :

			if(DEBUG_MSG_LVL_3) /** ver 2.0, debug level changed from 0 to 3 **/
			{
      	fn_userlog(c_ServiceName,"Inside Exercise Conformation Case ");
			}

			MEMSET (st_expl_reqres_data);

			memcpy (&st_expl_reqres_data,ptr_car_sbuf,sizeof (struct st_ex_pl_reqres ));

			fn_cnvt_nth_ex_pl_reqres(&st_expl_reqres_data);   /*** Ver 2.3 ***/

			if(i_dwnld_flg == DOWNLOAD && i_deferred_dwnld_flg == 1 )
      {
				if(DEBUG_MSG_LVL_0)
				{
        	fn_userlog(c_ServiceName,"Inside Deffered Download Condition");
				}
        if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
        tpfree ( ( char * )ptr_fml_ibuf);
        tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
      }

			/** Version 1.3**/
      /**sleep(3);       Added to differ the execution till exercise confirmation first response is processed **/

			EXEC SQL                    /***  Ver 1.4 ***/
        SELECT  NVL(FXB_SPN_FLG,'N')
        INTO    :c_spn_flg
        FROM    FXB_FO_XCHNG_BOOK
        WHERE   FXB_XCHNG_CD    = :c_xchng_cd
        AND     FXB_PIPE_ID     = :c_pipe_id
        AND     FXB_ORDR_SQNC   = :st_expl_reqres_data.st_ex_pl_data.d_expl_number
        AND     FXB_MOD_TRD_DT  = :c_mod_trd_dt;

      if ( SQLCODE != 0 )
      {
        fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");

        if(tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
        tpfree ( ( char * )ptr_fml_ibuf);
        tpfree ( (char *)ptr_car_sbuf );   /** Ver 2.1 **/
        tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );

      }

			if	(	c_spn_flg	!=	'S'	)
			{
				i_ret_val =  fn_expl_confirmation ( &st_expl_reqres_data,
																						c_xchng_cd,
          	            							 			c_pipe_id,
																						c_rout_str,			/*** Ver 1.1 Routing Added ***/
																					  c_trd_dt.arr,
                	      							 			c_ServiceName,
                  	    							 			i_dwnld_flg,
                    	  							 			c_err_msg );
			}

			/**********************************************
			else
			{
				i_ret_val =  fn_spnexpl_confirmation ( 	&st_expl_reqres_data,
                                            		c_xchng_cd,
                                            		c_pipe_id,
                                            		c_rout_str,
                                            		c_trd_dt.arr,
                                            		c_ServiceName,
                                            		i_dwnld_flg,
                                            		c_err_msg );

			}

			************************************************************/

			if(i_ret_val == -1)
      {
        if ( tpacall("SVC_ENQ_MSG", (char *)ptr_fml_ibuf,  0, TPNOTRAN|TPNOREPLY) == -1)
        {
          fn_userlog(c_ServiceName,"S3130",TPMSG, DEF_USR, DEF_SSSN, c_err_msg);
        }
      }

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"After Successful Call To fn_expl_cnfrmtn");
			}
			break;

		case GIVEUP_APPROVED_OUT :
 		case GIVEUP_APP_CONFIRM :
 		case GIVEUP_REJECTED_OUT :
 		case GIVEUP_APPROVE_ALL :	

			break;

		default:
		fn_userlog( c_ServiceName, "Protocol error %d", st_hdr.si_transaction_code);
			i_ret_val = -1;
		break;
	}

  if(DEBUG_MSG_LVL_3) /** Ver 2.0, added in debug **/
  {
	  fn_userlog(c_ServiceName,"End Of Switch Case");
  }

  tpfree ( (char *)ptr_car_sbuf );
  tpfree ( (char *)ptr_fml_ibuf );

  if ( i_ret_val == -1 )
  {
    tpreturn(TPFAIL, 0, (char *)NULL, 0, 0);
  }

  tpreturn(TPSUCCESS, 0, (char *)NULL, 0, 0);
}
