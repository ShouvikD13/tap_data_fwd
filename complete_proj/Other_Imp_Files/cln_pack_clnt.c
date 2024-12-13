/**********************************************************************************/
/*  Program           : CLN_PACK_CLNT                                         		*/
/*                                                                            		*/
/*  Input             : C_PIPE_ID                                             		*/
/*                                                                            		*/
/*  Output            :                                                       		*/
/*                                                                            		*/
/*  Description       : Pack client is a tuxedo client process. It picks up   		*/
/*											orders from database, packs them into exchange 						*/
/*											specific format and puts them in send queue.							*/
/*                                                                            		*/
/*	Log								: Ver 1.0 22-Jun-2009 Indrajit Bhadange											*/
/*                 		: Ver 1.1 27-Aug-2009 Indrajit Bhadange											*/
/*                 		: Ver 1.2 14-Sep-2009 Indrajit Bhadange											*/
/*                 		: Ver 1.3 08-Dec-2009 Shailesh Hinge												*/
/*                 		: Ver 1.4 08-Dec-2009 Shailesh Hinge												*/
/*										:	Ver 1.5	04-Mar-2010	Sandeep Patil													*/
/*                    : Ver 1.6 16-Mar-2010 Sandeep Patil	                    		*/
/*										:	Ver	1.7	09-Nov-2010	Sandeep Patil													*/
/*										:	Ver	1.8	17-Nov-2011	Sandeep Patil													*/
/*										:	Ver	1.9	17-May-2012	Sandeep Patil                         */
/*										: Ver 2.0 08-Jun-2012 Vishnu Nair                     			*/
/*										: Ver 2.1 12-Dec-2012 Navina D.                       			*/
/*										: Ver 2.2 01-Nov-2012 Bineet Singh                     			*/
/*										: Ver 2.3 24-Sep-2013 Navina D.                       			*/
/*										: Ver 2.4 23-Jan-2015 Sachin Birje.                   			*/
/*                    : Ver 2.7 01-Aug-2015 Sachin Birje.                         */
/*                    : Ver 2.8 12-Jun-2015 Sachin Birje.                         */
/*                    : Ver 3.0 12-Aug-2015 Anand Dhopte                          */
/*                    : Ver 3.2 20-Jun-2016 Sachin Birje                          */
/*                    : Ver 3.3 24-Nov-2016 Bhupendra Malik                       */
/*                    : Ver 3.4 30-Nov-2016 Sachin Birje                          */
/*                    : Ver 3.5 28-Nov-2017 Varadraj Gramopadhye                  */
/*										: Ver 3.6 03-May-2018	Bhushan Harekar												*/
/*                    : Ver 3.7 18-mar-2018 Sachin Birje                          */
/*										:	Ver 3.8 19-Feb-2019	Bhushan Harekar												*/
/*                    : Ver 3.9 30-Aug-2019 Suchita Dabir                         */
/*                    : Ver 4.1 04-Feb-2022 Sachin Birje                          */
/*                    : Ver 4.2 01-Sep-2022 Sachin Birje                          */
/*                    : Ver 4.3 01-Feb-2023 Mahesh Shinde													*/
/*                    : Ver 4.4 18-Mar-2023 Sachin Birje 													*/
/**********************************************************************************/
/*  Log								: Ver 1.1 Changes to add pack_val to reduce the number  		*/
/*                      of transactions.                                      		*/
/*                    : Ver 1.2 Changes to replace tuxedo service call with   		*/
/*                      C functions for time optimization.                    		*/
/*                    : Ver 1.3 Trigger look tab logic removed.               		*/
/*                    : Ver 1.4 Excercise Market handling added										*/
/*										:	Ver 1.5	Exchange Status Check While Forwarding Orders			*/
/*                    : Ver 1.6 NNF Message Download changes                  		*/
/*										: Ver	1.7	Message ID Changed To S While Inserting Into FTM	*/
/*										:	Ver 1.8 Download Bug Fix																	*/
/*										:	Ver 1.9 Download Bug Fix (Sleep Incresed From 1 Sec To 2)	*/
/*										: Ver 2.0 Changes to allow trading in sub series            */
/*                              in exchanges.                                     */
/*                    : Ver 2.1 SLTP FuturePLUS Handling                          */
/*										: Ver 2.2 Consolidated FNO Console NFO/BFO   				        */
/*                    : Ver 2.3 OptionPLUS Handling                               */
/*                    : Ver 2.4 Auto Reconnect Changes                            */
/*                    : Ver 2.7 FO Auto MTM Changes                               */
/*                    : Ver 2.8 FNO VTC order Changes                             */
/*                    : Ver 3.0 Contract Master and Trade Quote Merger Changes    */
/*                    : Ver 3.2 Pending trigger processing                        */
/*                    : Ver 3.3 Future Trail changes                              */
/*                    : Ver 3.4 Stream number hardcoded to 9                      */
/*                    : Ver 3.5 Update order status for failed order &						*/
/*																triggering the mail         										*/
/*										: Ver 3.6 NNF_CHANGES_7.21 ( PAN Changes )									*/
/*                    : Ver 3.7 Rollover with Spread Changes                      */
/*										: Ver 3.8 FO Direct Connectivity 														*/
/*                    : Ver 3.9 Handling NVL for last act ref ( Suchita D. )      */
/*                    : Ver 4.0 Stream Number Changes  (Sachin Birje)             */
/*                    : Ver 4.1 CR-ISEC14-169739 -Introduction of Algorithm Trading Strategies (Sachin Birje)     */
/*                    : Ver 4.2 Buleshift algo Changes                            */
/*                    : Ver 4.3 sleep changes from 1 second to 50 milisecond      */
/*                    : Ver 4.4 Query Optimization Minor Changes#1507             */
/**********************************************************************************/

/**** C headers ****/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

/**** Added for Pro c codes ****/
#include <sqlca.h>

/**** Tuxedo headers ****/
#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>

/***** Application headers *****/
#include <fo_exg_pack_lib.h>
#include <fn_msgq.h>
#include <fn_md5.h> // Used for calculating the MD5
#include <fo.h>
#include <fn_env.h>
#include <fn_battmpt.h>
#include <fn_tuxlib.h>
#include <fml_rout.h>
#include <fn_ddr.h>
#include <fn_log.h>			   // Used for logging using Tuxedo
#include <fn_scklib_tap.h>	   // Used for the socket programming
#include <fn_read_debug_lvl.h> // Used for setting up the debug level
#include <fo_view_def.h>
#include <fo_fml_def.h>

/** Ver 2.4 **/

#define LOCK 2049 /* Ver 1.2 */

/**** Global Variables ****/
/**
long int li_look_tab_hnd;**/

long int li_seq_nm = 0;
int i_key = 0;
int i_qid = 0;
long int li_max_q = 0;
long int li_cnt_q = 0;
int i_queue_chk_flg = 1;

int i_ord_mkt_stts;
int i_ext_mkt_stts;
int i_exr_mkt_stts;
int i_pmp_stts;
int i_upd_stts;

/**
int i_look_tab; **/

int i_part_stts;
int i_brkr_stts;
int i_exit_stts;

int i_reco_stts;

int i_ord_lmt_val = 0;
char c_filter2[32];
struct st_req_q_data st_q_packet;
struct st_req_q_data st_sndq_packets[20]; /* Ver 1.1 */
int i_pack_counter = 0;					  /* Ver 1.1 */
int i_pack_val = 0;						  /* Ver 1.1 */
char c_time[12];
char c_auto_connection = '\0'; /** Ver 2.4 **/

EXEC SQL BEGIN DECLARE SECTION;

char sql_c_xchng_ctcl_id[LEN_CTCL_ID];
char sql_c_xchng_trdr_id[LEN_TRDR_ID];
char sql_c_xchng_cd[3 + 1];
char sql_c_xchng_brkr_id[LEN_BRKR_ID];
varchar sql_c_nxt_trd_date[LEN_DATE];
char sql_c_pipe_id[2 + 1];
struct st_opm_pipe_mstr st_opm_mstr;
long sql_li_max_pnd_ord;

/***** ver 3.5 variables starts *****/
char c_ordr_rfrnc[20];
char c_sprd_ord_ind;
/***** ver 3.5 variables ends *****/

EXEC SQL END DECLARE SECTION;

void fn_gettime(void);

int fn_do_reco(char c_reco_mode, char *c_tm_stmp, char *c_ServiceName, char *c_err_msg);

int fn_prcs_reco(char c_reco_mode, /***	Ver 1.6	***/ char *c_tm_stmp, long li_stream_cnt, long li_stream_no, char *c_ServiceName, char *c_err_msg);

int fn_get_nxt_rec(char *c_ServiceName, char *c_err_msg);

int fn_rjct_rcrd(struct vw_xchngbook *ptr_st_rqst, struct vw_orderbook *ptr_st_ord, char *c_ServiceName, char *c_err_msg);

/* Ver 1.1 Starts */
int fn_write_packets_q(char *c_ServiceName, char *c_err_msg);

void fn_print_details(int i_index, char *c_ServiceName, char *c_err_msg);

/* Ver 1.1 Ends */

/* Ver 1.2 Starts */

int fn_get_ext_cnt(struct vw_contract *ptr_st_cnt, struct vw_nse_cntrct *ptr_st_nse_cnt, char *c_ServiceName, char *c_err_msg);

int fn_seq_to_omd(struct vw_xchngbook *ptr_st_xchngbook, char *c_ServiceName, char *c_err_msg, char *c_prgm_flg); /*** ver 2.7 **/

int fn_ref_to_ord(struct vw_orderbook *ptr_st_orderbook,
				  char *c_pan_no,	   /*** Ver 3.6 ***/
				  char *c_lst_act_ref, /*** Ver 3.8 ***/
				  char *c_esp_id,	   /*** Ver 4.1 ***/
				  char *c_algo_id,	   /*** Ver 4.1 ***/
				  char *c_source_flg,  /*** Ver 4.2 ***/
				  char *c_ServiceName,
				  char *c_err_msg);

int fn_upd_xchngbk(struct vw_xchngbook *ptr_st_xchngbook,
				   char *c_ServiceName,
				   char *c_err_msg);

int fn_upd_ordrbk(struct vw_orderbook *ptr_st_orderbook,
				  char *c_ServiceName,
				  char *c_err_msg);

int fn_qry_spdbk(struct vw_spdordbk *ptr_st_spd_ordbk,
				 char *c_ServiceName,
				 char *c_err_msg);

int fn_exrq_dtls(struct vw_exrcbook *ptr_st_exrcbook,
				 char *c_ServiceName,
				 char *c_err_msg);

int fn_upd_exbk(struct vw_exrcbook *ptr_st_exrcbook,
				char *c_ServiceName,
				char *c_err_msg);

/* Ver 1.2 Ends */

int fn_flr_ord_plc(char *c_ordr_rfrnc,
				   char c_sprd_ord_ind,
				   char *c_ServiceName,
				   char *c_err_msg);
void CLN_PACK_CLNT(int argc, char *argv[]) // service
{
	int i_trnsctn;
	int qi_ch_val;
	int i_lmt_counter = 1;
	int i_in_tran_flg;
	char c_ServiceName[33];
	char c_err_msg[256];
	char c_brkr_id[6];				/*** Ver 2.2 ***/
	strcpy(c_ServiceName, argv[0]); //

	INITBATDBGLVL(c_ServiceName); // here we are initliasing the debug level based on the c_service

	sprintf(st_opm_mstr.c_opm_pipe_id, argv[3]);

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, " In function CLN_PACK_CLNT %s:", c_ServiceName);
	}
	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "Service name is is :%s:", c_ServiceName);
		fn_userlog(c_ServiceName, "The pipe id is :%s:", st_opm_mstr.c_opm_pipe_id);
	}

	i_in_tran_flg = 0;

	while (i_exit_stts == DONT_EXIT)
	{
		while (((i_ord_mkt_stts == TRUE) ||(i_ext_mkt_stts == TRUE) ||(i_exr_mkt_stts == TRUE)) &&
(i_pmp_stts == TRUE && i_part_stts == TRUE && i_brkr_stts == TRUE))
		{
			if (i_in_tran_flg == 0)
			{
				/***** Begin Transaction *********/

				i_trnsctn = fn_begintran(c_ServiceName, c_err_msg);

				if (i_trnsctn == -1)
				{
					fn_errlog(c_ServiceName, "S31005", TPMSG, c_err_msg);
					return;
				}
				i_in_tran_flg = 1;
			}

			i_ch_val = fn_get_nxt_rec(c_ServiceName, c_err_msg);

			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "Before Switch");
			}

			switch (i_ch_val)
			{
			case NDF:

				if (DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName, "In NDF case");
				}
				if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
				{
					fn_gettime();
					fn_userlog(c_ServiceName, "TIME_STATS NDF after order :%d: at :%s:",
							   st_sndq_packets[i_pack_counter].st_exch_msg_data.st_net_header.i_seq_num, c_time);
				} /* Ver 1.2 Ends */
				if (i_pack_counter > 0) /* Ver 1.1 Starts */
				{
					if (fn_committran(c_ServiceName, i_trnsctn, c_err_msg) == -1)
					{
						if (DEBUG_MSG_LVL_0)
						{
							fn_userlog(c_ServiceName, "CRUCIAL COMMIT FAILED !!!!! [NDF]");
						}
						fn_errlog(c_ServiceName, "S31010", LIBMSG, c_err_msg);
						return;
					}

					if (fn_write_packets_q(c_ServiceName, c_err_msg) == -1)
					{
						fn_errlog(c_ServiceName, "S31015", LIBMSG, c_err_msg);
						return;
					}
				}
				else
				{
					fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
				} /* Ver 1.1 Ends */

				i_in_tran_flg = 0;

				/*Commented in Ver 1.3
					i_look_tab = FALSE;
					li_look_tab_hnd = tpsubscribe ( "TRG_LOOK_TAB",
																					(char *)c_filter2,
																					(TPEVCTL *)NULL,
																					0);

					if ( li_look_tab_hnd == -1 )
					{
						fn_errlog(c_ServiceName,"S31020", TPMSG, c_err_msg);
						return;
					}
					**/

				sleep(1); /******Put for reducing the resource utilization***/
				break;

			case OVER_LOAD:
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "OVER LOAD:Waiting for Release pack_counter is :%d:", i_pack_counter);
				}

				if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
				{
					fn_gettime();
					fn_userlog(c_ServiceName, "TIME_STATS OVERLOAD after order :%d: at :%s:",
							   st_sndq_packets[i_pack_counter].st_exch_msg_data.st_net_header.i_seq_num, c_time);
				} /* Ver 1.2 Ends */

				if (i_pack_counter > 0) /* Ver 1.1 Starts */
				{
					if (fn_committran(c_ServiceName, i_trnsctn, c_err_msg) == -1)
					{
						if (DEBUG_MSG_LVL_0)
						{
							fn_userlog(c_ServiceName, "CRUCIAL COMMIT FAILED !!!!! [OVERLOAD]");
						}
						fn_errlog(c_ServiceName, "S31025", LIBMSG, c_err_msg);
						return;
					}

					if (fn_write_packets_q(c_ServiceName, c_err_msg) == -1)
					{
						fn_errlog(c_ServiceName, "S31030", LIBMSG, c_err_msg);
						return;
					}
				}
				else
				{
					fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
				}
				i_in_tran_flg = 0; /* Ver 1.1 Ends */
				sleep(1);
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "Resuming after OVER LOAD wait.....,pack counter is :%d:", i_pack_counter);
				}
				if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
				{
					fn_gettime();
					fn_userlog(c_ServiceName, "TIME_STATS OVERLOAD released at :%s:", c_time);
				} /* Ver 1.2 Ends */
				break;

			case DATA_RTRVD:

				/* Ver 1.1 Starts */
				memcpy(&st_sndq_packets[i_pack_counter], &st_q_packet, sizeof(st_q_packet));

				if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
				{
					fn_gettime();
					fn_userlog(c_ServiceName, "TIME_STATS order :%d: packed at :%s:",
							   st_sndq_packets[i_pack_counter].st_exch_msg_data.st_net_header.i_seq_num, c_time);
				} /* Ver 1.2 Ends */

				i_pack_counter++;
				if (DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName, "In DATA RTRVD case");
				}

				if (i_lmt_counter <= i_ord_lmt_val)
				{
					i_lmt_counter++;
					if (DEBUG_MSG_LVL_1)
					{
						fn_userlog(c_ServiceName, "In Initial forwarding loop ,Pack counter is :%d:", i_pack_counter);
					}

					if (i_pack_counter >= i_pack_val || i_lmt_counter > i_ord_lmt_val)
					{

						if (fn_committran(c_ServiceName, i_trnsctn, c_err_msg) == -1)
						{
							if (DEBUG_MSG_LVL_0)
							{
								fn_userlog(c_ServiceName, "CRUCIAL COMMIT FAILED !!!!! [DATA RETRIEVED Initial forwarding]");
							}
							fn_errlog(c_ServiceName, "S31035", LIBMSG, c_err_msg);
							return;
						}

						i_in_tran_flg = 0;

						if (fn_write_packets_q(c_ServiceName, c_err_msg) == -1)
						{
							fn_errlog(c_ServiceName, "S31040", LIBMSG, c_err_msg);
							return;
						}
						if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
						{
							fn_gettime();
							fn_userlog(c_ServiceName, "TIME_STATS After writing packets in queue :%s:", c_time);
						} /* Ver 1.2 Ends */
					}

					if (i_lmt_counter > i_ord_lmt_val)
					{
						i_pmp_stts = FALSE; 
						i_ord_lmt_val = 0;
					}
				}
				else
				{
					if (DEBUG_MSG_LVL_1)
					{
						fn_userlog(c_ServiceName, "In Normal forwarding loop ,Pack counter is :%d:", i_pack_counter);
					}
					if (i_pack_counter >= i_pack_val)
					{

						if (fn_committran(c_ServiceName, i_trnsctn, c_err_msg) == -1)
						{
							if (DEBUG_MSG_LVL_0)
							{
								fn_userlog(c_ServiceName, "CRUCIAL COMMIT FAILED !!!!! [DATA RETRIEVED Normal forwarding]");
							}
							fn_errlog(c_ServiceName, "S31045", LIBMSG, c_err_msg);
							return;
						}

						i_in_tran_flg = 0;

						if (fn_write_packets_q(c_ServiceName, c_err_msg) == -1)
						{
							fn_errlog(c_ServiceName, "S31050", LIBMSG, c_err_msg);
							return;
						}
						if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
						{
							fn_gettime();
							fn_userlog(c_ServiceName, "TIME_STATS After writing packets in queue :%s:", c_time);
						} /* Ver 1.2 Ends */
					}
				}
				/* Ver 1.1 Ends */
				break;

			case DATA_CNCLD:
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "In DATA CANCELLED case: do nothing");
				}
				break;

			case DATA_SKPD:
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "In DATA SKIPPED case: do nothing");
				}
				break;

			case TKN_NA:
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "In Token not avaialble case: do nothing");
				}
				break;
			case SKIP_ORD: /* case added in ver 3.5 */
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "Inside SKIP_ORD case :-");
				}
				i_in_tran_flg = 0;
				i_pack_counter = 0;
				memset(&st_sndq_packets, '\0', sizeof(st_sndq_packets));
				fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
				i_ch_val = fn_flr_ord_plc(c_ordr_rfrnc, c_sprd_ord_ind, c_ServiceName, c_err_msg);
				if (i_ch_val == -1)
				{
					fn_userlog(c_ServiceName, "fn_flr_ord_plc function is failed");
					return;
				}
				break;
			case LOCK_ERR:
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "In LOCK ERROR case: Sleep for 1 second");
				}
				if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
				{
					fn_gettime();
					fn_userlog(c_ServiceName, "TIME_STATS LOCK after order :%d: at :%s:",
							   st_sndq_packets[i_pack_counter].st_exch_msg_data.st_net_header.i_seq_num, c_time);
				} /* Ver 1.2 Ends */
				i_in_tran_flg = 0;
				i_pack_counter = 0;
				memset(&st_sndq_packets, '\0', sizeof(st_sndq_packets));
				fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
				/* sleep(1); Commented in Ver 4.3 */
				usleep(50000);		 /**** Added in Ver 4.3 ***/
				if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
				{
					fn_gettime();
					fn_userlog(c_ServiceName, "TIME_STATS LOCK wait ends at :%s:", c_time);
				} /* Ver 1.2 Ends */
				break;
			case ERROR:
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "In ERROR case: Abort transaction");
				}
				fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
				return;

			default:
				fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "Logic error : unknown return type");
				}
				return;
			}

			/**** check for unsolicited messages ****/
			i_ch_val = fn_chk_stts(c_ServiceName);

			if (i_ch_val == -1)
			{
				fn_errlog(c_ServiceName, "S31055", LIBMSG, c_err_msg);
				return;
			}
		} /******** End of 2nd While Loop *******/

		if (i_in_tran_flg == 1)
		{
			i_in_tran_flg = 0;
			i_pack_counter = 0;										 /* Ver 1.1 */
			memset(&st_sndq_packets, '\0', sizeof(st_sndq_packets)); /* Ver 1.1 */
			fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
		}

		/**** check for unsolicited messages  ****/
		sleep(1); /******Put by sangeet for reducing the resource utilization***/

		i_ch_val = fn_chk_stts(c_ServiceName);

		if (i_ch_val == -1)
		{
			fn_errlog(c_ServiceName, "S31060", LIBMSG, c_err_msg);
			return;
		}

		if (i_upd_stts == TRUE)
		{
			i_trnsctn = fn_begintran(c_ServiceName, c_err_msg);

			if (i_trnsctn == -1)
			{
				fn_errlog(c_ServiceName, "S31065", LIBMSG, c_err_msg);
				return;
			}

			if (i_pmp_stts == TRUE)
			{
				EXEC SQL
					UPDATE opm_ord_pipe_mstr
						SET opm_pmp_stts = 1 WHERE opm_pipe_id = : sql_c_pipe_id;

				if (SQLCODE != 0)
				{
					fn_errlog(c_ServiceName, "S31070", SQLMSG, c_err_msg);
					return;
				}
				/* Start - Version 2.2 */

				EXEC SQL
					SELECT exg_brkr_id
						INTO : c_brkr_id
								   FROM exg_xchng_mstr
									   WHERE exg_xchng_cd = 'NFO';

				if (SQLCODE != 0)
				{
					fn_errlog(c_ServiceName, "S31075", SQLMSG, c_err_msg);
					return;
				}

				EXEC SQL
					INSERT INTO FTM_FO_TRD_MSG(FTM_XCHNG_CD,
											   FTM_BRKR_CD,
											   FTM_MSG_ID,
											   FTM_MSG,
											   FTM_TM)
						VALUES(
							'NFO',
							: c_brkr_id,
							  'T',
							  'Forwarding started for Pipe ID ' ||
							: sql_c_pipe_id,
							  sysdate);

				if (SQLCODE != 0)
				{
					fn_errlog(c_ServiceName, "S31080", SQLMSG, c_err_msg);
					return;
				}
				/* End - Version 2.2 */
			}
			else
			{
				EXEC SQL
					UPDATE opm_ord_pipe_mstr
						SET opm_pmp_stts = 0 WHERE opm_pipe_id = : sql_c_pipe_id;

				if (SQLCODE != 0)
				{
					fn_errlog(c_ServiceName, "S31085", SQLMSG, c_err_msg);
					return;
				}
			}

			i_ch_val = fn_committran(c_ServiceName, i_trnsctn, c_err_msg);

			if (i_ch_val == -1)
			{
				fn_errlog(c_ServiceName, "S31090", LIBMSG, c_err_msg);
				return;
			}

			i_upd_stts = FALSE;
		}
	} /******* End of 1st While Loop *********/
}

int fn_bat_init(int argc, char *argv[])
{
	long int li_ord_opn_hnd = 0;
	long int li_ord_cls_hnd = 0;
	long int li_ext_opn_hnd = 0;
	long int li_ext_cls_hnd = 0;
	long int li_exr_opn_hnd = 0;
	long int li_exr_cls_hnd = 0;
	long int li_part_sus_hnd = 0;
	long int li_part_act_hnd = 0;
	long int li_brkr_sus_hnd = 0;
	long int li_brkr_act_hnd = 0;
	long int li_strt_pmp_hnd = 0;
	long int li_stop_pmp_hnd = 0;
	long int li_reco_over_hnd = 0;

	int i_ch_val;
	int i_trnsctn;
	int i_stream_count_opm = 0; /** Ver 3.4 **/
	int i_stream_count = 0;		/** Ver 3.4 **/

	char *ptr_c_tmp;
	char c_ServiceName[33];
	char c_err_msg[256];
	char c_filter1[32];
	char c_reco_mode;
	char c_tm_stmp[LEN_DT];

	struct vw_sequence st_seq;

	EXEC SQL BEGIN DECLARE SECTION;
	char sql_exg_settlor_stts;
	char sql_exg_brkr_stts;
	char sql_exg_crrnt_stts;
	char sql_exg_extnd_mrkt_stts;
	char sql_exg_exrc_mkt_stts;
	EXEC SQL END DECLARE SECTION;

	memset(&st_sndq_packets, '\0', sizeof(st_sndq_packets)); /* Ver 1.1 */
	strcpy(c_ServiceName, "cln_pack_clnt");

	if (argc < 6)
	{
		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName,
					   "Usage - cln_snd_clnt <tag qualifier> <Exchange cd> <Pipe id> <ord_lmt_val> <Reco mode> <O-Time stamp>");
		}
		return (-1);
	}

	c_auto_connection = 'N'; /** Ver 2.4 **/

	strcpy(sql_c_pipe_id, argv[3]);
	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "The pipe id is :%s:", sql_c_pipe_id);
	}
	c_reco_mode = argv[5][0];
	i_ord_lmt_val = atoi(argv[4]);

	/*** Initialize the Routing string ***/
	fn_init_ddr_pop(argv[3], TRADING_SECTION, COMMON);

	/***  Get IPC key  ***/
	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName, "SEND_QUEUE");
	if (ptr_c_tmp == NULL)
	{
		fn_errlog(c_ServiceName, "S31095", LIBMSG, c_err_msg);
		return -1;
	}
	i_key = atoi(ptr_c_tmp);

	/** Create the Message Q identifier **/

	if (fn_crt_msg_q(&i_qid, i_key, CLIENT, c_ServiceName, c_err_msg) == -1)
	{
		fn_errlog(c_ServiceName, "S31100", c_err_msg, c_err_msg);
	}

	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName, "USER_TYPE");

	if (ptr_c_tmp == NULL)
	{
		fn_errlog(c_ServiceName, "L31005", LIBMSG, c_err_msg);
		return -1;
	}

	st_opm_mstr.si_user_typ_glb = atoi(ptr_c_tmp);
	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "USER TYPE IS :%d:", st_opm_mstr.si_user_typ_glb);
	}

	/******* Read the PACK VAL from ini file *********/
	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName, "PACK_VAL");
	if (ptr_c_tmp == NULL)
	{
		fn_errlog(c_ServiceName, "L31010", LIBMSG, c_err_msg);
		return (-1);
	}
	i_pack_val = atoi(ptr_c_tmp);
	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "PACK VAL IS :%d:", i_pack_val);
	}

	/**** Set Global Variables ****/
	i_ord_mkt_stts = FALSE;
	i_ext_mkt_stts = FALSE;
	i_exr_mkt_stts = FALSE;
	i_pmp_stts = FALSE;
	i_upd_stts = FALSE;

	/**
	i_look_tab = TRUE;**/

	i_part_stts = FALSE;
	i_brkr_stts = FALSE;
	i_exit_stts = DONT_EXIT;

	/**** Get exchange code from the database ****/

	EXEC SQL
		select opm_xchng_cd,
		opm_max_pnd_ord,
		opm_stream_no /*** Ver 3.4 ***/
			into : sql_c_xchng_cd,
		: sql_li_max_pnd_ord,
		: i_stream_count_opm /*** Ver 3.4 ***/
			  From opm_ord_pipe_mstr
				  where opm_pipe_id = : sql_c_pipe_id;

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31105", SQLMSG, c_err_msg);
		return (-1);
	}

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Exchange code is :%s:", sql_c_xchng_cd);
	}

	EXEC SQL
		SELECT exg_settlor_stts,
		exg_brkr_stts,
		exg_crrnt_stts,
		exg_extnd_mrkt_stts,
		exg_exrc_mkt_stts
			INTO : sql_exg_settlor_stts,
		: sql_exg_brkr_stts,
		: sql_exg_crrnt_stts,
		: sql_exg_extnd_mrkt_stts,
		: sql_exg_exrc_mkt_stts
			  FROM exg_xchng_mstr
				  WHERE exg_xchng_cd = : sql_c_xchng_cd;

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31110", SQLMSG, c_err_msg);
		return (-1);
	}

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Settlor status is :%c:", sql_exg_settlor_stts);
		fn_userlog(c_ServiceName, "broker status is :%c:", sql_exg_brkr_stts);
		fn_userlog(c_ServiceName, "exchange status is :%c:", sql_exg_crrnt_stts);
		fn_userlog(c_ServiceName, "extended market status is :%c:", sql_exg_extnd_mrkt_stts);
		fn_userlog(c_ServiceName, "exercise market status is :%c:", sql_exg_exrc_mkt_stts);
	}

	if (sql_exg_settlor_stts == 'A')
	{
		i_part_stts = TRUE;
	}

	if (sql_exg_brkr_stts == 'A')
	{
		i_brkr_stts = TRUE;
	}
	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg);

	if (i_trnsctn == -1)
	{
		fn_errlog(c_ServiceName, "S31115", LIBMSG, c_err_msg);
		return (-1);
	}

	fn_userlog(c_ServiceName, "i_stream_count_opm :%d:", i_stream_count_opm);

	if (i_stream_count_opm == 0)
	{
		/*** Ver 3.4 Starts here ********/
		i_stream_count = 0;
		EXEC SQL
			SELECT NVL(MAX(FXB_STREAM_NO), 0)
				INTO : i_stream_count
						   FROM FXB_FO_XCHNG_BOOK
							   WHERE FXB_PIPE_ID = : sql_c_pipe_id
														 AND FXB_MOD_TRD_DT = trunc(sysdate)
															 AND FXB_STREAM_NO NOT IN('-1', '99');
		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "S31120", SQLMSG, c_err_msg);
			return (-1);
		}

		if (i_stream_count < 9)
		{
			i_stream_count = 9;
		}

		fn_userlog(c_ServiceName, "MAX i_stream_count to be updated :%d:", i_stream_count);

		EXEC SQL
			UPDATE opm_ord_pipe_mstr
				SET opm_pmp_stts = 0,
					OPM_STREAM_NO = : i_stream_count
										  WHERE opm_pipe_id = : sql_c_pipe_id;
		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "S31125", SQLMSG, c_err_msg);
			return (-1);
		}

	} /*** Ver 3.4 Ends here **********/
	else
	{
		EXEC SQL
			UPDATE opm_ord_pipe_mstr
				SET opm_pmp_stts = 0 WHERE opm_pipe_id = : sql_c_pipe_id;

		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "S31130", SQLMSG, c_err_msg);
			return (-1);
		}
	}

	i_ch_val = fn_committran(c_ServiceName, i_trnsctn, c_err_msg);

	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S31135", LIBMSG, c_err_msg);
		return (-1);
	}

	sprintf(c_filter1, "FFO_FILTER=='%s'", sql_c_xchng_cd);
	sprintf(c_filter2, "FFO_FILTER=='%s'", sql_c_pipe_id);

	/**** Subscribe for triggers ****/

	li_ord_opn_hnd = tpsubscribe("TRG_ORD_OPN", (char *)c_filter1, (TPEVCTL *)NULL, 0);

	if (li_ord_opn_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31140", TPMSG, c_err_msg);
		return (-1);
	}

	li_ord_cls_hnd = tpsubscribe("TRG_ORD_CLS", (char *)c_filter1, (TPEVCTL *)NULL, 0);

	if (li_ord_cls_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31145", TPMSG, c_err_msg);
		return (-1);
	}

	li_ext_opn_hnd = tpsubscribe("TRG_EXT_OPN", (char *)c_filter1, (TPEVCTL *)NULL, 0);

	if (li_ext_opn_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31150", TPMSG, c_err_msg);
		return (-1);
	}

	li_ext_cls_hnd = tpsubscribe("TRG_EXT_CLS", (char *)c_filter1, (TPEVCTL *)NULL, 0);

	if (li_ext_cls_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31155", TPMSG, c_err_msg);
		return (-1);
	}

	li_exr_opn_hnd = tpsubscribe("TRG_EXR_OPN", (char *)c_filter1, (TPEVCTL *)NULL, 0);

	if (li_exr_opn_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31160", TPMSG, c_err_msg);
		return (-1);
	}

	li_exr_cls_hnd = tpsubscribe("TRG_EXR_CLS", (char *)c_filter1, (TPEVCTL *)NULL, 0);

	if (li_exr_cls_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31165", TPMSG, c_err_msg);
		return (-1);
	}

	li_part_sus_hnd = tpsubscribe("TRG_PART_SUS", (char *)c_filter1, (TPEVCTL *)NULL, 0);

	if (li_part_sus_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31170", TPMSG, c_err_msg);
		return (-1);
	}

	li_part_act_hnd = tpsubscribe("TRG_PART_ACT", (char *)c_filter1, (TPEVCTL *)NULL, 0);

	if (li_part_act_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31175", TPMSG, c_err_msg);
		return (-1);
	}

	li_brkr_sus_hnd = tpsubscribe("TRG_BRKR_SUS", (char *)c_filter1, (TPEVCTL *)NULL, 0);

	if (li_brkr_sus_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31180", TPMSG, c_err_msg);
		return (-1);
	}

	li_brkr_act_hnd = tpsubscribe("TRG_BRKR_ACT", (char *)c_filter1, (TPEVCTL *)NULL, 0);

	if (li_brkr_act_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31185", TPMSG, c_err_msg);
		return (-1);
	}

	li_strt_pmp_hnd = tpsubscribe("TRG_STRT_PMP", (char *)c_filter2, (TPEVCTL *)NULL, 0);

	if (li_strt_pmp_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31190", TPMSG, c_err_msg);
		return (-1);
	}

	li_stop_pmp_hnd = tpsubscribe("TRG_STOP_PMP", (char *)c_filter2, (TPEVCTL *)NULL, 0);

	if (li_stop_pmp_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31195", TPMSG, c_err_msg);
		return (-1);
	}

	li_reco_over_hnd = tpsubscribe("TRG_RECO_OVER", (char *)c_filter2, (TPEVCTL *)NULL, 0);

	if (li_reco_over_hnd == -1)
	{
		fn_errlog(c_ServiceName, "S31200", TPMSG, c_err_msg);
		return (-1);
	}

	EXEC SQL
		SELECT OPM_TRDR_ID,
		OPM_BRNCH_ID
			INTO : st_opm_mstr.c_opm_trdr_id,
		: st_opm_mstr.li_opm_brnch_id
			  FROM OPM_ORD_PIPE_MSTR
				  WHERE OPM_XCHNG_CD = : sql_c_xchng_cd
											 AND OPM_PIPE_ID = : sql_c_pipe_id;

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31205", SQLMSG, c_err_msg);
		return (-1);
	}

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Trader ID is :%s:", st_opm_mstr.c_opm_trdr_id);
		fn_userlog(c_ServiceName, "Branch ID is :%ld:", st_opm_mstr.li_opm_brnch_id);
	}

	EXEC SQL
		Select exg_nxt_trd_dt,
		exg_brkr_id,
		exg_ctcl_id
			into : sql_c_nxt_trd_date,
		: st_opm_mstr.c_xchng_brkr_id,
		: sql_c_xchng_ctcl_id
			  From exg_xchng_mstr
				  Where exg_xchng_cd = : sql_c_xchng_cd;

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31210", SQLMSG, c_err_msg);
		return (-1);
	}

	SETNULL(sql_c_nxt_trd_date);

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Next trade date is :%s:", sql_c_nxt_trd_date.arr);
		fn_userlog(c_ServiceName, "Broker id is :%s:", st_opm_mstr.c_xchng_brkr_id);
		fn_userlog(c_ServiceName, "CTCL id is :%s:", sql_c_xchng_ctcl_id);
	}

	if (c_reco_mode == 'B')
	{
		strcpy(c_tm_stmp, argv[6]);
		c_auto_connection = argv[7][0]; /** Ver 2.4 **/
	}
	else
	{
		strcpy(c_tm_stmp, " ");
	}

	ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName, "MAX_Q_CHK_INTERVAL");

	if (ptr_c_tmp == NULL)
	{
		fn_errlog(c_ServiceName, "S31215", LIBMSG, c_err_msg);
		return (-1);
	}

	li_max_q = atol(ptr_c_tmp);

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Max pending orders in database is :%ld:", sql_li_max_pnd_ord);
		fn_userlog(c_ServiceName, "Max Queue Check Interval-:%ld:", li_max_q);
	}

	/*** Ver 3.2 Starts Here ***/
	i_ch_val = fn_chk_stts(c_ServiceName); //

	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "S31220", LIBMSG, c_err_msg);
	}
	/*** Ver 3.2 Ends here ***/

	i_ch_val = fn_do_reco(c_reco_mode, c_tm_stmp, c_ServiceName, c_err_msg);

	// we are calling "c_do_reco" with te parameter (c_reco_mode , c_tm_stmp) both these variables are initialised with argv.

	if (i_ch_val == -1)
	{
		return (-1);
	}

	li_seq_nm = 0;

	if (sql_exg_crrnt_stts == EXCHANGE_OPEN)
	{
		i_ord_mkt_stts = TRUE;
	}

	if (sql_exg_extnd_mrkt_stts == EXCHANGE_OPEN)
	{
		i_ext_mkt_stts = TRUE;
	}

	if (sql_exg_exrc_mkt_stts == EXCHANGE_OPEN)
	{
		i_exr_mkt_stts = TRUE;
	}

	fn_var = CLN_PACK_CLNT;

	return (0);
}

void fn_bat_term(int argc, char *argv[])
{
	return;
}

int fn_prcs_trg(char *c_ServiceName, int i_trg_typ)
{
	int i_ch_val;
	char c_err_msg[256];

	char sql_xchng_crrnt_stts;		/***	Ver 1.5	***/
	char sql_xchng_exrc_mkt_stts;	/***  Ver 1.5 ***/
	char sql_xchng_extnd_mrkt_stts; /***  Ver 1.5 ***/

	if (i_trg_typ == SYSTM_TRG)
	{
		i_exit_stts = EXIT;
		return (-1);
	}

	/**Commented in Ver 1.3
	if ( strcmp ( c_trg_msg, "TRG_LOOK_TAB" ) == 0 )
		{
			i_look_tab = TRUE;

			i_ch_val = tpunsubscribe ( li_look_tab_hnd, 0 );

			if ( i_ch_val == -1 )
			{
				fn_errlog(c_ServiceName,"S31225", TPMSG, c_err_msg);
			}
		}
		else **/

	if (strcmp(c_trg_msg, "TRG_ORD_OPN") == 0)
	{
		i_ord_mkt_stts = TRUE;
	}
	else if (strcmp(c_trg_msg, "TRG_ORD_CLS") == 0)
	{
		i_ord_mkt_stts = FALSE;
	}
	else if (strcmp(c_trg_msg, "TRG_EXT_OPN") == 0)
	{
		i_ext_mkt_stts = TRUE;
	}
	else if (strcmp(c_trg_msg, "TRG_EXT_CLS") == 0)
	{
		i_ext_mkt_stts = FALSE;
	}
	else if (strcmp(c_trg_msg, "TRG_EXR_OPN") == 0)
	{
		i_exr_mkt_stts = TRUE;
	}
	else if (strcmp(c_trg_msg, "TRG_EXR_CLS") == 0)
	{
		i_exr_mkt_stts = FALSE;
	}
	else if (strcmp(c_trg_msg, "TRG_STRT_PMP") == 0)
	{
		/***	Ver 1.5 Starts	***/

		EXEC SQL
			SELECT exg_crrnt_stts,
			exg_exrc_mkt_stts,
			exg_extnd_mrkt_stts
				INTO : sql_xchng_crrnt_stts,
			: sql_xchng_exrc_mkt_stts,
			: sql_xchng_extnd_mrkt_stts
				  FROM exg_xchng_mstr
					  WHERE exg_xchng_cd = : sql_c_xchng_cd;

		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "S31230", SQLMSG, c_err_msg);
			return (-1);
		}

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "Before Forwardnig normal market status is :%c:", sql_xchng_crrnt_stts);
			fn_userlog(c_ServiceName, "Before Forwardnig extended market status is :%c:", sql_xchng_extnd_mrkt_stts);
			fn_userlog(c_ServiceName, "Before Forwardnig exercise market status is :%c:", sql_xchng_exrc_mkt_stts);
			fn_userlog(c_ServiceName, "Before Forwarding Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:i_ext_mkt_stts:%d:", i_ord_mkt_stts, i_exr_mkt_stts, i_ext_mkt_stts);
		}

		if (sql_xchng_crrnt_stts == EXCHANGE_OPEN)
		{
			i_ord_mkt_stts = TRUE;
		}
		if (sql_xchng_exrc_mkt_stts == EXCHANGE_OPEN)
		{
			i_exr_mkt_stts = TRUE;
		}
		if (sql_xchng_extnd_mrkt_stts == EXCHANGE_OPEN)
		{
			i_ext_mkt_stts = TRUE;
		}

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "After Forwarding Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:i_ext_mkt_stts:%d:", i_ord_mkt_stts, i_exr_mkt_stts, i_ext_mkt_stts);
		}

		/***	Ver 1.5 Ends ***/

		i_pmp_stts = TRUE;
		i_upd_stts = TRUE;
	}
	else if (strcmp(c_trg_msg, "TRG_STOP_PMP") == 0)
	{
		i_pmp_stts = FALSE;
		i_upd_stts = TRUE;
	}
	else if (strcmp(c_trg_msg, "TRG_RECO_OVER") == 0)
	{
		i_reco_stts = 0;
		fn_userlog(c_ServiceName, "After i_reco_stts = 0");
	}
	else if (strcmp(c_trg_msg, "TRG_PART_SUS") == 0)
	{
		i_part_stts = FALSE;
	}
	else if (strcmp(c_trg_msg, "TRG_PART_ACT") == 0)
	{
		i_part_stts = TRUE;
	}
	else if (strcmp(c_trg_msg, "TRG_BRKR_SUS") == 0)
	{
		i_brkr_stts = FALSE;
	}
	else if (strcmp(c_trg_msg, "TRG_BRKR_ACT") == 0)
	{
		i_brkr_stts = TRUE;
	}

	return (0);
}

int fn_rjct_rcrd(struct vw_xchngbook *ptr_st_rqst, struct vw_orderbook *ptr_st_ord, char *c_ServiceName, char *c_err_msg)
{
	char c_svc_nm[16];
	int i_ch_val;
	EXEC SQL BEGIN DECLARE SECTION;
	varchar c_tm_stmp[LEN_DATE];
	EXEC SQL END DECLARE SECTION;

	/******** set the acknowledge service name 	based on product type ****/
	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Inside function reject record");
		fn_userlog(c_ServiceName, "Product type is %c", ptr_st_ord->c_prd_typ);
	}

	if (ptr_st_ord->c_prd_typ == FUTURES)
	{
		strcpy(c_svc_nm, "SFO_FUT_ACK");
	}
	else
	{
		strcpy(c_svc_nm, "SFO_OPT_ACK");
	}

	EXEC SQL
		SELECT to_char(sysdate, 'dd-mon-yyyy hh24:mi:ss')
			INTO : c_tm_stmp
					   FROM dual;
	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31235", SQLMSG, c_err_msg);
		return -1;
	}
	SETNULL(c_tm_stmp);

	/* Populate the structure to update the database as thr record is rejected */
	ptr_st_rqst->c_plcd_stts = REJECT;
	ptr_st_rqst->c_rms_prcsd_flg = NOT_PROCESSED;
	ptr_st_rqst->l_ors_msg_typ = ORS_NEW_ORD_RJCT;
	strcpy(ptr_st_rqst->c_ack_tm, (char *)c_tm_stmp.arr);
	strcpy(ptr_st_rqst->c_xchng_rmrks, "Token id not available");
	ptr_st_rqst->d_jiffy = 0;
	ptr_st_rqst->c_oprn_typ = UPDATION_ON_EXCHANGE_RESPONSE;
	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Before calling SFO_UPD_XCHNGBK for rejecting record ");
	}

	/****************************************************************
			fn_cpy_ddr(ptr_st_rqst->c_rout_crt);
			i_ch_val = fn_call_svc (c_ServiceName,
															c_err_msg,
															ptr_st_rqst,
															ptr_st_rqst,
															"vw_xchngbook",
															"vw_xchngbook",
															sizeof ( struct vw_xchngbook ),
															sizeof ( struct vw_xchngbook ),
															0,
															"SFO_UPD_XCHNGBK" );
	****************************************************************/
	i_ch_val = fn_upd_xchngbk(ptr_st_rqst, c_ServiceName, c_err_msg);

	if (i_ch_val != 0)
	{
		fn_errlog(c_ServiceName, "S31240", LIBMSG, c_err_msg);
		return -1;
	}
	if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
	{
		fn_gettime();
		fn_userlog(c_ServiceName, "TIME_STATS Before calling UPD_XCHNGBK order reject case :%s:", c_time);
	} /* Ver 1.2 Ends */
	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "After calling SFO_UPD_XCHNGBK for rejecting record ");
	}

	fn_cpy_ddr(ptr_st_rqst->c_rout_crt);

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Before calling %s for rejecting record ", c_svc_nm);
	}

	i_ch_val = fn_call_svc(c_ServiceName,
						   c_err_msg,
						   ptr_st_rqst,
						   ptr_st_rqst,
						   "vw_xchngbook",
						   "vw_xchngbook",
						   sizeof(struct vw_xchngbook),
						   sizeof(struct vw_xchngbook),
						   0,
						   c_svc_nm);

	if (i_ch_val != SUCC_BFR)
	{
		fn_errlog(c_ServiceName, "S31245", LIBMSG, c_err_msg);
		return -1;
	}
	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "After calling %s for rejecting record ", c_svc_nm);
		fn_userlog(c_ServiceName, "Order rejection succesfull");
	}
	return 0;
}

/********************************* Function get next record****************************************/

int fn_get_nxt_rec(char *c_ServiceName, char *c_err_msg)
{
	int i_ch_val;
	int i_sprd_flg;
	int i_tmp;
	int i_counter;

	char c_prgm_flg = '\0';	  
	char c_pan_no[11];		  
	char c_lst_act_ref[22];	  
	char c_esp_id[51];		  
	char c_algo_id[51];		  
	char c_source_flg = '\0'; 

	EXEC SQL BEGIN DECLARE SECTION; /* Ver 1.2 Starts */
	// struct vw_spdordbk st_spd_ord_bk; //it wil not be used
	struct vw_sequence st_seq;
	struct vw_xchngbook st_rqst[3], st_rqst1;
	struct vw_orderbook st_ord[3], st_ord1;
	// struct vw_exrcbook st_exr;
	struct vw_nse_cntrct st_nse_cnt[3], st_nse_cnt1;
	struct vw_contract st_cnt;

	long li_count;
	int i_tot_cnt = 0;
	EXEC SQL END DECLARE SECTION; /* Ver 1.2 Ends */

	/*** Ver 3.5 Initialisation starts***/
	MEMSET(c_ordr_rfrnc);
	//MEMSET(c_sprd_ord_ind);// it will not be used
	/*** Ver 3.5 Initialisation ends ***/
	MEMSET(c_pan_no);	   /** Ver 3.6 **/
	MEMSET(c_lst_act_ref); /** Ver 3.8 **/
	MEMSET(c_esp_id);	   /** Ver 4.1 **/
	MEMSET(c_algo_id);	   /** Ver 4.1 **/
	if ((li_cnt_q >= li_max_q) || (i_queue_chk_flg == 1))
	{
		EXEC SQL
			Select
			count(*)
				into : li_count
						   From fxb_fo_xchng_book
							   Where fxb_mod_trd_dt = : sql_c_nxt_trd_date
															and fxb_xchng_cd = : sql_c_xchng_cd
																					 and fxb_pipe_id = : sql_c_pipe_id
																											 and fxb_plcd_stts = 'Q';
		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "S31250", SQLMSG, c_err_msg);
			return ERROR;
		}

		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Pipe id :%s: q chk cnt :%ld: prev_q_flg = :%d: ord queue count :%ld:", sql_c_pipe_id, li_cnt_q, i_queue_chk_flg, li_count);
		}

		if (li_count >= sql_li_max_pnd_ord)
		{
			i_queue_chk_flg = 1;
			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "Num of Ords in queued stts[%ld] exceeds max allwd[%ld] for pipe :%s:", li_count, sql_li_max_pnd_ord, sql_c_pipe_id);
			}
			return OVER_LOAD;
		}

		li_cnt_q = 0;
		i_queue_chk_flg = 0;
	}

	/**** Get the exchange book record ****/
	strcpy(st_rqst1.c_xchng_cd, sql_c_xchng_cd);

	strcpy(st_rqst1.c_pipe_id, sql_c_pipe_id);

	st_rqst1.l_ord_seq = li_seq_nm;  // set to '0'  in fn_bat_init.

	strcpy(st_rqst1.c_mod_trd_dt, (char *)sql_c_nxt_trd_date.arr); 

	if ((i_ord_mkt_stts == FALSE) && (i_exr_mkt_stts == FALSE) && (i_ext_mkt_stts == TRUE))
	{
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "In Extended Market selection");
			fn_userlog(c_ServiceName, "Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:i_ext_mkt_stts:%d:",
					   i_ord_mkt_stts, i_exr_mkt_stts, i_ext_mkt_stts);
		}

		st_rqst1.c_oprn_typ = FOR_EXTEND;
	}
	else if ((i_ord_mkt_stts == FALSE) && (i_exr_mkt_stts == TRUE) && (i_ext_mkt_stts == FALSE))
	{
		/**Ver 1.4 **/
		st_rqst1.c_oprn_typ = FOR_EXER;
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "In Normal Market selection");
			fn_userlog(c_ServiceName, "Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:i_ext_mkt_stts:%d:",
					   i_ord_mkt_stts, i_exr_mkt_stts, i_ext_mkt_stts);
		}
	}
	else
	{
		st_rqst1.c_oprn_typ = FOR_NORM;
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "In Normal Market selection");
			fn_userlog(c_ServiceName, "Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:i_ext_mkt_stts:%d:",
					   i_ord_mkt_stts, i_exr_mkt_stts, i_ext_mkt_stts);
		}
	}

	/* Ver 1.2 Starts */

	/****************************************************************

		fn_cpy_ddr(st_rqst1.c_rout_crt);

		i_ch_val = fn_call_svc (c_ServiceName,
														c_err_msg,
														&st_rqst1,
														&st_rqst1,
														"vw_xchngbook",
														"vw_xchngbook",
														sizeof ( st_rqst1 ),
														sizeof ( st_rqst1 ),
														0,
														"SFO_SEQ_TO_OMD" );
	*******************************************************************/
	if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
	{
		fn_gettime();
		fn_userlog(c_ServiceName, "TIME_STATS Before calling SEQ_TO_OMD :%s:", c_time);
	} /* Ver 1.2 Ends */

	i_ch_val = fn_seq_to_omd(&st_rqst1, c_ServiceName, c_err_msg, &c_prgm_flg); /*** ver 2.7 **/
	// in above function we are retrieving data from the xchng_book

	/*  If condition added in Ver 3.5 */
	if (i_ch_val == NULL_CASE)
	{
		fn_userlog(c_ServiceName, "In a case of SKIP_ORD for fn_seq_to_omd 1");
		return SKIP_ORD;
	}
	/*** Ver 3.5 ended ***/
	if ((i_ch_val != 0) && (i_ch_val != NO_DATA_FOUND))
	{
		fn_errlog(c_ServiceName, "S31255", LIBMSG, c_err_msg);
		return ERROR;
	} /* Ver 1.2 Ends */

	/**** No record found ****/
	if (i_ch_val == NO_DATA_FOUND)
	{
		return NDF;
	}
	if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
	{
		fn_gettime();
		fn_userlog(c_ServiceName, "TIME_STATS After calling SEQ_TO_OMD :%s:", c_time);
	} /* Ver 1.2 Ends */

	/** Ver 4.4 Starts here ***/
	if (st_rqst1.c_rms_prcsd_flg == 'P')
	{
		if (DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName, "Order ref / Mod number = |%s| / |%ld| Already Processed", st_rqst1.c_ordr_rfrnc, st_rqst1.l_mdfctn_cntr);
		}
		c_sprd_ord_ind = st_rqst1.c_spl_flg;
		strcpy(c_ordr_rfrnc, st_rqst1.c_ordr_rfrnc);
		return SKIP_ORD;
	}
	/** Ver 4.4 Ends here ***/

	if (DEBUG_MSG_LVL_0)
	{
		fn_userlog(c_ServiceName, "Order ref / Mod number = |%s| / |%ld|", st_rqst1.c_ordr_rfrnc,
				   st_rqst1.l_mdfctn_cntr);
	}

	if (st_rqst1.c_plcd_stts != REQUESTED)
	{
		return DATA_CNCLD;
	}

	//************************ 
	if ((st_rqst1.c_sprd_ord_ind == SPREAD_ORDER) ||
		(st_rqst1.c_sprd_ord_ind == L2_ORDER) ||
		(st_rqst1.c_sprd_ord_ind == L3_ORDER))
	{
		EXEC SQL
			SELECT FXB_ORDR_SQNC
				into : li_seq_nm
						   FROM FSD_FO_SPRD_DTLS,
					   FXB_FO_XCHNG_BOOK
						   WHERE FXB_ORDR_RFRNC = FSD_ORDR_RFRNC
							   AND FSD_SPRD_RFRNC = (SELECT FSD_SPRD_RFRNC FROM FSD_FO_SPRD_DTLS
														 WHERE FSD_ORDR_RFRNC = : st_rqst1.c_ordr_rfrnc)
														AND FSD_ORDR_RFRNC<> : st_rqst1.c_ordr_rfrnc
																				   AND FXB_PLCD_STTS = 'R' /*** Ver 3.7 ***/
																									   AND rownum < 2;

		/**Rownum condition added in order to avoid Mult. Rows Fetched error **/

		if (SQLCODE != 0 && SQLCODE != NO_DATA_FOUND)
		{
			fn_errlog(c_ServiceName, "S31260", SQLMSG, c_err_msg);
			return ERROR;
		}

		/**** Get the next request record ****/
		strcpy(st_rqst[1].c_xchng_cd, sql_c_xchng_cd);
		strcpy(st_rqst[1].c_pipe_id, sql_c_pipe_id);
		st_rqst[1].l_ord_seq = li_seq_nm;
		strcpy(st_rqst[1].c_mod_trd_dt, (char *)sql_c_nxt_trd_date.arr);
		st_rqst[1].c_oprn_typ = FOR_SPRD;
		/* Ver 1.2 Starts */
		/****************************************************************

				fn_cpy_ddr(st_rqst[1].c_rout_crt);

				i_ch_val = fn_call_svc(	c_ServiceName,
																c_err_msg,
																&st_rqst[1],
																&st_rqst[1],
																"vw_xchngbook",
																"vw_xchngbook",
																sizeof ( struct vw_xchngbook ),
																sizeof ( struct vw_xchngbook ),
																0,
																"SFO_SEQ_TO_OMD" );

		****************************************************************/
		c_prgm_flg = '\0';

		i_ch_val = fn_seq_to_omd(&st_rqst[1], c_ServiceName, c_err_msg, &c_prgm_flg); /*** ver 2.7 **/

		/*  If condition added in Ver 3.5 */
		if (i_ch_val == NULL_CASE)
		{
			fn_userlog(c_ServiceName, "In a case of SKIP_ORD for fn_seq_to_omd 2");
			return SKIP_ORD;
		}
		/*** Ver 3.5 ended ***/
		if (i_ch_val != 0) /* Ver 1.2 Ends */
		{
			fn_errlog(c_ServiceName, "S31265", LIBMSG, c_err_msg);
			return ERROR;
		}
		if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
		{
			fn_gettime();
			fn_userlog(c_ServiceName, "TIME_STATS After calling SEQ_TO_OMD L2 :%s:", c_time);
		} /* Ver 1.2 Ends */

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "Spread order / 2L");
			fn_userlog(c_ServiceName, "Order ref / Mod number = |%s| / |%ld|", st_rqst[1].c_ordr_rfrnc,
					   st_rqst[1].l_mdfctn_cntr);
		}
	}
	if (st_rqst1.c_sprd_ord_ind == L3_ORDER)
	{
		EXEC SQL
			SELECT FXB_ORDR_SQNC
				into : li_seq_nm
						   FROM FSD_FO_SPRD_DTLS,
					   FXB_FO_XCHNG_BOOK
						   WHERE FXB_ORDR_RFRNC = FSD_ORDR_RFRNC
							   AND FSD_SPRD_RFRNC = (SELECT FSD_SPRD_RFRNC FROM FSD_FO_SPRD_DTLS
														 WHERE FSD_ORDR_RFRNC = : st_rqst1.c_ordr_rfrnc)
								   AND FXB_PLCD_STTS = 'R' /*** Ver 3.7 ***/
					   AND FSD_ORDR_RFRNC NOT IN( : st_rqst1.c_ordr_rfrnc, : st_rqst[1].c_ordr_rfrnc);

		if (SQLCODE != 0 && SQLCODE != NO_DATA_FOUND)
		{
			fn_errlog(c_ServiceName, "S31270", SQLMSG, c_err_msg);
			return ERROR;
		}
		/******* Get the next request record. **************/
		strcpy(st_rqst[2].c_xchng_cd, sql_c_xchng_cd);
		strcpy(st_rqst[2].c_pipe_id, sql_c_pipe_id);
		st_rqst[2].l_ord_seq = li_seq_nm;
		strcpy(st_rqst[2].c_mod_trd_dt, (char *)sql_c_nxt_trd_date.arr);
		st_rqst[2].c_oprn_typ = FOR_SPRD;
		/* Ver 1.2 Starts */
		/****************************************************************


				fn_cpy_ddr(st_rqst[2].c_rout_crt);

				i_ch_val = fn_call_svc (c_ServiceName,
																c_err_msg,
																&st_rqst[2],
																&st_rqst[2],
																"vw_xchngbook",
																"vw_xchngbook",
																sizeof ( struct vw_xchngbook ),
																sizeof ( struct vw_xchngbook ),
																0,
																"SFO_SEQ_TO_OMD" );
		****************************************************************/
		c_prgm_flg = '\0';

		i_ch_val = fn_seq_to_omd(&st_rqst[2], c_ServiceName, c_err_msg, &c_prgm_flg); /*** ver 2.7 **/

		/*  If condition added in Ver 3.5 */
		if (i_ch_val == NULL_CASE)
		{
			fn_userlog(c_ServiceName, "In a case of SKIP_ORD for fn_seq_to_omd 3");
			return SKIP_ORD;
		}
		/*** Ver 3.5 ended ***/
		if (i_ch_val != 0) /* Ver 1.2 Ends */
		{
			fn_errlog(c_ServiceName, "S31275", LIBMSG, c_err_msg);
			return ERROR;
		}
		if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
		{
			fn_gettime();
			fn_userlog(c_ServiceName, "TIME_STATS After calling SEQ_TO_OMD L3 :%s:", c_time);
		} /* Ver 1.2 Ends */

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "3L");
			fn_userlog(c_ServiceName, "Order ref / Mod number = |%s| / |%ld|", st_rqst[2].c_ordr_rfrnc,
					   st_rqst[2].l_mdfctn_cntr);
		}
	}


	/**** Based on Order/Exercise request pickup the record from order and ****/
	/**** exercise book ****/
	switch (st_rqst1.c_ex_ordr_typ)

	{
	case ORDINARY_ORDER:

		if (i_ord_mkt_stts == FALSE && i_ext_mkt_stts == FALSE)

		{
			return DATA_SKPD;
		}

		/**** Get order book record ****/
		strcpy(st_ord1.c_ordr_rfrnc, st_rqst1.c_ordr_rfrnc);
		st_ord1.c_oprn_typ = FOR_SNDCLNT;
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Calling SFO_REF_TO_ORD for normal order");
		}
		/* Ver 1.2 Starts */
		/****************************************************************
					 fn_cpy_ddr(st_ord1.c_rout_crt);
					i_ch_val = fn_call_svc (c_ServiceName,
																	c_err_msg,
																	&st_ord1,
																	&st_ord1,
																	"vw_orderbook",
																	"vw_orderbook",
																	sizeof ( st_ord1 ),
																	sizeof ( st_ord1 ),
																	0,
																	"SFO_REF_TO_ORD" );
		****************************************************************/
		/** i_ch_val = fn_ref_to_ord(&st_ord1,c_ServiceName,c_err_msg); ** Commented in Ver 3.6 **/
		/** i_ch_val = fn_ref_to_ord(&st_ord1,c_pan_no,c_ServiceName,c_err_msg); *** Ver 3.6 **Commented  in Ver  3.8 ****/
		/** i_ch_val = fn_ref_to_ord(&st_ord1,c_pan_no,c_lst_act_ref,c_ServiceName,c_err_msg); *** Ver 3.8 *** Commented in Ver 4.1 */
		i_ch_val = fn_ref_to_ord(&st_ord1, c_pan_no, c_lst_act_ref, c_esp_id, c_algo_id, &c_source_flg, c_ServiceName, c_err_msg); /*** Ver 4.1 ***/ /** Ver 4.2 c_source_flg**/
																																					 /*  If condition added in Ver 3.5 */
		if (i_ch_val == NULL_CASE)
		{
			fn_userlog(c_ServiceName, "In a case of SKIP_ORD for fn_ref_to_ord 1");
			return SKIP_ORD;
		}
		/*** Ver 3.5 ended ***/
		if (i_ch_val != 0)
		{
			fn_errlog(c_ServiceName, "S31280", LIBMSG, c_err_msg);
			if (i_ch_val == LOCK) /* Ver 1.2 Ends */
			{
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "DISTRIBUTION lOCK ERROR");
				}
				return LOCK_ERR;
			}
			else
			{
				return ERROR;
			}
		}
		if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
		{
			fn_gettime();
			fn_userlog(c_ServiceName, "TIME_STATS After calling REF_TO_ORD :%s:", c_time);
		} /* Ver 1.2 Ends */
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "After Calling SFO_REF_TO_ORD for normal order");

			fn_userlog(c_ServiceName, " ORDINARY_ORDER CTCL ID |%s|", st_ord1.c_ctcl_id);
		}

		if (st_ord1.l_mdfctn_cntr != st_rqst1.l_mdfctn_cntr)
		{
			return DATA_CNCLD;
		}

		st_rqst1.c_plcd_stts = QUEUED;
		st_rqst1.c_oprn_typ = UPDATION_ON_ORDER_FORWARDING;
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Calling SFO_UPD_XCHNGBK for normal order");
		}

		/* Ver 1.2 Starts */
		/****************************************************************


					fn_cpy_ddr(st_rqst1.c_rout_crt);
					i_ch_val = fn_call_svc (c_ServiceName,
																	c_err_msg,
																	&st_rqst1,
																	&st_rqst1,
																	"vw_xchngbook",
																	"vw_xchngbook",
																	sizeof ( st_rqst1 ),
																	sizeof ( st_rqst1 ),
																	0,
																	"SFO_UPD_XCHNGBK" );
		****************************************************************/
		i_ch_val = fn_upd_xchngbk(&st_rqst1, c_ServiceName, c_err_msg);

		if (i_ch_val != 0)
		{
			fn_errlog(c_ServiceName, "S31285", LIBMSG, c_err_msg);
			if (i_ch_val == LOCK) /* Ver 1.2 Ends */
			{
				return LOCK_ERR;
			}
			else
			{
				return ERROR;
			}
		}
		if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
		{
			fn_gettime();
			fn_userlog(c_ServiceName, "TIME_STATS After calling UPD_XCHNGBK :%s:", c_time);
		} /* Ver 1.2 Ends */
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "After Calling SFO_UPD_XCHNGBK for normal order");
		}
		
		st_ord1.c_ordr_stts = QUEUED;
		st_ord1.c_oprn_typ = UPDATE_ORDER_STATUS;
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Calling SFO_UPD_ORDRBK for normal order");
		}

		/* Ver 1.2 Starts */
		/****************************************************************

					fn_cpy_ddr(st_ord1.c_rout_crt);
					i_ch_val = fn_call_svc (c_ServiceName,
																	c_err_msg,
																	&st_ord1,
																	&st_ord1,
																	"vw_orderbook",
																	"vw_orderbook",
																	sizeof ( st_ord1 ),
																	sizeof ( st_ord1 ),
																	0,
																	"SFO_UPD_ORDRBK" );
		****************************************************************/
		i_ch_val = fn_upd_ordrbk(&st_ord1, c_ServiceName, c_err_msg);
		if (i_ch_val != 0) /* Ver 1.2 Ends */
		{
			fn_errlog(c_ServiceName, "S31290", LIBMSG, c_err_msg);
			return ERROR;
		}
		if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
		{
			fn_gettime();
			fn_userlog(c_ServiceName, "TIME_STATS After calling UPD_ORDRBK :%s:", c_time);
		} /* Ver 1.2 Ends */

		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "After Calling SFO_UPD_ORDRBK for normal order");
		}

		/**** Convert the contract information to exchange format ****/
		strcpy(st_cnt.c_xchng_cd, st_ord1.c_xchng_cd);
		st_cnt.c_prd_typ = st_ord1.c_prd_typ;
		strcpy(st_cnt.c_undrlyng, st_ord1.c_undrlyng);
		strcpy(st_cnt.c_expry_dt, st_ord1.c_expry_dt);
		st_cnt.c_exrc_typ = st_ord1.c_exrc_typ;
		st_cnt.c_opt_typ = st_ord1.c_opt_typ;
		st_cnt.l_strike_prc = st_ord1.l_strike_prc;
		st_cnt.c_ctgry_indstk = st_ord1.c_ctgry_indstk;
		st_cnt.l_ca_lvl = st_ord1.l_ca_lvl;
		
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Converting conttract information to exchange format");
		}

		if (strcmp(sql_c_xchng_cd, "NFO") == 0)
		{
			st_cnt.c_rqst_typ = CONTRACT_TO_NSE_ID;
			/* Ver 1.2 Starts */
			/****************************************************************
						 fn_cpy_ddr(st_cnt.c_rout_crt);
						i_ch_val = fn_call_svc (c_ServiceName,
																		c_err_msg,
																		&st_cnt,
																		&st_nse_cnt1,
																		"vw_contract",
																		"vw_nse_cntrct",
																		sizeof ( st_cnt ),
																		sizeof ( st_nse_cnt1 ),
																		0,
																		"SFO_GT_EXT_CNT" );
			****************************************************************/
			i_ch_val = fn_get_ext_cnt(&st_cnt,
									  &st_nse_cnt1,
									  c_ServiceName,
									  c_err_msg);
			if (i_ch_val != 0) /* Ver 1.2 Ends */
			{
				fn_errlog(c_ServiceName, "S31295", LIBMSG, c_err_msg);
				return ERROR;
			}
			if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
			{
				fn_gettime();
				fn_userlog(c_ServiceName, "TIME_STATS After calling GT_EXT_CNT :%s:", c_time);
			} /* Ver 1.2 Ends */
		}
		else
		{
			/**** Code for BSE ****/
			return ERROR;
		}

	//=============================^&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "After Converting contract information to exchange format");
		}
		/**** If spread order ****/
		if ((st_rqst1.c_sprd_ord_ind == SPREAD_ORDER) ||
			(st_rqst1.c_sprd_ord_ind == L2_ORDER) ||
			(st_rqst1.c_sprd_ord_ind == L3_ORDER))
		{
			/**** Get next order book record ****/
			strcpy(st_ord[1].c_ordr_rfrnc, st_rqst[1].c_ordr_rfrnc);
			st_ord[1].c_oprn_typ = FOR_SNDCLNT;
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "Calling REF_TO_ORD for 2L order ");
			}
			/* Ver 1.2 Starts */
			/****************************************************************
						 fn_cpy_ddr(st_ord[1].c_rout_crt);
						i_ch_val = fn_call_svc (c_ServiceName,
																		c_err_msg,
																		&st_ord[1],
																		&st_ord[1],
																		"vw_orderbook",
																		"vw_orderbook",
																		sizeof ( st_ord[1] ),
																		sizeof ( st_ord[1] ),
																		0,
																		"SFO_REF_TO_ORD" );
			****************************************************************/
			/** i_ch_val = fn_ref_to_ord(&st_ord[1],c_ServiceName,c_err_msg); ** Commented in Ver 3.6 **/
			/**	i_ch_val = fn_ref_to_ord(&st_ord[1],c_pan_no,c_ServiceName,c_err_msg);*** Ver 3.6 ***Commented in Ver 3.8 **/
			/** i_ch_val = fn_ref_to_ord(&st_ord[1],c_pan_no,c_lst_act_ref,c_ServiceName,c_err_msg); ** Ver 3.8 *** Commented in Ver 4.1*/
			i_ch_val = fn_ref_to_ord(&st_ord[1], c_pan_no, c_lst_act_ref, c_esp_id, c_algo_id, &c_source_flg, c_ServiceName, c_err_msg); /** Ver 4.1 **/ /* Ver 4.2 c_source_flg*/

			/*  If condition added in Ver 3.5 */
			if (i_ch_val == NULL_CASE)
			{
				fn_userlog(c_ServiceName, "In a case of SKIP_ORD for fn_ref_to_ord 2");
				return SKIP_ORD;
			}
			/*** Ver 3.5 ended ***/
			if (i_ch_val != 0)
			{
				fn_errlog(c_ServiceName, "S31300", LIBMSG, c_err_msg);
				if (i_ch_val == LOCK) /* Ver 1.2 Ends */
				{
					return LOCK_ERR;
				}
				else
				{
					return ERROR;
				}
			}
			if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
			{
				fn_gettime();
				fn_userlog(c_ServiceName, "TIME_STATS After calling REF_TO_ORD L2:%s:", c_time);
			} /* Ver 1.2 Ends */
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "After Calling REF_TO_ORD for 2L order ");
			}

			st_spd_ord_bk.c_rqst_typ[0] = ORDREF_TO_SPDREF;
			strcpy(st_spd_ord_bk.c_ordr_rfrnc[0], st_ord[1].c_ordr_rfrnc);
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "Calling SFO_QRY_SPDBK for 2L order ");
			}
			/* Ver 1.2 Starts */
			/****************************************************************
							fn_cpy_ddr(st_spd_ord_bk.c_rout_crt);
							i_ch_val = fn_call_svc (c_ServiceName,
																			c_err_msg,
																			&st_spd_ord_bk,
																			&st_spd_ord_bk,
																			"vw_spdordbk",
																			"vw_spdordbk",
																			sizeof ( st_spd_ord_bk ),
																			sizeof ( st_spd_ord_bk ),
																			0,
																			"SFO_QRY_SPDBK" );
			****************************************************************/
			i_ch_val = fn_qry_spdbk(&st_spd_ord_bk, c_ServiceName, c_err_msg);

			if (i_ch_val != SUCC_BFR) /* Ver 1.2 Ends */
			{
				fn_errlog(c_ServiceName, "S31305", LIBMSG, c_err_msg);
				return ERROR;
			}
			if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
			{
				fn_gettime();
				fn_userlog(c_ServiceName, "TIME_STATS After calling QRY_SPDBK L2:%s:", c_time);
			} /* Ver 1.2 Ends */
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "After Calling SFO_QRY_SPDBK FOR 2L order ");
			}

			st_rqst[1].c_plcd_stts = QUEUED;
			st_rqst[1].c_oprn_typ = UPDATION_ON_ORDER_FORWARDING;
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "Calling SFO_UPD_XCHNGBK for 2L order ");
			}
			/* Ver 1.2 Starts */
			/****************************************************************
							fn_cpy_ddr(st_rqst[1].c_rout_crt);
							i_ch_val = fn_call_svc (c_ServiceName,
																			c_err_msg,
																			&st_rqst[1],
																			&st_rqst[1],
																			"vw_xchngbook",
																			"vw_xchngbook",
																			sizeof ( st_rqst[1] ),
																			sizeof ( st_rqst[1] ),
																			0,
																			"SFO_UPD_XCHNGBK" );
			****************************************************************/
			i_ch_val = fn_upd_xchngbk(&st_rqst[1], c_ServiceName, c_err_msg);

			if (i_ch_val != 0)
			{
				fn_errlog(c_ServiceName, "S31310", LIBMSG, c_err_msg);
				if (i_ch_val == LOCK) /* Ver 1.2 Ends */
				{
					return LOCK_ERR;
				}
				else
				{
					return ERROR;
				}
			}
			if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
			{
				fn_gettime();
				fn_userlog(c_ServiceName, "TIME_STATS After calling UPD_XCHNGBK L2:%s:", c_time);
			} /* Ver 1.2 Ends */
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "After Calling SFO_UPD_XCHNGBK for 2L order ");
			}

			st_ord[1].c_ordr_stts = QUEUED;
			st_ord[1].c_oprn_typ = UPDATE_ORDER_STATUS;
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "Calling SFO_UPD_ORDRBK for 2L order ");
			}
			/* Ver 1.2 Starts */
			/****************************************************************
							fn_cpy_ddr(st_ord[1].c_rout_crt);
							i_ch_val = fn_call_svc (c_ServiceName,
																			c_err_msg,
																			&st_ord[1],
																			&st_ord[1],
																			"vw_orderbook",
																			"vw_orderbook",
																			sizeof ( st_ord[1] ),
																			sizeof ( st_ord[1] ),
																			0,
																			"SFO_UPD_ORDRBK" );
			****************************************************************/
			i_ch_val = fn_upd_ordrbk(&st_ord[1], c_ServiceName, c_err_msg);
			if (i_ch_val != 0) /* Ver 1.2 Ends */
			{
				fn_errlog(c_ServiceName, "S31315", LIBMSG, c_err_msg);
				return ERROR;
			}
			if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
			{
				fn_gettime();
				fn_userlog(c_ServiceName, "TIME_STATS After calling UPD_ORDRBK L2:%s:", c_time);
			} /* Ver 1.2 Ends */
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "After Calling SFO_UPD_ORDRBK for 2L order ");
			}

			/**** Convert the contract information to exchange format ****/
			strcpy(st_cnt.c_xchng_cd, st_ord[1].c_xchng_cd);
			st_cnt.c_prd_typ = st_ord[1].c_prd_typ;
			strcpy(st_cnt.c_undrlyng, st_ord[1].c_undrlyng);
			strcpy(st_cnt.c_expry_dt, st_ord[1].c_expry_dt);
			st_cnt.c_exrc_typ = st_ord[1].c_exrc_typ;
			st_cnt.c_opt_typ = st_ord[1].c_opt_typ;
			st_cnt.l_strike_prc = st_ord[1].l_strike_prc;
			st_cnt.c_ctgry_indstk = st_ord[1].c_ctgry_indstk;
			st_cnt.l_ca_lvl = st_ord[1].l_ca_lvl;

			if (strcmp(sql_c_xchng_cd, "NFO") == 0)
			{
				st_cnt.c_rqst_typ = CONTRACT_TO_NSE_ID;
				if (DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName, "Calling SFO_GT_EXT_CNT for 2L order ");
				}
				/* Ver 1.2 Starts */
				/****************************************************************
								fn_cpy_ddr(st_cnt.c_rout_crt);
								i_ch_val = fn_call_svc (c_ServiceName,
																				c_err_msg,
																				&st_cnt,
																				&st_nse_cnt[1],
																				"vw_contract",
																				"vw_nse_cntrct",
																				sizeof ( st_cnt ),
																				sizeof ( st_nse_cnt[1] ),
																				0,
																				"SFO_GT_EXT_CNT" );
				****************************************************************/
				i_ch_val = fn_get_ext_cnt(&st_cnt,
										  &st_nse_cnt[1],
										  c_ServiceName,
										  c_err_msg);

				if (i_ch_val != 0) /* Ver 1.2 Ends */
				{
					fn_errlog(c_ServiceName, "S31320", LIBMSG, c_err_msg);
					return ERROR;
				}
				if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
				{
					fn_gettime();
					fn_userlog(c_ServiceName, "TIME_STATS After calling GT_EXT_CNT L2:%s:", c_time);
				} /* Ver 1.2 Ends */
				if (DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName, "After Calling SFO_GT_EXT_CNT for 2L order ");
				}
			}
			else
			{
				/**** Code for BSE ****/
				return ERROR;
			}
		}

		/******** if 3L Order *********/
		if (st_rqst1.c_sprd_ord_ind == L3_ORDER)
		{
			/*** Get next order book record   */
			strcpy(st_ord[2].c_ordr_rfrnc, st_rqst[2].c_ordr_rfrnc);
			st_ord[2].c_oprn_typ = FOR_SNDCLNT;
			/* Ver 1.2 Starts */
			/****************************************************************
					 fn_cpy_ddr(st_ord[2].c_rout_crt);
					i_ch_val = fn_call_svc (c_ServiceName,
																	c_err_msg,
																	&st_ord[2],
																	&st_ord[2],
																	"vw_orderbook",
																	"vw_orderbook",
																	sizeof ( st_ord[2] ),
																	sizeof ( st_ord[2] ),
																	0,
																	"SFO_REF_TO_ORD" );
			****************************************************************/
			/** i_ch_val = fn_ref_to_ord(&st_ord[2],c_ServiceName,c_err_msg); ** Commented in Ver 3.6 **/
			/** i_ch_val = fn_ref_to_ord(&st_ord[2],c_pan_no,c_ServiceName,c_err_msg);*** Ver 3.6 ***Commented in Ver 3.8 **/
			/**i_ch_val = fn_ref_to_ord(&st_ord[2],c_pan_no,c_lst_act_ref,c_ServiceName,c_err_msg); ** Ver 3.8 ***Commented in 4.1 **/
			i_ch_val = fn_ref_to_ord(&st_ord[2], c_pan_no, c_lst_act_ref, c_esp_id, c_algo_id, &c_source_flg, c_ServiceName, c_err_msg); /** Ver 4.1 */ /* Ver 4.2 c_source_flg*/

			/*  If condition added in Ver 3.5 */
			if (i_ch_val == NULL_CASE)
			{
				fn_userlog(c_ServiceName, "In a case of SKIP_ORD for fn_ref_to_ord 3");
				return SKIP_ORD;
			}
			/*** Ver 3.5 ended ***/
			if (i_ch_val != 0)
			{
				fn_errlog(c_ServiceName, "S31325", LIBMSG, c_err_msg);
				if (i_ch_val == LOCK) /* Ver 1.2 Ends */
				{
					return LOCK_ERR;
				}
				else
				{
					return ERROR;
				}
			}
			if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
			{
				fn_gettime();
				fn_userlog(c_ServiceName, "TIME_STATS After calling REF_TO_ORD L3:%s:", c_time);
			} /* Ver 1.2 Ends */
			st_rqst[2].c_plcd_stts = QUEUED;
			st_rqst[2].c_oprn_typ = UPDATION_ON_ORDER_FORWARDING;
			/* Ver 1.2 Starts */
			/****************************************************************
						fn_cpy_ddr(st_rqst[2].c_rout_crt);
						i_ch_val = fn_call_svc (c_ServiceName,
																	c_err_msg,
																	&st_rqst[2],
																	&st_rqst[2],
																	"vw_xchngbook",
																	"vw_xchngbook",
																	sizeof ( st_rqst[2] ),
																	sizeof ( st_rqst[2] ),
																	0,
																		"SFO_UPD_XCHNGBK" );
			****************************************************************/
			i_ch_val = fn_upd_xchngbk(&st_rqst[2], c_ServiceName, c_err_msg);

			if (i_ch_val != 0)
			{
				fn_errlog(c_ServiceName, "S31330", LIBMSG, c_err_msg);
				if (i_ch_val == LOCK) /* Ver 1.2 Ends */
				{
					return LOCK_ERR;
				}
				else
				{
					return ERROR;
				}
			}
			if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
			{
				fn_gettime();
				fn_userlog(c_ServiceName, "TIME_STATS After calling UPD_XCHNGBK L3:%s:", c_time);
			} /* Ver 1.2 Ends */

			st_ord[2].c_ordr_stts = QUEUED;
			st_ord[2].c_oprn_typ = UPDATE_ORDER_STATUS;
			/* Ver 1.2 Starts */
			/****************************************************************
						fn_cpy_ddr(st_ord[2].c_rout_crt);
						i_ch_val = fn_call_svc (c_ServiceName,
																		c_err_msg,
																		&st_ord[2],
																	&st_ord[2],
																	"vw_orderbook",
																	"vw_orderbook",
																	sizeof ( st_ord[2] ),
																		sizeof ( st_ord[2] ),
																	0,
																	"SFO_UPD_ORDRBK" );
			****************************************************************/
			i_ch_val = fn_upd_ordrbk(&st_ord[2], c_ServiceName, c_err_msg);

			if (i_ch_val != 0) /* Ver 1.2 Ends */
			{
				fn_errlog(c_ServiceName, "S31335", LIBMSG, c_err_msg);
				return ERROR;
			}
			if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
			{
				fn_gettime();
				fn_userlog(c_ServiceName, "TIME_STATS After calling UPD_ORDRBK L3:%s:", c_time);
			} /* Ver 1.2 Ends */

			/*** Convert the contract information to exchange format ***/
			strcpy(st_cnt.c_xchng_cd, st_ord[2].c_xchng_cd);
			st_cnt.c_prd_typ = st_ord[2].c_prd_typ;
			strcpy(st_cnt.c_undrlyng, st_ord[2].c_undrlyng);
			strcpy(st_cnt.c_expry_dt, st_ord[2].c_expry_dt);
			st_cnt.c_exrc_typ = st_ord[2].c_exrc_typ;
			st_cnt.c_opt_typ = st_ord[2].c_opt_typ;
			st_cnt.l_strike_prc = st_ord[2].l_strike_prc;
			st_cnt.c_ctgry_indstk = st_ord[2].c_ctgry_indstk;
			st_cnt.l_ca_lvl = st_ord[2].l_ca_lvl;

			if (strcmp(sql_c_xchng_cd, "NFO") == 0)
			{
				st_cnt.c_rqst_typ = CONTRACT_TO_NSE_ID;
				/* Ver 1.2 Starts */
				/****************************************************************
								fn_cpy_ddr(st_cnt.c_rout_crt);
								i_ch_val = fn_call_svc(c_ServiceName,
																		c_err_msg,
																		&st_cnt,
																		&st_nse_cnt[2],
																		"vw_contract",
																		"vw_nse_cntrct",
																		sizeof ( st_cnt ),
																		sizeof ( st_nse_cnt[2] ),
																		0,
																		"SFO_GT_EXT_CNT" );
				****************************************************************/
				i_ch_val = fn_get_ext_cnt(&st_cnt,
										  &st_nse_cnt[2],
										  c_ServiceName,
										  c_err_msg);

				if (i_ch_val != 0) /* Ver 1.2 Ends */
				{
					fn_errlog(c_ServiceName, "S31340", LIBMSG, c_err_msg);
					return ERROR;
				}
				if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
				{
					fn_gettime();
					fn_userlog(c_ServiceName, "TIME_STATS After calling GT_EXT_CNT L3:%s:", c_time);
				} /* Ver 1.2 Ends */
			}
			else
			{
				/*** Code for BSE ****/
				return ERROR;
			}
		}
		break;

	case EXERCISE:
	case DONT_EXERCISE:

		if ((i_exr_mkt_stts == FALSE) && (i_ext_mkt_stts == FALSE))
		{
			return DATA_SKPD;
		}
		strcpy(st_exr.c_exrc_ordr_rfrnc, st_rqst1.c_ordr_rfrnc);
		st_exr.c_oprn_typ = FOR_SNDCLNT;
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Calling SFO_EXRQ_DTLS for EXERCISE order ");
		}
		/* Ver 1.2 Starts */
		/****************************************************************
					fn_cpy_ddr(st_exr.c_rout_crt);
					i_ch_val = fn_call_svc(	c_ServiceName,
																c_err_msg,
																&st_exr,
																&st_exr,
																"vw_exrcbook",
																"vw_exrcbook",
																sizeof ( st_exr ),
																sizeof ( st_exr ),
																0,
																"SFO_EXRQ_DTLS" );
		****************************************************************/
		i_ch_val = fn_exrq_dtls(&st_exr, c_ServiceName, c_err_msg);

		if (i_ch_val != 0)
		{
			fn_errlog(c_ServiceName, "S31345", LIBMSG, c_err_msg);
			if (i_ch_val == LOCK) /* Ver 1.2 Ends */
			{
				return LOCK_ERR;
			}
			else
			{
				return ERROR;
			}
		}
		if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
		{
			fn_gettime();
			fn_userlog(c_ServiceName, "TIME_STATS After calling EXRQ_DTLS for Exercise order :%s:", c_time);
		} /* Ver 1.2 Ends */
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "After Calling SFO_EXRQ_DTLS for EXERCISE order ");
		}

		if (st_exr.l_mdfctn_cntr != st_rqst1.l_mdfctn_cntr)
		{
			return DATA_CNCLD;
		}

		st_rqst1.c_plcd_stts = QUEUED;
		st_rqst1.c_oprn_typ = UPDATION_ON_ORDER_FORWARDING;
		/* Ver 1.2 Starts */
		/****************************************************************
					fn_cpy_ddr(st_rqst1.c_rout_crt);
					i_ch_val = fn_call_svc (c_ServiceName,
																	c_err_msg,
																	&st_rqst1,
																	&st_rqst1,
																	"vw_xchngbook",
																	"vw_xchngbook",
																	sizeof ( st_rqst1 ),
																	sizeof ( st_rqst1 ),
																	0,
																	"SFO_UPD_XCHNGBK" );
		****************************************************************/
		i_ch_val = fn_upd_xchngbk(&st_rqst1, c_ServiceName, c_err_msg);

		if (i_ch_val != 0)
		{
			fn_errlog(c_ServiceName, "S31350", LIBMSG, c_err_msg);
			if (i_ch_val == LOCK) /* Ver 1.2 Ends */
			{
				return LOCK_ERR;
			}
			else
			{
				return ERROR;
			}
		}
		if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
		{
			fn_gettime();
			fn_userlog(c_ServiceName, "TIME_STATS After calling UPD_XCHNGBK for Exercise order :%s:", c_time);
		} /* Ver 1.2 Ends */
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "After Calling SFO_UPD_XCHNGBK for EXERCISE order ");
		}

		st_exr.c_exrc_stts = QUEUED;

		st_exr.c_oprn_typ = UPDATE_EXERCISE_REQUEST_STATUS;
		/* Ver 1.2 Starts */
		/****************************************************************
					fn_cpy_ddr(st_exr.c_rout_crt);
					i_ch_val = fn_call_svc (c_ServiceName,
															c_err_msg,
															&st_exr,
															&st_exr,
															"vw_exrcbook",
															"vw_exrcbook",
															sizeof ( st_exr ),
															sizeof ( st_exr ),
															0,
															"SFO_UPD_EXBK" );
		****************************************************************/
		i_ch_val = fn_upd_exbk(&st_exr, c_ServiceName, c_err_msg);

		if (i_ch_val != 0) /* Ver 1.2 Ends */
		{
			fn_errlog(c_ServiceName, "S31355", LIBMSG, c_err_msg);
			return ERROR;
		}
		if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
		{
			fn_gettime();
			fn_userlog(c_ServiceName, "TIME_STATS After calling UPD_EXBK for Exercise order :%s:", c_time);
		} /* Ver 1.2 Ends */
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "After Calling SFO_UPD_EXBK for EXERCISE order ");
		}

		/**** Convert the contract information to exchange format ****/
		strcpy(st_cnt.c_xchng_cd, st_exr.c_xchng_cd);
		st_cnt.c_prd_typ = st_exr.c_prd_typ;
		strcpy(st_cnt.c_undrlyng, st_exr.c_undrlyng);
		strcpy(st_cnt.c_expry_dt, st_exr.c_expry_dt);
		st_cnt.c_exrc_typ = st_exr.c_exrc_typ;
		st_cnt.c_opt_typ = st_exr.c_opt_typ;
		st_cnt.l_strike_prc = st_exr.l_strike_prc;
		st_cnt.c_ctgry_indstk = st_exr.c_ctgry_indstk;
		st_cnt.l_ca_lvl = st_exr.l_ca_lvl;

		if (strcmp(sql_c_xchng_cd, "NFO") == 0)
		{
			st_cnt.c_rqst_typ = CONTRACT_TO_NSE_ID;
			/* Ver 1.2 Starts */
			/****************************************************************
							fn_cpy_ddr(st_cnt.c_rout_crt);
							i_ch_val = fn_call_svc (c_ServiceName,
																			c_err_msg,
																			&st_cnt,
																			&st_nse_cnt1,
																			"vw_contract",
																			"vw_nse_cntrct",
																			sizeof ( st_cnt ),
																			sizeof ( st_nse_cnt1 ),
																			0,
																			"SFO_GT_EXT_CNT" );
			****************************************************************/
			i_ch_val = fn_get_ext_cnt(&st_cnt,
									  &st_nse_cnt1,
									  c_ServiceName,
									  c_err_msg);

			if (i_ch_val != 0) /* Ver 1.2 Ends */
			{
				fn_errlog(c_ServiceName, "S31360", LIBMSG, c_err_msg);
				return ERROR;
			}
			if (DEBUG_MSG_LVL_3) /* Ver 1.2 Starts */
			{
				fn_gettime();
				fn_userlog(c_ServiceName, "TIME_STATS After calling GT_EXT_CNT for Exercise order :%s:", c_time);
			} /* Ver 1.2 Ends */
		}
		else
		{
			/**** Code for BSE ****/
			return ERROR;
		}
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "After Calling SFO_GT_EXT_CNT for EXERCISE order ");
		}
		break;

	default:
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Unknown E and O type");
		}
		return ERROR;
		break;
	}

	if ((st_rqst1.c_sprd_ord_ind == SPREAD_ORDER) || // this "if" will not be used
		(st_rqst1.c_sprd_ord_ind == L2_ORDER) ||
		(st_rqst1.c_sprd_ord_ind == L3_ORDER))
	{
		st_rqst[0] = st_rqst1;
		st_ord[0] = st_ord1;
		st_nse_cnt[0] = st_nse_cnt1;

		switch (st_rqst1.c_sprd_ord_ind)
		{
		case SPREAD_ORDER:
			i_sprd_flg = 2;
			break;

		case L2_ORDER:
			i_sprd_flg = 2;
			break;

		case L3_ORDER:
			i_sprd_flg = 3;
			break;

		default:
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "Unknown E and O type");
			}
			return -1;
			break;
		}

		memset(&st_q_packet, '\0', sizeof(struct st_req_q_data));
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Packing SPREAD order STARTS");
		}

		for (i_tmp = 0; i_tmp < i_sprd_flg; i_tmp++)
		{
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "The token id for spread order is : %ld", st_nse_cnt[i_tmp].l_token_id);
			}
			if (st_nse_cnt[i_tmp].l_token_id == 0)
			{
				fn_errlog(c_ServiceName, "S31365", "Error - Token id not available", c_err_msg);

				for (i_counter = 0; i_counter < i_sprd_flg; i_counter++)
				{
					i_ch_val = fn_rjct_rcrd(&st_rqst[i_counter],
											&st_ord[i_counter],
											c_ServiceName,
											c_err_msg);

					if (i_ch_val == -1)
					{
						return ERROR;
					}
				}
				return TKN_NA;
			}
		}

		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Packing Spread order STARTS");
		}
		i_ch_val = fn_pack_sprdord_to_nse(st_rqst,
										  st_ord,
										  st_nse_cnt,
										  &st_spd_ord_bk,
										  &st_opm_mstr,
										  &st_q_packet,
										  c_pan_no,		 /*** Ver 3.6 ***/
										  c_lst_act_ref, /*** Ver 3.7 ***/
										  c_esp_id,		 /** Ver 4.1 **/
										  c_algo_id,	 /** Ver 4.1 **/
										  c_source_flg,	 /** Ver 4.2 **/
										  c_ServiceName);
		if (i_ch_val == -1)
		{
			return ERROR;
		}
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Packing Spread order ENDS");
		}
	}
	else if (st_rqst1.c_ex_ordr_typ == ORDINARY_ORDER) // for ordinary
	{
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Packing Ordinary order STARTS");
		}

		if (st_nse_cnt1.l_token_id == 0)
		{
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "The token id for Ordinarty order is : %ld", st_nse_cnt1.l_token_id);
			}
			fn_errlog(c_ServiceName, "S31370", "Error - Token id not available", c_err_msg);

			i_ch_val = fn_rjct_rcrd(&st_rqst1,
									&st_ord1,
									c_ServiceName,
									c_err_msg);
			if (i_ch_val == -1)
			{
				return ERROR;
			}

			return TKN_NA;
		}

		memset(&st_q_packet, '\0', sizeof(struct st_req_q_data));
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Packing ordinary order STARTS");
		}

		if (st_rqst1.c_slm_flg == 'S') /*** if condition added in Ver 3.8 **/
		{
			i_ch_val = fn_pack_ordnry_ord_to_nse(
				&st_rqst1,
				&st_ord1,
				&st_nse_cnt1,
				&st_opm_mstr,
				&st_q_packet,
				c_prgm_flg,	   /*** Ver 2.7 ***/
				c_pan_no,	   /*** Ver 3.6 ***/
				c_lst_act_ref, /*** Ver 3.8 ***/
				c_esp_id,	   /** Ver 4.1 **/
				c_algo_id,	   /** Ver 4.1 **/
				c_source_flg,  /** Ver 4.2 **/
				c_ServiceName);
			/*** Ver 3.8 Starts ***/
		}
		else
		{
			i_ch_val = fn_pack_ordnry_ord_to_nse_tr(
				&st_rqst1,
				&st_ord1,
				&st_nse_cnt1,
				&st_opm_mstr,
				&st_q_packet,
				c_prgm_flg,
				c_pan_no,
				c_lst_act_ref,
				c_esp_id,	  /** Ver 4.1 **/
				c_algo_id,	  /** Ver 4.1 **/
				c_source_flg, /** Ver 4.2 **/
				c_ServiceName);
		}
		/*** Ver 3.8 Ends ***/

		if (i_ch_val == -1)
		{
			return ERROR;
		}
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Packing ordinary order ENDS");
		}
	}
	else if ((st_rqst1.c_ex_ordr_typ == EXERCISE) ||
			 (st_rqst1.c_ex_ordr_typ == DONT_EXERCISE)) // for exer (it will ot be used)
	{
		memset(&st_q_packet, '\0', sizeof(struct st_req_q_data));
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Packing Exercise order STARTS");
		}

		i_ch_val = fn_pack_exer_ord_to_nse(&st_rqst1,
										   &st_exr,
										   &st_nse_cnt1,
										   &st_opm_mstr,
										   &st_q_packet,
										   c_ServiceName);
		if (i_ch_val == -1)
		{
			return ERROR;
		}
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Packing Exercise order ENDS");
		}
	}

	li_cnt_q++;

	return DATA_RTRVD;
}

/****************** Ver 1.6 Start ********************************/

int fn_do_reco(char c_reco_mode, char *c_tm_stmp, char *c_ServiceName, char *c_err_msg)
{

	char sql_c_tm_stmp[LEN_DT]; // #define LEN_DT 20 + 1

	int i_rtrn_val = 0;
	int i_ret_val = 0;
	int i_ch_val = 0;
	int i_trnsctn = 0;
	int i_pckt_cntr;
	int i_rec_stts;
	int i_err[5]; /** Ver 2.4 **/

	long li_stream_no;
	long li_stream_cnt;
	long li_buf_len = 0; /** Ver 2.4 **/

	char c_auto_reconnect = '\0'; /** Ver 2.4 **/
	FBFR32 *ptr_fml_Ibuf;		  /** Ver 2.4 **/
	char c_rqst_typ = '\0';		  /** Ver 2.4 **/
	char c_xchng_stts = '\0';	  /** Ver 2.4 **/
	char c_usr_id[10];			  /** Ver 2.4 **/
	char c_command[150];		  /** Ver 2.4 **/
	char c_msg[100];			  /** Ver 2.4 **/

	MEMSET(c_command); /** Ver 2.4 **/
	MEMSET(c_msg);	   /** Ver 2.4 **/

	varchar sql_c_tm_stmp1[LEN_DT];

	strcpy(sql_c_tm_stmp, c_tm_stmp);

	memset(&st_sndq_packets, '\0', sizeof(st_sndq_packets)); /***	Ver	1.8	***/

	EXEC SQL
		SELECT OPM_STREAM_NO
			INTO : li_stream_no
					   FROM OPM_ORD_PIPE_MSTR
						   WHERE OPM_XCHNG_CD = : sql_c_xchng_cd
													  AND OPM_PIPE_ID = : sql_c_pipe_id;

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31375", SQLMSG, c_err_msg);
		return -1;
	}

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Inside Function fn_do_reco li_stream_no :%ld:", li_stream_no);
	}

	if (c_reco_mode != 'N')
	{
		if (li_stream_no == 0)
		{
			i_rtrn_val = fn_prcs_reco(c_reco_mode, c_tm_stmp, li_stream_cnt, li_stream_no, c_ServiceName, c_err_msg);

			if (i_rtrn_val == -1)
			{
				return -1;
			}
		}
		else
		{
			for (li_stream_cnt = 1; li_stream_cnt <= li_stream_no; li_stream_cnt++)
			{
				if (DEBUG_MSG_LVL_0)
				{
					fn_userlog(c_ServiceName, "Stream Count Is :%ld:", li_stream_cnt);
				}

				i_rtrn_val = fn_prcs_reco(c_reco_mode, c_tm_stmp, li_stream_cnt, li_stream_no, c_ServiceName, c_err_msg);

				if (i_rtrn_val == -1)
				{
					return -1;
				}
			}

			/***	Ver 1.8 	New For Loop To Write messages On Queue	***/

			for (i_pckt_cntr = 0; i_pckt_cntr < li_stream_no; i_pckt_cntr++)
			{
				fn_userlog(c_ServiceName, "Pack  Counter IS :%d:", i_pckt_cntr);

				if (fn_write_msg_q(i_qid, (void *)&st_sndq_packets[i_pckt_cntr], sizeof(struct st_req_q_data), c_ServiceName, c_err_msg) == -1)
				{
					fn_errlog(c_ServiceName, "S31380", LIBMSG, c_err_msg);
					return -1;
				}

				/** sleep(1);	Commented In Ver 1.9 *** Sleep Is introduced temporarily forDownload  RnD	by Sandeep***/

				sleep(2); /***	Ver 1.9 Sleep Increased From 1 Sec To 2 ***/
			}
		}

		i_reco_stts = 1;

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, " Waiting for download over trigger");
		}

		while (i_reco_stts == 1)
		{
			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "Before Going To Sleep For 5 Seconds ");
			}

			sleep(5);

			/**** Check for unsolicited messages ****/

			i_ret_val = fn_chk_stts(c_ServiceName);

			if (i_ret_val == -1)
			{
				return -1;
			}
		}

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, " download processing done");
		}

		/**** Check for unacknowledged orders in the database ****/
		EXEC SQL
			Select 1 into : i_rec_stts
								From dual
									Where
									exists(Select fxb_plcd_stts
											   from fxb_fo_xchng_book
												   where fxb_plcd_stts IN('Q', 'E')
													   and fxb_xchng_cd = : sql_c_xchng_cd
																				and fxb_pipe_id = : sql_c_pipe_id
																										and fxb_mod_trd_dt = : sql_c_nxt_trd_date
																																   and fxb_stream_no = : li_stream_no);

		if ((SQLCODE != 0) && (SQLCODE != NO_DATA_FOUND))
		{
			fn_errlog(c_ServiceName, "S31385", SQLMSG, c_err_msg);
			return -1;
		}
		else if (SQLCODE == NO_DATA_FOUND)
		{
			i_rec_stts = 0;
		}

		if (i_rec_stts == 1)
		{
			EXEC SQL
				SELECT to_char(sysdate, 'dd-mon-yyyy hh24:mi:ss')
					INTO : sql_c_tm_stmp1
							   FROM DUAL;

			if (SQLCODE != 0)
			{
				fn_errlog(c_ServiceName, "S31390", SQLMSG, c_err_msg);
				return -1;
			}

			SETNULL(sql_c_tm_stmp1);

			sprintf(c_err_msg, "|%s| Unacknowleged orders still in the database", (char *)sql_c_tm_stmp1.arr);

			fn_pst_trg(c_ServiceName,
					   "TRG_ORS_CON",
					   c_err_msg,
					   sql_c_pipe_id);
			return -1;
		}

		i_trnsctn = fn_begintran(c_ServiceName, c_err_msg);

		if (i_trnsctn == -1)
		{
			fn_errlog(c_ServiceName, "S31395", LIBMSG, c_err_msg);
			return (-1);
		}

		EXEC SQL
			INSERT INTO ftm_fo_trd_msg(ftm_xchng_cd,
									   ftm_msg,
									   ftm_msg_id,
									   ftm_tm)
				VALUES( : sql_c_xchng_cd,
						  'Down Load Completed:' ||
						: sql_c_pipe_id,
						  'S', /***	Ver	1.7	***/
						  sysdate);
		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "S31400", SQLMSG, c_err_msg);
			fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
			return (-1);
		}

		i_ch_val = fn_committran(c_ServiceName, i_trnsctn, c_err_msg);

		if (i_ch_val == -1)
		{
			fn_errlog(c_ServiceName, "S31405", LIBMSG, c_err_msg);
			return (-1);
		}

		/*** Ver 2.4 Starts Here ***/
		EXEC SQL
			SELECT EXG_CRRNT_STTS
				INTO : c_xchng_stts
						   FROM EXG_XCHNG_MSTR
							   WHERE EXG_XCHNG_CD = 'NFO';
		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "S31410", SQLMSG, c_err_msg);
			return (-1);
		}

		fn_userlog(c_ServiceName, "i_reco_stts :%d: and c_auto_connection :%c: c_xchng_stts :%c:", i_reco_stts, c_auto_connection, c_xchng_stts);
		if (c_xchng_stts == EXCHANGE_OPEN) /** Open **/
		{
			if (i_reco_stts == 0 && c_auto_connection == 'Y')
			{
				sleep(1);

				ptr_fml_Ibuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);

				c_rqst_typ = 'E'; /** START FORWARDING ***/
				c_auto_reconnect = 'Y';
				strcpy(c_usr_id, "system");
				i_err[0] = Fadd32(ptr_fml_Ibuf, FFO_QUEUE_NAME, (char *)sql_c_pipe_id, 0);
				i_err[1] = Fadd32(ptr_fml_Ibuf, FFO_PIPE_ID, (char *)sql_c_pipe_id, 0);
				i_err[2] = Fadd32(ptr_fml_Ibuf, FFO_RQST_TYP, (char *)&c_rqst_typ, 0);
				i_err[3] = Fadd32(ptr_fml_Ibuf, FFO_USR_ID, (char *)c_usr_id, 0);
				i_err[4] = Fadd32(ptr_fml_Ibuf, FFO_OPERATION_TYP, (char *)&c_auto_reconnect, 0);

				i_ch_val = tpcall("SFO_CON_CLNT", (char *)ptr_fml_Ibuf, 0, (char **)&ptr_fml_Ibuf, &li_buf_len, 0);
				if (i_ch_val != 0)
				{
					fn_userlog(c_ServiceName, "Insider error condition");
					MEMSET(c_command);
					strcpy(c_msg, "Order Forwarding Failed, Pls check ...");
					sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
					system(c_command);
				}
				else
				{
					MEMSET(c_command);
					strcpy(c_msg, "Order Forwading Started ...");
					sprintf(c_command, "fo_xchng_relogin_mail.sh NFO %s '%s'", sql_c_pipe_id, c_msg);
					system(c_command);
				}

				fn_userlog(c_ServiceName, "After Service Called ");
				tpfree((char *)ptr_fml_Ibuf);
			}
		}
		/*** Ver 2.4 Ends Here ****/
	}

	return 0;
}

int fn_prcs_reco(char c_reco_mode, char *c_tm_stmp, long li_stream_cnt, long li_stream_no, char *c_ServiceName, char *c_err_msg)
{
	// c_reco_mode , c_tm_stmp (recieved from the tables in "fn_bat_init")
	// li_stream (we are getting from the table in "fn_do_reco")

	// this function called from "fn_do_reco"

	struct st_message_download st_nse_dwnld_req;
	/*
		struct st_message_download
		{
			struct st_int_header st_hdr;
			double d_sequence_number;
		};
	*/
	struct st_exch_msg st_exch_message;
	/*
		struct st_exch_msg
		{
			struct st_net_hdr st_net_header;
			union st_exch_snd_msg st_exch_sndmsg;
		};
	*/
	/*
			struct st_net_hdr
			{
				short si_message_length;
				int i_seq_num;
				unsigned char c_checksum[16];
			};
	*/
	/*
		union st_exch_snd_msg
		{
			struct st_sign_on_req st_signon_req;
			struct st_system_info_req st_sysinf_req;
			struct st_update_local_database st_updlcdb_req;
			struct st_message_download st_msg_dwld;
			struct st_oe_reqres st_oe_res;
			struct st_oe_reqres_tr st_oe_req_tr; // Added in Ver 2.9 
			struct st_om_rqst_tr st_om_req_tr;	 // Added in Ver 2.9 
			struct st_spd_oe_reqres st_spdoe_reqres;
			struct st_ex_pl_reqres st_expl_reqres;
		};

	*/

	EXEC SQL BEGIN DECLARE SECTION;
	int i_ord_bod_stts;
	int i_trd_bod_stts;
	double d_strt_tm_i;
	double d_strt_tm_o;
	double d_tm_diff;
	double d_tm;
	char sql_c_tm_stmp[LEN_DT];
	EXEC SQL END DECLARE SECTION;

	int i_rec_stts;
	int i_ch_val;
	unsigned char digest[16];
	long long ll_strt_tm;

	char c_pipe[3];
	char c_trd_dt[23];
	int i_snd_seq;

	/***************Memset the structures ******************************/

	memset(&st_nse_dwnld_req, '\0', sizeof(struct st_message_download));
	memset(&st_exch_message, '\0', sizeof(struct st_exch_msg));
	memset(&st_q_packet, '\0', sizeof(struct st_req_q_data));

	i_rec_stts = 0;

	strcpy(sql_c_tm_stmp, c_tm_stmp);

	strcpy(c_pipe, sql_c_pipe_id); // this variable we are getting from argv[3] in file "fn_bat_init".
	strcpy(c_trd_dt, (char *)sql_c_nxt_trd_date.arr); // sql_c_nxt_trd_date is fetched from a querry in "fn_bat_init" file.

	fn_userlog(c_ServiceName, "Inside Function fn_prcs_reco");
	fn_userlog(c_ServiceName, "Pipe Id Is :%s:", c_pipe);
	fn_userlog(c_ServiceName, "Trade Date Is :%s:", c_trd_dt);
	fn_userlog(c_ServiceName, "Time Stamp Is :%s:", sql_c_tm_stmp);

	if (c_reco_mode == 'N')
	{
		fn_userlog(c_ServiceName, "Inside No DownLoad Condition");
		i_rec_stts = 0;
		fn_userlog(c_ServiceName, "i_rec_stts Is :%d:", i_rec_stts);
	}
	else if (c_reco_mode == 'S')
	{
		i_rec_stts = 1;
		ll_strt_tm = 0;
	}
	else
	{
		i_ord_bod_stts = 0;

		/***	Commented In Ver 1.8

	EXEC SQL
		Select   1
		into     :i_ord_bod_stts
		From     dual
		Where    exists ( Select fxb_xchng_cd
						  from fxb_fo_xchng_book
						where fxb_xchng_cd   = :sql_c_xchng_cd
						and   fxb_pipe_id    = :sql_c_pipe_id
						and   fxb_mod_trd_dt = :sql_c_nxt_trd_date
						and   fxb_ack_tm is not null
						and   fxb_stream_no  = :li_stream_no );

	if ( ( SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
	{
		fn_errlog(c_ServiceName,"S31415", SQLMSG, c_err_msg);
		return -1;
	}
	else if ( SQLCODE == NO_DATA_FOUND )
	{
		i_ord_bod_stts = 0;
	}

	i_trd_bod_stts = 0;

		EXEC SQL
	  Select   1
	  into     :i_trd_bod_stts
	  From     dual
	  Where    exists ( Select ftd_ordr_rfrnc
						from ftd_fo_trd_dtls, fod_fo_ordr_dtls
						where ftd_ordr_rfrnc = fod_ordr_rfrnc
						and   ftd_xchng_cd   = :sql_c_xchng_cd
						and   fod_pipe_id    = :sql_c_pipe_id
						and   ftd_trd_dt     > :sql_c_nxt_trd_date
						and   ftd_stream_no  = :li_stream_no);

	if ( ( SQLCODE != 0 ) &&( SQLCODE != NO_DATA_FOUND ) )
		{
	  fn_errlog(c_ServiceName,"S31420", SQLMSG, c_err_msg);
	  return -1;
	}
	else if ( SQLCODE == NO_DATA_FOUND )
	{
	  i_trd_bod_stts = 0;
	}

	if ( i_ord_bod_stts == 1 || i_trd_bod_stts == 1 )
	{
	  i_rec_stts = 1 ;
	}

		**************************************/

		i_rec_stts = 1; /***	Ver 1.8	***/

		if (i_rec_stts == 1)
		{
			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "Sending a Download request");
			}

			EXEC SQL
				Select max(fxb_jiffy)
					into : d_strt_tm_o
							   From fxb_fo_xchng_book
								   where fxb_xchng_cd = : sql_c_xchng_cd
															  and fxb_pipe_id = : sql_c_pipe_id
																					  and fxb_mod_trd_dt = : sql_c_nxt_trd_date and fxb_ack_tm <= to_date( : sql_c_tm_stmp, 'dd-mon-yyyy-hh24:mi:ss')
																																					  and fxb_stream_no = : li_stream_cnt; /***	Ver 1.8 li_stream_no Is Replaced By li_stream_cnt	***/

			if ((SQLCODE != 0) && (SQLCODE != -1405))
			{
				fn_errlog(c_ServiceName, "S31425", SQLMSG, c_err_msg);
				return -1;
			}

			if (SQLCODE == -1405)
			{
				d_strt_tm_o = 0;
			}

			EXEC SQL
				SELECT max(ftd_jiffy)
					INTO : d_strt_tm_i
							   FROM ftd_fo_trd_dtls,
						   fod_fo_ordr_dtls
							   where ftd_ordr_rfrnc = fod_ordr_rfrnc
								   and ftd_xchng_cd = : sql_c_xchng_cd
															and fod_pipe_id = : sql_c_pipe_id and ftd_trd_dt > : sql_c_nxt_trd_date and ftd_trd_dt <= to_date( : sql_c_tm_stmp, 'dd-mon-yyyy-hh24:mi:ss') and ftd_stream_no = : li_stream_cnt; /***  Ver 1.8 li_stream_no Is Replaced By li_stream_cnt ***/

			if ((SQLCODE != 0) && (SQLCODE != -1405))
			{
				fn_errlog(c_ServiceName, "S31430", SQLMSG, c_err_msg);
				return -1;
			}
			if (SQLCODE == -1405)
			{
				d_strt_tm_i = 0;
			}

			if (d_strt_tm_i > d_strt_tm_o)
			{
				ll_strt_tm = (long long)d_strt_tm_i;  
			}
			else
			{
				ll_strt_tm = (long long)d_strt_tm_o;
			}

			// ll_strt_tm is stored in the "d_sequence_number" of "st_message_download"
		}
	}
	if (i_rec_stts == 1)
	{

		/*** Populate download request structure  ***/

		/**** fn_orstonse_char ( st_nse_dwnld_req.st_hdr.c_filler_1,
						   2,
						   " ",
						   1);		***** Commtented in Ver 3.8 ***/

		st_nse_dwnld_req.st_hdr.li_trader_id = (long int)atoi(st_opm_mstr.c_opm_trdr_id); /** Ver 3.8 **/

		st_nse_dwnld_req.st_hdr.li_log_time = 0;

		/**********
		fn_orstonse_char ( st_nse_dwnld_req.st_hdr.c_alpha_char,
						   LEN_ALPHA_CHAR,
						   " ",
							1);
		*******************************/

		/**st_nse_dwnld_req.st_hdr.c_alpha_char[0] = li_stream_cnt + 48  ;**Commented in ver 4.0 ***/
		st_nse_dwnld_req.st_hdr.c_alpha_char[0] = li_stream_cnt; /*** Ver 4.0 ***/

		fn_userlog(c_ServiceName, "PACK li_stream_no Is :%ld:", li_stream_cnt);
		fn_userlog(c_ServiceName, "PACK c_alpha_char Is :%c:", st_nse_dwnld_req.st_hdr.c_alpha_char[0]);

		st_nse_dwnld_req.st_hdr.si_transaction_code = DOWNLOAD_REQUEST;

		st_nse_dwnld_req.st_hdr.si_error_code = 0;

		fn_orstonse_char(st_nse_dwnld_req.st_hdr.c_filler_2,
						 8,
						 " ",
						 1);

		memset(st_nse_dwnld_req.st_hdr.c_time_stamp_1, 0, LEN_TIME_STAMP);

		memset(st_nse_dwnld_req.st_hdr.c_time_stamp_2, 0, LEN_TIME_STAMP);

		st_nse_dwnld_req.st_hdr.si_message_length = sizeof(st_nse_dwnld_req);

		memcpy((char *)&st_nse_dwnld_req.d_sequence_number, &ll_strt_tm, 8);

		if (DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName, "**Printing the download header structure st_nse_dwnld_req.st_hdr**");
			/**	fn_userlog(c_ServiceName,"c_filler_1 is : %s",st_nse_dwnld_req.st_hdr.c_filler_1);** Commented in Ver 3.8 **/
			fn_userlog(c_ServiceName, "li_trader_id is :%ld:", st_nse_dwnld_req.st_hdr.li_trader_id); /** Ver 3.8 **/
			fn_userlog(c_ServiceName, " li_log_time is : %ld", st_nse_dwnld_req.st_hdr.li_log_time);
			fn_userlog(c_ServiceName, "c_alpha_char is : %s", st_nse_dwnld_req.st_hdr.c_alpha_char);
			fn_userlog(c_ServiceName, "si_transaction_code is : %d", st_nse_dwnld_req.st_hdr.si_transaction_code);
			fn_userlog(c_ServiceName, "si_error_code is : %d", st_nse_dwnld_req.st_hdr.si_error_code);
			fn_userlog(c_ServiceName, "c_filler_2 is : %s", st_nse_dwnld_req.st_hdr.c_filler_2);
			fn_userlog(c_ServiceName, "c_time_stamp_1 is : %s", st_nse_dwnld_req.st_hdr.c_time_stamp_1);
			fn_userlog(c_ServiceName, "c_time_stamp_2 is : %s", st_nse_dwnld_req.st_hdr.c_time_stamp_2);
			fn_userlog(c_ServiceName, "si_message_length is : %d", st_nse_dwnld_req.st_hdr.si_message_length);
			fn_userlog(c_ServiceName, "**Printing the header structure st_spdord_req. st_hdr ENDS**");
			fn_userlog(c_ServiceName, "st_nse_dwnld_req.d_sequence_number is :%d:", st_nse_dwnld_req.d_sequence_number);
		}

		/***Packing the structure to put in Queue **/

		memcpy(&(st_exch_message.st_exch_sndmsg), &st_nse_dwnld_req, sizeof(st_nse_dwnld_req));

		i_ch_val = fn_get_reset_seq(c_pipe,
									c_trd_dt,
									GET_PLACED_SEQ,
									&i_snd_seq,
									c_ServiceName,
									c_err_msg);

		if (i_ch_val == -1)
		{
			fn_errlog(c_ServiceName, "S00005", LIBMSG, c_err_msg);
			return -1;
		}

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "Sequence number in packet is :%d:", i_snd_seq);
		}
		st_exch_message.st_net_header.i_seq_num = i_snd_seq;

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "The sequence number got is:%d:", i_snd_seq);
		}

		/****call routine for md5 ******/

		fn_cal_md5digest((void *)&st_nse_dwnld_req, sizeof(st_nse_dwnld_req), digest);
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, " After calculating checksum");
		}

		/*****Copy the checksum in network header *******/

		memcpy(st_exch_message.st_net_header.c_checksum, digest, sizeof(digest));

		/******Set length of the network header *******/

		st_exch_message.st_net_header.si_message_length = sizeof(st_exch_message.st_net_header) + sizeof(st_nse_dwnld_req);

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, " After calculating message length");
		}

		/***Packing the structure to in Queue datatype  **/

		st_q_packet.li_msg_type = DOWNLOAD_REQUEST;
		memcpy(&(st_q_packet.st_exch_msg_data), &st_exch_message, sizeof(st_exch_message));

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, " After packing the data in queue packet");
		}

		memcpy(&st_sndq_packets[li_stream_cnt - 1], &st_q_packet, sizeof(st_q_packet));

		/***	Commented In Ver 1.8

			if(fn_write_msg_q(i_qid, (void *)&st_q_packet, sizeof(st_q_packet), c_ServiceName, c_err_msg) == -1)
			{
				fn_errlog(c_ServiceName, "S31435", LIBMSG, c_err_msg);
				return -1;
			}

		*******************/

		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, " After writing message in queue ");
		}
	}

	return 0;
}

/****************** Ver 1.6 Ends ********************************/

/****************** Ver 1.1 Start ********************************/

int fn_write_packets_q(char *c_ServiceName, char *c_err_msg)
{
	int i_cntr;

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "Number of packets to be sent of send q is :%d:", i_pack_counter);
	}

	for (i_cntr = 0; i_cntr < i_pack_counter; i_cntr++)
	{
		if (DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName, "Writing packet :%d:", i_cntr + 1);
		}

		fn_print_details(i_cntr, c_ServiceName, c_err_msg);

		if (fn_write_msg_q(i_qid,
						   (void *)&st_sndq_packets[i_cntr],
						   sizeof(struct st_req_q_data),
						   c_ServiceName,
						   c_err_msg) == -1)
		{
			fn_errlog(c_ServiceName, "S31440", LIBMSG, c_err_msg);
			fn_userlog(c_ServiceName, "ERROR IN SENDING PACKET :%d: on send queue", i_cntr + 1);
			fn_print_details(i_cntr, c_ServiceName, c_err_msg);
			return -1;
		}
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, ":%d: packet(s) sent succesfully on send queue", i_cntr);
	}

	memset(&st_sndq_packets, '\0', sizeof(st_sndq_packets));
	i_pack_counter = 0;

	return 0;
}

void fn_print_details(int i_index, char *c_ServiceName, char *c_err_msg)
{
	if ((st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_spdoe_reqres.st_hdr.si_transaction_code == SP_BOARD_LOT_IN) ||
		(st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_spdoe_reqres.st_hdr.si_transaction_code == TWOL_BOARD_LOT_IN) ||
		(st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_spdoe_reqres.st_hdr.si_transaction_code == THRL_BOARD_LOT_IN))
	{
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Spread order reference is :%s: FSP sequence is :%d:",
					   st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_spdoe_reqres.c_oe_remarks,
					   st_sndq_packets[i_index].st_exch_msg_data.st_net_header.i_seq_num);
		}
	}
	else if ((st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_oe_res.st_hdr.si_transaction_code == BOARD_LOT_IN) ||
			 (st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_oe_res.st_hdr.si_transaction_code == ORDER_MOD_IN) ||
			 (st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_oe_res.st_hdr.si_transaction_code == ORDER_CANCEL_IN))
	{
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "order sequence is :%d: pipe id is :%s: FSP sequence is :%d:",
					   st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_oe_res.i_ordr_sqnc, sql_c_pipe_id,
					   st_sndq_packets[i_index].st_exch_msg_data.st_net_header.i_seq_num);
		}
	}
	else if ((st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_expl_reqres.st_hdr.si_transaction_code == EX_PL_ENTRY_IN) ||
			 (st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_expl_reqres.st_hdr.si_transaction_code == EX_PL_MOD_IN) ||
			 (st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_expl_reqres.st_hdr.si_transaction_code == EX_PL_MOD_IN))
	{
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Exercise order reference is :%s: FSP sequence is :%d:",
					   st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_expl_reqres.st_ex_pl_data.c_remarks,
					   st_sndq_packets[i_index].st_exch_msg_data.st_net_header.i_seq_num);
		}
	}
	else
	{
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Unknown transaction code ! ");
			fn_userlog(c_ServiceName, "Spread order reference is :%s:",
					   st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_spdoe_reqres.c_oe_remarks);
			fn_userlog(c_ServiceName, "order sequence is :%d: pipe id is :%s:",
					   st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_oe_res.i_ordr_sqnc, sql_c_pipe_id);
			fn_userlog(c_ServiceName, "Exercise order reference is :%s:",
					   st_sndq_packets[i_index].st_exch_msg_data.st_exch_sndmsg.st_expl_reqres.st_ex_pl_data.c_remarks);
			fn_userlog(c_ServiceName, "FSP sequence is :%d:",
					   st_sndq_packets[i_index].st_exch_msg_data.st_net_header.i_seq_num);
		}
	}
	return;
}

/****************** Ver 1.1 Ends ********************************/

/****************** Ver 1.2 Starts ********************************/

int fn_get_ext_cnt(struct vw_contract *ptr_st_cnt, struct vw_nse_cntrct *ptr_st_nse_cnt, char *c_ServiceName, char *c_err_msg)
{
	int i_len;
	int i_count;
	EXEC SQL BEGIN DECLARE SECTION;
	int i_sem_entity;
	varchar c_symbl[21];
	char c_stck_cd[7];
	char c_exg_cd[4];
	varchar c_expiry_dt[LEN_DATE];
	EXEC SQL END DECLARE SECTION;

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "In function get_ext_cnt");
	}

	if (strcmp(ptr_st_cnt->c_xchng_cd, "NFO") == 0)
	{
		i_sem_entity = NFO_ENTTY;
	}
	else
	{
		i_sem_entity = BFO_ENTTY;
	}

	strcpy((char *)c_expiry_dt.arr, ptr_st_cnt->c_expry_dt);
	SETLEN(c_expiry_dt);
	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "c_expiry_dt is :%s:", c_expiry_dt.arr);
	}

	MEMSET(c_symbl);

	strcpy(c_stck_cd, ptr_st_cnt->c_undrlyng);

	i_len = strlen(c_stck_cd);

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "c_stck_cd before :%s:", c_stck_cd);
	}

	for (i_count = 0; i_count < i_len; i_count++)
	{
		c_stck_cd[i_count] = toupper(c_stck_cd[i_count]);
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "c_stck_cd after :%s:", c_stck_cd);
	}

	EXEC SQL
		SELECT TRIM(sem_map_vl)
			INTO : c_symbl
					   FROM sem_stck_map
						   WHERE sem_entty = : i_sem_entity
												   AND sem_stck_cd = : c_stck_cd;

	if (SQLCODE != 0)
	{
		if (SQLCODE == NO_DATA_FOUND)
		{
			strcpy(c_symbl.arr, " ");
		}
		else
		{
			fn_errlog(c_ServiceName, "S31445", SQLMSG, c_err_msg);
			return -1;
		}
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "The sql code  is %ld", sqlca.sqlcode);
	}

	SETNULL(c_symbl);

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Symbol is :%s:", c_symbl.arr);
	}

	memset(ptr_st_nse_cnt, '\0', sizeof(struct vw_nse_cntrct));

	strcpy(ptr_st_nse_cnt->c_xchng_cd, ptr_st_cnt->c_xchng_cd);

	ptr_st_nse_cnt->c_prd_typ = ptr_st_cnt->c_prd_typ;

	strcpy(ptr_st_nse_cnt->c_expry_dt, (char *)c_expiry_dt.arr);

	ptr_st_nse_cnt->c_exrc_typ = ptr_st_cnt->c_exrc_typ;

	ptr_st_nse_cnt->c_opt_typ = ptr_st_cnt->c_opt_typ;

	ptr_st_nse_cnt->l_strike_prc = ptr_st_cnt->l_strike_prc;

	strcpy(ptr_st_nse_cnt->c_symbol, c_symbl.arr);

	ptr_st_nse_cnt->c_rqst_typ = ptr_st_cnt->c_rqst_typ;

	ptr_st_nse_cnt->c_ctgry_indstk = ptr_st_cnt->c_ctgry_indstk;

	if (strcmp(ptr_st_cnt->c_xchng_cd, "NFO") == 0)
	{
		strcpy(c_exg_cd, "NSE");
	}
	else if (strcmp(ptr_st_cnt->c_xchng_cd, "BFO") == 0)
	{
		strcpy(c_exg_cd, "BSE");
	}
	else
	{
		fn_errlog(c_ServiceName, "S31450", "Invalid option", c_err_msg);
		return -1;
	}

	/***** Commented in Ver 1.4
  EXEC SQL
	SELECT   nvl(ESM_XCHNG_SERIES ,' ')
	  INTO   :ptr_st_nse_cnt->c_series
	  FROM    ESS_SGMNT_STCK
						 ,ESM_XCHNG_SGMNT_MSTR
	 WHERE   esm_sgmnt_cd=ess_xchng_sgmnt_cd
	   AND   esm_xchng_cd=ess_xchng_cd
	   AND   ess_stck_cd  = :ptr_st_cnt->c_undrlyng
	   AND   ess_xchng_cd = :c_exg_cd;
	*****/

	/**** Ver 2.0 Starts ****/

	EXEC SQL
		SELECT nvl(ESS_XCHNG_SUB_SERIES, ' ')
			INTO : ptr_st_nse_cnt->c_series
					   FROM ESS_SGMNT_STCK
						   WHERE ess_stck_cd = : ptr_st_cnt->c_undrlyng
													 AND ess_xchng_cd = : c_exg_cd;

	/****** Ver 2.0 Ends  ****/

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31455", SQLMSG, c_err_msg);
		return -1;
	}

	/*** Commented in Ver 3.0 ***

	EXEC SQL
		  SELECT      NVL ( fcm_token_no, 0 ),
					  NVL( fcm_ca_lvl, 0 )
			INTO      :ptr_st_nse_cnt->l_token_id,
					  :ptr_st_nse_cnt->l_ca_lvl
			FROM      fcm_fo_cntrct_mstr
		   WHERE      fcm_xchng_cd  = :ptr_st_cnt->c_xchng_cd
			 AND      fcm_prdct_typ = :ptr_st_cnt->c_prd_typ
			 AND      fcm_undrlyng  = :ptr_st_cnt->c_undrlyng
			 AND      fcm_expry_dt  =  to_date ( :ptr_st_cnt->c_expry_dt, 'dd-Mon-yyyy' )
			 AND      fcm_exer_typ  = :ptr_st_cnt->c_exrc_typ
			 AND      fcm_opt_typ   = :ptr_st_cnt->c_opt_typ
			 AND      fcm_strk_prc  = :ptr_st_cnt->l_strike_prc;
	  ***/
	/*** Ver 3.0 Starts ***/

	EXEC SQL
		SELECT NVL(ftq_token_no, 0),
		NVL(ftq_ca_lvl, 0)
			INTO : ptr_st_nse_cnt->l_token_id,
		: ptr_st_nse_cnt->l_ca_lvl
			  FROM ftq_fo_trd_qt
				  WHERE ftq_xchng_cd = : ptr_st_cnt->c_xchng_cd
											 AND ftq_prdct_typ = : ptr_st_cnt->c_prd_typ
																	   AND ftq_undrlyng = : ptr_st_cnt->c_undrlyng
																								AND ftq_expry_dt = to_date( : ptr_st_cnt->c_expry_dt, 'dd-Mon-yyyy')
			AND ftq_exer_typ = : ptr_st_cnt->c_exrc_typ
									 AND ftq_opt_typ = : ptr_st_cnt->c_opt_typ
															 AND ftq_strk_prc = : ptr_st_cnt->l_strike_prc;

	/*** Ver 3.0 Ends   ***/

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31460", SQLMSG, c_err_msg);
		return -1;
	}

	if (DEBUG_MSG_LVL_3)
	{

		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->c_xchng_cd is :%s:", ptr_st_nse_cnt->c_xchng_cd);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->c_prd_typ is :%s:", ptr_st_nse_cnt->c_prd_typ);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->c_expry_dt is :%s:", ptr_st_nse_cnt->c_expry_dt);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->c_exrc_typ is :%s:", ptr_st_nse_cnt->c_exrc_typ);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->c_opt_typ is :%s:", ptr_st_nse_cnt->c_opt_typ);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->l_strike_prc is :%ld:", ptr_st_nse_cnt->l_strike_prc);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->c_symbol is :%s:", ptr_st_nse_cnt->c_symbol);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->c_ctgry_indstk is :%s:", ptr_st_nse_cnt->c_ctgry_indstk);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->c_rqst_typ is :%s:", ptr_st_nse_cnt->c_rqst_typ);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->c_series is :%s:", ptr_st_nse_cnt->c_series);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->l_token_id is :%ld:", ptr_st_nse_cnt->l_token_id);
		fn_userlog(c_ServiceName, " ptr_st_nse_cnt->l_ca_lvl is :%ld:", ptr_st_nse_cnt->l_ca_lvl);
	}
	return 0;
}

int fn_seq_to_omd(struct vw_xchngbook *ptr_st_xchngbook, char *c_ServiceName, char *c_err_msg, char *c_prgm_flg) /** Ver 2.7 c_prgm_flg added **/
{
	EXEC SQL BEGIN DECLARE SECTION;
	short i_qt_tm;
	short i_frwrd_tm;
	short i_rms_prcs_flg;
	short i_ors_msg_typ;
	short i_ack_tm;
	short i_xchng_rmrks;
	short i_xchng_can_qty;
	varchar c_rmrks[256];
	EXEC SQL END DECLARE SECTION;

	char c_ip_addrs[15 + 1];   /** ver 2.7 **/
	MEMSET(c_ip_addrs);		   /** ver 2.7 **/
	char c_prcimpv_flg = '\0'; /** Ver 3.3 **/

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Function SEQ_TO_OMD");
	}
	switch (ptr_st_xchngbook->c_oprn_typ)
	{

		if (DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName, "c_xchng_cd:%s:", ptr_st_xchngbook->c_xchng_cd);
			fn_userlog(c_ServiceName, "c_pipe_id:%s:", ptr_st_xchngbook->c_pipe_id);
			fn_userlog(c_ServiceName, "l_ord_seq:%ld:", ptr_st_xchngbook->l_ord_seq);
			fn_userlog(c_ServiceName, "mod_trd_dt:%s:", ptr_st_xchngbook->c_mod_trd_dt);
		}

	case FOR_SPRD:

		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "for Spread selection, l_ord_seq :%ld:", ptr_st_xchngbook->l_ord_seq);
		}
		EXEC SQL
			SELECT fxb_ordr_rfrnc,
			fxb_lmt_mrkt_sl_flg,
			fxb_dsclsd_qty,
			fxb_ordr_tot_qty,
			fxb_lmt_rt,
			fxb_stp_lss_tgr,
			fxb_mdfctn_cntr,
			to_char(fxb_ordr_valid_dt, 'dd-mon-yyyy'),
			/** fxb_ordr_type, *** Ver 2.8 **/
			DECODE(fxb_ordr_type, 'V', 'T', fxb_ordr_type), /** Ver 2.8 **/
			fxb_sprd_ord_ind,
			fxb_rqst_typ,
			fxb_quote,
			to_char(fxb_qt_tm, 'dd-mon-yyyy hh24:mi:ss'),
			to_char(fxb_rqst_tm, 'dd-mon-yyyy hh24:mi:ss'),
			to_char(fxb_frwd_tm, 'dd-mon-yyyy hh24:mi:ss'),
			fxb_plcd_stts,
			fxb_rms_prcsd_flg,
			fxb_ors_msg_typ,
			to_char(fxb_ack_tm, 'dd-mon-yyyy hh24:mi:ss'),
			fxb_xchng_rmrks,
			fxb_ex_ordr_typ,
			fxb_xchng_cncld_qty,
			fxb_spl_flag,
			fxb_ordr_sqnc
				INTO : ptr_st_xchngbook->c_ordr_rfrnc,
			: ptr_st_xchngbook->c_slm_flg,
			: ptr_st_xchngbook->l_dsclsd_qty,
			: ptr_st_xchngbook->l_ord_tot_qty,
			: ptr_st_xchngbook->l_ord_lmt_rt,
			: ptr_st_xchngbook->l_stp_lss_tgr,
			: ptr_st_xchngbook->l_mdfctn_cntr,
			: ptr_st_xchngbook->c_valid_dt,
			: ptr_st_xchngbook->c_ord_typ,
			: ptr_st_xchngbook->c_sprd_ord_ind,
			: ptr_st_xchngbook->c_req_typ,
			: ptr_st_xchngbook->l_quote,
			: ptr_st_xchngbook->c_qt_tm : i_qt_tm,
			: ptr_st_xchngbook->c_rqst_tm,
			: ptr_st_xchngbook->c_frwrd_tm : i_frwrd_tm,
			: ptr_st_xchngbook->c_plcd_stts,
			: ptr_st_xchngbook->c_rms_prcsd_flg : i_rms_prcs_flg,
			: ptr_st_xchngbook->l_ors_msg_typ : i_ors_msg_typ,
			: ptr_st_xchngbook->c_ack_tm : i_ack_tm,
			: c_rmrks : i_xchng_rmrks,
			: ptr_st_xchngbook->c_ex_ordr_typ,
			: ptr_st_xchngbook->l_xchng_can_qty : i_xchng_can_qty,
			: ptr_st_xchngbook->c_spl_flg,
			: ptr_st_xchngbook->l_ord_seq
				  FROM FXB_FO_XCHNG_BOOK
					  WHERE fxb_xchng_cd = : ptr_st_xchngbook->c_xchng_cd
												 AND fxb_pipe_id = : ptr_st_xchngbook->c_pipe_id
																		 AND fxb_ordr_sqnc = : ptr_st_xchngbook->l_ord_seq
																								   AND fxb_mod_trd_dt = to_date( : ptr_st_xchngbook->c_mod_trd_dt, 'dd-Mon-yyyy');

		break;

	case FOR_NORM:
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "for Normal selection, Pipe id:%s:", ptr_st_xchngbook->c_pipe_id);
		}
		EXEC SQL
			SELECT fxb_ordr_rfrnc,
			fxb_lmt_mrkt_sl_flg,
			fxb_dsclsd_qty,
			fxb_ordr_tot_qty,
			fxb_lmt_rt,
			fxb_stp_lss_tgr,
			fxb_mdfctn_cntr,  
			to_char(fxb_ordr_valid_dt, 'dd-mon-yyyy'),
			/** fxb_ordr_type,** Ver 2.8 **/
			DECODE(fxb_ordr_type, 'V', 'T', fxb_ordr_type), /** Ver 2.8 **/
			fxb_sprd_ord_ind,
			fxb_rqst_typ,
			fxb_quote,
			to_char(fxb_qt_tm, 'dd-mon-yyyy hh24:mi:ss'),
			to_char(fxb_rqst_tm, 'dd-mon-yyyy hh24:mi:ss'),
			to_char(fxb_frwd_tm, 'dd-mon-yyyy hh24:mi:ss'),
			fxb_plcd_stts,
			fxb_rms_prcsd_flg,
			fxb_ors_msg_typ,
			to_char(fxb_ack_tm, 'dd-mon-yyyy hh24:mi:ss'),
			fxb_xchng_rmrks,
			fxb_ex_ordr_typ,
			fxb_xchng_cncld_qty,
			fxb_spl_flag,
			fxb_ordr_sqnc,
			nvl(FXB_IP, 'NA'),		  /** Ver 2.7 **/
			NVL(FXB_PRCIMPV_FLG, 'N') /** Ver 3.3 **/
			INTO : ptr_st_xchngbook->c_ordr_rfrnc,
			: ptr_st_xchngbook->c_slm_flg,
			: ptr_st_xchngbook->l_dsclsd_qty,
			: ptr_st_xchngbook->l_ord_tot_qty,
			: ptr_st_xchngbook->l_ord_lmt_rt,
			: ptr_st_xchngbook->l_stp_lss_tgr,
			: ptr_st_xchngbook->l_mdfctn_cntr,
			: ptr_st_xchngbook->c_valid_dt,
			: ptr_st_xchngbook->c_ord_typ,
			: ptr_st_xchngbook->c_sprd_ord_ind,
			: ptr_st_xchngbook->c_req_typ,
			: ptr_st_xchngbook->l_quote,
			: ptr_st_xchngbook->c_qt_tm : i_qt_tm,
			: ptr_st_xchngbook->c_rqst_tm,
			: ptr_st_xchngbook->c_frwrd_tm : i_frwrd_tm,
			: ptr_st_xchngbook->c_plcd_stts,
			: ptr_st_xchngbook->c_rms_prcsd_flg : i_rms_prcs_flg,
			: ptr_st_xchngbook->l_ors_msg_typ : i_ors_msg_typ,
			: ptr_st_xchngbook->c_ack_tm : i_ack_tm,
			: c_rmrks : i_xchng_rmrks,
			: ptr_st_xchngbook->c_ex_ordr_typ,
			: ptr_st_xchngbook->l_xchng_can_qty : i_xchng_can_qty,
			: ptr_st_xchngbook->c_spl_flg,
			: ptr_st_xchngbook->l_ord_seq,
			: c_ip_addrs,	/** Ver 2.7 **/
			: c_prcimpv_flg /** Ver 3.3 **/
				  FROM FXB_FO_XCHNG_BOOK
					  WHERE fxb_xchng_cd = : ptr_st_xchngbook->c_xchng_cd
												 AND fxb_pipe_id = : ptr_st_xchngbook->c_pipe_id
																		 AND fxb_mod_trd_dt = to_date( : ptr_st_xchngbook->c_mod_trd_dt, 'dd-Mon-yyyy')
						  AND fxb_ordr_sqnc = (SELECT min(fxb_b.fxb_ordr_sqnc)
												   FROM FXB_FO_XCHNG_BOOK fxb_b
													   WHERE fxb_b.fxb_xchng_cd = : ptr_st_xchngbook->c_xchng_cd
																						AND fxb_b.fxb_mod_trd_dt = to_date( : ptr_st_xchngbook->c_mod_trd_dt, 'dd-Mon-yyyy')
																													   AND fxb_b.fxb_pipe_id = : ptr_st_xchngbook->c_pipe_id
																																					 AND fxb_b.fxb_plcd_stts = 'R'
											   /*** AND      nvl(fxb_b.fxb_rms_prcsd_flg, '*') != 'P'); *** Commented in Ver 4.4 **/
						  ); /*** Ver 4.4, added **/
		break;

	case FOR_EXTEND:
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "for Extended selection, Pipe id:%s:", ptr_st_xchngbook->c_pipe_id);
		}
		EXEC SQL
			SELECT fxb_ordr_rfrnc,
			fxb_lmt_mrkt_sl_flg,
			fxb_dsclsd_qty,
			fxb_ordr_tot_qty,
			fxb_lmt_rt,
			fxb_stp_lss_tgr,
			fxb_mdfctn_cntr,
			to_char(fxb_ordr_valid_dt, 'dd-mon-yyyy'),
			/** fxb_ordr_type,** Ver 2.8 **/
			DECODE(fxb_ordr_type, 'V', 'T', fxb_ordr_type), /** Ver 2.8 **/
			fxb_sprd_ord_ind,
			fxb_rqst_typ,
			fxb_quote,
			to_char(fxb_qt_tm, 'dd-mon-yyyy hh24:mi:ss'),
			to_char(fxb_rqst_tm, 'dd-mon-yyyy hh24:mi:ss'),
			to_char(fxb_frwd_tm, 'dd-mon-yyyy hh24:mi:ss'),
			fxb_plcd_stts,
			fxb_rms_prcsd_flg,
			fxb_ors_msg_typ,
			to_char(fxb_ack_tm, 'dd-mon-yyyy hh24:mi:ss'),
			fxb_xchng_rmrks,
			fxb_ex_ordr_typ,
			fxb_xchng_cncld_qty,
			fxb_spl_flag,
			fxb_ordr_sqnc
				INTO : ptr_st_xchngbook->c_ordr_rfrnc,
			: ptr_st_xchngbook->c_slm_flg,
			: ptr_st_xchngbook->l_dsclsd_qty,
			: ptr_st_xchngbook->l_ord_tot_qty,
			: ptr_st_xchngbook->l_ord_lmt_rt,
			: ptr_st_xchngbook->l_stp_lss_tgr,
			: ptr_st_xchngbook->l_mdfctn_cntr,
			: ptr_st_xchngbook->c_valid_dt,
			: ptr_st_xchngbook->c_ord_typ,
			: ptr_st_xchngbook->c_sprd_ord_ind,
			: ptr_st_xchngbook->c_req_typ,
			: ptr_st_xchngbook->l_quote,
			: ptr_st_xchngbook->c_qt_tm : i_qt_tm,
			: ptr_st_xchngbook->c_rqst_tm,
			: ptr_st_xchngbook->c_frwrd_tm : i_frwrd_tm,
			: ptr_st_xchngbook->c_plcd_stts,
			: ptr_st_xchngbook->c_rms_prcsd_flg : i_rms_prcs_flg,
			: ptr_st_xchngbook->l_ors_msg_typ : i_ors_msg_typ,
			: ptr_st_xchngbook->c_ack_tm : i_ack_tm,
			: c_rmrks : i_xchng_rmrks,
			: ptr_st_xchngbook->c_ex_ordr_typ,
			: ptr_st_xchngbook->l_xchng_can_qty : i_xchng_can_qty,
			: ptr_st_xchngbook->c_spl_flg,
			: ptr_st_xchngbook->l_ord_seq
				  FROM FXB_FO_XCHNG_BOOK
					  WHERE fxb_xchng_cd = : ptr_st_xchngbook->c_xchng_cd
												 AND fxb_pipe_id = : ptr_st_xchngbook->c_pipe_id
																		 AND fxb_mod_trd_dt = to_date( : ptr_st_xchngbook->c_mod_trd_dt, 'dd-Mon-yyyy')
				AND fxb_ordr_sqnc = (SELECT min(fxb_b.fxb_ordr_sqnc)
										 FROM FXB_FO_XCHNG_BOOK fxb_b
											 WHERE fxb_b.fxb_xchng_cd = : ptr_st_xchngbook->c_xchng_cd
																			  AND fxb_b.fxb_mod_trd_dt = to_date( : ptr_st_xchngbook->c_mod_trd_dt, 'dd-Mon-yyyy')
																											 AND fxb_b.fxb_pipe_id = : ptr_st_xchngbook->c_pipe_id
																																		   AND fxb_b.fxb_plcd_stts = 'R' AND fxb_b.fxb_mrkt_typ = 'X' AND nvl(fxb_b.fxb_rms_prcsd_flg, '*') != 'P');
		break;

	case FOR_EXER: /**Ver 1.4 **/
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "for Excercise selection, Pipe id:%s:", ptr_st_xchngbook->c_pipe_id);
		}
		EXEC SQL
			SELECT fxb_ordr_rfrnc,
			fxb_lmt_mrkt_sl_flg,
			fxb_dsclsd_qty,
			fxb_ordr_tot_qty,
			fxb_lmt_rt,
			fxb_stp_lss_tgr,
			fxb_mdfctn_cntr,
			to_char(fxb_ordr_valid_dt, 'dd-mon-yyyy'),
			/** fxb_ordr_type,** Ver 2.8 **/
			DECODE(fxb_ordr_type, 'V', 'T', fxb_ordr_type), /** Ver 2.8 **/
			fxb_sprd_ord_ind,
			fxb_rqst_typ,
			fxb_quote,
			to_char(fxb_qt_tm, 'dd-mon-yyyy hh24:mi:ss'),
			to_char(fxb_rqst_tm, 'dd-mon-yyyy hh24:mi:ss'),
			to_char(fxb_frwd_tm, 'dd-mon-yyyy hh24:mi:ss'),
			fxb_plcd_stts,
			fxb_rms_prcsd_flg,
			fxb_ors_msg_typ,
			to_char(fxb_ack_tm, 'dd-mon-yyyy hh24:mi:ss'),
			fxb_xchng_rmrks,
			fxb_ex_ordr_typ,
			fxb_xchng_cncld_qty,
			fxb_spl_flag,
			fxb_ordr_sqnc
				INTO : ptr_st_xchngbook->c_ordr_rfrnc,
			: ptr_st_xchngbook->c_slm_flg,
			: ptr_st_xchngbook->l_dsclsd_qty,
			: ptr_st_xchngbook->l_ord_tot_qty,
			: ptr_st_xchngbook->l_ord_lmt_rt,
			: ptr_st_xchngbook->l_stp_lss_tgr,
			: ptr_st_xchngbook->l_mdfctn_cntr,
			: ptr_st_xchngbook->c_valid_dt,
			: ptr_st_xchngbook->c_ord_typ,
			: ptr_st_xchngbook->c_sprd_ord_ind,
			: ptr_st_xchngbook->c_req_typ,
			: ptr_st_xchngbook->l_quote,
			: ptr_st_xchngbook->c_qt_tm : i_qt_tm,
			: ptr_st_xchngbook->c_rqst_tm,
			: ptr_st_xchngbook->c_frwrd_tm : i_frwrd_tm,
			: ptr_st_xchngbook->c_plcd_stts,
			: ptr_st_xchngbook->c_rms_prcsd_flg : i_rms_prcs_flg,
			: ptr_st_xchngbook->l_ors_msg_typ : i_ors_msg_typ,
			: ptr_st_xchngbook->c_ack_tm : i_ack_tm,
			: c_rmrks : i_xchng_rmrks,
			: ptr_st_xchngbook->c_ex_ordr_typ,
			: ptr_st_xchngbook->l_xchng_can_qty : i_xchng_can_qty,
			: ptr_st_xchngbook->c_spl_flg,
			: ptr_st_xchngbook->l_ord_seq
				  FROM FXB_FO_XCHNG_BOOK
					  WHERE fxb_xchng_cd = : ptr_st_xchngbook->c_xchng_cd
												 AND fxb_pipe_id = : ptr_st_xchngbook->c_pipe_id
																		 AND fxb_mod_trd_dt = to_date( : ptr_st_xchngbook->c_mod_trd_dt, 'dd-Mon-yyyy')
				AND fxb_ordr_sqnc = (SELECT min(fxb_b.fxb_ordr_sqnc)
										 FROM FXB_FO_XCHNG_BOOK fxb_b
											 WHERE fxb_b.fxb_xchng_cd = : ptr_st_xchngbook->c_xchng_cd
																			  AND fxb_b.fxb_mod_trd_dt = to_date( : ptr_st_xchngbook->c_mod_trd_dt, 'dd-Mon-yyyy')
																											 AND fxb_b.fxb_pipe_id = : ptr_st_xchngbook->c_pipe_id
																																		   AND fxb_b.fxb_plcd_stts = 'R' AND fxb_b.fxb_ex_ordr_typ = 'E' AND nvl(fxb_b.fxb_rms_prcsd_flg, '*') != 'P');
		break;
	default:
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Invalid operation type :%s:", ptr_st_xchngbook->c_oprn_typ);
		}
		return -1;
	}

	if (SQLCODE != 0)
	{
		if (SQLCODE == NO_DATA_FOUND)
		{
			if (DEBUG_MSG_LVL_1)
			{
				fn_userlog(c_ServiceName, "NDF for seq [%ld]", ptr_st_xchngbook->l_ord_seq);
			}
			return NO_DATA_FOUND;
		}
		/*** added in ver 3.5 ***/
		else if (SQLCODE == -1405)
		{
			fn_errlog(c_ServiceName, "S31465", SQLMSG, c_err_msg);
			fn_userlog(c_ServiceName, "Inside NULL_CASE for fn_seq_to_omd() ");
			c_sprd_ord_ind = ptr_st_xchngbook->c_spl_flg;
			strcpy(c_ordr_rfrnc, ptr_st_xchngbook->c_ordr_rfrnc);
			return NULL_CASE;
		}
		/*** Ver 3.5 ended ***/
		else
		{
			fn_errlog(c_ServiceName, "S31470", SQLMSG, c_err_msg);
			return -1;
		}
	}

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "c_ip_addrs[0] :%c:", c_ip_addrs[0]);
	}

	if (c_prcimpv_flg == 'Y') /** if condition added in Ver 3.3 **/
	{
		*c_prgm_flg = 'T'; /** Ver 3.3 **/
	}
	else
	{
		*c_prgm_flg = c_ip_addrs[0]; /** Ver 2.7 **/
	}
	if (i_xchng_rmrks == -1)
	{
		memset(ptr_st_xchngbook->c_xchng_rmrks, 0, sizeof(ptr_st_xchngbook->c_xchng_rmrks));
	}
	else
	{
		SETNULL(c_rmrks);
		strcpy(ptr_st_xchngbook->c_xchng_rmrks, (char *)c_rmrks.arr);
	}

	return 0;
}

int fn_ref_to_ord(struct vw_orderbook *ptr_st_orderbook, char *c_pan_no, char *c_lst_act_ref, char *c_esp_id, char *c_algo_id, char *c_source_flg, char *c_ServiceName, char *c_err_msg)
{
	EXEC SQL BEGIN DECLARE SECTION;
	short i_sprd_ord_rfrnc;
	varchar c_ucc_cd[11];
	varchar c_cp_code[11 + 2];
	short i_settlor;
	short i_ack_tm;
	short i_prev_ack_tm;
	short i_xchng_ack;
	short i_valid_dt;
	varchar v_pan_no[11];		/*** Ver 3.6 ***/
	varchar v_esp_id[51];		/*** Ver 4.1 ***/
	varchar v_algo_id[51];		/*** Ver 4.1 ***/
	char c_lst_act_ref_tmp[22]; /** Ver 3.8 **/
	EXEC SQL END DECLARE SECTION;

	char c_source_flg_tmp = '\0'; /** Ver 4.2 **/

	MEMSET(c_lst_act_ref_tmp); /** Ver 3.8 **/

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Function REF_TO_ORD");
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "c_ordr_rfrnc:%s:", ptr_st_orderbook->c_ordr_rfrnc);
		fn_userlog(c_ServiceName, "Requested Operation :%c:", ptr_st_orderbook->c_oprn_typ);
	}

	MEMSET(v_pan_no); /*** Ver 3.6 ***/
	MEMSET(ptr_st_orderbook->c_ctcl_id);
	EXEC SQL
		SELECT fod_clm_mtch_accnt,
		fod_clnt_ctgry,
		fod_pipe_id,
		fod_xchng_cd,
		decode( : ptr_st_orderbook->c_oprn_typ, 'S', (decode(fod_prdct_typ, 'P', 'F', 'U', 'F', 'I', 'O', fod_prdct_typ)), fod_prdct_typ), /*** Ver 2.3 ***/
		fod_undrlyng,
		to_char(fod_expry_dt, 'dd-Mon-yyyy'),
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
		to_char(fod_ordr_valid_dt, 'dd-Mon-yyyy'),
		to_char(fod_trd_dt, 'dd-Mon-yyyy'),
		fod_ordr_stts,
		fod_exec_qty,
		NVL(fod_exec_qty_day, 0),
		fod_cncl_qty,
		fod_exprd_qty,
		fod_sprd_ordr_ref,
		fod_mdfctn_cntr,
		fod_settlor,
		fod_ack_nmbr,
		fod_spl_flag,
		fod_indstk,
		to_char(fod_ord_ack_tm, 'dd-Mon-yyyy hh24:mi:ss'),
		to_char(fod_lst_rqst_ack_tm, 'dd-Mon-yyyy hh24:mi:ss'),
		fod_pro_cli_ind,
		nvl(fod_ctcl_id, ' '),
		fod_channel,
		nvl(fod_pan_no, '*'),						 /*** Ver 3.6 ***/
		nvl(fod_lst_act_ref, '0'), /*** Ver 3.8 ***/ /*** '*' changed to '0' in ver 3.9 ***/
		NVL(FOD_ESP_ID, '*'),						 /** Ver 4.1 **/
		NVL(FOD_ALGO_ID, '*'),						 /** Ver 4.1 **/
		NVL(FOD_SOURCE_FLG, '*')					 /** Ver 4.2 **/
		INTO : ptr_st_orderbook->c_cln_mtch_accnt,
		: ptr_st_orderbook->l_clnt_ctgry,
		: ptr_st_orderbook->c_pipe_id,
		: ptr_st_orderbook->c_xchng_cd,
		: ptr_st_orderbook->c_prd_typ,
		: ptr_st_orderbook->c_undrlyng,
		: ptr_st_orderbook->c_expry_dt,
		: ptr_st_orderbook->c_exrc_typ,
		: ptr_st_orderbook->c_opt_typ,
		: ptr_st_orderbook->l_strike_prc,
		: ptr_st_orderbook->c_ordr_flw,
		: ptr_st_orderbook->c_slm_flg,
		: ptr_st_orderbook->l_dsclsd_qty,
		: ptr_st_orderbook->l_ord_tot_qty,
		: ptr_st_orderbook->l_ord_lmt_rt,
		: ptr_st_orderbook->l_stp_lss_tgr,
		: ptr_st_orderbook->c_ord_typ,
		: ptr_st_orderbook->c_valid_dt : i_valid_dt,
		: ptr_st_orderbook->c_trd_dt,
		: ptr_st_orderbook->c_ordr_stts,
		: ptr_st_orderbook->l_exctd_qty,
		: ptr_st_orderbook->l_exctd_qty_day,
		: ptr_st_orderbook->l_can_qty,
		: ptr_st_orderbook->l_exprd_qty,
		: ptr_st_orderbook->c_sprd_ord_rfrnc : i_sprd_ord_rfrnc,
		: ptr_st_orderbook->l_mdfctn_cntr,
		: ptr_st_orderbook->c_settlor : i_settlor,
		: ptr_st_orderbook->c_xchng_ack : i_xchng_ack,
		: ptr_st_orderbook->c_spl_flg,
		: ptr_st_orderbook->c_ctgry_indstk,
		: ptr_st_orderbook->c_ack_tm : i_ack_tm,
		: ptr_st_orderbook->c_prev_ack_tm : i_prev_ack_tm,
		: ptr_st_orderbook->c_pro_cli_ind,
		: ptr_st_orderbook->c_ctcl_id,
		: ptr_st_orderbook->c_channel,
		: v_pan_no,			 /*** Ver 3.6 ***/
		: c_lst_act_ref_tmp, /*** Ver 3.8 ***/
		: v_esp_id,			 /** Ver 4.1 **/
		: v_algo_id,		 /** Ver 4.1 **/
		: c_source_flg_tmp	 /** Ver 4.2 **/
			  FROM fod_fo_ordr_dtls
				  WHERE fod_ordr_rfrnc = : ptr_st_orderbook->c_ordr_rfrnc
											   FOR UPDATE OF fod_ordr_rfrnc NOWAIT;

	if (SQLCODE != 0)
	{
		if (SQLCODE == -54)
		{
			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "In DISTRIBUTION_LOCK_ERR ");
			}
			fn_errlog(c_ServiceName, "S31475", SQLMSG, c_err_msg);
			return LOCK;
		}
		/*** added in ver 3.5 ***/
		else if (SQLCODE == -1405)
		{
			fn_userlog(c_ServiceName, "Inside NULL_CASE for ref_to_ord function ");
			c_sprd_ord_ind = ptr_st_orderbook->c_spl_flg;
			strcpy(c_ordr_rfrnc, ptr_st_orderbook->c_ordr_rfrnc);
			return NULL_CASE;
		}
		/*** Ver 3.5 ended ***/
		else
		{
			fn_errlog(c_ServiceName, "S31480", SQLMSG, c_err_msg);
			return -1;
		}
	}

	/*** Ver 3.6 Starts ***/
	SETNULL(v_pan_no);
	strcpy(c_pan_no, v_pan_no.arr);

	rtrim(c_lst_act_ref_tmp);				  /*** Ver 3.8 ***/
	strcpy(c_lst_act_ref, c_lst_act_ref_tmp); /***  Ver 3.8 ***/

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "Inside function fn_ref_to_ord c_pan_no is :%s:", c_pan_no);
		fn_userlog(c_ServiceName, "c_lst_act_ref_tmp :%s:", c_lst_act_ref_tmp); /*** Ver 3.8 ***/
		fn_userlog(c_ServiceName, "c_lst_act_ref :%s:", c_lst_act_ref);			/*** Ver 3.8 ***/
	}
	/*** Ver 3.6 Ends ***/

	/** Ver 4.1 Starts Here **/
	SETNULL(v_esp_id);
	SETNULL(v_algo_id);
	strcpy(c_esp_id, v_esp_id.arr);
	strcpy(c_algo_id, v_algo_id.arr);
	if (DEBUG_MSG_LVL_4)
	{
		fn_userlog(c_ServiceName, "c_esp_id :%s:, c_algo_id:%s:", c_esp_id, c_algo_id);
	}
	/*** Ver 4.1 Ends Here **/

	EXEC SQL
		SELECT NVL(RTRIM(CLM_CP_CD), ' '),
		NVL(RTRIM(CLM_CLNT_CD), CLM_MTCH_ACCNT)
			INTO : c_cp_code,
		: c_ucc_cd
			  FROM CLM_CLNT_MSTR
				  WHERE CLM_MTCH_ACCNT = : ptr_st_orderbook->c_cln_mtch_accnt;

	if (SQLCODE != 0)
	{
		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "match account is :%s:", ptr_st_orderbook->c_cln_mtch_accnt);
		}
		fn_errlog(c_ServiceName, "S31485", SQLMSG, c_err_msg);
		return -1;
	}

	SETNULL(c_ucc_cd);
	SETNULL(c_cp_code);

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "match account before is :%s:", ptr_st_orderbook->c_cln_mtch_accnt);
	}

	strcpy(ptr_st_orderbook->c_cln_mtch_accnt, (char *)c_ucc_cd.arr);
	strcpy(ptr_st_orderbook->c_settlor, (char *)c_cp_code.arr);

	rtrim(ptr_st_orderbook->c_expry_dt);
	rtrim(ptr_st_orderbook->c_ctcl_id);

	*c_source_flg = c_source_flg_tmp; /** Ver 4.2 **/

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "cp before is :%s:", c_cp_code.arr);
		fn_userlog(c_ServiceName, "jagan ctcl_id is :%s:", ptr_st_orderbook->c_ctcl_id);
		fn_userlog(c_ServiceName, "match account after is :%s:", ptr_st_orderbook->c_cln_mtch_accnt);
		fn_userlog(c_ServiceName, "cp after is :%s:", ptr_st_orderbook->c_settlor);
	}
	return 0;
}

int fn_upd_xchngbk(struct vw_xchngbook *ptr_st_xchngbook,
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
	int i_rec_exists;
	varchar c_xchng_rmrks[256];
	EXEC SQL END DECLARE SECTION;

	i_rec_exists = 0;

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Function UPD_XCHNGBK");
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, ":%s:", ptr_st_xchngbook->c_xchng_cd);
		fn_userlog(c_ServiceName, ":%s:", ptr_st_xchngbook->c_ordr_rfrnc);
		fn_userlog(c_ServiceName, ":%s:", ptr_st_xchngbook->c_pipe_id);
		fn_userlog(c_ServiceName, ":%s:", ptr_st_xchngbook->c_mod_trd_dt);
		fn_userlog(c_ServiceName, ":%ld:", ptr_st_xchngbook->l_ord_seq);
		fn_userlog(c_ServiceName, ":%c:", ptr_st_xchngbook->c_slm_flg);
		fn_userlog(c_ServiceName, ":%ld:", ptr_st_xchngbook->l_dsclsd_qty);
		fn_userlog(c_ServiceName, ":%ld:", ptr_st_xchngbook->l_ord_tot_qty);
		fn_userlog(c_ServiceName, ":%ld:", ptr_st_xchngbook->l_ord_lmt_rt);
		fn_userlog(c_ServiceName, ":%ld:", ptr_st_xchngbook->l_stp_lss_tgr);
		fn_userlog(c_ServiceName, ":%ld:", ptr_st_xchngbook->l_mdfctn_cntr);
		fn_userlog(c_ServiceName, ":%s:", ptr_st_xchngbook->c_valid_dt);
		fn_userlog(c_ServiceName, ":%c:", ptr_st_xchngbook->c_ord_typ);
		fn_userlog(c_ServiceName, ":%c:", ptr_st_xchngbook->c_sprd_ord_ind);
		fn_userlog(c_ServiceName, ":%c:", ptr_st_xchngbook->c_req_typ);
		fn_userlog(c_ServiceName, ":%ld:", ptr_st_xchngbook->l_quote);
		fn_userlog(c_ServiceName, ":%s:", ptr_st_xchngbook->c_qt_tm);
		fn_userlog(c_ServiceName, ":%s:", ptr_st_xchngbook->c_rqst_tm);
		fn_userlog(c_ServiceName, ":%c:", ptr_st_xchngbook->c_plcd_stts);
		fn_userlog(c_ServiceName, ":%c:", ptr_st_xchngbook->c_ex_ordr_typ);
		fn_userlog(c_ServiceName, ":%s:", ptr_st_xchngbook->c_xchng_rmrks);
		fn_userlog(c_ServiceName, ":%s:", ptr_st_xchngbook->c_mkrt_typ);
	}

	switch (ptr_st_xchngbook->c_oprn_typ)
	{
	case UPDATION_ON_ORDER_FORWARDING:
		EXEC SQL
			UPDATE fxb_fo_xchng_book
				SET fxb_frwd_tm = SYSDATE,
					fxb_plcd_stts = : ptr_st_xchngbook->c_plcd_stts
										  WHERE fxb_ordr_rfrnc = : ptr_st_xchngbook->c_ordr_rfrnc
																	   AND fxb_mdfctn_cntr = : ptr_st_xchngbook->l_mdfctn_cntr;

		break;

	case UPDATION_ON_EXCHANGE_RESPONSE:
		i_rec_exists = 0;
		if (ptr_st_xchngbook->l_dwnld_flg == DOWNLOAD)
		{
			EXEC SQL
				SELECT 1 INTO : i_rec_exists
									FROM fxb_fo_xchng_book
										WHERE FXB_JIFFY = : ptr_st_xchngbook->d_jiffy
																AND FXB_XCHNG_CD = : ptr_st_xchngbook->c_xchng_cd
																						 AND FXB_PIPE_ID = : ptr_st_xchngbook->c_pipe_id;

			if ((SQLCODE != 0) && (SQLCODE != NO_DATA_FOUND))
			{
				fn_errlog(c_ServiceName, "S31490", SQLMSG, c_err_msg);
				return -1;
			}
			if (i_rec_exists == 1)
			{
				fn_errlog(c_ServiceName, "S31495", "Record already Processed", c_err_msg);
				return -1;
			}
		}

		strcpy((char *)c_xchng_rmrks.arr, ptr_st_xchngbook->c_xchng_rmrks);
		rtrim(c_xchng_rmrks.arr);
		SETLEN(c_xchng_rmrks);

		EXEC SQL
			UPDATE fxb_fo_xchng_book
				SET fxb_plcd_stts = : ptr_st_xchngbook->c_plcd_stts,
					fxb_rms_prcsd_flg = : ptr_st_xchngbook->c_rms_prcsd_flg,
					fxb_ors_msg_typ = : ptr_st_xchngbook->l_ors_msg_typ,
					fxb_ack_tm = to_date( : ptr_st_xchngbook->c_ack_tm,
											'DD-Mon-yyyy hh24:mi:ss'),
					fxb_xchng_rmrks = rtrim(fxb_xchng_rmrks) || : c_xchng_rmrks,
					fxb_jiffy = : ptr_st_xchngbook->d_jiffy
									  WHERE fxb_ordr_rfrnc = : ptr_st_xchngbook->c_ordr_rfrnc
																   AND fxb_mdfctn_cntr = : ptr_st_xchngbook->l_mdfctn_cntr;
		break;

	default:
		fn_errlog(c_ServiceName, "S31500", "Invalid Operation Type", c_err_msg);
		return -1;
	}
	if (SQLCODE != 0)
	{
		if (SQLCODE == -2049)
		{
			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "In DISTRIBUTION_LOCK_ERR ");
			}
			return LOCK;
		}
		else
		{
			fn_errlog(c_ServiceName, "S31505", SQLMSG, c_err_msg);
			return -1;
		}
	}

	return 0;
}

int fn_upd_ordrbk(struct vw_orderbook *ptr_st_orderbook,
				  char *c_ServiceName,
				  char *c_err_msg)
{
	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Function UPD_ORDRBK");
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "c_ordr_stts:%c:", ptr_st_orderbook->c_ordr_stts);
		fn_userlog(c_ServiceName, "c_ordr_rfrnc:%s:", ptr_st_orderbook->c_ordr_rfrnc);
	}

	EXEC SQL
		UPDATE fod_fo_ordr_dtls
			SET fod_ordr_stts = : ptr_st_orderbook->c_ordr_stts
									  WHERE fod_ordr_rfrnc = : ptr_st_orderbook->c_ordr_rfrnc;

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31510", SQLMSG, c_err_msg);
		return -1;
	}
	return 0;
}

int fn_qry_spdbk(struct vw_spdordbk *ptr_st_spd_ordbk,   //nahi chahiye
				 char *c_ServiceName,
				 char *c_err_msg)
{
	int i_cnt = 0;

	EXEC SQL BEGIN DECLARE SECTION;
	sql_cursor sys_cursor;
	short i_ind;
	char c_xchng_ack[LEN_XCHNG_ACK];
	char c_pipe_id[2 + 1];
	char c_ordr_rfrnc[18 + 1];
	char c_sprd_ord_rfrnc[18 + 1];
	char c_ordr_rfrnc_tmp[18 + 1];
	char c_sprd_ord_ind;
	char c_xchng_ack_tmp[LEN_XCHNG_ACK];
	char c_pipe_id_tmp[2 + 1];
	long int l_mdfctn_cntr;
	long int l_ors_msg_typ;
	long int l_ord_tot_qty;
	long int l_exctd_qty;
	long int l_can_qty;
	varchar c_last_mod_tm[LEN_DATE];
	varchar c_ack_tm[LEN_DATE];

	EXEC SQL END DECLARE SECTION;

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Function QRY_SPDBK");
	}

	strcpy(c_ordr_rfrnc, (char *)ptr_st_spd_ordbk->c_ordr_rfrnc[0]);

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "c_ordr_rfrnc  is :%s:", c_ordr_rfrnc);
	}

	EXEC SQL ALLOCATE : sys_cursor;
	EXEC SQL EXECUTE
		BEGIN
			OPEN : sys_cursor FOR
					   SELECT fsd_sprd_rfrnc,
				   fsd_ordr_rfrnc,
				   fsd_sprd_ord_ind,
				   NVL(to_char(fsd_lst_mod_tm, 'dd-mon-yyyy hh24:mi:ss'), '*'),
				   NVL(fsd_ack_nmbr, ' '),
				   fsd_pipe_id,
				   fsd_mdfctn_cntr,
				   NVL(fsd_ors_msg_typ, 0),
				   NVL(fsd_ord_qty, 0),
				   NVL(fsd_exec_qty, 0),
				   NVL(fsd_cncl_qty, 0),
				   NVL(to_char(fsd_ack_tm, 'dd-mon-yyyy hh24:mi:ss'), '*')
					   FROM fsd_fo_sprd_dtls
						   WHERE fsd_sprd_rfrnc = (select distinct(fsd_sprd_rfrnc)
													   from fsd_fo_sprd_dtls
														   WHERE fsd_ordr_rfrnc = : c_ordr_rfrnc); /*** Ver 3.7 ***/
	/* WHERE  fsd_ordr_rfrnc = :c_ordr_rfrnc; *** Commented in Ver 3.7 ***/
	END;
	END - EXEC;

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31515", SQLMSG, c_err_msg);
		EXEC SQL FREE : sys_cursor;
		return -1;
	}

	for (i_cnt = 0; i_cnt <= 2; i_cnt++)
	{
		EXEC SQL FETCH : sys_cursor
							 INTO : c_sprd_ord_rfrnc,
			: c_ordr_rfrnc_tmp,
			: c_sprd_ord_ind,
			: c_last_mod_tm,
			: c_xchng_ack_tmp,
			: c_pipe_id_tmp,
			: l_mdfctn_cntr,
			: l_ors_msg_typ,
			: l_ord_tot_qty,
			: l_exctd_qty,
			: l_can_qty,
			: c_ack_tm;
		if (SQLCODE != 0)
		{
			if (SQLCODE == NO_DATA_FOUND)
			{
				fn_errlog(c_ServiceName, "S31520", SQLMSG, c_err_msg);
				if (DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName, "No data found in spread book for ordr_rfrnc :%s:", c_ordr_rfrnc);
				}
				break;
			}

			fn_errlog(c_ServiceName, "S31525", SQLMSG, c_err_msg);
			EXEC SQL CLOSE : sys_cursor;
			EXEC SQL FREE : sys_cursor;
			return -1;
		}

		strcpy(ptr_st_spd_ordbk->c_sprd_ord_rfrnc[i_cnt], c_sprd_ord_rfrnc);
		strcpy(ptr_st_spd_ordbk->c_ordr_rfrnc[i_cnt], c_ordr_rfrnc_tmp);
		ptr_st_spd_ordbk->c_sprd_ord_ind[i_cnt] = c_sprd_ord_ind;
		strcpy(ptr_st_spd_ordbk->c_xchng_ack[i_cnt], c_xchng_ack_tmp);
		strcpy(ptr_st_spd_ordbk->c_pipe_id[i_cnt], c_pipe_id_tmp);
		ptr_st_spd_ordbk->l_mdfctn_cntr[i_cnt] = l_mdfctn_cntr;
		ptr_st_spd_ordbk->l_ors_msg_typ[i_cnt] = l_ors_msg_typ;
		ptr_st_spd_ordbk->l_ord_tot_qty[i_cnt] = l_ord_tot_qty;
		ptr_st_spd_ordbk->l_exctd_qty[i_cnt] = l_exctd_qty;
		ptr_st_spd_ordbk->l_can_qty[i_cnt] = l_can_qty;

		SETNULL(c_last_mod_tm);
		SETNULL(c_ack_tm);

		if (strcmp((char *)c_last_mod_tm.arr, "*") == 0)
		{
			strcpy(ptr_st_spd_ordbk->c_lst_md_tm[i_cnt], " ");
		}
		else
		{
			strcpy(ptr_st_spd_ordbk->c_lst_md_tm[i_cnt], (char *)c_last_mod_tm.arr);
		}
		if (strcmp((char *)c_ack_tm.arr, "*") == 0)
		{
			strcpy(ptr_st_spd_ordbk->c_ack_tm[i_cnt], " ");
		}
		else
		{
			strcpy(ptr_st_spd_ordbk->c_ack_tm[i_cnt], (char *)c_ack_tm.arr);
		}
	}

	if (i_cnt == 0)
	{
		EXEC SQL EXECUTE
			BEGIN
				OPEN : sys_cursor FOR
						   SELECT fsd_sprd_rfrnc,
					   fsd_ordr_rfrnc,
					   fsd_sprd_ord_ind,
					   NVL(to_char(fsd_lst_mod_tm, 'dd-mon-yyyy hh24:mi:ss'), '*'),
					   NVL(fsd_ack_nmbr, ' '),
					   fsd_pipe_id,
					   fsd_mdfctn_cntr,
					   NVL(fsd_ors_msg_typ, 0),
					   NVL(fsd_ord_qty, 0),
					   NVL(fsd_exec_qty, 0),
					   NVL(fsd_cncl_qty, 0),
					   NVL(to_char(fsd_ack_tm, 'dd-mon-yyyy hh24:mi:ss'), '*')
						   FROM fsd_fo_sprd_dtls_hstry
							   WHERE fsd_sprd_rfrnc = (select distinct(fsd_sprd_rfrnc)
														   from fsd_fo_sprd_dtls_hstry
															   WHERE fsd_ordr_rfrnc = : c_ordr_rfrnc); /*** Ver 3.7 ***/
																									   /* WHERE   fsd_ordr_rfrnc = :c_ordr_rfrnc; *** Commented in Ver 3.7 ***/
		END;
		END - EXEC;

		if (SQLCODE != 0)
		{
			fn_errlog(c_ServiceName, "S31530", SQLMSG, c_err_msg);
			EXEC SQL FREE : sys_cursor;
			return -1;
		}

		for (i_cnt = 0; i_cnt <= 2; i_cnt++)
		{
			EXEC SQL FETCH : sys_cursor
								 INTO : c_sprd_ord_rfrnc,
				: c_ordr_rfrnc_tmp,
				: c_sprd_ord_ind,
				: c_last_mod_tm,
				: c_xchng_ack_tmp,
				: c_pipe_id_tmp,
				: l_mdfctn_cntr,
				: l_ors_msg_typ,
				: l_ord_tot_qty,
				: l_exctd_qty,
				: l_can_qty,
				: c_ack_tm;
			if (SQLCODE != 0)
			{
				if (SQLCODE == NO_DATA_FOUND)
				{
					break;
				}

				fn_errlog(c_ServiceName, "S31535", SQLMSG, c_err_msg);
				EXEC SQL CLOSE : sys_cursor;
				EXEC SQL FREE : sys_cursor;
				return -1;
			}
			strcpy(ptr_st_spd_ordbk->c_sprd_ord_rfrnc[i_cnt], c_sprd_ord_rfrnc);
			strcpy(ptr_st_spd_ordbk->c_ordr_rfrnc[i_cnt], c_ordr_rfrnc_tmp);
			ptr_st_spd_ordbk->c_sprd_ord_ind[i_cnt] = c_sprd_ord_ind;
			strcpy(ptr_st_spd_ordbk->c_xchng_ack[i_cnt], c_xchng_ack_tmp);
			strcpy(ptr_st_spd_ordbk->c_pipe_id[i_cnt], c_pipe_id_tmp);
			ptr_st_spd_ordbk->l_mdfctn_cntr[i_cnt] = l_mdfctn_cntr;
			ptr_st_spd_ordbk->l_ors_msg_typ[i_cnt] = l_ors_msg_typ;
			ptr_st_spd_ordbk->l_ord_tot_qty[i_cnt] = l_ord_tot_qty;
			ptr_st_spd_ordbk->l_exctd_qty[i_cnt] = l_exctd_qty;
			ptr_st_spd_ordbk->l_can_qty[i_cnt] = l_can_qty;

			SETNULL(c_last_mod_tm);
			SETNULL(c_ack_tm);
			if (strcmp((char *)c_last_mod_tm.arr, "*") == 0)
			{
				strcpy(ptr_st_spd_ordbk->c_lst_md_tm[i_cnt], " ");
			}
			else
			{
				strcpy(ptr_st_spd_ordbk->c_lst_md_tm[i_cnt], (char *)c_last_mod_tm.arr);
			}
			if (strcmp((char *)c_ack_tm.arr, "*") == 0)
			{
				strcpy(ptr_st_spd_ordbk->c_ack_tm[i_cnt], " ");
			}
			else
			{
				strcpy(ptr_st_spd_ordbk->c_ack_tm[i_cnt], (char *)c_ack_tm.arr);
			}
		}
		if (i_cnt == 0)
		{
			fn_errlog(c_ServiceName, "S31540", "No data found in spread book history", c_err_msg);
			EXEC SQL FREE : sys_cursor;
			return -1;
		}
	}
	while (i_cnt >= 0)
	{
		if (DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName, "Printing the output structure :%d:", i_cnt);

			fn_userlog(c_ServiceName, "c_sprd_ord_rfrnc :%s: (%d)", ptr_st_spd_ordbk->c_sprd_ord_rfrnc[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "c_ordr_rfrnc  :%s: (%d)", ptr_st_spd_ordbk->c_ordr_rfrnc[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "c_sprd_ord_ind  :%c: (%d)", ptr_st_spd_ordbk->c_sprd_ord_ind[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "c_lst_md_tm :%s: (%d)", ptr_st_spd_ordbk->c_lst_md_tm[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "c_xchng_ack :%s: (%d)", ptr_st_spd_ordbk->c_xchng_ack[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "c_pipe_id :%s: (%d)", ptr_st_spd_ordbk->c_pipe_id[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "l_mdfctn_cntr :%ld: (%d)", ptr_st_spd_ordbk->l_mdfctn_cntr[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "l_ors_msg_typ :%ld: (%d)", ptr_st_spd_ordbk->l_ors_msg_typ[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "l_ord_tot_qty :%ld: (%d)", ptr_st_spd_ordbk->l_ord_tot_qty[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "l_exctd_qty :%ld: (%d)", ptr_st_spd_ordbk->l_exctd_qty[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "l_can_qty :%ld: (%d)", ptr_st_spd_ordbk->l_can_qty[i_cnt], i_cnt);
			fn_userlog(c_ServiceName, "c_ack_tm  :%s: (%d)", ptr_st_spd_ordbk->c_ack_tm[i_cnt], i_cnt);
		}
		i_cnt--;
	}

	EXEC SQL CLOSE : sys_cursor;
	EXEC SQL FREE : sys_cursor;
	return 0;
}

int fn_exrq_dtls(struct vw_exrcbook *ptr_st_exrcbook,  //nahi chahiye
				 char *c_ServiceName,
				 char *c_err_msg)
{
	EXEC SQL BEGIN DECLARE SECTION;
	varchar c_expiry_dt[LEN_DATE];
	varchar c_trade_dt[LEN_DATE];
	varchar c_ucc_cd[11];
	varchar c_cp_code[11 + 2];
	short i_settlor;
	short i_xchng_ack;
	short i_ack_tm;
	EXEC SQL END DECLARE SECTION;

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Function EXRQ_DTLS");
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "Exercise Ref. No:%s:", ptr_st_exrcbook->c_exrc_ordr_rfrnc);
	}

	EXEC SQL
		SELECT feb_clm_mtch_accnt,
		feb_clnt_ctgry,
		feb_pipe_id,
		feb_xchng_cd,
		feb_prdct_typ,
		feb_indstk,
		feb_undrlyng,
		to_char(feb_expry_dt, 'dd-Mon-yyyy'),
		feb_exer_typ,
		feb_opt_typ,
		feb_strk_prc,
		feb_exrc_rqst_typ,
		feb_exrc_qty,
		feb_exrc_stts,
		to_char(feb_trd_dt, 'dd-Mon-yyyy'),
		feb_mdfctn_cntr,
		feb_settlor,
		feb_ack_nmbr,
		to_char(feb_exer_ack_tm, 'dd-Mon-yyyy hh24:mi:ss'),
		feb_pro_cli_ind
			INTO : ptr_st_exrcbook->c_cln_mtch_accnt,
		: ptr_st_exrcbook->l_clnt_ctgry,
		: ptr_st_exrcbook->c_pipe_id,
		: ptr_st_exrcbook->c_xchng_cd,
		: ptr_st_exrcbook->c_prd_typ,
		: ptr_st_exrcbook->c_ctgry_indstk,
		: ptr_st_exrcbook->c_undrlyng,
		: ptr_st_exrcbook->c_expry_dt,
		: ptr_st_exrcbook->c_exrc_typ,
		: ptr_st_exrcbook->c_opt_typ,
		: ptr_st_exrcbook->l_strike_prc,
		: ptr_st_exrcbook->c_exrc_rqst_typ,
		: ptr_st_exrcbook->l_exrc_qty,
		: ptr_st_exrcbook->c_exrc_stts,
		: c_trade_dt,
		: ptr_st_exrcbook->l_mdfctn_cntr,
		: ptr_st_exrcbook->c_settlor : i_settlor,
		: ptr_st_exrcbook->c_xchng_ack : i_xchng_ack,
		: ptr_st_exrcbook->c_ack_tm : i_ack_tm,
		: ptr_st_exrcbook->c_pro_cli_ind
			  FROM feb_fo_exrc_rqst_book
				  WHERE feb_exrc_rfrnc_no = : ptr_st_exrcbook->c_exrc_ordr_rfrnc
												  FOR UPDATE OF feb_exrc_rfrnc_no NOWAIT;

	if ((SQLCODE != 0) && (SQLCODE != NO_DATA_FOUND))
	{
		if (SQLCODE == -54)
		{
			if (DEBUG_MSG_LVL_0)
			{
				fn_userlog(c_ServiceName, "In DISTRIBUTION_LOCK_ERR ");
			}
			fn_errlog(c_ServiceName, "S31545", SQLMSG, c_err_msg);
			return LOCK;
		}
		else
		{
			fn_errlog(c_ServiceName, "S31550", SQLMSG, c_err_msg);
			return -1;
		}
	}

	EXEC SQL
		SELECT NVL(RTRIM(CLM_CP_CD), ' '),
		NVL(RTRIM(CLM_CLNT_CD), CLM_MTCH_ACCNT)
			INTO : c_cp_code,
		: c_ucc_cd
			  FROM CLM_CLNT_MSTR
				  WHERE CLM_MTCH_ACCNT = : ptr_st_exrcbook->c_cln_mtch_accnt;

	if (SQLCODE != 0)
	{
		if (DEBUG_MSG_LVL_0)
		{
			fn_userlog(c_ServiceName, "match account is :%s:", ptr_st_exrcbook->c_cln_mtch_accnt);
		}
		fn_errlog(c_ServiceName, "S31555", SQLMSG, c_err_msg);
		return -1;
	}

	SETNULL(c_ucc_cd);
	SETNULL(c_cp_code);

	strcpy(ptr_st_exrcbook->c_cln_mtch_accnt, (char *)c_ucc_cd.arr);
	strcpy(ptr_st_exrcbook->c_settlor, (char *)c_cp_code.arr);

	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "match account is :%s:", ptr_st_exrcbook->c_cln_mtch_accnt);
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "cp before is :%s:", c_cp_code.arr);
		fn_userlog(c_ServiceName, "match account after is :%s:", ptr_st_exrcbook->c_cln_mtch_accnt);
		fn_userlog(c_ServiceName, "cp after is :%s:", ptr_st_exrcbook->c_settlor);
	}

	SETNULL(c_trade_dt);
	strcpy(ptr_st_exrcbook->c_trd_dt, (char *)c_trade_dt.arr);
	rtrim(ptr_st_exrcbook->c_expry_dt);

	return 0;
}

int fn_upd_exbk(struct vw_exrcbook *ptr_st_exrcbook,  //nahi chahiye
				char *c_ServiceName,
				char *c_err_msg)
{
	if (DEBUG_MSG_LVL_1)
	{
		fn_userlog(c_ServiceName, "Function UPD_EXBK");
	}

	if (DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "Exercise order reference in :%s:", ptr_st_exrcbook->c_exrc_ordr_rfrnc);
	}

	EXEC SQL
		UPDATE feb_fo_exrc_rqst_book
			SET feb_exrc_stts = : ptr_st_exrcbook->c_exrc_stts
									  WHERE feb_exrc_rfrnc_no = : ptr_st_exrcbook->c_exrc_ordr_rfrnc;

	if (SQLCODE != 0)
	{
		fn_errlog(c_ServiceName, "S31560", SQLMSG, c_err_msg);
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
	tm = localtime(&tv.tv_sec);
	i_mili_sec = tv.tv_usec / 1000;
	sprintf(c_time, "%d:%d:%d:%d", tm->tm_hour, tm->tm_min, tm->tm_sec, i_mili_sec);
	return;
}

/****************** Ver 1.2 Ends ********************************/
/************** Ver 3.5 starts***************/
int fn_flr_ord_plc(char *c_ordr_rfrnc,
				   char c_sprd_ord_ind,
				   char *c_ServiceName,
				   char *c_err_msg)
{
	int i_trnsctn = 0;

	fn_userlog(c_ServiceName, "Inside function ==> fn_flr_ord_plc:");
	i_trnsctn = fn_begintran(c_ServiceName, c_err_msg);
	if (i_trnsctn == -1)
	{
		fn_errlog(c_ServiceName, "S31565", TPMSG, c_err_msg);
		return -1;
	}

	if (c_sprd_ord_ind != SPREAD_ORDER &&
		c_sprd_ord_ind != ROLLOVER_WITH_SPREAD && /** Ver 4.4 **/
		c_sprd_ord_ind != ROLLOVER &&			  /** Ver 4.4 **/
		c_sprd_ord_ind != L2_ORDER &&
		c_sprd_ord_ind != L3_ORDER)
	{
		EXEC SQL
			UPDATE FXB_FO_XCHNG_BOOK
				SET FXB_PLCD_STTS = 'M' WHERE FXB_ORDR_RFRNC = : c_ordr_rfrnc
																	 AND FXB_PLCD_STTS = 'R';
		if (SQLCODE != 0 && SQLCODE != NO_DATA_FOUND) /** Ver 4.4, NO_DATA_FOUND added **/
		{
			fn_userlog(c_ServiceName, ":In a function ==> fn_flr_ord_plc: Query 1  failed");
			fn_errlog(c_ServiceName, "S31570", SQLMSG, c_err_msg);
			fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
			return -1;
		}
	}
	else
	{
		EXEC SQL
			UPDATE FXB_FO_XCHNG_BOOK
				SET FXB_PLCD_STTS = 'M' WHERE FXB_ORDR_RFRNC
				IN(SELECT FSD_ORDR_RFRNC
					   FROM FSD_FO_SPRD_DTLS
						   WHERE FSD_SPRD_RFRNC = (SELECT FSD_SPRD_RFRNC
													   FROM FSD_FO_SPRD_DTLS
														   WHERE FSD_ORDR_RFRNC = : c_ordr_rfrnc))
					AND FXB_PLCD_STTS = 'R';

		if (SQLCODE != 0 && SQLCODE != NO_DATA_FOUND) /** Ver 4.4, NO_DATA_FOUND added **/
		{
			fn_userlog(c_ServiceName, "Inside function fn_flr_ord_plc :Query 2:  failed");
			fn_errlog(c_ServiceName, "S31575", SQLMSG, c_err_msg);
			fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg);
			return -1;
		}
	}

	if (fn_committran(c_ServiceName, i_trnsctn, c_err_msg) == -1)
	{
		fn_userlog(c_ServiceName, "CRUCIAL COMMIT FAILED !!!!! :In a function fn_flr_ord_plc:");
		return -1;
	}
	return 0;
}
/******************* 3.5 ends *****************/
