/******************************************************************************/
/*	Program	    			:	SFO_OPT_ACK                                           */
/*                                                                            */
/*  Input             : vw_xchngbook                                          */
/*                                                                            */
/*  Output            : vw_err_msg                                            */
/*                                                                            */
/*  Description       :	This service updates the order book, exchange book,   */
/*                      positions and the limits on getting Response from     */
/*                      the Exchange for Options orders. The various response */
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
/*  Log               : 1.1   17-dec-2007   Vidyayini Krish                   */
/*  Log               : 1.2w  23-Jul-2008   Wipro|Vaishali										*/
/*	Log								:	1.3		19-Jan-2009		Sandeep														*/
/*	Log								:	1.4		14-Sep-2009		Sandeep														*/
/*	Log								:	1.5		24-Sep-2009		Sandeep														*/
/*	Log								:	1.6		02-Sep-2010		Sandeep														*/
/*	Log								:	1.7		02-Feb-2011		Sandeep														*/
/*  Log               : 1.8   18-Sep-2013   Swati A.B                         */
/*  Log               : 1.9   20-Jan-2014   Sachin B.                         */
/*  Log               : 2.0   04-Jun-2014   Navina D./ Shailesh S.            */
/*  Log               : 2.1		20-Jun-2014   Bhushan 													*/
/*  Log               : 2.2		08-Aug-2014   Sachin B 													*/
/*  Log               : 2.3		07-Mar-2015   Ritesh Deolekar 		  						*/
/*  Log               : 2.5   03-Jul-2015   Ritesh Deolekar                   */
/*  Log								: 2.6		15-Sep-2015		Tanmay Warkhade										*/
/*  Log               : 2.7   14-Jan-2016   Tanmay Warkhade/Kishor B(Core Product)                   */
/*  Log               : 2.8   04-May-2016   Tanmay W.                         */
/*  Log               : 2.9   23-Aug-2016   Bhupendra Malik                   */
/*	Log								: 3.0   29-Aug-2016		Kishor B.													*/
/*  Log               : 3.2   07-Nov-2016   Kishor B.                         */
/*	Log								:	3.3		22-Nov-2016		Kishor B.													*/
/*	Log								:	3.4		07-Dec-2016		Kishor B.													*/
/*	Log								:	3.5		20-Jul-2017		Sachin Birje    									*/
/*  Log								:	3.6		24-Oct-2017		Mrinal Kishore                    */
/*  Log               : 3.7   04-Apr-2018   Mrinal Kishore                    */
/*  Log								: 3.8   16-Aug-2018		Bhushan Harekar                   */
/*  Log               : 3.9   03-Oct-2018   Suchita Dabir                     */
/*  Log               : 4.1   16-Jan-2019   Suchita Dabir                     */
/*  Log               : 4.2   01-Apr-2019   Parag Kanojia                     */
/*  Log               : 4.4   08-May-2019   Suchita Dabir                     */
/*  Log               : 4.5   08-May-2019   Suchita Dabir                     */
/*  Log               : 4.6   24-Jul-2019   Kiran Itkar                       */
/*  Log               : 4.7   30-Aug-2019   Suchita Dabir                     */
/*  Log               : 4.8   07-Jan-2020   Anand Dhopte                      */ 
/*  Log               : 4.9   06-May-2020   Navina D.                         */
/*  Log               : 5.0   30-Jan-2020   Suchita Dabir                     */
/*  Log               : 5.1   21-OCT-2020   Suchita Dabir                     */
/*	Log								: 5.2		02-Feb-2021		Kiran Itkar												*/
/*	Log								: 5.3		15-JUL-2021   Suchita Dabir									*/
/*	Log								: 5.5		16-Aug-2022   Suchita Dabir									*/
/*  Log 							: 5.6   07-Mar-2023   Vipul Sharma											*/
/*	Log  							: 5.7   01-Apr-2023   Bhavana  											*/
/******************************************************************************/
/*  1.0  -  New Release                                                       */
/*  1.1  -  IBM Changes                                                       */
/*  1.2w -  CRCSN13948-Order Log strengthening 																*/
/*  1.3	 -  Partial Execution & Cancellation Case Handeling                   */
/*	1.4	 -  Service Call Replaced By Function Call                            */
/*	1.5	 -	NVL Added While Selecting Ack Time In Case Of Rejection Case			*/ 
/*	1.6	 -  Cancellation Rejection Handling Changes	.													*/	
/*	1.7	 -  Elimination of ML Frmttr																					*/
/*  1.8  -  OptionPLUS handling                                               */
/*  1.9  -  Handling of new System squareoff type IMTM_SYS_SQUAREOFF and      */
/*          LMTNEG_SYS_SQUAREOFF.                                             */   
/*  2.0  -  Exception EOS for OptionPLUS                                      */
/*  2.1	 -	SLTP Order modification Remarks Changes														*/
/*  2.2	 -	FO Auto MTM Changes                   														*/
/*  2.3  -  character array c_sys_msg size changed from 4 to 11(Live Bug Fix) */
/*  2.5  -  Contract master & Trade Quote merger Changes (Ritesh Deolekar )   */
/*  2.6  -  OptionPLUS_MKT_TO_LIMIT 																					*/
/*  2.7  -  Bug Fix functional exit issue and core product lock               */
/*	2.8	 -  changes for modification ack and self cancellation in same leg 		*/
/*					for core and sltp products/Log Maintainance												*/
/*  2.9  -  MY EOS changes                                                    */
/*	3.0  -	fn_ors_rms  include                																*/
/*  3.2  - Functional Exit code for trade moved to ors_rms(Kishor B)          */
/*  3.3  - Maximum order Quantity Limit in F&O                                */	 
/*	3.4	 - Core Fix	& Fill Qty Fetch																					*/		
/*	3.5	 - Service call with TPNOTAN & TPNOREPLY option introduced            */
/*	3.6  - BankNifty client issue changes																			*/ 
/*  3.7  - Order Rejection Status Clash With My EOS Changes                   */
/*  3.8  - Order Expiry Issues																								*/
/*  3.9  - Double Margin Debit                                                */
/*  4.1  - Using traded quantity available in cancellation response packet    */
/*          for solicited cancellation response processing                    */
/*  4.2  - FO TAP last activity ref changes ( Parag K.)                       */
/*  4.4  - Handling of Margin Release on Order Rejection in                   */
/*          FPSL & OP due to Quantity Freeze ( Suchita Dabir )                */
/*  4.5  -  Handling for extra open position caused                           */
/*          by multiple triggers of exception EOS( Suchita Dabir )            */
/*  4.6  -  Physical delivery margin changes ( Kiran Itkar )                  */
/*  4.7  -  NVL handled for last act ref ( Suchita Dabir )                    */
/*  4.8  -  Array size increased to address memory issue                      */
/*  4.9  -  Remove SOVL check for Optionplus                                  */
/*  5.0  -  Handling for extra open position unsolicited cancellation(Suchita)*/
/*  5.1  -  CR_ISEC14_139243 Opls margin changes ( Suchita )                  */
/*	5.2	 -  Separate EOS for Stock and Index Underlyings in OptionPlus (Kiran)*/
/*  5.3  -  CR_ISEC04_132410 changes ( Suchita )                  */
/*  5.4  -  physical settlement minor changes                  ( Suchita )                  */
/*  5.5 -   Unmark Position on sq off order rejection LODM        - Suchita D.(16-Aug-2022)   */
/*  5.6  -  Changing fn_lock_usr to fn_lock_fno (Vipul Sharma)								*/
/*	5.7  -  real time margin reporting changes 			**/
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atmi.h>
#include <time.h>
#include <fml32.h>
#include <sqlca.h>
#include <fo.h>
#include <eba_to_ors.h>
#include <fo_view_def.h>
#include <fo_fml_def.h>
#include <fn_tuxlib.h>
#include <fn_log.h>
#include <fn_pos.h>
#include <fn_ddr.h>
#include <fn_session.h> /* Ver 1.1 */
#include <fml_rout.h> /* Ver 1.1 */
#include <fn_read_debug_lvl.h> /* Ver 1.1 */
#include <fn_ors_rms.h> /* Ver 3.0 */
#include <sys/time.h>  /* VER TOL : TUX on LINUX (Ravindra) */


EXEC SQL INCLUDE "table/fum_fo_undrlyng_mstr.h";    /*** Ver 1.8 ***/
EXEC SQL INCLUDE "table/exg_xchng_mstr.h";          /*** Ver 1.8 ***/

int fn_reftoord_forupd(struct vw_orderbook *ptr_st_orderbook,
                       char *c_ServiceName,
                       char *c_err_msg);

int fn_ref_to_omd ( struct vw_xchngbook *st_xchngbook,
                    struct vw_xchngbook *ptr_st_xchngbook,
                    char *c_mtm_flg,   /** Ver 2.2 **/
										char *c_lst_act_ref,           /** Ver 4.2 **/
                    char *c_ServiceName,
                    char *c_err_msg);

int fn_upd_xchngbkack( struct vw_xchngbook *ptr_st_xchngbook,
                       char *c_ServiceName,
                       char *c_err_msg);

int fn_upd_ordrbkack (struct vw_orderbook *ptr_st_orderbook,
											char *c_lst_act_ref,           /** Ver 4.2 **/
                      char *c_ServiceName,
                      char *c_err_msg);

/*** int fn_prcstrd( struct vw_orderbook *st_orderbook,
                     char *c_ServiceName,
                      char *c_err_msg); *** Commented in Ver 3.2 ***** Ver 2.7 *****/

void fn_gettime(void);

char c_time[12];


void SFO_OPT_ACK( TPSVCINFO *rqst )
{
	char	c_ServiceName[33];
	char	c_err_msg[256];
  char  c_imtm_remarks[256];
	char  c_command1[200];     /* ver 3.6 */  /*** array size changed to 200 from 100 in Ver 4.8 ***/
	char	c_mail_remarks[100]; /* ver 3.6 */
	int		i_trnsctn;
	int		i_returncode;
	int		i_ip_len;
	int		i_op_len;
  long  l_ftd_trd_qty = 0;  /* Ver 1.3 */ /*** Initialized in ver 4.1 ***/
/** commented in ver 4.1  long  l_ftd_qty;      * Ver 1.3 */
  long l_max_allowed_qty = 0 ; /*** Ver 3.3 ***/
  int   i_canrej_flag = 1; /*** ver 3.9 ***/ 
  
  char  c_tag [ 256 ];

	/*** Ver 1.8 Starts here ***/
  char c_sltp_ord_rfrnc[19];
  char c_fc_flg = '\0';
  char c_ordr_stts = '\0';
  char c_narration_id[4];
  char c_svc_nm[20];
  char c_slm_flg;
  char c_remarks[50];
  char c_ord_typ;
  char c_ordr_rfrnc[19];      /*** Ver 2.6 ***/
  char c_sltp_ord_rfrnc_lck[2][19]; /*** Ver 2.6 ***/
  char c_ordr_rfrnc_lck[2][19];     /*** Ver 2.6 ***/
  char c_ordr_rfrnc_core[19];       /*** Ver 2.7 ***/

  /*** ver 5.3 starts **/
  char c_ordr_flow_phy = '\0';
  char c_prvsnl_opnpos_flow = '\0';
  long l_prvsnl_pending_qty = 0;
  char c_sub_mode = '\0';
  char c_ref_rmrks_phy[133] ="\0";
  /*** ver 5.3 ends ***/

  char c_ref_rmrks[133] = "\0"; /*** Ver 5.1 ***/
  char  c_tmp_rmrks [ 133 ] = "\0";  /*** Ver 5.1 ***/

  double d_comp1_mrgn = 0.0;  /*** Ver 5.1 ***/
  double d_comp1_pl = 0.0;    /*** Ver 5.1 ***/
  double d_xchng_amt = 0.0;   /*** Ver 5.1 ***/
  double d_prem_amt = 0.0;    /*** Ver 5.1 ***/
  double d_spn_mrgn = 0.0;    /*** Ver 5.1 ***/
  double d_expr_mrgn = 0.0;   /*** Ver 5.1 ***/

  char c_fop_mtm_flg='\0'; /*** ver 5.5 **/
  struct vw_xchngbook st_cvr_xchngbk;   /*** Ver 2.6 ***/
 	char c_cvr_prd_typ = '\0';   					/*** Ver 2.6 ***/
  int i_status = 0;             /*** Ver 2.6 ***/
  int i_ferr[7];              /*** Ver 2.6 ***/
  int i;                  /*** Ver 2.6 ***/
  char c_mtm_sltp_req = '\0';
  char c_ordr_flw = '\0';
  char c_spl_flg = '\0';
  char c_frsh_ordr_stts = '\0';
  char c_sys_msg[11] = "\0";  /* Ver 2.3 Array size increase from 4 to 11 */  
	char c_ord_flw = '\0';
	char c_cover_ref[19] = "\0"; /*** Ver 2.0 ***/
	char c_ipord_ref[19] = "\0"; /*** Ver 2.8 ***/
  char c_mtm_flg= '\0'; /*** Ver 2.2 ***/
	char c_rqst_typ= '\0';   /*** Ver 4.6 ***/
	char c_span_flg= 'N';    /*** Ver 4.6 ***/

  int   i_cumopn_val_flg = 0;
  int   i_max_val_flg    = 0;
	int   i_commit_flg = 0; 
  long l_cvr_ord_lmt_rt = 0; /*** Ver 2.6 ***/
  long l_fsh_ord_lmt_rt = 0; /*** Ver 2.6 ***/
  long  l_exctd_qty = 0;
  long  l_frsh_xtd_qty = 0;
  long  li_tot_exec_qty = 0;
  long  l_open_qty = 0;
  long  li_dsclsd_qty = 0;
  long  li_stp_lss_tgr_prc = 0;
  long int li_lmt_rt = 0;
  long  l_exec_qty = 0;
  long  l_temp_stp_lss_tgr= 0;
  long  l_lot_sz = 0;
  long  l_new_max_qty = 0;
  long  l_new_lots= 0;
  /** long  l_ftd_ord_exe_qty = 0; Commented in Ver 3.4  **** Ver 2.8 ****/
  long 	l_ors_msg_typ=0; /*** Ver 2.8 ***/

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
  double d_frm_diffrnc_mrgn= 0.0;
  double d_lst_trdd_prc = 0.0;
  double d_tot_open_val = 0.0;
  double d_optpls_ordr_val = 0.0;
  /*** Ver 1.8 Ends here ***/

	/*** Ver 2.6 Start***/
  MEMSET(st_cvr_xchngbk);
  MEMSET(c_sltp_ord_rfrnc_lck);
  MEMSET(c_ordr_rfrnc_lck);
  /*** Ver 2.6 End***/
  MEMSET(c_mail_remarks);	/*** Ver 3.6 ***/
	MEMSET(c_command1);			/*** Ver 3.6 ***/
	char c_lst_act_ref [22] ;   /*** Ver 4.2 ***/
  MEMSET(c_lst_act_ref);  		/*** Ver 4.2 ***/


	EXEC SQL BEGIN DECLARE SECTION;
		struct vw_xchngbook *ptr_st_xchngbook;
		struct vw_xchngbook st_xchngbook;
		struct vw_orderbook st_orderbook;
		struct vw_tradebook st_tradebook;
		struct vw_pstn_actn st_pstn_actn;
		short int si_null;
	EXEC SQL END DECLARE SECTION;
	MEMSET(st_orderbook);   /*** Ver 2.6 ***/
	
	 /*** FBFR32 *ptr_fml_Sbuf; *** Ver 2.6 ***Commented in Ver 3.4 ***/
	struct vw_err_msg *ptr_st_err_msg;

	ptr_st_xchngbook = ( struct vw_xchngbook *)rqst->data;
	strcpy( c_ServiceName, rqst->name );
  INITDBGLVL(c_ServiceName);	

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
		/*** tpfree ( ( char * ) ptr_st_xchngbook ); Commented in Ver 3.4 ***/
		tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
	}

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog( c_ServiceName, "ptr_st_xchngbook->c_oprn_typ:%c:",
                                             ptr_st_xchngbook->c_oprn_typ );
	}

	strcpy( st_orderbook.c_ordr_rfrnc, ptr_st_xchngbook->c_ordr_rfrnc );
	st_orderbook.c_oprn_typ = FOR_UPDATE;

	
	i_ip_len = sizeof ( struct vw_orderbook );
	i_op_len = sizeof ( struct vw_orderbook );

  /*** Added for Order Routing ***/
	fn_cpy_ddr ( st_orderbook.c_rout_crt );

	/***	Commented In Ver 1.4
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
	******************************************************/


  /*** Ver 2.6 starts here ***/

  EXEC SQL
  SELECT
      fod_prdct_typ,
			DECODE(FOD_EOS_FLG,'M','MY_EOS','N','EOS','E','EXCPT_EOS','X'), /** Ver 2.9 **/
			DECODE(FOD_EOS_FLG,'M',FOD_CHANNEL,'E','SYS','N','SYS',FOD_CHANNEL), /** 2.9 **/
      FOD_ORDR_FLW /** ver 5.3 ***/ 
  INTO
      :c_cvr_prd_typ,
			:c_sys_msg,  /** Ver 2.9 **/
			:st_pstn_actn.c_channel, /*** Ver 2.9 **/
      :c_ordr_flow_phy /** ver 5.3 ***/
  FROM  FOD_FO_ORDR_DTLS
  WHERE fod_ordr_rfrnc = :st_orderbook.c_ordr_rfrnc;
  if(SQLCODE !=0)
  {
    fn_errlog ( c_ServiceName, "S31015", SQLMSG, ptr_st_err_msg->c_err_msg );
    fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
    tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
  }

	rtrim(c_sys_msg); /*** Ver 2.9 ***/
	rtrim(st_pstn_actn.c_channel); /*** Ver 2.9 ***/
	if(DEBUG_MSG_LVL_0) /** Ver 2.8 **/
	{
  	fn_userlog( c_ServiceName, "In SFO_OPT_ACK :: c_cvr_prd_typ : %c for st_orderbook.c_ordr_rfrnc :%s:",c_cvr_prd_typ, st_orderbook.c_ordr_rfrnc); /*** Corrected service name from SFO_FUT_ACK to SFO_OPT_ACK Ver 4.8 ***/
	}

  if (c_cvr_prd_typ == OPTIONPLUS)
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
      fn_errlog ( c_ServiceName, "S31020", SQLMSG, ptr_st_err_msg->c_err_msg );
      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
      tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
    }

  if(DEBUG_MSG_LVL_0) /** Ver 2.8 **/
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
    FOR   UPDATE OF FOD_ORDR_RFRNC NOWAIT;

		    if ( SQLCODE != 0)
    {
      if (SQLCODE == -54)
      {
        fn_userlog( c_ServiceName, "Resource busy and acquire with NOWAIT specified or timeout expired");
      }
      fn_errlog ( c_ServiceName, "S31025", SQLMSG, ptr_st_err_msg->c_err_msg );
      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
    }
    else
    {
      fn_userlog( c_ServiceName, "Sucessfully acquired FOR UPDATE NOWAIT on :%s: and :%s:",st_orderbook.c_ordr_rfrnc,c_sltp_ord_rfrnc);
    }

    fn_userlog( c_ServiceName, "After NOWAIT on :%s: and :%s:",st_orderbook.c_ordr_rfrnc,c_sltp_ord_rfrnc);
  }

  /*************** Ver 2.6 Ends here *******************/
  else /*** Else loop added in Ver 2.7 ***/
  {
	
	if(DEBUG_MSG_LVL_3) /** Ver 2.8 **/
  {
    fn_userlog( c_ServiceName, "st_orderbook.c_ordr_rfrnc :%s:",st_orderbook.c_ordr_rfrnc);
	}

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
      fn_errlog ( c_ServiceName, "S31030", SQLMSG, ptr_st_err_msg->c_err_msg );
      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
    }
    else
    {
      fn_userlog( c_ServiceName, "Sucessfully acquired FOR UPDATE NOWAIT on :%s:",st_orderbook.c_ordr_rfrnc);
    }

    fn_userlog( c_ServiceName, "After NOWAIT on :%s:",st_orderbook.c_ordr_rfrnc);
  }

	i_returncode = fn_reftoord_forupd(&st_orderbook,c_ServiceName,c_err_msg);
	if ( i_returncode != 0)
	{
		fn_errlog ( c_ServiceName, "S31035", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}


	fn_gettime();
	
	
	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog( c_ServiceName, "ORS2-RD Function fn_reftoord_forupd Time :%s:",c_time);
		fn_userlog( c_ServiceName, "c_ordr_stts  :%c:", st_orderbook.c_ordr_stts );
	}

	if ( st_orderbook.c_ordr_stts == REQUESTED )
	{
		ptr_st_xchngbook->c_oprn_typ = WITHOUT_ORS_MSG_TYP;
	}
	else
	{
		ptr_st_xchngbook->c_oprn_typ = WITH_ORS_MSG_TYP;
	}

	i_ip_len = sizeof ( struct vw_xchngbook );
	i_op_len = sizeof ( struct vw_xchngbook );

  /*** Added for Order Routing ***/
	fn_cpy_ddr ( ptr_st_xchngbook->c_rout_crt );

	/*** Commented In Ver 1.4
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
	********************************************************/



	i_returncode = fn_ref_to_omd ( ptr_st_xchngbook,&st_xchngbook,&c_mtm_flg,c_lst_act_ref,c_ServiceName,c_err_msg); /* Ver 2.2 *c_mtm_flg*/	/*** Added c_lst_act_ref in Ver 4.2 ***/

	if ( i_returncode != 0)
	{
		fn_errlog ( c_ServiceName, "S31040", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

  /** ver 5.3  starts ***/
  strcpy( c_ref_rmrks_phy, st_xchngbook.c_xchng_rmrks);
  rtrim(c_ref_rmrks_phy);
  /*** ver 5.3 ends ***/

	fn_gettime();
  if(DEBUG_MSG_LVL_3)   /** Ver 2.8 **/
  {
 	 fn_userlog( c_ServiceName, "ORS2-RD Function fn_ref_to_omd Time :%s:",c_time);
	}
	/*i_returncode = fn_lock_usr( c_ServiceName,  st_orderbook.c_cln_mtch_accnt ); Commented in Ver 5.6*/
  i_returncode = fn_lock_fno( c_ServiceName,  st_orderbook.c_cln_mtch_accnt ); /* Added in Ver 5.6 */
	if ( i_returncode != 0 )
	{
		fn_errlog ( c_ServiceName, "S31045", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

	if ( st_xchngbook.c_rms_prcsd_flg == 'P' )
	{
		fn_userlog( c_ServiceName, "Record already processed");
		strcpy ( ptr_st_err_msg->c_err_msg, "Record already processed" );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );

	  /*** Commented in Ver 3.2
  i_returncode = fn_prcstrd(&st_orderbook,c_ServiceName,c_err_msg);        **** Ver 2.7 ****
  ************/

  /*** Ver 3.2 Starts Here ***/
  i_returncode = fn_prcs_trd(c_ServiceName,c_err_msg,st_orderbook.c_prd_typ,st_orderbook.c_ordr_rfrnc);
  /*** Ver 3.2 Ends Here ***/

	if ( i_returncode != 0 )
  {
    fn_errlog ( c_ServiceName, "S31050", LIBMSG, c_err_msg );
  }

    tpfree ( ( char * ) ptr_st_err_msg );  /** Added in Ver 3.4 **/
    tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_xchngbook, 0, 0 );
	}

	/*** Ver 1.3 Starts ***/

  if(ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_ACPT)
  {
  	EXEC  SQL
      SELECT  nvl(sum(FTD_EXCTD_QTY),0)
      INTO    :l_ftd_trd_qty
      FROM    FTD_FO_TRD_DTLS
      WHERE   FTD_ORDR_RFRNC =:ptr_st_xchngbook->c_ordr_rfrnc
      AND     FTD_RMS_PRCSD_FLG = 'P'; /** ver 4.1 **/

    if(SQLCODE !=0 && SQLCODE != NO_DATA_FOUND )
    {
      fn_errlog ( c_ServiceName, "S31055", SQLMSG, ptr_st_err_msg->c_err_msg );
    	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ); /*** Added in Ver 3.4 ***/
      tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
    }

    /***  comment starts for version 4.1 ***
    EXEC  SQL
      SELECT  nvl(sum(FTD_EXCTD_QTY),0)
      INTO    :l_ftd_qty
      FROM    FTD_FO_TRD_DTLS
      WHERE   FTD_ORDR_RFRNC =:ptr_st_xchngbook->c_ordr_rfrnc
      AND     FTD_RMS_PRCSD_FLG = 'N';

    if(SQLCODE !=0 && SQLCODE != NO_DATA_FOUND )
    {
      fn_errlog ( c_ServiceName, "S31060", SQLMSG, ptr_st_err_msg->c_err_msg );
    	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ); *** Added in Ver 3.4 ***
      tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
    }
     *** comment ends for version 4.1 ***/

  	if(DEBUG_MSG_LVL_3)   /** Ver 2.8 **/
  	{
  	  fn_userlog(c_ServiceName,"Total Traded Quantity Is :%ld:",l_ftd_trd_qty);
      /* fn_userlog(c_ServiceName,"Unprocessed Trade Quantity Is :%ld:",l_ftd_qty); commented in ver 4.1 **/
		}
  }

  /*** Ver 1.3 Ends ***/

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

	if ( st_orderbook.c_prd_typ != OPTIONPLUS ) /*** if condition added in Ver 1.8 ***/
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
               ( st_orderbook.c_spl_flg == IMTM_SYS_SQUAREOFF ) ||    /** Ver 1.9 **/
               ( st_orderbook.c_spl_flg == LMTNEG_SYS_SQUAREOFF ) ||    /** Ver 1.9 **/
							 ( st_orderbook.c_spl_flg == PHYSICAL_DLVRY_SQOFF ) )    /** Ver 4.6 **/
          {
            fn_userlog(c_ServiceName,"I'am HERE 1" );

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

					/** Commented In Ver 1.3 
					st_pstn_actn.l_new_qty = 0;
					**/
					/** Commented In Ver 1.3
					st_pstn_actn.l_exec_qty  = st_orderbook.l_exctd_qty;
					***/

					if(ptr_st_xchngbook->l_ors_msg_typ	==	ORS_CAN_ORD_ACPT)			/***	Ver	1.6	***/
					{
            /** st_pstn_actn.l_exec_qty  = l_ftd_trd_qty; * Ver 1.3 */ /** commented in ver 4.1 **/

            /*** st_pstn_actn.l_new_qty = l_ftd_qty; * Ver 1.3 */ /** commented in ver 4.1 **/

            /*** ver 4.1 starts ***/
            st_pstn_actn.l_exec_qty  = ptr_st_xchngbook->l_xchng_can_qty;
            st_pstn_actn.l_new_qty   = ptr_st_xchngbook->l_xchng_can_qty - l_ftd_trd_qty;
            if(DEBUG_MSG_LVL_0)
            {
                 fn_userlog(c_ServiceName,"In cancellation acceptance st_pstn_actn.l_exec_qty :%ld: and unprocessed traded quantity is :%ld:",st_pstn_actn.l_exec_qty,st_pstn_actn.l_new_qty);
            }
            /*** ver 4.1 ends ***/
					}
					else if(ptr_st_xchngbook->l_ors_msg_typ  == ORS_CAN_ORD_RJCT )			/***  Ver 1.6 ***/
					{
						st_pstn_actn.l_new_qty = 0;
						st_pstn_actn.l_exec_qty  = st_orderbook.l_exctd_qty;
					}

					st_pstn_actn.l_new_rt  = 0;
					st_pstn_actn.l_exec_rt  = 0;

					break;

		case  ORS_SPD_ORD_CNCL:
		case  ORS_2L_ORD_CNCL:
		case  ORS_3L_ORD_CNCL:
		case  ORS_IOC_CAN_ACPT:
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

	i_returncode = fn_call_svc( c_ServiceName,
                              c_err_msg,
                              &st_pstn_actn,
                              &st_pstn_actn,
                              "vw_pstn_actn",
                              "vw_pstn_actn",
                              i_ip_len,
                              i_op_len,
                              0,
                              "SFO_UPD_OPT_POS" );
	if ( i_returncode != SUCC_BFR )
	{
		fn_errlog ( c_ServiceName, "S31065", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

  if ( st_pstn_actn.c_mtm_flag == FOR_REMARKS_ENTRY )
  {
    switch ( ptr_st_xchngbook->l_ors_msg_typ )
    {
      case  ORS_NEW_ORD_ACPT:
          sprintf(c_imtm_remarks,"RECEIVED SQUAREOFF CONFIRMATION FOR [%s]",
                                            ptr_st_xchngbook->c_ordr_rfrnc );
          i_returncode = fn_ins_opt_rep_tbl ( c_ServiceName,
                                          		c_err_msg,
                                          		&st_pstn_actn,
                                          		c_imtm_remarks );
          if ( i_returncode != 0 )
          {
            fn_errlog ( c_ServiceName, "S31070", LIBMSG, c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
            tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
          }
          break;

      case  ORS_NEW_ORD_RJCT:
          sprintf(c_imtm_remarks,"RECEIVED SQUAREOFF REJECTION FOR [%s]",
                                            ptr_st_xchngbook->c_ordr_rfrnc );
          i_returncode = fn_ins_opt_rep_tbl ( c_ServiceName,
                                          		c_err_msg,
                                          		&st_pstn_actn,
                                          		c_imtm_remarks );
          if ( i_returncode != 0 )
          {
            fn_errlog ( c_ServiceName, "S31075", LIBMSG, c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
            tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
          }
          break;

      case  ORS_CAN_ORD_ACPT:
          sprintf(c_imtm_remarks,"RECEIVED CANCELLATION ACCEPTANCE FOR [%s]",
                                            ptr_st_xchngbook->c_ordr_rfrnc );
          i_returncode = fn_ins_opt_rep_tbl ( c_ServiceName,
                                          		c_err_msg,
                                          		&st_pstn_actn,
                                          		c_imtm_remarks );
          if ( i_returncode != 0 )
          {
            fn_errlog ( c_ServiceName, "S31080", LIBMSG, c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
            tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
          }
          break;

			case  ORS_SPD_ORD_CNCL:
			case  ORS_2L_ORD_CNCL:
			case  ORS_3L_ORD_CNCL:
			case  ORS_IOC_CAN_ACPT:
          sprintf(c_imtm_remarks,"RECEIVED AUTO CANCELLATION ACCEPTANCE FOR [%s]", ptr_st_xchngbook->c_ordr_rfrnc );
          i_returncode = fn_ins_opt_rep_tbl ( c_ServiceName,
                                          		c_err_msg,
                                          		&st_pstn_actn,
                                          		c_imtm_remarks );
          if ( i_returncode != 0 )
          {
            fn_errlog ( c_ServiceName, "S31085", LIBMSG, c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
            tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
          }
          break;

      case  ORS_CAN_ORD_RJCT:
          sprintf(c_imtm_remarks,"RECEIVED CANCELLATION REJECTION FOR [%s]",
                                            ptr_st_xchngbook->c_ordr_rfrnc );
          i_returncode = fn_ins_opt_rep_tbl ( c_ServiceName,
                                          		c_err_msg,
                                          		&st_pstn_actn,
                                          		c_imtm_remarks );
          if ( i_returncode != 0 )
          {
            fn_errlog ( c_ServiceName, "S31090", LIBMSG, c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
            tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
          }
          break;
    }
  }
	} /*** Ver 1.8 if condition ends ***/	


	switch ( ptr_st_xchngbook->l_ors_msg_typ )
	{
		case	ORS_NEW_ORD_ACPT:
					if (	( st_orderbook.c_ordr_stts == QUEUED ) ||
								( st_orderbook.c_ordr_stts == 'F' )				)
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


					strcpy( st_orderbook.c_prev_ack_tm,ptr_st_xchngbook->c_entry_dt_tm );
					st_orderbook.c_oprn_typ = UPDATE_STATUS_ACK_TM;
					break;

		case	ORS_MOD_ORD_ACPT:
					if ( st_orderbook.l_exctd_qty == 0 )
					{
						if (	( st_orderbook.c_ordr_stts == QUEUED ) ||
									( st_orderbook.c_ordr_stts == 'F' )				)
						{
							st_orderbook.c_ordr_stts = ORDERED;
						}
					}
					else if ( st_orderbook.l_exctd_qty < st_orderbook.l_ord_tot_qty )
					{
						if (	( st_orderbook.c_ordr_stts == QUEUED ) ||
									( st_orderbook.c_ordr_stts == 'F' )				)
						{
							st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED;
						}
					}
					else
					{
						st_orderbook.c_ordr_stts = EXECUTED;
					}
			
					if(st_orderbook.c_slm_flg == 'S')  /*** If Condition Added in Ver 2.1 ***/
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

					/***  Commented in Ver 2.1 **
					*Ver 1.2w Order Log CR*
					strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Modification Accepted");
					***/
					break;

		case	ORS_MOD_ORD_RJCT:

					/** Ver 3.8 Starts **/
          if(DEBUG_MSG_LVL_0)
          {
            fn_userlog(c_ServiceName, "st_orderbook.l_can_qty:%ld:", st_orderbook.l_can_qty);
            fn_userlog(c_ServiceName, "st_orderbook.l_exprd_qty :%ld:",st_orderbook.l_exprd_qty); 
          }
					/** Ver 3.8 Ends **/

					if ( st_orderbook.l_exctd_qty == 0 )
					{
						/** if ( (st_orderbook.l_ord_tot_qty - st_orderbook.l_can_qty) > 0 ) ** Commented in Ver 3.8 **/
						if ( (st_orderbook.l_ord_tot_qty - (st_orderbook.l_can_qty + st_orderbook.l_exprd_qty)) > 0 ) /*** added in Ver 3.8 ***/
						{
							st_orderbook.c_ordr_stts = ORDERED;
						}
						/** Ver 3.8 Starts **/
						else if( st_orderbook.l_exprd_qty > 0 )
						{
							st_orderbook.c_ordr_stts = EXPIRED;	
						}
						/** Ver 3.8 Ends **/
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
						/** Ver 3.8 Starts **/
						else if( (st_orderbook.l_exctd_qty + st_orderbook.l_exprd_qty) == st_orderbook.l_ord_tot_qty )
						{
							st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED_AND_EXPIRED;
						}
						/** Ver 3.8 Ends **/
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
					/*****
					st_orderbook.l_can_qty = 0;
					******/
					/** st_orderbook.l_exprd_qty = 0; ** Commented in Ver 3.8 **/
					st_orderbook.c_oprn_typ = UPDATE_XCHNG_RESP_DTLS;
					
					/*Ver 1.2w Order Log CR*/
					strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Modification Rejected");

					break;

    case  ORS_SPD_ORD_CNCL:
    case  ORS_2L_ORD_CNCL:
    case  ORS_3L_ORD_CNCL:
    case  ORS_IOC_CAN_ACPT:
		case  ORS_ORD_FRZ_CNCL:
          st_orderbook.l_can_qty = st_orderbook.l_ord_tot_qty - ptr_st_xchngbook->l_xchng_can_qty; /** ver 5.0 **/
          if((st_orderbook.l_exctd_qty + st_orderbook.l_can_qty) == st_orderbook.l_ord_tot_qty) /* ver 5.0 **/ 
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
          else /*** ver 5.0 starts **/
          {
            /*** No trade is processed yet or is in-flight and cancellation arrived ***/
            if ( st_orderbook.l_exctd_qty == 0 )
            {
              st_orderbook.c_ordr_stts = ORDERED;
            }
            else /** Some trades are processed and some are not yet processed or are in-flight and cancellation arrived ***/
            {
              st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED;
            }

            if(DEBUG_MSG_LVL_0)
            {
              fn_userlog( c_ServiceName,"In Cancellation Acceptance FINAL ORDER STATUS IS :%c:",st_orderbook.c_ordr_stts);
            }
          }  /*** ver 5.0 ends ***/
          strcpy( st_orderbook.c_trd_dt , st_xchngbook.c_mod_trd_dt);
          /** st_orderbook.l_can_qty = st_orderbook.l_ord_tot_qty -
                                   ptr_st_xchngbook->l_xchng_can_qty; commented in ver 5.0 **/
          strcpy( st_orderbook.c_ack_tm , ptr_st_xchngbook->c_ack_tm);
          st_orderbook.l_exprd_qty = 0;
          st_orderbook.c_oprn_typ = UPDATE_XCHNG_RESP_DTLS_CA;
          break;

		case	ORS_CAN_ORD_ACPT:

          /*** ver 4.1 starts ***/
          /*** Even if trade is not processed or is in-flight , we'll get correct cancelled quantity ***/
          st_orderbook.l_can_qty = st_orderbook.l_ord_tot_qty - ptr_st_xchngbook->l_xchng_can_qty;
          if(DEBUG_MSG_LVL_0)
          {
             fn_userlog( c_ServiceName,"In Cancellation Acceptance l_xchng_can_qty is :%ld: and l_can_qty is :%ld: and st_orderbook.l_exctd_qty is :%ld: and st_orderbook.c_ordr_stts is :%c:",ptr_st_xchngbook->l_xchng_can_qty,st_orderbook.l_can_qty,st_orderbook.l_exctd_qty,st_orderbook.c_ordr_stts );
          }
          /*** ver 4.1 ends ***/

          if( (st_orderbook.l_exctd_qty + st_orderbook.l_can_qty) == st_orderbook.l_ord_tot_qty )/* ver 4.5 If condition added */
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
          else /*** ver 4.5 starts **/
          {
            /*** No trade is processed yet or is in-flight and cancellation arrived ***/
            if ( st_orderbook.l_exctd_qty == 0 )
            {
              st_orderbook.c_ordr_stts = ORDERED;
            }
            else /** Some trades are processed and some are not yet processed or are in-flight and cancellation arrived ***/
            {
              st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED;
            }

            if(DEBUG_MSG_LVL_0)
            {
              fn_userlog( c_ServiceName,"In Cancellation Acceptance FINAL ORDER STATUS IS :%c:",st_orderbook.c_ordr_stts);
            }
          }  /*** ver 4.5 ends ***/
					strcpy( st_orderbook.c_trd_dt , st_xchngbook.c_mod_trd_dt);

				  /***  Commented In Ver 1.3
					st_orderbook.l_can_qty = st_orderbook.l_ord_tot_qty - 
                                   st_orderbook.l_exctd_qty;
					***/
          /** st_orderbook.l_can_qty = st_orderbook.l_ord_tot_qty - l_ftd_trd_qty;  * Ver 1.3 */ /** commented in ver 4.1 **/
					strcpy( st_orderbook.c_ack_tm , ptr_st_xchngbook->c_ack_tm);
					st_orderbook.l_exprd_qty = 0;
					st_orderbook.c_oprn_typ = UPDATE_XCHNG_RESP_DTLS;

					/*Ver 1.2w Order Log CR*/
					strcpy(ptr_st_xchngbook->c_xchng_rmrks,"Cancellation Accepted");
					break;

		case	ORS_CAN_ORD_RJCT:

					/* Ver 3.7 Starts */
          if(DEBUG_MSG_LVL_0)
          {
						fn_userlog(c_ServiceName, "st_orderbook.l_can_qty:%ld:", st_orderbook.l_can_qty);
						fn_userlog(c_ServiceName, "st_orderbook.l_exprd_qty :%ld:",st_orderbook.l_exprd_qty); /* Ver 3.8 */
					}
					/* Ver 3.7 Ends */
					if ( st_orderbook.l_exctd_qty == 0 )
					{
		/** if ( (st_orderbook.l_ord_tot_qty - st_orderbook.l_can_qty) > 0 ) * Ver 3.7 ** Commented in Ver 3.8 **/
						if ( (st_orderbook.l_ord_tot_qty - (st_orderbook.l_can_qty + st_orderbook.l_exprd_qty)) > 0 ) /*** added in Ver 3.8 ***/
						{
							st_orderbook.c_ordr_stts = ORDERED;
						}
						/** Ver 3.8 Starts **/
						else if( st_orderbook.l_exprd_qty > 0 )
						{
							st_orderbook.c_ordr_stts = EXPIRED;
						}
						/** Ver 3.8 Ends **/
						/* Ver 3.7 Starts */
            else
            {
              st_orderbook.c_ordr_stts = CANCELLED;
            }
            /* Ver 3.7 Ends */
					}
					else if ( st_orderbook.l_exctd_qty < st_orderbook.l_ord_tot_qty )
					{
						/* Ver 3.7 Starts */
            if ( (st_orderbook.l_exctd_qty + st_orderbook.l_can_qty) == st_orderbook.l_ord_tot_qty )
            {
              st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED_AND_CANCELLED;
            }
						/** Ver 3.8 Starts **/
						else if ( (st_orderbook.l_exctd_qty + st_orderbook.l_exprd_qty) == st_orderbook.l_ord_tot_qty )
						{
							st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED_AND_EXPIRED;
						}	
						/** Ver 3.8 Ends **/
            else
            {
            /* Ver 3.7 Ends */
						st_orderbook.c_ordr_stts = PARTIALLY_EXECUTED;
						}     /* Ver 3.7 */
					}
					else
					{
						st_orderbook.c_ordr_stts = EXECUTED;
					}

					strcpy( st_orderbook.c_trd_dt , st_xchngbook.c_mod_trd_dt);
					strcpy( st_orderbook.c_ack_tm , ptr_st_xchngbook->c_ack_tm);
					/* st_orderbook.l_can_qty = 0;	***ommented in Ver 3.7 ***/
					/** st_orderbook.l_exprd_qty = 0; ** Commented in Ver 3.8 **/
					st_orderbook.c_oprn_typ = UPDATE_XCHNG_RESP_DTLS;
				
					/*Ver 1.2w Order Log CR*/
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

	/*** Commented In Ver 1.4 
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
	****************************************************/


	i_returncode = fn_upd_ordrbkack(&st_orderbook,c_lst_act_ref,c_ServiceName,c_err_msg);		/*** Added c_lst_act_ref in Ver 4.2 ***/

	if ( i_returncode != 0)
	{
		fn_errlog ( c_ServiceName, "S31095", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}

	fn_gettime();
  if(DEBUG_MSG_LVL_3)   /** Ver 2.8 **/
  {
		fn_userlog( c_ServiceName, "ORS2-RD Function fn_upd_ordrbkack Time :%s:",c_time);
	}
	ptr_st_xchngbook->c_rms_prcsd_flg = 'P';
	ptr_st_xchngbook->c_oprn_typ = UPDATE_RMS_PRCSD_FLG;

	i_ip_len = sizeof ( struct vw_xchngbook );
	i_op_len = sizeof ( struct vw_xchngbook );
				
  /*** Added for Order Routing ***/
	fn_cpy_ddr ( ptr_st_xchngbook->c_rout_crt );

	/***	Commented In Ver 1.4
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
	******************************************************/

	i_returncode = fn_upd_xchngbkack (ptr_st_xchngbook,c_ServiceName,c_err_msg);
	if ( i_returncode != 0)
	{
		fn_errlog ( c_ServiceName, "S31100", LIBMSG, c_err_msg );
  	fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
  	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
	}


	fn_gettime();
  if(DEBUG_MSG_LVL_3)   /** Ver 2.8 **/
  {
  	fn_userlog( c_ServiceName, "ORS2-RD Function fn_upd_xchngbkack Time :%s:",c_time);
	}
	/*** Ver 1.8 starts here ***/
  if ( st_orderbook.c_prd_typ == OPTIONPLUS )
 	{
    /** strcpy(st_pstn_actn.c_channel,"SYS"); Commented in Ver 2.9 ***/
    strcpy(st_pstn_actn.c_alias,"*");

    EXEC SQL
    SELECT
          fod_sltp_ordr_rfrnc,
					fod_ordr_stts,
          nvl(fod_amt_blckd,0),
          nvl(fod_lss_amt_blckd,0),
          nvl(fod_fc_flag,'*'),
          nvl(fod_diff_amt_blckd,0),
          nvl(fod_diff_lss_amt_blckd,0),
					nvl(fod_stp_lss_tgr,0)
    INTO
          :c_sltp_ord_rfrnc,
					:c_frsh_ordr_stts,
          :d_amt_blkd,
          :d_lss_amt_blkd,
          :c_fc_flg,
          :d_diff_amt_blkd,
          :d_diff_lss_amt_blkd,
					:l_temp_stp_lss_tgr
    FROM  fod_fo_ordr_dtls
    WHERE fod_ordr_rfrnc = :st_orderbook.c_ordr_rfrnc;

    if ( SQLCODE != 0 )
    {
      fn_errlog ( c_ServiceName, "S31105", SQLMSG, ptr_st_err_msg->c_err_msg );
      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
    }


		if(DEBUG_MSG_LVL_3)
   	{
    	 fn_userlog(c_ServiceName,"Match Account :%s:",st_orderbook.c_cln_mtch_accnt);
    	 fn_userlog(c_ServiceName,"Fresh order reference :%s:",c_sltp_ord_rfrnc);
     	 fn_userlog(c_ServiceName,"order status  :%c:", c_frsh_ordr_stts);
     	 fn_userlog(c_ServiceName,"Modification Counter :%ld:",st_orderbook.l_mdfctn_cntr);

        /*** format specifier changed to %lf from %ld in Ver 4.8 ***/
     	 fn_userlog(c_ServiceName,"amt_blckd   :%lf:",d_amt_blkd);   
     	 fn_userlog(c_ServiceName,"lss_amt_blckd :%lf:",d_lss_amt_blkd);
     	 fn_userlog(c_ServiceName,"diff_amt_blckd :%lf:",d_diff_amt_blkd);
     	 fn_userlog(c_ServiceName,"diff_lss_amt_blckd :%lf:",d_diff_lss_amt_blkd);
       /*** Ver 4.8 format specifier change Ends ***/
 
    	 fn_userlog(c_ServiceName,"l_temp_stp_lss_tgr :%ld:",l_temp_stp_lss_tgr);
   	}

    switch ( ptr_st_xchngbook->l_ors_msg_typ )
    {
      case  ORS_NEW_ORD_ACPT:
						
            break;

      case  ORS_NEW_ORD_RJCT:

            /*If it is a fresh order rejection, place a cancel request for the cover order*/
            if(c_fc_flg == 'F') /*Fresh Order*/
            {
              /*Check if the cover order is in open state*/
              EXEC SQL
              SELECT  FOD_ORDR_STTS,
                      FOD_EXEC_QTY
              INTO    :c_ordr_stts,
                      :l_exctd_qty
              FROM    FOD_FO_ORDR_DTLS
              WHERE   FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

              if(SQLCODE != 0)
              {
                fn_errlog ( c_ServiceName, "S31110", SQLMSG, ptr_st_err_msg->c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }

              if(c_ordr_stts == 'Q')
              {
                i_canrej_flag = 0; /*** Added in ver 3.9 ***/
                fn_userlog(c_ServiceName,"cancellation of cover order not possible as order already placed with the exchange"
);
                /*** Commented in Ver 2.0 ***
								fn_errlog ( c_ServiceName, "S31115", LIBMSG, c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								***/
              }
              else if(c_ordr_stts != 'J' && c_ordr_stts != 'C' && c_ordr_stts != 'F' && l_exctd_qty == 0) /*** In ver 4.4 'F' status is added ***/ 
              {
                i_canrej_flag = 0; /*** Added in ver 3.9 The flag indicates that the cover order is in ordered or requested state ***/
                strcpy(c_remarks,"System placed Cancellation");

                i_returncode = fn_call_svc_fml ( c_ServiceName,
                                       c_err_msg,
                                       "SFO_CANCEL_OPT",
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
                  fn_errlog ( c_ServiceName, "S31120", LIBMSG, c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }
              } /*** ver 3.9 starts ***/
              else if(c_ordr_stts != 'J' && c_ordr_stts != 'C' && c_ordr_stts != 'F') /**for full and part execution**//*** In ver 4.4 'F' status is added ***/
              {
                i_canrej_flag = 0;
              } /*** ver 3.9 ends ***/
             }

						 /*** Ver 2.0 ** Starts ***/
						 /*** else  if(c_fc_flg == 'C') *Cover Order* **** commented in ver 3.9 ***/
             if( ( c_fc_flg == 'C' ) || ( c_fc_flg == 'F' && i_canrej_flag == 1 )) /*** added in ver 3.9 ***/
             {

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
                  /** WHERE   FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc; *** commented in ver 3.9 ***/
                  WHERE   FOD_ORDR_RFRNC = DECODE( :c_fc_flg,'C',:c_sltp_ord_rfrnc,'F',:st_orderbook.c_ordr_rfrnc ); /*** added in ver 3.9 ***/

                if(SQLCODE != 0)
                {
                  fn_errlog ( c_ServiceName, "S31125", SQLMSG, ptr_st_err_msg->c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }

                /*** ver 3.9 starts ***/
                if(DEBUG_MSG_LVL_0)
                {
                  fn_userlog(c_ServiceName,"Inside rejection case c_fc_flg is :%c: and i_canrej_flag is :%d: and c_frsh_ordr_stts is :%c:",c_fc_flg,i_canrej_flag,c_frsh_ordr_stts );
                }
                /*** ver 3.9 ends ***/

                if(DEBUG_MSG_LVL_3)
                {
                  fn_userlog(c_ServiceName,"Margin Amount of the fresh order :%lf:", d_amt_blkd);
                  fn_userlog(c_ServiceName,"Loss Amount of the fresh order :%lf:", d_lss_amt_blkd);
                }

                if( c_frsh_ordr_stts == 'J' || c_frsh_ordr_stts == 'C' || c_frsh_ordr_stts == 'F') /*** In ver 4.4 'F' status is added ***/
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
                                    /** d_amt_blkd * -1, commented in ver 3.9 ***/
                                    d_amt_blkd, /*** added in ver 3.9 ***/
                                    &d_balance_amt);


                    if ( i_returncode != 0 )
                    {
                      fn_errlog ( c_ServiceName, "S31130", LIBMSG, c_err_msg );
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
                                    /** d_lss_amt_blkd * -1, commented in ver 3.9 ***/
                                    d_lss_amt_blkd, /*** added in ver 3.9 ***/
                                    &d_balance_amt);


                    if ( i_returncode != 0 )
                    {
                      fn_errlog ( c_ServiceName, "S31135", LIBMSG, c_err_msg );
                      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                    }
                  }

                  EXEC SQL
                  UPDATE     FOD_FO_ORDR_DTLS
                  SET  /***  FOD_AMT_BLCKD     = 0 ,
                             FOD_LSS_AMT_BLCKD = 0
                  ** WHERE   FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc; *** commented in ver 3.9 ***/
       /**ver 3.9 starts***/ FOD_AMT_BLCKD    = FOD_AMT_BLCKD - :d_amt_blkd,
                             FOD_LSS_AMT_BLCKD = FOD_LSS_AMT_BLCKD - :d_lss_amt_blkd
                  WHERE      FOD_ORDR_RFRNC    = DECODE( :c_fc_flg,'C',:c_sltp_ord_rfrnc,'F',:st_orderbook.c_ordr_rfrnc ); /*** ver 3.9 ends ***/

                  if(SQLCODE != 0)
                  {
                    fn_errlog ( c_ServiceName, "S31140", SQLMSG, ptr_st_err_msg->c_err_msg );
                    fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                  }
                }
             }

				 		 /*** Ver 2.0 ** Ends ***/
            break;

      case  ORS_MOD_ORD_ACPT:
						
						if(c_fc_flg == 'C') /*Cover*/
            {
              d_amt_blkd = 0;
              d_lss_amt_blkd = 0;


								/** commented in Ver 2.6 **

              EXEC SQL
                SELECT  FOD_AMT_BLCKD,
                        FOD_LSS_AMT_BLCKD,
                        FOD_ORDR_FLW
                INTO    :d_amt_blkd,
                        :d_lss_amt_blkd,
                        :c_ord_flw
                FROM    FOD_FO_ORDR_DTLS
                WHERE   FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

              if(SQLCODE != 0)
              {
                fn_errlog ( c_ServiceName, "S31145", SQLMSG, ptr_st_err_msg->c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }

              if(DEBUG_MSG_LVL_3)
              {
                fn_userlog(c_ServiceName,"Margin Amount of the fresh order :%lf:", d_amt_blkd);
                fn_userlog(c_ServiceName,"Loss Amount of the fresh order :%lf:", d_lss_amt_blkd);
              }
            }


            *Get the current margin percentage and sltp % diff. from the database *
            EXEC SQL
              SELECT  nvl(FMM_SLTP_PRCNT,0)
              INTO    :d_fmm_sltp_prcnt
              FROM   FMM_FO_MRGN_MSTR
              WHERE  FMM_PRDCT_TYP= 'O'
              AND    FMM_UNDRLYNG = :st_orderbook.c_undrlyng
              AND    FMM_XCHNG_CD = :st_orderbook.c_xchng_cd
              AND    FMM_UNDRLYNG_BSKT_ID =(SELECT  FCB_UNDRLYNG_BSKT_ID
                                            FROM    FCB_FO_CLN_BSKT_ALLTD
                                            WHERE   FCB_CLN_LVL =(SELECT CLM_CLNT_LVL
                                                                  FROM CLM_CLNT_MSTR
                                                                  WHERE CLM_MTCH_ACCNT = :st_orderbook.c_cln_mtch_accnt));

            if( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
            {
              fn_userlog(c_ServiceName,"******Failed in FMM fetch*******");
              fn_errlog ( c_ServiceName, "S31150", SQLMSG, ptr_st_err_msg->c_err_msg );
              fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }
            else if(SQLCODE != 0)
            {
              fn_errlog ( c_ServiceName, "S31155", SQLMSG, ptr_st_err_msg->c_err_msg );
              fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }

            if(DEBUG_MSG_LVL_3)
            {
              fn_userlog(c_ServiceName,"SLTP Percentage   :%lf:", d_fmm_sltp_prcnt);
            }
	
						** Getting the total executed value and quantity of FRESH order. **
    				EXEC SQL
      				SELECT  NVL(SUM(FTD_EXCTD_QTY * FTD_EXCTD_RT),0),
              				NVL(SUM(FTD_EXCTD_QTY),0)
      				INTO    :d_frsh_exe_val,
              				:li_tot_exec_qty
      				FROM    FTD_FO_TRD_DTLS
     				WHERE   FTD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

						if( SQLCODE != 0 )
						{
							fn_errlog ( c_ServiceName, "S31160", SQLMSG, ptr_st_err_msg->c_err_msg );
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}

						** Checking if Fresh order value & quantity is equals to zero. *
						if (d_frsh_exe_val == 0 || li_tot_exec_qty == 0)
						{
							fn_userlog(c_ServiceName, "******Fresh order is not yet executed.*********");
							fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
							fn_errlog( c_ServiceName, "B23007", DEFMSG,  ptr_st_err_msg->c_err_msg);
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}

						if (c_fc_flg == 'C')
   					{
							** Calculating the Weight Average Price for the executed quantity of FRESH Order. **
      				d_wgt_avrg_price = (double) (d_frsh_exe_val / li_tot_exec_qty);


							* Calculate open quantity *     commented in Ver 2.6 **
							EXEC SQL
								SELECT  NVL( SUM( DECODE( FOD_FC_FLAG, 'C', -1 * FTD_EXCTD_QTY,FTD_EXCTD_QTY ) ), 0 ),
                				NVL( SUM( DECODE( FOD_FC_FLAG, 'F', FTD_EXCTD_QTY, 0 ) ), 0 )
								INTO    :l_open_qty,
												:l_frsh_xtd_qty
								FROM    FOD_FO_ORDR_DTLS, FTD_FO_TRD_DTLS
								WHERE   FOD_CLM_MTCH_ACCNT    = :st_orderbook.c_cln_mtch_accnt
								AND     FOD_TRD_DT            = :st_orderbook.c_trd_dt
								AND     FOD_PRDCT_TYP         = :st_orderbook.c_prd_typ
								AND     FTD_ORDR_RFRNC        = FOD_ORDR_RFRNC
								AND     DECODE( FOD_FC_FLAG, 'F', FOD_ORDR_RFRNC, FOD_SLTP_ORDR_RFRNC ) = :c_sltp_ord_rfrnc;

							if( SQLCODE != 0 )
							{
								fn_errlog ( c_ServiceName, "S31165", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}

							if(l_open_qty <0 )
							{
								l_open_qty=0;
							}

							 if(DEBUG_MSG_LVL_3)
							{
								fn_userlog(c_ServiceName," st_orderbook.c_xchng_cd = :%s:", st_orderbook.c_xchng_cd);
								fn_userlog(c_ServiceName," st_orderbook.c_undrlyng = :%s:", st_orderbook.c_undrlyng);
							}
				
							EXEC SQL
								SELECT nvl(FRM_DIFFRNC_MRGN,0)
								INTO   :d_frm_diffrnc_mrgn
								FROM   FRM_FO_RANGE_MSTR
								WHERE  FRM_XCHNG_CD  = TRIM(:st_orderbook.c_xchng_cd)
								AND    TRIM(FRM_UNDRLYNG ) = TRIM(:st_orderbook.c_undrlyng)
								AND    FRM_ORDR_FLOW = :c_ord_flw                  *** This is fresh order flw ***
								AND    FRM_FROM_RANGE <= :l_temp_stp_lss_tgr
								AND    :l_temp_stp_lss_tgr < FRM_TO_RANGE
								AND    (:l_temp_stp_lss_tgr > 0 ) ;  * Mod sltp trggr price is used **

							if ( SQLCODE  !=  0  && SQLCODE != NO_DATA_FOUND)
							{
								fn_errlog ( c_ServiceName, "S31170", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}
					
							* Calculate Margin amount to be blocked *
							d_amt_to_be_blckd = ((double)(l_open_qty * d_wgt_avrg_price) * d_fmm_sltp_prcnt / 100.0);

							* Calculate Loss amount to be blocked *
							if ( c_ord_flw == BUY )
							{
								if( d_frm_diffrnc_mrgn < 100 )
								{
									d_lss_amt_to_be_blckd =(l_open_qty * (d_wgt_avrg_price - st_xchngbook.l_ord_lmt_rt ) );
								}
								else if ( d_frm_diffrnc_mrgn == 100 )
								{
									d_lss_amt_to_be_blckd =(l_open_qty * (d_wgt_avrg_price ) );
								}
							}
							else if ( c_ord_flw == SELL )
							{
								 d_lss_amt_to_be_blckd =(l_open_qty * (d_wgt_avrg_price - st_xchngbook.l_ord_lmt_rt ) * -1 );
							}


							** Commented in ver 2.6 ****/

							            EXEC SQL
              SELECT  FOD_AMT_BLCKD,
                      FOD_LSS_AMT_BLCKD
                      /*** FOD_LMT_RT  *** Commented in Ver 2.6 ***/
              INTO    :d_amt_blkd,
                      :d_lss_amt_blkd
                      /*** :l_fsh_ord_lmt_rt *** Commented  in Ver 2.6 ***/
              FROM    FOD_FO_ORDR_DTLS
              WHERE   FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

            if(SQLCODE != 0)
            {
              fn_errlog ( c_ServiceName, "S31175", SQLMSG, ptr_st_err_msg->c_err_msg );
              fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }

						/*** 
            EXEC SQL
              SELECT  FOD_LMT_RT
              INTO    :l_cvr_ord_lmt_rt
              FROM    FOD_FO_ORDR_DTLS
              WHERE   FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;  *** Commented on 23Oct2015 in Ver 2.6 ***/

						/*** Added on 23Oct2015 Ver 2.6 ***/
						EXEC SQL
							SELECT	FXB_LMT_RT
							INTO		:l_fsh_ord_lmt_rt
							FROM		FXB_FO_XCHNG_BOOK
							WHERE		FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc
							AND			FXB_MDFCTN_CNTR = (select max(FXB_MDFCTN_CNTR) FROM FXB_FO_XCHNG_BOOK
							WHERE		FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc
							AND     FXB_PLCD_STTS != 'J');

						
						if(SQLCODE != 0)
						{
							fn_errlog ( c_ServiceName, "S31180", SQLMSG, ptr_st_err_msg->c_err_msg );
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
							/*** Ver 2.6 ends ***/

            if(SQLCODE != 0)
            {
              fn_errlog ( c_ServiceName, "S31185", SQLMSG, ptr_st_err_msg->c_err_msg );
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
            i_returncode =   fn_cal_mrgn_sltp_op  
                              (
                                c_ServiceName,
                                c_sltp_ord_rfrnc,
                                l_fsh_ord_lmt_rt,
                                l_cvr_ord_lmt_rt,
                                &d_amt_to_be_blckd,
                                &d_lss_amt_to_be_blckd,
                                c_ref_rmrks,  /*** Ver 5.1***/
                                &d_comp1_mrgn,/*** Ver 5.1***/
                                &d_comp1_pl,  /*** Ver 5.1***/
                                &d_xchng_amt, /*** Ver 5.1***/
                                &d_prem_amt,  /*** Ver 5.1***/
                                &d_spn_mrgn,  /*** Ver 5.1***/
                                &d_expr_mrgn, /*** Ver 5.1***/
																'E',
                                c_err_msg
                              );

						if ( i_returncode != SUCC_BFR )
            {
              fn_errlog ( c_ServiceName, "S31190", LIBMSG, ptr_st_err_msg->c_err_msg );
              fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }	

							/************ Ver 2.6 Ends ***********/

						/* Difference margin & loss */
						d_diff_amt = (double) d_amt_blkd - d_amt_to_be_blckd;
						d_diff_loss = (double) d_lss_amt_blkd - d_lss_amt_to_be_blckd;
		
						d_diff_amt  =  (long) (long) d_diff_amt ;
						d_diff_loss =  (long) (long) d_diff_loss ;

					  if(DEBUG_MSG_LVL_3)
						{
								fn_userlog(c_ServiceName,"d_frm_diffrnc_mrgn = :%lf:", d_frm_diffrnc_mrgn);
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
										fn_errlog ( c_ServiceName, "S31195", LIBMSG, c_err_msg );
										fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
										tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
										
									}

									d_diff_amt = d_diff_amt - d_balance_amt ;
							}

	           /*** Ver 5.1 started ***/
      			 MEMSET(c_tmp_rmrks);
      			 strcpy ( c_tmp_rmrks, st_pstn_actn.c_ref_rmrks );
      			 sprintf( st_pstn_actn.c_ref_rmrks,"%s%s",c_ref_rmrks,c_tmp_rmrks);
             /*** Ver 5.1 ends ***/


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
																									DEBIT_TILL_LIMIT, /*** WHEN limit to TILL limit ***/
                              										d_lss_amt_blkd,
                              										&d_balance_amt);


										if ( i_returncode != 0 )
										{
											fn_errlog ( c_ServiceName, "S31200", LIBMSG, c_err_msg );
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
																								DEBIT_TILL_LIMIT, /*** WHEN limit to TILL limit ***/
																								d_diff_loss,
																								&d_balance_amt);


									if ( i_returncode != 0 )
									{
										fn_errlog ( c_ServiceName, "S31205", LIBMSG, c_err_msg );
										fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
            				tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); 
          				}
									d_diff_loss = d_diff_loss  - d_balance_amt ;
							}


            strcpy (st_pstn_actn.c_ref_rmrks, c_tmp_rmrks);    /**** Ver 5.1 ***/
            MEMSET(c_tmp_rmrks);                               /**** Ver 5.1 ***/


							EXEC SQL
								 UPDATE     FOD_FO_ORDR_DTLS
								 SET        FOD_AMT_BLCKD     = :d_amt_blkd - :d_diff_amt,
														FOD_LSS_AMT_BLCKD = :d_lss_amt_blkd - :d_diff_loss,
              							FOD_SLTP_MRGN = :d_comp1_mrgn,/*** Ver 5.1 ***/
          							    FOD_SLTP_PL   = :d_comp1_pl,  /*** Ver 5.1 ***/
        						    	  FOD_SLTP_SEBI_MRGN = :d_xchng_amt, /*** Ver 5.1 ***/
				            		  	FOD_OPLUS_PREMIUM  = :d_prem_amt,  /*** Ver 5.1 ***/
              							FOD_SPAN_MRGN = :d_spn_mrgn,       /*** Ver 5.1 ***/
              							FOD_EXPR_MRGN = :d_expr_mrgn       /*** Ver 5.1 ***/
								 WHERE      FOD_ORDR_RFRNC    = :c_sltp_ord_rfrnc;

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31210", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}

							EXEC SQL
								 UPDATE     FOD_FO_ORDR_DTLS
								 SET        FOD_DIFF_AMT_BLCKD      =  0,
														FOD_DIFF_LSS_AMT_BLCKD  =  0 
								 WHERE      FOD_ORDR_RFRNC          =  :st_orderbook.c_ordr_rfrnc;

							if(SQLCODE != 0)
							{
								fn_errlog ( c_ServiceName, "S31215", SQLMSG, ptr_st_err_msg->c_err_msg );
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}


						}	


            break;

          case  ORS_MOD_ORD_RJCT:

						/************** Commented in Ver 2.6 ***********

            if(c_fc_flg == 'C') *Cover*
            {
              d_amt_blkd = 0.0;
              d_lss_amt_blkd = 0.0;

              EXEC SQL
                 SELECT     FOD_AMT_BLCKD,
                            FOD_LSS_AMT_BLCKD
                 INTO       :d_amt_blkd,
                            :d_lss_amt_blkd
                 FROM       FOD_FO_ORDR_DTLS
                 WHERE      FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

              if(SQLCODE != 0)
              {
                fn_errlog ( c_ServiceName, "S31220", SQLMSG, ptr_st_err_msg->c_err_msg );
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
                  fn_errlog ( c_ServiceName, "S31225", LIBMSG, c_err_msg );
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
                  fn_errlog ( c_ServiceName, "S31230", LIBMSG, c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }
              }

              EXEC SQL
               UPDATE     FOD_FO_ORDR_DTLS
               SET        FOD_AMT_BLCKD     = :d_amt_blkd + :d_diff_amt_blkd ,
                          FOD_LSS_AMT_BLCKD = :d_lss_amt_blkd + :d_diff_lss_amt_blkd
               WHERE      FOD_ORDR_RFRNC    = :c_sltp_ord_rfrnc;

              if(SQLCODE != 0)
              {
                fn_errlog ( c_ServiceName, "S31235", SQLMSG, ptr_st_err_msg->c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }

              EXEC SQL
               UPDATE     FOD_FO_ORDR_DTLS
               SET        FOD_DIFF_AMT_BLCKD      = 0,
                          FOD_DIFF_LSS_AMT_BLCKD  = 0
               WHERE      FOD_ORDR_RFRNC          = :st_orderbook.c_ordr_rfrnc;

              if(SQLCODE != 0)
              {
                fn_errlog ( c_ServiceName, "S31240", SQLMSG, ptr_st_err_msg->c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }
            }
            break;

					**************** Commented Ver 2.6 Ends ***************/

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
                  fn_errlog ( c_ServiceName, "S31245", SQLMSG, ptr_st_err_msg->c_err_msg );
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
                  fn_errlog ( c_ServiceName, "S31250", SQLMSG, ptr_st_err_msg->c_err_msg );
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
                i_returncode =    fn_cal_mrgn_sltp_op
                                  (
                                    c_ServiceName,
                                    c_sltp_ord_rfrnc,
                                    l_fsh_ord_lmt_rt,
                                    l_cvr_ord_lmt_rt,
                                    &d_amt_to_be_blckd,
                                    &d_lss_amt_to_be_blckd,
                                		c_ref_rmrks,  /*** Ver 5.1***/
                                		&d_comp1_mrgn,/*** Ver 5.1***/
                                		&d_comp1_pl,  /*** Ver 5.1***/
                                		&d_xchng_amt, /*** Ver 5.1***/
                                		&d_prem_amt,  /*** Ver 5.1***/
                                		&d_spn_mrgn,  /*** Ver 5.1***/
                                		&d_expr_mrgn, /*** Ver 5.1***/
																		'E',
                                    c_err_msg
                                  );

								                if ( i_returncode != SUCC_BFR )
                {
                  fn_errlog ( c_ServiceName, "S31255", LIBMSG, ptr_st_err_msg->c_err_msg );
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

                /***  Debit margin loss/profit when limit .                           ***/
                /***  Release loss amount blocked only in case of  PROFIT .           ***/
                /***  In case of LOSS , deficit  loss amount is released or blocked.  ***/

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
                      fn_errlog ( c_ServiceName, "S31260", LIBMSG, c_err_msg );
                      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );

                    }

                    d_diff_amt = d_diff_amt - d_balance_amt ;
                }

	      		     /*** Ver 5.1 started ***/
				    		  MEMSET(c_tmp_rmrks);
      						strcpy ( c_tmp_rmrks, st_pstn_actn.c_ref_rmrks );
      						sprintf(st_pstn_actn.c_ref_rmrks,"%s%s",c_ref_rmrks,c_tmp_rmrks);
      					 /*** Ver 5.1 ends ***/

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
                        fn_errlog ( c_ServiceName, "S31265", LIBMSG, c_err_msg );
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
                      fn_errlog ( c_ServiceName, "S31270", LIBMSG, c_err_msg );
                      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                    }
                    d_diff_loss = d_diff_loss  - d_balance_amt ;
                }

            		strcpy ( st_pstn_actn.c_ref_rmrks, c_tmp_rmrks);    /**** Ver 5.1 ***/
            		MEMSET(c_tmp_rmrks);                                    /**** Ver 5.1 ***/

                EXEC SQL
                   UPDATE     FOD_FO_ORDR_DTLS
                   SET        FOD_AMT_BLCKD     = :d_amt_blkd - :d_diff_amt,
                              FOD_LSS_AMT_BLCKD = :d_lss_amt_blkd - :d_diff_loss,
              								FOD_SLTP_MRGN = :d_comp1_mrgn,/*** Ver 5.1 ***/
              								FOD_SLTP_PL   = :d_comp1_pl,  /*** Ver 5.1 ***/
     						   			      FOD_SLTP_SEBI_MRGN = :d_xchng_amt, /*** Ver 5.1 ***/
   			       						    FOD_OPLUS_PREMIUM  = :d_prem_amt,  /*** Ver 5.1 ***/
      						        		FOD_SPAN_MRGN = :d_spn_mrgn,       /*** Ver 5.1 ***/
			            				  	FOD_EXPR_MRGN = :d_expr_mrgn       /*** Ver 5.1 ***/
                   WHERE      FOD_ORDR_RFRNC    = :c_sltp_ord_rfrnc;

                if(SQLCODE != 0)
                {
                  fn_errlog ( c_ServiceName, "S31275", SQLMSG, ptr_st_err_msg->c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }

            }
						/*********** Ver 2.6 ** Ends ***********/





				
	
		

          case  ORS_CAN_ORD_ACPT:
					case  ORS_IOC_CAN_ACPT: /*** Ver 2.0 ***/

						/*** start Ver 2.0 ***/
						
						/* Calculate order status */
						c_ordr_stts = '\0';
						EXEC SQL
							SELECT  distinct decode(FOD_ORDR_STTS,'J','C','D','C',FOD_ORDR_STTS)
							INTO    :c_ordr_stts
							FROM    FOD_FO_ORDR_DTLS
							WHERE		FOD_ORDR_RFRNC in ( :st_orderbook.c_ordr_rfrnc, :c_sltp_ord_rfrnc);

						if( SQLCODE != 0 && SQLCODE != TOO_MANY_ROWS_FETCHED)
						{
							fn_errlog ( c_ServiceName, "S31280", LIBMSG, ptr_st_err_msg->c_err_msg );
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
              SELECT  NVL( SUM( DECODE( FOD_FC_FLAG, 'C', -1 * FTD_EXCTD_QTY, FTD_EXCTD_QTY ) ), 0 )
              INTO    :l_open_qty
              FROM    FOD_FO_ORDR_DTLS, FTD_FO_TRD_DTLS
              WHERE   FOD_CLM_MTCH_ACCNT    = :st_orderbook.c_cln_mtch_accnt
              AND     FOD_TRD_DT            = :st_orderbook.c_trd_dt
              AND     FOD_PRDCT_TYP         = :st_orderbook.c_prd_typ
              AND     FTD_ORDR_RFRNC        = FOD_ORDR_RFRNC
              AND     DECODE( FOD_FC_FLAG, 'F', FOD_ORDR_RFRNC, FOD_SLTP_ORDR_RFRNC ) = decode(:c_fc_flg,'F',:st_orderbook.c_ordr_rfrnc,:c_sltp_ord_rfrnc); 

            if( SQLCODE != 0 )
            {
                fn_errlog ( c_ServiceName, "S31285", SQLMSG, ptr_st_err_msg->c_err_msg );
      					fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ); /*** Added in Ver 3.4 ***/
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }

  					if(DEBUG_MSG_LVL_3)   /** Ver 2.8 **/
  					{
							fn_userlog(c_ServiceName,"l_open_qty is :%ld:", l_open_qty); 
						}
						if (c_ordr_stts == 'C' && l_open_qty == 0 )
						{
							/*** Release the margin and loss amount blocked ***/
							EXEC SQL
								SELECT
											NVL(FOD_AMT_BLCKD,0),
											NVL(FOD_LSS_AMT_BLCKD,0)
								INTO  :d_amt_blkd,
											:d_lss_amt_blkd
								FROM  FOD_FO_ORDR_DTLS
								WHERE FOD_ORDR_RFRNC =  decode(:c_fc_flg, 'C', :c_sltp_ord_rfrnc, :st_orderbook.c_ordr_rfrnc );

								if( SQLCODE != 0 )
								{
									fn_errlog ( c_ServiceName, "S31290", LIBMSG, ptr_st_err_msg->c_err_msg );
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
										fn_errlog ( c_ServiceName, "S31295", SQLMSG, ptr_st_err_msg->c_err_msg );
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
										fn_errlog ( c_ServiceName, "S31300", SQLMSG, ptr_st_err_msg->c_err_msg );
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
              							FOD_SLTP_MRGN = 0,/*** Ver 5.1 ***/
              							FOD_SLTP_PL   = 0,  /*** Ver 5.1 ***/
              							FOD_SLTP_SEBI_MRGN = 0, /*** Ver 5.1 ***/
              							FOD_OPLUS_PREMIUM  = 0,  /*** Ver 5.1 ***/
              							FOD_SPAN_MRGN = 0,       /*** Ver 5.1 ***/
              							FOD_EXPR_MRGN = 0       /*** Ver 5.1 ***/
										 WHERE  FOD_ORDR_RFRNC		=  decode(:c_fc_flg, 'C', :c_sltp_ord_rfrnc, :st_orderbook.c_ordr_rfrnc );
								
								if ( SQLCODE != 0 )
								{
									fn_errlog ( c_ServiceName, "S31305", SQLMSG, ptr_st_err_msg->c_err_msg );
									fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}
						}
						/*** End Ver 2.0 ***/

						/*** Commented in Ver 2.0 
            if(c_fc_flg == 'F') *Fresh*
            {
              d_amt_blkd = 0.0;
              d_lss_amt_blkd = 0.0;
              c_ordr_stts = '\0';

              EXEC SQL
                 SELECT     NVL(FOD_AMT_BLCKD,0),
                            NVL(FOD_LSS_AMT_BLCKD,0),
                            FOD_ORDR_STTS,
                            FOD_ORDR_FLW
                 INTO       :d_amt_blkd,
                            :d_lss_amt_blkd,
                            :c_ordr_stts,
                            :c_ordr_flw
                 FROM       FOD_FO_ORDR_DTLS
                 WHERE      FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

              if(SQLCODE != 0)
              {
                fn_errlog ( c_ServiceName, "S31310", SQLMSG, ptr_st_err_msg->c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); 
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
               AND        FMM_PRDCT_TYP ='O'
               AND        FMM_UNDRLYNG = :st_orderbook.c_undrlyng
               AND        FMM_UNDRLYNG_BSKT_ID =(SELECT  FCB_UNDRLYNG_BSKT_ID
                                               FROM    FCB_FO_CLN_BSKT_ALLTD
                                               WHERE   FCB_CLN_LVL =(SELECT CLM_CLNT_LVL
                                                                     FROM    CLM_CLNT_MSTR
                                                                     WHERE   CLM_MTCH_ACCNT =:st_orderbook.c_cln_mtch_accnt))
;

              if(SQLCODE != 0)
              {
                fn_errlog ( c_ServiceName, "S31315", SQLMSG, ptr_st_err_msg->c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
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
                fn_errlog ( c_ServiceName, "S31320", SQLMSG, ptr_st_err_msg->c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
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
               SELECT     NVL(FOD_LMT_RT,0),
													NVL(FOD_STP_LSS_TGR,0)
               INTO       :li_lmt_rt,
													:l_temp_stp_lss_tgr
               FROM       FOD_FO_ORDR_DTLS
               WHERE      FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

              if(SQLCODE != 0)
              {
                fn_errlog ( c_ServiceName, "S31325", SQLMSG, ptr_st_err_msg->c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
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
                fn_errlog ( c_ServiceName, "S31330", SQLMSG, ptr_st_err_msg->c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }

              * Calculate Margin amount to be blocked *
              d_amt_to_be_blckd = ((double)(l_open_qty * d_wgt_avrg_price) * d_fmm_sltp_prcnt / 100.0);

							* Get the minimum difference percentage ***
						
							EXEC SQL
								SELECT nvl(FRM_DIFFRNC_MRGN,0)
								INTO   :d_frm_diffrnc_mrgn
								FROM   FRM_FO_RANGE_MSTR
								WHERE  FRM_XCHNG_CD 	= :st_orderbook.c_xchng_cd
								AND    FRM_UNDRLYNG 	= :st_orderbook.c_undrlyng
								AND    FRM_ORDR_FLOW 	= :c_ordr_flw
								AND    FRM_FROM_RANGE <= :l_temp_stp_lss_tgr
                AND    :l_temp_stp_lss_tgr < FRM_TO_RANGE;

							if ( SQLCODE  !=  0  && SQLCODE != NO_DATA_FOUND)
							{
								fn_errlog( c_ServiceName, "S31335",SQLMSG,c_err_msg);
								fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							}	

              * Calculate Loss amount to be blocked *
              if ( c_ordr_flw == BUY )
              {
								if( d_frm_diffrnc_mrgn < 100 )
        				{
                	d_lss_amt_to_be_blckd = (double) (l_open_qty * ( li_lmt_rt - d_wgt_avrg_price) );
								}
								else if ( d_frm_diffrnc_mrgn == 100 )
        				{
                	d_lss_amt_to_be_blckd = (double) (l_open_qty * d_wgt_avrg_price );
								}
							}
              else if ( c_ordr_flw == SELL )
              {
                d_lss_amt_to_be_blckd = (double) (l_open_qty * (d_wgt_avrg_price - li_lmt_rt) );
              }

              * Difference margin & loss *
              d_diff_amt  = d_amt_blkd - d_amt_to_be_blckd;
              d_diff_loss = d_lss_amt_blkd + d_lss_amt_to_be_blckd;

							d_diff_amt = (long) (long) d_diff_amt ;
							d_diff_loss = (long) (long) d_diff_loss ;

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
                    fn_errlog ( c_ServiceName, "S31340", LIBMSG, ptr_st_err_msg->c_err_msg );
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
                    fn_errlog ( c_ServiceName, "S31345", LIBMSG, ptr_st_err_msg->c_err_msg );
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
                    fn_errlog ( c_ServiceName, "S31350", LIBMSG, ptr_st_err_msg->c_err_msg );
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
                  fn_errlog ( c_ServiceName, "S31355", LIBMSG, ptr_st_err_msg->c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }
            }
						*** Commented in Ver 2.0 ***/
						
					 if ( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_ACPT ) /*** Ver 2.0 ***/
					 {
            if(c_fc_flg == 'C' || c_fc_flg == 'F' || c_fc_flg == 'E' ) /*Cover*/ /*** Ver 2.0 Fresh flag added ***/
            {
							/***  Commit cancellation acceptance...and place squareoff order in another transaction ***/
              if ( fn_committran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ) == -1 )
              {
                fn_errlog ( c_ServiceName, "S31360", LIBMSG, c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
              }
							/***Ver 5.7 starts ***/

							i_returncode = fn_upd_nrmlmrgn_reporting(st_orderbook.c_cln_mtch_accnt,
                                       st_orderbook.c_xchng_cd,
                                       c_ServiceName,
                                       ptr_st_err_msg->c_err_msg
                                      );
  					if( i_returncode != 0 )
  					{
   					 	fn_errlog(c_ServiceName, "S31365", LIBMSG, ptr_st_err_msg->c_err_msg );
    					fn_userlog(c_ServiceName,"Error while calling function fn_upd_mrgn_reporting");
    					tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
  					}
						/***Ver 5.7 ends ***/
              i_commit_flg = 1;
					  	if(DEBUG_MSG_LVL_0)   /** Ver 2.8 **/
  						{
              	fn_userlog(c_ServiceName,"Transaction commited...i_commit_flg = :%d:",i_commit_flg);
              }	

              EXEC SQL
                 SELECT     FOD_ORDR_STTS
                 INTO       :c_ordr_stts
                 FROM       FOD_FO_ORDR_DTLS
                 WHERE      FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

              if(SQLCODE != 0)
              {
                fn_errlog ( c_ServiceName, "S31370", SQLMSG, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }

            /* Calculate open quantity */
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
              AND     DECODE( FOD_FC_FLAG, 'F', FOD_ORDR_RFRNC, FOD_SLTP_ORDR_RFRNC ) = decode(:c_fc_flg,'F',:st_orderbook.c_ordr_rfrnc,:c_sltp_ord_rfrnc); /*** Ver 2.0 decode added ***/

            if( SQLCODE != 0 )
            {
                fn_errlog ( c_ServiceName, "S31375", SQLMSG, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }
  					if(DEBUG_MSG_LVL_0)   /** Ver 2.8 **/
  					{
							fn_userlog(c_ServiceName,"l_open_qty is :%ld:", l_open_qty); /*** Ver 2.0 ***/
						}
						if (l_open_qty == 0  ) /*** if loop added in Ver 2.0 ***/
						{
							break;
						}

						fn_userlog( c_ServiceName, "CR-ISEC14-144377 Index/Stock :%c:",  st_orderbook.c_ctgry_indstk );   /*** Added in Ver 5.2 ***/

            EXEC SQL
             SELECT /*** MTM_OP_REQ Commented in Ver 5.2 ***/
										DECODE( :st_orderbook.c_ctgry_indstk,'I',MTM_OP_REQ,'S',MTM_OP_REQ_S,'N' )	/*** Ver 5.2 ***/ 
             INTO   :c_mtm_sltp_req
             FROM   MTM_APPLN_STTS
             WHERE  mtm_xchng_cd = :st_orderbook.c_xchng_cd
             AND    mtm_pipe_id LIKE :st_orderbook.c_pipe_id;

            if(SQLCODE != 0)
            {
              fn_errlog ( c_ServiceName, "S31380", SQLMSG, ptr_st_err_msg->c_err_msg );
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }

           if(DEBUG_MSG_LVL_3)
            {
              fn_userlog(c_ServiceName,"Order status of fresh order :%c:", c_ordr_stts);
              fn_userlog(c_ServiceName,"Request type :%c:", c_mtm_sltp_req);
            }

            if (c_mtm_sltp_req == 'A' || strcmp(c_sys_msg,"MY_EOS")==0 )  /* Request type is 'B' when EOS is run for Cancellation & squareoff both */ /*** Ver 2.9 ***/
            {
              if (c_ordr_stts == EXECUTED || /*** c_ordr_stts == PARTIALLY_EXECUTED || ***/ c_ordr_stts == PARTIALLY_EXECUTED_AND_EXPIRED || c_ordr_stts == PARTIALLY_EXECUTED_AND_CANCELLED || c_ordr_stts == CANCELLED || c_ordr_stts == REJECTED)  /* Check order status of fresh order */ 
							/*** CANCELLED,REJECTED condition added in Ver 2.0  and PARTIALLY_EXECUTED condition removed ***/
              {
                /* Place square off order */
                strcpy( c_svc_nm, "SFO_OPT_PLC_ORD");
                c_slm_flg=MARKET;
                c_ord_typ=GOOD_TILL_TODAY;
                li_dsclsd_qty = 0;
                li_stp_lss_tgr_prc = 0;
                li_lmt_rt = 0;
                /** strcpy (c_sys_msg,"EOS"); Commented in Ver 2.9 ***/
								if(strcmp(c_sys_msg,"MY_EOS")==0)  /*** If condition added in ver 2.9 ***/
								{
                  c_spl_flg=CLIENT_SQROFF;
                }
                else
								{
									c_spl_flg = SYSTEM_SQUAREOFF;
								}
                sql_fum_freeze_qty = 0;

  /*** Commented in Ver 3.3 
                 EXEC SQL
                   SELECT fum_freeze_qty
                   INTO   :sql_fum_freeze_qty
                   FROM   fum_fo_undrlyng_mstr
                   WHERE  fum_xchng_cd  = :st_pstn_actn.c_xchng_cd
                   AND    fum_prdct_typ = 'O'
                   AND    fum_undrlyng  = :st_pstn_actn.c_undrlyng; *** Comment Ends ***/

  						/*** Ver 3.3 starts ***/
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
                  AND    ftq_prdct_typ =  'O'
       						AND    ftq_opt_typ   =  :st_pstn_actn.c_opt_typ
       						AND    ftq_strk_prc  =  :st_pstn_actn.l_strike_prc
                  AND    ftq_undrlyng  =  :st_pstn_actn.c_undrlyng
                  AND    ftq_expry_dt  =  :st_pstn_actn.c_expry_dt;

  						/*** Ver 3.3 ends ***/

                if ( SQLCODE != 0 )
                {
                  fn_errlog ( c_ServiceName, "S31385", SQLMSG, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }

                /*** Ver 3.3 Starts ***/
                if( DEBUG_MSG_LVL_3)
                {
                  fn_userlog( c_ServiceName,"l_max_allowed_qty is :%ld:", l_max_allowed_qty );
                }
                /*** Ver 3.3 Ends ***/

                if( l_open_qty  > sql_fum_freeze_qty && sql_fum_freeze_qty != 0 )
                {
                  /*** l_open_qty = sql_fum_freeze_qty; Commented in Ver 3.3 ***/
                  l_open_qty = l_max_allowed_qty;  /*** Added in Ver 3.3 ***/
                }
							
								/*** Following are cumultive changes ***/

								EXEC SQL
											SELECT  NVL(EXG_MKT_MAX_VAL,0),
											NVL(EXG_MAX_CUM_OPNORD_VAL,0)
								INTO  :sql_exg_mkt_max_val,
											:sql_exg_max_cum_opnord_val
								FROM  EXG_XCHNG_MSTR
								WHERE EXG_XCHNG_CD = :st_pstn_actn.c_xchng_cd ;

							 if ( SQLCODE != 0 )
								{
									strcpy ( ptr_st_err_msg->c_err_msg, "Error while selecting Max Limit Val..Contact system support" );
									fn_errlog ( c_ServiceName, "S31390", SQLMSG, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}	
									
								EXEC SQL
								SELECT NVL( FTQ_LST_TRD_PRC, 0 ),
												FTQ_MIN_LOT_QTY          /*** Added In Ver 2.5 ***/
								INTO    :d_lst_trdd_prc,
											  :l_lot_sz                /*** Added In Ver 2.5 ***/	
								FROM    FTQ_FO_TRD_QT
								WHERE   FTQ_XCHNG_CD  =  :st_pstn_actn.c_xchng_cd
								AND     FTQ_PRDCT_TYP =  'O'
								AND     FTQ_UNDRLYNG  =  :st_pstn_actn.c_undrlyng
								AND     FTQ_EXPRY_DT  =  to_date ( :st_pstn_actn.c_expry_dt ,'dd-Mon-yyyy' )
								AND     FTQ_EXER_TYP  =  :st_pstn_actn.c_exrc_typ
								AND     FTQ_OPT_TYP   =  :st_pstn_actn.c_opt_typ
								AND     FTQ_STRK_PRC  =  :st_pstn_actn.l_strike_prc ;

								if ( SQLCODE != 0 )
								{
									strcpy ( ptr_st_err_msg->c_err_msg, "Error while selecting last traded price. Contact system support" );
									fn_errlog ( c_ServiceName, "S31395", SQLMSG, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								}

                /*** Commented In Ver 2.5 **** Lot size fetch introduced in above query ***	
	
								EXEC SQL
									SELECT  FCM_LOT_SZ
									INTO    :l_lot_sz
									FROM    FCM_FO_CNTRCT_MSTR
									WHERE   FCM_XCHNG_CD  = :st_pstn_actn.c_xchng_cd
									AND     FCM_UNDRLYNG  = :st_pstn_actn.c_undrlyng
									AND     ROWNUM < 2;

							 if ( SQLCODE != 0 )
							 {
								 strcpy ( ptr_st_err_msg->c_err_msg, "Error while selecting Lot size. Contact system support" );
								 fn_errlog ( c_ServiceName, "S31400", SQLMSG, ptr_st_err_msg->c_err_msg );
								 tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); 
							 }

                *** Ver 2.5 Comment Ends ***/		

								EXEC SQL
								SELECT  NVL(SUM(DECODE(FOD_PRDCT_TYP,'O',(FOD_LMT_RT+FOD_STRK_PRC),'I',(FOD_LMT_RT+FOD_STRK_PRC),FOD_LMT_RT)*(FOD_ORDR_TOT_QTY - NVL(FOD_EXEC_QTY,0) - NVL(FOD_CNCL_QTY,0))),0)
								INTO    :d_tot_open_val
								FROM    FOD_FO_ORDR_DTLS , EXG_XCHNG_MSTR
								WHERE   FOD_TRD_DT = EXG_NXT_TRD_DT
								AND     FOD_XCHNG_CD = EXG_XCHNG_CD
								AND     EXG_XCHNG_CD = :st_pstn_actn.c_xchng_cd
								AND     FOD_ORDR_STTS IN ('R','O','Q','P')
								AND     FOD_CLM_MTCH_ACCNT = :st_pstn_actn.c_cln_mtch_accnt ;

							 if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
							 {
								 strcpy ( ptr_st_err_msg->c_err_msg, "Error while selecting Lot size. Contact system support" );
								 fn_errlog ( c_ServiceName, "S31405", SQLMSG, ptr_st_err_msg->c_err_msg );
								 tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
							 }

								d_optpls_ordr_val = (l_open_qty * (d_lst_trdd_prc + st_pstn_actn.l_strike_prc) ) ;  /*** In paise ***/
								sql_exg_mkt_max_val = ( sql_exg_mkt_max_val*100 ) ; /*** Converting Market val into Paise ***/
							  sql_exg_max_cum_opnord_val = ( sql_exg_max_cum_opnord_val *100 ) ; /*** Converting Market val into Paise ***/

							 if( DEBUG_MSG_LVL_3)
							 {
								 fn_userlog( c_ServiceName,"--- d_optpls_ordr_val Is :%lf:", d_optpls_ordr_val );
								 fn_userlog( c_ServiceName,"--- d_tot_open_val Is :%lf:", d_tot_open_val);
								 fn_userlog( c_ServiceName,"--- sql_exg_mkt_max_val Is :%lf:", sql_exg_mkt_max_val);
								 fn_userlog( c_ServiceName,"--- sql_exg_max_cum_opnord_val Is :%lf:", sql_exg_max_cum_opnord_val );
								 fn_userlog( c_ServiceName,"--- d_lst_trdd_prc Is :%lf:", d_lst_trdd_prc);
								 fn_userlog( c_ServiceName,"--- l_lot_sz Is :%ld:", l_lot_sz);
							 }


								i_cumopn_val_flg = 0;

								EXEC SQL
											SELECT  1
											INTO    :i_cumopn_val_flg
											FROM    DUAL
											WHERE   (:d_tot_open_val + :d_optpls_ordr_val) > :sql_exg_max_cum_opnord_val;

								 if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
								 {
									strcpy ( ptr_st_err_msg->c_err_msg, "Error while selecting cumulative flag" );
									fn_errlog ( c_ServiceName, "S31410", SQLMSG, ptr_st_err_msg->c_err_msg );
									tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
								 }


							 if ( i_cumopn_val_flg == 1 )
							 {
								fn_userlog ( c_ServiceName,"Order Value exceeding max Cumulative open order value " );
								fn_userlog ( c_ServiceName,"Skipping Match accnt :%s: as Order Value exceeding max Cumulative open order value ",st_pstn_actn.c_cln_mtch_accnt);
								break ;
							 }

						 /*** Commented in Ver 4.9 starts ***
							i_max_val_flg = 0;

						 EXEC SQL
									SELECT  1
									INTO    :i_max_val_flg
									FROM    DUAL
									WHERE   :d_optpls_ordr_val > :sql_exg_mkt_max_val ;

						 if ( SQLCODE != 0 && SQLCODE != NO_DATA_FOUND )
						 {
								strcpy ( ptr_st_err_msg->c_err_msg , "Error while selecting Lot size. Contact system support" );
								fn_errlog ( c_ServiceName, "S31415", SQLMSG, ptr_st_err_msg->c_err_msg );
								tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						 }


							if ( i_max_val_flg == 1 )
						 {
							l_new_max_qty =  sql_exg_mkt_max_val  / ( d_lst_trdd_prc + st_pstn_actn.l_strike_prc ) ;

							*** Checking if Max qty is in multiples of Lot size ***

							if (  (l_new_max_qty % l_lot_sz) == 0 )
							{
								if ( l_new_max_qty > l_lot_sz )
								{
									l_new_max_qty = l_new_max_qty - l_lot_sz ; ** to ensure that the max limit is not reached , one lot is reduced **
								}

								if( DEBUG_MSG_LVL_3)
								{
									fn_userlog( c_ServiceName," Inside case qty  in multiples of lot size");
									fn_userlog( c_ServiceName," l_new_max_qty Is :%ld:",l_new_max_qty);
								}
							}
							else
							{
								l_new_lots    = l_new_max_qty / l_lot_sz ;
								l_new_max_qty = l_new_lots * l_lot_sz ;
								if ( l_new_max_qty > l_lot_sz )
								{
									l_new_max_qty = l_new_max_qty - l_lot_sz ; ** to ensure that the max limit is not reached , one lot is reduced **
								}

								if( DEBUG_MSG_LVL_3) 
								{
									fn_userlog( c_ServiceName," Inside case qty NOT in multiples of lot size");
									fn_userlog( c_ServiceName," l_new_max_qty Is :%ld:",l_new_max_qty);
									fn_userlog( c_ServiceName," l_new_lots Is :%ld:",l_new_lots);
								}

							}

							l_open_qty  = l_new_max_qty ;
						}
						*** Ver 4.9 ends ***/

						/*** cumultive changes ends ***/

						/*** starts Ver 2.0 ***/
						
						if( DEBUG_MSG_LVL_3)
						{
							fn_userlog( c_ServiceName," N c_fc_flg = :%c:",c_fc_flg);
							fn_userlog( c_ServiceName," N l_open_qty Is :%ld:",l_open_qty);
							fn_userlog( c_ServiceName," N st_pstn_actn.c_trnsctn_flw Is :%c:",st_pstn_actn.c_trnsctn_flw);
						}
						if ( c_fc_flg == 'C')
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
						if(DEBUG_MSG_LVL_3)  /** Ver 2.8 ***/
						{
							fn_userlog( c_ServiceName,"NN st_pstn_actn.c_trnsctn_flw Is :%c:",st_pstn_actn.c_trnsctn_flw);
						}
						if (l_open_qty < 0) 
						{
            	if(DEBUG_MSG_LVL_3)  /** Ver 2.8 ***/
            	{
								/** fn_userlog( c_ServiceName,"Sys msg set to EXCPT_EOS");  Commented in Ver 2.9 **/
								 fn_userlog( c_ServiceName,"Open Qty :-%ld:",l_open_qty);
							}
							/** strcpy(c_sys_msg,"EXCPT_EOS"); Commented in Ver 2.9 
							l_open_qty = l_open_qty * -1; ****/
						}
						/*** Ver 2.0 ends ****/
        
            i_returncode = fn_acall_svc_fml (  c_ServiceName,
																							 c_err_msg,
																							 "SFO_ORD_ROUT",
																							 0,
																							 24,
																							/*** FFO_ORDR_RFRNC, (char *)st_orderbook.c_ordr_rfrnc, *** Commented in Ver 2.0 ***/
																							FFO_ORDR_RFRNC, (char *)c_cover_ref, /*** Added in Ver 2.0 ***/
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
																							FFO_SYS_MSG, (char *)c_sys_msg);

						if ( i_returncode != SUCC_BFR )
						{
							fn_errlog( c_ServiceName, "S31420", LIBMSG, c_err_msg  );
							tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
						}
           }
					}
					} 
				 } /*** if loop ends Ver 2.0 ***/
					break;

        case ORS_CAN_ORD_RJCT:
          break;

        case ORS_ORD_EXP:
          if(c_fc_flg == 'F' || c_fc_flg == 'C' ) /*Fresh*/   /** Cover added in Ver 2.6 **/
          {
            d_amt_blkd = 0.0;
            d_lss_amt_blkd = 0.0;
            c_ordr_stts = '\0';

            EXEC SQL
               SELECT     NVL(FOD_AMT_BLCKD,0),
                          NVL(FOD_LSS_AMT_BLCKD,0),
                          FOD_ORDR_STTS,
                          FOD_ORDR_FLW,
													FOD_ORDR_RFRNC,
													FOD_SLTP_ORDR_RFRNC,
													FOD_LMT_RT
               INTO       :d_amt_blkd,
                          :d_lss_amt_blkd,
                          :c_ordr_stts,
                          :c_ordr_flw,
													:st_orderbook.c_ordr_rfrnc,
                          :c_sltp_ord_rfrnc,
                          :li_lmt_rt
               FROM       FOD_FO_ORDR_DTLS
               WHERE      FOD_ORDR_RFRNC = decode(:c_fc_flg,'F',:st_orderbook.c_ordr_rfrnc,:c_sltp_ord_rfrnc); /*added in Ver2.6 **/

            if(SQLCODE != 0)
            {
              fn_errlog ( c_ServiceName, "S31425", SQLMSG, ptr_st_err_msg->c_err_msg );
              fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }

            if(DEBUG_MSG_LVL_3)		
            {
              fn_userlog(c_ServiceName,"Margin Amount of the fresh order :%lf:", d_amt_blkd);
              fn_userlog(c_ServiceName,"Loss Amount of the fresh order :%lf:", d_lss_amt_blkd);
            }

            /*Get the current margin percentage from the database *Commented in ver 2.6 *
            EXEC SQL
             SELECT     FMM_SLTP_PRCNT
             INTO       :d_fmm_sltp_prcnt
             FROM       FMM_FO_MRGN_MSTR
             WHERE      FMM_XCHNG_CD = :st_orderbook.c_xchng_cd
             AND        FMM_PRDCT_TYP ='O'
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
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }


						 ** Commented in Ver 2.6 **

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
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
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

						*** Comment ends for Ver 2.6  **

            li_lmt_rt = 0;

            EXEC SQL
             SELECT     NVL(FOD_LMT_RT,0),
												NVL(FOD_STP_LSS_TGR,0)
             INTO       :li_lmt_rt,
											  :l_temp_stp_lss_tgr
             FROM       FOD_FO_ORDR_DTLS
             WHERE      FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

            if(SQLCODE != 0)
            {
              fn_errlog ( c_ServiceName, "S31440", SQLMSG, ptr_st_err_msg->c_err_msg );
              fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }

            * Calculate open quantity *      commmented in 2.6 **
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

						* Get the minimum difference percentage ***

            EXEC SQL
               SELECT nvl(FRM_DIFFRNC_MRGN,0)
               INTO   :d_frm_diffrnc_mrgn
               FOM   FRM_FO_RANGE_MSTR
               WHERE  FRM_XCHNG_CD   = :st_orderbook.c_xchng_cd
               AND    FRM_UNDRLYNG   = :st_orderbook.c_undrlyng
               AND    FRM_ORDR_FLOW  = :c_ordr_flw
							 AND    FRM_FROM_RANGE <= :l_temp_stp_lss_tgr
               AND    :l_temp_stp_lss_tgr < FRM_TO_RANGE;

            if ( SQLCODE  !=  0  && SQLCODE != NO_DATA_FOUND)
            {
              fn_errlog( c_ServiceName, "S31450",SQLMSG,c_err_msg);
              fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }

            * Calculate Margin amount to be blocked *
            d_amt_to_be_blckd = ((double)(l_open_qty * d_wgt_avrg_price) * d_fmm_sltp_prcnt / 100.0);

            * Calculate Loss amount to be blocked *
            if ( c_ordr_flw == BUY )
            {
							if( d_frm_diffrnc_mrgn < 100 )
              {
              	d_lss_amt_to_be_blckd = (double) (l_open_qty * ( li_lmt_rt - d_wgt_avrg_price) );
							}
							else if ( d_frm_diffrnc_mrgn == 100 )
              {
              	d_lss_amt_to_be_blckd = (double) (l_open_qty * d_wgt_avrg_price );
              }
            }
            else if ( c_ordr_flw == SELL )
            {
              d_lss_amt_to_be_blckd = (double) (l_open_qty * (d_wgt_avrg_price - li_lmt_rt) );
            }


						** Commented in Ver 2.6 **/


						 /** Added in Ver 2.6 Starts **/

						 long int li_sltp_lmt_rt = 0;

            EXEC SQL
            SELECT      NVL(FOD_LMT_RT,0)
            INTO        :li_sltp_lmt_rt
            FROM        FOD_FO_ORDR_DTLS
            WHERE       FOD_ORDR_RFRNC = :c_sltp_ord_rfrnc;

            if(SQLCODE != 0)
            {
              fn_errlog ( c_ServiceName, "S31455", SQLMSG, ptr_st_err_msg->c_err_msg );
              fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
              tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
            }

             i_returncode = fn_cal_mrgn_sltp_op ( c_ServiceName,
                                               st_orderbook.c_ordr_rfrnc,
                                               li_lmt_rt,
                                               li_sltp_lmt_rt,
                                               &d_amt_to_be_blckd,
                                               &d_lss_amt_to_be_blckd,
                                          		 c_ref_rmrks,  /*** Ver 5.1***/
                                          		 &d_comp1_mrgn,/*** Ver 5.1***/
                                          		 &d_comp1_pl,  /*** Ver 5.1***/
                                          		 &d_xchng_amt, /*** Ver 5.1***/
                                          		 &d_prem_amt,  /*** Ver 5.1***/
                                          		 &d_spn_mrgn,  /*** Ver 5.1***/
                                           		 &d_expr_mrgn, /*** Ver 5.1***/
																							 'E',
                                               c_err_msg
                                             );
  						if(DEBUG_MSG_LVL_0)   /** Ver 2.8 **/
  						{
            	  fn_userlog(c_ServiceName," d_amt_to_be_blckd is :%lf:",d_amt_to_be_blckd);
            	  fn_userlog(c_ServiceName," d_lss_amt_to_be_blckd is :%lf:",d_lss_amt_to_be_blckd);
							}

              if ( i_returncode != 0 )
              {
                 fn_errlog ( c_ServiceName, "S31460", LIBMSG, ptr_st_err_msg->c_err_msg );
                 fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                 tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }


            d_lss_amt_to_be_blckd = d_lss_amt_to_be_blckd  * -1;

             /** Added in Ver 2.6 Ends **/


            /* Difference margin & loss */
            d_diff_amt  = d_amt_blkd - d_amt_to_be_blckd;
            d_diff_loss = d_lss_amt_blkd + d_lss_amt_to_be_blckd;
			
						d_diff_amt = (long) (long) d_diff_amt ;
						d_diff_loss = (long) (long) d_diff_loss ;

            if(DEBUG_MSG_LVL_3)
            {
              fn_userlog(c_ServiceName,"Cover li_lmt_rt :%ld",li_lmt_rt);
              fn_userlog(c_ServiceName,"Fresh Open qty = :%ld",l_open_qty);
						}
						if(DEBUG_MSG_LVL_3) /**  Ver 2.8 **/
            {
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
                  fn_errlog ( c_ServiceName, "S31465", LIBMSG, ptr_st_err_msg->c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }

                d_amt_to_be_blckd = d_amt_to_be_blckd + d_balance_amt ;
              }

            /* Block Release the loss amount */
            strcpy( c_narration_id , ON_EXPIRY);

	           /*** Ver 5.1 started ***/
      			MEMSET(c_tmp_rmrks);
      			strcpy ( c_tmp_rmrks, st_pstn_actn.c_ref_rmrks );
      			sprintf(st_pstn_actn.c_ref_rmrks,"%s%s",c_ref_rmrks,c_tmp_rmrks);
      			/*** Ver 5.1 ends ***/

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
                  fn_errlog ( c_ServiceName, "S31470", LIBMSG, ptr_st_err_msg->c_err_msg );
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
                  fn_errlog ( c_ServiceName, "S31475", LIBMSG, ptr_st_err_msg->c_err_msg );
                  fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                  tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
                }
              }
               d_lss_amt_to_be_blckd = 0;
            }

            strcpy ( st_pstn_actn.c_ref_rmrks, c_tmp_rmrks);    /**** Ver 5.1 ***/
      			MEMSET(c_tmp_rmrks);                                    /**** Ver 5.1 ***/

            EXEC SQL
               UPDATE     FOD_FO_ORDR_DTLS
               SET        FOD_AMT_BLCKD = :d_amt_to_be_blckd,
                          FOD_LSS_AMT_BLCKD = abs(:d_lss_amt_to_be_blckd),
 						              FOD_SLTP_MRGN = :d_comp1_mrgn,/*** Ver 5.1 ***/
              			      FOD_SLTP_PL   = :d_comp1_pl,  /*** Ver 5.1 ***/
              						FOD_SLTP_SEBI_MRGN = :d_xchng_amt, /*** Ver 5.1 ***/
              					  FOD_OPLUS_PREMIUM  = :d_prem_amt,  /*** Ver 5.1 ***/
              						FOD_SPAN_MRGN = :d_spn_mrgn,       /*** Ver 5.1 ***/
              						FOD_EXPR_MRGN = :d_expr_mrgn       /*** Ver 5.1 ***/
               WHERE      FOD_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc;

              if(SQLCODE != 0)
              {
                fn_errlog ( c_ServiceName, "S31480", LIBMSG, ptr_st_err_msg->c_err_msg );
                fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
              }
          }
          break;
    }
 }
 /*** Ver 1.8 ends here ***/
/** below function added in ver 5.5 ***/
if ( st_orderbook.c_prd_typ == 'O' )
{ 
  if( ptr_st_xchngbook->l_ors_msg_typ == ORS_NEW_ORD_RJCT )
  {
      c_fop_mtm_flg = '\0'; 
      EXEC SQL      
         select NVL(FOP_MTM_FLG,'O')
           into :c_fop_mtm_flg         
         FROM   FOP_FO_OPTIONS_PSTN
         WHERE  FOP_XCHNG_CD = :st_pstn_actn.c_xchng_cd
         AND    FOP_CLM_MTCH_ACCNT =:st_pstn_actn.c_cln_mtch_accnt
         AND    FOP_PRDCT_TYP       = :st_pstn_actn.c_prd_typ
         AND    FOP_OPT_TYP         = :st_pstn_actn.c_opt_typ
         AND    FOP_EXER_TYP        = :st_pstn_actn.c_exrc_typ
         AND    FOP_UNDRLYNG        = :st_pstn_actn.c_undrlyng
         AND    FOP_EXPRY_DT        = :st_pstn_actn.c_expry_dt
         AND    FOP_STRK_PRC        = :st_pstn_actn.l_strike_prc;
         if(SQLCODE != 0)
         {
            fn_errlog ( c_ServiceName, "S31485", LIBMSG, ptr_st_err_msg->c_err_msg );
            fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg );
            tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
         }
      fn_userlog( c_ServiceName,"c_fop_mtm_flg :%c:",c_fop_mtm_flg); 
      if(DEBUG_MSG_LVL_3)
      {
       fn_userlog( c_ServiceName,"INSIDE REJ CASE");
      }
      if( c_fop_mtm_flg == 'D' )
      {
      i_returncode  = fn_unmark_lodm_rej_nspn( c_ServiceName , &st_pstn_actn ,c_err_msg);

      if( i_returncode != 0 )
      {
        fn_userlog ( c_ServiceName, "ERROR IN fn_unmark_lodm_rej_nspn");
        fn_errlog ( c_ServiceName, "S31490", LIBMSG, c_err_msg );
        fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ); 
        tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
      }
      }  

  }
}
	if ( i_commit_flg == 0 ) /*** Ver 1.8 ***/
	{
		if ( fn_committran( c_ServiceName, i_trnsctn, 
																			 ptr_st_err_msg->c_err_msg ) == -1 )
		{
			fn_errlog ( c_ServiceName, "S31495", LIBMSG, c_err_msg );
      fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ); /*** Added in Ver 3.4 ***/
			tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
		}
		
		/***Ver 5.7 call to func ***/

  i_returncode = fn_upd_nrmlmrgn_reporting(st_orderbook.c_cln_mtch_accnt,
                                       st_orderbook.c_xchng_cd,
                                       c_ServiceName,
                                       ptr_st_err_msg->c_err_msg
                                      );
  if( i_returncode != 0 )
  {
    fn_errlog(c_ServiceName, "S31500", LIBMSG, ptr_st_err_msg->c_err_msg );
    fn_userlog(c_ServiceName,"Error while calling function fn_upd_mrgn_reporting");
    tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
  }
  /***Ver 5.7 ends **/

	}
	
	i_commit_flg = 0; /*** Ver 1.8 ***/

	  /********* Ver 2.6 Start ***********/

  if (st_orderbook.c_prd_typ == OPTIONPLUS)
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
      fn_errlog ( c_ServiceName, "S31505", SQLMSG, ptr_st_err_msg->c_err_msg );
      tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
    }

    i_status = 0;

			/*** Commented in ver 2.8 ***
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

		*** Commented in ver 2.8 ***/

		/*** 	Added in ver 2.8 ***/
		EXEC SQL	
		SELECT FXB_ORS_MSG_TYP,
					 FXB_MDFCTN_CNTR,
					 nvl(to_char(FXB_ACK_TM, 'dd-mon-yyyy hh24:mi:ss'),'*'),
					 NVL(FXB_XCHNG_CNCLD_QTY,0)																						/***** Ver 3.4 ***/
     INTO  :st_cvr_xchngbk.l_ors_msg_typ,
         	 :st_cvr_xchngbk.l_mdfctn_cntr,
           :st_cvr_xchngbk.c_ack_tm,
					 :st_cvr_xchngbk.l_xchng_can_qty      													 /***** Ver 3.4 ***/
		FROM   FXB_FO_XCHNG_BOOK
		WHERE  FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc 
		AND    FXB_MDFCTN_CNTR = (
    													SELECT min(FXB_MDFCTN_CNTR)
    													FROM   FXB_FO_XCHNG_BOOK
    													WHERE  FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc 
    													AND    FXB_JIFFY = (
																									select min(FXB_JIFFY)
     	   																					from   FXB_FO_XCHNG_BOOK
     	   																					where  FXB_ORDR_RFRNC = :c_sltp_ord_rfrnc 
     	   																					AND    FXB_RMS_PRCSD_FLG = 'N'
     	   								 												 )
    													AND FXB_RMS_PRCSD_FLG = 'N'
														 )
		AND    FXB_RMS_PRCSD_FLG = 'N';

		/***   Added in ver 2.8 	***/

    if(SQLCODE !=0)
    {
      /** i_status = 1; ** Commented in Ver 2.8 ***/
      fn_userlog( c_ServiceName, "No opposite leg unprocessed record");
      fn_userlog( c_ServiceName, "Checking for Same leg "); /*** Added in Ver 2.8 ***/
      fn_errlog ( c_ServiceName, "S31510", SQLMSG, ptr_st_err_msg->c_err_msg );

    /***************** Added in Ver 2.8 Starts *****************/

      MEMSET(st_cvr_xchngbk.l_ors_msg_typ);
      MEMSET(st_cvr_xchngbk.l_mdfctn_cntr);
      MEMSET(st_cvr_xchngbk.c_ack_tm);

      EXEC SQL
      SELECT FXB_ORS_MSG_TYP,
             FXB_MDFCTN_CNTR,
             nvl(to_char(FXB_ACK_TM, 'dd-Mon-yyyy hh24:mi:ss'),'-'),
	           NVL(FXB_XCHNG_CNCLD_QTY,0)                                       /***** Ver 3.4 ***/
      INTO   :st_cvr_xchngbk.l_ors_msg_typ,
             :st_cvr_xchngbk.l_mdfctn_cntr,
             :st_cvr_xchngbk.c_ack_tm,
           	 :st_cvr_xchngbk.l_xchng_can_qty                                 /***** Ver 3.4 ***/
      FROM   FXB_FO_XCHNG_BOOK
      WHERE  FXB_ORDR_RFRNC =  :st_orderbook.c_ordr_rfrnc
      AND    FXB_JIFFY  = (select min(FXB_JIFFY )  					
      										 from   FXB_FO_XCHNG_BOOK
      										 where  FXB_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc 
													 AND    FXB_RMS_PRCSD_FLG = 'N'  
													)
      AND    FXB_RMS_PRCSD_FLG = 'N';

      if(SQLCODE !=0)
      {
        i_status = 1;
        fn_userlog( c_ServiceName, "No Same leg to process ");
        fn_errlog ( c_ServiceName, "S31515", SQLMSG, ptr_st_err_msg->c_err_msg );
      }
      else
      {
        fn_userlog( c_ServiceName, "Same leg unprocessed ... so process it...");
        strcpy(st_cvr_xchngbk.c_ordr_rfrnc, st_orderbook.c_ordr_rfrnc );
        fn_userlog( c_ServiceName, "Call SFO_OPT_CVRACK for Order Ref:%s:", st_orderbook.c_ordr_rfrnc );
      }
      /***************** Added in Ver 2.8 Ends *********************/
    }
    else
    {
      fn_userlog( c_ServiceName, "Opposite leg unprocessed record exist so process it...");
      strcpy(st_cvr_xchngbk.c_ordr_rfrnc, c_sltp_ord_rfrnc)  ;                                /** Added in Ver 2.8 ***/
      fn_userlog( c_ServiceName, "Call SFO_OPT_CVRACK for Order Ref:%s:", c_sltp_ord_rfrnc);  /** Added in Ver 2.8 ***/
    }

    strcpy(st_cvr_xchngbk.c_entry_dt_tm , st_cvr_xchngbk.c_ack_tm);

    if(i_status != 1)
    {
      /*** commented  in  Ver 2.8 ***
			if(DEBUG_MSG_LVL_0)
      {
        fn_userlog( c_ServiceName, "Before Calling  SFO_OPT_CVRACK for Order Ref:%s:", c_sltp_ord_rfrnc);
      }

      fn_userlog( c_ServiceName, "Call SFO_OPT_CVRACK for Order Ref:%s:", c_sltp_ord_rfrnc);
			
			** commented  in  Ver 2.8 ***/
		
      fn_cpy_ddr ( st_cvr_xchngbk.c_rout_crt );
  		
			if(DEBUG_MSG_LVL_3)   /** Ver 2.8 **/
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
       /*** strcpy(st_cvr_xchngbk.c_ordr_rfrnc, c_sltp_ord_rfrnc);  commented  in  Ver 2.8 ***/

		  /******** Added in Ver 2.8 ****** Commented in Ver 3.4 ***************

      EXEC SQL
      SELECT  NVL(SUM(ftd_exctd_qty),0)
      INTO    :l_ftd_ord_exe_qty
      FROM    ftd_fo_trd_dtls
      WHERE   ftd_ordr_rfrnc = :st_cvr_xchngbk.c_ordr_rfrnc ;

      if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
      {
        fn_errlog ( c_ServiceName, "S31520", SQLMSG, ptr_st_err_msg->c_err_msg );
        fn_aborttran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ); 
        tpreturn(TPFAIL, ERR_BFR, (char *)&ptr_st_err_msg, 0, 0 );
      }

 		 if(DEBUG_MSG_LVL_3)   ** Ver 2.8 **
  	 {
      fn_userlog( c_ServiceName, "l_ftd_ord_exe_qty: %ld",l_ftd_ord_exe_qty);
		 }
      st_cvr_xchngbk.l_xchng_can_qty = l_ftd_ord_exe_qty;

			*************** Commented Ver 3.4 Ends ****************/

      /******** Added in Ver 2.8 ******/

       i_returncode = fn_acall_svc ( c_ServiceName,
                     c_err_msg,
                     &st_cvr_xchngbk,
                     "vw_xchngbook",
                     sizeof ( st_cvr_xchngbk ),
                     TPNOREPLY,
                     "SFO_OPT_CVRACK" );
        if ( i_returncode != 0 )
        {
          fn_userlog( c_ServiceName, "Error in :%d:",i );
          fn_errlog( c_ServiceName, "S31525", Fstrerror32( i_ferr[i]),c_err_msg );
          /*** tpfree((char *)ptr_fml_Sbuf);
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Sbuf, 0, 0 ); Commented in Ver 3.4 ***/
					tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); /*** Added in Ver 3.4 ***/
        }

      if(DEBUG_MSG_LVL_0)
      {
        /** fn_userlog( c_ServiceName, "After Calling  SFO_OPT_CVRACK for Order Ref:%s:", c_sltp_ord_rfrnc); *commented in Ver 2.8 **/
        fn_userlog( c_ServiceName, "After Calling  SFO_OPT_CVRACK for Order Ref:%s:", st_cvr_xchngbk.c_ordr_rfrnc); /*** Added in Ver 2.8 ***/
      }
    }
  }

  /******************** Ver 2.6 End *******************/

	/******** Added in Ver 2.8 Starts ************/

  else if(st_orderbook.c_prd_typ == OPTIONS )  
  {
    i_status = 0;
    MEMSET(st_cvr_xchngbk.l_ors_msg_typ);
    MEMSET(st_cvr_xchngbk.l_mdfctn_cntr);
    MEMSET(st_cvr_xchngbk.c_ack_tm);


		EXEC SQL
		SELECT FXB_ORS_MSG_TYP,
					 fXB_MDFCTN_CNTR,
					 nvl(to_char(FXB_ACK_TM, 'dd-mon-yyyy hh24:mi:ss'),'*'),
           NVL(FXB_XCHNG_CNCLD_QTY,0)                                        /***** Ver 3.4 ***/
    INTO   :st_cvr_xchngbk.l_ors_msg_typ,
           :st_cvr_xchngbk.l_mdfctn_cntr,
           :st_cvr_xchngbk.c_ack_tm,
           :st_cvr_xchngbk.l_xchng_can_qty                                 /***** Ver 3.4 ***/
		FROM   FXB_FO_XCHNG_BOOK
		WHERE  FXB_ORDR_RFRNC =  :st_orderbook.c_ordr_rfrnc
    AND			    FXB_JIFFY = (
														select  min(FXB_JIFFY)
     												from   	FXB_FO_XCHNG_BOOK
     												where   FXB_ORDR_RFRNC = :st_orderbook.c_ordr_rfrnc 
     												AND    FXB_RMS_PRCSD_FLG = 'N'
														)		
		AND    FXB_RMS_PRCSD_FLG = 'N';

    if(SQLCODE !=0)
    {
      i_status = 1;
      fn_userlog( c_ServiceName, "No Same leg to process ");
      fn_errlog ( c_ServiceName, "S31530", SQLMSG, ptr_st_err_msg->c_err_msg );

    }
    else
    {
      fn_userlog( c_ServiceName, "Same leg unprocessed ... so process it...");
      strcpy(st_cvr_xchngbk.c_ordr_rfrnc, st_orderbook.c_ordr_rfrnc );
      fn_userlog( c_ServiceName, "Call SFO_OPT_CVRACK for Order Ref:%s:", st_orderbook.c_ordr_rfrnc );
    }
		if((st_cvr_xchngbk.l_ors_msg_typ == ORS_2LN_ORD_ACPT)|| (st_cvr_xchngbk.l_ors_msg_typ == ORS_3LN_ORD_ACPT) || 
			(st_cvr_xchngbk.l_ors_msg_typ == ORS_2L_ORD_CNCL) || (st_cvr_xchngbk.l_ors_msg_typ == ORS_3L_ORD_CNCL))
  	{
			MEMSET(c_ipord_ref);
			MEMSET(l_ors_msg_typ);

			strcpy(c_ipord_ref,st_orderbook.c_ordr_rfrnc);
			l_ors_msg_typ = st_cvr_xchngbk.l_ors_msg_typ;

      i_returncode = fn_spd_acpt(c_ServiceName,c_ipord_ref,l_ors_msg_typ);
      if( i_returncode == -1)
      {
       fn_errlog(c_ServiceName, "S31535", LIBMSG,  ptr_st_err_msg->c_err_msg);
       /*** free((char*) ptr_fml_Sbuf);
       tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Sbuf, 0, 0 ); Commented in Ver 3.4 ***/
       tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); /*** Added in Ver 3.4 ***/
      }
    }
			
    if((i_status != 1 ) && (st_cvr_xchngbk.l_ors_msg_typ != ORS_2LN_ORD_ACPT) && (st_cvr_xchngbk.l_ors_msg_typ != ORS_3LN_ORD_ACPT) && (st_cvr_xchngbk.l_ors_msg_typ != ORS_2L_ORD_CNCL) && (st_cvr_xchngbk.l_ors_msg_typ != ORS_3L_ORD_CNCL))
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

			/************ Commented in Ver 3.4  *******************
		  MEMSET(l_ftd_ord_exe_qty);

      EXEC SQL
      SELECT  NVL(SUM(ftd_exctd_qty),0)
      INTO    :l_ftd_ord_exe_qty
      FROM    ftd_fo_trd_dtls
      WHERE   ftd_ordr_rfrnc = :st_cvr_xchngbk.c_ordr_rfrnc ;

      if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
      {
        fn_errlog ( c_ServiceName, "S31540", SQLMSG, ptr_st_err_msg->c_err_msg );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Sbuf, 0, 0 );
      }
	

			
			if(DEBUG_MSG_LVL_3)   ** Ver 2.8 **
  		{
    		fn_userlog( c_ServiceName, "l_ftd_ord_exe_qty: %ld",l_ftd_ord_exe_qty);
      	fn_userlog( c_ServiceName, "st_cvr_xchngbk.can_qty:%ld:",st_cvr_xchngbk.l_xchng_can_qty);
			}
      st_cvr_xchngbk.l_xchng_can_qty = l_ftd_ord_exe_qty ;

			*********** Commented Part Ends Ver 3.4 ****************/

       i_returncode = fn_acall_svc ( c_ServiceName,
                     c_err_msg,
                     &st_cvr_xchngbk,
                     "vw_xchngbook",
                     sizeof ( st_cvr_xchngbk ),
                     TPNOREPLY,
                     "SFO_OPT_CVRACK" );

        if ( i_returncode != 0 )
        {
          fn_userlog( c_ServiceName, "Error in :%d:",i );
          fn_errlog( c_ServiceName, "S31545", Fstrerror32( i_ferr[i]),c_err_msg );
          /*** tpfree((char *)ptr_fml_Sbuf);
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_Sbuf, 0, 0 ); Commented in Ver 3.4 ***/
          tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); /*** Added in Ver 3.4 ***/
        }
		
      if(DEBUG_MSG_LVL_0)
      {
        fn_userlog( c_ServiceName, "After Calling  SFO_OPT_CVRACK for Order Ref:%s:", st_orderbook.c_ordr_rfrnc);
      }
    }
 }

  /******** Added in Ver 2.8 Ends ************/

	fn_gettime();

/************* Ver 1.7 Starts
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
      	fn_errlog ( c_ServiceName, "S31550", LIBMSG, c_err_msg );
      	tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
    	}
		}
  }
***********ver 1.7 Ends **/

	/************************* Commented in Ver 2.7 **************************

	if ( st_orderbook.c_prd_typ != OPTIONPLUS )	*** Ver Condiation added in 2.6 ***
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
	else									*** Ver 2.6 starts ***
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
		WHERE		ftd_ordr_rfrnc IN(:ptr_st_xchngbook->c_ordr_rfrnc,  :c_sltp_ord_rfrnc)
		AND			ftd_rms_prcsd_flg = 'N';

	EXEC SQL
		OPEN	cur_trd1;
	}
	*** Ver 2.6 ends ***

	if ( SQLCODE != 0 )
	{
  	fn_errlog ( c_ServiceName, "S31555", SQLMSG, ptr_st_err_msg->c_err_msg );
	if ( st_orderbook.c_prd_typ != OPTIONPLUS )	*** Ver Condiation added in 2.6 ***
	{
		EXEC SQL 
			CLOSE cur_trd;
	}

	*** Ver 2.6 starts ***
	else
	{
		EXEC SQL 
		CLOSE cur_trd1;
	}
	*** Ver 2.6 ends ***
	tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
	}

	while (1)
	{
						if ( st_orderbook.c_prd_typ != OPTIONPLUS )	*** Ver Condiation added in 2.6 ***
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
		else			*** Ver 2.6 starts ***
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
		*** Ver 2.6 ends ***
		if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
		{
  		fn_errlog ( c_ServiceName, "S31560", SQLMSG, ptr_st_err_msg->c_err_msg );
		if ( st_orderbook.c_prd_typ != OPTIONPLUS )	 *** Ver Condiation added in 2.6 ***
		{
			EXEC SQL 
				CLOSE cur_trd;
		}
		else		*** Ver 2.6 starts ***
		{
			EXEC SQL 
				CLOSE cur_trd1;
		}
		*** Ver 2.6 starts ***
  		tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
		}

		if ( SQLCODE == NO_DATA_FOUND )
		{
			break;
		}

		rtrim(st_tradebook.c_trd_dt);
  	fn_userlog(c_ServiceName,"Stage 1"); 

  	*** Added for Order Routing ***
		fn_cpy_ddr ( st_tradebook.c_rout_crt );

		i_returncode = fn_acall_svc( c_ServiceName,
                                ptr_st_err_msg->c_err_msg,
                                &st_tradebook,
                                "vw_tradebook",
                                sizeof( struct vw_tradebook),
                                TPNOREPLY,
                                "SFO_OPT_TCONF" );
		if ( i_returncode != SUCC_BFR )
		{
			fn_errlog ( c_ServiceName, "S31565", LIBMSG, c_err_msg );
		if ( st_orderbook.c_prd_typ != OPTIONPLUS )	*** Ver Condiation added in 2.6 ***
		{
			EXEC SQL
				CLOSE cur_trd;
		}
		else
		{
			EXEC SQL 
				CLOSE cur_trd1;
		}
  		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
		}

	}

	if ( st_orderbook.c_prd_typ != OPTIONPLUS )		*** Ver Condiation added in 2.6 ***
	{
		EXEC SQL
		CLOSE cur_trd;
	}
	else
	{
		EXEC SQL
		CLOSE cur_trd1;
	}
	********************* Comment Ends Ver 2.7 ********************/
	
	/***************** Ver 2.7 Added *** Starts *****************/

  /*** Commented in Ver 3.2
  i_returncode = fn_prcstrd(&st_orderbook,c_ServiceName,c_err_msg);
  ************/

  /*** Ver 3.2 Starts Here ***/
  i_returncode = fn_prcs_trd(c_ServiceName,c_err_msg,st_orderbook.c_prd_typ,st_orderbook.c_ordr_rfrnc);
  /*** Ver 3.2 Ends Here ***/

  if ( i_returncode != 0 )
  {
    fn_errlog ( c_ServiceName, "S31570", LIBMSG, c_err_msg );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
  }

	/***************** Ver 2.7 *** Ends *****************/
  fn_userlog( c_ServiceName,"st_pstn_actn.c_mtm_flag is kiran :%c:",st_pstn_actn.c_mtm_flag);
 
	if ( st_orderbook.c_prd_typ != OPTIONPLUS ) /*** if condition added in Ver 1.8 ***/
  {	
	if ( st_pstn_actn.c_mtm_flag == NO_OPEN_ORDERS )
	{
    switch ( ptr_st_xchngbook->l_ors_msg_typ )
    {
      case  ORS_NEW_ORD_ACPT:
          sprintf(st_pstn_actn.c_imtm_rmrks,
          "RECEIVED SQUAREOFF CONFIRMATION FOR [%s]",
                                            ptr_st_xchngbook->c_ordr_rfrnc );
          break;

      case  ORS_NEW_ORD_RJCT:
          sprintf(st_pstn_actn.c_imtm_rmrks,
          "RECEIVED SQUAREOFF REJECTION FOR [%s]",
                                            ptr_st_xchngbook->c_ordr_rfrnc );
          break;

      case  ORS_CAN_ORD_ACPT:
          sprintf(st_pstn_actn.c_imtm_rmrks,
          "RECEIVED CANCELLATION ACCEPTANCE FOR [%s]",
                                            ptr_st_xchngbook->c_ordr_rfrnc );
          break;

      case  ORS_CAN_ORD_RJCT:
          sprintf(st_pstn_actn.c_imtm_rmrks,
          "RECEIVED CANCELLATION REJECTION FOR [%s]",
                                            ptr_st_xchngbook->c_ordr_rfrnc );
          break;

			/******* default ???? ************/
    }

    sprintf( c_tag,"bat_opt_imtm_%s",st_pstn_actn.c_xchng_cd);

    /**************************************
    fn_cpy_ddr ( st_pstn_actn.c_rout_crt );
    **************************************/

    i_returncode = fn_acall_svc_fml ( c_ServiceName,
                                  		ptr_st_err_msg->c_err_msg,
                                  		"SFO_PRCS_OIMTM",
                                  		/***0, *** Ver 3.5 ***/
                                      TPNOTRAN|TPNOREPLY, /*** Ver 3.5 ***/
                                  		12,
                 		FFO_XCHNG_CD, ( char * )st_pstn_actn.c_xchng_cd,
                 		FFO_EBA_MTCH_ACT_NO, ( char *)st_pstn_actn.c_cln_mtch_accnt,
                 		FFO_PRDCT_TYP, ( char * )&st_pstn_actn.c_prd_typ,
                		FFO_UNDRLYNG, ( char * )st_pstn_actn.c_undrlyng,
                		FFO_CTGRY_INDSTK, ( char * )&st_pstn_actn.c_ctgry_indstk,
                		FFO_EXPRY_DT, ( char * )st_pstn_actn.c_expry_dt,
                		FFO_STRK_PRC, ( char * )&st_pstn_actn.l_strike_prc,
                		FFO_EXER_TYP, ( char * )&st_pstn_actn.c_exrc_typ,
                		FFO_OPT_TYP, ( char * )&st_pstn_actn.c_opt_typ,
                		FFO_IMTM_RMRKS, ( char * )st_pstn_actn.c_imtm_rmrks,
                		FFO_PIPE_ID, ( char * )st_xchngbook.c_pipe_id,
                		FFO_BAT_TAG, ( char * )c_tag);

		if ( i_returncode != SUCC_BFR )
		{
			fn_errlog ( c_ServiceName, "S31575", LIBMSG, c_err_msg );
  		tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
		}
	}
  else if ( strstr( c_ref_rmrks_phy,"Provisional Delivery Process Cancellation")) /** ver 5.3 loop added **/
  {
      /**c_span_flg = 'S'; commented in ver 5.4 **/
      c_span_flg = 'N'; /*** ver 5.4 **/
      c_rqst_typ = 'I';
      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog( c_ServiceName,"STRSTR :%s:%s:%c:%c:%s:%c:%ld:%s:%c:%c:",st_pstn_actn.c_cln_mtch_accnt,st_pstn_actn.c_xchng_cd,st_pstn_actn.c_prd_typ,st_pstn_actn.c_ctgry_indstk,st_pstn_actn.c_undrlyng,st_pstn_actn.c_opt_typ,st_pstn_actn.l_strike_prc,st_pstn_actn.c_expry_dt,st_pstn_actn.c_exrc_typ,c_ordr_flow_phy );
      }

      EXEC SQL
            SELECT decode(:c_ordr_flow_phy,'B',greatest(abs(FOP_IBUY_QTY),abs(FOP_EXBUY_QTY)) ,'S',greatest(abs(FOP_ISELL_QTY),abs(FOP_EXSELL_QTY) ),0) ,
            FOP_OPNPSTN_FLW
            INTO :l_prvsnl_pending_qty,
                 :c_prvsnl_opnpos_flow
             FROM FOP_FO_OPTIONS_PSTN 
        WHERE FOP_CLM_MTCH_ACCNT =:st_pstn_actn.c_cln_mtch_accnt
        AND   FOP_XCHNG_CD       =:st_pstn_actn.c_xchng_cd
        AND   FOP_PRDCT_TYP      =:st_pstn_actn.c_prd_typ
        AND   FOP_INDSTK         =:st_pstn_actn.c_ctgry_indstk
        AND   FOP_UNDRLYNG       =:st_pstn_actn.c_undrlyng
        AND   FOP_OPT_TYP        =:st_pstn_actn.c_opt_typ
        AND   FOP_STRK_PRC       =:st_pstn_actn.l_strike_prc
        AND   FOP_EXPRY_DT       =:st_pstn_actn.c_expry_dt
        AND   FOP_EXER_TYP       =:st_pstn_actn.c_exrc_typ
        AND   FOP_OPNPSTN_FLW    = :c_ordr_flow_phy;

   if ( SQLCODE != 0 )
   {
       fn_errlog ( c_ServiceName, "S31580", SQLMSG, c_err_msg );
       tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
   }

   if ( st_pstn_actn.c_prd_typ == 'O')
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
         fn_errlog ( c_ServiceName, "S31585", LIBMSG, c_err_msg );
         tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
        } 
     }
  }
	/** Ver 4.6 Starts Here ***/
  else if ( (st_pstn_actn.c_mtm_flag == NO_PHYDLVRY_OPEN_ORDERS ) && ( ptr_st_xchngbook->l_ors_msg_typ != ORS_NEW_ORD_RJCT ))/** ORS_NEW_ORD_RJCT added in ver 5.5 **/
  {
     fn_userlog(c_ServiceName,"INSIDE NO_PHYDLVRY_OPEN_ORDERS");
     c_rqst_typ='I';
    /*** call service for Dlvry ***/
    i_returncode = fn_acall_svc_fml ( c_ServiceName,
                                      c_err_msg,
                                      "SFO_DLVMRG_PRC",
                                      TPNOTRAN|TPNOREPLY,
                                      11,
                                      FFO_EBA_MTCH_ACT_NO, ( char *)st_pstn_actn.c_cln_mtch_accnt,
                                      FFO_XCHNG_CD, ( char * )st_pstn_actn.c_xchng_cd,
                                      FFO_PRDCT_TYP, ( char * )&st_pstn_actn.c_prd_typ,
                                      FFO_UNDRLYNG, ( char * )st_pstn_actn.c_undrlyng,
                                      FFO_EXPRY_DT, ( char * )st_pstn_actn.c_expry_dt,
                                      FFO_EXER_TYP, ( char * )&st_pstn_actn.c_exrc_typ,
                                      FFO_OPT_TYP, ( char * )&st_pstn_actn.c_opt_typ,
                                      FFO_STRK_PRC, ( char * )&st_pstn_actn.l_strike_prc,
                                      FFO_CTGRY_INDSTK, ( char * )&st_pstn_actn.c_ctgry_indstk,
                                      FFO_RQST_TYP, (char *)&c_rqst_typ,
																			FFO_STATUS_FLG,(char *)&c_span_flg );

   if ( i_returncode == -1 )
   {
      fn_errlog ( c_ServiceName, "S31590", LIBMSG, c_err_msg );
			tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 ); 
   }

  }
  /**** Ver 4.6 Ends Here ****/

  /** Ver 2.2 Starts Here ***/
  if( ptr_st_xchngbook->l_ors_msg_typ == ORS_CAN_ORD_RJCT  ||
      ptr_st_xchngbook->l_ors_msg_typ == ORS_NEW_ORD_RJCT
    )
  {
	if(DEBUG_MSG_LVL_3)   /** Ver 2.8 **/
  {
   fn_userlog(c_ServiceName," c_mtm_flg ORS_CAN_ORD_RJCT  :%c:",c_mtm_flg);
	}
   if(c_mtm_flg == 'A' ) /** Auto MTM **/
   {
     fn_upd_fmdstts(c_ServiceName,&st_pstn_actn,'N',c_err_msg);
   }
  }

  if( ptr_st_xchngbook->l_ors_msg_typ == ORS_NEW_ORD_ACPT &&
      ( strcmp(st_orderbook.c_channel, "SYS") == 0 )      &&
        st_orderbook.c_spl_flg == 'M'
    )
  {
    i_returncode = fo_ins_sqofford(c_ServiceName,c_err_msg,&st_orderbook,'I',st_pstn_actn.c_cntrct_tag);
    if( i_returncode != 0 )
    {
       fn_errlog ( c_ServiceName, "S31595", LIBMSG, c_err_msg );
       fn_userlog(c_ServiceName,"Error in fo_ins_sqofford");
    }
  }
 
  /** Ver 2.2 Ends Here **/
  
	} /*** If condition of Ver 1.8 Ends ***/
	/**** Ver 3.6 start ****/
	rtrim(st_xchngbook.c_xchng_rmrks);

	if(DEBUG_MSG_LVL_5)
	{
		fn_userlog( c_ServiceName, "c_prd_typ :%c:",st_orderbook.c_prd_typ);
		fn_userlog( c_ServiceName, "c_fc_flg :%c:",c_fc_flg);
		fn_userlog( c_ServiceName, "c_plcd_stts :%c:",st_xchngbook.c_plcd_stts);
		fn_userlog( c_ServiceName, "c_xchng_rmrks :%s:",st_xchngbook.c_xchng_rmrks);
  }

	if ( st_orderbook.c_prd_typ == OPTIONPLUS && c_fc_flg == 'C' && st_xchngbook.c_plcd_stts == CANCELLED && strncmp(st_xchngbook.c_xchng_rmrks,"The Price is out of the current execution range",45) == 0)
	{
		strcpy(c_mail_remarks,"The Price is out of the current execution range");
    sprintf(c_command1, "nohup ksh opplus_cvr_rej_mail.sh '%s' '%s' '%s' &",st_orderbook.c_cln_mtch_accnt,st_orderbook.c_ordr_rfrnc,c_mail_remarks);
		system(c_command1);
	}	
	/**** Ver 3.6 End ****/
	tpfree ( ( char * ) ptr_st_err_msg );
  tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_xchngbook, 0, 0 );
}

int fn_reftoord_forupd(struct vw_orderbook *ptr_st_orderbook,
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

  if( DEBUG_MSG_LVL_3 )
  {
  	fn_userlog( c_ServiceName,"Function REF_TO_ORD WITH FOR_UPDATE");
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
    fn_errlog ( c_ServiceName, "S31600", SQLMSG, c_err_msg );
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

int fn_ref_to_omd ( struct vw_xchngbook *ptr_i_st_xchngbook,
                    struct vw_xchngbook *ptr_st_xchngbook,
                    char  *c_mtm_flg,   /** Ver 2.2 **/
										char  *c_lst_act_ref,     /** Ver 4.2 **/
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
    short i_fxb_ip;   /* Ver 3.3 */
		char c_lst_act_ref_tmp [22];			/** Ver 4.2 **/
  EXEC SQL END DECLARE SECTION;

  char c_fxb_ip[15+1];  /** Ver 3.3 **/
  MEMSET(c_fxb_ip);     /** Ver 3.3 **/
	MEMSET(c_lst_act_ref_tmp);					/** Ver 4.2 **/

	if(DEBUG_MSG_LVL_3)   /** Ver 2.8 **/
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
             /* to_char( fxb_ack_tm, 'dd-mon-yyyy hh24:mi:ss' ),*/
							nvl(to_char(fxb_ack_tm,'dd-mon-yyyy hh24:mi:ss'),to_char(fxb_rqst_tm,'dd-mon-yyyy hh24:mi:ss')),		/*** Ver 1.5 ***/
              fxb_xchng_rmrks,
              fxb_ex_ordr_typ,
              fxb_xchng_cncld_qty,
              fxb_spl_flag,
              fxb_ip,     /** Ver 2.2 **/
							NVL(FXB_LST_ACT_REF,'0')      /** Ver 4.2 **/ /** '*' changed to '0' in ver 4.7 **/
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
              :c_fxb_ip:i_fxb_ip,  /** Ver 2.2 **/
							:c_lst_act_ref_tmp       /** Ver 4.2 **/
      FROM    fxb_fo_xchng_book
	    WHERE   fxb_ordr_rfrnc = :ptr_i_st_xchngbook->c_ordr_rfrnc
      AND     fxb_mdfctn_cntr = :ptr_i_st_xchngbook->l_mdfctn_cntr;


  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31605", SQLMSG, c_err_msg );
    return -1;
  }

	memcpy(ptr_st_xchngbook,ptr_i_st_xchngbook,sizeof(struct vw_xchngbook));

  *c_mtm_flg = c_fxb_ip[0];  /** Ver 2.2 **/
	rtrim(c_lst_act_ref_tmp);   /***  Ver 4.2 ***/
	strcpy(c_lst_act_ref,c_lst_act_ref_tmp);			/***  Ver 4.2 ***/
  if(DEBUG_MSG_LVL_4)
  {
	   fn_userlog( c_ServiceName,"Last Activity ref is :%s:",c_lst_act_ref);		 /***  Ver 4.2 ***/
  }
  return 0;
}

int fn_upd_ordrbkack (struct vw_orderbook *ptr_st_orderbook,
											char *c_lst_act_ref,    /** Ver 4.2 **/
                      char *c_ServiceName,
                      char *c_err_msg)
{

	if(DEBUG_MSG_LVL_3)   /** Ver 2.8 **/
  {
  	fn_userlog(c_ServiceName,"Inside Function Upd ORDERBK For OPT ACK");
  	fn_userlog(c_ServiceName,"Ack Time Is :%s:",ptr_st_orderbook->c_ack_tm);
  	fn_userlog(c_ServiceName,"PRV Ack Time Is :%s:",ptr_st_orderbook->c_prev_ack_tm);
		fn_userlog(c_ServiceName,"Operation Type Is ;%c:",ptr_st_orderbook->c_oprn_typ);
	}
	switch( ptr_st_orderbook->c_oprn_typ )
	{
  	case UPDATE_STATUS_ACK_TM:

   		EXEC SQL
      	UPDATE  fod_fo_ordr_dtls
      	SET     fod_ordr_stts = :ptr_st_orderbook->c_ordr_stts,
        	      fod_ord_ack_tm = to_date (:ptr_st_orderbook->c_prev_ack_tm,'dd-mon-yyyy hh24:mi:ss' ),
          	    fod_lst_rqst_ack_tm = to_date (:ptr_st_orderbook->c_ack_tm,'dd-mon-yyyy hh24:mi:ss' ),
								fod_lst_act_ref = :c_lst_act_ref    /*** Ver 4.2 ***/
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
								fod_lst_act_ref 		= :c_lst_act_ref    /*** Ver 4.2 ***/
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
								fod_lst_act_ref 		= :c_lst_act_ref    /*** Ver 4.2 ***/
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
   	fn_errlog ( c_ServiceName, "S31610", SQLMSG, c_err_msg );
   	return -1;
  }

 	 return 0;

}

int fn_upd_xchngbkack( struct vw_xchngbook *ptr_st_xchngbook,
                       char *c_ServiceName,
                       char *c_err_msg)
{
  EXEC SQL BEGIN DECLARE SECTION;
    varchar c_xchng_rmrks [ 256 ];
  EXEC SQL END DECLARE SECTION;

  strcpy ( ( char * )c_xchng_rmrks.arr, ptr_st_xchngbook->c_xchng_rmrks );
  SETLEN ( c_xchng_rmrks );

	if(DEBUG_MSG_LVL_3)   /** Ver 2.8 **/
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
    fn_errlog ( c_ServiceName, "S31615", SQLMSG, c_err_msg );
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

/********** Ver 2.7 ********* Starts ********* Commented in Ver 3.2 ***

int fn_prcstrd( struct vw_orderbook *ptr_st_orderbook, 
										 char *c_ServiceName,
                      char *c_err_msg)
{

  char c_sltp_ord_rfrnc[19];
	struct vw_tradebook st_tradebook;
	int i_returncode = 0 ;
  short int si_null;
		
	MEMSET(c_sltp_ord_rfrnc);
	MEMSET(st_tradebook);

	if(DEBUG_MSG_LVL_3)
  {
		fn_userlog(c_ServiceName," Into function fn_prcstrd");
  	fn_userlog(c_ServiceName,"c_sltp_ord_rfrnc is |%s|",c_sltp_ord_rfrnc );
  	fn_userlog(c_ServiceName,"ptr_st_orderbook->c_ordr_rfrnc is |%s|",ptr_st_orderbook->c_ordr_rfrnc );
  	fn_userlog(c_ServiceName,"ptr_st_orderbook->c_prd_typ is |%s|", ptr_st_orderbook->c_prd_typ ); 
	}

  if ( ptr_st_orderbook->c_prd_typ != OPTIONPLUS ) 
  {
  	EXEC SQL 
    declare cur_trd2 CURSOR FOR
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
  OPEN  cur_trd2;
  }
  else                
  {

   EXEC SQL
  SELECT fod_sltp_ordr_rfrnc
  INTO   :c_sltp_ord_rfrnc
  FROM   FOD_FO_ORDR_DTLS
  WHERE  fod_ordr_rfrnc = :ptr_st_orderbook->c_ordr_rfrnc;


  if(SQLCODE !=0)
  {
    fn_errlog ( c_ServiceName, "S31620", SQLMSG, c_err_msg );
    return -1;
  }

    EXEC SQL 
    declare cur_trd3 CURSOR FOR
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
    WHERE   ftd_ordr_rfrnc IN(:ptr_st_orderbook->c_ordr_rfrnc,  :c_sltp_ord_rfrnc)
    AND     ftd_rms_prcsd_flg = 'N';

  	EXEC SQL
  	OPEN  cur_trd3;
 	}
 
  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31625", SQLMSG, c_err_msg );  
      if ( ptr_st_orderbook->c_prd_typ != OPTIONPLUS )
      {
       EXEC SQL
       CLOSE cur_trd2;
      }
      else
      {
       EXEC SQL
       CLOSE cur_trd3;
      }
		return -1 ;
  }

  while (1)
  {
    if ( ptr_st_orderbook->c_prd_typ != OPTIONPLUS ) 
    {
    EXEC SQL
      FETCH cur_trd2
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
      FETCH cur_trd3
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
      fn_errlog ( c_ServiceName, "S31630", SQLMSG, c_err_msg );
			if ( ptr_st_orderbook->c_prd_typ != OPTIONPLUS )
      {
       EXEC SQL
       CLOSE cur_trd2;
      }
      else
      {
       EXEC SQL
       CLOSE cur_trd3;
      }
      return -1;
    }
    if ( SQLCODE == NO_DATA_FOUND )
    {
			if ( ptr_st_orderbook->c_prd_typ != OPTIONPLUS )
      {
       EXEC SQL
       CLOSE cur_trd2;
      }
      else
      {
       EXEC SQL
       CLOSE cur_trd3;
      }
      break;
    }
    rtrim(st_tradebook.c_trd_dt);

    fn_cpy_ddr ( st_tradebook.c_rout_crt );

    i_returncode = fn_acall_svc( c_ServiceName,
                                c_err_msg,
                                &st_tradebook,
                                "vw_tradebook",
                                sizeof( struct vw_tradebook),
                                TPNOREPLY,
                                "SFO_OPT_TCONF" );

    if ( i_returncode != SUCC_BFR )
    {
      fn_errlog ( c_ServiceName, "S31635", LIBMSG, c_err_msg );
			if ( ptr_st_orderbook->c_prd_typ != OPTIONPLUS )
    	{
     	 EXEC SQL
     	 CLOSE cur_trd2;
			}
			else
			{
     	 EXEC SQL
     	 CLOSE cur_trd3;
			}
			return -1;
    }
  } 
	return 0 ;
}
 Ver 3.2 Ends ****** Ver 2.7 ****** Ends *****/
