/* hardware.h -- January 9, 1990 */

typedef enum { HS_OFFLINE, HS_ERROR, HS_CONNECT } HS_STATE;

#define R_NODE_NUMBER     1300
#define R_BAUD_RATE       1302
#define R_MASTER_CODE_R   1304
#define R_EOC_MAX_FLOW    1306
#define R_EOC_DSTAGE      1308
#define R_DSTAGE_VAL_01   1310
#define R_DSTAGE_TIM_01   1312
#define R_DSTAGE_VAL_02   1314
#define R_DSTAGE_TIM_02   1316
#define R_DSTAGE_VAL_03   1318
#define R_DSTAGE_TIM_03   1320
#define R_DSTAGE_VAL_04   1322
#define R_DSTAGE_TIM_04   1324
#define R_DSTAGE_VAL_05   1326
#define R_DSTAGE_TIM_05   1328
#define R_PID_LOOP_NUM    1330
#define R_USER_CODE_R     1372
#define R_DEFAULT_ACCEP_R 1374
#define R_NUM_PUMPS       1376
#define R_1ST_STAGE_TIM   1378
#define R_2ND_STAGE_TIM   1380
#define R_3RD_STAGE_TIM   1382
#define R_4TH_STAGE_TIM   1384
#define R_5TH_STAGE_TIM   1386
#define R_AUTO_ALT_TIM    1388
#define R_PROPORTIONAL    1392
#define R_INTEGRAL        1394
#define R_DERIVATIVE      1396
#define R_PV_UNITS        1398
#define R_PV_TRANS_SPAN   1400
#define R_FLOW_TRANS_SPAN 1402
#define R_KW_TRANS_SPAN   1404
#define R_NUM_ZONES       1406
#define R_BYPASS_ENABLED  1408
#define R_ZONE_01_SP      1540
#define R_MANUAL_SPEED    1572
#define R_FKEY_STATUS_R   1616
#define R_KPAD_FLAG_R     1618
#define R_PV_01           1638
#define R_GPM_01_VAL      1670
#define R_KW_01_VAL       1678
#define R_SPEED_VALUE     1690
#define R_MASTER_CODE_W   1692
#define R_ACTIVE_ZONE     1694
#define R_USER_CODE_W     1696
#define R_ACTIVE_SP       1698
#define R_ACTIVE_PV       1700
#define R_DEFAULT_ACCEP_W 1702
#define R_FKEY_STATUS_W   1730
#define R_KPAD_FLAG_W     1732
#define R_ALT_SEQ_W       1734
#define R_LEDA_STATUS     1736
#define R_LEDB_STATUS     1738
#define R_LEDC_STATUS     1740
#define R_LEDD_STATUS     1742
#define R_PUMP_FAILURE    1744
#define R_ZONE_FAILURE    1748
#define R_XMIT_CALIBRATE  1750
#define R_FLOW_KW_FAILURE 1752

void     HardwareCleanUp(void);
void     HardwareCommunicate(void);
void     HardwareDoMenu(short item);
void     HardwareInit(short port);
void     HardwareProcessChar(int ch);
WORD     HardwareReadRAM(WORD adr, WORD offset);
void     HardwareRegsAdd(void);
void     HardwareRegsDelete(void);
void     HardwareRegsModify(int digit);
HS_STATE HardwareSerLink(void);
void     HardwareUpdateMenu(void);
short    HardwareWriteRAM(WORD adr, WORD offset, WORD val);
