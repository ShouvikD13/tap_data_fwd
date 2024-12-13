/* Ver 1.1 AIX Migration ROhit 24/12/2007 */
/* Ver 1.1 Debug Level Rohit 03/03/08 */

#define DEBUG 1

/* #define UID  "MatchDerivatives"
#define PWD  "match!@dr"
#define DRIVER  "MatchDerivatives" */  /* Ver 1.1 */
#define INIFILE "ODBC.ini"         /** Ver 1.1 **/
#define SQLTOKEN "ODBC_HECKYL_FIND"      /** Ver 1.1 **/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sqlext.h>
#include <ctype.h>
#include <atmi.h>       /* TUXEDO Header File            */
#include <userlog.h>    /* TUXEDO Header File            */
#include <sqlcode.h>
#include <fn_log.h>
#include <fn_env.h>			/* Ver 1.1 */
#include <string.h>    /* Added in VER TOL : TUX on LINUX */

char    ServiceName [ 33 ];  
HDBC  	hdbc;
HENV  	henv;
HSTMT 	hstmt;
RETCODE rc;
CHAR		uid[20];
CHAR		pwd[20];
CHAR		driver[20];

int fn_read_debug_flg(char * cptr_prcs_name);	/* Debug Level */


RETCODE EnvInit(HENV *henv, HDBC *hdbc)
{
	RETCODE rc;
	rc = SQLAllocEnv (henv);
	if ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO))
	{
		rc = SQLAllocConnect (*henv, hdbc);
	}
	return (rc);
}

void EnvClose(HENV henv, HDBC hdbc)
{
	SQLDisconnect (hdbc);
	SQLFreeConnect (hdbc);
	SQLFreeEnv (henv);
}

char* ODBC_error (HSTMT l_hstmt) 
{
	UCHAR sqlstate[10];
	static __thread UCHAR errmsg[SQL_MAX_MESSAGE_LENGTH];  /* Added in Ver TOL : Compilation patch */
	SDWORD  nativeerr;
	SWORD actualmsglen;
	RETCODE rc;

	rc = SQLError ( 
				henv, 
				hdbc, 
				l_hstmt,
				sqlstate, 
				&nativeerr, 
				errmsg,
				SQL_MAX_MESSAGE_LENGTH - 1, 
				&actualmsglen);

	userlog ("SQLSTATE = %s",sqlstate);
	userlog ("NATIVE ERROR = %d",nativeerr);
	userlog ("MSG = %s",errmsg);
	return (char *)errmsg;
}

RETCODE ODBC_Connect (UCHAR *p_driver, UCHAR *p_uid, UCHAR *p_pwd)
{
  RETCODE rc;
  int retries;

	userlog("Driver/uid/pwd = %s, %s, %s",p_driver, p_uid, p_pwd);

  for (retries = 1; retries <= 3; retries++)
  {
    rc = SQLConnect (hdbc, p_driver, SQL_NTS, p_uid, SQL_NTS, p_pwd, SQL_NTS);
    if ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO))
    {
			userlog("Connection Successful - %s, %s, %s", p_driver, p_uid,p_pwd);
      break;
    }
    else
    {

			userlog("rc = %d", rc);

      userlog("SQLConnect: Failed...");
      userlog(ODBC_error (SQL_NULL_HSTMT));
      userlog("SQLConnect: Retrying Connect.");
    }
  }
	return(rc);
}


char* Reset_Connection(HSTMT l_hstmt,char * svc, FBFR32* buf)
{
  RETCODE rc;
  UCHAR sqlstate[10];
  static __thread UCHAR errmsg[SQL_MAX_MESSAGE_LENGTH]; /* Added in Ver TOL : Compilation patch */
  SDWORD  nativeerr;
  SWORD actualmsglen;

  rc = SQLError (
        henv,
        hdbc,
        l_hstmt,
        sqlstate,
        &nativeerr,
        errmsg,
        SQL_MAX_MESSAGE_LENGTH - 1,
        &actualmsglen);

  userlog ("SQLSTATE = |%s|",sqlstate);
  userlog ("NATIVE ERROR = |%d|",nativeerr);
  userlog ("MSG = |%s|",errmsg);

  if ( ((strcmp((char*)sqlstate,"01000") == 0) && (nativeerr == 131)) ||
       (strcmp((char*)sqlstate,"08S01") == 0) )
  {
    SQLFreeStmt(hstmt, SQL_DROP);
    userlog("hdbc = %d",hdbc);
    SQLDisconnect (hdbc);

    rc = ODBC_Connect ((UCHAR *)driver, (UCHAR *)uid, (UCHAR *)pwd);

    userlog("RC = %d", rc);
    if (rc == -1)
    {
      userlog(" Reset Connection failed");
      strcpy((char*)errmsg,"ODBC Connection not Available");
      return (char*)errmsg;
    }
    else
    {
      userlog(" RESET CONNECTION SUCCESSFUL");
      strcpy((char*)errmsg,"");
      userlog("tpforwarding now");
      tpforward(svc,(char*)buf,0,0);
    }
}
 return (char *)errmsg;
}

int tpsvrinit( int argc, char *argv[] )
{
	RETCODE rc;
	int i_returncode;

	uid[0] =  0 ;
	pwd[0] =  0 ; 
	driver[0] =  0 ;

  int readaccesspara(void);		/* Ver 1.1 */

	/*strcpy(uid, UID);
	strcpy(pwd, PWD );
	strcpy(driver, DRIVER); */  /* Commented in Ver 1.1 */

	memset(uid,'\0',sizeof(uid));         /**Ver 1.1 **/
  memset(pwd,'\0',sizeof(pwd));         /**Ver 1.1 **/
  memset(driver,'\0',sizeof(driver));   /**Ver 1.1 **/

	if (tpopen() == -1)
  {
    userlog("TpOpen failed %s", tpstrerror(tperrno));
    return(-1);
  }																		/* Ver 1.1 */

	i_returncode = fn_create_pipe( );
	if ( i_returncode == -1 )
	{
		userlog( "ODBC : Error creating pipe" );
		return ( -1 );
	}

	if(readaccesspara() == -1)           /**ver 1.1 **/
  {
    userlog("Unable to read data from ini file.");
    return -1;
  }
  userlog("read ini file ...");

	rc = EnvInit (&henv, &hdbc);

	userlog("hdbc = %d",hdbc);
	userlog("henv = %d", henv);

	rc = ODBC_Connect ((UCHAR *)driver, (UCHAR *)uid, (UCHAR *)pwd);

	if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
	{
		userlog("Connect failed");
		return(0);
	}
	userlog(" connection successful");	
	
	fn_read_debug_flg(argv[0]);
  return(0);
}

void tpsvrdone()
{

	userlog("Environment Closed");

	EnvClose(henv, hdbc);
	fn_destroy_pipe( );
}



/** ver 1.1 **/
int readaccesspara()
{
  char *ptr;
  char c_filepath[200];

  ptr = NULL;

  memset(c_filepath,'\0',sizeof(c_filepath));

  userlog("Reading from INI file");

  sprintf(c_filepath,"%s/%s",tuxgetenv("BIN"),INIFILE);

  userlog(":%s:", c_filepath);

  if(fn_init_prcs_spc("heckyl_odbc",c_filepath,SQLTOKEN) == -1)
  {
    userlog("Unable to read SQL parameter from ini file.");
    return -1;
  }
  userlog("Reached point 1.2");

  ptr = fn_get_prcs_spc("heckyl_odbc","UID");

  if (ptr == NULL)
  {
    userlog("Unable to read User id from ini file.");
    return -1;
  }

  strcpy(uid,ptr);
  userlog(":%s:", uid);

  ptr = NULL;
  ptr = fn_get_prcs_spc("heckyl_odbc","PWD");

  if (ptr == NULL)
  {
    userlog("Unable to read pwd from ini file.");
    return -1;
  }

  strcpy(pwd,ptr);
  userlog(":%s:", pwd);

  ptr = NULL;
  ptr = fn_get_prcs_spc("heckyl_odbc","DRIVER");

  if (ptr == NULL)
  {
    userlog("Unable to read dl odbc driver from ini file.");
    return -1;
  }

  strcpy(driver,ptr);
  userlog(":%s:", driver);

  return 0;
}
