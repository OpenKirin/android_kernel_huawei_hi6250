/*
 * vdec hal for mp2
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
#ifndef __VDM_HAL_MPEG2_C__
#define __VDM_HAL_MPEG2_C__

#include    "public.h"
#include    "vdm_hal_api.h"
#include    "vdm_hal_local.h"
#include    "vdm_hal_mpeg2.h"

SINT32 MP2HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg);

SINT32 MP2HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	if (MP2HAL_CfgReg(pVdhRegCfg) != VDMHAL_OK) {
		dprint(PRN_ERROR, "MP2HAL_CfgReg ERROR\n");
		return VDMHAL_ERR;
	}
	return VDMHAL_OK;
}

SINT32 MP2HAL_CfgReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	SINT32 D32;
	D32 = 0;
	((BASIC_CFG0 *)(&D32))->mbamt_to_dec         = ((BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec;
	((BASIC_CFG0 *)(&D32))->load_qmatrix_flag    = 1;
	((BASIC_CFG0 *)(&D32))->marker_bit_detect_en = 1;
	((BASIC_CFG0 *)(&D32))->ac_last_detect_en    = 0;
	((BASIC_CFG0 *)(&D32))->coef_idx_detect_en   = 1;
	((BASIC_CFG0 *)(&D32))->vop_type_detect_en   = 0;
#ifdef ENV_SOS_KERNEL
	((BASIC_CFG0 *)(&D32))->sec_mode_en          = 1;
#else
	((BASIC_CFG0 *)(&D32))->sec_mode_en          = 0;
#endif
	WR_VREG( VREG_BASIC_CFG0, D32, 0  );

	D32 = 0;
	((BASIC_CFG1 *)(&D32))->video_standard       = 0x3;
	//((BASIC_CFG1 *)(&D32))->ddr_stride           = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->ddr_stride;
	((BASIC_CFG1 *)(&D32))->fst_slc_grp          = 1;
	((BASIC_CFG1 *)(&D32))->mv_output_en         = 1;
	((BASIC_CFG1 *)(&D32))->uv_order_en          = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((BASIC_CFG1 *)(&D32))->vdh_2d_en            = 1;
	((BASIC_CFG1 *)(&D32))->max_slcgrp_num       = 3;
	((BASIC_CFG1 *)(&D32))->ppfd_en              = 0;
	((BASIC_CFG1 *)(&D32))->line_num_output_en   = 0;
	((BASIC_CFG1 *)(&D32))->compress_en          = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->compress_en;
	/*set uv order 0: v first; 1: u first */
	WR_VREG( VREG_BASIC_CFG1, D32, 0 );

	D32 = 0;
	((AVM_ADDR *)(&D32))->av_msg_addr = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_AVM_ADDR, D32, 0);

	D32 = 0;
	((VAM_ADDR *)(&D32))->va_msg_addr = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_VAM_ADDR, D32, 0);

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

	D32 = 0;
	((YSTADDR_1D *)(&D32))->ystaddr_1d = (pVdhRegCfg->VdhYstAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_YSTADDR_1D, D32, 0);
	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);

	D32 = 0;
	((REF_PIC_TYPE *)(&D32))->ref_pic_type_0 = ((REF_PIC_TYPE *)(&pVdhRegCfg->VdhRefPicType))->ref_pic_type_0;
	((REF_PIC_TYPE *)(&D32))->ref_pic_type_1 = ((REF_PIC_TYPE *)(&pVdhRegCfg->VdhRefPicType))->ref_pic_type_1;
	WR_VREG( VREG_REF_PIC_TYPE, D32, 0 );
	D32 = 0;
	((FF_APT_EN *)(&D32))->ff_apt_en = 0;//USE_FF_APT_EN;
	WR_VREG( VREG_FF_APT_EN, D32, 0 );

	//HEAD_INF_OFFSET
	WR_VREG(VREG_HEAD_INF_OFFSET, pVdhRegCfg->VdhHeadInfOffset, 0);
	dprint(PRN_VDMREG, "HEAD_INF_OFFSET : 0x%x\n", pVdhRegCfg->VdhHeadInfOffset);

	//VREG_UVSTRIDE_1D
	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );
	//CFGINFO_ADDR
	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);
	dprint(PRN_VDMREG, "pPicParam->cfginfo_msg_addr:%x\n", pVdhRegCfg->VdhCfgInfoAddr);

	//DDR_INTERLEAVE_MODE
	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);

	return VDMHAL_OK;
}

#endif
