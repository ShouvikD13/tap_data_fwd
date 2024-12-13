/* Ver 1.1 AIX Migration 13/12/2007	ROhit */
/* Ver 1.1			Debug Level */
#include <stdio.h>
#include <userlog.h>
#include <atmi.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <fo.h>
#include <fn_log.h>
#include <fn_env.h>		/* Ver 1.1 */

int fn_read_debug_flg(char * cptr_prcs_name); /* Debug Level */

int tpsvrinit ( int argc, char * argv[] )
{
	char *ptr_c_tmp;
	char *c_path;
	int i_ch_val;
	char c_FileName[256];

	i_ch_val = fn_create_pipe ( );
	if ( i_ch_val == -1 )
	{
		userlog ( "EBA : Error creating pipe" );
		return ( -1 );
	}

	if ( tpopen( ) == -1 )
	{
		userlog ( "EBA : RM failed in opening connection - |%s|", TPMSG );
		return ( -1 );
	}

	c_path = tuxgetenv("BIN");
	if ( c_path == NULL )
	{
		userlog ( "EBA : Error getting Environment Variable - |%s|", TPMSG );
		return ( -1 );
	}

	sprintf( c_FileName, "%s/env.dat", c_path );

	i_ch_val=fn_init_prcs_spc( "fo_ftov_srvr", c_FileName, "SFO_FTOV" );
	if ( i_ch_val == -1 )
	{
		userlog( "EBA : Error in Reading the Config File" );
		return ( -1 );
	}	

	 fn_read_debug_flg(argv[0]); /* Debug Level */

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
