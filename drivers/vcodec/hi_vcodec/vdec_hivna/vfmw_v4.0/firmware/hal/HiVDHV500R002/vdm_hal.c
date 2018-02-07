/*
 * vdm hal interface
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
/*lint -e665*/
#include "basedef.h"
#include "vfmw.h"
#include "mem_manage.h"
#include "public.h"
#include "scd_drv.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"
#ifdef VFMW_MPEG2_SUPPORT
#include "vdm_hal_mpeg2.h"
#endif
#ifdef VFMW_H264_SUPPORT
#include "vdm_hal_h264.h"
#endif
#ifdef VFMW_HEVC_SUPPORT
#include "vdm_hal_hevc.h"
#endif
#ifdef VFMW_MPEG4_SUPPORT
#include "vdm_hal_mpeg4.h"
#endif
#ifdef VFMW_REAL8_SUPPORT
#include "vdm_hal_real8.h"
#endif
#ifdef VFMW_REAL9_SUPPORT
#include "vdm_hal_real9.h"
#endif
#ifdef VFMW_VC1_SUPPORT
#include "vdm_hal_vc1.h"
#endif
#ifdef VFMW_VP8_SUPPORT
#include "vdm_hal_vp8.h"
#endif
#ifdef VFMW_VP9_SUPPORT
#include "vdm_hal_vp9.h"
#endif
#include "vfmw_intf.h"
#ifdef HIVDEC_SMMU_SUPPORT
#include "smmu.h"
#endif

UINT8 g_not_direct_8x8_inference_flag;

VDMHAL_HWMEM_S  g_HwMem[MAX_VDH_NUM];
VDMHAL_BACKUP_S g_VdmRegState;

static VDMDRV_SLEEP_STAGE_E s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_NONE;
static VDMDRV_STATEMACHINE_E s_VdmState = VDM_IDLE_STATE;

inline UINT32 VDMHAL_GetIntMaskCfg(SINT32 ModuleLowlyEnable)
{/*lint !e695*/
	UINT32 D32 = 0;

#ifdef VDM_BUSY_WAITTING
	//mask int
	D32 = 0xFFFFFFFF;
#else
	//enable int
	if (ModuleLowlyEnable == 1)
		D32 = 0xFFFFFFFA;
	else
		D32 = 0xFFFFFFFE;
#endif

	return D32;
}

VOID VDMHAL_EnableInt(SINT32 VdhId)
{
	UINT32 D32  = 0;
	SINT32 *p32 = NULL;
	UADDR vdm_reg_phy_addr   = 0;
	SINT32 ModuleLowlyEnable = 0;
	ModuleLowlyEnable        = 0;

	switch (VdhId) {
	case 0:
		vdm_reg_phy_addr = gVdhRegBaseAddr;
		break;

	default:
		dprint(PRN_FATAL, "VdhId is wrong! VDMHAL_EnableInt\n");
		return;
	}

	if (VdhId > (MAX_VDH_NUM - 1)) {
		dprint(PRN_FATAL, "%s: VdhId : %d is more than %d\n", __func__, VdhId, (MAX_VDH_NUM - 1));
		return;
	}

	if (g_HwMem[VdhId].pVdmRegVirAddr == NULL) {
		if ((p32 = (SINT32 *) MEM_Phy2Vir(vdm_reg_phy_addr)) != NULL) {
			g_HwMem[VdhId].pVdmRegVirAddr = p32;
		} else {
			dprint(PRN_FATAL, "vdm register virtual address not mapped, reset failed\n");
			return;
		}
	}

	D32 = VDMHAL_GetIntMaskCfg(ModuleLowlyEnable);

	WR_VREG(VREG_INT_MASK, D32, VdhId);

	return;
}

UINT32 VDMHAL_ReduceCoreFrequency(VOID)
{
	UINT32 div_value = 0;
	UINT32 sc_div_vdec = 0;

	RD_CRG_VREG(PERI_CRG_CORE_DIV, sc_div_vdec);
	div_value = sc_div_vdec & 0x1F;

	sc_div_vdec = ((sc_div_vdec & 0x1F) << 1) + 1;
	if (sc_div_vdec > 0x1F)
		sc_div_vdec = 0x1F;

	WR_CRG_VREG(PERI_CRG_CORE_DIV, 0x1F0000 | sc_div_vdec);
	VFMW_OSAL_uDelay(1);

	return div_value;
}

VOID VDMHAL_RestoreCoreFrequency(UINT32 DivValue)
{
	WR_CRG_VREG(PERI_CRG_CORE_DIV, 0x1F0000 | DivValue);
	VFMW_OSAL_uDelay(1);

	return;
}

UINT32 VDMHAL_ReduceAXIFrequency(VOID)
{
	UINT32 div_value = 0;
	UINT32 sc_div_vcodecbus = 0;

	RD_CRG_VREG(PERI_CRG_AXI_DIV, sc_div_vcodecbus);
	div_value = sc_div_vcodecbus & 0x1F;

	sc_div_vcodecbus = ((sc_div_vcodecbus & 0x1F) << 1) + 1;
	if (sc_div_vcodecbus > 0x1F)
		sc_div_vcodecbus = 0x1F;

	WR_CRG_VREG(PERI_CRG_AXI_DIV, 0x1F0000 | sc_div_vcodecbus);
	VFMW_OSAL_uDelay(1);

	return div_value;
}

VOID VDMHAL_RestoreAXIFrequency(UINT32 DivValue)
{
	WR_CRG_VREG(PERI_CRG_AXI_DIV, 0x1F0000 | DivValue);
	VFMW_OSAL_uDelay(1);

	return;
}

SINT32 VDMHAL_CfgRpReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	SINT32 D32 = 0;
	WR_VREG(VREG_AVM_ADDR, pVdhRegCfg->VdhAvmAddr, 0);

	D32 = 0x2000C203;
	WR_VREG(VREG_BASIC_CFG1, D32, 0);

	D32 = 0x00300C03;
	WR_VREG(VREG_SED_TO, D32, 0);
	WR_VREG(VREG_ITRANS_TO, D32, 0);
	WR_VREG(VREG_PMV_TO, D32, 0);
	WR_VREG(VREG_PRC_TO, D32, 0);
	WR_VREG(VREG_RCN_TO, D32, 0);
	WR_VREG(VREG_DBLK_TO, D32, 0);
	WR_VREG(VREG_PPFD_TO, D32, 0);

	return VDMHAL_OK;
}

VOID VDMHAL_IMP_Init(VOID)
{
	memset(&g_HwMem[0], 0, sizeof(g_HwMem[0]));
	memset(&g_VdmRegState, 0, sizeof(g_VdmRegState));

	g_HwMem[0].pVdmRegVirAddr  = (SINT32 *) MEM_Phy2Vir(gVdhRegBaseAddr);
	g_HwMem[0].pPERICRGVirAddr = (SINT32 *) MEM_Phy2Vir(gPERICRG_RegBaseAddr);
	g_HwMem[0].pBpdRegVirAddr  = (SINT32 *) MEM_Phy2Vir(gBpdRegBaseAddr);

	VDMHAL_IMP_GlbReset();
	//VDMHAL_IMP_WriteScdEMARID();
	s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_NONE;
	s_VdmState = VDM_IDLE_STATE;
}

VOID VDMHAL_IMP_DeInit(VOID)
{
	s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_NONE;
	s_VdmState = VDM_IDLE_STATE;
}

VOID VDMHAL_IMP_ResetVdm(SINT32 VdhId)
{
	SINT32 i;
	SINT32 tmp = 0;
	UINT32 reg;
	UINT32 reg_rst_ok;
	UINT32 *pVdmResetVirAddr;
	UINT32 *pVdmResetOkVirAddr;

	pVdmResetVirAddr   = (SINT32 *) MEM_Phy2Vir(gSOFTRST_REQ_Addr);
	pVdmResetOkVirAddr = (SINT32 *) MEM_Phy2Vir(gSOFTRST_OK_ADDR);

	if (pVdmResetVirAddr == NULL || pVdmResetOkVirAddr == NULL) {
		dprint(PRN_FATAL, "VDMHAL_ResetVdm: map vdm register fail, vir(pVdmResetVirAddr) : (%pK), vir(pVdmResetOkVirAddr) : (%pK)\n", pVdmResetVirAddr, pVdmResetOkVirAddr);
		return;
	}

	RD_VREG(VREG_INT_MASK, tmp, VdhId);

	/* require mfde reset */
	reg = *(volatile UINT32 *)pVdmResetVirAddr;
	*(volatile UINT32 *)pVdmResetVirAddr = reg | (UINT32) (1 << MFDE_RESET_CTRL_BIT);

	/* wait for reset ok */
	for (i = 0; i < 100; i++) {
		reg_rst_ok = *(volatile UINT32 *)pVdmResetOkVirAddr;
		if (reg_rst_ok & (1 << MFDE_RESET_OK_BIT))
			break;
		VFMW_OSAL_uDelay(10);
	}

	if (i >= 100)
		dprint(PRN_FATAL, "%s reset failed\n", __func__);

	/* clear reset require */
	*(volatile UINT32 *)pVdmResetVirAddr = reg & (UINT32) (~(1 << MFDE_RESET_CTRL_BIT));

	WR_VREG(VREG_INT_MASK, tmp, VdhId);
	s_VdmState = VDM_IDLE_STATE;

	return;
}

VOID VDMHAL_IMP_GlbReset(VOID)
{
	SINT32 i;
	UINT32 reg, reg_rst_ok;
	UINT32 *pResetVirAddr   = NULL;
	UINT32 *pResetOKVirAddr = NULL;

	pResetVirAddr   = (SINT32 *) MEM_Phy2Vir(gSOFTRST_REQ_Addr);
	pResetOKVirAddr = (SINT32 *) MEM_Phy2Vir(gSOFTRST_OK_ADDR);

	if (pResetVirAddr == NULL || pResetOKVirAddr == NULL) {
		dprint(PRN_FATAL, "VDMHAL_GlbReset: map vdm register fail, vir(pResetVirAddr) : (%pK), vir(pResetOKVirAddr) : (%pK)\n", pResetVirAddr, pResetOKVirAddr);
		return;
	}

	/* require all reset, include mfde scd bpd */
	reg = *(volatile UINT32 *)pResetVirAddr;
	*(volatile UINT32 *)pResetVirAddr = reg | (UINT32) (1 << ALL_RESET_CTRL_BIT);

	/* wait for reset ok */
	for (i = 0; i < 100; i++) {
		reg_rst_ok = *(volatile UINT32 *)pResetOKVirAddr;
		if (reg_rst_ok & (1 << ALL_RESET_OK_BIT))
			break;
		VFMW_OSAL_uDelay(10);
	}

	if (i >= 100)
		dprint(PRN_FATAL, "Glb Reset Failed\n");

	/* clear reset require */
	*(volatile UINT32 *)pResetVirAddr = reg & (UINT32) (~(1 << ALL_RESET_CTRL_BIT));

	return;
}

VOID VDMHAL_IMP_ClearIntState(SINT32 VdhId)
{
	SINT32 *p32;
	SINT32 D32 = 0xFFFFFFFF;/*lint !e569*/

	if (VdhId > (MAX_VDH_NUM - 1)) {
		dprint(PRN_FATAL, "%s: VdhId : %d is more than %d\n", __func__, VdhId, (MAX_VDH_NUM - 1));
		return;
	}

	if (g_HwMem[VdhId].pVdmRegVirAddr == NULL) {
		if ((p32 = (SINT32 *) MEM_Phy2Vir(gVdhRegBaseAddr)) != NULL) {
			g_HwMem[VdhId].pVdmRegVirAddr = p32;
		} else {
			dprint(PRN_FATAL, " %s %d vdm register virtual address not mapped, reset failed\n", __func__, __LINE__);
			return;
		}
	}

	WR_VREG(VREG_INT_STATE, D32, VdhId);

	return;
}

SINT32 VDMHAL_IMP_CheckReg(REG_ID_E reg_id, SINT32 VdhId)
{
	SINT32 *p32;
	SINT32 dat = 0;
	UINT32 reg_type;

	if (VdhId > (MAX_VDH_NUM - 1)) {
		dprint(PRN_FATAL, "%s: Invalid VdhId is %d\n", __func__, VdhId);
		return VDMHAL_ERR;
	}

	if (g_HwMem[VdhId].pVdmRegVirAddr == NULL) {
		if ((p32 = (SINT32 *) MEM_Phy2Vir(gVdhRegBaseAddr)) != NULL) {
			g_HwMem[VdhId].pVdmRegVirAddr = p32;
		} else {
			dprint(PRN_FATAL, " %s %d vdm register virtual address not mapped, reset failed\n", __func__, __LINE__);
			return 0;
		}
	}

	switch (reg_id) {
	case VDH_STATE_REG:
		reg_type = VREG_VDH_STATE;
		break;

	case INT_STATE_REG:
		reg_type = VREG_INT_STATE;
		break;

	case INT_MASK_REG:
		reg_type = VREG_INT_MASK;
		break;

	case VCTRL_STATE_REG:
		reg_type = VREG_VCTRL_STATE;
		break;

	default:
		dprint(PRN_FATAL, "%s: unkown reg_id is %d\n", __func__, reg_id);
		return 0;
	}

	RD_VREG(reg_type, dat, 0);
	return dat;
}

SINT32 VDMHAL_IMP_PrepareDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	VDMHAL_HWMEM_S *pHwMem = &(g_HwMem[0]);
	SINT32 *p32;
	if (NULL == pVdhRegCfg)
	{
		dprint(PRN_FATAL, "%s: parameter is NULL\n", __func__);
		return VDMHAL_ERR;
	}
	if (NULL == pHwMem->pVdmRegVirAddr)
	{
		if (NULL != (p32 = (SINT32 *)MEM_Phy2Vir(gVdhRegBaseAddr)) )
		{
			pHwMem->pVdmRegVirAddr = p32;
		}
		else
		{
			dprint(PRN_FATAL, "vdm register virtual address not mapped, VDMHAL_PrepareDecfailed\n");
			return VDMHAL_ERR;
		}
	}
	if (VFMW_AVS == pVdhRegCfg->VidStd)
		WR_SCDREG(REG_AVS_FLAG, 0x00000001);
	else
		WR_SCDREG(REG_AVS_FLAG, 0x00000000);

	WR_SCDREG(REG_VDH_SELRST, 0x00000001);

	switch (pVdhRegCfg->VidStd) {
#ifdef VFMW_H264_SUPPORT
	case VFMW_H264:
		return H264HAL_StartDec(pVdhRegCfg);
#endif
#ifdef VFMW_HEVC_SUPPORT
	case VFMW_HEVC:
		return HEVCHAL_StartDec(pVdhRegCfg);
#endif
#ifdef VFMW_MPEG2_SUPPORT
	case VFMW_MPEG2:
		return MP2HAL_StartDec(pVdhRegCfg);
#endif
#ifdef VFMW_MPEG4_SUPPORT
	case VFMW_MPEG4:
		return MP4HAL_StartDec(pVdhRegCfg);
#endif
#ifdef VFMW_REAL8_SUPPORT
	case VFMW_REAL8:
		return RV8HAL_StartDec(pVdhRegCfg);
#endif
#ifdef VFMW_REAL9_SUPPORT
	case VFMW_REAL9:
		return RV9HAL_StartDec(pVdhRegCfg);
#endif
#ifdef VFMW_VC1_SUPPORT
	case VFMW_VC1:
		return VC1HAL_StartDec(pVdhRegCfg);
#endif
#ifdef VFMW_VP8_SUPPORT
	case VFMW_VP8:
		return VP8HAL_StartDec(pVdhRegCfg);
#endif
#ifdef VFMW_VP9_SUPPORT

	case VFMW_VP9:
		return VP9HAL_StartDec(pVdhRegCfg);
#endif
#ifdef VFMW_MVC_SUPPORT
	case VFMW_MVC:
		return H264HAL_StartDec(pVdhRegCfg);
#endif
	default:
		break;
	}

	return VDMHAL_ERR;
}

/*****************************************************************************************
  原型    SINT32 VDMHAL_IsVdmReady( )
  功能    检查VDM是否ready
  参数    无

  返回值  类布尔数据，1表示VDM已经ready，反之返回0
 ******************************************************************************************/
SINT32 VDMHAL_IMP_IsVdmReady(SINT32 VdhId)
{
	SINT32 Data32 = 0;

	VDMHAL_ASSERT_RET(g_HwMem[VdhId].pVdmRegVirAddr != NULL, "VDM register not mapped yet!");

	RD_VREG(VREG_VDH_STATE, Data32, VdhId);

	Data32 = (Data32 >> 17) & 1;
	Data32 = (Data32 == 0) ? 0 : 1;

	return Data32;
}

SINT32 VDMHAL_IsVdmRun(SINT32 VdhId)
{
	SINT32 Data32 = 0;

	if (g_HwMem[VdhId].pVdmRegVirAddr == NULL) {
		dprint(PRN_FATAL, "VDM register not mapped yet\n");
		return 0;
	}

	RD_VREG(VREG_VCTRL_STATE, Data32, VdhId);

	if (Data32 == 1)
		return 0;
	else
		return 1;    //work
}

SINT32 VDMHAL_IMP_BackupInfo(VOID)
{
	SINT32 i = 0;

	g_VdmRegState.Int_State_Reg = VDMHAL_IMP_CheckReg(INT_STATE_REG, 0);

	RD_VREG(VREG_BASIC_CFG1, g_VdmRegState.BasicCfg1, 0);
	RD_VREG(VREG_VDH_STATE, g_VdmRegState.VdmState, 0);

	RD_VREG(VREG_MB0_QP_IN_CURR_PIC, g_VdmRegState.Mb0QpInCurrPic, 0);
	RD_VREG(VREG_SWITCH_ROUNDING, g_VdmRegState.SwitchRounding, 0);

	{
		RD_VREG(VREG_SED_STA, g_VdmRegState.SedSta, 0);
		RD_VREG(VREG_SED_END0, g_VdmRegState.SedEnd0, 0);
		RD_VREG(VREG_DEC_CYCLEPERPIC, g_VdmRegState.DecCyclePerPic, 0);
		RD_VREG(VREG_RD_BDWIDTH_PERPIC, g_VdmRegState.RdBdwidthPerPic, 0);
		RD_VREG(VREG_WR_BDWIDTH_PERPIC, g_VdmRegState.WrBdWidthPerPic, 0);
		RD_VREG(VREG_RD_REQ_PERPIC, g_VdmRegState.RdReqPerPic, 0);
		RD_VREG(VREG_WR_REQ_PERPIC, g_VdmRegState.WrReqPerPic, 0);
		RD_VREG(VREG_LUMA_SUM_LOW, g_VdmRegState.LumaSumLow, 0);
		RD_VREG(VREG_LUMA_SUM_HIGH, g_VdmRegState.LumaSumHigh, 0);
	}

	for (i = 0; i < 32; i++) {
		RD_VREG(VREG_LUMA_HISTORGRAM + i * 4, g_VdmRegState.LumaHistorgam[i], 0);/*lint !e679*/
	}

	return VDMHAL_OK;
}

VOID VDMHAL_GetRegState(VDMHAL_BACKUP_S *pVdmRegState)
{
	memcpy(pVdmRegState, &g_VdmRegState, sizeof(g_VdmRegState));
	s_VdmState = VDM_IDLE_STATE;
}

SINT32 VDMHAL_IMP_PrepareRepair(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	VDMHAL_HWMEM_S *pHwMem = &(g_HwMem[0]);
	SINT32 *p32;
	if (NULL == pVdhRegCfg)
	{
		dprint(PRN_FATAL, "%s: parameter is NULL\n", __func__);
		return VDMHAL_ERR;
	}
	if ( NULL == pHwMem->pVdmRegVirAddr )
	{
		if ( NULL != (p32 = (SINT32 *)MEM_Phy2Vir(gVdhRegBaseAddr)) )
		{
			pHwMem->pVdmRegVirAddr = p32;
		}
		else
		{
			dprint(PRN_FATAL, "vdm register virtual address not mapped, VDMHAL_PrepareRepair failed\n");
			return VDMHAL_ERR;
		}
	}
	if (pVdhRegCfg->RepairTime == FIRST_REPAIR) {
		if (pVdhRegCfg->ValidGroupNum0 > 0)
			VDMHAL_CfgRpReg(pVdhRegCfg);
		else
			return VDMHAL_ERR;
	} else if (pVdhRegCfg->RepairTime == SECOND_REPAIR) {
		dprint(PRN_FATAL, "SECOND_REPAIR Parameter Error\n");
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}

VOID VDMHAL_IMP_StartHwRepair(SINT32 VdhId)
{
	SINT32 D32 = 0;

	RD_VREG(VREG_BASIC_CFG0, D32, VdhId);
#ifdef ENV_SOS_KERNEL
	D32 = 0x84000000;
#else
	D32 = 0x4000000;
#endif
	WR_VREG(VREG_BASIC_CFG0, D32, VdhId);

#ifdef HIVDEC_SMMU_SUPPORT
#ifdef ENV_SOS_KERNEL
	SMMU_SetMasterReg(MFDE, SECURE_ON, SMMU_OFF);
#else
	SMMU_SetMasterReg(MFDE, SECURE_OFF, SMMU_ON);
#endif
#endif

	VDMHAL_IMP_ClearIntState(VdhId);
	VDMHAL_EnableInt(VdhId);

	VFMW_OSAL_Mb();
	WR_VREG(VREG_VDH_START, 0, VdhId);
	WR_VREG(VREG_VDH_START, 1, VdhId);
	WR_VREG(VREG_VDH_START, 0, VdhId);

	return;
}

VOID VDMHAL_IMP_StartHwDecode(SINT32 VdhId)
{

#ifdef HIVDEC_SMMU_SUPPORT
#ifdef ENV_SOS_KERNEL
	SMMU_SetMasterReg(MFDE, SECURE_ON, SMMU_OFF);
#else
	SMMU_SetMasterReg(MFDE, SECURE_OFF, SMMU_ON);
#endif
#endif

	VDMHAL_IMP_ClearIntState(VdhId);
	VDMHAL_EnableInt(VdhId);

	VFMW_OSAL_Mb();
	WR_VREG(VREG_VDH_START, 0, 0);
	WR_VREG(VREG_VDH_START, 1, 0);
	WR_VREG(VREG_VDH_START, 0, 0);

	return;
}

VOID VDMHAL_IMP_WriteScdEMARID(VOID)
{
//    WR_SCDREG(REG_EMAR_ID, DEFAULT_EMAR_ID_VALUE);
}

VOID VDMHAL_ISR(SINT32 VdhId)
{
	VDMHAL_IMP_BackupInfo();
	VDMHAL_IMP_ClearIntState(VdhId);
	VFMW_OSAL_GiveEvent(G_VDMHWDONEEVENT);
}

VOID VDMHAL_AfterDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	s_VdmState = VDM_DECODE_STATE;

	if (pVdhRegCfg->VdmStateMachine == VDM_DECODE_STATE) {
		if (pVdhRegCfg->ErrRationAndRpStratageFlag)
			VDMHAL_ActivateVDH(pVdhRegCfg);
		else if (VDMHAL_IMP_PrepareRepair(pVdhRegCfg) == VDMHAL_OK)
			VDMHAL_IMP_StartHwRepair(0);
	} else if (pVdhRegCfg->VdmStateMachine == VDM_REPAIR_STATE_0) {
		if (pVdhRegCfg->AvsSecondFld == 1)
			;
		else if (pVdhRegCfg->IsMpeg4Nvop == 1)
#ifdef VFMW_MPEG4_SUPPORT
			VDMHAL_ActivateVDH(pVdhRegCfg);
#endif
		else if (pVdhRegCfg->IsVc1Skpic == 1)    //vc1 copy
#ifdef VFMW_VC1_SUPPORT
			VDMHAL_ActivateVDH(pVdhRegCfg);
#endif
		else
			VDMHAL_ActivateVDH(pVdhRegCfg);
	} else if (pVdhRegCfg->VdmStateMachine == VDM_REPAIR_STATE_1) {

		VDMHAL_ActivateVDH(pVdhRegCfg);
	}
}

VOID VDMHAL_ActivateVDH(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	s_VdmState = VDM_DECODE_STATE;
#ifdef VFMW_MPEG4_SUPPORT
	if (pVdhRegCfg->IsMpeg4Nvop == 1) {
		VDMHAL_IMP_PrepareRepair(pVdhRegCfg);
		VDMHAL_IMP_StartHwRepair(0);
	}
#endif
#ifdef VFMW_VC1_SUPPORT
	else if (pVdhRegCfg->IsVc1Skpic == 1) {
		VDMHAL_IMP_PrepareRepair(pVdhRegCfg);
		VDMHAL_IMP_StartHwRepair(0);
	}
#endif
	else {
		VDMHAL_IMP_PrepareDec(pVdhRegCfg);
		VDMHAL_IMP_StartHwDecode(0);
	}
}

SINT32 VDMHAL_PrepareSleep(VOID)
{
	SINT32 ret = VDMDRV_OK;

	VFMW_OSAL_SemaDown(G_VDH_SEM);
	if (s_eVdmDrvSleepState == VDMDRV_SLEEP_STAGE_NONE) {
		if (VDM_IDLE_STATE == s_VdmState) {
			dprint(PRN_ALWS, "%s, idle state \n", __func__);
			s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_SLEEP;
		} else {
			dprint(PRN_ALWS, "%s, work state \n", __func__);
			s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_PREPARE;
		}

		ret = VDMDRV_OK;
	} else {
		ret = VDMDRV_ERR;
	}

	VFMW_OSAL_SemaUp(G_VDH_SEM);
	return ret;
}

VOID VDMHAL_ForceSleep(VOID)
{
	dprint(PRN_ALWS, "%s, force state \n", __func__);
	VFMW_OSAL_SemaDown(G_VDH_SEM);
	if (s_eVdmDrvSleepState != VDMDRV_SLEEP_STAGE_SLEEP) {
		VDMHAL_IMP_ResetVdm(0);
		s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_SLEEP;
	}

	VFMW_OSAL_SemaUp(G_VDH_SEM);
}

VOID VDMHAL_ExitSleep(VOID)
{
	VFMW_OSAL_SemaDown(G_VDH_SEM);
	s_eVdmDrvSleepState = VDMDRV_SLEEP_STAGE_NONE;
	VFMW_OSAL_SemaUp(G_VDH_SEM);
}

VDMDRV_SLEEP_STAGE_E VDMHAL_GetSleepStage(VOID)
{
	return s_eVdmDrvSleepState;
}

VOID VDMHAL_SetSleepStage(VDMDRV_SLEEP_STAGE_E sleepState)
{
	VFMW_OSAL_SemaDown(G_VDH_SEM);
	s_eVdmDrvSleepState = sleepState;
	VFMW_OSAL_SemaUp(G_VDH_SEM);
}
