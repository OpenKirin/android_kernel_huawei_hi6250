/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/
#include "hisi_fb.h"
#include "hisi_dp.h"
#include "hisi_fb_def.h"

#include "avgen.h"
#include "dp_aux.h"
#include "link.h"
#include "intr.h"
#include "core.h"
#include "../hdcp22/host_lib_driver_linux_if.h"


/*lint -save -e* */
#define EDID_NUM 256
uint32_t g_bit_hpd_status;

static int handle_test_link_training(struct dp_ctrl *dptx)
{
	int retval;
	u8 lanes;
	u8 rate;
	struct video_params *vparams;
	struct dtd *mdtd;

	retval = dptx_read_dpcd(dptx, DP_TEST_LINK_RATE, &rate);
	if (retval)
		return retval;

	retval = dptx_bw_to_phy_rate(rate);
	if (retval < 0)
		return retval;

	rate = retval;

	retval = dptx_read_dpcd(dptx, DP_TEST_LANE_COUNT, &lanes);
	if (retval)
		return retval;

	HISI_FB_INFO("%s: Strating link training rate=%d, lanes=%d\n",
		 __func__, rate, lanes);

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	retval = dptx_video_ts_calculate(dptx, lanes, rate,
					 vparams->bpc, vparams->pix_enc,
					 mdtd->pixel_clock);
	if (retval)
		return retval;

	retval = dptx_link_training(dptx,
				    rate,
				    lanes);
	if (retval)
		HISI_FB_ERR("Link training failed %d\n", retval);
	else
		HISI_FB_INFO("Link training succeeded\n");

	return retval;
}

static int handle_test_link_video_timming(struct dp_ctrl *dptx)
{
	int retval, i;
	u8 test_h_total_lsb, test_h_total_msb, test_v_total_lsb,
	   test_v_total_msb, test_h_start_lsb, test_h_start_msb,
	   test_v_start_lsb, test_v_start_msb, test_hsync_width_lsb,
	   test_hsync_width_msb, test_vsync_width_lsb, test_vsync_width_msb,
	   test_h_width_lsb, test_h_width_msb, test_v_width_lsb,
	   test_v_width_msb;
	u32 h_total, v_total, h_start, v_start, h_width, v_width,
	    hsync_width, vsync_width, h_sync_pol, v_sync_pol, refresh_rate;
	enum video_format_type video_format;
	u8 vmode;
	u8 test_refresh_rate;
	struct video_params *vparams;
	struct dtd mdtd;

	vparams = &dptx->vparams;
	retval = 0;
	h_total = 0;
	v_total = 0;
	h_start = 0;
	v_start = 0;
	v_width = 0;
	h_width = 0;
	hsync_width = 0;
	vsync_width = 0;
	h_sync_pol = 0;
	v_sync_pol = 0;
	test_refresh_rate = 0;
	i = 0;

	/* H_TOTAL */
	retval = dptx_read_dpcd(dptx, DP_TEST_H_TOTAL_LSB, &test_h_total_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_H_TOTAL_MSB, &test_h_total_msb);
	if (retval)
		return retval;
	h_total |= test_h_total_lsb;
	h_total |= test_h_total_msb << 8;
	HISI_FB_INFO("h_total = %d\n", h_total);

	/* V_TOTAL */
	retval = dptx_read_dpcd(dptx, DP_TEST_V_TOTAL_LSB, &test_v_total_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_V_TOTAL_MSB, &test_v_total_msb);
	if (retval)
		return retval;
	v_total |= test_v_total_lsb;
	v_total |= test_v_total_msb << 8;
	HISI_FB_INFO("v_total = %d\n", v_total);

	/*  H_START */
	retval = dptx_read_dpcd(dptx, DP_TEST_H_START_LSB, &test_h_start_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_H_START_MSB, &test_h_start_msb);
	if (retval)
		return retval;
	h_start |= test_h_start_lsb;
	h_start |= test_h_start_msb << 8;
	HISI_FB_INFO("h_start = %d\n", h_start);

	/* V_START */
	retval = dptx_read_dpcd(dptx, DP_TEST_V_START_LSB, &test_v_start_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_V_START_MSB, &test_v_start_msb);
	if (retval)
		return retval;
	v_start |= test_v_start_lsb;
	v_start |= test_v_start_msb << 8;
	HISI_FB_INFO("v_start = %d\n", v_start);

	/* TEST_HSYNC */
	retval = dptx_read_dpcd(dptx, DP_TEST_H_SYNC_WIDTH_LSB,
				&test_hsync_width_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_H_SYNC_WIDTH_MSB,
				&test_hsync_width_msb);
	if (retval)
		return retval;
	hsync_width |= test_hsync_width_lsb;
	hsync_width |= (test_hsync_width_msb & (~(1 << 7))) << 8;
	h_sync_pol |= (test_hsync_width_msb & (1 << 7)) >> 8;
	HISI_FB_INFO("hsync_width = %d\n", hsync_width);
	HISI_FB_INFO("h_sync_pol = %d\n", h_sync_pol);

	/* TEST_VSYNC */
	retval = dptx_read_dpcd(dptx, DP_TEST_V_SYNC_WIDTH_LSB,
				&test_vsync_width_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_V_SYNC_WIDTH_MSB,
				&test_vsync_width_msb);
	if (retval)
		return retval;
	vsync_width |= test_vsync_width_lsb;
	vsync_width |= (test_vsync_width_msb & (~(1 << 7))) << 8;
	v_sync_pol |= (test_vsync_width_msb & (1 << 7)) >> 8;
	HISI_FB_INFO("vsync_width = %d\n", vsync_width);
	HISI_FB_INFO("v_sync_pol = %d\n", v_sync_pol);

	/* TEST_H_WIDTH */
	retval = dptx_read_dpcd(dptx, DP_TEST_H_WIDTH_LSB, &test_h_width_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_H_WIDTH_MSB, &test_h_width_msb);
	if (retval)
		return retval;
	h_width |= test_h_width_lsb;
	h_width |= test_h_width_msb << 8;
	HISI_FB_INFO("h_width = %d\n", h_width);

	/* TEST_V_WIDTH */
	retval = dptx_read_dpcd(dptx, DP_TEST_V_WIDTH_LSB, &test_v_width_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_V_WIDTH_MSB, &test_v_width_msb);
	if (retval)
		return retval;
	v_width |= test_v_width_lsb;
	v_width |= test_v_width_msb << 8;
	HISI_FB_INFO("v_width = %d\n", v_width);

	retval = dptx_read_dpcd(dptx, 0x234, &test_refresh_rate);
	if (retval)
		return retval;
	HISI_FB_INFO("test_refresh_rate = %d\n", test_refresh_rate);

	video_format = DMT;
	refresh_rate =  test_refresh_rate * 1000;

	if (h_total == 1056 && v_total == 628 && h_start == 216 &&
	    v_start == 27 && hsync_width == 128 && vsync_width == 4 &&
	    h_width == 800 && v_width == 600) {
		vmode = 9;
	} else if (h_total == 1088 && v_total == 517 && h_start == 224 &&
		   v_start == 31 && hsync_width == 112 && vsync_width == 8 &&
		   h_width == 848 && v_width == 480) {
		vmode = 14;
	} else if (h_total == 1344 && v_total == 806 && h_start == 296 &&
		   v_start == 35 && hsync_width == 136 && vsync_width == 6 &&
		   h_width == 1024 && v_width == 768) {
		vmode = 16;
	} else if (h_total == 1440 && v_total == 790 && h_start == 112 &&
		   v_start == 19 && hsync_width == 32 && vsync_width == 7 &&
		   h_width == 1280 && v_width == 768) {
		vmode = 22;
	} else if (h_total == 1664 && v_total == 798 && h_start == 320 &&
		   v_start == 27 && hsync_width == 128 && vsync_width == 7 &&
		   h_width == 1280 && v_width == 768) {
		vmode = 23;
	} else if (h_total == 1440 && v_total == 823 && h_start == 112 &&
		   v_start == 20 && hsync_width == 32 && vsync_width == 6 &&
		   h_width == 1280 && v_width == 800) {
		vmode = 27;
	} else if (h_total == 1800 && v_total == 1000 && h_start == 424 &&
		   v_start == 39 && hsync_width == 112 && vsync_width == 3 &&
		   h_width == 1280 && v_width == 960) {
		vmode = 32;
	} else if (h_total == 1688 && v_total == 1066 && h_start == 360 &&
		   v_start == 41 && hsync_width == 112 && vsync_width == 3 &&
		   h_width == 1280 && v_width  == 1024) {
		vmode = 35;
	} else if (h_total == 1792 && v_total == 795 && h_start == 368 &&
		   v_start == 24 && hsync_width == 112 && vsync_width == 6 &&
		   h_width == 1360 && v_width == 768) {
		vmode = 39;
	} else if (h_total == 1560 && v_total == 1080  && h_start == 112  &&
		   v_start == 27 && hsync_width == 32 && vsync_width == 4 &&
		   h_width == 1400 && v_width == 1050) {
		vmode = 41;
	} else if (h_total == 2160 && v_total == 1250 && h_start == 496 &&
		   v_start == 49 && hsync_width == 192 && vsync_width == 3 &&
		   h_width == 1600 && v_width == 1200) {
		vmode = 51;
	} else if (h_total == 2448 && v_total == 1394 && h_start == 528 &&
		   v_start == 49 && hsync_width == 200 && vsync_width == 3 &&
		   h_width == 1792 && v_width == 1344) {
		vmode = 62;
	} else if (h_total == 2600 && v_total == 1500 && h_start == 552 &&
		   v_start == 59  && hsync_width == 208 && vsync_width == 3 &&
		   h_width == 1920 && v_width == 1440) {
		vmode = 73;
	} else if (h_total == 2200 && v_total == 1125 && h_start == 192 &&
		   v_start == 41 && hsync_width == 44 && vsync_width == 5 &&
		   h_width == 1920 && v_width == 1080) {
		if (refresh_rate == 120000) {
			vmode = 63;
			video_format = VCEA;
		} else {
			vmode = 82;
		}
	} else if (h_total == 800 && v_total == 525 && h_start == 144 &&
		   v_start == 35 && hsync_width == 96 && vsync_width == 2 &&
		   h_width == 640 && v_width == 480) {
		vmode = 1;
		video_format = VCEA;
	} else if (h_total == 1650 && v_total == 750 && h_start == 260 &&
		   v_start == 25 && hsync_width == 40 && vsync_width == 5 &&
		   h_width == 1280  && v_width == 720) {
		vmode = 4;
		video_format = VCEA;
	} else if (h_total == 1680 && v_total == 831 && h_start == 328 &&
		   v_start == 28 && hsync_width == 128 && vsync_width == 6 &&
		   h_width == 1280 && v_width == 800) {
		vmode = 28;
		video_format = CVT;
	} else if (h_total == 1760 && v_total == 1235 && h_start == 112 &&
		   v_start == 32 && hsync_width == 32 && vsync_width == 4 &&
		   h_width == 1600 && v_width == 1200) {
		vmode = 40;
		video_format = CVT;
	} else if (h_total == 2208 && v_total == 1580 && h_start == 112 &&
		   v_start == 41 && hsync_width == 32 &&  vsync_width == 4 &&
		   h_width == 2048 && v_width == 1536) {
		vmode = 41;
		video_format = CVT;
	} else {
		HISI_FB_INFO("Unknown video mode\n");
		return -EINVAL;
	}

	if (!dptx_dtd_fill(&mdtd, vmode, refresh_rate, video_format)) {
		HISI_FB_INFO("%s: Invalid video mode value %d\n",
			 __func__, vmode);
		retval = -EINVAL;
		goto fail;
	}
	vparams->mdtd = mdtd;
	vparams->refresh_rate = refresh_rate;
	retval = dptx_video_ts_calculate(dptx, dptx->link.lanes,
					 dptx->link.rate, vparams->bpc,
					 vparams->pix_enc, mdtd.pixel_clock);
	if (retval)
		return retval;
	/* MMCM */
#ifdef CONFIG_DP_GENERATOR_REF
	dptx_video_reset(dptx, 1);
	retval = dptx_video_pixel_freq_change(dptx, mdtd.pixel_clock);
	if (retval) {
		dptx_video_reset(dptx, 0);
		goto fail;
	}
	dptx_video_reset(dptx, 0);

	vparams->mode = vmode;
	vparams->video_format = video_format;
	dptx_video_timing_change(dptx);
#endif
fail:
	return retval;

}

static int handle_test_link_audio_pattern(struct dp_ctrl *dptx)
{
	int retval, freq_id;
	u8 test_audio_mode, test_audio_smaple_range, test_audio_ch_count,
	   audio_ch_count, orig_sample_freq, sample_freq;
	struct audio_params *aparams;

	aparams = &dptx->aparams;
	retval = dptx_read_dpcd(dptx, DP_TEST_AUDIO_MODE, &test_audio_mode);
	if (retval)
		return retval;

	HISI_FB_INFO("test_audio_mode= %d\n", test_audio_mode);

	test_audio_smaple_range = test_audio_mode &
		DP_TEST_AUDIO_SAMPLING_RATE_MASK;
	test_audio_ch_count = (test_audio_mode & DP_TEST_AUDIO_CH_COUNT_MASK)
		>> DP_TEST_AUDIO_CH_COUNT_SHIFT;

	switch (test_audio_ch_count) {
	case DP_TEST_AUDIO_CHANNEL1:
		HISI_FB_INFO("DP_TEST_AUDIO_CHANNEL1\n");
		audio_ch_count = 1;
		break;
	case DP_TEST_AUDIO_CHANNEL2:
		HISI_FB_INFO("DP_TEST_AUDIO_CHANNEL2\n");
		audio_ch_count = 2;
		break;
	case DP_TEST_AUDIO_CHANNEL3:
		HISI_FB_INFO("DP_TEST_AUDIO_CHANNEL3\n");
		audio_ch_count = 3;
		break;
	case DP_TEST_AUDIO_CHANNEL4:
		HISI_FB_INFO("DP_TEST_AUDIO_CHANNEL4\n");
		audio_ch_count = 4;
		break;
	case DP_TEST_AUDIO_CHANNEL5:
		HISI_FB_INFO("DP_TEST_AUDIO_CHANNEL5\n");
		audio_ch_count = 5;
		break;
	case DP_TEST_AUDIO_CHANNEL6:
		HISI_FB_INFO("DP_TEST_AUDIO_CHANNEL6\n");
		audio_ch_count = 6;
		break;
	case DP_TEST_AUDIO_CHANNEL7:
		HISI_FB_INFO("DP_TEST_AUDIO_CHANNEL7\n");
		audio_ch_count = 7;
		break;
	case DP_TEST_AUDIO_CHANNEL8:
		HISI_FB_INFO("DP_TEST_AUDIO_CHANNEL8\n");
		audio_ch_count = 8;
		break;
	default:
		HISI_FB_INFO("Invalid TEST_AUDIO_CHANNEL_COUNT\n");
		return -EINVAL;
	}
	HISI_FB_INFO("test_audio_ch_count = %d\n", audio_ch_count);
	aparams->num_channels = audio_ch_count;

	switch (test_audio_smaple_range) {
	case DP_TEST_AUDIO_SAMPLING_RATE_32:
		HISI_FB_INFO("DP_TEST_AUDIO_SAMPLING_RATE_32\n");
		orig_sample_freq = 12;
		sample_freq = 3;
		freq_id = 0;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_44_1:
		HISI_FB_INFO("DP_TEST_AUDIO_SAMPLING_RATE_44_1\n");
		orig_sample_freq = 15;
		sample_freq = 0;
		freq_id = 1;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_48:
		HISI_FB_INFO("DP_TEST_AUDIO_SAMPLING_RATE_48\n");
		orig_sample_freq = 13;
		sample_freq = 2;
		freq_id = 2;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_88_2:
		HISI_FB_INFO("DP_TEST_AUDIO_SAMPLING_RATE_88_2\n");
		orig_sample_freq = 7;
		sample_freq = 8;
		freq_id = 3;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_96:
		HISI_FB_INFO("DP_TEST_AUDIO_SAMPLING_RATE_96\n");
		orig_sample_freq = 5;
		sample_freq = 10;
		freq_id = 4;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_176_4:
		HISI_FB_INFO("DP_TEST_AUDIO_SAMPLING_RATE_176_4\n");
		orig_sample_freq = 3;
		sample_freq = 12;
		freq_id = 5;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_192:
		HISI_FB_INFO("DP_TEST_AUDIO_SAMPLING_RATE_192\n");
		orig_sample_freq = 1;
		sample_freq = 14;
		freq_id = 6;
		break;
	default:
		HISI_FB_INFO("Invalid TEST_AUDIO_SAMPLING_RATE\n");
		return -EINVAL;
	}
	HISI_FB_INFO("sample_freq = %d\n", sample_freq);
	HISI_FB_INFO("orig_sample_freq = %d\n", orig_sample_freq);
	aparams->iec_samp_freq = sample_freq;
	aparams->iec_orig_samp_freq = orig_sample_freq;

	dptx_audio_num_ch_change(dptx);
#ifdef CONFIG_DP_GENERATOR_REF
	dptx_audio_samp_freq_config(dptx);
	//dptx_audio_clock_freq_change(dptx, freq_id);
#endif
	dptx_audio_infoframe_sdp_send(dptx);

	return retval;
}

static int handle_test_link_video_pattern(struct dp_ctrl *dptx)
{
	int retval;
	u8 misc, pattern, bpc, bpc_map, dynamic_range,
	   dynamic_range_map, color_format, color_format_map,
	   ycbcr_coeff,  ycbcr_coeff_map;
	struct video_params *vparams;
	struct dtd *mdtd;

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;
	retval = 0;

	retval = dptx_read_dpcd(dptx, DP_TEST_PATTERN, &pattern);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_MISC, &misc);
	if (retval)
		return retval;

	dynamic_range = (misc & DP_TEST_DYNAMIC_RANGE_MASK)
			>> DP_TEST_DYNAMIC_RANGE_SHIFT;
	switch (dynamic_range) {
	case DP_TEST_DYNAMIC_RANGE_VESA:
		HISI_FB_INFO("DP_TEST_DYNAMIC_RANGE_VESA\n");
		dynamic_range_map = VESA;
		break;
	case DP_TEST_DYNAMIC_RANGE_CEA:
		HISI_FB_INFO("DP_TEST_DYNAMIC_RANGE_CEA\n");
		dynamic_range_map = CEA;
		break;
	default:
		HISI_FB_INFO("Invalid TEST_BIT_DEPTH\n");
		return -EINVAL;
	}

	ycbcr_coeff = (misc & DP_TEST_YCBCR_COEFF_MASK)
			>> DP_TEST_YCBCR_COEFF_SHIFT;

	switch (ycbcr_coeff) {
	case DP_TEST_YCBCR_COEFF_ITU601:
		HISI_FB_INFO("DP_TEST_YCBCR_COEFF_ITU601\n");
		ycbcr_coeff_map = ITU601;
		break;
	case DP_TEST_YCBCR_COEFF_ITU709:
		HISI_FB_INFO("DP_TEST_YCBCR_COEFF_ITU709:\n");
		ycbcr_coeff_map = ITU709;
		break;
	default:
		HISI_FB_INFO("Invalid TEST_BIT_DEPTH\n");
		return -EINVAL;
	}
	color_format = misc & DP_TEST_COLOR_FORMAT_MASK;

	switch (color_format) {
	case DP_TEST_COLOR_FORMAT_RGB:
		HISI_FB_INFO("DP_TEST_COLOR_FORMAT_RGB\n");
		color_format_map = RGB;
		break;
	case DP_TEST_COLOR_FORMAT_YCBCR422:
		HISI_FB_INFO("DP_TEST_COLOR_FORMAT_YCBCR422\n");
		color_format_map = YCBCR422;
		break;
	case DP_TEST_COLOR_FORMAT_YCBCR444:
		HISI_FB_INFO("DP_TEST_COLOR_FORMAT_YCBCR444\n");
		color_format_map = YCBCR444;
		break;
	default:
		HISI_FB_INFO("Invalid  DP_TEST_COLOR_FORMAT\n");
		return -EINVAL;
	}

	bpc = (misc & DP_TEST_BIT_DEPTH_MASK)
		>> DP_TEST_BIT_DEPTH_SHIFT;

	switch (bpc) {
	case DP_TEST_BIT_DEPTH_6:
		bpc_map = COLOR_DEPTH_6;
		HISI_FB_INFO("TEST_BIT_DEPTH_6\n");
		break;
	case DP_TEST_BIT_DEPTH_8:
		bpc_map = COLOR_DEPTH_8;
		HISI_FB_INFO("TEST_BIT_DEPTH_8\n");
		break;
	case DP_TEST_BIT_DEPTH_10:
		bpc_map = COLOR_DEPTH_10;
		HISI_FB_INFO("TEST_BIT_DEPTH_10\n");
		break;
	case DP_TEST_BIT_DEPTH_12:
		bpc_map = COLOR_DEPTH_12;
		HISI_FB_INFO("TEST_BIT_DEPTH_12\n");
		break;
	case DP_TEST_BIT_DEPTH_16:
		bpc_map = COLOR_DEPTH_16;
		HISI_FB_INFO("TEST_BIT_DEPTH_16\n");
		break;
	default:
		HISI_FB_INFO("Invalid TEST_BIT_DEPTH\n");
		return -EINVAL;
	}

	vparams->dynamic_range = dynamic_range_map;
	HISI_FB_INFO("Change video dynamic range to %d\n", dynamic_range_map);

	vparams->colorimetry = ycbcr_coeff_map;
	HISI_FB_INFO("Change video colorimetry to %d\n", ycbcr_coeff_map);

	retval = dptx_video_ts_calculate(dptx, dptx->link.lanes,
					 dptx->link.rate,
					 bpc_map, color_format_map,
					 mdtd->pixel_clock);
	if (retval)
		return retval;

	vparams->pix_enc = color_format_map;
	HISI_FB_INFO("Change pixel encoding to %d\n", color_format_map);

	vparams->bpc = bpc_map;
	dptx_video_bpc_change(dptx);
	HISI_FB_INFO("Change bits per component to %d\n", bpc_map);
#ifdef CONFIG_DP_GENERATOR_REF
	dptx_video_ycc_mapping_change(dptx);
#endif
	dptx_video_ts_change(dptx);

	switch (pattern) {
	case DP_TEST_PATTERN_NONE:
		HISI_FB_INFO("TEST_PATTERN_NONE %d\n", pattern);
		break;
	case DP_TEST_PATTERN_COLOR_RAMPS:
		HISI_FB_INFO("TEST_PATTERN_COLOR_RAMPS %d\n", pattern);
		vparams->pattern_mode = RAMP;
		#ifdef CONFIG_DP_GENERATOR_REF
			dptx_video_pattern_set(dptx, RAMP);
			dptx_video_pattern_change(dptx);
		#endif
		HISI_FB_INFO("Change video pattern to RAMP\n");
		break;
	case DP_TEST_PATTERN_BW_VERITCAL_LINES:
		HISI_FB_INFO("TEST_PATTERN_BW_VERTICAL_LINES %d\n", pattern);
		break;
	case DP_TEST_PATTERN_COLOR_SQUARE:
		HISI_FB_INFO("TEST_PATTERN_COLOR_SQUARE %d\n", pattern);
		vparams->pattern_mode = COLRAMP;
		#ifdef CONFIG_DP_GENERATOR_REF
			dptx_video_pattern_set(dptx, COLRAMP);
			dptx_video_pattern_change(dptx);
		#endif
		HISI_FB_INFO("Change video pattern to COLRAMP\n");
		break;
	default:
		HISI_FB_INFO("Invalid TEST_PATTERN %d\n", pattern);
		return -EINVAL;
	}

	retval = handle_test_link_video_timming(dptx);
	if (retval)
		return retval;

	return 0;
}

static int handle_test_link_phy_pattern(struct dp_ctrl *dptx)
{
	int retval;
	u8 pattern;
	retval = 0;
	pattern = 0;

	retval = dptx_read_dpcd(dptx, DP_TEST_PHY_PATTERN, &pattern);
	if (retval)
		return retval;

	pattern &= DPTX_PHY_TEST_PATTERN_SEL_MASK;

	switch (pattern) {
	case DPTX_NO_TEST_PATTERN_SEL:
		HISI_FB_INFO("DPTX_D102_WITHOUT_SCRAMBLING\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_NONE);
		break;
	case DPTX_D102_WITHOUT_SCRAMBLING:
		HISI_FB_INFO("DPTX_D102_WITHOUT_SCRAMBLING\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_1);
		break;
	case DPTX_SYMBOL_ERROR_MEASUREMENT_COUNT:
		HISI_FB_INFO("DPTX_SYMBOL_ERROR_MEASUREMENT_COUNT\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_SYM_ERM);
		break;
	case DPTX_TEST_PATTERN_PRBS7:
		HISI_FB_INFO("DPTX_TEST_PATTERN_PRBS7\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_PRBS7);
		break;
	case DPTX_80BIT_CUSTOM_PATTERN_TRANS:
		HISI_FB_INFO("DPTX_80BIT_CUSTOM_PATTERN_TRANS\n");
		dptx_writel(dptx, DPTX_CUSTOMPAT0, 0x3e0f83e0);
		dptx_writel(dptx, DPTX_CUSTOMPAT1, 0x3e0f83e0);
		dptx_writel(dptx, DPTX_CUSTOMPAT2, 0xf83e0);
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_CUSTOM80);
		break;
	case DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN:
		HISI_FB_INFO("DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_CP2520_1);
		break;
	case DPTX_CP2520_HBR2_COMPLIANCE_PATTERN_2:
		HISI_FB_INFO("DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_CP2520_2);
		break;
	case DPTX_CP2520_HBR2_COMPLIANCE_PATTERN_3:
		HISI_FB_INFO("DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_4);
		break;
	default:
		HISI_FB_INFO("Invalid TEST_PATTERN\n");
		return -EINVAL;
	}

	return 0;
}

static int handle_automated_test_request(struct dp_ctrl *dptx)
{
	int retval;
	u8 test;

	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return -1;
	}

	dptx_triger_video_transfer(dptx, false);

	retval = dptx_read_dpcd(dptx, DP_TEST_REQUEST, &test);
	if (retval)
		return retval;

	if (test & DP_TEST_LINK_TRAINING) {
		HISI_FB_INFO("%s: DP_TEST_LINK_TRAINING\n", __func__);

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		retval = handle_test_link_training(dptx);

		if (retval) {
			return retval;
		} else {
			dptx_triger_video_transfer(dptx, true);
		}
	}

	if (test & DP_TEST_LINK_VIDEO_PATTERN) {
		HISI_FB_INFO("%s:DP_TEST_LINK_VIDEO_PATTERN\n", __func__);

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		retval = handle_test_link_video_pattern(dptx);

		if (retval) {
			return retval;
		} else {
			dptx_triger_video_transfer(dptx, true);
		}
	}

	if (test & DP_TEST_LINK_AUDIO_PATTERN) {
		HISI_FB_INFO("%s:DP_TEST_LINK_AUDIO_PATTERN\n", __func__);

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		retval = handle_test_link_audio_pattern(dptx);
		if (retval)
			return retval;
	}

	if (test & DP_TEST_LINK_EDID_READ) {
		/* Invalid, this should happen on HOTPLUG */
		HISI_FB_INFO("%s:DP_TEST_LINK_EDID_READ\n", __func__);
		return -ENOTSUPP;
	}

	if (test & DP_TEST_LINK_PHY_TEST_PATTERN) {
		/* TODO */
		HISI_FB_INFO("%s:DP_TEST_LINK_PHY_TEST_PATTERN\n", __func__);

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		dptx_link_set_ssc(dptx, true);

		dptx_link_adjust_drive_settings(dptx, NULL);   //Test only

		retval = handle_test_link_phy_pattern(dptx);
		if (retval)
			return retval;
	}

	return 0;
}

static int handle_sink_request(struct dp_ctrl *dptx)
{
	int retval;
	u8 vector;
#ifdef CONFIG_DP_HDCP_ENABLE
	u32 reg;
#endif
	retval = dptx_link_check_status(dptx);
	if (retval)
		return retval;

	retval = dptx_read_dpcd(dptx, DP_DEVICE_SERVICE_IRQ_VECTOR, &vector);
	if (retval)
		return retval;

	HISI_FB_INFO("%s: IRQ_VECTOR: 0x%02x\n", __func__, vector);

	/* TODO handle sink interrupts */
	if (!vector)
		return 0;

	if (vector & DP_REMOTE_CONTROL_COMMAND_PENDING) {
		/* TODO */
		HISI_FB_WARNING(
			  "%s: DP_REMOTE_CONTROL_COMMAND_PENDING: Not yet implemented",
			  __func__);
	}

	if (vector & DP_AUTOMATED_TEST_REQUEST) {
		HISI_FB_INFO("%s: DP_AUTOMATED_TEST_REQUEST", __func__);
		retval = handle_automated_test_request(dptx);
		if (retval) {
			HISI_FB_ERR("Automated test request failed\n");
			if (retval == -ENOTSUPP) {
				retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE,
							 DP_TEST_NAK);
				if (retval)
					return retval;
			}
		}
	}

	if (vector & DP_CP_IRQ) {
		HISI_FB_WARNING("%s: DP_CP_IRQ", __func__);
		retval = dptx_write_dpcd(dptx, DP_DEVICE_SERVICE_IRQ_VECTOR,
					 DP_CP_IRQ);
		#ifdef CONFIG_DP_HDCP_ENABLE
			reg = dptx_readl(dptx, DPTX_HDCP_CONFIG);
			reg |= DPTX_CFG_CP_IRQ;
			dptx_writel(dptx, DPTX_HDCP_CONFIG, reg);
			reg = dptx_readl(dptx, DPTX_HDCP_CONFIG);
			reg &= ~DPTX_CFG_CP_IRQ;
			dptx_writel(dptx, DPTX_HDCP_CONFIG, reg);
		#endif
		if (retval)
			return retval;
		/* TODO Check Re-authentication Request and Link integrity
		 * Failure bits in Bstatus
		 */
	}

	if (vector & DP_MCCS_IRQ) {
		/* TODO */
		HISI_FB_WARNING(
			  "%s: DP_MCCS_IRQ: Not yet implemented", __func__);
		retval = -ENOTSUPP;
	}

	if (vector & DP_DOWN_REP_MSG_RDY) {
		/* TODO */
		HISI_FB_WARNING("%s: DP_DOWN_REP_MSG_RDY: Not yet implemented",
			  __func__);
		retval = -ENOTSUPP;
	}

	if (vector & DP_UP_REQ_MSG_RDY) {
		/* TODO */
		HISI_FB_WARNING("%s: DP_UP_REQ_MSG_RDY: Not yet implemented",
			  __func__);
		retval = -ENOTSUPP;
	}

	if (vector & DP_SINK_SPECIFIC_IRQ) {
		/* TODO */
		HISI_FB_WARNING("%s: DP_SINK_SPECIFIC_IRQ: Not yet implemented",
			  __func__);
		retval = -ENOTSUPP;
	}

	return retval;
}
static void dptx_notify(struct dp_ctrl *dptx)
{
	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return;
	}

	wake_up_interruptible(&dptx->waitq);
}

void dptx_notify_shutdown(struct dp_ctrl *dptx)
{
	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return;
	}

	atomic_set(&dptx->shutdown, 1);
	wake_up_interruptible(&dptx->waitq);
}

int dptx_triger_video_transfer(struct dp_ctrl *dptx, bool benable)
{
	uint32_t m_fps;
	uint64_t pxl_clk_rate;
	struct dtd *mdtd;

	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return -EINVAL;
	}

	mdtd = &(dptx->vparams.mdtd);
	pxl_clk_rate = mdtd->pixel_clock * 1000;
	if (pxl_clk_rate % 1000000) {
		pxl_clk_rate = ((pxl_clk_rate /1000000) + 1) * 1000000;
	}
	m_fps = (uint32_t)(pxl_clk_rate / ((mdtd->h_active + mdtd->h_blanking)
                    * (mdtd->v_active + mdtd->v_blanking)));

	if(dptx->video_transfer_enable) {
		if (benable) {
			enable_ldi(dptx->hisifd);
		} else {
			disable_ldi(dptx->hisifd);
			mdelay(1000/m_fps+10);
		}
	}
	return 0;
}


int handle_hotunplug(struct hisi_fb_data_type *hisifd)
{
	struct dp_ctrl *dptx;

	if (!hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("+.\n");

	dptx = &(hisifd->dp);

	hisifd->hpd_release_sub_fnc(hisifd->fbi);

	/* Clear xmit enables */
	dptx_phy_enable_xmit(dptx, 4, false);

	/* Disable AUX Block */
	dptx_aux_disreset(dptx, false);

	/* Power down all lanes */
	/* TODO */
	dptx_phy_set_lanes_status(dptx, false);

#ifdef CONFIG_DP_HDCP_ENABLE
	if(dptx->hisifd->secure_ctrl.hdcp_dpc_sec_en)
		esm_driver_enable(0);
#endif

	atomic_set(&dptx->sink_request, 0);
	atomic_set(&dptx->aux.event, 0);
	dptx->link.trained = false;

	dptx_send_cable_notification(dptx, 0);
	dptx_disable_default_video_stream(dptx);

	dptx->video_transfer_enable = false;

	HISI_FB_INFO("-.\n");

	return 0;
}

static int dptx_read_edid_block(struct dp_ctrl *dptx,
	unsigned int block)
{
	int retval;
	uint8_t offset = block * 128;
	uint8_t segment = block >> 1;
#ifdef CONFIG_DP_EDID_DEBUG
	int i;
#endif

	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("block=%d.\n", block);
	if (segment != 0) {
		retval = dptx_write_bytes_to_i2c(dptx, 0x30, &segment, 1);
		if (retval) {
			HISI_FB_ERR("failed to  dptx_write_bytes_to_i2c 1!\n");
			return retval;
		}
	}
	/* TODO Skip if no E-DDC */

	retval = dptx_write_bytes_to_i2c(dptx, 0x50, &offset, 1);
	if (retval) {
		HISI_FB_ERR("failed to  dptx_write_bytes_to_i2c 2!\n");
		return retval;
	}

	retval = dptx_read_bytes_from_i2c(dptx, 0x50,
		&dptx->edid[block * 128], 128);
	if (retval) {
		HISI_FB_ERR("failed to  dptx_read_bytes_from_i2c 2!\n");
		return retval;
	}

#ifdef CONFIG_DP_EDID_DEBUG
	for (i = 0; i < 128; i++) {
		if (i != 0 && !(i % 16))
			printk("EDID : \n");

		if (!(i % 16))
			printk("%04x: ", i);

		printk("%02x ", dptx->edid[block * 128 + i]);
	}
#endif

	return 0;
}

static int dptx_read_edid(struct dp_ctrl *dptx)
{
	int i;
	int retval = 0;
	unsigned int ext_blocks = 0;
	uint8_t *first_edid_block;

	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return -EINVAL;
	}

	if (!(dptx->edid)) {
		HISI_FB_ERR("edid is NULL!\n");
		return -EINVAL;
	}

	memset(dptx->edid, 0, DPTX_DEFAULT_EDID_BUFLEN);
	retval = dptx_read_edid_block(dptx, 0);
	if (retval) {
		HISI_FB_ERR("failed to dptx_read_edid_block!\n");
		return -EINVAL;
	}

	ext_blocks = dptx->edid[126];

	first_edid_block = kmalloc(128, GFP_KERNEL);
	if (first_edid_block == NULL) {
		HISI_FB_ERR("Allocate buffer error\n");
		return -EINVAL;
	}
	memcpy(first_edid_block, dptx->edid, 128);

	if (dptx->edid) {
		kfree(dptx->edid);
		dptx->edid = NULL;
	}

	dptx->edid = kzalloc(128 * ext_blocks + 128, GFP_KERNEL);
	if (!dptx->edid) {
		HISI_FB_ERR("Allocate edid buffer error!\n");
		return -EINVAL;
	}

	memcpy(dptx->edid, first_edid_block, 128);
	for (i = 1; i <= ext_blocks; i++) {
		retval = dptx_read_edid_block(dptx, i);
		if (retval) {
			goto fail;
		}
	}

	return 0;

fail:
	if (first_edid_block) {
		kfree(first_edid_block);
		first_edid_block = NULL;
	}

	return retval;
}

static int dptx_check_edid(struct dp_ctrl *dptx)
{
	int i;
	u32 edid_sum = 0;

	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return -EINVAL;
	}

	if (!(dptx->edid)) {
		HISI_FB_ERR("edid is NULL!\n");
		return -EINVAL;
	}

	for (i = 0; i < 128; i++)
		edid_sum += dptx->edid[i];

	if (edid_sum & 0xFF) {
		HISI_FB_ERR("Invalid EDID checksum\n");
		return -EINVAL;
	}

	return 0;
}

static int dptx_change_video_mode_tu_fail(struct dp_ctrl *dptx)
{
	struct dtd *mdtd;

	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return -EINVAL;
	}

	mdtd = &(dptx->vparams.mdtd);

	if ((mdtd->pixel_clock > 500000) && (mdtd->h_active >= 3840)) {	/*4k 60HZ*/
		if (((dptx->link.lanes == 2) && (dptx->link.rate == DPTX_PHYIF_CTRL_RATE_HBR2)) ||
			((dptx->link.lanes == 4) && (dptx->link.rate == DPTX_PHYIF_CTRL_RATE_HBR)))
			return 95;	/*4k 30HZ*/
	}

	if (mdtd->h_active == 2560) {
		if ((dptx->link.lanes == 2) && (dptx->link.rate == DPTX_PHYIF_CTRL_RATE_HBR))
			return 16;	/*1080p 60HZ*/
	}

	return 4;
}

static int dptx_resolution_switch(struct hisi_fb_data_type *hisifd)
{
	struct dtd *mdtd;
	struct dp_ctrl *dptx;
	struct hisi_panel_info *pinfo;
	uint32_t m_fps;

	if (!hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	dptx = &(hisifd->dp);
	mdtd = &(dptx->vparams.mdtd);

	pinfo = &(hisifd->panel_info);

	pinfo->xres = mdtd->h_active;
	pinfo->yres = mdtd->v_active;
	pinfo->ldi.h_back_porch =
		(mdtd->h_blanking - mdtd->h_sync_offset -mdtd->h_sync_pulse_width);
	pinfo->ldi.h_front_porch = mdtd->h_sync_offset;
	pinfo->ldi.h_pulse_width = mdtd->h_sync_pulse_width;
	pinfo->ldi.v_back_porch =
		(mdtd->v_blanking - mdtd->v_sync_offset - mdtd->v_sync_pulse_width);
	pinfo->ldi.v_front_porch = mdtd->v_sync_offset;
	pinfo->ldi.v_pulse_width = mdtd->v_sync_pulse_width;
	pinfo->ldi.hsync_plr = 1 - mdtd->h_sync_polarity;
	pinfo->ldi.vsync_plr = 1 - mdtd->v_sync_polarity;
	pinfo->pxl_clk_rate_div = 1;

	pinfo->pxl_clk_rate = mdtd->pixel_clock * 1000;
	if (pinfo->pxl_clk_rate % 1000000) {
		pinfo->pxl_clk_rate = ((pinfo->pxl_clk_rate /1000000) + 1) * 1000000;
	}

	hisifd->fbi->var.xres = pinfo->xres;
	hisifd->fbi->var.yres = pinfo->yres;
	hisifd->fbi->var.pixclock = pinfo->pxl_clk_rate;

	m_fps = (uint32_t)(pinfo->pxl_clk_rate / ((mdtd->h_active + mdtd->h_blanking)
		* (mdtd->v_active + mdtd->v_blanking)));
	HISI_FB_INFO("xres=%d\n"
		"yres=%d\n"
		"h_back_porch=%d\n"
		"h_front_porch=%d\n"
		"h_pulse_width=%d\n"
		"v_back_porch=%d\n"
		"v_front_porch=%d\n"
		"v_pulse_width=%d\n"
		"hsync_plr=%d\n"
		"vsync_plr=%d\n"
		"pxl_clk_rate_div=%d\n"
		"pxl_clk_rate=%llu\n"
		"var.xres=%d\n"
		"var.yres=%d\n"
		"m_fps = %d",
		pinfo->xres,
		pinfo->yres,
		pinfo->ldi.h_back_porch,
		pinfo->ldi.h_front_porch,
		pinfo->ldi.h_pulse_width,
		pinfo->ldi.v_back_porch,
		pinfo->ldi.v_front_porch,
		pinfo->ldi.v_pulse_width,
		pinfo->ldi.hsync_plr,
		pinfo->ldi.vsync_plr,
		pinfo->pxl_clk_rate_div,
		pinfo->pxl_clk_rate,
		hisifd->fbi->var.xres,
		hisifd->fbi->var.yres,
		m_fps);

	dptx_send_cable_notification(dptx, 1);
	hisifd->hpd_open_sub_fnc(hisifd->fbi);
	dptx->video_transfer_enable = true;

	return 0;
}

static int handle_hotplug(struct hisi_fb_data_type *hisifd)
{
#if 0
	uint32_t phyifctrl;
	u8 preferred_vic[18];
#endif
	uint8_t rev;
	int retval;
	uint8_t vector;
	uint8_t checksum = 0;
	uint8_t blocks = 0;
	u8 preferred_vic[18];
	uint8_t test;

	struct video_params *vparams;
	struct hdcp_params *hparams;
	struct dtd mdtd;
	struct dp_ctrl *dptx;

	if (!hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("+.\n");

	dptx = &(hisifd->dp);

	vparams = &dptx->vparams;
	hparams = &dptx->hparams;

	dptx_video_config(dptx);

	dptx_soft_reset(dptx,
		DPTX_SRST_CTRL_PHY | DPTX_SRST_CTRL_HDCP | DPTX_SRST_CTRL_AUX);

	/* Enable AUX Block */
	dptx_aux_disreset(dptx, true);
	dptx_core_init_phy(dptx);

#if 0
	phyifctrl = dptx_readl(DPTX_PHYIF_CTRL);
	phyifctrl &= ~DPTX_PHYIF_CTRL_LANE_PWRDOWN_MASK(0);
	phyifctrl &= ~DPTX_PHYIF_CTRL_LANE_PWRDOWN_MASK(1);
	phyifctrl &= ~DPTX_PHYIF_CTRL_LANE_PWRDOWN_MASK(2);
	phyifctrl &= ~DPTX_PHYIF_CTRL_LANE_PWRDOWN_MASK(3);
	dptx_writel(DPTX_PHYIF_CTRL, phyifctrl);
#endif

	retval = dptx_read_dpcd(dptx, DP_DEVICE_SERVICE_IRQ_VECTOR, &vector);
	if (retval) {
		HISI_FB_ERR("failed to  dptx_read_dpcd DP_DEVICE_SERVICE_IRQ_VECTOR, retval=%d.", retval);
		return retval;
	}

	retval = dptx_read_edid(dptx);
	if (retval) {
		HISI_FB_ERR("failed to  dptx_read_edid, retval=%d.", retval);
		return retval;
	}

	retval = dptx_check_edid(dptx);
	if (retval) {
		vparams->video_format = VCEA;
		dptx_dtd_fill(&mdtd, 4, vparams->refresh_rate, vparams->video_format);
	} else {
		memcpy(preferred_vic, dptx->edid + 0x36, 0x12);
		retval = dptx_dtd_parse(dptx, &mdtd, preferred_vic);
		if (retval) {
			vparams->video_format = VCEA;
			dptx_dtd_fill(&mdtd, 4, vparams->refresh_rate, vparams->video_format);
		}
	}
	memcpy(&(dptx->vparams.mdtd), &mdtd, sizeof(mdtd));

	retval = dptx_read_dpcd(dptx, DP_DPCD_REV, &rev);
	if (retval) {
		/* Abort bringup */
		/* Reset core and try again */
		/* Abort all aux, and other work, reset the core */
		HISI_FB_ERR("failed to dptx_read_dpcd DP_DPCD_REV, retval=%d.\n", retval);
		return retval;
	}
	HISI_FB_INFO("DP Revision %x.%x .\n", (rev & 0xf0) >> 4, rev & 0xf);

	memset(dptx->rx_caps, 0, DPTX_RECEIVER_CAP_SIZE);
	retval = dptx_read_bytes_from_dpcd(dptx, DP_DPCD_REV,
		dptx->rx_caps, DPTX_RECEIVER_CAP_SIZE);
	if (retval) {
		HISI_FB_ERR("failed to dptx_read_bytes_from_dpcd DP_DPCD_REV, retval=%d.\n", retval);
		return retval;
	}

	/*
	* The TEST_EDID_READ is asserted on HOTPLUG. Check for it and
	* handle it here.
	*/
	if (vector & DP_AUTOMATED_TEST_REQUEST) {
		HISI_FB_INFO("DP_AUTOMATED_TEST_REQUEST");
		retval = dptx_read_dpcd(dptx, DP_TEST_REQUEST, &test);
		if (retval) {
			HISI_FB_ERR("failed to dptx_read_dpcd DP_TEST_REQUEST, retval=%d.\n", retval);
			return retval;
		}

		if (test & DP_TEST_LINK_EDID_READ) {
			blocks = dptx->edid[126];
			checksum = dptx->edid[127 + 128 * blocks];

			retval = dptx_write_dpcd(dptx, DP_TEST_EDID_CHECKSUM, checksum);
			if (retval) {
				HISI_FB_ERR("failed to dptx_write_dpcd DP_TEST_EDID_CHECKSUM, retval=%d.\n", retval);
				return retval;
			}

			retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_EDID_CHECKSUM_WRITE);
			if (retval) {
				HISI_FB_ERR("failed to dptx_write_dpcd DP_TEST_RESPONSE, retval=%d.\n", retval);
				return retval;
			}
		}
	}

	/* TODO No other IRQ should be set on hotplug */
	retval = dptx_link_training(dptx, dptx->max_rate, dptx->max_lanes);
	if (retval) {
		HISI_FB_ERR("failed to  dptx_link_training, retval=%d.\n", retval);
		return retval;
	}

	retval = dptx_video_ts_calculate(dptx, dptx->link.lanes,
		dptx->link.rate, vparams->bpc, vparams->pix_enc, vparams->mdtd.pixel_clock);
	if (retval) {
		HISI_FB_INFO("Can't change to the preferred video mode: frequency = %d\n",
						mdtd.pixel_clock);
		//HISI_FB_INFO("Changing to the default video mode\n");
		vparams->video_format = VCEA;
		vparams->mode = dptx_change_video_mode_tu_fail(dptx);
		retval = dptx_video_mode_change(dptx, vparams->mode);
		if (retval) {
			HISI_FB_ERR("Change mode error!\n");
			return retval;
		}
	} else {
		vparams->mdtd = mdtd;
		HISI_FB_INFO("pixel_frequency=%d.\n", mdtd.pixel_clock);

		/* MMCM */
	#ifdef CONFIG_DP_GENERATOR_REF
		dptx_video_reset(dptx, 1);
		retval = dptx_video_pixel_freq_change(dptx, mdtd.pixel_clock);
		if (retval) {
			HISI_FB_ERR("failed to  dptx_video_pixel_freq_change, retval=%d.\n", retval);
			dptx_video_reset(dptx, 0);
			return retval;
		}
		dptx_video_reset(dptx, 0);
		dptx_video_timing_change(dptx);
	#endif
	}

	dptx_resolution_switch(hisifd);
	dptx_video_timing_change(dptx);	/*dptx video reg depends on dss pixel clock.*/
	dptx_audio_config(dptx);	/*dptx audio reg depends on phy status(P0)*/

#ifdef CONFIG_DP_HDCP_ENABLE
	hparams->auth_fail_count = 0;

	if(hisifd->secure_ctrl.hdcp_dpc_sec_en) {
		esm_driver_enable(1);
	}
#endif

	HISI_FB_INFO("-.\n");

	return 0;
}

#ifdef CONFIG_DP_HDCP_ENABLE
static void dptx_en_dis_hdcp13(struct dp_ctrl *dptx, bool en)
{
	u32 reg;

	reg = dptx_readl(dptx, DPTX_HDCP_CONFIG);

	if (en) {
		reg |= BIT(2);
	} else {
		reg &= ~ BIT(2);
	}

	dptx_writel(dptx, DPTX_HDCP_CONFIG, reg);
}

static void handle_hdcp22_gpio_intr(struct dp_ctrl *dptx)
{
	u32 hdcpobs;

	hdcpobs = dptx_readl(dptx, DPTX_HDCP_OBS);
	HISI_FB_DEBUG("[HDCP22] DPTX_HDCP_OBS = 0x%x.\n", hdcpobs);

	if (hdcpobs & DPTX_HDCP22_BOOTED)
		HISI_FB_DEBUG("[HDCP22] ESM has booted.\n");

	if (hdcpobs & DPTX_HDCP22_CAP_CHECK_COMPLETE) {
		if (hdcpobs & DPTX_HDCP22_CAPABLE_SINK)
			HISI_FB_DEBUG("[HDCP22] sink is HDCP22 capable\n");
		else
			HISI_FB_DEBUG("[HDCP22] sink is not HDCP22 capable\n");
	}

	if (hdcpobs & DPTX_HDCP22_AUTH_SUCCESS)
		HISI_FB_DEBUG("[HDCP22] the authentication is succeded.\n");

	if (hdcpobs & DPTX_HDCP22_AUTH_FAILED)
		HISI_FB_DEBUG("[HDCP22] the authentication is failed.\n");

	if (hdcpobs & DPTX_HDCP22_RE_AUTH_REQ)
		HISI_FB_DEBUG("[HDCP22] the sink has requested a re-authentication.\n");

	dptx_writel(dptx, DPTX_HDCP_OBS, hdcpobs);
}

static void handle_hdcp_intr(struct dp_ctrl *dptx)
{
	u32 hdcpintsts;
	struct hdcp_params *hparams;

	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return ;
	}

	hparams = &dptx->hparams;

	hdcpintsts = dptx_readl(dptx, DPTX_HDCP_INT_STS);
	HISI_FB_DEBUG("DPTX_HDCP_INT_STS=0x%08x.\n", hdcpintsts);

	if (hdcpintsts & DPTX_HDCP_KSV_ACCESS_INT)
		HISI_FB_DEBUG("KSV memory access guaranteed for read, write access\n");

	if (hdcpintsts & DPTX_HDCP_KSV_SHA1)
		HISI_FB_DEBUG("SHA1 verification has been done\n");

	if (hdcpintsts & DPTX_HDCP_AUX_RESP_TIMEOUT) {
		dptx_en_dis_hdcp13(dptx, 0);
		HISI_FB_DEBUG("DPTX_HDCP_AUX_RESP_TIMEOUT\n");
	}

	if (hdcpintsts & DPTX_HDCP_FAILED) {
		hparams->auth_fail_count++;
		if (hparams->auth_fail_count > DPTX_HDCP_MAX_AUTH_RETRY) {
			dptx_en_dis_hdcp13(dptx, 0);
			HISI_FB_DEBUG("Reach max allowed retries count=%d.\n", hparams->auth_fail_count);
		}
		HISI_FB_INFO("HDCP authentication process was failed!\n");
	}

	if (hdcpintsts & DPTX_HDCP_ENGAGED) {
		hparams->auth_fail_count = 0;
		HISI_FB_DEBUG("HDCP authentication process was successful.\n");
	}

	if (hdcpintsts & DPTX_HDCP22_GPIOINT) {
		HISI_FB_DEBUG("HDCP22_GPIOINT\n");
		handle_hdcp22_gpio_intr(dptx);
	}

	dptx_writel(dptx, DPTX_HDCP_INT_CLR, hdcpintsts);
}
#endif

static void handle_aux_reply(struct dp_ctrl *dptx)
{
	uint32_t auxsts;
	uint32_t status;
	uint32_t auxm;
	uint32_t br;

	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return ;
	}

	auxsts = dptx_readl(dptx, DPTX_AUX_STS);

	status = (auxsts & DPTX_AUX_STS_STATUS_MASK) >> DPTX_AUX_STS_STATUS_SHIFT;
	auxm = (auxsts & DPTX_AUX_STS_AUXM_MASK) >> DPTX_AUX_STS_AUXM_SHIFT;
	br = (auxsts & DPTX_AUX_STS_BYTES_READ_MASK) >> DPTX_AUX_STS_BYTES_READ_SHIFT;

	HISI_FB_DEBUG("DPTX_AUX_STS=0x%08x: sts=%d, auxm=%d, br=%d, "
		"replyrcvd=%d, replyerr=%d, timeout=%d, disconn=%d.\n",
		auxsts, status, auxm, br,
		!!(auxsts & DPTX_AUX_STS_REPLY_RECEIVED),
		!!(auxsts & DPTX_AUX_STS_REPLY_ERR),
		!!(auxsts & DPTX_AUX_STS_TIMEOUT),
		!!(auxsts & DPTX_AUX_STS_SINK_DWA));

	switch (status) {
	case DPTX_AUX_STS_STATUS_ACK:
		HISI_FB_DEBUG("DPTX_AUX_STS_STATUS_ACK!\n");
		break;
	case DPTX_AUX_STS_STATUS_NACK:
		HISI_FB_DEBUG("DPTX_AUX_STS_STATUS_NACK!\n");
		break;
	case DPTX_AUX_STS_STATUS_DEFER:
		HISI_FB_DEBUG("DPTX_AUX_STS_STATUS_DEFER!\n");
		break;
	case DPTX_AUX_STS_STATUS_I2C_NACK:
		HISI_FB_DEBUG("DPTX_AUX_STS_STATUS_I2C_NACK!\n");
		break;
	case DPTX_AUX_STS_STATUS_I2C_DEFER:
		HISI_FB_DEBUG("DPTX_AUX_STS_STATUS_I2C_DEFER!\n");
		break;
	default:
		HISI_FB_ERR("Invalid AUX status 0x%x.\n", status);
		break;
	}

	dptx->aux.data[0] = dptx_readl(dptx, DPTX_AUX_DATA0);
	dptx->aux.data[1] = dptx_readl(dptx, DPTX_AUX_DATA1);
	dptx->aux.data[2] = dptx_readl(dptx, DPTX_AUX_DATA2);
	dptx->aux.data[3] = dptx_readl(dptx, DPTX_AUX_DATA3);
	dptx->aux.sts = auxsts;

	atomic_set(&dptx->aux.event, 1);
	dptx_notify(dptx);
}

irqreturn_t dptx_threaded_irq(int irq, void *dev)
{
	int retval = 0;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;
	uint32_t hpdsts;

	hisifd = dev;
	if (!hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return IRQ_HANDLED;
	}

	dptx = &(hisifd->dp);

	mutex_lock(&dptx->dptx_mutex);
	if (!dptx->dptx_enable) {
		HISI_FB_WARNING("dptx has already off!\n");
		mutex_unlock(&dptx->dptx_mutex);
		return IRQ_HANDLED;
	}

	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		g_bit_hpd_status = DPTX_HPDSTS_STATUS_GA;
	} else {
		g_bit_hpd_status = DPTX_HPDSTS_STATUS_EA03;
	}

	/*
	 * TODO this should be set after all AUX transactions that are
	 * queued are aborted. Currently we don't queue AUX and AUX is
	 * only started from this function.
	 */
	atomic_set(&dptx->aux.abort, 0);
	if (atomic_read(&dptx->c_connect)) {
		atomic_set(&dptx->c_connect, 0);

		hpdsts = dptx_readl(dptx, DPTX_HPDSTS);
		HISI_FB_INFO("HPDSTS = 0x%08x.\n", hpdsts);

		if (hpdsts & g_bit_hpd_status) {
			retval = handle_hotplug(hisifd);
			if (retval) {
				HISI_FB_ERR("DP Device Hotplug error %d\n", retval);
			}
		} else {
			retval = handle_hotunplug(hisifd);
			if (retval) {
				HISI_FB_ERR("DP Device Hotplug error %d\n", retval);
			}
		}
	}

	if (atomic_read(&dptx->sink_request)) {
		atomic_set(&dptx->sink_request, 0);
		retval = handle_sink_request(dptx);
		if (retval) {
			HISI_FB_ERR("Unable to handle sink request %d\n", retval);
		}
	}

	mutex_unlock(&dptx->dptx_mutex);

	return IRQ_HANDLED;
}

void dptx_hpd_handler(struct dp_ctrl *dptx, bool plugin, uint8_t dp_lanes)
{
	u32 reg = 0;

	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return ;
	}
	HISI_FB_INFO("DP Plug Type:(%d), Lanes:(%d)\n", plugin, dp_lanes);
	/* need to  check dp lanes */

	dptx->max_lanes = dp_lanes;

	reg = dptx_readl(dptx, DPTX_CCTL);
	if (plugin) {
		reg |= DPTX_CCTL_FORCE_HPD;
	} else {
		reg &= ~DPTX_CCTL_FORCE_HPD;
	}

	dptx_writel(dptx, DPTX_CCTL, reg);
}

void dptx_hpd_irq_handler(struct dp_ctrl *dptx)
{
	int retval = 0;
	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return ;
	}

	HISI_FB_INFO("DP Shot Plug!\n");

	dptx_notify(dptx);
	retval = handle_sink_request(dptx);
	if (retval) {
		HISI_FB_ERR("Unable to handle sink request %d\n", retval);
	}
}

irqreturn_t dptx_irq(int irq, void *dev)
{
	irqreturn_t retval = IRQ_HANDLED;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx = NULL;
	uint32_t ists;
	uint32_t ien;
	uint32_t hpdsts;

	hisifd = (struct hisi_fb_data_type *)dev;
	if (!hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return IRQ_HANDLED;
	}

	dptx = &(hisifd->dp);

	ists = dptx_readl(dptx, DPTX_ISTS);
	ien = dptx_readl(dptx, DPTX_IEN);
	hpdsts = dptx_readl(dptx, DPTX_HPDSTS);
	HISI_FB_DEBUG("DPTX_ISTS=0x%08x, DPTX_IEN=0x%08x, DPTX_HPDSTS=0x%08x.\n",
		ists, ien, hpdsts);

	if (!(ists & DPTX_ISTS_ALL_INTR)) {
		HISI_FB_INFO("IRQ_NONE, DPTX_ISTS=0x%08x.\n", ists);
		retval = IRQ_NONE;
		return retval;
	}

	if (ists & DPTX_ISTS_AUX_REPLY) {
		ists &= ~DPTX_ISTS_AUX_REPLY;
		handle_aux_reply(dptx);
		dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_AUX_REPLY);
	}

	if (ists & DPTX_ISTS_AUX_CMD_INVALID) {
		/* TODO abort AUX */
		/* handle_aux_reply(dptx); */
		dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_AUX_CMD_INVALID);
	}

	if (ists & DPTX_ISTS_HDCP) {
	#ifdef CONFIG_DP_HDCP_ENABLE
		handle_hdcp_intr(dptx);
		dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_HDCP);
	#endif
	}

	if (ists & DPTX_ISTS_SDP) {
		/* TODO Handle and clear */
	}

	if (ists & DPTX_ISTS_AUDIO_FIFO_OVERFLOW) {
		if (ien & DPTX_IEN_AUDIO_FIFO_OVERFLOW) {
			HISI_FB_INFO("DPTX_ISTS_AUDIO_FIFO_OVERFLOW!\n");
			dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_AUDIO_FIFO_OVERFLOW);
		}
	}

	if (ists & DPTX_ISTS_VIDEO_FIFO_OVERFLOW) {
		if (ien & DPTX_IEN_VIDEO_FIFO_OVERFLOW) {
			HISI_FB_ERR("DPTX_ISTS_VIDEO_FIFO_OVERFLOW!\n");
			dptx_triger_video_transfer(dptx, false);
			dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_VIDEO_FIFO_OVERFLOW);
		}
	}

	if (ists & DPTX_ISTS_TYPE_C) {
		/* TODO Handle and clear */
		dptx_writel(dptx, DPTX_TYPE_C_CTRL, DPTX_TYPEC_INTERRUPT_STATUS);

		HISI_FB_DEBUG("\n DPTX_TYPE_C_CTRL: [%02x] PRE", dptx_readl(dptx, DPTX_TYPE_C_CTRL));
		dptx_typec_reset_ack(dptx);
		HISI_FB_DEBUG("\n DPTX_TYPE_C_CTRL: [%02x] AFT", dptx_readl(dptx, DPTX_TYPE_C_CTRL));
	}

	if (ists & DPTX_ISTS_HPD) {
		if (hpdsts & DPTX_HPDSTS_IRQ) {
			dptx_writel(dptx, DPTX_HPDSTS, DPTX_HPDSTS_IRQ);
			atomic_set(&dptx->sink_request, 1);
			dptx_notify(dptx);
			retval = IRQ_WAKE_THREAD;
		}

		if (hpdsts & DPTX_HPDSTS_HOT_PLUG) {
			dptx_writel(dptx, DPTX_HPDSTS, DPTX_HPDSTS_HOT_PLUG);

			/* if (hpdsts & DPTX_HPDSTS_STATUS) { */
			if (1) {
				atomic_set(&dptx->aux.abort, 1);
				atomic_set(&dptx->c_connect, 1);
				dptx_notify(dptx);
				retval = IRQ_WAKE_THREAD;
			} else {
				HISI_FB_INFO("Hot plug - not connected\n");
			}
		}

		if (hpdsts & DPTX_HPDSTS_HOT_UNPLUG) {
			dptx_writel(dptx, DPTX_HPDSTS, DPTX_HPDSTS_HOT_UNPLUG);

			/* if (!(hpdsts & DPTX_HPDSTS_STATUS)) { */
			if (1) {
				atomic_set(&dptx->aux.abort, 1);
				atomic_set(&dptx->c_connect, 1);
				dptx_notify(dptx);
				retval = IRQ_WAKE_THREAD;
			} else {
				HISI_FB_INFO("Hot unplug - not disconnected\n");
			}
		}

		if (hpdsts & 0x80) {
			HISI_FB_INFO("DPTX_HPDSTS[7] HOTPLUG DEBUG INTERRUPT!\n");
			dptx_writel(dptx, DPTX_HPDSTS, 0x80 | DPTX_HPDSTS_HOT_UNPLUG);
		}
	}

	return retval;
}
/*lint -restore*/
