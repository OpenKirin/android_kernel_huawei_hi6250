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

#ifndef __HDLC_HRADWARE_MNTN_H__
#define __HDLC_HRADWARE_MNTN_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/
/* HDLCͳ����Ϣ��ַ */
#define PPP_HDLC_HARD_DATA_PROC_STAT    (&g_PppHdlcHardStat)

/* �����쳣��Ϣbitλ��:
        bit0�����Ƿ�����쳣����(1�ǽ����쳣����0��û�н����쳣����);
        bit1�����Ƿ����жϴ���(1���жϴ���0����ѯ����);
        bit2�����Ƿ�ȴ��ź�����ʱ(1�ǣ�0��);
        bit3�����Ƿ���ѯ��ʱ(1�ǣ�0��); */
#define     HDLC_EXCEPTION_IND_BITPOS                           (0UL)
#define     HDLC_INTERRUPT_IND_BITPOS                           (1UL)
#define     HDLC_SEM_TIMEOUT_IND_BITPOS                         (2UL)
#define     HDLC_WAIT_STATUS_TIMEOUT_IND_BITPOS                 (3UL)

/* HDLC Warning Log��Ϣ */
#define PPP_HDLC_WARNING_LOG(String)\
                DIAG_LogReport(DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(PS_PID_PPP_HDLC), DIAG_MODE_COMM, LOG_LEVEL_WARNING), (PS_PID_PPP_HDLC), __FILE__, __LINE__, "%s \r\n", String)

#define PPP_HDLC_WARNING_LOG1(String, Para1)\
                DIAG_LogReport(DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(PS_PID_PPP_HDLC), DIAG_MODE_COMM, LOG_LEVEL_WARNING), (PS_PID_PPP_HDLC), __FILE__, __LINE__, "%s, %d \r\n", String, Para1)

#define PPP_HDLC_WARNING_LOG2(String, Para1, Para2)\
                DIAG_LogReport(DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(PS_PID_PPP_HDLC), DIAG_MODE_COMM, LOG_LEVEL_WARNING), (PS_PID_PPP_HDLC), __FILE__, __LINE__, "%s, %d, %d \r\n", String, Para1, Para2)

#define PPP_HDLC_WARNING_LOG3(String, Para1, Para2, Para3)\
                DIAG_LogReport(DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(PS_PID_PPP_HDLC), DIAG_MODE_COMM, LOG_LEVEL_WARNING), (PS_PID_PPP_HDLC), __FILE__, __LINE__, "%s, %d, %d, %d \r\n", String, Para1, Para2, Para3)

#define PPP_HDLC_WARNING_LOG4(String, Para1, Para2, Para3, Para4)\
                DIAG_LogReport(DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(PS_PID_PPP_HDLC), DIAG_MODE_COMM, LOG_LEVEL_WARNING), (PS_PID_PPP_HDLC), __FILE__, __LINE__, "%s, %d, %d, %d, %d \r\n", String, Para1, Para2, Para3, Para4)


#define PPP_HDLC_ERROR_LOG(String)\
                DIAG_LogReport(DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(PS_PID_PPP_HDLC), DIAG_MODE_COMM, LOG_LEVEL_ERROR), (PS_PID_PPP_HDLC), __FILE__, __LINE__, "%s \r\n", String)

#define PPP_HDLC_ERROR_LOG1(String, Para1)\
                DIAG_LogReport(DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(PS_PID_PPP_HDLC), DIAG_MODE_COMM, LOG_LEVEL_ERROR), (PS_PID_PPP_HDLC), __FILE__, __LINE__, "%s, %d \r\n", String, Para1)

#define PPP_HDLC_ERROR_LOG2(String, Para1, Para2)\
                DIAG_LogReport(DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(PS_PID_PPP_HDLC), DIAG_MODE_COMM, LOG_LEVEL_ERROR), (PS_PID_PPP_HDLC), __FILE__, __LINE__, "%s, %d, %d \r\n", String, Para1, Para2)

#define PPP_HDLC_ERROR_LOG3(String, Para1, Para2, Para3)\
                DIAG_LogReport(DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(PS_PID_PPP_HDLC), DIAG_MODE_COMM, LOG_LEVEL_ERROR), (PS_PID_PPP_HDLC), __FILE__, __LINE__, "%s, %d, %d, %d \r\n", String, Para1, Para2, Para3)

#define PPP_HDLC_ERROR_LOG4(String, Para1, Para2, Para3, Para4)\
                DIAG_LogReport(DIAG_GEN_LOG_MODULE(VOS_GetModemIDFromPid(PS_PID_PPP_HDLC), DIAG_MODE_COMM, LOG_LEVEL_ERROR), (PS_PID_PPP_HDLC), __FILE__, __LINE__, "%s, %d, %d, %d, %d \r\n", String, Para1, Para2, Para3, Para4)

/* HDLCά��������Ϣ
        bit0��1��ʾ��Ҫ��ȡ������Ϣ
        bit1��1��ʾ��Ҫ��ȡ�Ĵ�����Ϣ
        bit2��1��ʾ��Ҫ��ȡ���ݱ�����Ϣ */
#define     PPP_HDLC_MNTN_TRACE_PARA                            (1)
#define     PPP_HDLC_MNTN_TRACE_REG                             (2)
#define     PPP_HDLC_MNTN_TRACE_DATA                            (4)

/* ��ά�ɲ�������ڴ���󳤶ȣ���ֹ��Ϊ��ά�ɲ����벻���ڴ浼�µ��帴λ */
#define     HDLC_MNTN_ALLOC_MEM_MAX_SIZE                        (2*1024)

/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

enum HDLC_MNTN_EVENT_TYPE_ENUM
{
    ID_HDLC_MNTN_FRM_REG_BEFORE_EN     = 101,     /* _H2ASN_MsgChoice HDLC_MNTN_FRM_REG_CONFIG_STRU */
    ID_HDLC_MNTN_FRM_REG_AFTER_EN,                /* _H2ASN_MsgChoice HDLC_MNTN_FRM_REG_CONFIG_STRU */
    ID_HDLC_MNTN_FRM_INPUT_PARA,                  /* _H2ASN_MsgChoice HDLC_MNTN_INPUT_PARA_LINK_STRU */
    ID_HDLC_MNTN_FRM_OUTPUT_PARA,                 /* _H2ASN_MsgChoice HDLC_MNTN_FRM_OUTPUT_PARA_STRU */
    ID_HDLC_MNTN_FRM_INPUT_DATA,                  /* _H2ASN_MsgChoice HDLC_MNTN_NODE_DATA_STRU */
    ID_HDLC_MNTN_FRM_OUTPUT_DATA,                 /* _H2ASN_MsgChoice HDLC_MNTN_NODE_DATA_STRU */
    ID_HDLC_MNTN_DEF_REG_BEFORE_EN,               /* _H2ASN_MsgChoice HDLC_MNTN_DEF_REG_CONFIG_STRU */
    ID_HDLC_MNTN_DEF_REG_AFTER_EN,                /* _H2ASN_MsgChoice HDLC_MNTN_DEF_REG_CONFIG_STRU */
    ID_HDLC_MNTN_DEF_INPUT_PARA,                  /* _H2ASN_MsgChoice HDLC_MNTN_INPUT_PARA_LINK_STRU */
    ID_HDLC_MNTN_DEF_OUTPUT_PARA,                 /* _H2ASN_MsgChoice HDLC_MNTN_DEF_OUTPUT_PARA_STRU */
    ID_HDLC_MNTN_DEF_INPUT_DATA,                  /* _H2ASN_MsgChoice HDLC_MNTN_NODE_DATA_STRU */
    ID_HDLC_MNTN_DEF_OUTPUT_DATA,                 /* _H2ASN_MsgChoice HDLC_MNTN_NODE_DATA_STRU */
    ID_HDLC_MNTN_DEF_UNCOMPLETED_INFO,            /* _H2ASN_MsgChoice HDLC_MNTN_DEF_UNCOMPLETED_INFO_STRU */

    ID_HDLC_MNTN_EVENT_TYPE_BUTT         = 0xFFFF
};
typedef VOS_UINT32 HDLC_MNTN_EVENT_TYPE_ENUM_UINT32;

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

/* ͳ����Ϣ */
typedef struct
{
    VOS_UINT32                  ulDefIpDataProcCnt;             /* ���װIP���ݰ�������� */
    VOS_UINT32                  ulDefPppDataProcCnt;            /* ���װ��PPP֡������� */
    VOS_UINT32                  ulDefUncompleteCnt;             /* ���װ�з�����֡������� */
    VOS_UINT32                  ulDefWaitIntCnt;                /* ���װ�ȴ��жϴ��� */
    VOS_UINT32                  ulDefWaitQueryCnt;              /* ���װ�ȴ���ѯ���� */
    VOS_UINT32                  ulDefLcpPauseCnt;               /* ���װLCP��ͣ���� */
    VOS_UINT32                  ulDefFullPauseCnt;              /* ���װ����ռ���ϱ��ռ�����ͣ���� */
    VOS_UINT32                  ulDefInputDiscardCnt;           /* ���װ�������ݰ����� */

    VOS_UINT32                  ulFrmIpDataProcCnt;             /* ��װIP���ݰ�������� */
    VOS_UINT32                  ulFrmPppDataProcCnt;            /* ��װ��PPP֡������� */
    VOS_UINT32                  ulFrmWaitIntCnt;                /* ��װ�ȴ��жϴ��� */
    VOS_UINT32                  ulFrmWaitQueryCnt;              /* ��װ�ȴ���ѯ���� */
    VOS_UINT32                  ulFrmAllocOutputMemFailCnt;     /* ��װ����Ŀ���ڴ�ʧ�ܴ��� */
    VOS_UINT32                  ulFrmAllocFirstMemFailCnt;      /* ��װ�����һ���ڵ�Ŀ���ڴ�ʧ�ܴ��� */
    VOS_UINT32                  ulFrmOutputLinkFullCnt;         /* ��װ������������� */
    VOS_UINT32                  ulFrmInputDiscardCnt;           /* ��װ�������ݰ����� */

    VOS_UINT32                  ulDefMaxInputCntOnce;           /* ���װһ������������ݸ��� */
    VOS_UINT32                  ulDefMaxInputSizeOnce;          /* ���װһ��������������ܳ��� */
    VOS_UINT32                  ulDefMaxValidCntOnce;           /* ���װһ����������Ч֡���� */
    VOS_UINT32                  ulDefMaxQueryCnt;               /* ���װ��ѯ������ */

    VOS_UINT32                  ulFrmMaxInputCntOnce;           /* ��װһ������������ݸ��� */
    VOS_UINT32                  ulFrmMaxInputSizeOnce;          /* ��װһ��������������ܳ��� */
    VOS_UINT32                  ulFrmMaxOutputCntOnce;          /* ��װһ��������ʹ�ýڵ������� */
    VOS_UINT32                  ulFrmMaxOutputSizeOnce;         /* ��װһ��������ʹ�ýڵ��ܳ��� */
    VOS_UINT32                  ulFrmMaxQueryCnt;               /* ��װ��ѯ������ */

    VOS_UINT32                  ulMaxCntOnce;                   /* PPPһ����ദ��Ľ����� */
    VOS_UINT32                  ulHdlcHardProcCnt;              /* PPPһ����ദ��Ľ����� */

    VOS_UINT32                  ulDefIsrCnt;                    /* ���װ�жϷ��������ô��� */
    VOS_UINT32                  ulFrmIsrCnt;                    /* ��װ�жϷ��������ô��� */
    VOS_UINT32                  ulContinueCnt;                  /* �����е�������Ҫ�ֶ�δ���Ĵ��� */
    VOS_UINT16                  usDefExpInfo;                   /* ���װ�쳣��Ϣ */
    VOS_UINT16                  usFrmExpInfo;                   /* ��װ�쳣��Ϣ */
    VOS_UINT32                  ulForbidHdlcBugNoCpy;           /* ���HDLC BUG,���������� */
    VOS_UINT32                  ulForbidHdlcBugCpy;             /* ���HDLC BUG,��Ҫ�������� */
} PPP_HDLC_HARD_DATA_PROC_STAT_ST;

/* ������������ά�ɲ�ṹ */
typedef struct
{
    VOS_MSG_HEADER                                              /* ��Ϣͷ */
    HDLC_MNTN_EVENT_TYPE_ENUM_UINT32    ulMsgname;
}HDLC_MNTN_TRACE_HEAD_STRU;


/* ���������������ݹ���*/
typedef struct
{
    HDLC_MNTN_TRACE_HEAD_STRU           stHead;                         /* _H2ASN_Skip */
    VOS_UINT16                          usNodeIndex;
    VOS_UINT16                          usLen;
    /* VOS_UINT8[usLen]�������ݷ�����������ʱ����ͷ�����������ݳ��� */
}HDLC_MNTN_NODE_DATA_STRU;


/* HDLC�ؼ��Ĵ�����ά�ɲ���Ϣ����ṹ */
typedef struct
{
    VOS_UINT32                          ulHdlcFrmRawInt;        /* ��װԭʼ�жϼĴ���ֵ */
    VOS_UINT32                          ulHdlcFrmStatus;        /* ��װ״̬�Ĵ���ֵ */
    VOS_UINT32                          ulHdlcDefRawInt;        /* ���װԭʼ�жϼĴ���ֵ */
    VOS_UINT32                          ulHdlcDefStatus;        /* ���װ״̬�Ĵ���ֵ */
}HDLC_REG_SAVE_INFO_STRU;


/*****************************************************************************
  8 UNION����
*****************************************************************************/



/*****************************************************************************
  9 OTHERS����
*****************************************************************************/

/* ͳ����Ϣ */
extern PPP_HDLC_HARD_DATA_PROC_STAT_ST g_PppHdlcHardStat;

/* ������ԭʼ�ж�ʱ��RAW_INT��STATUSֵ */
extern HDLC_REG_SAVE_INFO_STRU         g_stHdlcRegSaveInfo;


extern VOS_BOOL PPP_HDLC_HARD_MntnGetConfig(VOS_UINT32 ulMod);
extern VOS_VOID PPP_HDLC_HARD_MntnTraceMsg
(
    HDLC_MNTN_TRACE_HEAD_STRU          *pstHead,
    HDLC_MNTN_EVENT_TYPE_ENUM_UINT32    ulMsgname,
    VOS_UINT32                          ulDataLen
);
extern VOS_VOID PPP_HDLC_HARD_MntnTraceSingleData
(
    VOS_UINT16                          usDataLen,
    VOS_UINT8                          *pucDataAddr,
    HDLC_MNTN_EVENT_TYPE_ENUM_UINT32    ulEventType,
    VOS_UINT32                          ulNodeIndex
);
VOS_UINT32 PPP_HDLC_HARD_MntnGetCurrentStatSum(VOS_VOID);


#pragma pack()


#ifdef __cplusplus
    #if __cplusplus
            }
    #endif
#endif


#endif


