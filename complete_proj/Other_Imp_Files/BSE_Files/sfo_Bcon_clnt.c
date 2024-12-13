/******************************************************************************
 * Program Name : sfo_bcon_clnt.pc                                            *
 *                                                                            *
 * Version - Name         - Log                                               *
 * 1.0     - Sachin Birje - New Release                                       *
 * 1.1     - Sandip Tambe -29-Aug-2012- Changes for BSE CONSOLE               *
 * 1.2     - Navina D 14-Oct-2013 - Skip admin status check for connect All   *
 *                                  and Forward All options                   *  
*******************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>
#include <fcntl.h>

#include <sqlca.h>

#include <fo_fml_def.h>
#include <fo_view_def.h>
#include <fo.h>
#include <fn_env.h>
#include <fn_log.h>
#include <fn_tuxlib.h>
#include <fn_scklib_tap.h>        
#include <fn_read_debug_lvl.h>
#include <fn_msgq.h>
#include <fml_rout.h>
#include <fn_ddr.h>

#include <eba_to_ors.h>
#include <fo_bse_exg_con_lib.h>
#include <fo_bse_exch_comnN.h>
extern i_send_qid;

int fn_strt_bse_esr_clnt ( char *c_pipe_id,
                           char *c_ServiceName,
                           char *c_err_msg   );

int fn_strt_bse_pack_clnt ( char *c_pipe_id,
                            FBFR32 *ptr_fml_ibuf,
                            char *c_ServiceName,
                            char *c_err_msg  );

int fn_chk_n_strt_Bfrwrdng (char *c_pipe_id,
                           char *c_ServiceName,
                           char *c_err_msg);

int fn_stop_bse_pack_clnt(  char *c_pipe_id,
                            char *c_ServiceName,
                            char *c_err_msg   );

int fn_stop_bse_esr_clnt(  char *c_pipe_id,
                           char *c_ServiceName,
                           char *c_err_msg    );

int fn_stop_prcs(char *c_name ,
                long li_pro_id,
                char *c_ServiceName,
                char *c_err_msg);


int fn_get_prcs_id(char *c_prcs_nm,
                   char *c_xchng_cd,
                   char *c_pipe_id,
                   long *li_pro_id,
                   char *c_ServiceName,
                   char *c_err_msg);

int fn_strt_fobse_brdprcs(char *c_pipe_id,
                          char *c_ServiceName,
                          char *c_err_msg);

int fn_stop_fobse_brdclnt(char *c_pipe_id,
                          char *c_ServiceName,
                          char *c_err_msg);

int fn_connect_all( FBFR32 *ptr_fml_ibuf,
                    char *c_ServiceName,
                    char *c_err_msg);

int fn_fwdng_all (FBFR32 *ptr_fml_ibuf,
                  char *c_ServiceName,
                  char *c_err_msg);

int fn_mkt_stts_chng( char *c_trg_name,
                      int i_mkt_type,
                      int i_mkt_stts,
                      char *c_pipe_id,
                      char *c_ServiceName,
                      char *c_err_msg);

int fn_snd_opn_intrst_rqst  ( char *c_ServiceName,
                              char *c_err_msg
                            );
int fn_all_status(char   *c_pipe_id,
                  char   *c_all_status,
                  FBFR32 *ptr_fml_ibuf,
                  char   *c_ServiceName,
                  char   *c_err_msg);							/*** Ver 1.1 **/


#define MAX 50
#define PIPE  15

void SFO_BCON_CLNT ( TPSVCINFO *rqst )
{
  
  char c_rqst_typ;
  char c_prcs_nm[15];
  char c_ServiceName [ 33 ];
  char c_err_msg [ 256 ];
  char c_usr_id [ 9 ];
  char c_all_status[15];
  char c_pipe_id [ LEN_PIPE_ID ];
  char c_rgstrn_flg;
  char c_pipe_id1 [ LEN_PIPE_ID ];  /** 1.1 **/
  char c_pipe_ctgry[2];             /** 1.1 **/
  char c_pipe_catg_lst[100];        /** 1.1 **/
  char c_pipe_prmy_lst[100];        /** 1.1 **/
  char c_pipe_secdry_lst[100];      /** 1.1 **/
 
  int i_cnt;
  int i_ferr [ 3 ];
  int i_err [ 3 ];
  int i_adm_stts;
  int i_tran_id;
  int i_ch_val;

  TPINIT *pntr_st_tpinfo;

  FBFR32 *ptr_fml_ibuf;
  FBFR32 *ptr_fml_obuf;

  strcpy ( c_ServiceName, "SFO_BCON_CLNT" );

  INITDBGLVL(c_ServiceName);

  ptr_fml_ibuf = (FBFR32 *)rqst->data;
  ptr_fml_obuf = (FBFR32*)NULL ;


  MEMSET(c_all_status);

  i_err [ 0 ] = Fget32(ptr_fml_ibuf, FFO_RQST_TYP, 0, (char *)&c_rqst_typ, 0);
  i_ferr [ 0 ] = Ferror32;
  i_err [ 1 ] = Fget32(ptr_fml_ibuf,FFO_PIPE_ID,0,(char *)c_pipe_id, 0);
  i_ferr [ 1 ] = Ferror32;
  i_err [ 2 ] = Fget32(ptr_fml_ibuf, FFO_USR_ID, 0, (char *)c_usr_id, 0);
  i_ferr [ 2 ] = Ferror32;
  
  for( i_cnt=0 ; i_cnt < 3 ; i_cnt++)
  {
   if( i_err[i_cnt] == -1 )
   {
    fn_userlog(c_ServiceName, "Error in field no. %d",i_cnt);
    fn_errlog(c_ServiceName, "S31005", Fstrerror32(i_ferr[i_cnt]),c_err_msg );
    Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 ); 
   }
    
  }
  
  fn_userlog(c_ServiceName,"User ID:%s:,Pipe ID:%s:,Request Type:%c:",c_usr_id,c_pipe_id,c_rqst_typ); 
  fn_userlog(c_ServiceName,"---------------------------------------------");
  if ( strcmp ( c_usr_id, "system" ) != 0 )
  {
    fn_errlog(c_ServiceName, "B28501", DEFMSG, c_err_msg );
    Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
  }

  EXEC SQL
  SELECT opm_adm_stts
  INTO   :i_adm_stts
  FROM   opm_ord_pipe_mstr
  WHERE  opm_pipe_id = :c_pipe_id
  AND    opm_xchng_cd ='BFO'; /*** Commented in Ver 1.1  *** Uncommented in Ver 1.2 ***/

  if ( SQLCODE != 0 )
  {
    fn_errlog(c_ServiceName, "S31010", SQLMSG, c_err_msg );
    Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
  }

  if ( c_rqst_typ == CONNECT_ALL || c_rqst_typ == FRWD_ALL) /*** If condition added in Ver 1.2 ***/
	{
		fn_userlog(c_ServiceName,"Admin status check skipped");
  }
  else if ( c_rqst_typ != LOG_ON_REQ )
  {
    if ( i_adm_stts != 1 )
    {
      fn_errlog(c_ServiceName, "B28503", DEFMSG, c_err_msg );
      Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
    }
  }
  else
  {
    if ( i_adm_stts == 1 )
    {
      fn_errlog(c_ServiceName, "B28505", DEFMSG, c_err_msg );
      Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
    }
  }

  switch (c_rqst_typ)
  {
    case LOG_ON_REQ :
    	/************ Ver 1.1 Starts *******/
        EXEC SQL
             DECLARE cur_pipe_catg_list CURSOR FOR
             SELECT  opm_pipe_id,
                     opm_pipe_categary
             FROM    opm_ord_pipe_mstr
             WHERE   opm_xchng_cd = 'BFO';

        if ( SQLCODE != 0 )
        {
            fn_errlog(c_ServiceName, "S31015", SQLMSG, c_err_msg );
            EXEC SQL
              CLOSE cur_pipe_catg_list;
            tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
        }


        EXEC SQL
             OPEN  cur_pipe_catg_list;

        MEMSET(c_pipe_catg_lst);
        MEMSET(c_pipe_id1);
        MEMSET(c_pipe_ctgry);
        MEMSET(c_pipe_prmy_lst);
        MEMSET(c_pipe_secdry_lst);
        strcpy(c_pipe_prmy_lst,"P-");
        strcpy(c_pipe_secdry_lst,"F-");
				
				while(1)
				{
					EXEC SQL
						FETCH cur_pipe_catg_list
						INTO  :c_pipe_id1,
									:c_pipe_ctgry;
					  if ( (SQLCODE != 0 ) && ( SQLCODE != NO_DATA_FOUND ) )
            {
                fn_errlog(c_ServiceName, "S31020", SQLMSG, c_err_msg );
                EXEC SQL
                  CLOSE cur_pipe_catg_list;
                tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
            }
            if ( SQLCODE == NO_DATA_FOUND )
            {
              break;
            }
            rtrim(c_pipe_id1);
            rtrim(c_pipe_ctgry);
            if( strcmp(c_pipe_ctgry,"P") == 0 )
            {
              strcat(c_pipe_prmy_lst,(char *)c_pipe_id1);
              strcat(c_pipe_prmy_lst,":");
            }
            else if( strcmp(c_pipe_ctgry,"F") == 0 )
            {
              strcat(c_pipe_secdry_lst,(char *)c_pipe_id1);
              strcat(c_pipe_secdry_lst,":");
            }
				}
        strcpy(c_pipe_catg_lst,(char *)c_pipe_prmy_lst);
        strcat(c_pipe_catg_lst,",");
        strcat(c_pipe_catg_lst,(char *)c_pipe_secdry_lst);

        fn_userlog(c_ServiceName,"========== c_pipe_catg_lst [%s]",c_pipe_catg_lst);

        i_ch_val = Fadd32(ptr_fml_ibuf,FFO_PSTN_FLG,(char *)c_pipe_catg_lst,0);
        if( i_ch_val == -1 )
        {
             fn_errlog( c_ServiceName, "S31025", FMLMSG, c_err_msg );
             EXEC SQL
              CLOSE cur_pipe_catg_list;
             tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
        }
        EXEC SQL
             CLOSE cur_pipe_catg_list;

			/************ Ver 1.1 Ends *********/          
      i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
      if ( i_tran_id == -1 )
      {
        fn_errlog(c_ServiceName, "S31030", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }

      EXEC SQL
        UPDATE opm_ord_pipe_mstr
        SET    opm_adm_stts = 1
        WHERE  opm_pipe_id  = :c_pipe_id;

      if ( SQLCODE != 0 )
      {
        fn_errlog(c_ServiceName, "S31035", SQLMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }

      i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
      if ( i_ch_val == -1 )
      {
        fn_errlog(c_ServiceName, "S31040", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }


      tpreturn( TPSUCCESS, SUCC_BFR, (char *)ptr_fml_ibuf, 0 , 0 );    

      break;

   case LOG_OFF_REQ :

      i_tran_id = fn_begintran ( c_ServiceName, c_err_msg );
      if ( i_tran_id == -1 )
      {
        fn_errlog(c_ServiceName, "S31045", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }

      EXEC SQL
        UPDATE opm_ord_pipe_mstr
        SET    opm_adm_stts = 0
        WHERE  opm_pipe_id = :c_pipe_id;

      if ( SQLCODE != 0 )
      {
        fn_errlog(c_ServiceName, "S31050", SQLMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        fn_aborttran( c_ServiceName, i_tran_id, c_err_msg );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }

      i_ch_val = fn_committran ( c_ServiceName, i_tran_id, c_err_msg );
      if ( i_ch_val == -1 )
      {
        fn_errlog(c_ServiceName, "S31055", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;
 
   case STRT_ESR_CLNT   :
      i_ch_val = fn_strt_bse_esr_clnt ( c_pipe_id,
                                        c_ServiceName,
                                        c_err_msg);

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31060", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }

      break;
  case RGSTRN_TO_BSE :
  case AUTO_LOGIN :
       i_ch_val = fn_rgstrn_to_bse( ptr_fml_ibuf,
                                      c_ServiceName,
                                      c_err_msg);
       if ( i_ch_val == -1 )
       {
        strcat(c_err_msg ," **** RGSTRN FAIL ****");
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31065", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       }      
 
       break;  
 
   case LOGON_TO_BSE   :

        EXEC SQL 
         SELECT  BPH_USR_RGSTRN_FLG
         INTO    :c_rgstrn_flg
         FROM    BPH_BFO_PSSWD_HNDLG
         WHERE   BPH_PIPE_ID = :c_pipe_id;
       if ( SQLCODE != 0 )
       {
        fn_errlog(c_ServiceName, "S31070", SQLMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       } 
      if( c_rgstrn_flg == 'Y' )
      {                                              
        i_ch_val = fn_logon_bse_rqst( ptr_fml_ibuf,
                                      c_ServiceName,
                                      c_err_msg); 
        
       if ( i_ch_val == -1 )
       {
        strcat(c_err_msg ," **** Login failed ****");
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31075", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       }
      }
      else
      {
       fn_userlog(c_ServiceName,"REGISTRATION IS NOT SUCCESS ");
       fn_errlog(c_ServiceName, "S31080", LIBMSG, c_err_msg );
       Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
       tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 ); 
      }  
      break;
 
   case LOGOFF_FROM_BSE :
      i_ch_val = fn_logoff_from_bse( c_pipe_id,
                                     c_ServiceName,
                                     c_err_msg);
      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31085", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;
 
   case STOP_ESR_CLNT   :
      i_ch_val = fn_stop_bse_esr_clnt( c_pipe_id,
                                   c_ServiceName,
                                   c_err_msg);
       if ( i_ch_val == -1 )
       {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31090", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
       }
      break;

   case STRT_PACK_CLNT :

      i_ch_val = fn_strt_bse_pack_clnt (c_pipe_id,
                                    ptr_fml_ibuf,
                                    c_ServiceName,
                                    c_err_msg);

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31095", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;

   case STRT_FWD_ORDS :
      i_ch_val = fn_chk_n_strt_Bfrwrdng(c_pipe_id,
                                        c_ServiceName,
                                        c_err_msg);
      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31100", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break; 
   case STOP_FWD_ORDS :
      fn_pst_trg( c_ServiceName,
                 "TRG_STOP_PMP",
                 "TRG_STOP_PMP",
                 c_pipe_id);
      break;
 
   case STOP_PACK_CLNT :
      i_ch_val =  fn_stop_bse_pack_clnt(c_pipe_id,
                                        c_ServiceName,
                                        c_err_msg);

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31105", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;
 
   case CHNG_LGN_PASSWD :
        
      i_ch_val = fn_chng_optnl_passwd_req(ptr_fml_ibuf,
                                    OPTNL_PASSWD_CHNG,
                                    c_ServiceName,
                                    c_err_msg);
        
      if ( i_ch_val == -1 )
      {
        fn_errlog(c_ServiceName, "S31110", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;
   case STRT_BRD_CLNT :

      i_ch_val = fn_strt_fobse_brdprcs(c_pipe_id,
                                    c_ServiceName,
                                    c_err_msg);

      if( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31115" , LIBMSG,c_err_msg);
        Fadd32( ptr_fml_ibuf , FFO_ERR_MSG , c_err_msg ,0);
        tpreturn(TPFAIL , ERR_BFR, (char *)ptr_fml_ibuf ,0 ,0);
      }
      break;
   case STOP_BCAST_CLNT :

      i_ch_val =  fn_stop_fobse_brdclnt(c_pipe_id,
                                        c_ServiceName,
                                        c_err_msg);

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31120", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }

      break;
			
				/********** Ver 1.1 Starts *************/
     case STATUS_ALL_PROCSS:
			
				fn_userlog(c_ServiceName,"Before Calling fn_all_status for Pipe ID:%s:",c_pipe_id);
          i_ch_val = fn_all_status (c_pipe_id,
                                    c_all_status,
                                    ptr_fml_ibuf,
                                    c_ServiceName,
                                    c_err_msg);

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31125", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
				fn_userlog(c_ServiceName,"After Successfull Call fn_all_status for Pipe ID:%s:",c_pipe_id);
			break;
				/********** Ver 1.1 Ends **************/
   case CONNECT_ALL:
       fn_userlog(c_ServiceName,"--------------Connect All -----------------");
       i_ch_val = fn_connect_all(ptr_fml_ibuf,
                                c_ServiceName,
                                c_err_msg);

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31130", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;
 
   case FRWD_ALL:

      fn_userlog(c_ServiceName,"------------------ FWD_ALL -----------------");
      i_ch_val = fn_fwdng_all(ptr_fml_ibuf,
                                c_ServiceName,
                                c_err_msg);

      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31135", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;
   case SET_ORD_MKT_OPN:
      fn_userlog(c_ServiceName,"------------------ Send ORD_OPN ------------");
      i_ch_val = fn_mkt_stts_chng( "TRG_ORD_OPN",
                                    ORDER_MKT,
                                    OPEN,
                                    c_pipe_id,
                                    c_ServiceName,
                                    c_err_msg);
      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31140", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break; 
  case SET_ORD_MKT_CLS:
      i_ch_val = fn_mkt_stts_chng( "TRG_ORD_CLS",
                                    ORDER_MKT,
                                    CLOSED,
                                    c_pipe_id,
                                    c_ServiceName,
                                    c_err_msg);
      if ( i_ch_val == -1 )
      {
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31145", LIBMSG, c_err_msg );
        Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
      }
      break;

	case OPEN_INTRST_RQST	:

		i_ch_val	= fn_snd_opn_intrst_rqst	(	c_ServiceName,
																					c_err_msg
																				);	    

		if ( i_ch_val == -1 )
    {
    	fn_userlog(c_ServiceName,"%s",c_err_msg);
      fn_errlog(c_ServiceName, "S31150", LIBMSG, c_err_msg );
      Fadd32( ptr_fml_ibuf, FFO_ERR_MSG, c_err_msg, 0 );
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_fml_ibuf, 0, 0 );
    }

    break;
 
   default:
      i_ch_val = -1;
      fn_errlog( c_ServiceName, "S31155", "Protocol error", c_err_msg );
      break;
    
	}
	/******** Ver 1.1 Starts *****************************/

  if ( i_ch_val != -1 && c_rqst_typ == STATUS_ALL_PROCSS )
  {

          /**** Allocating output buffer ****/

          ptr_fml_obuf = (FBFR32 *)tpalloc("FML32",NULL,MIN_FML_BUF_LEN);
          if (ptr_fml_obuf == NULL)
          {
              fn_errlog( c_ServiceName, "S31160", TPMSG, c_err_msg );
              return ;
          }

          /**** Adding value in Output buffer ****/

          i_ch_val = Fadd32(ptr_fml_obuf,FFO_REMARKS,(char *)c_all_status,0);

          /***** Returning Value in output buffer **/

        tpreturn( TPSUCCESS, SUCC_BFR, (char *)ptr_fml_obuf, 0 , 0 );

  }
	/*********** Ver 1.1 Ends ***************************/

  tpreturn(TPSUCCESS, SUCC_BFR, NULL, 0, 0 );

}/**** End of Service ****/

int fn_strt_bse_esr_clnt ( char *c_pipe_id,
                           char *c_ServiceName,
                           char *c_err_msg   )
{
  int i_ch_val;
  char c_cmd_ln[256];
  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName," Inside fn_strt_bse_esr_clnt");
  }
  sprintf(c_cmd_ln, "nohup cln_bse_esr_clnt %s NA %s &", c_pipe_id, c_pipe_id);
  i_ch_val = system(c_cmd_ln);
  if (i_ch_val == -1 )
  {
    fn_errlog( c_ServiceName, "S31165", UNXMSG, c_err_msg );
    return -1;
  }
  return 0;

}

int  fn_stop_bse_esr_clnt( char *c_pipe_id,
                          char *c_ServiceName,
                          char *c_err_msg)

{

  int i_ch_val;
  long li_pro_id;
  char c_filter [256];
  char c_xchng_cd [3+1];
  char c_prcs_nm[20];

  EXEC SQL
    SELECT opm_xchng_cd
    INTO   :c_xchng_cd
    FROM   opm_ord_pipe_mstr
    WHERE  opm_pipe_id = :c_pipe_id;
  if ( SQLCODE != 0 )
  {
    fn_errlog(c_ServiceName, "S31170", SQLMSG, c_err_msg );
    return -1;
  }
  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName," INside fn_stop_bse_esr_clnt ");
  } 

  strcpy(c_prcs_nm,"cln_bse_esr_clnt");
  strcpy(c_xchng_cd,"NA");
  
  i_ch_val = fn_get_prcs_id(c_prcs_nm,
                             c_xchng_cd,
                             c_pipe_id,
                             &li_pro_id,
                             c_ServiceName,
                             c_err_msg);
  if( i_ch_val == 0 )
  {
    fn_userlog(c_ServiceName,"Process Id to be stopped:%d",li_pro_id);
  } 
  else
  {
    fn_userlog(c_ServiceName,"Process ID not found for cln_esr_clnt for pipe id:%s:Exchange code:%s:",c_pipe_id,c_xchng_cd);
    sprintf(c_err_msg,"Process ID not found for cln_esr_clnt for pipe id:%s:Exchange code:%s:",c_pipe_id,c_xchng_cd);
    return -1;
  }

  if ( li_pro_id != 0 )
  {
    i_ch_val = fn_stop_prcs(c_prcs_nm,li_pro_id,c_ServiceName,c_err_msg);
    if(i_ch_val == 0)
    {
      fn_userlog(c_ServiceName,"BSE ESR CLIENT STOPPED");
    }
    else
    {
      fn_userlog(c_ServiceName,"BSE ESR CLIENT COULD NOT BE STOPPED");
      sprintf(c_err_msg,"BSE ESR CLIENT COULD NOT BE STOPPED");
      return -1;
    }
  }
 
  return 0;


}


int fn_strt_bse_pack_clnt ( char *c_pipe_id,
                            FBFR32 *ptr_fml_ibuf,
                            char *c_ServiceName,
                            char *c_err_msg  )
{

  int   i_ch_val;
  int   i_login_stts;
  int   i_no_of_ordrs = 0;
  char  c_cmd_ln[256];
  char  c_run_mode;
  char  c_tm_stmp [ LEN_DT ];
  char  sql_c_clnt_nm[40];
  char  sql_c_clnt_stts;

  strcpy(sql_c_clnt_nm,"cln_bse_esr_clnt");

  /**** Checking if BSE  ESR is running or not ******/
 
  EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
      AND     bps_xchng_cd = 'NA'
      AND     bps_pipe_id = :c_pipe_id;

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31175", SQLMSG, c_err_msg);
    return -1;
  }


  if(sql_c_clnt_stts == 'N' )
  {
    sprintf(c_err_msg,"BSE ESR client is not running for pipe id :%s:,so pack client can not be started",c_pipe_id);
    return -1;
  }

  /**** Checking if user is already logged on or not ****/

  EXEC SQL
     SELECT opm_login_stts
     INTO   :i_login_stts
     FROM   opm_ord_pipe_mstr
     WHERE  opm_pipe_id = :c_pipe_id;

  if ( SQLCODE != 0 )
  {
    fn_errlog(c_ServiceName, "S31180", SQLMSG, c_err_msg);
    return -1;
  }

  if(i_login_stts == 0)
  {
    fn_userlog(c_ServiceName,"User is not logged in for pipe id :%s:",c_pipe_id);
    sprintf ( c_err_msg, "User is not logged in for pipe id :%s:",c_pipe_id);
    return -1;
  }

  i_ch_val = fn_unpack_fmltovar ( c_ServiceName,
                                  c_err_msg,
                                  ptr_fml_ibuf,
                                  2,
                                  FFO_SPL_FLG, (char *)&c_run_mode, NULL,
                                  FFO_ORD_TOT_QTY, (char*)&i_no_of_ordrs, NULL);
  if ( i_ch_val == -1 )
  {
    fn_errlog( c_ServiceName, "S31185", FMLMSG, c_err_msg );
    return -1;
  }

  if ( c_run_mode == 'B' )
  {
    i_ch_val = Fget32(ptr_fml_ibuf,FFO_TM_STMP,0,(char *)c_tm_stmp,0);

    if ( i_ch_val == -1 )
    {
       fn_errlog( c_ServiceName, "S31190", FMLMSG, c_err_msg );
       return -1;
    }
     sprintf ( c_cmd_ln, "nohup cln_bse_pack_clnt %s NA %s %d %c %s &",
              c_pipe_id, c_pipe_id, i_no_of_ordrs, c_run_mode, c_tm_stmp ); 
  }
  else
  {
    sprintf ( c_cmd_ln, "nohup cln_bse_pack_clnt %s NA %s %d %c &",
              c_pipe_id, c_pipe_id, i_no_of_ordrs, c_run_mode ); 
  }
 
  i_ch_val = system(c_cmd_ln);

  if (i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName, "L31005", LIBMSG, c_err_msg);
    return -1;
  }

  if (DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName,"Inside fn_strt_bse_pack_clnt"); 
   fn_userlog(c_ServiceName,"c_run_mode :%c:",c_run_mode);
   fn_userlog(c_ServiceName,"i_no_of_ordrs:%d:",i_no_of_ordrs);
   fn_userlog(c_ServiceName,"c_tm_stmp :%s:",c_tm_stmp);
   fn_userlog(c_ServiceName,"i_login_stts :%d:",i_login_stts);
  }

  return 0;

}


int fn_stop_bse_pack_clnt (  char *c_pipe_id,
                         char *c_ServiceName,
                         char *c_err_msg)
{
   char c_filter [256];

   sprintf (c_filter, "cln_bse_pack_clnt_%s",c_pipe_id);

   fn_pst_trg( c_ServiceName,"TRG_STP_BAT","TRG_STP_BAT",c_filter); 

   return 0;
}

int fn_chk_n_strt_Bfrwrdng(char *c_pipe_id,
                           char *c_ServiceName,
                           char *c_err_msg)
{
 
   EXEC SQL BEGIN DECLARE SECTION;
      char c_exg_nxt_trd_dt[15];
      char c_exg_crrnt_stts;
      char c_exg_extnd_mrkt_stts;
      char c_exg_exrc_mkt_stts;
    EXEC SQL END DECLARE SECTION;

    EXEC SQL
      SELECT exg_nxt_trd_dt,
             exg_crrnt_stts,
             exg_exrc_mkt_stts,
             exg_extnd_mrkt_stts
      INTO   :c_exg_nxt_trd_dt,
             :c_exg_crrnt_stts,
             :c_exg_exrc_mkt_stts,
             :c_exg_extnd_mrkt_stts
      FROM   exg_xchng_mstr
      WHERE exg_xchng_cd = 'BFO';

    if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "S31195", SQLMSG, c_err_msg );
      return -1;
    }

    fn_userlog (c_ServiceName, "FNO BSE Trade date |%s|", c_exg_nxt_trd_dt );
    fn_userlog (c_ServiceName, "FNO BSE Current status |%c|", c_exg_crrnt_stts );
    fn_userlog (c_ServiceName, "FNO BSE Exercise Market status |%c|", c_exg_exrc_mkt_stts );
    fn_userlog (c_ServiceName, "FNO BSE Extnd Market Status |%c|", c_exg_extnd_mrkt_stts );

    if (( c_exg_crrnt_stts      == 'O' ) || ( c_exg_exrc_mkt_stts   == 'O' ) || ( c_exg_extnd_mrkt_stts == 'O' ))
    {
      fn_userlog(c_ServiceName, "Posting trigger as Market Status is OPEN" );
      fn_pst_trg( c_ServiceName,
                 "TRG_STRT_PMP",
                 "TRG_STRT_PMP",
                 c_pipe_id);
      sprintf(c_err_msg,"Posting trigger as Market Status is OPEN");
    }
    else
    {
      fn_userlog(c_ServiceName, "Not posting trigger as Market Status is NOT OPEN" );
      sprintf(c_err_msg,"Not posting trigger as Market Status is NOT OPEN");
      return (-1);
    }

    return (0);

}

int fn_get_prcs_id(char *c_prcs_nm, char *c_xchng_cd, char *c_pipe_id, long *li_pro_id,char *c_ServiceName, char *c_err_msg)
{

  char  c_command[150];
  char  c_file_nm[100];
  char  c_user[20];
  char  c_process[20];
  char  c_exchange[13+1];
  FILE  *fptr;
  long   li_tmp_prc_id;

  MEMSET(c_file_nm);
  MEMSET(c_command);

  strcpy(c_file_nm, getenv("STATUS_FILE"));

  fn_userlog(c_ServiceName,"IN the process get process id");
  fn_userlog(c_ServiceName,"process Name:%s:",c_prcs_nm);
  fn_userlog(c_ServiceName,"Exchange Code:%s:",c_xchng_cd);
  fn_userlog(c_ServiceName,"Pipe Id:%s:",c_pipe_id);

  remove(c_file_nm);

  sprintf(c_command, "ps -efa -o pid,user,comm,args | grep -v 'grep' |grep $USER | grep %s | grep \"%s %s\" > %s",c_xchng_cd,
c_prcs_nm,c_pipe_id,c_file_nm);


  system(c_command);

  fptr = (FILE *)fopen(c_file_nm, "r");
  if(fptr == NULL)
  {
    fn_errlog(c_ServiceName,"S31200", LIBMSG, c_err_msg);
    return -1;
  }

  MEMSET(c_process);

  fscanf(fptr, "%d %s",&li_tmp_prc_id,c_process);
  fn_userlog(c_ServiceName, "Process :%s: is running and process id is :%d",c_process,li_tmp_prc_id);

  *li_pro_id = li_tmp_prc_id;

  fclose(fptr);
  return 0;
}

int fn_stop_prcs(char *c_name ,long li_pro_id, char* c_ServiceName,char *c_err_msg)
{

  union sigval  st_data;
  st_data.sival_int = SIGUSR2;
  fn_userlog(c_ServiceName,"Inside stop process for %s:",c_name);

  /*Raise a signal*/
  if(sigqueue(li_pro_id,SIGUSR2, st_data) == -1)
  {
   fn_errlog(c_ServiceName, "L31010", UNXMSG,c_err_msg);
   return -1;
  }
  fn_userlog(c_ServiceName, "Signal(%d) raised to %d", SIGUSR2, li_pro_id);
  return 0;


}
int fn_strt_fobse_brdprcs( char *c_pipe_id ,
                           char *c_ServiceName,
                           char *c_err_msg )
{
    int i_ch_val;
    char  c_cmd_ln[256];
		char  c_cmd_ln_c[256];
  fn_userlog(c_ServiceName ,"Inside function fn_strt_fobse_brdprcs() ");
	
	fn_userlog(c_ServiceName , " Starting Broadcast Client ");
 
	 sprintf(c_cmd_ln_c, "nohup cln_fobse_brdclnt %s NA %s &", c_pipe_id, c_pipe_id);
  i_ch_val = system(c_cmd_ln_c);
  if (i_ch_val == -1 )
  {
    fn_errlog( c_ServiceName, "S31205", UNXMSG, c_err_msg );
    fn_userlog(c_ServiceName,"S32175" ,"failed in starting Broadcast client");
    return -1;
  }
	
	fn_userlog(c_ServiceName , " Starting Broadcast process ");
 
	sprintf(c_cmd_ln, "nohup cln_fobse_brdprcs %s NA %s &", c_pipe_id, c_pipe_id);
  i_ch_val = system(c_cmd_ln);
  if (i_ch_val == -1 )
  {
    fn_errlog( c_ServiceName, "S31210", UNXMSG, c_err_msg );
    fn_userlog(c_ServiceName,"S32275" ,"failed in starting Broadcast process ");
    return -1;
  }
  return 0;

}
fn_stop_fobse_brdclnt(char *c_pipe_id, char *c_ServiceName, char *c_err_msg)
{
  int i_ch_val;
  long li_pro_id;
  char c_filter [256];
  char c_xchng_cd [3+1];
  char c_prcs_nm[20];
	char c_prcs_nm_c[20];

  EXEC SQL
    SELECT opm_xchng_cd
    INTO   :c_xchng_cd
    FROM   opm_ord_pipe_mstr
    WHERE  opm_pipe_id = :c_pipe_id;
  if ( SQLCODE != 0 )
  {
    fn_errlog(c_ServiceName, "S31215", SQLMSG, c_err_msg );
    return -1;
  }
  if(DEBUG_MSG_LVL_3)
  {
   fn_userlog(c_ServiceName," INside fn_stop_fobse_brdclnt ");
  }

  strcpy(c_prcs_nm,"cln_fobse_brdprcs");
  strcpy(c_xchng_cd,"NA");

  i_ch_val = fn_get_prcs_id(c_prcs_nm,
                             c_xchng_cd,
                             c_pipe_id,
                             &li_pro_id,
                             c_ServiceName,
                             c_err_msg);
  if( i_ch_val == 0 )
  {
    fn_userlog(c_ServiceName,"Process Id to be stopped:%d",li_pro_id);
  }
   else
  {
    fn_userlog(c_ServiceName,"Process ID not found for cln_fobse_brdprcs for pipe id:%s:Exchange code:%s: ",c_pipe_id,c_xchng_cd);
    sprintf(c_err_msg,"Process ID not found for cln_bobse_brdprcs for pipe id:%s:Exchange code:%s:",c_pipe_id,c_xchng_cd);
    return -1;
  }

  if ( li_pro_id != 0 )
  {
    i_ch_val = fn_stop_prcs(c_prcs_nm,li_pro_id,c_ServiceName,c_err_msg);
    if(i_ch_val == 0)
    {
      fn_userlog(c_ServiceName,"BSE BROADCAST PROCESS STOPPED");
    }
    else
    {
      fn_userlog(c_ServiceName,"BSE BROADCAST PROCESS COULD NOT BE STOPPED");
      sprintf(c_err_msg,"BSE BROADCAST COULD NOT BE STOPPED");
      return -1;
    }
  }

	fn_userlog(c_ServiceName ,"Now Stopping Broadcast Client ");
  strcpy(c_prcs_nm_c,"cln_fobse_brdclnt");
  /*strcpy(c_xchng_cd,"NA");*/
	li_pro_id =0;

  i_ch_val = fn_get_prcs_id(c_prcs_nm_c,
                             c_xchng_cd,
                             c_pipe_id,
                             &li_pro_id,
                             c_ServiceName,
                             c_err_msg);
  if( i_ch_val == 0 )
  {
    fn_userlog(c_ServiceName,"Process Id to be stopped:%d",li_pro_id);
  }
   else
  {
    fn_userlog(c_ServiceName,"Process ID not found for cln_fobse_brdclnt for pipe id:%s:Exchange code:%s: ",c_pipe_id,c_xchng_cd);
    sprintf(c_err_msg,"Process ID not found for cln_bobse_brdclnt for pipe id:%s:Exchange code:%s:",c_pipe_id,c_xchng_cd);
    return -1;
  }

  if ( li_pro_id != 0 )
  {
    i_ch_val = fn_stop_prcs(c_prcs_nm_c,li_pro_id,c_ServiceName,c_err_msg);
    if(i_ch_val == 0)
    {
      fn_userlog(c_ServiceName,"BSE BROADCAST CLIENT STOPPED");
    }
    else
    {
      fn_userlog(c_ServiceName,"BSE BROADCAST CLIENT COULD NOT BE STOPPED");
      sprintf(c_err_msg,"BSE BROADCAST CLIENT COULD NOT BE STOPPED");
      return -1;
    }
  }


  return 0;

 }

int fn_connect_all(FBFR32 *ptr_fml_ibuf,
                   char *c_ServiceName,
                   char *c_err_msg)
{
    int i;
    int i_len;
    int i_ch_val;
    int i_tran_id;
    int li_tot_qty;
    int count=0;
    int i_ferr [ 3 ];
    int i_err [ 3 ];
    int i_loop;
    long  l_user_id;
    char  c_run_mode;
    char  c_order_dtls;
    char  c_rqst_typ;
    char  c_dwn_flg;
    char  c_pipe_id[3];
    char  c_user_id[6];
    long  li_user_id;
    char  c_pipe_list1[MAX];
    char  c_pipe_list[PIPE][3];
    char  *null_ptr = 0;
    char  *ptr_c_flg;
    char  c_tm_stmp [ LEN_DT ];
    char c_auto_login_flg;

    char c_passwd[LEN_PASS_WD];

    MEMSET(c_pipe_list1);
    MEMSET(c_run_mode);

    fn_userlog(c_ServiceName,"connect all loop");

    i_ch_val = fn_unpack_fmltovar ( c_ServiceName,
                                    c_err_msg,
                                    ptr_fml_ibuf,
                                    2,  /*** Changed from 3 to 2 in Ver 1.1 ***/
                                    FFO_QUEUE_NAME, (char *)c_pipe_list1, NULL,
                                   /***  FFO_PASS_WD, (char*)c_passwd, NULL,  *** Commented in Ver 1.1 ***/
                                    FFO_SPL_FLG,(char *)&c_run_mode,0);

    if ( i_ch_val == -1 )
    {
      fn_errlog( c_ServiceName, "S31220", FMLMSG, c_err_msg );
      return -1;
    }

    fn_userlog(c_ServiceName," --------------------------step 1  --------------------------------------");

    fn_userlog(c_ServiceName,"c_pipe_list1 :%s:",c_pipe_list1);
    fn_userlog(c_ServiceName,"c_passwd :%s:",c_passwd);
    fn_userlog(c_ServiceName,"c_run_mode:%c:",c_run_mode);

    if ( c_run_mode == 'B' )
    {
         i_ch_val = Fget32(ptr_fml_ibuf,FFO_TM_STMP,0,(char *)c_tm_stmp,0);

         if ( i_ch_val == -1 )
         {
              fn_errlog( c_ServiceName, "S31225", FMLMSG, c_err_msg );
              return -1;
         }

    }

    fn_userlog(c_ServiceName,"c_tm_stmp:%s:",c_tm_stmp);

    i_len = strlen(c_pipe_list1);
    fn_userlog(c_ServiceName,"i_len = :%d:",i_len);

    for(i = 0;i< i_len; i++ )
    {
        if(c_pipe_list1[i]  == '-')
        {
        count++;
        }
    }

    fn_userlog(c_ServiceName,"count :%d:",count);

    for(i=0;i<count +1 ;i++)
    {
        fn_userlog(c_ServiceName," -------------------------- step 2 --------------------------------------");

        MEMSET(c_pipe_id);

        if(i == 0)
        {
        strcpy(c_pipe_list[i],strtok(c_pipe_list1,"-"));
        }
        else
        {
        strcpy(c_pipe_list[i],strtok(null_ptr, "-" ));

        }

        strcpy(c_pipe_id,c_pipe_list[i]);

        fn_userlog(c_ServiceName,"c_pipe_id :%s:",c_pipe_id);

        EXEC SQL
             SELECT opm_user_id
             INTO   :c_user_id
             FROM   opm_ord_pipe_mstr
             WHERE  opm_pipe_id = :c_pipe_id
             AND    opm_xchng_cd = 'BFO';

        if ( SQLCODE != 0 )
        {
            fn_errlog(c_ServiceName, "S31230", SQLMSG, c_err_msg );
            return -1;
        }

        fn_userlog(c_ServiceName,"c_user_id :%s:",c_user_id);

        fn_userlog(c_ServiceName," *********** Starting ESR Client ******************");

        i_ch_val = fn_strt_bse_esr_clnt ( c_pipe_id,
                                          c_ServiceName,
                                          c_err_msg);

        if ( i_ch_val == -1 )
        {
             fn_userlog(c_ServiceName,"%s",c_err_msg);
             fn_errlog(c_ServiceName, "S31235", LIBMSG, c_err_msg );
             return -1;
        }

       sleep(1);
       c_auto_login_flg='A'; /*** Ver 1.1 changed from M to A for Autologin **/
       l_user_id =  atol( c_user_id);
  

       i_err[0] = Fchg32( ptr_fml_ibuf, FFO_PIPE_ID, 0, (char *)c_pipe_id, 0 );
       i_ferr[0] = Ferror32;
       i_err[1] =  Fchg32( ptr_fml_ibuf, FFO_SPL_FLG, 0, (char *)&c_auto_login_flg, 0 );
       i_ferr[1] = Ferror32;
       i_err[2] =  Fchg32( ptr_fml_ibuf, FFO_EXCHNG_USR_ID, 0, (char *)&l_user_id, 0 );
       i_ferr[2] = Ferror32;
 
       for( i_loop=0; i_loop<=2; i_loop++ )
       {
        if(i_err[i_loop] == -1)
        {
         fn_errlog(c_ServiceName, "S31240", FMLMSG, c_err_msg);
          printf( "System error. Contact system support\n" );
          return -1; 
        }
       } 
       fn_userlog(c_ServiceName," *********** Registration to IML ******************"); 
       i_ch_val = fn_rgstrn_to_bse( ptr_fml_ibuf,
                                      c_ServiceName,
                                      c_err_msg);
       if ( i_ch_val == -1 )
       {
        strcat(c_err_msg ," **** RGSTRN FAIL ****");
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31245", LIBMSG, c_err_msg );
        return -1;
       }
       
       sleep(3);

			 /*** Commented in Ver 1.1 for Autologin ***
	
       i_ch_val = fn_logon_bse_rqst( ptr_fml_ibuf,
                                      c_ServiceName,
                                      c_err_msg);  
       if ( i_ch_val == -1 )
       {
        strcat(c_err_msg ," **** Logon FAIL ****");
        fn_userlog(c_ServiceName,"%s",c_err_msg);
        fn_errlog(c_ServiceName, "S31250", LIBMSG, c_err_msg );
        return -1;
       }
			 ***/
 
       sleep(3);
 
       fn_userlog(c_ServiceName," *********** Starting Pack Client ******************");

       i_ch_val =  Fchg32(ptr_fml_ibuf,FFO_SPL_FLG,0,(char *)&c_run_mode,0);
       if( i_ch_val == -1 )
       {
         fn_errlog( c_ServiceName, "S31255", FMLMSG, c_err_msg );
       }
      
       li_tot_qty = 0;

       i_ch_val = Fadd32(ptr_fml_ibuf,FFO_ORD_TOT_QTY,(char *)&li_tot_qty,0);
       if( i_ch_val == -1 )
       {
         fn_errlog( c_ServiceName, "S31260", FMLMSG, c_err_msg );
       }

       if ( c_run_mode == 'B' )
       {
           i_ch_val = Fadd32(ptr_fml_ibuf,FFO_TM_STMP,(char *)c_tm_stmp,0);
           if( i_ch_val == -1 )
           {
              fn_errlog( c_ServiceName, "S31265", FMLMSG, c_err_msg );
           }

       }

       fn_userlog(c_ServiceName," li_tot_qty:%d:",li_tot_qty);
       fn_userlog(c_ServiceName," c_run_mode :%c:",c_run_mode);


       i_ch_val = fn_strt_bse_pack_clnt (c_pipe_id,
                                     ptr_fml_ibuf,
                                     c_ServiceName,
                                     c_err_msg);

       if ( i_ch_val == -1 )
       {
           fn_userlog(c_ServiceName,"%s",c_err_msg);
           fn_errlog(c_ServiceName, "S31270", LIBMSG, c_err_msg );
           return -1;
       }

       sleep(1); 
         
       fn_userlog(c_ServiceName," ===================END OF PIPE :%s:===========================\n\n ",c_pipe_id);
   }

  return 0;
 }

int fn_fwdng_all(FBFR32 *ptr_fml_ibuf,
                   char *c_ServiceName,
                   char *c_err_msg)
{

    int i;
    int i_ch_val;
    int i_len;
    int count = 0 ;
    char *null_ptr = 0;
    char c_pipe_list1[MAX];
    char  c_pipe_list[PIPE][3];
    char c_pipe_id[3];

    i_ch_val = fn_unpack_fmltovar ( c_ServiceName,
                                    c_err_msg,
                                    ptr_fml_ibuf,
                                    1,
                                    FFO_QUEUE_NAME, (char *)c_pipe_list1, NULL);
    if ( i_ch_val == -1 )
    {
      fn_errlog( c_ServiceName, "S31275", FMLMSG, c_err_msg );
      return -1;
    }

    fn_userlog(c_ServiceName,"c_pipe_list1 :%s:",c_pipe_list1);

    i_len = strlen(c_pipe_list1);

    for(i = 0;i< i_len; i++ )
    {
        if(c_pipe_list1[i]  == '-')
        {
        count++;
        }
    }

    fn_userlog(c_ServiceName,"count :%d:",count);

    for(i=0;i<count + 1;i++)
    {

        MEMSET(c_pipe_id);

        if(i == 0)
        {
        strcpy(c_pipe_list[i],strtok(c_pipe_list1,"-"));
        }
        else
        {
        strcpy(c_pipe_list[i],strtok(null_ptr, "-" ));

        }

        strcpy(c_pipe_id,c_pipe_list[i]);

        fn_userlog(c_ServiceName,"c_pipe_id :%s:",c_pipe_id);


        i_ch_val =  fn_chk_n_strt_Bfrwrdng(c_pipe_id,
                                          c_ServiceName,
                                          c_err_msg);


        if ( i_ch_val == -1 )
        {
            fn_userlog(c_ServiceName,"%s",c_err_msg);
            fn_errlog(c_ServiceName, "S31280", LIBMSG, c_err_msg );
            return -1;
        }

    }
return 0;
 }

/*****************************************************************************/
/*** Checking and applying Market status                                     */
/*****************************************************************************/
int fn_mkt_stts_chng( char *c_trg_name,
                      int i_mkt_type,
                      int i_mkt_stts,
                      char *c_pipe_id,
                      char *c_ServiceName,
                      char *c_err_msg)
{

  int i_ch_val;

  EXEC SQL BEGIN DECLARE SECTION;
    struct vw_xchngstts st_stts;
    char sql_c_ip_pipe_id[2+1];
  EXEC SQL END DECLARE SECTION;

  fn_pst_trg( c_ServiceName,
              c_trg_name,
              c_trg_name,
              "BFO");

  strcpy ( sql_c_ip_pipe_id, c_pipe_id);
  
  EXEC SQL
    SELECT opm_xchng_cd
    INTO   :st_stts.c_xchng_cd
    FROM   opm_ord_pipe_mstr
    WHERE  opm_pipe_id = :c_pipe_id;

    if ( SQLCODE != 0 )
    {
      fn_errlog(c_ServiceName, "S31285", SQLMSG, c_err_msg );
      return -1;
    }

    if( i_mkt_type == ORDER_MKT )
    {
      if( i_mkt_stts == OPEN )
      {
          st_stts.c_crrnt_stts = EXCHANGE_OPEN ;
      }
      else
      {
          st_stts.c_crrnt_stts = EXCHANGE_UNDER_EXPIRY ;
      }

      st_stts.c_rqst_typ = UPD_NORMAL_MKT_STTS ;
    }
    
    fn_cpy_ddr(st_stts.c_rout_crt);

    i_ch_val = fn_acall_svc ( "sfo_con_clnt",
                              c_err_msg,
                              &st_stts,
                              "vw_xchngstts",
                              sizeof (st_stts ),
                              TPNOREPLY,
                              "SFO_UPD_XSTTS" );

    if ( i_ch_val == SYSTEM_ERROR )
    {
      fn_errlog ( c_ServiceName, "S31290", LIBMSG, c_err_msg );
      return -1;
    }

   return 0;
}
		
		/************ Ver 1.1 Starts ***/

int fn_all_status( char *c_pipe_id,
                   char *c_all_status,
                   FBFR32 *ptr_fml_ibuf,
                   char *c_ServiceName,
                   char *c_err_msg)
{

    int       i_ch_val;
    int       i_sck_id;
    char      *ptr_c_port;
    char      *ptr_c_add;
    char      c_ors_addrss[16];
    char      *c_status;
    char      sql_c_clnt_nm[40];
    char      sql_c_esr_clnt_stts;
    char      sql_c_pack_clnt_stts;
    char      sql_c_brd_clnt_stts;
    char      sql_c_login_stts;
    char      sql_c_fwd_stts;
    long int  li_ors_port_no;
		char  		sql_c_rgstr_stts;
		char  		sql_c_mrkt_opn_stts;


   fn_userlog(c_ServiceName,"************* Checking Status of ESR Client ********");

   strcpy(sql_c_clnt_nm,"cln_bse_esr_clnt");

    EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_esr_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
      AND     bps_xchng_cd = 'NA'
      AND     bps_pipe_id = :c_pipe_id;

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31295", SQLMSG, c_err_msg);
    return -1;
  }

   fn_userlog(c_ServiceName,"sql_c_clnt_stts - ESR :%c:",sql_c_esr_clnt_stts);
  fn_userlog(c_ServiceName,"************* Checking Status of PACK Client ********");

  strcpy(sql_c_clnt_nm,"cln_bse_pack_clnt");

    EXEC SQL
      SELECT  bps_stts
      INTO    :sql_c_pack_clnt_stts
      FROM    bps_btch_pgm_stts
      WHERE   bps_pgm_name = :sql_c_clnt_nm
      AND     bps_xchng_cd = 'NA'
      AND     bps_pipe_id = :c_pipe_id;

   if(SQLCODE != 0)
   {
      fn_errlog(c_ServiceName, "S31300", SQLMSG, c_err_msg);
      return -1;
   }


 		EXEC SQL
       SELECT opm_login_stts,
              opm_pmp_stts
       INTO   :sql_c_login_stts,
              :sql_c_fwd_stts
       FROM   opm_ord_pipe_mstr
       WHERE  opm_pipe_id = :c_pipe_id
       AND    opm_xchng_cd = 'BFO';

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31305", SQLMSG, c_err_msg);
    return -1;
  }
	EXEC SQL
		SELECT bph_usr_rgstrn_flg
		INTO 	 :sql_c_rgstr_stts
		FROM 	 bph_bfo_psswd_hndlg
		WHERE  bph_pipe_id= :c_pipe_id;

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31310", SQLMSG, c_err_msg);
    return -1;
  }
	EXEC SQL
		SELECT exg_crrnt_stts
		INTO 	 :sql_c_mrkt_opn_stts
		FROM   exg_xchng_mstr
		WHERE  exg_xchng_cd='BFO';

  if(SQLCODE != 0)
  {
    fn_errlog(c_ServiceName, "S31315", SQLMSG, c_err_msg);
    return -1;
  }


  fn_userlog(c_ServiceName,"sql_c_clnt_stts - PACK :%c:",sql_c_pack_clnt_stts);
  fn_userlog(c_ServiceName,"sql_c_login_stts:%c:",sql_c_login_stts);
  fn_userlog(c_ServiceName,"sql_c_fwd_stts :%c:",sql_c_fwd_stts);
  fn_userlog(c_ServiceName,"sql_c_rgstr_stts :%c:",sql_c_rgstr_stts);
  fn_userlog(c_ServiceName,"sql_c_mrkt_opn_stts :%c:",sql_c_mrkt_opn_stts);

  fn_userlog(c_ServiceName,"Final Remark:");
  fn_userlog(c_ServiceName,"Status : ESR CLNT - LOGIN - PACK CLNT - FWD-RGSTRN-MRKT_STTS ");
  sprintf(c_all_status,"%c-%c-%c-%c-%c-%c",sql_c_esr_clnt_stts,sql_c_login_stts,sql_c_pack_clnt_stts,sql_c_fwd_stts,sql_c_rgstr_stts,sql_c_mrkt_opn_stts);

  fn_userlog(c_ServiceName,"c_all_status FINAL:%s:",c_all_status);

  return 0;
}

		/*** Ver 1.1 Ends ******/

