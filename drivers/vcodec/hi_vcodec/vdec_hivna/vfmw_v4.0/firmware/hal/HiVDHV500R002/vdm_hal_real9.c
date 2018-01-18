/*
 * vdec hal for rv9
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */

#ifndef __VDM_HAL_REAL9_C__
#define __VDM_HAL_REAL9_C__

#include "public.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"
#include "vdm_hal_real9.h"
#include "vdm_hal.h"

VOID RV9HAL_WriteReg(OMXVDH_REG_CFG_S *pVdhRegCfg);

SINT32 RV9HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	RV9HAL_WriteReg(pVdhRegCfg);
	return VDMHAL_OK;
}

VOID RV9HAL_WriteReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	SINT32 D32;

	dprint(PRN_CTRL, "configuring VDM registers...\n");
	D32 = 0;
	((RV9_BASIC_CFG0 *)(&D32))->mbamt_to_dec             = ((RV9_BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec;
	((RV9_BASIC_CFG0 *)(&D32))->ld_qmatrix_flag          = 0;
	((RV9_BASIC_CFG0 *)(&D32))->marker_bit_detect_en     = 0;
	((RV9_BASIC_CFG0 *)(&D32))->ac_last_detect_en        = 0;
	((RV9_BASIC_CFG0 *)(&D32))->coef_idx_detect_en       = 1;
	((RV9_BASIC_CFG0 *)(&D32))->vop_type_detect_en       = 0;
#ifdef ENV_SOS_KERNEL
	((RV9_BASIC_CFG0 *)(&D32))->sec_mode_en              = 1;
#else
	((RV9_BASIC_CFG0 *)(&D32))->sec_mode_en              = 0;
#endif

	WR_VREG( VREG_BASIC_CFG0, D32, 0 );
	dprint(PRN_VDMREG, "BASIC_CFG0 : 0x%x\n", D32);

	D32 = 0;
	((RV9_BASIC_CFG1 *)(&D32))->video_standard       = 0x9;
	//((RV9_BASIC_CFG1 *)(&D32))->ddr_stride           = ((RV9_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->ddr_stride;
	((RV9_BASIC_CFG1 *)(&D32))->fst_slc_grp          = ((RV9_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->fst_slc_grp;
	((RV9_BASIC_CFG1 *)(&D32))->mv_output_en         = ((RV9_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->mv_output_en;
	((RV9_BASIC_CFG1 *)(&D32))->uv_order_en          = ((RV9_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((RV9_BASIC_CFG1 *)(&D32))->vdh_2d_en            = 1;
	((RV9_BASIC_CFG1 *)(&D32))->max_slcgrp_num       = 1;//0;
	((RV9_BASIC_CFG1 *)(&D32))->line_num_output_en   = 0;
	((RV9_BASIC_CFG1 *)(&D32))->compress_en          = ((RV9_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->compress_en;
	((RV9_BASIC_CFG1 *)(&D32))->ppfd_en              = 0;
	/*set uv order 0: v first; 1: u first */
	WR_VREG(VREG_BASIC_CFG1, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG1 : 0x%x\n", D32);

	//AVM_ADDR
	D32 = 0;
	((RV9_AVM_ADDR *)(&D32))->av_msg_addr = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_AVM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "AVM_ADDR : 0x%x\n", D32);

	//VAM_ADDR
	D32 = 0;
	((RV9_VAM_ADDR *)(&D32))->va_msg_addr = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_VAM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "VAM_ADDR : 0x%x\n", D32);

	//STREAM_BASE_ADDR
	WR_VREG(VREG_STREAM_BASE_ADDR, pVdhRegCfg->VdhStreamBaseAddr, 0);
	dprint(PRN_VDMREG, "STREAM_BASE_ADDR : 0x%x\n", pVdhRegCfg->VdhStreamBaseAddr);

	//TIME_OUT
	D32 = 0x00300C03;
	WR_VREG(VREG_SED_TO, D32, 0);
	WR_VREG(VREG_ITRANS_TO, D32, 0);
	WR_VREG(VREG_PMV_TO, D32, 0);
	WR_VREG(VREG_PRC_TO, D32, 0);
	WR_VREG(VREG_RCN_TO, D32, 0);
	WR_VREG(VREG_DBLK_TO, D32, 0);
	WR_VREG(VREG_PPFD_TO, D32, 0);
	dprint(PRN_VDMREG, "TIME_OUT : 0x%x\n", D32);

	//YSTADDR_1D
	D32 = (pVdhRegCfg->VdhYstAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_YSTADDR_1D, D32, 0);
	dprint(PRN_VDMREG, "YSTADDR_1D : 0x%x\n", D32);

	//YSTRIDE_1D
	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", pVdhRegCfg->VdhYstride);

	//UVOFFSET_1D
	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", pVdhRegCfg->VdhUvoffset);

	//PRCNUM_1D_CNT
	WR_VREG(VREG_HEAD_INF_OFFSET, pVdhRegCfg->VdhHeadInfOffset, 0);

	//REF_PIC_TYPE
	D32 = 0;
	WR_VREG(VREG_REF_PIC_TYPE, D32, 0);
	dprint(PRN_VDMREG, "REF_PIC_TYPE : 0x%x\n", D32);

	//FF_APT_EN
	D32 = 0;
	((RV9_FF_APT_EN *)(&D32))->ff_apt_en = 0;  //Always use FrameSave Mode, USE_FF_APT_EN
	WR_VREG(VREG_FF_APT_EN, D32, 0);
	dprint(PRN_VDMREG, "FF_APT_EN : 0x%x\n", D32);

	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );

	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);

	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);
}

#endif
