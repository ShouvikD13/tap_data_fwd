/******************************************************************************/
/*	Program	    			:	SFO_GET_SEQ                                           */
/*                                                                            */
/*  Input             : vw_sequence                                           */
/*                                                                            */
/*                                                                            */
/*  Output            : vw_sequence                                           */
/*                      vw_err_msg                                            */
/*                                                                            */
/*  Description       : This service fetchs the sequence number and update    */
/*                      it based on the request type                          */
/*                                                                            */
/*  Log               : 1.0    17-Oct-2001 A.Satheesh Kumar Reddy             */
/*                      1.1    27-Oct-2007 Vidyayini Krish                    */
/*                      1.2    10-Mar-2008 SN for 16 & 17  										*/
/*										:	1.3		 04-Apr-2008 Sandeep  										  		*/
/*										: 1.4    25-Jun-2009 Indrajit ORS PACK client  changes	*/
/*																				 TRAdE sequence fetched for UPDATE  */
/*										:	1.5		 07-Jul-2009 Sandeep 														*/
/*										:	1.6		 19-Jan-2012 Swati A.B                          */
/*                    : 1.7    05-Jul-2012 Sachin Birje                       */
/*                    : 1.8    01-Aug-2012 SH																	*/
/*                    : 1.9    23-Aug-2012 Sachin Birje                       */
/*                    : 2.0    20-Jul-2012 Mahesh Shinde                      */
/*                    : 2.1    05-Dec-2012 Swati A.B													*/
/*                    : 2.3    07-Dec-2017 Mrinal Kishore                     */
/*                    : 2.4    28-Feb-2023 Devesh Chawda                      */
/*                    : 2.5    24-Mar-2023 Devesh Chawda                      */
/******************************************************************************/
/*	1.3		-	IBM Migration Changes                                             */
/*																																						*/
/*	1.5		-	Use Of Database Sequence For Request Type GET_TRD_SEQ							*/
/*	1.6		-	BSE Chnages                                                       */
/*  1.7   - BSE Sequence Changes                                              */
/*  1.8   - NSE F8 Sequence Changes                                           */
/*  1.9   - BSE A3 Sequence Changes                                           */
/*  2.0   - Use Of Database Sequence For Request Type GET_ORDR_SEQ            */
/*  2.1   - Overnight Orders@3:30 Changes for Offline Orders 								  */
/*  2.3   - Use Of Database Sequence For Request Type GET_SPRDORD_SEQ         */
/*  2.4   - Pipe Id 21 , 22, 23 and 24 Sequence                               */
/*  2.5   - Pipe Id 25, 26 and 27 sequence                                    */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <atmi.h>
#include <sqlca.h>
#include <fml32.h>
#include <fo.h>
#include <fo_view_def.h>
#include <fn_tuxlib.h>
#include <fn_ddr.h>
#include <fn_log.h>            /*** Ver 1.3 ***/
#include <fn_read_debug_lvl.h> /*** Ver 1.3 ***/

void SFO_GET_SEQ(TPSVCINFO *rqst)
{
  char c_ServiceName[33];
  char c_errmsg[256];
  int i_trnsctn;
  char c_pipe_id[3];

  EXEC SQL BEGIN DECLARE SECTION;
  struct vw_sequence *ptr_st_i_seq;
  varchar c_trd_dt[LEN_DATE];
  varchar xchng_stmt[500];
  int i_xchng_seq;
  int i_len;
  int i_seq;
  EXEC SQL END DECLARE SECTION;

  struct vw_err_msg *ptr_st_err_msg;

  ptr_st_i_seq = (struct vw_sequence *)rqst->data;
  strcpy(c_ServiceName, rqst->name);
  INITDBGLVL(c_ServiceName); /*** Ver 1.3 ***/

  strcpy((char *)c_trd_dt.arr, ptr_st_i_seq->c_trd_dt);
  SETLEN(c_trd_dt);

  if (DEBUG_MSG_LVL_3) /*** Ver 1.3 ***/
  {
    fn_userlog(c_ServiceName, "c_pipe_id  :%s:", ptr_st_i_seq->c_pipe_id);
    fn_userlog(c_ServiceName, "c_trd_dt  :%s:", c_trd_dt.arr);
    fn_userlog(c_ServiceName, "c_rqst_typ  :%c:", ptr_st_i_seq->c_rqst_typ);
  }

  ptr_st_err_msg = (struct vw_err_msg *)tpalloc("VIEW32",
                                                "vw_err_msg",
                                                sizeof(struct vw_err_msg));
  if (ptr_st_err_msg == NULL)
  {
    fn_errlog(c_ServiceName, "S31005", TPMSG, c_errmsg);
    tpreturn(TPFAIL, NO_BFR, NULL, 0, 0);
  }

  memset(ptr_st_err_msg, 0, sizeof(struct vw_err_msg));

  /*** Added for Order Routing ***/
  fn_init_ddr_val(ptr_st_i_seq->c_rout_crt);

  i_trnsctn = fn_begintran(c_ServiceName, c_errmsg);

  if (i_trnsctn == -1)
  {
    strcpy(ptr_st_err_msg->c_err_msg, c_errmsg);
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
  }

  if (ptr_st_i_seq->c_rqst_typ == GET_XCHNG_SEQ)
  {
    if (strcmp(ptr_st_i_seq->c_pipe_id, "11") == 0)
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_11.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31010", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "12") == 0)
    {

      EXEC SQL
          SELECT GET_XCHNG_SEQ_12.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31015", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "13") == 0)
    {

      EXEC SQL
          SELECT GET_XCHNG_SEQ_13.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31020", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "14") == 0)
    {

      EXEC SQL
          SELECT GET_XCHNG_SEQ_14.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31025", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "15") == 0)
    {

      EXEC SQL
          SELECT GET_XCHNG_SEQ_15.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31030", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    } /** Ver 1.2 add */
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "16") == 0)
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_16.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31035", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "17") == 0)
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_17.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31040", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "18") == 0) /**Ver 1.8 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_18.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31045", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "21") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_21.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31050", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "22") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_22.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31055", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "23") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_23.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31060", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "24") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_24.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31065", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "25") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_25.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31070", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "26") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_26.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31075", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "27") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_27.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31080", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "28") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_28.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31085", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "31") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_31.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31090", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "32") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_32.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31095", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A1") == 0) /*** Ver 1.6 ***/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_A1.NEXTVAL /* Changed seq name from 31 to A1 */
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31100", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A2") == 0) /*** Ver 1.7 ***/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_A2.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31105", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A3") == 0) /*** Ver 1.9 ***/
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_A3.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31110", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    /*** Ver 2.1 Starts ***/
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "99") == 0)
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_99.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31115", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A9") == 0)
    {
      EXEC SQL
          SELECT GET_XCHNG_SEQ_A9.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31120", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    /*** Ver 2.1 Ends ***/
    else
    {
      fn_errlog(c_ServiceName, "S31125", "Invalid Pipe", "Invalid pipe");
    }
    /** Ver 1.2 ends */

    /* EXEC SQL
           UPDATE  fxs_fo_xchng_seq
           SET     fxs_seq_num = fxs_seq_num + 1
           WHERE   fxs_pipe_id = :ptr_st_i_seq->c_pipe_id
           AND     fxs_trd_dt = to_date ( :c_trd_dt, 'dd-Mon-yyyy');

    if( SQLCODE != 0 )
    {
     fn_errlog ( c_ServiceName, "S31130", SQLMSG, ptr_st_err_msg->c_err_msg);
     fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
     tpreturn ( TPFAIL, ERR_BFR, ( char *)ptr_st_err_msg, 0, 0);
    } */

    /* Ver 1.1 Ends */
  }
  else if (ptr_st_i_seq->c_rqst_typ == GET_ORDER_SEQ)
  {
    
    if (strcmp(ptr_st_i_seq->c_pipe_id, "11") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_11.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31145", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "12") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_12.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31150", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "13") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_13.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31155", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "14") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_14.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31160", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "15") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_15.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31165", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "16") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_16.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31170", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "17") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_17.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31175", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "18") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_18.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31180", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "21") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_21.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31185", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "22") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_22.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31190", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "23") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_23.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31195", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "24") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_24.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31200", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "25") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_25.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31205", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "26") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_26.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31210", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "27") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_27.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31215", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "28") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_28.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31220", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "31") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_31.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31225", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "32") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_32.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31230", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A1") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_A1.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31235", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A2") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_A2.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31240", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A3") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_A3.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31245", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    /*** Ver 2.1 Starts ***/
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "99") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_99.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31250", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A9") == 0)
    {
      EXEC SQL
          SELECT GET_ORDR_SEQ_A9.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31255", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    /*** Ver 2.1 Ends  ***/
    else
    {
      fn_userlog(c_ServiceName, "Invalid pipe |%s|", ptr_st_i_seq->c_pipe_id);
      fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
    }
    /******** Ver 2.0 Ended *******/
  }
  /****** Ver 1.4 starts *******/
  /*else if ( ptr_st_i_seq->c_rqst_typ == GET_PLACED_SEQ)
  {

     EXEC SQL
            SELECT  fsp_seq_num + 1
            INTO    :ptr_st_i_seq->l_seq_num
            FROM    fsp_fo_seq_plcd
            WHERE   fsp_pipe_id = :ptr_st_i_seq->c_pipe_id
            AND     fsp_trd_dt = to_date ( :c_trd_dt, 'dd-Mon-yyyy');

     if( SQLCODE != 0 )
     {
      fn_errlog ( c_ServiceName, "S31260", SQLMSG, ptr_st_err_msg->c_err_msg);
      fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
      tpreturn ( TPFAIL, ERR_BFR, ( char *)ptr_st_err_msg, 0, 0);
     }

  }*/
  else if (ptr_st_i_seq->c_rqst_typ == GET_PLACED_SEQ)
  {
    EXEC SQL
        UPDATE fsp_fo_seq_plcd
            SET fsp_seq_num = fsp_seq_num + 1 WHERE fsp_pipe_id = : ptr_st_i_seq->c_pipe_id
                                                                        AND fsp_trd_dt = to_date( : c_trd_dt, 'dd-Mon-yyyy')
                                                                                             RETURNING fsp_seq_num INTO : ptr_st_i_seq->l_seq_num;

    if (SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "S31265", SQLMSG, ptr_st_err_msg->c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
    }
  }
  else if (ptr_st_i_seq->c_rqst_typ == RESET_PLACED_SEQ)
  {
    EXEC SQL
        UPDATE fsp_fo_seq_plcd
            SET fsp_seq_num = '0' WHERE fsp_pipe_id = : ptr_st_i_seq->c_pipe_id
                                                            AND fsp_trd_dt = to_date( : c_trd_dt, 'dd-Mon-yyyy')
                                                                                 RETURNING fsp_seq_num INTO : ptr_st_i_seq->l_seq_num;

    if (SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "S31270", SQLMSG, ptr_st_err_msg->c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
    }
  }
  /************* Ver 1.2 Ends ******************/
  else if (ptr_st_i_seq->c_rqst_typ == GET_SPRDORD_SEQ)
  {
    /******* Commented in Ver 2.3 for using database sequence instead of table sequence
         EXEC SQL
              SELECT  fss_seq_num + 1
              INTO    :ptr_st_i_seq->l_seq_num
              FROM    fss_fo_spdord_seq
              WHERE   fss_pipe_id = :ptr_st_i_seq->c_pipe_id
              AND     fss_trd_dt = to_date ( :c_trd_dt, 'dd-Mon-yyyy')
              FOR UPDATE OF fss_seq_num;

      if( SQLCODE != 0 )
      {
       fn_errlog ( c_ServiceName, "S31275", SQLMSG, ptr_st_err_msg->c_err_msg);
       tpfree ( ( char *) ptr_st_i_seq );
       fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
       tpreturn ( TPFAIL, ERR_BFR, ( char *)ptr_st_err_msg, 0, 0);
      }

         EXEC SQL
              UPDATE  fss_fo_spdord_seq
              SET     fss_seq_num = fss_seq_num + 1
              WHERE   fss_pipe_id = :ptr_st_i_seq->c_pipe_id
              AND     fss_trd_dt = to_date ( :c_trd_dt, 'dd-Mon-yyyy');

      if( SQLCODE != 0 )
      {
       fn_errlog ( c_ServiceName, "S31280", SQLMSG, ptr_st_err_msg->c_err_msg);
       fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
       tpreturn ( TPFAIL, ERR_BFR, ( char *)ptr_st_err_msg, 0, 0);
      }
      ***************** ver 2.3 ***********************************************************/
    /*** Added in ver 2.3 for database sequence ***/
    if (strcmp(ptr_st_i_seq->c_pipe_id, "11") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_11.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31285", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "12") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_12.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31290", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "13") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_13.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31295", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "14") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_14.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31300", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "15") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_15.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31305", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "16") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_16.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31310", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "17") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_17.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31315", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "18") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_18.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31320", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "21") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_21.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31325", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "22") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_22.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31330", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "23") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_23.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31335", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "24") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_24.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31340", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "25") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_25.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31345", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "26") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_26.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31350", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "27") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_27.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31355", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "28") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_28.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31360", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "31") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_31.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31365", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "32") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_32.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31370", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A1") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_A1.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31375", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A2") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_A2.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31380", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A3") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_A3.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31385", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "99") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_99.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31390", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A9") == 0)
    {
      EXEC SQL
          SELECT GET_SPDORD_SEQ_A9.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31395", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else
    {
      fn_errlog(c_ServiceName, "S31400", "Invalid Pipe", "Invalid pipe");
      fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
    }
    /******** Ver 2.3 Ended *******/
  }
  else if (ptr_st_i_seq->c_rqst_typ == GET_TRD_SEQ)
  {
    /***	Commented In Ver 1.5

         EXEC SQL
              SELECT  fts_seq_num + 1
              INTO    :ptr_st_i_seq->l_seq_num
              FROM    fts_fo_trd_seq
              WHERE   fts_pipe_id = :ptr_st_i_seq->c_pipe_id
              AND     fts_trd_dt = to_date ( :c_trd_dt, 'dd-Mon-yyyy') ;

      if( SQLCODE != 0 )
      {
        fn_errlog ( c_ServiceName, "S31405", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
        tpreturn ( TPFAIL, ERR_BFR, ( char *)ptr_st_err_msg, 0, 0);
      }

         EXEC SQL
              UPDATE  fts_fo_trd_seq
              SET     fts_seq_num = fts_seq_num + 1
              WHERE   fts_pipe_id = :ptr_st_i_seq->c_pipe_id
              AND     fts_trd_dt = to_date ( :c_trd_dt, 'dd-Mon-yyyy');
      if( SQLCODE != 0 )
      {
        fn_errlog ( c_ServiceName, "S31410", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran( c_ServiceName, i_trnsctn, c_errmsg );
        tpreturn ( TPFAIL, ERR_BFR, ( char *)ptr_st_err_msg, 0, 0);
      }

      Comment Ended ***/

    if (strcmp(ptr_st_i_seq->c_pipe_id, "11") == 0)
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_11.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31415", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "12") == 0)
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_12.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31420", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "13") == 0)
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_13.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31425", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "14") == 0)
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_14.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31430", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "15") == 0)
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_15.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31435", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "16") == 0)
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_16.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31440", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "17") == 0)
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_17.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31445", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "18") == 0) /**Ver 1.8 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_18.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31450", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "21") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_21.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31455", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "22") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_22.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31460", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "23") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_23.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31465", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "24") == 0) /**Ver 2.4 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_24.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31470", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "25") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_25.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31475", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "26") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_26.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31480", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "27") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_27.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31485", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "28") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_28.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31490", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "31") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_31.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31495", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "32") == 0) /**Ver 2.5 **/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_32.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31500", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A1") == 0) /*** Ver 1.6 ***/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_A1.NEXTVAL /* Changed seq name from 31 to A1*/
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31505", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A2") == 0) /*** Ver 1.7 ***/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_A2.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31510", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A3") == 0) /*** Ver 1.9 ***/
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_A3.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31515", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    /*** Ver 2.1 Starts ***/
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "99") == 0)
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_99.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31520", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    else if (strcmp(ptr_st_i_seq->c_pipe_id, "A9") == 0)
    {
      EXEC SQL
          SELECT GET_TRD_SEQ_A9.NEXTVAL
              INTO : ptr_st_i_seq->l_seq_num
                         FROM DUAL;

      if (SQLCODE != 0)
      {
        fn_errlog(c_ServiceName, "S31525", SQLMSG, ptr_st_err_msg->c_err_msg);
        fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
        tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
      }
    }
    /*** Ver 2.1 Ends	***/
  }
  else if (ptr_st_i_seq->c_rqst_typ == GET_ASGNMT_SEQ)
  {
    EXEC SQL
            SELECT fas_seq_num +
        1 INTO : ptr_st_i_seq->l_seq_num
                     FROM fas_fo_asg_seq
                         WHERE fas_trd_dt = to_date( : c_trd_dt, 'dd-Mon-yyyy')
        FOR UPDATE OF fas_seq_num;

    if (SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "S31530", SQLMSG, ptr_st_err_msg->c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
    }

    EXEC SQL
        UPDATE fas_fo_asg_seq
            SET fas_seq_num = fas_seq_num + 1 WHERE fas_trd_dt = to_date( : c_trd_dt, 'dd-Mon-yyyy');
    if (SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "S31535", SQLMSG, ptr_st_err_msg->c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
    }
  }
  else if (ptr_st_i_seq->c_rqst_typ == GET_TEMPORD_SEQ)
  {

    EXEC SQL
            SELECT fts_seq_num +
        1 INTO : ptr_st_i_seq->l_seq_num
                     FROM fts_fo_tmpord_seq
                         WHERE fts_trd_dt = to_date( : c_trd_dt, 'dd-Mon-yyyy')
        FOR UPDATE OF fts_seq_num;

    if (SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "S31540", SQLMSG, ptr_st_err_msg->c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
    }

    EXEC SQL
        UPDATE fts_fo_tmpord_seq
            SET fts_seq_num = fts_seq_num + 1 WHERE fts_trd_dt = to_date( : c_trd_dt, 'dd-Mon-yyyy');
    if (SQLCODE != 0)
    {
      fn_errlog(c_ServiceName, "S31545", SQLMSG, ptr_st_err_msg->c_err_msg);
      fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
      tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
    }
  }
  else
  {
    fn_errlog(c_ServiceName, "S31550", "Logic error",
              ptr_st_err_msg->c_err_msg);
    fn_aborttran(c_ServiceName, i_trnsctn, c_errmsg);
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
  }

  if (fn_committran(c_ServiceName, i_trnsctn,
                    ptr_st_err_msg->c_err_msg) == -1)
  {
    tpreturn(TPFAIL, ERR_BFR, (char *)ptr_st_err_msg, 0, 0);
  }

  tpfree((char *)ptr_st_err_msg);
  tpreturn(TPSUCCESS, SUCC_BFR, (char *)ptr_st_i_seq,
           sizeof(struct vw_sequence), 0);
}
