#include <linux/module.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <pmic_interface.h>
#include <linux/mfd/hisi_pmic.h>
#include "dwc3-hisi.h"

/**
 * USB related register in PERICRG
 */
#define SCTRL_SCDEEPSLEEPED				(0x08)
#define PERI_CRG_PEREN0				(0x0)
#define GT_HCLK_USB2OTG             (1 << 6)
#define PERI_CRG_PERDIS0				(0x04)
#define PERI_CRG_PERRSTEN0				(0x60)
#define PERI_CRG_PERRSTDIS0				(0x64)
#define IP_HRST_USB2OTG					(1 << 4)
#define IP_HRST_USB2OTG_AHBIF			(1 << 5)
#define IP_HRST_USB2OTG_MUX				(1 << 6)
#define IP_RST_USB2OTG_ADP				(1 << 25)
#define IP_RST_USB2OTG_BC				(1 << 26)
#define IP_RST_USB2OTG_PHY				(1 << 27)
#define	IP_RST_USB2PHY_POR				(1 << 28)

#define PERI_CRG_ISODIS					(0x148)
#define USB_REFCLK_ISO_EN				(1u << 25)


/* PCTRL registers */
#define PCTRL_PERI_CTRL3				(0x10)
#define USB_TCXO_EN						(1 << 1)
#define PERI_CTRL3_MSK_START			(16)
#define PCTRL_PERI_CTRL24				(0x64)
#define SC_USB3PHY_ABB_GT_EN			(1 << 24)
#define SC_CLK_USB3PHY_3MUX1_SEL		(1 << 25)
/*lint -e750 -esym(750,*)*/
/* clk module will round to 228M */
#define USB3OTG_ACLK_FREQ		229000000

#define USBOTG3_CTRL2_VBUSVLDEXT	(1 << 3)
#define USBOTG3_CTRL2_VBUSVLDEXTSEL	(1 << 2)

#define USB_FPGA_RESET_GPIO 15

static void dwc3_release(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericrg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */
	void __iomem *otg_bc_base = hisi_dwc3->otg_bc_reg_base;
	void __iomem *pctrl_reg_base = hisi_dwc3->pctrl_reg_base;
	volatile uint32_t temp;

	/* enable hclk */
	writel(GT_HCLK_USB2OTG, pericrg_base + PERI_CRG_PEREN0);

	/* unreset controller */
	writel(IP_RST_USB2OTG_ADP | IP_HRST_USB2OTG_MUX | IP_HRST_USB2OTG_AHBIF,
			pericrg_base + PERI_CRG_PERRSTDIS0);

	/* !!TODO: change access registers to clk api */
	/* abb clk config */
	/* open ABBISO */
	writel(USB_REFCLK_ISO_EN, pericrg_base + PERI_CRG_ISODIS);

	/* open clk */
	writel(USB_TCXO_EN | (USB_TCXO_EN << PERI_CTRL3_MSK_START),
		   pctrl_reg_base + PCTRL_PERI_CTRL3);

	msleep(1);

	temp = readl(pctrl_reg_base + PCTRL_PERI_CTRL24);
	temp &= ~SC_CLK_USB3PHY_3MUX1_SEL;
	writel(temp,  pctrl_reg_base + PCTRL_PERI_CTRL24);

	/* unreset phy */
	writel(IP_RST_USB2PHY_POR, pericrg_base + PERI_CRG_PERRSTDIS0);

	udelay(100);
	hisi_usb_unreset_phy_if_fpga(hisi_dwc3);

	/* unreset phy clk domain */
	writel(IP_RST_USB2OTG_PHY, pericrg_base + PERI_CRG_PERRSTDIS0);

	udelay(100);

	/* unreset hclk domain */
	writel(IP_HRST_USB2OTG, pericrg_base + PERI_CRG_PERRSTDIS0);

	/* fake vbus valid signal */
	temp = readl(otg_bc_base + USBOTG3_CTRL2);
	temp |= (USBOTG3_CTRL2_VBUSVLDEXT | USBOTG3_CTRL2_VBUSVLDEXTSEL);
	writel(temp, otg_bc_base + USBOTG3_CTRL2);

	msleep(1);

}


static void dwc3_reset(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericrg_base = hisi_dwc3->pericfg_reg_base;
	void __iomem *pctrl_reg_base = hisi_dwc3->pctrl_reg_base;
	volatile uint32_t temp;

/*lint -e438 -esym(438,*)*/
/*lint -e529 -esym(529,*)*/
	/* reset controller */
	writel(IP_HRST_USB2OTG, pericrg_base + PERI_CRG_PERRSTEN0);

	/* reset phy */
	writel(IP_RST_USB2OTG_PHY, pericrg_base + PERI_CRG_PERRSTEN0);

	writel(IP_RST_USB2PHY_POR, pericrg_base + PERI_CRG_PERRSTEN0);

	/* disable abb clk */
	temp = readl(pctrl_reg_base + PCTRL_PERI_CTRL24);
	temp &= ~SC_USB3PHY_ABB_GT_EN;
	writel(temp, pctrl_reg_base + PCTRL_PERI_CTRL24);

	temp = readl(pctrl_reg_base + PCTRL_PERI_CTRL3);
	temp &= ~USB_TCXO_EN;
	writel(temp, pctrl_reg_base + PCTRL_PERI_CTRL3);

	/* reset usb controller */
	writel(IP_HRST_USB2OTG_AHBIF | IP_HRST_USB2OTG_MUX | IP_RST_USB2OTG_ADP,
			pericrg_base + PERI_CRG_PERRSTEN0);

	/* disable usb controller clk */
	writel(GT_HCLK_USB2OTG, pericrg_base + PERI_CRG_PERDIS0);

	msleep(1);
}

static int usb3_clk_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	/* set usb aclk 228MHz to improve performance */
	ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, USB3OTG_ACLK_FREQ);
	if (ret) {
		usb_err("Can't aclk rate set, current rate is %ld:\n",
				clk_get_rate(hisi_dwc3->gt_aclk_usb3otg));
	}
	return ret;
}

void set_usb2_eye_diagram_param(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *otg_bc_base = hisi_dwc3->otg_bc_reg_base;

	if (hisi_dwc3->fpga_flag != 0)
		return;

	/* set high speed phy parameter */
	if (hisi_dwc3->host_flag) {
		writel(hisi_dwc3->eye_diagram_host_param, otg_bc_base + USBOTG3_CTRL4);
		usb_dbg("set hs phy param 0x%x for host\n",
				readl(otg_bc_base + USBOTG3_CTRL4));
	} else {
		writel(hisi_dwc3->eye_diagram_param, otg_bc_base + USBOTG3_CTRL4);
		usb_dbg("set hs phy param 0x%x for device\n",
				readl(otg_bc_base + USBOTG3_CTRL4));
	}
}


/*lint -e438 +esym(438,*)*/
/*lint -e529 +esym(529,*)*/
static int kirin660_usb3phy_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;
	usb_dbg("+\n");

	if (!hisi_dwc3) {
		usb_err("hisi_dwc3 is NULL, dwc3-hisi have some problem!\n");
		return -ENOMEM;
	}

	ret = usb3_clk_init(hisi_dwc3);
	if (ret)
		return ret;

	dwc3_release(hisi_dwc3);
	set_usb2_eye_diagram_param(hisi_dwc3);

	set_hisi_dwc3_power_flag(1);

	usb_dbg("-\n");

	return 0;
}

static int kirin660_usb3phy_shutdown(struct hisi_dwc3_device *hisi_dwc3)
{
	usb_dbg("+\n");

	set_hisi_dwc3_power_flag(0);

	dwc3_reset(hisi_dwc3);
	usb_dbg("-\n");

	return 0;
}

static struct usb3_phy_ops kirin660_phy_ops = {
	.init		= kirin660_usb3phy_init,
	.shutdown	= kirin660_usb3phy_shutdown,
};

static int dwc3_kirin660_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = hisi_dwc3_probe(pdev, &kirin660_phy_ops);
	if (ret)
		usb_err("probe failed, ret=[%d]\n", ret);

	return ret;
}

static int dwc3_kirin660_remove(struct platform_device *pdev)
{
	int ret = 0;

	ret = hisi_dwc3_remove(pdev);
	if (ret)
		usb_err("hisi_dwc3_remove failed, ret=[%d]\n", ret);

	return ret;
}
#ifdef CONFIG_OF
static const struct of_device_id dwc3_kirin660_match[] = {
	{ .compatible = "hisilicon,kirin660-dwc3" },
	{},
};
MODULE_DEVICE_TABLE(of, dwc3_kirin660_match);
#else
#define dwc3_kirin660_match NULL
#endif

static struct platform_driver dwc3_kirin660_driver = {
	.probe		= dwc3_kirin660_probe,
	.remove		= dwc3_kirin660_remove,
	.driver		= {
		.name	= "usb3-kirin660",
		.of_match_table = of_match_ptr(dwc3_kirin660_match),
		.pm	= HISI_DWC3_PM_OPS,
	},
};
/*lint +e715 +e716 +e717 +e835 +e838 +e845 +e533 +e835 +e778 +e774 +e747 +e785 +e438 +e529*/
/*lint -e64 -esym(64,*)*/
/*lint -e528 -esym(528,*)*/
module_platform_driver(dwc3_kirin660_driver);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_LICENSE("GPL");
/*lint -e753 +esym(753,*)*/
/*lint -e64 +esym(64,*)*/
