/*
 * vdec hal for mp4
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
#include "basedef.h"
#include "vfmw.h"
#include "mem_manage.h"
#include "public.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"
#include "vdm_hal_mpeg4.h"

SINT32 MP4HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg);

SINT32 MP4HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	SINT32 ret;

	ret = MP4HAL_CfgReg(pVdhRegCfg);

	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "MP4HAL_CfgReg failed\n");
		return VDMHAL_ERR;
	}
	return VDMHAL_OK;
}

SINT32 MP4HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	SINT32 D32;

	//BASIC_CFG0
	D32 = 0;
	D32 = (pVdhRegCfg->VdhBasicCfg0 & 0x000FFFFF) // [15:0] mbamt_to_dec
		  | ( 1 << 22 )
		  | ( 0 << 23 )
		  | ( 0 << 24 )
		  | ( 1 << 25 )
		  | ( 1 << 30 )     // ld_qmatrix_flag
#ifdef ENV_SOS_KERNEL
		  | ( 1 << 31 );    // Secure Mode
#else
		  | ( 0 << 31 );    // Normal Mode
#endif
	WR_VREG( VREG_BASIC_CFG0, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG0 : 0x%x\n", D32);

	/*set uv order 0: v first; 1: u first */
	D32 = 0x2               // [3:0] video_standard
		  | (((pVdhRegCfg->VdhBasicCfg1 >> 13) & 0x1) << 13 )     // uv_order_en
		  | ( 1 << 14 )     // [14] fst_slc_grp
		  | ( 1 << 15 )     // [15] mv_output_en
		  | ( 1 << 16 )     // [27:16] max_slcgrp_num
		  | ( 0 << 28)      // line_num_output_en
		  | ( 1 << 29)
		  | (((pVdhRegCfg->VdhBasicCfg1 >> 30) & 0x1) << 30)   //compress_en
		  | ( 0 << 31 );    // [31] ppfd_en   0==not ppfd dec
	WR_VREG(VREG_BASIC_CFG1, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG1 : 0x%x\n", D32);

	D32 = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;  // mpeg4 down msg
	WR_VREG(VREG_AVM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "AVM_ADDR : 0x%x\n", D32);

	D32 = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;  // mpeg4 up msg
	WR_VREG(VREG_VAM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "VAM_ADDR : 0x%x\n", D32);

	WR_VREG(VREG_STREAM_BASE_ADDR, pVdhRegCfg->VdhStreamBaseAddr, 0);
	dprint(PRN_VDMREG, "STREAM_BASE_ADDR : 0x%x\n", pVdhRegCfg->VdhStreamBaseAddr);

	D32 = 0x00300C03;
	WR_VREG(VREG_SED_TO, D32, 0);
	WR_VREG(VREG_ITRANS_TO, D32, 0);
	WR_VREG(VREG_PMV_TO, D32, 0);
	WR_VREG(VREG_PRC_TO, D32, 0);
	WR_VREG(VREG_RCN_TO, D32, 0);
	WR_VREG(VREG_DBLK_TO, D32, 0);
	WR_VREG(VREG_PPFD_TO, D32, 0);

	D32 = (pVdhRegCfg->VdhYstAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_YSTADDR_1D, D32, 0);
	dprint(PRN_VDMREG, "YSTADDR_1D : 0x%x\n", D32);

	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", pVdhRegCfg->VdhYstride);

	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", pVdhRegCfg->VdhUvoffset);

	D32 = 0;
	WR_VREG(VREG_HEAD_INF_OFFSET, D32, 0);

	D32 = 0;
	WR_VREG(VREG_FF_APT_EN, D32, 0);
	dprint(PRN_VDMREG, "PRCMEM2_1D_CNT : 0x%x\n", D32);

	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );

	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);
	dprint(PRN_VDMREG, "pPicParam->cfginfo_msg_addr:%x\n", pVdhRegCfg->VdhCfgInfoAddr);

	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);

	return VDMHAL_OK;
}
