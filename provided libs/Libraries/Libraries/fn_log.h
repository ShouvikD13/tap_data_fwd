# define MSG_LEN 256

int fn_userlog( char *c_servicename, char *c_frmt_str, ... );

void fn_errlog ( char *c_servicename,
                char *c_errno,
                char *sys_msg,
                char *c_errmsg );

/*int fn_create_pipe();  Changed by Hemanshu for IBM ISO compliance*/
int fn_create_pipe(void);

/*void fn_destroy_pipe(); Changed by Hemanshu for IBM ISO compliance*/
void fn_destroy_pipe(void);

extern void rtrim(char *);

extern void errlog ( char *c_servicename, char *c_errno, char *sys_msg, char *c_usrid,
              long lisssn, char *ptr_c_errmsg);

void fn_log_tm ( char *c_servicename,
                 char *c_tmr_no        );
void fn_tm_stmp_ml();
