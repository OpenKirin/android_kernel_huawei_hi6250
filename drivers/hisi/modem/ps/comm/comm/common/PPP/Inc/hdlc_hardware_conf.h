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

#ifndef __HDLC_HRADWARE_CONF_H__
#define __HDLC_HRADWARE_CONF_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "TTFComm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 �궨��
*****************************************************************************/

/*�ڴ濽���궨��, ulDestlen >= ulSrcLen */
#define PPP_HDLC_HARD_MEM_CPY(pucDestBuffer, ulDestlen, pucSrcBuffer, ulSrcLen) \
                    g_stPppHdlcConfig.pMemCpyFunc( pucDestBuffer, ulDestlen, pucSrcBuffer, ulSrcLen, VOS_FILE_ID, __LINE__ )

/*�ڴ��������궨��, ulDestLen >= ulFillLen */
#define PPP_HDLC_HARD_MEM_SET(pucDestBuffer, ulDestLen, ucData, ulFillLen) \
                    g_stPppHdlcConfig.pMemSetFunc( pucDestBuffer, ulDestLen, ucData, ulFillLen, VOS_FILE_ID, __LINE__ )

/* HDLC��ʹ�õ�PID */
#define PS_PID_PPP_HDLC                 (g_stPppHdlcConfig.ulUserPid)

/* HDLC��д�Ĵ��� */
#define PPP_HDLC_WRITE_32REG(pAddr, value)      g_stPppHdlcConfig.pWrite32RegFunc(pAddr, value)
#define PPP_HDLC_READ_32REG(pAddr)              g_stPppHdlcConfig.pRead32RegFunc(pAddr)
/* HDLCʹ�õķ�װ���װ�ڴ���ʵ��ַת�� */
#define PPP_HDLC_HARD_BUF_PHY2VIRT(ulPAddr)     PPP_HDLC_HARD_BufPhy2Virt((VOS_UINT32)ulPAddr)
#define PPP_HDLC_HARD_BUF_VIRT2PHY(ulVAddr)     PPP_HDLC_HARD_BufVirt2Phy((VOS_UINT32)ulVAddr)
/* HDLCҵ��ģ���ڴ���ʵ��ַת�� */
#define PPP_HDLC_HARD_MEM_PHY2VIRT(ulPAddr)     PPP_HDLC_HARD_MemPhy2Virt((VOS_UINT32)ulPAddr)
#define PPP_HDLC_HARD_MEM_VIRT2PHY(ulVAddr)     PPP_HDLC_HARD_MemVirt2Phy((VOS_UINT32)ulVAddr)
/* HDLC����ַIOת�� */
#define PPP_HDLC_HARD_IO_ADDRESS(ulAddr)        PPP_HDLC_HARD_IOAddrCvt(ulAddr)
/* HDLC���װ����������󳤶� */
#define HDLC_DEF_IN_PER_MAX_CNT                 (g_stPppHdlcConfig.ulDefOneMaxSize)


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/

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
 �ṹ��    : PPP_HDLC_HARD_MEM_SET_FUNC
 Э����  :
 ASN.1���� :
 �ṹ˵��  : hdlcʹ�õ��ڴ����ú���
*****************************************************************************/
typedef VOS_VOID (*PPP_HDLC_HARD_MEM_SET_FUNC)
(
    VOS_VOID                           *pDest,
    VOS_SIZE_T                          ulDestSize,
    VOS_CHAR                            cChar,
    VOS_SIZE_T                          ulCount,
    VOS_INT32                           ulFileID,
    VOS_INT32                           usLineNo
);

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_MEM_CPY_FUNC
 Э����  :
 ASN.1���� :
 �ṹ˵��  : hdlcʹ�õ��ڴ濽������
*****************************************************************************/
typedef VOS_VOID (*PPP_HDLC_HARD_MEM_CPY_FUNC)
(
    VOS_VOID                           *pDest,
    VOS_SIZE_T                          ulDestSize,
    const VOS_VOID                     *pSrc,
    VOS_SIZE_T                          ulCount,
    VOS_INT32                           lFileID,
    VOS_INT32                           lLineNo
);

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_MEM_CVT_FUNC
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ҵ��ģ���ڴ���ʵ��ַת������
*****************************************************************************/
typedef VOS_UINT32 (*PPP_HDLC_HARD_MEM_CVT_FUNC)
(
    VOS_UINT32                          ulAddr
);

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_BUF_CVT_FUNC
 Э����  :
 ASN.1���� :
 �ṹ˵��  : hdlcʹ�õ��ڴ����ʵ��ַת������
*****************************************************************************/
typedef VOS_UINT_PTR (*PPP_HDLC_HARD_BUF_CVT_FUNC)
(
    VOS_UINT8                          *pucCurAddr,
    VOS_UINT8                          *pucStartAddr1,
    VOS_UINT8                          *pucStartAddr2,
    VOS_UINT32                          ulBufLen
);

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_WRITE_REG_FUNC
 Э����  :
 ASN.1���� :
 �ṹ˵��  : д32λ�Ĵ���
*****************************************************************************/
typedef VOS_VOID (*PPP_HDLC_HARD_WRITE_REG_FUNC)
(
    VOS_UINT32                          ulAddr,
    VOS_UINT32                          ulValue
);

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_READ_REG_FUNC
 Э����  :
 ASN.1���� :
 �ṹ˵��  : ��32λ�Ĵ���
*****************************************************************************/
typedef VOS_UINT32 (*PPP_HDLC_HARD_READ_REG_FUNC)
(
    VOS_UINT32                          ulAddr
);

/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_IO_CVT_FUNC
 Э����  :
 ASN.1���� :
 �ṹ˵��  : IO��ַӳ�亯��
*****************************************************************************/
typedef VOS_UINT32 (*PPP_HDLC_HARD_IO_CVT_FUNC)
(
    VOS_UINT32                          ulAddr
);


/*****************************************************************************
 �ṹ��    : PPP_HDLC_HARD_CONFIG_INFO_STRU
 Э����  :
 ASN.1���� :
 �ṹ˵��  : hdlc������Ϣ
                pMemPhy2VirtFunc��pMemVirt2PhyFunc��pBufPhy2VirtFunc��pBufVirt2PhyFunc��
             pIOAddrCvtFuncΪ�ձ�ʾ����Ҫת��
                pMemSetFunc��pMemCpyFunc��ulUserPid��ulPppTotalBufLen��pWrite32RegFunc��
             pRead32RegFunc������Ϊ��
                ulPppVirtAddr��ulPppPhyAddrΪServiceģ�����룬ҵ��㲻������
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          ulPppTotalBufLen;       /* HDLCʹ�õ��ڴ泤�ȣ���ҵ��ģ������ */
    VOS_UINT_PTR                        ulPppVirtAddr;          /* HDLCʹ���ڴ�����ַ��ҵ��ģ�����ó��ȣ�Service���� */
    VOS_UINT_PTR                        ulPppPhyAddr;           /* HDLCʹ���ڴ��ʵ��ַ */
    PPP_HDLC_HARD_MEM_SET_FUNC          pMemSetFunc;            /* HDLCʹ�õ��ڴ����ú�����ҵ��ģ������ */
    PPP_HDLC_HARD_MEM_CPY_FUNC          pMemCpyFunc;            /* HDLCʹ�õ��ڴ濽��������ҵ��ģ������ */
    PPP_HDLC_HARD_MEM_CVT_FUNC          pMemPhy2VirtFunc;       /* ҵ��ģ���ڴ�ʵ��ַת���ַ������ҵ��ģ������ */
    PPP_HDLC_HARD_MEM_CVT_FUNC          pMemVirt2PhyFunc;       /* ҵ��ģ���ڴ����ַתʵ��ַ������ҵ��ģ������ */
    PPP_HDLC_HARD_BUF_CVT_FUNC          pBufPhy2VirtFunc;       /* HDLC�ڴ�ʵ��ַת���ַ������ҵ��ģ������ */
    PPP_HDLC_HARD_BUF_CVT_FUNC          pBufVirt2PhyFunc;       /* HDLC�ڴ����ַתʵ��ַ������ҵ��ģ������ */
    VOS_UINT32                          ulUserPid;              /* ҵ��ģ��PID */
    PPP_HDLC_HARD_WRITE_REG_FUNC        pWrite32RegFunc;        /* д�Ĵ������� */
    PPP_HDLC_HARD_READ_REG_FUNC         pRead32RegFunc;         /* ���Ĵ������� */
    PPP_HDLC_HARD_IO_CVT_FUNC           pIOAddrCvtFunc;         /* IO��ַӳ�亯�� */
    VOS_UINT32                          ulDefOneMaxSize;        /* ���װ���뵥��������󳤶� */
}PPP_HDLC_HARD_CONFIG_INFO_STRU;


/*****************************************************************************
  8 UNION����
*****************************************************************************/



/*****************************************************************************
  9 ȫ�ֱ�������
*****************************************************************************/
extern PPP_HDLC_HARD_CONFIG_INFO_STRU          g_stPppHdlcConfig;


/*****************************************************************************
  10 ��������
*****************************************************************************/
VOS_UINT32 PPP_HdlcHardConfig(PPP_HDLC_HARD_CONFIG_INFO_STRU *pstHdlcConfig);

VOS_UINT8* PPP_HDLC_HARD_BufPhy2Virt(VOS_UINT32 ulPAddr);
VOS_UINT8* PPP_HDLC_HARD_BufVirt2Phy(VOS_UINT32 ulVAddr);

VOS_UINT8* PPP_HDLC_HARD_MemPhy2Virt(VOS_UINT32 ulPAddr);
VOS_UINT8* PPP_HDLC_HARD_MemVirt2Phy(VOS_UINT32 ulVAddr);

VOS_UINT_PTR PPP_HDLC_HARD_IOAddrCvt(VOS_UINT32 ulAddr);



#pragma pack()


#ifdef __cplusplus
    #if __cplusplus
            }
    #endif
#endif


#endif


