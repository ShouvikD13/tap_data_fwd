#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fn_env.h>
#include <fn_log.h>
#include <unistd.h>		/*  Ver 1.1 AIX Migration 11-Dec-2007 ROhit */

char list[MAX_TOKEN][MAX_LEN];
int count;

int fn_init_prcs_spc ( char *c_Servicename, char *c_filenm, char *c_prcs_nm )
{
	char szLine[MAX_LEN];
	FILE *fp;
	char szToken[MAX_LEN];
	char szPrcsName[MAX_LEN];
	int flag;
	extern int count;
	int matchfound;
	char c_msg [ MSG_LEN ];

	count = 0;
	matchfound = 0;
	flag = 0;

	if ( access( c_filenm, 0 ) == -1 )
	{
		sprintf ( c_msg, "INI File :%s: does not exist", c_filenm );	
		fn_userlog ( c_Servicename,  c_msg );
		return -1;
	}

	fp = fopen( c_filenm, "r" );
	if ( fp == NULL ) 
	{
		sprintf ( c_msg, "INI File :%s: Data Not Found", c_filenm );	
		fn_userlog ( c_Servicename,  c_msg );
		return -1;
	}
	while ( fgets ( szLine, MAX_LEN, fp ) != NULL ) // reading each line into szLine untill EOF
	{
		szLine[strlen(szLine) - 1 ] = '\0';
		if ( strncmp( szLine, "[", /*(int)*/strlen("[") ) == 0 )				/* AIX migration Ver 1.1 */
		{
			memset ( szToken, 0, sizeof( szToken ) );
			memset ( szPrcsName, 0, sizeof( szPrcsName ) );
			GetFieldValue( szLine, szToken, "[", 2 );  // so in , GetFieldValue() for this call what happeneing is we are calling first time with 
			GetFieldValue( szToken, szPrcsName, "]", 1 );
			if ( strcmp( szPrcsName, c_prcs_nm ) == 0 )
			{
				flag=1;
				matchfound=1;
			}
		}
		else if ( ( strncmp( szLine, "[",/*(int)*/ strlen("[") ) != 0 ) && 		/* AIX migration  Ver 1.1*/
              ( flag == 1 )                                        )
		{
			if ( strlen( szLine ) != 0 )
			{
				strcpy ( list [ count ], szLine );
				count++;
				if ( count == MAX_TOKEN )
				{
					fn_userlog ( c_Servicename, "Exceeding max token limit" );
					sprintf ( c_msg, "MAX_TOKEN :%d: Count of Token :%d:", 
                    MAX_TOKEN, count );
					fn_userlog ( c_Servicename,  c_msg );
					return -1;
				}
			}
			else
			{
				break;
			}
		}
	}
	fclose(fp);

	if ( matchfound == 0 )
	{
		sprintf ( c_msg, "%s Not Specified in the INI File %s\n", 
                     c_prcs_nm, c_filenm );
		fn_userlog ( c_Servicename,  c_msg );
		return -1;
	}

	return 0;
}

char* fn_get_prcs_spc ( char *c_Servicename, char *c_token )
{
	int i;
	static char c_val [ MAX_LEN ];
	int matchfound;
	char szToken[MAX_LEN];
	extern int count;
	char c_msg [ MSG_LEN ];

	matchfound = 0;

	for ( i=0; i<count; i++ )
	{
		memset( szToken, 0, sizeof( szToken ) );
		GetFieldValue( list[i], szToken, "=", 1 );
		if ( strcmp ( c_token, szToken ) == 0 )
		{
			memset( szToken, 0, sizeof( szToken ) );
			GetFieldValue( list[i], szToken, "=", 2 );
			strcpy ( c_val, szToken ); 
			matchfound = 1;
			break;
		}
	}
	
	if ( matchfound == 0 )
	{
		sprintf (c_msg, "Match Not Found for %s", c_token );
		fn_userlog ( c_Servicename,  c_msg );
		return NULL;
	}
	else
	{
		return c_val;
	}
}

void GetFieldValue ( char *szSourceLine, // source string
                     char *szFldValue, // Output string where extracted filed value will be stored 
                     char *szSeparator, // characters or string which will be used to seperate the field from source string 
                     int iFldCount )// Number of fields to be extracted
{
	int  i = 0;
/*	int  j = 0; */
	long j = 0;				/* AIX migration Ver 1.1*/

	char *ptr;
	char *pszDummyPtr;

	pszDummyPtr = szSourceLine; // list 

	for (i = 0; i < iFldCount - 1; i++)  // for first funcion call where i fieldcount = 2
	{
  	if ( ( ptr = (char *)strstr(pszDummyPtr, szSeparator) ) != NULL )//
  	{
    	pszDummyPtr = ptr + 1;

    	if (i == iFldCount - 2)
    	{
      	j = 0;

      	while ( ( strncmp ( pszDummyPtr + j, szSeparator, strlen ( szSeparator )) != 0  ) &&  ( j != strlen(pszDummyPtr)           )    )
      	{
        	szFldValue[j] = pszDummyPtr[j];
        	j++;
      	}
    	}
  	}
  	else
  	{
    	break;
  	}
	}

  if (iFldCount == 1)
  {
    j = 0;

    while ( ( strncmp ( pszDummyPtr + j, szSeparator, strlen ( szSeparator )) != 0  ) && ( j != /*(int)*/strlen(pszDummyPtr)                ) )				/* AIX mirgration Ver 1.1 */ 
    {
      szFldValue[j] = pszDummyPtr[j];
      j++;
    }
  }
}
