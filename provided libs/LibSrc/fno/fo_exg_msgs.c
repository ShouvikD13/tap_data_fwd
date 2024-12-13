/* Ver 1.1 : 24-May-2013 - Code Optimization - Sachin Birje ***/
/* Ver 1.2 - 28-May-2013 - TimeStamp Printing (Mahesh Shinde) */
/* Ver 1.3 - 05-Jun-2013 - Broker closeout changes Mahesh Shinde */
/* Ver 1.4 - 18-Jun-2013 - Broker closeout &RRM changes    Mahesh Shinde.    */
/* Ver 1.5 - 28-Feb-2014 - CR_ISEC14_48665 INDIA VIX Mahesh Shinde		*/
/* Ver 1.6 - 03-Jul-2015 - FCM-FTQ Merger Changes | Anand Dhopte  */
/* Ver 1.7 - 13-May-2016 - change from acall to call | Tanmay W.  */
/* Ver 1.8 - 12-Jun-2017 - Core Fix | Mrinal Kishore							*/
/* Ver 1.9 - 13-Apr-2018 - Stream No. Exchange Exception Handling Changes	| Parag Kanojia	*/
/* Ver 2.0 - 19-Feb-2019 - Fo Direct Connectivity  | Bhushan Harekar (19-Feb-2019) */
/* Ver TOL - 02-Aug-2023 - Tux on Linux | Agam (02-Aug-2023) */



#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include 	<sqlca.h>
#include  <fn_log.h>
#include  <fo.h>
#include <atmi.h>
#include <fn_tuxlib.h>
#include <fn_ddr.h>
#include <fn_scklib_tap.h>
#include <fo_view_def.h>
#include <fo_exg_msgs.h>
#include <fn_read_debug_lvl.h>
#include <ctype.h>                        /*** Ver 1.1 ***/
#include <time.h>  /** Ver 1.2 **/
#include <fo_exch_comnN.h>
#include <sys/time.h>   /* Ver TOL Tux on Linux */
#include <eba_to_ors.h> /* Ver TOL Tux on Linux */
EXEC SQL INCLUDE  "table/sem_stck_map.h"; /*** Ver 1.1 ***/


static short int si_mkt_stts;

/****** Ver 1.2 ****/

/* VER TOL : TUX on LINUX -- "fn_gettime" name chnaged to "fn_gettime_em" (Ravindra) */
void fn_gettime_em(void);
char c_time[12];

/***** Ver 1.2 ****/

int fn_get_reset_seq( char *c_pipe,
                char *c_trd_date,
                char c_req_typ,
								int *ptr_i_seq,
                char *c_ServiceName,
                char *c_err_msg
               )
	{

		struct vw_sequence st_s_sequence;
      /*
        struct vw_sequence
        {
          long l_seq_num;		
          char c_pipe_id[3];	
          char c_trd_dt[23];	
          char c_rqst_typ;	
          char c_rout_crt[4]; 
        };
      */
		struct vw_sequence st_r_sequence;

		
		int i_ip_len;
		int i_op_len;
		int i_returncode;

		memset(&st_s_sequence, '\0', sizeof(struct vw_sequence));
		memset(&st_r_sequence, '\0', sizeof(struct vw_sequence));

		strcpy( st_s_sequence.c_pipe_id,c_pipe);
		strcpy( st_s_sequence.c_trd_dt, c_trd_date );
		st_s_sequence.c_rqst_typ = c_req_typ;
		fn_cpy_ddr ( st_s_sequence.c_rout_crt );


      /* this function is inside the "fn_ddr"

        void fn_cpy_ddr ( char *c_ddr_var )
        {
          strcpy ( c_ddr_var, c_ddr_str );
          return;
        }
      */
	
		i_ip_len = sizeof ( struct vw_sequence );
		i_op_len = sizeof ( struct vw_sequence );	
	 	
		i_returncode = fn_call_svc(c_ServiceName,
					     									c_err_msg,
					     									&st_s_sequence,
					     									&st_r_sequence,
					     									"vw_sequence",
					     									"vw_sequence",
					     									i_ip_len,
					     									i_op_len,
					     									0,
					     									"SFO_GET_SEQ" );

    /*
        
    */
		 if ( i_returncode != SUCC_BFR )
		 {
		   fn_errlog( c_ServiceName, "S31120", LIBMSG, c_err_msg  );
		   return -1;
		 }

		  if ( st_r_sequence.l_seq_num == MAX_SEQ_NUM )  // #define MAX_SEQ_NUM 2147483647 
		  {
				st_s_sequence.c_rqst_typ = RESET_PLACED_SEQ;
			
			
			i_returncode = fn_call_svc( 		 c_ServiceName,
																			 c_err_msg,
																			 &st_s_sequence,
																			 &st_r_sequence,
																			 "vw_sequence",
																			 "vw_sequence",
																			 i_ip_len,
																			 i_op_len,
																			 0,
											 								 "SFO_GET_SEQ" );
			 if ( i_returncode != SUCC_BFR )
			 {
			   fn_errlog( c_ServiceName, "S31120", LIBMSG, c_err_msg  );
			   return -1;
			 }
			}	
	
			*ptr_i_seq = st_r_sequence.l_seq_num;
	
			return 0;
	}


/* to ham jo yaha se samjhe wo ye ki hm call karenge tpcall ko wo lake dega apne ko 'sequence number' agar wo sequence number (MAX_value par) nhi hai to ham return karenge same fetched sequence number from tpcall.   */
	/** Function to get the contract description  ***/
void fn_orstonse_cntrct_desc (struct vw_nse_cntrct *ptr_eba_cntrct,
															struct st_contract_desc *ptr_nse_cntrct,
															char *c_ServiceName, 
															char *c_err_msg)
	{
		if ( ptr_eba_cntrct->c_prd_typ == 'F')
		{
			ptr_nse_cntrct->c_instrument_name[0] = 'F';
			ptr_nse_cntrct->c_instrument_name[1] = 'U';
			ptr_nse_cntrct->c_instrument_name[2] = 'T';
		}
		else if(ptr_eba_cntrct->c_prd_typ == 'O' )
		{
			ptr_nse_cntrct->c_instrument_name[0] = 'O';
			ptr_nse_cntrct->c_instrument_name[1] = 'P';
			ptr_nse_cntrct->c_instrument_name[2] = 'T';
		}
		else
		{
			fn_userlog(c_ServiceName, "Invalid product type");
		}

		if ( ptr_eba_cntrct->c_ctgry_indstk == 'I')
		{
			ptr_nse_cntrct->c_instrument_name[3] = 'I';

			/**** Ver 1.5 ****/

			if(DEBUG_MSG_LVL_3) 
    	{
      	fn_userlog(c_ServiceName,"before ptr_eba_cntrct->c_symbol:%s:",ptr_eba_cntrct->c_symbol);
    	}

			if (strncmp(ptr_eba_cntrct->c_symbol,"INDIAVIX",(int)strlen("INDIAVIX")) == 0)  
			{
				if(DEBUG_MSG_LVL_3) 
    		{
      		fn_userlog(c_ServiceName,"Inside INDIAVIX");
    		}
				ptr_nse_cntrct->c_instrument_name[4] = 'V';	
			}	
			else
			{
				ptr_nse_cntrct->c_instrument_name[4] = 'D';
			}
			
			if(DEBUG_MSG_LVL_3)
    	{
      	fn_userlog(c_ServiceName,"before ptr_nse_cntrct->c_instrument_name |%s|",ptr_nse_cntrct->c_instrument_name);
    	}

			/**** End of Ver 1.5 ****/

			ptr_nse_cntrct->c_instrument_name[5] = 'X';
		}
		else if(ptr_eba_cntrct->c_ctgry_indstk == 'S' )
		{
			ptr_nse_cntrct->c_instrument_name[3] = 'S';
			ptr_nse_cntrct->c_instrument_name[4] = 'T';
			ptr_nse_cntrct->c_instrument_name[5] = 'K';
		}
		else
		{
			fn_userlog(c_ServiceName, "Should be Index/stock");
		}

		fn_orstonse_char ( ptr_nse_cntrct->c_symbol,
					LEN_SYMBOL_NSE,
					ptr_eba_cntrct->c_symbol,
					strlen(ptr_eba_cntrct->c_symbol) );

		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"before ptr_eba_cntrct->c_expry_dt:%s:",ptr_eba_cntrct->c_expry_dt);
			fn_userlog(c_ServiceName,"before ptr_nse_cntrct->c_instrument_name |%s|",ptr_nse_cntrct->c_instrument_name);/*ver1.5*/
		}

		strcat ( ptr_eba_cntrct->c_expry_dt, " 14:30:00" );

		if(DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName,"after ptr_eba_cntrct->c_expry_dt:%s:",ptr_eba_cntrct->c_expry_dt);
		}

		fn_timearr_to_long(ptr_eba_cntrct->c_expry_dt,&(ptr_nse_cntrct->li_expiry_date));

		if(DEBUG_MSG_LVL_1)
		{
			fn_userlog(c_ServiceName,"after conversion ptr_nse_cntrct->li_expiry_date:%ld:",ptr_nse_cntrct->li_expiry_date);
		}

		if ( ptr_eba_cntrct->c_prd_typ == 'F')
		{
			ptr_nse_cntrct->li_strike_price = -1;
		}
		else
		{
			ptr_nse_cntrct->li_strike_price = ptr_eba_cntrct->l_strike_prc;
		}

		if ( ptr_eba_cntrct->c_prd_typ == 'O' )
		{
			ptr_nse_cntrct->c_option_type[0] = ptr_eba_cntrct->c_opt_typ;
			ptr_nse_cntrct->c_option_type[1] = ptr_eba_cntrct->c_exrc_typ;
		}
		else
		{
			ptr_nse_cntrct->c_option_type[0] = 'X';
			ptr_nse_cntrct->c_option_type[1] = 'X';
		}
		ptr_nse_cntrct->si_ca_level = (short int)ptr_eba_cntrct->l_ca_lvl;
	}

int fn_mrkt_stts_hndlr (short int si_mkt_stts,
                        long int li_mkt_type,
                        char *c_xchng_cd,
                        char *c_ServiceName,
                        char *c_err_msg )
{
  char c_mrkt_stts;
  char c_mkt_typ;

  int i_ch_val;

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Inside Function fn_mrkt_stts_hndlr");
	}

  if( li_mkt_type != PL_MKT )
  {
		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Inside Market Type");
		}
    if(DEBUG_MSG_LVL_3)
			{
      	fn_userlog(c_ServiceName,"Inside Market Status Check");
			} /* Ver TOL Tux on Linux */
    switch(si_mkt_stts)
    {
			/* if(DEBUG_MSG_LVL_3)
			 {
       	fn_userlog(c_ServiceName,"Inside Market Status Check");
			 }     Ver TOL Tux on Linux */
      case PRE_OPEN :
      case PRE_OPEN_ENDED :
      case POST_CLOSE :
        return 0;
        break;

      case OPEN :
				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside OPEN Case");
				}
        c_mrkt_stts   = 'O';
				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Status Is :%c:",c_mrkt_stts);
				}
        break;

      case CLOSED :
				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside CLOSED Case");
				}
        c_mrkt_stts   = 'C';
				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Status Is :%c:",c_mrkt_stts);
				}
        break;

      case 'X' :
			
				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside Expired Case");
				}
        c_mrkt_stts   = 'X';
				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Status Is :%c:",c_mrkt_stts);
				}
        break;
    }

    switch (li_mkt_type)
    {
      case ORDER_MKT :

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside ORDER_MKT Case");
				}
        c_mkt_typ = ORDER_MARKET;
				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Type Is :%c:",c_mkt_typ);
				}
        break;

      case EXTND_MKT :

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside EXTND_MKT Case");
				}
	
        c_mkt_typ = EXTENDED_SEGMENT;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Type Is :%c:",c_mkt_typ);
				}
        break;

      case EXERCISE_MKT :

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside EXERCISE_MKT Case");
				}

        c_mkt_typ = EXERCISE_SEGMENT;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Type Is :%c:",c_mkt_typ);
				}
        break;

      case PL_MKT :

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside PL_MKT Case");
				}
        c_mkt_typ = PL_MARKET;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Type Is :%c:",c_mkt_typ);
				}
        break;
    }
  }

	i_ch_val = fn_mrkt_stts ( c_mrkt_stts,
                            c_mkt_typ,
                            c_xchng_cd,
                            c_ServiceName,
                            c_err_msg );

  if ( i_ch_val == -1 )
  {
  	fn_userlog(c_ServiceName,"Failed While calling Function fn_mrkt_stts");
    return -1;
  }

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"After Complition Of fn_mrkt_stts_hndlr");
	}
  return 0;
}

int fn_mrkt_stts ( char c_mrkt_stts,
                   char c_mkt_typ,
                   char *c_xchng_cd,
                   char *c_ServiceName,
                   char *c_err_msg )
{
  int i_ch_val;

  struct vw_xchngstts st_stts;

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Inside Function fn_mrkt_stts");
	}

  strcpy( st_stts.c_xchng_cd,c_xchng_cd);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_stts.c_xchng_cd);
  	fn_userlog(c_ServiceName,"Here Market Type Is :%c:",c_mkt_typ);
	}

  if( c_mkt_typ == EXERCISE_SEGMENT )
  {
		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Inside EXERCISE_SEGMENT Check");
		}
    st_stts.c_exrc_mkt_stts = c_mrkt_stts;
    st_stts.c_rqst_typ = UPD_EXER_MKT_STTS;
	
		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Exercise Market status Is :%c:",st_stts.c_exrc_mkt_stts);
    	fn_userlog(c_ServiceName,"Request Type Is ;%c:",st_stts.c_rqst_typ);
		}
  }

  fn_cpy_ddr(st_stts.c_rout_crt);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Before Call To SFO_UPD_XSTTS");
	}

  i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_err_msg,
                            &st_stts,
                            "vw_xchngstts",
                            sizeof (st_stts ),
                            TPNOREPLY,
                            "SFO_UPD_XSTTS" );

  if ( i_ch_val == SYSTEM_ERROR )
  {
    fn_errlog ( c_ServiceName, "S31060", LIBMSG, c_err_msg );
    return -1;
  }

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"After Complition Of fn_mrkt_stts");
	}
  return 0;
}

/******************************************************************************/
/*  To update contract informations which have got from exchange.             */
/*  INPUT PARAMETERS                                                          */
/*      st_cntrct_info_msg  - contract information message                    */
/*      c_errmsg           - Error message to be returned in case of error    */
/*  OUTPUT PARAMETERS                                                         */
/*      void                                                                  */
/******************************************************************************/

int fn_security_mstr_chg ( struct st_security_update_info *ptr_st_sec_upd_info,
                           char *c_xchng_cd,
                           char *c_ServiceName,
                           char *c_errmsg)
{
  struct vw_contract st_cntrct_dtls;
  struct vw_cntrt_gen_inf st_gen_info;
  struct st_cntrct_info  st_cnt_info;
  nfo_cntrct st_nse_cntrct;

  int i_ch_val;

	if(DEBUG_MSG_LVL_0)
	{
		fn_gettime_em();   /*** Ver 1.2 ***/
    fn_userlog(c_ServiceName,"TIME_STATS NDF inside fn_security_mstr_chg at :%s:",c_time); /*** Ver 1.2 ***/
  	fn_userlog(c_ServiceName,"Inside Function fn_security_mstr_chg ");
  	fn_userlog(c_ServiceName,"Exchange code is :%s:",c_xchng_cd);
	}

	if ( strcmp (c_xchng_cd,"NFO") == 0 )
	{
  	strcpy(st_nse_cntrct.c_xchng_cd,c_xchng_cd);

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Exchange Product Type Is :%c:",ptr_st_sec_upd_info->st_sc_info.c_instrument_name[0]);
    	fn_userlog(c_ServiceName,"Exchange Expiry Date Is :%ld:",ptr_st_sec_upd_info->st_sc_info.li_expiry_date);
    	fn_userlog(c_ServiceName,"Exchange Exercise Type Is :%c:",ptr_st_sec_upd_info->st_sc_info.c_option_type[1]);
    	fn_userlog(c_ServiceName,"Exchange Option Type Is :%c:",ptr_st_sec_upd_info->st_sc_info.c_option_type[0]);
    	fn_userlog(c_ServiceName,"Exchange Strike Price is :%ld:",ptr_st_sec_upd_info->st_sc_info.li_strike_price);
    	fn_userlog(c_ServiceName,"Exchange IND/STK is ;%c:",ptr_st_sec_upd_info->st_sc_info.c_instrument_name[3]);
    	fn_userlog(c_ServiceName,"Exchange CA LVL Is :%d:",ptr_st_sec_upd_info->st_sc_info.si_ca_level);
    	fn_userlog(c_ServiceName,"Exchange Symbol Is :%s:",ptr_st_sec_upd_info->st_sc_info.c_symbol);
    	fn_userlog(c_ServiceName,"Exchange Series Is :%s:",ptr_st_sec_upd_info->st_sc_info.c_series);
    	fn_userlog(c_ServiceName,"Exchange Token Id Is :%ld:",ptr_st_sec_upd_info->l_token);
		}

  	st_nse_cntrct.c_prd_typ = ptr_st_sec_upd_info->st_sc_info.c_instrument_name[0];
  	fn_long_to_timearr(st_nse_cntrct.c_expry_dt,ptr_st_sec_upd_info->st_sc_info.li_expiry_date );
  	st_nse_cntrct.c_exrc_typ = ptr_st_sec_upd_info->st_sc_info.c_option_type[1];
  	st_nse_cntrct.c_opt_typ = ptr_st_sec_upd_info->st_sc_info.c_option_type[0];
  	st_nse_cntrct.l_strike_prc = ptr_st_sec_upd_info->st_sc_info.li_strike_price;
  	st_nse_cntrct.c_ctgry_indstk = ptr_st_sec_upd_info->st_sc_info.c_instrument_name[3];

		if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
    {
      fn_gettime_em();
      fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_nsetoors_char at :%s:",c_time);
    }

  	fn_nsetoors_char ( st_nse_cntrct.c_symbol,
    	                 LEN_SYMBOL,
      	               ptr_st_sec_upd_info->st_sc_info.c_symbol,
        	             LEN_SYMBOL_NSE );

  	fn_nsetoors_char ( st_nse_cntrct.c_series,
    	                 LEN_SERIES,
      	               ptr_st_sec_upd_info->st_sc_info.c_series,
        	             LEN_SERIES_NSE );

  	st_nse_cntrct.l_ca_lvl = ptr_st_sec_upd_info->st_sc_info.si_ca_level;
  	st_nse_cntrct.l_token_id = ptr_st_sec_upd_info->l_token;

		if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
    {
      fn_gettime_em();
      fn_userlog(c_ServiceName,"TIME_STATS NDF after fn_nsetoors_char at :%s:",c_time);
    }

		if(DEBUG_MSG_LVL_3)
		{
  		fn_userlog(c_ServiceName,"After Conversion Into EBA Format");
  		fn_userlog(c_ServiceName,"Product Type Is :%c:",st_nse_cntrct.c_prd_typ);
  		fn_userlog(c_ServiceName,"Expiry Date Is :%ld:",st_nse_cntrct.c_expry_dt);
  		fn_userlog(c_ServiceName,"Exercise Type Is :%c:",st_nse_cntrct.c_exrc_typ);
  		fn_userlog(c_ServiceName,"Option Type Is :%c:",st_nse_cntrct.c_opt_typ);
  		fn_userlog(c_ServiceName,"Strike Price is :%ld:",st_nse_cntrct.l_strike_prc);
  		fn_userlog(c_ServiceName,"IND/STK is ;%c:",st_nse_cntrct.c_ctgry_indstk);
  		fn_userlog(c_ServiceName,"CA LVL Is :%d:",st_nse_cntrct.l_ca_lvl);
  		fn_userlog(c_ServiceName,"Symbol Is :%s:",st_nse_cntrct.c_symbol);
  		fn_userlog(c_ServiceName,"Series Is :%s:",st_nse_cntrct.c_series);
  		fn_userlog(c_ServiceName,"Token Id Is :%ld:",st_nse_cntrct.l_token_id);
		}

		if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
    {
      fn_gettime_em();
      fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_nse_to_eba_cntrct at :%s:",c_time);
    }

    i_ch_val = fn_nse_to_eba_cntrct ( st_nse_cntrct,
                                      &st_cntrct_dtls,
                                      c_ServiceName,
                                      c_errmsg );
    if ( i_ch_val == SYSTEM_ERROR )
    {
      fn_errlog(c_ServiceName, "S31040", LIBMSG, c_errmsg);
      return -1;
    }
    else if ( i_ch_val != 0 )
    {
      return 0;
    }
		
		if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
    {
      fn_gettime_em();
      fn_userlog(c_ServiceName,"TIME_STATS NDF after fn_nse_to_eba_cntrct at :%s:",c_time);
    }
  }
  else if( strcmp ( c_xchng_cd,"BFO") == 0 )
  {
    /*  BSE code to be added here   */
    fn_errlog ( c_ServiceName, "S31045",
                "BSE code not handled", c_errmsg );
    return 0;
  }

  /*  populate general contract information view  */
  st_gen_info.l_eba_cntrct_id = st_nse_cntrct.l_token_id;
  strcpy( st_gen_info.c_xchng_cd, st_cntrct_dtls.c_xchng_cd);
  st_gen_info.c_prd_typ = st_cntrct_dtls.c_prd_typ;
  strcpy( st_gen_info.c_undrlyng, st_cntrct_dtls.c_undrlyng);
  strcpy( st_gen_info.c_expry_dt , st_cntrct_dtls.c_expry_dt);
  st_gen_info.c_exrc_typ = st_cntrct_dtls.c_exrc_typ;
	st_gen_info.c_opt_typ = st_cntrct_dtls.c_opt_typ;
  st_gen_info.l_strike_prc = st_cntrct_dtls.l_strike_prc;
  st_gen_info.c_ctgry_indstk = st_cntrct_dtls.c_ctgry_indstk;
  st_gen_info.l_ca_lvl = st_cntrct_dtls.l_ca_lvl;
  st_gen_info.l_prmtd_to_trd = ptr_st_sec_upd_info->si_permited_trade;

 
  if(ptr_st_sec_upd_info->st_sec_elg[0].si_stts == SEC_SUSPENDED )
  {
		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"In Condition Of SEC_SUSPENDED");
		}
    st_gen_info.l_stts = 0;

		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Status Is :%ld:",st_gen_info.l_stts);
		}

  }
  else
  {
    st_gen_info.l_stts = 1;

		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Status Is :%ld:",st_gen_info.l_stts);
		}
  }
  st_gen_info.l_min_lot_qty = ptr_st_sec_upd_info->li_minimum_lot_quantity;
  st_gen_info.l_board_lot_qty = ptr_st_sec_upd_info->li_board_lot_quantity;
  st_gen_info.l_tick_sz = ptr_st_sec_upd_info->li_tick_size;
  st_gen_info.l_hgh_prc_rng = ptr_st_sec_upd_info->li_high_price_range;
  st_gen_info.l_low_prc_rng = ptr_st_sec_upd_info->li_low_price_range;

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Min. Lot Qty Is :%ld:",st_gen_info.l_min_lot_qty);
  	fn_userlog(c_ServiceName,"Board Lot Qty is :%ld:",st_gen_info.l_board_lot_qty);
  	fn_userlog(c_ServiceName,"Tick Size Is :%ld:",st_gen_info.l_tick_sz);
  	fn_userlog(c_ServiceName,"High Price Range Is :%ld:",st_gen_info.l_hgh_prc_rng);
  	fn_userlog(c_ServiceName,"Low Price Range Is :%ld:",st_gen_info.l_low_prc_rng);
  	fn_userlog(c_ServiceName,"Exchange Issue Start Date Is :%ld:",ptr_st_sec_upd_info->li_issue_start_date );
  	fn_userlog(c_ServiceName,"Exchange Interest Payment Date Is :%ld:",ptr_st_sec_upd_info->li_interest_payment_date );
  	fn_userlog(c_ServiceName,"Exchange Issue maturity Date Is :%ld:",ptr_st_sec_upd_info->li_issue_maturity_date );
  	fn_userlog(c_ServiceName,"Exchange Listing Date Is :%ld:",ptr_st_sec_upd_info->li_listing_date );
  	fn_userlog(c_ServiceName,"Exchange Expulsion Date Is :%ld:",ptr_st_sec_upd_info->li_expulsion_date );
  	fn_userlog(c_ServiceName,"Exchange Readmission Date Is :%ld:",ptr_st_sec_upd_info->li_readmission_date );
  	fn_userlog(c_ServiceName,"Exchange Record Date Is :%ld:",ptr_st_sec_upd_info->li_record_date );
  	fn_userlog(c_ServiceName,"Exchange ND Start Date Is :%ld:",ptr_st_sec_upd_info->li_nd_start_date );
  	fn_userlog(c_ServiceName,"Exchange ND End Date Is :%ld:",ptr_st_sec_upd_info->li_nd_end_date );
  	fn_userlog(c_ServiceName,"Exchange Book Closure Start Date :%ld:",ptr_st_sec_upd_info->li_book_closure_start_date );
  	fn_userlog(c_ServiceName,"Exchange Book Closure End Date :%ld:",ptr_st_sec_upd_info->li_book_closure_end_date );
  	fn_userlog(c_ServiceName,"Exchange Exercise Start Date Is :%ld:",ptr_st_sec_upd_info->li_exercise_start_date );
  	fn_userlog(c_ServiceName,"Exchange Exercise End Date Is :%ld:",ptr_st_sec_upd_info->li_exercise_end_date );
  	fn_userlog(c_ServiceName,"Exchange Local Update Date Time Is :%ld:",ptr_st_sec_upd_info->li_local_update_date_time );
 	} 
	
	fn_long_to_timearr ( st_cnt_info.c_issue_strt_dt,ptr_st_sec_upd_info->li_issue_start_date );
 	fn_long_to_timearr ( st_cnt_info.c_issue_int_pymt_dt,ptr_st_sec_upd_info->li_interest_payment_date );
 	fn_long_to_timearr ( st_cnt_info.c_issue_mat_dt,ptr_st_sec_upd_info->li_issue_maturity_date );
 	fn_long_to_timearr ( st_cnt_info.c_list_dt,ptr_st_sec_upd_info->li_listing_date );
 	fn_long_to_timearr ( st_cnt_info.c_exp_dt, ptr_st_sec_upd_info->li_expulsion_date );
 	fn_long_to_timearr ( st_cnt_info.c_read_dt,ptr_st_sec_upd_info->li_readmission_date );
 	fn_long_to_timearr ( st_cnt_info.c_rec_dt,ptr_st_sec_upd_info->li_record_date );
 	fn_long_to_timearr ( st_cnt_info.c_nd_strt_dt,ptr_st_sec_upd_info->li_nd_start_date );
 	fn_long_to_timearr ( st_cnt_info.c_nd_end_dt,ptr_st_sec_upd_info->li_nd_end_date );
 	fn_long_to_timearr ( st_cnt_info.c_bc_strt_dt,ptr_st_sec_upd_info->li_book_closure_start_date );
 	fn_long_to_timearr ( st_cnt_info.c_bc_end_dt,ptr_st_sec_upd_info->li_book_closure_end_date );
 	fn_long_to_timearr ( st_cnt_info.c_ex_strt_dt,ptr_st_sec_upd_info->li_exercise_start_date );
 	fn_long_to_timearr ( st_cnt_info.c_ex_end_dt,ptr_st_sec_upd_info->li_exercise_end_date );
 	fn_long_to_timearr ( st_cnt_info.c_tm,ptr_st_sec_upd_info->li_local_update_date_time );

	if(DEBUG_MSG_LVL_3)
	{
 		fn_userlog(c_ServiceName,"Issue Start Date Is :%s:",st_cnt_info.c_issue_strt_dt);
 		fn_userlog(c_ServiceName,"Interest Payment Date Is :%s:",st_cnt_info.c_issue_int_pymt_dt);
 		fn_userlog(c_ServiceName,"Issue maturity Date Is :%s:",st_cnt_info.c_issue_mat_dt);
 		fn_userlog(c_ServiceName,"Listing Date Is :%s:",st_cnt_info.c_list_dt);
 		fn_userlog(c_ServiceName,"Expulsion Date Is :%s:",st_cnt_info.c_exp_dt);
 		fn_userlog(c_ServiceName,"Readmission Date Is :%s:",st_cnt_info.c_read_dt);
 		fn_userlog(c_ServiceName,"Record Date Is :%s:",st_cnt_info.c_rec_dt);
 		fn_userlog(c_ServiceName,"ND Start Date Is :%s:",st_cnt_info.c_nd_strt_dt);
 		fn_userlog(c_ServiceName,"ND End Date Is :%s:",st_cnt_info.c_nd_end_dt);
 		fn_userlog(c_ServiceName,"Book Closure Start Date Is :%s:",st_cnt_info.c_bc_strt_dt);
 		fn_userlog(c_ServiceName,"Book Closure End Date Is :%s:",st_cnt_info.c_bc_end_dt);
 		fn_userlog(c_ServiceName,"Exercise Start Date Is :%s:",st_cnt_info.c_ex_strt_dt);
  	fn_userlog(c_ServiceName,"Exercise End Date Is :%s:",st_cnt_info.c_ex_end_dt);
 		fn_userlog(c_ServiceName,"Local Update Date Time Is :%s:",st_cnt_info.c_tm);
	}

 	st_cnt_info.c_rec_dt[11] = '\0';
 	strcpy( st_gen_info.c_record_dt ,st_cnt_info.c_rec_dt);
 	st_cnt_info.c_nd_strt_dt[11] = '\0';
 	strcpy( st_gen_info.c_nd_strt_dt ,st_cnt_info.c_nd_strt_dt);
 	st_cnt_info.c_nd_end_dt[11] = '\0';
 	strcpy( st_gen_info.c_nd_end_dt ,st_cnt_info.c_nd_end_dt);
 	st_cnt_info.c_bc_strt_dt[11] = '\0';
 	strcpy( st_gen_info.c_bk_cls_strt_dt,st_cnt_info.c_bc_strt_dt);
 	st_cnt_info.c_bc_end_dt[11] = '\0';
 	strcpy( st_gen_info.c_bk_cls_end_dt ,st_cnt_info.c_bc_end_dt);

	fn_cpy_ddr(st_gen_info.c_rout_crt);

	if(DEBUG_MSG_LVL_3)
	{
 		fn_userlog(c_ServiceName,"Before Call To SFO_UPDGEN_INF With Request Type UPDATE_GEN_INFO");
	}

		if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
    {
      fn_gettime_em();
      fn_userlog(c_ServiceName,"TIME_STATS NDF Before SFO_UPDGEN_INF at :%s:",c_time);
    }

 	st_gen_info.c_rqst_typ = UPDATE_GEN_INFO;

 	/*  call service routine  */
 	i_ch_val = fn_acall_svc (  c_ServiceName,
  	                         c_errmsg,
    	                       &st_gen_info,
      	                     "vw_cntrt_gen_inf",
        	                   sizeof (st_gen_info),
          	                 TPNOREPLY,
            	               "SFO_UPDGEN_INF" );
	
 	if ( i_ch_val == SYSTEM_ERROR )
 	{
  	fn_errlog ( c_ServiceName, "S31050", LIBMSG, c_errmsg );
   	return -1;
 	}

	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF after SFO_UPDGEN_INF at :%s:",c_time);
  }

	 return 0;

}

/******************************************************************************/
/*  To update details of participant status                                   */
/*  INPUT PARAMETERS                                                          */
/*      st_partstts  - Participant status structure                           */
/*      c_errmsg    - Error message to be returned in case of error           */
/*  OUTPUT PARAMETERS                                                         */
/*      void                                                                  */
/******************************************************************************/

int fn_part_mstr_chg( struct st_participant_update_info *ptr_part_upd_inf,
                      char *c_xchng_cd,
                      char *c_ServiceName,
                      char *c_errmsg)
{
  struct vw_xchngstts st_stts;
  int i_ch_val;

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Inside Function fn_part_mstr_chg");
  	fn_userlog(c_ServiceName,"Exchange code is :%s:",c_xchng_cd);
	}

	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF inside  fn_part_mstr_chg at :%s:",c_time);
  }

  fn_nsetoors_char (st_stts.c_settlor,
                    LEN_PARTI_ID,
                    ptr_part_upd_inf->c_participant_id,
                    LEN_SETTLOR);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Participent ID Is :%s:",st_stts.c_settlor);
	}

  st_stts.c_settlor[5] = '\0';

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"After Null Addition Participent ID Is :%s:",st_stts.c_settlor);
	}

  if( ptr_part_upd_inf->c_delete_flag == 'Y' )
  {
    st_stts.c_settlor_stts = SUSPEND;

		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Settoler Status Is :%c:",st_stts.c_settlor_stts);
		}
  }
  else
  {
    st_stts.c_settlor_stts =  ptr_part_upd_inf->c_participant_status;

		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Settoler Status Is :%c:",st_stts.c_settlor_stts);
		}
  }

  strcpy(st_stts.c_xchng_cd,c_xchng_cd);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Exchnage Code Is :%s:",st_stts.c_xchng_cd);
	}

	if(DEBUG_MSG_LVL_2)  /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF Before  SFO_UPD_XSTTS at :%s:",c_time);
  }

  fn_cpy_ddr(st_stts.c_rout_crt);

  st_stts.c_rqst_typ = UPD_PARTICIPANT_STTS;

	i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_errmsg,
                            &st_stts,
                            "vw_xchngstts",
                            sizeof (struct vw_xchngstts ),
                            TPNOREPLY,
                            "SFO_UPD_XSTTS" );
  if ( i_ch_val == SYSTEM_ERROR )
  {
    fn_errlog ( c_ServiceName, "S31055", LIBMSG, c_errmsg );
    return -1;
  }
	
	if(DEBUG_MSG_LVL_2)  /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF after  SFO_UPD_XSTTS at :%s:",c_time);
  }

  return 0;
}
	
/******************************************************************************/
/*  To update details of either trader message or general message based on    */
/*  message id as input parameter.                                            */
/*  INPUT PARAMETERS                                                          */
/*      st_temp_msg  - Trader / Geberal message structure                     */
/*      i_msg_id     - Type of message (TRADER_MSG / GENERAL_MSG)             */
/*      c_errmsg    - Error message to be returned in case of error           */
/*  OUTPUT PARAMETERS                                                         */
/*      void                                                                  */
/******************************************************************************/

int fn_gnrltrd_msg( struct st_bcast_message *ptr_nse_bcast_msg,
                    char *c_xchng_cd,
                    char *c_pipe_id,
                    char *c_ServiceName,
                    char *c_errmsg)
{
  struct vw_mkt_msg st_msg;

  int i_ch_val;
  int i_trnsctn;
	int iLen;             /*** Added in ver 1.3 ***/
  char *cPtr;           /*** Added in ver 1.3 ***/
  char c_undrlyng[20];  /*** Added in ver 1.3 ***/
  char c_stock_cd[7];   /*** Added in ver 1.3 ***/
  char c_trd_msg[300];
  char c_brk_stts= 'A'; /*** Initialized in ver 1.3 Initializd to 'A' in ver 1.4 ***/
  char c_closeout_flag = 'N'; /*** ver 1.4 ***/
	char c_message_typ   = 'S'; /*** ver 1.4 ***/
  char c_und_brkr_stts = 'N'; /*** ver 1.4 ***/
  char c_raise_trigger_type [30];


	MEMSET(st_msg.c_brkr_id);

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Inside Function fn_gnrltrd_msg");
  	fn_userlog(c_ServiceName,"Exchange code is :%s:",c_xchng_cd);
  	fn_userlog(c_ServiceName,"Exchange Time Stamp Is ;%ld:",ptr_nse_bcast_msg->st_hdr.li_log_time);
/* fn_userlog(c_ServiceName,"Exchng lib THE MSG is :%s:",st_msg.c_msg); * Ver 1.3 *Moved to Debug LVL 4 in Ver 1.8***/
	}

/************ Debug level Change from 0 to 4 Ver 1.8 ************/
if(DEBUG_MSG_LVL_4)
  {
    fn_userlog(c_ServiceName,"Exchng lib THE MSG is :%s:",st_msg.c_msg);
  }
/************ Ver 1.8 Ended ************/

	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF inside fn_gnrltrd_msg at :%s:",c_time);
  }

  fn_long_to_timearr( st_msg.c_tm_stmp,ptr_nse_bcast_msg->st_hdr.li_log_time);

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"After Conversion Into EBA Format Time Stamp Is :%s:",st_msg.c_tm_stmp);
	}

  fn_nsetoors_char( st_msg.c_msg,
                    LEN_MSG,
                    ptr_nse_bcast_msg->c_broadcast_message,
                    LEN_BROADCAST_MESSAGE);

  sprintf( c_trd_msg, "|%s| %s|",st_msg.c_tm_stmp,st_msg.c_msg);

  fn_pst_trg ( c_ServiceName, "TRG_ORS_CON", c_trd_msg,c_pipe_id );

	if(DEBUG_MSG_LVL_3)
	{
/***  	fn_userlog(c_ServiceName,"THE MSG is :%s:",st_msg.c_msg);	*** Moved to Debug LVL 4 in Ver 1.8 ******/
  	fn_userlog(c_ServiceName,"TRADER FLAG IS :%d:",ptr_nse_bcast_msg->st_bcast_dest.flg_trdr_ws);
  	fn_userlog(c_ServiceName,"CONTROL FLAG IS :%d:",ptr_nse_bcast_msg->st_bcast_dest.flg_cntrl_ws);
	}

/************ Debug level Change from 0 to 4 Ver 1.8 ************/
if(DEBUG_MSG_LVL_4)
  {
    fn_userlog(c_ServiceName,"Exchng lib THE MSG is :%s:",st_msg.c_msg);
  }
/************ Ver 1.8 Ended ************/

  if(ptr_nse_bcast_msg->st_bcast_dest.flg_trdr_ws == 1)
  {
    st_msg.c_msg_id = GENERAL_MSG;
		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Message ID is :%c:",st_msg.c_msg_id);
		}
  }
	else if (ptr_nse_bcast_msg->st_bcast_dest.flg_cntrl_ws == 1)
  {
    st_msg.c_msg_id = TRADER_MSG;
		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Message ID is :%c:",st_msg.c_msg_id);
		}
    fn_nsetoors_char (st_msg.c_brkr_id,
                      LEN_TRDR_ID,
                      ptr_nse_bcast_msg->c_broker_number,
                      LEN_BROKER_ID);

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Broker ID Is :%s:",st_msg.c_brkr_id);
		}
  }
  else
  {
    fn_userlog(c_ServiceName,"Invalid Message Type");
    return 0;
  }

  strcpy( st_msg.c_xchng_cd,c_xchng_cd);
	
	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_msg.c_xchng_cd);
  	fn_userlog(c_ServiceName,"Broker ID Is :%s:",st_msg.c_brkr_id);
/****  	fn_userlog(c_ServiceName,"Message Is :%s:",st_msg.c_msg);	** Moved to Debug LVL 4 in Ver 1.8 ******/
  	fn_userlog(c_ServiceName,"Time Stamp  Is :%s:",st_msg.c_tm_stmp);
  	fn_userlog(c_ServiceName,"Message ID Is :%c:",st_msg.c_msg_id);
  	fn_userlog(c_ServiceName,"Before Call to SFO_UPD_MSG");
	}

/************ Debug level Change from 0 to 4 Ver 1.8 ************/
		if(DEBUG_MSG_LVL_4)
  	{
    	fn_userlog(c_ServiceName,"Exchng lib THE MSG is :%s:",st_msg.c_msg);
  	}
/************ Ver 1.8 Ended ************/


	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF Before SFO_UPD_MSG at :%s:",c_time);
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
    fn_errlog ( c_ServiceName, "S31005", LIBMSG, c_errmsg );
    return -1;
  }
	
	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF after SFO_UPD_MSG at :%s:",c_time);
  }

	if(strstr(st_msg.c_msg,"closeout"))
  {
     c_brk_stts = 'C';

		 if(DEBUG_MSG_LVL_0)
		 {
     		fn_userlog(c_ServiceName,"Broker Status Is :%c:",c_brk_stts);
		 }
		
		 /*** Addded in ver 1.3 ***/

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

		 /************** 1.4 Starts ***************/

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
      /************** 1.4 Ends   ***************/	
     EXEC SQL
        SELECT  SEM_STCK_CD
        INTO    :c_stock_cd
        FROM    SEM_STCK_MAP
        WHERE   SEM_MAP_VL = :c_undrlyng
        AND     SEM_ENTTY   = 3;

      if ( SQLCODE  !=  0 )
      {
        fn_errlog ( c_ServiceName, "S31015", SQLMSG, c_errmsg );
        return -1;
      }

     if(DEBUG_MSG_LVL_0)
     {
        fn_userlog(c_ServiceName,"The Stock code is :%s:",c_stock_cd );
     }
		}
     /*** Ver 1.3 Ended ***/	
		if (strstr (st_msg.c_msg,"not") != NULL)   /*** Ver 1.4 Added for handeling of activation message ****/
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

		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Broker Status Is :%c:",c_brk_stts);
		}
  }
	/************ Ver 1.4 Starts ***********/
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
       fn_errlog ( c_ServiceName, "S31095", SQLMSG, c_errmsg );
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
		
		 if (strstr (st_msg.c_msg,"below") != NULL)   /*** Ver 1.4 Added for handeling of activation message ****/
     {
      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"Activation message for RRM");
      }
      c_message_typ = 'A';
     }	
  }
  /************ Ver 1.4 Ends   ***********/
  else
  {
    return 0;
  }


  i_trnsctn = fn_begintran( c_ServiceName, c_errmsg );
  if ( i_trnsctn == -1 )
  {
    fn_errlog ( c_ServiceName, "S31010", LIBMSG, c_errmsg );
    return -1;
  }

	if (c_closeout_flag != 'C' && c_closeout_flag != 'R')     /*** Condition added in ver 1.4 ***/
  {

		if ( c_message_typ == 'A' && c_brk_stts == 'C' ) /*** Ver 1.4 reactivation message handeling ***/
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
        fn_errlog ( c_ServiceName, "S31105", SQLMSG, c_errmsg );
        fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
        return -1;
      }
    }
    else                                              /*** End of ver 1.4 ***/
    {
  		EXEC SQL
    	UPDATE  EXG_XCHNG_MSTR
    	SET     EXG_BRKR_STTS    = :c_brk_stts
    	WHERE   EXG_XCHNG_CD     = :c_xchng_cd;

  		if ( SQLCODE != 0 )
  		{
    		fn_errlog ( c_ServiceName, "S31015", SQLMSG, c_errmsg );
    		fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
    		return -1;
  		}
		}
  }
	/*** Ver 1.3 started ***/

  if ( c_brk_stts == 'C')
  {
		/************** Ver 1.4 Starts ***********/
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
            SET  EXG_CLS_STTS = :c_closeout_flag 
            WHERE EXG_XCHNG_CD = :c_xchng_cd;

        if ( SQLCODE  !=  0 )
        {
            fn_errlog ( c_ServiceName, "S31015", SQLMSG, c_errmsg );
            fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
            return -1;
        }
    }
    /************** Ver 1.4 Ends ***********/
    else
    {

    if(DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName,"Broker Status closed updating underlying level status :%s:",c_stock_cd);
      fn_userlog(c_ServiceName,"Exchange Code is :%s:",c_xchng_cd);
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
        SET    FUM_BRKR_CLSOUT_FLG = :c_und_brkr_stts
        WHERE  FUM_UNDRLYNG =:c_stock_cd
        AND    FUM_XCHNG_CD =:c_xchng_cd;

    if ( SQLCODE  !=  0 )
    {
      fn_errlog ( c_ServiceName, "S31015", SQLMSG, c_errmsg );
      fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
      return -1;
    }
	 }
  }

	/*********  Ver 1.4 Starts  ********/

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
            SET  EXG_CLS_STTS = :c_closeout_flag
            WHERE EXG_XCHNG_CD = :c_xchng_cd;

        if ( SQLCODE  !=  0 )
        {
            fn_errlog ( c_ServiceName, "S31015", SQLMSG, c_errmsg );
            fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
            return -1;
        }
  }

  /**********   Ver 1.4 Ends   ********/
  /*** Ver 1.3 Ended ***/

	 if ( fn_committran( c_ServiceName, i_trnsctn, c_errmsg ) == -1 )
  {
    fn_errlog ( c_ServiceName, "S31020", LIBMSG, c_errmsg );
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

/******************************************************************************/
/*  To update detail of broker suspension because of broker limit exceed      */
/*  INPUT PARAMETERS                                                          */
/*      st_brklmtexcd  - Broker limit exceede structure                       */
/*      c_errmsg      - Error message to be returned in case of error         */
/*  OUTPUT PARAMETERS                                                         */
/*      void                                                                  */
/******************************************************************************/

int fn_turnover_exceeded( struct st_broadcast_tlimit_exceeded *st_brd_tlmt_exceed,
                          int i_msg_typ,
                          char *c_xchng_cd,
                          char *c_pipe_id,
                          char *c_ServiceName,
                          char *c_errmsg )
{
  char c_msg[256];
  char c_tmp_msg[256];
  char c_brk_stts;
  char c_trd_msg[300];
  char c_tm_stmp[ 20 + 1 ];

  int i_ch_val;
  int i_ret_val;

  struct vw_xchngstts st_stts;
  struct vw_mkt_msg st_msg;
  struct st_trd_msg  st_trdr_msg;
	struct st_bcast_message st_bcast_msg;

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Inside Function fn_turnover_exceeded");
  	fn_userlog(c_ServiceName,"Exchange code is :%s:",c_xchng_cd);
  	fn_userlog(c_ServiceName,"Message Type is :%d:",i_msg_typ);
	}

	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF inside fn_turnover_exceeded at :%s:",c_time);
  }
	
  if( (i_msg_typ == BCAST_TURNOVER_EXCEEDED) && ( st_brd_tlmt_exceed->si_warning_type == LIMIT_ABOUT_TO_EXCEED) )
  {
		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Inside Condition BCAST_TURNOVER_EXCEEDED && LIMIT_ABOUT_TO_EXCEED");
		}
		strcpy(st_bcast_msg.c_broadcast_message," Turnover limit is about to Exceed ");
		c_brk_stts = 'C';
  }
  else if( (i_msg_typ == BCAST_TURNOVER_EXCEEDED) && ( st_brd_tlmt_exceed->si_warning_type == LIMIT_EXCEEDED) )
  {
		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Inside Condition BCAST_TURNOVER_EXCEEDED && LIMIT_EXCEEDED");
		}
    c_brk_stts = 'S';
    strcpy(st_bcast_msg.c_broadcast_message,"Turnover limit is Exceeded");
  }
  else if (i_msg_typ == BROADCAST_BROKER_REACTIVATED)
  {
		if(DEBUG_MSG_LVL_0)
		{
    	fn_userlog(c_ServiceName,"Inside Condition Broker Is Reactivated");
		}
    strcpy(st_bcast_msg.c_broadcast_message," Broker is reactivated ");
    c_brk_stts = 'A';
  }

	st_bcast_msg.st_hdr.li_log_time = st_brd_tlmt_exceed->st_hdr.li_log_time;
  strcpy(st_bcast_msg.c_broker_number,st_brd_tlmt_exceed->c_broker_code);
	st_bcast_msg.st_bcast_dest.flg_cntrl_ws=1;

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Exchange Msg Is :%s:",st_bcast_msg.c_broadcast_message);
  	fn_userlog(c_ServiceName,"Exchange Log Time Is :%ld:",st_bcast_msg.st_hdr.li_log_time);
  	fn_userlog(c_ServiceName,"Trade Flag  Is :%d:",st_bcast_msg.st_bcast_dest.flg_trdr_ws);
  	fn_userlog(c_ServiceName,"Broker Code Is :%s:",st_bcast_msg.c_broker_number);
  	fn_userlog(c_ServiceName,"Before Call to Function fn_gnrltrd_msg");
	}
	
	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF before fn_gnrltrd_msg at :%s:",c_time);
  }

  i_ret_val = fn_gnrltrd_msg( &st_bcast_msg,
                              c_xchng_cd,
                              c_pipe_id,
                              c_ServiceName,
                              c_errmsg);

  if(i_ret_val == -1)
  {
    fn_errlog(c_ServiceName,"L31005", LIBMSG, c_errmsg);
    return -1;
  }

	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF after fn_gnrltrd_msg at :%s:",c_time);
  }

  if(( (i_msg_typ == BCAST_TURNOVER_EXCEEDED) && ( st_brd_tlmt_exceed->si_warning_type == 2 ) ) || (i_msg_typ == BROADCAST_BROKER_REACTIVATED) )
  {
    fn_nsetoors_char (st_stts.c_brkr_id,
                      LEN_BRKR_ID,
                      st_brd_tlmt_exceed->c_broker_code,
                      LEN_BROKER_ID);
    st_stts.c_brkr_stts = c_brk_stts;
		st_stts.c_rqst_typ	=	UPD_BRK_LMT_EXCD;
    strcpy( st_stts.c_xchng_cd,c_xchng_cd);

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Broker ID Is :%s:",st_stts.c_brkr_id);
    	fn_userlog(c_ServiceName,"Broker Status Is :%c:",st_stts.c_brkr_stts);
    	fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_stts.c_xchng_cd);
			fn_userlog(c_ServiceName,"Request Type Is :%c:",st_stts.c_rqst_typ);
		}

    fn_cpy_ddr(st_stts.c_rout_crt);

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Before Call to SFO_UPD_XSTTS With Request Type UPD_BRK_LMT_EXCD");
		}

		if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  	{
    	fn_gettime_em();
    	fn_userlog(c_ServiceName,"TIME_STATS NDF before SFO_UPD_XSTTS at :%s:",c_time);
  	}

		 i_ch_val = fn_acall_svc ( c_ServiceName,
                              c_errmsg,
                              &st_stts,
                              "vw_xchngstts",
                              sizeof (st_stts ),
                              TPNOREPLY,
                              "SFO_UPD_XSTTS" );

    if ( i_ch_val == SYSTEM_ERROR )
    {
      fn_errlog ( c_ServiceName, "S31075", LIBMSG, c_errmsg );
      return -1;
    }
		
		if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
    {
      fn_gettime_em();
      fn_userlog(c_ServiceName,"TIME_STATS NDF after SFO_UPD_XSTTS at :%s:",c_time);
    }
  }

  return 0;
}

int fn_brdmkt_stts_chng(struct st_bcast_vct_msgs *st_brd_vct_msg,
          	            char *c_xchng_cd,
                        char *c_pipe_id,
                        char *c_ServiceName,
                        char *c_errmsg )
{

  char c_tmp_alpha_char[3];

  int i_ret_val;
  long int li_mkt_type;

	union st_exch_brd_msg *ptr_exch_msg;

	struct st_bcast_message st_bcast_msg;

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Inside Function fn_brdmkt_stts_chng");
  	fn_userlog(c_ServiceName,"Exchange code is :%s:",c_xchng_cd);
  	fn_userlog(c_ServiceName,"Transaction Code Is :%d:",st_brd_vct_msg->st_hdr.si_trans_code);
	}

	if(DEBUG_MSG_LVL_2)
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF inside fn_brdmkt_stts_chng at :%s:",c_time);
  }

  MEMSET(c_tmp_alpha_char);

  switch(st_brd_vct_msg->st_hdr.si_trans_code)
  {
    case BC_OPEN_MSG:
    case BC_CLOSE_MSG:

      fn_nsetoors_char (c_tmp_alpha_char,
                        3,
                        st_brd_vct_msg->st_hdr.c_alpha_char,
                        LEN_ALPHA_CHAR );


			if(DEBUG_MSG_LVL_3)
			{
      	fn_userlog(c_ServiceName,"BRD MESSAGE:st_brd_vct_msg->st_hdr.c_alpha_char:%s:",st_brd_vct_msg->st_hdr.c_alpha_char);
      	fn_userlog(c_ServiceName,"c_tmp_alpha_char Is :%s:",c_tmp_alpha_char);
			}

      rtrim(c_tmp_alpha_char);

      if ( strcmp(c_tmp_alpha_char, "EX") == 0)
      {
         li_mkt_type = EXERCISE_MKT;

				 if(DEBUG_MSG_LVL_3)
				 {
         	fn_userlog(c_ServiceName,"Market Type Is :%ld:",li_mkt_type);
				 }
      }
      else if ( strcmp(c_tmp_alpha_char, "PL") == 0)
			    {
        li_mkt_type = PL_MKT;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Type Is :%ld:",li_mkt_type);
				}
      }
      else if ( strcmp(c_tmp_alpha_char, "S2") == 0)
      {
        li_mkt_type = EXTND_MKT;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Type Is :%ld:",li_mkt_type);
				}
      }
      else if ( strcmp(c_tmp_alpha_char, "S1") == 0)
      {
        li_mkt_type = ORDER_MKT;
	
				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Type Is :%ld:",li_mkt_type);
				}
      }
      else
      {
        fn_userlog(c_ServiceName,"BRD MESSAGE NOT HANDLED:st_brd_vct_msg->st_hdr.c_alpha_char:%s:", st_brd_vct_msg->st_hdr.c_alpha_char);
        break;
      }

      if ( st_brd_vct_msg->st_hdr.si_trans_code == BC_OPEN_MSG )
      {
        si_mkt_stts = OPEN;

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Market Status Is :%d:",si_mkt_stts);
				}
      }
      else if ( st_brd_vct_msg->st_hdr.si_trans_code == BC_CLOSE_MSG)
      {
        if ( (li_mkt_type == ORDER_MKT) || (li_mkt_type == EXTND_MKT) )
        {
          si_mkt_stts = 'X';

					if(DEBUG_MSG_LVL_3)
					{
          	fn_userlog(c_ServiceName,"Market Status Is :%d:",si_mkt_stts);
					}
        }
        else
        {
          si_mkt_stts = CLOSED;

					if(DEBUG_MSG_LVL_3)
					{
          	fn_userlog(c_ServiceName,"Market Status Is :%d:",si_mkt_stts);
					}
        }
      }
      else
      {
        fn_errlog(c_ServiceName,"L31010", "Invalid trans code", c_errmsg);
        break;
      }

			if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
      {
        fn_gettime_em();
        fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_fomrkt_stts_hndlr at :%s:",c_time);
      }

			i_ret_val = fn_fomrkt_stts_hndlr ( si_mkt_stts,
                                         li_mkt_type,
                                         c_xchng_cd,
                                         c_ServiceName,
                                         c_errmsg);
      if (i_ret_val == -1)
      {
        fn_errlog(c_ServiceName,"L31015", LIBMSG, c_errmsg);
        return -1;
      }

			if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
      {
        fn_gettime_em();
        fn_userlog(c_ServiceName,"TIME_STATS NDF after fn_fomrkt_stts_hndlr at :%s:",c_time);
      }

      break;
  }

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Before Call To fn_gnrltrd_msg");
	}

  fn_nsetoors_char (st_bcast_msg.c_broadcast_message,
                    LEN_MSG,
                    st_brd_vct_msg->c_broadcast_message,
                    st_brd_vct_msg->si_broadcast_message_length );

  st_bcast_msg.st_hdr.li_log_time = st_brd_vct_msg->st_hdr.li_log_time;
  st_bcast_msg.st_bcast_dest.flg_trdr_ws = 1;

	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_gnrltrd_msg at :%s:",c_time);
  }

  i_ret_val = fn_gnrltrd_msg( &st_bcast_msg,
                              c_xchng_cd,
                              c_pipe_id,
                              c_ServiceName,
                              c_errmsg);

  if(i_ret_val == -1)
  {
    fn_errlog(c_ServiceName,"L31020", LIBMSG, c_errmsg);
    return -1;
  }
	
	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF after fn_gnrltrd_msg at :%s:",c_time);
  }

  return 0;
}


int fn_fomrkt_stts_hndlr (short int si_mkt_stts,
                          long int  li_mkt_type,
                          char  *c_xchng_cd,
                          char  *c_ServiceName,
                          char  *c_errmsg)
{
  int i_ch_val;
  int i_ip_len; /** ver 1.7 ***/
  int i_op_len; /** ver 1.7 ***/

  char c_mkt_stts;
  char c_mkt_typ;

  struct vw_xchngstts st_stts;

  strcpy( st_stts.c_xchng_cd,c_xchng_cd);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Inside Function fn_fomrkt_stts_hndlr");
  	fn_userlog(c_ServiceName,"Exchange code is :%s:",c_xchng_cd);
  	fn_userlog(c_ServiceName,"Market Type Is :%ld:",li_mkt_type);
  	fn_userlog(c_ServiceName,"Market status Is :%d:",si_mkt_stts);
	}

  if( li_mkt_type != PL_MKT )
  {
    switch(si_mkt_stts)
    {
      case PRE_OPEN :
      case PRE_OPEN_ENDED :
      case POST_CLOSE :
        return 0;
        break;

      case OPEN :

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside OPEN case");
				}

        c_mkt_stts = 'O';
        break;

      case CLOSED :

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside CLOSED case");
				}

        c_mkt_stts = 'C';
        break;

      case 'X' :

				if(DEBUG_MSG_LVL_3)
				{
					fn_userlog(c_ServiceName,"Inside X case");
				}

        c_mkt_stts = 'X';
        break;
    }

    switch (li_mkt_type)
    {
      case ORDER_MKT :

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside ORDER_MKT case");
				}
        c_mkt_typ= ORDER_MARKET;
        break;

      case EXTND_MKT :

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside EXTND_MARKET case");
				}

        c_mkt_typ= EXTND_MARKET;
        break;

      case EXERCISE_MKT :

				if(DEBUG_MSG_LVL_3)
				{
        	fn_userlog(c_ServiceName,"Inside EXERCISE_MKT case");
				}
        c_mkt_typ = EXER_MARKET;
        break;

      case PL_MKT :

        c_mkt_typ = PL_MARKET;
        break;
    }

  }


  if( c_mkt_typ == EXERCISE_SEGMENT )
  {
		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Inside check EXERCISE_SEGMENT");
		}
    st_stts.c_exrc_mkt_stts = c_mkt_stts;

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Exercise Market Status Is :%c:",st_stts.c_exrc_mkt_stts);
		}
    st_stts.c_rqst_typ = UPD_EXER_MKT_STTS;
  }
  else if( c_mkt_typ == EXTENDED_SEGMENT )
  {
		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Inside check EXTENDED_SEGMENT");
		}
    st_stts.c_crrnt_stts = c_mkt_stts;
		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Market Status Is :%c:",st_stts.c_crrnt_stts);
		}
    st_stts.c_rqst_typ = UPD_EXTND_MKT_STTS;
	}

	else
  {
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Inside check NORMAL_SEGMENT");
		}
    st_stts.c_crrnt_stts = c_mkt_stts;

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Market Status Is :%c:",st_stts.c_crrnt_stts);
		}
    st_stts.c_rqst_typ = UPD_NORMAL_MKT_STTS;
  }

  fn_cpy_ddr(st_stts.c_rout_crt);

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Before Call To SFO_UPD_XSTTS");
  	fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_stts.c_xchng_cd);
	}

  /******* Commented in Ver 1.7 ***********
  i_ch_val = fn_acall_svc ( c_ServiceName,
                            c_errmsg,
                            &st_stts,
                            "vw_xchngstts",
                            sizeof (st_stts ),
                            TPNOREPLY,
                            "SFO_UPD_XSTTS" );
  *********** Commmented in Ver 1.7 ***********/

  /*********** Added in Ver 1.7 ***********/

   i_ip_len = sizeof (struct vw_xchngstts ) ;
   i_op_len = sizeof (struct vw_xchngstts ) ;

  i_ch_val = fn_call_svc ( c_ServiceName,
                            c_errmsg,
                            &st_stts,
                            &st_stts,
                            "vw_xchngstts",
                            "vw_xchngstts",
                            i_ip_len,
                            i_op_len,
                            0,
                            "SFO_UPD_XSTTS" );
  /*********** Added in Ver 1.7 ***********/
  if ( i_ch_val == SYSTEM_ERROR )
  {
    fn_errlog ( c_ServiceName, "S31080", LIBMSG, c_errmsg );
    return -1;
  }

  return 0;
}


/******************************************************************************/
/*  To convert NSE contract information to EBA contract information           */
/*  INPUT PARAMETERS                                                          */
/*      st_nse_cntrct  - NSE contract structure                               */
/*      ptr_cntrct     - EBA contract structure to be returned                */
/*      c_errmsg      - Error message to be returned in case of error         */
/*  OUTPUT PARAMETERS                                                         */
/*      void                                                                  */
/******************************************************************************/
int fn_nse_to_eba_cntrct ( nfo_cntrct st_nse_cntrct,
                           struct vw_contract *ptr_cntrct,
                           char *c_ServiceName,
                           char *c_errmsg )
{
  struct vw_nse_cntrct st_nse_dtls;
  int i_ch_val;
  int i_len;
  int i_count;

  strcpy( st_nse_dtls.c_xchng_cd,  st_nse_cntrct.c_xchng_cd);
  st_nse_dtls.c_prd_typ =  st_nse_cntrct.c_prd_typ;
  st_nse_cntrct.c_expry_dt[11] = '\0';
  strcpy( st_nse_dtls.c_expry_dt,  st_nse_cntrct.c_expry_dt);
  st_nse_dtls.c_exrc_typ =  st_nse_cntrct.c_exrc_typ;
  st_nse_dtls.c_opt_typ =  st_nse_cntrct.c_opt_typ;
  st_nse_dtls.l_strike_prc =  st_nse_cntrct.l_strike_prc;
  st_nse_dtls.c_ctgry_indstk =  st_nse_cntrct.c_ctgry_indstk;
  strcpy( st_nse_dtls.c_symbol,  st_nse_cntrct.c_symbol );
  strcpy( st_nse_dtls.c_series , st_nse_cntrct.c_series );
  st_nse_dtls.l_ca_lvl =  st_nse_cntrct.l_ca_lvl;

	if(DEBUG_MSG_LVL_3)
	{
  	fn_userlog(c_ServiceName,"Inside fn_nse_to_eba_cntrct Function");
  	fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_nse_dtls.c_xchng_cd);
  	fn_userlog(c_ServiceName,"Product Type Is :%c:",st_nse_dtls.c_prd_typ);
  	fn_userlog(c_ServiceName,"Expiry Date Is :%s:",st_nse_dtls.c_expry_dt);
  	fn_userlog(c_ServiceName,"Exercise Type Is :%c:",st_nse_dtls.c_exrc_typ);
  	fn_userlog(c_ServiceName,"Option Type Is :%c:",st_nse_dtls.c_opt_typ);
  	fn_userlog(c_ServiceName,"Strike Price Is :%ld:",st_nse_dtls.l_strike_prc);
  	fn_userlog(c_ServiceName,"IND/STK Is :%c:",st_nse_dtls.c_ctgry_indstk);
  	fn_userlog(c_ServiceName,"Symbol Is :%s:",st_nse_dtls.c_symbol);
  	fn_userlog(c_ServiceName,"Series Is :%s:",st_nse_dtls.c_series);
  	fn_userlog(c_ServiceName,"CA LVL Is :%ld:",st_nse_dtls.l_ca_lvl);
  	fn_userlog(c_ServiceName,"Before Call To SFO_CNT_FOR_ENT With Request Type NSE_ID_TO_CONTRACT");
	}

  /*********** Ver 1.1 **************
  fn_cpy_ddr(st_nse_dtls.c_rout_crt);
  st_nse_dtls.c_rqst_typ = NSE_ID_TO_CONTRACT;
  i_ch_val = fn_call_svc ( c_ServiceName,
                           c_errmsg,
                           &st_nse_dtls,
                           ptr_cntrct,
                           "vw_nse_cntrct",
                           "vw_contract",
                           sizeof (st_nse_dtls),
                           sizeof (struct vw_contract),
                           0,
                           "SFO_CNT_FOR_ENT" );

  if ( i_ch_val != SUCC_BFR )
  {
    fn_errlog ( c_ServiceName, "S31240", LIBMSG, c_errmsg );
    fn_userlog ( c_ServiceName,
                 "Contract details |%s|%c|%s|%c|%c|%ld|%c|%s|%s|%ld|",
                 st_nse_cntrct.c_xchng_cd, st_nse_cntrct.c_prd_typ,
                 st_nse_cntrct.c_expry_dt, st_nse_cntrct.c_exrc_typ,
                 st_nse_cntrct.c_opt_typ, st_nse_cntrct.l_strike_prc,
                 st_nse_cntrct.c_ctgry_indstk, st_nse_cntrct.c_symbol,
                 st_nse_cntrct.c_series, st_nse_cntrct.l_ca_lvl );
    return i_ch_val;
  }
  ************* Ver 1.1 Ends Here *********/

  /*** Ver 1.1  Starts here ****/
  MEMSET(sql_sem_map_vl); 
  strcpy(sql_sem_map_vl.arr,st_nse_dtls.c_symbol);
	SETLEN(sql_sem_map_vl);

  i_len=strlen(sql_sem_map_vl.arr);

  for(i_count = 0; i_count < i_len; i_count++)
  {

   sql_sem_map_vl.arr[i_count] = toupper(sql_sem_map_vl.arr[i_count]);

  }
  
  MEMSET(sql_sem_stck_cd);
 
  EXEC SQL
          SELECT sem_stck_cd
          INTO   :sql_sem_stck_cd
          FROM   SEM_STCK_MAP
          WHERE  sem_entty=3
          AND    sem_map_vl=:sql_sem_map_vl;
  
  if (SQLCODE !=0)
  {
    if (SQLCODE == NO_DATA_FOUND)
    { 
     fn_errlog ( c_ServiceName, "S31015", SQLMSG, c_errmsg ); 
    }
    return -1; 
  }

  rtrim(sql_sem_stck_cd);

  if(st_nse_dtls.c_exrc_typ  == 'X')
  {
    st_nse_dtls.c_exrc_typ = 'E';
  }
  if( st_nse_dtls.c_opt_typ  == 'X')
  {
    st_nse_dtls.c_opt_typ = '*';
  }
  if (st_nse_dtls.l_strike_prc == -1)
  {
    st_nse_dtls.l_strike_prc = 0;
  }     

  /*** Commented in Ver 1.6 ***
  EXEC SQL
          SELECT  fcm_xchng_cd,
                  fcm_prdct_typ,
                  fcm_undrlyng,
                  to_char(fcm_expry_dt,'DD-Mon-YYYY'),
                  fcm_exer_typ,
                  fcm_opt_typ,
                  fcm_strk_prc,
                  fcm_ca_lvl,
                  fcm_indstk
          INTO    :ptr_cntrct->c_xchng_cd,
                  :ptr_cntrct->c_prd_typ,
                  :ptr_cntrct->c_undrlyng,
                  :ptr_cntrct->c_expry_dt,
                  :ptr_cntrct->c_exrc_typ,
                  :ptr_cntrct->c_opt_typ,
                  :ptr_cntrct->l_strike_prc,
                  :ptr_cntrct->l_ca_lvl,
                  :ptr_cntrct->c_ctgry_indstk
          FROM    fcm_fo_cntrct_mstr
          WHERE   fcm_xchng_cd  = :st_nse_dtls.c_xchng_cd
          AND     fcm_prdct_typ = :st_nse_dtls.c_prd_typ
          AND     fcm_undrlyng  = :sql_sem_stck_cd
          AND     fcm_expry_dt  = to_date(:st_nse_dtls.c_expry_dt,
                                  'DD-Mon-YYYY')
          AND     fcm_exer_typ  = :st_nse_dtls.c_exrc_typ
          AND     fcm_opt_typ   = :st_nse_dtls.c_opt_typ
          AND     fcm_strk_prc  = :st_nse_dtls.l_strike_prc;

  ******/
 
  /*** Ver 1.6 Starts ***/

        EXEC SQL
          SELECT  ftq_xchng_cd,
                  ftq_prdct_typ,
                  ftq_undrlyng,
                  to_char(ftq_expry_dt,'DD-Mon-YYYY'),
                  ftq_exer_typ,
                  ftq_opt_typ,
                  ftq_strk_prc,
                  ftq_ca_lvl,
                  ftq_indstk
          INTO    :ptr_cntrct->c_xchng_cd,
                  :ptr_cntrct->c_prd_typ,
                  :ptr_cntrct->c_undrlyng,
                  :ptr_cntrct->c_expry_dt,
                  :ptr_cntrct->c_exrc_typ,
                  :ptr_cntrct->c_opt_typ,
                  :ptr_cntrct->l_strike_prc,
                  :ptr_cntrct->l_ca_lvl,
                  :ptr_cntrct->c_ctgry_indstk
          FROM    ftq_fo_trd_qt     
          WHERE   ftq_xchng_cd  = :st_nse_dtls.c_xchng_cd
          AND     ftq_prdct_typ = :st_nse_dtls.c_prd_typ
          AND     ftq_undrlyng  = :sql_sem_stck_cd
          AND     ftq_expry_dt  = to_date(:st_nse_dtls.c_expry_dt,
                                  'DD-Mon-YYYY')
          AND     ftq_exer_typ  = :st_nse_dtls.c_exrc_typ
          AND     ftq_opt_typ   = :st_nse_dtls.c_opt_typ
          AND     ftq_strk_prc  = :st_nse_dtls.l_strike_prc; 

  /*** Ver 1.6 Ends   ***/ 
 
  if ( SQLCODE != 0 )
  { 
   fn_errlog ( c_ServiceName, "S31015", SQLMSG, c_errmsg );
   fn_userlog ( c_ServiceName,
                 "Contract details |%s|%c|%s|%c|%c|%ld|%c|%s|%s|%ld|",
                 st_nse_cntrct.c_xchng_cd, st_nse_cntrct.c_prd_typ,
                 st_nse_cntrct.c_expry_dt, st_nse_cntrct.c_exrc_typ,
                 st_nse_cntrct.c_opt_typ, st_nse_cntrct.l_strike_prc,
                 st_nse_cntrct.c_ctgry_indstk, st_nse_cntrct.c_symbol,
                 st_nse_cntrct.c_series, st_nse_cntrct.l_ca_lvl );
   return -1;
  }
  rtrim( ptr_cntrct->c_xchng_cd);
  rtrim( ptr_cntrct->c_undrlyng);
  rtrim( ptr_cntrct->c_expry_dt);

  /*** Ver 1.1 Ends Here ****/

  return 0;
}

/******************************************************************************/
/*  To update details of contract status which have got from exchange.        */
/*  INPUT PARAMETERS                                                          */
/*      st_cntrctstts  - contract status message structure                    */
/*      c_errmsg      - Error message to be returned in case of error         */
/*  OUTPUT PARAMETERS                                                         */
/*      void                                                                  */
/******************************************************************************/

int fn_stock_status_chg ( struct st_security_status_update_info *ptr_sec_stts_inf,
                          char *c_xchng_cd,
                          char *c_ServiceName,
                          char *c_errmsg)
{
	char c_rout_str[4];
  struct vw_contract st_cntrct;
  struct vw_cntrt_gen_inf st_cntrctgeninfo;
  nfo_cntrct st_nse_cntrct;


  int i_ch_val;
  int i_count;
  long l_token_id;
	
	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Inside function fn_stock_status_chg");
  	fn_userlog(c_ServiceName,"Exchange code is :%s:",c_xchng_cd);
	}

	if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF inside  fn_stock_status_chg at :%s:",c_time);
  }

	strcpy(c_rout_str,"000");

  for( i_count=0; i_count<ptr_sec_stts_inf->si_number_of_records; i_count++ )
  {

    if ( (strcmp ( c_xchng_cd ,"NFO" )) == 0 )
    {
      l_token_id =  ptr_sec_stts_inf->st_t_and_e[i_count].l_token;

			if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  		{
    		fn_gettime_em();
    		fn_userlog(c_ServiceName,"TIME_STATS NDF before  fn_nse_to_eba_tkn at :%s:",c_time);
  		}

      i_ch_val = fn_nse_to_eba_tkn ( l_token_id,
                                     &st_cntrct,
																		 c_rout_str,
                                     c_ServiceName,
                                     c_errmsg );
      if ( i_ch_val == SYSTEM_ERROR )
      {
        fn_errlog ( c_ServiceName, "S31060", LIBMSG, c_errmsg );
        return -1;
      }
      else if ( i_ch_val != 0 )
      {
        return 0;
      }
			
			if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
  		{
    		fn_gettime_em();
    		fn_userlog(c_ServiceName,"TIME_STATS NDF after  fn_nse_to_eba_tkn at :%s:",c_time);
  		}
    }
    else
    {
			  /*  BSE code to be added here   */
      fn_errlog ( c_ServiceName, "S31065","BSE code not handled", c_errmsg );
      return 0;
    }

    strcpy( st_cntrctgeninfo.c_xchng_cd ,c_xchng_cd);
    st_cntrctgeninfo.c_prd_typ= st_cntrct.c_prd_typ ;
    strcpy( st_cntrctgeninfo.c_undrlyng , st_cntrct.c_undrlyng ) ;
    strcpy ( st_cntrctgeninfo.c_expry_dt , st_cntrct.c_expry_dt) ;
    st_cntrctgeninfo.c_exrc_typ= st_cntrct.c_exrc_typ ;
    st_cntrctgeninfo.c_opt_typ= st_cntrct.c_opt_typ ;
    st_cntrctgeninfo.l_strike_prc = st_cntrct.l_strike_prc ;
    st_cntrctgeninfo.c_ctgry_indstk = st_cntrct.c_ctgry_indstk;
    st_cntrctgeninfo.l_ca_lvl = st_cntrct.l_ca_lvl;

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Exchange Code Is :%s:",st_cntrctgeninfo.c_xchng_cd);
    	fn_userlog(c_ServiceName,"Product Type Is :%c:",st_cntrctgeninfo.c_prd_typ);
    	fn_userlog(c_ServiceName,"Underlying Is :%s:",st_cntrctgeninfo.c_undrlyng);
    	fn_userlog(c_ServiceName,"Expiry Date Is :%s:",st_cntrctgeninfo.c_expry_dt);
    	fn_userlog(c_ServiceName,"Exercise Type Is :%c:",st_cntrctgeninfo.c_exrc_typ);
    	fn_userlog(c_ServiceName,"Option Type Is :%c:",st_cntrctgeninfo.c_opt_typ);
    	fn_userlog(c_ServiceName,"Strike Price is :%ld:",st_cntrctgeninfo.l_strike_prc);
    	fn_userlog(c_ServiceName,"Index/Stock :%c:",st_cntrctgeninfo.c_ctgry_indstk);
		}
    if( ptr_sec_stts_inf->st_t_and_e[i_count].st_sec_stts[0].si_stts  == SEC_OPEN )
    {
			if(DEBUG_MSG_LVL_0)
			{
      	fn_userlog(c_ServiceName,"Inside Condition Of SEC_OPEN");
			}
      st_cntrctgeninfo.l_stts = 1;
    }
    else if(ptr_sec_stts_inf->st_t_and_e[i_count].st_sec_stts[0].si_stts == SEC_SUSPENDED )
    {
			if(DEBUG_MSG_LVL_0)
			{
      	fn_userlog(c_ServiceName,"Inside Condition Of SEC_SUSPENDED");
			}
      st_cntrctgeninfo.l_stts = 0;
    }
		 else
    {
      continue;
    }

    fn_cpy_ddr(st_cntrctgeninfo.c_rout_crt);
    st_cntrctgeninfo.c_rqst_typ = UPDATE_STATUS ;

		if(DEBUG_MSG_LVL_3)
		{
    	fn_userlog(c_ServiceName,"Before Call To SFO_UPDGEN_INF");
		}

		if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
    {
      fn_gettime_em();
      fn_userlog(c_ServiceName,"TIME_STATS NDF before  SFO_UPDGEN_INF at :%s:",c_time);
    }

    i_ch_val = fn_acall_svc ( c_ServiceName,
                              c_errmsg,
                              &st_cntrctgeninfo,
                              "vw_cntrt_gen_inf",
                              sizeof (st_cntrctgeninfo),
                              TPNOREPLY,
                              "SFO_UPDGEN_INF" );

    if ( i_ch_val == SYSTEM_ERROR )
    {
      fn_errlog ( c_ServiceName, "S31070", LIBMSG, c_errmsg );
      return -1;
    }

		if(DEBUG_MSG_LVL_2) /*** Ver 1.2 ***/
    {
      fn_gettime_em();
      fn_userlog(c_ServiceName,"TIME_STATS NDF after  SFO_UPDGEN_INF at :%s:",c_time);
    }

  }

  return 0;
}

int fn_nse_to_eba_tkn ( long int li_tkn,
                        struct vw_contract *ptr_cntrct,
												char *c_rout_str,
                        char *c_ServiceName,
                        char *c_err_msg )
{
  struct vw_nse_cntrct st_nse_dtls;
  int i_ch_val;

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Inside Function fn_nse_to_eba_tkn");
	}

  st_nse_dtls.l_token_id =  li_tkn;

	if(DEBUG_MSG_LVL_0)
	{
  	fn_userlog(c_ServiceName,"Token ID Is :%ld:",st_nse_dtls.l_token_id);
	}

  fn_cpy_ddr(c_rout_str);

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
  if ( i_ch_val != SUCC_BFR )
  {
    fn_errlog ( c_ServiceName, "L31025", LIBMSG, c_err_msg );
    fn_userlog ( c_ServiceName, "Token id |%ld|", li_tkn );
    return i_ch_val;
  }

  return 0;
}

/**** Ver 1.2 Starts ***/
void fn_gettime_em()
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
/*** Ver 1.2 ends ***/

/*** Ver 1.9 Starts ***/
int fn_exch_exception( struct  st_brd_exch_exception *st_brd_exch_exception_msg,
                          char *c_xchng_cd,
                          char *c_pipe_id,
                          char *c_ServiceName,
                          char *c_errmsg )
{
  int i_ret_val;

  struct st_bcast_message st_bcast_msg;
	MEMSET(st_bcast_msg);
	long int li_mail_log_time = 0;
	char c_mail_time[30];

  if(DEBUG_MSG_LVL_0)
  {
    fn_userlog(c_ServiceName,"Inside Function fn_exch_exception");
    fn_userlog(c_ServiceName,"Exchange code is :%s:",c_xchng_cd);
  }

  if(DEBUG_MSG_LVL_2)
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF inside fn_exch_exception at :%s:",c_time);
  }

  if( st_brd_exch_exception_msg->status == 1)
  {
    if(DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName,"Inside Condition BCAST_CONT_MSG(Exch Exception)");
    }
    strcpy(st_bcast_msg.c_broadcast_message,"Exception Handling Start of Outage,");
  }
	else if ( st_brd_exch_exception_msg->status == 0 )
	{
    if(DEBUG_MSG_LVL_0)
    {
      fn_userlog(c_ServiceName,"Inside Condition BCAST_CONT_MSG(Exch Exception)");
    }
    strcpy(st_bcast_msg.c_broadcast_message,"Exception Handling End of Outage,");
  }
	else
	{
		fn_userlog(c_ServiceName,"Inside Condition BCAST_CONT_MSG(Exch Exception) INVALID STATUS");
		return -1;
	}

	if( (st_brd_exch_exception_msg->streamnumber == 0 ) )
	{
		fn_userlog(c_ServiceName,"Inside Condition All Streams Impacted ");
		strcat(st_bcast_msg.c_broadcast_message,"All Streams Imapcted");
	}
	else if ( (st_brd_exch_exception_msg->streamnumber) )
	{
		fn_userlog(c_ServiceName,"Inside Condition Specific Stream No :%d: Impacted ",st_brd_exch_exception_msg->streamnumber);
		sprintf(st_bcast_msg.c_broadcast_message,"%s Specific Stream No :%d: Impacted ",st_bcast_msg.c_broadcast_message,st_brd_exch_exception_msg->streamnumber);
	}
  st_bcast_msg.st_hdr.li_log_time = st_brd_exch_exception_msg->st_hdr.li_log_time;
  st_bcast_msg.st_bcast_dest.flg_cntrl_ws=1;

  if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"Exchange Msg Is :%s:",st_bcast_msg.c_broadcast_message);
    fn_userlog(c_ServiceName,"Exchange Log Time Is :%ld:",st_bcast_msg.st_hdr.li_log_time);
    fn_userlog(c_ServiceName,"Before Call to Function fn_gnrltrd_msg");
  }

  if(DEBUG_MSG_LVL_2) 
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF before fn_gnrltrd_msg at :%s:",c_time);
  }

  i_ret_val = fn_gnrltrd_msg( &st_bcast_msg,
                              c_xchng_cd,
                              c_pipe_id,
                              c_ServiceName,
                              c_errmsg);

  if(i_ret_val == -1)
  {
    fn_errlog(c_ServiceName,"L31030", LIBMSG, c_errmsg);
    return -1;
  }

	li_mail_log_time = st_brd_exch_exception_msg->st_hdr.li_log_time;	
	fn_long_to_timearr(c_mail_time,li_mail_log_time);
	sprintf(st_bcast_msg.c_broadcast_message,"%s , Exchange Raised Exception At :%s:",st_bcast_msg.c_broadcast_message,c_mail_time);

	fn_send_exception_mailer(st_bcast_msg.c_broadcast_message);

  if(DEBUG_MSG_LVL_2)
  {
    fn_gettime_em();
    fn_userlog(c_ServiceName,"TIME_STATS NDF after fn_gnrltrd_msg at :%s:",c_time);
  }

  return 0;
}

void fn_send_exception_mailer(char *c_errmsg)
{
  char c_command[500] = {'\0'};

  rtrim(c_errmsg);

	fn_userlog("CLN_BRD_CLNT","Inside fn_send_exception_mailer");
  sprintf( c_command,"ksh $HOME/sh_scripts/fo_exch_exception_mailer.sh '%s'",c_errmsg);
  system(c_command);
	fn_userlog("CLN_BRD_CLNT","After fn_send_exception_mailer");
}

/*** Ver 1.9 Ends ***/

/*** Ver 2.0 Started ***/

void fn_orstonse_cntrct_desc_tr (struct vw_nse_cntrct *ptr_eba_cntrct,
                              	 struct st_contract_desc_tr *ptr_nse_cntrct_tr,
                              	 char *c_ServiceName,
                              	 char *c_err_msg)
 { 
    if ( ptr_eba_cntrct->c_prd_typ == 'F')
    {
      ptr_nse_cntrct_tr->c_instrument_name[0] = 'F';
      ptr_nse_cntrct_tr->c_instrument_name[1] = 'U';
      ptr_nse_cntrct_tr->c_instrument_name[2] = 'T';
    }
    else if(ptr_eba_cntrct->c_prd_typ == 'O' )
    {
      ptr_nse_cntrct_tr->c_instrument_name[0] = 'O';
      ptr_nse_cntrct_tr->c_instrument_name[1] = 'P';
      ptr_nse_cntrct_tr->c_instrument_name[2] = 'T';
    }
    else
    {
      fn_userlog(c_ServiceName, "Invalid product type");
    }

    if ( ptr_eba_cntrct->c_ctgry_indstk == 'I')
    {
      ptr_nse_cntrct_tr->c_instrument_name[3] = 'I';

      /**** Ver 1.5 ****/

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"before ptr_eba_cntrct->c_symbol:%s:",ptr_eba_cntrct->c_symbol);
      }

      if (strncmp(ptr_eba_cntrct->c_symbol,"INDIAVIX",(int)strlen("INDIAVIX")) == 0)
      {
        if(DEBUG_MSG_LVL_3)
        {
          fn_userlog(c_ServiceName,"Inside INDIAVIX");
        }
        ptr_nse_cntrct_tr->c_instrument_name[4] = 'V';
      }
      else
      {
        ptr_nse_cntrct_tr->c_instrument_name[4] = 'D';
      }

      if(DEBUG_MSG_LVL_3)
      {
        fn_userlog(c_ServiceName,"before ptr_nse_cntrct_tr->c_instrument_name |%s|",ptr_nse_cntrct_tr->c_instrument_name);
      }

      ptr_nse_cntrct_tr->c_instrument_name[5] = 'X';
    }
    else if(ptr_eba_cntrct->c_ctgry_indstk == 'S' )
    {
      ptr_nse_cntrct_tr->c_instrument_name[3] = 'S';
      ptr_nse_cntrct_tr->c_instrument_name[4] = 'T';
      ptr_nse_cntrct_tr->c_instrument_name[5] = 'K';
    }
    else
    {
      fn_userlog(c_ServiceName, "Should be Index/stock");
    }

    fn_orstonse_char ( ptr_nse_cntrct_tr->c_symbol,
          LEN_SYMBOL_NSE,
          ptr_eba_cntrct->c_symbol,
          strlen(ptr_eba_cntrct->c_symbol) );

    if(DEBUG_MSG_LVL_3)
    {
      fn_userlog(c_ServiceName,"before ptr_eba_cntrct->c_expry_dt:%s:",ptr_eba_cntrct->c_expry_dt);
      fn_userlog(c_ServiceName,"before ptr_nse_cntrct_tr->c_instrument_name |%s|",ptr_nse_cntrct_tr->c_instrument_name);
    }

    strcat ( ptr_eba_cntrct->c_expry_dt, " 14:30:00" );

    if(DEBUG_MSG_LVL_1)
    {
      fn_userlog(c_ServiceName,"after ptr_eba_cntrct->c_expry_dt:%s:",ptr_eba_cntrct->c_expry_dt);
    }

    fn_timearr_to_long(ptr_eba_cntrct->c_expry_dt,&(ptr_nse_cntrct_tr->li_expiry_date));

    if(DEBUG_MSG_LVL_1)
    {
      fn_userlog(c_ServiceName,"after conversion ptr_nse_cntrct_tr->li_expiry_date:%ld:",ptr_nse_cntrct_tr->li_expiry_date);
    }

    if ( ptr_eba_cntrct->c_prd_typ == 'F')
    {
      ptr_nse_cntrct_tr->li_strike_price = -1;
    }
    else
    {
      ptr_nse_cntrct_tr->li_strike_price = ptr_eba_cntrct->l_strike_prc;
    }

    if ( ptr_eba_cntrct->c_prd_typ == 'O' )
    {
      ptr_nse_cntrct_tr->c_option_type[0] = ptr_eba_cntrct->c_opt_typ;
      ptr_nse_cntrct_tr->c_option_type[1] = ptr_eba_cntrct->c_exrc_typ;
    }
    else
    {
      ptr_nse_cntrct_tr->c_option_type[0] = 'X';
      ptr_nse_cntrct_tr->c_option_type[1] = 'X';
    }
 }

/*** Ver 2.0 Ends ***/
