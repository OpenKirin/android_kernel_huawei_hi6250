/*
 * vdec hal for h264
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
#include "vdm_hal_h264.h"
#include "vfmw_intf.h"

SINT32 H264HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	SINT32 D32;

	dprint(PRN_VDMREG, "\n***********************H264HAL_StartDec ***********************\n");
	D32 = 0;
	((BASIC_CFG0 *)(&D32))->mbamt_to_dec         = ((BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec;
	((BASIC_CFG0 *)(&D32))->load_qmatrix_flag    = 1;
	((BASIC_CFG0 *)(&D32))->marker_bit_detect_en = 0;
	((BASIC_CFG0 *)(&D32))->ac_last_detect_en    = 0;
	((BASIC_CFG0 *)(&D32))->coef_idx_detect_en   = 1;
	((BASIC_CFG0 *)(&D32))->vop_type_detect_en   = 0;
#ifdef ENV_SOS_KERNEL
	((BASIC_CFG0 *)(&D32))->sec_mode_en          = 1;
#else
	((BASIC_CFG0 *)(&D32))->sec_mode_en          = 0;
#endif
	WR_VREG(VREG_BASIC_CFG0, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG0 : 0x%x\n", D32);

	D32 = 0;
	((BASIC_CFG1 *)(&D32))->video_standard       = 0x0;
	//((BASIC_CFG1 *)(&D32))->ddr_stride           = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->ddr_stride;
	((BASIC_CFG1 *)(&D32))->fst_slc_grp          = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->fst_slc_grp;
	((BASIC_CFG1 *)(&D32))->mv_output_en         = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->mv_output_en;
	((BASIC_CFG1 *)(&D32))->uv_order_en          = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((BASIC_CFG1 *)(&D32))->vdh_2d_en            = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->vdh_2d_en;
	((BASIC_CFG1 *)(&D32))->max_slcgrp_num       = 2;
	((BASIC_CFG1 *)(&D32))->compress_en          = ((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->compress_en;
	((BASIC_CFG1 *)(&D32))->ppfd_en              = 0;
	((BASIC_CFG1 *)(&D32))->line_num_output_en   = 0;//((BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->line_num_output_en;
	WR_VREG(VREG_BASIC_CFG1, D32, 0);
	dprint(PRN_VDMREG, "BASIC_CFG1 : 0x%x\n", D32);

	D32 = 0;
	((AVM_ADDR *)(&D32))->av_msg_addr = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_AVM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "AVM_ADDR : 0x%x\n", D32);

	D32 = 0;
	((VAM_ADDR *)(&D32))->va_msg_addr = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_VAM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "VAM_ADDR : 0x%x\n", D32);

	D32 = 0;
	((STREAM_BASE_ADDR *)(&D32))->stream_base_addr = (pVdhRegCfg->VdhStreamBaseAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_STREAM_BASE_ADDR, D32, 0);
	dprint(PRN_VDMREG, "STREAM_BASE_ADDR : 0x%x\n", D32);

	D32 = RD_SCDREG(REG_EMAR_ID);
	if (pVdhRegCfg->VdhEmarId == 0) {
		D32 = D32 & (~(0x100));
	} else {
		D32 = D32 | 0x100;
	}
	WR_SCDREG(REG_EMAR_ID, D32);

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
	dprint(PRN_VDMREG, "YSTADDR_1D : 0x%x\n", D32);

	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);
	dprint(PRN_VDMREG, "YSTRIDE_1D : 0x%x\n", pVdhRegCfg->VdhYstride);

	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);
	dprint(PRN_VDMREG, "UVOFFSET_1D : 0x%x\n", pVdhRegCfg->VdhUvoffset);

	D32 = 0;
	WR_VREG(VREG_HEAD_INF_OFFSET, D32, 0);

	D32 = 0;
	((PPFD_BUF_ADDR *)(&D32))->ppfd_buf_addr = (pVdhRegCfg->VdhPpfdBufAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_PPFD_BUF_ADDR, D32, 0);
	dprint(PRN_VDMREG, "PPFD_BUF_ADDR : 0x%x\n", D32);

	WR_VREG(VREG_PPFD_BUF_LEN, pVdhRegCfg->VdhPpfdBufLen, 0);
	dprint(PRN_VDMREG, "PPFD_BUF_LEN : 0x%x\n", pVdhRegCfg->VdhPpfdBufLen);

	WR_VREG(VREG_REF_PIC_TYPE, pVdhRegCfg->VdhRefPicType, 0);
	dprint(PRN_VDMREG, "REF_PIC_TYPE : 0x%x\n", pVdhRegCfg->VdhRefPicType);

	if (pVdhRegCfg->VdhFfAptEn == 0x2) {
		D32 = 0x2;
	} else {
		D32 = 0x0;
	}
	WR_VREG(VREG_FF_APT_EN, D32, 0);
	dprint(PRN_VDMREG, "FF_APT_EN : 0x%x\n", D32);

	//UVSTRIDE_1D
	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );
	dprint(PRN_VDMREG, "UVSTRIDE_1D = 0x%x\n", pVdhRegCfg->VdhUvstride);

	//CFGINFO_ADDR
	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);
	dprint(PRN_VDMREG, "pPicParam->cfginfo_msg_addr:%x\n", pVdhRegCfg->VdhCfgInfoAddr);

	//DDR_INTERLEAVE_MODE
	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);

	return VDMHAL_OK;
}
