/*
 * vdec regulator manager
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */
#include "omxvdec.h"
#include "regulator.h"
#include "platform.h"

#include <linux/hisi/hisi-iommu.h>
#include <linux/iommu.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/clk.h>

/*lint -e774*/

#define REGULATOR_NAME          "ldo_vdec"
#define VCODEC_CLOCK_NAME       "clk_vdec"
#define VCODEC_CLK_RATE         "dec_clk_rate"

static HI_U32  g_VdecClkRate_l  = 208000000;    //208MHZ
static HI_U32  g_VdecClkRate_n  = 332000000;    //332MHZ
static HI_U32  g_VdecClkRate_h  = 480000000;    //480MHZ
static HI_U32  g_CurClkRate     = 0;
static HI_U32  g_vdecQosMode    = 0x2;
static HI_BOOL g_VdecPowerOn    = HI_FALSE;
static struct  regulator    *g_VdecRegulator  = HI_NULL;
static struct  iommu_domain *g_VdecSmmuDomain = HI_NULL;
static VFMW_DTS_CONFIG_S     g_DtsConfig;
static struct clk *g_PvdecClk = HI_NULL;
struct semaphore   g_RegulatorSem;

#ifdef HIVDEC_SMMU_SUPPORT

/*----------------------------------------
    func: iommu enable intf
 ----------------------------------------*/
static HI_S32 VDEC_Enable_Iommu(struct device *dev)
{
	g_VdecSmmuDomain = hisi_ion_enable_iommu(NULL);
	if (g_VdecSmmuDomain == HI_NULL) {
		OmxPrint(OMX_FATAL, "%s iommu_domain_alloc failed\n", __func__);
		return HI_FAILURE;
	}
	return HI_SUCCESS;
}/*lint !e715 */

/*----------------------------------------
    func: iommu disable intf
 ----------------------------------------*/
static HI_VOID VDEC_Disable_Iommu(struct device *dev)
{
	if ((g_VdecSmmuDomain != NULL) && (dev != NULL))
		g_VdecSmmuDomain = NULL;
}

/*----------------------------------------
    func: get smmu page table base addr
 ----------------------------------------*/
static HI_U64 VDEC_GetSmmuBasePhy(struct device *dev)
{
	struct iommu_domain_data *domain_data = HI_NULL;

	if (VDEC_Enable_Iommu(dev) == HI_FAILURE)
		return 0;

	domain_data = (struct iommu_domain_data *)(g_VdecSmmuDomain->priv);

	return (HI_U64) (domain_data->phy_pgd_base);
}

#endif

/*----------------------------------------
    func: initialize vcodec clock rate
 ----------------------------------------*/
static HI_S32 VDEC_Init_ClockRate(struct device *dev)
{
	HI_S32 ret;
	struct device_node *np = HI_NULL;
	struct clk *pvdec_clk  = HI_NULL;

	if (dev == HI_NULL) {
		printk(KERN_CRIT "%s: invalid dev is NULL\n", __func__);
		return HI_FAILURE;
	}

	np = dev->of_node;

	pvdec_clk = devm_clk_get(dev, VCODEC_CLOCK_NAME);
	if (IS_ERR_OR_NULL(pvdec_clk)) {
		printk(KERN_CRIT "%s can not get clock\n", __func__);
		return HI_FAILURE;
	}
	g_PvdecClk = pvdec_clk;

	ret = of_property_read_u32_index(np, VCODEC_CLK_RATE, 0, &g_VdecClkRate_h);
	if (ret) {
		printk(KERN_CRIT "%s can not get g_VdecClkRate_h, return %d\n", __func__, ret);
		g_VdecClkRate_h = 480000000;    //480MHZ
		return HI_FAILURE;
	}
	OmxPrint(OMX_ALWS, "%s get g_VdecClkRate_h = %u\n", __func__, g_VdecClkRate_h);

	ret = of_property_read_u32_index(np, VCODEC_CLK_RATE, 1, &g_VdecClkRate_n);
	if (ret) {
		printk(KERN_CRIT "%s can not get g_VdecClkRate_n, return %d\n", __func__, ret);
		g_VdecClkRate_n = 332000000;    //332MHZ
		return HI_FAILURE;
	}
	OmxPrint(OMX_ALWS, "%s get g_VdecClkRate_n = %u\n", __func__, g_VdecClkRate_n);
	ret = of_property_read_u32_index(np, VCODEC_CLK_RATE, 2, &g_VdecClkRate_l);
	if (ret) {
		printk(KERN_CRIT "%s can not get g_VdecClkRate_l, return %d\n", __func__, ret);
		g_VdecClkRate_l = 208000000;    //208MHZ
		return HI_FAILURE;
	}
	OmxPrint(OMX_ALWS, "%s get g_VdecClkRate_l = %u\n", __func__, g_VdecClkRate_l);

	g_CurClkRate = g_VdecClkRate_l;

	return HI_SUCCESS;
}

/*----------------------------------------
    func: get dts configure
          as reg base & irq num
 ----------------------------------------*/
static HI_S32 VDEC_GetDtsConfigInfo(struct device *dev, VFMW_DTS_CONFIG_S *pDtsConfig)
{
	HI_S32 ret;
	struct device_node *np_crg = HI_NULL;
	struct device_node *np     = dev->of_node;
	struct resource res;

	if (np == HI_NULL) {
		printk(KERN_CRIT "%s: invalid device node is null\n", __func__);
		return HI_FAILURE;
	}

	if (pDtsConfig == HI_NULL) {
		printk(KERN_CRIT "%s: invalid pDtsConfig is null\n", __func__);
		return HI_FAILURE;
	}
	//Get irq num, return 0 if failed
	pDtsConfig->VdecIrqNumNorm = irq_of_parse_and_map(np, 0);
	if (pDtsConfig->VdecIrqNumNorm == 0) {
		printk(KERN_CRIT "%s irq_of_parse_and_map VdecIrqNumNorm failed\n", __func__);
		return HI_FAILURE;
	}

	pDtsConfig->VdecIrqNumProt = 323;//irq_of_parse_and_map(np, 1);
	if ( pDtsConfig->VdecIrqNumProt == 0) {
		printk(KERN_CRIT "%s irq_of_parse_and_map VdecIrqNumProt failed\n", __func__);
		return HI_FAILURE;
	}

	pDtsConfig->VdecIrqNumSafe = 324;//irq_of_parse_and_map(np, 2);



	if ( pDtsConfig->VdecIrqNumSafe == 0) {
		printk(KERN_CRIT "%s irq_of_parse_and_map VdecIrqNumSafe failed\n", __func__);


		return HI_FAILURE;
	}

	//Get reg base addr & size, return 0 if success
	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		printk(KERN_CRIT "%s of_address_to_resource failed return %d\n", __func__, ret);
		return HI_FAILURE;
	}
	pDtsConfig->VdhRegBaseAddr = res.start;
	pDtsConfig->VdhRegRange = resource_size(&res);

#ifdef HIVDEC_SMMU_SUPPORT
	//Get reg base addr & size, return 0 if failed
	pDtsConfig->SmmuPageBaseAddr = VDEC_GetSmmuBasePhy(dev);
	if (pDtsConfig->SmmuPageBaseAddr == 0) {
		printk(KERN_CRIT "%s Regulator_GetSmmuBasePhy failed\n", __func__);
		return HI_FAILURE;
	}
#endif

	np_crg = of_find_compatible_node(HI_NULL, HI_NULL, "hisilicon,crgctrl");
	ret = of_address_to_resource(np_crg, 0, &res);
	if (ret) {
		printk(KERN_CRIT "%s of_address_to_resource crg failed return %d\n", __func__, ret);
		return HI_FAILURE;
	}
	pDtsConfig->PERICRG_RegBaseAddr = res.start;

	//Check if is FPGA platform
#ifdef PLATFORM_KIRIN660
	ret = of_property_read_u32(np, "fpga_flag", &pDtsConfig->IsFPGA);
#else
	ret = of_property_read_u32(np, "vdec_fpga", &pDtsConfig->IsFPGA);
#endif
	if (ret) {
		pDtsConfig->IsFPGA = 0;
		OmxPrint(OMX_ALWS, "Is not FPGA platform\n");
	}

	/* get vdec qos mode */
	ret = of_property_read_u32(np, "vdec_qos_mode", &g_vdecQosMode);
	if (ret) {
		g_vdecQosMode = 0x2;
		OmxPrint(OMX_WARN, "get vdec qos mode failed set default\n");
	}

	ret = VDEC_Init_ClockRate(dev);
	if (ret != HI_SUCCESS) {
		printk(KERN_ERR "%s VDEC_Init_ClockRate failed\n", __func__);
		return HI_FAILURE;
		//fixme: continue or return?
	}

	return HI_SUCCESS;
}

/******************************** SHARE FUNC **********************************/

/*----------------------------------------
    func: regulator probe entry
 ----------------------------------------*/
HI_S32 VDEC_Regulator_Probe(struct device *dev)
{
	HI_S32 ret;

	g_VdecRegulator = HI_NULL;

	if (dev == HI_NULL) {
		printk(KERN_CRIT "%s, invalid params", __func__);
		return HI_FAILURE;
	}

	g_VdecRegulator = devm_regulator_get(dev, REGULATOR_NAME);
	if (g_VdecRegulator == HI_NULL) {
		printk(KERN_CRIT "%s get regulator failed\n", __func__);
		return HI_FAILURE;
	} else if (IS_ERR(g_VdecRegulator)) {
		OmxPrint(OMX_FATAL, "%s get regulator failed, error no = %ld\n", __func__, PTR_ERR(g_VdecRegulator));
		g_VdecRegulator = HI_NULL;
		return HI_FAILURE;
	}

	memset(&g_DtsConfig, 0, sizeof(g_DtsConfig));
	ret = VDEC_GetDtsConfigInfo(dev, &g_DtsConfig);
	if (ret != HI_SUCCESS) {
		printk(KERN_CRIT "%s Regulator_GetDtsConfigInfo failed\n", __func__);
		return HI_FAILURE;
	}

	ret = VFMW_SetDtsConfig(&g_DtsConfig);
	if (ret != HI_SUCCESS) {
		printk(KERN_CRIT "%s VFMW_SetDtsConfig failed\n", __func__);
		return HI_FAILURE;
	}
	VDEC_INIT_MUTEX(&g_RegulatorSem);

	return HI_SUCCESS;
}

/*----------------------------------------
    func: regulator deinitialize
 ----------------------------------------*/
HI_S32 VDEC_Regulator_Remove(struct device * dev)
{
	VDEC_DOWN_INTERRUPTIBLE(&g_RegulatorSem);

	VDEC_Disable_Iommu(dev);
	g_VdecRegulator = HI_NULL;
	g_PvdecClk      = HI_NULL;

	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_SUCCESS;
}

#ifdef PLATFORM_KIRIN970
static HI_S32 VDEC_Regulator_Enable_Kirin970(HI_VOID)
{
	HI_S32 ret;

	VDEC_DOWN_INTERRUPTIBLE(&g_RegulatorSem);

	if (g_VdecPowerOn == HI_TRUE) {
		VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);
		return HI_SUCCESS;
	}

	if (g_PvdecClk == HI_NULL) {
		printk(KERN_CRIT "%s: invalid g_PvdecClk is NULL\n", __func__);
		goto error_exit;
	}

	if (IS_ERR_OR_NULL(g_VdecRegulator)) {
		OmxPrint(OMX_WARN, "%s ERROR: g_VdecRegulator is NULL", __func__);
		goto error_exit;
	}

	OmxPrint(OMX_ALWS, "%s, call regulator_enable\n", __func__);
	ret = regulator_enable(g_VdecRegulator);
	if (ret != 0) {
		printk(KERN_CRIT "%s enable regulator failed\n", __func__);
		goto error_exit;
	}

	ret = clk_prepare_enable(g_PvdecClk);
	if (ret != 0) {
		printk(KERN_CRIT "%s clk_prepare_enable failed\n", __func__);
		goto error_regulator_disable;
	}

	ret  = clk_set_rate(g_PvdecClk, g_VdecClkRate_l);
	if (ret)
	{
		printk(KERN_CRIT "%s Failed to clk_set_rate:%u, return %d\n", __func__, g_VdecClkRate_l, ret);
		goto error_regulator_disable;
	}
	OmxPrint(OMX_ALWS, "%s set VdecClkRate low:%u\n", __func__, g_VdecClkRate_l);

	g_VdecPowerOn = HI_TRUE;

	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_SUCCESS;

error_regulator_disable:
	regulator_disable(g_VdecRegulator);

error_exit:
	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_FAILURE;
}
#endif

#ifdef PLATFORM_KIRIN660
static HI_S32 VDEC_Regulator_Enable_Kirin660(HI_VOID)
{
	HI_S32 ret;

	VDEC_DOWN_INTERRUPTIBLE(&g_RegulatorSem);

	if (g_VdecPowerOn == HI_TRUE) {
		VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);
		return HI_SUCCESS;
	}

	if (g_PvdecClk == HI_NULL) {
		printk(KERN_CRIT "%s: invalid g_PvdecClk is NULL\n", __func__);
		goto error_exit;
	}

	if (IS_ERR_OR_NULL(g_VdecRegulator)) {
		OmxPrint(OMX_WARN, "%s ERROR: g_VdecRegulator is NULL", __func__);
		goto error_exit;
	}

	OmxPrint(OMX_ALWS, "%s, call regulator_enable\n", __func__);
	ret = regulator_enable(g_VdecRegulator);
	if (ret != 0) {
		printk(KERN_CRIT "%s enable regulator failed\n", __func__);
		goto error_exit;
	}

	ret = clk_prepare_enable(g_PvdecClk);
	if (ret != 0) {
		printk(KERN_CRIT "%s clk_prepare_enable failed\n", __func__);
		goto error_regulator_disable;
	}

	ret  = clk_set_rate(g_PvdecClk, g_VdecClkRate_l);
	if (ret)
	{
		printk(KERN_CRIT "%s Failed to clk_set_rate:%u, return %d\n", __func__, g_VdecClkRate_l, ret);
		goto error_regulator_disable;
	}
	OmxPrint(OMX_ALWS, "%s set VdecClkRate low:%u\n", __func__, g_VdecClkRate_l);

	g_VdecPowerOn = HI_TRUE;

	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_SUCCESS;

error_regulator_disable:
	regulator_disable(g_VdecRegulator);

error_exit:
	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_FAILURE;
}
#endif

/*----------------------------------------
    func: enable regulator
 ----------------------------------------*/
HI_S32 VDEC_Regulator_Enable(HI_VOID)
{
#ifdef PLATFORM_KIRIN970
	return VDEC_Regulator_Enable_Kirin970();
#endif

#ifdef PLATFORM_KIRIN660
	return VDEC_Regulator_Enable_Kirin660();
#endif
}

#ifdef PLATFORM_KIRIN970
static HI_S32 VDEC_Regulator_Disable_Kirin970(HI_VOID)
{
	HI_S32 ret;

	VDEC_DOWN_INTERRUPTIBLE(&g_RegulatorSem);

	if (g_VdecPowerOn == HI_FALSE) {
		VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);
		return HI_SUCCESS;
	}

	if (g_PvdecClk == HI_NULL) {
		printk(KERN_CRIT "%s g_PvdecClk is NULL\n", __func__);
		goto error_exit;
	}

	if (IS_ERR_OR_NULL(g_VdecRegulator)) {
		OmxPrint(OMX_WARN, "%s g_VdecRegulator is NULL", __func__ );
		goto error_exit;
	}
	ret = clk_set_rate(g_PvdecClk, g_VdecClkRate_l);
	if (ret) {
		printk(KERN_CRIT "%s Failed to clk_set_rate:%u, return %d\n", __func__, g_VdecClkRate_l, ret);
		//goto error_exit;//continue, no return
	}
	OmxPrint(OMX_ALWS, "%s set VdecClkRate low:%u\n", __func__, g_VdecClkRate_l);

	clk_disable_unprepare(g_PvdecClk);

	OmxPrint(OMX_ALWS, "%s, call regulator_disable\n", __func__);
	ret = regulator_disable(g_VdecRegulator);
	if (ret != 0) {
		printk(KERN_CRIT "%s disable regulator failed\n", __func__);
		goto error_exit;
	}

	g_VdecPowerOn = HI_FALSE;

	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_SUCCESS;

error_exit:
	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_FAILURE;
}
#endif

#ifdef PLATFORM_KIRIN660
static HI_S32 VDEC_Regulator_Disable_Kirin660(HI_VOID)
{
	HI_S32 ret;

	VDEC_DOWN_INTERRUPTIBLE(&g_RegulatorSem);

	if (g_VdecPowerOn == HI_FALSE) {
		VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);
		return HI_SUCCESS;
	}

	if (g_PvdecClk == HI_NULL) {
		printk(KERN_CRIT "%s g_PvdecClk is NULL\n", __func__);
		goto error_exit;
	}

	if (IS_ERR_OR_NULL(g_VdecRegulator)) {
		OmxPrint(OMX_WARN, "%s g_VdecRegulator is NULL", __func__ );
		goto error_exit;
	}
	ret = clk_set_rate(g_PvdecClk, g_VdecClkRate_l);
	if (ret) {
		printk(KERN_CRIT "%s Failed to clk_set_rate:%u, return %d\n", __func__, g_VdecClkRate_l, ret);
		//goto error_exit;//continue, no return
	}
	OmxPrint(OMX_ALWS, "%s set VdecClkRate low:%u\n", __func__, g_VdecClkRate_l);

	clk_disable_unprepare(g_PvdecClk);

	OmxPrint(OMX_ALWS, "%s, call regulator_disable\n", __func__);
	ret = regulator_disable(g_VdecRegulator);
	if (ret != 0) {
		printk(KERN_CRIT "%s disable regulator failed\n", __func__);
		goto error_exit;
	}

	g_VdecPowerOn = HI_FALSE;

	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_SUCCESS;

error_exit:
	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_FAILURE;
}
#endif

/*----------------------------------------
    func: disable regulator
 ----------------------------------------*/
HI_S32 VDEC_Regulator_Disable(HI_VOID)
{
#ifdef PLATFORM_KIRIN970
	return VDEC_Regulator_Disable_Kirin970();
#endif

#ifdef PLATFORM_KIRIN660
	return VDEC_Regulator_Disable_Kirin660();
#endif
}

/*----------------------------------------
    func: get decoder clock rate
 ----------------------------------------*/
HI_S32 VDEC_Regulator_GetClkRate(CLK_RATE_E *pClkRate)
{
	VDEC_DOWN_INTERRUPTIBLE(&g_RegulatorSem);

	if (g_CurClkRate == g_VdecClkRate_h) {
		*pClkRate = CLK_RATE_HIGH;
	} else if (g_CurClkRate == g_VdecClkRate_n) {
		*pClkRate = CLK_RATE_NORMAL;
	} else if (g_CurClkRate == g_VdecClkRate_l) {
		*pClkRate = CLK_RATE_LOW;
	} else {
		OmxPrint(OMX_ERR, "%s: unkown clk rate %d, return CLK_RATE_HIGH\n", __func__, g_CurClkRate);
		*pClkRate = CLK_RATE_HIGH;
	}
	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_SUCCESS;
}

/*----------------------------------------
    func: decoder clock rate select
 ----------------------------------------*/
HI_S32 VDEC_Regulator_SetClkRate(CLK_RATE_E eClkRate)
{
	HI_S32 ret          = 0;
	HI_U32 rate         = 0;
	HI_U8 need_set_flag = 1;

	VDEC_DOWN_INTERRUPTIBLE(&g_RegulatorSem);

	if (g_DtsConfig.IsFPGA) {
		VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);
		return HI_SUCCESS;
	}

	if (IS_ERR_OR_NULL(g_PvdecClk)) {
		printk(KERN_ERR "Couldn't get clk [%s]\n", __func__);
		goto error_exit;
	}

	rate = (HI_U32) clk_get_rate(g_PvdecClk);
	switch (eClkRate) {
	case CLK_RATE_LOW:
		if (g_VdecClkRate_l == rate) {
			need_set_flag = 0;
		} else {
			rate = g_VdecClkRate_l;
			need_set_flag = 1;
		}
		break;

	case CLK_RATE_NORMAL:
		if (g_VdecClkRate_n == rate) {
			need_set_flag = 0;
		}
		else {
			rate = g_VdecClkRate_n;
			need_set_flag = 1;
		}
		break;

	case CLK_RATE_HIGH:
		if (g_VdecClkRate_h == rate) {
			need_set_flag = 0;
		} else {
			rate = g_VdecClkRate_h;
			need_set_flag = 1;
		}
		break;

	default:
		printk(KERN_ERR "[%s] unsupport clk rate enum %d\n", __func__, eClkRate);
		goto error_exit;
	}

	if (need_set_flag == 1) {
		ret = clk_set_rate(g_PvdecClk, rate);
		if (ret != 0) {
			printk(KERN_ERR "Failed to clk_set_rate %u HZ[%s] ret : %d\n", rate, __func__, ret);
			goto error_exit;
		}
		g_CurClkRate = rate;
	}

	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_SUCCESS;

error_exit:
	VDEC_UP_INTERRUPTIBLE(&g_RegulatorSem);

	return HI_FAILURE;
}
