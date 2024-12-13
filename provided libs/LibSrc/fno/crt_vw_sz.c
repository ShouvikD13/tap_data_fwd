#include <stdio.h>
#include <fo_view_def.h>

main()
{
	char szvw[32];

	printf ( "[SFO_FTOV]\n" );
	printf ( "vw_contract=%d\n", sizeof ( struct vw_contract ) );
	printf ( "vw_cntrct_qt=%d\n", sizeof ( struct vw_cntrct_qt ) );
	printf ( "vw_cntrctlong_qt=%d\n", sizeof ( struct vw_cntrctlong_qt ) );
	printf ( "vw_cntrt_gen_inf=%d\n", sizeof ( struct vw_cntrt_gen_inf ) );
	printf ( "vw_indx_qt=%d\n", sizeof ( struct vw_indx_qt ) );
	printf ( "vw_undrlyng=%d\n", sizeof ( struct vw_undrlyng ) );
	printf ( "vw_nse_cntrct=%d\n", sizeof ( struct vw_nse_cntrct ) );
	printf ( "vw_mtch_cntrct=%d\n", sizeof ( struct vw_mtch_cntrct ) );
	printf ( "vw_err_msg=%d\n", sizeof ( struct vw_err_msg ) );
	printf ( "vw_usr_prfl=%d\n", sizeof ( struct vw_usr_prfl ) );
	printf ( "vw_order=%d\n", sizeof ( struct vw_order ) );
	printf ( "vw_orderbook=%d\n", sizeof ( struct vw_orderbook ) );
	printf ( "vw_xchngbook=%d\n", sizeof ( struct vw_xchngbook ) );
	printf ( "vw_exrc_req=%d\n", sizeof ( struct vw_exrc_req ) );
	printf ( "vw_exrcbook=%d\n", sizeof ( struct vw_exrcbook ) );
	printf ( "vw_tradebook=%d\n", sizeof ( struct vw_tradebook ) );
	printf ( "vw_sequence=%d\n", sizeof ( struct vw_sequence ) );
	printf ( "vw_asgnmt=%d\n", sizeof ( struct vw_asgnmt ) );
	printf ( "vw_usr_dtls=%d\n", sizeof ( struct vw_usr_dtls ) );
	printf ( "vw_pstn_actn=%d\n", sizeof ( struct vw_pstn_actn ) );
	printf ( "vw_xchngstts=%d\n", sizeof ( struct vw_xchngstts ) );
	printf ( "vw_upd_limits=%d\n", sizeof ( struct vw_upd_limits ) );
	printf ( "vw_gt_lmt_dtls=%d\n", sizeof ( struct vw_gt_lmt_dtls ) );
	printf ( "vw_mkt_msg=%d\n", sizeof ( struct vw_mkt_msg ) );
	printf ( "vw_fut_pos=%d\n", sizeof ( struct vw_fut_pos ) );
	printf ( "vw_cntfut_pos=%d\n", sizeof ( struct vw_cntfut_pos ) );
	printf ( "vw_undfut_pos=%d\n", sizeof ( struct vw_undfut_pos ) );
	printf ( "vw_cntopt_pos=%d\n", sizeof ( struct vw_cntopt_pos ) );
	printf ( "vw_spdordbk=%d\n", sizeof ( struct vw_spdordbk ) );
	printf ( "vw_spdxchbk=%d\n", sizeof ( struct vw_spdxchbk ) );
}
