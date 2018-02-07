#include "drv_venc_efl.h"
#include "drv_venc_osal.h"
#include "hi_drv_mem.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*lint -e774 -e697 -e838*/
/*lint -e685 -e568 -e687 -e701 -e713 -e574 -e702 -e737*/
HI_U32 gVencIsFPGA               = 0;
HI_U32 gVeduIrqNumNorm      = 0;
HI_U32 gVeduIrqNumPort        = 0;
HI_U32 gVeduIrqNumSafe       = 0;
HI_U32 gVencRegBaseAddr     = 0;
HI_U32 gVencRegRange           = 0;
HI_U64 gSmmuPageBaseAddr = 0;

U_FUNC_VCPI_RAWINT    g_hw_done_type ;
VEDU_OSAL_EVENT     g_hw_done_event;
VeduEfl_SMMU_ERR_RW_ADDR g_smmu_err_mem;

/*******************************************************************/
VeduEfl_IpCtx_S VeduIpCtx;

HI_S32 VENC_SetDtsConfig(VeduEfl_DTS_CONFIG_S *pDtsConfig)
{
	if (!pDtsConfig){
		HI_ERR_VENC("%s FATAL: pDtsConfig = NULL.\n", __func__);
		return HI_FAILURE;
	}

	if (pDtsConfig->VeduIrqNumNorm == 0 || pDtsConfig->VeduIrqNumProt == 0 || pDtsConfig->VeduIrqNumSafe == 0 || pDtsConfig->VencRegBaseAddr == 0 ||
		pDtsConfig->VencRegRange == 0 || pDtsConfig->SmmuPageBaseAddr == 0){
		HI_ERR_VENC("%s invalid param: VeduIrqNumNorm=%d, VeduIrqNumProt=%d, VeduIrqNumSafe=%d, VencRegBaseAddr=0x%x, VencRegRange=%d, SmmuPageBaseAddr=0x%x\n", __func__,
		       		pDtsConfig->VeduIrqNumNorm, pDtsConfig->VeduIrqNumProt, pDtsConfig->VeduIrqNumSafe, pDtsConfig->VencRegBaseAddr, pDtsConfig->VencRegRange, pDtsConfig->SmmuPageBaseAddr);
		return HI_FAILURE;
	}
	gVencIsFPGA       = pDtsConfig->IsFPGA;
	gVeduIrqNumNorm       = pDtsConfig->VeduIrqNumNorm;
	gVeduIrqNumPort        = pDtsConfig->VeduIrqNumProt;
	gVeduIrqNumSafe    = pDtsConfig->VeduIrqNumSafe;

	gVencRegBaseAddr  = pDtsConfig->VencRegBaseAddr;
	gVencRegRange     = pDtsConfig->VencRegRange;
	gSmmuPageBaseAddr = pDtsConfig->SmmuPageBaseAddr;

	return HI_SUCCESS;
}

static HI_VOID Venc_ISR( HI_VOID )
{
	HI_U32 *pINTCLR     = NULL;
	S_HEVC_AVC_REGS_TYPE *pAllReg = NULL;

	HI_DBG_VENC("enter %s ()\n", __func__);

	if (!VeduIpCtx.pRegBase) {
		HI_ERR_VENC("%s, VeduIpCtx.pRegBase invalid !", __func__);
		return ;
	}
	pAllReg  = (S_HEVC_AVC_REGS_TYPE *)VeduIpCtx.pRegBase;/*lint !e826 */
	pINTCLR  = (HI_U32 *)&(pAllReg->VEDU_VCPI_INTCLR.u32);

	g_hw_done_type.bits.vcpi_rint_vedu_timeout = pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_vedu_timeout;
	g_hw_done_type.bits.vcpi_rint_vedu_slice_end = pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_vedu_slice_end;
	g_hw_done_type.bits.vcpi_rint_ve_eop = pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop;

#ifdef VENC_SIMULATE
	pAllReg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop = 0;
#endif
	if (g_hw_done_type.bits.vcpi_rint_vedu_timeout)
	{
		*pINTCLR = 0x08000000;
		VENC_DRV_OsalGiveEvent(&g_hw_done_event);
	}
	if (g_hw_done_type.bits.vcpi_rint_vedu_slice_end)
		*pINTCLR = 0x400;
	if (g_hw_done_type.bits.vcpi_rint_ve_eop)
	{
		*pINTCLR = 0xFFFFFFFF;
		VENC_DRV_OsalGiveEvent(&g_hw_done_event);
	}
	HI_DBG_VENC("out %s ()\n", __func__);
}

/******************************************************************************
Function   :
Description: IP-VEDU & IP-JPGE Open & Close
Calls      :
Input      :
Output     :
Return     :
Others     :
******************************************************************************/
HI_S32 VENC_DRV_EflOpenVedu(HI_VOID)
{
	HI_S32 s32Ret = HI_SUCCESS;
	MEM_BUFFER_S  MEM_SMMU_START_ADDR0;
	MEM_BUFFER_S  MEM_SMMU_START_ADDR1;

	HI_DBG_VENC("enter %s()\n", __func__);

	HiMemSet((HI_VOID *)&VeduIpCtx, 0, sizeof(VeduIpCtx));

	if (VENC_DRV_OsalLockCreate( &VeduIpCtx.pChnLock ) == HI_FAILURE){
		HI_ERR_VENC("%s, VENC_DRV_OsalLockCreate failed\n", __func__);
		return HI_FAILURE;
	}

	VeduIpCtx.pRegBase = (HI_U32 *)HiMmap(gVencRegBaseAddr, gVencRegRange);

	if (!VeduIpCtx.pRegBase){
		HI_ERR_VENC("%s, ioremap failed\n", __func__);
		VENC_DRV_OsalLockDestroy( VeduIpCtx.pChnLock );
		return HI_FAILURE;
	}

	HI_DBG_VENC("%s, HI_DDR_MEM_Init\n", __func__);
	if (HI_SUCCESS != DRV_MEM_INIT()) {
		HI_ERR_VENC("DRV_MEM_INIT failed!\n");
		VENC_DRV_OsalLockDestroy( VeduIpCtx.pChnLock );
		HiMunmap(VeduIpCtx.pRegBase);
		return HI_FAILURE;
	}

	HiMemSet((HI_VOID *)&g_smmu_err_mem, 0, sizeof(g_smmu_err_mem));
	HiMemSet((HI_VOID *)&MEM_SMMU_START_ADDR0, 0, sizeof(MEM_BUFFER_S));
	HiMemSet((HI_VOID *)&MEM_SMMU_START_ADDR1, 0, sizeof(MEM_BUFFER_S));

	MEM_SMMU_START_ADDR0.u32Size = SMMU_RWERRADDR_SIZE;
	s32Ret = DRV_MEM_KAlloc("SMMU_RDERR", "OMXVENC", &MEM_SMMU_START_ADDR0);
	if (s32Ret != HI_SUCCESS ) {
		VENC_DRV_OsalLockDestroy( VeduIpCtx.pChnLock );
		HiMunmap(VeduIpCtx.pRegBase);
		HI_ERR_VENC("%s, call DRV_MEM_KAlloc SMMU_RDERR Mem failed!\n", __func__);
		return HI_FAILURE;
	}

	MEM_SMMU_START_ADDR1.u32Size = SMMU_RWERRADDR_SIZE;
	s32Ret = DRV_MEM_KAlloc("SMMU_WRERR", "OMXVENC", &MEM_SMMU_START_ADDR1);
	if (s32Ret != HI_SUCCESS ) {
		VENC_DRV_OsalLockDestroy( VeduIpCtx.pChnLock );
		HiMunmap(VeduIpCtx.pRegBase);
		DRV_MEM_KFree(&MEM_SMMU_START_ADDR0);
		HI_ERR_VENC("%s, DRV_MEM_KAlloc  MMU Addr failed\n", __func__);
		return HI_FAILURE;
	}

	g_smmu_err_mem.RdAddr = MEM_SMMU_START_ADDR0.u64StartPhyAddr;//config alloc phyaddr,in order system don't dump
	g_smmu_err_mem.WrAddr = MEM_SMMU_START_ADDR1.u64StartPhyAddr;

	VeduIpCtx.IpFree = 1;
	VENC_HAL_SetSmmuAddr((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */
	VENC_HAL_DisableAllInt((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */
	VENC_HAL_ClrAllInt    ((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */
#ifdef IRQ_EN
	if (VENC_DRV_OsalIrqInit(gVeduIrqNumNorm, Venc_ISR) == HI_FAILURE){
		HI_ERR_VENC("%s, VENC_DRV_OsalIrqInit failed\n", __func__);
		VENC_DRV_OsalLockDestroy( VeduIpCtx.pChnLock );
		HiMunmap(VeduIpCtx.pRegBase);
		DRV_MEM_KFree(&MEM_SMMU_START_ADDR0);
		DRV_MEM_KFree(&MEM_SMMU_START_ADDR1);
		return HI_FAILURE;
	}
#endif
	/* creat thread to manage channel */
	VeduIpCtx.StopTask    = 0;
	VeduIpCtx.TaskRunning = 0;

	VENC_DRV_OsalInitEvent(&g_hw_done_event, 0);

	HI_DBG_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;
}

HI_S32 VENC_DRV_EflCloseVedu( HI_VOID )
{
	HI_U32 TimeOutCnt = 0;
#ifdef MD5_WC_EN
	int   i = 0;
	HI_U8 digesttmp[16] ;
	HI_U8 digesttmp2[100] ;
	HiMemSet(digesttmp, 0, 16);
	HiMemSet(digesttmp2, 0, 100);
#endif
	HI_DBG_VENC("enter %s()\n", __func__);
	VeduIpCtx.StopTask = 1;

	while ((VeduIpCtx.TaskRunning) && (TimeOutCnt < 100)) {
		HiSleepMs(1);
		TimeOutCnt ++;
	}

	VENC_HAL_DisableAllInt((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */
	VENC_HAL_ClrAllInt((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */

#ifdef IRQ_EN
	VENC_DRV_OsalIrqFree(gVeduIrqNumNorm);
#endif
	HiMunmap(VeduIpCtx.pRegBase);
	DRV_MEM_EXIT();
	VENC_DRV_OsalLockDestroy( VeduIpCtx.pChnLock );

	HI_DBG_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;
}

HI_S32 VENC_DRV_EflSuspendVedu(HI_VOID)
{
	HI_U32 TimeOutCnt = 0;
	HI_INFO_VENC("enter %s()\n", __func__);

	VeduIpCtx.StopTask = 1;

	while ((VeduIpCtx.TaskRunning) && (TimeOutCnt < 100)) {
		HiSleepMs(1);
		TimeOutCnt ++;
	}

	HI_INFO_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;
}

HI_S32 VENC_DRV_EflResumeVedu(HI_VOID)
{
	HI_INFO_VENC("enter %s()\n", __func__);

	VeduIpCtx.StopTask    = 0;
	VeduIpCtx.TaskRunning = 0;

	HI_INFO_VENC("out\n");
	return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
