#include <linux/module.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <pmic_interface.h>
#include <linux/mfd/hisi_pmic.h>
#include "dwc3-hisi.h"
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include "core.h"
#include <linux/hisi/usb/dwc3-kirin970.h>
#include <linux/jiffies.h>
#include <huawei_platform/usb/hw_pd_dev.h>

/*lint -e750 -esym(750,*)*/

#define IP_RST_USB3OTG_MISC				(1 << 7)
#define SCTRL_SCDEEPSLEEPED				0x08
#define USB_REFCLK_ISO_EN               (1 << 25)
#define SC_CLK_USB3PHY_3MUX1_SEL        (1 << 25)

#define SC_SEL_ABB_BACKUP               (1 << 8)
#define CLKDIV_MASK_START               (16)

#define PERI_CRG_CLKDIV21               0xFC

#define GT_CLK_ABB_BACKUP               (1 << 22)
#define PERI_CRG_CLK_DIS5               0x54

#define PMC_PPLL3CTRL0                  0x048
#define PPLL3_FBDIV_START               (8)
#define PPLL3_EN                        (1 << 0)
#define PPLL3_BP                        (1 << 1)
#define PPLL3_LOCK                      (1 << 26)

#define PMC_PPLL3CTRL1                  0x04C
#define PPLL3_INT_MOD                   (1 << 24)
#define GT_CLK_PPLL3                    (1 << 26)

#define PERI_CRG_CLK_EN5                0x50
#define PERI_CRG_PERRSTEN4 0x90
#define PERI_CRG_PERRSTDIS4 0x94

#define SC_USB3PHY_ABB_GT_EN            (1 << 15)
#define REF_SSP_EN                      (1 << 16)

/* clk freq usb default usb3.0 228M usb2.0 60M */
static uint32_t USB3OTG_ACLK_FREQ = 229000000;
static uint32_t USB2OTG_ACLK_FREQ = 61000000;

atomic_t misc_usecount = ATOMIC_INIT(0);

extern struct hisi_dwc3_device *hisi_dwc3_dev;
/* usb3 misc ctrl driver */
void __iomem *reg_base_usb_misc_ctrl;
static inline void set_misc_base_addr(void __iomem *base)
{
	printk(KERN_ERR "[USB.DBG]:misc base:%pK\n", base);
	reg_base_usb_misc_ctrl = base;
}

/*lint -e528 -esym(528,*)*/
static inline void __iomem *get_misc_base_addr(void)
{
	return reg_base_usb_misc_ctrl;
}
/*lint -e528 +esym(528,*)*/

/*lint -e715 -e716 -e717 -e835 -e838 -e845 -e533 -e835 -e778 -e774 -e747 -e785 -e438 -e529*/
void dwc3_misc_ctrl_get(void)
{
	void __iomem *pericfg_base;

	if (!hisi_dwc3_dev) {
		usb_err("[USB.DBG] usb driver not probe\n");
		return ;
	}
	pericfg_base = hisi_dwc3_dev->pericfg_reg_base;/*lint !e438 */

	if (atomic_add_return(1, &misc_usecount) == 1) {
		usb_err("[USB.DBG] first get, going to\n");
		/* make sure Misc-ctrl at reset status */
		writel(IP_RST_USB3OTG_MISC | IP_RST_USB3OTG_32K, pericfg_base + PERI_CRG_PERRSTEN4);
		udelay(100);

		/* dis-reset usb misc ctrl module */
		writel(IP_RST_USB3OTG_MISC | IP_RST_USB3OTG_32K, pericfg_base + PERI_CRG_PERRSTDIS4);
	} else {
		usb_err("more than 1 get, just return! [%d]\n", atomic_read(&misc_usecount));
	}
}

void dwc3_misc_ctrl_put(void)
{
	void __iomem *pericfg_base;

	if (!hisi_dwc3_dev) {
		usb_err("[USB.DBG] usb driver not probe\n");
		return ;
	}
	pericfg_base = hisi_dwc3_dev->pericfg_reg_base;/*lint !e438 */

	if (atomic_add_return(-1, &misc_usecount) == 0) {
		usb_err("[USB.DBG] first put, going to\n");
		/* reset usb misc ctrl module */
		writel(IP_RST_USB3OTG_MISC | IP_RST_USB3OTG_32K, pericfg_base + 0x90);
	} else {
		usb_err("some one have get, don't release![%d]\n", atomic_read(&misc_usecount));
	}
}

void usb3_misc_reg_writel(u32 val, unsigned long int misc_offset)
{
	writel(val, (get_misc_base_addr() + misc_offset));
}

u32 usb3_misc_reg_readl(unsigned long int misc_offset)
{
	return  readl(get_misc_base_addr() + misc_offset);
}

void usb3_misc_reg_setbit(u32 bit, unsigned long int misc_offset)
{
	volatile uint32_t temp;
	temp = usb3_misc_reg_readl(misc_offset);
	temp |= (1u << bit);
	usb3_misc_reg_writel(temp, misc_offset);
}

void usb3_misc_reg_clrbit(u32 bit, unsigned long int misc_offset)
{
	volatile uint32_t temp;
	temp = usb3_misc_reg_readl(misc_offset);
	temp &= ~(1u << bit);
	usb3_misc_reg_writel(temp, misc_offset);
}

void usb3_misc_reg_setvalue(u32 value, unsigned long int misc_offset)
{
	volatile uint32_t temp;
	temp = usb3_misc_reg_readl(misc_offset);
	temp |= value;
	usb3_misc_reg_writel(temp, misc_offset);
}

void usb3_misc_reg_clrvalue(u32 value, unsigned long int misc_offset)
{
	volatile uint32_t temp;
	temp = usb3_misc_reg_readl(misc_offset);
	temp &= value;
	usb3_misc_reg_writel(temp, misc_offset);
}

/* usb3 misc ctrl driver */

#define SET_NBITS_MASK(start, end) (((2u << ((end) - (start))) - 1) << (start))
#define SET_BIT_MASK(bit) SET_NBITS_MASK(bit, bit)

int config_usb_phy_controller(void)
{
	volatile uint32_t temp;
	static int flag = 1;
	/* Release USB20 PHY out of IDDQ mode */
	usb3_misc_reg_clrbit(0u, 0x14ul);

	if (flag) {
		flag = 0;
		/* Release USB31 PHY out of  TestPowerDown mode */
		usb3_misc_reg_clrbit(23u, 0x50ul);

		/* Tell the PHY power is stable */
		usb3_misc_reg_setvalue((1u << 1) | (1u << 9) | (1u << 11), 0x54ul);


		/*
		 * config the TCA mux mode
		 * register:
		 *      -- 0x204 0x208: write immediately
		 *      -- 0x200 0x210 0x214 0x240: read-update-write
		 */
		usb3_misc_reg_writel(0xffffu, 0x208ul);
		usb3_misc_reg_writel(0x3u, 0x204ul);

		usb3_misc_reg_clrvalue(~SET_NBITS_MASK(0, 1), 0x200ul);
		usb3_misc_reg_setbit(4u, 0x210ul);

		temp = usb3_misc_reg_readl(0x214);
		temp &= ~(SET_NBITS_MASK(0, 1) | SET_NBITS_MASK(3, 4));
		temp |= (0x1 | (0x2 << 3));
		usb3_misc_reg_writel(temp, 0x214);

		usb3_misc_reg_setvalue(0x3u | (0x3u << 2), 0x240ul);
	}

	/* Enable SSC */
	usb3_misc_reg_setbit(1, 0x5c);

	return 0;
}

static uint32_t is_abb_clk_selected(void __iomem *sctrl_base)
{
	volatile uint32_t scdeepsleeped;

	if(!sctrl_base) {
		usb_err("sctrl_base is NULL!\n");
		return 1;
	}

	scdeepsleeped = (uint32_t)readl(SCTRL_SCDEEPSLEEPED + sctrl_base);
	if ((scdeepsleeped & (1 << 20)) == 0)
		return 1;

	return 0;
}


static int set_combophy_clk(struct hisi_dwc3_device *hisi_dwc3)
{
/*lint -save -e550 */
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e438 */
	void __iomem *pctrl_base = hisi_dwc3->pctrl_reg_base;/*lint !e438 */
	void __iomem *sctrl_base = hisi_dwc3->sctrl_reg_base;
#define USB_CLK_TYPE_ABB (0xabb)
#define USB_CLK_TYPE_PAD (0x10ad)
	volatile uint32_t temp;
	int ret;
	int clk_type = USB_CLK_TYPE_ABB;

	if (is_abb_clk_selected(sctrl_base)) {
		/* usb refclk iso enable */
		writel(USB_REFCLK_ISO_EN, PERI_CRG_ISODIS + pericfg_base);

		/* enable usb_tcxo_en */
		ret = clk_prepare_enable(hisi_dwc3->gt_clk_usb3_tcxo_en);
		if (ret) {
			usb_err("clk_prepare_enable gt_clk_usb3_tcxo_en failed\n");
			return ret;
		}

		mdelay(10);

		/* select usbphy clk from abb */
		temp = (uint32_t)readl(pctrl_base + PCTRL_PERI_CTRL24);
		temp &= ~SC_CLK_USB3PHY_3MUX1_SEL;
		writel(temp, pctrl_base + PCTRL_PERI_CTRL24);
	} else {
		usb_err("pad clk? maybe is bug\n");
		/* select usbphy clk from pad */
		usb3_misc_reg_setbit(12, 0x54);

		usb3_misc_reg_setbit(4, 0xa0);

		temp = usb3_misc_reg_readl(0x1c);
		temp &= ~(SET_NBITS_MASK(3, 4));
		temp |= (0x2 << 3);
		usb3_misc_reg_writel(temp, 0x1cul);

		ret = clk_prepare_enable(hisi_dwc3->gt_clk_usb2phy_ref);
		if (ret) {
			usb_err("clk_prepare_enable gt_clk_usb2phy_ref failed\n");
			return ret;
		}


		clk_type = USB_CLK_TYPE_PAD;
	}

	return clk_type;
/*lint -restore */
}

static void set_gpio_if_fpga(struct hisi_dwc3_device *hisi_dwc3)
{
	if (hisi_dwc3->fpga_flag != 0) {
		if(gpio_request(15, "usb_fpga")) {
			usb_err("request gpio error!\n");
		}

		gpio_direction_output(15, 1);
		udelay(100);

		gpio_direction_output(15, 0);
		udelay(100);
	}
}

int usb3_misc_ctrl_is_unreset(void __iomem *pericfg_base)
{
	volatile uint32_t regval;
#define PERI_CRG_PERRSTSTAT4  0x98
	regval = (uint32_t)readl(PERI_CRG_PERRSTSTAT4 + pericfg_base);
	return !((IP_RST_USB3OTG_MISC | IP_RST_USB3OTG_32K) & regval);
}

int usb3_misc_ctrl_is_reset(void __iomem *pericfg_base)
{
	return !usb3_misc_ctrl_is_unreset(pericfg_base);
}

int usb3_clk_is_open(void __iomem *pericfg_base)
{
	volatile uint32_t hclk, aclk;
	uint32_t hclk_mask = GT_HCLK_USB3OTG_MISC;
	uint32_t aclk_mask = (GT_CLK_USB3OTG_REF | GT_ACLK_USB3OTG);

#define PERI_CRG_PERSTAT0 0x0c
#define PERI_CRG_PERSTAT4 0x4c
	hclk = (uint32_t)readl(PERI_CRG_PERSTAT0 + pericfg_base);
	aclk = (uint32_t)readl(PERI_CRG_PERSTAT4 + pericfg_base);

	return (   ((aclk_mask & aclk) == aclk_mask)
		&& ((hclk_mask & hclk) == hclk_mask) );
}

int usb3_hclk_is_close(void __iomem *pericfg_base)
{
	volatile uint32_t hclk;
	uint32_t hclk_mask = GT_HCLK_USB3OTG_MISC;

#define PERI_CRG_PERSTAT0 0x0c
	hclk = (uint32_t)readl(PERI_CRG_PERSTAT0 + pericfg_base);

	return ((hclk_mask & hclk) != hclk_mask);
}

int usb3_phy_controller_is_release(void)
{
	volatile uint32_t stat;
	uint32_t bitmask = (SET_NBITS_MASK(8, 9) | SET_NBITS_MASK(0,1));

	stat = usb3_misc_reg_readl(0xa0);

	return ((bitmask & stat) == bitmask);
}

void usb3_core_wait_for_idle(struct hisi_dwc3_device *hisi_dwc3)
{
	int n = 0;
	u32 reg;

	/*lint -e648 -esym(648,*)*/
	/* stop core */
	reg = readl(hisi_dwc3->usb_core_reg_base + DWC3_DCTL);
	reg &= ~(DWC3_DCTL_RUN_STOP);
	writel(reg, hisi_dwc3->usb_core_reg_base + DWC3_DCTL);

	/* prevents the interrupt from being generated */
	reg = readl(hisi_dwc3->usb_core_reg_base + DWC3_GEVNTSIZ(n));
	reg |= (DWC3_GEVNTSIZ_INTMASK);
	writel(reg, hisi_dwc3->usb_core_reg_base + DWC3_GEVNTSIZ(n));
	/*lint -e648 +esym(648,*)*/

	/* cleanup event */
	reg = readl(hisi_dwc3->usb_core_reg_base + DWC3_GEVNTCOUNT(n));
	writel(reg, hisi_dwc3->usb_core_reg_base + DWC3_GEVNTCOUNT(n));

	udelay(500);
}

void dwc3_set_combophy_clk(void)
{
	int clk_type = 0xabb;
	struct hisi_dwc3_device *hisi_dwc3 = hisi_dwc3_dev;

	clk_type = set_combophy_clk(hisi_dwc3);
	if(clk_type) {
		usb_err("usb clk type:%x\n", clk_type);
	}
}

static void dwc3_phy_release(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *otg_bc_base = hisi_dwc3->otg_bc_reg_base;

	set_misc_base_addr(otg_bc_base);

	/* unreset phy */
	usb3_misc_reg_setvalue(SET_NBITS_MASK(0, 1), 0xa0);
	udelay(100);
	set_gpio_if_fpga(hisi_dwc3);

	dwc3_set_combophy_clk();
}

static void close_combophy_clk(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *sctrl_base = hisi_dwc3->sctrl_reg_base;

	if (is_abb_clk_selected(sctrl_base)) {
		/* disable usb_tcxo_en */
		clk_disable_unprepare(hisi_dwc3->gt_clk_usb3_tcxo_en);
	} else {
		/* close phy ref clk */
		clk_disable_unprepare(hisi_dwc3->gt_clk_usb2phy_ref);
	}
}


static void dwc3_phy_reset(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;

	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		/* reset phy */
		usb3_misc_reg_clrvalue(~(SET_NBITS_MASK(0, 1)), 0xa0ul); /*lint !e835 */
		usb_err("reset phy\n");

		close_combophy_clk(hisi_dwc3);
	}
}

static void dwc3_open_controller_clk(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */
	volatile int temp;
	/* open misc clk gate */
	ret = clk_prepare_enable(hisi_dwc3->gt_aclk_usb3otg);
	if (ret) {
		usb_err("clk_prepare_enable gt_aclk_usb3otg failed\n");
		return;
	}

	temp = readl(pericfg_base + 0x48);
	usb_err("[0x48:%x]\n", temp);

	ret = clk_prepare_enable(hisi_dwc3->clk);
	if (ret) {
		usb_err("clk_prepare_enable clk failed\n");
		return;
	}
	temp = readl(pericfg_base + 0x48);
	usb_err("[0x48:%x]\n", temp);

	usb3_misc_reg_clrbit(4, 0xa0);

	/* switch phy */
	usb3_misc_reg_clrbit(5, 0xa0ul);

	usb3_misc_reg_setvalue((SET_NBITS_MASK(3, 4)), 0x1c);
}

static void dwc3_release(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *otg_bc_base = hisi_dwc3->otg_bc_reg_base;

	usb_err("+\n");
	set_misc_base_addr(otg_bc_base);

	dwc3_open_controller_clk(hisi_dwc3);

	config_usb_phy_controller();

	/* release usb2.0 phy */
	usb3_misc_reg_setbit(0, 0xa0);

	/* unreset controller */
	usb3_misc_reg_setvalue(SET_NBITS_MASK(8, 9), 0xa0);
	udelay(100);

	/* set vbus valid */
	usb3_misc_reg_setvalue(SET_NBITS_MASK(6, 7), 0x0);
	usb3_misc_reg_setvalue(SET_NBITS_MASK(5, 6), 0xc);

	/* require to delay 10us */
	udelay(100);
	usb_err("-\n");
}

static int kirin970_clk_check(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericfg_base;

	/*
	 * just check if usb module probe.
	 */
	if (!hisi_dwc3) {
		usb_err("usb module not probe\n");
		return -EBUSY;
	}
	pericfg_base = hisi_dwc3->pericfg_reg_base;

	return (usb3_misc_ctrl_is_reset(pericfg_base) || usb3_hclk_is_close(pericfg_base));
}

static int kirin970_event_sync(struct hisi_dwc3_device *hisi_dwc3)
{
	/*
	 * just check if usb module probe.
	 */
	if (!hisi_dwc3) {
		usb_err("usb module not probe\n");
		return -EBUSY;
	}

	return 1;
}

static void kirin970_notify_speed(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	usb_dbg("device speed is %d\n", hisi_dwc3->speed);

	/*if (hisi_dwc3->speed != 0xFF)
		pd_dpm_set_usb_speed(hisi_dwc3->speed);*/

	if ((hisi_dwc3->speed < USB_SPEED_WIRELESS) && (hisi_dwc3->speed > USB_SPEED_UNKNOWN)) {
		ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, USB2OTG_ACLK_FREQ);
		if (ret) {
			usb_err("Can't aclk rate set\n");
		}
	} else if ((hisi_dwc3->speed == 0xFF) && (get_hisi_dwc3_power_flag())) {
		ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, USB3OTG_ACLK_FREQ);
		if (ret) {
			usb_err("Can't aclk rate set\n");
		}
	}

	usb_dbg("current rate is %ld:\n", clk_get_rate(hisi_dwc3->gt_aclk_usb3otg));
}


static void dwc3_reset(struct hisi_dwc3_device *hisi_dwc3)
{
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;
/*lint -e438 -esym(438,*)*/
/*lint -e529 -esym(529,*)*/

	usb_err("+\n");
	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		/* set vbus valid */
		if(usb3_phy_controller_is_release()) {
			usb3_core_wait_for_idle(hisi_dwc3);
		}

		/* reset controller */
		usb3_misc_reg_clrvalue(~(SET_NBITS_MASK(8, 9)), 0xa0ul);
		usb_err("reset controller\n");

		/* reset usb2.0 phy */
		usb3_misc_reg_clrbit(0, 0xa0);
	}

	clk_disable_unprepare(hisi_dwc3->clk);
	clk_disable_unprepare(hisi_dwc3->gt_aclk_usb3otg);

	usb_err("close usb success\n");
}

static int usb3_clk_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;

	/* set usb aclk 228MHz to improve performance */
	usb_dbg("set aclk rate [%d]\n", USB3OTG_ACLK_FREQ);
	ret = clk_set_rate(hisi_dwc3->gt_aclk_usb3otg, USB3OTG_ACLK_FREQ);
	if (ret) {
		usb_err("Can't aclk rate set, current rate is %ld:\n", clk_get_rate(hisi_dwc3->gt_aclk_usb3otg));
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

enum phy_cr_start_type {
	PHY_CR_READ,
	PHY_CR_WRITE
};

#define USB31PHY_CR_ACK SET_BIT_MASK(7)
#define USB31PHY_CR_WR_EN SET_BIT_MASK(5)
#define USB31PHY_CR_RD_EN SET_BIT_MASK(3)
#define USB31PHY_CR_SEL SET_BIT_MASK(4)
#define USB31PHY_CR_CLK SET_BIT_MASK(2)

#define USB31PHY_CR_ADDR_MASK SET_NBITS_MASK(16, 31)

#define USB31PHY_CR_DATA_RD_START 16

#define USB_MISC_REG_PHY_CR_PARA_CTRL 0x54
#define USB_MISC_REG_PHY_CR_PARA_DATA 0x58

#define TX_VBOOST_LVL_REG (0xf)
#define TX_VBOOST_LVL_VALUE_START (6)
#define TX_VBOOST_LVL_ENABLE SET_BIT_MASK(9)

void usb31phy_cr_set_addr(u32 addr)
{
	volatile u32 tmp;
	tmp = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);
	tmp &= ~USB31PHY_CR_ADDR_MASK;
	tmp |= ((addr << 16) & ~SET_NBITS_MASK(0, 15));
	usb3_misc_reg_writel(tmp, USB_MISC_REG_PHY_CR_PARA_CTRL);
}

void usb31phy_cr_clk(void)
{
	volatile u32 tmp;

	/* clk up */
	tmp = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);
	tmp |= USB31PHY_CR_CLK;
	usb3_misc_reg_writel(tmp, USB_MISC_REG_PHY_CR_PARA_CTRL);

	/* clk down */
	tmp = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);
	tmp &= ~USB31PHY_CR_CLK;
	usb3_misc_reg_writel(tmp, USB_MISC_REG_PHY_CR_PARA_CTRL);
}

void usb31phy_cr_100clk(void)
{
	int i = 0;

	for(i = 0; i < 100; i++) {
		usb31phy_cr_clk();
	}
}

int usb31phy_cr_ack_wait_n = 100000;
#ifdef DEBUG_USB31PHY
void usb31phy_cr_dbg_setacktime(int n)
{
	usb31phy_cr_ack_wait_n = n;
}
#endif

int usb31phy_cr_wait_ack(void)
{
	int32_t i = usb31phy_cr_ack_wait_n;
	int ret = 0;

	while (!(usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL) & USB31PHY_CR_ACK)) {
		usb31phy_cr_clk();

		if(0 == i) {
			usb_err("wait phy cr timeout! %d, %d\n", USB31PHY_CR_ACK, usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL));
			ret = 1;
			break;
		}
		i--;
	}

	return ret;
}

void usb31phy_cr_start(enum phy_cr_start_type type)
{
	volatile u32 high, low;

	high = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);

	if (PHY_CR_READ == type) {
		high |= USB31PHY_CR_RD_EN;
	} else if(PHY_CR_WRITE == type) {
		high |= USB31PHY_CR_WR_EN;
	} else {
		usb_err("unknow phy cr type:%d\n", type);
	}

	usb3_misc_reg_writel(high, USB_MISC_REG_PHY_CR_PARA_CTRL);

	usb31phy_cr_clk();

	/* read & write */
	low = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);
	low &= ~USB31PHY_CR_RD_EN;
	low &= ~USB31PHY_CR_WR_EN;
	usb3_misc_reg_writel(low, USB_MISC_REG_PHY_CR_PARA_CTRL);
}

void usb31phy_cr_set_sel(void)
{
	volatile u32 temp;

	temp = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_CTRL);

	temp |= USB31PHY_CR_SEL;

	usb3_misc_reg_writel(temp, USB_MISC_REG_PHY_CR_PARA_CTRL);
}

u16 usb31phy_cr_read(u32 addr)
{
	volatile u32 data;

	usb31phy_cr_100clk();

	/* step-0: set cr para selk */
	usb31phy_cr_set_sel();

	/* step-1: set addr for read */
	usb31phy_cr_set_addr(addr);

	/* step-2: send read start cmd */
	usb31phy_cr_start(PHY_CR_READ);

	/* step-3: wait cmd ack */
	if (usb31phy_cr_wait_ack()) {
		usb_err("wait ack timeout!\n");
	}

	/* ack, read data now*/
	data = usb3_misc_reg_readl(USB_MISC_REG_PHY_CR_PARA_DATA);
	return ((data >> USB31PHY_CR_DATA_RD_START) & SET_NBITS_MASK(0, 15));
}

int usb31phy_cr_write(u32 addr, u16 value)
{
	int ret = 0;

	usb31phy_cr_100clk();

	/* step-0: set cr para selk */
	usb31phy_cr_set_sel();

	/* step-1: set addr for write */
	usb31phy_cr_set_addr(addr);

	/* step-2: set data for write */
	usb3_misc_reg_writel(value, USB_MISC_REG_PHY_CR_PARA_DATA);

	/* step-3: send write start cmd */
	usb31phy_cr_start(PHY_CR_WRITE);

	/* wait cmd ack if write success */
	if (usb31phy_cr_wait_ack()) {
		ret = 1;
	}

	return ret;
}

void set_vboost_for_usb3(uint32_t usb3_phy_tx_vboost_lvl)
{
	volatile u16 temp;

	if ((usb3_phy_tx_vboost_lvl > 7) || (VBOOST_LVL_DEFAULT_PARAM == usb3_phy_tx_vboost_lvl)) {
		usb_dbg("bad vboost (%d) use default (%d)\n", usb3_phy_tx_vboost_lvl, VBOOST_LVL_DEFAULT_PARAM);
		return ;
	}

	temp = usb31phy_cr_read(TX_VBOOST_LVL_REG);
	temp |= (u16)((TX_VBOOST_LVL_ENABLE | (usb3_phy_tx_vboost_lvl << TX_VBOOST_LVL_VALUE_START)) & SET_NBITS_MASK(0,15));
	usb31phy_cr_write(TX_VBOOST_LVL_REG, temp);

	temp = usb31phy_cr_read(TX_VBOOST_LVL_REG);
	usb_dbg("[0x%x]usb31 cr param:%x\n", TX_VBOOST_LVL_REG, temp);
}

/*lint -e438 +esym(438,*)*/
/*lint -e529 +esym(529,*)*/
static int kirin970_usb3phy_init(struct hisi_dwc3_device *hisi_dwc3);

extern struct hisi_dwc3_device *hisi_dwc3_dev;

static int kirin970_usb31_core_enable_u3(struct hisi_dwc3_device *hisi_dwc3)
{
	volatile u32 temp;
	void __iomem *pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */

	usb_err("+\n");

	/*
	 * check if usb controller is busy.
	 */
	if (get_hisi_dwc3_power_flag()) {
		usb_err("usb core is busy! please disconnect usb first!\n");
		return -EBUSY;
	}

	/*
	 * check if misc ctrl is release
	 * check if usb clk is open
	 */
	if (usb3_misc_ctrl_is_reset(pericfg_base) || usb3_hclk_is_close(pericfg_base)) {
		usb_err("misc ctrl or usb3 clk not ready.\n");
		/* Block device required */
		return -ENOTBLK;
	}

	/* open usb phy clk gate */
	dwc3_open_controller_clk(hisi_dwc3);

	temp = readl(pericfg_base + 0x48);
	if (!(temp & 0x3)) {
		usb_err("[PERI CFG 0X48:0x%x]\n", temp);
		usb_err("[misc 0xa0:0x%x]\n", usb3_misc_reg_readl(0xa0));
		return -ENOTBLK;
	}


	/* Release USB20 PHY out of IDDQ mode */
	usb3_misc_reg_clrbit(0u, 0x14ul);

	/* release usb2.0 phy */
	usb3_misc_reg_setbit(0, 0xa0);
	udelay(100);

	/* unreset controller */
	usb3_misc_reg_setvalue(SET_NBITS_MASK(8, 9), 0xa0);
	udelay(100);

	/* Set REFCLKPER */
	/* DP4 + usb2.0 need */
	temp = readl(hisi_dwc3->usb_core_reg_base + 0xc12c);
	temp |= (1ul << 26);
	writel(temp, hisi_dwc3->usb_core_reg_base + 0xc12c);

	/* enable U3 */
	temp = readl(hisi_dwc3->usb_core_reg_base + 0xc2c0);
	temp |= (1ul << 17);
	writel(temp, hisi_dwc3->usb_core_reg_base + 0xc2c0);

	usb_err("-\n");
	return 0;
}

extern struct hisi_dwc3_device *hisi_dwc3_dev;
void hisi_dwc3_usbcore_write(u32 offset, u32 value)
{
	void __iomem *pericfg_base = hisi_dwc3_dev->pericfg_reg_base;/*lint !e529 */
	if (!hisi_dwc3_dev) {
		usb_err("usb driver have not probed!\n");
		return ;
	}

	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		if(usb3_phy_controller_is_release()) {
			usb_err("set offset:%x.\n", offset);
			writel(value, hisi_dwc3_dev->usb_core_reg_base + offset);
		} else {
			usb_err(" usb controller is reset, please release first.\n");
		}
	} else {
		usb_err("misc ctrl is reset, please release it first.\n");
	}
}

volatile unsigned int hisi_dwc3_usbcore_read(u32 offset)
{
	void __iomem *pericfg_base = hisi_dwc3_dev->pericfg_reg_base;/*lint !e529 */

	if (!hisi_dwc3_dev) {
		usb_err("usb driver have not probed!\n");
		return 0;
	}

	if(usb3_misc_ctrl_is_unreset(pericfg_base) && usb3_clk_is_open(pericfg_base)) {
		if(usb3_phy_controller_is_release()) {
			usb_err("read offset:%x.\n", offset);
			return readl(hisi_dwc3_dev->usb_core_reg_base + offset);
		} else {
			usb_err(" usb controller is reset, please release first.\n");
		}
	}else {
		usb_err("misc ctrl is reset, please release it first.\n");
	}

	return 0;
}

static int kirin970_usb31_core_disable_u3(struct hisi_dwc3_device *hisi_dwc3)
{
	usb_err("+\n");

	if (get_hisi_dwc3_power_flag()) {
		usb_err("usb core is busy! please disconnect usb first!\n");
		return -EBUSY;
	}

	/* need reset controller */
	usb3_misc_reg_clrvalue(~(SET_NBITS_MASK(8, 9)), 0xa0ul);

	/* reset usb2.0 phy */
	usb3_misc_reg_clrbit(0, 0xa0);

	/* close misc clk gate */
	clk_disable_unprepare(hisi_dwc3->clk);
	clk_disable_unprepare(hisi_dwc3->gt_aclk_usb3otg);

	usb_err("-\n");

	return 0;
}

int kirin970_usb31_phy_notify(enum phy_change_type type)
{
	int ret = 0;
	struct hisi_dwc3_device *hisi_dwc3 = hisi_dwc3_dev;

	usb_err("+\n");
	if (!hisi_dwc3) {
		usb_err("hisi_dwc3 is NULL, dwc3-hisi have some problem!\n");
		return -ENOMEM;
	}

	/*
	 * check if usb controller is busy.
	 */
	if (get_hisi_dwc3_power_flag()) {
		usb_err("usb core is busy! please disconnect usb first!\n");
		return -EBUSY;
	}

	if (PHY_MODE_CHANGE_BEGIN == type) {
		kirin970_usb31_core_enable_u3(hisi_dwc3);
	} else if (PHY_MODE_CHANGE_END == type) {
		kirin970_usb31_core_disable_u3(hisi_dwc3);
	} else {
		usb_err("Bad param!\n");
		ret = 1;
	}
	usb_err("-\n");
	return ret;
}

int kirin970_usb31_controller_dump(void)
{
	int ret = 0;
	struct hisi_dwc3_device *hisi_dwc3 = hisi_dwc3_dev;

	usb_err("+\n");

	if (!hisi_dwc3) {
		usb_err("hisi_dwc3 is NULL, dwc3-hisi have some problem!\n");
		return -ENOMEM;
	}

	usb_err("[c2c0:0x%x], [c118:0x%x], [d050:0x%x], [misc 84:0x%x], [misc 28:0x%x]\n",
		readl(hisi_dwc3->usb_core_reg_base + 0xc2c0),
		readl(hisi_dwc3->usb_core_reg_base + 0xc118),
		readl(hisi_dwc3->usb_core_reg_base + 0xd050),
		usb3_misc_reg_readl(0x84),
		usb3_misc_reg_readl(0x28));

	usb_err("-\n");
	return ret;
}

static int kirin970_usb3phy_shutdown(struct hisi_dwc3_device *hisi_dwc3)
{
	static int flag = 0;
	usb_err("+\n");

	set_hisi_dwc3_power_flag(0);

	if (!flag) {
		dwc3_phy_reset(hisi_dwc3);
		flag = 1;
	}

	dwc3_reset(hisi_dwc3);

	dwc3_misc_ctrl_put();
	clk_disable_unprepare(hisi_dwc3_dev->gt_hclk_usb3otg);
	usb_err("-\n");

	return 0;
}

static int kirin970_get_dts_resource(struct hisi_dwc3_device *hisi_dwc3)
{
	struct device *dev = &hisi_dwc3->pdev->dev;
	/* get h clk handler */
	hisi_dwc3->gt_hclk_usb3otg = devm_clk_get(&hisi_dwc3->pdev->dev, "hclk_usb3otg");
	if (IS_ERR_OR_NULL(hisi_dwc3->gt_hclk_usb3otg)) {
		dev_err(dev, "get hclk_usb3otg failed\n");
		return -EINVAL;
	}

	/* get tcxo clk handler */
	hisi_dwc3->gt_clk_usb3_tcxo_en = devm_clk_get(&hisi_dwc3->pdev->dev, "clk_usb3_tcxo_en");
	if (IS_ERR_OR_NULL(hisi_dwc3->gt_clk_usb3_tcxo_en)) {
		dev_err(dev, "get clk_usb3_tcxo_en failed\n");
		return -EINVAL;
	}
	/* get usb2phy ref clk handler */
	hisi_dwc3->gt_clk_usb2phy_ref = devm_clk_get(&hisi_dwc3->pdev->dev, "clk_usb2phy_ref");
	if (IS_ERR_OR_NULL(hisi_dwc3->gt_clk_usb2phy_ref)) {
		dev_err(dev, "get clk_usb2phy_ref failed\n");
		return -EINVAL;
	}

	if (of_property_read_u32(dev->of_node, "quirk_dplus_gpio", &(hisi_dwc3->quirk_dplus_gpio))) {
		hisi_dwc3->quirk_dplus_gpio = 0;
		dev_info(dev, "not need quirk dplus\n");
	} else {
		dev_info(dev, "the board has dplus pull quirk gpio:%d\n",
			hisi_dwc3->quirk_dplus_gpio);
		if (gpio_request(hisi_dwc3->quirk_dplus_gpio, "quirk_dplus_gpio")) {
			usb_err("request gpio quirk_dplus_gpio error!\n");
			return -EINVAL;
		}
	}

	if (of_property_read_u32(dev->of_node, "usb_aclk_freq", &USB3OTG_ACLK_FREQ)) {
		USB3OTG_ACLK_FREQ = 229000000;
	}
	dev_info(dev, "boart config set usb aclk freq:%d\n", USB3OTG_ACLK_FREQ);

	if (of_property_read_u32(dev->of_node, "usb_support_dp", &(hisi_dwc3->support_dp))) {
		dev_err(dev, "usb driver not support dp\n");
		hisi_dwc3->support_dp = 0;
	}

	return 0;
}

static int kirin970_shared_phy_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;
	uint32_t temp;
	void __iomem *pericfg_base;

	usb_dbg("+\n");

	if (!hisi_dwc3)
		return -ENODEV;

	pericfg_base = hisi_dwc3->pericfg_reg_base;

	udelay(100);

	/* enable 2.0 phy refclk */
	ret = clk_prepare_enable(hisi_dwc3->gt_clk_usb2phy_ref);
	if (ret) {
		usb_err("clk_prepare_enable gt_clk_usb2phy_ref failed\n");
		return ret;
	}

	/* Release USB20 PHY out of IDDQ mode */
	usb3_misc_reg_clrbit(0u, 0x14ul);

	/* usb2 phy reference clock source select soc 19.2Mhz */
	usb3_misc_reg_setbit(4, 0xa0ul);
	temp = usb3_misc_reg_readl(0x1cul);
	temp &= ~(0x3 << 3);
	temp |= (0x2 << 3);
	usb3_misc_reg_writel(temp, 0x1cul);

	/* switch phy */
	usb3_misc_reg_setbit(5, 0xa0ul);

	temp = usb3_misc_reg_readl(0xa0ul);
	temp &= ~(0x3 << 6);
	temp |= (0x2 << 6);
	usb3_misc_reg_writel(temp, 0xa0ul);

	/* release 2.0 phy */
	usb3_misc_reg_setbit(0, 0xa0ul);
	udelay(100);

	usb_dbg("-\n");
	return 0;
}

static int kirin970_shared_phy_shutdown(struct hisi_dwc3_device *hisi_dwc3)
{
	uint32_t temp;
	void __iomem *pericfg_base;

	usb_dbg("+\n");

	if (!hisi_dwc3)
		return -ENODEV;

	pericfg_base = hisi_dwc3->pericfg_reg_base;

	/* reset 2.0 phy */
	usb3_misc_reg_clrbit(0, 0xa0);

	/* switch drv vbus s*/
	temp = usb3_misc_reg_readl(0xa0ul);
	temp &= ~(0x3 << 6);
	//temp |= (0x2 << 6);
	usb3_misc_reg_writel(temp, 0xa0ul);

	/* switch phy */
	usb3_misc_reg_clrbit(5, 0xa0ul);

	/* clock source select */
	// nothing

	/* enable siddq */
	usb3_misc_reg_setbit(0u, 0x14ul);

	/* disable 2.0 phy refclk */
	clk_disable_unprepare(hisi_dwc3->gt_clk_usb2phy_ref);

	usb_dbg("-\n");
	return 0;
}

static void kirin970_set_hi_impedance(struct hisi_dwc3_device *hisi_dwc)
{
	void __iomem *base = hisi_dwc->otg_bc_reg_base;

	usb_dbg("+\n");
	writel(0, base + BC_CTRL2);
	usb_dbg("-\n");
}

static struct usb3_phy_ops kirin970_phy_ops = {
	.init		= kirin970_usb3phy_init,
	.shutdown	= kirin970_usb3phy_shutdown,
	.get_dts_resource = kirin970_get_dts_resource,
	.shared_phy_init	= kirin970_shared_phy_init,
	.shared_phy_shutdown	= kirin970_shared_phy_shutdown,
	.set_hi_impedance	= kirin970_set_hi_impedance,
	.clk_check		= kirin970_clk_check,
	.event_sync		= kirin970_event_sync,
	.notify_speed	= kirin970_notify_speed,
};



static int kirin970_usb3phy_init(struct hisi_dwc3_device *hisi_dwc3)
{
	int ret;
	static int flag = 0;
	void __iomem *pericfg_base;
	usb_err("+\n");

	if (!hisi_dwc3) {
		usb_err("hisi_dwc3 is NULL, dwc3-hisi have some problem!\n");
		return -ENOMEM;
	}

	pericfg_base = hisi_dwc3->pericfg_reg_base;/*lint !e529 */

	/* pull down d+ */
	if (hisi_dwc3->quirk_dplus_gpio) {
		usb_dbg("set gpio to pull downd d+!\n");
		gpio_direction_output(hisi_dwc3->quirk_dplus_gpio, 1);
	}

	ret = usb3_clk_init(hisi_dwc3);
	if (ret)
		return ret;

	if (!hisi_dwc3->support_dp) {
		kirin970_phy_ops.event_sync = NULL;
	}

	ret = clk_prepare_enable(hisi_dwc3_dev->gt_hclk_usb3otg);
	if (ret) {
		usb_err("clk_enable gt_hclk_usb3otg failed\n");
		return -ENOTBLK;
	}

	dwc3_misc_ctrl_get();

	if (!hisi_dwc3->support_dp || !flag) {
		dwc3_phy_release(hisi_dwc3);
		flag = 1;
	}

	if (hisi_dwc3->support_dp && usb3_misc_ctrl_is_reset(pericfg_base)) {
		usb_err("[USBDP.DBG] goto here, need check who call.\n");

		ret = clk_prepare_enable(hisi_dwc3_dev->gt_hclk_usb3otg);
		if (ret) {
			usb_err("clk_enable gt_hclk_usb3otg failed\n");
			return -ENOTBLK;
		}

		dwc3_misc_ctrl_get();
	}

	dwc3_release(hisi_dwc3);

	/* need reset clk freq */
	ret = usb3_clk_init(hisi_dwc3);
	if (ret)
		return ret;

	set_usb2_eye_diagram_param(hisi_dwc3);
	set_vboost_for_usb3(hisi_dwc3->usb3_phy_tx_vboost_lvl);

	set_hisi_dwc3_power_flag(1);

	usb_err("-\n");

	return 0;
}

static int dwc3_kirin970_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = hisi_dwc3_probe(pdev, &kirin970_phy_ops);
	if (ret)
		usb_err("probe failed, ret=[%d]\n", ret);

	return ret;
}

static int dwc3_kirin970_remove(struct platform_device *pdev)
{
	int ret = 0;

	ret = hisi_dwc3_remove(pdev);
	if (ret)
		usb_err("hisi_dwc3_remove failed, ret=[%d]\n", ret);

	return ret;
}
#ifdef CONFIG_OF
static const struct of_device_id dwc3_kirin970_match[] = {
	{ .compatible = "hisilicon,kirin970-dwc3" },
	{},
};
MODULE_DEVICE_TABLE(of, dwc3_kirin970_match);
#else
#define dwc3_kirin970_match NULL
#endif

static struct platform_driver dwc3_kirin970_driver = {
	.probe		= dwc3_kirin970_probe,
	.remove		= dwc3_kirin970_remove,
	.driver		= {
		.name	= "usb3-kirin970",
		.of_match_table = of_match_ptr(dwc3_kirin970_match),
		.pm	= HISI_DWC3_PM_OPS,
	},
};
/*lint +e715 +e716 +e717 +e835 +e838 +e845 +e533 +e835 +e778 +e774 +e747 +e785 +e438 +e529*/
/*lint -e64 -esym(64,*)*/
/*lint -e528 -esym(528,*)*/
module_platform_driver(dwc3_kirin970_driver);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_LICENSE("GPL");
/*lint -e753 +esym(753,*)*/
/*lint -e64 +esym(64,*)*/
