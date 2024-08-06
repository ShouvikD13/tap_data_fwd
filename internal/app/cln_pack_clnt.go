package app

import (
	"DATA_FWD_TAP/models/structures"
	"log"

	"gorm.io/gorm"
)

// var sql_pipe_id [3]byte
// var sql_c_xchng_cd [4]byte

func Fn_bat_init(args []string, Db *gorm.DB) int {
	log.Println("Entering fn_bat_init")

	exchngbook := &structures.Vw_xchngbook{}
	arg := args[3]
	copy(exchngbook.C_pipe_id[:], arg)

	log.Printf("Copying pipe ID: %s", exchngbook.C_pipe_id[:])

	// in original file we are storing the "exchange code in a global variable but here we are directly storing it in the 'exchngbook.C_xchng_cd' "
	query := `Select opm_xchng_cd
				From opm_ord_pipe_mstr
				where opm_pipe_id = ?`

	row := Db.Raw(query, exchngbook.C_pipe_id).Row()

	log.Println("Executing query to fetch exchange code")

	err := row.Scan(&exchngbook.C_xchng_cd)
	if err != nil {
		log.Println("Error scanning row:", err)
		return -1
	}

	log.Printf("Exchange code fetched: %s", exchngbook.C_xchng_cd)
	log.Println("Exiting fn_bat_init")

	CLN_PACK_CLNT(args, Db)
	return 0
}

func CLN_PACK_CLNT(args []string, Db *gorm.DB) {
	log.Println("Entering CLN_PACK_CLNT")
	fnGetNxtRec(args, Db)
	log.Println("Exiting CLN_PACK_CLNT")
}

func fnGetNxtRec(args []string, Db *gorm.DB) {
	log.Println("Entering fnGetNxtCnt")

	exchngbook := &structures.Vw_xchngbook{}
	fnSeqToOmd(Db, exchngbook)

	log.Println("Exiting fnGetNxtCnt")
}

func fnSeqToOmd(db *gorm.DB, xchngbook *structures.Vw_xchngbook) int {
	log.Println("Entering fnSeqToOmd")
	log.Println("Before extracting the data from the 'fxb_ordr_rfrnc' and storing it in the 'xchngbook' structure")

	query := `
		SELECT fxb_ordr_rfrnc,
			fxb_lmt_mrkt_sl_flg,
			fxb_dsclsd_qty,
			fxb_ordr_tot_qty,
			fxb_lmt_rt,
			fxb_stp_lss_tgr,
			to_char(fxb_ordr_valid_dt, 'dd-mon-yyyy') as valid_dt,
			DECODE(fxb_ordr_type, 'V', 'T', fxb_ordr_type) as ord_typ,
			fxb_rqst_typ,
			fxb_ordr_sqnc
			FROM FXB_FO_XCHNG_BOOK
			WHERE 
			fxb_xchng_cd = ?
			AND fxb_pipe_id = ?
			AND fxb_mod_trd_dt = to_date(?, 'dd-Mon-yyyy')
			AND fxb_ordr_sqnc = (
				SELECT min(fxb_b.fxb_ordr_sqnc)
				FROM FXB_FO_XCHNG_BOOK fxb_b
				WHERE fxb_b.fxb_xchng_cd = ?
					AND fxb_b.fxb_mod_trd_dt = to_date(?, 'dd-Mon-yyyy')
					AND fxb_b.fxb_pipe_id = ?
					AND fxb_b.fxb_plcd_stts = 'R'
			)
	`

	log.Println("Executing query to fetch order details")

	row := db.Raw(query, xchngbook.C_xchng_cd, xchngbook.C_pipe_id[:], xchngbook.C_mod_trd_dt[:], xchngbook.L_ord_seq, xchngbook.C_pipe_id[:], xchngbook.C_mod_trd_dt[:], xchngbook.L_ord_seq).Row()

	/* xchngbook.C_xchng_cd
	   The value of 'xchngbook.C_xchng_cd' is obtained from a query in 'fn_bat_init':

	   'EXEC SQL
	   SELECT opm_xchng_cd,
	          opm_max_pnd_ord,
	          opm_stream_no
	   INTO   :sql_c_xchng_cd,
	          :sql_li_max_pnd_ord,
	          :i_stream_count_opm
	   FROM   opm_ord_pipe_mstr
	   WHERE  opm_pipe_id = :sql_c_pipe_id;'
	*/

	/* C_pipe_id

	   The value of this is obtained from args[3] and is initialized in 'fn_bat_init'.
	*/

	/* "C_mod_trd_dt"

	   The value of "C_mod_trd_dt" is obtained from a query in "fn_bat_init" and then set to 'null'. The logic for this is unclear.
	   [
	       SELECT exg_nxt_trd_dt,
	              exg_brkr_id,
	              exg_ctcl_id
	       INTO   :sql_c_nxt_trd_date,
	              :st_opm_mstr.c_xchng_brkr_id,
	              :sql_c_xchng_ctcl_id
	       FROM   exg_xchng_mstr
	       WHERE  exg_xchng_cd = :sql_c_xchng_cd;
	   ]
	*/

	/* "L_ord_seq"

	   It is set to 0 in fn_bat_init.

	   st_rqst1.l_ord_seq = li_seq_nm;  // Set to '0' in fn_bat_init.
	*/

	err := row.Scan(
		&xchngbook.C_slm_flg,
		&xchngbook.L_dsclsd_qty,
		&xchngbook.L_ord_tot_qty,
		&xchngbook.L_ord_lmt_rt,
		&xchngbook.L_stp_lss_tgr,
		&xchngbook.C_valid_dt,
		&xchngbook.C_ord_typ,
		&xchngbook.C_req_typ,
		&xchngbook.L_ord_seq,
	)

	if err != nil {
		log.Println("Error scanning row:", err)
		return -1
	}

	log.Printf("Data extracted and stored in the 'xchngbook' structure:")
	log.Printf("  C_slm_flg: %c", xchngbook.C_slm_flg)
	log.Printf("  L_dsclsd_qty: %d", xchngbook.L_dsclsd_qty)
	log.Printf("  L_ord_tot_qty: %d", xchngbook.L_ord_tot_qty)
	log.Printf("  L_ord_lmt_rt: %d", xchngbook.L_ord_lmt_rt)
	log.Printf("  L_stp_lss_tgr: %d", xchngbook.L_stp_lss_tgr)
	log.Printf("  C_valid_dt: %s", xchngbook.C_valid_dt)
	log.Printf("  C_ord_typ: %c", xchngbook.C_ord_typ)
	log.Printf("  C_req_typ: %c", xchngbook.C_req_typ)
	log.Printf("  L_ord_seq: %d", xchngbook.L_ord_seq)

	log.Println("Data extracted and stored in the 'xchngbook' structure successfully")
	log.Println("Exiting fnSeqToOmd")
	return 0
}
