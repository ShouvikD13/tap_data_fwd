/********************************************************************************/
/*  Program           : SFO_UPD_OPT_POS                                         */
/*                                                                              */
/*  Input             : vw_pstn_actn                                            */
/*                                                                              */
/*  Output            : vw_err_msg                                              */
/*                                                                              */
/*  Description       : This service updates the options positions based on     */
/*                      action.                                                 */
/*                                                                              */
/*  Log               : 1.0   02-Jan-2002  S. Swaminathan                       */
/*  Log               : 1.1   16-May-2003  S. Sinha|Infotech                    */
/*  Log               : 1.2   01-Nov-2006  Vidyayini Krish                      */
/*  Log               : 1.3   17-Oct-2006  Abhishek | Infotech                  */
/*  Log               : 1.4   18-Dec-2007  Vidyayini Krish                      */
/*	Log								:	1.5		14-Aug-2008	 Sandeep  | Infotech								  */
/*	Log								: 1.6		12-Jan-2009	 Sandeep	|	Infotech									*/
/*	Log								:	1.7		19-Jan-2009	 Sandeep	|	Infotech									*/
/*	Log								:	1.8		07-Sep-2009	 Shamili	|	Infotech									*/
/*	Log								:	1.9		26-Dec-2011	 Renuka	  |	Infotech									*/
/*  Log               : 2.0   19-Jan-2012  Sachin   | Infotech                  */
/*  Log               : 2.1   13-Aug-2012  Mahesh   | Infotech                  */
/*  Log               : 2.2   13-Aug-2012  Shamili  | Infotech                  */
/*  Log               : 2.3   07-Nov-2012  Swati A.B| Infotech                  */
/*  Log               : 2.4   13-Nov-2012  Swati A.B| Infotech                  */
/*  Log               : 2.5   20-May-2013  Samip M  | Infotech                  */
/*                    : 2.6		27-Jan-2013  Vishnu N | Infotech                  */
/*                    : 2.7   02-Apr-2014  Sachin B | Infotech                  */
/*                    : 2.8   08-Apr-2014  Vishnu N | Infotech                  */
/*                    : 2.9   26-May-2013  Shailesh S | Infotech                */
/*                    : 3.0   14-Mar-2013  Shashi K | Infotech                  */
/*                    : 3.1   14-Jul-2015  Navina D | Infotech                  */
/*                    : 3.4   19-Jul-2016  Sachin B | Infotech                  */
/*                    : 3.3   02-Feb-2016  Tanmay W  | Infotech                  */
/*                    : 3.5   29-Mar-2017  Bhupendra Malik                       */
/*                    : 3.6   21-Apr-2017  Ver 3.6 Bhupendra  Malik             */
/* Log                : 3.7   17-Jan-2019  Suchita D | Infotech                 */
/* Log                : 3.8   24-Jul-2019  Sachin B | Infotech                  */
/* Log                : 3.9   11-Dec-2020  Anuj Pancholi | Infotech             */
/* Log                : 4.0   21-Jan-2021  Mrinal Kishore | Infotech            */
/* Log                : 4.1   29-Jul-2021  Tanmay Patel | Infotech              */                  
/* Log                : 4.2   27-Aug-2021  Tanmay Patel | Infotech              */
/* Log                : 4.3   04-Sep-2021  Tanmay Patel | Infotech              */
/* Log                : 4.4   16-Dec-2021  Sachin Birje                         */
/* Log                : 4.5   07-Dec-2022  Karthik Sujit					            	*/
/********************************************************************************/
/*  1.0    -  New Release                                                       */
/*  1.1    -  Recomputation	of Buy and Sell Order margin for risk coverage		  */
/*  1.2	   -  Option Presquareoff Report																			  */	
/*  1.3    -  TDS Deduction for NRIs for premium on TRD_CONF, loss on ASGMNT    */
/*            and profit on EXER_CONF                                           */
/*  1.4    -  IBM Changes                                                       */
/*	1.5		 -	Introduction Of New Table FOR_OPT_IMTM_REP For Margin Detail Mail */
/*	1.6		 -	In Case Of Trigger price is not breached Executed Margin stored		*/
/*			   	  In the option MTM report table																		*/
/*	1.7		 -  Partial Execution & Cancellation Case Handeling										*/							  
/*	1.8		 -	Adding the Contract Details in Narration.													*/	
/*	1.9		 -	CRCSN43521  EQSAM-To capture pledge values                        */
/*  2.0    -  FNO BSE Changes                                                   */
/*  2.1    -  Handling of buissness error                                       */
/*  2.2    -  Margin percentage storage datatype is changed to double from long */
/*  2.3    -  CR-Overnight Orders Changes                                       */	
/*  2.4    -  Removal of Limit Alloc/Dealloc in Corp Action Changes							*/	
/*  2.5    -  Modify Allocation Changes                                         */
/*	2.6		 -  Changes for Auto MTM																							*/
/*  2.7    -  FDR Module Changes                                                */
/*  2.8    -  Service call to SVC_GET_QUOTE removed and function introduced     */
/*  2.9    -  Changes to avoid incorrect margin blocked after Adhoc expiry      */
/*  3.0   -   Stoped to write the file for system sqruare off records           */
/*  3.1   -   CR_ISEC14_66184: Margin calculation type casting fix for options  */
/*  3.4   -   Debug Level Changes                                               */
/*  3.3   -   Migration of RI to NRI 																						*/ 
/*  3.5   -   Core Fix                                                          */
/*  3.6   -   Cash Projection Changes                                           */
/*  3.7   -   Position Mismatch ( Executed qty from packet is used )            */ 
/*  3.8   -   Physical Delivery Margin Changes                                  */ 
/*  3.9   -   CR-ISEC14-141383 Separate ELM% for Deep OTM strikes               */
/*  4.0   -   Minor Change for Deep OTM option strikes (Mrinal Kishore)         */
/*  4.1   -   CR-ISEC10-156987 MarginShotfall_Communication (Tanmay Patel)      */
/*  4.2   -   Minor Change for MarginShotfall_Communication (Tanmay Patel)      */                                                     
/*  4.3   -   Minor Change for MarginShotfall_Communication (Tanmay Patel)      */
/*  4.4   -   CR-ISEC14-162094_NRI_TDS_UNSETTLED_PROF (Sachin Birje)            */
/*  4.5   -   CR-ISEC14-162075 SAM IN Currency & Commodity 									   	*/
/********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atmi.h>
#include <fml32.h>
#include <fo.h>
#include <eba_to_ors.h>
#include <fo_fml_def.h>
#include <fml_def.h>
/* COMMENTED IN VER TOL : TUX on LINUX (Ravindra) * #include <fo_view_def.h> */
#include <sqlca.h>
#include <fn_tuxlib.h>
#include <fn_pos.h>
#include <fn_ddr.h>
#include <math.h>
#include <fn_log.h> /* Ver 1.4 */
#include <fml_rout.h> /* Ver 1.4 */
#include <fn_read_debug_lvl.h> /* Ver 1.4 */
#include <fo_get_records.h> /** Ver 2.8 **/

#define   UPPER 1
#define   LOWER 2
#define   AUTO  3

/** Applies an action on an existing position for option type PUT **/
int	fn_apply_cnt_actn_put (	char *c_ServiceName,
													  struct vw_pstn_actn *ptr_st_pstn_actn,
												 	  struct vw_cntopt_pos *ptr_st_cntopt_pos,
                  				  struct vw_err_msg *ptr_st_err_msg );

/** Applies an action on an existing position for option type CALL **/
int	fn_apply_cnt_actn_call ( char *c_ServiceName,
													   struct vw_pstn_actn *ptr_st_pstn_actn,
												 	   struct vw_cntopt_pos *ptr_st_cntopt_pos,
                  				   struct vw_err_msg *ptr_st_err_msg );

/** Given a position and a type of action performed calculates the new **/
/** order  & Trade margin to be                                        **/
int fn_upd_mrgn_opt ( char *c_ServiceName,
                      struct vw_pstn_actn *ptr_st_pstn_actn,
                      struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                      struct vw_cntopt_pos *ptr_st_cntopt_pos,
                      struct vw_err_msg *ptr_st_err_msg );

int fn_call_actn_grp1_opt ( char *c_ServiceName,
                            struct vw_pstn_actn *ptr_st_pstn_actn,
                            struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                            struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                            double d_existing_dlvry_mrgn_val,  /*** ver 3.8 ***/
                            struct vw_err_msg *ptr_st_err_msg );

int fn_call_actn_add_mrgn( char *c_ServiceName,
                           struct vw_pstn_actn *ptr_st_pstn_actn,
                           struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                           struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                           struct vw_err_msg *ptr_st_err_msg,
													 char *c_imtm_addmrgn_flg);  /*** Ver 2.6 ***/

int fn_call_actn_imtm( char *c_ServiceName,
                       struct vw_pstn_actn *ptr_st_pstn_actn,
                       struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                       struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                       struct vw_err_msg *ptr_st_err_msg,
											 char *c_imtm_addmrgn_flg);  /*** Ver 2.6 ***/

int	fn_pos_validation (	char *c_ServiceName,
												struct vw_pstn_actn *ptr_st_pstn_actn,
											 	struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                 				struct vw_err_msg *ptr_st_err_msg);

int fn_call_actn_can_aft_asgn( 	char *c_ServiceName,
                       					struct vw_pstn_actn *ptr_st_pstn_actn,
                       					struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                       					struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                       					struct vw_err_msg *ptr_st_err_msg);


/* VER TOL : Tux on LINUX -- Function 'fn_nearest_paise' changed to 'fn_nearest_paise_uop' ( uos : upd_opt_pos ) (Ravindra) */
int fn_nearest_paise_uop(char *, double , int , int, char *, double * );

int fn_call_mrgn_cltn ( char *c_ServiceName,
                       	struct vw_pstn_actn *ptr_st_pstn_actn,
                       	struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                       	struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                       	struct vw_err_msg *ptr_st_err_msg);

int fn_upd_ins_trgprc(char *c_ServiceName,                  /** Ver 2.6  **/
                    struct vw_pstn_actn *ptr_st_pstn_actn,
                    struct vw_cntopt_pos st_cntopt_pos_to_be,         
                    struct vw_err_msg *ptr_st_err_msg);


int fn_get_mrgn_prntg_opt( char *c_ServiceName,
                           struct vw_pstn_actn  *ptr_st_pstn_actn,
                           struct vw_err_msg *ptr_st_err_msg,
                           double *d_initial_mrgn,
                           double *d_min_mrgn,
                           double *d_somc,
                           double *d_min_trg_price);           /*** Ver 2.6 ***/

/** Declaration for 1.2	**/

  double d_avl_mrgn_MTM ;
  double d_trd_mrgn_MTM_b ;
  double d_ord_mrgn_MTM_b ;
  double d_spread_mrgn_MTM_b ;
  double d_add_mrgn_MTM ;
	double d_trigger_priceMTM;
  double d_trd_mrgn_MTM_a ;
  double d_ord_mrgn_MTM_a ;
  double d_spread_mrgn_MTM_a ;
  long   l_opnpstn_MTM ;
  long   int li_cmp_MTM;
  double d_limit_OIMTM;

  char   c_expry_dt_MTM[22];
  char   c_bnk_accnt_MTM[13];
  char   c_time[21];
  char   c_filetime[9];
  char   c_filename[100];
  char   c_filedata[1000];
  char   c_otm_flg = 'N';  /** ver 3.9 **/
  double d_bft_amt_MTM ;
  double d_alctd_amt_MTM ;
	double d_plg_amt_MTM;
	double d_tot_shr_plgd_amt_MTM=0;  /***  Ver 1.9 ***/
	double d_eab_amt_MTM=0;           /***  Ver 1.9 ***/
	double d_nwb_amt_MTM=0;           /***  Ver 1.9 ***/
	double d_cab_amt_MTM=0;     /** ver 4.5 **/
	double d_cab_amt_mco_MTM=0; /** Ver 4.5 **/
  double d_fdr_amt_MTM=0;           /***  Ver 2.7 ***/
	
	/******* Commented in Ver 2.2 *********	

  long  l_init_mrgn_prcnt_MTM ;
  long  l_min_mrgn_prcnt_MTM ;
  long  l_somc_prcnt_MTM ;
  long  l_spread_mrgn_MTM ;
  long  l_avm_prcnt_MTM ;

	****************************************/


	/*********** Ver 2.2 starts *************/

	double d_init_mrgn_prcnt_MTM;
	double d_min_mrgn_prcnt_MTM ;
  double d_somc_prcnt_MTM ;
  double d_spread_mrgn_MTM ;
  double d_avm_prcnt_MTM ;

	/*********** Ver 2.2 ends *************/

  /*********** Ver 2.6 starts *************/
  double d_min_mrgn = 0.0;
  double d_somc  = 0.0;
  double d_min_trg_price= 0.0;
  double d_initial_mrgn = 0.0;

  /*********** Ver 2.6 ends *************/
	


  long  l_exctd_opn_pstn_MTM ;
  long  l_ord_opn_pstn_MTM ;
  FILE  *fp ;

/** Declaration for 1.2 **/

void SFO_UPD_OPT_POS( TPSVCINFO *rqst )
{
	char c_ServiceName[33];
  char c_err_msg[256];
  char c_cpos_avl;
  char c_can_aft_asgn_flg = 'N';
  char c_temp_str[500];  /*** Ver 3.0 ***/
  char c_imtm_addmrgn_flg = 'N'; /* Ver 2.6 */
  double  d_wag_price=0;    /* Ver 2.6 */
  int i_returncode;
  double d_existing_dlvry_mrgn_val = 0.0; /*** ver 3.8 ***/
 	char  c_usr_id[9];
  long    l_session_id;
  double  d_base_prc;
	char 		c_ltq_dt[23];
	char		c_xchng_cd[4];   /* VER TOL : TUX on LINUX -- Chnaged size from 3 to 4 (Ravindra) */
	char		c_routing_crt[4];
	double	d_value; 
	int			i_tick; 
	int			i_up_down;
	double	d_trg_prc;

  EXEC SQL BEGIN DECLARE SECTION;
    struct vw_pstn_actn *ptr_st_pstn_actn; 
    struct vw_cntopt_pos st_cntopt_pos_crrnt; 
    struct vw_cntopt_pos st_cntopt_pos_to_be;
    varchar c_trade_dt[12];
    char  c_mtm_flag;
  EXEC SQL END DECLARE SECTION;

  struct vw_err_msg *ptr_st_err_msg;

  ptr_st_pstn_actn = ( struct vw_pstn_actn *)rqst->data;
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

	rtrim(ptr_st_pstn_actn->c_cln_mtch_accnt);
	rtrim(ptr_st_pstn_actn->c_undrlyng);
	rtrim(ptr_st_pstn_actn->c_expry_dt);
	rtrim(ptr_st_pstn_actn->c_ref_rmrks);


  /*** Added for Order Routing ***/
  fn_init_ddr_val ( ptr_st_pstn_actn->c_rout_crt );

 /**** ver 3.4 Starts Here ****/
 if(DEBUG_MSG_LVL_1)
 {
  fn_userlog ( c_ServiceName,"c_cln_mtch_accnt:%s: c_xchng_cd:%s: c_prd_typ:%c: c_undrlyng:%s:",ptr_st_pstn_actn->c_cln_mtch_accnt,ptr_st_pstn_actn->c_xchng_cd,ptr_st_pstn_actn->c_prd_typ,ptr_st_pstn_actn->c_undrlyng);
  fn_userlog ( c_ServiceName,"c_expry_dt:%s: l_strike_prc:%ld: c_opt_typ:%c: l_actn_typ:%ld:",ptr_st_pstn_actn->c_expry_dt,ptr_st_pstn_actn->l_strike_prc,ptr_st_pstn_actn->c_opt_typ,ptr_st_pstn_actn->l_actn_typ);
  fn_userlog ( c_ServiceName,"c_trnsctn_flw:%c: l_new_rt:%ld: l_exec_qty:%ld: l_exec_rt:%ld:",ptr_st_pstn_actn->c_trnsctn_flw,ptr_st_pstn_actn->l_new_rt,ptr_st_pstn_actn->l_exec_qty,ptr_st_pstn_actn->l_exec_rt); 
 
 }
 /**** Ver 3.4 Ends here *****/
 
if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "SFO_UPD_OPT_POS - I" );
  fn_userlog ( c_ServiceName,"c_user_id        :%s:",
                              ptr_st_pstn_actn->c_user_id );
  fn_userlog ( c_ServiceName,"l_session_id      :%ld:",
                              ptr_st_pstn_actn->l_session_id );
  fn_userlog ( c_ServiceName,"c_cln_mtch_accnt :%s:",
                              ptr_st_pstn_actn->c_cln_mtch_accnt );
  fn_userlog ( c_ServiceName,"c_xchng_cd       :%s:",
                              ptr_st_pstn_actn->c_xchng_cd );
  fn_userlog ( c_ServiceName,"c_prd_typ        :%c:", 
                              ptr_st_pstn_actn->c_prd_typ );
  fn_userlog ( c_ServiceName,"c_undrlyng       :%s:", 
                              ptr_st_pstn_actn->c_undrlyng );
  fn_userlog ( c_ServiceName,"c_expry_dt       :%s:",
                              ptr_st_pstn_actn->c_expry_dt );
  fn_userlog ( c_ServiceName,"l_strike_prc       :%ld:",
                              ptr_st_pstn_actn->l_strike_prc );
  fn_userlog ( c_ServiceName,"c_exrc_typ       :%c:",
                              ptr_st_pstn_actn->c_exrc_typ );
  fn_userlog ( c_ServiceName,"c_opt_typ       :%c:",
                              ptr_st_pstn_actn->c_opt_typ );
  fn_userlog ( c_ServiceName,"l_actn_typ       :%ld:",
                              ptr_st_pstn_actn->l_actn_typ );
  fn_userlog ( c_ServiceName,"c_trnsctn_flw    :%c:",
                              ptr_st_pstn_actn->c_trnsctn_flw );
  fn_userlog ( c_ServiceName,"l_orgnl_qty      :%ld:",
                              ptr_st_pstn_actn->l_orgnl_qty );
  fn_userlog ( c_ServiceName,"l_orgnl_rt       :%ld:",
                              ptr_st_pstn_actn->l_orgnl_rt );
  fn_userlog ( c_ServiceName,"l_new_qty        :%ld:",
                              ptr_st_pstn_actn->l_new_qty );
  fn_userlog ( c_ServiceName,"l_new_rt         :%ld:",
                              ptr_st_pstn_actn->l_new_rt );
  fn_userlog ( c_ServiceName,"l_exec_qty       :%ld:",
                              ptr_st_pstn_actn->l_exec_qty );
  fn_userlog ( c_ServiceName,"l_exec_rt        :%ld:",
                              ptr_st_pstn_actn->l_exec_rt );
  fn_userlog ( c_ServiceName,"c_ref_rmrks      :%s:",
                              ptr_st_pstn_actn->c_ref_rmrks );
  fn_userlog ( c_ServiceName,"c_rout_crt       :%s:",
                              ptr_st_pstn_actn->c_rout_crt );
}

	/******** Get Contract level position for the Exchange, client, ********/
  /******** underlying, expiry date combination into CPOS_crrnt   ********/
	c_cpos_avl = 'Y';

  EXEC SQL
    SELECT  fop_clm_mtch_accnt,
						fop_xchng_cd,
						fop_prdct_typ,
						fop_indstk,
						fop_undrlyng,
						fop_expry_dt,
						fop_strk_prc,
						fop_exer_typ,
						fop_opt_typ,
						NVL( fop_ibuy_qty, 0 ),
						NVL( fop_ibuy_ord_val, 0 ),
						NVL( fop_isell_qty, 0 ),
						NVL( fop_isell_ord_val, 0 ),
						NVL( fop_exbuy_qty, 0 ),
						NVL( fop_exbuy_ord_val, 0 ),
						NVL( fop_exsell_qty, 0 ),
						NVL( fop_exsell_ord_val, 0 ),
						NVL( fop_buy_exctd_qty, 0 ),
						NVL( fop_sell_exctd_qty, 0 ),
						fop_opnpstn_flw,
						NVL( fop_opnpstn_qty, 0 ),
						NVL( fop_opnpstn_val, 0 ),
						NVL( fop_uordr_mrgn, 0 ),
						NVL( fop_uexctd_mrgn, 0 ),
						NVL( fop_iexrc_qty, 0 ),
						NVL( fop_exexrc_qty, 0 ),
						NVL( fop_exrc_qty, 0 ),
						NVL( fop_dp_qty, 0 ),
						NVL( fop_asgnd_qty, 0 ),
						NVL( fop_ord_blkd_qty, 0 ),
						NVL( fop_trd_blkd_qty, 0 ),
						NVL(fop_mtm_flg,'O'),
						NVL(fop_trggr_prc, 0 ),
						NVL(fop_idontexrc_qty, 0),
						NVL(fop_exdontexrc_qty, 0),
            NVL(fop_udlvry_mrgn,0.0)  /*** Ver 3.8 ***/
		INTO		:st_cntopt_pos_crrnt.c_cln_mtch_accnt,
            :st_cntopt_pos_crrnt.c_xchng_cd,
            :st_cntopt_pos_crrnt.c_prd_typ,
            :st_cntopt_pos_crrnt.c_ctgry_indstk,
            :st_cntopt_pos_crrnt.c_undrlyng,
            :st_cntopt_pos_crrnt.c_expry_dt,
            :st_cntopt_pos_crrnt.l_strike_prc,
            :st_cntopt_pos_crrnt.c_exrc_typ,
            :st_cntopt_pos_crrnt.c_opt_typ,
            :st_cntopt_pos_crrnt.l_ibuy_qty,
            :st_cntopt_pos_crrnt.d_ibuy_ord_vl,
            :st_cntopt_pos_crrnt.l_isell_qty,
            :st_cntopt_pos_crrnt.d_isell_ord_vl,
            :st_cntopt_pos_crrnt.l_exbuy_qty,
            :st_cntopt_pos_crrnt.d_exbuy_ord_vl,
            :st_cntopt_pos_crrnt.l_exsell_qty,
            :st_cntopt_pos_crrnt.d_exsell_ord_vl,
            :st_cntopt_pos_crrnt.l_buy_exctd_qty,
            :st_cntopt_pos_crrnt.l_sell_exctd_qty,
            :st_cntopt_pos_crrnt.c_opnpstn_flw,
            :st_cntopt_pos_crrnt.l_opnpstn_qty,
            :st_cntopt_pos_crrnt.d_org_opn_val,
            :st_cntopt_pos_crrnt.d_ordr_mrgn,
            :st_cntopt_pos_crrnt.d_trd_mrgn,
            :st_cntopt_pos_crrnt.l_iexrc_qty,
            :st_cntopt_pos_crrnt.l_exexrc_qty,
            :st_cntopt_pos_crrnt.l_exrc_qty,
            :st_cntopt_pos_crrnt.l_ord_dp_qty,
            :st_cntopt_pos_crrnt.l_asgnd_qty,
						:st_cntopt_pos_crrnt.l_blkd_ordr_qty,
						:st_cntopt_pos_crrnt.l_blkd_trd_qty,
						:c_mtm_flag,
						:st_cntopt_pos_crrnt.l_mtm_trg_prc,
						:st_cntopt_pos_crrnt.l_idont_exrc_qty,
						:st_cntopt_pos_crrnt.l_exdont_exrc_qty,
            :d_existing_dlvry_mrgn_val              /*** ver 3.8 ****/
		FROM		fop_fo_options_pstn
    WHERE fop_clm_mtch_accnt  = :ptr_st_pstn_actn->c_cln_mtch_accnt
    AND   fop_xchng_cd        = :ptr_st_pstn_actn->c_xchng_cd
    AND   fop_prdct_typ       = :ptr_st_pstn_actn->c_prd_typ
    AND   fop_opt_typ       	= :ptr_st_pstn_actn->c_opt_typ
    AND   fop_exer_typ       	= :ptr_st_pstn_actn->c_exrc_typ
    AND   fop_undrlyng        = :ptr_st_pstn_actn->c_undrlyng
    AND   fop_expry_dt        = :ptr_st_pstn_actn->c_expry_dt
    AND   fop_strk_prc      	= :ptr_st_pstn_actn->l_strike_prc;

  if ( ( SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
  {
    fn_errlog ( c_ServiceName, "S31010", SQLMSG, ptr_st_err_msg->c_err_msg );
    tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
  }

  /**** If a position doesnot exists, initialise the position values to ****/
	/**** default                                                         ****/
  fn_userlog( c_ServiceName," suchita c_mtm_flag is :%c",c_mtm_flag); 

  if ( SQLCODE == NO_DATA_FOUND )
  {
    strcpy( st_cntopt_pos_crrnt.c_cln_mtch_accnt,
																	ptr_st_pstn_actn->c_cln_mtch_accnt );
    strcpy( st_cntopt_pos_crrnt.c_xchng_cd, ptr_st_pstn_actn->c_xchng_cd );
    strcpy( st_cntopt_pos_crrnt.c_undrlyng, ptr_st_pstn_actn->c_undrlyng );
    strcpy( st_cntopt_pos_crrnt.c_expry_dt, ptr_st_pstn_actn->c_expry_dt );
    st_cntopt_pos_crrnt.c_prd_typ  = ptr_st_pstn_actn->c_prd_typ;
    st_cntopt_pos_crrnt.c_opt_typ  = ptr_st_pstn_actn->c_opt_typ;
    st_cntopt_pos_crrnt.c_ctgry_indstk  = ptr_st_pstn_actn->c_ctgry_indstk;
    st_cntopt_pos_crrnt.c_exrc_typ = ptr_st_pstn_actn->c_exrc_typ;
    st_cntopt_pos_crrnt.l_strike_prc = ptr_st_pstn_actn->l_strike_prc;
    st_cntopt_pos_crrnt.l_ibuy_qty = 0;
    st_cntopt_pos_crrnt.d_ibuy_ord_vl = 0;
    st_cntopt_pos_crrnt.l_isell_qty = 0;
    st_cntopt_pos_crrnt.d_isell_ord_vl = 0;
    st_cntopt_pos_crrnt.l_exbuy_qty = 0;
    st_cntopt_pos_crrnt.d_exbuy_ord_vl = 0;
    st_cntopt_pos_crrnt.l_exsell_qty = 0;
    st_cntopt_pos_crrnt.d_exsell_ord_vl = 0;
    st_cntopt_pos_crrnt.l_buy_exctd_qty = 0;
    st_cntopt_pos_crrnt.l_sell_exctd_qty = 0;
    st_cntopt_pos_crrnt.c_opnpstn_flw = 'N';
    st_cntopt_pos_crrnt.l_opnpstn_qty = 0;
    st_cntopt_pos_crrnt.d_org_opn_val = 0;
    st_cntopt_pos_crrnt.d_ordr_mrgn = 0;
    st_cntopt_pos_crrnt.d_trd_mrgn = 0;
    st_cntopt_pos_crrnt.l_exrc_qty = 0;
    st_cntopt_pos_crrnt.l_iexrc_qty = 0;
    st_cntopt_pos_crrnt.l_exexrc_qty = 0;
    st_cntopt_pos_crrnt.l_ord_dp_qty = 0;
    st_cntopt_pos_crrnt.l_asgnd_qty = 0;
	  st_cntopt_pos_crrnt.l_blkd_ordr_qty = 0;
		st_cntopt_pos_crrnt.l_blkd_trd_qty = 0;
		st_cntopt_pos_crrnt.l_mtm_trg_prc = 0;
		st_cntopt_pos_crrnt.l_idont_exrc_qty = 0;
		st_cntopt_pos_crrnt.l_exdont_exrc_qty = 0;

		c_cpos_avl='N';
    c_mtm_flag = 'O';

	}

if(DEBUG_MSG_LVL_3){
/****  Add description for all userlog ****/
	fn_userlog( c_ServiceName, "Current position"  );
	fn_userlog( c_ServiceName, "Match account     :%s:", 
																st_cntopt_pos_crrnt.c_cln_mtch_accnt);
  fn_userlog( c_ServiceName, "Exchange code     :%s:", 
																st_cntopt_pos_crrnt.c_xchng_cd );
  fn_userlog( c_ServiceName, "Product type      :%c:", 
																st_cntopt_pos_crrnt.c_prd_typ );
  fn_userlog( c_ServiceName, "Option Type       :%c:", 
																st_cntopt_pos_crrnt.c_opt_typ );
  fn_userlog( c_ServiceName, "Index / Stock     :%c:", 
																st_cntopt_pos_crrnt.c_ctgry_indstk );
  fn_userlog( c_ServiceName, "Underlying        :%s:", 
																st_cntopt_pos_crrnt.c_undrlyng );
  fn_userlog( c_ServiceName, "Expiry date       :%s:", 
																st_cntopt_pos_crrnt.c_expry_dt );
  fn_userlog( c_ServiceName, "Strike Price      :%ld:", 
																st_cntopt_pos_crrnt.l_strike_prc );
  fn_userlog( c_ServiceName, "Exercise Type     :%c:", 
																st_cntopt_pos_crrnt.c_exrc_typ );
  fn_userlog( c_ServiceName, "Ibuy qty          :%ld:", 
																st_cntopt_pos_crrnt.l_ibuy_qty );
  fn_userlog( c_ServiceName, "Ibuy order val    :%lf:", 
																st_cntopt_pos_crrnt.d_ibuy_ord_vl );
  fn_userlog( c_ServiceName, "Isell qty         :%ld:", 
																st_cntopt_pos_crrnt.l_isell_qty );
  fn_userlog( c_ServiceName, "Isell order val   :%lf:", 
																st_cntopt_pos_crrnt.d_isell_ord_vl );
  fn_userlog( c_ServiceName, "Exbuy qty         :%ld:", 
																st_cntopt_pos_crrnt.l_exbuy_qty );
  fn_userlog( c_ServiceName, "Ex buy order val  :%lf:", 
																st_cntopt_pos_crrnt.d_exbuy_ord_vl );
  fn_userlog( c_ServiceName, "Ex Sell qty       :%ld:",
																st_cntopt_pos_crrnt.l_exsell_qty );
  fn_userlog( c_ServiceName, "Ex sell order val :%lf:",
																st_cntopt_pos_crrnt.d_exsell_ord_vl );
  fn_userlog( c_ServiceName, "Buy Executed qty  :%ld:",
																st_cntopt_pos_crrnt.l_buy_exctd_qty );
  fn_userlog( c_ServiceName, "Sell Executed qty :%ld:",
																st_cntopt_pos_crrnt.l_sell_exctd_qty );
  fn_userlog( c_ServiceName, "Open Position Flow:%c:", 
																st_cntopt_pos_crrnt.c_opnpstn_flw );
  fn_userlog( c_ServiceName, "Open position qty :%ld:", 
																st_cntopt_pos_crrnt.l_opnpstn_qty );
  fn_userlog( c_ServiceName, "Open value        :%lf:", 
																st_cntopt_pos_crrnt.d_org_opn_val );
  fn_userlog( c_ServiceName, "Order Margin      :%lf:", 
																st_cntopt_pos_crrnt.d_ordr_mrgn );
  fn_userlog( c_ServiceName, "Trade Margin      :%lf:", 
																st_cntopt_pos_crrnt.d_trd_mrgn );
  fn_userlog( c_ServiceName, "Iexercise qty     :%ld:", 
																st_cntopt_pos_crrnt.l_iexrc_qty );
  fn_userlog( c_ServiceName, "Ex-exercise qty   :%ld:", 
																st_cntopt_pos_crrnt.l_exexrc_qty );
  fn_userlog( c_ServiceName, "Exercised qty     :%ld:", 
																st_cntopt_pos_crrnt.l_exrc_qty );
  fn_userlog( c_ServiceName, "DP Qty            :%ld:", 
																st_cntopt_pos_crrnt.l_ord_dp_qty );
  fn_userlog( c_ServiceName, "Assigned qty      :%ld:", 
																st_cntopt_pos_crrnt.l_asgnd_qty );
  fn_userlog( c_ServiceName, "IWTL don't exrc qty      :%ld:", 
																st_cntopt_pos_crrnt.l_idont_exrc_qty );
  fn_userlog( c_ServiceName, "Ex -don't exrc qty      :%ld:", 
																st_cntopt_pos_crrnt.l_exdont_exrc_qty );
}

	/** Initialise the to be position variable **/
		memcpy ( &st_cntopt_pos_to_be, &st_cntopt_pos_crrnt, 
                               sizeof( st_cntopt_pos_to_be ));

  if ( (( c_mtm_flag == UNDER_MTM ) || ( c_mtm_flag == UNDER_PHYDLVRY_MTM)) &&
       ( strcmp ( ptr_st_pstn_actn->c_user_id,"system" ) != 0 ) &&
       ( ptr_st_pstn_actn->l_session_id != 0 )                   )
  {
    fn_errlog(c_ServiceName, "B28009", DEFMSG, ptr_st_err_msg->c_err_msg );
    tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
  }

/***********************************************************
	strcpy( c_usr_id, "system" );
  l_session_id=0;
***********************************************************/
	strcpy( c_usr_id, ptr_st_pstn_actn->c_user_id );
  l_session_id = ptr_st_pstn_actn->l_session_id;


  if ( strcmp( ptr_st_pstn_actn->c_xchng_cd, "NFO" ) == 0 )
  {
    strcpy( c_xchng_cd, "NSE" );
  }
  else if( strcmp( ptr_st_pstn_actn->c_xchng_cd, "BFO" ) == 0 )  /*** Ver 2.0 ***/
  {
    strcpy( c_xchng_cd, "BSE" );
  }

	/** Commneted in ver 2.8 **

  *** Added for Order Routing ***
	fn_cpy_ddr ( c_routing_crt );

  i_returncode = fn_call_svc_fml( c_ServiceName,
                                  c_err_msg,
                                  "SVC_GET_QUOTE",
                                  0,
                                  5,
                                  3,
                    FML_USR_ID, (char *)c_usr_id,
                    FML_SSSN_ID, (char *)&l_session_id,
                    FML_STCK_CD, (char *)ptr_st_pstn_actn->c_undrlyng,
                    FML_XCHNG_CD, (char *)c_xchng_cd,
                    FFO_ROUT_CRT, (char *)c_routing_crt,
                    FML_QUOTE, ( char *)&ptr_st_pstn_actn->d_und_quote, NULL,
                    FML_GMS_AMNT_ALLCTD, (char *)&d_base_prc, NULL,
                    FML_QUOTE_TIME, (char *)c_ltq_dt, NULL );
  if ( i_returncode != SUCC_BFR )
  {
    fn_errlog(c_ServiceName, "S31015", SQLMSG, ptr_st_err_msg->c_err_msg );
    tpreturn( TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
  }
	**/

	
	/** ver 2.8 Starts **/

  i_returncode = fn_get_spotprc(c_ServiceName,
                                ptr_st_pstn_actn->c_undrlyng,
                                c_xchng_cd,
                                &ptr_st_pstn_actn->d_und_quote,
                                c_ltq_dt,
                                c_err_msg
                                );

  if ( i_returncode != 0 )
   {
    fn_errlog(c_ServiceName, "S31020", SQLMSG, ptr_st_err_msg->c_err_msg );
    tpreturn( TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
  }

  /** Ver  2.8 **/

	li_cmp_MTM = ptr_st_pstn_actn->d_und_quote;

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog( c_ServiceName, "Quote:%lf:", ptr_st_pstn_actn->d_und_quote );
 /*	fn_userlog( c_ServiceName, "BasePrice:%lf:", d_base_prc ); 	Commented in ver 2.8 */
  	fn_userlog( c_ServiceName, "Blked Flag :%c:", ptr_st_pstn_actn->c_nkd_blkd_flg);  /*** Ver 2.4 ***/
	}

  if ( ( ptr_st_pstn_actn->l_actn_typ != INTRADAY_MTM ) 			 &&
			 ( ptr_st_pstn_actn->l_actn_typ != MRGN_STMNT )	&&					/*** Ver 1.5 ***/
       ( ptr_st_pstn_actn->l_actn_typ != ADD_MARGIN     )      &&
       ( ptr_st_pstn_actn->l_actn_typ != CANCEL_AFTER_ASSIGNMENT  ) &&
       ( ptr_st_pstn_actn->l_actn_typ != EOD_MRGN_REP ) ) /** Ver 4.1 **/ 
  {
		i_returncode = fn_call_actn_grp1_opt ( c_ServiceName,
                                           ptr_st_pstn_actn,
                                           &st_cntopt_pos_crrnt,
                                           &st_cntopt_pos_to_be,
                                           d_existing_dlvry_mrgn_val, /*** ver 3.8 ***/
                                           ptr_st_err_msg );
		if ( i_returncode != 0 )
		{
      /*  2.5 Starts */
      if(i_returncode == INSUFFICIENT_LIMITS)
      {
        tpreturn( TPFAIL, INSUFFICIENT_LIMITS, (char *)ptr_st_err_msg, 0, 0 );
      }
      else
      {
      /*  2.5 Ends  */
			tpreturn( TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0 );
      }/* 2.5 */
		}
  }

  if ( ptr_st_pstn_actn->l_actn_typ == ADD_MARGIN )
  {
		/*** if position is SELL then only call add margin ***/
		if ( st_cntopt_pos_crrnt.l_opnpstn_qty < 0 )
		{
			c_imtm_addmrgn_flg='N';  /** Ver 2.6 **/
    	i_returncode  = fn_call_actn_add_mrgn ( c_ServiceName,
                                            	ptr_st_pstn_actn,
                                            	&st_cntopt_pos_crrnt,
                                            	&st_cntopt_pos_to_be,
                                            	ptr_st_err_msg,
																							&c_imtm_addmrgn_flg );  /*** Ver 2.6 **/

    	if ( i_returncode != 0 )
    	{
      	tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
    	}
		}
		else
		{
    	fn_errlog(c_ServiceName, "B28559", DEFMSG, ptr_st_err_msg->c_err_msg );
      tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
		}
  }

	if ( ptr_st_pstn_actn->l_actn_typ == INTRADAY_MTM  || ptr_st_pstn_actn->l_actn_typ == EOD_MRGN_REP )   /** EOD_MRGN_REP Added in Ver 4.1 **/ 
  {

		/** 1.2 starts **/

		ptr_st_pstn_actn->d_add_mrgn_amt = 0;
		strcpy(c_expry_dt_MTM,(char *)ptr_st_pstn_actn->c_expry_dt);	

		/** 1.2 ends **/

		c_imtm_addmrgn_flg ='N';  /*** Ver 2.6 ***/

    i_returncode  = fn_call_actn_imtm ( c_ServiceName,
                                        ptr_st_pstn_actn,
                                        &st_cntopt_pos_crrnt,
                                        &st_cntopt_pos_to_be,
                                        ptr_st_err_msg,
																				&c_imtm_addmrgn_flg ); /** Ver 2.6 **/ 

    if ( i_returncode != 0 )
    {
			if(DEBUG_MSG_LVL_3){
  			fn_userlog ( c_ServiceName,"Return code from Intra day MTM :%d:",i_returncode); 
			}

      switch ( i_returncode )
      {
        case INSUFFICIENT_LIMITS :

				/** Ver 1.2 start **/

				if(DEBUG_MSG_LVL_3){
  				fn_userlog(c_ServiceName,"STAGE I for INSUFF LMT -:%s:",ptr_st_pstn_actn->c_cln_mtch_accnt);
  				fn_userlog(c_ServiceName,"Limit :%lf: for :%s:",d_limit_OIMTM,ptr_st_pstn_actn->c_cln_mtch_accnt);
				}

				l_exctd_opn_pstn_MTM = st_cntopt_pos_to_be.l_opnpstn_qty ;
        
        /**** ver 3.9 starts ****/

            EXEC SQL
              SELECT  NVL(FTQ_OTM_FLG,'N')
              INTO    :c_otm_flg
              FROM   FTQ_FO_TRD_QT
              WHERE  FTQ_XCHNG_CD = :ptr_st_pstn_actn->c_xchng_cd
              AND    FTQ_UNDRLYNG = :ptr_st_pstn_actn->c_undrlyng
              AND    FTQ_PRDCT_TYP = :ptr_st_pstn_actn->c_prd_typ 
              AND    FTQ_EXPRY_DT = to_date(:ptr_st_pstn_actn->c_expry_dt,'dd-Mon-yyyy')
              AND    FTQ_STRK_PRC = :ptr_st_pstn_actn->l_strike_prc
              AND    FTQ_EXER_TYP = :ptr_st_pstn_actn->c_exrc_typ
              AND    FTQ_OPT_TYP =  :ptr_st_pstn_actn->c_opt_typ; 
         
        if ( SQLCODE != 0 && SQLCODE !=  NO_DATA_FOUND )		/* Ver 4.0 No Data Found handled */
        {
         fn_userlog(c_ServiceName,"COULDN'T GET  OTM/OTH FLAG ");
         fn_userlog(c_ServiceName,"SQLCODE IS |%ld|",SQLCODE);
        }
       
        /**** ver 3.9 ends ****/
				/** ver 4.0 starts **/
    		if ( SQLCODE == NO_DATA_FOUND )
    		{
      		c_otm_flg = 'N';
      		fn_userlog(c_ServiceName,"OTM/OTH Flag Set To Default for Contract :%s:%s:%c:%s:%ld:%c:%c:",ptr_st_pstn_actn->c_xchng_cd,ptr_st_pstn_actn->c_undrlyng,ptr_st_pstn_actn->c_prd_typ,ptr_st_pstn_actn->c_expry_dt,ptr_st_pstn_actn->l_strike_prc,ptr_st_pstn_actn->c_opt_typ,ptr_st_pstn_actn->c_exrc_typ);
    		}
    		/** ver 4.0 Ends **/

				EXEC SQL
			  		 SELECT /** nvl(fmm_init_mrgn_prcnt,0),  ** commented in ver 3.9 **/
                    decode(:c_otm_flg,'Y',nvl(fmm_deep_otm_im_prcnt,0),nvl(fmm_init_mrgn_prcnt,0)),  /** ver 3.9 **/
        						/** nvl(fmm_min_mrgn_prcnt,0),  ** commented in ver 3.9 **/
                    decode(:c_otm_flg,'Y',nvl(fmm_deep_otm_mm_prcnt,0),nvl(fmm_min_mrgn_prcnt,0)),  /** ver 3.9 **/
        						nvl(fmm_somc_prcnt,0),
        						nvl(fmm_spread_mrgn,0),
        						nvl(fmm_avm_prcnt,0)
						 INTO   :d_init_mrgn_prcnt_MTM, /* 2.2 datatype of margin var change to double */
        						:d_min_mrgn_prcnt_MTM,
        						:d_somc_prcnt_MTM,
        						:d_spread_mrgn_MTM,
        						:d_avm_prcnt_MTM
						 FROM   fmm_fo_mrgn_mstr
						 WHERE  fmm_prdct_typ = 'O'
						 AND    fmm_undrlyng = :ptr_st_pstn_actn->c_undrlyng
             AND    fmm_xchng_cd = :ptr_st_pstn_actn->c_xchng_cd        /*** Ver 2.0 ***/
						 AND    fmm_undrlyng_bskt_id =(SELECT  fcb_undrlyng_bskt_id
    																			 FROM    fcb_fo_cln_bskt_alltd
    																			 WHERE   fcb_cln_lvl =(SELECT clm_clnt_lvl
                          																			 FROM clm_clnt_mstr
                          																			 WHERE clm_mtch_accnt =:ptr_st_pstn_actn->c_cln_mtch_accnt));

				if ( SQLCODE != 0 )
				{
  				fn_userlog(c_ServiceName,"COULDN'T INSERT DATA INTO IMTM REPORT TABLE I");
  				fn_userlog(c_ServiceName,"SQLCODE IS |%ld|",SQLCODE);
				}

				d_nwb_amt_MTM = 0; /* Ver 1.9 */
        d_fdr_amt_MTM = 0; /* Ver 2.7 */
 
				EXEC SQL
						 SELECT fab_bnk_accnt,
        						fab_alctd_amt,
        						fab_bft_amt,
										fab_plg_amt,
									  nvl(FAB_NWB_AMT,0),     /* Ver 1.9 */
                    nvl(fab_fdr_amt,0)     /* Ver 2.7 */	
						 INTO   :c_bnk_accnt_MTM,
        						:d_alctd_amt_MTM,
        					  :d_bft_amt_MTM,
										:d_plg_amt_MTM,
										:d_nwb_amt_MTM,   			/* Ver 1.9 */
                    :d_fdr_amt_MTM          /* Ver 2.7 */
						 FROM   fab_fo_alc_bft_smry
						 WHERE  fab_clm_mtch_accnt = :ptr_st_pstn_actn->c_cln_mtch_accnt ;

				if ( SQLCODE != 0 )
				{
  					fn_userlog(c_ServiceName,"SQLCODE IS |%ld|",SQLCODE);
				}

				/******** Ver 1.9.********/
        d_eab_amt_MTM           = 0;
        d_tot_shr_plgd_amt_MTM  = 0;
        EXEC SQL
        SELECT  nvl(EAB_PLG_AMT,0) * 100 /* To be inserted in paise */
        INTO    :d_eab_amt_MTM
        FROM    EAB_EQ_ALC_BFT_SMRY
        WHERE   EAB_CLM_MTCH_ACCNT = :ptr_st_pstn_actn->c_cln_mtch_accnt ;

        if ( SQLCODE != 0 )
        {
        	fn_userlog(c_ServiceName,"SQLCODE IS |%ld|",SQLCODE);
        }

		/** Ver 4.5 starts **/
		d_cab_amt_MTM=0;

		EXEC SQL
		SELECT nvl(CAB_PLG_AMT,0) * 100
		INTO :d_cab_amt_MTM
		FROM CAB_CDX_ALC_BFT_SMRY
		WHERE CAB_CLM_MTCH_ACCNT = :ptr_st_pstn_actn->c_cln_mtch_accnt;

    if ( SQLCODE != 0 && ( SQLCODE != NO_DATA_FOUND ) )
    {
      fn_userlog(c_ServiceName,"SQLCODE IS |%ld|",SQLCODE);
    }

		d_cab_amt_mco_MTM=0;

    EXEC SQL
    SELECT nvl(CAB_PLG_AMT,0) 
    INTO   :d_cab_amt_mco_MTM
    FROM   CAB_COD_ALC_BFT_SMRY
    WHERE  CAB_CLM_MTCH_ACCNT = :ptr_st_pstn_actn->c_cln_mtch_accnt;

    if ( SQLCODE != 0 && ( SQLCODE != NO_DATA_FOUND ) )
    {
      fn_userlog(c_ServiceName,"SQLCODE IS |%ld|",SQLCODE);
    }

    /** d_tot_shr_plgd_amt_MTM  = d_nwb_amt_MTM + d_plg_amt_MTM + d_eab_amt_MTM; commented in ver 4.5 **/
		d_tot_shr_plgd_amt_MTM  = d_nwb_amt_MTM + d_plg_amt_MTM + d_eab_amt_MTM + d_cab_amt_MTM + d_cab_amt_mco_MTM;

		/** Ver 4.5 ends **/

        /***** End of Ver 1.9 ****/

				EXEC SQL
						 SELECT to_char(sysdate,'dd-Mon-yyyy hh24:mi:ss'),
										to_char(sysdate,'yyyymmdd')
						 INTO   :c_time,
        						:c_filetime
						 FROM    dual ;

				if ( SQLCODE != 0 )
				{
  				fn_userlog(c_ServiceName,"COULDN'T INSERT DATA INTO IMTM REPORT TABLE II");
  				fn_userlog(c_ServiceName,"SQLCODE IS |%ld|",SQLCODE);
				}

				if(DEBUG_MSG_LVL_3){
					fn_userlog(c_ServiceName,"TIME:%s:",c_time);
					fn_userlog(c_ServiceName,"MATCH:%s:",ptr_st_pstn_actn->c_cln_mtch_accnt);
					fn_userlog(c_ServiceName,"EXG_CD:%s:",ptr_st_pstn_actn->c_xchng_cd);
					fn_userlog(c_ServiceName,"UNDERLYNG:%s:",ptr_st_pstn_actn->c_undrlyng);
					fn_userlog(c_ServiceName,"PRD_TYP:%c:",ptr_st_pstn_actn->c_prd_typ);
					fn_userlog(c_ServiceName,"STRIKE_PRICE:%ld:",ptr_st_pstn_actn->l_strike_prc);
					fn_userlog(c_ServiceName,"TRIGGER_PRICE:%ld:",d_trigger_priceMTM);
					fn_userlog(c_ServiceName,"EXPRY_DT:%s:",c_expry_dt_MTM);
					fn_userlog(c_ServiceName,"EXER_TYP:%c:",ptr_st_pstn_actn->c_exrc_typ);
					fn_userlog(c_ServiceName,"OPT_TYP:%c:",ptr_st_pstn_actn->c_opt_typ);
					fn_userlog(c_ServiceName,"CONTRACT_TAG:%c:",ptr_st_pstn_actn->c_cntrct_tag);
					fn_userlog(c_ServiceName,"BANK_ACC:%s:",c_bnk_accnt_MTM);
					fn_userlog(c_ServiceName,"LIMIT:%lf:",d_limit_OIMTM);
					fn_userlog(c_ServiceName,"ALLC_AMT:%lf:",d_alctd_amt_MTM);
			  	fn_userlog(c_ServiceName,"BFT_AMT:%lf:",d_bft_amt_MTM);
					fn_userlog(c_ServiceName,"PLG_AMT:%lf:",d_plg_amt_MTM);
					fn_userlog(c_ServiceName,"INI_MARGIN:%lf:",d_init_mrgn_prcnt_MTM);
					fn_userlog(c_ServiceName,"MIN_MARGIN:%lf:",d_min_mrgn_prcnt_MTM);
					fn_userlog(c_ServiceName,"SOMC:%lf:",d_somc_prcnt_MTM);
					fn_userlog(c_ServiceName,"SPREAD:%lf:",d_spread_mrgn_MTM);
					fn_userlog(c_ServiceName,"ORDER_MARGIN_BEFORE:%lf:",d_ord_mrgn_MTM_b);
					fn_userlog(c_ServiceName,"TRADE_MARGIN_BEFORE:%lf:",d_trd_mrgn_MTM_b);
					fn_userlog(c_ServiceName,"SPREAD_MRGN_BEFORE:%lf:",d_spread_mrgn_MTM_b);
					fn_userlog(c_ServiceName,"ORDER_MARGIN_AFTER:%lf:",d_ord_mrgn_MTM_a);
					fn_userlog(c_ServiceName,"TRADE_MARGIN_AFTER:%lf:",d_trd_mrgn_MTM_a);
					fn_userlog(c_ServiceName,"SPREAD_MRGN_AFTER:%lf:",d_spread_mrgn_MTM_a);
					fn_userlog(c_ServiceName,"ADDITIONAL_MRGN:%lf:",d_add_mrgn_MTM);
					fn_userlog(c_ServiceName,"OPEN_POSITION:%ld:",l_opnpstn_MTM);
					fn_userlog(c_ServiceName,"QUOTE_PRICE:%ld:",li_cmp_MTM);	
					fn_userlog(c_ServiceName,"Tot Pldged value:%lf:",d_tot_shr_plgd_amt_MTM);	/* Ver 1.9 */
				}
					
				/*** Below d_tot_shr_plgd_amt_MTM added in Ver 1.9*/
        /*** d_fdr_amt_MTM added in Ver 2.7 ***/
 
				sprintf(c_filename,"%s/%s/fno/oimtm.txt",tuxgetenv("EBALOG_PATH"),c_filetime);
				fn_userlog(c_ServiceName,":%s:",c_filename);
				sprintf(c_filedata,"%s|%s|%s|%s|%c|%ld|%lf|%s|%c|%c|%c|%s|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%ld|%ld|%lf|%lf|",c_time,ptr_st_pstn_actn->c_cln_mtch_accnt,ptr_st_pstn_actn->c_xchng_cd,ptr_st_pstn_actn->c_undrlyng,ptr_st_pstn_actn->c_prd_typ,ptr_st_pstn_actn->l_strike_prc,d_trigger_priceMTM,c_expry_dt_MTM,ptr_st_pstn_actn->c_exrc_typ,ptr_st_pstn_actn->c_opt_typ,ptr_st_pstn_actn->c_cntrct_tag,c_bnk_accnt_MTM,d_limit_OIMTM,d_alctd_amt_MTM,d_bft_amt_MTM,d_plg_amt_MTM,d_init_mrgn_prcnt_MTM,d_min_mrgn_prcnt_MTM,d_somc_prcnt_MTM,d_spread_mrgn_MTM,d_ord_mrgn_MTM_b,d_trd_mrgn_MTM_b,d_spread_mrgn_MTM_b,d_ord_mrgn_MTM_a,d_trd_mrgn_MTM_a,d_spread_mrgn_MTM_a,d_add_mrgn_MTM,l_opnpstn_MTM,li_cmp_MTM,d_tot_shr_plgd_amt_MTM,d_fdr_amt_MTM); /*** 2.2 Margin var datatype chnage to double **/

				/** Inserting into file **/
				/**** Commented in Ver 3.0 *****
				fp = (FILE *)fopen(c_filename,"a") ;
				if(fp == NULL)
				{
    				fn_userlog(c_ServiceName,"UNABLE TO OPEN FILE FOR MTM");
				}
				fprintf(fp,"%s\n",c_filedata);
				fclose(fp);
        ****************************/

  		  /** Ver 1.2 Ends	**/

        tpfree ( ( char * ) ptr_st_err_msg ); 

      /***  START OF VER 3.0       ***/
        if(DEBUG_MSG_LVL_5)  /** ver 3.4 **/
        {
          fn_userlog(c_ServiceName,"Error case shashi");
        }

        if( ptr_st_pstn_actn->l_actn_typ == INTRADAY_MTM )
         {
          sprintf(c_temp_str,"^%s^%lf^%lf^%lf^%lf^%lf^%lf^%lf^%ld^%ld^%lf^%lf",c_expry_dt_MTM,d_ord_mrgn_MTM_b,d_trd_mrgn_MTM_b,d_spread_mrgn_MTM_b,d_ord_mrgn_MTM_a,d_trd_mrgn_MTM_a,d_spread_mrgn_MTM_a,d_add_mrgn_MTM,l_opnpstn_MTM,li_cmp_MTM,d_limit_OIMTM,d_trigger_priceMTM);

           strcat(ptr_st_pstn_actn->c_imtm_rmrks,c_temp_str);
           tpreturn ( TPFAIL, -7, (char * )ptr_st_pstn_actn, 0, 0 );
         }
          else
              tpreturn ( TPFAIL, INSUFFICIENT_LIMITS,( char * )ptr_st_pstn_actn, 0, 0 );

      /***  END OF VER 3.0   ***/
        tpreturn ( TPFAIL, INSUFFICIENT_LIMITS,( char * )ptr_st_pstn_actn, 0, 0 );
        break;

       default :
          tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
          break;
      }
    }
 }

	if ( ptr_st_pstn_actn->l_actn_typ == MRGN_STMNT ) /*** Ver 1.5 ***/
	{
		i_returncode  = fn_call_mrgn_cltn ( c_ServiceName,
                                        ptr_st_pstn_actn,
                                        &st_cntopt_pos_crrnt,
                                        &st_cntopt_pos_to_be,
                                        ptr_st_err_msg );
		
		if ( i_returncode != 0 )
		{
			fn_userlog(c_ServiceName,"Failed In Function ");
		}
		tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_pstn_actn, 0, 0 );
	}

  if ( ptr_st_pstn_actn->l_actn_typ == CANCEL_AFTER_ASSIGNMENT )
	{
    i_returncode  = fn_call_actn_can_aft_asgn ( c_ServiceName,
                                        				ptr_st_pstn_actn,
                                        				&st_cntopt_pos_crrnt,
                                        				&st_cntopt_pos_to_be,
                                        				ptr_st_err_msg );
    if ( i_returncode != 0 )
    {
if(DEBUG_MSG_LVL_3){
  fn_userlog(c_ServiceName,"Return code from cancellation after assignment:%d:",
																												i_returncode );
}
      switch ( i_returncode )
      {
        case INSUFFICIENT_LIMITS :
	/*** code has been changed here to update the db with the correct pos   **/
	/*** and the margin before doing any further action of cancelattion     **/
/************************************************************************
          tpfree ( ( char * ) ptr_st_err_msg );
      		tpreturn ( TPSUCCESS, INSUFFICIENT_LIMITS, 
																		( char * )ptr_st_pstn_actn, 0, 0 );
************************************************************************/
					c_can_aft_asgn_flg = 'Y';
          break;

        default :
          tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
          break;
      }
		}
	}

	if ( st_cntopt_pos_to_be.l_opnpstn_qty > 0 )
	{
		st_cntopt_pos_to_be.c_opnpstn_flw = BUY;
	}
	else if ( st_cntopt_pos_to_be.l_opnpstn_qty < 0 )
	{
		st_cntopt_pos_to_be.c_opnpstn_flw = SELL;
	}
	else
	{
		st_cntopt_pos_to_be.c_opnpstn_flw = NEUTRAL;
	}

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "New position"  );
	fn_userlog( c_ServiceName, "Match account     :%s:", 
																st_cntopt_pos_to_be.c_cln_mtch_accnt);
  fn_userlog( c_ServiceName, "Exchange code     :%s:", 
																st_cntopt_pos_to_be.c_xchng_cd );
  fn_userlog( c_ServiceName, "Product type      :%c:", 
																st_cntopt_pos_to_be.c_prd_typ );
  fn_userlog( c_ServiceName, "Option Type       :%c:", 
																st_cntopt_pos_to_be.c_opt_typ );
  fn_userlog( c_ServiceName, "Index / Stock     :%c:", 
																st_cntopt_pos_to_be.c_ctgry_indstk );
  fn_userlog( c_ServiceName, "Underlying        :%s:", 
																st_cntopt_pos_to_be.c_undrlyng );
  fn_userlog( c_ServiceName, "Expiry date       :%s:", 
																st_cntopt_pos_to_be.c_expry_dt );
  fn_userlog( c_ServiceName, "Strike Price      :%ld:", 
																st_cntopt_pos_to_be.l_strike_prc );
  fn_userlog( c_ServiceName, "Exercise Type     :%c:", 
																st_cntopt_pos_to_be.c_exrc_typ );
  fn_userlog( c_ServiceName, "Ibuy qty          :%ld:", 
																st_cntopt_pos_to_be.l_ibuy_qty );
  fn_userlog( c_ServiceName, "Ibuy order val    :%lf:", 
																st_cntopt_pos_to_be.d_ibuy_ord_vl );
  fn_userlog( c_ServiceName, "Isell qty         :%ld:", 
																st_cntopt_pos_to_be.l_isell_qty );
  fn_userlog( c_ServiceName, "Isell order val   :%lf:", 
																st_cntopt_pos_to_be.d_isell_ord_vl );
  fn_userlog( c_ServiceName, "Exbuy qty         :%ld:", 
																st_cntopt_pos_to_be.l_exbuy_qty );
  fn_userlog( c_ServiceName, "Ex buy order val  :%lf:", 
																st_cntopt_pos_to_be.d_exbuy_ord_vl );
  fn_userlog( c_ServiceName, "Ex Sell qty       :%ld:",
																st_cntopt_pos_to_be.l_exsell_qty );
  fn_userlog( c_ServiceName, "Ex sell order val :%lf:",
																st_cntopt_pos_to_be.d_exsell_ord_vl );
  fn_userlog( c_ServiceName, "Buy Executed qty  :%ld:",
																st_cntopt_pos_to_be.l_buy_exctd_qty );
  fn_userlog( c_ServiceName, "Sell Executed qty :%ld:",
																st_cntopt_pos_to_be.l_sell_exctd_qty );
  fn_userlog( c_ServiceName, "Open Position Flow:%c:", 
																st_cntopt_pos_to_be.c_opnpstn_flw );
  fn_userlog( c_ServiceName, "Open position qty :%ld:", 
																st_cntopt_pos_to_be.l_opnpstn_qty );
  fn_userlog( c_ServiceName, "Open value        :%lf:", 
																st_cntopt_pos_to_be.d_org_opn_val );
  fn_userlog( c_ServiceName, "Order Margin      :%lf:", 
																st_cntopt_pos_to_be.d_ordr_mrgn );
  fn_userlog( c_ServiceName, "Trade Margin      :%lf:", 
																st_cntopt_pos_to_be.d_trd_mrgn );
  fn_userlog( c_ServiceName, "Iexercise qty     :%ld:", 
																st_cntopt_pos_to_be.l_iexrc_qty );
  fn_userlog( c_ServiceName, "Ex-exercise qty   :%ld:", 
																st_cntopt_pos_to_be.l_exexrc_qty );
  fn_userlog( c_ServiceName, "Exercised qty     :%ld:", 
																st_cntopt_pos_to_be.l_exrc_qty );
  fn_userlog( c_ServiceName, "DP Qty            :%ld:", 
																st_cntopt_pos_to_be.l_ord_dp_qty );
  fn_userlog( c_ServiceName, "Assigned qty      :%ld:", 
																st_cntopt_pos_to_be.l_asgnd_qty );
  fn_userlog( c_ServiceName, "IMTM Trigger price:%ld:", 
																st_cntopt_pos_to_be.l_mtm_trg_prc );
  fn_userlog( c_ServiceName, "I dontexercise qty     :%ld:", 
																st_cntopt_pos_to_be.l_idont_exrc_qty );
  fn_userlog( c_ServiceName, "Ex-dontexercise qty   :%ld:", 
																st_cntopt_pos_to_be.l_exdont_exrc_qty );
}

	d_value = ( double )st_cntopt_pos_to_be.l_mtm_trg_prc;

	if ( st_cntopt_pos_to_be.c_opt_typ == CALL )
	{
		i_up_down = UPPER;
	}
	else if ( st_cntopt_pos_to_be.c_opt_typ == PUT )
	{
		i_up_down = LOWER;
	}

	d_trg_prc = 0.0;
	
	i_returncode = fn_nearest_paise_uop( c_ServiceName, 
                                   d_value, 
                                   5, 
                                   i_up_down,
                                   c_err_msg,
                                   &d_trg_prc );

	st_cntopt_pos_to_be.l_mtm_trg_prc = d_trg_prc;

  if(ptr_st_pstn_actn->l_actn_typ != EOD_MRGN_REP ) /** If condition added in ver 4.1 **/
  {
	/** Update the position table **/
	if( c_cpos_avl == 'N')
	{

    EXEC SQL
			/*** SELECT to_char(exg_nxt_trd_dt,'DD-Mon-YYYY')               Commented In Ver 2.3 ***/
			/*** SELECT to_char(GREATEST(exg_nxt_trd_dt,exg_tmp_trd_dt),'DD-Mon-YYYY') *** Ver 2.3 Reverted In Ver 2.3 Bug Fix ***/
      SELECT to_char(exg_nxt_trd_dt,'DD-Mon-YYYY')   /*** Reverted In Ver 2.3 Bug Fix ***/
      INTO   :c_trade_dt
      FROM   exg_xchng_mstr
      WHERE  exg_xchng_cd = :ptr_st_pstn_actn->c_xchng_cd
      AND    exg_mkt_typ = 'D';

    if ( SQLCODE != 0 )
    {
       fn_errlog(c_ServiceName, "S31025", SQLMSG, ptr_st_err_msg->c_err_msg );
       tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
    }

    EXEC SQL
      INSERT INTO fop_fo_options_pstn
      (
    		fop_clm_mtch_accnt,
				fop_xchng_cd,
				fop_prdct_typ,
				fop_opt_typ,
				fop_indstk,
				fop_undrlyng,
				fop_expry_dt,
				fop_strk_prc,
				fop_exer_typ,
				fop_ibuy_qty,
				fop_ibuy_ord_val,
				fop_isell_qty,
				fop_isell_ord_val,
				fop_exbuy_qty,
				fop_exbuy_ord_val,
				fop_exsell_qty,
				fop_exsell_ord_val,
				fop_buy_exctd_qty,
				fop_sell_exctd_qty,
				fop_opnpstn_flw,
				fop_opnpstn_qty,
				fop_opnpstn_val,
				fop_uordr_mrgn,
				fop_uexctd_mrgn,
				fop_iexrc_qty,
				fop_exexrc_qty,
				fop_exrc_qty,
				fop_dp_qty,
				fop_asgnd_qty,
				fop_trggr_prc,
				fop_idontexrc_qty,
				fop_exdontexrc_qty,
				fop_trd_dt
			)
      VALUES
      (
				:ptr_st_pstn_actn->c_cln_mtch_accnt,
        :st_cntopt_pos_to_be.c_xchng_cd,
        :st_cntopt_pos_to_be.c_prd_typ,
        :st_cntopt_pos_to_be.c_opt_typ,
        :st_cntopt_pos_to_be.c_ctgry_indstk,
        :st_cntopt_pos_to_be.c_undrlyng,
        :st_cntopt_pos_to_be.c_expry_dt,
        :st_cntopt_pos_to_be.l_strike_prc, 
        :st_cntopt_pos_to_be.c_exrc_typ,
        :st_cntopt_pos_to_be.l_ibuy_qty,
        :st_cntopt_pos_to_be.d_ibuy_ord_vl,
        :st_cntopt_pos_to_be.l_isell_qty,
        :st_cntopt_pos_to_be.d_isell_ord_vl,
        :st_cntopt_pos_to_be.l_exbuy_qty,
        :st_cntopt_pos_to_be.d_exbuy_ord_vl,
        :st_cntopt_pos_to_be.l_exsell_qty,
        :st_cntopt_pos_to_be.d_exsell_ord_vl,
        :st_cntopt_pos_to_be.l_buy_exctd_qty,
        :st_cntopt_pos_to_be.l_sell_exctd_qty,
        :st_cntopt_pos_to_be.c_opnpstn_flw,
        :st_cntopt_pos_to_be.l_opnpstn_qty,
        :st_cntopt_pos_to_be.d_org_opn_val,
        :st_cntopt_pos_to_be.d_ordr_mrgn,
        :st_cntopt_pos_to_be.d_trd_mrgn,
        :st_cntopt_pos_to_be.l_iexrc_qty,
        :st_cntopt_pos_to_be.l_exexrc_qty,
        :st_cntopt_pos_to_be.l_exrc_qty,
        :st_cntopt_pos_to_be.l_ord_dp_qty,
        :st_cntopt_pos_to_be.l_asgnd_qty,
        :st_cntopt_pos_to_be.l_mtm_trg_prc,
        :st_cntopt_pos_to_be.l_idont_exrc_qty,
        :st_cntopt_pos_to_be.l_exdont_exrc_qty,
				:c_trade_dt
			);
      if ( SQLCODE != 0 )
      {
        fn_errlog(c_ServiceName, "S31030", SQLMSG, ptr_st_err_msg->c_err_msg );
        tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
      }
	}
	else
	{

    /** Ver 2.9 Added ** Changes to avoid incorrect margin blocked and exercise quantity, after newposition taken in same contract after Adhoc expiry of particular underlying **/
    if( ptr_st_pstn_actn->l_actn_typ == EXER_CONF )
    {
      st_cntopt_pos_to_be.l_iexrc_qty  = 0;
      st_cntopt_pos_to_be.l_exexrc_qty = 0;
    }

    /** Ver 2.9 Ends **/

    EXEC SQL
      UPDATE  fop_fo_options_pstn
			SET			fop_ibuy_qty 			=	:st_cntopt_pos_to_be.l_ibuy_qty,
							fop_ibuy_ord_val 	=	:st_cntopt_pos_to_be.d_ibuy_ord_vl,
							fop_isell_qty			=	:st_cntopt_pos_to_be.l_isell_qty,
							fop_isell_ord_val =	:st_cntopt_pos_to_be.d_isell_ord_vl,
							fop_exbuy_qty 		=	:st_cntopt_pos_to_be.l_exbuy_qty,
							fop_exbuy_ord_val =	:st_cntopt_pos_to_be.d_exbuy_ord_vl,
							fop_exsell_qty 		=	:st_cntopt_pos_to_be.l_exsell_qty,
							fop_exsell_ord_val=	:st_cntopt_pos_to_be.d_exsell_ord_vl,
							fop_buy_exctd_qty =	:st_cntopt_pos_to_be.l_buy_exctd_qty,
							fop_sell_exctd_qty= :st_cntopt_pos_to_be.l_sell_exctd_qty,
							fop_opnpstn_flw 	=	:st_cntopt_pos_to_be.c_opnpstn_flw,
							fop_opnpstn_qty 	=	:st_cntopt_pos_to_be.l_opnpstn_qty,
							fop_opnpstn_val 	=	:st_cntopt_pos_to_be.d_org_opn_val,
							fop_uordr_mrgn 		=	:st_cntopt_pos_to_be.d_ordr_mrgn,
							fop_uexctd_mrgn 	=	:st_cntopt_pos_to_be.d_trd_mrgn,
							fop_iexrc_qty 			=	:st_cntopt_pos_to_be.l_iexrc_qty,
							fop_exexrc_qty 			=	:st_cntopt_pos_to_be.l_exexrc_qty,
							fop_exrc_qty 			=	:st_cntopt_pos_to_be.l_exrc_qty,
							fop_dp_qty 				=	:st_cntopt_pos_to_be.l_ord_dp_qty,
							fop_asgnd_qty 		=	:st_cntopt_pos_to_be.l_asgnd_qty,
							fop_trggr_prc 		=	:st_cntopt_pos_to_be.l_mtm_trg_prc,
							fop_idontexrc_qty 		=	:st_cntopt_pos_to_be.l_idont_exrc_qty,
							fop_exdontexrc_qty 		=	:st_cntopt_pos_to_be.l_exdont_exrc_qty
    	WHERE 	fop_clm_mtch_accnt  = :ptr_st_pstn_actn->c_cln_mtch_accnt
    	AND   	fop_xchng_cd        = :ptr_st_pstn_actn->c_xchng_cd
    	AND   	fop_prdct_typ       = :ptr_st_pstn_actn->c_prd_typ
    	AND   	fop_undrlyng        = :ptr_st_pstn_actn->c_undrlyng
    	AND   	fop_exer_typ      	= :ptr_st_pstn_actn->c_exrc_typ
    	AND   	fop_opt_typ      		= :ptr_st_pstn_actn->c_opt_typ
    	AND   	fop_expry_dt      	= :ptr_st_pstn_actn->c_expry_dt
    	AND   	fop_strk_prc      	= :ptr_st_pstn_actn->l_strike_prc;

   	if ( SQLCODE != 0 )
   	{
     	fn_errlog(c_ServiceName, "S31035", SQLMSG, ptr_st_err_msg->c_err_msg );
     	tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 );
   	}

		/*** ver 2.6 Starts **/

   if( (ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF ||
        ptr_st_pstn_actn->l_actn_typ == INTRADAY_MTM || 
			  c_imtm_addmrgn_flg           == 'Y'          ||
				ptr_st_pstn_actn->l_actn_typ == ADD_MARGIN ) && 
			 st_cntopt_pos_to_be.l_opnpstn_qty != 0				&& 
			 st_cntopt_pos_to_be.c_opnpstn_flw == 'S' 		&&
		   ptr_st_pstn_actn->c_prd_typ == 'O'						&&
			 st_cntopt_pos_to_be.l_mtm_trg_prc != 0       &&
       c_mtm_flag != UNDER_MTM 
		 )
 		 {
       i_returncode =fn_upd_ins_trgprc(c_ServiceName,
                                        ptr_st_pstn_actn,
                                        st_cntopt_pos_to_be,
                                        ptr_st_err_msg
                                      );

       if ( i_returncode != 0 )
       {
          fn_userlog(c_ServiceName,"Trigger price not inserted/updated");
          fn_errlog(c_ServiceName, "S31040", "Trigger price not inserted/updated", ptr_st_err_msg->c_err_msg );
       }
     }

	 /** Ver 2.6 Ends **/
	}

  if(DEBUG_MSG_LVL_3){
     fn_userlog(c_ServiceName,"c_mtm_flag **[%c]",c_mtm_flag);
  }

  ptr_st_pstn_actn->c_mtm_flag = 'O'; /* default */
  } /** Ver 4.1 **/

 fn_userlog(c_ServiceName,"Suchita ib :%ld: eb :%ld isl :%ld: exs :%ld: mtm :%c:",st_cntopt_pos_to_be.l_ibuy_qty,st_cntopt_pos_to_be.l_exbuy_qty,st_cntopt_pos_to_be.l_isell_qty,st_cntopt_pos_to_be.l_exsell_qty,c_mtm_flag);

  if ( ( c_mtm_flag == UNDER_MTM )             &&
       ( st_cntopt_pos_to_be.l_ibuy_qty == 0 )   &&
       ( st_cntopt_pos_to_be.l_exbuy_qty == 0 )  &&
       ( st_cntopt_pos_to_be.l_isell_qty == 0 )  &&
       ( st_cntopt_pos_to_be.l_exsell_qty == 0 ) )
  {
    ptr_st_pstn_actn->c_mtm_flag = NO_OPEN_ORDERS;
  }
	 
	/** Ver 3.8 Starts Here ***/
  else if ( ( c_mtm_flag == UNDER_PHYDLVRY_MTM )             &&
       ( st_cntopt_pos_to_be.l_ibuy_qty == 0 )   &&
       ( st_cntopt_pos_to_be.l_exbuy_qty == 0 )  &&
       ( st_cntopt_pos_to_be.l_isell_qty == 0 )  &&
       ( st_cntopt_pos_to_be.l_exsell_qty == 0 ) )
  {
    ptr_st_pstn_actn->c_mtm_flag = NO_PHYDLVRY_OPEN_ORDERS;
    fn_userlog( c_ServiceName," suchita ptr_st_pstn_actn->c_mtm_flag is :%c:",ptr_st_pstn_actn->c_mtm_flag);
  }
	 /*** Ver 3.8 Ends here ***/

  else if ( c_mtm_flag == UNDER_MTM )
  {
    ptr_st_pstn_actn->c_mtm_flag = FOR_REMARKS_ENTRY;
  }

 if(DEBUG_MSG_LVL_3){
    fn_userlog(c_ServiceName,"pstn_actn.c_mtm_flag **[%c]",
                              ptr_st_pstn_actn->c_mtm_flag);
    fn_userlog(c_ServiceName,"Tanmay c_can_aft_asgn_flg [%c]",c_can_aft_asgn_flg);  /** Tanmay **/
 }


  tpfree ( ( char * ) ptr_st_err_msg ); 

	if ( c_can_aft_asgn_flg == 'Y' )
	{
   	tpreturn ( TPSUCCESS, INSUFFICIENT_LIMITS, 
																		( char * )ptr_st_pstn_actn, 0, 0 );
	}
	else
	{
  	tpreturn ( TPSUCCESS, SUCC_BFR, ( char * )ptr_st_pstn_actn, 0, 0 );
	}
}

/** Applies an action on an existing position for option type PUT **/
int 	fn_apply_cnt_actn_put ( 	char *c_ServiceName,
													struct vw_pstn_actn *ptr_st_pstn_actn,
												 	struct vw_cntopt_pos *ptr_st_cntopt_pos,
                  				struct vw_err_msg *ptr_st_err_msg)
{

  struct vw_contract st_cntrct;
  struct vw_cntrct_qt st_cntrct_qt;
	long int li_temp_qty;
	long int li_flg_tmp_qty;

	if(DEBUG_MSG_LVL_3){
		fn_userlog( c_ServiceName, "Applying an action on an Existing Position" );
	}

  switch ( ptr_st_pstn_actn->l_actn_typ )
  {
    case  ORS_NEW_ORD_REQ:
    case  SYSTEM_PLACE_ORDER:
					/** Increase the IWTL position by the order qty and order rate **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_add_pos( ptr_st_pstn_actn->l_new_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_add_pos( ( ptr_st_pstn_actn->l_new_qty * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_NEW_ORD_ACPT:
					/** Increase the EXCH position by the order qty and order rate **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_add_pos( ptr_st_pstn_actn->l_new_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );
					}
					else
					{
						fn_add_pos( ( ptr_st_pstn_actn->l_new_qty * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );
					}
					break;

    case  ORS_NEW_ORD_RJCT:
					/** Decrease the IWTL position by the order qty and order rate **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_new_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ptr_st_pstn_actn->l_new_qty * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_MOD_ORD_REQ:
					/** Decrease the IWTL position by the unexecuted order qty and **/
          /** order rate. Increase the IWTL position by the unexecuted   **/
					/** new qty and new rate                                       **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );

						fn_add_pos( ptr_st_pstn_actn->l_new_qty -
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );

						fn_add_pos( ( ( ptr_st_pstn_actn->l_new_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_MOD_ORD_ACPT:
					/** Decrease the EXCH position by the unexecuted order qty and **/
          /** order rate. Increase the EXCH position by the unexecuted   **/
					/** new qty and new rate                                       **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );

						fn_add_pos( ptr_st_pstn_actn->l_new_qty -
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );

						fn_add_pos( ( ( ptr_st_pstn_actn->l_new_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );
					}
					break;

    case  ORS_MOD_ORD_RJCT:
					/** Decrease the IWTL position by the unexecuted new qty and **/
          /** new rate. Increase the IWTL position by the unexecuted   **/
					/** order qty and order rate                                 **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_new_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );

						fn_add_pos( ptr_st_pstn_actn->l_orgnl_qty -
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_new_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );

						fn_add_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_CAN_ORD_REQ:
					/** Decrease the IWTL position by the unexecuted order qty  **/
					/** and order rate                                          **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_CAN_ORD_ACPT:
					/** Decrease the EXCH position by the unexecuted order qty  **/
					/** and order rate                                          **/
          if( DEBUG_MSG_LVL_4 ) /** Ver 3.4 added in debug **/
          {
					  fn_userlog(c_ServiceName,"Inside ORS_CAN_ORD_ACPT ");			/* Ver 1.7 */
            fn_userlog(c_ServiceName,"Original Qty Is :%ld:",ptr_st_pstn_actn->l_orgnl_qty);	/* Ver 1.7 */
            fn_userlog(c_ServiceName,"Executed Qty Is :%ld:",ptr_st_pstn_actn->l_exec_qty);	  /* Ver 1.7 */
          }

          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );

						/***	Ver 1.7 Starts	***/ 
						/** if(ptr_st_cntopt_pos->l_ibuy_qty != ptr_st_cntopt_pos->l_exbuy_qty) commented in ver 3.7 **/
            if( ptr_st_pstn_actn->l_new_qty > 0 ) /** ver 3.7 **/ /** add unprocessed traded qty to iBuy **/	
						{
              if( DEBUG_MSG_LVL_4 ) /** Ver 3.4 added in debug **/
              {
							  fn_userlog(c_ServiceName,"Inside Put Buy Option");
							  fn_userlog(c_ServiceName,"Before IWTL QTY IS :%ld:",ptr_st_cntopt_pos->l_ibuy_qty);
							  fn_userlog(c_ServiceName,"Before EXCHNG QTY IS :%ld:", ptr_st_cntopt_pos->l_exbuy_qty);
							  fn_userlog(c_ServiceName,"Here Executed QTY IS :%ld:",ptr_st_pstn_actn->l_new_qty);
              }
             
              /*** ver 3.7 starts ***/
              if(DEBUG_MSG_LVL_0)
              {
                fn_userlog(c_ServiceName,"PosiTion MisMatch 1: iBuy Quantity before adding the unprocessed executed quantity is :%ld: and total unprocessed executed quantity is :%ld:",ptr_st_cntopt_pos->l_ibuy_qty,ptr_st_pstn_actn->l_new_qty);                           
              }
              /*** ver 3.7 ends ***/
 
							fn_add_pos(ptr_st_pstn_actn->l_new_qty,			
          							 ptr_st_pstn_actn->l_orgnl_rt,
          							 &(ptr_st_cntopt_pos->l_ibuy_qty),
          							 &(ptr_st_cntopt_pos->d_ibuy_ord_vl ) ); 

              /*** ver 3.7 starts ***/
              if(DEBUG_MSG_LVL_0)
              {
                fn_userlog(c_ServiceName,"PosiTion MisMatch 2: Corrected iBuy qty l_ibuy_qty is :%ld: Corrected exBuy qty l_exbuy_qty is :%ld:",ptr_st_cntopt_pos->l_ibuy_qty,ptr_st_cntopt_pos->l_exbuy_qty);
              }
               
              /*** ver 3.7 ends ***/
          
              if( DEBUG_MSG_LVL_4 ) /** Ver 3.4 added in debug **/
              { 
							  fn_userlog(c_ServiceName,"After IWTL QTY IS :%ld:",ptr_st_cntopt_pos->l_ibuy_qty);
							  fn_userlog(c_ServiceName,"After EXCHNG QTY IS :%ld:", ptr_st_cntopt_pos->l_exbuy_qty);
              }
						}
						/***  Ver 1.7 Ends  ***/

					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );

						/***  Ver 1.7 Starts  ***/
            
            /**	if(ptr_st_cntopt_pos->l_isell_qty != ptr_st_cntopt_pos->l_exsell_qty) commented in ver 3.7 **/
            if( ptr_st_pstn_actn->l_new_qty > 0 ) /** ver 3.7 Add unprocessed traded qty to existing iBuy **/  

						{
              if( DEBUG_MSG_LVL_4 ) /** Ver 3.4 added in debug **/
              {
							  fn_userlog(c_ServiceName,"Inside Put Sell Option");
						    fn_userlog(c_ServiceName,"Before IWTL QTY IS :%ld:",ptr_st_cntopt_pos->l_isell_qty);
							  fn_userlog(c_ServiceName,"Before EXCHNG QTY IS :%ld:",ptr_st_cntopt_pos->l_exsell_qty);
							  fn_userlog(c_ServiceName,"Here Executed QTY IS :%ld:",ptr_st_pstn_actn->l_new_qty);
	            }
             
              /*** ver 3.7 starts ***/
              if(DEBUG_MSG_LVL_0)
              {
                fn_userlog(c_ServiceName,"PosiTion MisMatch a: iSell Quantity before adding the unprocessed executed quantity is :%ld: and total unprocessed executed quantity is :%ld:",ptr_st_cntopt_pos->l_isell_qty,ptr_st_pstn_actn->l_new_qty);
              }
              /*** ver 3.7 ends ***/
 
							fn_add_pos(ptr_st_pstn_actn->l_new_qty * (-1),
          							 ptr_st_pstn_actn->l_orgnl_rt,
          							 &(ptr_st_cntopt_pos->l_isell_qty),
          							 &(ptr_st_cntopt_pos->d_isell_ord_vl ) );
 
              /*** ver 3.7 starts ***/
              if(DEBUG_MSG_LVL_0)
              {
                 fn_userlog(c_ServiceName,"PosiTion MisMatch b: Corrected iSell Qty l_isell_qty is :%ld: and Corrected exSell Qty l_exsell_qty is :%ld:",ptr_st_cntopt_pos->l_isell_qty,ptr_st_cntopt_pos->l_exsell_qty);
              }

              /*** ver 3.7 ends ***/

              if( DEBUG_MSG_LVL_4 ) /** Ver 3.4 added in debug **/
              { 
 							  fn_userlog(c_ServiceName,"After IWTL QTY IS :%ld:",ptr_st_cntopt_pos->l_isell_qty);
							  fn_userlog(c_ServiceName,"After EXCHNG QTY IS :%ld:",ptr_st_cntopt_pos->l_exsell_qty);
              }
						}
						/***  Ver 1.7 Ends  ***/
					}
					break;

    case  ORS_CAN_ORD_RJCT:
					/** Increase the IWTL position by the unexecuted order qty  **/
					/** and order rate                                          **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_add_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_add_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_TRD_CONF:
					/** Decrease the IWTL, EXCH position by the executed qty **/
					/** order rate. Increase the OPEN position by the        **/
					/** executed qty and executed rate. Calculate the fresh  **/
					/** and cover quantity in the transaction. Increase the  **/
					/** MTMed position too by the executed quantity and      **/
					/** executed rate. Update the buy executed and sell      **/
					/** executed quantity                                    **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {

						fn_del_pos( ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );

						fn_del_pos( ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );


						if( ptr_st_cntopt_pos->l_opnpstn_qty > 0 )
            {
              ptr_st_pstn_actn->l_fresh_qty = ptr_st_pstn_actn->l_exec_qty;
              ptr_st_pstn_actn->l_cover_qty = 0;
            }
            else
            {
              ptr_st_pstn_actn->l_cover_qty =
                        fn_minl(ptr_st_pstn_actn->l_exec_qty,
                                labs(ptr_st_cntopt_pos->l_opnpstn_qty) );

              ptr_st_pstn_actn->l_fresh_qty =
                        fn_maxl(ptr_st_pstn_actn->l_exec_qty -
                                ptr_st_pstn_actn->l_cover_qty,
                                0 );
            }

						if ( ptr_st_pstn_actn->l_cover_qty > 0 )
						{
							fn_add_pos_in_val ( ptr_st_pstn_actn->l_cover_qty,
                               ptr_st_cntopt_pos->d_org_opn_val,
                               ptr_st_cntopt_pos->l_opnpstn_qty,
													     &(ptr_st_cntopt_pos->l_opnpstn_qty),
													     &(ptr_st_cntopt_pos->d_org_opn_val ) );

						}

						if ( ptr_st_pstn_actn->l_fresh_qty > 0 )
						{
							fn_add_pos( ptr_st_pstn_actn->l_fresh_qty,
													ptr_st_pstn_actn->l_exec_rt,
													&(ptr_st_cntopt_pos->l_opnpstn_qty),
													&(ptr_st_cntopt_pos->d_org_opn_val ) );

						}

						ptr_st_cntopt_pos->l_buy_exctd_qty = 
																			ptr_st_cntopt_pos->l_buy_exctd_qty + 
																			ptr_st_pstn_actn->l_exec_qty;
					}
					else
					{

						fn_del_pos( ptr_st_pstn_actn->l_exec_qty * (-1),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );

						fn_del_pos( ptr_st_pstn_actn->l_exec_qty * (-1),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );

						if( ptr_st_cntopt_pos->l_opnpstn_qty < 0 )
            {
              ptr_st_pstn_actn->l_fresh_qty = ptr_st_pstn_actn->l_exec_qty;
              ptr_st_pstn_actn->l_cover_qty = 0;
            }
            else
            {
              ptr_st_pstn_actn->l_cover_qty =
                        fn_minl(ptr_st_pstn_actn->l_exec_qty,
                                ptr_st_cntopt_pos->l_opnpstn_qty );

              ptr_st_pstn_actn->l_fresh_qty =
                        fn_maxl(ptr_st_pstn_actn->l_exec_qty -
                                ptr_st_pstn_actn->l_cover_qty,
                                0 );
            }

						if ( ptr_st_pstn_actn->l_cover_qty > 0 )
						{
							fn_add_pos_in_val ( ptr_st_pstn_actn->l_cover_qty * ( -1 ),
                               ptr_st_cntopt_pos->d_org_opn_val,
                               ptr_st_cntopt_pos->l_opnpstn_qty,
													     &(ptr_st_cntopt_pos->l_opnpstn_qty),
													     &(ptr_st_cntopt_pos->d_org_opn_val ) );

						}

						if ( ptr_st_pstn_actn->l_fresh_qty > 0 )
						{
							fn_add_pos( ptr_st_pstn_actn->l_fresh_qty * (-1),
													ptr_st_pstn_actn->l_exec_rt,
													&(ptr_st_cntopt_pos->l_opnpstn_qty),
													&(ptr_st_cntopt_pos->d_org_opn_val ) );

						}

						ptr_st_cntopt_pos->l_sell_exctd_qty = 
																		ptr_st_cntopt_pos->l_sell_exctd_qty + 
																		(ptr_st_pstn_actn->l_exec_qty *(-1));
					}

					ptr_st_cntopt_pos->l_opnpstn_qty = 
                           ptr_st_cntopt_pos->l_buy_exctd_qty +
                           ptr_st_cntopt_pos->l_sell_exctd_qty;

					break;

    case  ORS_ORD_EXP:
					/** Decrease the IWTL,EXCH position by the unexecuted order qty  **/
					/** and order rate                                               **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );

						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty -
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );

						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );

					}
					break;

    case  ORS_NEW_EXER_REQ:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
					{
						ptr_st_cntopt_pos->l_iexrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					break;

    case  ORS_NEW_EXER_ACPT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
					{
						ptr_st_cntopt_pos->l_exexrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_exdont_exrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					break;

    case  ORS_NEW_EXER_RJCT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
					{
						ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_new_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty -= ptr_st_pstn_actn->l_new_qty;
					}
					break;

    case  ORS_MOD_EXER_REQ:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
					{
						ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
						ptr_st_cntopt_pos->l_iexrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty -=ptr_st_pstn_actn->l_orgnl_qty;
						ptr_st_cntopt_pos->l_idont_exrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					break;

    case  ORS_MOD_EXER_ACPT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
					{
						ptr_st_cntopt_pos->l_exexrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
						ptr_st_cntopt_pos->l_exexrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_exdont_exrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
						ptr_st_cntopt_pos->l_exdont_exrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					break;

    case  ORS_MOD_EXER_RJCT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
					{
						ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_new_qty;
						ptr_st_cntopt_pos->l_iexrc_qty += ptr_st_pstn_actn->l_orgnl_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty -= ptr_st_pstn_actn->l_new_qty;
						ptr_st_cntopt_pos->l_idont_exrc_qty +=ptr_st_pstn_actn->l_orgnl_qty;
					}
					break;

    case  ORS_CAN_EXER_REQ:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
					{
						ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty -=ptr_st_pstn_actn->l_orgnl_qty;
					}
					break;

    case  ORS_CAN_EXER_ACPT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
					{
						ptr_st_cntopt_pos->l_exexrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_exdont_exrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
					}
					break;

    case  ORS_CAN_EXER_RJCT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
					{
						ptr_st_cntopt_pos->l_iexrc_qty += ptr_st_pstn_actn->l_orgnl_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty +=ptr_st_pstn_actn->l_orgnl_qty;
					}
					break;

    case  EXER_CONF:
					ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_exec_qty;
					ptr_st_cntopt_pos->l_exexrc_qty -= ptr_st_pstn_actn->l_exec_qty;

					fn_del_pos_in_val ( ptr_st_pstn_actn->l_exec_qty,
                           ptr_st_cntopt_pos->d_org_opn_val,
                           ptr_st_cntopt_pos->l_opnpstn_qty,
											     &(ptr_st_cntopt_pos->l_opnpstn_qty),
											     &(ptr_st_cntopt_pos->d_org_opn_val ) );

					ptr_st_cntopt_pos->l_buy_exrc_qty += ptr_st_pstn_actn->l_exec_qty;
					ptr_st_cntopt_pos->l_buy_exctd_qty -= ptr_st_pstn_actn->l_exec_qty;

					/*** added on 18-May-2002 ****/
					ptr_st_cntopt_pos->l_exrc_qty += ptr_st_pstn_actn->l_exec_qty;
					break;

    case  EXER_RJCT:
					ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_exec_qty;
					ptr_st_cntopt_pos->l_exexrc_qty -= ptr_st_pstn_actn->l_exec_qty;
					break;

    case  ASGMNT:
					fn_del_pos_in_val ( ptr_st_pstn_actn->l_exec_qty * -1,
                           ptr_st_cntopt_pos->d_org_opn_val,
                           ptr_st_cntopt_pos->l_opnpstn_qty,
										     	 &(ptr_st_cntopt_pos->l_opnpstn_qty),
											     &(ptr_st_cntopt_pos->d_org_opn_val ) );

					ptr_st_cntopt_pos->l_sell_asgn_qty += ptr_st_pstn_actn->l_exec_qty;
					ptr_st_cntopt_pos->l_sell_exctd_qty -= 
																		(ptr_st_pstn_actn->l_exec_qty * (-1));

					/*** added on 18-May-2002 ****/
					ptr_st_cntopt_pos->l_asgnd_qty += ptr_st_pstn_actn->l_exec_qty;
					break;

    case  CLOSE_OUT_DEALS:
          /** Increase the OPEN position by the close out qty and  **/
          /** WEP rate.executed rate.                              **/

          ptr_st_pstn_actn->l_orgnl_qty =
                     ptr_st_cntopt_pos->l_opnpstn_qty * -1;
          ptr_st_pstn_actn->c_trnsctn_flw = ptr_st_cntopt_pos->c_opnpstn_flw;

          li_temp_qty = ptr_st_cntopt_pos->l_opnpstn_qty;
          li_flg_tmp_qty = ptr_st_cntopt_pos->l_opnpstn_qty;

          fn_add_pos_in_val(  li_temp_qty * (-1),
                              ptr_st_cntopt_pos->d_org_opn_val,
                              li_temp_qty,
                              &(ptr_st_cntopt_pos->l_opnpstn_qty),
                              &(ptr_st_cntopt_pos->d_org_opn_val ) );

          if ( li_flg_tmp_qty > 0 )
          {
            ptr_st_cntopt_pos->l_sell_exctd_qty = (-1) *
                                          ptr_st_cntopt_pos->l_buy_exctd_qty ;
          }
          else if ( li_flg_tmp_qty < 0 )
          {
            ptr_st_cntopt_pos->l_buy_exctd_qty = (-1) *
                                          ptr_st_cntopt_pos->l_sell_exctd_qty ;
          }

/************************************************************************
          ptr_st_cntopt_pos->l_buy_exctd_qty = 0;
          ptr_st_cntopt_pos->l_sell_exctd_qty = 0;
************************************************************************/
					break;

	}

  return SUCCESS;
}

/** Applies an action on an existing position for option type CALL **/
int 	fn_apply_cnt_actn_call ( 	char *c_ServiceName,
													struct vw_pstn_actn *ptr_st_pstn_actn,
												 	struct vw_cntopt_pos *ptr_st_cntopt_pos,
                  				struct vw_err_msg *ptr_st_err_msg)
{

	double d_blkd_vl=0.0;
	double d_premium=0.0;
	long int li_temp_qty;
	long int li_flg_tmp_qty;

	if(DEBUG_MSG_LVL_3){
		fn_userlog( c_ServiceName, "Applying an action on an Existing Position" );
	}

  switch ( ptr_st_pstn_actn->l_actn_typ )
  {
    case  ORS_NEW_ORD_REQ:
    case  SYSTEM_PLACE_ORDER:
					/** Increase the IWTL position by the order qty and order rate **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_add_pos( ptr_st_pstn_actn->l_new_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_add_pos( ( ptr_st_pstn_actn->l_new_qty * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_NEW_ORD_ACPT:
					/** Increase the EXCH position by the order qty and order rate **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_add_pos( ptr_st_pstn_actn->l_new_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );
					}
					else
					{
						fn_add_pos( ( ptr_st_pstn_actn->l_new_qty * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );
					}
					break;

    case  ORS_NEW_ORD_RJCT:
					/** Decrease the IWTL position by the order qty and order rate **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_new_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ptr_st_pstn_actn->l_new_qty * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_MOD_ORD_REQ:
					/** Decrease the IWTL position by the unexecuted order qty and **/
          /** order rate. Increase the IWTL position by the unexecuted   **/
					/** new qty and new rate                                       **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );

						fn_add_pos( ptr_st_pstn_actn->l_new_qty -
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );

						fn_add_pos( ( ( ptr_st_pstn_actn->l_new_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_MOD_ORD_ACPT:
					/** Decrease the EXCH position by the unexecuted order qty and **/
          /** order rate. Increase the EXCH position by the unexecuted   **/
					/** new qty and new rate                                       **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );

						fn_add_pos( ptr_st_pstn_actn->l_new_qty -
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );

						fn_add_pos( ( ( ptr_st_pstn_actn->l_new_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );
					}
					break;

    case  ORS_MOD_ORD_RJCT:
					/** Decrease the IWTL position by the unexecuted new qty and **/
          /** new rate. Increase the IWTL position by the unexecuted   **/
					/** order qty and order rate                                 **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_new_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );

						fn_add_pos( ptr_st_pstn_actn->l_orgnl_qty -
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_new_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_new_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );

						fn_add_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_CAN_ORD_REQ:
					/** Decrease the IWTL position by the unexecuted order qty  **/
					/** and order rate                                          **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_CAN_ORD_ACPT:
					/** Decrease the EXCH position by the unexecuted order qty  **/
					/** and order rate                                          **/

          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );
		
						/***	Ver 1.7 Starts	***/
						if(ptr_st_cntopt_pos->l_ibuy_qty != ptr_st_cntopt_pos->l_exbuy_qty)
						{
              if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
              {
							  fn_userlog(c_ServiceName,"Inside Call Buy Position ");
							  fn_userlog(c_ServiceName,"Before IWTL BUY QTY IS :%ld:",ptr_st_cntopt_pos->l_ibuy_qty);
							  fn_userlog(c_ServiceName,"Before EXCHNG BUY QTY IS :%ld:",ptr_st_cntopt_pos->l_exbuy_qty);
							  fn_userlog(c_ServiceName,"Executed QTY IS :%ld:",ptr_st_pstn_actn->l_exec_qty);
              }

							fn_add_pos(ptr_st_pstn_actn->l_new_qty,
          							 ptr_st_pstn_actn->l_orgnl_rt,
          							 &(ptr_st_cntopt_pos->l_ibuy_qty),
          							 &(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );

              if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
              { 
							  fn_userlog(c_ServiceName,"After IWTL BUY QTY IS :%ld:",ptr_st_cntopt_pos->l_ibuy_qty);
							  fn_userlog(c_ServiceName,"After EXCHNG BUY QTY IS :%ld:",ptr_st_cntopt_pos->l_exbuy_qty);
              }
						}
						/***  Ver 1.7 Ends  ***/
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );

						if(ptr_st_cntopt_pos->l_isell_qty != ptr_st_cntopt_pos->l_exsell_qty)
						{
              if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
              {  
							  fn_userlog(c_ServiceName,"Inside Call Sell Position ");
						  	fn_userlog(c_ServiceName,"Before IWTL Sell QTY IS :%ld:",ptr_st_cntopt_pos->l_isell_qty);
						  	fn_userlog(c_ServiceName,"Before EXCHNG Sell QTY IS :%ld:",ptr_st_cntopt_pos->l_exsell_qty);
						  	fn_userlog(c_ServiceName,"Executed QTY IS :%ld:",ptr_st_pstn_actn->l_exec_qty);
              }
							fn_add_pos(ptr_st_pstn_actn->l_new_qty * (-1),
            							ptr_st_pstn_actn->l_orgnl_rt,
            							&(ptr_st_cntopt_pos->l_isell_qty),
            							&(ptr_st_cntopt_pos->d_isell_ord_vl ) );

              if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
              {
							  fn_userlog(c_ServiceName,"After IWTL SELL QTY IS :%ld:",ptr_st_cntopt_pos->l_isell_qty);
						  	fn_userlog(c_ServiceName,"After EXCHNG SELL QTY IS :%ld:",ptr_st_cntopt_pos->l_exsell_qty); 
              }
						}
			
					}
					break;

    case  ORS_CAN_ORD_RJCT:
					/** Increase the IWTL position by the unexecuted order qty  **/
					/** and order rate                                          **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_add_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );
					}
					else
					{
						fn_add_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );
					}
					break;

    case  ORS_TRD_CONF:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {

						fn_del_pos( ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );

						fn_del_pos( ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );


						if( ptr_st_cntopt_pos->l_opnpstn_qty > 0 )
            {
              ptr_st_pstn_actn->l_fresh_qty = ptr_st_pstn_actn->l_exec_qty;
              ptr_st_pstn_actn->l_cover_qty = 0;
            }
            else
            {
              ptr_st_pstn_actn->l_cover_qty =
                        fn_minl(ptr_st_pstn_actn->l_exec_qty,
                                labs(ptr_st_cntopt_pos->l_opnpstn_qty) );

              ptr_st_pstn_actn->l_fresh_qty =
                        fn_maxl(ptr_st_pstn_actn->l_exec_qty -
                                ptr_st_pstn_actn->l_cover_qty,
                                0 );
            }

						if ( ptr_st_pstn_actn->l_cover_qty > 0 )
						{
							fn_add_pos_in_val ( ptr_st_pstn_actn->l_cover_qty,
                               ptr_st_cntopt_pos->d_org_opn_val,
                               ptr_st_cntopt_pos->l_opnpstn_qty,
													     &(ptr_st_cntopt_pos->l_opnpstn_qty),
													     &(ptr_st_cntopt_pos->d_org_opn_val ) );
						}

						if ( ptr_st_pstn_actn->l_fresh_qty > 0 )
						{
							fn_add_pos( ptr_st_pstn_actn->l_fresh_qty,
													ptr_st_pstn_actn->l_exec_rt,
													&(ptr_st_cntopt_pos->l_opnpstn_qty),
													&(ptr_st_cntopt_pos->d_org_opn_val) );

						}
						ptr_st_cntopt_pos->l_buy_exctd_qty = 
																			ptr_st_cntopt_pos->l_buy_exctd_qty + 
																			ptr_st_pstn_actn->l_exec_qty;
					}
					else
					{

						fn_del_pos( ptr_st_pstn_actn->l_exec_qty * (-1),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );

						fn_del_pos( ptr_st_pstn_actn->l_exec_qty * (-1),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );

						if( ptr_st_cntopt_pos->l_opnpstn_qty < 0 )
            {
              ptr_st_pstn_actn->l_fresh_qty = ptr_st_pstn_actn->l_exec_qty;
              ptr_st_pstn_actn->l_cover_qty = 0;
            }
            else
            {
              ptr_st_pstn_actn->l_cover_qty =
                        fn_minl(ptr_st_pstn_actn->l_exec_qty,
                                labs(ptr_st_cntopt_pos->l_opnpstn_qty) );

              ptr_st_pstn_actn->l_fresh_qty =
                        fn_maxl(ptr_st_pstn_actn->l_exec_qty -
                                ptr_st_pstn_actn->l_cover_qty,
                                0 );
            }

						if ( ptr_st_pstn_actn->l_cover_qty > 0 )
						{
							fn_add_pos_in_val ( ptr_st_pstn_actn->l_cover_qty * ( -1 ),
                               ptr_st_cntopt_pos->d_org_opn_val,
                               ptr_st_cntopt_pos->l_opnpstn_qty,
													     &(ptr_st_cntopt_pos->l_opnpstn_qty),
													     &(ptr_st_cntopt_pos->d_org_opn_val ) );
						}

						if ( ptr_st_pstn_actn->l_fresh_qty > 0 )
						{
							fn_add_pos( ptr_st_pstn_actn->l_fresh_qty * (-1),
													ptr_st_pstn_actn->l_exec_rt,
													&(ptr_st_cntopt_pos->l_opnpstn_qty),
													&(ptr_st_cntopt_pos->d_org_opn_val ) );
						}

						ptr_st_cntopt_pos->l_sell_exctd_qty = 
																		ptr_st_cntopt_pos->l_sell_exctd_qty + 
																		(ptr_st_pstn_actn->l_exec_qty *(-1));
					}

					ptr_st_cntopt_pos->l_opnpstn_qty = 
                           ptr_st_cntopt_pos->l_buy_exctd_qty +
                           ptr_st_cntopt_pos->l_sell_exctd_qty;
					break;

    case  ORS_ORD_EXP:
					/** Decrease the IWTL,EXCH position by the unexecuted order qty  **/
					/** and order rate                                               **/
          if ( ptr_st_pstn_actn->c_trnsctn_flw == BUY )
          {
						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty - 
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_ibuy_qty),
												&(ptr_st_cntopt_pos->d_ibuy_ord_vl ) );

						fn_del_pos( ptr_st_pstn_actn->l_orgnl_qty -
												ptr_st_pstn_actn->l_exec_qty,
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exbuy_qty),
												&(ptr_st_cntopt_pos->d_exbuy_ord_vl ) );
					}
					else
					{
						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_isell_qty),
												&(ptr_st_cntopt_pos->d_isell_ord_vl ) );

						fn_del_pos( ( ( ptr_st_pstn_actn->l_orgnl_qty - 
										      ptr_st_pstn_actn->l_exec_qty) * (-1) ),
												ptr_st_pstn_actn->l_orgnl_rt,
												&(ptr_st_cntopt_pos->l_exsell_qty),
												&(ptr_st_cntopt_pos->d_exsell_ord_vl ) );
					}
					break;

    case  ORS_NEW_EXER_REQ:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
          {
						ptr_st_cntopt_pos->l_iexrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					break;

    case  ORS_NEW_EXER_ACPT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
          {
						ptr_st_cntopt_pos->l_exexrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_exdont_exrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					break;

    case  ORS_NEW_EXER_RJCT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
          {
						ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_new_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty -= ptr_st_pstn_actn->l_new_qty;
					}
					break;

    case  ORS_MOD_EXER_REQ:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
          {
						ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
						ptr_st_cntopt_pos->l_iexrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
						ptr_st_cntopt_pos->l_idont_exrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					break;

    case  ORS_MOD_EXER_ACPT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
          {
						ptr_st_cntopt_pos->l_exexrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
						ptr_st_cntopt_pos->l_exexrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_exdont_exrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
						ptr_st_cntopt_pos->l_exdont_exrc_qty += ptr_st_pstn_actn->l_new_qty;
					}
					break;

    case  ORS_MOD_EXER_RJCT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
          {
						ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_new_qty;
						ptr_st_cntopt_pos->l_iexrc_qty += ptr_st_pstn_actn->l_orgnl_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty -= ptr_st_pstn_actn->l_new_qty;
						ptr_st_cntopt_pos->l_idont_exrc_qty += ptr_st_pstn_actn->l_orgnl_qty;
					}
					break;

    case  ORS_CAN_EXER_REQ:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
          {
						ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
					}
					break;

    case  ORS_CAN_EXER_ACPT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
          {
						ptr_st_cntopt_pos->l_exexrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_exdont_exrc_qty -= ptr_st_pstn_actn->l_orgnl_qty;
					}
					break;

    case  ORS_CAN_EXER_RJCT:
          if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
          {
						ptr_st_cntopt_pos->l_iexrc_qty += ptr_st_pstn_actn->l_orgnl_qty;
					}
					else
					{
						ptr_st_cntopt_pos->l_idont_exrc_qty += ptr_st_pstn_actn->l_orgnl_qty;
					}
					break;

    case  EXER_CONF:
					ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_exec_qty;
					ptr_st_cntopt_pos->l_exexrc_qty -= ptr_st_pstn_actn->l_exec_qty;

					fn_del_pos_in_val ( ptr_st_pstn_actn->l_exec_qty,
                           ptr_st_cntopt_pos->d_org_opn_val,
                           ptr_st_cntopt_pos->l_opnpstn_qty,
											     &(ptr_st_cntopt_pos->l_opnpstn_qty),
											     &(ptr_st_cntopt_pos->d_org_opn_val ) );

					ptr_st_cntopt_pos->l_buy_exrc_qty += ptr_st_pstn_actn->l_exec_qty;
					ptr_st_cntopt_pos->l_buy_exctd_qty -= ptr_st_pstn_actn->l_exec_qty;

					/*** added on 18-May-2002 ****/
					ptr_st_cntopt_pos->l_exrc_qty += ptr_st_pstn_actn->l_exec_qty;
					break;

    case  EXER_RJCT:
					ptr_st_cntopt_pos->l_iexrc_qty -= ptr_st_pstn_actn->l_exec_qty;
					ptr_st_cntopt_pos->l_exexrc_qty -= ptr_st_pstn_actn->l_exec_qty;
					break;

    case  ASGMNT:
					fn_del_pos_in_val ( ptr_st_pstn_actn->l_exec_qty * -1,
                           ptr_st_cntopt_pos->d_org_opn_val,
                           ptr_st_cntopt_pos->l_opnpstn_qty,
										     	 &(ptr_st_cntopt_pos->l_opnpstn_qty),
											     &(ptr_st_cntopt_pos->d_org_opn_val ) );

					ptr_st_cntopt_pos->l_sell_asgn_qty += ptr_st_pstn_actn->l_exec_qty;
					ptr_st_cntopt_pos->l_sell_exctd_qty -= 
																		(ptr_st_pstn_actn->l_exec_qty * (-1) );

					/*** added on 18-May-2002 ****/
					ptr_st_cntopt_pos->l_asgnd_qty += ptr_st_pstn_actn->l_exec_qty;
					break;

    case  CLOSE_OUT_DEALS:
          /** Increase the OPEN position by the close out qty and  **/
          /** WEP rate.executed rate.                              **/

          ptr_st_pstn_actn->l_orgnl_qty =
                     ptr_st_cntopt_pos->l_opnpstn_qty * -1;
          ptr_st_pstn_actn->c_trnsctn_flw = ptr_st_cntopt_pos->c_opnpstn_flw;

          li_temp_qty = ptr_st_cntopt_pos->l_opnpstn_qty;
          li_flg_tmp_qty = ptr_st_cntopt_pos->l_opnpstn_qty;

          fn_add_pos_in_val(  li_temp_qty * (-1),
                              ptr_st_cntopt_pos->d_org_opn_val,
                              li_temp_qty,
                              &(ptr_st_cntopt_pos->l_opnpstn_qty),
                              &(ptr_st_cntopt_pos->d_org_opn_val ) );

          if ( li_flg_tmp_qty > 0 )
          {
            ptr_st_cntopt_pos->l_sell_exctd_qty = (-1) *
                                          ptr_st_cntopt_pos->l_buy_exctd_qty ;
          }
          else if ( li_flg_tmp_qty < 0 )
          {
            ptr_st_cntopt_pos->l_buy_exctd_qty = (-1) *
                                          ptr_st_cntopt_pos->l_sell_exctd_qty ;
          }

/*****************************************************************
          ptr_st_cntopt_pos->l_buy_exctd_qty = 0;
          ptr_st_cntopt_pos->l_sell_exctd_qty = 0;
*****************************************************************/
					break;
	}

  return SUCCESS;
}

/** Given a position and a type of action performed calculates the new **/
/** order margin to be                                                 **/
int fn_upd_mrgn_opt ( char *c_ServiceName,
                      struct vw_pstn_actn *ptr_st_pstn_actn,
                      struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                      struct vw_cntopt_pos *ptr_st_cntopt_pos,
                      struct vw_err_msg *ptr_st_err_msg)
{
	int i_returncode;
	int i_ip_len;
	int i_op_len;

	long int li_itm_prc;
	long int li_oom_prc;
	long int li_actual_mrgn;
	long int li_min_mrgn;
	long int li_ose_qty;
	long int li_ibm_qty;
	long int li_ibsm_qty;
	long int li_ebsm_qty;
	long int li_ebm_qty;
	long int li_obe_qty;
	long int li_ism_qty;
	long int li_esm_qty;
	long int li_cls_prc;
	long int li_stk_prc;
	long int li_cboq;

	long l_wincrement;
	long l_xincrement;

	double d_initial_mrgn = 0.0;
	double d_min_mrgn = 0.0;
  double d_min_trg_price=0.0; /*** Ver 2.6 ***/
	double d_somc  = 0.0;
	double d_i_exp = 0.0;
	double d_e_exp = 0.0;
	double d_mcboq = 0.0;

	double d_actual_mrgn = 0.0;
	double d_rls_amt = 0.0;
	double d_cur_trd_mrgn = 0.0;
  double d_trigger_prc;
  double d_t1;
  double d_t2;

/****  Margin Calc  ***/
	double d_c1 = 0.0;
	double d_c2 = 0.0;
	double d_c3 = 0.0;
	double d_c3a = 0.0;
	double d_c4 = 0.0;
	double d_c5 = 0.0;
	struct vw_contract st_cntrct;
	struct vw_cntrct_qt st_cntrct_qt;

	if( ptr_st_pstn_actn->l_actn_typ == CLOSE_OUT_DEALS )
	{
		ptr_st_cntopt_pos->d_trd_mrgn = 0;
  	return SUCCESS;
	}

	/** Get the margin percentages **/
	i_returncode = fn_get_mrgn_prntg_opt( c_ServiceName,
                                 ptr_st_pstn_actn,
                                 ptr_st_err_msg,
                                 &d_initial_mrgn,
																 &d_min_mrgn,
                                 &d_somc,
                                 &d_min_trg_price);          /*** Ver 2.6 ***/ 

	if ( i_returncode == -1 )
	{
		return ( -1 );
	}

	if ( ptr_st_pstn_actn->c_opt_typ == 'P' )
	{
		li_cls_prc = (long)(ptr_st_pstn_actn->d_und_quote*100.0);
		li_stk_prc = ptr_st_cntopt_pos->l_strike_prc;

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "Close Price :%ld:", li_cls_prc );
	fn_userlog( c_ServiceName, "Strike Price :%ld:", li_stk_prc );
}

		if ( li_cls_prc < li_stk_prc )
		{
			li_itm_prc = li_stk_prc - li_cls_prc;
			li_oom_prc = 0;
		}
		else
		{
			li_oom_prc = li_cls_prc - li_stk_prc;
			li_itm_prc = 0;
		}

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "ITM rate :%ld:", li_itm_prc );
	fn_userlog( c_ServiceName, "OOM rate :%ld:", li_oom_prc );
}

	/*********** Calculate order level exposure **************/

	if( ptr_st_pstn_actn->l_actn_typ == ORS_NEW_ORD_REQ 	|| 
			ptr_st_pstn_actn->l_actn_typ == SYSTEM_PLACE_ORDER	||
			ptr_st_pstn_actn->l_actn_typ == ORS_NEW_ORD_ACPT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_NEW_ORD_RJCT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_MOD_ORD_REQ		||
			ptr_st_pstn_actn->l_actn_typ == ORS_MOD_ORD_ACPT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_MOD_ORD_RJCT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_CAN_ORD_REQ		||
			ptr_st_pstn_actn->l_actn_typ == ORS_CAN_ORD_ACPT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_CAN_ORD_RJCT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF			||
			ptr_st_pstn_actn->l_actn_typ == ORS_NEW_EXER_REQ	||
			ptr_st_pstn_actn->l_actn_typ == ORS_NEW_EXER_ACPT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_NEW_EXER_RJCT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_MOD_EXER_REQ	||
			ptr_st_pstn_actn->l_actn_typ == ORS_MOD_EXER_ACPT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_MOD_EXER_RJCT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_CAN_EXER_REQ	||
			ptr_st_pstn_actn->l_actn_typ == ORS_CAN_EXER_ACPT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_CAN_EXER_RJCT	||
			ptr_st_pstn_actn->l_actn_typ == EXER_RJCT	        ||
			ptr_st_pstn_actn->l_actn_typ == CANCEL_AFTER_ASSIGNMENT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_ORD_EXP )
	{
		if ( ptr_st_cntopt_pos->l_ibuy_qty != 0 )
		{
			li_ose_qty = fn_maxl ( ( ( ptr_st_cntopt_pos->l_sell_exctd_qty * -1 ) -
	                             ptr_st_cntopt_pos->l_buy_exctd_qty             ),
	                           0 );
			/****************Commented for Version 1.1 new margin calculation **************
	
			li_ibm_qty = fn_maxl ( (ptr_st_cntopt_pos->l_ibuy_qty - li_ose_qty),
	                           0 );
	
			ptr_st_cntopt_pos->d_ibuy_mrgn = 
         ( (double)li_ibm_qty * ( ptr_st_cntopt_pos->d_ibuy_ord_vl /
	                                (double)ptr_st_cntopt_pos->l_ibuy_qty  ) );
			*******************************************************************************/
			
			/**1.1 **/
			li_cboq = fn_minl( (ptr_st_cntopt_pos->l_ibuy_qty),	li_ose_qty);

			if(ptr_st_cntopt_pos->l_opnpstn_qty < 0)
			{
				d_mcboq=(double)li_cboq*(ptr_st_cntopt_pos->d_trd_mrgn/(double)ptr_st_cntopt_pos->l_opnpstn_qty*-1);
			}
			else
			{
				d_mcboq = (double)li_cboq ;
			}
      ptr_st_cntopt_pos->d_ibuy_mrgn =	fn_maxd( (ptr_st_cntopt_pos->d_ibuy_ord_vl - d_mcboq) , 0 ) ;
			/**1.1 ends**/
  		/***********SS:Log to check the margin calculation values************/
      if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/ 
      {
  		fn_userlog ( c_ServiceName, "SANGEET BUY CALCULATION I START");
  		fn_userlog ( c_ServiceName, "l_sell_exctd_qty is       :%ld:",ptr_st_cntopt_pos->l_sell_exctd_qty);
  		fn_userlog ( c_ServiceName, "l_buy_exctd_qty 	is       :%ld:",ptr_st_cntopt_pos->l_buy_exctd_qty);
  		fn_userlog ( c_ServiceName, "Buy Order Quantity is     :%ld:",ptr_st_cntopt_pos->l_ibuy_qty);
  		fn_userlog ( c_ServiceName, "Open Position Quantity is :%ld:",ptr_st_cntopt_pos->l_opnpstn_qty);
  		fn_userlog ( c_ServiceName, "Open Position Margin is   :%lf:",ptr_st_cntopt_pos->d_trd_mrgn);
  		fn_userlog ( c_ServiceName, "The Buy Order Value is    :%lf:",ptr_st_cntopt_pos->d_ibuy_ord_vl);
  		fn_userlog ( c_ServiceName, "li_cboq  is   :%ld:",li_cboq);
  		fn_userlog ( c_ServiceName, "d_mcboq is   :%lf:",d_mcboq);
  		fn_userlog ( c_ServiceName, "d_ibuy_mrgn is   :%lf:",ptr_st_cntopt_pos->d_ibuy_mrgn);
  		fn_userlog ( c_ServiceName, "SANGEET BUY CALCULATION I ENDS");
      }
  		/***********SS:Log to check the margin calculation values*************/

		}
		else
		{
			ptr_st_cntopt_pos->d_ibuy_mrgn = 0;
		}

		if ( ptr_st_cntopt_pos->l_exbuy_qty != 0 )
		{
			li_ose_qty = fn_maxl ( ( ( ptr_st_cntopt_pos->l_sell_exctd_qty * -1 ) -
	                               ptr_st_cntopt_pos->l_buy_exctd_qty           ),
	                           0 );
			/****************Commented for Version 1.1 new margin calculation **************
	
			li_ebm_qty = fn_maxl( ( ptr_st_cntopt_pos->l_exbuy_qty - li_ose_qty ),
	                          0 );
	
			ptr_st_cntopt_pos->d_exbuy_mrgn = 
          ( (double)li_ebm_qty ) * ( ptr_st_cntopt_pos->d_exbuy_ord_vl /
	                                   (double)ptr_st_cntopt_pos->l_exbuy_qty );
			******************************************************************************/
			/**1.1**/
			li_cboq = fn_minl( (ptr_st_cntopt_pos->l_exbuy_qty),	li_ose_qty);

			if(ptr_st_cntopt_pos->l_opnpstn_qty < 0)
			{
				d_mcboq=(double)li_cboq*(ptr_st_cntopt_pos->d_trd_mrgn/(double)ptr_st_cntopt_pos->l_opnpstn_qty*-1);
			}
			else
			{
				d_mcboq = (double)li_cboq ;
			}
      ptr_st_cntopt_pos->d_exbuy_mrgn =	fn_maxd( (ptr_st_cntopt_pos->d_exbuy_ord_vl - d_mcboq), 0) ;
			/**1.1 ends**/
  		/***********SS:Log to check the margin calculation values************/
      if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
      {
  		fn_userlog ( c_ServiceName, "SANGEET BUY CALCULATION II START");
  		fn_userlog ( c_ServiceName, "l_sell_exctd_qty is       :%ld:",ptr_st_cntopt_pos->l_sell_exctd_qty);
  		fn_userlog ( c_ServiceName, "l_buy_exctd_qty 	is       :%ld:",ptr_st_cntopt_pos->l_buy_exctd_qty);
  		fn_userlog ( c_ServiceName, "Buy Order Quantity is     :%ld:",ptr_st_cntopt_pos->l_exbuy_qty);
  		fn_userlog ( c_ServiceName, "Open Position Quantity is :%ld:",ptr_st_cntopt_pos->l_opnpstn_qty);
  		fn_userlog ( c_ServiceName, "Open Position Margin is   :%lf:",ptr_st_cntopt_pos->d_trd_mrgn);
  		fn_userlog ( c_ServiceName, "li_cboq  is   :%ld:",li_cboq);
  		fn_userlog ( c_ServiceName, "d_mcboq is   :%lf:",d_mcboq);
  		fn_userlog ( c_ServiceName, "d_exbuy_mrgn is   :%lf:",ptr_st_cntopt_pos->d_exbuy_mrgn);
  		fn_userlog ( c_ServiceName, "SANGEET BUY CALCULATION II ENDS");
      }
  		/***********SS:Log to check the margin calculation values*************/

		}
		else
		{
			ptr_st_cntopt_pos->d_exbuy_mrgn = 0;
		}

		/** code added on 03-may-2002 **/
		if ( ptr_st_cntopt_pos->l_isell_qty != 0 )
		{
			li_obe_qty = fn_maxl ( ( ptr_st_cntopt_pos->l_buy_exctd_qty -
															 ptr_st_cntopt_pos->l_iexrc_qty     -
     	                         ( ptr_st_cntopt_pos->l_sell_exctd_qty * -1 ) ), 
                             0 );

			li_ism_qty = fn_maxl ( ( ( ptr_st_cntopt_pos->l_isell_qty * -1 ) - 
     	                         li_obe_qty                                ), 
                             0 );

  /***********SS:Log to check the margin calculation values************/
  if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
  {
  fn_userlog ( c_ServiceName, "SANGEET CALCULATION I START");
  fn_userlog ( c_ServiceName, "li_obe_qty is       :%ld:",li_obe_qty);
  fn_userlog ( c_ServiceName, "li_esm_qty is       :%ld:",li_esm_qty);
  }
  /***********SS:Log to check the margin calculation values*************/
			/*** Commented in Ver 3.1 ***
			d_c1 =
        ( (double)li_ism_qty            * 
          ptr_st_pstn_actn->d_und_quote * 100.0 *
          d_initial_mrgn / 100.0                   ) -
		    (double)( li_oom_prc * li_ism_qty          ) + 
        (double)( li_itm_prc * li_ism_qty          ); ***/

			d_c1 =
        ( (double)li_ism_qty            *
          ptr_st_pstn_actn->d_und_quote * 100.0 *
          d_initial_mrgn / 100.0                   ) -
         ((double) li_oom_prc  * (double) li_ism_qty )  +
         ((double) li_itm_prc * (double) li_ism_qty )  ;   /*** Added in Ver 3.1 ***/

			d_c2 =  ( (double)li_ism_qty * 
                (double)ptr_st_cntopt_pos->l_strike_prc * 
                d_somc / 100.0 );

			d_c3 = fn_maxd( d_c1, d_c2 );													/****SS:NSM*****/

/******************************************************************
			d_c4 = ( ( ptr_st_cntopt_pos->d_isell_ord_vl      /  
							   (double)ptr_st_cntopt_pos->l_isell_qty   ) *
							 (double)li_ism_qty                             );
******************************************************************/
			d_c4 = ptr_st_cntopt_pos->d_isell_ord_vl * -1;				/*****SS:Premiun Receivable****/

				/***1.1***/
			  /*d_c5 = d_c3 - d_c4;  */
				d_c5 = d_c3 ;	

			ptr_st_cntopt_pos->d_isell_mrgn = fn_maxd( d_c5, 0 );	  /*****SS:Sale Margin*****/

  /********Log to check the margin calculation values********/
  if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
  {
  fn_userlog ( c_ServiceName, "d_c1 is       :%lf:",d_c1);
  fn_userlog ( c_ServiceName, "d_c2 is       :%lf:",d_c2);
  fn_userlog ( c_ServiceName, "d_c3 is       :%lf:",d_c3);
  fn_userlog ( c_ServiceName, "d_c4 is       :%lf:",d_c4);
  fn_userlog ( c_ServiceName, "d_c5 is       :%lf:",d_c5);
  fn_userlog ( c_ServiceName, "d_isell_mrgn is  :%lf:",ptr_st_cntopt_pos->d_isell_mrgn);
  fn_userlog ( c_ServiceName, "SANGEET CALCULATION I ENDS");
  }
  /********Log to check the margin calculation values********/

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "Order Level");
	fn_userlog( c_ServiceName, "d_isell_ord_vl :%lf:",
																		ptr_st_cntopt_pos->d_isell_ord_vl);
	fn_userlog( c_ServiceName, "d_c1 :%lf:", d_c1); 
	fn_userlog( c_ServiceName, "d_c2 :%lf:", d_c2); 
	fn_userlog( c_ServiceName, "d_c3 :%lf:", d_c3); 
	fn_userlog( c_ServiceName, "d_c4 :%lf:", d_c4); 
	fn_userlog( c_ServiceName, "d_c5 :%lf:", d_c5); 
	fn_userlog( c_ServiceName, "d_isell_mrgn :%lf:", 
								ptr_st_cntopt_pos->d_isell_mrgn); 
}
		}
		else
		{
			ptr_st_cntopt_pos->d_isell_mrgn = 0.0;
		}

		/** code added on 03-may-2002 **/
		if ( ptr_st_cntopt_pos->l_exsell_qty != 0 )
		{
			li_obe_qty = fn_maxl ( ( ptr_st_cntopt_pos->l_buy_exctd_qty -
															 ptr_st_cntopt_pos->l_exexrc_qty    -
     	                         ( ptr_st_cntopt_pos->l_sell_exctd_qty * -1 ) ), 
                             0 );

			li_esm_qty = fn_maxl ( ( ( ptr_st_cntopt_pos->l_exsell_qty * -1 ) - 
     	                         li_obe_qty                                ), 
                             0 );
  /***********SS:Log to check the margin calculation values************/
  if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
  {
  fn_userlog ( c_ServiceName, "SANGEET CALCULATION II STARTS");
  fn_userlog ( c_ServiceName, "li_obe_qty is       :%ld:",li_obe_qty);
  fn_userlog ( c_ServiceName, "li_esm_qty is       :%ld:",li_esm_qty);
  }
  /***********SS:Log to check the margin calculation values*************/
			
			/*** Commented in Ver 3.1 ***
			d_c1 =
        ( (double)li_esm_qty            * 
          ptr_st_pstn_actn->d_und_quote *  100.0 *
          d_initial_mrgn / 100.0                   ) -
		    (double)( li_oom_prc * li_esm_qty          ) + 
        (double)( li_itm_prc * li_esm_qty          ); ***/

			d_c1 =
			( (double)li_esm_qty            *
				ptr_st_pstn_actn->d_und_quote *  100.0 *
				d_initial_mrgn / 100.0									) -
			 ((double) li_oom_prc * (double) li_esm_qty)				+
			 ((double) li_itm_prc * (double) li_esm_qty)				;		/*** Ver 3.1 ***/

			d_c2 =  ( (double)li_esm_qty * 
                (double)ptr_st_cntopt_pos->l_strike_prc * 
                d_somc / 100.0 );

			d_c3 = fn_maxd( d_c1, d_c2 );						/****SS:NSM*****/

/******************************************************************
			d_c4 = ( ( ptr_st_cntopt_pos->d_exsell_ord_vl    / 
							   (double)ptr_st_cntopt_pos->l_exsell_qty   ) *
							 (double)li_esm_qty                              );
******************************************************************/
			d_c4 = ptr_st_cntopt_pos->d_exsell_ord_vl  * -1;		/*****SS:Premiun Receivable****/

			/***1.1***/
			/*d_c5 = d_c3 - d_c4;  */
			d_c5 = d_c3 ;	

			ptr_st_cntopt_pos->d_exsell_mrgn = fn_maxd( d_c5, 0 );		/*****SS:Sale Margin*****/
  /********Log to check the margin calculation values********/
  if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
  {
  fn_userlog ( c_ServiceName, "d_c1 is       :%lf:",d_c1);
  fn_userlog ( c_ServiceName, "d_c2 is       :%lf:",d_c2);
  fn_userlog ( c_ServiceName, "d_c3 is       :%lf:",d_c3);
  fn_userlog ( c_ServiceName, "d_c4 is       :%lf:",d_c4);
  fn_userlog ( c_ServiceName, "d_c5 is       :%lf:",d_c5);
  fn_userlog ( c_ServiceName, "d_exsell_mrgn is  :%lf:",ptr_st_cntopt_pos->d_exsell_mrgn);
  fn_userlog ( c_ServiceName, "SANGEET CALCULATION II ENDS");
  }
  /********Log to check the margin calculation values********/

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "Accept Level");
	fn_userlog( c_ServiceName, "d_exsell_ord_vl :%lf:",
																		ptr_st_cntopt_pos->d_exsell_ord_vl);
	fn_userlog( c_ServiceName, "d_c1 :%lf:", d_c1); 
	fn_userlog( c_ServiceName, "d_c2 :%lf:", d_c2); 
	fn_userlog( c_ServiceName, "d_c3 :%lf:", d_c3); 
	fn_userlog( c_ServiceName, "d_c4 :%lf:", d_c4); 
	fn_userlog( c_ServiceName, "d_c5 :%lf:", d_c5); 
	fn_userlog( c_ServiceName, "d_exsell_mrgn :%lf:", 
								ptr_st_cntopt_pos->d_exsell_mrgn); 
}
		}
		else
		{
			ptr_st_cntopt_pos->d_exsell_mrgn = 0.0;
		}

		d_i_exp = fn_maxd( ptr_st_cntopt_pos->d_ibuy_mrgn,
                       ptr_st_cntopt_pos->d_exbuy_mrgn );

		d_e_exp = fn_maxd( ptr_st_cntopt_pos->d_isell_mrgn,
                       ptr_st_cntopt_pos->d_exsell_mrgn );

		ptr_st_cntopt_pos->d_ordr_mrgn = fn_maxd ( d_i_exp, d_e_exp );

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "d_ibuy_mrgn :%lf:", 
                                 ptr_st_cntopt_pos->d_ibuy_mrgn );
	fn_userlog( c_ServiceName, "d_exbuy_mrgn :%lf:", 
                                 ptr_st_cntopt_pos->d_exbuy_mrgn );
	fn_userlog( c_ServiceName, "d_isell_mrgn :%lf:", 
                                 ptr_st_cntopt_pos->d_isell_mrgn );
	fn_userlog( c_ServiceName, "d_exsell_mrgn :%lf:", 
                                 ptr_st_cntopt_pos->d_exsell_mrgn );
	fn_userlog( c_ServiceName, "d_ordr_mrgn :%lf:", 
                                 ptr_st_cntopt_pos->d_ordr_mrgn );
	fn_userlog( c_ServiceName, "d_ordr_mrgn -Cur :%lf:", 
                                 ptr_st_cntopt_pos_crrnt->d_ordr_mrgn );
}
		/*********** Calculate order level exposure end **************/
	}


		if ( ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF ||
				 ptr_st_pstn_actn->l_actn_typ == EXER_CONF ||
				 ptr_st_pstn_actn->l_actn_typ == INTRADAY_MTM ||
				 ptr_st_pstn_actn->l_actn_typ == MRGN_STMNT   ||				/*** Ver 1.5 ***/
         ptr_st_pstn_actn->l_actn_typ == EOD_MRGN_REP ||        /*** Ver 4.1 ***/    
				 ptr_st_pstn_actn->l_actn_typ == ASGMNT    		 )
		{
      if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
      {
			  fn_userlog(c_ServiceName,"Inside fn_upd_mrgn_opt Function");
      }

			if ( ptr_st_cntopt_pos->l_opnpstn_qty < 0 )
			{
					/*** Commented in Ver 3.1 ***
					d_c1 =
            ( ( (double)ptr_st_cntopt_pos->l_opnpstn_qty * -1.0 ) * 
              ptr_st_pstn_actn->d_und_quote * 100.0               * 
              d_initial_mrgn / 100.0                                       ) -
		        (double)( li_oom_prc * ptr_st_cntopt_pos->l_opnpstn_qty * -1 ) + 
            (double)( li_itm_prc * ptr_st_cntopt_pos->l_opnpstn_qty * -1 ); ***/

					d_c1 =
          ( ( (double)ptr_st_cntopt_pos->l_opnpstn_qty * -1.0 ) *
            ptr_st_pstn_actn->d_und_quote * 100.0               *
            d_initial_mrgn / 100.0                                       ) -
           ((double) li_oom_prc *  (double) ( ptr_st_cntopt_pos->l_opnpstn_qty * -1 )) +
           ((double) li_itm_prc *  (double) ( ptr_st_cntopt_pos->l_opnpstn_qty * -1 ));	/*** Ver 3.1 ***/

					d_c2 =  ( ( (double)ptr_st_cntopt_pos->l_opnpstn_qty * -1.0 ) * 
                    (double)ptr_st_cntopt_pos->l_strike_prc             * 
                    d_somc / 100.0                                        );

					ptr_st_cntopt_pos->d_trd_mrgn =  fn_maxd( d_c1, d_c2 );
        if(DEBUG_MSG_LVL_3) /*** ver 3.4 ***/
        {
				  fn_userlog(c_ServiceName,"Here d_c1 is :%lf:",d_c1 );
				  fn_userlog(c_ServiceName,"Here d_c2 is :%lf:",d_c2 );
				  fn_userlog(c_ServiceName,"Here Trade Margin is :%lf:", ptr_st_cntopt_pos->d_trd_mrgn);
        }
			}
			else
			{
					ptr_st_cntopt_pos->d_trd_mrgn = 0;
			}

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "Trade Level");
	fn_userlog( c_ServiceName, "d_trd_mrgn :%lf:",
																						ptr_st_cntopt_pos->d_trd_mrgn);
}
		}
		if ( ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF ||
				 ptr_st_pstn_actn->l_actn_typ == EXER_CONF 		||
					 ptr_st_pstn_actn->l_actn_typ == ASGMNT    			 )
		{
			d_trigger_prc = 0.0;

			if ( ptr_st_cntopt_pos->l_opnpstn_qty < 0 )
			{
				d_trigger_prc = ( ( (double) ptr_st_pstn_actn->l_strike_prc  - 
												  (ptr_st_cntopt_pos->d_trd_mrgn /
											    (ptr_st_cntopt_pos->l_opnpstn_qty * (-1) ))) /
											( 1 - (d_min_mrgn/100.0 ) )                 );

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "d_trigger_prc :%lf:", d_trigger_prc);
}
			}
			ptr_st_cntopt_pos->l_mtm_trg_prc = d_trigger_prc;
		}
	}
	else
	{
		li_cls_prc = (long)(ptr_st_pstn_actn->d_und_quote * 100.0 );
		li_stk_prc = ptr_st_cntopt_pos->l_strike_prc;

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "Close Price :%ld:", li_cls_prc );
	fn_userlog( c_ServiceName, "Strike Price :%ld:", li_stk_prc );
}

		if ( li_cls_prc > li_stk_prc )
		{
			li_itm_prc = li_cls_prc - li_stk_prc;
			li_oom_prc = 0;
		}
		else
		{
			li_oom_prc = li_stk_prc - li_cls_prc;
			li_itm_prc = 0;
		}

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "ITM rate :%ld:", li_itm_prc );
	fn_userlog( c_ServiceName, "OOM rate :%ld:", li_oom_prc );
}
	if( ptr_st_pstn_actn->l_actn_typ == ORS_NEW_ORD_REQ   ||
      ptr_st_pstn_actn->l_actn_typ == SYSTEM_PLACE_ORDER  ||
      ptr_st_pstn_actn->l_actn_typ == ORS_NEW_ORD_ACPT  ||
      ptr_st_pstn_actn->l_actn_typ == ORS_NEW_ORD_RJCT  ||
      ptr_st_pstn_actn->l_actn_typ == ORS_MOD_ORD_REQ   ||
      ptr_st_pstn_actn->l_actn_typ == ORS_MOD_ORD_ACPT  ||
      ptr_st_pstn_actn->l_actn_typ == ORS_MOD_ORD_RJCT  ||
      ptr_st_pstn_actn->l_actn_typ == ORS_CAN_ORD_REQ   ||
      ptr_st_pstn_actn->l_actn_typ == ORS_CAN_ORD_ACPT  ||
      ptr_st_pstn_actn->l_actn_typ == ORS_CAN_ORD_RJCT  ||
      ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF      ||
			ptr_st_pstn_actn->l_actn_typ == ORS_NEW_EXER_REQ	||
			ptr_st_pstn_actn->l_actn_typ == ORS_NEW_EXER_ACPT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_NEW_EXER_RJCT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_MOD_EXER_REQ	||
			ptr_st_pstn_actn->l_actn_typ == ORS_MOD_EXER_ACPT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_MOD_EXER_RJCT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_CAN_EXER_REQ	||
			ptr_st_pstn_actn->l_actn_typ == ORS_CAN_EXER_ACPT	||
			ptr_st_pstn_actn->l_actn_typ == ORS_CAN_EXER_RJCT	||
			ptr_st_pstn_actn->l_actn_typ == EXER_RJCT	        ||
      ptr_st_pstn_actn->l_actn_typ == CANCEL_AFTER_ASSIGNMENT ||
      ptr_st_pstn_actn->l_actn_typ == ORS_ORD_EXP )
  {
		if ( ptr_st_cntopt_pos->l_ibuy_qty != 0 )
		{
			li_ose_qty = fn_maxl ( ( ( ptr_st_cntopt_pos->l_sell_exctd_qty * -1 ) -
	                             ptr_st_cntopt_pos->l_buy_exctd_qty             ),
	                           0 );
			/*********************Commented for version 1.1 *************************	
			li_ibm_qty = fn_maxl ( (ptr_st_cntopt_pos->l_ibuy_qty - li_ose_qty),
	                           0 );
	
			ptr_st_cntopt_pos->d_ibuy_mrgn = 
         ( (double)li_ibm_qty * ( ptr_st_cntopt_pos->d_ibuy_ord_vl /
	                                (double)ptr_st_cntopt_pos->l_ibuy_qty  ) );
			*************************************************************************/
			/**1.1 **/
			li_cboq = fn_minl( (ptr_st_cntopt_pos->l_ibuy_qty),	li_ose_qty);

			if(ptr_st_cntopt_pos->l_opnpstn_qty < 0)
			{
				d_mcboq=(double)li_cboq*(ptr_st_cntopt_pos->d_trd_mrgn/(double)ptr_st_cntopt_pos->l_opnpstn_qty*-1);
			}
			else
			{
				d_mcboq = (double)li_cboq ;
			}
      ptr_st_cntopt_pos->d_ibuy_mrgn =	fn_maxd( (ptr_st_cntopt_pos->d_ibuy_ord_vl - d_mcboq) , 0) ;
			/**1.1 ends**/
  		/***********SS:Log to check the margin calculation values************/
      if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
      {
  		fn_userlog ( c_ServiceName, "SANGEET BUY CALCULATION III START");
  		fn_userlog ( c_ServiceName, "l_sell_exctd_qty is       :%ld:",ptr_st_cntopt_pos->l_sell_exctd_qty);
  		fn_userlog ( c_ServiceName, "l_buy_exctd_qty 	is       :%ld:",ptr_st_cntopt_pos->l_buy_exctd_qty);
  		fn_userlog ( c_ServiceName, "Buy Order Quantity is     :%ld:",ptr_st_cntopt_pos->l_ibuy_qty);
  		fn_userlog ( c_ServiceName, "Open Position Quantity is :%ld:",ptr_st_cntopt_pos->l_opnpstn_qty);
  		fn_userlog ( c_ServiceName, "Open Position Margin is   :%lf:",ptr_st_cntopt_pos->d_trd_mrgn);
  		fn_userlog ( c_ServiceName, "li_cboq  is   :%ld:",li_cboq);
  		fn_userlog ( c_ServiceName, "d_mcboq is   :%lf:",d_mcboq);
  		fn_userlog ( c_ServiceName, "d_ibuy_mrgn is   :%lf:",ptr_st_cntopt_pos->d_ibuy_mrgn);
  		fn_userlog ( c_ServiceName, "SANGEET BUY CALCULATION III ENDS");
      }
  		/***********SS:Log to check the margin calculation values*************/

		}
		else
		{
			ptr_st_cntopt_pos->d_ibuy_mrgn = 0;
		}

		if ( ptr_st_cntopt_pos->l_exbuy_qty != 0 )
		{
			li_ose_qty = fn_maxl ( ( ( ptr_st_cntopt_pos->l_sell_exctd_qty * -1 ) -
	                               ptr_st_cntopt_pos->l_buy_exctd_qty           ),
	                           0 );
			/******************Commented for 1.1***************************************
	
			li_ebm_qty = fn_maxl( ( ptr_st_cntopt_pos->l_exbuy_qty - li_ose_qty ),
	                          0 );
	
			ptr_st_cntopt_pos->d_exbuy_mrgn = 
          ( (double)li_ebm_qty ) * ( ptr_st_cntopt_pos->d_exbuy_ord_vl /
	                                   (double)ptr_st_cntopt_pos->l_exbuy_qty );
			***************************************************************************/
			/**1.1**/
			li_cboq = fn_minl( (ptr_st_cntopt_pos->l_exbuy_qty),	li_ose_qty);

			if(ptr_st_cntopt_pos->l_opnpstn_qty < 0)
			{
				d_mcboq=(double)li_cboq*(ptr_st_cntopt_pos->d_trd_mrgn/(double)ptr_st_cntopt_pos->l_opnpstn_qty*-1);
			}
			else
			{
				d_mcboq = (double)li_cboq ;
			}
      ptr_st_cntopt_pos->d_exbuy_mrgn =	fn_maxd( (ptr_st_cntopt_pos->d_exbuy_ord_vl - d_mcboq) , 0) ;
			/**1.1 ends**/
  		/***********SS:Log to check the margin calculation values************/
      if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
      {
  		fn_userlog ( c_ServiceName, "SANGEET BUY CALCULATION IV START");
  		fn_userlog ( c_ServiceName, "l_sell_exctd_qty is       :%ld:",ptr_st_cntopt_pos->l_sell_exctd_qty);
  		fn_userlog ( c_ServiceName, "l_buy_exctd_qty 	is       :%ld:",ptr_st_cntopt_pos->l_buy_exctd_qty);
  		fn_userlog ( c_ServiceName, "Buy Order Quantity is     :%ld:",ptr_st_cntopt_pos->l_exbuy_qty);
  		fn_userlog ( c_ServiceName, "Open Position Quantity is :%ld:",ptr_st_cntopt_pos->l_opnpstn_qty);
  		fn_userlog ( c_ServiceName, "Open Position Margin is   :%lf:",ptr_st_cntopt_pos->d_trd_mrgn);
  		fn_userlog ( c_ServiceName, "li_cboq  is   :%ld:",li_cboq);
  		fn_userlog ( c_ServiceName, "d_mcboq is   :%lf:",d_mcboq);
  		fn_userlog ( c_ServiceName, "d_exbuy_mrgn is   :%lf:",ptr_st_cntopt_pos->d_exbuy_mrgn);
  		fn_userlog ( c_ServiceName, "SANGEET BUY CALCULATION IV ENDS");
      }
  		/***********SS:Log to check the margin calculation values*************/

		}
		else
		{
			ptr_st_cntopt_pos->d_exbuy_mrgn = 0;
		}

		/** code added on 03-may-2002 **/
		if ( ptr_st_cntopt_pos->l_isell_qty != 0 )
		{
			li_obe_qty = fn_maxl ( ( ptr_st_cntopt_pos->l_buy_exctd_qty -
															 ptr_st_cntopt_pos->l_iexrc_qty     -
     	                         ( ptr_st_cntopt_pos->l_sell_exctd_qty * -1 )), 
                             0 );

			li_ism_qty = fn_maxl ( ( ( ptr_st_cntopt_pos->l_isell_qty * -1 ) - 
     	                         li_obe_qty                                ), 
                             0 );

  /***********SS:Log to check the margin calculation values************/
  if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
  {
  fn_userlog ( c_ServiceName, "SANGEET CALCULATION III STARTS");
  fn_userlog ( c_ServiceName, "li_obe_qty is       :%ld:",li_obe_qty);
  fn_userlog ( c_ServiceName, "li_esm_qty is       :%ld:",li_esm_qty);
  }
  /***********SS:Log to check the margin calculation values*************/

			/*** Commented in Ver 3.1 ***
			d_c1 =
        ( (double)li_ism_qty    * 
          ptr_st_pstn_actn->d_und_quote * 100.0 *
          d_initial_mrgn / 100.0                           ) -
		    (double)( li_oom_prc * li_ism_qty ) + 
        (double)( li_itm_prc * li_ism_qty ); ***/

			d_c1 =
        ( (double)li_ism_qty    *
          ptr_st_pstn_actn->d_und_quote * 100.0 *
          d_initial_mrgn / 100.0                           ) -
        ((double) li_oom_prc * (double) li_ism_qty ) +
        ((double) li_itm_prc * (double) li_ism_qty );		/*** Ver 3.1 ***/

			d_c2 =  ( (double)li_ism_qty * 
                (double)ptr_st_cntopt_pos->l_strike_prc * 
                d_somc / 100.0 );

			d_c3 = fn_maxd( d_c1, d_c2 );											/****SS:NSM*****/

/*****************************************************************
			d_c4 = ( ( ptr_st_cntopt_pos->d_isell_ord_vl      /  
							   (double)ptr_st_cntopt_pos->l_isell_qty   ) *
							 (double)li_ism_qty                             );
*****************************************************************/
			d_c4 = ptr_st_cntopt_pos->d_isell_ord_vl * -1 ;		/*****SS:Premiun Receivable****/

			/***1.1***/
			/*d_c5 = d_c3 - d_c4;  */
			d_c5 = d_c3 ;	

			ptr_st_cntopt_pos->d_isell_mrgn = fn_maxd( d_c5, 0 );	/*****SS:Sale Margin*****/
  /********Log to check the margin calculation values********/
  if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
  {
  fn_userlog ( c_ServiceName, "d_c1 is       :%lf:",d_c1);
  fn_userlog ( c_ServiceName, "d_c2 is       :%lf:",d_c2);
  fn_userlog ( c_ServiceName, "d_c3 is       :%lf:",d_c3);
  fn_userlog ( c_ServiceName, "d_c4 is       :%lf:",d_c4);
  fn_userlog ( c_ServiceName, "d_c5 is       :%lf:",d_c5);
  fn_userlog ( c_ServiceName, "d_isell_mrgn is  :%lf:",ptr_st_cntopt_pos->d_isell_mrgn);
  fn_userlog ( c_ServiceName, "SANGEET CALCULATION III ENDS");
  }
  /********Log to check the margin calculation values********/
		}
		else
		{
			ptr_st_cntopt_pos->d_isell_mrgn = 0.0;
		}

		/** code added on 03-may-2002 **/
		if ( ptr_st_cntopt_pos->l_exsell_qty != 0 )
		{
			li_obe_qty = fn_maxl ( ( ptr_st_cntopt_pos->l_buy_exctd_qty -
															 ptr_st_cntopt_pos->l_exexrc_qty    -
     	                         ( ptr_st_cntopt_pos->l_sell_exctd_qty * -1 )), 
                             0 );

			li_esm_qty = fn_maxl ( ( ( ptr_st_cntopt_pos->l_exsell_qty * -1 ) - 
     	                         li_obe_qty                                ), 
                             0 );

  /***********SS:Log to check the margin calculation values************/
  if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
  { 
  fn_userlog ( c_ServiceName, "SANGEET CALCULATION IV STARTS");
  fn_userlog ( c_ServiceName, "li_obe_qty is       :%ld:",li_obe_qty);
  fn_userlog ( c_ServiceName, "li_esm_qty is       :%ld:",li_esm_qty);
  }
  /***********SS:Log to check the margin calculation values*************/

			/*** Commented in Ver 3.1 ***
			d_c1 =
        ( (double)li_esm_qty * 
          ptr_st_pstn_actn->d_und_quote *  100.0 *
          d_initial_mrgn / 100.0                          ) -
		    (double)( li_oom_prc * li_esm_qty ) + 
        (double)( li_itm_prc * li_esm_qty ); ***/

			d_c1 =
        ( (double)li_esm_qty *
          ptr_st_pstn_actn->d_und_quote *  100.0 *
          d_initial_mrgn / 100.0                          ) -
        ((double) li_oom_prc * (double) li_esm_qty ) +
        ((double) li_itm_prc * (double) li_esm_qty );	/*** Ver 3.1 ***/

			d_c2 =  ( (double)li_esm_qty * 
                (double)ptr_st_cntopt_pos->l_strike_prc * 
                d_somc / 100.0 );

			d_c3 = fn_maxd( d_c1, d_c2 );					/****SS:NSM*****/

/*******************************************************************
			d_c4 = ( ( ptr_st_cntopt_pos->d_exsell_ord_vl    / 
							   (double)ptr_st_cntopt_pos->l_exsell_qty   ) *
							 (double)li_esm_qty                              );
*******************************************************************/
			d_c4 = ptr_st_cntopt_pos->d_exsell_ord_vl * -1;   /*****SS:Premiun Receivable****/

			/***1.1***/
			/*d_c5 = d_c3 - d_c4;  */
			d_c5 = d_c3 ;	

			ptr_st_cntopt_pos->d_exsell_mrgn = fn_maxd( d_c5, 0 );	/*****SS:Sale Margin*****/
  /********Log to check the margin calculation values********/
  if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
  { 
  fn_userlog ( c_ServiceName, "d_c1 is       :%lf:",d_c1);
  fn_userlog ( c_ServiceName, "d_c2 is       :%lf:",d_c2);
  fn_userlog ( c_ServiceName, "d_c3 is       :%lf:",d_c3);
  fn_userlog ( c_ServiceName, "d_c4 is       :%lf:",d_c4);
  fn_userlog ( c_ServiceName, "d_c5 is       :%lf:",d_c5);
  fn_userlog ( c_ServiceName, "d_exsell_mrgn is  :%lf:",ptr_st_cntopt_pos->d_exsell_mrgn);
  fn_userlog ( c_ServiceName, "SANGEET CALCULATION IV ENDS");
  }
  /********Log to check the margin calculation values********/
		}
		else
		{
			ptr_st_cntopt_pos->d_exsell_mrgn = 0.0;
		}

		d_i_exp = fn_maxd( ptr_st_cntopt_pos->d_ibuy_mrgn,
                       ptr_st_cntopt_pos->d_exbuy_mrgn );

		d_e_exp = fn_maxd( ptr_st_cntopt_pos->d_isell_mrgn,
                       ptr_st_cntopt_pos->d_exsell_mrgn );

		ptr_st_cntopt_pos->d_ordr_mrgn = fn_maxd ( d_i_exp, d_e_exp );

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "d_ibuy_mrgn :%lf:", 
                                 ptr_st_cntopt_pos->d_ibuy_mrgn );
	fn_userlog( c_ServiceName, "d_exbuy_mrgn :%lf:", 
                                 ptr_st_cntopt_pos->d_exbuy_mrgn );
	fn_userlog( c_ServiceName, "d_isell_mrgn :%lf:", 
                                 ptr_st_cntopt_pos->d_isell_mrgn );
	fn_userlog( c_ServiceName, "d_exsell_mrgn :%lf:", 
                                 ptr_st_cntopt_pos->d_exsell_mrgn );
	fn_userlog( c_ServiceName, "d_ordr_mrgn :%lf:", 
                                 ptr_st_cntopt_pos->d_ordr_mrgn );
}
	}	


		if ( ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF ||
				 ptr_st_pstn_actn->l_actn_typ == EXER_CONF ||
				 ptr_st_pstn_actn->l_actn_typ == INTRADAY_MTM ||
				 ptr_st_pstn_actn->l_actn_typ == MRGN_STMNT   ||        /*** Ver 1.5 ***/
         ptr_st_pstn_actn->l_actn_typ == EOD_MRGN_REP ||        /*** Ver 4.1 ***/
				 ptr_st_pstn_actn->l_actn_typ == ASGMNT    			 )
		{
      if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
      {
			  fn_userlog(c_ServiceName,"Inside fn_upd_mrgn_opt Function");
      }
			if ( ptr_st_cntopt_pos->l_opnpstn_qty < 0 )
			{
					/*** Commented in Ver 3.1 ***
					d_c1 =
           ( (double)(ptr_st_cntopt_pos->l_opnpstn_qty * -1.0) * 
             ptr_st_pstn_actn->d_und_quote *  100.0 *
             d_initial_mrgn / 100.0                              ) -
		       (double)( li_oom_prc * ptr_st_cntopt_pos->l_opnpstn_qty * -1.0 ) + 
           (double)( li_itm_prc * ptr_st_cntopt_pos->l_opnpstn_qty * -1.0 ); ***/

					d_c1 =
						( (double)(ptr_st_cntopt_pos->l_opnpstn_qty * -1.0) *
							ptr_st_pstn_actn->d_und_quote *  100.0 *
							d_initial_mrgn / 100.0                              ) -
						((double) li_oom_prc * (double) (ptr_st_cntopt_pos->l_opnpstn_qty * -1.0  )) +
						((double) li_itm_prc * (double) (ptr_st_cntopt_pos->l_opnpstn_qty * -1.0  ));	/*** Ver 3.1 ***/

					d_c2 =  ( (double)( ptr_st_cntopt_pos->l_opnpstn_qty  * -1.0 ) * 
                    (double)ptr_st_cntopt_pos->l_strike_prc * 
                    d_somc / 100.0                                    );
					ptr_st_cntopt_pos->d_trd_mrgn =  fn_maxd( d_c1, d_c2 );
        if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
        {
				  fn_userlog(c_ServiceName,"d_c1 is :%lf:",d_c1);
			    fn_userlog(c_ServiceName,"d_c2 is :%lf:",d_c2);
			  	fn_userlog(c_ServiceName,"Trade Margin Is :%lf:",ptr_st_cntopt_pos->d_trd_mrgn);
        }

			}
			else
			{
					ptr_st_cntopt_pos->d_trd_mrgn = 0; 
			}
		}

		if ( ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF ||
				 ptr_st_pstn_actn->l_actn_typ == EXER_CONF 		||
					 ptr_st_pstn_actn->l_actn_typ == ASGMNT    			 )
		{
			d_trigger_prc = 0.0;

			if ( ptr_st_cntopt_pos->l_opnpstn_qty < 0 )
			{
				d_trigger_prc = ( ( (double) ptr_st_pstn_actn->l_strike_prc  + 
													( ptr_st_cntopt_pos->d_trd_mrgn /
											  	(ptr_st_cntopt_pos->l_opnpstn_qty * (-1) ))) /
													( 1 + (d_min_mrgn/100.0 ) )                 );

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "d_trigger_prc :%lf:", d_trigger_prc);
}
			}
			ptr_st_cntopt_pos->l_mtm_trg_prc = d_trigger_prc ;
		}
	}

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "Inside Margin Calculation Function" );
	fn_userlog ( c_ServiceName, "Option Type 			 :%c:",
                              ptr_st_cntopt_pos->c_opt_typ );
	fn_userlog ( c_ServiceName, "IWTL buy qty      :%ld:",
                              ptr_st_cntopt_pos->l_ibuy_qty );
	fn_userlog ( c_ServiceName, "IWTL buy val      :%lf:",
                              ptr_st_cntopt_pos->d_ibuy_ord_vl );
	fn_userlog ( c_ServiceName, "IWTL sell qty     :%ld:",
                              ptr_st_cntopt_pos->l_isell_qty );
	fn_userlog ( c_ServiceName, "IWTL sell val     :%lf:",
                              ptr_st_cntopt_pos->d_isell_ord_vl );
	fn_userlog ( c_ServiceName, "EXCH buy qty      :%ld:",
                              ptr_st_cntopt_pos->l_exbuy_qty );
	fn_userlog ( c_ServiceName, "EXCH buy val      :%lf:",
                              ptr_st_cntopt_pos->d_exbuy_ord_vl );
	fn_userlog ( c_ServiceName, "EXCH sell qty     :%ld:",
                              ptr_st_cntopt_pos->l_exsell_qty );
	fn_userlog ( c_ServiceName, "EXCH sell val     :%lf:",
                              ptr_st_cntopt_pos->d_exsell_ord_vl );
	fn_userlog ( c_ServiceName, "Buy exctd qty     :%ld:", 
                              ptr_st_cntopt_pos->l_buy_exctd_qty );
	fn_userlog ( c_ServiceName, "Sell exctd qty    :%ld:", 
                              ptr_st_cntopt_pos->l_sell_exctd_qty );
	fn_userlog ( c_ServiceName, "Open qty          :%ld:", 
                              ptr_st_cntopt_pos->l_opnpstn_qty );
	fn_userlog ( c_ServiceName, "Order Margin      :%lf:", 
                              ptr_st_cntopt_pos->d_ordr_mrgn );

	fn_userlog( c_ServiceName, "Cover Qty :%ld:", 
                              ptr_st_pstn_actn->l_cover_qty );
	fn_userlog( c_ServiceName, "Fresh Qty :%ld:", 
                              ptr_st_pstn_actn->l_fresh_qty );
	fn_userlog( c_ServiceName, "Open Qty :%ld:", 
                              ptr_st_cntopt_pos->l_opnpstn_qty );
	fn_userlog( c_ServiceName, "Order Flow :%c:", 
                              ptr_st_pstn_actn->c_trnsctn_flw );
	fn_userlog( c_ServiceName, "Quote Price :%lf:", 
                              ptr_st_pstn_actn->d_und_quote);
	fn_userlog( c_ServiceName, "d_initial_mrgn :%lf:", d_initial_mrgn );
	fn_userlog( c_ServiceName, "d_somc :%lf:", d_somc );
	fn_userlog( c_ServiceName, "l_strike_prc :%ld:", 
                              ptr_st_cntopt_pos_crrnt->l_strike_prc );
	fn_userlog( c_ServiceName, "d_actual_mrgn :%lf:", d_actual_mrgn );
	fn_userlog( c_ServiceName, "d_min_mrgn :%lf:", d_min_mrgn );
	fn_userlog( c_ServiceName, "d_trd_mrgn :%lf:", 
                              ptr_st_cntopt_pos->d_trd_mrgn );
  fn_userlog( c_ServiceName, "d_trg_price :%lf:", d_min_trg_price );   /*** Ver 2.6 ***/	
	fn_userlog ( c_ServiceName, "Exiting Margin Calculation Function" );
}

  return SUCCESS;
}

int fn_call_actn_grp1_opt ( char *c_ServiceName,
                            struct vw_pstn_actn *ptr_st_pstn_actn,
                            struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                            struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                            double d_existing_dlvry_mrgn_val, /*** ver 3.8 ***/
                            struct vw_err_msg *ptr_st_err_msg )
{
  int i_returncode;

	double d_profit;
	double d_loss;
	double d_pl_amt;
	double d_diff_TLM;
	double d_diff_OLM;
	double d_balance_amt;
	double d_release_amt;

	long int li_sell_prc;
	long int li_buy_prc;
	long int li_itm_amt;
	long int li_oom_amt;
	long int li_actual_mrgn;
	long int li_min_mrgn;
	long int li_ose_qty;
	long int li_ibm_qty;
	long int li_ebm_qty;
	long int li_obe_qty;

	long int l_recv_buff_len;

	double d_initial_mrgn = 0.0;
	double d_somc  = 0.0;
	double d_i_exp = 0.0;
	double d_e_exp = 0.0;
  double d_min_trg_price= 0.0 ;   /*** 2.6 ***/	

	double d_premium = 0.0;
	double d_trigger_prc = 0.0;
	double d_actual_mrgn = 0.0;
	double d_rls_amt = 0.0;
	double d_min_mrgn = 0.0;
	double d_cur_trd_mrgn = 0.0;
	double d_curr_TLM = 0.0;
	double d_tobe_TLM = 0.0;
  char c_narration_id[4];
	char c_reason_cd [ 25 ];
  char c_dr_without_lmt_flg;
  char c_pos_avl;

  /*** ver 3.8 starts ***/
  double d_current_delvry_mrgn_prcnt = 0.0;
  double d_var_mrgn = 0.0;
  double d_extrm_lss_mrgn = 0.0;
  double d_ISec_buff_mrgn_prcnt = 0.0;
  double d_dlvry_min_mrgn_prcnt = 0.0;
  double d_new_delvry_mrgn_val = 0.0;
  double d_cntrct_val = 0.0;
  char   c_settlement_flag = '\0';
  /*** ver 3.8 ends ***/

  char  c_tmp_rmrks [ 133 ] ;
  EXEC SQL INCLUDE "table/iai_info_account_info.h"; /* Ver 3.3 */
  EXEC SQL BEGIN DECLARE SECTION;
    struct vw_cntopt_pos st_cntpos_crrnt;
    struct vw_cntopt_pos st_cntpos_old;
		long int li_und_close_prc;
  EXEC SQL END DECLARE SECTION;

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "Building new position" );
	fn_userlog ( c_ServiceName, "Current position : " );
	fn_userlog ( c_ServiceName, "Action type       :%ld:", 
                                        ptr_st_pstn_actn->l_actn_typ );
	fn_userlog ( c_ServiceName, "Match 	:%s:", 
            							     ptr_st_cntopt_pos_crrnt->c_cln_mtch_accnt );
	fn_userlog ( c_ServiceName, "IWTL buy qty      :%ld:", 
                               ptr_st_cntopt_pos_crrnt->l_ibuy_qty );
	fn_userlog ( c_ServiceName, "IWTL buy val      :%lf:", 
            							     ptr_st_cntopt_pos_crrnt->d_ibuy_ord_vl );
	fn_userlog ( c_ServiceName, "IWTL sell qty     :%ld:", 
            							     ptr_st_cntopt_pos_crrnt->l_isell_qty );
	fn_userlog ( c_ServiceName, "IWTL sell val     :%lf:", 
            							     ptr_st_cntopt_pos_crrnt->d_isell_ord_vl );
	fn_userlog ( c_ServiceName, "EXCH buy qty      :%ld:", 
            							    ptr_st_cntopt_pos_crrnt->l_exbuy_qty );
	fn_userlog ( c_ServiceName, "EXCH buy val      :%lf:", 
            							    ptr_st_cntopt_pos_crrnt->d_exbuy_ord_vl );
	fn_userlog ( c_ServiceName, "EXCH sell qty     :%ld:", 
            							    ptr_st_cntopt_pos_crrnt->l_exsell_qty );
	fn_userlog ( c_ServiceName, "EXCH sell val      :%lf:", 
            							    ptr_st_cntopt_pos_crrnt->d_exsell_ord_vl );
	fn_userlog ( c_ServiceName, "Buy exctd qty     :%ld:", 
            							    ptr_st_cntopt_pos_crrnt->l_buy_exctd_qty );
	fn_userlog ( c_ServiceName, "Sell exctd qty    :%ld:", 
            							    ptr_st_cntopt_pos_crrnt->l_sell_exctd_qty );
	fn_userlog ( c_ServiceName, "Open qty          :%ld:", 
            							    ptr_st_cntopt_pos_crrnt->l_opnpstn_qty );
	fn_userlog ( c_ServiceName, "Original open val :%lf:", 
            							    ptr_st_cntopt_pos_crrnt->d_org_opn_val );
	fn_userlog ( c_ServiceName, "Blocked Flag 		 :%lf:",										/*** Ver 2.4 ***/ 
            							    ptr_st_pstn_actn->c_nkd_blkd_flg);
} 
	
	/** New exercise request cannot be placed for the same contract     **/
	/** Only it can be modified. In short IWTL exercise qty cannot be   **/
	/** greater than zero for new order placement                       **/

/*******************
	if ( 	( ptr_st_pstn_actn->l_actn_typ == ORS_NEW_EXER_REQ ) &&
				( ptr_st_cntopt_pos_crrnt->l_iexrc_qty > 0         )   )
	{
		fn_errlog( c_ServiceName, "B28552", DEFMSG, ptr_st_err_msg->c_err_msg );
		return FAILURE;
	}
************/
	if ( ptr_st_pstn_actn->l_actn_typ == ORS_NEW_EXER_REQ )
	{
		if  ( (ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE) &&
					(ptr_st_cntopt_pos_crrnt->l_iexrc_qty > 0   )   )	
		{
			fn_errlog( c_ServiceName, "B28552", DEFMSG, ptr_st_err_msg->c_err_msg );
			return FAILURE;
		}
		else if  ( (ptr_st_pstn_actn->c_trnsctn_flw == DONT_EXERCISE) &&
							 (ptr_st_cntopt_pos_crrnt->l_idont_exrc_qty > 0   )   )	
		{
			fn_errlog( c_ServiceName, "B28552", DEFMSG, ptr_st_err_msg->c_err_msg );
			return FAILURE;
		}
	}
	

	if ( ( ptr_st_pstn_actn->l_actn_typ == EXER_CONF )  &&
			 ( ptr_st_cntopt_pos_crrnt->l_opnpstn_qty < 
																	ptr_st_pstn_actn->l_exec_qty) )
	{
		fn_userlog( c_ServiceName, "l_opnpstn_qty :%ld:",
                                     ptr_st_cntopt_pos_crrnt->l_opnpstn_qty );
		fn_userlog( c_ServiceName, "l_exec_qty :%ld:",
                                     ptr_st_pstn_actn->l_exec_qty );

		fn_errlog( c_ServiceName, "B28553", DEFMSG, ptr_st_err_msg->c_err_msg );
		return FAILURE;
	}

	if ( ( ptr_st_pstn_actn->l_actn_typ == ASGMNT )  &&
			 ( (ptr_st_cntopt_pos_crrnt->l_opnpstn_qty * (-1)) < 
																	ptr_st_pstn_actn->l_exec_qty) )
	{
		fn_userlog( c_ServiceName, "l_opnpstn_qty :%ld:",
                                     ptr_st_cntopt_pos_crrnt->l_opnpstn_qty );
		fn_userlog( c_ServiceName, "l_exec_qty :%ld:",
                                     ptr_st_pstn_actn->l_exec_qty );

		fn_errlog( c_ServiceName, "B28560", DEFMSG, ptr_st_err_msg->c_err_msg );
		return FAILURE;
	}

	if ( ptr_st_cntopt_pos_to_be->c_opt_typ == 'P' )
	{
		i_returncode =	fn_apply_cnt_actn_put ( c_ServiceName,
  																					ptr_st_pstn_actn, 
																						ptr_st_cntopt_pos_to_be,
   	                    										ptr_st_err_msg );
	}
	else
	{
		i_returncode =	fn_apply_cnt_actn_call( c_ServiceName,
  																					ptr_st_pstn_actn, 
																						ptr_st_cntopt_pos_to_be,
   	                    										ptr_st_err_msg );
	}

 	if ( i_returncode == -1 )
 	{
 		return FAILURE;
 	}

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "Calcualted position" );
	fn_userlog ( c_ServiceName, "Option Type 			 :%c:",
                              ptr_st_cntopt_pos_to_be->c_opt_typ );
	fn_userlog ( c_ServiceName, "IWTL buy qty      :%ld:",
                              ptr_st_cntopt_pos_to_be->l_ibuy_qty );
	fn_userlog ( c_ServiceName, "IWTL buy val      :%lf:",
                              ptr_st_cntopt_pos_to_be->d_ibuy_ord_vl );
	fn_userlog ( c_ServiceName, "IWTL sell qty     :%ld:",
                              ptr_st_cntopt_pos_to_be->l_isell_qty );
	fn_userlog ( c_ServiceName, "IWTL sell val     :%lf:",
                              ptr_st_cntopt_pos_to_be->d_isell_ord_vl );
	fn_userlog ( c_ServiceName, "IWTL exrc qty     :%ld:",
                              ptr_st_cntopt_pos_to_be->l_iexrc_qty );
	fn_userlog ( c_ServiceName, "EXCH buy qty      :%ld:",
                              ptr_st_cntopt_pos_to_be->l_exbuy_qty );
	fn_userlog ( c_ServiceName, "EXCH buy val      :%lf:",
                              ptr_st_cntopt_pos_to_be->d_exbuy_ord_vl );
	fn_userlog ( c_ServiceName, "EXCH sell qty     :%ld:",
                              ptr_st_cntopt_pos_to_be->l_exsell_qty );
	fn_userlog ( c_ServiceName, "EXCH sell val     :%lf:",
                              ptr_st_cntopt_pos_to_be->d_exsell_ord_vl );
}

	i_returncode =	fn_pos_validation( c_ServiceName,
  																		ptr_st_pstn_actn, 
																			ptr_st_cntopt_pos_to_be,
   	                    							ptr_st_err_msg );

  if ( i_returncode != 0 )
  {
  	return FAILURE;
  }

	if ( ptr_st_pstn_actn->c_trnsctn_flw == DONT_EXERCISE )
	{
		return SUCCESS;
	}

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "Calculating margin" );
} 
	/** Calculate the margin on the new position **/


	i_returncode = fn_upd_mrgn_opt ( c_ServiceName, 
                               ptr_st_pstn_actn, 
											         ptr_st_cntopt_pos_crrnt, 
											         ptr_st_cntopt_pos_to_be, 
                               ptr_st_err_msg );

  if ( i_returncode != 0 )
  {
  	return FAILURE;
  }

 	switch ( ptr_st_pstn_actn->l_actn_typ )
 	{
   	case  SYSTEM_PLACE_ORDER:
   				strcpy( c_narration_id , ON_ORDER_PLACEMENT);
    			c_dr_without_lmt_flg = DEBIT_TILL_LIMIT; 					/*1.1*/
         	break;

   	case  ORS_NEW_ORD_REQ:
   				strcpy( c_narration_id , ON_ORDER_PLACEMENT);

					if ( ptr_st_pstn_actn->c_nkd_blkd_flg == 'P') /*** Ver 2.4 -- If condition Added ***/
          {
    				c_dr_without_lmt_flg = DEBIT_TILL_LIMIT;
						if(DEBUG_MSG_LVL_3) /*** Ver 2.4 ***/
						{
  							fn_userlog ( c_ServiceName, " Inside DEBIT_TILL_LIMIT case for Corp actn" );
						}
 
					}
					else 
					{
    				c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
					} 
         	break;

   	case  ORS_NEW_ORD_ACPT:
   				strcpy( c_narration_id , ON_ORDER_ACCEPTANCE);
          if ( ptr_st_pstn_actn->c_mtm_flag == ORD_ACCPT_FOR_SYSTEM_SQ_OFF )
          {
            c_dr_without_lmt_flg = DEBIT_TILL_LIMIT;
          }
          else
          {
            c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT;
          }
         	break;

   	case  ORS_NEW_ORD_RJCT:
   				strcpy( c_narration_id , ON_ORDER_REJECTION);
    			c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT; 
         	break;

   	case  ORS_MOD_ORD_REQ:
   				strcpy( c_narration_id , ON_MODIFICATION_PLACEMENT);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
         	break;

   	case  ORS_MOD_ORD_ACPT:
   				strcpy( c_narration_id , ON_MODIFICATION_ACCEPTANCE);
     			c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT;
         	break;

   	case  ORS_MOD_ORD_RJCT:
   				strcpy( c_narration_id , ON_MODIFICATION_REJECTION);
     			c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT; 
         	break;

   	case  ORS_CAN_ORD_REQ:
   				strcpy( c_narration_id , ON_CANCELLATION_PLACEMENT);
    			c_dr_without_lmt_flg = DEBIT_TILL_LIMIT; 
         	break;

   	case  ORS_CAN_ORD_ACPT:
   				strcpy( c_narration_id , ON_CANCELLATION_ACCEPTANCE);
    			c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT; 
         	break;

   	case  ORS_CAN_ORD_RJCT:
   				strcpy( c_narration_id , ON_CANCELLATION_REJECT);
    			c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT; 
         	break;

   	case  ORS_TRD_CONF:
   				strcpy( c_narration_id , ON_TRADE_EXECUTION);
    			c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT; 
         	break;

   	case  ORS_ORD_EXP:
   				strcpy( c_narration_id , ON_EXPIRY);
    			c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT; 
         	break;

   	case  ORS_NEW_EXER_REQ:
   				strcpy( c_narration_id , ON_EXERCISE_PLACEMENT);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
         	break;

   	case  ORS_NEW_EXER_ACPT:
   				strcpy( c_narration_id , ON_EXERCISE_ACCEPTANCE);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
         	break;

   	case  ORS_NEW_EXER_RJCT:
   				strcpy( c_narration_id , ON_EXERCISE_REJECTION);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
         	break;

   	case  ORS_MOD_EXER_REQ:
   				strcpy( c_narration_id , ON_EXERCISE_MODIFICATION_PLACEMENT);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
         	break;

   	case  ORS_MOD_EXER_ACPT:
   				strcpy( c_narration_id , ON_EXERCISE_MODIFICATION_ACCEPTANCE);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
         	break;

   	case  ORS_MOD_EXER_RJCT:
   				strcpy( c_narration_id , ON_EXERCISE_MODIFICATION_REJECTION);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
         	break;

   	case  ORS_CAN_EXER_REQ:
   				strcpy( c_narration_id , ON_EXERCISE_CANCELLATION_PLACEMENT);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
         	break;

   	case  ORS_CAN_EXER_ACPT:
   				strcpy( c_narration_id , ON_EXERCISE_CANCELLATION_ACCEPTANCE);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
         	break;

   	case  ORS_CAN_EXER_RJCT:
   				strcpy( c_narration_id , ON_EXERCISE_CANCELLATION_REJECT);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
         	break;


   	case  ASGMNT:
   				strcpy( c_narration_id , MARGIN_ADJ_ON_ASGMNT);
    			c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT; 
					break;

   	case  EXER_CONF:
   				strcpy( c_narration_id , MARGIN_ADJ_ON_EXER_CONF);
    			c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT; 
					break;

   	case  EXER_RJCT:
   				strcpy( c_narration_id , ON_EXERCISE_EXPIRY);
    			c_dr_without_lmt_flg = DEBIT_WHEN_LIMIT; 
					break;

   	case  CLOSE_OUT_DEALS:
   				strcpy( c_narration_id , MARGIN_ADJ_ON_CLOSE_OUT);
    			c_dr_without_lmt_flg = DEBIT_WITHOUT_LIMIT; 
					break;

   	default:
        	strcpy( ptr_st_err_msg->c_err_msg, "Invalid action type");
 					fn_userlog ( c_ServiceName,"Invalid action type");
					return FAILURE;
         	break;
	}

	if( ( ptr_st_pstn_actn->l_actn_typ != ASGMNT          ) &&
			( ptr_st_pstn_actn->l_actn_typ != CLOSE_OUT_DEALS ) &&
			( ptr_st_pstn_actn->l_actn_typ != EXER_CONF       ) )
/***************************************************************
			( ptr_st_pstn_actn->l_actn_typ != EXER_RJCT       ) 	)
***************************************************************/
	{
		d_diff_OLM = ptr_st_cntopt_pos_crrnt->d_ordr_mrgn -
								 ptr_st_cntopt_pos_to_be->d_ordr_mrgn;

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "Current OLM :%lf:", 
                                     ptr_st_cntopt_pos_crrnt->d_ordr_mrgn );
	fn_userlog ( c_ServiceName, "To be OLM :%lf:", 
                                     ptr_st_cntopt_pos_to_be->d_ordr_mrgn );
	fn_userlog ( c_ServiceName, "Difference in OLM :%lf:", d_diff_OLM );
}

		if ( d_diff_OLM != 0 )
		{
			i_returncode = fn_upd_limits( c_ServiceName,
										 								ptr_st_pstn_actn,
                  	 								ptr_st_err_msg,
										 								c_narration_id,
										 								c_dr_without_lmt_flg,
										 								d_diff_OLM,
																		&d_balance_amt);
      if ( i_returncode != 0 )
      {
    		return i_returncode;
      }

			ptr_st_cntopt_pos_to_be->d_ordr_mrgn =
													ptr_st_cntopt_pos_to_be->d_ordr_mrgn +
													d_balance_amt ;
		}
	}

	if( ( ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF )	    ||
			( ptr_st_pstn_actn->l_actn_typ == CLOSE_OUT_DEALS  )	||
			( ptr_st_pstn_actn->l_actn_typ == ASGMNT 			 )	    ||
			( ptr_st_pstn_actn->l_actn_typ == EXER_CONF		 )			)	
  {	
		d_curr_TLM = ptr_st_cntopt_pos_crrnt->d_trd_mrgn ;

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "Current TLM :%lf:", d_curr_TLM );
}

		if ( d_curr_TLM != 0 )
		{
			i_returncode = fn_upd_limits( c_ServiceName,
	   	   	 						 							ptr_st_pstn_actn,
       	            	 							ptr_st_err_msg,
				  	    			 							RELEASE_OF_TRADE_MRGN,
				  		    		 							DEBIT_WITHOUT_LIMIT,
				  			    	 							d_curr_TLM,
		  						    							&d_balance_amt);
   		if ( i_returncode != 0 )
   		{
  			return i_returncode;
   		}
		}
	}

	if ( ( ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF ) &&
       ( ptr_st_pstn_actn->c_trnsctn_flw == SELL      )    )
	{
		d_premium = ( (double)ptr_st_pstn_actn->l_exec_qty * 
                  (double)ptr_st_pstn_actn->l_exec_rt     );

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName,"Premium :%lf:", d_premium );
}

		if ( d_premium != 0 )
		{
    	strcpy ( c_tmp_rmrks, ptr_st_pstn_actn->c_ref_rmrks );

    	/*sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c", commented in Ver 3.6 */
			sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c-", /* Ver 3.6 */
                ptr_st_pstn_actn->c_undrlyng,
                ptr_st_pstn_actn->c_expry_dt,
                ptr_st_pstn_actn->l_strike_prc/100,
                ptr_st_pstn_actn->c_opt_typ,
                ptr_st_pstn_actn->c_exrc_typ );
			strcat(ptr_st_pstn_actn->c_ref_rmrks,c_tmp_rmrks); /* Ver 3.6 */

      /***** Ver 4.4 Starts Here *****/
      MEMSET( sql_iai_type ) ;
      EXEC SQL
      SELECT  IAI_TYPE
      INTO    :sql_iai_type
      FROM    IAI_INFO_ACCOUNT_INFO
      WHERE   IAI_MATCH_ACCOUNT_NO  = :ptr_st_pstn_actn->c_cln_mtch_accnt ;

      if ( SQLCODE != 0 )
      {
          fn_errlog( c_ServiceName, "S31045", SQLMSG, ptr_st_err_msg->c_err_msg );
          return FAILURE ;
      }
      SETNULL(sql_iai_type);
      /***** ver 4.4 Ends Here ******/
      if(strcmp(sql_iai_type.arr,"NRO_NON_PINS") != 0)  /*** Ver 4.4 ***/
      { 
		  	i_returncode = fn_upd_limits( c_ServiceName,
	   		 						 								ptr_st_pstn_actn,
       	          	 								ptr_st_err_msg,
					    			 								PREMIUM_ADJUSTMENTS,
						    		 								DEBIT_WITHOUT_LIMIT,
							    	 								d_premium,
								    								&d_balance_amt);
   	 	 if ( i_returncode != 0 )
   		 {
   			return i_returncode;
   		 }
      }
      /****   Ver 1.3 Starts    *****/
      /**** if (  strncmp( ptr_st_pstn_actn->c_cln_mtch_accnt, "751", 3) == 0 )  Commented in Ver 3.3 *****/

      /**** Commented in Ver 4.4 ******************	
			****** Added in Ver 3.3 Starts *******
      MEMSET( sql_iai_type ) ;
      EXEC SQL
      SELECT  IAI_TYPE
      INTO    :sql_iai_type
      FROM    IAI_INFO_ACCOUNT_INFO
      WHERE   IAI_MATCH_ACCOUNT_NO  = :ptr_st_pstn_actn->c_cln_mtch_accnt ;

    	if ( SQLCODE != 0 )
    	{
      		fn_errlog( c_ServiceName, "S31050", SQLMSG, ptr_st_err_msg->c_err_msg );
					return FAILURE ;
			}
    	
			SETNULL(sql_iai_type);
      ****** Ver 4.4 ********/

			if(DEBUG_MSG_LVL_4)
			{
				fn_userlog(c_ServiceName,"CUST TYPE IS :%s:",sql_iai_type.arr);	
			}
			/****** Added Ver 3.3 Ends *****/

			if(strcmp(sql_iai_type.arr,"NRO_NON_PINS") == 0)   /****** Check Added in Ver 3.3 ******/
      {
        strcpy( c_reason_cd, "Premium Adjustments" );
        if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
        { 
          fn_userlog( c_ServiceName, "Calling fn_tds for :%s:",c_reason_cd );
        }

        i_returncode = fn_tds(  c_ServiceName,
                                ptr_st_pstn_actn,
                                ptr_st_err_msg,
                                TDS_NRI,
                                c_reason_cd,
                                DEBIT_WITHOUT_LIMIT,
                                d_premium,
                                &d_balance_amt );

        if ( i_returncode != 0 )
        {
          return i_returncode;
        }

       /**** Ver 4.4 Starts here ***/
       i_returncode = fn_upd_limits( c_ServiceName,
                                    ptr_st_pstn_actn,
                                    ptr_st_err_msg,
                                    PREMIUM_ADJUSTMENTS,
                                    DEBIT_WITHOUT_LIMIT,
                                    d_premium,
                                    &d_balance_amt);
       if ( i_returncode != 0 )
       {
        return i_returncode;
       }
       /***** Ver 4.4 *****/
      }
      /*****      Ver 1.3 Ends          ***********/

      strcpy ( ptr_st_pstn_actn->c_ref_rmrks,c_tmp_rmrks );
		}
	}

	if ( ( ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF ) &&
       ( ptr_st_pstn_actn->c_trnsctn_flw == BUY       )    )
	{
		d_premium = ( (double)ptr_st_pstn_actn->l_exec_qty * 
                  (double)ptr_st_pstn_actn->l_exec_rt     ) * -1.0;

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName,"Premium :%lf:", d_premium );
}

		if ( d_premium != 0 )
		{

      /******** Ver 4.4 starts here ********/
      MEMSET( sql_iai_type );
      EXEC SQL
      SELECT  IAI_TYPE
      INTO    :sql_iai_type
      FROM    IAI_INFO_ACCOUNT_INFO
      WHERE   IAI_MATCH_ACCOUNT_NO  = :ptr_st_pstn_actn->c_cln_mtch_accnt ;

      if ( SQLCODE != 0 )
      {
        fn_errlog( c_ServiceName, "S31055", SQLMSG, ptr_st_err_msg->c_err_msg );
        return FAILURE ;
      }
      SETNULL(sql_iai_type);
      /******* Ver 4.4 Ends here **********/
     if(strcmp(sql_iai_type.arr,"NRO_NON_PINS") != 0) /** ver 4.4 ****/
     {
    	strcpy ( c_tmp_rmrks, ptr_st_pstn_actn->c_ref_rmrks );

    	/*sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c", commented in Ver 3.6 */
			sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c-", /* Ver 3.6 */
                ptr_st_pstn_actn->c_undrlyng,
                ptr_st_pstn_actn->c_expry_dt,
                ptr_st_pstn_actn->l_strike_prc/100,
                ptr_st_pstn_actn->c_opt_typ,
                ptr_st_pstn_actn->c_exrc_typ );
			strcat( ptr_st_pstn_actn->c_ref_rmrks,c_tmp_rmrks); /* Ver 3.6 */
			i_returncode = fn_upd_limits( c_ServiceName,
	   		 						 								ptr_st_pstn_actn,
       	          	 								ptr_st_err_msg,
					    			 								PREMIUM_ADJUSTMENTS,
						    		 								DEBIT_WITHOUT_LIMIT,
							    	 								d_premium,
								    								&d_balance_amt);
   		if ( i_returncode != 0 )
   		{
   			return i_returncode;
   		}
     } /*** Ver 4.4 if ends *******/

      /****   Ver 1.3 Starts    *****/
      /*** if (  strncmp( ptr_st_pstn_actn->c_cln_mtch_accnt, "751", 3) == 0 )   Commented in Ver 3.3 ******/

    	/****** Added in Ver 3.3 *******/
      /**** Commented in ver 4.4 *******
      MEMSET( sql_iai_type );
      EXEC SQL
      SELECT  IAI_TYPE
      INTO    :sql_iai_type
      FROM    IAI_INFO_ACCOUNT_INFO
      WHERE   IAI_MATCH_ACCOUNT_NO  = :ptr_st_pstn_actn->c_cln_mtch_accnt ;

 	    if ( SQLCODE != 0 )
   	  {
        fn_errlog( c_ServiceName, "S31060", SQLMSG, ptr_st_err_msg->c_err_msg );
        return FAILURE ;
      }
    	SETNULL(sql_iai_type);
      **************** Ver 4.4 Ends Here *********/

      if(DEBUG_MSG_LVL_4)
      {
      	fn_userlog(c_ServiceName,"CUST TYPE 2 IS :%s:",sql_iai_type.arr);
			}
      /****** Added Ver 3.3 Ends *****/

      if(strcmp(sql_iai_type.arr,"NRO_NON_PINS") == 0)   /****** Check Added in Ver 3.3 ******/
     	{
        strcpy( c_reason_cd, "Premium Adjustments" );
        if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
        { 
          fn_userlog( c_ServiceName, "Calling fn_tds for :%s:",c_reason_cd );
        }
 
        i_returncode = fn_tds(  c_ServiceName,
                                ptr_st_pstn_actn,
                                ptr_st_err_msg,
                                TDS_NRI,
                                c_reason_cd,
                                DEBIT_WITHOUT_LIMIT,
                                d_premium,
                                &d_balance_amt );

        if ( i_returncode != 0 )
        {
          return i_returncode;
        }

        /***** Ver 4.4 Starst Here ********/
        strcpy ( c_tmp_rmrks, ptr_st_pstn_actn->c_ref_rmrks );

        /*sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c", commented in Ver 3.6 */
        sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c-", /* Ver 3.6 */
                ptr_st_pstn_actn->c_undrlyng,
                ptr_st_pstn_actn->c_expry_dt,
                ptr_st_pstn_actn->l_strike_prc/100,
                ptr_st_pstn_actn->c_opt_typ,
                ptr_st_pstn_actn->c_exrc_typ );
        strcat( ptr_st_pstn_actn->c_ref_rmrks,c_tmp_rmrks); /* Ver 3.6 */
        i_returncode = fn_upd_limits( c_ServiceName,
                                    ptr_st_pstn_actn,
                                    ptr_st_err_msg,
                                    PREMIUM_ADJUSTMENTS,
                                    DEBIT_WITHOUT_LIMIT,
                                    d_premium,
                                    &d_balance_amt);
        if ( i_returncode != 0 )
        {
          return i_returncode;
        }
        /**** Ver 4.4 Ends Here *******/

      }
      /*****      Ver 1.3 Ends          ***********/

      strcpy ( ptr_st_pstn_actn->c_ref_rmrks,c_tmp_rmrks );

		}
	}

	if( ( ptr_st_pstn_actn->l_actn_typ == ORS_TRD_CONF )	    ||
			( ptr_st_pstn_actn->l_actn_typ == CLOSE_OUT_DEALS  )	||
			( ptr_st_pstn_actn->l_actn_typ == ASGMNT 			 )	    ||
			( ptr_st_pstn_actn->l_actn_typ == EXER_CONF		 )			)	
  {	
		d_tobe_TLM = ptr_st_cntopt_pos_to_be->d_trd_mrgn * (-1);

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "To be TLM :%lf:", 
                                     ptr_st_cntopt_pos_to_be->d_trd_mrgn );
}

		if ( d_tobe_TLM != 0 )
		{
			i_returncode = fn_upd_limits( c_ServiceName,
	   	   	 						 							ptr_st_pstn_actn,
       	            	 							ptr_st_err_msg,
				  	    			 							c_narration_id,
				  		    		 							DEBIT_TILL_LIMIT,
				  			    	 							d_tobe_TLM,
		  						    							&d_balance_amt);
   		if ( i_returncode != 0 )
   		{
  			return i_returncode;
   		}

      /*** Added on 25-Apr-02 to take care of balance amount    ***/
      /***  in case of DEBIT_TILL_LIMIT. d_balance_amt is added ***/
      /*** since it is always negative or Zero                  ***/

			ptr_st_cntopt_pos_to_be->d_trd_mrgn =
												ptr_st_cntopt_pos_to_be->d_trd_mrgn +
                        d_balance_amt;

			/** Added for recalulation of Trigger price in case of ***/
			/** insufficient margin                                ***/

			/** Get the margin percentages **/
			i_returncode = fn_get_mrgn_prntg_opt( c_ServiceName,
                               							ptr_st_pstn_actn,
                               							ptr_st_err_msg,
                               							&d_initial_mrgn,
															 							&d_min_mrgn,
                               							&d_somc,
                                            &d_min_trg_price);          /*** 2.6 ***/ 

			if ( i_returncode == -1 )
			{
				return ( -1 );
			}

			d_trigger_prc = 0.0;

			if ( ptr_st_pstn_actn->c_opt_typ == 'P' )
			{
				d_trigger_prc = ( ( (double) ptr_st_pstn_actn->l_strike_prc  - 
											  	(ptr_st_cntopt_pos_to_be->d_trd_mrgn /
										    	(ptr_st_cntopt_pos_to_be->l_opnpstn_qty * (-1)))) /
													( 1 - (d_min_mrgn/100.0 ) )                 );

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "d_trigger_prc in fn_call_actn_grp1_opt :%lf:",d_trigger_prc);
}
				ptr_st_cntopt_pos_to_be->l_mtm_trg_prc = d_trigger_prc;
			}
			else
			{
				d_trigger_prc = ( ( (double) ptr_st_pstn_actn->l_strike_prc  + 
													( ptr_st_cntopt_pos_to_be->d_trd_mrgn /
										  		(ptr_st_cntopt_pos_to_be->l_opnpstn_qty * (-1)))) /
													( 1 + (d_min_mrgn/100.0 ) )                 );

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "d_trigger_prc in fn_call_actn_grp1_opt:%lf:", d_trigger_prc);
}
				ptr_st_cntopt_pos_to_be->l_mtm_trg_prc = d_trigger_prc ;
			}
		}
   /*** Ver 3.8 starts ***/

   if( ( ptr_st_pstn_actn->l_actn_typ == ASGMNT )||( ptr_st_pstn_actn->l_actn_typ == EXER_CONF) || ( ptr_st_pstn_actn->l_actn_typ == CLOSE_OUT_DEALS ) )
   {
   fn_userlog(c_ServiceName,"1.Testing c_xchng_cd :%s: c_expry_dt :%s: c_prd_typ :%c: c_undrlyng :%s: c_exrc_typ :%c: l_strike_prc :%ld: c_ctgry_indstk :%c: ",ptr_st_pstn_actn->c_xchng_cd,ptr_st_pstn_actn->c_expry_dt,ptr_st_pstn_actn->c_prd_typ,ptr_st_pstn_actn->c_undrlyng,ptr_st_pstn_actn->c_exrc_typ,ptr_st_pstn_actn->l_strike_prc,ptr_st_pstn_actn->c_ctgry_indstk);

  
  fn_userlog(c_ServiceName,"ptr_st_pstn_actn->c_opt_typ :%c: ",ptr_st_pstn_actn->c_opt_typ);

   EXEC SQL
        SELECT NVL(FTQ_SETLMNT_FLG,'C')
         INTO :c_settlement_flag
         FROM  FTQ_FO_TRD_QT 
         WHERE FTQ_XCHNG_CD       = :ptr_st_pstn_actn->c_xchng_cd
          AND FTQ_PRDCT_TYP       = :ptr_st_pstn_actn->c_prd_typ
          AND FTQ_UNDRLYNG        = :ptr_st_pstn_actn->c_undrlyng
          AND FTQ_EXPRY_DT        = to_date( :ptr_st_pstn_actn->c_expry_dt,'dd-mon-yyyy' )
          AND FTQ_EXER_TYP        = :ptr_st_pstn_actn->c_exrc_typ
          AND FTQ_STRK_PRC        = :ptr_st_pstn_actn->l_strike_prc
          AND FTQ_OPT_TYP         = :ptr_st_pstn_actn->c_opt_typ
          AND FTQ_INDSTK          = :ptr_st_pstn_actn->c_ctgry_indstk;

   if ( SQLCODE != 0 )
   {
      fn_errlog( c_ServiceName, "S31065", SQLMSG, ptr_st_err_msg->c_err_msg );
      return FAILURE;
   }

   fn_userlog(c_ServiceName,"nspan:exascls c_settlement_flag is :%c:",c_settlement_flag);  
   if( c_settlement_flag == 'P' )
   {
      sprintf( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%0.2lf-%c%c",ptr_st_pstn_actn->c_undrlyng,ptr_st_pstn_actn->c_expry_dt,(double)ptr_st_pstn_actn->l_strike_prc/100.00,ptr_st_pstn_actn->c_opt_typ,ptr_st_pstn_actn->c_exrc_typ);

     rtrim(ptr_st_pstn_actn->c_ref_rmrks);
     fn_userlog(c_ServiceName,"nspan:exascls remarks :%s:",ptr_st_pstn_actn->c_ref_rmrks);

   if(( ptr_st_pstn_actn->l_actn_typ == ASGMNT )||( ptr_st_pstn_actn->l_actn_typ == EXER_CONF))
   {
      /****** In case of assignment and exercise release the delivery margin and reblock *****/
      fn_userlog(c_ServiceName,"nspan:exascls asgn,exer release reblock mrgn :%lf:",d_existing_dlvry_mrgn_val);
  
      if( d_existing_dlvry_mrgn_val > 0 )
      {
          i_returncode = fn_upd_limits( c_ServiceName,
                                    ptr_st_pstn_actn,
                                    ptr_st_err_msg,
                                    RELEASE_ON_PHYDLVRY_OPT_EOS,
                                    DEBIT_WITHOUT_LIMIT,
                                    d_existing_dlvry_mrgn_val,
                                    &d_balance_amt);
         if ( i_returncode != 0 )
         {
             return i_returncode;
         }  
  
         i_returncode = fn_upd_limits( c_ServiceName,
                                    ptr_st_pstn_actn,
                                    ptr_st_err_msg,
                                    BLK_DLVRY_OBLIGATION,
                                    DEBIT_TILL_LIMIT,
                                    (-1)*d_existing_dlvry_mrgn_val,
                                    &d_balance_amt);
        if ( i_returncode != 0 )
        {
          return i_returncode;
        }
     } 
     
   }
   else if ( ptr_st_pstn_actn->l_actn_typ == CLOSE_OUT_DEALS )
   {
       fn_userlog(c_ServiceName,"nspan:exascls clsout release mrgn :%lf:",d_existing_dlvry_mrgn_val);
       /*** Non-SPAN closeout release the delivery margin ***/
       if( d_existing_dlvry_mrgn_val > 0 )
       {
          i_returncode = fn_upd_limits( c_ServiceName,
                                    ptr_st_pstn_actn,
                                    ptr_st_err_msg,
                                    RELEASE_ON_PHYDLVRY_OPT_EOS,
                                    DEBIT_WITHOUT_LIMIT,
                                    d_existing_dlvry_mrgn_val,
                                    &d_balance_amt);
          if ( i_returncode != 0 )
          {
             return i_returncode;
          }
        }
    }
   
      EXEC SQL
           UPDATE FOP_FO_OPTIONS_PSTN
              SET    FOP_UDLVRY_MRGN = 0
                WHERE FOP_CLM_MTCH_ACCNT  = :ptr_st_pstn_actn->c_cln_mtch_accnt
                  AND FOP_XCHNG_CD        = :ptr_st_pstn_actn->c_xchng_cd
                  AND FOP_PRDCT_TYP       = :ptr_st_pstn_actn->c_prd_typ
                  AND FOP_OPT_TYP         = :ptr_st_pstn_actn->c_opt_typ
                  AND FOP_EXER_TYP        = :ptr_st_pstn_actn->c_exrc_typ
                  AND FOP_UNDRLYNG        = :ptr_st_pstn_actn->c_undrlyng
                  AND FOP_EXPRY_DT        = to_date( :ptr_st_pstn_actn->c_expry_dt,'dd-mon-yyyy' )
                  AND FOP_STRK_PRC        = :ptr_st_pstn_actn->l_strike_prc;
   if ( SQLCODE != 0 )
   {
      fn_errlog( c_ServiceName, "S31070", SQLMSG, ptr_st_err_msg->c_err_msg );
      return FAILURE;
   }
   } 
  } 
   /*** Ver 3.8 ends ***/
	}

	if ( ptr_st_pstn_actn->l_actn_typ == EXER_CONF )
  {
		EXEC SQL
			SELECT  fum_cls_prc
			INTO		:li_und_close_prc
			FROM		fum_fo_undrlyng_mstr
			WHERE 	fum_xchng_cd = :ptr_st_pstn_actn->c_xchng_cd
			AND			fum_prdct_typ = :ptr_st_pstn_actn->c_prd_typ
			AND			fum_ctgry_indstk = :ptr_st_pstn_actn->c_ctgry_indstk
			AND			fum_undrlyng = :ptr_st_pstn_actn->c_undrlyng;
	
	  if ( SQLCODE != 0 )
	  {
	    fn_errlog( c_ServiceName, "S31075", SQLMSG, ptr_st_err_msg->c_err_msg );
	    return FAILURE;
	  }

	  if ( ptr_st_pstn_actn->c_opt_typ == 'P' )
  	{	
			d_profit = (double) ptr_st_pstn_actn->l_exec_qty * 
								 (double)	( ptr_st_pstn_actn->l_strike_prc - li_und_close_prc);
		}
		else
		{
			d_profit = (double) ptr_st_pstn_actn->l_exec_qty * 
								 (double)	( li_und_close_prc - ptr_st_pstn_actn->l_strike_prc );
		}

		if ( d_profit != 0 )
		{
			if ( ptr_st_pstn_actn->c_ctgry_indstk == 'S' )
 			{
				strcpy( c_narration_id, MATCH_STOCK_OPTION_EXERCISE );
			}
			else if (  ptr_st_pstn_actn->c_ctgry_indstk == 'I' )
			{
				strcpy( c_narration_id, MATCH_INDEX_OPTION_EXERCISE );
			}
			else
			{
				strcpy( ptr_st_err_msg->c_err_msg, "Invalid Category" );
    		fn_errlog ( c_ServiceName, "S31080", DEFMSG, 
                                            ptr_st_err_msg->c_err_msg );
				return FAILURE;
			}

      /****** Ver 4.4 Starst Here ***********/
      MEMSET( sql_iai_type ) ;
      EXEC SQL
      SELECT  IAI_TYPE
      INTO    :sql_iai_type
      FROM    IAI_INFO_ACCOUNT_INFO
      WHERE   IAI_MATCH_ACCOUNT_NO  = :ptr_st_pstn_actn->c_cln_mtch_accnt ;

      if ( SQLCODE != 0 )
      {
          fn_errlog( c_ServiceName, "S31085", SQLMSG, ptr_st_err_msg->c_err_msg );
          return FAILURE;
      }
      SETNULL(sql_iai_type);
      /**** Ver 4.4 Ends Here ****/

     if(strcmp(sql_iai_type.arr,"NRO_NON_PINS") != 0)  /*** ver 4.4, if added ***/
     {
    	strcpy ( c_tmp_rmrks, ptr_st_pstn_actn->c_ref_rmrks );

								sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c", 
                ptr_st_pstn_actn->c_undrlyng,
                ptr_st_pstn_actn->c_expry_dt,
                ptr_st_pstn_actn->l_strike_prc/100,
                ptr_st_pstn_actn->c_opt_typ,
                ptr_st_pstn_actn->c_exrc_typ );
			i_returncode = fn_upd_limits( c_ServiceName,
		 	  	   	 						 					ptr_st_pstn_actn,
   	 		   	            	 					ptr_st_err_msg,
						  	    			 					c_narration_id,
						  		    		 					DEBIT_WITHOUT_LIMIT,
						  			    	 					d_profit,
				  						    					&d_balance_amt);
  		if ( i_returncode != 0 )
   		{
  			return i_returncode;
   		}
     }  /*** Ver 4.4 if ends here ***/

      /****   Ver 1.3 Starts    *****/
      /** if (  strncmp( ptr_st_pstn_actn->c_cln_mtch_accnt, "751", 3) == 0 )  ** Commented in Ver 3.3 *****/

			/****** Added in Ver 3.3 Starts *******/
      /******* Commented in ver 4.4 ***************
      MEMSET( sql_iai_type ) ;
      EXEC SQL
      SELECT  IAI_TYPE
      INTO    :sql_iai_type
      FROM    IAI_INFO_ACCOUNT_INFO
      WHERE   IAI_MATCH_ACCOUNT_NO  = :ptr_st_pstn_actn->c_cln_mtch_accnt ;

    	if ( SQLCODE != 0 )
    	{
			    fn_errlog( c_ServiceName, "S31090", SQLMSG, ptr_st_err_msg->c_err_msg );
      		return FAILURE;	
			}
			SETNULL(sql_iai_type);
      ************** Ver 4.4 Ends Here **************/

			/****** Added Ver 3.3 Ends ******/
	
			if(strcmp(sql_iai_type.arr,"NRO_NON_PINS") == 0)   /****** Check Added in Ver 3.3 ******/
      {
        strcpy( c_reason_cd, "Profit on Exercise" );
        if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
        {
          fn_userlog( c_ServiceName, "Calling fn_tds for :%s:",c_reason_cd );
        }
  
        i_returncode = fn_tds(  c_ServiceName,
                                ptr_st_pstn_actn,
                                ptr_st_err_msg,
                                TDS_NRI,
                                c_reason_cd,
                                DEBIT_WITHOUT_LIMIT,
                                d_profit,
                                &d_balance_amt );

        if ( i_returncode != 0 )
        {
          return i_returncode;
        }

        /******** Ver 4.4 Starts Here *******/
        strcpy ( c_tmp_rmrks, ptr_st_pstn_actn->c_ref_rmrks );

                sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c",
                ptr_st_pstn_actn->c_undrlyng,
                ptr_st_pstn_actn->c_expry_dt,
                ptr_st_pstn_actn->l_strike_prc/100,
                ptr_st_pstn_actn->c_opt_typ,
                ptr_st_pstn_actn->c_exrc_typ );
        i_returncode = fn_upd_limits( c_ServiceName,
                                    ptr_st_pstn_actn,
                                    ptr_st_err_msg,
                                    c_narration_id,
                                    DEBIT_WITHOUT_LIMIT,
                                    d_profit,
                                    &d_balance_amt);
        if ( i_returncode != 0 )
        {
          return i_returncode;
        }
        /******** Ver 4.4 Ends Here ********/
      }
      /*****      Ver 1.3 Ends          ***********/

      strcpy ( ptr_st_pstn_actn->c_ref_rmrks,c_tmp_rmrks );
		}
	}
	
	if ( ptr_st_pstn_actn->l_actn_typ == ASGMNT )
  {

		EXEC SQL
			SELECT  fum_cls_prc
			INTO		:li_und_close_prc
			FROM		fum_fo_undrlyng_mstr
			WHERE 	fum_xchng_cd = :ptr_st_pstn_actn->c_xchng_cd
			AND			fum_prdct_typ = :ptr_st_pstn_actn->c_prd_typ
			AND			fum_ctgry_indstk = :ptr_st_pstn_actn->c_ctgry_indstk
			AND			fum_undrlyng = :ptr_st_pstn_actn->c_undrlyng;
	
	  if ( SQLCODE != 0 )
	  {
	    fn_errlog( c_ServiceName, "S31095", SQLMSG, ptr_st_err_msg->c_err_msg );
	    return FAILURE;
	  }

	  if ( ptr_st_pstn_actn->c_opt_typ == 'P' )
  	{	
			d_loss = (double) ptr_st_pstn_actn->l_exec_qty * 
							 (double) ( li_und_close_prc - ptr_st_pstn_actn->l_strike_prc );
		}
		else
		{
			d_loss = (double) ptr_st_pstn_actn->l_exec_qty * 
							 (double)	( ptr_st_pstn_actn->l_strike_prc - li_und_close_prc );
		}

		if ( d_loss != 0 )
		{
			if ( ptr_st_pstn_actn->c_ctgry_indstk == 'S' )
 			{
				strcpy( c_narration_id, MATCH_STOCK_OPTION_ASSIGNMENT );
			}
			else if (  ptr_st_pstn_actn->c_ctgry_indstk == 'I' )
			{
				strcpy( c_narration_id, MATCH_INDEX_OPTION_ASSIGNMENT );
			}
			else
			{
				strcpy( ptr_st_err_msg->c_err_msg, "Invalid Category" );
    		fn_errlog ( c_ServiceName, "S31100", DEFMSG, 
                                    ptr_st_err_msg->c_err_msg );
				return FAILURE;
			}

      /******* Ver 4.4 starts here *********/
      MEMSET(sql_iai_type);
      EXEC SQL
      SELECT  IAI_TYPE
      INTO    :sql_iai_type
      FROM    IAI_INFO_ACCOUNT_INFO
      WHERE   IAI_MATCH_ACCOUNT_NO  = :ptr_st_pstn_actn->c_cln_mtch_accnt ;

      if ( SQLCODE != 0 )
      {
         fn_errlog( c_ServiceName, "S31105", SQLMSG, ptr_st_err_msg->c_err_msg );
         return FAILURE;
      }
      SETNULL(sql_iai_type);
      /******* Ver 4.4 Ends Here **********/
     if(strcmp(sql_iai_type.arr,"NRO_NON_PINS") != 0)  /** ver 4.4 ***/
     { 
    	strcpy ( c_tmp_rmrks, ptr_st_pstn_actn->c_ref_rmrks );

    	sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c",
                ptr_st_pstn_actn->c_undrlyng,
                ptr_st_pstn_actn->c_expry_dt,
                ptr_st_pstn_actn->l_strike_prc/100,
                ptr_st_pstn_actn->c_opt_typ,
                ptr_st_pstn_actn->c_exrc_typ );

			i_returncode = fn_upd_limits( c_ServiceName,
		 	  	   	 						 					ptr_st_pstn_actn,
   	 		   	            	 					ptr_st_err_msg,
						  	    			 					c_narration_id,
						  		    		 					DEBIT_WITHOUT_LIMIT,
						  			    	 					d_loss,
				  						    					&d_balance_amt);
  		if ( i_returncode != 0 )
   		{
  			return i_returncode;
   		}
     }  /** Ver 4.4 , if ends ****/
      /****   Ver 1.3 Starts    *****/

 			/**** if ( strncmp( ptr_st_pstn_actn->c_cln_mtch_accnt, "751", 3) == 0 )    Commented in Ver 3.3 *****/

			/****** Added in Ver 3.3 *******/
      /********** Ver 4.4 Commented ***************
      MEMSET(sql_iai_type);
      EXEC SQL
      SELECT  IAI_TYPE
      INTO    :sql_iai_type
      FROM    IAI_INFO_ACCOUNT_INFO
      WHERE   IAI_MATCH_ACCOUNT_NO  = :ptr_st_pstn_actn->c_cln_mtch_accnt ;

    	if ( SQLCODE != 0 )
    	{
			   fn_errlog( c_ServiceName, "S31110", SQLMSG, ptr_st_err_msg->c_err_msg );
      	 return FAILURE;	
			}
    	SETNULL(sql_iai_type);
      *********** ver 4.4 end Here *********/

      if(DEBUG_MSG_LVL_4)
      {
	    	fn_userlog(c_ServiceName,"CUST TYPE 4 IS :%s:",sql_iai_type.arr);
			}
			 /********* Added in Ver 3.3 ** Ends ************/

			if(strcmp(sql_iai_type.arr,"NRO_NON_PINS") == 0)   /****** check Added in Ver 3.3 *******/
 	    {
        strcpy( c_reason_cd, "Loss on Assignment" );
        if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
        {
          fn_userlog( c_ServiceName, "Calling fn_tds for :%s:",c_reason_cd );
        }

        i_returncode = fn_tds(  c_ServiceName,
                                ptr_st_pstn_actn,
                                ptr_st_err_msg,
                                TDS_NRI,
                                c_reason_cd,
                                DEBIT_WITHOUT_LIMIT,
                                d_loss,
                                &d_balance_amt );

        if ( i_returncode != 0 )
        {
          return i_returncode;
        }
        
        /****** Ver 4.4 Starts here *********/

        strcpy ( c_tmp_rmrks, ptr_st_pstn_actn->c_ref_rmrks );

        sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c",
                ptr_st_pstn_actn->c_undrlyng,
                ptr_st_pstn_actn->c_expry_dt,
                ptr_st_pstn_actn->l_strike_prc/100,
                ptr_st_pstn_actn->c_opt_typ,
                ptr_st_pstn_actn->c_exrc_typ );

       i_returncode = fn_upd_limits( c_ServiceName,
                                    ptr_st_pstn_actn,
                                    ptr_st_err_msg,
                                    c_narration_id,
                                    DEBIT_WITHOUT_LIMIT,
                                    d_loss,
                                    &d_balance_amt);
       if ( i_returncode != 0 )
       {
         return i_returncode;
       }
       /**** Ver 4.4 Ends here ***/
      }
      /*****      Ver 1.3 Ends          ***********/

      strcpy ( ptr_st_pstn_actn->c_ref_rmrks,c_tmp_rmrks );
		}
	}

	return SUCCESS;
}

int fn_call_actn_add_mrgn( char *c_ServiceName,
                           struct vw_pstn_actn *ptr_st_pstn_actn,
                           struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                           struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                           struct vw_err_msg *ptr_st_err_msg,
													 char *c_imtm_addmrgn_flg)  /** Ver 2.6 **/
{
	 int i_returncode;
  double d_amount;
  double d_balance_amt;
  char c_narration_id[4];
	double d_initial_mrgn = 0.0;
	double d_min_mrgn = 0.0;
	double d_somc  = 0.0;
	double d_trigger_prc  = 0.0;
	char   c_tmp_rmrks [ 133 ] ;   /** Ver 1.8 **/ 

  if ( ptr_st_pstn_actn->l_actn_typ == ADD_MARGIN )
	{
    ptr_st_cntopt_pos_to_be->d_trd_mrgn =
                                ptr_st_cntopt_pos_to_be->d_trd_mrgn +
                                ptr_st_pstn_actn->d_add_mrgn_amt ;
		strcpy ( c_narration_id,ON_USER_ADD_MARGIN);
	}
	else
	{
		strcpy ( c_narration_id,ON_SYSTEM_ADD_MARGIN);
	}

  d_amount = (-1) * ptr_st_pstn_actn->d_add_mrgn_amt ;

if(DEBUG_MSG_LVL_3){
  fn_userlog ( c_ServiceName, "Add margin amount :%lf:", d_amount );
}

	/****************** Ver 1.8 Starts ************************************/

  if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
  {
	  fn_userlog ( c_ServiceName, "----------------Remark added----------------------" );
  }

	strcpy ( c_tmp_rmrks, ptr_st_pstn_actn->c_ref_rmrks );

      sprintf ( ptr_st_pstn_actn->c_ref_rmrks,"OPT-%s-%s-%ld-%c%c",
                ptr_st_pstn_actn->c_undrlyng,
                ptr_st_pstn_actn->c_expry_dt,
                ptr_st_pstn_actn->l_strike_prc/100,
                ptr_st_pstn_actn->c_opt_typ,
                ptr_st_pstn_actn->c_exrc_typ );	

  if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
  {
	  fn_userlog ( c_ServiceName, "Remark : %s:",ptr_st_pstn_actn->c_ref_rmrks );
  }

	/****************** Ver 1.8 Ends ************************************/

  if ( d_amount != 0 )
  {
     i_returncode = fn_upd_limits( c_ServiceName,
                                   ptr_st_pstn_actn,
                                   ptr_st_err_msg,
                                   c_narration_id,
                                   DEBIT_WHEN_LIMIT,
                                   d_amount,
                                   &d_balance_amt);
     if ( i_returncode != 0 )
     {
       switch ( i_returncode )
       {
         case INSUFFICIENT_LIMITS :
					 d_limit_OIMTM = d_balance_amt ; /* 1.2 */
					 if(DEBUG_MSG_LVL_3){
					 fn_userlog(c_ServiceName, "The limit returned is :%lf:",d_balance_amt);
					 }
    			/*** fn_errlog(c_ServiceName,"B28558",DEFMSG,ptr_st_err_msg->c_err_msg);Ver 2.1	***/
           return INSUFFICIENT_LIMITS;
           break;

         default :
           return FAILURE;
           break;
       }
     }

			/** Get the margin percentages **/
			i_returncode = fn_get_mrgn_prntg_opt( c_ServiceName,
                               							ptr_st_pstn_actn,
                               							ptr_st_err_msg,
                               							&d_initial_mrgn,
															 							&d_min_mrgn,
                               							&d_somc,
                                            &d_min_trg_price);          /*** 2.6 ***/ 

			if ( i_returncode == -1 )
			{
				return ( -1 );
			}

			d_trigger_prc = 0.0;

			if ( ptr_st_pstn_actn->c_opt_typ == 'P' )
			{
				d_trigger_prc = ( ( (double) ptr_st_pstn_actn->l_strike_prc  - 
											  	(ptr_st_cntopt_pos_to_be->d_trd_mrgn /
										    	(ptr_st_cntopt_pos_to_be->l_opnpstn_qty * (-1)))) /
													( 1 - (d_min_mrgn/100.0 ) )                 );

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "d_trigger_prc in add margin:%lf:",d_trigger_prc);
}
				ptr_st_cntopt_pos_to_be->l_mtm_trg_prc = d_trigger_prc;
			}
			else
			{
				d_trigger_prc = ( ( (double) ptr_st_pstn_actn->l_strike_prc  + 
													( ptr_st_cntopt_pos_to_be->d_trd_mrgn /
										  		(ptr_st_cntopt_pos_to_be->l_opnpstn_qty * (-1)))) /
													( 1 + (d_min_mrgn/100.0 ) )                 );

if(DEBUG_MSG_LVL_3){
	fn_userlog( c_ServiceName, "d_trigger_prc in addmargin:%lf:", d_trigger_prc);
}
				ptr_st_cntopt_pos_to_be->l_mtm_trg_prc = d_trigger_prc ;
			}

		*c_imtm_addmrgn_flg='Y'; /** Ver 2.6 **/
		strcpy ( ptr_st_pstn_actn->c_ref_rmrks,c_tmp_rmrks );  /** Ver 1.8 **/
   }
   return SUCCESS;
}

int fn_call_actn_imtm( char *c_ServiceName,
                       struct vw_pstn_actn *ptr_st_pstn_actn,
                       struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                       struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                       struct vw_err_msg *ptr_st_err_msg,
											 char   *c_imtm_addmrgn_flg )  /** Ver 2.6 **/
{

  double  d_loss_prcnt;
	double d_initial_mrgn = 0.0;
	double d_min_mrgn = 0.0;
	double d_somc  = 0.0;
  double d_addnl_mrgn_needed;
  double d_old_trade_mrgn;
  double d_trigger_prc;
  double d_min_trg_price= 0.0 ;   /*** 2.6 ***/

  int i_returncode;
  int i_ret_val;
  int i_trnsctn;

	char	c_imtm_rmrks [ 256 ] ;
	char	c_err_msg [ 256 ] ;

  TPTRANID tranid ;


	/** Get the margin percentages **/
	i_returncode = fn_get_mrgn_prntg_opt( 	c_ServiceName,
                                 					ptr_st_pstn_actn,
                                 					ptr_st_err_msg,
                                 					&d_initial_mrgn,
																 					&d_min_mrgn,
                                 					&d_somc,
                                          &d_min_trg_price);          /*** 2.6 ***/ 

	if ( i_returncode == -1 )
	{
		return ( -1 );
	}

	if ( ptr_st_cntopt_pos_to_be->c_opt_typ == 'P' )
	{
		d_trigger_prc = ( ( (double) ptr_st_pstn_actn->l_strike_prc  - 
												(ptr_st_cntopt_pos_to_be->d_trd_mrgn /
											  (ptr_st_cntopt_pos_to_be->l_opnpstn_qty * (-1) ))) /
											( 1 - (d_min_mrgn/100.0 ) )                 );

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "In PUT d_trigger_prc is %lf,d_und_quote is %lf ",
												d_trigger_prc,ptr_st_pstn_actn->d_und_quote * 100.0 );
} 

		ptr_st_cntopt_pos_to_be->l_mtm_trg_prc = d_trigger_prc;

    if ( ptr_st_pstn_actn->l_actn_typ != EOD_MRGN_REP ) /** If condition addded in Ver 4.3 **/
    {
  	  if ( ( ptr_st_pstn_actn->d_und_quote * 100.0 ) > d_trigger_prc )
  	  {
    	  return SUCCESS;
  	  }
    }
	}
	else
	{
		d_trigger_prc = ( ( (double) ptr_st_pstn_actn->l_strike_prc  + 
												(ptr_st_cntopt_pos_to_be->d_trd_mrgn /
											  (ptr_st_cntopt_pos_to_be->l_opnpstn_qty * (-1) ))) /
											( 1 + (d_min_mrgn/100.0 ) )                 );
if(DEBUG_MSG_LVL_3)
{
	fn_userlog ( c_ServiceName, "d_trigger_prc is %lf and d_und_quote is %lf ",
												d_trigger_prc,ptr_st_pstn_actn->d_und_quote * 100.0 );
} 

		ptr_st_cntopt_pos_to_be->l_mtm_trg_prc = d_trigger_prc;

    if ( ptr_st_pstn_actn->l_actn_typ != EOD_MRGN_REP ) /** If condition addded in Ver 4.2 **/
    {
  	  if ( ( ptr_st_pstn_actn->d_und_quote * 100.0 ) < d_trigger_prc )
  	  {
    	  return SUCCESS;
  	  }
    } /** Ver 4.2 **/
	}

if(DEBUG_MSG_LVL_3){
	fn_userlog ( c_ServiceName, "ReCalculating margin in MTM loop" );
} 

	d_old_trade_mrgn = ptr_st_cntopt_pos_to_be->d_trd_mrgn;

	i_returncode = fn_upd_mrgn_opt ( 	c_ServiceName, 
                               			ptr_st_pstn_actn, 
											         			ptr_st_cntopt_pos_crrnt, 
											         			ptr_st_cntopt_pos_to_be, 
                               			ptr_st_err_msg );

  if ( i_returncode != 0 )
  {
  	return FAILURE;
  }

  if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
  {
	  fn_userlog ( c_ServiceName, "After ReCalculating margin in MTM loop" );
  }

	d_trigger_priceMTM = d_trigger_prc; /* 1.2 */
	d_ord_mrgn_MTM_b = ptr_st_cntopt_pos_crrnt->d_ordr_mrgn  ;   /****1.2****/
	d_trd_mrgn_MTM_b = ptr_st_cntopt_pos_crrnt->d_trd_mrgn ;   /****1.2****/
	d_spread_mrgn_MTM_b = 0 ; /****1.2****/
	d_ord_mrgn_MTM_a = ptr_st_cntopt_pos_to_be->d_ordr_mrgn  ;   /****1.2****/
	d_trd_mrgn_MTM_a = ptr_st_cntopt_pos_to_be->d_trd_mrgn ;   /****1.2****/
	d_spread_mrgn_MTM_a = 0 ; /****1.2****/
	l_opnpstn_MTM = ptr_st_cntopt_pos_to_be->l_opnpstn_qty ;        /****1.2****/
	d_addnl_mrgn_needed = ptr_st_cntopt_pos_to_be->d_trd_mrgn -
												d_old_trade_mrgn ;

if(DEBUG_MSG_LVL_3){
  fn_userlog ( c_ServiceName,"Addn. Mrgn needed      :%lf:",
                                                      d_addnl_mrgn_needed);
}

	d_add_mrgn_MTM = d_addnl_mrgn_needed; /* 1.2 */

  ptr_st_pstn_actn->d_add_mrgn_amt = d_addnl_mrgn_needed ;
  /*** Ver 4.1 Starts ***/
  if ( ptr_st_pstn_actn->l_actn_typ == EOD_MRGN_REP )
  {

   fn_userlog ( c_ServiceName," Tanmay Insert into FMR_FNO_MRGN_REPORT table.");

    EXEC SQL
   Insert into FMR_FNO_MRGN_REPORT 
   (
    FMR_CLM_MTCH_ACCNT,
    FMR_XCHNG_CD,
    FMR_PRDCT_TYP,
    FMR_UNDRLYNG,
    FMR_EXPRY_DT,
    FMR_EXER_TYP,
    FMR_OPT_TYP,
    FMR_STRK_PRC,
    FMR_TYP,
    FMR_BLCK_MRGN,
    FMR_LOSS_VL,
    FMR_REQR_MRGN,
    FMR_TRD_DT
   )
   Values
   (
    :ptr_st_pstn_actn->c_cln_mtch_accnt,
    :ptr_st_pstn_actn->c_xchng_cd,
    :ptr_st_pstn_actn->c_prd_typ,
    :ptr_st_pstn_actn->c_undrlyng,
    :ptr_st_pstn_actn->c_expry_dt,
    :ptr_st_pstn_actn->c_exrc_typ,
    :ptr_st_pstn_actn->c_opt_typ,
    :ptr_st_pstn_actn->l_strike_prc,
    'O',
    :d_old_trade_mrgn,
    0,
    :ptr_st_cntopt_pos_to_be->d_trd_mrgn,
    SYSDATE
   );

   if ( SQLCODE != 0 )
   {
      fn_errlog( c_ServiceName, "S31115", SQLMSG, ptr_st_err_msg->c_err_msg );
      return FAILURE;
    }

  }  
 
  /*** Ver 4.1 Ends ***/

  if ( ptr_st_pstn_actn->l_actn_typ != EOD_MRGN_REP )  /** If condition added in ver 4.1 **/
  {
    i_returncode  = fn_call_actn_add_mrgn ( c_ServiceName,
                                            ptr_st_pstn_actn,
                                            ptr_st_cntopt_pos_crrnt,
                                            ptr_st_cntopt_pos_to_be,
                                            ptr_st_err_msg,
				  																	c_imtm_addmrgn_flg );  /** Ver 2.6 **/					

    if ( i_returncode != 0 )
    {
      switch ( i_returncode )
      {
        case INSUFFICIENT_LIMITS :
        sprintf(ptr_st_pstn_actn->c_imtm_rmrks,
        "INSUFFICIENT LIMIT FOR ADDMARGIN-[%lf]",d_addnl_mrgn_needed);

        if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
        { 
          fn_userlog ( c_ServiceName,"In.:%s:",ptr_st_pstn_actn->c_imtm_rmrks);
        }
        return INSUFFICIENT_LIMITS;
        break;

        default :
        return FAILURE;
        break;
      }
    }
    sprintf(c_imtm_rmrks,
            "ADDED MARGIN FOR POSITION - [%lf]",d_addnl_mrgn_needed);

    i_returncode = fn_ins_opt_rep_tbl ( c_ServiceName,
                                    		c_err_msg,
                                    		ptr_st_pstn_actn,
                                    		c_imtm_rmrks );
 
    if ( i_returncode != 0 )
    {
      fn_errlog( c_ServiceName, "S31120",LIBMSG,c_err_msg);
      return FAILURE;
    }
  } /** Ver 4.1 **/ 
  return SUCCESS;
}

int 	fn_pos_validation ( 	char *c_ServiceName,
														struct vw_pstn_actn *ptr_st_pstn_actn,
												 		struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                  					struct vw_err_msg *ptr_st_err_msg)
{
	char		c_nkd_call_allwd;
	char		c_nkd_put_allwd;

	/** Check for Trade permisssion for Naked call writing and  **/
	/** Naked Put writing                                       **/

 /** code changed here to allow client square off orders ***/
 	if ( ( ( ptr_st_pstn_actn->l_actn_typ == ORS_NEW_ORD_REQ ) ||
			   ( ptr_st_pstn_actn->l_actn_typ == ORS_MOD_ORD_REQ )  )  &&
       ( ptr_st_pstn_actn->c_nkd_blkd_flg != 'N'              )  &&
			 ( ptr_st_pstn_actn->c_trnsctn_flw == SELL )                 )
	{
		if ( ( labs(ptr_st_cntopt_pos_to_be->l_isell_qty) >
																	ptr_st_cntopt_pos_to_be->l_opnpstn_qty ) ||
		     ( labs(ptr_st_cntopt_pos_to_be->l_exsell_qty) >
																	ptr_st_cntopt_pos_to_be->l_opnpstn_qty ) )
		{
 			EXEC SQL
    		SELECT  flc_nkd_call_wrt_allwd,
								flc_nkd_put_wrt_allwd
    		INTO    :c_nkd_call_allwd,
								:c_nkd_put_allwd
    		FROM    flc_fo_lvl_chrctr_mstr
    		WHERE   flc_cln_lvl = ( SELECT  clm_clnt_lvl
    														FROM    clm_clnt_mstr
    														WHERE   clm_mtch_accnt = 
																			:ptr_st_pstn_actn->c_cln_mtch_accnt );

	  	if ( SQLCODE != 0 )
	  	{
	    	fn_errlog( c_ServiceName, "S31125", SQLMSG, ptr_st_err_msg->c_err_msg );
	    	return FAILURE;
	  	}

			if ( ( (c_nkd_call_allwd == 'N') && 
					 	( ptr_st_pstn_actn->c_opt_typ == 'C' ) ) ||
		     	( (c_nkd_put_allwd == 'N') && 
					 	( ptr_st_pstn_actn->c_opt_typ == 'P' ) ) )
			{
	    	fn_errlog( c_ServiceName, "B28557", DEFMSG, ptr_st_err_msg->c_err_msg );
	    	return FAILURE;
			}
		}
	}

	/** Exercise Qty cannot be greater than the Open position qty minus **/
	/** any initially placed exercise request qty                       **/
 	if ( ( ptr_st_pstn_actn->l_actn_typ == ORS_NEW_EXER_REQ ) ||
			 ( ptr_st_pstn_actn->l_actn_typ == ORS_MOD_EXER_REQ )  )
	{
		if ( ptr_st_cntopt_pos_to_be->l_opnpstn_qty > 0 )
		{
			if ( ptr_st_pstn_actn->c_trnsctn_flw == EXERCISE )
			{
				if ( ptr_st_cntopt_pos_to_be->l_iexrc_qty  > 
								 ( ptr_st_cntopt_pos_to_be->l_opnpstn_qty - 
									 ptr_st_cntopt_pos_to_be->l_idont_exrc_qty)	)
				{
					fn_errlog( c_ServiceName,"B28561",DEFMSG,ptr_st_err_msg->c_err_msg );
					return FAILURE;
				}
			}
			else if ( ptr_st_pstn_actn->c_trnsctn_flw == DONT_EXERCISE )
			{
				if ( ptr_st_cntopt_pos_to_be->l_idont_exrc_qty	> 
								 ( ptr_st_cntopt_pos_to_be->l_opnpstn_qty - 
									 ptr_st_cntopt_pos_to_be->l_iexrc_qty)	)
				{
					fn_errlog(c_ServiceName, "B28561",DEFMSG,ptr_st_err_msg->c_err_msg );
					return FAILURE;
				}
			}
			else
			{
	    	fn_errlog( c_ServiceName, "S31130", 
							"Invalid Exercise type", ptr_st_err_msg->c_err_msg );
	    	return FAILURE;
			}
		}
		else
		{
			fn_errlog( c_ServiceName, "B28553", DEFMSG, ptr_st_err_msg->c_err_msg );
			return FAILURE;
		}
	}


  return SUCCESS;
}

int fn_call_actn_can_aft_asgn( 	char *c_ServiceName,
                       					struct vw_pstn_actn *ptr_st_pstn_actn,
                       					struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                       					struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                       					struct vw_err_msg *ptr_st_err_msg)
{
  double d_addnl_mrgn_reqd;
  double d_diff_OLM;
  double d_OLM_to_blk;
  double d_balance_amt;
  double d_bal_amt;
  int i_returncode;
  char c_dr_without_lmt_flg;
  char c_narration_id[4];

	i_returncode = fn_upd_mrgn_opt ( 	c_ServiceName, 
                               			ptr_st_pstn_actn, 
											         			ptr_st_cntopt_pos_crrnt, 
											         			ptr_st_cntopt_pos_to_be, 
                               			ptr_st_err_msg );

  if ( i_returncode != 0 )
  {
  	return FAILURE;
  }

/************************************************
	d_diff_OLM = ptr_st_cntopt_pos_crrnt->d_ordr_mrgn -
				 			 ptr_st_cntopt_pos_to_be->d_ordr_mrgn;
************************************************/

	if(DEBUG_MSG_LVL_3){
		fn_userlog ( c_ServiceName, "Current OLM :%lf:", 
                                  ptr_st_cntopt_pos_crrnt->d_ordr_mrgn );
		fn_userlog ( c_ServiceName, "To be OLM :%lf:", 
                                  ptr_st_cntopt_pos_to_be->d_ordr_mrgn );
	}

	if ( ptr_st_cntopt_pos_crrnt->d_ordr_mrgn != 0 )
	{
		/** Always a credit operation so ignore DEBIT_WITHOUT_LIMIT here **/
		i_returncode = fn_upd_limits( c_ServiceName,
						 											ptr_st_pstn_actn,
           	 											ptr_st_err_msg,
						 											RELEASE_OF_ADD_MARGIN,
							 										DEBIT_WITHOUT_LIMIT,
						 											ptr_st_cntopt_pos_crrnt->d_ordr_mrgn,
																	&d_balance_amt);
		if ( i_returncode != 0 )
 		{
 			return FAILURE;
 		}
	}

	d_OLM_to_blk = ptr_st_cntopt_pos_to_be->d_ordr_mrgn * (-1.0);
	if(DEBUG_MSG_LVL_3){
		fn_userlog ( c_ServiceName, "d_OLM_to_blk :%lf:", d_OLM_to_blk );
	}

	d_balance_amt = 0;

	if ( d_OLM_to_blk != 0 )
	{
		i_returncode = fn_upd_limits( c_ServiceName,
						 											ptr_st_pstn_actn,
           	 											ptr_st_err_msg,
						 											ON_SYSTEM_ADD_MARGIN,
						 											DEBIT_TILL_LIMIT,
						 											d_OLM_to_blk,
																	&d_balance_amt);

		if ( i_returncode != 0 )
 		{
 			return FAILURE;
 		}

	if(DEBUG_MSG_LVL_3){
		fn_userlog ( c_ServiceName, "Balance amount for calling DEBIT_WITHOUT_LIMIT :%lf:", d_balance_amt );
	}

		if ( d_balance_amt != 0 )
		{
			i_returncode = fn_upd_limits( c_ServiceName,
							 											ptr_st_pstn_actn,
             	 											ptr_st_err_msg,
							 											ON_SYSTEM_ADD_MARGIN,
							 											DEBIT_WITHOUT_LIMIT,
							 											d_balance_amt,
																		&d_bal_amt);

			if ( i_returncode != 0 )
 			{
 				return FAILURE;
 			}

    	return INSUFFICIENT_LIMITS;
		}

	}

  return SUCCESS;
}

int fn_nearest_paise_uop(  char		*c_ServiceName,
                       double	d_rate, 
                       int 		tick, 
	 										 int 		up_down,
											 char 	*c_err_msg,
                       double *d_amount )
{

/************************************************************
up_down =1 for lower bound and 2 for upper bound
tick will be the nearest paise
************************************************************/

	long	l_rate ;

	d_rate =  floor(d_rate);
	l_rate = (long) d_rate ;

	if(up_down == LOWER)
	{
		 d_rate = (double)(l_rate  - (  l_rate %  tick ) ) ;
		 *d_amount = d_rate;
	}
	else if(up_down == UPPER)
	{
		 if ( (  l_rate %  tick ) != 0)
		 {
	 		 d_rate = (double)(l_rate  - (  l_rate %  tick ) + tick ) ;
		 }
		 *d_amount = d_rate;
	}
	else
	{
		strcpy( c_err_msg, "Invalid up_down of bound" );
	 	fn_errlog( c_ServiceName, "S31135", DEFMSG, c_err_msg );
		return FAILURE;
	}

	return SUCCESS;
}

/**************  Ver 1.5 Starts  *******************/
int fn_call_mrgn_cltn( char *c_ServiceName,
                       struct vw_pstn_actn *ptr_st_pstn_actn,
                       struct vw_cntopt_pos *ptr_st_cntopt_pos_crrnt,
                       struct vw_cntopt_pos *ptr_st_cntopt_pos_to_be,
                       struct vw_err_msg *ptr_st_err_msg)
{
	double d_loss_prcnt;
  double d_initial_mrgn = 0.0;
  double d_min_mrgn = 0.0;
  double d_somc  = 0.0;
  double d_addnl_mrgn_needed;
  double d_old_trade_mrgn;
  double d_trigger_prc;
	double d_fop_trd_mrgn;
	double d_exstng_trd_mrgn; 	/*** Ver 1.6 ***/
  double d_min_trg_price=0.0; /*** 2.6 ***/

  int i_returncode;
  int i_ret_val;
  int i_trnsctn;

  char  c_imtm_rmrks [ 256 ] ;
  char  c_err_msg [ 256 ] ;

	
	/** Get the margin percentages **/
  if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
  {
	  fn_userlog(c_ServiceName,"Inside Function fn_call_mrgn_cltn For Margin statement");
  }

  i_returncode = fn_get_mrgn_prntg_opt(   c_ServiceName,
                                          ptr_st_pstn_actn,
                                          ptr_st_err_msg,
                                          &d_initial_mrgn,
                                          &d_min_mrgn,
                                          &d_somc,
                                          &d_min_trg_price);          /*** 2.6 ***/ 

  if ( i_returncode == -1 )
  {
    return ( -1 );
  }

	EXEC	SQL
		SELECT	fop_uexctd_mrgn
		INTO		:d_fop_trd_mrgn
		FROM		fop_fo_options_pstn
		WHERE		fop_clm_mtch_accnt  = :ptr_st_pstn_actn->c_cln_mtch_accnt
		AND			fop_prdct_typ       = :ptr_st_pstn_actn->c_prd_typ
		AND   	fop_opt_typ         = :ptr_st_pstn_actn->c_opt_typ
    AND   	fop_exer_typ        = :ptr_st_pstn_actn->c_exrc_typ
    AND   	fop_undrlyng        = :ptr_st_pstn_actn->c_undrlyng
    AND   	fop_expry_dt        = :ptr_st_pstn_actn->c_expry_dt
    AND   	fop_strk_prc        = :ptr_st_pstn_actn->l_strike_prc
    AND     fop_xchng_cd        = :ptr_st_pstn_actn->c_xchng_cd;  /** Ver 2.0 ***/

  if ( ( SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
  {
    fn_errlog ( c_ServiceName, "S31140", SQLMSG, ptr_st_err_msg->c_err_msg );
    /*** tpreturn ( TPFAIL, ERR_BFR, ( char * )ptr_st_err_msg, 0, 0 ); commented in Ver 3.5 ***/
		return ( FAILURE ); /* Ver 3.5 */
  }

	d_exstng_trd_mrgn = d_fop_trd_mrgn;	/*** Ver 1.6 ***/

	if ( ptr_st_cntopt_pos_to_be->c_opt_typ == 'P' )
  {
    if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
    {
		  fn_userlog(c_ServiceName,"Inside Trigger Price Calculation For Put Option ");
    }

    d_trigger_prc = ( ( (double) ptr_st_pstn_actn->l_strike_prc  -
                        (ptr_st_cntopt_pos_to_be->d_trd_mrgn /
                        (ptr_st_cntopt_pos_to_be->l_opnpstn_qty * (-1) ))) /
                      ( 1 - (d_min_mrgn/100.0 ) )                 );

		if(DEBUG_MSG_LVL_3)
		{
  		fn_userlog ( c_ServiceName, "In PUT d_trigger_prc is %lf,d_und_quote is %lf ",
                        d_trigger_prc,ptr_st_pstn_actn->d_und_quote * 100.0 );
		}

  	ptr_st_cntopt_pos_to_be->l_mtm_trg_prc = d_trigger_prc;

  	if ( ( ptr_st_pstn_actn->d_und_quote * 100.0 ) < d_trigger_prc )
  	{
      if(DEBUG_MSG_LVL_3) /*** Ver 3.4 added in debug 3 **/
      {
			  fn_userlog(c_ServiceName,"Inside Trigger Price less than d_und_quot");
      }

			i_returncode = fn_upd_mrgn_opt (  c_ServiceName,
                                    		ptr_st_pstn_actn,
                                    		ptr_st_cntopt_pos_crrnt,
                                    		ptr_st_cntopt_pos_to_be,
                                    		ptr_st_err_msg );
			if ( i_returncode != 0 )
    	{
    		return FAILURE;
    	}
			
			i_trnsctn = fn_begintran( c_ServiceName, ptr_st_err_msg->c_err_msg );
    	if ( i_trnsctn == -1 )
    	{
    		fn_errlog(c_ServiceName, "S31145", LIBMSG, ptr_st_err_msg->c_err_msg );
      	return FAILURE;
    	}
	
			if(DEBUG_MSG_LVL_3)
    	{
    		fn_userlog ( c_ServiceName, "Inside Quote Price Less Than Trigger Price");
      	fn_userlog ( c_ServiceName, "Old Trade Margin Is :%lf:",ptr_st_cntopt_pos_to_be->d_trd_mrgn);
      	fn_userlog ( c_ServiceName, "Trade Margin Is :%lf:",ptr_st_cntopt_pos_to_be->d_trd_mrgn);
      	fn_userlog ( c_ServiceName, "Trigger Price Is :%lf:",d_trigger_prc);
    	}
			
			EXEC SQL
    		INSERT INTO for_opt_imtm_rep
      	(
      		FOR_CLM_MTCH_ACCNT,
        	FOR_XCHNG_CD,
        	FOR_PRDCT_TYP,
        	FOR_UNDRLYNG,
        	FOR_EXPRY_DT,
        	FOR_STRK_PRC,
        	FOR_EXER_TYP,
        	FOR_OPT_TYP,
        	FOR_OPNPSTN_QTY,
        	FOR_TRD_MRGN,
					FOR_BLKD_TRD_MRGN,
        	FOR_TRGGR_PRC,
        	FOR_UND_QUOTE,
        	FOR_RPRT_TM_STMP
      	) 
      	VALUES
				(
      		:ptr_st_pstn_actn->c_cln_mtch_accnt,
        	:ptr_st_pstn_actn->c_xchng_cd,
        	:ptr_st_pstn_actn->c_prd_typ,
        	:ptr_st_pstn_actn->c_undrlyng,
        	:ptr_st_pstn_actn->c_expry_dt,
        	:ptr_st_pstn_actn->l_strike_prc,
        	:ptr_st_pstn_actn->c_exrc_typ,
        	:ptr_st_pstn_actn->c_opt_typ,
        	:ptr_st_cntopt_pos_to_be->l_opnpstn_qty,
        	:ptr_st_cntopt_pos_to_be->d_trd_mrgn,
					:d_fop_trd_mrgn,
        	:d_trigger_prc,
        	:ptr_st_pstn_actn->d_und_quote,
        	sysdate
      	);

			if ( SQLCODE != 0 )
    	{
    		fn_userlog(c_ServiceName,"Failed To Insert In To FOR_OPT_IMTM_REP");
      	fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg ); /** Ver 3.5 **/
				fn_errlog(c_ServiceName, "S31150", SQLMSG, ptr_st_err_msg->c_err_msg );
      	return FAILURE;
    	}
			if ( fn_committran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ) == -1 )
    	{
    		fn_errlog( c_ServiceName, "S31155",LIBMSG,ptr_st_err_msg->c_err_msg);
      	fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
      	return ( FAILURE );
    	}
			return SUCCESS;
		}
	}
	else 
	{
    if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
    {
		  fn_userlog(c_ServiceName,"Inside Trigger Price Calculation For Call Option ");
    }

		d_trigger_prc = ( ( (double) ptr_st_pstn_actn->l_strike_prc  +
                        (ptr_st_cntopt_pos_to_be->d_trd_mrgn /
                        (ptr_st_cntopt_pos_to_be->l_opnpstn_qty * (-1) ))) /
                      ( 1 + (d_min_mrgn/100.0 ) )                 );

		if(DEBUG_MSG_LVL_3)
		{
  		fn_userlog ( c_ServiceName, "d_trigger_prc is %lf and d_und_quote is %lf ",
                        d_trigger_prc,ptr_st_pstn_actn->d_und_quote * 100.0 );
		}

    ptr_st_cntopt_pos_to_be->l_mtm_trg_prc = d_trigger_prc;

		if ( ( ptr_st_pstn_actn->d_und_quote * 100.0 ) > d_trigger_prc )
		{
      if(DEBUG_MSG_LVL_3)  /** Ver 3.4 debug added **/
      {
			  fn_userlog(c_ServiceName,"Inside Trigger Price greater than d_und_quote");
      }

			i_returncode = fn_upd_mrgn_opt (  c_ServiceName,
                                    		ptr_st_pstn_actn,
                                    		ptr_st_cntopt_pos_crrnt,
                                    		ptr_st_cntopt_pos_to_be,
                                    		ptr_st_err_msg );
			if ( i_returncode != 0 )
      {
      	return FAILURE;
      }
	
			i_trnsctn = fn_begintran( c_ServiceName, ptr_st_err_msg->c_err_msg );
      if ( i_trnsctn == -1 )
      {
        fn_errlog(c_ServiceName, "S31160", LIBMSG, ptr_st_err_msg->c_err_msg );
        return FAILURE;
      }
			
			if(DEBUG_MSG_LVL_3)
      {
        fn_userlog ( c_ServiceName, "Inside Quote Price Less Than Trigger Price");
        fn_userlog ( c_ServiceName, "Old Trade Margin Is :%lf:",ptr_st_cntopt_pos_to_be->d_trd_mrgn)
;
        fn_userlog ( c_ServiceName, "Trade Margin Is :%lf:",ptr_st_cntopt_pos_to_be->d_trd_mrgn);
        fn_userlog ( c_ServiceName, "Trigger Price Is :%lf:",d_trigger_prc);
      }
			
			EXEC SQL
				INSERT INTO for_opt_imtm_rep
				(
					FOR_CLM_MTCH_ACCNT,
          FOR_XCHNG_CD,
          FOR_PRDCT_TYP,
          FOR_UNDRLYNG,
          FOR_EXPRY_DT,
          FOR_STRK_PRC,
          FOR_EXER_TYP,
          FOR_OPT_TYP,
          FOR_OPNPSTN_QTY,
          FOR_TRD_MRGN,
					FOR_BLKD_TRD_MRGN,
          FOR_TRGGR_PRC,
          FOR_UND_QUOTE,
          FOR_RPRT_TM_STMP
				)
				VALUES
				(
					:ptr_st_pstn_actn->c_cln_mtch_accnt,
          :ptr_st_pstn_actn->c_xchng_cd,
          :ptr_st_pstn_actn->c_prd_typ,
          :ptr_st_pstn_actn->c_undrlyng,
          :ptr_st_pstn_actn->c_expry_dt,
          :ptr_st_pstn_actn->l_strike_prc,
          :ptr_st_pstn_actn->c_exrc_typ,
          :ptr_st_pstn_actn->c_opt_typ,
          :ptr_st_cntopt_pos_to_be->l_opnpstn_qty,
          :ptr_st_cntopt_pos_to_be->d_trd_mrgn,
					:d_fop_trd_mrgn,
          :d_trigger_prc,
          :ptr_st_pstn_actn->d_und_quote,
          sysdate
        );
	
				if ( SQLCODE != 0 )
      	{
        	fn_userlog(c_ServiceName,"Failed To Insert In To FOR_OPT_IMTM_REP");
        	fn_errlog(c_ServiceName, "S31165", SQLMSG, ptr_st_err_msg->c_err_msg );
					fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg ); /** Ver 3.5 **/
        	return FAILURE;
      	}

      	if ( fn_committran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ) == -1 )
      	{
        	fn_errlog( c_ServiceName, "S31170",LIBMSG,ptr_st_err_msg->c_err_msg);
        	fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
        	return ( FAILURE );
      	}

			return SUCCESS;
    }
  }
	
	d_old_trade_mrgn = ptr_st_cntopt_pos_to_be->d_trd_mrgn;

  i_returncode = fn_upd_mrgn_opt (  c_ServiceName,
                                    ptr_st_pstn_actn,
                                    ptr_st_cntopt_pos_crrnt,
                                    ptr_st_cntopt_pos_to_be,
                                    ptr_st_err_msg );
	if ( i_returncode != 0 )
  {
    return FAILURE;
  }
	
	d_trigger_priceMTM = d_trigger_prc;
	d_ord_mrgn_MTM_b = ptr_st_cntopt_pos_crrnt->d_ordr_mrgn;
	d_trd_mrgn_MTM_b = ptr_st_cntopt_pos_crrnt->d_trd_mrgn ;
	d_spread_mrgn_MTM_b = 0 ;
	d_ord_mrgn_MTM_a = ptr_st_cntopt_pos_to_be->d_ordr_mrgn  ;
	d_trd_mrgn_MTM_a = ptr_st_cntopt_pos_to_be->d_trd_mrgn ;
	d_spread_mrgn_MTM_a = 0 ;
	l_opnpstn_MTM = ptr_st_cntopt_pos_to_be->l_opnpstn_qty ;
	
	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog ( c_ServiceName, "Before Inserting Into Report Table");
  	fn_userlog ( c_ServiceName, "Old Trade Margin Is :%lf:",ptr_st_cntopt_pos_to_be->d_trd_mrgn);
  	fn_userlog ( c_ServiceName, "Trade Margin Is :%lf:",ptr_st_cntopt_pos_to_be->d_trd_mrgn);
  	fn_userlog ( c_ServiceName, "Trigger Price Is :%lf:",d_trigger_prc);
	}


	i_trnsctn = fn_begintran( c_ServiceName, ptr_st_err_msg->c_err_msg );
	if ( i_trnsctn == -1 )
	{
		fn_errlog(c_ServiceName, "S31175", LIBMSG, ptr_st_err_msg->c_err_msg );
		return FAILURE;
	}

	EXEC SQL
		INSERT INTO for_opt_imtm_rep
		(
			FOR_CLM_MTCH_ACCNT,
      FOR_XCHNG_CD,
      FOR_PRDCT_TYP,
      FOR_UNDRLYNG,
      FOR_EXPRY_DT,
      FOR_STRK_PRC,
      FOR_EXER_TYP,
      FOR_OPT_TYP,
      FOR_OPNPSTN_QTY,
      FOR_TRD_MRGN,
			FOR_BLKD_TRD_MRGN,
      FOR_TRGGR_PRC,
      FOR_UND_QUOTE,
      FOR_RPRT_TM_STMP
    )
		VALUES
		(
			:ptr_st_pstn_actn->c_cln_mtch_accnt,
      :ptr_st_pstn_actn->c_xchng_cd,
      :ptr_st_pstn_actn->c_prd_typ,
      :ptr_st_pstn_actn->c_undrlyng,
      :ptr_st_pstn_actn->c_expry_dt,
      :ptr_st_pstn_actn->l_strike_prc,
      :ptr_st_pstn_actn->c_exrc_typ,
      :ptr_st_pstn_actn->c_opt_typ,
      :ptr_st_cntopt_pos_to_be->l_opnpstn_qty,
      :d_exstng_trd_mrgn,												/*** Ver 1.6 ***/
			:d_fop_trd_mrgn,
      :d_trigger_prc,
      :ptr_st_pstn_actn->d_und_quote,
      sysdate
    );

		if ( SQLCODE != 0 )
		{
			fn_userlog(c_ServiceName,"Failed To Insert In To FOR_OPT_IMTM_REP");
      fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );  /** Ver 3.5 **/
			fn_errlog(c_ServiceName, "S31180", SQLMSG, ptr_st_err_msg->c_err_msg );
      return FAILURE;
		}
	if ( fn_committran( c_ServiceName, i_trnsctn, ptr_st_err_msg->c_err_msg ) == -1 )
	{
		fn_errlog( c_ServiceName, "S31185",LIBMSG,ptr_st_err_msg->c_err_msg);
    fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
    return ( FAILURE );
 	}

	return SUCCESS; 
}			/*** Ver 1.5 Ends ***/

