/******************************************************************************/
/*  Program           : fo_shm.c                                              */
/*                                                                            */
/*  Input             :  -                                                    */
/*                                                                            */
/*  Output            :  -                                                    */
/*                                                                            */
/*  Description       :  Library functions related to Shared Memory           */
/*											 operations                                           */
/*                                                                            */
/*  Log               :                                                       */
/*                                                                            */
/******************************************************************************/

/**** C header ****/
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<errno.h>
#include <fn_log.h>  
#include <userlog.h> 
#include <string.h> 
#include <signal.h>
#include <stdlib.h>

#include<fo_shm.h> 
#include<fo.h>

extern long l_temp_tot_cnt; 
extern char* fn_get_prcs_spc(char *c_Servicename, char *c_token);

static long l_tot_cnt;

/**** Global variables ****/
int i_global_sem_id;	/*	semaphore identifier	*/
int i_global_shm_id;	/*	shared memory identifier	*/
int i_break_status_shm;	/* To identify user signal caught	*/

/******************************************************************************/
/* This function decreases the semaphore value by 1. This implies that it     */
/* wants to obtain a resource that the semaphore controls. If semaphore value */
/* is not greater than 0,waits till the value become gretaer than 0 (waits    */
/* for resource release).                                                     */
/*	INPUT PARAMETERS																													*/
/*		ptr_c_serviceName		-		Name of the process															*/
/*		ptr_c_errmsg				-		Error Message																		*/
/*	RETURN VALUE																														  */
/*							 	0 on success                                                */
/*							 -1 on failure                                                */
/******************************************************************************/
int fn_P( char *ptr_c_serviceName, 
					char *ptr_c_errmsg)
{
	struct sembuf st_sem_buf;

	/*******Populate semaphore buffer structure ********/
	st_sem_buf.sem_num = 0;
	st_sem_buf.sem_op =  1;
	st_sem_buf.sem_flg = 0;
	i_break_status_shm = DONT_EXIT;

	while(i_break_status_shm == DONT_EXIT )
	{
		if( semop( i_global_sem_id, &st_sem_buf, 1) == -1)
		{
			if(errno != EINTR)
			{
				fn_errlog(ptr_c_serviceName, "L31005", UNXMSG, ptr_c_errmsg);
				return -1;
			}
		}
		else
		{
			return 0;	
		}
	}
	fn_errlog(ptr_c_serviceName, "L31010", "Interrupt caught", ptr_c_errmsg);
	return -1;
}

/******************************************************************************/
/* This function increases the semaphore value by 1. This implies that it     */
/* wants to return a resource that the semaphore controls. If semaphore value */
/* is not greater than 0,waits till the value become gretaer than 0 (waits    */
/* for resource release).                                                     */
/*	INPUT PARAMETERS																													*/
/*		ptr_c_serviceName		-		Name of the process															*/
/*		ptr_c_errmsg				-		Error Message																		*/
/*	RETURN VALUE																														  */
/*							 	0 on success                                                */
/*							 -1 on failure                                                */
/******************************************************************************/
int fn_V(char *ptr_c_serviceName,
          char *ptr_c_errmsg)
{
  struct sembuf st_sem_buf;

  /*******Populate semaphore buffer structure ********/
  st_sem_buf.sem_num = 0;
  st_sem_buf.sem_op = 1;
  st_sem_buf.sem_flg = 0;
  if( semop( i_global_sem_id, &st_sem_buf, 1) == -1)
  {
		fn_errlog(ptr_c_serviceName, "L31015", UNXMSG, ptr_c_errmsg);
    return -1;
  }

  return 0;
}
 
/******************************************************************************/
/*	To get a semaphore identifier																							*/
/*	INPUT PARAMETERS																													*/
/*		i_proc_type					-		Process type																		*/
/*		ptr_c_serviceName		-		Name of the process															*/
/*		ptr_c_errmsg				-		Error Message																		*/
/*	RETURN VALUE																														  */
/*							 	0 on success                                                */
/*							 -1 on failure                                                */
/******************************************************************************/

int fn_get_shm_id(int i_proc_type,
									char *ptr_c_serviceName,
								  long l_shm_size,
									char *ptr_c_errmsg
								)
{
	char *ptr_c_tmp;
	int i_shm_ipc_key;
	int i_sem_ipc_key;
	union semun 
	{
     int val;
     struct semid_ds *buf;
     ushort *array;
	} arg;

  size_t shm;
  shm = l_shm_size; 
  ptr_c_tmp = (char *)fn_get_prcs_spc ( ptr_c_serviceName, "SEM_IPC_KEY" );
  if ( ptr_c_tmp == NULL )
  {
		fn_errlog(ptr_c_serviceName, "L31020", LIBMSG, ptr_c_errmsg);
		return -1;
	}
	i_sem_ipc_key = atoi(ptr_c_tmp);

  ptr_c_tmp = (char *)fn_get_prcs_spc ( ptr_c_serviceName, "SHM_IPC_KEY" );
  if ( ptr_c_tmp == NULL )
  {
		fn_errlog(ptr_c_serviceName, "L31025", LIBMSG, ptr_c_errmsg);
		return -1;
	}
	i_shm_ipc_key = atoi(ptr_c_tmp);

	if(i_proc_type == OWNER)
	{
		/*	Get a semaphore identifier	*/
		if((i_global_sem_id = semget(i_sem_ipc_key, 1, 
															( IPC_CREAT | IPC_EXCL | 0777 ))) == -1)
		{
			fn_errlog(ptr_c_serviceName, "L31030", UNXMSG, ptr_c_errmsg);
			return -1;
		}

		/* Get shared memory identifier	*/
		if(( i_global_shm_id = 
					shmget(i_shm_ipc_key,shm ,( IPC_CREAT | IPC_EXCL | 0666))) == -1)
		{
			fn_errlog(ptr_c_serviceName, "L31035", UNXMSG, ptr_c_errmsg);
			semctl(i_global_sem_id, 0, IPC_RMID, 0);
			return -1;
		}

		/*	set semaphore value as 1	*/
		arg.val = 1;
		if( semctl( i_global_sem_id, 0, SETVAL, arg) == -1)
		{
			fn_errlog(ptr_c_serviceName, "L31040", UNXMSG, ptr_c_errmsg);
			fn_dstry_shm(ptr_c_serviceName,ptr_c_errmsg);
			return -1;
		}

	}
	else if(i_proc_type == CLIENT)
	{
		/*	Get a semaphore identifier	*/
		if((i_global_sem_id = semget(i_sem_ipc_key, 1, 0)) == -1)
		{
			if( errno == ENOENT)
			{
				fn_errlog(ptr_c_serviceName,"L31045","Semaphore doesn't exist",
									ptr_c_errmsg);
				return 1;
			}
			else
			{
				fn_errlog(ptr_c_serviceName, "L31050", UNXMSG, ptr_c_errmsg);
				return -1;
			}
		}

		/* Get existing shared memory identifier	*/
		if((i_global_shm_id = shmget(i_shm_ipc_key, shm , 0)) == -1)
		{
			if( errno == ENOENT)
			{
				fn_errlog(ptr_c_serviceName,"L31055","Shared Memory doesn't exist",
									ptr_c_errmsg);
				return 1;
			}
			else
			{
				fn_errlog(ptr_c_serviceName, "L31060", UNXMSG, ptr_c_errmsg);
				return -1;
			}
		}
	}
	else
	{
		fn_errlog(ptr_c_serviceName,"L31065","Invalid Process type", ptr_c_errmsg);
		return -1;
	}
	return 0;
}

/******************************************************************************/
/*	To attach shared memory segment to the process														*/
/*	INPUT PARAMETERS																													*/
/*		ptr_v_shm_addr			-		Shared memory address pointer is returned       */
/*		ptr_c_serviceName		-		Name of the process															*/
/*		ptr_c_errmsg				-		Error Message																		*/
/*	RETURN VALUE																														  */
/*							 	0 on success                                                */
/*							 -1 on failure                                                */
/******************************************************************************/
int fn_attach_shm(void **ptr_v_shm_addr,
									char *ptr_c_serviceName,
									char *ptr_c_errmsg
									)
{
	/*	set semaphore lock	*/
	if( fn_P(ptr_c_serviceName, ptr_c_errmsg ) == -1)
	{
		return -1;
	}

	if((*ptr_v_shm_addr = shmat( i_global_shm_id, 0, 0)) == (char *)-1)
	{
			fn_errlog(ptr_c_serviceName, "L31070", UNXMSG, ptr_c_errmsg);
			if( fn_V(ptr_c_serviceName, ptr_c_errmsg) == -1)
			{
				return -1;
			}	
			return -1;
	}

	return 0;
}

/******************************************************************************/
/*  To detach shared memory segment from the process                          */
/*  INPUT PARAMETERS                                                          */
/*    ptr_v_shm_addr      -   pointer to Shared memory to be released         */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_detach_shm(void **ptr_v_shm_addr,
                  char *ptr_c_serviceName,
                  char *ptr_c_errmsg
                  )
{
  if(shmdt( *ptr_v_shm_addr) == -1)
  {
      fn_errlog(ptr_c_serviceName, "L31075", UNXMSG, ptr_c_errmsg);
      if( fn_V(ptr_c_serviceName, ptr_c_errmsg) == -1)
      {
        return -1;
      }
      return -1;
  }

  /*  Release semaphore lock  */
  if( fn_V(ptr_c_serviceName, ptr_c_errmsg ) == -1)
  {
    return -1;
  }

	return 0;
}

int fn_dstry_shm(char *ptr_c_serviceName,
								 char *ptr_c_errmsg)
{


	/*	remove shared memory identifier	*/
	if( shmctl(i_global_shm_id, IPC_RMID, NULL) == -1)
	{
		fn_errlog(ptr_c_serviceName, "L31080", UNXMSG, ptr_c_errmsg);
	  return -1;	
	}


	/*	remove semaphore identifier	*/
	if(semctl(i_global_sem_id, 0, IPC_RMID, 0) == -1)	
	{
		fn_errlog(ptr_c_serviceName, "L31085", UNXMSG, ptr_c_errmsg);
		return -1;
	}
	return 1;
}

