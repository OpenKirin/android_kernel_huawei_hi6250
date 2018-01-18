/******************************************************************************

  Copyright(C)2008,Hisilicon Co. LTD.

******************************************************************************
File Name       : ChrEventIDComm.h
Description     : ChrEventIDComm.h header file,����ֱͨ�¼�ID�ڴ˶��塣
History         :
   1.q00312157      2016-5-25     Draft Enact

******************************************************************************/

#ifndef __ERRLOGCOMM_H__
#define __ERRLOGCOMM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
1 Include Headfile
*****************************************************************************/
#include  "product_config.h"
#include  "vos.h"

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif

/*****************************************************************************
2 macro
*****************************************************************************/

/*DEFINE MODEM ALARMID BASE BEGIN*/
/* ֱͨ�¼�ö�ٷֶΣ�XML����1���ֽڣ���Ч��Χ 0~127����128���� */
#define COMM_EventID_ENUM_BASE                              (0)
/* ֱͨ�¼���Ϊ�����¼���ǹ����¼��������¼�ͳһ���Σ��ǹ����¼���������䡣 */
/* ֱͨ�����¼���0~53����54���� */
#define COMM_RELATION_EVENTID_ENUM_BASE                     (COMM_EventID_ENUM_BASE+0)
/* ֱͨ�ǹ����¼���54~127����74���� */
#define COMM_ABSOLUTE_EVENTID_ENUM_BASE                     (COMM_EventID_ENUM_BASE+54)
/* ֱͨ�ǹ����¼�������ֶ���ʼ����, ����PS COM :10, TLPS:20(TLRRC:12, TL2:3, LNAS:5), GUC:24, PHY:20 */
/* �����ǹ���ֱͨЭ��ջ�����¼����¼�ID��ԭ�����¼���ȡ10����ԭ�ǹ����¼��������ά�ֲ��䡣 */
#define COMM_PSCOM_EVENTID_ENUM_BASE                        (COMM_ABSOLUTE_EVENTID_ENUM_BASE+0)
#define COMM_TLRRC_EVENTID_ENUM_BASE                        (COMM_ABSOLUTE_EVENTID_ENUM_BASE+10)
#define COMM_TL2_EVENTID_ENUM_BASE                          (COMM_ABSOLUTE_EVENTID_ENUM_BASE+22)
#define COMM_LNAS_EVENTID_ENUM_BASE                         (COMM_ABSOLUTE_EVENTID_ENUM_BASE+25)
#define COMM_GUC_EVENTID_ENUM_BASE                          (COMM_ABSOLUTE_EVENTID_ENUM_BASE+30)
#define COMM_PHY_EVENTID_ENUM_BASE                          (COMM_ABSOLUTE_EVENTID_ENUM_BASE+54)



/* ����ģ���ֱͨ���¼�ö�ٷֶ� */
#define LNAS_SUBEVENTID_ENUM_BASE                           (11000)
#define GUCNAS_SUBEVENTID_ENUM_BASE                         (12000) /* GUC����ģ��ʹ�� */

#define CAS_SUBEVENTID_ENUM_BASE                            (13000)

#define LRRC_SUBEVENTID_ENUM_BASE                           (21000)
#define LL2_SUBEVENTID_ENUM_BASE                            (22000)
#define LPHY_SUBEVENTID_ENUM_BASE                           (23000)

#define TRRC_SUBEVENTID_ENUM_BASE                           (24000)
#define TL2_SUBEVENTID_ENUM_BASE                            (25000)
#define TPHY_SUBEVENTID_ENUM_BASE                           (26000)

#define WRRC_SUBEVENTID_ENUM_BASE                           (31000)
#define WL2_SUBEVENTID_ENUM_BASE                            (32000)
#define WPHY_SUBEVENTID_ENUM_BASE                           (33000)

#define GAS_SUBEVENTID_ENUM_BASE                            (34000)
#define GL2_SUBEVENTID_ENUM_BASE                            (35000)
#define GPHY_SUBEVENTID_ENUM_BASE                           (36000)

#define CRRC_SUBEVENTID_ENUM_BASE                           (37000)
#define CL2_SUBEVENTID_ENUM_BASE                            (38000)
#define CPHY_SUBEVENTID_ENUM_BASE                           (39000)

/*DEFINE MODEM ALARMID BASE END*/

/*LNAS �ڲ�ģ�����*/
#define LNAS_EMM_SUBEVENTID_ENUM_BASE          (LNAS_SUBEVENTID_ENUM_BASE)
#define LNAS_ESM_SUBEVENTID_ENUM_BASE          (LNAS_SUBEVENTID_ENUM_BASE+50)
#define LNAS_IMSA_SUBEVENTID_ENUM_BASE         (LNAS_SUBEVENTID_ENUM_BASE+100)
#define LNAS_LCS_SUBEVENTID_ENUM_BASE          (LNAS_SUBEVENTID_ENUM_BASE+150)
#define LNAS_LPP_SUBEVENTID_ENUM_BASE          (LNAS_SUBEVENTID_ENUM_BASE+170)


/*****************************************************************************
3 Massage Declare
*****************************************************************************/

/*****************************************************************************
 4 ENUM
*****************************************************************************/

/*******************************************************************************************************************
 --------------------------------------------------�¼�����------------------------------------------------------
*******************************************************************************************************************/

/*****************************************************************************
 ö����    : MODEM_COMM_ALARMID_ENUM_UINT8
 Э����  :
 ASN.1���� :
 ö��˵��  : �����¼��澯����ö�ٶ��壬��������:
            1�������¼��澯����ö�ٶ���
            2���ǹ����¼��澯����ö�����Ͷ���(�ǹ����¼�����������¼���
            ���¼��Ƕ���ǹ������¼��ļ���)

*****************************************************************************/
enum MODEM_COMM_EVENTID_ENUM
{
    /**************************************** ��������¼�EventID**********************************************/
    TDS_OOS_INFO_REPORT_FAULTID                        = COMM_RELATION_EVENTID_ENUM_BASE+0,



    /**************************************** ����ǹ����¼�EventID**********************************************/
    /* ����TLRRC�ǹ����¼�EventID */
    TRRC_STATISTIC_INFO_COLLECTION_EVENTID             = COMM_TLRRC_EVENTID_ENUM_BASE+0,
    LRRC_STATISTIC1_INFO_COLLECTION_EVENTID            = COMM_TLRRC_EVENTID_ENUM_BASE+1,
    LRRC_STATISTIC2_INFO_COLLECTION_EVENTID            = COMM_TLRRC_EVENTID_ENUM_BASE+2,
    LRRC_VOLTE_FAULT_AUTOAN_IND_EVENTID                = COMM_TLRRC_EVENTID_ENUM_BASE+3,
    LPS_VOLTE_STAT_IND_EVENTID                         = COMM_TLRRC_EVENTID_ENUM_BASE+4,

    /* ����TL�ǹ����¼�EventID */
    TMAC_STATISTIC_INFO_COLLECTION_EVENTID             = COMM_TL2_EVENTID_ENUM_BASE+0,
    LL2_STATISTIC_INFO_COLLECTION_EVENTID              = COMM_TL2_EVENTID_ENUM_BASE+1,

    /* ����LANS�ǹ����¼�EventID */
    LNAS_STATISTIC_INFO_COLLECTION_EVENTID             = COMM_LNAS_EVENTID_ENUM_BASE+0,

    /* ����GAS�ǹ����¼�EventID */
    GAS_STATISTIC_INFO_COLLECTION_EVENTID              = COMM_GUC_EVENTID_ENUM_BASE+0,

    /* ������Դ�˲�ǹ����¼�EventID */
    PS_RADIO_RESOURCE_CHECK_EVENTID                    = COMM_PSCOM_EVENTID_ENUM_BASE+1,
    /*Added by h00377722 for ���ֳ���CHR, 2017-03-14,begin*/
    /* ����WAS�ǹ����¼�EventID */
    WAS_STATISTIC_INFO_COLLECTION_EVENTID              = COMM_GUC_EVENTID_ENUM_BASE+1,
    /*Added by h00377722 for ���ֳ���CHR, 2017-03-14,end*/

    /* ��������ǹ����¼�EventID�ڴ˶��� */

    COMM_ERRORLOG_EVENTID_TYPE_BUTT
};
typedef VOS_UINT8  MODEM_COMM_EVENTID_ENUM_UINT8;


/*******************************************************************************************************************
 --------------------------------------------------���¼�����------------------------------------------------------
*******************************************************************************************************************/
/*****************************************************************************
 ö����    : MODEM_GAS_SUBEVENTID_ENUM_UINT16
 Э����  :
 ASN.1���� :
 ö��˵��  : GAS�澯�������¼�ö�ٶ���
*****************************************************************************/
enum MODEM_GAS_SUBEVENTID_ENUM
{
    GAS_NOT_SUPP_BSS_PAGING_COOR_CHR_RPT_STRU_ALARMID     = GAS_SUBEVENTID_ENUM_BASE+1,
    GAS_RADIO_RESOURCE_CHECK_CHR_RPT_STRU_ALARMID         = GAS_SUBEVENTID_ENUM_BASE+2,
    GAS_CUSTOM_C1_CELL_CHR_RPT_STRU_ALARMID               = GAS_SUBEVENTID_ENUM_BASE+3,

    GAS_ERRORLOG_ALARMID_BUTT
};
typedef VOS_UINT16  MODEM_GAS_SUBEVENTID_ENUM_UINT16;

/*****************************************************************************
 ö����    : MODEM_GUCNAS_SUBEVENTID_ENUM_UINT16
 Э����  :
 ASN.1���� :
 ö��˵��  : GUCNAS���¼�ö�ٶ���
*****************************************************************************/
enum MODEM_GUCNAS_SUBEVENTID_ENUM
{
    GUCNAS_RADIO_RESOURCE_CHECK_CHR_RPT_STRU_ALARMID       = GUCNAS_SUBEVENTID_ENUM_BASE+1,

    GUCNAS_ERRORLOG_ALARMID_BUTT
};
typedef VOS_UINT16  MODEM_GUCNAS_SUBEVENTID_ENUM_UINT16;



/*****************************************************************************
 ö����    : MODEM_CAS_SUBEVENTID_ENUM_UINT16
 Э����  :
 ASN.1���� :
 ö��˵��  : CAS���¼�ö�ٶ���
*****************************************************************************/
enum MODEM_CAS_SUBEVENTID_ENUM
{
    CAS_1X_RADIO_RESOURCE_CHECK_CHR_RPT_STRU_ALARMID       = CAS_SUBEVENTID_ENUM_BASE+1,
    CAS_HRPD_RADIO_RESOURCE_CHECK_CHR_RPT_STRU_ALARMID     = CAS_SUBEVENTID_ENUM_BASE+2,

    CAS_ERRORLOG_ALARMID_BUTT
};
typedef VOS_UINT16  MODEM_CAS_SUBEVENTID_ENUM_UINT16;

/*Added by h00377722 for ���ֳ���CHR, 2017-03-14,begin*/
/*****************************************************************************
 ö����    : MODEM_WAS_SUBEVENTID_ENUM_UINT16
 Э����  :
 ASN.1���� :
 ö��˵��  : WAS�澯�������¼�ö�ٶ���
*****************************************************************************/
enum MODEM_WAS_SUBEVENTID_ENUM
{
    WAS_RRC_ACCESS_LONG_CHR_RPT_STRU_ALARMID          = WRRC_SUBEVENTID_ENUM_BASE+1,
    WAS_RADIO_RESOURCE_CHECK_CHR_RPT_STRU_ALARMID     = WRRC_SUBEVENTID_ENUM_BASE+2,
    WAS_RRC_STATE_LAST_TIME_CHR_RPT_STRU_ALARMID      = WRRC_SUBEVENTID_ENUM_BASE+3,
    WAS_WOOS_SEARCH_SECTION_CHR_RPT_STRU_ALARMID      = WRRC_SUBEVENTID_ENUM_BASE+4,
    WAS_W2W_RESEL_SLOW_CHR_RPT_STRU_ALARMID           = WRRC_SUBEVENTID_ENUM_BASE+5,

    WAS_ERRORLOG_ALARMID_BUTT
};
typedef VOS_UINT16  MODEM_WAS_SUBEVENTID_ENUM_UINT16;
/*Added by h00377722 for ���ֳ���CHR, 2017-03-14,end*/

/*****************************************************************************
 ö����    : MODEM_TRRC_SUBEVENTID_ENUM_UINT16
 Э����  :
 ASN.1���� :
 ö��˵��  : TRRC�澯�������¼�ö�ٶ���
*****************************************************************************/
enum MODEM_TRRC_SUBEVENTID_ENUM
{
    TRRC_OUT_OF_SYNC_CELL_UPDATE_INFO_STRU_ALARMID     = TRRC_SUBEVENTID_ENUM_BASE+1,
    TRRC_OOS_INFO_STRU_ALARMID                         = TRRC_SUBEVENTID_ENUM_BASE+2,
    TRRC_APP_COLLECT_TDS_FREQ_REPORT_STRU_ALARMID      = TRRC_SUBEVENTID_ENUM_BASE+3,

    TRRC_CS_CALL_EST_CNF_INFO_STRU_ALARMID             = TRRC_SUBEVENTID_ENUM_BASE+4,
    TRRC_CS_CALL_CONN_REQ_INFO_STRU_ALARMID            = TRRC_SUBEVENTID_ENUM_BASE+5,
    TRRC_CS_CALL_AIRMSG_INFO_STRU_ALARMID              = TRRC_SUBEVENTID_ENUM_BASE+6,
    TRRC_APP_CONFIG_PCH_CELL_INFO_REPORT_STRU_ALARMID  = TRRC_SUBEVENTID_ENUM_BASE+7,

    /*Add by lilin ��Դ�˲�CHR 2017-3-13 begin*************/
    TRRC_CHR_RRM_RESOURCE_CHECK_INFO_LIST_STRU_ALARMID = TRRC_SUBEVENTID_ENUM_BASE+8,
    /*Add by lilin ��Դ�˲�CHR 2017-3-13 end*************/

    TRRC_ERRORLOG_ALARMID_BUTT
};
typedef VOS_UINT16  MODEM_TRRC_SUBEVENTID_ENUM_UINT16;

/*****************************************************************************
 ö����    : MODEM_LNAS_SUBEVENTID_ENUM_UINT16
 Э����  :
 ASN.1���� :
 ö��˵��  : LNAS�澯�������¼�ö�ٶ���
*****************************************************************************/
enum MODEM_LNAS_SUBEVENTID_ENUM
{
    LNAS_EMM_ERR_INFO_DSDS_RRM_RF_CHECKIN_STRU_ALARMID         = LNAS_EMM_SUBEVENTID_ENUM_BASE+1,
    LNAS_EMM_ERR_INFO_LTE_FAKE_NODEB_STRU_ALARMID              = LNAS_EMM_SUBEVENTID_ENUM_BASE+2,
    LNAS_EMM_ERR_INFO_LOAD_BALANCE_STRU_ALARMID                = LNAS_EMM_SUBEVENTID_ENUM_BASE+3,
    LNAS_EMM_ERR_INFO_DSDS_RF_RECOVER_RETRY_STRU_ALARMID       = LNAS_EMM_SUBEVENTID_ENUM_BASE+4,
    LNAS_EMM_ERR_INFO_GRADUAL_FORBIDDEN_OPTIMIZE_STRU_ALARMID  = LNAS_EMM_SUBEVENTID_ENUM_BASE+5,
    LNAS_EMM_ERR_INFO_VOLTE_WITH_TA_CHANGE_STRU_ALARMID        = LNAS_EMM_SUBEVENTID_ENUM_BASE+6,
    LNAS_EMM_ERR_INFO_NO_RAU_WITH_TAU_STRU_ALARMID             = LNAS_EMM_SUBEVENTID_ENUM_BASE+7,
    LNAS_EMM_ERR_INFO_IMS_BEARER_WITH_CN_MOD_CONFLICT_STRU_ALARMID  = LNAS_EMM_SUBEVENTID_ENUM_BASE+8,
    LNAS_EMM_ERR_INFO_3440_TIMEOUT_DO_NOT_REL_STRU_ALARMID          = LNAS_EMM_SUBEVENTID_ENUM_BASE+9,
    LNAS_EMM_ERR_INFO_CNMSG_DECODE_FAIL_STRU_ALARMID                = LNAS_EMM_SUBEVENTID_ENUM_BASE+10,
    LNAS_EMM_ERR_INFO_SMC_FAIL_STRU_ALARMID                         = LNAS_EMM_SUBEVENTID_ENUM_BASE+11,
    LNAS_EMM_ERR_INFO_LOCAL_DETACH_STRU_ALARMID                     = LNAS_EMM_SUBEVENTID_ENUM_BASE+12,
    LNAS_EMM_ERR_INFO_T3402_DEACTIVED_STRU_ALARMID                  = LNAS_EMM_SUBEVENTID_ENUM_BASE+13,
    LNAS_EMM_ERR_INFO_ATT_FAIL_ESM_NEED_DISABLE_LTE_STRU_ALARMID    = LNAS_EMM_SUBEVENTID_ENUM_BASE+14,
    LNAS_EMM_ERR_INFO_MMC_REL_DRGINIT_STRU_ALARMID                  = LNAS_EMM_SUBEVENTID_ENUM_BASE+15,
    LNAS_EMM_ERR_INFO_MO_BAR_ATTAMPT_STRU_ALARMID                   = LNAS_EMM_SUBEVENTID_ENUM_BASE+16,
    LNAS_ESM_ERR_INFO_SWITCH_APN_STRU_ALARMID                       = LNAS_EMM_SUBEVENTID_ENUM_BASE+17,
    LNAS_ESM_ERR_INFO_CNMSG_DECODE_FAIL_STRU_ALARMID                = LNAS_EMM_SUBEVENTID_ENUM_BASE+18,
    LNAS_IMSA_ERR_INFO_EMC_TCALL_TIMEOUT_STRU_ALARMID               = LNAS_EMM_SUBEVENTID_ENUM_BASE+19,
    LNAS_IMSA_ERR_INFO_DSDS_WAIT_BEAR_REL_TIMEOUT_STRU_ALARMID      = LNAS_EMM_SUBEVENTID_ENUM_BASE+20,
    LNAS_IMSA_ERR_INFO_DSDS_SMS_RF_OCCUPY_STRU_ALARMID              = LNAS_EMM_SUBEVENTID_ENUM_BASE+21,
    LNAS_IMSA_ERR_INFO_WAIT_RETRY_START_SWITCH_STRU_ALARMID         = LNAS_EMM_SUBEVENTID_ENUM_BASE+22,
    LNAS_IMSA_ERR_INFO_HANDOVER_INITIAL_PDN_CONN_STRU_ALARMID       = LNAS_EMM_SUBEVENTID_ENUM_BASE+23,
    LNAS_IMSA_ERR_INFO_REG_REJ_WAIT_RECOVERY_STRU_ALARMID           = LNAS_EMM_SUBEVENTID_ENUM_BASE+24,
    LNAS_EMM_ERR_INFO_DETACH_NO_RF_STRU_ALARMID                     = LNAS_EMM_SUBEVENTID_ENUM_BASE+25,

    LNAS_ERR_INFO_ALARMID_BUTT
};
typedef VOS_UINT16  MODEM_LNAS_SUBEVENTID_ENUM_UINT16;

/*****************************************************************************
 ö����    : MODEM_TRRC_SUBEVENTID_ENUM_UINT16
 Э����  :
 ASN.1���� :
 ö��˵��  : TRRC�澯�������¼�ö�ٶ���
*****************************************************************************/
enum MODEM_LRRC_SUBEVENTID_ENUM
{
    LPS_OM_VOLTE_FAULT_AUTOAN_INFO_STRU_ALARMID     = LRRC_SUBEVENTID_ENUM_BASE+1,
    LPS_OM_VOLTE_GENERAL_STAT_INFO_STRU_ALARMID      = LRRC_SUBEVENTID_ENUM_BASE+2,
    LRRC_OM_UE_CAP_FALL_REPORT_STRU_ALARMID          = LRRC_SUBEVENTID_ENUM_BASE+3,
    LRRC_OM_USER_PLANE_ERR_REL_REPORT_STRU_ALARMID   = LRRC_SUBEVENTID_ENUM_BASE+4,
    LRRC_CHR_RRM_RES_CHECK_INFO_LIST_STRU_ALARMID     = LRRC_SUBEVENTID_ENUM_BASE+5,
    LRRC_ERRORLOG_ALARMID_BUTT
};
typedef VOS_UINT16  MODEM_LRRC_SUBEVENTID_ENUM_UINT16;

/*****************************************************************************
 ö����    : MODEM_TLL2_SUBEVENTID_ENUM_UINT16
 Э����  :
 ASN.1���� :
 ö��˵��  : TLL2�澯�������¼�ö�ٶ���
*****************************************************************************/
enum MODEM_TLL2_SUBEVENTID_ENUM
{
    TLL2_OOS_INFO_STRU_ALARMID                        = TL2_SUBEVENTID_ENUM_BASE+1,
    /*niuxiufan preamble modify begin */
    LMAC_OM_PREAMBLE_SPEC_NOT_MATCH_INFO_STRU_ALARMID,         /*Preamble��ƥ��ֱͨCHR */
    /*niuxiufan preamble modify end */

    TLL2_ERRORLOG_ALARMID_BUTT
};
typedef VOS_UINT16  MODEM_TLL2_SUBEVENTID_ENUM_UINT16;

/*****************************************************************************
 ö����    : MODEM_CL2_SUBEVENTID_ENUM_UINT16
 Э����  :
 ASN.1���� :
 ö��˵��  : CL2���¼�ö�ٶ���
*****************************************************************************/
enum MODEM_CL2_SUBEVENTID_ENUM
{
    CTTF_RADIO_RESOURCE_CHECK_CHR_DIRECT_RPT_STRU_ALARMID   = CL2_SUBEVENTID_ENUM_BASE+1,

    CTTF_ERRORLOG_ALARMID_BUTT
};
typedef VOS_UINT16  MODEM_CL2_SUBEVENTID_ENUM_UINT16;



/*****************************************************************************
 5 STRUCT
*****************************************************************************/

/*****************************************************************************
6 UNION
*****************************************************************************/

/*****************************************************************************
7 Extern Global Variable
*****************************************************************************/

/*****************************************************************************
8 Fuction Extern
*****************************************************************************/

/*****************************************************************************
9 OTHERS
*****************************************************************************/


#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

#ifdef __cplusplus
  #if __cplusplus
      }
  #endif
#endif

#endif /* end of ErrLogComm.h */
