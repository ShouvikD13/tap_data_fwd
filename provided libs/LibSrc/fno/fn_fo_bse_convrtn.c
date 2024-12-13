#include<fo_bse_exch_comnN.h>
#include <stdlib.h>
#include <string.h>
#include <userlog.h>
#include <ctype.h>
#include <atmi.h>
#include <errno.h>

char    c_month_str[12+1][10] = { "Nil", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

long fn_swap_long(long l)
{
    return (long)(((l & 0xFF000000) >> 24) |
                           ((l & 0x00FF0000) >>  8) |
                           ((l & 0x0000FF00) <<  8) |
                           ((l & 0x000000FF) << 24));
}


short fn_swap_short(short s)
{
    return ( short)(((s & 0xFF00) >> 8) |
                            ((s & 0x00FF) << 8));
 }

unsigned long fn_swap_ulong(unsigned long l)
{
    return (unsigned long)(((l & 0xFF000000) >> 24) |
                           ((l & 0x00FF0000) >>  8) |
                           ((l & 0x0000FF00) <<  8) |
                           ((l & 0x000000FF) << 24));
}


unsigned long long fn_swap_ulnglng(long long l)
{
    return (( (unsigned long long) (fn_swap_ulong((unsigned long)((l << 32) >> 32))) << 32) |
                   (unsigned long)  fn_swap_ulong(((unsigned long)(l >> 32))));
}



/*--------------------------------------------------------------------------------------------*/
/*  Function to convert time present in char variable in a structure in the sequence          */
/*  DMYHMS (Day Month Year Hour Min Sec) into a string in the format 'DD-Mon-YYYY HH24:MI:SS' */
/*--------------------------------------------------------------------------------------------*/
void fn_tmstmp_dmyhms_to_arr ( struct st_tmstmp_dmyhms st_var_tstmp_dmyhms, char *c_time_arr)
{
  sprintf(c_time_arr, "%02d-%s-%4d %02d:%02d:%02d", st_var_tstmp_dmyhms.c_day,
                                                    c_month_str[st_var_tstmp_dmyhms.c_month],
                                                    st_var_tstmp_dmyhms.c_year + ADD_YEAR,
                                                    st_var_tstmp_dmyhms.c_hour,
                                                    st_var_tstmp_dmyhms.c_min,
                                                    st_var_tstmp_dmyhms.c_sec );


}


/*--------------------------------------------------------------------------------------------*/
/*  Function to convert time present in char variable in a structure in the sequence          */
/*  DMY (Day Month Year)  into a string in the format 'DD-Mon-YYYY'                           */
/*--------------------------------------------------------------------------------------------*/
void fn_tmstmp_dmy_to_arr ( struct st_tmstmp_dmy st_var_tstmp_dmy, char *c_time_arr)
{
  sprintf(c_time_arr, "%02d-%s-%4d", st_var_tstmp_dmy.c_day,
                                                    c_month_str[st_var_tstmp_dmy.c_month],
                                                    st_var_tstmp_dmy.c_year + ADD_YEAR
                                                    );


}

void fn_tmstmp_ymdhms_to_arr ( struct st_bfo_timestamp_ymdhms  st_var_tstmp_ymdhms, char *c_time_arr)
{
  sprintf(c_time_arr, "%02d-%s-%4d %02d:%02d:%02d", st_var_tstmp_ymdhms.c_day,
                                                    c_month_str[st_var_tstmp_ymdhms.c_month],
                                                    st_var_tstmp_ymdhms.c_year + ADD_YEAR,
                                                    st_var_tstmp_ymdhms.c_hour,
                                                    st_var_tstmp_ymdhms.c_min,
                                                    st_var_tstmp_ymdhms.c_sec );


}

/*--------------------------------------------------------------------------------------------*/
/*  Function to copy given string of given length into another string of given length         */
/*--------------------------------------------------------------------------------------------*/
void fn_string_to_char(char     *ptr_c_dest,
                       long int li_len_dest,
                       char     *ptr_c_src,
                       long int li_len_src)
{
  long int li_cnt;
  int i_tmp;

  for(i_tmp = li_len_src - 1; (*(ptr_c_src + i_tmp) == ' ') && (i_tmp >= 0); i_tmp--);

  i_tmp++;

  for(li_cnt = 0; ((li_cnt < i_tmp) && (li_cnt < li_len_dest)); li_cnt++)
  {
    *(ptr_c_dest + li_cnt) =  *(ptr_c_src + li_cnt);
  }

  *(ptr_c_dest + li_cnt ) = '\0';
}

/*--------------------------------------------------------------------------------------------*/