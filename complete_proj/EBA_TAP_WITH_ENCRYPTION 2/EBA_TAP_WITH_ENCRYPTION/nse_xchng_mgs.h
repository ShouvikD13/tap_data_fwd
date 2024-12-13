/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   nse_xchng_mgs.h
 * Author: C706822
 *
 * Created on February 26, 2019, 6:10 PM
 */

#ifndef NSE_XCHNG_MGS_H
#define NSE_XCHNG_MGS_H

namespace nse_xchng_msg_namespase {
    /*--------------------------------------------------------------------------------------------*/
    /*  Function to swap the given value of short datatype                                        */

    /*--------------------------------------------------------------------------------------------*/
    short fn_swap_short(short s);
    /*--------------------------------------------------------------------------------------------*/
    /*  Ver. 1.1 : Function to swap the given value of unsigned short datatype                    */

    /*--------------------------------------------------------------------------------------------*/
    unsigned short fn_swap_ushort(unsigned short s);

    /*--------------------------------------------------------------------------------------------*/
    /*  Function to swap the given value of long datatype                                         */

    /*--------------------------------------------------------------------------------------------*/
    long fn_swap_long(long l);

    /*--------------------------------------------------------------------------------------------*/
    /*  Function to swap the given value of long long datatype                                    */

    /*--------------------------------------------------------------------------------------------*/
    long long fn_swap_lnglng(long long l);

    /*--------------------------------------------------------------------------------------------*/
    /*  Function to swap the given value of long datatype into unsigned long datatype             */
    /*--------------------------------------------------------------------------------------------*/

    /** Ver. 1.1 unsigned long fn_swap_ulong(long l) commened. **/
    unsigned long fn_swap_ulong(unsigned long l);

    /*--------------------------------------------------------------------------------------------*/
    /*  Function to swap the given value of long long datatype  into unsigned long long datatype  */

    /*--------------------------------------------------------------------------------------------*/
    unsigned long long fn_swap_ulnglng(long long l);

    /*--------------------------------------------------------------------------------------------*/
    /*  Function to copy given string of given length into another string of given length         */

    /*--------------------------------------------------------------------------------------------*/
    void fn_string_to_char(char *ptr_c_dest,
            long int li_len_dest,
            char *ptr_c_src,
            long int li_len_src);

    /*--------------------------------------------------------------------------------------------*/

    /*--------------------------------------------------------------------------------------------*/
    void fn_char_to_string(char *ptr_c_dest,
            long int li_len_dest,
            char *ptr_c_src,
            long int li_len_src);
}


#endif /* NSE_XCHNG_MGS_H */

