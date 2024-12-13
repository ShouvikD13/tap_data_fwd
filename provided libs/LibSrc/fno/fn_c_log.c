/* ver 1.1 19-Sep-2007 IBM migration rohit **/
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>
#include <stdlib.h>
#include <fn_log.h>
#include <fo.h>
#include <string.h> /* Ver 1.1 */

int i_pip_fd;
int i_pid;

int fn_userlog(char *c_servicename, char *c_frmt_str, ...)
{
	int i_val;
	char c_writemsg[512];
	time_t ud_tm_stmp;
	char c_time[12];
	char c_msg[256];
	char c_tmp_msg[256];
	/*int     i_tot_len; */	 /* Ver 1.1 */
	long l_tot_len;			 /* Ver 1.1 */
	/*int     i_cnt; */		 /* Ver 1.1 */
	long l_cnt;				 /* Ver 1.1 */
	/* int     i_trv_ptr; */ /* Ver 1.1 */
	long l_trv_ptr;			 /* Ver 1.1 */
	va_list c_arg_lst;
	int i_int;
	long int li_long_int;
	float f_float;
	double d_double;
	char c_char;
	char *c_str;
	int i_msg_len;
	int i_wrtn_len;
	int i_remn_len;
	char *c_tmp_ptr;

	long l_msg_len;
	long l_wrtn_len;
	long l_remn_len;
	long l_val;

	ud_tm_stmp = time(NULL);
	strftime(c_time, 12, "%H%M%S", localtime(&ud_tm_stmp));

	va_start(c_arg_lst, c_frmt_str);
	l_trv_ptr = 0; /* All i_trv_ptr changed to l_trv_ptr  Ver 1.1 */
	c_msg[0] = '\0';
	l_tot_len = strlen(c_frmt_str); /* i_tot_len changed to l_tot_len  Ver 1.1 */
	/*i_cnt = 0; */					/* Ver 1.1 */
	l_cnt = 0;						/* Ver 1.1 */

	while (l_cnt <= l_tot_len) /* i_tot_len changed to l_tot_len  Ver 1.1 */
	{
		if (c_frmt_str[l_cnt] != '%') /* all i_cnt changed to l_cnt  Ver 1.1 */
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
			/**f_float = va_arg ( c_arg_lst, float );
			sprintf ( c_tmp_msg, "%s%f", c_msg, f_float ); ****commented int Ver 1.1 **/
			d_double = va_arg(c_arg_lst, double);		  /** added in Ver 1,1 **/
			sprintf(c_tmp_msg, "%s%lf", c_msg, d_double); /** added in Ver 1,1 **/
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
			}
			break;

		case 'c':
			/**c_char = va_arg ( c_arg_lst, char );
			sprintf ( c_tmp_msg, "%s%c", c_msg, c_char );	** commented int Ver 1.1 **/
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
			sprintf ( c_tmp_msg, "%s%x", c_msg, c_char );	** commented int Ver 1.1 **/
			i_int = va_arg(c_arg_lst, int);			  /** added in Ver 1.1 **/
			sprintf(c_tmp_msg, "%s%c", c_msg, i_int); /** added in Ver 1.1 **/
			break;

		case '%':
			c_msg[l_trv_ptr] = '%';
			c_msg[l_trv_ptr + 1] = '\0';
			l_trv_ptr++;
			strcpy(c_tmp_msg, c_msg);
			break;

		default:
			sprintf(c_tmp_msg, "Invalid format string %c", c_frmt_str[l_cnt]);
			l_cnt = l_tot_len + 1; /* i_tot_len changed to l_tot_len  Ver 1.1 */
			break;
		}

		strcpy(c_msg, c_tmp_msg);
		l_trv_ptr = strlen(c_msg); /* Ver 1.1 Impact variable i_trv_ptr changed to l_trv_ptr */
		l_cnt++;
	}

	sprintf(c_writemsg, "%s.%s.%d : %s\n", c_time, c_servicename, i_pid, c_msg);

	/** commented in Ver 1.1 *****
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
	** commented upto here *******/
	/** added in ver 1.1 ****/
	l_msg_len = strlen(c_writemsg) + 1;
	l_wrtn_len = 0;
	l_remn_len = l_msg_len;
	c_tmp_ptr = c_writemsg;
	while (l_wrtn_len < l_msg_len)
	{
		l_val = write(i_pip_fd, c_tmp_ptr, l_remn_len);
		if (l_val == -1)
		{
			return (-1);
		}
		l_wrtn_len = l_wrtn_len + l_val;
		l_remn_len = l_remn_len - l_val;
		c_tmp_ptr = c_tmp_ptr + l_val;
	}
	/** upto here **/
	return 0;
}

void fn_errlog(char *c_servicename,
			   char *c_errno,
			   char *sys_msg,
			   char *c_errmsg)
{
	char c_msg[256];
	long len;

	if (c_errno[0] != 'B')
	{
		fn_userlog(c_servicename, "ERROR - |%s - %s|", c_errno, sys_msg);
		if (strcmp(sys_msg, LIBMSG) != 0)
		{
			strcpy(c_errmsg, "Resource not available");
		}
	}
	else
	{
		strcpy(c_errmsg, "Business error logic not built");
	}

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

/* void rtrim(char* return_string)
{
  *int i;*	* ver 1.1 *
	long i;		* Ver 1.1 *
  int j = 0;
  * int str_length; *
	long str_length;	* Ver 1.1 *
  str_length = strlen(return_string);
  for(i=(str_length-1);return_string[i]==' '; i--)
  {
	j++;
  }
  return_string[i+1]='\0';
  return;
}  Commented in Ver TOL */

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
