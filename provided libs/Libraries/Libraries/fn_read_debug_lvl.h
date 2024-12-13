#define INITDBGLVL(A)  static int si_debug_flg = -1;if(si_debug_flg < 0) si_debug_flg =fn_get_debug_flg(A);gi_debug_flg=si_debug_flg
#define INITBATDBGLVL(A)  fn_read_debug_flg(A);gi_debug_flg =fn_get_debug_flg(A)
#define DEBUG_MSG_LVL_0   (gi_debug_flg >= 0)
#define DEBUG_MSG_LVL_1   (gi_debug_flg >= 1)
#define DEBUG_MSG_LVL_2   (gi_debug_flg >= 2)
#define DEBUG_MSG_LVL_3   (gi_debug_flg >= 3)
#define DEBUG_MSG_LVL_4   (gi_debug_flg >= 4)
#define DEBUG_MSG_LVL_5   (gi_debug_flg >= 5)

extern int gi_debug_flg;

int fn_get_debug_flg(char * cptr_servicename);
int fn_read_debug_flg(char * cptr_prcs_name);

void fn_get_channel(char usr_typ, char *channel); /* Ver 1.1 by Mihir */

int fn_simaltns_ssn_term(char *c_ServiceName, char *c_usrid, long l_sssn_id,char *c_mtch_accnt_no,char *c_retmsg );

