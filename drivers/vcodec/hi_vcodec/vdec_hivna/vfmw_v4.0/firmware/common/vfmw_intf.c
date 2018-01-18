/*
 * vfmw interface
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
/*lint -e652*/
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kern_levels.h>

#include "basedef.h"
#include "public.h"
#include "vfmw_intf.h"

#ifdef HI_TVP_SUPPORT
#include "tvp_adapter.h"
#endif

#ifdef VFMW_VC1_SUPPORT
#endif
#ifdef HIVDEC_SMMU_SUPPORT
#include "smmu.h"
#include "smmu_regs.h"
#endif
#include "vdm_hal_api.h"

#ifndef IRQ_HANDLED
#define IRQ_HANDLED                 (1)
#endif

#ifndef IRQF_DISABLED
#define IRQF_DISABLED               (0x00000020)
#endif
#define VDM_TIMEOUT               (1000)//ms
#define VDM_FPGA_TIMEOUT          (500000)//ms
#define SCD_TIMEOUT               (2000)//ms
#define SCD_FPGA_TIMEOUT          (200000)//ms
#define SCEN_IDENT                (0x828)

#define TIME_PERIOD(begin, end) ((end >= begin)? (end-begin):(0xffffffff - begin + end))

#ifdef HI_ADVCA_FUNCTION_RELEASE
#define ModPrint(format,arg...)    ({do{}while(0);0;})
#else
#define ModPrint(format,arg...) \
do { \
	printk(KERN_ALERT format, ## arg); \
} while (0)
#endif

#define  DO_OPEN_DRV_ERR()                                  \
do {                                                        \
	VCTRL_CloseVfmw();                                \
	return VCTRL_ERR;                                       \
} while(0)

static DRV_MEM_S g_RegsBaseAddr;

Vfmw_Osal_Func_Ptr g_vfmw_osal_fun_ptr;

VOID VCTRL_Suspend(VOID)
{
	UINT8 isScdSleep = 0;
	UINT8 isVdmSleep = 0;
	UINT8 isBpdSleep = 0;

	UINT32 SleepCount = 0;
	UINT32 BeginTime, EntrTime, CurTime;

	EntrTime = VFMW_OSAL_GetTimeInMs();

	SCDDRV_PrepareSleep();

	VDMHAL_PrepareSleep();

	BPDDRV_PrepareSleep();

	BeginTime = VFMW_OSAL_GetTimeInMs();
	do {
		if (SCDDRV_SLEEP_STAGE_SLEEP == SCDDRV_GetSleepStage())
			isScdSleep = 1;

		if (VDMHAL_GetSleepStage() == VDMDRV_SLEEP_STAGE_SLEEP)
			isVdmSleep = 1;

		if (BPDDRV_GetSleepStage() == BPDDRV_SLEEP_STAGE_SLEEP)
			isBpdSleep = 1;

		if ((isScdSleep == 1) && (isVdmSleep == 1) && (isBpdSleep == 1)) {
			break;
		} else {
			if (SleepCount > 30) {
				if (isScdSleep != 1) {
					dprint(PRN_FATAL, "Force scd sleep\n");
					SCDDRV_ForceSleep();
				}
				if (isVdmSleep != 1) {
					dprint(PRN_FATAL, "Force vdm sleep\n");
					VDMHAL_ForceSleep();
				}
				if (isBpdSleep != 1) {
					dprint(PRN_FATAL, "Force bpd sleep\n");
					BPDDRV_ForceSleep();
				}
				break;
			}

			VFMW_OSAL_mSleep(10);
			SleepCount++;
		}
	} while ((isScdSleep != 1) || (isVdmSleep != 1) || (isBpdSleep != 1));

#ifdef HI_TVP_SUPPORT
	if (TVP_VDEC_Suspend() != VDEC_OK)
		dprint(PRN_ALWS, "%s, Warning : TVP_VDEC_Suspend failed", __func__);
#endif

	CurTime = VFMW_OSAL_GetTimeInMs();
	dprint(PRN_FATAL, "Vfmw suspend totally take %d ms\n", TIME_PERIOD(EntrTime, CurTime));

	return;
}

VOID VCTRL_Resume(VOID)
{
	UINT32 EntrTime, CurTime;

	EntrTime = VFMW_OSAL_GetTimeInMs();

	VDMHAL_IMP_WriteScdEMARID();
	SMMU_InitGlobalReg();

	SCDDRV_ExitSleep();

	VDMHAL_ExitSleep();

	BPDDRV_ExitSleep();

#ifdef HI_TVP_SUPPORT
	if (TVP_VDEC_Resume() != VDEC_OK)
		dprint(PRN_ALWS, "%s, Warning : TVP_VDEC_Resume failed", __func__);
#endif

	CurTime = VFMW_OSAL_GetTimeInMs();
	dprint(PRN_FATAL, "Vfmw resume totally take %d ms\n", TIME_PERIOD(EntrTime, CurTime));

	return;
}

static SINT32 VCTRL_ISR(SINT32 irq, VOID *dev_id)
{
	UINT32 D32;
	D32 = RD_SCDREG(REG_SCD_INI_CLR)&0x1;
	if (D32 == 1)
		SCDDRV_ISR();

	RD_VREG(VREG_INT_STATE, D32, 0);
	if (D32 == 1)
		VDMHAL_ISR(0);

	/* RD_VREG(SMMU_INTSTAT_NS, D32, 0);
	if (D32 != 0)
		SMMU_IntServProc();*/
	return IRQ_HANDLED;
}

static SINT32 VCTRL_RequestIrq(UINT32 IrqNumNorm, UINT32 IrqNumProt, UINT32 IrqNumSafe)
{
#if !defined(VDM_BUSY_WAITTING)
	if (VFMW_OSAL_RequestIrq(IrqNumNorm, VCTRL_ISR, IRQF_DISABLED, "vdec_norm_irq", NULL) != 0) {    //for 2.6.24以后
		dprint(PRN_FATAL, "Request vdec norm irq %d failed\n", IrqNumNorm);
		return VCTRL_ERR;
	}
#endif

#if !defined(SMMU_BUSY_WAITTING)
#ifdef ENV_SOS_KERNEL
	if (VFMW_OSAL_RequestIrq(IrqNumProt, VCTRL_ISR, IRQF_DISABLED, "vdec_prot_smmu_irq", NULL) != 0) {    //for 2.6.24以后
		dprint(PRN_FATAL, "Request vdec prot irq %d failed\n", IrqNumProt);
		return VCTRL_ERR;
	}
#endif
#endif

	return VCTRL_OK;
}

static VOID VCTRL_FreeIrq(UINT32 IrqNumNorm, UINT32 IrqNumProt, UINT32 IrqNumSafe)
{
#if !defined(VDM_BUSY_WAITTING)
	VFMW_OSAL_FreeIrq(IrqNumNorm, NULL);
#endif

#if !defined(SMMU_BUSY_WAITTING)
#ifdef ENV_SOS_KERNEL
	VFMW_OSAL_FreeIrq(IrqNumProt, NULL);
#endif
#endif
}

static SINT32 VCTRL_HalInit(VOID)
{
#ifdef HIVDEC_SMMU_SUPPORT
	if (SMMU_Init() != SMMU_OK) {
		dprint(PRN_FATAL, "SMMU_Init failed\n");
		return VCTRL_ERR;
	}
#endif

	SCDDRV_init();
	VDMHAL_IMP_Init();
	SMMU_InitGlobalReg();

	return VCTRL_OK;
}

static VOID VCTRL_HalDeInit(VOID)
{
#ifdef HIVDEC_SMMU_SUPPORT
	SMMU_DeInit();
#endif
	VDMHAL_IMP_DeInit();
	SCDDRV_DeInit();
}

SINT32 VCTRL_OpenDrivers(VOID)
{
	MEM_RECORD_S *pstMem;
	SINT32 ret   = VCTRL_ERR;

	pstMem = &g_RegsBaseAddr.stVdhReg;
	if (MEM_MapRegisterAddr(gVdhRegBaseAddr, 256 * 1024, pstMem) == MEM_MAN_OK) {
		if (MEM_AddMemRecord(pstMem->PhyAddr, pstMem->VirAddr, pstMem->Length) != MEM_MAN_OK) {
			dprint(PRN_ERROR, "%s %d MEM_AddMemRecord failed\n", __func__, __LINE__);
			goto exit;
		}
	} else {
		dprint(PRN_FATAL, "Map vdh register failed! gVdhRegBaseAddr : 0x%x, gVdhRegRange : %d\n",
			gVdhRegBaseAddr, gVdhRegRange);
		goto exit;
	}

	ret = VCTRL_RequestIrq(gVdecIrqNumNorm, gVdecIrqNumProt, gVdecIrqNumSafe);
	if (ret != VCTRL_OK) {
		dprint(PRN_FATAL, "VCTRL_RequestIrq failed\n");
		goto exit;
	}

	if (VCTRL_HalInit() != VCTRL_OK) {
		dprint(PRN_FATAL, "VCTRL_HalInit failed\n");
		goto exit;
	}

	return VCTRL_OK;

exit:
	DO_OPEN_DRV_ERR();
}

SINT32 VCTRL_OpenVfmw(VOID)
{
	memset(&g_RegsBaseAddr, 0, sizeof(g_RegsBaseAddr));

	MEM_InitMemManager();
	if (VCTRL_OpenDrivers() != VCTRL_OK) {
		dprint(PRN_FATAL, "OpenDrivers fail\n");
		return VCTRL_ERR;
	}

#ifdef HI_TVP_SUPPORT
	if (TVP_VDEC_SecureInit() != VDEC_OK)
		ModPrint("%s, TVP_VDEC_SecureInit failed\n", __func__);
#endif

	return VCTRL_OK;
}

SINT32 VCTRL_CloseVfmw(VOID)
{
	MEM_RECORD_S *pstMem;

	VCTRL_HalDeInit();

	pstMem = &g_RegsBaseAddr.stVdhReg;
	if (pstMem->Length != 0) {
		MEM_UnmapRegisterAddr(pstMem->PhyAddr, pstMem->VirAddr, pstMem->Length);
		memset(pstMem, 0, sizeof(*pstMem));
	}
	MEM_DelMemRecord(pstMem->PhyAddr, pstMem->VirAddr, pstMem->Length);

	pstMem = &g_RegsBaseAddr.stCrgReg;
	if (pstMem->Length != 0) {
		MEM_UnmapRegisterAddr(pstMem->PhyAddr, pstMem->VirAddr, pstMem->Length);
		memset(pstMem, 0, sizeof(*pstMem));
	}
	MEM_DelMemRecord(pstMem->PhyAddr, pstMem->VirAddr, pstMem->Length);

	VCTRL_FreeIrq(gVdecIrqNumNorm, gVdecIrqNumProt, gVdecIrqNumSafe);

#ifdef HI_TVP_SUPPORT
	TVP_VDEC_SecureExit();
#endif
	return VCTRL_OK;
}

SINT32 VCTRL_VDMHal_Process(OMXVDH_REG_CFG_S *pVdmRegCfg, VDMHAL_BACKUP_S *pVdmRegState)
{
	HI_S32 Ret = HI_SUCCESS;
	VDMDRV_SLEEP_STAGE_E sleepState;
	CONFIG_VDH_CMD cmd = pVdmRegCfg->vdh_cmd;

	sleepState = VDMHAL_GetSleepStage();
	if (VDMDRV_SLEEP_STAGE_SLEEP == sleepState) {
		dprint(PRN_ALWS, "vdm sleep state\n");
		return HI_FAILURE;
	}

	if (pVdmRegCfg->vdh_reset_flag)
		VDMHAL_IMP_ResetVdm(0);

	switch (cmd) {
	case CONFIG_VDH_AfterDec_CMD:
		VDMHAL_AfterDec(pVdmRegCfg);
		break;

	case CONFIG_VDH_ACTIVATEDEC_CMD:
		VDMHAL_ActivateVDH(pVdmRegCfg);
		break;

	default:
		dprint(PRN_FATAL, " %s  , cmd type unknown:%d\n", cmd, __func__);
		return HI_FAILURE;
	}

	Ret = VFMW_OSAL_WaitEvent(G_VDMHWDONEEVENT, ((0 == gIsFPGA) ? VDM_TIMEOUT : VDM_FPGA_TIMEOUT));
	if (Ret == HI_SUCCESS) {
		VDMHAL_GetRegState(pVdmRegState);
	} else {
		dprint(PRN_FATAL, "VFMW_OSAL_WaitEvent wait time out\n");
		VDMHAL_IMP_ResetVdm(0);
	}

	sleepState = VDMHAL_GetSleepStage();
	if (sleepState == VDMDRV_SLEEP_STAGE_PREPARE)
		VDMHAL_SetSleepStage(VDMDRV_SLEEP_STAGE_SLEEP);

	return Ret;
}

SINT32 VCTRL_SCDHal_Process(OMXSCD_REG_CFG_S *pScdRegCfg,SCD_STATE_REG_S *pScdStateReg)
{
	HI_S32 Ret = HI_SUCCESS;
	SCDDRV_SLEEP_STAGE_E sleepState;
	CONFIG_SCD_CMD cmd = pScdRegCfg->cmd;

	sleepState = SCDDRV_GetSleepStage();
	if (SCDDRV_SLEEP_STAGE_SLEEP == sleepState) {
		dprint(PRN_ALWS, "SCD sleep state\n");
		return HI_FAILURE;
	}

	if (pScdRegCfg->SResetFlag) {
		if (SCDDRV_ResetSCD() != HI_SUCCESS) {
			dprint(PRN_FATAL, "VDEC_IOCTL_SCD_WAIT_HW_DONE  Reset SCD failed\n");
			return HI_FAILURE;
		}
	}

	switch (cmd) {
	case CONFIG_SCD_REG_CMD:
		Ret = SCDDRV_WriteReg(&pScdRegCfg->SmCtrlReg);
		if (Ret != HI_SUCCESS) {
			dprint(PRN_FATAL, "SCD busy\n");
			return HI_FAILURE;
		}

		Ret = VFMW_OSAL_WaitEvent(G_SCDHWDONEEVENT, ((0 == gIsFPGA) ? SCD_TIMEOUT : SCD_FPGA_TIMEOUT));
		if (Ret == HI_SUCCESS) {
			SCDDRV_GetRegState(pScdStateReg);
		} else {
			dprint(PRN_ALWS, "VDEC_IOCTL_SCD_WAIT_HW_DONE  wait time out\n");
			SCDDRV_ResetSCD();
		}

		sleepState = SCDDRV_GetSleepStage();
		if (sleepState == SCDDRV_SLEEP_STAGE_PREPARE) {
			SCDDRV_SetSleepStage(SCDDRV_SLEEP_STAGE_SLEEP);
		}
		break;

	default:
		dprint(PRN_ALWS, " cmd type unknown:%d\n", cmd);
		return HI_FAILURE;
	}

	return Ret;
}

SINT32 VCTRL_BPDHal_Process(OMXBPD_REG_S *pBpdReg)
{
	HI_S32 Ret = HI_SUCCESS;

	BPDDRV_SLEEP_STAGE_E sleepState = BPDDRV_GetSleepStage();
	if (sleepState == BPDDRV_SLEEP_STAGE_SLEEP) {
		dprint(PRN_ALWS, "BPD sleep state\n");
		return HI_FAILURE;
	}

	if (BPD_ConfigReg(pBpdReg)) {
		dprint(PRN_ALWS, "omxvdec_config_bpd:  failed\n");
		Ret = HI_FAILURE;
	}

	sleepState = BPDDRV_GetSleepStage();
	if (sleepState == BPDDRV_SLEEP_STAGE_PREPARE)
		BPDDRV_SetSleepStage(BPDDRV_SLEEP_STAGE_SLEEP);

	return Ret;
}

SINT32 VCTRL_VDMHAL_IsRun(VOID)
{
	return VDMHAL_IsVdmRun(0);
}

HI_BOOL VCTRL_Scen_Ident(VOID)
{
	return (RD_SCDREG(SCEN_IDENT) == 1) ? HI_TRUE : HI_FALSE;
}

HI_S32 VFMW_DRV_ModInit(HI_VOID)
{
	OSAL_InitInterface();
	VFMW_OSAL_SemaInit(G_SCD_SEM);
	VFMW_OSAL_SemaInit(G_VDH_SEM);
	VFMW_OSAL_SemaInit(G_BPD_SEM);

	VFMW_OSAL_SpinLockInit(G_SPINLOCK_SCD);
	VFMW_OSAL_SpinLockInit(G_SPINLOCK_VDH);
	VFMW_OSAL_SpinLockInit(G_SPINLOCK_RECORD);
	VFMW_OSAL_InitEvent(G_SCDHWDONEEVENT, 0);
	VFMW_OSAL_InitEvent(G_VDMHWDONEEVENT, 0);

#ifdef MODULE
	ModPrint("Load hi_vfmw.ko (%d) success.\n", VFMW_VERSION_NUM);
#endif

	return 0;
}

HI_VOID VFMW_DRV_ModExit(HI_VOID)
{
#ifdef MODULE
	ModPrint("Unload hi_vfmw.ko (%d) success.\n", VFMW_VERSION_NUM);
#endif

	return;
}

module_init(VFMW_DRV_ModInit);
module_exit(VFMW_DRV_ModExit);

MODULE_AUTHOR("gaoyajun");
MODULE_LICENSE("GPL");
