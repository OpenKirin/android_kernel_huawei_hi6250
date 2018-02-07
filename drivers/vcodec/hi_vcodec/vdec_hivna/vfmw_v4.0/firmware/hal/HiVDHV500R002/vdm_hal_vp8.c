/*
 * vdec hal for vp8
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
#ifndef __VDM_HAL_VP8_C__
#define __VDM_HAL_VP8_C__

#include "public.h"
#include "vdm_hal.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"
#include "vdm_hal_vp8.h"

SINT32 VP8HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	SINT32 D32;

	//BASIC_CFG0
	D32 = 0;
	((BASIC_CFG0 *)(&D32))->mbamt_to_dec      = ((BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec;
	((BASIC_CFG0 *)(&D32))->load_qmatrix_flag = 0;
#ifdef ENV_SOS_KERNEL
	((BASIC_CFG0 *)(&D32))->sec_mode_en       = 1;
#else
	((BASIC_CFG0 *)(&D32))->sec_mode_en       = 0;
#endif
	WR_VREG( VREG_BASIC_CFG0, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG0 : 0x%x\n", pVdhRegCfg->VdhBasicCfg0);

	//BASIC_CFG1
	/*set uv order 0: v first; 1: u first */
	D32 = 0x20000000;
	((BASIC_CFG1 *)(&D32))->video_standard    = 0x0C;
	//((BASIC_CFG1 *)(&D32))->ddr_stride        = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->ddr_stride;
	((BASIC_CFG1 *)(&D32))->fst_slc_grp       = 1;
	((BASIC_CFG1 *)(&D32))->mv_output_en      = 1;
	((BASIC_CFG1 *)(&D32))->uv_order_en       = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((BASIC_CFG1 *)(&D32))->vdh_2d_en         = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->vdh_2d_en;
	((BASIC_CFG1 *)(&D32))->max_slcgrp_num    = 0;
	((BASIC_CFG1 *)(&D32))->compress_en       = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->compress_en;
	((BASIC_CFG1 *)(&D32))->ppfd_en           = 0;
	WR_VREG( VREG_BASIC_CFG1, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG1 : 0x%x\n", pVdhRegCfg->VdhBasicCfg1);

	//AVM_ADDR
	D32 = 0;
	((AVM_ADDR *)(&D32))->av_msg_addr = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_AVM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "AVM_ADDR : 0x%x\n", D32);

	//VAM_ADDR
	D32 = 0;
	((VAM_ADDR *)(&D32))->va_msg_addr = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_VAM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "VAM_ADDR : 0x%x\n", D32);

	//STREAM_BASE_ADDR
	WR_VREG(VREG_STREAM_BASE_ADDR, pVdhRegCfg->VdhStreamBaseAddr, 0);
	dprint(PRN_VDMREG, "STREAM_BASE_ADDR : 0x%x\n", pVdhRegCfg->VdhStreamBaseAddr);

	//PPFD_BUF_ADDR
	D32 = (pVdhRegCfg->VdhPpfdBufAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_PPFD_BUF_ADDR, D32, 0);
	dprint(PRN_VDMREG, "PPFD_BUF_ADDR : 0x%x\n", D32);

	//PPFD_BUF_LEN
	WR_VREG(VREG_PPFD_BUF_LEN, pVdhRegCfg->VdhPpfdBufLen, 0);
	dprint(PRN_VDMREG, "PPFD_BUF_LEN : 0x%x\n", pVdhRegCfg->VdhPpfdBufLen);


	//YSTADDR_1D
	D32 = 0;
	((YSTADDR_1D *)(&D32))->ystaddr_1d = (pVdhRegCfg->VdhYstAddr) & 0xFFFFFFF0; //caution
	WR_VREG(VREG_YSTADDR_1D, D32, 0);
	dprint(PRN_VDMREG, "YSTADDR_1D : 0x%x\n", D32);

	//YSTRIDE_1D
	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", pVdhRegCfg->VdhYstride);

	//UVOFFSET_1D
	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", pVdhRegCfg->VdhUvoffset);

	WR_VREG(VREG_HEAD_INF_OFFSET, pVdhRegCfg->VdhHeadInfOffset, 0);

	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );

	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);

	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);
	return VDMHAL_OK;
}

SINT32 VP8HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	VP8HAL_CfgReg(pVdhRegCfg);
	return VDMHAL_OK;
}

#endif
