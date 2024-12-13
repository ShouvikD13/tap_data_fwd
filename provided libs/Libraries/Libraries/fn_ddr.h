#define	NFO_1_PIPE	"11"
#define	NFO_2_PIPE	"12" /* Changed by Mohit for Multiple pipe. */
#define	TRADING_SECTION	"TS"
#define	PORTFOLIO_SECTION	"PS"
#define	BACKOFFICE_SECTION	"BS"
#define	F_CS	"F_CS"
#define	O_CS	"O_CS"
#define	COMMON	"0"

void fn_init_ddr_pop ( char *c_pipe_id,
                       char *c_origin,
                       char *c_prdct_typ );

void fn_init_ddr_val ( char *c_ip_ddr_str );

void fn_cpy_ddr ( char *c_ddr_var );
