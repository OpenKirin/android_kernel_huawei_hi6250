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
#include "hdlc_hardware_service.h"
#include "TTFComm.h"
#include "TTFUtil.h"

#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif

/*****************************************************************************
   Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_HDLC_HARDWARE_SERVICE_C


/******************************************************************************
   2 �ⲿ������������
******************************************************************************/


/*****************************************************************************
   3 ˽�ж���
*****************************************************************************/


/*****************************************************************************
   4 ȫ�ֱ�������
*****************************************************************************/


/* ���װ����ķ�����֡��Ϣ */
PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU  l_stUncompletedInfo = {0};

/* ������װʹ�õ��ڴ� */
HDLC_DEF_BUFF_INFO_STRU        *l_pstHdlcDefBufInfo = VOS_NULL_PTR;

/* �����װʹ�õ��ڴ� */
HDLC_FRM_BUFF_INFO_STRU        *l_pstHdlcFrmBufInfo = VOS_NULL_PTR;

/* ҵ��ģ���װ��������� */
PPP_SERVICE_HDLC_HARD_DEF_CALLBACK_STRU     l_stHldcDefCallBackInfo;

/******************************************************************************
   5 ����ʵ��
******************************************************************************/

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardOpenClk
 ��������  : ��HDLCʱ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardOpenClk(VOS_VOID)
{
    PPP_Driver_HdlcHardPeriphClkOpen();
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardCloseClk
 ��������  : �ر�HDLCʱ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardCloseClk(VOS_VOID)
{
    PPP_Driver_HdlcHardPeriphClkClose();
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardGetWorkStatus
 ��������  : ��ȡHDLC����״̬
 �������  : ��
 �������  : ��
 �� �� ֵ  : enWorkStatus   -   HDLC����״̬
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
PPP_SERVICE_HDLC_HARD_WORK_STATUS_ENUM_UINT8
PPP_Service_HdlcHardGetWorkStatus(VOS_VOID)
{
    VOS_BOOL                            enFrmWork;
    VOS_BOOL                            enDefWork;
    PPP_SERVICE_HDLC_HARD_WORK_STATUS_ENUM_UINT8    enWorkStatus;

    PPP_Driver_HdlcHardWorkStatus(&enFrmWork, &enDefWork);

    if ((VOS_TRUE == enFrmWork) && (VOS_TRUE == enDefWork))
    {
        enWorkStatus = PPP_SERVICE_HDLC_HARD_BOTH_WORK;
    }
    else if (VOS_TRUE == enFrmWork)
    {
        enWorkStatus = PPP_SERVICE_HDLC_HARD_FRM_WORK;
    }
    else if (VOS_TRUE == enDefWork)
    {
        enWorkStatus = PPP_SERVICE_HDLC_HARD_DEF_WORK;
    }
    else
    {
        enWorkStatus = PPP_SERVICE_HDLC_HARD_NOT_WORK;
    }

    return enWorkStatus;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardSetUp
 ��������  : ��ʼ����·PPP���ӣ�����շ�����֡��Ϣ
 �������  : usPppId - PPP ID
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardSetUp(VOS_UINT16 usPppId)
{
    PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU  *pstUncompletedInfo;


    pstUncompletedInfo = HDLC_DEF_GET_UNCOMPLETED_INFO(usPppId);

    PPP_HDLC_HARD_MEM_SET(pstUncompletedInfo, sizeof(PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU),
                 0, sizeof(PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU));

    return;
}


/*****************************************************************************
 �� �� ��  : PPP_HDLC_HARD_Disable
 ��������  : ��HDLC����ȥʹ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardDisable(VOS_VOID)
{
    /* ���޲�������ΪHDLC�Ż���һ�������װ����װ���ʱ����Ӳ���Զ���frm_en��def_en���㣻
       ��װ����װ���̳���ʱ��Ӳ��Ҳ���Զ����㣬ʹ�ڲ�״̬������IDLE״̬��*/
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardInit
 ��������  : ��ʼ��HDLC������ڴ�����Ӳ�����õȹ���
 �������  : pstHdlcConfig  -   ������Ϣ
 �������  : ��
 �� �� ֵ  : VOS_OK     -   ��ʼ���ɹ�
             VOS_ERR    -   ��ʼ��ʧ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Service_HdlcHardInit
(
    PPP_HDLC_HARD_CONFIG_INFO_STRU             *pstHdlcConfig,
    PPP_SERVICE_HDLC_HARD_DEF_CALLBACK_STRU    *pstDefCBInfo
)
{
    VOS_UINT_PTR                            ulPppVirtAddr         = 0;
    VOS_UINT_PTR                            ulPppPhyAddr          = 0;
    VOS_UINT32                              ulPppTotalBufLen      = 0;
    PPP_DRIVER_HDLC_HARD_FRM_CONFIG_STRU    stFrmConfig;       /* ��װ������Ϣ */
    PPP_DRIVER_HDLC_HARD_DEF_CONFIG_STRU    stDefConfig;       /* ���װ������Ϣ */

    if (VOS_ERR == PPP_HdlcHardConfig(pstHdlcConfig))
    {
        return VOS_ERR;
    }

    if ((VOS_NULL_PTR == pstDefCBInfo) || (VOS_NULL_PTR == pstDefCBInfo->pDefErrProcFunc)
        || (VOS_NULL_PTR == pstDefCBInfo->pDefResProcFunc))
    {
        PPP_HDLC_ERROR_LOG("pDefResProcFunc is NULL");
        return VOS_ERR;
    }

    l_stHldcDefCallBackInfo.pDefResProcFunc = pstDefCBInfo->pDefResProcFunc;
    l_stHldcDefCallBackInfo.pDefErrProcFunc = pstDefCBInfo->pDefErrProcFunc;

    /* �����ڴ泤�ȣ�ʵ��ʹ�ó���Ϊ0x6140�����밴��0x8000(32K)���� */
    ulPppTotalBufLen  = pstHdlcConfig->ulPppTotalBufLen;

    /* �ڴ����� */
    ulPppVirtAddr     = (VOS_UINT_PTR)VOS_UnCacheMemAlloc(ulPppTotalBufLen, &ulPppPhyAddr);
    if (VOS_NULL_PTR == ulPppVirtAddr)
    {
        PPP_HDLC_ERROR_LOG("PPP_HDLC_HARD_InitBuf, ERROR, VOS_UnCacheMemAlloc Fail\r\n");
        return VOS_ERR;
    }

    PPP_HDLC_HARD_MEM_SET((void *)ulPppVirtAddr, ulPppTotalBufLen, 0, ulPppTotalBufLen);

    /* ����TtfMemoryMap.h��ʼ��HDLC�����ڴ� */
    l_pstHdlcDefBufInfo = (HDLC_DEF_BUFF_INFO_STRU *)ulPppVirtAddr;
    l_pstHdlcFrmBufInfo = (HDLC_FRM_BUFF_INFO_STRU *)(ulPppVirtAddr + sizeof(HDLC_DEF_BUFF_INFO_STRU));

    g_stPppHdlcConfig.ulPppPhyAddr = ulPppPhyAddr;
    g_stPppHdlcConfig.ulPppVirtAddr = ulPppVirtAddr;

    /* TTF_HDLC_MASTER_DEF_BUF_LEN������ṹHDLC_DEF_BUFF_INFO_STRU�Ĵ�Сһ�� */
    PPP_SERVICE_HDLC_HARD_STRU_SIZE_CHECK(TTF_HDLC_MASTER_DEF_BUF_LEN, HDLC_DEF_BUFF_INFO_STRU);

    /* TTF_HDLC_MASTER_FRM_BUF_LEN�ձ�����ṹHDLC_FRM_BUFF_INFO_STRU�Ĵ�Сһ�� */
    PPP_SERVICE_HDLC_HARD_STRU_SIZE_CHECK(TTF_HDLC_MASTER_FRM_BUF_LEN, HDLC_FRM_BUFF_INFO_STRU);

    /* ��װ���ò��� */
    stFrmConfig.pInputAddr      = PPP_HDLC_HARD_BUF_VIRT2PHY(l_pstHdlcFrmBufInfo->astInputParaLinkNodeBuf);
    stFrmConfig.pOutputAddr     = PPP_HDLC_HARD_BUF_VIRT2PHY(l_pstHdlcFrmBufInfo->astOutputParaLinkNodeBuf);
    stFrmConfig.pReportAddr     = PPP_HDLC_HARD_BUF_VIRT2PHY(l_pstHdlcFrmBufInfo->astRptNodeBuf);
    stFrmConfig.ulRptBufLen     = TTF_HDLC_FRM_RPT_BUF_LEN;

    /* ���װ���ò��� */
    stDefConfig.pInputAddr      = PPP_HDLC_HARD_BUF_VIRT2PHY(l_pstHdlcDefBufInfo->astInputParaLinkNodeBuf);
    stDefConfig.pOutputAddr     = PPP_HDLC_HARD_BUF_VIRT2PHY(l_pstHdlcDefBufInfo->aucOutputDataBuf);
    stDefConfig.pReportAddr     = PPP_HDLC_HARD_BUF_VIRT2PHY(l_pstHdlcDefBufInfo->astRptNodeBuf);
    stDefConfig.ulOutputBufLen  = TTF_HDLC_DEF_OUTPUT_DATA_BUF_LEN;
    stDefConfig.ulReportBufLen  = TTF_HDLC_DEF_RPT_BUF_LEN;
    stDefConfig.ulPerInMaxLen   = HDLC_DEF_IN_PER_MAX_CNT;

    if (VOS_ERR == PPP_Driver_HdlcHardInit(&stFrmConfig, &stDefConfig))
    {
        PPP_HDLC_ERROR_LOG("HDLC Hardware Init Fail");
        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardTraceInputPara
 ��������  : ��ȡ���������������
 �������  : ulEventType        - ��������,
             ulLinkNodeCnt      - �������������,
             pastLinkNodeBuf    - ������������׵�ַ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardTraceInputPara
(
    HDLC_MNTN_EVENT_TYPE_ENUM_UINT32    ulEventType,
    VOS_UINT32                          ulLinkNodeCnt,
    HDLC_PARA_LINK_NODE_STRU           *pastLinkNodeBuf
)
{
    HDLC_MNTN_INPUT_PARA_LINK_STRU      stInputPara;
    HDLC_MNTN_INPUT_PARA_LINK_STRU     *pstInputPara = &stInputPara;
    VOS_UINT32                          ulDataLen;


    ulDataLen = sizeof(HDLC_MNTN_INPUT_PARA_LINK_STRU);

    /* ��¼���ϱ������������нڵ����Ϣ*/
    pstInputPara->ulInputLinkNodeCnt   = ulLinkNodeCnt;

    /* ��������ÿ���ڵ������ */
    PPP_HDLC_HARD_MEM_CPY(&(pstInputPara->astInputParaLinkNodeBuf[0]),
                ulLinkNodeCnt * sizeof(HDLC_PARA_LINK_NODE_STRU),
                pastLinkNodeBuf,
                ulLinkNodeCnt * sizeof(HDLC_PARA_LINK_NODE_STRU));

    PPP_HDLC_HARD_MntnTraceMsg((HDLC_MNTN_TRACE_HEAD_STRU *)pstInputPara,
                               ulEventType, ulDataLen);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardTraceFrmInput
 ��������  : ��ȡ��װ����������������
 �������  : pstFrmBuffInfo -   ��װʹ�õ��ڴ�
             pstBuildInfo   -   �����������
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardTraceFrmInput
(
    HDLC_FRM_BUFF_INFO_STRU            *pstFrmBuffInfo,
    PPP_HDLC_HARD_FRM_PARA_STRU        *pstFrmPara
)
{
    VOS_UINT32                          ulNodeLoop;
    HDLC_PARA_LINK_NODE_STRU           *pstParaNode;
    VOS_UINT8                          *pucDataAddr;


    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_PARA))
    {
        PPP_Service_HdlcHardTraceInputPara(ID_HDLC_MNTN_FRM_INPUT_PARA,
                                             pstFrmPara->ulInputNodeParaCnt,
                                             &(pstFrmBuffInfo->astInputParaLinkNodeBuf[0]));
    }

    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_DATA))
    {
        /* ��¼���ϱ������������нڵ���������ݣ�ÿ���ڵ���һ��IP�� */
        for ( ulNodeLoop = 0; ulNodeLoop < pstFrmPara->ulInputNodeParaCnt; ulNodeLoop++ )
        {
            pstParaNode = &(pstFrmBuffInfo->astInputParaLinkNodeBuf[ulNodeLoop]);
            pucDataAddr = (VOS_UINT8*)PPP_HDLC_HARD_MEM_PHY2VIRT((VOS_UINT32)pstParaNode->pucDataAddr);
            PPP_HDLC_HARD_MntnTraceSingleData(pstParaNode->usDataLen, pucDataAddr,
                                              ID_HDLC_MNTN_FRM_INPUT_DATA, ulNodeLoop);
        }
    }

    return;
}


/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardUpdateFrmBuffInfo
 ��������  : ���·�װ�ڴ������Ϣ
 �������  : pstFrmPara         -   ��װ����
 �������  : ��
 �� �� ֵ  : VOS_OK     -   ���³ɹ�
             VOS_ERR    -   ����ʧ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Service_HdlcHardUpdateFrmBuffInfo
(
    PPP_HDLC_HARD_FRM_PARA_STRU        *pstFrmPara
)
{
    VOS_UINT32                          ulNodeParaIndex;
    HDLC_PARA_LINK_NODE_STRU           *pstParaLinkNode;
    PPP_HDLC_HARD_NODE_PARA_STRU       *pstNodePara;
    VOS_UINT32                          ulOutputSize = 0;
    PPP_HDLC_HARD_DATA_PROC_STAT_ST    *pstHdlcStat;
    HDLC_FRM_BUFF_INFO_STRU            *pstFrmBuffInfo;

    if ((pstFrmPara->ulInputNodeParaCnt > TTF_HDLC_FRM_INPUT_PARA_LINK_MAX_NUM)
        || (pstFrmPara->ulOutputNodeParaCnt > TTF_HDLC_FRM_OUTPUT_PARA_LINK_MAX_NUM))
    {
        PPP_HDLC_WARNING_LOG2("ulInputMemParaCnt or ulOutputMemParaCnt is too big",
            pstFrmPara->ulInputNodeParaCnt, pstFrmPara->ulOutputNodeParaCnt);
        return VOS_ERR;
    }

    pstFrmBuffInfo = HDLC_FRM_GET_BUF_INFO(pstFrmPara->usPppId);

    /* ��������������� */
    for (ulNodeParaIndex = 0; ulNodeParaIndex < pstFrmPara->ulInputNodeParaCnt; ulNodeParaIndex++)
    {
        pstParaLinkNode = &pstFrmBuffInfo->astInputParaLinkNodeBuf[ulNodeParaIndex];
        pstNodePara     = &pstFrmPara->astInputNodePara[ulNodeParaIndex];
        pstParaLinkNode->pucDataAddr    = PPP_HDLC_HARD_MEM_VIRT2PHY(pstNodePara->pucDataAddr);
        pstParaLinkNode->usDataLen      = pstNodePara->usDataLen;
        pstParaLinkNode->pstNextNode = VOS_NULL_PTR;

        if (0 != ulNodeParaIndex)
        {
            pstFrmBuffInfo->astInputParaLinkNodeBuf[ulNodeParaIndex - 1].pstNextNode =
                (HDLC_PARA_LINK_NODE_STRU*)PPP_HDLC_HARD_BUF_VIRT2PHY(pstParaLinkNode);
        }
    }

    /* �������������� */
    for (ulNodeParaIndex = 0; ulNodeParaIndex < pstFrmPara->ulOutputNodeParaCnt; ulNodeParaIndex++)
    {
        pstParaLinkNode = &pstFrmBuffInfo->astOutputParaLinkNodeBuf[ulNodeParaIndex];
        pstNodePara     = &pstFrmPara->astOutputNodePara[ulNodeParaIndex];
        pstParaLinkNode->pucDataAddr    = PPP_HDLC_HARD_MEM_VIRT2PHY(pstNodePara->pucDataAddr);
        pstParaLinkNode->usDataLen      = pstNodePara->usDataLen;
        pstParaLinkNode->pstNextNode    = VOS_NULL_PTR;

        if (0 != ulNodeParaIndex)
        {
            pstFrmBuffInfo->astOutputParaLinkNodeBuf[ulNodeParaIndex - 1].pstNextNode =
                    (HDLC_PARA_LINK_NODE_STRU*)PPP_HDLC_HARD_BUF_VIRT2PHY(pstParaLinkNode);
        }

        ulOutputSize += pstNodePara->usDataLen;
    }

    /* ͳ����Ϣ���� */
    pstHdlcStat = PPP_HDLC_HARD_DATA_PROC_STAT;
    pstHdlcStat->ulFrmMaxInputCntOnce   = TTF_MAX(pstHdlcStat->ulFrmMaxInputCntOnce, pstFrmPara->ulInputNodeParaCnt);
    pstHdlcStat->ulFrmMaxInputSizeOnce  = TTF_MAX(pstHdlcStat->ulFrmMaxInputSizeOnce, pstFrmPara->ulInputTotalSize);
    pstHdlcStat->ulFrmMaxOutputCntOnce  = TTF_MAX(pstHdlcStat->ulFrmMaxOutputCntOnce, pstFrmPara->ulOutputNodeParaCnt);
    pstHdlcStat->ulFrmMaxOutputSizeOnce = TTF_MAX(pstHdlcStat->ulFrmMaxOutputSizeOnce, ulOutputSize);

    /* ��������������������ݿ�ά�ɲ� */
    PPP_Service_HdlcHardTraceFrmInput(pstFrmBuffInfo, pstFrmPara);

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardTraceFrmOutput
 ��������  : ��ȡ��װ����������������
 �������  : ucFrmValidNum  -   ��Ч֡����
             usFrmOutSegNum -   ��Ч֡�ֶθ���
             pstFrmBuffInfo -   ��װ����ڴ��ַ
             pstBuildInfo   -   �����������
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardTraceFrmOutput
(
    VOS_UINT8                           ucFrmValidNum,
    VOS_UINT16                          usFrmOutSegNum,
    HDLC_FRM_BUFF_INFO_STRU            *pstFrmBuffInfo,
    PPP_HDLC_HARD_FRM_PARA_STRU        *pstFrmPara
)
{
    HDLC_MNTN_FRM_OUTPUT_PARA_STRU      stOutputPara;
    HDLC_MNTN_FRM_OUTPUT_PARA_STRU     *pstOutputPara = &stOutputPara;
    VOS_UINT32                          ulDataLen;
    VOS_UINT32                          ulNodeLoop;
    HDLC_PARA_LINK_NODE_STRU           *pstParaNode;
    VOS_UINT8                          *pucDataAddr;

    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_PARA))
    {
        ulDataLen     = sizeof(HDLC_MNTN_FRM_OUTPUT_PARA_STRU);

        pstOutputPara->ulOutputLinkNodeCnt   = pstFrmPara->ulOutputNodeParaCnt;
        pstOutputPara->ucFrmValidNum         = ucFrmValidNum;
        pstOutputPara->usOutputNodeUsedCnt   = usFrmOutSegNum;

        PPP_HDLC_HARD_MEM_CPY(&(pstOutputPara->astOutputParaLinkNodeBuf[0]),
                    TTF_HDLC_FRM_OUTPUT_PARA_LINK_MAX_NUM * sizeof(HDLC_PARA_LINK_NODE_STRU),
                   &(pstFrmBuffInfo->astOutputParaLinkNodeBuf[0]),
                   TTF_HDLC_FRM_OUTPUT_PARA_LINK_MAX_NUM * sizeof(HDLC_PARA_LINK_NODE_STRU));

        PPP_HDLC_HARD_MEM_CPY(&(pstOutputPara->astRptNodeBuf[0]),
                    TTF_HDLC_FRM_RPT_MAX_NUM * sizeof(HDLC_FRM_RPT_NODE_STRU),
                   &(pstFrmBuffInfo->astRptNodeBuf[0]),
                   TTF_HDLC_FRM_RPT_MAX_NUM * sizeof(HDLC_FRM_RPT_NODE_STRU));

        PPP_HDLC_HARD_MntnTraceMsg((HDLC_MNTN_TRACE_HEAD_STRU *)pstOutputPara,
                                        ID_HDLC_MNTN_FRM_OUTPUT_PARA, ulDataLen);
    }

    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_DATA))
    {
        for ( ulNodeLoop = 0; ulNodeLoop < usFrmOutSegNum; ulNodeLoop++ )
        {
            pstParaNode = &(pstFrmBuffInfo->astOutputParaLinkNodeBuf[ulNodeLoop]);
            pucDataAddr = (VOS_UINT8*)PPP_HDLC_HARD_MEM_PHY2VIRT((VOS_UINT32)pstParaNode->pucDataAddr);
            PPP_HDLC_HARD_MntnTraceSingleData(pstParaNode->usDataLen, pucDataAddr,
                                              ID_HDLC_MNTN_FRM_OUTPUT_DATA, ulNodeLoop);
        }
    }

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardFrmResultProc
 ��������  : ����Ӳ����װ���
 �������  : pstFrmPara     -   ��װ����
 �������  : pstFrmResult   -   ��װ�����Ϣ
 �� �� ֵ  : PPP_HDLC_HARD_PROC_RESULT_ENUM_UINT32  ��װ������
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
PPP_HDLC_HARD_PROC_RESULT_ENUM_UINT32 PPP_Service_HdlcHardFrmResultProc
(
    PPP_HDLC_HARD_FRM_PARA_STRU        *pstFrmPara,
    PPP_HDLC_HARD_FRM_RESULT_STRU      *pstFrmResult
)
{
    VOS_UINT16                          usFrmOutSegNum = 0;
    VOS_UINT8                           ucFrmValidNum = 0;
    VOS_UINT8                           ucRptSpaceIndex;
    VOS_UINT32                          ulOutputLinkIndex = 0;
    VOS_UINT8                          *pucFrmOutAddr;
    VOS_UINT16                          usFrmOutLen;
    HDLC_FRM_BUFF_INFO_STRU            *pstFrmBuffInfo;
    HDLC_PARA_LINK_NODE_STRU           *pstOutputParaLink;
    HDLC_FRM_RPT_NODE_STRU             *pstFrmRptNodeStru;

    PPP_Driver_HdlcHardGetFrmResult(&usFrmOutSegNum, &ucFrmValidNum);

    pstFrmBuffInfo     = HDLC_FRM_GET_BUF_INFO(pstFrmPara->usPppId);       /* ����usPppId�ҵ���Ӧ���ڴ�  */

    /* �ϱ���װ�����ݿ�ά�ɲ�:�ϱ��ռ���Ϣ������������� */
    PPP_Service_HdlcHardTraceFrmOutput(ucFrmValidNum, usFrmOutSegNum, pstFrmBuffInfo, pstFrmPara);

    /* ucFrmValidNum�϶�ҪС�ڵ���ʹ�õ��ڴ����usFrmOutSegNum */
    if( ucFrmValidNum > usFrmOutSegNum )
    {
        PPP_HDLC_WARNING_LOG2("frm_valid_num > usFrmOutSegNum", ucFrmValidNum, usFrmOutSegNum);
        return PPP_HDLC_HARD_PROC_FAIL;
    }

    /* ucFrmValidNumӦ��������������ͬulInputLinkNodeCnt��������װ������ܻ�Ⱥ���С */
    if( ucFrmValidNum > pstFrmPara->ulInputNodeParaCnt)
    {
        PPP_HDLC_WARNING_LOG2("frm_valid_num > ulInputNodeParaCnt", ucFrmValidNum, pstFrmPara->ulInputNodeParaCnt);
        return PPP_HDLC_HARD_PROC_FAIL;
    }


    /* usFrmOutSegNumʹ�õ��ڴ�����϶�С�ڵ���ulOutputLinkNodeCnt */
    if( usFrmOutSegNum  > pstFrmPara->ulOutputNodeParaCnt)
    {
        PPP_HDLC_WARNING_LOG2("frm_out_seg_num > ulOutputMemParaCnt", usFrmOutSegNum, pstFrmPara->ulOutputNodeParaCnt);
        return PPP_HDLC_HARD_PROC_FAIL;
    }

    pstFrmRptNodeStru  = &(pstFrmBuffInfo->astRptNodeBuf[0]);              /* ��װ�ϱ��ռ��׵�ַ */
    pstOutputParaLink  = &(pstFrmBuffInfo->astOutputParaLinkNodeBuf[0]);   /* ��װ��������׵�ַ */
    for (ucRptSpaceIndex = 0; ucRptSpaceIndex < ucFrmValidNum; ucRptSpaceIndex++)
    {
        pucFrmOutAddr    = pstFrmRptNodeStru[ucRptSpaceIndex].pucFrmOutOneAddr;
        usFrmOutLen      = pstFrmRptNodeStru[ucRptSpaceIndex].usFrmOutOneLen;
        /* ��װ��ĳ���Ϊ0���ߴ��ڷ�װ�����󳤶ȵĻ������쳣�����ܼ��������߱��� */
        if ( (0 == usFrmOutLen) || (HDLC_FRM_OUT_PER_MAX_CNT < usFrmOutLen) )
        {
            PPP_HDLC_WARNING_LOG1("invalid usFrmOutOneLen", usFrmOutLen);
            pstFrmResult->ucFrmValidCnt = ucRptSpaceIndex;
            return PPP_HDLC_HARD_PROC_PART_SUCC;
        }

        /* ��װ�ϱ�����ĵ�ַ�ͷ�װ��������ַ��һ��ʱ�Ѿ��쳣�����ܼ��������߱��� */
        pstOutputParaLink = &(pstFrmBuffInfo->astOutputParaLinkNodeBuf[ulOutputLinkIndex]);
        if( pucFrmOutAddr != pstOutputParaLink->pucDataAddr)
        {
            PPP_HDLC_WARNING_LOG(" SOC copy error!");
            pstFrmResult->ucFrmValidCnt= ucRptSpaceIndex;
            return PPP_HDLC_HARD_PROC_PART_SUCC;
        }

        pstFrmResult->astFrmResultNode[ucRptSpaceIndex].usDataLen = usFrmOutLen;
        pstFrmResult->astFrmResultNode[ucRptSpaceIndex].pucDataAddr = PPP_HDLC_HARD_MEM_PHY2VIRT(pucFrmOutAddr);

        /* ��װ��ĳ��ȴ��ڷ�װ����ڵ�����ݳ���˵���÷�װ��Ľ��ռ������������ڵ� */
        ulOutputLinkIndex++;
        if (usFrmOutLen > pstOutputParaLink->usDataLen)
        {
            ulOutputLinkIndex++;
        }
    }

    pstFrmResult->ucFrmValidCnt = ucRptSpaceIndex;

    return PPP_HDLC_HARD_PROC_ALL_SUCC;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardFrmProcException
 ��������  : ��װӲ���쳣����
 �������  : ulStatus           -   ��װ�Ĵ���״̬
             ulEnableInterrupt  -   �Ƿ�ʹ���жϷ�ʽ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardFrmProcException
(
    VOS_UINT32                          ulStatus,
    VOS_UINT32                          ulEnableInterrupt
)
{
    VOS_UINT32                          ulRawInt;

    if( VOS_TRUE == ulEnableInterrupt )
    {
        /* �������жϷ�������н��������жϲ������ʴ˴�ȡ������g_stHdlcRegSaveInfo�е�ԭʼ�жϼĴ���ֵ */
        ulRawInt                        =   g_stHdlcRegSaveInfo.ulHdlcFrmRawInt;
        g_PppHdlcHardStat.usFrmExpInfo |=   (1 << HDLC_INTERRUPT_IND_BITPOS);
    }
    else
    {
        ulRawInt  =   PPP_Driver_HdlcHardGetFrmRawInt();
    }

    PPP_HDLC_ERROR_LOG2("Frm Exception ocurr", ulStatus, ulRawInt);

    g_PppHdlcHardStat.usFrmExpInfo |=   (1 << HDLC_EXCEPTION_IND_BITPOS);

    PPP_Driver_HdlcHardShowFrmReg();
    PPP_Driver_HdlcHardShowDefReg();
    PPP_Driver_HdlcHardShowConfigInfo();

    /* ��λǰ��Delay 1s��֤��ά�ɲ�������� */
    VOS_TaskDelay(1000);

    /* ���HDLC�����쳣���򵥰��쳣���� */
    mdrv_om_system_error(HDLC_FRM_SYSTEM_ERROR_ID, __LINE__, (VOS_INT)ulStatus,
                         (VOS_CHAR *)&g_stHdlcRegSaveInfo,
                         sizeof(HDLC_REG_SAVE_INFO_STRU));

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardBuildFrmPara
 ��������  : ����Ӳ����װ����
 �������  : pstFrmPara         -   ҵ��ģ���ṩ�ķ�װ����
 �������  : pstDrvFrmPara      -   Ӳ��ʹ�õķ�װ����
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardBuildFrmPara
(
    PPP_HDLC_HARD_FRM_PARA_STRU        *pstFrmPara,
    PPP_DRIVER_HDLC_HARD_FRM_PARA_STRU *pstDrvFrmPara
)
{
    VOS_UINT32                          ulAccmFlag = 0xFFFFFFFF;

    /* �ڲ����ã���֤��ηǿ� */

    /*  IPģʽһ�����P��,PPPģʽһ�������P��
      LCP֡: AC��ѹ����P��ѹ�� */
    if (PPP_SERVICE_HDLC_HARD_IP_MODE == pstFrmPara->enPppMode)
    {
        if (PPP_SERVICE_HDLC_HARD_LCP != pstFrmPara->usProtocol)
        {
            if ( 1 == pstFrmPara->ulPppAcFlag )
            {
                pstDrvFrmPara->ulPppAcFlag = HDLC_ADDRESS_CTRL_COMPRESS;
            }

            if ( 1 == pstFrmPara->ulPppPcFlag )
            {
                pstDrvFrmPara->ulPppPcFlag = HDLC_PROTOCOL_ADD_WITH_COMPRESS;
            }

            ulAccmFlag = pstFrmPara->ulAccmFlag;
        }
    }
    else
    {
        if (PPP_SERVICE_HDLC_HARD_LCP != pstFrmPara->usProtocol)
        {
            if ( 1 == pstFrmPara->ulPppAcFlag )
            {
                pstDrvFrmPara->ulPppAcFlag = HDLC_ADDRESS_CTRL_COMPRESS;
            }
        }
        pstDrvFrmPara->ulPppPcFlag = HDLC_PROTOCOL_NO_ADD;
    }

    pstDrvFrmPara->usProtocol = pstFrmPara->usProtocol;
    pstDrvFrmPara->ulAccmFlag   = ulAccmFlag;
    pstDrvFrmPara->ulInputTotalSize = pstFrmPara->ulInputTotalSize;

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardFrmPacket
 ��������  : Ӳ����װ����
 �������  : pstFrmPara         -   ҵ��ģ���ṩ�ķ�װ����
 �������  : pstFrmResult   -  ��װ�����Ϣ
 �� �� ֵ  : PPP_HDLC_HARD_PROC_RESULT_ENUM_UINT32 ��װ������
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
PPP_HDLC_HARD_PROC_RESULT_ENUM_UINT32 PPP_Service_HdlcHardFrmPacket
(
    PPP_HDLC_HARD_FRM_PARA_STRU        *pstFrmPara,
    PPP_HDLC_HARD_FRM_RESULT_STRU      *pstFrmResult
)
{
    VOS_UINT32                          ulEnableInterrupt = 0;
    VOS_UINT32                          ulCheckParaResutl;
    VOS_UINT32                          ulFrmStatus;
    VOS_UINT32                          ulFrmResult;
    PPP_DRIVER_HDLC_HARD_FRM_PARA_STRU  stDrvFrmPara;


    if ((VOS_NULL_PTR == pstFrmPara) || (VOS_NULL_PTR == pstFrmResult))
    {
        PPP_HDLC_WARNING_LOG("pstFrmPara is NULL");
        return PPP_HDLC_HARD_PROC_PARA_ERR;
    }

    ulCheckParaResutl = PPP_Service_HdlcHardUpdateFrmBuffInfo(pstFrmPara);
    if (VOS_ERR == ulCheckParaResutl)
    {
        return PPP_HDLC_HARD_PROC_PARA_ERR;
    }

    PPP_HDLC_HARD_MEM_SET(&stDrvFrmPara, sizeof(PPP_DRIVER_HDLC_HARD_FRM_PARA_STRU),
                          0x00, sizeof(PPP_DRIVER_HDLC_HARD_FRM_PARA_STRU));
    PPP_Service_HdlcHardBuildFrmPara(pstFrmPara, &stDrvFrmPara);

    ulFrmStatus = PPP_Driver_HdlcHardFrmEnable(&stDrvFrmPara, &ulEnableInterrupt);

    /* ʹ���жϣ���ȴ��жϸ������ͷ��ź�����������ѯ���װ״̬�Ĵ��� */
    if ( HDLC_FRM_ALL_PKT_DONE != ulFrmStatus )
    {
        /* �Ĵ���״̬�쳣ʱ��Ҫ��λ */
        PPP_Service_HdlcHardFrmProcException(ulFrmStatus, ulEnableInterrupt);
        return PPP_HDLC_HARD_PROC_STATUS_ERR;
    }

    ulFrmResult = PPP_Service_HdlcHardFrmResultProc(pstFrmPara, pstFrmResult);

    return ulFrmResult;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardTraceDefInput
 ��������  : ��ȡ���װ����������������
 �������  : pstFrmBuffInfo -   ��װʹ�õ��ڴ�
             pstBuildInfo   -   �����������
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardTraceDefInput
(
    PPP_HDLC_HARD_DEF_PARA_STRU        *pstDefPara,
    HDLC_DEF_BUFF_INFO_STRU            *pstDefBuffInfo
)
{
    VOS_UINT32                          ulNodeLoop;
    HDLC_PARA_LINK_NODE_STRU           *pstParaNode;
    VOS_UINT8                          *pucDataAddr;


    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_PARA))
    {
        PPP_Service_HdlcHardTraceInputPara(ID_HDLC_MNTN_DEF_INPUT_PARA,
                                             pstDefPara->ulInputNodeParaCnt,
                                             &(pstDefBuffInfo->astInputParaLinkNodeBuf[0]));
    }

    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_DATA))
    {
        /* ��¼���ϱ������������нڵ���������ݣ�ÿ���ڵ���һ��IP�� */
        for ( ulNodeLoop = 0; ulNodeLoop < pstDefPara->ulInputNodeParaCnt; ulNodeLoop++ )
        {
            pstParaNode = &(pstDefBuffInfo->astInputParaLinkNodeBuf[ulNodeLoop]);

            pucDataAddr = (VOS_UINT8*)PPP_HDLC_HARD_MEM_PHY2VIRT((VOS_UINT32)pstParaNode->pucDataAddr);
            PPP_HDLC_HARD_MntnTraceSingleData(pstParaNode->usDataLen, pucDataAddr,
                            ID_HDLC_MNTN_DEF_INPUT_DATA, ulNodeLoop);
        }
    }

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardUpdateDefBuffInfo
 ��������  : ���½��װ�ڴ������Ϣ
 �������  : pstDefPara         -   ���װ����
 �������  : ��
 �� �� ֵ  : VOS_OK     -   ���³ɹ�
             VOS_ERR    -   ����ʧ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Service_HdlcHardUpdateDefBuffInfo
(
    PPP_HDLC_HARD_DEF_PARA_STRU        *pstDefPara
)
{
    VOS_UINT32                          ulNodeParaIndex;
    HDLC_PARA_LINK_NODE_STRU           *pstParaLinkNode;
    PPP_HDLC_HARD_NODE_PARA_STRU       *pstNodePara;
    PPP_HDLC_HARD_DATA_PROC_STAT_ST    *pstHdlcStat;
    HDLC_DEF_BUFF_INFO_STRU            *pstDefBuffInfo;

    if (pstDefPara->ulInputNodeParaCnt > TTF_HDLC_DEF_INPUT_PARA_LINK_MAX_NUM)
    {
        PPP_HDLC_WARNING_LOG1("ulInputMemParaCnt is too big", pstDefPara->ulInputNodeParaCnt);
        return VOS_ERR;
    }

    pstDefBuffInfo = HDLC_DEF_GET_BUF_INFO(pstDefPara->usPppId);

    /* ��������������� */
    for (ulNodeParaIndex = 0; ulNodeParaIndex < pstDefPara->ulInputNodeParaCnt; ulNodeParaIndex++)
    {
        pstParaLinkNode = &pstDefBuffInfo->astInputParaLinkNodeBuf[ulNodeParaIndex];
        pstNodePara     = &pstDefPara->astInputNodePara[ulNodeParaIndex];
        pstParaLinkNode->pucDataAddr    = PPP_HDLC_HARD_MEM_VIRT2PHY(pstNodePara->pucDataAddr);
        pstParaLinkNode->usDataLen      = pstNodePara->usDataLen;
        pstParaLinkNode->pstNextNode = VOS_NULL_PTR;

        if (0 != ulNodeParaIndex)
        {
            pstDefBuffInfo->astInputParaLinkNodeBuf[ulNodeParaIndex - 1].pstNextNode =
                    (HDLC_PARA_LINK_NODE_STRU*)PPP_HDLC_HARD_BUF_VIRT2PHY(pstParaLinkNode);
        }
    }

    /* ͳ����Ϣ���� */
    pstHdlcStat = PPP_HDLC_HARD_DATA_PROC_STAT;
    pstHdlcStat->ulDefMaxInputCntOnce   = TTF_MAX(pstHdlcStat->ulDefMaxInputCntOnce, pstDefPara->ulInputNodeParaCnt);
    pstHdlcStat->ulDefMaxInputSizeOnce  = TTF_MAX(pstHdlcStat->ulDefMaxInputSizeOnce, pstDefPara->ulInputTotalSize);

    /* �ϱ���������������ݿ�ά�ɲ� */
    PPP_Service_HdlcHardTraceDefInput(pstDefPara, pstDefBuffInfo);

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardBuildDefPara
 ��������  : ����Ӳ�����װ����
 �������  : pstDefPara         -   ҵ��ģ���ṩ�Ľ��װ����
 �������  : pstDrvDefPara      -   Ӳ��ʹ�õĽ��װ����
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardBuildDefPara
(
    PPP_HDLC_HARD_DEF_PARA_STRU        *pstDefPara,
    PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU *pstDrvDefPara
)
{
    PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU *pstUncompleteInfo;

    /* �ڲ����ã���֤��ηǿ� */
    pstUncompleteInfo = HDLC_DEF_GET_UNCOMPLETED_INFO(pstDefPara->usPppId);

    pstDrvDefPara->ulInputTotalSize = pstDefPara->ulInputTotalSize;


    /* IPģʽ�²�����Э����PPPģʽ�º��� */
    if (PPP_SERVICE_HDLC_HARD_IP_MODE == pstDefPara->enPppMode)
    {
        if (1 == pstDefPara->ulPppPcFlag)
        {
            pstDrvDefPara->ulPppPcFlag = HDLC_PROTOCOL_REMOVE_WITH_COMPRESS;
        }
    }
    else
    {
        pstDrvDefPara->ulPppPcFlag = HDLC_PROTOCOL_NO_REMOVE;
    }

    if (1 == pstDefPara->ulPppAcFlag)
    {
        pstDrvDefPara->ulPppAcFlag = HDLC_ADDRESS_CTRL_COMPRESS;
    }

    pstDrvDefPara->pstUncompleteInfo = pstUncompleteInfo;

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardTraceDefOutput
 ��������  : ��ȡ���װ�������
 �������  : pstBuildInfo    -   �����������
             usValidFrameNum -   ��Ч֡����
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardTraceDefOutput
(
    VOS_UINT16                          usValidFrameNum,
    HDLC_DEF_BUFF_INFO_STRU            *pstDefBuffInfo
)
{
    HDLC_MNTN_DEF_OUTPUT_PARA_STRU     *pstOutputPara;
    VOS_UINT32                          ulDataLen;
    VOS_UINT32                          ulNodeLoop;
    HDLC_DEF_RPT_NODE_STRU             *pstRptNode;
    VOS_UINT16                          usMaxFrameNum;
    VOS_UINT8                          *pucDataAddr;


    /* ���װ�ϱ��ռ��ά�ɲ� */
    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_PARA))
    {
        usMaxFrameNum = (HDLC_MNTN_ALLOC_MEM_MAX_SIZE - sizeof(HDLC_MNTN_DEF_OUTPUT_PARA_STRU)) /
                         sizeof(HDLC_DEF_RPT_NODE_STRU);
        usMaxFrameNum = TTF_MIN(usMaxFrameNum, usValidFrameNum);

        ulDataLen     = sizeof(HDLC_MNTN_DEF_OUTPUT_PARA_STRU) + usMaxFrameNum * sizeof(HDLC_DEF_RPT_NODE_STRU);
        pstOutputPara = (HDLC_MNTN_DEF_OUTPUT_PARA_STRU *)PS_MEM_ALLOC(PS_PID_PPP_HDLC, ulDataLen);

        if (VOS_NULL_PTR == pstOutputPara)
        {
            PPP_HDLC_WARNING_LOG1("PPP_HDLC_HARD_MntnDefTraceOutput, NORMAL, Alloc mem failed ulDataLen %d!\r\n", ulDataLen);
            return;
        }

        pstOutputPara->usDefValidNum = usValidFrameNum;
        pstOutputPara->usTraceNum    = usMaxFrameNum;

        PPP_HDLC_HARD_MEM_CPY((VOS_UINT8 *)(pstOutputPara + 1),
                    usMaxFrameNum * sizeof(HDLC_DEF_RPT_NODE_STRU),
                   (VOS_UINT8 *)(&(pstDefBuffInfo->astRptNodeBuf[0])),
                   usMaxFrameNum * sizeof(HDLC_DEF_RPT_NODE_STRU));

        PPP_HDLC_HARD_MntnTraceMsg((HDLC_MNTN_TRACE_HEAD_STRU *)pstOutputPara,
                                   ID_HDLC_MNTN_DEF_OUTPUT_PARA, ulDataLen);

        PS_MEM_FREE(PS_PID_PPP_HDLC, pstOutputPara);
    }

    /* ���װĿ�Ŀռ���ÿ����Ч֡��ά�ɲ� */
    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_DATA))
    {
        for ( ulNodeLoop = 0; ulNodeLoop < usValidFrameNum; ulNodeLoop++ )
        {
            pstRptNode = &(pstDefBuffInfo->astRptNodeBuf[ulNodeLoop]);
            pucDataAddr = PPP_HDLC_HARD_BUF_PHY2VIRT(pstRptNode->pucDefOutOneAddr);
            PPP_HDLC_HARD_MntnTraceSingleData(pstRptNode->usDefOutOneLen, pucDataAddr,
                                              ID_HDLC_MNTN_DEF_OUTPUT_DATA, ulNodeLoop);
        }
    }

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardProcDefResult
 ��������  : ����Ӳ����װ���
 �������  : pstDefPara     -   ҵ��ģ���ṩ�Ľ��װ����
             pstErrCnt      -   ���װ���������Ϣ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardProcDefResult
(
    PPP_HDLC_HARD_DEF_PARA_STRU                    *pstDefPara,
    PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU   *pstErrCnt
)
{
    VOS_UINT16                          usValidFrameNum;
    HDLC_DEF_BUFF_INFO_STRU            *pstDefBuffInfo;
    VOS_UINT32                          ulFrameLoop;
    HDLC_DEF_RPT_NODE_STRU             *pstRptNode;
    PPP_HDLC_HARD_DEF_RESULT_STRU       stDefResult;
    VOS_UINT16                          usDefOutLen;
    VOS_UINT8                          *pucDefOutOneAddr;
    VOS_UINT16                          usFistSegLen;

    pstDefBuffInfo  = HDLC_DEF_GET_BUF_INFO(pstDefPara->usPppId);
    usValidFrameNum = PPP_Driver_HdlcHardGetDefVaildNum();

    PPP_HDLC_HARD_MEM_SET(&stDefResult, sizeof(PPP_HDLC_HARD_DEF_RESULT_STRU),
                 0x00, sizeof(PPP_HDLC_HARD_DEF_RESULT_STRU));

    /* �ϱ����װ�����ݿ�ά�ɲ�:�ϱ��ռ���Ϣ��������� */
    PPP_Service_HdlcHardTraceDefOutput(usValidFrameNum, pstDefBuffInfo);

    /* ��Ч֡�����ֵ��� */
    if (TTF_HDLC_DEF_RPT_MAX_NUM < usValidFrameNum)
    {
        PPP_HDLC_WARNING_LOG2("PPP_HDLC_HARD_DefProcValidFrames, WARNING, usValidFrameNum = %d > TTF_HDLC_DEF_RPT_MAX_NUM = %d",
                      usValidFrameNum, TTF_HDLC_DEF_RPT_MAX_NUM);
        return;
    }

    g_PppHdlcHardStat.ulDefMaxValidCntOnce = TTF_MAX(g_PppHdlcHardStat.ulDefMaxValidCntOnce, usValidFrameNum);

    stDefResult.usPppId             = pstDefPara->usPppId;
    stDefResult.ulPara              = pstDefPara->ulPara;

    for ( ulFrameLoop = 0 ; ulFrameLoop < usValidFrameNum; ulFrameLoop++ )
    {
        pstRptNode = &(pstDefBuffInfo->astRptNodeBuf[ulFrameLoop]);
        usDefOutLen = pstRptNode->usDefOutOneLen;
        if ( (0 == usDefOutLen) || (HDLC_DEF_OUT_PER_MAX_CNT < usDefOutLen) )
        {
            PPP_HDLC_WARNING_LOG1("invalid usDefOutOneLen", usDefOutLen);
            continue;
        }

        pucDefOutOneAddr = PPP_HDLC_HARD_BUF_PHY2VIRT(pstRptNode->pucDefOutOneAddr);

        /* �жϸ�֡��ʼ�ӳ����Ƿ񳬹�����ռ�β�����������ƻش��� */
        if ((pucDefOutOneAddr + usDefOutLen) > HDLC_DEF_OUTPUT_BUF_END_ADDR)
        {
            if (pucDefOutOneAddr <= HDLC_DEF_OUTPUT_BUF_END_ADDR)
            {
                usFistSegLen = (VOS_UINT16)(HDLC_DEF_OUTPUT_BUF_END_ADDR - pucDefOutOneAddr);
                stDefResult.enSegment = PS_TRUE;
                stDefResult.astDataInfo[0].pucDataAddr = pucDefOutOneAddr;
                stDefResult.astDataInfo[0].usDataLen = usFistSegLen;
                stDefResult.astDataInfo[1].pucDataAddr = HDLC_DEF_OUTPUT_BUF_START_ADDR;
                stDefResult.astDataInfo[1].usDataLen = usDefOutLen - usFistSegLen;
            }
            else
            {
                PPP_HDLC_WARNING_LOG2("Def Result Proc Err", pucDefOutOneAddr, HDLC_DEF_OUTPUT_BUF_END_ADDR);
                continue;
            }
        }
        else
        {
            stDefResult.enSegment = PS_FALSE;
            stDefResult.astDataInfo[0].pucDataAddr = pucDefOutOneAddr;
            stDefResult.astDataInfo[0].usDataLen = usDefOutLen;
        }
        stDefResult.usProtocol = pstRptNode->usDefOutOnePro;
        PPP_SERVICE_HDLC_HARD_DEF_RES_PROC(&stDefResult);
    }

    PPP_SERVICE_HDLC_HARD_DEF_ERR_PROC(pstDefPara->usPppId, pstErrCnt);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardDefSaveUncompleteInfo
 ��������  : ������װ������֡��Ϣ
 �������  : usPppId        -   PPP ID
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardDefSaveUncompleteInfo
(
    VOS_UINT16                          usPppId
)
{
    PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU *pstUncompleteInfo;
    HDLC_DEF_BUFF_INFO_STRU            *pstDefBuffInfo;
    VOS_UINT32                          ulValidNum = 0;
    VOS_UINT8                          *pucDefOutOneAddr;
    HDLC_DEF_RPT_NODE_STRU             *pstRptNode;
    HDLC_MNTN_DEF_UNCOMPLETED_INFO_STRU stMntnUncompletedInfo;

    pstUncompleteInfo = HDLC_DEF_GET_UNCOMPLETED_INFO(usPppId);
    pstDefBuffInfo    = HDLC_DEF_GET_BUF_INFO(usPppId);

    PPP_Driver_HdlcHardGetDefUncompletInfo(pstUncompleteInfo, &ulValidNum);

    if (HDLC_DEF_UNCOMPLETED_NOT_EXIST == pstUncompleteInfo->ucExistFlag)
    {
        return;
    }

    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_PARA))
    {
        /* ��ȡ���װ����ķ�����֡��Ϣ */
        PPP_HDLC_HARD_MEM_CPY(&stMntnUncompletedInfo.stUncompletedInfo,
                    sizeof(PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU),
                    pstUncompleteInfo, sizeof(PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU));

        PPP_HDLC_HARD_MntnTraceMsg((HDLC_MNTN_TRACE_HEAD_STRU *)&stMntnUncompletedInfo,
                                   ID_HDLC_MNTN_DEF_UNCOMPLETED_INFO,
                                   sizeof(HDLC_MNTN_DEF_UNCOMPLETED_INFO_STRU));
    }


    /* ������֡���ϱ���Ϣ����Ч֡���棬���ǲ�������Ч֡��Ŀ�� */
    if (TTF_HDLC_DEF_RPT_MAX_NUM <= ulValidNum)
    {
        PPP_HDLC_WARNING_LOG2("ulValidNum >= TTF_HDLC_DEF_RPT_MAX_NUM ",
                     ulValidNum, TTF_HDLC_DEF_RPT_MAX_NUM);

        pstUncompleteInfo->ucExistFlag = HDLC_DEF_UNCOMPLETED_NOT_EXIST;

        return;
    }


    pstRptNode = &(pstDefBuffInfo->astRptNodeBuf[ulValidNum]);

    pucDefOutOneAddr = PPP_HDLC_HARD_BUF_PHY2VIRT(pstRptNode->pucDefOutOneAddr);

    if (pucDefOutOneAddr != HDLC_DEF_OUTPUT_BUF_START_ADDR)
    {
        if ((pucDefOutOneAddr - HDLC_DEF_OUTPUT_BUF_START_ADDR) >= pstRptNode->usDefOutOneLen)
        {
            PPP_HDLC_HARD_MEM_CPY(HDLC_DEF_OUTPUT_BUF_START_ADDR, TTF_HDLC_DEF_OUTPUT_DATA_BUF_LEN,
                    pucDefOutOneAddr, pstRptNode->usDefOutOneLen);
        }
        else
        {
            VOS_MemMove_s(HDLC_DEF_OUTPUT_BUF_START_ADDR, TTF_HDLC_DEF_OUTPUT_DATA_BUF_LEN,
                          pucDefOutOneAddr, pstRptNode->usDefOutOneLen);
        }

        pstRptNode->pucDefOutOneAddr = PPP_HDLC_HARD_BUF_VIRT2PHY(HDLC_DEF_OUTPUT_BUF_START_ADDR);

    }

    /* ������֡��Э�顢���Ⱥ��ڴ洢�ռ�ĵ�ַ�����ֻ���ݴ���Щ��Ϣ�����½��װ��ʱ����ԭ�����HDLC */
    pstUncompleteInfo->usDefOutOnePro   = pstRptNode->usDefOutOnePro;
    pstUncompleteInfo->usDefOutOneLen   = pstRptNode->usDefOutOneLen;
    pstUncompleteInfo->pucDefOutOneAddr = pstRptNode->pucDefOutOneAddr;

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardDefProcException
 ��������  : ���װӲ���쳣����
 �������  : ulStatus           -   ���װ�Ĵ���״̬
             ulEnableInterrupt  -   �Ƿ�ʹ���жϷ�ʽ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardDefProcException
(
    VOS_UINT32          ulStatus,
    VOS_UINT32          ulEnableInterrupt
)
{
    VOS_UINT32                          ulRawInt;


    if( VOS_TRUE == ulEnableInterrupt )
    {
        /* �������жϷ�������н��������жϲ������ʴ˴�ȡ������g_stHdlcRegSaveInfo�е�ԭʼ�жϼĴ���ֵ */
        ulRawInt                        =   g_stHdlcRegSaveInfo.ulHdlcDefRawInt;
        g_PppHdlcHardStat.usDefExpInfo |=   (1 << HDLC_INTERRUPT_IND_BITPOS);
    }
    else
    {
        ulRawInt  =   PPP_Driver_HdlcHardGetDefRawInt();
    }

    PPP_HDLC_ERROR_LOG2("Def Exception ocurr", ulStatus, ulRawInt);

    g_PppHdlcHardStat.usDefExpInfo |=   (1 << HDLC_EXCEPTION_IND_BITPOS);

    PPP_Driver_HdlcHardShowFrmReg();
    PPP_Driver_HdlcHardShowDefReg();
    PPP_Driver_HdlcHardShowConfigInfo();

    /* ��λǰ��Delay 1s��֤��ά�ɲ�������� */
    VOS_TaskDelay(1000);

    /* ���HDLC�����쳣���򵥰��쳣���� */
    mdrv_om_system_error(HDLC_DEF_SYSTEM_ERROR_ID, __LINE__, (VOS_INT)ulStatus,
                         (VOS_CHAR *)&g_stHdlcRegSaveInfo,
                         sizeof(HDLC_REG_SAVE_INFO_STRU));

    return;
}


/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardDefWaitAndProc
 ��������  : �ȴ����װ��ͣ����ɣ�Ȼ����������ݣ����ܻ��ж��ͣ�ȵĹ���
 �������  : ulEnableInterrupt  -   �ж��Ƿ�ʹ��
             pstDrvDefPara      -   Ӳ�����װ����
             pstDefPara         -   ҵ��ģ���ṩ�Ľ��װ����
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Service_HdlcHardDefWaitAndProc
(
    VOS_UINT32                          ulEnableInterrupt,
    PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU *pstDrvDefPara,
    PPP_HDLC_HARD_DEF_PARA_STRU        *pstDefPara
)
{
    VOS_UINT32                          ulDefStatus;
    VOS_UINT32                          ulContinue;
    PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU    stErrCnt;

    PPP_HDLC_HARD_MEM_SET(&stErrCnt, sizeof(PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU),
                          0x00, sizeof(PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU));

    for (; ;)
    {
        /* ʹ���жϣ���ȴ��жϸ������ͷ��ź�����������ѯ���װ״̬�Ĵ��� */
        ulDefStatus = PPP_Driver_HdlcHardDefWaitResult(ulEnableInterrupt);

        switch ( ulDefStatus )
        {
            case HDLC_DEF_STATUS_PAUSE_RPT_SPACE_FULL :
            case HDLC_DEF_STATUS_PAUSE_OUTPUT_SPACE_FULL :
                /* ������Ч֡������GO_ON�Ĵ��� */
                PPP_Service_HdlcHardProcDefResult(pstDefPara, VOS_NULL_PTR);
                PPP_Driver_HdlcHardDefCfgGoOnReg(ulDefStatus);

                ulContinue = VOS_TRUE;
                g_PppHdlcHardStat.ulDefFullPauseCnt++;
                break;
            case HDLC_DEF_STATUS_PAUSE_LCP :
                /* ������Ч֡��LCP֡���������üĴ���������GO_ON�Ĵ��� */
                PPP_Service_HdlcHardProcDefResult(pstDefPara, VOS_NULL_PTR);
                pstDrvDefPara->pstUncompleteInfo = VOS_NULL_PTR;
                PPP_Driver_HdlcHardDefCfgReg(pstDrvDefPara);
                PPP_Driver_HdlcHardDefCfgGoOnReg(ulDefStatus);

                ulContinue = VOS_TRUE;
                g_PppHdlcHardStat.ulDefLcpPauseCnt++;
                break;
            case HDLC_DEF_STATUS_DONE_WITHOUT_FRAMES :
                /* ���ݷ�����ָ֡ʾ�����������֡��Ϣ */
                PPP_Service_HdlcHardDefSaveUncompleteInfo(pstDefPara->usPppId);

                ulContinue = VOS_FALSE;
                break;
            case HDLC_DEF_STATUS_DONE_WITH_FRAMES :
                /* ������Ч֡������֡��LCP֡(���������һ֡)�����ݷ�����ָ֡ʾ�����������֡��Ϣ */
                PPP_Driver_HdlcHardGetDefErrorInfo(&stErrCnt);
                PPP_Service_HdlcHardProcDefResult(pstDefPara, &stErrCnt);
                PPP_Service_HdlcHardDefSaveUncompleteInfo(pstDefPara->usPppId);
                ulContinue = VOS_FALSE;
                break;
            case HDLC_DEF_STATUS_DOING :
            default:
                /* ��ӡ�쳣��־������PPP���� */
                PPP_Service_HdlcHardDefProcException(ulDefStatus, ulEnableInterrupt);

                ulContinue = VOS_FALSE;
                break;
        }

        /* ��ͣ״̬��Ҫ������������״̬���װ����˳� */
        if (VOS_TRUE != ulContinue)
        {
            break;
        }
    }

    g_stHdlcRegSaveInfo.ulHdlcDefRawInt = 0xFFFFFFFFU;
    g_stHdlcRegSaveInfo.ulHdlcDefStatus = 0xFFFFFFFFU;

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardDefPacket
 ��������  : Ӳ����ʽIP���ͽ��װ
 �������  : pstDefPara     -   ҵ��ģ���ṩ�Ľ��װ����
 �������  : ��
 �� �� ֵ  : PPP_HDLC_HARD_PROC_RESULT_ENUM_UINT32  Ӳ�����װ������
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
PPP_HDLC_HARD_PROC_RESULT_ENUM_UINT32 PPP_Service_HdlcHardDefPacket
(
    PPP_HDLC_HARD_DEF_PARA_STRU        *pstDefPara
)
{
    VOS_UINT32                          ulEnableInterrupt = 0;
    VOS_UINT32                          ulUpdDefBufResutl;
    PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU  stDrvDefPara;

    if (VOS_NULL_PTR == pstDefPara)
    {
        PPP_HDLC_WARNING_LOG("pstFrmPara is NULL");
        return PPP_HDLC_HARD_PROC_PARA_ERR;
    }

    ulUpdDefBufResutl = PPP_Service_HdlcHardUpdateDefBuffInfo(pstDefPara);
    if (VOS_ERR == ulUpdDefBufResutl)
    {
        return PPP_HDLC_HARD_PROC_PARA_ERR;
    }

    PPP_HDLC_HARD_MEM_SET(&stDrvDefPara, sizeof(PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU),
                          0x00, sizeof(PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU));
    PPP_Service_HdlcHardBuildDefPara(pstDefPara, &stDrvDefPara);

    PPP_Driver_HdlcHardDefEnable(&stDrvDefPara, &ulEnableInterrupt);

    /* �ȴ����װ��ͣ����ɣ�Ȼ����������ݣ����ܻ��ж��ͣ�ȵĹ��� */
    PPP_Service_HdlcHardDefWaitAndProc(ulEnableInterrupt, &stDrvDefPara, pstDefPara);

    return PPP_HDLC_HARD_PROC_ALL_SUCC;
}

/*****************************************************************************
 �� �� ��  : PPP_Service_HdlcHardGetMaxDefLen
 ��������  : ��ȡ���װ������볤��
 �������  : usPppId        -   PPP ID
 �������  : ��
 �� �� ֵ  : VOS_UINT32 ���װ������볤��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Service_HdlcHardGetMaxDefLen(VOS_UINT16 usPppId)
{
    PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU *pstUncompleteInfo;
    VOS_UINT32                          ulMaxDataLen1Time = TTF_HDLC_DEF_INPUT_PARA_LINK_MAX_SIZE;

    pstUncompleteInfo = HDLC_DEF_GET_UNCOMPLETED_INFO(usPppId);
    if (HDLC_DEF_UNCOMPLETED_EXIST == pstUncompleteInfo->ucExistFlag)
    {
        ulMaxDataLen1Time -= pstUncompleteInfo->usDefOutOneLen;
    }

    return ulMaxDataLen1Time;
}

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

