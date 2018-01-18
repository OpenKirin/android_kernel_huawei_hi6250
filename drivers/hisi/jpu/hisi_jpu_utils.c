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
#include <linux/delay.h>
#include <media/camera/jpeg/jpeg_base.h>
#include "hisi_jpu.h"

/*******************************************************************************
** gloabel Definition
*/

struct iommu_domain* g_hisijpu_domain = NULL;


/*******************************************************************************
** Global Variable declaration
*/
bool is_original_format(jpu_raw_format in_format, jpu_color_space out_format) {
	if(((HISI_JPEG_DECODE_RAW_YUV444 == in_format) && (HISI_JPEG_DECODE_OUT_YUV444 == out_format))
		|| ((HISI_JPEG_DECODE_RAW_YUV422_H2V1 == in_format) && ( HISI_JPEG_DECODE_OUT_YUV422_H2V1 == out_format))
		|| ((HISI_JPEG_DECODE_RAW_YUV422_H1V2 == in_format) && ( HISI_JPEG_DECODE_OUT_YUV422_H1V2 == out_format))
		|| (( HISI_JPEG_DECODE_RAW_YUV420 == in_format) && (HISI_JPEG_DECODE_OUT_YUV420 == out_format))
		|| (( HISI_JPEG_DECODE_RAW_YUV400 == in_format) && (HISI_JPEG_DECODE_OUT_YUV400 == out_format))){
		return true;
	}

	return false;
}
jpu_output_format hisi_out_format_hal2jpu(struct hisi_jpu_data_type *hisijd)
{
	jpu_data_t *jpu_req;
	jpu_output_format format  = JPU_OUTPUT_UNSUPPORT;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return format;
	}

	jpu_req = &(hisijd->jpu_req);
	HISI_JPU_INFO("in_img_format %d, out_color_format %d!\n",
		jpu_req->in_img_format, jpu_req->out_color_format);

	if(is_original_format(jpu_req->in_img_format, jpu_req->out_color_format)) {
		return JPU_OUTPUT_YUV_ORIGINAL;
	}

	/* YUV400 can't decode to yuv420 */
	if((HISI_JPEG_DECODE_RAW_YUV400 == jpu_req->in_img_format)
		&& (HISI_JPEG_DECODE_OUT_YUV420 == jpu_req->out_color_format)) {
		return format;
	}

	switch (jpu_req->out_color_format) {
	case HISI_JPEG_DECODE_OUT_YUV420:
		format =  JPU_OUTPUT_YUV420;
		break;
	case HISI_JPEG_DECODE_OUT_RGBA4444:
		format =  JPU_OUTPUT_RGBA4444;
		break;
	case HISI_JPEG_DECODE_OUT_BGRA4444:
		format =  JPU_OUTPUT_BGRA4444;
		break;
	case HISI_JPEG_DECODE_OUT_RGB565:
		format =  JPU_OUTPUT_RGB565;
		break;
	case HISI_JPEG_DECODE_OUT_BGR565:
		format =  JPU_OUTPUT_BGR565;
		break;
	case HISI_JPEG_DECODE_OUT_RGBA8888:
		format =  JPU_OUTPUT_RGBA8888;
		break;

	case HISI_JPEG_DECODE_OUT_BGRA8888:
		format =  JPU_OUTPUT_BGRA8888;
		break;
	default:
		break;
	}

	return format;
}
static int hisijpu_set_dht(jpu_data_t *pjpu_req, char __iomem *jpu_dec_base)
{
	int i;
	int j;

	if (!pjpu_req) {
		HISI_JPU_ERR("pjpu_req is NULL!\n");
		return -EINVAL;
	}

	if (!jpu_dec_base) {
		HISI_JPU_ERR("jpu_dec_base is NULL!\n");
		return -EINVAL;
	}

	for (i = 0; i < HDC_TABLE_NUM; i++) {
		outp32(jpu_dec_base + JPGD_REG_HDCTABLE + 4 * i, pjpu_req->hdc_tab[i]);
		HISI_JPU_DEBUG("hdc_tab[%d]%d\n", i, pjpu_req->hdc_tab[i]);
	}

	for (i = 0; i < HAC_TABLE_NUM; i++) {
		outp32(jpu_dec_base + JPGD_REG_HACMINTABLE + 4 * i,  pjpu_req->hac_min_tab[i] );
		outp32(jpu_dec_base + JPGD_REG_HACBASETABLE + 4 * i,  pjpu_req->hac_base_tab[i]);
		HISI_JPU_DEBUG(" hac_min_tab[%d] %ul\n", i, pjpu_req->hac_min_tab[i]);
		HISI_JPU_DEBUG(" hac_base_tab[%d] %ul\n", i, pjpu_req->hac_base_tab[i]);
	}

	for (j = 0; j < HAC_SYMBOL_TABLE_NUM; j++) {
		outp32(jpu_dec_base + JPGD_REG_HACSYMTABLE + 4 * j,  pjpu_req->hac_symbol_tab[j]);
		HISI_JPU_DEBUG(" gs_hac_symbol_tab[%d] %ul\n", j, pjpu_req->hac_symbol_tab[j]);
	}

	return 0;
}

static void hisijpu_set_dqt(jpu_data_t *pjpu_req, char __iomem *jpu_dec_base)
{
	uint32_t s32Cnt;
	if((!pjpu_req) || (!jpu_dec_base)) {
		HISI_JPU_ERR("pjpu_req or jpu_dec_base null");
		return;
	}

	for (s32Cnt = 0; s32Cnt < MAX_DCT_SIZE; s32Cnt++) {
		outp32(jpu_dec_base + JPGD_REG_QUANT + 4 * s32Cnt,  pjpu_req->qvalue[s32Cnt]);
	}

	return;
}

static int hisijpu_set_crop(struct hisi_jpu_data_type *hisijd)
{
	jpu_data_t *jpu_req;
	jpu_dec_reg_t *pjpu_dec_reg;

	if(!hisijd) {
		HISI_JPU_ERR("pjpu_req or hisijd null");
		return -EINVAL;
	}

	jpu_req = &(hisijd->jpu_req);
	pjpu_dec_reg = &(hisijd->jpu_dec_reg);

	/* for full decode */
	if (HISI_JPEG_DECODE_MODE_FULL_SUB >= jpu_req->decode_mode) {
		pjpu_dec_reg->crop_horizontal = jpu_set_bits32(pjpu_dec_reg->crop_horizontal,
			(jpu_req->inwidth - 1) << 16, 32, 0);
		pjpu_dec_reg->crop_vertical = jpu_set_bits32(pjpu_dec_reg->crop_vertical,
			(jpu_req->inheight- 1) << 16, 32, 0);
	} else {
		if((jpu_req->region_info.right < MIN_INPUT_WIDTH) || (jpu_req->region_info.bottom < MIN_INPUT_WIDTH)) {
			return -EINVAL;
		}
		/*lint -save -e737*/
		pjpu_dec_reg->crop_horizontal = jpu_set_bits32(pjpu_dec_reg->crop_horizontal,
			(((uint32_t)(jpu_req->region_info.right - 1)) << 16) | (jpu_req->region_info.left ), 32, 0);
		pjpu_dec_reg->crop_vertical = jpu_set_bits32(pjpu_dec_reg->crop_vertical,
			(((uint32_t)(jpu_req->region_info.bottom - 1)) << 16) | (jpu_req->region_info.top), 32, 0);
		/*lint -restore*/
	}

	return 0;
}

/******************************************************************************
**
*/
uint32_t jpu_set_bits32(uint32_t old_val, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (uint32_t)((1UL << bw) - 1UL);
	uint32_t tmp = old_val;

	tmp &= ~(mask << bs);

	return (tmp | ((val & mask) << bs));
}

void jpu_get_timestamp(struct timeval *tv)
{
	struct timespec ts;

	ktime_get_ts(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;

	/* struct timeval timestamp;
	do_gettimeofday(&timestamp);
	timestamp = ktime_to_timeval(ktime_get()); */
}

long jpu_timestamp_diff(struct timeval *lasttime, struct timeval *curtime)
{
	long ret;
	ret = (curtime->tv_usec >= lasttime->tv_usec) ?
		curtime->tv_usec - lasttime->tv_usec:
		1000000 - (lasttime->tv_usec - curtime->tv_usec);

	return ret;
}

static int hisi_sample_size_hal2jpu(int val)
{
	int ret = 0;

	switch (val) {
	case HISI_JPEG_DECODE_SAMPLE_SIZE_1:
		ret = JPU_FREQ_SCALE_1;
		break;
	case HISI_JPEG_DECODE_SAMPLE_SIZE_2:
		ret = JPU_FREQ_SCALE_2;
		break;
	case HISI_JPEG_DECODE_SAMPLE_SIZE_4:
		ret = JPU_FREQ_SCALE_4;
		break;
	case HISI_JPEG_DECODE_SAMPLE_SIZE_8:
		ret = JPU_FREQ_SCALE_8;
		break;

	default:
		HISI_JPU_ERR("not support sample size(%d)!\n", val);
		ret = -1;
		break;
	}

	return ret;
}

static bool isRGB_out(uint32_t format)
{
	switch (format) {
	case HISI_JPEG_DECODE_OUT_RGBA4444:
	case HISI_JPEG_DECODE_OUT_BGRA4444:
	case HISI_JPEG_DECODE_OUT_RGB565:
	case HISI_JPEG_DECODE_OUT_BGR565:
	case HISI_JPEG_DECODE_OUT_RGBA8888:
	case HISI_JPEG_DECODE_OUT_BGRA8888:
		return true;

	default:
		return false;
	}
}

/******************************************************************************
**
*/
int hisijpu_irq_request(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;
	HISI_JPU_INFO("++!\n");

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}
    /*lint -save -e747*/
	if (hisijd->jpu_done_irq) {
		HISI_JPU_INFO("request jpu_done_irq!\n");

		ret = request_irq(hisijd->jpu_done_irq, hisi_jpu_dec_done_isr, 0, IRQ_JPU_DEC_DONE_NAME, (void *)hisijd);
		if (ret != 0) {
			HISI_JPU_ERR("request_irq failed, irq_no=%d error=%d!\n", hisijd->jpu_done_irq, ret);
			return ret;
		} else {
			HISI_JPU_INFO("request_irq OK disable, irq_no=%d error=%d!\n", hisijd->jpu_done_irq, ret);
			disable_irq(hisijd->jpu_done_irq);
		}
	}

	if (hisijd->jpu_err_irq) {
		HISI_JPU_INFO("request jpu_err_irq!\n");

		ret = request_irq(hisijd->jpu_err_irq, hisi_jpu_dec_err_isr, 0, IRQ_JPU_DEC_ERR_NAME, (void *)hisijd);
		if (ret != 0) {
			HISI_JPU_ERR("request_irq failed, irq_no=%d error=%d!\n", hisijd->jpu_err_irq, ret);
			return ret;
		} else {
			disable_irq(hisijd->jpu_err_irq);
		}
	}

	if (hisijd->jpu_other_irq) {
		ret = request_irq(hisijd->jpu_other_irq, hisi_jpu_dec_other_isr, 0, IRQ_JPU_DEC_OTHER_NAME, (void *)hisijd);
		if (ret != 0) {
			HISI_JPU_ERR("request_irq failed, irq_no=%d error=%d!\n", hisijd->jpu_other_irq, ret);
			return ret;
		} else {
			disable_irq(hisijd->jpu_other_irq);
		}
	}
	HISI_JPU_INFO("--!\n");
	/*lint -restore*/

	return ret;
}

int hisijpu_irq_enable(struct hisi_jpu_data_type *hisijd)
{
	HISI_JPU_INFO("++!\n");

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (hisijd->jpu_done_irq)
		enable_irq(hisijd->jpu_done_irq);

	if (hisijd->jpu_err_irq)
		enable_irq(hisijd->jpu_err_irq);

	if (hisijd->jpu_other_irq)
		enable_irq(hisijd->jpu_other_irq);

	HISI_JPU_INFO("--!\n");

	return 0;
}

int hisijpu_irq_disable(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (hisijd->jpu_err_irq)
		disable_irq(hisijd->jpu_err_irq);

	if (hisijd->jpu_done_irq)
		disable_irq(hisijd->jpu_done_irq);

	if (hisijd->jpu_other_irq)
		disable_irq(hisijd->jpu_other_irq);

	return 0;
}

int hisijpu_irq_disable_nosync(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (hisijd->jpu_err_irq)
		disable_irq_nosync(hisijd->jpu_err_irq);

	if (hisijd->jpu_done_irq)
		disable_irq_nosync(hisijd->jpu_done_irq);

	if (hisijd->jpu_other_irq)
		disable_irq_nosync(hisijd->jpu_other_irq);

	return 0;
}

/* lint -save -e838 */
int hisijpu_media1_regulator_enable(struct hisi_jpu_data_type *hisijd)
{
	int ret;
#ifdef CONFIG_NO_USE_INTERFACE
	uint32_t tmp = 0;
#endif

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	HISI_JPU_DEBUG("+.\n");

#ifdef CONFIG_NO_USE_INTERFACE
	/********** set_pu_media1_subsys *********/
	outp32(hisijd->peri_crg_base + 0x150, 0x00000020);
	udelay(100);

	outp32(hisijd->peri_crg_base + 0x0A0, 0x00040000);

	/* step2 module clk enable */
	outp32(hisijd->peri_crg_base + 0x410, 0x7C002028);
	outp32(hisijd->sctrl_base + 0x1B0, 0x00000040);
	outp32(hisijd->peri_crg_base + 0x040, 0x00000040);
	udelay(1);

	/* step3 module clk disable */
	outp32(hisijd->peri_crg_base + 0x414, 0x7C002028);
	outp32(hisijd->sctrl_base + 0x1B4, 0x00000040);
	outp32(hisijd->peri_crg_base + 0x044, 0x00000040);
	udelay(1);

	/* step4 module iso disable */
	outp32(hisijd->peri_crg_base + 0x148, 0x00000040);

	/* step5 memory rempair */
	udelay(400);

	/* step6 module unrst */
	outp32(hisijd->peri_crg_base + 0x0A0, 0x00020000);

	/* step7 module clk enable */
	outp32(hisijd->peri_crg_base + 0x410, 0x7C002028);
	outp32(hisijd->sctrl_base + 0x1B0, 0x00000040);
	outp32(hisijd->peri_crg_base + 0x040, 0x00000040);

	/********** set_pu_vivobus ***********/
	/* step2 module clk enable */
	outp32(hisijd->media1_crg_base + 0x084, 0x00080008);
	outp32(hisijd->media1_crg_base , 0x08040040);
	udelay(1);

	/* step3 module clk disable */
	outp32(hisijd->media1_crg_base + 0x004 , 0x00040040);
	udelay(1);

	/* module clk enable */
	outp32(hisijd->media1_crg_base , 0x00040040);

	/* stepbus idle clear */
	outp32(hisijd->pmctrl_base + 0x380, 0x80000000);
	tmp = (uint32_t)inp32(hisijd->pmctrl_base + 0x384);
	if ((tmp | 0xffff7fff) == 0xffffffff) {
		HISI_JPU_ERR("vivobus fail to clear bus idle tmp 0x%x!\n", tmp);
	}
	udelay(1);

	tmp = (uint32_t)inp32(hisijd->pmctrl_base + 0x388);
	if ((tmp | 0xffff7fff) == 0xffffffff) {
		HISI_JPU_ERR("vivobus fail to clear bus idle 0x388 tmp 0x%x!\n", tmp);
	}
	udelay(1);

	ret = 0;

#else
	ret = regulator_enable(hisijd->media1_regulator);
	if (ret) {
		HISI_JPU_ERR("jpu media1_regulator failed, error=%d!\n", ret);
	}
#endif

	HISI_JPU_DEBUG("-.\n");

	return ret;
}

/* lint -save -e838 */
int hisijpu_jpu_regulator_enable(struct hisi_jpu_data_type *hisijd)
{
	int ret;
#ifdef CONFIG_NO_USE_INTERFACE
	uint32_t tmp = 0;
#endif
	HISI_JPU_DEBUG("+.\n");

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

#ifdef CONFIG_NO_USE_INTERFACE
	/*********** set_pu_isp ***********/
	/* step 1 module mtcmos on */
	outp32(hisijd->peri_crg_base + 0x150, 0x1);
	udelay(100);

	outp32(hisijd->media1_crg_base + 0x34, 0x04000000);

	/* step2 module clock enable */
	outp32(hisijd->media1_crg_base + 0x84, 0x30003000);
	outp32(hisijd->media1_crg_base + 0x010, 0x1E198000);
	udelay(1);

	/* step3 module clk disable 3 */
	outp32(hisijd->media1_crg_base + 0x14, 0x1E018000);
	udelay(1);

	/* step4 module iso disable */
	outp32(hisijd->peri_crg_base + 0x148, 0x00000001);

	/* step5 memory rempair */
	udelay(400);

	/* step6 module unrst */
	outp32(hisijd->media1_crg_base + 0x34, 0x01E00000);
	outp32(hisijd->media1_crg_base + 0x40, 0x0000000C);

	/* step7 module clk enable */
	outp32(hisijd->media1_crg_base + 0x010, 0x1E018000);

	/* step8 bus idle clear */
	outp32(hisijd->pmctrl_base + 0x380, 0x00200000);
	mdelay(1);

	tmp = (uint32_t)inp32(hisijd->pmctrl_base + 0x384);
	if ((tmp | 0xffffffdf) == 0xffffffff) {
		HISI_JPU_ERR("fail to isp clear bus idle tmp 0x%x!\n", tmp);
	}
	mdelay(1);
	tmp = (uint32_t)inp32(hisijd->pmctrl_base + 0x388);
	if ((tmp | 0xffffffdf) == 0xffffffff) {
		HISI_JPU_ERR("fail to isp clear bus idle 0x388 tmp 0x%x!\n", tmp);
	}
	mdelay(1);

	ret = 0;
#else
	ret = regulator_enable(hisijd->jpu_regulator);
	if (ret) {
		HISI_JPU_ERR("jpu regulator_enable failed, error=%d!\n", ret);
	}
#endif

	HISI_JPU_DEBUG("-.\n");

	return ret;
}

int hisijpu_jpu_regulator_disable(struct hisi_jpu_data_type *hisijd)
{
	int ret;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	HISI_JPU_DEBUG("+.\n");

#ifdef CONFIG_NO_USE_INTERFACE
	ret = 0;
#else
	ret = regulator_disable(hisijd->jpu_regulator);
	if (ret != 0) {
		HISI_JPU_ERR("jpu regulator_disable failed, error=%d!\n", ret);
	}
#endif
	HISI_JPU_DEBUG("-.\n");

	return ret;
}
/* lint -restore */

int hisijpu_media1_regulator_disable(struct hisi_jpu_data_type *hisijd)
{
	int ret;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	HISI_JPU_DEBUG("+.\n");

#ifdef CONFIG_NO_USE_INTERFACE
	ret = 0;
#else
	ret = regulator_disable(hisijd->media1_regulator);
	if (ret != 0) {
		HISI_JPU_ERR("jpu regulator_disable failed, error=%d!\n", ret);
	}
#endif
	HISI_JPU_DEBUG("-.\n");

	return ret;
}
/* lint -restore */

int hisijpu_clk_enable(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;
	HISI_JPU_DEBUG("+.\n");

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (hisijd->jpg_func_clk) {
		ret = jpeg_dec_set_rate(hisijd->jpg_func_clk, JPG_FUNC_CLK_DEFAULT_RATE);
		if (ret) {
			HISI_JPU_ERR("jpg_func_clk set clk failed, error=%d!\n", ret);
			return -EINVAL;
		}

		ret = jpeg_dec_clk_prepare_enable(hisijd->jpg_func_clk);
		if (ret) {
			HISI_JPU_ERR("jpg_func_clk clk_prepare failed, error=%d!\n", ret);
			return -EINVAL;
		}
	}

	HISI_JPU_DEBUG("-.\n");

	return 0;
}

int hisijpu_clk_disable(struct hisi_jpu_data_type *hisijd)
{
    int ret;
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	HISI_JPU_DEBUG("+.\n");

	if (hisijd->jpg_func_clk) {
		jpeg_dec_clk_disable_unprepare(hisijd->jpg_func_clk);

		ret = jpeg_dec_set_rate(hisijd->jpg_func_clk, JPG_FUNC_CLK_PD_RATE);
		if (ret != 0) {
		    HISI_JPU_ERR("fail to set power down rate!\n");
		    return -EINVAL;
		}
	}

	HISI_JPU_DEBUG("-.\n");

	return 0;
}

int hisijpu_enable_iommu(struct platform_device *pdev)
{
	struct iommu_domain *hisi_domain;

	if(!pdev) {
		HISI_JPU_ERR("pdev NULL!\n");
		return -EINVAL;
	}

	/* create iommu domain */
	hisi_domain = hisi_ion_enable_iommu(NULL);;
	if (!hisi_domain) {
		HISI_JPU_ERR("iommu_domain_alloc failed!\n");
		return -EINVAL;
	}

	g_hisijpu_domain = hisi_domain;

	return 0;
}

void hisijpu_disable_iommu(struct platform_device *pdev)
{
	if(!pdev) {
		HISI_JPU_ERR("pdev NULL!\n");
		return;
	}

	g_hisijpu_domain = NULL;
}

int hisi_jpu_register(struct hisi_jpu_data_type *hisijd)
{
	int ret;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	hisijd->jpu_dec_done_flag = 0;
	init_waitqueue_head(&hisijd->jpu_dec_done_wq);

	sema_init(&hisijd->blank_sem, 1);
	hisijd->power_on = false;
	hisijd->jpu_res_initialized = false;
	hisijd->ion_client = hisi_ion_client_create(HISI_JPU_ION_CLIENT_NAME);
	if (IS_ERR_OR_NULL(hisijd->ion_client)) {
		HISI_JPU_ERR("failed to create ion client!\n");
		return -ENOMEM;
	}

	ret = hisi_jpu_lb_alloc(hisijd);
	if (ret != 0) {
		HISI_JPU_ERR("hisi_jpu_lb_alloc failed!\n");
		goto err_out;
	}

	hisijd->jpg_func_clk = devm_clk_get(&(hisijd->pdev->dev), hisijd->jpg_func_clk_name);
	if (IS_ERR(hisijd->jpg_func_clk)) {
		HISI_JPU_ERR("jpg_func_clk devm_clk_get error!");
		ret = -ENXIO;
		goto err_out;
	}

	ret = hisijpu_irq_request(hisijd);
	if (ret) {
		HISI_JPU_ERR("hisijpu_irq_request failed!");
		ret = -ENXIO;
		goto err_out;
	}

err_out:
	return ret;
}

int hisi_jpu_unregister(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (hisijd->jpu_done_irq) {
		free_irq(hisijd->jpu_done_irq, 0);
	}

	if (hisijd->jpu_err_irq) {
		free_irq(hisijd->jpu_err_irq, 0);
	}

	if (hisijd->jpu_other_irq) {
		free_irq(hisijd->jpu_other_irq, 0);
	}

	hisi_jpu_lb_free(hisijd);

	if (hisijd->ion_client) {
		ion_client_destroy(hisijd->ion_client);
		hisijd->ion_client = NULL;
	}

	return 0;
}

/*lint -save -e438 -e550 -e715 -e747 -e774 -e778*/
void hisijpu_set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs)
{
	uint32_t mask = (uint32_t)((1UL << bw) - 1UL);
	uint32_t tmp;

	tmp = (uint32_t)inp32(addr);
	tmp &= ~(mask << bs);

	outp32(addr, tmp | ((val & mask) << bs));
}

int hisijpu_check_reg_state(char __iomem *addr, uint32_t val)
{
	uint32_t tmp;
	int delay_count = 0;
	bool is_timeout = true;
	if(!addr) {
		return -1;
	}

	while (1) {
		tmp = (uint32_t)inp32(addr);
		if(((tmp & val) == val) || (delay_count > 10)) {
			is_timeout = (delay_count > 10) ? true : false;
			udelay(10);
			break;
		} else {
			udelay(10);
			++delay_count;
		}
	}

	if(is_timeout) {
		HISI_JPU_ERR("fail to wait reg!\n");
		return -1;
	}

	return 0;
}
/*lint -restore*/

void hisi_jpu_dec_normal_reset(struct hisi_jpu_data_type *hisijd) {
	int ret;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return;
	}

	hisijpu_set_reg(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_1, 0x1, 1, 25);
	ret = hisijpu_check_reg_state(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_1, 0x01000000);
	if(ret) {
		HISI_JPU_ERR("fail to wait JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_1!\n");
	}

	hisijpu_set_reg(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_1, 0x0, 1, 25);
}

void hisi_jpu_dec_error_reset(struct hisi_jpu_data_type *hisijd) {
    int ret;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return;
	}

	/* step1 */
	hisijpu_set_reg(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 0x00010000, 32, 0);
	hisijpu_set_reg(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_1, 0x1, 1, 25);
	hisijpu_set_reg(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_2, 0x1, 1, 25);
	hisijpu_set_reg(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_0, 0x1, 1, 25);
	hisijpu_set_reg(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_1, 0x1, 1, 25);

	/* step2 */
	ret = hisijpu_check_reg_state(hisijd->jpu_top_base + JPGDEC_RO_STATE, 0x2);
	if(ret) {
		HISI_JPU_ERR("fail to wait JPGDEC_RO_STATE!\n");
	}

	ret = hisijpu_check_reg_state(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_1, 0x01000000);
	if(ret) {
		HISI_JPU_ERR("fail to wait JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_1!\n");
	}

	ret = hisijpu_check_reg_state(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_2, 0x01000000);
	if(ret) {
		HISI_JPU_ERR("fail to wait JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_2!\n");
	}

	ret = hisijpu_check_reg_state(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_0, 0x01000000);
	if(ret) {
		HISI_JPU_ERR("fail to wait JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_0!\n");
	}

	ret = hisijpu_check_reg_state(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_1, 0x01000000);
	if(ret) {
		HISI_JPU_ERR("fail to wait JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_1!\n");
	}

	/* step3,read bit0 is 1*/
	hisijpu_set_reg(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 1, 1, 0);
	ret = hisijpu_check_reg_state(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 0x1);
	if(ret) {
		HISI_JPU_ERR("fail to wait JPGDEC_CRG_CFG1!\n");
	}

	/* step4 */
	hisijpu_set_reg(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_1, 0x0, 1, 25);
	hisijpu_set_reg(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_2, 0x0, 1, 25);
	hisijpu_set_reg(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_0, 0x0, 1, 25);
	hisijpu_set_reg(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_1, 0x0, 1, 25);
	hisijpu_set_reg(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 0x0, 32, 0);

}

void hisijpu_smmu_on(struct hisi_jpu_data_type *hisijd)
{
	uint32_t phy_pgd_base;
	struct iommu_domain_data *domain_data;
	uint32_t fama_ptw_msb;

	if(!hisijd){
		HISI_JPU_ERR("hisijd is NULL!\n");
		return;
	}

	HISI_JPU_DEBUG("+.\n");

#ifdef CONFIG_JPU_SMMU_ENABLE
	/*
	** Set global mode:
	** SMMU_SCR_S.glb_nscfg = 0x3
	** SMMU_SCR_P.glb_prot_cfg = 0x0
	** SMMU_SCR.glb_bypass = 0x0
	*/
	/*set_reg(smmu_base + SMMU_SCR_S, 0x3, 2, 0);*/
	/*set_reg(smmu_base + SMMU_SCR_P, 0x0, 1, 0);*/
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_SCR, 0x0, 1, 0);

	/* for performance Ptw_mid: 0x1d, Ptw_pf: 0x1 */
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_SCR, 0x1, 4, 16);

	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_SCR, 0x1d, 6, 20);

	/*
	** Set interrupt mode:
	** Clear all interrupt state: SMMU_INCLR_NS = 0xFF
	** Enable interrupt: SMMU_INTMASK_NS = 0x0
	*/
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_INTRAW_NS, 0xff, 32, 0);
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_INTMASK_NS, 0x0, 32, 0);

	/*
	** Set non-secure pagetable addr:
	** SMMU_CB_TTBR0 = non-secure pagetable address
	** Set non-secure pagetable type:
	** SMMU_CB_TTBCR.cb_ttbcr_des= 0x1 (long descriptor)
	*/
	if(!g_hisijpu_domain) {
		HISI_JPU_ERR("g_hisijpu_domain null.\n");
		return;
	}
	domain_data = (struct iommu_domain_data *)(g_hisijpu_domain->priv);

	phy_pgd_base = (uint32_t)(domain_data->phy_pgd_base);
	fama_ptw_msb = (domain_data->phy_pgd_base >> 32) & 0x0000007F;

	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_CB_TTBR0, phy_pgd_base, 32, 0);
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_CB_TTBCR, 0x1, 1, 0);

	/* FAMA configuration */
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_FAMA_CTRL0, 0x80, 14, 0);
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_FAMA_CTRL1, fama_ptw_msb, 7, 0);
	HISI_JPU_DEBUG("-.\n");
#else
	/* Global bypass enable of non-secure context bank */
	hisijpu_set_reg(hisijd->jpu_smmu_base + SMMU_SCR, 0x1, 1, 0);
	HISI_JPU_DEBUG("-.\n");
	return;
#endif

}

int hisi_jpu_on(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if(hisijd->power_on) {
		HISI_JPU_DEBUG("hisijd has already power_on!\n");
		return ret;
	}

	/* step 1 mediasubsys enable */
	hisijpu_media1_regulator_enable(hisijd);

	/* step 2 clk_enable */
	hisijpu_clk_enable(hisijd);

	/* step 3 regulator_enable ispsubsys*/
	hisijpu_jpu_regulator_enable(hisijd);

	/* step 4 jpeg decoder inside clock enable */
	outp32(hisijd->jpu_top_base + JPGDEC_CRG_CFG0, 0x1);

	outp32(hisijd->jpu_top_base + JPGDEC_MEM_CFG, 0x02605550);

	hisijpu_smmu_on(hisijd);
	hisi_jpu_dec_interrupt_mask(hisijd);

	hisi_jpu_dec_interrupt_clear(hisijd);

	hisijpu_irq_enable(hisijd);
	hisi_jpu_dec_interrupt_unmask(hisijd);

	hisijd->power_on = true;

	return ret;
}

int hisi_jpu_off(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if(!hisijd->power_on) {
		HISI_JPU_DEBUG("hisijd has already power off!\n");
		return ret;
	}

	hisi_jpu_dec_interrupt_mask(hisijd);
	hisijpu_irq_disable(hisijd);

	/* jpeg decoder inside clock disable */
	outp32(hisijd->jpu_top_base + JPGDEC_CRG_CFG0, 0x0);

	/* ispsubsys regulator disable */
	hisijpu_jpu_regulator_disable(hisijd);

	/* clk disable*/
	hisijpu_clk_disable(hisijd);

	/* media disable */
	hisijpu_media1_regulator_disable(hisijd);

	hisijd->power_on = false;

	return ret;
}

int hisi_jpu_lb_alloc(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;
	size_t lb_size;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	lb_size = 128 * SZ_1K;

	/*alloc jpu dec lb buffer, start end shoulf 32KB align*/
#ifdef CONFIG_JPU_SMMU_ENABLE
	hisijd->lb_ion_handle = ion_alloc(hisijd->ion_client, lb_size,
		0, ION_HEAP(ION_SYSTEM_HEAP_ID), 0);
#else
	hisijd->lb_ion_handle = ion_alloc(hisijd->ion_client, lb_size,
		0, ION_HEAP(ION_GRALLOC_HEAP_ID), 0);
#endif
	if (IS_ERR_OR_NULL(hisijd->lb_ion_handle)) {
		HISI_JPU_ERR("failed to ion alloc lb_ion_handle!");
		ret = -ENOMEM;
		goto err_ion_handle;
	}

	if (!ion_map_kernel(hisijd->ion_client, hisijd->lb_ion_handle)) {
		HISI_JPU_ERR("failed to ion_map_kernel!");
		ret = -ENOMEM;
		goto err_map_kernel;
	}

#ifdef CONFIG_JPU_SMMU_ENABLE
	if(ion_map_iommu(hisijd->ion_client, hisijd->lb_ion_handle, &(hisijd->iommu_format))) {
		ret = -ENOMEM;
		HISI_JPU_ERR("failed to ion_map_iommu!");
		goto err_map_iommu;
	}
	hisijd->lb_addr = (uint32_t)(hisijd->iommu_format.iova_start >> 4);
#else
	if (ion_phys(hisijd->ion_client, hisijd->lb_ion_handle, &hisijd->lb_addr, &lb_size) < 0) {
		HISI_JPU_ERR("failed to get ion phys!\n");
		goto err_map_iommu;
	}
#endif

	HISI_JPU_INFO("lb_size=%zu, hisijd->lb_addr 0x%x\n",lb_size, hisijd->lb_addr);

	return 0;

err_map_iommu:
	if (hisijd->lb_ion_handle) {
		ion_unmap_kernel(hisijd->ion_client, hisijd->lb_ion_handle);
	}

err_map_kernel:
	if (hisijd->lb_ion_handle) {
		ion_free(hisijd->ion_client, hisijd->lb_ion_handle);
		hisijd->lb_ion_handle = NULL;
	}

err_ion_handle:
	return ret;
}

void hisi_jpu_lb_free(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return ;
	}

	if (hisijd->lb_ion_handle) {
	#ifdef CONFIG_JPU_SMMU_ENABLE
		ion_unmap_iommu(hisijd->ion_client, hisijd->lb_ion_handle);
	#endif
		ion_unmap_kernel(hisijd->ion_client, hisijd->lb_ion_handle);
		ion_free(hisijd->ion_client, hisijd->lb_ion_handle);
		hisijd->lb_ion_handle = NULL;
	}
}

/*lint -save -e774 -e438 -e550 -e732*/
int hisi_jpu_dec_reg_default(struct hisi_jpu_data_type *hisijd)
{
	jpu_dec_reg_t *jpu_dec_reg;
	char __iomem *jpu_dec_base;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	jpu_dec_base = hisijd->jpu_dec_base;
	if (!hisijd) {
		HISI_JPU_ERR("jpu_dec_base is NULL!\n");
		return -EINVAL;
	}

	jpu_dec_reg = &(hisijd->jpu_dec_reg_default);

	/* cppcheck-suppress * */
	memset(jpu_dec_reg, 0, sizeof(jpu_dec_reg_t));

	jpu_dec_reg->dec_start = inp32(jpu_dec_base + JPEG_DEC_START);
	jpu_dec_reg->preftch_ctrl = inp32(jpu_dec_base + JPEG_DEC_PREFTCH_CTRL);
	jpu_dec_reg->frame_size = inp32(jpu_dec_base + JPEG_DEC_FRAME_SIZE);
	jpu_dec_reg->crop_horizontal = inp32(jpu_dec_base + JPEG_DEC_CROP_HORIZONTAL);
	jpu_dec_reg->crop_vertical = inp32(jpu_dec_base + JPEG_DEC_CROP_VERTICAL);
	jpu_dec_reg->bitstreams_start = inp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_START);
	jpu_dec_reg->bitstreams_end = inp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_END);
	jpu_dec_reg->frame_start_y = inp32(jpu_dec_base + JPEG_DEC_FRAME_START_Y);
	jpu_dec_reg->frame_stride_y = inp32(jpu_dec_base + JPEG_DEC_FRAME_STRIDE_Y);
	jpu_dec_reg->frame_start_c = inp32(jpu_dec_base + JPEG_DEC_FRAME_START_C);
	jpu_dec_reg->frame_stride_c = inp32(jpu_dec_base + JPEG_DEC_FRAME_STRIDE_C);
	jpu_dec_reg->lbuf_start_addr = inp32(jpu_dec_base + JPEG_DEC_LBUF_START_ADDR);
	jpu_dec_reg->output_type = inp32(jpu_dec_base + JPEG_DEC_OUTPUT_TYPE);
	jpu_dec_reg->freq_scale = inp32(jpu_dec_base + JPEG_DEC_FREQ_SCALE);
	jpu_dec_reg->middle_filter = inp32(jpu_dec_base + JPEG_DEC_MIDDLE_FILTER);
	jpu_dec_reg->sampling_factor = inp32(jpu_dec_base + JPEG_DEC_SAMPLING_FACTOR);
	jpu_dec_reg->dri = inp32(jpu_dec_base + JPEG_DEC_DRI);
	jpu_dec_reg->over_time_thd = inp32(jpu_dec_base + JPEG_DEC_OVER_TIME_THD);
	jpu_dec_reg->hor_phase0_coef01 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF01);
	jpu_dec_reg->hor_phase0_coef23 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF23);
	jpu_dec_reg->hor_phase0_coef45 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF45);
	jpu_dec_reg->hor_phase0_coef67 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF67);
	jpu_dec_reg->hor_phase2_coef01 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF01);
	jpu_dec_reg->hor_phase2_coef23 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF23);
	jpu_dec_reg->hor_phase2_coef45 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF45);
	jpu_dec_reg->hor_phase2_coef67 = inp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF67);
	jpu_dec_reg->ver_phase0_coef01 = inp32(jpu_dec_base + JPEG_DEC_VER_PHASE0_COEF01);
	jpu_dec_reg->ver_phase0_coef23 = inp32(jpu_dec_base + JPEG_DEC_VER_PHASE0_COEF23);
	jpu_dec_reg->ver_phase2_coef01 = inp32(jpu_dec_base + JPEG_DEC_VER_PHASE2_COEF01);
	jpu_dec_reg->ver_phase2_coef23 = inp32(jpu_dec_base + JPEG_DEC_VER_PHASE2_COEF23);
	jpu_dec_reg->csc_in_dc_coef = inp32(jpu_dec_base + JPEG_DEC_CSC_IN_DC_COEF);
	jpu_dec_reg->csc_out_dc_coef = inp32(jpu_dec_base + JPEG_DEC_CSC_OUT_DC_COEF);
	jpu_dec_reg->csc_trans_coef0 = inp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF0);
	jpu_dec_reg->csc_trans_coef1 = inp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF1);
	jpu_dec_reg->csc_trans_coef2 = inp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF2);
	jpu_dec_reg->csc_trans_coef3 = inp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF3);
	jpu_dec_reg->csc_trans_coef4 = inp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF4);

	return 0;
}

int hisi_jpu_dec_reg_init(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	// cppcheck-suppress *
	memcpy(&(hisijd->jpu_dec_reg), &(hisijd->jpu_dec_reg_default), sizeof(jpu_dec_reg_t));

	return 0;
}

int hisi_jpu_dec_set_cvdr(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_CVDR_CFG, 0x070f2000);

	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_0, 0x80000000);
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_WR_CFG_1, 0x80000000);

	/* Wr_qos_max:0x1;wr_qos_threshold_01_start:0x1;wr_qos_threshold_01_stop:0x1,WR_QOS&RD_QOS encode will also set this */
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_CVDR_WR_QOS_CFG, 0x10333311);
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_CVDR_RD_QOS_CFG, 0x10333311);

	/* NRRD1 */
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_LIMITER_NR_RD_1, 0xf002222);
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_1, 0x80060080);

	/* NRRD2 */
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_LIMITER_NR_RD_2, 0xf003333);
	outp32(hisijd->jpu_cvdr_base + JPGDEC_CVDR_AXI_JPEG_NR_RD_CFG_2, 0x80060080);
	return 0;
}

int hisi_jpu_dec_set_unreset(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	/* module reset */
	outp32(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 0x1);
	outp32(hisijd->jpu_top_base + JPGDEC_CRG_CFG1, 0x0);

	return 0;
}

int hisi_jpu_dec_set_reg(struct hisi_jpu_data_type *hisijd, jpu_dec_reg_t *jpu_dec_reg)
{
	char __iomem *jpu_dec_base;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (!jpu_dec_reg) {
		HISI_JPU_ERR("jpu_dec_reg is NULL!\n");
		return -EINVAL;
	}

	jpu_dec_base = hisijd->jpu_dec_base;
	if (!hisijd) {
		HISI_JPU_ERR("jpu_dec_base is NULL!\n");
		return -EINVAL;
	}

	if (g_debug_jpu_dec) {
		HISI_JPU_INFO("jpu reg val:\n"
		"dec_start=%d\n"
		"preftch_ctrl=%d\n"
		"frame_size=%d\n"
		"crop_horizontal=%d\n"
		"crop_vertical=%d\n"
		"bitstreams_start=%d\n"
		"bitstreams_end=%d\n"
		"frame_start_y=%d\n"
		"frame_stride_y=%d\n"
		"frame_start_c=%d\n"
		"frame_stride_c=%d\n"
		"lbuf_start_addr=%d\n"
		"output_type=%d\n"
		"freq_scale=%d\n"
		"middle_filter=%d\n"
		"sampling_factor=%d\n"
		"dri=%d\n"
		"over_time_thd=%d\n"
		"hor_phase0_coef01=%d\n"
		"hor_phase0_coef23=%d\n"
		"hor_phase0_coef45=%d\n"
		"hor_phase0_coef67=%d\n"
		"hor_phase2_coef01=%d\n"
		"hor_phase2_coef23=%d\n"
		"hor_phase2_coef45=%d\n"
		"hor_phase2_coef67=%d\n"
		"ver_phase0_coef01=%d\n"
		"ver_phase0_coef23=%d\n"
		"ver_phase2_coef01=%d\n"
		"ver_phase2_coef23=%d\n"
		"csc_in_dc_coef=%d\n"
		"csc_out_dc_coef=%d\n"
		"csc_trans_coef0=%d\n"
		"csc_trans_coef1=%d\n"
		"csc_trans_coef2=%d\n"
		"csc_trans_coef3=%d\n"
		"csc_trans_coef4=%d\n",
		jpu_dec_reg->dec_start,
		jpu_dec_reg->preftch_ctrl,
		jpu_dec_reg->frame_size,
		jpu_dec_reg->crop_horizontal,
		jpu_dec_reg->crop_vertical,
		jpu_dec_reg->bitstreams_start,
		jpu_dec_reg->bitstreams_end,
		jpu_dec_reg->frame_start_y,
		jpu_dec_reg->frame_stride_y,
		jpu_dec_reg->frame_start_c,
		jpu_dec_reg->frame_stride_c,
		jpu_dec_reg->lbuf_start_addr,
		jpu_dec_reg->output_type,
		jpu_dec_reg->freq_scale,
		jpu_dec_reg->middle_filter,
		jpu_dec_reg->sampling_factor,
		jpu_dec_reg->dri,
		jpu_dec_reg->over_time_thd,
		jpu_dec_reg->hor_phase0_coef01,
		jpu_dec_reg->hor_phase0_coef23,
		jpu_dec_reg->hor_phase0_coef45,
		jpu_dec_reg->hor_phase0_coef67,
		jpu_dec_reg->hor_phase2_coef01,
		jpu_dec_reg->hor_phase2_coef23,
		jpu_dec_reg->hor_phase2_coef45,
		jpu_dec_reg->hor_phase2_coef67,
		jpu_dec_reg->ver_phase0_coef01,
		jpu_dec_reg->ver_phase0_coef23,
		jpu_dec_reg->ver_phase2_coef01,
		jpu_dec_reg->ver_phase2_coef23,
		jpu_dec_reg->csc_in_dc_coef,
		jpu_dec_reg->csc_out_dc_coef,
		jpu_dec_reg->csc_trans_coef0,
		jpu_dec_reg->csc_trans_coef1,
		jpu_dec_reg->csc_trans_coef2,
		jpu_dec_reg->csc_trans_coef3,
		jpu_dec_reg->csc_trans_coef4);
	}

	outp32(jpu_dec_base + JPEG_DEC_PREFTCH_CTRL, jpu_dec_reg->preftch_ctrl);
	outp32(jpu_dec_base + JPEG_DEC_FRAME_SIZE, jpu_dec_reg->frame_size);
	outp32(jpu_dec_base + JPEG_DEC_CROP_HORIZONTAL, jpu_dec_reg->crop_horizontal);
	outp32(jpu_dec_base + JPEG_DEC_CROP_VERTICAL, jpu_dec_reg->crop_vertical);
	outp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_START, jpu_dec_reg->bitstreams_start);
	outp32(jpu_dec_base + JPEG_DEC_BITSTREAMS_END, jpu_dec_reg->bitstreams_end);
	outp32(jpu_dec_base + JPEG_DEC_FRAME_START_Y, jpu_dec_reg->frame_start_y);
	outp32(jpu_dec_base + JPEG_DEC_FRAME_STRIDE_Y, jpu_dec_reg->frame_stride_y);
	outp32(jpu_dec_base + JPEG_DEC_FRAME_START_C, jpu_dec_reg->frame_start_c);
	outp32(jpu_dec_base + JPEG_DEC_FRAME_STRIDE_C, jpu_dec_reg->frame_stride_c);
	outp32(jpu_dec_base + JPEG_DEC_LBUF_START_ADDR, jpu_dec_reg->lbuf_start_addr);
	outp32(jpu_dec_base + JPEG_DEC_OUTPUT_TYPE, jpu_dec_reg->output_type);
	outp32(jpu_dec_base + JPEG_DEC_FREQ_SCALE, jpu_dec_reg->freq_scale);
	outp32(jpu_dec_base + JPEG_DEC_MIDDLE_FILTER, jpu_dec_reg->middle_filter);
	outp32(jpu_dec_base + JPEG_DEC_SAMPLING_FACTOR, jpu_dec_reg->sampling_factor);
	outp32(jpu_dec_base + JPEG_DEC_DRI, jpu_dec_reg->dri);
	outp32(jpu_dec_base + JPEG_DEC_OVER_TIME_THD, jpu_dec_reg->over_time_thd);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF01, jpu_dec_reg->hor_phase0_coef01);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF23, jpu_dec_reg->hor_phase0_coef23);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF45, jpu_dec_reg->hor_phase0_coef45);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE0_COEF67, jpu_dec_reg->hor_phase0_coef67);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF01, jpu_dec_reg->hor_phase2_coef01);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF23, jpu_dec_reg->hor_phase2_coef23);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF45, jpu_dec_reg->hor_phase2_coef45);
	outp32(jpu_dec_base + JPEG_DEC_HOR_PHASE2_COEF67, jpu_dec_reg->hor_phase2_coef67);
	outp32(jpu_dec_base + JPEG_DEC_VER_PHASE0_COEF01, jpu_dec_reg->ver_phase0_coef01);
	outp32(jpu_dec_base + JPEG_DEC_VER_PHASE0_COEF23, jpu_dec_reg->ver_phase0_coef23);
	outp32(jpu_dec_base + JPEG_DEC_VER_PHASE2_COEF01, jpu_dec_reg->ver_phase2_coef01);
	outp32(jpu_dec_base + JPEG_DEC_VER_PHASE2_COEF23, jpu_dec_reg->ver_phase2_coef23);
	outp32(jpu_dec_base + JPEG_DEC_CSC_IN_DC_COEF, jpu_dec_reg->csc_in_dc_coef);
	outp32(jpu_dec_base + JPEG_DEC_CSC_OUT_DC_COEF, jpu_dec_reg->csc_out_dc_coef);
	outp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF0, jpu_dec_reg->csc_trans_coef0);
	outp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF1, jpu_dec_reg->csc_trans_coef1);
	outp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF2, jpu_dec_reg->csc_trans_coef2);
	outp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF3, jpu_dec_reg->csc_trans_coef3);
	outp32(jpu_dec_base + JPEG_DEC_CSC_TRANS_COEF4, jpu_dec_reg->csc_trans_coef4);

	/* to make sure start reg set at last */
	outp32(jpu_dec_base + JPEG_DEC_START, 0x80000001);

	return 0;
}
/*lint -restore*/

void hisijpu_dump_info(struct jpu_data*  jpu_info)
{
	uint32_t i;
	int j;

	if(!jpu_info) {
		return;
	}

	HISI_JPU_INFO(" informat %d,outformat %d \n",
		jpu_info->in_img_format,jpu_info->out_color_format);

	HISI_JPU_INFO("input buffer: [w:%d h:%d align_w:%d align_h:%d\n"
		"num_compents:%d\n"
		"sample_rate:%d\n"
		"start_addr:%llx\n"
		"end_addr::%llx\n",
		jpu_info->inwidth, jpu_info->inheight, jpu_info->pix_width, jpu_info->pix_height,
		jpu_info->num_components,
		jpu_info->sample_rate,
		jpu_info->start_addr,
		jpu_info->end_addr);

	HISI_JPU_INFO("out buffer: start_addr_y:%llx\n"
		"last_page_y::%llx\n"
		"start_addr_c::%llx\n"
		"last_page_c::%llx\n"
		"stride_y :%d\n"
		"stride_c:%d\n"
		"restart_interval:%d\n",
		jpu_info->start_addr_y,
		jpu_info->last_page_y,
		jpu_info->start_addr_c,
		jpu_info->last_page_c,
		jpu_info->stride_y,
		jpu_info->stride_c,
		jpu_info->restart_interval);

	HISI_JPU_INFO("region[ %d %d %d %d]\n",
		jpu_info->region_info.left,jpu_info->region_info.right,
		jpu_info->region_info.top, jpu_info->region_info.bottom);

	for(i = 0; i < jpu_info->num_components; i++) {
		HISI_JPU_INFO("i=%u\n"
			"s32ComponentId:%d\n"
			"s32ComponentIndex:%d\n"
			"s32QuantTblNo:%d\n"
			"s32DcTblNo:%d\n"
			"s32AcTblNo:%d\n"
			"u8HorSampleFac:%d\n"
			"u8VerSampleFac:%d\n",
			i,
			jpu_info->component_info[i].s32ComponentId,
			jpu_info->component_info[i].s32ComponentIndex,
			jpu_info->component_info[i].s32QuantTblNo,
			jpu_info->component_info[i].s32DcTblNo,
			jpu_info->component_info[i].s32AcTblNo,
			jpu_info->component_info[i].u8HorSampleFac,
			jpu_info->component_info[i].u8VerSampleFac);
	}

	for(j = 0; j < HDC_TABLE_NUM; j++) {
		HISI_JPU_DEBUG("hdc_tab %ul\n", jpu_info->hdc_tab[j]);
	}

	for(i = 0; i < HAC_TABLE_NUM; i++) {
		HISI_JPU_DEBUG("i:%d hac_min_tab:%ul\n", i, jpu_info->hac_min_tab[i]);
		HISI_JPU_DEBUG("i:%d hac_base_tab:%ul\n", i, jpu_info->hac_base_tab[i]);
	}

	for(j = 0; j < HAC_SYMBOL_TABLE_NUM; j++) {
		HISI_JPU_DEBUG(":hdc_tab %ul\n", jpu_info->hac_symbol_tab[j]);
	}

	for(j = 0; j < MAX_DCT_SIZE; j++) {
		HISI_JPU_DEBUG("qvalue %d\n", jpu_info->qvalue[j]);
	}
 }

int hisijpu_check_inputbuffer(jpu_data_t *jpu_req)
{
	if (!jpu_req) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if ((jpu_req->sample_rate >= HISI_JPEG_DECODE_SAMPLE_SIZE_MAX)
		|| (HISI_JPEG_DECODE_SAMPLE_SIZE_1 > jpu_req->sample_rate)) {
		HISI_JPU_ERR("sample_rate(%d) is out of range!\n",
			jpu_req->sample_rate);
		return -EINVAL;
	}

	if (((jpu_req->inwidth/jpu_req->sample_rate) < MIN_INPUT_WIDTH)
		|| (jpu_req->inwidth > MAX_INPUT_WIDTH)
		||((jpu_req->inheight/jpu_req->sample_rate) < MIN_INPUT_HEIGHT)
		|| (jpu_req->inheight > MAX_INPUT_HEIGHT)) {
		HISI_JPU_ERR("the image inwidth=%d, inheight=%d ,sample_rate %d is out of range!\n",
			jpu_req->inwidth, jpu_req->inheight, jpu_req->sample_rate);
		return -EINVAL;
	}

	return 0;
}

int hisijpu_check_outbuffer(jpu_data_t *jpu_req)
{
	uint32_t out_addr_align = 0;
	uint32_t out_format;
	uint32_t stride_align_mask = 0;

	if (!jpu_req) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if (jpu_req->out_color_format < 0) {
		HISI_JPU_ERR("jpu_req out_color_format is %d not support!\n", jpu_req->out_color_format);
		return -EINVAL;
	}

	/* jpu_req->pix_width; aligned to mcu width */
	/* jpu_req->pix_height; aligned to mcu height */
	/*lint -save -e571*/
	out_format = (uint32_t)jpu_req->out_color_format;
	/*lint -restore*/

	if (isRGB_out(out_format)) {
		out_addr_align = JPU_OUT_RGB_ADDR_ALIGN;
		stride_align_mask = JPU_OUT_STRIDE_ALIGN - 1;
	} else {
		out_addr_align = JPU_OUT_YUV_ADDR_ALIGN;
		stride_align_mask = JPU_OUT_STRIDE_ALIGN/2 - 1;
	}

	/*
	** start address for planar Y or RGB, unit is 16 byte, must align to 64 byte (YUV format) or 128 byte (RGB format)
	** stride for planar Y or RGB, unit is 16 byte, must align to 64 byte (YUV format) or 128 byte (RGB format)
	*/
	if ((jpu_req->start_addr_y > JPU_MAX_ADDR)
		|| (jpu_req->start_addr_y & (out_addr_align - 1))) {
		HISI_JPU_ERR("start_addr_y(%llu) is not %d bytes aligned!\n",
			jpu_req->start_addr_y, out_addr_align - 1);
		return -EINVAL;
	}

	if ((jpu_req->stride_y < JPU_MIN_STRIDE)
		||(jpu_req->stride_y > JPU_MAX_STRIDE)
		|| (jpu_req->stride_y & stride_align_mask)) {
		HISI_JPU_ERR("stride_y(%d) is not %d bytes aligned!\n",
			jpu_req->stride_y, stride_align_mask);
		return -EINVAL;
	}

	/*
	** start address for planar UV, unit is 16 byte, must align to 64 byte
	** stride for planar UV, unit is 16 byte, must align to 64 byte
	*/
	if ((jpu_req->start_addr_c > JPU_MAX_ADDR)
		||(jpu_req->start_addr_c & (out_addr_align - 1))) {
		HISI_JPU_ERR("start_addr_c(%llu) is not %d bytes aligned!\n",
			jpu_req->start_addr_c, out_addr_align - 1);
		return -EINVAL;
	}

	if ((jpu_req->stride_c > JPU_MAX_STRIDE/2)
		|| (jpu_req->stride_c & stride_align_mask)) {
		HISI_JPU_ERR("stride_c(%d) is not %d bytes aligned!\n",
			jpu_req->stride_c, stride_align_mask);
		return -EINVAL;
	}

	return 0;
}

int hisijpu_check_region_decode_info(jpu_data_t *jpu_req)
{
	if (!jpu_req) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if (jpu_req->decode_mode < HISI_JPEG_DECODE_MODE_REGION) {
		return 0;
	}

	if ((jpu_req->region_info.left >= jpu_req->region_info.right)
		|| (jpu_req->region_info.top >= jpu_req->region_info.bottom)
		|| ((((jpu_req->region_info.right - jpu_req->region_info.left) + 1) /jpu_req->sample_rate) < MIN_INPUT_WIDTH)
		|| ((((jpu_req->region_info.bottom- jpu_req->region_info.top) + 1) /jpu_req->sample_rate) < MIN_INPUT_HEIGHT)) {
		HISI_JPU_ERR("region[%d %d %d %d] invalid sample_rate %d!\n",
			jpu_req->region_info.left, jpu_req->region_info.right,
			jpu_req->region_info.top, jpu_req->region_info.bottom,
			jpu_req->sample_rate);
		return -EINVAL;
	}

	if(HISI_JPEG_DECODE_OUT_YUV422_H2V1 == jpu_req->out_color_format) {
		if((((jpu_req->region_info.right - jpu_req->region_info.left) + 1)/jpu_req->sample_rate) % 2) {
			HISI_JPU_ERR("region[%d %d] width invalid!\n",
				jpu_req->region_info.left, jpu_req->region_info.right);
			return -EINVAL;
		}
	}

	if((HISI_JPEG_DECODE_OUT_YUV420 == jpu_req->out_color_format)
		|| (HISI_JPEG_DECODE_OUT_YUV444 == jpu_req->out_color_format)) {
		if(((((jpu_req->region_info.right - jpu_req->region_info.left) + 1)/jpu_req->sample_rate) % 2)
			|| ((((jpu_req->region_info.bottom - jpu_req->region_info.top) + 1)/jpu_req->sample_rate) % 2)) {
			HISI_JPU_ERR("region[%d %d %d %d] width or height invalid, jpu_req->sample_rate %d!\n",
				jpu_req->region_info.left, jpu_req->region_info.right,
				jpu_req->region_info.top, jpu_req->region_info.bottom,
				jpu_req->sample_rate);
			return -EINVAL;
		}
	}

	/* 440 height should be even num*/
	if(HISI_JPEG_DECODE_OUT_YUV422_H1V2 == jpu_req->out_color_format) {
		if((((jpu_req->region_info.bottom - jpu_req->region_info.top) + 1)/jpu_req->sample_rate) % 2) {
			HISI_JPU_ERR("region[%d %d %d %d] height invalid, sample_rate %d!\n",
				jpu_req->region_info.left, jpu_req->region_info.right,
				jpu_req->region_info.top, jpu_req->region_info.bottom,
				jpu_req->sample_rate);
			return -EINVAL;
		}
	}

	return 0;
}

int hisijpu_check_full_decode_info(jpu_data_t *jpu_req)
{
	if (!jpu_req) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if (HISI_JPEG_DECODE_MODE_FULL_SUB < jpu_req->decode_mode) {
		HISI_JPU_INFO("decode_mode %d\n", jpu_req->decode_mode);
		return 0;
	}

	if(HISI_JPEG_DECODE_OUT_YUV422_H2V1 == jpu_req->out_color_format) {
		if((jpu_req->pix_width/jpu_req->sample_rate) % 2) {
			HISI_JPU_ERR("out_color_format %d, jpu_req->pix_width %d invalid!\n",
				jpu_req->out_color_format,jpu_req->pix_width);
			return -EINVAL;
		}
	}

	if(HISI_JPEG_DECODE_OUT_YUV420 == jpu_req->out_color_format) {
		if(((jpu_req->pix_width/jpu_req->sample_rate) % 2) ||
			((jpu_req->pix_height/jpu_req->sample_rate) % 2)) {
			HISI_JPU_ERR("out_color_format %d, jpu_req->pix_width %d invalid!\n",
				jpu_req->out_color_format,jpu_req->pix_width);
			return -EINVAL;
		}
	}

	if(HISI_JPEG_DECODE_OUT_YUV422_H1V2 == jpu_req->out_color_format) {
		if((jpu_req->pix_height/jpu_req->sample_rate) % 2) {
			HISI_JPU_ERR("out_color_format %d, jpu_req->pix_height %d invalid!\n",
				jpu_req->out_color_format,jpu_req->pix_height);
			return -EINVAL;
		}
	}

	return 0;
}

int hisijpu_check_format(jpu_data_t *jpu_req)
{
	if (!jpu_req) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if (jpu_req->out_color_format != HISI_JPEG_DECODE_OUT_YUV420) {
		return 0;
	}

	if ((HISI_JPEG_DECODE_RAW_YUV422_H2V1 == jpu_req->in_img_format)
		|| (HISI_JPEG_DECODE_RAW_YUV444 == jpu_req->in_img_format)){
		if (HISI_JPEG_DECODE_SAMPLE_SIZE_8 == jpu_req->sample_rate) {
			HISI_JPU_INFO("sample_rate %d, in_img_format %d\n", jpu_req->sample_rate, jpu_req->in_img_format);
			return -1;
		}
	}

	return 0;
}

int hisijpu_check_userdata(struct hisi_jpu_data_type *hisijd, jpu_data_t *jpu_req)
{
	int ret;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (!jpu_req) {
		HISI_JPU_ERR("jpu_req is NULL!\n");
		return -EINVAL;
	}

	if ((jpu_req->num_components == 0) ||
		(jpu_req->num_components > PIXEL_COMPONENT_NUM)) {
		HISI_JPU_ERR("the num_components %d is out of range!\n",
			jpu_req->num_components);
		return -EINVAL;
	}

	ret = hisijpu_check_inputbuffer(jpu_req);
	if(ret) {
		HISI_JPU_ERR("check input buffer error!\n");
		return -EINVAL;
	}

	/* start address for line buffer, unit is 16 byte, must align to 128 byte */
	if (hisijd->lb_addr & (JPU_LB_ADDR_ALIGN - 1)) {
		HISI_JPU_ERR("lb_addr(0x%x) is not %d bytes aligned!\n",
			hisijd->lb_addr, JPU_LB_ADDR_ALIGN - 1);
		return -EINVAL;
	}

	if (jpu_req->decode_mode >= HISI_JPEG_DECODE_MODE_MAX) {
		HISI_JPU_ERR("the image decode_mode=%d is out of range!\n",
			jpu_req->decode_mode);
		return -EINVAL;
	}

	if (jpu_req->progressive_mode) {
		HISI_JPU_ERR("not support progressive mode!\n");
		return -EINVAL;
	}

	if (jpu_req->arith_code) {
		HISI_JPU_ERR("not support arith_code mode!\n");
		return -EINVAL;
	}

	if (jpu_req->out_color_format >= HISI_JPEG_DECODE_OUT_FORMAT_MAX) {
		HISI_JPU_ERR("out_color_format(%d) is out of range!\n",
		jpu_req->out_color_format);
		return -EINVAL;
	}
	/* is format can handle for chip limit*/
	ret = hisijpu_check_format(jpu_req);
	if (ret) {
		HISI_JPU_ERR("this format not support 8 sample\n");
		return -EINVAL;
	}

	ret = hisijpu_check_outbuffer(jpu_req);
	if (ret) {
		HISI_JPU_ERR("check outbuffer error\n");
		return -EINVAL;
	}

	ret = hisijpu_check_region_decode_info(jpu_req);
	if (ret) {
		HISI_JPU_ERR("check region decode info error\n");
		return -EINVAL;
	}

	ret = hisijpu_check_full_decode_info(jpu_req);
	if (ret) {
		HISI_JPU_ERR("check full decode info error\n");
		return -EINVAL;
	}

	if (g_debug_jpu_dec) {
		hisijpu_dump_info(jpu_req);
	}

	return 0;
}

int hisijpu_job_exec(struct hisi_jpu_data_type *hisijd, void __user *argp)
{
	int ret;
	unsigned long ret1;
	jpu_data_t *jpu_req;
	jpu_dec_reg_t *pjpu_dec_reg;
	struct timeval tv0 = {0};
	struct timeval tv1 = {0};
	long timediff = 0;

	uint8_t yfac = 0;
	uint8_t ufac = 0;
	uint8_t vfac = 0;
	int freq_scale = 0;
	int out_format = 0;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_JPU_ERR("argp is NULL!\n");
		return -EINVAL;
	}

	HISI_JPU_DEBUG("+.\n");

	jpu_req = &(hisijd->jpu_req);
	pjpu_dec_reg = &(hisijd->jpu_dec_reg);


	down(&hisijd->blank_sem);
	hisi_jpu_on(hisijd);

	ret1 = copy_from_user(jpu_req, argp, sizeof(jpu_data_t));
	if (ret1) {
		HISI_JPU_ERR("copy_from_user failed!\n");
		ret = -EINVAL;
		goto err_out;
	}

	ret = hisijpu_check_userdata(hisijd, jpu_req);
	if (ret) {
		HISI_JPU_ERR("hisijpu_check_userdata failed!\n");
		ret = -EINVAL;
		goto err_out;
	}

	if (!hisijd->jpu_res_initialized) {
		hisi_jpu_dec_reg_default(hisijd);
		hisijd->jpu_res_initialized = true;
	}

	hisi_jpu_dec_reg_init(hisijd);
	hisi_jpu_dec_set_cvdr(hisijd);
	hisi_jpu_dec_set_unreset(hisijd);

	/* prefetch control */
	pjpu_dec_reg->preftch_ctrl = jpu_set_bits32(pjpu_dec_reg->preftch_ctrl,
		jpu_req->smmu_enable ? 0x0 : 0x1, 1, 0);
	/* define reset interval */
	pjpu_dec_reg->dri = jpu_set_bits32(pjpu_dec_reg->dri,
		jpu_req->restart_interval, 32, 0);

	/* frame size */
	pjpu_dec_reg->frame_size = jpu_set_bits32(pjpu_dec_reg->frame_size,
		((jpu_req->pix_width - 1) | ((jpu_req->pix_height - 1) << 16)), 32, 0);

	/* input bitstreams addr */
	pjpu_dec_reg->bitstreams_start = jpu_set_bits32(pjpu_dec_reg->bitstreams_start,
		(uint32_t)jpu_req->start_addr, 32, 0);
	pjpu_dec_reg->bitstreams_end = jpu_set_bits32(pjpu_dec_reg->bitstreams_end,
		(uint32_t)jpu_req->end_addr, 32, 0);

	/* output buffer addr */
	pjpu_dec_reg->frame_start_y = jpu_set_bits32(pjpu_dec_reg->frame_start_y,
		(uint32_t)jpu_req->start_addr_y, 28, 0);
	pjpu_dec_reg->frame_stride_y = jpu_set_bits32(pjpu_dec_reg->frame_stride_y,
		jpu_req->stride_y | ((uint32_t)(jpu_req->last_page_y << 12)), 29, 0);
	pjpu_dec_reg->frame_start_c = jpu_set_bits32(pjpu_dec_reg->frame_start_c,
		(uint32_t)jpu_req->start_addr_c, 28, 0);
	pjpu_dec_reg->frame_stride_c = jpu_set_bits32(pjpu_dec_reg->frame_stride_c,
		jpu_req->stride_c | ((uint32_t)(jpu_req->last_page_c << 12)), 29, 0);

	/* start address for line buffer,unit is 16 byte, must align to 128 byte */
	pjpu_dec_reg->lbuf_start_addr = jpu_set_bits32(pjpu_dec_reg->lbuf_start_addr,
		hisijd->lb_addr, 28, 0);

	/* output type , should compare with input format*/
	out_format = hisi_out_format_hal2jpu(hisijd);
	if (out_format < 0) {
		HISI_JPU_ERR("hisi_out_format_hal2jpu failed!\n");
		ret = -EINVAL;
		goto err_out;
	}

	pjpu_dec_reg->output_type = jpu_set_bits32(pjpu_dec_reg->output_type,
		(uint32_t)out_format, 16, 0);

	/* frequence scale */
	freq_scale = hisi_sample_size_hal2jpu(jpu_req->sample_rate);
	if (freq_scale < 0) {
		HISI_JPU_ERR("hisi_sample_size_hal2jpu failed!\n");
		ret = -EINVAL;
		goto err_out;
	}
	pjpu_dec_reg->freq_scale = jpu_set_bits32(pjpu_dec_reg->freq_scale,
		(uint32_t)freq_scale, 2, 0);

	/* for  decode region*/
	ret = hisijpu_set_crop(hisijd);
	if (ret) {
		HISI_JPU_ERR("hisijpu_set_crop ret = %d\n", ret);
		goto err_out;
	}

	/* MIDDLE FITER can use default value */

	/* sampling factor */
	yfac = (((jpu_req->component_info[0].u8HorSampleFac << 4) |
		jpu_req->component_info[0].u8VerSampleFac) & 0xff);
	ufac = (((jpu_req->component_info[1].u8HorSampleFac << 4) |
		jpu_req->component_info[1].u8VerSampleFac) & 0xff);
	vfac = (((jpu_req->component_info[2].u8HorSampleFac << 4) |
		jpu_req->component_info[2].u8VerSampleFac) & 0xff);
	pjpu_dec_reg->sampling_factor = jpu_set_bits32(pjpu_dec_reg->sampling_factor,
		(vfac | (ufac << 8) | (yfac << 16)), 24, 0);

	/* set dqt table */
	hisijpu_set_dqt(jpu_req, hisijd->jpu_dec_base);

	/* set dht table */
	hisijpu_set_dht(jpu_req, hisijd->jpu_dec_base);

	/* set over_time */

	/* start to work */
	pjpu_dec_reg->dec_start = jpu_set_bits32(pjpu_dec_reg->dec_start, 0x1, 1, 0);

	ret = hisi_jpu_dec_set_reg(hisijd, pjpu_dec_reg);
	if (ret) {
		HISI_JPU_ERR("hisi_jpu_dec_set_reg ret = %d\n", ret);
	}

	if (g_debug_jpu_dec_job_timediff)
		jpu_get_timestamp(&tv0);

	ret = hisi_jpu_dec_done_config(hisijd);
	if (ret != 0) {
		HISI_JPU_ERR("hisi_jpu_dec_done_config failed! ret = %d\n", ret);
	}

err_out:
	hisi_jpu_off(hisijd);
	up(&hisijd->blank_sem);

	if (g_debug_jpu_dec_job_timediff) {
		jpu_get_timestamp(&tv1);
		timediff = jpu_timestamp_diff(&tv0, &tv1);
		HISI_JPU_INFO("jpu job exec timediff is %ld us!", timediff);
	}

	HISI_JPU_DEBUG("-.\n");

	return ret;
}
