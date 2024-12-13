#include<pthread.h>
#include<string.h>
#include<fo.h>
#include<fn_log.h>
#include<errno.h>
#include<fn_pthread.h>

int i_break_status_thrd;

void fn_set_break_thrd()
{
	i_break_status_thrd = EXIT;
}

int fn_thrd_cond_wait( pthread_cond_t *ptr_cond_cntrl,
											 pthread_mutex_t *ptr_mut_cntrl,
											 char *c_ServiceName,
											 char *c_errmsg)
{
	i_break_status_thrd = DONT_EXIT;
	int i_ch_val;

	while (i_break_status_thrd == DONT_EXIT)
	{
		i_ch_val = pthread_cond_wait ( ptr_cond_cntrl , ptr_mut_cntrl );

		if( i_ch_val != 0 )
		{
			if (errno != EINTR)
			{
				fn_userlog(c_ServiceName, "SHAILESH: wait failed 2");
				fn_errlog( c_ServiceName, "L31005", (char *)UNXMSG, c_errmsg);
				return -1;
			}
		}
		else
		{
			return 0;
		}
	}
	fn_userlog(c_ServiceName, "User signal is caught");
	return -1;
}

void( fn_lock_sig_unlock pthread_cond_t *ptr_cond_cntrl,
												pthread_mutex_t *ptr_mut_cntrl,
												int *ptr_i_cond_var,
												int i_value)
{
	pthread_mutex_lock(ptr_mut_cntrl);
	*ptr_i_cond_var = i_value;
	pthread_cond_signal(ptr_cond_cntrl);
	pthread_mutex_unlock(ptr_mut_cntrl);
}
