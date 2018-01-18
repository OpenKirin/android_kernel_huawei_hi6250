/*
 * vdec hal for hevc
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
#include "vdm_hal_hevc.h"
#include "vfmw_intf.h"

SINT32 HEVCHAL_CfgVdmReg(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	SINT32 D32;

	//BASIC_CFG0
	D32 = 0;
	((HEVC_BASIC_CFG0 *)(&D32))->marker_bit_detect_en = 0;
	((HEVC_BASIC_CFG0 *)(&D32))->ac_last_detect_en    = 0;
	((HEVC_BASIC_CFG0 *)(&D32))->coef_idx_detect_en   = 1; //(run_cnt>64) check enable switch
	((HEVC_BASIC_CFG0 *)(&D32))->vop_type_detect_en   = 0;
	((HEVC_BASIC_CFG0 *)(&D32))->load_qmatrix_flag    = ((HEVC_BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->load_qmatrix_flag;
	((HEVC_BASIC_CFG0 *)(&D32))->luma_sum_en          = 0; //enable switch:conculate luma pixel
	((HEVC_BASIC_CFG0 *)(&D32))->luma_histogram_en    = 0; //enable switch:conculate luma histogram
	((HEVC_BASIC_CFG0 *)(&D32))->mbamt_to_dec         = ((HEVC_BASIC_CFG0 *)(&pVdhRegCfg->VdhBasicCfg0))->mbamt_to_dec;
#ifdef ENV_SOS_KERNEL
	((HEVC_BASIC_CFG0*)(&D32))->vdh_safe_flag         = 1;
#else
	((HEVC_BASIC_CFG0*)(&D32))->vdh_safe_flag         = 0;
#endif
	WR_VREG( VREG_BASIC_CFG0, D32, 0 );

	//BASIC_CFG1
	/*set uv order 0: v first; 1: u first */
	D32 = 0;
	((HEVC_BASIC_CFG1 *)(&D32))->video_standard       = 0xD;
	((HEVC_BASIC_CFG1 *)(&D32))->fst_slc_grp          = ((HEVC_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->fst_slc_grp;
	((HEVC_BASIC_CFG1 *)(&D32))->mv_output_en         = 1;
	((HEVC_BASIC_CFG1 *)(&D32))->uv_order_en          = ((HEVC_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->uv_order_en;
	((HEVC_BASIC_CFG1 *)(&D32))->vdh_2d_en            = ((HEVC_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->vdh_2d_en;
	((HEVC_BASIC_CFG1 *)(&D32))->max_slcgrp_num       = 3;
	((HEVC_BASIC_CFG1 *)(&D32))->line_num_output_en   = 0; //enable switch:output "decodered pixel line of current frame" to DDR
	((HEVC_BASIC_CFG1 *)(&D32))->frm_cmp_en           = ((HEVC_BASIC_CFG1 *)(&pVdhRegCfg->VdhBasicCfg1))->frm_cmp_en;
	((HEVC_BASIC_CFG1 *)(&D32))->ppfd_en              = 0;
	WR_VREG( VREG_BASIC_CFG1, D32, 0 );

	//AVM_ADDR
	D32 = 0;
	((AVM_ADDR *)(&D32))->av_msg_addr = (pVdhRegCfg->VdhAvmAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_AVM_ADDR, D32, 0);
	dprint(PRN_VDMREG, "AVM_ADDR : 0x%x\n", D32);

	//VAM_ADDR
	D32 = 0;
	((VAM_ADDR *)(&D32))->va_msg_addr = (pVdhRegCfg->VdhVamAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_VAM_ADDR, D32, 0);

	//STREAM_BASE_ADDR
	D32 = 0;
	((STREAM_BASE_ADDR *)(&D32))->stream_base_addr = (pVdhRegCfg->VdhStreamBaseAddr) & 0xFFFFFFF0;
	WR_VREG(VREG_STREAM_BASE_ADDR, D32, 0);

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

	//YSTRIDE_1D
	WR_VREG(VREG_YSTRIDE_1D, pVdhRegCfg->VdhYstride, 0);

	//UVOFFSET_1D
	WR_VREG(VREG_UVOFFSET_1D, pVdhRegCfg->VdhUvoffset, 0);

	//HEAD_INF_OFFSET
	D32 = 0;
	WR_VREG(VREG_HEAD_INF_OFFSET, D32, 0);

	//PPFD_BUF_ADDR
	D32 = 0;
	((PPFD_BUF_ADDR *)(&D32))->ppfd_buf_addr = 0;
	WR_VREG(VREG_PPFD_BUF_ADDR, D32, 0);

	//PPFD_BUF_LEN
	D32 = 0;
	//((PPFD_BUF_LEN*)(&D32))->ppfd_buf_len = pHwMem->ppfd_buf_len;
	((PPFD_BUF_LEN *)(&D32))->ppfd_buf_len = 0;
	WR_VREG(VREG_PPFD_BUF_LEN, D32, 0);

	//UVSTRIDE_1D
	WR_VREG( VREG_UVSTRIDE_1D, pVdhRegCfg->VdhUvstride, 0 );

	//CFGINFO_ADDR
	WR_VREG(VREG_CFGINFO_ADDR, pVdhRegCfg->VdhCfgInfoAddr, 0);
	dprint(PRN_VDMREG, "pPicParam->cfginfo_msg_addr:%x\n", pVdhRegCfg->VdhCfgInfoAddr);

	//DDR_INTERLEAVE_MODE
	D32 = 0x03;
	WR_VREG(VREG_DDR_INTERLEAVE_MODE, D32, 0);

    //FF_APT_EN
	D32 = 0x2;
	WR_VREG(VREG_FF_APT_EN, D32, 0);
	dprint(PRN_VDMREG, "VREG_FF_APT_EN : 0x%x\n", D32);

	return VDMHAL_OK;
}

SINT32 HEVCHAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg)
{
	HEVCHAL_CfgVdmReg(pVdhRegCfg);
	return VDMHAL_OK;
}
