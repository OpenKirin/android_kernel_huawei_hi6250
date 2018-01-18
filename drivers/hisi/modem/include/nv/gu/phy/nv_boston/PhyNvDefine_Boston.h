/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : phyNvDefine_Boston
  版 本 号   : 初稿
  作    者   : h00165915
  生成日期   : 2016年7月1日
  最近修改   :
  功能描述   : PhyNvDefine.h 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2016年7月1日
    作    者   : h00165915
    修改内容   : 创建文件

******************************************************************************/

#ifndef __PHYNVDEFINE_BOSTON_H__
#define __PHYNVDEFINE_BOSTON_H__


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#include "nv_common_interface.h"

#pragma pack(1)

/*****************************************************************************
  2 宏定义
*****************************************************************************/

/* ET支持的最大MIPI个数 */
#define UCOM_NV_ET_MIPI_NUM             ( 6 )

#define UCOM_NV_ET_APT_TRIGGER_NUM      ( 2 )
#define UCOM_NV_W_TX_DCDC_MIPI_NUM      ( 4 )                                   /* DCDC MIPI个数 */

/* CDMA tas mipi最大单元个数 */
#define UCOM_NV_CDMA_TAS_MIPI_NUM       ( 4 )
#define UCOM_NV_W_TEMP_NUM              ( 0x10  )                               /* 温度补偿设置的温度点，前5个分别代表 -20,0,20,40,60，第6个用于字节对齐 */

/* MRX最大支持有效档位,目前最大支持5档 */
#define UCOM_NV_W_MRX_GAIN_MAX_NUM      ( 5 )

#define UCOM_NV_GUC_MIPI_INIT_UNIT_MAX_NUM      ( 16 )

#define UCOM_NV_TAS_DPDT_MIPI_UNIT_MAX_NUM      ( 4 )
#define UCOM_NV_G_NOTCH_MIPI_UNIT_MAX_NUM       ( 4 )
#define UCOM_NV_G_PAVCC_MIPI_UNIT_MAX_NUM       ( 4 )
#define UCOM_NV_G_TUNER_MIPI_UNIT_MAX_NUM       ( 4 )
#define UCOM_NV_G_MIPI_INIT_UNIT_MAX_NUM        ( 16 )
#define UCOM_NV_MAX_MIPI_ANT_UNIT_NUMBER        ( 4 )

#define UCOM_NV_RFIC_SETTING_LIST_NUM           ( 10 )                          /* Rf Setting组数，最多10组 */
#define UCOM_NV_RFIC_SETTING_REG_NUM            ( 3 )                           /* 每组Rf Setting寄存器个数，3个 */
/*****************************************************************************
 结构名    : UCOM_NV_FEM_PIN_TO_GPIO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : NV_FEMPIN_TO_GPIO_STRU定义在tl drv的nv头文件中
*****************************************************************************/
typedef NV_FEMPIN_TO_GPIO_STRU UCOM_NV_FEM_PIN_TO_GPIO_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GPIO_MIPI_CTRL_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : FEM_GPIO_MIPIDEV_CTRL_STRU定义在tl drv的nv头文件中
*****************************************************************************/
typedef FEM_GPIO_MIPIDEV_CTRL_STRU UCOM_NV_GPIO_MIPI_CTRL_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_RFFE_GPIO_VALUE_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : RFFE_GPIO_VALUE_STRU定义在tl drv的nv头文件中
*****************************************************************************/
typedef RFFE_GPIO_VALUE_STRU UCOM_NV_RFFE_GPIO_VALUE_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_ANT_SWITCH_MIPI_CTRL_WORD_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : RF_NV_MIPIDEV_04CMD定义在tl drv的nv头文件中
*****************************************************************************/
typedef RF_NV_MIPIDEV_04CMD UCOM_NV_ANT_SWITCH_MIPI_CTRL_WORD_STRU;

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 消息头定义
*****************************************************************************/


/*****************************************************************************
  5 消息定义
*****************************************************************************/


/*****************************************************************************
  6 STRUCT定义
*****************************************************************************/

/*****************************************************************************
 结构名    : UCOM_NV_GUC_BAND_DL_RFIC_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 下行RFIC配置,包括RFIC ID,通道号，输入的PORT口
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitRficIdx                  :2;         /* RFIC ID：[0,2]分别表示RFIC1，RFIC2，RFIC3 */
    VOS_UINT32                          bitRficRxCh                 :2;         /* 下行RX通道号，每个RFIC有3个通道，分别是RX1，RX2，RX3; 对于下行4*4MIMO固定使用RX1A1B+RX3A3B,但是采用同一个TX_PLL1控制，因此MIMO场景不用选RX通道，但4收切2收时，
                                                                                只切向Main和Div Ant,RFIC的两收RxCh需要填写使用的通道号；如果支持2收场景，RxCh表示使用的Rx通道号;
                                                                                4R 和2R复用同一个NV */


    VOS_UINT32                          bitRfPortCrossFlag          :1;         /*主集天线的信号流接到了 RFIC DRX Port口，软件需要在CTU通道交叉到主集，只支持2收的交叉*/
    VOS_UINT32                          bitMainAntRxPortSel         :4;         /* 主集LNA的RFIC端口选择，即信号通过哪个端口输入到RFIC */
    VOS_UINT32                          bitDivAntRxPortSel          :4;         /* 分集LNA的RFIC端口选择 */
    VOS_UINT32                          bitRsv0                     :3;         /* 预留位 */
    VOS_UINT32                          bitRsv1                     :4;
    VOS_UINT32                          bitRsv2                     :4;
    VOS_UINT32                          bitRsv3                     :4;
    VOS_UINT32                          bitRsv4                     :4;

} UCOM_NV_GUC_BAND_DL_RFIC_PARA_STRU;

/*****************************************************************************
 结构名    : NV_BAND_UL_RFIC_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 上行RFIC配置,包括RFIC ID,通道号，输出的PORT口
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitRficIdx                 :2;          /* 上行通道所在的RFIC ID,支持收发分离 */
    VOS_UINT32                          bitRficTxCh                :2;          /* 上行通道号，上行MIMO固定使用Tx1Ch,因此当满足上行MIMO后，不取此通道；单发情况下，填写Tx1Iq1,Tx2Iq；上行2Tx转1Tx复用NV，策略和Rx一样*/
    VOS_UINT32                          bitMainAntRficTxPortSel    :8;          /* 上行PORT口选择 */
    VOS_UINT32                          bitMainAntRficTxMrxPortSel :4;           /*上行环回通道对Mrx通道的选择，0: MRX1 RF input selected  1: MRX2 RF input selected*/
    VOS_UINT32                          bitRsv0                    :4;
    VOS_UINT32                          bitRsv1                    :4;
    VOS_UINT32                          bitRsv2                    :4;
    VOS_UINT32                          bitRsv3                    :4;

}UCOM_NV_GUC_BAND_UL_RFIC_PARA_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_BAND_DL_PATH_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 每个PATH的下行RFIC通道配置
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitRxAntNum                 :3;         /* 接收天线数 */
    VOS_UINT32                          bitRsv0                     :5;
    VOS_UINT32                          bitRsv1                     :8;
    VOS_UINT32                          bitRsv2                     :8;
    VOS_UINT32                          bitRsv3                     :8;

    UCOM_NV_GUC_BAND_DL_RFIC_PARA_STRU  stBandDlRficPara;                       /* 下行RFIC参数 */

} UCOM_NV_GUC_BAND_DL_PATH_PARA_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_BAND_UL_PATH_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 每个PATH的上行RFIC通道配置
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

    UCOM_NV_GUC_BAND_UL_RFIC_PARA_STRU  stBandUlRficPara;                       /* RFIC参数 */
}UCOM_NV_GUC_BAND_UL_PATH_PARA_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_BAND_DL_GPIO_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 前端下行GPIO配置，包括天线开关，TUNER等，支持天线开关和TUNER单独配置
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulMainRxAntSel;                         /*ASM通路上的切换开关,各天线开关或出去*/
    VOS_UINT32                              ulMainRxAntSelExt;                      /*ASM通路上的切换开关,各天线开关或出去*/
    VOS_UINT32                              ulDivRxAntSel;
    VOS_UINT32                              ulDivRxAntSelExt;
    VOS_UINT32                              ulMainRxTunerSel;                       /*ASM通路上的切换开关*/
    VOS_UINT32                              ulMainRxTunerSelExt;
    VOS_UINT32                              ulDivRxTunerSel;
    VOS_UINT32                              ulDivRxTunerSelExt;
    VOS_UINT32                              ulRsv0;                             /* 预留 */
    VOS_UINT32                              ulRsv1;
    VOS_UINT32                              ulRsv2;
    VOS_UINT32                              ulRsv3;
    VOS_UINT32                              ulRsv4;
    VOS_UINT32                              ulRsv5;
    VOS_UINT32                              ulRsv6;
    VOS_UINT32                              ulRsv7;
} UCOM_NV_GUC_BAND_DL_GPIO_PARA_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_BAND_DL_MIPI_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 前端下行MIPI配置，包括天线开关，TUNER等，支持天线开关和TUNER单独配置
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU              astMipiMainAntRxSel[RX_ANT_SEL_MIPI_NUM];            /*Ant Mipi num=4,需要射频评估*/
    PHY_MIPIDEV_UNIT_STRU              astMipiDivAntRxSel[RX_ANT_SEL_MIPI_NUM ];
    PHY_MIPIDEV_UNIT_STRU              astMipiMainTunerRxSel[ANT_TUNER_MIPI_NUM ];
    PHY_MIPIDEV_UNIT_STRU              astMipiDivTunerRxSel[ANT_TUNER_MIPI_NUM ];           /*Tuner Mipi num=2,需要射频评估*/
    PHY_MIPIDEV_UNIT_STRU              astMipiMainAntRxCloseSel[RX_ANT_SEL_MIPI_NUM];       /* 新增NV */
    PHY_MIPIDEV_UNIT_STRU              astMipiDivAntRxCloseSel[RX_ANT_SEL_MIPI_NUM];        /* 新增分集关闭NV */
    PHY_MIPIDEV_UNIT_STRU              astMipiMRsv1[RX_ANT_SEL_MIPI_NUM];
} UCOM_NV_GUC_BAND_DL_MIPI_PARA_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_BAND_DL_FEM_PATH_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 每条PATH的前端下行MIPI配置，包括天线开关，TUNER等，
             支持天线开关和TUNER单独配置
*****************************************************************************/
typedef struct
{
    UCOM_NV_GUC_BAND_DL_GPIO_PARA_STRU           stBandDlGpioPara;              /* 每条PATH下行GPIO配置 */
    UCOM_NV_GUC_BAND_DL_MIPI_PARA_STRU           stBandDlMipiPara;              /* 每条PATH下行MIPI配置 */
}UCOM_NV_GUC_BAND_DL_FEM_PATH_PARA_STRU;

typedef  UCOM_NV_GUC_BAND_DL_FEM_PATH_PARA_STRU      UCOM_NV_GSM_RF_DL_FEM_PATH_STRU;
typedef  UCOM_NV_GUC_BAND_DL_FEM_PATH_PARA_STRU      UCOM_NV_CDMA_RF_DL_FEM_PATH_STRU;
typedef  UCOM_NV_GUC_BAND_DL_FEM_PATH_PARA_STRU      UCOM_NV_W_DL_FEM_PATH_PARA_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_BAND_UL_GPIO_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 上行GPIO配置，包括天线开关，TUNER等，支持天线开关和TUNER单独配置
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulTxAntSel;                             /* 上行天线开关 */
    VOS_UINT32                              ulTxAntSelExt;
    VOS_UINT32                              ulTxTuner;                              /* 上行Tuner开关 */
    VOS_UINT32                              ulTxTunerExt;
    VOS_UINT32                              ulPaEnCtrl;                             /* PA线控开关 */
    VOS_UINT32                              ulPaHighModeCtrl;                       /* PA高增益控制字 */
    VOS_UINT32                              ulPaMidModeCtrl;                        /* PA中增益控制字 */
    VOS_UINT32                              ulPaLowModeCtrl;                        /* PA低增益控制字 */
    VOS_UINT32                              ulPaUltraLowModeCtrl;                   /* utralow */
    VOS_UINT32                              ulRsv;
} UCOM_NV_GUC_BAND_UL_GPIO_PARA_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_BAND_UL_MIPI_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 上行MIPI配置，包括天线开关，TUNER等，支持天线开关和TUNER单独配置
             由于Boston需要对档位控制字进行转换
             所以该结构体如果在astMipiPaHighMode之前增加成员需要重新计算偏移
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU               astMipiAntSelTx[TX_ANT_SEL_MIPI_NUM];   /* MIPI上行天线开关  TX_ANT_SEL_MIPI_NUM =4 */
    PHY_MIPIDEV_UNIT_STRU               astMipiTunerTx[ANT_TUNER_MIPI_NUM];     /* MIPI上行TUNER开关 Tx Tuner is only for Gsm,num=2 */

    PHY_MIPIDEV_UNIT_STRU               astMipiPaOnCtrl[PA_MODE_MIPI_NUM];      /* MIPI PA On控制 */
    PHY_MIPIDEV_UNIT_STRU               astMipiPaOffCtrl[PA_MODE_MIPI_NUM];     /* MIPI PA OFF控制 */
    PHY_MIPIDEV_UNIT_STRU               astMipiPaHighMode[PA_MODE_MIPI_NUM];    /* MIPI PA高增益控制字 PA_MODE_MIPI_NUM=4 */
    PHY_MIPIDEV_UNIT_STRU               astMipiPaMidMode[PA_MODE_MIPI_NUM];     /* MIPI PA中增益控制字 PA_MODE_MIPI_NUM=4 */
    PHY_MIPIDEV_UNIT_STRU               astMipiPaLowMode[PA_MODE_MIPI_NUM];     /* MIPI PA低增益控制字 PA_MODE_MIPI_NUM=4 */
    PHY_MIPIDEV_UNIT_STRU               astMipiPaUltraLowMode[PA_MODE_MIPI_NUM];
    PHY_MIPIDEV_UNIT_STRU               astMipiAntSelTxClose[TX_ANT_SEL_MIPI_NUM];
    PHY_MIPIDEV_UNIT_STRU               stMipiPaTrigger;                        /* GSM用 */
} UCOM_NV_GUC_BAND_UL_MIPI_PARA_STRU;

/*****************************************************************************
 结构名    : NV_BAND_EXT_LNA_PATH_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 外置LNA按照天线数定义结构信息
*****************************************************************************/
typedef struct
{
    NV_EXT_LNA_ANTINFO_STRU                    stMainAntLnaInfo;
    NV_EXT_LNA_ANTINFO_STRU                    stDivAntLnaInfo;
    PHY_MIPIDEV_UNIT_STRU                      astRsv[LNA_MIPI_TRIG_NUM];       /* 预留 */

}UCOM_NV_GUC_BAND_EXT_LNA_PATH_PARA_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_RX_AGC_CAL_RESULT_TABLE_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : AGC某一个PATH对应的总的校准结果，GUC只用一个PATH，区分BAND
*****************************************************************************/
typedef struct
{
    NV_RX_AGC_CAL_RESULT_STRU           stMainAntAgcCalResult;                  /* 主集天线对应的AGC校准结果 */
    NV_RX_AGC_CAL_RESULT_STRU           stDivAntAgcCalResult;                   /* 分集天线对应的AGC校准结果 */

}UCOM_NV_GUC_RX_AGC_CAL_RESULT_TABLE_STRU;

/*****************************************************************************
 结构名    : NV_GUC_RX_DCOC_CAL_RESULT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 某一个PATH的DCOC的校准结果
*****************************************************************************/
typedef struct
{
    NV_RX_DCOC_CAL_RESULT_STRU          stMainAntCalResult;                    /* 主集的DCOC校准结果 */
    NV_RX_DCOC_CAL_RESULT_STRU          stDivAntCalResult;                     /* 分集的DCOC校准结果 */
}UCOM_NV_GUC_RX_PATH_DCOC_CAL_RESULT_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_RX_AGC_TABLE_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 某一个PATH中每根天线所选择的AGC TABLE INDEX，根据INDEX去索引对应的
 AGC的切换门限、AGC DEFAULT GAIN和AGC的RFIC控制字
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          bitMainAntRxAgcTableIdx         :4;     /* 主集天线对应的AGC TABLE INDEX */
    VOS_UINT32                          bitDivAntRxAgcTableIdx          :4;     /* 分集天线对应的AGC TABLE INDEX */
    VOS_UINT32                          bitRsv0                         :4;
    VOS_UINT32                          bitRsv1                         :4;
    VOS_UINT32                          bitRsv2                         :8;
    VOS_UINT32                          bitRsv3                         :8;
}UCOM_NV_GUC_RX_AGC_TABLE_INFO_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_RF_BAND_CONFIG_STRU
 结构说明  : GUC Band Config配置
*****************************************************************************/
typedef struct
{
    UCOM_NV_GUC_BAND_DL_PATH_PARA_STRU stBandDlPathPara;                        /* 下行相关 */
    UCOM_NV_GUC_BAND_UL_PATH_PARA_STRU stBandUlPathPara;                        /* 上行相关 */

}UCOM_NV_GUC_RF_BAND_CONFIG_STRU;

typedef UCOM_NV_GUC_RF_BAND_CONFIG_STRU       UCOM_NV_GSM_RF_BAND_CONFIG_STRU;
typedef UCOM_NV_GUC_RF_BAND_CONFIG_STRU       UCOM_NV_CDMA_BAND_CONFIG_STRU;
typedef UCOM_NV_GUC_RF_BAND_CONFIG_STRU       UCOM_NV_W_RF_BAND_CONFIG_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_BAND_UL_FEM_PATH_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 前端上行配置，每个PATH的配置
*****************************************************************************/
typedef struct
{
    UCOM_NV_GUC_BAND_UL_GPIO_PARA_STRU           stBandUlGpioPara;              /* 上行GPIO配置 */
    UCOM_NV_GUC_BAND_UL_MIPI_PARA_STRU           stBandUlMipiPara;              /* 上行MIPI配置 */
}UCOM_NV_GUC_BAND_UL_FEM_PATH_PARA_STRU;

typedef  UCOM_NV_GUC_BAND_UL_FEM_PATH_PARA_STRU    UCOM_NV_GSM_RF_UL_FEM_PATH_STRU;
typedef  UCOM_NV_GUC_BAND_UL_FEM_PATH_PARA_STRU    UCOM_NV_CDMA_RF_UL_FEM_PATH_STRU;
typedef  UCOM_NV_GUC_BAND_UL_FEM_PATH_PARA_STRU    UCOM_NV_W_UL_FEM_PATH_PARA_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_GSM_RF_EXT_LNA_PATH_STRU
 结构说明  : 外置LNA 单个 Path 总结构参数结构体定义，包括Band号，Path信息
*****************************************************************************/
typedef struct
{
    UCOM_NV_GUC_BAND_EXT_LNA_PATH_PARA_STRU stExtLnaPathPara;
}UCOM_NV_GSM_RF_EXT_LNA_PATH_STRU;

typedef   UCOM_NV_GUC_BAND_EXT_LNA_PATH_PARA_STRU    UCOM_NV_CDMA_RF_EXT_LNA_PATH_STRU;
typedef   UCOM_NV_GUC_BAND_EXT_LNA_PATH_PARA_STRU    UCOM_NV_W_EXT_LNA_PATH_PARA_STRU;

/*****************************************************************************
 结构名    : NV_GUC_RF_RX_CAL_FREQ_LIST_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 需要校准的频点列表，每个BAND最多支持32个频点，该NV区分BAND号
*****************************************************************************/
typedef struct
{
    PHY_UINT32                              ulValidCalFreqNum;                      /* 需要校准的频点个数 */
    PHY_UINT32                              aulRxCalFreqList[RF_RX_MAX_FREQ_NUM];   /* RF_RX_FREQ_NUM =32 ,GUTL单位100KHz,CDMA:1KHz*/

}NV_GUC_RF_RX_CAL_FREQ_LIST_STRU;

/*****************************************************************************
 结构名    : NV_GUC_RF_TX_CAL_FREQ_LIST_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 需要校准的频点列表，每个BAND最多支持32个频点，该NV区分BAND号
*****************************************************************************/
typedef struct
{
    PHY_UINT32                          ulValidCalFreqNum;                      /* 需要校准的频点个数 */
    PHY_UINT32                          aulTxCalFreqList[RF_TX_MAX_FREQ_NUM];   /* RF_RX_FREQ_NUM =32 ,GUTL单位100KHz,CDMA:1KHz*/
}NV_GUC_RF_TX_CAL_FREQ_LIST_STRU;

/*****************************************************************************
 结构名    : NV_RF_TX_CAL_FREQ_LIST_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 需要校准的频点列表，每个BAND最多支持32个频点，该NV区分BAND号
*****************************************************************************/
typedef   NV_GUC_RF_TX_CAL_FREQ_LIST_STRU       UCOM_NV_GSM_RF_TX_CAL_FREQ_LIST_STRU;
typedef   NV_GUC_RF_TX_CAL_FREQ_LIST_STRU       UCOM_NV_WCDMA_RF_TX_CAL_FREQ_LIST_STRU;
typedef   NV_GUC_RF_TX_CAL_FREQ_LIST_STRU       UCOM_NV_CDMA_RF_TX_CAL_FREQ_LIST_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_RF_RX_CAL_FREQ_LIST_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 需要校准的频点列表，每个BAND最多支持32个频点，该NV区分BAND号
*****************************************************************************/
typedef   NV_GUC_RF_RX_CAL_FREQ_LIST_STRU       UCOM_NV_GSM_RF_RX_CAL_FREQ_LIST_STRU;
typedef   NV_GUC_RF_RX_CAL_FREQ_LIST_STRU       UCOM_NV_CDMA_RF_RX_CAL_FREQ_LIST_STRU;
typedef   NV_GUC_RF_RX_CAL_FREQ_LIST_STRU       UCOM_NV_WCDMA_RF_RX_CAL_FREQ_LIST_STRU;


/*****************************************************************************
 结构名    : NV_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU
 结构说明  : GSM/WCDMA/CDMA 的AGC慢速切换门限
*****************************************************************************/
typedef   NV_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU     UCOM_NV_GSM_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU;
typedef   NV_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU     UCOM_NV_CDMA_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU;
typedef   NV_ALG_RX_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU     UCOM_NV_W_ALG_RX_AGC_THRESHOLD_TABLE_BANK_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GSM_ALG_RX_FAST_AGC_SWITCH_THRESHOLD_TABLE_BANK_STRU
 结构说明  : GSM的AGC快速切换门限
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          auhwBlockInitAgc[AGC_MAX_TABLE_NUM];    /* 快速测量初始档位 */
    VOS_UINT16                          auhwNoBlockInitAgc[AGC_MAX_TABLE_NUM];  /* 快速测量初始档位 */
    NV_RX_AGC_GAIN_THRESHOLD_STRU       astAgcSwitchTable[AGC_MAX_TABLE_NUM];   /*AGC_MAX_TABLE = 10*/

}UCOM_NV_GSM_ALG_RX_AGC_FAST_SWITCH_THRESHOLD_TABLE_BANK_STRU;

/*****************************************************************************
 结构名    : NV_GUC_RF_RX_AGC_TABLE_USED_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : AGC的TABLE INDEX，最多支持6个PATH，GUC只需要使用一个PATH，区分BAND
*****************************************************************************/
typedef     UCOM_NV_GUC_RX_AGC_TABLE_INFO_STRU      UCOM_NV_GSM_RF_RX_AGC_TABLE_USED_INFO_STRU;
typedef     UCOM_NV_GUC_RX_AGC_TABLE_INFO_STRU      UCOM_NV_CDMA_RF_RX_AGC_TABLE_USED_INFO_STRU;
typedef     UCOM_NV_GUC_RX_AGC_TABLE_INFO_STRU      UCOM_NV_W_RF_RX_AGC_TABLE_USED_INFO_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GSM_RX_AGC_CAL_RESULT_TABLE_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : AGC的校准结果值，包括增益值和频补值
*****************************************************************************/
typedef UCOM_NV_GUC_RX_AGC_CAL_RESULT_TABLE_STRU   UCOM_NV_GSM_RX_AGC_CAL_RESULT_TABLE_STRU;
typedef UCOM_NV_GUC_RX_AGC_CAL_RESULT_TABLE_STRU   UCOM_NV_CDMA_RX_AGC_CAL_RESULT_TABLE_STRU;
typedef UCOM_NV_GUC_RX_AGC_CAL_RESULT_TABLE_STRU   UCOM_NV_W_RX_AGC_CAL_RESULT_TABLE_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GSM_RX_PATH_DCOC_CAL_RESULT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : DCOC的校准结果值,包括校准码字和BBP上报的残余直流值
*****************************************************************************/
typedef     UCOM_NV_GUC_RX_PATH_DCOC_CAL_RESULT_STRU     UCOM_NV_GSM_RX_PATH_DCOC_CAL_RESULT_STRU;
typedef     UCOM_NV_GUC_RX_PATH_DCOC_CAL_RESULT_STRU     UCOM_NV_CDMA_RX_PATH_DCOC_CAL_RESULT_STRU;
typedef     UCOM_NV_GUC_RX_PATH_DCOC_CAL_RESULT_STRU     UCOM_NV_W_RX_PATH_DCOC_CAL_RESULT_STRU;

typedef     NV_RX_DCOC_CAL_RESULT_STRU                   NV_GUC_RX_DCOC_CAL_RESULT_STRU;

typedef     NV_RX_DCOC_COMP_VAlUE_STRU                   NV_GUC_RX_DCOC_COMP_VAlUE_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GSM_RX_PATH_IP2_CAL_RESULT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : IP2的校准结果值
*****************************************************************************/
typedef NV_RX_PATH_IP2_CAL_RESULT_STRU      UCOM_NV_GSM_RX_PATH_IP2_CAL_RESULT_STRU;
typedef NV_RX_PATH_IP2_CAL_RESULT_STRU      UCOM_NV_CDMA_RX_PATH_IP2_CAL_RESULT_STRU;
typedef NV_RX_PATH_IP2_CAL_RESULT_STRU      UCOM_NV_W_RX_PATH_IP2_CAL_RESULT_STRU;

typedef NV_RX_IP2_CAL_RESULT_STRU           UCOM_NV_W_RX_IP2_CAL_RESULT_STRU;

/*****************************************************************************
 枚举名    : UCOM_NV_W_RF_RX_AGC_RFIC_TABLE_BANK_STRU
 协议表格  :
 枚举说明  : AGC档位切换控制字,boston及之后使用(因为RF变化)
*****************************************************************************/
typedef NV_RF_RX_AGC_RFIC_TABLE_BANK_STRU   UCOM_NV_G_RF_RX_AGC_RFIC_TABLE_BANK_STRU;
typedef NV_RF_RX_AGC_RFIC_TABLE_BANK_STRU   UCOM_NV_CDMA_RF_RX_AGC_RFIC_TABLE_BANK_STRU;
typedef NV_RF_RX_AGC_RFIC_TABLE_BANK_STRU   UCOM_NV_W_RF_RX_AGC_RFIC_TABLE_BANK_STRU;

/*****************************************************************************
 枚举名    : NV_GUC_RF_RX_AGC_GAIN_TABLE_BANK_STRU
 协议表格  :
 枚举说明  : AGC GAIN
*****************************************************************************/
typedef NV_RF_RX_AGC_GAIN_TABLE_BANK_STRU   UCOM_NV_G_RF_RX_AGC_GAIN_TABLE_BANK_STRU;
typedef NV_RF_RX_AGC_GAIN_TABLE_BANK_STRU   UCOM_NV_CDMA_RF_RX_AGC_GAIN_TABLE_BANK_STRU;
typedef NV_RF_RX_AGC_GAIN_TABLE_BANK_STRU   UCOM_NV_W_RF_RX_AGC_GAIN_TABLE_BANK_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_CDMA_RF_RX_RFFE_ILOSS_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 前端的插损值，CDMA只有1个PATH，每个PATH最多支持双收，区分Band号
*****************************************************************************/
typedef NV_RX_PATH_ILOSS_STRU       UCOM_NV_CDMA_RF_RX_RFFE_ILOSS_STRU;
typedef NV_RX_PATH_ILOSS_STRU       UCOM_NV_W_RF_RX_RFFE_ILOSS_STRU;


typedef NV_RX_AGC_GAIN_DEFAULT_STRU  NV_GUC_RX_AGC_GAIN_DEFAULT_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_RF_HW_BASIC_INFO_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : RFIC和ABB连接关系NV项，包括RFIC个数、RFIC和ABB连接通道等
*****************************************************************************/
typedef  NV_MODEM_RF_HW_BASIC_INFO_STRU    UCOM_NV_GUC_RF_HW_BASIC_INFO_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_TAS_DPDT_MIPI_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :CDMA TAS MIPI 结构体
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU                    astTasDpdtThrough[UCOM_NV_CDMA_TAS_MIPI_NUM];    /* 直通 */
    PHY_MIPIDEV_UNIT_STRU                    astTasDpdtSwap[UCOM_NV_CDMA_TAS_MIPI_NUM];       /* 交叉 */
}UCOM_NV_GUC_TAS_DPDT_MIPI_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_W_BAND_BIT_MASK_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : BAND是否支持
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwBandMask;                             /* bit0为1表示支持band1, 类推 */
    VOS_UINT32                          uwBandMaskEx;
}UCOM_NV_W_BAND_BIT_MASK_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_TX_FEATURE_DEFINE_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 上行特性控制
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwBand      : 6;                        /* WCDMA:1 - 63表示band1-63,默认值0,
                                                                                   CDMA: 0 - 10表示bc0 - bc10
                                                                                   若有多个索引band填写相同，则第一个生效 */
    VOS_UINT32                          uwAptCtrl   : 2;                        /* APT开关,0:关，1:开，默认值0 */
    VOS_UINT32                          uwDpdCtrl   : 3;                        /* DPD开关,默认值0
                                                                                   0:DPD关，
                                                                                   1:APT+DPD，DPD在APT下提取
                                                                                   2:ET+DPD，DPD在ET下提取 */
    VOS_UINT32                          uwEtCtrl    : 3;                        /* ET特性控制,默认值0
                                                                                   0:ET关，
                                                                                   1:最高一档为ET，
                                                                                   2:最高二档为ET，但ET参数不区分档位，取最高档(H)参数，
                                                                                   3:最高二档为ET，但ET参数区分档位参数 */
    VOS_UINT32                          uwRslv0     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv1     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv2     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv3     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv4     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv5     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv6     : 6;                        /* 保留 */

}UCOM_NV_TX_FEATURE_DEFINE_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_W_FE_TX_FEATURE_CTRL_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : BAND上行特性控制
*****************************************************************************/
typedef struct
{
    UCOM_NV_TX_FEATURE_DEFINE_STRU      astTxFeature[16];                           /* 特性开关，若有多个索引band填写相同，则第一个生效 */
}UCOM_NV_W_FE_TX_FEATURE_CTRL_STRU;


typedef UCOM_NV_W_FE_TX_FEATURE_CTRL_STRU     UCOM_NV_CDMA_FE_TX_FEATURE_CTRL_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_TX_FEATURE_DEFINE_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 上行特性控制
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwBand      : 6;                        /* WCDMA:1 - 63表示band1-63,默认值0,
                                                                                   CDMA: 0 - 10表示bc0 - bc10
                                                                                   若有多个索引band填写相同，则第一个生效 */

    VOS_UINT32                          uwRslv0     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv1     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv2     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv3     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv4     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv5     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv6     : 2;                        /* 保留 */
    VOS_UINT32                          uwRslv7     : 2;                        /* 保留 */

    VOS_UINT32                          uwRslv8     : 10;                       /* 保留 */

}UCOM_NV_RX_FEATURE_DEFINE_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_W_FE_RX_FEATURE_CTRL_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : BAND是否支持特性
*****************************************************************************/
typedef struct
{
    UCOM_NV_RX_FEATURE_DEFINE_STRU      astRxFeature[16];

}UCOM_NV_W_FE_RX_FEATURE_CTRL_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_W_BBP_TX_TIMING_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : W上行时序定义
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          uhwTxChanDelay;                         /* 上行调制延时补偿 */
    VOS_UINT16                          uhwTxInitDelay;                         /* 提前打开上行RF的chip数。取值范围0~1023。典型值800us等于3072chip */
    VOS_UINT16                          uhwTxOffDelay;                          /* 延后关闭上行RF的chip数。 */
    VOS_UINT16                          uhwTxIdleOnDelay;                       /* 提前打开上行RF IDLE状态的chip数。取值范围0~1023。典型值100us等于384chip */
    VOS_UINT16                          uhwTxIdleOffDelay;                      /* 延后关闭上行RF IDLE状态的chip数。 */
    VOS_UINT16                          uhwTxAbbInitDelay;                      /* 保留未使用 */
    VOS_UINT16                          uhwTxAbbOffDelay;                       /* 保留未使用 */
    VOS_UINT16                          shwTxGainOpenTime;                      /* 上行发射功率生效时间，单位chip。表示上行发射功率需提前上行时隙头多久时间送至射频器件。
                                                                                   由于射频器件有延时，为使上行发射功率在上行时隙头处生效，需提前将上行发射功率计算结果送至射频器件 */
    VOS_UINT16                          uhwEtDataPathIntDelay;                  /* ET数据通路整数延时,默认138 */
    VOS_UINT16                          uhwTxPaStableTime;                      /* 发射功率变化后，RF与PA的稳定时间，包含MRX通道配置后的稳定时间（MRX打开在上行时隙头位置开启），单位chip */
    VOS_UINT16                          uhwMrxAbbStableTime;                    /* MRX ABB线控开启后，等待uhwMrxAbbStableTime可以启动MRX功率估计，单位chip */
    VOS_UINT16                          uhwFachTxDataValidOpenTime;             /* FACH态，TX数据发送打开位置，必然晚于tx_abb_open_time，以此保证ABB复位期间无数据vld输出，单位样点，默认0 */
    VOS_UINT16                          uhwFachTxDataValidCloseTime;            /* FACH态，TX数据发送关闭位置，不晚于tx_abb_close_time，单位样点，默认0 */
    VOS_UINT16                          uhwDchTxDataValidOpenTime;              /* DCH态，TX数据发送打开位置，必然晚于tx_abb_open_time，以此保证ABB复位期间无数据vld输出，单位样点，默认0 */
    VOS_UINT16                          uhwDchTxDataValidCloseTime;             /* DCH态，TX数据发送关闭位置，不晚于tx_abb_close_time，单位样点，默认0 */
    VOS_UINT16                          uhwReserved0;                           /* 保留 */

    VOS_UINT16                          uhwReserved1;                           /* 保留 */
    VOS_UINT16                          uhwReserved2;                           /* 保留 */
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
 结构名    : UCOM_NV_W_TX_BAND_TIMING_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : W上行时序定义
*****************************************************************************/
typedef struct
{
    UCOM_NV_W_PA_GAIN_SWITH_TIMING_STRU     stPaGainSwichTiming;                /* PA增益切换时序 */
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
 结构名    : UCOM_NV_CDMA_TX_BAND_TIMING_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : CDMA上行时序定义
*****************************************************************************/
typedef struct
{
    UCOM_NV_CDMA_PA_GAIN_SWITCH_TIMING_STRU stPaGainSwichTiming;                /* PA增益切换时序 */

    VOS_UINT16                              uhwPaOnAdvanceChip;
    VOS_UINT16                              uhwPaOffDelayChip;
    VOS_UINT16                              uhwAptPaVccTiming;                  /* vcc bias has the same timing */
    VOS_UINT16                              uhwAptTrig1Timing;
    VOS_UINT16                              uhwAptTrig2Timing;

    VOS_UINT16                              uhwEtMOnTiming;                     /* 提前打开ETM的chip数, ETM需要在PA之前打开 */
    VOS_UINT16                              uhwEtMOffTiming;                    /* 延后关闭ETM的chip数, ETM需要在PA之后关闭 */
    VOS_UINT16                              uhwEtMGainSwichTiming;              /* 切换ETM档位的chip数, ETM需要在PA档位之前配置 */

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
 结构名    : UCOM_NV_W_UL_CFR_GATE_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              uwDefaultCfrFirstGate;
    VOS_UINT32                              uwDefaultCfrSecondGate;
    VOS_UINT32                              uwDpdCfrFirstGate;
    VOS_UINT32                              uwDpdCfrSecondGate;
}UCOM_NV_W_UL_CFR_GATE_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_WCDMA_TX_MAXPOWER_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct
{
    VOS_INT16                                   shwTxMaxPowerRach;                             /* 三个频段，上行仅存在DPCH或存在RACH时UE可允许的最大发射功率，单位0.1dbm;第0列为RACH,第1列为DPCH, */
    VOS_INT16                                   shwTxMaxPowerDpch;                             /* 三个频段，上行仅存在DPCH或存在RACH时UE可允许的最大发射功率，单位0.1dbm;第0列为RACH,第1列为DPCH, */
    VOS_INT16                                   ashwTxMaxPowerTempComp[UCOM_NV_W_TEMP_NUM];    /* 最大发射功率补偿增益，单位0.1dbm;3个频段，5个预先定义的温度点:-20,0,20,40,60,第6个点用于对齐，不使用 */
}UCOM_NV_WCDMA_TX_MAXPOWER_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_WCDMA_TX_MAXPOWER_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct
{
    VOS_INT16                                   shwTxMinPower;                                  /* UE允许的最小发射功率，单位0.1dbm;3个频段，5个预先定义的温度点:-20,0,20,40,60,第6个点用于对齐，不使用 */
    VOS_INT16                                   shwRslv;                                        /* 保留 */
    VOS_INT16                                   ashwTxMinPowerTempComp[UCOM_NV_W_TEMP_NUM];     /* 最小发射功率温度补偿 */
}UCOM_NV_WCDMA_TX_MINPOWER_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_W_TX_POWER_STANDARD_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct
{
    UCOM_NV_WCDMA_TX_MAXPOWER_STRU              stTxWiredMaxPower;              /* 有线 */
    UCOM_NV_WCDMA_TX_MAXPOWER_STRU              stTxWirelessMaxPower;           /* 无线 */
    UCOM_NV_WCDMA_TX_MINPOWER_STRU              stTxMinPower;
}UCOM_NV_W_TX_POWER_STANDARD_STRU;


/*****************************************************************************
 结构名    : NV_GUC_BAND_UL_DCDC_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : DCDC控制参数
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwDcdcType              :2;             /* 0: PDM, 1:MIPI */
    VOS_UINT32                          uwDcdcPinCtrlEn         :2;             /* 是否使能DCDC GPIO PIN脚配置 */
    VOS_UINT32                          uwDcdcPinNum            :8;             /* 对应的PIN脚 0 - 31, FF为无效 */
    VOS_UINT32                          uwDcdcDefaultPdmValue   :8;             /* 不使能APT时，PDM方式时的默认配置值，
                                                                                   使能则调用APT表(上层不带配置值则取表中最大功率对应的值) */
    VOS_UINT32                          uwRsv0                  :4;             /* 保留 */
    VOS_UINT32                          uwRsv1                  :8;             /* 保留 */

    PHY_MIPIDEV_UNIT_STRU               astDcdcDefaultMipiWord[UCOM_NV_W_TX_DCDC_MIPI_NUM];              /* 不使能APT时，MIPI方式时的默认配置值，
                                                                                   使能则调用APT表(上层不带配置值则取表中最大功率对应的值) */

    VOS_UINT32                          uwRsv2;                                 /* 保留 */
    VOS_UINT32                          uwRsv3;                                 /* 保留 */
    VOS_UINT32                          uwRsv4;                                 /* 保留 */
} NV_W_UL_DCDC_PARA_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_W_UL_MIPI_VBIAS_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_CMD_STRU                astVbiasH2ULGain[UCOM_NV_PA_GAIN_MAX_NUM];
}UCOM_NV_W_UL_MIPI_VBIAS_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_W_ET_LUT_EXTRACT_ETM_CFG_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : ET LUT提取控制ETM的MIPI参数
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU               stPaVccCmd;                             /* ETM DAC1配置 */
    PHY_MIPIDEV_UNIT_STRU               stPaVbaisCmd;                           /* ETM DAC2配置 */
    PHY_MIPIDEV_UNIT_STRU               astEtLutEtmMipi[4];                     /* ETM MIPI改配，只在ET LUT提取时配置，
                                                                                   用于控制DAC1/DAC2分别输出  */
}UCOM_NV_W_ET_LUT_EXTRACT_ETM_CFG_STRU;

typedef UCOM_NV_W_ET_LUT_EXTRACT_ETM_CFG_STRU     UCOM_NV_CDMA_ET_LUT_EXTRACT_ETM_CFG_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_W_UL_DCDC_ET_APT_CFG_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :
*****************************************************************************/
typedef struct
{
    NV_W_UL_DCDC_PARA_STRU                  stDcdcPara; /* DCDC默认配置,非ET非APT模式下使用 */

    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptOn[UCOM_NV_ET_MIPI_NUM];    /* MIPI ET/APT On控制 */
    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptOff[UCOM_NV_ET_MIPI_NUM];   /* MIPI ET/APT OFF控制 */

    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptH[UCOM_NV_ET_MIPI_NUM];     /* MIPI ET/APT高增益控制字 */
    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptM[UCOM_NV_ET_MIPI_NUM];     /* MIPI ET/APT中增益控制字 */
    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptL[UCOM_NV_ET_MIPI_NUM];     /* MIPI ET/APT低增益控制字 */
    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptUL[UCOM_NV_ET_MIPI_NUM];    /* MIPI ET/APT超低增益控制字 */

    PHY_MIPIDEV_CMD_STRU                    astAptPaVccH2ULGain[UCOM_NV_PA_GAIN_MAX_NUM];/* 任意档位填写了MIPI,则表示该功能有效，
                                                                                            每时隙头发送，但ET模式下不会发送 */
    UCOM_NV_W_UL_MIPI_VBIAS_STRU            astMipiAptPaVbias[UCOM_NV_W_TX_PA_VBIAS_NUM];/* 任意档位填写了MIPI,则表示该功能有效，
                                                                                            每时隙头发送，ET模式下也会发送 */
    PHY_MIPIDEV_UNIT_STRU                   astMipiEtAptTrigger[UCOM_NV_ET_APT_TRIGGER_NUM];/* 填写了MIPI,则表示该功能有效，
                                                                                               每时隙头发送，ET模式下也会发送 */

    UCOM_NV_W_ET_LUT_EXTRACT_ETM_CFG_STRU   stEtLutExtMipiCfg;                      /* ET LUT提取控制ETM的MIPI参数，
                                                                                       若为trigger模式，则要包含trigger指令 */

}UCOM_NV_W_UL_DCDC_ET_APT_CFG_STRU;

typedef UCOM_NV_W_UL_DCDC_ET_APT_CFG_STRU     UCOM_NV_CDMA_UL_DCDC_ET_APT_CFG_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_WCDMA_RX_AGC_FREQ_COMP_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 需要校准的WCDMA的频点频率,boston及之后使用(因为RF变化)
*****************************************************************************/
typedef NV_RX_AGC_FREQ_COMP_CAL_STRU UCOM_NV_WCDMA_RX_AGC_FREQ_COMP_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_W_CLOSE_LOOP_TUNER_PARA_STRU
 结构说明  : 闭环tuner参数结构体
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              uwBandSupport;                      /* 对应bit为1表示这个band支持动态天线调谐,否则不支持 */
    VOS_UINT32                              uwBandExten;                        /* 以后band ID可能超出32 */
    VOS_UINT16                              uhwActiveTime;                      /* 天线调谐配置提前帧头配置量，当上行打开后，WBBP每次在帧头提前cpu_tx_sw_active_time进行配置，单位为chip */
    VOS_UINT16                              uhwSampleTime;                      /* 天线调谐平均检测次数,默认配置为4 */
    VOS_UINT16                              uhwDelayFrame;                      /* 完成一轮调谐后的等待时间，单位帧 */
    VOS_UINT16                              uhwEchoLossThreshold;               /* 天线调谐回波损耗门限,精度0.1dBm */
    VOS_UINT32                              uwForWardGpioWord;                  /* 驻波检测前向配置 */
    VOS_UINT32                              uwReverseGpioWord;                  /* 驻波检测反向配置 */
    PHY_MIPIDEV_CMD_STRU                    stTunerMipiCmd;                     /* MIPI控制字 */
    UCOM_NV_W_ANT_TUNER_CODE_STRU           stAntTunerCode;                     /* 天线调谐码字信息 */
} UCOM_NV_W_CLOSE_LOOP_TUNER_PARA_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_ANT_TUNER_CTRL_INFO_STRU
 结构说明  : 天线调谐的控制结构体,MIPI型tuner,考虑兼容多种型号，提供最多6个控制字
*****************************************************************************/
typedef struct
{
    UCOM_NV_TUNER_SUPPORT_MODE_ENUM_UINT16  enAntTunerMode;                     /* 0:硬件不支持，1:支持开环，2:支持AP+Sensor, 3:支持动态调谐 */
    UCOM_NV_RFFE_CTRL_ENUM_UINT16           enAntTunerCtrlType;                 /* 天线调谐是通过MIPI控制还是GPIO,0表示GPIO,1表示MIPI */
    PHY_MIPIDEV_UNIT_STRU                   stTunerActiveReg;                   /* tuner的公共初始化控制字，用于一些寄存器只需要模式加载配置一次 */
    PHY_MIPIDEV_UNIT_STRU                   stTunerIdleReg;                     /* 配置tuner为IDLE态或低功耗态的控制字 */
    UCOM_NV_W_CLOSE_LOOP_TUNER_PARA_STRU    stCloseLoopTunerCtrl;               /* 闭环tuner参数 */
} UCOM_NV_W_ANT_TUNER_CTRL_INFO_STRU;

typedef UCOM_NV_W_ANT_TUNER_CTRL_INFO_STRU UCOM_NV_CDMA_ANT_TUNER_CTRL_INFO_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_GUC_MIPI_INIT_WORD_STRU
 结构说明  : 初始化mipi器件的接口
*****************************************************************************/
typedef struct
{
   PHY_MIPIDEV_UNIT_STRU        astTxMipiInitWord[UCOM_NV_GUC_MIPI_INIT_UNIT_MAX_NUM];
}UCOM_NV_GUC_MIPI_INIT_WORD_STRU;
/*****************************************************************************
 结构名    : UCOM_NV_GSM_REDUCE_CURRENT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 二级限流
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          uhwReduceCurrentEn;                     /* 二级限流使能 */
    UCOM_NV_RFFE_CTRL_ENUM_UINT16       enCtrlMode;                             /* 控制方式MIPI或GPIO */
    VOS_UINT32                          uwGpioMask;                             /* 二级限流使用的管脚 */
    PHY_MIPIDEV_UNIT_STRU               stDefaultMipiData;                      /* 内含二级限流对应的PMU地址，缺省时的电流值(第一个时隙), */
    PHY_MIPIDEV_UNIT_STRU               stLimitMipiData;                        /* 内含二级限流对应的PMU地址，缺省时的电流值(第一个时隙) */
}UCOM_NV_GSM_REDUCE_CURRENT_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_MIPI_COMM_STRU
 协议表格  :
 结构说明  : 用于一些mipi功能公共指令定义
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU                   astMipiWord[UCOM_NV_MIPI_COMM_NUM];
}UCOM_NV_MIPI_COMM_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_DPD_LUT_ALG_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : DPD提取算法参数
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwDpdAlgSel             :2;             /* DPD算法选择0:数学特性计算DPD,1:物理特性计算DPD,默认值:1 */
    VOS_UINT32                          uwCwType                :4;             /* Sine单音表选择，0:1.92M正表,1:1.92M共轭表,
                                                                                   2:0.96M正表,3:0.96M共轭表,4:0.48M正表,5:0.48M共轭表
                                                                                   默认值:2 */
    VOS_UINT32                          uwDpdSmoothLen          :5;             /* 平滑长度,默认值:9 */
    VOS_UINT32                          uwPaGainCorrectFactor   :14;            /* LUT归一化相关值,默认值:1036 */
    VOS_UINT32                          uwProtectIndex          :7;             /* LUT表保护索引值,默认值:10 */

    VOS_UINT32                          uwStepDataLen           :12;            /* 一阶的采数长度，必须为一个单音周期点数的整数倍
                                                                                   默认值:512 */
    VOS_UINT32                          uwRedundantStepNum      :4;             /* 阶梯波形冗余阶数，用于找突变点,默认值:1 */
    VOS_INT32                           swStepVBackOff          :8;             /* STEP V回退值，默认值:2 */
    VOS_UINT32                          uwStepVDefault          :8;             /* STEP V初值，默认值: 90 */

    VOS_UINT32                          uwReserved0             :8;             /* 保留 */
    VOS_UINT32                          uwReserved1             :8;             /* 保留 */
    VOS_UINT32                          uwReserved2             :8;             /* 保留 */
    VOS_UINT32                          uwReserved3             :4;             /* 保留 */
    VOS_UINT32                          uwReserved4             :4;             /* 保留 */

}UCOM_NV_DPD_LUT_ALG_PARA_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_ET_LUT_ALG_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : ET提取算法参数
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwCwType                :4;             /* 0:1.92M正表,1:1.92M共轭表,
                                                                                   2:0.96M正表,3:0.96M共轭表,4:0.48M正表,5:0.48M共轭表
                                                                                   默认值:2 */
    VOS_UINT32                          uwStepDataLen           :12;            /* 一阶的采数长度，必须为一个单音周期点数的整数倍
                                                                                   默认值:512 */
    VOS_UINT32                          uwRedundantStepNum      :3;             /* 阶梯波形冗余阶数，用于找突变点,默认值:1 */
    VOS_UINT32                          uwProtectIdx            :4;             /* 保护索引值，前面几阶可能PA没导通,默认值:0 */
    VOS_UINT32                          uwIsoPowerOrGain        :1;             /* 是功率压缩还是增益压缩 0:增益压缩 1:功率压缩 */
    VOS_UINT32                          uwReserved1             :4;             /* 保留 */
    VOS_UINT32                          uwReserved2             :4;             /* 保留 */
}UCOM_NV_ET_LUT_ALG_PARA_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_CDMA_ET_LUT_ALG_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : ET提取算法参数  具体参数可能要改
*****************************************************************************/
typedef struct
{
    VOS_UINT32       uwCwType                :4;     /* 0:1.92M正表,    1:1.92M共轭表,
                                                        2:0.96M正表,    3:0.96M共轭表,
                                                        4:0.48M正表,    5:0.48M共轭表
                                                        默认值:2 */

    VOS_UINT32       uwStepDataLen           :12;    /* 一阶的采数长度，必须为一个单音周期点数的整数倍
                                                                                   默认值:512 */
    /* 冗余采样阶数，C模不需要 */
    VOS_UINT32       uwRedundantStepNum      :3;     /* 阶梯波形冗余阶数，用于找突变点,默认值:2,C模不需要 */

    /* 冗余采样阶数，C模不需要 */
    VOS_UINT32       uwProtectIdx            :4;     /* 保护索引值，前面几阶可能PA没导通,默认值:2,C模不需要 */

    VOS_UINT32       uwIsoPowerOrGain        :1;     /* 保留 */
    VOS_UINT32       uwReserved1             :4;     /* 保留 */
    VOS_UINT32       uwReserved2             :4;     /* 保留 */
}UCOM_NV_CDMA_ET_LUT_ALG_PARA_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_W_ET_DELAY_CAL_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : Et Delay校准信息
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwCalAlgSel                 : 2;        /* 校准方法选择，默认值2:
                                                                                   0:二分加遍历，中心值+-uhwSearchRangeOffset的范围二分，
                                                                                     到uhwSweepRange范围遍历
                                                                                   1:遍历，中心值+-uhwSearchRangeOffset的范围，步进为uhwSweepStep
                                                                                   2:三次曲线拟合找最底点，中心值+-uhwSearchRangeOffset的范围
                                                                                     平分为uhwPartNum段 */
    VOS_UINT32                          uwSampleLen2PowerN          : 4;        /* 采数长度1<<N，必须为256的整数倍，默认值10 */
    VOS_UINT32                          uwPartNum                   : 5;        /* 搜索区间等分个数，用这些点拟合曲线,有效值4-31 */
    VOS_INT32                           swEtLutBackoff              : 6;        /* ET LUT查表回退，实际补在温补上,单位0.1dB，默认值0 */
    VOS_UINT32                          uwCenterEnvelopTimeDelay    : 15;       /* delay的中心值，默认值27100 */

    VOS_UINT32                          uwSearchRangeOffset         : 11;       /* 搜索的offset，默认值1000 */
    VOS_UINT32                          uwSweepOffset               : 14;       /* 二分搜索收敛到该范围*2后开始按步长遍历，默认值200 */
    VOS_UINT32                          uwSweepStep                 : 7;        /* 扫描步长，若uwSweepStep > uwSweepRange则不会扫描,默认值5 */

    VOS_UINT32                          uwAcFreq                    : 16;       /* 邻道积分的中心频率，单位KHz，默认值5000 */
    VOS_UINT32                          uwIntegralBW                : 16;       /* 计算ACLR积分带宽，单位KHz，默认值3840 */

    VOS_INT32                           swAdcOutdBFS                : 8;        /* ADC输入的期望dBFS，单位0.1，默认值-80 */
    VOS_UINT32                          uwRslv1                     : 8;        /* 保留 */
    VOS_UINT32                          uwRslv2                     : 8;        /* 保留 */
    VOS_UINT32                          uwRslv3                     : 8;        /* 保留 */

}UCOM_NV_W_ET_DELAY_CAL_PARA_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_CDMA_ET_DELAY_CAL_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : Et Delay校准参数  for boston
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwCalAlgSel                 : 2;        /* 校准方法选择，默认值2:
                                                                                   0:二分加遍历，中心值+-uhwSearchRangeOffset的范围二分，
                                                                                     到uhwSweepRange范围遍历
                                                                                   1:遍历，中心值+-uhwSearchRangeOffset的范围，步进为uhwSweepStep
                                                                                   2:三次曲线拟合找最底点，中心值+-uhwSearchRangeOffset的范围
                                                                                     平分为uhwPartNum段 */
    VOS_UINT32                          uwSampleLen2PowerN          : 4;        /* 采数长度1<<N，必须为256的整数倍，默认值10 */
    VOS_UINT32                          uwPartNum                   : 5;        /* 搜索区间等分个数，用这些点拟合曲线,有效值4-31 */
    VOS_INT32                           swEtLutBackoff              : 6;        /* ET LUT查表回退，实际补在温补上,单位0.1dB，默认值0 */
    VOS_UINT32                          uwCenterEnvelopTimeDelay    : 15;       /* delay的中心值，默认值896 */

    VOS_UINT32                          uwSearchRangeOffset         : 11;       /* 搜索的offset，默认值500 */
    VOS_UINT32                          uwSweepOffset               : 14;       /* 二分搜索收敛到该范围*2后开始按步长遍历，默认值200 */
    VOS_UINT32                          uwSweepStep                 : 7;        /* 扫描步长，若uwSweepStep > uwSweepRange则不会扫描,默认值5 */

    VOS_UINT32                          uwAcFreq                    : 16;       /* 邻道积分的中心频率，单位KHz，默认值1230 */
    VOS_UINT32                          uwIntegralBW                : 16;       /* 计算ACLR积分带宽，单位KHz，默认值1000 */

    VOS_INT32                           swAdcOutdBFS                : 8;        /* ADC输入的期望dBFS，单位0.1，默认值-80 */
    VOS_UINT32                          uwRslv1                     : 8;        /* 保留 */
    VOS_UINT32                          uwRslv2                     : 8;        /* 保留 */
    VOS_UINT32                          uwRslv3                     : 8;        /* 保留 */

}UCOM_NV_CDMA_ET_DELAY_CAL_PARA_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_CDMA_ET_DPD_LUT_SELF_CAL_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : CDMA ET校准控制参数
*****************************************************************************/
typedef struct
{
    VOS_INT16                                   ashwDpdPowerBackOff[4];         /* DPD校准的回退功率，单位0.1dB，默认值30 */
    UCOM_NV_DPD_LUT_ALG_PARA_STRU               stDpdAlgPara;                   /* DPD提取算法参数 */
    UCOM_NV_CDMA_ET_LUT_ALG_PARA_STRU           stEtLutAlgPara;                 /* ET提取算法参数 */
    UCOM_NV_CDMA_ET_DELAY_CAL_PARA_STRU         stEtDelayCalPara;               /* Et Delay校准控制参数 */

}UCOM_NV_CDMA_ET_DPD_LUT_SELF_CAL_PARA_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_ET_DPD_LUT_SELF_CAL_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : ET/DPD校准控制参数
*****************************************************************************/
typedef struct
{
    VOS_INT16                               ashwDpdPowerBackOff[4];             /* DPD校准的回退功率，单位0.1dB，默认值30 */
    UCOM_NV_DPD_LUT_ALG_PARA_STRU           stDpdAlgPara;                       /* DPD提取算法参数 */
    UCOM_NV_ET_LUT_ALG_PARA_STRU            stEtAlgPara;                        /* ET提取算法参数 */
    UCOM_NV_W_ET_DELAY_CAL_PARA_STRU        stEtDelayCalPara;                   /* Et Delay校准控制参数 */
}UCOM_NV_ET_DPD_LUT_SELF_CAL_PARA_STRU;



/*****************************************************************************
 结构名    : UCOM_NV_GSM_CLOSE_MIPI_PA_VCC_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : close vcc control
*****************************************************************************/
typedef struct
{
  PHY_MIPIDEV_UNIT_STRU   astMipiClosePaVcc[UCOM_NV_G_PAVCC_MIPI_UNIT_MAX_NUM];
}UCOM_NV_GSM_CLOSE_MIPI_PA_VCC_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_G_PA_VCC_MIPI_CMD_FRAME_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 打开PaVcc的mipi指令帧
*****************************************************************************/
typedef struct
{
    PHY_MIPIDEV_UNIT_STRU               astMipiPaVcc[UCOM_NV_G_PAVCC_MIPI_UNIT_MAX_NUM];
}UCOM_NV_G_PA_VCC_MIPI_CMD_FRAME_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_ANT_TUNER_CTRL_INFO_STRU
 结构说明  : 天线调谐的控制结构体,MIPI型tuner,考虑兼容多种型号，提供最多6个控制字
*****************************************************************************/
typedef struct
{
    UCOM_NV_TUNER_SUPPORT_MODE_ENUM_UINT16  enAntTunerMode;                     /* 硬件是否支持天线调谐，0不支持，1支持 */
    UCOM_NV_RFFE_CTRL_ENUM_UINT16           enAntTunerCtrlType;                 /* 天线调谐是通过MIPI控制还是GPIO,0表示GPIO,1表示MIPI */
    UCOM_NV_GSM_TUNER_GPIO_MASK_STRU        stTunerGpioMask;
    PHY_MIPIDEV_UNIT_STRU                   astTunerActiveReg[UCOM_NV_G_TUNER_MIPI_UNIT_MAX_NUM];  /* tuner的公共初始化控制字，用于一些寄存器只需要模式加载配置一次 */
    PHY_MIPIDEV_UNIT_STRU                   astTunerIdleReg[UCOM_NV_G_TUNER_MIPI_UNIT_MAX_NUM];    /* 配置tuner为IDLE态或低功耗态的控制字 */
} UCOM_NV_G_ANT_TUNER_CTRL_INFO_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GSM_NOTCH_CHAN_CTRL_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 天线开关滤波器通道的控制
*****************************************************************************/
typedef struct
{
    VOS_UINT16                              uhwNotchSupportFlag;                /* DCS1800 TX NOTCH开关支持标志，1表示支持，0表示不支持 */
    VOS_UINT16                              uhwNotchControlMode;                /* NOTCH控制方式:1表示GPIO模式，2表示Mipi方式，3表示GPIO+MIPI组合方式 */
    VOS_UINT32                              uwNotchGpioSetValue;                /* Nothc GPIO控制值，uhwNotchControlMode=1或者3时有效 */
    VOS_UINT32                              uwNotchGpioDefaultValue;            /* Nothc GPIO控制值，uhwNotchControlMode=1或者3时有效 */
    PHY_MIPIDEV_UNIT_STRU                   astNotchMipiNew[UCOM_NV_G_NOTCH_MIPI_UNIT_MAX_NUM]; /* 天线NOTCH通道MIPI接口控制地址 */
    UCOM_NV_GSM_NOTCH_POWER_COMP_STRU       stNotchPowerComp;                   /* Notch发射功率补偿 */

 }UCOM_NV_GSM_NOTCH_CHAN_CTRL_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_RF_RFIC_INIT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : RFIC初始化寄存器
*****************************************************************************/
typedef   NV_MODEM_RF_RFIC_INIT_STRU       UCOM_NV_GUC_RF_RFIC_INIT_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_TEMP_DEFINE_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :温度表定义
*****************************************************************************/
typedef   NV_MODEM_TEMP_DEFINE_STRU        UCOM_NV_GUC_TEMP_DEFINE_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :单个频点的温度补偿
*****************************************************************************/
typedef struct
{
    VOS_UINT32                              ulFreq;                              /* 当前频率,GUTL单位是100KHz,C模单位是KHz*/
    VOS_INT8                                acTempCompValue[MAX_TEMP_DEFINE_NUM];/* TX的温补单位0.1dB，RX的温补单位0.125dB */
}UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_TEMP_COMP_SINGEL_LEVEL_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 每档PA GAIN对应的温度补偿
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* 频点个数,目前GUC最大支持3个频点 */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_TRX_TEMP_COMP_FREQ_NUM];
}UCOM_NV_TEMP_COMP_SINGEL_LEVEL_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_RF_RFIC_INIT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 4档PA GAIN对应的温度补偿
*****************************************************************************/
typedef struct
{
    UCOM_NV_TEMP_COMP_SINGEL_LEVEL_STRU    astTxTempComp[UCOM_NV_PA_GAIN_MAX_NUM];/* 0--高增益,1--中增益,2--低增益,3--超低增益 */
}UCOM_NV_GUC_TX_TEMP_COMP_STRU;


/*****************************************************************************
 结构名    : UCOM_NV_RX_TEMP_COMP_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  :RX AGC对应的温度补偿
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* 频点个数 */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_TRX_TEMP_COMP_FREQ_NUM];
}UCOM_NV_GUC_RX_TEMP_COMP_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_MRX_TEMP_COMP_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : MRX对应的温度补偿
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* 频点个数 */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_TRX_TEMP_COMP_FREQ_NUM];
}UCOM_NV_GUC_MRX_TEMP_COMP_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_APT_TEMP_COMP_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : APT/ET对应的温度补偿
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* 频点个数 */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_W_APT_FREQ_COMP_NUM];
}UCOM_NV_GUC_APT_TEMP_COMP_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_DPD_TEMP_COMP_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : DPD对应的温度补偿
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* 频点个数 */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_W_DPD_FREQ_COMP_NUM];
}UCOM_NV_GUC_DPD_TEMP_COMP_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_ET_TIMEDELAY_TEMP_COMP_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : ET TIME DELAY对应的温度补偿
*****************************************************************************/
typedef struct
{
    VOS_UINT16                             usFreqNum;                           /* 频点个数，不区分信道，所以应该为1 */
    VOS_UINT16                             usRsv;
    UCOM_NV_TEMP_COMP_SINGLE_FREQ_STRU    astTempComp[UCOM_NV_ET_DELAY_TEMP_COMP_MAX_NUM]; /* 不区分信道，大小为1 */
}UCOM_NV_GUC_ET_TIME_DELAY_TEMP_COMP_STRU;

/*****************************************************************************
 结构名    : NV_MODEM_EXT_LNA_POR_INIT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : EXT_LNA初始化结构体
*****************************************************************************/
typedef  NV_MODEM_RF_6H0X_INIT_REG_INFO_STRU UCOM_NV_GUC_RF_6H0X_POR_INIT_STRU;

/*****************************************************************************
 结构名    : NV_MODEM_RF_6H0X_INIT_TABLEIDX_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : EXT LNA初始化寄存器索引
*****************************************************************************/
typedef  NV_MODEM_RF_6H0X_INIT_TABLEIDX_STRU UCOM_NV_GUC_RF_6H0X_INIT_TABLEIDX_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GUC_INETRFERENCE_TIMING_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : Rx Blank Delay
*****************************************************************************/
typedef struct
{
    VOS_UINT8                             ucGBlankWRxDelay;                     /* 单位chip,按照u模chip时间计算:1chip=(10000/38400)us */
    VOS_UINT8                             ucGBlankCRxDelay;                     /* 单位chip,按照c模chip时间计算:1chip=(1250/1536)us */
    VOS_UINT8                             ucRsv13;                              /* 预留 */
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
 结构名    : UCOM_NV_RFIC_SETTING_MIPI_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 每组Rf Setting寄存器个数，与射频确认3个mipi寄存器
*****************************************************************************/
typedef struct
{
    NV_RF_MIPI_INIT_STRU                astReg[UCOM_NV_RFIC_SETTING_REG_NUM];   /* 每组Rf Setting寄存器个数，3个 */
}UCOM_NV_RFIC_SETTING_MIPI_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GSM_RFIC_SETTING_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : RFIC SETTING值(以High Gain为例，最多10组，需要校准选出最优)
*****************************************************************************/
typedef struct
{

    PHY_UINT16                          usValidListNum;                         /* Rf Setting组数 */
    PHY_UINT16                          usRsv;                                  /* 保留位 */
    UCOM_NV_RFIC_SETTING_MIPI_STRU      astRfSettingList[UCOM_NV_RFIC_SETTING_LIST_NUM]; /* Rf Setting值，最多10组 */
}UCOM_NV_GSM_RFIC_SETTING_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GSM_RFIC_SETTING_LIST_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : RFIC SETTING表，分High Gain和Low Gain
*****************************************************************************/
typedef struct
{
    UCOM_NV_GSM_RFIC_SETTING_STRU       stHighGain;                             /* High Gain Para */
    UCOM_NV_GSM_RFIC_SETTING_STRU       stLowGain;                              /* Low Gain Para */
}UCOM_NV_GSM_RFIC_SETTING_LIST_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GSM_RFIC_SETTING_PARA_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : RFIC SETTING相关参数(Vdac值和功率门限)，分High Gain和Low Gain
*****************************************************************************/
typedef struct
{
    PHY_UINT16                          usDacTargetHigh;                        /* High Gain Vdac值 */
    PHY_UINT16                          usPowerLimit10thHigh;                   /* High Gain 功率门限，单位0.1dBm */
    PHY_UINT16                          usDacTargetLow;                         /* Low Gain Vdac值 */
    PHY_UINT16                          usPowerLimit10thLow;                    /* Low Gain 功率门限，单位0.1dBm */
}UCOM_NV_GSM_RFIC_SETTING_PARA_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GSM_RFIC_SETTING_CAL_RESULT_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : 校准nv，RFIC SETTING最优值，分High Gain和Low Gain
*****************************************************************************/
typedef struct
{
    UCOM_NV_RFIC_SETTING_MIPI_STRU      stHighGain;                             /* High Gain Rf Setting校准值 */
    UCOM_NV_RFIC_SETTING_MIPI_STRU      stLowGain;                              /* Low Gain Rf Setting校准值 */
}UCOM_NV_GSM_RFIC_SETTING_CAL_RESULT_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_WG_READBACK_MIPI_STRU
 协议表格  :
 ASN.1描述 :
 结构说明  : gu MIPI回读控制和数据
*****************************************************************************/
typedef struct
{
    PHY_UINT16                      uhwCycMipiRdEn;                             /* 周期性MIPI回读功能是否使能 */
    PHY_UINT16                      uhwRsv;

    PHY_MIPIDEV_CMD_STRU            astRficMipi[UCOM_NV_RDBACK_MIPI_NUM];       /* 定制的周期性读取的RFIC mipi */
    PHY_MIPIDEV_CMD_STRU            astRffeMipi[UCOM_NV_RDBACK_MIPI_NUM];       /* 定制的周期性读取的RFFE mipi */
}UCOM_NV_WG_READBACK_MIPI_STRU;

/*****************************************************************************
 结构名    : UCOM_NV_GSM_RAMP_COEFF_REF_STRU
 协议表格  :
 结构说明  : GSM上爬坡表参考系数，16个点
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          auhwRampUp[UCOM_NV_G_RAMP_COEF_NUM];    /* 爬坡参考系数，扩大1024倍 */
}UCOM_NV_GSM_RAMP_COEFF_REF_STRU;

/*****************************************************************************
  7 UNION定义
*****************************************************************************/


/*****************************************************************************
  8 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  9 全局变量声明
*****************************************************************************/


/*****************************************************************************
  10 函数声明
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

