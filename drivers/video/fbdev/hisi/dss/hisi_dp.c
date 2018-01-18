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

#include <linux/hisi/contexthub/tca.h>

#include "hisi_fb_def.h"
#include "hisi_fb.h"
#include "hisi_dp.h"

#include "dp/avgen.h"
#include "dp/intr.h"
#include "dp/core.h"
#include "dp/intr.h"


struct platform_device *g_dp_pdev = NULL;


/*******************************************************************************
**
*/
static int dp_clk_enable(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	struct clk *clk_tmp = NULL;
	int ret = 0;

	if (pdev == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	clk_tmp = hisifd->dss_auxclk_dpctrl_clk;
	if (clk_tmp) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_auxclk_dpctrl_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_auxclk_dpctrl_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	clk_tmp = hisifd->dss_pclk_dpctrl_clk;
	if (clk_tmp) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pclk_dpctrl_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pclk_dpctrl_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	clk_tmp = hisifd->dss_aclk_dpctrl_clk;
	if (clk_tmp) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_aclk_dpctrl_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_aclk_dpctrl_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return 0;//lint !e438
}//lint !e550

static int dp_clk_disable(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	struct clk *clk_tmp = NULL;

	if (pdev == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	clk_tmp = hisifd->dss_auxclk_dpctrl_clk;
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	clk_tmp = hisifd->dss_pclk_dpctrl_clk;
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	clk_tmp = hisifd->dss_aclk_dpctrl_clk;
	if (clk_tmp) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	return 0;
}

static int dp_clock_setup(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	int ret;
	uint32_t default_aclk_dpctrl_rate;

	if (pdev == NULL) {
		HISI_FB_ERR("pdev NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	hisifd->dss_auxclk_dpctrl_clk = devm_clk_get(&pdev->dev, hisifd->dss_auxclk_dpctrl_name);
	if (IS_ERR(hisifd->dss_auxclk_dpctrl_clk)) {
		ret = PTR_ERR(hisifd->dss_auxclk_dpctrl_clk);//lint !e712
		HISI_FB_ERR("fb%d %s get fail ret = %d.\n",
			hisifd->index, hisifd->dss_auxclk_dpctrl_name, ret);
		return ret;
	} else {
		ret = clk_set_rate(hisifd->dss_auxclk_dpctrl_clk, DEFAULT_AUXCLK_DPCTRL_RATE);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_auxclk_dpctrl_clk clk_set_rate(%lu) failed, error=%d!\n",
				hisifd->index, DEFAULT_AUXCLK_DPCTRL_RATE, ret);
			return -EINVAL;
		}

		HISI_FB_INFO("dss_auxclk_dpctrl_clk:[%lu]->[%lu].\n",
			DEFAULT_AUXCLK_DPCTRL_RATE, clk_get_rate(hisifd->dss_auxclk_dpctrl_clk));
	}

	default_aclk_dpctrl_rate = 0;
	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		default_aclk_dpctrl_rate = DEFAULT_ACLK_DPCTRL_RATE_CS;
	} else {
		default_aclk_dpctrl_rate = DEFAULT_ACLK_DPCTRL_RATE_ES;
	}

	hisifd->dss_aclk_dpctrl_clk = devm_clk_get(&pdev->dev, hisifd->dss_aclk_dpctrl_name);
	if (IS_ERR(hisifd->dss_aclk_dpctrl_clk)) {
		ret = PTR_ERR(hisifd->dss_aclk_dpctrl_clk);//lint !e712
		HISI_FB_ERR("fb%d dss_aclk_dpctrl_clk get fail ret = %d.\n",
			hisifd->index, ret);
		return ret;
	} else {
		ret = clk_set_rate(hisifd->dss_aclk_dpctrl_clk, default_aclk_dpctrl_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_aclk_dpctrl_clk clk_set_rate(%lu) failed, error=%d!\n",
				hisifd->index, default_aclk_dpctrl_rate, ret);
			return -EINVAL;
		}

		HISI_FB_INFO("dss_aclk_dpctrl_clk:[%lu]->[%lu].\n",
			default_aclk_dpctrl_rate, clk_get_rate(hisifd->dss_aclk_dpctrl_clk));
	}

	hisifd->dss_pclk_dpctrl_clk = devm_clk_get(&pdev->dev, hisifd->dss_pclk_dpctrl_name);
	if (IS_ERR(hisifd->dss_pclk_dpctrl_clk)) {
		ret = PTR_ERR(hisifd->dss_pclk_dpctrl_clk);//lint !e712
		HISI_FB_ERR("fb%d dss_pclk_dpctrl_clk get fail ret = %d.\n",
			hisifd->index, ret);
		return ret;
	}

	return 0;
}

static int dp_on(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;

	if (pdev == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);

	mutex_lock(&dptx->dptx_mutex);

	if (dptx->dptx_enable) {
		HISI_FB_ERR("dptx has already on.\n");
		mutex_unlock(&dptx->dptx_mutex);
		return 0;
	}

	/* dp dis reset */
	outp32(hisifd->peri_crg_base + PERRSTDIS0, 0x00000400);
	dp_clk_enable(pdev);

	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		if (!dptx_check_dptx_id(dptx)) {
			HISI_FB_ERR("DPTX_ID not match to 0x%04x:0x%04x!\n",
				DPTX_ID_DEVICE_ID, DPTX_ID_VENDOR_ID);
			ret = -ENODEV;
			goto err_out;
		}
	}

	ret = dptx_core_init(dptx);
	if (ret) {
		HISI_FB_ERR("DPTX core init failed!\n");
		ret = -ENODEV;
		goto err_out;
	}

	/* FIXME: clear intr */
	dptx_global_intr_dis(dptx);
	/*dptx_global_intr_clear(dptx);*/
	enable_irq(dptx->irq);
	/* Enable all top-level interrupts */
	dptx_global_intr_en(dptx);

	dptx->dptx_enable = true;

	ret = panel_next_on(pdev);
	if (ret) {
		HISI_FB_ERR("panel_next_on failed!\n");
	}

err_out:
	mutex_unlock(&dptx->dptx_mutex);

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dp_off(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;

	if (pdev == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);

	mutex_lock(&dptx->dptx_mutex);

	if (!dptx->dptx_enable) {
		HISI_FB_ERR("dptx has already off.\n");
		mutex_unlock(&dptx->dptx_mutex);
		return 0;
	}

	/* FIXME: clear intr */
	dptx_global_intr_dis(dptx);
	disable_irq_nosync(dptx->irq);

	/* FIXME: */
	if (dptx->video_transfer_enable) {
		handle_hotunplug(hisifd);
	}

	mdelay(10);

	dp_clk_disable(pdev);

	/* dp reset */
	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		outp32(hisifd->peri_crg_base + PERRSTEN0, 0x00000400);
	}

	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;

	ret = panel_next_off(pdev);
	if (ret) {
		HISI_FB_ERR("Panel DP next off error !!\n");
	}

	mutex_unlock(&dptx->dptx_mutex);

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return ret;
}

void dptx_send_cable_notification(struct dp_ctrl *dptx, int val)
{
	int state = 0;

	if (!dptx) {
		HISI_FB_ERR("dptx is NULL!\n");
		return;
	}

	state = dptx->sdev.state;

	switch_set_state(&dptx->sdev, val);

	HISI_FB_INFO("cable state %s %d\n",
		dptx->sdev.state == state ? "is same" : "switched to", dptx->sdev.state);
}

/*******************************************************************************
**
*/
int hisi_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode)
{
	int ret;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;
	uint8_t dp_lanes = 0;

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("g_dp_pdev is NULL!\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);

	mutex_lock(&dptx->dptx_mutex);

	if (!dptx->dptx_enable) {
		HISI_FB_ERR("dptx has already off.\n");
		ret = -EINVAL;
		goto fail;
	}

	HISI_FB_INFO("DP HPD Type:(%d), Mode:(%d)\n", irq_type, mode);

	switch (mode) {
	case TCPC_DP:
		dp_lanes = 4;
		break;
	case TCPC_USB31_AND_DP_2LINE:
		dp_lanes = 2;
		break;
	default:
		HISI_FB_ERR("not supported tcpc_mux_ctrl_type=%d.\n", mode);
		ret = -EINVAL;
		goto fail;
	}

	switch (irq_type) {
	case TCA_IRQ_HPD_OUT:
		dptx_hpd_handler(dptx, false, dp_lanes);
		break;
	case TCA_IRQ_HPD_IN:
		dptx_hpd_handler(dptx, true, dp_lanes);
		break;
	case TCA_IRQ_SHORT:
		dptx_hpd_irq_handler(dptx);
		break;
	default:
		HISI_FB_ERR("not supported tca_irq_type=%d.\n", irq_type);
		ret = -EINVAL;
		goto fail;
	}

fail:
	mutex_unlock(&dptx->dptx_mutex);

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return 0;
}
EXPORT_SYMBOL_GPL(hisi_dptx_hpd_trigger);

int hisi_dptx_triger(bool enable)
{
	struct hisi_fb_data_type *hisifd;
	int ret;

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("g_dp_pdev is NULL!\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("fb%d, +. DP Device %s\n", hisifd->index, enable ? "ON" : "OFF");

	if (enable) {
		ret = dp_on(g_dp_pdev);
		if (ret) {
			HISI_FB_ERR("dp_on failed!\n");
		}
	} else {
		ret = dp_off(g_dp_pdev);
		if (ret) {
			HISI_FB_ERR("dp_off failed!\n");
		}
	}

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return ret;
}
EXPORT_SYMBOL_GPL(hisi_dptx_triger);

int hisi_dptx_notify_switch(void)
{
	int ret;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;
	bool lanes_status_change = false;

	if (g_dp_pdev == NULL) {
		HISI_FB_ERR("g_dp_pdev is NULL!\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(g_dp_pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	dptx = &(hisifd->dp);
	HISI_FB_INFO("fb%d, + [DP] Status: %d.\n", hisifd->index, dptx->dptx_enable);
	mutex_lock(&dptx->dptx_mutex);

	if (dptx->dptx_enable) {
		lanes_status_change = true;
		dptx_disable_default_video_stream(dptx);
		dptx_phy_set_lanes_status(dptx, false);
	}

	mutex_unlock(&dptx->dptx_mutex);

	if (!lanes_status_change) {
		ret = dp_on(g_dp_pdev);
		if (ret) {
			HISI_FB_ERR("DP on failed!\n");
			return -EINVAL;
		}
	}

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return 0;
}
EXPORT_SYMBOL_GPL(hisi_dptx_notify_switch);

bool hisi_dptx_ready(void)
{
	return (g_dp_pdev != NULL);
}
EXPORT_SYMBOL_GPL(hisi_dptx_ready);

/*******************************************************************************
**
*/
static int dp_device_init(struct platform_device *pdev)
{
	struct dp_ctrl *dptx;
	struct hisi_fb_data_type *hisifd;
	int ret;

	if (pdev == NULL) {
		HISI_FB_ERR("pdev NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("fb%d +.\n", hisifd->index);

	ret = dp_clock_setup(pdev);
	if (ret) {
		HISI_FB_ERR("DP clock setup failed!\n");
		return ret;
	}

	dptx = &(hisifd->dp);

	dptx->dev = &pdev->dev;
	dptx->base = hisifd->dp_base;
	dptx->hisifd = hisifd;
	if (IS_ERR(dptx->base)) {
		HISI_FB_ERR("dptx base is NULL!\n");
		return -EINVAL;
	}

	dptx->irq = hisifd->dp_irq;
	if (!dptx->irq) {
		HISI_FB_ERR("dptx irq is NULL!\n");
		return -EINVAL;
	}

	mutex_init(&dptx->dptx_mutex);
	init_waitqueue_head(&dptx->waitq);
	atomic_set(&(dptx->sink_request), 0);
	atomic_set(&(dptx->shutdown), 0);
	atomic_set(&(dptx->c_connect), 0);

	dptx->max_rate = DPTX_PHYIF_CTRL_RATE_HBR2;
	dptx->max_lanes = 4;
	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;

	dptx_video_params_reset(&dptx->vparams);
	dptx_audio_params_reset(&dptx->aparams);

	dptx->edid = devm_kzalloc(&pdev->dev, DPTX_DEFAULT_EDID_BUFLEN, GFP_KERNEL);
	if (!dptx->edid) {
		HISI_FB_ERR("dptx base is NULL!\n");
		return -ENOMEM;
	}
	memset(dptx->edid, 0, DPTX_DEFAULT_EDID_BUFLEN);

	dptx->sdev.name = "hisi-dp";
	if (switch_dev_register(&dptx->sdev) < 0) {
		HISI_FB_ERR("dp switch registration failed!\n");
		ret = -ENODEV;
		goto err_edid_alloc;
	}

	ret = devm_request_threaded_irq(&(pdev->dev),
		dptx->irq, dptx_irq, dptx_threaded_irq,
		IRQF_SHARED | IRQ_LEVEL, "dwc_dptx", (void *)hisifd);//lint !e747
	if (ret) {
		HISI_FB_ERR("Request for irq %d failed!\n", dptx->irq);
		goto err_sdev_register;
	} else {
		disable_irq(dptx->irq);
	}

	HISI_FB_INFO("fb%d -.\n", hisifd->index);

	return 0;

err_sdev_register:
	switch_dev_unregister(&dptx->sdev);

err_edid_alloc:
	if (dptx->edid != NULL) {
		devm_kfree(&pdev->dev, dptx->edid);
		dptx->edid = NULL;
	}

	return ret;
}

static int dp_remove(struct platform_device *pdev)
{
	int ret;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;

	if (pdev == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	dptx = &(hisifd->dp);

	switch_dev_unregister(&dptx->sdev);

	if (dptx->edid != NULL) {
		devm_kfree(&pdev->dev, dptx->edid);
		dptx->edid = NULL;
	}

	dptx->dptx_enable = false;
	dptx->video_transfer_enable = false;
	dptx_core_deinit(dptx);

	dptx_notify_shutdown(dptx);
	mdelay(15); //lint !e778 !e747 !e774 !e845

	ret = panel_next_remove(pdev);

#if 0
	hisi_dp_hpd_unregister(hisifd);
#endif

	g_dp_pdev = NULL;

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dp_probe(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	struct platform_device *dpp_dev;
	struct hisi_fb_panel_data *pdata;
	int ret;

	if (pdev == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	ret = dp_device_init(pdev);
	if (ret) {
		HISI_FB_ERR("fb%d mipi_dsi_irq_clk_setup failed, error=%d!\n", hisifd->index, ret);
		return -EINVAL;
	}

#if 0
	ret = hisi_dp_hpd_register(hisifd);
	if (ret) {
		HISI_FB_ERR("fb%d hisi_dp_hpd_register failed, error=%d!\n", hisifd->index, ret);
		return -EINVAL;
	}
#endif

	/* alloc device */
	dpp_dev = platform_device_alloc(DEV_NAME_DSS_DPE, pdev->id);
	if (!dpp_dev) {
		HISI_FB_ERR("fb%d platform_device_alloc failed, error=%d!\n", hisifd->index, ret);
		ret = -ENOMEM;
		goto err_device_alloc;
	}

	/* link to the latest pdev */
	hisifd->pdev = dpp_dev;

	/* alloc panel device data */
	ret = platform_device_add_data(dpp_dev, dev_get_platdata(&pdev->dev),
		sizeof(struct hisi_fb_panel_data));
	if (ret) {
		HISI_FB_ERR("fb%d platform_device_add_data failed error=%d!\n", hisifd->index, ret);
		ret = -EINVAL;
		goto err_device_put;
	}

	/* data chain */
	pdata = dev_get_platdata(&dpp_dev->dev);
	pdata->on = NULL;
	pdata->off = NULL;
	pdata->remove = dp_remove;
	pdata->next = pdev;

	/* get/set panel info */
	memcpy(&hisifd->panel_info, pdata->panel_info, sizeof(struct hisi_panel_info));

	/* set driver data */
	platform_set_drvdata(dpp_dev, hisifd);
	/* device add */
	ret = platform_device_add(dpp_dev);
	if (ret) {
		HISI_FB_ERR("fb%d platform_device_add failed, error=%d!\n", hisifd->index, ret);
		ret = -EINVAL;
		goto err_device_put;
	}

	g_dp_pdev = pdev;

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);

	return 0;

err_device_put:
	platform_device_put(dpp_dev);
err_device_alloc:
	return ret;
}

/*lint -save -e* */
static struct platform_driver this_driver = {
	.probe = dp_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = DEV_NAME_DP,
	},
};
/*lint -restore*/

static int __init dp_driver_init(void)
{
	int ret;

	ret = platform_driver_register(&this_driver);//lint !e64

	if (ret) {
		HISI_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

/*lint -e528 -esym(528,*)*/
module_init(dp_driver_init);
/*lint -e528 +esym(528,*)*/
