/* Log  1.1 IBM migration ranjit */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>		/* ver 1.1 */

void main ( )
{
	char c_msg [ 512 * 10 ];
	char c_disp_msg [ 512 ];
	int i_disp_len;
	int i_pip_fd;
	int i_val;	
	int i_cnt;
	int i_flag;
	char c_Logpipe[256];
	char c_Logfile[256];
	char c_Logbkfile[256];
	char c_cmd[512];
	FILE *f_ptr_logfil;
	long int li_ln_cnt;
	long l_cnt = 0;			/** ver 1.1 **/
	long int l_val = 0; /** ver 1.1 **/
  time_t  ud_tm_stmp;
  char    c_time[12];
  char    c_date[7];
  char    c_curr_date[7];

  ud_tm_stmp = time(NULL);
	/* To append date (MMDDYY format) to the FNOLOG */
 	strftime( c_date, 7, "%m%d%y", localtime ( &ud_tm_stmp ) );
	sprintf( c_Logfile, "%s/FNOLOG.%s", getenv("LOG"), c_date );

	f_ptr_logfil = fopen ( c_Logfile, "a+" );
	if ( f_ptr_logfil == NULL )
	{
		printf ( "Error opening file : %s\n", strerror ( errno ) );
		exit ( 0 );
	} 

	sprintf( c_Logpipe, "%s/LOGPIP", getenv("BIN") );

	i_val = mknod ( c_Logpipe, S_IFIFO|S_IRWXU, 0 );
	if ( i_val == -1 && errno != EEXIST )
	{
		sprintf ( c_msg, "Error creating Special File : %s\n", strerror ( errno ) );
		fprintf ( f_ptr_logfil, "%s\n", c_msg );
		fclose ( f_ptr_logfil );
		exit ( 0 );
	}

	li_ln_cnt = 0;
	i_pip_fd = open ( c_Logpipe, O_RDONLY|O_CREAT );
	if ( i_pip_fd == -1 )
	{
		sprintf ( c_msg, "Error opening pipe : %s\n", strerror ( errno ) );
		fprintf ( f_ptr_logfil, "%s\n", c_msg );
		fclose ( f_ptr_logfil );
		exit ( 0 );
	}
	
	i_disp_len = 0;
	while( 1 )
	{

		/*** commented in IBM ********
		i_val = read ( i_pip_fd, c_msg, 512*10 );	
		if ( i_val < 0 )
		{
			sprintf ( c_msg, "Error : %s\n", strerror ( errno ) );
			fprintf ( f_ptr_logfil, "%s\n", c_msg );
			fclose ( f_ptr_logfil );
			break;
		}
		else if ( i_val > 0 )
		{
			for ( i_cnt = 0; i_cnt < i_val; i_cnt++ )
			{
				if ( c_msg [ i_cnt ] != '\0' )
				{
					c_disp_msg [ i_disp_len ] = c_msg [ i_cnt ];
					i_disp_len++;
				}
				else
				{
					c_disp_msg [ i_disp_len ] = c_msg [ i_cnt ];
		******commented upto here in ver 1.1 **/
		/*added in ver 1.1 ***/	
		l_val = read ( i_pip_fd, c_msg, 512*10 );
		if (l_val < 0 )
		{
			sprintf ( c_msg, "Error : %s\n", strerror ( errno ) );
      fprintf ( f_ptr_logfil, "%s\n", c_msg );
      fclose ( f_ptr_logfil );
      break;
		}
		else if ( l_val > 0	)
		{
			for ( l_cnt = 0; l_cnt < l_val ; l_cnt++ )
			{
				if ( c_msg [ l_cnt ] != '\0' )
				{
					c_disp_msg [ i_disp_len ] = c_msg [ l_cnt ];
					i_disp_len++;
				}
				else
				{
					c_disp_msg [ i_disp_len ] = c_msg [l_cnt];		/** added upto here in ver 1.1 **/
					fprintf ( f_ptr_logfil, "%s", c_disp_msg );
					fflush ( f_ptr_logfil );
					i_disp_len = 0;
					li_ln_cnt++;

  				ud_tm_stmp = time(NULL);
					/* If the Date is changed we have to create a new FNOLOG */
  				strftime( c_curr_date, 7, "%m%d%y", localtime ( &ud_tm_stmp ) );
					if ( strcmp( c_curr_date, c_date ) == 0 )
					{
						i_flag = 0;
					}
					else
					{
						i_flag = 1;
					}

					if ( li_ln_cnt >= 100000 || i_flag )
					{
						fclose ( f_ptr_logfil );

						/* If the date got changed there is no need to add   */
						/* time stamp                                        */
						if ( !i_flag )
						{
  						ud_tm_stmp = time(NULL);
  						strftime( c_time, 12, "%H%M%S", localtime ( &ud_tm_stmp ) );

							/* We are now adding End Time and Date to the FNOLOG */
							sprintf( c_Logbkfile, "%s/FNOLOG_%s.%s", getenv("LOG"), 
												c_time, c_curr_date );
							sprintf ( c_cmd, "mv %s %s", c_Logfile, c_Logbkfile );
							system ( c_cmd );
						}

  					ud_tm_stmp = time(NULL);
						/* To append date (MMDDYY format) to the FNOLOG */
					 	strftime( c_date, 7, "%m%d%y", localtime ( &ud_tm_stmp ) );
						sprintf( c_Logfile, "%s/FNOLOG.%s", getenv("LOG"), c_date );

						f_ptr_logfil = fopen ( c_Logfile, "a+" );
						if ( f_ptr_logfil == NULL )
						{
							printf ( "Error opening file : %s\n", strerror ( errno ) );
							break;
						} 
						li_ln_cnt=0;
					}
				}
			}
		}
		else
		{
			sleep(1);
		}
	}

	fclose ( f_ptr_logfil );
	close ( i_pip_fd );
}
