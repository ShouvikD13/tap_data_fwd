/******************************************************************************/
/*  Program           : fn_ors_log.c       	                            */
/*                                                                            */
/*  Description       : Library functions related to Data Logging operations 	*/
/*                                                                            */
/*  Log               :                                                       */
/*                                                                            */
/*  Ver 1.0   10-Aug-2009   Initial Release (Indrajit)                        */
/******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <fo.h>				
#include <fn_ors_log.h>	

int i_pip_fd;
int i_pid;

/******************************************************************************/
/*	This function writes the formatted string to the user defined pipe. It 		*/
/*	variable no of arguments, based on which it creates a formatted string		*/
/*	INPUT PARAMETERS																													*/
/*		c_servicename			- Name of the calling service													*/
/*		c_frmt_str				- String with format specifiers												*/
/*		...								- 'n' no of tokens, where 'n' is the no of format 		*/
/*												specifiers in the previous argument									*/
/*	OUTPUT PARAMETERS																													*/
/*				0 - Success																													*/
/*			 -1 - Failure																													*/
/******************************************************************************/
int fn_userlog(char *c_servicename, char *c_frmt_str, ...)
{
	int			i_val;
	int     i_msg_len;
	int     i_wrtn_len;
	int     i_remn_len;
	time_t	ud_tm_stmp;
	char		c_writemsg[512];
	char		c_time[12];
	char    c_msg[256];
	char    *c_tmp_ptr;
	va_list c_arg_lst;

	ud_tm_stmp = time(NULL);
	strftime(c_time, 12, "%H%M%S", localtime(&ud_tm_stmp)); 

	c_msg[0] = '\0';

	va_start(c_arg_lst, c_frmt_str);
	vsprintf(c_msg, c_frmt_str, c_arg_lst);
	va_end(c_arg_lst);
	sprintf(c_writemsg, "%s.%s.%d : %s\n", c_time, c_servicename, i_pid, c_msg);

	i_msg_len = strlen(c_writemsg) + 1;
	i_wrtn_len = 0;
	i_remn_len = i_msg_len;
	c_tmp_ptr = c_writemsg;
	while(i_wrtn_len < i_msg_len)
	{
		i_val = write(i_pip_fd, c_tmp_ptr, i_remn_len);
		if(i_val == -1)
		{
			return(-1);
		}
		i_wrtn_len = i_wrtn_len + i_val;
		i_remn_len = i_remn_len - i_val;
		c_tmp_ptr = c_tmp_ptr + i_val;
	}

	return 0;
}

/******************************************************************************/
/*	This function writes the error no and error message to the user defined 	*/
/*	pipe alongwith the servicename.																						*/
/*	INPUT PARAMETERS																													*/
/*		c_servicename			- Name of the calling service													*/
/*		c_errno						- Error No																						*/
/*		sys_msg						- Error Message																				*/
/*		c_errmsg					- Error Message string to be returned									*/
/*	OUTPUT PARAMETERS																													*/
/*				None																																*/
/******************************************************************************/
void fn_errlog(char *c_servicename, 
							 char *c_errno, 
							 char *sys_msg, 
							 char *c_errmsg)
{
	char c_msg[256];
	long len;

	if(c_errno[0] != 'B')
	{
		fn_userlog(c_servicename, "ERROR - |%s - %s|", c_errno, sys_msg);
		if(strcmp(sys_msg, LIBMSG) != 0)
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

/******************************************************************************/
/*	This function creates a handler to the user defined pipe. It opens the 		*/
/*	handler in write only mode.																								*/
/*	INPUT PARAMETERS																													*/
/*				None																																*/
/*	OUTPUT PARAMETERS																													*/
/*						0 - Success																											*/
/*					 -1	- Failure																											*/
/******************************************************************************/
int fn_create_pipe()
{
	char c_Logpipe[256];
	char c_msg[256];

	sprintf(c_Logpipe, "%s/LOGPIP", getenv("BIN"));	
	i_pip_fd = open(c_Logpipe, O_WRONLY | O_NDELAY);
	if(i_pip_fd == -1)
	{
		printf("EBA : Error creating pipe :%s:\n", UNXMSG);
		return(-1);
	}
	i_pid=getpid();
	return(0);
}

/******************************************************************************/
/*	This function closes the handler to the user defined pipe. 								*/
/*	INPUT PARAMETERS																													*/
/*				None																																*/
/*	OUTPUT PARAMETERS																													*/
/*				None																																*/
/******************************************************************************/
void fn_destroy_pipe()
{
	close(i_pip_fd);
	return;
}

/******************************************************************************/
/*	This function trims the blank spaces on the right side of the string			*/
/*	INPUT PARAMETERS																													*/
/*		return_string 	- String which is to be trimmed													*/
/*	OUTPUT PARAMETERS																													*/
/*				None																																*/
/******************************************************************************/
void rtrim(char* return_string)
{
  int i;
  int j = 0;
  int str_length;
  str_length = strlen(return_string);
  for(i = (str_length - 1); return_string[i] == ' '; i--)
  {
    j++;
  }
  return_string[i + 1] = '\0';
  return;
}

