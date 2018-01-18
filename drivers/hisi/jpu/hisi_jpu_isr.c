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

#include "hisi_jpu.h"


static void hisi_jpu_dec_done_isr_handler(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return;
	}

	hisijd->jpu_dec_done_flag = 1;

	wake_up_interruptible_all(&hisijd->jpu_dec_done_wq);
}

int hisi_jpu_dec_done_config(struct hisi_jpu_data_type *hisijd)
{
	int ret = 0;
	uint32_t timeout_interval = 0;
	int times = 0;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -1;
	}

	if (hisijd->fpga_flag == 0) {
		timeout_interval = JPU_DEC_DONE_TIMEOUT_THRESHOLD_ASIC;
	} else {
		timeout_interval = JPU_DEC_DONE_TIMEOUT_THRESHOLD_FPGA;
	}

REDO_0:
	/*lint -save -e665 -e666 -e40 -e578 -e712 -e713 -e747 -e774 -e778 -e845*/
	ret = wait_event_interruptible_timeout(hisijd->jpu_dec_done_wq,
		hisijd->jpu_dec_done_flag, (unsigned long)msecs_to_jiffies(timeout_interval));
	if (ret == -ERESTARTSYS) {
		if (times < 50) {
			times++;
			mdelay(10);
			goto REDO_0;
		}
	}
	/*lint -restore*/

	hisijd->jpu_dec_done_flag = 0;

	if (ret <= 0) {
		HISI_JPU_ERR("wait_for jpu_dec_done_flag timeout!ret=%d, jpu_dec_done_flag=%d.\n",
			ret, hisijd->jpu_dec_done_flag);

		ret = -ETIMEDOUT;
	} else {
		HISI_JPU_INFO("finish decode jpu_dec_done_flag=%d.\n", hisijd->jpu_dec_done_flag);
		hisi_jpu_dec_normal_reset(hisijd);
		ret = 0;
	}

	return ret;
}

int hisi_jpu_dec_err_config(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	if (g_debug_jpu_dec) {
		HISI_JPU_ERR("jpu decode err!\n");
	}

	hisi_jpu_dec_error_reset(hisijd);

	return 0;
}

int hisi_jpu_dec_other_config(struct hisi_jpu_data_type *hisijd)
{
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return -EINVAL;
	}

	hisi_jpu_dec_error_reset(hisijd);

	return 0;
}

/*lint -save -e438 -e550 -e715*/
irqreturn_t hisi_jpu_dec_done_isr(int irq, void *ptr)
{
	struct hisi_jpu_data_type *hisijd;
	char __iomem *jpu_top_base;
	int isr_s1 = 0;

	hisijd = (struct hisi_jpu_data_type *)ptr;
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		goto err_out;
	}

	jpu_top_base = hisijd->jpu_top_base;

	isr_s1 = inp32(jpu_top_base + JPGDEC_IRQ_REG2);
	outp32(jpu_top_base + JPGDEC_IRQ_REG0, 0x1);

	hisi_jpu_dec_done_isr_handler(hisijd);

err_out:
	return IRQ_HANDLED;
}

irqreturn_t hisi_jpu_dec_err_isr(int irq, void *ptr)
{
	struct hisi_jpu_data_type *hisijd;
	char __iomem *jpu_jpu_top_base;
	int isr_s1;

	hisijd = (struct hisi_jpu_data_type *)ptr;
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		goto err_out;
	}

	jpu_jpu_top_base = hisijd->jpu_top_base;

	isr_s1 = inp32(jpu_jpu_top_base + JPGDEC_IRQ_REG2);
	outp32(jpu_jpu_top_base + JPGDEC_IRQ_REG0, 0x4);

	hisi_jpu_dec_err_config(hisijd);

	err_out:
	return IRQ_HANDLED;
}

irqreturn_t hisi_jpu_dec_other_isr(int irq, void *ptr)
{
	struct hisi_jpu_data_type *hisijd;
	char __iomem *jpu_top_base;
	int isr_state;

	hisijd = (struct hisi_jpu_data_type *)ptr;
	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		goto err_out;
	}

	jpu_top_base = hisijd->jpu_top_base;

	isr_state = inp32(jpu_top_base + JPGDEC_IRQ_REG2);
	outp32(jpu_top_base + JPGDEC_IRQ_REG0, 0x8);

	hisi_jpu_dec_other_config(hisijd);

	err_out:
	return IRQ_HANDLED;
}

void hisi_jpu_dec_interrupt_unmask(struct hisi_jpu_data_type *hisijd)
{
	char __iomem *jpu_top_base;
	uint32_t unmask;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return ;
	}

	jpu_top_base = hisijd->jpu_top_base;

	unmask = ~0;
	unmask &= ~(BIT_JPGDEC_INT_DEC_ERR | BIT_JPGDEC_INT_DEC_FINISH | BIT_JPGDEC_INT_OVER_TIME);
	outp32(jpu_top_base + JPGDEC_IRQ_REG1, unmask);
}

void hisi_jpu_dec_interrupt_mask(struct hisi_jpu_data_type *hisijd)
{
	char __iomem *jpu_top_base;
	uint32_t mask;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return ;
	}

	jpu_top_base = hisijd->jpu_top_base;

	mask = ~0;
	outp32(jpu_top_base + JPGDEC_IRQ_REG1, mask);
}

void hisi_jpu_dec_interrupt_clear(struct hisi_jpu_data_type *hisijd)
{
	char __iomem *jpu_top_base;

	if (!hisijd) {
		HISI_JPU_ERR("hisijd is NULL!\n");
		return ;
	}

	jpu_top_base = hisijd->jpu_top_base;

	/* clear jpg decoder IRQ state
	* [3]: jpgdec_int_over_time;
	* [2]: jpgdec_int_dec_err;
	* [1]: jpgdec_int_bs_res;
	* [0]: jpgdec_int_dec_finish;*/
	outp32(jpu_top_base + JPGDEC_IRQ_REG0, 0xf);
}
/*lint -restore*/

