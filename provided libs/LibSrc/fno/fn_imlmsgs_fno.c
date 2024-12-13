
/******************************************************************/
/*				File 				: fn_imlmsgs_fno.c													*/
/*				Description : Swap Functions to read Data from IML 			*/
/*																																*/
/******************************************************************/

/*------------------------------------------------------*/
/*  Function to swap the given value of short datatype  */
/*------------------------------------------------------*/
short fn_swap_short(short s)
{
    return ( short)(((s & 0xFF00) >> 8) |
                            ((s & 0x00FF) << 8));
}
/*-------------------------------------------------------------*/
/*  Function to swap the given value of unsigned short datatype*/
/*-------------------------------------------------------------*/
unsigned short fn_swap_ushort(unsigned short s)
{
    return ( unsigned short)(((s & 0xFF00) >> 8) |
                            ((s & 0x00FF) << 8));
}

/*-------------------------------------------------------------*/
/*  Function to swap the given value of long datatype          */
/*-------------------------------------------------------------*/
long fn_swap_long(long l)
{
    return (long)(((l & 0xFF000000) >> 24) |
                           ((l & 0x00FF0000) >>  8) |
                           ((l & 0x0000FF00) <<  8) |
                           ((l & 0x000000FF) << 24));
}

/*-------------------------------------------------------------*/
/*  Function to swap the given value of long long datatype     */
/*-------------------------------------------------------------*/
long long fn_swap_lnglng(long long l)
{
    return (((long long) (fn_swap_long((long)((l << 32) >> 32))) << 32) |
                  (long)  fn_swap_long(((long)(l >> 32))));
}

/*--------------------------------------------------------------------------------------------*/
/*  Function to swap the given value of long datatype into unsigned long datatype             */
/*--------------------------------------------------------------------------------------------*/
unsigned long fn_swap_ulong(unsigned long l)
{
    return (unsigned long)(((l & 0xFF000000) >> 24) |
                           ((l & 0x00FF0000) >>  8) |
                           ((l & 0x0000FF00) <<  8) |
                           ((l & 0x000000FF) << 24));
}

/*--------------------------------------------------------------------------------------------*/
/*  Function to swap the given value of long long datatype  into unsigned long long datatype  */
/*--------------------------------------------------------------------------------------------*/
unsigned long long fn_swap_ulnglng(long long l)
{
    return (( (unsigned long long) (fn_swap_ulong((unsigned long)((l << 32) >> 32))) << 32) |
                   (unsigned long)  fn_swap_ulong(((unsigned long)(l >> 32))));
}



