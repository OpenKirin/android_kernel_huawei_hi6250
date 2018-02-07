/*
 * hi6555 codec stub driver.
 *
 * Copyright (c) 2015 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/clk.h>
#include <linux/timer.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>
#include <asm/io.h>
#include <linux/spinlock.h>
#include <linux/switch.h>
#include <linux/wakelock.h>
#include <linux/vmalloc.h>
#include <linux/moduleparam.h>
#include <linux/printk.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/of_gpio.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/gpio.h>
#include <sound/jack.h>
#include <linux/input/matrix_keypad.h>
#include <linux/irq.h>
#include <sound/hi6555c_common.h>
#include <linux/interrupt.h>
#include <linux/irqnr.h>
#include <linux/hisi/drv_pmic_if.h>
#include <linux/hisi/hilog.h>
#include <soc_crgperiph_interface.h>
#include <linux/pm_runtime.h>
#include "hs_auto_calib/hs_auto_calib.h"
#include "hi6555c_stub.h"
#include "hi6555c_debug.h"
#include "hi6xxx_fpga_test.h"

static struct snd_soc_codec *g_codec = NULL;
static void __iomem *reg_base_addr[HI6555C_REG_CNT] = {0};

static struct hi6555c_reg_page reg_page_array[] = {
	{PAGE_SoCCODEC, HI6555C_SoCCODEC_START, HI6555C_SoCCODEC_END, "PAGE_SoCCODEC"},
	{PAGE_ASPCFG, HI6555C_ASPCFG_START, HI6555C_ASPCFG_END, "PAGE_ASPCFG"},
	{PAGE_AO_IOC, HI6555C_AOIOC_START, HI6555C_AOIOC_END, "PAGE_AO_IOC"},
};

static bool _hi6555c_reg_value_valid(unsigned int reg_type, unsigned int reg_value)
{
	bool is_valid = true;
	unsigned int reg_page_array_size = ARRAY_SIZE(reg_page_array);
	unsigned int i = 0;

	for (i = 0; i < reg_page_array_size; i++) {
		if (reg_type != reg_page_array[i].page_tag) {
			is_valid = false;
			continue;
		}

		is_valid  = true;

		if ((reg_value < reg_page_array[i].page_reg_begin) ||
		    (reg_value > reg_page_array[i].page_reg_end)) {
			is_valid = false;
			loge("%s: offset 0x%x is invalid\n", reg_page_array[i].page_name, reg_value);
		}
		break;
	}

	return is_valid;
}

static unsigned int _hi6555c_reg_read(struct hi6555c_priv *priv, unsigned int reg)
{
	volatile unsigned int ret = 0;
	unsigned int reg_type = 0;
	unsigned int reg_value = 0;
	unsigned long flags = 0;

	reg_type  = reg & PAGE_TYPE_MASK;
	reg_value = reg & PAGE_VALUE_MASK;

	if (!_hi6555c_reg_value_valid(reg_type, reg_value)) {
		loge("invalid reg:0x%x\n", reg);
		return INVALID_REG_VALUE;
	}

	spin_lock_irqsave(&priv->lock, flags);

	switch (reg_type) {
	case PAGE_SoCCODEC:
		ret = readl(reg_base_addr[HI6555C_SOCCODEC] + reg_value);
		break;
	case PAGE_ASPCFG:
		ret = readl(reg_base_addr[HI6555C_ASPCFG] + reg_value);
		break;
	case PAGE_AO_IOC:
		ret = readl(reg_base_addr[HI6555C_AOIOC] + reg_value);
		break;
	default:
		loge("cannot read reg=0x%x\n", reg);
		ret = INVALID_REG_VALUE;
		break;
	}

	spin_unlock_irqrestore(&priv->lock, flags);

	return ret;
}

static void _hi6555c_reg_write(struct hi6555c_priv *priv, unsigned int reg, unsigned int value)
{
	unsigned int reg_type = 0;
	unsigned int reg_value = 0;
	unsigned long flags = 0;

	reg_type  = reg & PAGE_TYPE_MASK;
	reg_value = reg & PAGE_VALUE_MASK;

	if (!_hi6555c_reg_value_valid(reg_type, reg_value)) {
		loge("invalid reg:0x%x, value:%d\n", reg, value);
		return;
	}

	spin_lock_irqsave(&priv->lock, flags);

	switch (reg_type) {
	case PAGE_SoCCODEC:
		writel(value, reg_base_addr[HI6555C_SOCCODEC] + reg_value);
		break;
	case PAGE_ASPCFG:
		writel(value, reg_base_addr[HI6555C_ASPCFG] + reg_value);
		break;
	case PAGE_AO_IOC:
		writel(value, reg_base_addr[HI6555C_AOIOC] + reg_value);
		break;
	default:
		loge("reg is invalid: reg=0x%x, value=0x%x\n", reg, value);
		break;
	}

	spin_unlock_irqrestore(&priv->lock, flags);
}

unsigned int codec_reg_read(unsigned int reg)
{
	struct hi6555c_priv *priv = NULL;

	if (!g_codec) {
		loge("g_codec is null\n");
		return INVALID_REG_VALUE;
	}

	priv = snd_soc_codec_get_drvdata(g_codec);

	if (!priv) {
		loge("priv is null\n");
		return INVALID_REG_VALUE;
	}

	return _hi6555c_reg_read(priv, reg);
}

void codec_reg_write(unsigned int reg, unsigned int value)
{
	struct hi6555c_priv *priv = NULL;

	if (!g_codec) {
		loge("g_codec is null\n");
		return;
	}

	priv = snd_soc_codec_get_drvdata(g_codec);

	if (!priv) {
		loge("priv is null\n");
		return;
	}

	_hi6555c_reg_write(priv, reg, value);
}

/*
static void hi6555c_set_reg_bits(unsigned int reg, unsigned int value)
{
	unsigned int val = 0;

	val = codec_reg_read(reg) | (value);
	codec_reg_write(reg, val);
}

static void hi6555c_clr_reg_bits(unsigned int reg, unsigned int value)
{
	unsigned int val = 0;

	val = codec_reg_read(reg) & ~(value);
	codec_reg_write(reg, val);
}
*/

static inline unsigned int hi6555c_reg_read(struct snd_soc_codec *codec,
				unsigned int reg)
{
	return 0;
}

static inline int hi6555c_reg_write(struct snd_soc_codec *codec,
				unsigned int reg,
				unsigned int value)
{
	return 0;
}

static const struct snd_kcontrol_new hi6555c_snd_controls[] = {};

static const struct snd_soc_dapm_widget hi6555c_dapm_widgets[] = {};

static const struct snd_soc_dapm_route hi6555c_route_map[] = {};

static int hi6555c_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	return 0;
}

static int hi6555c_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	return 0;
}

struct snd_soc_dai_ops hi6555c_dai_ops = {
	.startup    = hi6555c_startup,
	.hw_params  = hi6555c_hw_params,
};

struct snd_soc_dai_driver hi6555c_dai[] = {
	{
		.name = "hi6555c-dai",
		.playback = {
			.stream_name = "Playback",
			.channels_min = HI6555C_PB_MIN_CHANNELS,
			.channels_max = HI6555C_PB_MAX_CHANNELS,
			.rates = HI6555C_RATES,
			.formats = HI6555C_FORMATS
		},
		.capture = {
			.stream_name = "Capture",
			.channels_min = HI6555C_CP_MIN_CHANNELS,
			.channels_max = HI6555C_CP_MAX_CHANNELS,
			.rates = HI6555C_RATES,
			.formats = HI6555C_FORMATS
		},
		.ops = &hi6555c_dai_ops,
	},
};

static void hi6555c_io_init(void)
{
	/* playback io cfg */
	codec_reg_write(0xfff11060, 0x5);
	codec_reg_write(0xfff11034, 0x3);
	codec_reg_write(0xfff11038, 0x3);
	codec_reg_write(0xfff11044, 0x3);

	/* capture io cfg */
	codec_reg_write(0xfff11098, 0x3);
	codec_reg_write(0xfff1109c, 0x3);
}

/*lint -e429*/
static int hi6555c_set_priv(struct snd_soc_codec *codec)
{
	struct hi6555c_priv *priv = NULL;
	struct device *dev = codec->dev;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		loge("priv devm_kzalloc failed\n");
		return -ENOMEM;
	}

	snd_soc_codec_set_drvdata(codec, priv);

	g_codec = codec;
	priv->codec = codec;

	priv->v_codec_reg[0] = 0;
	priv->v_codec_reg[1] = 0;

	spin_lock_init(&priv->lock);

	return 0;
}
/*lint +e429*/

static int hi6555c_codec_probe(struct snd_soc_codec *codec)
{
	int ret = 0;
	struct hi6555c_priv *priv = NULL;

	logi("Begin\n");

	ret = hi6555c_set_priv(codec);
	if (0 != ret) {
		loge("hi6555c_set_priv err, ret=%d\n", ret);
		goto end;
	}

	priv = snd_soc_codec_get_drvdata(codec);

	priv->asp_use_count = 0;
	priv->asp_cfg_regu = devm_regulator_get(codec->dev, "asp");
	if (IS_ERR(priv->asp_cfg_regu)) {
		loge("get asp regulators err:%pK\n", priv->asp_cfg_regu);
		ret = PTR_ERR(priv->asp_cfg_regu);
		goto end;
	}

	ret = regulator_enable(priv->asp_cfg_regu);
	priv->asp_use_count++;
	if (0 != ret) {
		loge("couldn't enable asp_cfg_regu regulators %d\n", ret);
		goto end;
	}

	hi6555c_io_init();
end:
	logi("End\n");

	return ret;
}

static int hi6555c_codec_remove(struct snd_soc_codec *codec)
{
	struct hi6555c_priv *priv = NULL;

	priv = snd_soc_codec_get_drvdata(codec);

	if (regulator_disable(priv->asp_cfg_regu)) {
		loge("regulator_disable failed\n");
	}

	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_hi6555c = {
	.probe    = hi6555c_codec_probe,
	.remove  = hi6555c_codec_remove,
	.read      = hi6555c_reg_read,
	.write     = hi6555c_reg_write,
	.controls = hi6555c_snd_controls,
	.num_controls = ARRAY_SIZE(hi6555c_snd_controls),
	.dapm_widgets = hi6555c_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(hi6555c_dapm_widgets),
	.dapm_routes = hi6555c_route_map,
	.num_dapm_routes = ARRAY_SIZE(hi6555c_route_map),
};

static int hi6555c_base_addr_map(struct platform_device *pdev)
{
	struct resource *res = NULL;
	unsigned int i;

	for (i = 0; i < HI6555C_REG_CNT; i++) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (!res) {
			loge("platform_get_resource %d err\n", i);
			return -ENOENT;
		}

		reg_base_addr[i] = (char * __force)(ioremap(res->start, resource_size(res)));
		if (!reg_base_addr[i]) {
			loge("cannot map register memory:%d\n", i);
			return -ENOMEM;
		}
	}

	return 0;
}

static void hi6555c_base_addr_unmap(void)
{
	unsigned int i;

	for (i = 0; i < HI6555C_REG_CNT; i++) {
		if (reg_base_addr[i]) {
			iounmap(reg_base_addr[i]);
			reg_base_addr[i] = NULL;
		}
	}

	return;
}

static int hi6555c_stub_probe(struct platform_device *pdev)
{
	int ret = 0;

	logi("Begin\n");

	hi6xxx_fpga_test_init();

	ret = hi6555c_base_addr_map(pdev);
	if (0 != ret) {
		hi6555c_base_addr_unmap();
		return ret;
	}

	logi("End\n");

	return snd_soc_register_codec(&pdev->dev, &soc_codec_dev_hi6555c, hi6555c_dai, ARRAY_SIZE(hi6555c_dai));
}

static int hi6555c_stub_remove(struct platform_device *pdev)
{
	logd("Begin\n");

	snd_soc_unregister_codec(&pdev->dev);

	hi6555c_base_addr_unmap();

	hi6xxx_fpga_test_deinit();

	return 0;
}

static const struct of_device_id hi6555c_stub_codec_match[] = {
	{ .compatible = "hisilicon,hi6555c-stub-codec", },
	{},
};

static struct platform_driver hi6555c_stub_driver = {
	.driver = {
		.name  = "hi6555c-stub-codec",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hi6555c_stub_codec_match),
	},
	.probe  = hi6555c_stub_probe,
	.remove = hi6555c_stub_remove,
};

static int __init hi6555c_stub_codec_init(void)
{
	logi("Begin\n");
	return platform_driver_register(&hi6555c_stub_driver);
}
module_init(hi6555c_stub_codec_init);

static void __exit hi6555c_stub_codec_exit(void)
{
	logi("Begin\n");
	platform_driver_unregister(&hi6555c_stub_driver);
}
module_exit(hi6555c_stub_codec_exit);

MODULE_DESCRIPTION("ASoC hi6555c stub driver");
MODULE_LICENSE("GPL");
