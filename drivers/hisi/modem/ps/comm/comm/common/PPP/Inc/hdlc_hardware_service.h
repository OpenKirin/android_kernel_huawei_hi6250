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

#ifndef __HDLC_HRADWARE_SERVICE_H__
#define __HDLC_HRADWARE_SERVICE_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "hdlc_hardware_mntn.h"
#include "hdlc_hardware_conf.h"
#include "hdlc_hardware_driver.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/

/* ----------------HDLC�Ż������ڴ� START---------------- */
/* HDLC Master�豸ʹ���ڴ����ַ */
/* ���װ�������������������ڵ����,�޸Ļ�Ӱ��HDLC_INPUT_PARA_LINK_MAX_SIZE��ֵ */
#define TTF_HDLC_DEF_INPUT_PARA_LINK_MAX_NUM        (20)

/* ���װ��������ڴ��С�ͻ���ַ
   HDLC_DEF_INPUT_PARA_LINK_MAX_NUM * sizeof(HDLC_PARA_LINK_NODE_STRU) */
#define TTF_HDLC_DEF_INPUT_PARA_LINK_BUF_LEN        (TTF_HDLC_DEF_INPUT_PARA_LINK_MAX_NUM * 12)

/* ���װ��������������������ܳ��� */
#define TTF_HDLC_DEF_INPUT_PARA_LINK_MAX_SIZE       (13*1024)


/* һ�ν��װ���IP��/��PPP֡������ = һ�ν��װ������볤��/��СIP������(20B) */
#define TTF_HDLC_DEF_OUTPUT_MAX_NUM                 (TTF_HDLC_DEF_INPUT_PARA_LINK_MAX_SIZE/20)

/* ���װ��Ϣ�ϱ������� */
#define TTF_HDLC_DEF_RPT_MAX_NUM                    (TTF_HDLC_DEF_OUTPUT_MAX_NUM)

/* ���װ��Ϣ�ϱ��ռ��ڴ��С�ͻ���ַ
   һ�ν��װ��������IP��/��PPP֡����*sizeof(HDLC_FRM_RPT_NODE) */
#define TTF_HDLC_DEF_RPT_BUF_LEN                    (TTF_HDLC_DEF_RPT_MAX_NUM*8)

/* ((TTF_HDLC_DEF_INPUT_PARA_LINK_MAX_SIZE / 6 + 1) * 8)
 * 6Ϊռ�����ռ����С��Ч֡
 * 8Ϊ���ֽڶ���
 * +1 ΪС������ȡ��
*/

#define TTF_HDLC_DEF_OUTPUT_DATA_BUF_LEN            (18 * 1024)

    /* ��HDLC MASTER�豸ʹ�õ��ڴ��ܳ��ȣ����ṹ��HDLC_DEF_BUFF_INFO_STRU�仯ʱ��
        �ú�Ӧ����Ӧ�仯 */
#define TTF_HDLC_MASTER_DEF_BUF_LEN                 (TTF_HDLC_DEF_INPUT_PARA_LINK_BUF_LEN + \
                                                        TTF_HDLC_DEF_RPT_BUF_LEN + \
                                                        TTF_HDLC_DEF_OUTPUT_DATA_BUF_LEN)

/* ��װ�������������������ڵ�������޸Ļ�Ӱ��HDLC_INPUT_PARA_LINK_MAX_SIZE��ֵ */
#define TTF_HDLC_FRM_INPUT_PARA_LINK_MAX_NUM        (20)

/* ��װ��������������������ܳ��� */
#define TTF_HDLC_FRM_INPUT_PARA_LINK_MAX_SIZE       (15*1024)

/* ��װ��������ڴ��С = TTF_HDLC_FRM_INPUT_PARA_LINK_MAX_NUM * sizeof(HDLC_PARA_LINK_NODE_STRU) */
#define TTF_HDLC_FRM_INPUT_PARA_LINK_BUF_LEN        (TTF_HDLC_FRM_INPUT_PARA_LINK_MAX_NUM * 12)

/* ��װ��Ϣ������ */
#define TTF_HDLC_FRM_RPT_MAX_NUM                    (TTF_HDLC_FRM_INPUT_PARA_LINK_MAX_NUM)

/* ��װ��Ϣ�ϱ��ռ��ڴ��С�ͻ���ַ
   TTF_HDLC_FRM_RPT_MAX_NUM * sizeof(HDLC_DEF_RPT_NODE) */
#define TTF_HDLC_FRM_RPT_BUF_LEN                    (TTF_HDLC_FRM_RPT_MAX_NUM * 8)

/* ��װ�������������������ڵ���� */
#define TTF_HDLC_FRM_OUTPUT_PARA_LINK_MAX_NUM       (TTF_HDLC_FRM_INPUT_PARA_LINK_MAX_NUM * 2)

/* ��װ��������ڴ��С�ͻ���ַ
   TTF_HDLC_FRM_OUTPUT_PARA_LINK_MAX_NUM * sizeof(HDLC_PARA_LINK_NODE_STRU) */
#define TTF_HDLC_FRM_OUTPUT_PARA_LINK_BUF_LEN       (TTF_HDLC_FRM_OUTPUT_PARA_LINK_MAX_NUM * 12)

/* ��HDLC MASTER�豸ʹ�õ��ڴ��ܳ��ȣ����ṹ��HDLC_FRM_BUFF_INFO_STRU�仯ʱ��
    �ú�Ӧ����Ӧ�仯 */
#define TTF_HDLC_MASTER_FRM_BUF_LEN                 (TTF_HDLC_FRM_INPUT_PARA_LINK_BUF_LEN + \
                                                        TTF_HDLC_FRM_RPT_BUF_LEN + \
                                                        TTF_HDLC_FRM_OUTPUT_PARA_LINK_BUF_LEN)

/* ��HDLC MASTER�豸ʹ�õ��ڴ��ܳ��� */
#define TTF_HDLC_MASTER_LINK_TOTAL_LEN              (TTF_GET_32BYTE_ALIGN_VALUE((TTF_HDLC_MASTER_DEF_BUF_LEN + \
                                                        TTF_HDLC_MASTER_FRM_BUF_LEN)))

/* ----------------HDLC�Ż������ڴ� END---------------- */

/* ���ڽ��װ���ܳ����ƻ��������һ֡���ݿ��ܻᱻ�ֳ�2�� */
#define PPP_SERVICE_HDLC_HARD_DEF_MAX_DATA_CNT      (2)


/* LCPЭ�� */
#define PPP_SERVICE_HDLC_HARD_LCP                   (0xc021)

/* ����PPP ID��ȡ��Ӧ�ڴ棬��ʱֻ��һ·����Ϊ��ֵ�����������ж�· */
#define HDLC_DEF_GET_UNCOMPLETED_INFO(usPppId)      (&l_stUncompletedInfo)
#define HDLC_DEF_GET_BUF_INFO(usPppId)              (l_pstHdlcDefBufInfo)
#define HDLC_FRM_GET_BUF_INFO(usPppId)              (l_pstHdlcFrmBufInfo)


/* ���ṹ���С�Ƿ�Ϸ� */
#if (defined(LLT_TEST_CODE))
#define PPP_SERVICE_HDLC_HARD_STRU_SIZE_CHECK(_size, _stru) \
{ \
    char STRU_SIZE_CHECK_##_stru[((_size) == sizeof(_stru)) ? 1 : -1]; \
    (void)STRU_SIZE_CHECK_##_stru[0]; \
}
#else
#define PPP_SERVICE_HDLC_HARD_STRU_SIZE_CHECK(_size, _stru)
#endif

/* TTF_MAX(TTF_HDLC_DEF_INPUT_PARA_LINK_MAX_NUM, TTF_HDLC_FRM_INPUT_PARA_LINK_MAX_NUM) */
#define HDLC_INPUT_PARA_LINK_MAX_SIZE               (20)
#define HDLC_OUTPUT_PARA_LINK_MAX_SIZE              (TTF_HDLC_FRM_OUTPUT_PARA_LINK_MAX_NUM)


/* ���װ����ռ��ס�β��ַ�������ж��Ƿ��ƻ� */
#define HDLC_DEF_OUTPUT_BUF_START_ADDR              ((VOS_UINT8 *)(&l_pstHdlcDefBufInfo->aucOutputDataBuf[0]))
#define HDLC_DEF_OUTPUT_BUF_END_ADDR                (HDLC_DEF_OUTPUT_BUF_START_ADDR + TTF_HDLC_DEF_OUTPUT_DATA_BUF_LEN)

/* Լ����Ӳ��, ��װһ������ύ1502�ֽڸ�Framer */
#define HDLC_FRM_IN_PER_MAX_CNT                     (1502L)
#define HDLC_FRM_OUT_PER_MAX_CNT                    (3013UL)

/* ��װ���ڴ����ֵ */
#define HDLC_FRM_GET_MAX_FRAMED_LEN(usLen)          (2*usLen + 13)


/* ���װ�ϱ�ÿ֡�1502�ֽ�, IPv4�1500�ֽ�, PPPģʽ����Э���ֶ�2�ֽ�, ��1502�ֽ� */
#define HDLC_DEF_OUT_PER_MAX_CNT                    (1502UL)


/* �����װĿ�Ŀռ�ʧ�ܶ�ʱ��ʱ��,��λ���� */
#define HDLC_FRM_TIME_INTERVAL                      (100)

/* ���ķ�װ�󳤶�����Ϊԭ������������˷�װһ�����������Ҫ2���ڴ����洢��װ��� */
#define PPP_HDLC_MAX_OUT_NODE_FOR_ONE_INPUT         (2)

/* ���װ��������� */
#define PPP_SERVICE_HDLC_HARD_DEF_RES_PROC(pstRslt)        l_stHldcDefCallBackInfo.pDefResProcFunc(pstRslt)

/* ���װ������Ϣ������ */
#define PPP_SERVICE_HDLC_HARD_DEF_ERR_PROC(usPppId, pstErrCnt)        l_stHldcDefCallBackInfo.pDefErrProcFunc(usPppId, pstErrCnt)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

/*****************************************************************************
 �ṹ��    : PPP_SERVICE_HDLC_HARD_WORK_STATUS_ENUM
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC����״̬
*****************************************************************************/
enum PPP_SERVICE_HDLC_HARD_WORK_STATUS_ENUM
{
   PPP_SERVICE_HDLC_HARD_NOT_WORK   = 0,
   PPP_SERVICE_HDLC_HARD_FRM_WORK,
   PPP_SERVICE_HDLC_HARD_DEF_WORK,
   PPP_SERVICE_HDLC_HARD_BOTH_WORK,
   PPP_SERVICE_HDLC_HARD_WORK_STATUS_BUTT
};

typedef VOS_UINT8 PPP_SERVICE_HDLC_HARD_WORK_STATUS_ENUM_UINT8;

/*****************************************************************************
 �ṹ��    : PPP_SERVICE_HDLC_HARD_MODE_ENUM
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC����ģʽ
*****************************************************************************/
enum PPP_SERVICE_HDLC_HARD_MODE_ENUM
{
    PPP_SERVICE_HDLC_HARD_IP_MODE    = 0,
    PPP_SERVICE_HDLC_HARD_PPP_MODE   = 1,
    PPP_SERVICE_HDLC_HARD_MODE_BUTT
};

typedef VOS_UINT8 PPP_SERVICE_HDLC_HARD_MODE_ENUM_UINT8;


/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_PROC_RESULT_ENUM
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC������
*****************************************************************************/
enum PPP_HDLC_HARD_PROC_RESULT_ENUM
{
    PPP_HDLC_HARD_PROC_ALL_SUCC = 0,
    PPP_HDLC_HARD_PROC_PART_SUCC,
    PPP_HDLC_HARD_PROC_STATUS_ERR,
    PPP_HDLC_HARD_PROC_PARA_ERR,
    PPP_HDLC_HARD_PROC_FAIL,
    PPP_HDLC_HARD_PROC_RESULT_BUTT
};

typedef VOS_UINT32 PPP_HDLC_HARD_PROC_RESULT_ENUM_UINT32;


/*****************************************************************************
 �ṹ��    : PPP_HDLC_PARA_CHECK_RESULT_ENUM
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC���������ö��
*****************************************************************************/
enum PPP_HDLC_PARA_CHECK_RESULT_ENUM
{
    PPP_HDLC_PARA_CHECK_PASS            = 0,      /* ��������Ҫ�󣬽������ݳ��� */
    PPP_HDLC_PARA_CHECK_FAIL_DISCARD    = 1,      /* ���ݰ�����������Ҫ����Ҫ���������Ǽ���������һ�����ݰ� */
    PPP_HDLC_PARA_CHECK_FAIL_KEEP       = 2,      /* ���ݰ�����������Ҫ�󣬵���Ҫ�����������´δ��� */

    PPP_HDLC_PARA_CHECK_BUTT
};
typedef VOS_UINT32 PPP_HDLC_PARA_CHECK_RESULT_ENUM_UINT32;


/*****************************************************************************
  4 OTHERS����
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
 �ṹ��    : HDLC_PARA_LINK_NODE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��������ڵ�ṹ:
           �����֤���װ�������������ʼ��ַWord���룻(�μ�HiHDLCV200�߼�������˵����)��
           ���ϵ��£���ַ���ӣ��ֱ�Ϊ�ڵ��ڴ�ָ�롢�ڵ㳤�ȡ��¸��ڵ��ַ��
           ����LEN��Ч16bits���ڵ�Half_Word����ַADDR 32bits
*****************************************************************************/
typedef struct _HDLC_PARA_LINK_NODE_STRU
{
    VOS_UINT8                          *pucDataAddr;            /* �ò����ڵ�ָ���ڴ��ַ */
    VOS_UINT16                          usDataLen;              /* �ڴ泤�ȣ���λByte */
    VOS_UINT8                           aucReserve[2];
    struct _HDLC_PARA_LINK_NODE_STRU   *pstNextNode;            /* ָ����һ�������ڵ��ָ�� */
} HDLC_PARA_LINK_NODE_STRU;

/*****************************************************************************
 �ṹ��    : HDLC_DEF_RPT_NODE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ���װ�ϱ��ռ���ṹ:
            ����֤���װ�ϱ��ռ���ʼ��ַWord���룻(�μ�HiHDLCV200�߼�������˵����)��
            ���ϵ��£���ַ���ӣ��ֱ�Ϊ�������Э�����͡����Ⱥʹ洢��ַ��
            ����LEN��Ч16bits���ڵ�Half_Word��Э��PRO��Ч16bits���ڸ�Half-Word����ַADDR 32bits
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usDefOutOneLen;         /* ��Ч֡�ĳ��� */
    VOS_UINT16                          usDefOutOnePro;         /* ��Ч֡��Э�� */
    VOS_UINT8                          *pucDefOutOneAddr;       /* ָ����װ������Ч֡����洢����ʼ��ַ */
} HDLC_DEF_RPT_NODE_STRU;

/*****************************************************************************
 �ṹ��    : HDLC_DEF_BUFF_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ������װʹ�õ��ڴ�
*****************************************************************************/
typedef struct
{
    HDLC_PARA_LINK_NODE_STRU            astInputParaLinkNodeBuf[TTF_HDLC_DEF_INPUT_PARA_LINK_MAX_NUM];/* ���װ�����������ʹ�õ��ڴ� */
    HDLC_DEF_RPT_NODE_STRU              astRptNodeBuf[TTF_HDLC_DEF_RPT_MAX_NUM];                      /* ���װ��Ϣ�ϱ��ռ�ʹ�õ��ڴ� */
    VOS_UINT8                           aucOutputDataBuf[TTF_HDLC_DEF_OUTPUT_DATA_BUF_LEN];           /* ���װĿ�Ŀռ�ʹ�õ��ڴ� */
} HDLC_DEF_BUFF_INFO_STRU;


/*****************************************************************************
 �ṹ��    : HDLC_FRM_RPT_NODE_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��װ�ϱ��ռ���ṹ:
            �����֤��װ�ϱ��ռ���ʼ��ַWord���룻(�μ�HiHDLCV200�߼�������˵����)��
            ���ϵ��£���ַ���ӣ��ֱ�Ϊ���������ʼ��ַ�����ݳ��ȣ�
            ����LEN��Ч16bits���ڵ�Half_Word����ַADDR 32bits
*****************************************************************************/
typedef struct
{
    VOS_UINT8                          *pucFrmOutOneAddr;       /* ָ���װ������Ч֡������洢��ʼ��ַ */
    VOS_UINT16                          usFrmOutOneLen;         /* ��Ч֡�ĳ��� */
    VOS_UINT8                           aucReserve[2];
} HDLC_FRM_RPT_NODE_STRU;

/*****************************************************************************
 �ṹ��    : HDLC_FRM_BUFF_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : �����װʹ�õ��ڴ�
*****************************************************************************/
typedef struct
{
    HDLC_PARA_LINK_NODE_STRU            astInputParaLinkNodeBuf[TTF_HDLC_FRM_INPUT_PARA_LINK_MAX_NUM];  /* ��װ�����������ʹ�õ��ڴ� */
    HDLC_FRM_RPT_NODE_STRU              astRptNodeBuf[TTF_HDLC_FRM_RPT_MAX_NUM];                        /* ��װ��Ϣ�ϱ��ռ�ʹ�õ��ڴ� */
    HDLC_PARA_LINK_NODE_STRU            astOutputParaLinkNodeBuf[TTF_HDLC_FRM_OUTPUT_PARA_LINK_MAX_NUM];/* ��װ�����������ʹ�õ��ڴ� */
} HDLC_FRM_BUFF_INFO_STRU;

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_NODE_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC��װ�����װ�ڵ��ڴ����
*****************************************************************************/
typedef struct
{
    VOS_UINT8                          *pucDataAddr;        /* ָ�����ݵ��ڴ��ַ */
    VOS_UINT16                          usDataLen;          /* ���ݳ��� */
    VOS_UINT8                           aucReserve[2];
    VOS_VOID                           *pDataNode;          /* �˲�������ҵ��ģ���ݴ���������ڵ���Ϣ��Service��ʹ�ô˲��� */
}PPP_HDLC_HARD_NODE_PARA_STRU;

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_FRM_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : HDLC��װ����
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usPppId;                /* PPP ID */
    VOS_UINT16                          usProtocol;             /* Э������ */
    PPP_HDLC_HARD_NODE_PARA_STRU        astInputNodePara[TTF_HDLC_FRM_INPUT_PARA_LINK_MAX_NUM];      /* ��װ����ڵ���� */
    VOS_UINT32                          ulInputNodeParaCnt;     /* ��װ����ڵ�������� */
    VOS_UINT32                          ulInputTotalSize;       /* ���װ����ڵ��ܴ�С */
    PPP_HDLC_HARD_NODE_PARA_STRU        astOutputNodePara[TTF_HDLC_FRM_OUTPUT_PARA_LINK_MAX_NUM];    /* ��װ����ڵ���� */
    VOS_UINT32                          ulOutputNodeParaCnt;    /* ��װ����ڵ�������� */
    VOS_UINT32                          ulPppAcFlag;            /* PPP�ĵ�ַ������ѹ����ʶ */
    VOS_UINT32                          ulPppPcFlag;            /* PPP��Э����ѹ����ʶ */
    VOS_UINT32                          ulAccmFlag;             /* PPP��ͬ�첽�����ֱ�ʶ */
    PPP_SERVICE_HDLC_HARD_MODE_ENUM_UINT8       enPppMode;      /* PPP����ģʽ */
    VOS_UINT8                           aucReserve[3];
} PPP_HDLC_HARD_FRM_PARA_STRU;

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_FRM_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :  HDLC��װ���
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucFrmValidCnt;      /* �ɹ���װ���ĸ��� */
    PPP_HDLC_HARD_NODE_PARA_STRU        astFrmResultNode[TTF_HDLC_FRM_RPT_MAX_NUM];
}PPP_HDLC_HARD_FRM_RESULT_STRU;

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_DEF_DATA_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :  HDLC���װ������Ϣ
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usDataLen;        /* ���װ���ݳ��� */
    VOS_UINT16                          usReserve;
    VOS_UINT8                          *pucDataAddr;      /* ���װ���ݵ�ַ */
}PPP_HDLC_HARD_DEF_DATA_INFO_STRU;


/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_DEF_RESULT_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :  HDLC���װ���
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usPppId;            /* PPP ID */
    VOS_UINT16                          usProtocol;         /* Э������ */
    PS_BOOL_ENUM_UINT8                  enSegment;          /* �����Ƿ񱻷ֶ� */
    PPP_HDLC_HARD_DEF_DATA_INFO_STRU    astDataInfo[PPP_SERVICE_HDLC_HARD_DEF_MAX_DATA_CNT];     /* ���ڽ��װ������ܳ����ƻأ�����п���һ֡���ݱ��ֳ����� */
    VOS_UINT32                          ulPara;             /* ҵ��ģ��͸������ */
}PPP_HDLC_HARD_DEF_RESULT_STRU;

/*****************************************************************************
 �ṹ��    : PPP_SERVICE_HDLC_HARD_DEF_RES_PROC_FUNC
 Э����  :
 ASN.1���� :
 �ṹ˵��  :  ҵ��ģ�鴦��HDLC���װ�������
*****************************************************************************/
typedef VOS_VOID (*PPP_SERVICE_HDLC_HARD_DEF_RES_PROC_FUNC)(PPP_HDLC_HARD_DEF_RESULT_STRU*);

/*****************************************************************************
 �ṹ��    : PPP_SERVICE_HDLC_HARD_DEF_ERR_INFO_PROC_FUNC
 Э����  :
 ASN.1���� :
 �ṹ˵��  :  ҵ��ģ�鴦��HDLC���װ������Ϣ����
*****************************************************************************/
typedef VOS_VOID (*PPP_SERVICE_HDLC_HARD_DEF_ERR_INFO_PROC_FUNC)(VOS_UINT16 usPppId, PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU *pstErrCnt);


/*****************************************************************************
 �ṹ��    : PPP_SERVICE_HDLC_HARD_DEF_CALLBACK_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ҵ��ģ����װ�ص��ṹ
*****************************************************************************/
typedef struct
{
    PPP_SERVICE_HDLC_HARD_DEF_RES_PROC_FUNC         pDefResProcFunc;        /* ���װ��������� */
    PPP_SERVICE_HDLC_HARD_DEF_ERR_INFO_PROC_FUNC    pDefErrProcFunc;        /* ���װ������Ϣ������ */
}PPP_SERVICE_HDLC_HARD_DEF_CALLBACK_STRU;

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_DEF_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :  HDLC���װ����
*****************************************************************************/
typedef struct
{
    VOS_UINT16                          usPppId;                /* PPP ID */
    PPP_SERVICE_HDLC_HARD_MODE_ENUM_UINT8       enPppMode;      /* PPP����ģʽ */
    VOS_UINT8                           ucReserve;
    VOS_UINT32                          ulPara;                 /* ҵ��ģ��͸���������ڽ���д��ظ�ҵ��ģ�� */
    PPP_HDLC_HARD_NODE_PARA_STRU        astInputNodePara[TTF_HDLC_DEF_INPUT_PARA_LINK_MAX_NUM];      /* ���װ����ڵ���� */
    VOS_UINT32                          ulInputNodeParaCnt;     /* ���װ���뼰�ڵ�������� */
    VOS_UINT32                          ulInputTotalSize;       /* ���װ����ڵ��ܴ�С */
    VOS_UINT32                          ulPppAcFlag;            /* PPP�ĵ�ַ������ѹ����ʶ */
    VOS_UINT32                          ulPppPcFlag;            /* PPP��Э����ѹ����ʶ */
} PPP_HDLC_HARD_DEF_PARA_STRU;


/* ά����ؽṹ */

/*****************************************************************************
 �ṹ��    : HDLC_MNTN_DEF_UNCOMPLETED_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :  ���װ������֡��ά�ɲ�ṹ
*****************************************************************************/
typedef struct
{
    HDLC_MNTN_TRACE_HEAD_STRU           stHead;                         /* _H2ASN_Skip */
    PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU  stUncompletedInfo;
}HDLC_MNTN_DEF_UNCOMPLETED_INFO_STRU;

/*****************************************************************************
 �ṹ��    : HDLC_MNTN_INPUT_PARA_LINK_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :  ������������ά�ɲ�ṹ
*****************************************************************************/
typedef struct
{
    HDLC_MNTN_TRACE_HEAD_STRU           stHead;                         /* _H2ASN_Skip */
    VOS_UINT32                          ulInputLinkNodeCnt;     /* �����������ڵ���� */
    HDLC_PARA_LINK_NODE_STRU            astInputParaLinkNodeBuf[HDLC_INPUT_PARA_LINK_MAX_SIZE];
}HDLC_MNTN_INPUT_PARA_LINK_STRU;

/*****************************************************************************
 �ṹ��    : HDLC_MNTN_FRM_OUTPUT_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :  ��װ��������ά�ɲ�ṹ
*****************************************************************************/
typedef struct
{
    HDLC_MNTN_TRACE_HEAD_STRU           stHead;                         /* _H2ASN_Skip */
    VOS_UINT8                           ucFrmValidNum;
    VOS_UINT8                           ucReserve1[1];
    VOS_UINT16                          usOutputNodeUsedCnt;
    VOS_UINT32                          ulOutputLinkNodeCnt;    /* �����������ڵ������ֻ�ڷ�װ��Ч */
    HDLC_PARA_LINK_NODE_STRU            astOutputParaLinkNodeBuf[TTF_HDLC_FRM_OUTPUT_PARA_LINK_MAX_NUM];/* ��װ�����������ʹ�õ��ڴ� */
    HDLC_FRM_RPT_NODE_STRU              astRptNodeBuf[TTF_HDLC_FRM_RPT_MAX_NUM];                        /* ��װ��Ϣ�ϱ��ռ�ʹ�õ��ڴ� */
}HDLC_MNTN_FRM_OUTPUT_PARA_STRU;

/*****************************************************************************
 �ṹ��    : HDLC_MNTN_DEF_OUTPUT_PARA_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  :  ���װ��������ά�ɲ�ṹ
*****************************************************************************/
typedef struct
{
    HDLC_MNTN_TRACE_HEAD_STRU           stHead;                         /* _H2ASN_Skip */
    VOS_UINT16                          usDefValidNum;
    VOS_UINT16                          usTraceNum;
    /* HDLC_DEF_RPT_NODE_STRU[usDefValidNum]�˴�Ϊ��װ��Ϣ�ϱ��ռ�ʹ�õ��ڴ棬
       �����ڴ�ʱ�ɽṹ���ȼ��ϱ��ռ���Ҫ���ڴ��С */
}HDLC_MNTN_DEF_OUTPUT_PARA_STRU;

/*****************************************************************************
  8 UNION����
*****************************************************************************/


/*****************************************************************************
  9 ȫ�ֱ�������
*****************************************************************************/



/*****************************************************************************
  10 ��������
*****************************************************************************/
extern VOS_UINT32 PPP_Service_HdlcHardInit
(
    PPP_HDLC_HARD_CONFIG_INFO_STRU             *pstHdlcConfig,
    PPP_SERVICE_HDLC_HARD_DEF_CALLBACK_STRU    *pstDefCBInfo
);
extern VOS_VOID   PPP_Service_HdlcHardDisable(VOS_VOID);
extern VOS_VOID   PPP_Service_HdlcHardSetUp(VOS_UINT16 usPppId);
extern VOS_UINT32 PPP_Service_HdlcHardGetMaxDefLen(VOS_UINT16 usPppId);
extern PPP_HDLC_HARD_PROC_RESULT_ENUM_UINT32 PPP_Service_HdlcHardFrmPacket
(
    PPP_HDLC_HARD_FRM_PARA_STRU        *pstFrmPara,
    PPP_HDLC_HARD_FRM_RESULT_STRU      *pstFrmResult
);
extern PPP_HDLC_HARD_PROC_RESULT_ENUM_UINT32 PPP_Service_HdlcHardDefPacket
(
    PPP_HDLC_HARD_DEF_PARA_STRU        *pstDefPara
);
extern VOS_VOID PPP_Service_HdlcHardOpenClk(VOS_VOID);
extern VOS_VOID PPP_Service_HdlcHardCloseClk(VOS_VOID);
extern PPP_SERVICE_HDLC_HARD_WORK_STATUS_ENUM_UINT8
PPP_Service_HdlcHardGetWorkStatus(VOS_VOID);


#pragma pack()


#ifdef __cplusplus
    #if __cplusplus
            }
    #endif
#endif

#endif

