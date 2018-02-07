/******************************************************************************

  Copyright(C)2008,Hisilicon Co. LTD.

 ******************************************************************************
  File Name       : ToolNvInterface.h
  Description     : ToolNvInterface.h header file
  History         :

******************************************************************************/

#ifndef __TOOLNVINTERFACE_H__
#define __TOOLNVINTERFACE_H__

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

/*****************************************************************************
  2 Macro
*****************************************************************************/
#define GSM_RX_AGC_GAIN_NUM               (8)
#define W_RX_NOBLOCK_AGC_GAIN_MAX_COUNT   (8)
#define W_RX_BLOCK_AGC_GAIN_MAX_COUNT     (8)
#define WCDMA_PA_MODE_CNT                ((PA_MODE_BUTT) - (PA_MODE_HIGH))
#define W_PA_CNT                          (3)
#define NV_GSM_LINEAR_PA_GAIN_NUM         (4)
#define W_HI6363_PA_CNT                   (4)
/*****************************************************************************
  3 Massage Declare
*****************************************************************************/


/*****************************************************************************
  4 Enum
*****************************************************************************/
/* W PAģʽ */
enum  W_TX_PA_MODE_ENUM
{
    PA_MODE_AUTO  = 0,
    PA_MODE_HIGH  = 1,
    PA_MODE_MID   = 2,
    PA_MODE_LOW   = 3,
    PA_MODE_BUTT  = 4
};

/*****************************************************************************
  5 STRUCT
*****************************************************************************/
/*****************************************************************************
*                                                                            *
*                           ����������Ϣ�ṹ                                 *
*                                                                            *
******************************************************************************/

//Tcxo Cal
/*****************************************************************************
 NV��      : NV_XO_INIT_FREQUENCY_DEFAULT
 �ṹ����  : XO_AFC_FREQERR_DEFAULT_STRU
 �ṹ˵��  : XOУ׼����ֵ
*****************************************************************************/
typedef struct
{
    INT16                   sInitAfc;
    UINT16                  usFreqErrThreshold;
    INT16           sAfcLowerLimit;
    INT16           sAfcUpperLimit;
}XO_AFC_FREQERR_DEFAULT_STRU;

/*****************************************************************************
 NV��      : NV_DCXO_C_TRIM_DEFAULT
 �ṹ����  : XO_AFC_FREQERR_DEFAULT_STRU
 �ṹ˵��  : C_TRIM�ĳ�ʼֵ
*****************************************************************************/
typedef struct
{
    INT16                   sInitCfix;
    UINT16                  usFreqErrThreshold;                /* 0.1ppm */
    UINT16          usCfixLowerLimit;
    UINT16          usCfixUpperLimit;
}DCXO_C_TRIM_DEFAULT_STRU;


//W Trx Cal
/*****************************************************************************
 NV��      : NV_DXCO_C2_FIX
 �ṹ����  : DXCO_C2_FIX_DEFAULT
 �ṹ˵��  : C2 У׼����
*****************************************************************************/
typedef struct
{
    UINT16              usC2fixDefault;
    UINT16              usC2fixLowerLimit;
    UINT16              usC2fixUpperLimit;
    UINT16              usC2fixFactoryCal; //0��ʾ����Ҫ����У׼,������ʾ��Ҫ
}DXCO_C2_FIX_DEFAULT;
//W Trx Cal
/*****************************************************************************
 NV��      : NV_W_TX_APC_CAL_PARA
 �ṹ����  : NV_W_TX_APC_CAL_PARA_STRU
 �ṹ˵��  : Apc У׼����
*****************************************************************************/
typedef struct
{
    INT16   sBbGainCalUpperLimit10th;
    INT16   sBbGainCalLowerLimit10th;
    INT16   sBbGainCalExtend10th;
    INT16   sReserved[3];
}NV_W_TX_APC_CAL_PARA_STRU;

typedef NV_W_TX_APC_CAL_PARA_STRU NV_CDMA_TX_APC_CAL_PARA_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_APC_CAL_PARA
 �ṹ����  : NV_W_TX_APC_CAL_PARA_STRU
 �ṹ˵��  : hi6363 Apc У׼����
*****************************************************************************/
typedef struct
{
    INT16   sBbGainCalUpperLimit10th;
    INT16   sBbGainCalLowerLimit10th;
    INT16   sBbGainCalExtend10th;
    INT16   sDbbAtten10th;     // ������ʾ˥��,������ʾ����,����Rf GainУ׼��DbbԤ˥��
    INT16   sAptDbbAtten10th;
    INT16   sEtDbbAtten10th;   // ������ʾ˥��,������ʾ����
}NV_W_TX_CAL_PARA_STRU;

typedef NV_W_TX_CAL_PARA_STRU NV_CDMA_TX_CAL_PARA_STRU;
//W APT Comp Correction
/*****************************************************************************
 NV��      : NV_W_TX_PA_APT_COMP_CORRECTION
 �ṹ����  : NV_W_TX_APT_COMP_CORRECTION_STRU
 �ṹ˵��  : Apt����PA Gain mask
*****************************************************************************/
typedef struct
{
    UINT32 ulGainMask;
}NV_W_TX_APT_COMP_CORRECTION_STRU;

//RFIC Ctrl mask
/*****************************************************************************
 NV��      : NV_W_TX_RFIC_CTRL_MASK
 �ṹ����  : NV_W_TX_RFIC_CTRL_MASK_STRU
 �ṹ˵��  : RFIC����mask
*****************************************************************************/
typedef struct
{
    UINT32  ulRfGainMask;       // Consist of Vdd sel, Pmax gain, RF gain and RF bias
    UINT32  ulBbGainMask;       // Consist of RF BB gain
}NV_W_TX_RFIC_CTRL_MASK_STRU;


/*****************************************************************************
 NV��      : MAX_BB_GAIN_DBB_ATTEN_STRU
 �ṹ����  : MAX_BB_GAIN_DBB_ATTEN_STRU
 �ṹ˵��  : Max bb gain Dbb Atten
*****************************************************************************/
typedef struct
{
    UINT8  ucMaxRfBbGainIndex;
    INT8   cMaxDbbAtten;
}MAX_BB_GAIN_DBB_ATTEN_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_APC_GENERATION_RULE_ANY_PA_STRU
 �ṹ����  : NV_W_TX_APC_GENERATION_RULE_ANY_PA_STRU
 �ṹ˵��  : Apc Generation Rule
*****************************************************************************/
typedef struct
{
    UINT16 usRfGainLimit;
    UINT16 usValidCnt;
    MAX_BB_GAIN_DBB_ATTEN_STRU astData[20];
}NV_W_TX_APC_GENERATION_RULE_ANY_PA_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_APC_GENERATION_RULE
 �ṹ����  : NV_W_TX_APC_GENERATION_RULE_STRU
 �ṹ˵��  : Apc Generation Rule
*****************************************************************************/
typedef struct
{
    NV_W_TX_APC_GENERATION_RULE_ANY_PA_STRU astAnyPaApcRule[W_PA_CNT];
}NV_W_TX_APC_GENERATION_RULE_STRU;

typedef NV_W_TX_APC_GENERATION_RULE_STRU NV_CDMA_TX_APC_GENERATION_RULE_STRU;

/*****************************************************************************
 NV��      : MAX_BB_GAIN_DBB_ATTEN_STRU
 �ṹ����  : MAX_BB_GAIN_DBB_ATTEN_STRU
 �ṹ˵��  : hi6363 Max bb gain Dbb Atten
*****************************************************************************/
typedef struct
{
    UINT8  ucMaxRfBbGainIndex;
    UINT8  ucRfBiasIndex;
    UINT8  aucRsv[2];
}MAX_BB_GAIN_BIAS_INDEX_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_GENERATION_RULE_ANY_PA_STRU
 �ṹ����  : NV_W_TX_GENERATION_RULE_ANY_PA_STRU
 �ṹ˵��  : hi6363 Apc Generation Rule
*****************************************************************************/
typedef struct
{
    UINT16 usRfGainLimit;
    UINT16 usValidCnt;
    MAX_BB_GAIN_BIAS_INDEX_STRU astData[20];
}NV_W_TX_GENERATION_RULE_ANY_PA_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_APC_GENERATION_RULE
 �ṹ����  : NV_W_TX_APC_GENERATION_RULE_STRU
 �ṹ˵��  : hi6363 Apc Generation Rule
*****************************************************************************/
typedef struct
{
    NV_W_TX_GENERATION_RULE_ANY_PA_STRU astAnyPaApcRule[W_HI6363_PA_CNT];
}NV_W_TX_GENERATION_RULE_STRU;

typedef NV_W_TX_GENERATION_RULE_STRU NV_CDMA_TX_GENERATION_RULE_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_RF_BIAS_INDEX_(VOICE_MODE)_B*
 �ṹ����  : NV_W_TX_RF_BIAS_INDEX_STRU
 �ṹ˵��  : Rf Bias Index
*****************************************************************************/
typedef struct
{
    UINT32  ulValidCount;
    UINT16  ausRfBiasIndex[20];
}NV_W_TX_RF_BIAS_INDEX_STRU;

typedef NV_W_TX_RF_BIAS_INDEX_STRU NV_CDMA_TX_RF_BIAS_INDEX_STRU;

/*****************************************************************************
 NV��      : NV_W_RX_AGC_GAIN_CAL_PARA
 �ṹ����  : NV_W_RX_AGC_GAIN_CAL_PARA_STRU
 �ṹ˵��  : Agc Gain Cal Para
*****************************************************************************/
typedef struct
{
    INT16   asNoBlockIor[W_RX_NOBLOCK_AGC_GAIN_MAX_COUNT];
    INT16   asBlockIor[W_RX_BLOCK_AGC_GAIN_MAX_COUNT];
}NV_W_RX_AGC_GAIN_CAL_PARA_STRU;

typedef NV_W_RX_AGC_GAIN_CAL_PARA_STRU NV_CDMA_RX_AGC_GAIN_CAL_PARA_STRU;

/*****************************************************************************
 NV��      : NV_WG_RX_AGC_GAIN_CAL_THRESHOLD
 �ṹ����  : NV_WG_RX_AGC_GAIN_CAL_THRESHOLD_STRU
 �ṹ˵��  : Rx Agc Cal Threshold
*****************************************************************************/
typedef struct
{
    INT16   sUpLimit;
    INT16   sLowLimit;
}NV_WG_RX_AGC_GAIN_CAL_THRESHOLD_STRU;


//W Pd Cal
/*****************************************************************************
 NV��      : NV_W_TX_HDET_CAL_PARA_B*
 �ṹ����  : NV_W_PD_CAL_PARA_ANYBAND_STRU
 �ṹ˵��  : Pd Cal Para
*****************************************************************************/
typedef struct
{
    INT16   sMinPowerLmt10th;
    UINT16  usMinPdLmt;
    INT16   sMaxPowerLmt10th;
    UINT16  usMaxPdLmt;
}NV_W_PD_CAL_PARA_ANYBAND_STRU

//��չNV_W_PD_CAL_PARA_ANYBAND_STRU�ṹ�壬����Eagle PA
typedef struct
{
       INT16   sMinPowerLmt10th;
       UINT16  usMinPdLmt;
       INT16   sMaxPowerLmt10th;
       UINT16  usMaxPdLmt;
       INT16   sSampleRangeLowLmt;
       INT16   sSampleRangeUpperLmt;
}NV_W_PD_CAL_PARA_EXTEND_ANYBAND_STRU;
typedef NV_W_PD_CAL_PARA_ANYBAND_STRU NV_CDMA_PD_CAL_PARA_ANYBAND_STRU;


/*****************************************************************************
 NV��      : NV_W_HDET_VGA_CAL_LIST
 �ṹ����  : NV_W_HDET_VGA_CAL_LIST_STRU
 �ṹ˵��  : Pd Vga Cal List
*****************************************************************************/
typedef struct
{
    UINT32  ulValidCount;
    UINT16  usPdVga[16];
}NV_W_HDET_VGA_CAL_LIST_STRU;

typedef NV_W_HDET_VGA_CAL_LIST_STRU NV_CDMA_HDET_VGA_CAL_LIST_STRU;

/*****************************************************************************
 NV��      : NV_W_HDET_VGA_THRESHOLD
 �ṹ����  : NV_W_HDET_VGA_THRESHOLD_STRU
 �ṹ˵��  : Pd VGA THRESHOLD
*****************************************************************************/
typedef struct
{
    UINT16      usVgaThreshold;
}NV_W_HDET_VGA_THRESHOLD_STRU;

typedef NV_W_HDET_VGA_THRESHOLD_STRU NV_CDMA_HDET_VGA_THRESHOLD_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_HDET_VGA_B*
 �ṹ����  : NV_W_HDET_VGA_STRU
 �ṹ˵��  : Pd VGA
*****************************************************************************/
typedef struct
{
    UINT16      usPdVga;
}NV_W_HDET_VGA_STRU;

/*****************************************************************************
 NV��      : NV_W_HDET_HKADC_PARA
 �ṹ����  : NV_W_HDET_HKADC_PARA_STRU
 �ṹ˵��  : Pd Hkadc Para
*****************************************************************************/
typedef struct
{
    UINT16  usRefVolt;
    UINT16  usBitWidth;
}NV_W_HDET_HKADC_PARA_STRU;

typedef NV_W_HDET_HKADC_PARA_STRU NV_CDMA_HDET_HKADC_PARA_STRU;

/*****************************************************************************
 NV��      : NV_W_HDET_DCOFFSET_THRESHOLD
 �ṹ����  : NV_W_HDET_DCOFFSET_THRESHOLD_STRU
 �ṹ˵��  : Pd DcOffset Threshold
*****************************************************************************/
typedef struct
{
    INT16  sTarget;
    INT16  sLowerLimit;
    INT16  sUpperLimit;
}NV_W_HDET_DCOFFSET_THRESHOLD_STRU;

typedef NV_W_HDET_DCOFFSET_THRESHOLD_STRU NV_CDMA_HDET_DCOFFSET_THRESHOLD_STRU;

/*****************************************************************************
 NV��      : NV_W_HDET_DCOFFSET_CAL_LIST
 �ṹ����  : NV_W_HDET_DCOFFSET_CAL_LIST_STRU
 �ṹ˵��  : Pd DcOffset Para
*****************************************************************************/
typedef struct
{
    UINT16   usMaxDcoffset;
    UINT16   usMinDcoffset;
}NV_W_HDET_DCOFFSET_CAL_LIST_STRU;

typedef NV_W_HDET_DCOFFSET_CAL_LIST_STRU NV_CDMA_HDET_DCOFFSET_CAL_LIST_STRU;

/*****************************************************************************
 NV��      : NV_W_HDET_VGA_THRESHOLD
 �ṹ����  : NV_W_HDET_DCOFFSET_STRU
 �ṹ˵��  : Pd DcOffset
*****************************************************************************/
typedef struct
{
    UINT16   usPdDcOffset;
}NV_W_HDET_DCOFFSET_STRU;

//W FreqComp Cal
/*****************************************************************************
 NV��      : NV_WG_RX_FREQ_COMP_CAL_THRESHOLD
 �ṹ����  : NV_WG_RX_FREQ_COMP_CAL_THRESHOLD_STRU
 �ṹ˵��  : Rx Freq Comp Cal Threshold
*****************************************************************************/
typedef struct
{
    INT16   sUpLimit;
    INT16   sLowLimit;
}NV_WG_RX_FREQ_COMP_CAL_THRESHOLD_STRU;


//W APT Cal


/*****************************************************************************
 NV��      : NV_W_TX_APT_CAL_PARA_B*
 �ṹ����  : APT_CAL_PARA_ITEM_STRU
 �ṹ˵��  : Pa APT
 *****************************************************************************/
typedef struct
{
    INT16      sTargetAclr10th;
    INT16      sAclrMultiplierUnit;
    INT16      sAclrDividerUnit;
    INT16      sPowerMultiplierUnit;
    INT16      sPowerDividerUnit;
    INT16      sVoltDeltaLowerLimit;
    INT16      sVoltDeltaUpperLimit;
}APT_CAL_PARA_ITEM_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_APT_CAL_PARA_B*
 �ṹ����  : NV_W_TX_APT_CAL_PARA_STRU
 �ṹ˵��  : Pa APT
 *****************************************************************************/
typedef struct
{
    UINT16                 usAptCalMask;
    APT_CAL_PARA_ITEM_STRU astAptCalPara[3];
}NV_W_TX_APT_CAL_PARA_STRU;

typedef NV_W_TX_APT_CAL_PARA_STRU NV_CDMA_TX_APT_CAL_PARA_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_APT_CAL_PARA_B*
 �ṹ����  : NV_W_TX_APT_CAL_PARA_STRU
 �ṹ˵��  : hi6363 Pa APT
 *****************************************************************************/
typedef struct
{
    UINT16                 usAptCalMask;
    APT_CAL_PARA_ITEM_STRU astAptCalPara[4];
}NV_W_APT_CAL_PARA_STRU;

typedef NV_W_APT_CAL_PARA_STRU NV_CDMA_APT_CAL_PARA_STRU;

//GSM TX cal
/*****************************************************************************
 NV��      : NV_GSM*_TX_SWEEP_DAC_RANGE
 �ṹ����  : NV_GSM_BAND_CTRL_VOLT_THRESHOLD
 �ṹ˵��  : GSM Range
 *****************************************************************************/
typedef struct
{
    UINT16  usMaxDac;
    UINT16  usMultiSlotsDac;
    UINT16  usMinDac;
}NV_GSM_BAND_CTRL_VOLT_THRESHOLD;

/*****************************************************************************
 NV��      : NV_GSM_RX_AGC_GAIN_VALID_RANGE
 �ṹ����  : NV_GSM_RX_AGC_GAIN_VALID_RANGE_STRU
 �ṹ˵��  : Gsm Rx agc gain Range
*****************************************************************************/

typedef struct
{
    UINT16  usGainRange;
}NV_GSM_RX_AGC_GAIN_VALID_RANGE_STRU;


/*****************************************************************************
 NV��      : NV_GSM_RX_AGC_GAIN_CAL_PARA
 �ṹ����  : NV_GSM_RX_GAIN_CTRL_PARA_STRU
 �ṹ˵��  : Gsm Rx Agc Gain Ctrl
*****************************************************************************/
typedef struct
{
    INT16  sAgclor[8];
}NV_GSM_RX_GAIN_CTRL_PARA_STRU;

//Edge Tx Cal
/*****************************************************************************
 NV��      : NV_EDGE_TX_RF_GAIN_CTRL
 �ṹ����  : NV_EDGE_RF_GAIN_CTRL_STRU
 �ṹ˵��  : Edge RF Gain Ctrl
*****************************************************************************/
typedef struct
{
    UINT32  ulValidCount;
    UINT16  ausRfGainCtrl[20];
}NV_EDGE_RF_GAIN_CTRL_STRU;

/*****************************************************************************
 NV��      : NV_EDGE*_TX_RF_BIAS_INDEX
 �ṹ����  : NV_EDGE_RF_GAIN_CTRL_STRU
 �ṹ˵��  : Edge RF Bias Index
*****************************************************************************/
typedef struct
{
    UINT32  ulValidCount;
    UINT16  ausRfBaisIndex[20];
}NV_EDGE_RF_BASI_INDEX_STRU;

/*****************************************************************************
 NV��      : NV_EDGE*_TX_RF_BIAS
 �ṹ����  : NV_EDGE_RF_BIAS_STUR
 �ṹ˵��  : Edge RF Bias
*****************************************************************************/
typedef struct
{
    UINT32  ulValidCount;
    UINT16  ausRfBias[16];

}NV_EDGE_RF_BIAS_STUR;

/*****************************************************************************
 NV��      : NV_EDGE_TX_RFIC_BB_GAIN_TABLE
 �ṹ����  : BB_GAIN_CTRL_VS_ATTEN_STRU
 �ṹ˵��  : BB Gain Table
*****************************************************************************/
typedef struct
{
    UINT16   usBbGainCtrl;
    INT16    sBbAtten10th;
}BB_GAIN_CTRL_VS_ATTEN_STRU;

/*****************************************************************************
 NV��      : NV_EDGE_TX_RFIC_BB_GAIN_TABLE
 �ṹ����  : NV_EDGE_BB_GAIN_STRU
 �ṹ˵��  : Edge BB Gain
*****************************************************************************/
typedef struct
{
    UINT32  ulValidCount;
    BB_GAIN_CTRL_VS_ATTEN_STRU astBbGain[32];
}NV_EDGE_BB_GAIN_STRU;

/*****************************************************************************
 NV��      : NV_EDGE*_TX_APC_GENERATION_RULE
 �ṹ����  : NV_EDGE_TX_APC_GENERATION_RULE
 �ṹ˵��  : Edge Tx Apc Generation Rule
*****************************************************************************/
typedef struct
{
    INT16  sDbbAtten10thAllowed;
    UINT32 ulValidCnt;
    UINT16 ausRfBbGainIndex[20];
}NV_EDGE_TX_APC_GENERATION_RULE;

/*****************************************************************************
 NV��      : NV_EDGE*_TX_RF_GAIN_ATTEN
 �ṹ����  : NV_EDGE_RF_GAIN_ATTEN_STRU
 �ṹ˵��  : Edge RF Gain Atten
*****************************************************************************/
typedef struct
{
    UINT32  ulValidCount;
    INT16   asRfAtten10th[20];
}NV_EDGE_RF_GAIN_ATTEN_STRU;


/*****************************************************************************
 NV��      : NV_EDGE_TX_APC_CAL_PARA
 �ṹ����  : NV_EDGE_TX_APC_CAL_PARA_STRU
 �ṹ˵��  : Apc Cal Para
*****************************************************************************/
typedef struct
{
     INT16  sGainAtten10th;
}NV_EDGE_TX_APC_CAL_PARA_STRU;

/*****************************************************************************
 NV��      : NV_GSM*_RX_AGC_GAIN_DEFAULT
 �ṹ����  : NV_GSM_ANY_BAND_RX_AGC_GAIN_STRU
 �ṹ˵��  : Apc Cal Para
*****************************************************************************/
typedef struct
{
    INT16   asPower8th[GSM_RX_AGC_GAIN_NUM];
}NV_GSM_ANY_BAND_RX_AGC_GAIN_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_LINEAR_PA_GAIN_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����PA������
*****************************************************************************/
typedef struct
{
    UINT16                          usNumber;
    INT16                           shwGain[NV_GSM_LINEAR_PA_GAIN_NUM];
}NV_EDGE_LINEAR_PA_GAIN_STRU;


/*****************************************************************************
 NV��      : NV_GSM*_PA_PRECHG_CAL_THRESHOLD
 �ṹ����  : NV_GSM_BAND_PA_PRECHG_CAL_THRESHOLD_STRU
 �ṹ˵��  : Gsm Pa Precharge
*****************************************************************************/
typedef struct
{
    INT16   usNvIdGsmPaPrechargeThreshold;
}NV_GSM_BAND_PA_PRECHG_CAL_THRESHOLD_STRU;


/*****************************************************************************
 NV��      : NV_W_TX_PA_GAIN_DEFAULT_B*
 �ṹ����  : NV_W_TX_PA_GAIN_STRU
 �ṹ˵��  : Pa Gain
*****************************************************************************/
typedef struct
{
    INT16  asPaGain10th[W_PA_CNT];
}NV_W_TX_PA_GAIN_STRU;

typedef NV_W_TX_PA_GAIN_STRU NV_CDMA_TX_PA_GAIN_STRU;


/*****************************************************************************
 NV��      : NV_W_TX_PA_GAIN_DEFAULT_B*
 �ṹ����  : NV_W_TX_PA_GAIN_STRU
 �ṹ˵��  : hi6363 Pa Gain
*****************************************************************************/
typedef struct
{
    INT16  asPaGain10th[W_HI6363_PA_CNT];
}NV_W_PA_GAIN_STRU;

typedef NV_W_PA_GAIN_STRU NV_CDMA_PA_GAIN_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_RF_GAIN_ATTEN_HIGH(MID,LOW)_GAIN_(VOICE_MODE)_B*
 �ṹ����  : NV_W_TX_RF_GAIN_ATTEN_STRU
 �ṹ˵��  : Rf Gain Atten
*****************************************************************************/
typedef struct
{
    UINT32  ulValidCount;
    INT16   asRfAtten10th[20];
}NV_W_TX_RF_GAIN_ATTEN_STRU;

typedef NV_W_TX_RF_GAIN_ATTEN_STRU NV_CDMA_TX_RF_GAIN_ATTEN_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_PA_APT_TABLE(COMP)_DEFAULT_B*
 �ṹ����  : W_APT_STRU
 �ṹ˵��  : APT
*****************************************************************************/
typedef struct
{
    INT16      asPwr10th;
    UINT16     usVcc;
    UINT16     usBias1;
    UINT16     usBias2;
    UINT16     usBias3;
}W_APT_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_PA_APT_TABLE(COMP)_DEFAULT_B*
 �ṹ����  : NV_W_TX_APT_TABLE_STRU
 �ṹ˵��  : APT
*****************************************************************************/
typedef struct
{
    UINT16      usAptStepMode;
    UINT16      ausValidCount[3];
    W_APT_STRU  astAptTable[96];
}NV_W_TX_APT_TABLE_STRU;

typedef NV_W_TX_APT_TABLE_STRU NV_CDMA_TX_APT_TABLE_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_PA_APT_TABLE(COMP)_DEFAULT_B*
 �ṹ����  : NV_W_TX_APT_TABLE_STRU
 �ṹ˵��  : hi6363 APT default
*****************************************************************************/
typedef struct
{
    UINT16      usAptStepMode;
    UINT16      ausValidCount[4];
    W_APT_STRU  astAptTable[96];
}NV_W_APT_TABLE_STRU;

typedef NV_W_APT_TABLE_STRU NV_CDMA_APT_TABLE_STRU;

/*****************************************************************************
 NV��      : NV_W_TX_PA_APT_TABLE(COMP)_DEFAULT_B*
 �ṹ����  : NV_W_TX_GAIN_APT_COMP_STRU
 �ṹ˵��  : APT COMP
*****************************************************************************/
typedef struct
{
    INT16  ashwAptComp[96];
}NV_W_TX_GAIN_APT_COMP_STRU;

typedef NV_W_TX_GAIN_APT_COMP_STRU NV_CDMA_TX_GAIN_APT_COMP_STRU;

/*****************************************************************************
 NV��      : NV_GE_TX_LINEAR_PA_GAIN_CAL_PARA
 �ṹ����  : NV_GE_TX_LINEAR_PA_GAIN_CAL_PARA_STRU
 �ṹ˵��  : LINEAR PA GAIN PARA
*****************************************************************************/
typedef struct
{
    UINT16  usCalPcl[NV_GSM_LINEAR_PA_GAIN_NUM];
}NV_GE_TX_LINEAR_PA_GAIN_CAL_PARA_STRU;

/*****************************************************************************
 NV��      : NV_GE_TX_LINEAR_APC_CAL
 �ṹ����  : NV_GE_TX_LINEAR_APC_CAL_STRU
 �ṹ˵��  : DBBĬ��˥��ֵ
*****************************************************************************/
typedef struct
{
    INT16  sDbbAtten10th;
}NV_GE_TX_LINEAR_APC_CAL_STRU;


/*****************************************************************************
 NV��      : NV_GSM_RX_IQ_MISMATCH_CAL_PARA
 �ṹ����  : NV_GSM_RX_IQ_MISMATCH_CAL_PARA_STRU
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    INT16   sRefIor10th;
    INT16   sAmpCorrThreshold;
    INT16   sPhsCorrThreshold;
}NV_GSM_RX_IQ_MISMATCH_CAL_PARA_STRU;


/*****************************************************************************
 NV��      : NV_ET_COEFF_CAL_PARA_ANYBAND
 �ṹ����  : NV_ET_COEFF_CAL_PARA_ANYBAND_STRU
 �ṹ˵��  : ET Lut������У׼����
*****************************************************************************/
typedef struct
{
    UINT32  ulSampleCnt;
    INT16   asPower[8];
}NV_ET_COEFF_CAL_PARA_ANYBAND_STRU;


/*****************************************************************************
 NV��      : ��
 �ṹ����  : NV_ET_LUT_CAL_PARA_STRU
 �ṹ˵��  : ET LUTУ׼����ȡ���ò���
*****************************************************************************/
typedef struct
{
    UINT32  ulSampleVccCnt;                             /* �����ѹ������Ĭ��5 */
    INT16   asVccRef[32];                               /* �����ѹ,��λmv��ֵ��ȡ��� */
    INT16   sPaGainComp;                                /* ET��λ��PA Gain������ԼΪѹ��������λ0.1dB��Ĭ��30 */
    INT16   sEtIsoCompress;                             /* ETѹ��������λ0.1dB��Ĭ��30 */
    INT16   sEtLutPower10th;                            /* ET LUTУ׼����ȡʱ�Ĺ��ʣ���λ0.1dB��Ĭ��235 */
    UINT16  usPaVbias;                                  /* ET LUTУ׼����ȡʱPaVbias��ѹ���ã���Чֵ0-255��Ĭ��180 */
    INT16   usGainLimitUpper;                           /* ET LUTУ׼ʱGain�����ޣ�Ĭ��ֵ14����1.4 */
    INT16   usGainLimitLower;                           /* ET LUTУ׼ʱGain�����ޣ�Ĭ��ֵ6����0.6 */
    INT16   sEtDbbAtten;                                /* ET LUTУ׼����ȡʱ�����ֻ��˹��ʣ���λ0.1dB��Ĭ��40 */
    INT16   sShiftPower;                                /* ��ȡ��lut���й���P��ƽ�ƺ��Ŀ�깦P1�ʲ�ֵ��sShiftPower = P - P1 */
    UINT16  usEtCalMode;                                /* Ĭ��ֵ 0: ������ETУ׼��һ����ѹ������
                                                                  1: ������ETУ׼��һ����ѹ������
                                                                  2: ��ѹ��ETУ׼ѹ������
                                                                  3: ��ѹ��ETУ׼ѹ������ */
    UINT16  usRsv1;                                     /* Ĭ��ֵ 0 */
    UINT16  usRsv2;                                     /* Ĭ��ֵ 0 */
    UINT16  usRsv3;                                     /* Ĭ��ֵ 0 */

}NV_ET_LUT_CAL_PARA_STRU;

/*****************************************************************************
 NV��      : NV_ET_LUT_CAL_PARA_ANYBAND_STRU
 �ṹ����  : NV_ET_LUT_CAL_PARA_ANYBAND_STRU
 �ṹ˵��  : ET LUTУ׼����ȡ���ò���
*****************************************************************************/
typedef struct
{
    NV_ET_LUT_CAL_PARA_STRU     stHEtCalPara;           /* �ߵ�ETУ׼���ò��� */
    NV_ET_LUT_CAL_PARA_STRU     stMEtCalPara;           /* �е�ETУ׼���ò��� */
}NV_ET_LUT_CAL_PARA_ANYBAND_STRU;

/*****************************************************************************
 NV��      : NV_EDGE_RF_GAIN_CTRL_HI6363
 �ṹ����  : NV_EDGE_RF_GAIN_CTRL_HI6363_STRU
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    UINT32  ulValidCount;
    UINT32  aulRfGainCtrl[20];
}NV_EDGE_RF_GAIN_CTRL_HI6363_STRU;

/*****************************************************************************
 NV��      : NV_EDGE_RF_BIAS
 �ṹ����  : NV_EDGE_RF_BIAS_STRU
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    INT32   lValidCount;
    INT32   ulRfBias[16];

}NV_EDGE_RF_BIAS_STRU;

/*****************************************************************************
 NV��      : BB_GAIN_CTRL_VS_ATTEN_HI6363
 �ṹ����  : BB_GAIN_CTRL_VS_ATTEN_HI6363_STRU
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    UINT32  ulBbGainCtrl;
    INT32   lBbAtten10th;
}BB_GAIN_CTRL_VS_ATTEN_HI6363_STRU;

/*****************************************************************************
 NV��      : NV_EDGE_TX_RFIC_BB_GAIN_CTRL_TABLE_LHB
 �ṹ����  : NV_EDGE_BB_GAIN_HI6363_STRU
 �ṹ˵��  : EDGE HI6363/6351 BB gain �����ֱ�
*****************************************************************************/
typedef struct
{
    INT32  ulValidCount;
    BB_GAIN_CTRL_VS_ATTEN_HI6363_STRU astBbGain[32];
}NV_EDGE_BB_GAIN_HI6363_STRU;

/*****************************************************************************
 NV��      : NV_GUC_MRX_GAIN_MEAS_THRESHOLD_STRU
 �ṹ����  : NV_GUC_MRX_GAIN_MEAS_THRESHOLD_STRU
 �ṹ˵��  : hi6363 MRX GAIN�л�����
*****************************************************************************/
typedef struct
{
    INT16 asMrxGainThreshold[5];
    INT16 sRsv;
}NV_GUC_MRX_GAIN_MEAS_THRESHOLD_STRU;

/*****************************************************************************
 NV��      : NV_<GSMBand>_TX_APC_CAL_PARA_TABLE
 �ṹ����  : NV_GSM_TX_APC_CAL_PARA_TABLE_STRU
 �ṹ˵��  : GSMУ׼�������
*****************************************************************************/
typedef struct
{
    UINT16 usPaGainVaildCnt;		/*TX��Ч��λ����*/
    UINT16 usRsv0;
    INT16 asPaGain[4];			/*TX����λ����*/
    UINT16 ausPaGainCalPcl[4];		/*����λУ׼ʱ���õ�PCL�ȼ�*/
    INT16 usRsv1;

    UINT16 usMaxDacHG;			/*RFIC High GainɨDAC��ʱ���DACֵ;ȡֵ��Χ��0~4095*/
    UINT16 usMultiSlotsDacHG;		/*RFIC High Gain����4ʱ϶ɨDAC��ʱ���DACֵ;ȡֵ��Χ��0~4095*/
    UINT16 usMinDacHG;			/*RFIC High GainɨDAC��ʱ��СDACֵ;ȡֵ��Χ��0~4095*/
    UINT16 usFrameNumsOf1SlotHG;	/*RFIC High Gain��slotУ׼֡��*/
    UINT16 usFrameNumsOf4SlotsHG;	/*RFIC High Gain 4slotУ׼֡��*/

    INT16 sPrechargeThreshold;		/*PrechargeУ׼����*/
    UINT16 usPrechgVoltInit;		/*PrechargeУ׼Ĭ��ֵ*/

    UINT16 usMaxDacLG;			/*RFIC Low GainɨDAC��ʱ���DACֵ;ȡֵ��Χ��0~4095*/			
    UINT16 usMultiSlotsDacLG;		/*RFIC Low Gain����4ʱ϶ɨDAC��ʱ���DACֵ;ȡֵ��Χ��0~4095*/
    UINT16 usMinDacLG;			/*RFIC Low GainɨDAC��ʱ��СDACֵ;ȡֵ��Χ��0~4095*/
    UINT16 usFrameNumsOf1SlotLG;	/*RFIC Low Gain��slotУ׼֡��*/
    UINT16 usFrameNumsOf4SlotsLG;	/*RFIC Low Gain 4slotУ׼֡��*/
    
    UINT16 usRsv2;
}NV_GSM_PA_TX_CAL_PARA_STRU;

typedef struct
{
    NV_GSM_PA_TX_CAL_PARA_STRU stSaturatedPa;
    NV_GSM_PA_TX_CAL_PARA_STRU stLinearPa;
}NV_GSM_TX_APC_CAL_PARA_TABLE_STRU;

/*****************************************************************************
 NV��      : NV_<EDGEBand>_TX_APC_CAL_PARA_TABLE
 �ṹ����  : NV_EDGE_TX_APC_CAL_PARA_TABLE_STRU
 �ṹ˵��  : EDGEУ׼�������
*****************************************************************************/
typedef struct
{
    UINT16 usPaGainVaildCnt;
    UINT16 usRsv;
    INT16 asPaGain[4];
    UINT16 usPaGainCalPcl[4];
    INT16 sDbbAtten10th;

    UINT16 ausRsv[13];
}NV_EDGE_PA_TX_CAL_PARA_STRU;

typedef struct
{
    NV_EDGE_PA_TX_CAL_PARA_STRU stSaturatedPa;
    NV_EDGE_PA_TX_CAL_PARA_STRU stLinearPa;
}NV_EDGE_TX_APC_CAL_PARA_TABLE_STRU;

/*****************************************************************************
 NV��      : NV_<EDGEBand>_TX_APC_GENERATION_RULE
 �ṹ����  : NV_EDGE_TX_APC_CAL_PARA_STRU
 �ṹ˵��  : EDGE TX APC������ɹ���
*****************************************************************************/
typedef struct
{
    UINT16 usMaxBbGainIndex;       //��ǰRF Gain��Ӧ�����BB Gain��λ
    UINT16 usRfBiasIndex;          // ��ǰRF Gain��Ӧ��RF Bias��λ
    UINT16 usRsv[2];               // Ԥ��
}NV_RFIC_RF_GAIN_CFG_STRU;

typedef struct
{
    UINT32 ulValidCount;
    NV_RFIC_RF_GAIN_CFG_STRU astRfGainCfg[20];
    UINT16 ausRsv[20];
}NV_EDGE_TX_APC_GENERATION_RULE_STRU;

/*****************************************************************************
 NV��      : NV_GSM_RF_RX_AGC_GAIN_TABLE_BANK
 �ṹ����  : NV_MODEM_RF_RX_AGC_GAIN_TABLE_BANK_STRU
 �ṹ˵��  : GSMģʽ��AGC Table��Դ��
*****************************************************************************/
typedef struct
{
    INT16 asGainNBlock[8];
    INT16 asGainBlock[8];
    INT16 asRsv[8];
}NV_GUC_RX_AGC_GAIN_DEFAULT_STRU;

typedef struct
{
    NV_GUC_RX_AGC_GAIN_DEFAULT_STRU astAgcGainDefaultTable[10];
}NV_MODEM_RF_RX_AGC_GAIN_TABLE_BANK_STRU;

/*****************************************************************************
 NV��      : NV_<GSMBand>_RF_RX_RFFE_ILOSS
 �ṹ����  : NV_MODEM_RF_RX_RFFE_ILOSS_STRU
 �ṹ˵��  : ÿ��Band��Ӧͨ����ǰ�˲����ֵ
*****************************************************************************/
typedef struct
{
    INT16 sAnt0RffeIloss;
    INT16 sAnt1RffeIloss;
    INT16 ulbitRsv0;
    INT16 ulbitRsv1;
}NV_MODEM_RF_RX_RFFE_ILOSS_STRU;

/*****************************************************************************
 NV��      : NV_GUC_RF_RX_AGC_CAL_THRESHOLD
 �ṹ����  : NV_MODEM_RF_RX_CAL_THRESHOLD_STRU
 �ṹ˵��  : ������ʽ��AGC GainУ׼����
*****************************************************************************/
typedef struct
{
    INT16 sUpperLimit;
    INT16 sLowerLimit;
}NV_RF_RX_CAL_THRESHOLD_STRU;

typedef struct
{
    /* AGC GainУ׼���� */
    NV_RF_RX_CAL_THRESHOLD_STRU stAgcGain;
    /* Freq CompУ׼���� */
    NV_RF_RX_CAL_THRESHOLD_STRU stFreqComp;
}NV_MODEM_RF_RX_CAL_THRESHOLD_STRU;


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

#endif /* end of NasNvInterface.h */
