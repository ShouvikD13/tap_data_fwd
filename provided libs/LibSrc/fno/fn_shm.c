/******************************************************************************/
/*  Program           : fn_shm.c                                              */
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
/* Sridhar.T.N   version1.0 	25-oct-01                                       */
/* Smitha Nuti   version1.1 	30-Oct-07    AIX migration                      */
/* Shailesh Hinge version1.2 	02-Jan-09    NNF change for Extended Market			*/
/******************************************************************************/

/**** C header ****/
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<errno.h>
#include <fn_log.h>   	/* Ver 1.1 */
#include <userlog.h>    /* Ver 1.1 */
#include <string.h>    	/* Ver 1.1 */
#include <signal.h>     /* Ver 1.1 */
#include <stdlib.h>     /* Ver 1.1 */

#include<fn_shm.h> 
#include<fo.h>

/* Ver 1.1 add */

extern char* fn_get_prcs_spc(char *c_Servicename, char *c_token);

/* Ver 1.1 add ends */

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
	st_sem_buf.sem_op = -1;
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
		        	/* COMMENTED IN VER TOL shmget(i_shm_ipc_key,SHM_SIZE,( IPC_CREAT | IPC_EXCL | 0666))) == -1) */
				shmget(i_shm_ipc_key,SHM_STAT,( IPC_CREAT | IPC_EXCL | 0666))) == -1)
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

		/* initialize shared memory	*/
		if( fn_init_shm( ptr_c_serviceName, ptr_c_errmsg) == -1)
		{
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
				return -1;
			}
			else
			{
				fn_errlog(ptr_c_serviceName, "L31050", UNXMSG, ptr_c_errmsg);
				return -1;
			}
		}

		/* Get existing shared memory identifier	*/
		if((i_global_shm_id = shmget(i_shm_ipc_key, SHM_STAT, 0)) == -1) /* SHM_SIZE chnaged to SHM_STAT in VER TOL :TUX on LINUX */
		{
			if( errno == ENOENT)
			{
				fn_errlog(ptr_c_serviceName,"L31055","Shared Memory doesn't exist",
									ptr_c_errmsg);
				return -1;
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

/******************************************************************************/
/*  To Initialize shared memory contents                                      */
/*  INPUT PARAMETERS                                                          */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_init_shm(char *ptr_c_serviceName, 
								char *ptr_c_errmsg
								)
{
	struct st_prcs_data *ptr_st_tmp_procs_data;
	struct st_xchg_data *ptr_st_tmp_xchg_data;
	struct st_ord_refack *ptr_st_ord_refack;
	char *ptr_v_shm_addr;
	int i_tmp;

	/*	attach shared memory	*/
	if((ptr_v_shm_addr = (char *)shmat( i_global_shm_id, 0, 0)) == (char *) -1)
	{
			fn_errlog(ptr_c_serviceName, "L31080", UNXMSG, ptr_c_errmsg);
			return -1;
	}

	/*	initialize process data	*/
	for(i_tmp =0; i_tmp < MAX_PRCS;	i_tmp++)
	{
		ptr_st_tmp_procs_data = 
			(struct st_prcs_data *)(ptr_v_shm_addr + (i_tmp * PROC_DATA_SIZE));
		ptr_st_tmp_procs_data->ul_prcs_id = 0;
		ptr_st_tmp_procs_data->uc_prcs_no = (i_tmp + 1);
		ptr_st_tmp_procs_data->uc_prcs_stts = STOP;
	}
	
	/*	initialize exchange status and market status	*/
	ptr_st_tmp_xchg_data = 
				(struct st_xchg_data *)(ptr_v_shm_addr + XCHG_DATA_START);
	ptr_st_tmp_xchg_data->uc_xchg_logon = NOT_LOGGED;
	ptr_st_tmp_xchg_data->uc_ord_mkt_stts = CLOSE;
	ptr_st_tmp_xchg_data->uc_ex_mkt_stts = CLOSE;
	ptr_st_tmp_xchg_data->uc_pl_mkt_stts = CLOSE;
  ptr_st_tmp_xchg_data->uc_ext_mkt_stts = CLOSE;          /*Ver 1.2 */

	/*** Initialize all the counters to zero. ***/
	memset ( (ptr_v_shm_addr + MTRCS_DATA_STRT ), '\0', MTRCS_DATA_SIZE );  

	/*** Initialize the order reference and acknowledgement as "*" ***/
	ptr_st_ord_refack=(struct st_ord_refack *)(ptr_v_shm_addr +	REFACK_DATA_STRT);
	strcpy(ptr_st_ord_refack->c_ord_ref, "*");
	strcpy(ptr_st_ord_refack->c_ord_ack, "*");

	/*** Initialize Nifty contract details to null. ***/
	memset ( (ptr_v_shm_addr + NIFTY_DATA_STRT ), '\0', NIFTY_DATA_SIZE );  

	/*	detach shared memory */
	if( shmdt(ptr_v_shm_addr) == -1)
	{
		fn_errlog(ptr_c_serviceName, "L31085", UNXMSG, ptr_c_errmsg);
		return -1;
	}

	return 0;
}

/******************************************************************************/
/*  To get exchange log on status                                             */
/*  INPUT PARAMETERS                                                          */
/*		ptr_i_xchg_stts			_		pointer to Exchange status to be returned			  */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_get_xchg_logon_stts(int *ptr_i_xchg_stts,
													 char *ptr_c_serviceName,
													 char *ptr_c_errmsg
													)
{
	struct st_xchg_data *ptr_st_tmp_xchg_data;
	char *ptr_v_shm_addr;	
	/*	attach shared memory	*/
	if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) 				== -1)
	{
		return -1;
	}
	
	/*	get exchange log on status	*/
	ptr_st_tmp_xchg_data = 
					(struct st_xchg_data *) (ptr_v_shm_addr + XCHG_DATA_START);
	*ptr_i_xchg_stts = ptr_st_tmp_xchg_data->uc_xchg_logon;

	/*	detach shared memory	*/
	if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
	{
		return -1;
	}
	return 0;
}

/******************************************************************************/
/*  To set exchange log on status                                             */
/*  INPUT PARAMETERS                                                          */
/*    i_xchg_stts     		-   Exchange status 												        */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_set_xchg_logon_stts(int i_xchg_stts,
                           char *ptr_c_serviceName,
                           char *ptr_c_errmsg
                          )
{
  struct st_xchg_data *ptr_st_tmp_xchg_data;
	char *ptr_v_shm_addr;	

  /*  attach shared memory  */
  if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

  /*  get exchange log on status  */
  ptr_st_tmp_xchg_data =
          (struct st_xchg_data *) (ptr_v_shm_addr + XCHG_DATA_START);
  ptr_st_tmp_xchg_data->uc_xchg_logon = i_xchg_stts;

  /*  detach shared memory  */
  if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }
	return 0;
}

/******************************************************************************/
/*  To get the statuses of all process                                        */
/*  INPUT PARAMETERS                                                          */
/*    i_prcs_no [ ]				-   Process number array														*/
/*    i_prcs_stts [ ]			-   Process number array												  	*/
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_get_all_prcs_stts(int i_prcs_no [ ],
                         int i_prcs_stts [ ],
                         char *ptr_c_serviceName,
                         char *ptr_c_errmsg
                        )
{
	int i_tmp;
	struct st_prcs_data *ptr_st_tmp_prcs_data;
	char *ptr_v_shm_addr;	

  /*  attach shared memory  */
  if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }
	
	/*	to get all status and store in a given structure	*/
	for(i_tmp = 0; i_tmp < MAX_PRCS ; i_tmp++)
	{
		ptr_st_tmp_prcs_data = 
			(struct st_prcs_data *) (ptr_v_shm_addr + (i_tmp * PROC_DATA_SIZE));
		i_prcs_no[i_tmp]= ptr_st_tmp_prcs_data->uc_prcs_no;
		i_prcs_stts[i_tmp]= ptr_st_tmp_prcs_data->uc_prcs_stts;
	}

  /*  detach shared memory  */
  if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

	return 0;
}

/******************************************************************************/
/*  To get the status of specified process                                    */
/*  INPUT PARAMETERS                                                          */
/*    ptr_i_prcs_stts     -   pointer to a process status to be returned		  */
/*		i_prcs_no						-		Process number for which status is needed				*/
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_get_prcs_stts(int *ptr_i_prcs_stts,
										 int i_prcs_no,
                     char *ptr_c_serviceName,
                     char *ptr_c_errmsg
                    )
{
  int i_tmp;
  struct st_prcs_data *ptr_st_tmp_prcs_data;
	char *ptr_v_shm_addr;	
	
	if( ( i_prcs_no > MAX_PRCS ) || ( i_prcs_no < 1 ) )
	{
		fn_errlog(ptr_c_serviceName,"L31090","Invalid process No", ptr_c_errmsg);
		return -1;
	}

  /*  attach shared memory  */
  if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

  /*  to get status of a process and store in a given pointer    */
  ptr_st_tmp_prcs_data =
   (struct st_prcs_data *)(ptr_v_shm_addr + ((i_prcs_no - 1) * PROC_DATA_SIZE));
	*ptr_i_prcs_stts = ptr_st_tmp_prcs_data->uc_prcs_stts;
  
	/*  detach shared memory  */
  if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }
	return 0;
}

/******************************************************************************/
/*  To set the status of specified process                                    */
/*  INPUT PARAMETERS                                                          */
/*    i_prcs_stts		      -   pointer to a process status to be returned      */
/*    i_prcs_no           -   Process number for which status is needed       */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_set_prcs_stts(int i_prcs_no,
                     int i_prcs_stts,
                     char *ptr_c_serviceName,
                     char *ptr_c_errmsg
                    )
{
  struct st_prcs_data *ptr_st_tmp_prcs_data;
	char *ptr_v_shm_addr;	
	
	if( ( i_prcs_no > MAX_PRCS ) || ( i_prcs_no < 1 ) )
	{
		fn_errlog(ptr_c_serviceName,"L31095","Invalid process No", ptr_c_errmsg);
		return -1;
	}

  /*  attach shared memory  */
  if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

  /*  to set status of a process and store in a given pointer    */
  ptr_st_tmp_prcs_data =
   (struct st_prcs_data *)(ptr_v_shm_addr + ((i_prcs_no - 1) * PROC_DATA_SIZE));
  ptr_st_tmp_prcs_data->uc_prcs_stts = i_prcs_stts;

  /*  detach shared memory  */
  if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }
	return 0;
}

/******************************************************************************/
/*  To remove shared memory identifier and semaphore identifier               */
/*  INPUT PARAMETERS                                                          */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
void fn_dstry_shm(char *ptr_c_serviceName,
								 char *ptr_c_errmsg)
{
	/*	remove shared memory identifier	*/
	if( shmctl(i_global_shm_id, IPC_RMID, NULL) == -1)
	{
		fn_errlog(ptr_c_serviceName, "L31100", UNXMSG, ptr_c_errmsg);
	}

	/*	remove semaphore identifier	*/
	if(semctl(i_global_sem_id, 0, IPC_RMID, 0) == -1)	
	{
		fn_errlog(ptr_c_serviceName, "L31105", UNXMSG, ptr_c_errmsg);
	}
}

/******************************************************************************/
/*  To get status of a specified market type                                  */
/*  INPUT PARAMETERS                                                          */
/*		i_mkt_type					-		market type for which status is to be returned	*/
/*		ptr_i_stts					-		pointer to status variable where 								*//*														result is stored																*/
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_get_mkt_stts(int i_mkt_type,
										int *ptr_i_stts,
                    char *ptr_c_serviceName,
                    char *ptr_c_errmsg
                   )
{
	struct st_xchg_data *ptr_st_tmp_xchg_data;
	char *ptr_v_shm_addr;	

	if ( ( i_mkt_type != ORDER_MKT ) &&
			 ( i_mkt_type != EXERCISE_MKT ) &&
			 ( i_mkt_type != PL_MKT ) 	&&	
       ( i_mkt_type != EXTND_MKT )    )     /*Ver 1.2 */
	{
		fn_errlog(ptr_c_serviceName, "L31110", "Invalid Market type", ptr_c_errmsg);
		return -1;
	}

  /*  attach shared memory  */
  if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

	/*	To get exchange data structure	*/
	ptr_st_tmp_xchg_data =
				(struct st_xchg_data *)(ptr_v_shm_addr + XCHG_DATA_START);

	/*	based on type, read market status	*/
	switch(i_mkt_type)
	{
		case ORDER_MKT:		
			*ptr_i_stts = ptr_st_tmp_xchg_data->uc_ord_mkt_stts;
			break;

/*Added in Ver 1.2 */
    case EXTND_MKT:
      *ptr_i_stts = ptr_st_tmp_xchg_data->uc_ext_mkt_stts;
      break;

		case EXERCISE_MKT:		
			*ptr_i_stts = ptr_st_tmp_xchg_data->uc_ex_mkt_stts;
			break;

		case PL_MKT:		
			*ptr_i_stts = ptr_st_tmp_xchg_data->uc_pl_mkt_stts;
			break;

		default:
			fn_errlog(ptr_c_serviceName,"L31115","Invalid market type", ptr_c_errmsg);
 			/*  detach shared memory  */
  		if( fn_detach_shm((void *)&ptr_v_shm_addr, 
												ptr_c_serviceName, 
												ptr_c_errmsg) == -1)
      {
        return -1;
      }
			return -1;
			break;
	}
  
	/*  detach shared memory  */
  if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }
	return 0;
}

/******************************************************************************/
/*  To set status of a specified market type                                  */
/*  INPUT PARAMETERS                                                          */
/*    i_mkt_type          -   market type for which status is to be returned  */
/*    i_stts          		-   market status to be updated                     */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_set_mkt_stts(int i_mkt_type,
                    int i_stts,
                    char *ptr_c_serviceName,
                    char *ptr_c_errmsg
                   )
{
	struct st_xchg_data *ptr_st_tmp_xchg_data;
	char *ptr_v_shm_addr;	

	if ( ( i_mkt_type != ORDER_MKT ) &&
			 ( i_mkt_type != EXERCISE_MKT ) &&
			 ( i_mkt_type != PL_MKT ) 	&&	
       ( i_mkt_type != EXTND_MKT )    )     /*Ver 1.2 */
	{
		fn_errlog(ptr_c_serviceName, "L31120", "Invalid Market type", ptr_c_errmsg);
		return -1;
	}

  /*  attach shared memory  */
  if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

  /*  To get exchange data structure  */
  ptr_st_tmp_xchg_data =
        (struct st_xchg_data *)(ptr_v_shm_addr + XCHG_DATA_START);

  /*  based on type, read market status */
  switch(i_mkt_type)
  {
    case ORDER_MKT:
      ptr_st_tmp_xchg_data->uc_ord_mkt_stts = i_stts;
      break;

   /*Added in Ver 1.2 */
    case EXTND_MKT:
      ptr_st_tmp_xchg_data->uc_ext_mkt_stts =i_stts;
      break;

    case EXERCISE_MKT:
      ptr_st_tmp_xchg_data->uc_ex_mkt_stts = i_stts;
      break;

    case PL_MKT:
      ptr_st_tmp_xchg_data->uc_pl_mkt_stts = i_stts;
      break;

		default:
      fn_errlog(ptr_c_serviceName,"L31125","Invalid market type", ptr_c_errmsg);
      /*  detach shared memory  */
  		if( fn_detach_shm((void *)&ptr_v_shm_addr, 
												ptr_c_serviceName, 
												ptr_c_errmsg) == -1)
      {
        return -1;
      }
			return -1;
			break;
  }

  /*  detach shared memory  */
  if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }
  return 0;
}

/******************************************************************************/
/*  To get the process number of a process for which process ID is given      */
/*  INPUT PARAMETERS                                                          */
/*    li_proc_id          -   process ID                                      */
/*    ptr_i_proc_no       -   process identification number to be returned    */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_get_proc_no(long int li_proc_id,
									 int *ptr_i_proc_no,
                   char *ptr_c_serviceName,
                   char *ptr_c_errmsg
                  )
{
	struct st_prcs_data *ptr_st_tmp_prcs_data;
	char *ptr_v_shm_addr;	
	int i_tmp;

  /*  attach shared memory  */
  if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

	/*	to get corresponding process identification number	*/
	for(i_tmp = 0; i_tmp < MAX_PRCS; i_tmp++)
	{
		ptr_st_tmp_prcs_data	=
			(struct st_prcs_data *)(ptr_v_shm_addr + (i_tmp * PROC_DATA_SIZE));
		if(ptr_st_tmp_prcs_data->ul_prcs_id	==	li_proc_id)
		{
			*ptr_i_proc_no	=	ptr_st_tmp_prcs_data->uc_prcs_no;
			break;
		}
	}
	
	/*	If match is not found	*/
	if(i_tmp == MAX_PRCS)
	{
		fn_errlog(ptr_c_serviceName,"L31130", "Process ID not found", ptr_c_errmsg);
	  /*  detach shared memory  */
 		if( fn_detach_shm((void *)&ptr_v_shm_addr, 
											ptr_c_serviceName, 
											ptr_c_errmsg) == -1)
    {
   		 return -1;
  	}
		return -1;
	}

  /*  detach shared memory  */
  if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

  return 0;
}

/******************************************************************************/
/*  To get the process ID of a process for which process identification				*/
/*	number is given      																											*/
/*  INPUT PARAMETERS                                                          */
/*    i_proc_no           -   process identification number                   */
/*    ptr_li_proc_id      -   process ID to be returned										    */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_get_proc_id(int i_proc_no,
									 long int *ptr_li_proc_id,
                   char *ptr_c_serviceName,
                   char *ptr_c_errmsg
                  )
{
	struct st_prcs_data *ptr_st_tmp_prcs_data;
	char *ptr_v_shm_addr;	
	
	if( ( i_proc_no > MAX_PRCS ) || ( i_proc_no < 1 ) )
	{
		fn_errlog(ptr_c_serviceName,"L31135","Invalid process No", ptr_c_errmsg);
		return -1;
	}

  /*  attach shared memory  */
  if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

	/*	get process ID of a process	*/
	ptr_st_tmp_prcs_data	=
	(struct st_prcs_data *)(ptr_v_shm_addr + ((i_proc_no - 1) * PROC_DATA_SIZE));
	*ptr_li_proc_id = ptr_st_tmp_prcs_data->ul_prcs_id;

  /*  detach shared memory  */
  if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

  return 0;
}

/******************************************************************************/
/*  To raise a given signal to a specified process                            */
/*  INPUT PARAMETERS                                                          */
/*    i_proc_no           -   process identification number                   */
/*    i_signo				      -   the signal to be raised                         */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_raise_sig(int i_proc_no,
								 int i_signo,
								 char *ptr_c_serviceName,
								 char *ptr_c_errmsg
								)
{
	long int li_tmp_proc_id;
	union sigval st_data;
	/*	Get process ID	*/
	if(fn_get_proc_id(i_proc_no, &li_tmp_proc_id, ptr_c_serviceName, ptr_c_errmsg)
												==	-1)
	{
		return -1;
	}
	
	st_data.sival_int = i_signo;

	/*	raise a signal	*/
	if ( li_tmp_proc_id != 0 )
	{
		if( sigqueue(li_tmp_proc_id, i_signo, st_data) == -1)
		{
			fn_errlog(ptr_c_serviceName, "L31140", UNXMSG, ptr_c_errmsg);
			return -1;
		}
		fn_userlog(ptr_c_serviceName, "Signal(%d) raised to %d",i_signo, i_proc_no);
	}
	return 0;
}

/******************************************************************************/
/* This function sets the break_status_shm variable to indicate that the user */
/* signal is caught.                                                          */
/******************************************************************************/
void fn_set_break_shm()
{
  i_break_status_shm = EXIT;
}

/******************************************************************************/
/*  To set the process ID of a process for which process identification				*/
/*	number is given      																											*/
/*  INPUT PARAMETERS                                                          */
/*    i_proc_no           -   process identification number                   */
/*    li_proc_id      		-   process ID 																	    */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_set_proc_id(int i_proc_no,
									 long int li_proc_id,
                   char *ptr_c_serviceName,
                   char *ptr_c_errmsg
                  )
{
	struct st_prcs_data *ptr_st_tmp_prcs_data;
	char *ptr_v_shm_addr;	
	
	if( ( i_proc_no > MAX_PRCS ) || ( i_proc_no < 1 ) )
	{
		fn_errlog(ptr_c_serviceName,"L31145","Invalid process No", ptr_c_errmsg);
		return -1;
	}

  /*  attach shared memory  */
  if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

	/*	get process ID of a process	*/
	ptr_st_tmp_prcs_data	=
	(struct st_prcs_data *)(ptr_v_shm_addr + ((i_proc_no - 1) * PROC_DATA_SIZE));
	ptr_st_tmp_prcs_data->ul_prcs_id = li_proc_id;

  /*  detach shared memory  */
  if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
  {
    return -1;
  }

  return 0;
}

int fn_set_cntr ( long int li_metric_cntr,
                  char *c_serviceName, 
                  char *c_errmsg)
{
	char *ptr_v_shm_addr;
	long int *ptr_li_temp_val;

	/***  attach  to shared memory    ****/
	if( fn_attach_shm((void *)&ptr_v_shm_addr,
                     c_serviceName,
                     c_errmsg) == -1)
	{
		fn_errlog(c_serviceName,"L31150", LIBMSG, c_errmsg);
		return -1;
	}
	/*** Increment the respective metric counter and store in a given pointer. **/
	ptr_li_temp_val = (long int *)
					(ptr_v_shm_addr +MTRCS_DATA_STRT +(li_metric_cntr * MTRCS_CNTR_SIZE));
	(*ptr_li_temp_val)++;

	/***  detach  from the shared memory  ****/
	if( fn_detach_shm ((void *)&ptr_v_shm_addr,
                      c_serviceName,
                      c_errmsg) == -1)
	{
		fn_errlog(c_serviceName,"L31155", LIBMSG, c_errmsg);
		return -1;
	}
	return 0;
}


int fn_get_cntr ( long int li_metric_cntr[ ],
                  char *c_serviceName, 
                  char *c_errmsg)
{

	char *ptr_v_shm_addr;

	/***  attach  to shared memory  ****/
	if( fn_attach_shm((void *)&ptr_v_shm_addr,
                     c_serviceName,
                     c_errmsg) == -1)
	{
		fn_errlog(c_serviceName,"L31160", LIBMSG, c_errmsg);
		return -1;
	}

	/** memcopy the existing values of various counters in shared memory. **/
	memcpy ( (char*)li_metric_cntr, ((char*)ptr_v_shm_addr+ MTRCS_DATA_STRT), 
																MTRCS_DATA_SIZE );

	/*** detach  from the shared memory ***/
	if( fn_detach_shm ((void *)&ptr_v_shm_addr,
                      c_serviceName,
                      c_errmsg) == -1)
	{
		fn_errlog(c_serviceName,"L31165", LIBMSG, c_errmsg);
		return -1;
	}
	return 0;
}


int fn_reset_cntrs ( char *c_serviceName, 
                  	 char *c_errmsg)
{
	char *ptr_v_shm_addr;

	/***  attach  to shared memory  ****/
	if( fn_attach_shm((void *)&ptr_v_shm_addr,
                     c_serviceName,
                     c_errmsg) == -1)
	{
		fn_errlog(c_serviceName,"L31170", LIBMSG, c_errmsg);
		return -1;
	}

	/*** Initialize all the counters to zero. ***/
	memset ( (ptr_v_shm_addr + MTRCS_DATA_STRT ), '\0', MTRCS_DATA_SIZE );  

	/*** detach  from the shared memory ***/
	if( fn_detach_shm ((void *)&ptr_v_shm_addr,
                      c_serviceName,
                      c_errmsg) == -1)
	{
		fn_errlog(c_serviceName,"L31175", LIBMSG, c_errmsg);
		return -1;
	}
	return 0;
}

/******************************************************************************/
/*  To set Order reference/ Exchange ack details                              */
/*  INPUT PARAMETERS                                                          */
/*		ptr_c_ref_ack			  -		pointer to character array of Order reference / */
/*														Exchange acknowledgement											  */
/*		i_type							- 	Type of input whether request or response				*/
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_set_ref_ack_dtls(char *ptr_c_ref_ack,
												int i_type,
											  char *ptr_c_serviceName,
											  char *ptr_c_errmsg)
{
	char *ptr_v_shm_addr;	
	struct st_ord_refack *ptr_st_ord_refack;

	/*	attach shared memory	*/
	if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) 				== -1)
	{
		return -1;
	}
	
	ptr_st_ord_refack = (struct st_ord_refack *)(ptr_v_shm_addr+REFACK_DATA_STRT);
	if ( i_type == ORD_REQ )
	{
		strcpy ( ptr_st_ord_refack->c_ord_ref, ptr_c_ref_ack);
	}
	else
	{
		strcpy ( ptr_st_ord_refack->c_ord_ack, ptr_c_ref_ack);
	} 

	/*	detach shared memory	*/
	if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
	{
		return -1;
	}
	return 0;
}

/******************************************************************************/
/*  To get Order reference/ Exchange ack details                              */
/*  INPUT PARAMETERS                                                          */
/*		ptr_st_ref_ack		  -		pointer to structure of Order reference / 			*/
/*														Exchange acknowledgement											  */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_get_ref_ack_dtls(struct st_ord_refack *ptr_st_ref_ack,
											  char *ptr_c_serviceName,
											  char *ptr_c_errmsg)
{
	char *ptr_v_shm_addr;	
	struct st_ord_refack *ptr_st_ord_refack;

	/*	attach shared memory	*/
	if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) 				== -1)
	{
		return -1;
	}
	
	ptr_st_ord_refack = (struct st_ord_refack *)(ptr_v_shm_addr+REFACK_DATA_STRT);
	strcpy ( ptr_st_ref_ack->c_ord_ref,ptr_st_ord_refack->c_ord_ref);
	strcpy ( ptr_st_ref_ack->c_ord_ack,ptr_st_ord_refack->c_ord_ack);

	/*	detach shared memory	*/
	if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
	{
		return -1;
	}
	return 0;
}

/******************************************************************************/
/*  To get/set Nifty contract details                                         */
/*  INPUT PARAMETERS                                                          */
/*		ptr_st_nifty_dat		-		pointer to structure of nifty data              */
/*		i_type							-		whether to get / set nifty data                 */
/*    ptr_c_serviceName   -   Name of the process                             */
/*    ptr_c_errmsg        -   Error Message                                   */
/*  RETURN VALUE                                                              */
/*                0 on success                                                */
/*               -1 on failure                                                */
/******************************************************************************/
int fn_getorset_nifty_data(struct st_nifty_data *ptr_st_nifty_dat,
                          int i_type,
                          char *ptr_c_serviceName,
                          char *ptr_c_errmsg)
{
	char *ptr_v_shm_addr;	
	struct st_nifty_data *ptr_st_tmp_nifty_dat;

	/*	attach shared memory	*/
	if( fn_attach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) 				== -1)
	{
		return -1;
	}
	
	ptr_st_tmp_nifty_dat=(struct st_nifty_data *)(ptr_v_shm_addr+NIFTY_DATA_STRT);
	if ( i_type == GET_NIFTY_DATA )
	{
		memcpy(ptr_st_nifty_dat, ptr_st_tmp_nifty_dat, NIFTY_DATA_SIZE);
	}
	else
	{
		memcpy(ptr_st_tmp_nifty_dat, ptr_st_nifty_dat, NIFTY_DATA_SIZE);
	} 

	/*	detach shared memory	*/
	if( fn_detach_shm((void *)&ptr_v_shm_addr, 
										ptr_c_serviceName, 
										ptr_c_errmsg) == -1)
	{
		return -1;
	}
	return 0;
}
