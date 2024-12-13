
/*  Program       : cln_timer_signal.c                                        */
/*                                                                            */
/*  Description   : This utility sends the signal to process                  */
/*                                                                            */
/*  Input         : <list of pipe IDs>																				*/
/*                                                                            */
/*  Output        : Success/Failure                                           */
/*                                                                            */
/*  Log           :                                                           */
/*                                                                            */
/*  Ver 1.0   16-Oct-2013 Sachin Birje  Set the sinal to process for timer setting */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fn_env.h>
#include <fn_log.h>
#include <fn_msgq.h>
#include <string.h>
#include <signal.h>
#include <fo.h>
#include <fcntl.h>

#include <atmi.h>
#include <fcntl.h>
#include <errno.h>
#include <fo_shm.h>
#define CH_EXIT 5

void fn_err_dstry_liveq( int i_qlist[], int i_qcnt );
char c_ServiceName[33];

int fn_send_signal(char *c_prcs_nm,  char *c_qtype, long *li_pro_id,char *c_ServiceName, char *c_err_msg);


struct st_q_details *st_q_data;

int main( int argc, char *argv[] )
{
  char *ptr_c_tmp;
  char c_errmsg[256];
  char c_fileName[256];	

	int i_qid;
	int i_msgq_typ;
	int i_ch_val;
	int i_cnt;
	int i_alive_qid[10];
	int i_qcnt=0;
  int i_return_code=0;
  int i_choice;

  long l_tot_cnt;
  long l_timer;

  long li_pro_id;
  long l_current_throtal;
  long l_sleep_time;

  char c_filter [256];
  char c_xchng_cd [3+1];
  char c_prcs_nm[20];
  char c_pipe_id[4];
  char c_err_msg[256];
  char c_filename[256];
  char c_qtype[20];

  MEMSET(c_qtype);
	
	/* check for command line arguments  */
  if(argc < 2)
  {
    perror("Usage - cln_timer_signal <pipe id1>");
    exit(-1);
  }

	strcpy(c_ServiceName, argv[0]);

	fn_userlog( c_ServiceName, "Utility Started ");


  sprintf( c_filename, "%s/BRD.ini", getenv("BIN"));

  /* Create Log pipe */
  i_ch_val = fn_create_pipe();
  if( i_ch_val == -1 )
  {
    exit(-1);
  }

  i_ch_val = fn_init_prcs_spc( c_ServiceName,
                               c_filename,
                              "cln_timer_signal" );
  if( i_ch_val == -1 )
  {
    fn_errlog(c_ServiceName,"S31005", LIBMSG, c_errmsg);
    fn_destroy_pipe();
    exit(0);
  }

  l_tot_cnt = (sizeof(st_q_data) * 5 );

  i_return_code = fn_get_shm_id(CLIENT,c_ServiceName,l_tot_cnt,c_errmsg);

  if( i_return_code == -1 )
  {
   fn_errlog( c_ServiceName, "S31010","", c_errmsg );
   exit(-1);
 
  }
  else if( i_return_code == 1)
  {
    fn_userlog(c_ServiceName,"c_errmsg :%s:",c_errmsg);
    fn_errlog( c_ServiceName, "S31015","", c_errmsg );
    i_return_code = fn_get_shm_id(OWNER,c_ServiceName,l_tot_cnt,c_errmsg);

    if( i_return_code == -1)
    {
     fn_errlog( c_ServiceName, "S31020","", c_errmsg );
     exit(-1); 
    }    
  }

  fn_userlog(c_ServiceName," before attach");
  i_return_code = fn_attach_shm((void *)&st_q_data,c_ServiceName,c_errmsg);
  if( i_return_code == -1)
  {
   fn_errlog( c_ServiceName, "S31025","", c_errmsg );
   exit(-1); 
  }
 
   st_q_data[0].i_qid = 4;

   printf ( "\n\n\t Throtal Time Setting \n\n");
   printf ( "1. FUTIDX_QUEUE \n" ); 
   printf ( "2. FUTSTK_QUEUE \n" ); 
   printf ( "3. OPTIDX_QUEUE \n" ); 
   printf ( "4. OPTSTK_QUEUE \n" ); 
   printf ( "5. Exit         \n" ); 
   printf ( "\nEnter the Choice :" ); 
   scanf  ( "%d",&i_choice);
   switch(i_choice)
   {  
    case 1 :

            ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"FUTIDX_QUEUE");
            strcpy(c_qtype,"FUTIDX_QUEUE");
            i_qid = atoi(ptr_c_tmp); 
            st_q_data[i_choice].i_qid  = i_qid;
            
            ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"FUTIDX_TIMER");
            l_current_throtal = atol(ptr_c_tmp);
            printf ( "\nCurrent Throtal Value (in Micro Sec) :%ld",l_current_throtal);

            printf ( "\nEnter FUTIDX_QUEUE Throtal (in Micro Sec) :");
            scanf  ( "\n%ld", &l_timer);
            st_q_data[i_choice].l_timer = l_timer;
            printf ( "\nEnter FUTIDX_QUEUE Sleep (in Micro Sec) :");
            scanf  ( "\n%ld", &l_sleep_time);
            st_q_data[i_choice].l_sleep_time= l_sleep_time;
            break;
    case 2 :
            ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"FUTSTK_QUEUE");
            strcpy(c_qtype,"FUTSTK_QUEUE");
            i_qid = atoi(ptr_c_tmp);
            st_q_data[i_choice].i_qid =i_qid;

            ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"FUTSTK_TIMER");
            l_current_throtal = atol(ptr_c_tmp);
            printf ( "\nCurrent Throtal Value (in Micro Sec) :%ld",l_current_throtal);
 
            printf ( "\nEnter FUTSTK_QUEUE Throtal (in Micro Sec) :");
            scanf  ( "\n%ld", &l_timer);
            st_q_data[i_choice].l_timer = l_timer;
            printf ( "\nEnter FUTSTK_QUEUE Sleep (in Micro Sec) :");
            scanf  ( "\n%ld", &l_sleep_time);
            st_q_data[i_choice].l_sleep_time= l_sleep_time;
            break;
    case 3 :
            ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OPTIDX_QUEUE");
            strcpy(c_qtype,"OPTIDX_QUEUE");
            i_qid = atoi(ptr_c_tmp);
            st_q_data[i_choice].i_qid = i_qid;
 
            ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OPTIDX_TIMER");
            l_current_throtal = atol(ptr_c_tmp);
            printf ( "\nCurrent Throtal Value (in Micro Sec) :%ld",l_current_throtal);
 
            printf ( "\nEnter FUTSTK_QUEUE Throtal (in Micro Sec) :");
            scanf  ( "\n%ld", &l_timer);
            st_q_data[i_choice].l_timer = l_timer;
            printf ( "\nEnter FUTSTK_QUEUE Sleep (in Micro Sec) :");
            scanf  ( "\n%ld", &l_sleep_time);
            st_q_data[i_choice].l_sleep_time= l_sleep_time;

            break;
    case 4 :
            ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OPTSTK_QUEUE");
            strcpy(c_qtype,"OPTSTK_QUEUE");
            i_qid = atoi(ptr_c_tmp);
            st_q_data[i_choice].i_qid = i_qid;
  
            ptr_c_tmp = (char *)fn_get_prcs_spc(c_ServiceName,"OPTSTK_TIMER");
            l_current_throtal = atol(ptr_c_tmp);
            printf ( "\nCurrent Throtal Value (in Micro Sec) :%ld",l_current_throtal);
 
            printf ( "\nEnter OPTSTK_QUEUE Throtal (in Micro Sec) :");
            scanf  ( "\n%ld", &l_timer);
            st_q_data[i_choice].l_timer = l_timer;
            printf ( "\nEnter OPTSTK_QUEUE Sleep (in Micro Sec) :");
            scanf  ( "\n%ld", &l_sleep_time);
            st_q_data[i_choice].l_sleep_time= l_sleep_time;
            break; 
    case 5 :
            i_choice = CH_EXIT;
            exit(-1); 
            break;
    default :

            printf ( "\n Invalid Option Selected \n");
            exit(-1);
            break;
    }

    i_return_code = fn_detach_shm((void *)&st_q_data,c_ServiceName,c_errmsg);
    if( i_return_code == -1)
    {
     fn_errlog( c_ServiceName, "S31030","", c_errmsg );
     exit(-1); 
    }


   strcpy(c_xchng_cd,"NA");

   strcpy(c_prcs_nm,"cln_brdqt_prcs");

   i_ch_val = fn_send_signal(c_prcs_nm,
                             c_qtype,
                             &li_pro_id,
                             c_ServiceName,
                             c_err_msg);
   if( i_ch_val != 0 )
   {
    fn_userlog(c_ServiceName,"Process ID not found for cln_brdqt_prcs for Exchange code:%s:",c_qtype);
    exit(-1); 
   }

	 fn_userlog( c_ServiceName, "Utility Ended ");
}

int fn_send_signal(char *c_prcs_nm,  char *c_qtype, long *li_pro_id,char *c_ServiceName, char *c_err_msg)
{

  char  c_command[150];
  char  c_file_nm[100];
  char  c_user[20];
  char  c_process[20];
  char  c_exchange[13+1];
  FILE  *fptr;
  long   li_tmp_prc_id;

  union sigval  st_data;

  MEMSET(c_file_nm);
  MEMSET(c_command);

  strcpy(c_file_nm, getenv("STATUS_FILE"));

  fn_userlog(c_ServiceName,"IN the process get process id");
  fn_userlog(c_ServiceName,"process Name:%s:",c_prcs_nm);
  fn_userlog(c_ServiceName,"c_qtype :%s:",c_qtype);

  remove(c_file_nm);
  sprintf(c_command, "ps -efa -o pid,user,comm,args | grep -v 'grep' |grep $USER | grep %s | grep %s > %s",c_prcs_nm,c_qtype,c_file_nm);

  system(c_command);

  fn_userlog(c_ServiceName," c_file_nm :%s:",c_file_nm);
  fptr = (FILE *)fopen(c_file_nm, "r");
  if(fptr == NULL)
  {
    fn_errlog(c_ServiceName,"S31035", LIBMSG, c_err_msg);
    return -1;
  }

  MEMSET(c_process);

   fscanf(fptr, "%d %s",&li_tmp_prc_id,c_process);
   fn_userlog(c_ServiceName, "Process :%s: is running and process id is :%ld",c_process,li_tmp_prc_id);

   if ( li_tmp_prc_id != 0 )
   {
    st_data.sival_int = SIGUSR1;
    if(sigqueue(li_tmp_prc_id,SIGUSR1, st_data) == -1)
    {
      fn_errlog(c_ServiceName, "L31010", UNXMSG,c_err_msg);
      return -1;
    }
    fn_userlog(c_ServiceName, "Signal Sent to :%ld:",li_tmp_prc_id);
   }

  fclose(fptr);
  return 0;
}

