# include <stdio.h>
# include <fo.h>
# include <fn_ddr.h>
# include <string.h>	/* Ver 2.1 */
/***************************************************************************/
/******Version 2.0  12-Jun-2007 by Smitha Nuti for new UBB	***************/
/******Version 2.1  13-dec-2007 by ROhit For AIX Migration	***************/
/***************************************************************************/

char  c_ddr_str [ 4 ];

void fn_init_ddr_pop ( char *c_pipe_id,
                       char *c_origin,
                       char *c_prdct_typ )
{
	/***************************************/
	/**Version 2.0 for new Routing pattern**/
	/***************************************/
	if(c_pipe_id[0] == '1')
	{
		c_ddr_str[0] = c_pipe_id[1];
		c_ddr_str[1] = c_pipe_id[1];
		c_ddr_str[2] = c_pipe_id[1];
		c_ddr_str[3] = '\0';
	}
	else
	{
		strcpy(c_ddr_str,"000");
	}
}

void fn_init_ddr_val ( char *c_ip_ddr_str )
{
	strcpy ( c_ddr_str, c_ip_ddr_str );
	return;
}

void fn_cpy_ddr ( char *c_ddr_var )
{
	strcpy ( c_ddr_var, c_ddr_str );
	return;
}
