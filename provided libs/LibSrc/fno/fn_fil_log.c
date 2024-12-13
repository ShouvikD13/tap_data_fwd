/*  Ver 1.1 AIX Migration Rohit */
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <fn_log.h>
#include <fn_fil_log.h>
#include <fo.h>
#include <string.h>

int fn_fillog(FILE *ptr_fp,
			  char *c_servicename,
			  char *c_frmt_str,
			  ...)
{
	int i_val;
	char c_writemsg[512];
	time_t ud_tm_stmp;
	char c_time[12];
	char c_msg[256];
	char c_tmp_msg[256];
	/*int     i_tot_len; */ /* Ver 1.1 */
	long l_tot_len;
	/*int     i_cnt; */
	long l_cnt; /* Ver 1.1 */
	/* int     i_trv_ptr; */
	long l_trv_ptr; /* Ver 1.1*/
	va_list c_arg_lst;
	int i_int;
	long int li_long_int;
	float f_float;
	double d_double;
	char c_char;
	char *c_str;

	ud_tm_stmp = time(NULL);
	strftime(c_time, 12, "%H%M%S", localtime(&ud_tm_stmp));

	va_start(c_arg_lst, c_frmt_str);
	l_trv_ptr = 0; /* i_trv_ptr changed to l_trv_ptr */
	c_msg[0] = '\0';
	l_tot_len = strlen(c_frmt_str); /* i__tot_len changed to l_tot_len */
	l_cnt = 0;						/* i_cnt changed to l_cnt */

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
			/*f_float = va_arg ( c_arg_lst, float ); */ /* Ver 1.1 */
			d_double = va_arg(c_arg_lst, double);
			sprintf(c_tmp_msg, "%s%lf", c_msg, d_double);
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
			/*c_char = va_arg ( c_arg_lst, char ); */
			i_int = va_arg(c_arg_lst, int);
			sprintf(c_tmp_msg, "%s%c", c_msg, i_int);
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
			/*c_char = va_arg ( c_arg_lst, char ); */
			i_int = va_arg(c_arg_lst, int);
			sprintf(c_tmp_msg, "%s%x", c_msg, i_int);
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

	sprintf(c_writemsg, "%s : %s\n", c_time, c_msg);

	i_val = fprintf(ptr_fp, "%s", c_writemsg);
	if (i_val == -1)
	{
		return (-1);
	}

	return 0;
}

int fn_write_file_bin(FILE *ptr_fp,
					  char *c_data,
					  long li_len,
					  char *c_ServiceName,
					  char *c_err_msg)
{
	time_t ud_tm_stmp;
	struct msg_log_hdr st_loghdr;
	/*int i_ch_val; */
	long l_ch_val; /* Ver 1.1 */

	ud_tm_stmp = time(NULL);
	strcpy(st_loghdr.c_time, asctime(localtime(&ud_tm_stmp)));
	st_loghdr.c_time[strlen(st_loghdr.c_time) - 1] = '\0';
	st_loghdr.li_len = li_len;

	l_ch_val = fwrite((char *)&st_loghdr,
					  sizeof(struct msg_log_hdr),
					  1,
					  ptr_fp);
	if (l_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "L31005", UNXMSG, c_err_msg);
		return -1;
	}

	l_ch_val = fwrite(c_data,
					  li_len,
					  1,
					  ptr_fp);
	if (l_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "L31010", (char *)UNXMSG, c_err_msg);
		return -1;
	}
	return 0;
}

int fn_read_file_bin(FILE *ptr_fp,
					 char *c_data,
					 struct msg_log_hdr *ptr_st_loghdr,
					 char *c_ServiceName,
					 char *c_err_msg)
{
	/* int i_ch_val; */
	long l_ch_val; /* Ver 1.1 */
	l_ch_val = fread((char *)ptr_st_loghdr,
					 sizeof(struct msg_log_hdr),
					 1,
					 ptr_fp);
	if ((l_ch_val == -1) && (feof(ptr_fp) != 0))
	{
		fn_errlog(c_ServiceName, "L31015", (char *)UNXMSG, c_err_msg);
		return -1;
	}
	else if (feof(ptr_fp) != 0)
	{
		return END_OF_FILE;
	}

	l_ch_val = fread(c_data,
					 ptr_st_loghdr->li_len,
					 1,
					 ptr_fp);
	if (l_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "L31020", (char *)UNXMSG, c_err_msg);
		return -1;
	}
	return 0;
}

int fn_create_file(FILE **ptr_fp,
				   char *c_fil_nam,
				   char *c_file_mode,
				   char c_fil_path_typ,
				   char *c_ServiceName,
				   char *c_err_msg)
{
	char c_dname[512];
	char c_pname[512];

	time_t ud_tm_stmp;

	char *ptr_c_tmp;

	if (c_fil_path_typ == RELATIVE)
	{
		ud_tm_stmp = time(NULL);
		strftime(c_dname, 10, "%Y%m%d", localtime(&ud_tm_stmp));
		ptr_c_tmp = getenv("EBALOG_PATH");
		if (ptr_c_tmp == NULL)
		{
			fn_errlog(c_ServiceName, "L31025", (char *)UNXMSG, c_err_msg);
			return -1;
		}
		sprintf(c_pname, "%s/%s/%s", ptr_c_tmp, c_dname, c_fil_nam);
	}
	else
	{
		strcpy(c_pname, c_fil_nam);
	}

	*ptr_fp = fopen(c_pname, c_file_mode);
	if (*ptr_fp == NULL)
	{
		fn_errlog(c_ServiceName, "L31030", (char *)UNXMSG, c_err_msg);
		return -1;
	}

	return (0);
}

void fn_destroy_file(FILE *ptr_fp)
{
	fclose(ptr_fp);
	return;
}
