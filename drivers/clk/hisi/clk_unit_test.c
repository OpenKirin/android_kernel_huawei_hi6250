/*
 * Copyright (C) 2016-2017
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Standard functionality for the debug clock API.  See Documentation/clk.txt
 */
#if 0
#include <linux/clk-provider.h>
#include <linux/clk/clk-conf.h>
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/hwspinlock.h>
#include <soc_pmctrl_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_crgperiph_interface.h>

#include <soc_media1_crg_interface.h>
#include <soc_media2_crg_interface.h>

#include <soc_crgperiph_interface.h>
#include <soc_iomcu_interface.h>
#include <pmic_interface.h>
#include <soc_pctrl_interface.h>
#include <linux/of_address.h>
#include "clk_unit_test.h"
#include <linux/clk.h>

#define WIDTH_TO_MASK(width)			((1 << (width)) - 1)

#define hi3xxx_CLK_GATE_STATUS_OFFSET		0x8
/*lint -e750 +esym(750,*) */

#define CLK_ON	1
#define CLK_OFF	0


struct hi3xxx_periclk {
	struct clk_hw	hw;
	void __iomem	*enable;	/* enable register */
	void __iomem	*reset;		/* reset register */
	u32		ebits;		/* bits in enable/disable register */
	u32		rbits;		/* bits in reset/unreset register */
	void __iomem	*sctrl;		/*sysctrl addr*/
	void __iomem	*pmctrl;	/*pmctrl addr*/
	const char 	*friend;
	spinlock_t	*lock;
	u32		flags;
	struct hwspinlock	*clk_hwlock;
	u32		peri_dvfs_sensitive;/*0:non,1:direct avs,rate(HZ):sensitive rate*/
	u32		perivolt_poll_id;
	u32		sensitive_pll;
	u32		always_on;
	u32		gate_abandon_enable;
	u32		sync_time;
	u32		clock_id;
	int		pmu_clk_enable;
};

struct hi3xxx_divclk {
	struct clk_hw	hw;
	void __iomem	*reg;		/* divider register */
	u8		shift;
	u8		width;
	u32		mbits;		/* mask bits in divider register */
	const struct clk_div_table	*table;
	spinlock_t	*lock;
};

extern struct list_head clocks;
static DEFINE_MUTEX(clock_list_lock);
#define to_clk_gate(_hw) container_of(_hw, struct clk_gate, hw)
#define to_clk_mux(_hw) container_of(_hw, struct clk_mux, hw)

int
clock_enable_emulator(char * name)
{
	struct clk_core *clk = NULL;
	char *clk_name = NULL;
	int ret = 0;
	int err = 0;

	mutex_lock(&clock_list_lock);

	clk_name = name;

	/* Check if we have such a clock in the clocks list. if exist, prepare and enable it.*/
	list_for_each_entry(clk, &clocks, node) {
		if (!clk)
			goto out;
		if (strcmp(clk->name, clk_name) == 0) {
			pr_err("[old]:enable_refcnt = %d\n", clk->enable_count);

			ret = clk_prepare_enable(clk->hw->clk);
			if (ret) {
				err = -EINVAL;
				goto out;
			}
			pr_err("[new]:enable_refcnt = %d\n", clk->enable_count);
			err = -1;
			goto out;
		}
	}

	pr_err("clk name error!\n");

out:
	mutex_unlock(&clock_list_lock);
	return err;
}

int
clock_disable_emulator(char * name)
{
	struct clk_core *clk = NULL;
	char *clk_name = NULL;
	int err = 0;

	mutex_lock(&clock_list_lock);

	/*copy clock name from user space.*/
	clk_name = name;
	/* Check if we have such a clock in the clocks list. if exist, disable and unprepare it.*/
	list_for_each_entry(clk, &clocks, node) {
		if (!clk)
			goto out;
        if (strcmp(clk->name, clk_name) == 0) {
			pr_err("[old]:enable_refcnt = %d\n", clk->enable_count);

			clk_disable_unprepare(clk->hw->clk);

			pr_err("[new]:enable_refcnt = %d\n", clk->enable_count);
			err = -1;
			goto out;
		}
	}
	pr_err("clk name error!\n");

out:
	mutex_unlock(&clock_list_lock);
	return err;
}

int
clock_setrate_emulator(char * name, unsigned long setrate)
{
	struct clk_core *clk = NULL;
	char *clk_name;
	unsigned long rate;
	int ret = 0;
	int err = 0;

	clk_name = name;
	mutex_lock(&clock_list_lock);

	rate = setrate;

	/* Check if we have such a clock in the clocks list. if exist, set rate of this clock.*/
	list_for_each_entry(clk, &clocks, node) {
		if (!clk)
			goto out;
		if (strcmp(clk->name, clk_name) == 0) {
			pr_err("\n\n");
			pr_err("[%s]: old rate = %ld , target rate = %ld\n", clk->name, clk->rate, rate);
            ret = clk_set_rate(clk->hw->clk, rate);
			rate = clk_get_rate(clk->hw->clk);
			pr_err("ret = %ld , rate = %ld\n", ret, rate);
			if (ret)
				err = -EINVAL;
			goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error. */
	pr_err("clk name error!\n\n");

out:
	mutex_unlock(&clock_list_lock);
	return err;
}

static unsigned int hisi_get_table_div(const struct clk_div_table *table,
							unsigned int val)
{
	const struct clk_div_table *clkt;

	for (clkt = table; clkt->div; clkt++)
		if (clkt->val == val)
			return clkt->div;
	return 0;
}

void
clkgate_offsetbit_check(char * name, u32 clk_offset, unsigned int clk_bit, u32 flag)
{
	struct clk_core *clk = NULL;
	struct hi3xxx_periclk *pclk;
	char *clk_name;
	bool err = false;
    u32 val;
    u32 rdata[2]={0};
    struct device_node *np = NULL;
	clk_name = name;
	mutex_lock(&clock_list_lock);

	/* Check if we have such a clock in the clocks list. if exist, set rate of this clock.*/
	list_for_each_entry(clk, &clocks, node) {
		if (!clk)
			goto out;
		if (strcmp(clk->name, clk_name) == 0) {
            pclk = container_of(clk->hw, struct hi3xxx_periclk, hw);
            val = readl(pclk->enable + hi3xxx_CLK_GATE_STATUS_OFFSET);
            val &= BIT(clk_bit);
            np = of_find_node_by_name(NULL, clk_name);
            if(!np){
		        pr_err("[%s] fail to find np!\n",__func__);
                goto out;
            }
            if (of_property_read_u32_array(np, "hisilicon,hi3xxx-clkgate",
				       &rdata[0], 2)) {
		        pr_err("[%s] node doesn't have clkmux-reg property!\n",
				__func__);
		        goto out;
            }
            if(flag == 1){
			    if (pclk->enable && val && (rdata[0] == clk_offset))
				    err = true;
            }else{
                if (!val && (rdata[0] == clk_offset))
                    err = true;
            }
			goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error. */
	pr_err("clk name error!\n\n");

out:
	mutex_unlock(&clock_list_lock);
    if(!err)
        pr_err("Fail: [%s] fail to gate!\n", name);
    else
        pr_err("Succ: [%s] succ to gate!\n", name);
}

void
clkmux_offsetbit_check(char * name, u32 clk_offset, u32 shift, u32 clk_bit)
{
	struct clk_core *clk = NULL;
    struct clk_mux *mux;
	char *clk_name;
	bool err = false;
    u32 val;
    u32 rdata[2]={0};
    struct device_node *np = NULL;

	clk_name = name;
	mutex_lock(&clock_list_lock);

	/* Check if we have such a clock in the clocks list. if exist, set rate of this clock.*/
	list_for_each_entry(clk, &clocks, node) {
		if (!clk)
			goto out;

		if (strcmp(clk->name, clk_name) == 0) {
            mux = to_clk_mux(clk->hw);
            val = readl(mux->reg) >> ((mux->shift));
            val &= mux->mask;
            val ^= clk_bit;
            np = of_find_node_by_name(NULL, clk_name);
            if(!np){
		        pr_err("[%s] fail to find np!\n",__func__);
                goto out;
            }
	        if (of_property_read_u32_array(np, "hisilicon,clkmux-reg",
				       &rdata[0], 2)) {
		        pr_err("[%s] node doesn't have clkmux-reg property!\n",
				__func__);
		        goto out;
	        }
            if (!val && ((clk_offset == rdata[0])) && mux->shift == shift)
				err = true;
			goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error. */
	pr_err("clk name error!\n\n");

out:
	mutex_unlock(&clock_list_lock);
    if(!err)
        pr_err("Fail: [%s] fail to mux!\n", name);
    else
        pr_err("Succ: [%s] succ to mux!\n", name);
}

void
clkdiv_offsetbit_check(char * name, u32 clk_offset, unsigned int div, u32 div_start, u32 div_end)
{
	struct clk_core *clk = NULL;
    struct hi3xxx_divclk *dclk;
	char *clk_name;
	bool err = false;
    u32 val;
    u32 rdiv;
    u32 width;
    struct device_node *np = NULL;
    u32 rdata[2]={0};
	clk_name = name;
	mutex_lock(&clock_list_lock);

	/* Check if we have such a clock in the clocks list. if exist, set rate of this clock.*/
	list_for_each_entry(clk, &clocks, node) {
		if (!clk)
			goto out;
		if (strcmp(clk->name, clk_name) == 0) {
            dclk = container_of(clk->hw, struct hi3xxx_divclk, hw);
            width = div_end - div_start + 1;
            val = readl(dclk->reg) >>dclk->shift;
	        val &= WIDTH_TO_MASK(dclk->width);
	        rdiv = hisi_get_table_div(dclk->table, val);
			np = of_find_node_by_name(NULL, clk_name);
            if(!np){
		        pr_err("[%s] fail to find np!\n",__func__);
                goto out;
            }
	        if (of_property_read_u32_array(np, "hisilicon,clkdiv",
				       &rdata[0], 2)) {
		        pr_err("[%s] node doesn't have clkdiv-reg property!\n",
				__func__);
		        goto out;
	        }

            if(rdiv == div && dclk->shift == div_start && dclk->width == width && (clk_offset == rdata[0]))
                err = true;
            goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error. */
	pr_err("clk name error!\n\n");

out:
	mutex_unlock(&clock_list_lock);
    if(!err)
        pr_err("Fail: [%s] fail to div!\n", name);
    else
        pr_err("Succ: [%s] succ to div!\n", name);
}

void
clkmaskgate_offsetbit_check(char * name, unsigned int clk_offset, unsigned int clk_bit, u32 flag)
{
	struct clk_core *clk = NULL;
    struct clk_gate *gate;
	char *clk_name;
	int err = 0;
    u32 val;
    u32 rdata[2]={0};
    struct device_node *np = NULL;

	clk_name = name;
	mutex_lock(&clock_list_lock);

	/* Check if we have such a clock in the clocks list. if exist, set rate of this clock.*/
	list_for_each_entry(clk, &clocks, node) {
		if (!clk)
			goto out;

		if (strcmp(clk->name, clk_name) == 0) {
            gate = to_clk_gate(clk->hw);
            val = readl(gate->reg);
            val &= BIT(clk_bit);

            np = of_find_node_by_name(NULL, clk_name);
            if(!np){
		        pr_err("[%s] fail to find np!\n",__func__);
                goto out;
            }
            if (of_property_read_u32_array(np, "hisilicon,clkgate",&rdata[0], 2)) {
			    pr_err("[%s] node doesn't have clkgate property!\n", __func__);
			    goto out;
            }
            if(flag == 1){
			    if (val && (rdata[0] == clk_offset))
				    err = true;
            }else{
                if (!val && (rdata[0] == clk_offset))
				    err = true;
            }
			goto out;
		}
	}
	/* if clk wasn't in the clocks list, clock name is error. */
	pr_err("clk name error!\n\n");

out:
	mutex_unlock(&clock_list_lock);
    if(!err)
        pr_err("Fail: [%s] fail to andgate!\n", name);
    else
        pr_err("Succ: [%s] succ to andgate!\n", name);
}



int hisi_clk_test_check(void)
{
	pr_err("********************************\n");
	pr_err("\n\n\n");
	pr_err("START CLOCK UNIT TEST\n");
	pr_err("\n\n\n");
	pr_err("********************************\n");

	char * name = NULL;

	name = "clk_ppll0_media";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check(name, 0x1B0, SOC_SCTRL_SCPEREN4_gt_clk_ppll0_media_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check(name, 0x1B0, SOC_SCTRL_SCPEREN4_gt_clk_ppll0_media_START, CLK_ON);

	name = "clk_ppll2_media";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check(name, 0x410, SOC_CRGPERIPH_PEREN6_gt_clk_ppll2_media_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check(name, 0x410, SOC_CRGPERIPH_PEREN6_gt_clk_ppll2_media_START, CLK_OFF);


	name = "clk_ppll3_media";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check(name, 0x410, SOC_CRGPERIPH_PEREN6_gt_clk_ppll3_media_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check(name, 0x410, SOC_CRGPERIPH_PEREN6_gt_clk_ppll3_media_START, CLK_OFF);

	name = "clk_ppll4_media";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check(name, 0x410, SOC_CRGPERIPH_PEREN6_gt_clk_ppll4_media_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check(name, 0x410, SOC_CRGPERIPH_PEREN6_gt_clk_ppll4_media_START, CLK_OFF);

	name = "pclk_wd0";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_wd0_high", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_wd0_START, CLK_ON);
   
	clock_setrate_emulator(name, 116000000);
    clkmux_offsetbit_check("clk_wd0_mux", 0x140, SOC_CRGPERIPH_PERTIMECTRL_wdog0enov_START, 0x1);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 2, 0, 1);

	clock_setrate_emulator(name, 58000000);
    clkmux_offsetbit_check("clk_wd0_mux", 0x140, SOC_CRGPERIPH_PERTIMECTRL_wdog0enov_START, 0x1);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, 0, 1);

	clock_setrate_emulator(name, 232000000);
    clkmux_offsetbit_check("clk_wd0_mux", 0x140, SOC_CRGPERIPH_PERTIMECTRL_wdog0enov_START, 0x1);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, 0, 1);

	clock_setrate_emulator(name, 32764);
    clkmux_offsetbit_check("clk_wd0_mux", 0x140, SOC_CRGPERIPH_PERTIMECTRL_wdog0enov_START, 0x0);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_wd0_high", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_wd0_START, CLK_OFF);

	name = "pclk_dss";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_dss", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_pclk_dss_START, CLK_ON);
    clkgate_offsetbit_check("pclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_disp_noc_subsys_START, CLK_ON);

    clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_dss", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_pclk_dss_START, CLK_OFF);


	name = "pclk_dsi0";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_dsi0", 0x50, SOC_CRGPERIPH_PEREN5_gt_pclk_dsi0_START, CLK_ON);

    clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, 0, 1);

    clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, 0, 1);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_dsi0", 0x50, SOC_CRGPERIPH_PEREN5_gt_pclk_dsi0_START, CLK_OFF);

	name = "clk_codecssi";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_codecssi", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_codecssi_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_codecssi", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_codecssi_START, CLK_OFF);


	name = "pclk_codecssi";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_codecssi", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_codecssi_START, CLK_ON);
    clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, 0, 1);

    clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, 0, 1);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_codecssi", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_codecssi_START, CLK_OFF);


	name = "pclk_ioc";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_ioc", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_ioc_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_ioc", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_ioc_START, CLK_OFF);

	name = "hclk_usb2otg";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("hclk_usb2otg", 0x0, SOC_CRGPERIPH_PEREN0_gt_hclk_usb2otg_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("hclk_usb2otg", 0x0, SOC_CRGPERIPH_PEREN0_gt_hclk_usb2otg_START, CLK_OFF);


	name = "hclk_sdio";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("hclk_sdio", 0x0, SOC_CRGPERIPH_PEREN0_gt_hclk_sdio_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("hclk_sdio", 0x0, SOC_CRGPERIPH_PEREN0_gt_hclk_sdio_START, CLK_OFF);

	name = "pclk_mmc0_ioc";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_mmc0_ioc", 0x40, SOC_CRGPERIPH_PEREN4_gt_pclk_mmc0_ioc_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_mmc0_ioc", 0x40, SOC_CRGPERIPH_PEREN4_gt_pclk_mmc0_ioc_START, CLK_OFF);

	name = "pclk_mmc1_ioc";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_mmc1_ioc", 0x420, SOC_CRGPERIPH_PEREN7_gt_pclk_mmc1_ioc_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_mmc1_ioc", 0x420, SOC_CRGPERIPH_PEREN7_gt_pclk_mmc1_ioc_START, CLK_OFF);

	name = "pclk_perf_stat";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_perf_stat", 0x40, SOC_CRGPERIPH_PEREN4_gt_pclk_perf_stat_START, CLK_ON);
    clkmaskgate_offsetbit_check("autodiv_dmabus", 0x404, SOC_CRGPERIPH_PERI_AUTODIV_ACPU_dmabus_div_auto_reduce_bypass_acpu_START, CLK_ON);

	clock_setrate_emulator(name, 116000000);
    clkdiv_offsetbit_check("clk_dmabus_div", 0xEC, 2, SOC_CRGPERIPH_CLKDIV17_div_dmabus_START, SOC_CRGPERIPH_CLKDIV17_div_dmabus_END);

	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("clk_dmabus_div", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_dmabus_START, SOC_CRGPERIPH_CLKDIV17_div_dmabus_END);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_perf_stat", 0x40, SOC_CRGPERIPH_PEREN4_gt_pclk_perf_stat_START, CLK_OFF);
    clkmaskgate_offsetbit_check("autodiv_dmabus", 0x404, SOC_CRGPERIPH_PERI_AUTODIV_ACPU_dmabus_div_auto_reduce_bypass_acpu_START, CLK_OFF);


	name = "clk_perf_stat";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_perf_stat", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_perf_stat_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_perf_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_perf_stat_div_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_a53hpm_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_a53hpm_START, CLK_ON);

	clock_setrate_emulator(name, 480000000);
    clkdiv_offsetbit_check("clk_perf_div", 0xD0, 1, SOC_CRGPERIPH_CLKDIV10_div_perf_stat_START, SOC_CRGPERIPH_CLKDIV10_div_perf_stat_END);

	clock_setrate_emulator(name, 30000000);
    clkdiv_offsetbit_check("clk_perf_div", 0xD0, 16, SOC_CRGPERIPH_CLKDIV10_div_perf_stat_START, SOC_CRGPERIPH_CLKDIV10_div_perf_stat_END);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_perf_stat", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_perf_stat_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_perf_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_perf_stat_div_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_a53hpm_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_a53hpm_START, CLK_OFF);


	name = "clk_dmac";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_dmac", 0x30, SOC_CRGPERIPH_PEREN3_gt_aclk_dmac_acpu_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_dmac", 0x30, SOC_CRGPERIPH_PEREN3_gt_aclk_dmac_acpu_START, CLK_OFF);

	name = "clk_uart3";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_uart7";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_uart8";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_i2c6";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_i2c6_gt", 0x10, SOC_IOMCU_CLKEN0_iomcu_clken0_27i2c6_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_i2c6_gt", 0x10, SOC_IOMCU_CLKEN0_iomcu_clken0_27i2c6_START, CLK_OFF);

	name = "clk_i2c1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_i2c1_gt", 0x10, SOC_IOMCU_CLKEN0_iomcu_clken0_3i2c1_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_i2c1_gt", 0x10, SOC_IOMCU_CLKEN0_iomcu_clken0_3i2c1_START, CLK_OFF);

	name = "clk_secs";
	pr_err("\n");
	clock_enable_emulator(name);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clk_sec_div", 0xD0, 1, SOC_CRGPERIPH_CLKDIV10_div_sec_START, SOC_CRGPERIPH_CLKDIV10_div_sec_END);
    clkmux_offsetbit_check("clk_sec_pll_mux", 0xD0, SOC_CRGPERIPH_CLKDIV10_sel_sec_pll_START, 0x1);

	clock_setrate_emulator(name, 30000000);
    clkdiv_offsetbit_check("clk_sec_div", 0xD0, 32, SOC_CRGPERIPH_CLKDIV10_div_sec_START, SOC_CRGPERIPH_CLKDIV10_div_sec_END);
    clkmux_offsetbit_check("clk_sec_pll_mux", 0xD0, SOC_CRGPERIPH_CLKDIV10_sel_sec_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clk_sec_div", 0xD0, 1, SOC_CRGPERIPH_CLKDIV10_div_sec_START, SOC_CRGPERIPH_CLKDIV10_div_sec_END);
    clkmux_offsetbit_check("clk_sec_pll_mux", 0xD0, SOC_CRGPERIPH_CLKDIV10_sel_sec_pll_START, 0x0);

	clock_disable_emulator(name);



	name = "clk_socp_acpu";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_socp_acpu", 0x10, SOC_CRGPERIPH_PEREN1_gt_clk_socp_acpu_START, CLK_ON);


	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_socp_acpu", 0x10, SOC_CRGPERIPH_PEREN1_gt_clk_socp_acpu_START, CLK_OFF);


	name = "clk_timestp_gt";
	pr_err("\n");
	clock_enable_emulator(name);
    clkmaskgate_offsetbit_check("clk_timestp_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_time_stamp_div_START, CLK_ON);

	clock_disable_emulator(name);
    /*alwayson*/
    clkmaskgate_offsetbit_check("clk_timestp_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_time_stamp_div_START, CLK_ON);

	name = "clk_time_stamp";
	pr_err("\n");

	clock_setrate_emulator(name, 116000000);
    clkdiv_offsetbit_check("clk_timestp_div", 0x128, 2, SOC_CRGPERIPH_PERI_CTRL2_div_time_stamp_START, SOC_CRGPERIPH_PERI_CTRL2_div_time_stamp_END);

	clock_setrate_emulator(name, 14500000);
    clkdiv_offsetbit_check("clk_timestp_div", 0x128, 16, SOC_CRGPERIPH_PERI_CTRL2_div_time_stamp_START, SOC_CRGPERIPH_PERI_CTRL2_div_time_stamp_END);

	name = "clk_ipf";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ipf", 0x0, SOC_CRGPERIPH_PEREN0_gt_clk_ipf_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ipf", 0x0, SOC_CRGPERIPH_PEREN0_gt_clk_ipf_START, CLK_OFF);


	name = "clk_ipf_psam";
	pr_err("\n");
    clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ipf_psam", 0x0, SOC_CRGPERIPH_PEREN0_gt_clk_ipf_psam_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ipf_psam", 0x0, SOC_CRGPERIPH_PEREN0_gt_clk_ipf_psam_START, CLK_OFF);


	name = "clk_slimbus";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_slimbus", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_slimbus_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_slimbus_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_slimbus_START, CLK_ON);

	clock_setrate_emulator(name, 1600000000);
    clkdiv_offsetbit_check("clk_slimbus_div", 0xD8, 1, SOC_CRGPERIPH_CLKDIV12_div_slimbus_START, SOC_CRGPERIPH_CLKDIV12_div_slimbus_END);
    clkmux_offsetbit_check("clk_slimbus_mux", 0xD8, SOC_CRGPERIPH_CLKDIV12_sel_slimbus_pll_START, 0x0);


    clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clk_slimbus_div", 0xD8, 1, SOC_CRGPERIPH_CLKDIV12_div_slimbus_START, SOC_CRGPERIPH_CLKDIV12_div_slimbus_END);
    clkmux_offsetbit_check("clk_slimbus_mux", 0xD8, SOC_CRGPERIPH_CLKDIV12_sel_slimbus_pll_START, 0x1);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("clk_slimbus_div", 0xD8, 64, SOC_CRGPERIPH_CLKDIV12_div_slimbus_START, SOC_CRGPERIPH_CLKDIV12_div_slimbus_END);
    clkmux_offsetbit_check("clk_slimbus_mux", 0xD8, SOC_CRGPERIPH_CLKDIV12_sel_slimbus_pll_START, 0x2);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clk_slimbus_div", 0xD8, 1, SOC_CRGPERIPH_CLKDIV12_div_slimbus_START, SOC_CRGPERIPH_CLKDIV12_div_slimbus_END);
    clkmux_offsetbit_check("clk_slimbus_mux", 0xD8, SOC_CRGPERIPH_CLKDIV12_sel_slimbus_pll_START, 0x2);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_slimbus", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_slimbus_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_slimbus_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_slimbus_START, CLK_OFF);

	name = "clk_emmc";
	pr_err("\n");
	clock_enable_emulator(name);
	clkgate_offsetbit_check("clk_emmc", 0x170, SOC_SCTRL_SCPEREN1_gt_clk_emmc_START, CLK_ON);
	clock_setrate_emulator(name, 3200000);
	clkmux_offsetbit_check("clk_emmc_mux", 0x264, SOC_SCTRL_SCCLKDIV5_sel_clk_emmc_START, 0x0);

	clock_setrate_emulator(name, 1622016000);
	clkmux_offsetbit_check("clk_emmc_mux", 0x264, SOC_SCTRL_SCCLKDIV5_sel_clk_emmc_START, 0x2);
	clkdiv_offsetbit_check("clk_emmc_div", 0x260, 1, SOC_SCTRL_SCCLKDIV4_div_emmc_pll_START, SOC_SCTRL_SCCLKDIV4_div_emmc_pll_END);
	clkmaskgate_offsetbit_check("clk_emmc_gt", 0x260, SOC_SCTRL_SCCLKDIV4_sc_gt_clk_emmc_pll_START, CLK_ON);

	clock_setrate_emulator(name, 101376000);
	clkmux_offsetbit_check("clk_emmc_mux", 0x264, SOC_SCTRL_SCCLKDIV5_sel_clk_emmc_START, 0x2);
	clkdiv_offsetbit_check("clk_emmc_div", 0x260, 16, SOC_SCTRL_SCCLKDIV4_div_emmc_pll_START, SOC_SCTRL_SCCLKDIV4_div_emmc_pll_END);
	clkmaskgate_offsetbit_check("clk_emmc_gt", 0x260, SOC_SCTRL_SCCLKDIV4_sc_gt_clk_emmc_pll_START, CLK_ON);


	clock_setrate_emulator(name, 1200000000);
	clkmux_offsetbit_check("clk_emmc_mux", 0x264, SOC_SCTRL_SCCLKDIV5_sel_clk_emmc_START, 0x1);
	clkgate_offsetbit_check("clk_ao_emmc", 0x00, SOC_CRGPERIPH_PEREN0_gt_clk_ao_emmc_START, CLK_ON);
	clkmaskgate_offsetbit_check("clk_ao_emmc_gt", 0xf0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_ao_emmc_pll_START, CLK_ON);
	clkmux_offsetbit_check("clk_ao_emmc_mux", 0xb4, SOC_CRGPERIPH_CLKDIV3_sel_ao_emmc_pll_START, 0x1);
	 clkdiv_offsetbit_check("clk_ao_emmc_div", 0xb4, 1, SOC_CRGPERIPH_CLKDIV3_div_ao_emmc_START, SOC_CRGPERIPH_CLKDIV3_div_ao_emmc_END);

	clock_setrate_emulator(name, 60000000);
	clkmux_offsetbit_check("clk_emmc_mux", 0x264, SOC_SCTRL_SCCLKDIV5_sel_clk_emmc_START, 0x1);
	clkgate_offsetbit_check("clk_ao_emmc", 0x00, SOC_CRGPERIPH_PEREN0_gt_clk_ao_emmc_START, CLK_ON);
	clkmaskgate_offsetbit_check("clk_ao_emmc_gt", 0xf0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_ao_emmc_pll_START, CLK_ON);
	clkmux_offsetbit_check("clk_ao_emmc_mux", 0xb4, SOC_CRGPERIPH_CLKDIV3_div_ao_emmc_START, 0x0);
	 clkdiv_offsetbit_check("clk_ao_emmc_div", 0xb4, 16, SOC_CRGPERIPH_CLKDIV3_div_ao_emmc_START, SOC_CRGPERIPH_CLKDIV3_div_ao_emmc_END);

		clock_disable_emulator(name);
	clkgate_offsetbit_check("clk_emmc", 0x170, SOC_SCTRL_SCPEREN1_gt_clk_emmc_START, CLK_OFF);

	name = "clk_sd";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_sd", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_sd_START, CLK_ON);

	clock_setrate_emulator(name, 960000000);
    clkmux_offsetbit_check("clk_sd_muxsys", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sd_div", 0xB8, 1, SOC_CRGPERIPH_CLKDIV4_div_sd_START, SOC_CRGPERIPH_CLKDIV4_div_sd_END);
    clkmaskgate_offsetbit_check("clk_sd_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_sd_START, CLK_ON);
    clkmux_offsetbit_check("clk_sd_muxpll", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_START, 0x2);

	clock_setrate_emulator(name, 320000000);
    clkmux_offsetbit_check("clk_sd_muxsys", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sd_div", 0xB8, 3, SOC_CRGPERIPH_CLKDIV4_div_sd_START, SOC_CRGPERIPH_CLKDIV4_div_sd_END);
    clkmux_offsetbit_check("clk_sd_muxpll", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_START, 0x2);

	clock_setrate_emulator(name, 1622016000);
    clkmux_offsetbit_check("clk_sd_muxsys", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sd_div", 0xB8, 1, SOC_CRGPERIPH_CLKDIV4_div_sd_START, SOC_CRGPERIPH_CLKDIV4_div_sd_END);
    clkmux_offsetbit_check("clk_sd_muxpll", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_START, 0x0);

    clock_setrate_emulator(name, 1200000000);
    clkmux_offsetbit_check("clk_sd_muxsys", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sd_div", 0xB8, 1, SOC_CRGPERIPH_CLKDIV4_div_sd_START, SOC_CRGPERIPH_CLKDIV4_div_sd_END);
    clkmux_offsetbit_check("clk_sd_muxpll", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_START, 0x1);

    clock_setrate_emulator(name, 75000000);
    clkmux_offsetbit_check("clk_sd_muxsys", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sd_div", 0xB8, 16, SOC_CRGPERIPH_CLKDIV4_div_sd_START, SOC_CRGPERIPH_CLKDIV4_div_sd_END);
    clkmux_offsetbit_check("clk_sd_muxpll", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_START, 0x1);

    clock_setrate_emulator(name, 3200000);
    clkmux_offsetbit_check("clk_sd_muxsys", 0xB8, SOC_CRGPERIPH_CLKDIV4_sel_sd_pll_END, 0x0);
    clkmaskgate_offsetbit_check("clk_sd_sys_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_sd_sys_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_sd", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_sd_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_sd_sys_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_sd_sys_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_sd_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_sd_START, CLK_OFF);

	name = "hclk_sd";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("hclk_sd", 0x0, SOC_CRGPERIPH_PEREN0_gt_hclk_sd_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("hclk_sd", 0x0, SOC_CRGPERIPH_PEREN0_gt_hclk_sd_START, CLK_OFF);

	name = "clk_sdio";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_sdio", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_sdio_START, CLK_ON);

	clock_setrate_emulator(name, 3200000);
    clkmux_offsetbit_check("clk_sdio_muxsy", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_END, 0x0);
    clkmaskgate_offsetbit_check("clk_sdio_sys_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_sdio1_sys_START, CLK_ON);

	clock_setrate_emulator(name, 1622016000);
    clkmux_offsetbit_check("clk_sdio_muxsy", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sdio_div", 0xC0, 1, SOC_CRGPERIPH_CLKDIV6_div_sdio1_START, SOC_CRGPERIPH_CLKDIV6_div_sdio1_END);
    clkmaskgate_offsetbit_check("clk_sdio_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_sdio1_START, CLK_ON);
    clkmux_offsetbit_check("clk_sdio_muxpl", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_START, 0x0);

    clock_setrate_emulator(name, 101376000);
    clkmux_offsetbit_check("clk_sdio_muxsy", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sdio_div", 0xC0, 16, SOC_CRGPERIPH_CLKDIV6_div_sdio1_START, SOC_CRGPERIPH_CLKDIV6_div_sdio1_END);
    clkmux_offsetbit_check("clk_sdio_muxpl", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_START, 0x0);

    clock_setrate_emulator(name, 1200000000);
    clkmux_offsetbit_check("clk_sdio_muxsy", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sdio_div", 0xC0, 1, SOC_CRGPERIPH_CLKDIV6_div_sdio1_START, SOC_CRGPERIPH_CLKDIV6_div_sdio1_END);
    clkmux_offsetbit_check("clk_sdio_muxpl", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_START, 0x1);

    clock_setrate_emulator(name, 75000000);
    clkmux_offsetbit_check("clk_sdio_muxsy", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sdio_div", 0xC0, 16, SOC_CRGPERIPH_CLKDIV6_div_sdio1_START, SOC_CRGPERIPH_CLKDIV6_div_sdio1_END);
    clkmux_offsetbit_check("clk_sdio_muxpl", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_START, 0x1);

    clock_setrate_emulator(name, 960000000);
    clkmux_offsetbit_check("clk_sdio_muxsy", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sdio_div", 0xC0, 1, SOC_CRGPERIPH_CLKDIV6_div_sdio1_START, SOC_CRGPERIPH_CLKDIV6_div_sdio1_END);
    clkmux_offsetbit_check("clk_sdio_muxpl", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_START, 0x2);

    clock_setrate_emulator(name, 96000000);
    clkmux_offsetbit_check("clk_sdio_muxsy", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_END, 0x1);
    clkdiv_offsetbit_check("clk_sdio_div", 0xC0, 10, SOC_CRGPERIPH_CLKDIV6_div_sdio1_START, SOC_CRGPERIPH_CLKDIV6_div_sdio1_END);
    clkmux_offsetbit_check("clk_sdio_muxpl", 0xC0, SOC_CRGPERIPH_CLKDIV6_sel_sdio1_pll_START, 0x2);

    clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_sdio", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_sdio_START, CLK_OFF);

    name = "clk_gpuhpm";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_gpuhpm", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_gpuhpm_START, CLK_ON);

	clock_setrate_emulator(name, 1660000000);
	clock_setrate_emulator(name, 480000000);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_gpuhpm", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_gpuhpm_START, CLK_OFF);

	name = "clk_perihpm";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_perihpm", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_perihpm_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_perihpm", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_perihpm_START, CLK_OFF);

	name = "clk_aohpm";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_perihpm", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_aohpm_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_perihpm", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_aohpm_START, CLK_OFF);

	name = "clk_uart1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_uart1", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart1_START, CLK_ON);

	clock_setrate_emulator(name, 324403200);
    clkmux_offsetbit_check("clkmux_uarth", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uarth_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uarth", 0xB0, 1, SOC_CRGPERIPH_CLKDIV2_div_uarth_START, SOC_CRGPERIPH_CLKDIV2_div_uarth_END);
    clkmaskgate_offsetbit_check("clkgt_uarth", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uarth_START, CLK_ON);

	clock_setrate_emulator(name, 20275200);
    clkmux_offsetbit_check("clkmux_uarth", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uarth_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uarth", 0xB0, 16, SOC_CRGPERIPH_CLKDIV2_div_uarth_START, SOC_CRGPERIPH_CLKDIV2_div_uarth_END);
    clkmaskgate_offsetbit_check("clkgt_uarth", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uarth_START, CLK_ON);

    clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clkmux_uarth", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uarth_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_uart1", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart1_START, CLK_OFF);


	name = "clk_uart4";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_uart4", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart4_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_uart4", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart4_START, CLK_OFF);

	name = "pclk_uart1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_uart1", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart1_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_uart1", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart1_START, CLK_OFF);

	name = "pclk_uart4";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_uart4", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart4_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_uart4", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart4_START, CLK_OFF);

	name = "clk_uart2";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_uart2", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart2_START, CLK_ON);

	clock_setrate_emulator(name, 324403200);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uartl", 0xB0, 1, SOC_CRGPERIPH_CLKDIV2_div_uartl_START, SOC_CRGPERIPH_CLKDIV2_div_uartl_END);
    clkmaskgate_offsetbit_check("clkgt_uartl", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uartl_START, CLK_ON);

	clock_setrate_emulator(name, 20275200);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uartl", 0xB0, 16, SOC_CRGPERIPH_CLKDIV2_div_uartl_START, SOC_CRGPERIPH_CLKDIV2_div_uartl_END);
    clkmaskgate_offsetbit_check("clkgt_uartl", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uartl_START, CLK_ON);

    clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_uart2", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart2_START, CLK_OFF);

	name = "clk_uart5";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_uart5", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart5_START, CLK_ON);

	clock_setrate_emulator(name, 324403200);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uartl", 0xB0, 1, SOC_CRGPERIPH_CLKDIV2_div_uartl_START, SOC_CRGPERIPH_CLKDIV2_div_uartl_END);
    clkmaskgate_offsetbit_check("clkgt_uartl", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uartl_START, CLK_ON);

	clock_setrate_emulator(name, 20275200);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uartl", 0xB0, 16, SOC_CRGPERIPH_CLKDIV2_div_uartl_START, SOC_CRGPERIPH_CLKDIV2_div_uartl_END);
    clkmaskgate_offsetbit_check("clkgt_uartl", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uartl_START, CLK_ON);

    clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_uart5", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart5_START, CLK_OFF);

	name = "pclk_uart2";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_uart2", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart2_START, CLK_ON);

	clock_setrate_emulator(name, 324403200);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uartl", 0xB0, 1, SOC_CRGPERIPH_CLKDIV2_div_uartl_START, SOC_CRGPERIPH_CLKDIV2_div_uartl_END);
    clkmaskgate_offsetbit_check("clkgt_uartl", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uartl_START, CLK_ON);

	clock_setrate_emulator(name, 20275200);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uartl", 0xB0, 16, SOC_CRGPERIPH_CLKDIV2_div_uartl_START, SOC_CRGPERIPH_CLKDIV2_div_uartl_END);
    clkmaskgate_offsetbit_check("clkgt_uartl", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uartl_START, CLK_ON);

    clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_uart2", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart2_START, CLK_OFF);

	name = "pclk_uart5";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_uart5", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart5_START, CLK_ON);

	clock_setrate_emulator(name, 324403200);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uartl", 0xB0, 1, SOC_CRGPERIPH_CLKDIV2_div_uartl_START, SOC_CRGPERIPH_CLKDIV2_div_uartl_END);
    clkmaskgate_offsetbit_check("clkgt_uartl", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uartl_START, CLK_ON);

	clock_setrate_emulator(name, 20275200);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uartl", 0xB0, 16, SOC_CRGPERIPH_CLKDIV2_div_uartl_START, SOC_CRGPERIPH_CLKDIV2_div_uartl_END);
    clkmaskgate_offsetbit_check("clkgt_uartl", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uartl_START, CLK_ON);

    clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clkmux_uartl", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uartl_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_uart5", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart5_START, CLK_OFF);

	name = "clk_uart0";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_uart0", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart0_START, CLK_ON);

	clock_setrate_emulator(name, 324403200);
    clkmux_offsetbit_check("clkmux_uart0", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uart0_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uart0", 0xB0, 1, SOC_CRGPERIPH_CLKDIV2_div_uart0_START, SOC_CRGPERIPH_CLKDIV2_div_uart0_END);
    clkmaskgate_offsetbit_check("clkgt_uart0", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uart0_START, CLK_ON);

	clock_setrate_emulator(name, 20275200);
    clkmux_offsetbit_check("clkmux_uart0", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uart0_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_uart0", 0xB0, 16, SOC_CRGPERIPH_CLKDIV2_div_uart0_START, SOC_CRGPERIPH_CLKDIV2_div_uart0_END);
    clkmaskgate_offsetbit_check("clkgt_uart0", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_uart0_START, CLK_ON);

    clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clkmux_uart0", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_uart0_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_uart0", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_uart0_START, CLK_OFF);

	name = "pclk_uart0";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_i2c3";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_i2c3", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_i2c3_START, CLK_ON);

	clock_setrate_emulator(name, 324403200);
    clkmux_offsetbit_check("clkmux_i2c", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_i2c_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_i2c", 0xE8, 1, SOC_CRGPERIPH_CLKDIV16_div_i2c_START, SOC_CRGPERIPH_CLKDIV16_div_i2c_END);

	clock_setrate_emulator(name, 20275200);
    clkmux_offsetbit_check("clkmux_i2c", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_i2c_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_i2c", 0xE8, 16, SOC_CRGPERIPH_CLKDIV16_div_i2c_START, SOC_CRGPERIPH_CLKDIV16_div_i2c_END);

    clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clkmux_i2c", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_i2c_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_i2c3", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_i2c3_START, CLK_OFF);

	name = "clk_i2c4";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_i2c4", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_i2c4_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_i2c4", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_i2c4_START, CLK_OFF);

	name = "clk_i2c7";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_i2c7", 0x10, SOC_CRGPERIPH_PEREN1_gt_clk_i2c7_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_i2c7", 0x10, SOC_CRGPERIPH_PEREN1_gt_clk_i2c7_START, CLK_OFF);

	name = "pclk_i2c3";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_i2c3", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_i2c3_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_i2c3", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_i2c3_START, CLK_OFF);

	name = "pclk_i2c4";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_i2c4", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_i2c4_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_i2c4", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_i2c4_START, CLK_OFF);

	name = "pclk_i2c7";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_i2c7", 0x10, SOC_CRGPERIPH_PEREN1_gt_clk_i2c7_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_i2c7", 0x10, SOC_CRGPERIPH_PEREN1_gt_clk_i2c7_START, CLK_OFF);

	name = "clk_spi1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_spi1", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_spi1_START, CLK_ON);

	clock_setrate_emulator(name, 324403200);
    clkmux_offsetbit_check("clkmux_spi", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_spi_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_spi", 0xC4, 1, SOC_CRGPERIPH_CLKDIV7_div_spi_START, SOC_CRGPERIPH_CLKDIV7_div_spi_END);
    clkmaskgate_offsetbit_check("clkgt_spi", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_spi_START, CLK_ON);

	clock_setrate_emulator(name, 20275200);
    clkmux_offsetbit_check("clkmux_spi", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_spi_START, 0x1);
    clkdiv_offsetbit_check("clkdiv_spi", 0xC4, 16, SOC_CRGPERIPH_CLKDIV7_div_spi_START, SOC_CRGPERIPH_CLKDIV7_div_spi_END);
    clkmaskgate_offsetbit_check("clkgt_spi", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_spi_START, CLK_ON);

    clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clkmux_spi", 0xAC, SOC_CRGPERIPH_CLKDIV1_sel_spi_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_spi1", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_spi1_START, CLK_OFF);

	name = "clk_spi4";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_spi4", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_spi4_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_spi4", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_spi4_START, CLK_OFF);

	name = "pclk_spi1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_spi1", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_spi1_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_spi1", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_spi1_START, CLK_OFF);


	name = "pclk_spi4";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_spi4", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_spi4_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_spi4", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_spi4_START, CLK_OFF);

	name = "clk_usb2otg_ref";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_usb2otg_ref", 0x00, SOC_CRGPERIPH_PEREN0_gt_clk_usb2otg_ref_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_usb2otg_ref", 0x00, SOC_CRGPERIPH_PEREN0_gt_clk_usb2otg_ref_START, CLK_OFF);

/*
	 name = "clk_abb_192";
	 pr_err("\n");
	 clock_enable_emulator(name);
       clkgate_offsetbit_check("clk_abb_192", 0x37, PMIC_CLK_ABB_EN_reg_xo_abb_en_START, CLK_ON);
	 clock_disable_emulator(name);
       clkgate_offsetbit_check("clk_abb_192", 0x37, PMIC_CLK_ABB_EN_reg_xo_abb_en_START, CLK_OFF);

	 name = "clk_usb_tcxo_en";
	 pr_err("\n");
	 clock_enable_emulator(name);
     clkgate_offsetbit_check("clk_usb_tcxo_en", 0x10, SOC_PCTRL_PERI_CTRL3_usb_tcxo_en_START, CLK_ON);
	 clock_disable_emulator(name);
     clkgate_offsetbit_check("clk_usb_tcxo_en", 0x10, SOC_PCTRL_PERI_CTRL3_usb_tcxo_en_START, CLK_OFF);


	name = "clk_ufs_tcxo_en";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ufs_tcxo_en", 0x10, SOC_PCTRL_PERI_CTRL3_ufs_tcxo_en_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ufs_tcxo_en", 0x10, SOC_PCTRL_PERI_CTRL3_ufs_tcxo_en_START, CLK_OFF);
 */

	name = "clk_ufsio_ref";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ufsio_ref", 0x1B0, SOC_SCTRL_SCPEREN4_gt_clk_ufsio_ref_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ufsio_ref", 0x1B0, SOC_SCTRL_SCPEREN4_gt_clk_ufsio_ref_START, CLK_OFF);

	name = "clk_ufs_subsys";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ufsio_ref", 0x1B0, SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START, CLK_ON);
	clock_setrate_emulator(name, 300000);
	clkdiv_offsetbit_check("clk_ufssub_div", 0x274, 64, SOC_SCTRL_SCCLKDIV9_div_ufs_subsys_pll_START, SOC_SCTRL_SCCLKDIV9_div_ufs_subsys_pll_END);
	clkmux_offsetbit_check("clk_ufssub_mux", 0x274, SOC_SCTRL_SCCLKDIV9_sel_ufs_subsys_pll_START, 0x0);

	clock_setrate_emulator(name, 1622016000);
	clkdiv_offsetbit_check("clk_ufssub_div", 0x274, 1, SOC_SCTRL_SCCLKDIV9_div_ufs_subsys_pll_START, SOC_SCTRL_SCCLKDIV9_div_ufs_subsys_pll_END);
	clkmux_offsetbit_check("clk_ufssub_mux", 0x274, SOC_SCTRL_SCCLKDIV9_sel_ufs_subsys_pll_START, 0x1);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ufsio_ref", 0x1B0, SOC_SCTRL_SCPEREN4_gt_clk_ufs_subsys_START, CLK_OFF);


	name = "clk_ao_asp";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ao_asp", 0x0, SOC_CRGPERIPH_PEREN0_gt_clk_ao_asp_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_ao_asp_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_ao_asp_pll_START, CLK_ON);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clk_ao_asp_div", 0x108, 1, SOC_CRGPERIPH_CLKDIV24_div_ao_asp_START, SOC_CRGPERIPH_CLKDIV24_div_ao_asp_END);
    clkmux_offsetbit_check("clkmux_ao_asp", 0x100, SOC_CRGPERIPH_CLKDIV22_sel_ao_asp_pll_START, 0x1);
	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clk_ao_asp_div", 0x108, 1, SOC_CRGPERIPH_CLKDIV24_div_ao_asp_START, SOC_CRGPERIPH_CLKDIV24_div_ao_asp_END);
    clkmux_offsetbit_check("clkmux_ao_asp", 0x100, SOC_CRGPERIPH_CLKDIV22_sel_ao_asp_pll_START, 0x0);
	clock_setrate_emulator(name, 60000000);
    clkdiv_offsetbit_check("clk_ao_asp_div", 0x108, 16, SOC_CRGPERIPH_CLKDIV24_div_ao_asp_START, SOC_CRGPERIPH_CLKDIV24_div_ao_asp_END);
    clkmux_offsetbit_check("clkmux_ao_asp", 0x100, SOC_CRGPERIPH_CLKDIV22_sel_ao_asp_pll_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ao_asp", 0x0, SOC_CRGPERIPH_PEREN0_gt_clk_ao_asp_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_ao_asp_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_ao_asp_pll_START, CLK_OFF);


	name = "pclk_rtc";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_rtc", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_rtc_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_rtc", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_rtc_START, CLK_OFF);


	name = "pclk_rtc1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_rtc1", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_rtc1_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_rtc1", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_rtc1_START, CLK_OFF);


	name = "pclk_gpio0";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio0", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio0_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio0", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio0_START, CLK_OFF);

	name = "pclk_gpio1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio1", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio1_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio1", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio1_START, CLK_OFF);

	name = "pclk_gpio2";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio2", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio2_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio2", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio2_START, CLK_OFF);

	name = "pclk_gpio3";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio3", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio3_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio3", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio3_START, CLK_OFF);

	name = "pclk_gpio4";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio4", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio4_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio4", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio4_START, CLK_OFF);

	name = "pclk_gpio5";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio5", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio5_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio5", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio5_START, CLK_OFF);

    name = "pclk_gpio6";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio6", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio6_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio6", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio6_START, CLK_OFF);

	name = "pclk_gpio7";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio7", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio7_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio7", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio7_START, CLK_OFF);

	name = "pclk_gpio8";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio8", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio8_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio8", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio8_START, CLK_OFF);

	name = "pclk_gpio9";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio9", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio9_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio9", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio9_START, CLK_OFF);

	name = "pclk_gpio10";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio10", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio10_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio10", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio10_START, CLK_OFF);

	name = "pclk_gpio11";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio11", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio11_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio11", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio11_START, CLK_OFF);

	name = "pclk_gpio12";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio12", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio12_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio12", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio12_START, CLK_OFF);

	name = "pclk_gpio13";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio13", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio13_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio13", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio13_START, CLK_OFF);

	name = "pclk_gpio14";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio14", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio14_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio14", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio14_START, CLK_OFF);

	name = "pclk_gpio15";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio15", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio15_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio15", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio15_START, CLK_OFF);

	name = "pclk_gpio16";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio16", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio16_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio16", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio16_START, CLK_OFF);

	name = "pclk_gpio17";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio17", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio17_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio17", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio17_START, CLK_OFF);

	name = "pclk_gpio18";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio18", 0x1B0, SOC_SCTRL_SCPEREN4_gt_pclk_gpio18_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio18", 0x1B0, SOC_SCTRL_SCPEREN4_gt_pclk_gpio18_START, CLK_OFF);

	name = "pclk_gpio19";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio19", 0x1B0, SOC_SCTRL_SCPEREN4_gt_pclk_gpio19_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio19", 0x1B0, SOC_SCTRL_SCPEREN4_gt_pclk_gpio19_START, CLK_OFF);

	name = "pclk_gpio20";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio20", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio20_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio20", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio20_START, CLK_OFF);

	name = "pclk_gpio21";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio21", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio21_START, CLK_ON);
	clock_setrate_emulator(name, 232000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 1, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_setrate_emulator(name, 58000000);
    clkdiv_offsetbit_check("sc_div_cfgbus", 0xEC, 4, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_START, SOC_CRGPERIPH_CLKDIV17_div_cfgbus_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_gpio21", 0x10, SOC_CRGPERIPH_PEREN1_gt_pclk_gpio21_START, CLK_OFF);

	name = "pclk_ao_gpio0";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio0", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio0_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio0", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio0_START, CLK_OFF);

	name = "pclk_ao_gpio1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio1", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio1_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio1", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio1_START, CLK_OFF);

	name = "pclk_ao_gpio2";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio2", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio2_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio2", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio2_START, CLK_OFF);

	name = "pclk_ao_gpio3";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio3", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio3_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio3", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio3_START, CLK_OFF);

	name = "pclk_ao_gpio4";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio4", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio4_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio4", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio4_START, CLK_OFF);

	name = "pclk_ao_gpio5";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio5", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio5_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio5", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio5_START, CLK_OFF);

    name = "pclk_ao_gpio6";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio6", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio6_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_gpio6", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_ao_gpio6_START, CLK_OFF);

	name = "clk_blpwm";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "pclk_pctrl";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_pctrl", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_pctrl_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_ptp_gt", 0xF8, SOC_CRGPERIPH_CLKDIV20_sc_gt_clk_ptp_START, CLK_ON);
	clock_setrate_emulator(name, 324403200);
    clkdiv_offsetbit_check("clk_ptp_div", 0xD8, 1, SOC_CRGPERIPH_CLKDIV12_div_ptp_START, SOC_CRGPERIPH_CLKDIV12_div_ptp_END);
	clock_setrate_emulator(name, 20275200);
    clkdiv_offsetbit_check("clk_ptp_div", 0xD8, 16, SOC_CRGPERIPH_CLKDIV12_div_ptp_START, SOC_CRGPERIPH_CLKDIV12_div_ptp_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_pctrl", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_pctrl_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_ptp_gt", 0xF8, SOC_CRGPERIPH_CLKDIV20_sc_gt_clk_ptp_START, CLK_ON);

	name = "clk_pwm";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_pwm", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_pwm_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_ptp_gt", 0xF8, SOC_CRGPERIPH_CLKDIV20_sc_gt_clk_ptp_START, CLK_ON);
	clock_setrate_emulator(name, 324403200);
    clkdiv_offsetbit_check("clk_ptp_div", 0xD8, 1, SOC_CRGPERIPH_CLKDIV12_div_ptp_START, SOC_CRGPERIPH_CLKDIV12_div_ptp_END);
	clock_setrate_emulator(name, 20275200);
    clkdiv_offsetbit_check("clk_ptp_div", 0xD8, 16, SOC_CRGPERIPH_CLKDIV12_div_ptp_START, SOC_CRGPERIPH_CLKDIV12_div_ptp_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_pwm", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_pwm_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_ptp_gt", 0xF8, SOC_CRGPERIPH_CLKDIV20_sc_gt_clk_ptp_START, CLK_ON);

	name = "clk_out0";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_out0", 0x160, SOC_SCTRL_SCPEREN0_gt_clk_out0_START, CLK_ON);

	clock_setrate_emulator(name, 19200000);
    clkdiv_offsetbit_check("clkdiv_out0tcxo", 0x254, 1, SOC_SCTRL_SCCLKDIV1_div_clkout0_tcxo_START, SOC_SCTRL_SCCLKDIV1_div_clkout0_tcxo_END);
    clkmux_offsetbit_check("clkmux_clkout0", 0x254, SOC_SCTRL_SCCLKDIV1_sel_clk_out0_START, 0x1);
	clock_setrate_emulator(name, 2400000);
    clkdiv_offsetbit_check("clkdiv_out0tcxo", 0x254, 8, SOC_SCTRL_SCCLKDIV1_div_clkout0_tcxo_START, SOC_SCTRL_SCCLKDIV1_div_clkout0_tcxo_END);
    clkmux_offsetbit_check("clkmux_clkout0", 0x254, SOC_SCTRL_SCCLKDIV1_sel_clk_out0_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clkdiv_out0_pll", 0xe0, 1, SOC_CRGPERIPH_CLKDIV14_div_clkout0_pll_START, SOC_CRGPERIPH_CLKDIV14_div_clkout0_pll_END);
    clkmux_offsetbit_check("clkmux_clkout0", 0x254, SOC_SCTRL_SCCLKDIV1_sel_clk_out0_START, 0x2);
    clkmaskgate_offsetbit_check("clkgt_out0", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_out0_START, CLK_ON);
	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("clkdiv_out0_pll", 0xe0, 64, SOC_CRGPERIPH_CLKDIV14_div_clkout0_pll_START, SOC_CRGPERIPH_CLKDIV14_div_clkout0_pll_END);
    clkmux_offsetbit_check("clkmux_clkout0", 0x254, SOC_SCTRL_SCCLKDIV1_sel_clk_out0_START, 0x2);
    clkmaskgate_offsetbit_check("clkgt_out0", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_out0_START, CLK_ON);

	clock_setrate_emulator(name, 32764);
    clkmux_offsetbit_check("clkmux_clkout0", 0x254, SOC_SCTRL_SCCLKDIV1_sel_clk_out0_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_out0", 0x160, SOC_SCTRL_SCPEREN0_gt_clk_out0_START, CLK_OFF);


	name = "clk_out1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_out1", 0x160, SOC_SCTRL_SCPEREN0_gt_clk_out1_START, CLK_ON);

	clock_setrate_emulator(name, 19200000);
    clkdiv_offsetbit_check("clkdiv_out1tcxo", 0x254, 1, SOC_SCTRL_SCCLKDIV1_div_clkout1_tcxo_START, SOC_SCTRL_SCCLKDIV1_div_clkout1_tcxo_END);
    clkmux_offsetbit_check("clkmux_clkout1", 0x254, SOC_SCTRL_SCCLKDIV1_sel_clk_out1_START, 0x1);
	clock_setrate_emulator(name, 2400000);
    clkdiv_offsetbit_check("clkdiv_out1tcxo", 0x254, 8, SOC_SCTRL_SCCLKDIV1_div_clkout1_tcxo_START, SOC_SCTRL_SCCLKDIV1_div_clkout1_tcxo_END);
    clkmux_offsetbit_check("clkmux_clkout1", 0x254, SOC_SCTRL_SCCLKDIV1_sel_clk_out1_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clkdiv_out1_pll", 0xe0, 1, SOC_CRGPERIPH_CLKDIV14_div_clkout1_pll_START, SOC_CRGPERIPH_CLKDIV14_div_clkout1_pll_END);
    clkmux_offsetbit_check("clkmux_clkout1", 0x254, SOC_SCTRL_SCCLKDIV1_sel_clk_out1_START, 0x2);
    clkmaskgate_offsetbit_check("clkgt_out1", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_out1_START, CLK_ON);
	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("clkdiv_out1_pll", 0xe0, 64, SOC_CRGPERIPH_CLKDIV14_div_clkout1_pll_START, SOC_CRGPERIPH_CLKDIV14_div_clkout1_pll_END);
    clkmux_offsetbit_check("clkmux_clkout1", 0x254, SOC_SCTRL_SCCLKDIV1_sel_clk_out1_START, 0x2);
    clkmaskgate_offsetbit_check("clkgt_out1", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_out1_START, CLK_ON);

	clock_setrate_emulator(name, 32764);
    clkmux_offsetbit_check("clkmux_clkout1", 0x254, SOC_SCTRL_SCCLKDIV1_sel_clk_out1_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_out1", 0x160, SOC_SCTRL_SCPEREN0_gt_clk_out1_START, CLK_OFF);

	name = "pclk_syscnt";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_syscnt", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_syscnt_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_syscnt", 0x160, SOC_SCTRL_SCPEREN0_gt_pclk_syscnt_START, CLK_OFF);


	name = "clk_syscnt";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_syscnt", 0x160, SOC_SCTRL_SCPEREN0_gt_clk_syscnt_START, CLK_ON);
	clock_setrate_emulator(name, 19200000);
	clkdiv_offsetbit_check("clk_div_syscnt", 0x268, 1, SOC_SCTRL_SCCLKDIV6_div_syscnt_START, SOC_SCTRL_SCCLKDIV6_div_syscnt_END);
	clock_setrate_emulator(name, 1200000);
	clkdiv_offsetbit_check("clk_div_syscnt", 0x268, 16, SOC_SCTRL_SCCLKDIV6_div_syscnt_START, SOC_SCTRL_SCCLKDIV6_div_syscnt_END);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_syscnt", 0x160, SOC_SCTRL_SCPEREN0_gt_clk_syscnt_START, CLK_ON);

	name = "clk_mdm2gps0";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_mdm2gps1";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_mdm2gps2";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_mdm2gps0_en";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_mdm2gps1_en";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_mdm2gps2_en";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_asp_subsys";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_asp_subsys", 0x170, SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_acpu_START, CLK_ON);
	clock_setrate_emulator(name, 1622016000);
	clkmux_offsetbit_check("clk_asp_pll_sel", 0x268, SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_START, 0x0);

	clock_setrate_emulator(name, 19200000);
	clkmux_offsetbit_check("clk_asp_pll_sel", 0x268, SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_START, 0x3);

	clock_setrate_emulator(name, 134400000);
	clkmux_offsetbit_check("clk_asp_pll_sel", 0x268, SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_START, 0x1);

	clock_setrate_emulator(name, 960000000);
	clkmux_offsetbit_check("clk_asp_pll_sel", 0x268, SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_START, 0x2);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_asp_subsys", 0x170, SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_acpu_START, CLK_OFF);

		name = "clk_asp_subsys_peri";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_asp_subsys_peri", 0x170, SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_peri_acpu_START, CLK_ON);
	clock_setrate_emulator(name, 1622016000);
	clkmux_offsetbit_check("clk_asp_pll_sel", 0x268, SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_peri_START, 0x0);
	clkdiv_offsetbit_check("clkdiv_asp_subsys_peri", 0x268, 1, SOC_SCTRL_SCCLKDIV6_div_asp_subsys_peri_START, SOC_SCTRL_SCCLKDIV6_div_asp_subsys_peri_END);
	clkmaskgate_offsetbit_check("clkgt_asp_subsys_peri", 0x268, SOC_SCTRL_SCCLKDIV6_sc_gt_clk_asp_subsys_peri_START, CLK_ON);

	clock_setrate_emulator(name, 202752000);
	clkmux_offsetbit_check("clk_asp_pll_sel", 0x268, SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_peri_START, 0x0);
	clkdiv_offsetbit_check("clkdiv_asp_subsys_peri", 0x268, 8, SOC_SCTRL_SCCLKDIV6_div_asp_subsys_peri_START, SOC_SCTRL_SCCLKDIV6_div_asp_subsys_peri_END);

	clock_setrate_emulator(name, 19200000);
	clkmux_offsetbit_check("clk_asp_pll_sel", 0x268, SOC_SCTRL_SCCLKDIV6_sel_asp_subsys_peri_START, 0x1);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_asp_subsys_peri", 0x170, SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_peri_acpu_START, CLK_OFF);
	    clkmaskgate_offsetbit_check("clkgt_asp_subsys_peri", 0x268, SOC_SCTRL_SCCLKDIV6_sc_gt_clk_asp_subsys_peri_START, CLK_OFF);


	name = "clk_asp_tcxo";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_asp_tcxo", 0x160, SOC_SCTRL_SCPEREN0_gt_clk_asp_tcxo_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_asp_tcxo", 0x160, SOC_SCTRL_SCPEREN0_gt_clk_asp_tcxo_START, CLK_OFF);
/*
	name = "abb_audio_gt0";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("abb_audio_gt0", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_audio_gt_en0_START, CLK_ON);
    clkgate_offsetbit_check("abb_audio_en0", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_audio_en0_START, CLK_ON);
    clkgate_offsetbit_check("clk_abb_192", 0x37, PMIC_CLK_ABB_EN_reg_xo_abb_en_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("abb_audio_gt0", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_audio_gt_en0_START, CLK_OFF);
    clkgate_offsetbit_check("abb_audio_en0", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_audio_en0_START, CLK_OFF);

	name = "abb_audio_gt1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("abb_audio_gt1", 0x40, SOC_CRGPERIPH_PEREN4_sc_abb_pll_audio_gt_en1_START, CLK_ON);
    clkgate_offsetbit_check("abb_audio_en1", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_audio_en1_START, CLK_ON);
    clkgate_offsetbit_check("clk_abb_192", 0x37, PMIC_CLK_ABB_EN_reg_xo_abb_en_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("abb_audio_gt1", 0x40, SOC_CRGPERIPH_PEREN4_sc_abb_pll_audio_gt_en1_START, CLK_OFF);
    clkgate_offsetbit_check("abb_audio_en1", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_audio_en1_START, CLK_OFF);
 */
	name = "peri_volt_hold";
	pr_err("\n");
	clock_enable_emulator(name);
	clock_disable_emulator(name);

	name = "clk_isp_snclk0";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_isp_snclk0", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_isp_snclk0_START, CLK_ON);

	clock_setrate_emulator(name, 48000000);
    clkdiv_offsetbit_check("clk_div_ispsn0", 0x108, 1, SOC_CRGPERIPH_CLKDIV24_div_isp_snclk0_START, SOC_CRGPERIPH_CLKDIV24_div_isp_snclk0_END);
    clkmux_offsetbit_check("clk_mux_ispsn0", 0x108, SOC_CRGPERIPH_CLKDIV24_sel_isp_snclk0_START, 0x1);
    clkmaskgate_offsetbit_check("clk_ispsn_gt", 0x108, SOC_CRGPERIPH_CLKDIV24_sc_gt_clk_isp_snclk_START, CLK_ON);
	clock_setrate_emulator(name, 12000000);
    clkdiv_offsetbit_check("clk_div_ispsn0", 0x108, 4, SOC_CRGPERIPH_CLKDIV24_div_isp_snclk0_START, SOC_CRGPERIPH_CLKDIV24_div_isp_snclk0_END);

	clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clk_mux_ispsn0", 0x108, SOC_CRGPERIPH_CLKDIV24_sel_isp_snclk0_START, 0x0);
    clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_isp_snclk0", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_isp_snclk0_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_ispsn_gt", 0x108, SOC_CRGPERIPH_CLKDIV24_sc_gt_clk_isp_snclk_START, CLK_OFF);


	name = "clk_isp_snclk1";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_isp_snclk1", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_isp_snclk1_START, CLK_ON);

	clock_setrate_emulator(name, 48000000);
    clkdiv_offsetbit_check("clk_div_ispsn1", 0x10C, 1, SOC_CRGPERIPH_CLKDIV25_div_isp_snclk1_START, SOC_CRGPERIPH_CLKDIV25_div_isp_snclk1_END);
    clkmux_offsetbit_check("clk_mux_ispsn1", 0x10C, SOC_CRGPERIPH_CLKDIV25_sel_isp_snclk1_START, 0x1);
    clkmaskgate_offsetbit_check("clk_ispsn_gt", 0x108, SOC_CRGPERIPH_CLKDIV24_sc_gt_clk_isp_snclk_START, CLK_ON);
	clock_setrate_emulator(name, 12000000);
    clkdiv_offsetbit_check("clk_div_ispsn1", 0x10C, 4, SOC_CRGPERIPH_CLKDIV25_div_isp_snclk1_START, SOC_CRGPERIPH_CLKDIV25_div_isp_snclk1_END);

	clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clk_mux_ispsn1", 0x10C, SOC_CRGPERIPH_CLKDIV25_sel_isp_snclk1_START, 0x0);
    clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_isp_snclk1", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_isp_snclk1_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_ispsn_gt", 0x108, SOC_CRGPERIPH_CLKDIV24_sc_gt_clk_isp_snclk_START, CLK_OFF);

	name = "clk_isp_snclk2";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_isp_snclk2", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_isp_snclk2_START, CLK_ON);

	clock_setrate_emulator(name, 48000000);
    clkdiv_offsetbit_check("clk_div_ispsn2", 0x10C, 1, SOC_CRGPERIPH_CLKDIV25_div_isp_snclk2_START, SOC_CRGPERIPH_CLKDIV25_div_isp_snclk2_END);
    clkmux_offsetbit_check("clk_mux_ispsn2", 0x10C, SOC_CRGPERIPH_CLKDIV25_sel_isp_snclk2_START, 0x1);
    clkmaskgate_offsetbit_check("clk_ispsn_gt", 0x108, SOC_CRGPERIPH_CLKDIV24_sc_gt_clk_isp_snclk_START, CLK_ON);
	clock_setrate_emulator(name, 12000000);
    clkdiv_offsetbit_check("clk_div_ispsn2", 0x10C, 4, SOC_CRGPERIPH_CLKDIV25_div_isp_snclk2_START, SOC_CRGPERIPH_CLKDIV25_div_isp_snclk2_END);

	clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clk_mux_ispsn2", 0x10C, SOC_CRGPERIPH_CLKDIV25_sel_isp_snclk2_START, 0x0);
    clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_isp_snclk2", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_isp_snclk2_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_ispsn_gt", 0x108, SOC_CRGPERIPH_CLKDIV24_sc_gt_clk_isp_snclk_START, CLK_OFF);


	name = "clk_rxdphy0_cfg";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_rxdphy0_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_rxdphy0_cfg_START, CLK_ON);
	clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clk_rxdcfg_mux", 0xC4, SOC_CRGPERIPH_CLKDIV7_sel_rxdphy_cfg_START, 0x1);
	clock_setrate_emulator(name, 80000000);
    clkmux_offsetbit_check("clk_rxdcfg_mux", 0xC4, SOC_CRGPERIPH_CLKDIV7_sel_rxdphy_cfg_START, 0x0);
    clkmaskgate_offsetbit_check("clk_rxdcfg_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_rxdphy_cfg_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_rxdphy0_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_rxdphy0_cfg_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_rxdcfg_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_rxdphy_cfg_START, CLK_OFF);

	name = "clk_rxdphy1_cfg";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_rxdphy1_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_rxdphy1_cfg_START, CLK_ON);
	clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clk_rxdcfg_mux", 0xC4, SOC_CRGPERIPH_CLKDIV7_sel_rxdphy_cfg_START, 0x1);
	clock_setrate_emulator(name, 80000000);
    clkmux_offsetbit_check("clk_rxdcfg_mux", 0xC4, SOC_CRGPERIPH_CLKDIV7_sel_rxdphy_cfg_START, 0x0);
    clkmaskgate_offsetbit_check("clk_rxdcfg_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_rxdphy_cfg_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_rxdphy1_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_rxdphy1_cfg_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_rxdcfg_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_rxdphy_cfg_START, CLK_OFF);

	name = "clk_rxdphy2_cfg";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_rxdphy2_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_rxdphy2_cfg_START, CLK_ON);
	clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clk_rxdcfg_mux", 0xC4, SOC_CRGPERIPH_CLKDIV7_sel_rxdphy_cfg_START, 0x1);
	clock_setrate_emulator(name, 80000000);
    clkmux_offsetbit_check("clk_rxdcfg_mux", 0xC4, SOC_CRGPERIPH_CLKDIV7_sel_rxdphy_cfg_START, 0x0);
    clkmaskgate_offsetbit_check("clk_rxdcfg_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_rxdphy_cfg_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_rxdphy2_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_rxdphy2_cfg_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_rxdcfg_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_rxdphy_cfg_START, CLK_OFF);

	name = "clk_txdphy0_cfg";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_txdphy0_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_txdphy0_cfg_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_txdphy0_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_txdphy0_cfg_START, CLK_OFF);

	name = "clk_txdphy1_cfg";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_txdphy1_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_txdphy1_cfg_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_txdphy1_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_txdphy1_cfg_START, CLK_OFF);

	name = "clk_txdphy0_ref";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_txdphy0_ref", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_txdphy0_ref_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_txdphy0_ref", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_txdphy0_ref_START, CLK_OFF);


	name = "clk_txdphy1_ref";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_txdphy1_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_txdphy1_ref_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_txdphy1_cfg", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_txdphy1_ref_START, CLK_OFF);


	name = "clk_media_tcxo";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_media_tcxo", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_media_tcxo_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_media_tcxo", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_media_tcxo_START, CLK_OFF);

	name = "aclk_asc";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("aclk_asc", 0x20, SOC_MEDIA1_CRG_PEREN2_gt_aclk_asc_START, CLK_ON);
    clkgate_offsetbit_check("clk_mmbuf", 0x20, SOC_MEDIA1_CRG_PEREN2_gt_clk_mmbuf_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_mmbuf_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_aclk_mmbuf_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 1, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x1);

	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 64, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x1);

    clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 1, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x2);

	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 64, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x2);

    clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 1, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x4);

    clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 1, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x8);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 64, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("aclk_asc", 0x20, SOC_MEDIA1_CRG_PEREN2_gt_aclk_asc_START, CLK_OFF);
    clkgate_offsetbit_check("clk_mmbuf", 0x20, SOC_MEDIA1_CRG_PEREN2_gt_clk_mmbuf_START, CLK_OFF);

	name = "clk_dss_axi_mm";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_dss_axi_mm", 0x20, SOC_MEDIA1_CRG_PEREN2_gt_clk_dss_axi_mm_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_dss_axi_mm", 0x20, SOC_MEDIA1_CRG_PEREN2_gt_clk_dss_axi_mm_START, CLK_OFF);

	name = "pclk_mmbuf";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_mmbuf", 0x20, SOC_MEDIA1_CRG_PEREN2_gt_pclk_mmbuf_START, CLK_ON);
    clkmaskgate_offsetbit_check("pclk_mmbuf_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_pclk_mmbuf_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_mmbuf_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_aclk_mmbuf_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("pclk_mmbuf_div", 0x78, 1, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_END);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 1, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x1);

	clock_setrate_emulator(name, 5859375);
    clkdiv_offsetbit_check("pclk_mmbuf_div", 0x78, 4, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_END);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 64, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x1);

    clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("pclk_mmbuf_div", 0x78, 1, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_END);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 1, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x2);

	clock_setrate_emulator(name, 6336000);
    clkdiv_offsetbit_check("pclk_mmbuf_div", 0x78, 4, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_END);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 64, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x2);

    clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("pclk_mmbuf_div", 0x78, 1, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_END);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 1, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x4);

    clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("pclk_mmbuf_div", 0x78, 1, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_END);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 1, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x8);

	clock_setrate_emulator(name, 4687500);
    clkdiv_offsetbit_check("pclk_mmbuf_div", 0x78, 4, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV6_div_pclk_mmbuf_END);
    clkdiv_offsetbit_check("aclk_mmbuf_div", 0x7C, 64, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_START, SOC_MEDIA1_CRG_CLKDIV7_div_aclk_mmbuf_END);
    clkmux_offsetbit_check("aclk_mmbuf_sw", 0x88, SOC_MEDIA1_CRG_CLKDIV10_sel_mmbuf_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_mmbuf", 0x20, SOC_MEDIA1_CRG_PEREN2_gt_pclk_mmbuf_START, CLK_OFF);


	name = "clk_gps";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("abb_pll_gps_gt0", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_gps_gt_en0_START, CLK_ON);
    clkgate_offsetbit_check("abb_pll_gps_en0", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_gps_en0_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("abb_pll_gps_gt0", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_gps_gt_en0_START, CLK_OFF);
    clkgate_offsetbit_check("abb_pll_gps_en0", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_gps_en0_START, CLK_OFF);
 
	name = "clk_gps_fac";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("abb_pll_gps_gt1", 0x40, SOC_CRGPERIPH_PEREN4_sc_abb_pll_gps_gt_en1_START, CLK_ON);
    clkgate_offsetbit_check("abb_pll_gps_en1", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_gps_en1_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("abb_pll_gps_gt1", 0x40, SOC_CRGPERIPH_PEREN4_sc_abb_pll_gps_gt_en1_START, CLK_OFF);
    clkgate_offsetbit_check("abb_pll_gps_en1", 0x30, SOC_CRGPERIPH_PEREN3_sc_abb_pll_gps_en1_START, CLK_OFF);


	name = "clk_track";
	pr_err("\n");
	clock_setrate_emulator(name, 114285714);
	clock_setrate_emulator(name, 228571428);
	clock_setrate_emulator(name, 28571428);

	name = "clk_vcodecbus";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_vcodecbus", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_clk_vcodecbus_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_vcodbus_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_vcodecbus_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clk_vcodbus_div", 0xBC, 1, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_START, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_END);
    clkmux_offsetbit_check("clk_vcodbus_mux", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vcodecbus_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clk_vcodbus_div", 0xBC, 64, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_START, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_END);
    clkmux_offsetbit_check("clk_vcodbus_mux", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vcodecbus_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clk_vcodbus_div", 0xBC, 1, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_START, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_END);
    clkmux_offsetbit_check("clk_vcodbus_mux", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vcodecbus_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clk_vcodbus_div", 0xBC, 1, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_START, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_END);
    clkmux_offsetbit_check("clk_vcodbus_mux", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vcodecbus_pll_START, 0x4);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clk_vcodbus_div", 0xBC, 1, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_START, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_END);
    clkmux_offsetbit_check("clk_vcodbus_mux", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vcodecbus_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_vcodecbus", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_clk_vcodecbus_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_vcodbus_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_vcodecbus_START, CLK_OFF);

	name = "clk_edc0";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_edc0", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_edc0_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_edc0_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_edc0_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clk_edc0_div", 0x68, 1, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_START, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_END);
    clkmux_offsetbit_check("sel_edc0_pll", 0x68, SOC_MEDIA1_CRG_CLKDIV2_sel_edc0_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clk_edc0_div", 0x68, 64, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_START, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_END);
    clkmux_offsetbit_check("sel_edc0_pll", 0x68, SOC_MEDIA1_CRG_CLKDIV2_sel_edc0_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clk_edc0_div", 0x68, 1, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_START, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_END);
    clkmux_offsetbit_check("sel_edc0_pll", 0x68, SOC_MEDIA1_CRG_CLKDIV2_sel_edc0_pll_START, 0x2);

	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("clk_edc0_div", 0x68, 64, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_START, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_END);
    clkmux_offsetbit_check("sel_edc0_pll", 0x68, SOC_MEDIA1_CRG_CLKDIV2_sel_edc0_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clk_edc0_div", 0x68, 1, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_START, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_END);
    clkmux_offsetbit_check("sel_edc0_pll", 0x68, SOC_MEDIA1_CRG_CLKDIV2_sel_edc0_pll_START, 0x4);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clk_edc0_div", 0x68, 1, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_START, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_END);
    clkmux_offsetbit_check("sel_edc0_pll", 0x68, SOC_MEDIA1_CRG_CLKDIV2_sel_edc0_pll_START, 0x8);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("clk_edc0_div", 0x68, 64, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_START, SOC_MEDIA1_CRG_CLKDIV2_div_edc0_END);
    clkmux_offsetbit_check("sel_edc0_pll", 0x68, SOC_MEDIA1_CRG_CLKDIV2_sel_edc0_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_edc0", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_edc0_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_edc0_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_edc0_START, CLK_OFF);

	name = "clk_vcodec_peri";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_vcodecbus", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_clk_vcodecbus_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_vcodbus_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_vcodecbus_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clk_vcodbus_div", 0xBC, 1, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_START, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_END);
    clkmux_offsetbit_check("clk_vcodbus_mux", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vcodecbus_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clk_vcodbus_div", 0xBC, 64, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_START, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_END);
    clkmux_offsetbit_check("clk_vcodbus_mux", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vcodecbus_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clk_vcodbus_div", 0xBC, 1, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_START, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_END);
    clkmux_offsetbit_check("clk_vcodbus_mux", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vcodecbus_pll_START, 0x2);

	clock_setrate_emulator(name, 9600000000);
    clkdiv_offsetbit_check("clk_vcodbus_div", 0xBC, 1, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_START, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_END);
    clkmux_offsetbit_check("clk_vcodbus_mux", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vcodecbus_pll_START, 0x4);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clk_vcodbus_div", 0xBC, 1, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_START, SOC_CRGPERIPH_CLKDIV5_div_vcodecbus_END);
    clkmux_offsetbit_check("clk_vcodbus_mux", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vcodecbus_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_vcodecbus", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_clk_vcodecbus_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_vcodbus_gt", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_vcodecbus_START, CLK_OFF);

	name = "clk_ldi0";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ldi0", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_ldi0_START, CLK_ON);
    clkmaskgate_offsetbit_check("clkgt_ldi0", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ldi0_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clkdiv_ldi0", 0x60, 1, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_START, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_END);
    clkmux_offsetbit_check("clkmux_ldi0", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_ldi0_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clkdiv_ldi0", 0x60, 64, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_START, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_END);
    clkmux_offsetbit_check("clkmux_ldi0", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_ldi0_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clkdiv_ldi0", 0x60, 1, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_START, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_END);
    clkmux_offsetbit_check("clkmux_ldi0", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_ldi0_pll_START, 0x2);

	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("clkdiv_ldi0", 0x60, 64, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_START, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_END);
    clkmux_offsetbit_check("clkmux_ldi0", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_ldi0_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clkdiv_ldi0", 0x60, 1, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_START, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_END);
    clkmux_offsetbit_check("clkmux_ldi0", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_ldi0_pll_START, 0x4);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clkdiv_ldi0", 0x60, 1, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_START, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_END);
    clkmux_offsetbit_check("clkmux_ldi0", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_ldi0_pll_START, 0x8);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("clkdiv_ldi0", 0x60, 64, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_START, SOC_MEDIA1_CRG_CLKDIV0_div_ldi0_END);
    clkmux_offsetbit_check("clkmux_ldi0", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_ldi0_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ldi0", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_ldi0_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clkgt_ldi0", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ldi0_START, CLK_OFF);


	name = "clk_venc";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_vencfreq", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_clk_venc_START, CLK_ON);
    clkmaskgate_offsetbit_check("clkgt_venc", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_venc_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clkdiv_venc", 0xC0, 1, SOC_CRGPERIPH_CLKDIV6_div_venc_START, SOC_CRGPERIPH_CLKDIV6_div_venc_END);
    clkmux_offsetbit_check("clkmux_venc", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_venc_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clkdiv_venc", 0xC0, 64, SOC_CRGPERIPH_CLKDIV6_div_venc_START, SOC_CRGPERIPH_CLKDIV6_div_venc_END);
    clkmux_offsetbit_check("clkmux_venc", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_venc_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clkdiv_venc", 0xC0, 1, SOC_CRGPERIPH_CLKDIV6_div_venc_START, SOC_CRGPERIPH_CLKDIV6_div_venc_END);
    clkmux_offsetbit_check("clkmux_venc", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_venc_pll_START, 0x2);

	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("clkdiv_venc", 0xC0, 64, SOC_CRGPERIPH_CLKDIV6_div_venc_START, SOC_CRGPERIPH_CLKDIV6_div_venc_END);
    clkmux_offsetbit_check("clkmux_venc", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_venc_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clkdiv_venc", 0xC0, 1, SOC_CRGPERIPH_CLKDIV6_div_venc_START, SOC_CRGPERIPH_CLKDIV6_div_venc_END);
    clkmux_offsetbit_check("clkmux_venc", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_venc_pll_START, 0x4);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clkdiv_venc", 0xC0, 1, SOC_CRGPERIPH_CLKDIV6_div_venc_START, SOC_CRGPERIPH_CLKDIV6_div_venc_END);
    clkmux_offsetbit_check("clkmux_venc", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_venc_pll_START, 0x8);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("clkdiv_venc", 0xC0, 64, SOC_CRGPERIPH_CLKDIV6_div_venc_START, SOC_CRGPERIPH_CLKDIV6_div_venc_END);
    clkmux_offsetbit_check("clkmux_venc", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_venc_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_vencfreq", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_clk_venc_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clkgt_venc", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_venc_START, CLK_OFF);


	name = "pclk_venc";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_venc", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_pclk_venc_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_venc", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_pclk_venc_START, CLK_OFF);


	name = "clk_vdecfreq";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_vdecfreq", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_clk_vdec_START, CLK_ON);
    clkmaskgate_offsetbit_check("clkgt_vdec", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_vdec_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clkdiv_vdec", 0xC4, 1, SOC_CRGPERIPH_CLKDIV7_div_vdec_START, SOC_CRGPERIPH_CLKDIV7_div_vdec_END);
    clkmux_offsetbit_check("clkmux_vdec", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vdec_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clkdiv_vdec", 0xC4, 64, SOC_CRGPERIPH_CLKDIV7_div_vdec_START, SOC_CRGPERIPH_CLKDIV7_div_vdec_END);
    clkmux_offsetbit_check("clkmux_vdec", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vdec_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clkdiv_vdec", 0xC4, 1, SOC_CRGPERIPH_CLKDIV7_div_vdec_START, SOC_CRGPERIPH_CLKDIV7_div_vdec_END);
    clkmux_offsetbit_check("clkmux_vdec", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vdec_pll_START, 0x2);

	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("clkdiv_vdec", 0xC4, 64, SOC_CRGPERIPH_CLKDIV7_div_vdec_START, SOC_CRGPERIPH_CLKDIV7_div_vdec_END);
    clkmux_offsetbit_check("clkmux_vdec", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vdec_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clkdiv_vdec", 0xC4, 1, SOC_CRGPERIPH_CLKDIV7_div_vdec_START, SOC_CRGPERIPH_CLKDIV7_div_vdec_END);
    clkmux_offsetbit_check("clkmux_vdec", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vdec_pll_START, 0x4);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clkdiv_vdec", 0xC4, 1, SOC_CRGPERIPH_CLKDIV7_div_vdec_START, SOC_CRGPERIPH_CLKDIV7_div_vdec_END);
    clkmux_offsetbit_check("clkmux_vdec", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vdec_pll_START, 0x8);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("clkdiv_vdec", 0xC4, 64, SOC_CRGPERIPH_CLKDIV7_div_vdec_START, SOC_CRGPERIPH_CLKDIV7_div_vdec_END);
    clkmux_offsetbit_check("clkmux_vdec", 0xC8, SOC_CRGPERIPH_CLKDIV8_sel_vdec_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_vdecfreq", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_clk_vdec_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clkgt_vdec", 0xF0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_vdec_START, CLK_OFF);

	name = "pclk_vdec";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_vdec", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_pclk_vdec_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_vdec", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_pclk_vdec_START, CLK_OFF);


	name = "clk_vivobus";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_vivobusfreq", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_vivobus_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_vivobus_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x4);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x8);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 64, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_vivobusfreq", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_vivobus_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_vivobus_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_START, CLK_ON);

	name = "aclk_dss";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("aclk_dss", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_aclk_dss_START, CLK_ON);
	   clkgate_offsetbit_check("aclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PERSTAT1_st_aclk_disp_noc_subsys_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("aclk_dss", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_aclk_dss_START, CLK_OFF);
	clkgate_offsetbit_check("aclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PERSTAT1_st_aclk_disp_noc_subsys_START, CLK_OFF);


	name = "aclk_isp";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("aclk_isp", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_aclk_isp_START, CLK_ON);
	clkgate_offsetbit_check("aclk_isp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PERSTAT1_st_aclk_isp_noc_subsys_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("aclk_isp", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_aclk_isp_START, CLK_OFF);
	clkgate_offsetbit_check("aclk_isp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PERSTAT1_st_aclk_isp_noc_subsys_START, CLK_OFF);


	name = "clk_vivobus2ddr";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_vivobus2ddr", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_vivobus2ddrc_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_vivobus_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 64, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x4);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_vivobus2ddr", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_vivobus2ddrc_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_vivobus_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_START, CLK_ON);


	name = "clk_ispcpu";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ispcpufreq", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_cpu_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_ispcpu_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_isp_cpu_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clk_ispcpu_div", 0x70, 1, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_START, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_END);
    clkmux_offsetbit_check("sel_ispcpu", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_isp_cpu_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clk_ispcpu_div", 0x70, 64, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_START, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_END);
    clkmux_offsetbit_check("sel_ispcpu", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_isp_cpu_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clk_ispcpu_div", 0x70, 1, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_START, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_END);
    clkmux_offsetbit_check("sel_ispcpu", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_isp_cpu_pll_START, 0x2);

	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("clk_ispcpu_div", 0x70, 64, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_START, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_END);
    clkmux_offsetbit_check("sel_ispcpu", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_isp_cpu_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clk_ispcpu_div", 0x70, 1, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_START, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_END);
    clkmux_offsetbit_check("sel_ispcpu", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_isp_cpu_pll_START, 0x4);

    clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clk_ispcpu_div", 0x70, 1, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_START, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_END);
    clkmux_offsetbit_check("sel_ispcpu", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_isp_cpu_pll_START, 0x8);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("clk_ispcpu_div", 0x70, 64, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_START, SOC_MEDIA1_CRG_CLKDIV4_div_isp_cpu_END);
    clkmux_offsetbit_check("sel_ispcpu", 0x60, SOC_MEDIA1_CRG_CLKDIV0_sel_isp_cpu_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ispcpufreq", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_cpu_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_ispcpu_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_isp_cpu_START, CLK_OFF);


	name = "clk_isp_sys";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_isp_sys", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_sys_START, CLK_ON);
    clkgate_offsetbit_check("clk_media_tcxo", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_media_tcxo_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_isp_sys", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_sys_START, CLK_OFF);
    clkgate_offsetbit_check("clk_media_tcxo", 0x40, SOC_CRGPERIPH_PEREN4_gt_clk_media_tcxo_START, CLK_OFF);


	name = "clk_ispfunc";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ispfuncfreq", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc_START, CLK_ON);
    clkmaskgate_offsetbit_check("clkgt_ispfunc", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ispfunc_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clkdiv_ispfunc", 0x6C, 1, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_START, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_END);
    clkmux_offsetbit_check("clkmux_ispfunc", 0x6C, SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clkdiv_ispfunc", 0x6C, 64, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_START, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_END);
    clkmux_offsetbit_check("clkmux_ispfunc", 0x6C, SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clkdiv_ispfunc", 0x6C, 1, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_START, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_END);
    clkmux_offsetbit_check("clkmux_ispfunc", 0x6C, SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc_pll_START, 0x2);

	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("clkdiv_ispfunc", 0x6C, 64, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_START, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_END);
    clkmux_offsetbit_check("clkmux_ispfunc", 0x6C, SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clkdiv_ispfunc", 0x6C, 1, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_START, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_END);
    clkmux_offsetbit_check("clkmux_ispfunc", 0x6C, SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc_pll_START, 0x4);

    clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clkdiv_ispfunc", 0x6C, 1, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_START, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_END);
    clkmux_offsetbit_check("clkmux_ispfunc", 0x6C, SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc_pll_START, 0x8);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("clkdiv_ispfunc", 0x6C, 64, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_START, SOC_MEDIA1_CRG_CLKDIV3_div_ispfunc_END);
    clkmux_offsetbit_check("clkmux_ispfunc", 0x6C, SOC_MEDIA1_CRG_CLKDIV3_sel_ispfunc_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ispfuncfreq", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_ispfunc_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clkgt_ispfunc", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ispfunc_START, CLK_OFF);


	name = "clk_ivpdsp_tcxo";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ivpdsp_tcxo", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_ivp32dsp_tcxo_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ivpdsp_tcxo", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_ivp32dsp_tcxo_START, CLK_OFF);

	name = "clk_ivpdsp_core";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ivpdsp_corefreq", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_ivp32dsp_core_crg_START, CLK_ON);
    clkmaskgate_offsetbit_check("clkgt_ivp32dsp", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ivp32dsp_core_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clkdiv_ivpdsp", 0x68, 1, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_START, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_END);
    clkmux_offsetbit_check("clkmux_ivp32dsp", 0x78, SOC_MEDIA1_CRG_CLKDIV6_sel_ivp32dsp_core_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clkdiv_ivpdsp", 0x68, 64, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_START, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_END);
    clkmux_offsetbit_check("clkmux_ivp32dsp", 0x78, SOC_MEDIA1_CRG_CLKDIV6_sel_ivp32dsp_core_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clkdiv_ivpdsp", 0x68, 1, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_START, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_END);
    clkmux_offsetbit_check("clkmux_ivp32dsp", 0x78, SOC_MEDIA1_CRG_CLKDIV6_sel_ivp32dsp_core_pll_START, 0x2);

	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("clkdiv_ivpdsp", 0x68, 64, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_START, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_END);
    clkmux_offsetbit_check("clkmux_ivp32dsp", 0x78, SOC_MEDIA1_CRG_CLKDIV6_sel_ivp32dsp_core_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clkdiv_ivpdsp", 0x68, 1, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_START, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_END);
    clkmux_offsetbit_check("clkmux_ivp32dsp", 0x78, SOC_MEDIA1_CRG_CLKDIV6_sel_ivp32dsp_core_pll_START, 0x4);

    clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clkdiv_ivpdsp", 0x68, 1, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_START, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_END);
    clkmux_offsetbit_check("clkmux_ivp32dsp", 0x78, SOC_MEDIA1_CRG_CLKDIV6_sel_ivp32dsp_core_pll_START, 0x8);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("clkdiv_ivpdsp", 0x68, 64, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_START, SOC_MEDIA1_CRG_CLKDIV2_div_ivp32dsp_core_END);
    clkmux_offsetbit_check("clkmux_ivp32dsp", 0x78, SOC_MEDIA1_CRG_CLKDIV6_sel_ivp32dsp_core_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ivpdsp_corefreq", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_ivp32dsp_core_crg_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clkgt_ivp32dsp", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_ivp32dsp_core_START, CLK_OFF);

	name = "aclk_venc";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("aclk_venc", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_aclk_venc_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("aclk_venc", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_aclk_venc_START, CLK_OFF);

	name = "aclk_vdec";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("aclk_vdec", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_aclk_vdec_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("aclk_vdec", 0x0, SOC_MEDIA2_CRG_PEREN0_gt_aclk_vdec_START, CLK_OFF);

	name = "clk_ispcputocfg";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ispcputocfg", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_cputocfg_START, CLK_ON);
    clkgate_offsetbit_check("pclk_isp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_isp_noc_subsys_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ispcputocfg", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_cputocfg_START, CLK_OFF);
    clkgate_offsetbit_check("pclk_isp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_isp_noc_subsys_START, CLK_OFF);

    name = "aclk_noc_isp";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("aclk_noc_isp", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_aclk_noc_isp_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("aclk_noc_isp", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_aclk_noc_isp_START, CLK_OFF);

	name = "pclk_noc_isp";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_noc_isp", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_noc_isp_START, CLK_ON);
    clkgate_offsetbit_check("pclk_isp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_isp_noc_subsys_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_noc_isp", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_noc_isp_START, CLK_OFF);
    clkgate_offsetbit_check("pclk_isp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_isp_noc_subsys_START, CLK_OFF);

    name = "clk_socp_deflat";
	pr_err("\n");
	clock_enable_emulator(name);
	 clkgate_offsetbit_check("clk_socp_acpu", 0x10, SOC_CRGPERIPH_PEREN1_gt_clk_socp_acpu_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_socp_def_gt", 0xFC, SOC_CRGPERIPH_CLKDIV21_sc_gt_clk_socp_deflate_START, CLK_ON);
    clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clk_socpdef_div", 0x700, 1, SOC_CRGPERIPH_CLKDIV26_div_socp_deflate_START, SOC_CRGPERIPH_CLKDIV26_div_socp_deflate_END);
    clock_setrate_emulator(name, 50688000);
    clkdiv_offsetbit_check("clk_socpdef_div", 0x700, 32, SOC_CRGPERIPH_CLKDIV26_div_socp_deflate_START, SOC_CRGPERIPH_CLKDIV26_div_socp_deflate_END);
    clock_disable_emulator(name);
	 clkgate_offsetbit_check("clk_socp_acpu", 0x10, SOC_CRGPERIPH_PEREN1_gt_clk_socp_acpu_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_socp_def_gt", 0xFC, SOC_CRGPERIPH_CLKDIV21_sc_gt_clk_socp_deflate_START, CLK_OFF);

    name = "tclk_socp";
	pr_err("\n");
	clock_enable_emulator(name);
     clkgate_offsetbit_check("clk_socp_acpu", 0x10, SOC_CRGPERIPH_PEREN1_gt_clk_socp_acpu_START, CLK_ON);
    clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_socp_acpu", 0x10, SOC_CRGPERIPH_PEREN1_gt_clk_socp_acpu_START, CLK_OFF);

    name = "clk_a57hpm";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_a57hpm", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_a57hpm_START, CLK_ON);
	clkmaskgate_offsetbit_check("clk_a53hpm_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_a53hpm_START, CLK_ON);
    clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_a57hpm", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_a57hpm_START, CLK_OFF);

    name = "clk_a53hpm";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_a53hpm", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_a53hpm_START, CLK_ON);
	clkmaskgate_offsetbit_check("clk_a53hpm_gt", 0xF4, SOC_CRGPERIPH_CLKDIV19_sc_gt_clk_a53hpm_START, CLK_ON);
    clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_a53hpm", 0x50, SOC_CRGPERIPH_PEREN5_gt_clk_a53hpm_START, CLK_OFF);

    name = "clk_asp_subsys";
	pr_err("\n");
	clock_enable_emulator(name);
    clock_disable_emulator(name);
    clock_setrate_emulator(name, 1660000000);
    clock_setrate_emulator(name, 134400000);
    clock_setrate_emulator(name, 1920000000);
	clock_setrate_emulator(name, 1500000000);

	name = "aclk_noc_dss";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("aclk_noc_dss", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_aclk_noc_dss_START, CLK_ON);
    clkgate_offsetbit_check("aclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_aclk_disp_noc_subsys_START, CLK_ON);
	clkgate_offsetbit_check("clk_vivobusfreq", 0x00, SOC_MEDIA1_CRG_PEREN0_gt_clk_vivobus_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_vivobus_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_START, CLK_ON);

	clock_setrate_emulator(name, 1500000000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x1);
	clock_setrate_emulator(name, 23437500);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 64, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x1);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x2);

	clock_setrate_emulator(name, 25344000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 64, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x2);

	clock_setrate_emulator(name, 960000000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x4);

	clock_setrate_emulator(name, 1200000000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 1, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x8);

	clock_setrate_emulator(name, 18750000);
    clkdiv_offsetbit_check("clk_vivobus_div", 0x74, 64, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_START, SOC_MEDIA1_CRG_CLKDIV5_div_vivobus_END);
    clkmux_offsetbit_check("clk_vivobus_mux", 0x74, SOC_MEDIA1_CRG_CLKDIV5_sel_vivobus_pll_START, 0x8);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("aclk_noc_dss", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_aclk_noc_dss_START, CLK_OFF);
    clkgate_offsetbit_check("aclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_aclk_disp_noc_subsys_START, CLK_OFF);
	clkgate_offsetbit_check("clk_vivobusfreq", 0x00, SOC_MEDIA1_CRG_PEREN0_gt_clk_vivobus_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_vivobus_gt", 0x84, SOC_MEDIA1_CRG_CLKDIV9_sc_gt_clk_vivobus_START, CLK_ON);

	name = "pclk_noc_dss_cfg";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_noc_dss_cfg", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_noc_dss_START,CLK_ON);
    clkgate_offsetbit_check("pclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_disp_noc_subsys_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_noc_dss_cfg", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_noc_dss_START, CLK_OFF);
    clkgate_offsetbit_check("pclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_disp_noc_subsys_START, CLK_OFF);

	name = "pclk_mmbuf_cfg";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_mmbuf_cfg", 0x20, SOC_MEDIA1_CRG_PEREN2_gt_pclk_mmbuf_cfg_START, CLK_ON);
    clkgate_offsetbit_check("pclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_disp_noc_subsys_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_mmbuf_cfg", 0x20, SOC_MEDIA1_CRG_PEREN2_gt_pclk_mmbuf_cfg_START, CLK_OFF);
    clkgate_offsetbit_check("pclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_disp_noc_subsys_START, CLK_OFF);

	name = "pclk_dss";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_dss", 0x00, SOC_MEDIA1_CRG_PEREN0_gt_pclk_dss_START, CLK_ON);
    clkgate_offsetbit_check("pclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_disp_noc_subsys_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_dss", 0x00, SOC_MEDIA1_CRG_PEREN0_gt_pclk_dss_START, CLK_OFF);
    clkgate_offsetbit_check("pclk_disp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_pclk_disp_noc_subsys_START, CLK_OFF);

	name = "clk_noc_ivp32_cfg";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_noc_ivp32_cfg", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_clk_noc_ivp32_cfg_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_noc_ivp32_cfg", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_clk_noc_ivp32_cfg_START, CLK_OFF);

	name = "clk_asp_subsys";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_asp_subsys", 0x170, SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_acpu_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_asp_subsys", 0x170, SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_acpu_START, CLK_OFF);

	name = "clk_asp_subsys_peri";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_asp_subsys_peri", 0x170, SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_peri_acpu_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_asp_subsys_peri", 0x170, SOC_SCTRL_SCPEREN1_gt_clk_asp_subsys_peri_acpu_START, CLK_OFF);


	name = "clk_ispi2c";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ispi2c", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_i2c_START, CLK_ON);

	clock_setrate_emulator(name, 108134400);
    clkmux_offsetbit_check("clk_ispi2c_mux", 0x78, SOC_MEDIA1_CRG_CLKDIV6_sel_isp_i2c_START, 0x1);
    /*ÏÈset_rate£¬ÔÚenable*/
    clkgate_offsetbit_check("clk_isp_i2c_media", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_isp_i2c_media_START, CLK_ON);
    clkdiv_offsetbit_check("clk_div_isp_i2c", 0xB8, 3, 7, 10);
    clkmaskgate_offsetbit_check("clk_gt_isp_i2c", 0xF8, SOC_CRGPERIPH_CLKDIV20_sc_gt_clk_isp_i2c_START, CLK_ON);
    clkmaskgate_offsetbit_check("gt_clk_320m_pll", 0xF8, SOC_CRGPERIPH_CLKDIV20_sc_gt_clk_320m_pll_START, CLK_ON);
    clkmux_offsetbit_check("sc_sel_320m_pll", 0x100, SOC_CRGPERIPH_CLKDIV22_sel_320m_pll_START, 0x1);

	clock_setrate_emulator(name, 19200000);
    clkmux_offsetbit_check("clk_ispi2c_mux", 0x78, SOC_MEDIA1_CRG_CLKDIV6_sel_isp_i2c_START, 0x0);

	clock_setrate_emulator(name, 20275200);
    clkdiv_offsetbit_check("clk_div_isp_i2c", 0xB8, 16, 7, 10);

	clock_setrate_emulator(name, 324403200);
    clkdiv_offsetbit_check("clk_div_isp_i2c", 0xB8, 1, 7, 10);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ispi2c", 0x0, SOC_MEDIA1_CRG_PEREN0_gt_clk_isp_i2c_START, CLK_OFF);
    clkgate_offsetbit_check("clk_isp_i2c_media", 0x30, SOC_CRGPERIPH_PEREN3_gt_clk_isp_i2c_media_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_gt_isp_i2c", 0xF8, SOC_CRGPERIPH_CLKDIV20_sc_gt_clk_isp_i2c_START, CLK_OFF);

	name = "clk_ao_loadmonitor";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_ao_loadmonitor", 0x1b0, SOC_SCTRL_SCPEREN4_gt_clk_ao_loadmonitor_START, CLK_ON);
    clkmaskgate_offsetbit_check("clk_ao_loadmonitor_gt", 0x26c, SOC_SCTRL_SCCLKDIV7_sc_gt_clk_ao_loadmonitor_START, CLK_ON);

	clock_setrate_emulator(name, 1622016000);
    clkdiv_offsetbit_check("clk_ao_loadmonitor_div", 0x26c, 1, SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_START, SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_END);
    clkmux_offsetbit_check("clk_ao_loadmonitor_sw", 0x26c, SOC_SCTRL_SCCLKDIV7_sel_ao_loadmonitor_START, 0x1);
	clock_setrate_emulator(name, 101376000);
    clkdiv_offsetbit_check("clk_ao_loadmonitor_div", 0x26c, 16, SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_START, SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_END);
    clkmux_offsetbit_check("clk_ao_loadmonitor_sw", 0x26c, SOC_SCTRL_SCCLKDIV7_sel_ao_loadmonitor_START, 0x1);

	clock_setrate_emulator(name, 134400000);
    clkdiv_offsetbit_check("clk_ao_loadmonitor_div", 0x26c, 1, SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_START, SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_END);
    clkmux_offsetbit_check("clk_ao_loadmonitor_sw", 0x26c, SOC_SCTRL_SCCLKDIV7_sel_ao_loadmonitor_START, 0x0);
	clock_setrate_emulator(name, 8400000);
    clkdiv_offsetbit_check("clk_ao_loadmonitor_div", 0x26c, 16, SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_START, SOC_SCTRL_SCCLKDIV7_div_ao_loadmonitor_END);
    clkmux_offsetbit_check("clk_ao_loadmonitor_sw", 0x26c, SOC_SCTRL_SCCLKDIV7_sel_ao_loadmonitor_START, 0x0);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_ao_loadmonitor", 0x1b0, SOC_SCTRL_SCPEREN4_gt_clk_ao_loadmonitor_START, CLK_OFF);
    clkmaskgate_offsetbit_check("clk_ao_loadmonitor_gt", 0x26c, SOC_SCTRL_SCCLKDIV7_sc_gt_clk_ao_loadmonitor_START, CLK_OFF);

	name = "pclk_ao_loadmonitor";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_loadmonitor", 0x1b0, SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_ao_loadmonitor", 0x1b0, SOC_SCTRL_SCPEREN4_gt_pclk_ao_loadmonitor_START, CLK_OFF);

	name = "pclk_loadmonitor";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_loadmonitor", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_loadmonitor_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_loadmonitor", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_loadmonitor_START, CLK_OFF);


	name = "clk_loadmonitor";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_loadmonitor", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_loadmonitor_START, CLK_ON);
	clkmaskgate_offsetbit_check("clk_loadmonitor_gt", 0xf0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_loadmonitor_START, CLK_ON);

	clock_setrate_emulator(name, 324403200);
    clkdiv_offsetbit_check("clk_loadmonitor_div", 0xb8, 1, SOC_CRGPERIPH_CLKDIV4_div_loadmonitor_START, SOC_CRGPERIPH_CLKDIV4_div_loadmonitor_END);

	clock_setrate_emulator(name, 81100800);
    clkdiv_offsetbit_check("clk_loadmonitor_div", 0xb8, 4, SOC_CRGPERIPH_CLKDIV4_div_loadmonitor_START, SOC_CRGPERIPH_CLKDIV4_div_loadmonitor_END);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_loadmonitor", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_loadmonitor_START, CLK_OFF);
	clkmaskgate_offsetbit_check("clk_loadmonitor_gt", 0xf0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_loadmonitor_START, CLK_OFF);

	name = "pclk_loadmonitor_l";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("pclk_loadmonitor_l", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_loadmonitor_1_START, CLK_ON);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("pclk_loadmonitor_l", 0x20, SOC_CRGPERIPH_PEREN2_gt_pclk_loadmonitor_1_START, CLK_OFF);

	name = "clk_loadmonitor_l";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("clk_loadmonitor_l", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_loadmonitor_1_START, CLK_ON);
	clkmaskgate_offsetbit_check("clk_loadmonitor_gt", 0xf0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_loadmonitor_START, CLK_ON);

	clock_setrate_emulator(name, 324403200);
    clkdiv_offsetbit_check("clk_loadmonitor_div", 0xb8, 1, SOC_CRGPERIPH_CLKDIV4_div_loadmonitor_START, SOC_CRGPERIPH_CLKDIV4_div_loadmonitor_END);

	clock_setrate_emulator(name, 81100800);
    clkdiv_offsetbit_check("clk_loadmonitor_div", 0xb8, 4, SOC_CRGPERIPH_CLKDIV4_div_loadmonitor_START, SOC_CRGPERIPH_CLKDIV4_div_loadmonitor_END);

	clock_disable_emulator(name);
    clkgate_offsetbit_check("clk_loadmonitor_l", 0x20, SOC_CRGPERIPH_PEREN2_gt_clk_loadmonitor_1_START, CLK_OFF);
	clkmaskgate_offsetbit_check("clk_loadmonitor_gt", 0xf0, SOC_CRGPERIPH_CLKDIV18_sc_gt_clk_loadmonitor_START, CLK_OFF);

    name = "aclk_noc_isp";
	pr_err("\n");
	clock_enable_emulator(name);
    clkgate_offsetbit_check("aclk_noc_isp", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_aclk_noc_isp_START, CLK_ON);
	clkgate_offsetbit_check("aclk_isp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_aclk_isp_noc_subsys_START, CLK_ON);
	clock_disable_emulator(name);
    clkgate_offsetbit_check("aclk_noc_isp", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_aclk_noc_isp_START, CLK_OFF);
	clkgate_offsetbit_check("aclk_isp_noc_subsys", 0x10, SOC_MEDIA1_CRG_PEREN1_gt_aclk_isp_noc_subsys_START, CLK_OFF);
    return 0;
}
#endif
