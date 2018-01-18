#ifndef __VDM_HAL_REAL9_HEADER__
#define __VDM_HAL_REAL9_HEADER__

#include "basedef.h"

typedef struct
{
	USIGN mbamt_to_dec:                         20;
	USIGN memory_clock_gating_en:               1;
	USIGN module_clock_gating_en:               1;
	USIGN marker_bit_detect_en:                 1;
	USIGN ac_last_detect_en:                    1;
	USIGN coef_idx_detect_en:                   1;
	USIGN vop_type_detect_en:                   1;
	USIGN reserved:                             4;
	USIGN ld_qmatrix_flag:                      1;
	USIGN sec_mode_en:                          1;
} RV9_BASIC_CFG0;
typedef struct
{
	USIGN video_standard:                       4;
	USIGN ddr_stride:                           9;
	USIGN uv_order_en:                          1;
	USIGN fst_slc_grp:                          1;
	USIGN mv_output_en:                         1;
	USIGN max_slcgrp_num:                       12;
	USIGN line_num_output_en:                   1;
	USIGN vdh_2d_en:                            1;
	USIGN compress_en:                          1;
	USIGN ppfd_en:                              1;
} RV9_BASIC_CFG1;
typedef struct
{
	USIGN av_msg_addr:                          32;
} RV9_AVM_ADDR;
typedef struct
{
	USIGN va_msg_addr:                          32;
} RV9_VAM_ADDR;
typedef struct
{
	USIGN ff_apt_en:                            1;
	USIGN reserved:                             31;
} RV9_FF_APT_EN;
SINT32 RV9HAL_StartDec(OMXVDH_REG_CFG_S *pVdhRegCfg);
#endif
