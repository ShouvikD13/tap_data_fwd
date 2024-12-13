/***** Ver 1.1 : SLTP Changes as per Modified structure : Sachin Birje *****/
/***** Ver 1.2 : RRM and Broker Suspension Message Handling :Sandip Tambe***/
/***** Ver 1.3 : BSE derivative new IML changes             :Sandip Tambe***/

#include <stdio.h>
#include <time.h>
#include <memory.h>
#include <fn_fo_bse_fprint.h>
#include <fo_bse_exch_comnN.h>
#include <fn_log.h>
#include <fn_fo_bse_convrtn.h>
#include <netinet/in.h>
#include <fn_fobse_bcast.h>				/*** Ver 1.2 **/

#define MAX_SCK_MSG      1024



/***********************************************************************************/
/* This function writes a blank padded string to file terminating it with new line */
/* INPUT PARAMETERS                                                                */
/*    fptr      - File handler to which data is to be written                      */
/*    c_const   - Field description                                                */
/*    c_ptr     - Blank padded string                                              */
/*    i_len     - Actual data length in the blank padded string                    */
/* OUTPUT PARAMETERS                                                               */
/*          None                                                                   */
/***********************************************************************************/

void fn_bfprint_string(FILE *fptr,
                      char *c_const,
                      char *c_ptr,
                      int  i_len)
{
  int i_loop;

  fprintf(fptr, "%s:", c_const);
  for(i_loop = 0; i_loop < i_len; i_loop++)
  {
    fprintf(fptr, "%c", c_ptr[i_loop]);
  }
  fprintf(fptr, ":\n");
}




/******************************************************************************/
/* This function takes char buffer as input and depending on the message type */
/* maps them to respective structures and writes the data to the file pointed */
/* to by the file pointer.                                                    */
/* INPUT PARAMETERS                                                           */
/*      fptr         - File handler                                           */
/*      li_msg_typ   - Message Type                                           */
/*      c_msg_data   - Structure to be printed                                */
/*      c_ServiceName- Name of the service                                    */
/*      c_errmsg     - Error String                                           */
/* OUTPUT PARAMETERS                                                          */
/*          0 - Success                                                       */
/******************************************************************************/
int fn_bfprint_log(FILE *fptr,
                  char c_iml_header,
                  void *c_msg_data,
                  char *c_ServiceName,
                  char *c_errmsg)
{
  char      c_dname[22];
  char      c_start[30];   /* VER TOL : TUX on LINUX -- Size chnaged from 20 to 30 */
  char      c_end[30];     /* VER TOL : TUX on LINUX -- Size chnaged from 15 to 30 */
  time_t    tm_stmp;
  short int si_err_code;
  int       i_loop = 0;
  long int  li_msg_typ;
  char      c_time_str[22];
  long long ll_timestamp1;
  long long ll_timestamp2;
  long long ll_jiffy;


  struct st_header st_sndhdr;
 
/***
  struct st_send_rqst_data st_rqst_data;
  struct st_bfo_usr_rgstrn_req st_rgstr_rqst;
  struct st_bfo_logon_req st_logon_rqst;
  struct st_bfo_logon_req st_logoff_rqst;
  ****/
  tm_stmp = time(NULL);
  strftime(c_dname, 21, "%d-%b-%Y %H:%M:%S", localtime(&tm_stmp));
  fprintf(fptr ,"==========================================================================================\n");

  fprintf(fptr,"\t\t Timestamp :%s:\n", c_dname);
  fprintf(fptr ,"-----------------------------------------------------------\n");

  memset(&st_sndhdr, 0, sizeof(st_sndhdr));
  memcpy(&st_sndhdr, c_msg_data, sizeof(st_sndhdr));
  /***************************************************/
  st_sndhdr.l_slot_no         = ntohl(st_sndhdr.l_slot_no);
  st_sndhdr.l_message_length  = ntohl(st_sndhdr.l_message_length);
  st_sndhdr.l_msg_type        = ntohl(st_sndhdr.l_msg_type);

  st_sndhdr.l_slot_no         = fn_swap_long(st_sndhdr.l_slot_no);
  st_sndhdr.l_message_length  = fn_swap_long(st_sndhdr.l_message_length);
  st_sndhdr.l_msg_type        = fn_swap_long(st_sndhdr.l_msg_type);  
  /***************************************************/
  
  fprintf(fptr  , "\t\t HEADER  ::\n");
  fprintf(fptr, "%-35s=:%ld:\n", "Slot No", st_sndhdr.l_slot_no);
  fprintf(fptr, "%-35s=:%ld:\n", "Message Length",st_sndhdr.l_message_length);
  fprintf(fptr ,"-----------------------------------------------------------\n");
  fprintf(fptr  , "\t\t DATA   ::\n");

  strcpy(c_start, "\t\t Printing New packet");
  strcpy(c_end, "\t\t Packet Printed");

  fprintf(fptr, "%s\n", c_start);

     /**  memset(&st_rqst_data,0,sizeof(st_rqst_data));
       memcpy(&st_rqst_data,	c_msg_data , sizeof(c_msg_data)); ***/
       
       li_msg_typ = st_sndhdr.l_msg_type;;
       if(li_msg_typ == 99 )
         li_msg_typ = 0;

       switch(li_msg_typ)
       {

         case USR_RGSTRN_REQ :
           { 
               struct st_bfo_usr_rgstrn_req st_rgstr_rqst; 

                 memset(&st_rgstr_rqst,0,sizeof(st_rgstr_rqst));
                 memcpy(&st_rgstr_rqst,(struct st_rgstr_rqst *)c_msg_data, sizeof(st_rgstr_rqst));
               
                 st_rgstr_rqst.l_msg_typ = ntohl(st_rgstr_rqst.l_msg_typ);
                 st_rgstr_rqst.l_slot_no = ntohl(st_rgstr_rqst.l_slot_no);
 
                 st_rgstr_rqst.l_msg_typ = fn_swap_long(st_rgstr_rqst.l_msg_typ);
                 st_rgstr_rqst.l_slot_no = fn_swap_long(st_rgstr_rqst.l_slot_no);
                 st_rgstr_rqst.l_member_id = fn_swap_long(st_rgstr_rqst.l_member_id);
                 st_rgstr_rqst.l_trader_id = fn_swap_long(st_rgstr_rqst.l_trader_id);
                
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Type",st_rgstr_rqst.l_msg_typ);
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Slot No     ",st_rgstr_rqst.l_slot_no);
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Member Id   ",st_rgstr_rqst.l_member_id);
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Trader Id  ",st_rgstr_rqst.l_trader_id);
                 fprintf(fptr, "\n%s\n\n", c_end);
                 fflush(fptr);
                 break;
           }
        case LOGON_REQ :
           {       
                 struct st_bfo_logon_req st_logon_rqst; 
                 memset(&st_logon_rqst,0,sizeof(st_logon_rqst));
                 memcpy(&st_logon_rqst,(struct st_logon_rqst*)c_msg_data, sizeof(st_logon_rqst));
 
                 st_logon_rqst.l_msg_typ = fn_swap_long(st_logon_rqst.l_msg_typ);
                 st_logon_rqst.l_msg_tag = fn_swap_long(st_logon_rqst.l_msg_tag);

                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Type",st_logon_rqst.l_msg_typ);
                 fn_bfprint_string(fptr,"\t\t Password      =",st_logon_rqst.c_password,sizeof(st_logon_rqst.c_password));
                 fprintf(fptr,"%-35s=:%c:\n", "\t\t Filler ",st_logon_rqst.c_filler); 
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag ",st_logon_rqst.l_msg_tag);
                 fprintf(fptr, "\n%s\n\n", c_end);
                 fflush(fptr);
                 break;
           }
        case LOGOFF_REQ :
          {  
                 struct st_bfo_logon_req st_logoff_rqst; 
                 memset(&st_logoff_rqst,0,sizeof(st_logoff_rqst));
                 memcpy(&st_logoff_rqst,(struct st_logoff_rqst*)c_msg_data, sizeof(st_logoff_rqst));
                 st_logoff_rqst.l_msg_typ = fn_swap_long(st_logoff_rqst.l_msg_typ); 
                 st_logoff_rqst.l_msg_tag = fn_swap_long(st_logoff_rqst.l_msg_tag);           
                 fprintf(fptr,"%-35s=:%ld:\n", " Message Type",st_logoff_rqst.l_msg_typ);
                 fn_bfprint_string(fptr,"Password =",st_logoff_rqst.c_password,sizeof(st_logoff_rqst.c_password));
                 fprintf(fptr,"%-35s=:%c:\n", "Filler ",st_logoff_rqst.c_filler);
                 fprintf(fptr,"%-35s=:%ld:\n", " Msg Tag ",st_logoff_rqst.l_msg_tag);
                 fprintf(fptr, "\n%s\n\n", c_end);
                 fflush(fptr);
                 break;
          }
         case MANDATORY_PASSWD_CHNG :
         case OPTNL_PASSWD_CHNG :
          {
               struct st_bfo_upd_passwd_req st_chng_passwd_reqst;
               memset(&st_chng_passwd_reqst,0,sizeof(st_chng_passwd_reqst));
               memcpy(&st_chng_passwd_reqst,(struct st_bfo_upd_passwd_req*)c_msg_data, sizeof(st_chng_passwd_reqst));
               st_chng_passwd_reqst.l_msg_typ = fn_swap_long(st_chng_passwd_reqst.l_msg_typ);
               st_chng_passwd_reqst.l_msg_tag = fn_swap_long(st_chng_passwd_reqst.l_msg_tag);

               fprintf(fptr,"%-35s=:%ld:\n", " Message Type",st_chng_passwd_reqst.l_msg_typ);
            fn_bfprint_string(fptr,"\t\t Old Password  =",st_chng_passwd_reqst.c_old_pwd,sizeof(st_chng_passwd_reqst.c_old_pwd));                    
             fn_bfprint_string(fptr,"\t\t New Password  =",st_chng_passwd_reqst.c_new_pwd,sizeof(st_chng_passwd_reqst.c_new_pwd));
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag ",st_chng_passwd_reqst.l_msg_tag); 
             fprintf(fptr, "\n%s\n\n", c_end);
             fflush(fptr);
             break; 
          }
         case PRSNL_ORDR_DWNLD:
         case PRSNL_TRD_DWNLD:
         case PRSN_SLTP_ORDR_DWNLD:
          {
              struct st_prsnl_dwnld st_prsnl_dwnld_rqst;
              memset(&st_prsnl_dwnld_rqst,0,sizeof(st_prsnl_dwnld_rqst));
              memcpy(&st_prsnl_dwnld_rqst,(struct st_prsnl_dwnld*)c_msg_data,sizeof(st_prsnl_dwnld_rqst));
              st_prsnl_dwnld_rqst.l_msg_typ = fn_swap_long(st_prsnl_dwnld_rqst.l_msg_typ);
              st_prsnl_dwnld_rqst.l_msg_tag = fn_swap_long(st_prsnl_dwnld_rqst.l_msg_tag);
              
              fprintf(fptr,"%-35s=:%ld:\n", " Message Type",st_prsnl_dwnld_rqst.l_msg_typ);
              fprintf(fptr,"%-35s=:%d:\n", " Hour ",st_prsnl_dwnld_rqst.st_dwnld_time.c_hour);
              fprintf(fptr,"%-35s=:%d:\n", " Minute",st_prsnl_dwnld_rqst.st_dwnld_time.c_min);
              fprintf(fptr,"%-35s=:%d:\n", " Seconds",st_prsnl_dwnld_rqst.st_dwnld_time.c_sec);
              fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag ",st_prsnl_dwnld_rqst.l_msg_tag);
              fprintf(fptr, "\n%s\n\n", c_end);
              fflush(fptr);
              break; 
           } 
         case LIMIT_MARKET_ORD:
           {
             struct st_ord_rqst st_lmtmkrt_ord;
             memset(&st_lmtmkrt_ord,0,sizeof(struct st_ord_rqst));
             memcpy(&st_lmtmkrt_ord,(struct st_ord_rqst*)c_msg_data,sizeof(st_lmtmkrt_ord));
             st_lmtmkrt_ord.l_msg_typ = fn_swap_long(st_lmtmkrt_ord.l_msg_typ);
             st_lmtmkrt_ord.l_scrip_cd = fn_swap_long(st_lmtmkrt_ord.l_scrip_cd);
             st_lmtmkrt_ord.l_msg_tag1 = fn_swap_long(st_lmtmkrt_ord.l_msg_tag1);
             st_lmtmkrt_ord.l_qty      = fn_swap_long(st_lmtmkrt_ord.l_qty);
             st_lmtmkrt_ord.l_revealed_qty = fn_swap_long(st_lmtmkrt_ord.l_revealed_qty);
             st_lmtmkrt_ord.l_rate = fn_swap_long(st_lmtmkrt_ord.l_rate);
             st_lmtmkrt_ord.l_trigr_rate = fn_swap_long(st_lmtmkrt_ord.l_trigr_rate); /** Ver 1.1 ***/
             st_lmtmkrt_ord.l_resrvd_fld2 = fn_swap_long(st_lmtmkrt_ord.l_resrvd_fld2);
             st_lmtmkrt_ord.l_resrvd_fld3 = fn_swap_long(st_lmtmkrt_ord.l_resrvd_fld3);
             st_lmtmkrt_ord.l_filler1  = fn_swap_long(st_lmtmkrt_ord.l_filler1);
             st_lmtmkrt_ord.l_filler2 = fn_swap_long(st_lmtmkrt_ord.l_filler2);
             st_lmtmkrt_ord.l_msg_tag2 = fn_swap_long(st_lmtmkrt_ord.l_msg_tag2);
             st_lmtmkrt_ord.ll_ordr_id = fn_swap_ulnglng(st_lmtmkrt_ord.ll_ordr_id);       
             st_lmtmkrt_ord.ll_location_id = fn_swap_ulnglng(st_lmtmkrt_ord.ll_location_id);
             st_lmtmkrt_ord.l_filler3 = fn_swap_long(st_lmtmkrt_ord.l_filler3);
             st_lmtmkrt_ord.s_filler4 = fn_swap_long(st_lmtmkrt_ord.s_filler4);
             st_lmtmkrt_ord.s_filler5 = fn_swap_long(st_lmtmkrt_ord.s_filler5);
             st_lmtmkrt_ord.s_client_typ  = fn_swap_short(st_lmtmkrt_ord.s_client_typ);
             st_lmtmkrt_ord.s_retention = fn_swap_short(st_lmtmkrt_ord.s_retention);

             fprintf(fptr,"%-35s=:%ld:\n", " Message Type",st_lmtmkrt_ord.l_msg_typ);
             fprintf(fptr,"%-35s=:%ld:\n", " Scrip Code ",st_lmtmkrt_ord.l_scrip_cd);
             fprintf(fptr,"%-35s=:%ld:\n", " MSG TAG1 ",st_lmtmkrt_ord.l_msg_tag1);
             fprintf(fptr,"%-35s=:%ld:\n", " Quantity ",st_lmtmkrt_ord.l_qty);
             fprintf(fptr,"%-35s=:%ld:\n", " Releaved Qty ",st_lmtmkrt_ord.l_revealed_qty);
             fprintf(fptr,"%-35s=:%ld:\n", " Rate ",st_lmtmkrt_ord.l_rate);
             fprintf(fptr,"%-35s=:%ld:\n", " TRIG RATE  ",st_lmtmkrt_ord.l_trigr_rate);  /** Ver 1.1 ***/
             fprintf(fptr,"%-35s=:%ld:\n", " Resrvd Field2 ",st_lmtmkrt_ord.l_resrvd_fld2);
             fprintf(fptr,"%-35s=:%ld:\n", " Resrvd Field3 ",st_lmtmkrt_ord.l_resrvd_fld3);
             fprintf(fptr,"%-35s=:%ld:\n", " Filler1 ",st_lmtmkrt_ord.l_filler1);
             fprintf(fptr,"%-35s=:%ld:\n", " Filler2 ",st_lmtmkrt_ord.l_filler2);
             fprintf(fptr,"%-35s=:%ld:\n", " MSG TAG2 ",st_lmtmkrt_ord.l_msg_tag2);  
             fprintf(fptr,"%-35s=:%lld:\n"," Order Id ",st_lmtmkrt_ord.ll_ordr_id);
             fprintf(fptr,"%-35s=:%lld:\n"," Location ID ",st_lmtmkrt_ord.ll_location_id);
             fprintf(fptr,"%-35s=:%ld:\n", " Filler3 ",st_lmtmkrt_ord.l_filler3);
             fprintf(fptr,"%-35s=:%d:\n", " Filler4 ",st_lmtmkrt_ord.s_filler4);
             fprintf(fptr,"%-35s=:%d:\n", " Filler5 ",st_lmtmkrt_ord.s_filler5);
             fn_bfprint_string(fptr," Client Id   \t\t\t=",st_lmtmkrt_ord.c_client_id,sizeof(st_lmtmkrt_ord.c_client_id));           
             fn_bfprint_string(fptr," RESRVD Filed \t\t\t =",st_lmtmkrt_ord.c_resrvd_fld,sizeof(st_lmtmkrt_ord.c_resrvd_fld));        
             fn_bfprint_string(fptr," C_Msg_Tag  \t\t\t =",st_lmtmkrt_ord.c_msg_tag,sizeof(st_lmtmkrt_ord.c_msg_tag));
             fn_bfprint_string(fptr," C_Filler6   \t\t\t=",st_lmtmkrt_ord.c_filler6,sizeof(st_lmtmkrt_ord.c_filler6));
             fprintf(fptr,"%-35s=:%c:\n", " Action Code   ",st_lmtmkrt_ord.c_ordr_actn_cd);
             fprintf(fptr,"%-35s=:%c:\n", " Buy / Sell   ",st_lmtmkrt_ord.c_ordr_buysell_flg);
             fprintf(fptr,"%-35s=:%c:\n", " Order Type    ",st_lmtmkrt_ord.c_ord_typ);
             fprintf(fptr,"%-35s=:%c:\n", " C_Filler7    ",st_lmtmkrt_ord.c_filler7);
             fprintf(fptr,"%-35s=:%d:\n", " Client Type",st_lmtmkrt_ord.s_client_typ);
             fprintf(fptr,"%-35s=:%d:\n", " Market Protection ",st_lmtmkrt_ord.s_mrkt_protection);
             fprintf(fptr,"%-35s=:%d:\n", " Retention ",st_lmtmkrt_ord.s_retention);
             fprintf(fptr,"%-35s=:%d:\n", " s_filler8",st_lmtmkrt_ord.s_filler8);            
    
            fprintf(fptr, "\n%s\n\n", c_end);
              fflush(fptr);
              break;    

             
           }
        case ADD_IOC_BUY_ORDR :
        case ADD_IOC_SELL_ORDR:
          {
            struct st_ioc_ordr_rqst st_ioc_ordr_request;
            memset(&st_ioc_ordr_request,0,sizeof(struct st_ioc_ordr_rqst));
            memcpy(&st_ioc_ordr_request,(struct st_ioc_ordr_rqst*)c_msg_data,sizeof(st_ioc_ordr_request));
            st_ioc_ordr_request.l_msg_typ = fn_swap_long(st_ioc_ordr_request.l_msg_typ);
            st_ioc_ordr_request.l_scrip_cd = fn_swap_long(st_ioc_ordr_request.l_scrip_cd);
            st_ioc_ordr_request.l_msg_tag = fn_swap_long(st_ioc_ordr_request.l_msg_tag);
            st_ioc_ordr_request.l_qty = fn_swap_long(st_ioc_ordr_request.l_qty);
            st_ioc_ordr_request.l_avaqty = fn_swap_long(st_ioc_ordr_request.l_avaqty);
            st_ioc_ordr_request.l_rate = fn_swap_long(st_ioc_ordr_request.l_rate);
            st_ioc_ordr_request.ll_ordr_id = fn_swap_ulnglng(st_ioc_ordr_request.ll_ordr_id);
            st_ioc_ordr_request.s_retention = fn_swap_short(st_ioc_ordr_request.s_retention);
            st_ioc_ordr_request.l_miniqty = fn_swap_long(st_ioc_ordr_request.l_miniqty);
            st_ioc_ordr_request.l_cust_cd = fn_swap_long(st_ioc_ordr_request.l_cust_cd);
            st_ioc_ordr_request.s_caclass = fn_swap_short(st_ioc_ordr_request.s_caclass);
            st_ioc_ordr_request.s_mrkt_protection = fn_swap_short(st_ioc_ordr_request.s_mrkt_protection);
            st_ioc_ordr_request.ll_location_id = fn_swap_ulnglng(st_ioc_ordr_request.ll_location_id);
           
            fprintf(fptr,"%-35s=:%ld:\n", " Message Type",st_ioc_ordr_request.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", " Scrip Code ",st_ioc_ordr_request.l_scrip_cd);
            fprintf(fptr,"%-35s=:%ld:\n", " Msg Tag    ",st_ioc_ordr_request.l_msg_tag);
            fprintf(fptr,"%-35s=:%ld:\n", " Quantity   ",st_ioc_ordr_request.l_qty);
            fprintf(fptr,"%-35s=:%ld:\n", " Available Qty",st_ioc_ordr_request.l_avaqty);
            fprintf(fptr,"%-35s=:%ld:\n", " Rate       ",st_ioc_ordr_request.l_rate);
            fprintf(fptr,"%-35s=:%lld:\n", " Order Id   ",st_ioc_ordr_request.ll_ordr_id);
            fn_bfprint_string(fptr," Client Id   \t\t\t=",st_ioc_ordr_request.c_client_id,sizeof(st_ioc_ordr_request.c_client_id));
            fprintf(fptr,"%-35s=:%d:\n", " Retension  ",st_ioc_ordr_request.s_retention);
            fprintf(fptr,"%-35s=:%ld:\n", " Mini Qty   ",st_ioc_ordr_request.l_miniqty);
            fprintf(fptr,"%-35s=:%ld:\n", " Cust Id    ",st_ioc_ordr_request.l_cust_cd);                
            fprintf(fptr,"%-35s=:%d:\n", " CAClass    ",st_ioc_ordr_request.s_caclass);
            fprintf(fptr,"%-35s=:%d:\n", " Market Protection ",st_ioc_ordr_request.s_mrkt_protection);
            fprintf(fptr,"%-35s=:%lld:\n", " Location Id ",st_ioc_ordr_request.ll_location_id);

            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break;
          }
       case ADD_STOPLOSS_BUY_ORD:
       case UPD_STOPLOSS_BUY_ORD:
       case ADD_STOPLOSS_SELL_ORD:
       case UPD_STOPLOSS_SELL_ORD:
          {
            struct st_stoploss_ord_rqst st_stoploss_ord_request;
            memset(&st_stoploss_ord_request,0,sizeof(struct st_stoploss_ord_rqst));
            memcpy(&st_stoploss_ord_request,(struct st_stoploss_ord_rqst*)c_msg_data,sizeof(st_stoploss_ord_request));
            st_stoploss_ord_request.l_msg_typ  = fn_swap_long(st_stoploss_ord_request.l_msg_typ);
            st_stoploss_ord_request.l_scrip_cd = fn_swap_long(st_stoploss_ord_request.l_scrip_cd);
            st_stoploss_ord_request.l_msg_tag  = fn_swap_long(st_stoploss_ord_request.l_msg_tag);
            st_stoploss_ord_request.l_qty      = fn_swap_long(st_stoploss_ord_request.l_qty);
            st_stoploss_ord_request.l_avaqty   = fn_swap_long(st_stoploss_ord_request.l_avaqty);
            st_stoploss_ord_request.l_sltp_rate= fn_swap_long(st_stoploss_ord_request.l_sltp_rate);
            st_stoploss_ord_request.l_limit_rate = fn_swap_long(st_stoploss_ord_request.l_limit_rate);
            st_stoploss_ord_request.s_retention  = fn_swap_short(st_stoploss_ord_request.s_retention);
            st_stoploss_ord_request.l_cust_id    = fn_swap_long(st_stoploss_ord_request.l_cust_id);
            st_stoploss_ord_request.s_caclass    = fn_swap_short(st_stoploss_ord_request.s_caclass);
            st_stoploss_ord_request.ll_ordr_id   = fn_swap_ulnglng(st_stoploss_ord_request.ll_ordr_id);
            st_stoploss_ord_request.ll_location_id = fn_swap_ulnglng(st_stoploss_ord_request.ll_location_id);
            
            fprintf(fptr,"%-35s=:%ld:\n", " Message Type ",st_stoploss_ord_request.l_msg_typ);  
            fprintf(fptr,"%-35s=:%ld:\n", " Scrip Code   ",st_stoploss_ord_request.l_scrip_cd);
            fprintf(fptr,"%-35s=:%ld:\n", " Message Tag  ",st_stoploss_ord_request.l_msg_tag);
            fprintf(fptr,"%-35s=:%ld:\n", " Quantity     ",st_stoploss_ord_request.l_qty);
            fprintf(fptr,"%-35s=:%ld:\n", " Available Qty",st_stoploss_ord_request.l_avaqty);
            fprintf(fptr,"%-35s=:%ld:\n", " SLTP Rate    ",st_stoploss_ord_request.l_sltp_rate);
            fprintf(fptr,"%-35s=:%ld:\n", " Limit Rate   ",st_stoploss_ord_request.l_limit_rate);
            fprintf(fptr,"%-35s=:%lld:\n", " Order Id     ",st_stoploss_ord_request.ll_ordr_id);                   
            fn_bfprint_string(fptr," Client Id   \t\t\t=",st_stoploss_ord_request.c_client_id,sizeof(st_stoploss_ord_request.c_client_id));
            fprintf(fptr,"%-35s=:%d:\n", "  Retention    ",st_stoploss_ord_request.s_retention);
            fn_tmstmp_dmy_to_arr(st_stoploss_ord_request.st_timestmp_dmy, c_time_str);
            fprintf(fptr, "%-35s=:%s:\n", "\t\t Transaction Time", c_time_str);
            fprintf(fptr,"%-35s=:%c:\n", " C_Filler    ",st_stoploss_ord_request.c_filler); 
            fprintf(fptr,"%-35s=:%ld:\n", " Cust Id      ",st_stoploss_ord_request.l_cust_id);
            fprintf(fptr,"%-35s=:%ld:\n", " CACLass      ",st_stoploss_ord_request.s_caclass);
            fprintf(fptr,"%-35s=:%lld:\n", " Location Id  ",st_stoploss_ord_request.ll_location_id);
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break;
          }
       case DEL_STOPLOSS_BUY_ORD:
       case DEL_STOPLOSS_SELL_ORD:
       case DEL_SLTP_BUY_AFTER_TRIGGER:
       case DEL_SLTP_SELL_AFTER_TRIGGER:
          {
            struct st_del_ord_rqst st_del_ord_request;
            memset(&st_del_ord_request,0,sizeof(struct st_del_ord_rqst));
            memcpy(&st_del_ord_request,(struct st_del_ord_rqst*)c_msg_data,sizeof(st_del_ord_request));
            st_del_ord_request.l_msg_typ = htonl(st_del_ord_request.l_msg_typ);
            st_del_ord_request.l_scrip_cd = htonl(st_del_ord_request.l_scrip_cd);
            st_del_ord_request.l_msg_tag = htonl(st_del_ord_request.l_msg_tag);
  
            st_del_ord_request.l_msg_typ = fn_swap_long(st_del_ord_request.l_msg_typ);
            st_del_ord_request.l_msg_tag = fn_swap_long(st_del_ord_request.l_msg_tag);
            st_del_ord_request.l_scrip_cd = fn_swap_long(st_del_ord_request.l_scrip_cd);
            st_del_ord_request.ll_transn_id = fn_swap_ulnglng(st_del_ord_request.ll_transn_id);
            
            fprintf(fptr,"%-35s=:%ld:\n", " Message Type ",st_del_ord_request.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", " Scrip code    ",st_del_ord_request.l_scrip_cd);
            fprintf(fptr,"%-35s=:%ld:\n", " Msg Tag      ",st_del_ord_request.l_msg_tag);
            fprintf(fptr,"%-35s=:%lld",  " Order ID     ",st_del_ord_request.ll_transn_id);
 
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break;
          }
        case UPD_SLTP_BUY_AFTER_TRG:
        case UPD_SLTP_SELL_AFTER_TRG:
          {
           struct st_upd_sltp_rqst st_upd_sltp_request;
           memset(&st_upd_sltp_request,0,sizeof(st_upd_sltp_request));
           memcpy(&st_upd_sltp_request,(struct st_upd_sltp_request*)c_msg_data,sizeof(st_upd_sltp_request));

           /***** Conver from Host to network byte ***********/
           st_upd_sltp_request.l_msg_typ = htonl(st_upd_sltp_request.l_msg_typ);
           st_upd_sltp_request.l_scrip_cd = htonl(st_upd_sltp_request.l_scrip_cd);
            st_upd_sltp_request.l_msg_tag = htonl(st_upd_sltp_request.l_msg_tag);
            st_upd_sltp_request.l_qty = htonl(st_upd_sltp_request.l_qty);
            st_upd_sltp_request.l_avaqty = htonl(st_upd_sltp_request.l_avaqty);
            st_upd_sltp_request.l_rate   = htonl(st_upd_sltp_request.l_rate);
            st_upd_sltp_request.s_retention = htons(st_upd_sltp_request.s_retention);
            st_upd_sltp_request.l_miniqty = htonl(st_upd_sltp_request.l_miniqty);
            st_upd_sltp_request.l_cust_cd = htonl(st_upd_sltp_request.l_cust_cd);
            st_upd_sltp_request.s_caclass = htons(st_upd_sltp_request.s_caclass);

          /***** Convertion from EBA to Exchange format **********/
            st_upd_sltp_request.l_msg_typ = fn_swap_long(st_upd_sltp_request.l_msg_typ);
            st_upd_sltp_request.l_scrip_cd = fn_swap_long(st_upd_sltp_request.l_scrip_cd);
            st_upd_sltp_request.l_msg_tag = fn_swap_long(st_upd_sltp_request.l_msg_tag);
            st_upd_sltp_request.l_qty = fn_swap_long(st_upd_sltp_request.l_qty);
            st_upd_sltp_request.l_avaqty = fn_swap_long(st_upd_sltp_request.l_avaqty);
            st_upd_sltp_request.l_rate   = fn_swap_long(st_upd_sltp_request.l_rate);
            st_upd_sltp_request.s_retention = fn_swap_short(st_upd_sltp_request.s_retention);
            st_upd_sltp_request.l_miniqty = fn_swap_long(st_upd_sltp_request.l_miniqty);
            st_upd_sltp_request.l_cust_cd = fn_swap_long(st_upd_sltp_request.l_cust_cd);
            st_upd_sltp_request.s_caclass = fn_swap_short(st_upd_sltp_request.s_caclass);
            st_upd_sltp_request.ll_order_id = fn_swap_ulnglng(st_upd_sltp_request.ll_order_id);
            st_upd_sltp_request.ll_location_id = fn_swap_ulnglng(st_upd_sltp_request.ll_location_id);

            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Typ     ",st_upd_sltp_request.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Scrip Code      ",st_upd_sltp_request.l_scrip_cd);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag         ",st_upd_sltp_request.l_msg_tag);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Qty             ",st_upd_sltp_request.l_qty);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Avalable Qty    ",st_upd_sltp_request.l_avaqty);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Rate            ", st_upd_sltp_request.l_rate);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\t Order Id        ",st_upd_sltp_request.ll_order_id);
            fn_bfprint_string(fptr," Client Id   \t\t\t=",st_upd_sltp_request.c_client_id,sizeof(st_upd_sltp_request.c_client_id));
            fprintf(fptr,"%-35s=:%d:\n", "\t\t  Retention       ",st_upd_sltp_request.s_retention);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Mini Qty        ",st_upd_sltp_request.l_miniqty);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Cust cd         ",st_upd_sltp_request.l_cust_cd);
            fprintf(fptr,"%-35s=:%d:\n", "\t\t  CACLASS         ",st_upd_sltp_request.s_caclass);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\t Location id    ",st_upd_sltp_request.ll_location_id);
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break;
          }  
        case OPN_INTRST_RQST:
          { 
            struct st_opn_intrst_rqst st_opn_intrst_data;
            memset(&st_opn_intrst_data,0,sizeof(st_opn_intrst_data));
            memcpy(&st_opn_intrst_data,(struct st_opn_intrst_rqst*)c_msg_data,sizeof(st_opn_intrst_data)); 
         
            st_opn_intrst_data.l_msg_typ = fn_swap_long(st_opn_intrst_data.l_msg_typ);
            st_opn_intrst_data.l_msg_tag = fn_swap_long(st_opn_intrst_data.l_msg_tag);
            st_opn_intrst_data.l_rsrvd_fld = fn_swap_long(st_opn_intrst_data.l_rsrvd_fld);
            
            fprintf(fptr,"%-35s=:%ld:\n", " Message Type ",st_opn_intrst_data.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", " Message Tag  ",st_opn_intrst_data.l_msg_tag);
            fprintf(fptr,"%-35s=:%ld:\n", " Rsrvd Field  ",st_opn_intrst_data.l_rsrvd_fld);
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break;
          }
        default:
              { 
                 fprintf(fptr,"Inside default -- Message code =:%d:\n", li_msg_typ);
                 fn_bfprint_asc( fptr, c_msg_data, c_ServiceName, c_errmsg );
                 fflush(fptr);
                 break;
              }
        } /*** End of li_msg_typ ***/

        return(0);
}



/******************************************************************************/
/* This function takes char buffer as input and depending on the message type */
/* maps them to respective structures and writes the data to the file pointed */
/* to by the file pointer.                                                    */
/* INPUT PARAMETERS                                                           */
/*      fptr         - File handler                                           */
/*      li_msg_typ   - Message Type                                           */
/*      c_msg_data   - Structure to be printed                                */
/*      c_ServiceName- Name of the service                                    */
/*      c_errmsg     - Error String                                           */
/* OUTPUT PARAMETERS                                                          */
/*          0 - Success                                                       */
/******************************************************************************/
int fn_bfprint_rcvlog(FILE *fptr,
                  char c_iml_header,
                  void *c_msg_data,
                  char *c_ServiceName,
                  char *c_errmsg)
{
  char      c_dname[22];
  char      c_start[30];   /* VER TOL : TUX on LINUX -- Size chnaged from 20 to 30 */
  char      c_end[30];     /* VER TOL : TUX on LINUX -- Size chnaged from 15 to 30 */
  time_t    tm_stmp;
  char      c_time_str[22]; 
  short int si_err_code;
  int       i_loop = 0;
  long int  li_msg_typ;
  long long ll_timestamp1;
  long long ll_timestamp2;
  long long ll_jiffy;
  int l_record = 0;
  int l_tot_records = 0;

  struct st_header st_sndhdr;
  
  tm_stmp = time(NULL);
  strftime(c_dname, 21, "%d-%b-%Y %H:%M:%S", localtime(&tm_stmp));
  fprintf(fptr ,"==========================================================================================\n");
  fprintf(fptr,"\t Timestamp :%s:\n", c_dname);
  fprintf(fptr ,"-----------------------------------------------------------------------\n");
  memset(&st_sndhdr, 0, sizeof(st_sndhdr));
  memcpy(&st_sndhdr, c_msg_data, sizeof(st_sndhdr));
  /***************************************************/
  st_sndhdr.l_slot_no         = ntohl(st_sndhdr.l_slot_no);
  st_sndhdr.l_message_length  = ntohl(st_sndhdr.l_message_length);
  st_sndhdr.l_msg_type        = ntohl(st_sndhdr.l_msg_type);

  st_sndhdr.l_slot_no         = fn_swap_long(st_sndhdr.l_slot_no);
  st_sndhdr.l_message_length  = fn_swap_long(st_sndhdr.l_message_length);
  st_sndhdr.l_msg_type        = fn_swap_long(st_sndhdr.l_msg_type);
  /***************************************************/

  fprintf(fptr  , "\t HEADER :: \n");
  fprintf(fptr, "%-35s=:%ld:\n", "\t\tSlot No", st_sndhdr.l_slot_no);
  fprintf(fptr, "%-35s=:%ld:\n", "\t\tMessage Length",st_sndhdr.l_message_length);
  fprintf(fptr ,"-----------------------------------------------------------------------\n");
  fprintf(fptr  , "\t DATA   ::\n");

  strcpy(c_start, "\t Printing New packet");
  strcpy(c_end, "\t Packet Printed");

  fprintf(fptr, "%s\n", c_start);

     /**  memset(&st_rqst_data,0,sizeof(st_rqst_data));
       memcpy(&st_rqst_data,  c_msg_data , sizeof(c_msg_data)); ***/

       li_msg_typ = st_sndhdr.l_msg_type;;
       if(li_msg_typ == 99 )
         li_msg_typ = 0;
       switch(li_msg_typ)
       {

         case USR_RGSTRN_REQ_OUT :
           {
               struct st_bfo_rgstrn_reply st_rgstrn_out;

                 memset(&st_rgstrn_out,0,sizeof(st_rgstrn_out));
                 memcpy(&st_rgstrn_out,(struct st_rgstrn_out *)c_msg_data, sizeof(st_rgstrn_out));

                 st_rgstrn_out.l_msg_type = ntohl(st_rgstrn_out.l_msg_type);
                 st_rgstrn_out.l_slot_no = ntohl(st_rgstrn_out.l_slot_no);
                 st_rgstrn_out.l_msg_tag = ntohl(st_rgstrn_out.l_msg_tag);

                 st_rgstrn_out.l_msg_type = fn_swap_long(st_rgstrn_out.l_msg_type);
                 st_rgstrn_out.l_slot_no = fn_swap_long(st_rgstrn_out.l_slot_no);
                 st_rgstrn_out.l_msg_tag = fn_swap_long(st_rgstrn_out.l_msg_tag);

                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Type",st_rgstrn_out.l_msg_type);
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Slot No     ",st_rgstrn_out.l_slot_no);
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Tag   ",st_rgstrn_out.l_msg_tag);
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Error no     ",st_rgstrn_out.l_error_no);
                 fn_bfprint_string(fptr,"\t\t Error Text ",st_rgstrn_out.c_error_text,sizeof(st_rgstrn_out.c_error_text));
                 fprintf(fptr, "\n%s\n\n", c_end);
                 fflush(fptr);
                 break;
           }
        case LOGON_REQ_OUT :
           {
                 struct st_bfo_logon_reply st_login_out;
                 memset(&st_login_out,0,sizeof(st_login_out));
                 memcpy(&st_login_out,(struct st_login_out*)c_msg_data, sizeof(st_login_out));

                 st_login_out.l_msg_type = ntohl(st_login_out.l_msg_type);
                 st_login_out.l_current_sssn = ntohl(st_login_out.l_current_sssn);
                 st_login_out.l_msg_tag = ntohl(st_login_out.l_msg_tag);
                 st_login_out.s_reply_code = ntohs(st_login_out.s_reply_code);
                 st_login_out.s_filler   = ntohs(st_login_out.s_filler);
                 st_login_out.s_settlement_no= ntohs(st_login_out.s_settlement_no);
           
                 st_login_out.l_msg_type = fn_swap_long(st_login_out.l_msg_type);
                 st_login_out.l_current_sssn = fn_swap_long(st_login_out.l_current_sssn);
                 st_login_out.l_msg_tag  = fn_swap_long(st_login_out.l_msg_tag);
                 st_login_out.s_reply_code = fn_swap_short(st_login_out.s_reply_code);
                 st_login_out.s_filler = fn_swap_short(st_login_out.s_filler);
                 st_login_out.s_settlement_no = fn_swap_short(st_login_out.s_settlement_no);
 
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Type   ",st_login_out.l_msg_type);
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Current session ",st_login_out.l_current_sssn);
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message tag       ",st_login_out.l_msg_tag);
                 fprintf(fptr,"%-35s=:%d:\n", "\t\t Replay Code    ",st_login_out.s_reply_code);
                 fn_bfprint_string(fptr,"\t\t Replay Text   ",st_login_out.c_reply_txt,sizeof(st_login_out.c_reply_txt));
                 fn_bfprint_string(fptr,"\t\t Msg of the day",st_login_out.c_msg_of_the_day,sizeof(st_login_out.c_msg_of_the_day));
                 fn_tmstmp_ymdhms_to_arr(st_login_out.st_crrnt_logon_time, c_time_str);
                 fprintf(fptr, "%-35s=:%s:\n", "\t\t Current Login Time", c_time_str);
                 fn_tmstmp_ymdhms_to_arr(st_login_out.st_prvs_logon_time,c_time_str);
                 fprintf(fptr, "%-35s=:%s:\n", "\t\t Previous Login Time",c_time_str);
                 fprintf(fptr,"%-35s=:%c:\n", "\t\t Centure   ",st_login_out.c_century);
                 fprintf(fptr,"%-35s=:%c:\n", "\t\t Scrip Master Version ",st_login_out.uc_scrip_mstr_version);
                 fprintf(fptr,"%-35s=:%c:\n", "\t\t Scrip Master Release ",st_login_out.uc_scrip_mstr_release);
                 fprintf(fptr, "%-35s=:%s:\n", "\t\t Index       ", st_login_out.c_index_id);
                 fprintf(fptr,"%-35s=:%d\n", "\t\t Filler        ", st_login_out.s_filler);
                 fprintf(fptr,"%-35s=:%d\n", "\t\t Settlement No ",st_login_out.s_settlement_no);
                 fprintf(fptr,"%-35s=:%ld", "\t\t Message Tag   ",st_login_out.l_msg_tag); 
                 fprintf(fptr, "\n%s\n\n", c_end);
                 fflush(fptr);
                 break;
           }
        case LOGOFF_REQ_OUT :
          {
                 struct st_bfo_logoff_rply st_logoff_rply;
                 memset(&st_logoff_rply,0,sizeof(st_logoff_rply));
                 memcpy(&st_logoff_rply,(struct st_logoff_rply*)c_msg_data, sizeof(st_logoff_rply));

                 st_logoff_rply.l_msg_typ = ntohl(st_logoff_rply.l_msg_typ);
                 st_logoff_rply.l_current_sssn = ntohl(st_logoff_rply.l_current_sssn);
                 st_logoff_rply.l_msg_tag =  ntohl(st_logoff_rply.l_msg_tag);
                 st_logoff_rply.s_reply_cd = ntohs(st_logoff_rply.s_reply_cd);
                 st_logoff_rply.s_filler = ntohs(st_logoff_rply.s_filler);

                 st_logoff_rply.l_msg_typ = fn_swap_long(st_logoff_rply.l_msg_typ);
                 st_logoff_rply.l_current_sssn = fn_swap_long(st_logoff_rply.l_current_sssn);
                 st_logoff_rply.l_msg_tag = fn_swap_long(st_logoff_rply.l_msg_tag);
                 st_logoff_rply.s_reply_cd = fn_swap_short(st_logoff_rply.s_reply_cd);
                 st_logoff_rply.s_filler = fn_swap_short(st_logoff_rply.s_filler);

                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Type",st_logoff_rply.l_msg_typ);
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Tag ",st_logoff_rply.l_msg_tag);
                 fprintf(fptr,"%-35s=:%ld:\n", "\t\t Current SSSN ",st_logoff_rply.l_current_sssn);
                 fprintf(fptr,"%-35s=:%d:\n", "\t\t Reply Code ",st_logoff_rply.s_reply_cd);
                 fprintf(fptr,"%-35s=:%d:\n", "\t\t Filler ",st_logoff_rply.s_filler);
                 fn_bfprint_string(fptr,"\t\t Replay Msg ",st_logoff_rply.c_reply_msg,sizeof(st_logoff_rply.c_reply_msg));

                 fprintf(fptr, "\n%s\n\n", c_end);
                 fflush(fptr);
                 break;
          }
       case MANDATORY_PASSWD_CHNG :
       case OPTNL_PASSWD_CHNG :
          {
               struct st_bfo_upd_passwd_rply st_chng_passwd_rply;
               memset(&st_chng_passwd_rply,0,sizeof(st_chng_passwd_rply));
               memcpy(&st_chng_passwd_rply,(struct st_bfo_upd_passwd_rply*)c_msg_data,sizeof(st_chng_passwd_rply));
               st_chng_passwd_rply.l_msg_type = ntohl(st_chng_passwd_rply.l_msg_type);
               st_chng_passwd_rply.l_msg_type = fn_swap_long(st_chng_passwd_rply.l_msg_type);
               st_chng_passwd_rply.s_reply_code = ntohs(st_chng_passwd_rply.s_reply_code);
               st_chng_passwd_rply.s_reply_code = fn_swap_short(st_chng_passwd_rply.s_reply_code);
               st_chng_passwd_rply.l_msg_tag    = ntohl(st_chng_passwd_rply.l_msg_tag);
               st_chng_passwd_rply.l_msg_tag    = fn_swap_long(st_chng_passwd_rply.l_msg_tag);
               
               fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Type",st_chng_passwd_rply.l_msg_type);
               fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Tag ",st_chng_passwd_rply.l_msg_tag);
               fprintf(fptr,"%-35s=:%d:\n", "\t\t Reply Code ",st_chng_passwd_rply.s_reply_code);
               fn_bfprint_string(fptr,"\t\t Replay Msg ",st_chng_passwd_rply.c_reply_txt,sizeof(st_chng_passwd_rply.c_reply_txt));
               fprintf(fptr, "\n%s\n\n", c_end);
               fflush(fptr);
               break; 
                                 
          }          
     
         case PRSNL_ORDR_DWNLD:
         case PRSNL_TRD_DWNLD:
         case PRSN_SLTP_ORDR_DWNLD:
         {

          if( st_sndhdr.l_slot_no == 1 )
          {
           struct st_prsnl_dwnld_rply st_prsnl_dwnld_reply;
           memset(&st_prsnl_dwnld_reply,0,sizeof(st_prsnl_dwnld_reply));
           memcpy(&st_prsnl_dwnld_reply,(struct st_prsnl_dwnld_rply*)c_msg_data,sizeof(st_prsnl_dwnld_reply));
          
           st_prsnl_dwnld_reply.l_msg_typ = ntohl(st_prsnl_dwnld_reply.l_msg_typ);
           st_prsnl_dwnld_reply.l_msg_typ = fn_swap_long(st_prsnl_dwnld_reply.l_msg_typ);
           st_prsnl_dwnld_reply.s_rply_cd = ntohs(st_prsnl_dwnld_reply.s_rply_cd);
           st_prsnl_dwnld_reply.s_rply_cd = fn_swap_short(st_prsnl_dwnld_reply.s_rply_cd);
           st_prsnl_dwnld_reply.l_msg_tag = ntohl(st_prsnl_dwnld_reply.l_msg_tag);
           st_prsnl_dwnld_reply.l_msg_tag = fn_swap_long(st_prsnl_dwnld_reply.l_msg_tag);
 
           fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Type",st_prsnl_dwnld_reply.l_msg_typ);
           fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Tag ",st_prsnl_dwnld_reply.l_msg_tag);
           fprintf(fptr,"%-35s=:%d:\n", "\t\t Reply Code ",st_prsnl_dwnld_reply.s_rply_cd);
          }
          else
          {
           if( li_msg_typ == PRSNL_ORDR_DWNLD)
           {
            struct st_prsnl_ordr_ums st_prsnl_ordr_ums_data;
            memset(&st_prsnl_ordr_ums_data,0,sizeof(st_prsnl_ordr_ums_data)); 
            memcpy(&st_prsnl_ordr_ums_data,(struct st_prsnl_ordr_ums*)c_msg_data,sizeof(st_prsnl_ordr_ums_data));
 
            st_prsnl_ordr_ums_data.l_msg_typ = ntohl(st_prsnl_ordr_ums_data.l_msg_typ);
            st_prsnl_ordr_ums_data.l_no_of_records = ntohl(st_prsnl_ordr_ums_data.l_no_of_records);
            st_prsnl_ordr_ums_data.l_msg_typ=fn_swap_long(st_prsnl_ordr_ums_data.l_msg_typ);
            st_prsnl_ordr_ums_data.l_no_of_records=fn_swap_long(st_prsnl_ordr_ums_data.l_no_of_records);
            l_tot_records = st_prsnl_ordr_ums_data.l_no_of_records;
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Type",st_prsnl_ordr_ums_data.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Type",st_prsnl_ordr_ums_data.l_no_of_records);          
               
            for(l_record = 0 ; l_record < l_tot_records ; l_record++)
            {
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_scrip_cd=ntohl(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_scrip_cd);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_rate=ntohl(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_rate);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_original_qty=ntohl(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_original_qty);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_pendng_qty=ntohl(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_pendng_qty);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_retention=ntohl(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_retention);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_msg_tag=ntohl(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_msg_tag);   
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_ltid =ntohl(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_ltid);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_cust_cd=ntohl(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_cust_cd);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].s_ca_class=ntohs(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].s_ca_class);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_min_qty=ntohl(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_min_qty);
             
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_scrip_cd=fn_swap_long(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_scrip_cd);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_rate=fn_swap_long(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_rate);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_original_qty=fn_swap_long(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_original_qty);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_pendng_qty=fn_swap_long(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_pendng_qty);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_retention=fn_swap_long(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_retention);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_msg_tag=fn_swap_long(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_msg_tag);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_ltid=fn_swap_long(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_ltid);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_cust_cd=fn_swap_long(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_cust_cd);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].s_ca_class=fn_swap_short(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].s_ca_class);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_min_qty=fn_swap_long(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_min_qty);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].ll_order_id = fn_swap_ulnglng(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].ll_order_id);
             st_prsnl_ordr_ums_data.st_ums_order_data[l_record].ll_location_id=fn_swap_ulnglng(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].ll_location_id);
             fprintf(fptr,"%-35s=:%ld:\n", "\n \t\t Record  No  ",l_record);  
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Scrip Code ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_scrip_cd);
             fprintf(fptr,"%-35s=:%lld:\n", "\t\t Order Id ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].ll_order_id);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Rate       ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_rate);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Orgnl Qty ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_original_qty);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Pending Qty",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_pendng_qty); 
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Retention ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_retention);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag   ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_msg_tag);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t LTID      ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_ltid);
             fprintf(fptr,"%-35s=:%c:\n", "\t\t  Order Typ ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].c_ordr_typ);
             fprintf(fptr,"%-35s=:%c:\n", "\t\t  Order Flow",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].c_ordr_flw);
             fn_tmstmp_ymdhms_to_arr(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].st_tmstmp_ymdhms, c_time_str);
             fprintf(fptr, "%-35s=:%s:\n", "\t\t Last Modified Time", c_time_str);
             fn_bfprint_string(fptr," Client Id   \t\t\t=",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].c_client_id,sizeof(st_prsnl_ordr_ums_data.st_ums_order_data[l_record].c_client_id));
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Cust Cd   ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_cust_cd);
             fprintf(fptr,"%-35s=:%d:\n", "\t\t Ca Class  ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].s_ca_class);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Min Qty  ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].l_min_qty);
              fprintf(fptr,"%-35s=:%lld:\n", "\t\t Location Id  ",st_prsnl_ordr_ums_data.st_ums_order_data[l_record].ll_location_id);
             
            } 
           }
           else if( li_msg_typ == PRSNL_TRD_DWNLD )
           {
            struct st_prsnl_trade_ums st_ums_trade_dtls_data;
            memset(&st_ums_trade_dtls_data,0,sizeof(st_ums_trade_dtls_data));
            memcpy(&st_ums_trade_dtls_data,(struct st_prsnl_trade_ums*)c_msg_data,sizeof(st_ums_trade_dtls_data));
            
            st_ums_trade_dtls_data.l_msg_typ = ntohl(st_ums_trade_dtls_data.l_msg_typ);
            st_ums_trade_dtls_data.l_no_of_records = ntohl(st_ums_trade_dtls_data.l_no_of_records);
            st_ums_trade_dtls_data.l_msg_typ = fn_swap_long(st_ums_trade_dtls_data.l_msg_typ);
            st_ums_trade_dtls_data.l_no_of_records = fn_swap_long(st_ums_trade_dtls_data.l_no_of_records);
            l_tot_records = st_ums_trade_dtls_data.l_no_of_records;
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Type",st_ums_trade_dtls_data.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t No Of Records",st_ums_trade_dtls_data.l_no_of_records);   

            for(l_record = 0 ; l_record < l_tot_records ; l_record++ )
            {
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_scrip_cd=ntohl(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_scrip_cd);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_trade_id=ntohl(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_trade_id);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_rate=ntohl(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_rate);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_qty=ntohl(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_qty);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_opp_member=ntohl(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_opp_member);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_OPP_trader=ntohl(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_OPP_trader);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_trnsctn_tag=ntohl(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_trnsctn_tag);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_cust_cd=ntohl(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_cust_cd);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].s_ca_class=ntohs(st_ums_trade_dtls_data.st_ums_trade_data[l_record].s_ca_class);

             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_scrip_cd=fn_swap_long(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_scrip_cd);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_trade_id=fn_swap_long(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_trade_id);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_rate=fn_swap_long(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_rate);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_qty=fn_swap_long(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_qty);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_opp_member=fn_swap_long(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_opp_member);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_OPP_trader=fn_swap_long(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_OPP_trader);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_trnsctn_tag=fn_swap_long(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_trnsctn_tag);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_cust_cd=fn_swap_long(st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_cust_cd);
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].s_ca_class=fn_swap_short(st_ums_trade_dtls_data.st_ums_trade_data[l_record].s_ca_class);
          
             st_ums_trade_dtls_data.st_ums_trade_data[l_record].ll_trnsctn_id=fn_swap_ulnglng(st_ums_trade_dtls_data.st_ums_trade_data[l_record].ll_trnsctn_id);
             fprintf(fptr,"%-35s=:%ld:\n", "\n \t\t Record No . ",l_record); 
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Scrip Code ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_scrip_cd);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Trade Id  ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_trade_id);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Rate      ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_rate);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Qty       ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_qty);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Opp Member ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_opp_member);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Opp Trader  ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_OPP_trader);
             fn_tmstmp_ymdhms_to_arr(st_ums_trade_dtls_data.st_ums_trade_data[l_record].st_tmstmp_ymdhms, c_time_str);
             fprintf(fptr, "%-35s=:%s:\n", "\t\t Last Modified Time", c_time_str);
             fn_bfprint_string(fptr," Client Id   \t\t\t=",st_ums_trade_dtls_data.st_ums_trade_data[l_record].c_client_id,sizeof(st_ums_trade_dtls_data.st_ums_trade_data[l_record].c_client_id));
             fprintf(fptr,"%-35s=:%c:\n", "\t\t  Order Typ ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].c_ordr_typ);
             fprintf(fptr,"%-35s=:%c:\n", "\t\t  Order Flow",st_ums_trade_dtls_data.st_ums_trade_data[l_record].c_ordr_flw);
             fprintf(fptr,"%-35s=:%lld:\n", "\t\t Trnsctn Id ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].ll_trnsctn_id);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Trnsctn Tag ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_trnsctn_tag);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Cust Code   ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].l_cust_cd);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Ca Class   ",st_ums_trade_dtls_data.st_ums_trade_data[l_record].s_ca_class);           
            }
           }
           else if( li_msg_typ == PRSN_SLTP_ORDR_DWNLD )
           {        
            struct st_prsnl_sltp_ums st_sltp_ums_data;
            memset(&st_sltp_ums_data,0,sizeof(st_sltp_ums_data));
            memcpy(&st_sltp_ums_data,(struct st_prsnl_sltp_ums*)c_msg_data,sizeof(st_sltp_ums_data));
   
            st_sltp_ums_data.l_msg_typ = ntohl(st_sltp_ums_data.l_msg_typ);
            st_sltp_ums_data.l_no_of_records = ntohl(st_sltp_ums_data.l_no_of_records);
            st_sltp_ums_data.l_no_of_records = fn_swap_long(st_sltp_ums_data.l_no_of_records);
            st_sltp_ums_data.l_msg_typ = fn_swap_long(st_sltp_ums_data.l_msg_typ);
            l_tot_records = st_sltp_ums_data.l_no_of_records;

            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Typ   ",st_sltp_ums_data.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Total Records DWNLD",st_sltp_ums_data.l_no_of_records);
           
            for(l_record = 0 ; l_record < l_tot_records ; l_record++ )
            {
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_scrip_cd=ntohl(st_sltp_ums_data.st_ums_sltp_data[l_record].l_scrip_cd);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_trg_rate=ntohl(st_sltp_ums_data.st_ums_sltp_data[l_record].l_trg_rate);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_limit_rate=ntohl(st_sltp_ums_data.st_ums_sltp_data[l_record].l_limit_rate);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_qty=ntohl(st_sltp_ums_data.st_ums_sltp_data[l_record].l_qty);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_avalbl_qty=ntohl(st_sltp_ums_data.st_ums_sltp_data[l_record].l_avalbl_qty);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_retention=ntohl(st_sltp_ums_data.st_ums_sltp_data[l_record].l_retention);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_msg_tag=ntohl(st_sltp_ums_data.st_ums_sltp_data[l_record].l_msg_tag);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_cust_cd=ntohl(st_sltp_ums_data.st_ums_sltp_data[l_record].l_cust_cd);
             st_sltp_ums_data.st_ums_sltp_data[l_record].s_ca_class=ntohs(st_sltp_ums_data.st_ums_sltp_data[l_record].s_ca_class);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_scrip_cd=fn_swap_long(st_sltp_ums_data.st_ums_sltp_data[l_record].l_scrip_cd);
             st_sltp_ums_data.st_ums_sltp_data[l_record].ll_ordr_id=fn_swap_ulnglng(st_sltp_ums_data.st_ums_sltp_data[l_record].ll_ordr_id);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_trg_rate=fn_swap_long(st_sltp_ums_data.st_ums_sltp_data[l_record].l_trg_rate);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_limit_rate=fn_swap_long(st_sltp_ums_data.st_ums_sltp_data[l_record].l_limit_rate);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_qty=fn_swap_long(st_sltp_ums_data.st_ums_sltp_data[l_record].l_qty);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_avalbl_qty=fn_swap_long(st_sltp_ums_data.st_ums_sltp_data[l_record].l_avalbl_qty);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_retention=fn_swap_long(st_sltp_ums_data.st_ums_sltp_data[l_record].l_retention);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_msg_tag=fn_swap_long(st_sltp_ums_data.st_ums_sltp_data[l_record].l_msg_tag);
             st_sltp_ums_data.st_ums_sltp_data[l_record].l_cust_cd=fn_swap_long(st_sltp_ums_data.st_ums_sltp_data[l_record].l_cust_cd);
             st_sltp_ums_data.st_ums_sltp_data[l_record].s_ca_class=fn_swap_short(st_sltp_ums_data.st_ums_sltp_data[l_record].s_ca_class);
             st_sltp_ums_data.st_ums_sltp_data[l_record].ll_location_id= fn_swap_ulnglng(st_sltp_ums_data.st_ums_sltp_data[l_record].ll_location_id);
             fprintf(fptr,"%-35s=:%ld:\n", " \n \t\t  Record No ",l_record);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Scrip Code  ",st_sltp_ums_data.st_ums_sltp_data[l_record].l_scrip_cd);
             fprintf(fptr,"%-35s=:%lld:\n", "\t\t Order ID     ",st_sltp_ums_data.st_ums_sltp_data[l_record].ll_ordr_id);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t TRG Rate     ",st_sltp_ums_data.st_ums_sltp_data[l_record].l_trg_rate);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Limit Rate   ",st_sltp_ums_data.st_ums_sltp_data[l_record].l_limit_rate);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Quantity     ",st_sltp_ums_data.st_ums_sltp_data[l_record].l_qty);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Available Qty ",st_sltp_ums_data.st_ums_sltp_data[l_record].l_avalbl_qty);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Retention    ",st_sltp_ums_data.st_ums_sltp_data[l_record].l_retention);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag      ",st_sltp_ums_data.st_ums_sltp_data[l_record].l_msg_tag);
             fprintf(fptr,"%-35s=:%c:\n", "\t\t  Order Type   ",st_sltp_ums_data.st_ums_sltp_data[l_record].c_ordr_typ);
             fprintf(fptr,"%-35s=:%c:\n", "\t\t  Order Flow   ",st_sltp_ums_data.st_ums_sltp_data[l_record].c_ordr_flw);
             fn_tmstmp_ymdhms_to_arr(st_sltp_ums_data.st_ums_sltp_data[l_record].st_tmstmp_ymdhms, c_time_str);
             fprintf(fptr, "%-35s=:%s:\n", "\t\t Last Modified Time", c_time_str);
             fn_bfprint_string(fptr," Client Id   \t\t\t=",st_sltp_ums_data.st_ums_sltp_data[l_record].c_client_id,sizeof(st_sltp_ums_data.st_ums_sltp_data[l_record].c_client_id));
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Cust Code    ",st_sltp_ums_data.st_ums_sltp_data[l_record].l_cust_cd); 
             fprintf(fptr,"%-35s=:%d:\n", "\t\t Ca Class     ",st_sltp_ums_data.st_ums_sltp_data[l_record].s_ca_class);
             fprintf(fptr,"%-35s=:%lld:\n", "\t\t Location Id  ",st_sltp_ums_data.st_ums_sltp_data[l_record].ll_location_id); 
            }     
           }
          } 
          fprintf(fptr, "\n%s\n\n", c_end);
          fflush(fptr);
          break;          
 
          }
        case PRSNL_DWNLD_CMPLT :
          {
             struct st_end_of_ums_dwnld st_end_of_ums_dwnld_data;
             memset(&st_end_of_ums_dwnld_data,0,sizeof(st_end_of_ums_dwnld_data));
             memcpy(&st_end_of_ums_dwnld_data,(struct st_end_of_ums_dwnld_data*)c_msg_data,sizeof(st_end_of_ums_dwnld_data));
          
             st_end_of_ums_dwnld_data.l_msg_typ = ntohl(st_end_of_ums_dwnld_data.l_msg_typ);
             st_end_of_ums_dwnld_data.l_no_of_recrd_dwnlded = ntohl(st_end_of_ums_dwnld_data.l_no_of_recrd_dwnlded);
             st_end_of_ums_dwnld_data.l_msg_tag = ntohl(st_end_of_ums_dwnld_data.l_msg_tag);
             st_end_of_ums_dwnld_data.l_msg_typ = fn_swap_long(st_end_of_ums_dwnld_data.l_msg_typ);
             st_end_of_ums_dwnld_data.l_no_of_recrd_dwnlded = fn_swap_long(st_end_of_ums_dwnld_data.l_no_of_recrd_dwnlded);
             st_end_of_ums_dwnld_data.l_msg_tag = fn_swap_long(st_end_of_ums_dwnld_data.l_msg_tag);

             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Typ   ",st_end_of_ums_dwnld_data.l_msg_typ);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t No of Records Downloaded",st_end_of_ums_dwnld_data.l_no_of_recrd_dwnlded);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag       ",st_end_of_ums_dwnld_data.l_msg_tag);
             fprintf(fptr, "\n%s\n\n", c_end);
             fflush(fptr);
             break;   
          
           }  

         case LIMIT_MARKET_ORD:
          {
            struct st_ord_rqst_rply st_lmtmkt_ord_rply;
            memset(&st_lmtmkt_ord_rply,0,sizeof(st_lmtmkt_ord_rply));
            memcpy(&st_lmtmkt_ord_rply,(struct st_lmtmkt_ord_rply*)c_msg_data,sizeof(st_lmtmkt_ord_rply));
            
            st_lmtmkt_ord_rply.l_msg_typ = ntohl(st_lmtmkt_ord_rply.l_msg_typ);
            st_lmtmkt_ord_rply.l_msg_tag1= ntohl(st_lmtmkt_ord_rply.l_msg_tag1);
            st_lmtmkt_ord_rply.s_rply_cd = ntohs(st_lmtmkt_ord_rply.s_rply_cd);
            st_lmtmkt_ord_rply.s_filler1 = ntohs(st_lmtmkt_ord_rply.s_filler1);
            st_lmtmkt_ord_rply.l_amended_qty = ntohl(st_lmtmkt_ord_rply.l_amended_qty);
            st_lmtmkt_ord_rply.l_msg_tag2 = ntohl(st_lmtmkt_ord_rply.l_msg_tag2);
            st_lmtmkt_ord_rply.l_pending_qty = ntohl(st_lmtmkt_ord_rply.l_pending_qty);
            st_lmtmkt_ord_rply.l_filler4 = ntohl(st_lmtmkt_ord_rply.l_filler4);
            st_lmtmkt_ord_rply.s_filler5 = ntohs(st_lmtmkt_ord_rply.s_filler5);
            st_lmtmkt_ord_rply.s_filler6 = ntohs(st_lmtmkt_ord_rply.s_filler6);

            st_lmtmkt_ord_rply.ll_ord_id = fn_swap_ulnglng(st_lmtmkt_ord_rply.ll_ord_id);
            st_lmtmkt_ord_rply.l_msg_typ = fn_swap_long(st_lmtmkt_ord_rply.l_msg_typ);
            st_lmtmkt_ord_rply.l_msg_tag1 = fn_swap_long(st_lmtmkt_ord_rply.l_msg_tag1);
            st_lmtmkt_ord_rply.s_rply_cd = fn_swap_short(st_lmtmkt_ord_rply.s_rply_cd);
            st_lmtmkt_ord_rply.s_filler1 = fn_swap_short(st_lmtmkt_ord_rply.s_filler1);
            st_lmtmkt_ord_rply.l_amended_qty = fn_swap_long(st_lmtmkt_ord_rply.l_amended_qty);
            st_lmtmkt_ord_rply.l_msg_tag2 = fn_swap_long(st_lmtmkt_ord_rply.l_msg_tag2);
            st_lmtmkt_ord_rply.l_pending_qty = fn_swap_long(st_lmtmkt_ord_rply.l_pending_qty);
            st_lmtmkt_ord_rply.l_filler4 = fn_swap_long(st_lmtmkt_ord_rply.l_filler4);
            st_lmtmkt_ord_rply.s_filler5 = fn_swap_short(st_lmtmkt_ord_rply.s_filler5);
            st_lmtmkt_ord_rply.s_filler6 = fn_swap_short(st_lmtmkt_ord_rply.s_filler6);
                     
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Type",st_lmtmkt_ord_rply.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag     ",st_lmtmkt_ord_rply.l_msg_tag1);
            fprintf(fptr,"%-35s=:%d:\n", "\t\t Reply code  ",st_lmtmkt_ord_rply.s_rply_cd);
            fprintf(fptr,"%-35s=:%d:\n", "\t\t S_FIller1   ",st_lmtmkt_ord_rply.s_filler1);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Amended Qty ",st_lmtmkt_ord_rply.l_amended_qty);
            fn_tmstmp_dmyhms_to_arr(st_lmtmkt_ord_rply.st_tmstmp_dmyhms, c_time_str);
            fprintf(fptr, "%-35s=:%s:\n", "\t\t Transaction Time", c_time_str);
            fn_bfprint_string(fptr," \t\t Milli Seconds \t",st_lmtmkt_ord_rply.c_millisec,sizeof(st_lmtmkt_ord_rply.c_millisec));
            fprintf(fptr,"%-35s=:%lld:\n", "\t\t Order Id ",st_lmtmkt_ord_rply.ll_ord_id); 
            fprintf(fptr,"%-35s=:%c:\n", "\t\t  Action Code   ",st_lmtmkt_ord_rply.c_ordr_actn_cd);
            fprintf(fptr,"%-35s=:%c:\n", "\t\t  Buy / Sell    ",st_lmtmkt_ord_rply.c_buysell_flg);
            fprintf(fptr,"%-35s=:%c:\n", "\t\t  Order Type   ",st_lmtmkt_ord_rply.c_ord_typ);
            fn_bfprint_string(fptr,"\t\t C_Filler2  ",st_lmtmkt_ord_rply.c_filler2,sizeof(st_lmtmkt_ord_rply.c_filler2));
            fn_bfprint_string(fptr,"\t\t Reply Txt   ",st_lmtmkt_ord_rply.c_rply_txt,sizeof(st_lmtmkt_ord_rply.c_rply_txt));
            fn_bfprint_string(fptr,"\t\t C_Filler3  ",st_lmtmkt_ord_rply.c_filler3,sizeof(st_lmtmkt_ord_rply.c_filler3));
            fn_bfprint_string(fptr,"\t\t C_Reservd_Fld ",st_lmtmkt_ord_rply.c_reserved_fld,sizeof(st_lmtmkt_ord_rply.c_reserved_fld));
            fn_bfprint_string(fptr,"\t\t C_MsgTag3  ",st_lmtmkt_ord_rply.c_msg_tag3,sizeof(st_lmtmkt_ord_rply.c_msg_tag3));
  
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t L_MsgTag2    ",st_lmtmkt_ord_rply.l_msg_tag2);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Pending Qty ",st_lmtmkt_ord_rply.l_pending_qty);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t L_Filler4   ",st_lmtmkt_ord_rply.l_filler4);
            fprintf(fptr,"%-35s=:%d:\n", "\t\t S_Filler5   ",st_lmtmkt_ord_rply.s_filler5);
            fprintf(fptr,"%-35s=:%d:\n", "\t\t S_FIller6   ",st_lmtmkt_ord_rply.s_filler6);
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break; 
          }
       case CONVRTED_MKT_TO_LMT:
          {
            struct st_mkrt_to_lmt_ord st_mkrt_to_lmt_ord_rpl;
            memset(&st_mkrt_to_lmt_ord_rpl,0,sizeof(st_mkrt_to_lmt_ord_rpl));
            memcpy(&st_mkrt_to_lmt_ord_rpl,(struct st_mkrt_to_lmt_ord_rpl*)c_msg_data,sizeof(st_mkrt_to_lmt_ord_rpl));
                        
            st_mkrt_to_lmt_ord_rpl.l_msg_typ = ntohl(st_mkrt_to_lmt_ord_rpl.l_msg_typ);
            st_mkrt_to_lmt_ord_rpl.l_scrip_cd = ntohl(st_mkrt_to_lmt_ord_rpl.l_scrip_cd);
            st_mkrt_to_lmt_ord_rpl.l_msg_tag = ntohl(st_mkrt_to_lmt_ord_rpl.l_msg_tag);
            st_mkrt_to_lmt_ord_rpl.l_cnvtdrate = ntohl(st_mkrt_to_lmt_ord_rpl.l_cnvtdrate);

            st_mkrt_to_lmt_ord_rpl.l_msg_typ = fn_swap_long(st_mkrt_to_lmt_ord_rpl.l_msg_typ);
            st_mkrt_to_lmt_ord_rpl.l_scrip_cd = fn_swap_long(st_mkrt_to_lmt_ord_rpl.l_scrip_cd);
            st_mkrt_to_lmt_ord_rpl.l_msg_tag = fn_swap_long(st_mkrt_to_lmt_ord_rpl.l_msg_tag);
            st_mkrt_to_lmt_ord_rpl.l_cnvtdrate = fn_swap_long(st_mkrt_to_lmt_ord_rpl.l_cnvtdrate);
            st_mkrt_to_lmt_ord_rpl.ll_order_id = fn_swap_ulnglng(st_mkrt_to_lmt_ord_rpl.ll_order_id);  
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Typ     ",st_mkrt_to_lmt_ord_rpl.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Scrip Code     ",st_mkrt_to_lmt_ord_rpl.l_scrip_cd);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\t Order ID    ",st_mkrt_to_lmt_ord_rpl.ll_order_id);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag     ",st_mkrt_to_lmt_ord_rpl.l_msg_tag);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Converted Rate    ",st_mkrt_to_lmt_ord_rpl.l_cnvtdrate);                
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break; 
               
          }
        case ADD_IOC_BUY_ORDR:
        case ADD_IOC_SELL_ORDR:
        case DEL_STOPLOSS_BUY_ORD:
        case DEL_STOPLOSS_SELL_ORD:
        case UPD_SLTP_BUY_AFTER_TRG:
        case UPD_SLTP_SELL_AFTER_TRG:
        case DEL_SLTP_BUY_AFTER_TRIGGER:
        case DEL_SLTP_SELL_AFTER_TRIGGER:
          {
            struct st_del_ord_rply st_ioc_sltp_ord_rply;
            memset(&st_ioc_sltp_ord_rply,0,sizeof(st_ioc_sltp_ord_rply));
            memcpy(&st_ioc_sltp_ord_rply,(struct st_del_ord_rply*)c_msg_data,sizeof(st_ioc_sltp_ord_rply));
            
            st_ioc_sltp_ord_rply.l_msg_typ = ntohl(st_ioc_sltp_ord_rply.l_msg_typ);
            st_ioc_sltp_ord_rply.l_msg_tag = ntohl(st_ioc_sltp_ord_rply.l_msg_tag);
            st_ioc_sltp_ord_rply.s_rply_cd = ntohs(st_ioc_sltp_ord_rply.s_rply_cd);
            st_ioc_sltp_ord_rply.s_ratediff= ntohs(st_ioc_sltp_ord_rply.s_ratediff);
          
            st_ioc_sltp_ord_rply.l_msg_typ = fn_swap_long(st_ioc_sltp_ord_rply.l_msg_typ);
            st_ioc_sltp_ord_rply.l_msg_tag = fn_swap_long(st_ioc_sltp_ord_rply.l_msg_tag);
            st_ioc_sltp_ord_rply.s_rply_cd = fn_swap_short(st_ioc_sltp_ord_rply.s_rply_cd);
            st_ioc_sltp_ord_rply.s_ratediff = fn_swap_short(st_ioc_sltp_ord_rply.s_ratediff);
            st_ioc_sltp_ord_rply.ll_transn_id = fn_swap_ulnglng(st_ioc_sltp_ord_rply.ll_transn_id);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Typ     ",st_ioc_sltp_ord_rply.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag         ",st_ioc_sltp_ord_rply.l_msg_tag);
            fprintf(fptr,"%-35s=:%d:\n", "\t\tReply Code     ",st_ioc_sltp_ord_rply.s_rply_cd);
            fprintf(fptr,"%-35s=:%d:\n", "\t\t Rate Diff      ",st_ioc_sltp_ord_rply.s_ratediff);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\t Transction Id ",st_ioc_sltp_ord_rply.ll_transn_id);
            fn_tmstmp_dmyhms_to_arr(st_ioc_sltp_ord_rply.st_timestmp_dmyhms, c_time_str);
            fprintf(fptr, "%-35s=:%s:\n", "\t\t Transaction Time", c_time_str);
            fn_bfprint_string(fptr,"\t\t Reply Txt   ",st_ioc_sltp_ord_rply.c_rply_txt,sizeof(st_ioc_sltp_ord_rply.c_rply_txt)); 
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break;

          }
         case ADD_STOPLOSS_BUY_ORD:
         case UPD_STOPLOSS_BUY_ORD:
         case ADD_STOPLOSS_SELL_ORD:
         case UPD_STOPLOSS_SELL_ORD:
          {
           struct st_stoploss_ord_rply st_sltp_ord_reply;
           memset(&st_sltp_ord_reply,0, sizeof(st_sltp_ord_reply));
           memcpy(&st_sltp_ord_reply,(struct st_stoploss_ord_rply*)c_msg_data,sizeof(st_sltp_ord_reply));
            
           st_sltp_ord_reply.l_msg_typ = ntohl(st_sltp_ord_reply.l_msg_typ);            
           st_sltp_ord_reply.l_msg_tag = ntohl(st_sltp_ord_reply.l_msg_tag);
           st_sltp_ord_reply.s_rply_cd = ntohs(st_sltp_ord_reply.s_rply_cd);
           
            st_sltp_ord_reply.l_msg_typ = fn_swap_long(st_sltp_ord_reply.l_msg_typ);
            st_sltp_ord_reply.l_msg_tag = fn_swap_long(st_sltp_ord_reply.l_msg_tag);
            st_sltp_ord_reply.s_rply_cd = fn_swap_short(st_sltp_ord_reply.s_rply_cd);
            st_sltp_ord_reply.ll_transn_id = fn_swap_ulnglng(st_sltp_ord_reply.ll_transn_id);
            fn_tmstmp_dmyhms_to_arr(st_sltp_ord_reply.st_timestmp_dmyhms, c_time_str);
          
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Typ     ",st_sltp_ord_reply.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag         ",st_sltp_ord_reply.l_msg_tag);
            fprintf(fptr,"%-35s=:%d:\n", "\t\t  Reply Code     ",st_sltp_ord_reply.s_rply_cd);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\t Transction Id  ",st_sltp_ord_reply.ll_transn_id);
            fprintf(fptr, "%-35s=:%s:\n", "\t\t Transaction Time", c_time_str);
            fn_bfprint_string(fptr,"\t\t Reply Txt   ",st_sltp_ord_reply.c_reply_txt,sizeof(st_sltp_ord_reply.c_reply_txt));
            
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break;
          }
         case STOPLOSS_ORDR_UMS:
          {
           struct st_sltp_ord_ums st_sltp_ord_ums_data;
           memset(&st_sltp_ord_ums_data,0,sizeof(st_sltp_ord_ums_data));
           memcpy(&st_sltp_ord_ums_data,(struct st_sltp_ord_ums*)c_msg_data,sizeof(st_sltp_ord_ums_data));

           st_sltp_ord_ums_data.l_msg_typ = ntohl(st_sltp_ord_ums_data.l_msg_typ);
           st_sltp_ord_ums_data.l_scrip_cd = ntohl(st_sltp_ord_ums_data.l_scrip_cd);
           st_sltp_ord_ums_data.l_rate = ntohl(st_sltp_ord_ums_data.l_rate);
           st_sltp_ord_ums_data.l_qty = ntohl(st_sltp_ord_ums_data.l_qty);
           st_sltp_ord_ums_data.l_avaqty = ntohl(st_sltp_ord_ums_data.l_avaqty);
           st_sltp_ord_ums_data.l_msg_tag = ntohl(st_sltp_ord_ums_data.l_msg_tag);
           st_sltp_ord_ums_data.l_cust_cd = ntohl(st_sltp_ord_ums_data.l_cust_cd);
           st_sltp_ord_ums_data.s_ca_class = ntohs(st_sltp_ord_ums_data.s_ca_class);
           st_sltp_ord_ums_data.s_retention = ntohs(st_sltp_ord_ums_data.s_retention);
           
           st_sltp_ord_ums_data.l_msg_typ = fn_swap_long(st_sltp_ord_ums_data.l_msg_typ);
           st_sltp_ord_ums_data.l_scrip_cd = fn_swap_long(st_sltp_ord_ums_data.l_scrip_cd);
           st_sltp_ord_ums_data.l_rate = fn_swap_long(st_sltp_ord_ums_data.l_rate);
           st_sltp_ord_ums_data.l_qty = fn_swap_long(st_sltp_ord_ums_data.l_qty);
           st_sltp_ord_ums_data.l_avaqty = fn_swap_long(st_sltp_ord_ums_data.l_avaqty);
           st_sltp_ord_ums_data.l_msg_tag = fn_swap_long(st_sltp_ord_ums_data.l_msg_tag);
           st_sltp_ord_ums_data.l_cust_cd = fn_swap_long(st_sltp_ord_ums_data.l_cust_cd);
           st_sltp_ord_ums_data.s_ca_class = fn_swap_short(st_sltp_ord_ums_data.s_ca_class);
           st_sltp_ord_ums_data.s_retention = fn_swap_short(st_sltp_ord_ums_data.s_retention); 
           st_sltp_ord_ums_data.ll_ordr_id = fn_swap_ulnglng(st_sltp_ord_ums_data.ll_ordr_id);
           st_sltp_ord_ums_data.ll_location_id = fn_swap_ulnglng(st_sltp_ord_ums_data.ll_location_id);

           fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Typ     ",st_sltp_ord_ums_data.l_msg_typ);
           fprintf(fptr,"%-35s=:%ld:\n", "\t\t Scrip Code      ",st_sltp_ord_ums_data.l_scrip_cd);
           fprintf(fptr,"%-35s=:%lld:\n", "\t\t Order Id        ",st_sltp_ord_ums_data.ll_ordr_id);
           fprintf(fptr,"%-35s=:%ld:\n", "\t\t Rate            ",st_sltp_ord_ums_data.l_rate);
           fprintf(fptr,"%-35s=:%ld:\n", "\t\t Quantity        ",st_sltp_ord_ums_data.l_qty);
           fprintf(fptr,"%-35s=:%ld:\n", "\t\t Avilable Qty    ",st_sltp_ord_ums_data.l_avaqty);
           fprintf(fptr,"%-35s=:%ld:\n", "\t\t MSG Tag         ",st_sltp_ord_ums_data.l_msg_tag);
           fprintf(fptr,"%-35s=:%c:\n", "\t\t  Order Type      ",st_sltp_ord_ums_data.c_ordr_typ);
           fprintf(fptr,"%-35s=:%c:\n", "\t\t  Order Flow      ",st_sltp_ord_ums_data.c_ordr_flw);  
           fn_tmstmp_dmyhms_to_arr(st_sltp_ord_ums_data.st_timestamp_dmyhms, c_time_str);  
           fprintf(fptr, "%-35s=:%s:\n", "\t\t Trade Time", c_time_str);
           fn_bfprint_string(fptr," Client Id   \t\t\t=",st_sltp_ord_ums_data.c_client_id,sizeof(st_sltp_ord_ums_data.c_client_id));
           fprintf(fptr,"%-35s=:%ld:\n", "\t\t Cust Code      ",st_sltp_ord_ums_data.l_cust_cd);
           fprintf(fptr,"%-35s=:%d:\n", "\t\t CA Class        ",st_sltp_ord_ums_data.s_ca_class);
           fprintf(fptr,"%-35s=:%d:\n", "\t\t Retention       ",st_sltp_ord_ums_data.s_retention);
           fprintf(fptr,"%-35s=:%lld", "\t\t  Location ID     ",st_sltp_ord_ums_data.ll_location_id);
           fprintf(fptr, "\n%s\n\n", c_end);
           fflush(fptr);
           break;           
          } 
         case BSE_TRADE_CONFIRMATION:
          {
            struct st_trd_cnfrmtn st_trd_cnfrmtn_reply;
            memset(&st_trd_cnfrmtn_reply,0,sizeof(st_trd_cnfrmtn_reply));
            memcpy(&st_trd_cnfrmtn_reply,(struct st_trd_cnfrmtn*)c_msg_data,sizeof(st_trd_cnfrmtn_reply));

            st_trd_cnfrmtn_reply.l_msg_type  = ntohl(st_trd_cnfrmtn_reply.l_msg_type);
            st_trd_cnfrmtn_reply.l_total_rec = ntohl(st_trd_cnfrmtn_reply.l_total_rec);
            
            st_trd_cnfrmtn_reply.l_msg_type  = fn_swap_long(st_trd_cnfrmtn_reply.l_msg_type);
            st_trd_cnfrmtn_reply.l_total_rec = fn_swap_long(st_trd_cnfrmtn_reply.l_total_rec);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Message Typ     ",st_trd_cnfrmtn_reply.l_msg_type);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Total Records   ",st_trd_cnfrmtn_reply.l_total_rec); 
            l_tot_records = st_trd_cnfrmtn_reply.l_total_rec;  
            for( l_record = 0 ; l_record < l_tot_records ; l_record = l_record +1)
            {
             fprintf(fptr,"%-35s=:%ld:\n", "\n \t\t Trade Confirmation No   ",l_record);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_scrp_cd = ntohl(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_scrp_cd);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_trd_id  = ntohl(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_trd_id);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_rt      = ntohl(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_rt);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_trd_qty = ntohl(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_trd_qty);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_opp_member_id = ntohl(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_opp_member_id);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_opp_trader_id = ntohl(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_opp_trader_id);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_msg_tag = ntohl(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_msg_tag);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_custdn_cd = ntohl(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_custdn_cd);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].s_ca_class = ntohs(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].s_ca_class);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_scrp_cd = fn_swap_long(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_scrp_cd);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_trd_id = fn_swap_long(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_trd_id);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_rt = fn_swap_long(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_rt);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_trd_qty =fn_swap_long(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_trd_qty);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_opp_member_id=fn_swap_long(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_opp_member_id);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_opp_trader_id=fn_swap_long(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_opp_trader_id);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_msg_tag = fn_swap_long(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_msg_tag);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_custdn_cd =fn_swap_long(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_custdn_cd);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].s_ca_class = fn_swap_short(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].s_ca_class);
             st_trd_cnfrmtn_reply.st_trd_dtls[l_record].ll_transaction_id = fn_swap_ulnglng(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].ll_transaction_id);
                                              
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Scrip Code      ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_scrp_cd);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Trade Id       ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_trd_id);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Rate           ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_rt);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Trade Qty      ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_trd_qty);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Opp Member Id   ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_opp_member_id);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t Opp Trader Id      ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_opp_trader_id);
             fn_tmstmp_ymdhms_to_arr(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].st_timestmp_dmyhms, c_time_str);
             fprintf(fptr, "%-35s=:%s:\n", "\t\t Trade Time", c_time_str); 
             fn_bfprint_string(fptr," Client Id   \t\t\t=",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].c_client_id,sizeof(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].c_client_id));
             fprintf(fptr,"%-35s=:%c:\n", "\t\t Trnsctn Type ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].c_trnsctn_type);
             fprintf(fptr,"%-35s=:%c:\n", "\t\t Trade Flow  ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].c_trd_flw);
             fn_bfprint_string(fptr," Order Timestamp   ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].c_order_timestamp,sizeof(st_trd_cnfrmtn_reply.st_trd_dtls[l_record].c_order_timestamp));
             fprintf(fptr,"%-35s=:%lld:\n", "\t\t Transaction id ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].ll_transaction_id);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t  Msg Tag      ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_msg_tag);
             fprintf(fptr,"%-35s=:%ld:\n", "\t\t  Custodian Code ",st_trd_cnfrmtn_reply.st_trd_dtls[l_record].l_custdn_cd); 
             fprintf(fptr,"%-35s=:%d:\n", "\t\t   Ca Class   ", st_trd_cnfrmtn_reply.st_trd_dtls[l_record].s_ca_class); 
            }           
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break;
          }
         case KILL_MIN_FILL_ORDR:
         {
            struct st_kill_min_fill_ordr st_cancel_ordr;
            memset(&st_cancel_ordr,0,sizeof(st_cancel_ordr));
            memcpy(&st_cancel_ordr,(struct st_kill_min_fill_ordr*)c_msg_data,sizeof(st_cancel_ordr));
           
            st_cancel_ordr.l_msg_typ = ntohl(st_cancel_ordr.l_msg_typ);
            st_cancel_ordr.l_scrip_cd = ntohl(st_cancel_ordr.l_scrip_cd);
            st_cancel_ordr.l_killed_qty = ntohl(st_cancel_ordr.l_killed_qty);
            st_cancel_ordr.l_ltid_id = ntohl(st_cancel_ordr.l_ltid_id);
            st_cancel_ordr.l_msg_tag = ntohl(st_cancel_ordr.l_msg_tag);

            st_cancel_ordr.l_msg_typ = fn_swap_long(st_cancel_ordr.l_msg_typ);
            st_cancel_ordr.l_scrip_cd = fn_swap_long(st_cancel_ordr.l_scrip_cd);
            st_cancel_ordr.l_killed_qty = fn_swap_long(st_cancel_ordr.l_killed_qty);
            st_cancel_ordr.l_ltid_id = fn_swap_long(st_cancel_ordr.l_ltid_id);
            st_cancel_ordr.l_msg_tag = fn_swap_long(st_cancel_ordr.l_msg_tag);
            st_cancel_ordr.ll_ordr_id = fn_swap_ulnglng(st_cancel_ordr.ll_ordr_id);
             
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Type  ",st_cancel_ordr.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Scrip Code ",st_cancel_ordr.l_scrip_cd);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\t Order ID  ",st_cancel_ordr.ll_ordr_id);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t  Killed Qty ",st_cancel_ordr.l_killed_qty);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t LTID       ",st_cancel_ordr.l_ltid_id);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t MSG Tag    ",st_cancel_ordr.l_msg_tag);
           
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break;
          } 
         case OPN_INTRST_RQST:
          {
           if( st_sndhdr.l_slot_no == 1 )
           {
            struct st_opn_intrst_rply st_opn_intrst_data;
            memset(&st_opn_intrst_data,0,sizeof(st_opn_intrst_data)); 
            memcpy(&st_opn_intrst_data,(struct st_opn_intrst_rply*)c_msg_data,sizeof(st_opn_intrst_data));
            st_opn_intrst_data.l_msg_typ = ntohl(st_opn_intrst_data.l_msg_typ);
            st_opn_intrst_data.s_rply_cd = ntohs(st_opn_intrst_data.s_rply_cd);
            st_opn_intrst_data.s_filler  = ntohs(st_opn_intrst_data.s_filler);
            st_opn_intrst_data.l_msg_tag = ntohl(st_opn_intrst_data.l_msg_tag);
            
            st_opn_intrst_data.l_msg_typ = fn_swap_long(st_opn_intrst_data.l_msg_typ);
            st_opn_intrst_data.s_rply_cd = fn_swap_short(st_opn_intrst_data.s_rply_cd);
            st_opn_intrst_data.s_filler = fn_swap_short(st_opn_intrst_data.s_filler);
            st_opn_intrst_data.l_msg_tag = fn_swap_long(st_opn_intrst_data.l_msg_tag);

            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Type   ",st_opn_intrst_data.l_msg_typ);
            fprintf(fptr,"%-35s=:%d:\n", "\t\t  Reply Code  ",st_opn_intrst_data.s_rply_cd); 
            fprintf(fptr,"%-35s=:%d:\n", "\t\t  Filler      ",st_opn_intrst_data.s_filler);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag     ",st_opn_intrst_data.l_msg_tag); 
           }
           else
           {
            struct st_ums_opn_intrst st_ums_oi_data;
            memset(&st_ums_oi_data,0,sizeof(st_ums_oi_data));
            memcpy(&st_ums_oi_data,(struct st_ums_opn_intrst*)c_msg_data,sizeof(st_ums_oi_data));
            
            st_ums_oi_data.l_msg_typ = ntohl(st_ums_oi_data.l_msg_typ);
            st_ums_oi_data.l_no_of_recrd = ntohl(st_ums_oi_data.l_no_of_recrd);
            st_ums_oi_data.l_msg_tag = ntohl(st_ums_oi_data.l_msg_tag);
           
            st_ums_oi_data.l_msg_typ = fn_swap_long(st_ums_oi_data.l_msg_typ);
            st_ums_oi_data.l_no_of_recrd = fn_swap_long(st_ums_oi_data.l_no_of_recrd);
            st_ums_oi_data.l_msg_tag = fn_swap_long(st_ums_oi_data.l_msg_tag);
            
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Type   ",st_ums_oi_data.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Total No Of Records ",st_ums_oi_data.l_no_of_recrd);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag     ",st_ums_oi_data.l_msg_tag);
          
            l_tot_records =st_ums_oi_data.l_no_of_recrd; 

            for(l_record = 0 ; l_record < l_tot_records ; l_record++)
            {
             st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_contrct_id=ntohl(st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_contrct_id);
             st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_qty=ntohl(st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_qty);
             st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_chng=ntohl(st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_chng);
             st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_filler1=ntohl(st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_filler1);
             st_ums_oi_data.st_ums_opn_intrst_data[l_record].s_filler2=ntohs(st_ums_oi_data.st_ums_opn_intrst_data[l_record].s_filler2);
             st_ums_oi_data.st_ums_opn_intrst_data[l_record].s_filler3=ntohs(st_ums_oi_data.st_ums_opn_intrst_data[l_record].s_filler3);
             
              st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_contrct_id=fn_swap_long(st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_contrct_id);
              st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_qty=fn_swap_long(st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_qty);
              st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_chng=fn_swap_long(st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_chng);
              st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_filler1=fn_swap_long(st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_filler1);
              st_ums_oi_data.st_ums_opn_intrst_data[l_record].s_filler2=fn_swap_short(st_ums_oi_data.st_ums_opn_intrst_data[l_record].s_filler2);
              st_ums_oi_data.st_ums_opn_intrst_data[l_record].s_filler3=fn_swap_short(st_ums_oi_data.st_ums_opn_intrst_data[l_record].s_filler3);
              st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_val=fn_swap_ulnglng(st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_val);
              fprintf(fptr,"%-35s=:%ld:\n", "\t\t Record No ",l_record );
              fprintf(fptr,"%-35s=:%ld:\n", "\t\t Contract id  ",st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_contrct_id);
              fprintf(fptr,"%-35s=:%ld:\n", "\t\t Open Interest Qty ",st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_qty);
              fprintf(fptr,"%-35s=:%lld:\n", "\t\t Open Interest Val  ",st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_val);
              fprintf(fptr,"%-35s=:%ld:\n", "\t\t Open Interest Chng ",st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_opn_intrst_chng);
              fprintf(fptr,"%-35s=:%ld:\n", "\t\t l_Filler1    ",st_ums_oi_data.st_ums_opn_intrst_data[l_record].l_filler1);
              fprintf(fptr,"%-35s=:%d:\n", "\t\t s_FIller2    ",st_ums_oi_data.st_ums_opn_intrst_data[l_record].s_filler2);
              fprintf(fptr,"%-35s=:%d:\n", "\t\t s_FIller     ",st_ums_oi_data.st_ums_opn_intrst_data[l_record].s_filler3);
                
            } 
 
           }

            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break; 
          }
         case END_OF_UMS_OPN_INTRST:
          {
            struct st_end_of_oi_ums st_end_of_ums_data;
            memset(&st_end_of_ums_data,0,sizeof(st_end_of_ums_data));
            memcpy(&st_end_of_ums_data,(struct st_end_of_oi_ums*)c_msg_data,sizeof(st_end_of_ums_data));

            st_end_of_ums_data.l_msg_typ = ntohl(st_end_of_ums_data.l_msg_typ);
            st_end_of_ums_data.l_tot_records = ntohl(st_end_of_ums_data.l_tot_records);
            st_end_of_ums_data.l_msg_tag = ntohl(st_end_of_ums_data.l_msg_tag);

            st_end_of_ums_data.l_msg_typ = fn_swap_long(st_end_of_ums_data.l_msg_typ);
            st_end_of_ums_data.l_tot_records = fn_swap_long(st_end_of_ums_data.l_tot_records);
            st_end_of_ums_data.l_msg_tag = fn_swap_long(st_end_of_ums_data.l_msg_tag);

            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Type      ",st_end_of_ums_data.l_msg_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Total Records ",st_end_of_ums_data.l_tot_records);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t Msg Tag       ",st_end_of_ums_data.l_msg_tag);

            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
            break;
          }       
										/*** Ver 1.2 RRM UMS Handling Starts ****/ 
				case RRM_UMS :
					{
						struct st_rrm_alrt_ums st_rrm_ums_data;
						memset (&st_rrm_ums_data,0,sizeof(st_rrm_ums_data));
						memcpy (&st_rrm_ums_data,(struct st_rrm_ums_data*)c_msg_data,sizeof(st_rrm_ums_data));
					
						st_rrm_ums_data.li_msg_type = fn_swap_long(st_rrm_ums_data.li_msg_type);
						st_rrm_ums_data.si_no_records = fn_swap_short(st_rrm_ums_data.si_no_records);
						st_rrm_ums_data.si_rsrvd_fld1 = fn_swap_short(st_rrm_ums_data.si_rsrvd_fld1);
					  
            fprintf(fptr,"%-35s=:%ld:\n", " Message Type ",st_rrm_ums_data.li_msg_type);
            fprintf(fptr,"%-35s=:%d:\n", " No Of records",st_rrm_ums_data.si_no_records);
            fprintf(fptr,"%-35s=:%d:\n", " Rsrvd Field  ",st_rrm_ums_data.si_rsrvd_fld1);

						for(i_loop =0 ; i_loop < st_rrm_ums_data.si_no_records ; i_loop++)
						{
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_instrument_cd = fn_swap_long(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_instrument_cd);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_rate = fn_swap_long(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_rate);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_qntity = fn_swap_long(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_qntity);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_avlbl_qntity = fn_swap_long(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_qntity);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_odr_tag = fn_swap_long(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_odr_tag);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_rsvrd_fld2 = fn_swap_long(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_rsvrd_fld2);

							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_segment_typ = fn_swap_long(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_segment_typ);

							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_filler1 = fn_swap_long(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_filler1);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_event = fn_swap_short(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_event);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_error_cd =fn_swap_short(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_error_cd);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_retention = fn_swap_short(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_retention);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_filler2 = fn_swap_short(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_filler2);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_ordr_id = fn_swap_ulnglng(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_ordr_id); 
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_value = fn_swap_ulnglng(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_value);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_utlzd_value = fn_swap_ulnglng(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_utlzd_value);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_rrm_value = fn_swap_ulnglng(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_rrm_value);
							 st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_utlzd_prcnt = fn_swap_ulnglng(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_utlzd_prcnt);


            fprintf(fptr,"%-35s=:%ld:\n", "\t\tINSTRUMENT CODE",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_instrument_cd);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\tRATE",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_rate);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\tQUANTITY",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_qntity);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\tAVAILABLE QUANTITY",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_avlbl_qntity);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\tORDER TAG",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_odr_tag);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\tRESERVED FIELD",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_rsvrd_fld2);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\tSEGMENT TYPE",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_segment_typ);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\tFILLER1 ",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].li_filler1);

						fprintf(fptr,"%-35s=:%d:\n", "\t\tEVENT",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_event);
						fprintf(fptr,"%-35s=:%d:\n", "\t\tERROR CODE",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_error_cd);
						fprintf(fptr,"%-35s=:%d:\n", "\t\tRETENTION",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_retention);
						fprintf(fptr,"%-35s=:%d:\n", "\t\tFILLER2 ",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].si_filler2);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\tORDER ID ",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_ordr_id);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\tVALUE ",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_value);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\tUTILIZED VALUE ",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_utlzd_value);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\tRRM VALUE ",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_rrm_value);
            fprintf(fptr,"%-35s=:%lld:\n", "\t\tUTILIZED PERCENTAGE",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].ll_utlzd_prcnt);
            fprintf(fptr,"%-35s=:%s:\n", "\t\tADD/UPDATE FLAG",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].c_add_upd_flg);
            fprintf(fptr,"%-35s=:%s:\n", "\t\tFILLER4",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].c_filler4);
            fprintf(fptr,"%-35s=:%s:\n", "\t\tORDER TYPE   ",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].c_ordr_typ );
					 
            fn_bfprint_string(fptr,"\t\tMESSAGE \t\t\t ",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].c_message,sizeof(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].c_message));
            fprintf(fptr,"%-35s=:%s:\n", "\t\tCP CODE ",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].c_cp_code );
            fprintf(fptr,"%-35s=:%s:\n","\t\tFILLER5 ",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].c_filler5 );
						
            fn_bfprint_string(fptr,"\t\tCLIENT ID \t\t\t=",st_rrm_ums_data.st_sub_rrm_alrt[i_loop].c_client_id,sizeof(st_rrm_ums_data.st_sub_rrm_alrt[i_loop].c_client_id));
						}
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
						break;
					}
				case POS_ALERT_UMS:
					{
            struct st_pos_alrt_ums st_pos_ums_data;
            memset (&st_pos_ums_data,0,sizeof(st_pos_ums_data));
            memcpy (&st_pos_ums_data,(struct st_pos_ums_data*)c_msg_data,sizeof(st_pos_ums_data));

            st_pos_ums_data.li_msg_type = fn_swap_long(st_pos_ums_data.li_msg_type);
            st_pos_ums_data.si_no_records = fn_swap_short(st_pos_ums_data.si_no_records);
            st_pos_ums_data.si_resrvd_fld = fn_swap_short(st_pos_ums_data.si_resrvd_fld);

            fprintf(fptr,"%-35s=:%ld:\n", " Message Type ",st_pos_ums_data.li_msg_type);
            fprintf(fptr,"%-35s=:%d:\n", " No Of records",st_pos_ums_data.si_no_records);
            fprintf(fptr,"%-35s=:%d:\n", " Rsrvd Field  ",st_pos_ums_data.si_resrvd_fld);
				
						for ( i_loop = 0 ; i_loop < st_pos_ums_data.si_no_records ; i_loop++ )
						{
							st_pos_ums_data.st_ums_alrt[i_loop].li_instr_cd = fn_swap_long(st_pos_ums_data.st_ums_alrt[i_loop].li_instr_cd);
							st_pos_ums_data.st_ums_alrt[i_loop].li_alrt_type = fn_swap_long(st_pos_ums_data.st_ums_alrt[i_loop].li_alrt_type);
							st_pos_ums_data.st_ums_alrt[i_loop].li_percent = fn_swap_long(st_pos_ums_data.st_ums_alrt[i_loop].li_percent);
							st_pos_ums_data.st_ums_alrt[i_loop].li_percent = (st_pos_ums_data.st_ums_alrt[i_loop].li_percent *100);
							st_pos_ums_data.st_ums_alrt[i_loop].li_filler1 = fn_swap_long( st_pos_ums_data.st_ums_alrt[i_loop].li_filler1);
							st_pos_ums_data.st_ums_alrt[i_loop].li_filler2 = fn_swap_long( st_pos_ums_data.st_ums_alrt[i_loop].li_filler2);
							st_pos_ums_data.st_ums_alrt[i_loop].li_rsvrd_fld1 = fn_swap_long( st_pos_ums_data.st_ums_alrt[i_loop].li_rsvrd_fld1);
							st_pos_ums_data.st_ums_alrt[i_loop].li_seg_type = fn_swap_long(st_pos_ums_data.st_ums_alrt[i_loop].li_seg_type);
							st_pos_ums_data.st_ums_alrt[i_loop].li_filler3 = fn_swap_long( st_pos_ums_data.st_ums_alrt[i_loop].li_filler3);
							st_pos_ums_data.st_ums_alrt[i_loop].si_prdct_typ = fn_swap_short(st_pos_ums_data.st_ums_alrt[i_loop].si_prdct_typ);
							st_pos_ums_data.st_ums_alrt[i_loop].si_rrm_state = fn_swap_short(st_pos_ums_data.st_ums_alrt[i_loop].si_rrm_state);
							st_pos_ums_data.st_ums_alrt[i_loop].si_filler5 = fn_swap_short(st_pos_ums_data.st_ums_alrt[i_loop].si_filler5);
							st_pos_ums_data.st_ums_alrt[i_loop].si_clnt_type = fn_swap_short(st_pos_ums_data.st_ums_alrt[i_loop].si_clnt_type);
							 st_pos_ums_data.st_ums_alrt[i_loop].li_filler7 = fn_swap_ulnglng(st_pos_ums_data.st_ums_alrt[i_loop].li_filler7); 
							 st_pos_ums_data.st_ums_alrt[i_loop].li_filler8 = fn_swap_ulnglng(st_pos_ums_data.st_ums_alrt[i_loop].li_filler8); 
							 st_pos_ums_data.st_ums_alrt[i_loop].li_filler9 = fn_swap_ulnglng(st_pos_ums_data.st_ums_alrt[i_loop].li_filler9); 
							 st_pos_ums_data.st_ums_alrt[i_loop].li_filler10= fn_swap_ulnglng(st_pos_ums_data.st_ums_alrt[i_loop].li_filler10); 
							 st_pos_ums_data.st_ums_alrt[i_loop].li_filler11= fn_swap_ulnglng(st_pos_ums_data.st_ums_alrt[i_loop].li_filler11); 

            fprintf(fptr,"%-35s=:%ld:\n", "\t INSTRUMENT CODE\t ",st_pos_ums_data.st_ums_alrt[i_loop].li_instr_cd);
            fprintf(fptr,"%-35s=:%ld:\n", "\t ALERT TYPE \t ",st_pos_ums_data.st_ums_alrt[i_loop].li_alrt_type);
            fprintf(fptr,"%-35s=:%ld:\n", "\t PERCENTAGE \t ",st_pos_ums_data.st_ums_alrt[i_loop].li_percent);
            fprintf(fptr,"%-35s=:%ld:\n", "\t FILLER 1\t ",st_pos_ums_data.st_ums_alrt[i_loop].li_filler1);
            fprintf(fptr,"%-35s=:%ld:\n", "\t FILLER_2 \t ",st_pos_ums_data.st_ums_alrt[i_loop].li_filler2);
            fprintf(fptr,"%-35s=:%ld:\n", "\t RESERVED FIELD_1\t ",st_pos_ums_data.st_ums_alrt[i_loop].li_rsvrd_fld1);
            fprintf(fptr,"%-35s=:%ld:\n", "\t SEGEMENT TYPE\t ",st_pos_ums_data.st_ums_alrt[i_loop].li_seg_type);
            fprintf(fptr,"%-35s=:%ld:\n", "\t FILLER_3 \t ",st_pos_ums_data.st_ums_alrt[i_loop].li_filler3);
            fprintf(fptr,"%-35s=:%d:\n", " \tPRODUCT TYPE\t",st_pos_ums_data.st_ums_alrt[i_loop].si_prdct_typ);
            fprintf(fptr,"%-35s=:%d:\n", " \tRRM_STATE\t",st_pos_ums_data.st_ums_alrt[i_loop].si_rrm_state);
            fprintf(fptr,"%-35s=:%d:\n", " \tFILLER_5\t",st_pos_ums_data.st_ums_alrt[i_loop].si_filler5);
            fprintf(fptr,"%-35s=:%d:\n", " Rsrvd Field  ",st_pos_ums_data.st_ums_alrt[i_loop].si_clnt_type);
            fprintf(fptr,"%-35s=:%lld:\n", " \tFILLER_7\t",st_pos_ums_data.st_ums_alrt[i_loop].li_filler7);
            fprintf(fptr,"%-35s=:%lld:\n", " \tFILLER_8\t",st_pos_ums_data.st_ums_alrt[i_loop].li_filler8);
            fprintf(fptr,"%-35s=:%lld:\n", " \tFILLER_9\t",st_pos_ums_data.st_ums_alrt[i_loop].li_filler9);
            fprintf(fptr,"%-35s=:%lld:\n", " \tFILLER_10\t",st_pos_ums_data.st_ums_alrt[i_loop].li_filler10);
            fprintf(fptr,"%-35s=:%lld:\n", " \tFILLER_11\t",st_pos_ums_data.st_ums_alrt[i_loop].li_filler11);
            fprintf(fptr,"%-35s=:%s:\n", " \tINC_DEC_STTS\t",st_pos_ums_data.st_ums_alrt[i_loop].c_inc_dec_stts);
            fn_bfprint_string(fptr,"\t\tMESSAGE \t\t\t ",st_pos_ums_data.st_ums_alrt[i_loop].c_message,sizeof(st_pos_ums_data.st_ums_alrt[i_loop].c_message));
            fprintf(fptr,"%-35s=:%s:\n", "\t\tCP CODE ",st_pos_ums_data.st_ums_alrt[i_loop].c_cp_code );
            fprintf(fptr,"%-35s=:%s:\n","\t\tFILLER15 ",st_pos_ums_data.st_ums_alrt[i_loop].c_filler15 );
            fn_bfprint_string(fptr,"\t\tCLIENT ID \t\t\t=",st_pos_ums_data.st_ums_alrt[i_loop].c_client_id,sizeof(st_pos_ums_data.st_ums_alrt[i_loop].c_client_id));
						}
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
						break;
					}
				case BROKER_SUSPENDED_UMS:
					{
            struct st_broker_susp st_brkr_susp_ums;
            memset (&st_brkr_susp_ums,0,sizeof(st_brkr_susp_ums));
            memcpy (&st_brkr_susp_ums,(struct st_brkr_susp_ums*)c_msg_data,sizeof(st_brkr_susp_ums));
            st_brkr_susp_ums.li_msg_type = fn_swap_long(st_brkr_susp_ums.li_msg_type);
            st_brkr_susp_ums.li_filler = fn_swap_long(st_brkr_susp_ums.li_filler);

            fprintf(fptr,"%-35s=:%ld:\n", "\t\t MESSAGE TYPE ",st_brkr_susp_ums.li_msg_type);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t FILLER",st_brkr_susp_ums.li_filler);
            fprintf(fptr,"%-35s=:%s:\n", "\t\tMARKET FLAG",st_brkr_susp_ums.c_mrkt_flg);
            fprintf(fptr,"%-35s=:%s:\n", "\t\tREASON",st_brkr_susp_ums.c_reason);
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
						break;
					}
				case CAP_ADEQ_UMS_70 :
				case CAP_ADEQ_UMS_80 :
				case CAP_ADEQ_UMS_85 :
				{
            struct st_cap_adeq_ums ptr_st_cap_adeq_ums;
            memset (&ptr_st_cap_adeq_ums,0,sizeof(ptr_st_cap_adeq_ums));
            memcpy (&ptr_st_cap_adeq_ums,(struct ptr_st_cap_adeq_ums*)c_msg_data,sizeof(ptr_st_cap_adeq_ums));
            ptr_st_cap_adeq_ums.li_msg_type = fn_swap_long(ptr_st_cap_adeq_ums.li_msg_type);
            ptr_st_cap_adeq_ums.li_term_id = fn_swap_long(ptr_st_cap_adeq_ums.li_term_id);

            fprintf(fptr,"%-35s=:%ld:\n", "\t\t MESSAGE TYPE ",ptr_st_cap_adeq_ums.li_msg_type);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t NO oF RECORDS",ptr_st_cap_adeq_ums.li_term_id);
            fprintf(fptr,"%-35s=:%s:\n", "\t\tMESSAGE",ptr_st_cap_adeq_ums.c_message);
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
						break;
				}	
				case CANCELLED_ORDR_UMS :
				{
            struct st_cancel_ordr_ums ptr_st_cancl_ordr_ums;
            memset (&ptr_st_cancl_ordr_ums,0,sizeof(ptr_st_cancl_ordr_ums));
            memcpy (&ptr_st_cancl_ordr_ums,(struct ptr_st_cancl_ordr_ums*)c_msg_data,sizeof(ptr_st_cancl_ordr_ums));
            ptr_st_cancl_ordr_ums.li_msg_type = fn_swap_long(ptr_st_cancl_ordr_ums.li_msg_type);
            ptr_st_cancl_ordr_ums.li_no_of_recrds = fn_swap_long(ptr_st_cancl_ordr_ums.li_no_of_recrds);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t MESSAGE TYPE ",ptr_st_cancl_ordr_ums.li_msg_type);
            fprintf(fptr,"%-35s=:%ld:\n", "\t\t NO oF RECORDS",ptr_st_cancl_ordr_ums.li_no_of_recrds);
						for ( i_loop = 0 ; i_loop < ptr_st_cancl_ordr_ums.li_no_of_recrds ; i_loop++ )
						{
							ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_instr_code =
							fn_swap_long(ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_instr_code);
							ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_msg_tag =
							fn_swap_long(ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_msg_tag);
							ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_qntity =
							fn_swap_long(ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_qntity);
							ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_reason =
							fn_swap_long(ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_reason);
							ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].ll_ordr_id =
							fn_swap_ulnglng(ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].ll_ordr_id);
							
            	fprintf(fptr,"%-35s=:%ld:\n", "\t INSTRUMENT CODE\t ",ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_instr_code);
            	fprintf(fptr,"%-35s=:%ld:\n", "\t MESSAGE TAG \t ",ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_msg_tag);
            	fprintf(fptr,"%-35s=:%ld:\n", "\t QUANTITY \t ",ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_qntity);
            fprintf(fptr,"%-35s=:%ld:\n", " \tREASON\t",ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].li_reason);
            fprintf(fptr,"%-35s=:%lld:\n", " \tORDER ID \t",ptr_st_cancl_ordr_ums.st_cncl_ums_rply[i_loop].ll_ordr_id);
						}
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
						break;
				}
				case PROTOCOL_MSG_CONN_REF :/*** 800 **/
				{
            struct st_conn_refused_reply ptr_st_conn_refused_reply;
            memset (&ptr_st_conn_refused_reply,0,sizeof(ptr_st_conn_refused_reply));
            memcpy (&ptr_st_conn_refused_reply,(struct ptr_st_conn_refused_reply*)c_msg_data,sizeof(ptr_st_conn_refused_reply));
					  ptr_st_conn_refused_reply.li_msg_type=fn_swap_long(ptr_st_conn_refused_reply.li_msg_type);
					  ptr_st_conn_refused_reply.li_slot_no=fn_swap_long(ptr_st_conn_refused_reply.li_slot_no);
            ptr_st_conn_refused_reply.li_msg_tag=fn_swap_long(ptr_st_conn_refused_reply.li_msg_tag);
            ptr_st_conn_refused_reply.li_err_no=fn_swap_long(ptr_st_conn_refused_reply.li_err_no);

            fprintf(fptr,"%-35s=:%ld:\n","\t\t MESSAGE TYPE",ptr_st_conn_refused_reply.li_msg_type);
            fprintf(fptr,"%-35s=:%ld:\n","\t\t SLOT NO",ptr_st_conn_refused_reply.li_slot_no);
            fprintf(fptr,"%-35s=:%ld:\n","\t\t MESSG TAG ",ptr_st_conn_refused_reply.li_msg_tag);
            fprintf(fptr,"%-35s=:%ld:\n","\t\t ERROR NO ",ptr_st_conn_refused_reply.li_err_no);
            fprintf(fptr,"%-35s=:%s:\n","\t\t ERROR MESSAGE ",ptr_st_conn_refused_reply.c_err_txt);
            fprintf(fptr, "\n%s\n\n", c_end);
            fflush(fptr);
						break;
				}
				case CAP_UTLZTN_UMS:
				{
					struct st_cap_utlztn ptr_st_cap_utlztn;
					memset (&ptr_st_cap_utlztn,0,sizeof(ptr_st_cap_utlztn));
					memcpy (&ptr_st_cap_utlztn,(struct ptr_st_cap_utlztn*)c_msg_data,sizeof(ptr_st_cap_utlztn));
					ptr_st_cap_utlztn.li_msg_type =fn_swap_long(ptr_st_cap_utlztn.li_msg_type);
					ptr_st_cap_utlztn.li_refrnc_tag =fn_swap_long(ptr_st_cap_utlztn.li_refrnc_tag);
				  ptr_st_cap_utlztn.si_alert_typ = fn_swap_short( ptr_st_cap_utlztn.si_alert_typ);
					ptr_st_cap_utlztn.si_no_records = fn_swap_short(ptr_st_cap_utlztn.si_no_records);
					ptr_st_cap_utlztn.si_resrvd_fld = fn_swap_short(ptr_st_cap_utlztn.si_resrvd_fld);
					ptr_st_cap_utlztn.si_filler = fn_swap_short(ptr_st_cap_utlztn.si_filler);

					fprintf(fptr,"%-35s=:%ld:\n","\t\t MESSAGE TYPE",ptr_st_cap_utlztn.li_msg_type);
					fprintf(fptr,"%-35s=:%ld:\n","\t\t REFERENCE TAG ",ptr_st_cap_utlztn.li_refrnc_tag);
					fprintf(fptr,"%-35s=:%d:\n","\t\t ALERT TYPE",ptr_st_cap_utlztn.si_alert_typ);
					fprintf(fptr,"%-35s=:%d:\n","\t\t No OF RECORDS",ptr_st_cap_utlztn.si_no_records);
					fprintf(fptr,"%-35s=:%d:\n","\t\t RESERVED FIELD",ptr_st_cap_utlztn.si_resrvd_fld);
					fprintf(fptr,"%-35s=:%d:\n","\t\t FILLER       ",ptr_st_cap_utlztn.si_filler);
          l_tot_records =ptr_st_cap_utlztn.si_no_records; 
          for(l_record = 0 ; l_record < l_tot_records ; l_record++)
          {
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_member_cd = fn_swap_long(ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_member_cd);
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_rsrvd_fld1 = fn_swap_long(ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_rsrvd_fld1);
						 ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_filler1 = fn_swap_long(ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_filler1);
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_percnt_utlzd = fn_swap_long(ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_percnt_utlzd);
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_tot_col = fn_swap_ulnglng(ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_tot_col);
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_utlz_col = fn_swap_ulnglng(ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_utlz_col);
					  ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_avail_col = fn_swap_ulnglng( ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_avail_col);
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_filler2 = fn_swap_ulnglng( ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_filler2);
						 ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler3 = fn_swap_short (ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler3);
						 ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_inc_dec_stts = fn_swap_short ( ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_inc_dec_stts );
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_membr_stts = fn_swap_short (  ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_membr_stts);
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_rsvrd_fld2 = fn_swap_short ( ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_rsvrd_fld2 );
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler4 = fn_swap_short (ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler4);
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler5 = fn_swap_short (ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler5);
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler6 = fn_swap_short (ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler6);
						ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler7 = fn_swap_short (ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler7);
						
					fprintf(fptr,"%-35s=:%ld:\n","\t\t MEMBER CODE ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_member_cd);
					fprintf(fptr,"%-35s=:%ld:\n","\t\t RESERVED FLD1",ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_rsrvd_fld1);
					fprintf(fptr,"%-35s=:%ld:\n","\t\t FILLER 1 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_filler1);
					fprintf(fptr,"%-35s=:%ld:\n","\t\t UTILIZED PERCENT ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].li_percnt_utlzd);
					fprintf(fptr,"%-35s=:%lld:\n","\t\t TOTAL COLATERAL ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_tot_col);
					fprintf(fptr,"%-35s=:%lld:\n","\t\t UTILIZED COLATERAL ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_utlz_col);
					fprintf(fptr,"%-35s=:%lld:\n","\t\t AVAILABLE COLATERAL ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_avail_col);
					fprintf(fptr,"%-35s=:%lld:\n","\t\t FILLER2   ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].ll_filler2);
					fprintf(fptr,"%-35s=:%d:\n","\t\t FILLER3   ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler3);
					fprintf(fptr,"%-35s=:%d:\n","\t\t INC/DEC STATUS  ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_inc_dec_stts);
					fprintf(fptr,"%-35s=:%d:\n","\t\t MEMBER  STATUS  ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_membr_stts);
					fprintf(fptr,"%-35s=:%d:\n","\t\t RESERVED FIELD2 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_rsvrd_fld2);
					fprintf(fptr,"%-35s=:%d:\n","\t\t FILLER4 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler4);
					fprintf(fptr,"%-35s=:%d:\n","\t\t FILLER5 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler5);
					fprintf(fptr,"%-35s=:%d:\n","\t\t FILLER6 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler6);
					fprintf(fptr,"%-35s=:%d:\n","\t\t FILLER7 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].si_filler7);
					fprintf(fptr,"%-35s=:%c:\n","\t\t MARKET TYPE ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].c_mrkt_typ);
					fprintf(fptr,"%-35s=:%c:\n","\t\t RSVRD_FLD3 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].c_rsvrd_fld3);
					fprintf(fptr,"%-35s=:%c:\n","\t\t RSVRD_FLD4 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].c_rsvrd_fld4);
					fprintf(fptr,"%-35s=:%c:\n","\t\t MEMBER TYPE ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].c_membr_type);
					fprintf(fptr,"%-35s=:%c:\n","\t\t RSVRD_FLD5 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].c_rsvrd_fld5);
					fprintf(fptr,"%-35s=:%c:\n","\t\t FILLER8 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].c_filler8);
					fprintf(fptr,"%-35s=:%s:\n","\t\t MESSAGE ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].c_message);
					fprintf(fptr,"%-35s=:%s:\n","\t\t RSVRD_FLD6 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].c_rsvrd_fld6);
					fprintf(fptr,"%-35s=:%s:\n","\t\t RSVRD_FLD7 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].c_rsvrd_fld7);
					fprintf(fptr,"%-35s=:%s:\n","\t\t FILLER9 ",ptr_st_cap_utlztn.st_sub_utlztn[l_record].c_filler9);
						 
					}	
					fprintf(fptr, "\n%s\n\n", c_end);
          fflush(fptr);
          break;

					break;
				}	
				/*** Ver 1.2 END ****/
				/** Ver 1.3 starts ***/
				case ORDR_RATE_UMS :
				case RESUME_ORDR_REQ_UMS:
					{
								struct st_ordr_rate_ums ptr_st_ordr_rt;
								memset(&ptr_st_ordr_rt,0,sizeof( struct st_ordr_rate_ums));
								memcpy(&ptr_st_ordr_rt,(struct st_ordr_rate_ums *)c_msg_data,sizeof(ptr_st_ordr_rt));
								ptr_st_ordr_rt.li_msg_type = fn_swap_long(ptr_st_ordr_rt.li_msg_type);
								ptr_st_ordr_rt.li_order_rate = fn_swap_long(ptr_st_ordr_rt.li_order_rate);
								ptr_st_ordr_rt.li_max_slots = fn_swap_long(ptr_st_ordr_rt.li_max_slots);
								ptr_st_ordr_rt.li_filler1 = fn_swap_long(ptr_st_ordr_rt.li_filler1);
								ptr_st_ordr_rt.si_filler2 = fn_swap_short(ptr_st_ordr_rt.si_filler2);
								ptr_st_ordr_rt.si_filler3 = fn_swap_short(ptr_st_ordr_rt.si_filler3);
								fprintf(fptr,"%-35s=:%ld:\n", " Message Type",ptr_st_ordr_rt.li_msg_type);
								fprintf(fptr,"%-35s=:%ld:\n", " Order Rate",ptr_st_ordr_rt.li_order_rate);
								fprintf(fptr,"%-35s=:%ld:\n", " Max Slots ",ptr_st_ordr_rt.li_max_slots);
								fprintf(fptr,"%-35s=:%ld:\n", " Filler1",ptr_st_ordr_rt.li_filler1);
								fprintf(fptr,"%-35s=:%d:\n", " Filler2",ptr_st_ordr_rt.si_filler2);
								fprintf(fptr,"%-35s=:%d:\n", " Filler3",ptr_st_ordr_rt.si_filler3);
                fprintf(fptr, "\n%s\n\n", c_end);
                fflush(fptr);
								break;
					}
					/*** Ver 1.3 ends ***/
         default:
              {
                 fprintf(fptr,"Inside default -- Message code =:%d:\n", li_msg_typ);
                 fn_bfprint_asc( fptr, c_msg_data, c_ServiceName, c_errmsg );
                 fflush(fptr);
                 break;
              }
        } /*** End of li_msg_typ ***/

        return(0);
}


/******************************************************************************/
/* This function takes a char buffer as input and prints every byte of the    */
/* buffer to the file in ascii mode                                           */
/* INPUT PARAMETERS                                                           */
/*      fptr         - File handler                                           */
/*      c_msg_data   - Structure to be printed                                */
/*      c_ServiceName- Name of the service                                    */
/*      c_errmsg     - Error String                                           */
/* OUTPUT PARAMETERS                                                          */
/*          0 - Success                                                       */
/******************************************************************************/

int fn_bfprint_asc(FILE *fptr,
                  void *c_msg_data,
                  char *c_ServiceName,
                  char *c_errmsg)
{
  int     i_loop;
  char    c_dname[22];
  char    c_start[20];
  char    c_end[15];
  time_t  tm_stmp;

  tm_stmp = time(NULL);
  strftime(c_dname, 21, "%d-%b-%Y %H:%M:%S", localtime(&tm_stmp));
  fprintf(fptr,"Timestamp :%s:\n", c_dname);

  strcpy(c_start, "Printing New packet");
  strcpy(c_end, "Packet Printed");
  fprintf(fptr,"%s\n", c_start);

  for(i_loop = 0; i_loop < MAX_SCK_MSG; i_loop++)
  {
    fprintf(fptr, ":%d: ", (char *)c_msg_data + i_loop);
    if(i_loop != 0 && ((i_loop + 1) % 16) == 0)
    {
      fprintf(fptr, "\n");
    }
  }
  fprintf(fptr,"\n%s\n\n", c_end);
  fflush(fptr);
  return 0;
}
    
