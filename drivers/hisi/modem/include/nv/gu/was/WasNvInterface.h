/******************************************************************************

  Copyright(C)2008,Hisilicon Co. LTD.

 ******************************************************************************
  File Name       : WasNvInterface.h
  Description     : WasNvInterface.h header file
  History         :
  1.��    ��   : 2017��1��17��
    ��    ��   : s00184266
    �޸�ԭ��   : dts2017011702266, ����DESCRIPTION�ֶ�����

******************************************************************************/

#ifndef __WASNVINTERFACE_H__
#define __WASNVINTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

#include "PsTypeDef.h"

/*****************************************************************************
  2 Macro
*****************************************************************************/
#define WAS_MAX_NETWORKPARA_SIZE                (144)                           /*  NET WorkPara�ļ����Ⱥ궨�� */
/* Modified by x00220225 for �ƶ�Ƶ���ռ�, 2015-07-14, begin */
#define WAS_HISTORY_SEARCH_PLMN_MAX_NUM         (16)
#define WAS_HISTORY_PLMN_MAX_NUM                (32)                            /* �洢��NV�е����PLMN��Ϣ */
/* Modified by x00220225 for �ƶ�Ƶ���ռ�, 2015-07-14, end */
#define WAS_OPERATOR_FREQS_MAX_NUM              (16)
#define WAS_OPERATOR_PLMN_MAX_NUM               (20)


#define NV_ITEM_MEAS_THRESHOLD_SIZE             (40)
#define WAS_MAPWEIGHT_MAX_NUM                   (16)
#define WAS_HISTORY_PLMN_ID_BYTES               (3)
/* Modified by x00220225 for �ƶ�Ƶ���ռ�, 2015-07-14, begin */
#define WAS_HISTORY_SEARCH_FREQS_MAX_NUM        (6)
#define WAS_HISTORY_FREQS_MAX_NUM               (16)                             /* �洢��NV�е�PLMN��Ӧ��Ƶ�������� */
/* Modified by x00220225 for �ƶ�Ƶ���ռ�, 2015-07-14, end */
#define WAS_UE_ACCESS_CAPA_NUM                  (20)
#define NVIM_MAX_MCC_SIZE                       (3)
#define NVIM_MAX_MNC_SIZE                       (3)
#define NVIM_MAX_RPLMN_FDD_FREQ_NUM             (8)
#define NV_ITEM_RPLMN_INFO_SIZE                 (56)
#define NV_ITEM_NETWORKPARA_SIZE                (144)
#define NV_Item_WAS_RadioAccess_Capa_SIZE       (20)
#define NV_Item_WAS_RadioAccess_Capa_New_SIZE   (48)                            /* WAS����NV��NV_Item_WAS_RadioAccess_Capa_New */
#define NV_Item_WCDMA_PLMN_FREQ_PAIR_List_SIZE  (260)
/* Added by x00220225 for �ƶ�Ƶ���ռ�, 2015-07-17, begin */
#define NV_Item_WCDMA_HISTORY_PLMN_FREQ_LIST_SIZE  (1156)
/* Added by x00220225 for �ƶ�Ƶ���ռ�, 2015-07-17, end */
#define NV_Item_WCDMA_OPERATOR_FREQ_List_SIZE   (56)
#define NV_ITEM_BG_THRESHOLD_SIZE               (8)
#define NV_ITEM_PDCP_LOSSLESS_SIZE              (2)
#define NV_ITEM_DYN_FREQUENCY_SIZE              (2)
#define NV_ITEM_INIT_FREQUENCY_SIZE             (2)
#define NV_ITEM_PLMN_SEARCH_THRESHOLD_SIZE      (4)
#define NV_ITEM_PLMN_SEARCH_FLOW_SIZE           (8)
#define NV_ITEM_ALL_BAND_SEARCH_PARA_SIZE       (8)
#define NV_ITEM_BROKEN_CELL_PARA_SIZE           (4)
#define NV_ITEM_WAS_ERRORLOG_ITEM_NUM           (4)
#define NV_ITEM_WCDMA_IND_FRED_SIZE             (2)
#define NV_ITEM_FREQBANDS_LIST_SIZE             (52)
#define WAS_NV_MEAS_THRESHOLD_HALF_THRHLD_SIZE  (NV_ITEM_MEAS_THRESHOLD_SIZE/2)
#define WAS_NV_MEAS_THRESHOLD_RESERVE_SIZE      (WAS_NV_MEAS_THRESHOLD_HALF_THRHLD_SIZE-4)
#define NV_ITEM_WAS_ERRORLOG_CSHO_LEN_SIZE      (2)
#define WAS_NV_PTL_VER_R3                       (0)                             /* WAS_PTL_VER_R3 */
#define WAS_NV_PTL_VER_R4                       (1)                             /* WAS_PTL_VER_R4 */
#define WAS_NV_PTL_VER_R5                       (2)                             /* WAS_PTL_VER_R5 */
#define WAS_NV_PTL_VER_R6                       (3)                             /* WAS_PTL_VER_R6 */
#define WAS_NV_PTL_VER_R7                       (4)                             /* WAS_PTL_VER_R7 */
#define WAS_NV_PTL_VER_R8                       (5)                             /* WAS_PTL_VER_R8 */
#define WAS_NV_PTL_VER_R9                       (6)                             /* WAS_PTL_VER_R9 */

#define WAS_INVALID_MNC                         (0xffffffff)
#define WAS_INVALID_MCC                         (0xffffffff)
#define WAS_MAX_CSFB_MCC_BAND_NUM               (80)
#define WAS_CSFB_MAX_DETECT_CELL_NUM            (3)                              /* С������ʱ����ϱ��ļ��С������ */
#define WAS_CSFB_MAX_REPORT_CELL_NUM            (3)                              /* CSFB����ָ��Ƶ������ʱ����ϱ���С������ */

#define WAS_OPERATOR_CLOUD_PLMN_MAX_NUM         (50)
#define WAS_OPERATOR_CLOUD_FREQS_MAX_NUM        (16)
#define WAS_NV_BROKEN_CELL_RSCP_HIGH_THRESH     (-85)                           /* ����borokencell�б��е�С���Ǹ�����С�������� */
#define WAS_NV_BROKEN_CELL_RSCP_OFFSET          (20)                            /* ����Brokencell�е�С��������÷ų�bar�б������ */

#define WAS_NV_ERACH_BROKEN_CELL_N_FAIL               (1)                       /* ����Erach BrokenCell�б��е�С����RRMM_EST_REQ��Ĭ��ֵ */
#define WAS_NV_ERACH_BROKEN_CELL_MAX_BAR_LEN          (300)                     /* ����Erach BrokenCell�б��е�С�����Barʱ�䣬��λ�� */
#define WAS_NV_ERACH_BROKEN_CELL_INIT_BAR_LEN         (60)                      /* ����Erach BrokenCell�б��е�С���ĳ�ʼBarʱ�䣬��λ�� */
#define WAS_NV_ERACH_BROKEN_CELL_RSCP_HIGH_THRESH     (-80)                     /* ����Erach BrokenCell�б��е�С���Ǹ�����С�������� */
#define WAS_NV_ERACH_BROKEN_CELL_ECN0_THRESH          (-12)                     /* Erach BrokenCell�б��е�С���������ڴ����޲ſ����˳�bar�б� */

#define WAS_NV_ERACH_BROKEN_CELL_RSCP_OFFSET          (15)                      /* ����Erach BrokenCell�е�С��������÷ų�bar�б������ */
#define WAS_NV_ERACH_BROKEN_CELL_MIN_RSCP_OFFSET      (10)                      /* ����Erach BrokenCell�е�С��������÷ų�bar�б����С���� */

#define WAS_NV_DEFAULT_CQQUALMIN                (-18)
#define WAS_NV_DEFAULT_CQRXLEVMIN               (-110)
#define WAS_NV_MAX_ATT_PLMN_NUM                 16
#define WAS_NV_CELLULAR_PREFER_HIGH_THRESH             (-60)                    /* CELLULAR_PREFER Ĭ��High RSCP ���ޣ���λ��dbm */
#define WAS_NV_CELLULAR_PREFER_LOW_THRESH              (-85)                    /* CELLULAR_PREFER Ĭ��Low RSCP ���ޣ���λ��dbm */
#define WAS_NV_CELLULAR_PREFER_OFFSET                  (5)                      /* CELLULAR_PREFER Ĭ��Offset����λ��dbm */
#define WAS_NV_CELLULAR_PREFER_TEVALUATION             (5000)                   /* CELLULAR_PREFER Ĭ������ʱ�䣬��λ��ms */
#define WAS_NV_CELLULAR_PREFER_MIN_TEVALUATION         (1000)                   /* CELLULAR_PREFER Ĭ��������Сʱ�䣬��λ��ms */

#define WAS_NV_ACALLBAR_DEFAULT_TIME                   (1280)                   /* AC ALL BAR Ĭ�ϳͷ�ʱ�䣬��λ��s*/
#define WAS_NV_ACALLBAR_MIN_TIME                       (10)                     /* AC ALL BAR ��С�ͷ�ʱ�䣬��λ��s*/
#define WAS_NV_ACALLBAR_MAX_TIME                       (1280)                   /* AC ALL BAR ���ͷ�ʱ�䣬��λ��s*/
#define WAS_NV_MAX_NOT_SUPPORT_FAKE_DMCR_MCC_NUM       (10)
#define WAS_NV_MAX_EMERGENCY_SUITABLE_CAMP_MCC_NUM     (16)

#define WAS_HISTORY_FREQ_SRCH_DEFAULT_ENABLE_BITMAP             ( (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_SPEC_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_FAST_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_HISTORY_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_IDLE_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_FACH_OOS_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_PCH_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_DCH_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_L2W_REDIR_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_CSFB_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_PRE_BAND_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_LTE_OOS_SEARCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_GEO_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_COMMON\)

#define WAS_FULL_BAND_FREQ_SRCH_DEFAULT_ENABLE_BITMAP           ( (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_SPEC_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_FAST_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_HISTORY_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_IDLE_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_FACH_OOS_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_PCH_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_DCH_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_L2W_REDIR_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_CSFB_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_PRE_BAND_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_LTE_OOS_SEARCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_GEO_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_COMMON\)

#define WAS_CLOUD_FREQ_FROM_CSS_DEFAULT_ENABLE_BITMAP           ( (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_SPEC_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_FAST_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_HISTORY_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_IDLE_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_FACH_OOS_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_PCH_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_DCH_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_L2W_REDIR_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_CSFB_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_PRE_BAND_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_LTE_OOS_SEARCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_GEO_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_COMMON\)

#define WAS_PREFER_BAND_FROM_CSS_DEFAULT_ENABLE_BITMAP          ( (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_SPEC_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_FAST_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_HISTORY_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_IDLE_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_FACH_OOS_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_PCH_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_DCH_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_L2W_REDIR_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_CSFB_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_PRE_BAND_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_LTE_OOS_SEARCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_GEO_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_COMMON\)
#define WAS_OPERATOR_FREQ_FROM_CSS_DEFAULT_ENABLE_BITMAP        ( (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_SPEC_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_FAST_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_HISTORY_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_IDLE_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_FACH_OOS_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_PCH_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_DCH_OOS_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_L2W_REDIR_SRCH \
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_CSFB_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_PRE_BAND_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_LTE_OOS_SEARCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_NAS_GEO_SRCH\
                                                                | (VOS_UINT32)WAS_NETWORK_SEARCH_PROGRESS_COMMON\)
/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/


/*****************************************************************************
  5 STRUCT
*****************************************************************************/
/*****************************************************************************
*                                                                            *
*                           ����������Ϣ�ṹ                                 *
*                                                                            *
******************************************************************************/
/*****************************************************************************
 ö �� ��  : WAS_NETWORK_SEARCH_PROGRESS_ENUM
 ö��˵��  : WAS��������ö��
 �޸���ʷ  :
  1.��  ��   : 2015��10��19��
    ��  ��   : w00134354
    �޸����� : �����ɽṹ

*****************************************************************************/
enum WAS_NETWORK_SEARCH_PROGRESS_ENUM
{
    WAS_NETWORK_SEARCH_PROGRESS_NONE                           = 0x00000000,     
    WAS_NETWORK_SEARCH_PROGRESS_SPEC_SRCH                      = 0x00000001,   /* bit0: NAS SPEC SRCH */
    WAS_NETWORK_SEARCH_PROGRESS_FAST_SRCH                      = 0x00000002,   /* bit1: NAS SPEC FAST SRCH */
    WAS_NETWORK_SEARCH_PROGRESS_HISTORY_SRCH                   = 0x00000004,   /* bit2: NAS HISTORY SRCH */
    WAS_NETWORK_SEARCH_PROGRESS_IDLE_OOS_SRCH                  = 0x00000008,   /* bit3: IDLE OOS SRCH */
    WAS_NETWORK_SEARCH_PROGRESS_FACH_OOS_SRCH                  = 0x00000010,   /* bit4: FACH OOS SRCH */
    WAS_NETWORK_SEARCH_PROGRESS_PCH_OOS_SRCH                   = 0x00000020,   /* bit5: PCH OOS SRCH */
    WAS_NETWORK_SEARCH_PROGRESS_DCH_OOS_SRCH                   = 0x00000040,   /* bit6: DCH OOS SRCH */
    WAS_NETWORK_SEARCH_PROGRESS_L2W_REDIR_SRCH                 = 0x00000080,   /* bit7: L2G REDIR(include L2W CSFB) SRCH */
    WAS_NETWORK_SEARCH_PROGRESS_NAS_CSFB_SRCH                  = 0x00000100,   /* bit8: NAS CSFB SRCH */
    WAS_NETWORK_SEARCH_PROGRESS_NAS_PRE_BAND_SRCH              = 0x00000200,   /* bit9: NAS prefer band SRCH  */
    WAS_NETWORK_SEARCH_PROGRESS_LTE_OOS_SEARCH                 = 0x00000400,   /* bit10:lte OOS SRCH */
    WAS_NETWORK_SEARCH_PROGRESS_NAS_GEO_SRCH                   = 0x00000800,   /* bit11:NAS GEO SRCH */    
    WAS_NETWORK_SEARCH_PROGRESS_COMMON                         = 0x00001000,   /* bit12:ͨ������ */
    WAS_NETWORK_SEARCH_PROGRESS_BG_HISTORY_SRCH                = 0x00002000,   /* bit13:��ģ���ߴ�ģ�µ�bg������������history�� */
    WAS_NETWORK_SEARCH_PROGRESS_BG_SPEC_SRCH                   = 0x00004000,   /* bit14:��ģ���ߴ�ģ�µ�bg������������spec�� */
    /* Added by z00184470 for �����Ż�6.0, 2016-09-07, begin */
    WAS_NETWORK_SEARCH_PROGRESS_PLMN_LIST_SRCH                 = 0x00008000,   /* bit15:��ģ���ߴ�ģ�µ�PLMN LIST���� */
    WAS_NETWORK_SEARCH_PROGRESS_ANY_CELL_SRCH                  = 0x00010000,   /* bit16:��ģ���ߴ�ģ�µ�ANY CELL���� */
    /* Added by z00184470 for �����Ż�6.0, 2016-09-07, end */
    WAS_NETWORK_SEARCH_PROGRESS_BUTT
};
typedef VOS_UINT32 WAS_NETWORK_SEARCH_PROGRESS_ENUM_UINT32;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_NETPARA_STRU
 �ṹ˵��  : NET PARA��Ϣ�ṹ                       ID:8246 en_NV_Item_NetWorkPara
 DESCRIPTION:NET PARA��Ϣ�ṹ     
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               aucFileContent[WAS_MAX_NETWORKPARA_SIZE];   /* File���� */
}WAS_NVIM_NETPARA_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_MEAS_THRESHOLD_STRU
 �ṹ˵��  : ��NVIM�ж�����MeasThreshold����        ID:8263 en_NV_Item_Meas_Threshold
 DESCRIPTION:�������޲���
*****************************************************************************/
typedef struct
{
    VOS_INT16                               sMeasRptRscpThreshold;              /* 1a/1c�¼��������RSCP����    */
    VOS_INT16                               sMeasRptEcn0Threshold;              /* 1a/1c�¼��������ECN0����    */
    VOS_INT16                               sCellSrchRscpThreshold;             /* С���������RSCP����         */
    VOS_INT16                               sCellSrchEcn0Threshold;             /* С���������ECN0����         */
    VOS_INT16                               asReserve[WAS_NV_MEAS_THRESHOLD_RESERVE_SIZE];     /* �����ֶ� */
}WAS_NVIM_MEAS_THRESHOLD_STRU;

/* Modified by x00220225 for �ƶ�Ƶ���ռ�, 2015-07-17, begin */
/*****************************************************************************
 �ṹ��    : WAS_NVIM_PLMN_FREQ_PAIR_STRU
 �ṹ˵��  : �ṹ��WAS_NVIM_PLMN_FREQ_PAIR_LIST_STRU�����ݳ�Ա�ṹ��
 ע��      : �ýṹ���Ѿ��������ã�ʹ���µĽṹ�� WAS_NVIM_HISTORY_FREQ_LIST_STRU
 DESCRIPTION:�ѷ���
*****************************************************************************/

typedef struct
{
    VOS_UINT8                               aucPlmnId[WAS_HISTORY_PLMN_ID_BYTES];
    VOS_UINT8                               ucFreqCnt;
    VOS_UINT16                              ausFreq[WAS_HISTORY_SEARCH_FREQS_MAX_NUM];
}WAS_NVIM_PLMN_FREQ_PAIR_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_HISTORY_FREQ_LIST_STRU
 �ṹ˵��  : �ṹ��WAS_NVIM_HISTORY_SEARCH_PLMN_FREQ_LIST_STRU�����ݳ�Ա�ṹ��
 DESCRIPTION:��ʷƵ���б���Ϣ���ѷ���
*****************************************************************************/

typedef struct
{
    VOS_UINT8                               aucPlmnId[WAS_HISTORY_PLMN_ID_BYTES];
    VOS_UINT8                               ucFreqCnt;
    VOS_UINT16                              ausFreq[WAS_HISTORY_FREQS_MAX_NUM];
}WAS_NVIM_HISTORY_FREQ_LIST_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_PLMN_FREQ_PAIR_LIST_STRU
 �ṹ˵��  : ��NV�ж�ȡ���������ʷPLMN�Ͷ�Ӧ��Ƶ�� ID:8284 en_NV_Item_WCDMA_PLMN_FREQ_PAIR_List
 ע��      : �ýṹ���Ѿ��������ã�ʹ���µĽṹ�� WAS_NVIM_HISTORY_SEARCH_PLMN_FREQ_STRU
 DESCRIPTION:�ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulPlmnCnt;
    WAS_NVIM_PLMN_FREQ_PAIR_STRU            astNvPlmnFreqPair[WAS_HISTORY_SEARCH_PLMN_MAX_NUM];
}WAS_NVIM_PLMN_FREQ_PAIR_LIST_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_HISTORY_SEARCH_PLMN_FREQ_LIST_STRU
 �ṹ˵��  : ��NV�ж�ȡ���������ʷPLMN�Ͷ�Ӧ��Ƶ�� ID:3023 en_NV_Item_WCDMA_HISTORY_PLMN_FREQ_LIST
 DESCRIPTION:��ʷPLMN�Ͷ�Ӧ��Ƶ����Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulPlmnCnt;                          /* ������NV�е�PLMN������Range:[0,32] */
    WAS_NVIM_HISTORY_FREQ_LIST_STRU         astNvPlmnFreqPair[WAS_HISTORY_PLMN_MAX_NUM];/* �����PLMN�����Ӧ��Ƶ����Ϣ */
}WAS_NVIM_HISTORY_SEARCH_PLMN_FREQ_LIST_STRU;
/* Modified by x00220225 for �ƶ�Ƶ���ռ�, 2015-07-17, end */

typedef struct
{
    VOS_UINT8    ucMcc[NVIM_MAX_MCC_SIZE];
    VOS_UINT8    ucMnc[NVIM_MAX_MNC_SIZE];
}NVIM_PLMN_ID_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_PLMN_FREQ_PAIR_LIST_STRU
 �ṹ˵��  : ��NV�ж�ȡ���������ʷPLMN�Ͷ�Ӧ��Ƶ�� ID:8314 en_NV_Item_Opr_Freq_List
 DESCRIPTION:��ʷPLMN�Ͷ�Ӧ��Ƶ����Ϣ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enOperatorFreqListValidFlg;              /* Ƶ���б��Ƿ���Ч��־ */
    NVIM_PLMN_ID_STRU                       stPlmnId;                                /* Plmn ID */
    VOS_UINT8                               ucFreqNum;                               /* �������plmn��Ƶ����� */
    VOS_UINT16                              ausFreqInfo[24]; /* �������plmn��Ƶ����Ϣ */
}WAS_NVIM_OPERATOR_FREQ_LIST_STRU;

/*****************************************************************************
 �ṹ��    : WAS_PLMN_ID_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :PLMN���ݽṹ
 DESCRIPTION:PLMN���ݽṹ��Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulMcc;              /* MCC,3 bytes      */
    VOS_UINT32                          ulMnc;              /* MNC,2 or 3 bytes */
}WAS_PLMN_ID_STRU;
/*****************************************************************************
 �ṹ��    : WAS_NVIM_OPERATOR_FREQ_INFO_STRU
 �ṹ˵��  : ��Ӫ�̶���Ƶ����Ϣ
 DESCRIPTION:��Ӫ�̶���Ƶ����Ϣ
*****************************************************************************/
typedef struct
{
    WAS_PLMN_ID_STRU                        stPlmnId;                                /* Plmn ID */
    VOS_UINT8                               ucFreqNum;                               /* �������plmn��Ƶ����� */
    VOS_UINT8                               ucResvered1;                             /* ����λ */
    VOS_UINT8                               ucResvered2;                             /* ����λ */
    VOS_UINT8                               ucResvered3;                             /* ����λ */
    VOS_UINT16                              ausFreqInfo[WAS_OPERATOR_FREQS_MAX_NUM]; /* �������plmn��Ƶ����Ϣ */
}WAS_NVIM_OPERATOR_FREQ_INFO_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_OPERATOR_FREQ_PLMN_LIST_STRU
 �ṹ˵��  : ��NV�ж�ȡ���������ʷPLMN�Ͷ�Ӧ��Ƶ�� ID:3029 en_NV_Item_Was_Operator_Freq_List_Info
 DESCRIPTION:��ű������ʷPLMN�Ͷ�Ӧ��Ƶ����Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucPlmnNum;                          /* ���Ƶ�PLMN���������Ϊ20�� */
    VOS_UINT8                               ucReportCellNum;                    /* ��Ӫ��Ƶ������ʱֻ��DSP�ϱ���С��������Сֵ��Range:[1,32] */
    VOS_UINT8                               ucSearchOperatorBandFlg;            /* �Ƿ�Ҫ����ʷƵ�����Ӫ��Ƶ�����ڵ�band */
    VOS_UINT8                               aucResvered2;
    WAS_NVIM_OPERATOR_FREQ_INFO_STRU        ausPlmnFreqList[WAS_OPERATOR_PLMN_MAX_NUM]; /* ���Ƶ�PLMN ID�Լ�Ƶ����Ϣ */
}WAS_NVIM_OPERATOR_FREQ_PLMN_LIST_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_BG_THRESHOLD_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��NVIM�ж����ı�����Threshold����      ID:8316 en_NV_Item_BG_Threshold
 DESCRIPTION:��ű�����Threshold������Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucStatus;                           /*������ѡ��,0:������,1����,Ĭ��ֵ:1*/
    VOS_INT8                                cWCDMA_RSSI_Threshold;              /*Range:[-125,0]*/ /*WCDMA RSSI����,Ĭ��ֵ:-95*/
    VOS_INT8                                cWCDMA_Cpich_EcNo_Threshold;        /*Range:[-25,0]*/  /*WCDMA Cpich_EcNo����,Ĭ��ֵ:-11*/
    VOS_INT8                                cWCDMA_Cpich_Rscp_Threshold;        /*Range:[-125,0]*/ /*WCDMA Cpich_Rscp����,Ĭ��ֵ:-100*/
    VOS_INT8                                cGSM_RSSI_Threshold;                /*Range:[-125,0]*/ /*GSM RSSI����,Ĭ��ֵ:-95*/
    VOS_UINT8                               ucBgGsmMeasSqualOffset;             /* �����ȼ�RAT��ʱGSM������������Squal offset */
    VOS_UINT8                               ucBgGsmMeasSrxlevOffset;            /* �����ȼ�RAT��ʱGSM������������Srxlev offset */
    PS_BOOL_ENUM_UINT8                      enBgModifyDrxThresholdFlg;          /* �޸�BG��DRX���ޱ�� */
}WAS_NVIM_BG_THRESHOLD_STRU;

/*****************************************************************************
 �ṹ��    : WAS_BROKEN_CELL_STRU
 �ṹ˵��  : ���Broken Cell�Ĳ�������              ID:8341 en_NV_Item_WAS_BROKEN_CELL_PARA
 DESCRIPTION:���Broken Cell�Ĳ���������Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usNfail;                            /* ������Nfail��RRC_CONN_REQ�ط�,����С������Bar�б� */
    VOS_UINT16                              usTBarFailLen;                      /* С�������ֹ�б��ʱ�䣬��λ:�� */
} WAS_BROKEN_CELL_STRU;

/*****************************************************************************
 �ṹ��    : WAS_BROKEN_CELL_STRU
 �ṹ˵��  : ���Broken Cell�Ĳ�������              ID:3030 en_NV_Item_Was_Broken_Cell_Info
 DESCRIPTION:���Broken Cell�Ĳ���������Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucRaNfail;                          /* ������Nfail��RRC_CONN_REQ�ط�,����С������Bar�б� */
    VOS_UINT8                               ucRejNfail;                         /* RRC CONNECTION REJ N�ۺ�С������bar�б� */
    VOS_UINT8                               ucRscpOffset;                       /* ��С������������ô��dbʱ��С����bar�б�ų�������פ�����Խ��� */
    VOS_UINT8                               ucRsv1;                             /* ����λ */
    VOS_UINT16                              usRaMaxBarLen;                      /* barС�������ʱ������λΪ���ӣ�������ʱ��С���Զ���bar */
    VOS_UINT16                              usRaTBarInitialLen;                 /* С�������ֹ�б�ĳ�ʼʱ�䣬��λ:�� */
    VOS_UINT16                              usRejMaxBarLen;                     /* RejBarС�������ʱ��,��λ���� */
    VOS_UINT16                              usRejTBarInialLen;                  /* RRC CONNECTION REJ N�ۺ� Bar��С���ĳ�ʼʱ������λs */
    VOS_INT16                               sRscpHighThresh;                    /* ����bar�б��е�С���������ڴ����ޣ���С���ų�bar�б�Ͳ����������� */
    VOS_INT16                               sEcn0Thresh;                        /* ����λ */
    VOS_UINT16                              usRsv3;                             /* ����λ */
    VOS_UINT16                              usRsv4;                             /* ����λ */
} WAS_NVIM_BROKEN_CELL_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_UE_FEATURE_CTRL
 �ṹ˵��  : �洢��NVIM�����V3����������Ϣ         ID:8472 en_NV_Item_Was_UeFeature_Ctrl
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucIntegAlg;                         /* Ĭ��ֵΪ6,֧��UIA1&UIA2 */
    VOS_UINT8                               ucCipherAlg;                        /* Ĭ��ֵΪ7,֧��UEA1&UEA2 */
    VOS_UINT8                               ucWasFeatrue;                       /* Ĭ��ֵΪ0,bitλ���ο��Ƹ����ƶ�����˫DRX�����Ƿ�֧��*/
	PS_BOOL_ENUM_UINT8                      enEPchCbsSupport;                   /* �Ƿ�֧��Epch�½���Cbs */
}WAS_NVIM_UE_FEATURE_CTRL;

/*****************************************************************************
 �ṹ��    : WAS_WEAK_SIGNAL_ENERGY_THRESHOLD_STRU
 �ṹ˵��  : Errorlog���źŵ�����                   ID:8499 en_NV_Item_WAS_Errorlog_Energy_Threshold
 DESCRIPTION: Errorlog���źŵ����� 
*****************************************************************************/
typedef struct
{
    VOS_INT16                               sRscpThreshold;                     /* WCDMA ErrorLog�������źŵ�RSCP����ֵ��Range:[-130,-25] */
    VOS_INT16                               sECN0Threshold;                     /* WCDMA ErrorLog�������źŵ�ECN0����ֵ��Range:[-25,0] */
}WAS_WEAK_SIGNAL_ENERGY_THRESHOLD_STRU;

/*****************************************************************************
 ö����    : WAS_TX_RX_FREQ_SEPARAT_ENUM_UINT8
 Э����  :
 ASN.1���� :
 ö��˵��  :
*****************************************************************************/
enum WAS_TX_RX_FREQ_SEPARAT_ENUM
{
    WAS_TX_RX_FREQ_SEPARAT_DEFAULT_TX_RX_SEPARATION = 0,
    WAS_TX_RX_FREQ_SEPARAT_SPARE2,
    WAS_TX_RX_FREQ_SEPARAT_SPARE1,
    WAS_TX_RX_FREQ_SEPARAT_BUTT
} ;
typedef VOS_UINT8 WAS_TX_RX_FREQ_SEPARAT_ENUM_UINT8;

/*****************************************************************************
 �ṹ��    : WAS_RF_CAPA_STRU
 Э����  : 10.3.3.33 RF capability FDD
 ASN.1���� :
 �ṹ˵��  : RF������Ϣ
 DESCRIPTION: RF������Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucPowerClass;                       /* UE���ʼ���                               */
    WAS_TX_RX_FREQ_SEPARAT_ENUM_UINT8       enTxRxFreqSeparate;                 /* Tx/Rx Ƶ������                           */
    VOS_UINT8                               aucReserve1[2];                     /* 4�ֽڶ��룬���� */
}WAS_RF_CAPA_STRU;


/*****************************************************************************
 ö����    : WAS_DL_SIMUL_HS_DSCH_CFG_ENUM_UINT8
 Э����  :
 ASN.1���� :
 ö��˵��  :
*****************************************************************************/
enum  WAS_DL_SIMUL_HS_DSCH_CFG_ENUM
{
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS32 = 0,
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS64,
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS128,
    WAS_DL_SIMUL_HS_DSCH_CFG_KBPS384,
    WAS_DL_SIMUL_HS_DSCH_CFG_BUTT
};
typedef VOS_UINT8 WAS_DL_SIMUL_HS_DSCH_CFG_ENUM_UINT8;

/*****************************************************************************
 ö����    : WAS_PTL_VER_TYPE_ENUM_UINT8
 Э����  : 10.2.39    RRC CONNECTION REQUEST
 ASN.1���� : AccessStratumReleaseIndicator
 ö��˵��  : Э��汾
             Absence of the IE implies R3.
             The IE also indicates the release of the RRC transfer syntax
             supported by the UE 13 spare values are needed
*****************************************************************************/
enum WAS_PTL_VER_TYPE_ENUM
{
    WAS_PTL_VER_ENUM_R3                 = WAS_NV_PTL_VER_R3,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R3 = 0 */
    WAS_PTL_VER_ENUM_R4                 = WAS_NV_PTL_VER_R4,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R4 = 1 */
    WAS_PTL_VER_ENUM_R5                 = WAS_NV_PTL_VER_R5,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R5 = 2 */
    WAS_PTL_VER_ENUM_R6                 = WAS_NV_PTL_VER_R6,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R6 = 3 */
    WAS_PTL_VER_ENUM_R7                 = WAS_NV_PTL_VER_R7,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R7 = 4 */
    WAS_PTL_VER_ENUM_R8                 = WAS_NV_PTL_VER_R8,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R8 = 5 */
    WAS_PTL_VER_ENUM_R9                 = WAS_NV_PTL_VER_R9,                    /* _H2ASN_Replace  WAS_NV_PTL_VER_R9 = 6 */
    WAS_PTL_VER_ENUM_BUTT
};
typedef VOS_UINT8 WAS_PTL_VER_TYPE_ENUM_UINT8;

/*****************************************************************************
 ö����    : WAS_PTL_TOTAL_RLC_AM_BUF_SIZE_ENUM_UINT8
 Э����  :
 ASN.1���� :
 ö��˵��  :
*****************************************************************************/
enum WAS_TOTAL_RLC_AM_BUF_SIZE_ENUM
{
    WAS_TOTAL_RLC_AM_BUF_SIZE_DUMMY = 0,
    WAS_TOTAL_RLC_AM_BUF_SIZE_KB10,
    WAS_TOTAL_RLC_AM_BUF_SIZE_KB50,
    WAS_TOTAL_RLC_AM_BUF_SIZE_KB100,
    WAS_TOTAL_RLC_AM_BUF_SIZE_KB150,
    WAS_TOTAL_RLC_AM_BUF_SIZE_KB500,
    WAS_TOTAL_RLC_AM_BUF_SIZE_KB1000,
    WAS_TOTAL_RLC_AM_BUF_SIZE_BUTT
};
typedef VOS_UINT8 WAS_TOTAL_RLC_AM_BUF_SIZE_ENUM_UINT8;

/*****************************************************************************
 ö����    : WAS_MAX_AM_ENTITY_NUM_UINT8
 Э����  :
 ASN.1���� :
 ö��˵��  :
*****************************************************************************/
enum WAS_MAX_AM_ENTITY_NUM
{
    WAS_MAX_AM_ENTITY_NUM_DUMMY = 0,
    WAS_MAX_AM_ENTITY_NUM_AM4,
    WAS_MAX_AM_ENTITY_NUM_AM5,
    WAS_MAX_AM_ENTITY_NUM_AM6,
    WAS_MAX_AM_ENTITY_NUM_AM8,
    WAS_MAX_AM_ENTITY_NUM_AM16,
    WAS_MAX_AM_ENTITY_NUM_AM30,
    WAS_MAX_AM_ENTITY_NUM_BUTT
};
typedef UINT8 WAS_MAX_AM_ENTITY_NUM_UINT8;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_UE_CAPA_STRU
 �ṹ˵��  : �洢��NVIM�е�UE������Ϣ               ID:9008 en_NV_Item_WAS_RadioAccess_Capa_New
 DESCRIPTION: �洢��NVIM�е�UE������Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulHspaStatus;                       /* 0��ʾδ����,��ôDPA��UPA��֧��;1��ʾ���� */

    WAS_RF_CAPA_STRU                        stRfCapa;                           /* RF ������Ϣ                                  */

    PS_BOOL_ENUM_UINT8                      enSupportPwrBoosting;               /* 16QAM������أ��Ƿ�֧��E-DPCCH Power Boosting*/
    PS_BOOL_ENUM_UINT8                      enSf4Support;                       /* �Ƿ�֧��ul dpcch ʹ�� slotFormat4 */

    WAS_DL_SIMUL_HS_DSCH_CFG_ENUM_UINT8     enDlSimulHsDschCfg;                 /* ENUMERATED  OPTIONAL                         */
    WAS_PTL_VER_TYPE_ENUM_UINT8             enAsRelIndicator;                   /* Access Stratum Release Indicator             */

    PS_BOOL_ENUM_UINT8                      enHSDSCHSupport;                    /* �Ƿ�֧��enHSDSCHSupport�ı�־                */
    VOS_UINT8                               ucHSDSCHPhyCategory;                /* ֧��HS-DSCH���������ͱ�־                  */

    PS_BOOL_ENUM_UINT8                      enMacEhsSupport;
    VOS_UINT8                               ucHSDSCHPhyCategoryExt;
    PS_BOOL_ENUM_UINT8                      enMultiCellSupport;                 /* �Ƿ�֧�� Multi cell support,���֧��MultiCell,Ex2���� */
    VOS_UINT8                               ucHSDSCHPhyCategoryExt2;            /* HS-DSCH physical layer category extension 2 */

    PS_BOOL_ENUM_UINT8                      enCellSpecTxDiversityForDc;         /*  This IE is optionally present if Dual-Cell HSDPA is supported. Otherwise it is not needed.
                                                                                    The IE is not needed in the INTER RAT HANDOVER INFO message. Otherwise, it is optional*/
    PS_BOOL_ENUM_UINT8                      enEFDPCHSupport;                    /* �Ƿ�֧��E-FDPCH�ı�־,FDPCH֧��ʱ��NV����Ч */
    PS_BOOL_ENUM_UINT8                      enEDCHSupport;                      /* �Ƿ�֧��EDCH�ı�־                           */
    VOS_UINT8                               ucEDCHPhyCategory;                  /* ֧��UPA�ĵȼ�                                */
    PS_BOOL_ENUM_UINT8                      enSuppUl16QAM;                      /* �Ƿ�֧������16QAM����֧��ʱucEDCHPhyCategoryExt����Ч */
    VOS_UINT8                               ucEDCHPhyCategoryExt;               /* ���е��ز�֧��16QAMʱ����д7 */
    PS_BOOL_ENUM_UINT8                      enSuppEDpdchInterpolationFormula;   /* 16QAM������أ��Ƿ�֧��E-DPDCH power interpolation formula */
    PS_BOOL_ENUM_UINT8                      enSuppHsdpaInFach;                  /* ֧��CELL_FACH��HS-DSCH�Ľ��� */
    PS_BOOL_ENUM_UINT8                      enSuppHsdpaInPch;                   /* ֧��CELL_PCH��URA_PCH��HS-DSCH�Ľ��� */

    PS_BOOL_ENUM_UINT8                      enMacIsSupport;                     /* �Ƿ�֧��MAC_I/MAC_Is */

    PS_BOOL_ENUM_UINT8                      enFDPCHSupport;                     /* �Ƿ�֧��FDPCH�ı�־                          */

    PS_BOOL_ENUM_UINT8                      enHsscchLessSupport;                /* �Ƿ�֧�� hsscchlessHsdschOperation           */
    PS_BOOL_ENUM_UINT8                      enUlDpcchDtxSupport;                /* �Ƿ�֧�� discontinuousDpcchTransmission      */

    PS_BOOL_ENUM_UINT8                      enAdjFreqMeasWithoutCmprMode;       /* �Ƿ�֧�� Adjacent Frequency measurements without compressed mode */

    PS_BOOL_ENUM_UINT8                      enMimoSingleStreamStrict;           /* �Ƿ�����ֻ��ʹ�õ���MIMO */
    PS_BOOL_ENUM_UINT8                      enMimoWithDlTxDiversity;            /* R9���ԣ���MIMO����ʱ�����п����ŵ��Ƿ�����ʹ�÷ּ� */

    /* V7r1 ˫ģ����NV�� LTE���� */
    PS_BOOL_ENUM_UINT8                      enSptAbsPriBasedReselInUtra;        /* ֧��UTRA�е����ȼ���ѡ��Ĭ��Ϊ0��1Ϊ֧�֣�0Ϊ��֧�� */

    VOS_UINT8                               ucHSDSCHPhyCategoryExt3;            /* HS-DSCH physical layer category extension 3 */
    PS_BOOL_ENUM_UINT8                      enDcMimoSupport;                    /* �Ƿ�֧��DC+MIMO */
    PS_BOOL_ENUM_UINT8                      enSuppCommEDCH;                     /* E-RACH�������� */

    PS_BOOL_ENUM_UINT8                      enDcUpaSupport;                     /* �Ƿ�֧��DC UPA�ı�־ */
    VOS_UINT8                               ucEDCHPhyCategoryExt2;              /* EDCH  category extension 2*/
    PS_BOOL_ENUM_UINT8                      enEdpdchGainFactorFlg;              /* E-DPDCH���ʻ�������ʹ�ܱ�־λ */
    PS_BOOL_ENUM_UINT8                      enHo2EutranUnSupportFlg;            /* �Ƿ�֧�ֵ�L��HO */
    PS_BOOL_ENUM_UINT8                      enEutranMeasUnSupportFlg;           /* �Ƿ�֧�ֵ�����̬L�Ĳ��� */
    VOS_UINT8                               aucReserve2[5];
}WAS_NVIM_UE_CAPA_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_UE_CAPA_CUSTOMED_STRU
 �ṹ˵��  : ����������洢��NVIM�е�UE������Ϣ,��ǰ��BOSTON��֧����ʹ��   ID:9008 en_NV_Item_WAS_RadioAccess_Capa_New
 DESCRIPTION: ����������洢��NVIM�е�UE������Ϣ,��ǰ��BOSTON��֧����ʹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulHspaStatus;                       /* 0��ʾδ����,��ôDPA��UPA��֧��;1��ʾ���� */

    WAS_RF_CAPA_STRU                        stRfCapa;                           /* RF ������Ϣ                                  */

    PS_BOOL_ENUM_UINT8                      enSupportPwrBoosting;               /* 16QAM������أ��Ƿ�֧��E-DPCCH Power Boosting*/
    PS_BOOL_ENUM_UINT8                      enReserve1;                         /* ����enSf4Support��ԭλ����Ϊreserved */

    WAS_DL_SIMUL_HS_DSCH_CFG_ENUM_UINT8     enDlSimulHsDschCfg;                 /* ENUMERATED  OPTIONAL                         */
    WAS_PTL_VER_TYPE_ENUM_UINT8             enAsRelIndicator;                   /* Access Stratum Release Indicator             */

    PS_BOOL_ENUM_UINT8                      enHSDSCHSupport;                    /* �Ƿ�֧��enHSDSCHSupport�ı�־                */
    VOS_UINT8                               ucHSDSCHPhyCategory;                /* ֧��HS-DSCH���������ͱ�־                  */

    PS_BOOL_ENUM_UINT8                      enMacEhsSupport;
    VOS_UINT8                               ucHSDSCHPhyCategoryExt;
    PS_BOOL_ENUM_UINT8                      enMultiCellSupport;                 /* �Ƿ�֧�� Multi cell support,���֧��MultiCell,Ex2���� */
    VOS_UINT8                               ucHSDSCHPhyCategoryExt2;            /* HS-DSCH physical layer category extension 2 */

    PS_BOOL_ENUM_UINT8                      enCellSpecTxDiversityForDc;         /*  This IE is optionally present if Dual-Cell HSDPA is supported. Otherwise it is not needed.
                                                                                    The IE is not needed in the INTER RAT HANDOVER INFO message. Otherwise, it is optional*/
    PS_BOOL_ENUM_UINT8                      enReserve2;                         /* ����enEFDPCHSupport��ԭλ����Ϊreserved */
    PS_BOOL_ENUM_UINT8                      enEDCHSupport;                      /* �Ƿ�֧��EDCH�ı�־                           */
    VOS_UINT8                               ucEDCHPhyCategory;                  /* ֧��UPA�ĵȼ�                                */
    PS_BOOL_ENUM_UINT8                      enSuppUl16QAM;                      /* �Ƿ�֧������16QAM����֧��ʱucEDCHPhyCategoryExt����Ч */
    VOS_UINT8                               ucEDCHPhyCategoryExt;               /* ���е��ز�֧��16QAMʱ����д7 */
    PS_BOOL_ENUM_UINT8                      enSuppEDpdchInterpolationFormula;   /* 16QAM������أ��Ƿ�֧��E-DPDCH power interpolation formula */
    PS_BOOL_ENUM_UINT8                      enReserve3;                         /* ����enSuppHsdpaInFach��ԭλ����Ϊreserved */
    PS_BOOL_ENUM_UINT8                      enReserve4;                         /* ����enSuppHsdpaInPch��ԭλ����Ϊreserved */

    PS_BOOL_ENUM_UINT8                      enMacIsSupport;                     /* �Ƿ�֧��MAC_I/MAC_Is */

    PS_BOOL_ENUM_UINT8                      enReserve5;                         /* ����enFDPCHSupport��ԭλ����Ϊreserved */

    PS_BOOL_ENUM_UINT8                      enReserve6;                         /* ����enHsscchLessSupport��ԭλ����Ϊreserved */
    PS_BOOL_ENUM_UINT8                      enReserve7;                         /* ����enUlDpcchDtxSupport��ԭλ����Ϊreserved */

    PS_BOOL_ENUM_UINT8                      enAdjFreqMeasWithoutCmprMode;       /* �Ƿ�֧�� Adjacent Frequency measurements without compressed mode */

    PS_BOOL_ENUM_UINT8                      enMimoSingleStreamStrict;           /* �Ƿ�����ֻ��ʹ�õ���MIMO */
    PS_BOOL_ENUM_UINT8                      enMimoWithDlTxDiversity;            /* R9���ԣ���MIMO����ʱ�����п����ŵ��Ƿ�����ʹ�÷ּ� */

    /* V7r1 ˫ģ����NV�� LTE���� */
    PS_BOOL_ENUM_UINT8                      enSptAbsPriBasedReselInUtra;        /* ֧��UTRA�е����ȼ���ѡ��Ĭ��Ϊ0��1Ϊ֧�֣�0Ϊ��֧�� */

    VOS_UINT8                               ucHSDSCHPhyCategoryExt3;            /* HS-DSCH physical layer category extension 3 */
    PS_BOOL_ENUM_UINT8                      enDcMimoSupport;                    /* �Ƿ�֧��DC+MIMO */
    PS_BOOL_ENUM_UINT8                      enSuppCommEDCH;                     /* E-RACH�������� */

    PS_BOOL_ENUM_UINT8                      enDcUpaSupport;                     /* �Ƿ�֧��DC UPA�ı�־ */
    VOS_UINT8                               ucEDCHPhyCategoryExt2;              /* EDCH  category extension 2*/
    PS_BOOL_ENUM_UINT8                      enEdpdchGainFactorFlg;              /* E-DPDCH���ʻ�������ʹ�ܱ�־λ */
    PS_BOOL_ENUM_UINT8                      enReserve8;                         /* ����enHo2EutranUnSupportFlg��ԭλ����Ϊreserved */
    PS_BOOL_ENUM_UINT8                      enReserve9;                         /* ����enEutranMeasUnSupportFlg��ԭλ����Ϊreserved */
    VOS_UINT8                               aucReserve2[5];
}WAS_NVIM_UE_CAPA_CUSTOMED_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_RLC_CAPBILITY_STRU
 �ṹ˵��  : RLC�����ϱ�����NV         ID:3048 en_NV_Item_WAS_Rlc_Capbility
 DESCRIPTION: RLC�����ϱ����Ʋ���
*****************************************************************************/
typedef struct
{
    WAS_TOTAL_RLC_AM_BUF_SIZE_ENUM_UINT8    enTotalRlcAmBufSize;                /* TOTAL RLC AM BUF���� */
    WAS_MAX_AM_ENTITY_NUM_UINT8             enMaxAmEntity;                      /* ���AMʵ����� */
    VOS_UINT8                               ucReserve1;
    VOS_UINT8                               ucReserve2;
}WAS_NVIM_RLC_CAPBILITY_STRU;

/*****************************************************************************
 �ṹ��    : WAS_PRACH_PARA_STRU
 �ṹ˵��  : W�·����������ʱ������Ҫ�Ĳ���        ID:9023 en_NV_Item_Wcdma_Prach_Para
 DESCRIPTION: W�·����������ʱ������Ҫ�Ĳ���
*****************************************************************************/
typedef struct
{
    VOS_INT8                                cInitTxPowerExt;                    /*Range:[0,20]*/ /* ��ʼ���书�ʵ��ڲ��� */
    VOS_UINT8                               ucPrbRetransMaxExt;                 /* ������뷢��������ڲ��� */
    VOS_UINT8                               aucReserve[2];
    VOS_UINT32                              ulASC0Para;                         /* �����������������㷨���� */
}WAS_PRACH_PARA_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_FASTDORM_PARA_STRU
 �ṹ˵��  : FASTDORM������ز���                   ID:9027 en_NV_Item_Fastdorm_Para
 DESCRIPTION: FASTDORM������ز���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucWasMaxSCRINumInPCH;               /* WAS�����P��DRX����<CN��DRX���ڳ�����СֵʱSCRI����ʹ���  */
    VOS_UINT8                               ucNasRetryInterval;                 /* NAS�����Retryʱ����������Ϊ��λ*/
    VOS_UINT8                               ucT323Default;                      /* �������õ�T323Ϊ0ʱʹ��NV�����õ�Ĭ��ֵ */
    VOS_UINT8                               aucReserved[1];                     /* Ԥ�� */
}NVIM_FASTDORM_PARA_STRU;

/*****************************************************************************
 �ṹ��    : WAS_CSFB_PPAC_STRU
 �ṹ˵��  : �������������Ϣ                       ID:9050 en_NV_Item_CSFB_PPAC_Para
 DESCRIPTION: �������������Ϣ��
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enSuppPPACFlg;                      /* PPAC��־ */
    PS_BOOL_ENUM_UINT8                      enCsfbRcvAllSibFlg;                 /* CSFB�Ƿ�ȫ��ϵͳ��Ϣ��־ ,Ĭ��ֵΪ0����־��ȫ�� */
    PS_BOOL_ENUM_UINT8                      enCsfbSearchFailFastReturnFlg;      /* W��ָ��Ƶ���С������ʧ�ܺ󣬲��ٷ���ȫƵ��������������LTE */
    VOS_UINT8                               ucFrWaitGmmProcAndTimerLen;         /* CSFB���̽�����FRʱ�Ƿ�ȴ�GMM���̽�����Ǻ͵ȴ�FR��ʱ�� */
}WAS_CSFB_PPAC_STRU;

/*****************************************************************************
 �ṹ��    : WAS_3G_TO_2G_STRU
 �ṹ˵��  : 3G��2G��ѡ�Ż�NV
 DESCRIPTION: 3G��2G��ѡ�Ż�������
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enSuppDetectCellSearchFlg;          /* ֧����������־ */
    VOS_UINT8                               ucDetectCellSearchThreshold;        /* ������������Ĭ������ֵ */
    VOS_UINT8                               ucStartDetectCellSearchCnt;         /* ��������������Ҫ����С����������ֹͣ���޵Ĵ��� */
    VOS_UINT8                               ucStopDetectCellSearchCnt;          /* ֹͣ����������Ҫ����С����������ֹͣ���޵Ĵ��� */
}WAS_3G_TO_2G_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_REPORT_CELL_SIGN_STRU
 �ṹ˵��  : С���ź�ǿ���ϱ���ʱ�����Ⱥ�RSCP�仯������ֵ   ID:9067 en_NV_Item_Report_Cell_Sign
 DESCRIPTION: С���ź�ǿ���ϱ���ʱ�����Ⱥ�RSCP�仯������������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucPeriodTimerLen;                   /* ��ʱ�����ȣ�Range:[0,255],��λ:�� */
    VOS_UINT8                               ucRscpThreshold;                    /* RSCP������ֵ��Range:[0,255],��λ:db */
    VOS_UINT8                               ucEcn0Threshold;                    /* ECN0������ֵ��Range:[0,255],��λ:db */
    VOS_UINT8                               aucReserve1[1];
}WAS_NVIM_REPORT_CELL_SIGN_STRU;

/*****************************************************************************
 �ṹ��    : WAS_CUSTOMIZED_PARA_STRU
 �ṹ˵��  : W��������ѡ����                        ID: 9069 en_NV_Item_Was_Customized_Para
 DESCRIPTION: W��������ѡ����������
*****************************************************************************/
typedef struct
{
    /* Was_Customized_Para */
    VOS_UINT8                               ucSbmMaskSupportType;               /* ������������:�ڷǹ�������ʱ�Ƿ�����2.1GHzƵ�Σ�0x0:������,0x1:���� */
    VOS_UINT8                               ucDchOosSearchInterRatFlg;          /* DCH�´���CUʱT314����ʱ�Ƿ�������ϵͳ���*/
    VOS_UINT8                               ucOosAllBandHistoryFreqValidFlg;    /* ��������ָ��PLMN�ѣ���ʷƵ��������ȫƵ������ʱ��������������ʷƵ�� */
    VOS_UINT8                               ucOosAllBandAddHistoryFreqCnt;      /* ��������ȫƵ������ָ��Ƶ���б���ʱ������ʷƵ��ĸ��� */
    VOS_UINT32                              ulDocomMcc;                         /* DOCOMO PLMN MCC */
    VOS_UINT32                              ulDocomMnc;                         /* DOCOMO PLMN MNC */
}WAS_CUSTOMIZED_PARA_STRU;

/*****************************************************************************
 �ṹ��    : WAS_W2L_CEll_RESEL_OFFSET_STRU
 �ṹ˵��  : W2L��ѡ�͹������NV�ṹ                ID:9124 en_NV_Item_LOW_POWER_Cell_Resel_OffSet
 DESCRIPTION: W2L��ѡ�͹������������
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enOffsetEnableFlg;                  /* offset NVʹ�ܱ��    */
    PS_BOOL_ENUM_UINT8                      enR8BasedCellReselFlg;              /* offset NVʹ�ܱ��,enW2LR8BasedCellReselFlg���ܿ���    */
    VOS_UINT8                               aucReserve1[2];                     /* ����λ */
    VOS_UINT16                              usWRscpThresh;                      /* W����С��RSCP���ޣ��������޲���ʹ����ѡOFFSET��ȡ����ֵ,Range:[0,928]*/
    VOS_UINT16                              usWEcnoThresh;                      /* W����С��ECNO���ޣ��������޲���ʹ����ѡOFFSET��ȡ����ֵ,Range:[0,160]*/
    VOS_UINT16                              usEutranCellRsrpOffset;             /* EUTRANС��RSRP OFFSET,Range:[0,1040] */
    VOS_UINT16                              usEutranCellRsrqOffset;             /* EUTRANС��RSRQ OFFSET,Range:[0,160] */
    VOS_UINT16                              usLteServRsrpOffSet;                /*Range:[0,784]*/
    VOS_UINT16                              usLteServRsrqOffSet;                /*Range:[0,256]*/
    VOS_UINT16                              usUtranRscpOffSet;                  /*Range:[0,376]*/
    VOS_UINT16                              usUtranEcn0OffSet;                  /*Range:[0,248]*/
}WAS_W2L_CEll_RESEL_OFFSET_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_UE_POSITION_CAPABILITIES_STRU
 �ṹ˵��  : UE�Ķ�λ�����ṹ                       ID:9093 en_NV_Item_UE_POSITION_CAPABILITIES
 DESCRIPTION: UE��λ����������
*****************************************************************************/
typedef struct
{
   VOS_UINT8                                ucGpsUeAssisted;                    /* UE������ GPS ��λ����: 0: ��֧��, 1: ֧�� */
   VOS_UINT8                                ucGpsUeBased;                       /* ����UE�� GPS ��λ����: 0: ��֧��, 1: ֧�� */
   VOS_UINT8                                ucGpsUeStandalone;                  /* UE������ GPS ��λ����: 0: ��֧��, 1: ֧�� */
   VOS_UINT8                                ucGpsValidInPch;                    /* PCH̬�µ� GPS ��λ����(��W ģʹ��): 0: ��֧��, 1: ֧�� */
   VOS_UINT8                                ucLcsVaCapability;                  /* LCS VALUE ADD��λ����(��G ģʹ��): 0: ��֧��, 1: ֧�� */
   VOS_UINT8                                ucAddPosCapability;                 /* ���ӵĶ�λ����(��G ģʹ��): 0: ��֧��, 1: ֧�� */

   VOS_UINT8                                aucRsv[2];                          /* ����λ */
}NVIM_UE_POSITION_CAPABILITIES_STRU;


/*****************************************************************************
 �ṹ��    : WAS_NVIM_PLMN_SEARCH_THRESHOLD_STRU
 �ṹ˵��  : ��NV�ж�ȡ�������ȫƵ������������      ID:8483 en_NV_Item_Plmn_Search_Threshold
 DESCRIPTION: ȫƵ������������ʹ�õ�RSSI��������
*****************************************************************************/
typedef struct
{
    VOS_INT8                                cBgSearchHighQulityThreshold;       /* ������RSSI���������� */
    VOS_INT8                                cOtherSearchHighQulityThreshold;    /* ������RSSI���������� */
    VOS_INT8                                cLowQulityThreshold;                /* RSSI���������� */
    VOS_UINT8                               ucInterRatMeasOffset;               /* ����ͣ��ϵͳ���������ϼ�ȥ�ĳ��� */
}WAS_NVIM_PLMN_SEARCH_THRESHOLD_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_PLMN_LIST_SEARCH_THRESHOLD_STRU
 �ṹ˵��  : ��NV�ж�ȡ�������PLMN LISTȫƵ������������
 DESCRIPTION: ����W��PLMN LIST����ʱȫƵ����������
*****************************************************************************/
typedef struct
{
    VOS_INT8                                cPlmnListSrchHighQulityThreshold;   /* PLMN����RSSI���������� */
    VOS_INT8                                cPlmnListSrchLowQulityThreshold;    /* PLMN����RSSI���������� */
    VOS_INT8                                cReserve1;                           /* ���� */
    VOS_INT8                                cReserve2;                           /* ���� */
    VOS_INT8                                cReserve3;                           /* ���� */
    VOS_INT8                                cReserve4;                           /* ���� */
    VOS_INT8                                cReserve5;                           /* ���� */
    VOS_INT8                                cReserve6;                           /* ���� */
}WAS_NVIM_PLMN_LIST_SEARCH_THRESHOLD_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_PLMN_SEARCH_FLOW_STRU
 �ṹ˵��  : ��NV�ж�ȡ�������ȫƵ������������     ID:8484  en_NV_Item_Plmn_Search_Flow
 DESCRIPTION: ȫƵ�����������е��������̿���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucBgLowQulityFreqScrhFlag;          /* �����ѵ�����Ƶ���Ƿ���Ҫ�ѱ�־*/
    VOS_UINT8                               ucBgMultipleFreqScrhFlag;           /* �����ѱ�Ƶ���Ƿ���Ҫ�ѱ�־*/
    VOS_UINT8                               ucPlmnListLowQulityFreqScrhFlag;    /* PLMNLIST������Ƶ���Ƿ���Ҫ�ѱ�־*/
    VOS_UINT8                               ucPlmnListMultipleFreqScrhFlag;     /* PLMNLIST��Ƶ���Ƿ���Ҫ�ѱ�־*/
    VOS_UINT8                               ucOtherLowQulityFreqScrhFlag;       /* �����������̵�����Ƶ���Ƿ���Ҫ�ѱ�־*/
    VOS_UINT8                               ucOtherMultipleFreqScrhFlag;        /* �����������̱�Ƶ���Ƿ���Ҫ�ѱ�־*/
    VOS_UINT8                               aucReserve1[2];
}WAS_NVIM_PLMN_SEARCH_FLOW_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_ALL_BAND_SEARCH_PARA_STRU
 �ṹ˵��  : ��NV�ж�ȡ�������ȫƵ�����������ò��� ID:8485 en_NV_Item_All_Band_Search_Para
 DESCRIPTION: ȫƵ�����������е�������������
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulRawScanFftFlag;                   /* ����band�Ĵ�ɨ�㷨 */
    VOS_UINT8                               aucReserv[2];                       /* 4�ֽڶ��룬�����ֽ� */
    VOS_UINT8                               ucFftFineScanFreqOffsetNum;         /* FFT�㷨ϸɨʱȡ����Ƶ���ƫ��Ƶ�����*/
    VOS_UINT8                               ucCellSearchFreqNum;                /* ϸɨ��һ��Ƶ����Ҫ��С��������Ƶ�����*/
    VOS_UINT8                               ucMultipleSrchFreqOffsetNum;        /* ��Ƶ������ʱ��ȡ����Ƶ���ƫ��Ƶ�����*/
    VOS_UINT8                               ucPlmnListHistoryFreqThreshold;     /* PLMNLIST����ʱ��ʷƵ��ĸ�������*/
    VOS_UINT8                               ucWcdmaRawScanFftStep;              /* FFT�㷨ʱ����������*/
    VOS_UINT8                               ucWcdmaRawScanRssiStep;             /* RSSI�㷨ʱ����������*/
}WAS_NVIM_ALL_BAND_SEARCH_PARA_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_WCDMA_IND_FREQ_STRU
 �ṹ˵��  : WCDMA IND FREQ��Ϣ�ṹ                 ID:8248 en_NV_Item_Wcdma_Ind_Freq UINT�ṹ���ɽṹ
 DESCRIPTION: �����û����õ�ָ��Ƶ��ֵ����Ƶ�����ȼ�������NV���д洢������Ƶ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usUserIndFreq;
    VOS_UINT8                               aucReserve[2];
}WAS_NVIM_WCDMA_IND_FREQ_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_PDCP_LOSSLESS_SWITCH_STRU
 �ṹ˵��  : PDCP LOSSLESS SWITCH��Ϣ�ṹ           ID:8324 en_NV_Item_PDCP_LossLess_Switch UINT�ṹ���ɽṹ
 DESCRIPTION: �Ƿ�֧��PDCP����Ǩ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usNvPdcpLossLessSwitch;    /* PDCP ����Ǩ�ƿ���,Range:[0,1]*/
    VOS_UINT8                               aucReserve[2];
}WAS_NVIM_PDCP_LOSSLESS_SWITCH_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_OLPC_MAPWEIGHT_PARA_STRU
 �ṹ˵��  : WCDMA OLPC MAPWEIGHT PARA��Ϣ�ṹ      ID:8525 en_NV_Item_Wcdma_OLPC_MapWeight_Para UINT�ṹ���ɽṹ
 DESCRIPTION: ��ͬ���ʶ�Ӧ��mapweight��ֵ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              ausMapWeight[WAS_MAPWEIGHT_MAX_NUM];    /*TOT OLPC�㷨����*/
}WAS_NVIM_OLPC_MAPWEIGHT_PARA_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_W_CELLSRCH_MEAS_PARA
 �ṹ˵��  : W_CELLSRCH_MEAS_PARA��Ϣ�ṹ           ID:8256 ����NV��
 DESCRIPTION: �ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usSysInfoRcvOptimSwitch;
    VOS_UINT16                              usPIOptimSwitch;
    VOS_UINT16                              usCellSrchMeasOptimSwitch;
    VOS_UINT16                              usCellSrchPeriod;
    VOS_INT16                               sCsStartRSCPThreshold;
    VOS_INT16                               sCsStopRSCPThreshold;
    VOS_INT16                               sCsStartECN0Threshold;
    VOS_INT16                               sCsStopECN0Threshold;
    VOS_UINT16                              usMeasIntraNCellNum;
    VOS_UINT16                              usMeasFilterFactor;
}WAS_NVIM_W_CELLSRCH_MEAS_PARA;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_UE_ACCESS_CAPA
 �ṹ˵��  : UE_ACCESS_CAPA��Ϣ�ṹ                 ID:8264 ����NV��
 DESCRIPTION: �ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               aucWasUeAccessCapa[WAS_UE_ACCESS_CAPA_NUM];
}WAS_NVIM_UE_ACCESS_CAPA;

/*****************************************************************************
 �ṹ��    : NV_ITEM_BG_IRAT_LIST_STRU
 �ṹ˵��  : BG_IRAT_LIST��Ϣ�ṹ                   ID:8487 ����NV��
 DESCRIPTION: �ѷ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucValid;
    VOS_UINT8                               ucReserve;
    VOS_UINT16                              usBgIratListTime;                   /*Range:[6,65536]*/
}NV_ITEM_BG_IRAT_LIST_STRU;


/*****************************************************************************
 �ṹ��    : WAS_NVIM_T_CR_MAX_STRU
 Э����  : 10.3.2.3 Cell selection and re-selection info for SIB3/4
 ASN.1���� : T_CRMax
 �ṹ˵��  : �����ƶ�������
 DESCRIPTION: �����ƶ�������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucTCrMax;       /* ��λ:10s��0��ʾ��ʹ�ã�(0, 3, 6, 12, 18, 24) */
    VOS_UINT8                           ucNCr;          /* ucTCRMax��Ϊ0ʱ��Ч��default 8��Integer(1..16) */
    VOS_UINT8                           ucTCrMaxHyst;   /* ucTCRMax��Ϊ0ʱ��Ч����λ:1s��0��ʾ��ʹ�ã�(0, 10, 20, 30, 40, 50, 60, 70) */
    VOS_UINT8                           ucReserve;
}WAS_NVIM_T_CR_MAX_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NV_OOS_RL_FAIL_PARA_STRU
 �ṹ˵��  :
 DESCRIPTION: ����̬����������ʱ���ĳ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucOosTimerLen;
    PS_BOOL_ENUM_UINT8                      enT313UseDefaultValueFlg;
    VOS_UINT8                               ucReserve[2];
}WAS_NV_OOS_RL_FAIL_PARA_STRU;
/*****************************************************************************
 �ṹ��    : NVIM_SBM_CUSTOM_DUAL_IMSI_STRU
 �ṹ˵��  : �ձ���EMOIBLE�����е�plmn��Ϣ����plmn LTE��Ҫ�õ�
 DESCRIPTION: �ձ���EMOIBLE�����е�plmn��Ϣ����plmn LTE��Ҫ�õ�
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           aucJapanEmMcc[3];  /* �ձ�EMBILE�����MCC��Ϣ */
    VOS_UINT8                           ucJapanEmMncCount; /* �ձ�EMBILE�����MNC���ȣ�2����3 */
    VOS_UINT8                           aucJapanEmMnc[3];  /* �ձ�EMBILE�����MNC��Ϣ,����Ϊ��λ����λ��f */
    VOS_UINT8                           ucReserve;
} NVIM_JAPAN_EM_PLMN_INFO_STRU;
/*****************************************************************************
 �ṹ��    : NVIM_SBM_CUSTOM_DUAL_IMSI_STRU
 �ṹ˵��  : ����˫imsi���غ�MCC��Ϣ
 DESCRIPTION: ����˫imsi���غ�MCC��Ϣ
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucSbmCustomDualImsiFlg;
    VOS_UINT8                           aucJapanMcc[3];      /* �ձ�������һ��MCC */
    VOS_UINT8                           aucJapanMcc1[3];     /* �ձ���������һ��MCC */
    VOS_UINT8                           ucJapanSbmMncCount;  /* �ձ����������MNC���� */
    VOS_UINT32                          aulJapanSbmMnc[5];   /* �ձ�������������5��MNC,����չ */
    NVIM_JAPAN_EM_PLMN_INFO_STRU        stJapanEmPlmnInfo;   /* �ձ���EMOBILE�����plmn��Ϣ */
    VOS_UINT8                           ucSbmOosTimerLen;    /* פ�����ձ������������������Ķ�ʱ������ʱǰֻ��band1��band11 */
    VOS_UINT8                           aucReserve[3];
} NVIM_SBM_CUSTOM_DUAL_IMSI_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_SBM_CUSTOM_DUAL_IMSI_STRU_NEW
 �ṹ˵��  : ����˫imsi���غ�MCC��Ϣ
 DESCRIPTION: ����˫imsi���غ�MCC��Ϣ
*****************************************************************************/

typedef struct
{
    VOS_UINT8                           ucSbmCustomDualImsiFlg;/* ˫Imsi�ܿ��� */
    VOS_UINT8                           ucDualImsiCellSearchChkFlg; /* �Ƿ���������ʱ��band���  */
    VOS_UINT8                           aucReserve[2];       /* ����λ */
    VOS_UINT8                           ucSbmOosTimerLen;    /* פ�����ձ������������������Ķ�ʱ�� */
    VOS_UINT8                           aucJapanMcc[3];      /* �ձ�������һ��MCC */
    VOS_UINT8                           aucJapanMcc1[3];     /* �ձ���������һ��MCC */
    VOS_UINT8                           ucJapanSbmMncCount;  /* �ձ����������MNC���� */
    VOS_UINT32                          aulJapanSbmMnc[5];   /* �ձ�������������5��MNC,����չ */
    NVIM_JAPAN_EM_PLMN_INFO_STRU        stJapanEmPlmnInfo;   /* �ձ���EMOBILE�����plmn��Ϣ */
    VOS_UINT32                          ulSbmSupBandInJapan;         /* �������ձ�����֧�ֵ�band�������ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
    VOS_UINT32                          ulEmSupBandInJapan;          /* EM���ձ�����֧�ֵ�band�������ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
    VOS_UINT32                          ulOtherOperateSupBandInJapan;/* ������Ӫ�����ձ�����֧�ֵ�band�������ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
    VOS_UINT32                          ulSupBandOutJapan;           /* �ձ�����֧�ֵ�band,���ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
    VOS_UINT32                          ulLteDelWcdmaBandInJapan;    /* �ձ�����פ��Lteʱ��Ҫ���ε�W��band,���ȫ1����Ϊ��Ҫ�������е�W��band */
} NVIM_SBM_CUSTOM_DUAL_IMSI_STRU_NEW;

/*****************************************************************************
 �ṹ��    : NVIM_SBM_CUSTOM_DUAL_IMSI_STRU
 �ṹ˵��  : WAS��˫imsi��Ϣ
 DESCRIPTION: WAS��˫imsi��Ϣ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enNvSbmDualImsiFlg;     /* ����˫imsi�����Ƿ�򿪣�Ĭ�Ϲرգ� */
    PS_BOOL_ENUM_UINT8                      enDualImsiCellSearchChkFlg; /* �Ƿ���������ʱ��band���  */
    VOS_UINT8                               ucSbmOosTimerLen;       /* פ�����ձ������������������Ķ�ʱ������ʱǰֻ��band1��band11 */
    VOS_UINT8                               ucJapanSbmMncCount;     /* �ձ����������MNC���� */
    VOS_UINT32                              aulJapanSbmMnc[5];      /* �ձ�������������5��MNC,����չ֧�ֲ��� */
    VOS_UINT32                              ulJapanMcc;             /* �ձ�������һ��MCC */
    VOS_UINT32                              ulJapanMcc1;            /* �ձ���������һ��MCC */
    VOS_UINT32                              ulEmMcc;                /* �ձ�����embile�����MCC */
    VOS_UINT32                              ulEmMnc;                /* �ձ�����embile�����MNC */
    VOS_UINT32                              ulSbmSupBandInJapan;    /* �������ձ�����֧�ֵ�band�������ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
    VOS_UINT32                              ulEmSupBandInJapan;      /* EM���ձ�����֧�ֵ�band�������ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
    VOS_UINT32                              ulOtherOperateSupBandInJapan;/* ������Ӫ�����ձ�����֧�ֵ�band�������ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
    VOS_UINT32                              ulSupBandOutJapan;        /* �ձ�����֧�ֵ�band,���ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
} WAS_NV_SBM_CUSTOM_DUAL_IMSI_STRU;
/*****************************************************************************
 �ṹ��    : WAS_NV_YOIGO_CUSTOM_PARA_INFO_STRU
 �ṹ˵��  : YOIGO���ƿ��غ�PLMN��Ϣ
 DESCRIPTION: YOIGO���ƿ��غ�PLMN��Ϣ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8            enYoigoCustomSwitchFlag;          /* YOIGO���ƿ����Ƿ�򿪱�־��Ĭ�Ϲر� */
    VOS_UINT8                     aucReserve[3];                    /* ����λ */
    VOS_UINT32                    ulYoigoMcc;                       /* YOIGO�����MCC */
    VOS_UINT32                    ulYoigoMnc;                       /* YOIGO�����MNC */
}WAS_NV_YOIGO_CUSTOM_PARA_INFO_STRU;

typedef struct
{
    VOS_UINT8                   aucUeWcdmaBandPriodiry[NVIM_MAX_FDD_FREQ_BANDS_NUM];
}NVIM_WAS_FREQ_BAND_PRIORITY_STRU;

/*****************************************************************************
 �ṹ��    : WAS_ERR_LOG_CS_HO_LEN_STRU
 �ṹ˵��  : Errorlog:CS �����л���ʱ��
 DESCRIPTION: Errorlog:CS �����л���ʱ��
 �޸���ʷ  :
 1.��    ��    : 2014��9��2��
   ��    ��    : 00134354
   �޸�����    : �½��ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usCSHandoverTime;                   /* cs�л���Ҫ��Ĭ��ʱ�䣬������ʱ��������ϱ� */
    VOS_UINT8                               aucReserve[2];                     /* ����λ */
}WAS_ERR_LOG_CS_HO_LEN_STRU;

/*****************************************************************************
 �ṹ��    : NVIM_ERR_LOG_STAY_SELF_RAT_TIMER_THRESHOLD_STRU
 �ṹ˵��  : �ڱ�ģͣ����ʱ�䳤�����ޣ�Ĭ��Ϊ10����
 DESCRIPTION: �ڱ�ģͣ����ʱ�䳤������
 �޸���ʷ  :
 1.��    ��    : 2015��2��6��
   ��    ��    : 00134354
   �޸�����    : �½��ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usTimerLength;                   /* �ڱ�ģͣ����ʱ�䳤��, ��λΪ���� */
    VOS_UINT8                               aucReserve[2];                  /* ����λ */
}NVIM_ERR_LOG_STAY_SELF_RAT_TIMER_THRESHOLD_STRU;


/*****************************************************************************
 �ṹ��    : WAS_ERR_LOG_3G_NOT_TO_LTE_TIMER_STRU
 �ṹ˵��  : ��Wģפ����ʱ�䳤�ȣ�Ĭ��Ϊ10����
 DESCRIPTION: ��Wģפ����ʱ�䳤��
 �޸���ʷ  :
 1.��    ��    : 2015��2��6��
   ��    ��    : 00134354
   �޸�����    : �½��ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              usCampOnTime;                   /* ��Wģ�µ�ʱ�䣬������ʱ��������ϱ� */
    VOS_UINT8                               aucReserve[2];                  /* ����λ */
}WAS_ERR_LOG_3G_NOT_TO_LTE_TIMER_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NV_T320_CTRL_STRU
 �ṹ˵��  : t320�Ƿ�֧�ֿ���
 DESCRIPTION: t320�Ƿ�֧�ֿ���
 �޸���ʷ  :
 1.��    ��    : 2015��2��6��
   ��    ��    : 00134354
   �޸�����    : �½��ṹ��
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                              enT320SupportFlg;               /* t320�Ƿ�֧�ֿ��� */
    VOS_UINT8                                       aucReserve[3];                  /* ����λ */
}WAS_NV_T320_CTRL_STRU;
/*****************************************************************************
 �ṹ��    : WAS_CSFB_MCC_BAND_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :���Ҷ���ȫƵ��������band���ݽṹ
 DESCRIPTION: ���Ҷ���ȫƵ��������band���ݽṹ
*****************************************************************************/
typedef struct
{
    WAS_PLMN_ID_STRU        stPlmn;
    VOS_UINT32              ulSupportWBand;
}WAS_CSFB_MCC_BAND_STRU;

/*****************************************************************************
 �ṹ��    : WAS_CSFB_MCC_BAND_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CSFBʱ���ݹ��Ҷ���ȫƵ��������band
 DESCRIPTION: CSFBʱ���ݹ��Ҷ���ȫƵ��������band
*****************************************************************************/
typedef struct
{
    VOS_UINT32                   ulMccNum;                                      /* ָʾ������MCC��Ч */
    WAS_CSFB_MCC_BAND_STRU       astUeCapMccBand[WAS_MAX_CSFB_MCC_BAND_NUM];
}WAS_CSFB_MCC_BAND_INFO_STRU;

/*****************************************************************************
 ö �� ��  : WAS_CS_NOT_RCV_SPEC_SIB_ENUM
 ö��˵��  : cs�����в����ܲ���ϵͳ��Ϣ��bitmap
 �޸���ʷ  :
  1.��  ��   : 2016��10��10��
    ��  ��   : w00380530
    �޸����� : �����ɽṹ

*****************************************************************************/
enum WAS_CS_NOT_RCV_SPEC_SIB_ENUM
{
 
    WAS_CS_NOT_RCV_SPEC_SIB11BIS               = 0x1,   /* bit0: ������sib11bis */
    WAS_CS_NOT_RCV_SPEC_SIB12                  = 0x2,   /* bit1: ������sib12 */
    WAS_CS_NOT_RCV_SPEC_SIB18                  = 0x4,   /* bit2: ������sib18 */
    WAS_CS_NOT_RCV_SPEC_SIB19                  = 0x8,   /* bit3: ������sib19 */
    
    WWAS_CS_NOT_RCV_SPEC_SIB_BUTT
};
typedef VOS_UINT8 WAS_CS_NOT_RCV_SPEC_SIB_ENUM_UINT8;

/*****************************************************************************
 �ṹ��    : WAS_NV_DMCR_CTRL_INFO_STRU
 �ṹ˵��  : DMCR����
 DESCRIPTION: DMCR����
 �޸���ʷ  :
 1.��    ��    : 2015��2��6��
   ��    ��    : 00134354
   �޸�����    : �½��ṹ��
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                     enCsDmcrFlg;            /* L2W�ض���csҵ���CSFB������������DMCR���� */
    PS_BOOL_ENUM_UINT8                     enPsDmcrFlg;            /* L2W�ض���psҵ��DMCR���� */
    PS_BOOL_ENUM_UINT8                     enCuDmcrFlg;            /* CU������DMCR���� */
    PS_BOOL_ENUM_UINT8                     enCsNotRcvSpecSibFlg;   /* cs�����в����ܲ���ϵͳ��Ϣ�ı��λ */
    WAS_CS_NOT_RCV_SPEC_SIB_ENUM_UINT8     ucNotRcvSpecSibBitmap;  /* cs�����в����ܲ���ϵͳ��Ϣ��bitmap */
    VOS_UINT8                              ucReserve1;            /* ����λ */
    VOS_UINT8                              ucReserve2;            /* ����λ */
    VOS_UINT8                              ucReserve3;            /* ����λ */
}WAS_NV_DMCR_CTRL_INFO_STRU;

/*****************************************************************************
 �ṹ��    : WAS_CSFB_SEARCH_CTRL_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CSFBʱ���ݶ�ʱ�������С��
 DESCRIPTION: CSFBʱ���ݶ�ʱ�������С������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                          ucCsfbSearchCtrlInfo;               /* CSFBʱW�������Ŀ���NV,��1bit��ʾ�Ƿ��������С��
                                                                              ��2bit��ʾ�Ƿ�����פ��ָ��С������ǿ��С�� */
    VOS_UINT8                          ucCsfbDetectCellNum;                /* �������С���ĸ��� */
    VOS_UINT8                          ucCsfbReprotCellNum;                /* ָ��Ƶ������ʱ�ϱ���С������ */
    VOS_UINT8                          ucReserve1;
}WAS_CSFB_SEARCH_CTRL_INFO_STRU;
/*****************************************************************************
 �ṹ��    : WAS_AC_CTRL_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����̬�����AC������Ϣ����NV
 DESCRIPTION: ����̬�����AC������Ϣ��������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                          ucIgnorAcInfo;                      /* ����̬�����AC������Ϣ */
    VOS_UINT8                          ucReserve1;
    VOS_UINT8                          ucReserve2;
    VOS_UINT8                          ucReserve3;
}WAS_AC_CTRL_INFO_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NV_RPT_CONN_LOC_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : �ϱ�����̬λ����Ϣ����
 DESCRIPTION: �ϱ�����̬λ����Ϣ����
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                     enRptConnLocInfoFlg;                 /* �ϱ�����̬λ����Ϣ���� */
    VOS_UINT8                              ucRcvDchSysInfoTimerLen;             /* DCH�½�㲥����ʱ�䣬��λs */
    VOS_UINT8                              aucReserve1;
    VOS_UINT8                              aucReserve2;
}WAS_NV_RPT_CONN_LOC_INFO_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NV_RADOM_ACC_CTRL_STRU
 �ṹ˵��  : ����������NV
 DESCRIPTION: ��������������
 �޸���ʷ  :
 1.��    ��    : 2015��6��15��
   ��    ��    : j00169676
   �޸�����    : �½��ṹ��
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                              enModifyN300Flg;                /* �Ƿ���N300 */
    VOS_UINT8                                       ucN300Num;                      /* ����N300�Ĵ��� */
    VOS_UINT8                                       aucReserve1;                    /* ����λ */
    VOS_UINT8                                       aucReserve2;                    /* ����λ */
}WAS_NV_RADOM_ACC_CTRL_STRU;

/*****************************************************************************
 �ṹ��    : WAS_EUTRA_CELL_RESEL_CTRL_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : W2L��ѡ��ض��Ʋ��������ڽ��3G��4G��ʽ֮��ƽ���л�������
 DESCRIPTION: W2L��ѡ��ض��Ʋ��������ڽ��3G��4G��ʽ֮��Ƶ���л�������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                          ucJudgeL2WReselOrRedirTimeLen;           /* �ڸö�ʱ���涨ʱ�����ж��Ƿ���L2W��ѡ���ض��� */
    VOS_UINT8                          ucL2WReselOrRedirMaxNum;                 /* ����ΪucJudgeL2WReselOrRedirTimeLenʱ���ڴ�Lģ��ѡ���ض���Wģ�������� */
    VOS_UINT8                          ucW2LReselPenalTimeLen;                  /* W2L��ѡ�ͷ���ʱ��ʱ�� */
    VOS_UINT8                          ucW2LReselOffset;                        /* ���ͷ���ʱ������ʱ��W2L��ѡƫ�ò��� */
}WAS_EUTRA_CELL_RESEL_CTRL_INFO_STRU;
/*****************************************************************************
 �ṹ��    : WAS_NV_SBM_CUSTOM_DCDPA_CTRL_STRU
 �ṹ˵��  : �������ƶ�̬����DCDPA,��NV������3001
 DESCRIPTION: �������ƶ�̬����DCDPA,��NV������3001
 �޸���ʷ  :
 1.��    ��    : 2015��5��4��
   ��    ��    : x81004927 
   �޸�����    : �½��ṹ��
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8            enSbmUnSupDcdpa;
    PS_BOOL_ENUM_UINT8            enEmUnSupDcdpa;
    PS_BOOL_ENUM_UINT8            enOtherUnSupDcdpaInJapan;
    PS_BOOL_ENUM_UINT8            enUnSupDcdpaOutJapan;
    VOS_UINT8                     aucReserve1;                  /* ����λ */
    VOS_UINT8                     aucReserve2;                  /* ����λ */
    VOS_UINT8                     aucReserve3;                  /* ����λ */
    VOS_UINT8                     aucReserve4;                  /* ����λ */
}WAS_NV_SBM_CUSTOM_DCDPA_CTRL_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NV_SBM_CUSTOM_LTE_BAND_INFO_STRU
 �ṹ˵��  : ����˫imsi���غ�MCC��Ϣ,LTEԤ�Ƶ�band��Ϣ
 DESCRIPTION: ����˫imsi���غ�MCC��Ϣ,LTEԤ�Ƶ�band��Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucSbmCustomSrchFlg; /*  SBm ���� ���أ�LTEģʹ�ã����ڿ���˫IMSI��������������*/
    VOS_UINT8                           aucReserve[3];          /* ����λ */
    VOS_UINT32                          aulSbmSupBandInJapan[2];         /* �������ձ�����֧�ֵ�band�������ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
    VOS_UINT32                          aulEmSupBandInJapan[2];          /* EM���ձ�����֧�ֵ�band�������ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
    VOS_UINT32                          aulOtherOperateSupBandInJapan[2];/* ������Ӫ�����ձ�����֧�ֵ�band�������ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
    VOS_UINT32                          aulSupBandOutJapan[2];           /* �ձ�����֧�ֵ�band,���ȫ1����Ϊ֧�ֵ�band�������ƣ�֧��UE֧�ֵ�����band */
} WAS_NV_SBM_CUSTOM_LTE_BAND_INFO_STRU;
/*****************************************************************************
 �ṹ��    : WAS_NV_CAPBILITY_FDDLIST_CTRL_STRU
 �ṹ˵��  : FDD LIST�����ϱ�����NV
 DESCRIPTION: FDD LIST�����ϱ���������
 �޸���ʷ  :
 1.��    ��    : 2015��7��15��
   ��    ��    : j00169676
   �޸�����    : �½��ṹ��
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                              enFddList3CtrFlg;                /* �Ƿ���FDD LIST�����ϱ� */
    PS_BOOL_ENUM_UINT8                              enInterHoShortFddList3Flg;       /* �Ƿ���GSM��ѯ����ʱ���ü�LTE���� */
    PS_BOOL_ENUM_UINT8                              enShortExternIndFlg;             /* �Ƿ�ü�externInd���ϱ� */
    VOS_UINT8                                       aucReserve1;                     /* ����λ */
}WAS_NV_CAPBILITY_FDDLIST_CTRL_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NV_CELL_INDI_OFFSET_CTRL_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :D̬�²�������ʱ��С��ƫ�ò�������NV
 DESCRIPTION: D̬�²�������ʱ��С��ƫ�ò�����������
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                 enModifyCellIndiOffsetFlg;               /* �Ƿ���С��ƫ�õı�� */
    VOS_INT8                           cCellIndiOffsetHighThreshold;            /* �����ж��Ƿ���Ҫ����С��ƫ�õ�������� */
    VOS_INT8                           cCellIndiOffsetLowThreshold;             /* �����ж��Ƿ���Ҫ����С��ƫ�õ�������� */
    VOS_UINT8                          ucModifyOffsetForHighThreshold;          /* ������������������޵�С��ƫ�õ�ֵ */
    VOS_UINT8                          ucModifyOffsetForLowThreshold;           /* ������������������޵�С��ƫ�õ�ֵ */
    VOS_UINT8                          ucReserve1;                              /* ����λ */
    VOS_UINT8                          ucReserve2;                              /* ����λ */
    VOS_UINT8                          ucReserve3;                              /* ����λ */
}WAS_NV_CELL_INDI_OFFSET_CTRL_INFO_STRU;
/*****************************************************************************
 �ṹ��    : WAS_NV_NOISE_OPTMIZE_CTRL_STRU
 �ṹ˵��  : ��ˮ���Ż�����NV
 DESCRIPTION: ��ˮ���Ż���������
 �޸���ʷ  :
 1.��    ��    : 2015��7��15��
   ��    ��    : j00169676
   �޸�����    : �½��ṹ��
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                              enNoiseOptmizeFlg;              /* �Ƿ�����ˮ����� */
    PS_BOOL_ENUM_UINT8                              enSilenceFlg;                   /* �Ƿ�������Ĭ����� */
    VOS_UINT8                                       ucTimerLen;                     /* ��λs */
    PS_BOOL_ENUM_UINT8                              enNoiseOptmizeAllProcedureFlg;  /* ���ⳡ��������ˮ���Ż����� */
}WAS_NV_NOISE_OPTIMIZE_CTRL_STRU;
/*****************************************************************************
 �ṹ��    : WAS_NV_CU_CELL_SEARCH_CTRL_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :WAS CU�����Ż�����NV
 DESCRIPTION: WAS CU�����Ż���������
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                 enSearchCampCellFlg;         /* rl fail�Ƿ�����֮ǰפ����Ƶ������ */
    VOS_INT8                           cCuRSSIThreshold;            /* rl fail������ɨ���� */
    PS_BOOL_ENUM_UINT8                 enCuPschSrchFlg;             /* rl fail�����Ƿ���Ҫϸɨ */
    VOS_UINT8                          ucCuSpecCellNum;             /* rl fail��ɨϸɨ����������ָ������������������ϱ���Ƶ������ǿС���ĸ��� */
    VOS_UINT8                          ucCuSpecFreqCellNum;         /* rl fail��ɨϸɨ����������ָ��Ƶ�㣬������ϱ���Ƶ������ǿС���ĸ��� */   
    VOS_UINT8                          ucReserve1;                  /* ����λ */
    VOS_UINT8                          ucReserve2;                  /* ����λ */
    VOS_UINT8                          ucReserve3;                  /* ����λ */    
}WAS_NV_CU_CELL_SEARCH_CTRL_INFO_STRU;


/*****************************************************************************
 �ṹ��    : WAS_NV_CLOUD_FREQ_STRATEGY_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :WAS ��ͨ�ſ���NV,��NV�ṹ����NV�ж�ȡ�󱣴�nv�Ľṹ
 DESCRIPTION: WAS ��ͨ�ſ���NV,��NV�ṹ����NV�ж�ȡ�󱣴�nv�Ľṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                 enCloudEnableFlg;              /* ��ͨ��NV�����Ƿ�� */
    PS_BOOL_ENUM_UINT8                 enWcdmaPrefFreqFlg;            /* ��ͨ�Ź��ܴ�ʱ��WCDMA��Ӫ��Ƶ���Ƿ���ƶ˻�ȡ */
    PS_BOOL_ENUM_UINT8                 enWcdmaPrefBandFlg;            /* ��ͨ�Ź��ܴ�ʱ��WCDMA�Ƿ����ƶ�Ԥ��band������ */
    VOS_UINT8                          ucReserve1;                    /* ����λ */    
}WAS_NV_CLOUD_STRATEGY_INFO_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NV_NETWORK_SEARCH_CUSTOMIZE_CFG_STRU
 �ṹ˵��  : ������������NV
 DESCRIPTION: ������������NV
 �޸���ʷ  :
  1.��  ��   : 2015��10��19��
    ��  ��   : w00134354
    �޸����� : �����ɽṹ

*****************************************************************************/
typedef struct
{
    VOS_UINT32              ulHistoryFreqEnableBitmap;                          /* ��ʷƵ����������ʹ��bitλ */
    VOS_UINT32              ulOperatorSrchEnableBitmap;                         /* ��Ӫ��������������ʹ��bitλ */
    VOS_UINT32              ulFullBandableBitmap;                               /* ȫƵ����������ʹ��bitλ */
    VOS_UINT32              ulCloudFreqFromCssBitmap;                           /* ��Ӫ��Ƶ����ƶ˻�ȡʹ��bitλ */
    VOS_UINT32              ulPreferBandFromCssBitmap;                          /* PreferBand���ƶ˻�ȡʹ��bitλ */    
    VOS_UINT32              ulReserved1;                                        /* ����λ */
    VOS_UINT32              ulReserved2;                                        /* ����λ */
    VOS_UINT32              ulReserved3;                                        /* ����λ */
    VOS_UINT32              ulReserved4;                                        /* ����λ */
    VOS_UINT32              ulReserved5;                                        /* ����λ */
    VOS_UINT32              ulReserved6;                                        /* ����λ */
    VOS_UINT32              ulReserved7;                                        /* ����λ */
    VOS_UINT32              ulReserved8;                                        /* ����λ */
    VOS_UINT32              ulReserved9;                                        /* ����λ */
    VOS_UINT32              ulReserved10;                                       /* ����λ */
}WAS_NV_NETWORK_SEARCH_CUSTOMIZE_CFG_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NV_T314_CFG_STRU
 �ṹ˵��  : ����û������T314ʱʹ��NV����Ĭ�ϲ���
 DESCRIPTION: ����û������T314ʱʹ��NV����Ĭ�ϲ���
 �޸���ʷ  :
  1.��  ��   : 2015��12��29��
    ��  ��   : j00169676
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8      enCfgOnOffFlg;                                      /* �����Ƿ���Ч���� */
    VOS_UINT8               ucT314DefaultValue;                                 /* ��������T314Ϊ0ʱʹ�ø�ֵ */
    VOS_UINT8               ucReserved1;                                        /* ����λ */
    VOS_UINT8               ucReserved2;                                        /* ����λ */
}WAS_NV_T314_CFG_STRU;
/*****************************************************************************
 �ṹ��    : WAS_NVIM_GSM_MEAS_OFFSET_STRU
 �ṹ˵��  : GSM���������������NV
 DESCRIPTION: GSM�������������������
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8              enGsmMeasUseDefaultSCriterionFlg;           /* ��ֹ��GSM��ѡ��GSM���������о�ʱ��S׼��ʹ��Ĭ���������ò���-18��-58 */
    PS_BOOL_ENUM_UINT8              enOutOfServiceUseDefaultSCriterionFlg;      /* ���������о�ʱ�Ƿ�ʹ��Ĭ��S׼���� */
    VOS_UINT8                       ucGsmMeasRscpOffset;                        /* ����ͣ��ϵͳ���������ϼ�ȥ�ĳ��ͣ�RSCP���� */
    VOS_UINT8                       ucReserve1;
    VOS_INT8                        cQRxlevmin;
    VOS_INT8                        cQQualmin;
    VOS_UINT8                       ucReserve2;
    VOS_UINT8                       ucReserve3;
}WAS_NVIM_GSM_MEAS_OFFSET_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_W2L_RESEL_THRES_CFG_NV_STRU
 �ṹ˵��  : W2L��ѡ�Ż��������NV
 DESCRIPTION: W2L��ѡ�Ż������������
*****************************************************************************/
typedef struct
{
    VOS_UINT8                        ucFeatureSwitch;                        /* ���ܿ���*/
    VOS_UINT8                        ucReserve1;                             /* ����λ  */
    VOS_INT16                        sRsrqThres;                             /* L�ķ���С��Rsrq˽�����ޣ���λ1dB */
    VOS_INT16                        sDeltaRsrq;                             /* ��ϵͳ��ѡ��L��RSRQԼ���ĳ������ޣ���λ1dB */
    VOS_INT16                        sUtraRscp;                              /* ���ڴ�ֵʱ��utra��ѡ��L��ԭ���̴�����λ1dB */
    VOS_INT16                        sUtraEcn0;                              /* ���ڴ�ֵʱ��utra��ѡ��L��ԭ���̴�����λ1dB */
    VOS_UINT16                       ucReserve2;                             /* ����λ */
}WAS_NVIM_W2L_RESEL_THRES_CFG_NV_STRU;
/*****************************************************************************
 �ṹ��    : WAS_NV_HISTORY_RSSI_SEARCH_STRU
 �ṹ˵��  : ���������Ż���ʷƵ��ɨƵ�����NV
 DESCRIPTION: ���������Ż���ʷƵ��ɨƵ���������
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8               ucHistoryRssiSearchFlg;                         /* ��ʷƵ��ɨƵ���� */
    PS_BOOL_ENUM_UINT8               ucHistoryPschSearchFlg;                         /* ��ʷƵ��ɨƵ���Ƿ�ϸɨ�Ŀ��� */
    VOS_UINT8                        ucHistoryRssiFreqSearchNum;                     /* ��ʷƵ��rssiɨ��������·�Ƶ�����������Ĭ��Ϊ50�����֧��100 */
    VOS_UINT8                        ucHistoryPschFreqSearchNum;                     /* ��ʷƵ��ϸɨ��������·�Ƶ�����������Ĭ��Ϊ5 */
    VOS_INT8                         cHistoryRssiBgSearchThreshold;                  /* ����������ʷƵ���RSSIɨƵ�����ޣ���λ1db */
    VOS_INT8                         cHistoryRssiPlmnListSearchThreshold;            /* list������ʷƵ���RSSIɨƵ�����ޣ���λ1db */
    VOS_INT8                         cHistoryRssiOtherSearchThreshold;               /* ����������ʷƵ���RSSIɨƵ�����ޣ���λ1db */
    VOS_UINT8                        ucHistoryReportCellSearchFreqSearchNum;               /* ��ʷƵ��С������Ҫ������ϱ���Ƶ����������Ĭ��Ϊ5 */
    VOS_UINT8                        ucAnycellReportCellSearchFreqSearchNum;               /* anycell������С������Ҫ������ϱ���Ƶ�������� */
    VOS_UINT8                        ucHistoryCellSearchFreqMaxNum;                        /* ��ʷƵ��С������Ҫ�����С���ѵ�Ƶ�������� */
    VOS_UINT8                        ucBgSearchCellSearchFreqMaxNum;                    /* BG������С������Ҫ�����С���ѵ�Ƶ��������*/
    VOS_UINT8                        ucAnycellCellSearchFreqMaxNum;                     /* anycell������С������Ҫ�����С���ѵ�Ƶ�������� */
    VOS_UINT8                        ucPlmnlistCellSearchFreqMaxNum;                    /* plmnlist������С������Ҫ�����С���ѵ�Ƶ�������� */
    VOS_INT8                         cHistoryRssiSpecSearchThreshold;                  /* EPLMN ��ʷƵ������ */
    PS_BOOL_ENUM_UINT8               enSearchNotEplmnFreqFlg;                           /* �Ƿ�������EPLMN MCC��� */
    VOS_UINT8                        ucReserve1;                                         /* ����λ */
}WAS_NV_HISTORY_RSSI_SEARCH_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_OUT_SERVICE_RESEL_THRES_STRU
 �ṹ˵��  : S׼�������Ҳ��������������������������������ѡ�����ޣ�
             ��:RSCP����-115��ecn0����-18��������ѡ
 DESCRIPTION: S׼�������Ҳ��������������������������������ѡ�����ޣ�
              ��:RSCP����-115��ecn0����-18��������ѡ
*****************************************************************************/
typedef struct
{
    VOS_INT8                         cW2GRscpThres;                          /* ���ڴ�ֵʱ��������������ѡ����λ1db */
    VOS_INT8                         cW2GEcn0Thres;                          /* ���ڴ�ֵʱ��������������ѡ����λ1dB */
    PS_BOOL_ENUM_UINT8               enOutofServiceReselSwitch;              /* �����Ƿ�� */
    VOS_INT8                         ucReserve1;                             /* ����λ */
    VOS_INT8                         ucReserve2;                             /* ����λ */
    VOS_INT8                         ucReserve3;                             /* ����λ */
    VOS_INT8                         ucReserve4;                             /* ����λ */
    VOS_INT8                         ucReserve5;                             /* ����λ */
}WAS_NVIM_OUT_SERVICE_RESEL_THRES_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_NW_REL_TIMER_INFO_STRU
 �ṹ˵��  : ��������������ʱ����������ʱ������NV
 DESCRIPTION: ��������������ʱ����������ʱ����������
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8               enNoConnectionStartNwRelTimerFlg;       /* û����������ʱ����timer���� */
    VOS_UINT8                        ucNwRelTimerLen;                        /* ��ʱ��ʱ����Ĭ��10s */
    VOS_UINT8                        ucReserve1;                             /* ����λ */
    VOS_UINT8                        ucReserve2;                             /* ����λ */
}WAS_NVIM_NW_REL_TIMER_INFO_STRU;
/*****************************************************************************
 �ṹ��    : WAS_NVIM_ERACH_BROKEN_CELL_STRU
 �ṹ˵��  : ���ERACH Broken Cell�Ĳ�������              ID:3055 en_NV_Item_Was_Erach_Broken_Cell_Info
 DESCRIPTION: ���ERACH Broken Cell�Ĳ�������    
*****************************************************************************/

typedef struct
{
    PS_BOOL_ENUM_UINT8                      enNvErachBackToRachFlg;             /* �Ƿ���Erach���˹��� */
    VOS_UINT8                               ucEraNfail;                         /* ������Nfail��EST_REQ���ط�������С������ERACH Bar�б� */
    VOS_UINT8                               ucRscpOffset;                       /* ��С������������ô��dbʱ��С����ERACH bar�б�ų������Գ���ERACH���� */
    VOS_UINT8                               ucReserved;                         /* ����λ */
    VOS_UINT16                              usEraMaxBarlen;                     /* ERACH barС�������ʱ������λΪ���ӣ�������ʱ��С���Զ���bar */
    VOS_UINT16                              usEraTBarInitialLen;                /* С������ERACH bar�б�ĳ�ʼʱ�䣬��λ:�� */
    VOS_INT16                               sRscpHighThresh;                    /* ����ERACH bar�б��е�С���������ڴ����ޣ���С���ų�ERACH bar�б�Ͳ����������� */
    VOS_INT16                               sEcn0Thresh;                        /* С����������һ����������ucRscpOffset���Һ���EcNo���޲ſ��ܽ�Bar */
}WAS_NVIM_ERACH_BROKEN_CELL_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_ERACH_SYS_CHG_IND_CTRL_STRU
 �ṹ˵��  : ���ERACH ϵͳ��Ϣ���CR�Ĳ�������              ID:3056 en_NV_Item_Was_Erach_Sys_ChgInd_Ctrl
 DESCRIPTION: ���ERACH ϵͳ��Ϣ���CR�Ĳ������� 
*****************************************************************************/

typedef struct
{
    PS_BOOL_ENUM_UINT8                      enSysChgIndCtrlFlg;                 /*  �Ƿ���ERACH CR�����Ĺ��� */
    PS_BOOL_ENUM_UINT8                      enPeriodCellUpdateInPchFlg;         /* ERACH�£�pch�²���CU���߲�������Ŀ��أ�0Ϊ�߲������棬1Ϊ��CU���� */
    VOS_UINT8                               ucReserved2;                       /* ����λ */
    VOS_UINT8                               ucReserved3;                       /* ����λ */
}WAS_NVIM_ERACH_SYS_CHG_IND_CTRL_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_ERRLOG_ACTIVE_RPT_TIME_INTERVAL_CTRL_STRU
 �ṹ˵��  : ���ڿ���CHR�����ϱ���ʱ������������
 DESCRIPTION: ���ڿ���CHR�����ϱ���ʱ������������
1.��  ��   : 2017��1��10��
  ��  ��   : w00402148
  �޸����� : �����������ʧ���ϱ�ʱ������NV����
*****************************************************************************/
typedef struct
{
    VOS_UINT8                        ucRptTimeIntervalForOtaMsgCheckFail;       /* ���ڿտڼ��ʧ���¼����ϱ��������λΪmin */
    VOS_UINT8                        ucRptRaFailMinTimeInterval;                /* ���ڿ����������ʧ�ܵ���С�ϱ��������λΪmin */
    VOS_UINT8                        ucRptRrcAccessLongMinTimeInterval;         /* ���ڿ����������ʱ���������С�ϱ��������λΪmin  */
    VOS_UINT8                        ucRptRadioResCheckMinTimeInterval;         /* ���ڿ�����Դ�˲��¼�����С�ϱ��������λΪmin  */
    VOS_UINT8                        ucRptStateLastTimeMinTimeInterval;         /* ���ڿ��Ƹ�Э��̬ռ�ȵ���С�ϱ��������λΪhour  */
    VOS_UINT8                        ucRptStateLastTimForPoweroff;              /* ���ڿ��Ƹ�Э��̬ռ�ȵ���С�ϱ��������λΪhour(�ػ�����)  */
    VOS_UINT8                        ucRptW2WReselSlowMinTimeInterval;          /* ���ڿ���W2W��ѡ������С�ϱ��������λΪhour  */
    VOS_UINT8                        ucRptWoosSectionChrRptInterval;            /* ���ڿ���WOOS�׶��ϱ�ʱ��������λmin  */
    VOS_UINT8                        ucReserve8;                                /* ����λ  */
    VOS_UINT8                        ucReserve9;                                /* ����λ  */
    VOS_UINT8                        ucReserve10;                               /* ����λ  */
    VOS_UINT8                        ucReserve11;                               /* ����λ  */
}WAS_NVIM_ERRLOG_ACTIVE_RPT_TIME_INTERVAL_CTRL_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_ERRLOG_THRESHOLD_CONFIG_STRU
 �ṹ˵��  : ���ڿ���CHR���޲�����NV����
 DESCRIPTION: ���ڿ���CHR���޲�����NV����
1.��  ��   : 2017��1��10��
  ��  ��   : w00402148
  �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    VOS_INT16                        sRptRaFailRscpThreshold;                   /* �������ʧ��RSCP�ϱ����ޣ����������޲��ϱ�  */
    VOS_INT16                        sRptRaFailEcNoThreshold;                   /* �������ʧ��Ec/No�ϱ����ޣ����������޲��ϱ�  */

    VOS_UINT8                        ucRptRrcAccessLongTimeThreshold;           /* �������ʱ��������ж����ޣ���λsec  */
    VOS_UINT8                        ucRptW2WReselSlowTimeThreshold;            /* W2W��ѡʱ���ж����ޣ���λsec  */
    VOS_UINT8                        ucReserve3;                                /* ����λ  */
    VOS_UINT8                        ucReserve4;                                /* ����λ  */

    VOS_INT16                        sReserve1;                                /* ����λ  */
    VOS_INT16                        sReserve2;                                /* ����λ  */
    VOS_INT16                        sReserve3;                                /* ����λ  */
    VOS_INT16                        sReserve4;                                /* ����λ  */
    VOS_INT16                        sReserve5;                                /* ����λ  */
    VOS_INT16                        sReserve6;                                /* ����λ  */
    VOS_INT16                        sReserve7;                                /* ����λ  */
    VOS_INT16                        sReserve8;                                /* ����λ  */
    VOS_INT16                        sReserve9;                                /* ����λ  */
    VOS_INT16                        sReserve10;                               /* ����λ  */
}WAS_NVIM_ERRLOG_THRESHOLD_CONFIG_STRU;

/* Added by h00377722 for ����CHR�ϱ�, 2017-03-8, begin */
/*****************************************************************************
 �ṹ��    : WAS_NVIM_ERRLOG_THRESHOLD_CONFIG_STRU
 �ṹ˵��  : ���ڿ���ֱͨCHR�Ŀ���
 DESCRIPTION: ���ڿ���ֱͨCHR�Ŀ���
1.��  ��   : 2017��3��8��
  ��  ��   : h00377722
  �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enRaLongFlg;                        /* �������ʱ����������ϱ�NV���� */
    PS_BOOL_ENUM_UINT8                      enResCheckFlg;                      /* ��Դ�˲������ϱ�NV���� */
    PS_BOOL_ENUM_UINT8                      enStateLastTimeFlg;                 /* Э��̬����ʱ�������ϱ�NV���� */
    PS_BOOL_ENUM_UINT8                      enW2WReselSlowFlg;                  /* W2W��ѡ���¼������ϱ�NV���� */
    PS_BOOL_ENUM_UINT8                      enWoosSectionChrFlag;               /* WOOS�׶��¼������ϱ�NV���� */
    VOS_UINT8                               ucReserve1;                         /* ����λ  */
    VOS_UINT8                               ucReserve2;                         /* ����λ  */
    VOS_UINT8                               ucReserve3;                         /* ����λ  */
    VOS_UINT8                               ucReserve4;                         /* ����λ  */
    VOS_UINT8                               ucReserve5;                         /* ����λ  */
    VOS_UINT8                               ucReserve6;                         /* ����λ  */
    VOS_UINT8                               ucReserve7;                         /* ����λ  */
    VOS_UINT8                               ucReserve8;                         /* ����λ  */
    VOS_UINT8                               ucReserve9;                         /* ����λ  */
    VOS_UINT8                               ucReserve10;                        /* ����λ  */
    VOS_UINT8                               ucReserve11;                        /* ����λ  */
    VOS_UINT8                               ucReserve12;                        /* ����λ  */
    VOS_UINT8                               ucReserve13;                        /* ����λ  */
    VOS_UINT8                               ucReserve14;                        /* ����λ  */
    VOS_UINT8                               ucReserve15;                        /* ����λ  */
}WAS_NVIM_DIRECT_CHR_CTRL_STRU;
/* Added by h00377722 for ����CHR�ϱ�, 2017-03-8, end */

/*****************************************************************************
 �� �� ��  : WAS_NV_CAMP_HANDLE_CUSTOMIZE_CFG_STRU
 �ṹ˵��  : ����פ���о�����NV
 DESCRIPTION: ����פ���о���������
 �޸���ʷ  :
  1.��  ��   : 2015��4��6��
    ��  ��   : z00184470
    �޸����� : �����ɽṹ

*****************************************************************************/
typedef struct
{
    VOS_UINT32              ulCampHandleCustomEnableBitmap;                     /* פ���о�����ʹ��bitλ��ÿһλ���ض��������������Ӧ�������Ӧ��ϵ�ɲ�����������ö�ٽṹWAS_NETWORK_SEARCH_PROGRESS_ENUM */
}WAS_NV_CAMP_HANDLE_CUSTOMIZE_CFG_STRU;

/*****************************************************************************
 �ṹ��    : WAS_NVIM_SVALUE_CUSTOMIZE_CTRL_STRU
 �ṹ˵��  : �ض���CSFB��������CSFB����S׼��������              ID:3062 en_NV_Item_WAS_SValue_CUSTOMIZE_CFG
 DESCRIPTION: �ض���CSFB��������CSFB����S׼��������
*****************************************************************************/

typedef struct
{
    PS_BOOL_ENUM_UINT8                      enSValueCtrlFlg;                   /*  �Ƿ�s׼���ƹ��� */
    VOS_INT8                                cRscp_Threshold;                   /* WCDMA Cpich_Rscp����,Ĭ��ֵ:-105 ,��Χ0~-127*/
    VOS_INT8                                cEcNo_Threshold;                   /* WCDMA Cpich_EcNo����,Ĭ��ֵ:-18 */
    VOS_UINT8                               ucReserved1;                       /* ����λ */
    VOS_UINT8                               ucReserved2;                       /* ����λ */
    VOS_UINT8                               ucReserved3;                       /* ����λ */
    VOS_UINT8                               ucReserved4;                       /* ����λ */
    VOS_UINT8                               ucReserved5;                       /* ����λ */

}WAS_NVIM_SVALUE_CUSTOMIZE_CTRL_STRU;
/*****************************************************************************
 �ṹ��    : WAS_NVIM_MFBI_CTRL_STRU
 �ṹ˵��  : ���ڿ����Ƿ�֧��MFBI
 DESCRIPTION: �����Ƿ�֧��MFBI�Ŀ��ƿ���
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8               enLteMfbiSptFlg;                           /* �Ƿ�֧��MFBI */
    PS_BOOL_ENUM_UINT8               enNeedAttActiveFlg;                        /* �Ƿ���Ҫ�о�ATT����  */
    VOS_UINT8                        ucReserve1;
    VOS_UINT8                        ucReserve2;
}WAS_NVIM_MFBI_CTRL_STRU;

/* Added by h00377722 for �����Ż�6.0, 2016-9-7, begin */
/*****************************************************************************
 �� �� ��  : WAS_TAFITF_NvimReadGeoSearchForSib1
 �ṹ˵��  : ��ȡGeo�����Ƿ���Ҫ��SIB1�Ŀ��ƿ���
 DESCRIPTION: ��ȡGeo�����Ƿ���Ҫ��SIB1�Ŀ��ƿ���
 �޸���ʷ  :
  1.��  ��   : 2016��9��7��
    ��  ��   : h00377722
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8      enSib1OnOffFlg;                                     /* �����Ƿ���Ч���� */
    VOS_UINT8               ucReserved1;                                        /* ����λ */
    VOS_UINT8               ucReserved2;                                        /* ����λ */
    VOS_UINT8               ucReserved3;                                        /* ����λ */
}WAS_NV_GEO_SEARCH_FOR_SIB1_STRU;

/* Added by h00377722 for �����Ż�6.0, 2016-9-7, end */

/* Added by z00184470 for �����Ż�6.0, 2016-09-07, begin */
/*****************************************************************************
 �� �� ��  : WAS_NV_MULTI_MODEM_HISTORY_FERQ_SHARE_CFG_STRU
 �ṹ˵��  : ��MODEM��ʷƵ�㹲����NV
 DESCRIPTION: ��MODEM��ʷƵ�㹲����NV
 �޸���ʷ  :
  1.��  ��   : 2016��9��7��
    ��  ��   : z00184470
    �޸����� : �����ɽṹ

*****************************************************************************/
typedef struct
{
    VOS_UINT32              ulMultiModemHistoryFreqShareCfgEnableBitmap;        /* ��MODEM��ʷƵ�㹲������ʹ��bitλ */    
}WAS_NV_MULTI_MODEM_HISTORY_FERQ_SHARE_CFG_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NV_NET_SRCH_RMV_INTER_RAT_FREQ_BAND_CTRL_STRU
 �ṹ˵��  : ��ģ����ɾ��ϵͳƵ�㶨��NV����
 DESCRIPTION: ��ģ����ɾ��ϵͳƵ�㶨��NV����
 �޸���ʷ  :
  1.��  ��   : 2016��9��7��
    ��  ��   : z00184470
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8      enNetSrchRmvInterRatFreqBandEnableFlg;              /* ��������ɾ����ϵͳƵ��ʹ�ܿ��� */
    VOS_UINT8               ucReserved1;                                        /* ����λ */
    VOS_UINT8               ucReserved2;                                        /* ����λ */
    VOS_UINT8               ucReserved3;                                        /* ����λ */
}WAS_NV_NET_SRCH_RMV_INTER_RAT_FREQ_BAND_CTRL_STRU;

/* Added by z00184470 for �����Ż�6.0, 2016-09-07, end */
/*****************************************************************************
 �ṹ��    : WAS_NVIM_W2L_RESEL_PUNISH_CUSTOMIZE_CTRL_STRU
 �ṹ˵��  : W2L��ѡ����nasʧ�ܣ�W2L��ѡ�ͷ��Ŀ��ƿ���              ID:3069 en_NV_Item_WAS_W2L_Resel_Punish_CUSTOMIZE_CFG
 DESCRIPTION: W2L��ѡ����nasʧ�ܣ�W2L��ѡ�ͷ��Ŀ��ƿ���
*****************************************************************************/

typedef struct
{
    PS_BOOL_ENUM_UINT8                      enW2LReselPunishCtrlFlg;           /*  W2L��ѡ�ͷ��Ŀ��ƿ���  */
    VOS_UINT8                               ucW2LReselPunishTimerLen;          /* W2L��ѡ�ͷ��Ķ�ʱ���ĳ��ȣ���λ:s */
    VOS_UINT8                               ucReserved1;                       /* ����λ */
    VOS_UINT8                               ucReserved2;                       /* ����λ */  

}WAS_NVIM_W2L_RESEL_PUNISH_CUSTOMIZE_CTRL_STRU;
/*****************************************************************************
 �� �� ��  : WAS_NV_RE_CALC_ACTIVE_TIME_CTRL_STRU
 �ṹ˵��  : ���¼��㼤��ʱ����ƿ���
 DESCRIPTION: ���¼��㼤��ʱ����ƿ���
 �޸���ʷ  :
  1.��  ��   : 2016��10��12��
    ��  ��   : j00169676
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8      enRecalActiveTimeFlg;                               /* ���ƴ����¼��㼤��ʱ��ʹ�ܿ��� */
    VOS_UINT8               ucReserved1;                                        /* ����λ */
    VOS_UINT8               ucReserved2;                                        /* ����λ */
    VOS_UINT8               ucReserved3;                                        /* ����λ */
}WAS_NV_RE_CALC_ACTIVE_TIME_CTRL_STRU;
/*****************************************************************************
 �� �� ��  : WAS_NV_PRINT_LEVEL_CTRL_STRU
 �ṹ˵��  : Normal��Info�����ӡ��ʹ�ܿ���
 DESCRIPTION: WAS Normal��Info�����ӡ��ʹ�ܿ���
 �޸���ʷ  :
  1.��  ��   : 2016��10��25��
    ��  ��   : j00169676
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8      enPrintInfoFlg;                                     /* ���ƴ�Normal��Info�����ӡ��ʹ�ܿ��� */
    VOS_UINT8               ucReserved1;                                        /* ����λ */
    VOS_UINT8               ucReserved2;                                        /* ����λ */
    VOS_UINT8               ucReserved3;                                        /* ����λ */
}WAS_NV_PRINT_LEVEL_CTRL_STRU;

/*****************************************************************************
 �ṹ˵��  : ��ѡ��ʱ�����Ʋ���
 DESCRIPTION: ��ѡ��ʱ�����Ʋ���
 �޸���ʷ  :
  1.��  ��   : 2016��11��21��
    ��  ��   : z00184470
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8              enTreseltionCustCtrlFlg;                    /* ��ѡ��ʱ������ʹ���ܿ���  */
    PS_BOOL_ENUM_UINT8              enSValueNotFulfilCustValidFlg;              /* ��������S׼��ʱ�������Ƿ���Ч�ı�� */
    VOS_UINT16                      usEcn0Delta;                                /* ֻ�е�Ŀ��С����EC/N0��ȥ����С����EC/N0���ڻ���ڸ����޴�Сʱ�����Ʋ���Ч */
    VOS_UINT16                      usIntraFreqTreselectionCustLenInIdlePch;    /* IDLE��PCH̬ͬƵ��ѡ��ʱ���ɶ���ʱ������λΪ���ø�������DRX���ڳ��� */
    VOS_UINT16                      usIntraFreqTreselectionLowCustLenInIdlePch; /* IDLE��PCH̬ͬƵ��ѡ��ʱ���ɶ������ʱ������λΪms */
    VOS_UINT16                      usIntraFreqTreselectionHighCustLenInIdlePch;/* IDLE��PCH̬ͬƵ��ѡ��ʱ���ɶ������ʱ������λΪms */
    VOS_UINT16                      usInterFreqTreselectionCustLenInIdlePch;    /* IDLE��PCH̬��Ƶ��ѡ��ʱ���ɶ���ʱ������λΪ���ø�������DRX���ڳ��� */
    VOS_UINT16                      usInterFreqTreselectionLowCustLenInIdlePch; /* IDLE��PCH̬��Ƶ��ѡ��ʱ���ɶ������ʱ������λΪms */
    VOS_UINT16                      usInterFreqTreselectionHighCustLenInIdlePch;/* IDLE��PCH̬��Ƶ��ѡ��ʱ���ɶ������ʱ������λΪms */    
    VOS_UINT16                      usReserve1;                                 /* ����λ */
    VOS_UINT16                      usReserve2;                                 /* ����λ */
    VOS_UINT16                      usReserve3;                                 /* ����λ */
    VOS_UINT16                      usReserve4;                                 /* ����λ */
    VOS_UINT16                      usReserve5;                                 /* ����λ */
    VOS_UINT16                      usReserve6;                                 /* ����λ */
}WAS_NVIM_TRESELECTION_CUSTOMIZE_CFG_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NVIM_HO2EUTRAN_CFG_STRU
 �ṹ˵��  : �洢��NVIM�е�W�л���LTE���ܿ��ƿ���
 DESCRIPTION: �洢��NVIM�е�W�л���LTE���ܿ��ƿ���
 �޸���ʷ  :
  1.��    ��   : 2016��11��11��
    ��    ��   : h00390293
    �޸�ԭ��   : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enHo2EutranUnSupportFlg;            /* �Ƿ�֧�ֵ�L��HO */
    VOS_UINT8                               ucReserve1;
    VOS_UINT8                               ucReserve2;
    VOS_UINT8                               ucReserve3;
}WAS_NVIM_HO2EUTRAN_CFG_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NVIM_EUTRANMEAS_CFG_STRU
 �ṹ˵��  : �洢��NVIM�е�W����̬LTE�������ܿ��ƿ���
 DESCRIPTION: �洢��NVIM�е�W����̬LTE�������ܿ��ƿ���
 �޸���ʷ  :
  1.��    ��   : 2016��11��11��
    ��    ��   : h00390293
    �޸�ԭ��   : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enEutranMeasUnSupportFlg;           /* �Ƿ�֧�ֵ�����̬L�Ĳ��� */
    VOS_UINT8                               ucReserve1;
    VOS_UINT8                               ucReserve2;
    VOS_UINT8                               ucReserve3;
}WAS_NVIM_EUTRANMEAS_CFG_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NVIM_FDPCH_EFDPCH_CFG_STRU
 �ṹ˵��  : �洢��NVIM�е�FDPCH/EFDPCHʹ�ܿ��ƿ���
 DESCRIPTION: �洢��NVIM�е�FDPCH/EFDPCHʹ�ܿ��ƿ���
 �޸���ʷ  :
  1.��    ��   : 2016��11��11��
    ��    ��   : h00390293
    �޸�ԭ��   : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enFDPCHSupport;                     /* �Ƿ�֧��FDPCH�ı�־                          */
    PS_BOOL_ENUM_UINT8                      enEFDPCHSupport;                    /* �Ƿ�֧��E-FDPCH�ı�־,FDPCH֧��ʱ��NV����Ч */
    VOS_UINT8                               ucReserve1;
    VOS_UINT8                               ucReserve2;
}WAS_NVIM_EFDPCH_FDPCH_CFG_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NVIM_CPC_CFG_STRU
 �ṹ˵��  : �洢��NVIM�е�CPC�������
 DESCRIPTION: �洢��NVIM�е�CPC�������
 �޸���ʷ  :
  1.��    ��   : 2016��11��11��
    ��    ��   : h00390293
    �޸�ԭ��   : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enSf4Support;                       /* �Ƿ�֧��ul dpcch ʹ�� slotFormat4 */
    PS_BOOL_ENUM_UINT8                      enHsscchLessSupport;                /* �Ƿ�֧�� hsscchlessHsdschOperation           */
    PS_BOOL_ENUM_UINT8                      enUlDpcchDtxSupport;                /* �Ƿ�֧�� discontinuousDpcchTransmission      */
    VOS_UINT8                               ucReserve1;
}WAS_NVIM_CPC_CFG_STRU;
  
/*****************************************************************************
 �� �� ��  : WAS_NVIM_EFACH_CFG_STRU
 �ṹ˵��  : �洢��NVIM�е�EFACH������� 
 DESCRIPTION: �洢��NVIM�е�EFACH�������  
 �޸���ʷ  :
  1.��    ��   : 2016��11��11��
    ��    ��   : h00390293
    �޸�ԭ��   : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enSuppHsdpaInFach;                  /* ֧��CELL_FACH��HS-DSCH�Ľ��� */
    PS_BOOL_ENUM_UINT8                      enSuppHsdpaInPch;                   /* ֧��CELL_PCH��URA_PCH��HS-DSCH�Ľ��� */
    VOS_UINT8                               ucReserve1;
    VOS_UINT8                               ucReserve2;    
}WAS_NVIM_EFACH_CFG_STRU;

/*****************************************************************************
 �� �� ��  : NVIM_GUAS_ACBAR_CFG_INFO_STRU
 �ṹ˵��  : �洢��NVIM�еĸ����뼼����AC BAR������� 
 DESCRIPTION: �洢��NVIM�еĸ����뼼����AC BAR������� 
 �޸���ʷ  :
  1.��    ��   : 2016��11��11��
    ��    ��   : h00390293
    �޸�ԭ��   : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enSuppAcAllBarFlg;                  /* ֧��AC ALL BAR�����⴦�� */
    VOS_UINT8                               ucReserve1;    
    VOS_UINT8                               ucReserve2;  
    VOS_UINT8                               ucReserve3;  
    VOS_UINT16                              usAcAllBartime;                     /* AC ALL BAR�ͷ���ʱ��ʱ��,��λ:�� */    
    VOS_UINT16                              usReserve1;                         /* ����λ */
    VOS_UINT16                              usReserve2;                         /* ����λ */
    VOS_UINT16                              usReserve3;                         /* ����λ */     
}NVIM_GUAS_AC_BAR_CFG_INFO_STRU;

/*****************************************************************************
 �� �� ��  : NVIM_GUAS_AC_BAR_CFG_STRU
 �ṹ˵��  : �洢��NVIM�е�AC BAR������� 
 DESCRIPTION: �洢��NVIM�е�AC BAR������� 
 �޸���ʷ  :
  1.��    ��   : 2016��11��11��
    ��    ��   : h00390293
    �޸�ԭ��   : �����ɽṹ
*****************************************************************************/
typedef struct
{
    NVIM_GUAS_AC_BAR_CFG_INFO_STRU          stGsmAcBarCfg;                       /* GSM��AC BAR������Ϣ */
    NVIM_GUAS_AC_BAR_CFG_INFO_STRU          stWcdmaAcBarCfg;                     /* WCDMA��AC BAR������Ϣ */
}NVIM_GUAS_AC_BAR_CFG_STRU;

/* Added by h00377722 for ϵͳ��Ϣ�Ż�, 2016-11-15, begin */
/*****************************************************************************
 ö �� ��  : WAS_SIB_POS_NOTIFY_ENUM_UINT8
 ö��˵��  : ֧�־�ȷ������̳���
 DESCRIPTION: ֧�־�ȷ������̳���
 �޸���ʷ  :
  1.��  ��   : 2016��10��10��
    ��  ��   : w00380530
    �޸����� : �����ɽṹ

*****************************************************************************/
enum WAS_SIB_POS_NOTIFY_ENUM
{
    
    WAS_SIB_POS_NOTIFY_RESEL                = 0x00000001,   /* bit0: ��ѡ */
    WAS_SIB_POS_NOTIFY_CELL_SELECT          = 0x00000002,   /* bit1: ����ѡ֮�������פ������ */
    WAS_SIB_POS_NOTIFY_SIB_UPDATE           = 0x00000004,   /* bit2: ϵͳ��Ϣ���¡�6Сʱ���� */
    WAS_SIB_POS_NOTIFY_BG_SEARCH            = 0x00000008,   /* bit3: ����ģ�µı����ѡ�inter list�ѡ�inter list pre band�� */
    WAS_SIB_POS_NOTIFY_ANR                  = 0x00000010,   /* bit4: anr */
    WAS_SIB_POS_NOTIFY_RCV_SIB7             = 0x00000020,   /* bit5: �����Խ�SIB7 */
    WAS_SIB_POS_NOTIFY_DCH_SYSINFO          = 0x00000040,   /* bit6: D�½�ϵͳ��Ϣ */
    WAS_SIB_POS_NOTIFY_GEO                  = 0x00000080,   /* bit7: GEO���� */
    WAS_SIB_POS_NOTIFY_USER_LIST            = 0x00000100,   /* bit8: user list���� */  
    
    WAS_SIB_POS_NOTIFY_BUTT
};
typedef VOS_UINT32 WAS_SIB_POS_NOTIFY_ENUM_UINT32;

/*****************************************************************************
 �� �� ��  : WAS_NV_FAKE_DMCR_CFG_STRU
 �ṹ˵��  : CSFBʱ���粻֧��DMCRʱ����Ҫ��SIB11\SIB19�ȶ���NV
 DESCRIPTION: CSFBʱ���粻֧��DMCRʱ����Ҫ��SIB11\SIB19�ȶ���NV
 �޸���ʷ  :
  1.��  ��   : 2016��12��20��
    ��  ��   : j00169676
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8              enFakeDmcrSwitch;
    VOS_UINT8                       ucMccNum;
    VOS_UINT8                       ucRsv1;
    VOS_UINT8                       ucRsv2;
    VOS_UINT32                      aulNotSupportFakeDmcrMccList[WAS_NV_MAX_NOT_SUPPORT_FAKE_DMCR_MCC_NUM];
}WAS_NV_FAKE_DMCR_CFG_STRU;

/*****************************************************************************
 �ṹ��     : WAS_NVIM_SYS_INFO_CHG_FAIL_OPTIMIZE_CFG_STRU
 DESCRIPTION: ϵͳ��Ϣ���ʧ���Ż�����NV
 �޸���ʷ   :
 1.��    ��    : 2016��12��13��
   ��    ��    : z00184470
   �޸�����    : �½��ṹ��
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                              enSysInfoChgFailOptmizeFlg; /* ϵͳ��Ϣ���ʧ���Ż�ʹ���ܿ��� */
    PS_BOOL_ENUM_UINT8                              enCsMtCallCustFlg;          /* ϵͳ��Ϣ���ʧ���Ż�ʹ���ܿ��ش򿪵�����£�ָʾ�Ƿ���Ҫ�Ա���ҵ�����ⶨ�Ƶı�� */
    VOS_UINT8                                       ucReserve1;                 /* ����λ */
    VOS_UINT8                                       ucReserve2;                 /* ����λ */
}WAS_NVIM_SYS_INFO_CHG_FAIL_OPTIMIZE_CFG_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NV_SIB_PRECISE_RCV_CFG_STRU
 �ṹ˵��  : ��ȷ��㲥���ö���NV
 DESCRIPTION: ��ȷ��㲥���ö���NV
 �޸���ʷ  :
  1.��  ��   : 2016��11��15��
    ��  ��   : h00377722
    �޸����� : �����ɽṹ

*****************************************************************************/
typedef struct
{
    VOS_UINT32              ulSibPreciseRcvCfgEnableBitmap;                     /* ��ȷ��㲥����ʹ��bitλ */
    VOS_UINT16              usSibPreciseRcvCfgSbRepMin;                         /* �յ�MIB�ж�Sb1��Sb2�����Ƿ�С�ڸ�ֵ��С�������յ�SB��Ϣʱ�ŷ��;�ȷ�� */
    VOS_UINT8               ucReserved1;                                        /* ����λ */
    VOS_UINT8               ucReserved2;                                        /* ����λ */
}WAS_NV_SIB_PRECISE_RCV_CFG_STRU;
/* Added by h00377722 for ϵͳ��Ϣ�Ż�, 2016-11-15, end */

/*****************************************************************************
 �� �� ��  :  WAS_NVIM_INITIAL_SIR_TARGET_CTRL_STRU
 �ṹ˵��  :  ��������InitailSirTarget��Detaֵ
 DESCRIPTION: ��������InitailSirTarget��Detaֵ
 �޸���ʷ  :
  1.��  ��   : 2016��2��3��
    ��  ��   : j00169676
    �޸����� : �����ɽṹ
*****************************************************************************/

typedef struct
{
    VOS_UINT8                        ucInitailSirTargetDeta1;                   /* ucPwrOffsetPilotPdpdchΪ[0-6)ʱ��Detaֵ��Ĭ��Ϊ0 */
    VOS_UINT8                        ucInitailSirTargetDeta2;                   /* ucPwrOffsetPilotPdpdchΪ[6-12)ʱ��Detaֵ��Ĭ��Ϊ15 */
    VOS_UINT8                        ucInitailSirTargetDeta3;                   /* ucPwrOffsetPilotPdpdchΪ[12-18)ʱ��Detaֵ��Ĭ��Ϊ30 */
    VOS_UINT8                        ucInitailSirTargetDeta4;                   /* ucPwrOffsetPilotPdpdchΪ[18-24]ʱ��Detaֵ��Ĭ��Ϊ45 */
}WAS_NVIM_INITIAL_SIR_TARGET_CTRL_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NVIM_INTERRAT_RESEL_OPTIMIZE_CTRL_STRU
 �ṹ˵��  : ��ϵͳ��ѡ����Ż����ƿ���
 DESCRIPTION: ��ϵͳ��ѡ����Ż����ƿ���
 �޸���ʷ  :
  1.��  ��   : 2016��2��3��
    ��  ��   : j00169676
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8               enNotAllowedInterRatReselCallProc;         /* ���з���������Ƿ�������ϵͳ��ѡʹ�ܿ��� */
    VOS_UINT8                        ucRsv1;
    VOS_UINT8                        ucRsv2;
    VOS_UINT8                        ucRsv3;
    VOS_UINT8                        ucRsv4;
    VOS_UINT8                        ucRsv5;
    VOS_UINT8                        ucRsv6;
    VOS_UINT8                        ucRsv7;
    VOS_UINT8                        ucRsv8;
    VOS_UINT8                        ucRsv9;
    VOS_UINT8                        ucRsv10;
    VOS_UINT8                        ucRsv11;
    VOS_UINT8                        ucRsv12;
    VOS_UINT8                        ucRsv13;
    VOS_UINT8                        ucRsv14;
    VOS_UINT8                        ucRsv15;
}WAS_NVIM_INTERRAT_RESEL_OPTIMIZE_CTRL_STRU;
/*****************************************************************************
 �� �� ��  : NVIM_GUAS_DSDS_NEED_APPLY_SEARCH_CTRL_STRU
 �ṹ˵��  : DSDS1.0 was��gas�Ƿ�Ҫ����������Դ
 DESCRIPTION: was��gas�Ƿ�Ҫ����������Դ����NV
 �޸���ʷ  :
  1.��    ��   : 2017��01��05��
    ��    ��   : p00179010
    �޸�ԭ��   : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                      enGasDsdsUnNeedApplySearchFlg;                  /* chicago C20�汾gas��ϵͳ�����Ƿ���Ҫ����������Դ�ı�� */
    PS_BOOL_ENUM_UINT8                      enWasDsdsUnNeedApplySearchFlg;                  /* chicago C20�汾wasͬϵͳ����ϵͳ�����Ƿ���Ҫ����������Դ�ı�� */
    VOS_UINT8                               ucReserve1;
    VOS_UINT8                               ucReserve2;
    VOS_UINT8                               ucReserve3;
    VOS_UINT8                               ucReserve4;
    VOS_UINT8                               ucReserve5;
    VOS_UINT8                               ucReserve6;
    VOS_UINT16                              usReserve7;
    VOS_UINT16                              usReserve8;
    VOS_UINT32                              ulReserve9;
    VOS_UINT32                              ulReserve10;
}NVIM_GUAS_DSDS_NEED_APPLY_SEARCH_CTRL_STRU;
/* Added by p00179010 for DSDS�Ż�&&OOS�����Ż�, 2017-1-13, begin */
/*****************************************************************************
 �� �� ��  : NVIM_WAS_OOS_CELL_SEARCH_OPTIMIZE_CFG_STRU
 �ṹ˵��  : was��������������Ҫ��������ز���
 DESCRIPTION: was��������������Ҫ��������ز���NV
 �޸���ʷ  :
  1.��    ��   : 2017��01��05��
    ��    ��   : p00179010
    �޸�ԭ��   : �����ɽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                               ucOosHistorySearchCount;        /* WAS����̬�³�����������ʷƵ������� */
    PS_BOOL_ENUM_UINT8                      enOosHistorySearchFlg;          /* WAS����̬�³�����������ʷƵ��Ŀ��أ���ϵͳ��ȫ��������ؿ��ƣ���ϵͳ��Ҫ��nvΪ3037�ĳ�����������󽻼� */        
    PS_BOOL_ENUM_UINT8                      enOosPrefbandSearchFlg;         /* WAS����̬�³���������prefband�Ŀ��أ���ϵͳ��ȫ��������ؿ��ƣ���ϵͳ��Ҫ��nvΪ3037�ĳ�����������󽻼� */    
    PS_BOOL_ENUM_UINT8                      enOosAllbandSearchFlg;          /* WAS����̬�³���������allband�Ŀ��أ���ϵͳ��ȫ��������ؿ��ƣ���ϵͳ��Ҫ��nvΪ3037�ĳ�����������󽻼� */  
    VOS_UINT8                               ucReserve1;
    VOS_UINT8                               ucReserve2;
    VOS_UINT8                               ucReserve3;
    VOS_UINT8                               ucReserve4;
    VOS_UINT32                              ulReserve5;
    VOS_UINT32                              ulReserve6;
}NVIM_WAS_OOS_CELL_SEARCH_OPTIMIZE_CFG_STRU;

/* Added by p00179010 for DSDS�Ż�&&OOS�����Ż�, 2017-1-13, end */
/*****************************************************************************
 �� �� ��  : WAS_NVIM_EDRX_CFG_STRU
 �ṹ˵��  : EDRX���ƿ���
 DESCRIPTION :EDRX NV���ƿ���
 �޸���ʷ  :
  1.��  ��   : 2016��11��22��
    ��  ��   : z00370395
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8      enEdrxSupport;                                      /* ����Edrx��ʹ�ܿ��� */
    VOS_UINT8               ucReserved1;                                        /* ����λ */
    VOS_UINT8               ucReserved2;                                        /* ����λ */
    VOS_UINT8               ucReserved3;                                        /* ����λ */
}WAS_NVIM_EDRX_CFG_STRU;

/*****************************************************************************
 �� �� ��  : NVIM_GUAS_CSFB_REDIRECT_OPTIMIZE_CFG_STRU
 �ṹ˵��  : GUAS CSFB���ض����Ż�����
 DESCRIPTION: GUAS CSFB���ض����Ż�����
 �޸���ʷ  :
  1.��    ��   : 2017��03��13��
    ��    ��   : z00370395
    �޸�ԭ��   : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8                  enGAlwaysRedirToLRcvRelWithEarfcn;      /* GAS�յ�����Release��Ϣ�д�EARFCN,�Ƿ��������ض��� */
    PS_BOOL_ENUM_UINT8                  enWAlwaysRedirToLRcvRelWithEarfcn;      /* WAS�յ�����Release��Ϣ�д�EARFCN,�Ƿ��������ض��� */    
    VOS_UINT8                           ucReserve1;         
    VOS_UINT8                           ucReserve2;         
    VOS_UINT8                           ucReserve3;
    VOS_UINT8                           ucReserve4;
    VOS_UINT8                           ucReserve5;
    VOS_UINT8                           ucReserve6;
}NVIM_GUAS_CSFB_REDIRECT_OPTIMIZE_CFG_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NVIM_MEAS_EVT_EVAL_CUSTOMIZE_CFG_STRU
 �ṹ˵��  : �����¼��������Ʋ���
 DESCRIPTION: �����¼��������Ʋ���
 �޸���ʷ  :
  1.��  ��   : 2017��03��11��
    ��  ��   : z00184470
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8              enMeasEvtEvalCustCtrlFlg;                   /* �����¼���������ʹ���ܿ���  */
    VOS_UINT8                       ucReserve1;                                 /* ����λ */
    VOS_UINT16                      usEvt1bTimeToTrigCustLen;                   /* 1b�¼�������ʱ������ʱ������λΪms */
    VOS_UINT16                      usReserve2;                                 /* ����λ */
    VOS_UINT16                      usReserve3;                                 /* ����λ */
    VOS_UINT16                      usReserve4;                                 /* ����λ */
    VOS_UINT16                      usReserve5;                                 /* ����λ */
    VOS_UINT16                      usReserve6;                                 /* ����λ */
    VOS_UINT16                      usReserve7;                                 /* ����λ */
    VOS_INT16                       sReserve8;                                  /* ����λ */
    VOS_INT16                       sReserve9;                                  /* ����λ */
}WAS_NVIM_MEAS_EVT_EVAL_CUSTOMIZE_CFG_STRU;
/*****************************************************************************

 �� �� ��  : WAS_NV_EMERGENCY_CAMP_CFG_STRU
 �ṹ˵��  : ��������ʱSuitableפ������NV�ṹ
 DESCRIPTION: ��ϵͳ��ѡ����Ż����ƿ���
 �޸���ʷ  :
  1.��  ��   : 2017��4��1��
    ��  ��   : j00169676
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8              enEmergencySuitableCampFlg;                 /* �����������ʱ�Ƿ�����suitableפ�� */
    VOS_UINT8                       ucMccNum;                                   /* ƥ��Ĺ����� */
    VOS_UINT8                       ucRsv1;
    VOS_UINT8                       ucRsv2;
    VOS_UINT32                      ulMccList[WAS_NV_MAX_EMERGENCY_SUITABLE_CAMP_MCC_NUM];
}WAS_NV_EMERGENCY_CAMP_CFG_STRU;

/*****************************************************************************
 �� �� ��  : WAS_NV_INTER_FREQ_HANDOVER_CUST_CFG_STRU
 �ṹ˵��  : ������Ƶ�л�Ƶ��ʧ����ض��Ʋ���
 DESCRIPTION: ������Ƶ�л�Ƶ��ʧ����ض��Ʋ���
 �޸���ʷ  :
  1.��  ��   : 2017��5��13��
    ��  ��   : z00184470
    �޸����� : �����ɽṹ
*****************************************************************************/
typedef struct
{
    PS_BOOL_ENUM_UINT8               enEnableFlg;                               /*Range:[0,1]*/
    VOS_INT8                         cServCellRscpThreshold;                    /*Range:[-125,0]*/
    VOS_INT8                         cServCellEcn0Threshold;                    /*Range:[-25,0]*/
    VOS_UINT8                        ucMaxInterFreqHoFailNum;
    VOS_UINT8                        ucMinRscpOffset;
    VOS_UINT8                        ucMaxRscpOffset;
    VOS_UINT8                        ucRscpOffsetStep;
    VOS_UINT8                        ucRsv1;
    VOS_UINT8                        ucRsv2;
    VOS_UINT8                        ucRsv3;
    VOS_UINT8                        ucRsv4;
    VOS_UINT8                        ucRsv5;
}WAS_NV_INTER_FREQ_HANDOVER_CUST_CFG_STRU;

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

#endif /* end of WasNvInterface.h */
