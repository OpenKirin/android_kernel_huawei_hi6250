#include <linux/clk.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/hisi/hisi-iommu.h>

#include "venc_regulator.h"
#include "drv_venc_osal.h"
#include "drv_venc.h"

#define VENC_CLK_RATE   "enc_clk_rate"
#define REGULATOR_NAME  "ldo_venc"
#define VENC_CLOCK_NAME "clk_venc"

static  struct clk       *g_PvencClk        = HI_NULL;
struct  iommu_domain     *g_hisi_mmu_domain = HI_NULL;
static  struct regulator *g_VencRegulator   = NULL;
static  VeduEfl_DTS_CONFIG_S g_VencDtsConfig;
static  VENC_CLK_TYPE g_currClk = CLK_RATE_LOW;

static HI_U32 g_vencQosMode  = 0x2;
static HI_BOOL g_VencPowerOn = HI_FALSE;

/*lint -e838 -e747 -e774 -e845*/
static int Venc_Enable_Iommu(struct platform_device *pdev)
{
	struct iommu_domain *hisi_domain = NULL;
	struct iommu_domain_data* domain_data = NULL;
	uint64_t phy_pgd_base = 0;

	if ((!pdev) || (!(&pdev->dev) ))
	{
		HI_ERR_VENC("%s,  invalid Parameters!\n", __func__);
		return -1;
	}

	/* create iommu domain */
	hisi_domain = hisi_ion_enable_iommu(NULL);
	if (!hisi_domain) {
		HI_ERR_VENC("%s, iommu_domain_alloc failed!\n", __func__);
		return -1;
	}

	g_hisi_mmu_domain = hisi_domain;

	domain_data = (struct iommu_domain_data *)(g_hisi_mmu_domain->priv);
	if(NULL != domain_data)
	{
		phy_pgd_base = (uint64_t)(domain_data->phy_pgd_base);
		HI_INFO_VENC("%s, phy_pgd_base:0x%x\n", __func__, phy_pgd_base);
	}
	else
	{
	   return -1;
	}

	return 0;
}

static int Venc_Disable_Iommu(struct platform_device *pdev)
{
	if((NULL != g_hisi_mmu_domain) && (NULL != pdev))
	{
		g_hisi_mmu_domain = NULL;
		return 0;
	}

	return -1;
}

static HI_S32 Venc_GetDtsConfigInfo(struct platform_device *pdev, VeduEfl_DTS_CONFIG_S *pDtsConfig)
{
	HI_U32 rate_h = 0;
	HI_U32 rate_n = 0;
	HI_U32 rate_l = 0;
	HI_S32 ret    = HI_FAILURE;
	struct resource res;
	struct clk *pvenc_clk    = HI_NULL;
	struct device_node *np   = NULL;
	struct device *dev       = &pdev->dev;
	struct iommu_domain_data *domain_data = NULL;

	if (!dev){
		HI_ERR_VENC("invalid argument dev:0x%x\n", dev);
		return HI_FAILURE;
	}

	np = dev->of_node;

	HiMemSet(&res, 0, sizeof(res));
	if ((!np) || (!pDtsConfig)){
		HI_ERR_VENC("invalid argument np:0x%x, pDtsConfig:0x%x \n", np, pDtsConfig);
		return HI_FAILURE;
	}

	/* 1 read IRQ num from dts */
	pDtsConfig->VeduIrqNumNorm = irq_of_parse_and_map(np, 0);
	if (pDtsConfig->VeduIrqNumNorm == 0){
		HI_ERR_VENC("%s, irq_of_parse_and_map VeduIrqNumNorm failed!\n", __func__);
		return HI_FAILURE;
	}

	pDtsConfig->VeduIrqNumProt = irq_of_parse_and_map(np, 1);
	if (pDtsConfig->VeduIrqNumProt == 0){
		HI_ERR_VENC("%s, irq_of_parse_and_map VeduIrqNumProt failed!\n", __func__);
		return HI_FAILURE;
	}

	pDtsConfig->VeduIrqNumSafe = irq_of_parse_and_map(np, 2);
	if (pDtsConfig->VeduIrqNumSafe == 0){
		HI_ERR_VENC("%s, irq_of_parse_and_map VeduIrqNumSafe failed!\n", __func__);
		return HI_FAILURE;
	}

	/* 2 read venc register start address, range */
	ret = of_address_to_resource(np, 0, &res);
	if (ret){
		HI_ERR_VENC("%s of_address_to_resource failed! ret = %d\n", __func__, ret);
		return HI_FAILURE;
	}
	pDtsConfig->VencRegBaseAddr = res.start;/*lint !e712 */
	pDtsConfig->VencRegRange    = resource_size(&res);/*lint !e712 */

	/* 3 read venc clk rate [low, high], venc clock */
	pvenc_clk  = devm_clk_get(dev, VENC_CLOCK_NAME);
	if (IS_ERR_OR_NULL(pvenc_clk)){
		HI_ERR_VENC("can not get venc clock, pvenc_clk = 0x%x!\n", pvenc_clk);
		return HI_FAILURE;
	}
	g_PvencClk  = pvenc_clk;

	ret = of_property_read_u32_index(np, VENC_CLK_RATE, 0, &rate_h);
	ret += of_property_read_u32_index(np, VENC_CLK_RATE, 1, &rate_n);
	ret += of_property_read_u32_index(np, VENC_CLK_RATE, 2, &rate_l);
	if (ret){
		HI_ERR_VENC("%s can not get venc rate, return %d\n", __func__, ret);
		return HI_FAILURE;
	}
	pDtsConfig->highRate   = rate_h;
	pDtsConfig->normalRate = rate_n;
	pDtsConfig->lowRate    = rate_l;
	HI_INFO_VENC("%s, venc_clk_rate: highRate:%u, normalRate:%u, lowRate:%u\n", __func__,  pDtsConfig->highRate, pDtsConfig->normalRate, pDtsConfig->lowRate);

	/* 4 fpga platform */
	ret = of_property_read_u32(np, "venc_fpga", &pDtsConfig->IsFPGA);
	if (ret)
		HI_INFO_VENC("%s, read failed!\n", __func__);

	/* 5 get venc qos mode */
	ret = of_property_read_u32(np, "venc_qos_mode", &g_vencQosMode);
	if (ret){
		g_vencQosMode = 0x2;
		HI_ERR_VENC("get venc qos mode failed set default!\n");
	}

	domain_data = (struct iommu_domain_data *)(g_hisi_mmu_domain->priv);
	if (domain_data){
		pDtsConfig->SmmuPageBaseAddr = (uint64_t)(domain_data->phy_pgd_base);
		HI_INFO_VENC("%s, SmmuPageBaseAddr:0x%lx\n", __func__, pDtsConfig->SmmuPageBaseAddr);
	}

	if (pDtsConfig->SmmuPageBaseAddr == 0){
		HI_ERR_VENC("%s, Regulator_GetSmmuBasePhy failed!\n", __func__);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

static HI_S32 Venc_Regulator_Get(struct platform_device *pdev)
{
	HI_INFO_VENC("enter %s!\n", __func__);

	g_VencRegulator = HI_NULL;
	g_VencRegulator = devm_regulator_get(&pdev->dev, REGULATOR_NAME);
	if (IS_ERR_OR_NULL(g_VencRegulator)){
		HI_ERR_VENC("%s, get regulator failed, error no = %ld!\n", __func__, PTR_ERR(g_VencRegulator));
		g_VencRegulator = HI_NULL;

		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_S32 Venc_Regulator_Init(struct platform_device *pdev)
{
	HI_S32 ret = 0;

	if (!pdev){
		HI_ERR_VENC("%s, invalid argument!!\n", __func__);
		return HI_FAILURE;
	}

	/* 1 get regulator interface */
	ret = Venc_Regulator_Get(pdev);
	if (ret < 0){
		HI_ERR_VENC("%s, Venc_Regulator_Get failed!\n", __func__);
		return HI_FAILURE;
	}

	/* 2 create smmu domain */
	ret = Venc_Enable_Iommu(pdev);
	if (ret < 0){
		HI_ERR_VENC("%s, VENC_Enable_Iommu failed!\n", __func__);
		return HI_FAILURE;
	}

	/* 3 read venc dts info from dts */
	HiMemSet(&g_VencDtsConfig, 0, sizeof(VeduEfl_DTS_CONFIG_S));
	ret = Venc_GetDtsConfigInfo(pdev, &g_VencDtsConfig);
	if (ret != HI_SUCCESS){
		HI_ERR_VENC("%s VENC_GetDtsConfigInfo failed.\n", __func__);
		return HI_FAILURE;
	}

	/* 4 set dts into to efi */
	ret = VENC_SetDtsConfig(&g_VencDtsConfig);
	if (ret != HI_SUCCESS){
		HI_ERR_VENC("%s VENC_SetDtsConfig failed.\n", __func__);
		return HI_FAILURE;
	}

	HI_INFO_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;
}

HI_VOID Venc_Regulator_Deinit(struct platform_device *pdev)
{
	if (pdev)
		Venc_Disable_Iommu(pdev);
	return;
}

#ifdef PLATFORM_KIRIN970
static HI_S32 Venc_Regulator_Enable_Kirin970(HI_VOID)
{
	HI_S32 ret = HI_FAILURE;

	if (HI_TRUE == g_VencPowerOn)
	{
		return HI_SUCCESS;
	}

	if(IS_ERR_OR_NULL(g_PvencClk) || IS_ERR_OR_NULL(g_VencRegulator))
	{
		HI_ERR_VENC("invalid_argument g_PvencClk:0x%x, g_VencRegulator:0x%x\n",
					g_PvencClk, g_VencRegulator);
		return HI_FAILURE;
	}

	ret = regulator_enable(g_VencRegulator);
	if (ret != 0)
	{
		HI_ERR_VENC("enable regulator failed!\n");
		return HI_FAILURE;
	}

	ret = clk_prepare_enable(g_PvencClk);
	if (ret != 0)
	{
		HI_ERR_VENC("clk_prepare_enable failed!\n");
		goto on_error_regulator;
	}

	ret = clk_set_rate(g_PvencClk, g_VencDtsConfig.lowRate);
	if(ret != 0)
	{
		HI_ERR_VENC("clk_set_rate lowRate failed!\n");
		goto on_error_regulator;
	}

	g_currClk = CLK_RATE_LOW;
	HI_INFO_VENC("%s, clk_set_rate lowRate:%u\n", __func__, g_VencDtsConfig.lowRate);

	g_VencPowerOn = HI_TRUE;
	HI_INFO_VENC("++\n");
	return HI_SUCCESS;

on_error_regulator:
	regulator_disable(g_VencRegulator);
	return HI_FAILURE;
}
#endif

HI_S32 Venc_Regulator_Enable(HI_VOID)
{
#ifdef PLATFORM_KIRIN970
	return Venc_Regulator_Enable_Kirin970();
#endif
}

#ifdef PLATFORM_KIRIN970
static HI_S32 Venc_Regulator_Disable_Kirin970(HI_VOID)
{
	HI_S32 ret = HI_FAILURE;
	HI_DBG_VENC("%s, Venc_Regulator_Disable g_VencRegulator:0x%x\n", __func__, g_VencRegulator);

	if (HI_FALSE == g_VencPowerOn)
	{
		return HI_SUCCESS;
	}

	if(IS_ERR_OR_NULL(g_PvencClk) || IS_ERR_OR_NULL(g_VencRegulator))
	{
		HI_ERR_VENC("invalid_argument g_PvencClk:0x%x, g_VencRegulator:0x%x\n",
					g_PvencClk, g_VencRegulator);
		return HI_FAILURE;
	}

	ret = clk_set_rate(g_PvencClk, g_VencDtsConfig.lowRate);
	if(ret != 0)
	{
		HI_ERR_VENC("clk_set_rate lowRate:%u failed!\n", g_VencDtsConfig.lowRate);
		//return HI_FAILURE;//continue, no need return
	}
	HI_INFO_VENC("%s, clk_set_rate lowRate:%u\n", __func__, g_VencDtsConfig.lowRate);
	g_currClk = CLK_RATE_LOW;

	clk_disable_unprepare(g_PvencClk);

	ret = regulator_disable(g_VencRegulator);
	if (ret != 0)
	{
		HI_ERR_VENC("disable regulator failed!\n");
	}

	g_VencPowerOn = HI_FALSE;
	HI_INFO_VENC("--\n");

	return HI_SUCCESS;
}/*lint !e715 */
#endif

HI_S32 Venc_Regulator_Disable(HI_VOID)
{
#ifdef PLATFORM_KIRIN970
	return Venc_Regulator_Disable_Kirin970();
#endif
}

HI_S32 Venc_SetClkRate(VENC_CLK_TYPE clk_type)
{
	HI_U64 clk;
	HI_S32 ret  = HI_SUCCESS;

	if (g_currClk != clk_type) {
		switch (clk_type) {
		case CLK_RATE_LOW :
			clk = g_VencDtsConfig.lowRate;
			break;

		case CLK_RATE_NORMAL :
			clk = g_VencDtsConfig.normalRate;
			break;

		case CLK_RATE_HIGH :
			clk = g_VencDtsConfig.highRate;
			break;

		default:
			HI_ERR_VENC("%s, invalid clk clk_type: %d\n", __func__, clk_type);
			return HI_FAILURE;
		}

		HI_INFO_VENC("%s, clk_type%d, clk %u\n", __func__, clk_type, clk);
		if (clk != clk_get_rate(g_PvencClk)) {
			ret = clk_set_rate(g_PvencClk, clk);
			if (ret == 0)
				g_currClk = clk_type;
			else
				HI_ERR_VENC("clk_set_rate high failed\n");
		}
	}

	HI_DBG_VENC("clk_set_rate high success\n");
	return ret;
}
