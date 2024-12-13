	/********************************************************************************************
Service       : sfo_heckyl.c

purpose       : To insert fetched data into the Heckyl's table 	 

Input Param   : For date
                Exchange Code

Output        : 

Stored
Procedure     : stp_NetInterSettTrf

Author        : ICICI Infotech

Date          : 21-08-2015

Version       :  1.0      Release(Tanmay Warkhade)
Version       :  1.1      Global indices data retrieval  - 07-Nov-2016 (Tanmay Warkhade)
Version       :  1.2      Global index Query change - 19-Nov-2016 (Tanmay Warkhade)

*******************************************************************************************/
#include<stdio.h>
#include <unistd.h>     /* ODBC Header file */
#include <sqlext.h>     /* ODBC Header file */
#include <sqlca.h>      /* Oracle header file   */
#include <atmi.h>       /* TUXEDO Header File   */
#include <stdlib.h>
#include <userlog.h>    /* TUXEDO Header File   */
#include <fml32.h>      /* TUXEDO Header File   */
#include <Usysflds.h>   /* TUXEDO Field Definitions     */
#include <string.h>     
#include <fn_read_debug_lvl.h>    /* Debug Level */
#include <fn_log.h>
#include <fo_fml_def.h>
#include <fml_def.h>
#include <fo.h>
#define MAX_FETCH 10     /** ver 1.1 **/
#define TOTAL_FML 31			/** ver 1.1 **/

char* Reset_Connection(HSTMT l_hstmt,char * svc, FBFR32* buf);  /* Ver 1.1 */

extern HDBC hdbc;
extern HENV henv;
extern char *ODBC_error(HSTMT);
SQLUSMALLINT    statusarray[MAX_FETCH];   /** Ver 1.1 **/
SQLINTEGER    pramsprocessed = 0;

struct fno_ltq_data                    /** Ver 1.1 **/
{
  	double   sql_ltq_rt;
  	double   sql_opn_prc;
  	double   sql_cls_prc;
  	double   sql_low_prc;
  	double   sql_high_prc;
    char     sql_ltq_dt[22+1]	;
    int 		 i_index_value;

}; 

void SFO_HECKYL_FIND(TPSVCINFO *ptr_data)
{
 /* Declare input output buffers */
  FBFR32 *ptr_fml_Ibuf;
  FBFR32 *ptr_fml_Obuf;

 	RETCODE rc;
 	HSTMT   hstmt;
 	RETCODE rc1[TOTAL_FML];
	SQLINTEGER i_index_value ;     /** Ver 1.1 **/
	SQLINTEGER sql_ltq_rt;					  /** Ver 1.1 **/
	SQLINTEGER sql_opn_prc;					  /** Ver 1.1 **/
	SQLINTEGER sql_cls_prc;					  /** Ver 1.1 **/
	SQLINTEGER sql_low_prc;					  /** Ver 1.1 **/
	SQLINTEGER sql_high_prc;				  /** Ver 1.1 **/
	SQLINTEGER sql_ltq_dt;

	char c_ServiceName [ 33 ]; /* Name of called service */

	MEMSET(c_ServiceName);
	
	int   i;
	int   j;                 /*** Ver 1.1 **/
	int   i_loop;
  int   i_no_rows;
  int   i_returncode=0;    /*** Ver 1.1 ***/
  int   tot_col=0;        /*** Ver 1.1 ***/
	char  c_run_mode;      /*** Ver 1.1 ***/
	int   i_err[3]; 	
	int   i_ferr[3]; 	

	int   i_odbcerr[8]; 	   /*** Ver 1.1 ***/
	int   i_odbcferr[8]; 		 /*** Ver 1.1 ***/	
	char  c_match_id[15];
	char  c_stk_cd[12];
	char  c_stk_nm[301];
  char s_exec_string[300];
  char  c_errmsg[256];
	char  c_isin_no[13];
	long  i_qty;
 	long  l_space_avl;
	long  l_space_rqd;
  struct fno_ltq_data fno_ltq_arr[MAX_FETCH];     /** Ver 1.1 ***/

	unsigned short n_bufferlength;
	
 /* Retrive data from Input buffer */
  strcpy( c_ServiceName, ptr_data->name );

  INITDBGLVL( c_ServiceName ); /* Debug level */

  ptr_fml_Ibuf = (FBFR32 *)ptr_data->data;
  ptr_fml_Obuf = (FBFR32 *)NULL;

  if(DEBUG_MSG_LVL_3)     /*** Ver 1.1 ***/
  {
	  fn_userlog(c_ServiceName,"Start of Process Heckyl");
	}

	i_returncode = Fget32(ptr_fml_Ibuf,FFO_RQST_TYP ,0,(char *)&c_run_mode, 0);       /*** Ver 1.1 ***/ 
	
	if (i_returncode == -1)                                                          /*** Ver 1.1 ***/
  {
		fn_errlog(c_ServiceName, "S31005", TPMSG,c_errmsg);
    Fadd32(ptr_fml_Ibuf, FML_ERR_MSG,  c_errmsg, 0);
    tpreturn(TPFAIL, 0, (char *)ptr_fml_Ibuf, 0, 0);	
	}

	if(DEBUG_MSG_LVL_3)
  {
    fn_userlog(c_ServiceName,"c_run_mode :%c:",c_run_mode);                           /*** Ver 1.1 ***/
	}

	ptr_fml_Obuf = (FBFR32 *)tpalloc("FML32",NULL,MIN_FML_BUF_LEN * 10);
 	if (ptr_fml_Obuf == NULL)
  {
    fn_errlog(c_ServiceName, "S31010", TPMSG,c_errmsg);
    Fadd32(ptr_fml_Ibuf, FML_ERR_MSG,  c_errmsg, 0);
    tpreturn(TPFAIL, 0, (char *)ptr_fml_Ibuf, 0, 0);
  }

	if ( c_run_mode == 'N')
	{

 	rc = SQLAllocStmt(hdbc, &hstmt);
    if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
    {
      fn_userlog(c_ServiceName,"Unable to Allocate a hstmt <%d>.",rc);
      fn_errlog(c_ServiceName,"S31015",ODBC_MSG,c_errmsg);
      tpfree((char *)ptr_fml_Obuf);
      Fadd32(ptr_fml_Ibuf, FML_ERR_MSG, c_errmsg, 0);
      tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
    }

    MEMSET(s_exec_string);

    sprintf(s_exec_string,"INSERT INTO tbl_LF_FinX_ICICICodeMaster_History SELECT * FROM tbl_LF_FinX_ICICICodeMaster ");

    fn_userlog(c_ServiceName,"Execute string is <%s>",s_exec_string);

    /********** Execute the stored procedure *********/

    rc=SQLExecDirect(hstmt,(UCHAR *)s_exec_string,strlen(s_exec_string));
    if ((rc != SQL_SUCCESS) )
    {
      fn_userlog(c_ServiceName,"Unable to Allocate a hstmt <%d>.",rc);
      fn_errlog(c_ServiceName,"S31020",ODBC_MSG,c_errmsg);
      tpfree((char *)ptr_fml_Obuf);
      Fadd32(ptr_fml_Ibuf, FML_ERR_MSG, c_errmsg, 0);
      tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
     }

	rc = SQLAllocStmt(hdbc, &hstmt);	
  if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
  {
  	fn_userlog(c_ServiceName,"Unable to Allocate a hstmt <%d>.",rc);
    errlog(c_ServiceName,"S31025",ODBC_MSG,DEF_USR,DEF_SSSN,c_errmsg);
    tpfree((char *)ptr_fml_Obuf);
    Fadd32(ptr_fml_Ibuf, FML_ERR_MSG, c_errmsg, 0);
    tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
  }   

		MEMSET( s_exec_string );

	 sprintf(s_exec_string,"truncate table tbl_LF_FinX_ICICICodeMaster");

	 fn_userlog(c_ServiceName,"Execute string is <%s>",s_exec_string);

	 rc=SQLExecDirect(hstmt,(UCHAR *)s_exec_string,strlen(s_exec_string));
    if ((rc != SQL_SUCCESS) )
    {
      fn_userlog(c_ServiceName,"Unable to Allocate a hstmt <%d>.",rc);
      fn_errlog(c_ServiceName,"S31030",ODBC_MSG,c_errmsg);
      tpfree((char *)ptr_fml_Obuf);
      Fadd32(ptr_fml_Ibuf, FML_ERR_MSG, c_errmsg, 0);
      tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
    }
   i_no_rows = Foccur32((FBFR32 *)ptr_fml_Ibuf,FFO_UNDRLYNG);
   for(i=0;i<i_no_rows;i++)
   {
		MEMSET(c_stk_cd);
		MEMSET(c_stk_nm);
		MEMSET(c_isin_no);
   rtrim(c_match_id);
   i_err [0] = Fget32(ptr_fml_Ibuf,FFO_UNDRLYNG ,i,(char *)c_stk_cd ,0);
   i_ferr[0] = Ferror32;
   i_err [1] = Fget32(ptr_fml_Ibuf,FFO_STCK_CD ,i,(char *)c_stk_nm ,0);
   i_ferr[1] = Ferror32;
   i_err [2] = Fget32(ptr_fml_Ibuf,FFO_ISIN_NMBR ,i,(char *)c_isin_no ,0);
   i_ferr[2] = Ferror32;

   for( i_loop=0; i_loop<=2; i_loop++)
   {
		  fn_userlog(c_ServiceName ," i_ferr [i_loop] :%d:", i_ferr [i_loop] );
     if(i_ferr[i_loop] == -1)
     {
      fn_errlog(c_ServiceName, "S31035", FMLMSG, c_errmsg);
      fn_userlog(c_ServiceName,"Error [%d] in Fchange at [%d]", i_ferr[i_loop],i_loop );
     /**  printf( "System error. Contact system support\n" );  ***/
      break;
     }
   }
  if(DEBUG_MSG_LVL_3)
  {
		fn_userlog(c_ServiceName,"c_stk_cd :%s:",c_stk_cd);
		fn_userlog(c_ServiceName,"c_stk_nm :%s:",c_stk_nm);
		fn_userlog(c_ServiceName,"c_isin_no :%s:",c_isin_no);
	}	

		rc = SQLAllocStmt(hdbc, &hstmt);
    if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
    {
      fn_userlog(c_ServiceName,"Unable to Allocate a hstmt <%d>.",rc);
      fn_errlog(c_ServiceName,"S31040",ODBC_MSG,c_errmsg);
      tpfree((char *)ptr_fml_Obuf);
      Fadd32(ptr_fml_Ibuf, FML_ERR_MSG, c_errmsg, 0);
      tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
    }
		MEMSET(s_exec_string);

		sprintf(s_exec_string,"INSERT INTO tbl_LF_FinX_ICICICodeMaster(ISEC_Stock_Code,NSE_Stock_Code,ISIN,Sync_Date) values('%s', '%s', '%s', SYSDATETIME())", c_stk_cd,c_stk_nm,c_isin_no); 

	  fn_userlog(c_ServiceName,"Execute string is <%s>",s_exec_string);
	
    /********** Execute the stored procedure *********/

    rc=SQLExecDirect(hstmt,(UCHAR *)s_exec_string,strlen(s_exec_string));
    /**if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)) **/
    if ((rc != SQL_SUCCESS) )
    {
      fn_userlog(c_ServiceName,"Unable to Allocate a hstmt <%d>.",rc);
      fn_errlog(c_ServiceName,"S31045",ODBC_MSG,c_errmsg);
      tpfree((char *)ptr_fml_Obuf);
      Fadd32(ptr_fml_Ibuf, FML_ERR_MSG, c_errmsg, 0);
      tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
    }



	  l_space_avl = Funused32(ptr_fml_Obuf); 
		l_space_rqd  = Fneeded32(1, n_bufferlength);	
		if (l_space_avl < l_space_rqd)
    {
      l_space_rqd = Fsizeof32(ptr_fml_Obuf) + MIN_FML_BUF_LEN;
      ptr_fml_Obuf=(FBFR32 *)tprealloc((char *)ptr_fml_Obuf,l_space_rqd);
      if (ptr_fml_Obuf == NULL)
      {
        fn_errlog(c_ServiceName, "S31050", FMLMSG, c_errmsg);
        Fadd32( ptr_fml_Ibuf,FML_ERR_MSG, c_errmsg, 0 );
        tpfree((char *)ptr_fml_Obuf);
        tpreturn(TPFAIL,0L, (char *)ptr_fml_Ibuf, 0, 0);
      }	
		}

		rc = SQLFreeStmt(hstmt, SQL_DROP);
	 }
  }	
 	/** Ver 1.1 Starts ***/
	else if( c_run_mode == 'G') 
  {

		MEMSET(fno_ltq_arr);

		rc = SQLAllocStmt(hdbc, &hstmt);
  	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
    {
     fn_userlog(c_ServiceName,"Unable to Allocate a hstmt <%d>.",rc);
     errlog(c_ServiceName,"S31055",ODBC_MSG,DEF_USR,DEF_SSSN,c_errmsg);
     tpfree((char *)ptr_fml_Obuf);
     Fadd32(ptr_fml_Ibuf, FML_ERR_MSG, c_errmsg, 0);
     tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
    }

    MEMSET( s_exec_string );

    sprintf(s_exec_string,"");

     /* rc = SQLPrepare(hstmt,(UCHAR*)"SELECT Top 3 Price, tradetime, PriceOpen, PreviousDayClose, Low, high, GlobalIndexId  from tbl_LF_FinX_GlobalIndexesOutput where GlobalIndexId in('3','4','11') group by GlobalIndexId, Price, tradetime , PriceOpen, High, Low, PreviousDayClose  order by tradetime desc", SQL_NTS) ;  commented in 1.2 ****/  


	rc = SQLPrepare(hstmt,(UCHAR*)"select Price,tradetime,PriceOpen,PreviousDayClose, Low, high , GlobalIndexId from ( select  top 1 GlobalIndexId, Price,PriceOpen, tradetime,PreviousDayClose, Low, high from tbl_LF_FinX_GlobalIndexesOutput  where GlobalIndexId = '3'  order by tradetime desc ) first_tab union all select Price,tradetime,PriceOpen,PreviousDayClose, Low, high , GlobalIndexId from ( select top 1  GlobalIndexId, Price,PriceOpen, tradetime,PreviousDayClose, Low, high from tbl_LF_FinX_GlobalIndexesOutput  where GlobalIndexId = '11' order by tradetime desc ) second_tab union all select Price,tradetime,PriceOpen,PreviousDayClose, Low, high , GlobalIndexId from ( select top 1 GlobalIndexId, Price,PriceOpen, tradetime,PreviousDayClose, Low, high from tbl_LF_FinX_GlobalIndexesOutput  where GlobalIndexId = '4' order by tradetime desc )third_tab", SQL_NTS);


  rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_BIND_TYPE,(SQLPOINTER)sizeof(struct fno_ltq_data), 0);
  if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
  {
    fn_userlog( c_ServiceName,"Unable to Allocate a hstmt.");
    strcpy(c_errmsg,ODBC_MSG);
    strcpy(c_errmsg,Reset_Connection(hstmt,c_ServiceName,ptr_fml_Ibuf));
    fn_errlog( c_ServiceName, "S31060", c_errmsg, c_errmsg  );
     tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
  }

  rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)MAX_FETCH, 0);
  if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
  {
    fn_userlog( c_ServiceName,"Unable to Allocate a hstmt.");
    strcpy(c_errmsg,ODBC_MSG);
    strcpy(c_errmsg,Reset_Connection(hstmt,c_ServiceName,ptr_fml_Ibuf)); 
    fn_errlog( c_ServiceName, "S31065", c_errmsg, c_errmsg  );
     tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
  }

/* Specify an array in which to return the status of each set of ** parameters.  */

  rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_STATUS_PTR, (SQLPOINTER)statusarray, 0);
  if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
  {
    fn_userlog( c_ServiceName,"Unable to Allocate a hstmt.");
    strcpy(c_errmsg,ODBC_MSG);
    strcpy(c_errmsg,Reset_Connection(hstmt,c_ServiceName,ptr_fml_Ibuf)); /** Ver 1.1 **/
    fn_errlog( c_ServiceName, "S31070", c_errmsg, c_errmsg  );
     tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
  }

  rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &pramsprocessed, 0);
  if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
  {
    fn_userlog( c_ServiceName,"Unable to Allocate a hstmt.");
    strcpy(c_errmsg,ODBC_MSG);
    strcpy(c_errmsg,Reset_Connection(hstmt,c_ServiceName,ptr_fml_Ibuf)); /** Ver 1.1 **/
    fn_errlog( c_ServiceName, "S31075", c_errmsg, c_errmsg  );
    tpreturn(TPFAIL, 0L, (char *)ptr_fml_Obuf, 0L, 0);
  }


		sql_ltq_rt = SQL_NTS;
		rc1[0 ] = SQLBindCol(hstmt, 1, SQL_C_DOUBLE, &fno_ltq_arr[0].sql_ltq_rt,(SDWORD)sizeof(fno_ltq_arr[0].sql_ltq_rt), &sql_ltq_rt);

		sql_ltq_dt = SQL_NTS;
	  rc1[1 ] = SQLBindCol(hstmt, 2, SQL_C_CHAR, fno_ltq_arr[0].sql_ltq_dt, 24, &sql_ltq_dt);

		sql_opn_prc = SQL_NTS;
		rc1[2 ] = SQLBindCol(hstmt, 3, SQL_C_DOUBLE, &fno_ltq_arr[0].sql_opn_prc,(SDWORD)sizeof(fno_ltq_arr[0].sql_opn_prc), &sql_opn_prc);

		sql_cls_prc = SQL_NTS;
		rc1[3 ] = SQLBindCol(hstmt, 4, SQL_C_DOUBLE, &fno_ltq_arr[0].sql_cls_prc,(SDWORD)sizeof(fno_ltq_arr[0].sql_cls_prc), &sql_cls_prc);

		sql_low_prc = SQL_NTS;
		rc1[4 ] = SQLBindCol(hstmt, 5, SQL_C_DOUBLE, &fno_ltq_arr[0].sql_low_prc,(SDWORD)sizeof(fno_ltq_arr[0].sql_low_prc), &sql_low_prc);

		sql_high_prc = SQL_NTS;
		rc1[5 ] = SQLBindCol(hstmt, 6, SQL_C_DOUBLE, &fno_ltq_arr[0].sql_high_prc,(SDWORD)sizeof(fno_ltq_arr[0].sql_high_prc), &sql_high_prc);

		i_index_value = SQL_NTS;
	  rc1[6 ] = SQLBindCol(hstmt, 7, SQL_INTEGER , &fno_ltq_arr[0].i_index_value, (SDWORD)sizeof(fno_ltq_arr[0].sql_ltq_rt), &i_index_value);

  	tot_col=3;

		for (i=0;i<tot_col;i++) /* Check for Errors */
    {
     if ((rc1[i] != SQL_SUCCESS) && (rc1[i] != SQL_SUCCESS_WITH_INFO))
     {
      fn_userlog( c_ServiceName," Counter = :%d:",i);
      strcpy(c_errmsg,ODBC_MSG);
      strcpy(c_errmsg,Reset_Connection(hstmt,c_ServiceName,ptr_fml_Ibuf)); /** Ver 1.1 **/
      fn_errlog( c_ServiceName, "S31080", c_errmsg, c_errmsg  );
      tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
     }
    }

	  rc = SQLExecute(hstmt);
    if ((rc != SQL_SUCCESS) )
    {
      fn_userlog(c_ServiceName,"Unable to Allocate a hstmt <%d>.",rc);
      fn_errlog(c_ServiceName,"S31085",ODBC_MSG,c_errmsg);
      tpfree((char *)ptr_fml_Obuf);
      Fadd32(ptr_fml_Ibuf, FML_ERR_MSG, c_errmsg, 0);
      tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
    }	
	
   
  while ((rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0)) != SQL_NO_DATA)
  {
		fn_userlog( c_ServiceName,"Param Processed :%d:",pramsprocessed); 
    for ( i = 0; i< pramsprocessed ; i++ )
    {
  		if(DEBUG_MSG_LVL_3)
  		{
				fn_userlog( c_ServiceName,"i_index_value :%d:", fno_ltq_arr[i].i_index_value ) ;
				fn_userlog( c_ServiceName,"sql_ltq_rt :%lf:", fno_ltq_arr[i].sql_ltq_rt ) ;
				fn_userlog( c_ServiceName,"sql_opn_prc :%lf:", fno_ltq_arr[i].sql_opn_prc ) ;
				fn_userlog( c_ServiceName,"sql_cls_prc :%lf:", fno_ltq_arr[i].sql_cls_prc ) ;
				fn_userlog( c_ServiceName,"sql_low_prc :%lf:", fno_ltq_arr[i].sql_low_prc ) ;
				fn_userlog( c_ServiceName,"sql_high_prc :%lf:", fno_ltq_arr[i].sql_high_prc ) ;
				fn_userlog( c_ServiceName,"sql_ltq_dt :%s:", fno_ltq_arr[i].sql_ltq_dt) ;
			}	

   		i_odbcerr[0] = Fadd32(ptr_fml_Obuf,FFO_INDX_VAL ,(char *)&fno_ltq_arr[i].i_index_value ,0);
   		i_odbcferr[0] = Ferror32;
   		i_odbcerr[1] = Fadd32(ptr_fml_Obuf,FFO_FUT_PRC ,(char *)&fno_ltq_arr[i].sql_ltq_rt,0);
   		i_odbcferr[1] = Ferror32; 
   		i_odbcerr[2] = Fadd32(ptr_fml_Obuf,FFO_NXT_TRD_DT ,(char *)fno_ltq_arr[i].sql_ltq_dt,0);
   		i_odbcferr[2] = Ferror32;
   		i_odbcerr[3] = Fadd32(ptr_fml_Obuf,FFO_RATE ,(char *)&fno_ltq_arr[i].sql_opn_prc ,0); /** ver 1.2 FML changed FFO_OPN_PRC to FFO_RATE **/
   		i_odbcferr[3] = Ferror32;
   		i_odbcerr[4] = Fadd32(ptr_fml_Obuf,FFO_EFF_CLS_PRC ,(char *)&fno_ltq_arr[i].sql_cls_prc ,0); /** Ver 1.2 FFO_CLS_PRC to FFO_EFF_CLS_PRC **/
   		i_odbcferr[4] = Ferror32;
   		i_odbcerr[5] = Fadd32(ptr_fml_Obuf,FFO_OI_HIGH ,(char *)&fno_ltq_arr[i].sql_high_prc ,0); /** Ver 1.2 FFO_HGH_PRC to FFO_OI_HIGH **/
   		i_odbcferr[5] = Ferror32;
   		i_odbcerr[6] = Fadd32(ptr_fml_Obuf,FFO_OI_LOW ,(char *)&fno_ltq_arr[i].sql_low_prc ,0); /** Ver 1.2 FFO_LOW_PRC to FFO_OI_LOW **/
   		i_odbcferr[6] = Ferror32;

      for (j=0;j <= 6;j++)
      {
       if (i_odbcerr[j] == -1)
 	     {
 	     	fn_userlog(c_ServiceName,"Unable to fadd ");
 	     	fn_errlog(c_ServiceName,"S31090",ODBC_MSG,c_errmsg);
      	fn_userlog(c_ServiceName,"Error [%d] in Fchange at [%d]", i_ferr[j],j );
 	     	tpfree((char *)ptr_fml_Obuf);
 	     	Fadd32(ptr_fml_Ibuf, FML_ERR_MSG, c_errmsg, 0);
 	     	tpreturn(TPFAIL, 0L, (char *)ptr_fml_Ibuf, 0, 0);
 	   	 }
			}
		}	
	 }
	}
	 /** Ver 1.1 * Ends **/

  tpreturn(TPSUCCESS, 0, (char *)ptr_fml_Obuf, 0, 0);
} 
