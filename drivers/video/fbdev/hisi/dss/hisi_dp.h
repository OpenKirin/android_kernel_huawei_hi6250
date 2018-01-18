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
#ifndef HISI_DP_H
#define HISI_DP_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/switch.h>

#include "dp/drm_dp_helper.h"
#include "dp/drm_dp_helper_additions.h"
#include "dp/reg.h"
#include "dp/dbg.h"


#define CONFIG_DP_HDCP_ENABLE
//#define CONFIG_DP_GENERATOR_REF
#define CONFIG_DP_EDID_DEBUG
#define CONFIG_DP_SETTING_COMBOPHY 1

#define DPTX_RECEIVER_CAP_SIZE	(0x100)
#define DPTX_SDP_NUM		(0x10)
#define DPTX_SDP_LEN	(0x9)
#define DPTX_SDP_SIZE	(9 * 4)
#define DPTX_DEFAULT_EDID_BUFLEN	(128UL)

/* The max rate and lanes supported by the core */
#define DPTX_MAX_LINK_RATE	DPTX_PHYIF_CTRL_RATE_HBR2
#define DPTX_MAX_LINK_LANES	(4)

/* The default rate and lanes to use for link training */
#define DPTX_DEFAULT_LINK_RATE DPTX_MAX_LINK_RATE
#define DPTX_DEFAULT_LINK_LANES DPTX_MAX_LINK_LANES

#define DP_SYSFS_ATTRS_NUM	(10)

#define DPTX_HDCP_REG_DPK_CRC_ORIG	0x331c1169
#define DPTX_HDCP_MAX_AUTH_RETRY	10

#define DPTX_AUX_TIMEOUT	(2000)

#define DEFAULT_AUXCLK_DPCTRL_RATE	16000000UL
#define DEFAULT_ACLK_DPCTRL_RATE_ES	288000000UL
#define DEFAULT_ACLK_DPCTRL_RATE_CS	238000000UL

/* #define DPTX_DEVICE_INFO(pdev) platform_get_drvdata(pdev)->panel_info->dp */

enum dp_tx_hpd_states {
	HPD_OFF,
	HPD_ON,
};

enum pixel_enc_type {
	RGB = 0,
	YCBCR420 = 1,
	YCBCR422 = 2,
	YCBCR444 = 3,
	YONLY = 4,
	RAW = 5

};

enum color_depth {
	COLOR_DEPTH_INVALID = 0,
	COLOR_DEPTH_6 = 6,
	COLOR_DEPTH_8 = 8,
	COLOR_DEPTH_10 = 10,
	COLOR_DEPTH_12 = 12,
	COLOR_DEPTH_16 = 16
};

enum pattern_mode {
	TILE = 0,
	RAMP = 1,
	CHESS = 2,
	COLRAMP = 3
};

enum dynamic_range_type {
	CEA = 1,
	VESA = 2
};

enum colorimetry_type {
	ITU601 = 1,
	ITU709 = 2
};

enum video_format_type {
	VCEA = 0,
	CVT = 1,
	DMT = 2
};

/**
 * struct dptx_link - The link state.
 * @status: Holds the sink status register values.
 * @trained: True if the link is successfully trained.
 * @rate: The current rate that the link is trained at.
 * @lanes: The current number of lanes that the link is trained at.
 * @preemp_level: The pre-emphasis level used for each lane.
 * @vswing_level: The vswing level used for each lane.
 */
struct dptx_link {
	uint8_t status[DP_LINK_STATUS_SIZE];
	bool trained;
	uint8_t rate;
	uint8_t lanes;
	uint8_t preemp_level[4];
	uint8_t vswing_level[4];
};

/**
 * struct dptx_aux - The aux state
 * @sts: The AUXSTS register contents.
 * @data: The AUX data register contents.
 * @event: Indicates an AUX event ocurred.
 * @abort: Indicates that the AUX transfer should be aborted.
 */
struct dptx_aux {
	uint32_t sts;
	uint32_t data[4];
	atomic_t event;
	atomic_t abort;
};

struct sdp_header {
	uint8_t HB0;
	uint8_t HB1;
	uint8_t HB2;
	uint8_t HB3;
} __packed;

struct sdp_full_data {
	uint8_t en;
	uint32_t payload[9];
	uint8_t blanking;
	uint8_t cont;
} __packed;

struct hdcp_aksv {
	uint32_t lsb;
	uint32_t msb;
};

struct hdcp_dpk {
	uint32_t lsb;
	uint32_t msb;
};

struct hdcp_params {
	struct hdcp_aksv aksv;
	struct hdcp_dpk dpk[40];
	uint32_t enc_key;
	uint32_t crc32;
	uint32_t auth_fail_count;
	uint32_t hdcp13_is_en;
};

typedef struct hdcp13_int_params {
	u8 auth_fail_count;
	int hdcp13_is_en;
}hdcp13_int_params_t;

enum _master_hdcp_op_type_ {
	DSS_HDCP13_ENABLE = 1,
	DSS_HDCP22_ENABLE,
	DSS_HDCP13_ENCRYPT_ENABLE,
	DSS_HDCP_DPC_SEC_EN,
	HDCP_OP_SECURITY_MAX,
};

struct audio_params {
	uint8_t iec_channel_numcl0;
	uint8_t iec_channel_numcr0;
	uint8_t use_lut;
	uint8_t iec_samp_freq;
	uint8_t iec_word_length;
	uint8_t iec_orig_samp_freq;
	uint8_t data_width;
	uint8_t num_channels;
	uint8_t inf_type;
	uint8_t mute;
	uint8_t ats_ver;
};

struct dtd {
	uint16_t pixel_repetition_input;
	int pixel_clock;
	uint8_t interlaced; /* 1 for interlaced, 0 progressive */
	uint16_t h_active;
	uint16_t h_blanking;
	uint16_t h_image_size;
	uint16_t h_sync_offset;
	uint16_t h_sync_pulse_width;
	uint8_t h_sync_polarity;
	uint16_t v_active;
	uint16_t v_blanking;
	uint16_t v_image_size;
	uint16_t v_sync_offset;
	uint16_t v_sync_pulse_width;
	uint8_t v_sync_polarity;
};

struct video_params {
	enum pixel_enc_type pix_enc;
	enum pattern_mode pattern_mode;
	struct dtd mdtd;
	uint8_t mode;
	enum color_depth bpc;
	enum colorimetry_type colorimetry;
	enum dynamic_range_type dynamic_range;
	uint8_t aver_bytes_per_tu;
	uint8_t aver_bytes_per_tu_frac;
	uint8_t init_threshold;
	uint32_t refresh_rate;
	uint8_t video_format;
};

/**
 * struct dp_ctrl - The representation of the DP TX core
 * @mutex:
 * @base: Base address of the registers
 * @irq: IRQ number
 * @version: Contents of the IP_VERSION register
 * @max_rate: The maximum rate that the controller supports
 * @max_lanes: The maximum lane count that the controller supports
 * @dev: The struct device
 * @root: The debugfs root
 * @regset: The debugfs regset
 * @vparams: The video params to use
 * @aparams: The audio params to use
 * @waitq: The waitq
 * @shutdown: Signals that the driver is shutting down and that all
 *            operations should be aborted.
 * @c_connect: Signals that a HOT_PLUG or HOT_UNPLUG has occurred.
 * @sink_request: Signals the a HPD_IRQ has occurred.
 * @rx_caps: The sink's receiver capabilities.
 * @edid: The sink's EDID.
 * @aux: AUX channel state for performing an AUX transfer.
 * @link: The current link state.
 */
struct dp_ctrl {
	struct mutex dptx_mutex; /* generic mutex for dptx */

	void __iomem *base;
	uint32_t irq;

	uint32_t version;
	uint8_t max_rate;
	uint8_t max_lanes;

	struct device *dev;
	struct switch_dev sdev;
	struct hisi_fb_data_type *hisifd;

	struct video_params vparams;
	struct audio_params aparams;
	struct hdcp_params hparams;

	wait_queue_head_t waitq;

	atomic_t shutdown;
	atomic_t c_connect;
	atomic_t sink_request;

	uint8_t rx_caps[DPTX_RECEIVER_CAP_SIZE];

	uint8_t *edid;

	struct sdp_full_data sdp_list[DPTX_SDP_NUM];
	struct dptx_aux aux;
	struct dptx_link link;

	bool dptx_enable;
	bool video_transfer_enable;

	uint32_t hpd_state;

	int sysfs_index;
	struct attribute *sysfs_attrs[DP_SYSFS_ATTRS_NUM];
	struct attribute_group sysfs_attr_group;
};

static inline uint32_t dptx_readl(struct dp_ctrl *dp, uint32_t offset)
{
	uint32_t data = readl(dp->base + offset);
	return data;
}

static inline void dptx_writel(struct dp_ctrl *dp, uint32_t offset, uint32_t data)
{
	writel(data, dp->base + offset);
}

/*
 * Wait functions
 */
#define dptx_wait(_dptx, _cond, _timeout)				\
	({								\
		int __retval;						\
		__retval = wait_event_interruptible_timeout(		\
			_dptx->waitq,					\
			((_cond) || (atomic_read(&_dptx->shutdown))),	\
			msecs_to_jiffies(_timeout));			\
		if (atomic_read(&_dptx->shutdown)) {			\
			__retval = -ESHUTDOWN;				\
		}							\
		else if (!__retval) {					\
			__retval = -ETIMEDOUT;				\
		}							\
		__retval;						\
	})


void dptx_notify_shutdown(struct dp_ctrl *dptx);
void dptx_send_cable_notification(struct dp_ctrl *dptx, int val);

struct hisi_fb_data_type;
int hisi_dp_hpd_register(struct hisi_fb_data_type *hisifd);
void hisi_dp_hpd_unregister(struct hisi_fb_data_type *hisifd);


#endif  /* HISI_DP_H */
