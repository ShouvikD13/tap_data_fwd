/********************************************************************************/
/*	File : ud_fobse_socket.c																										*/
/*  		 : Socket handling functions for BSE Broadcast from IML									*/
/* LOG	 :																																			*/
/*																																							*/
/* Ver 1.0 : 20-Dec-2012 Sandip Tambe																						*/ 	
/********************************************************************************/
	
#include <ud_fobse_socket.h>
#include <sys/poll.h>
#include <fn_mcast_log.h>
#include <fn_fobse_bcast.h>
int i_sig_tm_out_trp = 0;

long int li_utc_offset;
long int li_ten_yrs;

/******************************************************************************/
/* fn_siganl() - Function to handle signals                                   */
/******************************************************************************/
Sigfunc *fn_signal ( int     i_signo,
                     Sigfunc *ptr_fn_new_hnd )
{
  int i_ch_val;

  struct sigaction st_act;
  struct sigaction st_oact;

  st_act.sa_handler = ptr_fn_new_hnd;
  sigemptyset(&st_act.sa_mask);
  st_act.sa_flags = 0;
  if (i_signo == SIGALRM)
  {
    #ifdef SA_INTERRUPT
      st_act.sa_flags |= SA_INTERRUPT;
    #endif
  }
  else
  {
    #ifdef SA_RESTART
      st_act.sa_flags |= SA_RESTART;
    #endif
  }

  i_ch_val = sigaction ( i_signo,
                         &st_act,
                         &st_oact );
  if (i_ch_val < 0)
  {
    return (SIG_ERR);
  }

  return (st_oact.sa_handler);
}
/******************************************************************************/
/* fn_alm_intr() - Alarm interrrupt                                           */
/******************************************************************************/
void fn_alm_intr(int i_signo)
{
  i_sig_tm_out_trp = 1;
  return;
}

/******************************************************************************/
/* fn_socket() - Creates a socket for the client at the given ip              */
/*                        address                                             */
/******************************************************************************/
int fn_socket ( int *ptr_i_fd,
                int i_family,
                int i_type,
                int i_protocol )
{
  int i_res;
  int i_ch_val;

  i_res = NOERROR;

  i_ch_val = socket ( i_family,
                      i_type,
                      i_protocol );
  if (i_ch_val == -1)
  {
    i_res = SCKERROR;
    return (i_res);
  }
  *ptr_i_fd = i_ch_val;

  return (i_res);
}

/******************************************************************************/
/* fn_bind() - Binds the socket to the given ip address and port.             */
/******************************************************************************/
int fn_bind ( int  i_fd,
              int  i_family,
              char *ptr_c_ip_addr,
              uint32_t ptr_ud_ip_addr,
              int  i_prt_no,
              int  i_bnd_flg )
{
  int i_res;
  int i_ch_val;

  struct sockaddr_in st_machn_addr;

  i_res = NOERROR;

  bzero(&st_machn_addr, sizeof(st_machn_addr));
  st_machn_addr.sin_family = i_family;
  if ( i_bnd_flg == CLIENT )
  {
    st_machn_addr.sin_addr.s_addr = inet_addr(ptr_c_ip_addr);
  }
  else
  {
    st_machn_addr.sin_addr.s_addr = htonl ( ptr_ud_ip_addr );
  }
  st_machn_addr.sin_port = htons(i_prt_no);

  i_ch_val = bind ( i_fd,
                    (SA *)&st_machn_addr,
                    (socklen_t)sizeof(st_machn_addr) );
  if (i_ch_val == -1)
  {
    i_res = BNDERROR;
  }

  return (i_res);
}
/******************************************************************************/
/* fn_close() - Closes a socket                                               */
/******************************************************************************/
int fn_close ( int i_fd )
{
  int i_res;
  int i_ch_val;

  i_res = NOERROR;

  i_ch_val = close(i_fd);
  if (i_ch_val == -1)
  {
    i_res = CLSERROR;
  }

  return (i_res);
}
/******************************************************************************/
/* fn_recv() - Receives N bytes from the designated port                      */
/******************************************************************************/
int fn_recv ( int  i_fd,
              char *ptr_c_stm,
              int  *ptr_i_sz,
              long int li_tm )
{
  int     i_res;
  int     i_read;
  Sigfunc *ptr_fn_old_hnd;

  i_res = NOERROR;
  ptr_fn_old_hnd = fn_signal(SIGALRM, fn_alm_intr);

  alarm(li_tm);
  if ((i_read = read(i_fd, ptr_c_stm, *ptr_i_sz)) < 0)
  {
    if (errno == EINTR)
    {
      if (i_sig_tm_out_trp == 1)
      {
        errno = ETIMEDOUT;
        i_sig_tm_out_trp = 0;
      }
      i_res = RCVERROR;
      *ptr_i_sz = i_read;
      alarm(TM_0);
      fn_signal(SIGALRM, ptr_fn_old_hnd);
      return (i_res);
    }
    else
    {
      i_res = RCVERROR;
      *ptr_i_sz = i_read;
      alarm(TM_0);
      fn_signal(SIGALRM, ptr_fn_old_hnd);
      return (i_res);
    }
  }
  else if (i_read == 0)
  {
    i_res = RCVERROR;
    *ptr_i_sz = i_read;
    alarm(TM_0);
    fn_signal(SIGALRM, ptr_fn_old_hnd);
    return (i_res);
  }
  *ptr_i_sz = i_read;
  alarm(TM_0);
  fn_signal(SIGALRM, ptr_fn_old_hnd);
  return (i_res);
}

/******************************************************************************/
/* fn_recvfrom() - Receives a datagram packet                                 */
/******************************************************************************/
int fn_recvfrom ( int  i_fd,
                  char *ptr_c_stm,
                  int  *ptr_i_sz,
                  int  i_flags,
                  int  *i_family,
                  char *ptr_c_ip_addr,
                  int  *i_prt_no,
                  long int li_tm )
{
  int i_ch_val;
  int i_res;
  Sigfunc *ptr_fn_old_hnd;
  socklen_t ud_addr_len;
  struct sockaddr_in st_machn_addr;

  i_res = NOERROR;
  bzero(&st_machn_addr, sizeof(st_machn_addr));
  ptr_fn_old_hnd = fn_signal(SIGALRM, fn_alm_intr);
  ud_addr_len = (socklen_t)sizeof(st_machn_addr);

  alarm(li_tm);
  i_ch_val = recvfrom ( i_fd,
                        ptr_c_stm,
                        *ptr_i_sz,
                        i_flags,
                        (SA *)&st_machn_addr,
                        &ud_addr_len );
  if (i_ch_val == -1)
  {
    if (errno == EINTR)
    {
      i_res = RCVERROR;
      if (i_sig_tm_out_trp == 1)
      {
        errno = ETIMEDOUT;
        i_sig_tm_out_trp = 0;
      }
      *ptr_i_sz = 0;
      alarm(TM_0);
      fn_signal(SIGALRM, ptr_fn_old_hnd);
      return (i_res);
    }
  }

  *ptr_i_sz = i_ch_val;
  alarm(TM_0);
  fn_signal(SIGALRM, ptr_fn_old_hnd);

  *i_family = st_machn_addr.sin_family;
  ptr_c_ip_addr = inet_ntoa(st_machn_addr.sin_addr);
  *i_prt_no = ntohs(st_machn_addr.sin_port);

  return (i_res);
}

void fn_sck_init(void)
{
  struct tm tenyrs_tm;

  tenyrs_tm.tm_sec = 0;
  tenyrs_tm.tm_min = 0;
  tenyrs_tm.tm_hour = 0;
  tenyrs_tm.tm_mday = 1;
  tenyrs_tm.tm_mon = 0;
  tenyrs_tm.tm_year = 70;
  tenyrs_tm.tm_wday = 0;
  tenyrs_tm.tm_yday = 0;
  tenyrs_tm.tm_isdst = 0;

  li_utc_offset = mktime(&tenyrs_tm) * (-1);

  tenyrs_tm.tm_sec = 0;
  tenyrs_tm.tm_min = 0;
  tenyrs_tm.tm_hour = 0;
  tenyrs_tm.tm_mday = 1;
  tenyrs_tm.tm_mon = 0;
  tenyrs_tm.tm_year = 80;
  tenyrs_tm.tm_wday = 0;
  tenyrs_tm.tm_yday = 0;
  tenyrs_tm.tm_isdst = 0;

  li_ten_yrs = mktime(&tenyrs_tm) + li_utc_offset;
}

