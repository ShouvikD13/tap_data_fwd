/*	Version 1.1 - 13-Aug-2009	 Routing Added									Sandeep Patil			*/
/*	Version	1.2	-	18-Aug-2009	 Prematured Trde Handled   			Sandeep Patil     */
/*	Version 1.3 - 14-Sep-2009  Function Call Handeled					Sandeep	Patil			*/
/*	Version 1.4	-	07-Oct-2009	 Intractive Message Handeled		Sandeep Patil			*/
/*	Version 1.5 - 30-Oct-2009  Sleep Removed             			Sangeet           */
/*	Version	1.6	-	08-Mar-2010	 Jiffy Incremented By 1					Sandeep Patil			*/
/*														 In Case Of 2L Order															*/
/*														 Cancellation																			*/
/*	Version	1.7	-	10-Mar-2010	 NNF Message DownLoad Changes 	Shailesh Hinge		*/
/*	Version	1.8	-	01-Jul-2010	 Stream No. Check Added In			Sandeep	Patil			*/
/*														 Case Of Download	.																*/
/*	Version	1.9	-	30-Sep-2010	 NNF Changes Data Type Of 			Sandeep Patil			*/
/*														 User ID Changed From Short												*/
/*														 Int To Long.																			*/
/*	Version	2.0	-	28-Feb-2011	Silver Light Changes.						Sandeep Patil			*/
/*  Version 2.1 - 06-Feb-2012	2L Order OEREMARK Change				Sandip Tambe			*/
/*  Version 2.2 - 05-Mar-2012	SPAN Change                     Swati A.B         */
/*  Version 2.3 - 16-May-2012 Fix for Trade Mismatch and      Navina D.         */
/*                            solicited cancellation response                   */
/*                            received in download                              */
/*  Version 2.4 - 13-Jun-2012 FNO BSE Changes                                   */
/*  Version 2.5 - 28-Nov-2012 SLTP FuturePLUS Handling			  Navina D.         */
/*  Version 2.6 - 04-Dec-2012 Broker closeout changes         Mahesh Shinde     */
/*  Version 2.7 - 26-Dec-2013 Histry Search removed           Sangeet Sinha     */
/*  Version 2.8 - 14-Feb-2013 Exchange initiated expiry       Navina D.					*/
/*  Version 2.9 - 03-May-2013 Broker closeout &RRM changes    Mahesh Shinde.		*/
/*  Version 3.0 - 25-Sep-2013 OptionPLUS Handling							Navina D.         */
/*	Version 3.1 - 22-Apr-2014 "=" Replaced by "=="															*/ 
/*														For IOC order cancellation	    Bhushan S.				*/
/*  Version 3.2 - 22-Apr-2014 function call fn_ref_to_ord                       */
/*                            commented as data already rcvd                    */
/*                            by exchange and fn_seq_to_omd    Vishnu N.        */
/*  Version 3.3 - 05-May-2014 Trade Execution range hndlng    Swati A.B         */
/*	Version 3.4	- 20-May-2014	Function call fn_seq_to_omd 											*/
/*														commented and data is retrieved										*/
/*														while updating exchange book		 Vishnu N					*/
/*  Version 3.5 - 06-Jun-2014 Unsolicated cancellation resp                     */
/*                            for sltp futureplus & Oplus     Navina D.         */
/*  Version 3.6 - 27-Jun-2014 Display SLTP Remark With Time   Bhushan Sonwane   */
/*  Version 3.8 - 28-Oct-2014 Check added to restrict                           */
/*                            processing of expiry response                     */
/*                            received from exchange if order                   */
/*                            is already expired in EBA       Anand Dhopte      */
/*  Version 3.9 - 31-Aug-2015 Cancellation due to SELF Trade  Navina D.         */
/*	Version 4.1 - 05-Nov-2015 Calling ACK in triggered response Tanmay W.				*/
/*  Version 4.2 - 16-Dec-2015 Sleep time changed in trade confirmation (Sachin Birje)*/
/*  Version 4.3 - 22-Jan-2016 Trade insert and process time updation   (Sachin Birje)*/
/*  Version 4.4 - 02-Feb-2015 Change sfo_prcs_trd call to acall   (Bhupendra )  */
/*  Version 4.5 - 25-Apr-2016 Added nowait query and sleep time reduced  (Aakash Chavan/sachin Birje)  */
/*  Version 4.7 - 01-Jun-2016 Order Reference number selected from FXB for order */
/*                            freez response processing (Sachin birje)           */
/*  Version 4.8 - 02-Jun-2016 Update Ack Number only for New Order Accept &			*/ 
/*														Reject Response Received From Exchange.						*/ 
/*														(Ritesh Deolekar)  						 										*/
/*  Version 4.9 - 12-Jul-2016 Debug Level Changes (Sachin Birje)                */
/*  Version 5.0 - 25-Aug-2016 Future SLTP Trail order changes ( Sachin Birje )                          */
/*  Version 5.1 - 12-Sep-2016 16388 error code handling from error queue (Tanmay .W)*/
/* 	Version 5.3 - 25-Sep-2016 ORS handelling (Tanmay .W)																							  */			
/* 	Version 5.4 - 25-Nov-2016 ORS handelling(seq zero && 16388 with spread seq) (Tanmay .W)  */			
/* 	Version 5.5 - 06-Oct-2017 Core Fix (Sachin Birje.)                          */
/*  Version 5.6 - 24-Apr-2018 Processing solicited cancellation response in download ( Suchita D.)*/
/*  Version 5.7 - 14-Mar-2018 Rollover with Spread Changes (Sachin Birje)       */
/*  Version 5.8 - 22-May-2018 Processing ORDER_ERROR_OUT response which remained unprocessed
                              due to "Resource Busy" in case of freezed order response( Suchita )*/
/*  Version 5.9 - 06-Jun-2018 Skipping fn_seq_to_omd for 2L/3L Order partial cancellation(Suchita)*/  
/*  Version 6.0 - 07-Feb-2019 Using traded qty available in cancellation response packet for solicited cancellation response processing.(Suchita Dabir)                                        */
/*  Version 6.1 - 14-Feb-2019 Trimmed Protocol for Non-NEAT Front End  ( Suchita Dabir / Parag K.) */
/*  Version 6.3 - 14-May-2019 Order not expired in exchange initiated expiry    */
/*                due to multiple 99 modification counter(Suchita Dabir)        */
/*  Version 6.4 - 31-Aug-2019 Last Activity Ref Changes (Parag Kanojia)         */
/*  Version 6.5 - 20-Sep-2019 Last Activity Ref LIVE BUG FIX FOR SLTP (Anuj Pancholi) */
/*  Version 6.6 - 26-Apr-2020 Stream number check removed in Download check (Sachin Birje)*/
/*  Version 6.7 - 28-May-2020 Added order seq/ref no in download condition  (Sachin Birje)*/
/*  Version 6.8 - 16-JUL-2020 Unsolicited cancellation with error code 0 ( Suchita D ) */
/*  Version 6.9 - 07-Sep-2020 CR-ISEC18-138913 IOC cancellation display (Suchita Dabir ) */
/*  Version 7.0 - 04-Nov-2020 Rollover with Spread P-Exec Cancellation Bug Fix (Mrinal Kishore)*/
/*  Version 7.1 - 02-Sep-2021 Stream Number changes to read as number (Sachin Birje) */
/*  Version 7.2 - 03-Nov-2022 LPP Error Messages Changes  (Sachin Birje) */
/*  Version 7.3 - 07-Mar-2023 Minor Change Client Lock Changes (Sachin Birje) */
/*  Version TOL - 01-Aug-2023 Tux on Linux (Agam) */
/********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <brd_cln_srvr.h>
#include <fn_scklib_tap.h>
#include <time.h>

#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>

#include <sqlca.h>

#include <fo_fml_def.h>
#include <fo_view_def.h>
#include <fo.h>
#include <fn_env.h>
#include <fn_tuxlib.h>
#include <fn_fil_log.h>
#include <fn_ddr.h>
#include <fn_log.h>
#include <fml_rout.h>
#include <userlog.h>
#include <fn_session.h>
#include <fo_exg_rcv_lib.h>
#include <fn_read_debug_lvl.h>
#include <fn_ors_rms.h>  /** ver 5.0 **/
#include <sys/time.h>  /* ver TOL Tux on linux */

EXEC SQL INCLUDE "table/fxb_fo_xchng_book.h"; /**  Ver 3.4 ***/

// VER TOL : TUX on LINUX -- Following function names changed (Ravindra) 
// (1) fn_upd_xchngbk -> fn_upd_xchngbk _erl
// (2) fn_fxb_rms -> fn_fxb_rms_erl
// (3) fn_ftd_rms -> fn_ftd_rms_erl

int fn_ref_to_ord(struct vw_orderbook *ptr_st_orderbook,
                  char *c_ServiceName,
                  char *c_err_msg);

int fn_seq_to_omd(struct vw_xchngbook *ptr_st_xchngbook,
                  char *c_ServiceName,
                  char *c_err_msg);

int fn_upd_xchngbk_erl( struct vw_xchngbook *ptr_st_xchngbook,
										char *c_lst_act_ref,			/*** Ver 6.1 ***/
                    char *c_ServiceName,
                    char *c_err_msg);

int fn_upd_ordrbk(struct vw_orderbook *ptr_st_orderbook,
                  char *c_ServiceName,
                  char *c_err_msg);

int fn_ack_to_ord ( struct vw_orderbook *ptr_st_orderbook,
                    char *c_ServiceName,
										char *c_usr_id,																/***	Ver	2.0	***/
                    char *c_err_msg);

int fn_upd_trdbk	(	char *c_ServiceName,			/***	Ver	2.0	***/
                    char *c_err_msg,
										char *c_usr_id,
										char *c_lst_act_ref,      /*** Ver 6.1 ***/
										struct vw_tradebook *ptr_st_i_trdbk);

int fn_ac_to_ord (	char *c_ServiceName,			/***	Ver	2.0	***/
                    char *c_err_msg,
                    char *c_usr_id,
                    struct vw_orderbook *ptr_st_orderbook   
                 );

int fn_fxb_rms_erl   (  char *c_ServiceName,      /***  Ver 5.3 ***/
                    char *c_err_msg,
                    char c_prd_typ,
                    char *c_ordr_rfrnc                
                 );

int fn_ftd_rms_erl  (		char *c_ServiceName,       /**** ver 5.3 ****/
                    char *c_err_msg,
                    char c_prd_typ,
                    char *c_ordr_rfrnc
								);



void fn_gettime(void);

char c_time [12];

/******************************************************************************/
/*  To update open order details like GTD / GTDs orders in database.					*/
/*  INPUT PARAMETERS                                                          */
/*      ptr_oe_reqres - Pointer to a message structure got from Exchange  	  */
/*      c_ServiceName - Service Name currently running                        */
/*      c_err_msg     - Error message to be returned                          */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/

int fn_opn_ord_dtls ( struct st_oe_reqres *ptr_oe_reqres,
											char *c_pipe_id,
											char *c_rout_str,
                      char *c_ServiceName,
                      char *c_err_msg )
{
	int i_ch_val;

	struct vw_orderbook st_i_ordbk;

	if(DEBUG_MSG_LVL_3)  /*** Ver 4.9 ** debug changed form 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside Function fn_opn_ord_dtls");
	}

	sprintf( st_i_ordbk.c_xchng_ack, "%16.0lf",ptr_oe_reqres->d_order_number);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Exchange Ack Is :%s:",st_i_ordbk.c_xchng_ack);
		fn_userlog(c_ServiceName,"Total Volume :%ld:",ptr_oe_reqres->li_volume);
		fn_userlog(c_ServiceName,"Total Volume Remaining :%ld:",ptr_oe_reqres->li_total_volume_remaining);
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	st_i_ordbk.l_exctd_qty = ptr_oe_reqres->li_volume - ptr_oe_reqres->li_total_volume_remaining;
	st_i_ordbk.l_ord_tot_qty = ptr_oe_reqres->li_volume;
	strcpy(st_i_ordbk.c_pipe_id, c_pipe_id);

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Pipe ID is :%s:",c_pipe_id);
		fn_userlog(c_ServiceName,"Executed Qty Is :%ld:",st_i_ordbk.l_exctd_qty);
		fn_userlog(c_ServiceName,"Total Order Qty Is :%ld:",st_i_ordbk.l_ord_tot_qty);
	}

	/*fn_cpy_ddr(st_i_ordbk.c_rout_crt); */			/*** Ver 1.1 ***/

	strcpy(st_i_ordbk.c_rout_crt,c_rout_str);		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Before Call To SFO_OPN_ORD");
	}

	i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_err_msg,
                            &st_i_ordbk,
                            "vw_orderbook",
                            sizeof ( st_i_ordbk ),
                            TPNOREPLY,
                            "SFO_OPN_ORD" );
  if ( i_ch_val != 0 )
  {
    fn_errlog(c_ServiceName,"L31005", LIBMSG, c_err_msg);
    return -1;
  }

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"After Complition Of fn_opn_ord_dtls");
	}

  return 0;
}

/******************************************************************************/
/* To update the accept and reject responses from Exchange.				            */
/*  INPUT PARAMETERS                                                          */
/*      ptr_oe_reqres - Pointer to a message structure got from Exchange   		*/
/*			c_pipe_id     - Pipe Id																								*/
/*			i_dwnld_flg   - Down Load Flag																				*/
/*      c_ServiceName - Service Name currently running                        */
/*      c_err_msg     - Error message to be returned                          */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/

int fn_ord_confirmation ( struct st_oe_reqres *ptr_oe_reqres,
										 			char   *c_xchng_cd,
										 			char   *c_pipe_id,
													char   *c_rout_str,
										 			char   *c_trd_dt,
													char   *c_skpord_ref,
										 			int    i_dwnld_flg,
                     			char   *c_ServiceName,
                     			char   *c_err_msg )

{
	char c_ordr_rfrnc [19];
	char c_xchng_rmrks[256];

	int i_ch_val;
  int i_ord_ack=0;   /** Ver 7.2 **/

	long l_xchng_can_qty;

	struct vw_xchngbook st_i_xchngbk;
  char c_prd_typ = '\0'; /*** version 5.8 ***/

  /*** ver 6.8 starts ***/
  char c_fod_spec_flg='\0';
  char c_fod_order_ref[30]="\0";
  char c_ord_ack_nse[20]="\0";
  /*** ver 6.8 ends ***/

	MEMSET(st_i_xchngbk);
	MEMSET(c_xchng_rmrks);

	if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug level changed from 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside Function fn_ord_cnfrmtn");
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"I/P Exchange Code Is :%s:",c_xchng_cd);
		fn_userlog(c_ServiceName,"I/P Pipe ID Is :%s:",c_pipe_id);
		fn_userlog(c_ServiceName,"I/P Trade Date :%s:",c_trd_dt);
	}

	strcpy(st_i_xchngbk.c_xchng_cd,c_xchng_cd);
	strcpy(st_i_xchngbk.c_pipe_id,c_pipe_id);
	strcpy(st_i_xchngbk.c_mod_trd_dt,c_trd_dt);
	st_i_xchngbk.l_ord_seq = ptr_oe_reqres->i_ordr_sqnc;

	if(DEBUG_MSG_LVL_0)
	{
		/** fn_userlog(c_ServiceName,"Sequence Number Is :%ld:",st_i_xchngbk.l_ord_seq); ** Ver 4.9 **/
    fn_userlog(c_ServiceName,"fn_ord_cnfrmtn:Sequence Number Is :%ld:",st_i_xchngbk.l_ord_seq); /** Ver 4.9 **/
	}

	st_i_xchngbk.c_oprn_typ = FOR_RCV;

	/***	Commented In Ver 1.3
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
	*************************************************************/

	/***  Commented in Ver 3.4 data recieved from seq_to_omd is now retrieved while updating exchange book 

	i_ch_val = fn_seq_to_omd (&st_i_xchngbk,c_ServiceName,c_err_msg);

	if ( i_ch_val != 0 )
  {
    fn_errlog(c_ServiceName,"L31010", LIBMSG, c_err_msg);
		return -1;
  }
 
	fn_gettime();

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_SEQ_TO_OMD :%s:",c_time);
	}

 strcpy(c_ordr_rfrnc,st_i_xchngbk.c_ordr_rfrnc);

 ********************************/

	switch (ptr_oe_reqres->st_hdr.si_transaction_code)
	{
		case ORDER_CONFIRMATION_OUT :
		case ORDER_MOD_CONFIRM_OUT :
		/*** case ORDER_ERROR_OUT : *** Ver 7.2 **/
		case ORDER_MOD_REJ_OUT :
		case ORDER_CXL_REJ_OUT :

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside ORDER ACCEPT/REJECT Case");
			}

			if ( i_dwnld_flg == DOWNLOAD && (strcmp(st_i_xchngbk.c_ordr_rfrnc,c_skpord_ref) == 0) )
			{
				break;
			}

			i_ch_val = fn_ord_acpt_rjct(ptr_oe_reqres,
																	st_i_xchngbk,  /** Ver 3.2 **/	
																	c_skpord_ref,	 /** ver 3.4 **/
																	c_xchng_cd,
																	c_pipe_id,
																	c_rout_str,
																	c_trd_dt,
																	c_ordr_rfrnc,
																	i_dwnld_flg,
																	c_ServiceName,
																	c_err_msg);

		  if (i_ch_val == -1)
  		{
   			fn_userlog(c_ServiceName,"Failed While Calling Function fn_ord_acpt_rjct ");
   	 		return -1;
  		}
			
			if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug level changed from 0 to 3 **/
			{
				fn_userlog(c_ServiceName,"After Successful Call To fn_ord_acpt_rjct");
			}

			break;

    case ORDER_ERROR_OUT :   /** Ver 7.2 Starts here ***/

      if(DEBUG_MSG_LVL_3)  
      {
         fn_userlog(c_ServiceName,"After Successful Call To ORDER_ERROR_OUT");
      }

      if ( i_dwnld_flg == DOWNLOAD && (strcmp(st_i_xchngbk.c_ordr_rfrnc,c_skpord_ref) == 0) )
      {
        break;
      }

      if (  ptr_oe_reqres->st_hdr.si_error_code == 17070 ) 
      {
       memset(c_ord_ack_nse,'\0',sizeof(c_ord_ack_nse));
       sprintf(c_ord_ack_nse,"%16.0lf",ptr_oe_reqres->d_order_number);
       sleep(2);   /** Sleep added to wait for stop loss trigger message to process **/
       EXEC SQL
         SELECT  count(1) 
           INTO  :i_ord_ack
           FROM  FXB_FO_XCHNG_BOOK
          WHERE  FXB_XCHNG_CD  = :st_i_xchngbk.c_xchng_cd
            AND  FXB_PIPE_ID   = :st_i_xchngbk.c_pipe_id
            AND  FXB_ORDR_RFRNC IN ( SELECT FOD_ORDR_RFRNC 
                                       FROM FOD_FO_ORDR_DTLS 
                                      WHERE FOD_ACK_NMBR  =:c_ord_ack_nse 
                                        AND FOD_PIPE_ID =:st_i_xchngbk.c_pipe_id)
            AND  FXB_MOD_TRD_DT = to_date(:st_i_xchngbk.c_mod_trd_dt,'dd-Mon-yyyy')
            AND  upper(FXB_XCHNG_RMRKS) like upper('%Stop Loss Order Triggered%'); 
             if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
             {
                  fn_errlog(c_ServiceName,"L91910", LIBMSG, c_err_msg);
                  fn_userlog(c_ServiceName,"CR_139156 ERROR IN FXB FETCH");
                  return -1;
             }

       fn_userlog(c_ServiceName,"i_ord_ack :%d:",i_ord_ack);
 
       if( i_ord_ack > 0 )
       {
        i_ch_val = fn_ord_can ( ptr_oe_reqres,
                                c_xchng_cd,
                                c_pipe_id,
                                c_rout_str,
                                c_trd_dt,
                                c_ordr_rfrnc,
                                i_dwnld_flg,
                                c_ServiceName,
                                c_err_msg);
        if (i_ch_val == -1)
        {
          fn_userlog(c_ServiceName,"Failed While Calling Function fn_spnord_can ");
          return -1;
        }
       }
       else
       {
          i_ch_val = fn_ord_acpt_rjct(ptr_oe_reqres,
                                  st_i_xchngbk,
                                  c_skpord_ref,
                                  c_xchng_cd,
                                  c_pipe_id,
                                  c_rout_str,
                                  c_trd_dt,
                                  c_ordr_rfrnc,
                                  i_dwnld_flg,
                                  c_ServiceName,
                                  c_err_msg);

          if (i_ch_val == -1)
          {
            fn_userlog(c_ServiceName,"Failed While Calling Function fn_ord_acpt_rjct ");
            return -1;
          }
        }
      }
      else
      {
         i_ch_val = fn_ord_acpt_rjct(ptr_oe_reqres,
                                  st_i_xchngbk, 
                                  c_skpord_ref,
                                  c_xchng_cd,
                                  c_pipe_id,
                                  c_rout_str,
                                  c_trd_dt,
                                  c_ordr_rfrnc,
                                  i_dwnld_flg,
                                  c_ServiceName,
                                  c_err_msg);

        if (i_ch_val == -1)
        {
          fn_userlog(c_ServiceName,"Failed While Calling Function fn_ord_acpt_rjct ");
          return -1;
        }

      }

      break;
     /*** Ver 7.2 Ends Here **/
	
		case ORDER_CANCEL_CONFIRM_OUT :

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside ORDER_CANCEL_CONFIRM_OUT Case");
				fn_userlog(c_ServiceName,"st_i_xchngbk.c_req_typ = :%c:", st_i_xchngbk.c_req_typ);		/*** Ver 2.3 ***/
				fn_userlog(c_ServiceName,"The ERROR CODE ORDER_CANCEL_CONFIRM_OUT IS :%d:",ptr_oe_reqres->st_hdr.si_error_code); /* Ver 3.3 */
			} 
      /*** version 5.6 starts ***/

      /** commented in ver 6.8 **
      if ( i_dwnld_flg == DOWNLOAD  && ( (ptr_oe_reqres->st_hdr.si_error_code != 16388) && (ptr_oe_reqres->st_hdr.si_error_code != 17070) && (ptr_oe_reqres->st_hdr.si_error_code != 17071) &&  (ptr_oe_reqres->i_ordr_sqnc != 0)  ) ) ** checks for error code added in ver 5.9 ** 
      {
           i_ch_val = fn_seq_to_omd (&st_i_xchngbk,c_ServiceName,c_err_msg);
           if ( i_ch_val != 0 )
           {
                fn_errlog(c_ServiceName,"L91910", LIBMSG, c_err_msg);
                return -1;
           }
      }
      *** version 5.6 ends ***

			*** ver 2.3 start here ***
			if ( (i_dwnld_flg == DOWNLOAD) && (st_i_xchngbk.c_req_typ != 'C') && 
			(ptr_oe_reqres->st_hdr.si_error_code != 17070 && ptr_oe_reqres->st_hdr.si_error_code != 17071 && ptr_oe_reqres->st_hdr.si_error_code != 16388 &&  ptr_oe_reqres->i_ordr_sqnc != 0 ) ) 		
			*** Ver 5.4 seq number zero handled *** *** Ver 5.1 16388 error code  handled ***	*** Ver 3.3 17070 error code  handled ****** Ver 3.9 17071 error code  handled ***	
			{
				fn_userlog(c_ServiceName,"solicited cancellation response received in download");
				fn_userlog(c_ServiceName,"Order Reference = :%s:",st_i_xchngbk.c_ordr_rfrnc); 
				break;
			}
			*** ver 2.3 Ends here ***
     comment ends for ver 6.8 ***/

     /** ver 6.8 starts  **/
      fn_userlog(c_ServiceName,"CR_139156 before seq to omd i_ordr_sqnc :%d:",ptr_oe_reqres->i_ordr_sqnc);

           /*** loop executes in case of solicited cancellation and unsolicited with reason code 0 ***/

       if( ptr_oe_reqres->i_ordr_sqnc != 0 && ( ptr_oe_reqres->st_hdr.si_error_code != 17070 && ptr_oe_reqres->st_hdr.si_error_code != 17071 && ptr_oe_reqres->st_hdr.si_error_code != 16388 && ptr_oe_reqres->st_ord_flg.flg_ioc != 1))
       {
        memset(c_ord_ack_nse,'\0',sizeof(c_ord_ack_nse));
        sprintf(c_ord_ack_nse,"%16.0lf",ptr_oe_reqres->d_order_number);
        fn_userlog(c_ServiceName,"CR_139156 c_ord_ack_nse:%s:",c_ord_ack_nse);

           EXEC SQL
                SELECT FOD_SPL_FLAG
                  INTO :c_fod_spec_flg
                  FROM FOD_FO_ORDR_DTLS
                 WHERE FOD_ACK_NMBR  = :c_ord_ack_nse
                   AND ROWNUM < 2;
           if ( SQLCODE != 0 )
           {
                fn_errlog(c_ServiceName,"L91910", LIBMSG, c_err_msg);
                fn_userlog(c_ServiceName,"CR_139156 ERROR IN FOD FETCH");
                  return -1;
           }
           fn_userlog(c_ServiceName,"CR_139156 c_fod_spec_flg :%c:",c_fod_spec_flg);
           if( c_fod_spec_flg == 'R' || c_fod_spec_flg == 'O' || c_fod_spec_flg == '2' || c_fod_spec_flg == '3'|| c_fod_spec_flg == 'J' )
           {

            st_i_xchngbk.c_req_typ = '\0';

            EXEC SQL
                 SELECT  Fxb_rqst_typ
                   INTO  :st_i_xchngbk.c_req_typ
                   FROM  FXB_FO_XCHNG_BOOK
                  WHERE  FXB_XCHNG_CD  = :st_i_xchngbk.c_xchng_cd
                    AND  FXB_PIPE_ID   = :st_i_xchngbk.c_pipe_id
                    AND  FXB_ORDR_RFRNC IN ( SELECT FOD_ORDR_RFRNC FROM FOD_FO_ORDR_DTLS WHERE FOD_ACK_NMBR  =:c_ord_ack_nse )
                    AND  FXB_MOD_TRD_DT = to_date(:st_i_xchngbk.c_mod_trd_dt,'dd-Mon-yyyy')
                    AND  FXB_RMS_PRCSD_FLG = 'N'
                    AND  ROWNUM < 2;
             if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
             {
                  fn_errlog(c_ServiceName,"L91910", LIBMSG, c_err_msg);
                  fn_userlog(c_ServiceName,"CR_139156 ERROR IN FXB FETCH");
                  return -1;
             }
             else if ( SQLCODE == NO_DATA_FOUND )
             {
                 fn_userlog(c_ServiceName,"NO UNPROCESSED RECORD");
             }
           }
           else
           {
            i_ch_val = fn_seq_to_omd (&st_i_xchngbk,c_ServiceName,c_err_msg);
            if ( i_ch_val != 0 )
            {
                fn_errlog(c_ServiceName,"L91910", LIBMSG, c_err_msg);
                return -1;
            }
          }
        }
     fn_userlog(c_ServiceName,"CR_139156 aftr seqtomd st_i_xchngbk.c_req_typ :%c: c_ordr_rfrnc :%s:",st_i_xchngbk.c_req_typ,st_i_xchngbk.c_ordr_rfrnc);

    /*** Request time is C for solicited cancellations and blank for unsolicited cancellations**/

    /*** ver 6.8 ends  ***/

			/*** Ver 3.3 Error code 17070 send for trade price excceding trade excution range specified by exchange ***/
			if((ptr_oe_reqres->st_ord_flg.flg_ioc == 1) || (ptr_oe_reqres->st_hdr.si_error_code == 17070 ) || (ptr_oe_reqres->st_hdr.si_error_code == 17071) 
				|| ( ptr_oe_reqres->i_ordr_sqnc == 0) || (ptr_oe_reqres->st_hdr.si_error_code == 16388 )||(st_i_xchngbk.c_req_typ != 'C')) /*** Ver 5.4 16388 and sqnc added **/	/*** Ver 3.3 17070 error code check added ***/  /*** Ver 3.9 17071 error code check added ***//*** req typ addedin ver 6.8 **/
			{
				if(DEBUG_MSG_LVL_0)
				{
					/**fn_userlog(c_ServiceName,"Inside IOC Flag Check Condition");			*** Ver 4.9 **/
					fn_userlog(c_ServiceName,"The IOC flag    Is :%d:",ptr_oe_reqres->st_ord_flg.flg_ioc);    /** Ver 3.3 **/
          fn_userlog(c_ServiceName,"The error code  Is :%d:",ptr_oe_reqres->st_hdr.si_error_code); /** Ver 3.3 **/
				}
			/*** Commented In Ver 1.5
				fn_userlog(c_ServiceName,"Before Going To Sleep For 3 Seconds In IOC Cancellation");
				sleep(3);			Added to differ the execution till order confirmation is processed 			***/

				i_ch_val = fn_ord_can ( ptr_oe_reqres,
																c_xchng_cd,
																c_pipe_id,
																c_rout_str,
																c_trd_dt,
																c_ordr_rfrnc,
																i_dwnld_flg,
																c_ServiceName,
																c_err_msg);

				if (i_ch_val == -1)
      	{
        	fn_userlog(c_ServiceName,"Failed While Calling Function fn_ord_can ");
        	return -1;
     	 	}
			}
			else 
			{
				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Inside Normal Cancellation Handling");
				}
				i_ch_val = fn_ord_acpt_rjct(ptr_oe_reqres,
																		st_i_xchngbk,  /** Ver 3.2 **/
																		c_skpord_ref,             /** Ver 3.4 **/
																		c_xchng_cd,
																		c_pipe_id,
																		c_rout_str,
																		c_trd_dt,
                                    c_ordr_rfrnc,
																	  i_dwnld_flg,
                                    c_ServiceName,
                                    c_err_msg);

      	if (i_ch_val == -1)
     		{
        	fn_userlog(c_ServiceName,"Failed While Calling Function fn_ord_acpt_rjct ");
       	  return -1;
     		}
			}

			break;
		case FREEZE_TO_CONTROL :

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside FREEZE_TO_CONTROL Case");
			}
			
			/***	Commented In Ver 1.5
			fn_userlog(c_ServiceName,"Before Going To Sleep For 5 Seconds In FREEZE_TO_CONTROL Case");
			sleep(5);		This sleep is added bcoz sometimes we receive freeze response immediately after the sltp 
										 trigger response, which may lead to transaction timed out , the purpose of sleeping for 5 seconds 
										is to avoid conflict between sltp trigger(3 sec) thus getting extra 2 seconds	***/	
			i_ch_val = fn_ord_frz_exp ( ptr_oe_reqres,
																	c_xchng_cd,
																	c_pipe_id,
																	c_rout_str,
																	c_trd_dt,
																  c_ordr_rfrnc,
																	i_dwnld_flg,
																	c_ServiceName,
																	c_err_msg);

			if (i_ch_val == -1)
      {
      	fn_userlog(c_ServiceName,"Failed While Calling Function fn_ord_acpt_rjct ");
        return -1;
      }

      /*** version 5.8 started ***/

      EXEC SQL
      SELECT FOD_PRDCT_TYP 
      INTO   :c_prd_typ
      FROM   FOD_FO_ORDR_DTLS
      WHERE  FOD_ORDR_RFRNC = :c_ordr_rfrnc;

      if( SQLCODE != 0 )
      {
         fn_userlog(c_ServiceName,"Failed While Fetching FOD_PRDCT_TYP :%d:",c_prd_typ);
         return -1;
      }
      
      if( DEBUG_MSG_LVL_3 )
      { 
      	fn_userlog(c_ServiceName,"The c_prd_typ is :%c: for c_ordr_rfrnc :%s: ",c_prd_typ,c_ordr_rfrnc);
 			} 
      i_ch_val =  fn_fxb_rms_erl ( c_ServiceName,
                               c_err_msg,
                               c_prd_typ,
                               c_ordr_rfrnc );
      if( i_ch_val != 0 )
      {
         fn_userlog(c_ServiceName," Failed for fn_fxb_rms_erl " );
         return -1;
      }
      /*** version 5.8 ended ***/
			break;

		case BATCH_ORDER_CXL_OUT :

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside BATCH_ORDER_CXL_OUT Case");
			}

			if(ptr_oe_reqres->st_ord_flg.flg_frozen == 1 )
			{
				if(DEBUG_MSG_LVL_0)
				{
		  		fn_userlog(c_ServiceName, "Freezed order got cancelled");
				}
			}	/*** Ver 2.8 ***/
			
			i_ch_val = fn_ord_can ( ptr_oe_reqres,
															c_xchng_cd,
															c_pipe_id,
															c_rout_str,
															c_trd_dt,
															c_ordr_rfrnc,
															i_dwnld_flg,
															c_ServiceName,
															c_err_msg);

			if (i_ch_val == -1)
			{
				fn_userlog(c_ServiceName,"Failed While Calling Function fn_ord_can ");
				return -1;
			}
			/*** }	Commented in Ver 2.8 ***/

			break;
	
		default :
			fn_userlog( c_ServiceName, "Protocol error %d", ptr_oe_reqres->st_hdr.si_transaction_code);
			break;

	}
}

int fn_ord_acpt_rjct( struct st_oe_reqres *ptr_oe_reqres,
											struct vw_xchngbook st_xchngbk, /** Ver 3.2 **/
											char *c_skpord_ref,							/** Ver 3.4 **/
											char *c_xchng_cd,
											char *c_pipe_id,
											char *c_rout_str,
											char *c_trd_dt,
											char *c_ordr_rfrnc,
											int  i_dwnld_flg,
											char *c_ServiceName,
											char *c_err_msg)
{
	struct vw_xchngbook st_i_xchngbk;
	struct vw_orderbook st_i_ordrbk;

	char c_tmp[8];
	char c_svc_name[15+1];
	char c_xchng_rmrks[256];
	char c_mtch_accnt[11];
	char c_xchng_err_msg[256];
	char c_spn_allwd_flg;

	int i_ch_val;
	int i_trnsctn;
  int i_rec_exists = 0; /** Ver 3.4 **/ 
	
	long int li_ors_msg_typ;
	long long ll_log_tm_stmp;
	char c_lst_act_ref [22];    /*** Parag ***/		/*** Ver 6.1 ***/

	if(DEBUG_MSG_LVL_3)   /** Ver 4.9, debug level changed from 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside fn_ord_acpt_rjct Function");
	}

	MEMSET(st_i_xchngbk);
	MEMSET(st_i_ordrbk);
	MEMSET(c_xchng_rmrks);
	MEMSET(c_xchng_err_msg);
	MEMSET(c_lst_act_ref);		 /*** Parag ***/   /*** Ver 6.1 ***/

	memcpy(&st_i_xchngbk,&st_xchngbk,sizeof(st_xchngbk)); /** ver 3.2 **/

	/***	Commented in Ver 3.4 

	strcpy ( st_i_ordrbk.c_ordr_rfrnc ,c_ordr_rfrnc); 


	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"Order Reference Is :%s:",st_i_ordrbk.c_ordr_rfrnc);
	}

	***/

	switch (ptr_oe_reqres->st_hdr.si_transaction_code)
	{
		case ORDER_CONFIRMATION_OUT :

			if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug level changed from 0 to 3 **/
			{
				fn_userlog(c_ServiceName,"ORDER_CONFIRMATION_OUT Case");
			}

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_reqres->li_last_modified);
				fn_userlog(c_ServiceName,"Exchange Entry Date Time Is :%ld:",ptr_oe_reqres->li_entry_date_time);
			}

			st_i_xchngbk.l_ors_msg_typ = ORS_NEW_ORD_ACPT;
		  fn_long_to_timearr ( st_i_xchngbk.c_ack_tm,ptr_oe_reqres->li_last_modified );
     	fn_long_to_timearr ( st_i_xchngbk.c_entry_dt_tm,ptr_oe_reqres->li_entry_date_time );
		
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Message Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
		  	fn_userlog(c_ServiceName,"Exchange Book Entry Date Time :%s:",st_i_xchngbk.c_entry_dt_tm);
			}

			break;

		case ORDER_MOD_CONFIRM_OUT :
		
			if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug level changed from 0 to 3 **/
			{
				fn_userlog(c_ServiceName,"ORDER_MOD_CONFIRM_OUT Case");
			}

			st_i_xchngbk.l_ors_msg_typ = ORS_MOD_ORD_ACPT ;

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_reqres->li_last_modified);
			}

			fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_reqres->li_last_modified);

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Message Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);		
			}

			break;
	
		case ORDER_CANCEL_CONFIRM_OUT :

			if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug level changed from 0 to 3 **/
			{
				fn_userlog(c_ServiceName,"ORDER_CANCEL_CONFIRM_OUT Case");
			}

			st_i_xchngbk.l_ors_msg_typ = ORS_CAN_ORD_ACPT ;

      /*** ver 6.0 starts ***/
      st_i_xchngbk.l_xchng_can_qty = ptr_oe_reqres->li_volume_filled_today; 

      if( DEBUG_MSG_LVL_0 )
      {
        fn_userlog( c_ServiceName,"In cancellation acceptance, st_i_xchngbk.l_xchng_can_qty is :%ld:",st_i_xchngbk.l_xchng_can_qty );
      }
      /*** ver 6.0 ends ***/

			fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_reqres->li_last_modified);
	
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_reqres->li_last_modified);
				fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
			}

			break;

		case ORDER_ERROR_OUT :
	
			if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug level changed from 0 to 3 **/
			{
				fn_userlog(c_ServiceName,"ORDER_ERROR_OUT Case");
			}

			st_i_xchngbk.l_ors_msg_typ = ORS_NEW_ORD_RJCT;


			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Message Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_reqres->li_entry_date_time);
				fn_userlog(c_ServiceName,"Exchange Entry date Time Is :%ld:",ptr_oe_reqres->li_entry_date_time);
			}

			fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_reqres->li_entry_date_time);
			fn_long_to_timearr ( st_i_xchngbk.c_entry_dt_tm,ptr_oe_reqres->li_entry_date_time );
		

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
				fn_userlog(c_ServiceName,"Exchange Book Entry date Time Is :%s:", st_i_xchngbk.c_entry_dt_tm);
				fn_userlog(c_ServiceName,"The ERROR CODE FOR ORDER RJCT IS :%d:",ptr_oe_reqres->st_hdr.si_error_code);
			}

			i_ch_val = fn_get_errmsg( ptr_oe_reqres->st_hdr.si_error_code,ptr_oe_reqres->si_reason_code,c_xchng_err_msg,c_ServiceName);
			
			if(i_ch_val == -1 )
			{
				fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
				return -1;
			}
	
			strcpy(c_xchng_rmrks,c_xchng_err_msg);

			break;

		case ORDER_MOD_REJ_OUT :

			if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug level changed from 0 to 3 **/
			{
				fn_userlog(c_ServiceName,"Inside ORDER_MOD_REJ_OUT Case");
			}

			st_i_xchngbk.l_ors_msg_typ = ORS_MOD_ORD_RJCT ;
		
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_reqres->li_last_modified);
			}

			fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_reqres->li_last_modified); 

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
				fn_userlog(c_ServiceName,"The ERROR CODE FOR MOD RJCT IS :%d:",ptr_oe_reqres->st_hdr.si_error_code);
			}

			i_ch_val = fn_get_errmsg( ptr_oe_reqres->st_hdr.si_error_code,ptr_oe_reqres->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

			if(i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
        return -1;
      }

      strcpy(c_xchng_rmrks,c_xchng_err_msg);

			break;

		case ORDER_CXL_REJ_OUT :

			if(DEBUG_MSG_LVL_3)   /** Ver 4.9, debug level changed from 0 to 3 **/
			{
				fn_userlog(c_ServiceName,"Inside ORDER_CXL_REJ_OUT Case");
			}

			st_i_xchngbk.l_ors_msg_typ = ORS_CAN_ORD_RJCT;
			
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_reqres->li_last_modified);
			}

			fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_reqres->li_last_modified);

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm); 
				fn_userlog(c_ServiceName,"The ERROR CODE FOR MOD RJCT IS :%d:",ptr_oe_reqres->st_hdr.si_error_code);
			}
		
			i_ch_val = fn_get_errmsg( ptr_oe_reqres->st_hdr.si_error_code,ptr_oe_reqres->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

      if(i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
        return -1;
      }

      strcpy(c_xchng_rmrks,c_xchng_err_msg);

			break;

		default :

			fn_userlog(c_ServiceName, "Protocol error %d", ptr_oe_reqres->st_hdr.si_transaction_code);
			break;
	}

	/*fn_cpy_ddr(st_i_ordrbk.c_rout_crt); */		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_ordrbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "Before Call To SFO_REF_TO_ORD");
	}
	
  st_i_ordrbk.c_oprn_typ = FOR_VIEW;

	/***	Commented In Ver 1.3
  i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_i_ordrbk,
                           &st_i_ordrbk,
                           "vw_orderbook",
                           "vw_orderbook",
                           sizeof ( st_i_ordrbk ),
                           sizeof ( st_i_ordrbk ),
                           0,
                           "SFO_REF_TO_ORD" ); 	

	************************************************************/

	/*** Commented in ver 3.2 **

	i_ch_val = fn_ref_to_ord (&st_i_ordrbk,c_ServiceName,c_err_msg);

	if ( i_ch_val != 0)
  {
  	fn_errlog(c_ServiceName,"L31015", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
 		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_REF_TO_ORD :%s:",c_time);
		fn_userlog(c_ServiceName,"Order Reference Is :%s:",st_i_ordrbk.c_ordr_rfrnc); 
		fn_userlog(c_ServiceName,"Modification Counter Is :%ld:", st_i_ordrbk.l_mdfctn_cntr);
		fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_i_ordrbk.c_xchng_cd);
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",st_i_ordrbk.c_pipe_id);
		fn_userlog(c_ServiceName,"Match Account Is :%s:",st_i_ordrbk.c_cln_mtch_accnt);
	}

  strcpy ( st_i_xchngbk.c_ordr_rfrnc, st_i_ordrbk.c_ordr_rfrnc );
  st_i_xchngbk.l_mdfctn_cntr = st_i_ordrbk.l_mdfctn_cntr;
  strcpy ( st_i_xchngbk.c_xchng_cd, st_i_ordrbk.c_xchng_cd );
  strcpy ( st_i_xchngbk.c_pipe_id, st_i_ordrbk.c_pipe_id );
  strcpy(c_mtch_accnt, st_i_ordrbk.c_cln_mtch_accnt);


  *** if ( st_i_ordrbk.c_prd_typ == FUTURES || st_i_ordrbk.c_prd_typ == FUTURE_PLUS ) Commented in Ver 2.5 ***
	if ( st_i_ordrbk.c_prd_typ == FUTURES || st_i_ordrbk.c_prd_typ == FUTURE_PLUS || st_i_ordrbk.c_prd_typ == SLTP_FUTUREPLUS )		*** Ver 2.5 ***
 	{
 		strcpy( c_svc_name , "SFO_FUT_ACK" );
 	}
 	else
 	{
 		strcpy( c_svc_name , "SFO_OPT_ACK" );
 	}

	*****/

	/** Ver 3.2 Starts **/
  if( DEBUG_MSG_LVL_3 )  /** Ver 4.9 **/
  {
	  fn_userlog(c_ServiceName,"Exchange Product Type is :%c:",ptr_oe_reqres->st_con_desc.c_instrument_name[0]);
  }

  if (ptr_oe_reqres->st_con_desc.c_instrument_name[0] == 'F')
  {
    strcpy( c_svc_name , "SFO_FUT_ACK" );
  }
  else
  {
    strcpy( c_svc_name , "SFO_OPT_ACK" );
  }

  /** Ver 3.2 Ends **/

	switch (st_i_xchngbk.l_ors_msg_typ)
	{
		case  ORS_NEW_ORD_ACPT :
 	  case  ORS_MOD_ORD_ACPT :
	  case  ORS_CAN_ORD_ACPT :
		
			st_i_xchngbk.c_plcd_stts = ACCEPT;
      break;

		case  ORS_NEW_ORD_RJCT :
	  case  ORS_MOD_ORD_RJCT :
	  case  ORS_CAN_ORD_RJCT :

			st_i_xchngbk.c_plcd_stts = REJECT;
      break;
	}
 	
	st_i_xchngbk.c_rms_prcsd_flg = NOT_PROCESSED;
	strcpy ( st_i_xchngbk.c_xchng_rmrks ,c_xchng_rmrks );  
	memcpy ( &ll_log_tm_stmp,ptr_oe_reqres->st_hdr.c_time_stamp_1, 8 );

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",st_i_xchngbk.c_xchng_rmrks);
		fn_userlog(c_ServiceName,"Log time Stamp Is :%lld:",ll_log_tm_stmp);
	}

	st_i_xchngbk.d_jiffy = (double) ll_log_tm_stmp;

	fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_xchngbk.d_jiffy);

	st_i_xchngbk.l_dwnld_flg = i_dwnld_flg;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Down Load Flag Is :%ld:",st_i_xchngbk.l_dwnld_flg);
	}

	/*fn_cpy_ddr(st_i_xchngbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
	
	st_i_xchngbk.c_oprn_typ = UPDATION_ON_EXCHANGE_RESPONSE;

	st_i_xchngbk.l_bit_flg = ptr_oe_reqres->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1];

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"IN NORM_ACPT_RJCT, the stream number receievd from exchange is:%ld:",st_i_xchngbk.l_bit_flg);	/*Ver 1.7*/
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Before Call To SFO_UPD_XCHNGBK");
	}

	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"L31020", LIBMSG, c_err_msg);
    return -1;
  }

	/*** Commented In Ver 1.3
	i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_i_xchngbk,
                           &st_i_xchngbk,
                           "vw_xchngbook",
                           "vw_xchngbook",
                           sizeof ( st_i_xchngbk ),
                           sizeof ( st_i_xchngbk ),
                           0,
                           "SFO_UPD_XCHNGBK" );

	*******************************************************/

  /***  Commented in Ver 3.4 ** 

	i_ch_val = fn_upd_xchngbk_erl(&st_i_xchngbk,c_ServiceName,c_err_msg);

  if( ( i_ch_val != 0) && ( i_ch_val != RECORD_ALREADY_PRCSD ) )
  {
    fn_errlog(c_ServiceName,"L31025", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }

	else if ( i_ch_val == RECORD_ALREADY_PRCSD )
	{
		fn_userlog ( c_ServiceName, "Record already processed" );
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return 0;
	}

  **************************************/

  /**  Ver 3.4 Starts **/

  if ( st_i_xchngbk.l_dwnld_flg == DOWNLOAD )
  {
    EXEC SQL
      SELECT 1
      INTO :i_rec_exists
      FROM FXB_FO_XCHNG_BOOK
      WHERE FXB_JIFFY   = :st_i_xchngbk.d_jiffy
      AND FXB_XCHNG_CD  = :st_i_xchngbk.c_xchng_cd
      AND FXB_PIPE_ID   = :st_i_xchngbk.c_pipe_id
      AND FXB_ORDR_SQNC = :st_i_xchngbk.l_ord_seq; /*** Ver 6.7 ****/
      /**AND FXB_STREAM_NO = :st_i_xchngbk.l_bit_flg;***COmmented in Ver 6.6 ****/

    if ( ( SQLCODE != 0 ) && (SQLCODE != NO_DATA_FOUND) )
    {
      fn_errlog(c_ServiceName,"L31030", LIBMSG, c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return -1;
    }
    if ( i_rec_exists == 1 )
    {
      fn_userlog ( c_ServiceName, "Record already processed for order confirmation" );
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return 0;
    }
  }

  if(DEBUG_MSG_LVL_3)
  {
	fn_userlog(c_ServiceName,"Last Activity Ref Before is :%lld:",ptr_oe_reqres->ll_lastactivityref);	/*** Ver 6.1 ***/
  }

	sprintf(c_lst_act_ref,"%lld",ptr_oe_reqres->ll_lastactivityref);  /*** Parag ***//*** Ver 6.1 ***/
  rtrim(c_lst_act_ref);   /*** Parag ***/		/*** Ver 6.1 ***/

	if(DEBUG_MSG_LVL_3)
  {
  	fn_userlog(c_ServiceName,"Exchange code :%s:,order sequence :%ld:,pipe id :%s:",st_i_xchngbk.c_xchng_cd,st_i_xchngbk.l_ord_seq,st_i_xchngbk.c_pipe_id);
		fn_userlog(c_ServiceName,"Last Activity Ref is :%s:",c_lst_act_ref);			/*** Ver 6.1 ***/
	}

  MEMSET(sql_fxb_ordr_rfrnc);
  sql_fxb_mdfctn_cntr = 0;

  /*** ver 6.0 starts ***/
  if( ptr_oe_reqres->st_hdr.si_transaction_code == ORDER_CANCEL_CONFIRM_OUT )
  {
  EXEC SQL
    UPDATE  FXB_FO_XCHNG_BOOK
    SET FXB_PLCD_STTS       = :st_i_xchngbk.c_plcd_stts,
        FXB_RMS_PRCSD_FLG   = :st_i_xchngbk.c_rms_prcsd_flg,
        FXB_ORS_MSG_TYP     = :st_i_xchngbk.l_ors_msg_typ,
        FXB_ACK_TM          = to_date( :st_i_xchngbk.c_ack_tm,'DD-Mon-yyyy hh24:mi:ss' ),
        FXB_XCHNG_RMRKS     = rtrim(fxb_xchng_rmrks)||:st_i_xchngbk.c_xchng_rmrks,
        FXB_JIFFY           = :st_i_xchngbk.d_jiffy,
        FXB_STREAM_NO       = :st_i_xchngbk.l_bit_flg,
        FXB_XCHNG_CNCLD_QTY = :st_i_xchngbk.l_xchng_can_qty,
        FXB_LST_ACT_REF     = :c_lst_act_ref  /*** Ver 6.1 ***/
    WHERE FXB_XCHNG_CD      = :st_i_xchngbk.c_xchng_cd
    AND   FXB_ORDR_SQNC     = :st_i_xchngbk.l_ord_seq
    AND   FXB_PIPE_ID       = :st_i_xchngbk.c_pipe_id
    AND   FXB_MOD_TRD_DT    = :c_trd_dt
    RETURNING FXB_ORDR_RFRNC,
              FXB_MDFCTN_CNTR
    INTO      :sql_fxb_ordr_rfrnc,
              :sql_fxb_mdfctn_cntr;

  }
  else /*** ver 6.0 ends ***/
  {
  EXEC SQL
    UPDATE  FXB_FO_XCHNG_BOOK
    SET FXB_PLCD_STTS       = :st_i_xchngbk.c_plcd_stts,
        FXB_RMS_PRCSD_FLG   = :st_i_xchngbk.c_rms_prcsd_flg,
        FXB_ORS_MSG_TYP     = :st_i_xchngbk.l_ors_msg_typ,
        FXB_ACK_TM          = to_date( :st_i_xchngbk.c_ack_tm,'DD-Mon-yyyy hh24:mi:ss' ),
        FXB_XCHNG_RMRKS     = rtrim(fxb_xchng_rmrks)||:st_i_xchngbk.c_xchng_rmrks,
        FXB_JIFFY           = :st_i_xchngbk.d_jiffy,
        FXB_STREAM_NO       = :st_i_xchngbk.l_bit_flg,
				FXB_LST_ACT_REF     = :c_lst_act_ref    /*** Parag ***/	/*** Ver 6.1 ***/
    WHERE FXB_XCHNG_CD      = :st_i_xchngbk.c_xchng_cd
    AND   FXB_ORDR_SQNC     = :st_i_xchngbk.l_ord_seq
    AND   FXB_PIPE_ID       = :st_i_xchngbk.c_pipe_id
    AND   FXB_MOD_TRD_DT    = :c_trd_dt
    RETURNING FXB_ORDR_RFRNC,
              FXB_MDFCTN_CNTR
    INTO      :sql_fxb_ordr_rfrnc,
              :sql_fxb_mdfctn_cntr;
   }

	 if (SQLCODE != 0 ) 
   {
     fn_errlog(c_ServiceName,"L31035", SQLMSG, c_err_msg);
     fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
     return -1;
   }


	
  if ( i_dwnld_flg == DOWNLOAD && (strcmp(sql_fxb_ordr_rfrnc,c_skpord_ref) == 0) )
  {
		fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
		return 0; 
  }

  strcpy(st_i_xchngbk.c_ordr_rfrnc,sql_fxb_ordr_rfrnc);
  st_i_xchngbk.l_mdfctn_cntr=sql_fxb_mdfctn_cntr;

	if(DEBUG_MSG_LVL_0)
  {
  	/**fn_userlog(c_ServiceName,"Order reference:%s:,Modification cnt :%ld:",sql_fxb_ordr_rfrnc,sql_fxb_mdfctn_cntr); ** Ver 5.5 ***/
  	fn_userlog(c_ServiceName,"Order reference:%s:,Modification cnt :%d:",sql_fxb_ordr_rfrnc,sql_fxb_mdfctn_cntr); /** Ver 5.5 modifier changed from %ld to %d ***/
  	/** fn_userlog(c_ServiceName,"st_i_xchngbk.c_ordr_rfrnc :%s:,st_i_xchngbk.l_mdfctn_cntr :%ld:",st_i_xchngbk.c_ordr_rfrnc,st_i_xchngbk.l_mdfctn_cntr); *** Ver 4.9 **/
	}

	/** Ver 3.4 Ends **/ 
	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_XCHNGBK :%s:",c_time);
	}

	strcpy(st_i_ordrbk.c_ordr_rfrnc,st_i_xchngbk.c_ordr_rfrnc);  /** Ver 3.2 **/
	sprintf( st_i_ordrbk.c_xchng_ack,"%16.0lf", ptr_oe_reqres->d_order_number ); 

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Ack Is :%lf:",ptr_oe_reqres->d_order_number);
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		fn_userlog(c_ServiceName,"Order Book Exchange Ack Is :%s:",st_i_ordrbk.c_xchng_ack);
	}

	/*fn_cpy_ddr(st_i_ordrbk.c_rout_crt); */		/*** Ver 1.1 ***/

	strcpy(st_i_ordrbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Before Call To SFO_UPD_ORDRBK");
	}

  st_i_ordrbk.c_oprn_typ = UPDATE_XCHNG_RESPONSE;
		
	/*** Commented In Ver 1.3
  i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_i_ordrbk,
                           &st_i_ordrbk,
                           "vw_orderbook",
                           "vw_orderbook",
                           sizeof ( st_i_ordrbk ),
                           sizeof ( st_i_ordrbk ),
                           0,
                           "SFO_UPD_ORDRBK" );

	*********************************************************/

	/*** Update taken inside if condition in Ver 4.8 ***/

	if(DEBUG_MSG_LVL_3)   /** Ver 4.9 debug changed from 0  to 3 **/
  {
		fn_userlog(c_ServiceName,"st_i_xchngbk.l_ors_msg_typ is  :%ld:",st_i_xchngbk.l_ors_msg_typ);
	}

  if (st_i_xchngbk.l_ors_msg_typ == ORS_NEW_ORD_ACPT || st_i_xchngbk.l_ors_msg_typ == ORS_NEW_ORD_RJCT )
  {
		i_ch_val = fn_upd_ordrbk (&st_i_ordrbk,c_ServiceName,c_err_msg);

  	if ( i_ch_val != 0)
  	{
   	 	fn_errlog(c_ServiceName,"L31040", LIBMSG, c_err_msg);
    	fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    	return -1;
  	}

		fn_gettime();

		if(DEBUG_MSG_LVL_3)  /** Ver 4.9 debug changed from 0  to 3 **/
		{
			fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_ORDRBK :%s:",c_time);
      fn_userlog(c_ServiceName,"After callng to function fn_upd_ordrbk.");
		}
	}
	
  /*** Ver 4.8 Ends ***/	

	i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
	
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"L31045", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Commit :%s:",c_time);
	}

  /*fn_cpy_ddr(st_i_xchngbk.c_rout_crt); */		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);  /*** Ver 1.1 ***/

	i_ch_val = fn_acall_svc ( c_ServiceName,
                             c_err_msg,
                             &st_i_xchngbk,
                             "vw_xchngbook",
                             sizeof ( st_i_xchngbk ),
                             TPNOREPLY,
                             c_svc_name );
  if ( i_ch_val != 0 )
  {
    fn_errlog(c_ServiceName,"L31050", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"ORS2-RD After Call To FUT/OPT ACK :%s:",c_time);
		fn_userlog(c_ServiceName,"After Complition Of fn_ord_acpt_rjct");
	}

	return 0;
}

int fn_ord_can ( struct st_oe_reqres *ptr_oe_reqres,
								 char *c_xchng_cd,
								 char *c_pipe_id,
								 char *c_rout_str,
								 char *c_trd_dt,
                 char *c_ordr_rfrnc,
							   int  i_dwnld_flg,
                 char *c_ServiceName,
                 char *c_err_msg)
{
	
	char c_xchng_rmrks[256];
	char c_svc_name[15+1];
	char c_mtch_accnt[10 + 1];
	int i_trnsctn;
  int i_ch_val;
	char c_lst_act_ref[22];   /*** Ver 6.1 ***/

	long l_token_id;

	long long ll_log_tm_stmp;

	struct vw_xchngbook st_i_xchngbk;
  struct vw_orderbook st_i_ordbk;
  struct vw_contract st_cntrct;

	MEMSET(st_i_xchngbk);
	MEMSET(st_i_ordbk);
	MEMSET(st_cntrct);
	MEMSET(c_xchng_rmrks);
	MEMSET(c_lst_act_ref);   /*** Ver 6.1 ***/

	if(DEBUG_MSG_LVL_3)  /**** Ver 4.9, debug level changed from 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside Function fn_ord_can");
	}

	switch (ptr_oe_reqres->st_hdr.si_transaction_code)
	{
		case ORDER_CANCEL_CONFIRM_OUT : 
	  case ORDER_ERROR_OUT :  /** Ver 7.2 **/
	
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside ORDER_CANCEL_CONFIRM_OUT Case");
			}
			st_i_xchngbk.l_ors_msg_typ = ORS_IOC_CAN_ACPT;

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Message Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_reqres->li_last_modified);
				fn_userlog(c_ServiceName,"Exchange Volume Filled Today Is :%ld:",ptr_oe_reqres->li_volume_filled_today);
				fn_userlog(c_ServiceName,"Exchange Token NO. Is :%ld:",ptr_oe_reqres->l_token_no);
				fn_userlog(c_ServiceName,"Error code Is :%d:",ptr_oe_reqres->st_hdr.si_error_code);  /*** Ver 3.3 ***/
			}

			fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_reqres->li_last_modified);

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
			}
			
			st_i_xchngbk.l_xchng_can_qty = ptr_oe_reqres->li_volume_filled_today;

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Book Cancelled Qty Is :%ld:",st_i_xchngbk.l_xchng_can_qty);
			}

			/*** strcpy(c_xchng_rmrks , "IOC order cancelled" ); Commented In Ver 3.3 ***/
			
			if ( ptr_oe_reqres->st_hdr.si_error_code == 17070 ) /*** Ver 3.3 ***/
      {
        /**strcpy(c_xchng_rmrks , "The Price is out of the current execution range" ); ** ver 7.2 **/
        strcpy(c_xchng_rmrks , "The Price is out of the current LPP range" ); /* Ver 7.2 **/
      }
			else if ( ptr_oe_reqres->st_hdr.si_error_code == 17071 ) /*** Ver 3.9 ***/
      {
        strcpy(c_xchng_rmrks , "The order could have resulted in self trade" );
      }
      /** added in ver 6.9 ***/
      else if ( ptr_oe_reqres->st_hdr.si_error_code == 16388 && ptr_oe_reqres->st_ord_flg.flg_ioc == 1 )
      {
        strcpy(c_xchng_rmrks , "IOC order cancelled by system");
      }
      /*** ver 6.9 ends ***/
      else if ( ptr_oe_reqres->st_hdr.si_error_code == 16388 ) /*** Ver 5.4 ***/
      {
        strcpy(c_xchng_rmrks , "Order Cancelled by system");
      }
      /** added in ver 6.9 ***/
      else if ( ptr_oe_reqres->st_ord_flg.flg_ioc == 1 )
      {
        strcpy(c_xchng_rmrks , "IOC order has been cancelled by Exchange" );
      }
      /*** ver 6.9 ends ***/
      else
      {
        /** strcpy(c_xchng_rmrks , "IOC order cancelled" ); commented in ver 6.8 ***/
        strcpy(c_xchng_rmrks , "Order has been cancelled by Exchange"); /*** ver 6.8 ***/
      }


			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",c_xchng_rmrks);
			}

			l_token_id 	=  ptr_oe_reqres->l_token_no;

			break;
	
		case BATCH_ORDER_CXL_OUT :

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside BATCH_ORDER_CXL_OUT Case");
			}

			if (ptr_oe_reqres->st_ord_flg.flg_frozen == 1 )
			{
				if(DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "Freezed order got cancelled");				
				}

				st_i_xchngbk.l_ors_msg_typ = ORS_ORD_FRZ_CNCL;
				
				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_reqres->li_last_modified);
					fn_userlog(c_ServiceName,"Exchange Volume Filled Today Is :%ld:", ptr_oe_reqres->li_volume_filled_today);
				}

			  fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_reqres->li_last_modified);

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
				}

				st_i_xchngbk.l_xchng_can_qty = ptr_oe_reqres->li_volume_filled_today;

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Exchange Book Cancelled Qty Is :%ld:",st_i_xchngbk.l_xchng_can_qty);
				}

			  strcpy(c_xchng_rmrks ,"Order Cancelled by Exchange");

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",c_xchng_rmrks);
				}
			}
			/*** Ver 2.8 starts ***/
			else
			{
				if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug changed from 0 to 3 ***/
				{
					fn_userlog(c_ServiceName, "Exchange initiated expiry");				
				}

				st_i_xchngbk.l_ors_msg_typ = ORS_ORD_EXP;
				
				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_reqres->li_last_modified);
				}

			  fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_reqres->li_last_modified);

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
				}

				strcpy(c_xchng_rmrks ,"Exchange Expiry");

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",c_xchng_rmrks);
				}
			}
			/*** Ver 2.8 ends ***/

			break;
		
		default :
		  fn_userlog( c_ServiceName, "Protocol error %d", ptr_oe_reqres->st_hdr.si_transaction_code);
			break;	
	}

	if(DEBUG_MSG_LVL_3)
  {
		fn_userlog(c_ServiceName,"Exchange Ack Is :%lf:",ptr_oe_reqres->d_order_number );
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	sprintf (st_i_ordbk.c_xchng_ack, "%16.0lf", ptr_oe_reqres->d_order_number ); 

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Order Book Exchange Ack Is :%s:",st_i_ordbk.c_xchng_ack);
	}

	strcpy ( st_i_ordbk.c_pipe_id , c_pipe_id );

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",st_i_ordbk.c_pipe_id);
	}

/*fn_cpy_ddr(st_i_ordbk.c_rout_crt);	*/		/*** Ver 1.1 ***/

	strcpy(st_i_ordbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Before Call To SFO_ACK_TO_ORD");
	}

	st_i_ordbk.c_oprn_typ = FOR_VIEW;

	i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_i_ordbk,
                           &st_i_ordbk,
                           "vw_orderbook",
                           "vw_orderbook",
                           sizeof ( st_i_ordbk ),
                           sizeof ( st_i_ordbk ),
                           0,
                           "SFO_ACK_TO_ORD" );

	if ( i_ch_val != SUCC_BFR)
  {
    if ( i_ch_val == TOO_MANY_ROWS_FETCHED )
    {
			fn_userlog(c_ServiceName,"Inside TOO_MANY_ROWS_FETCHED");
      if ( strcmp ( c_xchng_cd, "NFO" ) == 0 )
      {
        i_ch_val = fn_nse_to_eba_tkn ( l_token_id,
                                       &st_cntrct,
																			 c_rout_str,
                                       c_ServiceName,
                                       c_err_msg );
        if ( i_ch_val != 0 )
        {
          return -1;
        }

				strcpy ( st_i_ordbk.c_xchng_cd, st_cntrct.c_xchng_cd );
        st_i_ordbk.c_prd_typ = st_cntrct.c_prd_typ;
        strcpy ( st_i_ordbk.c_expry_dt, st_cntrct.c_expry_dt );
        st_i_ordbk.c_exrc_typ = st_cntrct.c_exrc_typ;
        st_i_ordbk.c_opt_typ = st_cntrct.c_opt_typ;
        st_i_ordbk.l_strike_prc = st_cntrct.l_strike_prc;
        st_i_ordbk.c_ctgry_indstk = st_cntrct.c_ctgry_indstk;
        strcpy ( st_i_ordbk.c_undrlyng, st_cntrct.c_undrlyng );
        st_i_ordbk.l_ca_lvl = st_cntrct.l_ca_lvl;
  			sprintf (st_i_ordbk.c_xchng_ack, "%16.0lf", ptr_oe_reqres->d_order_number );
        strcpy ( st_i_ordbk.c_pipe_id , c_pipe_id );
	
        /*fn_cpy_ddr(st_i_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
				}
				strcpy(st_i_ordbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
        st_i_ordbk.c_oprn_typ = FOR_VIEW;

				i_ch_val = fn_call_svc ( c_ServiceName,
                                 c_err_msg,
                                 &st_i_ordbk,
                                 &st_i_ordbk,
                                 "vw_orderbook",
                                 "vw_orderbook",
                                 sizeof ( st_i_ordbk ),
                                 sizeof ( st_i_ordbk ),
                                 0,
                                 "SFO_AC_TO_ORD" );

        if ( i_ch_val != SUCC_BFR)
        {
          fn_errlog(c_ServiceName,"L31055", LIBMSG, c_err_msg);
          return -1;
        }
				
				fn_gettime();
	
				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_AC_TO_ORD :%s:", c_time);
				}

				strcpy (c_ordr_rfrnc,st_i_ordbk.c_ordr_rfrnc ); 
			}
			else
			{
				fn_errlog(c_ServiceName,"S31005", "Logic error", c_err_msg);
				return -1;
			}
		}
		else
		{
			fn_errlog(c_ServiceName,"L31060", LIBMSG, c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return -1;
    }
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_ACK_TO_ORD :%s:",c_time);
		fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_i_ordbk.c_xchng_cd);
	}

	if(DEBUG_MSG_LVL_0) 
	{
		/** fn_userlog(c_ServiceName,"Order Reference No. Is :%s:",st_i_ordbk.c_ordr_rfrnc); ** Ver 4.9 **/
    fn_userlog(c_ServiceName,"fn_ord_can:Order Reference No. Is :%s:",st_i_ordbk.c_ordr_rfrnc);  /*** Ver 4.9 **/
	}

	strcpy(st_i_xchngbk.c_pipe_id,c_pipe_id);
	strcpy ( st_i_xchngbk.c_ordr_rfrnc , st_i_ordbk.c_ordr_rfrnc );
  strcpy ( st_i_xchngbk.c_xchng_cd , st_i_ordbk.c_xchng_cd );
  strcpy ( st_i_xchngbk.c_mod_trd_dt , ( char * )c_trd_dt);	
  strcpy ( c_mtch_accnt , st_i_ordbk.c_cln_mtch_accnt );            
	strcpy(st_i_xchngbk.c_xchng_rmrks,c_xchng_rmrks);
  strcpy (c_ordr_rfrnc,st_i_ordbk.c_ordr_rfrnc );
  st_i_xchngbk.l_ord_tot_qty = st_i_ordbk.l_ord_tot_qty;
  st_i_xchngbk.l_ord_lmt_rt = st_i_ordbk.l_ord_lmt_rt;
  st_i_xchngbk.l_ord_seq = 0;
  /*** ver 6.3 starts ***/
  if( ptr_oe_reqres->st_hdr.si_transaction_code == BATCH_ORDER_CXL_OUT )
  {
       st_i_xchngbk.l_mdfctn_cntr = st_i_ordbk.l_mdfctn_cntr + 1;
       if(DEBUG_MSG_LVL_0)
       {
           fn_userlog(c_ServiceName,"Order Book Modification counter is :%ld:,Exchange Book Modification counter is :%ld: and Order Ref num is :%s: for BATCH_ORDER_CXL_OUT",st_i_ordbk.l_mdfctn_cntr,st_i_xchngbk.l_mdfctn_cntr,st_i_ordbk.c_ordr_rfrnc);
       }

  }/*** ver 6.3 ends ***/
  else
  {
       st_i_xchngbk.l_mdfctn_cntr = EXPIRY_MDFCTN_CNTR;
  }
	/*** st_i_xchngbk.c_plcd_stts = CANCELLED;		-Commented in Ver 2.8 ***/
	
	/*** Ver 2.8 starts ***/
	if (st_i_xchngbk.l_ors_msg_typ == ORS_ORD_EXP) /*** Ver 3.1 ** "=" Replaced by "=="  The condition was always true for IOC order cancellation ***/
	{
		if(st_i_ordbk.c_ordr_stts == EXPIRED || st_i_ordbk.c_ordr_stts == PARTIALLY_EXECUTED_AND_EXPIRED )   /*** Ver 3.8 ***/
    {
      fn_userlog(c_ServiceName," Order Reference No. :%s: Is Already Expired. ",st_i_ordbk.c_ordr_rfrnc);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return 0;
    }
		st_i_xchngbk.c_plcd_stts = EXPIRED;
	}
	else
	{
		st_i_xchngbk.c_plcd_stts = CANCELLED;
  }
	/*** Ver 2.8 ends ***/

	st_i_xchngbk.c_rms_prcsd_flg = NOT_PROCESSED;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Book Modification Counter Is :%ld:",st_i_xchngbk.l_mdfctn_cntr);
		fn_userlog(c_ServiceName,"Exchange Book Placed Status Is :%c:",st_i_xchngbk.c_plcd_stts);
		fn_userlog(c_ServiceName,"Exchange Book RMS Processed Flag :%c:",st_i_xchngbk.c_rms_prcsd_flg);
		fn_userlog(c_ServiceName,"Exchange Book ORS Msg Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
		fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
		fn_userlog(c_ServiceName,"Exchange Book Remark Is :%s:",st_i_xchngbk.c_xchng_rmrks);
		fn_userlog(c_ServiceName,"Exchange Book Pipe ID Is :%s:",st_i_xchngbk.c_pipe_id);
		fn_userlog(c_ServiceName,"Exchange Book Order Reference Is :%s:",st_i_xchngbk.c_ordr_rfrnc);
		fn_userlog(c_ServiceName,"Exchange Book Exchange Code Is :%s:",st_i_xchngbk.c_xchng_cd);
		fn_userlog(c_ServiceName,"Exchange Book Trade Date Is :%s:",st_i_xchngbk.c_mod_trd_dt);
		fn_userlog(c_ServiceName,"Exchange Book Total Order Qty Is :%ld:",st_i_xchngbk.l_ord_tot_qty);
		fn_userlog(c_ServiceName,"Exchange Book Limit Rate Is :%ld:",st_i_xchngbk.l_ord_lmt_rt);
	}

	memcpy ( &ll_log_tm_stmp,ptr_oe_reqres->st_hdr.c_time_stamp_1, 8 );

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Log Time Stamp Is :%lld:",ll_log_tm_stmp);
	}

	st_i_xchngbk.d_jiffy = (double) ll_log_tm_stmp;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_xchngbk.d_jiffy);
	}
	
	st_i_xchngbk.l_dwnld_flg = i_dwnld_flg;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Book DownLoad Flag Is :%ld:",st_i_xchngbk.l_dwnld_flg);
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

  /*fn_cpy_ddr(st_i_xchngbk.c_rout_crt); */		/*** Ver 1.1 **/

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 **/

	st_i_xchngbk.l_bit_flg	=	ptr_oe_reqres->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1];		/***	Ver 1.7	***/

	sprintf(c_lst_act_ref,"%lld",ptr_oe_reqres->ll_lastactivityref);  /*** Ver 6.1 ***/
  rtrim(c_lst_act_ref);   /*** Ver 6.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"IN ORD_CNCL, the stream number receievd from exchange is:%ld:",st_i_xchngbk.l_bit_flg); 
		fn_userlog(c_ServiceName,"Before Call To SFO_UPD_XCHNGBK For INSERTION_ON_ORDER_FTE");
	}

	st_i_xchngbk.c_oprn_typ = INSERTION_ON_ORDER_FTE;

	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );

  if ( i_trnsctn ==  -1 )
  {
    fn_errlog(c_ServiceName,"L31065", LIBMSG, c_err_msg);
    return -1;
  }

/***	i_ch_val = fn_call_svc ( c_ServiceName,	*** Commented in Ver 6.1 ***
                           c_err_msg,
                           &st_i_xchngbk,
                           &st_i_xchngbk,
                           "vw_xchngbook",
                           "vw_xchngbook",
                           sizeof ( st_i_xchngbk ),
                           sizeof ( st_i_xchngbk ),
                           0,
                           "SFO_UPD_XCHNGBK" );	*** Ver 6.1 ***/

	i_ch_val = fn_upd_xchngbk_erl(&st_i_xchngbk,c_lst_act_ref,c_ServiceName,c_err_msg);		/*** Added in ver 6.1 ***/

  if ( ( i_ch_val != SUCC_BFR) &&
       ( i_ch_val != RECORD_ALREADY_PRCSD ) )
  {
    fn_errlog(c_ServiceName,"L31070", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }

	else if ( i_ch_val == RECORD_ALREADY_PRCSD )
	{
		fn_userlog ( c_ServiceName, "Record already processed" );
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return 0;
	}

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_XCHNGBK :%s:",c_time);
	}

	i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"L31075", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
 		fn_userlog(c_ServiceName,"ORS2-RD After Commiting Transaction :%s:",c_time);
	}

	if ( st_i_ordbk.c_prd_typ == OPTIONS || st_i_ordbk.c_prd_typ == OPTIONPLUS ) /*** Ver Option plus added in Ver 3.5 ***/
 	{
   	strcpy ( c_svc_name , "SFO_OPT_ACK" );
 	}
 	else if ( st_i_ordbk.c_prd_typ == FUTURES || st_i_ordbk.c_prd_typ == FUTURE_PLUS || st_i_ordbk.c_prd_typ == SLTP_FUTUREPLUS) /*** Ver Option plus added in Ver 3.5 ***/ 
 	{
   	strcpy ( c_svc_name , "SFO_FUT_ACK" );
 	}

  /*fn_cpy_ddr(st_i_xchngbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

  i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_err_msg,
                            &st_i_xchngbk,
                            "vw_xchngbook",
                            sizeof ( st_i_xchngbk ),
                            TPNOREPLY,
                            c_svc_name );
  if ( i_ch_val != SUCC_BFR )
  {
    fn_errlog(c_ServiceName,"L31080", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To OPT/FUT ACK :%s:",c_time);
		fn_userlog(c_ServiceName,"After Complition Of fn_ord_can");
	}

	return 0;

}

/******************************************************************************/
/*  To update details about order freeze / order tiggered / order expired     */
/*  INPUT PARAMETERS                                                          */
/*      ptr_oe_reqres - Pointer to a message structure got from Exchange      */
/*			c_pipe_id			- Pipe Id																								*/
/*			c_ordr_rfrnc  - Order Reference																				*/
/*			i_dwnld_flg   - Download Flag																					*/
/*      c_ServiceName - Service Name currently running                        */
/*      c_err_msg     - Error message to be returned                          */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/

int fn_ord_frz_exp ( struct st_oe_reqres *ptr_oe_reqres,
										 char   *c_xchng_cd,
										 char *c_pipe_id,	
										 char *c_rout_str,
										 char *c_trd_dt,
                     char *c_ordr_rfrnc,
										 int  i_dwnld_flg,
                     char *c_ServiceName,
                     char *c_err_msg)
{

	char c_xchng_rmrks[256];
	char c_svc_name [15 +1];
	char c_ord_res_stts;
  char c_ordr_rfrnc_tmp[19];  /** ver 4.7 **/
	char c_lst_act_ref[22];     /*** Ver 6.1 ***/

	int i_trnsctn;
  int i_ch_val;
  int i_ordr_sqnc=0;   /** ver 4.7 **/

	long long ll_log_tm_stmp;

	struct vw_xchngbook st_i_xchngbk;
  struct vw_xchngbook st_tmp_xchngbk;
  struct vw_orderbook st_i_ordbk;

	MEMSET(st_i_xchngbk);
	MEMSET(st_tmp_xchngbk);
	MEMSET(st_i_ordbk);
	MEMSET(c_xchng_rmrks);
  MEMSET(c_ordr_rfrnc_tmp);  /** ver 4.7 **/
	MEMSET(c_lst_act_ref);     /** Ver 6.1 **/

	if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug changed from 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside fn_ord_frz_exp");
	}
	
	switch (ptr_oe_reqres->st_hdr.si_transaction_code)
	{
		case FREEZE_TO_CONTROL :

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"FREEZE_TO_CONTROL Case");
			}

			st_i_xchngbk.l_ors_msg_typ = ORS_ORD_FRZ ;
			c_ord_res_stts = FREEZED;

			if(DEBUG_MSG_LVL_3)		
			{
				fn_userlog(c_ServiceName,"Message Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				fn_userlog(c_ServiceName,"Order Status Is :%c:",c_ord_res_stts);
			}

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_reqres->st_hdr.li_log_time );
			}

			fn_long_to_timearr(st_i_xchngbk.c_ack_tm,ptr_oe_reqres->st_hdr.li_log_time );

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
			}

      if ( ptr_oe_reqres->si_reason_code == 18 )
			{
				strcpy(c_xchng_rmrks , "Order Freeze - Quantity");
			
				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",c_xchng_rmrks);
				}
			}
			else
			{
				strcpy(c_xchng_rmrks , "Order Freeze - Price");

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",c_xchng_rmrks);
				}
			}
	
			break;

		case  ORS_ORD_EXP :
			
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"ORS_ORD_EXP Case");
			}

			c_ord_res_stts = EXPIRED;

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Order Status Is :%c:",c_ord_res_stts);
			}

			break;

		default :
      fn_userlog( c_ServiceName, "Protocol error %d", ptr_oe_reqres->st_hdr.si_transaction_code);
			break;
	}

	if ( c_ord_res_stts == EXPIRED )
	{
		sprintf (st_i_ordbk.c_xchng_ack, "%16.0lf", ptr_oe_reqres->d_order_number ); 
		strcpy ( st_i_ordbk.c_pipe_id , c_pipe_id );

	  /*fn_cpy_ddr(st_i_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		}

		strcpy(st_i_ordbk.c_rout_crt,c_rout_str); /*** Ver 1.1 ***/
		
		st_i_ordbk.c_oprn_typ = FOR_VIEW;
		
		i_ch_val = fn_call_svc ( c_ServiceName,
                             c_err_msg,
                             &st_i_ordbk,
                             &st_i_ordbk,
                             "vw_orderbook",
                             "vw_orderbook",
                             sizeof ( st_i_ordbk ),
                             sizeof ( st_i_ordbk ),
                             0,
                             "SFO_ACK_TO_ORD" );

    if ( i_ch_val != SUCC_BFR)
    {
      fn_errlog(c_ServiceName,"L31085", LIBMSG, c_err_msg);
      return -1;
    }

		fn_gettime();

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_ACK_TO_ORD :%s:",c_time);
		}

		strcpy ( st_i_xchngbk.c_ordr_rfrnc , st_i_ordbk.c_ordr_rfrnc );
		strcpy ( st_i_xchngbk.c_xchng_cd , st_i_ordbk.c_xchng_cd );
    strcpy ( st_i_xchngbk.c_pipe_id , st_i_ordbk.c_pipe_id );
    strcpy ( st_i_xchngbk.c_mod_trd_dt , ( char * )c_trd_dt );	
    st_i_xchngbk.l_ord_tot_qty = st_i_ordbk.l_ord_tot_qty;
    st_i_xchngbk.l_ord_lmt_rt = st_i_ordbk.l_ord_lmt_rt;
    st_i_xchngbk.l_ord_seq = 0;
    st_i_xchngbk.l_mdfctn_cntr = EXPIRY_MDFCTN_CNTR;
    st_i_xchngbk.c_rms_prcsd_flg = NOT_PROCESSED;
  }
	else
	{
    /**** Ver 4.7 Starts her ***/
   i_ordr_sqnc=0;
   i_ordr_sqnc = ptr_oe_reqres->i_ordr_sqnc;
    
    EXEC SQL
      SELECT FXB_ORDR_RFRNC
        INTO :c_ordr_rfrnc_tmp
        FROM FXB_FO_XCHNG_BOOK
       WHERE FXB_XCHNG_CD = :c_xchng_cd
         AND FXB_ORDR_SQNC = :i_ordr_sqnc
         AND FXB_PIPE_ID = :c_pipe_id
         AND FXB_MOD_TRD_DT = :c_trd_dt;
    if( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName,"L31090", SQLMSG , c_err_msg);
      return -1;
    }

    strcpy(c_ordr_rfrnc,c_ordr_rfrnc_tmp);
    /**** Ver 4.7 Ends here ****/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"I/P Order Reference No. Is :%s:",c_ordr_rfrnc);
			fn_userlog(c_ServiceName,"I/P Exchange Code Is :%s:",c_xchng_cd);
			fn_userlog(c_ServiceName,"I/P Pipe ID Is :%s:",c_pipe_id);
		}

		strcpy ( st_i_xchngbk.c_ordr_rfrnc , c_ordr_rfrnc );
		strcpy ( st_i_xchngbk.c_xchng_cd , c_xchng_cd );
    strcpy ( st_i_xchngbk.c_pipe_id , c_pipe_id );
    strcpy ( st_i_xchngbk.c_mod_trd_dt , ( char * )c_trd_dt );
    st_i_xchngbk.l_ord_seq = 0;
    st_i_xchngbk.l_mdfctn_cntr = 0;
    st_i_xchngbk.c_rms_prcsd_flg = PROCESSED;

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Exchange Volume Is :%ld:",ptr_oe_reqres->li_volume);
			fn_userlog(c_ServiceName,"Exchange Price Is :%ld:",ptr_oe_reqres->li_price);
		}

	  st_i_xchngbk.l_ord_tot_qty = ptr_oe_reqres->li_volume;
    st_i_xchngbk.l_ord_lmt_rt = ptr_oe_reqres->li_price;

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Exchange Book Total Order Qty Is :%ld:",st_i_xchngbk.l_ord_tot_qty);
			fn_userlog(c_ServiceName,"Exchange Book Order Limit Rate Is :%ld:",st_i_xchngbk.l_ord_lmt_rt);
		}
	}

	st_i_xchngbk.c_plcd_stts = c_ord_res_stts;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Placed Status Is :%c:",st_i_xchngbk.c_plcd_stts);
	}

	memcpy ( &ll_log_tm_stmp,ptr_oe_reqres->st_hdr.c_time_stamp_1, 8 );

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Log Time Stamp Is :%lld:",ll_log_tm_stmp);
	}

  st_i_xchngbk.d_jiffy = (double) ll_log_tm_stmp;
	strcpy(st_i_xchngbk.c_xchng_rmrks,c_xchng_rmrks);

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_xchngbk.d_jiffy);
	}

	st_i_xchngbk.l_dwnld_flg = i_dwnld_flg;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Book DownLoad Flag Is :%ld:",st_i_xchngbk.l_dwnld_flg);
	}

  /*fn_cpy_ddr(st_i_xchngbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

	st_i_xchngbk.l_bit_flg	=	ptr_oe_reqres->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1];			/***	Ver 1.7	***/

	sprintf(c_lst_act_ref,"%lld",ptr_oe_reqres->ll_lastactivityref);   /*** Ver 6.1 ***/
  rtrim(c_lst_act_ref);     /*** Ver 6.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"IN FRZ_EXP, the stream number receievd from exchange is:%ld:",st_i_xchngbk.l_bit_flg);  /*Ver 1.7*/
		fn_userlog(c_ServiceName,"Before Call To SFO_UPD_XCHNGBK With INSERTION_ON_ORDER_FTE");
		fn_userlog(c_ServiceName,"Last Activity Ref is :%s:",c_lst_act_ref);      /*** Ver 6.1 ***/
	}

  st_i_xchngbk.c_oprn_typ = INSERTION_ON_ORDER_FTE;

	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn ==  -1 )
  {
    fn_errlog(c_ServiceName,"L31095", LIBMSG, c_err_msg);
    return -1;
  }
	
/***	i_ch_val = fn_call_svc ( c_ServiceName,		*** Commented in Ver 6.1 ***
                           c_err_msg,
                           &st_i_xchngbk,
                           &st_i_xchngbk,
                           "vw_xchngbook",
                           "vw_xchngbook",
                           sizeof ( st_i_xchngbk ),
                           sizeof ( st_i_xchngbk ),
                           0,
                           "SFO_UPD_XCHNGBK" );		*** Ver 6.1 ***/

	i_ch_val = fn_upd_xchngbk_erl(&st_i_xchngbk,c_lst_act_ref,c_ServiceName,c_err_msg);   /*** Added in ver 6.1 ***/

  if ( ( i_ch_val != SUCC_BFR) &&
       ( i_ch_val != RECORD_ALREADY_PRCSD ) )
  {
    fn_errlog(c_ServiceName,"L31100", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }
	else if ( i_ch_val == RECORD_ALREADY_PRCSD )
	{	
		fn_userlog ( c_ServiceName, "Record already processed" );
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return 0;
	}

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_XCHNGBK :%s:",c_time);
	}

	if ( c_ord_res_stts == FREEZED )
	{
		if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug changed from 0 to 3 **/
		{
			fn_userlog(c_ServiceName,"Order Reference No. Is :%s:",c_ordr_rfrnc);
		}

		strcpy ( st_i_ordbk.c_ordr_rfrnc ,c_ordr_rfrnc);

		if(DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName,"Order Book Order Reference No. Is :%s:",st_i_ordbk.c_ordr_rfrnc);
		}

	  if(DEBUG_MSG_LVL_3)  /** ver 4.9 , debug changed from 0 to 3 **/
		{
			fn_userlog(c_ServiceName,"Exchange Ack Is :%lf:",ptr_oe_reqres->d_order_number);
		}

		sprintf (st_i_ordbk.c_xchng_ack, "%16.0lf", ptr_oe_reqres->d_order_number );

		if(DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName,"Order Book Ack No. Is :%s:",st_i_ordbk.c_xchng_ack);
		}

    /*fn_cpy_ddr(st_i_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		}

		strcpy(st_i_ordbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/		

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Before Call To SFO_UPD_ORDRBK With UPDATE_XCHNG_RESPONSE");
		}

    st_i_ordbk.c_oprn_typ = UPDATE_XCHNG_RESPONSE;

 		i_ch_val = fn_call_svc ( c_ServiceName,
                             c_err_msg,
                             &st_i_ordbk,
                             &st_i_ordbk,
                             "vw_orderbook",
                             "vw_orderbook",
                             sizeof ( st_i_ordbk ),
                             sizeof ( st_i_ordbk ),
                             0,
                             "SFO_UPD_ORDRBK" );
    if ( i_ch_val != SUCC_BFR)
    {
      fn_errlog(c_ServiceName,"L31105", LIBMSG, c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return -1;
    }

		strcpy ( st_i_ordbk.c_ordr_rfrnc ,c_ordr_rfrnc);
    st_i_ordbk.c_ordr_stts = FREEZED ;

   /* fn_cpy_ddr(st_i_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		}

		strcpy(st_i_ordbk.c_rout_crt,c_rout_str); /*** Ver 1.1 ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Before Call To SFO_UPD_ORDRBK With UPDATE_ORDER_STATUS");
		}

    st_i_ordbk.c_oprn_typ = UPDATE_ORDER_STATUS;

/***		i_ch_val = fn_call_svc ( c_ServiceName,		*** Commented in Ver 6.1 ***
                             c_err_msg,
                             &st_i_ordbk,
                             &st_i_ordbk,
                             "vw_orderbook",
                             "vw_orderbook",
                             sizeof ( st_i_ordbk ),
                             sizeof ( st_i_ordbk ),
                             0,
                             "SFO_UPD_ORDRBK" );	*** Ver 6.1 ***/

		i_ch_val = fn_upd_ordrbk_ref (&st_i_ordbk,c_lst_act_ref,c_ServiceName,c_err_msg);    /*** Ver 6.1 ***/	/*** doubt***/

    if ( i_ch_val != SUCC_BFR)
    {
      fn_errlog(c_ServiceName,"L31110", LIBMSG, c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return -1;
    }
	
		fn_gettime();

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_ORDRBK :%s:",c_time);
		}
	}

  i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);

  if ( i_ch_val == -1 )
  {
   	fn_errlog(c_ServiceName,"L31115", LIBMSG, c_err_msg);
   	return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Commit Transaction :%s:",c_time);
	}

  /****** In case of order expiration do the following  *****/
	if (c_ord_res_stts == EXPIRED )
  {

   	if ( st_i_ordbk.c_prd_typ == OPTIONS )
   	{
     	strcpy ( c_svc_name , "SFO_OPT_ACK" );
   	}
   	else if ( st_i_ordbk.c_prd_typ == FUTURES || st_i_ordbk.c_prd_typ == FUTURE_PLUS ) 
   	{
     	strcpy ( c_svc_name , "SFO_FUT_ACK" );
   	}

    /*fn_cpy_ddr(st_i_xchngbk.c_rout_crt); */		/*** Ver 1.1 ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		}

		strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
	

    i_ch_val = fn_acall_svc ( c_ServiceName,
                              c_err_msg,
                              &st_i_xchngbk,
                              "vw_xchngbook",
                              sizeof ( st_i_xchngbk ),
                              TPNOREPLY,
                              c_svc_name );
    if ( i_ch_val != SUCC_BFR )
    {
      fn_errlog(c_ServiceName,"L31120", LIBMSG, c_err_msg);
      return -1;
    }

		fn_gettime();

		if(DEBUG_MSG_LVL_3)
		{
 			fn_userlog(c_ServiceName,"ORS2-RD After Call To FUT/OPT ACK :%s:",c_time);
		}
  }

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"After Complition Of fn_ord_frz_exp");
	}

	return 0;
}

/******************************************************************************/
/*  This Handels Stop Loss Order Triggered Response.  									      */
/*  INPUT PARAMETERS                                                          */
/*      ptr_st_trd_conf- Pointer to a message structure got from Exchange     */
/*      c_pipe_id      - Pipe Id                                              */
/*      c_ord_res_stts - Order Response Status                                */
/*      i_dwnld_flg    - Download Flag                                        */
/*      c_ServiceName  - Service Name currently running                       */
/*      c_err_msg      - Error message to be returned                         */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/

int fn_stpls_trg ( struct st_trade_confirm *ptr_st_trd_conf,
									 char *c_xchng_cd,
									 char *c_pipe_id,
									 char *c_rout_str,
									 char *c_trd_dt,
                   char c_ord_res_stts,
                   char *c_ServiceName,
									 int  i_dwnld_flg,
                   char *c_err_msg )
{
   /**** Commented in ver 5.5 ***********
	*** ver 4.1 Starts***
	FBFR32 *ptr_fml_Sbuf;	
	ptr_fml_Sbuf  = ( FBFR32 *)NULL; 
	*** ver 4.1 Ends ***
  ******* Ver 5.5 Comment ends Here ********/

	int i_trnsctn;
  int i_ch_val;
  int i_returncode;
	/*** ver 4.1 Starts***/
	int i_xchngbkerr[7];			
	int i_ferr[19];				
	int i;						
	/*** ver 4.1 Ends***/
  char  c_ordr_stts='\0'; /** Ver 4.5 **/
	char c_mrkt_sl_flg = '\0'; /** Ver 5.0 **/
  char c_sltp_trail_flg = '\0'; /** Ver 5.0 **/
  char c_cep_flg = '\0'; /** Ver 5.0 **/
  char c_addmod_flg = '\0'; /** Ver 5.0 **/

	char frsh_rms_prcsd_flg= '\0';  /**** Ver 4.1***/
	char c_svc_name [ 16 ];
	char c_xchng_rmrks[256];
	char c_ordr_rfrnc [19];
  char c_refack_tmp[LEN_ORD_REFACK];
	char c_mtch_accnt [10 + 1];
	char c_spn_flg;				/***Ver	4.1	***/
	char	rms_prcsd_flg = '\0';		/*** ver 4.1 ***/
  char c_prd_typ='\0';				/*** ver 5.3 ****/
  /** long l_ftd_ord_exe_qty = 0; **** Ver 4.1 ***Commented in ver 5.4 *******/
	char c_lst_act_ref[22];     /*** Ver 6.1 ***/

  long long ll_log_tm_stmp;

  struct vw_xchngbook st_i_xchngbk;
  struct vw_orderbook st_i_ordbk;
	struct vw_xchngbook st_cvr_xchngbk; /*** ver 4.1**/

	MEMSET(st_i_xchngbk);
	MEMSET(st_i_ordbk);
	MEMSET(c_xchng_rmrks);
    MEMSET(st_cvr_xchngbk);			/*** ver 4.1 ***/
	MEMSET(c_lst_act_ref); 				/*** Ver 6.1 ***/

	if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug changed from 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside fn_stpls_trg");
	}

  /**** Commented in Ver 5.5 *************************
  	*** Ver 4.1 starts ***
	ptr_fml_Sbuf = (FBFR32*)tpalloc("FML32",NULL,MIN_FML_BUF_LEN);
	if ( ptr_fml_Sbuf == NULL )
	{
		fn_userlog(c_ServiceName,"ptr_fml_Sbuf is NULL ");
		fn_errlog(c_ServiceName, "S31010", TPMSG,  c_err_msg);
		tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
	}
	*** Ver 4.1 ends ***
  ********** Ver 5.5 Ends here ***************/

  /*** strcpy( c_xchng_rmrks , "Stop Loss Order Triggered" );  *** Commented in Ver 3.6 ***/

	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn ==  -1 )
  {
    fn_errlog(c_ServiceName,"L31125", LIBMSG, c_err_msg);
    return -1;
  }

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Ack Is :%lf:",ptr_st_trd_conf->d_response_order_number);
	}

	sprintf(st_i_ordbk.c_xchng_ack,"%16.0lf",ptr_st_trd_conf->d_response_order_number);

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"Order Book Exchange Ack Is :%s:",st_i_ordbk.c_xchng_ack);
	}

  strcpy ( st_i_ordbk.c_pipe_id , c_pipe_id );

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Order Book Pipe ID Is :%s:",st_i_ordbk.c_pipe_id);
	}

  /*fn_cpy_ddr(st_i_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_ordbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Before Call To SFO_ACK_TO_ORD With FOR_UPDATE");
	}

  /**Ver 4.5 Starts ***/

  fn_userlog( c_ServiceName, "Trying to take lock for Xchange Ack :%s: for pipe id :%s:",st_i_ordbk.c_xchng_ack, st_i_ordbk.c_pipe_id);

   c_ordr_stts ='\0';

   EXEC SQL
     SELECT FOD_ORDR_STTS , 
						 FOD_LMT_MRKT_SL_FLG,    /*** Ver 5.0 **/
             NVL(FOD_PRCIMPV_FLG,'N'),         /*** Ver 5.0 **/
             FOD_ORDR_RFRNC ,									/*** Ver 5.3 ***/
						 FOD_PRDCT_TYP                    /*** Ver 5.3 ***/
      INTO :c_ordr_stts,
						 :c_mrkt_sl_flg,           /*** Ver 5.0 **/
           	 :c_sltp_trail_flg,         /*** Ver 5.0 **/
             :c_ordr_rfrnc, 									 /**** ver 5.3 ***/
						 :c_prd_typ	                       /*** Ver 5.3 ***/
      FROM FOD_FO_ORDR_DTLS
      WHERE FOD_ACK_NMBR   = :st_i_ordbk.c_xchng_ack
      AND FOD_PIPE_ID   = :st_i_ordbk.c_pipe_id
    FOR   UPDATE OF FOD_ORDR_RFRNC NOWAIT;

    if ( SQLCODE != 0)
    {
      if (SQLCODE == -54)
      {
        fn_userlog( c_ServiceName, "Resource busy and acquire with NOWAIT specified or timeout expired");
      }

      fn_errlog(c_ServiceName,"L31130", SQLMSG, c_err_msg);
      fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
      return -1;
    }
    else
    {
      fn_userlog( c_ServiceName, "Sucessfully acquired FOR UPDATE NOWAIT on :%s: for pipe id  :%s:",st_i_ordbk.c_xchng_ack, st_i_ordbk.c_pipe_id);
    }

    fn_userlog( c_ServiceName, "c_ordr_stts :%c:",c_ordr_stts);

    if( c_ordr_stts == 'Q' )
    {
      fn_userlog(c_ServiceName,"Order in Queued Status, trigger write in error queue"); 
      fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
   		
    	i_returncode =  fn_fxb_rms_erl ( c_ServiceName,        /** Function calling added in ver 5.3 **/
                                        c_err_msg,
                                        c_prd_typ,
                                        c_ordr_rfrnc) ;

      return -1;
    }
    
  /**Ver 4.5 Ends ***/ 

  st_i_ordbk.c_oprn_typ = FOR_UPDATE;

	i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_i_ordbk,
                           &st_i_ordbk,
                           "vw_orderbook",
                           "vw_orderbook",
                           sizeof ( st_i_ordbk ),
                           sizeof ( st_i_ordbk ),
                           0,
                           "SFO_ACK_TO_ORD" );


  if ( i_ch_val != SUCC_BFR)
  {
    fn_errlog(c_ServiceName,"L31135", LIBMSG, c_err_msg);
    fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg ); 
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_ACK_TO_ORD For Update :%s:",c_time);
	}

	strcpy(c_ordr_rfrnc,st_i_ordbk.c_ordr_rfrnc);

	/**i_returncode = fn_lock_usr( c_ServiceName,  st_i_ordbk.c_cln_mtch_accnt ); ** Ver 7.3 **/
	i_returncode = fn_lock_fno( c_ServiceName,  st_i_ordbk.c_cln_mtch_accnt ); /** Ver 7.3 **/
  if ( i_returncode != 0 )
  {
    fn_errlog ( c_ServiceName, "L31140", LIBMSG, c_err_msg );
    fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
    return -1;
  }

  strcpy ( st_i_xchngbk.c_ordr_rfrnc , st_i_ordbk.c_ordr_rfrnc );
  strcpy ( st_i_xchngbk.c_xchng_cd , st_i_ordbk.c_xchng_cd );
  strcpy ( c_mtch_accnt , st_i_ordbk.c_cln_mtch_accnt );           
  strcpy ( st_i_xchngbk.c_pipe_id , st_i_ordbk.c_pipe_id );
  strcpy ( st_i_xchngbk.c_mod_trd_dt , ( char * )c_trd_dt );
  st_i_xchngbk.l_ord_tot_qty = st_i_ordbk.l_ord_tot_qty;
  st_i_xchngbk.l_ord_lmt_rt = st_i_ordbk.l_ord_lmt_rt;
  st_i_xchngbk.l_ord_seq = 0;
  st_i_xchngbk.l_mdfctn_cntr = 0;

  if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_st_trd_conf->li_activity_time);
	}

  fn_long_to_timearr ( st_i_xchngbk.c_ack_tm, ptr_st_trd_conf->li_activity_time );

  strcpy( c_xchng_rmrks , "Stop Loss Order Triggered at " );    /*** Ver 3.6 ***/
  strcat( c_xchng_rmrks , st_i_xchngbk.c_ack_tm );            /*** Ver 3.6 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
		fn_userlog(c_ServiceName,"Order Status Is :%c:",c_ord_res_stts);
	}

  st_i_xchngbk.c_plcd_stts = c_ord_res_stts;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Book Placed Status Is :%c:",st_i_xchngbk.c_plcd_stts);
	}

  st_i_xchngbk.c_rms_prcsd_flg = PROCESSED;
	st_i_xchngbk.l_ors_msg_typ = ORS_ORD_TRG;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",c_xchng_rmrks);
	}

	strcpy( st_i_xchngbk.c_xchng_rmrks ,c_xchng_rmrks);

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Book Remark Is :%s:",st_i_xchngbk.c_xchng_rmrks);
	}

	st_i_xchngbk.l_dwnld_flg = i_dwnld_flg;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Book DownLoad Flag Is :%ld:",st_i_xchngbk.l_dwnld_flg);
	}

	memcpy(&ll_log_tm_stmp,ptr_st_trd_conf->st_hdr.c_time_stamp_1, 8);

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Log Time Stamp Is :%lld:",ll_log_tm_stmp);
	}

	st_i_xchngbk.d_jiffy = (double) ll_log_tm_stmp;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_xchngbk.d_jiffy);
	}

  /*fn_cpy_ddr(st_i_xchngbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str); /*** Ver 1.1 ***/

	st_i_xchngbk.l_bit_flg	=	ptr_st_trd_conf->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1];		/***	Ver	1.7	***/

	sprintf(c_lst_act_ref,"%lld",ptr_st_trd_conf->ll_lastactivityref); /*** Ver 6.1 ***/
  rtrim(c_lst_act_ref);   /*** Ver 6.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"IN SLTP_TRG, the stream number receievd from exchange is:%ld:",st_i_xchngbk.l_bit_flg);  
		fn_userlog(c_ServiceName,"Before Call To SFO_UPD_XCHNGBK With INSERTION_ON_ORDER_FTE");
		fn_userlog(c_ServiceName,"Last Activity Ref is :%s:",c_lst_act_ref);      /*** Ver 6.1 ***/
	}

  st_i_xchngbk.c_oprn_typ = INSERTION_ON_ORDER_FTE;

/***	i_ch_val = fn_call_svc ( c_ServiceName,		*** Commented in Ver 6.1 ***
                           c_err_msg,
                           &st_i_xchngbk,
                           &st_i_xchngbk,
                           "vw_xchngbook",
                           "vw_xchngbook",
                           sizeof ( st_i_xchngbk ),
                           sizeof ( st_i_xchngbk ),
                           0,
                           "SFO_UPD_XCHNGBK" );		*** Ver 6.1 ***/

	i_ch_val = fn_upd_xchngbk_erl(&st_i_xchngbk,c_lst_act_ref,c_ServiceName,c_err_msg);   /*** Added in ver 6.1 ***/

  if ( ( i_ch_val != SUCC_BFR ) &&
       ( i_ch_val != RECORD_ALREADY_PRCSD ) )
  {
    fn_errlog(c_ServiceName,"L31145", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }

	else if ( i_ch_val == RECORD_ALREADY_PRCSD )
	{
		fn_userlog ( c_ServiceName, "Record already processed" );
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return 0;
	}

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_XCHNGBK :%s:",c_time);
		fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_st_trd_conf->li_activity_time);
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

  fn_long_to_timearr ( st_i_ordbk.c_ack_tm, ptr_st_trd_conf->li_activity_time );

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Order Book Ack Time Is :%s:",st_i_ordbk.c_ack_tm);
	}

  /*fn_cpy_ddr(st_i_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/
	strcpy(st_i_ordbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Before Call To SFO_UPD_ORDRBK With UPDATE_XCHNG_RESP_DTLS_OT");
	}

  st_i_ordbk.c_oprn_typ = UPDATE_XCHNG_RESP_DTLS_OT;

/***	i_returncode = fn_call_svc( c_ServiceName,		*** Commented in Ver 6.1 ***
                              c_err_msg,
                              &st_i_ordbk,
                              &st_i_ordbk,
                              "vw_orderbook",
                              "vw_orderbook",
                              sizeof ( st_i_ordbk ),
                              sizeof ( st_i_ordbk ),
                              0,
                              "SFO_UPD_ORDRBK" );		*** Ver 6.1 ***/

	i_ch_val = fn_upd_ordrbk_ref (&st_i_ordbk,c_lst_act_ref,c_ServiceName,c_err_msg);		/*** Ver 6.1 ***/

  if ( i_returncode != SUCC_BFR )
  {
    fn_errlog ( c_ServiceName, "L31150", LIBMSG, c_err_msg );
    fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_ORDRBK With UPDATE_XCHNG_RESP_DTLS_OT :%s:",c_time);
	}

	/*** Ver 5.0 Starts Here ****/
  c_cep_flg='\0';
  c_addmod_flg='\0';
  if(st_i_ordbk.c_prd_typ =='F' )
  {
     if( c_mrkt_sl_flg == 'S' && c_sltp_trail_flg == 'Y' )
     {
			 if(DEBUG_MSG_LVL_3)
			 {
    			fn_userlog(c_ServiceName," st_i_ordbk.c_prd_typ*********:%c:",st_i_ordbk.c_prd_typ);
    			fn_userlog(c_ServiceName," st_i_ordbk.c_xchng_cd********:%s:",st_i_ordbk.c_xchng_cd);
    			fn_userlog(c_ServiceName," st_i_ordbk.c_ordr_rfrnc******:%s:",st_i_ordbk.c_ordr_rfrnc);
    			fn_userlog(c_ServiceName," st_i_ordbk.c_undrlyng********:%s:",st_i_ordbk.c_undrlyng);
    			fn_userlog(c_ServiceName," st_i_ordbk.c_expry_dt********:%s:",st_i_ordbk.c_expry_dt);
    			fn_userlog(c_ServiceName," st_i_ordbk.c_ordr_flw********:%c:",st_i_ordbk.c_ordr_flw);
			  }

       /*** Send the instruction to CEP to delete the record from CEP MAP ****/
       c_cep_flg ='R';
       i_returncode  = fn_cal_incrmntl_price(c_ServiceName,
                              c_addmod_flg,
                              c_cep_flg,
                              c_err_msg,
                              &st_i_ordbk,
                              &st_i_xchngbk
                              );
       if ( i_returncode != 0 )
       {
          fn_errlog ( c_ServiceName, "L31155", LIBMSG, c_err_msg );
          fn_userlog( c_ServiceName, "Error while calling fn_cal_incrmntl_price");
       }


     }
  }
  /*** Ver 5.0 Ends here *****/

	i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"L31160", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Commiting Transaction :%s:",c_time);
		fn_userlog(c_ServiceName,"After Complition Of fn_stpls_trg");
	}
	
	

	/******************** Added in Ver 4.1 starts *************************/
		
		MEMSET(st_cvr_xchngbk);
    i_returncode =  fn_fxb_rms_erl  (  c_ServiceName,        /****** Function calling added in ver 5.3 ********/
                   											c_err_msg,
                    										c_prd_typ,
                    										c_ordr_rfrnc) ; 

  	if ( i_returncode == -1 )
  	{
  	  fn_errlog ( c_ServiceName, "L31165", LIBMSG, c_err_msg );
  	  return -1;
  	}


   i_returncode =  fn_ftd_rms_erl ( c_ServiceName,        /** Function calling added in ver 5.3 **/
                                        c_err_msg,
                                        c_prd_typ,
                                        c_ordr_rfrnc) ;

    if ( i_returncode == -1 )
    {
      fn_errlog ( c_ServiceName, "L31170", LIBMSG, c_err_msg );
      return -1;
    }

   /***** Commented in Ver 5.3 Ends  ****

  if(st_i_ordbk.c_prd_typ =='U' || st_i_ordbk.c_prd_typ =='I')
  {
		
		MEMSET(st_cvr_xchngbk);

		EXEC SQL
		 SELECT
  	 fxb_rms_prcsd_flg
 	  INTO
 	  :frsh_rms_prcsd_flg
 	  FROM FXB_FO_XCHNG_BOOK
 	  WHERE FXB_ORDR_RFRNC = :c_ordr_rfrnc
 	  AND    FXB_MDFCTN_CNTR = (select max(FXB_MDFCTN_CNTR)
   													 from   FXB_FO_XCHNG_BOOK
   													 where  FXB_ORDR_RFRNC = :c_ordr_rfrnc);

 	  if(SQLCODE != 0)
 	  {
 	     fn_errlog(c_ServiceName, "S31015", SQLMSG,  c_err_msg);
			 return 0;
 	  }

	
		if( frsh_rms_prcsd_flg != 'N' )
		{
			 fn_userlog( c_ServiceName, "No unprocessed record");
		}
		else
		{
				EXEC SQL
				SELECT FXB_ORS_MSG_TYP,
					 FXB_MDFCTN_CNTR,
					 nvl(to_char(FXB_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss'),'-')
				INTO   :st_cvr_xchngbk.l_ors_msg_typ,
					 :st_cvr_xchngbk.l_mdfctn_cntr,
					 :st_cvr_xchngbk.c_ack_tm
				FROM   FXB_FO_XCHNG_BOOK
				WHERE  FXB_ORDR_RFRNC = :c_ordr_rfrnc
				AND    FXB_MDFCTN_CNTR = (select max(FXB_MDFCTN_CNTR)
										from   FXB_FO_XCHNG_BOOK
										where  FXB_ORDR_RFRNC = :c_ordr_rfrnc)
				AND    FXB_RMS_PRCSD_FLG = 'N';

			if(SQLCODE !=0)
			{
				fn_errlog(c_ServiceName, "S31020", SQLMSG, c_err_msg);
					return 0 ;
			}	

			***************** Commented in Ver 5.3 ******************

			EXEC SQL
			SELECT  NVL(FOD_SPN_FLG,'N')
			INTO    :c_spn_flg
			FROM    FOD_FO_ORDR_DTLS
			where FOD_ORDR_RFRNC =:c_ordr_rfrnc;
			if ( SQLCODE != 0 )
			{
				fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");
				fn_errlog(c_ServiceName, "S31025", SQLMSG,  c_err_msg);
				return 0;
			}

			fn_cpy_ddr ( st_cvr_xchngbk.c_rout_crt );
			strcpy(st_cvr_xchngbk.c_entry_dt_tm,st_cvr_xchngbk.c_ack_tm);

			if(DEBUG_MSG_LVL_3)  ** Ver 4.9, changed from 0 to 3 **
			{
				fn_userlog( c_ServiceName, "Before Calling  SFO_FUT_ACK/SFO_OPT_ACK/SFO_PRCS_RSP for Order Ref:%s:", c_ordr_rfrnc);
				fn_userlog( c_ServiceName, "st_cvr_xchngbk.c_entry_dt_tm:%s:", st_cvr_xchngbk.c_entry_dt_tm);
				fn_userlog( c_ServiceName, "st_cvr_xchngbk.c_ack_tm:%s:", st_cvr_xchngbk.c_ack_tm);
				fn_userlog( c_ServiceName, "st_cvr_xchngbk.l_mdfctn_cntrm:%ld:", st_cvr_xchngbk.l_mdfctn_cntr);
				fn_userlog( c_ServiceName, "st_cvr_xchngbk.l_ors_msg_typ:%ld:", st_cvr_xchngbk.l_ors_msg_typ);
			}

    	EXEC SQL
      SELECT  NVL(SUM(ftd_exctd_qty),0)
      INTO    :l_ftd_ord_exe_qty
      FROM    ftd_fo_trd_dtls
      WHERE   ftd_ordr_rfrnc = :c_ordr_rfrnc;

    	if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
    	{
    	  fn_errlog ( c_ServiceName, "S31030", SQLMSG, c_err_msg );
				fn_userlog(c_ServiceName,"Error While Selecting executed qty .");
				return 0;
    	}
			
      if( DEBUG_MSG_LVL_3 )  ** ver 4.9 ***
      {	
			  fn_userlog( c_ServiceName, "l_ftd_ord_exe_qty: %ld",l_ftd_ord_exe_qty); 
      }

      st_cvr_xchngbk.l_xchng_can_qty = l_ftd_ord_exe_qty;

			if ( c_spn_flg != 'S' )
			{
      if( DEBUG_MSG_LVL_0 )  ** ver 4.9 ***
      {
				fn_userlog( c_ServiceName, "unprocessed record exist so process it...");
      }

				st_cvr_xchngbk.d_jiffy       = 0;
				st_cvr_xchngbk.l_dwnld_flg   = 0;
				strcpy(st_cvr_xchngbk.c_xchng_rmrks,"*");
				strcpy(st_cvr_xchngbk.c_ordr_rfrnc, c_ordr_rfrnc);
		

				if(st_i_ordbk.c_prd_typ =='I')
				{
						i_returncode = fn_acall_svc ( c_ServiceName,
																					c_err_msg,
																					&st_cvr_xchngbk,
																					"vw_xchngbook",
																					sizeof ( st_cvr_xchngbk),
																					TPNOREPLY,
																					"SFO_OPT_ACK" );
						if ( i_returncode != 0 )
						{
							fn_userlog( c_ServiceName, "Error while calling SFO_OPT_ACK");
							fn_errlog( c_ServiceName, "S31035",LIBMSG, c_err_msg); 
							return 0 ;

						}
						
	      ***************** Commented in Ver 5.3 ******************

				}
				else if(st_i_ordbk.c_prd_typ =='U')
				{
						i_returncode = fn_acall_svc ( c_ServiceName,
																					c_err_msg,
																					&st_cvr_xchngbk,
																					"vw_xchngbook",
																					sizeof ( st_cvr_xchngbk),
																					TPNOREPLY,
																					"SFO_FUT_ACK" );
						if ( i_returncode != 0 )
						{
							fn_userlog( c_ServiceName, "Error while calling SFO_FUT_ACK");
							fn_errlog( c_ServiceName, "S31040", LIBMSG, c_err_msg);
							return 0 ;

						}
				} 
			}
			else
			{
				***************** Commented in Ver 5.3 ******************					 
				i_xchngbkerr [0] = Fadd32(ptr_fml_Sbuf,FFO_ROUT_CRT, (char *)st_cvr_xchngbk.c_rout_crt,0);
				i_ferr[0] = Ferror32;
				i_xchngbkerr [1] = Fadd32(ptr_fml_Sbuf,FFO_ORDR_RFRNC, (char *)st_i_ordbk.c_ordr_rfrnc,0);
				i_ferr[1] = Ferror32;
				i_xchngbkerr [2] = Fadd32(ptr_fml_Sbuf,FFO_ORS_MSG_TYP, (char *)&st_cvr_xchngbk.l_ors_msg_typ,0);
				i_ferr[2] = Ferror32;
				i_xchngbkerr [3] = Fadd32(ptr_fml_Sbuf,FFO_MDFCTN_CNTR, (char *)&st_cvr_xchngbk.l_mdfctn_cntr,0);
				i_ferr[3] = Ferror32;
				i_xchngbkerr [4] = Fadd32(ptr_fml_Sbuf,FFO_ACK_TM, (char *)st_cvr_xchngbk.c_ack_tm,0);
				i_ferr[4] = Ferror32;
				i_xchngbkerr [5] = Fadd32(ptr_fml_Sbuf,FFO_PREV_ACK_TM, (char *)st_cvr_xchngbk.c_entry_dt_tm,0);
				i_ferr[5] = Ferror32;
				i_xchngbkerr [6] = Fadd32(ptr_fml_Sbuf,FFO_XCHNG_CAN_QTY , (char *)&st_cvr_xchngbk.l_xchng_can_qty,0);
				i_ferr[6] = Ferror32;

				for (i = 0;i < 6 ;i++)
				{
					if(i_xchngbkerr[i] == -1 )
					{
						fn_userlog( c_ServiceName, "Error in :%d:",i );
						tpfree((char *)ptr_fml_Sbuf);
						fn_errlog(c_ServiceName, "S31045", FMLMSG,  c_err_msg);
						return 0;
					}
				}

				i_returncode  = tpacall ( "SFO_PRCS_RSP", (char *)ptr_fml_Sbuf,0, TPNOTRAN|TPNOREPLY);

				if ( i_returncode != SUCC_BFR )
				{
					fn_userlog( c_ServiceName, "Error while calling SFO_PRCS_RSP");
					fn_errlog ( c_ServiceName, "S31050", LIBMSG, c_err_msg );
					return 0;
				}
			}
    }  
	 }      Commented in Ver 5.3 Ends  ****/

	/**************** Ver 4.1 Ends *******************/
	
	return 0;
}


int fn_sprd_ord_confirmation ( struct st_spd_oe_reqres *ptr_st_spdoe_reqres,
															 char   *c_xchng_cd,
												  		 char   *c_pipe_id,
															 char   *c_rout_str,
															 char   *c_trd_dt,
                          		 int    i_dwnld_flg,
                          		 char   *c_ServiceName,
                          		 char   *c_err_msg )
{
	int i_ch_val;
	
	if(DEBUG_MSG_LVL_3)  /*** Ver 4.9, debug changed from 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside fn_sprd_ord_confirmation ");
	}

  switch (ptr_st_spdoe_reqres->st_hdr.si_transaction_code)
	{
		case SP_ORDER_CONFIRMATION :
		case SP_ORDER_ERROR :
		case TWOL_ORDER_CONFIRMATION :
		case TWOL_ORDER_ERROR :
		case THRL_ORDER_CONFIRMATION :
		case THRL_ORDER_ERROR :
    case SP_ORDER_MOD_CON_OUT:   /** Ver 5.7 ***/
    case SP_ORDER_MOD_REJ_OUT:  /** Ver 5.7 ***/

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Before Call To fn_spd_acpt_rjct");
			}

			i_ch_val = fn_spd_acpt_rjct(ptr_st_spdoe_reqres,
																	c_xchng_cd,
																	c_pipe_id,
																	c_rout_str,
																	c_trd_dt,
																	i_dwnld_flg,
																	c_ServiceName,
																	c_err_msg);

			if (i_ch_val == -1)
      {
        fn_userlog(c_ServiceName,"Failed While Calling Function fn_spd_acpt_rjct");
        return -1;
      }

			break;

		case SP_ORDER_CXL_CONFIRM_OUT :
		case TWOL_ORDER_CXL_CONFIRMATION :
		case THRL_ORDER_CXL_CONFIRMATION :
    case SP_ORDER_CANCEL_REJ_OUT:  /** ver 5.7 **/

			/***	Commented In Ver 1.5
			fn_userlog(c_ServiceName,"Before Going To Sleep For 3 Seconds In 2L/3L Cancellation Confirmation");
			sleep(3);					Added to differ execution till the spred order confirmation is processed 	***/

		 sleep(3);					/*** Ver 2.1 ***/
		
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Before Call To fn_spd_ord_cncl");
			}

      if ( ptr_st_spdoe_reqres->st_order_flgs.flg_ioc == 1 )  /** Ver 5.7 if condition added **/
      {	
			i_ch_val = fn_spd_ord_cncl ( ptr_st_spdoe_reqres,
																	 c_xchng_cd,
																	 c_pipe_id,
																	 c_rout_str,
																	 c_trd_dt,
																	 i_dwnld_flg,
																	 c_ServiceName,
																	 c_err_msg);

			if (i_ch_val == -1)
      {
        fn_userlog(c_ServiceName,"Failed While Calling Function fn_spd_ord_cncl");
        return -1;
      }
      }  /** Ver 5.7 starts here ***/
      else
      {
        i_ch_val = fn_spd_acpt_rjct(ptr_st_spdoe_reqres,
                                  c_xchng_cd,
                                  c_pipe_id,
                                  c_rout_str,
                                  c_trd_dt,
                                  i_dwnld_flg,
                                  c_ServiceName,
                                  c_err_msg);

        if (i_ch_val == -1)
        {
          fn_userlog(c_ServiceName,"Failed While Calling Function fn_spd_acpt_rjct");
          return -1;
        }
       }  /*** ver 5.7 Ends Here ***/

			break;
    case BATCH_SPREAD_CXL_OUT: /*** suchita ver 5.7 ***/
      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Inside BATCH_SPREAD_CXL_OUT Case");
      }

        i_ch_val = fn_spd_ord_cncl ( ptr_st_spdoe_reqres,
                                   c_xchng_cd,
                                   c_pipe_id,
                                   c_rout_str,
                                   c_trd_dt,
                                   i_dwnld_flg,
                                   c_ServiceName,
                                   c_err_msg);

        if (i_ch_val == -1)
        {
          fn_userlog(c_ServiceName,"Failed While Calling Function fn_spd_ord_cncl");
          return -1;
        }

        break; /*** suchita ver 5.7 ends ***/

		default :
      fn_userlog( c_ServiceName, "Protocol error %d", ptr_st_spdoe_reqres->st_hdr.si_transaction_code);
			break;
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"After Complition Of fn_sprd_ord_confirmation");
	}

	return 0;
}

/******************************************************************************/
/*  To Update Accept Reject Response From Exchange.                           */
/*  INPUT PARAMETERS                                                          */
/*      ptr_st_spdoe_reqres- Pointer to a message structure got from Exchange */
/*      c_pipe_id      - Pipe Id                                              */
/*      i_dwnld_flg    - Download Flag                                        */
/*      c_ServiceName  - Service Name currently running                       */
/*      c_err_msg      - Error message to be returned                         */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/

int fn_spd_acpt_rjct ( struct st_spd_oe_reqres *ptr_st_spdoe_reqres,
											 char   *c_xchng_cd,
											 char   *c_pipe_id,
											 char   *c_rout_str,
										   char   *c_trd_dt,
                       int    i_dwnld_flg,
                       char   *c_ServiceName,
                       char   *c_err_msg )
{
	char c_svc_name [15+1];
	char c_refack_tmp[LEN_ORD_REFACK];
	char c_xchng_rmrks [256];
	char c_mtch_accnt [10 + 1];
	char c_xchng_err_msg [256]; 
	char c_spd_ord_seq[10 +1 ];			/** Ver 2.1 **/
	int i_ch_val;
  int i_trnsctn;
  int i_cnt;
  int i_rec_cnt;
  int i_rec_exists = 0;           /** Ver 7.0 **/
	char c_lst_act_ref[22];         /** Ver 6.1 **/

	long long ll_tmp_log_tm_stmp;

	long li_temp_bit_flg;							/**Ver 1.7 **/

	struct vw_xchngbook st_i_xchngbk;
  struct vw_orderbook st_i_ordrbk;
  struct vw_exrcbook st_i_exrcbk;
  struct vw_spdordbk st_spd_ordbk;
  struct vw_spdxchbk st_spd_xchbk;

	MEMSET(st_i_xchngbk);
	MEMSET(st_i_ordrbk);
	MEMSET(st_i_exrcbk);
	MEMSET(st_spd_ordbk);
	MEMSET(st_spd_xchbk);
	MEMSET(c_xchng_rmrks);
	MEMSET(c_xchng_err_msg);
	MEMSET(c_spd_ord_seq);					/*** Ver 2.1 ***/
	MEMSET(c_lst_act_ref);					/*** Ver 6.1 ***/

	if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug changed from 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside Function fn_spd_acpt_rjct");
	}

	memcpy ( &ll_tmp_log_tm_stmp, ptr_st_spdoe_reqres->st_hdr.c_time_stamp_1, 8 );

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Log Time stamp Is :%lld:",ll_tmp_log_tm_stmp);
	}
	/**** Ver 2.1 Comment Starts *****************************
  fn_nsetoors_char ( st_spd_ordbk.c_sprd_ord_rfrnc[0],
                     LEN_ORDR_RFRNC,
                     ptr_st_spdoe_reqres->c_oe_remarks,
                     LEN_REMARKS );
	*********************Ver 2.1 Comment ends *******************/
	/******** Ver 2.1 new code Starts ************/
	EXEC SQL
		SELECT to_char(sysdate ,'yyyymmdd') ||:c_pipe_id 
		INTO :c_spd_ord_seq
		FROM DUAL;	
	if(SQLCODE != 0)
	{
		fn_errlog(c_ServiceName,"S31055",SQLMSG,c_err_msg);
		return -1;
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"spread order seq before sprintf=:%s:",st_spd_ordbk.c_sprd_ord_rfrnc);
	}
	memset ( st_spd_ordbk.c_sprd_ord_rfrnc, 0,sizeof (st_spd_ordbk.c_sprd_ord_rfrnc) );
	sprintf(st_spd_ordbk.c_sprd_ord_rfrnc[0],"%s%08d",c_spd_ord_seq,ptr_st_spdoe_reqres->i_sprd_seq_no);
	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"spread order seq after sprintf=:%s:",st_spd_ordbk.c_sprd_ord_rfrnc);
	}

	/*** Ver 2.1 New Code Ends ***/
  /*fn_cpy_ddr(st_spd_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_spd_ordbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

  st_spd_ordbk.c_rqst_typ[0] = SPDREF_TO_ORDREF;

  i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_spd_ordbk,
                           &st_spd_ordbk,
                           "vw_spdordbk",
                           "vw_spdordbk",
                           sizeof ( st_spd_ordbk ),
                           sizeof ( st_spd_ordbk ),
                           0,
                           "SFO_QRY_SPDBK" );
  if ( i_ch_val != SUCC_BFR)
  {
    fn_errlog(c_ServiceName,"L31175", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"After Call To SFO_QRY_SPDBK :%s:",c_time);
	}

	sprintf(st_spd_ordbk.c_xchng_ack[0],"%16.0lf",ptr_st_spdoe_reqres->d_order_number );

	if(DEBUG_MSG_LVL_0)
	{
  	/*** fn_userlog(c_ServiceName,"Spread Order Reference Is :%s:",st_spd_ordbk.c_sprd_ord_rfrnc[0]);
  	fn_userlog(c_ServiceName,"Exchange Ack Is :%s:",st_spd_ordbk.c_xchng_ack[0]); **** Ver 4.9 **/
    fn_userlog(c_ServiceName,"fn_spd_acpt_rjct:Exchange Ack Is :%s:",st_spd_ordbk.c_xchng_ack[0]); /**** Ver 4.9 **/
	}

  switch (ptr_st_spdoe_reqres->st_hdr.si_transaction_code)
	{
		case	SP_ORDER_CONFIRMATION :

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside SP_ORDER_CONFIRMATION Case");		
			}

			for ( i_cnt=0; i_cnt<=1; i_cnt++ )
			{
				st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_SPN_ORD_ACPT;

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
				}
			}
	
			break;
		
		case	SP_ORDER_ERROR :

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside SP_ORDER_ERROR Case");
				fn_userlog(c_ServiceName,"The ERROR CODE FOR ORDER RJCT IS :%d:",ptr_st_spdoe_reqres->st_hdr.si_error_code);
			}

      i_ch_val = fn_get_errmsg( ptr_st_spdoe_reqres->st_hdr.si_error_code,ptr_st_spdoe_reqres->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

			if(i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
        return -1;
      }

      strcpy(c_xchng_rmrks,c_xchng_err_msg);

			for ( i_cnt=0; i_cnt<=1; i_cnt++ )
			{
				st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_SPN_ORD_RJCT;

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
				}
			}

			break;

		case	TWOL_ORDER_CONFIRMATION :
		
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside TWOL_ORDER_CONFIRMATION Case");
			}

			for ( i_cnt=0; i_cnt<=1; i_cnt++ )
			{
				st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_2LN_ORD_ACPT;

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
				}
			}
			break;

		case	TWOL_ORDER_ERROR :


			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside TWOL_ORDER_ERROR Case");
				fn_userlog(c_ServiceName,"The ERROR CODE FOR ORDER RJCT IS :%d:",ptr_st_spdoe_reqres->st_hdr.si_error_code);
			}

      i_ch_val = fn_get_errmsg( ptr_st_spdoe_reqres->st_hdr.si_error_code,ptr_st_spdoe_reqres->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

      if(i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
        return -1;
      }

      strcpy(c_xchng_rmrks,c_xchng_err_msg);

			for ( i_cnt=0; i_cnt<=1; i_cnt++ )
			{
				st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_2LN_ORD_RJCT;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
				}
			}
			break;

		case	THRL_ORDER_CONFIRMATION :			/***	1.6	***/

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside THRL_ORDER_CONFIRMATION Case");
			}

			for ( i_cnt=0; i_cnt<=2; i_cnt++ )
			{
				st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_3LN_ORD_ACPT;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
				}
			}
			break;

		case	THRL_ORDER_ERROR :


			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside THRL_ORDER_ERROR Case");
				fn_userlog(c_ServiceName,"The ERROR CODE FOR ORDER RJCT IS :%d:",ptr_st_spdoe_reqres->st_hdr.si_error_code);
			}

      i_ch_val = fn_get_errmsg( ptr_st_spdoe_reqres->st_hdr.si_error_code,ptr_st_spdoe_reqres->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

      if(i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
        return -1;
      }

      strcpy(c_xchng_rmrks,c_xchng_err_msg);

			for ( i_cnt=0; i_cnt<=2; i_cnt++ )			/***	Ver 1.6	***/
			{
				st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_3LN_ORD_RJCT;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
				}
			}
			break;

    /***** Ver 5.7 Starts Here ****/
    case SP_ORDER_MOD_CON_OUT:
    if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Inside SP_ORDER_MOD_CON_OUT Case");
      }

      for ( i_cnt=0; i_cnt<=1; i_cnt++ )
      {
        st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_SPRD_MOD_ACPT;

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
        }
      }
      break;

    case  SP_ORDER_MOD_REJ_OUT:

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Inside SP_ORDER_MOD_REJ_OUT Case");
        fn_userlog(c_ServiceName,"The ERROR CODE FOR ORDER RJCT IS :%d:",ptr_st_spdoe_reqres->st_hdr.si_error_code);
      }

      i_ch_val = fn_get_errmsg( ptr_st_spdoe_reqres->st_hdr.si_error_code,ptr_st_spdoe_reqres->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

      if(i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
        return -1;
      }

      strcpy(c_xchng_rmrks,c_xchng_err_msg);

      for ( i_cnt=0; i_cnt<=1; i_cnt++ )
      {
        st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_SPRD_MOD_RJCT;

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
        }
      }
      break;
     case  SP_ORDER_CANCEL_REJ_OUT:

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Inside SP_ORDER_CANCEL_REJ_OUT Case");
        fn_userlog(c_ServiceName,"The ERROR CODE FOR ORDER RJCT IS :%d:",ptr_st_spdoe_reqres->st_hdr.si_error_code);
      }

      i_ch_val = fn_get_errmsg( ptr_st_spdoe_reqres->st_hdr.si_error_code,ptr_st_spdoe_reqres->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

      if(i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
        return -1;
      }

      strcpy(c_xchng_rmrks,c_xchng_err_msg);
      for ( i_cnt=0; i_cnt<=1; i_cnt++ )
      {
        st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_SPRD_CAN_RJCT;

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
        }
      }
      break;

      case  SP_ORDER_CXL_CONFIRM_OUT:

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Inside SP_ORDER_CXL_CONFIRM_OUT Case");
        fn_userlog(c_ServiceName,"The ERROR CODE FOR ORDER RJCT IS :%d:",ptr_st_spdoe_reqres->st_hdr.si_error_code);
      }

      i_ch_val = fn_get_errmsg( ptr_st_spdoe_reqres->st_hdr.si_error_code,ptr_st_spdoe_reqres->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

      if(i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
        return -1;
      }

      strcpy(c_xchng_rmrks,c_xchng_err_msg);
      for ( i_cnt=0; i_cnt<=1; i_cnt++ )
      {
        st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_SPRD_CAN_ACPT;

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
        }
      }
      break;
    /*** Ver 5.7 Ends Here *****/
		default :
      fn_userlog( c_ServiceName, "Protocol error %d", ptr_st_spdoe_reqres->st_hdr.si_transaction_code);
			break;
	}

	if (st_spd_ordbk.l_ors_msg_typ[0] == ORS_3LN_ORD_RJCT || st_spd_ordbk.l_ors_msg_typ[0] == ORS_3LN_ORD_ACPT)
	{
		 i_rec_cnt = 3;

		 if(DEBUG_MSG_LVL_3)
		 {
			 fn_userlog(c_ServiceName,"For ORS_3LN_ORD_RJCTORS_3LN_ORD_ACPT Count Is :%d:",i_rec_cnt);
		 }
	}
	else
	{
		i_rec_cnt = 2;

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Count Is :%d:",i_rec_cnt);
		}
	}

	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"L31180", LIBMSG, c_err_msg);
    return -1;
  }

  for ( i_cnt=0; i_cnt<i_rec_cnt; i_cnt++ )
	{
		strcpy ( st_i_ordrbk.c_ordr_rfrnc, st_spd_ordbk.c_ordr_rfrnc[i_cnt] );

	/*fn_cpy_ddr(st_i_ordrbk.c_rout_crt);*/				/*** Ver 1.1 ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		}

		strcpy(st_i_ordrbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
		st_i_ordrbk.c_oprn_typ = FOR_VIEW;

		i_ch_val = fn_call_svc ( c_ServiceName,
                             c_err_msg,
                             &st_i_ordrbk,
                             &st_i_ordrbk,
                             "vw_orderbook",
                             "vw_orderbook",
                             sizeof ( st_i_ordrbk ),
                             sizeof ( st_i_ordrbk ),
                             0,
                             "SFO_REF_TO_ORD" );
   	if ( i_ch_val != SUCC_BFR )
   	{
	 	  fn_errlog(c_ServiceName,"L31185", LIBMSG, c_err_msg);
   		fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    	return -1;
    }

		fn_gettime();

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"ORS2-RD After call To SFO_REF_TO_ORD");
		}

   	strcpy ( st_i_xchngbk.c_ordr_rfrnc, st_i_ordrbk.c_ordr_rfrnc );

	 	st_i_xchngbk.l_mdfctn_cntr = st_i_ordrbk.l_mdfctn_cntr;
   	strcpy ( st_i_xchngbk.c_xchng_cd, st_i_ordrbk.c_xchng_cd );
   	strcpy ( c_mtch_accnt, st_i_ordrbk.c_cln_mtch_accnt );  
   	strcpy ( st_i_xchngbk.c_pipe_id, st_i_ordrbk.c_pipe_id );

	 	switch ( st_spd_ordbk.l_ors_msg_typ[0])
	 	{
	  	case ORS_SPN_ORD_ACPT :
      case ORS_2LN_ORD_ACPT :
      case ORS_3LN_ORD_ACPT :
      case ORS_SPRD_MOD_ACPT:  /** Ver 5.7 ***/
      case ORS_SPRD_CAN_ACPT:  /** Ver 5.7 ***/

        st_i_xchngbk.c_plcd_stts = ACCEPT;

        break;

      case ORS_SPN_ORD_RJCT :
      case ORS_2LN_ORD_RJCT :
      case ORS_3LN_ORD_RJCT :
      case ORS_SPRD_MOD_RJCT:  /** Ver 5.7 ***/
      case ORS_SPRD_CAN_RJCT:  /** Ver 5.7 ***/

        st_i_xchngbk.c_plcd_stts = REJECT;

        break;
  	}			

		st_spd_xchbk.l_dwnld_flg[i_cnt] = i_dwnld_flg;
		st_i_xchngbk.c_rms_prcsd_flg = NOT_PROCESSED;

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Spread Book ORS Msg Type :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
		}

  	st_i_xchngbk.l_ors_msg_typ = st_spd_ordbk.l_ors_msg_typ[i_cnt];
		fn_long_to_timearr (st_spd_ordbk.c_ack_tm[i_cnt] , ptr_st_spdoe_reqres->li_entry_date_time );
		strcpy(st_i_xchngbk.c_ack_tm,st_spd_ordbk.c_ack_tm[i_cnt]);
		fn_long_to_timearr(st_i_xchngbk.c_entry_dt_tm, ptr_st_spdoe_reqres->li_entry_date_time );

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Exchange Book Entry Date Time :%s:",st_i_xchngbk.c_entry_dt_tm);
		}

		strcpy ( st_i_xchngbk.c_xchng_rmrks ,c_xchng_rmrks);
		st_i_xchngbk.d_jiffy = (double) ll_tmp_log_tm_stmp + i_cnt;
		st_i_xchngbk.l_dwnld_flg = st_spd_xchbk.l_dwnld_flg[i_cnt];
		fn_long_to_timearr (st_spd_xchbk.c_entry_dt_tm[i_cnt],ptr_st_spdoe_reqres->li_entry_date_time );

    /*** Ver 7.0 Starts ***/

    if( ptr_st_spdoe_reqres->st_hdr.si_transaction_code  == SP_ORDER_CXL_CONFIRM_OUT )
    {
      if( i_cnt == 0)
      {
        st_i_xchngbk.l_xchng_can_qty = ptr_st_spdoe_reqres->li_volume_filled_today;
      }
      else
      {
        st_i_xchngbk.l_xchng_can_qty = ptr_st_spdoe_reqres->st_spd_lg_inf[i_cnt - 1].li_volume_filled_today;
      }
    }
    /*** Ver 7.0 Ends ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Modification Counter is :%ld:",st_i_xchngbk.l_mdfctn_cntr);
			fn_userlog(c_ServiceName,"RMS Processed Flag Is :%c:",st_i_xchngbk.c_rms_prcsd_flg);
			fn_userlog(c_ServiceName,"ORS Message Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
			fn_userlog(c_ServiceName,"Remarks :%s:",st_i_xchngbk.c_xchng_rmrks);
			fn_userlog(c_ServiceName,"Jiffy :%lf:",st_i_xchngbk.d_jiffy);
			fn_userlog(c_ServiceName,"Placed Status Is :%c:",st_i_xchngbk.c_plcd_stts);
			fn_userlog(c_ServiceName,"Order Reference Is :%s:",st_i_xchngbk.c_ordr_rfrnc);
		}

  	/*fn_cpy_ddr(st_i_xchngbk.c_rout_crt);*/		/*** Ver 1.1 ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		}

		strcpy(st_i_xchngbk.c_rout_crt,c_rout_str); /*** Ver 1.1 ***/
 	 	st_i_xchngbk.c_oprn_typ = UPDATION_ON_EXCHANGE_RESPONSE;

		/**Ver 1.7 starts **/
		li_temp_bit_flg = st_i_xchngbk.l_bit_flg;
		st_i_xchngbk.l_bit_flg	=	ptr_st_spdoe_reqres->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1];

		sprintf(c_lst_act_ref,"%lld",ptr_st_spdoe_reqres->ll_lastactivityref);   /** Ver 6.1 **/
    rtrim(c_lst_act_ref);   /*** Ver 6.1 ***/
		
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"IN SPD_ACPT_RJCT, the stream number receieved from exchange is:%ld:",st_i_xchngbk.l_bit_flg);
			fn_userlog(c_ServiceName,"Last Activity Ref is :%s:",c_lst_act_ref);      /*** Ver 6.1 ***/
		}
		/**Ver 1.7 ends **/

/***		i_ch_val = fn_call_svc ( c_ServiceName,		*** Commented in Ver 6.1 ***
                      	     c_err_msg,
                    	       &st_i_xchngbk,
                  	         &st_i_xchngbk,
                	           "vw_xchngbook",
              	             "vw_xchngbook",
            	               sizeof ( st_i_xchngbk ),
          	                 sizeof ( st_i_xchngbk ),
        	                   0,
      	                     "SFO_UPD_XCHNGBK" );		*** Ver 6.1 ***/

    /*** Ver 7.0 Starts ***/
    if(DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName,"fn_upd_xchngbk_erl:Order Reference Is :%s:",st_i_xchngbk.c_ordr_rfrnc);
      fn_userlog(c_ServiceName,"st_i_xchngbk.l_xchng_can_qty:%ld:",st_i_xchngbk.l_xchng_can_qty);
    }

    if ( st_i_xchngbk.l_dwnld_flg == DOWNLOAD )
    {
      EXEC SQL
        SELECT 1
        INTO :i_rec_exists
        FROM FXB_FO_XCHNG_BOOK
        WHERE FXB_JIFFY   = :st_i_xchngbk.d_jiffy
        AND FXB_XCHNG_CD  = :st_i_xchngbk.c_xchng_cd
        AND FXB_PIPE_ID   = :st_i_xchngbk.c_pipe_id
        AND FXB_ORDR_RFRNC    = :st_i_xchngbk.c_ordr_rfrnc;

      if ( ( SQLCODE != 0 ) && (SQLCODE != NO_DATA_FOUND) )
      {
        fn_errlog(c_ServiceName,"L31190", LIBMSG, c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
        return -1;
      }
      if ( i_rec_exists == 1 )
      {
        fn_userlog ( c_ServiceName, "Record already processed for order confirmation" );
        strcpy( c_err_msg, "Record already Processed" );
        fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
        return 0;
      }
    }

    if(DEBUG_MSG_LVL_3)
    {
      fn_userlog(c_ServiceName,"st_i_xchngbk.c_xchng_rmrks :%s:",st_i_xchngbk.c_xchng_rmrks);
    }

    if (st_i_xchngbk.l_ors_msg_typ == ORS_SPRD_CAN_ACPT)
    {
      EXEC SQL
        UPDATE  FXB_FO_XCHNG_BOOK
        SET  FXB_PLCD_STTS       = :st_i_xchngbk.c_plcd_stts,
           FXB_RMS_PRCSD_FLG   = :st_i_xchngbk.c_rms_prcsd_flg,
           FXB_ORS_MSG_TYP     = :st_i_xchngbk.l_ors_msg_typ,
           FXB_ACK_TM          = to_date( :st_i_xchngbk.c_ack_tm,'DD-Mon-yyyy hh24:mi:ss' ),
           FXB_XCHNG_RMRKS     = rtrim(fxb_xchng_rmrks)||:st_i_xchngbk.c_xchng_rmrks,
           FXB_JIFFY           = :st_i_xchngbk.d_jiffy,
           FXB_STREAM_NO       = :st_i_xchngbk.l_bit_flg,
           FXB_XCHNG_CNCLD_QTY = :st_i_xchngbk.l_xchng_can_qty,
           FXB_LST_ACT_REF     = :c_lst_act_ref
        WHERE  FXB_ORDR_RFRNC    = :st_i_xchngbk.c_ordr_rfrnc
        AND  FXB_MDFCTN_CNTR     = :st_i_xchngbk.l_mdfctn_cntr;
    }
    else
    {
      EXEC SQL
        UPDATE  FXB_FO_XCHNG_BOOK
        SET  FXB_PLCD_STTS       = :st_i_xchngbk.c_plcd_stts,
           FXB_RMS_PRCSD_FLG   = :st_i_xchngbk.c_rms_prcsd_flg,
           FXB_ORS_MSG_TYP     = :st_i_xchngbk.l_ors_msg_typ,
           FXB_ACK_TM          = to_date( :st_i_xchngbk.c_ack_tm,'DD-Mon-yyyy hh24:mi:ss' ),
           FXB_XCHNG_RMRKS     = rtrim(fxb_xchng_rmrks)||:st_i_xchngbk.c_xchng_rmrks,
           FXB_JIFFY           = :st_i_xchngbk.d_jiffy,
           FXB_STREAM_NO       = :st_i_xchngbk.l_bit_flg,
           FXB_LST_ACT_REF     = :c_lst_act_ref
        WHERE  FXB_ORDR_RFRNC    = :st_i_xchngbk.c_ordr_rfrnc
        AND  FXB_MDFCTN_CNTR     = :st_i_xchngbk.l_mdfctn_cntr;
    }
    if(SQLCODE != 0)
    {
      fn_errlog(c_ServiceName,"L31195", SQLMSG, c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return -1;
    }

    /*** Ver 7.0 Ends ***/

		/*i_ch_val = fn_upd_xchngbk_erl(&st_i_xchngbk,c_lst_act_ref,c_ServiceName,c_err_msg);   *** Added in ver 6.1 ***

  	if ( ( i_ch_val != SUCC_BFR ) &&
    	   ( i_ch_val != RECORD_ALREADY_PRCSD ) )
  	{
  		fn_errlog(c_ServiceName,"L31200", LIBMSG, c_err_msg);
    	fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    	return -1;
  	}
		else if ( i_ch_val == RECORD_ALREADY_PRCSD )
		{
			fn_userlog ( c_ServiceName, "Record already processed" );
    	fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    	return 0;
  	}
    *** commented in ver 7.0 ***/

		st_i_xchngbk.l_bit_flg = li_temp_bit_flg ; 			/**Ver 1.7 **/

		fn_gettime();

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_XCHNGBK :%s:",c_time);
		}
	
		sprintf( st_i_ordrbk.c_xchng_ack ,"%16.0lf",ptr_st_spdoe_reqres->d_order_number);

		if(DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName,"Spread Exchange Ack Is :%s:",st_spd_ordbk.c_xchng_ack[i_cnt]);
  		/** fn_userlog(c_ServiceName,"Order Book Exchng Ack Is :%s:",st_i_ordrbk.c_xchng_ack); **ver 4.9 **/
		}

    strcpy ( st_i_ordrbk.c_ordr_rfrnc,st_spd_ordbk.c_ordr_rfrnc[i_cnt] );

		if(DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName,"Order Book Ref. :%s:",st_i_ordrbk.c_ordr_rfrnc);
		}

  	/*fn_cpy_ddr(st_i_ordrbk.c_rout_crt);*/		  /*** Ver 1.1 ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		}
		strcpy(st_i_ordrbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
  	st_i_ordrbk.c_oprn_typ = UPDATE_XCHNG_RESPONSE;

		i_ch_val = fn_call_svc ( c_ServiceName,
    	                       c_err_msg,
      	                     &st_i_ordrbk,
        	                   &st_i_ordrbk,
          	                 "vw_orderbook",
            	               "vw_orderbook",
              	             sizeof ( st_i_ordrbk ),
                	           sizeof ( st_i_ordrbk ),
                  	         0,
                    	       "SFO_UPD_ORDRBK" );
  	if ( i_ch_val != SUCC_BFR)
  	{
  		fn_errlog(c_ServiceName,"L31205", LIBMSG, c_err_msg);
    	fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    	return -1;
  	}

		fn_gettime();

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_ORDRBK :%s:",c_time);
		}

		strcpy ( st_spd_xchbk.c_xchng_cd[i_cnt], st_i_xchngbk.c_xchng_cd );
  	strcpy ( st_spd_xchbk.c_ordr_rfrnc[i_cnt], st_i_xchngbk.c_ordr_rfrnc );
		strcpy ( st_spd_xchbk.c_pipe_id[i_cnt], st_i_xchngbk.c_pipe_id );
  	strcpy ( st_spd_xchbk.c_mod_trd_dt[i_cnt], st_i_xchngbk.c_mod_trd_dt );
  	st_spd_xchbk.l_ord_seq[i_cnt] = st_i_xchngbk.l_ord_seq;
  	st_spd_xchbk.c_slm_flg[i_cnt] = st_i_xchngbk.c_slm_flg;
  	st_spd_xchbk.l_dsclsd_qty[i_cnt] = st_i_xchngbk.l_dsclsd_qty;
  	st_spd_xchbk.l_ord_tot_qty[i_cnt] = st_i_xchngbk.l_ord_tot_qty;
  	st_spd_xchbk.l_ord_lmt_rt[i_cnt] = st_i_xchngbk.l_ord_lmt_rt;
  	st_spd_xchbk.l_stp_lss_tgr[i_cnt] = st_i_xchngbk.l_stp_lss_tgr;
  	st_spd_xchbk.l_mdfctn_cntr[i_cnt] = st_i_xchngbk.l_mdfctn_cntr;
  	strcpy ( st_spd_xchbk.c_valid_dt[i_cnt], st_i_xchngbk.c_valid_dt );
  	st_spd_xchbk.c_ord_typ[i_cnt] = st_i_xchngbk.c_ord_typ;
  	st_spd_xchbk.c_sprd_ord_ind[i_cnt] = st_i_xchngbk.c_sprd_ord_ind;
  	st_spd_xchbk.c_req_typ[i_cnt] = st_i_xchngbk.c_req_typ;
  	st_spd_xchbk.l_quote[i_cnt] = st_i_xchngbk.l_quote;
		strcpy ( st_spd_xchbk.c_qt_tm[i_cnt], st_i_xchngbk.c_qt_tm );
  	strcpy ( st_spd_xchbk.c_rqst_tm[i_cnt], st_i_xchngbk.c_rqst_tm );
  	strcpy ( st_spd_xchbk.c_frwrd_tm[i_cnt], st_i_xchngbk.c_frwrd_tm );
  	st_spd_xchbk.c_plcd_stts[i_cnt] = st_i_xchngbk.c_plcd_stts;
  	st_spd_xchbk.c_rms_prcsd_flg[i_cnt] = st_i_xchngbk.c_rms_prcsd_flg;
  	strcpy ( st_spd_xchbk.c_ack_tm[i_cnt], st_i_xchngbk.c_ack_tm );
  	strcpy ( st_spd_xchbk.c_xchng_rmrks[i_cnt], st_i_xchngbk.c_xchng_rmrks );
  	st_spd_xchbk.c_ex_ordr_typ[i_cnt] = st_i_xchngbk.c_ex_ordr_typ;
    if( ptr_st_spdoe_reqres->st_hdr.si_transaction_code  == SP_ORDER_CXL_CONFIRM_OUT )
    {
        if( i_cnt == 0)
        {
             st_spd_xchbk.l_xchng_can_qty[i_cnt] = ptr_st_spdoe_reqres->li_volume - ptr_st_spdoe_reqres->li_total_vol_remaining;
        }
        else
        {
        st_spd_xchbk.l_xchng_can_qty[i_cnt] = ptr_st_spdoe_reqres->st_spd_lg_inf[i_cnt - 1].li_volume - ptr_st_spdoe_reqres->st_spd_lg_inf[i_cnt - 1].li_total_vol_remaining; /****** suchita *******/
        }
        fn_userlog(c_ServiceName,"CANCELLATION RWSst_spd_xchbk.l_xchng_can_qty :%ld: and i_cnt :%d:",st_spd_xchbk.l_xchng_can_qty[i_cnt],i_cnt); 
    }
    else
    {
  	st_spd_xchbk.l_xchng_can_qty[i_cnt] = st_i_xchngbk.l_xchng_can_qty;
    }
  	st_spd_xchbk.l_bit_flg[i_cnt] = st_i_xchngbk.l_bit_flg;
  	st_spd_xchbk.c_spl_flg[i_cnt] = st_i_xchngbk.c_spl_flg;
  	st_spd_xchbk.c_ordr_flw[i_cnt] = st_i_xchngbk.c_ordr_flw;
  	st_spd_xchbk.c_oprn_typ[i_cnt] = st_i_xchngbk.c_oprn_typ;
 	  st_spd_xchbk.l_ors_msg_typ[i_cnt] = st_i_xchngbk.l_ors_msg_typ;
 		st_spd_xchbk.d_jiffy[i_cnt] = st_i_xchngbk.d_jiffy;
  	st_spd_xchbk.l_dwnld_flg[i_cnt] = st_i_xchngbk.l_dwnld_flg;
  	strcpy ( st_spd_xchbk.c_entry_dt_tm[i_cnt], st_i_xchngbk.c_entry_dt_tm );

	}
	
	/*fn_cpy_ddr(st_spd_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_spd_ordbk.c_rout_crt,c_rout_str); /*** Ver 1.1 ***/

  st_spd_ordbk.c_rqst_typ[0] = UPDATE_XCHNG_RESPONSE;
	
	i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_spd_ordbk,
                           &st_spd_ordbk,
                           "vw_spdordbk",
                           "vw_spdordbk",
                           sizeof ( st_spd_ordbk ),
                           sizeof ( st_spd_ordbk ),
                           0,
                           "SFO_UPD_SPDBK" );
  if ( i_ch_val != SUCC_BFR)
  {
    fn_errlog(c_ServiceName,"L31210", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }
	
	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_SPDBK :%s:",c_time);
	}

	/*fn_cpy_ddr(st_spd_xchbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_spd_xchbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

	i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"L31215", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Commit :%s:",c_time);
	}

	i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_err_msg,
                            &st_spd_xchbk,
                            "vw_spdxchbk",
                            sizeof ( st_spd_xchbk ),
                            TPNOREPLY,
                            "SFO_SPD_ACK" );
  if ( i_ch_val != 0 )
  {
    fn_errlog(c_ServiceName,"L31220", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_SPD_ACK :%s:",c_time);
	}

	return 0;
}

/******************************************************************************/
/*  This handels Spread Order Cancellation Response.													*/
/*  INPUT PARAMETERS                                                          */
/*      ptr_st_spdoe_reqres- Pointer to a message structure got from Exchange */
/*      c_pipe_id      - Pipe Id                                              */
/*      i_dwnld_flg    - Download Flag                                        */
/*      c_ServiceName  - Service Name currently running                       */
/*      c_err_msg      - Error message to be returned                         */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/

int fn_spd_ord_cncl ( struct st_spd_oe_reqres *ptr_st_spdoe_reqres,
											char   *c_xchng_cd,
                      char   *c_pipe_id,
										  char   *c_rout_str,
											char   *c_trd_dt,
                      int    i_dwnld_flg,
                      char   *c_ServiceName,
                      char   *c_err_msg )
{
	struct vw_xchngbook st_i_xchngbk;
  struct vw_orderbook st_i_ordrbk;
  struct vw_exrcbook st_i_exrcbk;
  struct vw_spdordbk st_spd_ordbk;
  struct vw_spdxchbk st_spd_xchbk;

  char c_svc_name [15+1];
  char c_refack_tmp[LEN_ORD_REFACK];
	char c_xchng_rmrks [256];
	char c_mtch_accnt [10 + 1];
  char c_xchng_err_msg [ 256 ];   /*** Ver 5.7 ***/
	char c_lst_act_ref [22];				/*** Ver 6.1 ***/

  int i_ch_val;
  int i_trnsctn;
  int i_cnt;
  int i_rec_cnt;
	
	long int li_exctd_qty;

	long li_temp_bit_flg;							/**Ver 1.7 **/

	long long ll_tmp_log_tm_stmp;

	MEMSET(st_i_xchngbk);
	MEMSET(st_i_ordrbk);
	MEMSET(st_i_exrcbk);
	MEMSET(st_spd_ordbk);
	MEMSET(st_spd_xchbk);
	MEMSET(c_lst_act_ref);					/*** Ver 6.1 ***/

	if(DEBUG_MSG_LVL_3)  /*** Ver 4.9, debug changed from 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside Function fn_spd_ord_cncl");
	}

	memcpy ( &ll_tmp_log_tm_stmp, ptr_st_spdoe_reqres->st_hdr.c_time_stamp_1, 8 );

 	sprintf(st_spd_ordbk.c_xchng_ack[0],"%16.0lf",ptr_st_spdoe_reqres->d_order_number );
  strcpy ( st_spd_ordbk.c_pipe_id[0], c_pipe_id );
  /*fn_cpy_ddr(st_spd_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_spd_ordbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

  st_spd_ordbk.c_rqst_typ[0] = ACK_TO_ORD;

  i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_spd_ordbk,
                           &st_spd_ordbk,
                           "vw_spdordbk",
                           "vw_spdordbk",
                           sizeof ( st_spd_ordbk ),
                           sizeof ( st_spd_ordbk ),
                           0,
                           "SFO_QRY_SPDBK" );

  if ( i_ch_val != SUCC_BFR )
  {
    fn_errlog(c_ServiceName,"L31225", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_QRY_SPDBK For ACK_TO_ORD :%s:",c_time);
	}

	switch (ptr_st_spdoe_reqres->st_hdr.si_transaction_code)
	{
		case SP_ORDER_CXL_CONFIRM_OUT:

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside SP_ORDER_CXL_CONFIRM_OUT Case");
			}

			for(i_cnt=0; i_cnt<=1; i_cnt++ )
			{
				st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_SPD_ORD_CNCL;

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
				}

				strcpy(st_spd_xchbk.c_xchng_rmrks[i_cnt],"Spread order cancelled" );
				i_rec_cnt = 2;
			}
			break;

		case TWOL_ORDER_CXL_CONFIRMATION:

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside TWOL_ORDER_CXL_CONFIRMATION Case");
			}

			for( i_cnt=0; i_cnt<=1; i_cnt++ )
			{
				st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_2L_ORD_CNCL;

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
				}
				strcpy(st_spd_xchbk.c_xchng_rmrks[i_cnt],"2L order cancelled" );
				i_rec_cnt = 2;
			}

			break;
	
		case THRL_ORDER_CXL_CONFIRMATION:
		
			for ( i_cnt=0; i_cnt<=2; i_cnt++ )
			{
				st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_3L_ORD_CNCL;

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
				}

				strcpy(st_spd_xchbk.c_xchng_rmrks[i_cnt],"3L order cancelled" );
				i_rec_cnt = 3;
			}
			break;
     /*** Ver 5.7 Starts here ****/
     case SP_ORDER_CANCEL_REJ_OUT:

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Inside SP_ORDER_CANCEL_REJ_OUT Case");
      }

      for(i_cnt=0; i_cnt<=1; i_cnt++ )
      {
        st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_SPRD_CAN_RJCT;

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Msg Type Is :%ld:",st_spd_ordbk.l_ors_msg_typ[i_cnt]);
        }

        i_ch_val = fn_get_errmsg( ptr_st_spdoe_reqres->st_hdr.si_error_code,ptr_st_spdoe_reqres->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);
      if(i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
        return -1;
      }

      strcpy(st_spd_xchbk.c_xchng_rmrks[i_cnt],c_xchng_err_msg);
        i_rec_cnt = 2;
      }
      break;
      case BATCH_SPREAD_CXL_OUT:
      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Inside BATCH_SPREAD_CXL_OUT Case");
        fn_userlog(c_ServiceName, "Exchange initiated expiry");
      }

      for(i_cnt=0; i_cnt<=1; i_cnt++ )
      {
        st_spd_ordbk.l_ors_msg_typ[i_cnt] = ORS_ORD_EXP;

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_st_spdoe_reqres->li_last_modified);
        }

        fn_long_to_timearr (st_spd_xchbk.c_ack_tm[i_cnt],ptr_st_spdoe_reqres->li_last_modified);

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_spd_xchbk.c_ack_tm[i_cnt]);
        }

        strcpy(c_xchng_rmrks ,"Exchange Expiry");

        strcpy(st_spd_xchbk.c_xchng_rmrks[i_cnt],"Exchange Expiry");
        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",c_xchng_rmrks);
        }
        i_rec_cnt = 2;
      }
      break;

    default :
      fn_userlog( c_ServiceName, "Protocol error %d", ptr_st_spdoe_reqres->st_hdr.si_transaction_code);
      break;
      /**** Ver 5.7 Ends here ***/
	}


  i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"L31230", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Begin Transaction :%s:",c_time);
	}

	for ( i_cnt=0; i_cnt<i_rec_cnt; i_cnt++ )
	{
		strcpy ( st_i_ordrbk.c_ordr_rfrnc, st_spd_ordbk.c_ordr_rfrnc[i_cnt] );
		
		if(DEBUG_MSG_LVL_0)
		{
			/** fn_userlog(c_ServiceName,"Order Book Order Reference Is :%s:",st_i_ordrbk.c_ordr_rfrnc); *** ver 4.9 **/
      fn_userlog(c_ServiceName,"fn_spd_ord_cncl:Order Book Order Reference Is :%s:",st_i_ordrbk.c_ordr_rfrnc); /** ver 4.9**/
		}

		/*fn_cpy_ddr(st_i_ordrbk.c_rout_crt);*/			/*** Ver 1.1 ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		}
		strcpy(st_i_ordrbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
		st_i_ordrbk.c_oprn_typ = FOR_VIEW;

		i_ch_val = fn_call_svc ( c_ServiceName,
                             c_err_msg,
                             &st_i_ordrbk,
                             &st_i_ordrbk,
                             "vw_orderbook",
                             "vw_orderbook",
                             sizeof ( st_i_ordrbk ),
                             sizeof ( st_i_ordrbk ),
                             0,
                             "SFO_REF_TO_ORD" );

		if ( i_ch_val != SUCC_BFR)
    {
      fn_errlog(c_ServiceName,"L31235", LIBMSG, c_err_msg);
      fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
      return -1;
    }

		fn_gettime();

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_REF_TO_ORD :%s:",c_time);
		}

		strcpy ( c_mtch_accnt, st_i_ordrbk.c_cln_mtch_accnt);   
		strcpy ( st_i_xchngbk.c_ordr_rfrnc , st_i_ordrbk.c_ordr_rfrnc );
    strcpy ( st_i_xchngbk.c_xchng_cd , st_i_ordrbk.c_xchng_cd );
    strcpy ( st_i_xchngbk.c_pipe_id , st_i_ordrbk.c_pipe_id );
    strcpy ( st_i_xchngbk.c_mod_trd_dt , ( char * )c_trd_dt );
    st_i_xchngbk.l_ord_tot_qty = st_i_ordrbk.l_ord_tot_qty;
    st_i_xchngbk.l_ord_lmt_rt = st_i_ordrbk.l_ord_lmt_rt;
    st_i_xchngbk.l_ord_seq = 0;
    st_i_xchngbk.l_mdfctn_cntr = EXPIRY_MDFCTN_CNTR;
    /** st_i_xchngbk.c_plcd_stts = CANCELLED; *** Ver 5.7 ***/ 
    st_i_xchngbk.c_rms_prcsd_flg = NOT_PROCESSED;

    /*** Ver 5.7 starts here ***/
    if ( st_spd_ordbk.l_ors_msg_typ[0] == ORS_SPRD_CAN_RJCT )
    {
       st_i_xchngbk.c_plcd_stts = REJECT;
    }
    else if( st_spd_ordbk.l_ors_msg_typ[0] == ORS_ORD_EXP )
    {
      st_i_xchngbk.c_plcd_stts = EXPIRED; 
    }
    else
    {
       st_i_xchngbk.c_plcd_stts = CANCELLED;
    }
    /*** ver 5.7 ends here ***/

		fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_st_spdoe_reqres->li_entry_date_time );

		st_i_xchngbk.l_ors_msg_typ = st_spd_ordbk.l_ors_msg_typ[i_cnt];
		strcpy ( st_i_xchngbk.c_xchng_rmrks ,st_spd_xchbk.c_xchng_rmrks[i_cnt]);
		st_i_xchngbk.d_jiffy = (double)ll_tmp_log_tm_stmp + i_cnt ;			/***	Ver 1.6	***/
		st_spd_xchbk.l_dwnld_flg[i_cnt] = i_dwnld_flg;
		st_i_xchngbk.l_dwnld_flg = st_spd_xchbk.l_dwnld_flg[i_cnt];

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Match Account Is :%s:",c_mtch_accnt);
			fn_userlog(c_ServiceName,"Exchange Book Order Reference Is :%s:",st_i_xchngbk.c_ordr_rfrnc);
			fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_i_xchngbk.c_xchng_cd);
			fn_userlog(c_ServiceName,"Pipe ID Is :%s:",st_i_xchngbk.c_pipe_id);
			fn_userlog(c_ServiceName,"Mod Trade Date Is :%s:",st_i_xchngbk.c_mod_trd_dt);
			fn_userlog(c_ServiceName,"Total Order Qty Is :%ld:",st_i_xchngbk.l_ord_tot_qty);
			fn_userlog(c_ServiceName,"Order Limit Rate Is :%ld:",st_i_xchngbk.l_ord_lmt_rt);
			fn_userlog(c_ServiceName,"Order Seq Is :%ld:",st_i_xchngbk.l_ord_seq);
			fn_userlog(c_ServiceName,"Modification Counter Is :%ld:",st_i_xchngbk.l_mdfctn_cntr);
			fn_userlog(c_ServiceName,"Placed Status Is :%c:",st_i_xchngbk.c_plcd_stts);
		}
		
		/**Ver 1.7 starts **/
		li_temp_bit_flg = st_i_xchngbk.l_bit_flg;
		st_i_xchngbk.l_bit_flg	=	ptr_st_spdoe_reqres->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1];

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"IN SPD_CNCL, the stream number receieved from exchange is:%ld:",st_i_xchngbk.l_bit_flg);
		}
		/**Ver 1.7 ends **/

		sprintf(c_lst_act_ref,"%lld",ptr_st_spdoe_reqres->ll_lastactivityref);   /** Ver 6.1 **/
    rtrim(c_lst_act_ref);   /*** Ver 6.1 ***/

		if(DEBUG_MSG_LVL_3)			/*** Ver 6.1 ***/
		{
			fn_userlog(c_ServiceName,"Last Activity Ref is :%s:",c_lst_act_ref);      /*** Ver 6.1 ***/
		}

	  st_i_xchngbk.c_oprn_typ = INSERTION_ON_ORDER_FTE;

/***		i_ch_val = fn_call_svc ( c_ServiceName,		*** Commented in Ver 6.1 ***
                             c_err_msg,
                             &st_i_xchngbk,
                             &st_i_xchngbk,
                             "vw_xchngbook",
                             "vw_xchngbook",
                             sizeof ( st_i_xchngbk ),
                             sizeof ( st_i_xchngbk ),
                             0,
                             "SFO_UPD_XCHNGBK" );	*** Ver 6.1 ***/

		i_ch_val = fn_upd_xchngbk_erl(&st_i_xchngbk,c_lst_act_ref,c_ServiceName,c_err_msg);   /*** Added in ver 6.1 ***/

    if ( ( i_ch_val != SUCC_BFR ) &&
         ( i_ch_val != RECORD_ALREADY_PRCSD ) )
    {
      fn_errlog(c_ServiceName,"L31240", LIBMSG, c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return -1;
    }
		else if ( i_ch_val == RECORD_ALREADY_PRCSD )
		{
			fn_userlog ( c_ServiceName, "Record already processed" );
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return 0;
    }

		st_i_xchngbk.l_bit_flg = li_temp_bit_flg ; 					/**Ver 1.7 **/

		fn_gettime();

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog ( c_ServiceName,"ORS2-RD After Call To SFO_UPD_XCHNGBK :%s:",c_time);
		}

		strcpy ( st_spd_xchbk.c_xchng_cd[i_cnt], st_i_xchngbk.c_xchng_cd );
    strcpy ( st_spd_xchbk.c_ordr_rfrnc[i_cnt], st_i_xchngbk.c_ordr_rfrnc );
    strcpy ( st_spd_xchbk.c_pipe_id[i_cnt], st_i_xchngbk.c_pipe_id );
    strcpy ( st_spd_xchbk.c_mod_trd_dt[i_cnt], st_i_xchngbk.c_mod_trd_dt );
    st_spd_xchbk.l_ord_seq[i_cnt] = st_i_xchngbk.l_ord_seq;
    st_spd_xchbk.c_slm_flg[i_cnt] = st_i_xchngbk.c_slm_flg;
    st_spd_xchbk.l_dsclsd_qty[i_cnt] = st_i_xchngbk.l_dsclsd_qty;
    st_spd_xchbk.l_ord_tot_qty[i_cnt] = st_i_xchngbk.l_ord_tot_qty;
    st_spd_xchbk.l_ord_lmt_rt[i_cnt] = st_i_xchngbk.l_ord_lmt_rt;
    st_spd_xchbk.l_stp_lss_tgr[i_cnt] = st_i_xchngbk.l_stp_lss_tgr;
    st_spd_xchbk.l_mdfctn_cntr[i_cnt] = st_i_xchngbk.l_mdfctn_cntr;
    strcpy ( st_spd_xchbk.c_valid_dt[i_cnt], st_i_xchngbk.c_valid_dt );
    st_spd_xchbk.c_ord_typ[i_cnt] = st_i_xchngbk.c_ord_typ;
    st_spd_xchbk.c_sprd_ord_ind[i_cnt] = st_i_xchngbk.c_sprd_ord_ind;
    st_spd_xchbk.c_req_typ[i_cnt] = st_i_xchngbk.c_req_typ;
    st_spd_xchbk.l_quote[i_cnt] = st_i_xchngbk.l_quote;
    strcpy ( st_spd_xchbk.c_qt_tm[i_cnt], st_i_xchngbk.c_qt_tm );
    strcpy ( st_spd_xchbk.c_rqst_tm[i_cnt], st_i_xchngbk.c_rqst_tm );
    strcpy ( st_spd_xchbk.c_frwrd_tm[i_cnt], st_i_xchngbk.c_frwrd_tm );
    st_spd_xchbk.c_plcd_stts[i_cnt] = st_i_xchngbk.c_plcd_stts;
    st_spd_xchbk.c_rms_prcsd_flg[i_cnt] = st_i_xchngbk.c_rms_prcsd_flg;
    strcpy ( st_spd_xchbk.c_ack_tm[i_cnt], st_i_xchngbk.c_ack_tm );
    st_spd_xchbk.c_ex_ordr_typ[i_cnt] = st_i_xchngbk.c_ex_ordr_typ;
    st_spd_xchbk.l_bit_flg[i_cnt] = st_i_xchngbk.l_bit_flg;
    st_spd_xchbk.c_spl_flg[i_cnt] = st_i_xchngbk.c_spl_flg;
    st_spd_xchbk.c_ordr_flw[i_cnt] = st_i_xchngbk.c_ordr_flw;
    st_spd_xchbk.c_oprn_typ[i_cnt] = st_i_xchngbk.c_oprn_typ;
    st_spd_xchbk.l_ors_msg_typ[i_cnt] = st_i_xchngbk.l_ors_msg_typ;
    st_spd_xchbk.d_jiffy[i_cnt] = st_i_xchngbk.d_jiffy;
    st_spd_xchbk.l_dwnld_flg[i_cnt] = st_i_xchngbk.l_dwnld_flg;
    strcpy ( st_spd_xchbk.c_entry_dt_tm[i_cnt], st_i_xchngbk.c_entry_dt_tm );


		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Spread Book Exchange Code Is :%s:",st_spd_xchbk.c_xchng_cd[i_cnt]);
			fn_userlog(c_ServiceName,"Spread Book Order Reference Is :%s:",st_spd_xchbk.c_ordr_rfrnc[i_cnt]);
			fn_userlog(c_ServiceName,"Spread Book Pipe Id Is :%s:",st_spd_xchbk.c_pipe_id[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Trade Date Is :%s:",st_spd_xchbk.c_mod_trd_dt[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Order Sequence Is :%ld:",st_spd_xchbk.l_ord_seq[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book LimitMarket Flag :%c:",st_spd_xchbk.c_slm_flg[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Disclosed Qty Is :%ld:",st_spd_xchbk.l_dsclsd_qty[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Total Order Qty Is :%ld:",st_spd_xchbk.l_ord_tot_qty[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Order Limit Rate Is :%ld:",st_spd_xchbk.l_ord_lmt_rt[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Stop Loss Trigger Is :%ld:",st_spd_xchbk.l_stp_lss_tgr[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Modification Counter Is :%ld:",st_spd_xchbk.l_mdfctn_cntr[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Order Type Is :%c:",st_spd_xchbk.c_ord_typ[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Request Type Is :%c:",st_spd_xchbk.c_req_typ[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Quote Time Is :%s:",st_spd_xchbk.c_qt_tm[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Request Time Is :%s:",st_spd_xchbk.c_rqst_tm[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Forward Time Is :%s:",st_spd_xchbk.c_frwrd_tm[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Placed Status Is :%c:",st_spd_xchbk.c_plcd_stts[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book RMS Processed Flag Is :%c:",st_spd_xchbk.c_rms_prcsd_flg[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Ack Time Is :%s:",st_spd_xchbk.c_ack_tm[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Order Type Is :%c:",st_spd_xchbk.c_ex_ordr_typ[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Bit Flag Is :%ld:",st_spd_xchbk.l_bit_flg[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book SPL Flag Is :%c:",st_spd_xchbk.c_spl_flg[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Order Flow Is :%c:",st_spd_xchbk.c_ordr_flw[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Operation Type Is :%c:",st_spd_xchbk.c_oprn_typ[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Msg Type Is :%ld:",st_spd_xchbk.l_ors_msg_typ[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Jiffy Is :%lf:",st_spd_xchbk.d_jiffy[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book DownLoad Flag is :%ld:",st_spd_xchbk.l_dwnld_flg[i_cnt]);
    	fn_userlog(c_ServiceName,"Spread Book Entry Date Time :%s:",st_spd_xchbk.c_entry_dt_tm[i_cnt]);
		}
		if( i_cnt == 0)
		{
			st_spd_xchbk.l_xchng_can_qty[i_cnt] = ptr_st_spdoe_reqres->li_volume - ptr_st_spdoe_reqres->li_total_vol_remaining;
		}
		else
		{
			st_spd_xchbk.l_xchng_can_qty[i_cnt] =
				ptr_st_spdoe_reqres->st_spd_lg_inf[i_cnt - 1].li_volume -  
			  ptr_st_spdoe_reqres->st_spd_lg_inf[i_cnt - 1].li_total_vol_remaining;
		}

		st_spd_ordbk.l_exctd_qty[i_cnt] = st_spd_xchbk.l_xchng_can_qty[i_cnt];
		st_spd_ordbk.l_can_qty[i_cnt] 	= st_spd_ordbk.l_ord_tot_qty[i_cnt] - st_spd_xchbk.l_xchng_can_qty[i_cnt]; 
		
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Executed Qty Is :%ld:",st_spd_ordbk.l_exctd_qty[i_cnt]);
			fn_userlog(c_ServiceName,"Cancel Qty Is :%ld:",st_spd_ordbk.l_can_qty[i_cnt]);
		}
	}

	/*fn_cpy_ddr(st_spd_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}
	strcpy(st_spd_ordbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
  st_spd_ordbk.c_rqst_typ[0] = UPDATE_XCHNG_RESPONSE_RC;

	i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_spd_ordbk,
                           &st_spd_ordbk,
                           "vw_spdordbk",
                           "vw_spdordbk",
                           sizeof ( st_spd_ordbk ),
                           sizeof ( st_spd_ordbk ),
                           0,
                           "SFO_UPD_SPDBK" );

  if ( i_ch_val != SUCC_BFR )
  {
    fn_errlog(c_ServiceName,"L31245", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }
	
	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName,"ORS2-RD After Call To SFO_UPD_SPDBK :%s:",c_time);
	}

	i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"L31250", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName,"ORS2-RD After Commit :%s:",c_time);
	}
  /*fn_cpy_ddr(st_spd_xchbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_spd_xchbk.c_rout_crt,c_rout_str); /*** Ver 1.1 ***/

  i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_err_msg,
                            &st_spd_xchbk,
                            "vw_spdxchbk",
                            sizeof ( st_spd_xchbk ),
                            TPNOREPLY,
                            "SFO_SPD_ACK" );
  if ( i_ch_val != 0 )
  {
    fn_errlog(c_ServiceName,"L31255", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName,"ORS2-RD After Call To SFO_SPD_ACK  :%s:",c_time);
		fn_userlog(c_ServiceName,"After Complition Of fn_spd_ord_cncl");
	}

	return 0;
}

/******************************************************************************/
/*  This Updates Trade Conformation Details. 				                          */
/*  INPUT PARAMETERS                                                          */
/*      ptr_st_trd_conf- Pointer to a message structure got from Exchange     */
/*      c_pipe_id      - Pipe Id                                              */
/*      i_dwnld_flg    - Download Flag                                        */
/*      c_ServiceName  - Service Name currently running                       */
/*      c_err_msg      - Error message to be returned                         */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/

int fn_trd_confirmation ( struct st_trade_confirm *ptr_st_trd_conf,
													char *c_xchng_cd,
                     			char *c_pipe_id,
												  char *c_rout_str,
													char *c_trd_dt,
													char *c_trd_ref,
										 			int i_dwnld_flg,
                     			char *c_ServiceName,
                     			char *c_err_msg )
{
	int i_trnsctn;
  int i_ch_val;
	int i_rec_cnt=0;    /*** Ver 1.2 ***/

	long long ll_log_tm_stmp;
	long long ll_log_tm_stmp_1;

	long li_ftt_stream_no;						/**Ver 1.7 **/

	double d_tm_stmp;

  struct vw_orderbook st_i_ordbk;
  struct vw_tradebook st_i_trdbk;
  struct vw_sequence st_seq;
  struct vw_contract st_cntrct;

  char c_svc_name [ 16 ];
  char c_refack_tmp[LEN_ORD_REFACK];
	char rfrnc[19],c_date[5] ;
	char ack[LEN_XCHNG_ACK];
	char c_mtch_accnt [10 + 1];
	char c_xchng_ack[17];
	char c_utrd_dt[23];
	char c_tmpjiffy[16];
	char c_trg_filter[17];
	char c_usr_id[16];				   /***	Ver	2.0	***/
	char c_span_flag  = 'N';     /*** Ver 2.2 ***/
	char c_lst_act_ref [22];		 /*** ver 6.1 ***/
	MEMSET(c_lst_act_ref);			 /*** ver 6.1 ***/

  int i2,i3 =0 ;
	
	MEMSET(st_i_ordbk);
	MEMSET(st_i_trdbk);
	MEMSET(st_seq);
	MEMSET(st_cntrct);

	if(DEBUG_MSG_LVL_3)  /** Ver 4.9 , debug changed from 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside Function fn_trd_cnfrmtn");
	}

/*	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"L31260", LIBMSG, c_err_msg);
    return -1;
  }	****/				/*** Commented In Ver 1.2 ***/

	sprintf(st_i_ordbk.c_xchng_ack,"%16.0lf",ptr_st_trd_conf->d_response_order_number);
	strcpy ( st_i_ordbk.c_pipe_id , c_pipe_id );

  fn_userlog(c_ServiceName,"Inside Function fn_trd_cnfrmtn NORMAL For :%s:",st_i_ordbk.c_xchng_ack);

	if(DEBUG_MSG_LVL_0)
	{
		/*** fn_userlog(c_ServiceName,"Exchange Ack Is :%s:",st_i_ordbk.c_xchng_ack); *** Ver 4.9 ***/
    fn_userlog(c_ServiceName,"fn_trd_cnfrmtn:Exchange Ack Is :%s:",st_i_ordbk.c_xchng_ack);  /** ver 4.9 **/
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",st_i_ordbk.c_pipe_id);
		fn_userlog(c_ServiceName,"EXCHANGE JIFFY IS :%lld:",ptr_st_trd_conf->st_hdr.c_time_stamp_1);
	}

	strcpy(c_tmpjiffy,ptr_st_trd_conf->st_hdr.c_time_stamp_1);

	/*fn_cpy_ddr(st_i_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_ordbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
  st_i_ordbk.c_oprn_typ = FOR_VIEW;

	memcpy (&ll_log_tm_stmp_1, ptr_st_trd_conf->st_hdr.c_time_stamp_1, 8 );
	d_tm_stmp = (double)ll_log_tm_stmp_1;

	sprintf(c_lst_act_ref,"%lld",ptr_st_trd_conf->ll_lastactivityref);  /*** Parag ***/ /*** Ver 6.1 ***/
  rtrim(c_lst_act_ref);     /*** Ver 6.1 ***/

	if(DEBUG_MSG_LVL_5)				/*** Added in Ver 6.1 ***/
  {
		fn_userlog(c_ServiceName,"Last Actiivity ref in trade confirmation is :%s:",c_lst_act_ref);
	}

	while (i_rec_cnt != 2)    /*** Ver 1.2 ***/
	{
		/***	Commented In Ver 1.3
		i_ch_val = fn_call_svc ( c_ServiceName,
    	                       c_err_msg,
      	                     &st_i_ordbk,
        	                   &st_i_ordbk,
          	                 "vw_orderbook",
            	               "vw_orderbook",
              	             sizeof ( st_i_ordbk ),
                	           sizeof ( st_i_ordbk ),
                  	         0,
                    	       "SFO_ACK_TO_ORD" );
		*****************************************************/

		i_ch_val = fn_ack_to_ord (&st_i_ordbk,c_ServiceName,c_usr_id,c_err_msg);			/***	Ver	2.0	***/

  	if ( i_ch_val != 0)
  	{
			if ( i_ch_val == TOO_MANY_ROWS_FETCHED )
    	{
				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"TOO_MANY_ROWS_FETCHED");
				}
      	if ( strcmp ( c_xchng_cd, "NFO" ) == 0 )
      	{
					fn_userlog(c_ServiceName,"Before Call To SFO_AC_TO_ORD");
        	i_ch_val = fn_nse_to_eba_tkn ( ptr_st_trd_conf->l_token,
         	                               &st_cntrct,
																				 c_rout_str,
            	                           c_ServiceName,
              	                         c_err_msg );
        	if ( i_ch_val != 0 )
        	{
						fn_errlog(c_ServiceName,"L31265", LIBMSG, c_err_msg);
          	return -1;
        	}

        	strcpy ( st_i_ordbk.c_xchng_cd, st_cntrct.c_xchng_cd );
        	st_i_ordbk.c_prd_typ = st_cntrct.c_prd_typ;
        	strcpy ( st_i_ordbk.c_expry_dt, st_cntrct.c_expry_dt );
        	st_i_ordbk.c_exrc_typ = st_cntrct.c_exrc_typ;
        	st_i_ordbk.c_opt_typ = st_cntrct.c_opt_typ;
        	st_i_ordbk.l_strike_prc = st_cntrct.l_strike_prc;
        	st_i_ordbk.c_ctgry_indstk = st_cntrct.c_ctgry_indstk;
        	strcpy ( st_i_ordbk.c_undrlyng, st_cntrct.c_undrlyng );
        	st_i_ordbk.l_ca_lvl = st_cntrct.l_ca_lvl;
					sprintf(st_i_ordbk.c_xchng_ack,"%16.0lf",ptr_st_trd_conf->d_response_order_number);
        	strcpy ( st_i_ordbk.c_pipe_id , c_pipe_id );

        	/*fn_cpy_ddr(st_i_ordbk.c_rout_crt);*/		/*** Ver 1.1 ***/

					if(DEBUG_MSG_LVL_3)
					{
						fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
					}
					strcpy(st_i_ordbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

        	st_i_ordbk.c_oprn_typ = FOR_VIEW;
					
					/***	Commented In Ver 2.0
					i_ch_val = fn_call_svc ( c_ServiceName,
          	                       c_err_msg,
            	                     &st_i_ordbk,
              	                   &st_i_ordbk,
                	                 "vw_orderbook",
                  	               "vw_orderbook",
                    	             sizeof ( st_i_ordbk ),
                      	           sizeof ( st_i_ordbk ),
                        	         0,
                          	       "SFO_AC_TO_ORD" );
					****************************************************/

					i_ch_val	=	fn_ac_to_ord (	c_ServiceName,			/***	Ver	2.0	***/
																			c_err_msg,
																			c_usr_id,
																			&st_i_ordbk
																	 );

        	if ( i_ch_val !=  0 )
        	{
          	fn_errlog(c_ServiceName,"L31270", LIBMSG, c_err_msg);
          	return -1;
        	}
		
					fn_gettime();

					if(DEBUG_MSG_LVL_3)
					{
						fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_AC_TO_ORD :%s:",c_time);
					}
      	}
      	else
      	{
        	fn_errlog(c_ServiceName,"S31060", "Logic error", c_err_msg);
        	return -1;
      	}

			}
			else
			{
				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Order Confirmation Is Not Processed");
          /*** fn_userlog(c_ServiceName,"Before Going To Sleep For 2.5 Seconds"); *** Ver 4.2 ***/
          /***fn_userlog(c_ServiceName,"Before Going To Sleep For 5 MiliSeconds");  *** Ver 4.2 *** ver 4.5 ***/
          fn_userlog(c_ServiceName,"Before Going To Sleep For 100 MicroSeconds");  /** Ver 4.5 **/
				}

        /**** sleep (2.5); **** ver 4.2 ***/
        /**** usleep(5000); **** 5 milliseconds sleep ** Ver 4.2 *** commented in Ver 4.5 *****/
        usleep(5000);   /**** Ver 4.5 ***/
        i_rec_cnt ++;

				if(DEBUG_MSG_LVL_1)  /** Ver 4.5 , Debug level set to zero ***/ /** Ver 4.9, debug changed from 0 to 1 **/
				{
					fn_userlog(c_ServiceName,"Record Count Is :%d: for :%s:",i_rec_cnt,st_i_ordbk.c_xchng_ack); /** Ver 4.5 **/
				}
    	}
	
		}
	
		if(i_ch_val == 0)
  	{
			/*** Here Value of i_ch_val depends on call to SFO_ACK_TO_ORD For Normal Order & SFO_AC_TO_ORD For Spread Order ***/

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside Success Condition");
			}

  		break;
  	}
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_ACK_TO_ORD :%s:",c_time);
	}

	if(i_rec_cnt == 2)    /*** Ver 1.2 ***/
  {
		/***	strcpy(c_trg_filter,"NA");			Ver 1.6	***/

	
		sprintf ( c_trg_filter, "FFO_FILTER=='NFO'" );	/***	Ver 1.6	***/
		
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Before Inserting Into FTT Table");
			fn_userlog(c_ServiceName,"Before Inserting Order Number Is :%lf:",ptr_st_trd_conf->d_response_order_number);
			fn_userlog(c_ServiceName,"Before Inserting Pipe ID Is :%s:",c_pipe_id);
			fn_userlog(c_ServiceName,"Before Inserting Token No. Is :%ld:",ptr_st_trd_conf->l_token);
			fn_userlog(c_ServiceName,"Before Inserting Trade Date Is :%s:",c_trd_dt);
			fn_userlog(c_ServiceName,"Before Inserting Trade Ref Is :%s:",c_trd_ref);
			fn_userlog(c_ServiceName,"Before Inserting Fill No. Is :%ld:",ptr_st_trd_conf->li_fill_number);
			fn_userlog(c_ServiceName,"Before Inserting Activity Time Is :%ld:",ptr_st_trd_conf->li_activity_time);
			fn_userlog(c_ServiceName,"Before Inserting Fill Qty Is ;%ld:",ptr_st_trd_conf->li_fill_quantity);
			fn_userlog(c_ServiceName,"Before Inserting Fill Price Is :%ld:",ptr_st_trd_conf->li_fill_price);
			fn_userlog(c_ServiceName,"Before Inserting Time Stamp Is :%lf:",d_tm_stmp);
			fn_userlog(c_ServiceName,"Trigger Filter Is :%s:",c_trg_filter);
		}

		fn_long_to_timearr(c_utrd_dt,ptr_st_trd_conf->li_activity_time);
		sprintf(c_xchng_ack,"%16.0lf",ptr_st_trd_conf->d_response_order_number);
		
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Before Inserting Activity Time Is ;%s:",c_utrd_dt);
			fn_userlog(c_ServiceName,"Before Inserting Ack Is :%s:",c_xchng_ack);	
		}

    i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
    if ( i_trnsctn == -1)
    {
      fn_errlog(c_ServiceName,"L31275", LIBMSG, c_err_msg);
      return -1;
    }

		li_ftt_stream_no = (long) ptr_st_trd_conf->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1];

    EXEC SQL
      INSERT INTO ftt_fo_tmp_trd
			(
				FTT_ACK_NMBR,
				FTT_PIPE_ID,
				FTT_TOKEN_NO,
				FTT_XCHNGTRD_NO,
				FTT_TRD_TM,
				FTT_EXCTD_QTY,
				FTT_EXCTD_RT,
				FTT_JIFFY,
				FTT_PRCSD_FLG,
				FTT_STREAM_NO,
        FTT_XCHNG_CD,       /*** Ver 2.4 ***/
				FTT_LST_ACT_REF,		/*** Ver 6.1 ***/
				FTT_TRANS_CD 				/*** Ver 6.1 ***/
			)
      VALUES
      (
        to_char(:st_i_ordbk.c_xchng_ack),
        :c_pipe_id,
        :ptr_st_trd_conf->l_token,
        :ptr_st_trd_conf->li_fill_number,
        to_date(:c_utrd_dt,'DD-Mon-yyyy hh24:mi:ss'),
        :ptr_st_trd_conf->li_fill_quantity,
        :ptr_st_trd_conf->li_fill_price,
				:d_tm_stmp,
        'N',
				:li_ftt_stream_no,						/**Ver 1.7 **/
        :c_xchng_cd,                  /*** Ver 2.4 ***/
				:c_lst_act_ref,							  /*** Ver 6.1 ***/
				:ptr_st_trd_conf->st_hdr.si_transaction_code			/*** Ver 6.1 ***/
      );

    if ( SQLCODE != 0 )
    {
      fn_errlog ( c_ServiceName, "S31065",SQLMSG, c_err_msg);
      fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
      return -1;
    }

    i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName,"L31280", LIBMSG, c_err_msg);
      return -1;
    }

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Before Posting Trigger TRG_PRCS_TRD");
		}

    fn_pst_trg(c_ServiceName,"TRG_PRCS_TRD","TRG_PRCS_TRD",c_trg_filter);
		
		return 0;			/***	Ver 1.6	***/
  }

	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );	/***	Ver 1.2 ***/
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"L31285", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Transaction Begin :%s:",c_time);
	}

  strcpy ( st_seq.c_pipe_id, st_i_ordbk.c_pipe_id );
  strcpy ( st_seq.c_trd_dt, ( char * )c_trd_dt );
	
	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",st_seq.c_pipe_id);
		fn_userlog(c_ServiceName,"Trade Date Is :%s:",st_seq.c_trd_dt);
	}
 
  /*fn_cpy_ddr(st_seq.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}
	strcpy(st_seq.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
  st_seq.c_rqst_typ = GET_TRD_SEQ;

	i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_seq,
                           &st_seq,
                           "vw_sequence",
                           "vw_sequence",
                           sizeof ( st_seq ),
                           sizeof ( st_seq ),
                           0,
                           "SFO_GET_SEQ" );

  if ( i_ch_val != SUCC_BFR )
  {
    fn_errlog(c_ServiceName,"L31290", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }
	
	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_GET_SEQ :%s:",c_time);
		fn_userlog(c_ServiceName,"Sequence Number Is :%ld:",st_seq.l_seq_num);
		fn_userlog(c_ServiceName,"Trade Reference Is :%s:",c_trd_ref);
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",c_pipe_id);
	}

	sprintf ( st_i_trdbk.c_trd_rfrnc, "%s%s%08ld",c_trd_ref, c_pipe_id, st_seq.l_seq_num );
	strcpy ( st_i_trdbk.c_cln_mtch_accnt , st_i_ordbk.c_cln_mtch_accnt );
	strcpy ( c_mtch_accnt , st_i_ordbk.c_cln_mtch_accnt );
  strcpy ( st_i_trdbk.c_xchng_cd , st_i_ordbk.c_xchng_cd );
  st_i_trdbk.c_prd_typ = st_i_ordbk.c_prd_typ;
  strcpy ( st_i_trdbk.c_undrlyng , st_i_ordbk.c_undrlyng );
  strcpy ( st_i_trdbk.c_expry_dt , st_i_ordbk.c_expry_dt );
  st_i_trdbk.c_exrc_typ = st_i_ordbk.c_exrc_typ;
  st_i_trdbk.c_opt_typ = st_i_ordbk.c_opt_typ;
  st_i_trdbk.l_strike_prc = st_i_ordbk.l_strike_prc;
  st_i_trdbk.c_ctgry_indstk = st_i_ordbk.c_ctgry_indstk;
  strcpy ( st_i_trdbk.c_ordr_rfrnc , st_i_ordbk.c_ordr_rfrnc );
  strcpy ( st_i_trdbk.c_xchng_ack , st_i_ordbk.c_xchng_ack );
 	st_i_trdbk.l_xchng_trd_no = ptr_st_trd_conf->li_fill_number;
	fn_long_to_timearr(st_i_trdbk.c_trd_dt,ptr_st_trd_conf->li_activity_time);
	st_i_trdbk.c_trd_flw = st_i_ordbk.c_ordr_flw;
  st_i_trdbk.l_exctd_qty = ptr_st_trd_conf->li_fill_quantity;
	st_i_trdbk.l_exctd_rt = ptr_st_trd_conf->li_fill_price;
	st_i_trdbk.l_dwnld_flg = i_dwnld_flg;
	memcpy (&ll_log_tm_stmp, ptr_st_trd_conf->st_hdr.c_time_stamp_1, 8 );
	st_i_trdbk.d_jiffy = (double) ll_log_tm_stmp;
	
	st_i_trdbk.l_ca_lvl	=	 (long) ptr_st_trd_conf->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1];			/*** Ver 1.7	***/

  /*fn_cpy_ddr(st_i_trdbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_trdbk.c_rout_crt,c_rout_str); /*** Ver 1.1 ***/
	
	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Trade Reference Is :%s:",st_i_trdbk.c_trd_rfrnc);
		fn_userlog(c_ServiceName,"Match Account Is :%s:",st_i_trdbk.c_cln_mtch_accnt);
		fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_i_trdbk.c_xchng_cd);
		fn_userlog(c_ServiceName,"Product Type Is :%c:",st_i_trdbk.c_prd_typ);
		fn_userlog(c_ServiceName,"Underlying Is :%s:",st_i_trdbk.c_undrlyng);
		fn_userlog(c_ServiceName,"Expiry Date Is :%s:",st_i_trdbk.c_expry_dt);
		fn_userlog(c_ServiceName,"Exercise Type Is :%c:",st_i_trdbk.c_exrc_typ);
		fn_userlog(c_ServiceName,"Option Type Is :%c:",st_i_trdbk.c_opt_typ);
		fn_userlog(c_ServiceName,"Strike Price is :%ld:",st_i_trdbk.l_strike_prc);
		fn_userlog(c_ServiceName,"IndexStock Is :%c:",st_i_trdbk.c_ctgry_indstk);
		fn_userlog(c_ServiceName,"Order Reference No. Is :%s:",st_i_trdbk.c_ordr_rfrnc);
		fn_userlog(c_ServiceName,"Exchange Ack Is :%s:",st_i_trdbk.c_xchng_ack);
		fn_userlog(c_ServiceName,"Exchange Trade No. Is :%ld:", ptr_st_trd_conf->li_fill_number);
		fn_userlog(c_ServiceName,"Trade No. Is :%ld:",st_i_trdbk.l_xchng_trd_no);
		fn_userlog(c_ServiceName,"Exchange Trade Date Is :%ld:",ptr_st_trd_conf->li_activity_time);
		fn_userlog(c_ServiceName,"Trade Date Is :%s:",st_i_trdbk.c_trd_dt);
		fn_userlog(c_ServiceName,"Trade Flow Is :%c:",st_i_trdbk.c_trd_flw);
		fn_userlog(c_ServiceName,"In TRD_CONF, the stream number receieved from exchange is :%ld:",st_i_trdbk.l_ca_lvl);
		fn_userlog(c_ServiceName,"Exchange Executed Qty Is :%ld:", ptr_st_trd_conf->li_fill_quantity);
		fn_userlog(c_ServiceName,"Exchange Executed Rate Is :%ld:",ptr_st_trd_conf->li_fill_price);
		fn_userlog(c_ServiceName,"Executed Qty Is :%ld:",st_i_trdbk.l_exctd_qty);
		fn_userlog(c_ServiceName,"Executed Rate Is :%ld:",st_i_trdbk.l_exctd_rt);
		fn_userlog(c_ServiceName,"DownLoad flag Is :%ld:",st_i_trdbk.l_dwnld_flg);
		fn_userlog(c_ServiceName,"Exchange Log Time Stamp Is :%lld:",ll_log_tm_stmp);
		fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_trdbk.d_jiffy);
	}
	
	strcpy(st_i_trdbk.c_user_id,st_i_ordbk.c_user_id);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"USER ID IS  :%s:",st_i_trdbk.c_user_id);     
		fn_userlog(c_ServiceName,"CALLING SFO_UPD_TRDBK");
		fn_userlog(c_ServiceName,"Last Activity Ref is :%s:",c_lst_act_ref);      /*** Ver 6.1 ***/
	}

	st_i_trdbk.c_oprn_typ = INSERT_TRADE_CONF;

	/***	Commented In Ver 2.0

	i_ch_val = fn_call_svc ( c_ServiceName,
                            c_err_msg,
                            &st_i_trdbk,
                            &st_i_trdbk,
                            "vw_tradebook",
                            "vw_tradebook",
                            sizeof ( st_i_trdbk ),
                            sizeof ( st_i_trdbk ),
                            0,
                            "SFO_UPD_TRDBK" );

	*********************************/

	i_ch_val = fn_upd_trdbk (	c_ServiceName,						/***	Ver	2.0	***/
													 	c_err_msg,
														c_usr_id,
														c_lst_act_ref,            /*** Ver 6.1 ***/
													 	&st_i_trdbk
												 );

  if ( ( i_ch_val != SUCC_BFR ) &&
       ( i_ch_val != RECORD_ALREADY_PRCSD ) )
  {
    fn_errlog(c_ServiceName,"L31295", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }

  else if ( i_ch_val == RECORD_ALREADY_PRCSD )
  {
		fn_userlog ( c_ServiceName, "Record already processed" );
		fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return 0;
	}

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName, "ORS2-RD After Call To SFO_UPD_TRDBK :%s:",c_time);
	}

	i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"L31300", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName, "ORS2-RD After Commit :%s:",c_time);
	}

  if(DEBUG_MSG_LVL_3) /** ver 4.9 **/
  {
	  fn_userlog(c_ServiceName," INSIDE fn_trd_confirmation -- checking For SPAN Flag");
  }

	 EXEC  SQL                                                           /*** Ver 2.2 ***/
     SELECT  nvl(CLM_SPAN_ALLWD,'N')
     INTO    :c_span_flag
     FROM    CLM_CLNT_MSTR
     WHERE   CLM_MTCH_ACCNT =:st_i_trdbk.c_cln_mtch_accnt;

    if(SQLCODE !=0 && SQLCODE != NO_DATA_FOUND )
    {
			fn_errlog(c_ServiceName,"L31305", SQLMSG, c_err_msg);
    	return -1;
    }

  if(DEBUG_MSG_LVL_3)  /** ver 4.9 **/
  {
	  fn_userlog(c_ServiceName," SPAN Flag Is :%c: for Customer :%s: ",c_span_flag,st_i_trdbk.c_cln_mtch_accnt);
  }

	if (c_span_flag == 'N')  /*** Ver 2.2 - Span Check Added ***/
	{
    if(DEBUG_MSG_LVL_3)  /** ver 4.9 **/
    {	
		  fn_userlog(c_ServiceName, " Inside NORMAL Customer ");
    }

		if ( st_i_trdbk.c_prd_typ == OPTIONS || st_i_trdbk.c_prd_typ == OPTIONPLUS )	/*** OPTIONPLUS added in Ver 3.0 ***/
  	{
      if(DEBUG_MSG_LVL_3)  /** ver 4.9 **/
      {
			  fn_userlog(c_ServiceName," Calling SFO_OPT_TCONF ");
      }
  
   		strcpy ( c_svc_name , "SFO_OPT_TCONF" );
  	}
  	/*** else if ( st_i_trdbk.c_prd_typ == FUTURES || st_i_trdbk.c_prd_typ == FUTURE_PLUS  ) Commented in Ver 2.5 ***/
  	else if ( st_i_trdbk.c_prd_typ == FUTURES || st_i_trdbk.c_prd_typ == FUTURE_PLUS || st_i_trdbk.c_prd_typ == SLTP_FUTUREPLUS )		/*** Ver 2.5 ***/
  	{
      if(DEBUG_MSG_LVL_3)  /** ver 4.9 **/
      {
			  fn_userlog(c_ServiceName," Calling SFO_FUT_TCONF ");
      }

   		strcpy ( c_svc_name , "SFO_FUT_TCONF" );
  	}

 		/*fn_cpy_ddr(st_i_trdbk.c_rout_crt);*/				/*** Ver 1.1 ***/

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		}

		strcpy(st_i_trdbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
	
		i_ch_val = fn_acall_svc ( c_ServiceName,
    	                        c_err_msg,
      	                      &st_i_trdbk,
        	                    "vw_tradebook",
          	                  sizeof ( st_i_trdbk ),
            	                TPNOREPLY,
              	              c_svc_name );
  	if ( i_ch_val != 0 )
  	{
    	fn_errlog(c_ServiceName,"L31310", LIBMSG, c_err_msg);
    	return -1;
  	}
	}
	else if (c_span_flag == 'Y')   /*** Ver 2.2 ***/
	{
		strcpy(st_i_trdbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/
    if(DEBUG_MSG_LVL_3)  /** ver 4.9 **/
    {
		  fn_userlog(c_ServiceName, " Inside SPAN Customer ");
    }

		if(DEBUG_MSG_LVL_3)
  	{
    	fn_userlog(c_ServiceName,"  Calling SFO_PRCS_TRD ");
    	fn_userlog(c_ServiceName," st_i_trdbk.c_rout_crt Is :%s:",st_i_trdbk.c_rout_crt);
    	fn_userlog(c_ServiceName,"  st_i_trdbk.l_eba_cntrct_id Is :%ld:",st_i_trdbk.l_eba_cntrct_id);
    	fn_userlog(c_ServiceName,"Trade Reference Is :%s:",st_i_trdbk.c_trd_rfrnc);
    	fn_userlog(c_ServiceName,"Match Account Is :%s:",st_i_trdbk.c_cln_mtch_accnt);
    	fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_i_trdbk.c_xchng_cd);
    	fn_userlog(c_ServiceName,"Product Type Is :%c:",st_i_trdbk.c_prd_typ);
    	fn_userlog(c_ServiceName,"Underlying Is :%s:",st_i_trdbk.c_undrlyng);
    	fn_userlog(c_ServiceName,"Expiry Date Is :%s:",st_i_trdbk.c_expry_dt);
    	fn_userlog(c_ServiceName,"Exercise Type Is :%c:",st_i_trdbk.c_exrc_typ);
    	fn_userlog(c_ServiceName,"Option Type Is :%c:",st_i_trdbk.c_opt_typ);
    	fn_userlog(c_ServiceName,"Strike Price is :%ld:",st_i_trdbk.l_strike_prc);
    	fn_userlog(c_ServiceName,"IndexStock Is :%c:",st_i_trdbk.c_ctgry_indstk);
    	fn_userlog(c_ServiceName,"Order Reference No. Is :%s:",st_i_trdbk.c_ordr_rfrnc);
    	fn_userlog(c_ServiceName,"Exchange Ack Is :%s:",st_i_trdbk.c_xchng_ack);
    	fn_userlog(c_ServiceName,"Exchange Trade No. Is :%ld:", ptr_st_trd_conf->li_fill_number);
    	fn_userlog(c_ServiceName,"Trade No. Is :%ld:",st_i_trdbk.l_xchng_trd_no);
    	fn_userlog(c_ServiceName,"Exchange Trade Date Is :%ld:",ptr_st_trd_conf->li_activity_time);
    	fn_userlog(c_ServiceName,"Trade Date Is :%s:",st_i_trdbk.c_trd_dt);
    	fn_userlog(c_ServiceName,"Trade Flow Is :%c:",st_i_trdbk.c_trd_flw);
    	fn_userlog(c_ServiceName,"In TRD_CONF, the stream number receieved from exchange is :%ld:",
																st_i_trdbk.l_ca_lvl);
    	fn_userlog(c_ServiceName,"Exchange Executed Qty Is :%ld:", ptr_st_trd_conf->li_fill_quantity);
    	fn_userlog(c_ServiceName,"Exchange Executed Rate Is :%ld:",ptr_st_trd_conf->li_fill_price);
    	fn_userlog(c_ServiceName,"Executed Qty Is :%ld:",st_i_trdbk.l_exctd_qty);
    	fn_userlog(c_ServiceName,"Executed Rate Is :%ld:",st_i_trdbk.l_exctd_rt);
    	fn_userlog(c_ServiceName,"DownLoad flag Is :%ld:",st_i_trdbk.l_dwnld_flg);
    	fn_userlog(c_ServiceName,"Exchange Log Time Stamp Is :%lld:",ll_log_tm_stmp);
    	fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_trdbk.d_jiffy);
 	 	}

		i_ch_val       = fn_acall_svc_fml ( c_ServiceName,   /*** Ver 4.4 change call to acall ***/
                                       c_err_msg,
                                       "SFO_PRCS_TRD",
                                       0,
                                       18,
																/***   0,  commented in ver 4.4 **/
                                       FFO_ROUT_CRT, (char *)st_i_trdbk.c_rout_crt,  
                                       FFO_ORDR_RFRNC, (char *)st_i_trdbk.c_ordr_rfrnc,
                                       FFO_TRD_RFRNC, (char *)st_i_trdbk.c_trd_rfrnc,
                                       FFO_EBA_MTCH_ACT_NO, (char *)st_i_trdbk.c_cln_mtch_accnt,
                                       FFO_EBA_CNTRCT_ID, (char *)&st_i_trdbk.l_eba_cntrct_id,/* Needtoget */
                                       FFO_XCHNG_CD, (char *)st_i_trdbk.c_xchng_cd,
                                       FFO_PRDCT_TYP, (char *)&st_i_trdbk.c_prd_typ,
                                       FFO_UNDRLYNG, (char *)st_i_trdbk.c_undrlyng,
                                       FFO_EXPRY_DT, (char *)st_i_trdbk.c_expry_dt,
                                       FFO_EXER_TYP, (char *)&st_i_trdbk.c_exrc_typ,
                                       FFO_OPT_TYP, (char *)&st_i_trdbk.c_opt_typ,
                                       FFO_STRK_PRC, (char *)&st_i_trdbk.l_strike_prc,
                                       FFO_CTGRY_INDSTK, (char *)&st_i_trdbk.c_ctgry_indstk,
                                       FFO_CA_LVL, (char *)&st_i_trdbk.l_ca_lvl,    
                                       FFO_TRD_FLW, (char *)&st_i_trdbk.c_trd_flw,
                                       FFO_EXEC_QTY, (char *)&st_i_trdbk.l_exctd_qty,
                                       FFO_EXCTD_RT, (char *)&st_i_trdbk.l_exctd_rt,
                                       FFO_TRD_DT, (char *)st_i_trdbk.c_trd_dt
                                     );
		if ( i_ch_val != 0 )
    {
      fn_errlog(c_ServiceName,"L31315", LIBMSG, c_err_msg);
      return -1;
    }


	}
	

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName, "ORS2-RD After Call To FUT/OPT TCONF :%s:",c_time);
  	fn_userlog(c_ServiceName, "After futopt tconf");
	}

	return 0;
}

/******************************************************************************/
/*  To update the response for new order / Exercise in database.              */
/*  INPUT PARAMETERS                                                          */
/*      ptr_st_expl_reqres- Pointer to a message structure got from receive   */
/*			c_pie_id			- Pipe Id																								*/
/*			i_dwnld_flg   - Download Flag																				  */
/*      c_ServiceName - Service Name currently running                        */
/*      c_err_msg     - Error message to be returned                          */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/


int fn_expl_reqres ( struct st_ex_pl_reqres *ptr_st_expl_reqres,
										 char *c_xchng_cd,
                     char *c_pipe_id,
										 char *c_rout_str,
										 char *c_trd_dt,
                     int  i_dwnld_flg,
                     char *c_ServiceName,
                     char *c_err_msg )
{
	int i_trnsctn;
  int i_ch_val;
  int i_tmp_cntr;

	long l_mdfctn_cntr;
	long long ll_log_tm_stmp;

  struct vw_xchngbook st_i_xchngbk;
  struct vw_exrcbook st_i_exrcbk;
  struct vw_orderbook st_i_ordbk;

  char c_refack_tmp[LEN_ORD_REFACK];

	MEMSET(st_i_xchngbk);
	MEMSET(st_i_exrcbk);
	MEMSET(st_i_ordbk);

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"Inside Function fn_expl_reqres");
	}

	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"L31320", LIBMSG, c_err_msg);
    return -1;
  }

  sprintf(st_i_exrcbk.c_xchng_ack,"%16.0lf",ptr_st_expl_reqres->st_ex_pl_data.d_expl_number);

	memcpy(&ll_log_tm_stmp,ptr_st_expl_reqres->st_hdr.c_time_stamp_1, 8 );

	st_i_xchngbk.l_ors_msg_typ = ORS_NEW_EXER_RES;

	fn_nsetoors_char ( st_i_xchngbk.c_ordr_rfrnc, 
										 LEN_ORDR_RFRNC,
										 ptr_st_expl_reqres->st_ex_pl_data.c_remarks, 
										 LEN_REMARKS_1 );

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"Exchange Book Order Ref Is :%s:",st_i_xchngbk.c_ordr_rfrnc);
	}

	strcpy(st_i_exrcbk.c_exrc_ordr_rfrnc,st_i_xchngbk.c_ordr_rfrnc);

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"Exercise Book Order Ref Is :%s:",st_i_exrcbk.c_exrc_ordr_rfrnc);
	}

	EXEC SQL
		SELECT FEB_MDFCTN_CNTR
		INTO :st_i_xchngbk.l_mdfctn_cntr
		FROM FEB_FO_EXRC_RQST_BOOK
		WHERE FEB_EXRC_RFRNC_NO = :st_i_exrcbk.c_exrc_ordr_rfrnc;
		if ( SQLCODE != 0 )
		{
			fn_errlog ( c_ServiceName, "S31070", SQLMSG, c_err_msg );
			fn_userlog(c_ServiceName,"Failed for Order reference :%s:",st_i_exrcbk.c_exrc_ordr_rfrnc);
			return -1;
		}


	strcpy ( st_i_xchngbk.c_xchng_cd, c_xchng_cd );
	strcpy ( st_i_xchngbk.c_pipe_id, c_pipe_id );
	strcpy ( st_i_xchngbk.c_mod_trd_dt, c_trd_dt );
  st_i_xchngbk.c_plcd_stts = PLACED;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_i_xchngbk.c_xchng_cd);
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",st_i_xchngbk.c_pipe_id);
		fn_userlog(c_ServiceName,"Mod Trade Date Is :%s:",st_i_xchngbk.c_mod_trd_dt);  
	}

	st_i_xchngbk.d_jiffy = (double)ll_log_tm_stmp;
	st_i_xchngbk.l_dwnld_flg = i_dwnld_flg;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_xchngbk.d_jiffy);
		fn_userlog(c_ServiceName,"DownLoad Flag Is :%ld:",st_i_xchngbk.l_dwnld_flg);
	}

	/*fn_cpy_ddr(st_i_xchngbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str); /*** Ver 1.1 ***/
	st_i_xchngbk.l_bit_flg	=	ptr_st_expl_reqres->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1];			/***	Ver 1.7	***/
 
  if(DEBUG_MSG_LVL_3)  /*** Ver 4.9 **/
  { 
	  fn_userlog(c_ServiceName,"IN EXER_REXP, the stream number recd is :%ld:",st_i_xchngbk.l_bit_flg);
  }

	st_i_xchngbk.c_oprn_typ = UPDATE_PLACED_STTS;

	i_ch_val = fn_call_svc ( c_ServiceName,
												c_err_msg,
												&st_i_xchngbk,
												&st_i_xchngbk,
												"vw_xchngbook",
												"vw_xchngbook",
												sizeof ( st_i_xchngbk ),
												sizeof ( st_i_xchngbk ),
												0,
												"SFO_UPD_XCHNGBK" );

	if ( ( i_ch_val != SUCC_BFR ) &&
	 ( i_ch_val != RECORD_ALREADY_PRCSD ) )
	{
	fn_errlog(c_ServiceName,"L31325", LIBMSG, c_err_msg);
	fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
	return -1;
	}

	else if ( i_ch_val == RECORD_ALREADY_PRCSD )
	{
	fn_userlog(c_ServiceName,"Inside RECORD_ALREADY_PRCSD");
	fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
	fn_userlog ( c_ServiceName, "Record already processed" );
	return 0;
	}

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_XCHNGBK With UPDATE_PLACED_STTS :%s:",c_time);
		fn_userlog(c_ServiceName,"Exercise Order Refrence Is :%s:",st_i_exrcbk.c_exrc_ordr_rfrnc);
	}

	/*fn_cpy_ddr(st_i_exrcbk.c_rout_crt);*/		 /*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_exrcbk.c_rout_crt,c_rout_str); /*** Ver 1.1 ***/
	st_i_exrcbk.c_oprn_typ = UPDATE_XCHNG_RESPONSE;

	i_ch_val = fn_call_svc ( c_ServiceName,
											 c_err_msg,
											 &st_i_exrcbk,
											 &st_i_exrcbk,
											 "vw_exrcbook",
											 "vw_exrcbook",
											 sizeof ( st_i_exrcbk ),
											 sizeof ( st_i_exrcbk ),
											 0,
											 "SFO_UPD_EXBK" );

	if ( i_ch_val != SUCC_BFR )
	{
	fn_errlog(c_ServiceName,"L31330", LIBMSG, c_err_msg);
	fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
	return -1;
	}

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_EXBK With UPDATE_XCHNG_RESPONSE :%s:",c_time);
	}

	i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
	if ( i_ch_val == -1 )
	{
	fn_errlog(c_ServiceName,"L31335", LIBMSG, c_err_msg);
	return -1;
	}

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Commit :%s:",c_time);
		fn_userlog(c_ServiceName,"After Complition Of fn_expl_reqres");
	}

	return 0;
}	

/******************************************************************************/
/*  To update the accept and reject responses from Exchange.                  */
/*  INPUT PARAMETERS                                                          */
/*      ptr_st_expl_reqres- Pointer to a message structure got from Exchange  */
/*      c_pipe_id     - Pipe Id                                               */
/*      i_dwnld_flg   - Download Flag                                         */
/*      c_ServiceName - Service Name currently running                        */
/*      c_err_msg     - Error message to be returned                          */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/

int fn_expl_confirmation ( 	struct st_ex_pl_reqres *ptr_st_expl_reqres,
														char *c_xchng_cd,
														char *c_pipe_id,
														char *c_rout_str,
														char *c_trd_dt,
														char *c_ServiceName,
														int  i_dwnld_flg,
                						char *c_err_msg )
{
	char c_svc_name [15+1];
  char c_refack_tmp[LEN_ORD_REFACK];
	char c_ordr_rfrnc [19];
	char c_xchng_rmrks[256];
	char c_mtch_accnt [10 + 1];
	char c_xchng_err_msg [ 256 ];

	int i_ch_val;
  int i_trnsctn;

	long long ll_log_tm_stmp;

	struct vw_xchngbook st_i_xchngbk;
  struct vw_orderbook st_i_ordrbk;
  struct vw_exrcbook st_i_exrcbk;

	MEMSET(st_i_xchngbk);
	MEMSET(st_i_ordrbk);
	MEMSET(st_i_exrcbk);
	MEMSET(c_xchng_rmrks);
	MEMSET(c_xchng_err_msg);

	if(DEBUG_MSG_LVL_3)  /** Ver 4.9, debug changed from 0 to 3 **/
	{
		fn_userlog(c_ServiceName,"Inside function fn_expl_cnfrmtn");
	}

	switch (ptr_st_expl_reqres->st_hdr.si_transaction_code)
	{
		case EX_PL_CONFIRMATION :
		
			if(DEBUG_MSG_LVL_3)
			{	
				fn_userlog(c_ServiceName,"Inside EX_PL_CONFIRMATION Case");
			}

			if(ptr_st_expl_reqres->st_hdr.si_error_code == 0 )
			{
				st_i_xchngbk.l_ors_msg_typ = ORS_NEW_EXER_ACPT;

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"MSG Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				}
			}
			else
			{
				st_i_xchngbk.l_ors_msg_typ = ORS_NEW_EXER_RJCT;

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"MSG Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				}
			}

			fn_long_to_timearr (st_i_xchngbk.c_entry_dt_tm,ptr_st_expl_reqres->st_ex_pl_data.li_entry_date_tm);
			
			break;

		case EX_PL_MOD_CONFIRMATION :

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside EX_PL_MOD_CONFIRMATION Case");
			}

			if(ptr_st_expl_reqres->st_hdr.si_error_code == 0 )
			{
				st_i_xchngbk.l_ors_msg_typ = ORS_MOD_EXER_ACPT;

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"MSG Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				}
			}
			else
			{
				st_i_xchngbk.l_ors_msg_typ = ORS_MOD_EXER_RJCT;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"MSG Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				}
			}

			break;

		case EX_PL_CXL_CONFIRMATION :

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside EX_PL_CXL_CONFIRMATION Case");
			}

	  	if(ptr_st_expl_reqres->st_hdr.si_error_code == 0 )
			{
				st_i_xchngbk.l_ors_msg_typ = ORS_CAN_EXER_ACPT;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"MSG Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				}
			}
			else
			{
				st_i_xchngbk.l_ors_msg_typ = ORS_CAN_EXER_RJCT;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"MSG Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
				}
			}

			break;

		case EX_PL_ENTRY_IN :
		
			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside EX_PL_ENTRY_IN Case");
			}

			st_i_xchngbk.l_ors_msg_typ = ORS_NEW_EXER_RJCT;

			if(DEBUG_MSG_LVL_3)
			{
      	fn_userlog(c_ServiceName,"MSG Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
			}

      fn_long_to_timearr (st_i_xchngbk.c_entry_dt_tm,ptr_st_expl_reqres->st_ex_pl_data.li_entry_date_tm);
			break;

		default :
      fn_userlog( c_ServiceName, "Protocol error %d",ptr_st_expl_reqres->st_hdr.si_transaction_code);
			break;
	}

	if(ptr_st_expl_reqres->st_hdr.si_error_code != 0 )
	{
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"The ERROR CODE FOR ORDER RJCT IS :%d:",ptr_st_expl_reqres->st_hdr.si_error_code);
		}

		i_ch_val = fn_get_errmsg( ptr_st_expl_reqres->st_hdr.si_error_code,ptr_st_expl_reqres->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

    if(i_ch_val == -1 )
    {
   		fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
      return -1;
    }

    strcpy(c_xchng_rmrks,c_xchng_err_msg);
	}


	sprintf(st_i_exrcbk.c_xchng_ack,"%16.0lf",ptr_st_expl_reqres->st_ex_pl_data.d_expl_number );

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"Exchange Ack Is :%s:",st_i_exrcbk.c_xchng_ack);
	}

  strcpy ( st_i_exrcbk.c_pipe_id , c_pipe_id );

  /*fn_cpy_ddr(st_i_exrcbk.c_rout_crt);*/			/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_exrcbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Before Call To SFO_ACK_TO_EXRQ");
	}

  i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_i_exrcbk,
                           &st_i_exrcbk,
                           "vw_exrcbook",
                           "vw_exrcbook",
                           sizeof ( st_i_exrcbk ),
                           sizeof ( st_i_exrcbk ),
                           0,
                           "SFO_ACK_TO_EXRQ" );

  if ( i_ch_val != SUCC_BFR )
  {
  	fn_errlog(c_ServiceName,"L31340", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_ACK_TO_EXRQ :%s:",c_time);
	}

	strcpy ( st_i_xchngbk.c_ordr_rfrnc, st_i_exrcbk.c_exrc_ordr_rfrnc );
  st_i_xchngbk.l_mdfctn_cntr = st_i_exrcbk.l_mdfctn_cntr;
  strcpy ( st_i_xchngbk.c_xchng_cd, st_i_exrcbk.c_xchng_cd );
  strcpy ( st_i_xchngbk.c_pipe_id, st_i_exrcbk.c_pipe_id );
  st_i_xchngbk.c_ord_typ = st_i_exrcbk.c_exrc_rqst_typ;

  strcpy(c_svc_name , "SFO_EXREQ_ACK" );

  strcpy(c_mtch_accnt, st_i_exrcbk.c_cln_mtch_accnt);             
	strcpy(c_ordr_rfrnc, st_i_exrcbk.c_exrc_ordr_rfrnc);   

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Order Reference NO. Is :%s:",st_i_exrcbk.c_exrc_ordr_rfrnc);
		fn_userlog(c_ServiceName,"Modification Counter Is :%ld:",st_i_exrcbk.l_mdfctn_cntr);
		fn_userlog(c_ServiceName,"Exchange Code is :%s:",st_i_exrcbk.c_xchng_cd);
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",st_i_exrcbk.c_pipe_id);
		fn_userlog(c_ServiceName,"Match Account Is :%s:",st_i_exrcbk.c_cln_mtch_accnt);
		fn_userlog(c_ServiceName,"Request Type is :%c:",st_i_exrcbk.c_exrc_rqst_typ);
	}

	switch(st_i_xchngbk.l_ors_msg_typ)
	{
		case ORS_NEW_EXER_ACPT :
		case ORS_MOD_EXER_ACPT :
		case ORS_CAN_EXER_ACPT :
	
			st_i_xchngbk.c_plcd_stts = ACCEPT;
      break;

		case ORS_NEW_EXER_RJCT :
		case ORS_MOD_EXER_RJCT :
		case ORS_CAN_ORD_RJCT  :

			st_i_xchngbk.c_plcd_stts = REJECT;
      break;
	}

  st_i_xchngbk.c_rms_prcsd_flg = NOT_PROCESSED;
	fn_long_to_timearr(st_i_xchngbk.c_ack_tm,ptr_st_expl_reqres->st_hdr.li_log_time );
	strcpy ( st_i_xchngbk.c_xchng_rmrks ,c_xchng_rmrks);		
	memcpy(&ll_log_tm_stmp,ptr_st_expl_reqres->st_hdr.c_time_stamp_1, 8 );
  st_i_xchngbk.d_jiffy = (double)ll_log_tm_stmp;
	st_i_xchngbk.l_dwnld_flg = i_dwnld_flg;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
		fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",st_i_xchngbk.c_xchng_rmrks);
		fn_userlog(c_ServiceName,"DownLoad Flag Is :%ld:",st_i_xchngbk.l_dwnld_flg);
		fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_xchngbk.d_jiffy);
	}

  /*fn_cpy_ddr(st_i_xchngbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

	st_i_xchngbk.l_bit_flg	=	ptr_st_expl_reqres->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1];			/***	Ver 1.7	***/

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"IN EXER_CONF, the stream number receievd from exchange is:%ld:",st_i_xchngbk.l_bit_flg); 
	}

  st_i_xchngbk.c_oprn_typ = UPDATION_ON_EXCHANGE_RESPONSE;

	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"L31345", LIBMSG, c_err_msg);
    return -1;
  }

	 i_ch_val = fn_call_svc ( c_ServiceName,
                            c_err_msg,
                            &st_i_xchngbk,
                            &st_i_xchngbk,
                            "vw_xchngbook",
                            "vw_xchngbook",
                            sizeof ( st_i_xchngbk ),
                            sizeof ( st_i_xchngbk ),
                            0,
                            "SFO_UPD_XCHNGBK" );
  if ( ( i_ch_val != SUCC_BFR ) &&
       ( i_ch_val != RECORD_ALREADY_PRCSD ) )
  {
    fn_errlog(c_ServiceName,"L31350", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }

	else if ( i_ch_val == RECORD_ALREADY_PRCSD )
	{
		fn_userlog ( c_ServiceName, "Record already processed" );
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return 0;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_XCHNGBK :%s:",c_time);
	}

	i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"L31355", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Commit; :%s:",c_time);
	}

  /*fn_cpy_ddr(st_i_xchngbk.c_rout_crt);*/		/*** Ver 1.1 ***/

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);	/*** Ver 1.1 ***/

	i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_err_msg,
                            &st_i_xchngbk,
                            "vw_xchngbook",
                            sizeof ( st_i_xchngbk ),
                            TPNOREPLY,
                            c_svc_name );
  if ( i_ch_val != 0 )
  {
    fn_errlog(c_ServiceName,"L31360", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_EXREQ_ACK :%s:",c_time);
		fn_userlog(c_ServiceName,"After Complition Of fn_expl_cnfrmtn");
	}

	return 0;
}

/******************************************
int fn_nse_to_eba_tkn ( long int li_tkn,
                        struct vw_contract *ptr_cntrct,
                        char *c_rout_str,
                        char *c_ServiceName,
                        char *c_err_msg )
{
  struct vw_nse_cntrct st_nse_dtls;
  int i_ch_val;

  fn_userlog(c_ServiceName,"Inside Function fn_nse_to_eba_tkn");
  st_nse_dtls.l_token_id =  li_tkn;
  fn_userlog(c_ServiceName,"Token ID Is :%ld:",st_nse_dtls.l_token_id);

 * fn_cpy_ddr(st_nse_dtls.c_rout_crt);*     *** Ver 1.1 ***
  strcpy(st_nse_dtls.c_rout_crt,c_rout_str);  *** Ver 1.1 ***

  st_nse_dtls.c_rqst_typ = NSE_TOKEN_TO_CONTRACT;
  i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_nse_dtls,
                           ptr_cntrct,
                           "vw_nse_cntrct",
                           "vw_contract",
                           sizeof (st_nse_dtls),
                           sizeof (struct vw_contract),
                           0,
                           "SFO_CNT_FOR_ENT" );

  if ( i_ch_val != SUCC_BFR)
  {
    fn_errlog ( c_ServiceName, "L31365", LIBMSG, c_err_msg );
    fn_userlog ( c_ServiceName, "Token id |%ld|", li_tkn );
    return i_ch_val;
  }

	fn_gettime();
	fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_CNT_FOR_ENT :%s:",c_time);
  return 0;
}

*******************************************************************************/

int fn_get_errmsg( int i_error_cd,
									 int i_reason_cd ,
								   char *c_remarks,
                   char *c_ServiceName )
{

EXEC SQL BEGIN DECLARE SECTION;
  varchar c_errcd_msg [ 256 ];
	varchar c_rsn_err_msg [ 256 ];
	char c_err_msg [256];
EXEC SQL END DECLARE SECTION;


	MEMSET(c_errcd_msg);
	MEMSET(c_rsn_err_msg);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName, " ERROR code :%d:, Reason code :%d:", i_error_cd, i_reason_cd );
	}
	
	EXEC SQL
		SELECT FNO_ERR_MSG
		INTO	 :c_errcd_msg
		FROM	 FNO_EXCHNG_ERR_MSG
		WHERE	 FNO_ERR_CD = :i_error_cd;

	if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
	{
		fn_errlog(c_ServiceName, "S31075", SQLMSG, c_err_msg);
		return -1;
	}

	SETNULL(c_errcd_msg);

	EXEC SQL
		SELECT FNO_ERR_MSG
		INTO 	 :c_rsn_err_msg
		FROM   FNO_EXCHNG_ERR_MSG
		WHERE  FNO_ERR_CD = :i_reason_cd;

	if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
  {
    fn_errlog(c_ServiceName, "S31080", SQLMSG, c_err_msg);
    return -1;
  }

	SETNULL(c_rsn_err_msg);

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Error Code Remark Is :%s:",c_errcd_msg.arr);
		fn_userlog(c_ServiceName,"Reason Code Remark Is :%s:",c_rsn_err_msg.arr);
	}

	strcpy(c_remarks,c_errcd_msg.arr);

	strcat(c_remarks,"-");

	strcat(c_remarks,c_rsn_err_msg.arr);

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"Error Message Is :%s:",c_remarks);
	}

	return 0;
}

void fn_gettime()
{
  int i_mili_sec = 0;
  struct timeval tv;
  struct timezone tz;
  struct tm *tm;

  gettimeofday(&tv, &tz);
  tm=localtime(&tv.tv_sec);
  i_mili_sec= tv.tv_usec/1000;
  sprintf(c_time,"%d:%d:%d:%d",tm->tm_hour, tm->tm_min, tm->tm_sec, i_mili_sec);

  return;
}

int fn_ref_to_ord(struct vw_orderbook *ptr_st_orderbook,
                  char *c_ServiceName,
                  char *c_err_msg)
{
  EXEC SQL BEGIN DECLARE SECTION;
    short i_sprd_ord_rfrnc;
    short i_settlor;
    short i_ack_tm;
    short i_prev_ack_tm;
    short i_xchng_ack;
    short i_valid_dt;
  EXEC SQL END DECLARE SECTION;


  if( DEBUG_MSG_LVL_0 )
  {
  	/** fn_userlog( c_ServiceName,"Inside Function fn_ref_to_ord"); *********
    fn_userlog( c_ServiceName,"c_ordr_rfrnc:%s:",ptr_st_orderbook->c_ordr_rfrnc); *** Ver 4.9 ***/
    fn_userlog( c_ServiceName,"fn_ref_to_ord: c_ordr_rfrnc:%s:,Requested Operation :%c:",ptr_st_orderbook->c_ordr_rfrnc,ptr_st_orderbook->c_oprn_typ);  /** ver 4.9 **/
    /*** fn_userlog( c_ServiceName,"Requested Operation :%c:",ptr_st_orderbook->c_oprn_typ); *** ver 4.9 **/
  }

  MEMSET( ptr_st_orderbook->c_ctcl_id);

  EXEC SQL
      SELECT  FOD_CLM_MTCH_ACCNT,
              FOD_CTCL_ID ,
              FOD_CLNT_CTGRY,
              FOD_PIPE_ID,
              FOD_XCHNG_CD,
              FOD_PRDCT_TYP,
              FOD_UNDRLYNG,
              to_char ( FOD_EXPRY_DT, 'dd-Mon-yyyy' ),
              FOD_EXER_TYP,
              FOD_OPT_TYP,
              FOD_STRK_PRC,
              FOD_ORDR_FLW,
              FOD_LMT_MRKT_SL_FLG,
              FOD_DSCLSD_QTY,
              FOD_ORDR_TOT_QTY,
              FOD_LMT_RT,
              FOD_STP_LSS_TGR,
              FOD_ORDR_TYPE,
              to_char ( FOD_ORDR_VALID_DT, 'dd-Mon-yyyy' ),
              to_char ( FOD_TRD_DT, 'dd-Mon-yyyy' ),
              FOD_ORDR_STTS,
              FOD_EXEC_QTY,
              NVL(FOD_EXEC_QTY_DAY,0),
              FOD_CNCL_QTY,
							FOD_EXPRD_QTY,
           		FOD_SPRD_ORDR_REF,
           		FOD_MDFCTN_CNTR,
           		FOD_SETTLOR,
           		FOD_ACK_NMBR,
           		FOD_SPL_FLAG,
           		FOD_INDSTK,
           		to_char ( FOD_ORD_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss' ),
           		to_char ( FOD_LST_RQST_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss' ),
           		FOD_PRO_CLI_IND ,
           		FOD_CHANNEL
     		INTO  :ptr_st_orderbook->c_cln_mtch_accnt,
        	    :ptr_st_orderbook->c_ctcl_id,
           		:ptr_st_orderbook->l_clnt_ctgry,
           		:ptr_st_orderbook->c_pipe_id,
           		:ptr_st_orderbook->c_xchng_cd,
           		:ptr_st_orderbook->c_prd_typ,
           		:ptr_st_orderbook->c_undrlyng,
           		:ptr_st_orderbook->c_expry_dt,
           		:ptr_st_orderbook->c_exrc_typ,
           		:ptr_st_orderbook->c_opt_typ,
           		:ptr_st_orderbook->l_strike_prc,
           		:ptr_st_orderbook->c_ordr_flw,
           		:ptr_st_orderbook->c_slm_flg,
           		:ptr_st_orderbook->l_dsclsd_qty,
           		:ptr_st_orderbook->l_ord_tot_qty,
           		:ptr_st_orderbook->l_ord_lmt_rt,
           		:ptr_st_orderbook->l_stp_lss_tgr,
           		:ptr_st_orderbook->c_ord_typ,
           		:ptr_st_orderbook->c_valid_dt:i_valid_dt,
           		:ptr_st_orderbook->c_trd_dt,
           		:ptr_st_orderbook->c_ordr_stts,
           		:ptr_st_orderbook->l_exctd_qty,
           		:ptr_st_orderbook->l_exctd_qty_day,
           		:ptr_st_orderbook->l_can_qty,
           		:ptr_st_orderbook->l_exprd_qty,
           		:ptr_st_orderbook->c_sprd_ord_rfrnc:i_sprd_ord_rfrnc,
           		:ptr_st_orderbook->l_mdfctn_cntr,
           		:ptr_st_orderbook->c_settlor:i_settlor,
           		:ptr_st_orderbook->c_xchng_ack:i_xchng_ack,
           		:ptr_st_orderbook->c_spl_flg,
           		:ptr_st_orderbook->c_ctgry_indstk,
           		:ptr_st_orderbook->c_ack_tm:i_ack_tm,
           		:ptr_st_orderbook->c_prev_ack_tm:i_prev_ack_tm,
           		:ptr_st_orderbook->c_pro_cli_ind,
           		:ptr_st_orderbook->c_channel
     		FROM  fod_fo_ordr_dtls
     		WHERE fod_ordr_rfrnc = :ptr_st_orderbook->c_ordr_rfrnc;

				if ( SQLCODE != 0 )
  			{
    			fn_errlog ( c_ServiceName, "S31085", SQLMSG, c_err_msg );
    			return -1;
  			}

 	rtrim ( ptr_st_orderbook->c_expry_dt );
 	rtrim ( ptr_st_orderbook->c_ctcl_id );

  if( DEBUG_MSG_LVL_3 )
  {
    fn_userlog(c_ServiceName,"Match Account :%s:",ptr_st_orderbook->c_cln_mtch_accnt);
    fn_userlog(c_ServiceName,"CTCL ID :%s:",ptr_st_orderbook->c_ctcl_id);
    fn_userlog(c_ServiceName,"CLient Category :%ld:",ptr_st_orderbook->l_clnt_ctgry);
    fn_userlog(c_ServiceName,"Pipe Id :%s:",ptr_st_orderbook->c_pipe_id);
    fn_userlog(c_ServiceName,"Exchange Code Is :%s:",ptr_st_orderbook->c_xchng_cd);
    fn_userlog(c_ServiceName,"Product Typ Is :%c:",ptr_st_orderbook->c_prd_typ);
    fn_userlog(c_ServiceName,"Underlying Is :%s:",ptr_st_orderbook->c_undrlyng);
    fn_userlog(c_ServiceName,"Expiry Date Is :%s:",ptr_st_orderbook->c_expry_dt);
    fn_userlog(c_ServiceName,"Exercise Type Is :%c:",ptr_st_orderbook->c_exrc_typ);
    fn_userlog(c_ServiceName,"Option Type Is :%c:",ptr_st_orderbook->c_opt_typ);
    fn_userlog(c_ServiceName,"Strike Price Is :%ld:",ptr_st_orderbook->l_strike_prc);
    fn_userlog(c_ServiceName,"Order Flow Is :%c:",ptr_st_orderbook->c_ordr_flw);
    fn_userlog(c_ServiceName,"LMT/MKT/SL Flag Is :%c:",ptr_st_orderbook->c_slm_flg);
    fn_userlog(c_ServiceName,"Disclosed Qty Is :%ld:",ptr_st_orderbook->l_dsclsd_qty);
    fn_userlog(c_ServiceName,"Total Order Qty Is :%ld:",ptr_st_orderbook->l_ord_tot_qty);
    fn_userlog(c_ServiceName,"Order Limit Rate :%ld:",ptr_st_orderbook->l_ord_lmt_rt);
    fn_userlog(c_ServiceName,"SLTP Is :%ld:",ptr_st_orderbook->l_stp_lss_tgr);
    fn_userlog(c_ServiceName,"Order Type Is :%c:",ptr_st_orderbook->c_ord_typ);
    fn_userlog(c_ServiceName,"Valid Date Is :%s:",ptr_st_orderbook->c_valid_dt);
    fn_userlog(c_ServiceName,"Trade Date Is :%s:",ptr_st_orderbook->c_trd_dt);
    fn_userlog(c_ServiceName,"Order status Is :%c:",ptr_st_orderbook->c_ordr_stts);
    fn_userlog(c_ServiceName,"Executed Qty Is :%ld:", ptr_st_orderbook->l_exctd_qty);
    fn_userlog(c_ServiceName,"Executed Qty Per Day :%ld:",ptr_st_orderbook->l_exctd_qty_day);
    fn_userlog(c_ServiceName,"Cancelled Qty Is :%ld:",ptr_st_orderbook->l_can_qty);
    fn_userlog(c_ServiceName,"Expired Qty Is :%ld:",ptr_st_orderbook->l_exprd_qty);
    fn_userlog(c_ServiceName,"Sprd Ord Rfrnc Is ;%s:",ptr_st_orderbook->c_sprd_ord_rfrnc);
    fn_userlog(c_ServiceName,"Modification Counter Is :%ld:",ptr_st_orderbook->l_mdfctn_cntr);
    fn_userlog(c_ServiceName,"Settlor Is :%s:",ptr_st_orderbook->c_settlor);
    fn_userlog(c_ServiceName,"Exchange Ack Is ;%s:",ptr_st_orderbook->c_xchng_ack);
    fn_userlog(c_ServiceName,"SPL Flag is :%c:",ptr_st_orderbook->c_spl_flg);
    fn_userlog(c_ServiceName,"Category IND/STK Is :%c:",ptr_st_orderbook->c_ctgry_indstk);
    fn_userlog(c_ServiceName,"Ack Time Is :%s:",ptr_st_orderbook->c_ack_tm);
    fn_userlog(c_ServiceName,"Prv Ack Time Is :%s:",ptr_st_orderbook->c_prev_ack_tm);
    fn_userlog(c_ServiceName,"Pro Cli Ind :%c:",ptr_st_orderbook->c_pro_cli_ind);
    fn_userlog(c_ServiceName,"Channel is :%s:",ptr_st_orderbook->c_channel);
  }

  return 0;
}

int fn_seq_to_omd(struct vw_xchngbook *ptr_st_xchngbook,
                  char *c_ServiceName,
                  char *c_err_msg)
{
  EXEC SQL BEGIN DECLARE SECTION;
    short i_qt_tm;
    short i_frwrd_tm;
    short i_rms_prcs_flg;
    short i_ors_msg_typ;
    short i_ack_tm;
    short i_xchng_rmrks;
    short i_xchng_can_qty;
    varchar c_rmrks [ 256 ];
  EXEC SQL END DECLARE SECTION;

	if(DEBUG_MSG_LVL_0)
	{
    /*** Commented in ver 4.9***	fn_userlog( c_ServiceName,"Function fn_seq_to_omd");
  	fn_userlog( c_ServiceName,"Exchnage Code Is :%s:",ptr_st_xchngbook->c_xchng_cd);
  	fn_userlog( c_ServiceName,"Pipe ID Is :%s:",ptr_st_xchngbook->c_pipe_id);
  	fn_userlog( c_ServiceName,"Order Sequence Is :%ld:",ptr_st_xchngbook->l_ord_seq); ***** Ver 4.9 ***/
    fn_userlog( c_ServiceName,"fn_seq_to_omd: Exchnage Code Is :%s: Pipe ID Is :%s: Order Sequence Is :%ld::",ptr_st_xchngbook->c_xchng_cd,ptr_st_xchngbook->c_pipe_id,ptr_st_xchngbook->l_ord_seq);  /** Ver 4.9 **/
	}


   EXEC SQL
     SELECT  FXB_ORDR_RFRNC,
             FXB_LMT_MRKT_SL_FLG,
             FXB_DSCLSD_QTY,
             FXB_ORDR_TOT_QTY,
             FXB_LMT_RT,
             FXB_STP_LSS_TGR,
             FXB_MDFCTN_CNTR,
             to_char( FXB_ORDR_VALID_DT, 'dd-mon-yyyy' ),
             FXB_ORDR_TYPE,
             FXB_SPRD_ORD_IND,
             FXB_RQST_TYP,
             FXB_QUOTE,
             to_char( FXB_QT_TM, 'dd-mon-yyyy hh24:mi:ss' ),
             to_char( FXB_RQST_TM, 'dd-mon-yyyy hh24:mi:ss' ),
             to_char( FXB_FRWD_TM, 'dd-mon-yyyy hh24:mi:ss' ),
             FXB_PLCD_STTS,
			       FXB_RMS_PRCSD_FLG,
             FXB_ORS_MSG_TYP,
             to_char ( FXB_ACK_TM, 'dd-mon-yyyy hh24:mi:ss' ),
             FXB_XCHNG_RMRKS,
             FXB_EX_ORDR_TYP,
             FXB_XCHNG_CNCLD_QTY,
             FXB_SPL_FLAG,
             FXB_ORDR_SQNC
     INTO    :ptr_st_xchngbook->c_ordr_rfrnc,
             :ptr_st_xchngbook->c_slm_flg,
             :ptr_st_xchngbook->l_dsclsd_qty,
             :ptr_st_xchngbook->l_ord_tot_qty,
             :ptr_st_xchngbook->l_ord_lmt_rt,
             :ptr_st_xchngbook->l_stp_lss_tgr,
             :ptr_st_xchngbook->l_mdfctn_cntr,
             :ptr_st_xchngbook->c_valid_dt,
             :ptr_st_xchngbook->c_ord_typ,
             :ptr_st_xchngbook->c_sprd_ord_ind,
             :ptr_st_xchngbook->c_req_typ,
             :ptr_st_xchngbook->l_quote,
             :ptr_st_xchngbook->c_qt_tm:i_qt_tm,
             :ptr_st_xchngbook->c_rqst_tm,
             :ptr_st_xchngbook->c_frwrd_tm:i_frwrd_tm,
             :ptr_st_xchngbook->c_plcd_stts,
             :ptr_st_xchngbook->c_rms_prcsd_flg:i_rms_prcs_flg,
             :ptr_st_xchngbook->l_ors_msg_typ:i_ors_msg_typ,
             :ptr_st_xchngbook->c_ack_tm:i_ack_tm,
             :c_rmrks:i_xchng_rmrks,
             :ptr_st_xchngbook->c_ex_ordr_typ,
             :ptr_st_xchngbook->l_xchng_can_qty:i_xchng_can_qty,
             :ptr_st_xchngbook->c_spl_flg,
             :ptr_st_xchngbook->l_ord_seq
     FROM    FXB_FO_XCHNG_BOOK
     WHERE   FXB_XCHNG_CD  = :ptr_st_xchngbook->c_xchng_cd
     AND     FXB_PIPE_ID   = :ptr_st_xchngbook->c_pipe_id
     AND     FXB_ORDR_SQNC = :ptr_st_xchngbook->l_ord_seq
     AND     FXB_MOD_TRD_DT =  to_date(:ptr_st_xchngbook->c_mod_trd_dt,'dd-Mon-yyyy');

  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31090", SQLMSG, c_err_msg );
    return -1;

  }

  if ( i_xchng_rmrks == -1 )
  {
    memset ( ptr_st_xchngbook->c_xchng_rmrks, 0,sizeof ( ptr_st_xchngbook->c_xchng_rmrks ) );
  }
	else
  {
    SETNULL ( c_rmrks );
    strcpy ( ptr_st_xchngbook->c_xchng_rmrks, (char *)c_rmrks.arr );
  }

  return 0;
}

int fn_upd_xchngbk_erl( struct vw_xchngbook *ptr_st_xchngbook,
										char *c_lst_act_ref,		/*** Ver 6.1 ***/
                    char *c_ServiceName,
                    char *c_err_msg)
{

  EXEC SQL BEGIN DECLARE SECTION;
    varchar c_xchng_rmrks [ 256 ];
  EXEC SQL END DECLARE SECTION;


  int i_rec_exists = 0;

	if(DEBUG_MSG_LVL_0)
	{
  	/** fn_userlog(c_ServiceName,"Inside Function fn_upd_xchngbk_erl"); ************
  	fn_userlog(c_ServiceName,"Order Reference Is :%s:",ptr_st_xchngbook->c_ordr_rfrnc); Ver 4.9 *********/
    fn_userlog(c_ServiceName,"fn_upd_xchngbk_erl:Order Reference Is :%s:",ptr_st_xchngbook->c_ordr_rfrnc); /** Ver 4.9 **/
	}

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Modification Counter Is :%ld:",ptr_st_xchngbook->l_mdfctn_cntr);
  	fn_userlog(c_ServiceName,"Placed Status Is ;%c:",ptr_st_xchngbook->c_plcd_stts);
  	fn_userlog(c_ServiceName,"RMS Processed Flag Is :%c:",ptr_st_xchngbook->c_rms_prcsd_flg);
  	fn_userlog(c_ServiceName,"ORS Msg Typ Is :%ld:",ptr_st_xchngbook->l_ors_msg_typ);
  	fn_userlog(c_ServiceName,"Ack Time Is :%s:",ptr_st_xchngbook->c_ack_tm);
		fn_userlog(c_ServiceName,"Stream No. Is :%ld:",ptr_st_xchngbook->l_bit_flg);
		fn_userlog(c_ServiceName,"Last Activity Ref inside fn_upd_xchngbk_erl is :%s:",c_lst_act_ref);    /*** Ver 6.1 ***/
	}

  switch ( ptr_st_xchngbook->c_oprn_typ )
  {
    case UPDATION_ON_EXCHANGE_RESPONSE:

      if ( ptr_st_xchngbook->l_dwnld_flg == DOWNLOAD )
      {
        EXEC SQL
          SELECT 1
          INTO :i_rec_exists
          FROM FXB_FO_XCHNG_BOOK
          WHERE FXB_JIFFY 	= :ptr_st_xchngbook->d_jiffy
          AND FXB_XCHNG_CD 	= :ptr_st_xchngbook->c_xchng_cd
          AND FXB_PIPE_ID 	= :ptr_st_xchngbook->c_pipe_id
          AND FXB_ORDR_RFRNC    = :ptr_st_xchngbook->c_ordr_rfrnc;  /** Ver 6.7 ***/
					/*** AND FXB_STREAM_NO	=	:ptr_st_xchngbook->l_bit_flg;			***	Ver	1.8	*** Commented in Ver 6.6 ***/

        if ( ( SQLCODE != 0 ) && (SQLCODE != NO_DATA_FOUND) )
        {
          fn_errlog ( c_ServiceName, "S31095", SQLMSG, c_err_msg );
          return -1;
        }
        if ( i_rec_exists == 1 )
        {
          strcpy( c_err_msg, "Record already Processed" );
          return RECORD_ALREADY_PRCSD;
        }

      }

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"ptr_st_xchngbook->c_xchng_rmrks :%s:",ptr_st_xchngbook->c_xchng_rmrks);
		}

		/*strcpy ( (char *)c_xchng_rmrks.arr,ptr_st_xchngbook->c_xchng_rmrks );
    rtrim ( c_xchng_rmrks.arr );
    SETLEN ( c_xchng_rmrks );*/


    EXEC SQL
      UPDATE  FXB_FO_XCHNG_BOOK
      SET  FXB_PLCD_STTS       = :ptr_st_xchngbook->c_plcd_stts,
           FXB_RMS_PRCSD_FLG   = :ptr_st_xchngbook->c_rms_prcsd_flg,
           FXB_ORS_MSG_TYP     = :ptr_st_xchngbook->l_ors_msg_typ,
           FXB_ACK_TM          = to_date( :ptr_st_xchngbook->c_ack_tm,'DD-Mon-yyyy hh24:mi:ss' ),
           FXB_XCHNG_RMRKS     = rtrim(fxb_xchng_rmrks)||:ptr_st_xchngbook->c_xchng_rmrks,
           FXB_JIFFY           = :ptr_st_xchngbook->d_jiffy,
           FXB_STREAM_NO       = :ptr_st_xchngbook->l_bit_flg,            /**Ver 1.7 **/
					 FXB_LST_ACT_REF     = :c_lst_act_ref														/** Ver 6.1 **/
      WHERE  FXB_ORDR_RFRNC    = :ptr_st_xchngbook->c_ordr_rfrnc
      AND  FXB_MDFCTN_CNTR     = :ptr_st_xchngbook->l_mdfctn_cntr;

    break;

    case UPDATE_PLACED_STTS:

      i_rec_exists = 0;
      if ( ptr_st_xchngbook->l_dwnld_flg == DOWNLOAD )
      {
        EXEC SQL
          SELECT 1
          INTO :i_rec_exists
          FROM dual
          WHERE EXISTS ( SELECT FXB_ORDR_RFRNC
                         FROM FXB_FO_XCHNG_BOOK
                         WHERE FXB_ORDR_RFRNC = :ptr_st_xchngbook->c_ordr_rfrnc
                         AND   FXB_JIFFY >= :ptr_st_xchngbook->d_jiffy); 
												 /**** AND 	FXB_STREAM_NO = :ptr_st_xchngbook->l_bit_flg);     ***  Ver 1.8 *** Commented in Ver 6.6 ***/
        if ( ( SQLCODE != 0 ) && (SQLCODE != NO_DATA_FOUND) )
        {
          fn_errlog ( c_ServiceName, "S31100",SQLMSG, c_err_msg );
          return -1;
        }

        if ( i_rec_exists == 1 )
        {
          strcpy(c_err_msg, "Record already Processed" );
          return RECORD_ALREADY_PRCSD;
				}

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"The Order reference is :%s:",ptr_st_xchngbook->c_ordr_rfrnc);
				}

        EXEC SQL
          SELECT FOD_MDFCTN_CNTR
          INTO :ptr_st_xchngbook->l_mdfctn_cntr
          FROM FOD_FO_ORDR_DTLS
          WHERE FOD_ORDR_RFRNC = :ptr_st_xchngbook->c_ordr_rfrnc;

        if ( SQLCODE != 0 )
        {
          if (SQLCODE == NO_DATA_FOUND)
          {
            EXEC SQL
              SELECT FEB_MDFCTN_CNTR
              INTO :ptr_st_xchngbook->l_mdfctn_cntr
              FROM FEB_FO_EXRC_RQST_BOOK
              WHERE FEB_EXRC_RFRNC_NO = :ptr_st_xchngbook->c_ordr_rfrnc;

            if ( SQLCODE != 0 )
            {
              fn_errlog ( c_ServiceName, "S31105",SQLMSG, c_err_msg );
              fn_userlog(c_ServiceName,"Failed for Order reference :%s:",ptr_st_xchngbook->c_ordr_rfrnc);
              return -1;
            }
          }
          else
          {
              fn_errlog ( c_ServiceName, "S31110",SQLMSG, c_err_msg );
              fn_userlog(c_ServiceName,"Failed for Order reference :%s:",ptr_st_xchngbook->c_ordr_rfrnc);
              return -1;
            }
         }
      }

    EXEC SQL
      UPDATE  FXB_FO_XCHNG_BOOK
      SET  FXB_PLCD_STTS   = 'E',
           FXB_JIFFY       = :ptr_st_xchngbook->d_jiffy,
           FXB_STREAM_NO   = :ptr_st_xchngbook->l_bit_flg      /**Ver 1.7 **/
      WHERE  FXB_ORDR_RFRNC  = :ptr_st_xchngbook->c_ordr_rfrnc
      AND  FXB_MDFCTN_CNTR = :ptr_st_xchngbook->l_mdfctn_cntr;

	break;

	/*** Ver 6.1 Starts ***/
	case INSERTION_ON_ORDER_FTE:
      i_rec_exists = 0;
      if ( ptr_st_xchngbook->l_dwnld_flg == DOWNLOAD )
      {

       if( strcmp(ptr_st_xchngbook->c_xchng_cd,"NFO") == 0 )
       {
        if ( ptr_st_xchngbook->l_ors_msg_typ == ORS_2L_ORD_CNCL ||  ptr_st_xchngbook->l_ors_msg_typ ==  ORS_3L_ORD_CNCL || ptr_st_xchngbook->l_ors_msg_typ == ORS_ORD_TRG )
        {
          if( DEBUG_MSG_LVL_3 )
          {
            fn_userlog(c_ServiceName,"Inside DOWNLOAD For ORS_2L_ORD_CNCL & ORS_3L_ORD_CNCL");
            fn_userlog(c_ServiceName,"Jiffy Is :%lf:",ptr_st_xchngbook->d_jiffy);
            fn_userlog(c_ServiceName,"Order Sequence Is :%ld:",ptr_st_xchngbook->l_ord_seq);
          }

          EXEC SQL
            SELECT 1
            INTO :i_rec_exists
            FROM FXB_FO_XCHNG_BOOK
            WHERE FXB_JIFFY = :ptr_st_xchngbook->d_jiffy
            AND FXB_XCHNG_CD = :ptr_st_xchngbook->c_xchng_cd
            AND FXB_PIPE_ID = :ptr_st_xchngbook->c_pipe_id
            AND FXB_ORDR_SQNC = :ptr_st_xchngbook->l_ord_seq;
            /*** AND FXB_STREAM_NO = :ptr_st_xchngbook->l_bit_flg; *** Ver 6.6 ***/

          if ( ( SQLCODE != 0 ) && (SQLCODE != NO_DATA_FOUND) )
          {
            fn_errlog ( c_ServiceName, "S31115",SQLMSG, c_err_msg );
            tpfree ( ( char * ) ptr_st_xchngbook );
						return -1;
          }

				}
        else
        {
          EXEC SQL
            SELECT 1
            INTO :i_rec_exists
            FROM FXB_FO_XCHNG_BOOK
            WHERE FXB_JIFFY = :ptr_st_xchngbook->d_jiffy
            AND FXB_XCHNG_CD = :ptr_st_xchngbook->c_xchng_cd
            AND FXB_PIPE_ID = :ptr_st_xchngbook->c_pipe_id
            AND FXB_ORDR_RFRNC = :ptr_st_xchngbook->c_ordr_rfrnc;  /** Ver 6.7 **/
            /** AND FXB_STREAM_NO = :ptr_st_xchngbook->l_bit_flg **** Ver 6.6 ***/
          if ( ( SQLCODE != 0 ) && (SQLCODE != NO_DATA_FOUND) )
          {
            fn_errlog ( c_ServiceName, "S31120",SQLMSG, c_err_msg );
            tpfree ( ( char * ) ptr_st_xchngbook );
						return -1;
          }
        }

       }

			 if ( i_rec_exists == 1 )
       {
         tpfree ( ( char * ) ptr_st_xchngbook );
         strcpy( c_err_msg, "Record already Processed" );
				 return RECORD_ALREADY_PRCSD;
       }

      }

			 EXEC SQL
          INSERT INTO FXB_FO_XCHNG_BOOK
          (
            FXB_XCHNG_CD,
            FXB_ORDR_RFRNC,
            FXB_MDFCTN_CNTR,
            FXB_PIPE_ID,
            FXB_MOD_TRD_DT,
            FXB_ORDR_SQNC,
            FXB_PLCD_STTS,
            FXB_RMS_PRCSD_FLG,
            FXB_ORS_MSG_TYP,
            FXB_ACK_TM,
            FXB_XCHNG_RMRKS,
            FXB_RQST_TYP,
            FXB_JIFFY,
            FXB_ORDR_TOT_QTY,
            FXB_LMT_RT,
            FXB_STREAM_NO,
            FXB_XCHNG_CNCLD_QTY,
						FXB_LST_ACT_REF
          )
          VALUES
          (
            :ptr_st_xchngbook->c_xchng_cd,
            :ptr_st_xchngbook->c_ordr_rfrnc,
            :ptr_st_xchngbook->l_mdfctn_cntr,
            :ptr_st_xchngbook->c_pipe_id,
            to_date (:ptr_st_xchngbook->c_mod_trd_dt,'DD-Mon-yyyy'),
            0,
            :ptr_st_xchngbook->c_plcd_stts,
            :ptr_st_xchngbook->c_rms_prcsd_flg,
            :ptr_st_xchngbook->l_ors_msg_typ,
            to_date (:ptr_st_xchngbook->c_ack_tm,'DD-Mon-yyyy hh24:mi:ss'),
            :ptr_st_xchngbook->c_xchng_rmrks,
            'T',
						:ptr_st_xchngbook->d_jiffy,
            :ptr_st_xchngbook->l_ord_tot_qty,
            :ptr_st_xchngbook->l_ord_lmt_rt,
            :ptr_st_xchngbook->l_bit_flg,
            :ptr_st_xchngbook->l_xchng_can_qty,
						:c_lst_act_ref
          );

      break;

		/*** Ver 6.1 Ends ***/
  default :

    strcpy( c_err_msg, "Invalid Operation Type" );
    fn_errlog ( c_ServiceName, "S31125", DEFMSG, c_err_msg );
    return -1;
    break;

  }

  if(SQLCODE != 0)
  {
    fn_errlog ( c_ServiceName, "S31130", SQLMSG, c_err_msg );
    return -1;
  }

  return 0;
}

int fn_upd_ordrbk(struct vw_orderbook *ptr_st_orderbook,
                  char *c_ServiceName,
                  char *c_err_msg)
{
	if(DEBUG_MSG_LVL_3)  /** ver 4.9 ***/
	{
  	fn_userlog( c_ServiceName,"Function UPD_ORDRBK");
	}

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog ( c_ServiceName,"c_ordr_rfrnc:%s:", ptr_st_orderbook->c_ordr_rfrnc );
    fn_userlog ( c_ServiceName,"c_xchng_ack:%s:",ptr_st_orderbook->c_xchng_ack);
  }

  EXEC SQL
        UPDATE  FOD_FO_ORDR_DTLS
        SET     FOD_ACK_NMBR  = :ptr_st_orderbook->c_xchng_ack
        WHERE   FOD_ORDR_RFRNC= :ptr_st_orderbook->c_ordr_rfrnc;

  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31135", SQLMSG, c_err_msg );
    return -1;
  }

  return 0;
}

int fn_ack_to_ord ( struct vw_orderbook *ptr_st_orderbook,
                    char *c_ServiceName,
										char *c_usr_id,			/***	Ver	2.0	***/
                    char *c_err_msg)
{
 
	varchar c_user_id[16];			/***	Ver	2.0	***/

   EXEC SQL BEGIN DECLARE SECTION;
    short  i_settlor;
    short  i_sprd_ord_ref;
    short  i_ack_tm;
    short  i_prev_ack_tm;
  EXEC SQL END DECLARE SECTION;

  if (DEBUG_MSG_LVL_0)
  {
    /***** fn_userlog ( c_ServiceName,"Inside Function fn_ack_to_ord");  **************
    fn_userlog ( c_ServiceName,"c_xchng_ack:%s:", ptr_st_orderbook->c_xchng_ack );
    fn_userlog ( c_ServiceName,"c_pipe_id:%s:", ptr_st_orderbook->c_pipe_id ); *** Ver 4.9 ***/
    fn_userlog ( c_ServiceName,"fn_ack_to_ord: c_xchng_ack:%s: c_pipe_id:%s:", ptr_st_orderbook->c_xchng_ack,ptr_st_orderbook->c_pipe_id);  /**** Ver 4.9 ***/
  }

  EXEC SQL
      SELECT  FOD_CLM_MTCH_ACCNT,
              FOD_CLNT_CTGRY,
              FOD_ORDR_RFRNC,
              FOD_XCHNG_CD,
              FOD_PRDCT_TYP,
              FOD_UNDRLYNG,
              to_char ( FOD_EXPRY_DT, 'dd-Mon-yyyy' ),
              FOD_EXER_TYP,
              FOD_OPT_TYP,
              FOD_STRK_PRC,
              FOD_ORDR_FLW,
              FOD_LMT_MRKT_SL_FLG,
              NVL( FOD_DSCLSD_QTY, 0 ),
              NVL ( FOD_ORDR_TOT_QTY, 0 ),
              NVL( FOD_LMT_RT, 0 ),
              FOD_STP_LSS_TGR,
              FOD_ORDR_TYPE,
              to_char ( FOD_ORDR_VALID_DT, 'dd-Mon-yyyy' ),
              to_char ( FOD_TRD_DT, 'dd-Mon-yyyy' ),
              FOD_ORDR_STTS,
              NVL( FOD_EXEC_QTY, 0 ),
              NVL( FOD_EXEC_QTY_DAY, 0 ),
              NVL( FOD_CNCL_QTY, 0 ),
              NVL( FOD_EXPRD_QTY, 0 ),
              FOD_SPRD_ORDR_REF,
              FOD_MDFCTN_CNTR,
              FOD_SETTLOR,
              FOD_SPL_FLAG,
              FOD_INDSTK,
              to_char ( FOD_ORD_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss' ),
              to_char ( FOD_LST_RQST_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss' ),
      			  NVL( trim(FOD_USR_ID), '*' )	
      INTO    :ptr_st_orderbook->c_cln_mtch_accnt,
              :ptr_st_orderbook->l_clnt_ctgry,
              :ptr_st_orderbook->c_ordr_rfrnc,
              :ptr_st_orderbook->c_xchng_cd,
              :ptr_st_orderbook->c_prd_typ,
              :ptr_st_orderbook->c_undrlyng,
              :ptr_st_orderbook->c_expry_dt,
              :ptr_st_orderbook->c_exrc_typ,
              :ptr_st_orderbook->c_opt_typ,
              :ptr_st_orderbook->l_strike_prc,
              :ptr_st_orderbook->c_ordr_flw,
              :ptr_st_orderbook->c_slm_flg,
              :ptr_st_orderbook->l_dsclsd_qty,
              :ptr_st_orderbook->l_ord_tot_qty,
              :ptr_st_orderbook->l_ord_lmt_rt,
              :ptr_st_orderbook->l_stp_lss_tgr,
              :ptr_st_orderbook->c_ord_typ,
              :ptr_st_orderbook->c_valid_dt,
              :ptr_st_orderbook->c_trd_dt,
              :ptr_st_orderbook->c_ordr_stts,
              :ptr_st_orderbook->l_exctd_qty,
              :ptr_st_orderbook->l_exctd_qty_day,
              :ptr_st_orderbook->l_can_qty,
              :ptr_st_orderbook->l_exprd_qty,
              :ptr_st_orderbook->c_sprd_ord_rfrnc :i_sprd_ord_ref,
              :ptr_st_orderbook->l_mdfctn_cntr,
              :ptr_st_orderbook->c_settlor :i_settlor,
              :ptr_st_orderbook->c_req_typ,
              :ptr_st_orderbook->c_ctgry_indstk,
              :ptr_st_orderbook->c_ack_tm :i_ack_tm,
              :ptr_st_orderbook->c_prev_ack_tm :i_prev_ack_tm,
							:c_user_id									/***	Ver	2.0	***/
     /***     :ptr_st_orderbook->c_user_id			Commented In Ver	2.0			***/
      FROM    FOD_FO_ORDR_DTLS
      WHERE   FOD_ACK_NMBR = :ptr_st_orderbook->c_xchng_ack
      AND     FOD_PIPE_ID  = :ptr_st_orderbook->c_pipe_id;

  	if (  SQLCODE != 0 )
  	{
    	if ( SQLCODE == TOO_MANY_ROWS_FETCHED )
    	{
      	return TOO_MANY_ROWS_FETCHED;
    	}

    	fn_errlog ( c_ServiceName, "S31140", SQLMSG, c_err_msg );
			return -1;
  	}



	/***	Commented In Ver 2.0
	rtrim(ptr_st_orderbook->c_user_id);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"User id right trimmed to :%s:",ptr_st_orderbook->c_user_id);
	}
	******************************************/


  rtrim ( ptr_st_orderbook->c_expry_dt );

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName, "Values Inside Function fn_ack_to_ord");
		fn_userlog ( c_ServiceName, "Match Account Is :%s:",ptr_st_orderbook->c_cln_mtch_accnt);
		fn_userlog ( c_ServiceName, "CLNT Ctgry :%ld:",ptr_st_orderbook->l_clnt_ctgry);
		fn_userlog ( c_ServiceName, "Order Reference Is :%s:",ptr_st_orderbook->c_ordr_rfrnc);
		fn_userlog ( c_ServiceName, "Exchnage Code Is :%s:",ptr_st_orderbook->c_xchng_cd);
		fn_userlog ( c_ServiceName, "Product Type Is :%c:",ptr_st_orderbook->c_prd_typ);
		fn_userlog ( c_ServiceName, "Underlying Is :%s:",ptr_st_orderbook->c_undrlyng);
		fn_userlog ( c_ServiceName, "Expiry Date Is :%s:",ptr_st_orderbook->c_expry_dt);
		fn_userlog ( c_ServiceName, "Exercise Type Is :%c:",ptr_st_orderbook->c_exrc_typ);
		fn_userlog ( c_ServiceName, "Option Type Is :%c:",ptr_st_orderbook->c_opt_typ);
		fn_userlog ( c_ServiceName, "Strike Price Is :%ld:",ptr_st_orderbook->l_strike_prc);
		fn_userlog ( c_ServiceName, "Order Flow Is :%c:",ptr_st_orderbook->c_ordr_flw);
		fn_userlog ( c_ServiceName, "SLM Flag Is :%c:",ptr_st_orderbook->c_slm_flg);
		fn_userlog ( c_ServiceName, "Disclosed Qty Is :%ld:",ptr_st_orderbook->l_dsclsd_qty);
		fn_userlog ( c_ServiceName, "Total Order Qty Is :%ld:",ptr_st_orderbook->l_ord_tot_qty);
		fn_userlog ( c_ServiceName, "Order Limit Rate Is :%ld:",ptr_st_orderbook->l_ord_lmt_rt);
		fn_userlog ( c_ServiceName, "SLTP is :%ld:",ptr_st_orderbook->l_stp_lss_tgr);
		fn_userlog ( c_ServiceName, "Order Type Is :%c:",ptr_st_orderbook->c_ord_typ);
		fn_userlog ( c_ServiceName, "Valid Date Is :%s:",ptr_st_orderbook->c_valid_dt);
		fn_userlog ( c_ServiceName, "Trade date IS :%s:",ptr_st_orderbook->c_trd_dt);
		fn_userlog ( c_ServiceName, "Order Status Is :%c:",ptr_st_orderbook->c_ordr_stts);
		fn_userlog ( c_ServiceName, "Executed Qty Is :%ld:",ptr_st_orderbook->l_exctd_qty);
		fn_userlog ( c_ServiceName, "Executed Qty Day Is :%ld:",ptr_st_orderbook->l_exctd_qty_day);
		fn_userlog ( c_ServiceName, "Cancel Qty is :%ld:",ptr_st_orderbook->l_can_qty);
		fn_userlog ( c_ServiceName, "Expired Qty Is :%ld:",ptr_st_orderbook->l_exprd_qty);
		fn_userlog ( c_ServiceName, "Sprd Ord Rfrnc Is :%s:",ptr_st_orderbook->c_sprd_ord_rfrnc);
		fn_userlog ( c_ServiceName, "Modification Counter Is :%ld:",ptr_st_orderbook->l_mdfctn_cntr);
		fn_userlog ( c_ServiceName, "Settlor Is :%c:",ptr_st_orderbook->c_settlor );
		fn_userlog ( c_ServiceName, "Req Type Is :%c:",ptr_st_orderbook->c_req_typ);
		fn_userlog ( c_ServiceName, "IND/STK :%c:",ptr_st_orderbook->c_ctgry_indstk);
		fn_userlog ( c_ServiceName, "Ack Time Is :%s:",ptr_st_orderbook->c_ack_tm );
		fn_userlog ( c_ServiceName, "Prv Ack Time Is :%s:",ptr_st_orderbook->c_prev_ack_tm );
  	fn_userlog ( c_ServiceName, "OUTSIDE c_usr_id :%s:", c_user_id.arr);
	}

	SETNULL(c_user_id);									/***	Ver	2.0	***/
	strcpy(c_usr_id,c_user_id.arr);			/***	Ver	2.0	***/

	/***	Commented In Ver 2.0
  if ( ptr_st_orderbook->c_user_id[0]=='*' )
  {
    strcpy( ptr_st_orderbook->c_user_id, "" );
    fn_userlog ( c_ServiceName, "INSIDE c_user_id:%s:", ptr_st_orderbook->c_user_id );
  }
	**************************/

  return 0;
}

/******************************************************************************/
/*  To update details of either trader message or general message based on    */
/*  message id as input parameter.                                            */
/*  INPUT PARAMETERS                                                          */
/*      st_temp_msg  - Trader / Geberal message structure                     */
/*      i_msg_id     - Type of message (TRADER_MSG / GENERAL_MSG)             */
/*      c_err_msg    - Error message to be returned in case of error          */
/*  OUTPUT PARAMETERS                                                         */
/*      void                                                                  */
/******************************************************************************/

int fn_intgnrltrd_msg( struct st_trader_int_msg *ptr_trdr_int_msg,								/***	Ver 1.4	***/
         		           char *c_xchng_cd,
            	         char *c_pipe_id,
               		     char *c_ServiceName,
                  		 char *c_errmsg)
{
  struct vw_mkt_msg st_msg;

  int i_ch_val;
  int i_trnsctn;
	int iLen;             /*** Added in ver 2.6 ***/
  char *cPtr;           /*** Added in ver 2.6 ***/
  char c_undrlyng[20];  /*** Added in ver 2.6 ***/
  char c_stock_cd[7];   /*** Added in ver 2.6 ***/

  char c_trd_msg[300];
  char c_brk_stts= 'A'; /*** Initialized in ver 2.6 Initializd to 'A' in ver 2.9 ***/
	char c_closeout_flag = 'N'; /*** ver 2.9 ***/
	char c_message_typ   = 'S'; /*** ver 2.9 ***/
	char c_und_brkr_stts = 'N'; /*** ver 2.9 ***/
  char c_raise_trigger_type [30];

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Inside Function fn_gnrltrd_msg");
  	fn_userlog(c_ServiceName,"Exchange code is :%s:",c_xchng_cd);
  	fn_userlog(c_ServiceName,"Exchange Time Stamp Is ;%ld:",ptr_trdr_int_msg->st_hdr.li_log_time);
  	fn_userlog(c_ServiceName,"Resc lib Message Is :%s:",st_msg.c_msg);
	}

  fn_long_to_timearr( st_msg.c_tm_stmp,ptr_trdr_int_msg->st_hdr.li_log_time);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"After Conversion Into EBA Format Time Stamp Is ;%s:",st_msg.c_tm_stmp);
	}

  fn_nsetoors_char( st_msg.c_msg,
                    LEN_MSG,
                    ptr_trdr_int_msg->c_broadcast_message,
                    LEN_BROADCAST_MESSAGE);

  sprintf( c_trd_msg, "|%s| %s|",st_msg.c_tm_stmp,st_msg.c_msg);

  fn_pst_trg ( c_ServiceName, "TRG_ORS_CON", c_trd_msg,c_pipe_id );

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"THE MSG is :%s:",st_msg.c_msg);
	}

  st_msg.c_msg_id = TRADER_MSG;

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Message ID is :%c:",st_msg.c_msg_id);
	}
	
	/***	Commented In Ver 1.9
	sprintf(st_msg.c_brkr_id,"%d",ptr_trdr_int_msg->si_trader_id);
	**********************/

	sprintf(st_msg.c_brkr_id,"%ld",ptr_trdr_int_msg->li_trader_id);	/***	Ver	1.9	Trader Id Data Type Changed From Short Int To Long	***/

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Broker ID Is :%s:",st_msg.c_brkr_id);
	}

  strcpy( st_msg.c_xchng_cd,c_xchng_cd);

	if(DEBUG_MSG_LVL_0)  /*** Ver 2.6 logs put in to 0 debug ***/
	{
  	fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_msg.c_xchng_cd);
  	fn_userlog(c_ServiceName,"Broker ID Is :%s:",st_msg.c_brkr_id);
  	fn_userlog(c_ServiceName,"Message Is :%s:",st_msg.c_msg);
  	fn_userlog(c_ServiceName,"Time Stamp  Is :%s:",st_msg.c_tm_stmp);
  	fn_userlog(c_ServiceName,"Message ID Is :%c:",st_msg.c_msg_id);
  	fn_userlog(c_ServiceName,"Before Call to SFO_UPD_MSG");
	}

  fn_cpy_ddr(st_msg.c_rout_crt);

  i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_errmsg,
                            &st_msg,
                            "vw_mkt_msg",
                            sizeof (struct vw_mkt_msg ),
                            TPNOREPLY,
                            "SFO_UPD_MSG" );

  if ( i_ch_val == SYSTEM_ERROR )
  {
    fn_errlog ( c_ServiceName, "S31145", LIBMSG, c_errmsg );
    return -1;
  }

	if(strstr(st_msg.c_msg,"closeout"))
  {
     c_brk_stts = 'C';

		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Broker Status Is :%c:",c_brk_stts);
		}
		
		/*** Addded in ver 2.6 ***/

     iLen=strlen(st_msg.c_msg);

     if(DEBUG_MSG_LVL_0)
     {
       fn_userlog ( c_ServiceName,"The lenght of string is |%d|",iLen);
     }

     cPtr = (char *)(st_msg.c_msg + iLen);
     cPtr--;

     while (cPtr != st_msg.c_msg)
     {
       if (' ' == *cPtr)
       {
         break;
       }
       cPtr--;
     }

     cPtr++;
     strcpy ( c_undrlyng,cPtr );

     if(DEBUG_MSG_LVL_0)
     {
        fn_userlog(c_ServiceName,"The Underlying1 is :%s:",cPtr );
        fn_userlog(c_ServiceName,"The Underlying2 is :%s:",c_undrlyng);
     }

		 /************** 2.9 Starts ***************/

     if(strcmp(c_undrlyng,"closeout")==0 )
     {
				 if(DEBUG_MSG_LVL_0)
				 {
           fn_userlog(c_ServiceName,"Global level closeout Mode");
				 }
         c_closeout_flag = 'C';
     }
     else
     {
     /************** 2.9 Ends   ***************/

     EXEC SQL
        SELECT  SEM_STCK_CD
        INTO    :c_stock_cd
        FROM    SEM_STCK_MAP
        WHERE   SEM_MAP_VL = :c_undrlyng
        AND     SEM_ENTTY   = 3;

      if ( SQLCODE  !=  0 )
      {
        fn_errlog ( c_ServiceName, "S31150", SQLMSG, c_errmsg );
        return -1;
      }
     
		 if(DEBUG_MSG_LVL_0)
     {
        fn_userlog(c_ServiceName,"The Stock code  is :%s:",c_stock_cd );
		 }
	
     /*** Ver 2.6 Ended ***/
		 }

		if (strstr (st_msg.c_msg,"not") != NULL)   /*** Ver 2.9 Added for handeling of activation message ****/
		{
			if(DEBUG_MSG_LVL_0)
      {
        fn_userlog(c_ServiceName,"Activation message for closeout");
      }
			c_message_typ = 'A';
		}
  }
  else if(strstr(st_msg.c_msg,"suspended"))
  {
    c_brk_stts = 'S';

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Broker Status Is :%c:",c_brk_stts);
		}
  }
	/************ Ver 2.9 Starts ***********/
	else if( (strstr(st_msg.c_msg,"Risk"))  && (strstr(st_msg.c_msg,"Reduction")))
	{

		if(DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName,"In Risk Reduction Mode");
		}

		iLen=strlen(st_msg.c_msg);

    if(DEBUG_MSG_LVL_0)
    {
      fn_userlog ( c_ServiceName,"The lenght of string is |%d|",iLen);
    }

    cPtr = (char *)(st_msg.c_msg + iLen);
    cPtr--;

    while (cPtr != st_msg.c_msg)
    {
      if (' ' == *cPtr)
      {
        break;
      }
      cPtr--;
    }

    cPtr++;
    strcpy ( c_undrlyng,cPtr );

    if(DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName,"The Underlying1 is :%s:",cPtr );
      fn_userlog(c_ServiceName,"The Underlying2 is :%s:",c_undrlyng);
    }

    EXEC SQL
       SELECT  SEM_STCK_CD
       INTO    :c_stock_cd
       FROM    SEM_STCK_MAP
       WHERE   SEM_MAP_VL = :c_undrlyng
       AND     SEM_ENTTY   = 3;

     if ( SQLCODE  !=  0 && SQLCODE != NO_DATA_FOUND )
     {
       fn_errlog ( c_ServiceName, "S31155", SQLMSG, c_errmsg );
       return -1;
     }

     if(DEBUG_MSG_LVL_0)
     {
       fn_userlog(c_ServiceName,"The RRM FOR Stock code  is :%s:",c_stock_cd );
     }

     if ( SQLCODE == NO_DATA_FOUND )
     {
       c_closeout_flag = 'R';
     }

		if (strstr (st_msg.c_msg,"below") != NULL)   /*** Ver 2.9 Added for handeling of activation message ****/
    {
			if(DEBUG_MSG_LVL_3)
    	{
      	fn_userlog(c_ServiceName,"Activation message for RRM");
    	}
      c_message_typ = 'A';
    }

	}
	/************ Ver 2.9 Ends   ***********/
  else
  {
    return 0;
  }


  i_trnsctn = fn_begintran( c_ServiceName, c_errmsg );
  if ( i_trnsctn == -1 )
  {
    fn_errlog ( c_ServiceName, "S31160", LIBMSG, c_errmsg );
    return -1;
  }

	if (c_closeout_flag != 'C' && c_closeout_flag != 'R')     /*** Condition added in ver 2.9 ***/
	{
		if ( c_message_typ == 'A' && c_brk_stts == 'C' ) /*** Ver 2.9 reactivation message handeling ***/
		{
			if(DEBUG_MSG_LVL_3)
     	{
       	fn_userlog(c_ServiceName,"The closeout message type is :%c:",c_message_typ );
     	}

			EXEC SQL
      UPDATE  EXG_XCHNG_MSTR
      SET     EXG_BRKR_STTS    = 'A'
      WHERE   EXG_XCHNG_CD     = :c_xchng_cd;

    	if ( SQLCODE != 0 )
    	{
      	fn_errlog ( c_ServiceName, "S31165", SQLMSG, c_errmsg );
      	fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
      	return -1;
    	}
		}
		else																							/*** End of ver 2.9 ***/	
		{

  		EXEC SQL
    		UPDATE  EXG_XCHNG_MSTR
    		SET     EXG_BRKR_STTS    = :c_brk_stts
    		WHERE   EXG_XCHNG_CD     = :c_xchng_cd;
	
  		if ( SQLCODE != 0 )
  		{
    		fn_errlog ( c_ServiceName, "S31170", SQLMSG, c_errmsg );
    		fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
    		return -1;
  		}
		}
	}	
	/*** Ver 2.6 started ***/

  if ( c_brk_stts == 'C')
  {
		/************** Ver 2.9 Starts ***********/
		if (c_closeout_flag == 'C' )
		{
			if(DEBUG_MSG_LVL_0) 
			{
       	fn_userlog(c_ServiceName,"Broker Status closed update underlying level for :%s:",c_stock_cd);
				fn_userlog(c_ServiceName,"The message type is :%c:",c_message_typ );
      }

			if (c_message_typ == 'A')	
			{
				c_closeout_flag = 'N';
			}

       EXEC SQL
            UPDATE EXG_XCHNG_MSTR
            SET  EXG_CLS_STTS  = :c_closeout_flag
            WHERE EXG_XCHNG_CD = :c_xchng_cd;

        if ( SQLCODE  !=  0 )
        {
            fn_errlog ( c_ServiceName, "S31175", SQLMSG, c_errmsg );
            fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
            return -1;
        }
		}
		else
    {
		/************** Ver 2.9 Ends ***********/
    	if(DEBUG_MSG_LVL_0) {
       fn_userlog(c_ServiceName,"Broker Status closed update underlying level for :%s:",c_stock_cd);
			 fn_userlog(c_ServiceName,"The message type is :%c:",c_message_typ );
    	}

			if (c_message_typ == 'A')
      {
        c_und_brkr_stts = 'N';
      }
			else 
			{
				c_und_brkr_stts = 'Y';
			}

    	EXEC SQL
        UPDATE FUM_FO_UNDRLYNG_MSTR
        SET   FUM_BRKR_CLSOUT_FLG = :c_und_brkr_stts
        WHERE  FUM_UNDRLYNG =:c_stock_cd
				AND    FUM_XCHNG_CD =:c_xchng_cd;

    	if ( SQLCODE  !=  0 )
    	{
      	fn_errlog ( c_ServiceName, "S31180", SQLMSG, c_errmsg );
      	fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
      	return -1;
    	}
		}
  }
	/*********  Ver 2.9 Starts  ********/

	if (c_closeout_flag == 'R' )
  {
      if(DEBUG_MSG_LVL_0) {
       fn_userlog(c_ServiceName,"Broker in RRM Status "); 
			 fn_userlog(c_ServiceName,"The message type is :%c:",c_message_typ );
      }

			if (c_message_typ == 'A')
      {
        c_closeout_flag = 'N';
      }

       EXEC SQL
            UPDATE EXG_XCHNG_MSTR
            SET  EXG_CLS_STTS  = :c_closeout_flag 
            WHERE EXG_XCHNG_CD = :c_xchng_cd;

        if ( SQLCODE  !=  0 )
        {
            fn_errlog ( c_ServiceName, "S31185", SQLMSG, c_errmsg );
            fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
            return -1;
        }
  }

	/**********   Ver 2.9 Ends   ********/

  /*** Ver 2.6 Ended ***/

  if ( fn_committran( c_ServiceName, i_trnsctn, c_errmsg ) == -1 )
  {
    fn_errlog ( c_ServiceName, "S31190", LIBMSG, c_errmsg );
    return -1;
  }

  if(c_brk_stts == 'S')
  {
    strcpy ( c_raise_trigger_type , "TRG_BRKR_SUS");
    fn_pst_trg ( c_ServiceName,c_raise_trigger_type, c_raise_trigger_type,c_xchng_cd);

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"After Posting Trigger TRG_BRKR_SUS");
		}
  }

	 return 0;

}

int fn_upd_trdbk  ( char *c_ServiceName,      /***  Ver 2.0 ***/
                    char *c_err_msg,
										char *c_usr_id,
										char *c_lst_act_ref,      /*** Ver 6.1 ***/
                    struct vw_tradebook *ptr_st_i_trdbk)
{

	int	i_rec_found = 0;
  long l_stream_no=0;


  if(DEBUG_MSG_LVL_3)   /*** ver 4.9 ***/
  {
	  fn_userlog(c_ServiceName,"Inside Function fn_upd_trdbk.");
		fn_userlog(c_ServiceName,"Last Activity Ref inside fn_upd_trdbk is :%s:",c_lst_act_ref);      /*** Ver 6.1 ***/
  }

  if ( ptr_st_i_trdbk->l_dwnld_flg == DOWNLOAD )
  {

      /*** ver 6.1 starts ***/
    if(DEBUG_MSG_LVL_0) 
    {
      fn_userlog(c_ServiceName,"INSIDE DOWNLOAD CASE IN fn_upd_trdbk.");
    }
      l_stream_no=0;

      EXEC SQL
      SELECT nvl(MAX( FXB_STREAM_NO ),0)
        INTO :l_stream_no
        FROM FXB_FO_XCHNG_BOOK
       WHERE FXB_ORDR_RFRNC = :ptr_st_i_trdbk->c_ordr_rfrnc
         AND FXB_STREAM_NO NOT IN ('-1','99');

      if ( SQLCODE != 0 )
      {
         fn_errlog ( c_ServiceName, "S31195", SQLMSG, c_err_msg );
         tpfree ( ( char * ) ptr_st_i_trdbk );
         return -1;
      }
      ptr_st_i_trdbk->l_ca_lvl =l_stream_no;
      /*** ver 6.1 ends ***/

		if(DEBUG_MSG_LVL_3)		/*** ver 2.3 ***/
		{
  		fn_userlog(c_ServiceName,"Jiffy :%lf:",ptr_st_i_trdbk->d_jiffy);
			fn_userlog(c_ServiceName,"Stream Number :%ld:",ptr_st_i_trdbk->l_ca_lvl);
			fn_userlog(c_ServiceName,"Order ack :%s:",ptr_st_i_trdbk-> c_xchng_ack);
		}

  	EXEC SQL
      SELECT 1
      INTO  :i_rec_found
      FROM 	FTD_FO_TRD_DTLS
      WHERE FTD_JIFFY = :ptr_st_i_trdbk->d_jiffy
      AND 	FTD_XCHNG_CD = :ptr_st_i_trdbk->c_xchng_cd
      /*** AND 	FTD_STREAM_NO = :ptr_st_i_trdbk->l_ca_lvl       ***  Ver 1.3 *** Commented in Ver 6.6 ***/
      AND   FTD_XCHNG_TRD_NO = :ptr_st_i_trdbk->l_xchng_trd_no /***  Ver 5.7 ***/
			AND   FTD_ORD_ACK_NMBR = :ptr_st_i_trdbk-> c_xchng_ack;	 /***  Ver 2.3 ***/

    if ( ( SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
    {
    	fn_errlog(c_ServiceName,"S31200",SQLMSG,c_err_msg );
      tpfree ( ( char * ) ptr_st_i_trdbk );
			return -1;
    }
  }
	
	if ( i_rec_found == 1 )
	{
		fn_userlog(c_ServiceName,"Record Already Processed.");
	  tpfree ( ( char * ) ptr_st_i_trdbk );
		return RECORD_ALREADY_PRCSD;
	}
	else
	{
      /*** ver 6.1 starts ***/
      l_stream_no=0;

      EXEC SQL
      SELECT nvl(MAX( FXB_STREAM_NO ),0)
        INTO :l_stream_no
        FROM FXB_FO_XCHNG_BOOK
       WHERE FXB_ORDR_RFRNC = :ptr_st_i_trdbk->c_ordr_rfrnc
         AND FXB_STREAM_NO NOT IN ('-1','99');

      if ( SQLCODE != 0 )
      {
         fn_errlog ( c_ServiceName, "S31205", SQLMSG, c_err_msg );
         tpfree ( ( char * ) ptr_st_i_trdbk );
         return -1;
      } 
      ptr_st_i_trdbk->l_ca_lvl =l_stream_no; 

      if ( l_stream_no == 0 )
      {
        fn_userlog(c_ServiceName,"INSERTING TRADE DETAILS WITH STREAM NO ZERO!!!");
      }

      /*** ver 6.1 ends ***/

      EXEC SQL
        INSERT INTO FTD_FO_TRD_DTLS
        (
          FTD_XCHNG_CD,
          FTD_CLM_MTCH_ACCNT,
          FTD_EBA_TRD_REF,
          FTD_ORDR_RFRNC,
          FTD_ORD_ACK_NMBR,
          FTD_XCHNG_TRD_NO,
          FTD_PRDCT_TYP,
          FTD_INDSTK,
          FTD_UNDRLYNG,
          FTD_EXPRY_DT,
          FTD_EXER_TYP,
          FTD_OPT_TYP,
          FTD_STRK_PRC,
          FTD_TRD_DT,
          FTD_TRD_FLW,
          FTD_EXCTD_QTY,
          FTD_EXCTD_RT,
          FTD_RMS_PRCSD_FLG,
          FTD_UPLD_MTCH_FLG,
          FTD_JIFFY,
          FTD_USR_ID,                         
          FTD_STREAM_NO ,
          FTD_INSRT_TM,             /*** ver 4.3 ***/                     
					FTD_LST_ACT_REF						/*** Ver 6.1 ***/
        )
        VALUES
        (
          :ptr_st_i_trdbk->c_xchng_cd,
          :ptr_st_i_trdbk->c_cln_mtch_accnt,
          :ptr_st_i_trdbk->c_trd_rfrnc,
          :ptr_st_i_trdbk->c_ordr_rfrnc,
          :ptr_st_i_trdbk->c_xchng_ack,
          :ptr_st_i_trdbk->l_xchng_trd_no,
          :ptr_st_i_trdbk->c_prd_typ,
          :ptr_st_i_trdbk->c_ctgry_indstk,
          :ptr_st_i_trdbk->c_undrlyng,
          to_date ( :ptr_st_i_trdbk->c_expry_dt, 'DD-Mon-yyyy' ),
          :ptr_st_i_trdbk->c_exrc_typ,
          :ptr_st_i_trdbk->c_opt_typ,
          :ptr_st_i_trdbk->l_strike_prc,
          to_date ( :ptr_st_i_trdbk->c_trd_dt, 'DD-Mon-yyyy hh24:mi:ss' ),
          :ptr_st_i_trdbk->c_trd_flw,
          :ptr_st_i_trdbk->l_exctd_qty,
          :ptr_st_i_trdbk->l_exctd_rt,
         	'N',
          'N',
          :ptr_st_i_trdbk->d_jiffy,
          :c_usr_id,
          :ptr_st_i_trdbk->l_ca_lvl     ,
          sysdate,                    /*** ver 4.3 ***/ 
					:c_lst_act_ref							/*** Ver 6.1 ***/
        );

        if ( SQLCODE != 0 )
        {
          fn_errlog ( c_ServiceName, "S31210", SQLMSG, c_err_msg );
          tpfree ( ( char * ) ptr_st_i_trdbk );
					return -1;
        }
	}

	return 0;
}
int fn_ac_to_ord (  char *c_ServiceName,      /***  Ver 2.0 ***/
                    char *c_err_msg,
                    char *c_usr_id,
                    struct vw_orderbook *ptr_st_orderbook
                 )
{

	varchar c_user_id [16];

  short  i_sprd_ord_ref;
	short	 i_ack_tm;
	short  i_settlor;
	short  i_prev_ack_tm;

	fn_userlog(c_ServiceName,"Inside Function fn_ac_to_ord.");

  EXEC SQL
      SELECT  FOD_CLM_MTCH_ACCNT,
              FOD_CLNT_CTGRY,
              FOD_ORDR_RFRNC,
              FOD_XCHNG_CD,
              FOD_PRDCT_TYP,
              FOD_UNDRLYNG,
              TO_CHAR ( FOD_EXPRY_DT, 'dd-Mon-yyyy' ),
              FOD_EXER_TYP,
              FOD_OPT_TYP,
              FOD_STRK_PRC,
              FOD_ORDR_FLW,
              FOD_LMT_MRKT_SL_FLG,
              NVL( FOD_DSCLSD_QTY, 0 ),
              NVL ( FOD_ORDR_TOT_QTY, 0 ),
              NVL( FOD_LMT_RT, 0 ),
              FOD_STP_LSS_TGR,
              FOD_ORDR_TYPE,
              TO_CHAR ( FOD_ORDR_VALID_DT, 'dd-Mon-yyyy' ),
              TO_CHAR ( FOD_TRD_DT, 'dd-Mon-yyyy' ),
              FOD_ORDR_STTS,
              NVL( FOD_EXEC_QTY, 0 ),
              NVL( FOD_EXEC_QTY_DAY, 0 ),
              NVL( FOD_CNCL_QTY, 0 ),
              NVL( FOD_EXPRD_QTY, 0 ),
              FOD_SPRD_ORDR_REF,
              FOD_MDFCTN_CNTR,
              FOD_SETTLOR,
              FOD_SPL_FLAG,
              FOD_INDSTK,
              TO_CHAR ( FOD_ORD_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss' ),
              TO_CHAR ( FOD_LST_RQST_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss' ),
              NVL( TRIM(FOD_USR_ID), '*')                    
      INTO    :ptr_st_orderbook->c_cln_mtch_accnt,
              :ptr_st_orderbook->l_clnt_ctgry,
              :ptr_st_orderbook->c_ordr_rfrnc,
              :ptr_st_orderbook->c_xchng_cd,
              :ptr_st_orderbook->c_prd_typ,
              :ptr_st_orderbook->c_undrlyng,
              :ptr_st_orderbook->c_expry_dt,
              :ptr_st_orderbook->c_exrc_typ,
              :ptr_st_orderbook->c_opt_typ,
              :ptr_st_orderbook->l_strike_prc,
              :ptr_st_orderbook->c_ordr_flw,
              :ptr_st_orderbook->c_slm_flg,
              :ptr_st_orderbook->l_dsclsd_qty,
              :ptr_st_orderbook->l_ord_tot_qty,
              :ptr_st_orderbook->l_ord_lmt_rt,
              :ptr_st_orderbook->l_stp_lss_tgr,
              :ptr_st_orderbook->c_ord_typ,
              :ptr_st_orderbook->c_valid_dt,
              :ptr_st_orderbook->c_trd_dt,
              :ptr_st_orderbook->c_ordr_stts,
              :ptr_st_orderbook->l_exctd_qty,
              :ptr_st_orderbook->l_exctd_qty_day,
              :ptr_st_orderbook->l_can_qty,
              :ptr_st_orderbook->l_exprd_qty,
              :ptr_st_orderbook->c_sprd_ord_rfrnc :i_sprd_ord_ref,
              :ptr_st_orderbook->l_mdfctn_cntr,
              :ptr_st_orderbook->c_settlor :i_settlor,
              :ptr_st_orderbook->c_req_typ,
              :ptr_st_orderbook->c_ctgry_indstk,
              :ptr_st_orderbook->c_ack_tm :i_ack_tm,
              :ptr_st_orderbook->c_prev_ack_tm :i_prev_ack_tm,
							:c_user_id
      FROM    FOD_FO_ORDR_DTLS
      WHERE   FOD_ACK_NMBR   = :ptr_st_orderbook->c_xchng_ack
      AND     FOD_PIPE_ID    = :ptr_st_orderbook->c_pipe_id
      AND     FOD_XCHNG_CD   = :ptr_st_orderbook->c_xchng_cd
      AND     FOD_PRDCT_TYP  = :ptr_st_orderbook->c_prd_typ
      AND     FOD_INDSTK     = :ptr_st_orderbook->c_ctgry_indstk
      AND     FOD_UNDRLYNG   = :ptr_st_orderbook->c_undrlyng
      AND     FOD_EXPRY_DT   = to_date ( :ptr_st_orderbook->c_expry_dt,'dd-mon-yyyy' )
      AND     FOD_EXER_TYP   = :ptr_st_orderbook->c_exrc_typ
      AND     FOD_OPT_TYP    = :ptr_st_orderbook->c_opt_typ
      AND     FOD_STRK_PRC   = :ptr_st_orderbook->l_strike_prc;
	
	if ( SQLCODE != 0  ) /***v2.7 condition SQLCODE NDF removed***/
	{
		fn_userlog ( c_ServiceName,"HISTRY SERCH for NDF REMOVED AS IT Looks NON PRACTICAL: Sangeet");
  	fn_errlog ( c_ServiceName, "S31215", SQLMSG,c_err_msg );
  	return -1;
	}

	if ( SQLCODE == NO_DATA_FOUND )
	{
		fn_userlog ( c_ServiceName,"INSIDE HISTORY");

		
    EXEC SQL
      SELECT  FOD_CLM_MTCH_ACCNT,
              FOD_CLNT_CTGRY,
              FOD_ORDR_RFRNC,
              FOD_XCHNG_CD,
              FOD_PRDCT_TYP,
              FOD_UNDRLYNG,
              TO_CHAR ( FOD_EXPRY_DT, 'dd-Mon-yyyy' ),
              FOD_EXER_TYP,
              FOD_OPT_TYP,
              FOD_STRK_PRC,
              FOD_ORDR_FLW,
              FOD_LMT_MRKT_SL_FLG,
              NVL( FOD_DSCLSD_QTY, 0 ),
              NVL ( FOD_ORDR_TOT_QTY, 0 ),
              NVL( FOD_LMT_RT, 0 ),
              FOD_STP_LSS_TGR,
              FOD_ORDR_TYPE,
              TO_CHAR ( FOD_ORDR_VALID_DT, 'dd-Mon-yyyy' ),
              TO_CHAR ( FOD_TRD_DT, 'dd-Mon-yyyy' ),
              FOD_ORDR_STTS,
              NVL( FOD_EXEC_QTY, 0 ),
              NVL( FOD_CNCL_QTY, 0 ),
              NVL( FOD_EXPRD_QTY, 0 ),
              NVL( FOD_SPRD_ORDR_REF, '*' ),
              FOD_MDFCTN_CNTR,
              NVL( FOD_SETTLOR, '*' ),
              NVL( FOD_SPL_FLAG, ' ' ),
              NVL( FOD_INDSTK, '*' ),
              TO_CHAR ( FOD_ORD_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss' ),
              TO_CHAR ( FOD_LST_RQST_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss' ),
              NVL( TRIM(FOD_USR_ID), '*' )
      INTO    :ptr_st_orderbook->c_cln_mtch_accnt,
              :ptr_st_orderbook->l_clnt_ctgry,
              :ptr_st_orderbook->c_ordr_rfrnc,
              :ptr_st_orderbook->c_xchng_cd,
              :ptr_st_orderbook->c_prd_typ,
              :ptr_st_orderbook->c_undrlyng,
              :ptr_st_orderbook->c_expry_dt,
              :ptr_st_orderbook->c_exrc_typ,
              :ptr_st_orderbook->c_opt_typ,
              :ptr_st_orderbook->l_strike_prc,
              :ptr_st_orderbook->c_ordr_flw,
              :ptr_st_orderbook->c_slm_flg,
              :ptr_st_orderbook->l_dsclsd_qty,
            	:ptr_st_orderbook->l_ord_tot_qty,
              :ptr_st_orderbook->l_ord_lmt_rt,
              :ptr_st_orderbook->l_stp_lss_tgr,
              :ptr_st_orderbook->c_ord_typ,
              :ptr_st_orderbook->c_valid_dt,
              :ptr_st_orderbook->c_trd_dt,
              :ptr_st_orderbook->c_ordr_stts,
              :ptr_st_orderbook->l_exctd_qty,
              :ptr_st_orderbook->l_can_qty,
              :ptr_st_orderbook->l_exprd_qty,
              :ptr_st_orderbook->c_sprd_ord_rfrnc,
              :ptr_st_orderbook->l_mdfctn_cntr,
              :ptr_st_orderbook->c_settlor,
              :ptr_st_orderbook->c_req_typ,
              :ptr_st_orderbook->c_ctgry_indstk,
              :ptr_st_orderbook->c_ack_tm,
              :ptr_st_orderbook->c_prev_ack_tm,
              :c_user_id
      FROM    FOD_FO_ORDR_DTLS_HSTRY
      WHERE   FOD_ACK_NMBR = :ptr_st_orderbook->c_xchng_ack
      AND     FOD_PIPE_ID  = :ptr_st_orderbook->c_pipe_id
      AND     FOD_XCHNG_CD   = :ptr_st_orderbook->c_xchng_cd
      AND     FOD_PRDCT_TYP  = :ptr_st_orderbook->c_prd_typ
      AND     FOD_INDSTK     = :ptr_st_orderbook->c_ctgry_indstk
      AND     FOD_UNDRLYNG   = :ptr_st_orderbook->c_undrlyng
      AND     FOD_EXPRY_DT   = to_date ( :ptr_st_orderbook->c_expry_dt,'dd-mon-yyyy' )
      AND     FOD_EXER_TYP   = :ptr_st_orderbook->c_exrc_typ
      AND     FOD_OPT_TYP    = :ptr_st_orderbook->c_opt_typ
      AND     FOD_STRK_PRC   = :ptr_st_orderbook->l_strike_prc;

      if ( SQLCODE != 0 )
      {
        fn_errlog ( c_ServiceName,"S31220", SQLMSG,c_err_msg );
      	return -1;
      }

	}

	SETNULL(c_user_id);
	strcpy(c_usr_id,c_user_id.arr);

	return 0;
}

/*** Added function in Ver 5.3 Starts ***/

int fn_fxb_rms_erl (    char *c_ServiceName,     
                    char *c_err_msg,
                    char c_prd_typ,
                    char *c_ordr_rfrnc)
{
		int i = 0;
		int i_ferr[7];
  	int i_xchngbkerr[7];
		int i_returncode = 0;
		int i_status = 0;
	  char c_spn_flg = '\0';	
  	char c_sltp_ord_rfrnc[19];
		/***** long l_ftd_ord_exe_qty= 0;  **** Commented in Ver 5.4 ******/
		struct vw_xchngbook st_cvr_xchngbk;

 	 	FBFR32 *ptr_fml_Sbuf;
  	ptr_fml_Sbuf  = ( FBFR32 *)NULL;
    MEMSET(st_cvr_xchngbk);
    MEMSET(i_xchngbkerr);
    MEMSET(i_ferr);

  	if(DEBUG_MSG_LVL_3)
  	{
			fn_userlog(c_ServiceName," c_ordr_rfrnc is :%s:",c_ordr_rfrnc ) ;
			fn_userlog(c_ServiceName," c_prd_typ is :%c:",c_prd_typ ) ;
		}
		EXEC SQL
    SELECT
				 FXB_ORS_MSG_TYP,
         FXB_MDFCTN_CNTR,
         nvl(to_char(FXB_ACK_TM, 'dd-mon-yyyy hh24:mi:ss'),'*'),
         NVL(FXB_XCHNG_CNCLD_QTY, 0)                                    /***** Ver 5.4 ****/
    INTO  :st_cvr_xchngbk.l_ors_msg_typ,
          :st_cvr_xchngbk.l_mdfctn_cntr,
          :st_cvr_xchngbk.c_ack_tm,
          :st_cvr_xchngbk.l_xchng_can_qty                               /***** Ver 5.4 ****/
    FROM  FXB_FO_XCHNG_BOOK
    WHERE FXB_ORDR_RFRNC = :c_ordr_rfrnc
    AND   FXB_MDFCTN_CNTR =  
													( SELECT min(FXB_MDFCTN_CNTR)
                            FROM   FXB_FO_XCHNG_BOOK
                            WHERE  FXB_ORDR_RFRNC =   :c_ordr_rfrnc
                            AND    FXB_JIFFY =
																							(select min(FXB_JIFFY)
																							 from   FXB_FO_XCHNG_BOOK
																							 where  FXB_ORDR_RFRNC = :c_ordr_rfrnc
																							 AND    FXB_RMS_PRCSD_FLG = 'N')
													   AND    FXB_RMS_PRCSD_FLG = 'N'
													 )
	  AND   FXB_RMS_PRCSD_FLG = 'N';

    if(SQLCODE != 0 && SQLCODE !=  NO_DATA_FOUND )
    {
       fn_errlog(c_ServiceName, "L31370", SQLMSG,  c_err_msg);
       return -1;
    }
    else if( SQLCODE ==  NO_DATA_FOUND )
    {
				fn_userlog( c_ServiceName, "No unprocessed fresh record checking for cover");

				if(c_prd_typ == 'U' || c_prd_typ == 'I')
				{	
			 
					EXEC SQL
					SELECT
						fod_sltp_ordr_rfrnc,
						FOD_PRDCT_TYP	
					INTO
							:c_sltp_ord_rfrnc,
							:c_prd_typ
					FROM  FOD_FO_ORDR_DTLS
					WHERE fod_ordr_rfrnc =:c_ordr_rfrnc;

					if(SQLCODE !=0)
					{
						fn_errlog ( c_ServiceName, "L31375", SQLMSG, c_err_msg );
						return -1;
					}

					EXEC SQL
					SELECT
							 FXB_ORS_MSG_TYP,
							 FXB_MDFCTN_CNTR,
							 nvl(to_char(FXB_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss'),'-'),
         			 NVL(FXB_XCHNG_CNCLD_QTY, 0)                                    /***** Ver 5.4 ****/
				INTO   :st_cvr_xchngbk.l_ors_msg_typ,
							 :st_cvr_xchngbk.l_mdfctn_cntr,
							 :st_cvr_xchngbk.c_ack_tm,
         			 :st_cvr_xchngbk.l_xchng_can_qty                                /***** Ver 5.4 ****/
					FROM FXB_FO_XCHNG_BOOK
					WHERE FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc
					AND   FXB_MDFCTN_CNTR =  
															( SELECT min(FXB_MDFCTN_CNTR)
																FROM   FXB_FO_XCHNG_BOOK
																WHERE  FXB_ORDR_RFRNC =   :c_sltp_ord_rfrnc
																AND    FXB_JIFFY =
																									(select min(FXB_JIFFY)
																									 from   FXB_FO_XCHNG_BOOK
																									 where  FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc
																									 AND    FXB_RMS_PRCSD_FLG = 'N')
																 AND    FXB_RMS_PRCSD_FLG = 'N'
															 )
					AND    FXB_RMS_PRCSD_FLG = 'N';

					if(SQLCODE !=0 && SQLCODE != NO_DATA_FOUND)
					{
							fn_errlog(c_ServiceName, "L31380", SQLMSG, c_err_msg);
							return -1 ;
					}
					if(SQLCODE == NO_DATA_FOUND )
					{
							fn_userlog( c_ServiceName, "No unprocessed cover record ");
							return 0 ;
					}
					else 
					{
						i_status = 1;
						fn_userlog( c_ServiceName, "unprocessed cover record so process it");
						strcpy(st_cvr_xchngbk.c_ordr_rfrnc,c_sltp_ord_rfrnc) ;
					}
			 }
		}
		else 
		{
			i_status = 1; 
			strcpy(st_cvr_xchngbk.c_ordr_rfrnc, c_ordr_rfrnc);
		}

		if(i_status == 1)
		{
			if(DEBUG_MSG_LVL_0)
			{
				fn_userlog( c_ServiceName, "Before Calling  Ack processing for Order Ref:%s:", st_cvr_xchngbk.c_ordr_rfrnc);
			}

			strcpy(st_cvr_xchngbk.c_entry_dt_tm , st_cvr_xchngbk.c_ack_tm);

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog( c_ServiceName, "st_cvr_xchngbk.c_entry_dt_tm:%s:", st_cvr_xchngbk.c_entry_dt_tm);
        fn_userlog( c_ServiceName, "st_cvr_xchngbk.c_ack_tm:%s:", st_cvr_xchngbk.c_ack_tm);
        fn_userlog( c_ServiceName, "st_cvr_xchngbk.l_mdfctn_cntrm:%ld:", st_cvr_xchngbk.l_mdfctn_cntr);
        fn_userlog( c_ServiceName, "st_cvr_xchngbk.l_ors_msg_typ:%ld:", st_cvr_xchngbk.l_ors_msg_typ);
      }
      st_cvr_xchngbk.d_jiffy       = 0;
      st_cvr_xchngbk.l_dwnld_flg   = 0;
      fn_cpy_ddr ( st_cvr_xchngbk.c_rout_crt );
      strcpy(st_cvr_xchngbk.c_xchng_rmrks,"*");
  
 	 		EXEC SQL
 	 		SELECT  NVL(FOD_SPN_FLG,'N')
 	 		INTO    :c_spn_flg
 	 		FROM    FOD_FO_ORDR_DTLS
 	 		where FOD_ORDR_RFRNC =:st_cvr_xchngbk.c_ordr_rfrnc;

 	 		if ( SQLCODE != 0 )
 	 		{
 	  			  fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");
    				fn_errlog(c_ServiceName, "L31385", SQLMSG,  c_err_msg);
  	  			return -1;
 	 		}

  		if(DEBUG_MSG_LVL_3) 
  		{
     		 fn_userlog( c_ServiceName, "st_cvr_xchngbk.c_entry_dt_tm:%s:", st_cvr_xchngbk.c_entry_dt_tm);
     		 fn_userlog( c_ServiceName, "st_cvr_xchngbk.l_mdfctn_cntrm:%ld:", st_cvr_xchngbk.l_mdfctn_cntr);
     		 fn_userlog( c_ServiceName, "st_cvr_xchngbk.l_ors_msg_typ:%ld:", st_cvr_xchngbk.l_ors_msg_typ);
  		}

			/******* Commented in Ver 5.4 **********
	
  		EXEC SQL
  		SELECT  NVL(SUM(ftd_exctd_qty),0)
  		INTO    :l_ftd_ord_exe_qty
  		FROM    ftd_fo_trd_dtls
  		WHERE   ftd_ordr_rfrnc = :st_cvr_xchngbk.c_ordr_rfrnc;

  		if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
  		{
   			fn_errlog ( c_ServiceName, "L31390", SQLMSG, c_err_msg );
   			fn_userlog(c_ServiceName,"Error While Selecting executed qty .");
   			return -1;
  		}

  		if( DEBUG_MSG_LVL_3 )
  		{
    		 fn_userlog( c_ServiceName, "l_ftd_ord_exe_qty: %ld",l_ftd_ord_exe_qty);
  		}

  		st_cvr_xchngbk.l_xchng_can_qty = l_ftd_ord_exe_qty;

			********** Commented Ver 5.4 Ends ************/

			if ( c_spn_flg == 'N' || ( c_spn_flg == 'S'  && c_prd_typ == 'P' ) )
  		{
    		if( DEBUG_MSG_LVL_0 ) 
    		{
      		 fn_userlog( c_ServiceName, "unprocessed record exist so process it...");
    		}			

				st_cvr_xchngbk.d_jiffy       = 0;
				st_cvr_xchngbk.l_dwnld_flg   = 0;
				strcpy(st_cvr_xchngbk.c_xchng_rmrks,"*");

				if(c_prd_typ =='I' || c_prd_typ =='O')
				{
					 i_returncode = fn_acall_svc ( c_ServiceName,
																				 c_err_msg,
																				 &st_cvr_xchngbk,
																				 "vw_xchngbook",
																				 sizeof ( st_cvr_xchngbk),
																				 TPNOREPLY,
																				 "SFO_OPT_ACK" );

					if ( i_returncode != 0 )
					{
					 fn_userlog( c_ServiceName, "Error while calling SFO_OPT_ACK");
					 fn_errlog( c_ServiceName, "L31395",LIBMSG, c_err_msg);
					 return -1 ;
					}
				}
				else if(c_prd_typ =='U' || c_prd_typ =='P' || c_prd_typ =='F' )
				{
				 i_returncode = fn_acall_svc ( c_ServiceName,
																		 c_err_msg,
																		 &st_cvr_xchngbk,
																		 "vw_xchngbook",
																		 sizeof ( st_cvr_xchngbk),
																		 TPNOREPLY,
																		 "SFO_FUT_ACK" );

						 if ( i_returncode != 0 )
						 {
									fn_userlog( c_ServiceName, "Error while calling SFO_FUT_ACK");
									fn_errlog( c_ServiceName, "L31400", LIBMSG, c_err_msg);
									return -1 ;

						 }
				 }
		 }
		 else if ( c_spn_flg == 'S'  &&  c_prd_typ != 'P' )
  	 {
     	 ptr_fml_Sbuf = (FBFR32*)tpalloc("FML32",NULL,MIN_FML_BUF_LEN);

       if ( ptr_fml_Sbuf == NULL )
       {
        fn_userlog(c_ServiceName,"ptr_fml_Sbuf is NULL ");
        fn_errlog(c_ServiceName, "L31405", TPMSG,  c_err_msg);
        return -1;
       }

        i_xchngbkerr [0] = Fadd32(ptr_fml_Sbuf,FFO_ROUT_CRT, (char *)st_cvr_xchngbk.c_rout_crt,0);
        i_ferr[0] = Ferror32;
        i_xchngbkerr [1] = Fadd32(ptr_fml_Sbuf,FFO_ORDR_RFRNC, (char *)st_cvr_xchngbk.c_ordr_rfrnc ,0);
        i_ferr[1] = Ferror32;
        i_xchngbkerr [2] = Fadd32(ptr_fml_Sbuf,FFO_ORS_MSG_TYP, (char *)&st_cvr_xchngbk.l_ors_msg_typ,0);
        i_ferr[2] = Ferror32;
        i_xchngbkerr [3] = Fadd32(ptr_fml_Sbuf,FFO_MDFCTN_CNTR, (char *)&st_cvr_xchngbk.l_mdfctn_cntr,0);
        i_ferr[3] = Ferror32;
        i_xchngbkerr [4] = Fadd32(ptr_fml_Sbuf,FFO_ACK_TM, (char *)st_cvr_xchngbk.c_ack_tm,0);
        i_ferr[4] = Ferror32;
        i_xchngbkerr [5] = Fadd32(ptr_fml_Sbuf,FFO_PREV_ACK_TM, (char *)st_cvr_xchngbk.c_entry_dt_tm,0);
        i_ferr[5] = Ferror32;
        i_xchngbkerr [6] = Fadd32(ptr_fml_Sbuf,FFO_XCHNG_CAN_QTY , (char *)&st_cvr_xchngbk.l_xchng_can_qty,0);
        i_ferr[6] = Ferror32;

        for (i = 0;i < 6 ;i++)
        {
          if(i_xchngbkerr[i] == -1 )
          {
            fn_userlog( c_ServiceName, "Error in :%d:",i );
            tpfree((char *)ptr_fml_Sbuf);
            fn_errlog(c_ServiceName, "L31410", FMLMSG,  c_err_msg);
            return -1;
          }
        }

        i_returncode  = tpacall ( "SFO_PRCS_RSP", (char *)ptr_fml_Sbuf,0, TPNOTRAN|TPNOREPLY);

        if ( i_returncode != SUCC_BFR )
        {
          fn_userlog( c_ServiceName, "Error while calling SFO_PRCS_RSP");
          fn_errlog ( c_ServiceName, "L31415", LIBMSG, c_err_msg );
          tpfree((char *)ptr_fml_Sbuf);
          return -1;
        }

        tpfree((char *)ptr_fml_Sbuf);  /*** Ver 5.5 ****/
		 }
   }
	 return 0;
}

int fn_ftd_rms_erl (char *c_ServiceName,
                    char *c_err_msg,
								    char c_prd_typ,
                    char *c_ordr_rfrnc)
{
    int i = 0;
		int i_err[18];
    int i_ferr[18];
    int i_returncode = 0;
    char c_spn_flg = '\0';
    char c_sltp_ord_rfrnc[19];
    struct vw_tradebook st_tradebook;   
 		short int si_null;
    FBFR32 *ptr_fml_Sbuf;
    ptr_fml_Sbuf  = ( FBFR32 *)NULL;

		MEMSET(st_tradebook);
		MEMSET(i_ferr);
		MEMSET(i_err);

		EXEC SQL
			SELECT  NVL(FOD_SPN_FLG,'N')
			INTO    :c_spn_flg
			FROM    FOD_FO_ORDR_DTLS
			where FOD_ORDR_RFRNC =:c_ordr_rfrnc;

	 if ( SQLCODE != 0 )
	 {
			 fn_userlog(c_ServiceName,"Error While Selecting Span Flag.");
			 fn_errlog(c_ServiceName, "L31420", SQLMSG,  c_err_msg);
			 return -1;
	 }	

	 if(c_prd_typ =='U' || c_prd_typ =='I')
	 {

		EXEC SQL
				SELECT
					fod_sltp_ordr_rfrnc
				INTO
					:c_sltp_ord_rfrnc
				FROM  FOD_FO_ORDR_DTLS
				WHERE fod_ordr_rfrnc = :c_ordr_rfrnc;

			if( SQLCODE !=0 )
			{
				fn_errlog ( c_ServiceName, "L31425", SQLMSG, c_err_msg );
				return -1;
			}

			EXEC SQL
			SELECT		ftd_clm_mtch_accnt,
								ftd_ordr_rfrnc,
								ftd_xchng_cd,
								ftd_prdct_typ,
								ftd_indstk,
								ftd_undrlyng,
								to_char(ftd_expry_dt,'DD-Mon-YYYY'),
								ftd_exer_typ,
								ftd_opt_typ,
								ftd_strk_prc,
								ftd_ordr_rfrnc,
								ftd_eba_trd_ref,
								ftd_ord_ack_nmbr,
								ftd_xchng_trd_no,
								to_char(ftd_trd_dt,'DD-Mon-YYYY hh24:mi:ss'),
								ftd_trnsctn_typ,
								ftd_trd_flw,
								ftd_exctd_qty,
								ftd_exctd_rt
					INTO  :st_tradebook.c_cln_mtch_accnt,
								:st_tradebook.c_ordr_rfrnc,
								:st_tradebook.c_xchng_cd,
								:st_tradebook.c_prd_typ,
								:st_tradebook.c_ctgry_indstk,
								:st_tradebook.c_undrlyng,
								:st_tradebook.c_expry_dt,
								:st_tradebook.c_exrc_typ,
								:st_tradebook.c_opt_typ,
								:st_tradebook.l_strike_prc,
								:st_tradebook.c_ordr_rfrnc,
								:st_tradebook.c_trd_rfrnc,
								:st_tradebook.c_xchng_ack,
								:st_tradebook.l_xchng_trd_no,
								:st_tradebook.c_trd_dt,
								:st_tradebook.c_trnsctn_typ:si_null,
								:st_tradebook.c_trd_flw,
								:st_tradebook.l_exctd_qty,
								:st_tradebook.l_exctd_rt
				FROM    ftd_fo_trd_dtls
				WHERE   ftd_ordr_rfrnc IN(:c_ordr_rfrnc,:c_sltp_ord_rfrnc)
				AND     ftd_rms_prcsd_flg = 'N'
				AND     ROWNUM < 2
				ORDER   BY FTD_TRD_DT;

			if(SQLCODE !=0 && SQLCODE !=NO_DATA_FOUND )
			{
				fn_errlog ( c_ServiceName, "L31430", SQLMSG, c_err_msg );
				return -1;
			}

			if( SQLCODE != NO_DATA_FOUND )
			{
				rtrim(st_tradebook.c_trd_dt);

				fn_cpy_ddr ( st_tradebook.c_rout_crt );

			 if(DEBUG_MSG_LVL_3)
			 {
				fn_userlog( c_ServiceName, "Calling Trade service");
			 }
			 if( c_spn_flg == 'N')
			 {
				 if(c_prd_typ =='U' )
				 {
					i_returncode = fn_acall_svc( c_ServiceName,
																			 c_err_msg,
																			&st_tradebook,
																			"vw_tradebook",
																			sizeof( struct vw_tradebook),
																			TPNOREPLY,
																			"SFO_FUT_TCONF" );

					if ( i_returncode != SUCC_BFR )
					 {
							fn_errlog ( c_ServiceName, "L31435", LIBMSG, c_err_msg );
							return -1;
					 }
				}
				else if(c_prd_typ =='I' )	
				{
					
				 i_returncode = fn_acall_svc( c_ServiceName,
												c_err_msg,
												&st_tradebook,
												"vw_tradebook",
												sizeof( struct vw_tradebook),
												TPNOREPLY,
												"SFO_OPT_TCONF" );

					if ( i_returncode != SUCC_BFR )
					{
						fn_errlog ( c_ServiceName, "L31440", LIBMSG, c_err_msg );
						return -1;
					}	
				 }
			 }
			 else if( c_spn_flg == 'S')
			 {
         ptr_fml_Sbuf = (FBFR32*)tpalloc("FML32",NULL,MIN_FML_BUF_LEN);

         if ( ptr_fml_Sbuf == NULL )
         {
            fn_userlog(c_ServiceName,"ptr_fml_Sbuf is NULL ");
            fn_errlog(c_ServiceName, "L31445", TPMSG,  c_err_msg);
            return -1;
         }


            i_err   [ 0 ]     = Fadd32( ptr_fml_Sbuf ,FFO_ROUT_CRT,(char *) st_tradebook.c_rout_crt,0);
            i_ferr  [ 0 ]     = Ferror32;
            i_err   [ 1 ]     = Fadd32( ptr_fml_Sbuf ,FFO_ORDR_RFRNC,(char *) st_tradebook.c_ordr_rfrnc,0);
            i_ferr  [ 1 ]     = Ferror32;
            i_err   [ 2 ]     = Fadd32( ptr_fml_Sbuf ,FFO_TRD_RFRNC,(char *) st_tradebook.c_trd_rfrnc,0);
            i_ferr  [ 2 ]     = Ferror32;
            i_err   [ 3 ]     = Fadd32( ptr_fml_Sbuf ,FFO_EBA_MTCH_ACT_NO,(char *) st_tradebook.c_cln_mtch_accnt,0);
            i_ferr  [ 3 ]     = Ferror32;
            i_err   [ 4 ]     = Fadd32( ptr_fml_Sbuf ,FFO_EBA_CNTRCT_ID,(char *) &st_tradebook.l_eba_cntrct_id,0);
            i_ferr  [ 4 ]     = Ferror32;
            i_err   [ 5 ]     = Fadd32( ptr_fml_Sbuf ,FFO_XCHNG_CD,(char *) st_tradebook.c_xchng_cd,0);
            i_ferr  [ 5 ]     = Ferror32;
            i_err   [ 6 ]     = Fadd32( ptr_fml_Sbuf ,FFO_PRDCT_TYP,(char *) &st_tradebook.c_prd_typ,0);
            i_ferr  [ 6 ]     = Ferror32;
            i_err   [ 7 ]     = Fadd32( ptr_fml_Sbuf ,FFO_UNDRLYNG,(char *) st_tradebook.c_undrlyng,0);
            i_ferr  [ 7 ]     = Ferror32;
            i_err   [ 8 ]     = Fadd32( ptr_fml_Sbuf ,FFO_EXPRY_DT,(char *) st_tradebook.c_expry_dt,0);
            i_ferr  [ 8 ]     = Ferror32;
            i_err   [ 9 ]     = Fadd32( ptr_fml_Sbuf ,FFO_EXER_TYP,(char *) &st_tradebook.c_exrc_typ,0);
            i_ferr  [ 9 ]     = Ferror32;
            i_err   [ 10  ]   = Fadd32( ptr_fml_Sbuf  ,FFO_OPT_TYP,(char *) &st_tradebook.c_opt_typ,0);
            i_ferr  [ 10  ]   = Ferror32;
            i_err   [ 11  ]   = Fadd32( ptr_fml_Sbuf ,FFO_STRK_PRC,(char *) &st_tradebook.l_strike_prc,0);
            i_ferr  [ 11  ]   = Ferror32;
            i_err   [ 12  ]   = Fadd32( ptr_fml_Sbuf ,FFO_CTGRY_INDSTK,(char *) &st_tradebook.c_ctgry_indstk,0);
            i_ferr  [ 12  ]   = Ferror32;
            i_err   [ 13  ]   = Fadd32( ptr_fml_Sbuf  ,FFO_CA_LVL,(char *) &st_tradebook.l_ca_lvl,0);
            i_ferr  [ 13  ]   = Ferror32;
            i_err   [ 14  ]   = Fadd32( ptr_fml_Sbuf ,FFO_TRD_FLW,(char *) &st_tradebook.c_trd_flw,0);
            i_ferr  [ 14  ]   = Ferror32;
            i_err   [ 15  ]   = Fadd32( ptr_fml_Sbuf ,FFO_EXEC_QTY,(char *) &st_tradebook.l_exctd_qty,0);
            i_ferr  [ 15  ]   = Ferror32;
            i_err   [ 16  ]   = Fadd32( ptr_fml_Sbuf  ,FFO_EXCTD_RT,(char *) &st_tradebook.l_exctd_rt,0);
	          i_ferr  [ 16  ]   = Ferror32;
            i_err   [ 17  ]   = Fadd32( ptr_fml_Sbuf ,FFO_TRD_DT,(char *) st_tradebook.c_trd_dt,0);
            i_ferr  [ 17  ]   = Ferror32;


            for ( i=0 ; i < 18 ;  i++ )
            {
              if ( (i_err[ i ] == -1 ) && (i_ferr[ i ]  != FNOTPRES ) )
              {
                fn_userlog( c_ServiceName, "Error in :%d:",i );
                fn_errlog( c_ServiceName, "L31450", Fstrerror32( i_ferr[i]),c_err_msg );
            		tpfree((char *)ptr_fml_Sbuf);
                Fadd32( ptr_fml_Sbuf, FFO_ERR_MSG, c_err_msg , 0 );
                return -1;
              }
            }

           i_returncode = tpacall ( "SFO_PRCS_TRD", (char *)ptr_fml_Sbuf,0, TPNOTRAN|TPNOREPLY);

					if ( i_returncode != SUCC_BFR )
					{
						fn_errlog ( c_ServiceName, "L31455", LIBMSG, c_err_msg );
            tpfree((char *)ptr_fml_Sbuf);
						return -1;
					}

          tpfree((char *)ptr_fml_Sbuf); /*** Ver 5.5 ***/ 
				}
			}
	 }
	 else if(c_prd_typ =='F' || c_prd_typ =='O' || c_prd_typ =='P' )
	 {
			EXEC SQL
			SELECT		ftd_clm_mtch_accnt,
								ftd_ordr_rfrnc,
								ftd_xchng_cd,
								ftd_prdct_typ,
								ftd_indstk,
								ftd_undrlyng,
								to_char(ftd_expry_dt,'DD-Mon-YYYY'),
								ftd_exer_typ,
								ftd_opt_typ,
								ftd_strk_prc,
								ftd_eba_trd_ref,
								ftd_ord_ack_nmbr,
								ftd_xchng_trd_no,
								to_char(ftd_trd_dt,'DD-Mon-YYYY hh24:mi:ss'),
								ftd_trnsctn_typ,
								ftd_trd_flw,
								ftd_exctd_qty,
								ftd_exctd_rt
					INTO  :st_tradebook.c_cln_mtch_accnt,
								:st_tradebook.c_ordr_rfrnc,
								:st_tradebook.c_xchng_cd,
								:st_tradebook.c_prd_typ,
								:st_tradebook.c_ctgry_indstk,
								:st_tradebook.c_undrlyng,
								:st_tradebook.c_expry_dt,
								:st_tradebook.c_exrc_typ,
								:st_tradebook.c_opt_typ,
								:st_tradebook.l_strike_prc,
								:st_tradebook.c_trd_rfrnc,
								:st_tradebook.c_xchng_ack,
								:st_tradebook.l_xchng_trd_no,
								:st_tradebook.c_trd_dt,
								:st_tradebook.c_trnsctn_typ:si_null,
								:st_tradebook.c_trd_flw,
								:st_tradebook.l_exctd_qty,
								:st_tradebook.l_exctd_rt
				FROM    ftd_fo_trd_dtls
				WHERE   ftd_ordr_rfrnc = :c_ordr_rfrnc
				AND     ftd_rms_prcsd_flg = 'N'
				AND     ROWNUM < 2
				ORDER   BY FTD_TRD_DT;

			if(SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
			{
				 fn_errlog(c_ServiceName, "L31460", SQLMSG,  c_err_msg);
				 return -1;
			}
			 
			if( SQLCODE != NO_DATA_FOUND )
			{
				 rtrim(st_tradebook.c_trd_dt);
				 fn_cpy_ddr ( st_tradebook.c_rout_crt );

				 if(DEBUG_MSG_LVL_3)   
				 {
					fn_userlog( c_ServiceName, "Calling Trade service");
				 }

				 if( c_spn_flg == 'N' || ( c_spn_flg == 'S' && c_prd_typ =='P' ) )
				 {
					if(c_prd_typ =='F' || c_prd_typ =='P' )
					{
					 i_returncode = fn_acall_svc( c_ServiceName,
															 c_err_msg,
															 &st_tradebook,
															 "vw_tradebook",
															 sizeof( struct vw_tradebook),
															 TPNOREPLY,
															 "SFO_FUT_TCONF" );


						if ( i_returncode != SUCC_BFR )
						{
							 fn_errlog ( c_ServiceName, "L31465", LIBMSG, c_err_msg );
							 return -1;
						}
					}
					else if(c_prd_typ =='O' )	
					{
					 i_returncode = fn_acall_svc( c_ServiceName,
													c_err_msg,
													&st_tradebook,
													"vw_tradebook",
													sizeof( struct vw_tradebook),
													TPNOREPLY,
													"SFO_OPT_TCONF" );

						 if ( i_returncode != SUCC_BFR )
						 {
							fn_errlog ( c_ServiceName, "L31470", LIBMSG, c_err_msg );
							return -1;
						 }
					 }
					}
				  else if( c_spn_flg == 'S' && c_prd_typ !='P' )
					{

       			ptr_fml_Sbuf = (FBFR32*)tpalloc("FML32",NULL,MIN_FML_BUF_LEN);

       			if ( ptr_fml_Sbuf == NULL )
       			{
        				fn_userlog(c_ServiceName,"ptr_fml_Sbuf is NULL ");
        				fn_errlog(c_ServiceName, "L31475", TPMSG,  c_err_msg);
        				return -1;
       			}


  					i_err   [ 0 ]     = Fadd32( ptr_fml_Sbuf ,FFO_ROUT_CRT,(char *) st_tradebook.c_rout_crt,0);
  					i_ferr  [ 0 ]     = Ferror32;
  					i_err   [ 1 ]     = Fadd32( ptr_fml_Sbuf ,FFO_ORDR_RFRNC,(char *) st_tradebook.c_ordr_rfrnc,0);
  					i_ferr  [ 1 ]     = Ferror32;
 						i_err   [ 2 ]     = Fadd32( ptr_fml_Sbuf ,FFO_TRD_RFRNC,(char *) st_tradebook.c_trd_rfrnc,0);
  					i_ferr  [ 2 ]     = Ferror32;
  					i_err   [ 3 ]     = Fadd32( ptr_fml_Sbuf ,FFO_EBA_MTCH_ACT_NO,(char *) st_tradebook.c_cln_mtch_accnt,0);
  					i_ferr  [ 3 ]     = Ferror32;
  					i_err   [ 4 ]     = Fadd32( ptr_fml_Sbuf ,FFO_EBA_CNTRCT_ID,(char *) &st_tradebook.l_eba_cntrct_id,0);
  					i_ferr  [ 4 ]     = Ferror32;
  					i_err   [ 5 ]     = Fadd32( ptr_fml_Sbuf ,FFO_XCHNG_CD,(char *) st_tradebook.c_xchng_cd,0);
  					i_ferr  [ 5 ]     = Ferror32;
  					i_err   [ 6 ]     = Fadd32( ptr_fml_Sbuf ,FFO_PRDCT_TYP,(char *) &st_tradebook.c_prd_typ,0);
						i_ferr  [ 6 ]     = Ferror32;
  					i_err   [ 7 ]     = Fadd32( ptr_fml_Sbuf ,FFO_UNDRLYNG,(char *) st_tradebook.c_undrlyng,0);
  					i_ferr  [ 7 ]     = Ferror32;
  					i_err   [ 8 ]     = Fadd32( ptr_fml_Sbuf ,FFO_EXPRY_DT,(char *) st_tradebook.c_expry_dt,0);
  					i_ferr  [ 8 ]     = Ferror32;
  					i_err   [ 9 ]     = Fadd32( ptr_fml_Sbuf ,FFO_EXER_TYP,(char *) &st_tradebook.c_exrc_typ,0);
  					i_ferr  [ 9 ]     = Ferror32;
  					i_err   [ 10  ]   = Fadd32( ptr_fml_Sbuf  ,FFO_OPT_TYP,(char *) &st_tradebook.c_opt_typ,0);
  					i_ferr  [ 10  ]   = Ferror32;
  					i_err   [ 11  ]   = Fadd32( ptr_fml_Sbuf ,FFO_STRK_PRC,(char *) &st_tradebook.l_strike_prc,0);
  					i_ferr  [ 11  ]   = Ferror32;
  					i_err   [ 12  ]   = Fadd32( ptr_fml_Sbuf ,FFO_CTGRY_INDSTK,(char *) &st_tradebook.c_ctgry_indstk,0);
  					i_ferr  [ 12  ]   = Ferror32;
  					i_err   [ 13  ]   = Fadd32( ptr_fml_Sbuf  ,FFO_CA_LVL,(char *) &st_tradebook.l_ca_lvl,0);
  					i_ferr  [ 13  ]   = Ferror32;
  					i_err   [ 14  ]   = Fadd32( ptr_fml_Sbuf ,FFO_TRD_FLW,(char *) &st_tradebook.c_trd_flw,0);
  					i_ferr  [ 14  ]   = Ferror32;
  					i_err   [ 15  ]   = Fadd32( ptr_fml_Sbuf ,FFO_EXEC_QTY,(char *) &st_tradebook.l_exctd_qty,0);
  					i_ferr  [ 15  ]   = Ferror32;
  					i_err   [ 16  ]   = Fadd32( ptr_fml_Sbuf  ,FFO_EXCTD_RT,(char *) &st_tradebook.l_exctd_rt,0);
  					i_ferr  [ 16  ]   = Ferror32;
  					i_err   [ 17  ]   = Fadd32( ptr_fml_Sbuf ,FFO_TRD_DT,(char *) st_tradebook.c_trd_dt,0);
  					i_ferr  [ 17  ]   = Ferror32;


  					for ( i=0 ; i < 18 ;  i++ )
  					{
    					if ( (i_err[ i ] == -1 ) && (i_ferr[ i ]  != FNOTPRES ) )
    					{
      					fn_userlog( c_ServiceName, "Error in :%d:",i );
      					fn_errlog( c_ServiceName, "L31480", Fstrerror32( i_ferr[i]),c_err_msg );
            		tpfree((char *)ptr_fml_Sbuf);
      					Fadd32( ptr_fml_Sbuf, FFO_ERR_MSG, c_err_msg, 0 );
                return -1;
    				  }
  					}

					 i_returncode = tpacall ( "SFO_PRCS_TRD", (char *)ptr_fml_Sbuf,0, TPNOTRAN|TPNOREPLY); 

						 if ( i_returncode != SUCC_BFR )
						 {
								fn_errlog ( c_ServiceName, "L31485", LIBMSG, c_err_msg );
            		tpfree((char *)ptr_fml_Sbuf);
								return -1;
						 }

             tpfree((char *)ptr_fml_Sbuf);  /*** Ver 5.5 ****/
					}
			}
	 }
	 return 0;
}
/*** Added function in Ver 5.3 Ends  ***/



/*** Version 6.1 starts ***/

/******************************************************************************/
/* To update the accept and reject responses from Exchange.				            */
/*  INPUT PARAMETERS                                                          */
/*      st_om_rspn_tr - Pointer to a message structure got from Exchange   		*/
/*			c_pipe_id     - Pipe Id																								*/
/*			i_dwnld_flg   - Down Load Flag																				*/
/*      c_ServiceName - Service Name currently running                        */
/*      c_err_msg     - Error message to be returned                          */
/*  OUTPUT PARAMETERS                                                         */
/*        0   - Success                                                       */
/*        -1  - Failure                                                       */
/******************************************************************************/

int fn_ord_confirmation_trim ( struct st_oe_rspn_tr *ptr_oe_rspn_tr,
										 			char   *c_xchng_cd,
										 			char   *c_pipe_id,
													char   *c_rout_str,
										 			char   *c_trd_dt,
													char   *c_skpord_ref,
										 			int    i_dwnld_flg,
                     			char   *c_ServiceName,
                     			char   *c_err_msg )

{
	char c_ordr_rfrnc [19];
	char c_xchng_rmrks[256];

	int i_ch_val;
  /*** ver 6.8 starts ***/
  char c_fod_spec_flg='\0';
  char c_fod_order_ref[30]="\0";
  char c_ord_ack_nse[20]="\0";
  /*** ver 6.8 ends ***/

	long l_xchng_can_qty;

	struct vw_xchngbook st_i_xchngbk;
  char c_prd_typ = '\0'; 

	MEMSET(st_i_xchngbk);
	MEMSET(c_xchng_rmrks);

	if(DEBUG_MSG_LVL_3) 
	{
		fn_userlog(c_ServiceName,"Inside Function fn_ord_cnfrmtn");
	}

	if(DEBUG_MSG_LVL_3)
	{
				fn_userlog(c_ServiceName,"I/P Exchange Code Is :%s:",c_xchng_cd);
				fn_userlog(c_ServiceName,"I/P Pipe ID Is :%s:",c_pipe_id);
				fn_userlog(c_ServiceName,"I/P Trade Date :%s:",c_trd_dt);
	}

	strcpy(st_i_xchngbk.c_xchng_cd,c_xchng_cd);
	strcpy(st_i_xchngbk.c_pipe_id,c_pipe_id);
	strcpy(st_i_xchngbk.c_mod_trd_dt,c_trd_dt);
	st_i_xchngbk.l_ord_seq = ptr_oe_rspn_tr->li_ordr_sqnc;

	if(DEBUG_MSG_LVL_0)
	{
				fn_userlog(c_ServiceName,"fn_ord_cnfrmtn:Sequence Number Is :%ld:",st_i_xchngbk.l_ord_seq); 
	}

	st_i_xchngbk.c_oprn_typ = FOR_RCV;

	switch (ptr_oe_rspn_tr->si_transaction_code)
	{
						case ORDER_CONFIRMATION_OUT_TR :
						case ORDER_MOD_CONFIRM_OUT_TR :
								if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"Inside ORDER ACCEPT/REJECT Case");
								}

								if ( i_dwnld_flg == DOWNLOAD && (strcmp(st_i_xchngbk.c_ordr_rfrnc,c_skpord_ref) == 0) )
								{
									break;
								}

								i_ch_val = fn_ord_acpt_rjct_trim(ptr_oe_rspn_tr,
																								 st_i_xchngbk,  
																								 c_skpord_ref,	 
																								 c_xchng_cd,
																								 c_pipe_id,
																								 c_rout_str,
																								 c_trd_dt,
																								 c_ordr_rfrnc,
																								 i_dwnld_flg,
																								 c_ServiceName,
																								 c_err_msg);

								if (i_ch_val == -1)
								{
												fn_userlog(c_ServiceName,"Failed While Calling Function fn_ord_acpt_rjct ");
												return -1;
								}
							
								if(DEBUG_MSG_LVL_3)  
								{
												fn_userlog(c_ServiceName,"After Successful Call To fn_ord_acpt_rjct");
								}

								break;
	
						case ORDER_CXL_CONFIRMATION_TR :
								if(DEBUG_MSG_LVL_3)
								{
												fn_userlog(c_ServiceName,"Inside ORDER_CANCEL_CONFIRM_OUT Case");
												fn_userlog(c_ServiceName,"st_i_xchngbk.c_req_typ = :%c:", st_i_xchngbk.c_req_typ);		
												fn_userlog(c_ServiceName,"The ERROR CODE ORDER_CANCEL_CONFIRM_OUT IS :%d:",ptr_oe_rspn_tr->si_error_code); 
								} 
             /********** commented in ver 6.8 **
								if ( i_dwnld_flg == DOWNLOAD  && ( (ptr_oe_rspn_tr->si_error_code != 16388) && (ptr_oe_rspn_tr->si_error_code != 17070) && (ptr_oe_rspn_tr->si_error_code != 17071) &&  (ptr_oe_rspn_tr->li_ordr_sqnc != 0)  ) ) 
								{
												  i_ch_val = fn_seq_to_omd (&st_i_xchngbk,c_ServiceName,c_err_msg);
												  if ( i_ch_val != 0 )
												  {
												       fn_errlog(c_ServiceName,"L91910", LIBMSG, c_err_msg);
												       return -1;
												  }
								}


								if ( (i_dwnld_flg == DOWNLOAD) && (st_i_xchngbk.c_req_typ != 'C') && 
								(ptr_oe_rspn_tr->si_error_code != 17070 && ptr_oe_rspn_tr->si_error_code != 17071 && ptr_oe_rspn_tr->si_error_code != 16388 &&  ptr_oe_rspn_tr->li_ordr_sqnc != 0 ) ) 		
								{
												fn_userlog(c_ServiceName,"solicited cancellation response received in download");
												fn_userlog(c_ServiceName,"Order Reference = :%s:",st_i_xchngbk.c_ordr_rfrnc); 
												break;
								}

                   *********** version 6.8  ends ***/

                /**** ver 6.8 starts ********/

                /*** loop executes in case of solicited cancellation and unsolicited with reason code 0 ***/
                fn_userlog(c_ServiceName,"CR_139156 before seqtomd li_ordr_sqnc :%ld:",ptr_oe_rspn_tr->li_ordr_sqnc);
                fn_userlog(c_ServiceName,"CR_139156 si_error_code :%d:",ptr_oe_rspn_tr->si_error_code);
                if( ptr_oe_rspn_tr->li_ordr_sqnc != 0 && ( ptr_oe_rspn_tr->si_error_code!= 17070 && ptr_oe_rspn_tr->si_error_code!= 17071 && ptr_oe_rspn_tr->si_error_code!= 16388  &&  ptr_oe_rspn_tr->st_ord_flg.flg_ioc !=1 ))
                {

                        memset(c_ord_ack_nse,'\0',sizeof(c_ord_ack_nse));
                        sprintf(c_ord_ack_nse,"%16.0lf",ptr_oe_rspn_tr->d_order_number);
                        fn_userlog(c_ServiceName,"CR_139156 c_ord_ack_nse:%s:",c_ord_ack_nse);

                        EXEC SQL
                        SELECT FOD_SPL_FLAG
                        INTO :c_fod_spec_flg
                        FROM FOD_FO_ORDR_DTLS
                        WHERE FOD_ACK_NMBR  = :c_ord_ack_nse
                        AND ROWNUM < 2;

                        if ( SQLCODE != 0 )
                        {
                            fn_errlog(c_ServiceName,"L91910", SQLMSG, c_err_msg);
                            fn_userlog(c_ServiceName,"CR_139156 ERROR IN FOD FETCH");
                            return -1;
                        }

                        fn_userlog(c_ServiceName,"CR_139156 c_fod_spec_flg :%c:",c_fod_spec_flg);
                        if( c_fod_spec_flg == 'R' || c_fod_spec_flg == 'O' || c_fod_spec_flg == '2' || c_fod_spec_flg == '3' || c_fod_spec_flg == 'J')
                        {
                          st_i_xchngbk.c_req_typ = '\0';

                          EXEC SQL
                          SELECT  Fxb_rqst_typ
                          INTO  :st_i_xchngbk.c_req_typ
                          FROM  FXB_FO_XCHNG_BOOK
                          WHERE  FXB_XCHNG_CD  = :st_i_xchngbk.c_xchng_cd
                          AND  FXB_PIPE_ID   = :st_i_xchngbk.c_pipe_id
                          AND  FXB_ORDR_RFRNC in ( select FOD_ORDR_RFRNC FROM FOD_FO_ORDR_DTLS WHERE FOD_ACK_NMBR  =:c_ord_ack_nse )
                          AND  FXB_MOD_TRD_DT = to_date(:st_i_xchngbk.c_mod_trd_dt,'dd-Mon-yyyy')
                          AND  FXB_RMS_PRCSD_FLG = 'N'
                          AND  ROWNUM < 2;

                         if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND)
                         {
                          fn_errlog(c_ServiceName,"L91910", SQLMSG, c_err_msg);
                          fn_userlog(c_ServiceName,"CR_139156 ERROR IN FXB FETCH");
                          return -1;
                         }
                         else if( SQLCODE == NO_DATA_FOUND)
                         {
                          fn_userlog(c_ServiceName,"CR_139156 NO DATA FOUND NO UNPRCESSED RED");
                         }

                       }
                       else
                       {
                          i_ch_val = fn_seq_to_omd (&st_i_xchngbk,c_ServiceName,c_err_msg);
                          if ( i_ch_val != 0 )
                          {
                               fn_errlog(c_ServiceName,"L91910", LIBMSG, c_err_msg);
                               return -1;
                          }
                       }
                }

                fn_userlog(c_ServiceName,"CR_139156 aftr seqtomd st_i_xchngbk.c_req_typ :%c: c_ordr_rfrnc :%s:",st_i_xchngbk.c_req_typ,st_i_xchngbk.c_ordr_rfrnc);

               /**** ver 6.8 ends ***/



								if((ptr_oe_rspn_tr->st_ord_flg.flg_ioc == 1) || (ptr_oe_rspn_tr->si_error_code == 17070 ) || (ptr_oe_rspn_tr->si_error_code == 17071) 
									|| ( ptr_oe_rspn_tr->li_ordr_sqnc == 0) || (ptr_oe_rspn_tr->si_error_code == 16388 ) || (st_i_xchngbk.c_req_typ != 'C')) /*** req typ added in ver 6.8 ***/
								{
													if(DEBUG_MSG_LVL_0)
													{
														fn_userlog(c_ServiceName,"The IOC flag    Is :%d:",ptr_oe_rspn_tr->st_ord_flg.flg_ioc); 
												    fn_userlog(c_ServiceName,"The error code  Is :%d:",ptr_oe_rspn_tr->si_error_code); 
													}

													i_ch_val = fn_ord_can_trim ( ptr_oe_rspn_tr,
																									c_xchng_cd,
																									c_pipe_id,
																									c_rout_str,
																									c_trd_dt,
																									c_ordr_rfrnc,
																									i_dwnld_flg,
																									c_ServiceName,
																									c_err_msg);

													if (i_ch_val == -1)
													{
												  	fn_userlog(c_ServiceName,"Failed While Calling Function fn_ord_can ");
												  	return -1;
												 	}
								}
								else 
								{
														if(DEBUG_MSG_LVL_3)
														{
															fn_userlog(c_ServiceName,"Inside Normal Cancellation Handling");
														}
														i_ch_val = fn_ord_acpt_rjct_trim(ptr_oe_rspn_tr,
																														 st_i_xchngbk,  
																															c_skpord_ref,   
																															c_xchng_cd,
																															c_pipe_id,
																															c_rout_str,
																															c_trd_dt,
																															c_ordr_rfrnc,
																															i_dwnld_flg,
																															c_ServiceName,
																															c_err_msg);

														if (i_ch_val == -1)
														{
													  	fn_userlog(c_ServiceName,"Failed While Calling Function fn_ord_acpt_rjct ");
													 	  return -1;
														}
								}

								break;
						default :
											fn_userlog( c_ServiceName, "Protocol error %d", ptr_oe_rspn_tr->si_transaction_code);
											break;

	}
}


int fn_ord_acpt_rjct_trim( struct st_oe_rspn_tr *ptr_oe_rspn_tr,
														struct vw_xchngbook st_xchngbk, 
														char *c_skpord_ref,							
														char *c_xchng_cd,
														char *c_pipe_id,
														char *c_rout_str,
														char *c_trd_dt,
														char *c_ordr_rfrnc,
														int  i_dwnld_flg,
														char *c_ServiceName,
														char *c_err_msg)
{
	struct vw_xchngbook st_i_xchngbk;
	struct vw_orderbook st_i_ordrbk;

	char c_tmp[8];
	char c_svc_name[15+1];
	char c_xchng_rmrks[256];
	char c_mtch_accnt[11];
	char c_xchng_err_msg[256];
	char c_spn_allwd_flg;

  char c_stream_no='\0';

	int i_ch_val;
	int i_trnsctn;
  int i_rec_exists = 0; 
	char c_lst_act_ref [22];		/*** Parag ***/
	long long ll_log_tm_stmp;

  char c_stream_no_str[2]="\0"; /** 6.8 ver **/

	if(DEBUG_MSG_LVL_3)   
	{
		fn_userlog(c_ServiceName,"Inside fn_ord_acpt_rjct Function");
	}

	MEMSET(st_i_xchngbk);
	MEMSET(st_i_ordrbk);
	MEMSET(c_xchng_rmrks);
	MEMSET(c_xchng_err_msg);
	MEMSET(c_lst_act_ref);	/*** Parag ***/

	memcpy(&st_i_xchngbk,&st_xchngbk,sizeof(st_xchngbk));

	switch (ptr_oe_rspn_tr->si_transaction_code)
	{
		case ORDER_CONFIRMATION_OUT_TR :

					if(DEBUG_MSG_LVL_3)  
		      { 
							fn_userlog(c_ServiceName,"Exchange Entry Date Time Is :%ld:",ptr_oe_rspn_tr->li_entry_date_time);
							fn_userlog(c_ServiceName,"The ERROR CODE is :%d:",ptr_oe_rspn_tr->si_error_code);
							fn_userlog(c_ServiceName,"The REASON CODE is :%d:",ptr_oe_rspn_tr->si_reason_code );
					}

					fn_long_to_timearr ( st_i_xchngbk.c_entry_dt_tm,ptr_oe_rspn_tr->li_entry_date_time );

					if( ptr_oe_rspn_tr->si_error_code != 0 || ptr_oe_rspn_tr->si_reason_code != 0)
		      {
              if(DEBUG_MSG_LVL_3)
              {
							 fn_userlog(c_ServiceName,"Order Rejection Case");
              }

							st_i_xchngbk.l_ors_msg_typ = ORS_NEW_ORD_RJCT;

              if(DEBUG_MSG_LVL_3)
              {
							  fn_userlog(c_ServiceName,"Exchange Entry date Time and Exchange Ack Time Is :%ld: ",ptr_oe_rspn_tr->li_entry_date_time);
              }

							fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_rspn_tr->li_entry_date_time);


							i_ch_val = fn_get_errmsg( ptr_oe_rspn_tr->si_error_code,ptr_oe_rspn_tr->si_reason_code,c_xchng_err_msg,c_ServiceName);
							if(i_ch_val == -1 )
							{
							  fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
							  return -1;
							}
	
							strcpy(c_xchng_rmrks,c_xchng_err_msg);
					}
					else
		      {
							if(DEBUG_MSG_LVL_3)  
							{
										fn_userlog(c_ServiceName,"Order Confirmation Case");
							}
 
							if(DEBUG_MSG_LVL_3)
							{
										fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_rspn_tr->li_last_modified);
							}
              st_i_xchngbk.l_ors_msg_typ = ORS_NEW_ORD_ACPT;
						  fn_long_to_timearr ( st_i_xchngbk.c_ack_tm,ptr_oe_rspn_tr->li_last_modified );
					}

					if(DEBUG_MSG_LVL_3)
					{
							fn_userlog(c_ServiceName,"Message Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
							fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
							fn_userlog(c_ServiceName,"Exchange Book Entry Date Time :%s:",st_i_xchngbk.c_entry_dt_tm);
					}

					break;

		case ORDER_MOD_CONFIRM_OUT_TR :
		
					if(DEBUG_MSG_LVL_3)  
		      { 
							fn_userlog(c_ServiceName,"The ERROR CODE is :%d:",ptr_oe_rspn_tr->si_error_code);
							fn_userlog(c_ServiceName,"The REASON CODE is :%d:",ptr_oe_rspn_tr->si_reason_code );
							fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_rspn_tr->li_last_modified);
          }

          fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_rspn_tr->li_last_modified);

					if( ptr_oe_rspn_tr->si_error_code != 0 || ptr_oe_rspn_tr->si_reason_code != 0)
		      {
							st_i_xchngbk.l_ors_msg_typ = ORS_MOD_ORD_RJCT ;
							i_ch_val = fn_get_errmsg( ptr_oe_rspn_tr->si_error_code,ptr_oe_rspn_tr->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

							if(i_ch_val == -1 )
							{
							  fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
							  return -1;
							}
							strcpy(c_xchng_rmrks,c_xchng_err_msg);

          }
          else
		      {
							if(DEBUG_MSG_LVL_3)  
							{
											fn_userlog(c_ServiceName,"ORDER_MOD_CONFIRM_OUT Case");
							}

              st_i_xchngbk.l_ors_msg_typ = ORS_MOD_ORD_ACPT ;

					}		

					if(DEBUG_MSG_LVL_3)
					{
								fn_userlog(c_ServiceName,"Message Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
								fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);		
					}

					break;
	
		case ORDER_CXL_CONFIRMATION_TR :

					if(DEBUG_MSG_LVL_3)  
					{
						    fn_userlog(c_ServiceName,"The ERROR CODE is :%d:",ptr_oe_rspn_tr->si_error_code);
					      fn_userlog(c_ServiceName,"The REASON CODE is :%d:",ptr_oe_rspn_tr->si_reason_code );
								fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_rspn_tr->li_last_modified);
					}
 
					fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_rspn_tr->li_last_modified);


					if( ptr_oe_rspn_tr->si_error_code != 0 || ptr_oe_rspn_tr->si_reason_code != 0)
		      {
						  st_i_xchngbk.l_ors_msg_typ = ORS_CAN_ORD_RJCT;
							i_ch_val = fn_get_errmsg( ptr_oe_rspn_tr->si_error_code,ptr_oe_rspn_tr->si_reason_code,(char *)c_xchng_err_msg,c_ServiceName);

							if(i_ch_val == -1 )
							{
							  fn_userlog(c_ServiceName,"Failed while fetching the Error Code ");
							  return -1;
							}

							strcpy(c_xchng_rmrks,c_xchng_err_msg);
          }
					else
		      {
							st_i_xchngbk.l_ors_msg_typ = ORS_CAN_ORD_ACPT ;
				
						  st_i_xchngbk.l_xchng_can_qty = ptr_oe_rspn_tr->li_volume_filled_today; 

						  if( DEBUG_MSG_LVL_0 )
						  {
						    fn_userlog( c_ServiceName,"In cancellation acceptance, st_i_xchngbk.l_xchng_can_qty is :%ld:",st_i_xchngbk.l_xchng_can_qty );
						  }
			
					}
	
					if(DEBUG_MSG_LVL_3)
					{
								fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
					}

					break;

		default :

				fn_userlog(c_ServiceName, "Protocol error %d", ptr_oe_rspn_tr->si_transaction_code);
				break;
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_ordrbk.c_rout_crt,c_rout_str);	

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "Before Call To SFO_REF_TO_ORD");
	}
	
  st_i_ordrbk.c_oprn_typ = FOR_VIEW;

  if( DEBUG_MSG_LVL_3 )  
  {
	  fn_userlog(c_ServiceName,"Exchange Product Type is :%c:",ptr_oe_rspn_tr->st_con_desc_tr.c_instrument_name[0]);
  }

  if (ptr_oe_rspn_tr->st_con_desc_tr.c_instrument_name[0] == 'F')
  {
    strcpy( c_svc_name , "SFO_FUT_ACK" );
  }
  else
  {
    strcpy( c_svc_name , "SFO_OPT_ACK" );
  }


	switch (st_i_xchngbk.l_ors_msg_typ)
	{
		case  ORS_NEW_ORD_ACPT :
 	  case  ORS_MOD_ORD_ACPT :
	  case  ORS_CAN_ORD_ACPT :
		
			st_i_xchngbk.c_plcd_stts = ACCEPT;
      break;

		case  ORS_NEW_ORD_RJCT :
	  case  ORS_MOD_ORD_RJCT :
	  case  ORS_CAN_ORD_RJCT :

			st_i_xchngbk.c_plcd_stts = REJECT;
      break;
	}
 	
	st_i_xchngbk.c_rms_prcsd_flg = NOT_PROCESSED;
	strcpy ( st_i_xchngbk.c_xchng_rmrks ,c_xchng_rmrks );  

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",st_i_xchngbk.c_xchng_rmrks);
	}

	memcpy ( &ll_log_tm_stmp,ptr_oe_rspn_tr->c_time_stamp_1, 8 );
	st_i_xchngbk.d_jiffy = (double) ll_log_tm_stmp;

	fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_xchngbk.d_jiffy);

	st_i_xchngbk.l_dwnld_flg = i_dwnld_flg;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Down Load Flag Is :%ld:",st_i_xchngbk.l_dwnld_flg);
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);	
	
	st_i_xchngbk.c_oprn_typ = UPDATION_ON_EXCHANGE_RESPONSE;

  /**st_i_xchngbk.l_bit_flg = (long)ptr_oe_rspn_tr->c_time_stamp_2; **/
  /**c_stream_no = ptr_oe_rspn_tr->c_time_stamp_2; **** Ver 7.1 **/
  /* st_i_xchngbk.l_bit_flg = atol(&c_stream_no);   commented in 6.8 version **/
  /** c_stream_no_str[0] = c_stream_no; ** ver 6.8 ** ** Ver 7.1 commented**/

   /*** st_i_xchngbk.l_bit_flg = atol(c_stream_no_str); ** ver 6.8 ** ommented in ver 7.1**/
  st_i_xchngbk.l_bit_flg = (long)ptr_oe_rspn_tr->c_time_stamp_2; /** Ver 7.1 **/

   fn_userlog(c_ServiceName,"CR_139156 BIT BIT :%ld:",st_i_xchngbk.l_bit_flg ); /** ver 6.8 **/

	if(DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName,"IN NORM_ACPT_RJCT, the stream number receievd from exchange is:%ld:",st_i_xchngbk.l_bit_flg);	
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Before Call To SFO_UPD_XCHNGBK");
	}

	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"L31490", LIBMSG, c_err_msg);
    return -1;
  }

  if ( st_i_xchngbk.l_dwnld_flg == DOWNLOAD )
  {
			EXEC SQL
			  SELECT 1
			  INTO :i_rec_exists
			  FROM FXB_FO_XCHNG_BOOK
			  WHERE FXB_JIFFY   = :st_i_xchngbk.d_jiffy
			  AND FXB_XCHNG_CD  = :st_i_xchngbk.c_xchng_cd
			  AND FXB_PIPE_ID   = :st_i_xchngbk.c_pipe_id
        AND   FXB_ORDR_SQNC     = :st_i_xchngbk.l_ord_seq; /** Ver 6.7 ***/
			  /*** AND FXB_STREAM_NO = :st_i_xchngbk.l_bit_flg; *** Ver 6.6 ***/

			if ( ( SQLCODE != 0 ) && (SQLCODE != NO_DATA_FOUND) )
			{
			  fn_errlog(c_ServiceName,"L31495", LIBMSG, c_err_msg);
			  fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
			  return -1;
			}
			if ( i_rec_exists == 1 )
			{
			  fn_userlog ( c_ServiceName, "Record already processed for order confirmation" );
			  fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
			  return 0;
			}
  }

  if(DEBUG_MSG_LVL_3)
  {
  	fn_userlog(c_ServiceName,"Last Activity Ref Before in trim is :%lld:",ptr_oe_rspn_tr->ll_lastactivityref);
  }

	sprintf(c_lst_act_ref,"%lld",ptr_oe_rspn_tr->ll_lastactivityref);	/*** Parag ***/
	rtrim(c_lst_act_ref);		/*** Parag ***/

	if(DEBUG_MSG_LVL_3)
  {
  	fn_userlog(c_ServiceName,"Exchange code :%s:,order sequence :%ld:,pipe id :%s:",st_i_xchngbk.c_xchng_cd,st_i_xchngbk.l_ord_seq,st_i_xchngbk.c_pipe_id);
		fn_userlog(c_ServiceName,"Last Activity Ref in trim is :%s:",c_lst_act_ref);     
	}

  MEMSET(sql_fxb_ordr_rfrnc);
  sql_fxb_mdfctn_cntr = 0;

 if( st_i_xchngbk.l_ors_msg_typ == ORS_CAN_ORD_ACPT )
 {
  EXEC SQL
    UPDATE  FXB_FO_XCHNG_BOOK
    SET FXB_PLCD_STTS       = :st_i_xchngbk.c_plcd_stts,
        FXB_RMS_PRCSD_FLG   = :st_i_xchngbk.c_rms_prcsd_flg,
        FXB_ORS_MSG_TYP     = :st_i_xchngbk.l_ors_msg_typ,
        FXB_ACK_TM          = to_date( :st_i_xchngbk.c_ack_tm,'DD-Mon-yyyy hh24:mi:ss' ),
        FXB_XCHNG_RMRKS     = rtrim(fxb_xchng_rmrks)||:st_i_xchngbk.c_xchng_rmrks,
        FXB_JIFFY           = :st_i_xchngbk.d_jiffy,
        FXB_STREAM_NO       = :st_i_xchngbk.l_bit_flg,
        FXB_XCHNG_CNCLD_QTY = :st_i_xchngbk.l_xchng_can_qty,
        FXB_LST_ACT_REF     = :c_lst_act_ref    /*** Parag ***/
    WHERE FXB_XCHNG_CD      = :st_i_xchngbk.c_xchng_cd
    AND   FXB_ORDR_SQNC     = :st_i_xchngbk.l_ord_seq
    AND   FXB_PIPE_ID       = :st_i_xchngbk.c_pipe_id
    AND   FXB_MOD_TRD_DT    = :c_trd_dt
    RETURNING FXB_ORDR_RFRNC,
              FXB_MDFCTN_CNTR
    INTO      :sql_fxb_ordr_rfrnc,
              :sql_fxb_mdfctn_cntr;
 }
 else
 {
  EXEC SQL
    UPDATE  FXB_FO_XCHNG_BOOK
    SET FXB_PLCD_STTS       = :st_i_xchngbk.c_plcd_stts,
        FXB_RMS_PRCSD_FLG   = :st_i_xchngbk.c_rms_prcsd_flg,
        FXB_ORS_MSG_TYP     = :st_i_xchngbk.l_ors_msg_typ,
        FXB_ACK_TM          = to_date( :st_i_xchngbk.c_ack_tm,'DD-Mon-yyyy hh24:mi:ss' ),
        FXB_XCHNG_RMRKS     = rtrim(fxb_xchng_rmrks)||:st_i_xchngbk.c_xchng_rmrks,
        FXB_JIFFY           = :st_i_xchngbk.d_jiffy,
        FXB_STREAM_NO       = :st_i_xchngbk.l_bit_flg,
				FXB_LST_ACT_REF			= :c_lst_act_ref		/*** Parag ***/
    WHERE FXB_XCHNG_CD      = :st_i_xchngbk.c_xchng_cd
    AND   FXB_ORDR_SQNC     = :st_i_xchngbk.l_ord_seq
    AND   FXB_PIPE_ID       = :st_i_xchngbk.c_pipe_id
    AND   FXB_MOD_TRD_DT    = :c_trd_dt
    RETURNING FXB_ORDR_RFRNC,
              FXB_MDFCTN_CNTR
    INTO      :sql_fxb_ordr_rfrnc,
              :sql_fxb_mdfctn_cntr;
   }
  
	 if (SQLCODE != 0 ) 
   {
     fn_errlog(c_ServiceName,"L31500", SQLMSG, c_err_msg);
     fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
     return -1;
   }


	
  if ( i_dwnld_flg == DOWNLOAD && (strcmp(sql_fxb_ordr_rfrnc,c_skpord_ref) == 0) )
  {
		fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
		return 0; 
  }

  strcpy(st_i_xchngbk.c_ordr_rfrnc,sql_fxb_ordr_rfrnc);
  st_i_xchngbk.l_mdfctn_cntr=sql_fxb_mdfctn_cntr;

	if(DEBUG_MSG_LVL_0)
  {
  	fn_userlog(c_ServiceName,"Order reference:%s:,Modification cnt :%d:",sql_fxb_ordr_rfrnc,sql_fxb_mdfctn_cntr);
	}

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_XCHNGBK :%s:",c_time);
	}

	strcpy(st_i_ordrbk.c_ordr_rfrnc,st_i_xchngbk.c_ordr_rfrnc); 
	sprintf( st_i_ordrbk.c_xchng_ack,"%16.0lf", ptr_oe_rspn_tr->d_order_number ); 

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Exchange Ack Is :%lf:",ptr_oe_rspn_tr->d_order_number);
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		fn_userlog(c_ServiceName,"Order Book Exchange Ack Is :%s:",st_i_ordrbk.c_xchng_ack);
	}

	strcpy(st_i_ordrbk.c_rout_crt,c_rout_str);	

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Before Call To SFO_UPD_ORDRBK");
	}

  st_i_ordrbk.c_oprn_typ = UPDATE_XCHNG_RESPONSE;

	if(DEBUG_MSG_LVL_3)   
  {
		fn_userlog(c_ServiceName,"st_i_xchngbk.l_ors_msg_typ is  :%ld:",st_i_xchngbk.l_ors_msg_typ);
	}

  if (st_i_xchngbk.l_ors_msg_typ == ORS_NEW_ORD_ACPT || st_i_xchngbk.l_ors_msg_typ == ORS_NEW_ORD_RJCT )
  {
		i_ch_val = fn_upd_ordrbk (&st_i_ordrbk,c_ServiceName,c_err_msg);

  	if ( i_ch_val != 0)
  	{
   	 	fn_errlog(c_ServiceName,"L31505", LIBMSG, c_err_msg);
    	fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    	return -1;
  	}

		fn_gettime();

		if(DEBUG_MSG_LVL_3)  
		{
			fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_ORDRBK :%s:",c_time);
      fn_userlog(c_ServiceName,"After callng to function fn_upd_ordrbk.");
		}
	}

	i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
	
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"L31510", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Commit :%s:",c_time);
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);  

	i_ch_val = fn_acall_svc ( c_ServiceName,
                             c_err_msg,
                             &st_i_xchngbk,
                             "vw_xchngbook",
                             sizeof ( st_i_xchngbk ),
                             TPNOREPLY,
                             c_svc_name );
  if ( i_ch_val != 0 )
  {
    fn_errlog(c_ServiceName,"L31515", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"ORS2-RD After Call To FUT/OPT ACK :%s:",c_time);
		fn_userlog(c_ServiceName,"After Complition Of fn_ord_acpt_rjct");
	}

	return 0;
}



int fn_trd_confirmation_trim ( struct st_trade_confirm_tr *ptr_st_trade_confirm_tr,
													char *c_xchng_cd,
                     			char *c_pipe_id,
												  char *c_rout_str,
													char *c_trd_dt,
													char *c_trd_ref,
										 			int i_dwnld_flg,
                     			char *c_ServiceName,
                     			char *c_err_msg )
{
	int i_trnsctn;
  int i_ch_val;
	int i_rec_cnt=0;    

	long long ll_log_tm_stmp;
	long long ll_log_tm_stmp_1;

	long li_ftt_stream_no;					

	double d_tm_stmp;

  struct vw_orderbook st_i_ordbk;
  struct vw_tradebook st_i_trdbk;
  struct vw_sequence st_seq;
  struct vw_contract st_cntrct;

  char c_svc_name [ 16 ];
  char c_refack_tmp[LEN_ORD_REFACK];
	char rfrnc[19],c_date[5] ;
	char ack[LEN_XCHNG_ACK];
	char c_mtch_accnt [10 + 1];
	char c_xchng_ack[17];
	char c_utrd_dt[23];
	char c_tmpjiffy[16];
	char c_trg_filter[17];
	char c_usr_id[16];				   
	char c_span_flag  = 'N';     
	char c_lst_act_ref [22];
	MEMSET(c_lst_act_ref);

  int i2,i3 =0 ;
	
	MEMSET(st_i_ordbk);
	MEMSET(st_i_trdbk);
	MEMSET(st_seq);
	MEMSET(st_cntrct);

	if(DEBUG_MSG_LVL_3)  
	{
		fn_userlog(c_ServiceName,"Inside Function fn_trd_cnfrmtn_trim");
	}


	sprintf(st_i_ordbk.c_xchng_ack,"%16.0lf",ptr_st_trade_confirm_tr->d_response_order_number);
	strcpy ( st_i_ordbk.c_pipe_id , c_pipe_id );

	if(DEBUG_MSG_LVL_0)
	{
    fn_userlog(c_ServiceName,"fn_trd_cnfrmtn_trim:Exchange Ack Is :%s:",st_i_ordbk.c_xchng_ack);
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",st_i_ordbk.c_pipe_id);
    fn_userlog(c_ServiceName,"EXCHANGE JIFFY IS :%s:",ptr_st_trade_confirm_tr->c_timestamp1);
	}

	strcpy(c_tmpjiffy,ptr_st_trade_confirm_tr->c_timestamp1);

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_ordbk.c_rout_crt,c_rout_str);	
  st_i_ordbk.c_oprn_typ = FOR_VIEW;

	memcpy (&ll_log_tm_stmp_1, ptr_st_trade_confirm_tr->c_timestamp1, 8 );
	d_tm_stmp = (double)ll_log_tm_stmp_1;

  sprintf(c_lst_act_ref,"%lld",ptr_st_trade_confirm_tr->ll_lastactivityref);  /*** Parag ***/ /*** Ver 6.4 ***/
  rtrim(c_lst_act_ref);     /*** Ver 6.4 ***/

  if(DEBUG_MSG_LVL_5)       /*** Added in Ver 6.4 ***/
  {
    fn_userlog(c_ServiceName,"Last Actiivity ref in trade confirmation is :%s:",c_lst_act_ref);
  }

	while (i_rec_cnt != 2)    
	{

		i_ch_val = fn_ack_to_ord (&st_i_ordbk,c_ServiceName,c_usr_id,c_err_msg);			

  	if ( i_ch_val != 0)
  	{
			if ( i_ch_val == TOO_MANY_ROWS_FETCHED )
    	{
				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"TOO_MANY_ROWS_FETCHED");
				}
      	if ( strcmp ( c_xchng_cd, "NFO" ) == 0 )
      	{
					fn_userlog(c_ServiceName,"Before Call To SFO_AC_TO_ORD");
        	i_ch_val = fn_nse_to_eba_tkn ( ptr_st_trade_confirm_tr->l_token,
         	                               &st_cntrct,
																				 c_rout_str,
            	                           c_ServiceName,
              	                         c_err_msg );
        	if ( i_ch_val != 0 )
        	{
						fn_errlog(c_ServiceName,"L31520", LIBMSG, c_err_msg);
          	return -1;
        	}

        	strcpy ( st_i_ordbk.c_xchng_cd, st_cntrct.c_xchng_cd );
        	st_i_ordbk.c_prd_typ = st_cntrct.c_prd_typ;
        	strcpy ( st_i_ordbk.c_expry_dt, st_cntrct.c_expry_dt );
        	st_i_ordbk.c_exrc_typ = st_cntrct.c_exrc_typ;
        	st_i_ordbk.c_opt_typ = st_cntrct.c_opt_typ;
        	st_i_ordbk.l_strike_prc = st_cntrct.l_strike_prc;
        	st_i_ordbk.c_ctgry_indstk = st_cntrct.c_ctgry_indstk;
        	strcpy ( st_i_ordbk.c_undrlyng, st_cntrct.c_undrlyng );
        	st_i_ordbk.l_ca_lvl = st_cntrct.l_ca_lvl;
					sprintf(st_i_ordbk.c_xchng_ack,"%16.0lf",ptr_st_trade_confirm_tr->d_response_order_number);
        	strcpy ( st_i_ordbk.c_pipe_id , c_pipe_id );


					if(DEBUG_MSG_LVL_3)
					{
						fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
					}
					strcpy(st_i_ordbk.c_rout_crt,c_rout_str);	

        	st_i_ordbk.c_oprn_typ = FOR_VIEW;
					


					i_ch_val	=	fn_ac_to_ord (	c_ServiceName,		
																			c_err_msg,
																			c_usr_id,
																			&st_i_ordbk
																	 );

        	if ( i_ch_val !=  0 )
        	{
          	fn_errlog(c_ServiceName,"L31525", LIBMSG, c_err_msg);
          	return -1;
        	}
		
					fn_gettime();

					if(DEBUG_MSG_LVL_3)
					{
						fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_AC_TO_ORD :%s:",c_time);
					}
      	}
      	else
      	{
        	fn_errlog(c_ServiceName,"S31225", "Logic error", c_err_msg);
        	return -1;
      	}

			}
			else
			{
				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Order Confirmation Is Not Processed");
          fn_userlog(c_ServiceName,"Before Going To Sleep For 100 MicroSeconds"); 
				}


        usleep(5000);   
        i_rec_cnt ++;

				if(DEBUG_MSG_LVL_1)  
				{
					fn_userlog(c_ServiceName,"Record Count Is :%d: for :%s:",i_rec_cnt,st_i_ordbk.c_xchng_ack); 
				}
    	}
	
		}
	
		if(i_ch_val == 0)
  	{
			/*** Here Value of i_ch_val depends on call to SFO_ACK_TO_ORD For Normal Order & SFO_AC_TO_ORD For Spread Order ***/

			if(DEBUG_MSG_LVL_3)
			{
				fn_userlog(c_ServiceName,"Inside Success Condition");
			}

  		break;
  	}
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_ACK_TO_ORD :%s:",c_time);
	}

	if(i_rec_cnt == 2)    
  {

	
		sprintf ( c_trg_filter, "FFO_FILTER=='NFO'" );	
		
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Before Inserting Into FTT Table");
			fn_userlog(c_ServiceName,"Before Inserting Order Number Is :%lf:",ptr_st_trade_confirm_tr->d_response_order_number);
			fn_userlog(c_ServiceName,"Before Inserting Pipe ID Is :%s:",c_pipe_id);
			fn_userlog(c_ServiceName,"Before Inserting Token No. Is :%ld:",ptr_st_trade_confirm_tr->l_token);
			fn_userlog(c_ServiceName,"Before Inserting Trade Date Is :%s:",c_trd_dt);
			fn_userlog(c_ServiceName,"Before Inserting Trade Ref Is :%s:",c_trd_ref);
			fn_userlog(c_ServiceName,"Before Inserting Fill No. Is :%ld:",ptr_st_trade_confirm_tr->li_fill_number);
			fn_userlog(c_ServiceName,"Before Inserting Activity Time Is :%ld:",ptr_st_trade_confirm_tr->li_activity_time);
			fn_userlog(c_ServiceName,"Before Inserting Fill Qty Is ;%ld:",ptr_st_trade_confirm_tr->li_fill_quantity);
			fn_userlog(c_ServiceName,"Before Inserting Fill Price Is :%ld:",ptr_st_trade_confirm_tr->li_fill_price);
			fn_userlog(c_ServiceName,"Before Inserting Time Stamp Is :%lf:",d_tm_stmp); /*** jiffy ***/
			fn_userlog(c_ServiceName,"Trigger Filter Is :%s:",c_trg_filter);
		}

		fn_long_to_timearr(c_utrd_dt,ptr_st_trade_confirm_tr->li_activity_time);
		sprintf(c_xchng_ack,"%16.0lf",ptr_st_trade_confirm_tr->d_response_order_number);
		
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Before Inserting Activity Time Is ;%s:",c_utrd_dt);
			fn_userlog(c_ServiceName,"Before Inserting Ack Is :%s:",c_xchng_ack);	
		}

    i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
    if ( i_trnsctn == -1)
    {
      fn_errlog(c_ServiceName,"L31530", LIBMSG, c_err_msg);
      return -1;
    }

		/** li_ftt_stream_no = (long) ptr_st_trade_confirm_tr->c_time_stamp_2[LEN_TIME_STAMP -1]; **/
		/**li_ftt_stream_no  = (long) ptr_st_trade_confirm_tr->d_timestamp2; ***/

/*    sprintf(c_lst_act_ref,"%lld",ptr_st_trade_confirm_tr->ll_lastactivityref);  *** Commented in ver 6.4 ***
    rtrim(c_lst_act_ref);   *** Parag *** Ver 6.4 ***/

    /*** We are assigning li_ftt_stream_no as 0 because while final processing of the trade **
      ** stream number gets inserted from FXB table ***/

    li_ftt_stream_no = 0; /*** ver 6.4 ***/
		if(DEBUG_MSG_LVL_0)
  	{
			fn_userlog(c_ServiceName,"Before Inserting in ftt_fo_tmp_trd");
    	fn_userlog(c_ServiceName,"Transaction Code IS  :%d:",ptr_st_trade_confirm_tr->si_transaction_code);
    	fn_userlog(c_ServiceName,"Last Activity Ref is :%s:",c_lst_act_ref);
  	}

    EXEC SQL
      INSERT INTO ftt_fo_tmp_trd
			(
				FTT_ACK_NMBR,
				FTT_PIPE_ID,
				FTT_TOKEN_NO,
				FTT_XCHNGTRD_NO,
				FTT_TRD_TM,
				FTT_EXCTD_QTY,
				FTT_EXCTD_RT,
				FTT_JIFFY,
				FTT_PRCSD_FLG,
				FTT_STREAM_NO,
        FTT_XCHNG_CD,
				FTT_LST_ACT_REF,
				FTT_TRANS_CD
			)
      VALUES
      (
        to_char(:st_i_ordbk.c_xchng_ack),
        :c_pipe_id,
        :ptr_st_trade_confirm_tr->l_token,
        :ptr_st_trade_confirm_tr->li_fill_number,
        to_date(:c_utrd_dt,'DD-Mon-yyyy hh24:mi:ss'),
        :ptr_st_trade_confirm_tr->li_fill_quantity,
        :ptr_st_trade_confirm_tr->li_fill_price,
				:d_tm_stmp,
        'N',
				:li_ftt_stream_no,						
        :c_xchng_cd,
				:c_lst_act_ref,
				:ptr_st_trade_confirm_tr->si_transaction_code
      );

    if ( SQLCODE != 0 )
    {
      fn_errlog ( c_ServiceName, "S31230",SQLMSG, c_err_msg);
      fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
      return -1;
    }

    i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
    if ( i_ch_val == -1 )
    {
      fn_errlog(c_ServiceName,"L31535", LIBMSG, c_err_msg);
      return -1;
    }

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Before Posting Trigger TRG_PRCS_TRD");
		}

    fn_pst_trg(c_ServiceName,"TRG_PRCS_TRD","TRG_PRCS_TRD",c_trg_filter);
		
		return 0;			
  }

	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );
  if ( i_trnsctn == -1)
  {
    fn_errlog(c_ServiceName,"L31540", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Transaction Begin :%s:",c_time);
	}

  strcpy ( st_seq.c_pipe_id, st_i_ordbk.c_pipe_id );
  strcpy ( st_seq.c_trd_dt, ( char * )c_trd_dt );
	
	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",st_seq.c_pipe_id);
		fn_userlog(c_ServiceName,"Trade Date Is :%s:",st_seq.c_trd_dt);
	}
 
 
	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}
	strcpy(st_seq.c_rout_crt,c_rout_str);	
  st_seq.c_rqst_typ = GET_TRD_SEQ;

	i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_seq,
                           &st_seq,
                           "vw_sequence",
                           "vw_sequence",
                           sizeof ( st_seq ),
                           sizeof ( st_seq ),
                           0,
                           "SFO_GET_SEQ" );

  if ( i_ch_val != SUCC_BFR )
  {
    fn_errlog(c_ServiceName,"L31545", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }
	
	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_GET_SEQ :%s:",c_time);
		fn_userlog(c_ServiceName,"Sequence Number Is :%ld:",st_seq.l_seq_num);
		fn_userlog(c_ServiceName,"Trade Reference Is :%s:",c_trd_ref);
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",c_pipe_id);
	}

	sprintf ( st_i_trdbk.c_trd_rfrnc, "%s%s%08ld",c_trd_ref, c_pipe_id, st_seq.l_seq_num );
	strcpy ( st_i_trdbk.c_cln_mtch_accnt , st_i_ordbk.c_cln_mtch_accnt );
	strcpy ( c_mtch_accnt , st_i_ordbk.c_cln_mtch_accnt );
  strcpy ( st_i_trdbk.c_xchng_cd , st_i_ordbk.c_xchng_cd );
  st_i_trdbk.c_prd_typ = st_i_ordbk.c_prd_typ;
  strcpy ( st_i_trdbk.c_undrlyng , st_i_ordbk.c_undrlyng );
  strcpy ( st_i_trdbk.c_expry_dt , st_i_ordbk.c_expry_dt );
  st_i_trdbk.c_exrc_typ = st_i_ordbk.c_exrc_typ;
  st_i_trdbk.c_opt_typ = st_i_ordbk.c_opt_typ;
  st_i_trdbk.l_strike_prc = st_i_ordbk.l_strike_prc;
  st_i_trdbk.c_ctgry_indstk = st_i_ordbk.c_ctgry_indstk;
  strcpy ( st_i_trdbk.c_ordr_rfrnc , st_i_ordbk.c_ordr_rfrnc );
  strcpy ( st_i_trdbk.c_xchng_ack , st_i_ordbk.c_xchng_ack );
 	st_i_trdbk.l_xchng_trd_no = ptr_st_trade_confirm_tr->li_fill_number;
	fn_long_to_timearr(st_i_trdbk.c_trd_dt,ptr_st_trade_confirm_tr->li_activity_time);
	st_i_trdbk.c_trd_flw = st_i_ordbk.c_ordr_flw;
  st_i_trdbk.l_exctd_qty = ptr_st_trade_confirm_tr->li_fill_quantity;
	st_i_trdbk.l_exctd_rt = ptr_st_trade_confirm_tr->li_fill_price;
	st_i_trdbk.l_dwnld_flg = i_dwnld_flg;
	memcpy (&ll_log_tm_stmp, ptr_st_trade_confirm_tr->c_timestamp1, 8 );
	st_i_trdbk.d_jiffy = (double) ll_log_tm_stmp;
	
	/** st_i_trdbk.l_ca_lvl	=	 (long) ptr_st_trade_confirm_tr->st_hdr.c_time_stamp_2[LEN_TIME_STAMP -1]; **/			
  st_i_trdbk.l_ca_lvl	=	 (long) ptr_st_trade_confirm_tr->d_timestamp2;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
	}

	strcpy(st_i_trdbk.c_rout_crt,c_rout_str); 
	
	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Trade Reference Is :%s:",st_i_trdbk.c_trd_rfrnc);
		fn_userlog(c_ServiceName,"Match Account Is :%s:",st_i_trdbk.c_cln_mtch_accnt);
		fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_i_trdbk.c_xchng_cd);
		fn_userlog(c_ServiceName,"Product Type Is :%c:",st_i_trdbk.c_prd_typ);
		fn_userlog(c_ServiceName,"Underlying Is :%s:",st_i_trdbk.c_undrlyng);
		fn_userlog(c_ServiceName,"Expiry Date Is :%s:",st_i_trdbk.c_expry_dt);
		fn_userlog(c_ServiceName,"Exercise Type Is :%c:",st_i_trdbk.c_exrc_typ);
		fn_userlog(c_ServiceName,"Option Type Is :%c:",st_i_trdbk.c_opt_typ);
		fn_userlog(c_ServiceName,"Strike Price is :%ld:",st_i_trdbk.l_strike_prc);
		fn_userlog(c_ServiceName,"IndexStock Is :%c:",st_i_trdbk.c_ctgry_indstk);
		fn_userlog(c_ServiceName,"Order Reference No. Is :%s:",st_i_trdbk.c_ordr_rfrnc);
		fn_userlog(c_ServiceName,"Exchange Ack Is :%s:",st_i_trdbk.c_xchng_ack);
		fn_userlog(c_ServiceName,"Exchange Trade No. Is :%ld:", ptr_st_trade_confirm_tr->li_fill_number);
		fn_userlog(c_ServiceName,"Trade No. Is :%ld:",st_i_trdbk.l_xchng_trd_no);
		fn_userlog(c_ServiceName,"Exchange Trade Date Is :%ld:",ptr_st_trade_confirm_tr->li_activity_time);
		fn_userlog(c_ServiceName,"Trade Date Is :%s:",st_i_trdbk.c_trd_dt);
		fn_userlog(c_ServiceName,"Trade Flow Is :%c:",st_i_trdbk.c_trd_flw);
		fn_userlog(c_ServiceName,"In TRD_CONF, the stream number receieved from exchange is :%ld:",st_i_trdbk.l_ca_lvl);
		fn_userlog(c_ServiceName,"Exchange Executed Qty Is :%ld:", ptr_st_trade_confirm_tr->li_fill_quantity);
		fn_userlog(c_ServiceName,"Exchange Executed Rate Is :%ld:",ptr_st_trade_confirm_tr->li_fill_price);
		fn_userlog(c_ServiceName,"Executed Qty Is :%ld:",st_i_trdbk.l_exctd_qty);
		fn_userlog(c_ServiceName,"Executed Rate Is :%ld:",st_i_trdbk.l_exctd_rt);
		fn_userlog(c_ServiceName,"DownLoad flag Is :%ld:",st_i_trdbk.l_dwnld_flg);
		fn_userlog(c_ServiceName,"Exchange Log Time Stamp Is :%lld:",ll_log_tm_stmp);
		fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_trdbk.d_jiffy);
	}
	
	strcpy(st_i_trdbk.c_user_id,st_i_ordbk.c_user_id);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"USER ID IS  :%s:",st_i_trdbk.c_user_id);     
		fn_userlog(c_ServiceName,"CALLING SFO_UPD_TRDBK");
		fn_userlog(c_ServiceName,"Last Activity Ref is :%s:",c_lst_act_ref);
	}

	st_i_trdbk.c_oprn_typ = INSERT_TRADE_CONF;



	i_ch_val = fn_upd_trdbk (	c_ServiceName,					
													 	c_err_msg,
														c_usr_id,
														c_lst_act_ref,
													 	&st_i_trdbk
												 );

  if ( ( i_ch_val != SUCC_BFR ) &&
       ( i_ch_val != RECORD_ALREADY_PRCSD ) )
  {
    fn_errlog(c_ServiceName,"L31550", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }

  else if ( i_ch_val == RECORD_ALREADY_PRCSD )
  {
		fn_userlog ( c_ServiceName, "Record already processed" );
		fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return 0;
	}

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName, "ORS2-RD After Call To SFO_UPD_TRDBK :%s:",c_time);
	}

	i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"L31555", LIBMSG, c_err_msg);
    return -1;
  }

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName, "ORS2-RD After Commit :%s:",c_time);
	}

  if(DEBUG_MSG_LVL_3) 
  {
	  fn_userlog(c_ServiceName," INSIDE fn_trd_confirmation -- checking For SPAN Flag");
  }

	 EXEC  SQL                                                         
     SELECT  nvl(CLM_SPAN_ALLWD,'N')
     INTO    :c_span_flag
     FROM    CLM_CLNT_MSTR
     WHERE   CLM_MTCH_ACCNT =:st_i_trdbk.c_cln_mtch_accnt;

    if(SQLCODE !=0 && SQLCODE != NO_DATA_FOUND )
    {
			fn_errlog(c_ServiceName,"L31560", SQLMSG, c_err_msg);
    	return -1;
    }

  if(DEBUG_MSG_LVL_3)  
  {
	  fn_userlog(c_ServiceName," SPAN Flag Is :%c: for Customer :%s: ",c_span_flag,st_i_trdbk.c_cln_mtch_accnt);
  }

	if (c_span_flag == 'N')  
	{
    if(DEBUG_MSG_LVL_3) 
    {	
		  fn_userlog(c_ServiceName, " Inside NORMAL Customer ");
    }

		if ( st_i_trdbk.c_prd_typ == OPTIONS || st_i_trdbk.c_prd_typ == OPTIONPLUS )	
  	{
      if(DEBUG_MSG_LVL_3) 
      {
			  fn_userlog(c_ServiceName," Calling SFO_OPT_TCONF ");
      }
  
   		strcpy ( c_svc_name , "SFO_OPT_TCONF" );
  	}
  	
  	else if ( st_i_trdbk.c_prd_typ == FUTURES || st_i_trdbk.c_prd_typ == FUTURE_PLUS || st_i_trdbk.c_prd_typ == SLTP_FUTUREPLUS )		
  	{
      if(DEBUG_MSG_LVL_3) 
      {
			  fn_userlog(c_ServiceName," Calling SFO_FUT_TCONF ");
      }

   		strcpy ( c_svc_name , "SFO_FUT_TCONF" );
  	}

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
		}

		strcpy(st_i_trdbk.c_rout_crt,c_rout_str);
	
		i_ch_val = fn_acall_svc ( c_ServiceName,
    	                        c_err_msg,
      	                      &st_i_trdbk,
        	                    "vw_tradebook",
          	                  sizeof ( st_i_trdbk ),
            	                TPNOREPLY,
              	              c_svc_name );
  	if ( i_ch_val != 0 )
  	{
    	fn_errlog(c_ServiceName,"L31565", LIBMSG, c_err_msg);
    	return -1;
  	}
	}
	else if (c_span_flag == 'Y')   
	{
		strcpy(st_i_trdbk.c_rout_crt,c_rout_str);	
    if(DEBUG_MSG_LVL_3) 
    {
		  fn_userlog(c_ServiceName, " Inside SPAN Customer ");
    }

		if(DEBUG_MSG_LVL_3)
  	{
    	fn_userlog(c_ServiceName,"  Calling SFO_PRCS_TRD ");
    	fn_userlog(c_ServiceName," st_i_trdbk.c_rout_crt Is :%s:",st_i_trdbk.c_rout_crt);
    	fn_userlog(c_ServiceName,"  st_i_trdbk.l_eba_cntrct_id Is :%ld:",st_i_trdbk.l_eba_cntrct_id);
    	fn_userlog(c_ServiceName,"Trade Reference Is :%s:",st_i_trdbk.c_trd_rfrnc);
    	fn_userlog(c_ServiceName,"Match Account Is :%s:",st_i_trdbk.c_cln_mtch_accnt);
    	fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_i_trdbk.c_xchng_cd);
    	fn_userlog(c_ServiceName,"Product Type Is :%c:",st_i_trdbk.c_prd_typ);
    	fn_userlog(c_ServiceName,"Underlying Is :%s:",st_i_trdbk.c_undrlyng);
    	fn_userlog(c_ServiceName,"Expiry Date Is :%s:",st_i_trdbk.c_expry_dt);
    	fn_userlog(c_ServiceName,"Exercise Type Is :%c:",st_i_trdbk.c_exrc_typ);
    	fn_userlog(c_ServiceName,"Option Type Is :%c:",st_i_trdbk.c_opt_typ);
    	fn_userlog(c_ServiceName,"Strike Price is :%ld:",st_i_trdbk.l_strike_prc);
    	fn_userlog(c_ServiceName,"IndexStock Is :%c:",st_i_trdbk.c_ctgry_indstk);
    	fn_userlog(c_ServiceName,"Order Reference No. Is :%s:",st_i_trdbk.c_ordr_rfrnc);
    	fn_userlog(c_ServiceName,"Exchange Ack Is :%s:",st_i_trdbk.c_xchng_ack);
    	fn_userlog(c_ServiceName,"Exchange Trade No. Is :%ld:", ptr_st_trade_confirm_tr->li_fill_number);
    	fn_userlog(c_ServiceName,"Trade No. Is :%ld:",st_i_trdbk.l_xchng_trd_no);
    	fn_userlog(c_ServiceName,"Exchange Trade Date Is :%ld:",ptr_st_trade_confirm_tr->li_activity_time);
    	fn_userlog(c_ServiceName,"Trade Date Is :%s:",st_i_trdbk.c_trd_dt);
    	fn_userlog(c_ServiceName,"Trade Flow Is :%c:",st_i_trdbk.c_trd_flw);
    	fn_userlog(c_ServiceName,"In TRD_CONF, the stream number receieved from exchange is :%ld:",
																st_i_trdbk.l_ca_lvl);
    	fn_userlog(c_ServiceName,"Exchange Executed Qty Is :%ld:", ptr_st_trade_confirm_tr->li_fill_quantity);
    	fn_userlog(c_ServiceName,"Exchange Executed Rate Is :%ld:",ptr_st_trade_confirm_tr->li_fill_price);
    	fn_userlog(c_ServiceName,"Executed Qty Is :%ld:",st_i_trdbk.l_exctd_qty);
    	fn_userlog(c_ServiceName,"Executed Rate Is :%ld:",st_i_trdbk.l_exctd_rt);
    	fn_userlog(c_ServiceName,"DownLoad flag Is :%ld:",st_i_trdbk.l_dwnld_flg);
    	fn_userlog(c_ServiceName,"Exchange Log Time Stamp Is :%lld:",ll_log_tm_stmp);
    	fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_trdbk.d_jiffy);
 	 	}

		i_ch_val       = fn_acall_svc_fml ( c_ServiceName, 
                                       c_err_msg,
                                       "SFO_PRCS_TRD",
                                       0,
                                       18,
                                       FFO_ROUT_CRT, (char *)st_i_trdbk.c_rout_crt,  
                                       FFO_ORDR_RFRNC, (char *)st_i_trdbk.c_ordr_rfrnc,
                                       FFO_TRD_RFRNC, (char *)st_i_trdbk.c_trd_rfrnc,
                                       FFO_EBA_MTCH_ACT_NO, (char *)st_i_trdbk.c_cln_mtch_accnt,
                                       FFO_EBA_CNTRCT_ID, (char *)&st_i_trdbk.l_eba_cntrct_id,/* Needtoget */
                                       FFO_XCHNG_CD, (char *)st_i_trdbk.c_xchng_cd,
                                       FFO_PRDCT_TYP, (char *)&st_i_trdbk.c_prd_typ,
                                       FFO_UNDRLYNG, (char *)st_i_trdbk.c_undrlyng,
                                       FFO_EXPRY_DT, (char *)st_i_trdbk.c_expry_dt,
                                       FFO_EXER_TYP, (char *)&st_i_trdbk.c_exrc_typ,
                                       FFO_OPT_TYP, (char *)&st_i_trdbk.c_opt_typ,
                                       FFO_STRK_PRC, (char *)&st_i_trdbk.l_strike_prc,
                                       FFO_CTGRY_INDSTK, (char *)&st_i_trdbk.c_ctgry_indstk,
                                       FFO_CA_LVL, (char *)&st_i_trdbk.l_ca_lvl,    
                                       FFO_TRD_FLW, (char *)&st_i_trdbk.c_trd_flw,
                                       FFO_EXEC_QTY, (char *)&st_i_trdbk.l_exctd_qty,
                                       FFO_EXCTD_RT, (char *)&st_i_trdbk.l_exctd_rt,
                                       FFO_TRD_DT, (char *)st_i_trdbk.c_trd_dt
                                     );
		if ( i_ch_val != 0 )
    {
      fn_errlog(c_ServiceName,"L31570", LIBMSG, c_err_msg);
      return -1;
    }


	}
	

	fn_gettime();

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName, "ORS2-RD After Call To FUT/OPT TCONF :%s:",c_time);
  	fn_userlog(c_ServiceName, "After futopt tconf");
	}

	return 0;
}

int fn_ord_can_trim ( struct st_oe_rspn_tr *ptr_oe_rspn_tr,
                 			char *c_xchng_cd,
                 			char *c_pipe_id,
                 			char *c_rout_str,
                 			char *c_trd_dt,
                 			char *c_ordr_rfrnc,
                 			int  i_dwnld_flg,
                 			char *c_ServiceName,
                 			char *c_err_msg)
{

  char c_xchng_rmrks[256];
  char c_svc_name[15+1];
  char c_mtch_accnt[10 + 1];
  int i_trnsctn;
  int i_ch_val;
	char c_lst_act_ref[22];     

  char c_stream_no='\0';

  char c_stream_no_str[2]="\0"; /** 6.8 ver **/

  long l_token_id;

  long long ll_log_tm_stmp;

  struct vw_xchngbook st_i_xchngbk;
  struct vw_orderbook st_i_ordbk;
  struct vw_contract st_cntrct;

  MEMSET(st_i_xchngbk);
  MEMSET(st_i_ordbk);
  MEMSET(st_cntrct);
  MEMSET(c_xchng_rmrks);
	MEMSET(c_lst_act_ref);		

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"Inside Function fn_ord_can");
  }

  switch (ptr_oe_rspn_tr->si_transaction_code)
	{
    case ORDER_CXL_CONFIRMATION_TR :

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Inside ORDER_CXL_CONFIRMATION_TR Case");
      }
      st_i_xchngbk.l_ors_msg_typ = ORS_IOC_CAN_ACPT;

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Message Type Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
        fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_rspn_tr->li_last_modified);
        fn_userlog(c_ServiceName,"Exchange Volume Filled Today Is :%ld:",ptr_oe_rspn_tr->li_volume_filled_today);
        fn_userlog(c_ServiceName,"Exchange Token NO. Is :%ld:",ptr_oe_rspn_tr->l_token_no);
        fn_userlog(c_ServiceName,"Error code Is :%d:",ptr_oe_rspn_tr->si_error_code);
      }

      fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_rspn_tr->li_last_modified);

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
      }

      st_i_xchngbk.l_xchng_can_qty = ptr_oe_rspn_tr->li_volume_filled_today;

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Exchange Book Cancelled Qty Is :%ld:",st_i_xchngbk.l_xchng_can_qty);
      }

      if ( ptr_oe_rspn_tr->si_error_code == 17070 )
      {
        strcpy(c_xchng_rmrks , "The Price is out of the current execution range" );
      }
      else if ( ptr_oe_rspn_tr->si_error_code == 17071 )
      {
        strcpy(c_xchng_rmrks , "The order could have resulted in self trade" );
      }
      /** added in ver 6.9 ***/
      else if( ptr_oe_rspn_tr->si_error_code == 16388 && ptr_oe_rspn_tr->st_ord_flg.flg_ioc == 1)
      {
        strcpy(c_xchng_rmrks , "IOC order cancelled by system");
      }
      /*** ver 6.9 ends ***/
      else if ( ptr_oe_rspn_tr->si_error_code == 16388 )
      {
        strcpy(c_xchng_rmrks , "Order Cancelled by system");
      }
      /** added in ver 6.9 ***/
      else if ( ptr_oe_rspn_tr->st_ord_flg.flg_ioc == 1 )
      {
        strcpy(c_xchng_rmrks , "IOC order has been cancelled by Exchange" );
      }
      /*** ver 6.9 ends ***/
      else
      {
        /** strcpy(c_xchng_rmrks , "IOC order cancelled" ); commented in ver 6.8 **/
        strcpy(c_xchng_rmrks , "Order has been cancelled by Exchange"); /** ver 6.8 **/ 
      }


      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",c_xchng_rmrks);
      }

      l_token_id  =  ptr_oe_rspn_tr->l_token_no;

      break;

    case BATCH_ORDER_CXL_OUT :
			if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Inside BATCH_ORDER_CXL_OUT Case");
      }

      if (ptr_oe_rspn_tr->st_ord_flg.flg_frozen == 1 )
      {
        if(DEBUG_MSG_LVL_0)
        {
          fn_userlog(c_ServiceName, "Freezed order got cancelled");
        }

        st_i_xchngbk.l_ors_msg_typ = ORS_ORD_FRZ_CNCL;

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_rspn_tr->li_last_modified);
          fn_userlog(c_ServiceName,"Exchange Volume Filled Today Is :%ld:", ptr_oe_rspn_tr->li_volume_filled_today);
        }

        fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_rspn_tr->li_last_modified);

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
        }

        st_i_xchngbk.l_xchng_can_qty = ptr_oe_rspn_tr->li_volume_filled_today;

        if(DEBUG_MSG_LVL_3)
        {
					fn_userlog(c_ServiceName,"Exchange Book Cancelled Qty Is :%ld:",st_i_xchngbk.l_xchng_can_qty);
        }

        strcpy(c_xchng_rmrks ,"Order Cancelled by Exchange");

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",c_xchng_rmrks);
        }
      }
      else
      {
        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName, "Exchange initiated expiry");
        }

        st_i_xchngbk.l_ors_msg_typ = ORS_ORD_EXP;

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Exchange Ack Time Is :%ld:",ptr_oe_rspn_tr->li_last_modified);
        }

        fn_long_to_timearr (st_i_xchngbk.c_ack_tm,ptr_oe_rspn_tr->li_last_modified);

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
        }

        strcpy(c_xchng_rmrks ,"Exchange Expiry");

				if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Exchange Remark Is :%s:",c_xchng_rmrks);
        }
      }

      break;

    default :
      fn_userlog( c_ServiceName, "Protocol error %d", ptr_oe_rspn_tr->si_transaction_code);
      break;
  }

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"Exchange Ack Is :%lf:",ptr_oe_rspn_tr->d_order_number );
    fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
  }

  sprintf (st_i_ordbk.c_xchng_ack, "%16.0lf", ptr_oe_rspn_tr->d_order_number );

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"Order Book Exchange Ack Is :%s:",st_i_ordbk.c_xchng_ack);
  }

  strcpy ( st_i_ordbk.c_pipe_id , c_pipe_id );

  if(DEBUG_MSG_LVL_3)
  {
		fn_userlog(c_ServiceName,"Pipe ID Is :%s:",st_i_ordbk.c_pipe_id);
  }

  strcpy(st_i_ordbk.c_rout_crt,c_rout_str);

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"Before Call To SFO_ACK_TO_ORD");
  }

  st_i_ordbk.c_oprn_typ = FOR_VIEW;

  i_ch_val = fn_call_svc ( c_ServiceName,
                           c_err_msg,
                           &st_i_ordbk,
                           &st_i_ordbk,
                           "vw_orderbook",
                           "vw_orderbook",
                           sizeof ( st_i_ordbk ),
                           sizeof ( st_i_ordbk ),
                           0,
                           "SFO_ACK_TO_ORD" );

  if ( i_ch_val != SUCC_BFR)
  {
    if ( i_ch_val == TOO_MANY_ROWS_FETCHED )
    {
      fn_userlog(c_ServiceName,"Inside TOO_MANY_ROWS_FETCHED");
      if ( strcmp ( c_xchng_cd, "NFO" ) == 0 )
      {
        i_ch_val = fn_nse_to_eba_tkn ( l_token_id,
                                       &st_cntrct,
																			 c_rout_str,
                                       c_ServiceName,
                                       c_err_msg );
        if ( i_ch_val != 0 )
        {
          return -1;
        }

        strcpy ( st_i_ordbk.c_xchng_cd, st_cntrct.c_xchng_cd );
        st_i_ordbk.c_prd_typ = st_cntrct.c_prd_typ;
        strcpy ( st_i_ordbk.c_expry_dt, st_cntrct.c_expry_dt );
        st_i_ordbk.c_exrc_typ = st_cntrct.c_exrc_typ;
        st_i_ordbk.c_opt_typ = st_cntrct.c_opt_typ;
        st_i_ordbk.l_strike_prc = st_cntrct.l_strike_prc;
        st_i_ordbk.c_ctgry_indstk = st_cntrct.c_ctgry_indstk;
        strcpy ( st_i_ordbk.c_undrlyng, st_cntrct.c_undrlyng );
        st_i_ordbk.l_ca_lvl = st_cntrct.l_ca_lvl;
        sprintf (st_i_ordbk.c_xchng_ack, "%16.0lf", ptr_oe_rspn_tr->d_order_number );
        strcpy ( st_i_ordbk.c_pipe_id , c_pipe_id );

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
        }
        strcpy(st_i_ordbk.c_rout_crt,c_rout_str);
        st_i_ordbk.c_oprn_typ = FOR_VIEW;

        i_ch_val = fn_call_svc ( c_ServiceName,
                                 c_err_msg,
                                 &st_i_ordbk,
                                 &st_i_ordbk,
																 "vw_orderbook",
                                 "vw_orderbook",
                                 sizeof ( st_i_ordbk ),
                                 sizeof ( st_i_ordbk ),
                                 0,
                                 "SFO_AC_TO_ORD" );

        if ( i_ch_val != SUCC_BFR)
        {
          fn_errlog(c_ServiceName,"L31575", LIBMSG, c_err_msg);
          return -1;
        }

        fn_gettime();

        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_AC_TO_ORD :%s:", c_time);
        }

        strcpy (c_ordr_rfrnc,st_i_ordbk.c_ordr_rfrnc );
      }
      else
      {
        fn_errlog(c_ServiceName,"S31235", "Logic error", c_err_msg);
        return -1;
      }
    }
    else
    {
      fn_errlog(c_ServiceName,"L31580", LIBMSG, c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return -1;
		}
  }

  fn_gettime();

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_ACK_TO_ORD :%s:",c_time);
    fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_i_ordbk.c_xchng_cd);
  }

  if(DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName,"fn_ord_can:Order Reference No. Is :%s:",st_i_ordbk.c_ordr_rfrnc);
  }

  strcpy(st_i_xchngbk.c_pipe_id,c_pipe_id);
  strcpy ( st_i_xchngbk.c_ordr_rfrnc , st_i_ordbk.c_ordr_rfrnc );
  strcpy ( st_i_xchngbk.c_xchng_cd , st_i_ordbk.c_xchng_cd );
  strcpy ( st_i_xchngbk.c_mod_trd_dt , ( char * )c_trd_dt);
  strcpy ( c_mtch_accnt , st_i_ordbk.c_cln_mtch_accnt );
  strcpy(st_i_xchngbk.c_xchng_rmrks,c_xchng_rmrks);
  strcpy (c_ordr_rfrnc,st_i_ordbk.c_ordr_rfrnc );
  st_i_xchngbk.l_ord_tot_qty = st_i_ordbk.l_ord_tot_qty;
  st_i_xchngbk.l_ord_lmt_rt = st_i_ordbk.l_ord_lmt_rt;
  st_i_xchngbk.l_ord_seq = 0;
  if (st_i_xchngbk.l_ors_msg_typ == ORS_ORD_EXP)
  {
       st_i_xchngbk.l_mdfctn_cntr = st_i_ordbk.l_mdfctn_cntr + 1;
       if(DEBUG_MSG_LVL_0)
       {
           fn_userlog(c_ServiceName,"TRIM EXPIRY Ordr Bk Mod cntr is :%ld:,Exch Book Mod cntr is :%ld: and Ordr Ref num is :%s: for ORS_ORD_EXP",st_i_ordbk.l_mdfctn_cntr,st_i_xchngbk.l_mdfctn_cntr,st_i_ordbk.c_ordr_rfrnc);
       }
  }
  else
  { 
       st_i_xchngbk.l_mdfctn_cntr = EXPIRY_MDFCTN_CNTR;
  }

  if (st_i_xchngbk.l_ors_msg_typ == ORS_ORD_EXP)
  {
    if(st_i_ordbk.c_ordr_stts == EXPIRED || st_i_ordbk.c_ordr_stts == PARTIALLY_EXECUTED_AND_EXPIRED )
    {
      fn_userlog(c_ServiceName," Order Reference No. :%s: Is Already Expired. ",st_i_ordbk.c_ordr_rfrnc);
      fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
      return 0;
    }
    st_i_xchngbk.c_plcd_stts = EXPIRED;
  }
  else
  {
    st_i_xchngbk.c_plcd_stts = CANCELLED;
  }

  st_i_xchngbk.c_rms_prcsd_flg = NOT_PROCESSED;

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"Exchange Book Modification Counter Is :%ld:",st_i_xchngbk.l_mdfctn_cntr);
    fn_userlog(c_ServiceName,"Exchange Book Placed Status Is :%c:",st_i_xchngbk.c_plcd_stts);
    fn_userlog(c_ServiceName,"Exchange Book RMS Processed Flag :%c:",st_i_xchngbk.c_rms_prcsd_flg);
    fn_userlog(c_ServiceName,"Exchange Book ORS Msg Is :%ld:",st_i_xchngbk.l_ors_msg_typ);
    fn_userlog(c_ServiceName,"Exchange Book Ack Time Is :%s:",st_i_xchngbk.c_ack_tm);
    fn_userlog(c_ServiceName,"Exchange Book Remark Is :%s:",st_i_xchngbk.c_xchng_rmrks);
		fn_userlog(c_ServiceName,"Exchange Book Pipe ID Is :%s:",st_i_xchngbk.c_pipe_id);
    fn_userlog(c_ServiceName,"Exchange Book Order Reference Is :%s:",st_i_xchngbk.c_ordr_rfrnc);
    fn_userlog(c_ServiceName,"Exchange Book Exchange Code Is :%s:",st_i_xchngbk.c_xchng_cd);
    fn_userlog(c_ServiceName,"Exchange Book Trade Date Is :%s:",st_i_xchngbk.c_mod_trd_dt);
    fn_userlog(c_ServiceName,"Exchange Book Total Order Qty Is :%ld:",st_i_xchngbk.l_ord_tot_qty);
    fn_userlog(c_ServiceName,"Exchange Book Limit Rate Is :%ld:",st_i_xchngbk.l_ord_lmt_rt);
  }

  memcpy ( &ll_log_tm_stmp,ptr_oe_rspn_tr->c_time_stamp_1, 8 );

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"Log Time Stamp Is :%lld:",ll_log_tm_stmp);
  }

  st_i_xchngbk.d_jiffy = (double) ll_log_tm_stmp;

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"Jiffy Is :%lf:",st_i_xchngbk.d_jiffy);
  }

  st_i_xchngbk.l_dwnld_flg = i_dwnld_flg;

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"Exchange Book DownLoad Flag Is :%ld:",st_i_xchngbk.l_dwnld_flg);
    fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
  }

  strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);

	/*** st_i_xchngbk.l_bit_flg  = ptr_oe_rspn_tr->c_time_stamp_2; ***/
  /** c_stream_no = ptr_oe_rspn_tr->c_time_stamp_2; *** Ver 7.1 **/
  /* st_i_xchngbk.l_bit_flg = atol(&c_stream_no); commented in ver 6.8 **/
   /** c_stream_no_str[0] = c_stream_no; ** ver 6.8 ** commented in ver 7.1 **/

   /** st_i_xchngbk.l_bit_flg = atol(c_stream_no_str); ** ver 6.8 ** Commented in Ver 7.1 **/
   st_i_xchngbk.l_bit_flg  = (long)ptr_oe_rspn_tr->c_time_stamp_2; /** Ver 7.1 **/

   fn_userlog(c_ServiceName,"CR_139156 BIT BIT :%ld:",st_i_xchngbk.l_bit_flg ); /** ver 6.8 **/

  sprintf(c_lst_act_ref,"%lld",ptr_oe_rspn_tr->ll_lastactivityref);  /*** Ver 6.4 ***/
  rtrim(c_lst_act_ref);   /*** Ver 6.4 ***/

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"IN ORD_CNCL, the stream number receievd from exchange is:%ld:",st_i_xchngbk.l_bit_flg);
    fn_userlog(c_ServiceName,"Before Call To SFO_UPD_XCHNGBK For INSERTION_ON_ORDER_FTE");
  }

  st_i_xchngbk.c_oprn_typ = INSERTION_ON_ORDER_FTE;

  i_trnsctn = fn_begintran(c_ServiceName, c_err_msg );

  if ( i_trnsctn ==  -1 )
  {
    fn_errlog(c_ServiceName,"L31585", LIBMSG, c_err_msg);
    return -1;
  }

/***  i_ch_val = fn_call_svc ( c_ServiceName, *** Commented in ***
                           c_err_msg,
                           &st_i_xchngbk,
                           &st_i_xchngbk,
                           "vw_xchngbook",
                           "vw_xchngbook",
                           sizeof ( st_i_xchngbk ),
                           sizeof ( st_i_xchngbk ),
                           0,
                           "SFO_UPD_XCHNGBK" );	******/

	i_ch_val = fn_upd_xchngbk_erl(&st_i_xchngbk,c_lst_act_ref,c_ServiceName,c_err_msg);  

	if ( ( i_ch_val != SUCC_BFR) &&
       ( i_ch_val != RECORD_ALREADY_PRCSD ) )
  {
    fn_errlog(c_ServiceName,"L31590", LIBMSG, c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return -1;
  }

  else if ( i_ch_val == RECORD_ALREADY_PRCSD )
  {
    fn_userlog ( c_ServiceName, "Record already processed" );
    fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
    return 0;
  }

  fn_gettime();

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"ORS2-RD After Call To SFO_UPD_XCHNGBK :%s:",c_time);
  }

  i_ch_val = fn_committran (c_ServiceName, i_trnsctn, c_err_msg);
  if ( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"L31595", LIBMSG, c_err_msg);
    return -1;
  }

  fn_gettime();

  if(DEBUG_MSG_LVL_3)
	{
    fn_userlog(c_ServiceName,"ORS2-RD After Commiting Transaction :%s:",c_time);
  }

  if ( st_i_ordbk.c_prd_typ == OPTIONS || st_i_ordbk.c_prd_typ == OPTIONPLUS )
  {
    strcpy ( c_svc_name , "SFO_OPT_ACK" );
  }
  else if ( st_i_ordbk.c_prd_typ == FUTURES || st_i_ordbk.c_prd_typ == FUTURE_PLUS || st_i_ordbk.c_prd_typ == SLTP_FUTUREPLUS)
  {
    strcpy ( c_svc_name , "SFO_FUT_ACK" );
  }

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"Rout String Is :%s:",c_rout_str);
  }

  strcpy(st_i_xchngbk.c_rout_crt,c_rout_str);

  i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_err_msg,
                            &st_i_xchngbk,
                            "vw_xchngbook",
                            sizeof ( st_i_xchngbk ),
                            TPNOREPLY,
                            c_svc_name );
  if ( i_ch_val != SUCC_BFR )
  {
    fn_errlog(c_ServiceName,"L31600", LIBMSG, c_err_msg);
    return -1;
	}

  fn_gettime();

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"ORS2-RD After Call To OPT/FUT ACK :%s:",c_time);
    fn_userlog(c_ServiceName,"After Complition Of fn_ord_can");
  }

  return 0;

}

int fn_upd_ordrbk_ref(struct vw_orderbook *ptr_st_orderbook,
											char *c_lst_act_ref,
                  		char *c_ServiceName,
                  		char *c_err_msg)
{
  int  i_recent_act_ref = 0;         /*** Ver 6.5 ***/
  long long ll_fod_last_act_no = 0;  /*** Ver 6.5 ***/
  long long ll_lst_act_ref  = 0;     /*** Ver 6.5 ***/
 
  char c_fod_last_act_no[22];        /*** Ver 6.5 ***/ 
  MEMSET(c_fod_last_act_no);         /*** Ver 6.5 ***/ 

	if(DEBUG_MSG_LVL_3)
  {
		fn_userlog(c_ServiceName,"Inside fn_upd_ordrbk_ref");
		fn_userlog(c_ServiceName,"Last Activity Ref is :%s:",c_lst_act_ref);
	}

  /** Ver 6.5 Starts **/
   EXEC SQL
    SELECT   NVL(FOD_LST_ACT_REF,'0')
    INTO     :c_fod_last_act_no
    FROM     FOD_FO_ORDR_DTLS
    WHERE    FOD_ORDR_RFRNC = :ptr_st_orderbook->c_ordr_rfrnc; 
  
   if( SQLCODE != 0 )
   {
    fn_errlog(c_ServiceName,"L31605", SQLMSG , c_err_msg);
    return -1;
   }

   rtrim(c_fod_last_act_no);
   sscanf(c_fod_last_act_no,"%lld",&ll_fod_last_act_no);
   sscanf(c_lst_act_ref,"%lld",&ll_lst_act_ref);
   
   i_recent_act_ref = ( ll_fod_last_act_no < ll_lst_act_ref ) ? 1 : 0;

   if(DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName,"Testing Response SLTP ANUJ"); /*** TESTING ***/
      fn_userlog(c_ServiceName,"Last act no flag.:%d: c_ordr_rfrnc :%s: ll_fod_last_act_no :%lld: ll_lst_act_ref :%lld:",i_recent_act_ref,ptr_st_orderbook->c_ordr_rfrnc,ll_fod_last_act_no,ll_lst_act_ref);
    }
   
  /** Ver 6.5 Ends **/ 

	switch( ptr_st_orderbook->c_oprn_typ )
  {
		case  UPDATE_XCHNG_RESP_DTLS_OT:
      EXEC SQL
        UPDATE  fod_fo_ordr_dtls
        SET     fod_lst_rqst_ack_tm = to_date (:ptr_st_orderbook->c_ack_tm,
                                               'dd-mon-yyyy hh24:mi:ss' ),
                fod_lmt_mrkt_sl_flg = 'L',
                fod_stp_lss_tgr     = 0,
                FOD_PRCIMPV_FLG = DECODE(FOD_PRCIMPV_FLG,'Y','N',FOD_PRCIMPV_FLG),
      /** FOD_LST_ACT_REF = :c_lst_act_ref commented in Ver 6.5 **/
								FOD_LST_ACT_REF = DECODE(:i_recent_act_ref,1,:c_lst_act_ref,FOD_LST_ACT_REF) /** Ver 6.5 **/ 
        WHERE   fod_ordr_rfrnc= :ptr_st_orderbook->c_ordr_rfrnc;
      break;

		case  UPDATE_ORDER_STATUS:
      EXEC SQL
        UPDATE  fod_fo_ordr_dtls
        SET     fod_ordr_stts = :ptr_st_orderbook->c_ordr_stts,
				/** FOD_LST_ACT_REF = :c_lst_act_ref commented in Ver 6.5 **/
                FOD_LST_ACT_REF = DECODE(:i_recent_act_ref,1,:c_lst_act_ref,FOD_LST_ACT_REF) /** Ver 6.5 **/
        WHERE   fod_ordr_rfrnc= :ptr_st_orderbook->c_ordr_rfrnc;
      break;

		default :

    	strcpy( c_err_msg, "Invalid Operation Type" );
    	fn_errlog ( c_ServiceName, "S31240", DEFMSG, c_err_msg );
    	return -1;
    break;	
	}
	if(SQLCODE != 0)
  {
    fn_errlog ( c_ServiceName, "S31245", SQLMSG, c_err_msg );
    return -1;
  }

  return 0;

}
/*** Version 6.1 ends ***/ 
