/*
 * vdec hal for vc1
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */

#ifndef __VDM_HAL_VC1_C__
#define __VDM_HAL_VC1_C__

#include "basedef.h"
#include "public.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"
#include "vdm_hal_vc1.h"

SINT32 VC1HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg);

SINT32 VC1HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	VC1HAL_CfgReg(pVdhRegCfg);
	return VDMHAL_OK;
}

SINT32 VC1HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	SINT32 D32;

	D32 = 0;
	D32 = (pVdhRegCfg->VdhBasicCfg0 & 0x000FFFFF)  // [19:0] mbamt_to_dec
		 | ( 0 << 22 )
		 | ( 0 << 23 )
		 | ( 1 << 24 )
		 | ( 0 << 25 )
		 | ( 1 << 30 ) 	// [30] ld_qmatrix_flag
#ifdef ENV_SOS_KERNEL
		 | ( 1 << 31 );	// [31] sec_mode, 1==Secure Mode
#else
		 | ( 0 << 31 );	// [31] sec_mode, 0==Normal Mode
#endif
	WR_VREG( VREG_BASIC_CFG0, D32, 0 );

	/*set uv order 0: v first; 1: u first */
	D32 = 0;
	((BASIC_CFG1*)(&D32))->video_standard     = 0x1;
	//((BASIC_CFG1*)(&D32))->ddr_stride         = ((BASIC_CFG1*)(&pVdhRegCfg->VdhBasicCfg1))->ddr_stride;
	((BASIC_CFG1*)(&D32))->uv_order_en        = ((BASIC_CFG1*)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((BASIC_CFG1*)(&D32))->fst_slc_grp        = 1;
	((BASIC_CFG1*)(&D32))->mv_output_en       = 1;
	((BASIC_CFG1*)(&D32))->max_slcgrp_num     = 1;
	((BASIC_CFG1*)(&D32))->line_num_output_en = 0;
	((BASIC_CFG1*)(&D32))->vdh_2d_en          = ((BASIC_CFG1*)(&pVdhRegCfg->VdhBasicCfg1))->vdh_2d_en;
	((BASIC_CFG1*)(&D32))->compress_en        = ((BASIC_CFG1*)(&pVdhRegCfg->VdhBasicCfg1))->compress_en;
	((BASIC_CFG1*)(&D32))->ppfd_en            = 0;
	WR_VREG( VREG_BASIC_CFG1, D32, 0 );

	WR_VREG(VREG_AVM_ADDR, pVdhRegCfg->VdhAvmAddr, 0);
	WR_VREG(VREG_VAM_ADDR, pVdhRegCfg->VdhVamAddr, 0);
	WR_VREG(VREG_STREAM_BASE_ADDR, pVdhRegCfg->VdhStreamBaseAddr, 0);


	//TIME_OUT
	D32 = 0x00300C03;
	WR_VREG(VREG_SED_TO, D32, 0);
	WR_VREG(VREG_ITRANS_TO, D32, 0);
	WR_VREG(VREG_PMV_TO, D32, 0);
	WR_VREG(VREG_PRC_TO, D32, 0);
	WR_VREG(VREG_RCN_TO, D32, 0);
	WR_VREG(VREG_DBLK_TO, D32, 0);
	WR_VREG(VREG_PPFD_TO, D32, 0);

	WR_VREG(VREG_YSTADDR_1D, pVdhRegCfg->VdhYstAddr, 0);
	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);
	WR_VREG(VREG_FF_APT_EN, pVdhRegCfg->VdhFfAptEn, 0);
	WR_VREG(VREG_REF_PIC_TYPE, pVdhRegCfg->VdhRefPicType, 0);

	WR_VREG(VREG_HEAD_INF_OFFSET, pVdhRegCfg->VdhHeadInfOffset, 0);

	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );

	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);

	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);
	return VDMHAL_OK;
}

#endif
