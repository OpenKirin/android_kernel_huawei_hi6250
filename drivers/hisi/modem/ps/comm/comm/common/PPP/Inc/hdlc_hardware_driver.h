/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __HDLC_HRADWARE_DRIVER_H__
#define __HDLC_HRADWARE_DRIVER_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "hdlc_hardware_mntn.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/

#if ((SC_CTRL_MOD_6250_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_3660_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_KIRIN970_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_M533 == SC_CTRL_MOD))
/* =========dallas/chicago/boston��ʹ��: HDLCʱ���ڼĴ���CRG_CLKEN4 �� CRG_CLKDIS4 24bit��λ��======== */
#define     HDLC_CRG_CLK_BITPOS                                 (24UL)
#else
/* =========V7R2��ʹ��: HDLCʱ���ڼĴ���CRG_CLKEN4 �� CRG_CLKDIS4 10bit��λ��======== */
#define     HDLC_CRG_CLK_BITPOS                                 (10UL)
#endif

#if (SC_CTRL_MOD_P532 == SC_CTRL_MOD)
#define     HDLC_CRG_CLKEN_OFFSET                               (0x24)      /* HDLCʱ��ʹ��OFFSET */
#define     HDLC_CRG_CLKDIS_OFFSET                              (0x28)      /* HDLCʱ�ӹر�OFFSET */
#define     HDLC_CRG_CLKSTAT_OFFSET                             (0x2C)      /* HDLCʱ��״̬OFFSET */
#elif (SC_CTRL_MOD_6932_SFT == SC_CTRL_MOD)
#define     HDLC_CRG_CLKEN_OFFSET                               (0x30)      /* HDLCʱ��ʹ��OFFSET */
#define     HDLC_CRG_CLKDIS_OFFSET                              (0x34)      /* HDLCʱ�ӹر�OFFSET */
#define     HDLC_CRG_CLKSTAT_OFFSET                             (0x38)      /* HDLCʱ��״̬OFFSET */
#elif ((SC_CTRL_MOD_6250_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_3660_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_KIRIN970_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_M533 == SC_CTRL_MOD))
/* =========dallas/chicago/boston��ʹ��: HDLCʱ��ʹ�ܿ��ƼĴ���MDM_CRG_CLKEN0��
                          HDLCʱ�ӹرտ��ƼĴ���MDM_CRG_CLKDIS0��
                          HDLCʱ�ӿ���״̬MDM_CRG_CLKSTAT0��Ի���ַ��ƫ��======== */
#define     HDLC_CRG_CLKEN_OFFSET                               (0x00)      /* HDLCʱ��ʹ��OFFSET */
#define     HDLC_CRG_CLKDIS_OFFSET                              (0x04)      /* HDLCʱ�ӹر�OFFSET */
#define     HDLC_CRG_CLKSTAT_OFFSET                             (0x08)      /* HDLCʱ��״̬OFFSET */
#else
#define     HDLC_CRG_CLKEN_OFFSET                               (0x30)      /* HDLCʱ��ʹ��OFFSET */
#define     HDLC_CRG_CLKDIS_OFFSET                              (0x34)      /* HDLCʱ�ӹر�OFFSET */
#define     HDLC_CRG_CLKSTAT_OFFSET                             (0x38)      /* HDLCʱ��״̬OFFSET */
#endif


/* =========ʱ��ʹ�ܼĴ���======== */
#define     HDLC_CRG_CLKEN4_ADDR(base)                          ((base) + (HDLC_CRG_CLKEN_OFFSET))
/* =========ʱ�ӹرռĴ���======== */
#define     HDLC_CRG_CLKENDIS4_ADDR(base)                       ((base) + (HDLC_CRG_CLKDIS_OFFSET))
/* =========ʱ��״̬�Ĵ���======== */
#define     HDLC_CRG_CLKSTA4_ADDR(base)                         ((base) + (HDLC_CRG_CLKSTAT_OFFSET))

/* definition of bit positions and masks in a register */
/* ========================= 1.hdlc_frm_en (0x10) begin ======================= */
/*
   ����Ĵ�����32bit Reg�е�bitλ��:
        �Ĵ���frm_en��bit0;
*/
#define     HDLC_FRM_EN_BITPOS                                  (0UL)

/* ========================= 2.hdlc_def_en (0x60) begin ======================= */
/*
   ����Ĵ�����32bit Reg�е�bitλ��:
        �Ĵ���def_en��bit0;
*/
#define     HDLC_DEF_EN_BITPOS                                  (0UL)

/* ========================= 3.hdlc_frm_cfg (0x20) begin ======================= */
/*
   ����Ĵ�����32bit Reg�е�bitλ��:
        �Ĵ���frm_in_lli_1dor2d��bit0;
        �Ĵ���frm_acfc��bit1;
*/
#define     HDLC_FRM_IN_LLI_1DOR2D_BITPOS                       (0UL)
#define     HDLC_FRM_ACFC_BITPOS                                (1UL)
#define     HDLC_FRM_PFC_BITPOS                                 (2UL)

/*
   �Ĵ���frm_pfc(2bits)��ֵ����:
        00b: Ӳ��ģ�����P��, P����ѹ��;
        01b: Ӳ��ģ�����P��, P��ѹ��;
        11b: Ӳ��ģ�鲻���P��;
*/
#define     HDLC_PROTOCOL_ADD_WITHOUT_COMPRESS                  (0x00)
#define     HDLC_PROTOCOL_ADD_WITH_COMPRESS                     (0x01)
#define     HDLC_PROTOCOL_NO_ADD                                (0x03)

/* ========================= 4.hdlc_def_cfg (0x70) begin ======================= */
/*
   ����Ĵ�����32bit Reg�е�bitλ��:
        �Ĵ���def_uncompleted_ago��bit0;
        �Ĵ���def_acfc��bit1;
*/
#define     HDLC_DEF_IS_UNCOMPLETED_AGO_BITPOS                  (0UL)
#define     HDLC_DEF_ACFC_BITPOS                                (1UL)
#define     HDLC_DEF_PFC_BITPOS                                 (2UL)


/* �Ĵ���frm_protocol(16bits)��MASK�� */
#define     HDLC_FRM_PROTOCOL_MASK                              (0x0000FFFF)

/*
   �Ĵ���def_pfc(2bits)��ֵ����:
        00b: P����ѹ��, �����;
        01b: P��ѹ��, �����;
        11b: P�򲻰���;
*/
#define     HDLC_PROTOCOL_REMOVE_WITHOUT_COMPRESS               (0x00)
#define     HDLC_PROTOCOL_REMOVE_WITH_COMPRESS                  (0x01)
#define     HDLC_PROTOCOL_NO_REMOVE                             (0x03)

/* ========================= hdlc_prior_ctrl (0x04) begin ======================= */
/*
   �Ĵ���hdlc_prior_ctrl(2bits)��ֵ����:
        00b: ˭�ȱ�ʹ�����Ƚ�˭������;
        01b: ���з�װ���ȼ���;
        10b: ���н��װ���ȼ���;
        11b: ��Ч;
*/
#define     HDLC_PRIOR_FCFS               (0x00)
#define     HDLC_PRIOR_DL_FRM_HIGHER      (0x01)
#define     HDLC_PRIOR_UL_DEF_HIGHER      (0x02)
#define     HDLC_PRIOR_INVALID            (0x03)


/* Boston�汾�����÷�װ���װ��Ϣ����󳤶ȼĴ���0x5DF(1503)Bytes */
#define     HDLC_MAX_FRM_DEF_INFO_LEN                           (0x000005DFU)

/* ====================== ��װ״̬�Ĵ�����ַ hdlc_frm_status (0x28) begin ====================== */
/* ��װģ��״̬�Ĵ�����ַ: ���ƫ�Ƶ�ַ��0x28 */

/* �Ĵ���frm_out_seg_num(16bits)��MASK�� */
#define     HDLC_FRM_OUT_SEG_NUM_MASK                       (0x0000FFFF)

/* �Ĵ���frm_valid_num(8bits)��MASK�� */
#define     HDLC_FRM_VALID_NUM_MASK                         (0x000000FF)

/* �Ĵ���frm_all_pkt_done(1bit)��MASK�� */
#define     HDLC_FRM_ALL_PKT_DONE_MASK                      (0x00000001)

/* ��ѯ��װ״̬��Ϣ������ */
#define     HDLC_FRM_STATUS_MASK                            (0x00000003)

/*
   �Ĵ���frm_block_done([0]λ)��ֵ����:
        0b: δ���һ���������ݴ���;
        1b: ���һ���������ݴ���;
*/
/*
   �Ĵ���frm_error_index([1]λ)��ֵ����:
        1b: ��װ��������;
*/
#define     HDLC_FRM_ALL_PKT_DOING                              (0x00)
#define     HDLC_FRM_ALL_PKT_DONE                               (0x01)
#define     HDLC_FRM_STOP_EXCEPTION_OCCUR                       (0x02)
#define     HDLC_FRM_DONE_EXCEPTION_OCCUR                       (0x03)




/* ======================= ���װ״̬�Ĵ�����ַdlc_def_status (0x88) begin ===================== */
/* �Ĵ���dlc_def_status(5bits)��MASK�� */
#define     HDLC_DEFRAMER_BLOCK_STATUS_MASK                     (0x0000001B)

/* ��ѯ���װ״̬��Ϣ������ */
#define     HDLC_DEF_STATUS_MASK                                (0x0000003B)

/*
   �Ĵ���dlc_def_status(5bits)��ֵ����:
        000000b: δ���һ�����ݴ���;
        010000b: δ���һ�����ݴ������װ�ⲿ��ȷ֡��Ϣ�ϱ��ռ������ͣ;
        001000b: δ���һ�����ݴ���, ���װ�ⲿ����洢�ռ������ͣ;
        000001b: δ���һ�����ݴ���, �ѽ��LCP֡, Ӳ��������ͣ״̬;
        000010b: ���һ�����ݴ���, ����֡�ϱ�;
        000011b: ���һ�����ݴ���, ����֡�ϱ�;
        1xx0xxb: ���쳣����;
*/
#define     HDLC_DEF_STATUS_DOING                               (0x00)
#define     HDLC_DEF_STATUS_PAUSE_RPT_SPACE_FULL                (0x10)
#define     HDLC_DEF_STATUS_PAUSE_OUTPUT_SPACE_FULL             (0x08)
#define     HDLC_DEF_STATUS_PAUSE_LCP                           (0x01)
#define     HDLC_DEF_STATUS_DONE_WITHOUT_FRAMES                 (0x02)
#define     HDLC_DEF_STATUS_DONE_WITH_FRAMES                    (0x03)



/* ���װ�Ƿ��з�����֡��ʶ */
#define     HDLC_DEF_UNCOMPLETED_NOT_EXIST                      (0x0)
#define     HDLC_DEF_UNCOMPLETED_EXIST                          (0x1)

/* ��ѯ��װ����װ��ɵĴ��� */
#define     HDLC_FRM_MAX_WAIT_RESULT_NUM                        (0xFFFF)
#define     HDLC_DEF_MAX_WAIT_RESULT_NUM                        (0xFFFF)

/* �ȴ���װ����װ��Ӧ�ж�ʱ��,�Ժ���Ϊ��λ */
#define     HDLC_FRM_MASTER_INT_TIMER_LEN                       (5000)
#define     HDLC_DEF_MASTER_INT_TIMER_LEN                       (5000)

/* ������ѯ�����ж�ʹ�ܵ�ˮ�ߣ�Ĭ��ֵ */
#define     HDLC_DEF_INTERRUPT_LIMIT_DEFAULT                    (2*1024)
#define     HDLC_FRM_INTERRUPT_LIMIT_DEFAULT                    (2*1024)

/* AXI���߶�д����ʱ���ж�ֵ����ֵ��SoC�ṩ������������� */
#define     HDLC_AXI_REQ_TIMEOUT_VALUE                          (255)

/* HDLC��ַ������ѹ�� */
#define     HDLC_ADDRESS_CTRL_NOCOMPRESS            (0)
#define     HDLC_ADDRESS_CTRL_COMPRESS              (1)


/* HDLCӲ�����װ����zָʾBITλ */
#define PPP_DRIVER_HDLC_HARD_DEF_FCS_ERR            (0)
#define PPP_DRIVER_HDLC_HARD_DEF_FRAME_TOO_LONG     (1)
#define PPP_DRIVER_HDLC_HARD_DEF_FRAME_TOO_SHORT    (2)
#define PPP_DRIVER_HDLC_HARD_DEF_PROTOCOL_ERR       (3)
#define PPP_DRIVER_HDLC_HARD_DEF_CTRL_ERR           (4)
#define PPP_DRIVER_HDLC_HARD_DEF_ADDR_ERR           (5)
#define PPP_DRIVER_HDLC_HARD_DEF_FLAG_POS_ERR       (6)


/* ��ȡHDLC DRIVER������Ϣ */
#define PPP_DRIVER_HDLC_HARD_GET_CONFIG             (&g_stHdlcConfigInfo)

/* ��ȡHDLC DRIVER��װ������Ϣ */
#define PPP_DRIVER_HDLC_HARD_GET_FRM_CONF           (&(PPP_DRIVER_HDLC_HARD_GET_CONFIG->stFrmConfig))

/* ��ȡHDLC DRIVER���װ������Ϣ */
#define PPP_DRIVER_HDLC_HARD_GET_DEF_CONF           (&(PPP_DRIVER_HDLC_HARD_GET_CONFIG->stDefConfig))

/* ��ȡϵͳ����������ַ */
#define PPP_DRIVER_HDLC_HARD_GET_SC_BASE_ARRD       (PPP_DRIVER_HDLC_HARD_GET_CONFIG->ulHdlcScCtrlBaseAddr)

/* ������ѯ�����ж�ʹ�ܵ�ˮ�� */
#define     HDLC_DEF_INTERRUPT_LIMIT                            (PPP_DRIVER_HDLC_HARD_GET_CONFIG->ulHdlcDefIntLimit)
#define     HDLC_FRM_INTERRUPT_LIMIT                            (PPP_DRIVER_HDLC_HARD_GET_CONFIG->ulHdlcFrmIntLimit)

/* HDLC IP����ַ */
#define     HDLC_IP_BASE_ADDR                                   (PPP_DRIVER_HDLC_HARD_GET_CONFIG->ulHDLCIPBaseAddr)



/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/
/** ****************************************************************************
 * Name        : PPP_DRIVER_HDLC_REG_ENUM_UINT8
 *
 * Description :
 *******************************************************************************/
enum PPP_DRIVER_HDLC_REG_ENUM
{
    PPP_DRIVER_MAX_FRM_INFO_REG                 = 0x00,
    PPP_DRIVER_MAX_DEF_INFO_REG                 = 0x01,

    PPP_DRIVER_HDLC_REG_BUTT
};
typedef VOS_UINT8 PPP_DRIVER_HDLC_REG_ENUM_UINT8;

/*****************************************************************************
  4 ȫ�ֱ�������
*****************************************************************************/

/*****************************************************************************
  5 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  6 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  7 STRUCT����
*****************************************************************************/

/*****************************************************************************
 �ṹ��    : PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : һ�����ݽ���Ĵ���֡��ͳ�ƽṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usFCSErrCnt;          /* FCS���� */
    VOS_UINT16                          usLenLongCnt;         /* ֡���ȹ���, ����1502�ֽ� */
    VOS_UINT16                          usLenShortCnt;        /* ֡���ȹ���, ����4�ֽ� */
    VOS_UINT16                          usErrProtocolCnt;     /* Э���ֶβ�����xxxxxxx0, xxxxxxx1����ʽ */
    VOS_UINT16                          usErrCtrlCnt;         /* �����ֶβ�Ϊ0xFF */
    VOS_UINT16                          usErrAddrCnt;         /* ��ַ�ֶβ�Ϊ0x03 */
    VOS_UINT16                          usFlagPosErrCnt;      /* 0x7D�������0x7E */
    VOS_UINT8                           ucErrType;            /* �������� */
    VOS_UINT8                           ucReserve;
}PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU;

/*****************************************************************************
 �ṹ��    : PPP_DRIVER_HDLC_HARD_FRM_CONFIG_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC��װ���������Ϣ�ṹ
*****************************************************************************/
typedef struct
{
    VOS_VOID                           *pInputAddr;             /* ���������ַ */
    VOS_VOID                           *pOutputAddr;            /* ��������ַ */
    VOS_VOID                           *pReportAddr;            /* ��װ����ϱ���ַ */
    VOS_UINT32                          ulRptBufLen;            /* �ϱ��ռ��С */
}PPP_DRIVER_HDLC_HARD_FRM_CONFIG_STRU;

/*****************************************************************************
 �ṹ��    : PPP_DRIVER_HDLC_HARD_DEF_CONFIG_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC���װ���������Ϣ�ṹ
*****************************************************************************/
typedef struct
{
    VOS_VOID                           *pInputAddr;             /* ���������ַ */
    VOS_VOID                           *pOutputAddr;            /* ��������ַ */
    VOS_VOID                           *pReportAddr;            /* ��װ����ϱ���ַ */
    VOS_UINT32                          ulOutputBufLen;         /* ����ռ��С */
    VOS_UINT32                          ulReportBufLen;         /* �ϱ��ռ��С */
    VOS_UINT32                          ulPerInMaxLen;          /* ����������󵥰����� */
}PPP_DRIVER_HDLC_HARD_DEF_CONFIG_STRU;


/*****************************************************************************
 �ṹ��    : PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC���������Ϣ�ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT_PTR                        ulHDLCIPBaseAddr;      /* �ӵ����ȡ��HDLC����ַ */
    VOS_UINT_PTR                        ulHdlcScCtrlBaseAddr;  /* ϵͳ����������ַ */
    VOS_UINT_PTR                        ulHdlcDefMasterSem;    /* ���װ�ź��� */
    VOS_UINT_PTR                        ulHdlcFrmMasterSem;    /* ��װ�ź��� */
    VOS_INT32                           slHdlcISRDef;          /* ���װ�жϺ� */
    VOS_INT32                           slHdlcISRFrm;          /* ��װ�жϺ� */
    VOS_UINT32                          ulHdlcDefIntLimit;     /* ���װ�ж�ˮ�� */
    VOS_UINT32                          ulHdlcFrmIntLimit;     /* ��װ�ж�ˮ�� */
    PPP_DRIVER_HDLC_HARD_FRM_CONFIG_STRU    stFrmConfig;       /* ��װ������Ϣ */
    PPP_DRIVER_HDLC_HARD_DEF_CONFIG_STRU    stDefConfig;       /* ���װ������Ϣ */
}PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU;

/*****************************************************************************
 �ṹ��    : PPP_DRIVER_HDLC_HARD_FRM_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC��װ�����ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulInputTotalSize;       /* ���������ܴ�С�������ж���ʹ���жϻ�����ѵ��ʽ */
    VOS_UINT32                          ulAccmFlag;             /* ͬ�첽������ */
    VOS_UINT32                          ulPppAcFlag;            /* ��ַ������ѹ����Ϣ */
    VOS_UINT32                          ulPppPcFlag;            /* Э����ѹ����Ϣ */
    VOS_UINT16                          usProtocol;             /* Э������ */
    VOS_UINT8                           aucReserve[2];
}PPP_DRIVER_HDLC_HARD_FRM_PARA_STRU;

/*****************************************************************************
 �ṹ��    : PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC���װ������֡��Ϣ�洢�ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucExistFlag;            /* ������֡�Ƿ���ڱ�ʶ */
    VOS_UINT8                           aucReserve1[3];
    VOS_UINT16                          usDefOutOnePro;         /* ������֡��Э�� */
    VOS_UINT16                          usDefOutOneLen;         /* ������֡�ĳ��� */
    VOS_UINT8                          *pucDefOutOneAddr;       /* ָ����װ���ķ�����֡����洢����ʼ��ַ */
    VOS_UINT32                          ulDefStAgo;             /* ���ڱ���def_uncomplet_st_now�Ĵ���ֵ */
    VOS_UINT32                          ulDefInfoFrlCntAgo;     /* ���ڱ���def_info_frl_cnt_ago�Ĵ���ֵ */
}PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU;

/*****************************************************************************
 �ṹ��    : PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC���װ�����ṹ
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulInputTotalSize;       /* ���������ܴ�С�������ж���ʹ���жϻ�����ѵ��ʽ */
    VOS_UINT32                          ulPppPcFlag;            /* Э������Ӽ�ѹ����Ϣ */
    VOS_UINT32                          ulPppAcFlag;            /* ��ַ������ѹ����Ϣ */
    PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU *pstUncompleteInfo;   /*�ϴν��װ�Ĳ�����֡��Ϣ */
}PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU;


/* ��װ�Ĵ�����Ϣ */
typedef struct
{
    VOS_UINT32                          ulStateSwRst;
    VOS_UINT32                          ulPriorTimeoutCtrl;
    VOS_UINT32                          ulRdErrCurrAddr;
    VOS_UINT32                          ulWrErrCurrAddr;
    VOS_UINT32                          ulHdlcFrmEn;
    VOS_UINT32                          ulHdlcFrmRawInt;
    VOS_UINT32                          ulHdlcFrmIntStatus;
    VOS_UINT32                          ulHdlcFrmIntClr;
    VOS_UINT32                          ulHdlcFrmCfg;
    VOS_UINT32                          ulHdlcFrmAccm;
    VOS_UINT32                          ulHdlcFrmStatus;
    VOS_UINT32                          ulFrmInLliAddr;
    VOS_UINT32                          ulFrmInSublliAddr;
    VOS_UINT32                          ulFrmInPktLen;
    VOS_UINT32                          ulFrmInBlkAddr;
    VOS_UINT32                          ulFrmInBlkLen;
    VOS_UINT32                          ulFrmOutLliAddr;
    VOS_UINT32                          ulFrmOutSpaceAddr;
    VOS_UINT32                          ulFrmOutSpaceDep;
    VOS_UINT32                          ulFrmRptAddr;
    VOS_UINT32                          ulFrmRptDep;
}PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU;

/* ���װ�Ĵ�����Ϣ */
typedef struct
{
    VOS_UINT32                          ulStateSwRst;
    VOS_UINT32                          ulPriorTimeoutCtrl;
    VOS_UINT32                          ulRdErrCurrAddr;
    VOS_UINT32                          ulWrErrCurrAddr;
    VOS_UINT32                          ulHdlcDefEn;
    VOS_UINT32                          ulHdlcDefRawInt;
    VOS_UINT32                          ulHdlcDefIntStatus;
    VOS_UINT32                          ulHdlcDefIntClr;
    VOS_UINT32                          ulHdlcDefCfg;
    VOS_UINT32                          ulDefUncompletedLen;
    VOS_UINT32                          ulDefUncompletedPro;
    VOS_UINT32                          ulDefUncompletedAddr;
    VOS_UINT32                          ulDefUncompleteStAgo;
    VOS_UINT32                          ulHdlcDefGoOn;
    VOS_UINT32                          ulHdlcDefStatus;
    VOS_UINT32                          ulDefUncompletStNow;
    VOS_UINT32                          ulDefInLliAddr;
    VOS_UINT32                          ulDefInPktAddr;
    VOS_UINT32                          ulDefInPktLen;
    VOS_UINT32                          ulDefInPktLenMax;
    VOS_UINT32                          ulDefOutSpcAddr;
    VOS_UINT32                          ulDefOutSpaceDep;
    VOS_UINT32                          ulDefRptAddr;
    VOS_UINT32                          ulDefRptDep;
    VOS_UINT32                          ulHdlcDefErrInfor0;
    VOS_UINT32                          ulHdlcDefErrInfor1;
    VOS_UINT32                          ulHdlcDefErrInfor2;
    VOS_UINT32                          ulHdlcDefErrInfor3;
    VOS_UINT32                          ulDefInfoFr1CntAgo;
    VOS_UINT32                          ulDefInfoFr1CntNow;
}PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU;


/* ά�������Ϣ */

/* ��װ��ؼĴ�����ά�ɲ�ṹ */
typedef struct
{
    HDLC_MNTN_TRACE_HEAD_STRU           stHead;                     /* _H2ASN_Skip */
    PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU   stFrmRegInfo;
} HDLC_MNTN_FRM_REG_CONFIG_STRU;

/* ���װ��ؼĴ�����ά�ɲ�ṹ */
typedef struct
{
    HDLC_MNTN_TRACE_HEAD_STRU           stHead;                     /* _H2ASN_Skip */
    PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU   stDefRegInfo;
} HDLC_MNTN_DEF_REG_CONFIG_STRU;


/*****************************************************************************
  8 UNION����
*****************************************************************************/



/*****************************************************************************
  9 OTHERS����
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardPeriphClkOpen(VOS_VOID);
VOS_VOID PPP_Driver_HdlcHardPeriphClkClose(VOS_VOID);
VOS_UINT32 PPP_Driver_HdlcHardInit
(
    PPP_DRIVER_HDLC_HARD_FRM_CONFIG_STRU   *pstFrmConfig,
    PPP_DRIVER_HDLC_HARD_DEF_CONFIG_STRU   *pstDefConfig
);
VOS_VOID PPP_Driver_HdlcHardWorkStatus
(
    VOS_BOOL                           *penFrmWork,
    VOS_BOOL                           *penDefWork
);
VOS_VOID PPP_Driver_HdlcHardGetFrmResult
(
    VOS_UINT16                         *pusFrmOutSegNum,
    VOS_UINT8                          *pucFrmValidNum
);
VOS_UINT32 PPP_Driver_HdlcHardFrmEnable
(
    PPP_DRIVER_HDLC_HARD_FRM_PARA_STRU *pstDrvFrmPara,
    VOS_UINT32                         *pulEnableInterrupt
);
VOS_UINT16 PPP_Driver_HdlcHardGetDefVaildNum(VOS_VOID);
VOS_VOID PPP_Driver_HdlcHardGetDefUncompletInfo
(
    PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU *pstUncompleteInfo,
    VOS_UINT32                                     *pucValidNum
);
VOS_VOID PPP_Driver_HdlcHardGetDefErrorInfo
(
    PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU   *pstErrCnt
);

VOS_UINT32 PPP_Driver_HdlcHardDefWaitResult
(
    VOS_UINT32          ulEnableInterrupt
);
VOS_UINT32 PPP_Driver_HdlcHardDefEnable
(
    PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU *pstDrvDefPara,
    VOS_UINT32                         *pulEnableInterrupt
);
VOS_VOID PPP_Driver_HdlcHardDefCfgGoOnReg
(
    VOS_UINT32          ulDefStatus
);
VOS_VOID PPP_Driver_HdlcHardDefCfgReg
(
    PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU *pstDrvDefPara
);
VOS_VOID PPP_Driver_HdlcHardGetFrmRegInfo(PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU *pstRegConfig);

VOS_VOID PPP_Driver_HdlcHardGetDefRegInfo(PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU *pstRegConfig);
VOS_UINT32 PPP_Driver_HdlcHardGetDefRawInt(VOS_VOID);
VOS_UINT32 PPP_Driver_HdlcHardGetFrmRawInt(VOS_VOID);
VOS_VOID PPP_Driver_HdlcHardShowFrmReg(VOS_VOID);
VOS_VOID PPP_Driver_HdlcHardShowDefReg(VOS_VOID);
VOS_VOID PPP_Driver_HdlcHardShowConfigInfo(VOS_VOID);
VOS_VOID PPP_Driver_HdlcSetMaxFrmDefInfoLen(VOS_VOID);



#pragma pack()


#ifdef __cplusplus
    #if __cplusplus
            }
    #endif
#endif


#endif

