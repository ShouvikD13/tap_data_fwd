function in thiss file primarily deals with the data manipulation for the network communication , including the byte swaping and string manipulation.

this is because in network communication data required in a specific byte order (big-endian).so all the function (byte - swaping) defined in 'nse_xchng_msg.cpp' file are handling the conversion.

1. fn_swap_short(short s)

Swaps the bytes of a short integer.
Example: 0x1234 becomes 0x3412.

2. fn_swap_ushort(unsigned short s)

Swaps the bytes of an unsigned short integer.
Similar to fn_swap_short but for unsigned short integers.

3. fn_swap_long(long l)

Swaps the bytes of a long integer.
Example: 0x12345678 becomes 0x78563412.

4. fn_swap_lnglng(long long l)

Swaps the bytes of a long long integer.
It performs two swaps: one for each 32-bit part of the 64-bit integer.

5. fn_swap_ulong(unsigned long l)

Swaps the bytes of an unsigned long integer.
Similar to fn_swap_long but for unsigned long integers.

6. fn_swap_ulnglng(long long l)

Swaps the bytes of an unsigned long long integer.
Similar to fn_swap_lnglng but for unsigned long long integers.
## (check out the nse_xchng_msg.cpp, i already provide the full explinition for above function)

7. fn_string_to_char(char *ptr_c_dest, long int li_len_dest, char *ptr_c_src, long int li_len_src)

Copies a source string (ptr_c_src) to a destination string (ptr_c_dest), removing trailing spaces and ensuring the destination string is null-terminated.
Ensures the destination string length does not exceed li_len_dest.

## (check out the nse_xchng_msg.cpp, i already provide the full explinition for above function)

8. fn_char_to_string(char *ptr_c_dest, long int li_len_dest, char *ptr_c_src, long int li_len_src)
Converts a source string (ptr_c_src) to uppercase and copies it to a destination string (ptr_c_dest).
Pads the destination string with spaces if it is longer than the source string.
