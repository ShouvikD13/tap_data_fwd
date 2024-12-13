#include <stdio.h>
#include <userlog.h>
#include <atmi.h>
#include <time.h>
#include <fo.h>
#include <fn_log.h>

int fn_read_debug_flg(char * cptr_prcs_name);	/* Debug Level */

int tpsvrinit ( int argc, char * argv[] )
{
	char *ptr_c_tmp;
	int i_ch_val;

	i_ch_val = fn_create_pipe ( );
	if ( i_ch_val == -1 )
	{
		userlog ( "EBA : Error creating pipe" );
		return ( -1 );
	}

	if ( tpopen( ) == -1 )
	{
		userlog ( "EBA : RM failed in opening connection |%s|", TPMSG );
		return ( -1 );
	}

	fn_read_debug_flg(argv[0]);	/* Debug Level */

	return ( 0 );
}

void tpsvrdone ( )
{
	if ( tpclose ( ) == -1 )
	{
		userlog ( "EBA : RM failed in closing connection |%s|", TPMSG );
	}

	fn_destroy_pipe ( );
}
