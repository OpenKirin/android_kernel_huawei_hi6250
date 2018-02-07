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

/******************************************************************************
  1 ͷ�ļ�����
******************************************************************************/
#include "hdlc_hardware_mntn.h"
#include "msp_diag_comm.h"
#include "hdlc_hardware_conf.h"

#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif

/*****************************************************************************
   Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_HDLC_HARDWARE_MNTN_C

/******************************************************************************
   2 �ⲿ������������
******************************************************************************/


/*****************************************************************************
   3 ˽�ж���
*****************************************************************************/


/*****************************************************************************
   4 ȫ�ֱ�������
*****************************************************************************/


/* ͳ����Ϣ */
PPP_HDLC_HARD_DATA_PROC_STAT_ST         g_PppHdlcHardStat   = {0};

/* ��ά�ɲ�ȼ����� */
VOS_UINT32                              g_ulHdlcMntnConfig = 0;

/* ������ԭʼ�ж�ʱ��RAW_INT��STATUSֵ */
HDLC_REG_SAVE_INFO_STRU                 g_stHdlcRegSaveInfo;


/******************************************************************************
   5 ����ʵ��
******************************************************************************/

/*****************************************************************************
 �� �� ��  : PPP_HDLC_HARD_MntnGetConfig
 ��������  : ��ȡ��ά�ɲ�ȼ�
 �������  : ulMod      -   ��ά�ɲ⹴ȡ��
 �������  : ��
 �� �� ֵ  : VOS_TRUE   -   ��ȡ
             VOS_FALSE  -   ����ȡ
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_BOOL PPP_HDLC_HARD_MntnGetConfig(VOS_UINT32 ulMod)
{
    if (0 == (g_ulHdlcMntnConfig & ulMod))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

/*****************************************************************************
 �� �� ��  : PPP_HDLC_HARD_MntnSetConfig
 ��������  : ���ÿ�ά�ɲ�ȼ�
 �������  : ulConfig - ��ά�ɲ�ȼ�
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_HDLC_HARD_MntnSetConfig(VOS_UINT32 ulConfig)
{
    g_ulHdlcMntnConfig = ulConfig;

    return g_ulHdlcMntnConfig;
}


/*****************************************************************************
 �� �� ��  : PPP_HDLC_HARD_MntnTraceMsg
 ��������  : ������������
 �������  : pstHead    - ��Ϣ�׵�ַ
             ulMsgname  - ��ϢID
             ulDataLen  - ��Ϣ����
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_HDLC_HARD_MntnTraceMsg
(
    HDLC_MNTN_TRACE_HEAD_STRU          *pstHead,
    HDLC_MNTN_EVENT_TYPE_ENUM_UINT32    ulMsgname,
    VOS_UINT32                          ulDataLen
)
{
    pstHead->ulReceiverCpuId = VOS_LOCAL_CPUID;
    pstHead->ulReceiverPid   = PS_PID_PPP_HDLC;
    pstHead->ulSenderCpuId   = VOS_LOCAL_CPUID;
    pstHead->ulSenderPid     = PS_PID_PPP_HDLC;
    pstHead->ulLength        = ulDataLen - VOS_MSG_HEAD_LENGTH;

    pstHead->ulMsgname       = ulMsgname;

    DIAG_TraceReport(pstHead);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_HDLC_HARD_MntnShowStatInfo
 ��������  : ��ӡͳ����Ϣ
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_HDLC_HARD_MntnShowStatInfo(VOS_VOID)
{
    vos_printf("\n================HDLC Hardware STAT INFO Begin==========================\n");

    vos_printf("���װ����IP���Ͱ�����            = %d\n", g_PppHdlcHardStat.ulDefIpDataProcCnt);
    vos_printf("���װ����PPP���Ͱ�����           = %d\n", g_PppHdlcHardStat.ulDefPppDataProcCnt);
    vos_printf("���װ���������֡����            = %d\n", g_PppHdlcHardStat.ulDefUncompleteCnt);
    vos_printf("���װ�ȴ��жϴ���                = %d\n", g_PppHdlcHardStat.ulDefWaitIntCnt);
    vos_printf("���װ��ѯ����                    = %d\n", g_PppHdlcHardStat.ulDefWaitQueryCnt);
    vos_printf("���װ�жϴ���                    = %d\n", g_PppHdlcHardStat.ulDefIsrCnt);
    vos_printf("���װLCP֡��ͣ����               = %d\n", g_PppHdlcHardStat.ulDefLcpPauseCnt);
    vos_printf("���װ�ռ�����ͣ����              = %d\n", g_PppHdlcHardStat.ulDefFullPauseCnt);
    vos_printf("���װ�����������ݰ�����          = %d\n", g_PppHdlcHardStat.ulDefInputDiscardCnt);

    vos_printf("��װ����IP���Ͱ�����              = %d\n", g_PppHdlcHardStat.ulFrmIpDataProcCnt);
    vos_printf("��װ����PPP���Ͱ�����             = %d\n", g_PppHdlcHardStat.ulFrmPppDataProcCnt);
    vos_printf("��װ�ȴ��жϴ���                  = %d\n", g_PppHdlcHardStat.ulFrmWaitIntCnt);
    vos_printf("��װ��ѯ����                      = %d\n", g_PppHdlcHardStat.ulFrmWaitQueryCnt);
    vos_printf("��װ�жϴ���                      = %d\n", g_PppHdlcHardStat.ulFrmIsrCnt);
    vos_printf("��װ����Ŀ�Ŀռ��ڴ�ʧ�ܴ���      = %d\n", g_PppHdlcHardStat.ulFrmAllocOutputMemFailCnt);
    vos_printf("��װ�����һ��Ŀ�Ŀռ��ڴ�ʧ�ܴ���= %d\n", g_PppHdlcHardStat.ulFrmAllocFirstMemFailCnt);
    vos_printf("��װ�����������������            = %d\n", g_PppHdlcHardStat.ulFrmOutputLinkFullCnt);
    vos_printf("��װ�����������ݰ�����            = %d\n", g_PppHdlcHardStat.ulFrmInputDiscardCnt);

    vos_printf("���װ�����������ڵ���          = %d\n", g_PppHdlcHardStat.ulDefMaxInputCntOnce);
    vos_printf("���װ������������ܳ���          = %d\n", g_PppHdlcHardStat.ulDefMaxInputSizeOnce);
    vos_printf("���װ�����Ч֡������          = %d\n", g_PppHdlcHardStat.ulDefMaxValidCntOnce);
    vos_printf("���װ��ѯ������                = %d\n", g_PppHdlcHardStat.ulDefMaxQueryCnt);

    vos_printf("��װ�����������ڵ���            = %d\n", g_PppHdlcHardStat.ulFrmMaxInputCntOnce);
    vos_printf("��װ������������ܳ���            = %d\n", g_PppHdlcHardStat.ulFrmMaxInputSizeOnce);
    vos_printf("��װ���ʹ�����ڵ����          = %d\n", g_PppHdlcHardStat.ulFrmMaxOutputCntOnce);
    vos_printf("��װ���ʹ�����ڵ��ܳ���        = %d\n", g_PppHdlcHardStat.ulFrmMaxOutputCntOnce);
    vos_printf("��װ��ѯ������                  = %d\n", g_PppHdlcHardStat.ulFrmMaxQueryCnt);

    vos_printf("���δ������ڵ���                = %d\n", g_PppHdlcHardStat.ulMaxCntOnce);
    vos_printf("�����ܽڵ���                      = %d\n", g_PppHdlcHardStat.ulHdlcHardProcCnt);
    vos_printf("continue����                      = %d\n", g_PppHdlcHardStat.ulContinueCnt);
    vos_printf("usDefExpInfo��ʶ                  = %d\n", g_PppHdlcHardStat.usDefExpInfo);
    vos_printf("usFrmExpInfo��ʶ                  = %d\n", g_PppHdlcHardStat.usFrmExpInfo);

    vos_printf("���HDLC BUG���������ݴ���        = %d\n", g_PppHdlcHardStat.ulForbidHdlcBugNoCpy);
    vos_printf("���HDLC BUG�������ݴ���          = %d\n", g_PppHdlcHardStat.ulForbidHdlcBugCpy);


    vos_printf("================HDLC Hardware STAT INFO End==========================\n");

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_HDLC_HARD_MntnTraceSingleData
 ��������  : ��ȡ��������
 �������  : usDataLen   -   �������ݳ���
             pucDataAddr -   �����׵�ַ
             ulEventType -   ��������
             ulNodeIndex -   ����������������±�
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_HDLC_HARD_MntnTraceSingleData
(
    VOS_UINT16                          usDataLen,
    VOS_UINT8                          *pucDataAddr,
    HDLC_MNTN_EVENT_TYPE_ENUM_UINT32    ulEventType,
    VOS_UINT32                          ulNodeIndex
)
{
    VOS_UINT32                          ulDataLen;
    HDLC_MNTN_NODE_DATA_STRU           *pstNodeData;
    VOS_UINT32                          ulAllocDataLen;


    ulAllocDataLen = TTF_MIN(usDataLen, HDLC_MNTN_ALLOC_MEM_MAX_SIZE);

    /* ��Ϣ���ȵ�����Ϣ�ṹ��С���������ݳ��� */
    ulDataLen   = ulAllocDataLen + sizeof(HDLC_MNTN_NODE_DATA_STRU);

    pstNodeData = (HDLC_MNTN_NODE_DATA_STRU *)PS_MEM_ALLOC(PS_PID_PPP_HDLC, ulDataLen);

    if (VOS_NULL_PTR == pstNodeData)
    {
        PPP_HDLC_WARNING_LOG1("Alloc mem failed", ulEventType);
        return;
    }

    /* ���ڱ�ʶ����һ�����������еĵڼ���IP�� */
    pstNodeData->usNodeIndex = (VOS_UINT16)ulNodeIndex;
    pstNodeData->usLen       = usDataLen;

    PPP_HDLC_HARD_MEM_CPY((VOS_UINT8 *)(pstNodeData + 1), ulAllocDataLen, pucDataAddr, ulAllocDataLen);

    PPP_HDLC_HARD_MntnTraceMsg((HDLC_MNTN_TRACE_HEAD_STRU *)pstNodeData,
                               ulEventType, ulDataLen);

    PS_MEM_FREE(PS_PID_PPP_HDLC, pstNodeData);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_HDLC_HARD_MntnGetCurrentStatSum
 ��������  : ��ȡ��ǰͳ��ֵ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ͳ��ֵ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��10��22��
    ��    ��   : h00309869
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_HDLC_HARD_MntnGetCurrentStatSum(VOS_VOID)
{
    VOS_UINT32                              ulStatNum;
    VOS_UINT32                              ulLoop;
    VOS_UINT32                              ulStatSum   = 0;
    VOS_UINT32                             *pulStat     = (VOS_UINT32 *)&g_PppHdlcHardStat;


    ulStatNum = sizeof(PPP_HDLC_HARD_DATA_PROC_STAT_ST) / sizeof(VOS_UINT32);   /* �����ж��ٸ�UINT32ͳ�Ʊ���������UINT16ͳ�Ʊ����ϲ���һ�� */

    for ( ulLoop = 0 ; ulLoop < ulStatNum; ulLoop++ )
    {
        ulStatSum += *pulStat++;
    }

    return ulStatSum;
}

/*****************************************************************************
 �� �� ��  : PPP_Help
 ��������  : ������ӡ
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��9��25��
    ��    ��   : x59651
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_HDLC_HARD_Help(VOS_VOID)
{
    vos_printf("********************PPP�����Ϣ************************\n");
    vos_printf("PPP_HDLC_HARD_MntnShowStatInfo      ��ӡͳ����Ϣ\n");
    vos_printf("PPP_INPUT_ShowStatInfo              ��ӡ g_PppDataQCtrl��Ϣ(��A����Ч)\n");
    vos_printf("PPP_HDLC_HARD_MntnSetConfig         ���ÿ�ά�ɲ�ȼ�:\n");
    vos_printf("                                    1--������2--�Ĵ�����4--����\n");
    vos_printf("PPP_Driver_HdlcHardShowDefReg       ��ӡ���װ�Ĵ�����Ϣ\n");
    vos_printf("PPP_Driver_HdlcHardShowFrmReg       ��ӡ��װ�Ĵ�����Ϣ\n");
    vos_printf("PPP_Driver_HdlcHardSetDefIntLimit   ���ý��װ�ж�ˮ��\n");
    vos_printf("PPP_Driver_HdlcHradSetFrmIntLimit   ���÷�װ�ж�ˮ��\n");
    vos_printf("PPP_Driver_HdlcHardShowConfigInfo    ��ӡ������Ϣ\n");

    return;
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

