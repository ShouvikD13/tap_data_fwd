/************************************************/
/*	log		:		Ver 1.1		IBM Migration		Rohit		*/
/*        :   Ver 1.2   Implementation of new functions for begin and commit        */
/*                      transaction with no log printing (24-Feb-2014)(Sachin Birje)*/
/*        :   Ver 1.3   New function introduced (04Sep14) (Sachin Birje)            */
/************************************************************************************/
int fn_call_svc ( char *c_ServiceName,
                  char *c_err_msg,
                  void *st_input,
                  void *st_output,
                  char *c_inp_view,
                  char *c_out_view,
                 /* int  i_ip_len, */
									long l_ip_len,		/* Ver 1.1 */
                 /* int  i_op_len, */	/* Ver 1.1 */
									long l_op_len,
                  long int li_flg,
                  char *c_svcnm );

int fn_acall_svc( char *c_ServiceName,
                  char *c_err_msg,
                  void *st_input,
                  char *c_inp_view,
                  int  i_ip_len,
                  long int li_flg,
                  char *c_svcnm );

int fn_agetrep_svc (  char *c_ServiceName,
                      char *c_err_msg,
                      void *st_output,
                      char *c_out_view,
                      int  i_op_len,
                      long int li_flg,
                      int  *ptr_i_cd );

int fn_call_svc_fml ( char *c_ServiceName,
                      char *c_err_msg,
                      char *c_svc_nam,
                      long int li_flag,
                      int i_tot_ifields,
                      int i_tot_ofields,
                      ... );

int fn_acall_svc_fml ( char *c_ServiceName,
                       char *c_err_msg,
                       char *c_svc_nam,
                       long int li_flag,
                       int i_tot_ifields,
                       ... );

int fn_agetrep_svc_fml ( char *c_ServiceName,
                         char *c_err_msg,
                         int *ptr_i_cd,
                         long int li_flag,
                         int i_tot_ofields,
                         ... );

int fn_unpack_fmltostruct ( char *c_ServiceName,
                            char *c_err_msg,
                            FBFR32 *ptr_fml_ibuf,
                            void *st_input,
                            char *c_inp_view );

int fn_unpack_fmltovar ( char *c_ServiceName,
                         char *c_err_msg,
                         FBFR32 *ptr_fml_ibuf,
                         int i_tot_ifields,
                         ... );

int fn_unpack_fmltovar_dflt ( char *c_ServiceName,
                         char *c_err_msg,
                         FBFR32 *ptr_fml_ibuf,
                         int i_tot_ifields,
                         ... );                         /** Ver 1.3 **/

int fn_pack_structtofml ( char *c_ServiceName,
                          char *c_err_msg,
                          FBFR32 **ptr_fml_obuf,
                          void *st_output,
                          char *c_out_view,
                          int i_flg );

int fn_pack_vartofml ( char *c_ServiceName,
                       char *c_err_msg,
                       FBFR32 **ptr_fml_obuf,
                       int i_tot_ofields,
                       ... );

FBFR32* fn_create_rs ( char *c_ServiceName,
                       char *c_err_msg,
                       char *c_svc_nam,
                       long int li_flag,
                       int i_tot_ifields,
                       ... );

void fn_rewind_rs ( FBFR32* ptr_fml_buf );

int fn_reccnt_rs ( FBFR32* ptr_fml_buf,
                   char *c_ServiceName,
                   char *c_err_msg,
                   FLDID32 l_cnt_fml );

int fn_getnxt_rs ( FBFR32* ptr_fml_buf,
                   char *c_ServiceName,
                   char *c_err_msg,
                   int i_tot_ifields,
                    ... );

void fn_dstry_rs ( FBFR32* ptr_fml_buf );

void fn_bat_pst_msg_fno ( char *c_pgm_nm,
                      char *c_msg,
                      char *c_tag );

void fn_set_msghnd ( void );

void fn_pst_trg ( char *c_servicename,
                  char *c_trg_name,
                  char *c_msg,
                  char *filter );

int fn_chk_trg ( void );

extern char c_trg_msg [ 256 ];

#define LOCAL_TRNSCTN   1 
#define REMOTE_TRNSCTN	2
#define TRAN_TIMEOUT     300

int fn_begintran ( char *c_servicename, 
                   char *c_err_msg );
int fn_committran ( char *c_servicename, 
                    int i_trnsctn, 
                    char *c_err_msg );
int fn_aborttran ( char *c_servicename, 
                   int i_trnsctn, 
                   char *c_err_msg );

double fn_maxd( double d_val1, double d_val2 );

double fn_mind( double d_val1, double d_val2 );

long int fn_maxl( long int li_val1, long int li_val2 );

long int fn_minl( long int li_val1, long int li_val2 );

#define SYSTEM_ERROR -100

/***	Ver 1.1	:	Addition Starts	***/
int fn_long_to_int( long l_input, int *i_out, char *c_err_msg );

int fn_unlong_to_unint( unsigned long ul_input, unsigned int *ui_out, char *c_err_msg );

/***  Ver 1.1 : Addition Ends	***/

/*** Ver 1.2 Starts Here ***/

int fn_nolog_begintran ( char *c_servicename,
                         char *c_err_msg );
int fn_nolog_committran ( char *c_servicename,
                          int i_trnsctn,
                          char *c_err_msg );

/*** ver 1.2 Ends Here ***/
