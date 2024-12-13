/**************************************************************************/
/* File 			 : fn_nw_to_host_format.c																		*/
/* Description : network to host conversion Function Definitions 					*/ 
/* Ver 1.0		 : 02-Dec-2012	Sandip Tambe																*/
/* Ver 1.1		 : 16-May-2013	Sandeep Patil	(	Msg Type 2020 Handling )		*/	
/**************************************************************************/
/**** C header ****/
#include <string.h>
#include <stdlib.h>
#include <fn_log.h>
#include <netinet/in.h>
#include <ctype.h>
#include <fn_fobse_bcast.h>
#include <fn_imlmsgs_fno.h>

/*** 2002 session change broadcast ****/
void fn_cnvt_to_hfmt_swap_sssn_chng_brd ( struct st_sssn_chng_brd *ptr_st_var_sssn_chng )
{
  ptr_st_var_sssn_chng->li_msg_type = fn_swap_long( ptr_st_var_sssn_chng->li_msg_type );
  ptr_st_var_sssn_chng->li_msg_type = ntohl( ptr_st_var_sssn_chng->li_msg_type );
  ptr_st_var_sssn_chng->si_cur_sssn = fn_swap_short( ptr_st_var_sssn_chng->si_cur_sssn );
  ptr_st_var_sssn_chng->si_cur_sssn = ntohs( ptr_st_var_sssn_chng->si_cur_sssn );
  ptr_st_var_sssn_chng->si_hrs = fn_swap_short( ptr_st_var_sssn_chng->si_hrs );
  ptr_st_var_sssn_chng->si_hrs = ntohs( ptr_st_var_sssn_chng->si_hrs );
  ptr_st_var_sssn_chng->si_mins = fn_swap_short( ptr_st_var_sssn_chng->si_mins );
  ptr_st_var_sssn_chng->si_mins = ntohs( ptr_st_var_sssn_chng->si_mins );
  ptr_st_var_sssn_chng->si_scnds = fn_swap_short( ptr_st_var_sssn_chng->si_scnds );
  ptr_st_var_sssn_chng->si_scnds = ntohs( ptr_st_var_sssn_chng->si_scnds );
  ptr_st_var_sssn_chng->si_mlscnds = fn_swap_short( ptr_st_var_sssn_chng->si_mlscnds );
  ptr_st_var_sssn_chng->si_mlscnds = ntohs( ptr_st_var_sssn_chng->si_mlscnds );
}

/***** Time Broadcast 2001 ********/

void fn_cnvt_to_hfmt_swap_time_brd ( struct st_time_brd *ptr_st_var_time )
{
  ptr_st_var_time->li_msg_type = fn_swap_long ( ptr_st_var_time->li_msg_type );
  ptr_st_var_time->li_msg_type = ntohl ( ptr_st_var_time->li_msg_type );
  ptr_st_var_time->si_hrs = fn_swap_short ( ptr_st_var_time->si_hrs );
  ptr_st_var_time->si_hrs = ntohs ( ptr_st_var_time->si_hrs );
  ptr_st_var_time->si_mins = fn_swap_short ( ptr_st_var_time->si_mins );
  ptr_st_var_time->si_mins = ntohs ( ptr_st_var_time->si_mins );
  ptr_st_var_time->si_scnds = fn_swap_short ( ptr_st_var_time->si_scnds );
  ptr_st_var_time->si_scnds = ntohs ( ptr_st_var_time->si_scnds );
  ptr_st_var_time->si_mlscnds = fn_swap_short ( ptr_st_var_time->si_mlscnds );
  ptr_st_var_time->si_mlscnds = ntohs ( ptr_st_var_time->si_mlscnds );
}

/***** 2004 Message Broadcast *********/

void fn_cnvt_to_hfmt_swap_upd_msg_brd ( struct st_news_brd *ptr_st_var_news_brd )
{
  ptr_st_var_news_brd->li_msg_type= fn_swap_long( ptr_st_var_news_brd->li_msg_type  );
  ptr_st_var_news_brd->li_msg_type= ntohl( ptr_st_var_news_brd->li_msg_type );
  ptr_st_var_news_brd->si_hrs= fn_swap_short( ptr_st_var_news_brd->si_hrs );
  ptr_st_var_news_brd->si_hrs= ntohs( ptr_st_var_news_brd->si_hrs );
  ptr_st_var_news_brd->si_mins= fn_swap_short( ptr_st_var_news_brd->si_mins );
  ptr_st_var_news_brd->si_mins= ntohs( ptr_st_var_news_brd->si_mins );
  ptr_st_var_news_brd->si_scnds= fn_swap_short( ptr_st_var_news_brd->si_scnds );
  ptr_st_var_news_brd->si_scnds= ntohs( ptr_st_var_news_brd->si_scnds );
  ptr_st_var_news_brd->si_mlscnds= fn_swap_short( ptr_st_var_news_brd->si_mlscnds );
  ptr_st_var_news_brd->si_mlscnds= ntohs( ptr_st_var_news_brd->si_mlscnds );
  ptr_st_var_news_brd->si_nws_ctgry= fn_swap_short( ptr_st_var_news_brd->si_nws_ctgry );
  ptr_st_var_news_brd->si_nws_ctgry= ntohs( ptr_st_var_news_brd->si_nws_ctgry );
  ptr_st_var_news_brd->li_nws_id= fn_swap_long( ptr_st_var_news_brd->li_nws_id  );
  ptr_st_var_news_brd->li_nws_id= ntohl( ptr_st_var_news_brd->li_nws_id );
}

/**** 2007 Market Picture Broadcast **********/

void fn_cnvt_to_hfmt_swap_upd_mkt_info_dtls(struct st_mkt_picture_dtls *ptr_st_var_mkt_picture_dtls)
{
  int i_cnt,i_count1;
  ptr_st_var_mkt_picture_dtls->li_msg_type = fn_swap_long(ptr_st_var_mkt_picture_dtls->li_msg_type);
  ptr_st_var_mkt_picture_dtls->li_msg_type = ntohl ( ptr_st_var_mkt_picture_dtls->li_msg_type );
  ptr_st_var_mkt_picture_dtls->si_hrs = fn_swap_short( ptr_st_var_mkt_picture_dtls->si_hrs );
  ptr_st_var_mkt_picture_dtls->si_hrs = ntohs( ptr_st_var_mkt_picture_dtls->si_hrs );
  ptr_st_var_mkt_picture_dtls->si_mins = fn_swap_short( ptr_st_var_mkt_picture_dtls->si_mins );
  ptr_st_var_mkt_picture_dtls->si_mins = ntohs( ptr_st_var_mkt_picture_dtls->si_mins );
  ptr_st_var_mkt_picture_dtls->si_scnds = fn_swap_short( ptr_st_var_mkt_picture_dtls->si_scnds );
  ptr_st_var_mkt_picture_dtls->si_scnds = ntohs( ptr_st_var_mkt_picture_dtls->si_scnds );
  ptr_st_var_mkt_picture_dtls->si_mlscnds = fn_swap_short( ptr_st_var_mkt_picture_dtls->si_mlscnds);
  ptr_st_var_mkt_picture_dtls->si_mlscnds = ntohs( ptr_st_var_mkt_picture_dtls->si_mlscnds );
  ptr_st_var_mkt_picture_dtls->si_trdng_sssn = fn_swap_short( ptr_st_var_mkt_picture_dtls->si_trdng_sssn );
  ptr_st_var_mkt_picture_dtls->si_trdng_sssn = ntohs( ptr_st_var_mkt_picture_dtls->si_trdng_sssn );
  ptr_st_var_mkt_picture_dtls->si_no_recs = fn_swap_short( ptr_st_var_mkt_picture_dtls->si_no_recs );
  ptr_st_var_mkt_picture_dtls->si_no_recs = ntohs( ptr_st_var_mkt_picture_dtls->si_no_recs );
  for ( i_cnt = 0 ; i_cnt < ptr_st_var_mkt_picture_dtls->si_no_recs ; i_cnt++ )
  {
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_scrp_cd = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_scrp_cd );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_scrp_cd = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_scrp_cd );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_opn_rt = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_opn_rt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_opn_rt = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_opn_rt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_cls_rt = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_cls_rt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_cls_rt = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_cls_rt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_high_rt = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_high_rt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_high_rt = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_high_rt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_low_rt = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_low_rt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_low_rt = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_low_rt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_no_of_trd = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_no_of_trd  );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_no_of_trd = ntohl(ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_no_of_trd);
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_vol = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_vol  );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_vol = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_vol );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_val = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_val  );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_val = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_val );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lst_trd_qty = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lst_trd_qty );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lst_trd_qty = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lst_trd_qty );
   ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lst_trd_rt = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lst_trd_rt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lst_trd_rt = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lst_trd_rt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_tot_buy_qty = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_tot_buy_qty );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_tot_buy_qty = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_tot_buy_qty );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_tot_sell_qty = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_tot_sell_qty );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_tot_sell_qty = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_tot_sell_qty );
/*****************************************************************************************/
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lowr_ckt_lmt = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lowr_ckt_lmt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lowr_ckt_lmt = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_lowr_ckt_lmt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_upper_ckt_lmt = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_upper_ckt_lmt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_upper_ckt_lmt = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_upper_ckt_lmt );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_wightd_avg = fn_swap_long( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_wightd_avg );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_wightd_avg = ntohl( ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].li_wightd_avg );
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].si_no_of_prc_pnts = fn_swap_short (ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].si_no_of_prc_pnts);
    ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].si_no_of_prc_pnts = ntohs (ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].si_no_of_prc_pnts);

	  for(i_count1 =0 ;i_count1 <ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].si_no_of_prc_pnts;i_count1++)
	  {
			ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_bst_buy_rt = fn_swap_long(ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_bst_buy_rt);
			ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_bst_buy_rt = ntohl(ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_bst_buy_rt);

			ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_tot_buy_qty = fn_swap_long(ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_tot_buy_qty);
			ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_tot_buy_qty = ntohl(ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_tot_buy_qty);

			ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_bst_sell_rt = fn_swap_long(ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_bst_sell_rt);
			ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_bst_sell_rt = ntohl (ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_bst_sell_rt);

			ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_tot_sell_qty = fn_swap_long(ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_tot_sell_qty);
			ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_tot_sell_qty = ntohl(ptr_st_var_mkt_picture_dtls->st_mkt_pic[i_cnt].st_bst_rt_qty[i_count1].li_tot_sell_qty);
		
	  } 
 	}
}

/********** Open Price Broadcast 2013 **************/

void fn_cnvt_to_hfmt_swap_opn_prc_brd ( struct st_open_price_brd *ptr_st_var_open_price )
{
  int i_cnt;
  ptr_st_var_open_price->st_opn_prc.li_msg_type = fn_swap_long( ptr_st_var_open_price->st_opn_prc.li_msg_type )
;
  ptr_st_var_open_price->st_opn_prc.li_msg_type = ntohl( ptr_st_var_open_price->st_opn_prc.li_msg_type );
  ptr_st_var_open_price->st_opn_prc.si_hrs = fn_swap_short( ptr_st_var_open_price->st_opn_prc.si_hrs);
  ptr_st_var_open_price->st_opn_prc.si_hrs = ntohs( ptr_st_var_open_price->st_opn_prc.si_hrs );
  ptr_st_var_open_price->st_opn_prc.si_mins = fn_swap_short( ptr_st_var_open_price->st_opn_prc.si_mins );
  ptr_st_var_open_price->st_opn_prc.si_mins = ntohs( ptr_st_var_open_price->st_opn_prc.si_mins );
  ptr_st_var_open_price->st_opn_prc.si_scnds = fn_swap_short( ptr_st_var_open_price->st_opn_prc.si_scnds );
  ptr_st_var_open_price->st_opn_prc.si_scnds = ntohs( ptr_st_var_open_price->st_opn_prc.si_scnds );
  ptr_st_var_open_price->st_opn_prc.si_mlscnds = fn_swap_short( ptr_st_var_open_price->st_opn_prc.si_mlscnds );
  ptr_st_var_open_price->st_opn_prc.si_mlscnds = ntohs( ptr_st_var_open_price->st_opn_prc.si_mlscnds
 );
  ptr_st_var_open_price->st_opn_prc.si_no_recs = fn_swap_short( ptr_st_var_open_price->st_opn_prc.si_no_recs );
  ptr_st_var_open_price->st_opn_prc.si_no_recs = ntohs( ptr_st_var_open_price->st_opn_prc.si_no_recs);
  for ( i_cnt = 0 ; i_cnt < ptr_st_var_open_price->st_opn_prc.si_no_recs ; i_cnt++ )
  {
    ptr_st_var_open_price->st_opn_prc.st_prc[i_cnt].li_scrip_cd = fn_swap_long( ptr_st_var_open_price->st_opn_prc.st_prc[i_cnt].li_scrip_cd );
    ptr_st_var_open_price->st_opn_prc.st_prc[i_cnt].li_scrip_cd = ntohl( ptr_st_var_open_price->st_opn_prc.st_prc[i_cnt].li_scrip_cd );
    ptr_st_var_open_price->st_opn_prc.st_prc[i_cnt].li_price = fn_swap_long( ptr_st_var_open_price->st_opn_prc.st_prc[i_cnt].li_price );
    ptr_st_var_open_price->st_opn_prc.st_prc[i_cnt].li_price = ntohl( ptr_st_var_open_price->st_opn_prc.st_prc[i_cnt].li_price );
	}
}

/**** 2014 Closing Price Broadcast *********/

void fn_cnvt_to_hfmt_swap_cls_prc_brd ( struct st_cls_price_brd *ptr_st_var_cls_price )
{
  int i_cnt;
  ptr_st_var_cls_price->st_cls_prc.li_msg_type = fn_swap_long( ptr_st_var_cls_price->st_cls_prc.li_msg_type )
;
  ptr_st_var_cls_price->st_cls_prc.li_msg_type = ntohl( ptr_st_var_cls_price->st_cls_prc.li_msg_type);
  ptr_st_var_cls_price->st_cls_prc.si_hrs = fn_swap_short( ptr_st_var_cls_price->st_cls_prc.si_hrs );
  ptr_st_var_cls_price->st_cls_prc.si_hrs = ntohs( ptr_st_var_cls_price->st_cls_prc.si_hrs );
  ptr_st_var_cls_price->st_cls_prc.si_mins = fn_swap_short( ptr_st_var_cls_price->st_cls_prc.si_mins);
  ptr_st_var_cls_price->st_cls_prc.si_mins = ntohs( ptr_st_var_cls_price->st_cls_prc.si_mins );
  ptr_st_var_cls_price->st_cls_prc.si_scnds = fn_swap_short( ptr_st_var_cls_price->st_cls_prc.si_scnds );
  ptr_st_var_cls_price->st_cls_prc.si_scnds = ntohs( ptr_st_var_cls_price->st_cls_prc.si_scnds );
  ptr_st_var_cls_price->st_cls_prc.si_mlscnds = fn_swap_short( ptr_st_var_cls_price->st_cls_prc.si_mlscnds );
  ptr_st_var_cls_price->st_cls_prc.si_mlscnds = ntohs( ptr_st_var_cls_price->st_cls_prc.si_mlscnds );
  ptr_st_var_cls_price->st_cls_prc.si_no_recs = fn_swap_short( ptr_st_var_cls_price->st_cls_prc.si_no_recs );
  ptr_st_var_cls_price->st_cls_prc.si_no_recs = ntohs( ptr_st_var_cls_price->st_cls_prc.si_no_recs ) ;
  for ( i_cnt = 0 ; i_cnt < ptr_st_var_cls_price->st_cls_prc.si_no_recs ; i_cnt++ )
  {
    ptr_st_var_cls_price->st_cls_prc.st_prc[i_cnt].li_scrip_cd = fn_swap_long( ptr_st_var_cls_price->st_cls_prc.st_prc[i_cnt].li_scrip_cd );
    ptr_st_var_cls_price->st_cls_prc.st_prc[i_cnt].li_scrip_cd = ntohl( ptr_st_var_cls_price->st_cls_prc.st_prc[i_cnt].li_scrip_cd );
    ptr_st_var_cls_price->st_cls_prc.st_prc[i_cnt].li_price = fn_swap_long( ptr_st_var_cls_price->st_cls_prc.st_prc[i_cnt].li_price );
    ptr_st_var_cls_price->st_cls_prc.st_prc[i_cnt].li_price = ntohl( ptr_st_var_cls_price->st_cls_prc.st_prc[i_cnt].li_price );
  }
}

/********* 2015 Open Interest Broadcast *****/

void fn_cnvt_to_hfmt_swap_opn_intrst_brd(struct st_opn_intrst_brd *ptr_st_opn_instr_dtls)
{
	int i_count;

	ptr_st_opn_instr_dtls->li_msg_type = fn_swap_long(ptr_st_opn_instr_dtls->li_msg_type);
	ptr_st_opn_instr_dtls->li_msg_type = ntohl(ptr_st_opn_instr_dtls->li_msg_type);

	ptr_st_opn_instr_dtls->si_hrs = fn_swap_short(ptr_st_opn_instr_dtls->si_hrs);
	ptr_st_opn_instr_dtls->si_hrs = ntohs( ptr_st_opn_instr_dtls->si_hrs);

	ptr_st_opn_instr_dtls->si_mins = fn_swap_short(ptr_st_opn_instr_dtls->si_mins);
	ptr_st_opn_instr_dtls->si_mins =ntohs (ptr_st_opn_instr_dtls->si_mins);

	ptr_st_opn_instr_dtls->si_scnds = fn_swap_short(ptr_st_opn_instr_dtls->si_scnds);
	ptr_st_opn_instr_dtls->si_scnds = ntohs(ptr_st_opn_instr_dtls->si_scnds);

	ptr_st_opn_instr_dtls->si_mlscnds = fn_swap_short(ptr_st_opn_instr_dtls->si_mlscnds);
	ptr_st_opn_instr_dtls->si_mlscnds = ntohs(ptr_st_opn_instr_dtls->si_mlscnds);

	ptr_st_opn_instr_dtls->si_no_records = fn_swap_short(ptr_st_opn_instr_dtls->si_no_records);
	ptr_st_opn_instr_dtls->si_no_records = ntohs(ptr_st_opn_instr_dtls->si_no_records);
	
	for(i_count=0;i_count< ptr_st_opn_instr_dtls->si_no_records ; i_count++)
	{
		ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_instr_id = fn_swap_long(ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_instr_id);
	ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_instr_id = ntohl(ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_instr_id);

		ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_qty = fn_swap_long(ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_qty);
		ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_qty = ntohl(ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_qty);

		ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_val = fn_swap_ulnglng(ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_val);
	
/******fn_swap_lnglng************15Decchanged for testing *****************
	ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_val = fn_swap_lnglng(ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_val);
	*******************************/
	ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_val = ntohl(ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_val);

		ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_chng = fn_swap_long(ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_chng);
		ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_chng = ntohl(ptr_st_opn_instr_dtls->st_opn_intrst[i_count].li_opn_intrst_chng);
 
	}
}

/***** 2011 Sensex Broadcast *******/

void fn_cnvt_to_hfmt_swap_sensex_dtls (struct st_sensex_dtl_brd *ptr_st_sensex_dtl_brd)/**2011*/
{
  int i_count;

  ptr_st_sensex_dtl_brd->li_msg_type = fn_swap_long(ptr_st_sensex_dtl_brd->li_msg_type);
  ptr_st_sensex_dtl_brd->li_msg_type = ntohl(ptr_st_sensex_dtl_brd->li_msg_type );

  ptr_st_sensex_dtl_brd->si_hrs = fn_swap_short(ptr_st_sensex_dtl_brd->si_hrs);
  ptr_st_sensex_dtl_brd->si_hrs = ntohs(ptr_st_sensex_dtl_brd->si_hrs);

  ptr_st_sensex_dtl_brd->si_mins = fn_swap_short(ptr_st_sensex_dtl_brd->si_mins);
  ptr_st_sensex_dtl_brd->si_mins = ntohs(ptr_st_sensex_dtl_brd->si_mins);

  ptr_st_sensex_dtl_brd->si_scnds = fn_swap_short(ptr_st_sensex_dtl_brd->si_scnds);
  ptr_st_sensex_dtl_brd->si_scnds = ntohs(ptr_st_sensex_dtl_brd->si_scnds);

  ptr_st_sensex_dtl_brd->si_mlscnds = fn_swap_short(ptr_st_sensex_dtl_brd->si_mlscnds);
  ptr_st_sensex_dtl_brd->si_mlscnds = ntohs(ptr_st_sensex_dtl_brd->si_mlscnds);

  ptr_st_sensex_dtl_brd->si_trdng_sssn = fn_swap_short(ptr_st_sensex_dtl_brd->si_trdng_sssn);
  ptr_st_sensex_dtl_brd->si_trdng_sssn = ntohs(ptr_st_sensex_dtl_brd->si_trdng_sssn);

  ptr_st_sensex_dtl_brd->si_no_recs = fn_swap_short(ptr_st_sensex_dtl_brd->si_no_recs);
  ptr_st_sensex_dtl_brd->si_no_recs = ntohs(ptr_st_sensex_dtl_brd->si_no_recs);

  for(i_count =0 ; i_count<ptr_st_sensex_dtl_brd->si_no_recs ; i_count++)
  {
    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_cd=fn_swap_long(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_cd);
    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_cd= ntohl(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_cd);

    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_high = fn_swap_long(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_high);
    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_high = ntohl(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_high);

    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_low = fn_swap_long(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_low);
    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_low = ntohl(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_low );
    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_opn = fn_swap_long(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_opn);
    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_opn = ntohl(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_opn );

    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_cls = fn_swap_long(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_cls);
    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_cls = ntohl(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_cls );

    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_val = fn_swap_long(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_val);
    ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_val = ntohl(ptr_st_sensex_dtl_brd->st_sub_sen[i_count].li_indx_val );

  }
}

/**** 2012 All Indices Broadcast *********/


void fn_cnvt_to_hfmt_swap_upd_indices_all_brd(struct st_all_indx_dtl_brd *ptr_st_var_all_indx_dtl)
{
	int i_count;
  ptr_st_var_all_indx_dtl->li_msg_type = fn_swap_long(ptr_st_var_all_indx_dtl->li_msg_type);
  ptr_st_var_all_indx_dtl->li_msg_type = ntohl(ptr_st_var_all_indx_dtl->li_msg_type );

  ptr_st_var_all_indx_dtl->si_hrs = fn_swap_short(ptr_st_var_all_indx_dtl->si_hrs);
  ptr_st_var_all_indx_dtl->si_hrs = ntohs(ptr_st_var_all_indx_dtl->si_hrs);

  ptr_st_var_all_indx_dtl->si_mins = fn_swap_short(ptr_st_var_all_indx_dtl->si_mins);
  ptr_st_var_all_indx_dtl->si_mins = ntohs(ptr_st_var_all_indx_dtl->si_mins);

  ptr_st_var_all_indx_dtl->si_scnds = fn_swap_short(ptr_st_var_all_indx_dtl->si_scnds);
  ptr_st_var_all_indx_dtl->si_scnds = ntohs(ptr_st_var_all_indx_dtl->si_scnds);

  ptr_st_var_all_indx_dtl->si_mlscnds = fn_swap_short(ptr_st_var_all_indx_dtl->si_mlscnds);
  ptr_st_var_all_indx_dtl->si_mlscnds = ntohs(ptr_st_var_all_indx_dtl->si_mlscnds);

  ptr_st_var_all_indx_dtl->si_trdng_sssn = fn_swap_short(ptr_st_var_all_indx_dtl->si_trdng_sssn);
  ptr_st_var_all_indx_dtl->si_trdng_sssn = ntohs(ptr_st_var_all_indx_dtl->si_trdng_sssn);

  ptr_st_var_all_indx_dtl->si_no_recs = fn_swap_short(ptr_st_var_all_indx_dtl->si_no_recs);
  ptr_st_var_all_indx_dtl->si_no_recs = ntohs(ptr_st_var_all_indx_dtl->si_no_recs);

	for(i_count=0; i_count< ptr_st_var_all_indx_dtl->si_no_recs ; i_count++)
	{
    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_cd=fn_swap_long(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_cd);
    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_cd= ntohl(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_cd);

    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_high = fn_swap_long(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_high);
    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_high = ntohl(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_high);

    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_low = fn_swap_long(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_low);
    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_low = ntohl(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_low );
    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_opn = fn_swap_long(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_opn);
    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_opn = ntohl(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_opn );

    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_cls = fn_swap_long(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_cls);
    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_cls = ntohl(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_cls );

    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_val = fn_swap_long(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_val);
    ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_val = ntohl(ptr_st_var_all_indx_dtl->st_sub_indices[i_count].li_indx_val );

	}
}

	/**** Trader Limit Exceeded ****/

void fn_cnvt_to_hfmt_swap_trd_limit(struct st_trd_lmt_exceed *ptr_st_lmt_exceed)
{
	ptr_st_lmt_exceed->li_msg_type = fn_swap_long(ptr_st_lmt_exceed->li_msg_type);
	ptr_st_lmt_exceed->li_msg_type = ntohl(ptr_st_lmt_exceed->li_msg_type);
	
	ptr_st_lmt_exceed->li_member_cd = fn_swap_long(ptr_st_lmt_exceed->li_member_cd);
	ptr_st_lmt_exceed->li_member_cd = ntohl(ptr_st_lmt_exceed->li_member_cd);

	ptr_st_lmt_exceed->li_trader_cd = fn_swap_long(ptr_st_lmt_exceed->li_trader_cd);
	ptr_st_lmt_exceed->li_trader_cd = ntohl(ptr_st_lmt_exceed->li_trader_cd);

	ptr_st_lmt_exceed->ll_trd_limit =fn_swap_lnglng(ptr_st_lmt_exceed->ll_trd_limit);

	ptr_st_lmt_exceed->ll_lmt_used = fn_swap_lnglng(ptr_st_lmt_exceed->ll_lmt_used);

}

/********* Position Alert UMS *********/
	
void fn_cnvt_to_hfmt_swap_pos_alrt(struct st_pos_alrt_ums *ptr_st_pos_alrt)
{
	int i_cnt;

	ptr_st_pos_alrt->li_msg_type = fn_swap_long(ptr_st_pos_alrt->li_msg_type);
	ptr_st_pos_alrt->li_msg_type = ntohl(ptr_st_pos_alrt->li_msg_type);
	
	ptr_st_pos_alrt->si_no_records = fn_swap_short(ptr_st_pos_alrt->si_no_records);
	ptr_st_pos_alrt->si_no_records = ntohs(ptr_st_pos_alrt->si_no_records);
	
	for(i_cnt =0 ;i_cnt < ptr_st_pos_alrt->si_no_records ; i_cnt++)
	{
		ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_instr_cd = fn_swap_long(ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_instr_cd);
		ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_instr_cd = ntohl(ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_instr_cd);

		ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_alrt_type = fn_swap_long(ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_alrt_type);
		ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_alrt_type = ntohl(ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_alrt_type);

		ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_percent = fn_swap_long(ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_percent);
		ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_percent = ntohl(ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_percent);

		ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_seg_type = fn_swap_long(ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_seg_type);
		ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_seg_type = ntohl(ptr_st_pos_alrt->st_ums_alrt[i_cnt].li_seg_type);
	
		ptr_st_pos_alrt->st_ums_alrt[i_cnt].si_prdct_typ = fn_swap_short(ptr_st_pos_alrt->st_ums_alrt[i_cnt].si_prdct_typ);
		ptr_st_pos_alrt->st_ums_alrt[i_cnt].si_prdct_typ = ntohs(ptr_st_pos_alrt->st_ums_alrt[i_cnt].si_prdct_typ);

		ptr_st_pos_alrt->st_ums_alrt[i_cnt].si_clnt_type = fn_swap_short(ptr_st_pos_alrt->st_ums_alrt[i_cnt].si_clnt_type);
		ptr_st_pos_alrt->st_ums_alrt[i_cnt].si_clnt_type = ntohs(ptr_st_pos_alrt->st_ums_alrt[i_cnt].si_clnt_type);

	}
}

/***	Ver	1.1	Starts	***/

void fn_cnvt_to_hfmt_swap_mrkt_pctr(struct st_mrkt_pctr_dtls *ptr_market_dtls)
{
	int i_cnt=0;
	int	i_count=0;

	char c_ServiceName[256];

	ptr_market_dtls->li_msg_typ			=	fn_swap_long (ptr_market_dtls->li_msg_typ);
	ptr_market_dtls->li_msg_typ			= ntohl (ptr_market_dtls->li_msg_typ);
	ptr_market_dtls->si_hrs					=	fn_swap_short	(ptr_market_dtls->si_hrs);
	ptr_market_dtls->si_hrs					=	ntohs	(ptr_market_dtls->si_hrs);
	ptr_market_dtls->si_mins				=	fn_swap_short	(ptr_market_dtls->si_mins);
	ptr_market_dtls->si_mins				=	ntohs	(ptr_market_dtls->si_mins);
	ptr_market_dtls->si_scnds				=	fn_swap_short	(ptr_market_dtls->si_scnds);
	ptr_market_dtls->si_scnds				=	ntohs (ptr_market_dtls->si_scnds);
	ptr_market_dtls->si_mlscnds			=	fn_swap_short	(ptr_market_dtls->si_mlscnds);
	ptr_market_dtls->si_mlscnds			=	ntohs (ptr_market_dtls->si_mlscnds);
	ptr_market_dtls->si_no_of_rcrds	=	fn_swap_short	(ptr_market_dtls->si_no_of_rcrds); 
	ptr_market_dtls->si_no_of_rcrds	=	ntohs	(ptr_market_dtls->si_no_of_rcrds);

	for ( i_cnt = 0 ; i_cnt < ptr_market_dtls->si_no_of_rcrds ; i_cnt++ )
	{
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_scrp_cd =	fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_scrp_cd);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_scrp_cd =	ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_scrp_cd);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_opn_rt	=	fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_opn_rt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_opn_rt = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_opn_rt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_prvs_cls_rt = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_prvs_cls_rt);			ptr_market_dtls->st_mrkt_pctr[i_cnt].li_prvs_cls_rt = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_prvs_cls_rt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_high_rt = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_high_rt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_high_rt = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_high_rt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_low_rt = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_low_rt); 		
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_low_rt = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_low_rt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_no_of_trds = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_no_of_trds);				ptr_market_dtls->st_mrkt_pctr[i_cnt].li_no_of_trds = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_no_of_trds);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_volume = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_volume);		
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_volume = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_volume);		
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_value =	fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_value);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_value = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_value);	
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lst_trd_qty = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lst_trd_qty);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lst_trd_qty = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lst_trd_qty);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lst_trd_prc = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lst_trd_prc);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lst_trd_prc = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lst_trd_prc);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_clsng_rt	=	fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_clsng_rt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_clsng_rt	=	ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_clsng_rt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_indctv_eqlbrm_prc = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_indctv_eqlbrm_prc);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_indctv_eqlbrm_prc = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_indctv_eqlbrm_prc);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_indctv_eqlbrm_qty = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_indctv_eqlbrm_qty);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_indctv_eqlbrm_qty = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_indctv_eqlbrm_qty);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].ll_mrkt_pctr_tm	=	fn_swap_lnglng(ptr_market_dtls->st_mrkt_pctr[i_cnt].ll_mrkt_pctr_tm);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_total_bid_qty = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_total_bid_qty);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_total_bid_qty = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_total_bid_qty);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_total_offer_qty = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_total_offer_qty);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_total_offer_qty = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_total_offer_qty);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lwr_crct_lmt	=	fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lwr_crct_lmt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lwr_crct_lmt	=	ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_lwr_crct_lmt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_uppr_crct_lmt = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_uppr_crct_lmt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_uppr_crct_lmt = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_uppr_crct_lmt);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_wghtd_avg_prc = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_wghtd_avg_prc);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].li_wghtd_avg_prc = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].li_wghtd_avg_prc);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].si_mrkt_typ = fn_swap_short(ptr_market_dtls->st_mrkt_pctr[i_cnt].si_mrkt_typ);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].si_mrkt_typ = ntohs(ptr_market_dtls->st_mrkt_pctr[i_cnt].si_mrkt_typ);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].si_sssn_nmbr = fn_swap_short(ptr_market_dtls->st_mrkt_pctr[i_cnt].si_sssn_nmbr);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].si_sssn_nmbr = ntohs(ptr_market_dtls->st_mrkt_pctr[i_cnt].si_sssn_nmbr);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].si_no_of_prc_pnts = fn_swap_short(ptr_market_dtls->st_mrkt_pctr[i_cnt].si_no_of_prc_pnts);
		ptr_market_dtls->st_mrkt_pctr[i_cnt].si_no_of_prc_pnts = ntohs(ptr_market_dtls->st_mrkt_pctr[i_cnt].si_no_of_prc_pnts);

		for( i_count = 0; i_count < ptr_market_dtls->st_mrkt_pctr[i_cnt].si_no_of_prc_pnts;  i_count++ )
		{
			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_bid_rt	= fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_bid_rt);
			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_bid_rt	= ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_bid_rt);

			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_bid_qty	= fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_bid_qty);
			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_bid_qty	= ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_bid_qty);

			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_bid_at_prc_pnt	= fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_bid_at_prc_pnt);
			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_bid_at_prc_pnt	= ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_bid_at_prc_pnt);

			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_offr_rt = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_offr_rt);
			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_offr_rt = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_offr_rt);

			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_offr_qty = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_offr_qty);
			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_offr_qty= ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_best_offr_qty);

			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_ask_at_prc_pnt = fn_swap_long(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_ask_at_prc_pnt);
			ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_ask_at_prc_pnt = ntohl(ptr_market_dtls->st_mrkt_pctr[i_cnt].st_bid_offr_qty[i_count].li_ask_at_prc_pnt); 

		}

	}
	
}
/***	Ver	1.1	Ends		***/
