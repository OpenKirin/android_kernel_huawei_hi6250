/* Copyright (c) 2010-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_fb.h"
#include "hisi_dp.h"


static ssize_t dp_tx_sysfs_hpd_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dp_ctrl *dptx = NULL;
	int ret = 0;

	if (NULL == dev) {
		HISI_FB_ERR("dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("buf NULL Pointer!\n");
		return -1;
	}

	dptx = &(hisifd->dp);
	if (!dptx) {
		HISI_FB_ERR("invalid dptx!\n");
		return -EINVAL;
	}

	ret = snprintf(buf, PAGE_SIZE, "hpd_state=%d.\n",
		dptx->hpd_state);

	HISI_FB_DEBUG("hpd_state=%d.\n",
		dptx->hpd_state);

	return ret;
}

static ssize_t dp_tx_sysfs_hpd_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct fb_info *fbi = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct dp_ctrl *dptx = NULL;
	int hpd = 0;

	if (NULL == dev) {
		HISI_FB_ERR("dev NULL Pointer!\n");
		return -1;
	}

	fbi = dev_get_drvdata(dev);
	if (NULL == fbi) {
		HISI_FB_ERR("fbi NULL Pointer!\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)fbi->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd NULL Pointer!\n");
		return -1;
	}

	if (NULL == buf) {
		HISI_FB_ERR("buf NULL Pointer!\n");
		return -1;
	}

	HISI_FB_INFO("+!\n");

	dptx = &(hisifd->dp);
	if (!dptx) {
		HISI_FB_ERR("invalid dptx!\n");
		return -EINVAL;
	}

	hpd = (int)simple_strtoul(buf, NULL, 0);

	HISI_FB_INFO("hpd=%d.\n", hpd);

	dptx->hpd_state = hpd;

	switch (hpd) {
	case HPD_OFF:
	#if 0
		if (!dptx->dptx_enable) {
			HISI_FB_INFO("HPD is already off!\n");
			return ret;
		}
	#endif

		//hisifd->hpd_release_sub_fnc(fbi);
		dptx_send_cable_notification(dptx, 0);
		break;
	case HPD_ON:
	#if 0
		if (dptx->dptx_enable) {
			HISI_FB_INFO("HPD is already on!\n");
			return ret;
		}
	#endif

		//hisifd->hpd_open_sub_fnc(fbi);
		dptx_send_cable_notification(dptx, 1);
		break;
	default:
		HISI_FB_ERR("Invalid HPD state requested!\n");
		break;
	}

	HISI_FB_INFO("-.\n");

	return count;
}

static DEVICE_ATTR(dp_hpd, S_IRUGO | S_IWUSR,
	dp_tx_sysfs_hpd_show, dp_tx_sysfs_hpd_store);

int hisi_dp_hpd_register(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	struct dp_ctrl *dptx = NULL;

	if (!hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	dptx = &(hisifd->dp);
	if (!dptx) {
		HISI_FB_ERR("invalid dptx!\n");
		return -EINVAL;
	}

	if (hisifd->sysfs_attrs_append_fnc) {
		hisifd->sysfs_attrs_append_fnc(hisifd, &dev_attr_dp_hpd.attr);
	}

	dptx->hpd_state = HPD_OFF;

	return ret;
}

void hisi_dp_hpd_unregister(struct hisi_fb_data_type *hisifd)
{
	struct dp_ctrl *dptx = NULL;

	if (!hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return;
	}

	dptx = &(hisifd->dp);
	if (!dptx) {
		HISI_FB_ERR("invalid dptx!\n");
		return;
	}

	dptx->hpd_state = HPD_OFF;
}
