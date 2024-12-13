/**************************************************************************
* Ver 1.1 - 	MG (27-Nov-2002)																						*
*							Changed in function fn_call_svc to pass back error message	*
*							in case of error INSUFFICIENT_LIMITS												*
*							Note - Only relevant servers have been recompiled - not all	*
* Ver 1.2     Messaging for transaction commit														*
							Sridhar/Sangeet (05-Feb-2003)																*
* Ver 1.3 		IBM/AIX	changes 																						*
* Ver 1.4     Implementation of new functions for begin and commit        *
*             transaction with no log printing (24-Feb-2014)(Sachin Birje)*
* Ver 1.5     New function to unpack FML to Variable (04-Sep-2014) | Sachin Birje*
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <atmi.h>
#include <fml32.h>
#include <fo.h>
#include <fn_log.h>
#include <fo_view_def.h>
#include <fo_fml_def.h>
#include <fml_def.h>
#include <signal.h>
#include <limits.h>	 /* VER TOL : TUX on LINUX -- Chnaged sys/limits.h to limits.h */
#include <userlog.h> /* Ver 1.3 */

#include <string.h>	  /* Ver 1.3 */
#include <fml_rout.h> /* ver 1.3 32 bit */

long fn_call_svc(char *c_ServiceName,
				 char *c_err_msg,
				 void *st_input,
				 void *st_output,
				 char *c_inp_view,
				 char *c_out_view,
				 /*int  i_ip_len, */ /* Ver 1.3 */
				 long l_ip_len,
				 /*int  i_op_len, */ /* Ver 1.3 */
				 long l_op_len,
				 long int li_flg,
				 char *c_svcnm); /* Ver 1.3 64 bit portability : return type changed from int to long */

int fn_acall_svc(char *c_ServiceName,
				 char *c_err_msg,
				 void *st_input,
				 char *c_inp_view,
				 int i_ip_len,
				 long int li_flg,
				 char *c_svcnm);

long fn_agetrep_svc(char *c_ServiceName,
					char *c_err_msg,
					void *st_output,
					char *c_out_view,
					int i_op_len,
					long int li_flg,
					int *ptr_i_cd); /* Ver 1.3 Return type changed to long */

long fn_call_svc_fml(char *c_ServiceName,
					 char *c_err_msg,
					 char *c_svc_nam,
					 long int li_flag,
					 int i_tot_ifields,
					 int i_tot_ofields,
					 ...); /* ver 1.3 64 bit portability Return type changed from int  to long */

int fn_acall_svc_fml(char *c_ServiceName,
					 char *c_err_msg,
					 char *c_svc_nam,
					 long int li_flag,
					 int i_tot_ifields,
					 ...);
long fn_agetrep_svc_fml(char *c_ServiceName,
						char *c_err_msg,
						int *ptr_i_cd,
						long int li_flag,
						int i_tot_ofields,
						...); /* Ver 1.3 return Type  changed int to long */

int fn_unpack_fmltostruct(char *c_ServiceName,
						  char *c_err_msg,
						  FBFR32 *ptr_fml_ibuf,
						  void *st_input,
						  char *c_inp_view);

int fn_unpack_fmltovar(char *c_ServiceName,
					   char *c_err_msg,
					   FBFR32 *ptr_fml_ibuf,
					   int i_tot_ifields,
					   ...);

/*** Ver 1.5 Starts Here ***/

int fn_unpack_fmltovar_dflt(char *c_ServiceName,
							char *c_err_msg,
							FBFR32 *ptr_fml_ibuf,
							int i_tot_ifields,
							...);
/*** Ver 1.5 Ends Here ***/

int fn_pack_structtofml(char *c_ServiceName,
						char *c_err_msg,
						FBFR32 **ptr_fml_obuf,
						void *st_output,
						char *c_out_view,
						int i_flg);

int fn_pack_vartofml(char *c_ServiceName,
					 char *c_err_msg,
					 FBFR32 **ptr_fml_obuf,
					 int i_tot_ofields,
					 ...);

FBFR32 *fn_create_rs(char *c_ServiceName,
					 char *c_err_msg,
					 char *c_svc_nam,
					 long int li_flag,
					 int i_tot_ifields,
					 ...);

void fn_rewind_rs(FBFR32 *ptr_fml_buf);

int fn_reccnt_rs(FBFR32 *ptr_fml_buf,
				 char *c_ServiceName,
				 char *c_err_msg,
				 FLDID32 l_cnt_fml);

int fn_getnxt_rs(FBFR32 *ptr_fml_buf,
				 char *c_ServiceName,
				 char *c_err_msg,
				 int i_tot_ifields,
				 ...);

void fn_dstry_rs(FBFR32 *ptr_fml_buf);

void fn_unsol_msg_hnd(char *ptr_c_i_trg,
					  long int li_len,
					  long int li_flg);

void fn_set_msghnd(void);

void fn_pst_trg(char *c_servicename,
				char *c_trg_name,
				char *c_msg,
				char *filter);

int fn_chk_trg(void);

/*long int li_rs_reccnt; */ /* Ver 1.3 */
int i_rs_reccnt;
char c_trg_msg[256];
int i_trg_got;

#define LOCAL_TRNSCTN 1
#define REMOTE_TRNSCTN 2
#define TRAN_TIMEOUT 300

int fn_begintran(char *c_servicename,
				 char *c_err_msg);

int fn_nolog_begintran(char *c_servicename,
					   char *c_err_msg); /*** Ver 1.4 ***/

int fn_committran(char *c_servicename,
				  int i_trnsctn,
				  char *c_err_msg);

int fn_nolog_committran(char *c_servicename,
						int i_trnsctn,
						char *c_err_msg); /*** Ver 1.4 ***/

int fn_aborttran(char *c_servicename,
				 int i_trnsctn,
				 char *c_err_msg);

#define SYSTEM_ERROR -100

long fn_call_svc(char *c_ServiceName,
				 char *c_err_msg,
				 void *st_input,
				 void *st_output,
				 char *c_inp_view,
				 char *c_out_view,
				 /*int  i_ip_len,*/ /* Ver 1.3 */
				 long l_ip_len,
				 /*int  i_op_len, */ /* Ver 1.3 */
				 long l_op_len,
				 long int li_flg,
				 char *c_svcnm) /* Ver 1.3 64 bit portability : return type int changed to long */
{
	/*int  i_returncode;*/
	long l_returncode; /* Ver 1.3 64 bit portability */
	long li_recvbuf;
	struct vw_err_msg st_err;
	void *ptr_view_Ibuf;
	void *ptr_view_Obuf;

	ptr_view_Ibuf = (char *)tpalloc("VIEW32",
									c_inp_view,
									l_ip_len); /* ver 1.3 i_ip_len changed to l_ip_len  */
	if (ptr_view_Ibuf == NULL)
	{
		fn_errlog(c_ServiceName, "L31005", TPMSG, c_err_msg);
		return (SYSTEM_ERROR);
	}

	ptr_view_Obuf = (char *)tpalloc("VIEW32",
									c_out_view,
									l_op_len); /* ver 1.3 i_op_len changed to l_op_len  */
	if (ptr_view_Obuf == NULL)
	{
		fn_errlog(c_ServiceName, "L31010", TPMSG, c_err_msg);
		tpfree((char *)ptr_view_Ibuf);
		return (SYSTEM_ERROR);
	}

	memcpy((char *)ptr_view_Ibuf,
		   (char *)st_input,
		   l_ip_len); /* ver 1.3 */

	l_returncode = tpcall(c_svcnm,
						  (char *)ptr_view_Ibuf,
						  0,
						  (char **)&ptr_view_Obuf,
						  &li_recvbuf,
						  li_flg);
	if (l_returncode == -1)
	{
		if (TPCODE == TPESVCFAIL)
		{
			l_returncode = tpurcode;
			if (tpurcode == NO_BFR)
			{
				strcpy(c_err_msg, "Resource not available");
			}
			else if (tpurcode == ERR_BFR)
			{
				memcpy((char *)&st_err,
					   (char *)ptr_view_Obuf,
					   sizeof(st_err));
				strcpy(c_err_msg, st_err.c_err_msg);
			}
			/* 1.1  - code added - start */
			else if (tpurcode == INSUFFICIENT_LIMITS)
			{
				memcpy((char *)&st_err,
					   (char *)ptr_view_Obuf,
					   sizeof(st_err));
				strcpy(c_err_msg, st_err.c_err_msg);
			}
			/* 1.1 - end */
			else
			{
				memcpy((char *)st_output,
					   (char *)ptr_view_Obuf,
					   l_op_len); /* Ver 1.3 */
			}
		}
		else
		{
			fn_errlog(c_ServiceName, "L31015", TPMSG, c_err_msg);
			/*	i_returncode = SYSTEM_ERROR;	*/
			l_returncode = SYSTEM_ERROR; /* Ver 1.3 64 bit portability */
		}
		tpfree((char *)ptr_view_Ibuf);
		tpfree((char *)ptr_view_Obuf);
		/*return ( i_returncode );*/
		return (l_returncode); /* Ver 1.3 64 bit portability */
	}

	/*	i_returncode = tpurcode;	*/
	l_returncode = tpurcode; /* Ver 1.3 64 bit portability */
	memcpy((char *)st_output,
		   (char *)ptr_view_Obuf,
		   l_op_len); /* Ver 1.3 */

	tpfree((char *)ptr_view_Ibuf);
	tpfree((char *)ptr_view_Obuf);

	/*return ( i_returncode );*/
	return (l_returncode); /* Ver 1.3 64 bit portability */
}

int fn_acall_svc(char *c_ServiceName,
				 char *c_err_msg,
				 void *st_input,
				 char *c_inp_view,
				 int i_ip_len,
				 long int li_flg,
				 char *c_svcnm)
{
	int i_returncode;
	struct vw_err_msg st_err;
	void *ptr_view_Ibuf;
	unsigned int ul_ip_len; /* Ver 1.3 */

	ptr_view_Ibuf = (char *)tpalloc("VIEW32",
									c_inp_view,
									i_ip_len);
	ul_ip_len = i_ip_len; /* Ver 1.3 */

	if (ptr_view_Ibuf == NULL)
	{
		fn_errlog(c_ServiceName, "L31020", TPMSG, c_err_msg);
		return (SYSTEM_ERROR);
	}

	memcpy((char *)ptr_view_Ibuf,
		   (char *)st_input,
		   /*i_ip_len */ ul_ip_len); /* Ver 1.3 */

	i_returncode = tpacall(c_svcnm,
						   (char *)ptr_view_Ibuf,
						   0,
						   li_flg);
	if (i_returncode == -1)
	{
		fn_errlog(c_ServiceName, "L31025", TPMSG, c_err_msg);
		tpfree((char *)ptr_view_Ibuf);
		i_returncode = SYSTEM_ERROR;
		return (i_returncode);
	}

	tpfree((char *)ptr_view_Ibuf);

	return (i_returncode);
}

long fn_agetrep_svc(char *c_ServiceName,
					char *c_err_msg,
					void *st_output,
					char *c_out_view,
					int i_op_len,
					long int li_flg,
					int *ptr_i_cd) /* Ver 1.3 return type changed to long */
{
	/* int  i_returncode; */
	long l_returncode; /* Ver 1.3 64 bit portability */
	long li_recvbuf;
	struct vw_err_msg st_err;
	void *ptr_view_Obuf;
	unsigned int ui_op_len; /* Ver 1.3 */

	ptr_view_Obuf = (char *)tpalloc("VIEW32",
									c_out_view,
									i_op_len);
	ui_op_len = i_op_len; /* Ver 1.3 */
	if (ptr_view_Obuf == NULL)
	{
		fn_errlog(c_ServiceName, "L31030", TPMSG, c_err_msg);
		return (SYSTEM_ERROR);
	}

	l_returncode = tpgetrply(ptr_i_cd,
							 (char **)&ptr_view_Obuf,
							 &li_recvbuf,
							 li_flg); /* Ver 1.3 Type changed from int to long  */
	if (l_returncode == -1)			  /* Ver 1.3 Type changed  from int to long */
	{
		if (TPCODE == TPESVCFAIL)
		{
			l_returncode = tpurcode; /* Ver 1.3 Type changed from int to long */
			if (tpurcode == NO_BFR)
			{
				strcpy(c_err_msg, "Resource not available");
			}
			else if (tpurcode == ERR_BFR)
			{
				memcpy((char *)&st_err,
					   (char *)ptr_view_Obuf,
					   sizeof(st_err));
				strcpy(c_err_msg, st_err.c_err_msg);
			}
			else
			{
				memcpy((char *)st_output,
					   (char *)ptr_view_Obuf,
					   /*i_op_len*/ ui_op_len); /* Ver 1.3 */
			}
		}
		else
		{
			fn_errlog(c_ServiceName, "L31035", TPMSG, c_err_msg);
			l_returncode = SYSTEM_ERROR; /*  Ver 1.3 Type changed from int to long */
		}
		tpfree((char *)ptr_view_Obuf);
		return (l_returncode); /* Ver 1.3 Type changed from int to long */
	}

	l_returncode = tpurcode; /* Ver 1.3 Type changed from int to long */
	memcpy((char *)st_output,
		   (char *)ptr_view_Obuf,
		   /*i_op_len*/ ui_op_len); /* Ver 1.3 */

	tpfree((char *)ptr_view_Obuf);

	return (l_returncode); /* Ver 1.3 Type changed from int to long */
}

long fn_call_svc_fml(char *c_ServiceName,
					 char *c_err_msg,
					 char *c_svc_nam,
					 long int li_flag,
					 int i_tot_ifields,
					 int i_tot_ofields,
					 ...) /* Ver 1.3 return Type changed from int to long */
{
	va_list c_arg_lst;
	int i_cnt;
	int i_ch_val;
	/*	unsigned long l_ip_fml;  */

	unsigned int i_ip_fml; /* Ver 1.3 64 bit portability Type changed from unsigned long to unsigned int */
	char *c_ip_dat;
	/* unsigned long l_op_fml;	*/

	unsigned int i_op_fml; /* Ver 1.3 64 bit portability Type changed from unsigned long to unsigned int */
	char *c_op_dat;
	FBFR32 *ptr_fml_tibuf;
	FBFR32 *ptr_fml_tobuf;
	long int li_len_tobuf;
	/*int i_ret_val; */
	long l_ret_val; /* Ver 1.3 64 bit Portability */
	int *i_ind;

	va_start(c_arg_lst, i_tot_ofields);

	/**** Allocat input and output buffer ****/
	ptr_fml_tibuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);
	if (ptr_fml_tibuf == NULL)
	{
		fn_errlog(c_ServiceName, "L31040", TPMSG, c_err_msg);
		return SYSTEM_ERROR;
	}

	ptr_fml_tobuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);
	if (ptr_fml_tobuf == NULL)
	{
		fn_errlog(c_ServiceName, "L31045", TPMSG, c_err_msg);
		tpfree((char *)ptr_fml_tibuf);
		return SYSTEM_ERROR;
	}

	for (i_cnt = 1; i_cnt <= i_tot_ifields; i_cnt++)
	{
		/*l_ip_fml = va_arg ( c_arg_lst, unsigned long );*/
		i_ip_fml = va_arg(c_arg_lst, unsigned int); /* Ver 1.3 64-bit Portability second parameter  */
													/* Type changed from unsigned long to unsigned int .Variable l_ip_fml changed to i_ip_fml */
		c_ip_dat = va_arg(c_arg_lst, char *);
		i_ch_val = Fadd32(ptr_fml_tibuf, i_ip_fml, (char *)c_ip_dat, 0); /* Ver 1.3  */
																		 /* 2nd parameter l_ip_fml changed to i_ip_fml */
		if (i_ch_val == -1)
		{
			fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
			fn_errlog(c_ServiceName, "L31050", FMLMSG, c_err_msg);
			tpfree((char *)ptr_fml_tibuf);
			tpfree((char *)ptr_fml_tobuf);
			return SYSTEM_ERROR;
		}
	}

	i_ch_val = tpcall(c_svc_nam, (char *)ptr_fml_tibuf, 0,
					  (char **)&ptr_fml_tobuf, &li_len_tobuf, li_flag);
	if (i_ch_val == -1)
	{
		if (tperrno != TPESVCFAIL)
		{
			fn_errlog(c_ServiceName, "L31055", TPMSG, c_err_msg);
			/*	i_ret_val = SYSTEM_ERROR;	*/
			l_ret_val = SYSTEM_ERROR; /* Ver 1.3 64 bit portability */
		}
		else
		{
			/* i_ret_val = tpurcode;	*/
			l_ret_val = tpurcode; /* Ver 1.3 64 bit portability */
			Fget32(ptr_fml_tobuf, FFO_ERR_MSG, 0, (char *)c_err_msg, 0);
			Fget32(ptr_fml_tobuf, FML_ERR_MSG, 0, (char *)c_err_msg, 0);
		}
		tpfree((char *)ptr_fml_tibuf);
		tpfree((char *)ptr_fml_tobuf);
		/*	return (i_ret_val); */
		return (l_ret_val); /* Ver 1.3 64 bit portability */
	}

	/* i_ret_val = tpurcode; */
	l_ret_val = tpurcode; /* Ver 1.3 64 bit portability */
	for (i_cnt = 1; i_cnt <= i_tot_ofields; i_cnt++)
	{
		/*	l_op_fml = va_arg ( c_arg_lst, unsigned long );*/
		i_op_fml = va_arg(c_arg_lst, unsigned int); /* Ver 1.3 64 bit portability */
													/* 2nd parameter Type  changed from unsigned long to unsigned int */
													/* variable l_op_fml changed to i_op_fml */
		c_op_dat = va_arg(c_arg_lst, char *);
		i_ind = va_arg(c_arg_lst, int *);
		i_ch_val = Fget32(ptr_fml_tobuf, i_op_fml, 0, (char *)c_op_dat, 0);
		/*2nd parameter  changed from l_op_fml changed to i_op_fml */
		if (i_ch_val == -1)
		{
			if ((i_ind != NULL) && (Ferror32 == FNOTPRES))
			{
				*i_ind = -1;
			}
			else
			{
				fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
				fn_errlog(c_ServiceName, "L31060", FMLMSG, c_err_msg);
				tpfree((char *)ptr_fml_tibuf);
				tpfree((char *)ptr_fml_tobuf);
				return SYSTEM_ERROR;
			}
		}
		else
		{
			if (i_ind != NULL)
			{
				*i_ind = 0;
			}
		}
	}

	tpfree((char *)ptr_fml_tibuf);
	tpfree((char *)ptr_fml_tobuf);

	/*return ( i_ret_val );	*/
	return (l_ret_val); /* Ver 1.3 64 bit portability */
}

int fn_acall_svc_fml(char *c_ServiceName,
					 char *c_err_msg,
					 char *c_svc_nam,
					 long int li_flag,
					 int i_tot_ifields,
					 ...)
{
	va_list c_arg_lst;
	int i_cnt;
	int i_ch_val;
	/* unsigned long l_ip_fml; */
	unsigned int i_ip_fml; /* Ver 1.3 64 bit portability */
	char *c_ip_dat;
	FBFR32 *ptr_fml_tibuf;
	int i_ret_val;

	va_start(c_arg_lst, i_tot_ifields);

	/**** Allocat input and output buffer ****/
	ptr_fml_tibuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);
	if (ptr_fml_tibuf == NULL)
	{
		fn_errlog(c_ServiceName, "L31065", TPMSG, c_err_msg);
		return SYSTEM_ERROR;
	}

	for (i_cnt = 1; i_cnt <= i_tot_ifields; i_cnt++)
	{
		i_ip_fml = va_arg(c_arg_lst, unsigned int); /* Ver  1.3 64 bit portability l_op_fml changed to i_op_fml */
													/* ver 1.3 2nd parameter type unsigned long changed to unsigned int */
		c_ip_dat = va_arg(c_arg_lst, char *);
		i_ch_val = Fadd32(ptr_fml_tibuf, i_ip_fml, (char *)c_ip_dat, 0);
		/*  ver 1.3 2nd parameter l_op_fml changed to i_op_fml */
		if (i_ch_val == -1)
		{
			fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
			fn_errlog(c_ServiceName, "L31070", FMLMSG, c_err_msg);
			tpfree((char *)ptr_fml_tibuf);
			return SYSTEM_ERROR;
		}
	}

	i_ch_val = tpacall(c_svc_nam, (char *)ptr_fml_tibuf, 0, li_flag);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "L31075", TPMSG, c_err_msg);
		tpfree((char *)ptr_fml_tibuf);
		return SYSTEM_ERROR;
	}

	i_ret_val = 0;

	tpfree((char *)ptr_fml_tibuf);

	return (i_ret_val);
}

long fn_agetrep_svc_fml(char *c_ServiceName,
						char *c_err_msg,
						int *ptr_i_cd,
						long int li_flag,
						int i_tot_ofields,
						...) /* return Type changed  int to  long */

{
	va_list c_arg_lst;
	int i_cnt;
	int i_ch_val;
	/*	unsigned long l_op_fml; */
	unsigned int i_op_fml; /* Ver 1.3 64 bit portability */
	char *c_op_dat;
	FBFR32 *ptr_fml_tobuf;
	long int li_len_tobuf;
	/* int i_ret_val;  */
	long l_ret_val; /*Ver 1.3 Type changed int to long */
	int *i_ind;

	va_start(c_arg_lst, i_tot_ofields);

	/**** Allocat input and output buffer ****/
	ptr_fml_tobuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);
	if (ptr_fml_tobuf == NULL)
	{
		fn_errlog(c_ServiceName, "L31080", TPMSG, c_err_msg);
		return SYSTEM_ERROR;
	}

	i_ch_val = tpgetrply(ptr_i_cd, (char **)&ptr_fml_tobuf, &li_len_tobuf,
						 li_flag);
	if (i_ch_val == -1)
	{
		if (tperrno != TPESVCFAIL)
		{
			fn_errlog(c_ServiceName, "L31085", TPMSG, c_err_msg);
			l_ret_val = SYSTEM_ERROR; /* Ver 1.3 Type changed int to long */
		}
		else
		{
			l_ret_val = tpurcode; /* Ver 1.3 Type changed int to long */
			Fget32(ptr_fml_tobuf, FFO_ERR_MSG, 0, (char *)c_err_msg, 0);
			Fget32(ptr_fml_tobuf, FML_ERR_MSG, 0, (char *)c_err_msg, 0);
		}
		tpfree((char *)ptr_fml_tobuf);
		return (l_ret_val); /* Ver 1.3 Ver 1.3 Type changed int to long */
	}

	l_ret_val = tpurcode; /* Ver 1.3 Type changed int to long */

	for (i_cnt = 1; i_cnt <= i_tot_ofields; i_cnt++)
	{
		i_op_fml = va_arg(c_arg_lst, unsigned int); /* Ver 1.3 */
													/*2nd parameter Type changed from unsigned long to unsigned int */
		c_op_dat = va_arg(c_arg_lst, char *);
		i_ind = va_arg(c_arg_lst, int *);
		i_ch_val = Fget32(ptr_fml_tobuf, i_op_fml, 0, (char *)c_op_dat, 0);
		/* Ver 1.3 variable changed from l_op_fml to i_op_fml */
		if (i_ch_val == -1)
		{
			if ((i_ind != NULL) && (Ferror32 == FNOTPRES))
			{
				*i_ind = -1;
			}
			else
			{
				fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
				fn_errlog(c_ServiceName, "L31090", FMLMSG, c_err_msg);
				tpfree((char *)ptr_fml_tobuf);
				return SYSTEM_ERROR;
			}
		}
		else
		{
			if (i_ind != NULL)
			{
				*i_ind = 0;
			}
		}
	}

	tpfree((char *)ptr_fml_tobuf);

	return (l_ret_val); /* Ver 1.3 Ver 1.3 Type changed int to long */
}

int fn_unpack_fmltostruct(char *c_ServiceName,
						  char *c_err_msg,
						  FBFR32 *ptr_fml_ibuf,
						  void *st_input,
						  char *c_inp_view)
{
	int i_ch_val;

	i_ch_val = Fvftos32(ptr_fml_ibuf,
						(char *)st_input,
						c_inp_view);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "L31095", FMLMSG, c_err_msg);
		return SYSTEM_ERROR;
	}

	return 0;
}

int fn_unpack_fmltovar(char *c_ServiceName,
					   char *c_err_msg,
					   FBFR32 *ptr_fml_ibuf,
					   int i_tot_ifields,
					   ...)
{
	va_list c_arg_lst;
	int i_cnt;
	int i_ch_val;
	/*unsigned long l_ip_fml; */
	unsigned int i_ip_fml; /* Ver 1.3 64 bit portability  */
	char *c_ip_dat;
	int *i_ind;

	va_start(c_arg_lst, i_tot_ifields);

	for (i_cnt = 1; i_cnt <= i_tot_ifields; i_cnt++)
	{
		i_ip_fml = va_arg(c_arg_lst, unsigned int); /* Ver 1.3 2nd parameter Type unsigned long  changed to unsigned int */
		c_ip_dat = va_arg(c_arg_lst, char *);
		i_ind = va_arg(c_arg_lst, int *);
		i_ch_val = Fget32(ptr_fml_ibuf, i_ip_fml, 0, (char *)c_ip_dat, 0);
		/* Ver 1.3 2nd parameter l_ip_fml changed to i_ip_fml*/
		if (i_ch_val == -1)
		{
			if ((i_ind != NULL) && (Ferror32 == FNOTPRES))
			{
				*i_ind = -1;
			}
			else
			{
				fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
				fn_errlog(c_ServiceName, "L31100", FMLMSG, c_err_msg);
				return SYSTEM_ERROR;
			}
		}
		else
		{
			if (i_ind != NULL)
			{
				*i_ind = 0;
			}
		}
	}

	return 0;
}

/**** Ver 1.5 Starts Here ****/
int fn_unpack_fmltovar_dflt(char *c_ServiceName,
							char *c_err_msg,
							FBFR32 *ptr_fml_ibuf,
							int i_tot_ifields,
							...)
{
	va_list c_arg_lst;
	int i_cnt;
	int i_ch_val;
	/*unsigned long l_ip_fml; */
	unsigned int i_ip_fml;
	char *c_ip_dat;
	char *c_dflt_val;

	va_start(c_arg_lst, i_tot_ifields);

	for (i_cnt = 1; i_cnt <= i_tot_ifields; i_cnt++)
	{
		i_ip_fml = va_arg(c_arg_lst, unsigned int);
		c_ip_dat = va_arg(c_arg_lst, char *);
		c_dflt_val = va_arg(c_arg_lst, char *);

		i_ch_val = Fget32(ptr_fml_ibuf, i_ip_fml, 0, (char *)c_ip_dat, 0);

		if (i_ch_val == -1)
		{
			if ((strcmp(c_dflt_val, "NULL") != 0) && (Ferror32 == FNOTPRES))
			{
				strcpy(c_ip_dat, c_dflt_val);
			}
			else
			{
				fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
				fn_errlog(c_ServiceName, "L31105", FMLMSG, c_err_msg);
				return SYSTEM_ERROR;
			}
		}
	}

	return 0;
}

/**** Ver 1.5 Ends Here ****/

int fn_pack_structtofml(char *c_ServiceName,
						char *c_err_msg,
						FBFR32 **ptr_fml_obuf,
						void *st_output,
						char *c_out_view,
						int i_flg)
{
	int i_ch_val;
	static int i_buf_cnt;

	if (*ptr_fml_obuf == NULL)
	{
		*ptr_fml_obuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);
		if (*ptr_fml_obuf == NULL)
		{
			fn_errlog(c_ServiceName, "L31110", TPMSG, c_err_msg);
			return SYSTEM_ERROR;
		}
		i_buf_cnt = 1;
	}

	i_ch_val = Fvstof32(*ptr_fml_obuf,
						(char *)st_output,
						i_flg,
						c_out_view);
	if (i_ch_val == -1)
	{
		if (Ferror32 == FNOSPACE)
		{
			i_buf_cnt++;
			*ptr_fml_obuf = (FBFR32 *)tprealloc((char *)*ptr_fml_obuf,
												i_buf_cnt * MIN_FML_BUF_LEN);
			if (*ptr_fml_obuf == NULL)
			{
				fn_errlog(c_ServiceName, "L31115", TPMSG, c_err_msg);
				return SYSTEM_ERROR;
			}

			i_ch_val = Fvstof32(*ptr_fml_obuf,
								(char *)st_output,
								i_flg,
								c_out_view);
			if (i_ch_val == -1)
			{
				fn_errlog(c_ServiceName, "L31120", FMLMSG, c_err_msg);
				tpfree((char *)*ptr_fml_obuf);
				return SYSTEM_ERROR;
			}
		}
		else
		{
			fn_errlog(c_ServiceName, "L31125", FMLMSG, c_err_msg);
			tpfree((char *)*ptr_fml_obuf);
			return SYSTEM_ERROR;
		}
	}

	return 0;
}

int fn_pack_vartofml(char *c_ServiceName,
					 char *c_err_msg,
					 FBFR32 **ptr_fml_obuf,
					 int i_tot_ofields,
					 ...)
{
	va_list c_arg_lst;
	int i_cnt;
	int i_ch_val;
	/*unsigned long l_op_fml; */
	unsigned int i_op_fml; /* Ver 1.3 64 bit portability */
	char *c_op_dat;
	static int i_buf_cnt;

	if (*ptr_fml_obuf == NULL)
	{
		*ptr_fml_obuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);
		if (*ptr_fml_obuf == NULL)
		{
			fn_errlog(c_ServiceName, "L31130", TPMSG, c_err_msg);
			return SYSTEM_ERROR;
		}
		i_buf_cnt = 1;
	}

	va_start(c_arg_lst, i_tot_ofields);
	for (i_cnt = 1; i_cnt <= i_tot_ofields; i_cnt++)
	{
		i_op_fml = va_arg(c_arg_lst, unsigned int); /* ver 1.3 2nd parameter unsigned long changed to unsigned int */
													/* variable l_op_fml changed to i_op_fml */
		c_op_dat = va_arg(c_arg_lst, char *);
		i_ch_val = Fadd32(*ptr_fml_obuf, i_op_fml, (char *)c_op_dat, 0);
		/* ver 1.3 2nd parameter l_op_fml changed to i_op_fml */
		if (i_ch_val == -1)
		{
			if (Ferror32 == FNOSPACE)
			{
				i_buf_cnt++;
				*ptr_fml_obuf = (FBFR32 *)tprealloc((char *)*ptr_fml_obuf,
													i_buf_cnt * MIN_FML_BUF_LEN);
				if (*ptr_fml_obuf == NULL)
				{
					fn_errlog(c_ServiceName, "L31135", TPMSG, c_err_msg);
					return SYSTEM_ERROR;
				}

				i_ch_val = Fadd32(*ptr_fml_obuf, i_op_fml, (char *)c_op_dat, 0);
				/* ver 1.3 2nd parameter l_op_fml changed to i_op_fml */
				if (i_ch_val == -1)
				{
					fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
					fn_errlog(c_ServiceName, "L31140", FMLMSG, c_err_msg);
					tpfree((char *)*ptr_fml_obuf);
					return SYSTEM_ERROR;
				}
			}
			else
			{
				fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
				fn_errlog(c_ServiceName, "L31145", FMLMSG, c_err_msg);
				return SYSTEM_ERROR;
			}
		}
	}

	return 0;
}

FBFR32 *fn_create_rs(char *c_ServiceName,
					 char *c_err_msg,
					 char *c_svc_nam,
					 long int li_flag,
					 int i_tot_ifields,
					 ...)
{
	va_list c_arg_lst;
	int i_cnt;
	int i_ch_val;
	/*unsigned long l_ip_fml; */ /* Ver 1.3 */
	unsigned int i_ip_fml;
	char *c_ip_dat;
	FBFR32 *ptr_fml_tibuf;
	FBFR32 *ptr_fml_tobuf;
	long int li_len_tobuf;

	va_start(c_arg_lst, i_tot_ifields);

	/**** Allocat input and output buffer ****/
	ptr_fml_tibuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);
	if (ptr_fml_tibuf == NULL)
	{
		fn_errlog(c_ServiceName, "L31150", TPMSG, c_err_msg);
		return NULL;
	}

	ptr_fml_tobuf = (FBFR32 *)tpalloc("FML32", NULL, MIN_FML_BUF_LEN);
	if (ptr_fml_tobuf == NULL)
	{
		fn_errlog(c_ServiceName, "L31155", TPMSG, c_err_msg);
		tpfree((char *)ptr_fml_tibuf);
		return NULL;
	}

	for (i_cnt = 1; i_cnt <= i_tot_ifields; i_cnt++)
	{
		i_ip_fml = va_arg(c_arg_lst, unsigned int); /* ver 1.3 2nd parameter Type unsigned long to unsigned int */
													/* Variable l_ip_fml changed to i_ip_fml */
		c_ip_dat = va_arg(c_arg_lst, char *);
		i_ch_val = Fadd32(ptr_fml_tibuf, i_ip_fml, (char *)c_ip_dat, 0);
		/* Ver 1.3 2nd parameter l_op_fml changed to i_ip_fml */
		if (i_ch_val == -1)
		{
			fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
			fn_errlog(c_ServiceName, "L31160", FMLMSG, c_err_msg);
			tpfree((char *)ptr_fml_tibuf);
			tpfree((char *)ptr_fml_tobuf);
			return NULL;
		}
	}

	i_ch_val = tpcall(c_svc_nam, (char *)ptr_fml_tibuf, 0,
					  (char **)&ptr_fml_tobuf, &li_len_tobuf, li_flag);
	if (i_ch_val == -1)
	{
		if (tperrno != TPESVCFAIL)
		{
			fn_errlog(c_ServiceName, "L31165", TPMSG, c_err_msg);
		}
		else
		{
			Fget32(ptr_fml_tobuf, FFO_ERR_MSG, 0, (char *)c_err_msg, 0);
			Fget32(ptr_fml_tobuf, FML_ERR_MSG, 0, (char *)c_err_msg, 0);
		}
		tpfree((char *)ptr_fml_tibuf);
		tpfree((char *)ptr_fml_tobuf);
		return NULL;
	}

	tpfree((char *)ptr_fml_tibuf);

	i_rs_reccnt = 0; /* Ver 1.3 variable li_rs_reccnt changed to i_rs_reccnt */

	return ptr_fml_tobuf;
}

void fn_rewind_rs(FBFR32 *ptr_fml_buf)
{
	i_rs_reccnt = 0; /* Ver 1.3 variable li_rs_reccnt changed to i_rs_reccnt */
	return;
}

int fn_reccnt_rs(FBFR32 *ptr_fml_buf,
				 char *c_ServiceName,
				 char *c_err_msg,
				 FLDID32 l_cnt_fml)
{
	int i_ch_val;

	i_ch_val = Foccur32(ptr_fml_buf, l_cnt_fml);
	if (i_ch_val == -1)
	{
		fn_errlog(c_ServiceName, "L31170", FMLMSG, c_err_msg);
		return SYSTEM_ERROR;
	}

	return i_ch_val;
}

int fn_getnxt_rs(FBFR32 *ptr_fml_buf,
				 char *c_ServiceName,
				 char *c_err_msg,
				 int i_tot_ifields,
				 ...)
{
	va_list c_arg_lst;
	int i_cnt;
	int i_ch_val;
	/*unsigned long l_ip_fml; */ /* ver 1.3 */
	unsigned int i_ip_fml;
	char *c_ip_dat;
	int *i_ind;

	va_start(c_arg_lst, i_tot_ifields);

	for (i_cnt = 1; i_cnt <= i_tot_ifields; i_cnt++)
	{
		i_ip_fml = va_arg(c_arg_lst, unsigned int);
		/* Ver 1.3 2nd parameter Type unsigned long changed to unsigned int  */
		/* variable l_ip_fml changed to i_ip_fml */
		c_ip_dat = va_arg(c_arg_lst, char *);
		i_ind = va_arg(c_arg_lst, int *);
		i_ch_val = Fget32(ptr_fml_buf, i_ip_fml, i_rs_reccnt, (char *)c_ip_dat, 0);
		/* Ver 1.3 variable li_rs_reccnt changed to i_rs_reccnt */
		/* Ver 1.3 2nd parameter l_ip_fml changed to i_ip_fml */
		if (i_ch_val == -1)
		{
			if ((i_ind != NULL) && (Ferror32 == FNOTPRES))
			{
				*i_ind = -1;
			}
			else if (Ferror32 == FNOTPRES)
			{
				return NO_DATA_FOUND;
			}
			else
			{
				fn_userlog(c_ServiceName, "Error in field :%d:", i_cnt);
				fn_errlog(c_ServiceName, "L31175", FMLMSG, c_err_msg);
				return SYSTEM_ERROR;
			}
		}
		else
		{
			if (i_ind != NULL)
			{
				*i_ind = 0;
			}
		}
	}

	i_rs_reccnt++; /* ver 1.3 variable li_i_rs_reccnt changed to i_rs_reccnt */
	return 0;
}

void fn_dstry_rs(FBFR32 *ptr_fml_buf)
{
	tpfree((char *)ptr_fml_buf);
	return;
}

void fn_unsol_msg_hnd(char *ptr_c_i_trg,
					  long int li_len,
					  long int li_flg)
{
	int i_ch_val;

	i_ch_val = Fget32((FBFR32 *)ptr_c_i_trg,
					  FFO_TRG_DAT,
					  0,
					  (char *)c_trg_msg,
					  0);
	if (i_ch_val == -1)
	{
		userlog("Fget error |%s|", FMLMSG);
		return;
	}

	fn_userlog("UNSOL_HND", "Trigger got is |%s|", c_trg_msg);
	i_trg_got = 1;
	return;
}

void fn_set_msghnd(void)
{
	tpsetunsol(fn_unsol_msg_hnd);
	return;
}

void fn_pst_trg(char *c_servicename,
				char *c_trg_name,
				char *c_msg,
				char *filter)
{
	FBFR32 *ptr_fml_msg;
	char c_retmsg[256];
	char c_tmp_msg[256];
	int i_err[2];
	int i;

	ptr_fml_msg = (FBFR32 *)tpalloc("FML32",
									NULL,
									MIN_FML_BUF_LEN);
	if (ptr_fml_msg == NULL)
	{
		fn_errlog(c_servicename, "L31180", TPMSG, c_retmsg);
		return;
	}

	i_err[0] = Fadd32(ptr_fml_msg, FFO_TRG_DAT, (char *)c_msg, 0);
	i_err[1] = Fadd32(ptr_fml_msg, FFO_FILTER, (char *)filter, 0);

	for (i = 0; i < 2; i++)
	{
		if (i_err[i] == -1)
		{
			sprintf(c_tmp_msg, "Error in field %d", i);
			fn_userlog(c_servicename, c_tmp_msg);
			fn_errlog(c_servicename, "L31185", TPMSG, c_retmsg);
			return;
		}
	}
	fn_userlog(c_servicename, "Posting trigger |%s|", c_trg_name);
	/* fn_userlog ( c_servicename, "Message |%s|", c_msg );*/
	fn_userlog(c_servicename, "Filter  |%s|", filter);

	i_err[0] = tppost(c_trg_name, (char *)ptr_fml_msg, 0, TPNOTRAN);
	if (i_err[0] == -1)
	{
		fn_errlog(c_servicename, "L31190", TPMSG, c_retmsg);
	}

	tpfree((char *)ptr_fml_msg);
	return;
}

int fn_chk_trg(void)
{
	if (i_trg_got == 1)
	{
		i_trg_got = 0;
		return 1;
	}
	else
	{
		tpchkunsol();
		if (i_trg_got == 1)
		{
			i_trg_got = 0;
			return 1;
		}
	}

	return 0;
}

void fn_bat_pst_msg_fno(char *c_pgm_nm,
						char *c_msg,
						char *c_tag)
{
	int i_ch_val;
	char c_retmsg[256];

	fn_userlog(c_pgm_nm, "%s - %s", c_tag, c_msg);

	i_ch_val = fn_acall_svc_fml(c_pgm_nm,
								c_retmsg,
								"SVC_BAT_MSG",
								TPNOTRAN | TPNOREPLY,
								2,
								FML_EVENT, (char *)c_msg,
								FML_FML_NAME, (char *)c_tag);
	if (i_ch_val == -1)
	{
		fn_errlog(c_pgm_nm, "L31195", LIBMSG, c_retmsg);
	}

	return;
}

int fn_begintran(char *c_servicename,
				 char *c_err_msg)
{
	// #define TRAN_TIMEOUT     300      defined above
	// #define TPMSG  tpstrerror(tperrno) defined  in fo.h
	// #define REMOTE_TRNSCTN	2   defined above
	// #define LOCAL_TRNSCTN   1  defined above
	int i_trnsctn;
	char c_msg[256];

	if (tpgetlev() == 0)
	{
		if ((tpbegin(TRAN_TIMEOUT, 0)) == -1)
		{
			fn_errlog(c_servicename, "L31200", TPMSG, c_err_msg);
			return (-1);
		}
		i_trnsctn = LOCAL_TRNSCTN;
	}
	else
	{
		i_trnsctn = REMOTE_TRNSCTN;
	}
	sprintf(c_msg, "EBA : i_trnsctn:%d:", i_trnsctn);
	fn_userlog(c_servicename, c_msg);

	return i_trnsctn;
}

int fn_committran(char *c_servicename,
				  int i_trnsctn,
				  char *c_err_msg)
{
	// #define REMOTE_TRNSCTN	2   defined above
	// #define LOCAL_TRNSCTN   1  defined above
	if (i_trnsctn == LOCAL_TRNSCTN)
	{
		if ((tpcommit(0)) == -1)
		{
			fn_errlog(c_servicename, "L31205", TPMSG, c_err_msg);
			tpabort(0);
			return (-1);
		}
		fn_userlog(c_servicename, "Transaction committed");
	}
	else if (i_trnsctn == REMOTE_TRNSCTN)
	{
		/* No log needed*/
	}
	else
	{
		fn_userlog(c_servicename, "Invalid Transaction Number |%d|", i_trnsctn);
	}
	return (0);
}

int fn_aborttran(char *c_servicename,
				 int i_trnsctn,
				 char *c_err_msg)
{
	if (i_trnsctn == LOCAL_TRNSCTN)
	{
		tpabort(0);
	}
	return (0);
}

long int fn_maxl(long int li_val1, long int li_val2)
{
	if (li_val1 > li_val2)
	{
		return li_val1;
	}
	else
	{
		return li_val2;
	}
}

long int fn_minl(long int li_val1, long int li_val2)
{
	if (li_val1 < li_val2)
	{
		return li_val1;
	}
	else
	{
		return li_val2;
	}
}

double fn_maxd(double d_val1, double d_val2)
{
	if (d_val1 > d_val2)
	{
		return d_val1;
	}
	else
	{
		return d_val2;
	}
}

double fn_mind(double d_val1, double d_val2)
{
	if (d_val1 < d_val2)
	{
		return d_val1;
	}
	else
	{
		return d_val2;
	}
}

/***********Function for long to int conversion(Ver 1.3)***************/
// int fn_long_to_int(long l_input,int *i_out,char *c_err_msg )
//{
//   if((INT_MIN > l_input ) || (l_input > INT_MAX ))
//   {
//     sprintf(c_err_msg ,"%s","Value not in range of INT");
//     return -1;
//   }
//
//   i_out =  (int*)((char*)(&l_input) + 4);
//   return 0 ;
// }

/***********Function for unsigned long to int conversion(Ver 1.3)*********/
// int fn_unlong_to_unint(unsigned long ul_input,unsigned int *ui_out,char *c_err_msg )
//{
//   char c_str[20];
//   if((-UINT_MAX > ul_input )|| (ul_input > UINT_MAX ))
//   {
//     sprintf(c_err_msg ,"%s","Value not in range of unsigned INT");
//     return -1 ;
//   }
//   ui_out =  (unsigned int*)((char*)(&ul_input) + 4);
//   return 0;
// }

/*** Ver 1.4 Starts Here ****/

int fn_nolog_begintran(char *c_servicename,
					   char *c_err_msg)
{
	int i_trnsctn;
	char c_msg[256];

	if (tpgetlev() == 0)
	{
		if ((tpbegin(TRAN_TIMEOUT, 0)) == -1)
		{
			fn_errlog(c_servicename, "L31210", TPMSG, c_err_msg);
			return (-1);
		}
		i_trnsctn = LOCAL_TRNSCTN;
	}
	else
	{
		i_trnsctn = REMOTE_TRNSCTN;
	}
	return i_trnsctn;
}

int fn_nolog_committran(char *c_servicename,
						int i_trnsctn,
						char *c_err_msg)
{
	if (i_trnsctn == LOCAL_TRNSCTN)
	{
		if ((tpcommit(0)) == -1)
		{
			fn_errlog(c_servicename, "L31215", TPMSG, c_err_msg);
			tpabort(0);
			return (-1);
		}
	}
	else if (i_trnsctn == REMOTE_TRNSCTN)
	{
		/* No log needed*/
	}
	else
	{
		fn_userlog(c_servicename, "Invalid Transaction Number |%d|", i_trnsctn);
	}
	return (0);
}

/*** Ver 1.4 Ends Here ***/
