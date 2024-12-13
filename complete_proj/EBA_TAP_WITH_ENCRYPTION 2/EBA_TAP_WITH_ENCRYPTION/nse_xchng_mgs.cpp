#include <iostream>
#include <cstring>
#include "nse_xchng_mgs.h"


/*--------------------------------------------------------------------------------------------*/
/*  Function to swap the given value of short datatype                                        */
/*--------------------------------------------------------------------------------------------*/
short nse_xchng_msg_namespase::fn_swap_short(short s) {
    return (short) (((s & 0xFF00) >> 8) |
            ((s & 0x00FF) << 8));
}
/*--------------------------------------------------------------------------------------------*/
/*  Ver. 1.1 : Function to swap the given value of unsigned short datatype                    */
/*--------------------------------------------------------------------------------------------*/
unsigned short nse_xchng_msg_namespase::fn_swap_ushort(unsigned short s) {
    return (unsigned short) (((s & 0xFF00) >> 8) |
            ((s & 0x00FF) << 8));
}

/*--------------------------------------------------------------------------------------------*/
/*  Function to swap the given value of long datatype                                         */
/*--------------------------------------------------------------------------------------------*/
long nse_xchng_msg_namespase::fn_swap_long(long l) {
    return (long) (((l & 0xFF000000) >> 24) |
            ((l & 0x00FF0000) >> 8) |
            ((l & 0x0000FF00) << 8) |
            ((l & 0x000000FF) << 24));
}

/*--------------------------------------------------------------------------------------------*/
/*  Function to swap the given value of long long datatype                                    */
/*--------------------------------------------------------------------------------------------*/
long long nse_xchng_msg_namespase::fn_swap_lnglng(long long l) {
    return (((long long) (fn_swap_long((long) ((l << 32) >> 32))) << 32) |
            (long) fn_swap_long(((long) (l >> 32))));
}
/*
    Let's consider a 64-bit integer: 0x123456789ABCDEF0
    Lower 32 bits: 0x9ABCDEF0
    Upper 32 bits: 0x12345678
    Swapped lower 32 bits: 0xF0DEBC9A
    Swapped upper 32 bits: 0x78563412
    Combined result: 0xF0DEBC9A78563412
*/

/*--------------------------------------------------------------------------------------------*/
/*  Function to swap the given value of long datatype into unsigned long datatype             */
/*--------------------------------------------------------------------------------------------*/

/** Ver. 1.1 unsigned long fn_swap_ulong(long l) commened. **/
unsigned long nse_xchng_msg_namespase::fn_swap_ulong(unsigned long l) {
    return (unsigned long) (((l & 0xFF000000) >> 24) |
            ((l & 0x00FF0000) >> 8) |
            ((l & 0x0000FF00) << 8) |
            ((l & 0x000000FF) << 24));
}

/*--------------------------------------------------------------------------------------------*/
/*  Function to swap the given value of long long datatype  into unsigned long long datatype  */
/*--------------------------------------------------------------------------------------------*/
unsigned long long nse_xchng_msg_namespase::fn_swap_ulnglng(long long l) {
    return (((unsigned long long) (fn_swap_ulong((unsigned long) ((l << 32) >> 32))) << 32) |
            (unsigned long) fn_swap_ulong(((unsigned long) (l >> 32))));
}

/*--------------------------------------------------------------------------------------------*/
/*  Function to copy given string of given length into another string of given length         */
/*--------------------------------------------------------------------------------------------*/
void nse_xchng_msg_namespase::fn_string_to_char(char *ptr_c_dest,
        long int li_len_dest, char *ptr_c_src, long int li_len_src) {
             long int li_cnt;
             int i_tmp;

    for (i_tmp = li_len_src - 1; (*(ptr_c_src + i_tmp) == ' ') && (i_tmp >= 0); i_tmp--);

    i_tmp++;

    for (li_cnt = 0; ((li_cnt < i_tmp) && (li_cnt < li_len_dest)); li_cnt++) {
        *(ptr_c_dest + li_cnt) = *(ptr_c_src + li_cnt);
    }

    *(ptr_c_dest + li_cnt) = '\0';
}
/*
    explanition of above function:-

    ptr_c_dest: Pointer to the destination character array.
    li_len_dest: Length of the destination character array.
    ptr_c_src: Pointer to the source character array.
    li_len_src: Length of the source character array.

    long int li_cnt; --> Used as a loop counter for copying characters.
    int i_tmp; --> Used to find the last non-space character in the source string.

    For example, if ptr_c_src contains "hello " (with three trailing spaces), i_tmp will be set to 5th index.

    Copy Characters from Source to Destination:
    -------------------------------------------

    for (li_cnt = 0; ((li_cnt < i_tmp) && (li_cnt < li_len_dest)); li_cnt++) {
    *(ptr_c_dest + li_cnt) = *(ptr_c_src + li_cnt);
    }

    here :- this loop starts from 0 till one of 2 conditions meet, 1. li_cnt should be lesser than i_tmp and li_len_dest both.

    *(ptr_c_dest + li_cnt) = '\0';

    After the loop, the destination string is null-terminated to ensure it is a proper C-string.


*/
/*--------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------*/
void nse_xchng_msg_namespase::fn_char_to_string(char *ptr_c_dest,
        long int li_len_dest,
        char *ptr_c_src,
        long int li_len_src) {
    long int li_cnt;

    for (li_cnt = 0; li_cnt < li_len_src; li_cnt++) {
        *(ptr_c_dest + li_cnt) = std::toupper(*(ptr_c_src + li_cnt));
    }

    if ((li_len_dest - li_len_src) > 0) {
        memset((ptr_c_dest + li_len_src), ' ', (li_len_dest - li_len_src));
    }
}
/*
    Converts a source string (ptr_c_src) to uppercase and copies it to a destination string (ptr_c_dest).
    Pads the destination string with spaces if it is longer than the source string.
*/

