/******************************************************************************/
/*  Program           : SFO_BRDLZO_ROUT 	                                    */
/*                                                                            */
/*  Input             : C_PIPE_ID                                             */
/*                                                                            */
/*  Output            : This service decompress the broadcast packets and     */
/*                      and write the data in respective Unix Queues          */
/*                                                                            */
/******************************************************************************/
/*Ver 1.0             : New Release	| Sachin Birje.  													*/
/*Ver 1.1 27-Feb-2014 : Handling of INDIA VIX (CR_ISEC14_48665) Mahesh Shinde */
/*Ver 1.2 09-Sep-2014 : View to FML changes	:	 Samip M												*/
/*Ver 1.3 11-Dec-2014 : NNF:Trade execution range brdcast data	:	 Navina D.  */
/*Ver 1.4 22-May-2015 : Auto Forwarding changes  : Sachin Birje               */
/*Ver 1.5 25-Apr-2018 : Stream No. Exchange Exception Handling Changes	:	Parag Kanojia		*/
/*Ver 1.6 11-Jul-2018 : Write specific messages on other queue Varadraj G     */
/*Ver 1.7 14-Nov-2019 : Spread Rollover Changes   Sachin Birje                */
/*Ver 1.8 31-Mar-2020 : New processing Queues introduced |Sachin Birje        */
/******************************************************************************/

/**** C header ****/
#include <stdio.h>
#include <sqlca.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
/**** Tuxedo header ****/
#include <atmi.h>
#include <fml32.h>
#include <Usysflds.h>

/** #include <fo_view_def.h> **/	/** Commented for Ver 1.2 **/
#include <fo_fml_def.h>
#include <fml_def.h>
#include <fo.h>
#include <fn_tuxlib.h>
#include <fn_battmpt.h>
#include <fn_scklib_tap.h>
#include <fo_exg_brd_lib.h>
#include <fn_msgq.h>

/* Process specific header  */
#include <fn_env.h>
#include <fn_log.h>
#include <userlog.h>
#include <brd_cln_srvr.h>
#include <fn_fil_log.h>
#include <fn_ddr.h>

#include <string.h>
#include <stdlib.h>
#include <fml_rout.h>
#include <fn_signal.h>
#include <lzo1z.h>
#include <fn_read_debug_lvl.h>
#include <fo_shm.h>


void fn_procs_msgs ( char *c_bcast_data, 
                     char *c_ServiceName,
                     char *c_err_msg );

void fn_procs_msgs_1 ( char *c_bcast_data, 
                     	 char *c_ServiceName,
                     	 char *c_err_msg );
void fn_gettime(void);
char c_time[12];

char c_xchng_cd [ 3+1 ];
char c_pipe_id [ 5+1 ];
char c_brd_port[10];
char c_tbt_mod;	

int  i_dmp_val;
int	 i_lzo_ret_val;
int  i_sck_id;
int i_exit_stts;

LZO_EXTERN(int) i_lzo_ret_val;

long int li_port;
long l_temp_tot_count;

extern int i_nrml_qid;
extern int i_fi_qid;
extern int i_fs_qid;
extern int i_oi_qid;
extern int i_os_qid;
extern int i_othrs_qid;  /*** Added in ver 1.6 ***/
extern int i_os_qid_q;   /*** Added in ver 1.6 ***/
extern int i_othrs_qid1; /*** Added in Ver 1.8 ***/
extern int i_othrs_qid2; /*** Added in Ver 1.8 ***/
extern int i_othrs_qid3; /*** Added in Ver 1.8 ***/

extern struct st_tkn_val *st_tkn;

void fn_get_prd_typ(char *c_ServiceName, long l_search_item, char *c_prd_type);

void SFO_BRDLZO_ROUT ( TPSVCINFO *rqst )
{

  FBFR32 *ptr_fml_ibuf;
  FLDLEN32  ud_len;
	char c_ServiceName [33];
	char c_err_msg [256];
	char c_run_mod;
  char *ptr_car_sbuf;

	int	 i_ret_val;
  int i_errno;
  int i_err [ 7 ];
  int i_ferr [ 7 ];
  int i_cnt;
  int li_sbuf_len;

  TPINIT *ptr_st_tpinfo;

  struct st_bcast_pack_data st_bcastdata;

  struct st_brd_q_data st_brd_qdata;

  strcpy(c_ServiceName, "SFO_BRDLZO_ROUT" );
  INITDBGLVL(c_ServiceName);
  
  ptr_fml_ibuf = (FBFR32 *)rqst->data;

  strcpy(c_xchng_cd,"NFO");

  ptr_car_sbuf =(char *)tpalloc("CARRAY", NULL, MIN_FML_BUF_LEN);

  if (ptr_car_sbuf == NULL)
  {
    fn_errlog( c_ServiceName, "S31005", TPMSG, c_err_msg  );
    tpfree ( ( char * )ptr_fml_ibuf);
    tpreturn(TPFAIL, 0, (char *)NULL, 0, 0 );
  }

  i_ret_val = Fget32(ptr_fml_ibuf, FFO_TMPLT, 0,(char *)&li_sbuf_len, 0);

  if(i_ret_val == -1 )
  {
    fn_errlog(c_ServiceName,"S31010", DEF_USR,c_err_msg);
    tpfree ( ( char * )ptr_car_sbuf );
    tpfree ( ( char * )ptr_fml_ibuf);
    tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  }

  ud_len = (FLDLEN32)sizeof(struct st_bcast_pack_data); 
  i_ret_val = Fget32(ptr_fml_ibuf, FFO_CBUF, 0,(char *)ptr_car_sbuf, &ud_len);

  if (i_ret_val == -1)
  {
    fn_errlog(c_ServiceName,"S31015",DEF_USR,c_err_msg);
    tpfree ( ( char * )ptr_car_sbuf );
    tpfree ( ( char * )ptr_fml_ibuf);
    tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  }

  i_ret_val = Fget32(ptr_fml_ibuf, FFO_DWL_FLG, 0,(char *)&c_run_mod, 0);

  if (i_ret_val == -1)
  {
    fn_errlog(c_ServiceName,"S31020",DEF_USR,c_err_msg);
    tpfree ( ( char * )ptr_car_sbuf );
    tpfree ( ( char * )ptr_fml_ibuf);
    tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  }
  
  i_ret_val = Fget32(ptr_fml_ibuf, FFO_SOURCE_FLG, 0,(char *)&c_tbt_mod, 0);

  if (i_ret_val == -1)
  {
    fn_errlog(c_ServiceName,"S31025",DEF_USR,c_err_msg);
    tpfree ( ( char * )ptr_car_sbuf );
    tpfree ( ( char * )ptr_fml_ibuf);
    tpreturn ( TPFAIL, 0, (char *)NULL, 0, 0 );
  }

	if ( c_run_mod == WITH_QUOTES )
  {
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Inside RUN_MOD = WITH_QUOTES");
		}

    fn_procs_msgs ( ptr_car_sbuf,
                    c_ServiceName,
                    c_err_msg );
  }
  else
  {
		if(DEBUG_MSG_LVL_3)
		{
			fn_userlog(c_ServiceName,"Inside RUN_MOD = WITHOUT_QUOTES");
		}
    fn_procs_msgs_1 ( ptr_car_sbuf,
                      c_ServiceName,
                      c_err_msg );
  }

  tpfree ( ( char * )ptr_car_sbuf );
  tpreturn(TPSUCCESS, 0, (char *)NULL, 0, 0);	
}


void fn_procs_msgs ( char *c_bcast_data,
                     char *c_ServiceName,
                     char *c_err_msg )
{
	char c_serv_addr[20];
	char *ptr_c_data;
	char c_decmp_data[512];

	int i_family;
  int i_port_no;
	long int li_buf_len ;
	int i_ch_val;
	int i_ret_val;
	int i_msg_count;
	int i_ip_len;
  int i_q_qid;
  int i_ret_cd;

  int i_tmp;
  char c_qtype;

  long l_token_id;
  long l_rec_cnt;

	unsigned int i_op_len;
	int i_trnsctn_cd;
	unsigned short int si_err_cd;

	union st_exch_brd_msg st_exch_msg;
  union st_exch_brd_msg *ptr_exch_msg;
  struct st_bcast_pack_data st_bcastdata;
  struct st_bcast_cmp_packet *st_bcastcmp_packet;


  struct st_brdcst_msgs st_brdcst_sndq_msg;
  struct st_brd_q_data st_brd_qdata;
  struct st_eqoi_msg st_eqoi_data;


  MEMSET(st_brdcst_sndq_msg);
  MEMSET(st_brd_qdata);

	i_exit_stts = DONT_EXIT;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Inside Function fn_procs_msgs WITH_QUOTES");
	}

  memset ( &st_bcastdata, '\0', sizeof ( struct st_bcast_pack_data ) );
  memcpy ( &st_bcastdata, c_bcast_data, sizeof(st_bcastdata) );

  ptr_c_data = (char *) st_bcastdata.c_pack_data;

  if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName, "No of packets |%d|",st_bcastdata.si_no_of_packets);
	}

	for(i_msg_count=0;i_msg_count<st_bcastdata.si_no_of_packets;i_msg_count++)
	{
		i_ip_len = 0;

		st_bcastcmp_packet = (struct st_bcast_cmp_packet * )ptr_c_data;

		if(DEBUG_MSG_LVL_3)
		{
     	fn_userlog(c_ServiceName, "Compressed %d",st_bcastcmp_packet->si_comp_len);
		}

    if ( st_bcastcmp_packet->si_comp_len > 0 )
    {
      i_ip_len = st_bcastcmp_packet->si_comp_len;

      memset(c_decmp_data, '\0', sizeof(c_decmp_data));

      if(DEBUG_MSG_LVL_2)
      {
       fn_gettime();
       fn_userlog(c_ServiceName,"TIME_STATS NDF Before lzo1z_decompress at :%s:",c_time);
      }
 
      i_lzo_ret_val= lzo1z_decompress ( (char *)st_bcastcmp_packet->c_comp_data,
                      										i_ip_len,
                      										(char *)c_decmp_data,
                      										&i_op_len,
                      										NULL );

      ptr_exch_msg = ( union st_exch_brd_msg * )( (char *)c_decmp_data + 8 );

      ptr_c_data += ( sizeof(st_bcastcmp_packet->si_comp_len) + i_ip_len );
     
      if(DEBUG_MSG_LVL_2)
      {
       fn_gettime();
       fn_userlog(c_ServiceName,"TIME_STATS NDF After lzo1z_decompress at :%s:", c_time);
      }
    }
    else
    {
        ptr_exch_msg =  ( union st_exch_brd_msg * ) ( (char *)st_bcastcmp_packet->c_comp_data + 8 );

        ptr_c_data += ( sizeof(st_bcastcmp_packet->si_comp_len) + ((struct st_bcast_header *)ptr_exch_msg)->si_message_length + 8);
    }
  
   	if(DEBUG_MSG_LVL_3)
	  {
				fn_userlog(c_ServiceName, "Message received - |%d|",ptr_exch_msg->st_hdr.si_transaction_code);
				fn_userlog(c_ServiceName,"Before Switch Case");
		}

  	switch ( ptr_exch_msg->st_hdr.si_transaction_code )
		{
		case BCAST_JRNL_VCT_MSG:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_JRNL_VCT_MSG");
					fn_userlog(c_ServiceName,"Inside BCAST_JRNL_VCT_MSG Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_gnrltrd_msg at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg); 
        st_brdcst_sndq_msg.l_msg_typ = BCAST_JRNL_VCT_MSG;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_bcast_msg),&ptr_exch_msg->st_bcast_msg,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_bcast_msg));
       
        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31030", LIBMSG, c_err_msg);
         return;
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_gnrltrd_msg at :%s:",c_time);
        }
 
					break;

		case BCAST_SYSTEM_INFORMATION_OUT:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_SYSTEM_INFORMATION_OUT");
					fn_userlog(c_ServiceName,"Inside BCAST_SYSTEM_INFORMATION_OUT Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_system_information_out at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg); 
        st_brdcst_sndq_msg.l_msg_typ = BCAST_SYSTEM_INFORMATION_OUT;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_sys_info_dat),&ptr_exch_msg->st_sys_info_dat,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_sys_info_dat));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
         {
          fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
          fn_errlog(c_ServiceName, "S31035", LIBMSG, c_err_msg);
          return ;
         }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_system_information_out at :%s:",c_time);
        }

					break;

		case BCAST_SECURITY_MSTR_CHG:
				
				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_SECURITY_MSTR_CHG");
					fn_userlog(c_ServiceName,"Inside BCAST_SECURITY_MSTR_CHG Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_security_mstr_chg at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = BCAST_SECURITY_MSTR_CHG;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_sec_upd_info),&ptr_exch_msg->st_sec_upd_info,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_sec_upd_info));

        /*** if(fn_write_msg_q(i_nrml_qid, *****/
             if(fn_write_msg_q(i_othrs_qid1, /*** Ver 1.8, added in new msg queue for processing **/
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
         {
          fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
          fn_errlog(c_ServiceName, "S31040", LIBMSG, c_err_msg);
          return ;
         }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_security_mstr_chg at :%s:",c_time);
        }
				
					break;

		case BCAST_INSTR_MSTR_CHG:

					/*** Not Handled ***/

          break;
		
		case BCAST_PART_MSTR_CHG:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_PART_MSTR_CHG");
			  	fn_userlog(c_ServiceName,"Inside BCAST_PART_MSTR_CHG Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before  fn_part_mstr_chg at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = BCAST_PART_MSTR_CHG;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_part_upd_info),&ptr_exch_msg->st_part_upd_info,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_part_upd_info));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
         {
          fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
          fn_errlog(c_ServiceName, "S31045", LIBMSG, c_err_msg);
          return ;
         }

          if(DEBUG_MSG_LVL_2)
          {
           fn_gettime();
           fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_part_mstr_chg at :%s:",c_time);
          }

          break;

		case BCAST_STOCK_STATUS_CHG:
		case BCAST_STOCK_STATUS_CHG_PREOPEN:
	
				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case Stock Status Change");
					fn_userlog(c_ServiceName,"Inside BCAST_STOCK_STATUS_CHG Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before  fn_stock_status_chg at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = BCAST_STOCK_STATUS_CHG;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_sec_stts_upd_info),&ptr_exch_msg->st_sec_stts_upd_info,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_sec_stts_upd_info));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
         {
          fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
          fn_errlog(c_ServiceName, "S31050", LIBMSG, c_err_msg);
          return ;
         }
        
        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After  fn_stock_status_chg at :%s:",c_time);
        }
					break;

		case BCAST_TURNOVER_EXCEEDED:
		case BROADCAST_BROKER_REACTIVATED:

					if(DEBUG_MSG_LVL_1)
					{
						fn_userlog(c_ServiceName,"Inside Case Broker Activation / DeActivation");
						fn_userlog(c_ServiceName,"Inside BCAST_TURNOVER_EXCEEDED Exchange Code Is :%s:",c_xchng_cd);
					}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_turnover_exceeded at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);

        st_brdcst_sndq_msg.l_msg_typ = BCAST_TURNOVER_EXCEEDED;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_tlmt_exceed),&ptr_exch_msg->st_brd_tlmt_exceed,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_tlmt_exceed));

      /*** commented in Ver 1.6 *******
        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31055", LIBMSG, c_err_msg);
         return ;
        }
      ****** Ver 1.6 comment ends *****/
     /************* Added in Ver 1.6 *************/

        if(fn_write_msg_q(i_othrs_qid,
            (void *)&st_brdcst_sndq_msg,
             sizeof(st_brdcst_sndq_msg),
             c_ServiceName,
             c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31060", LIBMSG, c_err_msg);
         return ;
        }
     /*********** Ver 1.6 Ends *******************/

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_turnover_exceeded at :%s:",c_time);
        }
					break;

		case BC_OPEN_MSG:
		case BC_CLOSE_MSG:
		case BC_PRE_OR_POST_DAY_MSG:
		case BC_PRE_OPEN_ENDED:
		case EQUAL_BC_POSTCLOSE_MSG:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case Market status Change");  
					fn_userlog(c_ServiceName,"Inside BC_OPEN_MSG Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_brdmkt_stts_chng at :%s:",c_time);
        }

        MEMSET(st_brdcst_sndq_msg);
        /** st_brdcst_sndq_msg.l_msg_typ = BC_OPEN_MSG;  ** Ver 1.4 **/
        st_brdcst_sndq_msg.l_msg_typ = ptr_exch_msg->st_hdr.si_transaction_code; /** ver 1.4 **/ 
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_vct_msg),&ptr_exch_msg->st_brd_vct_msg,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_vct_msg));

        /********* commented in ver 1.6 *********************
        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31065", LIBMSG, c_err_msg);
         return ;
        } 

       ********************* Ver 1.6 comment ends ************/

       /************ Added in Ver 1.6 *********************/


        if(fn_write_msg_q(i_othrs_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31070", LIBMSG, c_err_msg);
         return ;
        }
       /****************** Ver 1.6 Ends ********************/
        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_brdmkt_stts_chng at :%s:",c_time);
        }
	 
					break;


		case BCAST_TICKER_AND_MKT_INDEX:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_TICKER_AND_MKT_INDEX");
					fn_userlog(c_ServiceName,"Inside BCAST_TICKER_AND_MKT_INDEX Exchange Code Is :%s:",c_xchng_cd);
				}
        
        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_ticker_and_mkt_index at :%s:",c_time);
        }
         MEMSET(st_brdcst_sndq_msg);

         st_brdcst_sndq_msg.l_msg_typ= BCAST_TICKER_AND_MKT_INDEX;       

         memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_ticker),&ptr_exch_msg->st_ticker,sizeof(ptr_exch_msg->st_ticker));

         /*** if(fn_write_msg_q(i_nrml_qid, *** commented in 1.8 **/
         if(fn_write_msg_q(i_othrs_qid2,   /*** Ver 1.8  added in new msg q for processing ***/
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
         {
          fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
          fn_errlog(c_ServiceName, "S31075", LIBMSG, c_err_msg);
          return ;
         }   

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_ticker_and_mkt_index at :%s:",c_time);
        }

					break;

		case BCAST_MBO_MBP_UPDATE:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_MBO_MBP_UPDATE");
					fn_userlog(c_ServiceName,"Inside BCAST_MBO_MBP_UPDATE Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_mbo_mbp_update at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);

        st_brdcst_sndq_msg.l_msg_typ= BCAST_MBO_MBP_UPDATE;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_mbo_mbp),&ptr_exch_msg->st_brd_mbo_mbp,sizeof(ptr_exch_msg->st_brd_mbo_mbp));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brd_qdata,
                    sizeof(st_brd_qdata),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "L31081", LIBMSG, c_err_msg);
         return ;
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_mbo_mbp_update at :%s:",c_time);
        }

				break;

		case BCAST_ONLY_MBP:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_ONLY_MBP");
					fn_userlog(c_ServiceName,"Inside BCAST_ONLY_MBP Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_bcast_only_mbp at :%s:",c_time);
        }

        for( i_tmp = 0; i_tmp<ptr_exch_msg->st_brd_only_mbp.si_no_of_records; i_tmp++ )
        {
          MEMSET(st_brd_qdata);
          l_token_id = ptr_exch_msg->st_brd_only_mbp.st_only_mbp[i_tmp].l_token;
          c_qtype='\0';
          fn_get_prd_typ(c_ServiceName,l_token_id,&c_qtype);

          switch(c_qtype)
          {
            case 'A' :
                      i_q_qid = i_fi_qid;

                      break;

            case 'B' :
                      i_q_qid = i_fs_qid ;
                      break;

            case 'C' :
                      i_q_qid = i_oi_qid;
                      break;

            case 'D' :
                      i_q_qid = i_os_qid;
                      break;
            case 'E' :                           /*** case added in ver 1.6 ***/
                      i_q_qid = i_os_qid_q;
                      break;

            default  :

                     fn_userlog(c_ServiceName,"Invalid CQtype:%c:",c_qtype);
                     fn_userlog(c_ServiceName,"l_token_id for Invalid CQtype:%c:",c_qtype);
                     continue; 
                     break;
          }

         st_brd_qdata.l_msg_typ= l_token_id;
         memcpy(&(st_brd_qdata.st_brd_data.st_brd_only_mbp.st_only_mbp[0]),&ptr_exch_msg->st_brd_only_mbp.st_only_mbp[i_tmp],sizeof(ptr_exch_msg->st_brd_only_mbp.st_only_mbp[0]));

         if(fn_write_msg_q(i_q_qid,
                    (void *)&st_brd_qdata,
                    sizeof(st_brd_qdata),
                    c_ServiceName,
                    c_err_msg) == -1)
         {
          fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
          fn_errlog(c_ServiceName, "L31082", LIBMSG, c_err_msg);
          return ;
         }

        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_bcast_only_mbp at :%s:",c_time);
        }

					break;

		case BCAST_MW_ROUND_ROBIN:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_MW_ROUND_ROBIN");
					fn_userlog(c_ServiceName,"Inside BCAST_MW_ROUND_ROBIN Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_mw_round_robin at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);   
        st_brdcst_sndq_msg.l_msg_typ = BCAST_MW_ROUND_ROBIN;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_inq_res),&ptr_exch_msg->st_brd_inq_res,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_inq_res));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31080", LIBMSG, c_err_msg);
         return ;
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_mw_round_robin at :%s:",c_time);
        }
					break;

		case SECURITY_OPEN_PRICE:
			
					 /*** Not Handled ***/
          break;

		case BCAST_CIRCUIT_MSG:
			
					/*** Ignored ***/
          break;

		case BCAST_INDICES:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_INDICES");
					fn_userlog(c_ServiceName,"Inside BCAST_INDICES Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_bcast_indices at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = BCAST_INDICES;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_indices),&ptr_exch_msg->st_brd_indices,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_indices));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31085", LIBMSG, c_err_msg);
         return ;
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_bcast_indices at :%s:",c_time);
        }

					break;

		case BCAST_INDUSTRY_INDEX_UPDATE:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_INDUSTRY_INDEX_UPDATE");
					fn_userlog(c_ServiceName,"Inside BCAST_INDUSTRY_INDEX_UPDATE Exchange Code Is :%s:",c_xchng_cd);
				}
	
        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_bcast_industry_index_update at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = BCAST_INDUSTRY_INDEX_UPDATE;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_indust_indices),&ptr_exch_msg->st_brd_indust_indices,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_indust_indices));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31090", LIBMSG, c_err_msg);
         return ;
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_bcast_industry_index_update at :%s:",c_time);
        }

					break;

		case BCAST_SPD_MBP_DELTA:

					 /*** Not Handled ***/
          /****** Ver 1.7 Starts here *******/
          if(DEBUG_MSG_LVL_1)
        {
          fn_userlog(c_ServiceName,"Inside Case BCAST_SPD_MBP_DELTA");
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_bcast_industry_index_update at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = BCAST_SPD_MBP_DELTA;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_spread_mkt_info),&ptr_exch_msg->st_spread_mkt_info,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_spread_mkt_info));

        /**** if(fn_write_msg_q(i_nrml_qid, *** commented in ver 1.8 ***/
        if(fn_write_msg_q(i_othrs_qid3,   /*** Added in ver 1.8, new message queue ***/
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31095", LIBMSG, c_err_msg);
         return ;
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After BCAST_SPD_MBP_DELTA at :%s:",c_time);
        }



          /***** Ver 1.7 Ends Here *******/


          break;

    /*** Ver 1.7 Starts Here ******/
    case BCAST_SPD_MSTR_CHG:
 
         if(DEBUG_MSG_LVL_1)
        {
          fn_userlog(c_ServiceName,"Inside Case BCAST_SPD_MSTR_CHG");
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before BCAST_SPD_MSTR_CHG :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = BCAST_SPD_MSTR_CHG;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_spd_update_info),&ptr_exch_msg->st_brd_spd_update_info,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_spd_update_info));

        /*** if(fn_write_msg_q(i_nrml_qid, ** comented in ver 1.8 ***/
        if(fn_write_msg_q(i_othrs_qid3,  /*** ver 1.8, added in new msg q for processing ***/
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31100", LIBMSG, c_err_msg);
         return ;
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After BCAST_SPD_MSTR_CHG at :%s:",c_time);
        }
   

 
         break;

    /*** Ver 1.7 Ends Here ***/
		case BCAST_INDEX_MSTR_CHG:
			
					/*** Not Handled ***/
          break;

		case BCAST_BASE_PRICE:

					 /*** Not Handled ***/
          break;

				case BCAST_INDEX_MAP_TABLE:

					/*** Not Handled ***/
          break;

		case RPRT_MARKET_STATS_OUT_RPT:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case RPRT_MARKET_STATS_OUT_RPT");
					fn_userlog(c_ServiceName,"Inside RPRT_MARKET_STATS_OUT_RPT Exchange Code Is :%s:",c_xchng_cd);
				}
        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_rprt_market_stats_out_rpt at :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = RPRT_MARKET_STATS_OUT_RPT;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_rp_mkt_stts),&ptr_exch_msg->st_rp_mkt_stts,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_rp_mkt_stts));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31105", LIBMSG, c_err_msg);
         return ;
        }
 
        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_rprt_market_stats_out_rpt at :%s:",c_time);
        }

					break;

		case MKT_MVMT_CM_OI_IN:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case MKT_MVMT_CM_OI_IN");
					fn_userlog(c_ServiceName,"Inside MKT_MVMT_CM_OI_IN Exchange Code Is :%s:",c_xchng_cd);
				}
         
        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_eq_oi_qt at :%s:",c_time);
        }

        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = MKT_MVMT_CM_OI_IN;
        /* memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg),&ptr_exch_msg,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg)); **/
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_eqoi_data.st_hdr),&ptr_exch_msg->st_hdr,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_eqoi_data.st_hdr));
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_eqoi_data.st_eq_oi),&ptr_exch_msg->st_eq_oi,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_eqoi_data.st_eq_oi));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31110", LIBMSG, c_err_msg);
         return ;
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_eq_oi_qt at :%s:",c_time);
        }
					break;

    case BCAST_GI_INDICES_ASSETS :
           if(DEBUG_MSG_LVL_1)
           {
            fn_userlog(c_ServiceName,"Inside Case BCAST_GI_INDICES_ASSETS");
            fn_userlog(c_ServiceName,"Inside BCAST_GI_INDICES_ASSETS Exchange Code Is :%s:",c_xchng_cd);
           }                
             
           if(DEBUG_MSG_LVL_2)
           {
            fn_gettime();
            fn_userlog(c_ServiceName,"TIME_STATS NDF Before fn_upd_global_indices at :%s:",c_time);
           }
         MEMSET(st_brdcst_sndq_msg);    
         st_brdcst_sndq_msg.l_msg_typ = BCAST_GI_INDICES_ASSETS;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_index_details),&ptr_exch_msg->st_index_details,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_index_details));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31115", LIBMSG, c_err_msg);
         return ;
        }
 
        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After fn_upd_global_indices at :%s:",c_time);
        }
        break;

			/**** Added in Ver 1.1 ****/
			
			case BCAST_INDICES_VIX :

           if(DEBUG_MSG_LVL_1)
           {
            	fn_userlog(c_ServiceName,"Inside Case BCAST_INDICES_VIX");
            	fn_userlog(c_ServiceName,"Inside BCAST_INDICES_VIX Exchange Code Is :%s:",c_xchng_cd);
           }

           if(DEBUG_MSG_LVL_2)
           {
            	fn_gettime();
            	fn_userlog(c_ServiceName,"TIME_STATS NDF Before India VIX  at :%s:",c_time);
           }

         MEMSET(st_brdcst_sndq_msg);

         st_brdcst_sndq_msg.l_msg_typ = BCAST_INDICES_VIX;

        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_indvix_dtls),&ptr_exch_msg->st_indvix_dtls,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_indvix_dtls));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         	fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         	fn_errlog(c_ServiceName, "S31120", LIBMSG, c_err_msg);
         	return ;
        }

        if(DEBUG_MSG_LVL_2)
        {
         	fn_gettime();
         	fn_userlog(c_ServiceName,"TIME_STATS NDF After India VIX at :%s:",c_time);
        }
        break;

			/**** Added in Ver 1.1 ****/

			/*** Ver 1.3 starts ***/
			case BCAST_TRADE_EXECUTION_RANGE:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case Bcast trade execution range");  
					fn_userlog(c_ServiceName,"Inside BCAST_TRADE_EXECUTION_RANGE Exchange Code Is :%s:",c_xchng_cd);
				}

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF at :%s:",c_time);
        }

        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = BCAST_TRADE_EXECUTION_RANGE;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_trd_exec_rng),&ptr_exch_msg->st_brd_trd_exec_rng,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_trd_exec_rng));

        if(fn_write_msg_q(i_nrml_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31125", LIBMSG, c_err_msg);
         return ;
        } 

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF at :%s:",c_time);
        }
	 
				break;
			/*** Ver 1.3 ends ***/

			/*** Ver 1.5 Starts ***/
        case BCAST_CONT_MSG:

        if(DEBUG_MSG_LVL_0)
        {
          fn_userlog(c_ServiceName,"Inside Case BCAST_CONT_MESSAGE(EXCEPTION)");
          fn_userlog(c_ServiceName,"Inside BCAST_CONT_MESSAGE Exchange Code Is :%s:",c_xchng_cd);
        }

        if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF Before is :%s:",c_time);
        }
        MEMSET(st_brdcst_sndq_msg);
        st_brdcst_sndq_msg.l_msg_typ = BCAST_CONT_MSG;
        memcpy(&(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_exch_exception_msg),&ptr_exch_msg->st_brd_exch_exception_msg,sizeof(st_brdcst_sndq_msg.st_xchng_brdmsg.st_brd_exch_exception_msg));

        if(fn_write_msg_q(i_othrs_qid,
                    (void *)&st_brdcst_sndq_msg,
                    sizeof(st_brdcst_sndq_msg),
                    c_ServiceName,
                    c_err_msg) == -1)
        {
         fn_userlog(c_ServiceName,"Failed to write in Transmit queque.");
         fn_errlog(c_ServiceName, "S31130", LIBMSG, c_err_msg);
         return ;
        }

				if(DEBUG_MSG_LVL_2)
        {
         fn_gettime();
         fn_userlog(c_ServiceName,"TIME_STATS NDF After is :%s:",c_time);
        }

        break;
        /*** Ver 1.5 Ends ***/

  	default:

					fn_userlog ( c_ServiceName, "Warning - Invalid message type - %d",ptr_exch_msg->st_hdr.si_transaction_code );
					break;
		}
	}
	
}

void fn_procs_msgs_1 ( char *c_bcast_data,
                       char *c_ServiceName,
                       char *c_err_msg )
{
	char c_serv_addr[20];
  char *ptr_c_data;
  char c_decmp_data[512];

  int i_family;
  int i_port_no;
  long int li_buf_len ;
  int i_ch_val;
	int i_ret_val;
  int i_msg_count;
  int i_ip_len;
  unsigned int i_op_len;
  unsigned short int si_err_cd;

  union st_exch_brd_msg st_exch_msg;
  union st_exch_brd_msg *ptr_exch_msg;
  struct st_bcast_pack_data st_bcastdata;
  struct st_bcast_cmp_packet *st_bcastcmp_packet;

  i_exit_stts = DONT_EXIT;

	if(DEBUG_MSG_LVL_3)
	{
		fn_userlog(c_ServiceName,"Inside Function fn_procs_msgs_1");
	}

  memset ( &st_bcastdata, '\0', sizeof ( struct st_bcast_pack_data ) );
  memcpy ( &st_bcastdata, c_bcast_data, sizeof(st_bcastdata) );

  ptr_c_data = (char *) st_bcastdata.c_pack_data;

	if(DEBUG_MSG_LVL_3)
	{
   	fn_userlog(c_ServiceName, "No of packets |%d|",st_bcastdata.si_no_of_packets);
	}

  for(i_msg_count=0;i_msg_count<st_bcastdata.si_no_of_packets;i_msg_count++)
  {
    i_ip_len = 0;

    st_bcastcmp_packet = (struct st_bcast_cmp_packet * )ptr_c_data;

  	if(DEBUG_MSG_LVL_3)
		{
     	fn_userlog(c_ServiceName, "Compressed %d",st_bcastcmp_packet->si_comp_len);
		}

    if ( st_bcastcmp_packet->si_comp_len > 0 )
    {
      i_ip_len = st_bcastcmp_packet->si_comp_len;

      memset(c_decmp_data, '\0', sizeof(c_decmp_data));

      i_lzo_ret_val = lzo1z_decompress ( (char *)st_bcastcmp_packet->c_comp_data,
                      										 i_ip_len,
                      										 (char *)c_decmp_data,
                      										 &i_op_len,
                      										 NULL);

      ptr_exch_msg = ( union st_exch_brd_msg * )( (char *)c_decmp_data + 8 );

      ptr_c_data += ( sizeof(st_bcastcmp_packet->si_comp_len) + i_ip_len );
    }
    else
    {
      ptr_exch_msg =  ( union st_exch_brd_msg * ) ( (char *)st_bcastcmp_packet->c_comp_data + 8 );

      ptr_c_data += ( sizeof(st_bcastcmp_packet->si_comp_len) + ((struct st_bcast_header *)ptr_exch_msg)->si_message_length + 8);
    }

   	if(DEBUG_MSG_LVL_3)
	  {
    	fn_userlog(c_ServiceName, "Message received - |%d|",ptr_exch_msg->st_hdr.si_transaction_code);
		}

	  	switch ( ptr_exch_msg->st_hdr.si_transaction_code )
      {
        case BCAST_JRNL_VCT_MSG:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case BCAST_JRNL_VCT_MSG");
				}
		
				i_ret_val = fn_gnrltrd_msg( &(ptr_exch_msg->st_bcast_msg),
                                    c_xchng_cd,
																		c_pipe_id,
                                    c_ServiceName,
                                    c_err_msg);

        if ( i_ret_val == -1 )
        {
          i_exit_stts = EXIT;
        }

          break;

        case BCAST_SYSTEM_INFORMATION_OUT:

					 /*** Not Handled ***/
          break;

        case BCAST_SECURITY_MSTR_CHG:

					 /*** Not Handled ***/
          break;

        case BCAST_INSTR_MSTR_CHG:

          /*** Not Handled ***/

          break;

        case BCAST_PART_MSTR_CHG:
	
					 /*** Not Handled ***/
          break;

        case BCAST_STOCK_STATUS_CHG:
        case BCAST_STOCK_STATUS_CHG_PREOPEN:

					 /*** Not Handled ***/
          break;

        case BCAST_TURNOVER_EXCEEDED:
        case BROADCAST_BROKER_REACTIVATED:

					 /*** Not Handled ***/
          break;

        case BC_OPEN_MSG:
        case BC_CLOSE_MSG:
        case BC_PRE_OR_POST_DAY_MSG:
        case BC_PRE_OPEN_ENDED:
        case EQUAL_BC_POSTCLOSE_MSG:

				if(DEBUG_MSG_LVL_1)
				{
					fn_userlog(c_ServiceName,"Inside Case Market status Change");
				}

				i_ret_val = fn_brdmkt_stts_chng ( &(ptr_exch_msg->st_brd_vct_msg),
                          	               c_xchng_cd,
																					 c_pipe_id,
                              	           c_ServiceName,
                                	         c_err_msg);
          if ( i_ret_val == -1 )
          {
            i_exit_stts = EXIT;
          }

					
          break;

				case BCAST_TICKER_AND_MKT_INDEX:

					 /*** Not Handled ***/
          break;

        case BCAST_MBO_MBP_UPDATE:

					 /*** Not Handled ***/
          break;

        case BCAST_ONLY_MBP:

					 /*** Not Handled ***/
          break;

        case BCAST_MW_ROUND_ROBIN:

					 /*** Not Handled ***/
          break;

        case SECURITY_OPEN_PRICE:

           /*** Not Handled ***/
          break;

        case BCAST_CIRCUIT_MSG:

          /*** Ignored ***/
          break;

        case BCAST_INDICES:

					 /*** Not Handled ***/
          break;

        case BCAST_INDUSTRY_INDEX_UPDATE:

					 /*** Not Handled ***/
          break;

        case BCAST_SPD_MBP_DELTA:

           /*** Not Handled ***/
          break;

        case BCAST_INDEX_MSTR_CHG:

          /*** Not Handled ***/
          break;

        case BCAST_BASE_PRICE:

           /*** Not Handled ***/
          break;

				case BCAST_INDEX_MAP_TABLE:

          /*** Not Handled ***/
          break;

        case RPRT_MARKET_STATS_OUT_RPT:

					 /*** Not Handled ***/
          break;

        case MKT_MVMT_CM_OI_IN:

					 /*** Not Handled ***/
          break;

        default:

          fn_userlog ( c_ServiceName, "Warning - Invalid message type - %d",ptr_exch_msg->st_hdr.si_transaction_code );
          break;
      }

		}

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

void fn_get_prd_typ(char *c_ServiceName,long l_search_item, char *c_prd_type)
{
long  first;
long  last;
long  middle;

      first = 1;
      last = st_tkn[0].l_tkn_id;
      middle =(first+last)/2;

      while( first <= last)
      {
        if ( st_tkn[middle].l_tkn_id < l_search_item)
            first = middle + 1;
        else if ( st_tkn[middle].l_tkn_id == l_search_item)
        {
          if(DEBUG_MSG_LVL_1)
          {
           fn_userlog(c_ServiceName,"Element found");
          }

          *c_prd_type = st_tkn[middle].c_val;
          return;
        }
        else
        last = middle - 1;

        middle = (first + last)/2;
      }
      if ( first > last )
      fn_userlog(c_ServiceName,"Not found! %ld is not present in the list.", l_search_item);
}

