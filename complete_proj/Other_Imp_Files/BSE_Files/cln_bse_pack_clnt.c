/**********************************************************************************/
/*  Program           : CLN_BSE_PACK_CLNT                                      		*/
/*                                                                            		*/
/*  Input             : C_PIPE_ID                                             		*/
/*                                                                            		*/
/*  Output            :                                                       		*/
/*                                                                            		*/
/*  Description       : Pack client is a tuxedo client process. It picks up   		*/
/*											orders from database, packs them into exchange 						*/
/*											specific format and puts them in send queue.							*/
/*                                                                            		*/
/* Log                :New Release : Sachin Birje.                                */
/* Log                :1.1		18-Dec-2012		Navina D.															*/
/* Log                :1.3    30-Sep-2013   Navina D.                             */
/* Log                :1.4    30-Nov-2013   Sachin B.                             */
/* Log                :1.6    12-Aug-2015   Anand Dhopte                          */
/* Log                :1.7    25-Sep-2023   Ravi Malla	                          */
/* Log                :1.8    27-Dec-2023   Prajakta Sutar                        */
/**********************************************************************************/
/*  1.1   -   SLTP FuturePLUS handling                                            */
/*  1.3   -   OptionPLUS handling                                                 */
/*  1.4   -   FNO VTC Changes                                                     */
/*  1.6   -   Contract Master and Trade Quote Table Merger Changes                */
/*  1.7   -   BSE New ETI																					                */
/*  1.8   -   Download changes for BSE derivatives                                */
/**********************************************************************************/

/**** C headers ****/

#include <stdio.h>
#include <stdlib.h>    
#include <string.h>   
#include <unistd.h>  
#include <ctype.h>
#include <time.h>

#include <netinet/in.h>
#include <fcntl.h>

/**** Added for Pro c codes ****/
#include <sqlca.h>

/**** Tuxedo headers ****/
#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>


/***** Application headers *****/
#include <fo_exg_bse_pack_lib.h>
#include <fn_msgq.h>
#include <fo.h>
#include <fn_env.h>
#include <fn_battmpt.h>
#include <fn_tuxlib.h>
#include <fml_rout.h>  
#include <fn_ddr.h>
#include <fn_log.h>
#include <fn_scklib_tap.h>
#include <fn_read_debug_lvl.h> 
#include <fo_view_def.h>
#include<fn_fo_bse_convrtn.h>

#define LOCK 2049  

/**** Global Variables ****/

long int li_seq_nm = 0;
int i_key =0;
int i_qid=0;
long int li_max_q=0;
long int li_cnt_q = 0;			
int i_queue_chk_flg = 1;			

int i_ord_mkt_stts;
int i_exr_mkt_stts;
int i_pmp_stts;
int i_upd_stts;


int i_part_stts;
int i_brkr_stts;
int i_exit_stts;

int i_reco_stts;
int i_reco_stts_ord = 0; /**  Ver 1.8  **/
int i_reco_stts_trd = 0; /**  Ver 1.8  **/
int i_ord_lmt_val = 0;
char c_filter2[32];
struct st_send_rqst_data st_q_packet;
struct st_send_rqst_data st_sndq_packets[20]; 
int i_pack_counter = 0;
char c_time[12];
int i_pack_val = 0;

EXEC SQL BEGIN DECLARE SECTION;
	
	char sql_c_xchng_ctcl_id[LEN_CTCL_ID];
	char sql_c_xchng_trdr_id [ LEN_TRDR_ID ];
	char sql_c_xchng_cd[3+1];
	char sql_c_xchng_brkr_id[LEN_BRKR_ID];
	varchar sql_c_nxt_trd_date[ LEN_DATE ];
	char sql_c_pipe_id [ 2+1 ];
	struct st_opm_pipe_mstr st_opm_mstr;
  long sql_li_max_pnd_ord;

EXEC SQL END DECLARE SECTION;


void fn_gettime(void);

int fn_dwnld_rqst( char c_reco_mode,
                  char *c_tm_stmp,
                  char *c_ServiceName,
                  char *c_err_msg
                  );

int fn_crt_dwnld_rqst(short int si_rqst_type,
                        char *c_tm_stmp,
                        char *c_ServiceName,
                  char *c_err_msg);


int fn_get_nxt_bse_rec ( char *c_ServiceName,
                     char *c_err_msg );

int fn_rjct_rcrd ( struct vw_xchngbook *ptr_st_rqst,
                   struct vw_orderbook *ptr_st_ord,
                   char *c_ServiceName,
                   char *c_err_msg );

int fn_write_packets_q( char *c_ServiceName,
                     		char *c_err_msg
                    	);

int fn_get_bse_scrip_cd( struct vw_contract *ptr_st_cnt,
                    struct vw_nse_cntrct *ptr_st_nse_cnt,
                    char *c_ServiceName,
                    char *c_err_msg);

int fn_seq_to_bseomd(struct vw_xchngbook *ptr_st_xchngbook,
                  char *c_ServiceName,
                  char *c_err_msg);

int fn_bseref_to_ord(struct vw_orderbook *ptr_st_orderbook,
                  char *c_ServiceName,
                  char *c_err_msg);

int fn_upd_bse_xchngbk( struct vw_xchngbook *ptr_st_xchngbook,
                    char *c_ServiceName,
                    char *c_err_msg);

int fn_upd_bse_ordrbk(struct vw_orderbook *ptr_st_orderbook,
                  char *c_ServiceName,
                  char *c_err_msg);

int fn_qry_spdbk( struct vw_spdordbk *ptr_st_spd_ordbk,
                  char *c_ServiceName,
                  char *c_err_msg);

int fn_exrq_dtls( struct vw_exrcbook *ptr_st_exrcbook,
                  char *c_ServiceName,
                  char *c_err_msg);

int fn_upd_exbk( struct vw_exrcbook *ptr_st_exrcbook,
                 char *c_ServiceName,
                 char *c_err_msg);


void CLN_BSE_PACK_CLNT ( int argc, char *argv[] )
{
		int i_trnsctn;	
		int i_ch_val;
		int i_lmt_counter = 1;
		int i_in_tran_flg;
		char c_ServiceName[33];
		char c_err_msg [ 256 ];

		strcpy( c_ServiceName, argv[0] );

		INITBATDBGLVL(c_ServiceName);

		sprintf(st_opm_mstr.c_opm_pipe_id, argv[3] );
		
 	  if(DEBUG_MSG_LVL_0)
		{ 
	 	 fn_userlog(c_ServiceName," In function CLN_BSE_PACK_CLNT %s:",c_ServiceName);
		}
		if(DEBUG_MSG_LVL_3)
		{ 
	 	 fn_userlog(c_ServiceName,"Service name is is :%s:",c_ServiceName);
		 fn_userlog(c_ServiceName,"The pipe id is :%s:",st_opm_mstr.c_opm_pipe_id);
		} 

		i_in_tran_flg = 0;

		while (i_exit_stts == DONT_EXIT)
		{

			while ((( i_ord_mkt_stts == TRUE ) ||
				( i_exr_mkt_stts == TRUE ))&&
				( i_pmp_stts  == TRUE &&
					i_part_stts == TRUE &&
					i_brkr_stts == TRUE ))
			{
				if ( i_in_tran_flg == 0 )
				{
					/***** Begin Transaction *********/

					i_trnsctn = fn_begintran( c_ServiceName, c_err_msg );

					if ( i_trnsctn == -1)
					{
						fn_errlog(c_ServiceName,"S31005", TPMSG, c_err_msg);
						return;
					}
					i_in_tran_flg = 1;
				}

				i_ch_val = fn_get_nxt_bse_rec (c_ServiceName,c_err_msg );

				if(DEBUG_MSG_LVL_0)
				{ 
					fn_userlog(c_ServiceName,"Before Switch");
				}

				switch (i_ch_val) 
				{
					case NDF: 
						if(DEBUG_MSG_LVL_0)
						{ 
							fn_userlog(c_ServiceName,"In NDF case");
						}
						if( DEBUG_MSG_LVL_3 )   
						{
							fn_gettime();
							fn_userlog(c_ServiceName,"TIME_STATS NDF after order at :%s:",c_time);
						}  
						if ( i_pack_counter > 0 )  
					  {
							if ( fn_committran( c_ServiceName, i_trnsctn, c_err_msg ) == -1 )
							{
								if(DEBUG_MSG_LVL_0)
								{ 
									fn_userlog(c_ServiceName,"CRUCIAL COMMIT FAILED !!!!! [NDF]");
								}
								fn_errlog(c_ServiceName, "S31010", LIBMSG, c_err_msg);
								return;
							}
							
							if ( fn_write_packets_q( c_ServiceName, c_err_msg) == -1)
              {
                fn_errlog(c_ServiceName, "S31015", LIBMSG, c_err_msg);
                return;
              }
						}	
						else
						{
							fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg );
						} 

						i_in_tran_flg = 0;

						sleep(1); /******Put for reducing the resource utilization***/
						break;

					case OVER_LOAD:
						if(DEBUG_MSG_LVL_0)
						{ 
							fn_userlog( c_ServiceName, "OVER LOAD:Waiting for Release pack_counter is :%d:",i_pack_counter);
						}

						if( DEBUG_MSG_LVL_3 )
						{
						 	fn_gettime();
							fn_userlog(c_ServiceName,"TIME_STATS OVERLOAD after order at :%s:",c_time);
						}

						if ( i_pack_counter > 0 )  
					  {
							if ( fn_committran( c_ServiceName, i_trnsctn, c_err_msg ) == -1 )
							{
								if(DEBUG_MSG_LVL_0)
								{ 
									fn_userlog(c_ServiceName,"CRUCIAL COMMIT FAILED !!!!! [OVERLOAD]");
								}
								fn_errlog(c_ServiceName, "S31020", LIBMSG, c_err_msg);
								return;
							}
						 	
							if ( fn_write_packets_q( c_ServiceName, c_err_msg) == -1)
              {
                fn_errlog(c_ServiceName, "S31025", LIBMSG, c_err_msg);
                return;
              } 
						}	
						else
						{
							fn_aborttran(c_ServiceName, i_trnsctn, c_err_msg );
						}
						i_in_tran_flg = 0; 
						sleep(1);
						if( DEBUG_MSG_LVL_0 ) 
						{
							fn_userlog(c_ServiceName,"Resuming after OVER LOAD wait.....,pack counter is :%d:",i_pack_counter);
						}
						if( DEBUG_MSG_LVL_3 )  
						{
							fn_gettime();
							fn_userlog(c_ServiceName,"TIME_STATS OVERLOAD released at :%s:",c_time);
						} 
						break;

					case DATA_RTRVD:

						
						memcpy(&st_sndq_packets[i_pack_counter], &st_q_packet, sizeof(st_q_packet));

						if( DEBUG_MSG_LVL_3 )  
						{
							fn_gettime();
							fn_userlog(c_ServiceName,"TIME_STATS order packed at :%s:", c_time);
						}

						i_pack_counter++;

						if( DEBUG_MSG_LVL_0 ) 
						{
							fn_userlog(c_ServiceName,"In DATA RTRVD case");
						}

						if ( i_lmt_counter <= i_ord_lmt_val ) 
						{
						  i_lmt_counter++;

							if( DEBUG_MSG_LVL_0 ) 
							{
								fn_userlog(c_ServiceName,"In Initial forwarding loop ,Pack counter is :%d:",i_pack_counter);
							}

             if ( i_pack_counter >= i_pack_val || i_lmt_counter > i_ord_lmt_val )
             {

							if ( fn_committran( c_ServiceName, i_trnsctn, c_err_msg ) == -1 )
							{
								if( DEBUG_MSG_LVL_0 ) 
								{
									fn_userlog(c_ServiceName,"CRUCIAL COMMIT FAILED !!!!! [DATA RETRIEVED Initial forwarding]");
								}
							 	fn_errlog(c_ServiceName, "S31030", LIBMSG, c_err_msg);
								return;
						  }

					 		i_in_tran_flg = 0;
					  
							if ( fn_write_packets_q( c_ServiceName, c_err_msg) == -1)
							{
								fn_errlog(c_ServiceName, "S31035", LIBMSG, c_err_msg);
								return;
							}		
							if( DEBUG_MSG_LVL_3 ) 
							{
								fn_gettime();
								fn_userlog(c_ServiceName,"TIME_STATS After writing packets in queue :%s:",c_time);
							}
						
             }	
						 if ( i_lmt_counter > i_ord_lmt_val )
  					 {
							i_pmp_stts = FALSE;
							i_ord_lmt_val =0;
  					 } 		
							
						}
						else 
						{
							if( DEBUG_MSG_LVL_0 ) 
							{
								fn_userlog(c_ServiceName,"In Normal forwarding loop ,Pack counter is :%d:",i_pack_counter);
							}
             
              if ( i_pack_counter >= i_pack_val )
              {
 
							 if ( fn_committran( c_ServiceName, i_trnsctn, c_err_msg ) == -1 )
							 {
								if( DEBUG_MSG_LVL_0 ) 
								{
									fn_userlog(c_ServiceName,"CRUCIAL COMMIT FAILED !!!!! [DATA RETRIEVED Normal forwarding]");
								}
								fn_errlog(c_ServiceName, "S31040", LIBMSG, c_err_msg);
								return;
							 }

	 						 i_in_tran_flg = 0;
						 	
							 if ( fn_write_packets_q( c_ServiceName, c_err_msg) == -1)
               {
                 fn_errlog(c_ServiceName, "S31045", LIBMSG, c_err_msg);
                 return;
               } 
							 if( DEBUG_MSG_LVL_3 ) 
							 {
								fn_gettime();
								fn_userlog(c_ServiceName,"TIME_STATS After writing packets in queue :%s:",c_time);
							 }
              }
						}
				
						break;

					case DATA_CNCLD:
					if( DEBUG_MSG_LVL_0 ) 
						{
							fn_userlog(c_ServiceName,"In DATA CANCELLED case: do nothing");
						}
						break; 

					case DATA_SKPD:
						if( DEBUG_MSG_LVL_0 ) 
						{
							fn_userlog(c_ServiceName,"In DATA SKIPPED case: do nothing");
						}
						break; 

					case TKN_NA :
						if( DEBUG_MSG_LVL_0 ) 
						{
							fn_userlog(c_ServiceName,"In Token not avaialble case: do nothing");
						}
						break;

					case LOCK_ERR :												
						if( DEBUG_MSG_LVL_0 ) 
						{
							fn_userlog(c_ServiceName,"In LOCK ERROR case: Sleep for 1 second");
						}
						if( DEBUG_MSG_LVL_3 )  
						{
							fn_gettime();
							fn_userlog(c_ServiceName,"TIME_STATS LOCK after order :%d: at :%s:",c_time);
						} 
						i_in_tran_flg = 0;
						i_pack_counter = 0;
						memset(&st_sndq_packets, '\0', sizeof(st_sndq_packets));
						fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg ); 
						sleep(1);
						if( DEBUG_MSG_LVL_3 ) 
						{
							fn_gettime();
							fn_userlog(c_ServiceName,"TIME_STATS LOCK wait ends at :%s:",c_time);
						} 
						break;

					case ERROR: 
						if( DEBUG_MSG_LVL_0 ) 
						{
							fn_userlog(c_ServiceName,"In ERROR case: Abort transaction");
						}
						fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg ); 
						return;

					default :
						fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg ); 
						if( DEBUG_MSG_LVL_0 ) 
						{
							fn_userlog ( c_ServiceName, "Logic error : unknown return type" );
						}
						return;
				} 

				/**** check for unsolicited messages ****/
				i_ch_val = fn_chk_stts ( c_ServiceName ) ;

				if ( i_ch_val == -1 )
				{
					fn_errlog(c_ServiceName,"S31050", LIBMSG, c_err_msg);
					return;
				} 
			} /******** End of 2nd While Loop *******/

			if ( i_in_tran_flg == 1 )
			{
				i_in_tran_flg = 0;
				i_pack_counter = 0;  
				memset(&st_sndq_packets, '\0', sizeof(st_sndq_packets)); 
				fn_aborttran( c_ServiceName, i_trnsctn, c_err_msg ); 
			}

			/**** check for unsolicited messages  ****/
			sleep(1);

			i_ch_val = fn_chk_stts ( c_ServiceName ) ;

			if ( i_ch_val == -1 )
			{
				fn_errlog(c_ServiceName,"S31055", LIBMSG, c_err_msg);
				return;
			}

			if ( i_upd_stts == TRUE )
			{
				i_trnsctn = fn_begintran( c_ServiceName, c_err_msg );

				if ( i_trnsctn == -1)
				{
					fn_errlog(c_ServiceName,"S31060", LIBMSG, c_err_msg);
					return;
				}

				if ( i_pmp_stts == TRUE )
				{	
					EXEC SQL
					UPDATE opm_ord_pipe_mstr
					SET opm_pmp_stts = 1
					WHERE opm_pipe_id = :sql_c_pipe_id;

					if ( SQLCODE != 0 )
					{
						fn_errlog(c_ServiceName,"S31065", SQLMSG, c_err_msg);
						return;
					}
				}
				else
				{
					EXEC SQL
					UPDATE opm_ord_pipe_mstr
					SET opm_pmp_stts = 0
					WHERE opm_pipe_id = :sql_c_pipe_id;

					if ( SQLCODE != 0 )
					{
						fn_errlog(c_ServiceName,"S31070", SQLMSG, c_err_msg);
						return;
					}
				}

				i_ch_val = fn_committran( c_ServiceName, i_trnsctn, c_err_msg );
	
				if ( i_ch_val == -1 )
				{
					fn_errlog(c_ServiceName,"S31075", LIBMSG, c_err_msg);
					return;
				}

				i_upd_stts = FALSE;
			}
		} /******* End of 1st While Loop *********/      
	}

int fn_bat_init ( int argc, char *argv[] )
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

		char *ptr_c_tmp;
		char c_ServiceName [ 33 ];
		char c_err_msg [ 256 ];
		char c_filter1[32];
		char c_reco_mode;
		char c_tm_stmp [ LEN_DT ];

		struct vw_sequence  st_seq;

		EXEC SQL BEGIN DECLARE SECTION;
		char sql_exg_settlor_stts;
		char sql_exg_brkr_stts;
		char sql_exg_crrnt_stts;
		char sql_exg_extnd_mrkt_stts;
		char sql_exg_exrc_mkt_stts;
		EXEC SQL END DECLARE SECTION;

		memset(&st_sndq_packets, '\0', sizeof(st_sndq_packets)); 
		strcpy ( c_ServiceName, "cln_bse_pack_clnt" );

		if ( argc < 6 ) 
		{
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, 
				"Usage - cln_snd_clnt <tag qualifier> <Exchange cd> <Pipe id> <ord_lmt_val> <Reco mode> <O-Time stamp>" );
			}
			return ( -1 );
		}

		strcpy( sql_c_pipe_id, argv[3] );
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog(c_ServiceName,"The pipe id is :%s:",sql_c_pipe_id);
		}
		c_reco_mode = argv[5][0];
		i_ord_lmt_val = atoi(argv[4]);

		/*** Initialize the Routing string ***/
		fn_init_ddr_pop ( argv[3],TRADING_SECTION,COMMON);

		/***  Get IPC key  ***/
		ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName, "SEND_QUEUE");
		if ( ptr_c_tmp == NULL )
		{
  		fn_errlog(c_ServiceName,"S31080", LIBMSG, c_err_msg);
  		return -1;
		}
		i_key = atoi(ptr_c_tmp);

    fn_userlog(c_ServiceName,"SEND_QUEUE ID :%d:",i_key);
		/** Create the Message Q identifier **/
		
		if(fn_crt_msg_q(&i_qid, i_key, CLIENT, c_ServiceName, c_err_msg) == -1)
		{
			fn_errlog(c_ServiceName,"S31085", c_err_msg, c_err_msg);
		}

		ptr_c_tmp = (char*) fn_get_prcs_spc(c_ServiceName, "USER_TYPE");

		if(ptr_c_tmp == NULL)
		{
			fn_errlog(c_ServiceName,"L31005", LIBMSG, c_err_msg);
			return -1;
		}
    fn_userlog(c_ServiceName," I_QID :%d:",i_qid);	
		st_opm_mstr.si_user_typ_glb = atoi(ptr_c_tmp);
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog(c_ServiceName,"USER TYPE IS :%d:",st_opm_mstr.si_user_typ_glb);
		}
   
	 /******* Read the PACK VAL from ini file *********/
    ptr_c_tmp = (char *)fn_get_prcs_spc ( c_ServiceName, "PACK_VAL" );
    if (ptr_c_tmp == NULL )
    {
      fn_errlog(c_ServiceName,"L31010", LIBMSG, c_err_msg);
      return ( -1 );
    }
    i_pack_val = atoi(ptr_c_tmp);

    if( DEBUG_MSG_LVL_0 )
    {
      fn_userlog(c_ServiceName,"PACK VAL IS :%d:",i_pack_val);
    }
		
		/**** Set Global Variables ****/
		i_ord_mkt_stts = FALSE;
		i_exr_mkt_stts = FALSE;
		i_pmp_stts = FALSE;
		i_upd_stts = FALSE;

		i_part_stts = FALSE;
		i_brkr_stts = FALSE;
		i_exit_stts = DONT_EXIT;

		/**** Get exchange code from the database ****/

		EXEC SQL
		select	opm_xchng_cd,
						opm_max_pnd_ord
		into		:sql_c_xchng_cd,
						:sql_li_max_pnd_ord	
		From 		opm_ord_pipe_mstr
		where 	opm_pipe_id = :sql_c_pipe_id;

		if ( SQLCODE != 0 )
		{
			fn_errlog(c_ServiceName,"S31090", SQLMSG, c_err_msg);
			return ( -1 );
		}
		
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog(c_ServiceName,"Exchange code is :%s:",sql_c_xchng_cd);
		}
		

		EXEC SQL
		SELECT 	exg_settlor_stts,
						exg_brkr_stts,
						exg_crrnt_stts,				
						exg_extnd_mrkt_stts,			
						exg_exrc_mkt_stts			
		INTO		:sql_exg_settlor_stts,
						:sql_exg_brkr_stts,
						:sql_exg_crrnt_stts,
						:sql_exg_extnd_mrkt_stts,
						:sql_exg_exrc_mkt_stts
		FROM exg_xchng_mstr
		WHERE exg_xchng_cd = :sql_c_xchng_cd;

		if ( SQLCODE != 0 )
		{
			fn_errlog(c_ServiceName,"S31095", SQLMSG, c_err_msg);
			return ( -1 );
		}

		if( DEBUG_MSG_LVL_0 ) 
		{
    	fn_userlog(c_ServiceName,"Settlor status is :%c:",sql_exg_settlor_stts);
    	fn_userlog(c_ServiceName,"broker status is :%c:",sql_exg_brkr_stts);
    	fn_userlog(c_ServiceName,"exchange status is :%c:",sql_exg_crrnt_stts);
    	fn_userlog(c_ServiceName,"extended market status is :%c:",sql_exg_extnd_mrkt_stts);
    	fn_userlog(c_ServiceName,"exercise market status is :%c:",sql_exg_exrc_mkt_stts);
		}

		if ( sql_exg_settlor_stts == 'A' )
		{
			i_part_stts = TRUE;
		}

		if ( sql_exg_brkr_stts == 'A' )
		{
			i_brkr_stts = TRUE;
		}
		i_trnsctn = fn_begintran( c_ServiceName, c_err_msg );

		if ( i_trnsctn == -1)
		{
			fn_errlog(c_ServiceName,"S31100", LIBMSG, c_err_msg);
			return ( -1 );
		}

		EXEC SQL
		UPDATE opm_ord_pipe_mstr
		SET opm_pmp_stts = 0
		WHERE opm_pipe_id = :sql_c_pipe_id;

		if ( SQLCODE != 0 )
		{
			fn_errlog(c_ServiceName,"S31105", SQLMSG, c_err_msg);
			return ( -1 );
		}

		i_ch_val = fn_committran( c_ServiceName, i_trnsctn, c_err_msg );

		if ( i_ch_val == -1 )
		{
			fn_errlog(c_ServiceName,"S31110", LIBMSG, c_err_msg);
			return ( -1 );
		}

		sprintf ( c_filter1, "FFO_FILTER=='%s'", sql_c_xchng_cd );
		sprintf ( c_filter2, "FFO_FILTER=='%s'", sql_c_pipe_id );

		/**** Subscribe for triggers ****/
		
		li_ord_opn_hnd = tpsubscribe ( "TRG_ORD_OPN", (char *)c_filter1,(TPEVCTL *)NULL, 0);
	
		if ( li_ord_opn_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31115", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_ord_cls_hnd = tpsubscribe ( "TRG_ORD_CLS", (char *)c_filter1,(TPEVCTL *)NULL, 0); 
	
		if ( li_ord_cls_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31120", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_ext_opn_hnd = tpsubscribe ( "TRG_EXT_OPN", (char *)c_filter1,(TPEVCTL *)NULL, 0);
	
		if ( li_ext_opn_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31125", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_ext_cls_hnd = tpsubscribe ( "TRG_EXT_CLS", (char *)c_filter1,(TPEVCTL *)NULL, 0); 
	
		if ( li_ext_cls_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31130", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_exr_opn_hnd = tpsubscribe ( "TRG_EXR_OPN", (char *)c_filter1,(TPEVCTL *)NULL, 0);
	
		if ( li_exr_opn_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31135", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_exr_cls_hnd = tpsubscribe ( "TRG_EXR_CLS", (char *)c_filter1,(TPEVCTL *)NULL, 0); 
	
		if ( li_exr_cls_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31140", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_part_sus_hnd = tpsubscribe ( "TRG_PART_SUS", (char *)c_filter1,(TPEVCTL *)NULL, 0); 
	
		if ( li_part_sus_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31145", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_part_act_hnd = tpsubscribe ( "TRG_PART_ACT", (char *)c_filter1,(TPEVCTL *)NULL, 0); 
	
		if ( li_part_act_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31150", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_brkr_sus_hnd = tpsubscribe ( "TRG_BRKR_SUS", (char *)c_filter1,(TPEVCTL *)NULL, 0); 
	
		if ( li_brkr_sus_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31155", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_brkr_act_hnd = tpsubscribe ( "TRG_BRKR_ACT", (char *)c_filter1,(TPEVCTL *)NULL, 0); 
	
		if ( li_brkr_act_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31160", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_strt_pmp_hnd = tpsubscribe ( "TRG_STRT_PMP", (char *)c_filter2,(TPEVCTL *)NULL, 0);
	
		if ( li_strt_pmp_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31165", TPMSG, c_err_msg);
			return ( -1 );
		} 

		li_stop_pmp_hnd = tpsubscribe ( "TRG_STOP_PMP", (char *)c_filter2,(TPEVCTL *)NULL, 0);
	
		if ( li_stop_pmp_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31170", TPMSG, c_err_msg);
			return ( -1 );
		}

		li_reco_over_hnd = tpsubscribe ( "TRG_RECO_OVER", (char *)c_filter2,(TPEVCTL *)NULL, 0);
	
		if ( li_reco_over_hnd == -1 )
		{
			fn_errlog(c_ServiceName,"S31175", TPMSG, c_err_msg);
			return ( -1 );
		}
	
	  /**  Ver 1.8 Begins  **/
  	li_reco_over_hnd = tpsubscribe ( "TRG_RECO_OVER_TRD", (char *)c_filter2,(TPEVCTL *)NULL, 0);
  	if ( li_reco_over_hnd == -1 )
  	{
    	fn_errlog(c_ServiceName,"S31180", TPMSG, c_err_msg);
    	return ( -1 );
  	}
  	/**  Ver 1.8 Ends  **/

		EXEC SQL
		SELECT	OPM_TRDR_ID,OPM_BRNCH_ID,LPAD(OPM_USER_ID,3,'0')  /* USER_ID added in Ver 1.7 **/
		INTO		:st_opm_mstr.c_opm_trdr_id,
						:st_opm_mstr.li_opm_brnch_id,:st_opm_mstr.li_opm_user_id   /* st_opm_mstr.li_opm_user_id Added in Ver 1.7 */
		FROM	OPM_ORD_PIPE_MSTR
		WHERE	OPM_XCHNG_CD = :sql_c_xchng_cd
		AND	OPM_PIPE_ID	 = :sql_c_pipe_id ;

		if ( SQLCODE != 0 )
		{
			fn_errlog(c_ServiceName,"S31185", SQLMSG, c_err_msg);
			return ( -1 );
		}

		if( DEBUG_MSG_LVL_0 ) 
		{
    	fn_userlog(c_ServiceName,"Trader ID is :%s:",st_opm_mstr.c_opm_trdr_id);
    	fn_userlog(c_ServiceName,"Branch ID is :%ld:",st_opm_mstr.li_opm_brnch_id);
		}

		EXEC SQL
		Select	  exg_nxt_trd_dt,
							TO_NUMBER(TRIM(EXG_BRKR_ID)),    /* TO_NUMBER and trim added in Ver 1.7 */
							exg_ctcl_id
		into	:sql_c_nxt_trd_date,
					:st_opm_mstr.c_xchng_brkr_id,
					:sql_c_xchng_ctcl_id
		From	exg_xchng_mstr
		Where	exg_xchng_cd = :sql_c_xchng_cd;

		if ( SQLCODE != 0 )
		{
			fn_errlog(c_ServiceName,"S31190", SQLMSG, c_err_msg);
			return ( -1 );
		}

		SETNULL ( sql_c_nxt_trd_date );

		if( DEBUG_MSG_LVL_0 ) 
		{
    	fn_userlog(c_ServiceName,"Next trade date is :%s:",sql_c_nxt_trd_date.arr);
    	fn_userlog(c_ServiceName,"Broker id is :%s:",st_opm_mstr.c_xchng_brkr_id);
    	fn_userlog(c_ServiceName,"CTCL id is :%s:",sql_c_xchng_ctcl_id);
		}

		if ( c_reco_mode == 'B' )
		{
			strcpy ( c_tm_stmp, argv[6] );
		}
		else
		{
			strcpy ( c_tm_stmp, " " );
		}

	  ptr_c_tmp = ( char * ) fn_get_prcs_spc ( c_ServiceName, "MAX_Q_CHK_INTERVAL" );

    if ( ptr_c_tmp == NULL )
    {
      fn_errlog(c_ServiceName,"S31195", LIBMSG, c_err_msg);
      return ( -1 );
    }

    li_max_q= atol(ptr_c_tmp);

		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog(c_ServiceName,"Max pending orders in database is :%ld:",sql_li_max_pnd_ord);
    	fn_userlog( c_ServiceName, "Max Queue Check Interval-:%ld:",li_max_q) ;
		}


    i_ch_val = fn_dwnld_rqst ( c_reco_mode,c_tm_stmp,c_ServiceName,c_err_msg );

    if( DEBUG_MSG_LVL_0 )
    {
      fn_userlog(c_ServiceName,"DWNLD flg i_ch_val :%d:",i_ch_val);  /* Ver 1.7 */
      fn_userlog(c_ServiceName,"Download completed for ALL STREAMS");
    }

		if ( i_ch_val == -1 )
		{
			return ( -1 );
		}

		li_seq_nm = 0;

		if ( sql_exg_crrnt_stts == EXCHANGE_OPEN )
		{
			i_ord_mkt_stts = TRUE;
		}

		if ( sql_exg_exrc_mkt_stts == EXCHANGE_OPEN )
		{
			i_exr_mkt_stts = TRUE;
		}
		
		fn_var = CLN_BSE_PACK_CLNT;
 
    if( DEBUG_MSG_LVL_0 )
    {
      fn_userlog(c_ServiceName,"DWNLD flg Before init");  /* Ver 1.7 */
    }
   
		return ( 0 ); 
	}

void fn_bat_term (  int argc,char *argv[] )
	{
		return;
	}

int fn_prcs_trg ( char *c_ServiceName,int i_trg_typ )
	{
		int i_ch_val;
		char c_err_msg[256];

		char	sql_xchng_crrnt_stts;					
		char	sql_xchng_exrc_mkt_stts;		
		char  sql_xchng_extnd_mrkt_stts;	

		if ( i_trg_typ == SYSTM_TRG )
		{
			i_exit_stts = EXIT;
			return ( -1 );
		}

		if ( strcmp ( c_trg_msg, "TRG_ORD_OPN" ) == 0 )
		{
			i_ord_mkt_stts = TRUE;
		}
		else if ( strcmp ( c_trg_msg, "TRG_ORD_CLS" ) == 0 )
		{
			i_ord_mkt_stts = FALSE;
		}		
		else if  ( strcmp ( c_trg_msg, "TRG_EXR_OPN" ) == 0 )
		{
			i_exr_mkt_stts = TRUE;
		}
		else if ( strcmp ( c_trg_msg, "TRG_EXR_CLS" ) == 0 )
		{
			i_exr_mkt_stts = FALSE; 
		}
		else if ( strcmp ( c_trg_msg, "TRG_STRT_PMP" ) == 0 )
		{

			EXEC SQL
				SELECT	exg_crrnt_stts,
								exg_exrc_mkt_stts,
								exg_extnd_mrkt_stts
				INTO		:sql_xchng_crrnt_stts,
								:sql_xchng_exrc_mkt_stts,
								:sql_xchng_extnd_mrkt_stts
				FROM		exg_xchng_mstr
				WHERE		exg_xchng_cd	=	:sql_c_xchng_cd;

			if ( SQLCODE != 0 )
    	{
      	fn_errlog(c_ServiceName,"S31200", SQLMSG, c_err_msg);
      	return ( -1 );
    	}

    	if( DEBUG_MSG_LVL_0 )
    	{
      	fn_userlog(c_ServiceName,"Before Forwardnig normal market status is :%c:",sql_xchng_crrnt_stts);
      	fn_userlog(c_ServiceName,"Before Forwardnig extended market status is :%c:",sql_xchng_extnd_mrkt_stts);
      	fn_userlog(c_ServiceName,"Before Forwardnig exercise market status is :%c:",sql_xchng_exrc_mkt_stts);
				fn_userlog(c_ServiceName,"Before Forwarding Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:",i_ord_mkt_stts,i_exr_mkt_stts);

			}

			if (sql_xchng_crrnt_stts	== EXCHANGE_OPEN )
			{
				 i_ord_mkt_stts = TRUE;
			}
			if ( sql_xchng_exrc_mkt_stts	==	EXCHANGE_OPEN )
			{
				i_exr_mkt_stts = TRUE;
			}
			if( DEBUG_MSG_LVL_0 )
			{
				fn_userlog(c_ServiceName,"After Forwarding Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:",i_ord_mkt_stts,i_exr_mkt_stts);
			}

			i_pmp_stts = TRUE;
			i_upd_stts = TRUE;
		}
		else if ( strcmp ( c_trg_msg, "TRG_STOP_PMP" ) == 0 )
		{
			i_pmp_stts = FALSE; 
			i_upd_stts = TRUE;
		}
		else if ( strcmp ( c_trg_msg, "TRG_RECO_OVER" ) == 0 )
		{
			i_reco_stts = 0;	
			i_reco_stts_ord = 0;  /*** Ver 1.8 ***/
			fn_userlog(c_ServiceName,"After i_reco_stts = 0 i_reco_stts_ord |%d|",i_reco_stts_ord);
		}
		else if ( strcmp ( c_trg_msg, "TRG_PART_SUS" ) == 0 )
		{
			i_part_stts = FALSE;
		}
		else if ( strcmp ( c_trg_msg, "TRG_PART_ACT" ) == 0 )
		{
			i_part_stts = TRUE;
		}
		else if ( strcmp ( c_trg_msg, "TRG_BRKR_SUS" ) == 0 )
		{
			i_brkr_stts = FALSE;
		}
		else if ( strcmp ( c_trg_msg, "TRG_BRKR_ACT" ) == 0 )
		{
			i_brkr_stts = TRUE;
		}
		else if ( strcmp ( c_trg_msg, "TRG_RECO_OVER_TRD" ) == 0 )  /*** Ver 1.8 Added trigger trade over ***/
    {
      i_reco_stts = 0;
			i_reco_stts_trd = 0;
      fn_userlog(c_ServiceName,"After i_reco_stts = 0 i_reco_stts_trd |%d|",i_reco_stts_trd);
    }

		return ( 0 );
	}


int fn_rjct_rcrd(struct vw_xchngbook *ptr_st_rqst,
			struct vw_orderbook *ptr_st_ord, 
			char *c_ServiceName,
			char *c_err_msg )
	{
		char c_svc_nm [ 16 ];
		int i_ch_val;
		EXEC SQL BEGIN DECLARE SECTION;
		varchar c_tm_stmp [ LEN_DATE ];
		EXEC SQL END DECLARE SECTION;

		/******** set the acknowledge service name 	based on product type ****/
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog ( c_ServiceName, "Inside function reject record");
			fn_userlog ( c_ServiceName, "Product type is %c",ptr_st_ord->c_prd_typ);
		}

		if ( ptr_st_ord->c_prd_typ == FUTURES )
		{
			strcpy( c_svc_nm , "SFO_FUT_ACK" );
		}
		else
		{
			strcpy( c_svc_nm , "SFO_OPT_ACK" );
		}

		EXEC SQL
			SELECT to_char ( sysdate, 'dd-mon-yyyy hh24:mi:ss' )
				INTO :c_tm_stmp
				FROM dual;
		if ( SQLCODE != 0 )
		{
			fn_errlog(c_ServiceName,"S31205", SQLMSG, c_err_msg);
			return -1;
		}
		SETNULL ( c_tm_stmp );

		/* Populate the structure to update the database as thr record is rejected */
		ptr_st_rqst->c_plcd_stts = REJECT;
		ptr_st_rqst->c_rms_prcsd_flg = NOT_PROCESSED;
		ptr_st_rqst->l_ors_msg_typ = ORS_NEW_ORD_RJCT;
		strcpy ( ptr_st_rqst->c_ack_tm , ( char *) c_tm_stmp.arr );
		strcpy ( ptr_st_rqst->c_xchng_rmrks , "Token id not available" );
		ptr_st_rqst->d_jiffy =0;
		ptr_st_rqst->c_oprn_typ = UPDATION_ON_EXCHANGE_RESPONSE;
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog ( c_ServiceName, "Before calling SFO_UPD_XCHNGBK for rejecting record ");
		}

		i_ch_val = fn_upd_bse_xchngbk(ptr_st_rqst,c_ServiceName,c_err_msg);

		if ( i_ch_val != 0 )
		{
			fn_errlog(c_ServiceName,"S31210", LIBMSG, c_err_msg);
			return -1;
		}
		if( DEBUG_MSG_LVL_3 ) 
		{
			fn_gettime();
			fn_userlog(c_ServiceName,"TIME_STATS Before calling UPD_XCHNGBK order reject case :%s:",c_time);
		} 
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog ( c_ServiceName, "After calling SFO_UPD_XCHNGBK for rejecting record ");
		}

		fn_cpy_ddr(ptr_st_rqst->c_rout_crt);
		
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog ( c_ServiceName, "Before calling %s for rejecting record ",c_svc_nm);
		}

		i_ch_val = fn_call_svc (c_ServiceName,
														c_err_msg,
														ptr_st_rqst,
														ptr_st_rqst,
														"vw_xchngbook",
														"vw_xchngbook",
														sizeof ( struct vw_xchngbook ),
														sizeof ( struct vw_xchngbook ),
														0,
														c_svc_nm );

		if ( i_ch_val != SUCC_BFR )
		{
		fn_errlog(c_ServiceName,"S31215", LIBMSG, c_err_msg);
		return -1;
		}
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog ( c_ServiceName, "After calling %s for rejecting record ",c_svc_nm);
			fn_userlog ( c_ServiceName, "Order rejection succesfull");
		}
		return 0;
	}

/*********************Download Request ****************/
int fn_dwnld_rqst( char c_reco_mode,
                  char *c_tm_stmp,
                  char *c_ServiceName,
                  char *c_err_msg
                )
{

  EXEC SQL BEGIN DECLARE SECTION;
    int i_ord_bod_stts;
    int i_trd_bod_stts;
    double d_strt_tm_i;
    double d_strt_tm_o;
    double d_tm_diff;
    double d_tm;
    char sql_c_tm_stmp [ LEN_DT ];
  EXEC SQL END DECLARE SECTION;

  int i_rec_stts;
  int i_ch_val;
  short int si_rqst_type;
  int i;
  unsigned char digest[16];
  long long ll_strt_tm;
	short s_max_partition_id = 0; /*** Ver 1.8 ***/
	short int s_partition_id = 0; /*** Ver 1.8 ***/

  char  c_pipe[3];
  char  c_trd_dt[23];
  int i_snd_seq;

  /***************Memset the structures ******************************/


  i_rec_stts = 0;

  strcpy (c_pipe, sql_c_pipe_id);
  if(DEBUG_MSG_LVL_0)
  {
   fn_userlog(c_ServiceName,"Inside Function fn_dwnld_rqst");
   fn_userlog(c_ServiceName,"Pipe Id Is :%s:",c_pipe);
   fn_userlog(c_ServiceName,"Time Stamp Is :%s:",c_tm_stmp);
  }

  if ( c_reco_mode == 'N' )
  {
    fn_userlog(c_ServiceName,"Inside No DownLoad Condition");
    i_rec_stts = 0;
    fn_userlog(c_ServiceName,"i_rec_stts Is :%d:",i_rec_stts);
  }
  else
  {
		/*** Added in Ver 1.8 ***/
		EXEC SQL
    SELECT OPM_STREAM_NO                                                                  /* Stream number */
    INTO  :s_max_partition_id
    FROM   OPM_ORD_PIPE_MSTR,
           EXG_XCHNG_MSTR
    WHERE  OPM_XCHNG_CD = :sql_c_xchng_cd
    AND    OPM_XCHNG_CD = EXG_XCHNG_CD
    AND    OPM_PIPE_ID  = :sql_c_pipe_id;

   /*for(i = 1; i < 3 ;i++) * changed to 3 from 4 for dwnld in Ver 1.7 */
   for(s_partition_id = 1; s_partition_id <= s_max_partition_id ;s_partition_id++) /* Ver 1.8 */
   {
    /*** Commented in Ver 1.8 ******* 
     switch(i)
     {
      case  1:
              si_rqst_type  = PRSNL_ORDR_DWNLD;
              break;
      case  2:
              si_rqst_type  = PRSNL_TRD_DWNLD;
              break;
      * commented in Ver 1.7 case  3:
              si_rqst_type  = PRSN_SLTP_ORDR_DWNLD; *
              break; 
     }
			***************************/

     if( DEBUG_MSG_LVL_0 )
     {
      fn_userlog(c_ServiceName,"In Ver 1.7 s_partition_id is :%d:",s_partition_id); /* Ver 1.7 */
      fn_userlog(c_ServiceName,"Inside FOr Loop");
     }

    if(fn_crt_dwnld_rqst(s_partition_id,c_tm_stmp,c_ServiceName,c_err_msg) == -1)
    {
      errlog(c_ServiceName, "S31220", LIBMSG, (char *)DEF_USR, DEF_SSSN, c_err_msg);
      return -1;
    }

   }

  }

  return(0);

}


int fn_crt_dwnld_rqst(short int s_partition_id,
                        char *c_tm_stmp,
                        char *c_ServiceName,
                  char *c_err_msg)
{
 int i_hr=0;
 int i_min=0;
 int i_sec=0;
 int i_ret_val=0; /*** Ver 1.8 ***/

 /*** uint16_t	s_partition_id=0;															* Ver 1.7 */
 unsigned char c_ord_lst_seq[LEN_APPL_BEG_MSGID + 1];   /* Ver 1.7 */
 uint64_t ll_trd_lst_seq;                               /* Ver 1.7 */

 /* commented in ver 1.7 struct st_prsnl_dwnld st_prsnl_dwnld_rqst; 
  MEMSET(st_prsnl_dwnld_rqst); */

 struct st_send_rqst_data st_q_con_clnt;
 MEMSET(st_q_con_clnt); 

 RetransmitMEMessageRequestT st_dwnld;  /* Ver 1.7 */
 RetransmitRequestT st_trddwnld;        /* Ver 1.7 */

 MEMSET(st_dwnld);                      /* Ver 1.7 */
 MEMSET(st_trddwnld);                   /* Ver 1.7 */

 if( DEBUG_MSG_LVL_3 )
 {
  fn_userlog(c_ServiceName," Inside fn_crt_dwnld_rqst");
  fn_userlog(c_ServiceName," c_tm_stmp :%s:",c_tm_stmp);
 }

 /* commented in ver 1.7 
 if(strcmp(c_tm_stmp," ")== 0)
 {
 st_prsnl_dwnld_rqst.st_dwnld_time.c_hour= 0;
 st_prsnl_dwnld_rqst.st_dwnld_time.c_min = 0;
 st_prsnl_dwnld_rqst.st_dwnld_time.c_sec = 0;
 }
 else
 {
  EXEC SQL
      SELECT TO_NUMBER(SUBSTR(:c_tm_stmp,1,2)),
            TO_NUMBER(SUBSTR(:c_tm_stmp,4,2)),
             TO_NUMBER(SUBSTR(:c_tm_stmp,7,2))
      INTO   :i_hr,
             :i_min,
             :i_sec
      From dual;

  st_prsnl_dwnld_rqst.st_dwnld_time.c_hour =(char )i_hr;
  st_prsnl_dwnld_rqst.st_dwnld_time.c_min  =(char )i_min;
  st_prsnl_dwnld_rqst.st_dwnld_time.c_sec  =(char )i_sec;
 }
  if( DEBUG_MSG_LVL_3 )
  {
   fn_userlog(c_ServiceName," i_hr :%d:",i_hr);
   fn_userlog(c_ServiceName," i_min:%d:",i_min);
   fn_userlog(c_ServiceName," i_sec:%d:",i_sec);
  }
  st_prsnl_dwnld_rqst.st_hdr.l_slot_no = htonl(REQ_SLOT_NO);
  st_prsnl_dwnld_rqst.l_msg_typ = htonl(si_rqst_type);
  st_prsnl_dwnld_rqst.c_filler = ' ';
	Ver 1.7 ****/
	/*** Commented in 1.8 *******
  st_q_con_clnt.li_msg_type = si_rqst_type; * Ver 1.7 *

  if(si_rqst_type == PRSNL_ORDR_DWNLD)
  {
   * commented in ver 1.7  st_prsnl_dwnld_rqst.l_msg_tag = htonl(PRSNL_ORDR_DWNLD_TAG); *
			st_dwnld.MessageHeaderIn.TemplateID= htons(TID_RETRANSMIT_ME_MESSAGE_REQUEST);  * Ver 1.7 *
  }
  else if(si_rqst_type == PRSNL_TRD_DWNLD)
  {
   * commented in Ver 1.7 st_prsnl_dwnld_rqst.l_msg_tag = htonl(PRSNL_TRD_DWNLD_TAG); *
			st_trddwnld.MessageHeaderIn.TemplateID= htons(TID_RETRANSMIT_REQUEST);  * Ver 1.7 *
  }
  * commented in Ver 1.7 else if(si_rqst_type == PRSN_SLTP_ORDR_DWNLD )
  {
   st_prsnl_dwnld_rqst.l_msg_tag = htonl(PRSN_SLTP_ORDR_DWNLD_TAG); 
  } *
  else
  {
  return -1;
  }
		**********************************/
 /** Ver 1.7 starts **/

	EXEC SQL
    SELECT (SUBSTR(OPM_PIPE_ID,2,1) || '0'),                                              /* <pipe id[1] || '0' <Sequence> */
           TO_NUMBER(TRIM(EXG_BRKR_ID)||'00'|| LPAD(OPM_USER_ID,3,'0')),                  /* <Brokerd id>00<userid> */
     			 (SUBSTR(OPM_PIPE_ID,2,1) || '0'),                                              /* <pipe id[1] || '0' <Sequence> */
           TO_NUMBER(TRIM(EXG_BRKR_ID)||'00'|| LPAD(OPM_USER_ID,3,'0'))                   /* <Brokerd id>00<userid> */
    INTO   :st_dwnld.RequestHeader.MsgSeqNum,
           :st_dwnld.SubscriptionScope,                                                    /* Not used */
       		 :st_trddwnld.RequestHeader.MsgSeqNum,
           :st_trddwnld.SubscriptionScope                                                    /* Not used */
    FROM   OPM_ORD_PIPE_MSTR,
           EXG_XCHNG_MSTR
    WHERE  OPM_XCHNG_CD = :sql_c_xchng_cd
    AND    OPM_XCHNG_CD = EXG_XCHNG_CD
    AND    OPM_PIPE_ID  = :sql_c_pipe_id;

  if ( SQLCODE != 0)
  {
    fn_userlog(c_ServiceName,"Error while selecting Pipe specific and Exchange specific details.");
		fn_errlog(c_ServiceName, "L31065", LIBMSG, c_err_msg);
    return ( -1 );
  }

	/**** s_partition_id=1;    * changed to 1 in Ver 1.8 */
	ll_trd_lst_seq = 0;
	MEMSET(c_ord_lst_seq);

	/*if(si_rqst_type == PRSNL_ORDR_DWNLD)
	{
	******** Ver 1.8 ****/
	st_dwnld.MessageHeaderIn.BodyLen = htonl(sizeof(RetransmitMEMessageRequestT));
  st_dwnld.MessageHeaderIn.TemplateID = htons(TID_RETRANSMIT_ME_MESSAGE_REQUEST);
  strcpy(st_dwnld.MessageHeaderIn.NetworkMsgID,"");
  strcpy(st_dwnld.MessageHeaderIn.Pad2,"");
  st_dwnld.RequestHeader.SenderSubID = NO_VALUE_UINT;
  st_dwnld.SubscriptionScope = NO_VALUE_UINT;
  st_dwnld.PartitionID = s_partition_id;
  st_dwnld.RefApplID = 4;               /* 4 : Session data */

  memset(st_dwnld.ApplBegMsgID,NO_VALUE_STR,16);   /** Ver 1.8 **/
  memset(st_dwnld.ApplEndMsgID,NO_VALUE_STR,16);   /** Ver 1.8 **/
 
  /** commented in Ver 1.8 memcpy(st_dwnld.ApplBegMsgID, c_ord_lst_seq,16);
  memset(st_dwnld.ApplEndMsgID,0,16); **/
  st_dwnld.Pad1[0] = '\0';

	fn_userlog(c_ServiceName,"Pipe id :%s: retransmit_order Request TemplateId:%d: BodyLen:%ld: ",sql_c_pipe_id,st_dwnld.MessageHeaderIn.TemplateID,st_dwnld.MessageHeaderIn.BodyLen);

  st_dwnld.MessageHeaderIn.BodyLen = fn_swap_ulong(st_dwnld.MessageHeaderIn.BodyLen);
  st_dwnld.MessageHeaderIn.TemplateID = fn_swap_ushort(st_dwnld.MessageHeaderIn.TemplateID);
  st_dwnld.RequestHeader.MsgSeqNum = fn_swap_ulong(st_dwnld.RequestHeader.MsgSeqNum);
  st_dwnld.RequestHeader.SenderSubID = fn_swap_ulong(st_dwnld.RequestHeader.SenderSubID);
  st_dwnld.SubscriptionScope = fn_swap_ulong(st_dwnld.SubscriptionScope);
  st_dwnld.PartitionID = fn_swap_ushort(st_dwnld.PartitionID);

  st_q_con_clnt.li_msg_type = PRSNL_ORDR_DWNLD;
	memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_dwnld,sizeof(st_dwnld));

	if(fn_write_msg_q(i_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)
  {
    fn_errlog(c_ServiceName, "L31065", LIBMSG, c_err_msg);
    return -1;
  }

  fn_userlog(c_ServiceName," After fn_write_msg_q");
	 i_reco_stts_ord = 1;  /**  Ver 1.4  **/

    if( DEBUG_MSG_LVL_0 )
    {
      fn_userlog(c_ServiceName," Waiting for download over trigger");
    }

    while ( i_reco_stts_ord == 1 )   /** ( i_reco_stts == 1 )  Ver 1.4  **/
    {
      if( DEBUG_MSG_LVL_0)
      {
        fn_userlog(c_ServiceName,"Before Going To Sleep For 5 Seconds ");
      }

      sleep(5);

      /**** Check for unsolicited messages ****/

      i_ret_val = fn_chk_stts ( c_ServiceName ) ;

      if ( i_ret_val == -1 )
      {
        return -1;
      }
    }

		if( DEBUG_MSG_LVL_0 )
    {
      fn_userlog(c_ServiceName,"Order download processing done for stream |%d|",s_partition_id);  /* ver 1.4 */
    }
/*	}
	else if(si_rqst_type == PRSNL_TRD_DWNLD)
	{*/
	st_trddwnld.MessageHeaderIn.BodyLen = htonl(sizeof(RetransmitRequestT));
  st_trddwnld.MessageHeaderIn.TemplateID = htons(TID_RETRANSMIT_REQUEST);
  strcpy(st_trddwnld.MessageHeaderIn.NetworkMsgID,"");
  strcpy(st_trddwnld.MessageHeaderIn.Pad2,"");
  st_trddwnld.RequestHeader.SenderSubID = NO_VALUE_UINT;
  st_trddwnld.ApplBegSeqNum = htonll((ll_trd_lst_seq == 0)? NO_VALUE_ULONG : ll_trd_lst_seq);
  st_trddwnld.ApplEndSeqNum = NO_VALUE_ULONG;
  st_trddwnld.PartitionID = s_partition_id;
  st_trddwnld.RefApplID = 1;
  st_trddwnld.Pad1[0] = '\0';

	fn_userlog(c_ServiceName,"Pipe ID :%s: retransmit_trade Request TemplateId:%d: BodyLen:%ld: ",sql_c_pipe_id,st_trddwnld.MessageHeaderIn.TemplateID,st_trddwnld.MessageHeaderIn.BodyLen);

  st_trddwnld.MessageHeaderIn.BodyLen = fn_swap_ulong(st_trddwnld.MessageHeaderIn.BodyLen);
  st_trddwnld.MessageHeaderIn.TemplateID = fn_swap_ushort(st_trddwnld.MessageHeaderIn.TemplateID);
  st_trddwnld.RequestHeader.MsgSeqNum = fn_swap_ulong(st_trddwnld.RequestHeader.MsgSeqNum);
  st_trddwnld.RequestHeader.SenderSubID = fn_swap_ulong(st_trddwnld.RequestHeader.SenderSubID);
  st_trddwnld.ApplBegSeqNum = fn_swap_ulnglng(st_trddwnld.ApplBegSeqNum);
  st_trddwnld.ApplEndSeqNum = fn_swap_ulnglng(st_trddwnld.ApplEndSeqNum);
  st_trddwnld.SubscriptionScope = fn_swap_ulong(st_trddwnld.SubscriptionScope);
  st_trddwnld.PartitionID = fn_swap_ushort(st_trddwnld.PartitionID);
	MEMSET(st_q_con_clnt);
  st_q_con_clnt.li_msg_type = PRSNL_TRD_DWNLD;
	memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_trddwnld,sizeof(st_trddwnld));
	
	if(fn_write_msg_q(i_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)
  {
    fn_errlog(c_ServiceName, "L31065", LIBMSG, c_err_msg);
    return -1;
  }

  fn_userlog(c_ServiceName," After fn_write_msg_q");

	 i_reco_stts_trd = 1;

    if( DEBUG_MSG_LVL_0 )
    {
      fn_userlog(c_ServiceName," Waiting for trade download over trigger");
    }

    while ( i_reco_stts_trd == 1 )
    {
      if( DEBUG_MSG_LVL_0)
      {
        fn_userlog(c_ServiceName,"Before Going To Sleep For 5 Seconds ");
      }

      sleep(5);

      /**** Check for unsolicited messages ****/

      i_ret_val = fn_chk_stts ( c_ServiceName ) ;

      if ( i_ret_val == -1 )
      {
        return -1;
      }
    }

		if( DEBUG_MSG_LVL_0 )
    {
      fn_userlog(c_ServiceName,"Trade download processing done for stream |%d|",s_partition_id);
    }
/*	}*/
  /** Ver 1.7 ends **/
  
	/*** commented in Ver 1.7
	else
	{
  st_prsnl_dwnld_rqst.st_hdr.l_message_length = htonl(sizeof(st_prsnl_dwnld_rqst) - sizeof(st_prsnl_dwnld_rqst.st_hdr));

  st_prsnl_dwnld_rqst.st_hdr.l_slot_no = fn_swap_long(st_prsnl_dwnld_rqst.st_hdr.l_slot_no);
  st_prsnl_dwnld_rqst.l_msg_typ = fn_swap_long(st_prsnl_dwnld_rqst.l_msg_typ);
  st_prsnl_dwnld_rqst.l_msg_tag = fn_swap_long(st_prsnl_dwnld_rqst.l_msg_tag);
  st_prsnl_dwnld_rqst.st_hdr.l_message_length = fn_swap_long(st_prsnl_dwnld_rqst.st_hdr.l_message_length);

  memcpy(&(st_q_con_clnt.st_bsnd_rqst_data),&st_prsnl_dwnld_rqst,sizeof(st_prsnl_dwnld_rqst));
  if( DEBUG_MSG_LVL_4 )
  {
   fn_userlog(c_ServiceName,"Inside FN_CRT_DWNLD_RQST");
   fn_userlog(c_ServiceName,"l_slot_no  :%ld:",st_prsnl_dwnld_rqst.st_hdr.l_slot_no);
   fn_userlog(c_ServiceName,"l_msg_typ  :%ld:",st_prsnl_dwnld_rqst.l_msg_typ);
   fn_userlog(c_ServiceName,"l_msg_tag :%ld:",st_prsnl_dwnld_rqst.l_msg_tag);
   fn_userlog(c_ServiceName,"l_message_length :%ld",st_prsnl_dwnld_rqst.st_hdr.l_message_length);
   fn_userlog(c_ServiceName,"DWNLD TIME :%d:%d:%d:",st_prsnl_dwnld_rqst.st_dwnld_time.c_hour,st_prsnl_dwnld_rqst.st_dwnld_time.c_min,st_prsnl_dwnld_rqst.st_dwnld_time.c_sec);
  }
	}***

  if(fn_write_msg_q(i_qid,
                    (void *)&st_q_con_clnt,
                    sizeof(st_q_con_clnt),
                    c_ServiceName,
                    c_err_msg) == -1)
  {
    fn_errlog(c_ServiceName, "L31065", LIBMSG, c_err_msg);
    return -1;
  }
	********************** Ver 1.8 **************/
  fn_userlog(c_ServiceName," End of  fn_crt_dwnld_rqst");
return 0;
}

 
/************************** Download request Ends *******************/


/********************************* Function get next record**********/


int fn_get_nxt_bse_rec ( char *c_ServiceName,
                     char *c_err_msg )
{
	int i_ch_val;
	int i_sprd_flg;
	int i_tmp;
	int i_counter;

	EXEC SQL BEGIN DECLARE SECTION;
		struct vw_spdordbk st_spd_ord_bk;
		struct vw_sequence   st_seq;
		struct vw_xchngbook  st_rqst[3], st_rqst1;
		struct vw_orderbook  st_ord[3], st_ord1;
		struct vw_exrcbook   st_exr;
		struct vw_nse_cntrct st_nse_cnt[3], st_nse_cnt1;
		struct vw_contract   st_cnt;

		long 	li_count;
		int 	i_tot_cnt=0;          
	EXEC SQL END DECLARE SECTION; 

	if ( (li_cnt_q >= li_max_q) || (i_queue_chk_flg == 1) )
	{
		EXEC SQL
			 Select   count(*)
			 into     :li_count
			 From     fxb_fo_xchng_book
			 Where    fxb_mod_trd_dt = :sql_c_nxt_trd_date 
			 and 			fxb_xchng_cd = :sql_c_xchng_cd
			 and      fxb_pipe_id = :sql_c_pipe_id
			 and      fxb_plcd_stts = 'Q';
		if ( SQLCODE != 0 )
		{
			fn_errlog(c_ServiceName,"S31225", SQLMSG, c_err_msg);
			return ERROR;
		}

		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog(c_ServiceName,"Pipe id :%s: q chk cnt :%ld: prev_q_flg = :%d: ord queue count :%ld:",sql_c_pipe_id,li_cnt_q,i_queue_chk_flg,li_count);
		}

		if(li_count >= sql_li_max_pnd_ord)
		{
			i_queue_chk_flg = 1;
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog(c_ServiceName,"Num of Ords in queued stts[%ld] exceeds max allwd[%ld] for pipe :%s:",li_count,sql_li_max_pnd_ord,sql_c_pipe_id);
			}
			return OVER_LOAD;
		}

		li_cnt_q = 0;	
		i_queue_chk_flg = 0;
	}


	/**** Get the exchange book record ****/
	strcpy (st_rqst1.c_xchng_cd, sql_c_xchng_cd);

	strcpy (st_rqst1.c_pipe_id, sql_c_pipe_id);

	st_rqst1.l_ord_seq = li_seq_nm;

	strcpy (st_rqst1.c_mod_trd_dt, (char*) sql_c_nxt_trd_date.arr);

	if((i_ord_mkt_stts == FALSE) && (i_exr_mkt_stts == TRUE))
	{
		
		st_rqst1.c_oprn_typ = FOR_EXER;
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog(c_ServiceName,"In Normal Market selection");
			fn_userlog(c_ServiceName,"Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:",
							i_ord_mkt_stts,i_exr_mkt_stts);
		}
	}
	else 
	{
		st_rqst1.c_oprn_typ = FOR_NORM;           
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog(c_ServiceName,"In Normal Market selection");
			fn_userlog(c_ServiceName,"Market Status:i_ord_mkt_stts:%d:i_exr_mkt_stts:%d:",
							i_ord_mkt_stts,i_exr_mkt_stts);
		}
	}

	if( DEBUG_MSG_LVL_3 )  
	{
		fn_gettime();
		fn_userlog(c_ServiceName,"TIME_STATS Before calling SEQ_TO_OMD :%s:",c_time);
	}

	i_ch_val = fn_seq_to_bseomd(&st_rqst1,c_ServiceName,c_err_msg);


	if ( (i_ch_val != 0) &&( i_ch_val != NO_DATA_FOUND ) )
	{
	  fn_errlog(c_ServiceName,"S31230", LIBMSG, c_err_msg);
	  return ERROR;
	}

	/**** No record found ****/
	if ( i_ch_val == NO_DATA_FOUND )
	{
	  return NDF;
	}
	if( DEBUG_MSG_LVL_3 )  
	{
		fn_gettime();
		fn_userlog(c_ServiceName,"TIME_STATS After calling SEQ_TO_OMD :%s:",c_time);
	} 

	if( DEBUG_MSG_LVL_0 ) 
	{
		fn_userlog ( c_ServiceName, "Order ref / Mod number = |%s| / |%ld|",st_rqst1.c_ordr_rfrnc ,
							 st_rqst1.l_mdfctn_cntr );
	}
	
	if ( st_rqst1.c_plcd_stts != REQUESTED )
	{
	  return DATA_CNCLD;
	}

	switch ( st_rqst1.c_ex_ordr_typ )
	{
		case ORDINARY_ORDER :

			if ( i_ord_mkt_stts == FALSE )

			{
				return DATA_SKPD;
			}

			/**** Get order book record ****/
			strcpy (st_ord1.c_ordr_rfrnc, st_rqst1.c_ordr_rfrnc);
			st_ord1.c_oprn_typ = FOR_SNDCLNT;
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "Calling SFO_REF_TO_ORD for normal order" );
			}
			
      i_ch_val = fn_bseref_to_ord(&st_ord1,c_ServiceName,c_err_msg);

			if (i_ch_val != 0)
			{
				fn_errlog(c_ServiceName,"S31235", LIBMSG, c_err_msg);
				if( i_ch_val == LOCK )
				{
					if( DEBUG_MSG_LVL_0 ) 
					{
						fn_userlog ( c_ServiceName, "DISTRIBUTION lOCK ERROR" );
					}
					return LOCK_ERR;
				}
				else
				{
					return ERROR;
				}
			}
			if( DEBUG_MSG_LVL_3 )  
			{
				fn_gettime();
				fn_userlog(c_ServiceName,"TIME_STATS After calling REF_TO_ORD :%s:",c_time);
			} 
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "After Calling SFO_REF_TO_ORD for normal order" );

				fn_userlog(c_ServiceName," ORDINARY_ORDER CTCL ID |%s|",st_ord1.c_ctcl_id );
			}

			if ( st_ord1.l_mdfctn_cntr != st_rqst1.l_mdfctn_cntr )
			{
				return DATA_CNCLD;
			}

			st_rqst1.c_plcd_stts = QUEUED;
			st_rqst1.c_oprn_typ = UPDATION_ON_ORDER_FORWARDING;
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "Calling SFO_UPD_XCHNGBK for normal order" );
			}

			i_ch_val = fn_upd_bse_xchngbk(&st_rqst1,c_ServiceName,c_err_msg);

			if (i_ch_val != 0)
			{
				fn_errlog(c_ServiceName,"S31240", LIBMSG, c_err_msg);
				if( i_ch_val == LOCK )
				{
				  return LOCK_ERR;
				}
				else
				{
				  return ERROR;
				}
			}
			if( DEBUG_MSG_LVL_3 ) 
			{
				fn_gettime();
				fn_userlog(c_ServiceName,"TIME_STATS After calling UPD_XCHNGBK :%s:",c_time);
			} 
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "After Calling SFO_UPD_XCHNGBK for normal order" );
			}
			st_ord1.c_ordr_stts = QUEUED;
			st_ord1.c_oprn_typ  = UPDATE_ORDER_STATUS;
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "Calling SFO_UPD_ORDRBK for normal order" );
			}

			i_ch_val = fn_upd_bse_ordrbk(&st_ord1,c_ServiceName,c_err_msg);
			if (i_ch_val != 0)
			{
			  fn_errlog(c_ServiceName,"S31245", LIBMSG, c_err_msg);
			  return ERROR;
			}
			if( DEBUG_MSG_LVL_3 )  
			{
				fn_gettime();
				fn_userlog(c_ServiceName,"TIME_STATS After calling UPD_ORDRBK :%s:",c_time);
			} 
		
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "After Calling SFO_UPD_ORDRBK for normal order" );
			}

			/**** Convert the contract information to exchange format ****/
			strcpy (st_cnt.c_xchng_cd, st_ord1.c_xchng_cd);
			st_cnt.c_prd_typ = st_ord1.c_prd_typ;
			strcpy (st_cnt.c_undrlyng, st_ord1.c_undrlyng);
			strcpy (st_cnt.c_expry_dt, st_ord1.c_expry_dt);
			st_cnt.c_exrc_typ = st_ord1.c_exrc_typ;
			st_cnt.c_opt_typ = st_ord1.c_opt_typ;
			st_cnt.l_strike_prc   = st_ord1.l_strike_prc;
			st_cnt.c_ctgry_indstk = st_ord1.c_ctgry_indstk;
			st_cnt.l_ca_lvl       = st_ord1.l_ca_lvl;
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "Converting conttract information to exchange format" );
			}

		  /** st_cnt.c_rqst_typ = CONTRACT_TO_NSE_ID; **/

     	i_ch_val = fn_get_bse_scrip_cd(&st_cnt,
                                 &st_nse_cnt1,
                                 c_ServiceName,
                                 c_err_msg);
		  if (i_ch_val != 0)
		  {
		    fn_errlog(c_ServiceName,"S31250", LIBMSG, c_err_msg);
		    return ERROR;
		  }
			if( DEBUG_MSG_LVL_3 ) 
			{
				fn_gettime();
				fn_userlog(c_ServiceName,"TIME_STATS After calling GT_EXT_CNT :%s:",c_time);
			}

 
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "After Converting contract information to exchange format" );
			}

		 break;
      
		case EXERCISE :
		case DONT_EXERCISE :
     
			if ( i_exr_mkt_stts == FALSE )
			{
				return DATA_SKPD;
			}
			strcpy (st_exr.c_exrc_ordr_rfrnc, st_rqst1.c_ordr_rfrnc);
			st_exr.c_oprn_typ = FOR_SNDCLNT;
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "Calling SFO_EXRQ_DTLS for EXERCISE order " );
			}
      /******* COmmented, Bcoz Currently not allowed in BSE ********
			i_ch_val = fn_exrq_dtls(&st_exr,c_ServiceName,c_err_msg);

			if (i_ch_val != 0)
			{
				fn_errlog(c_ServiceName,"S31255", LIBMSG, c_err_msg);
				if( i_ch_val == LOCK ) 
				{
					return LOCK_ERR;
				}
				else
				{
					return ERROR;
				}
			}
			if( DEBUG_MSG_LVL_3 )  
			{
				fn_gettime();
				fn_userlog(c_ServiceName,"TIME_STATS After calling EXRQ_DTLS for Exercise order :%s:",c_time);
			} 
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "After Calling SFO_EXRQ_DTLS for EXERCISE order " );
			}

			if ( st_exr.l_mdfctn_cntr != st_rqst1.l_mdfctn_cntr )
			{
				return DATA_CNCLD;
			}

			st_rqst1.c_plcd_stts = QUEUED;
			st_rqst1.c_oprn_typ = UPDATION_ON_ORDER_FORWARDING;
			
      i_ch_val = fn_upd_bse_xchngbk(&st_rqst1,c_ServiceName,c_err_msg);

			if (i_ch_val != 0)
			{
			  fn_errlog(c_ServiceName,"S31260", LIBMSG, c_err_msg);
			  if( i_ch_val == LOCK ) 
			  {
			    return LOCK_ERR;
			  }
			  else
			  {
			    return ERROR;
			  }
			}
			if( DEBUG_MSG_LVL_3 ) 
			{
				fn_gettime();
				fn_userlog(c_ServiceName,"TIME_STATS After calling UPD_XCHNGBK for Exercise order :%s:",c_time);
			} 
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "After Calling SFO_UPD_XCHNGBK for EXERCISE order " );
			}

			st_exr.c_exrc_stts = QUEUED;

			st_exr.c_oprn_typ  = UPDATE_EXERCISE_REQUEST_STATUS;
			
      i_ch_val = fn_upd_exbk(&st_exr,c_ServiceName,c_err_msg);

			if (i_ch_val != 0)
			{
				fn_errlog(c_ServiceName,"S31265", LIBMSG, c_err_msg);
				return ERROR;
			}
			if( DEBUG_MSG_LVL_3 ) 
			{
				fn_gettime();
				fn_userlog(c_ServiceName,"TIME_STATS After calling UPD_EXBK for Exercise order :%s:",c_time);
			} 
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "After Calling SFO_UPD_EXBK for EXERCISE order " );
			}

			**** Convert the contract information to exchange format ****
			strcpy (st_cnt.c_xchng_cd, st_exr.c_xchng_cd);
			st_cnt.c_prd_typ = st_exr.c_prd_typ;
			strcpy (st_cnt.c_undrlyng, st_exr.c_undrlyng);
			strcpy (st_cnt.c_expry_dt, st_exr.c_expry_dt);
			st_cnt.c_exrc_typ = st_exr.c_exrc_typ;
			st_cnt.c_opt_typ = st_exr.c_opt_typ;
			st_cnt.l_strike_prc   = st_exr.l_strike_prc;
			st_cnt.c_ctgry_indstk = st_exr.c_ctgry_indstk;
			st_cnt.l_ca_lvl = st_exr.l_ca_lvl;

			if ( strcmp(sql_c_xchng_cd , "BFO") == 0)
			{
				 ** st_cnt.c_rqst_typ = CONTRACT_TO_NSE_ID; **
  			
         i_ch_val = fn_get_bse_scrip_cd(&st_cnt,
                                  &st_nse_cnt1,
                                  c_ServiceName,
                                  c_err_msg);

				if (i_ch_val != 0)
				{
					fn_errlog(c_ServiceName,"S31270", LIBMSG, c_err_msg);
					return ERROR;
				}
				if( DEBUG_MSG_LVL_3 )  
				{
					fn_gettime();
					fn_userlog(c_ServiceName,"TIME_STATS After calling GT_EXT_CNT for Exercise order :%s:",c_time);
				} 
			}
			else
			{
				return ERROR;
			}
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "After Calling SFO_GT_EXT_CNT for EXERCISE order " );
			}
      ************/
			break;
      
		default:
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "Unknown E and O type");
			}
			return ERROR;
			break;
		}
   
	if (st_rqst1.c_ex_ordr_typ == ORDINARY_ORDER )
	{
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog ( c_ServiceName, "Packing Ordinary order STARTS" );
		}
  
		if (st_nse_cnt1.l_token_id == 0)
		{
			if( DEBUG_MSG_LVL_0 ) 
			{
				fn_userlog ( c_ServiceName, "The token id for Ordinarty order is : %ld",st_nse_cnt1.l_token_id );
			}
			fn_errlog ( c_ServiceName, "S31275", "Error - Token id not available",c_err_msg);


			i_ch_val = fn_rjct_rcrd ( &st_rqst1,
																&st_ord1,
																c_ServiceName,
																c_err_msg );

			if ( i_ch_val == -1 )
			{
				return ERROR;
			}

			return TKN_NA;
		}
 
		memset(&st_q_packet, '\0', sizeof(struct st_send_rqst_data));
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog ( c_ServiceName, "Packing ordinary order STARTS" );
		}

		i_ch_val =  fn_pack_ordnry_ord_to_bse(&st_rqst1,
																					&st_ord1,
																					&st_nse_cnt1,
																					&st_opm_mstr,
																					&st_q_packet,	
																					c_ServiceName
																					);
  	if ( i_ch_val == -1 )
   	{
     	return ERROR;
   	}
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog ( c_ServiceName, "Packing ordinary order ENDS" );
		}

	}
  /********* COmmented , bcoz currently not allowed BSE ******
	else if( 	(st_rqst1.c_ex_ordr_typ == EXERCISE ) ||
						(st_rqst1.c_ex_ordr_typ == DONT_EXERCISE ))
	{
		memset(&st_q_packet, '\0', sizeof(struct st_send_rqst_data));
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog ( c_ServiceName, "Packing Exercise order STARTS" );
		}
  ****
		i_ch_val= fn_pack_exer_ord_to_nse(&st_rqst1,
																			&st_exr,
																			&st_nse_cnt1,
																		  &st_opm_mstr,
																			&st_q_packet,	
																			c_ServiceName
																			);
  	if ( i_ch_val == -1 )
   	{
     	return ERROR;
   	}  ***
		if( DEBUG_MSG_LVL_0 ) 
		{
			fn_userlog ( c_ServiceName, "Packing Exercise order ENDS" );
		}

	} ****/

	li_cnt_q++;		

	return DATA_RTRVD;

}


int fn_write_packets_q( char *c_ServiceName,
 			               		char *c_err_msg
										)
{
	int i_cntr;


	if(DEBUG_MSG_LVL_3)
  {
		fn_userlog(c_ServiceName,"Number of packets to be sent of send q is :%d:",i_pack_counter);
	}

	for( i_cntr = 0 ; i_cntr < i_pack_counter ; i_cntr++ )
	{
		if(DEBUG_MSG_LVL_3)  /* Ver 1.7 */
  	{
			fn_userlog(c_ServiceName,"Writing packet :%d:",i_cntr+1);
			fn_userlog(c_ServiceName, "Message Type from SNDQ :%ld:",st_sndq_packets[i_cntr].li_msg_type);
			fn_userlog(c_ServiceName,"Sequence Number(Before Swap) Is :%lld:", ntohl(st_sndq_packets[i_cntr].st_bsnd_rqst_data.st_eti_order_req.ClOrdID));
      fn_userlog(c_ServiceName,"Val of TemplateID for Order Data : <%ld>", ntohs(st_sndq_packets[i_cntr].st_bsnd_rqst_data.st_eti_order_req.MessageHeaderIn.TemplateID));
		}

		if(fn_write_msg_q(i_qid, 
											(void *)&st_sndq_packets[i_cntr], 
											sizeof(struct st_send_rqst_data), 
											c_ServiceName, 
											c_err_msg)== -1)
     {
      fn_errlog(c_ServiceName, "S31280", LIBMSG, c_err_msg);
			fn_userlog(c_ServiceName,"ERROR IN SENDING PACKET :%d: on send queue",i_cntr+1);
      return -1;
     }
	}

	if(DEBUG_MSG_LVL_3)
  {
		fn_userlog(c_ServiceName,":%d: packet(s) sent succesfully on send queue",i_cntr);
	}

	memset(&st_sndq_packets, '\0', sizeof(st_sndq_packets));
	i_pack_counter = 0;


	return 0;
}
 
  	 
int fn_get_bse_scrip_cd( struct vw_contract *ptr_st_cnt,
                    struct vw_nse_cntrct *ptr_st_nse_cnt,
                    char *c_ServiceName,
                    char *c_err_msg)
{
  int i_len;
  int i_count;
  EXEC SQL BEGIN DECLARE SECTION;
    varchar c_symbl[21];
    char c_stck_cd[7];
    char c_exg_cd [ 4 ];
    varchar c_expiry_dt [ LEN_DATE ];
  EXEC SQL END DECLARE SECTION;

	if( DEBUG_MSG_LVL_0 ) 
	{
  	fn_userlog(c_ServiceName,"In function get_ext_cnt");
	}

  strcpy ( ( char * )c_expiry_dt.arr, ptr_st_cnt->c_expry_dt );
  SETLEN ( c_expiry_dt );
  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"c_expiry_dt is :%s:",c_expiry_dt.arr);
  }

  memset(ptr_st_nse_cnt, '\0', sizeof(struct vw_nse_cntrct));

  /*** Commented in Ver 1.6 ***
 
  EXEC SQL
        SELECT      NVL ( fcm_token_no, 0 ),
                    NVL( fcm_ca_lvl, 0 )
          INTO      :ptr_st_nse_cnt->l_token_id,
                    :ptr_st_nse_cnt->l_ca_lvl
          FROM      fcm_fo_cntrct_mstr
         WHERE      fcm_xchng_cd  = :ptr_st_cnt->c_xchng_cd
          *** AND      fcm_prdct_typ = :ptr_st_cnt->c_prd_typ  *** Commented in Ver 1.1 ***
					*** AND      fcm_prdct_typ =  decode(:ptr_st_cnt->c_prd_typ,'U','F',:ptr_st_cnt->c_prd_typ)    *** Ver 1.1 *** Com
mented in Ver 1.3 ***
           AND      fcm_prdct_typ =  decode(:ptr_st_cnt->c_prd_typ,'U','F','I','O',:ptr_st_cnt->c_prd_typ) *** OptionPLUS handling added in Ver 1.3 ***
           AND      fcm_undrlyng  = :ptr_st_cnt->c_undrlyng
           AND      fcm_expry_dt  =  to_date ( :ptr_st_cnt->c_expry_dt, 'dd-Mon-yyyy' )
           AND      fcm_exer_typ  = :ptr_st_cnt->c_exrc_typ
           AND      fcm_opt_typ   = :ptr_st_cnt->c_opt_typ
           AND      fcm_strk_prc  = :ptr_st_cnt->l_strike_prc;

    ***/

		/*** Ver 1.6 Starts ***/

   /** Ver 1.7 starts **/
  	if(DEBUG_MSG_LVL_5)
  	{
    	fn_userlog(c_ServiceName," ptr_st_cnt->c_xchng_cd |%s|",ptr_st_cnt->c_xchng_cd);
    	fn_userlog(c_ServiceName," ptr_st_cnt->c_undrlyng |%s|",ptr_st_cnt->c_undrlyng);
    	fn_userlog(c_ServiceName," ptr_st_cnt->c_prd_typ |%c|",ptr_st_cnt->c_prd_typ);
    	fn_userlog(c_ServiceName," ptr_st_cnt->c_expry_dt |%s|",ptr_st_cnt->c_expry_dt);
    	fn_userlog(c_ServiceName," ptr_st_cnt->c_exrc_typ |%c|",ptr_st_cnt->c_exrc_typ);
    	fn_userlog(c_ServiceName," ptr_st_cnt->c_opt_typ |%c|",ptr_st_cnt->c_opt_typ);
    	fn_userlog(c_ServiceName," ptr_st_cnt->l_strike_prc |%ld|",ptr_st_cnt->l_strike_prc);
  	}
    EXEC SQL
        SELECT      NVL ( btq_token_no, 0 ),
                    NVL( btq_ca_lvl, 0 )
          INTO      :ptr_st_nse_cnt->l_token_id,
                    :ptr_st_nse_cnt->l_ca_lvl
          FROM      btq_bfo_trd_qt
         WHERE      btq_xchng_cd  = :ptr_st_cnt->c_xchng_cd
           AND      btq_prdct_typ =  decode(:ptr_st_cnt->c_prd_typ,'U','F','P','F','I','O',:ptr_st_cnt->c_prd_typ)
           AND      btq_undrlyng  = :ptr_st_cnt->c_undrlyng
           AND      btq_expry_dt  =  to_date ( :ptr_st_cnt->c_expry_dt, 'dd-Mon-yyyy' )
           AND      btq_exer_typ  = :ptr_st_cnt->c_exrc_typ
           AND      btq_opt_typ   = :ptr_st_cnt->c_opt_typ
           AND      btq_strk_prc  = :ptr_st_cnt->l_strike_prc;

    /*** Ver 1.6 Ends   ***/
    /** Used BTQ table instead of ftq in Ver 1.7 and Ver 1.7 ends **/ 
  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31285", SQLMSG, c_err_msg );
    return -1;
  }

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName," ptr_st_nse_cnt->l_token_id is :%ld:",ptr_st_nse_cnt->l_token_id);
    fn_userlog(c_ServiceName," ptr_st_nse_cnt->l_ca_lvl is :%ld:",ptr_st_nse_cnt->l_ca_lvl);
  }
  return 0;
}


int fn_seq_to_bseomd(struct vw_xchngbook *ptr_st_xchngbook,
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

	if( DEBUG_MSG_LVL_0 ) 
	{
  	fn_userlog( c_ServiceName,"Function SEQ_TO_OMD");
    fn_userlog( c_ServiceName," INside Switch case ");
     fn_userlog( c_ServiceName,"c_xchng_cd:%s:",ptr_st_xchngbook->c_xchng_cd );
     fn_userlog( c_ServiceName,"c_pipe_id:%s:",ptr_st_xchngbook->c_pipe_id );
     fn_userlog( c_ServiceName,"mod_trd_dt:%s:",ptr_st_xchngbook->c_mod_trd_dt);
	}
  switch(ptr_st_xchngbook->c_oprn_typ)
  {
     fn_userlog( c_ServiceName," INside Switch case ");
     fn_userlog( c_ServiceName,"c_xchng_cd:%s:",ptr_st_xchngbook->c_xchng_cd );
     fn_userlog( c_ServiceName,"c_pipe_id:%s:",ptr_st_xchngbook->c_pipe_id );
     fn_userlog( c_ServiceName,"mod_trd_dt:%s:",ptr_st_xchngbook->c_mod_trd_dt);

    case FOR_SPRD:
			if( DEBUG_MSG_LVL_0 ) 
			{
    		fn_userlog( c_ServiceName,"for Spread selection, l_ord_seq :%ld:",ptr_st_xchngbook->l_ord_seq);
			}
    EXEC SQL
    SELECT  fxb_ordr_rfrnc,
            fxb_lmt_mrkt_sl_flg,
            fxb_dsclsd_qty,
            fxb_ordr_tot_qty,
            fxb_lmt_rt,
            fxb_stp_lss_tgr,
            fxb_mdfctn_cntr,
            to_char( fxb_ordr_valid_dt, 'dd-mon-yyyy' ),
            /* fxb_ordr_type, * Ver 1.4**/
            DECODE(fxb_ordr_type,'V','T',fxb_ordr_type),  /** Ver 1.4 **/ 
            fxb_sprd_ord_ind,
            fxb_rqst_typ,
            fxb_quote,
            to_char( fxb_qt_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            to_char( fxb_rqst_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            to_char( fxb_frwd_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            fxb_plcd_stts,
            fxb_rms_prcsd_flg,
            fxb_ors_msg_typ,
            to_char ( fxb_ack_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            fxb_xchng_rmrks,
            fxb_ex_ordr_typ,
            fxb_xchng_cncld_qty,
            fxb_spl_flag,
            fxb_ordr_sqnc
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
    WHERE   fxb_xchng_cd  = :ptr_st_xchngbook->c_xchng_cd
    AND     fxb_pipe_id   = :ptr_st_xchngbook->c_pipe_id
    AND     fxb_ordr_sqnc = :ptr_st_xchngbook->l_ord_seq
    AND     fxb_mod_trd_dt =  to_date(:ptr_st_xchngbook->c_mod_trd_dt,'dd-Mon-yyyy');

    break;

    case FOR_NORM:
			if( DEBUG_MSG_LVL_0 ) 
			{
    		fn_userlog( c_ServiceName,"for Normal selection, Pipe id:%s:",ptr_st_xchngbook->c_pipe_id);
		fn_userlog( c_ServiceName,"for Normal selection, exg_cd %s:",ptr_st_xchngbook->c_xchng_cd);
		fn_userlog( c_ServiceName,"for Normal selection, date %s:",ptr_st_xchngbook->c_mod_trd_dt);
			}
    EXEC SQL
    SELECT  fxb_ordr_rfrnc,
            fxb_lmt_mrkt_sl_flg,
            fxb_dsclsd_qty,
            fxb_ordr_tot_qty,
            fxb_lmt_rt,
            fxb_stp_lss_tgr,
            fxb_mdfctn_cntr,
            to_char( fxb_ordr_valid_dt, 'dd-mon-yyyy' ),
            /* fxb_ordr_type, * Ver 1.4 **/
            DECODE(fxb_ordr_type,'V','T',fxb_ordr_type),  /** Ver 1.4 **/ 
            fxb_sprd_ord_ind,
            fxb_rqst_typ,
            fxb_quote,
            to_char( fxb_qt_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            to_char ( fxb_rqst_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            to_char ( fxb_frwd_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            fxb_plcd_stts,
            fxb_rms_prcsd_flg,
            fxb_ors_msg_typ,
            to_char ( fxb_ack_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            fxb_xchng_rmrks,
            fxb_ex_ordr_typ,
            fxb_xchng_cncld_qty,
            fxb_spl_flag,
            fxb_ordr_sqnc
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
    WHERE   fxb_xchng_cd  = :ptr_st_xchngbook->c_xchng_cd
    AND     fxb_pipe_id   = :ptr_st_xchngbook->c_pipe_id
    AND     fxb_mod_trd_dt =  to_date(:ptr_st_xchngbook->c_mod_trd_dt, 'dd-Mon-yyyy')
    AND     fxb_ordr_sqnc = ( 
											SELECT  min(fxb_b.fxb_ordr_sqnc)
                      FROM     FXB_FO_XCHNG_BOOK fxb_b
                      WHERE    fxb_b.fxb_xchng_cd    = :ptr_st_xchngbook->c_xchng_cd
                      AND      fxb_b.fxb_mod_trd_dt  = to_date(:ptr_st_xchngbook->c_mod_trd_dt,'dd-Mon-yyyy')
                      AND      fxb_b.fxb_pipe_id     = :ptr_st_xchngbook->c_pipe_id
                      AND      fxb_b.fxb_plcd_stts   = 'R'
                      AND      nvl(fxb_b.fxb_rms_prcsd_flg, '*') != 'P');

    break;

    case FOR_EXER :							
			if( DEBUG_MSG_LVL_0 ) 
			{
    		fn_userlog( c_ServiceName,"for Excercise selection, Pipe id:%s:",ptr_st_xchngbook->c_pipe_id);
			}
    EXEC SQL
    SELECT  fxb_ordr_rfrnc,
            fxb_lmt_mrkt_sl_flg,
            fxb_dsclsd_qty,
            fxb_ordr_tot_qty,
            fxb_lmt_rt,
            fxb_stp_lss_tgr,
            fxb_mdfctn_cntr,
            to_char( fxb_ordr_valid_dt, 'dd-mon-yyyy' ),
            /**  fxb_ordr_type, ** Ver 1.4 **/
            DECODE(fxb_ordr_type,'V','T',fxb_ordr_type),  /** Ver 1.4 **/ 
            fxb_sprd_ord_ind,
            fxb_rqst_typ,
            fxb_quote,
            to_char( fxb_qt_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            to_char ( fxb_rqst_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            to_char ( fxb_frwd_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            fxb_plcd_stts,
            fxb_rms_prcsd_flg,
            fxb_ors_msg_typ,
            to_char ( fxb_ack_tm, 'dd-mon-yyyy hh24:mi:ss' ),
            fxb_xchng_rmrks,
            fxb_ex_ordr_typ,
            fxb_xchng_cncld_qty,
            fxb_spl_flag,
            fxb_ordr_sqnc
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
    WHERE   fxb_xchng_cd  = :ptr_st_xchngbook->c_xchng_cd
    AND     fxb_pipe_id   = :ptr_st_xchngbook->c_pipe_id
    AND     fxb_mod_trd_dt =  to_date(:ptr_st_xchngbook->c_mod_trd_dt, 'dd-Mon-yyyy')
    AND     fxb_ordr_sqnc = (
											SELECT  min(fxb_b.fxb_ordr_sqnc)
                      FROM     FXB_FO_XCHNG_BOOK fxb_b
                      WHERE    fxb_b.fxb_xchng_cd    = :ptr_st_xchngbook->c_xchng_cd
                      AND      fxb_b.fxb_mod_trd_dt  = to_date(:ptr_st_xchngbook->c_mod_trd_dt, 'dd-Mon-yyyy')
                      AND      fxb_b.fxb_pipe_id     = :ptr_st_xchngbook->c_pipe_id
                     	AND      fxb_b.fxb_plcd_stts   = 'R'
                      AND      fxb_b.fxb_ex_ordr_typ = 'E'
                      AND      fxb_b.fxb_ordr_sqnc != 0
                      AND      nvl(fxb_b.fxb_rms_prcsd_flg, '*') != 'P');
    break;
    default:
			if( DEBUG_MSG_LVL_0 ) 
			{
    		fn_userlog( c_ServiceName,"Invalid operation type :%s:",ptr_st_xchngbook->c_oprn_typ);
			}
    return -1;
  }

  if ( SQLCODE != 0 )
  {
    if ( SQLCODE == NO_DATA_FOUND )
    {
			if( DEBUG_MSG_LVL_0 ) 
			{
      	fn_userlog(c_ServiceName,"NDF for seq [%ld]",ptr_st_xchngbook->l_ord_seq);
			}
      return NO_DATA_FOUND;
    }
    else
    {
      fn_errlog ( c_ServiceName, "S31290", SQLMSG, c_err_msg );
      return -1;
    }
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

int fn_bseref_to_ord(struct vw_orderbook *ptr_st_orderbook,
                  char *c_ServiceName,
                  char *c_err_msg)
{
  EXEC SQL BEGIN DECLARE SECTION;
    short i_sprd_ord_rfrnc;
    varchar c_ucc_cd [11];
    varchar c_cp_code [11 + 2];
    short i_settlor;
    short i_ack_tm;
    short i_prev_ack_tm;
    short i_xchng_ack;
    short i_valid_dt;
  EXEC SQL END DECLARE SECTION;
    long l_vendor_cd;
    long l_prgrm_trdng_flg;

	if( DEBUG_MSG_LVL_0 ) 
	{
  	fn_userlog( c_ServiceName,"Function REF_TO_ORD");
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
              decode(:ptr_st_orderbook->c_oprn_typ,'S',
                      (decode(fod_prdct_typ,'P','F',fod_prdct_typ)),fod_prdct_typ),
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
      INTO    :ptr_st_orderbook->c_cln_mtch_accnt,
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
      FOR   UPDATE OF fod_ordr_rfrnc NOWAIT;

  if ( SQLCODE != 0 )
  {
    if( SQLCODE == -54 )
    {
			if( DEBUG_MSG_LVL_0 ) 
			{
     		fn_userlog(c_ServiceName,"In DISTRIBUTION_LOCK_ERR ");
			}
      fn_errlog ( c_ServiceName, "S31295", SQLMSG, c_err_msg );
      return LOCK;
    }
    else
    {
      fn_errlog ( c_ServiceName, "S31300", SQLMSG, c_err_msg );
      return -1;

    }

  }

	if(i_xchng_ack <0)  /** Added in Ver 1.7 **/
  {
    strcpy(ptr_st_orderbook->c_xchng_ack,"0");
  }

  EXEC SQL
    SELECT  NVL(RTRIM( CLM_CP_CD ), ' '),
            NVL(RTRIM( CLM_CLNT_CD ),CLM_MTCH_ACCNT )
    INTO    :c_cp_code,
            :c_ucc_cd
    FROM    CLM_CLNT_MSTR
    WHERE   CLM_MTCH_ACCNT = :ptr_st_orderbook->c_cln_mtch_accnt;

  if( SQLCODE != 0 )
  {
			if( DEBUG_MSG_LVL_0 ) 
			{
    		fn_userlog( c_ServiceName, "match account is :%s:",ptr_st_orderbook->c_cln_mtch_accnt);
			}
    fn_errlog( c_ServiceName, "S31305", SQLMSG, c_err_msg);
    return -1;
  }

  SETNULL(c_ucc_cd);
  SETNULL(c_cp_code);

  if( DEBUG_MSG_LVL_3 )
  {
    fn_userlog( c_ServiceName, "match account before is :%s:",ptr_st_orderbook->c_cln_mtch_accnt);
  }

  strcpy(ptr_st_orderbook->c_cln_mtch_accnt, (char *)c_ucc_cd.arr);
  strcpy(ptr_st_orderbook->c_settlor, (char *)c_cp_code.arr);

  rtrim ( ptr_st_orderbook->c_expry_dt );
  rtrim ( ptr_st_orderbook->c_ctcl_id );

  if( DEBUG_MSG_LVL_3 )
  {
    fn_userlog( c_ServiceName, "cp before is :%s:",c_cp_code.arr);
    fn_userlog( c_ServiceName,"jagan ctcl_id is :%s:",ptr_st_orderbook->c_ctcl_id);
    fn_userlog( c_ServiceName, "match account after is :%s:",ptr_st_orderbook->c_cln_mtch_accnt);
    fn_userlog( c_ServiceName, "cp after is :%s:",ptr_st_orderbook->c_settlor);
  }
  return 0;
}

int fn_upd_bse_xchngbk( struct vw_xchngbook *ptr_st_xchngbook,
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
    int   i_rec_exists;
    varchar c_xchng_rmrks [ 256 ];
  EXEC SQL END DECLARE SECTION;

  i_rec_exists = 0;

	if( DEBUG_MSG_LVL_0 ) 
	{
  	fn_userlog( c_ServiceName,"Function UPD_XCHNGBK");
	}

  if( DEBUG_MSG_LVL_3 )
  {
      fn_userlog( c_ServiceName, ":%s:",ptr_st_xchngbook->c_xchng_cd);
      fn_userlog( c_ServiceName, ":%s:",ptr_st_xchngbook->c_ordr_rfrnc);
      fn_userlog( c_ServiceName, ":%s:",ptr_st_xchngbook->c_pipe_id);
      fn_userlog( c_ServiceName, ":%s:",ptr_st_xchngbook->c_mod_trd_dt);
      fn_userlog( c_ServiceName, ":%ld:",ptr_st_xchngbook->l_ord_seq);
      fn_userlog( c_ServiceName, ":%c:",ptr_st_xchngbook->c_slm_flg);
      fn_userlog( c_ServiceName, ":%ld:",ptr_st_xchngbook->l_dsclsd_qty);
      fn_userlog( c_ServiceName, ":%ld:",ptr_st_xchngbook->l_ord_tot_qty);
      fn_userlog( c_ServiceName, ":%ld:",ptr_st_xchngbook->l_ord_lmt_rt);
      fn_userlog( c_ServiceName, ":%ld:",ptr_st_xchngbook->l_stp_lss_tgr);
      fn_userlog( c_ServiceName, ":%ld:",ptr_st_xchngbook->l_mdfctn_cntr);
      fn_userlog( c_ServiceName, ":%s:",ptr_st_xchngbook->c_valid_dt);
      fn_userlog( c_ServiceName, ":%c:",ptr_st_xchngbook->c_ord_typ);
      fn_userlog( c_ServiceName, ":%c:",ptr_st_xchngbook->c_sprd_ord_ind);
      fn_userlog( c_ServiceName, ":%c:",ptr_st_xchngbook->c_req_typ);
      fn_userlog( c_ServiceName, ":%ld:",ptr_st_xchngbook->l_quote);
      fn_userlog( c_ServiceName, ":%s:",ptr_st_xchngbook->c_qt_tm );
      fn_userlog( c_ServiceName, ":%s:",ptr_st_xchngbook->c_rqst_tm);
      fn_userlog( c_ServiceName, ":%c:",ptr_st_xchngbook->c_plcd_stts);
      fn_userlog( c_ServiceName, ":%c:",ptr_st_xchngbook->c_ex_ordr_typ);
      fn_userlog( c_ServiceName, ":%s:",ptr_st_xchngbook->c_xchng_rmrks);
      fn_userlog( c_ServiceName, ":%s:",ptr_st_xchngbook->c_mkrt_typ );
  }

  switch ( ptr_st_xchngbook->c_oprn_typ )
  {
    case  UPDATION_ON_ORDER_FORWARDING:
      EXEC SQL
      UPDATE  fxb_fo_xchng_book
      SET fxb_frwd_tm     = SYSDATE,
          fxb_plcd_stts   = :ptr_st_xchngbook->c_plcd_stts
      WHERE  fxb_ordr_rfrnc  = :ptr_st_xchngbook->c_ordr_rfrnc
      AND  fxb_mdfctn_cntr = :ptr_st_xchngbook->l_mdfctn_cntr;

    break;

    case UPDATION_ON_EXCHANGE_RESPONSE:
      i_rec_exists = 0;
      if ( ptr_st_xchngbook->l_dwnld_flg == DOWNLOAD )
      {
        EXEC SQL
          SELECT 1
          INTO :i_rec_exists
          FROM fxb_fo_xchng_book
          WHERE FXB_JIFFY = :ptr_st_xchngbook->d_jiffy
          AND FXB_XCHNG_CD = :ptr_st_xchngbook->c_xchng_cd
          AND FXB_PIPE_ID = :ptr_st_xchngbook->c_pipe_id;

        if ( ( SQLCODE != 0 ) && (SQLCODE != NO_DATA_FOUND) )
        {
          fn_errlog ( c_ServiceName, "S31310",SQLMSG, c_err_msg );
          return -1;
        }
        if ( i_rec_exists == 1 )
        {
          fn_errlog ( c_ServiceName, "S31315","Record already Processed", c_err_msg );
          return -1;
        }
      }

      strcpy ( ( char * )c_xchng_rmrks.arr, ptr_st_xchngbook->c_xchng_rmrks );
      rtrim ( c_xchng_rmrks.arr );
      SETLEN ( c_xchng_rmrks );

      EXEC SQL
          UPDATE  fxb_fo_xchng_book
             SET  fxb_plcd_stts       = :ptr_st_xchngbook->c_plcd_stts,
                  fxb_rms_prcsd_flg   = :ptr_st_xchngbook->c_rms_prcsd_flg,
                  fxb_ors_msg_typ     = :ptr_st_xchngbook->l_ors_msg_typ,
                  fxb_ack_tm          = to_date( :ptr_st_xchngbook->c_ack_tm,
                                                 'DD-Mon-yyyy hh24:mi:ss' ),
                  fxb_xchng_rmrks     = rtrim(fxb_xchng_rmrks)||:c_xchng_rmrks,
                  fxb_jiffy           = :ptr_st_xchngbook->d_jiffy
           WHERE  fxb_ordr_rfrnc      = :ptr_st_xchngbook->c_ordr_rfrnc
             AND  fxb_mdfctn_cntr     = :ptr_st_xchngbook->l_mdfctn_cntr;
      break;

    default :
      fn_errlog ( c_ServiceName, "S31320", "Invalid Operation Type", c_err_msg );
      return -1;

  }
  if ( SQLCODE != 0 )
  {
    if( SQLCODE == -2049 )
    {
			if( DEBUG_MSG_LVL_0 ) 
			{
      	fn_userlog(c_ServiceName,"In DISTRIBUTION_LOCK_ERR ");
			}
      return LOCK;
    }
    else
    {
      fn_errlog ( c_ServiceName, "S31325", SQLMSG, c_err_msg );
      return -1;
    }
  }

  return 0;
}


int fn_upd_bse_ordrbk(struct vw_orderbook *ptr_st_orderbook,
                  char *c_ServiceName,
                  char *c_err_msg)
{
	if( DEBUG_MSG_LVL_0 ) 
	{
 		fn_userlog( c_ServiceName,"Function UPD_ORDRBK");
	}	

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog ( c_ServiceName,"c_ordr_stts:%c:", ptr_st_orderbook->c_ordr_stts );
    fn_userlog ( c_ServiceName,"c_ordr_rfrnc:%s:", ptr_st_orderbook->c_ordr_rfrnc );
  }

  EXEC SQL
        UPDATE  fod_fo_ordr_dtls
        SET     fod_ordr_stts = :ptr_st_orderbook->c_ordr_stts
        WHERE   fod_ordr_rfrnc= :ptr_st_orderbook->c_ordr_rfrnc;

  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31330", SQLMSG, c_err_msg );
    return -1;
  }
  return 0;
}

int fn_qry_spdbk( struct vw_spdordbk *ptr_st_spd_ordbk,
                  char *c_ServiceName,
                  char *c_err_msg)
{
  int i_cnt = 0;

  EXEC SQL BEGIN DECLARE SECTION;
    sql_cursor     sys_cursor;
    short i_ind;
    char c_xchng_ack[ LEN_XCHNG_ACK ];
    char c_pipe_id[2+1];
    char c_ordr_rfrnc[18+1];
    char c_sprd_ord_rfrnc[18+1];
    char c_ordr_rfrnc_tmp[18+1];
    char c_sprd_ord_ind;
    char c_xchng_ack_tmp[ LEN_XCHNG_ACK ];
    char c_pipe_id_tmp[2+1];
    long int l_mdfctn_cntr;
    long int l_ors_msg_typ;
    long int l_ord_tot_qty;
    long int l_exctd_qty;
    long int l_can_qty;
    varchar c_last_mod_tm[ LEN_DATE ];
    varchar c_ack_tm[ LEN_DATE ];

  EXEC SQL END DECLARE SECTION;

	if( DEBUG_MSG_LVL_0 ) 
	{
  	fn_userlog( c_ServiceName,"Function QRY_SPDBK");
	}

  strcpy( c_ordr_rfrnc, (char *)ptr_st_spd_ordbk->c_ordr_rfrnc[0] );

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog ( c_ServiceName,"c_ordr_rfrnc  is :%s:", c_ordr_rfrnc );
  }

 	EXEC SQL ALLOCATE :sys_cursor;
  EXEC SQL EXECUTE
      BEGIN
        OPEN :sys_cursor FOR
          SELECT  fsd_sprd_rfrnc,
                  fsd_ordr_rfrnc,
                  fsd_sprd_ord_ind,
                  NVL(to_char(fsd_lst_mod_tm,'dd-mon-yyyy hh24:mi:ss'),'*'),
                  NVL(fsd_ack_nmbr,' '),
                  fsd_pipe_id,
                  fsd_mdfctn_cntr,
                  NVL(fsd_ors_msg_typ,0),
                  NVL ( fsd_ord_qty, 0 ),
                  NVL ( fsd_exec_qty, 0 ),
                  NVL ( fsd_cncl_qty, 0 ),
                  NVL ( to_char(fsd_ack_tm,'dd-mon-yyyy hh24:mi:ss'),'*')
           FROM   fsd_fo_sprd_dtls
           WHERE  fsd_ordr_rfrnc = :c_ordr_rfrnc;
       END;
  END-EXEC;

  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31335", SQLMSG, c_err_msg );
    EXEC SQL FREE :sys_cursor;
    return -1;
  }

  for ( i_cnt = 0; i_cnt <= 2; i_cnt++ )
  {
    EXEC SQL  FETCH :sys_cursor
                INTO  :c_sprd_ord_rfrnc,
                      :c_ordr_rfrnc_tmp,
                      :c_sprd_ord_ind,
                      :c_last_mod_tm,
                      :c_xchng_ack_tmp,
                      :c_pipe_id_tmp,
                      :l_mdfctn_cntr,
                      :l_ors_msg_typ,
                      :l_ord_tot_qty,
                      :l_exctd_qty,
                      :l_can_qty,
                      :c_ack_tm;
    if ( SQLCODE != 0 )
    {
      if ( SQLCODE == NO_DATA_FOUND )
      {
        fn_errlog ( c_ServiceName, "S31340", SQLMSG, c_err_msg );
				if(DEBUG_MSG_LVL_3)
    		{
					fn_userlog ( c_ServiceName,"No data found in spread book for ordr_rfrnc :%s:", c_ordr_rfrnc );
				}
        break;
      }

      fn_errlog ( c_ServiceName, "S31345", SQLMSG, c_err_msg );
      EXEC SQL CLOSE :sys_cursor;
      EXEC SQL FREE :sys_cursor;
      return -1;
    }

		strcpy( ptr_st_spd_ordbk->c_sprd_ord_rfrnc[i_cnt],c_sprd_ord_rfrnc);
		strcpy( ptr_st_spd_ordbk->c_ordr_rfrnc[i_cnt],c_ordr_rfrnc_tmp);
		ptr_st_spd_ordbk->c_sprd_ord_ind[i_cnt] = c_sprd_ord_ind;
		strcpy( ptr_st_spd_ordbk->c_xchng_ack[i_cnt], c_xchng_ack_tmp);
		strcpy( ptr_st_spd_ordbk->c_pipe_id[i_cnt], c_pipe_id_tmp);
		ptr_st_spd_ordbk->l_mdfctn_cntr[i_cnt] = l_mdfctn_cntr;
		ptr_st_spd_ordbk->l_ors_msg_typ[i_cnt] = l_ors_msg_typ;
		ptr_st_spd_ordbk->l_ord_tot_qty[i_cnt] = l_ord_tot_qty;
		ptr_st_spd_ordbk->l_exctd_qty[i_cnt] = l_exctd_qty;
		ptr_st_spd_ordbk->l_can_qty[i_cnt] = l_can_qty;

    SETNULL ( c_last_mod_tm );
    SETNULL ( c_ack_tm );

    if(strcmp((char *)c_last_mod_tm.arr,"*")== 0)
    {
      strcpy ( ptr_st_spd_ordbk->c_lst_md_tm[i_cnt] ," ");
    }
    else
    {
      strcpy ( ptr_st_spd_ordbk->c_lst_md_tm[i_cnt] ,( char * ) c_last_mod_tm.arr );
    }
    if(strcmp((char *)c_ack_tm.arr,"*")== 0)
    {
      strcpy ( ptr_st_spd_ordbk->c_ack_tm[i_cnt] ," ");
    }
    else
    {
      strcpy ( ptr_st_spd_ordbk->c_ack_tm[i_cnt] , ( char * ) c_ack_tm.arr );
    }
  }

  if ( i_cnt == 0 )
  {
    EXEC SQL EXECUTE
        BEGIN
          OPEN :sys_cursor FOR
            SELECT  fsd_sprd_rfrnc,
                    fsd_ordr_rfrnc,
                    fsd_sprd_ord_ind,
                    NVL(to_char ( fsd_lst_mod_tm, 'dd-mon-yyyy hh24:mi:ss' ),'*'),
                    NVL(fsd_ack_nmbr,' '),
                    fsd_pipe_id,
                    fsd_mdfctn_cntr,
                    NVL(fsd_ors_msg_typ,0),
                    NVL ( fsd_ord_qty, 0 ),
                    NVL ( fsd_exec_qty, 0 ),
                    NVL ( fsd_cncl_qty, 0 ),
                    NVL ( to_char ( fsd_ack_tm, 'dd-mon-yyyy hh24:mi:ss' ), '*' )
            FROM    fsd_fo_sprd_dtls_hstry
            WHERE   fsd_ordr_rfrnc = :c_ordr_rfrnc;
          END;
        END-EXEC;

    if ( SQLCODE != 0 )
    {
      fn_errlog ( c_ServiceName, "S31350", SQLMSG, c_err_msg );
      EXEC SQL FREE :sys_cursor;
      return -1;
    }

    for ( i_cnt = 0; i_cnt <= 2; i_cnt++ )
    {
      EXEC SQL  FETCH :sys_cursor
                  INTO  :c_sprd_ord_rfrnc,
                        :c_ordr_rfrnc_tmp,
                        :c_sprd_ord_ind,
                        :c_last_mod_tm,
                        :c_xchng_ack_tmp,
                        :c_pipe_id_tmp,
                        :l_mdfctn_cntr,
                        :l_ors_msg_typ,
                        :l_ord_tot_qty,
                        :l_exctd_qty,
                        :l_can_qty,
                        :c_ack_tm;
      if ( SQLCODE != 0 )
      {
        if ( SQLCODE == NO_DATA_FOUND )
        {
          break;
        }

        fn_errlog ( c_ServiceName, "S31355", SQLMSG, c_err_msg );
        EXEC SQL CLOSE :sys_cursor;
        EXEC SQL FREE :sys_cursor;
        return -1;
      }
			strcpy( ptr_st_spd_ordbk->c_sprd_ord_rfrnc[i_cnt],c_sprd_ord_rfrnc);
    	strcpy( ptr_st_spd_ordbk->c_ordr_rfrnc[i_cnt],c_ordr_rfrnc_tmp);
    	ptr_st_spd_ordbk->c_sprd_ord_ind[i_cnt] = c_sprd_ord_ind;
    	strcpy( ptr_st_spd_ordbk->c_xchng_ack[i_cnt], c_xchng_ack_tmp);
    	strcpy( ptr_st_spd_ordbk->c_pipe_id[i_cnt], c_pipe_id_tmp);
    	ptr_st_spd_ordbk->l_mdfctn_cntr[i_cnt] = l_mdfctn_cntr;
    	ptr_st_spd_ordbk->l_ors_msg_typ[i_cnt] = l_ors_msg_typ;
    	ptr_st_spd_ordbk->l_ord_tot_qty[i_cnt] = l_ord_tot_qty;
    	ptr_st_spd_ordbk->l_exctd_qty[i_cnt] = l_exctd_qty;
    	ptr_st_spd_ordbk->l_can_qty[i_cnt] = l_can_qty;


      SETNULL ( c_last_mod_tm );
      SETNULL ( c_ack_tm );
      if(strcmp((char *)c_last_mod_tm.arr,"*")== 0)
      {
        strcpy ( ptr_st_spd_ordbk->c_lst_md_tm[i_cnt] ," ");
      }
      else
      {
        strcpy ( ptr_st_spd_ordbk->c_lst_md_tm[i_cnt] ,( char * ) c_last_mod_tm.arr );
      }
      if(strcmp((char *)c_ack_tm.arr,"*")== 0)
      {
        strcpy ( ptr_st_spd_ordbk->c_ack_tm[i_cnt] ," ");
      }
      else
      {
        strcpy ( ptr_st_spd_ordbk->c_ack_tm[i_cnt] , ( char * ) c_ack_tm.arr );
      }
    }
    if ( i_cnt == 0 )
    {
      fn_errlog ( c_ServiceName, "S31360", "No data found in spread book history", c_err_msg );
      EXEC SQL FREE :sys_cursor;
      return -1;
    }

  }
	while (i_cnt >= 0 )
  {
		if( DEBUG_MSG_LVL_0 ) 
		{
    	fn_userlog ( c_ServiceName,"Printing the output structure :%d:",i_cnt);

    	fn_userlog ( c_ServiceName,"c_sprd_ord_rfrnc :%s: (%d)", ptr_st_spd_ordbk->c_sprd_ord_rfrnc[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"c_ordr_rfrnc  :%s: (%d)", ptr_st_spd_ordbk->c_ordr_rfrnc[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"c_sprd_ord_ind  :%c: (%d)", ptr_st_spd_ordbk->c_sprd_ord_ind[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"c_lst_md_tm :%s: (%d)", ptr_st_spd_ordbk->c_lst_md_tm[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"c_xchng_ack :%s: (%d)", ptr_st_spd_ordbk->c_xchng_ack[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"c_pipe_id :%s: (%d)", ptr_st_spd_ordbk->c_pipe_id[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"l_mdfctn_cntr :%ld: (%d)", ptr_st_spd_ordbk->l_mdfctn_cntr[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"l_ors_msg_typ :%ld: (%d)", ptr_st_spd_ordbk->l_ors_msg_typ[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"l_ord_tot_qty :%ld: (%d)", ptr_st_spd_ordbk->l_ord_tot_qty[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"l_exctd_qty :%ld: (%d)", ptr_st_spd_ordbk->l_exctd_qty[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"l_can_qty :%ld: (%d)", ptr_st_spd_ordbk->l_can_qty[i_cnt],i_cnt );
    	fn_userlog ( c_ServiceName,"c_ack_tm  :%s: (%d)", ptr_st_spd_ordbk->c_ack_tm[i_cnt],i_cnt );
		}
		i_cnt--; 
  }

  EXEC SQL CLOSE :sys_cursor;
  EXEC SQL FREE :sys_cursor;
  return 0;
}

int fn_exrq_dtls( struct vw_exrcbook *ptr_st_exrcbook,
                  char *c_ServiceName,
                  char *c_err_msg)
{
  EXEC SQL BEGIN DECLARE SECTION;
    varchar c_expiry_dt [ LEN_DATE ];
    varchar c_trade_dt [ LEN_DATE ];
    varchar c_ucc_cd [11];
    varchar c_cp_code [11 + 2];
    short i_settlor ;
    short i_xchng_ack ;
    short  i_ack_tm;
  EXEC SQL END DECLARE SECTION;

	if( DEBUG_MSG_LVL_0 ) 
	{
  	fn_userlog( c_ServiceName,"Function EXRQ_DTLS");
	}

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog ( c_ServiceName,"Exercise Ref. No:%s:", ptr_st_exrcbook->c_exrc_ordr_rfrnc );
  }

  EXEC SQL
      SELECT  feb_clm_mtch_accnt,
              feb_clnt_ctgry,
              feb_pipe_id,
              feb_xchng_cd,
              feb_prdct_typ,
              feb_indstk,
              feb_undrlyng,
              to_char ( feb_expry_dt, 'dd-Mon-yyyy' ),
              feb_exer_typ,
              feb_opt_typ,
              feb_strk_prc,
              feb_exrc_rqst_typ,
              feb_exrc_qty,
              feb_exrc_stts,
              to_char ( feb_trd_dt, 'dd-Mon-yyyy' ),
              feb_mdfctn_cntr,
              feb_settlor,
              feb_ack_nmbr,
              to_char ( feb_exer_ack_tm, 'dd-Mon-yyyy hh24:mi:ss' ),
              feb_pro_cli_ind
      INTO    :ptr_st_exrcbook->c_cln_mtch_accnt,
              :ptr_st_exrcbook->l_clnt_ctgry,
              :ptr_st_exrcbook->c_pipe_id,
              :ptr_st_exrcbook->c_xchng_cd,
              :ptr_st_exrcbook->c_prd_typ,
              :ptr_st_exrcbook->c_ctgry_indstk,
              :ptr_st_exrcbook->c_undrlyng,
              :ptr_st_exrcbook->c_expry_dt,
              :ptr_st_exrcbook->c_exrc_typ,
              :ptr_st_exrcbook->c_opt_typ,
              :ptr_st_exrcbook->l_strike_prc,
              :ptr_st_exrcbook->c_exrc_rqst_typ,
              :ptr_st_exrcbook->l_exrc_qty,
              :ptr_st_exrcbook->c_exrc_stts,
              :c_trade_dt,
              :ptr_st_exrcbook->l_mdfctn_cntr,
              :ptr_st_exrcbook->c_settlor:i_settlor,
              :ptr_st_exrcbook->c_xchng_ack:i_xchng_ack,
              :ptr_st_exrcbook->c_ack_tm:i_ack_tm,
              :ptr_st_exrcbook->c_pro_cli_ind
      FROM    feb_fo_exrc_rqst_book
      WHERE   feb_exrc_rfrnc_no = :ptr_st_exrcbook->c_exrc_ordr_rfrnc
      FOR UPDATE OF feb_exrc_rfrnc_no NOWAIT;


  if ( ( SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
  {
    if( SQLCODE == -54  )
    {
			if( DEBUG_MSG_LVL_0 ) 
			{
      	fn_userlog(c_ServiceName,"In DISTRIBUTION_LOCK_ERR ");
			}
      fn_errlog ( c_ServiceName, "S31365", SQLMSG, c_err_msg );
      return LOCK;
    }
    else
    {
      fn_errlog ( c_ServiceName, "S31370", SQLMSG, c_err_msg );
      return -1;
    }
  }

	if(i_xchng_ack <0)  /** Added in Ver 1.7 **/
	{
		strcpy(ptr_st_exrcbook->c_xchng_ack,"0");	
	}

  EXEC SQL
  SELECT  NVL(RTRIM( CLM_CP_CD ), ' '),
          NVL(RTRIM( CLM_CLNT_CD ), CLM_MTCH_ACCNT)
  INTO    :c_cp_code,
          :c_ucc_cd
  FROM    CLM_CLNT_MSTR
  WHERE   CLM_MTCH_ACCNT = :ptr_st_exrcbook->c_cln_mtch_accnt;

  if( SQLCODE != 0 )
  {
		if( DEBUG_MSG_LVL_0 ) 
		{
    	fn_userlog( c_ServiceName, "match account is :%s:",ptr_st_exrcbook->c_cln_mtch_accnt);
		}
    fn_errlog( c_ServiceName, "S31375", SQLMSG, c_err_msg);
    return -1;
  }

  SETNULL(c_ucc_cd);
  SETNULL(c_cp_code);

  strcpy(ptr_st_exrcbook->c_cln_mtch_accnt, (char *)c_ucc_cd.arr);
  strcpy(ptr_st_exrcbook->c_settlor, (char *)c_cp_code.arr);

	if( DEBUG_MSG_LVL_0 ) 
	{
  	fn_userlog( c_ServiceName, "match account is :%s:",ptr_st_exrcbook->c_cln_mtch_accnt);
	}

  if( DEBUG_MSG_LVL_3 )
  {
    fn_userlog( c_ServiceName, "cp before is :%s:",c_cp_code.arr);
    fn_userlog( c_ServiceName, "match account after is :%s:",ptr_st_exrcbook->c_cln_mtch_accnt);
    fn_userlog( c_ServiceName, "cp after is :%s:",ptr_st_exrcbook->c_settlor);
  }

  SETNULL(c_trade_dt);
  strcpy(ptr_st_exrcbook->c_trd_dt, (char *)c_trade_dt.arr);
  rtrim( ptr_st_exrcbook->c_expry_dt );

  return 0;
}

int fn_upd_exbk( struct vw_exrcbook *ptr_st_exrcbook,
                 char *c_ServiceName,
                 char *c_err_msg)
{
	if( DEBUG_MSG_LVL_0 ) 
	{
  	fn_userlog( c_ServiceName,"Function UPD_EXBK");
	}

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog ( c_ServiceName,"Exercise order reference in :%s:", ptr_st_exrcbook->c_exrc_ordr_rfrnc );
  }

  EXEC SQL
      UPDATE  feb_fo_exrc_rqst_book
      SET     feb_exrc_stts = :ptr_st_exrcbook->c_exrc_stts
      WHERE   feb_exrc_rfrnc_no = :ptr_st_exrcbook->c_exrc_ordr_rfrnc;

  if ( SQLCODE != 0 )
  {
    fn_errlog ( c_ServiceName, "S31380", SQLMSG, c_err_msg );
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

