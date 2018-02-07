/******************************************************************************

                  ��Ȩ���� (C), 2001-2011, ��Ϊ�������޹�˾

 ******************************************************************************
  �� �� ��   : phyNvDefine_Boston
  �� �� ��   : ����
  ��    ��   : h00165915
  ��������   : 2016��7��1��
  ����޸�   :
  ��������   : PhyNvDefine.h ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��7��1��
    ��    ��   : h00165915
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __PHYNVDEFINE_BOSTON_H__
#define __PHYNVDEFINE_BOSTON_H__


/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#include "nv_common_interface.h"

#pragma pack(1)

/*****************************************************************************
  2 �궨��
*****************************************************************************/

/* ET֧�ֵ����MIPI���� */
#define UCOM_NV_ET_MIPI_NUM             ( 6 )

#define UCOM_NV_ET_APT_TRIGGER_NUM      ( 2 )
#define UCOM_NV_W_TX_DCDC_MIPI_NUM      ( 4 )                                   /* DCDC MIPI���� */

/* CDMA tas mipi���Ԫ���� */
#define UCOM_NV_CDMA_TAS_MIPI_NUM       ( 4 )
#define UCOM_NV_W_TEMP_NUM              ( 0x10  )                               /* �¶Ȳ������õ��¶ȵ㣬ǰ5���ֱ���� -20,0,20,40,60����6�������ֽڶ��� */

/* MRX���֧����Ч��λ,Ŀǰ���֧��5�� */
#define UCOM_NV_W_MRX_GAIN_MAX_NUM      ( 5 )

#define UCOM_NV_GUC_MIPI_INIT_UNIT_MAX_NUM      ( 16 )

#define UCOM_NV_TAS_DPDT_MIPI_UNIT_MAX_NUM      ( 4 )
#define UCOM_NV_G_NOTCH_MIPI_UNIT_MAX_NUM       ( 4 )
#define UCOM_NV_G_PAVCC_MIPI_UNIT_MAX_NUM       ( 4 )
#define UCOM_NV_G_TUNER_MIPI_UNIT_MAX_NUM       ( 4 )
#define UCOM_NV_G_MIPI_INIT_UNIT_MAX_NUM        ( 16 )
#define UCOM_NV_MAX_MIPI_ANT_UNIT_NUMBER        ( 4 )

#define UCOM_NV_RFIC_SETTING_LIST_NUM           ( 10 )                          /* Rf Setting���������10�� */
#define UCOM_NV_RFIC_SETTING_REG_NUM            ( 3 )                           /* ÿ��Rf Setting�Ĵ���������3�� */
/*****************************************************************************
 �ṹ��    : UCOM_NV_FEM_PIN_TO_GPIO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : NV_FEMPIN_TO_GPIO_STRU������tl drv��nvͷ�ļ���
*****************************************************************************/
typedef NV_FEMPIN_TO_GPIO_STRU UCOM_NV_FEM_PIN_TO_GPIO_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GPIO_MIPI_CTRL_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : FEM_GPIO_MIPIDEV_CTRL_STRU������tl drv��nvͷ�ļ���
*****************************************************************************/
typedef FEM_GPIO_MIPIDEV_CTRL_STRU UCOM_NV_GPIO_MIPI_CTRL_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_RFFE_GPIO_VALUE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : RFFE_GPIO_VALUE_STRU������tl drv��nvͷ�ļ���
*****************************************************************************/
typedef RFFE_GPIO_VALUE_STRU UCOM_NV_RFFE_GPIO_VALUE_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_ANT_SWITCH_MIPI_CTRL_WORD_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : RF_NV_MIPIDEV_04CMD������tl drv��nvͷ�ļ���
*****************************************************************************/
typedef RF_NV_MIPIDEV_04CMD UCOM_NV_ANT_SWITCH_MIPI_CTRL_WORD_STRU;

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

/*****************************************************************************
  4 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  6 STRUCT����
*****************************************************************************/

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_BAND_DL_RFIC_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����RFIC����,����RFIC ID,ͨ���ţ������PORT��
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitRficIdx                  :2;         /* RFIC ID��[0,2]�ֱ��ʾRFIC1��RFIC2��RFIC3 */
    VOS_UINT32                          bitRficRxCh                 :2;         /* ����RXͨ���ţ�ÿ��RFIC��3��ͨ�����ֱ���RX1��RX2��RX3; ��������4*4MIMO�̶�ʹ��RX1A1B+RX3A3B,���ǲ���ͬһ��TX_PLL1���ƣ����MIMO��������ѡRXͨ������4����2��ʱ��
                                                                                ֻ����Main��Div Ant,RFIC������RxCh��Ҫ��дʹ�õ�ͨ���ţ����֧��2�ճ�����RxCh��ʾʹ�õ�Rxͨ����;
                                                                                4R ��2R����ͬһ��NV */


    VOS_UINT32                          bitRfPortCrossFlag          :1;         /*�������ߵ��ź����ӵ��� RFIC DRX Port�ڣ������Ҫ��CTUͨ�����浽������ֻ֧��2�յĽ���*/
    VOS_UINT32                          bitMainAntRxPortSel         :4;         /* ����LNA��RFIC�˿�ѡ�񣬼��ź�ͨ���ĸ��˿����뵽RFIC */
    VOS_UINT32                          bitDivAntRxPortSel          :4;         /* �ּ�LNA��RFIC�˿�ѡ�� */
    VOS_UINT32                          bitRsv0                     :3;         /* Ԥ��λ */
    VOS_UINT32                          bitRsv1                     :4;
    VOS_UINT32                          bitRsv2                     :4;
    VOS_UINT32                          bitRsv3                     :4;
    VOS_UINT32                          bitRsv4                     :4;

} UCOM_NV_GUC_BAND_DL_RFIC_PARA_STRU;

/*****************************************************************************
 �ṹ��    : NV_BAND_UL_RFIC_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����RFIC����,����RFIC ID,ͨ���ţ������PORT��
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitRficIdx                 :2;          /* ����ͨ�����ڵ�RFIC ID,֧���շ����� */
    VOS_UINT32                          bitRficTxCh                :2;          /* ����ͨ���ţ�����MIMO�̶�ʹ��Tx1Ch,��˵���������MIMO�󣬲�ȡ��ͨ������������£���дTx1Iq1,Tx2Iq������2Txת1Tx����NV�����Ժ�Rxһ��*/
    VOS_UINT32                          bitMainAntRficTxPortSel    :8;          /* ����PORT��ѡ�� */
    VOS_UINT32                          bitMainAntRficTxMrxPortSel :4;           /*���л���ͨ����Mrxͨ����ѡ��0: MRX1 RF input selected  1: MRX2 RF input selected*/
    VOS_UINT32                          bitRsv0                    :4;
    VOS_UINT32                          bitRsv1                    :4;
    VOS_UINT32                          bitRsv2                    :4;
    VOS_UINT32                          bitRsv3                    :4;

}UCOM_NV_GUC_BAND_UL_RFIC_PARA_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_BAND_DL_PATH_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ÿ��PATH������RFICͨ������
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitRxAntNum                 :3;         /* ���������� */
    VOS_UINT32                          bitRsv0                     :5;
    VOS_UINT32                          bitRsv1                     :8;
    VOS_UINT32                          bitRsv2                     :8;
    VOS_UINT32                          bitRsv3                     :8;

    UCOM_NV_GUC_BAND_DL_RFIC_PARA_STRU  stBandDlRficPara;                       /* ����RFIC���� */

} UCOM_NV_GUC_BAND_DL_PATH_PARA_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_BAND_UL_PATH_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ÿ��PATH������RFICͨ������
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitAptPdmPinSel             :3;
    VOS_UINT32                          bitGsmRampSel               :3;
    VOS_UINT32                          bitGmskDataSel              :3;
    VOS_UINT32                          bitRsv0                     :3;
    VOS_UINT32                          bitRsv1                     :4;
    VOS_UINT32                          bitRsv2                     :8;
    VOS_UINT32                          bitRsv3                     :8;

    UCOM_NV_GUC_BAND_UL_RFIC_PARA_STRU  stBandUlRficPara;                       /* RFIC���� */
}UCOM_NV_GUC_BAND_UL_PATH_PARA_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_BAND_DL_GPIO_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ǰ������GPIO���ã��������߿��أ�TUNER�ȣ�֧�����߿��غ�TUNER��������
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulMainRxAntSel;                         /*ASMͨ·�ϵ��л�����,�����߿��ػ��ȥ*/
    VOS_UINT32                              ulMainRxAntSelExt;                      /*ASMͨ·�ϵ��л�����,�����߿��ػ��ȥ*/
    VOS_UINT32                              ulDivRxAntSel;
    VOS_UINT32                              ulDivRxAntSelExt;
    VOS_UINT32                              ulMainRxTunerSel;                       /*ASMͨ·�ϵ��л�����*/
    VOS_UINT32                              ulMainRxTunerSelExt;
    VOS_UINT32                              ulDivRxTunerSel;
    VOS_UINT32                              ulDivRxTunerSelExt;
    VOS_UINT32                              ulRsv0;                             /* Ԥ�� */
    VOS_UINT32                              ulRsv1;
    VOS_UINT32                              ulRsv2;
    VOS_UINT32                              ulRsv3;
    VOS_UINT32                              ulRsv4;
    VOS_UINT32                              ulRsv5;
    VOS_UINT32                              ulRsv6;
    VOS_UINT32                              ulRsv7;
} UCOM_NV_GUC_BAND_DL_GPIO_PARA_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_BAND_DL_MIPI_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ǰ������MIPI���ã��������߿��أ�TUNER�ȣ�֧�����߿��غ�TUNER��������
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU              astMipiMainAntRxSel[RX_ANT_SEL_MIPI_NUM];            /*Ant Mipi num=4,��Ҫ��Ƶ����*/
    PHY_MIPIDEV_UNIT_STRU              astMipiDivAntRxSel[RX_ANT_SEL_MIPI_NUM ];
    PHY_MIPIDEV_UNIT_STRU              astMipiMainTunerRxSel[ANT_TUNER_MIPI_NUM ];
    PHY_MIPIDEV_UNIT_STRU              astMipiDivTunerRxSel[ANT_TUNER_MIPI_NUM ];           /*Tuner Mipi num=2,��Ҫ��Ƶ����*/
    PHY_MIPIDEV_UNIT_STRU              astMipiMainAntRxCloseSel[RX_ANT_SEL_MIPI_NUM];       /* ����NV */
    PHY_MIPIDEV_UNIT_STRU              astMipiDivAntRxCloseSel[RX_ANT_SEL_MIPI_NUM];        /* �����ּ��ر�NV */
    PHY_MIPIDEV_UNIT_STRU              astMipiMRsv1[RX_ANT_SEL_MIPI_NUM];
} UCOM_NV_GUC_BAND_DL_MIPI_PARA_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_BAND_DL_FEM_PATH_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ÿ��PATH��ǰ������MIPI���ã��������߿��أ�TUNER�ȣ�
             ֧�����߿��غ�TUNER��������
*****************************************************************************/
typedef struct
{
    UCOM_NV_GUC_BAND_DL_GPIO_PARA_STRU           stBandDlGpioPara;              /* ÿ��PATH����GPIO���� */
    UCOM_NV_GUC_BAND_DL_MIPI_PARA_STRU           stBandDlMipiPara;              /* ÿ��PATH����MIPI���� */
}UCOM_NV_GUC_BAND_DL_FEM_PATH_PARA_STRU;

typedef  UCOM_NV_GUC_BAND_DL_FEM_PATH_PARA_STRU      UCOM_NV_GSM_RF_DL_FEM_PATH_STRU;
typedef  UCOM_NV_GUC_BAND_DL_FEM_PATH_PARA_STRU      UCOM_NV_CDMA_RF_DL_FEM_PATH_STRU;
typedef  UCOM_NV_GUC_BAND_DL_FEM_PATH_PARA_STRU      UCOM_NV_W_DL_FEM_PATH_PARA_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_BAND_UL_GPIO_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����GPIO���ã��������߿��أ�TUNER�ȣ�֧�����߿��غ�TUNER��������
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulTxAntSel;                             /* �������߿��� */
    VOS_UINT32                              ulTxAntSelExt;
    VOS_UINT32                              ulTxTuner;                              /* ����Tuner���� */
    VOS_UINT32                              ulTxTunerExt;
    VOS_UINT32                              ulPaEnCtrl;                             /* PA�߿ؿ��� */
    VOS_UINT32                              ulPaHighModeCtrl;                       /* PA����������� */
    VOS_UINT32                              ulPaMidModeCtrl;                        /* PA����������� */
    VOS_UINT32                              ulPaLowModeCtrl;                        /* PA����������� */
    VOS_UINT32                              ulPaUltraLowModeCtrl;                   /* utralow */
    VOS_UINT32                              ulRsv;
} UCOM_NV_GUC_BAND_UL_GPIO_PARA_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_BAND_UL_MIPI_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����MIPI���ã��������߿��أ�TUNER�ȣ�֧�����߿��غ�TUNER��������
             ����Boston��Ҫ�Ե�λ�����ֽ���ת��
             ���Ըýṹ�������astMipiPaHighMode֮ǰ���ӳ�Ա��Ҫ���¼���ƫ��
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU               astMipiAntSelTx[TX_ANT_SEL_MIPI_NUM];   /* MIPI�������߿���  TX_ANT_SEL_MIPI_NUM =4 */
    PHY_MIPIDEV_UNIT_STRU               astMipiTunerTx[ANT_TUNER_MIPI_NUM];     /* MIPI����TUNER���� Tx Tuner is only for Gsm,num=2 */

    PHY_MIPIDEV_UNIT_STRU               astMipiPaOnCtrl[PA_MODE_MIPI_NUM];      /* MIPI PA On���� */
    PHY_MIPIDEV_UNIT_STRU               astMipiPaOffCtrl[PA_MODE_MIPI_NUM];     /* MIPI PA OFF���� */
    PHY_MIPIDEV_UNIT_STRU               astMipiPaHighMode[PA_MODE_MIPI_NUM];    /* MIPI PA����������� PA_MODE_MIPI_NUM=4 */
    PHY_MIPIDEV_UNIT_STRU               astMipiPaMidMode[PA_MODE_MIPI_NUM];     /* MIPI PA����������� PA_MODE_MIPI_NUM=4 */
    PHY_MIPIDEV_UNIT_STRU               astMipiPaLowMode[PA_MODE_MIPI_NUM];     /* MIPI PA����������� PA_MODE_MIPI_NUM=4 */
    PHY_MIPIDEV_UNIT_STRU               astMipiPaUltraLowMode[PA_MODE_MIPI_NUM];
    PHY_MIPIDEV_UNIT_STRU               astMipiAntSelTxClose[TX_ANT_SEL_MIPI_NUM];
    PHY_MIPIDEV_UNIT_STRU               stMipiPaTrigger;                        /* GSM�� */
} UCOM_NV_GUC_BAND_UL_MIPI_PARA_STRU;

/*****************************************************************************
 �ṹ��    : NV_BAND_EXT_LNA_PATH_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ����LNA��������������ṹ��Ϣ
*****************************************************************************/
typedef struct
{
    NV_EXT_LNA_ANTINFO_STRU                    stMainAntLnaInfo;
    NV_EXT_LNA_ANTINFO_STRU                    stDivAntLnaInfo;
    PHY_MIPIDEV_UNIT_STRU                      astRsv[LNA_MIPI_TRIG_NUM];       /* Ԥ�� */

}UCOM_NV_GUC_BAND_EXT_LNA_PATH_PARA_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_RX_AGC_CAL_RESULT_TABLE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : AGCĳһ��PATH��Ӧ���ܵ�У׼�����GUCֻ��һ��PATH������BAND
*****************************************************************************/
typedef struct
{
    NV_RX_AGC_CAL_RESULT_STRU           stMainAntAgcCalResult;                  /* �������߶�Ӧ��AGCУ׼��� */
    NV_RX_AGC_CAL_RESULT_STRU           stDivAntAgcCalResult;                   /* �ּ����߶�Ӧ��AGCУ׼��� */

}UCOM_NV_GUC_RX_AGC_CAL_RESULT_TABLE_STRU;

/*****************************************************************************
 �ṹ��    : NV_GUC_RX_DCOC_CAL_RESULT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ĳһ��PATH��DCOC��У׼���
*****************************************************************************/
typedef struct
{
    NV_RX_DCOC_CAL_RESULT_STRU          stMainAntCalResult;                    /* ������DCOCУ׼��� */
    NV_RX_DCOC_CAL_RESULT_STRU          stDivAntCalResult;                     /* �ּ���DCOCУ׼��� */
}UCOM_NV_GUC_RX_PATH_DCOC_CAL_RESULT_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_RX_AGC_TABLE_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ĳһ��PATH��ÿ��������ѡ���AGC TABLE INDEX������INDEXȥ������Ӧ��
 AGC���л����ޡ�AGC DEFAULT GAIN��AGC��RFIC������
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitMainAntRxAgcTableIdx         :4;     /* �������߶�Ӧ��AGC TABLE INDEX */
    VOS_UINT32                          bitDivAntRxAgcTableIdx          :4;     /* �ּ����߶�Ӧ��AGC TABLE INDEX */
    VOS_UINT32                          bitRsv0                         :4;
    VOS_UINT32                          bitRsv1                         :4;
    VOS_UINT32                          bitRsv2                         :8;
    VOS_UINT32                          bitRsv3                         :8;
}UCOM_NV_GUC_RX_AGC_TABLE_INFO_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_RF_BAND_CONFIG_STRU
 �ṹ˵��  : GUC Band Config����
*****************************************************************************/
typedef struct
{
    UCOM_NV_GUC_BAND_DL_PATH_PARA_STRU stBandDlPathPara;                        /* ������� */
    UCOM_NV_GUC_BAND_UL_PATH_PARA_STRU stBandUlPathPara;                        /* ������� */

}UCOM_NV_GUC_RF_BAND_CONFIG_STRU;

typedef UCOM_NV_GUC_RF_BAND_CONFIG_STRU       UCOM_NV_GSM_RF_BAND_CONFIG_STRU;
typedef UCOM_NV_GUC_RF_BAND_CONFIG_STRU       UCOM_NV_CDMA_BAND_CONFIG_STRU;
typedef UCOM_NV_GUC_RF_BAND_CONFIG_STRU       UCOM_NV_W_RF_BAND_CONFIG_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_BAND_UL_FEM_PATH_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ǰ���������ã�ÿ��PATH������
*****************************************************************************/
typedef struct
{
    UCOM_NV_GUC_BAND_UL_GPIO_PARA_STRU           stBandUlGpioPara;              /* ����GPIO���� */
    UCOM_NV_GUC_BAND_UL_MIPI_PARA_STRU           stBandUlMipiPara;              /* ����MIPI���� */
}UCOM_NV_GUC_BAND_UL_FEM_PATH_PARA_STRU;

typedef  UCOM_NV_GUC_BAND_UL_FEM_PATH_PARA_STRU    UCOM_NV_GSM_RF_UL_FEM_PATH_STRU;
typedef  UCOM_NV_GUC_BAND_UL_FEM_PATH_PARA_STRU    UCOM_NV_CDMA_RF_UL_FEM_PATH_STRU;
typedef  UCOM_NV_GUC_BAND_UL_FEM_PATH_PARA_STRU    UCOM_NV_W_UL_FEM_PATH_PARA_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_RF_EXT_LNA_PATH_STRU
 �ṹ˵��  : ����LNA ���� Path �ܽṹ�����ṹ�嶨�壬����Band�ţ�Path��Ϣ
*****************************************************************************/
typedef struct
{
    UCOM_NV_GUC_BAND_EXT_LNA_PATH_PARA_STRU stExtLnaPathPara;
}UCOM_NV_GSM_RF_EXT_LNA_PATH_STRU;

typedef   UCOM_NV_GUC_BAND_EXT_LNA_PATH_PARA_STRU    UCOM_NV_CDMA_RF_EXT_LNA_PATH_STRU;
typedef   UCOM_NV_GUC_BAND_EXT_LNA_PATH_PARA_STRU    UCOM_NV_W_EXT_LNA_PATH_PARA_STRU;

/*****************************************************************************
 �ṹ��    : NV_GUC_RF_RX_CAL_FREQ_LIST_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��ҪУ׼��Ƶ���б�ÿ��BAND���֧��32��Ƶ�㣬��NV����BAND��
*****************************************************************************/
typedef struct
{
    PHY_UINT32                              ulValidCalFreqNum;                      /* ��ҪУ׼��Ƶ����� */
    PHY_UINT32                              aulRxCalFreqList[RF_RX_MAX_FREQ_NUM];   /* RF_RX_FREQ_NUM =32 ,GUTL��λ100KHz,CDMA:1KHz*/

}NV_GUC_RF_RX_CAL_FREQ_LIST_STRU;

/*****************************************************************************
 �ṹ��    : NV_GUC_RF_TX_CAL_FREQ_LIST_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��ҪУ׼��Ƶ���б�ÿ��BAND���֧��32��Ƶ�㣬��NV����BAND��
*****************************************************************************/
typedef struct
{
    PHY_UINT32                          ulValidCalFreqNum;                      /* ��ҪУ׼��Ƶ����� */
    PHY_UINT32                          aulTxCalFreqList[RF_TX_MAX_FREQ_NUM];   /* RF_RX_FREQ_NUM =32 ,GUTL��λ100KHz,CDMA:1KHz*/
}NV_GUC_RF_TX_CAL_FREQ_LIST_STRU;

/*****************************************************************************
 �ṹ��    : NV_RF_TX_CAL_FREQ_LIST_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��ҪУ׼��Ƶ���б�ÿ��BAND���֧��32��Ƶ�㣬��NV����BAND��
*****************************************************************************/
typedef   NV_GUC_RF_TX_CAL_FREQ_LIST_STRU       UCOM_NV_GSM_RF_TX_CAL_FREQ_LIST_STRU;
typedef   NV_GUC_RF_TX_CAL_FREQ_LIST_STRU       UCOM_NV_WCDMA_RF_TX_CAL_FREQ_LIST_STRU;
typedef   NV_GUC_RF_TX_CAL_FREQ_LIST_STRU       UCOM_NV_CDMA_RF_TX_CAL_FREQ_LIST_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_RF_RX_CAL_FREQ_LIST_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��ҪУ׼��Ƶ���б�ÿ��BAND���֧��32��Ƶ�㣬��NV����BAND��
*****************************************************************************/
typedef   NV_GUC_RF_RX_CAL_FREQ_LIST_STRU       UCOM_NV_GSM_RF_RX_CAL_FREQ_LIST_STRU;
typedef   NV_GUC_RF_RX_CAL_FREQ_LIST_STRU       UCOM_NV_CDMA_RF_RX_CAL_FREQ_LIST_STRU;
typedef   NV_GUC_RF_RX_CAL_FREQ_LIST_STRU       UCOM_NV_WCDMA_RF_RX_CAL_FREQ_LIST_STRU;


/*****************************************************************************
 �ṹ��    : NV_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU
 �ṹ˵��  : GSM/WCDMA/CDMA ��AGC�����л�����
*****************************************************************************/
typedef   NV_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU     UCOM_NV_GSM_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU;
typedef   NV_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU     UCOM_NV_CDMA_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU;
typedef   NV_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU     UCOM_NV_W_ALG_RX_AGC_THRESHOLD_TABLE_BANK_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_ALG_RX_FAST_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU
 �ṹ˵��  : GSM��AGC�����л�����
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          auhwBlockInitAgc[AGC_MAX_TABLE_NUM];    /* ���ٲ�����ʼ��λ */
    VOS_UINT16                          auhwNoBlockInitAgc[AGC_MAX_TABLE_NUM];  /* ���ٲ�����ʼ��λ */
    NV_RX_AGC_GAIN_THRESHOLD_STRU       astAgcSwitchTable[AGC_MAX_TABLE_NUM];   /*AGC_MAX_TABLE = 10*/

}UCOM_NV_GSM_ALG_RX_AGC_FAST_SWITCH_THRESHOLD_TABLE_BANK_STRU;

/*****************************************************************************
 �ṹ��    : NV_GUC_RF_RX_AGC_TABLE_USED_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : AGC��TABLE INDEX�����֧��6��PATH��GUCֻ��Ҫʹ��һ��PATH������BAND
*****************************************************************************/
typedef     UCOM_NV_GUC_RX_AGC_TABLE_INFO_STRU      UCOM_NV_GSM_RF_RX_AGC_TABLE_USED_INFO_STRU;
typedef     UCOM_NV_GUC_RX_AGC_TABLE_INFO_STRU      UCOM_NV_CDMA_RF_RX_AGC_TABLE_USED_INFO_STRU;
typedef     UCOM_NV_GUC_RX_AGC_TABLE_INFO_STRU      UCOM_NV_W_RF_RX_AGC_TABLE_USED_INFO_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_RX_AGC_CAL_RESULT_TABLE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : AGC��У׼���ֵ����������ֵ��Ƶ��ֵ
*****************************************************************************/
typedef UCOM_NV_GUC_RX_AGC_CAL_RESULT_TABLE_STRU   UCOM_NV_GSM_RX_AGC_CAL_RESULT_TABLE_STRU;
typedef UCOM_NV_GUC_RX_AGC_CAL_RESULT_TABLE_STRU   UCOM_NV_CDMA_RX_AGC_CAL_RESULT_TABLE_STRU;
typedef UCOM_NV_GUC_RX_AGC_CAL_RESULT_TABLE_STRU   UCOM_NV_W_RX_AGC_CAL_RESULT_TABLE_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_RX_PATH_DCOC_CAL_RESULT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : DCOC��У׼���ֵ,����У׼���ֺ�BBP�ϱ��Ĳ���ֱ��ֵ
*****************************************************************************/
typedef     UCOM_NV_GUC_RX_PATH_DCOC_CAL_RESULT_STRU     UCOM_NV_GSM_RX_PATH_DCOC_CAL_RESULT_STRU;
typedef     UCOM_NV_GUC_RX_PATH_DCOC_CAL_RESULT_STRU     UCOM_NV_CDMA_RX_PATH_DCOC_CAL_RESULT_STRU;
typedef     UCOM_NV_GUC_RX_PATH_DCOC_CAL_RESULT_STRU     UCOM_NV_W_RX_PATH_DCOC_CAL_RESULT_STRU;

typedef     NV_RX_DCOC_CAL_RESULT_STRU                   NV_GUC_RX_DCOC_CAL_RESULT_STRU;

typedef     NV_RX_DCOC_COMP_VAlUE_STRU                   NV_GUC_RX_DCOC_COMP_VAlUE_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_RX_PATH_IP2_CAL_RESULT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : IP2��У׼���ֵ
*****************************************************************************/
typedef NV_RX_PATH_IP2_CAL_RESULT_STRU      UCOM_NV_GSM_RX_PATH_IP2_CAL_RESULT_STRU;
typedef NV_RX_PATH_IP2_CAL_RESULT_STRU      UCOM_NV_CDMA_RX_PATH_IP2_CAL_RESULT_STRU;
typedef NV_RX_PATH_IP2_CAL_RESULT_STRU      UCOM_NV_W_RX_PATH_IP2_CAL_RESULT_STRU;

typedef NV_RX_IP2_CAL_RESULT_STRU           UCOM_NV_W_RX_IP2_CAL_RESULT_STRU;

/*****************************************************************************
 ö����    : UCOM_NV_W_RF_RX_AGC_RFIC_TABLE_BANK_STRU
 Э����  :
 ö��˵��  : AGC��λ�л�������,boston��֮��ʹ��(��ΪRF�仯)
*****************************************************************************/
typedef NV_RF_RX_AGC_RFIC_TABLE_BANK_STRU   UCOM_NV_G_RF_RX_AGC_RFIC_TABLE_BANK_STRU;
typedef NV_RF_RX_AGC_RFIC_TABLE_BANK_STRU   UCOM_NV_CDMA_RF_RX_AGC_RFIC_TABLE_BANK_STRU;
typedef NV_RF_RX_AGC_RFIC_TABLE_BANK_STRU   UCOM_NV_W_RF_RX_AGC_RFIC_TABLE_BANK_STRU;

/*****************************************************************************
 ö����    : NV_GUC_RF_RX_AGC_GAIN_TABLE_BANK_STRU
 Э����  :
 ö��˵��  : AGC GAIN
*****************************************************************************/
typedef NV_RF_RX_AGC_GAIN_TABLE_BANK_STRU   UCOM_NV_G_RF_RX_AGC_GAIN_TABLE_BANK_STRU;
typedef NV_RF_RX_AGC_GAIN_TABLE_BANK_STRU   UCOM_NV_CDMA_RF_RX_AGC_GAIN_TABLE_BANK_STRU;
typedef NV_RF_RX_AGC_GAIN_TABLE_BANK_STRU   UCOM_NV_W_RF_RX_AGC_GAIN_TABLE_BANK_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_CDMA_RF_RX_RFFE_ILOSS_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ǰ�˵Ĳ���ֵ��CDMAֻ��1��PATH��ÿ��PATH���֧��˫�գ�����Band��
*****************************************************************************/
typedef NV_RX_PATH_ILOSS_STRU       UCOM_NV_CDMA_RF_RX_RFFE_ILOSS_STRU;
typedef NV_RX_PATH_ILOSS_STRU       UCOM_NV_W_RF_RX_RFFE_ILOSS_STRU;


typedef NV_RX_AGC_GAIN_DEFAULT_STRU  NV_GUC_RX_AGC_GAIN_DEFAULT_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_RF_HW_BASIC_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : RFIC��ABB���ӹ�ϵNV�����RFIC������RFIC��ABB����ͨ����
*****************************************************************************/
typedef  NV_MODEM_RF_HW_BASIC_INFO_STRU    UCOM_NV_GUC_RF_HW_BASIC_INFO_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_TAS_DPDT_MIPI_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :CDMA TAS MIPI �ṹ��
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU                    astTasDpdtThrough[UCOM_NV_CDMA_TAS_MIPI_NUM];    /* ֱͨ */
    PHY_MIPIDEV_UNIT_STRU                    astTasDpdtSwap[UCOM_NV_CDMA_TAS_MIPI_NUM];       /* ���� */
}UCOM_NV_GUC_TAS_DPDT_MIPI_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_W_BAND_BIT_MASK_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : BAND�Ƿ�֧��
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwBandMask;                             /* bit0Ϊ1��ʾ֧��band1, ���� */
    VOS_UINT32                          uwBandMaskEx;
}UCOM_NV_W_BAND_BIT_MASK_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_TX_FEATURE_DEFINE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : �������Կ���
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwBand      : 6;                        /* WCDMA:1 - 63��ʾband1-63,Ĭ��ֵ0,
                                                                                   CDMA: 0 - 10��ʾbc0 - bc10
                                                                                   ���ж������band��д��ͬ�����һ����Ч */
    VOS_UINT32                          uwAptCtrl   : 2;                        /* APT����,0:�أ�1:����Ĭ��ֵ0 */
    VOS_UINT32                          uwDpdCtrl   : 3;                        /* DPD����,Ĭ��ֵ0
                                                                                   0:DPD�أ�
                                                                                   1:APT+DPD��DPD��APT����ȡ
                                                                                   2:ET+DPD��DPD��ET����ȡ */
    VOS_UINT32                          uwEtCtrl    : 3;                        /* ET���Կ���,Ĭ��ֵ0
                                                                                   0:ET�أ�
                                                                                   1:���һ��ΪET��
                                                                                   2:��߶���ΪET����ET���������ֵ�λ��ȡ��ߵ�(H)������
                                                                                   3:��߶���ΪET����ET�������ֵ�λ���� */
    VOS_UINT32                          uwRslv0     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv1     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv2     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv3     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv4     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv5     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv6     : 6;                        /* ���� */

}UCOM_NV_TX_FEATURE_DEFINE_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_W_FE_TX_FEATURE_CTRL_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : BAND�������Կ���
*****************************************************************************/
typedef struct
{
    UCOM_NV_TX_FEATURE_DEFINE_STRU      astTxFeature[16];                           /* ���Կ��أ����ж������band��д��ͬ�����һ����Ч */
}UCOM_NV_W_FE_TX_FEATURE_CTRL_STRU;


typedef UCOM_NV_W_FE_TX_FEATURE_CTRL_STRU     UCOM_NV_CDMA_FE_TX_FEATURE_CTRL_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_TX_FEATURE_DEFINE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : �������Կ���
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwBand      : 6;                        /* WCDMA:1 - 63��ʾband1-63,Ĭ��ֵ0,
                                                                                   CDMA: 0 - 10��ʾbc0 - bc10
                                                                                   ���ж������band��д��ͬ�����һ����Ч */

    VOS_UINT32                          uwRslv0     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv1     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv2     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv3     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv4     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv5     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv6     : 2;                        /* ���� */
    VOS_UINT32                          uwRslv7     : 2;                        /* ���� */

    VOS_UINT32                          uwRslv8     : 10;                       /* ���� */

}UCOM_NV_RX_FEATURE_DEFINE_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_W_FE_RX_FEATURE_CTRL_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : BAND�Ƿ�֧������
*****************************************************************************/
typedef struct
{
    UCOM_NV_RX_FEATURE_DEFINE_STRU      astRxFeature[16];

}UCOM_NV_W_FE_RX_FEATURE_CTRL_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_W_BBP_TX_TIMING_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : W����ʱ����
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          uhwTxChanDelay;                         /* ���е�����ʱ���� */
    VOS_UINT16                          uhwTxInitDelay;                         /* ��ǰ������RF��chip����ȡֵ��Χ0~1023������ֵ800us����3072chip */
    VOS_UINT16                          uhwTxOffDelay;                          /* �Ӻ�ر�����RF��chip���� */
    VOS_UINT16                          uhwTxIdleOnDelay;                       /* ��ǰ������RF IDLE״̬��chip����ȡֵ��Χ0~1023������ֵ100us����384chip */
    VOS_UINT16                          uhwTxIdleOffDelay;                      /* �Ӻ�ر�����RF IDLE״̬��chip���� */
    VOS_UINT16                          uhwTxAbbInitDelay;                      /* ����δʹ�� */
    VOS_UINT16                          uhwTxAbbOffDelay;                       /* ����δʹ�� */
    VOS_UINT16                          shwTxGainOpenTime;                      /* ���з��书����Чʱ�䣬��λchip����ʾ���з��书������ǰ����ʱ϶ͷ���ʱ��������Ƶ������
                                                                                   ������Ƶ��������ʱ��Ϊʹ���з��书��������ʱ϶ͷ����Ч������ǰ�����з��书�ʼ�����������Ƶ���� */
    VOS_UINT16                          uhwEtDataPathIntDelay;                  /* ET����ͨ·������ʱ,Ĭ��138 */
    VOS_UINT16                          uhwTxPaStableTime;                      /* ���书�ʱ仯��RF��PA���ȶ�ʱ�䣬����MRXͨ�����ú���ȶ�ʱ�䣨MRX��������ʱ϶ͷλ�ÿ���������λchip */
    VOS_UINT16                          uhwMrxAbbStableTime;                    /* MRX ABB�߿ؿ����󣬵ȴ�uhwMrxAbbStableTime��������MRX���ʹ��ƣ���λchip */
    VOS_UINT16                          uhwFachTxDataValidOpenTime;             /* FACH̬��TX���ݷ��ʹ�λ�ã���Ȼ����tx_abb_open_time���Դ˱�֤ABB��λ�ڼ�������vld�������λ���㣬Ĭ��0 */
    VOS_UINT16                          uhwFachTxDataValidCloseTime;            /* FACH̬��TX���ݷ��͹ر�λ�ã�������tx_abb_close_time����λ���㣬Ĭ��0 */
    VOS_UINT16                          uhwDchTxDataValidOpenTime;              /* DCH̬��TX���ݷ��ʹ�λ�ã���Ȼ����tx_abb_open_time���Դ˱�֤ABB��λ�ڼ�������vld�������λ���㣬Ĭ��0 */
    VOS_UINT16                          uhwDchTxDataValidCloseTime;             /* DCH̬��TX���ݷ��͹ر�λ�ã�������tx_abb_close_time����λ���㣬Ĭ��0 */
    VOS_UINT16                          uhwReserved0;                           /* ���� */

    VOS_UINT16                          uhwReserved1;                           /* ���� */
    VOS_UINT16                          uhwReserved2;                           /* ���� */
    VOS_UINT16                          uhwReserved3;
    VOS_UINT16                          uhwReserved4;
    VOS_UINT16                          uhwReserved5;
    VOS_UINT16                          uhwReserved6;
    VOS_UINT16                          uhwReserved7;
    VOS_UINT16                          uhwReserved8;
    VOS_UINT16                          uhwReserved9;
    VOS_UINT16                          uhwReserved10;

}UCOM_NV_W_BBP_TX_TIMING_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_W_TX_BAND_TIMING_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : W����ʱ����
*****************************************************************************/
typedef struct
{
    UCOM_NV_W_PA_GAIN_SWITH_TIMING_STRU     stPaGainSwichTiming;                /* PA�����л�ʱ�� */
    UCOM_NV_W_MIPI_APT_TIMING_CFG_STRU      stMipiAptTimingInfo;
    VOS_UINT16                              uhwPaOnAdvanceChip;
    VOS_UINT16                              uhwPaOffDelayChip;
    VOS_UINT16                              uhwTxPhaseCompTiming;
    VOS_UINT16                              uhwReserved0;
    VOS_UINT16                              uhwReserved1;
    VOS_UINT16                              uhwReserved2;
    VOS_UINT16                              uhwReserved3;
    VOS_UINT16                              uhwReserved4;
    VOS_UINT16                              uhwReserved5;
    VOS_UINT16                              uhwReserved6;
    VOS_UINT16                              uhwReserved7;
    VOS_UINT16                              uhwReserved8;
}UCOM_NV_W_TX_BAND_TIMING_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_CDMA_TX_BAND_TIMING_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CDMA����ʱ����
*****************************************************************************/
typedef struct
{
    UCOM_NV_CDMA_PA_GAIN_SWITCH_TIMING_STRU stPaGainSwichTiming;                /* PA�����л�ʱ�� */

    VOS_UINT16                              uhwPaOnAdvanceChip;
    VOS_UINT16                              uhwPaOffDelayChip;
    VOS_UINT16                              uhwAptPaVccTiming;                  /* vcc bias has the same timing */
    VOS_UINT16                              uhwAptTrig1Timing;
    VOS_UINT16                              uhwAptTrig2Timing;

    VOS_UINT16                              uhwEtMOnTiming;                     /* ��ǰ��ETM��chip��, ETM��Ҫ��PA֮ǰ�� */
    VOS_UINT16                              uhwEtMOffTiming;                    /* �Ӻ�ر�ETM��chip��, ETM��Ҫ��PA֮��ر� */
    VOS_UINT16                              uhwEtMGainSwichTiming;              /* �л�ETM��λ��chip��, ETM��Ҫ��PA��λ֮ǰ���� */

    VOS_UINT16                              uhwReserved0;
    VOS_UINT16                              uhwReserved1;
    VOS_UINT16                              uhwReserved2;
    VOS_UINT16                              uhwReserved3;
    VOS_UINT16                              uhwReserved4;
    VOS_UINT16                              uhwReserved5;
    VOS_UINT16                              uhwReserved6;
    VOS_UINT16                              uhwReserved7;

}UCOM_NV_CDMA_TX_BAND_TIMING_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_W_UL_CFR_GATE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              uwDefaultCfrFirstGate;
    VOS_UINT32                              uwDefaultCfrSecondGate;
    VOS_UINT32                              uwDpdCfrFirstGate;
    VOS_UINT32                              uwDpdCfrSecondGate;
}UCOM_NV_W_UL_CFR_GATE_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_WCDMA_TX_MAXPOWER_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    VOS_INT16                                   shwTxMaxPowerRach;                             /* ����Ƶ�Σ����н�����DPCH�����RACHʱUE�����������书�ʣ���λ0.1dbm;��0��ΪRACH,��1��ΪDPCH, */
    VOS_INT16                                   shwTxMaxPowerDpch;                             /* ����Ƶ�Σ����н�����DPCH�����RACHʱUE�����������书�ʣ���λ0.1dbm;��0��ΪRACH,��1��ΪDPCH, */
    VOS_INT16                                   ashwTxMaxPowerTempComp[UCOM_NV_W_TEMP_NUM];    /* ����书�ʲ������棬��λ0.1dbm;3��Ƶ�Σ�5��Ԥ�ȶ�����¶ȵ�:-20,0,20,40,60,��6�������ڶ��룬��ʹ�� */
}UCOM_NV_WCDMA_TX_MAXPOWER_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_WCDMA_TX_MAXPOWER_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    VOS_INT16                                   shwTxMinPower;                                  /* UE�������С���书�ʣ���λ0.1dbm;3��Ƶ�Σ�5��Ԥ�ȶ�����¶ȵ�:-20,0,20,40,60,��6�������ڶ��룬��ʹ�� */
    VOS_INT16                                   shwRslv;                                        /* ���� */
    VOS_INT16                                   ashwTxMinPowerTempComp[UCOM_NV_W_TEMP_NUM];     /* ��С���书���¶Ȳ��� */
}UCOM_NV_WCDMA_TX_MINPOWER_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_W_TX_POWER_STANDARD_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    UCOM_NV_WCDMA_TX_MAXPOWER_STRU              stTxWiredMaxPower;              /* ���� */
    UCOM_NV_WCDMA_TX_MAXPOWER_STRU              stTxWirelessMaxPower;           /* ���� */
    UCOM_NV_WCDMA_TX_MINPOWER_STRU              stTxMinPower;
}UCOM_NV_W_TX_POWER_STANDARD_STRU;


/*****************************************************************************
 �ṹ��    : NV_GUC_BAND_UL_DCDC_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : DCDC���Ʋ���
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwDcdcType              :2;             /* 0: PDM, 1:MIPI */
    VOS_UINT32                          uwDcdcPinCtrlEn         :2;             /* �Ƿ�ʹ��DCDC GPIO PIN������ */
    VOS_UINT32                          uwDcdcPinNum            :8;             /* ��Ӧ��PIN�� 0 - 31, FFΪ��Ч */
    VOS_UINT32                          uwDcdcDefaultPdmValue   :8;             /* ��ʹ��APTʱ��PDM��ʽʱ��Ĭ������ֵ��
                                                                                   ʹ�������APT��(�ϲ㲻������ֵ��ȡ��������ʶ�Ӧ��ֵ) */
    VOS_UINT32                          uwRsv0                  :4;             /* ���� */
    VOS_UINT32                          uwRsv1                  :8;             /* ���� */

    PHY_MIPIDEV_UNIT_STRU               astDcdcDefaultMipiWord[UCOM_NV_W_TX_DCDC_MIPI_NUM];              /* ��ʹ��APTʱ��MIPI��ʽʱ��Ĭ������ֵ��
                                                                                   ʹ�������APT��(�ϲ㲻������ֵ��ȡ��������ʶ�Ӧ��ֵ) */

    VOS_UINT32                          uwRsv2;                                 /* ���� */
    VOS_UINT32                          uwRsv3;                                 /* ���� */
    VOS_UINT32                          uwRsv4;                                 /* ���� */
} NV_W_UL_DCDC_PARA_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_W_UL_MIPI_VBIAS_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_CMD_STRU                astVbiasH2ULGain[UCOM_NV_PA_GAIN_MAX_NUM];
}UCOM_NV_W_UL_MIPI_VBIAS_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_W_ET_LUT_EXTRACT_ETM_CFG_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ET LUT��ȡ����ETM��MIPI����
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU               stPaVccCmd;                             /* ETM DAC1���� */
    PHY_MIPIDEV_UNIT_STRU               stPaVbaisCmd;                           /* ETM DAC2���� */
    PHY_MIPIDEV_UNIT_STRU               astEtLutEtmMipi[4];                     /* ETM MIPI���䣬ֻ��ET LUT��ȡʱ���ã�
                                                                                   ���ڿ���DAC1/DAC2�ֱ����  */
}UCOM_NV_W_ET_LUT_EXTRACT_ETM_CFG_STRU;

typedef UCOM_NV_W_ET_LUT_EXTRACT_ETM_CFG_STRU     UCOM_NV_CDMA_ET_LUT_EXTRACT_ETM_CFG_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_W_UL_DCDC_ET_APT_CFG_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :
*****************************************************************************/
typedef struct
{
    NV_W_UL_DCDC_PARA_STRU                  stDcdcPara; /* DCDCĬ������,��ET��APTģʽ��ʹ�� */

    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptOn[UCOM_NV_ET_MIPI_NUM];    /* MIPI ET/APT On���� */
    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptOff[UCOM_NV_ET_MIPI_NUM];   /* MIPI ET/APT OFF���� */

    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptH[UCOM_NV_ET_MIPI_NUM];     /* MIPI ET/APT����������� */
    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptM[UCOM_NV_ET_MIPI_NUM];     /* MIPI ET/APT����������� */
    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptL[UCOM_NV_ET_MIPI_NUM];     /* MIPI ET/APT����������� */
    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptUL[UCOM_NV_ET_MIPI_NUM];    /* MIPI ET/APT������������� */

    PHY_MIPIDEV_CMD_STRU                    astAptPaVccH2ULGain[UCOM_NV_PA_GAIN_MAX_NUM];/* ���⵵λ��д��MIPI,���ʾ�ù�����Ч��
                                                                                            ÿʱ϶ͷ���ͣ���ETģʽ�²��ᷢ�� */
    UCOM_NV_W_UL_MIPI_VBIAS_STRU            astMipiAptPaVbias[UCOM_NV_W_TX_PA_VBIAS_NUM];/* ���⵵λ��д��MIPI,���ʾ�ù�����Ч��
                                                                                            ÿʱ϶ͷ���ͣ�ETģʽ��Ҳ�ᷢ�� */
    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptTrigger[UCOM_NV_ET_APT_TRIGGER_NUM];/* ��д��MIPI,���ʾ�ù�����Ч��
                                                                                               ÿʱ϶ͷ���ͣ�ETģʽ��Ҳ�ᷢ�� */

    UCOM_NV_W_ET_LUT_EXTRACT_ETM_CFG_STRU   stEtLutExtMipiCfg;                      /* ET LUT��ȡ����ETM��MIPI������
                                                                                       ��Ϊtriggerģʽ����Ҫ����triggerָ�� */

}UCOM_NV_W_UL_DCDC_ET_APT_CFG_STRU;

typedef UCOM_NV_W_UL_DCDC_ET_APT_CFG_STRU     UCOM_NV_CDMA_UL_DCDC_ET_APT_CFG_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_WCDMA_RX_AGC_FREQ_COMP_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��ҪУ׼��WCDMA��Ƶ��Ƶ��,boston��֮��ʹ��(��ΪRF�仯)
*****************************************************************************/
typedef NV_RX_AGC_FREQ_COMP_CAL_STRU UCOM_NV_WCDMA_RX_AGC_FREQ_COMP_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_W_CLOSE_LOOP_TUNER_PARA_STRU
 �ṹ˵��  : �ջ�tuner�����ṹ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              uwBandSupport;                      /* ��ӦbitΪ1��ʾ���band֧�ֶ�̬���ߵ�г,����֧�� */
    VOS_UINT32                              uwBandExten;                        /* �Ժ�band ID���ܳ���32 */
    VOS_UINT16                              uhwActiveTime;                      /* ���ߵ�г������ǰ֡ͷ�������������д򿪺�WBBPÿ����֡ͷ��ǰcpu_tx_sw_active_time�������ã���λΪchip */
    VOS_UINT16                              uhwSampleTime;                      /* ���ߵ�гƽ��������,Ĭ������Ϊ4 */
    VOS_UINT16                              uhwDelayFrame;                      /* ���һ�ֵ�г��ĵȴ�ʱ�䣬��λ֡ */
    VOS_UINT16                              uhwEchoLossThreshold;               /* ���ߵ�г�ز��������,����0.1dBm */
    VOS_UINT32                              uwForWardGpioWord;                  /* פ�����ǰ������ */
    VOS_UINT32                              uwReverseGpioWord;                  /* פ����ⷴ������ */
    PHY_MIPIDEV_CMD_STRU                    stTunerMipiCmd;                     /* MIPI������ */
    UCOM_NV_W_ANT_TUNER_CODE_STRU           stAntTunerCode;                     /* ���ߵ�г������Ϣ */
} UCOM_NV_W_CLOSE_LOOP_TUNER_PARA_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_ANT_TUNER_CTRL_INFO_STRU
 �ṹ˵��  : ���ߵ�г�Ŀ��ƽṹ��,MIPI��tuner,���Ǽ��ݶ����ͺţ��ṩ���6��������
*****************************************************************************/
typedef struct
{
    UCOM_NV_TUNER_SUPPORT_MODE_ENUM_UINT16  enAntTunerMode;                     /* 0:Ӳ����֧�֣�1:֧�ֿ�����2:֧��AP+Sensor, 3:֧�ֶ�̬��г */
    UCOM_NV_RFFE_CTRL_ENUM_UINT16           enAntTunerCtrlType;                 /* ���ߵ�г��ͨ��MIPI���ƻ���GPIO,0��ʾGPIO,1��ʾMIPI */
    PHY_MIPIDEV_UNIT_STRU                   stTunerActiveReg;                   /* tuner�Ĺ�����ʼ�������֣�����һЩ�Ĵ���ֻ��Ҫģʽ��������һ�� */
    PHY_MIPIDEV_UNIT_STRU                   stTunerIdleReg;                     /* ����tunerΪIDLE̬��͹���̬�Ŀ����� */
    UCOM_NV_W_CLOSE_LOOP_TUNER_PARA_STRU    stCloseLoopTunerCtrl;               /* �ջ�tuner���� */
} UCOM_NV_W_ANT_TUNER_CTRL_INFO_STRU;

typedef UCOM_NV_W_ANT_TUNER_CTRL_INFO_STRU UCOM_NV_CDMA_ANT_TUNER_CTRL_INFO_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_MIPI_INIT_WORD_STRU
 �ṹ˵��  : ��ʼ��mipi�����Ľӿ�
*****************************************************************************/
typedef struct
{
   PHY_MIPIDEV_UNIT_STRU        astTxMipiInitWord[UCOM_NV_GUC_MIPI_INIT_UNIT_MAX_NUM];
}UCOM_NV_GUC_MIPI_INIT_WORD_STRU;
/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_REDUCE_CURRENT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��������
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          uhwReduceCurrentEn;                     /* ��������ʹ�� */
    UCOM_NV_RFFE_CTRL_ENUM_UINT16       enCtrlMode;                             /* ���Ʒ�ʽMIPI��GPIO */
    VOS_UINT32                          uwGpioMask;                             /* ��������ʹ�õĹܽ� */
    PHY_MIPIDEV_UNIT_STRU               stDefaultMipiData;                      /* �ں�����������Ӧ��PMU��ַ��ȱʡʱ�ĵ���ֵ(��һ��ʱ϶), */
    PHY_MIPIDEV_UNIT_STRU               stLimitMipiData;                        /* �ں�����������Ӧ��PMU��ַ��ȱʡʱ�ĵ���ֵ(��һ��ʱ϶) */
}UCOM_NV_GSM_REDUCE_CURRENT_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_MIPI_COMM_STRU
 Э����  :
 �ṹ˵��  : ����һЩmipi���ܹ���ָ���
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU                   astMipiWord[UCOM_NV_MIPI_COMM_NUM];
}UCOM_NV_MIPI_COMM_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_DPD_LUT_ALG_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : DPD��ȡ�㷨����
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwDpdAlgSel             :2;             /* DPD�㷨ѡ��0:��ѧ���Լ���DPD,1:�������Լ���DPD,Ĭ��ֵ:1 */
    VOS_UINT32                          uwCwType                :4;             /* Sine������ѡ��0:1.92M����,1:1.92M�����,
                                                                                   2:0.96M����,3:0.96M�����,4:0.48M����,5:0.48M�����
                                                                                   Ĭ��ֵ:2 */
    VOS_UINT32                          uwDpdSmoothLen          :5;             /* ƽ������,Ĭ��ֵ:9 */
    VOS_UINT32                          uwPaGainCorrectFactor   :14;            /* LUT��һ�����ֵ,Ĭ��ֵ:1036 */
    VOS_UINT32                          uwProtectIndex          :7;             /* LUT��������ֵ,Ĭ��ֵ:10 */

    VOS_UINT32                          uwStepDataLen           :12;            /* һ�׵Ĳ������ȣ�����Ϊһ���������ڵ�����������
                                                                                   Ĭ��ֵ:512 */
    VOS_UINT32                          uwRedundantStepNum      :4;             /* ���ݲ������������������ͻ���,Ĭ��ֵ:1 */
    VOS_INT32                           swStepVBackOff          :8;             /* STEP V����ֵ��Ĭ��ֵ:2 */
    VOS_UINT32                          uwStepVDefault          :8;             /* STEP V��ֵ��Ĭ��ֵ: 90 */

    VOS_UINT32                          uwReserved0             :8;             /* ���� */
    VOS_UINT32                          uwReserved1             :8;             /* ���� */
    VOS_UINT32                          uwReserved2             :8;             /* ���� */
    VOS_UINT32                          uwReserved3             :4;             /* ���� */
    VOS_UINT32                          uwReserved4             :4;             /* ���� */

}UCOM_NV_DPD_LUT_ALG_PARA_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_ET_LUT_ALG_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ET��ȡ�㷨����
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwCwType                :4;             /* 0:1.92M����,1:1.92M�����,
                                                                                   2:0.96M����,3:0.96M�����,4:0.48M����,5:0.48M�����
                                                                                   Ĭ��ֵ:2 */
    VOS_UINT32                          uwStepDataLen           :12;            /* һ�׵Ĳ������ȣ�����Ϊһ���������ڵ�����������
                                                                                   Ĭ��ֵ:512 */
    VOS_UINT32                          uwRedundantStepNum      :3;             /* ���ݲ������������������ͻ���,Ĭ��ֵ:1 */
    VOS_UINT32                          uwProtectIdx            :4;             /* ��������ֵ��ǰ�漸�׿���PAû��ͨ,Ĭ��ֵ:0 */
    VOS_UINT32                          uwIsoPowerOrGain        :1;             /* �ǹ���ѹ����������ѹ�� 0:����ѹ�� 1:����ѹ�� */
    VOS_UINT32                          uwReserved1             :4;             /* ���� */
    VOS_UINT32                          uwReserved2             :4;             /* ���� */
}UCOM_NV_ET_LUT_ALG_PARA_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_CDMA_ET_LUT_ALG_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ET��ȡ�㷨����  �����������Ҫ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32       uwCwType                :4;     /* 0:1.92M����,    1:1.92M�����,
                                                        2:0.96M����,    3:0.96M�����,
                                                        4:0.48M����,    5:0.48M�����
                                                        Ĭ��ֵ:2 */

    VOS_UINT32       uwStepDataLen           :12;    /* һ�׵Ĳ������ȣ�����Ϊһ���������ڵ�����������
                                                                                   Ĭ��ֵ:512 */
    /* �������������Cģ����Ҫ */
    VOS_UINT32       uwRedundantStepNum      :3;     /* ���ݲ������������������ͻ���,Ĭ��ֵ:2,Cģ����Ҫ */

    /* �������������Cģ����Ҫ */
    VOS_UINT32       uwProtectIdx            :4;     /* ��������ֵ��ǰ�漸�׿���PAû��ͨ,Ĭ��ֵ:2,Cģ����Ҫ */

    VOS_UINT32       uwIsoPowerOrGain        :1;     /* ���� */
    VOS_UINT32       uwReserved1             :4;     /* ���� */
    VOS_UINT32       uwReserved2             :4;     /* ���� */
}UCOM_NV_CDMA_ET_LUT_ALG_PARA_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_W_ET_DELAY_CAL_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Et DelayУ׼��Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwCalAlgSel                 : 2;        /* У׼����ѡ��Ĭ��ֵ2:
                                                                                   0:���ּӱ���������ֵ+-uhwSearchRangeOffset�ķ�Χ���֣�
                                                                                     ��uhwSweepRange��Χ����
                                                                                   1:����������ֵ+-uhwSearchRangeOffset�ķ�Χ������ΪuhwSweepStep
                                                                                   2:���������������׵㣬����ֵ+-uhwSearchRangeOffset�ķ�Χ
                                                                                     ƽ��ΪuhwPartNum�� */
    VOS_UINT32                          uwSampleLen2PowerN          : 4;        /* ��������1<<N������Ϊ256����������Ĭ��ֵ10 */
    VOS_UINT32                          uwPartNum                   : 5;        /* ��������ȷָ���������Щ���������,��Чֵ4-31 */
    VOS_INT32                           swEtLutBackoff              : 6;        /* ET LUT�����ˣ�ʵ�ʲ����²���,��λ0.1dB��Ĭ��ֵ0 */
    VOS_UINT32                          uwCenterEnvelopTimeDelay    : 15;       /* delay������ֵ��Ĭ��ֵ27100 */

    VOS_UINT32                          uwSearchRangeOffset         : 11;       /* ������offset��Ĭ��ֵ1000 */
    VOS_UINT32                          uwSweepOffset               : 14;       /* ���������������÷�Χ*2��ʼ������������Ĭ��ֵ200 */
    VOS_UINT32                          uwSweepStep                 : 7;        /* ɨ�貽������uwSweepStep > uwSweepRange�򲻻�ɨ��,Ĭ��ֵ5 */

    VOS_UINT32                          uwAcFreq                    : 16;       /* �ڵ����ֵ�����Ƶ�ʣ���λKHz��Ĭ��ֵ5000 */
    VOS_UINT32                          uwIntegralBW                : 16;       /* ����ACLR���ִ�����λKHz��Ĭ��ֵ3840 */

    VOS_INT32                           swAdcOutdBFS                : 8;        /* ADC���������dBFS����λ0.1��Ĭ��ֵ-80 */
    VOS_UINT32                          uwRslv1                     : 8;        /* ���� */
    VOS_UINT32                          uwRslv2                     : 8;        /* ���� */
    VOS_UINT32                          uwRslv3                     : 8;        /* ���� */

}UCOM_NV_W_ET_DELAY_CAL_PARA_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_CDMA_ET_DELAY_CAL_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Et DelayУ׼����  for boston
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwCalAlgSel                 : 2;        /* У׼����ѡ��Ĭ��ֵ2:
                                                                                   0:���ּӱ���������ֵ+-uhwSearchRangeOffset�ķ�Χ���֣�
                                                                                     ��uhwSweepRange��Χ����
                                                                                   1:����������ֵ+-uhwSearchRangeOffset�ķ�Χ������ΪuhwSweepStep
                                                                                   2:���������������׵㣬����ֵ+-uhwSearchRangeOffset�ķ�Χ
                                                                                     ƽ��ΪuhwPartNum�� */
    VOS_UINT32                          uwSampleLen2PowerN          : 4;        /* ��������1<<N������Ϊ256����������Ĭ��ֵ10 */
    VOS_UINT32                          uwPartNum                   : 5;        /* ��������ȷָ���������Щ���������,��Чֵ4-31 */
    VOS_INT32                           swEtLutBackoff              : 6;        /* ET LUT�����ˣ�ʵ�ʲ����²���,��λ0.1dB��Ĭ��ֵ0 */
    VOS_UINT32                          uwCenterEnvelopTimeDelay    : 15;       /* delay������ֵ��Ĭ��ֵ896 */

    VOS_UINT32                          uwSearchRangeOffset         : 11;       /* ������offset��Ĭ��ֵ500 */
    VOS_UINT32                          uwSweepOffset               : 14;       /* ���������������÷�Χ*2��ʼ������������Ĭ��ֵ200 */
    VOS_UINT32                          uwSweepStep                 : 7;        /* ɨ�貽������uwSweepStep > uwSweepRange�򲻻�ɨ��,Ĭ��ֵ5 */

    VOS_UINT32                          uwAcFreq                    : 16;       /* �ڵ����ֵ�����Ƶ�ʣ���λKHz��Ĭ��ֵ1230 */
    VOS_UINT32                          uwIntegralBW                : 16;       /* ����ACLR���ִ�����λKHz��Ĭ��ֵ1000 */

    VOS_INT32                           swAdcOutdBFS                : 8;        /* ADC���������dBFS����λ0.1��Ĭ��ֵ-80 */
    VOS_UINT32                          uwRslv1                     : 8;        /* ���� */
    VOS_UINT32                          uwRslv2                     : 8;        /* ���� */
    VOS_UINT32                          uwRslv3                     : 8;        /* ���� */

}UCOM_NV_CDMA_ET_DELAY_CAL_PARA_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_CDMA_ET_DPD_LUT_SELF_CAL_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : CDMA ETУ׼���Ʋ���
*****************************************************************************/
typedef struct
{
    VOS_INT16                                   ashwDpdPowerBackOff[4];         /* DPDУ׼�Ļ��˹��ʣ���λ0.1dB��Ĭ��ֵ30 */
    UCOM_NV_DPD_LUT_ALG_PARA_STRU               stDpdAlgPara;                   /* DPD��ȡ�㷨���� */
    UCOM_NV_CDMA_ET_LUT_ALG_PARA_STRU           stEtLutAlgPara;                 /* ET��ȡ�㷨���� */
    UCOM_NV_CDMA_ET_DELAY_CAL_PARA_STRU         stEtDelayCalPara;               /* Et DelayУ׼���Ʋ��� */

}UCOM_NV_CDMA_ET_DPD_LUT_SELF_CAL_PARA_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_ET_DPD_LUT_SELF_CAL_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ET/DPDУ׼���Ʋ���
*****************************************************************************/
typedef struct
{
    VOS_INT16                               ashwDpdPowerBackOff[4];             /* DPDУ׼�Ļ��˹��ʣ���λ0.1dB��Ĭ��ֵ30 */
    UCOM_NV_DPD_LUT_ALG_PARA_STRU           stDpdAlgPara;                       /* DPD��ȡ�㷨���� */
    UCOM_NV_ET_LUT_ALG_PARA_STRU            stEtAlgPara;                        /* ET��ȡ�㷨���� */
    UCOM_NV_W_ET_DELAY_CAL_PARA_STRU        stEtDelayCalPara;                   /* Et DelayУ׼���Ʋ��� */
}UCOM_NV_ET_DPD_LUT_SELF_CAL_PARA_STRU;



/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_CLOSE_MIPI_PA_VCC_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : close vcc control
*****************************************************************************/
typedef struct
{
  PHY_MIPIDEV_UNIT_STRU   astMipiClosePaVcc[UCOM_NV_G_PAVCC_MIPI_UNIT_MAX_NUM];
}UCOM_NV_GSM_CLOSE_MIPI_PA_VCC_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_G_PA_VCC_MIPI_CMD_FRAME_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��PaVcc��mipiָ��֡
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU               astMipiPaVcc[UCOM_NV_G_PAVCC_MIPI_UNIT_MAX_NUM];
}UCOM_NV_G_PA_VCC_MIPI_CMD_FRAME_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_ANT_TUNER_CTRL_INFO_STRU
 �ṹ˵��  : ���ߵ�г�Ŀ��ƽṹ��,MIPI��tuner,���Ǽ��ݶ����ͺţ��ṩ���6��������
*****************************************************************************/
typedef struct
{
    UCOM_NV_TUNER_SUPPORT_MODE_ENUM_UINT16  enAntTunerMode;                     /* Ӳ���Ƿ�֧�����ߵ�г��0��֧�֣�1֧�� */
    UCOM_NV_RFFE_CTRL_ENUM_UINT16           enAntTunerCtrlType;                 /* ���ߵ�г��ͨ��MIPI���ƻ���GPIO,0��ʾGPIO,1��ʾMIPI */
    UCOM_NV_GSM_TUNER_GPIO_MASK_STRU        stTunerGpioMask;
    PHY_MIPIDEV_UNIT_STRU                   astTunerActiveReg[UCOM_NV_G_TUNER_MIPI_UNIT_MAX_NUM];  /* tuner�Ĺ�����ʼ�������֣�����һЩ�Ĵ���ֻ��Ҫģʽ��������һ�� */
    PHY_MIPIDEV_UNIT_STRU                   astTunerIdleReg[UCOM_NV_G_TUNER_MIPI_UNIT_MAX_NUM];    /* ����tunerΪIDLE̬��͹���̬�Ŀ����� */
} UCOM_NV_G_ANT_TUNER_CTRL_INFO_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_NOTCH_CHAN_CTRL_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ���߿����˲���ͨ���Ŀ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              uhwNotchSupportFlag;                /* DCS1800 TX NOTCH����֧�ֱ�־��1��ʾ֧�֣�0��ʾ��֧�� */
    VOS_UINT16                              uhwNotchControlMode;                /* NOTCH���Ʒ�ʽ:1��ʾGPIOģʽ��2��ʾMipi��ʽ��3��ʾGPIO+MIPI��Ϸ�ʽ */
    VOS_UINT32                              uwNotchGpioSetValue;                /* Nothc GPIO����ֵ��uhwNotchControlMode=1����3ʱ��Ч */
    VOS_UINT32                              uwNotchGpioDefaultValue;            /* Nothc GPIO����ֵ��uhwNotchControlMode=1����3ʱ��Ч */
    PHY_MIPIDEV_UNIT_STRU                   astNotchMipiNew[UCOM_NV_G_NOTCH_MIPI_UNIT_MAX_NUM]; /* ����NOTCHͨ��MIPI�ӿڿ��Ƶ�ַ */
    UCOM_NV_GSM_NOTCH_POWER_COMP_STRU       stNotchPowerComp;                   /* Notch���书�ʲ��� */

 }UCOM_NV_GSM_NOTCH_CHAN_CTRL_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_RF_RFIC_INIT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : RFIC��ʼ���Ĵ���
*****************************************************************************/
typedef   NV_MODEM_RF_RFIC_INIT_STRU       UCOM_NV_GUC_RF_RFIC_INIT_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_TEMP_DEFINE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :�¶ȱ���
*****************************************************************************/
typedef   NV_MODEM_TEMP_DEFINE_STRU        UCOM_NV_GUC_TEMP_DEFINE_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :����Ƶ����¶Ȳ���
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulFreq;                              /* ��ǰƵ��,GUTL��λ��100KHz,Cģ��λ��KHz*/
    VOS_INT8                                acTempCompValue[MAX_TEMP_DEFINE_NUM];/* TX���²���λ0.1dB��RX���²���λ0.125dB */
}UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_TEMP_COMP_SINGEL_LEVEL_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ÿ��PA GAIN��Ӧ���¶Ȳ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* Ƶ�����,ĿǰGUC���֧��3��Ƶ�� */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_TRX_TEMP_COMP_FREQ_NUM];
}UCOM_NV_TEMP_COMP_SINGEL_LEVEL_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_RF_RFIC_INIT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : 4��PA GAIN��Ӧ���¶Ȳ���
*****************************************************************************/
typedef struct
{
    UCOM_NV_TEMP_COMP_SINGEL_LEVEL_STRU    astTxTempComp[UCOM_NV_PA_GAIN_MAX_NUM];/* 0--������,1--������,2--������,3--�������� */
}UCOM_NV_GUC_TX_TEMP_COMP_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_RX_TEMP_COMP_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :RX AGC��Ӧ���¶Ȳ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* Ƶ����� */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_TRX_TEMP_COMP_FREQ_NUM];
}UCOM_NV_GUC_RX_TEMP_COMP_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_MRX_TEMP_COMP_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : MRX��Ӧ���¶Ȳ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* Ƶ����� */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_TRX_TEMP_COMP_FREQ_NUM];
}UCOM_NV_GUC_MRX_TEMP_COMP_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_APT_TEMP_COMP_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : APT/ET��Ӧ���¶Ȳ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* Ƶ����� */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_W_APT_FREQ_COMP_NUM];
}UCOM_NV_GUC_APT_TEMP_COMP_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_DPD_TEMP_COMP_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : DPD��Ӧ���¶Ȳ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* Ƶ����� */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_W_DPD_FREQ_COMP_NUM];
}UCOM_NV_GUC_DPD_TEMP_COMP_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_ET_TIMEDELAY_TEMP_COMP_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ET TIME DELAY��Ӧ���¶Ȳ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* Ƶ��������������ŵ�������Ӧ��Ϊ1 */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_ET_DELAY_TEMP_COMP_MAX_NUM]; /* �������ŵ�����СΪ1 */
}UCOM_NV_GUC_ET_TIME_DELAY_TEMP_COMP_STRU;

/*****************************************************************************
 �ṹ��    : NV_MODEM_EXT_LNA_POR_INIT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : EXT_LNA��ʼ���ṹ��
*****************************************************************************/
typedef  NV_MODEM_RF_6H0X_INIT_REG_INFO_STRU UCOM_NV_GUC_RF_6H0X_POR_INIT_STRU;

/*****************************************************************************
 �ṹ��    : NV_MODEM_RF_6H0X_INIT_TABLEIDX_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : EXT LNA��ʼ���Ĵ�������
*****************************************************************************/
typedef  NV_MODEM_RF_6H0X_INIT_TABLEIDX_STRU UCOM_NV_GUC_RF_6H0X_INIT_TABLEIDX_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GUC_INETRFERENCE_TIMING_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : Rx Blank Delay
*****************************************************************************/
typedef struct
{
    VOS_UINT8                             ucGBlankWRxDelay;                     /* ��λchip,����uģchipʱ�����:1chip=(10000/38400)us */
    VOS_UINT8                             ucGBlankCRxDelay;                     /* ��λchip,����cģchipʱ�����:1chip=(1250/1536)us */
    VOS_UINT8                             ucRsv13;                              /* Ԥ�� */
    VOS_UINT8                             ucRsv12;
    VOS_UINT8                             ucRsv11;
    VOS_UINT8                             ucRsv10;
    VOS_UINT8                             ucRsv9;
    VOS_UINT8                             ucRsv8;
    VOS_UINT8                             ucRsv7;
    VOS_UINT8                             ucRsv6;
    VOS_UINT8                             ucRsv5;
    VOS_UINT8                             ucRsv4;
    VOS_UINT8                             ucRsv3;
    VOS_UINT8                             ucRsv2;
    VOS_UINT8                             ucRsv1;
    VOS_UINT8                             ucRsv0;
}UCOM_NV_GUC_INETRFERENCE_TIMING_STRU;


/*****************************************************************************
 �ṹ��    : UCOM_NV_RFIC_SETTING_MIPI_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ÿ��Rf Setting�Ĵ�������������Ƶȷ��3��mipi�Ĵ���
*****************************************************************************/
typedef struct
{
    NV_RF_MIPI_INIT_STRU                astReg[UCOM_NV_RFIC_SETTING_REG_NUM];   /* ÿ��Rf Setting�Ĵ���������3�� */
}UCOM_NV_RFIC_SETTING_MIPI_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_RFIC_SETTING_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : RFIC SETTINGֵ(��High GainΪ�������10�飬��ҪУ׼ѡ������)
*****************************************************************************/
typedef struct
{

    PHY_UINT16                          usValidListNum;                         /* Rf Setting���� */
    PHY_UINT16                          usRsv;                                  /* ����λ */
    UCOM_NV_RFIC_SETTING_MIPI_STRU      astRfSettingList[UCOM_NV_RFIC_SETTING_LIST_NUM]; /* Rf Settingֵ�����10�� */
}UCOM_NV_GSM_RFIC_SETTING_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_RFIC_SETTING_LIST_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : RFIC SETTING����High Gain��Low Gain
*****************************************************************************/
typedef struct
{
    UCOM_NV_GSM_RFIC_SETTING_STRU       stHighGain;                             /* High Gain Para */
    UCOM_NV_GSM_RFIC_SETTING_STRU       stLowGain;                              /* Low Gain Para */
}UCOM_NV_GSM_RFIC_SETTING_LIST_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_RFIC_SETTING_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : RFIC SETTING��ز���(Vdacֵ�͹�������)����High Gain��Low Gain
*****************************************************************************/
typedef struct
{
    PHY_UINT16                          usDacTargetHigh;                        /* High Gain Vdacֵ */
    PHY_UINT16                          usPowerLimit10thHigh;                   /* High Gain �������ޣ���λ0.1dBm */
    PHY_UINT16                          usDacTargetLow;                         /* Low Gain Vdacֵ */
    PHY_UINT16                          usPowerLimit10thLow;                    /* Low Gain �������ޣ���λ0.1dBm */
}UCOM_NV_GSM_RFIC_SETTING_PARA_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_RFIC_SETTING_CAL_RESULT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : У׼nv��RFIC SETTING����ֵ����High Gain��Low Gain
*****************************************************************************/
typedef struct
{
    UCOM_NV_RFIC_SETTING_MIPI_STRU      stHighGain;                             /* High Gain Rf SettingУ׼ֵ */
    UCOM_NV_RFIC_SETTING_MIPI_STRU      stLowGain;                              /* Low Gain Rf SettingУ׼ֵ */
}UCOM_NV_GSM_RFIC_SETTING_CAL_RESULT_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_WG_READBACK_MIPI_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : gu MIPI�ض����ƺ�����
*****************************************************************************/
typedef struct
{
    PHY_UINT16                      uhwCycMipiRdEn;                             /* ������MIPI�ض������Ƿ�ʹ�� */
    PHY_UINT16                      uhwRsv;

    PHY_MIPIDEV_CMD_STRU            astRficMipi[UCOM_NV_RDBACK_MIPI_NUM];       /* ���Ƶ������Զ�ȡ��RFIC mipi */
    PHY_MIPIDEV_CMD_STRU            astRffeMipi[UCOM_NV_RDBACK_MIPI_NUM];       /* ���Ƶ������Զ�ȡ��RFFE mipi */
}UCOM_NV_WG_READBACK_MIPI_STRU;

/*****************************************************************************
 �ṹ��    : UCOM_NV_GSM_RAMP_COEFF_REF_STRU
 Э����  :
 �ṹ˵��  : GSM�����±�ο�ϵ����16����
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          auhwRampUp[UCOM_NV_G_RAMP_COEF_NUM];    /* ���²ο�ϵ��������1024�� */
}UCOM_NV_GSM_RAMP_COEFF_REF_STRU;

/*****************************************************************************
  7 UNION����
*****************************************************************************/


/*****************************************************************************
  8 OTHERS����
*****************************************************************************/


/*****************************************************************************
  9 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of phyNvDefine_Boston */

