/*
 * PCIe host controller driver for Kirin SoCs
 *
 * Copyright (C) 2015 Hilisicon Electronics Co., Ltd.
 *		http://www.huawei.com
 *
 * Author: Xiaowei Song <songxiaowei@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "pcie-kirin.h"
#include "pcie-kirin-common.h"
/*lint -e438 -e550 -e713 -e732 -e737 -e774 -e838  -esym(438,*) -esym(550,*) -esym(713,*) -esym(732,*) -esym(737,*) -esym(774,*) -esym(838,*) */
unsigned int g_rc_num;

struct kirin_pcie g_kirin_pcie[] = {
	{
		.irq = {
				{
					.name = "kirin-pcie0-inta",
				},
				{
					.name = "kirin-pcie0-msi",
				},
				{
					.name = "kirin-pcie0-intc",
				},
				{
					.name = "kirin-pcie0-intd",
				},
				{
					.name = "kirin-pcie0-linkdown",
				}
			},
	},
#ifdef CONFIG_KIRIN_PCIE_KIRIN970
	{
		.irq = {
				{
					.name = "kirin-pcie1-inta",
				},
				{
					.name = "kirin-pcie1-msi",
				},
				{
					.name = "kirin-pcie1-intc",
				},
				{
					.name = "kirin-pcie1-intd",
				},
				{
					.name = "kirin-pcie1-linkdown",
				}
			},
	},
#endif
};

static int kirin_pcie_link_up(struct pcie_port *pp);

void kirin_elb_writel(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	writel(val, pcie->apb_base + reg);
}

u32 kirin_elb_readl(struct kirin_pcie *pcie, u32 reg)
{
	return readl(pcie->apb_base + reg);
}

void kirin_phy_writel(struct kirin_pcie *pcie, u32 val, u32 reg)
{
	writel(val, pcie->phy_base + reg);
}

u32 kirin_phy_readl(struct kirin_pcie *pcie, u32 reg)
{
	return readl(pcie->phy_base + reg);
}

static int32_t kirin_pcie_get_clk(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	pcie->phy_ref_clk = devm_clk_get(&pdev->dev, "pcie_phy_ref");
	if (IS_ERR(pcie->phy_ref_clk)) {
		PCIE_PR_ERR("Failed to get pcie_phy_ref clock");
		return PTR_ERR(pcie->phy_ref_clk);
	}

	pcie->pcie_aux_clk = devm_clk_get(&pdev->dev, "pcie_aux");
	if (IS_ERR(pcie->pcie_aux_clk)) {
		PCIE_PR_ERR("Failed to get pcie_aux clock");
		return PTR_ERR(pcie->pcie_aux_clk);
	}

	pcie->apb_phy_clk = devm_clk_get(&pdev->dev, "pcie_apb_phy");
	if (IS_ERR(pcie->apb_phy_clk)) {
		PCIE_PR_ERR("Failed to get pcie_apb_phy clock");
		return PTR_ERR(pcie->apb_phy_clk);
	}

	pcie->apb_sys_clk = devm_clk_get(&pdev->dev, "pcie_apb_sys");
	if (IS_ERR(pcie->apb_sys_clk)) {
		PCIE_PR_ERR("Failed to get pcie_apb_sys clock");
		return PTR_ERR(pcie->apb_sys_clk);
	}

	pcie->pcie_aclk = devm_clk_get(&pdev->dev, "pcie_aclk");
	if (IS_ERR(pcie->pcie_aclk)) {
		PCIE_PR_ERR("Failed to get pcie_aclk clock");
		return PTR_ERR(pcie->pcie_aclk);
	}

	PCIE_PR_INFO("Successed to get all clock");

	return 0;
}

static int32_t kirin_pcie_get_baseaddr(struct pcie_port *pp, struct platform_device *pdev)
{
	struct resource *apb;
	struct resource *phy;
	struct resource *dbi;
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
	struct device_node *np;

	apb = platform_get_resource_byname(pdev, IORESOURCE_MEM, "apb");
	pcie->apb_base = devm_ioremap_resource(&pdev->dev, apb);
	if (IS_ERR(pcie->apb_base)) {
		PCIE_PR_ERR("Failed to get PCIeCTRL apb base");
		return PTR_ERR(pcie->apb_base);
	}

	phy = platform_get_resource_byname(pdev, IORESOURCE_MEM, "phy");
	pcie->phy_base = devm_ioremap_resource(&pdev->dev, phy);
	if (IS_ERR(pcie->phy_base)) {
		PCIE_PR_ERR("Failed to get PCIePHY base");
		return PTR_ERR(pcie->phy_base);
	}

	dbi = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	pp->dbi_base = devm_ioremap_resource(&pdev->dev, dbi);
	if (IS_ERR(pp->dbi_base)) {
		PCIE_PR_ERR("Failed to get PCIe dbi base");
		return PTR_ERR(pp->dbi_base);
	}

	np = pdev->dev.of_node;
	if (of_property_read_u32(np, "iatu_base_offset", &pcie->dtsinfo.iatu_base_offset)) {
		PCIE_PR_ERR("Failed to get iatu_base_offset info");
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,crgctrl");
	if (!np) {
		PCIE_PR_ERR("Failed to get crgctrl node ");
		return -1;
	}
	pcie->crg_base = of_iomap(np, 0);
	if (!pcie->crg_base) {
		PCIE_PR_ERR("Failed to iomap crg_base iomap");
		return -1;
	}

	np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (!np) {
		PCIE_PR_ERR("Failed to get sysctrl Node ");
		return -1;
	}
	pcie->sctrl_base = of_iomap(np, 0);
	if (!pcie->sctrl_base) {
		PCIE_PR_ERR("Failed to iomap sctrl_base");
		return -1;
	}

	PCIE_PR_INFO("Successed to get all resource");
	return 0;
}

static int32_t kirin_pcie_get_pinctrl(struct kirin_pcie *pcie, struct platform_device *pdev)
{
        int gpio_id;

        gpio_id = of_get_named_gpio(pdev->dev.of_node, "reset-gpio", 0);
        if (gpio_id < 0) {
                PCIE_PR_ERR("Failed to get perst gpio number");
                return -1;
        }

        pcie->gpio_id_reset = gpio_id;

        return 0;
}
static void kirin_pcie_get_boardtype(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	int len;
	struct device_node *np;
	struct kirin_pcie_dtsinfo *dtsinfo;

	np = pdev->dev.of_node;
	dtsinfo = &pcie->dtsinfo;
	if (of_property_read_u32(np, "board_type", &dtsinfo->board_type)) {
		PCIE_PR_ERR("Failed to get board_type");
		dtsinfo->board_type = 2;
	}
	PCIE_PR_INFO("The board_type value is [%d] ", dtsinfo->board_type);

	if (of_property_read_u32(np, "chip_type", &dtsinfo->chip_type)) {
		PCIE_PR_ERR("Failed to get chip_type");
		dtsinfo->chip_type = 0;
	}
	PCIE_PR_INFO("The chip_type value is [%d] ", dtsinfo->chip_type);

	if (of_find_property(np, "ep_flag", &len)) {
		dtsinfo->ep_flag = 1;
		PCIE_PR_INFO("EndPoint Device");
	} else {
		dtsinfo->ep_flag = 0;
		PCIE_PR_INFO("RootComplex");
	}
}

static void kirin_pcie_get_eyeparam(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	int ret;
	u32 eye_param = 0;
	struct device_node *np;
	struct kirin_pcie_dtsinfo *dtsinfo;

	np = pdev->dev.of_node;
	dtsinfo = &pcie->dtsinfo;
	ret = of_property_read_u32(np, "eye_param_ctrl2", &eye_param);
	if (ret)
		dtsinfo->pcie_eye_param_ctrl2 = 0x0;
	else
		dtsinfo->pcie_eye_param_ctrl2 = eye_param;

	ret = of_property_read_u32(np, "eye_param_ctrl3", &eye_param);
	if (ret)
		dtsinfo->pcie_eye_param_ctrl3 = 0x0;
	else
		dtsinfo->pcie_eye_param_ctrl3 = eye_param;

	PCIE_PR_INFO("eye_param_2 = [0x%x] ", dtsinfo->pcie_eye_param_ctrl2);
	PCIE_PR_INFO("eye_param_3 = [0x%x] ", dtsinfo->pcie_eye_param_ctrl3);
}

static int32_t kirin_pcie_get_isoinfo(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	size_t array_num = 2;
	struct device_node *np;
	struct kirin_pcie_dtsinfo *dtsinfo;
	u32 val[2] = {0, 0};

	np = pdev->dev.of_node;
	dtsinfo = &pcie->dtsinfo;

	if (of_property_read_u32_array(np, "isoen", val, array_num)) {
		PCIE_PR_ERR("Failed to get isoen info");
		return -1;
	}
	dtsinfo->isoen_offset = val[0];
	dtsinfo->isoen_value = val[1];

	if (of_property_read_u32_array(np, "isodis", val, array_num)) {
		PCIE_PR_ERR("Failed to get isodis info");
		return -1;
	}
	dtsinfo->isodis_offset = val[0];
	dtsinfo->isodis_value = val[1];

	return 0;
}

void kirin_pcie_iso_ctrl(struct kirin_pcie *pcie, int en_flag)
{
	if (en_flag)
		writel(pcie->dtsinfo.isoen_value, pcie->sctrl_base + pcie->dtsinfo.isoen_offset);
	else
		writel(pcie->dtsinfo.isodis_value, pcie->sctrl_base + pcie->dtsinfo.isodis_offset);
}

static int32_t kirin_pcie_get_assertinfo(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	size_t array_num = 2;
	struct device_node *np;
	struct kirin_pcie_dtsinfo *dtsinfo;
	u32 val[2] = {0, 0};

	np = pdev->dev.of_node;
	dtsinfo = &pcie->dtsinfo;

	if (of_property_read_u32_array(np, "phy_assert", val, array_num)) {
		PCIE_PR_ERR("Failed to get phy_assert info");
		return -1;
	}
	dtsinfo->phy_assert_offset = val[0];
	dtsinfo->phy_assert_value = val[1];

	if (of_property_read_u32_array(np, "phy_deassert", val, array_num)) {
		PCIE_PR_ERR("Failed to get phy_deassert info");
		return -1;
	}
	dtsinfo->phy_deassert_offset = val[0];
	dtsinfo->phy_deassert_value = val[1];

	if (of_property_read_u32_array(np, "core_assert", val, array_num)) {
		PCIE_PR_ERR("Failed to get phy_assert info");
		return -1;
	}
	dtsinfo->core_assert_offset = val[0];
	dtsinfo->core_assert_value = val[1];

	if (of_property_read_u32_array(np, "core_deassert", val, array_num)) {
		PCIE_PR_ERR("Failed to get phy_deassert info");
		return -1;
	}
	dtsinfo->core_deassert_offset = val[0];
	dtsinfo->core_deassert_value = val[1];

	return 0;
}

static void kirin_pcie_get_linkstate(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo;

	dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(pdev->dev.of_node, "ep_ltr_latency", &dtsinfo->ep_ltr_latency);
	if (ret) {
		PCIE_PR_DEBUG("Not to set ep ltr_latency ?");
		dtsinfo->ep_ltr_latency = 0x0;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "ep_l1ss_ctrl2", &dtsinfo->ep_l1ss_ctrl2);
	if (ret) {
		PCIE_PR_DEBUG("Not to set ep L1ss_ctrl2 ?");
		dtsinfo->ep_l1ss_ctrl2 = 0x0;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "l1ss_ctrl1", &dtsinfo->l1ss_ctrl1);
	if (ret) {
		PCIE_PR_DEBUG("Not to set L1ss_ctrl1 ?");
		dtsinfo->l1ss_ctrl1 = 0x0;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "aspm_state", &dtsinfo->aspm_state);
	if (ret) {
		PCIE_PR_DEBUG("Not to set aspm_state ?");
		dtsinfo->aspm_state = ASPM_L1;
	}

	PCIE_PR_DEBUG(" ltr_latency = [0x%x], l1ss_ctrl2 = [0x%x] ",
			dtsinfo->ep_ltr_latency, dtsinfo->ep_l1ss_ctrl2);
	PCIE_PR_DEBUG(" l1ss_ctrl1 = [0x%x], aspm_state = [0x%x] ",
			dtsinfo->l1ss_ctrl1, dtsinfo->aspm_state);
}

static void kirin_pcie_get_eco(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo;

	dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(pdev->dev.of_node, "eco", &dtsinfo->eco);
	if (ret) {
		PCIE_PR_DEBUG("Not choose SRAM ECO");
		dtsinfo->eco = 0x0;
	}

	PCIE_PR_DEBUG(" set eco to [0x%x]", dtsinfo->eco);
}

static void kirin_pcie_get_fts(struct kirin_pcie *pcie, struct platform_device *pdev)
{
	int ret;
	struct kirin_pcie_dtsinfo *dtsinfo;

	dtsinfo = &pcie->dtsinfo;

	ret = of_property_read_u32(pdev->dev.of_node, "n_fts", &dtsinfo->n_fts);
	if (ret) {
		PCIE_PR_DEBUG("n_fts is not set in dts");
		dtsinfo->n_fts = 0x0;
	}

	ret = of_property_read_u32(pdev->dev.of_node, "ack_fts", &dtsinfo->ack_fts);
	if (ret) {
		PCIE_PR_DEBUG("ack_fts is not set in dts");
		dtsinfo->ack_fts = 0x0;
	}

	PCIE_PR_DEBUG(" set n_fts to [0x%x], ack_fts to [0x%x]", dtsinfo->n_fts, dtsinfo->ack_fts);
}


static int32_t kirin_pcie_get_dtsinfo(u32 *rc_id, struct platform_device *pdev)
{
	struct pcie_port *pp;
	struct kirin_pcie *pcie;
	struct device_node *np;

	if (!pdev->dev.of_node) {
		PCIE_PR_ERR("Of_node is null");
		return -EINVAL;
	}

	if (of_property_read_u32(pdev->dev.of_node, "rc-id", rc_id)) {
		dev_err(&pdev->dev, "Failed to get rc_id info\n");
		return -EINVAL;
	}

	np = of_find_node_by_name(NULL, "kirin_pcie");
	if (!np) {
		PCIE_PR_ERR("Failed to get kirin_pcie info");
		return -1;
	}

	if (of_property_read_u32(np, "rc_num", &g_rc_num)) {
		PCIE_PR_ERR("Failed to get rc_num info");
		return -1;
	}

	if (*rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", *rc_id);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[*rc_id];
	pcie->rc_id = *rc_id;
	pp = &pcie->pp;

	kirin_pcie_get_boardtype(pcie, pdev);

	kirin_pcie_get_eyeparam(pcie, pdev);

	kirin_pcie_get_linkstate(pcie, pdev);

	kirin_pcie_get_eco(pcie, pdev);

	kirin_pcie_get_fts(pcie, pdev);

	if (kirin_pcie_get_isoinfo(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_assertinfo(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_clk(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_pinctrl(pcie, pdev))
		return -ENODEV;

	if (kirin_pcie_get_baseaddr(pp, pdev))
		return -ENODEV;

	return 0;
}

static void kirin_pcie_sideband_dbi_w_mode(struct pcie_port *pp, bool on)
{
	u32 val;
	struct kirin_pcie *pcie = to_kirin_pcie(pp);

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL0_ADDR);
	if (on)
		val = val | PCIE_ELBI_SLV_DBI_ENABLE;
	else
		val = val & ~PCIE_ELBI_SLV_DBI_ENABLE;

	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL0_ADDR);
}

static void kirin_pcie_sideband_dbi_r_mode(struct pcie_port *pp, bool on)
{
	u32 val;
	struct kirin_pcie *pcie = to_kirin_pcie(pp);

	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL1_ADDR);
	if (on)
		val = val | PCIE_ELBI_SLV_DBI_ENABLE;
	else
		val = val & ~PCIE_ELBI_SLV_DBI_ENABLE;

	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL1_ADDR);
}

static void kirin_pcie_set_fts(struct pcie_port *pp)
{
	u32 val;
	struct kirin_pcie *pcie = to_kirin_pcie(pp);//lint !e826

	PCIE_PR_INFO("++");

	/* set ack_req for aspm */
	if (pcie->dtsinfo.ack_fts) {
		kirin_pcie_rd_own_conf(pp, PCIE_ACK_FREQ_ASPM_CTRL, 4, &val);
		val &= ~PCIE_ACK_FREQ_ASPM_MASK;
		val |= pcie->dtsinfo.ack_fts;
		kirin_pcie_wr_own_conf(pp, PCIE_ACK_FREQ_ASPM_CTRL, 4, val);
	}

	/* set N_FTS */
	if (pcie->dtsinfo.n_fts) {
		kirin_pcie_rd_own_conf(pp, PCIE_GEN2_CTRL_OFF, 4, &val);
		val &= ~PCIE_GEN2_CTRL_MASK;
		val |= pcie->dtsinfo.n_fts;
		kirin_pcie_wr_own_conf(pp, PCIE_GEN2_CTRL_OFF, 4, val);
	}

	PCIE_PR_INFO("--");
}

static int kirin_pcie_establish_link(struct pcie_port *pp)
{
	int count = 0;
	struct kirin_pcie *pcie = to_kirin_pcie(pp);

	PCIE_PR_INFO("++");

	if (kirin_pcie_link_up(pp)) {
		PCIE_PR_ERR("Link already up");
		return 0;
	}

	kirin_pcie_set_fts(pp);

	/* setup root complex */
	dw_pcie_setup_rc(pp);
	PCIE_PR_DEBUG("Setup rc done ");

	/* assert LTSSM enable */
	kirin_elb_writel(pcie, PCIE_LTSSM_ENABLE_BIT,
			  PCIE_APP_LTSSM_ENABLE);

	/* check if the link is up or not */
	while (!kirin_pcie_link_up(pp)) {
		mdelay(1);
		count++;
		if (count == 200) {
			PCIE_PR_ERR("Link Fail, Reg should be 0x11 or 0x41 not [0x%x] ",
				 kirin_elb_readl(pcie, SOC_PCIECTRL_STATE4_ADDR));
			dsm_pcie_dump_info(pcie, DSM_ERR_ESTABLISH_LINK);
			return -EINVAL;
		}
	}

	PCIE_PR_INFO("PCIe Link success ");
	return 0;
}

/*EP rigist hook fun for link event notification*/
int kirin_pcie_register_event(struct kirin_pcie_register_event *reg)
{
	int ret = 0;
	struct pci_dev *dev;
	struct pcie_port *pp;
	struct kirin_pcie *pcie;

	if (!reg || !reg->user) {
		PCIE_PR_INFO("Event registration or user of event is null");
		return -ENODEV;
	}

	dev = (struct pci_dev *)reg->user;
	pp = (struct pcie_port *)(dev->bus->sysdata);
	/*lint -e826 -esym(826,*)*/
	pcie = container_of(pp, struct kirin_pcie, pp);
	/*lint -e826 +esym(826,*)*/

	if (pp) {
		pcie->event_reg = reg;
		PCIE_PR_INFO("Event 0x%x is registered for RC", reg->events);
	} else {
		PCIE_PR_INFO("PCIe: did not find RC for pci endpoint device");
		ret = -ENODEV;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(kirin_pcie_register_event);

int kirin_pcie_deregister_event(struct kirin_pcie_register_event *reg)
{
	int ret = 0;
	struct pci_dev *dev;
	struct pcie_port *pp;
	struct kirin_pcie *pcie;

	if (!reg || !reg->user) {
		PCIE_PR_INFO("Event registration or user of event is NULL");
		return -ENODEV;
	}

	dev = (struct pci_dev *)reg->user;
	pp = (struct pcie_port *)(dev->bus->sysdata);
	/*lint -e826 -esym(826,*)*/
	pcie = container_of(pp, struct kirin_pcie, pp);
	/*lint -e826 +esym(826,*)*/

	if (pp) {
		pcie->event_reg = NULL;
		PCIE_PR_INFO("deregistered ");
	} else {
		PCIE_PR_INFO("No RC for this EP device ");
		ret = -ENODEV;
	}
	return ret;
}
EXPORT_SYMBOL_GPL(kirin_pcie_deregister_event);

/*print apb and cfg-register of RC*/
static void dump_link_register(struct kirin_pcie *pcie)
{
	struct pcie_port *pp = &pcie->pp;
	int i;
	u32 j;
	u32 val0, val1, val2, val3;

	if (!pcie->is_power_on)
		return;

	PCIE_PR_INFO("####DUMP RC CFG Register ");
	for (i = 0; i < 0x18; i++) {
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0x0, 4, &val0);
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0x4, 4, &val1);
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0x8, 4, &val2);
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0xC, 4, &val3);
		printk(KERN_INFO "0x%-8x: %8x %8x %8x %8x \n", 0x10 * i, val0, val1, val2, val3);
	}
	for (i = 0; i < 6; i++) {
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0x0 + 0x700, 4, &val0);
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0x4 + 0x700, 4, &val1);
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0x8 + 0x700, 4, &val2);
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0xC + 0x700, 4, &val3);
		printk(KERN_INFO "0x%-8x: %8x %8x %8x %8x \n", 0x10 * i + 0x700, val0, val1, val2, val3);
	}
	for (i = 0; i < 0x9; i++) {
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0x0 + 0x8A0, 4, &val0);
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0x4 + 0x8A0, 4, &val1);
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0x8 + 0x8A0, 4, &val2);
		kirin_pcie_rd_own_conf(pp, 0x10 * i + 0xC + 0x8A0, 4, &val3);
		printk(KERN_INFO "0x%-8x: %8x %8x %8x %8x \n", 0x10 * i + 0x8A0, val0, val1, val2, val3);
	}



	PCIE_PR_INFO("####DUMP APB CORE Register : ");
	for (j = 0; j < 0x4; j++) {
		printk(KERN_INFO "0x%-8x: %8x %8x %8x %8x \n",
			0x10 * j,
			kirin_elb_readl(pcie, 0x10 * j + 0x0),
			kirin_elb_readl(pcie, 0x10 * j + 0x4),
			kirin_elb_readl(pcie, 0x10 * j + 0x8),
			kirin_elb_readl(pcie, 0x10 * j + 0xC));
	}
	for (j = 0; j < 0x2; j++) {
		printk(KERN_INFO "0x%-8x: %8x %8x %8x %8x \n",
			0x10 * j + 0x400,
			kirin_elb_readl(pcie, 0x10 * j + 0x0 + 0x400),
			kirin_elb_readl(pcie, 0x10 * j + 0x4 + 0x400),
			kirin_elb_readl(pcie, 0x10 * j + 0x8 + 0x400),
			kirin_elb_readl(pcie, 0x10 * j + 0xC + 0x400));
	}


	PCIE_PR_INFO("####DUMP APB PHY Register : ");
	printk(KERN_INFO "0x%-8x: %8x %8x %8x %8x %8x ",
		0x0,
		kirin_phy_readl(pcie, 0x0),
		kirin_phy_readl(pcie, 0x4),
		kirin_phy_readl(pcie, 0x8),
		kirin_phy_readl(pcie, 0xc),
		kirin_phy_readl(pcie, 0x400));
	printk("\n");

	return;

}


/*notify EP about link-down event*/
static void kirin_pcie_notify_callback(struct kirin_pcie *pcie, enum kirin_pcie_event event)
{
	if ((pcie->event_reg != NULL) && (pcie->event_reg->callback != NULL) &&
			(pcie->event_reg->events & event)) {
		struct kirin_pcie_notify *notify = &pcie->event_reg->notify;
		notify->event = event;
		notify->user = pcie->event_reg->user;
		PCIE_PR_INFO("Callback for the event : %d", event);
		pcie->event_reg->callback(notify);
	} else {
		PCIE_PR_INFO("EP does not register this event : %d", event);
	}
}

static void kirin_handle_work(struct work_struct *work)
{
	/*lint -e826 -esym(826,*)*/
	struct kirin_pcie *pcie = container_of(work, struct kirin_pcie, handle_work);
	/*lint -e826 +esym(826,*)*/

	dsm_pcie_dump_info(pcie, DSM_ERR_LINK_DOWN);

	kirin_pcie_notify_callback(pcie, KIRIN_PCIE_EVENT_LINKDOWN);

	dump_link_register(pcie);
}


static irqreturn_t kirin_pcie_linkdown_irq_handler(int irq, void *arg)
{
	struct pcie_port *pp = arg;
	struct kirin_pcie *pcie = to_kirin_pcie(pp);

	PCIE_PR_ERR("Triggle linkdown irq[%d]", irq);

	schedule_work(&pcie->handle_work);

	return IRQ_HANDLED;
}
static irqreturn_t kirin_pcie_msi_irq_handler(int irq, void *arg)
{
	struct pcie_port *pp = arg;

	PCIE_PR_ERR("Triggle msi irq[%d]", irq);
	return dw_handle_msi_irq(pp);
}

#ifdef CONFIG_KIRIN_PCIE_TEST
static irqreturn_t kirin_pcie_intx_irq_handler(int irq, void *arg)
{
	PCIE_PR_ERR("Triggle intx irq[%d]", irq);
	return IRQ_HANDLED;
}
#endif

static void kirin_pcie_msi_init(struct pcie_port *pp)
{
	dw_pcie_msi_init(pp);

}

static void kirin_pcie_enable_interrupts(struct pcie_port *pp)
{
	if (IS_ENABLED(CONFIG_PCI_MSI))
		kirin_pcie_msi_init(pp);
}

void kirin_pcie_readl_rc(struct pcie_port *pp,
					void __iomem *dbi_base, u32 *val)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
	if (!pcie->is_power_on)
		return;

	kirin_pcie_sideband_dbi_r_mode(pp, true);
	*val = readl(dbi_base);
	kirin_pcie_sideband_dbi_r_mode(pp, false);
}

void kirin_pcie_writel_rc(struct pcie_port *pp,
					u32 val, void __iomem *dbi_base)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
	if (!pcie->is_power_on)
		return;

	kirin_pcie_sideband_dbi_w_mode(pp, true);
	writel(val, dbi_base);
	kirin_pcie_sideband_dbi_w_mode(pp, false);
}

int kirin_pcie_rd_own_conf(struct pcie_port *pp, int where, int size,
				u32 *val)
{
	int ret;
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
	if (!pcie->is_power_on)
		return -EINVAL;

	kirin_pcie_sideband_dbi_r_mode(pp, true);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
	ret = dw_pcie_cfg_read(pp->dbi_base + (where & ~0x3), where, size, val);
#else
	ret = dw_pcie_cfg_read(pp->dbi_base + where, size, val);
#endif
	kirin_pcie_sideband_dbi_r_mode(pp, false);
	return ret;
}

int kirin_pcie_wr_own_conf(struct pcie_port *pp, int where, int size,
				u32 val)
{
	int ret;
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
	if (!pcie->is_power_on)
		return -EINVAL;

	kirin_pcie_sideband_dbi_w_mode(pp, true);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
	ret = dw_pcie_cfg_write(pp->dbi_base + (where & ~0x3),
			where, size, val);
#else
	ret = dw_pcie_cfg_write(pp->dbi_base + where, size, val);
#endif
	kirin_pcie_sideband_dbi_w_mode(pp, false);
	return ret;
}

static int kirin_pcie_link_up(struct pcie_port *pp)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pp);
	u32 val;

	if (!pcie->is_power_on || pcie->usr_suspend)
		return 0;

	val = kirin_elb_readl(pcie, PCIE_ELBI_RDLH_LINKUP);

	if ((val & PCIE_LINKUP_ENABLE) == PCIE_LINKUP_ENABLE)
		return 1;

	return 0;
}

static int kirin_pcie_host_init(struct pcie_port *pp)
{
	if (kirin_pcie_establish_link(pp))
		return -1;

	kirin_pcie_enable_interrupts(pp);

	return 0;
}

static struct pcie_host_ops kirin_pcie_host_ops = {
	.readl_rc = kirin_pcie_readl_rc,
	.writel_rc = kirin_pcie_writel_rc,
	.rd_own_conf = kirin_pcie_rd_own_conf,
	.wr_own_conf = kirin_pcie_wr_own_conf,
	.link_up = kirin_pcie_link_up,
	.host_init = kirin_pcie_host_init,
};

static int __init kirin_add_pcie_port(struct pcie_port *pp,
					   struct platform_device *pdev)
{
	int ret;
	int index;
	struct kirin_pcie *pcie = to_kirin_pcie(pp);

	PCIE_PR_INFO("++");
	for (index = 0; index < MAX_IRQ_NUM; index++) {
		pcie->irq[index].num = platform_get_irq(pdev, index);
		if (!pcie->irq[index].num) {
			PCIE_PR_ERR("Failed to get [%s] irq ,num = [%d]", pcie->irq[index].name,
				pcie->irq[index].num);
			return -ENODEV;
		}
	}
#ifdef CONFIG_KIRIN_PCIE_TEST
	ret = devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_INTC].num,
				kirin_pcie_intx_irq_handler,
				(unsigned long)IRQF_TRIGGER_RISING, pcie->irq[IRQ_INTC].name, pp);
	ret |= devm_request_irq(&pdev->dev, (unsigned int)pcie->irq[IRQ_INTD].num,
				kirin_pcie_intx_irq_handler,
				(unsigned long)IRQF_TRIGGER_RISING, pcie->irq[IRQ_INTD].name, pp);
	if (ret) {
		PCIE_PR_ERR("Failed to request intx irq");
		return ret;
	}
#endif

	ret = devm_request_irq(&pdev->dev, pcie->irq[IRQ_LINKDOWN].num,
				kirin_pcie_linkdown_irq_handler,
				IRQF_TRIGGER_RISING, pcie->irq[IRQ_LINKDOWN].name, pp);
	if (ret) {
		PCIE_PR_ERR("Failed to request linkdown irq");
		return ret;
	}

	PCIE_PR_INFO("pcie->irq[1].name = [%s], pcie->irq[4].name = [%s]",
				 pcie->irq[IRQ_MSI].name, pcie->irq[IRQ_LINKDOWN].name);

	if (IS_ENABLED(CONFIG_PCI_MSI)) {
		pp->msi_irq = pcie->irq[IRQ_MSI].num;
		ret = devm_request_irq(&pdev->dev, pp->msi_irq,
					kirin_pcie_msi_irq_handler,
					IRQF_SHARED | IRQF_TRIGGER_RISING, pcie->irq[IRQ_MSI].name, pp);
		if (ret) {
			PCIE_PR_ERR("Failed to request msi irq");
			return ret;
		}
	}

	pp->ops = &kirin_pcie_host_ops;

	PCIE_PR_INFO("Add pcie port sucessed ");
	PCIE_PR_INFO("--");
	return 0;
}


#ifdef CONFIG_KIRIN_PCIE_TEST
static int kirin_pcie_ep_init(struct pci_dev *pdev,
							const struct pci_device_id *ent)
{
	int ret;

	PCIE_PR_DEBUG("++");
	PCIE_PR_DEBUG("Pci dev on bus[0x%d] has dev_id[0x%x] and ven_id[0x%x]",
			pdev->bus->number, ent->device, ent->vendor);

	ret = pci_enable_device(pdev);
	if (ret) {
		PCIE_PR_ERR("Failed to enable pci device");
		return ret;
	}
	pci_set_master(pdev);

	PCIE_PR_DEBUG("--");
	return 0;
}

static struct pci_device_id kirin_pci_devid[] = {
	{ .vendor = 0x19e5,
	.device = 0x0,
	.subvendor = PCI_ANY_ID,
	.subdevice = PCI_ANY_ID,
	.class = 0x028000,
	.class_mask = 0xffff00,
	.driver_data = 0,
	},
	{ .vendor = 0x19e5,
	.device = 0x1,
	.subvendor = PCI_ANY_ID,
	.subdevice = PCI_ANY_ID,
	.class = 0x028000,
	.class_mask = 0xffff00,
	.driver_data = 0,
	},
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, kirin_pci_devid);

static struct pci_driver kirin_pcie_ep_driver = {
	node:		{},
	name :		"kirin-pcie-ep",
	id_table :	kirin_pci_devid,
	probe :		kirin_pcie_ep_init,
};

#endif

static int kirin_pcie_probe(struct platform_device *pdev)
{
	struct kirin_pcie *pcie;
	struct pcie_port *pp;
	int ret;
	u32 rc_id;
	struct kirin_pcie_dtsinfo *dtsinfo;

	PCIE_PR_INFO("++");

	if (kirin_pcie_get_dtsinfo(&rc_id, pdev)) {
		PCIE_PR_ERR("Failed to get dts info");
		return -EINVAL;
	}

	PCIE_PR_INFO("PCIe No.%d probe", rc_id);

	pcie = &g_kirin_pcie[rc_id];
	pcie->rc_dev = NULL;
	pcie->ep_dev = NULL;
	pp = &pcie->pp;
	pp->dev = &(pdev->dev);
	dtsinfo = &pcie->dtsinfo;

	if (gpio_request((unsigned int)pcie->gpio_id_reset, "pcie_reset")) {
		PCIE_PR_ERR("Failed to request gpio-%d", pcie->gpio_id_reset);
		return -1;
	}

	if (dtsinfo->ep_flag) {
		PCIE_PR_INFO("@@@EP Mode");
		ret = kirin_pcie_power_on(pp, RC_POWER_ON);
		if (ret) {
			PCIE_PR_ERR("Failed to power on EP ");
			return -EINVAL;
		}
		kirin_elb_writel(pcie, 0x200000, 0x0);
		writel(0x10023, pp->dbi_base + 0x710);
		writel(0x10001, pp->dbi_base + 0xa0);
		kirin_elb_writel(pcie, 0x800, 0x1c);
		kirin_pcie_sideband_dbi_r_mode(pp, 1);
		kirin_pcie_sideband_dbi_w_mode(pp, 1);
		ret = 0x19e5 + (rc_id << 16);
		writel(ret, pp->dbi_base);
		ret = readl(pcie->pp.dbi_base);
		PCIE_PR_INFO("DevID&VendorID is [0x%x]", ret);
		/*Modify EP device class from 0x0(unsupport) to net devices*/
		ret = readl(pcie->pp.dbi_base + 0x8);
		ret |= 0x02800000;
		writel(ret, pp->dbi_base + 0x8);
		ret = readl(pcie->pp.dbi_base + 0x8);
		PCIE_PR_INFO("Device class is [0x%x]", ret);

		pp->ops = &kirin_pcie_host_ops;
	} else {
		PCIE_PR_INFO("###RC Mode");
		pcie->is_enumerated = 0;
		pcie->is_power_on = 0;
		pcie->usr_suspend = 0;
		ret = kirin_add_pcie_port(pp, pdev);
		if (ret < 0) {
			PCIE_PR_ERR("Failed to assign resource, ret=[%d]", ret);
			return ret;
		}
	}

	platform_set_drvdata(pdev, pcie);

	INIT_WORK(&pcie->handle_work, kirin_handle_work);

#if defined(CONFIG_KIRIN_PCIE_TEST) && defined(CONFIG_DEBUG_FS)
	pcie_debug_init(pcie);
#endif
	PCIE_PR_INFO("--");
	return 0;

}

static int kirin_pcie_save_rc_cfg(struct kirin_pcie *pcie)
{
	struct pcie_port *pp;
	u32 val = 0;
	int ret;

	pp = &(pcie->pp);

	kirin_pcie_rd_own_conf(pp, PORT_MSI_CTRL_ADDR, 4, &val);
	pcie->msi_controller_config[0] = val;
	kirin_pcie_rd_own_conf(pp, PORT_MSI_CTRL_UPPER_ADDR, 4, &val);
	pcie->msi_controller_config[1] = val;
	kirin_pcie_rd_own_conf(pp, PORT_MSI_CTRL_INT0_ENABLE, 4, &val);
	pcie->msi_controller_config[2] = val;

	ret = pci_save_state(pcie->rc_dev);
	if (ret) {
		PCIE_PR_ERR("Failed to save state of RC.");
		return -1;
	}
	pcie->rc_saved_state = pci_store_saved_state(pcie->rc_dev);

	return 0;
}


static int kirin_pcie_restore_rc_cfg(struct kirin_pcie *pcie)
{
	struct pcie_port *pp;

	pp = &(pcie->pp);

	kirin_pcie_wr_own_conf(pp, PORT_MSI_CTRL_ADDR, 4, pcie->msi_controller_config[0]);
	kirin_pcie_wr_own_conf(pp, PORT_MSI_CTRL_UPPER_ADDR, 4, pcie->msi_controller_config[1]);
	kirin_pcie_wr_own_conf(pp, PORT_MSI_CTRL_INT0_ENABLE, 4, pcie->msi_controller_config[2]);

	pci_load_saved_state(pcie->rc_dev, pcie->rc_saved_state);
	pci_restore_state(pcie->rc_dev);
	pci_load_saved_state(pcie->rc_dev, pcie->rc_saved_state);

	return 0;
}


static int kirin_pcie_shutdown_prepare(struct pci_dev *dev)
{
	int ret;
	u32 pm;
	u32 value;
	int index = 0;
	struct pcie_port *pp;
	struct kirin_pcie *pcie;

	PCIE_PR_INFO("++");

	if (!dev) {
		PCIE_PR_ERR("pci_dev is null");
		return -1;
	}
	pp = dev->sysdata;
	pcie = to_kirin_pcie(pp);

	/*Enable PME*/
	pm = pci_find_capability(dev, PCI_CAP_ID_PM);
	if (!pm) {
		PCIE_PR_ERR("Failed to get PCI_CAP_ID_PM");
		return -1;
	}
	kirin_pcie_rd_own_conf(pp, pm + PCI_PM_CTRL, 4, &value);
	value |= 0x100;
	kirin_pcie_wr_own_conf(pp, pm + PCI_PM_CTRL, 4, value);

	/*Broadcast PME_turn_off MSG*/
	ret = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	ret |= PME_TURN_OFF_BIT;
	kirin_elb_writel(pcie, ret, SOC_PCIECTRL_CTRL7_ADDR);

	do {
		ret = kirin_elb_readl(pcie, SOC_PCIECTRL_STATE1_ADDR);
		ret = ret & PME_ACK_BIT;
		if (index >= 100) {
			PCIE_PR_ERR("Failed to get PME_TO_ACK");
			return -1;
		}
		index++;
		udelay((unsigned long)10);
	} while (ret != PME_ACK_BIT);

	PCIE_PR_INFO("Get PME ACK ");

	PCIE_PR_INFO("--");
	return 0;
}

static void kirin_pcie_shutdown(struct platform_device *pdev)
{
	struct kirin_pcie *pcie;

	PCIE_PR_INFO("++");

	pcie = dev_get_drvdata(&(pdev->dev));
	if (pcie == NULL) {
		PCIE_PR_ERR("Failed to get drvdata");
		return;
	}

	if (pcie->is_power_on) {
		if (kirin_pcie_power_on((&pcie->pp), RC_POWER_OFF)) {
			PCIE_PR_ERR("Failed to power off");
			return;
		}
	}

	PCIE_PR_INFO("--");
}

#ifdef CONFIG_PM
static int kirin_pcie_resume_noirq(struct device *dev)
{
	struct pci_dev *rc_dev;
	struct pcie_port *pp;
	struct kirin_pcie *pcie;

	PCIE_PR_INFO("++");

	pcie = dev_get_drvdata(dev);
	if (!pcie) {
		PCIE_PR_ERR("Failed to get drvdata");
		return -EINVAL;
	}
	pp = &pcie->pp;
	rc_dev = pcie->rc_dev;

	if (pcie->is_enumerated && (!pcie->usr_suspend)) {
		if (kirin_pcie_power_on(pp, RC_POWER_RESUME)) {
			PCIE_PR_ERR("Failed to power on ");
			return -EINVAL;
		}

		/* assert LTSSM enable */
		kirin_elb_writel(pcie, PCIE_LTSSM_ENABLE_BIT,
			PCIE_APP_LTSSM_ENABLE);

		PCIE_PR_DEBUG("Begin to recover RC cfg ");
		if (rc_dev)
			kirin_pcie_restore_rc_cfg(pcie);

	}

	PCIE_PR_INFO("--");

	return 0;
}


static int kirin_pcie_suspend_noirq(struct device *dev)
{
	struct kirin_pcie *pcie;
	struct pci_dev *rc_dev;
	struct pcie_port *pp;

	PCIE_PR_INFO("++");

	pcie = dev_get_drvdata(dev);
	if (pcie == NULL) {
		PCIE_PR_ERR("Failed to get drvdata");
		return -EINVAL;
	}
	rc_dev = pcie->rc_dev;
	pp = &pcie->pp;

	if (pcie->is_power_on) {
		if (!pcie->usr_suspend) {
			kirin_pcie_lp_ctrl(pcie->rc_id, DISABLE);
			kirin_pcie_shutdown_prepare(rc_dev);
		}
		if (kirin_pcie_power_on(pp, RC_POWER_SUSPEND)) {
			PCIE_PR_ERR("Failed to power off ");
			return -EINVAL;
		}
	}

	PCIE_PR_INFO("--");

	return 0;
}

#else

#define kirin_pcie_suspend_noirq NULL
#define kirin_pcie_resume_noirq NULL

#endif

static const struct dev_pm_ops kirin_pcie_dev_pm_ops = {
	.suspend_noirq	= kirin_pcie_suspend_noirq,
	.resume_noirq	= kirin_pcie_resume_noirq,
};

static const struct of_device_id kirin_pcie_match_table[] = {
	{
		.compatible = "hisilicon,kirin-pcie",
		.data = NULL,
	},
	/*lint -e785 -esym(785,*)*/
	{},
	/*lint -e785 +esym(785,*)*/
};

MODULE_DEVICE_TABLE(of, kirin_pcie_match_table);

struct platform_driver kirin_pcie_driver = {
	.probe			= kirin_pcie_probe,
	.shutdown		= kirin_pcie_shutdown,
	.driver			= {
		.name			= "Kirin-pcie",
		.owner			= THIS_MODULE,
		.pm				= &kirin_pcie_dev_pm_ops,
		.of_match_table = of_match_ptr(kirin_pcie_match_table),
	},
};

static int kirin_pcie_usr_suspend(u32 rc_idx)
{
	int ret;
	struct pcie_port *pp;
	struct pci_dev *rc_dev;
	struct kirin_pcie *pcie = &g_kirin_pcie[rc_idx];

	PCIE_PR_INFO("++");

	if (pcie->usr_suspend || !pcie->is_power_on) {
		PCIE_PR_ERR("Already suspend by EP ");
		return -EINVAL;
	}

	pp = &pcie->pp;
	rc_dev = pcie->rc_dev;

	if (!rc_dev) {
		PCIE_PR_ERR("Failed to get RC dev");
		return -1;
	}

	kirin_pcie_lp_ctrl(rc_idx, DISABLE);

	ret = kirin_pcie_shutdown_prepare(rc_dev);
	if (ret)
		return -EINVAL;

	/*phy rst from sys to pipe */
	ret = kirin_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
	ret |= 0x1 << 17;
	kirin_phy_writel(pcie, ret, SOC_PCIEPHY_CTRL1_ADDR);

	pcie->usr_suspend = 1;

	/*关闭所有时钟*/
	ret = kirin_pcie_power_on(pp, RC_POWER_OFF);
	if (ret) {
		PCIE_PR_ERR("Failed to power off ");
		return -EINVAL;
	}

	PCIE_PR_INFO("--");
	return 0;
}

static int kirin_pcie_usr_resume(u32 rc_idx)
{
	int ret;
	struct pcie_port *pp;
	struct pci_dev *rc_dev;
	struct kirin_pcie_dtsinfo *dtsinfo;
	struct kirin_pcie *pcie = &g_kirin_pcie[rc_idx];

	PCIE_PR_INFO("++");

	pp = &pcie->pp;
	rc_dev = pcie->rc_dev;
	dtsinfo = &pcie->dtsinfo;

	if (!rc_dev) {
		PCIE_PR_ERR("Failed to get RC dev");
		return -1;
	}

	ret = kirin_pcie_power_on(pp, RC_POWER_ON);
	if (ret) {
		PCIE_PR_ERR("Failed to power on");
		return -EINVAL;
	}
	pcie->usr_suspend = 0;
	ret = kirin_pcie_establish_link(&pcie->pp);
	if (ret)
		return -EINVAL;

	kirin_pcie_restore_rc_cfg(pcie);

	kirin_pcie_lp_ctrl(rc_idx, ENABLE);
	PCIE_PR_INFO("--");

	return 0;
}

/*
* EP Power ON/OFF callback Function:
* param: rc_idx---which rc the EP link with
*        resume_flag---1:PowerOn, 0: PowerOFF
*/
int kirin_pcie_pm_control(int resume_flag, u32 rc_idx)
{
	PCIE_PR_DEBUG("RC = [%u] ", rc_idx);

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_idx);
		return -EINVAL;
	}

	if (resume_flag) {
		dsm_pcie_clear_info();
		return kirin_pcie_usr_resume(rc_idx);
	} else {
		return kirin_pcie_usr_suspend(rc_idx);
	}
}
EXPORT_SYMBOL_GPL(kirin_pcie_pm_control);

int kirin_pcie_ep_off(u32 rc_idx)
{
	struct kirin_pcie *pcie = &g_kirin_pcie[rc_idx];

	return pcie->usr_suspend == 1;
}
EXPORT_SYMBOL_GPL(kirin_pcie_ep_off);

/*
* API FOR EP to control L1&L1-substate
* param: rc_idx---which rc the EP link with
*        enable---KIRIN_PCIE_LP_ON:enable L1 and L1-substate,
*				  KIRIN_PCIE_LP_Off: disable, others: illegal
*/
int kirin_pcie_lp_ctrl(u32 rc_idx, u32 enable)
{
	struct kirin_pcie * pcie = &g_kirin_pcie[rc_idx];
	struct kirin_pcie_dtsinfo *dtsinfo;

	PCIE_PR_DEBUG("++");

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_idx);
		return -EINVAL;
	}

	if (!pcie->is_power_on) {
		PCIE_PR_ERR("PCIe%d is power off ", rc_idx);
		return -EINVAL;
	}

	dtsinfo = &(pcie->dtsinfo);

	if (enable) {
		PCIE_PR_DEBUG("Enable");
		if (pcie->dtsinfo.board_type == BOARD_ASIC) {
			kirin_pcie_config_l0sl1(pcie->rc_id,  (enum link_aspm_state)dtsinfo->aspm_state);
			kirin_pcie_config_l1ss(pcie->rc_id, L1SS_PM_ASPM_ALL);
		}
	} else {
		PCIE_PR_DEBUG("Disable");
		kirin_pcie_config_l1ss(pcie->rc_id, L1SS_CLOSE);
		kirin_pcie_config_l0sl1(pcie->rc_id, ASPM_CLOSE);
	}

	PCIE_PR_DEBUG("-");

	return 0;
}
EXPORT_SYMBOL_GPL(kirin_pcie_lp_ctrl);


/*
* Enumerate Function:
* param: rc_idx---which rc the EP link with
*/
int kirin_pcie_enumerate(u32 rc_idx)
{
	int ret;
	u32 val;
	u32 dev_id;
	u32 vendor_id;
	struct pcie_port *pp;
	struct pci_bus *bus1;
	struct pci_dev *dev;
	struct kirin_pcie *pcie;
	struct kirin_pcie_dtsinfo *dtsinfo;

	PCIE_PR_INFO("++");
	PCIE_PR_DEBUG("RC[%u] begin to Enumerate ", rc_idx);

	if (rc_idx >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_idx);
		return -EINVAL;
	}
	pcie = &g_kirin_pcie[rc_idx];
	pp = &pcie->pp;
	dtsinfo = &pcie->dtsinfo;

	if (pcie->is_enumerated) {
		PCIE_PR_ERR("Enumeration was done successed before");
		return 0;
	}

	/*clk on*/
	ret = kirin_pcie_power_on(pp, RC_POWER_ON);
	if (ret) {
		PCIE_PR_ERR("Failed to power RC");
		dsm_pcie_dump_info(pcie, DSM_ERR_POWER_ON);
		return ret;
	}
	kirin_pcie_readl_rc(pp, pp->dbi_base, &val);
	val += rc_idx;
	kirin_pcie_writel_rc(pp, val, pp->dbi_base);

	if (dtsinfo->board_type == BOARD_FPGA) {
		kirin_pcie_writel_rc(pp, 0x10002, pp->dbi_base + 0xa0);
	}

	ret = dw_pcie_host_init(pp);
	if (ret) {
		PCIE_PR_ERR("Failed to initialize host");
		dsm_pcie_dump_info(pcie, DSM_ERR_ENUMERATE);
		goto FAIL_TO_POWEROFF;
	}

	kirin_pcie_rd_own_conf(pp, PCI_VENDOR_ID, 2, &vendor_id);
	kirin_pcie_rd_own_conf(pp, PCI_DEVICE_ID, 2, &dev_id);
	pcie->rc_dev = pci_get_device(vendor_id, dev_id, pcie->rc_dev);
	if (!pcie->rc_dev) {
		PCIE_PR_ERR("Failed to get RC device ");
		goto FAIL_TO_POWEROFF;
	}

	ret = kirin_pcie_save_rc_cfg(pcie);
	if (ret)
		goto FAIL_TO_POWEROFF;

	bus1 = pcie->rc_dev->subordinate;
	if (bus1) {
		list_for_each_entry(dev, &bus1->devices, bus_list) {
			if (pci_is_pcie(dev)) {
				pcie->ep_dev = dev;
				pcie->is_enumerated = 1;
				pcie->ep_devid = dev->device;
				pcie->ep_venid = dev->vendor;
				PCIE_PR_INFO("ep vendorid = 0x%x, deviceid = 0x%x", pcie->ep_venid, pcie->ep_devid);
			}
		}
	} else {
		PCIE_PR_ERR("Bus1 is null");
		pcie->ep_dev = NULL;
		pci_stop_and_remove_bus_device(pcie->rc_dev);
		goto FAIL_TO_POWEROFF;
	}

	if (!pcie->ep_dev) {
		PCIE_PR_ERR("There is no ep dev");
		pci_stop_and_remove_bus_device(pcie->rc_dev);
		goto FAIL_TO_POWEROFF;
	}

	kirin_pcie_lp_ctrl(pcie->rc_id, ENABLE);

	pcie->usr_suspend = 0;

	PCIE_PR_INFO("--");
	return 0;

FAIL_TO_POWEROFF:
	if (kirin_pcie_power_on(pp, RC_POWER_OFF)) {
			PCIE_PR_ERR("Failed to power off.");
	}
	return -1;
}
EXPORT_SYMBOL(kirin_pcie_enumerate);

int __init kirin_pcie_init(void)
{
	int ret = 0;

#ifdef CONFIG_KIRIN_PCIE_TEST
	ret = pci_register_driver(&kirin_pcie_ep_driver);
	if (ret)
		PCIE_PR_ERR("Failed to register kirin ep driver");
#endif

	ret = platform_driver_probe(&kirin_pcie_driver, kirin_pcie_probe);
	return ret;
}
/*lint -e438 -e550 -e713 -e732 -e737 -e774 -e838 +esym(438,*) +esym(713,*) +esym(732,*) +esym(737,*) +esym(774,*) +esym(550,*) +esym(838,*) */

subsys_initcall(kirin_pcie_init);

MODULE_AUTHOR("Xiaowei Song<songxiaowei@huawei.com>");
MODULE_DESCRIPTION("Hisilicon Kirin pcie driver");
MODULE_LICENSE("GPL");
