
#include "pcie-kirin.h"
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/stat.h>

/*lint -e438 -e550 -e715 -e732 -e747 -e750 -e838 -esym(438,*) -esym(550,*) -esym(715,*) -esym(732,*) -esym(747,*) -esym(750,*) -esym(838,*) */
#define CHIP_TYPE_ES 1
#define CHIP_TYPE_CS 2
#define ECO 1
#define ECO_TEST 2
#define SRAM_SIZE 0x4000 //4096*16
#define SRAM_TEST_SIZE 16
#define PCIE0_SRAM_BASE 0xFC018000
#define PCIE1_SRAM_BASE 0xFC098000
#define LANEN_DIG_ASIC_RX_OVRD_IN_4 0x402c
#define AXI_CLK_FREQ 207500000

#define CRGPERIHPH_PEREN7 0x420
#define CRGPERIHPH_PERRSTDIS0 0x64
#define CRGPERIHPH_PERRSTDIS3 0x88
#define CRGPERIPH_CLKDIV21 0xfc
#define CRGPERIPH_CLKDIV26 0x700
#define CRGPERIPH_PCIECTRL0 0x800
#define CRGPERIPH_PCIECTRL1 0x804
#define CRGPERIPH_PEREN12 0x470
#define CRGPERIPH_PERDIS12 0x474

#define IP_PRST_PCIE_PHY (0x1 << 27)

#define SCTRL_SCISODIS 0x44
#define PCIE0PHYISODIS (0x1 << 5)
/*ES register*/
#define ES_SOC_PCIE0PHY_ADDR 0xfc040000
#define ES_SOC_PCIEPHY_CTRL16 0x40
#define ES_SOC_PCIEPHY_CTRL17 0x44
#define ES_SOC_PCIEPHY_STAT16 0x440

#define ES_SC_PCIEPHY_CTRL16_MASK 0xFC000003
#define ES_SC_PCIEPHY_CTRL17_MASK 0xFC000000

#define PCIEPLL_REFDIV (0x1 << 2)
#define PCIEPLL_FBDIV (0x9c << 8)
#define PCIEPLL_POSTDIV1 (0x6 << 20)
#define PCIEPLL_POSTDIV2 (0x5 << 23)
#define PCIEPLL_CFG_VLD (0x1 << 25)
#define PCIEPLL_INT_MOD (0x0 << 24)
#define PCIEPLL_FRACDIV (0x400000 << 0)

#define GT_PCLK_PCIE_PHY (0x1 << 9)
#define GT_CLK_PCIE_HP (0x1 << 15)
#define GT_CLK_PCIE_DEBOUNCE (0x1 << 12)
#define GT_CLK_PCIE1_HP (0x1 << 19)
#define GT_CLK_PCIE1_DEBOUNCE (0x1 << 16)

/*cs register*/
#define CS_SC_MMC1PLL_CTRL0 0xc00
#define CS_SC_MMC1PLL_CTRL1 0xc04
#define CS_SC_MMC1PLL_CTRL16 0xC40
#define CS_SC_MMC1PLL_CTRL17 0xC44
#define CS_SC_MMC1PLL_CTRL20 0xC50
#define CS_SC_MMC1PLL_CTRL21 0xC54
#define CS_SC_MMC1PLL_STAT0 0xE00

/*define ie,oe cfg*/
#define IO_IE_POLAR (0x1 << 29)
#define IO_IE_EN_SOFT (0x1 << 28)
#define IO_IE_EN_HARD_BYPASS (0x1 << 27)
#define IO_OE_EN_HARD_BYPASS (0x1 << 11)
#define IO_HARD_CTRL_DEBOUNCE_BYPASS (0x1 << 10)
#define IO_OE_POLAR (0x1 << 9)
#define IO_OE_GT_MODE (0x2 << 7)
#define IO_OE_EN_SOFT (0x1 << 6)
#define DEBOUNCE_WAITCFG_IN (0xf << 20)
#define DEBOUNCE_WAITCFG_OUT (0xf << 13)

/*cs add noc power domain*/
#define PMCTRL_ADDR 0xFFF31000
#define NOC_POWER_IDLEREQ_1 0x38c
#define NOC_POWER_IDLE_1 0x394

enum cspll {
	SCPLL = 0,
	FNPLL = 1,
};

static u32 g_phy_apb_init = 0;

static int kirin_pcie_gpio_cfg(struct kirin_pcie *pcie, int pull_up)
{
	u32 val;

	if (pull_up) {
		if (pcie->dtsinfo.board_type == BOARD_FPGA) {
			val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
			val |= PERST_FUN_SEC;
			val |= PERST_ASSERT_EN;
			kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);
		} else {
			if (gpio_direction_output((unsigned int)pcie->gpio_id_reset, 1)) {
			        PCIE_PR_ERR("Failed to set reset_gpio direction input");
			        return -1;
			}
		}
	} else {
		if (pcie->dtsinfo.board_type == BOARD_FPGA) {
			val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
			val |= PERST_FUN_SEC;
			val &= ~PERST_ASSERT_EN;
			kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);
		} else {
			if (gpio_direction_output((unsigned int)pcie->gpio_id_reset, 0) ){
			        PCIE_PR_ERR("Failed to set reset_gpio direction input");
			        return -1;
			}
		}
	}

	return 0;
}

static int kirin_pcie_clk_ctrl(struct clk *clk, int clk_on)
{
	int ret = 0;

	if (clk_on) {
		ret = clk_prepare_enable(clk);
		if (ret)
			return ret;
	} else {
		clk_disable_unprepare(clk);
	}

	return ret;
}

static void kirin_pcie_reset_ctrl(struct kirin_pcie *pcie, int reset)
{
	u32 val;

	if (reset) {
		if (pcie->rc_id == 1) {
			val = pcie->dtsinfo.phy_assert_value;
			writel(val, pcie->crg_base + pcie->dtsinfo.phy_assert_offset);
		}
		if (g_phy_apb_init == 0) {
			val = g_kirin_pcie[0].dtsinfo.phy_assert_value;
			writel(val, g_kirin_pcie[0].crg_base + g_kirin_pcie[0].dtsinfo.phy_assert_offset);
		}
		val = pcie->dtsinfo.core_assert_value;
		writel(val, pcie->crg_base + pcie->dtsinfo.core_assert_offset);
	} else {
		val = pcie->dtsinfo.core_deassert_value | pcie->dtsinfo.phy_deassert_value;
		writel(val, pcie->crg_base + pcie->dtsinfo.core_deassert_offset);
	}
}

static void kirin_pcie_natural_cfg(struct kirin_pcie *pcie)
{
	u32 val;

	/*pull up sys_aux_pwr_det in sc_pciectrl_ctrl7*/
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	val |= (0x1 << 10);
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL7_ADDR);

	if (pcie->dtsinfo.ep_flag) {
		/*input*/
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
		val |= (0x3 << 2);
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);
	} else {
		/*output, pull down*/
		val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
		val &= ~(0x3 << 2);
		kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);
	}

	val = kirin_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
       val |= (0x1 << 17);//phy_reset
       val &= ~(0x1 << 19);//pipe_lane0_reset_n
       kirin_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

	if (pcie->dtsinfo.chip_type == CHIP_TYPE_ES) {
		 /*pull up phy0_ana_pwr_en signal */
	       val = kirin_phy_readl(pcie, SOC_PCIEPHY_CTRL33_ADDR);
	       val |= (0x1 << 1);
	       kirin_phy_writel(pcie, val, SOC_PCIEPHY_CTRL33_ADDR);
	}

	/*fix chip bug: TxDetectRx fail*/
	val = kirin_phy_readl(pcie, SOC_PCIEPHY_CTRL38_ADDR);
	val |= (0x1 << 2);
	kirin_phy_writel(pcie, val, SOC_PCIEPHY_CTRL38_ADDR);

}

static void kirin_pcie_es_pll_init(struct kirin_pcie *pcie, int enable)
{
	u32 val;
	void __iomem    *phy0_base;

	if (enable) {
		if (g_phy_apb_init == 0) {
			phy0_base = ioremap(ES_SOC_PCIE0PHY_ADDR, 0x40000);
			if (!phy0_base) {
				PCIE_PR_ERR("Failed to phy0_base ioremap");
				return;
			}
			/*pull down phy iso*/
			writel(PCIE0PHYISODIS, pcie->sctrl_base + SCTRL_SCISODIS);
			/*CRGPERIHPH  PEREN7[9] =0x1, enable phy0_pclk*/
			writel(GT_PCLK_PCIE_PHY, pcie->crg_base + CRGPERIHPH_PEREN7);
			/*CRGPERIHPH  PERRSTDIS3[27] =0x1, deassert apb phy0*/
			writel(IP_PRST_PCIE_PHY, pcie->crg_base + CRGPERIHPH_PERRSTDIS3);

			val = readl(phy0_base + ES_SOC_PCIEPHY_CTRL16);
			val &= ES_SC_PCIEPHY_CTRL16_MASK;
			val |= (PCIEPLL_REFDIV | PCIEPLL_FBDIV | PCIEPLL_POSTDIV1 | PCIEPLL_POSTDIV2);
			writel(val, phy0_base + ES_SOC_PCIEPHY_CTRL16);

			val = readl(phy0_base + ES_SOC_PCIEPHY_CTRL17);
			val &= ES_SC_PCIEPHY_CTRL17_MASK;
			val |= (PCIEPLL_CFG_VLD | PCIEPLL_FRACDIV);
			writel(val, phy0_base + ES_SOC_PCIEPHY_CTRL17);
			iounmap(phy0_base);
		}
		g_phy_apb_init++;
	} else {
		g_phy_apb_init--;
	}
}

/**
*kirin_pcie_cs_scpll_init: Init scpll,VCO = 3G;
*/
static void kirin_pcie_cs_scpll_init(struct kirin_pcie *pcie)
{
	u32 val;

	/*choose SCPLL*/
	val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL1);
	val &= ~(0x1 << 27);
	kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL1);

	val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL0);
	val &= 0xFC000003;
	/*scpll pciepll refdiv = 0x1*/
	val |= (0x1 << 2);
	/*scpll pciepll fbdiv = 0x9c*/
	val |= (0x9c << 8);
	/*scpll pciepll postdiv1 = 0x6*/
	val |= (0x6 << 20);
	/*scpll pciepll postdiv2 = 0x5*/
	val |= (0x5 << 23);
	kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL0);

	val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL1);
	val &= 0xFC000000;
	/*scpll pciepll cfg vld = 0x1*/
	val |= (0x1 << 25);
	/*scpll pciepll int mod = 0x0*/
	val &= ~(0x1 << 24);
	/*scpll pciepll fracdiv = 0x400000*/
	val |= (0x400000 << 0);
	kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL1);
}

/**
*kirin_pcie_cs_fnpll_init: Init fnpll,VCO = 3G;
*/
static void kirin_pcie_cs_fnpll_init(struct kirin_pcie *pcie)
{
	u32 val;

	/*choose FNPLL*/
	val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL1);
	val |= (0x1 << 27);
	kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL1);

	val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL16);
	val &= 0xF000FFFF;
	/*fnpll fbdiv = 0xD0*/
	val |= (0xd0 << 16);
	kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL16);

	val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL17);
	val &= 0xFF000000;
	/*fnpll fracdiv = 0x555555*/
	val |= (0x555555 << 0);
	kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL17);

	val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL20);
	val &= 0xF5FF88FF;
	/*fnpll dll_en = 0x1*/
	val |= (0x1 << 27);
	/*fnpll postdiv1 = 0x5*/
	val |= (0x5 << 8);
	/*fnpll postdiv2 = 0x4*/
	val |= (0x4 << 12);
	/*fnpll pll_mode = 0x0*/
	val &= ~(0x1 << 25);
	kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL20);

	kirin_phy_writel(pcie, 0x20, CS_SC_MMC1PLL_CTRL21);
}

static void kirin_pcie_cs_pll_init(struct kirin_pcie *pcie, enum cspll pll)
{
	if (pll == SCPLL)
		kirin_pcie_cs_scpll_init(pcie);
	else
		kirin_pcie_cs_fnpll_init(pcie);
}

static void kirin_pcie_pll_init(struct kirin_pcie *pcie, int enable)
{
	if (pcie->dtsinfo.chip_type == CHIP_TYPE_ES)
		kirin_pcie_es_pll_init(pcie, enable);
	else if (enable)
		kirin_pcie_cs_pll_init(pcie, FNPLL); /*choose fnpll*/
}

static int kirin_pcie_es_pll_ctrl(struct kirin_pcie *pcie, int enable)
{
	u32 val;

	if (enable) {
		/*gt_clk_pciepll = 0*/
		val = kirin_phy_readl(pcie, ES_SOC_PCIEPHY_CTRL17);
		val &= ~(0x1 << 26);
		kirin_phy_writel(pcie, val, ES_SOC_PCIEPHY_CTRL17);

		/*pciepll_en = 1*/
		val = kirin_phy_readl(pcie, ES_SOC_PCIEPHY_CTRL16);
		val |= (0x1 << 0);
		kirin_phy_writel(pcie, val, ES_SOC_PCIEPHY_CTRL16);

		udelay(200);//lint !e778  !e774  !e516 !e747
		val = kirin_phy_readl(pcie, ES_SOC_PCIEPHY_STAT16);
		if (pcie->dtsinfo.board_type != BOARD_FPGA) {
			if (!(val & 0x1)) {
				PCIE_PR_ERR("wait for pll_lock timeout");
				return -1;
			}
		}
		/*pciepll_bp = 0*/
		val = kirin_phy_readl(pcie, ES_SOC_PCIEPHY_CTRL16);
		val &= ~(0x1 << 1);
		kirin_phy_writel(pcie, val, ES_SOC_PCIEPHY_CTRL16);

		/*gt_clk_pciepll = 1*/
		val = kirin_phy_readl(pcie, ES_SOC_PCIEPHY_CTRL17);
		val |= (0x1 << 26);
		kirin_phy_writel(pcie, val, ES_SOC_PCIEPHY_CTRL17);
	} else {
		/*gt_clk_pciepll = 0*/
		val = kirin_phy_readl(pcie, ES_SOC_PCIEPHY_CTRL17);
		val &= ~(0x1 << 26);
		kirin_phy_writel(pcie, val, ES_SOC_PCIEPHY_CTRL17);

		/*pciepll_en = 0*/
		val = kirin_phy_readl(pcie, ES_SOC_PCIEPHY_CTRL16);
		val &= ~(0x1 << 0);
		kirin_phy_writel(pcie, val, ES_SOC_PCIEPHY_CTRL16);

		/*pciepll_bp = 1*/
		val = kirin_phy_readl(pcie, ES_SOC_PCIEPHY_CTRL16);
		val |= (0x1 << 1);
		kirin_phy_writel(pcie, val, ES_SOC_PCIEPHY_CTRL16);

		/*gt_clk_pciepll = 1*/
		val = kirin_phy_readl(pcie, ES_SOC_PCIEPHY_CTRL17);
		val |= (0x1 << 26);
		kirin_phy_writel(pcie, val, ES_SOC_PCIEPHY_CTRL17);
	}

	 return 0;
}

static int kirin_pcie_cs_scpll_ctrl(struct kirin_pcie *pcie, int enable)
{
	u32 val;
	int time = 200;

	if (enable) {
		/*pciepll_en = 1*/
		val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL0);
		val |= (0x1 << 0);
		kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL0);

		if (pcie->dtsinfo.board_type != BOARD_FPGA) {
			val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_STAT0);
			/*choose SCPLL*/
			while (!(val & 0x1)) {
				if (!time) {
					PCIE_PR_ERR("wait for pll_lock timeout");
					return -1;
				}
				time --;
				udelay(1);//lint !e778  !e774  !e516 !e747
				val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_STAT0);
			}
		}
		/*pciepll_bp = 0*/
		val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL0);
		val &= ~(0x1 << 1);
		kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL0);

	} else {
		/*pciepll_en = 0*/
		val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL0);
		val &= ~(0x1 << 0);
		kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL0);

		/*pciepll_bp = 1*/
		val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL0);
		val |= (0x1 << 1);
		kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL0);
	}

	 return 0;
}

static int kirin_pcie_cs_fnpll_ctrl(struct kirin_pcie *pcie, int enable)
{
	u32 val;
	int time = 200;

	if (enable) {
		/*pd = 0*/
		val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL16);
		val &= ~(0x1 << 0);
		kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL16);

		if (pcie->dtsinfo.board_type != BOARD_FPGA) {
			val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_STAT0);
			/*choose FNPLL*/
			while (!(val & 0x10)) {
				if (!time) {
					PCIE_PR_ERR("wait for pll_lock timeout");
					return -1;
				}
				time --;
				udelay(1);//lint !e778  !e774  !e516 !e747
				val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_STAT0);
			}
		}
		/*pciepll_bp = 0*/
		val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL20);
		val &= ~(0x1 << 16);
		kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL20);

	} else {
		/*pd = 1*/
		val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL16);
		val |= (0x1 << 0);
		kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL16);

		/*pciepll_bp = 1*/
		val = kirin_phy_readl(pcie, CS_SC_MMC1PLL_CTRL20);
		val |= (0x1 << 16);
		kirin_phy_writel(pcie, val, CS_SC_MMC1PLL_CTRL20);
	}

	 return 0;
}

static int kirin_pcie_cs_pll_ctrl(struct kirin_pcie *pcie, int enable, enum cspll pll)
{
	int ret;

	if (pll == SCPLL)
		ret = kirin_pcie_cs_scpll_ctrl(pcie, enable);
	else
		ret = kirin_pcie_cs_fnpll_ctrl(pcie, enable);

	return ret;
}

static int kirin_pcie_pll_ctrl(struct kirin_pcie *pcie, int enable)
{
	int ret;

	if (pcie->dtsinfo.chip_type == CHIP_TYPE_ES)
		ret = kirin_pcie_es_pll_ctrl(pcie, enable);
	else
		ret = kirin_pcie_cs_pll_ctrl(pcie, enable, FNPLL);

	return ret;
}

static void kirin_pcie_phyref_gt(struct kirin_pcie *pcie, int open)
{
	u32 val;

	if (open) {
		if (pcie->rc_id == 1) {
			/*   PCIE1  */
			/*en hard gt mode*/
			val = readl(pcie->crg_base + CRGPERIPH_PCIECTRL1);
			val &= ~(0x1 << 1);
			writel(val, pcie->crg_base + CRGPERIPH_PCIECTRL1);

			/*disable soft gt mode*/
			val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
			val |= (0x1 << 18);
			writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
		} else {
			/*   PCIE0  */
			/*en hard gt mode*/
			val = readl(pcie->crg_base + CRGPERIPH_PCIECTRL0);
			val &= ~(0x1 << 1);
			writel(val, pcie->crg_base + CRGPERIPH_PCIECTRL0);

			/*disable soft gt mode*/
			val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
			val |= (0x1 << 14);
			writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
		}
	} else {
		if (pcie->rc_id == 1) {
			/*   PCIE1  */
			/*disable hard gt mode*/
			val = readl(pcie->crg_base + CRGPERIPH_PCIECTRL1);
			val |= (0x1 << 1);
			writel(val, pcie->crg_base + CRGPERIPH_PCIECTRL1);

			/*disable soft gt mode*/
			val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
			val |= (0x1 << 18);
			writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);

		} else {
			/*   PCIE0  */
			/*disable hard gt mode*/
			val = readl(pcie->crg_base + CRGPERIPH_PCIECTRL0);
			val |= (0x1 << 1);
			writel(val, pcie->crg_base + CRGPERIPH_PCIECTRL0);

			/*disable soft gt mode*/
			val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
			val |= (0x1 << 14);
			writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
		}
	}
}

static void kirin_pcie_hp_debounce_gt(struct kirin_pcie *pcie, int open)
{
	u32 val;

	if (open) {
		if (pcie->rc_id == 0) {
			/*gt_clk_pcie_hp/gt_clk_pcie_debounce open*/
			val = readl(pcie->crg_base + CRGPERIPH_PEREN12);
			val |= (GT_CLK_PCIE_HP|GT_CLK_PCIE_DEBOUNCE);
			writel(val, pcie->crg_base + CRGPERIPH_PEREN12);
		} else {
			/*gt_clk_pcie1_hp/gt_clk_pcie1_debounce open*/
			val = readl(pcie->crg_base + CRGPERIPH_PEREN12);
			val |= (GT_CLK_PCIE1_HP|GT_CLK_PCIE1_DEBOUNCE);
			writel(val, pcie->crg_base + CRGPERIPH_PEREN12);
		}
	} else {
		if (pcie->rc_id == 0) {
			/*gt_clk_pcie_hp/gt_clk_pcie_debounce close*/
			val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
			val |= (GT_CLK_PCIE_HP|GT_CLK_PCIE_DEBOUNCE);
			writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
		} else {
			/*gt_clk_pcie1_hp/gt_clk_pcie1_debounce close*/
			val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
			val |= (GT_CLK_PCIE1_HP|GT_CLK_PCIE1_DEBOUNCE);
			writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
		}
	}
}


static void kirin_pcie_oe_ctrl(struct kirin_pcie *pcie, int en_flag)
{
	u32 val;
	u32 crg_pciectrl_offset;

	if (pcie->rc_id == 0)
		crg_pciectrl_offset = CRGPERIPH_PCIECTRL0;
	else
		crg_pciectrl_offset = CRGPERIPH_PCIECTRL1;

	val = readl(pcie->crg_base + crg_pciectrl_offset);

	/*set ie cfg*/
	val |= IO_IE_EN_HARD_BYPASS;

	/*set oe cfg*/
	val &= ~IO_HARD_CTRL_DEBOUNCE_BYPASS;

	/* set phy_debounce in&out time*/
	val |= (DEBOUNCE_WAITCFG_IN | DEBOUNCE_WAITCFG_OUT);

	/*select oe_gt_mode */
	val |= IO_OE_GT_MODE;

	if (en_flag)
		val &= ~IO_OE_EN_HARD_BYPASS;
	else
		val |= IO_OE_EN_HARD_BYPASS;

	writel(val, pcie->crg_base + crg_pciectrl_offset);
}


static void kirin_pcie_ioref_gt(struct kirin_pcie *pcie, int open)
{
       u32 val;
       u32 crg_pciectrl_offset;
       u32 crg_perdis12_val;

       if (pcie->rc_id == 0) {
               crg_pciectrl_offset = CRGPERIPH_PCIECTRL0;
               crg_perdis12_val = (0x1 << 13);
       } else {
               crg_pciectrl_offset = CRGPERIPH_PCIECTRL1;
               crg_perdis12_val = (0x1 << 17);
       }

       if (open) {
               kirin_elb_writel(pcie, 0x20000070, SOC_PCIECTRL_CTRL21_ADDR);
               kirin_pcie_oe_ctrl(pcie, ENABLE);

               /*en hard gt mode*/
               val = readl(pcie->crg_base + crg_pciectrl_offset);
               val &= ~(0x1 << 0);
               writel(val, pcie->crg_base + crg_pciectrl_offset);

               /*disable soft gt mode*/
               val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
               val |= crg_perdis12_val;
               writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
       } else {
               /*disable hard gt mode*/
               val = readl(pcie->crg_base + crg_pciectrl_offset);
               val |= (0x1 << 0);
               writel(val, pcie->crg_base + crg_pciectrl_offset);

               /*disable soft gt mode*/
               val = readl(pcie->crg_base + CRGPERIPH_PERDIS12);
               val |= crg_perdis12_val;
               writel(val, pcie->crg_base + CRGPERIPH_PERDIS12);
	       kirin_pcie_oe_ctrl(pcie, DISABLE);
       }
}

static int kirin_pcie_allclk_ctrl(struct kirin_pcie *pcie, int clk_on)
{
	u32 val;

	if (clk_on) {
		/*choose 100MHz clk src: Bit[8]==1 pad, Bit[8]==0 pll */
		val = kirin_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
		val &= ~(0x1 << 8);
		kirin_phy_writel(pcie, val, SOC_PCIEPHY_CTRL1_ADDR);

		if ((pcie->rc_id == 0) && (pcie->dtsinfo.chip_type == CHIP_TYPE_ES)) {
			/*pull up phy_ref_use_cio_pad*/
			val = kirin_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
			val |= (0x1 << 14);
			kirin_phy_writel(pcie, val, SOC_PCIEPHY_CTRL0_ADDR);
		}

		kirin_pcie_pll_init(pcie, ENABLE);

		if (pcie->dtsinfo.chip_type == CHIP_TYPE_ES) {
			/*   PCIE1 div gt*/
			/*CRGPERIPH CLKDIV21[14] = 0x1*/
			writel(0x40004000, pcie->crg_base + CRGPERIPH_CLKDIV21);

			/*CRGPERIPH CLKDIV26[8:5] = 0x0*/
			writel(0x1E00000, pcie->crg_base + CRGPERIPH_CLKDIV26);
		}

		if (kirin_pcie_pll_ctrl(pcie, ENABLE)) {
			PCIE_PR_ERR("Failed to enable pll");
			return -1;
		}
		kirin_pcie_hp_debounce_gt(pcie, ENABLE);
		kirin_pcie_phyref_gt(pcie, ENABLE);
		kirin_pcie_ioref_gt(pcie, ENABLE);

		if (pcie->dtsinfo.chip_type == CHIP_TYPE_CS) {
			if (clk_set_rate(pcie->pcie_aclk, AXI_CLK_FREQ)) {
				PCIE_PR_ERR("Failed to set rate");
				goto GT_CLOSE;
			}
		}
		if (kirin_pcie_clk_ctrl(pcie->pcie_aclk, ENABLE)) {
			PCIE_PR_ERR("Failed to enable pcie_aclk ");
			goto GT_CLOSE;
		}
		PCIE_PR_DEBUG("aclk on Done ");

		if (kirin_pcie_clk_ctrl(pcie->pcie_aux_clk, ENABLE)) {
			PCIE_PR_ERR("Failed to enable pcie_aux_clk ");
			goto AUX_CLK_FAIL;
		}
	} else {
		kirin_pcie_clk_ctrl(pcie->pcie_aux_clk, DISABLE);
		kirin_pcie_clk_ctrl(pcie->pcie_aclk, DISABLE);

		kirin_pcie_ioref_gt(pcie, DISABLE);
		kirin_pcie_phyref_gt(pcie, DISABLE);
		kirin_pcie_hp_debounce_gt(pcie, DISABLE);

		kirin_pcie_pll_ctrl(pcie, DISABLE);
		kirin_pcie_pll_init(pcie, DISABLE);
	}

	return 0;

AUX_CLK_FAIL:
	kirin_pcie_clk_ctrl(pcie->pcie_aclk, DISABLE);

GT_CLOSE:
	kirin_pcie_ioref_gt(pcie, DISABLE);
	kirin_pcie_phyref_gt(pcie, DISABLE);
	kirin_pcie_hp_debounce_gt(pcie, DISABLE);

	kirin_pcie_pll_ctrl(pcie, DISABLE);
	kirin_pcie_pll_init(pcie, DISABLE);

	return -1;
}

static int kirin_pcie_eco(struct kirin_pcie *pcie)
{
	void __iomem *checkdata;
	void __iomem *sramdata;
	u32 phy_sram_base;
	struct kirin_pcie_dtsinfo *dtsinfo;

	if (pcie->rc_id == 0)
		phy_sram_base = PCIE0_SRAM_BASE;
	else
		phy_sram_base = PCIE1_SRAM_BASE;

	dtsinfo = &(pcie->dtsinfo);

	sramdata = ioremap(phy_sram_base, SRAM_SIZE);
	if (!sramdata) {
		PCIE_PR_ERR("Failed to ioremap sramdata");
		return -1;
	}

	if (dtsinfo->eco == ECO_TEST) {
		/*lint -e124 -esym(124,*)  */
		checkdata = (char *)kmalloc(SRAM_TEST_SIZE * 2, GFP_KERNEL);
		if (!checkdata) {
			PCIE_PR_ERR("Failed to alloc checkdata");
			goto UNMAP;
		}
		memset(checkdata, 0x0, SRAM_TEST_SIZE * 2);
		memcpy_fromio(checkdata, sramdata, SRAM_TEST_SIZE);
		memset_io(sramdata, 0x0, SRAM_TEST_SIZE);
		if (memcmp(sramdata, checkdata + SRAM_TEST_SIZE, SRAM_TEST_SIZE) != 0) {
			PCIE_PR_ERR("sram data clear fail");
			goto ECO_TEST_FAIL;
		}
		memcpy_toio(sramdata, checkdata, SRAM_TEST_SIZE);
		if (memcmp(sramdata, checkdata + SRAM_TEST_SIZE, SRAM_TEST_SIZE) == 0) {
			PCIE_PR_ERR("sram data all is 0, write eco data fail");
			goto ECO_TEST_FAIL;
		}
		PCIE_PR_ERR("ECO TEST SUCCESS!");
		kfree(checkdata);
		/*lint -e124 +esym(124,*) */
	}

	/*updata firmware in ES, to solve 70ms delay*/
	writel(0x7CF, sramdata + 0xB84);
	writel(0x7CF, sramdata + 0xD80);

	iounmap(sramdata);
	return 0;
ECO_TEST_FAIL:
	kfree(checkdata);
UNMAP:
	iounmap(sramdata);
	return -1;
}

static int kirin_pcie_phy_init(struct kirin_pcie *pcie)
{
	u32 reg_val;
	u32 sram_init_done = 0x1 << 0;
	u32 time = 10;
	struct kirin_pcie_dtsinfo *dtsinfo;

	dtsinfo = &pcie->dtsinfo;

	/*Set Bit0=0'b: pull down phy0_sram_bypass signal, choose ECO */
	/*Set Bit0=1'b: pull up phy0_sram_bypass signal, not choose ECO */
	reg_val = kirin_phy_readl(pcie, SOC_PCIEPHY_CTRL40_ADDR);
	if (dtsinfo->eco)
		reg_val &= ~(0x1 << 0);
	else
		reg_val |= (0x1 << 0);
	kirin_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL40_ADDR);

	/*pull down phy_test_powerdown signal */
	reg_val = kirin_phy_readl(pcie, SOC_PCIEPHY_CTRL0_ADDR);
	reg_val &= ~(0x1 << 22);
	kirin_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL0_ADDR);

	if (dtsinfo->eco) {
		reg_val = kirin_phy_readl(pcie, SOC_PCIEPHY_CTRL1_ADDR);
		reg_val &= ~(0x1 << 17);//phy_reset_sel
		reg_val |= (0x1 << 16);//phy_reset
		kirin_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL1_ADDR);
		udelay(1);//lint !e778  !e774
		reg_val &= ~(0x1 << 16);//phy_dereset
		kirin_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL1_ADDR);
	}

	/* deassert controller perst_n*/
	reg_val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	reg_val |= (0x1 << 2);
	kirin_elb_writel(pcie, reg_val, SOC_PCIECTRL_CTRL12_ADDR);
	udelay(10);//lint !e778  !e774  !e516 !e747

	if (dtsinfo->eco) {
		reg_val = kirin_phy_readl(pcie, SOC_PCIEPHY_STATE39_ADDR);
		while (!(reg_val & sram_init_done)) {
			udelay(100);//lint !e778  !e774  !e516 !e747
			if (time == 0) {
				PCIE_PR_ERR("phy0_sram_init_done fail");
				return -1;
			}
			time--;
			reg_val = kirin_phy_readl(pcie, SOC_PCIEPHY_STATE39_ADDR);
		}
		if (kirin_pcie_eco(pcie)) {
			PCIE_PR_ERR("eco init fail");
			return -1;
		}

		/*pull up phy0_sram_ext_ld_done signal, not choose ECO */
		reg_val = kirin_phy_readl(pcie, SOC_PCIEPHY_CTRL40_ADDR);
		reg_val |= (0x1 << 4);
		kirin_phy_writel(pcie, reg_val, SOC_PCIEPHY_CTRL40_ADDR);
	}

	return 0;
}

static int kirin_pcie_noc_power(struct kirin_pcie *pcie, int enable)
{
	void __iomem *pmctrl;
	u32 time = 100;
	u32 val;
	int ret = 0;

	if (pcie->dtsinfo.chip_type == CHIP_TYPE_ES)
		return 0;

	pmctrl = ioremap(PMCTRL_ADDR, 0x1000);
	if (!pmctrl) {
		PCIE_PR_ERR("Failed to ioremap pmctrl");
		return -1;
	}

	time = 100;
	if (enable) {
		writel(0x10001, pmctrl + NOC_POWER_IDLEREQ_1);
		val = readl(pmctrl + NOC_POWER_IDLE_1);
		while(!(val & 0x1)) {
			udelay(1);
			if (time == 0) {
				PCIE_PR_ERR("Failed to enable noc power idle");
				ret = -1;
				goto END;
			}
			time--;
			val = readl(pmctrl + NOC_POWER_IDLE_1);
		}
	} else {
		writel(0x10000, pmctrl + NOC_POWER_IDLEREQ_1);
		val = readl(pmctrl + NOC_POWER_IDLE_1);
		while(val & 0x1) {
			udelay(1);
			if (time == 0) {
				ret = -1;
				PCIE_PR_ERR("Failed to disable noc power idle");
				goto END;
			}
			time--;
			val = readl(pmctrl + NOC_POWER_IDLE_1);
		}
	}

END:
	iounmap(pmctrl);
	return ret;
}

int kirin_pcie_turn_on(struct pcie_port *pp, enum rc_power_status on_flag)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pp);//lint !e826
	u32 time = 10;
	u32 val;
	u32 pipe_clk_stable = 0x1 << 19;

	PCIE_PR_DEBUG("++ ON ++");

	if (pcie->is_power_on) {
		PCIE_PR_INFO("PCIe%d already power on", pcie->rc_id);
		return 0;
	}

	/*pull down phy ISO*/
	kirin_pcie_iso_ctrl(pcie, DISABLE);
	PCIE_PR_DEBUG("iso disable Done ");

	/*enable pcie_phy_apb_pclk,pcie_ctrl_apb_pclk*/
	if (kirin_pcie_clk_ctrl(pcie->apb_phy_clk, ENABLE)) {
		PCIE_PR_ERR("Failed to enable apb_phy_clk");
		return -1;
	}

	if (kirin_pcie_clk_ctrl(pcie->apb_sys_clk, ENABLE)) {
		PCIE_PR_ERR("Failed to enable apb_sys_clk");
		goto APBPHY_CLK_CLOSE;
	}

	PCIE_PR_DEBUG("pciectrl/pciephy pclk open Done ");

	/*unset module*/
	kirin_pcie_reset_ctrl(pcie, DISABLE);
	PCIE_PR_DEBUG("module unreset Done ");

	kirin_pcie_natural_cfg(pcie);
	PCIE_PR_DEBUG("natural cfg Done ");

	if (kirin_pcie_allclk_ctrl(pcie, ENABLE)) {
		goto APBSYS_CLK_CLOSE;
	}

	if (kirin_pcie_phy_init(pcie)) {
		goto ALLCLK_CLOSE;
	}

	/*unrst EP*/
	mdelay((unsigned long)22);//lint !e778  !e774  !e516  !e747  !e845

	kirin_pcie_gpio_cfg(pcie, ENABLE);
	mdelay((unsigned long)7);//lint !e778  !e774  !e516  !e747  !e845

	time = 100;
	if (pcie->dtsinfo.board_type != BOARD_FPGA) {
		val = kirin_phy_readl(pcie, SOC_PCIEPHY_STATE0_ADDR);
		while (val & pipe_clk_stable) {
			mdelay(1);//lint !e778  !e774  !e516 !e747 !e845
			if (time == 0) {
				PCIE_PR_ERR("PIPE clk is not stable");
				goto GPIO_DISABLE;
			}
			time--;
			val = kirin_phy_readl(pcie, SOC_PCIEPHY_STATE0_ADDR);
		}
	}

	if (kirin_pcie_noc_power(pcie, DISABLE)) {
		goto GPIO_DISABLE;
	}

	pcie->is_power_on = 1;

	PCIE_PR_DEBUG("-- ON --");
	return 0;

GPIO_DISABLE:
	kirin_pcie_gpio_cfg(pcie, DISABLE);
ALLCLK_CLOSE:
	kirin_pcie_allclk_ctrl(pcie, DISABLE);
APBSYS_CLK_CLOSE:
	kirin_pcie_reset_ctrl(pcie, ENABLE);
	kirin_pcie_clk_ctrl(pcie->apb_sys_clk, DISABLE);
APBPHY_CLK_CLOSE:
	kirin_pcie_clk_ctrl(pcie->apb_phy_clk, DISABLE);

	return -1;
}

int kirin_pcie_turn_off(struct pcie_port *pp, enum rc_power_status on_flag)
{
	struct kirin_pcie *pcie = to_kirin_pcie(pp);//lint !e826
	u32 val;
	int ret = 0;

	PCIE_PR_DEBUG("++ OFF ++");

	if (!pcie->is_power_on) {
		PCIE_PR_INFO("PCIe%d already power off", pcie->rc_id);
		return 0;
	}

	if (kirin_pcie_noc_power(pcie, ENABLE))
		return -1;

	kirin_pcie_gpio_cfg(pcie, DISABLE);

	mdelay((unsigned long)10);//lint !e778  !e774  !e516  !e747  !e845

	/*rst controller perst_n*/
	val = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL12_ADDR);
	val &= ~(0x1 << 2);
	kirin_elb_writel(pcie, val, SOC_PCIECTRL_CTRL12_ADDR);

	kirin_pcie_allclk_ctrl(pcie, DISABLE);
	kirin_pcie_reset_ctrl(pcie, ENABLE);

	kirin_pcie_clk_ctrl(pcie->apb_sys_clk, DISABLE);
	kirin_pcie_clk_ctrl(pcie->apb_phy_clk, DISABLE);

	if (pcie->dtsinfo.chip_type == CHIP_TYPE_CS) {
		kirin_pcie_iso_ctrl(pcie, ENABLE);
	}
	pcie->is_power_on = 0;
	PCIE_PR_DEBUG("-- OFF --");

	return ret;
}


/*lint -e438 -e550 -e715 -e732 -e747 -e750 -e838 +esym(438,*) +esym(550,*) +esym(715,*) +esym(732,*) +esym(747,*) +esym(750,*) +esym(838,*) */

