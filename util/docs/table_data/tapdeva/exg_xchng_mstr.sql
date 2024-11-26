REM INSERTING into exg_xchng_mstr 
SET DEFINE OFF;
Insert into exg_xchng_mstr  (EXG_XCHNG_CD,EXG_XCHNG_NM,EXG_PR_OPNTM,EXG_OPN_TM,EXG_NXT_TRD_DT,EXG_MTCH_BNK_ACCNT,EXG_DPID_PL_ACCNT,EXG_DPCLNT_PL_ACCNT,EXG_CRRNT_STTS,EXG_CLS_TM,EXG_CCID,EXG_BRKR_ID,EXG_MKT_TYP,EXG_MIN_TRD_VAL,EXG_BRKR_STTS,EXG_EXRC_RQ_OPN_TM,EXG_EXRC_RQ_CLS_TM,EXG_EXRC_MKT_STTS,EXG_PYIN_PYOUT_BNK_ACCNT,EXG_MG_MNT_BNK_ACCNT,EXG_DSCLSD_PRCNTG,EXG_MAX_PSWD_VLD_DAYS,EXG_SETTLOR_ID,EXG_SETTLOR_STTS,EXG_GTD_VALID_DAYS,EXG_SEC_TM,EXG_PART_TM,EXG_INST_TM,EXG_IDX_TM,EXG_DEF_STLMNT_PD,EXG_WRN_PCNT,EXG_VOL_FRZ_PCNT,EXG_BRD_LOT_QTY,EXG_TCK_SZ,EXG_GTC_DYS,EXG_DSCLSD_QTY_PCNT,EXG_TRD_SPLT_DT,EXG_OFLN_NXT_TRD_DT,EXG_OFLN_STTS,EXG_BRKR_NAME,EXG_CTCL_ID,EXG_TM_DIFF,EXG_SETT_OFF_FLG,EXG_DL_DWNLD_ALLWD,EXG_EXTND_MRKT_STTS,EXG_VER_NO,EXG_PR_CLSTM,EXG_CLS_PRC_UPD_DT,EXG_CDSL_DPID_PL_ACCNT,EXG_CDSL_DPCLNT_PL_ACCNT,EXG_SP_PR_OPNTM,EXG_SP_PR_CLSTM,EXG_SP_PR_MKT_STTS,EXG_PRC_RNG_UPD_DT,EXG_TMP_TRD_DT,EXG_TMP_MKT_STTS,EXG_MKT_MAX_VAL,EXG_LMT_MAX_VAL,EXG_MAX_MRGNPLS_XCPTN_ALLWD,EXG_MAX_CUM_OPNORD_VAL,EXG_MCAS_MKT_STTS,EXG_CLS_STTS,EXG_NSDL_PL_DPID,EXG_MAXORDR_SKIP_CNT,EXG_CMIT_FAIL_CNT) values ('BFO','BSE - Exchange                ',915,915,to_timestamp('23-08-2024','DD-MM-YYYY'),'20082','1       ',null,'O',1530,null,'8051 ','D',500,'A',1000,1615,'C','405100961   ',null,100,14,null,'A',6,to_timestamp('22-08-2024','DD-MM-YYYY'),to_timestamp('22-08-2024','DD-MM-YYYY'),to_timestamp('22-08-2024','DD-MM-YYYY'),to_timestamp('22-08-2024','DD-MM-YYYY'),10,8224,8224,200,5,0,100,to_timestamp('03-03-2008','DD-MM-YYYY'),to_timestamp('25-06-2003','DD-MM-YYYY'),'C','IBSL','1.11111E+11',0,'N','N','X',null,908,null,'16014301','1024    ',0,0,null,null,to_timestamp('23-08-2024','DD-MM-YYYY'),'X',50000000,50000000,null,500000000,'X','N',null,null,3);
Insert into exg_xchng_mstr  (EXG_XCHNG_CD,EXG_XCHNG_NM,EXG_PR_OPNTM,EXG_OPN_TM,EXG_NXT_TRD_DT,EXG_MTCH_BNK_ACCNT,EXG_DPID_PL_ACCNT,EXG_DPCLNT_PL_ACCNT,EXG_CRRNT_STTS,EXG_CLS_TM,EXG_CCID,EXG_BRKR_ID,EXG_MKT_TYP,EXG_MIN_TRD_VAL,EXG_BRKR_STTS,EXG_EXRC_RQ_OPN_TM,EXG_EXRC_RQ_CLS_TM,EXG_EXRC_MKT_STTS,EXG_PYIN_PYOUT_BNK_ACCNT,EXG_MG_MNT_BNK_ACCNT,EXG_DSCLSD_PRCNTG,EXG_MAX_PSWD_VLD_DAYS,EXG_SETTLOR_ID,EXG_SETTLOR_STTS,EXG_GTD_VALID_DAYS,EXG_SEC_TM,EXG_PART_TM,EXG_INST_TM,EXG_IDX_TM,EXG_DEF_STLMNT_PD,EXG_WRN_PCNT,EXG_VOL_FRZ_PCNT,EXG_BRD_LOT_QTY,EXG_TCK_SZ,EXG_GTC_DYS,EXG_DSCLSD_QTY_PCNT,EXG_TRD_SPLT_DT,EXG_OFLN_NXT_TRD_DT,EXG_OFLN_STTS,EXG_BRKR_NAME,EXG_CTCL_ID,EXG_TM_DIFF,EXG_SETT_OFF_FLG,EXG_DL_DWNLD_ALLWD,EXG_EXTND_MRKT_STTS,EXG_VER_NO,EXG_PR_CLSTM,EXG_CLS_PRC_UPD_DT,EXG_CDSL_DPID_PL_ACCNT,EXG_CDSL_DPCLNT_PL_ACCNT,EXG_SP_PR_OPNTM,EXG_SP_PR_CLSTM,EXG_SP_PR_MKT_STTS,EXG_PRC_RNG_UPD_DT,EXG_TMP_TRD_DT,EXG_TMP_MKT_STTS,EXG_MKT_MAX_VAL,EXG_LMT_MAX_VAL,EXG_MAX_MRGNPLS_XCPTN_ALLWD,EXG_MAX_CUM_OPNORD_VAL,EXG_MCAS_MKT_STTS,EXG_CLS_STTS,EXG_NSDL_PL_DPID,EXG_MAXORDR_SKIP_CNT,EXG_CMIT_FAIL_CNT) values ('NDX','NSE Currency Derivatives      ',900,900,to_timestamp('28-06-2024','DD-MM-YYYY'),'CDS670','1       ',null,'C',2000,null,'7730 ','X',500,'A',1830,1700,'X','405070670   ',null,0,10,null,'A',0,to_timestamp('26-06-2024','DD-MM-YYYY'),to_timestamp('26-06-2024','DD-MM-YYYY'),to_timestamp('26-06-2024','DD-MM-YYYY'),to_timestamp('26-06-2024','DD-MM-YYYY'),60,0,0,983040,0,0,0,to_timestamp('05-08-2009','DD-MM-YYYY'),to_timestamp('05-08-2009','DD-MM-YYYY'),null,'ICICI SECURITIES LTD','1.11111E+11',0,'N','Y',null,'34900',908,null,'16014301','1024    ',0,0,null,null,null,null,420000000,420000000,null,2000000000,'X','N',null,null,3);
Insert into exg_xchng_mstr  (EXG_XCHNG_CD,EXG_XCHNG_NM,EXG_PR_OPNTM,EXG_OPN_TM,EXG_NXT_TRD_DT,EXG_MTCH_BNK_ACCNT,EXG_DPID_PL_ACCNT,EXG_DPCLNT_PL_ACCNT,EXG_CRRNT_STTS,EXG_CLS_TM,EXG_CCID,EXG_BRKR_ID,EXG_MKT_TYP,EXG_MIN_TRD_VAL,EXG_BRKR_STTS,EXG_EXRC_RQ_OPN_TM,EXG_EXRC_RQ_CLS_TM,EXG_EXRC_MKT_STTS,EXG_PYIN_PYOUT_BNK_ACCNT,EXG_MG_MNT_BNK_ACCNT,EXG_DSCLSD_PRCNTG,EXG_MAX_PSWD_VLD_DAYS,EXG_SETTLOR_ID,EXG_SETTLOR_STTS,EXG_GTD_VALID_DAYS,EXG_SEC_TM,EXG_PART_TM,EXG_INST_TM,EXG_IDX_TM,EXG_DEF_STLMNT_PD,EXG_WRN_PCNT,EXG_VOL_FRZ_PCNT,EXG_BRD_LOT_QTY,EXG_TCK_SZ,EXG_GTC_DYS,EXG_DSCLSD_QTY_PCNT,EXG_TRD_SPLT_DT,EXG_OFLN_NXT_TRD_DT,EXG_OFLN_STTS,EXG_BRKR_NAME,EXG_CTCL_ID,EXG_TM_DIFF,EXG_SETT_OFF_FLG,EXG_DL_DWNLD_ALLWD,EXG_EXTND_MRKT_STTS,EXG_VER_NO,EXG_PR_CLSTM,EXG_CLS_PRC_UPD_DT,EXG_CDSL_DPID_PL_ACCNT,EXG_CDSL_DPCLNT_PL_ACCNT,EXG_SP_PR_OPNTM,EXG_SP_PR_CLSTM,EXG_SP_PR_MKT_STTS,EXG_PRC_RNG_UPD_DT,EXG_TMP_TRD_DT,EXG_TMP_MKT_STTS,EXG_MKT_MAX_VAL,EXG_LMT_MAX_VAL,EXG_MAX_MRGNPLS_XCPTN_ALLWD,EXG_MAX_CUM_OPNORD_VAL,EXG_MCAS_MKT_STTS,EXG_CLS_STTS,EXG_NSDL_PL_DPID,EXG_MAXORDR_SKIP_CNT,EXG_CMIT_FAIL_CNT) values ('NCD','NCDEX                         ',955,1000,to_timestamp('28-06-2024','DD-MM-YYYY'),'2000000011','123456  ',null,'C',2355,null,'34   ','C',500,null,10,2345,'C','12345       ',null,10,365,'P0001','A',7,to_timestamp('23-12-2008','DD-MM-YYYY'),to_timestamp('23-12-2008','DD-MM-YYYY'),to_timestamp('23-12-2008','DD-MM-YYYY'),to_timestamp('23-12-2008','DD-MM-YYYY'),10,82.24,82.24,1,5,7,0.1,to_timestamp('19-02-2008','DD-MM-YYYY'),to_timestamp('19-02-2008','DD-MM-YYYY'),null,'ICICI Comm Trade Limited','13284',0.0000116,'N','Y',null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,'X','N',null,null,3);
Insert into exg_xchng_mstr  (EXG_XCHNG_CD,EXG_XCHNG_NM,EXG_PR_OPNTM,EXG_OPN_TM,EXG_NXT_TRD_DT,EXG_MTCH_BNK_ACCNT,EXG_DPID_PL_ACCNT,EXG_DPCLNT_PL_ACCNT,EXG_CRRNT_STTS,EXG_CLS_TM,EXG_CCID,EXG_BRKR_ID,EXG_MKT_TYP,EXG_MIN_TRD_VAL,EXG_BRKR_STTS,EXG_EXRC_RQ_OPN_TM,EXG_EXRC_RQ_CLS_TM,EXG_EXRC_MKT_STTS,EXG_PYIN_PYOUT_BNK_ACCNT,EXG_MG_MNT_BNK_ACCNT,EXG_DSCLSD_PRCNTG,EXG_MAX_PSWD_VLD_DAYS,EXG_SETTLOR_ID,EXG_SETTLOR_STTS,EXG_GTD_VALID_DAYS,EXG_SEC_TM,EXG_PART_TM,EXG_INST_TM,EXG_IDX_TM,EXG_DEF_STLMNT_PD,EXG_WRN_PCNT,EXG_VOL_FRZ_PCNT,EXG_BRD_LOT_QTY,EXG_TCK_SZ,EXG_GTC_DYS,EXG_DSCLSD_QTY_PCNT,EXG_TRD_SPLT_DT,EXG_OFLN_NXT_TRD_DT,EXG_OFLN_STTS,EXG_BRKR_NAME,EXG_CTCL_ID,EXG_TM_DIFF,EXG_SETT_OFF_FLG,EXG_DL_DWNLD_ALLWD,EXG_EXTND_MRKT_STTS,EXG_VER_NO,EXG_PR_CLSTM,EXG_CLS_PRC_UPD_DT,EXG_CDSL_DPID_PL_ACCNT,EXG_CDSL_DPCLNT_PL_ACCNT,EXG_SP_PR_OPNTM,EXG_SP_PR_CLSTM,EXG_SP_PR_MKT_STTS,EXG_PRC_RNG_UPD_DT,EXG_TMP_TRD_DT,EXG_TMP_MKT_STTS,EXG_MKT_MAX_VAL,EXG_LMT_MAX_VAL,EXG_MAX_MRGNPLS_XCPTN_ALLWD,EXG_MAX_CUM_OPNORD_VAL,EXG_MCAS_MKT_STTS,EXG_CLS_STTS,EXG_NSDL_PL_DPID,EXG_MAXORDR_SKIP_CNT,EXG_CMIT_FAIL_CNT) values ('MCO','MCX Derivative exchange       ',900,900,to_timestamp('24-08-2024','DD-MM-YYYY'),'20086','10      ',null,'O',2330,null,'72250','C',500,'A',956,2330,'O','405117015   ',null,25,90,null,'A',0,to_timestamp('21-08-2024','DD-MM-YYYY'),to_timestamp('21-08-2024','DD-MM-YYYY'),to_timestamp('21-08-2024','DD-MM-YYYY'),to_timestamp('21-08-2024','DD-MM-YYYY'),10,8224,8224,200,5,0,25,to_timestamp('01-06-2020','DD-MM-YYYY'),to_timestamp('14-10-2002','DD-MM-YYYY'),'C','IBSL','1.11111E+11',0,'N','N','X',null,908,null,'16014301','1024    ',0,0,null,null,to_timestamp('22-08-2024','DD-MM-YYYY'),'X',100000000000000,10000000000000,null,10000000000000,null,'N',null,null,3);
Insert into exg_xchng_mstr  (EXG_XCHNG_CD,EXG_XCHNG_NM,EXG_PR_OPNTM,EXG_OPN_TM,EXG_NXT_TRD_DT,EXG_MTCH_BNK_ACCNT,EXG_DPID_PL_ACCNT,EXG_DPCLNT_PL_ACCNT,EXG_CRRNT_STTS,EXG_CLS_TM,EXG_CCID,EXG_BRKR_ID,EXG_MKT_TYP,EXG_MIN_TRD_VAL,EXG_BRKR_STTS,EXG_EXRC_RQ_OPN_TM,EXG_EXRC_RQ_CLS_TM,EXG_EXRC_MKT_STTS,EXG_PYIN_PYOUT_BNK_ACCNT,EXG_MG_MNT_BNK_ACCNT,EXG_DSCLSD_PRCNTG,EXG_MAX_PSWD_VLD_DAYS,EXG_SETTLOR_ID,EXG_SETTLOR_STTS,EXG_GTD_VALID_DAYS,EXG_SEC_TM,EXG_PART_TM,EXG_INST_TM,EXG_IDX_TM,EXG_DEF_STLMNT_PD,EXG_WRN_PCNT,EXG_VOL_FRZ_PCNT,EXG_BRD_LOT_QTY,EXG_TCK_SZ,EXG_GTC_DYS,EXG_DSCLSD_QTY_PCNT,EXG_TRD_SPLT_DT,EXG_OFLN_NXT_TRD_DT,EXG_OFLN_STTS,EXG_BRKR_NAME,EXG_CTCL_ID,EXG_TM_DIFF,EXG_SETT_OFF_FLG,EXG_DL_DWNLD_ALLWD,EXG_EXTND_MRKT_STTS,EXG_VER_NO,EXG_PR_CLSTM,EXG_CLS_PRC_UPD_DT,EXG_CDSL_DPID_PL_ACCNT,EXG_CDSL_DPCLNT_PL_ACCNT,EXG_SP_PR_OPNTM,EXG_SP_PR_CLSTM,EXG_SP_PR_MKT_STTS,EXG_PRC_RNG_UPD_DT,EXG_TMP_TRD_DT,EXG_TMP_MKT_STTS,EXG_MKT_MAX_VAL,EXG_LMT_MAX_VAL,EXG_MAX_MRGNPLS_XCPTN_ALLWD,EXG_MAX_CUM_OPNORD_VAL,EXG_MCAS_MKT_STTS,EXG_CLS_STTS,EXG_NSDL_PL_DPID,EXG_MAXORDR_SKIP_CNT,EXG_CMIT_FAIL_CNT) values ('NFO','National Stock Exchange       ',915,915,to_timestamp('23-08-2024','DD-MM-YYYY'),'20086','1       ',null,'O',2359,null,'7730 ','D',500,'A',1000,1615,'C','405008178   ',null,100,10,null,'A',6,to_timestamp('22-08-2024','DD-MM-YYYY'),to_timestamp('22-08-2024','DD-MM-YYYY'),to_timestamp('22-08-2024','DD-MM-YYYY'),to_timestamp('22-08-2024','DD-MM-YYYY'),10,8224,8224,200,5,0,100,to_timestamp('03-03-2008','DD-MM-YYYY'),to_timestamp('25-06-2003','DD-MM-YYYY'),'C','IBSL','1.11111E+11',0,'N','N','X','16100',908,null,'16014301','1024    ',0,0,null,null,to_timestamp('23-08-2024','DD-MM-YYYY'),'O',5000000000,1000000000,null,150000000,'X','N',null,null,3);




INSERT INTO exg_xchng_mstr (
    EXG_XCHNG_CD, EXG_XCHNG_NM, EXG_PR_OPNTM, EXG_OPN_TM, EXG_NXT_TRD_DT, 
    EXG_MTCH_BNK_ACCNT, EXG_DPID_PL_ACCNT, EXG_DPCLNT_PL_ACCNT, EXG_CRRNT_STTS, 
    EXG_CLS_TM, EXG_CCID, EXG_BRKR_ID, EXG_MKT_TYP, EXG_MIN_TRD_VAL, 
    EXG_BRKR_STTS, EXG_EXRC_RQ_OPN_TM, EXG_EXRC_RQ_CLS_TM, EXG_EXRC_MKT_STTS, 
    EXG_PYIN_PYOUT_BNK_ACCNT, EXG_MG_MNT_BNK_ACCNT, EXG_DSCLSD_PRCNTG, 
    EXG_MAX_PSWD_VLD_DAYS, EXG_SETTLOR_ID, EXG_SETTLOR_STTS, EXG_GTD_VALID_DAYS, 
    EXG_SEC_TM, EXG_PART_TM, EXG_INST_TM, EXG_IDX_TM, EXG_DEF_STLMNT_PD, 
    EXG_WRN_PCNT, EXG_VOL_FRZ_PCNT, EXG_BRD_LOT_QTY, EXG_TCK_SZ, EXG_GTC_DYS, 
    EXG_DSCLSD_QTY_PCNT, EXG_TRD_SPLT_DT, EXG_OFLN_NXT_TRD_DT, EXG_OFLN_STTS, 
    EXG_BRKR_NAME, EXG_CTCL_ID, EXG_TM_DIFF, EXG_SETT_OFF_FLG, EXG_DL_DWNLD_ALLWD, 
    EXG_EXTND_MRKT_STTS, EXG_VER_NO, EXG_PR_CLSTM, EXG_CLS_PRC_UPD_DT, 
    EXG_CDSL_DPID_PL_ACCNT, EXG_CDSL_DPCLNT_PL_ACCNT, EXG_SP_PR_OPNTM, 
    EXG_SP_PR_CLSTM, EXG_SP_PR_MKT_STTS, EXG_PRC_RNG_UPD_DT, EXG_TMP_TRD_DT, 
    EXG_TMP_MKT_STTS, EXG_MKT_MAX_VAL, EXG_LMT_MAX_VAL, EXG_MAX_MRGNPLS_XCPTN_ALLWD, 
    EXG_MAX_CUM_OPNORD_VAL, EXG_MCAS_MKT_STTS, EXG_CLS_STTS, EXG_NSDL_PL_DPID, 
    EXG_MAXORDR_SKIP_CNT, EXG_CMIT_FAIL_CNT
) 
VALUES (
    'BFO', 'BSE - Exchange                ', 915, 915, 
    '2024-08-23 00:00:00'::timestamp(0) without time zone, 
    '20082', '1       ', NULL, 'O', 1530, NULL, '8051 ', 
    'D', 500, 'A', 1000, 1615, 'C', '405100961   ', NULL, 
    100, 14, NULL, 'A', 6, 
    '2024-08-22 00:00:00'::timestamp(0) without time zone, 
    '2024-08-22 00:00:00'::timestamp(0) without time zone, 
    '2024-08-22 00:00:00'::timestamp(0) without time zone, 
    '2024-08-22 00:00:00'::timestamp(0) without time zone, 
    10, 8224, 8224, 200, 5, 0, 100, 
    '2008-03-03 00:00:00'::timestamp(0) without time zone, 
    '2003-06-25 00:00:00'::timestamp(0) without time zone, 
    'C', 'IBSL', '1.11111E+11', 0, 'N', 'N', 'X', NULL, 908, 
    NULL, '16014301', '1024    ', 0, 0, NULL, NULL, 
    '2024-08-23 00:00:00'::timestamp(0) without time zone, 
    'X', 50000000, 50000000, NULL, 500000000, 'X', 'N', NULL, NULL, 3
);
