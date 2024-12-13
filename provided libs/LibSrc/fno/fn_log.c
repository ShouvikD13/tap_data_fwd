/*Ver 1.1 IBM migration  rohit */
/** Ver 1.2 Printing time in milliseconds- Sachin Birje ***/
/** Ver 1.3 Writing FNO userlog into ULOG file using userlog function - Sachin Birje **/
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>
#include <stdlib.h>
#include <fml32.h>
#include <atmi.h>
#include <fo_fml_def.h>
#include <fml_def.h>
#include <fn_log.h>
#include <fo.h>
#include <string.h>	 /* Ver 1.1 */
#include <userlog.h> /* Ver 1.1 */

int i_pip_fd;
int i_pid;
char c_time[12];		  /*** Ver 1.2 **/
void fn_tm_stmp_ml(void); /** Ver 1.2 **/

int fn_userlog(char *c_servicename, char *c_frmt_str, ...)
{
	int i_val;
	char c_writemsg[512];
	time_t ud_tm_stmp;
	/*** char		c_time[12]; *** Ver 1.2 ***/
	char c_msg[256];
	char c_tmp_msg[256];
	/* int     i_tot_len;		*/ /* Ver 1.1 */
	long l_tot_len;				   /* Ver 1.1 */
	/* int     i_cnt; */		   /* Ver 1.1 */
	long l_cnt;					   /* Ver 1.1 */
	/*int     i_trv_ptr; */		   /* Ver 1.1 */
	long l_trv_ptr;				   /* Ver 1.1 */
	va_list c_arg_lst;
	int i_int;
	long int li_long_int;
	long long ll_long_long;
	float f_float;
	double d_double;
	char c_char;
	char *c_str;
	int i_msg_len;
	int i_wrtn_len;
	int i_remn_len;
	char *c_tmp_ptr;
	long l_msg_len;	 /** Ver 1.1 **/
	long l_wrtn_len; /** Ver 1.1 **/
	long l_remn_len; /** Ver 1.1 **/
	long l_val;		 /** Ver 1.1 **/

	/**** Commented in Ver 1.2 *****
	  ud_tm_stmp = time(NULL);
	  strftime( c_time, 12, "%H%M%S", localtime ( &ud_tm_stmp ) );
	******/
	/*** fn_tm_stmp_ml();  *** Ver 1.2 ** Commented in ver 1.3 ***/

	va_start(c_arg_lst, c_frmt_str);
	l_trv_ptr = 0; /* Variable i_trv_ptr changed to l_trv_ptr Ver 1.1 */
	c_msg[0] = '\0';
	l_tot_len = strlen(c_frmt_str); /* i_tot_len variable changed to l_tot_len Ver 1.1 */
	l_cnt = 0;						/* Variable i_cnt changed to l_cnt Ver 1.1 */

	while (l_cnt <= l_tot_len)
	{
		if (c_frmt_str[l_cnt] != '%')
		{
			c_msg[l_trv_ptr] = c_frmt_str[l_cnt];
			c_msg[l_trv_ptr + 1] = '\0';
			l_trv_ptr++;
			l_cnt++;
			continue;
		}

		l_cnt++;

		switch (c_frmt_str[l_cnt])
		{
		case 'd':
			i_int = va_arg(c_arg_lst, int);
			sprintf(c_tmp_msg, "%s%d", c_msg, i_int);
			break;

		case 'f':
			/**f_float =  va_arg ( c_arg_lst,  float );
			sprintf ( c_tmp_msg, "%s%f", c_msg, f_float ); ** commented in Ver 1.1 */
			d_double = va_arg(c_arg_lst, double);		  /** added int Ver 1.1 **/
			sprintf(c_tmp_msg, "%s%lf", c_msg, d_double); /** added int Ver 1.1 **/
			break;

		case 'l':
			l_cnt++;
			switch (c_frmt_str[l_cnt])
			{
			case 'd':
				li_long_int = va_arg(c_arg_lst, long);
				sprintf(c_tmp_msg, "%s%ld", c_msg, li_long_int);
				break;

			case 'f':
				d_double = va_arg(c_arg_lst, double);
				sprintf(c_tmp_msg, "%s%lf", c_msg, d_double);
				break;

			case 'l':
				l_cnt++;
				if (c_frmt_str[l_cnt] == 'd')
				{
					ll_long_long = va_arg(c_arg_lst, long long);
					sprintf(c_tmp_msg, "%s%lld", c_msg, ll_long_long);
				}
				else
				{
					sprintf(c_tmp_msg, "Invalid format string %c",
							c_frmt_str[l_cnt]);
					l_cnt = l_tot_len + 1;
				}
				break;

			default:
				sprintf(c_tmp_msg, "Invalid format string %c",
						c_frmt_str[l_cnt]);
				l_cnt = l_tot_len + 1;
				break;
			}
			break;

		case 'c':
			/**c_char = va_arg ( c_arg_lst,  char );
			sprintf ( c_tmp_msg, "%s%c", c_msg, c_char );	commented int Ver 1.1	**/
			i_int = va_arg(c_arg_lst, int);			  /** added in Ver 1.1 **/
			sprintf(c_tmp_msg, "%s%c", c_msg, i_int); /** added in Ver 1.1 **/
			break;

		case 's':
			c_str = va_arg(c_arg_lst, char *);
			while (*c_str != '\0')
			{
				c_msg[l_trv_ptr] = *c_str;
				c_msg[l_trv_ptr + 1] = '\0';
				l_trv_ptr++;
				c_str++;
			}
			strcpy(c_tmp_msg, c_msg);
			break;

		case 'x':
			/**c_char = va_arg ( c_arg_lst, char );
			sprintf ( c_tmp_msg, "%s%x", c_msg, c_char );		commented int Ver 1.1**/
			i_int = va_arg(c_arg_lst, int);			  /** added in Ver 1.1 **/
			sprintf(c_tmp_msg, "%s%x", c_msg, i_int); /** added in Ver 1.1 **/
			break;

		case '%':
			c_msg[l_trv_ptr] = '%';
			c_msg[l_trv_ptr + 1] = '\0';
			l_trv_ptr++;
			strcpy(c_tmp_msg, c_msg);
			break;

		default:
			sprintf(c_tmp_msg, "Invalid format string %c", c_frmt_str[l_cnt]);
			l_cnt = l_tot_len + 1;
			break;
		}

		strcpy(c_msg, c_tmp_msg);
		l_trv_ptr = strlen(c_msg);
		l_cnt++;
	}

	/**sprintf( c_writemsg, "%s.%s.%d : %s\n", c_time, c_servicename, i_pid, c_msg ); **** ver 1.3 ****/
	sprintf(c_writemsg, "%s:%s\n", c_servicename, c_msg); /*** ver 1.3 ***/

	/** commented in Ver 1.1 IBM ***
  i_msg_len = strlen ( c_writemsg ) + 1;
  i_wrtn_len = 0;
  i_remn_len = i_msg_len;
  c_tmp_ptr = c_writemsg;
  while ( i_wrtn_len < i_msg_len )
  {
	i_val = write ( i_pip_fd, c_tmp_ptr, i_remn_len );
	if ( i_val == -1 )
	{
	  return ( -1 );
	}
	i_wrtn_len = i_wrtn_len + i_val;
	i_remn_len = i_remn_len - i_val;
	c_tmp_ptr = c_tmp_ptr + i_val;
  }
	***** commented upto here ******/

	/***********Commented in Ver 1.3 *********************
	  ** addede in Ver 1.1 **
	  l_msg_len = strlen ( c_writemsg ) + 1;
	l_wrtn_len = 0;
	l_remn_len = l_msg_len;
	c_tmp_ptr = c_writemsg;
	while ( l_wrtn_len < l_msg_len )
	{
	  l_val = write ( i_pip_fd, c_tmp_ptr, l_remn_len );
	  if ( l_val == -1 )
	  {
		return ( -1 );
	  }
	  l_wrtn_len = l_wrtn_len + l_val;
	  l_remn_len = l_remn_len - l_val;
	  c_tmp_ptr = c_tmp_ptr + l_val;
	  }
	*********** Ver 1.3 Ends here **************************/

	userlog("%s", c_writemsg); /*** Ver 1.3 writing FNO userlog in tuxedo userlog file**/

	return 0;
}

void fn_errlog(char *c_servicename,
			   char *c_errno,
			   char *sys_msg,
			   char *c_errmsg)
{
	char c_msg[256];
	char c_qspace[QSPACE_LEN];
	char c_qname[QNAME_LEN];
	FBFR32 *fml_Obuf;
	long len;

	if ((fml_Obuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN)) == (FBFR32 *)NULL)
	{
		fn_userlog(c_servicename, "EBA:Error allocating Buffer :%s:", TPMSG);
		return;
	}

	if (c_errno[0] != 'B')
	{
		fn_userlog(c_servicename, "ERROR - |%s - %s|", c_errno, sys_msg);
		if (strcmp(sys_msg, LIBMSG) != 0)
		{
			strcpy(c_errmsg, "Resource not available");
		}

		/************************************************************
		if ( Fadd32 ( fml_Obuf, FFO_QSPACE_NAME, (char *)c_qspace, 0 ) == -1 )
		{
			sprintf (c_msg, "EBA : Error Adding QSpace Name for SFO_ENQ_MSG :%s:",
																																		FMLMSG);
			fn_userlog( c_servicename, c_msg );
			tpfree( (char *)fml_Obuf );
			return;
		}

		if ( Fadd32 ( fml_Obuf, FFO_QUEUE_NAME, (char *)c_qname, 0 ) == -1 )
		{
			sprintf (c_msg, "EBA : Error Adding Queue Name for SFO_ENQ_MSG :%s:",
																																		FMLMSG);
			fn_userlog( c_servicename, c_msg );
			tpfree( (char *)fml_Obuf );
			return;
		}

		if ( Fadd32 ( fml_Obuf, FFO_ERR_NO, (char *)c_errno, 0 ) == -1 )
		{
			sprintf (c_msg, "EBA : Error Adding Error Number for SFO_ENQ_MSG :%s:",
																																		FMLMSG);
			fn_userlog( c_servicename, c_msg );
			tpfree( (char *)fml_Obuf );
			return;
		}

		if ( Fadd32 ( fml_Obuf, FFO_SYS_MSG, (char *)sys_msg, 0 ) == -1 )
		{
			sprintf (c_msg, "EBA : Error Adding System Message for SFO_ENQ_MSG :%s:",
																																		FMLMSG);
			fn_userlog( c_servicename, c_msg );
			tpfree( (char *)fml_Obuf );
			return;
		}

		if ( Fadd32 ( fml_Obuf, FFO_ERR_MSG, (char *)c_errmsg, 0 ) == -1 )
		{
			sprintf(c_msg, "EBA : Error Adding Error Message for SFO_ENQ_MSG :%s:",
																																		FMLMSG);
			fn_userlog( c_servicename, c_msg );
			tpfree( (char *)fml_Obuf );
			return;
		}

		if ( Fadd32 ( fml_Obuf, FFO_SVC_NAME, (char *)c_servicename, 0 ) == -1 )
		{
			sprintf(c_msg, "EBA : Error Adding Service Name for SFO_ENQ_MSG :%s:",
																																		FMLMSG);
			fn_userlog( c_servicename, c_msg );
			tpfree( (char *)fml_Obuf );
			return;
		}

		if ( tpacall(	"SFO_ENQ_MSG",
									( char * )fml_Obuf,
									0,
									TPNOTRAN|TPNOREPLY ) == -1 )
		{
			sprintf(c_msg,"EBA : Error Calling SFO_ENQ_MSG :%s:", TPMSG);
			fn_userlog( c_servicename, c_msg );
			tpfree( (char *)fml_Obuf );
			return;
		}

		***********************************************/
	}
	else
	{

		if (Fadd32(fml_Obuf, FFO_ERR_NO, (char *)c_errno, 0) == -1)
		{
			fn_userlog(c_servicename,
					   "EBA : Error Adding Error Number for SFO_ERRLOG :%s:",
					   FMLMSG);
			tpfree((char *)fml_Obuf);
			strcpy(c_errmsg, "Resource not available");
			return;
		}

		if (tpcall("SFO_ERRLOG",
				   (char *)fml_Obuf,
				   0,
				   (char **)&fml_Obuf,
				   &len,
				   TPNOTRAN) == -1)
		{
			fn_userlog(c_servicename,
					   "EBA : Error Calling SFO_ERRLOG :%s:",
					   TPMSG);
			tpfree((char *)fml_Obuf);
			strcpy(c_errmsg, "Resource not available");
			return;
		}

		if (Fget32(fml_Obuf, FFO_ERR_MSG, 0, (char *)c_msg, 0) == -1)
		{
			fn_userlog(c_servicename,
					   "EBA : Error calling SFO_ERRLOG Service :%s:",
					   FMLMSG);
			tpfree((char *)fml_Obuf);
			strcpy(c_errmsg, "Resource not available");
			return;
		}
		fn_userlog(c_servicename, c_msg);
		strcpy(c_errmsg, c_msg);
	}

	tpfree((char *)fml_Obuf);
	return;
}

int fn_create_pipe()
{
	char c_Logpipe[256];
	char c_msg[256];

	sprintf(c_Logpipe, "%s/LOGPIP", getenv("BIN"));
	i_pip_fd = open(c_Logpipe, O_WRONLY | O_NDELAY);
	if (i_pip_fd == -1)
	{
		sprintf(c_msg, "EBA : Error creating pipe :%s:", UNXMSG);
		userlog(c_msg);
		return (-1);
	}
	i_pid = getpid();
	return (0);
}

void fn_destroy_pipe()
{
	close(i_pip_fd);
	return;
}

// void rtrim(char* return_string)
//{
//  /*int i; */	 /* Ver 1.1 */
//	long i;		/* Ver 1.1 */
// int j = 0;
///* int str_length; */		/* Ver 1.1 */
//	long str_length;		/* Ver 1.1 */
//  str_length = strlen(return_string);
//
//  for(i=(str_length-1);return_string[i]==' '; i--)
//  {
//    if ( i < 0 )
//    {
//       break;
//    }
//    j++;
//  }
//  return_string[i+1]='\0';
//  return;
//}

// void errlog ( char *c_servicename, char *c_errno, char *sys_msg, char *c_usrid,
//               long lisssn, char *ptr_c_errmsg)
//{

/*****************************************************************************/
/*  Fetch all values from the error master table Throw the appropriate error */
/*  message and if it is a system error, log int the uel file                */
/*****************************************************************************/

// char  errbuf[256];
//  long  len;
//  FBFR32  *fml_Obuf;

//  fml_Obuf = (FBFR32 *)NULL;

// strcpy(ptr_c_errmsg, "Resource unavailable.");
//
// if((fml_Obuf = (FBFR32 *)tpalloc("FML32",NULL,MIN_FML_BUF_LEN)) ==
//                                                    (FBFR32 *)NULL )
// {
//   userlog("Error allocating buffer for logging error");
//  return;
// }

// if( Fadd32( fml_Obuf,FML_USR_ID, c_usrid, 0 ) == -1)
// {
//   userlog("Error Adding User id to buffer for calling error message");
//   tpfree((char *)fml_Obuf);
//   return;
// }

// if( Fadd32( fml_Obuf, FML_SSSN_ID, (char *)&lisssn, 0 ) == -1)
// {
//   userlog("Error Adding Session id to buffer for calling error message");
//   tpfree((char *)fml_Obuf);
//   return;
// }

// if( Fadd32( fml_Obuf, FML_ERR_NO, c_errno, 0 ) == -1)
//{
//   userlog("Error Adding Error number to buffer for calling error message");
//   tpfree((char *)fml_Obuf);
//   return;
// }
//
// if( Fadd32( fml_Obuf, FML_SYS_MSG, sys_msg, 0 ) == -1)
// {
//  userlog("Error Adding System message to buffer for calling error message");
// tpfree((char *)fml_Obuf);
// return;
//  }
//
// if(Fadd32(fml_Obuf,FML_SVC_NAME,c_servicename,0)== -1)
//{
// userlog("Error Adding Service name to buffer for calling error message");
// tpfree((char *)fml_Obuf);
//    return;
// }
//
// if(tpcall("SVC_ERRLOG",(char *)fml_Obuf,0,(char **)&fml_Obuf,
//                                          &len,TPNOTRAN) == -1)
//  {
//   userlog("Error Calling System error service :%s:",TPMSG);
//  tpfree((char *)fml_Obuf);
// return;
//  }
//
//  if( Fget32(fml_Obuf, FML_ERR_MSG, 0, ptr_c_errmsg, 0) == -1)
//  {
//    userlog("Error Calling Errmsg Service - eba2");
//    tpfree((char *)fml_Obuf);
//    return;
//  }
//
//  tpfree((char *)fml_Obuf);
//  return;
//
//}

void fn_log_tm(char *c_servicename,
			   char *c_tmr_no)
{
	struct timeb log_time;
	char c_tm_stmp[30];
	char c_tm_stmp_ml[50];

	ftime(&log_time);
	strftime(c_tm_stmp, 30, "%H:%M:%S", (localtime(&log_time.time)));
	c_tm_stmp[9] = '\0';
	sprintf(c_tm_stmp_ml, "%s:%d", c_tm_stmp, log_time.millitm);
	fn_userlog(c_servicename, "%s - %s", c_tmr_no, c_tm_stmp_ml);
}

void fn_tm_stmp_ml()
{
	struct timeb log_time;
	char c_tm_stmp[30];

	memset(c_time, '\0', sizeof(c_time));

	ftime(&log_time);
	strftime(c_tm_stmp, 30, "%H%M%S", (localtime(&log_time.time)));
	c_tm_stmp[9] = '\0';
	sprintf(c_time, "%s.%d", c_tm_stmp, log_time.millitm);
}
