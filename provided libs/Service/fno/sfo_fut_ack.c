/******************************************************************************/
/*	Program	    			:	SFO_FUT_ACK                                           */
/*                                                                            */
/*  Input             : vw_xchngbook                                          */
/*                                                                            */
/*  Output            : vw_err_msg                                            */
/*                                                                            */
/*  Description       :	This service updates the order book, exchange book,   */
/*                      positions and the limits on getting Response from     */
/*                      the Exchange for Futures orders. The various response */
/*                      got from the Exchange are:                            */
/*                                                                            */
/*                        ORS_NEW_ORD_ACPT                                    */
/*                        ORS_NEW_ORD_RJCT                                    */
/*                        ORS_MOD_ORD_ACPT                                    */
/*                        ORS_MOD_ORD_RJCT                                    */
/*                        ORS_CAN_ORD_ACPT                                    */
/*                        ORS_CAN_ORD_RJCT                                    */
/*                        ORS_ORD_EXP                                         */
/*                        ORS_SPD_ORD_CNCL                                    */
/*                        ORS_2L_ORD_CNCL                                     */
/*                        ORS_3L_ORD_CNCL                                     */
/*                                                                            */
/*  Log               : 1.0   12-Nov-2001   S. Swaminathan                    */
/*  Log               : 1.1   13-Dec-2007   Vidyayini Krish                   */
/*  Log               : 1.2   10-Mar-2007   Shailesh 													*/
/*  Log               : 1.3   23-Jul-2008   Wipro|Vaishali 										*/
/*	Log								: 1.4   19-Jan-2009		Sandeep														*/
/*	Log								:	1.5		14-Sep-2009		Sandeep														*/
/*	Log								: 1.6		24-Sep-2009		Sandeep														*/
/*	Log								:	1.7		02-Sep-2010		Sandeep														*/		
/*	Log								:	1.8		27-Oct-2010		Sandeep														*/
/*	Log								:	1.9		02-02-2011		Sandeep														*/
/*  Log               : 2.0   20-Nov-2012   Mahesh                            */
/*  Log               : 2.1   29-Nov-2012   Navina                            */
/*  Log               : 2.2   10-Jun-2013   Mahesh                            */
/*  Log               : 2.3   16-Jul-2013   Navina                            */
/*	Log               : 2.4		11-Dec-2013		Bhushan														*/
/*	Log               : 2.5		20-Jan-2014		Sachin 														*/
/*	Log               : 2.6		09-Jun-2014		Navina 														*/
/*  Log               : 2.7   20-Jun-2014		Bhushan														*/ 
/*  Log               : 2.8   08-Aug-2014		Sachin B													*/ 
/*  Log               : 2.9   20-Feb-2015   Anand Dhopte                      */
/*  Log               : 3.0   07-Mar-2015   Ritesh Deolekar                   */
/*	Log								: 3.1   31-Jul-2015	  Tanmay W.													*/
/*	Log				        : 3.2   22-Sep-2015	  Aakash													  */
/*	Log				        : 3.5   23-Oct-2015	  Navina D.												  */
/*  Log								: 3.6		14-Jan-2016		Kishor B. 		  									*/
/*  Log								: 3.7		04-May-2016		Tanmay W./Kishor B. 							*/
/*  Log               : 4.0   18-Aug-2016   Sachin Birje.                     */
/*  Log               : 4.1   23-Aug-2016   Bhupendra Malik                   */
/*  Log								: 4.2   25-Aug-2016   Kishor B. 												*/	 
/*  Log               : 4.3   03-Nov_2016   Kishor B.                         */
/*  Log								: 4.4   21-Nov-2016   Navina D. 												*/	 
/*  Log  							: 4.5   22-Nov-2016   Kishor B./Tanmay W.							  */
/*  Log  							: 4.6   20-Jul-2017   Sachin Birje       							  */
/*  Log               : 4.8   04-Apr-2018   Mrinal Kishore                    */
/*	Log								: 4.9		09-Aug-2018		Bhushan Harekar										*/
/*  Log               : 5.0   05-Oct-2018   Suchita Dabir                     */
/*  Log               : 5.1   15-Jan-2018   Suchita Dabir                     */
/*  Log               : 5.2   06-Mar-2018   Suchita Dabir                     */
/*  Log               : 5.3   01-Apr-2019		Parag Kanojia                     */
/*  Log               : 5.5   08-May-2019   Suchita Dabir                     */
/*  Log               : 5.6   08-May-2019   Suchita Dabir                     */
/*  Log               : 5.7   30-Aug-2019   Suchita Dabir                     */
/*  Log               : 5.8   29-Jan-2020   Suchita Dabir                     */
/*  Log               : 5.9   04-Mar-2020   Navina D.                         */
/*  Log               : 6.0   24-Dec-2020   Mrinal Kishore                    */
/*	Log								: 6.1		02-Feb-2021		Kiran Itkar												*/
/*	Log								: 6.2		12-JUL-2021   Suchita Dabir.										*/
/*	Log								:	6.3		25-Oct-2021		Shlok Ghan												*/
/*	Log								:	6.4		25-Oct-2021		Suchita Dabir											*/	
/*  Log               : 6.5   07-Mar-2023   Vipul Sharma   										*/
/*	Log 							: 6.6   01-Apr-2023 	bhavana 											*/
/******************************************************************************/
/*  1.0  -  New Release                                                       */
/*  1.1  -  IBM Changes                                                       */
/*  1.2  -  FP Changes    	                                                  */
/*  1.3w -  CRCSN13948-Order Log strengthening                                */
/*	1.4  -  Partial Execution & Cancellation Case Handeling										*/
/*	1.5	 -	Service Call Replaced By Function Call 														*/
/*	1.6  -  NVL Added While Selecting Ack Time In Case Of Rejection Case      */
/*	1.7	 -  Cancellation Rejection Handling																		*/
/*	1.8	 -	Addhoc Margin Changes While Call To sfo_prcs_fimtm.               */
/*	1.9	 -	Elimnation of ML Frmttr															              */
/*  2.0  -  ROLLOVER changes                                                  */
/*  2.1  -  SLTP FuturePLUS Handling                                          */
/*  2.2  -  Fix																																*/
/*  2.3  -  Cancellation response processing and trigger of square off to be  */
/*          processed in separate transaction in sltp F+                      */
/*	2.4	 -	Fresh and Cover Order Rejection Handling For SLTP F+							*/
/*	2.5	 -	Handling of new System Squareoff Type IMTM_SYS_SQUAREOFF and      */
/*          LMTNEG_SYS_SQUAREOFF                                							*/
/*  2.6  -  Exception EOS for FP SLTP                                         */
/*  2.7	 -	SLTP Order modification Remarks Changes														*/
/*  2.8	 -	FO Auto MTM Changes                   														*/
/*  2.9  -  Special Flag query commented as it is already selected in         */
/*          fn_Freftoord_forupd()                                             */
/*  3.0  -  character array c_sys_msg size changed from 4 to 11(Live Bug Fix) */
/*	3.1  -  Market to Limit changes																						*/
/*	3.2  -  Market to Limit changes for deadlock issue      									*/
/*	3.5  -  Modification Acceptance bug fix for Sltp F+      									*/
/*	3.6	 -  Processing trades on functional exit  and Core product lock				*/		 
/*	3.7	 -  Changes for modification ack and self cancellation in same leg		*/ 
/*					for core and sltp products and Changes for spread order acceptance*/
/*					and Log Maintenance. 							  															*/
/*  4.0  -  Future SLTP Trail Order Changes                                   */
/*  4.1  -  My Eos changes                                                    */
/*	4.2  -  fn_ors_rms  include									              								*/
/*  4.3  -  fn_prcs_trd moved in fn_ors_rms                                   */
/*	4.4  -  Core fix           									              								*/
/*  4.5  -  Max order qty CR-ISEC04-84771 & Fill Qty Fetch									  */
/*  4.6  -  Service call with TPNOTRAN and TPNOREPLY options                  */
/*  4.8  -  ORDER REJECTION STATUS CLASH with My EOS                          */
/*	4.9	 -	Order Expiry Issue																								*/
/*  5.0  -  Double margin debit                                               */
/*  5.1  -  Using traded quantity available in cancellation response packet   */
/*          for solicited cancellation response processing( Suchita Dabir )   */
/*  5.2  -  Rollover with spread changes                                      */
/*  5.3  -  FO TAP last activity ref changes ( Parag K. )                     */
/*  5.5  -  Handling of Margin Release on Order Rejection in                  */
/*          FPSL & OP due to Quantity Freeze ( Suchita Dabir )                */
/*  5.6  -  Handling for extra open position caused                           */
/*          by multiple triggers of exception EOS( Suchita Dabir )            */
/*  5.7  -  NVL changes for last activity ref ( Suchita Dabir )               */
/*  5.8  -  Handling for extra open position unsolicited cancellation(Suchita)*/
/*  5.9  -  FPSL margin change CR-ISEC14-133200   - Navina D.                 */
/*  6.0  -  Rollover with Spread P-Executed Cancellation Bug Fix - Mrinal K.  */
/*	6.1	 -  Separate EOS for Stock and Index Underlyings in FPSL - Kiran Itkar*/
/*	6.2	 -  CR_ISEC04_132410 changes - Suchita Dabir                          */
/*	6.3	 - 	CR-ISEC14-150750_Introduction of Profit Order in FPSL	-	Shlok Ghan	*/
/*	6.4  -  Physical settlement minor changes - Suchita Dabir                 */
/*  6.5  -  Changing fn_lock_usr to fn_lock_fno (Vipul Sharma)								*/
/*  6.6  -  real time margin reporting changes  (bhavana ) 							*/
/*  TOL  -  Tux on Linux  (Agam)                                              */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atmi.h>
#include <fml32.h>
#include <sqlca.h>
#include <time.h>
#include <fo.h>
#include <eba_to_ors.h>
#include <fo_view_def.h>
#include <fo_fml_def.h>
#include <fn_tuxlib.h>
#include <fn_log.h>
#include <fn_pos.h>
#include <fn_ddr.h>
#include <fn_session.h>
#include <fml_rout.h> /* 1.1 */
#include <fn_read_debug_lvl.h> /* Ver 1.1 */
#include <fn_ors_rms.h> /* Ver 4.2 */
#include <unistd.h> /** Ver 5.2 **/
#include <sys/time.h>  /* Added in Ver TOL */

EXEC SQL INCLUDE "table/fum_fo_undrlyng_mstr.h";    /*** Ver 2.1 ***/

int fn_Freftoord_forupd(struct vw_orderbook *ptr_st_orderbook,
                       char *c_ServiceName,
                       char *c_err_msg);

int fn_Fref_to_omd ( struct vw_xchngbook *st_xchngbook,
                    struct vw_xchngbook *ptr_st_xchngbook,
                    char *c_mtm_flg,              /** ver 2.8 **/
                    char *c_prcimp_flg,            /** Ver 4.0 **/
										char *c_lst_act_ref,					 /** Ver 5.3 **/
                    char *c_ServiceName,
                    char *c_err_msg);

int fn_upd_Fxchngbkack( struct vw_xchngbook *ptr_st_xchngbook,
                        char *c_ServiceName,
                        char *c_err_msg);

int fn_upd_Fordrbkack (struct vw_orderbook *ptr_st_orderbook,
											 char c_prcimp_flg,   /** Ver 4.0 **/
											 char *c_lst_act_ref,           /** Ver 5.3 **/
                       char *c_ServiceName,
                       char *c_err_msg);

void fn_gettime(void);

/*** int fn_prcs_trd(struct vw_orderbook *ptr_st_orderbook,
                char *c_ServiceName,
                char *c_err_msg); Commented in Ver 4.3 *** Ver 3.6 ***/

char c_time[12];

void SFO_FUT_ACK( TPSVCINFO *rqst )
{

	/*** FBFR32 *ptr_fml_Sbuf;	*** Ver 3.2 ***
	ptr_fml_Sbuf  = ( FBFR32 *)NULL; *** Ver 2.5 ***  Commented in Ver 4.4 ***/

	char	c_ServiceName[33];
	char	c_err_msg[256];
	char	c_imtm_remarks[256];
	int		i_trnsctn;
	int		i_returncode;
	int		i_ip_len;
	int		i_op_len;
	int   i_commit_flg = 0;		/*** Ver 2.3 ***/
	int 	i_xchngbkerr[7];
  long  l_ftd_trd_qty = 0;  /*Ver 1.4*/ /*** Initialized in ver 5.1 ***/
  /*** commented in ver 5.1 long  l_ftd_qty;      *Ver 1.4*/
  long l_max_allowed_qty = 0 ; /*** Ver 4.5 ***/
  int   i_canrej_flag = 1; /*** ver 5.0 ***/

  long l_prvsnl_pending_qty = 0;
  char c_rqst_typ = '\0';
  char  c_span_flg = '\0';
  char c_prvsnl_opnpos_flow = '\0';
  char c_sub_mode = '\0';

	char	c_tag [ 256 ];
  char 	c_svc_name [15+1];             /*Ver 1.2*/
	char	c_prc_flg = 'N';       /***  Ver 1.8 ***/
  char  c_spl_flg = '\0';      /***  Ver 2.0 ***/
  char  c_mtm_flg = '\0';      /***  Ver 2.8 **/
  char  c_prcimp_flg = '\0';      /***  Ver 4.0 **/
   char	c_cvr_prd_typ = '\0';	/*** Ver 3.2 ***/

	/*** Ver 2.1 starts here ***/
	char c_sltp_ord_rfrnc[19];
	char c_ordr_rfrnc[19];			/*** Ver 3.2 ***/
  char c_ordr_rfrnc_core[19];       /*** Ver 3.6 ***/
	
	char c_sltp_ord_rfrnc_lck[2][19];	/*** Ver 3.2 ***/
	char c_ordr_rfrnc_lck[2][19];			/*** Ver 3.2 ***/

	char c_fc_flg = '\0';			
	char c_ordr_stts = '\0';		
	char c_narration_id[4];
	char c_svc_nm[20];
	char c_slm_flg;
	char c_remarks[50];
  char c_remarks_phy[100] = "\0"; /** ver 6.2 **/
  char c_ordr_flow_phy = '\0'; /*** ver 6.2 ***/
	char c_ord_typ;
	char c_mtm_sltp_req = '\0';
	char c_ordr_flw = '\0';
	char c_sys_msg[11] = "\0"; /* Ver 3.0 Array size increase from 4 to 11 */ 
	char c_sys_msg1[11] = "\0"; /* Ver 6.3 */ 
   char c_nkd_blkd_flg='\0'; /** ver 6.3 ***/
	char c_cover_ref[19] = "\0"; /*** Ver 2.6 ***/
  char c_ipord_ref[19] = "\0";   /*** Ver 3.7 ***/
  char c_prcimpv_flg='\0';  /** Ver 4.0 **/
  char c_rqst_flg='\0';  /** Ver 4.0 **/
  char c_new_mod_ord_flg='\0';  /** Ver 4.0 **/

	long	l_exctd_qty = 0;					
 	/*** long  l_ftd_ord_exe_qty = 0; Commented in Ver 4.5 **** Ver 3.7 ****/
	long	l_frsh_xtd_qty = 0;				
	long	li_tot_exec_qty = 0;			
	long	l_open_qty = 0;		
	long	li_dsclsd_qty = 0;
	long	li_stp_lss_tgr_prc = 0;
	long int li_lmt_rt = 0;
	long l_exec_qty = 0;
	long l_cvr_ord_lmt_rt = 0; /*** Ver 3.1 ***/
	long l_fsh_ord_lmt_rt = 0; /*** Ver 3.1 ***/
  long l_ors_msg_typ=0; /*** Ver 3.7 ***/

	double d_fmm_sltp_prcnt = 0.0;			
	double d_amt_to_be_blckd = 0.0;			
	double d_lss_amt_to_be_blckd = 0.0; 
	double d_amt_blkd = 0.0;							
	double d_lss_amt_blkd = 0.0;					
	double d_diff_amt_blkd = 0.0;				
	double d_diff_lss_amt_blkd = 0.0;		
	double d_frsh_exe_val = 0.0;	
	double d_balance_amt = 0.0;
	double d_wgt_avrg_price = 0.0;
	double d_diff_amt= 0.0;
	double d_diff_loss= 0.0;
	/*** Ver 2.1 ends here ***/

  double d_trail_amt=0.0;  /** Ver 4.0 **/
  double d_lmt_offset=0.0; /** Ver 4.0 **/

	char c_frsh_ordr_stts = '\0';         /*** Ver 2.4 ***/

	struct vw_xchngbook st_cvr_xchngbk;		/*** Ver 3.2 ***/
	int i_status = 0;							/*** Ver 3.2 ***/
	int i_ferr[7];							/*** Ver 3.2 ***/	
	int i;									/*** Ver 3.2 ***/	
	varchar c_sys_dt[LEN_DATE];     /*** Ver 2.5 ***/
	char c_lst_act_ref [22] ; 	/*** Ver 5.3 ***/
	MEMSET(c_lst_act_ref);	/*** Ver 5.3 ***/

  /** ver 5.2 started **/
  int   i_trnsctn_temp = 0;
  char  c_other_ordr_ref[19];
  char  c_another_ordr_ref[19];

  struct vw_orderbook st_next_ordbk;
  struct vw_orderbook st_frst_ordbk;

  long     l_sroll_diff_amt = 0;
  long     l_sroll_lss_amt = 0;
  char     c_another_ordr_rfrnc[2][19];
  int      i_source_dest_flg = 0;
  char     c_expry_date_current_ordr[23];

	char c_ref_rmrks[100] = "\0";				/*** Ver 5.9 ***/
	char	c_tmp_rmrks [ 100 ] = "\0";	  /*** Ver 5.9 ***/
	double d_comp1_mrgn = 0.0;  /*** Ver 5.9  ***/
  double d_comp1_pl = 0.0;    /*** Ver 5.9 ***/
  double d_xchng_amt = 0.0;   /*** Ver 5.9  ***/

	long  l_pnd_ord= -1 ; /*** Ver 6.3 ***/
	long	l_tot_pend = 0; /*** ver 6.3 ***/
  char c_can_ordr_ref[19] = "\0" ;  /*** Ver 6.3 ***/

  TPTRANID tranid;

  MEMSET( c_other_ordr_ref );
  MEMSET( c_another_ordr_ref );
  MEMSET( st_next_ordbk );
  MEMSET( st_frst_ordbk );
  MEMSET( c_expry_date_current_ordr );
  MEMSET( c_another_ordr_rfrnc );
  /** ver 5.2 ends**/


	/*** Ver 3.2 Start***/
	MEMSET(st_cvr_xchngbk);	
	MEMSET(c_sltp_ord_rfrnc_lck);
	MEMSET(c_ordr_rfrnc_lck);
	/*** Ver 3.2 End***/

	EXEC SQL BEGIN DECLARE SECTION;
		struct vw_xchngbook *ptr_st_xchngbook;
		struct vw_xchngbook st_xchngbook;
		struct vw_orderbook st_orderbook;
		struct vw_tradebook st_tradebook;
		struct vw_pstn_actn st_pstn_actn;
		short int si_null;
	EXEC SQL END DECLARE SECTION;
	
	struct vw_err_msg *ptr_st_err_msg;

	ptr_st_xchngbook = ( struct vw_xchngbook *)rqst->data;
	strcpy( c_ServiceName, rqst->name );
	INITDBGLVL(c_ServiceName);

	MEMSET(st_pstn_actn);     /*** Ver 2.2 ***/
	MEMSET(st_orderbook);		/*** Ver 3.2 ***/

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
  fn_init_ddr_val ( ptr_st_xchngbook->c_rout_crt );

	i_trnsctn = fn_begintran( c_ServiceName, c_err_msg );
	if ( i_trnsctn == -1 )
	{
		fn_errlog ( c_ServiceName, "S31010", LIBMSG, c_err_msg );
		strcpy ( ptr_st_err_msg->c_err_msg, c_err_msg );
		/*** tpfree ( ( char * ) ptr_st_xchngbook );  Commented in Ver 4.4 ***/
		tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
	}
	
  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {	
		fn_userlog(c_ServiceName, "Stage1");
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog( c_ServiceName, "ptr_st_xchngbook->c_oprn_typ:%c:",
                                             ptr_st_xchngbook->c_oprn_typ );
	}

	strcpy( st_orderbook.c_ordr_rfrnc, ptr_st_xchngbook->c_ordr_rfrnc );

	st_orderbook.c_oprn_typ = FOR_UPDATE;

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog( c_ServiceName, "ptr_st_xchngbook->c_ordr_rfrnc is :%s:",ptr_st_xchngbook->c_ordr_rfrnc);
		fn_userlog( c_ServiceName, "pst_orderbook.c_ordr_rfrnc is :%s:",st_orderbook.c_ordr_rfrnc);
	}
	
	i_ip_len = sizeof ( struct vw_orderbook );
	i_op_len = sizeof ( struct vw_orderbook );

  /*** Added for Order Routing ***/
	fn_cpy_ddr ( st_orderbook.c_rout_crt );

	/*** Commented In Ver 1.5
	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              &st_orderbook,
                              &st_orderbook,
                              "vw_orderbook",
                              "vw_orderbook",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_REF_TO_ORD" );
	*****************************************************/

	/*** ptr_fml_Sbuf = (FBFR32 *)tpalloc( "FML32", NULL, MIN_FML_BUF_LEN );		*** Ver 2.5 ***
	  if ( ptr_fml_Sbuf == NULL )	*** Ver 2.5 ***
	  {
			fn_errlog ( c_ServiceName, "S31015", LIBMSG, c_err_msg );
			tpfree((char *)ptr_fml_Sbuf);
			tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Sbuf, 0, 0 );
	  } Commented in Ver 4.4 ***/

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog(c_ServiceName, "Stage2");
	}

	/*** Ver 3.2 starts here ***/

	EXEC SQL
	SELECT
			fod_prdct_typ,				
			DECODE(FOD_EOS_FLG,'M','MY_EOS','N','EOS','E','EXCPT_EOS','S','SYS_EVENT','X'), /** Ver 4.1 **/ /*** SYS_EVENT added in Ver 6.3 ***/
			DECODE(FOD_EOS_FLG,'M',FOD_CHANNEL,'E','SYS','N','SYS','S','SYS',FOD_CHANNEL),  /** Ver 4.1 **/ /*** Ver 6.3 ***/
      NVL(FOD_SROLL_DIFF_AMT,0), /** ver 5.2 **/
      NVL(FOD_SROLL_LSS_AMT,0), /** ver 5.2 **/
      FOD_SPL_FLAG, /** ver 5.2 **/
      FOD_EXPRY_DT, /** ver 5.2 **/
      FOD_ORDR_FLW, /** ver 6.2 **/
			NVL(FOD_SLTP_ORDR_RFRNC,'*')	/*** ver 6.3 ***/
	INTO
			:c_cvr_prd_typ,	
			:c_sys_msg, /** Ver 4.1 **/
			:st_pstn_actn.c_channel,  /*** Ver 4.1 ***/
      :l_sroll_diff_amt,/** ver 5.2 **/
      :l_sroll_lss_amt,/** ver 5.2 **/
      :c_spl_flg,/** ver 5.2 **/
      :c_expry_date_current_ordr,/** ver 5.2 **/
      :c_ordr_flow_phy,
			:c_sltp_ord_rfrnc			/*** ver 6.3***/ 
	FROM  FOD_FO_ORDR_DTLS
	WHERE fod_ordr_rfrnc = :st_orderbook.c_ordr_rfrnc;
	if(SQLCODE !=0)
	{
		fn_errlog ( c_ServiceName, "S31020", SQLMSG, ptr_st_err_msg->c_err_msg );
		fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
		tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
	}

	rtrim(c_sys_msg); /*** Ver 4.1 ***/
	rtrim(st_pstn_actn.c_channel); /*** Ver 4.1 ***/
  if(DEBUG_MSG_LVL_0)   /*** Ver 3.7 ***/
  {
		fn_userlog( c_ServiceName, "In SFO_FUT_ACK :: c_cvr_prd_typ : %c for st_orderbook.c_ordr_rfrnc :%s:",c_cvr_prd_typ, st_orderbook.c_ordr_rfrnc);
		fn_userlog( c_ServiceName, "c_sys_msg:%s: st_pstn_actn.c_channel:%s:",c_sys_msg, st_pstn_actn.c_channel); /*** Ver 4.1 ***/
	}
	if (c_cvr_prd_typ == SLTP_FUTUREPLUS)
	{
		EXEC SQL
			SELECT
					fod_sltp_ordr_rfrnc				
			INTO
					:c_sltp_ord_rfrnc	
			FROM  FOD_FO_ORDR_DTLS
			WHERE fod_ordr_rfrnc = :st_orderbook.c_ordr_rfrnc;
		if(SQLCODE !=0)
		{
			fn_errlog ( c_ServiceName, "S31025", SQLMSG, ptr_st_err_msg->c_err_msg );
			fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
			tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
		}

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog( c_ServiceName, "st_orderbook.c_ordr_rfrnc :%s:,c_sltp_ord_rfrnc = :%s:",st_orderbook.c_ordr_rfrnc,c_sltp_ord_rfrnc);
	}

		EXEC  SQL
		SELECT  FOD_ORDR_RFRNC,
				FOD_SLTP_ORDR_RFRNC
		INTO    :c_ordr_rfrnc_lck,
				:c_sltp_ord_rfrnc_lck
		FROM    FOD_FO_ORDR_DTLS
		WHERE   FOD_ORDR_RFRNC in (:st_orderbook.c_ordr_rfrnc,:c_sltp_ord_rfrnc)
		FOR		UPDATE OF FOD_ORDR_RFRNC NOWAIT;

		if ( SQLCODE != 0)
		{
			if (SQLCODE == -54)
			{
				fn_userlog( c_ServiceName, "Resource busy and acquire with NOWAIT specified or timeout expired");
			}
			fn_errlog ( c_ServiceName, "S31030", SQLMSG, ptr_st_err_msg->c_err_msg );
			fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
			tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
		}
		else
		{
			fn_userlog( c_ServiceName, "Sucessfully acquired FOR UPDATE NOWAIT on :%s: and :%s:",st_orderbook.c_ordr_rfrnc,c_sltp_ord_rfrnc);
		}

		fn_userlog( c_ServiceName, "After NOWAIT on :%s: and :%s:",st_orderbook.c_ordr_rfrnc,c_sltp_ord_rfrnc);
	}
  else if( c_cvr_prd_typ == FUTURES || c_cvr_prd_typ == FUTURE_PLUS ) /** Else loop added in Ver 3.6 **/
  {
        fn_userlog( c_ServiceName, "st_orderbook.c_ordr_rfrnc :%s:",st_orderbook.c_ordr_rfrnc);
    MEMSET(c_ordr_rfrnc_core);
    /*** ver 5.2 started ***/
    if(DEBUG_MSG_LVL_0)
    {
       fn_userlog( c_ServiceName, "ROLLOVER_WITH_SPREAD :%c:",c_spl_flg);
    }
    if( c_spl_flg == ROLLOVER_WITH_SPREAD )
    {
         EXEC SQL
         SELECT FSD_ORDR_RFRNC,
		            FOD_EXPRY_DT,
		            FOD_UNDRLYNG,
		            FOD_PRDCT_TYP,
		            FOD_EXER_TYP,
		            FOD_XCHNG_CD,
		      			FOD_CLM_MTCH_ACCNT,
		      			FOD_LMT_RT,
                FOD_ORDR_FLW,
                FOD_INDSTK,
                FOD_STRK_PRC,
                FOD_OPT_TYP
         INTO   :c_other_ordr_ref,
	              :st_next_ordbk.c_expry_dt,
		      			:st_next_ordbk.c_undrlyng,
		      			:st_next_ordbk.c_prd_typ,
		      			:st_next_ordbk.c_exrc_typ,
		      			:st_next_ordbk.c_xchng_cd,
		      			:st_next_ordbk.c_cln_mtch_accnt,
		      			:st_next_ordbk.l_ord_lmt_rt,
                :st_next_ordbk.c_ordr_flw,
                :st_next_ordbk.c_ctgry_indstk,
                :st_next_ordbk.l_strike_prc,
                :st_next_ordbk.c_opt_typ
	 			FROM    FSD_FO_SPRD_DTLS,FOD_FO_ORDR_DTLS
				WHERE   FSD_SPRD_RFRNC = ( SELECT FSD_SPRD_RFRNC
													        FROM   FSD_FO_SPRD_DTLS
																	WHERE  FSD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc )
				AND     FSD_ORDR_RFRNC = FOD_ORDR_RFRNC
	      AND     FSD_ORDR_RFRNC != :st_orderbook.c_ordr_rfrnc;


        if( SQLCODE != 0 )
        {
             fn_errlog ( c_ServiceName, "S31035", SQLMSG, ptr_st_err_msg->c_err_msg );
             fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
             tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
        }
        rtrim( c_expry_date_current_ordr );
        rtrim( st_next_ordbk.c_expry_dt );

        if(DEBUG_MSG_LVL_0)
        {
        fn_userlog( c_ServiceName,"ROLLOVER_WITH_SPREAD c_other_ordr_ref :%s:",c_other_ordr_ref);
        fn_userlog( c_ServiceName,"ROLLOVER_WITH_SPREAD st_next_ordbk.c_expry_dt :%s:",st_next_ordbk.c_expry_dt);
        fn_userlog( c_ServiceName,"ROLLOVER_WITH_SPREAD c_expry_date_current_ordr :%s:",c_expry_date_current_ordr);
      	fn_userlog( c_ServiceName,"ROLLOVER_WITH_SPREAD st_next_ordbk.c_expry_dt :%s:",st_next_ordbk.c_expry_dt );
        fn_userlog( c_ServiceName,"ROLLOVER_WITH_SPREAD st_next_ordbk.c_undrlyng :%s:",st_next_ordbk.c_undrlyng);
        fn_userlog( c_ServiceName,"ROLLOVER_WITH_SPREAD st_next_ordbk.c_prd_typ :%c:",st_next_ordbk.c_prd_typ);
        fn_userlog( c_ServiceName,"ROLLOVER_WITH_SPREAD st_next_ordbk.c_exrc_typ :%c:",st_next_ordbk.c_exrc_typ );
        fn_userlog( c_ServiceName,"ROLLOVER_WITH_SPREAD st_next_ordbk.c_xchng_cd :%s:",st_next_ordbk.c_xchng_cd );
        fn_userlog( c_ServiceName,"ROLLOVER_WITH_SPREAD st_next_ordbk.c_cln_mtch_accnt :%s:",st_next_ordbk.c_cln_mtch_accnt );
        }

	   EXEC SQL
           SELECT COUNT(1)
           INTO :i_source_dest_flg
           FROM   DUAL
           WHERE  TO_DATE(:c_expry_date_current_ordr,'dd-Mon-yyyy') < TO_DATE(:st_next_ordbk.c_expry_dt,'dd-Mon-yyyy');

        if( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND)
        {
            fn_errlog ( c_ServiceName, "S31040", SQLMSG, ptr_st_err_msg->c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
            tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
        }

        if(DEBUG_MSG_LVL_0)
        {
          fn_userlog( c_ServiceName, "ROLLOVER_WITH_SPREAD i_source_dest_flg :%d:",i_source_dest_flg);
        }

        EXEC SQL
             SELECT FOD_ORDR_RFRNC,
                    FOD_ORDR_RFRNC
             INTO   :c_ordr_rfrnc_lck,
                    :c_another_ordr_rfrnc
             FROM   FOD_FO_ORDR_DTLS
             WHERE  FOD_ORDR_RFRNC IN(:st_orderbook.c_ordr_rfrnc,:c_other_ordr_ref)
             FOR UPDATE OF FOD_ORDR_RFRNC NOWAIT;
        
        if( SQLCODE != 0 )
        {
            if (SQLCODE == -54)
            {
               fn_userlog( c_ServiceName, "ROLLOVER_WITH_SPREAD Resource busy and acquire with NOWAIT specified or timeout expired");
               fn_errlog ( c_ServiceName, "S31045", SQLMSG, ptr_st_err_msg->c_err_msg );
               fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
               tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }
        }
        else
        {
               fn_userlog( c_ServiceName, "ROLLOVER_WITH_SPREAD Sucessfully acquired FOR UPDATE NOWAIT on :%s: and :%s:",st_orderbook.c_ordr_rfrnc,c_other_ordr_ref );
        }
    }
    else
    {/*** ver 5.2 ended ***/
    EXEC  SQL
    SELECT  FOD_ORDR_RFRNC
    INTO    :c_ordr_rfrnc_core
    FROM    FOD_FO_ORDR_DTLS
    WHERE   FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc
    FOR   UPDATE OF FOD_ORDR_RFRNC NOWAIT;

    if ( SQLCODE != 0)
    {
      if (SQLCODE == -54)
      {
        fn_userlog( c_ServiceName, "Resource busy and acquire with NOWAIT specified or timeout expired");
      }
      fn_errlog ( c_ServiceName, "S31050", SQLMSG, ptr_st_err_msg->c_err_msg );
      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
    }
    else
    {
      fn_userlog( c_ServiceName, "Sucessfully acquired FOR UPDATE NOWAIT on :%s:",st_orderbook.c_ordr_rfrnc);
    }

    fn_userlog( c_ServiceName, "After NOWAIT on :%s: ",st_orderbook.c_ordr_rfrnc);
    }
  }
	/*** Ver 3.2 starts here ***/

	i_returncode = fn_Freftoord_forupd(&st_orderbook,c_ServiceName,c_err_msg);

	if ( i_returncode != 0)
	{
		fn_errlog ( c_ServiceName, "S31055", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog(c_ServiceName, "Stage3");
	}
	
	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog( c_ServiceName, "c_ordr_stts  :%c:", st_orderbook.c_ordr_stts );
	}

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog(c_ServiceName, "Stage4 st_orderbook.c_ordr_stts is :%c:",st_orderbook.c_ordr_stts );
	}

	if ( st_orderbook.c_ordr_stts == REQUESTED )
	{
		ptr_st_xchngbook->c_oprn_typ = WITHOUT_ORS_MSG_TYP;
	}
	else
	{
		ptr_st_xchngbook->c_oprn_typ = WITH_ORS_MSG_TYP;
	}

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog( c_ServiceName, " ptr_st_xchngbook->c_oprn_typ is :%c:",ptr_st_xchngbook->c_oprn_typ );
	}
	i_ip_len = sizeof ( struct vw_xchngbook );
	i_op_len = sizeof ( struct vw_xchngbook );

  /*** Added for Order Routing ***/
	fn_cpy_ddr ( ptr_st_xchngbook->c_rout_crt );

	/*** Commented In Ver 1.5
	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              ptr_st_xchngbook,
                              &st_xchngbook,
                              "vw_xchngbook",
                              "vw_xchngbook",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_REF_TO_OMD" );
	*************************************************/

  i_returncode = fn_Fref_to_omd ( ptr_st_xchngbook,&st_xchngbook,&c_mtm_flg,&c_prcimp_flg,c_lst_act_ref,c_ServiceName,c_err_msg);
   /*** Ver 4.0 c_prcimp_flg added **/		/*** Added c_lst_act_ref in Ver 5.3 ***/

	if ( i_returncode != 0)
	{
		fn_errlog ( c_ServiceName, "S31060", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

   strcpy( c_remarks_phy , st_xchngbook.c_xchng_rmrks) ; /** ver 6.2 ***/
  rtrim( c_remarks_phy); /** ver 6.2 **/
  fn_userlog( c_ServiceName, " SUCHITAA c_remarks_phy :%s:",c_remarks_phy);

	/*i_returncode = fn_lock_usr( c_ServiceName,  st_orderbook.c_cln_mtch_accnt ); Commented in Ver 6.5*/
  i_returncode = fn_lock_fno( c_ServiceName,  st_orderbook.c_cln_mtch_accnt ); /* Added in Ver 6.5 */
	if ( i_returncode != 0 )
	{
		fn_errlog ( c_ServiceName, "S31065", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog( c_ServiceName, " st_xchngbook.c_rms_prcsd_flg is :%c:",st_xchngbook.c_rms_prcsd_flg );
	}

	if ( st_xchngbook.c_rms_prcsd_flg == 'P' )
	{
  	if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  	{
			fn_userlog( c_ServiceName, "Record already processed");
		}

		strcpy ( ptr_st_err_msg->c_err_msg, "Record already processed" );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
		
		/** Ver 3.6 Starts **/
    if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  	{
		  fn_userlog( c_ServiceName, "Before Call of fn_prcs_trd.........1");
		}
  	
    /*** Commented in Ver 4.3
    i_returncode = fn_prcs_trd(&st_orderbook,c_ServiceName,c_err_msg);
      ***/

    /**** Ver 4.3 Starts Here ****/
    i_returncode = fn_prcs_trd(c_ServiceName,c_err_msg,st_orderbook.c_prd_typ,st_orderbook.c_ordr_rfrnc);
    /**** Ver 4.3 Ends Here ****/

		if ( i_returncode != 0 )
 		{
   		fn_errlog ( c_ServiceName, "S31070", LIBMSG, c_err_msg );
   		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
 	  }
  	if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  	{
    	fn_userlog( c_ServiceName, "After Call of fn_prcs_trd.........1");
		}
		/** Ver 3.6 Ends **/
		tpfree ( ( char * ) ptr_st_err_msg );	/*** Ver 4.4 ***/
  	tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_xchngbook, 0, 0 );
	}

	/*** Ver 1.4 Starts ***/

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog( c_ServiceName, " ptr_st_xchngbook->l_ors_msg_typ is :%ld:",ptr_st_xchngbook->l_ors_msg_typ );
	}

 	if(ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_ACPT)
 	{
 
  	EXEC  SQL
    	SELECT  nvl(sum(FTD_EXCTD_QTY),0)
     	INTO    :l_ftd_trd_qty
     	FROM    FTD_FO_TRD_DTLS
     	WHERE   FTD_ORDR_RFRNC =:ptr_st_xchngbook->c_ordr_rfrnc
        AND   FTD_RMS_PRCSD_FLG = 'P'; /*** added in ver 5.1 ***/

   	if(SQLCODE !=0 && SQLCODE != NO_DATA_FOUND )
   	{
    	fn_errlog ( c_ServiceName, "S31075", SQLMSG, ptr_st_err_msg->c_err_msg );
	  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ); /*** Ver 4.4 ***/
     	tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
   	}

    /*** commented in ver 5.1 ***
   	EXEC  SQL
     	SELECT  nvl(sum(FTD_EXCTD_QTY),0)
     	INTO    :l_ftd_qty
     	FROM    FTD_FO_TRD_DTLS
     	WHERE   FTD_ORDR_RFRNC =:ptr_st_xchngbook->c_ordr_rfrnc
     	AND     FTD_RMS_PRCSD_FLG = 'N';

   	if(SQLCODE !=0 && SQLCODE != NO_DATA_FOUND )
   	{
     	fn_errlog ( c_ServiceName, "S31080", SQLMSG, ptr_st_err_msg->c_err_msg );
	  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ); *** Ver 4.4 ***
     	tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
   	} *********/

  	if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  	{
   		fn_userlog(c_ServiceName,"Total Traded Quantity Is :%ld:",l_ftd_trd_qty);
      /** fn_userlog(c_ServiceName,"Unprocessed Quantity Is :%ld:",l_ftd_qty); commented in ver 5.1 ***/
 		}
	}

 /*** Ver 1.4 Ends ***/

  strcpy(st_pstn_actn.c_user_id,"system");
  st_pstn_actn.l_session_id  = 0;
	strcpy( st_pstn_actn.c_cln_mtch_accnt , st_orderbook.c_cln_mtch_accnt );
	st_pstn_actn.l_eba_cntrct_id = st_orderbook.l_eba_cntrct_id;
	strcpy( st_pstn_actn.c_xchng_cd , st_orderbook.c_xchng_cd );
	st_pstn_actn.c_prd_typ = st_orderbook.c_prd_typ;
	strcpy( st_pstn_actn.c_undrlyng , st_orderbook.c_undrlyng );
	strcpy( st_pstn_actn.c_expry_dt , st_orderbook.c_expry_dt );
	st_pstn_actn.c_exrc_typ  = st_orderbook.c_exrc_typ;
	st_pstn_actn.c_opt_typ = st_orderbook.c_opt_typ;
	st_pstn_actn.l_strike_prc = st_orderbook.l_strike_prc;
	st_pstn_actn.c_ctgry_indstk = st_orderbook.c_ctgry_indstk;
	st_pstn_actn.l_ca_lvl  = st_orderbook.l_ca_lvl;
	st_pstn_actn.c_cntrct_tag  = '*';
	st_pstn_actn.l_actn_typ = ptr_st_xchngbook->l_ors_msg_typ;
	st_pstn_actn.c_trnsctn_flw = st_orderbook.c_ordr_flw;
	st_pstn_actn.l_fresh_qty  = 0;
	st_pstn_actn.l_cover_qty  = 0;
	st_pstn_actn.d_add_mrgn_amt  = 0;
  st_pstn_actn.l_curr_rt = 0;

  sprintf(st_pstn_actn.c_ref_rmrks,"%s-%ld",ptr_st_xchngbook->c_ordr_rfrnc,
                               ptr_st_xchngbook->l_mdfctn_cntr);
  rtrim(st_pstn_actn.c_ref_rmrks);

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog(c_ServiceName,"Product type = :%c:",st_orderbook.c_prd_typ);
	}

 if ( st_orderbook.c_prd_typ != SLTP_FUTUREPLUS )	/*** if condition added in Ver 2.1 ***/
 {
	switch ( ptr_st_xchngbook->l_ors_msg_typ )
	{
		case	ORS_NEW_ORD_ACPT:
		case	ORS_NEW_ORD_RJCT:
					st_pstn_actn.l_orgnl_qty = 0;
					st_pstn_actn.l_orgnl_rt  = 0;
					st_pstn_actn.l_new_qty = st_xchngbook.l_ord_tot_qty;
					st_pstn_actn.l_new_rt  = st_xchngbook.l_ord_lmt_rt;
					st_pstn_actn.l_exec_qty  = 0;
					st_pstn_actn.l_exec_rt  = 0;
					if ( ( st_orderbook.c_spl_flg == SYSTEM_SQUAREOFF ) || 
							 ( st_orderbook.c_spl_flg == JOINT_SQUAREOFF  ) ||
               ( st_orderbook.c_spl_flg == IMTM_SYS_SQUAREOFF ) ||  /** Ver 2.5 **/ 
               ( st_orderbook.c_spl_flg == LMTNEG_SYS_SQUAREOFF) )  /** Ver 2.5 **/
					{
						st_pstn_actn.c_mtm_flag = ORD_ACCPT_FOR_SYSTEM_SQ_OFF;
					}
					break;

		case	ORS_MOD_ORD_ACPT:
		case	ORS_MOD_ORD_RJCT:
					st_pstn_actn.l_orgnl_qty = st_orderbook.l_ord_tot_qty;
					st_pstn_actn.l_orgnl_rt  = st_orderbook.l_ord_lmt_rt;
					st_pstn_actn.l_new_qty = st_xchngbook.l_ord_tot_qty;
					st_pstn_actn.l_new_rt  = st_xchngbook.l_ord_lmt_rt;
					st_pstn_actn.l_exec_qty  = st_orderbook.l_exctd_qty;
					st_pstn_actn.l_exec_rt  = 0;
					break;

		case	ORS_CAN_ORD_ACPT:
		case	ORS_CAN_ORD_RJCT:
					st_pstn_actn.l_orgnl_qty = st_orderbook.l_ord_tot_qty;
					st_pstn_actn.l_orgnl_rt  = st_orderbook.l_ord_lmt_rt;

					/***	Commented In Ver 1.4
					st_pstn_actn.l_new_qty = 0;
					****/
					/***  Commented In Ver 1.4
					st_pstn_actn.l_exec_qty  = st_orderbook.l_exctd_qty;
					****/

					if( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_ACPT )			/***	Ver	1.7	***/
					{

            /** st_pstn_actn.l_exec_qty  = l_ftd_trd_qty;     * Ver 1.4 */ /** commented in ver 5.1 **/
            /** st_pstn_actn.l_new_qty   =  l_ftd_qty;        * Ver 1.4 *//*** commented in ver 5.1 **/

            /*** ver 5.1 starts ***/
            st_pstn_actn.l_exec_qty  = ptr_st_xchngbook->l_xchng_can_qty; /*** executed qty from exchange **/

            st_pstn_actn.l_new_qty  = ptr_st_xchngbook->l_xchng_can_qty - l_ftd_trd_qty; /*** Total Unprocessed trade quantity **/

            if(DEBUG_MSG_LVL_0)
            {
              fn_userlog(c_ServiceName,"In cancellation acceptance st_pstn_actn.l_exec_qty is :%ld: and total unprocessed trade quantity is st_pstn_actn.l_new_qty :%ld:",st_pstn_actn.l_exec_qty,st_pstn_actn.l_new_qty);
            }
            /*** ver 5.1 ends ***/
					}
					else if ( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_RJCT )	/***  Ver 1.7 ***/
					{
						st_pstn_actn.l_new_qty = 0;	
						st_pstn_actn.l_exec_qty  = st_orderbook.l_exctd_qty;
					}
          st_pstn_actn.l_new_rt  = 0;
					st_pstn_actn.l_exec_rt  = 0;
					break;

		case	ORS_SPD_ORD_CNCL:
		case	ORS_2L_ORD_CNCL:
		case	ORS_3L_ORD_CNCL:
		case	ORS_IOC_CAN_ACPT:
		case  ORS_ORD_FRZ_CNCL:
					st_pstn_actn.l_actn_typ = ORS_ORD_EXP;
					st_pstn_actn.l_orgnl_qty = st_orderbook.l_ord_tot_qty;
					st_pstn_actn.l_orgnl_rt  = st_orderbook.l_ord_lmt_rt;
					st_pstn_actn.l_new_qty = 0;
					st_pstn_actn.l_new_rt  = 0;
					st_pstn_actn.l_exec_qty  = ptr_st_xchngbook->l_xchng_can_qty;
					st_pstn_actn.l_exec_rt  = 0;
					break;

		case	ORS_ORD_EXP:
					st_pstn_actn.l_orgnl_qty = st_orderbook.l_ord_tot_qty;
					st_pstn_actn.l_orgnl_rt  = st_orderbook.l_ord_lmt_rt;
					st_pstn_actn.l_new_qty = 0;
					st_pstn_actn.l_new_rt  = 0;
					st_pstn_actn.l_exec_qty  = st_orderbook.l_exctd_qty;
					st_pstn_actn.l_exec_rt  = 0;
					break;

		default:
  				strcpy( ptr_st_err_msg->c_err_msg, "Invalid Operation Type" );
  				fn_userlog ( c_ServiceName, "ERROR :%s:", ptr_st_err_msg->c_err_msg );
  				fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  				tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
					break;
	} 

	i_ip_len = sizeof ( struct vw_pstn_actn );
	i_op_len = sizeof ( struct vw_pstn_actn );

  /*** Added for Order Routing ***/
  
	fn_cpy_ddr ( st_pstn_actn.c_rout_crt );

	/*Ver 1.2 starts */

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog ( c_ServiceName, "Prdct Typ recd:%c:", st_pstn_actn.c_prd_typ );
	}

	/*** Ver 2.0 starts ***/

	/***	Commented In Ver 2.9

  EXEC SQL
    SELECT FOD_SPL_FLAG
    INTO  :c_spl_flg
    FROM FOD_FO_ORDR_DTLS
    WHERE FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31085", SQLMSG, ptr_st_err_msg->c_err_msg );
    fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); 
  }

	st_pstn_actn.c_nkd_blkd_flg = 'N';      ***  Ver 2.2 ***

  if (c_spl_flg ==  'R')
  {
    st_pstn_actn.c_nkd_blkd_flg = 'R';
  }

	*******************************/
 /** ver 5.2 starts **/
  if(st_orderbook.c_spl_flg == ROLLOVER_WITH_SPREAD )
  {
     st_pstn_actn.c_nkd_blkd_flg = ROLLOVER_WITH_SPREAD;
  }
  else /** ver 5.2 ends ***/
  {
	  st_pstn_actn.c_nkd_blkd_flg = st_orderbook.c_spl_flg;			/***	Added In Ver 2.9	***/
  }

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog ( c_ServiceName, "Special flag is :%c:", st_pstn_actn.c_nkd_blkd_flg );
  }

  /*** Ver 2.0 ends ***/

	if( st_pstn_actn.c_prd_typ == FUTURES )
  	strcpy(c_svc_name,"SFO_UPD_FUT_POS");
	else
	{
  	if ( st_pstn_actn.c_prd_typ == FUTURE_PLUS )
  		strcpy(c_svc_name,"SFO_UPD_FUT_PLS");
  	else
  	{
      strcpy( ptr_st_err_msg->c_err_msg, "Invalid Prdct Type" );
      fn_userlog ( c_ServiceName, "ERROR :%s:", ptr_st_err_msg->c_err_msg );
      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
      tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
  	}
	}
  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog ( c_ServiceName, "calling service :%s:", c_svc_name );
	}
/*Ver 1.2 ends */

	if( st_orderbook.c_spl_flg != ROLLOVER_WITH_SPREAD ) /** if condition added in ver 5.2 **/
	{
		i_returncode = fn_call_svc( c_ServiceName,
			              c_err_msg,
				      &st_pstn_actn,
				      &st_pstn_actn,
				      "vw_pstn_actn",
				      "vw_pstn_actn",
					i_ip_len,
					i_op_len,
					0,
					c_svc_name );	/*Ver 1.2 */
		if ( i_returncode != SUCC_BFR )
		{
		 fn_errlog ( c_ServiceName, "S31090", LIBMSG, c_err_msg );
  		 fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  		 tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
		}
	}
	if ( st_pstn_actn.c_mtm_flag == FOR_REMARKS_ENTRY )
	{
		switch ( ptr_st_xchngbook->l_ors_msg_typ )
		{
			case	ORS_NEW_ORD_ACPT:
					sprintf(c_imtm_remarks,"RECEIVED SQUAREOFF CONFIRMATION FOR [%s]",
																						ptr_st_xchngbook->c_ordr_rfrnc );
  				i_returncode = fn_ins_rep_tbl ( c_ServiceName,
                             							c_err_msg,
                             							&st_pstn_actn,
                             							c_imtm_remarks );
					if ( i_returncode != 0 )
					{
						fn_errlog ( c_ServiceName, "S31095", LIBMSG, c_err_msg );
  					fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  					tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
					}
					break;

			case	ORS_NEW_ORD_RJCT:
					sprintf(c_imtm_remarks,"RECEIVED SQUAREOFF REJECTION FOR [%s]",
																						ptr_st_xchngbook->c_ordr_rfrnc );
  				i_returncode = fn_ins_rep_tbl ( c_ServiceName,
                             							c_err_msg,
                             							&st_pstn_actn,
                             							c_imtm_remarks );
					if ( i_returncode != 0 )
					{
						fn_errlog ( c_ServiceName, "S31100", LIBMSG, c_err_msg );
  					fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  					tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
					}
					break;

			case	ORS_CAN_ORD_ACPT:
					sprintf(c_imtm_remarks,"RECEIVED CANCELLATION ACCEPTANCE FOR [%s]",
																						ptr_st_xchngbook->c_ordr_rfrnc );
  				i_returncode = fn_ins_rep_tbl ( c_ServiceName,
                             							c_err_msg,
                             							&st_pstn_actn,
                             							c_imtm_remarks );
					if ( i_returncode != 0 )
					{
						fn_errlog ( c_ServiceName, "S31105", LIBMSG, c_err_msg );
  					fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  					tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
					}
					break;

			case	ORS_SPD_ORD_CNCL:
			case	ORS_2L_ORD_CNCL:
			case	ORS_3L_ORD_CNCL:
			case	ORS_IOC_CAN_ACPT:
					sprintf(c_imtm_remarks,"RECEIVED AUTO CANCELLATION ACCEPTANCE FOR [%s]", ptr_st_xchngbook->c_ordr_rfrnc );
  				i_returncode = fn_ins_rep_tbl ( c_ServiceName,
                             							c_err_msg,
                             							&st_pstn_actn,
                             							c_imtm_remarks );
					if ( i_returncode != 0 )
					{
						fn_errlog ( c_ServiceName, "S31110", LIBMSG, c_err_msg );
  					fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  					tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
					}
					break;

			case	ORS_CAN_ORD_RJCT:
					sprintf(c_imtm_remarks,"RECEIVED CANCELLATION REJECTION FOR [%s]",
																						ptr_st_xchngbook->c_ordr_rfrnc );
  				i_returncode = fn_ins_rep_tbl ( c_ServiceName,
                             							c_err_msg,
                             							&st_pstn_actn,
                             							c_imtm_remarks );
					if ( i_returncode != 0 )
					{
						fn_errlog ( c_ServiceName, "S31115", LIBMSG, c_err_msg );
  					fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  					tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
					}
					break;
		}

	}
  
 } /*** Ver 2.1 ***/

	switch ( ptr_st_xchngbook->l_ors_msg_typ )
	{
		case	ORS_NEW_ORD_ACPT:
					if ( ( st_orderbook.c_ordr_stts == QUEUED ) ||
               ( st_orderbook.c_ordr_stts == FREEZED )  )
					{
						st_orderbook.c_ordr_stts = ORDERED;
					}
					strcpy( st_orderbook.c_ack_tm, ptr_st_xchngbook->c_ack_tm );	
					/** This value is to update the fod_ord_ack_tm     **/
					/** c_prev_ack_tm is a little misleading name ..   **/
					strcpy( st_orderbook.c_prev_ack_tm, 
                  ptr_st_xchngbook->c_entry_dt_tm );	
					st_orderbook.c_oprn_typ = UPDATE_STATUS_ACK_TM;
					break;

		case	ORS_NEW_ORD_RJCT:
					st_orderbook.c_ordr_stts = REJECTED;
					strcpy( st_orderbook.c_ack_tm, ptr_st_xchngbook->c_ack_tm );	
					/** This value is to update the fod_ord_ack_tm     **/
					/** c_prev_ack_tm is a little misleading name ..   **/
					strcpy( st_orderbook.c_prev_ack_tm, 
                  ptr_st_xchngbook->c_entry_dt_tm );	
					st_orderbook.c_oprn_typ = UPDATE_STATUS_ACK_TM;
					break;

		case	ORS_MOD_ORD_ACPT:
					if ( st_orderbook.l_exctd_qty == 0 )
					{
						if ( ( st_orderbook.c_ordr_stts == QUEUED ) ||
                 ( st_orderbook.c_ordr_stts == FREEZED )  )
						{
							st_orderbook.c_ordr_stts = ORDERED;
						}
					}
					else if ( st_orderbook.l_exctd_qty < st_orderbook.l_ord_tot_qty )
					{
						if ( ( st_orderbook.c_ordr_stts == QUEUED ) ||
                 ( st_orderbook.c_ordr_stts == FREEZED )  )
						{
							st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED;
						}
					}
					else
					{
						st_orderbook.c_ordr_stts = EXECUTED;
					}

          if(st_orderbook.c_slm_flg == 'S')  /*** If Condition Added in Ver 2.7 ***/
          {
            strcpy(ptr_st_xchngbook->c_xchng_rmrks,"SLTP Order Modification Accepted");
          }
          else
          {
          strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Modification Accepted");
          }

					st_orderbook.c_slm_flg = st_xchngbook. c_slm_flg;
					st_orderbook.l_dsclsd_qty = st_xchngbook.l_dsclsd_qty;
					st_orderbook.l_ord_tot_qty = st_xchngbook.l_ord_tot_qty;
					st_orderbook.l_ord_lmt_rt = st_xchngbook.l_ord_lmt_rt;
					st_orderbook.l_stp_lss_tgr = st_xchngbook.l_stp_lss_tgr;
					st_orderbook.c_ord_typ = st_xchngbook.c_ord_typ;
					strcpy( st_orderbook.c_valid_dt , st_xchngbook.c_valid_dt);
					strcpy( st_orderbook.c_trd_dt , st_xchngbook.c_mod_trd_dt);
					strcpy( st_orderbook.c_ack_tm , ptr_st_xchngbook->c_ack_tm);
					st_orderbook.c_oprn_typ = UPDATE_ORDER_MODIFICATION;
				
					/*** Commented in Ver 2.7 ** 
					*Ver 1.3w Added For Order Log CR *
					strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Modification Accepted");
					***/
					break;

		case	ORS_MOD_ORD_RJCT:

          /* Ver 4.9 Starts */
          if(DEBUG_MSG_LVL_0)
          {
              fn_userlog ( c_ServiceName, "st_orderbook.l_can_qty:%ld:", st_orderbook.l_can_qty);
              fn_userlog ( c_ServiceName, "st_orderbook.l_exprd_qty :%ld:",st_orderbook.l_exprd_qty);
          }
          /* Ver 4.9 Ends */

					if ( st_orderbook.l_exctd_qty == 0 )
					{
						/** if ( (st_orderbook.l_ord_tot_qty - st_orderbook.l_can_qty) > 0 ) ** Commented in Ver 4.9 **/
						if ( (st_orderbook.l_ord_tot_qty - ( st_orderbook.l_can_qty + st_orderbook.l_exprd_qty)) > 0 ) /*** added in Ver 4.9 ***/
            {
              st_orderbook.c_ordr_stts = ORDERED;
            }
						/*** Ver 4.9 Starts ***/
						else if( st_orderbook.l_exprd_qty > 0 )
						{
							st_orderbook.c_ordr_stts = EXPIRED;
						}
						/*** Ver 4.9 Ends ***/
            else
            {
              st_orderbook.c_ordr_stts = CANCELLED;
            }
					}
					else if ( st_orderbook.l_exctd_qty < st_orderbook.l_ord_tot_qty )
					{
            if ( (st_orderbook.l_exctd_qty + st_orderbook.l_can_qty) ==
                            st_orderbook.l_ord_tot_qty )
            {
              st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED_AND_CANCELLED;
            }
						/*** Ver 4.9 Starts ***/
						else if ( (st_orderbook.l_exctd_qty + st_orderbook.l_exprd_qty) == st_orderbook.l_ord_tot_qty )
						{
							st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED_AND_EXPIRED;
						}
						/*** Ver 4.9 Ends ***/
            else
            {
              st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED;
            }
					}
					else
					{
						st_orderbook.c_ordr_stts = EXECUTED;
					}
					strcpy( st_orderbook.c_ack_tm , ptr_st_xchngbook->c_ack_tm);
					strcpy( st_orderbook.c_trd_dt , st_xchngbook.c_mod_trd_dt);
					/*********
					st_orderbook.l_can_qty = 0;
					*******/
					/** st_orderbook.l_exprd_qty = 0; ** Commented in Ver 4.9 **/
					st_orderbook.c_oprn_typ = UPDATE_XCHNG_RESP_DTLS;

					/*Ver 1.3w Added For Order Log CR */
          strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Modification Rejected");
					break;

		case	ORS_SPD_ORD_CNCL:
		case	ORS_2L_ORD_CNCL:
		case	ORS_3L_ORD_CNCL:
		case	ORS_IOC_CAN_ACPT:
		case  ORS_ORD_FRZ_CNCL:

          st_orderbook.l_can_qty=st_orderbook.l_ord_tot_qty - ptr_st_xchngbook->l_xchng_can_qty; /* ver 5.8 */
          fn_userlog ( c_ServiceName,"EOS_Qty :%ld: :%ld:",st_orderbook.l_can_qty,ptr_st_xchngbook->l_xchng_can_qty); /* ver 5.8 */
 
          if((st_orderbook.l_exctd_qty + st_orderbook.l_can_qty) == st_orderbook.l_ord_tot_qty ) /*** ver 5.8 If condition added ***/
          {
				 	  /** l_xchng_can_qty holds the executed qty ***/
					  if ( ptr_st_xchngbook->l_xchng_can_qty == 0 )
					  {
						  st_orderbook.c_ordr_stts = CANCELLED;
					  }
					  else 
					  {
						  st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED_AND_CANCELLED;
					  }
          }
          else /*** ver 5.8 starts ***/
          {
            /*** No trade is processed yet or is in-flight and cancellation arrived ***/
            if( st_orderbook.l_exctd_qty == 0 )
            {
              st_orderbook.c_ordr_stts = ORDERED;
            }
            else /** Some trades are processed and some are not yet processed or are in-flight and cancellation arrived ***/
            {
              st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED;
            }
            if(DEBUG_MSG_LVL_0)
            {
              fn_userlog( c_ServiceName,"In IOC Cancellation Acceptance FINAL ORDER STATUS IS :%c:",st_orderbook.c_ordr_stts );
            }
          } /*** ver 5.8 ends ***/
					strcpy( st_orderbook.c_trd_dt , st_xchngbook.c_mod_trd_dt);

					/** st_orderbook.l_can_qty = st_orderbook.l_ord_tot_qty - 
                                   ptr_st_xchngbook->l_xchng_can_qty; commented in ver 5.8 ***/

					strcpy( st_orderbook.c_ack_tm , ptr_st_xchngbook->c_ack_tm);
					st_orderbook.l_exprd_qty = 0;
					st_orderbook.c_oprn_typ = UPDATE_XCHNG_RESP_DTLS_CA;
					break;

		case	ORS_CAN_ORD_ACPT:

          /** ver 5.1 starts ***/
          /*** Even if trade is not processed or is in-flight , we'll get correct cancelled quantity ***/
          st_orderbook.l_can_qty = st_orderbook.l_ord_tot_qty - ptr_st_xchngbook->l_xchng_can_qty;

          if(DEBUG_MSG_LVL_0)
          {
              fn_userlog ( c_ServiceName,"In Cancellation Acceptance l_xchng_can_qty is :%ld: and l_can_qty is :%ld: st_orderbook.l_exctd_qty is :%ld: and st_orderbook.c_ordr_stts is :%c:",ptr_st_xchngbook->l_xchng_can_qty,st_orderbook.l_can_qty,st_orderbook.l_exctd_qty,st_orderbook.c_ordr_stts );
          }
          /*** ver 5.1 ends ***/

          /*** ver 5.6 - If-else condition added ***/
          if( (st_orderbook.l_exctd_qty + st_orderbook.l_can_qty) == st_orderbook.l_ord_tot_qty ) /*** ver 5.6 If condition added ***/
          {
					 if ( st_orderbook.l_exctd_qty == 0 )
					 {
						 st_orderbook.c_ordr_stts = CANCELLED;
					 }
					 else 
					 {
						 st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED_AND_CANCELLED;
					 }
          }
          else /*** ver 5.6 starts ***/
          {
            /*** No trade is processed yet or is in-flight and cancellation arrived ***/
            if( st_orderbook.l_exctd_qty == 0 )
            {
              st_orderbook.c_ordr_stts = ORDERED;
            }
            else /** Some trades are processed and some are not yet processed or are in-flight and cancellation arrived ***/
            {
              st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED;
            }
            if(DEBUG_MSG_LVL_0)
            {
              fn_userlog( c_ServiceName,"In Cancellation Acceptance FINAL ORDER STATUS IS :%c:",st_orderbook.c_ordr_stts );
            }
          } /*** ver 5.6 ends ***/
					strcpy( st_orderbook.c_trd_dt , st_xchngbook.c_mod_trd_dt);

					/*** Commented In Ver 1.4
					st_orderbook.l_can_qty = st_orderbook.l_ord_tot_qty - 
                                   st_orderbook.l_exctd_qty;
					***/

	          /*** Commented In Ver 5.1
          st_orderbook.l_can_qty = st_orderbook.l_ord_tot_qty - l_ftd_trd_qty;  * Ver 1.4 */			
	
					strcpy( st_orderbook.c_ack_tm , ptr_st_xchngbook->c_ack_tm);
					st_orderbook.l_exprd_qty = 0;
					st_orderbook.c_oprn_typ = UPDATE_XCHNG_RESP_DTLS_CA;

  				/*Ver 1.3w Added For Order Log CR */
          strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Cancellation Accepted");
					break;

		case	ORS_CAN_ORD_RJCT:

					/* Ver 4.8 Starts */
          if(DEBUG_MSG_LVL_0)
          {
              fn_userlog ( c_ServiceName, "st_orderbook.l_can_qty:%ld:", st_orderbook.l_can_qty);
						fn_userlog ( c_ServiceName, "st_orderbook.l_exprd_qty:%ld:",st_orderbook.l_exprd_qty);/* Ver 4.9 */
          }
					/* Ver 4.8 Ends */
					if ( st_orderbook.l_exctd_qty == 0 )
					{
			/*** if ( (st_orderbook.l_ord_tot_qty - st_orderbook.l_can_qty) > 0 )	* Ver 4.8 ** Commented in Ver 4.9 ***/
						if ( (st_orderbook.l_ord_tot_qty - ( st_orderbook.l_can_qty + st_orderbook.l_exprd_qty )) > 0 )  /** added in Ver 4.9 **/
						{ 
							st_orderbook.c_ordr_stts = ORDERED;
						}
            /* Ver 4.9 Starts */
            else if( st_orderbook.l_exprd_qty > 0)
            {
              st_orderbook.c_ordr_stts = EXPIRED;
            }
            /* Ver 4.9 Ends */
						/* Ver 4.8 Starts */
						else
            {
              st_orderbook.c_ordr_stts = CANCELLED;
            }
						/* Ver 4.8 Ends */	
					}
					else if ( st_orderbook.l_exctd_qty < st_orderbook.l_ord_tot_qty )
					{
						/* Ver 4.8 Starts */
						if ( (st_orderbook.l_exctd_qty + st_orderbook.l_can_qty) == st_orderbook.l_ord_tot_qty )
						{
            	st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED_AND_CANCELLED;
          	}
						/* Ver 4.9 Starts */
						else if( (st_orderbook.l_exctd_qty + st_orderbook.l_exprd_qty) == st_orderbook.l_ord_tot_qty )
						{
							st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED_AND_EXPIRED;
						}
						/* Ver 4.9 Ends */
						else
						{
						/* Ver 4.8 Ends */
							st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED;
						}			/* Ver 4.8 */
					}
					else
					{
						st_orderbook.c_ordr_stts = EXECUTED;
					}

					strcpy( st_orderbook.c_trd_dt , st_xchngbook.c_mod_trd_dt);
					strcpy( st_orderbook.c_ack_tm , ptr_st_xchngbook->c_ack_tm);
					/* st_orderbook.l_can_qty = 0; * Commented in Ver 4.8 */
					/* st_orderbook.l_exprd_qty = 0; Commented in Ver 4.9 */
					st_orderbook.c_oprn_typ = UPDATE_XCHNG_RESP_DTLS;

          /*Ver 1.3w Added For Order Log CR */
          strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Cancellation Rejected");
					break;

		case	ORS_ORD_EXP:

					if ( st_orderbook.l_exctd_qty == 0 )
					{
						st_orderbook.c_ordr_stts = EXPIRED;
					}
					else
					{
						st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED_AND_EXPIRED;
					}

					strcpy( st_orderbook.c_trd_dt , st_xchngbook.c_mod_trd_dt);
					strcpy( st_orderbook.c_ack_tm , ptr_st_xchngbook->c_ack_tm);
					st_orderbook.l_can_qty = 0;
					st_orderbook.l_exprd_qty = st_orderbook.l_ord_tot_qty - 
                                   st_orderbook.l_exctd_qty;
					st_orderbook.c_oprn_typ = UPDATE_XCHNG_RESP_DTLS;
					break;

		default:
  				strcpy( ptr_st_err_msg->c_err_msg, "Invalid Operation Type" );
  				fn_userlog ( c_ServiceName, "ERROR :%s:", ptr_st_err_msg->c_err_msg );
  				fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  				tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
					break;
	}

	i_ip_len = sizeof ( struct vw_orderbook );
	i_op_len = sizeof ( struct vw_orderbook );
				
  /*** Added for Order Routing ***/
	fn_cpy_ddr ( st_orderbook.c_rout_crt );

	/***	Commented In Ver 1.5
	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              &st_orderbook,
                              &st_orderbook,
                              "vw_orderbook",
                              "vw_orderbook",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_UPD_ORDRBK" );
	*************************************************/

 i_returncode = fn_upd_Fordrbkack(&st_orderbook,c_prcimp_flg,c_lst_act_ref,c_ServiceName,c_err_msg);  /*** Ver 4.0 , c_prcimp_flg added **/	/*** Added c_lst_act_ref in Ver 5.3 ***/

	if ( i_returncode != 0 )
	{
		fn_errlog ( c_ServiceName, "S31120", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

	ptr_st_xchngbook->c_rms_prcsd_flg = 'P';
	ptr_st_xchngbook->c_oprn_typ = UPDATE_RMS_PRCSD_FLG;

	i_ip_len = sizeof ( struct vw_xchngbook );
	i_op_len = sizeof ( struct vw_xchngbook );
				
  /*** Added for Order Routing ***/
	fn_cpy_ddr ( ptr_st_xchngbook->c_rout_crt );

	/***  Commented In Ver 1.5
	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              ptr_st_xchngbook,
                              ptr_st_xchngbook,
                              "vw_xchngbook",
                              "vw_xchngbook",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_UPD_XCHNGBK" );
	*************************************************/

	i_returncode = fn_upd_Fxchngbkack (ptr_st_xchngbook,c_ServiceName,c_err_msg);
	if ( i_returncode != 0)
	{
		fn_errlog ( c_ServiceName, "S31125", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

  /*** Ver 4.0 Starts Here ****/
  d_trail_amt=0;
  d_lmt_offset=0;
  c_new_mod_ord_flg='\0';

  if ( st_orderbook.c_prd_typ == 'F' )
  {
    if( c_prcimp_flg == 'Y' )  /** Trail order modification ***/
    {
      if( ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_ACPT ||
          ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_RJCT ||
          ptr_st_xchngbook->l_ors_msg_typ == ORS_NEW_ORD_ACPT ||
          ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_ACPT
        )
      {
        /**** Calculate the new incremental price ***/
        if( ptr_st_xchngbook->l_ors_msg_typ == ORS_NEW_ORD_ACPT ||
            ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_ACPT
          )
        {
           c_rqst_flg = 'A' ;
           c_new_mod_ord_flg='P';
        }
        else if( ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_RJCT
               )
        {
           c_rqst_flg = 'J' ;
           c_new_mod_ord_flg='M';
        }
        else if( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_ACPT )
        {
           c_rqst_flg = 'R' ;
           c_new_mod_ord_flg='M';
        }

        i_returncode  = fn_cal_incrmntl_price(c_ServiceName,
                              c_new_mod_ord_flg,
                              c_rqst_flg,
                              ptr_st_err_msg->c_err_msg,
                              &st_orderbook,
                              &st_xchngbook
                              );
        if (  i_returncode != 0 )
        {
          fn_errlog ( c_ServiceName, "S31130", SQLMSG, c_err_msg );
          fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
        }

      } /***
      else
      {
        c_rqst_flg = 'A' ;
        c_new_mod_ord_flg='M';
        i_returncode  = fn_cal_incrmntl_price(c_ServiceName,
                              c_new_mod_ord_flg,
                              c_rqst_flg,
                              ptr_st_err_msg->c_err_msg,
                              &st_orderbook,
                              &st_xchngbook
                              );
        if (  i_returncode != 0 )
        {
          fn_errlog ( c_ServiceName, "S31135", LIBMSG, c_err_msg );
          fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
          free((char*) ptr_st_xchngbook);
          tpfree((char*) ptr_fml_Sbuf);
          Fadd32( ptr_fml_Ibuf, FFO_ERR_MSG, ptr_st_err_msg->c_err_msg, 0 );
          free((char*) ptr_st_err_msg);
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Ibuf, 0, 0 );
        }
      } ***/
    }
  }

  /*** ver 4.0 Ends Here *****/

 /** ver 5.2 started **/
 /** Releases margin for modification acceptance of orders for which the quantity has been reduced
     and modification rejection of orders for which the quantity has been increased **/

 fn_userlog(c_ServiceName,"-*-*-*- i_source_dest_flg :%d:",i_source_dest_flg );
 fn_userlog(c_ServiceName,"-*-*-*- st_orderbook.c_prd_typ :%c: ",st_orderbook.c_prd_typ );
 fn_userlog(c_ServiceName,"-*-*-*- st_orderbook.c_spl_flg :%c:",st_orderbook.c_spl_flg );

 if( st_orderbook.c_prd_typ == 'F' && st_orderbook.c_spl_flg == ROLLOVER_WITH_SPREAD && i_source_dest_flg == 1 )
  {
   memcpy( &st_frst_ordbk, &st_orderbook , sizeof( struct vw_orderbook) );
   fn_userlog(c_ServiceName,"ptr_st_xchngbook->l_ors_msg_typ :%ld:",ptr_st_xchngbook->l_ors_msg_typ);
   fn_userlog(c_ServiceName,"l_sroll_diff_amt :%ld:",l_sroll_diff_amt );

	 /*** Ver 6.0 Starts ***/
   fn_userlog(c_ServiceName,"st_frst_ordbk.l_exctd_qty :%ld:",st_frst_ordbk.l_exctd_qty);
	 st_frst_ordbk.l_ord_tot_qty = (st_frst_ordbk.l_ord_tot_qty - st_frst_ordbk.l_exctd_qty);
	 /*** Ver 6.0 Ends ***/
   st_next_ordbk.l_ord_lmt_rt = st_frst_ordbk.l_ord_lmt_rt;
   st_next_ordbk.l_ord_tot_qty = st_frst_ordbk.l_ord_tot_qty;

   fn_userlog( c_ServiceName,"st_frst_ordbk.l_ord_lmt_rt :%ld: st_frst_ordbk.l_ord_tot_qty :%ld:",st_frst_ordbk.l_ord_lmt_rt,st_frst_ordbk.l_ord_tot_qty );

    switch ( ptr_st_xchngbook->l_ors_msg_typ )
    {
       case ORS_NEW_ORD_ACPT:
       case ORS_MOD_ORD_ACPT:

            fn_userlog(c_ServiceName," ***** INSIDE ORS_NEW_ORD_ACPT/ORS_MOD_ORD_ACPT CASE ***** ");

            if( ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_ACPT )
            {
                strcpy( c_narration_id , REL_ROMARG_ORDMOD_ACCPT);
            }
            else
            {
                strcpy( c_narration_id , REL_ROMARG_ORD_ACCPT);
            }
            if( l_sroll_diff_amt != 0 )
            {
              i_returncode = fn_upd_limits( c_ServiceName,
                                            &st_pstn_actn,
                                            ptr_st_err_msg,
                                            c_narration_id,
                                            DEBIT_WITHOUT_LIMIT,
                                            (double) l_sroll_diff_amt,
                                            &d_balance_amt );

             if ( i_returncode != 0 )
             {
                      fn_errlog ( c_ServiceName, "S31140", LIBMSG, c_err_msg );
                      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
             }
             }

             if( ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_ACPT )
            {
                strcpy( c_narration_id , REL_ROPNL_ORDMOD_ACCPT);
            }
            else
            {
                strcpy( c_narration_id , REL_ROPNL_ORD_ACCPT);
            }


             if( l_sroll_lss_amt != 0)
             {
             i_returncode = fn_upd_limits( c_ServiceName,
                                           &st_pstn_actn,
                                           ptr_st_err_msg,
                                           c_narration_id,
                                           DEBIT_WITHOUT_LIMIT,
                                           (double)l_sroll_lss_amt,
                                           &d_balance_amt);

              if ( i_returncode != 0 )
              {
                   fn_errlog ( c_ServiceName, "S31145", LIBMSG, c_err_msg );
                   fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                   tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }
              }
              i_returncode = tpsuspend ( &tranid, 0 );

              i_trnsctn_temp = fn_begintran( c_ServiceName, ptr_st_err_msg->c_err_msg); 
              if ( i_trnsctn_temp == -1 )
              {
                   tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
              } 
              fn_userlog( c_ServiceName," *********** before fn_chk_nrml_mrgn *********** " );

              i_returncode = fn_chk_nrml_mrgn(c_ServiceName,st_frst_ordbk,st_next_ordbk,&d_diff_amt,&d_diff_loss,EXG_RESPONSE,c_err_msg );

              if ( i_returncode != 0 )
              {
                   fn_errlog ( c_ServiceName, "S31150", LIBMSG, c_err_msg );
                   fn_aborttran( c_ServiceName, i_trnsctn_temp, ptr_st_err_msg->c_err_msg );
                   fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                   tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }

              fn_userlog( c_ServiceName," i_trnsctn is :%d:",i_trnsctn );

              fn_userlog( c_ServiceName," *********** after fn_chk_nrml_mrgn *********** " );

fn_userlog( c_ServiceName, " d_diff_amt and d_diff_loss is :%lf: :%lf:",d_diff_amt,d_diff_loss );

              i_trnsctn_temp = fn_aborttran( c_ServiceName, i_trnsctn_temp, ptr_st_err_msg->c_err_msg);
              if ( i_trnsctn_temp == -1 )
              {
                    tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
              }

              i_returncode = tpresume(&tranid, 0);
              if ( i_returncode == -1 )
              {
                  tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
              }

fn_userlog( c_ServiceName, "Before checking if negative margin  d_diff_amt and d_diff_loss is :%lf: :%lf:",d_diff_amt,d_diff_loss );

              if( ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_ACPT )
              {
                strcpy( c_narration_id , BLK_ROMARG_ORDMOD_ACCPT);
              }   
              else
              {
                strcpy( c_narration_id , BLK_ROMARG_ORD_ACCPT);
              }  

              if( d_diff_amt < 0 )
              {
                  i_returncode = fn_upd_limits( c_ServiceName,
                                                &st_pstn_actn,
                                                ptr_st_err_msg,
                                                c_narration_id,
                                                DEBIT_TILL_LIMIT,
                                                d_diff_amt,
                                                &d_balance_amt);

                  if ( i_returncode != 0 )
                  {
                       fn_errlog ( c_ServiceName, "S31155", LIBMSG, c_err_msg );
                       fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                       tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                  }
                  /*** 100 Rs difference out of which customer has only 70 rs so bal amount 30,update 70 in fod ***/
                  d_diff_amt = (d_diff_amt * -1)+d_balance_amt;
              }
              else
              {
                    d_diff_amt = 0;
fn_userlog(c_ServiceName," ** After checking if negative margin d_diff_amt:%lf:",d_diff_amt );
              }

              if( ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_ACPT )
              {
                strcpy( c_narration_id , BLK_ROPNL_ORDMOD_ACCPT);
              }
              else
              {
                strcpy( c_narration_id , BLK_ROPNL_ORD_ACCPT);
              }


              if( d_diff_loss < 0 )
              {
                  i_returncode = fn_upd_limits( c_ServiceName,
                                                &st_pstn_actn,
                                                ptr_st_err_msg,
                                                c_narration_id,
                                                DEBIT_TILL_LIMIT,
                                                d_diff_loss,
                                                &d_balance_amt);

                  if ( i_returncode != 0 )
                  {
                       fn_errlog ( c_ServiceName, "S31160", LIBMSG, c_err_msg );
                       fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                       tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                  }
                  d_diff_loss = ( d_diff_loss * -1 ) + d_balance_amt;
              }
              else
              {
                   d_diff_loss = 0;
              }
fn_userlog(c_ServiceName," ************* d_diff_amt is :%lf: and d_diff_loss is :%lf:************* suchita *******",d_diff_amt,d_diff_loss);
              fn_userlog(c_ServiceName,"Before FOD Update d_balance_amt :%lf:",d_balance_amt);
              EXEC SQL
              UPDATE FOD_FO_ORDR_DTLS
              SET    FOD_SROLL_DIFF_AMT = :d_diff_amt,FOD_SROLL_LSS_AMT = :d_diff_loss
              WHERE  FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

              if( SQLCODE != 0 )
              {
                   fn_errlog ( c_ServiceName, "S31165", LIBMSG, c_err_msg );
                   fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                   tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }
              break;

       case ORS_CAN_ORD_ACPT:
fn_userlog(c_ServiceName,"****  INSIDE ORS_CAN_ORD_ACPT **** and l_sroll_diff_amt is :%ld:",l_sroll_diff_amt);

           strcpy( c_narration_id , REL_ROMARG_ORDCAN_ACCPT);


            if( l_sroll_diff_amt != 0 )
            {
            i_returncode = fn_upd_limits( c_ServiceName,
                                          &st_pstn_actn,
                                          ptr_st_err_msg,
                                          c_narration_id,
                                          DEBIT_WITHOUT_LIMIT, 
                                          (double)l_sroll_diff_amt,
                                          &d_balance_amt);


             if ( i_returncode != 0 )
             {
                  fn_errlog ( c_ServiceName, "S31170", LIBMSG, c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
             }
             l_sroll_diff_amt = 0;
             }

             strcpy( c_narration_id , REL_ROPNL_ORDCAN_ACCPT);


             if( l_sroll_lss_amt != 0)
             {
             i_returncode = fn_upd_limits( c_ServiceName,
                                           &st_pstn_actn,
                                           ptr_st_err_msg,
                                           c_narration_id,
                                           DEBIT_WITHOUT_LIMIT, 
                                           (double)l_sroll_lss_amt,
                                           &d_balance_amt);

              if ( i_returncode != 0 )
              {
                   fn_errlog ( c_ServiceName, "S31175", LIBMSG, c_err_msg );
                   fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                   tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );

              }
              l_sroll_lss_amt = 0;
              }

             EXEC SQL
                  UPDATE FOD_FO_ORDR_DTLS
                  SET    FOD_SROLL_DIFF_AMT = :l_sroll_diff_amt,FOD_SROLL_LSS_AMT = :l_sroll_lss_amt
                  WHERE  FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

                  if( SQLCODE != 0 )
                  {
                   fn_errlog ( c_ServiceName, "S31180", LIBMSG, c_err_msg );
                   fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                   tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                  }
            break;

       case ORS_MOD_ORD_RJCT:
       case ORS_CAN_ORD_RJCT:
            fn_userlog( c_ServiceName," **** INSIDE ORS_MOD_ORD_RJCT/ORS_CAN_ORD_RJCT **** ");
            if( ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_RJCT )
            {
                strcpy( c_narration_id , ON_ROMOD_REJ);
            }
            else if ( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_RJCT )
            {
                strcpy( c_narration_id , ON_ROCAN_REJ);
            }

            if( l_sroll_diff_amt != 0 )
            {
            i_returncode = fn_upd_limits(  c_ServiceName,
                                           &st_pstn_actn,
                                           ptr_st_err_msg,
                                           c_narration_id,
                                           DEBIT_WITHOUT_LIMIT,
                                           (double) l_sroll_diff_amt,
                                           &d_balance_amt);


             if ( i_returncode != 0 )
             {
                  fn_errlog ( c_ServiceName, "S31185", LIBMSG, c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
             }
             }

            if( ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_RJCT )
            {
                strcpy( c_narration_id , ON_ROMOD_PL_REJ);
            }
            else if ( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_RJCT )
            {
                strcpy( c_narration_id , ON_ROCAN_PL_REJ);
            }
             if( l_sroll_lss_amt != 0 )
             {
               i_returncode = fn_upd_limits( c_ServiceName,
                                             &st_pstn_actn,
                                             ptr_st_err_msg,
                                             c_narration_id,
                                             DEBIT_WITHOUT_LIMIT,
                                             (double) l_sroll_lss_amt,
                                             &d_balance_amt);

                if ( i_returncode != 0 )
                {
                     fn_errlog ( c_ServiceName, "S31190", LIBMSG, c_err_msg );
                     fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                     tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }
              }
              i_returncode = tpsuspend ( &tranid, 0 );
              if ( i_returncode == -1 )
              {
                     fn_errlog ( c_ServiceName, "S31195", LIBMSG, c_err_msg );
                     fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                     tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
               }

              i_trnsctn_temp = fn_begintran( c_ServiceName, ptr_st_err_msg->c_err_msg);
              if ( i_trnsctn_temp == -1 )
              {
                   fn_errlog ( c_ServiceName, "S31200", LIBMSG, c_err_msg );
                   tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
              }

i_returncode = fn_chk_nrml_mrgn(c_ServiceName,st_frst_ordbk,st_next_ordbk,&d_diff_amt,&d_diff_loss,EXG_RESPONSE,c_err_msg );

                if ( i_returncode != 0 )
                {
                     fn_errlog ( c_ServiceName, "S31205", LIBMSG, c_err_msg );
                     fn_aborttran( c_ServiceName, i_trnsctn_temp, ptr_st_err_msg->c_err_msg );
                     fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                     tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }

              fn_userlog(c_ServiceName," d_diff_amt :%lf: and d_diff_loss is :%lf:",d_diff_amt,d_diff_loss);

              i_trnsctn_temp = fn_aborttran( c_ServiceName, i_trnsctn_temp, ptr_st_err_msg->c_err_msg);
              if ( i_trnsctn_temp == -1 )
              {
                     fn_errlog ( c_ServiceName, "S31210", LIBMSG, c_err_msg );
                     tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
              }

              i_returncode = tpresume(&tranid, 0);
              if ( i_returncode == -1 )
              {
                  fn_errlog ( c_ServiceName, "S31215", LIBMSG, c_err_msg );
                  tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
              }

            if( ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_RJCT )
            {
                strcpy( c_narration_id ,BLK_ROMARG_ORDMOD_RJCT);
            }
            else if ( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_RJCT )
            {
                strcpy( c_narration_id ,BLK_ROMARG_ORDCAN_RJCT);
            }

              if( d_diff_amt < 0 )
              {
                  i_returncode = fn_upd_limits( c_ServiceName,
                                                &st_pstn_actn,
                                                ptr_st_err_msg,
                                                c_narration_id,
                                                DEBIT_TILL_LIMIT,
                                                d_diff_amt,
                                                &d_balance_amt);

                  if ( i_returncode != 0 )
                  {
                       fn_errlog ( c_ServiceName, "S31220", LIBMSG, c_err_msg );
                       fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                       tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                  }
                  d_diff_amt = (d_diff_amt * -1)+ d_balance_amt;
              }
              else
              {
                    d_diff_amt = 0;
              }

             if( ptr_st_xchngbook->l_ors_msg_typ == ORS_MOD_ORD_RJCT )
             {
                strcpy( c_narration_id ,BLK_ROPNL_ORDMOD_RJCT);
             }
             else if ( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_RJCT )
             {
                strcpy( c_narration_id ,BLK_ROPNL_ORDCAN_RJCT);
             }

              if( d_diff_loss < 0 )
              {
                  i_returncode = fn_upd_limits( c_ServiceName,
                                                &st_pstn_actn,
                                                ptr_st_err_msg,
                                                c_narration_id,
                                                DEBIT_TILL_LIMIT,
                                                d_diff_loss,
                                                &d_balance_amt);

                  if ( i_returncode != 0 )
                  {
                       fn_errlog ( c_ServiceName, "S31225", LIBMSG, c_err_msg );
                       fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                       tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                  }
                  d_diff_loss = (d_diff_loss * -1)+d_balance_amt;
               }
               else
               {
                   d_diff_loss = 0;

               }
                  fn_userlog(c_ServiceName,"Before FOD Update d_balance_amt :%lf:",d_balance_amt);
                  EXEC SQL
                  UPDATE FOD_FO_ORDR_DTLS
                  SET    FOD_SROLL_DIFF_AMT = :d_diff_amt,FOD_SROLL_LSS_AMT = :d_diff_loss
                  WHERE  FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

                  if( SQLCODE != 0 )
                  {
                   fn_errlog ( c_ServiceName, "S31230", LIBMSG, c_err_msg );
                   fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                   tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                  }
            break;
       case ORS_ORD_EXP:
       case ORS_NEW_ORD_RJCT:
            fn_userlog(c_ServiceName,"**** INSIDE ORS_NEW_ORD_RJCT / ORS_ORD_EXP ****");

            if( ptr_st_xchngbook->l_ors_msg_typ == ORS_ORD_EXP )
            {
                strcpy( c_narration_id ,ON_ROORD_REJ);
            }
            else
            {
                strcpy( c_narration_id ,ON_ROORD_REJ);
            }

            if( l_sroll_diff_amt != 0 )
            {
            i_returncode = fn_upd_limits( c_ServiceName,
                                          &st_pstn_actn,
                                          ptr_st_err_msg,
                                          c_narration_id,
                                          DEBIT_WITHOUT_LIMIT,
                                          (double)l_sroll_diff_amt,
                                          &d_balance_amt);


            if ( i_returncode != 0 )
            {
                  fn_errlog ( c_ServiceName, "S31235", LIBMSG, c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }
            l_sroll_diff_amt = 0;

            }

             if( l_sroll_lss_amt != 0 )
             {
               i_returncode = fn_upd_limits( c_ServiceName,
                                             &st_pstn_actn,
                                             ptr_st_err_msg,
                                             c_narration_id,
                                             DEBIT_WITHOUT_LIMIT,
                                             (double) l_sroll_lss_amt,
                                             &d_balance_amt);

                if ( i_returncode != 0 )
                {
                     fn_errlog ( c_ServiceName, "S31240", LIBMSG, c_err_msg );
                     fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                     tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }
                 l_sroll_lss_amt = 0;
              }

             EXEC SQL
                  UPDATE FOD_FO_ORDR_DTLS
                  SET    FOD_SROLL_DIFF_AMT = :l_sroll_diff_amt,FOD_SROLL_LSS_AMT=:l_sroll_lss_amt
                  WHERE  FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

                  if( SQLCODE != 0 )
                  {
                   fn_errlog ( c_ServiceName, "S31245", LIBMSG, c_err_msg );
                   fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                   tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                  }
            break;
    }
 }
fn_userlog( c_ServiceName," printingggg i_trnsctn_temp :%d: i_trnsctn :%d:", i_trnsctn_temp,i_trnsctn );
 /** ver 5.2 ended **/
 /*** Ver 2.1 starts here ***/
 if ( st_orderbook.c_prd_typ == SLTP_FUTUREPLUS )	
 {
		/**  strcpy(st_pstn_actn.c_channel,"SYS"); Commented in Ver 4.1 ***/
		strcpy(st_pstn_actn.c_alias,"*");

  	if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  	{
			fn_userlog(c_ServiceName," stage 1 ");
		}

		EXEC SQL
		SELECT
					fod_sltp_ordr_rfrnc,				
					nvl(fod_amt_blckd,0),				
					nvl(fod_lss_amt_blckd,0),		
					nvl(fod_fc_flag,'*'),
					nvl(fod_diff_amt_blckd,0),
					nvl(fod_diff_lss_amt_blckd,0)
		INTO
					:c_sltp_ord_rfrnc,	
	   		  :d_amt_blkd,				
					:d_lss_amt_blkd,		
					:c_fc_flg,
					:d_diff_amt_blkd,
					:d_diff_lss_amt_blkd
		FROM  fod_fo_ordr_dtls
		WHERE fod_ordr_rfrnc = :st_orderbook.c_ordr_rfrnc;
		if ( ( SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
		{
			fn_errlog ( c_ServiceName, "S31250", SQLMSG, ptr_st_err_msg->c_err_msg );
			fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
			tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
		}
	
  	if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  	{		
			fn_userlog(c_ServiceName," ptr_st_xchngbook->l_ors_msg_typ is = :%ld:",ptr_st_xchngbook->l_ors_msg_typ);
		}
	if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/ /* Added in Ver TOL */
  		 {
		 		fn_userlog(c_ServiceName," stage 2 ");
	 	}
		switch ( ptr_st_xchngbook->l_ors_msg_typ )
		{
  		// if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  		// {
		// 		fn_userlog(c_ServiceName," stage 2 ");
		// 	}   /** Debug message removed in Ver TOL **/
		
			case	ORS_NEW_ORD_ACPT:
						break;

			case	ORS_NEW_ORD_RJCT:
						
						/*If it is a fresh order rejection, place a cancel request for the cover order*/
						if(c_fc_flg == 'F') /*Fresh Order*/
						{
  						if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  						{
								fn_userlog(c_ServiceName,"Stage 3 ");
							}

							/*Check if the cover order is in open state*/
							EXEC SQL
							SELECT	FOD_ORDR_STTS,
											FOD_EXEC_QTY
							INTO		:c_ordr_stts,
											:l_exctd_qty
							FROM		FOD_FO_ORDR_DTLS
							WHERE		FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31255", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}

							if(c_ordr_stts == 'Q')
							{
                i_canrej_flag = 0; /*** Added in ver 5.0 ***/
								/*** fn_userlog(c_ServiceName,"cancellation of cover order not possible as order already placed with the exchange");
								fn_errlog ( c_ServiceName, "S31260", LIBMSG, c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								** Commented in Ver 2.4 ***/

  							if(DEBUG_MSG_LVL_0)   /*** Ver 3.7 ***/
  							{	
									fn_userlog(c_ServiceName,"Cover order in QUEUED state, hence dont release margin and loss");  /*** Ver 2.4 ***/
								}
							}
						  else if(c_ordr_stts != 'J' && c_ordr_stts != 'C' && c_ordr_stts != 'F' && l_exctd_qty == 0) /*** In ver 5.5 'F' status is added ***/	
							{
                i_canrej_flag = 0; /*** Added in ver 5.0 ***/
								strcpy(c_remarks,"System placed Cancellation");

								i_returncode = fn_call_svc_fml ( c_ServiceName,
																			 c_err_msg,
																			 "SFO_CANCEL_FUT",
																			 0,
																			 9,
																			 0,
																			 FFO_USR_ID, (char *)st_pstn_actn.c_user_id,
																			 FFO_SSSN_ID, (char *)&st_pstn_actn.l_session_id,
																			 FFO_XCHNG_CD,(char *)st_orderbook.c_xchng_cd,
																			 FFO_EBA_MTCH_ACT_NO,(char *)st_orderbook.c_cln_mtch_accnt,
																			 FFO_ORDR_RFRNC, (char *)c_sltp_ord_rfrnc,
																			 FFO_XCHNG_RMRKS, (char *)c_remarks,
																			 FFO_CHANNEL,(char *)st_pstn_actn.c_channel,     
																			 FFO_ALIAS,(char *)st_pstn_actn.c_alias,
																			 FFO_PIPE_ID, (char *)st_orderbook.c_pipe_id );

								if ( i_returncode != SUCC_BFR )
								{
									fn_errlog ( c_ServiceName, "S31265", LIBMSG, c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}
							} /*** ver 5.0 started ***/
              else if(c_ordr_stts != 'J' && c_ordr_stts != 'C' && c_ordr_stts != 'F' ) /*** In ver 5.5 'F' status is added ***/ 
              {
                  i_canrej_flag = 0;
              } /*** ver 5.0 ended ***/
						 }
								
						 /*** Ver 2.4 ** Starts ***/

				/*** else if(c_fc_flg == 'C') *Cover Order* commented in ver 5.0 ***/
             if( ( c_fc_flg == 'C' ) || ( c_fc_flg == 'F' && i_canrej_flag == 1 )) /*** added in ver 5.0 ***/
             {
  							if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  							{
									fn_userlog(c_ServiceName," Stage 4 ");
								}

                d_amt_blkd = 0;
                d_lss_amt_blkd = 0;

                EXEC SQL
                  SELECT  FOD_AMT_BLCKD,
                          FOD_LSS_AMT_BLCKD,
                          FOD_ORDR_STTS
                  INTO    :d_amt_blkd,
                          :d_lss_amt_blkd,
                          :c_frsh_ordr_stts
                  FROM    FOD_FO_ORDR_DTLS
           /***   WHERE   FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc; commented in ver 5.0 ***/
                  WHERE   FOD_ORDR_RFRNC = DECODE( :c_fc_flg,'C',:c_sltp_ord_rfrnc,'F',:st_orderbook.c_ordr_rfrnc); /*** added in ver 5.0 ***/

                if(SQLCODE != 0)
                {
                  fn_errlog ( c_ServiceName, "S31270", SQLMSG, ptr_st_err_msg->c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }

                /*** ver 5.0 starts ***/
                if(DEBUG_MSG_LVL_0)
                {
                   fn_userlog(c_ServiceName,"Inside rejection case c_fc_flg is :%c: i_canrej_flag is :%d: and c_frsh_ordr_stts is :%c:",c_fc_flg,i_canrej_flag,c_frsh_ordr_stts );
                }
                /*** ver 5.0 ends ***/

                if(DEBUG_MSG_LVL_3)
                {
                  fn_userlog(c_ServiceName,"Margin Amount of the fresh order :%lf:", d_amt_blkd);
                  fn_userlog(c_ServiceName,"Loss Amount of the fresh order :%lf:", d_lss_amt_blkd);
                }

                if( c_frsh_ordr_stts == 'J'  || c_frsh_ordr_stts == 'C' || c_frsh_ordr_stts == 'F' ) /*** Ver 2.6 Cancel status added ***/ /*** In ver 5.5 'F' status is added ***/
                {
                  /*Release the margin and loss amount blocked*/
                  strcpy( c_narration_id , ON_ORDER_REJECTION);

                  if(d_amt_blkd != 0)
                  {
                    if(DEBUG_MSG_LVL_3)
                    {
                      fn_userlog(c_ServiceName,"Release the margin amount blocked");
                    }

                    i_returncode = fn_upd_limits( c_ServiceName,
                                    &st_pstn_actn,
                                    ptr_st_err_msg,
                                    c_narration_id,
                                    DEBIT_WITHOUT_LIMIT,
                         /***       d_amt_blkd * -1, commented in ver 5.0 ***/
                                    d_amt_blkd, /*** added in ver 5.0 ***/
                                    &d_balance_amt);


                    if ( i_returncode != 0 )
                    {
                      fn_errlog ( c_ServiceName, "S31275", LIBMSG, c_err_msg );
                      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                    }
                  }
									if(d_lss_amt_blkd != 0)
                  {
                    if(DEBUG_MSG_LVL_3)
                    {
                      fn_userlog(c_ServiceName,"Release the loss amount blocked");
                    }

                    i_returncode = fn_upd_limits( c_ServiceName,
                                    &st_pstn_actn,
                                    ptr_st_err_msg,
                                    c_narration_id,
                                    DEBIT_WITHOUT_LIMIT,
                             /***   d_lss_amt_blkd * -1, added in ver 5.0 ***/
                                    d_lss_amt_blkd, /*** added in ver 5.0 ***/
                                    &d_balance_amt);


                    if ( i_returncode != 0 )
                    {
                      fn_errlog ( c_ServiceName, "S31280", LIBMSG, c_err_msg );
                      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                    }
                  }

                  EXEC SQL
                  UPDATE     FOD_FO_ORDR_DTLS
               /* SET        FOD_AMT_BLCKD     = 0 ,
                             FOD_LSS_AMT_BLCKD = 0
                  WHERE      FOD_ORDR_RFRNC    = :c_sltp_ord_rfrnc; *** commented in ver 5.0 */
                  /** ver 5.0 starts **/      
                  SET        FOD_AMT_BLCKD     = FOD_AMT_BLCKD -  :d_amt_blkd,
                             FOD_LSS_AMT_BLCKD = FOD_LSS_AMT_BLCKD - :d_lss_amt_blkd,
														 FOD_SLTP_MRGN			= 0,				/*** Ver 5.9 ***/	
														 FOD_SLTP_PL				= 0,				/*** Ver 5.9 ***/	 
														 FOD_SLTP_SEBI_MRGN= 0					/*** Ver 5.9 ***/
                  WHERE      FOD_ORDR_RFRNC    = DECODE( :c_fc_flg,'C',:c_sltp_ord_rfrnc,'F',:st_orderbook.c_ordr_rfrnc); /*** ver 5.0 ends ***/
 
                  if(SQLCODE != 0)
                  {
                    fn_errlog ( c_ServiceName, "S31285", SQLMSG, ptr_st_err_msg->c_err_msg );
                    fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                  }
                }
             }

				 		 /*** Ver 2.4 ** Ends ***/

						break;

			case	ORS_MOD_ORD_ACPT:
				/*** Ver 3.1 starts here ***/
				if(c_fc_flg == 'C') /*Cover*/
        {
						d_amt_blkd = 0;
						d_lss_amt_blkd = 0;

						EXEC SQL
							SELECT  FOD_AMT_BLCKD,
											FOD_LSS_AMT_BLCKD
											/*** FOD_LMT_RT	 *** Commented on 23Oct2015 in Ver 3.5 ***/
							INTO    :d_amt_blkd,
											:d_lss_amt_blkd
											/***:l_fsh_ord_lmt_rt *** Commented on 23Oct2015 in Ver 3.5 ***/
							FROM    FOD_FO_ORDR_DTLS
							WHERE   FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

						if(SQLCODE != 0)
						{
							fn_errlog ( c_ServiceName, "S31290", SQLMSG, ptr_st_err_msg->c_err_msg );
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}

						/*** EXEC SQL
							SELECT  FOD_LMT_RT
							INTO    :l_cvr_ord_lmt_rt
							FROM    FOD_FO_ORDR_DTLS
							WHERE   FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc; *** Commented on 23Oct2015 in Ver 3.5 ***/


							/*** Added on 23Oct2015 Ver 3.5 ***/
							EXEC SQL
								SELECT	FXB_LMT_RT
								INTO		:l_fsh_ord_lmt_rt
								FROM		FXB_FO_XCHNG_BOOK
								WHERE		FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc
								AND			FXB_MDFCTN_CNTR = (select max(FXB_MDFCTN_CNTR) FROM FXB_FO_XCHNG_BOOK
								WHERE		FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc
								AND     FXB_PLCD_STTS != 'J' );

							
							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31295", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}

							EXEC SQL
								SELECT	FXB_LMT_RT
								INTO		:l_cvr_ord_lmt_rt
								FROM		FXB_FO_XCHNG_BOOK
								WHERE		FXB_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc
								AND			FXB_MDFCTN_CNTR = (select max(FXB_MDFCTN_CNTR) FROM FXB_FO_XCHNG_BOOK
								WHERE		FXB_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc);
							/*** Ver 3.5 ends ***/

						if(SQLCODE != 0)
						{
							fn_errlog ( c_ServiceName, "S31300", SQLMSG, ptr_st_err_msg->c_err_msg );
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}

						if(DEBUG_MSG_LVL_3)
						{
							fn_userlog(c_ServiceName,"Margin Amount of the fresh order :%lf:", d_amt_blkd);
							fn_userlog(c_ServiceName,"Loss Amount of the fresh order :%lf:", d_lss_amt_blkd);
							fn_userlog(c_ServiceName,"Fresh order Limit Rate :%ld:", l_fsh_ord_lmt_rt);
							fn_userlog(c_ServiceName,"Cover order Limit Rate :%ld:", l_cvr_ord_lmt_rt);
						}
            
						/* Calculate Margin amount to be blocked */
						i_returncode =    fn_cal_mrgn_sltp 
															(
																c_ServiceName, 
																c_sltp_ord_rfrnc,
																l_fsh_ord_lmt_rt,				
																l_cvr_ord_lmt_rt,
																&d_amt_to_be_blckd,
																&d_lss_amt_to_be_blckd,
																c_ref_rmrks,  /*** Ver 5.9 ***/
                                &d_comp1_mrgn,/*** Ver 5.9 ***/
                                &d_comp1_pl,  /*** Ver 5.9 ***/
                                &d_xchng_amt, /*** Ver 5.9 ***/
																c_err_msg	
															);

						if ( i_returncode != SUCC_BFR )
						{
							fn_errlog ( c_ServiceName, "S31305", LIBMSG, ptr_st_err_msg->c_err_msg );
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}

						/* Difference margin & loss */
						d_diff_amt = (double) d_amt_blkd - d_amt_to_be_blckd;
						d_diff_loss = (double) d_lss_amt_blkd - d_lss_amt_to_be_blckd;
		
						d_diff_amt  =  (long) (long) d_diff_amt ;
						d_diff_loss =  (long) (long) d_diff_loss ;

						 if(DEBUG_MSG_LVL_3)
						{
							fn_userlog(c_ServiceName,"d_amt_blkd = :%lf:", d_amt_blkd);
							fn_userlog(c_ServiceName,"d_amt_to_be_blckd = :%lf:", d_amt_to_be_blckd);
							fn_userlog(c_ServiceName,"d_diff_amt = :%lf:", d_diff_amt);

							fn_userlog(c_ServiceName,"d_lss_amt_blkd = :%lf:", d_lss_amt_blkd);
							fn_userlog(c_ServiceName,"d_lss_amt_to_be_blckd = :%lf:", d_lss_amt_to_be_blckd);
							fn_userlog(c_ServiceName,"d_diff_loss = :%lf:", d_diff_loss);
						}

						/* Not releasing the profit */
						/* Block difference in the margin amount */

						/***	Debit margin loss/profit when limit .                           ***/
						/***	Release loss amount blocked only in case of  PROFIT .					  ***/   
						/***	In case of LOSS , deficit  loss amount is released or blocked.  ***/

						strcpy( c_narration_id , ON_MODIFICATION_ACCEPTANCE );
						if(d_diff_amt != 0)
						{
								d_balance_amt = 0.0;

								if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"Block diff in margin amount");
								}
								i_returncode = fn_upd_limits( c_ServiceName,
																							&st_pstn_actn,
																							ptr_st_err_msg,
																							c_narration_id,
																							DEBIT_TILL_LIMIT, /*** WHEN limit to TILL limit ***/
																							d_diff_amt,
																							&d_balance_amt);

								if ( i_returncode != 0 )
								{
									fn_errlog ( c_ServiceName, "S31310", LIBMSG, c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
									
								}

								d_diff_amt = d_diff_amt - d_balance_amt ;
						}

						/*** Ver 5.9 started ***/
						MEMSET(c_tmp_rmrks);    
						strcpy ( c_tmp_rmrks, st_pstn_actn.c_ref_rmrks ); 
						sprintf(st_pstn_actn.c_ref_rmrks,"%s%s",c_ref_rmrks,c_tmp_rmrks);  
						/*** Ver 5.9 ends ***/

						/* Block difference in the loss amount */
						strcpy( c_narration_id , ON_MODIFICATION_ACCEPTANCE );
						if( d_lss_amt_to_be_blckd < 0)
						{
								if(DEBUG_MSG_LVL_3)
								{
									 fn_userlog(c_ServiceName,"d_lss_amt_to_be_blckd < 0 Condition..");
								}

								if( d_lss_amt_blkd !=0)
								{
									i_returncode = fn_upd_limits( c_ServiceName,
																								&st_pstn_actn,
																								ptr_st_err_msg,
																								c_narration_id,
																								DEBIT_TILL_LIMIT,  /*** WHEN limit to TILL limit ***/
																								d_lss_amt_blkd,
																								&d_balance_amt);


									if ( i_returncode != 0 )
									{
										fn_errlog ( c_ServiceName, "S31315", LIBMSG, c_err_msg );
										fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
										tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); 
									}
								}
								
								d_diff_loss= d_lss_amt_blkd ;
						}
						else  if(d_diff_loss != 0)
						{
								d_balance_amt = 0.0;

								if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"Block diff in loss amount");
								}
								i_returncode = fn_upd_limits( c_ServiceName,
																							&st_pstn_actn,
																							ptr_st_err_msg,
																							c_narration_id,
																							DEBIT_TILL_LIMIT,  /*** WHEN limit to TILL limit ***/
																							d_diff_loss,
																							&d_balance_amt);


								if ( i_returncode != 0 )
								{
									fn_errlog ( c_ServiceName, "S31320", LIBMSG, c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); 
								}
								d_diff_loss = d_diff_loss  - d_balance_amt ;
						}

						strcpy ( st_pstn_actn.c_ref_rmrks, c_tmp_rmrks);    /**** Ver 5.9 ***/
		        MEMSET(c_tmp_rmrks);																/**** Ver 5.9 ***/

						EXEC SQL
							 UPDATE     FOD_FO_ORDR_DTLS
							 SET        FOD_AMT_BLCKD     = :d_amt_blkd - :d_diff_amt,
													FOD_LSS_AMT_BLCKD = :d_lss_amt_blkd - :d_diff_loss,
													FOD_SLTP_MRGN = :d_comp1_mrgn,			/*** Ver 5.9 ***/	
													FOD_SLTP_PL   = :d_comp1_pl,				/*** Ver 5.9 ***/ 
													FOD_SLTP_SEBI_MRGN = :d_xchng_amt		/*** Ver 5.9 ***/
							 WHERE      FOD_ORDR_RFRNC    = :c_sltp_ord_rfrnc;

						if(SQLCODE != 0)
						{
							fn_errlog ( c_ServiceName, "S31325", SQLMSG, ptr_st_err_msg->c_err_msg );
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}
				}
				/*** Ver 3.1 Ends here ***/
						break;

					case	ORS_MOD_ORD_RJCT:
						/*** Commented in Ver 3.1 ***
						if(c_fc_flg == 'C') *Cover*
						{
								fn_userlog(c_ServiceName," Stage 5 ");

							d_amt_blkd = 0.0;
							d_lss_amt_blkd = 0.0;

							EXEC SQL	
								 SELECT			FOD_AMT_BLCKD,
														FOD_LSS_AMT_BLCKD
								 INTO				:d_amt_blkd,
														:d_lss_amt_blkd
								 FROM				FOD_FO_ORDR_DTLS
								 WHERE			FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31330", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}
							
							if(DEBUG_MSG_LVL_3)
							{
								fn_userlog(c_ServiceName,"Margin Amount of the fresh order :%lf:", d_amt_blkd);
								fn_userlog(c_ServiceName,"Loss Amount of the fresh order :%lf:", d_lss_amt_blkd);
								fn_userlog(c_ServiceName,"d_diff_amt_blkd is :%lf:",d_diff_amt_blkd);
								fn_userlog(c_ServiceName,"d_diff_lss_amt_blkd is :%lf:",d_diff_lss_amt_blkd);
							}

							*Release the diff margin amount blocked*
							strcpy( c_narration_id , ON_MODIFICATION_REJECTION);
							if(d_diff_amt_blkd != 0)
							{	
								if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"Release the diff margin amount blocked");
								}

								i_returncode = fn_upd_limits( c_ServiceName,
																&st_pstn_actn,
																ptr_st_err_msg,
																c_narration_id,
																DEBIT_WITHOUT_LIMIT,
																d_diff_amt_blkd * -1,
																&d_balance_amt);

								if ( i_returncode != 0 )
								{
									fn_errlog ( c_ServiceName, "S31335", LIBMSG, c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}
							}

							*Release the diff loss amount blocked*
							strcpy( c_narration_id , ON_MODIFICATION_REJECTION);
							if(d_diff_lss_amt_blkd != 0)
							{	
								if(DEBUG_MSG_LVL_3)
								{		
									fn_userlog(c_ServiceName,"Release the diff loss amount blocked");
								}
								i_returncode = fn_upd_limits( c_ServiceName,
																	&st_pstn_actn,
																	ptr_st_err_msg,
																	c_narration_id,
																	DEBIT_WITHOUT_LIMIT,
																	d_diff_lss_amt_blkd * -1,
																	&d_balance_amt);


								if ( i_returncode != 0 )
								{
									fn_errlog ( c_ServiceName, "S31340", LIBMSG, c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}
							}

							EXEC SQL	
							 UPDATE			FOD_FO_ORDR_DTLS
							 SET				FOD_AMT_BLCKD			= :d_amt_blkd + :d_diff_amt_blkd ,
													FOD_LSS_AMT_BLCKD = :d_lss_amt_blkd + :d_diff_lss_amt_blkd 
							 WHERE			FOD_ORDR_RFRNC		= :c_sltp_ord_rfrnc;

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31345", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}

							EXEC SQL	
							 UPDATE			FOD_FO_ORDR_DTLS
							 SET				FOD_DIFF_AMT_BLCKD			= 0, 
													FOD_DIFF_LSS_AMT_BLCKD	= 0 
							 WHERE			FOD_ORDR_RFRNC					= :st_orderbook.c_ordr_rfrnc;

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31350", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}
						}
						break;
						*****/
						
						/*** Ver 3.1 starts here ***/
						if(c_fc_flg == 'C') /*Cover*/
						{
								d_amt_blkd = 0;
								d_lss_amt_blkd = 0;

								EXEC SQL
									SELECT  FOD_AMT_BLCKD,
													FOD_LSS_AMT_BLCKD,
													FOD_LMT_RT
									INTO    :d_amt_blkd,
													:d_lss_amt_blkd,
													:l_fsh_ord_lmt_rt
									FROM    FOD_FO_ORDR_DTLS
									WHERE   FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

								if(SQLCODE != 0)
								{
									fn_errlog ( c_ServiceName, "S31355", SQLMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}

								EXEC SQL
									SELECT  FOD_LMT_RT
									INTO    :l_cvr_ord_lmt_rt
									FROM    FOD_FO_ORDR_DTLS
									WHERE   FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

								if(SQLCODE != 0)
								{
									fn_errlog ( c_ServiceName, "S31360", SQLMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}

								if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"Margin Amount of the fresh order :%lf:", d_amt_blkd);
									fn_userlog(c_ServiceName,"Loss Amount of the fresh order :%lf:", d_lss_amt_blkd);
									fn_userlog(c_ServiceName,"Fresh order Limit Rate :%ld:", l_fsh_ord_lmt_rt);
									fn_userlog(c_ServiceName,"Cover order Limit Rate :%ld:", l_cvr_ord_lmt_rt);
								}
								
								/* Calculate Margin amount to be blocked */
								i_returncode =    fn_cal_mrgn_sltp 
																	(
																		c_ServiceName, 
																		c_sltp_ord_rfrnc,
																		l_fsh_ord_lmt_rt,				
																		l_cvr_ord_lmt_rt,
																		&d_amt_to_be_blckd,
																		&d_lss_amt_to_be_blckd,
																		c_ref_rmrks,  /*** Ver 5.9 ***/
                                    &d_comp1_mrgn,/*** Ver 5.9 ***/
                                    &d_comp1_pl,  /*** Ver 5.9 ***/
                                    &d_xchng_amt, /*** Ver 5.9 ***/
																		c_err_msg	
																	);

								if ( i_returncode != SUCC_BFR )
								{
									fn_errlog ( c_ServiceName, "S31365", LIBMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}

								/* Difference margin & loss */
								d_diff_amt = (double) d_amt_blkd - d_amt_to_be_blckd;
								d_diff_loss = (double) d_lss_amt_blkd - d_lss_amt_to_be_blckd;
				
								d_diff_amt  =  (long) (long) d_diff_amt ;
								d_diff_loss =  (long) (long) d_diff_loss ;

								 if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"d_amt_blkd = :%lf:", d_amt_blkd);
									fn_userlog(c_ServiceName,"d_amt_to_be_blckd = :%lf:", d_amt_to_be_blckd);
									fn_userlog(c_ServiceName,"d_diff_amt = :%lf:", d_diff_amt);

									fn_userlog(c_ServiceName,"d_lss_amt_blkd = :%lf:", d_lss_amt_blkd);
									fn_userlog(c_ServiceName,"d_lss_amt_to_be_blckd = :%lf:", d_lss_amt_to_be_blckd);
									fn_userlog(c_ServiceName,"d_diff_loss = :%lf:", d_diff_loss);
								}

								/* Not releasing the profit */
								/* Block difference in the margin amount */

								/***	Debit margin loss/profit when limit .                           ***/
								/***	Release loss amount blocked only in case of  PROFIT .					  ***/   
								/***	In case of LOSS , deficit  loss amount is released or blocked.  ***/

								strcpy( c_narration_id , ON_MODIFICATION_REJECTION );
								if(d_diff_amt != 0)
								{
										d_balance_amt = 0.0;

										if(DEBUG_MSG_LVL_3)
										{
											fn_userlog(c_ServiceName,"Block diff in margin amount");
										}
										i_returncode = fn_upd_limits( c_ServiceName,
																									&st_pstn_actn,
																									ptr_st_err_msg,
																									c_narration_id,
																									DEBIT_WITHOUT_LIMIT,
																									d_diff_amt,
																									&d_balance_amt);

										if ( i_returncode != 0 )
										{
											fn_errlog ( c_ServiceName, "S31370", LIBMSG, c_err_msg );
											fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
											tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
											
										}

										d_diff_amt = d_diff_amt - d_balance_amt ;
								}

								/*** Ver 5.9 started ***/
								MEMSET(c_tmp_rmrks);    
								strcpy ( c_tmp_rmrks, st_pstn_actn.c_ref_rmrks ); 
								sprintf(st_pstn_actn.c_ref_rmrks,"%s%s",c_ref_rmrks,c_tmp_rmrks);  
								/*** Ver 5.9 ends ***/

								/* Block difference in the loss amount */
								strcpy( c_narration_id , ON_MODIFICATION_REJECTION );
								if( d_lss_amt_to_be_blckd < 0)
								{
										if(DEBUG_MSG_LVL_3)
										{
											 fn_userlog(c_ServiceName,"d_lss_amt_to_be_blckd < 0 Condition..");
										}

										if( d_lss_amt_blkd !=0)
										{
											i_returncode = fn_upd_limits( c_ServiceName,
																										&st_pstn_actn,
																										ptr_st_err_msg,
																										c_narration_id,
																										DEBIT_WITHOUT_LIMIT,
																										d_lss_amt_blkd,
																										&d_balance_amt);


											if ( i_returncode != 0 )
											{
												fn_errlog ( c_ServiceName, "S31375", LIBMSG, c_err_msg );
												fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
												tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); 
											}
										}
										
										d_diff_loss= d_lss_amt_blkd ;
								}
								else  if(d_diff_loss != 0)
								{
										d_balance_amt = 0.0;

										if(DEBUG_MSG_LVL_3)
										{
											fn_userlog(c_ServiceName,"Block diff in loss amount");
										}
										i_returncode = fn_upd_limits( c_ServiceName,
																									&st_pstn_actn,
																									ptr_st_err_msg,
																									c_narration_id,
																									DEBIT_WITHOUT_LIMIT,
																									d_diff_loss,
																									&d_balance_amt);


										if ( i_returncode != 0 )
										{
											fn_errlog ( c_ServiceName, "S31380", LIBMSG, c_err_msg );
											fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
											tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); 
										}
										d_diff_loss = d_diff_loss  - d_balance_amt ;
								}

								strcpy ( st_pstn_actn.c_ref_rmrks, c_tmp_rmrks);    /**** Ver 5.9 ***/
								MEMSET(c_tmp_rmrks);																/**** Ver 5.9 ***/

								EXEC SQL
									 UPDATE     FOD_FO_ORDR_DTLS
									 SET        FOD_AMT_BLCKD     = :d_amt_blkd - :d_diff_amt,
															FOD_LSS_AMT_BLCKD = :d_lss_amt_blkd - :d_diff_loss,
															FOD_SLTP_MRGN = :d_comp1_mrgn,			/**** Ver 5.9 ***/	
												      FOD_SLTP_PL   = :d_comp1_pl,			  /**** Ver 5.9 ***/
						                  FOD_SLTP_SEBI_MRGN = :d_xchng_amt   /**** Ver 5.9 ***/
									 WHERE      FOD_ORDR_RFRNC    = :c_sltp_ord_rfrnc;

								if(SQLCODE != 0)
								{
									fn_errlog ( c_ServiceName, "S31385", SQLMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}

						}
						/*** Ver 3.1 Ends here ***/

					case	ORS_CAN_ORD_ACPT:
					case  ORS_IOC_CAN_ACPT: /*** Ver 2.6 ***/

  				if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  				{
						fn_userlog(c_ServiceName," Stage 6");
					}

          /****** ver 6.3 starts **/
				  l_pnd_ord= -1;

					fn_userlog(c_ServiceName,"st_orderbook.c_ordr_rfrnc:%s:c_sltp_ord_rfrnc:%s:",st_orderbook.c_ordr_rfrnc,c_sltp_ord_rfrnc);

		      EXEC SQL
 			    select count(0) 
      		INTO :l_pnd_ord
      		from FOD_FO_ORDR_DTLS
      		where
      		FOD_ORDR_STTS in ('R','Q','O','P')
					AND DECODE(FOD_FC_FLAG,'F',FOD_ORDR_RFRNC,FOD_SLTP_ORDR_RFRNC )= DECODE(:c_fc_flg,'F',:st_orderbook.c_ordr_rfrnc, :c_sltp_ord_rfrnc );

          if( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND)
          {
            fn_errlog ( c_ServiceName, "S31390", SQLMSG, ptr_st_err_msg->c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
            tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
          }

          if(DEBUG_MSG_LVL_0)
          {
            fn_userlog(c_ServiceName,"l_pnd_ord:%:ld",l_pnd_ord);
          }
         /****** ver 6.3 ends ***********/

					/*** start Ver 2.6 ***/
					/* Calculate order status */
					c_ordr_stts = '\0';
					EXEC SQL
						SELECT  distinct decode(FOD_ORDR_STTS,'J','C','D','C',FOD_ORDR_STTS)
						INTO    :c_ordr_stts
						FROM    FOD_FO_ORDR_DTLS
						WHERE		FOD_ORDR_RFRNC in ( :st_orderbook.c_ordr_rfrnc, :c_sltp_ord_rfrnc);

					if( SQLCODE != 0 && SQLCODE != TOO_MANY_ROWS_FETCHED)
					{
						fn_errlog ( c_ServiceName, "S31395", LIBMSG, ptr_st_err_msg->c_err_msg );
						fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
						tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
					}

					if (SQLCODE == TOO_MANY_ROWS_FETCHED)
					{
						c_ordr_stts = '\0';
					}

					if(DEBUG_MSG_LVL_3)
					{
						fn_userlog(c_ServiceName,"c_ordr_stts = :%c:",c_ordr_stts);
					}

					/* Calculate open quantity */
					EXEC SQL
						SELECT  NVL( SUM( DECODE( FOD_FC_FLAG, 'C', -1 * FTD_EXCTD_QTY,'T', -1 * FTD_EXCTD_QTY, FTD_EXCTD_QTY ) ), 0 ) /*** T handling in Ver 6.3 ***/
						INTO    :l_open_qty
						FROM    FOD_FO_ORDR_DTLS, FTD_FO_TRD_DTLS
						WHERE   FOD_CLM_MTCH_ACCNT    = :st_orderbook.c_cln_mtch_accnt
						AND     FOD_TRD_DT            = :st_orderbook.c_trd_dt
						AND     FOD_PRDCT_TYP         = :st_orderbook.c_prd_typ
						AND     FTD_ORDR_RFRNC        = FOD_ORDR_RFRNC
						AND     DECODE( FOD_FC_FLAG, 'F', FOD_ORDR_RFRNC, FOD_SLTP_ORDR_RFRNC ) = decode(:c_fc_flg,'F',:st_orderbook.c_ordr_rfrnc,:c_sltp_ord_rfrnc);  

					if( SQLCODE != 0 )
					{
							fn_errlog ( c_ServiceName, "S31400", SQLMSG, ptr_st_err_msg->c_err_msg );
					  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );	/*** Ver 4.4 ***/
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
					}

  				if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  				{
						fn_userlog(c_ServiceName,"l_open_qty is :%ld:", l_open_qty); 
					}

					if ( (c_ordr_stts == 'C' && l_open_qty == 0) ||  (l_pnd_ord == 0 && l_open_qty == 0) )/*** ver 6.3 l_pnd_ord adaded ***/
					{

  					if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  					{
							fn_userlog(c_ServiceName," Stage 7 ");
						}
	
						/*** Release the margin and loss amount blocked ***/
						EXEC SQL
							SELECT
										NVL(FOD_AMT_BLCKD,0),
										NVL(FOD_LSS_AMT_BLCKD,0)
							INTO  :d_amt_blkd,
										:d_lss_amt_blkd
							FROM  FOD_FO_ORDR_DTLS
							WHERE FOD_ORDR_RFRNC = decode(:c_fc_flg, 'C', :c_sltp_ord_rfrnc, 'T', :c_sltp_ord_rfrnc , :st_orderbook.c_ordr_rfrnc );/***Handling for T in  ver 6.3 ******/

							if( SQLCODE != 0 )
							{
								fn_errlog ( c_ServiceName, "S31405", LIBMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}
							
							strcpy( c_narration_id , ON_CANCELLATION_ACCEPTANCE);
							
							if(d_amt_blkd > 0)
							{
								d_balance_amt  = 0.0;

								if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"Release old margin amount blocked");
								}
								
								i_returncode = fn_upd_limits( c_ServiceName,
																						&st_pstn_actn,
																						ptr_st_err_msg,
																						c_narration_id, 
																						DEBIT_WITHOUT_LIMIT,
																						d_amt_blkd,
																						&d_balance_amt);


								if ( i_returncode != 0 )
								{
									fn_errlog ( c_ServiceName, "S31410", SQLMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}
							}
											
							if(d_lss_amt_blkd > 0)
							{
								d_balance_amt  = 0.0;

								if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"Release old loss amount");
								}
								i_returncode = fn_upd_limits( c_ServiceName,
																						&st_pstn_actn,
																						ptr_st_err_msg,
																						c_narration_id, 
																						DEBIT_WITHOUT_LIMIT,
																						d_lss_amt_blkd,
																						&d_balance_amt);


								if ( i_returncode != 0 )
								{
									fn_errlog ( c_ServiceName, "S31415", SQLMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}
							}

							if( DEBUG_MSG_LVL_3 )
							{
								fn_userlog(c_ServiceName,"Update margin & loss released");
							}

							EXEC SQL
									 UPDATE FOD_FO_ORDR_DTLS
									 SET    FOD_AMT_BLCKD			= 0,
													FOD_LSS_AMT_BLCKD = 0,
 												  FOD_SLTP_MRGN			= 0,				/*** Ver 5.9 ***/	
													FOD_SLTP_PL				= 0,				/*** Ver 5.9 ***/	 
													FOD_SLTP_SEBI_MRGN= 0					/*** Ver 5.9 ***/									
									 WHERE  FOD_ORDR_RFRNC		= decode(:c_fc_flg, 'C', :c_sltp_ord_rfrnc, 'T', :c_sltp_ord_rfrnc, :st_orderbook.c_ordr_rfrnc ); /*** T handled in Ver 6.3 ***/
							
							if ( SQLCODE != 0 )
							{
								fn_errlog ( c_ServiceName, "S31420", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}
					}
					/*** End Ver 2.6 ***/

						/*** Commented in Ver 2.6 ***
						if(c_fc_flg == 'F') *Fresh*
						{
								fn_userlog(c_ServiceName," Stage 8");

							d_amt_blkd = 0.0;
							d_lss_amt_blkd = 0.0;
							c_ordr_stts = '\0';

							EXEC SQL	
								 SELECT			NVL(FOD_AMT_BLCKD,0),
														NVL(FOD_LSS_AMT_BLCKD,0),
														FOD_ORDR_STTS,
														FOD_ORDR_FLW
								 INTO				:d_amt_blkd,
														:d_lss_amt_blkd,
														:c_ordr_stts,
														:c_ordr_flw
								 FROM				FOD_FO_ORDR_DTLS
								 WHERE			FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31425", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
							}
							
							if(DEBUG_MSG_LVL_3)
							{
								fn_userlog(c_ServiceName,"Margin Amount of the fresh order :%lf:", d_amt_blkd);
								fn_userlog(c_ServiceName,"Loss Amount of the fresh order :%lf:", d_lss_amt_blkd);
							}

							*Get the current margin percentage from the database *
							EXEC SQL
							 SELECT     FMM_SLTP_PRCNT
							 INTO       :d_fmm_sltp_prcnt
							 FROM       FMM_FO_MRGN_MSTR
							 WHERE      FMM_XCHNG_CD = :st_orderbook.c_xchng_cd
							 AND        FMM_PRDCT_TYP ='F'
							 AND        FMM_UNDRLYNG = :st_orderbook.c_undrlyng
							 AND        FMM_UNDRLYNG_BSKT_ID =(SELECT  FCB_UNDRLYNG_BSKT_ID
																							 FROM    FCB_FO_CLN_BSKT_ALLTD
																							 WHERE   FCB_CLN_LVL =(SELECT CLM_CLNT_LVL
																																		 FROM    CLM_CLNT_MSTR
																																		 WHERE   CLM_MTCH_ACCNT =:st_orderbook.c_cln_mtch_accnt));

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31430", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
							}

							** Getting the total executed value and quantity of FRESH order. **
							 EXEC SQL
								SELECT  NVL(SUM(FTD_EXCTD_QTY * FTD_EXCTD_RT),0),
												NVL(SUM(FTD_EXCTD_QTY),0)
								INTO    :d_frsh_exe_val,
												:li_tot_exec_qty
								FROM    FTD_FO_TRD_DTLS
								WHERE   FTD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

							if( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND)
							{
								fn_errlog ( c_ServiceName, "S31435", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
							}
							else if (SQLCODE == NO_DATA_FOUND)
							{
								d_frsh_exe_val = 0;
								li_tot_exec_qty = 0;
							}

							** Calculating the Weight Average Price for the executed quantity of FRESH Order. **

							d_wgt_avrg_price = 0;
							
							if (li_tot_exec_qty != 0 )
							{
								d_wgt_avrg_price = (double) (d_frsh_exe_val / li_tot_exec_qty);
							}

							
							li_lmt_rt = 0;

							EXEC SQL	
							 SELECT			NVL(FOD_LMT_RT,0)
							 INTO				:li_lmt_rt
							 FROM				FOD_FO_ORDR_DTLS
							 WHERE			FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc; 

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31440", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
							}
							
							* Calculate open quantity *
							EXEC SQL
								SELECT  NVL( SUM( DECODE( FOD_FC_FLAG, 'C', -1 * FTD_EXCTD_QTY,
																																 FTD_EXCTD_QTY ) ), 0 ),
												NVL( SUM( DECODE( FOD_FC_FLAG, 'F', FTD_EXCTD_QTY, 0 ) ), 0 )
								INTO    :l_open_qty,
												:l_frsh_xtd_qty
								FROM    FOD_FO_ORDR_DTLS, FTD_FO_TRD_DTLS
								WHERE   FOD_CLM_MTCH_ACCNT    = :st_orderbook.c_cln_mtch_accnt
								AND     FOD_TRD_DT            = :st_orderbook.c_trd_dt
								AND     FOD_PRDCT_TYP         = :st_orderbook.c_prd_typ
								AND     FTD_ORDR_RFRNC        = FOD_ORDR_RFRNC
								AND     DECODE( FOD_FC_FLAG, 'F', FOD_ORDR_RFRNC, FOD_SLTP_ORDR_RFRNC ) = :st_orderbook.c_ordr_rfrnc;

							if( SQLCODE != 0 )
							{
								fn_errlog ( c_ServiceName, "S31445", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}

							* Calculate Margin amount to be blocked *
							d_amt_to_be_blckd = ((double)(l_open_qty * d_wgt_avrg_price) * d_fmm_sltp_prcnt / 100.0);

							* Calculate Loss amount to be blocked *
							if ( c_ordr_flw == BUY )
							{
								d_lss_amt_to_be_blckd = (double) (l_open_qty * ( li_lmt_rt - d_wgt_avrg_price) );
							}
							else if ( c_ordr_flw == SELL )
							{
								d_lss_amt_to_be_blckd = (double) (l_open_qty * (d_wgt_avrg_price - li_lmt_rt) );  
							}

							* Difference margin & loss *
							d_diff_amt  = d_amt_blkd - d_amt_to_be_blckd;
							d_diff_loss = d_lss_amt_blkd + d_lss_amt_to_be_blckd;

							if(DEBUG_MSG_LVL_3)
							{
								fn_userlog(c_ServiceName,"Cover li_lmt_rt :%ld",li_lmt_rt);
								fn_userlog(c_ServiceName,"Fresh Open qty = :%ld",l_open_qty);	
								fn_userlog(c_ServiceName,"d_wgt_avrg_price = :%lf:",d_wgt_avrg_price);
								fn_userlog(c_ServiceName,"d_amt_blkd = :%lf:", d_amt_blkd);
								fn_userlog(c_ServiceName,"d_lss_amt_blkd = :%lf:", d_lss_amt_blkd);
								fn_userlog(c_ServiceName,"d_amt_to_be_blckd = :%lf:", d_amt_to_be_blckd);
								fn_userlog(c_ServiceName,"d_lss_amt_to_be_blckd = :%lf:", d_lss_amt_to_be_blckd);
								fn_userlog(c_ServiceName,"d_diff_amt = :%lf:",d_diff_amt);
								fn_userlog(c_ServiceName,"d_diff_loss = :%lf:",d_diff_loss);
							}

							* Block Release the margin amount *
							strcpy( c_narration_id , ON_CANCELLATION_ACCEPTANCE);
							if(d_diff_amt != 0)
							{
									d_balance_amt  = 0.0;

									if(DEBUG_MSG_LVL_3)
									{
										fn_userlog(c_ServiceName,"Block the margin amount");
									}

									i_returncode = fn_upd_limits( c_ServiceName,
																								&st_pstn_actn,
																								ptr_st_err_msg,
																								c_narration_id,
																								DEBIT_TILL_LIMIT,
																								d_diff_amt,
																								&d_balance_amt);


									if ( i_returncode != 0 )
									{
										fn_errlog ( c_ServiceName, "S31450", LIBMSG, ptr_st_err_msg->c_err_msg );
										fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
										tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
									}

									d_amt_to_be_blckd = d_amt_to_be_blckd + d_balance_amt ;
								}

							* Block Release the loss amount *
							strcpy( c_narration_id , ON_CANCELLATION_ACCEPTANCE);
								
							if( d_lss_amt_to_be_blckd <= 0 )
							{
								if(d_diff_loss != 0)
								{
									d_balance_amt  = 0.0;

									if(DEBUG_MSG_LVL_3)
									{
										fn_userlog(c_ServiceName,"Block the loss amount");
									}

									i_returncode = fn_upd_limits( c_ServiceName,
																								&st_pstn_actn,
																								ptr_st_err_msg,
																								c_narration_id,
																								DEBIT_TILL_LIMIT,
																								d_diff_loss,
																								&d_balance_amt);


									if ( i_returncode != 0 )
									{
										fn_errlog ( c_ServiceName, "S31455", LIBMSG, ptr_st_err_msg->c_err_msg );
										fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
										tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
									}

									 d_lss_amt_to_be_blckd = d_lss_amt_to_be_blckd - d_balance_amt ;
								}
							}
							else if(d_lss_amt_to_be_blckd > 0)
							{
								if(d_lss_amt_blkd != 0)
								{
									d_balance_amt  = 0.0;

									if(DEBUG_MSG_LVL_3)
									{
										fn_userlog(c_ServiceName,"Release old loss amount in case or profit");
									}
								
									i_returncode = fn_upd_limits( c_ServiceName,
																								&st_pstn_actn,
																								ptr_st_err_msg,
																								c_narration_id,
																								DEBIT_TILL_LIMIT,
																								d_lss_amt_blkd,
																								&d_balance_amt);


									if ( i_returncode != 0 )
									{
										fn_errlog ( c_ServiceName, "S31460", LIBMSG, ptr_st_err_msg->c_err_msg );
										fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
										tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
									}
								}
								 d_lss_amt_to_be_blckd = 0;
							}

							EXEC SQL
								 UPDATE     FOD_FO_ORDR_DTLS
								 SET        FOD_AMT_BLCKD = :d_amt_to_be_blckd,
														FOD_LSS_AMT_BLCKD = abs(:d_lss_amt_to_be_blckd) 
								 WHERE      FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

								if(SQLCODE != 0)
								{
									fn_errlog ( c_ServiceName, "S31465", LIBMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}
						}
						*** Comment end in Ver 2.6 ***/
					 if ( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_ACPT && strcmp(c_sys_msg,"X") != 0 ) /*** Ver 2.6 ***/ /*** Ver 6.3 skip user event for sqroff at mkt ***/
					 {
						if(c_fc_flg == 'C'  || c_fc_flg == 'F' || c_fc_flg == 'E' || c_fc_flg == 'T') /*Cover*/ /*** Ver 2.6 Fresh flag added ***/ /*** T handled in Ver 6.3 ***/
						{
  						if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  						{	
								fn_userlog(c_ServiceName,"Stage 9");
							}

							/***  Ver 2.3 starts ...Commit cancellation acceptance...and place squareoff order in another transaction ***/
							if ( fn_committran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ) == -1 )	 
							{
								fn_errlog ( c_ServiceName, "S31470", LIBMSG, c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
							}
							/**Ver 6.6 call to func **/
							i_returncode = fn_upd_nrmlmrgn_reporting(st_orderbook.c_cln_mtch_accnt,
                                       st_orderbook.c_xchng_cd,
                                       c_ServiceName,
                                       ptr_st_err_msg->c_err_msg
                                      );
              if( i_returncode != 0 )
             {
               fn_errlog(c_ServiceName, "S31475", LIBMSG, c_err_msg );
               tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
             }
						/**Ver 6.6 **/
						
							i_commit_flg = 1;
  						if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  						{
								fn_userlog(c_ServiceName,"Transaction commited...i_commit_flg = :%d:",i_commit_flg);
							}
							/*** Ver 2.3 ends ***/

							EXEC SQL	
								 SELECT			FOD_ORDR_STTS
								 INTO				:c_ordr_stts
								 FROM				FOD_FO_ORDR_DTLS
								 WHERE			FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31480", SQLMSG, ptr_st_err_msg->c_err_msg );
								/*** fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );  *** Commented in Ver 2.3 ***/
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}

						/* Calculate open quantity */
						EXEC SQL
							SELECT  NVL( SUM( DECODE( FOD_FC_FLAG, 'C', -1 * FTD_EXCTD_QTY,
																										 'T', -1 * FTD_EXCTD_QTY,	/*** ver 6.3 ***/
																															 FTD_EXCTD_QTY ) ), 0 ),
											NVL( SUM( DECODE( FOD_FC_FLAG, 'F', FTD_EXCTD_QTY, 0 ) ), 0 )
							INTO    :l_open_qty,
											:l_frsh_xtd_qty
							FROM    FOD_FO_ORDR_DTLS, FTD_FO_TRD_DTLS
							WHERE   FOD_CLM_MTCH_ACCNT    = :st_orderbook.c_cln_mtch_accnt
							AND     FOD_TRD_DT            = :st_orderbook.c_trd_dt
							AND     FOD_PRDCT_TYP         = :st_orderbook.c_prd_typ
							AND     FTD_ORDR_RFRNC        = FOD_ORDR_RFRNC
							AND     DECODE( FOD_FC_FLAG, 'F', FOD_ORDR_RFRNC, FOD_SLTP_ORDR_RFRNC ) = decode(:c_fc_flg,'F',:st_orderbook.c_ordr_rfrnc,'T', :c_sltp_ord_rfrnc,:c_sltp_ord_rfrnc); /*** Ver 2.6 decode added ***/ /*** T handled in Ver 6.3 ***/

						if( SQLCODE != 0 )
						{
								fn_errlog ( c_ServiceName, "S31485", SQLMSG, ptr_st_err_msg->c_err_msg );
								/*** fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );		 *** Commented in Ver 2.3 ***/
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}

  					if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  					{
							fn_userlog(c_ServiceName,"l_open_qty is :%ld:", l_open_qty); /*** Ver 2.6 ***/
						}

						if (l_open_qty == 0  ) /*** if loop added in Ver 2.6 ***/
						{
							break;
						}
						fn_userlog(c_ServiceName,"l_open_qty is :%ld:", l_open_qty);	
						fn_userlog( c_ServiceName, "CR-ISEC14-144377 Index/Stock :%c:",  st_orderbook.c_ctgry_indstk );   /*** Added in Ver 6.1 ***/
	
						EXEC SQL	
						 SELECT /*** MTM_SLTP_REQ Commented in Ver 6.1 ***/
										/*** DECODE added in Ver 6.1 ****/
										DECODE ( :st_orderbook.c_ctgry_indstk,'I',MTM_SLTP_REQ,'S',MTM_SLTP_REQ_S,'N' )
						 INTO		:c_mtm_sltp_req
						 FROM		MTM_APPLN_STTS
						 WHERE  mtm_xchng_cd = :st_orderbook.c_xchng_cd
						 AND    mtm_pipe_id LIKE :st_orderbook.c_pipe_id;
						
						if(SQLCODE != 0)
						{
							fn_errlog ( c_ServiceName, "S31490", SQLMSG, ptr_st_err_msg->c_err_msg );
							/*** fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );		 *** Commented in Ver 2.3 ***/
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}
						
						if(DEBUG_MSG_LVL_3)	/*** Ver 2.3 ***/
						{
							fn_userlog(c_ServiceName,"Order status of fresh order :%c:", c_ordr_stts);
							fn_userlog(c_ServiceName,"Request type :%c:", c_mtm_sltp_req);
						}


           /************ ver 6.3 starts ***************/
    if( c_fc_flg == 'C' || c_fc_flg == 'T')
    {
    if (c_fc_flg == 'C')
    {
			fn_userlog(c_ServiceName,"c_fc_flg is == :%c:", c_fc_flg);
      EXEC SQL
        select FOD_ORDR_RFRNC
        INTO
        :c_can_ordr_ref
        from FOD_FO_ORDR_DTLS
        where
        FOD_FC_FLAG= 'T'
        and FOD_ORDR_STTS in ('R','Q','P','O')
        and DECODE( FOD_FC_FLAG, 'F', FOD_ORDR_RFRNC, FOD_SLTP_ORDR_RFRNC ) = :c_sltp_ord_rfrnc;
    }
    else if (c_fc_flg == 'T')
    {
      EXEC SQL
        select FOD_ORDR_RFRNC
        INTO
        :c_can_ordr_ref
        from FOD_FO_ORDR_DTLS
        where
        FOD_FC_FLAG= 'C'
        and FOD_ORDR_STTS in ('R','Q','P','O')
        and DECODE( FOD_FC_FLAG, 'F', FOD_ORDR_RFRNC, FOD_SLTP_ORDR_RFRNC ) = :c_sltp_ord_rfrnc;
    }


    if(SQLCODE !=0 && SQLCODE !=NO_DATA_FOUND)
    {
      fn_errlog ( c_ServiceName, "S31495", SQLMSG, ptr_st_err_msg->c_err_msg );
      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
      tpreturn(TPFAIL, ERR_BFR, (char *)&ptr_st_err_msg, 0, 0 );
    }

    if (SQLCODE != NO_DATA_FOUND)
    {
      fn_userlog( c_ServiceName, "Place Cancellation of :%s:",c_can_ordr_ref);
      strcpy(c_remarks,"System placed Cancellation");
      /*** strcpy(c_sys_msg1,"SYS_EVENT"); ***/
			strcpy(c_sys_msg1,c_sys_msg);
			if (strcmp(c_sys_msg,"SYS_EVENT")==0)
			{
      	c_nkd_blkd_flg = 'Z';
			}
			fn_userlog(c_ServiceName,"c_can_ordr_ref == :%s:",c_can_ordr_ref);

      i_returncode = fn_acall_svc_fml ( c_ServiceName,
                                       ptr_st_err_msg->c_err_msg,
                                       "SFO_CANCEL_FUT",
                                       0,
                                       11,
                                       FFO_USR_ID, (char *)st_pstn_actn.c_user_id,
                                       FFO_SSSN_ID, (char *)&st_pstn_actn.l_session_id,
                                       FFO_XCHNG_CD,(char *)st_pstn_actn.c_xchng_cd,
                                       FFO_EBA_MTCH_ACT_NO,(char *)st_pstn_actn.c_cln_mtch_accnt,
                                       FFO_ORDR_RFRNC, (char *)c_can_ordr_ref,
                                       FFO_XCHNG_RMRKS, (char *)c_remarks,
                                       FFO_CHANNEL,(char *)st_pstn_actn.c_channel,
                                       FFO_ALIAS,(char *)st_pstn_actn.c_alias,
                                       FFO_PIPE_ID, (char *)st_orderbook.c_pipe_id,
                                       FFO_MKT_TYP,(char *)&c_nkd_blkd_flg,  
                                       FFO_SYS_MSG, (char *)c_sys_msg1);
      if ( i_returncode != SUCC_BFR )
      {
        fn_errlog ( c_ServiceName, "S31500", LIBMSG, ptr_st_err_msg->c_err_msg );
        fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
        tpreturn(TPFAIL, ERR_BFR, (char *)&ptr_st_err_msg, 0, 0 );
      }
    }
		} /** End Of Outer if **/

              l_tot_pend = 0;

              EXEC SQL
                select NVL(SUM(decode(FOD_ORDR_STTS,'E',0,'B',0,'D',0,'C',0,'X',0,1)),0)
                INTO
                :l_tot_pend
                from FOD_FO_ORDR_DTLS
                where
                DECODE( FOD_FC_FLAG, 'F', FOD_ORDR_RFRNC, FOD_SLTP_ORDR_RFRNC ) = decode(:c_fc_flg, 'F',:st_orderbook.c_ordr_rfrnc,:c_can_ordr_ref);

              if(SQLCODE !=0 )
              {
                fn_errlog ( c_ServiceName, "S31505", SQLMSG, ptr_st_err_msg->c_err_msg ); 
              /* tpfree((char *)ptr_fml_Ibuf); */  
                tpreturn(TPFAIL, ERR_BFR, (char *)&ptr_st_err_msg, 0, 0 );
              }

              fn_userlog( c_ServiceName,"Pending order present :%ld:", l_tot_pend);	

		/*** ver 6.3 End ***/
    if (c_mtm_sltp_req == 'A' || strcmp(c_sys_msg,"MY_EOS")==0 || strcmp(c_sys_msg,"SYS_EVENT")==0)	/* Request type is 'B' when EOS is run for Cancellation & squareoff both */ /*** Ver 4.1 ***/	/*** ver 6.3  SYS_EVENT Added ***/
		{
  						if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  						{
								fn_userlog(c_ServiceName," Stage 10");
							}
							fn_userlog(c_ServiceName,"l_tot_pend is :%ld:",l_tot_pend);
			
							if ((c_ordr_stts == EXECUTED || /*** c_ordr_stts == PARTIALLY_EXECUTED || ***/ 
										c_ordr_stts == PARTIALLY_EXECUTED_AND_EXPIRED || c_ordr_stts == PARTIALLY_EXECUTED_AND_CANCELLED 
								||  c_ordr_stts == CANCELLED || c_ordr_stts == REJECTED) && l_tot_pend == 0 )	/* Check order status of fresh order */ 
								/*** CANCELLED,REJECTED condition added in Ver 2.6  and PARTIALLY_EXECUTED condition removed ***/	/*** l_tot_pend added in ver 6.3 ***/
							{
								fn_userlog(c_ServiceName,"Placing square off order");	
								/* Place square off order */
								strcpy( c_svc_nm, "SFO_FUT_PLC_ORD");
								c_slm_flg=MARKET;
								c_ord_typ=GOOD_TILL_TODAY;
								li_dsclsd_qty = 0;
								li_stp_lss_tgr_prc = 0;
								li_lmt_rt = 0;
								/**strcpy (c_sys_msg,"EOS"); commented in Ver 4.1 **/
								 /*** Ver 4.1 starts ***/
								if(DEBUG_MSG_LVL_3)  
  								{
									fn_userlog(c_ServiceName,"c_sys_msg *************:%s:",c_sys_msg);
								}
								 /*** Ver 4.1 ends ***/
								if(strcmp(c_sys_msg,"MY_EOS")==0)   /*** If condition added in ver 4.1 ***/
								{
									c_spl_flg=CLIENT_SQROFF;
								}
								else
								{
									c_spl_flg = SYSTEM_SQUAREOFF;
								}
								sql_fum_freeze_qty = 0;

								  /*** EXEC SQL
									 SELECT fum_freeze_qty
									 INTO   :sql_fum_freeze_qty
									 FROM   fum_fo_undrlyng_mstr
									 WHERE  fum_xchng_cd  = :st_pstn_actn.c_xchng_cd
									 AND    fum_prdct_typ = 'F'
									 AND    fum_undrlyng  = :st_pstn_actn.c_undrlyng; *** Commented in Ver 4.5 ***/

  							/*** Ver 4.5 starts ***/
                if( DEBUG_MSG_LVL_3)
                {
                  fn_userlog( c_ServiceName,"st_pstn_actn.c_xchng_cd is :%s:", st_pstn_actn.c_xchng_cd);
                  fn_userlog( c_ServiceName,"st_pstn_actn.c_undrlyng is :%s:", st_pstn_actn.c_undrlyng);
                  fn_userlog( c_ServiceName,"st_pstn_actn.c_expry_dt is :%s:", st_pstn_actn.c_expry_dt);
                }

   							EXEC SQL
       						SELECT fum_freeze_qty,trunc(fum_freeze_qty/ftq_min_lot_qty) * ftq_min_lot_qty
       						INTO   :sql_fum_freeze_qty,
              					 :l_max_allowed_qty
       						FROM   ftq_fo_trd_qt,fum_fo_undrlyng_mstr
       						WHERE  fum_xchng_cd  =  ftq_xchng_cd
       						AND    fum_prdct_typ =  ftq_prdct_typ
       						AND    fum_undrlyng  =  ftq_undrlyng
       						AND    ftq_xchng_cd  =  :st_pstn_actn.c_xchng_cd
       						AND    ftq_prdct_typ =  'F' 
       						AND    ftq_undrlyng  =  :st_pstn_actn.c_undrlyng
       						AND    ftq_expry_dt  =  :st_pstn_actn.c_expry_dt;
  							/*** Ver 4.5 ends ***/


								if ( SQLCODE != 0 )
								{
									fn_errlog ( c_ServiceName, "S31510", SQLMSG, ptr_st_err_msg->c_err_msg );
									/*** fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );  *** Commented in Ver 2.3 ***/
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}

								/*** Ver 4.5 Starts ***/
                if( DEBUG_MSG_LVL_3)
                {
  								fn_userlog( c_ServiceName,"l_max_allowed_qty is :%ld:", l_max_allowed_qty ); 
								}
								/*** Ver 4.5 Ends ***/

							if ( strcmp(c_sys_msg,"SYS_EVENT")!=0 ) /*** Ver 6.3 ***/
							{
								if( l_open_qty  > sql_fum_freeze_qty && sql_fum_freeze_qty != 0 )
								{
									/*** l_open_qty = sql_fum_freeze_qty; Commented in Ver 4.5 ***/
								 	l_open_qty = l_max_allowed_qty;  /*** Added in Ver 4.5 ***/
								}
							}

								/*** starts Ver 2.6 ***/
								
								if( DEBUG_MSG_LVL_3)
								{
									fn_userlog( c_ServiceName," N c_fc_flg = :%c:",c_fc_flg);
									fn_userlog( c_ServiceName," N l_open_qty Is :%ld:",l_open_qty);
									fn_userlog( c_ServiceName," N st_pstn_actn.c_trnsctn_flw Is :%c:",st_pstn_actn.c_trnsctn_flw);
								}
                    
                 /*** ver 6.3 starts ***/
                 fn_userlog( c_ServiceName," N c_spl_flg :%c:",c_spl_flg);
                if( strcmp(c_sys_msg,"SYS_EVENT")==0 )
                {
                  c_nkd_blkd_flg = 'Z';
                } /** ver 6.3 ends ***/ 
  
								if ( c_fc_flg == 'C' || c_fc_flg == 'T') /*** T handled in Ver 6.3 ***/
								{	
									strcpy(c_cover_ref,st_orderbook.c_ordr_rfrnc);
									if (l_open_qty < 0 ) 
									{
										st_pstn_actn.c_trnsctn_flw = (st_pstn_actn.c_trnsctn_flw == 'B') ? 'S' : 'B'; 
									}
								}
								else if ( c_fc_flg == 'E')
								{	
									strcpy(c_cover_ref,st_orderbook.c_ordr_rfrnc);
								}
								else 
								{
									strcpy(c_cover_ref,c_sltp_ord_rfrnc);
									if (l_open_qty > 0 ) 
									{
										st_pstn_actn.c_trnsctn_flw = (st_pstn_actn.c_trnsctn_flw == 'B') ? 'S' : 'B'; 
									}
								}

  							if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  							{
									fn_userlog( c_ServiceName,"NN st_pstn_actn.c_trnsctn_flw Is :%c:",st_pstn_actn.c_trnsctn_flw);
								}	
								if (l_open_qty < 0) 
								{
								  /**	fn_userlog( c_ServiceName,"Sys msg set to EXCPT_EOS"); commented in Ver 4.1 **/
									/** strcpy(c_sys_msg,"EXCPT_EOS");  
									l_open_qty = l_open_qty * -1; Commented in Ver 4.1 **/
									fn_userlog( c_ServiceName,"Open Qty :%ld:",l_open_qty); /** Added in Ver 4.1 ***/
								}

								/*** Ver 2.6 ends ****/

								i_returncode = fn_acall_svc_fml ( c_ServiceName,		/*** Ver 2.3 changed from fn_call_svc_fml to fn_acall_svc_fml ***/
																								 c_err_msg,
																								 "SFO_ORD_ROUT",
																								 0,
																								 25, /** ver 6.3 24 to 25  ***/
																								/*** 2,		*** Commented in Ver 2.3 ***/
																								/*** FFO_ORDR_RFRNC, (char *)st_orderbook.c_ordr_rfrnc, *** Commented in Ver 2.6 ***/
																								FFO_ORDR_RFRNC, (char *)c_cover_ref, /*** Added in Ver 2.6 ***/
																								FFO_SVC_NAME, (char *)c_svc_nm,
																								FFO_USR_ID, (char *)st_pstn_actn.c_user_id,
																								FFO_SSSN_ID, (char *)&st_pstn_actn.l_session_id,
																								FFO_XCHNG_CD, (char *)st_pstn_actn.c_xchng_cd,
																								FFO_EBA_MTCH_ACT_NO, (char *)st_pstn_actn.c_cln_mtch_accnt,
																								FFO_PRDCT_TYP, (char *)&st_pstn_actn.c_prd_typ,
																								FFO_UNDRLYNG, (char *)st_pstn_actn.c_undrlyng,
																								FFO_EXPRY_DT, (char *)st_pstn_actn.c_expry_dt,
																								FFO_EXER_TYP, (char *)&st_pstn_actn.c_exrc_typ,
																								FFO_OPT_TYP , (char *)&st_pstn_actn.c_opt_typ,
																								FFO_STRK_PRC, (char *)&st_pstn_actn.l_strike_prc,
																								FFO_CTGRY_INDSTK , (char *)&st_pstn_actn.c_ctgry_indstk,
																								FFO_ORDR_FLW, (char *)&st_pstn_actn.c_trnsctn_flw,
																								FFO_LMT_MKT_SL_FLG ,(char *)&c_slm_flg,
																								FFO_ORD_TYP ,(char *)&c_ord_typ,
																								FFO_DSCLSD_QTY , &li_dsclsd_qty,
																								FFO_STP_LSS_TGR , &li_stp_lss_tgr_prc,
																								FFO_ORD_TOT_QTY, (char *)&l_open_qty,
																								FFO_LMT_RT, &li_lmt_rt,
																								FFO_SPL_FLG ,(char *)&c_spl_flg,
																								FFO_CHANNEL, (char *)st_pstn_actn.c_channel, 
																								FFO_ALIAS, (char *)st_pstn_actn.c_alias,  
																								FFO_SYS_MSG, (char *)c_sys_msg,
                                                FFO_NKD_BLKD_FLG,(char *)&c_nkd_blkd_flg);  /*** ver 6.3 **/
																								/*** FFO_ORDR_RFRNC, (char *)st_orderbook.c_ordr_rfrnc, NULL,
																								FFO_PIPE_ID, (char *)st_orderbook.c_pipe_id, NULL);		*** Commented in Ver 2.3 ***/

							if ( i_returncode != SUCC_BFR )
							{
								fn_errlog( c_ServiceName, "S31515", LIBMSG, c_err_msg  );
								/*** fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );		 *** Commented in Ver 2.3 ***/
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );  /*** Ver 2.3 Changed from c_err_msg to ptr_st_err_msg ***/
							}
						}
					}
					}
				 } /*** if loop ended in Ver 2.6 ***/
					break;

				case ORS_CAN_ORD_RJCT:
					break;

				case ORS_ORD_EXP:

          if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
          {
					 fn_userlog(c_ServiceName ,"c_fc_flg is is :%c:",c_fc_flg );
					}

					if(c_fc_flg == 'F' || c_fc_flg == 'C' || c_fc_flg == 'T') /*Fresh*/  /** Cover added in Ver 3.1 **/ /*** Ver 6.3 T handled ***/
					{
            if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
            {
							fn_userlog(c_ServiceName,"Stage 11");
							fn_userlog(c_ServiceName," c_fc_flg is :%c:",c_fc_flg);
						}
						d_amt_blkd = 0.0;
						d_lss_amt_blkd = 0.0;
						c_ordr_stts = '\0';

						EXEC SQL	
							 SELECT			NVL(FOD_AMT_BLCKD,0),
													NVL(FOD_LSS_AMT_BLCKD,0),
													FOD_ORDR_STTS,
													FOD_ORDR_FLW,
													FOD_ORDR_RFRNC,
													FOD_SLTP_ORDR_RFRNC,
													FOD_LMT_RT
							 INTO				:d_amt_blkd,
													:d_lss_amt_blkd,
													:c_ordr_stts,
													:c_ordr_flw,
													:st_orderbook.c_ordr_rfrnc,
													:c_sltp_ord_rfrnc,
													:li_lmt_rt
							 FROM				FOD_FO_ORDR_DTLS
							 WHERE			FOD_ORDR_RFRNC = decode(:c_fc_flg,'F',:st_orderbook.c_ordr_rfrnc,:c_sltp_ord_rfrnc); /*added in Ver 3.1 **/

						if(SQLCODE != 0)
						{
							fn_errlog ( c_ServiceName, "S31520", SQLMSG, ptr_st_err_msg->c_err_msg );
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}
						
						if(DEBUG_MSG_LVL_3)
						{
							fn_userlog(c_ServiceName,"Margin Amount of the fresh order :%lf:", d_amt_blkd);
							fn_userlog(c_ServiceName,"Loss Amount of the fresh order :%lf:", d_lss_amt_blkd);
						}

						/*Get the current margin percentage from the database *   Commented in Ver 3.1 **

 						 EXEC SQL
						 SELECT     FMM_SLTP_PRCNT
						 INTO       :d_fmm_sltp_prcnt
						 FROM       FMM_FO_MRGN_MSTR
						 WHERE      FMM_XCHNG_CD = :st_orderbook.c_xchng_cd
						 AND        FMM_PRDCT_TYP ='F'
						 AND        FMM_UNDRLYNG = :st_orderbook.c_undrlyng
						 AND        FMM_UNDRLYNG_BSKT_ID =(SELECT  FCB_UNDRLYNG_BSKT_ID
																						 FROM    FCB_FO_CLN_BSKT_ALLTD
																						 WHERE   FCB_CLN_LVL =(SELECT CLM_CLNT_LVL
																																	 FROM    CLM_CLNT_MSTR
																																	 WHERE   CLM_MTCH_ACCNT =:st_orderbook.c_cln_mtch_accnt));

						if(SQLCODE != 0)
						{
							fn_errlog ( c_ServiceName, "S31525", SQLMSG, ptr_st_err_msg->c_err_msg );
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}

						** Commented in Ver 3.1 **

						** Getting the total executed value and quantity of FRESH order. ** 
						 EXEC SQL
							SELECT  NVL(SUM(FTD_EXCTD_QTY * FTD_EXCTD_RT),0),
											NVL(SUM(FTD_EXCTD_QTY),0)
							INTO    :d_frsh_exe_val,
											:li_tot_exec_qty
							FROM    FTD_FO_TRD_DTLS
							WHERE   FTD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

						if( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND)
						{
							fn_errlog ( c_ServiceName, "S31530", SQLMSG, ptr_st_err_msg->c_err_msg );
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}
						else if (SQLCODE == NO_DATA_FOUND)
						{
							d_frsh_exe_val = 0;
							li_tot_exec_qty = 0;
						}

						** Calculating the Weight Average Price for the executed quantity of FRESH Order. **  Commented in ver 3.1 **

						d_wgt_avrg_price = 0;
						
						if (li_tot_exec_qty != 0 )
						{
							d_wgt_avrg_price = (double) (d_frsh_exe_val / li_tot_exec_qty);
						}

	
						*** Comment ends for Ver 3.1 **/
						
						/** Added in Ver 3.1 Starts **/
							
						long int li_sltp_lmt_rt = 0;

						EXEC SQL	
						SELECT			NVL(FOD_LMT_RT,0)
						INTO				:li_sltp_lmt_rt
						FROM				FOD_FO_ORDR_DTLS
						WHERE				FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

						if(SQLCODE != 0)
						{
							fn_errlog ( c_ServiceName, "S31535", SQLMSG, ptr_st_err_msg->c_err_msg );
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}
						
						 /** Added in Ver 3.1 Ends **/	

						/* Calculate open quantity *      ** Commented in Ver 3.1 ** 

						EXEC SQL
							SELECT  NVL( SUM( DECODE( FOD_FC_FLAG, 'C', -1 * FTD_EXCTD_QTY,
																															 FTD_EXCTD_QTY ) ), 0 ),
											NVL( SUM( DECODE( FOD_FC_FLAG, 'F', FTD_EXCTD_QTY, 0 ) ), 0 )
							INTO    :l_open_qty,
											:l_frsh_xtd_qty
							FROM    FOD_FO_ORDR_DTLS, FTD_FO_TRD_DTLS
							WHERE   FOD_CLM_MTCH_ACCNT    = :st_orderbook.c_cln_mtch_accnt
							AND     FOD_TRD_DT            = :st_orderbook.c_trd_dt
							AND     FOD_PRDCT_TYP         = :st_orderbook.c_prd_typ
							AND     FTD_ORDR_RFRNC        = FOD_ORDR_RFRNC
							AND     DECODE( FOD_FC_FLAG, 'F', FOD_ORDR_RFRNC, FOD_SLTP_ORDR_RFRNC ) = :st_orderbook.c_ordr_rfrnc;

						if( SQLCODE != 0 )
						{
							fn_errlog ( c_ServiceName, "S31540", SQLMSG, ptr_st_err_msg->c_err_msg );
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}
			
						* Calculate Margin amount to be blocked *    ** Commented in Ver 3.1 **

						d_amt_to_be_blckd = ((double)(l_open_qty * d_wgt_avrg_price) * d_fmm_sltp_prcnt / 100.0);

						* Calculate Loss amount to be blocked *
						if ( c_ordr_flw == BUY )
						{
							d_lss_amt_to_be_blckd = (double) (l_open_qty * ( li_lmt_rt - d_wgt_avrg_price) );
						}
						else if ( c_ordr_flw == SELL )
						{
							d_lss_amt_to_be_blckd = (double) (l_open_qty * (d_wgt_avrg_price - li_lmt_rt) );  
						}
					
						** Comment ends Ver 3.1 ****/


							 /** Added in Ver 3.1 Starts **/

						 i_returncode = fn_cal_mrgn_sltp ( c_ServiceName,	
																							 st_orderbook.c_ordr_rfrnc,
																							 li_lmt_rt,
																							 li_sltp_lmt_rt,
																							 &d_amt_to_be_blckd, 
																							 &d_lss_amt_to_be_blckd,
																							 c_ref_rmrks,  /*** Ver 5.9 ***/
																							&d_comp1_mrgn,/*** Ver 5.9 ***/
																							&d_comp1_pl,  /*** Ver 5.9 ***/
																							&d_xchng_amt, /*** Ver 5.9 ***/
																							 c_err_msg 
																						 );
              if(DEBUG_MSG_LVL_0)   /*** Ver 3.7 ***/
              {
						 		fn_userlog(c_ServiceName," d_amt_to_be_blckd is :%lf:",d_amt_to_be_blckd);
								fn_userlog(c_ServiceName," d_lss_amt_to_be_blckd is :%lf:",d_lss_amt_to_be_blckd);
							}

							if ( i_returncode != 0 )
              {
                 fn_errlog ( c_ServiceName, "S31545", LIBMSG, ptr_st_err_msg->c_err_msg );
                 fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                 tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }


						d_lss_amt_to_be_blckd = d_lss_amt_to_be_blckd  * -1;

						 /** Added in Ver 3.1 Ends **/
	
				

						/* Difference margin & loss */
						d_diff_amt  = d_amt_blkd - d_amt_to_be_blckd;
						d_diff_loss = d_lss_amt_blkd + d_lss_amt_to_be_blckd ;


            if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
            {	
						 	fn_userlog(c_ServiceName," d_diff_amt is :%lf:",d_diff_amt);
						 	fn_userlog(c_ServiceName," d_diff_loss is :%lf:",d_diff_loss);
						}		
				
	
	
						if(DEBUG_MSG_LVL_3)
						{
							fn_userlog(c_ServiceName,"Cover li_lmt_rt :%ld",li_lmt_rt);
							fn_userlog(c_ServiceName,"Fresh Open qty = :%ld",l_open_qty);	
							fn_userlog(c_ServiceName,"d_wgt_avrg_price = :%lf:",d_wgt_avrg_price);
							fn_userlog(c_ServiceName,"d_amt_blkd = :%lf:", d_amt_blkd);
							fn_userlog(c_ServiceName,"d_lss_amt_blkd = :%lf:", d_lss_amt_blkd);
							fn_userlog(c_ServiceName,"d_amt_to_be_blckd = :%lf:", d_amt_to_be_blckd);
							fn_userlog(c_ServiceName,"d_lss_amt_to_be_blckd = :%lf:", d_lss_amt_to_be_blckd);
							fn_userlog(c_ServiceName,"d_diff_amt = :%lf:",d_diff_amt);
							fn_userlog(c_ServiceName,"d_diff_loss = :%lf:",d_diff_loss);
						}

						/* Block Release the margin amount */
						strcpy( c_narration_id , ON_EXPIRY);
						if(d_diff_amt != 0)
						{
								d_balance_amt  = 0.0;

								if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"Block the margin amount");
								}

								i_returncode = fn_upd_limits( c_ServiceName,
																							&st_pstn_actn,
																							ptr_st_err_msg,
																							c_narration_id,
																							DEBIT_TILL_LIMIT,
																							d_diff_amt,
																							&d_balance_amt);


								if ( i_returncode != 0 )
								{
									fn_errlog ( c_ServiceName, "S31550", LIBMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}

								d_amt_to_be_blckd = d_amt_to_be_blckd + d_balance_amt ;
							}


						/*** Ver 5.9 started ***/
						MEMSET(c_tmp_rmrks);    
						strcpy ( c_tmp_rmrks, st_pstn_actn.c_ref_rmrks ); 
						sprintf(st_pstn_actn.c_ref_rmrks,"%s%s",c_ref_rmrks,c_tmp_rmrks);  
						/*** Ver 5.9 ends ***/

						/* Block Release the loss amount */
						strcpy( c_narration_id , ON_EXPIRY);
							
						if( d_lss_amt_to_be_blckd <= 0 )
						{
							if(d_diff_loss != 0)
							{
								d_balance_amt  = 0.0;

								if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"Block the loss amount");
								}

								i_returncode = fn_upd_limits( c_ServiceName,
																							&st_pstn_actn,
																							ptr_st_err_msg,
																							c_narration_id,
																							DEBIT_TILL_LIMIT,
																							d_diff_loss,
																							&d_balance_amt);


								if ( i_returncode != 0 )
								{
									fn_errlog ( c_ServiceName, "S31555", LIBMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}

								 d_lss_amt_to_be_blckd = d_lss_amt_to_be_blckd - d_balance_amt ;
							}
						}
						else if(d_lss_amt_to_be_blckd > 0)
						{
							if(d_lss_amt_blkd != 0)
							{
								d_balance_amt  = 0.0;

								if(DEBUG_MSG_LVL_3)
								{
									fn_userlog(c_ServiceName,"Release old loss amount in case or profit");
								}
							
								i_returncode = fn_upd_limits( c_ServiceName,
																							&st_pstn_actn,
																							ptr_st_err_msg,
																							c_narration_id,
																							DEBIT_TILL_LIMIT,
																							d_lss_amt_blkd,
																							&d_balance_amt);


								if ( i_returncode != 0 )
								{
									fn_errlog ( c_ServiceName, "S31560", LIBMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}
							}
							 d_lss_amt_to_be_blckd = 0;
						}
						strcpy ( st_pstn_actn.c_ref_rmrks, c_tmp_rmrks);    /**** Ver 5.9 ***/
						MEMSET(c_tmp_rmrks);																/**** Ver 5.9 ***/

						EXEC SQL
							 UPDATE     FOD_FO_ORDR_DTLS
							 SET        FOD_AMT_BLCKD = :d_amt_to_be_blckd,
													FOD_LSS_AMT_BLCKD = abs(:d_lss_amt_to_be_blckd),
													FOD_SLTP_MRGN = :d_comp1_mrgn,			/**** Ver 5.9 ***/	
													FOD_SLTP_PL   = :d_comp1_pl,			  /**** Ver 5.9 ***/
													FOD_SLTP_SEBI_MRGN = :d_xchng_amt   /**** Ver 5.9 ***/
							 WHERE      FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31565", LIBMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}
					}
					break;
			}	
 }
 /*** Ver 2.1 ends here ***/


	if ( i_commit_flg == 0 ) /*** Ver 2.3 ***/
	{
		if ( fn_committran( c_ServiceName, i_trnsctn, 
																			 ptr_st_err_msg->c_err_msg ) == -1 )
		{
			fn_errlog ( c_ServiceName, "S31570", LIBMSG, c_err_msg );
	  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );	/*** Ver 4.4 ***/
			tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
		}

		/***Ver 6.6 ***/
    i_returncode = fn_upd_nrmlmrgn_reporting(st_orderbook.c_cln_mtch_accnt,
                                       st_orderbook.c_xchng_cd,
                                       c_ServiceName,
                                       ptr_st_err_msg->c_err_msg
                                      );
    if( i_returncode != 0 )
    {
     fn_errlog(c_ServiceName, "S31575", LIBMSG, c_err_msg );
     tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
    }
    /***Ver 6.6 ***/

	}
	
	i_commit_flg = 0; /*** Ver 2.3 ***/

	/*** Ver 3.2 Start ***/
	if (st_orderbook.c_prd_typ == SLTP_FUTUREPLUS)
	{
		MEMSET(st_cvr_xchngbk);	

		EXEC SQL
			SELECT
					fod_sltp_ordr_rfrnc				
			INTO
					:c_sltp_ord_rfrnc	
		FROM  FOD_FO_ORDR_DTLS
		WHERE fod_ordr_rfrnc = :st_orderbook.c_ordr_rfrnc;
		if(SQLCODE !=0)
		{
			fn_errlog ( c_ServiceName, "S31580", SQLMSG, ptr_st_err_msg->c_err_msg );
			tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
		}

		i_status = 0;

		/**** commented in Ver 3.7 ****

		EXEC SQL
		  SELECT FXB_ORS_MSG_TYP,
				 FXB_MDFCTN_CNTR,
	  			 nvl(to_char(FXB_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss'),'-')
		  INTO   :st_cvr_xchngbk.l_ors_msg_typ,
				 :st_cvr_xchngbk.l_mdfctn_cntr,
				 :st_cvr_xchngbk.c_ack_tm
		  FROM   FXB_FO_XCHNG_BOOK
		  WHERE  FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc
		  AND    FXB_MDFCTN_CNTR = (select max(FXB_MDFCTN_CNTR)
									from   FXB_FO_XCHNG_BOOK
									where  FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc)
		  AND    FXB_RMS_PRCSD_FLG = 'N';

		**** commented in Ver 3.7 ****/
	
	/*** Added Ver 3.7 Starts ***/

	EXEC SQL
	SELECT FXB_ORS_MSG_TYP,
				 FXB_MDFCTN_CNTR,
				 nvl(to_char(FXB_ACK_TM, 'dd-mon-yyyy hh24:mi:ss'),'*'),
				 NVL(FXB_XCHNG_CNCLD_QTY,0)                                   /**** Ver 4.5 *****/
  INTO   :st_cvr_xchngbk.l_ors_msg_typ,
         :st_cvr_xchngbk.l_mdfctn_cntr,
         :st_cvr_xchngbk.c_ack_tm,
         :st_cvr_xchngbk.l_xchng_can_qty                              /***** Ver 4.5 ***/
	FROM   FXB_FO_XCHNG_BOOK
	WHERE  FXB_ORDR_RFRNC =  :c_sltp_ord_rfrnc 
	AND    FXB_MDFCTN_CNTR =
													(
    												SELECT min(FXB_MDFCTN_CNTR)
    												FROM   FXB_FO_XCHNG_BOOK
    												WHERE  FXB_ORDR_RFRNC =  :c_sltp_ord_rfrnc 
    												AND    FXB_JIFFY =
        																			(select min(FXB_JIFFY)
        							 												from   FXB_FO_XCHNG_BOOK
        							 												where  FXB_ORDR_RFRNC =  :c_sltp_ord_rfrnc 
        							 												AND    FXB_RMS_PRCSD_FLG = 'N'
        																			)
    												AND FXB_RMS_PRCSD_FLG = 'N'
													)
	AND    FXB_RMS_PRCSD_FLG = 'N';	
	
	 /*** Added Ver 3.7 Ends ***/

		if(SQLCODE !=0)
		{
      /** i_status = 1; ** Commented in Ver 3.7 ***/
			fn_userlog( c_ServiceName, "No opposite leg unprocessed record");
			fn_userlog( c_ServiceName, "Checking for Same leg "); /*** Added  in Ver 3.7 ***/ 
			fn_errlog ( c_ServiceName, "S31585", SQLMSG, ptr_st_err_msg->c_err_msg );

		/***************** Added in Ver 3.7 Starts *****************/
			
			MEMSET(st_cvr_xchngbk.l_ors_msg_typ);
			MEMSET(st_cvr_xchngbk.l_mdfctn_cntr);
			MEMSET(st_cvr_xchngbk.c_ack_tm);

	
			EXEC SQL
			SELECT FXB_ORS_MSG_TYP,
						 FXB_MDFCTN_CNTR,
						 nvl(to_char(FXB_ACK_TM, 'dd-mon-yyyy hh24:mi:ss'),'*'),
             NVL(FXB_XCHNG_CNCLD_QTY,0)                                   /**** Ver 4.5 *****/
	    INTO   :st_cvr_xchngbk.l_ors_msg_typ,
             :st_cvr_xchngbk.l_mdfctn_cntr,
             :st_cvr_xchngbk.c_ack_tm,
             :st_cvr_xchngbk.l_xchng_can_qty                              /***** Ver 4.5 ***/
			FROM   FXB_FO_XCHNG_BOOK
			WHERE  FXB_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc 
			AND    FXB_JIFFY =
    										(select min(FXB_JIFFY)
    										from   FXB_FO_XCHNG_BOOK
    										where  FXB_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc 
    										AND    FXB_RMS_PRCSD_FLG = 'N'
												)
			AND    FXB_RMS_PRCSD_FLG = 'N'; 

			 /**** Added in Ver 3.7 Ends ***/

		  if(SQLCODE !=0)
    	{
				i_status = 1;
			  fn_userlog( c_ServiceName, "No Same leg to process ");
      	fn_errlog ( c_ServiceName, "S31590", SQLMSG, ptr_st_err_msg->c_err_msg );
			}
			else
			{
        if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
        {	
					fn_userlog( c_ServiceName, "Same leg unprocessed ... so process it...");
				}
      	strcpy(st_cvr_xchngbk.c_ordr_rfrnc, st_orderbook.c_ordr_rfrnc );
        if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
        {   
      		fn_userlog( c_ServiceName, "Call SFO_FUT_CVRACK for Order Ref:%s:", st_orderbook.c_ordr_rfrnc );
				}
			}
			/***************** Added in Ver 3.7 Ends *********************/
		}
		else
		{
      if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
      { 
				fn_userlog( c_ServiceName, "Opposite leg unprocessed record exist so process it...");
			}
			strcpy(st_cvr_xchngbk.c_ordr_rfrnc, c_sltp_ord_rfrnc)  ;    														/** Added in Ver 3.7 ***/
      if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
      {
			 fn_userlog( c_ServiceName, "Call SFO_FUT_CVRACK for Order Ref:%s:", c_sltp_ord_rfrnc);	/** Added in Ver 3.7 ***/
			}
		}

		strcpy(st_cvr_xchngbk.c_entry_dt_tm , st_cvr_xchngbk.c_ack_tm);

		if(i_status != 1)
		{
			if(DEBUG_MSG_LVL_0)
			{
				fn_userlog( c_ServiceName, "Before Calling  SFO_FUT_CVRACK for Order Ref:%s:", c_sltp_ord_rfrnc);
			}

			/** fn_userlog( c_ServiceName, "Call SFO_FUT_CVRACK for Order Ref:%s:", c_sltp_ord_rfrnc); ** Commented in Ver 3.7 ***/
			fn_cpy_ddr ( st_cvr_xchngbk.c_rout_crt );
		
      if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
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

			/******** Added in Ver 3.7 ******COmmented in Ver 4.5 *****

    	EXEC SQL
      SELECT  NVL(SUM(ftd_exctd_qty),0)
      INTO    :l_ftd_ord_exe_qty
      FROM    ftd_fo_trd_dtls
      WHERE   ftd_ordr_rfrnc = :st_cvr_xchngbk.c_ordr_rfrnc ;

    	if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
    	{
    	  fn_errlog ( c_ServiceName, "S31595", SQLMSG, ptr_st_err_msg->c_err_msg );
    	  *** fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );  *** Commented in Ver 4.4 ***
    	  tpreturn(TPFAIL, ERR_BFR, (char *)&ptr_st_err_msg, 0, 0 );
    	}
	
      if(DEBUG_MSG_LVL_3)  *** Ver 3.7 ***
      {
 	    	fn_userlog( c_ServiceName, "l_ftd_ord_exe_qty: %ld",l_ftd_ord_exe_qty);
			}
  	  st_cvr_xchngbk.l_xchng_can_qty = l_ftd_ord_exe_qty;

			******** Added in Ver 3.7 ******

			******* Commented Ver 4.5 Ends *****/

			/** strcpy(st_cvr_xchngbk.c_ordr_rfrnc, c_sltp_ord_rfrnc)  ;   *** Commented in Ver 3.7 **********/ 

       
			 i_returncode = fn_acall_svc ( c_ServiceName,
										 c_err_msg,
										 &st_cvr_xchngbk,
										 "vw_xchngbook",
										 sizeof ( st_cvr_xchngbk ),
										 TPNOREPLY,
										 "SFO_FUT_CVRACK" );
			  if ( i_returncode != 0 )
			  {
					fn_userlog( c_ServiceName, "Error in :%d:",i );
					fn_errlog( c_ServiceName, "S31600", Fstrerror32( i_ferr[i]),c_err_msg );
					/*** tpfree((char *)ptr_fml_Sbuf);  *** Commented in Ver 4.4 ***/
					tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );	/*** Ver 4.4 changed from ptr_fml_Sbuf to ptr_st_err_msg ***/
			  }
			
			if(DEBUG_MSG_LVL_0)
			{
				fn_userlog( c_ServiceName, "After Calling  SFO_FUT_CVRACK for Order Ref:%s:", c_sltp_ord_rfrnc);
			}
		}
	}
	/*** Ver 3.2 End ***/

	/******** Added in Ver 3.7 Starts ************/

	else if(st_orderbook.c_prd_typ == FUTURES || st_orderbook.c_prd_typ == FUTURE_PLUS)
	{
	 
			i_status = 0;

      MEMSET(st_cvr_xchngbk.l_ors_msg_typ);
      MEMSET(st_cvr_xchngbk.l_mdfctn_cntr);
      MEMSET(st_cvr_xchngbk.c_ack_tm);

			EXEC SQL
			SELECT FXB_ORS_MSG_TYP,
						 FXB_MDFCTN_CNTR,
						 nvl(to_char(FXB_ACK_TM, 'dd-mon-yyyy hh24:mi:ss'),'*'),
             NVL(FXB_XCHNG_CNCLD_QTY,0)                                   /**** Ver 4.5 *****/
      INTO   :st_cvr_xchngbk.l_ors_msg_typ,
             :st_cvr_xchngbk.l_mdfctn_cntr,
             :st_cvr_xchngbk.c_ack_tm,
             :st_cvr_xchngbk.l_xchng_can_qty                              /***** Ver 4.5 ***/
			FROM   FXB_FO_XCHNG_BOOK
			WHERE  FXB_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc 
			AND    FXB_JIFFY =
    										(select min(FXB_JIFFY)
    										from   FXB_FO_XCHNG_BOOK
    										where  FXB_ORDR_RFRNC =  :st_orderbook.c_ordr_rfrnc 
    										AND    FXB_RMS_PRCSD_FLG = 'N')
			AND    FXB_RMS_PRCSD_FLG = 'N';

      if(SQLCODE !=0)
      {
			  i_status = 1;
        fn_userlog( c_ServiceName, "No Same leg to process ");
        fn_errlog ( c_ServiceName, "S31605", SQLMSG, ptr_st_err_msg->c_err_msg );
      }
      else
     	{
        strcpy(st_cvr_xchngbk.c_ordr_rfrnc, st_orderbook.c_ordr_rfrnc );
      	if(DEBUG_MSG_LVL_3) 
      	{
        	fn_userlog( c_ServiceName, "Same leg unprocessed ... so process it...");
        	fn_userlog( c_ServiceName, "Call SFO_FUT_CVRACK for Order Ref:%s:", st_orderbook.c_ordr_rfrnc );
				}
      }	

      if(st_cvr_xchngbk.l_ors_msg_typ == ORS_2LN_ORD_ACPT || st_cvr_xchngbk.l_ors_msg_typ == ORS_3LN_ORD_ACPT ||
          st_cvr_xchngbk.l_ors_msg_typ == ORS_2L_ORD_CNCL || st_cvr_xchngbk.l_ors_msg_typ == ORS_3L_ORD_CNCL)
      {
        l_ors_msg_typ = st_cvr_xchngbk.l_ors_msg_typ;
        strcpy(c_ipord_ref,st_orderbook.c_ordr_rfrnc);

        i_returncode = fn_spd_acpt(c_ServiceName,c_ipord_ref,l_ors_msg_typ);
        if( i_returncode == -1)
        {
          fn_errlog ( c_ServiceName, "S31610", SQLMSG, ptr_st_err_msg->c_err_msg );
          /*** fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ); *** Commented in Ver 4.4 ***/
          tpreturn(TPFAIL, ERR_BFR, (char *)&ptr_st_err_msg, 0, 0 );
				}
      }

      if(DEBUG_MSG_LVL_3) 
      {
				fn_userlog( c_ServiceName, "  test st_cvr_xchngbk.l_xchng_can_qty is :%ld:",st_cvr_xchngbk.l_xchng_can_qty);
			}
  if(i_status!=1 && (st_cvr_xchngbk.l_ors_msg_typ!=ORS_2LN_ORD_ACPT && st_cvr_xchngbk.l_ors_msg_typ != ORS_3LN_ORD_ACPT &&        st_cvr_xchngbk.l_ors_msg_typ != ORS_2L_ORD_CNCL && st_cvr_xchngbk.l_ors_msg_typ != ORS_3L_ORD_CNCL))
    {
      fn_cpy_ddr ( st_cvr_xchngbk.c_rout_crt );
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

			/** MEMSET(l_ftd_ord_exe_qty);   Commented in Ver 4.5 *******
	
			EXEC SQL
      SELECT  NVL(SUM(ftd_exctd_qty),0)
      INTO    :l_ftd_ord_exe_qty
      FROM    ftd_fo_trd_dtls
      WHERE   ftd_ordr_rfrnc = :st_cvr_xchngbk.c_ordr_rfrnc ;

      if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
      {
        fn_errlog ( c_ServiceName, "S31615", SQLMSG, ptr_st_err_msg->c_err_msg );
        *** fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ); *** Commented in Ver 4.4 ***
        tpreturn(TPFAIL, ERR_BFR, (char *)&ptr_st_err_msg, 0, 0 );
      }

      if(DEBUG_MSG_LVL_3)  
      {
      	fn_userlog( c_ServiceName, "l_ftd_ord_exe_qty: %ld",l_ftd_ord_exe_qty); 
			}	
			st_cvr_xchngbk.l_xchng_can_qty = l_ftd_ord_exe_qty ;
		
			********** Commented Ver 4.5 Ends ********/

      if(DEBUG_MSG_LVL_3) 
      {
				fn_userlog( c_ServiceName, "st_cvr_xchngbk.can_qty:%ld:",st_cvr_xchngbk.l_xchng_can_qty);
			}
       
       i_returncode = fn_acall_svc ( c_ServiceName,
                     c_err_msg,
                     &st_cvr_xchngbk,
                     "vw_xchngbook",
                     sizeof ( st_cvr_xchngbk ),
                     TPNOREPLY,
                     "SFO_FUT_CVRACK" );
       
        if ( i_returncode != 0 )
        {
          fn_userlog( c_ServiceName, "Error in :%d:",i );
          fn_errlog( c_ServiceName, "S31620", Fstrerror32( i_ferr[i]),c_err_msg );
          /*** tpfree((char *)ptr_fml_Sbuf); Commented in Ver 4.4 ***/
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );	/*** Ver 4.4 changed from ptr_fml_Sbuf to ptr_st_err_msg ***/
        }

      if(DEBUG_MSG_LVL_0)
      {
        fn_userlog( c_ServiceName, "After Calling  SFO_FUT_CVRACK for Order Ref:%s:", st_orderbook.c_ordr_rfrnc );
      }
    }
	}

	/******** Added in Ver 3.7 Ends ************/
	


/************Ver 1.9 Starts
	if ( ptr_st_xchngbook->l_ors_msg_typ != ORS_ORD_EXP )
	{
		if ( i_trnsctn == LOCAL_TRNSCTN )
		{
			fn_cpy_ddr ( st_xchngbook.c_rout_crt );

			i_returncode = fn_acall_svc( c_ServiceName,
   	                               ptr_st_err_msg->c_err_msg,
   	                               &st_xchngbook,
   	                               "vw_xchngbook",
   	                               sizeof( struct vw_xchngbook),
   	                               TPNOREPLY,
   	                               "SFO_ML_FRMTTR" );
			if ( i_returncode != SUCC_BFR )
			{
				fn_errlog ( c_ServiceName, "S31625", LIBMSG, c_err_msg );
 				tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
			}
		}
	}
******Version 1.9 Ends******/
/*** Commented in Ver 3.6 
	if ( st_orderbook.c_prd_typ != SLTP_FUTUREPLUS )	*** Ver 3.1 ***
	{
	EXEC SQL
		DECLARE	cur_trd CURSOR FOR
		SELECT	ftd_clm_mtch_accnt,
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
		FROM		ftd_fo_trd_dtls
		WHERE		ftd_ordr_rfrnc = :ptr_st_xchngbook->c_ordr_rfrnc
		AND			ftd_rms_prcsd_flg = 'N';
  
	EXEC SQL
		OPEN	cur_trd;
	}
	else *** Ver 3.1 ***
	{
	EXEC SQL
		DECLARE	cur_trd1 CURSOR FOR
		SELECT	ftd_clm_mtch_accnt,
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
		FROM		ftd_fo_trd_dtls
		WHERE		ftd_ordr_rfrnc  in ( :ptr_st_xchngbook->c_ordr_rfrnc, :c_sltp_ord_rfrnc)
		AND			ftd_rms_prcsd_flg = 'N';
  
	EXEC SQL
		OPEN	cur_trd1;
	}

	if ( SQLCODE != 0 )
	{
  	fn_errlog ( c_ServiceName, "S31630", SQLMSG, ptr_st_err_msg->c_err_msg );
		if ( st_orderbook.c_prd_typ != SLTP_FUTUREPLUS )	*** Ver 3.1 ***
		{
		EXEC SQL 
			CLOSE cur_trd;
		}
		else *** Ver 3.1 ***
		{
			EXEC SQL 
			CLOSE cur_trd1;
		}
  	tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
	}


	while (1)
	{
		if ( st_orderbook.c_prd_typ != SLTP_FUTUREPLUS )	*** Ver 3.1 ***
		{
		EXEC SQL
			FETCH	cur_trd
			INTO	:st_tradebook.c_cln_mtch_accnt,
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
						:st_tradebook.l_exctd_rt;
		}
		else *** Ver 3.1 ***
		{
			EXEC SQL
			FETCH	cur_trd1
			INTO	:st_tradebook.c_cln_mtch_accnt,
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
						:st_tradebook.l_exctd_rt;
		}

		if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
		{
  		fn_errlog ( c_ServiceName, "S31635", SQLMSG, ptr_st_err_msg->c_err_msg );
			
			if ( st_orderbook.c_prd_typ != SLTP_FUTUREPLUS )	*** Ver 3.1 ***
			{
			EXEC SQL 
				CLOSE cur_trd;
			}
			else *** Ver 3.1 ***
			{
				EXEC SQL 
					CLOSE cur_trd1;
			}
  		tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
		}

		if ( SQLCODE == NO_DATA_FOUND )
		{
			break;
		}

  	*** Added for Order Routing ***
		fn_cpy_ddr ( st_tradebook.c_rout_crt );

		i_returncode = fn_acall_svc( c_ServiceName,
                                ptr_st_err_msg->c_err_msg,
                                &st_tradebook,
                                "vw_tradebook",
                                sizeof( struct vw_tradebook),
                                TPNOREPLY,
                                "SFO_FUT_TCONF" );
		if ( i_returncode != SUCC_BFR )
		{
			fn_errlog ( c_ServiceName, "S31640", LIBMSG, c_err_msg );

			if ( st_orderbook.c_prd_typ != SLTP_FUTUREPLUS )	*** Ver 3.1 ***
			{
			EXEC SQL
				CLOSE cur_trd;
			}
			else *** Ver 3.1 ***
			{
				EXEC SQL 
					CLOSE cur_trd1;
			}
  		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
		}
     
	}

	if ( st_orderbook.c_prd_typ != SLTP_FUTUREPLUS )	*** Ver 3.1 ***
	{
	EXEC SQL
		CLOSE cur_trd;
	}
	else *** Ver 3.1 ***
	{
		EXEC SQL
		CLOSE cur_trd1;
	}

  **** Ver 3.6  Comments Ends ****/

 /** Ver 3.6 Starts **/
      if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
      {
    		fn_userlog( c_ServiceName, "Before Call of fn_prcs_trd.........2");
			}

    /*** Commented in Ver 4.3
    i_returncode = fn_prcs_trd(&st_orderbook,c_ServiceName,c_err_msg);
      ***/

    /*** Ver 4.3 Starts Here ***/
  i_returncode = fn_prcs_trd(c_ServiceName,c_err_msg,st_orderbook.c_prd_typ,st_orderbook.c_ordr_rfrnc);
    /*** Ver 4.3 Ends Here ***/

  if ( i_returncode != 0 )
  {
    fn_errlog ( c_ServiceName, "S31645", LIBMSG, c_err_msg );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
  }
      if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
      {
    		fn_userlog( c_ServiceName, "After Call of fn_prcs_trd.........2");
			}
 /** Ver 3.6 Ends **/

 if ( st_orderbook.c_prd_typ != SLTP_FUTUREPLUS )	/*** if condition added in Ver 2.1 ***/
 {
	if ( st_pstn_actn.c_mtm_flag == NO_OPEN_ORDERS )
	{
		switch ( ptr_st_xchngbook->l_ors_msg_typ )
		{
			case	ORS_NEW_ORD_ACPT:
					sprintf(st_pstn_actn.c_imtm_rmrks,
					"RECEIVED SQUAREOFF CONFIRMATION FOR [%s]",
																						ptr_st_xchngbook->c_ordr_rfrnc );
					break;

			case	ORS_NEW_ORD_RJCT:
					sprintf(st_pstn_actn.c_imtm_rmrks,
					"RECEIVED SQUAREOFF REJECTION FOR [%s]",
																						ptr_st_xchngbook->c_ordr_rfrnc );
					break;

			case	ORS_CAN_ORD_ACPT:
					sprintf(st_pstn_actn.c_imtm_rmrks,
					"RECEIVED CANCELLATION ACCEPTANCE FOR [%s]",
																						ptr_st_xchngbook->c_ordr_rfrnc );
					break;

			case	ORS_CAN_ORD_RJCT:
					sprintf(st_pstn_actn.c_imtm_rmrks,
					"RECEIVED CANCELLATION REJECTION FOR [%s]",
																						ptr_st_xchngbook->c_ordr_rfrnc );
					break;
			/******default ????? ************/
		}

    sprintf( c_tag,"bat_intra_mtm_%s",st_pstn_actn.c_xchng_cd);

  	/************************************** 
		fn_cpy_ddr ( st_pstn_actn.c_rout_crt );
  	**************************************/

    i_returncode = fn_acall_svc_fml ( c_ServiceName,
                                  		ptr_st_err_msg->c_err_msg,
                                  		"SFO_PRCS_FIMTM",
                                  		/***0, **** Ver 4.6 ***/
                                      TPNOTRAN|TPNOREPLY,  /**** Ver 4.6 ***/
                                  		10,     /***  Converted From 9 To 10 For Ver 1.8  ***/
                 		FFO_XCHNG_CD, ( char * )st_pstn_actn.c_xchng_cd,
                 		FFO_EBA_MTCH_ACT_NO, ( char *)st_pstn_actn.c_cln_mtch_accnt,
                 		FFO_PRDCT_TYP, ( char * )&st_pstn_actn.c_prd_typ,
                		FFO_UNDRLYNG, ( char * )st_pstn_actn.c_undrlyng,
                		FFO_CTGRY_INDSTK, ( char * )&st_pstn_actn.c_ctgry_indstk,
                		FFO_CNTRCT_TAG, ( char * )&st_pstn_actn.c_cntrct_tag,
                 		FFO_IMTM_RMRKS, ( char * )st_pstn_actn.c_imtm_rmrks,
                 		FFO_PIPE_ID, ( char * )st_xchngbook.c_pipe_id,
                    FFO_SPL_FLG,(char *)&c_prc_flg,     /***  Ver 1.8 ***/
                 		FFO_BAT_TAG, ( char * )c_tag);
		if ( i_returncode != SUCC_BFR )
		{
			fn_errlog ( c_ServiceName, "S31650", LIBMSG, c_err_msg );
  		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
		}
  }

  /** Ver 2.8 Starts Here ***/
  if( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_RJCT ||
      ptr_st_xchngbook->l_ors_msg_typ == ORS_NEW_ORD_RJCT )
  {
      if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
      {
    		fn_userlog(c_ServiceName," c_mtm_flg ORS_CAN_ORD_RJCT  :%c:",c_mtm_flg);
			}
    if( c_mtm_flg == 'A' )  /** Auto MTM **/
    {
      fn_upd_fmdstts(c_ServiceName,&st_pstn_actn,'N',c_err_msg);
    }
  } 

	if( DEBUG_MSG_LVL_3 )
  {
    fn_userlog( c_ServiceName,"st_orderbook.c_channel :%s:",st_orderbook.c_channel);
    fn_userlog( c_ServiceName,"st_orderbook.c_spl_flg :%c:",st_orderbook.c_spl_flg);
  }

  if( ptr_st_xchngbook->l_ors_msg_typ == ORS_NEW_ORD_ACPT &&
      ( strcmp(st_orderbook.c_channel, "SYS") == 0 )      &&
        st_orderbook.c_spl_flg == 'M'
    )
  {
    fo_ins_sqofford(c_ServiceName,c_err_msg,&st_orderbook,'I',st_pstn_actn.c_cntrct_tag);
  }


  /** Ver 2.8 Ends Here ***/


  /*** ver 6.2 starts ***/  
  fn_userlog( c_ServiceName,"BEFORE STRSTR--------------------");
  if ( strstr( c_remarks_phy,"Provisional Delivery Process Cancellation") )
  {
 			fn_userlog( c_ServiceName,"INSIDE STRSTR-------");
      /***c_span_flg = 'S';***commented in ver 6.4***/
			c_span_flg = 'N'; /*** ver 6.4 ***/
      c_rqst_typ = 'I'; 
      fn_userlog( c_ServiceName,"STRSTR :%s:%s:%c:%c:%s:%s:%c:%c:",st_pstn_actn.c_cln_mtch_accnt,st_pstn_actn.c_xchng_cd,st_pstn_actn.c_prd_typ,st_pstn_actn.c_ctgry_indstk,st_pstn_actn.c_undrlyng,st_pstn_actn.c_expry_dt,st_pstn_actn.c_exrc_typ,c_ordr_flow_phy);

      EXEC SQL
            SELECT decode(:c_ordr_flow_phy,'B',greatest(abs(FfP_IBUY_QTY),abs(FFP_EXBUY_QTY)) ,'S',greatest(abs(FFP_ISELL_QTY),abs(FFP_EXSELL_QTY) ),0) ,
           FFP_OPNPSTN_FLW
            INTO :l_prvsnl_pending_qty,
                 :c_prvsnl_opnpos_flow
             FROM FFP_FO_FUTURES_PSTN
        WHERE  FFP_CLM_MTCH_ACCNT =:st_pstn_actn.c_cln_mtch_accnt
        AND  FFP_XCHNG_CD       =:st_pstn_actn.c_xchng_cd
        AND  FFP_PRDCT_TYP      =:st_pstn_actn.c_prd_typ
        AND  FFP_INDSTK         =:st_pstn_actn.c_ctgry_indstk
        AND  FFP_UNDRLYNG       =:st_pstn_actn.c_undrlyng
        AND  FFP_EXPRY_DT       =:st_pstn_actn.c_expry_dt
        AND  FFP_EXER_TYP       =:st_pstn_actn.c_exrc_typ
        AND  FFP_OPNPSTN_FLW    =:c_ordr_flow_phy;

  if ( SQLCODE != 0 )
  {
      fn_errlog ( c_ServiceName, "S31655", SQLMSG, c_err_msg );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
   }




      if( st_pstn_actn.c_prd_typ == 'F' )
      {
          if( c_prvsnl_opnpos_flow == 'B')
          {
             c_sub_mode = 'A';
          }
          else
          {
             c_sub_mode = 'S';
          }
      }
      else if ( st_pstn_actn.c_prd_typ == 'O')
      {
           if( st_pstn_actn.c_opt_typ == 'C' )
           {
              if( c_prvsnl_opnpos_flow == 'B' )
              {
                  c_sub_mode = 'A';
              }
              else
              {
                   c_sub_mode = 'S';
              }
          }
          else if ( st_pstn_actn.c_prd_typ == 'P' )
          {
              if( c_prvsnl_opnpos_flow == 'B' )
              {
                  c_sub_mode = 'S';
              }
              else
              {
                 c_sub_mode = 'A';
              }
          }
      }
      fn_userlog( c_ServiceName,"STRSTR :%ld:",l_prvsnl_pending_qty);
       if(l_prvsnl_pending_qty  == 0 )
       {
         i_returncode = fn_acall_svc_fml ( c_ServiceName,
                                c_err_msg,
                               "SFO_PRVSNL_PRC",
                                TPNOTRAN|TPNOREPLY,
                                13,
                                FFO_EBA_MTCH_ACT_NO,(char*)st_pstn_actn.c_cln_mtch_accnt,
                                FFO_XCHNG_CD,(char*)st_pstn_actn.c_xchng_cd,
                                FFO_PRDCT_TYP,(char*)&st_pstn_actn.c_prd_typ,
                                FFO_UNDRLYNG, (char*)st_pstn_actn.c_undrlyng,
                                FFO_EXPRY_DT, (char*)st_pstn_actn.c_expry_dt,
                                FFO_EXER_TYP, (char*)&st_pstn_actn.c_exrc_typ,
                                FFO_OPT_TYP, (char*)&st_pstn_actn.c_opt_typ,
                                FFO_STRK_PRC, (char*)&st_pstn_actn.l_strike_prc,
                                FFO_CTGRY_INDSTK, (char*)&st_pstn_actn.c_ctgry_indstk,
                                FFO_RQST_TYP, (char*)&c_rqst_typ,
                                FFO_STATUS_FLG,(char *)&c_span_flg,
                                FFO_MOD_CAN_FLG,(char *)&c_sub_mode,
                                FFO_OPNPSTN_FLW,(char *)&c_prvsnl_opnpos_flow);

    if ( i_returncode == -1 )
    {
      fn_errlog ( c_ServiceName, "S31660", LIBMSG, c_err_msg );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
    }
  }
} /** ver 6.2 ends  ***/
 } /*** Ver 2.1 ***/

	tpfree ( ( char * ) ptr_st_err_msg );
  tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_xchngbook, 0, 0 );
}

int fn_Freftoord_forupd(struct vw_orderbook *ptr_st_orderbook,
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

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
  	fn_userlog( c_ServiceName,"Function REF_TO_ORD WITH FOR_UPDATE");
	}
  if( DEBUG_MSG_LVL_3 )
  {
    fn_userlog( c_ServiceName,"c_ordr_rfrnc:%s:",ptr_st_orderbook->c_ordr_rfrnc);
    fn_userlog( c_ServiceName,"Requested Operation :%c:",ptr_st_orderbook->c_oprn_typ);
  }


   MEMSET( ptr_st_orderbook->c_ctcl_id);

   EXEC SQL
      SELECT  fod_clm_mtch_accnt,
              fod_clnt_ctgry,
              fod_pipe_id,
              fod_xchng_cd,
              decode(:ptr_st_orderbook->c_oprn_typ,'S',(decode(fod_prdct_typ,'P','F',fod_prdct_typ)),fod_prdct_typ),
              fod_undrlyng,
              to_char ( fod_expry_dt, 'dd-Mon-yyyy' ),
              fod_exer_typ,
              fod_opt_typ,
              fod_strk_prc,
              fod_ordr_flw,
              fod_lmt_mrkt_sl_flg,
              fod_dsclsd_qty,
              fod_ordr_tot_qty,
              fod_lmt_rt,
              fod_stp_lss_tgr,
              fod_ordr_type,
              to_char ( fod_ordr_valid_dt, 'dd-Mon-yyyy' ),
              to_char ( fod_trd_dt, 'dd-Mon-yyyy' ),
              fod_ordr_stts,
              fod_exec_qty,
              NVL(fod_exec_qty_day,0),
              fod_cncl_qty,
              fod_exprd_qty, 
		          fod_sprd_ordr_ref,
       	      fod_mdfctn_cntr,
           	  fod_settlor,
              fod_ack_nmbr,
              fod_spl_flag,
              fod_indstk,
              to_char ( fod_ord_ack_tm, 'dd-Mon-yyyy hh24:mi:ss' ),
              to_char ( fod_lst_rqst_ack_tm, 'dd-Mon-yyyy hh24:mi:ss' ),
              fod_pro_cli_ind,
              nvl(fod_ctcl_id,' '),
              fod_channel
					INTO  :ptr_st_orderbook->c_cln_mtch_accnt,
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
             :ptr_st_orderbook->c_ctcl_id,
             :ptr_st_orderbook->c_channel
       FROM  fod_fo_ordr_dtls
       WHERE fod_ordr_rfrnc = :ptr_st_orderbook->c_ordr_rfrnc
       FOR   UPDATE OF fod_ordr_rfrnc;
	
	if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31665", SQLMSG, c_err_msg );
    return -1;
  }

  rtrim ( ptr_st_orderbook->c_expry_dt );
  rtrim ( ptr_st_orderbook->c_ctcl_id );

  if( DEBUG_MSG_LVL_3 )
  {
    fn_userlog( c_ServiceName, "match account after is :%s:",ptr_st_orderbook->c_cln_mtch_accnt);
    fn_userlog( c_ServiceName, "cp after is :%s:",ptr_st_orderbook->c_settlor);
  }

  return 0;
}

int fn_Fref_to_omd ( struct vw_xchngbook *ptr_i_st_xchngbook,
                    struct vw_xchngbook *ptr_st_xchngbook,
                    char   *c_mtm_flg,             /** Ver 2.8 **/
                    char   *c_prcimp_flg,          /** Ver 4.0 **/
										char	 *c_lst_act_ref,		 /** Ver 5.3 **/
                    char *c_ServiceName,
                    char *c_err_msg)
{

  EXEC SQL BEGIN DECLARE SECTION;
    short i_slm_flg;
    short i_dsclsd_qty;
    short i_ord_lmt_rt;
    short i_stp_lss_tgr;
    short i_valid_dt;
    short i_ord_typ;
    short i_sprd_ord_ind;
    short i_quote;
    short i_qt_tm;
    short i_frwrd_tm;
    short i_rms_prcsd_flg;
    short i_ors_msg_typ;
    short i_ack_tm;
    short i_xchng_rmrks;
    short i_xchng_can_qty;
    short i_spl_flg;
    short i_req_typ;
    short i_ord_tot_qty;
    short i_rqst_tm;
    short i_ex_ordr_typ;
    short i_fxb_ip;   /** Ver 2.8 **/
		char c_lst_act_ref_tmp [22];		/** Ver 5.3 **/
  EXEC SQL END DECLARE SECTION;

  char c_fxb_ip[15+1];  /** Ver 2.8 **/
  char c_prcimpflg='\0';  /** Ver 4.0 **/

  MEMSET(c_fxb_ip);     /** Ver 2.8 **/
	MEMSET(c_lst_act_ref_tmp);				/** Ver 5.3 **/

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
		fn_userlog( c_ServiceName,"Inside Function REF_TO_OMD");
	}
	EXEC SQL
      SELECT  fxb_xchng_cd,
              fxb_pipe_id,
              to_char ( fxb_mod_trd_dt, 'dd-mon-yyyy' ),
              fxb_ordr_sqnc,
              fxb_lmt_mrkt_sl_flg,
              fxb_dsclsd_qty,
              fxb_ordr_tot_qty,
              fxb_lmt_rt,
              fxb_stp_lss_tgr,
              to_char( fxb_ordr_valid_dt, 'dd-mon-yyyy' ),
              fxb_ordr_type,
              fxb_sprd_ord_ind,
              fxb_rqst_typ,
              fxb_quote,
              to_char ( fxb_qt_tm, 'dd-mon-yyyy hh24:mi:ss' ),
              to_char ( fxb_rqst_tm, 'dd-mon-yyyy hh24:mi:ss' ),
              to_char ( fxb_frwd_tm, 'dd-mon-yyyy hh24:mi:ss' ),
              fxb_plcd_stts,
              fxb_rms_prcsd_flg,
             /* to_char ( fxb_ack_tm, 'dd-mon-yyyy hh24:mi:ss' ),*/
							nvl(to_char(fxb_ack_tm,'dd-mon-yyyy hh24:mi:ss'),to_char(fxb_rqst_tm,'dd-mon-yyyy hh24:mi:ss')),   /*** Ver 1.6 ***/
              fxb_xchng_rmrks,
              fxb_ex_ordr_typ,
              fxb_xchng_cncld_qty,
              fxb_spl_flag,
              fxb_ip,                     /** Ver 2.8 **/ 
              NVL(FXB_PRCIMPV_FLG,'N'),   /** Ver 4.0 **/
              NVL(FXB_LST_ACT_REF,'0')      /** Ver 5.3 **/ /*** '*' changed to '0' in ver 5.7 ***/ 
			INTO    :ptr_i_st_xchngbook->c_xchng_cd,
              :ptr_i_st_xchngbook->c_pipe_id,
              :ptr_i_st_xchngbook->c_mod_trd_dt,
              :ptr_i_st_xchngbook->l_ord_seq,
              :ptr_i_st_xchngbook->c_slm_flg:i_slm_flg,
              :ptr_i_st_xchngbook->l_dsclsd_qty:i_dsclsd_qty,
              :ptr_i_st_xchngbook->l_ord_tot_qty:i_ord_tot_qty,
              :ptr_i_st_xchngbook->l_ord_lmt_rt:i_ord_lmt_rt,
              :ptr_i_st_xchngbook->l_stp_lss_tgr:i_stp_lss_tgr,
              :ptr_i_st_xchngbook->c_valid_dt:i_valid_dt,
              :ptr_i_st_xchngbook->c_ord_typ:i_ord_typ,
              :ptr_i_st_xchngbook->c_sprd_ord_ind:i_sprd_ord_ind,
              :ptr_i_st_xchngbook->c_req_typ:i_req_typ,
              :ptr_i_st_xchngbook->l_quote:i_quote,
              :ptr_i_st_xchngbook->c_qt_tm:i_qt_tm,
              :ptr_i_st_xchngbook->c_rqst_tm:i_rqst_tm,
              :ptr_i_st_xchngbook->c_frwrd_tm:i_frwrd_tm,
              :ptr_i_st_xchngbook->c_plcd_stts,
              :ptr_i_st_xchngbook->c_rms_prcsd_flg:i_rms_prcsd_flg,
              :ptr_i_st_xchngbook->c_ack_tm:i_ack_tm,
              :ptr_i_st_xchngbook->c_xchng_rmrks:i_xchng_rmrks,
              :ptr_i_st_xchngbook->c_ex_ordr_typ:i_ex_ordr_typ,
              :ptr_i_st_xchngbook->l_xchng_can_qty:i_xchng_can_qty,
              :ptr_i_st_xchngbook->c_spl_flg:i_spl_flg,
              :c_fxb_ip:i_fxb_ip,
              :c_prcimpflg,           /** Ver 4.0 **/
							:c_lst_act_ref_tmp			/** Ver 5.3 **/
      FROM    fxb_fo_xchng_book
      WHERE   fxb_ordr_rfrnc = :ptr_i_st_xchngbook->c_ordr_rfrnc
      AND     fxb_mdfctn_cntr = :ptr_i_st_xchngbook->l_mdfctn_cntr;

  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31670", SQLMSG, c_err_msg );
    return -1;
  }

	memcpy(ptr_st_xchngbook,ptr_i_st_xchngbook,sizeof(struct vw_xchngbook));

  *c_prcimp_flg = c_prcimpflg;  /** Ver 4.0 **/
  *c_mtm_flg = c_fxb_ip[0];
	rtrim(c_lst_act_ref_tmp);		/***  Ver 5.3 ***/
	strcpy(c_lst_act_ref,c_lst_act_ref_tmp);		/***  Ver 5.3 ***/
  if(DEBUG_MSG_LVL_4)
  {
	 fn_userlog( c_ServiceName,"Last Activity ref is :%s:",c_lst_act_ref);   /*** Ver 5.3 ***/
  }
  return 0;
}

int fn_upd_Fordrbkack (struct vw_orderbook *ptr_st_orderbook,
                      char c_prcimp_flg,   /** Ver 4.0 **/
											char *c_lst_act_ref,		/** Ver 5.3 **/
                      char *c_ServiceName,
                      char *c_err_msg)
{


  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
  	fn_userlog(c_ServiceName,"Inside Function Upd Orderbk For FUT ACK");
  	fn_userlog(c_ServiceName,"Ack Time Is :%s:",ptr_st_orderbook->c_ack_tm);
  	fn_userlog(c_ServiceName,"PRV Ack Time Is :%s:",ptr_st_orderbook->c_prev_ack_tm);
		fn_userlog(c_ServiceName,"Operation Type Is ;%c:",ptr_st_orderbook->c_oprn_typ);
		fn_userlog( c_ServiceName,"Last Activity ref is :%s:",c_lst_act_ref);   /*** Ver 5.3 ***/
	}
	
	switch( ptr_st_orderbook->c_oprn_typ )
	{
		case UPDATE_STATUS_ACK_TM:

			EXEC SQL
  			UPDATE  fod_fo_ordr_dtls
  			SET     fod_ordr_stts = :ptr_st_orderbook->c_ordr_stts,
        			  fod_ord_ack_tm = to_date (:ptr_st_orderbook->c_prev_ack_tm,'dd-mon-yyyy hh24:mi:ss' ),
          			fod_lst_rqst_ack_tm = to_date (:ptr_st_orderbook->c_ack_tm,'dd-mon-yyyy hh24:mi:ss' ),
								fod_lst_act_ref = :c_lst_act_ref		/*** Ver 5.3 ***/
  			WHERE   fod_ordr_rfrnc= :ptr_st_orderbook->c_ordr_rfrnc;

				break;


		case  UPDATE_ORDER_MODIFICATION:

   	 	EXEC SQL
     		UPDATE  fod_fo_ordr_dtls
     		SET     fod_lmt_mrkt_sl_flg = :ptr_st_orderbook->c_slm_flg,
        	      fod_dsclsd_qty      = :ptr_st_orderbook->l_dsclsd_qty,
             		fod_ordr_type       = :ptr_st_orderbook->c_ord_typ,
             		fod_stp_lss_tgr     = :ptr_st_orderbook->l_stp_lss_tgr,
             		fod_ordr_tot_qty    = :ptr_st_orderbook->l_ord_tot_qty,
             		fod_lmt_rt          = :ptr_st_orderbook->l_ord_lmt_rt,
             		fod_ordr_stts       = :ptr_st_orderbook->c_ordr_stts,
             		fod_ordr_valid_dt   = to_date (:ptr_st_orderbook->c_valid_dt,'dd-mon-yyyy' ),
             		fod_trd_dt          = to_date (:ptr_st_orderbook->c_trd_dt, 'dd-mon-yyyy' ),
             		fod_lst_rqst_ack_tm = to_date (:ptr_st_orderbook->c_ack_tm,'dd-mon-yyyy hh24:mi:ss' ),
                fod_prcimpv_flg     = :c_prcimp_flg,   /*** Ver 4.0 **/
								fod_lst_act_ref 		= :c_lst_act_ref   /*** Ver 5.3 ***/
     		WHERE   fod_ordr_rfrnc      = :ptr_st_orderbook->c_ordr_rfrnc;
   		break;

	  case  UPDATE_XCHNG_RESP_DTLS:

   		EXEC SQL
     		UPDATE  fod_fo_ordr_dtls
     		SET     fod_ordr_stts       = :ptr_st_orderbook->c_ordr_stts,
        		    fod_cncl_qty        = :ptr_st_orderbook->l_can_qty,
             		fod_exprd_qty       = :ptr_st_orderbook->l_exprd_qty,
             		fod_trd_dt          = to_date (:ptr_st_orderbook->c_trd_dt,'dd-mon-yyyy' )
     		WHERE   fod_ordr_rfrnc= :ptr_st_orderbook->c_ordr_rfrnc;
   		break;

		case  UPDATE_XCHNG_RESP_DTLS_CA:

 	 		EXEC SQL
    		UPDATE  fod_fo_ordr_dtls
    		SET     fod_ordr_stts       = :ptr_st_orderbook->c_ordr_stts,
        		    fod_cncl_qty        = :ptr_st_orderbook->l_can_qty,
            		fod_exprd_qty       = :ptr_st_orderbook->l_exprd_qty,
            		fod_trd_dt          = to_date (:ptr_st_orderbook->c_trd_dt,'dd-mon-yyyy' ),
            		fod_lst_rqst_ack_tm = to_date (:ptr_st_orderbook->c_ack_tm,'dd-mon-yyyy hh24:mi:ss' ),
								fod_lst_act_ref 		= :c_lst_act_ref    /*** Added in Ver 5.3 ***/
    		WHERE   fod_ordr_rfrnc= :ptr_st_orderbook->c_ordr_rfrnc;

  	break;

		default:

    	strcpy( c_err_msg, "Invalid Operation Type" );
      fn_userlog ( c_ServiceName, "ERROR :%s:", c_err_msg );
			return -1;
      break;
  }

  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31675", SQLMSG, c_err_msg );
    return -1;
  }

  return 0;

}

int fn_upd_Fxchngbkack( struct vw_xchngbook *ptr_st_xchngbook,
                       char *c_ServiceName,
                       char *c_err_msg)
{
  EXEC SQL BEGIN DECLARE SECTION;
    varchar c_xchng_rmrks [ 256 ];
  EXEC SQL END DECLARE SECTION;

  strcpy ( ( char * )c_xchng_rmrks.arr, ptr_st_xchngbook->c_xchng_rmrks );
  SETLEN ( c_xchng_rmrks );

  if(DEBUG_MSG_LVL_3)   /*** Ver 3.7 ***/
  {
    fn_userlog(c_ServiceName,"Inside Function fn_upd_xchngbkack");
  	fn_userlog(c_ServiceName,"RMS Prcsd Flg Is :%c:",ptr_st_xchngbook->c_rms_prcsd_flg);
  	fn_userlog(c_ServiceName,"Order Reference Is :%s:",ptr_st_xchngbook->c_ordr_rfrnc);
  	fn_userlog(c_ServiceName,"Modification Counter Is :%ld:",ptr_st_xchngbook->l_mdfctn_cntr);
	}

  EXEC SQL
    UPDATE  fxb_fo_xchng_book
    SET  fxb_rms_prcsd_flg   = :ptr_st_xchngbook->c_rms_prcsd_flg,
         fxb_xchng_rmrks     = rtrim(fxb_xchng_rmrks)||'-'||:c_xchng_rmrks
    WHERE  fxb_ordr_rfrnc    = :ptr_st_xchngbook->c_ordr_rfrnc
    AND  fxb_mdfctn_cntr     = :ptr_st_xchngbook->l_mdfctn_cntr;

  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31680", SQLMSG, c_err_msg );
    return -1;
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


/*** Commented in Ver 4.3 *** Ver 3.6 starts here ***
int fn_prcs_trd(struct vw_orderbook *ptr_st_orderbook,
                char *c_ServiceName,
                char *c_err_msg)
{

    struct vw_tradebook st_tradebook;

  	char c_sltp_ord_rfrnc[19];
  	int   i_returncode = 0;
    short int si_null;

		MEMSET(c_sltp_ord_rfrnc);
		MEMSET(st_tradebook);

    if(DEBUG_MSG_LVL_3)   *** Ver 3.7 ***
    {
    		fn_userlog( c_ServiceName, "In fn_prcs_trd...............");
		}
  	if ( ptr_st_orderbook->c_prd_typ != SLTP_FUTUREPLUS )
  	{
  	EXEC SQL
    	DECLARE cur_trd CURSOR FOR
    	SELECT  ftd_clm_mtch_accnt,
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
    	FROM    ftd_fo_trd_dtls
    	WHERE   ftd_ordr_rfrnc = :ptr_st_orderbook->c_ordr_rfrnc
    	AND     ftd_rms_prcsd_flg = 'N';

  	EXEC SQL
    	OPEN  cur_trd;
  	}
  	else
  	{

     EXEC SQL
      SELECT
           fod_sltp_ordr_rfrnc
      INTO
           :c_sltp_ord_rfrnc
      FROM  FOD_FO_ORDR_DTLS
      WHERE fod_ordr_rfrnc = :ptr_st_orderbook->c_ordr_rfrnc;

      if(SQLCODE !=0)
      {
        fn_errlog ( c_ServiceName, "S31685", SQLMSG, c_err_msg );
        return -1;
      }

  	EXEC SQL
    	DECLARE cur_trd1 CURSOR FOR
    	SELECT  ftd_clm_mtch_accnt,
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
    	FROM	  ftd_fo_trd_dtls
    	WHERE   ftd_ordr_rfrnc  in ( :ptr_st_orderbook->c_ordr_rfrnc, :c_sltp_ord_rfrnc)
    	AND     ftd_rms_prcsd_flg = 'N';

  	EXEC SQL
    	OPEN  cur_trd1;
  	}

  	if ( SQLCODE != 0 )
  	{
    	fn_errlog ( c_ServiceName, "S31690", SQLMSG, c_err_msg );
    	if ( ptr_st_orderbook->c_prd_typ != SLTP_FUTUREPLUS )
    	{
    	EXEC SQL
      	CLOSE cur_trd;
    	}
    	else
    	{
      	EXEC SQL
      	CLOSE cur_trd1;
    	}
			return -1;
  	}

  	while (1)
  	{
    	if ( ptr_st_orderbook->c_prd_typ != SLTP_FUTUREPLUS )
    	{
    	EXEC SQL
      	FETCH cur_trd
      	INTO  :st_tradebook.c_cln_mtch_accnt,
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
             :st_tradebook.l_exctd_rt;
     }
     else
     { 
       EXEC SQL
       FETCH cur_trd1
       INTO  :st_tradebook.c_cln_mtch_accnt,
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
             :st_tradebook.l_exctd_rt;
     }

     if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
     {
       fn_errlog ( c_ServiceName, "S31695", SQLMSG, c_err_msg );
 
       if ( ptr_st_orderbook->c_prd_typ != SLTP_FUTUREPLUS )
       {
       EXEC SQL
         CLOSE cur_trd;
       }
       else
       {
         EXEC SQL
           CLOSE cur_trd1;
       }
			 return -1;
     }

     if ( SQLCODE == NO_DATA_FOUND )
     {
      if ( ptr_st_orderbook->c_prd_typ != SLTP_FUTUREPLUS )
      {
      EXEC SQL
        CLOSE cur_trd;
      }
      else
      {
        EXEC SQL
          CLOSE cur_trd1;
      }
       break;
     }

    *** Added for Order Routing ***
    fn_cpy_ddr ( st_tradebook.c_rout_crt );

    i_returncode = fn_acall_svc( c_ServiceName,
                                c_err_msg,
                                &st_tradebook,
                                "vw_tradebook",
                                sizeof( struct vw_tradebook),
                                TPNOREPLY,
                                "SFO_FUT_TCONF" );
    if ( i_returncode != SUCC_BFR )
    {
      fn_errlog ( c_ServiceName, "S31700", LIBMSG, c_err_msg );

      if ( ptr_st_orderbook->c_prd_typ != SLTP_FUTUREPLUS ) 
      {
      EXEC SQL
        CLOSE cur_trd;
      }
      else
      {
        EXEC SQL
          CLOSE cur_trd1;
      }
			return -1;
    }

  }

  if ( ptr_st_orderbook->c_prd_typ != SLTP_FUTUREPLUS )
  {
  EXEC SQL
    CLOSE cur_trd;
  }
  else
  {
    EXEC SQL
    CLOSE cur_trd1;
  }
  return 0;
}
*** Ver 3.6 Ends here ***/
