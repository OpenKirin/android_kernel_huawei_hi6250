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
#include "hdlc_hardware_driver.h"
#include "product_config.h"
#include "mdrv.h"
#include "soc_hdlc_interface.h"
#include "TTFUtil.h"
#include "hdlc_hardware_conf.h"

#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif

/*****************************************************************************
   Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_HDLC_HARDWARE_DRIVER_C


/******************************************************************************
   2 �ⲿ������������
******************************************************************************/


/*****************************************************************************
   3 ˽�ж���
*****************************************************************************/


/*****************************************************************************
   4 ȫ�ֱ�������
*****************************************************************************/

/* HDLC���������Ϣ */
PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU   g_stHdlcConfigInfo;

/******************************************************************************
   5 ����ʵ��
******************************************************************************/

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardFrmIsr
 ��������  : HDLC��װ�ж����֪ͨ�������
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardFrmIsr(unsigned int ulPara)
{
    PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU  *pstConf = PPP_DRIVER_HDLC_HARD_GET_CONFIG;

    g_stHdlcRegSaveInfo.ulHdlcFrmRawInt = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_RAW_INT_ADDR(HDLC_IP_BASE_ADDR));
    g_stHdlcRegSaveInfo.ulHdlcFrmStatus = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_STATUS_ADDR(HDLC_IP_BASE_ADDR));

    /* �յ�һ���жϺ����ԭʼ�ж� */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_INT_CLR_ADDR(HDLC_IP_BASE_ADDR), 0xFFFFFFFFU);

    /* �ͷŷ�װ����ź��� */
    /*lint -e(455) VOS_SmV�������, �������� */
    VOS_SmV(pstConf->ulHdlcFrmMasterSem);

    g_PppHdlcHardStat.ulFrmIsrCnt++;

    /* drv�ṩ�Ľӿڲ����ķ���ֵ */
    return 1; /* IRQ_HANDLED; */
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardDefIsr
 ��������  : HDLC���װ�ж����֪ͨ�������
 �������  : ��
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardDefIsr(unsigned int ulPara)
{
    PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU  *pstConf = PPP_DRIVER_HDLC_HARD_GET_CONFIG;

    g_stHdlcRegSaveInfo.ulHdlcDefRawInt = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_RAW_INT_ADDR(HDLC_IP_BASE_ADDR));
    g_stHdlcRegSaveInfo.ulHdlcDefStatus = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_STATUS_ADDR(HDLC_IP_BASE_ADDR));

    /* �յ�һ���жϺ����ԭʼ�ж� */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_INT_CLR_ADDR(HDLC_IP_BASE_ADDR), 0xFFFFFFFFU);

    /* �ͷŷ�װ����ź��� */
    /*lint -e(455) VOS_SmV�������, �������� */
    VOS_SmV(pstConf->ulHdlcDefMasterSem);

    g_PppHdlcHardStat.ulDefIsrCnt++;


    /* drv�ṩ�Ľӿڲ����ķ���ֵ */
    return 1; /* IRQ_HANDLED; */
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardGetPeriphClkStatus
 ��������  : ��ѯHDLCʱ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : VOS_TRUE:�� VOS_FALSE:��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��1��3��
    ��    ��   : c00184031
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_BOOL PPP_Driver_HdlcHardGetPeriphClkStatus(VOS_VOID)
{
    VOS_UINT32      ulValue = 0;

    ulValue = PPP_HDLC_READ_32REG(HDLC_CRG_CLKSTA4_ADDR(PPP_DRIVER_HDLC_HARD_GET_SC_BASE_ARRD));
    ulValue &= (1 << HDLC_CRG_CLK_BITPOS);

    /* ���Ϊ��˵����ǰʱ��δ���� */
    if (0 == ulValue)
    {
        return VOS_FALSE;
    }
    else
    {
        return VOS_TRUE;
    }
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardPeriphClkOpen
 ��������  : ��HDLCʱ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��6��13��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardPeriphClkOpen(VOS_VOID)
{
    VOS_UINT32      ulValue = 0;

    ulValue |= (1 << HDLC_CRG_CLK_BITPOS);
    PPP_HDLC_WRITE_32REG(HDLC_CRG_CLKEN4_ADDR(PPP_DRIVER_HDLC_HARD_GET_SC_BASE_ARRD), ulValue);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardPeriphClkClose
 ��������  : �ر�HDLCʱ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��6��13��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardPeriphClkClose(VOS_VOID)
{
    VOS_UINT32      ulValue = 0;

    ulValue |= (1 << HDLC_CRG_CLK_BITPOS);
    PPP_HDLC_WRITE_32REG(HDLC_CRG_CLKENDIS4_ADDR(PPP_DRIVER_HDLC_HARD_GET_SC_BASE_ARRD), ulValue);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardInit
 ��������  : ��HDLC���г�ʼ��: HDLC���÷����쳣ʱ, ���쳣�ж��ϱ��Ŀ���
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
VOS_UINT32 PPP_Driver_HdlcHardInit
(
    PPP_DRIVER_HDLC_HARD_FRM_CONFIG_STRU   *pstFrmConfig,
    PPP_DRIVER_HDLC_HARD_DEF_CONFIG_STRU   *pstDefConfig
)
{
    VOS_UINT_PTR                          ulBaseAddr;
    PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU  *pstHdlcConf = PPP_DRIVER_HDLC_HARD_GET_CONFIG;

    PPP_HDLC_HARD_MEM_SET(PPP_DRIVER_HDLC_HARD_GET_CONFIG, sizeof(PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU),
                          0x00, sizeof(PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU));

    /* ��ȡHDLC����ַ */
    ulBaseAddr              = (VOS_UINT_PTR)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_HDLC);

    HDLC_IP_BASE_ADDR       = PPP_HDLC_HARD_IO_ADDRESS(ulBaseAddr);

    if (VOS_NULL_PTR == HDLC_IP_BASE_ADDR)
    {
        PPP_HDLC_ERROR_LOG1("HDLC base addr is null", ulBaseAddr);
        return VOS_ERR;
    }

#if ((SC_CTRL_MOD_P532 == SC_CTRL_MOD) || (SC_CTRL_MOD_6932_SFT == SC_CTRL_MOD))
    ulBaseAddr              = (VOS_UINT_PTR)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_SYSCTRL_PD);
#elif ((SC_CTRL_MOD_6250_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_3660_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_KIRIN970_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_M533 == SC_CTRL_MOD))
    /* =========dallas/chicago/boston��ʹ��: HDLC�Ĵ�������ַλ��======== */
    ulBaseAddr              = (VOS_UINT_PTR)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_SYSCTRL_MDM);
#else
    ulBaseAddr              = (VOS_UINT_PTR)mdrv_misc_get_ip_baseaddr(BSP_IP_TYPE_SYSCTRL);
#endif

    pstHdlcConf->ulHdlcScCtrlBaseAddr  = PPP_HDLC_HARD_IO_ADDRESS(ulBaseAddr);

    if (VOS_NULL_PTR == pstHdlcConf->ulHdlcScCtrlBaseAddr)
    {
        PPP_HDLC_ERROR_LOG1("PPP_HDLC_HARD_Init HDLC SCCTRL base addr is null,0x%x\r\n",
                      ulBaseAddr);
        return VOS_ERR;
    }

#if ((SC_CTRL_MOD_KIRIN970_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_M533 == SC_CTRL_MOD))
    /* Boston�汾������������װ�Ĵ���ʱ�п��������뿪ʱ��ָ��δִ����ϵ�������ʧ�� */
    PPP_Driver_HdlcHardPeriphClkOpen();
#else
    /* �ر�HDLCʱ�� */
    PPP_Driver_HdlcHardPeriphClkClose();
#endif

    /*��ȡHDLC���װ�жϺ�*/
    pstHdlcConf->slHdlcISRDef   = mdrv_int_get_num(BSP_INT_TYPE_HDLC_DEF);

    /*��ȡHDLC��װ�жϺ�*/
    pstHdlcConf->slHdlcISRFrm   = mdrv_int_get_num(BSP_INT_TYPE_HDLC_FRM);

    if ( VOS_OK != VOS_SmBCreate("HdlcDefMasterSem", 0, VOS_SEMA4_FIFO, (VOS_SEM *)&(pstHdlcConf->ulHdlcDefMasterSem)) )
    {
        PPP_HDLC_ERROR_LOG("PPP_HDLC_HARD_Init, ERROR, Create g_ulHdlcDefMasterSem failed!\r\n");
        return VOS_ERR;
    }

    if ( VOS_OK != VOS_SmBCreate("HdlcFrmMasterSem", 0, VOS_SEMA4_FIFO, (VOS_SEM *)&(pstHdlcConf->ulHdlcFrmMasterSem)) )
    {
        PPP_HDLC_ERROR_LOG("PPP_HDLC_HARD_Init, ERROR, Create g_ulHdlcFrmMasterSem failed!\r\n");
        return VOS_ERR;
    }

    /* �жϹҽ� */
    if (VOS_OK != mdrv_int_connect(pstHdlcConf->slHdlcISRDef, (VOIDFUNCPTR)PPP_Driver_HdlcHardDefIsr, 0))
    {
        PPP_HDLC_ERROR_LOG1("PPP_HDLC_HARD_Init, ERROR, Connect slHdlcISRDef %d to PPP_HDLC_HARD_DefIsr failed!\r\n",
                      pstHdlcConf->slHdlcISRDef);
        return VOS_ERR;
    }

    /* �ж�ʹ�� */
    if (VOS_OK != mdrv_int_enable(pstHdlcConf->slHdlcISRDef))
    {
        PPP_HDLC_ERROR_LOG1("PPP_HDLC_HARD_Init, ERROR, Enable slHdlcISRDef %d failed!\r\n",
                      pstHdlcConf->slHdlcISRDef);
        return VOS_ERR;
    }

    /* �жϹҽ� */
    if (VOS_OK != mdrv_int_connect(pstHdlcConf->slHdlcISRFrm, (VOIDFUNCPTR)PPP_Driver_HdlcHardFrmIsr, 0))
    {
        PPP_HDLC_ERROR_LOG1("PPP_HDLC_HARD_Init, ERROR, Connect slHdlcISRFrm %d to PPP_HDLC_HARD_FrmIsr failed!\r\n",
                      pstHdlcConf->slHdlcISRFrm);
        return VOS_ERR;
    }

    if (VOS_OK != mdrv_int_enable(pstHdlcConf->slHdlcISRFrm))
    {
        PPP_HDLC_ERROR_LOG1("PPP_HDLC_HARD_Init, ERROR, Enable slHdlcISRFrm %d failed!\r\n",
                      pstHdlcConf->slHdlcISRFrm);
        return VOS_ERR;
    }

    pstHdlcConf->ulHdlcDefIntLimit = HDLC_DEF_INTERRUPT_LIMIT_DEFAULT;
    pstHdlcConf->ulHdlcFrmIntLimit = HDLC_FRM_INTERRUPT_LIMIT_DEFAULT;


    PPP_HDLC_HARD_MEM_CPY(&(pstHdlcConf->stFrmConfig), sizeof(PPP_DRIVER_HDLC_HARD_FRM_CONFIG_STRU),
                            pstFrmConfig, sizeof(PPP_DRIVER_HDLC_HARD_FRM_CONFIG_STRU));
    PPP_HDLC_HARD_MEM_CPY(&(pstHdlcConf->stDefConfig), sizeof(PPP_DRIVER_HDLC_HARD_DEF_CONFIG_STRU),
                            pstDefConfig, sizeof(PPP_DRIVER_HDLC_HARD_DEF_CONFIG_STRU));

#if ((SC_CTRL_MOD_KIRIN970_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_M533 == SC_CTRL_MOD))
    if (VOS_TRUE == PPP_Driver_HdlcHardGetPeriphClkStatus())
    {
        /* Boston�汾�����÷�װ��Ϣ����󳤶ȼĴ���0x5DF(1503)Bytes */
        PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_INFO_MAX_LEN_ADDR(HDLC_IP_BASE_ADDR), HDLC_MAX_FRM_DEF_INFO_LEN);
        PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_INFO_MAX_LEN_ADDR(HDLC_IP_BASE_ADDR), HDLC_MAX_FRM_DEF_INFO_LEN);
    }

    PPP_Driver_HdlcHardPeriphClkClose();

#endif

    return VOS_OK;
}    /* link_HDLCInit */


/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardWorkStatus
 ��������  : ��ȡHDLC����״̬
 �������  : ��
 �������  : penFrmWork     - ��װ����״̬
             penDefWork     - ���װ����״̬
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardWorkStatus
(
    VOS_BOOL                           *penFrmWork,
    VOS_BOOL                           *penDefWork
)
{
    VOS_UINT32                          ulFrmValue;
    VOS_UINT32                          ulDefValue;

    /* SoC���ڴ�����һ�����������ʱ���Զ���ʹ��λ���� */
    ulFrmValue = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_EN_ADDR(HDLC_IP_BASE_ADDR));
    ulDefValue = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_EN_ADDR(HDLC_IP_BASE_ADDR));

    if (0x01 == (ulFrmValue & 0x01))
    {
        *penFrmWork = VOS_TRUE;
    }
    else
    {
        *penFrmWork = VOS_FALSE;
    }

    if (0x01 == (ulDefValue & 0x01))
    {
        *penDefWork = VOS_TRUE;
    }
    else
    {
        *penDefWork = VOS_FALSE;
    }

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardSetDefIntLimit
 ��������  : ���ý��װ�ж�ˮ��
 �������  : ulIntLimit - �ж�ˮ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardSetDefIntLimit(VOS_UINT32 ulIntLimit)
{
    PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU  *pstHdlcConf = PPP_DRIVER_HDLC_HARD_GET_CONFIG;

    pstHdlcConf->ulHdlcDefIntLimit = ulIntLimit;

    return pstHdlcConf->ulHdlcDefIntLimit;
}


/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHradSetFrmIntLimit
 ��������  : ���÷�װ�ж�ˮ��
 �������  : ulIntLimit - �ж�ˮ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHradSetFrmIntLimit(VOS_UINT32 ulIntLimit)
{
    PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU  *pstHdlcConf = PPP_DRIVER_HDLC_HARD_GET_CONFIG;

    pstHdlcConf->ulHdlcFrmIntLimit = ulIntLimit;

    return pstHdlcConf->ulHdlcFrmIntLimit;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardCommCfgReg
 ��������  : ����ͨ�üĴ���
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
VOS_VOID PPP_Driver_HdlcHardCommCfgReg(VOS_VOID)
{
    /*
    prior_timeout_ctrl(0x04)
      31  24   23                16   15  9         8             7   2    1          0
    |--------|----------------------|-------|-------------------|-------|---------------|
    |   Rsv  | axireq_timeout_value |  Rsv  | axireq_timeout_en |  Rsv  |hdlc_prior_ctrl|

    Reserved             [31:24] 8'b0   h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    axireq_timeout_value [23:16] 8'b0   h/s R/W  �������AXI���߶�д����ʱ���ж�ֵ
    Reserved             [15:9]  2'b0   h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    axireq_timeout_en    [8]     1'b0   h/s R/W  �Ƿ�����Ӳ���ж�AXI���߶�д����ʱ����������ã�
                                                   0������
                                                   1����
    Reserved             [7:2]   1'b0   h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    hdlc_prior_ctrl      [1:0]   1'b0   h/s R/W  HDLC��װ�����װ���ȼ����üĴ�����
                                                    00��һ���ڹ����У���һ��Ҳ��ʹ������£��Ƚ��Ѵ��ڹ����е��Ǹ������꣬
                                                        ��˭�ȱ�ʹ�����Ƚ�˭�����ꣻ
                                                    01�����з�װ���ȼ��ߣ�
                                                    10�����н��װ���ȼ��ߣ�
                                                    11����Ч��
                                                    (HDLC�ڲ����Ʊ���ͣ�ߵļ�����ʼ�����������װ����ͣ����װ���������������
                                                    ���װ�Ϳ�ʼ��������������װ����ͣ�����װ�������ݰ���������󣬷�װ�Ϳ�ʼ����������)
    */

    VOS_UINT32                          ulValue = 0x0;


    /* ʹ��AXI����ʱ�жϣ�debugʱʹ�ã�����HDLC���ó�ʱʱ����̣�����������ģʽ�²����� */
/*    SET_BIT_TO_DWORD(ulValue, 8); */

    /* ����AXI����ʱʱ������ֵ��SoC�ṩ�����ұ�֤��ƽ̨���� */
    ulValue |= (HDLC_AXI_REQ_TIMEOUT_VALUE << 16);

    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_PRIROR_TIMEOUT_CTRL_ADDR(HDLC_IP_BASE_ADDR), ulValue);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardCommWaitSem
 ��������  : �ȴ���װ���װ��ͣ�����
 �������  : ulHdlcMasterSem    -       �ȴ���װ����װ�ź���
             ulSemTimeoutLen    -       �ȴ���װ����װ��Ӧ�ж�ʱ��
 �������  :
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
/*lint -e{454} VOS_SmP�������, �������� */
VOS_UINT32 PPP_Driver_HdlcHardCommWaitSem
(
    VOS_UINT32          ulHdlcMasterSem,
    VOS_UINT32          ulSemTimeoutLen
)
{
    VOS_UINT32                          ulResult;

    /* �ȴ���װ����װ��� */
    ulResult = VOS_SmP(ulHdlcMasterSem, ulSemTimeoutLen);

    if (VOS_OK != ulResult)
    {
        PPP_HDLC_WARNING_LOG2("PPP_HDLC_HARD_CommWaitSem, WARNING, VOS_SmP ulHdlcMasterSem 0x%x failed! ErrorNo = 0x%x\r\n",
                      ulHdlcMasterSem, ulResult);

        g_PppHdlcHardStat.usDefExpInfo |=   (1 << HDLC_SEM_TIMEOUT_IND_BITPOS);
        g_PppHdlcHardStat.usFrmExpInfo |=   (1 << HDLC_SEM_TIMEOUT_IND_BITPOS);

        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardFrmCfgBufReg
 ��������  : ���÷�װʹ�õ��ڴ�����ؼĴ���
 �������  : pstFrmBuffInfo -   ��װʹ�õ��ڴ���Ϣ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��20��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardFrmCfgBufReg(VOS_VOID)
{
    PPP_DRIVER_HDLC_HARD_FRM_CONFIG_STRU   *pstFrmConf;

    pstFrmConf = PPP_DRIVER_HDLC_HARD_GET_FRM_CONF;

    /* ����װ��������������ʼ��ַ���ø��Ĵ���frm_in_lli_addr */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_IN_LLI_ADDR(HDLC_IP_BASE_ADDR),
            (VOS_UINT32)pstFrmConf->pInputAddr);

    /* ����װ��������������ʼ��ַ���ø��Ĵ���frm_out_lli_addr */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_OUT_LLI_ADDR(HDLC_IP_BASE_ADDR),
            (VOS_UINT32)pstFrmConf->pOutputAddr);

    /* ����װ��Ч֡�����Ϣ�ϱ��ռ���ʼ��ַ���ø��Ĵ���frm_rpt_addr */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_RPT_ADDR(HDLC_IP_BASE_ADDR),
            (VOS_UINT32)pstFrmConf->pReportAddr);

    /* ����װ��Ч֡�����Ϣ�ϱ��ռ�������ø��Ĵ���frm_rpt_dep��[15:0]λ */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_RPT_DEP_ADDR(HDLC_IP_BASE_ADDR),
                    (VOS_UINT32)(pstFrmConf->ulRptBufLen & 0xFFFF));

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardFrmCfgReg
 ��������  : IPģʽ�����÷�װ��ؼĴ���
 �������  : ulAccmFlag     -       ͬ�첽������
             ulFrmCfg       -       ���װ����
 �������  :
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��20��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardFrmCfgReg
(
    PPP_DRIVER_HDLC_HARD_FRM_PARA_STRU *pstDrvFrmPara
)
{
    VOS_UINT32                          ulFrmCfg = 0;

    /*
    hdlc_frm_cfg   (0x20)
      31              16 15           4 3      2   1      0
    |-------------------|--------------|--------|------|------|
    |      protocol     |      Rsv     |   Pfc  | Acfc |1dor2d|

    frm_protocol         [31:16] 16'b0   h/s R/W  ��װЭ��ֵ����ЧЭ��ֵ�涨�μ�����б�
    Reserved             [15:4]  12'b0   h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    frm_pfc              [3:2]   2'b0    h/s R/W  P��ѹ��ָʾ��00��Ӳ��ģ�����P��P����ѹ��;
                                                               01��Ӳ��ģ�����P��P��ѹ��;
                                                               11��Ӳ��ģ�鲻���P��;
                                                               ��������Ч;
    frm_acfc             [1]     1'b0    h/s R/W  AC��ѹ��ָʾ��0��AC����ѹ��;1����ʾAC��ѹ��;
    frm_in_lli_1dor2d    [0]     1'b0    h/s R/W  ��װ����һά���ά����ѡ��ָʾ�Ĵ�����
                                                                0Ϊһά;1Ϊ��ά;
    */

    ulFrmCfg |= (pstDrvFrmPara->ulPppPcFlag << HDLC_FRM_PFC_BITPOS);
    if (HDLC_ADDRESS_CTRL_COMPRESS == pstDrvFrmPara->ulPppAcFlag)
    {
        TTF_SET_A_BIT(ulFrmCfg, HDLC_FRM_ACFC_BITPOS);
    }
    ulFrmCfg |= ( ((VOS_UINT32)pstDrvFrmPara->usProtocol) << 16 );

    /* ���Ĵ���hdlc_frm_cfg��[0]λfrm_in_lli_1dor2d����Ϊ0 */

    /* ����hdlc_frm_cfg�� P��� AC�� */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_ACCM_ADDR(HDLC_IP_BASE_ADDR), pstDrvFrmPara->ulAccmFlag);

    /* ���üĴ���hdlc_frm_cfg��[31:16]λfrm_protocolΪusProtocol */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_CFG_ADDR(HDLC_IP_BASE_ADDR), ulFrmCfg);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardTraceRegConfig
 ��������  : ��ȡ�Ĵ�����������
 �������  : ulEnable   -   ��װ���װʹ��ǰ����ʹ�ܺ�VOS_TRUE: ʹ�ܺ�,VOS_FALSE: ʹ��ǰ
             ulValue    -   ʹ�ܼĴ������õ�ֵ
             ulEnableInterrupt - �жϷ�ʽ������ѯ��ʽ������ʹ�ܺ�ȡ�Ĵ���ʱ��Ч
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��20��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardTraceFrmRegConfig
(
    VOS_UINT32                              ulEnable,
    VOS_UINT32                              ulValue,
    VOS_UINT32                              ulEnableInterrupt,
    PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU  *pstFrmRegInfo
)
{
    HDLC_MNTN_FRM_REG_CONFIG_STRU           stRegMntn;
    PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU  *pstRegConfig = &(stRegMntn.stFrmRegInfo);
    VOS_UINT32                              ulDataLen;

    /* �ڲ����ñ�֤�����ǿ� */

    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_REG))
    {
        ulDataLen    = sizeof(HDLC_MNTN_FRM_REG_CONFIG_STRU);

        /* ����ȫ���Ĵ������� */
        PPP_HDLC_HARD_MEM_CPY(pstRegConfig, sizeof(PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU),
                     pstFrmRegInfo, sizeof(PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU));

        /* ʹ��ǰ������ʹ�ܼĴ�����û�����ã���Ϊ����֮��HDLC�Ὺʼ��������ı������Ĵ�����ֵ */
        if( VOS_FALSE == ulEnable )
        {
            pstRegConfig->ulHdlcFrmEn    = ulValue;
            PPP_HDLC_HARD_MntnTraceMsg((HDLC_MNTN_TRACE_HEAD_STRU *)&stRegMntn,
                                       ID_HDLC_MNTN_FRM_REG_BEFORE_EN, ulDataLen);
        }
        else
        {
            /* ʹ�ܺ󹴰�ʱ����������жϷ�ʽ����RawInt��Statusȡg_stHdlcRegSaveInfo�����ֵ */
            if( VOS_TRUE == ulEnableInterrupt )
            {
                pstRegConfig->ulHdlcFrmRawInt   = g_stHdlcRegSaveInfo.ulHdlcFrmRawInt;
                pstRegConfig->ulHdlcFrmStatus   = g_stHdlcRegSaveInfo.ulHdlcFrmStatus;
            }
            PPP_HDLC_HARD_MntnTraceMsg((HDLC_MNTN_TRACE_HEAD_STRU *)&stRegMntn,
                                       ID_HDLC_MNTN_FRM_REG_AFTER_EN, ulDataLen);
        }
    }

    return;
}


/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardFrmCfgEnReg
 ��������  : ����ʹ�ܼĴ���
 �������  : ulTotalLen --����װ���ݰ����ܳ���
 �������  :
 �� �� ֵ  : VOS_TRUE   - �����ж�ģʽ
             VOS_FALSE  - ������ѯģʽ
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��20��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardFrmCfgEnReg(VOS_UINT32 ulTotalLen)
{
    PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU   stRegConfig;


    /*
    1.hdlc_frm_en   (0x10)
      31   25 24  23 18 17  16  15  14  13  12  11  10   9   8  7    1  0
    |--------|---|-----|---|---|---|---|---|---|---|---|---|---|------|---|
    |   Rsv  |en | Rsv |en |en |en |en |en |en |en |en |en |en |  Rsv |en |

    Reserved            [31:25] 7'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    frm_over_int_en     [24]    1'b0    h/s R/W  һ�������װ�����ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��;
    Reserved            [23:18] 6'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    frm_rpt_dep_err_en  [17]    1'b0    h/s R/W  ��װ�ⲿ��ȷ֡�����ϱ��ռ䲻���ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��;
    frm_out_spc_err_en  [16]    1'b0    h/s R/W  ��װ�ⲿ����洢�ռ䲻���ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    frm_rpt_prm_err_en  [15]    1'b0    h/s R/W  ��װ�ϱ��ռ���ز��������ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    frm_out_prm_err_en  [14]    1'b0    h/s R/W  ��װ���������ز��������ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    frm_in_prm_err_en   [13]    1'b0    h/s R/W  ��װ����������ز��������ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    frm_cfg_err_en      [12]    1'b0    h/s R/W  ��װЭ�鼰��ѹ��ָʾ���ô����ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    frm_wr_timeout_en   [11]    1'b0    h/s R/W  ��װʱAXI����д����timeout�ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    frm_rd_timeout_en   [10]    1'b0    h/s R/W  ��װʱAXI���߶�����timeout�ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    frm_wr_err_en       [9]     1'b0    h/s R/W  ��װʱAXI����д���������ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    frm_rd_err_en       [8]     1'b0    h/s R/W  ��װʱAXI���߶����������ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    Reserved            [7:1]   7'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    frm_en              [0]     1'b0    h/s R/W  һ�������װʹ�ܣ������frm_enд��1'b1������װ����;һ�������װ��ɺ���Ӳ���Զ���frm_en���㣻
                                                 ��װ���̳���ʱ��Ӳ��Ҳ���frm_en�Զ����㣬ʹ�ڲ�״̬������IDLE״̬��
                                                 дʱ����һ�������װʹ��;0����ʹ�ܷ�װ����;1��ʹ�ܷ�װ����;
                                                 ��ʱ����һ�������װ����״̬;0��û�ڽ��з�װ����;1�����ڽ��з�װ����
    */

    VOS_UINT32          ulEnableInterrupt;
    VOS_UINT32          ulValue;
    const VOS_UINT32    ulInterruptValue    = 0x0103FF01;   /* ʹ���жϷ�ʽʱ����ʹ�ܼĴ�����ֵ */
    const VOS_UINT32    ulPollValue         = 0x01;         /* ʹ����ѯ��ʽʱ����ʹ�ܼĴ�����ֵ */


    /* �жϴ���װ���ݵ��ܳ��ȣ�������������ʹ���жϷ�ʽ������ʹ����ѯ��ʽ */
    if( ulTotalLen > HDLC_FRM_INTERRUPT_LIMIT )
    {
        /* ���÷�װ���ʹ�ܼĴ���hdlc_frm_en��[31:0]λΪ0x0103FF01 */
        ulValue             = ulInterruptValue;
        ulEnableInterrupt   = VOS_TRUE;

        g_PppHdlcHardStat.ulFrmWaitIntCnt++;
    }
    else
    {
        /* ���÷�װ���ʹ�ܼĴ���hdlc_frm_en��[31:0]λΪ0x01 */
        ulValue             = ulPollValue;
        ulEnableInterrupt   = VOS_FALSE;

        g_PppHdlcHardStat.ulFrmWaitQueryCnt++;
    }

    /* ʹ��ǰ����ϴη�װ�����װ��ԭʼ�ж� */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_INT_CLR_ADDR(HDLC_IP_BASE_ADDR), 0xFFFFFFFFU);
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_INT_CLR_ADDR(HDLC_IP_BASE_ADDR), 0xFFFFFFFFU);

    /* �ϱ��Ĵ�����ά�ɲ� */
    PPP_Driver_HdlcHardGetFrmRegInfo(&stRegConfig);
    PPP_Driver_HdlcHardTraceFrmRegConfig(VOS_FALSE, ulValue, ulEnableInterrupt, &stRegConfig);

    /* ʹ��Ӳ��֮ǰ��ǿ��ARM˳��ִ�н���ǰ���ָ�� */
    TTF_FORCE_ARM_INSTUCTION();

    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_EN_ADDR(HDLC_IP_BASE_ADDR), ulValue);

    return ulEnableInterrupt;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardFrmWaitStatusChange
 ��������  : ��ѯ״̬�Ĵ���
 �������  : ��
 �������  :
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardFrmWaitStatusChange(VOS_VOID)
{
    VOS_UINT32              ulFrmRsltWaitNum;           /* ��ֹӲ���쳣�ı������� */
    volatile VOS_UINT32     ulFrmStatus = 0;            /* ��װ״̬ */


   /* ��ѯhdlc_frm_status (0x28)�ĵ�[0]λ�͵�[1]λ���κ�һ��Ϊ1���߳�ʱ�򷵻� */
    ulFrmRsltWaitNum = 0UL;

    while (ulFrmRsltWaitNum < HDLC_FRM_MAX_WAIT_RESULT_NUM)
    {
        /* ��ȡ hdlc_frm_status��[0][1]λ */
        ulFrmStatus  =   PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_STATUS_ADDR(HDLC_IP_BASE_ADDR));

        if (HDLC_FRM_ALL_PKT_DOING != (ulFrmStatus & HDLC_FRM_STATUS_MASK))
        {
            break;
        }

        ulFrmRsltWaitNum++;
    }

    if ( HDLC_FRM_MAX_WAIT_RESULT_NUM <= ulFrmRsltWaitNum )
    {
        PPP_HDLC_WARNING_LOG2("PPP_HDLC_HARD_FrmWaitStatusChange, WARNING, wait hdlc_frm_status timeout %d status 0x%x!\r\n",
                      ulFrmRsltWaitNum, ulFrmStatus);

        g_PppHdlcHardStat.usFrmExpInfo |=   (1 << HDLC_WAIT_STATUS_TIMEOUT_IND_BITPOS);

        return VOS_ERR;
    }

    g_PppHdlcHardStat.ulFrmMaxQueryCnt = TTF_MAX(g_PppHdlcHardStat.ulFrmMaxQueryCnt, ulFrmRsltWaitNum);

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardFrmWaitResult
 ��������  : �ȴ���װ��ͣ�����
 �������  : ulEnableInterrupt  -   �ж��Ƿ�ʹ��
 �������  : ��
 �� �� ֵ  : ��װ״̬
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��20��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardFrmWaitResult
(
    VOS_UINT32                              ulEnableInterrupt
)
{
    VOS_UINT32                              ulFrmStatus;        /* ��װ״̬ */
    VOS_UINT32                              ulResult;
    PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU   stRegConfig;
    PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU  *pstHdlcConf = PPP_DRIVER_HDLC_HARD_GET_CONFIG;


    if (VOS_TRUE == ulEnableInterrupt)
    {
        /* �ȴ��жϵõ���������״̬ */
        ulResult = PPP_Driver_HdlcHardCommWaitSem(pstHdlcConf->ulHdlcFrmMasterSem, HDLC_FRM_MASTER_INT_TIMER_LEN);

        /* �������жϷ�������н��������жϲ�������Statusָʾ�Ƿ�����bit��ԭʼ�жϼĴ���
           �������ʴ˴�ȡ������g_stHdlcRegSaveInfo�е�״ֵ̬ */
        ulFrmStatus = g_stHdlcRegSaveInfo.ulHdlcFrmStatus;
    }
    else
    {
        /* ��ѯ�õ��������� */
        ulResult = PPP_Driver_HdlcHardFrmWaitStatusChange();

        /* ��ѯhdlc_frm_status (0x28)��ȡ��װ״̬�����䷵�� */
        ulFrmStatus  =  PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_STATUS_ADDR(HDLC_IP_BASE_ADDR));
    }

    /* �ϱ��Ĵ�����ά�ɲ� */
    PPP_Driver_HdlcHardGetFrmRegInfo(&stRegConfig);
    PPP_Driver_HdlcHardTraceFrmRegConfig(VOS_TRUE, 0, ulEnableInterrupt, &stRegConfig);

    /* �Ȳ���˵��HDLC���ڹ��� */
    if (VOS_OK != ulResult)
    {
        return HDLC_FRM_ALL_PKT_DOING;
    }

    ulFrmStatus &=  HDLC_FRM_STATUS_MASK;

    return ulFrmStatus;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardFrmEnable
 ��������  : ���÷�װ�Ĵ�����ʹ�ܷ�װ����
 �������  : pstDrvFrmPara      -   ��װ����
 �������  : pulEnableInterrupt -   �ж��Ƿ�ʹ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardFrmEnable
(
    PPP_DRIVER_HDLC_HARD_FRM_PARA_STRU *pstDrvFrmPara,
    VOS_UINT32                         *pulEnableInterrupt
)
{
    VOS_UINT32                          ulFrmStatus;
    VOS_UINT32                          ulEnableInterrupt;

    /* ���÷�װ�����װͨ�üĴ��� */
    PPP_Driver_HdlcHardCommCfgReg();

    /* �����ڴ���ؼĴ��� */
    PPP_Driver_HdlcHardFrmCfgBufReg();

    /* ���÷�װ��ؼĴ��� */
    PPP_Driver_HdlcHardFrmCfgReg(pstDrvFrmPara);

    /* ����ʹ�ܼĴ��������ϱ�ʹ��ǰ�Ĵ�����ά�ɲ� */
    ulEnableInterrupt = PPP_Driver_HdlcHardFrmCfgEnReg(pstDrvFrmPara->ulInputTotalSize);

    *pulEnableInterrupt = ulEnableInterrupt;

    ulFrmStatus = PPP_Driver_HdlcHardFrmWaitResult(ulEnableInterrupt);

    return ulFrmStatus;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardGetFrmResult
 ��������  : ��ȡ��װ���
 �������  : ��
 �������  : pusFrmOutSegNum    -   ��Ч֡���ռ��Ƭ�θ���
             pucFrmValidNum     -   ��Ч֡����
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��20��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardGetFrmResult
(
    VOS_UINT16                         *pusFrmOutSegNum,
    VOS_UINT8                          *pucFrmValidNum
)
{
    /* ��Ч֡���ռ��Ƭ�θ���usFrmOutSegNum= hdlc_frm_status�Ĵ���[31:16]λ��ֵ */
    *pusFrmOutSegNum = (VOS_UINT16)TTF_Read32RegByBit(SOC_ARM_HDLC_FRM_STATUS_ADDR(HDLC_IP_BASE_ADDR), 16, 31);

    /* ��Ч֡����usFrmValidNum= hdlc_frm_status�Ĵ���[15:8]λ��ֵ */
    *pucFrmValidNum  = (VOS_UINT8)TTF_Read32RegByBit(SOC_ARM_HDLC_FRM_STATUS_ADDR(HDLC_IP_BASE_ADDR), 8, 15);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardGetFrmRawInt
 ��������  : ��ȡ��װ�жϼĴ���ֵ
 �������  : ��
 �������  : ��
 �� �� ֵ  : VOS_UINT32  ��װ�жϼĴ���ֵ
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��20��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardGetFrmRawInt(VOS_VOID)
{
    VOS_UINT32                          ulRawInt;

    ulRawInt  =   PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_RAW_INT_ADDR(HDLC_IP_BASE_ADDR));

    return ulRawInt;
}


/************************************************************,*****************
 �� �� ��  : PPP_Driver_HdlcHardDefCfgBufReg
 ��������  : ���ý��װʹ�õ��ڴ�����ؼĴ���
 �������  : pstDrvDefPara    -   ���װʹ�õ��ڴ���Ϣ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardDefCfgBufReg(VOS_VOID)
{
    PPP_DRIVER_HDLC_HARD_DEF_CONFIG_STRU   *pstDefConf;

    pstDefConf = PPP_DRIVER_HDLC_HARD_GET_DEF_CONF;

    /* ����װ��������������ʼ��ַ���ø��Ĵ���def_in_lli_addr(0x90) */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_IN_LLI_ADDR(HDLC_IP_BASE_ADDR),
                    (VOS_UINT32)pstDefConf->pInputAddr);

    /* ����װ��������������ʼ��ַ���ø��Ĵ���def_out_spc_addr(0xA0) */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_OUT_SPC_ADDR(HDLC_IP_BASE_ADDR),
                    (VOS_UINT32)pstDefConf->pOutputAddr);

    /* ����װ��������������ʼ��ַ���ø��Ĵ���def_out_space_dep(0xA4)��16λ */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_OUT_SPACE_DEP_ADDR(HDLC_IP_BASE_ADDR),
                    (VOS_UINT32)(pstDefConf->ulOutputBufLen & 0xFFFF));

#ifndef PPPC_HDLC_NOC_ST_TEST
    /* ����װ��Ч֡�����Ϣ�ϱ��ռ���ʼ��ַ���ø��Ĵ���def_rpt_addr(0xA8) */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_RPT_ADDR(HDLC_IP_BASE_ADDR),
                    (VOS_UINT32)pstDefConf->pReportAddr);
#else
    /* ����NOC����,��CDSP��Resv�ռ�0xE39D9000���ø�HDLC */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_RPT_ADDR(HDLC_IP_BASE_ADDR), 0xE39D9000);
#endif

    /* ����װ��Ч֡�����Ϣ�ϱ��ռ�������ø��Ĵ���def_rpt_dep (0xAC)��16λ */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_RPT_DEP_ADDR(HDLC_IP_BASE_ADDR),
                    (VOS_UINT32)(pstDefConf->ulReportBufLen & 0xFFFF));

    return;
}


/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardDefCfgReg
 ��������  : ���ý��װ���üĴ���
 �������  : pstDrvDefPara    -   ���װ������Ϣ
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardDefCfgReg
(
    PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU *pstDrvDefPara
)
{
    PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU *pstUncompleteInfo;
    PPP_DRIVER_HDLC_HARD_DEF_CONFIG_STRU           *pstDefConf;
    VOS_UINT32                                      ulDefCfg = 0;


    /*
        hdlc_def_cfg  (0x70)
         31                           4 3     2   1     0
        |-------------------------------|-------|-----|-----|
        |              Rsv              |  Pfc  | Acfc| ago |
        Reserved             [31:4]  28'b0   h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
        def_pfc              [3:2]   2'b0    h/s R/W  P��ѹ��ָʾ��00��P����ѹ��������룻01��P��ѹ��������룻11��P�򲻰��룻��������Ч��
        def_acfc             [1]     1'b0    h/s R/W  AC��ѹ��ָʾ��0��AC����ѹ����1����ʾAC��ѹ����
        def_uncompleted_ago  [0]     1'b0    h/s R/W  ����ָʾ��Ӧ��ǰ���װ��������ͬһPPP/IP���ŵ���ǰ������װ���������Ƿ��н��������֡��
                                                      Ϊ��֧�ֶ��PPP/IP���Ŷ����ӵ����ã�0��û�У�1����
        */

    pstDefConf = PPP_DRIVER_HDLC_HARD_GET_DEF_CONF;

    ulDefCfg |= pstDrvDefPara->ulPppPcFlag << HDLC_DEF_PFC_BITPOS;

    if (HDLC_ADDRESS_CTRL_COMPRESS == pstDrvDefPara->ulPppAcFlag)
    {
        TTF_SET_A_BIT(ulDefCfg, HDLC_DEF_ACFC_BITPOS);
    }

    /* ���÷�����֡�����Ϣ */
    pstUncompleteInfo = pstDrvDefPara->pstUncompleteInfo;
    if ((VOS_NULL_PTR != pstUncompleteInfo)
        && (HDLC_DEF_UNCOMPLETED_EXIST == pstUncompleteInfo->ucExistFlag))
    {
        /*
        def_uncompleted_len  (0x74)
         31                 16  15                  0
        |---------------------|----------------------|
        |         Rsv         |         Len          |
        Reserved             [31:16] 16'b0   h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
        def_uncompleted_len  [15:0]  16'b0   h/s R/W  ��Ӧ��ǰ���װ��������ͬһPPP/IP���ŵ���ǰ������װ����������������֡�ĳ��ȣ�Ϊ��֧�ֶ��PPP/IP���Ŷ����ӵ�����
        */
        PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_UNCOMPLETED_LEN_ADDR(HDLC_IP_BASE_ADDR),
                        (VOS_UINT32)pstUncompleteInfo->usDefOutOneLen & 0xFFFF);

        /*
        def_uncompleted_pro  (0x78)
         31                 16  15                  0
        |---------------------|----------------------|
        |         Rsv         |         Pro          |
        Reserved             [31:16] 16'b0   h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
        def_uncompleted_pro  [15:0]  16'b0   h/s R/W  ��Ӧ��ǰ���װ��������ͬһPPP/IP���ŵ���ǰ������װ����������������֡��
                                                      Э�飬Ϊ��֧�ֶ��PPP/IP���Ŷ����ӵ����ã��������е�0Byte��1Byte��2Byte��Ч��
        */
        PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_UNCOMPLETED_PRO_ADDR(HDLC_IP_BASE_ADDR),
                        (VOS_UINT32)pstUncompleteInfo->usDefOutOnePro & 0xFFFF);

        /*
        def_uncompleted_addr  (0x7C)
         31                  0
        |----------------------|
        |         Addr         |
        def_uncompleted_addr [31:0]  32'b0   h/s R/W  ��Ӧ��ǰ���װ��������ͬһPPP/IP���ŵ���ǰ������װ����������������֡��
                                                      �ⲿ�洢��ʼ��ַ��Ϊ��֧�ֶ��PPP/IP���Ŷ����ӵ����ã��õ�ַ��������ԭ���ϱ���ͬ���µ�ַ��
        */
        PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_UNCOMPLETED_ADDR(HDLC_IP_BASE_ADDR),
                        (VOS_UINT32)pstUncompleteInfo->pucDefOutOneAddr);

        /*
        def_uncomplet_st_ago  (0x80)
         31                  16 15             5 4     0
        |----------------------|----------------|-------|
        |         Ago          |       Rsv      |  Ago  |
        crc16_result_ago     [31:16] 16'b0   h/s R/W  �뵱ǰ���װ��������ͬһPPP/IP���ŵ���ǰ������׽��װ��������������������֡ʱ��CRCУ��ֵ
        Reserved             [15:5]  11'b0   h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
        def_data_st_curr_ago [4:0]   5'b0    h/s R/W  �뵱ǰ���װ��������ͬһPPP/IP���ŵ���ǰ������׽��װ��������������������֡ʱ������״̬����ǰ״̬
        */
        PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_UNCOMPLETED_ST_AGO_ADDR(HDLC_IP_BASE_ADDR),
                        (VOS_UINT32)pstUncompleteInfo->ulDefStAgo);

        /*
        def_info_frl_cnt_ago  (0xC0)
         31        27 26                 16 15   11 10              0
        |------------|---------------------|-------|-----------------|
        |    Rsv     |         Ago         |  Rsv  |       Ago       |
        Reserved             [31:27] 5'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
        def_framel_cnt_ago   [26:16] 11'b0   h/s R/W  �뵱ǰ���װ��������ͬһPPP/IP���ŵ���ǰ������׽��װ��������������������֡ʱ��֡����
        Reserved             [15:11] 5'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
        def_info_cnt_ago     [10:0]  11'b0   h/s R/W  �뵱ǰ���װ��������ͬһPPP/IP���ŵ���ǰ������׽��װ��������������������֡ʱ����Ϣ����
        */
        PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_INFO_FRL_CNT_AGO_ADDR(HDLC_IP_BASE_ADDR),
                        (VOS_UINT32)pstUncompleteInfo->ulDefInfoFrlCntAgo);

        TTF_SET_A_BIT(ulDefCfg, HDLC_DEF_IS_UNCOMPLETED_AGO_BITPOS);
    }

    /* ����ulMode��P���AC���Ƿ�ѹ������hdlc_def_cfg (0x70) */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_CFG_ADDR(HDLC_IP_BASE_ADDR), ulDefCfg);

    /* ��������������󵥰����� */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_IN_PKT_LEN_MAX_ADDR(HDLC_IP_BASE_ADDR),
                    (VOS_UINT32)pstDefConf->ulPerInMaxLen);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardDefWaitStatusChange
 ��������  : ��ѯ״̬�Ĵ���
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
VOS_UINT32 PPP_Driver_HdlcHardDefWaitStatusChange(VOS_VOID)
{
    /*
    hdlc_def_status  (0x88)
      31 30   24 23   8 7 6  5   4   3   2  1  0
    |---|-------|------|---|---|---|---|---|----|
    |Rsv|  Type |  Num |Rsv|Idx|Ful|Ful|Now|Stat|
    Reserved             [31]    1'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    def_err_type         [30:24] 7'b0    h/s RO   ��֡�ϱ�ʱ������֡���ͣ���Ӧ��bitλΪ1���������������ʹ���
                                                  bit 30����������6��ת���ַ�0x7D�����һ��Flag��
                                                  bit 29����������5����AC����ѹ��ʱ��Address��ֵ��0xFF��
                                                  bit 28����������4����AC����ѹ��ʱ��Control��ֵ��0x03��
                                                  bit 27����������3����P�������ʱ���յ��Ƿ���Protocol��ֵ��
                                                  bit 26����������2�����װ��֡�ֽ���С��4bites��
                                                  bit 25����������1�����װ��֡�ֽ�������1502bytes��PPP֡��Information�򲻳���1500Bytes������Э���򲻳���1502Bytes����
                                                  bit 24����������0�� CRCУ�����
    def_valid_num        [23:8]  16'b0   h/s RO   ��֡�ϱ�ʱ����Ч֡��Ŀ�������������һ�����ܵķ�����֡��
    Reserved             [7:6]   2'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    def_error_index      [5]     1'b0    h/s RO   ���װ��������ָʾ
    def_rpt_ful          [4]     1'b0    h/s RO   ���װ�ⲿ��ȷ֡��Ϣ�ϱ��ռ������ָͣʾ
    def_out_spc_ful      [3]     1'b0    h/s RO   ���װ�ⲿ����洢�ռ������ָͣʾ
    def_uncompleted_now  [2]     1'b0    h/s RO   ����ָʾ��ǰ�����Ƿ��н��������֡��Ϊ��֧�ֶ��PPP/IP���Ŷ����ӵ����ã�0��û�У�1����
    def_all_pkt_pro_stat [1:0]   2'b0    h/s RO   һ������������״̬��00��δ���һ������������01��δ���һ�������������ѽ��LCP֡��Ӳ��������ͣ״̬��
                                                  10�����һ����������������֡�ϱ���11: ���һ����������������֡�ϱ���
    */
    VOS_UINT32              ulRsltWaitNum;           /* ��ֹӲ���쳣�ı������� */
    volatile VOS_UINT32     ulStatus = 0;            /* ���װ״̬ */

   /* ��ѯhdlc_frm_status (0x28)�ĵ�[0]λ�͵�[1]λ���κ�һ��Ϊ1���߳�ʱ�򷵻� */

    ulRsltWaitNum = 0UL;

    while (ulRsltWaitNum < HDLC_DEF_MAX_WAIT_RESULT_NUM)
    {
        /* ��ѯ״̬�Ĵ���hdlc_def_status (0x88)��0-1��3-5λ���κ�һλ��Ϊ1��ʾ���װģ����ͣ��ֹͣ */
        ulStatus  =   PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_STATUS_ADDR(HDLC_IP_BASE_ADDR));

        if (HDLC_DEF_STATUS_DOING != (ulStatus & HDLC_DEF_STATUS_MASK))
        {
            break;
        }

        ulRsltWaitNum++;
    }

    if ( HDLC_DEF_MAX_WAIT_RESULT_NUM <= ulRsltWaitNum )
    {
        PPP_HDLC_WARNING_LOG2("PPP_HDLC_HARD_DefWaitStatusChange, WARNING, wait hdlc_def_status timeout %d status 0x%x!\r\n",
                      ulRsltWaitNum, ulStatus);

        g_PppHdlcHardStat.usDefExpInfo |=   (1 << HDLC_WAIT_STATUS_TIMEOUT_IND_BITPOS);

        return VOS_ERR;
    }

    g_PppHdlcHardStat.ulDefMaxQueryCnt = TTF_MAX(g_PppHdlcHardStat.ulDefMaxQueryCnt, ulRsltWaitNum);

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardTraceDefRegconfig
 ��������  : ��ȡ���װ�Ĵ�����������
 �������  : ulEnable   -   ��װ���װʹ��ǰ����ʹ�ܺ�VOS_TRUE: ʹ�ܺ�,VOS_FALSE: ʹ��ǰ
             ulValue    -   ʹ�ܼĴ������õ�ֵ
             ulEnableInterrupt - �жϷ�ʽ������ѯ��ʽ������ʹ�ܺ�ȡ�Ĵ���ʱ��Ч
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��20��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardTraceDefRegconfig
(
    VOS_UINT32                              ulEnable,
    VOS_UINT32                              ulValue,
    VOS_UINT32                              ulEnableInterrupt,
    PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU  *pstDefRegInfo
)
{
    HDLC_MNTN_DEF_REG_CONFIG_STRU           stRegMntn;
    PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU  *pstRegConfig = &(stRegMntn.stDefRegInfo);
    VOS_UINT32                              ulDataLen;

    /* �ڲ����ñ�֤�����ǿ� */

    if (VOS_TRUE == PPP_HDLC_HARD_MntnGetConfig(PPP_HDLC_MNTN_TRACE_REG))
    {
        ulDataLen    = sizeof(HDLC_MNTN_DEF_REG_CONFIG_STRU);

        /* ����ȫ���Ĵ������� */
        PPP_HDLC_HARD_MEM_CPY(pstRegConfig, sizeof(PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU),
                     pstDefRegInfo, sizeof(PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU));

        /* ʹ��ǰ������ʹ�ܼĴ�����û�����ã���Ϊ����֮��HDLC�Ὺʼ��������ı������Ĵ�����ֵ */
        if( VOS_FALSE == ulEnable)
        {
            pstRegConfig->ulHdlcDefEn   = ulValue;
            PPP_HDLC_HARD_MntnTraceMsg((HDLC_MNTN_TRACE_HEAD_STRU *)&stRegMntn,
                                       ID_HDLC_MNTN_DEF_REG_BEFORE_EN, ulDataLen);
        }
        else
        {
            /* ʹ�ܺ󹴰�ʱ����������жϷ�ʽ����RawInt��Statusȡg_stHdlcRegSaveInfo�����ֵ */
            if( VOS_TRUE == ulEnableInterrupt )
            {
                pstRegConfig->ulHdlcDefRawInt   = g_stHdlcRegSaveInfo.ulHdlcDefRawInt;
                pstRegConfig->ulHdlcDefStatus   = g_stHdlcRegSaveInfo.ulHdlcDefStatus;
            }
            PPP_HDLC_HARD_MntnTraceMsg((HDLC_MNTN_TRACE_HEAD_STRU *)&stRegMntn,
                                       ID_HDLC_MNTN_DEF_REG_AFTER_EN, ulDataLen);
        }
    }

    return;
}


/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardDefWaitResult
 ��������  : �ȴ����װ��ͣ�����
 �������  : ulEnableInterrupt  -   �ж��Ƿ�ʹ��
 �������  : ���װ״̬
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardDefWaitResult
(
    VOS_UINT32                              ulEnableInterrupt
)
{
    VOS_UINT32                              ulStatus;                /* ���װ״̬ */
    VOS_UINT32                              ulResult;
    PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU   stRegConfig;
    PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU  *pstHdlcConf = PPP_DRIVER_HDLC_HARD_GET_CONFIG;


    if (VOS_TRUE == ulEnableInterrupt)
    {
        /* �ȴ��жϵõ���ͣ�����״̬ */
        ulResult = PPP_Driver_HdlcHardCommWaitSem(pstHdlcConf->ulHdlcDefMasterSem, HDLC_DEF_MASTER_INT_TIMER_LEN);

        /* �������жϷ�������н��������жϲ�������Statusָʾ�Ƿ�����bit��ԭʼ
           �жϼĴ����������ʴ˴�ȡ������g_stHdlcRegSaveInfo�е�״ֵ̬ */
        ulStatus = g_stHdlcRegSaveInfo.ulHdlcDefStatus;

    }
    else
    {
        /* ��ѯ�õ���ͣ����� */
        ulResult = PPP_Driver_HdlcHardDefWaitStatusChange();

        /* ��ѯhdlc_def_status (0x88)��ȡ���װ״̬�����䷵�� */
        ulStatus  =  PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_STATUS_ADDR(HDLC_IP_BASE_ADDR));
    }

    /* �ϱ��Ĵ�����ά�ɲ� */
    PPP_Driver_HdlcHardGetDefRegInfo(&stRegConfig);
    PPP_Driver_HdlcHardTraceDefRegconfig(VOS_TRUE, 0, ulEnableInterrupt, &stRegConfig);

    /* �Ȳ���˵��HDLC���ڹ��� */
    if (VOS_OK != ulResult)
    {
        return HDLC_DEF_STATUS_DOING;
    }

    ulStatus &=  HDLC_DEF_STATUS_MASK;

    return ulStatus;
}


/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardDefCfgEnReg
 ��������  : ���ݱ��ν��װ��������С������ʹ�úο���ʽ�ȴ�HDLC�Ľ��
 �������  : ulLinkTotalSize     -   ���ν��װ���������ڵ���������(��λ�ֽ�)
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardDefCfgEnReg
(
    VOS_UINT32                      ulTotalLen
)
{
    /*
    hdlc_def_en   (0x60)
      31   25 24  23 19 18  17  16  15  14  13  12  11  10   9   8   7   1  0
    |--------|---|-----|---|---|---|---|---|---|---|---|---|---|---|------|---|
    |   Rsv  |en | Rsv |en |en |en |en |en |en |en |en |en |en |en |  Rsv |en |

    Reserved            [31:25] 7'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    def_over_int_en     [24]    1'b0    h/s R/W  һ��������װ�����ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��;
    Reserved            [23:19] 5'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    def_rpt_ful_en      [18]    1'b0    h/s R/W  ���װ�ⲿ��ȷ֡��Ϣ�ϱ��ռ������ͣ�ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��;
    def_out_spc_ful_en  [17]    1'b0    h/s R/W  ���װ�ⲿ����洢�ռ������ͣ�ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    def_lcp_int_en      [16]    1'b0    h/s R/W  ���װ�����ЧLCP֡��ͣ�ж��ϱ�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    def_rpt_prm_err_en  [15]    1'b0    h/s R/W  ���װ�ϱ��ռ���ز��������ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    def_out_prm_err_en  [14]    1'b0    h/s R/W  ���װ����ռ���ز��������ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    def_in_prm_err_en   [13]    1'b0    h/s R/W  ���װ����������ز��������ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    def_cfg_err_en      [12]    1'b0    h/s R/W  ���װЭ��ѹ��ָʾ���ô����ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    def_wr_timeout_en   [11]    1'b0    h/s R/W  ���װʱAXI����д����timeout�ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    def_rd_timeout _en  [10]    1'b0    h/s R/W  ���װʱAXI���߶�����timeout�ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    def_wr_err_en       [9]     1'b0    h/s R/W  ���װʱAXI����д���������ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    def_rd_err_en       [8]     1'b0    h/s R/W  ���װʱAXI���߶����������ж�ʹ��;0���жϽ�ֹ;1���ж�ʹ��
    Reserved            [7:1]   7'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    def_en              [0]     1'b0    h/s R/W  һ������������װʹ�ܣ������def_enд��1'b1�������װ������һ������������װ��ɺ���Ӳ���Զ���def_en���㣻
                                                 ���װ���̳���ʱ��Ӳ��Ҳ���def_en�Զ����㣬ʹ�ڲ�״̬������IDLE״̬�����üĴ������ؽ��װ����״̬��
                                                 дʱ����һ������������װʹ�ܣ�0����ʹ�ܽ��װ����1��ʹ�ܽ��װ����
                                                 ��ʱ����һ������������װ����״̬��0��û�ڽ��н��װ����1�����ڽ��н��װ����
    */
    VOS_UINT32          ulEnableInterrupt;
    VOS_UINT32          ulValue;
    const VOS_UINT32    ulInterruptValue    = 0x0107FF01;   /* ʹ���жϷ�ʽʱ����ʹ�ܼĴ�����ֵ */
    const VOS_UINT32    ulPollValue         = 0x01;         /* ʹ����ѯ��ʽʱ����ʹ�ܼĴ�����ֵ */
    PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU   stRegConfig;


    if( ulTotalLen > HDLC_DEF_INTERRUPT_LIMIT )
    {
        /* ���÷�װ���ʹ�ܼĴ���hdlc_def_en��[31:0]λΪ0x0107FF01 */
        ulValue             = ulInterruptValue;
        ulEnableInterrupt   = VOS_TRUE;

        g_PppHdlcHardStat.ulDefWaitIntCnt++;
    }
    else
    {
        /* ���÷�װ���ʹ�ܼĴ���hdlc_frm_en��[31:0]λΪ0x01 */
        ulValue             = ulPollValue;
        ulEnableInterrupt   = VOS_FALSE;

        g_PppHdlcHardStat.ulDefWaitQueryCnt++;
    }

    /* ʹ��ǰ����ϴη�װ�����װ��ԭʼ�ж� */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_INT_CLR_ADDR(HDLC_IP_BASE_ADDR), 0xFFFFFFFFU);
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_INT_CLR_ADDR(HDLC_IP_BASE_ADDR), 0xFFFFFFFFU);

    /* �ϱ��Ĵ�����ά�ɲ� */
    PPP_Driver_HdlcHardGetDefRegInfo(&stRegConfig);
    PPP_Driver_HdlcHardTraceDefRegconfig(VOS_FALSE, ulValue, ulEnableInterrupt, &stRegConfig);

    /* ʹ��Ӳ��֮ǰ��ǿ��ARM˳��ִ�н���ǰ���ָ�� */
    TTF_FORCE_ARM_INSTUCTION();

    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_EN_ADDR(HDLC_IP_BASE_ADDR), ulValue);

    return ulEnableInterrupt;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardDefEnable
 ��������  : ���ý��װ�Ĵ�����ʹ�ܽ��װ����
 �������  : pstDrvDefPara      -   ��װ����
 �������  : pulEnableInterrupt -   �ж��Ƿ�ʹ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardDefEnable
(
    PPP_DRIVER_HDLC_HARD_DEF_PARA_STRU *pstDrvDefPara,
    VOS_UINT32                         *pulEnableInterrupt
)
{
    if ((VOS_NULL_PTR == pstDrvDefPara) || (VOS_NULL_PTR == pulEnableInterrupt))
    {
        PPP_HDLC_WARNING_LOG2("pstDrvDefPara or pulEnableInterrupt is NULL", pstDrvDefPara, pulEnableInterrupt);
        return VOS_ERR;
    }

    /* ���÷�װ�����װͨ�üĴ��� */
    PPP_Driver_HdlcHardCommCfgReg();

    /* �����ڴ���ؼĴ��� */
    PPP_Driver_HdlcHardDefCfgBufReg();

    /* ����ѹ��ָʾ��������֡�����Ϣ�Ĵ��� */
    PPP_Driver_HdlcHardDefCfgReg(pstDrvDefPara);

    /* ����ʹ�ܼĴ��� */
    *pulEnableInterrupt = PPP_Driver_HdlcHardDefCfgEnReg(pstDrvDefPara->ulInputTotalSize);

    return VOS_OK;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardDefCfgGoOnReg
 ��������  : ����HDLC��ͣ״̬������GO_ON�Ĵ�����Ӧ����λ
 �������  : ulDefStatus  -   ���װ״̬
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardDefCfgGoOnReg
(
    VOS_UINT32          ulDefStatus
)
{
    /*
    hdlc_def_go_on  (0x84)
     31                  17  16  15    9   8  7   1  0
    |----------------------|----|-------|----|-----|----|
    |         Rsv          |Goon|  Rsv  |Goon| Rsv |Goon|
    Reserved             [31:17] 15'b0   h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    def_rpt_ful_goon     [16]    1'b0    h/s WO   �ⲿ���װ��Ч֡��Ϣ�ϱ��ռ������ͣ���
    Reserved             [15:9]  7'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    def_outspc_ful_goon  [8]     1'b0    h/s WO   �ⲿ���װ������ݴ洢�ռ������ͣ״̬���
    Reserved             [7:1]   7'b0    h/s R/W  ����λ����ʱ����0��дʱ��Ӱ�졣
    def_lcp_goon         [0]     1'b0    h/s WO   ���һ���Ϸ�LCP֡���µ�Ӳ����ͣ״̬����������װģ��δ������һ������װ�����ݣ�<=2KB(def_in_pkt_len_max)�������һ���Ϸ�LCP֡�������ͣ��֡���ȴ��������˼Ĵ���д"1"���ټ�������ʣ������ݡ�
    */

    /* GO_ONǰ����ϴν��װ��ԭʼ�ж� */
    PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_INT_CLR_ADDR(HDLC_IP_BASE_ADDR), 0xFFFFFFFFU);

    if (HDLC_DEF_STATUS_PAUSE_RPT_SPACE_FULL == ulDefStatus )
    {
        PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_GO_ON_ADDR(HDLC_IP_BASE_ADDR),
                        (VOS_UINT32)0x10000);
    }
    else if (HDLC_DEF_STATUS_PAUSE_OUTPUT_SPACE_FULL == ulDefStatus )
    {
        PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_GO_ON_ADDR(HDLC_IP_BASE_ADDR),
                        (VOS_UINT32)0x100);
    }
    else if (HDLC_DEF_STATUS_PAUSE_LCP == ulDefStatus )
    {
        PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_GO_ON_ADDR(HDLC_IP_BASE_ADDR),
                        (VOS_UINT32)0x1);
    }
    else
    {
        PPP_HDLC_ERROR_LOG1("PPP_HDLC_HARD_DefCfgGoOnReg, ERROR, Wrong ulDefStatus %d!\r\n", ulDefStatus);
    }

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardGetDefVaildNum
 ��������  : ��ȡ���װ��Ч֡����
 �������  : ��
 �������  : ��
 �� �� ֵ  : VOS_UINT16 ���װ��Ч֡����
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT16 PPP_Driver_HdlcHardGetDefVaildNum(VOS_VOID)
{
    VOS_UINT16                          usValidFrameNum;

    usValidFrameNum = (VOS_UINT16)TTF_Read32RegByBit(SOC_ARM_HDLC_DEF_STATUS_ADDR(HDLC_IP_BASE_ADDR), 8, 23);

    return usValidFrameNum;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardGetDefUncompletInfo
 ��������  : ��ȡ���װ������֡��Ϣ
 �������  : ��
 �������  : pstUncompleteInfo  -   ���װ������֡��Ϣ
             pucValidNum        -   ���װ��Ч֡����
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardGetDefUncompletInfo
(
    PPP_DRIVER_HDLC_HARD_DEF_UNCOMPLETED_INFO_STRU *pstUncompleteInfo,
    VOS_UINT32                                     *pulValidNum
)
{
    VOS_UINT32                          ulStatus;                /* ���װ״̬ */
    VOS_UINT32                          ulValidNum;


    ulStatus  =   PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_STATUS_ADDR(HDLC_IP_BASE_ADDR));

    /* ��ѯ״̬�Ĵ���hdlc_def_status (0x88)�ĵ�2λ
       Ϊ1��ʾ���ν��װ�з�����֡�����
       Ϊ0��ʾ�޷�����֡��� */
    if (0 == (ulStatus & 0x4))
    {
        pstUncompleteInfo->ucExistFlag = HDLC_DEF_UNCOMPLETED_NOT_EXIST;

        return;
    }

    g_PppHdlcHardStat.ulDefUncompleteCnt++;

    /* def_valid_num        [23:8]  16'b0   h/s RO   ��֡�ϱ�ʱ����Ч֡��Ŀ�������������һ�����ܵķ�����֡�� */
    ulValidNum = (ulStatus & 0xFFFF00) >> 8;


    *pulValidNum = ulValidNum;
    /* �з�����֡ʱ��Ҫ��ȡ������def_uncomplet_st_now(0x8C)��def_info_frl_cnt_now(0xC4)
       ��ȡ�������ϱ��ռ���Ч֮֡��ķ�����֡���ȡ�Э������ݵ�ַ */
    pstUncompleteInfo->ucExistFlag = HDLC_DEF_UNCOMPLETED_EXIST;

    /* ���ֻ���ݴ���Щ��Ϣ�����½��װ��ʱ����ԭ�����HDLC */
    pstUncompleteInfo->ulDefStAgo         = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_UNCOMPLETED_ST_NOW_ADDR(HDLC_IP_BASE_ADDR));
    pstUncompleteInfo->ulDefInfoFrlCntAgo = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_INFO_FRL_CNT_NOW_ADDR(HDLC_IP_BASE_ADDR));

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardGetDefErrorInfo
 ��������  : ��ȡ����֡��Ϣ
 �������  : ��
 �������  : pstErrCnt      -   ����֡��Ϣ
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��10��
    ��    ��   : l00164359
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardGetDefErrorInfo
(
    PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU   *pstErrCnt
)
{
    VOS_UINT8                           ucErrType;


    /* ��ѯ״̬�Ĵ���hdlc_def_status (0x88)�ĵ�24:30��Ӧ����λΪ1��ʾ��ĳ�ִ���֡�����
       Ϊ0��ʾ��֡��� */
    ucErrType = (VOS_UINT8)TTF_Read32RegByBit(SOC_ARM_HDLC_DEF_STATUS_ADDR(HDLC_IP_BASE_ADDR) , 24, 30);

    if ((0 == ucErrType) || (VOS_NULL_PTR == pstErrCnt))
    {
        return;
    }

    PPP_HDLC_HARD_MEM_SET(pstErrCnt, sizeof(PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU),
                          0x00, sizeof(PPP_DRIVER_HDLC_HARD_DEF_ERR_FRAMES_CNT_STRU));

    pstErrCnt->ucErrType = ucErrType;

    /* get fcs error count */
    if (1 == TTF_GET_A_BIT(ucErrType, PPP_DRIVER_HDLC_HARD_DEF_FCS_ERR))
    {
        pstErrCnt->usFCSErrCnt        = (VOS_UINT16)TTF_Read32RegByBit(SOC_ARM_HDLC_DEF_ERR_INFO_0_ADDR(HDLC_IP_BASE_ADDR), 0, 15);
        PPP_HDLC_WARNING_LOG("bad hdlc fcs");
    }

    /* get frame too long error count */
    if (1 == TTF_GET_A_BIT(ucErrType, PPP_DRIVER_HDLC_HARD_DEF_FRAME_TOO_LONG))
    {
        pstErrCnt->usLenLongCnt       = (VOS_UINT16)TTF_Read32RegByBit(SOC_ARM_HDLC_DEF_ERR_INFO_0_ADDR(HDLC_IP_BASE_ADDR), 16, 31);
        PPP_HDLC_WARNING_LOG("bad hdlc frame length too long");
    }

    /* get frame too short error count */
    if (1 == TTF_GET_A_BIT(ucErrType, PPP_DRIVER_HDLC_HARD_DEF_FRAME_TOO_SHORT))
    {
        pstErrCnt->usLenShortCnt      = (VOS_UINT16)TTF_Read32RegByBit(SOC_ARM_HDLC_DEF_ERR_INFO_1_ADDR(HDLC_IP_BASE_ADDR), 0, 15);
        PPP_HDLC_WARNING_LOG("bad hdlc frame length too short");
    }

    /* get error protocol count */
    if (1 == TTF_GET_A_BIT(ucErrType, PPP_DRIVER_HDLC_HARD_DEF_PROTOCOL_ERR))
    {
        pstErrCnt->usErrProtocolCnt   = (VOS_UINT16)TTF_Read32RegByBit(SOC_ARM_HDLC_DEF_ERR_INFO_1_ADDR(HDLC_IP_BASE_ADDR), 16, 31);
        PPP_HDLC_WARNING_LOG("bad hdlc frame protocol");
    }

    /* get error control count */
    if (1 == TTF_GET_A_BIT(ucErrType, PPP_DRIVER_HDLC_HARD_DEF_CTRL_ERR))
    {
        pstErrCnt->usErrCtrlCnt       = (VOS_UINT16)TTF_Read32RegByBit(SOC_ARM_HDLC_DEF_ERR_INFO_2_ADDR(HDLC_IP_BASE_ADDR), 0, 15);
        PPP_HDLC_WARNING_LOG("bad hdlc frame control");
    }

    /* get error address count */
    if (1 == TTF_GET_A_BIT(ucErrType, PPP_DRIVER_HDLC_HARD_DEF_ADDR_ERR))
    {
        pstErrCnt->usErrAddrCnt       = (VOS_UINT16)TTF_Read32RegByBit(SOC_ARM_HDLC_DEF_ERR_INFO_2_ADDR(HDLC_IP_BASE_ADDR), 16, 31);
        PPP_HDLC_WARNING_LOG("bad hdlc frame address");
    }

    /* get error flag position count */
    if (1 == TTF_GET_A_BIT(ucErrType, PPP_DRIVER_HDLC_HARD_DEF_FLAG_POS_ERR))
    {
        pstErrCnt->usFlagPosErrCnt    = (VOS_UINT16)TTF_Read32RegByBit(SOC_ARM_HDLC_DEF_ERR_INFO_3_ADDR(HDLC_IP_BASE_ADDR), 0, 15);
        PPP_HDLC_WARNING_LOG("bad hdlc frame flag position");
    }

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardGetDefRawInt
 ��������  : ��ȡ���װ�жϼĴ���ֵ
 �������  : ��
 �������  : ��
 �� �� ֵ  : VOS_UINT32  ���װ�жϼĴ���ֵ
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2012��4��20��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcHardGetDefRawInt(VOS_VOID)
{
    VOS_UINT32                          ulRawInt;

    ulRawInt  =   PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_RAW_INT_ADDR(HDLC_IP_BASE_ADDR));

    return ulRawInt;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardShowConfigInfo
 ��������  : ��ӡHDLC������Ϣ
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
VOS_VOID PPP_Driver_HdlcHardShowConfigInfo(VOS_VOID)
{
    PPP_DRIVER_HDLC_HARD_CONFIG_INFO_STRU  *pstHdlcConf = PPP_DRIVER_HDLC_HARD_GET_CONFIG;

    /* ���IP�����Ϣ */
    vos_printf("HDLC IP����ַ                   = %d\n", HDLC_IP_BASE_ADDR);
    vos_printf("���װ�ж��ź�                  = %d\n", pstHdlcConf->slHdlcISRDef);
    vos_printf("��װ�ж��ź�                    = %d\n", pstHdlcConf->slHdlcISRFrm);
    vos_printf("���װ�ź���                    = %d\n", pstHdlcConf->ulHdlcDefMasterSem);
    vos_printf("��װ�ź���                      = %d\n", pstHdlcConf->ulHdlcFrmMasterSem);
    vos_printf("���װ�ж�ˮ��                  = %d\n", pstHdlcConf->ulHdlcDefIntLimit);
    vos_printf("��װ�ж�ˮ��                    = %d\n", pstHdlcConf->ulHdlcFrmIntLimit);
    vos_printf("ϵͳ����������ַ                = %d\n", pstHdlcConf->ulHdlcScCtrlBaseAddr);

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardGetFrmRegInfo
 ��������  : ��ȡ��װ�Ĵ�����Ϣ
 �������  : ��
 �������  : pstRegConfig   -   ��װ�Ĵ�����Ϣ
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardGetFrmRegInfo(PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU *pstRegConfig)
{
    if (VOS_NULL_PTR == pstRegConfig)
    {
        PPP_HDLC_WARNING_LOG("pstRegConfig");
        return;
    }

    /* ����ȫ���Ĵ������� */
    pstRegConfig->ulStateSwRst          = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_STATE_SW_RST_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulPriorTimeoutCtrl    = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_PRIROR_TIMEOUT_CTRL_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulRdErrCurrAddr       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_RD_ERR_CURR_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulWrErrCurrAddr       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_WR_ERR_CURR_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcFrmEn           = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_EN_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcFrmRawInt       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_RAW_INT_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcFrmIntStatus    = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_INT_STATUS_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcFrmIntClr       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_INT_CLR_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcFrmCfg          = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_CFG_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcFrmAccm         = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_ACCM_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcFrmStatus       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_STATUS_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulFrmInLliAddr        = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_IN_LLI_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulFrmInSublliAddr     = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_IN_SUBLLI_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulFrmInPktLen         = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_IN_PKT_LEN_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulFrmInBlkAddr        = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_IN_BLK_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulFrmInBlkLen         = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_IN_BLK_LEN_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulFrmOutLliAddr       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_OUT_LLI_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulFrmOutSpaceAddr     = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_OUT_SPACE_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulFrmOutSpaceDep      = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_OUT_SPACE_DEP_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulFrmRptAddr          = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_RPT_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulFrmRptDep           = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_RPT_DEP_ADDR(HDLC_IP_BASE_ADDR));

    return;
}

/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardGetDefRegInfo
 ��������  : ��ȡ���װ�Ĵ�����Ϣ
 �������  : ��
 �������  : pstRegConfig   -   ���װ�Ĵ�����Ϣ
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��1��
    ��    ��   : t00359887
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardGetDefRegInfo(PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU *pstRegConfig)
{
    if (VOS_NULL_PTR == pstRegConfig)
    {
        PPP_HDLC_WARNING_LOG("pstRegConfig");
        return;
    }

    /* ����ȫ���Ĵ������� */
    pstRegConfig->ulStateSwRst             = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_STATE_SW_RST_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulPriorTimeoutCtrl       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_PRIROR_TIMEOUT_CTRL_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulRdErrCurrAddr          = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_RD_ERR_CURR_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulWrErrCurrAddr          = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_WR_ERR_CURR_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefEn              = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_EN_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefRawInt          = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_RAW_INT_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefIntStatus       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_INT_STATUS_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefIntClr          = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_INT_CLR_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefCfg             = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_CFG_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefUncompletedLen      = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_UNCOMPLETED_LEN_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefUncompletedPro      = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_UNCOMPLETED_PRO_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefUncompletedAddr     = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_UNCOMPLETED_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefUncompleteStAgo     = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_UNCOMPLETED_ST_AGO_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefGoOn            = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_GO_ON_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefStatus          = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_STATUS_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefUncompletStNow      = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_UNCOMPLETED_ST_NOW_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefInLliAddr           = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_IN_LLI_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefInPktAddr           = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_IN_PKT_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefInPktLen            = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_IN_PKT_LEN_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefInPktLenMax         = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_IN_PKT_LEN_MAX_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefOutSpcAddr          = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_OUT_SPC_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefOutSpaceDep         = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_OUT_SPACE_DEP_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefRptAddr             = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_RPT_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefRptDep              = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_RPT_DEP_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefErrInfor0       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_ERR_INFO_0_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefErrInfor1       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_ERR_INFO_1_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefErrInfor2       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_ERR_INFO_2_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulHdlcDefErrInfor3       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_ERR_INFO_3_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefInfoFr1CntAgo       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_INFO_FRL_CNT_AGO_ADDR(HDLC_IP_BASE_ADDR));
    pstRegConfig->ulDefInfoFr1CntNow       = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_INFO_FRL_CNT_NOW_ADDR(HDLC_IP_BASE_ADDR));

    return;
}


/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardShowFrmReg
 ��������  : ��ӡ��װ�Ĵ�����ǰ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��1��30��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardShowFrmReg(VOS_VOID)
{
    PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU   stRegConfig;
    PPP_DRIVER_HDLCHARD_FRM_REG_INFO_STRU  *pstRegConfig = &stRegConfig;

    PPP_Driver_HdlcHardGetFrmRegInfo(pstRegConfig);

    vos_printf("\n================HDLC Hardware ShowFrmReg Begin==========================\n");

    vos_printf("ulStateSwRst                    = 0x%x\n", pstRegConfig->ulStateSwRst);
    vos_printf("ulPriorTimeoutCtrl              = 0x%x\n", pstRegConfig->ulPriorTimeoutCtrl);
    vos_printf("ulRdErrCurrAddr                 = 0x%x\n", pstRegConfig->ulRdErrCurrAddr);
    vos_printf("ulWrErrCurrAddr                 = 0x%x\n", pstRegConfig->ulWrErrCurrAddr);
    vos_printf("ulHdlcFrmEn                     = 0x%x\n", pstRegConfig->ulHdlcFrmEn);
    vos_printf("ulHdlcFrmRawInt                 = 0x%x\n", pstRegConfig->ulHdlcFrmRawInt);
    vos_printf("ulHdlcFrmIntStatus              = 0x%x\n", pstRegConfig->ulHdlcFrmIntStatus);
    vos_printf("ulHdlcFrmIntClr                 = 0x%x\n", pstRegConfig->ulHdlcFrmIntClr);
    vos_printf("ulHdlcFrmCfg                    = 0x%x\n", pstRegConfig->ulHdlcFrmCfg);
    vos_printf("ulHdlcFrmAccm                   = 0x%x\n", pstRegConfig->ulHdlcFrmAccm);
    vos_printf("ulHdlcFrmStatus                 = 0x%x\n", pstRegConfig->ulHdlcFrmStatus);
    vos_printf("ulFrmInLliAddr                  = 0x%x\n", pstRegConfig->ulFrmInLliAddr);
    vos_printf("ulFrmInSublliAddr               = 0x%x\n", pstRegConfig->ulFrmInSublliAddr);
    vos_printf("ulFrmInPktLen                   = 0x%x\n", pstRegConfig->ulFrmInPktLen);
    vos_printf("ulFrmInBlkAddr                  = 0x%x\n", pstRegConfig->ulFrmInBlkAddr);
    vos_printf("ulFrmInBlkLen                   = 0x%x\n", pstRegConfig->ulFrmInBlkLen);
    vos_printf("ulFrmOutLliAddr                 = 0x%x\n", pstRegConfig->ulFrmOutLliAddr);
    vos_printf("ulFrmOutSpaceAddr               = 0x%x\n", pstRegConfig->ulFrmOutSpaceAddr);
    vos_printf("ulFrmOutSpaceDep                = 0x%x\n", pstRegConfig->ulFrmOutSpaceDep);
    vos_printf("ulFrmRptAddr                    = 0x%x\n", pstRegConfig->ulFrmRptAddr);
    vos_printf("ulFrmRptDep                     = 0x%x\n", pstRegConfig->ulFrmRptDep);

    vos_printf("\n================HDLC Hardware ShowFrmReg End==========================\n");

    return;
}


/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcHardShowDefReg
 ��������  : ��ӡ���װ�Ĵ�����ǰ����
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2013��1��30��
    ��    ��   : c00191211
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcHardShowDefReg(VOS_VOID)
{
    PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU   stRegConfig;
    PPP_DRIVER_HDLCHARD_DEF_REG_INFO_STRU  *pstRegConfig = &stRegConfig;


    PPP_Driver_HdlcHardGetDefRegInfo(pstRegConfig);

    vos_printf("\n================HDLC Hardware ShowDefReg Begin==========================\n");

    vos_printf("ulStateSwRst             = 0x%x\n", pstRegConfig->ulStateSwRst);
    vos_printf("ulPriorTimeoutCtrl       = 0x%x\n", pstRegConfig->ulPriorTimeoutCtrl);
    vos_printf("ulRdErrCurrAddr          = 0x%x\n", pstRegConfig->ulRdErrCurrAddr);
    vos_printf("ulWrErrCurrAddr          = 0x%x\n", pstRegConfig->ulWrErrCurrAddr);
    vos_printf("ulHdlcDefEn              = 0x%x\n", pstRegConfig->ulHdlcDefEn);
    vos_printf("ulHdlcDefRawInt          = 0x%x\n", pstRegConfig->ulHdlcDefRawInt);
    vos_printf("ulHdlcDefIntStatus       = 0x%x\n", pstRegConfig->ulHdlcDefIntStatus);
    vos_printf("ulHdlcDefIntClr          = 0x%x\n", pstRegConfig->ulHdlcDefIntClr);
    vos_printf("ulHdlcDefCfg             = 0x%x\n", pstRegConfig->ulHdlcDefCfg);
    vos_printf("ulDefUncompletedLen      = 0x%x\n", pstRegConfig->ulDefUncompletedLen);
    vos_printf("ulDefUncompletedPro      = 0x%x\n", pstRegConfig->ulDefUncompletedPro);
    vos_printf("ulDefUncompletedAddr     = 0x%x\n", pstRegConfig->ulDefUncompletedAddr);
    vos_printf("ulDefUncompleteStAgo     = 0x%x\n", pstRegConfig->ulDefUncompleteStAgo);
    vos_printf("ulHdlcDefGoOn            = 0x%x\n", pstRegConfig->ulHdlcDefGoOn);
    vos_printf("ulHdlcDefStatus          = 0x%x\n", pstRegConfig->ulHdlcDefStatus);
    vos_printf("ulDefUncompletStNow      = 0x%x\n", pstRegConfig->ulDefUncompletStNow);
    vos_printf("ulDefInLliAddr           = 0x%x\n", pstRegConfig->ulDefInLliAddr);
    vos_printf("ulDefInPktAddr           = 0x%x\n", pstRegConfig->ulDefInPktAddr);
    vos_printf("ulDefInPktLen            = 0x%x\n", pstRegConfig->ulDefInPktLen);
    vos_printf("ulDefInPktLenMax         = 0x%x\n", pstRegConfig->ulDefInPktLenMax);
    vos_printf("ulDefOutSpcAddr          = 0x%x\n", pstRegConfig->ulDefOutSpcAddr);
    vos_printf("ulDefOutSpaceDep         = 0x%x\n", pstRegConfig->ulDefOutSpaceDep);
    vos_printf("ulDefRptAddr             = 0x%x\n", pstRegConfig->ulDefRptAddr);
    vos_printf("ulDefRptDep              = 0x%x\n", pstRegConfig->ulDefRptDep);
    vos_printf("ulHdlcDefErrInfor0       = 0x%x\n", pstRegConfig->ulHdlcDefErrInfor0);
    vos_printf("ulHdlcDefErrInfor1       = 0x%x\n", pstRegConfig->ulHdlcDefErrInfor1);
    vos_printf("ulHdlcDefErrInfor2       = 0x%x\n", pstRegConfig->ulHdlcDefErrInfor2);
    vos_printf("ulHdlcDefErrInfor3       = 0x%x\n", pstRegConfig->ulHdlcDefErrInfor3);
    vos_printf("ulDefInfoFr1CntAgo       = 0x%x\n", pstRegConfig->ulDefInfoFr1CntAgo);
    vos_printf("ulDefInfoFr1CntNow       = 0x%x\n", pstRegConfig->ulDefInfoFr1CntNow);

    vos_printf("\n================HDLC Hardware ShowDefReg End==========================\n");

}

#if ((SC_CTRL_MOD_KIRIN970_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_M533 == SC_CTRL_MOD))
/*****************************************************************************
 �� �� ��  : PPP_Driver_HdlcGetRegValue
 ��������  : ��ȡHDLC�Ĵ�����Ϣ
 �������  : enHdlcRegType   �Ĵ�������
 �������  : ��
 �� �� ֵ  : �Ĵ���ֵ
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��1��4��
    ��    ��   : c00184031
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_UINT32 PPP_Driver_HdlcGetRegValue
(
    PPP_DRIVER_HDLC_REG_ENUM_UINT8      enHdlcRegType,
    VOS_UINT32                         *pulValue
)
{
    /* �����߱�֤pulValue�ǿ� */

    switch (enHdlcRegType)
    {
        case PPP_DRIVER_MAX_FRM_INFO_REG:

            *pulValue = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_FRM_INFO_MAX_LEN_ADDR(HDLC_IP_BASE_ADDR));

            break;

        case PPP_DRIVER_MAX_DEF_INFO_REG:

            *pulValue = PPP_HDLC_READ_32REG(SOC_ARM_HDLC_DEF_INFO_MAX_LEN_ADDR(HDLC_IP_BASE_ADDR));

            break;

        default:

            /* �Ƿ��������ֵ��Ϊȫf */
            *pulValue = 0xffffffff;
            PPP_HDLC_WARNING_LOG1("Invalid Hdlc Reg Type!", enHdlcRegType);

            return PS_FAIL;
    }

    return PS_SUCC;
}

/*****************************************************************************
 �� �� ��  : PPP_HDLC_ReadVer
 ��������  : ��ȡHDLC IP�汾��,kirin970֮ǰ�汾��֧��kirin
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��11��10��
    ��    ��   : c00184031
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_HDLC_ReadVer(VOS_VOID)
{
    VOS_UINT32                          ulVer = 0;

    /* ��ʱ�� */
    PPP_Driver_HdlcHardPeriphClkOpen();

    if (VOS_TRUE == PPP_Driver_HdlcHardGetPeriphClkStatus())
    {
        /* ulVer = hdlc_version�Ĵ���[31:0]λ��ֵ */
        ulVer  = TTF_Read32RegByBit(SOC_ARM_HDLC_VERSION_ADDR(HDLC_IP_BASE_ADDR), 0, 31);
        vos_printf("HDLC Version %d\r\n!", ulVer);
    }

    /* ��ʱ�� */
    PPP_Driver_HdlcHardPeriphClkClose();

    return;
}
#endif

/*****************************************************************************
 �� �� ��  : PPP_Driver_SetMaxFrmDefInfoLen
 ��������  : ��������װ���װ�Ĵ�����Ϣ��ֻ����һ��
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2017��1��4��
    ��    ��   : c00184031
    �޸�����   : �����ɺ���

*****************************************************************************/
VOS_VOID PPP_Driver_HdlcSetMaxFrmDefInfoLen(VOS_VOID)
{
#if ((SC_CTRL_MOD_KIRIN970_SFT == SC_CTRL_MOD) || (SC_CTRL_MOD_M533 == SC_CTRL_MOD))
    /* ��װ���װ��Ϣ��ֻ����һ�� */
    static VOS_UINT8                    ucCfgHdlcMaxLenFlag = 0;
    VOS_UINT32                          ulHdlcRegValue = 0;
    VOS_UINT32                          ulResult;


    /* ��ֹ��ʼ��ʱ���ڿ�ʱ��������������󳤶���Ϣ��ʧ��,ֻ���һ�� */
    if (0 == ucCfgHdlcMaxLenFlag)
    {
        ucCfgHdlcMaxLenFlag = 1;
        ulResult = PPP_Driver_HdlcGetRegValue(PPP_DRIVER_MAX_FRM_INFO_REG, &ulHdlcRegValue);
        if (PS_SUCC != ulResult)
        {
            PPP_HDLC_WARNING_LOG2("Get Frm Reg Fail!", ulResult, ulHdlcRegValue);
        }

        /* ���Ĵ���ʧ��Ҳ��ǿ������һ�� */
        if (HDLC_MAX_FRM_DEF_INFO_LEN != ulHdlcRegValue)
        {
            PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_FRM_INFO_MAX_LEN_ADDR(
                    HDLC_IP_BASE_ADDR), HDLC_MAX_FRM_DEF_INFO_LEN);
        }

        ulResult = PPP_Driver_HdlcGetRegValue(PPP_DRIVER_MAX_DEF_INFO_REG, &ulHdlcRegValue);
        if (PS_SUCC != ulResult)
        {
            PPP_HDLC_WARNING_LOG2("Get Def Reg Fail", ulResult, ulHdlcRegValue);
        }

        /* ���Ĵ���ʧ��Ҳ��ǿ������һ�� */
        if (HDLC_MAX_FRM_DEF_INFO_LEN != ulHdlcRegValue)
        {
            PPP_HDLC_WRITE_32REG(SOC_ARM_HDLC_DEF_INFO_MAX_LEN_ADDR(
                    HDLC_IP_BASE_ADDR), HDLC_MAX_FRM_DEF_INFO_LEN);
        }
    }
#endif
}


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


