/***********************************************************************************
* File : fn_read_debug_lvl.c
*
*  Description: This library file contains the definition of functions used in
*               implementation of printing logs as per the debug levels 
*               maintained in serverdebugconfig.ini
*
*
*  Called by:
*
*  Calling Programs/Sub-routines:
*
*  Author : 3I-INFOTECH (DSK)
*
*  Created on: 25-Feb-2008
*
*  Ver 1.1   : 12-Sep-2016  Adding fn_get_channel (Mihir)
*  Ver 1.2   : 14-Sep-2016  Adding OFF line handling in fn_get_channel (Samip)
*  Ver 1.3   : 13-Oct-2016  Adding Ios and android native handling in fn_get_channel (Mihir)
*  Ver 1.4   : 21-Feb-2017  Handling New To IDirect order channel in fn_get_channel (Prachi Panhale)
*	 Ver 1.5   : 04-Nov-2019  CR-ISEC18-127640_Sub-broker_dual_order_placement 
*														API channel handling (Prachi Panhale)
****************************************************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<userlog.h>
# include <math.h>
# include <ctype.h>
# include <atmi.h>
# include <eba.h>
# include <fml32.h>
# include <Usysflds.h>   /* TUXEDO Field Definitions      */
# include <fml_def.h>
//#include<sys/limits.h>   
#include <limits.h>
#include <fml_rout.h>  
#include <fn_read_debug_lvl.h>

#define MAXSERVICELISTSZ 20

typedef struct st_servicedebuglist{
char c_servicename[30];
int i_debuglevel;
} servicedbglvl_t;

servicedbglvl_t servicedebuglist[MAXSERVICELISTSZ];

int gi_servicecount;
int gi_debug_flg;

int fn_get_debug_flg(char * cptr_servicename);
 
int fn_trim(char * c_str);

int fn_read_debug_flg(char * cptr_prcs_name)
{
  char c_filename[50]; 
  char c_line[300];
  char c_paraval[50]; 
  char* cptr_tmp;
  int  i_tokensrch=0;
  int i_debug_val;
  FILE *fp;

  memset(c_filename,'\0',sizeof(c_filename));
  memset(c_line,'\0',sizeof(c_line));
  memset(c_paraval,'\0',sizeof(c_paraval));
  memset(&servicedebuglist,0,sizeof(servicedbglvl_t));

  i_debug_val = 0;
  gi_servicecount = 0;

  strcpy(c_filename,"serverdebugconfig.ini");

  if ( access( c_filename, R_OK ) == -1 )
  {
    userlog( "Cannot access INI File :%s:\n", c_filename );
    printf( "Cannot access INI File :%s:\n", c_filename );
    return i_debug_val;
  }
  
  fp = fopen(c_filename,"r");
  if(fp == NULL)
  {
    userlog( "Unable to open  File :%s: in read mode\n", c_filename );
    printf( "Unable to open  File :%s: in read mode\n", c_filename );
    return i_debug_val;
  }
  
  while(fgets(c_line,255,fp) != NULL)
  {
 /*   userlog("Before trim :%s:\n",c_line); */
    fn_trim(c_line);
  /*  userlog("After trim :%s: i_tokensrch :%d: prcs name :%s:\n",c_line,i_tokensrch,cptr_prcs_name); 
    printf("After trim :%s: i_tokensrch :%d: prcs name :%s:\n",c_line,i_tokensrch,cptr_prcs_name); */

    if((c_line[0] == '#') || (strlen(c_line) < 3))
    {
/*      userlog("Ignoring comment :%s:\n",c_line); */
      continue;
    }

    if(i_tokensrch == 0) 
    {
      if((c_line[0] == '[') && (strncmp((c_line + 1),cptr_prcs_name,strlen(cptr_prcs_name)) == 0))
      {
        i_tokensrch = 1;
/*        printf("After trim :%s: i_tokensrch :%d: prcs name :%s:\n",c_line,i_tokensrch,cptr_prcs_name); */
      }
      continue;
    }

    if((i_tokensrch == 1) && (c_line[0] == '['))
    {
      break;
    }
    else if(i_tokensrch == 1) 
    {
      cptr_tmp = strstr(c_line,"=");
      strcpy(c_paraval,cptr_tmp + 1);
      *cptr_tmp = '\0';
      strcpy(servicedebuglist[gi_servicecount].c_servicename,c_line);
      fn_trim(c_paraval);
      fn_trim(servicedebuglist[gi_servicecount].c_servicename);

      if(strlen(c_paraval) != 0)
      {
        i_debug_val = atoi(c_paraval);
        if(i_debug_val < 0 || i_debug_val > 10)
        {
          servicedebuglist[gi_servicecount].i_debuglevel = 0;
        }
        else
        {
          servicedebuglist[gi_servicecount].i_debuglevel = i_debug_val;
        }
        gi_servicecount++;
      }
    }
  }
  fclose(fp);
  return i_debug_val;
}

int fn_trim(char * c_str)
{
  int i_strlen =0;
  int i_idx;

  i_strlen = strlen(c_str);
  for(i_idx = 0;i_idx < i_strlen;i_idx++)
  {
    if(*(c_str + 0) == ' ' || *(c_str + 0) == '\t')
    {
      strcpy(c_str, c_str + 1);
    }
    else
    {
      break;
    }
  }
  i_strlen = strlen(c_str);
  for(i_idx = i_strlen - 1;i_idx > -1;i_idx--)
  {
    if((*(c_str + i_idx) == ' ') || (*(c_str + i_idx) == '\t') || (*(c_str + i_idx) == '\n'))
    {
      *(c_str + i_idx) = '\0';
    }
    else
    {
      break;
    }
  }
  return 0;
}

int fn_get_debug_flg(char * cptr_servicename)
{
   int i_idx;
   int i_debuglvl;

   i_debuglvl = 0;
   for( i_idx = 0;i_idx < gi_servicecount; i_idx++)
   {
/*      userlog("In function fn_get_debug_flg ,servicename :%s: prcs name :%s:\n",servicedebuglist[i_idx].c_servicename,cptr_servicename ); */
      if(strcmp(servicedebuglist[i_idx].c_servicename,cptr_servicename) == 0)
      {
        i_debuglvl = servicedebuglist[i_idx].i_debuglevel;
/*        userlog("In function fn_get_debug_flg,debuglvl :%d:\n",servicedebuglist[i_idx].i_debuglevel); */
        break;
      }
   }
 /*  printf("In function fn_get_debug_flg ,called from :%s: is :%d:\n",cptr_servicename,i_debuglvl);  */
 /*   userlog("In function fn_get_debug_flg ,called from :%s: is :%d:\n",cptr_servicename,i_debuglvl); */ 
   return i_debuglvl;
}

/*** Ver 1.1 starts ***/
void fn_get_channel(char usr_typ, char *channel)
{
  if( (strcmp(channel, "SYS") != 0) && (strcmp(channel, "SEP") != 0) &&
       (strcmp(channel, "WE1") != 0) && (strcmp(channel, "WE5") != 0) &&
        (strcmp(channel, "WE2") != 0) && (strcmp(channel, "OFF") != 0) && /** Ver 1.2 **/
				(strcmp(channel, "WEA") != 0) )		/* ver 1.4 WEA added */
  {
    if(usr_typ == 'S')        /* Super User */
    {
      strcpy(channel, "CN1");
    }
    else if ( usr_typ == 'C') /* Call N Trade */
    {
      strcpy(channel,"CNT");
    }
    else if( usr_typ == 'M')  /* LOW BANDWIDTH SITE */
    {
      strcpy(channel,"WE3");
    }
    else if (usr_typ == 'L')  /* RIA */
    {
      strcpy(channel,"WE4");
    }
    else if (usr_typ == 'X')  /* ANDROID */
    {
      strcpy(channel,"WE6");
    }
    else if (usr_typ == 'I')  /* IPHONE  */
    {
      strcpy(channel,"WE7");
    }
    else if (usr_typ == 'W')  /* WINDOWS */
    {
      /* strcpy(channel,"WE8"); commented ver 1.5 */
      strcpy(channel,"WI8");	/* corrected ver 1.5 */
    }
    else if (usr_typ == 'T')  /* WINDOWS MOBILE */
    {
      strcpy(channel,"WE9");
    }
    else if (usr_typ == 'H')  /* HTML5 */
    {
      strcpy(channel,"WEH");
    }
    else if (usr_typ == 'N')  /* IDIRECT LITE ANDROID */
    {
      strcpy(channel,"WEN");
    }
    else if (usr_typ == 'O')  /* IDIRECT LITE IOS */
    {
      strcpy(channel,"WEO");
    }
    else if (usr_typ == 'P')  /* IDIRECT LITE WINDOWS */
    {
      strcpy(channel,"WEP");
    }
    else if (usr_typ == 'F')  /* READ ONLY ANDROID */ 
    {
      strcpy(channel,"REA");
    }
    else if (usr_typ == 'G')  /* READ ONLY IOS */
    {
      strcpy(channel,"REI");
    }
    else if (usr_typ == 'J')  /* READ ONLY WINDOWS */
    {
      strcpy(channel,"REW");
    }
    else if (usr_typ == 'R')  /* IBANK ANDROID */
    {
      strcpy(channel,"WER");
    }
    else if (usr_typ == 'U')  /* IBANK IOS */
    {
      strcpy(channel,"WEU");
    }
    else if (usr_typ == 'V')  /* IBANK WINDOWS */
    {
      strcpy(channel,"WEV");
    }
    else if (usr_typ == 'K')  /* EIPO */
    {
      strcpy(channel,"EIP");
    }
    else if (usr_typ == 'Y')  /* NEW TRADE RACER NORMAL */
    {
      strcpy(channel,"NET");
    }
    else if (usr_typ == 'Z')  /* NEW TRADE RACER SUPER USER */
    {
      strcpy(channel,"CN2");
    }
    else if ( usr_typ == 'Q') /* IBANK INFINITY */
    {
      strcpy(channel,"WEQ");
    }
    /*** Ver 1.3 starts ***/
		else if (usr_typ == 'B')  /* IDIRECT iPHONE NATIVE */
		{
			strcpy(channel,"WE8");
		}
		else if (usr_typ == 'D')  /* IDIRECT ANDROID NATIVE */
		{
			strcpy(channel,"WED");
		}
		/*** Ver 1.3 ends ***/
    else
    {
      strcpy(channel,"WEB");
    }
  }
}
/*** Ver 1.1 ends ***/

/* ver 1.5 start */
int fn_simaltns_ssn_term( char *c_ServiceName, char *c_usrid, long l_sssn_id,char *c_mtch_accnt_no,char *c_retmsg )
{
  FBFR32  *ptr_fml_Sbuffer;
  int   i;

  if(DEBUG_MSG_LVL_3)
  {
    userlog("Inside function fn_simaltns_ssn_term for match:%s: node:%s: session:%ld:",c_mtch_accnt_no,c_usrid,l_sssn_id );
  }

  ptr_fml_Sbuffer=(FBFR32 *)tpalloc("FML32",NULL,MIN_FML_BUF_LEN);
  if (ptr_fml_Sbuffer == NULL)
  {
    errlog(c_ServiceName,"L31005",TPMSG,c_usrid,l_sssn_id,c_retmsg);
    Fadd32(ptr_fml_Sbuffer,FML_ERR_MSG, c_retmsg, 0 );
    return(-1);
  }

  if(Fadd32(ptr_fml_Sbuffer, FML_USR_ID, (char *)c_usrid,0) == -1)
  {
    tpfree((char *)ptr_fml_Sbuffer);
    errlog(c_ServiceName,"L31010", FMLMSG, c_usrid , l_sssn_id, c_retmsg);
    return (-1);
   }

  if(Fadd32(ptr_fml_Sbuffer, FML_SSSN_ID, (char *)&l_sssn_id,0) == -1)
  {
    tpfree((char *)ptr_fml_Sbuffer);
    errlog(c_ServiceName,"L31015", FMLMSG, c_usrid , l_sssn_id, c_retmsg);
    return (-1);
  }
	if(Fadd32(ptr_fml_Sbuffer, FML_EBA_MTCH_ACT_NO, (char *)c_mtch_accnt_no,0) == -1)
  {
    tpfree((char *)ptr_fml_Sbuffer);
    errlog(c_ServiceName,"L31020", FMLMSG, c_usrid , l_sssn_id, c_retmsg);
    return (-1);
  }

  if(DEBUG_MSG_LVL_3)
  {
    userlog("Calling SVC_TERM_SSSN for match:%s:",c_mtch_accnt_no);
  }

  if(tpacall("SVC_TERM_SSSN", (char *)ptr_fml_Sbuffer, 0, TPNOREPLY | TPNOTRAN) ==  -1 )
  {
    tpfree((char *)ptr_fml_Sbuffer);
    errlog(c_ServiceName,"L31025", TPMSG, DEF_USR, l_sssn_id, c_retmsg);
    return (0);
  }
if(DEBUG_MSG_LVL_3)
  {
    userlog("After SVC_TERM_SSSN for match:%s:",c_mtch_accnt_no);
  }
	tpfree((char *)ptr_fml_Sbuffer);
  return 0;
}
/* ver 1.5 ends */

