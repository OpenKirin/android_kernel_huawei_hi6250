/*
 * Hisilicon dmss pattern trace driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "dmsspt: " fmt
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/bitops.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/hisi/util.h>
#include <soc_dmss_interface.h>
#include <mntn_public_interface.h>
#include "dmss_pt_dump.h"
#include "securec.h"
#include "hisi_ddr_autofsgt_proxy_kernel.h"

#ifdef SOC_DMSS_GLB_TRACE_CTRL0_ADDR
/*pattern trace related registers define*/
#define DMSS_GLB_INT_STATUS_PTR(base) ((volatile SOC_DMSS_GLB_INT_STATUS_UNION *)SOC_DMSS_GLB_INT_STATUS_ADDR(base))
#define DMSS_GLB_INT_CLEAR_PTR(base) ((volatile SOC_DMSS_GLB_INT_CLEAR_UNION *)SOC_DMSS_GLB_INT_CLEAR_ADDR(base))
#define DMSS_GLB_ADDR_INTLV_PTR(base) ((volatile SOC_DMSS_GLB_ADDR_INTLV_UNION *)SOC_DMSS_GLB_ADDR_INTLV_ADDR(base))
#define DMSS_GLB_TRACE_CTRL0_PTR(base) ((volatile SOC_DMSS_GLB_TRACE_CTRL0_UNION *)SOC_DMSS_GLB_TRACE_CTRL0_ADDR(base))
#define DMSS_GLB_TRACE_CTRL1_PTR(base) ((volatile SOC_DMSS_GLB_TRACE_CTRL1_UNION *)SOC_DMSS_GLB_TRACE_CTRL1_ADDR(base))
#define DMSS_GLB_TRACE_CTRL2_PTR(base) ((volatile SOC_DMSS_GLB_TRACE_CTRL2_UNION *)SOC_DMSS_GLB_TRACE_CTRL2_ADDR(base))
#define DMSS_GLB_TRACE_REC0_PTR(base) ((volatile SOC_DMSS_GLB_TRACE_REC0_UNION *)SOC_DMSS_GLB_TRACE_REC0_ADDR(base))
#define DMSS_GLB_TRACE_REC1_PTR(base) ((volatile SOC_DMSS_GLB_TRACE_REC1_UNION *)SOC_DMSS_GLB_TRACE_REC1_ADDR(base))
#define DMSS_GLB_TRACE_FILTER0_PTR(base) ((volatile SOC_DMSS_GLB_TRACE_FILTER0_UNION *)SOC_DMSS_GLB_TRACE_FILTER0_ADDR(base))
#define DMSS_GLB_TRACE_FILTER1_PTR(base) ((volatile SOC_DMSS_GLB_TRACE_FILTER1_UNION *)SOC_DMSS_GLB_TRACE_FILTER1_ADDR(base))
#define DMSS_GLB_TRACE_FILTER2_PTR(base, mid_grps) ((volatile SOC_DMSS_GLB_TRACE_FILTER2_UNION *)SOC_DMSS_GLB_TRACE_FILTER2_ADDR(base, mid_grps))
#define DMSS_GLB_TRACE_FREQ_PTR(base) ((volatile SOC_DMSS_GLB_TRACE_FREQ_UNION *)SOC_DMSS_GLB_TRACE_FREQ_ADDR(base))
#define DMSS_GLB_TRACE_INT_EN_PTR(base) ((volatile SOC_DMSS_GLB_TRACE_INT_EN_UNION *)SOC_DMSS_GLB_TRACE_INT_EN_ADDR(base))
#define DMSS_GLB_TRACE_LOCK_DOWN_PTR(base) ((volatile SOC_DMSS_GLB_TRACE_LOCK_DOWN_UNION *)SOC_DMSS_GLB_TRACE_LOCK_DOWN_ADDR(base))
#define DMSS_GLB_TRACE_STAT0_PTR(base, dmis) ((volatile SOC_DMSS_GLB_TRACE_STAT0_UNION *)SOC_DMSS_GLB_TRACE_STAT0_ADDR(base, dmis))
#define DMSS_GLB_TRACE_STAT1_PTR(base, dmis) ((volatile SOC_DMSS_GLB_TRACE_STAT1_UNION *)SOC_DMSS_GLB_TRACE_STAT1_ADDR(base, dmis))
#define DMSS_GLB_TRACE_STAT2_PTR(base, dmis) ((volatile SOC_DMSS_GLB_TRACE_STAT2_UNION *)SOC_DMSS_GLB_TRACE_STAT2_ADDR(base, dmis))

#define NAME_LEN_MAX 16
#define DMSSPT_SHOW_LEN_MAX	128
static u64 dmsspt_addr_chans2phy(unsigned int dmis, unsigned int gbl_trace_stat0, u32 intlv_gran);
static inline u16 dmsspt_addr_phy2chans(u64 phy_addr);

struct dmsspt_devices {
	spinlock_t	lock;
	void __iomem 	*dmss_base;
	u32 irq;
	u32 irq_cpu_core; /*使用哪个core处理中断*/
	u32 init_completed; /*dmsspt初始化完成*/
	u32 save_cnt; /*本轮记录过程调用dmsspt_save_trace的次数*/
	struct dmsspt_region pt_region;
	struct pattern_trace_info pt_info;
	struct pattern_trace_stat pt_stat;
};
static struct dmsspt_devices *g_pt_devp;

struct dmsspt_cfg {
	/*全局配置*/
	u8 roll_en; /*1/0 卷绕记录使能/禁止*/
	u8 unaligned_mode;/*0/1 识别32B/64B地址非对齐写命令*/
	u8 rec_ts_max_intrvl; /*强制插入timestamp时间间隔*/
	u8 rec_pri; /*TRACE命令的优先级配置*/
	u8 rec_mid; /*TRACE命令的MID配置*/
	u8 rec_intlv_gran; /*写入通道交织粒度*/
	u8 cur_freq; /*当前时钟频率信息记录*/
	u16 rec_top_addr; /*trace空间通道内地址上限, 单位MB*/
	u16 rec_base_addr; /*trace空间通道内地址下限, 单位MB*/

	/*停止与中断*/
	u32 max_pattern_num; /*命令pattern的最大记录个数, 任意DMI到达计数后停止*/
	u32 trace_prd; /*命令pattern的最大TRACE时间*/
	u16 int_trigger; /*当记录了n*1024个TRACE命令时，主动触发中断*/
	u8 trace_int_en; /*是否使能中断*/

	/*过滤条件*/
	u16 filter_top_addr; /*被TRACE命令的上限地址过滤, 单位MB*/
	u16 filter_base_addr; /*被TRACE命令的下限地址过滤, 单位MB*/
	u8 filter_ch; /*被TRACE命令的通道过滤*/
	u8 filter_type_wr; /*被TRACE命令的写类型过滤*/
	u8 filter_type_rd; /*被TRACE命令的读类型过滤*/
	u16 filter_asi; /*被TRACE命令的ASI过滤，0代表不被记录*/
	u32 filter_mid[4]; /*被TRACE命令的MID过滤，0代表不被记录*/
};

/*pattern trace default config*/
static struct dmsspt_cfg g_pt_cfg;

static int dmsspt_is_stop(void)
{
	int is_stop;
	SOC_DMSS_GLB_TRACE_CTRL0_UNION ctrl0;

	ctrl0 = (SOC_DMSS_GLB_TRACE_CTRL0_UNION)readl(DMSS_GLB_TRACE_CTRL0_PTR(g_pt_devp->dmss_base));
	is_stop = ctrl0.reg.trace_en ? 0 : 1;

	return is_stop;
}

static void dmsspt_enable(int enable)
{
	volatile SOC_DMSS_GLB_TRACE_CTRL0_UNION *ctrl0_ptr;
	volatile SOC_DMSS_GLB_INT_CLEAR_UNION *int_clear_ptr;

	ctrl0_ptr = DMSS_GLB_TRACE_CTRL0_PTR(g_pt_devp->dmss_base);
	int_clear_ptr = DMSS_GLB_INT_CLEAR_PTR(g_pt_devp->dmss_base);

	int_clear_ptr->reg.trace_int_clear = 1;
	ctrl0_ptr->reg.trace_en = enable ? 1 : 0;
	g_pt_devp->save_cnt = 0;
}

static u32 intlv_to_bytes(u32 intlv_gran)
{
	u32 intlv_bytes = 128;

	if((intlv_gran > 0) && (intlv_gran < 7)){
		intlv_bytes = 128*(0x01<<(intlv_gran - 1));
	} else {
		pr_err("ddr intlv is invalid!!!\n");
		WARN_ON(1);
	}

	return intlv_bytes;
}

static void dmsspt_get_intlv(void)
{
	SOC_DMSS_GLB_ADDR_INTLV_UNION intlv;

	intlv = (SOC_DMSS_GLB_ADDR_INTLV_UNION)readl(DMSS_GLB_ADDR_INTLV_PTR(g_pt_devp->dmss_base));

	g_pt_devp->pt_info.intlv = intlv_to_bytes(intlv.reg.intlv_gran);
	/*g_pt_devp->pt_info.intlv_mode = intlv.reg.intlv_mode;*/
	g_pt_devp->pt_info.intlv_mode = intlv.reg.ch4_order;

	g_pt_cfg.rec_intlv_gran = intlv.reg.intlv_gran;
}

static void dmsspt_get_stat(void)
{
	unsigned int i;
	u64 pt_buf_sta, pt_buf_end, wptr, step;

	pt_buf_sta = g_pt_devp->pt_region.base;
	pt_buf_end = g_pt_devp->pt_region.base + g_pt_devp->pt_region.size;
	step = g_pt_devp->pt_info.intlv_mode ? g_pt_devp->pt_info.intlv:(g_pt_devp->pt_info.intlv*2);


	for(i=0; i<DMSS_TRACE_MAX; i++)
	{
		g_pt_devp->pt_stat.trace_cur_address[i] = readl(DMSS_GLB_TRACE_STAT0_PTR(g_pt_devp->dmss_base, i));
		g_pt_devp->pt_stat.trace_pattern_cnt[i] = readl(DMSS_GLB_TRACE_STAT1_PTR(g_pt_devp->dmss_base, i));
		g_pt_devp->pt_stat.trace_roll_cnt[i] = readl(DMSS_GLB_TRACE_STAT2_PTR(g_pt_devp->dmss_base, i));

		wptr = dmsspt_addr_chans2phy(i, g_pt_devp->pt_stat.trace_cur_address[i],\
			g_pt_cfg.rec_intlv_gran);

		g_pt_devp->pt_info.pt_rptr[i] = 0;
		if (wptr >= pt_buf_end)
		{	/*非卷绕模式写指针可能会超trace buffer end地址*/
			pr_info("Rollback wptr%u:[%pK]->[%pK]\n", i, (void *)wptr, (void *)(pt_buf_sta + i * step));
			wptr = pt_buf_sta + i * step;

			if(0 == g_pt_devp->save_cnt)
			{
				g_pt_devp->pt_info.pt_rptr[i] = wptr + 2*step;
				pr_info("Move rptr%u->%pK\n", i, (void *)g_pt_devp->pt_info.pt_rptr[i]);
			}
		}

		if(g_pt_devp->pt_stat.trace_roll_cnt[i] && (0 == g_pt_devp->save_cnt))
		{
			if(wptr + 2*step < pt_buf_end)
				g_pt_devp->pt_info.pt_rptr[i] = wptr + 2*step;

			pr_info("Move rptr%u->%pK\n", i, (void *)g_pt_devp->pt_info.pt_rptr[i]);
		}

		g_pt_devp->pt_info.pt_wptr[i] = wptr;

		pr_info("pt_wptr%u:%pK\n", i, (void *)g_pt_devp->pt_info.pt_wptr[i]);
		pr_info("trace_cur_addr%u:0x%x\n", i, g_pt_devp->pt_stat.trace_cur_address[i]);
		pr_info("trace_pattern_cnt%u:0x%x\n",i, g_pt_devp->pt_stat.trace_pattern_cnt[i]);
	}

	g_pt_devp->pt_stat.trace_end_time = sched_clock();
}


static void dmsspt_pre_hw_init(struct dmsspt_cfg *cfg)
{
	int ret;
	u16 chans_addr;
	chans_addr = dmsspt_addr_phy2chans(g_pt_devp->pt_region.base);
	cfg->rec_base_addr = chans_addr;

	chans_addr = dmsspt_addr_phy2chans(g_pt_devp->pt_region.base + g_pt_devp->pt_region.size - 1);
	cfg->rec_top_addr = chans_addr;

	ret = irq_set_affinity(g_pt_devp->irq, cpumask_of(g_pt_devp->irq_cpu_core));
	if (ret < 0) {
		WARN_ON(1); /*lint !e730*/
		pr_warn("[%s] irq affinity fail,%d\n", __func__, g_pt_devp->irq_cpu_core);
	}
}

static void dmsspt_hw_init(struct dmsspt_cfg *cfg)
{
	unsigned int mid_grps;
	volatile SOC_DMSS_GLB_TRACE_CTRL1_UNION *ctrl1_ptr = DMSS_GLB_TRACE_CTRL1_PTR(g_pt_devp->dmss_base);
	volatile SOC_DMSS_GLB_TRACE_CTRL2_UNION *ctrl2_ptr = DMSS_GLB_TRACE_CTRL2_PTR(g_pt_devp->dmss_base);
	volatile SOC_DMSS_GLB_TRACE_REC0_UNION *rec0_ptr = DMSS_GLB_TRACE_REC0_PTR(g_pt_devp->dmss_base);
	volatile SOC_DMSS_GLB_TRACE_REC1_UNION *rec1_ptr = DMSS_GLB_TRACE_REC1_PTR(g_pt_devp->dmss_base);
	//volatile SOC_DMSS_GLB_TRACE_FREQ_UNION *freq_ptr = DMSS_GLB_TRACE_FREQ_PTR(g_pt_devp->dmss_base);
	volatile SOC_DMSS_GLB_TRACE_INT_EN_UNION *int_en_ptr = DMSS_GLB_TRACE_INT_EN_PTR(g_pt_devp->dmss_base);
	volatile SOC_DMSS_GLB_TRACE_FILTER0_UNION *filter0_ptr = DMSS_GLB_TRACE_FILTER0_PTR(g_pt_devp->dmss_base);
	volatile SOC_DMSS_GLB_TRACE_FILTER1_UNION *filter1_ptr = DMSS_GLB_TRACE_FILTER1_PTR(g_pt_devp->dmss_base);
	volatile SOC_DMSS_GLB_TRACE_FILTER2_UNION *filter2_ptr = DMSS_GLB_TRACE_FILTER2_PTR(g_pt_devp->dmss_base, 0);

	ctrl1_ptr->reg.max_pattern_num = cfg->max_pattern_num;
	ctrl1_ptr->reg.unaligned_mode = cfg->unaligned_mode;
	ctrl1_ptr->reg.roll_en = cfg->roll_en;

	ctrl2_ptr->reg.trace_prd = cfg->trace_prd;

	rec0_ptr->reg.rec_base_addr = cfg->rec_base_addr;
	rec0_ptr->reg.rec_top_addr = cfg->rec_top_addr;

	rec1_ptr->reg.rec_mid = cfg->rec_mid;
	rec1_ptr->reg.rec_pri = cfg->rec_pri;
	rec1_ptr->reg.rec_ts_max_intrvl = cfg->rec_ts_max_intrvl;
	rec1_ptr->reg.rec_intlv_gran = cfg->rec_intlv_gran;

	//freq_ptr->reg.cur_freq = cfg->cur_freq;

	int_en_ptr->reg.trace_int_en = cfg->trace_int_en;
	int_en_ptr->reg.int_trigger = cfg->int_trigger;

	filter0_ptr->reg.filter_base_addr = cfg->filter_base_addr;
	filter0_ptr->reg.filter_top_addr = cfg->filter_top_addr;

	filter1_ptr->reg.filter_asi = cfg->filter_asi;
	filter1_ptr->reg.filter_ch = cfg->filter_ch;
	filter1_ptr->reg.filter_type_wr = cfg->filter_type_wr;
	filter1_ptr->reg.filter_type_rd = cfg->filter_type_rd;

	for(mid_grps=0; mid_grps<4; mid_grps++) {
		filter2_ptr = DMSS_GLB_TRACE_FILTER2_PTR(g_pt_devp->dmss_base, mid_grps);
		filter2_ptr->value = cfg->filter_mid[mid_grps];
	}

	g_pt_devp->pt_stat.trace_unaligned_mode = cfg->unaligned_mode;
}

/*trans dmc channel address to phy address*/
static u64 dmsspt_addr_chans2phy(unsigned int dmis, unsigned int gbl_trace_stat0, u32 intlv_gran)
{
/*
segment             high addr  | chsel(2bit) | low addr(size=intlv gran)
phyaddr bit def     zz...zzz   |     yy      | xx...xx
local variable  addr_high_bits |    chsel    | addr_low_bits
*/
	u32 chsel, chsel_start;
	u32 stat_cur_addr_ch = gbl_trace_stat0 >> 31;
	u64 stat_cur_addr = ((u64)(gbl_trace_stat0 & 0x7fffffff)) << 7;
	u64 phy_addr, addr_high_bits, addr_low_bits;

	if((dmis >= DMSS_TRACE_MAX) || (!intlv_gran) || (intlv_gran >= 7))
	{
		pr_err("%s param error: dmis=%u, intlv_gran=%u", __func__, dmis, intlv_gran);
		return 0;
	}

	/*
	        DMI1       DMI0
		 |          |
	      -------    -------
	      |     |    |     |
	     DMC3  DMC1 DMC2  DMC0
	*/
	if (g_pt_devp->pt_info.intlv_mode) {
		/*性能模式: 0->1->2->3*/
		chsel = dmis + stat_cur_addr_ch*2;
	} else {
		/*功耗模式: 0->2->1->3*/
		chsel = dmis*2 + stat_cur_addr_ch;
	}
	/*
	intlv_gran |  chsel     | chsel_start
	1: 128byte | addr[8:7]  | 7
	2: 256byte | addr[9:8]  | 8
	3: 512byte | addr[10:9] | 9
	...
	*/
	chsel_start = 6 + intlv_gran;
	addr_low_bits = stat_cur_addr & ((1<<chsel_start) - 1);
	addr_high_bits = stat_cur_addr >> chsel_start;
	phy_addr = (addr_high_bits<<(chsel_start + 2)) | (chsel<<chsel_start) | addr_low_bits;

	return phy_addr;
}

static u16 dmsspt_addr_phy2chans(u64 phy_addr)
{
	u16 chans_addr = 0;
	u64 tmp;

	tmp = phy_addr >> (2 + 20); /* 4 channel(2bit) + 单位:MB(20bit)*/

	if(tmp & (~0xffffUL)) {
		pr_err("%s: phy_addr=%pK, fail!!!\n", __func__, (void *)phy_addr);
		WARN_ON(1);
	} else {
		chans_addr = (u16)tmp;
	}

	return chans_addr;
}


#ifdef CONFIG_HISI_DEBUG_FS
static void dmsspt_init_store(unsigned int val)
{
	unsigned long flags;

	pr_info("%s start\n", __func__);
	if( !val || !dmsspt_is_stop()) {
		pr_warn("pattern trace is running\n");
		return;
	}
	spin_lock_irqsave(&g_pt_devp->lock, flags);
	dmsspt_pre_hw_init(&g_pt_cfg);
	dmsspt_hw_init(&g_pt_cfg);
	g_pt_devp->init_completed = 1;
	spin_unlock_irqrestore(&g_pt_devp->lock, flags);

	pr_info("%s done\n", __func__);
}

static void dmsspt_start_store(unsigned int val)
{
	unsigned long flags;

	pr_info("%s start\n", __func__);
	if(!dmsspt_is_finished()) {
		pr_err("the last trace save is not finished!\n");
		return;
	}

#ifdef CONFIG_KIRIN970_DDR_AUTO_FSGT
	(void)ddr_autofsgt_ctrl(DDR_AUTOFSGT_PROXY_CLIENT_DMSSPT, DDR_AUTOFSGT_LOGIC_DIS);
#endif
	if (val && dmsspt_is_stop() && g_pt_devp->init_completed) {
		spin_lock_irqsave(&g_pt_devp->lock, flags);
		g_pt_devp->pt_stat.trace_begin_time = sched_clock();
		dmsspt_enable(1);
		spin_unlock_irqrestore(&g_pt_devp->lock, flags);
	}
	pr_info("%s done\n", __func__);
}

static void dmsspt_stop_store(unsigned int val)
{
	unsigned long flags;

	pr_info("%s start\n", __func__);
	if (val && !dmsspt_is_stop()) {
		pr_info("manual stop pattern trace\n");
		spin_lock_irqsave(&g_pt_devp->lock, flags);
		dmsspt_enable(0);
		g_pt_devp->pt_stat.trace_end_time = sched_clock();
		dmsspt_get_stat();
		dmsspt_save_trace(&g_pt_devp->pt_info, &g_pt_devp->pt_stat, 1);
		spin_unlock_irqrestore(&g_pt_devp->lock, flags);
	}

#ifdef CONFIG_KIRIN970_DDR_AUTO_FSGT
	(void)ddr_autofsgt_ctrl(DDR_AUTOFSGT_PROXY_CLIENT_DMSSPT, DDR_AUTOFSGT_LOGIC_EN);
#endif
	pr_info("%s done\n", __func__);
}

static void dmsspt_irq_core_store(unsigned int val)
{
	if (val < 8)
		g_pt_devp->irq_cpu_core = val;
	else {
		WARN_ON(1);
	}
}

static ssize_t dmsspt_finished_show(char *kbuf)
{
	int cnt;

	if(dmsspt_is_finished())
	{
		cnt = snprintf_s(kbuf, (ssize_t)DMSSPT_SHOW_LEN_MAX, (ssize_t)DMSSPT_SHOW_LEN_MAX, "#finished#\n");
	}
	else
	{
		cnt = snprintf_s(kbuf, (ssize_t)DMSSPT_SHOW_LEN_MAX, (ssize_t)DMSSPT_SHOW_LEN_MAX, "#saving#\n");
	}

	return cnt;
}

static ssize_t dmsspt_region_show(char *kbuf)
{
	int cnt;

	cnt = snprintf_s(kbuf, (ssize_t)DMSSPT_SHOW_LEN_MAX, (ssize_t)DMSSPT_SHOW_LEN_MAX, "base=0x%pK, size=0x%llx\n",\
		(void *)g_pt_devp->pt_region.base, g_pt_devp->pt_region.size);
	return cnt;
}

static ssize_t dmsspt_intlv_show(char *kbuf)
{
	int cnt;

	cnt = snprintf_s(kbuf, (ssize_t)DMSSPT_SHOW_LEN_MAX, (ssize_t)DMSSPT_SHOW_LEN_MAX, "intlv=%u, ch4_order=%u\n",\
		g_pt_devp->pt_info.intlv, g_pt_devp->pt_info.intlv_mode);

	return cnt;
}


/* debugfs node for demand dmsspt, such as init/start/stop .etc*/
struct dmsspt_op_node{
	char *name;
	void (*store)(unsigned int);
	ssize_t (*show)(char *kbuf);
};

static const struct dmsspt_op_node g_pt_op_nodes[] = {
	{"init", dmsspt_init_store, NULL},
	{"start", dmsspt_start_store, NULL},
	{"stop", dmsspt_stop_store, NULL},
	{"irq_core", dmsspt_irq_core_store, NULL},
	{"finished", NULL, dmsspt_finished_show},
	{"rec_region", NULL, dmsspt_region_show},
	{"intlv", NULL, dmsspt_intlv_show},
};

static const struct dmsspt_op_node * find_op_node(struct file *filp)
{
	int i, ret;
	const struct dmsspt_op_node *np = NULL;

	for(i=0; i<ARRAY_SIZE(g_pt_op_nodes); i++)
	{
		ret = strncmp(g_pt_op_nodes[i].name, filp->private_data, strnlen(g_pt_op_nodes[i].name, NAME_LEN_MAX));
		if(!ret) {
			np = &g_pt_op_nodes[i];
			break;
		}
	}

	return np;
}

static ssize_t dmsspt_comm_store(struct file *filp, const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	int ret;
	unsigned int val = 0;
	char kbuf[4] = {'\0'};
	const struct dmsspt_op_node *np;

	if (NULL == ubuf || 0 == cnt) {
		pr_err("buf is null !\n");
		return -EINVAL;
	}

	if (cnt >= sizeof(kbuf)) {
		pr_err("input char too many! \n");
		return -ENOMEM;
	}

	if (copy_from_user(kbuf, ubuf, cnt)) {
		return -EINVAL;
	}

	ret = kstrtouint(kbuf, 10, &val);
	if(ret < 0)
	{
		pr_err("input error: %x %x %x!\n", kbuf[0], kbuf[1], kbuf[2]);
		return -EINVAL;
	}

	np = find_op_node(filp);
	if(np&&np->store)
		np->store(val);

	return (ssize_t)cnt;
}

ssize_t dmsspt_comm_show(struct file *filp, char __user *ubuf, size_t size, loff_t *ppos)
{
	int cnt = 0;
	char kbuf[DMSSPT_SHOW_LEN_MAX] = {0};
	const struct dmsspt_op_node *np;

	if(!access_ok(VERIFY_WRITE, ubuf, DMSSPT_SHOW_LEN_MAX))
	{
		pr_err("ubuf is invalid@%s\n", (char *)filp->private_data);
		return -EINVAL;
	}

	np = find_op_node(filp);
	if(np && np->show) {
		cnt = np->show(kbuf);
		return simple_read_from_buffer(ubuf, size, ppos, kbuf, cnt);
	}

	return 0;
}

static int dmsspt_open(struct inode *inode, struct file *filp)
{
	filp->private_data = inode->i_private;
	return 0;
}

static const struct file_operations dmss_op_fops = {
	.open		= dmsspt_open,
	.write		= dmsspt_comm_store,
	.read		= dmsspt_comm_show,
};

/*debugfs node for config dmsspt*/
struct dmsspt_mod_cfg {
	char *name;
	unsigned int bitwidth;
	void *val_p;
};

static const struct dmsspt_mod_cfg g_pt_mod_cfg[] = {
	{"roll_en",		1, &g_pt_cfg.roll_en},
	{"unaligned_mode",	1, &g_pt_cfg.unaligned_mode},
	{"rec_ts_max_intrvl",	4, &g_pt_cfg.rec_ts_max_intrvl},
	{"rec_pri",		3, &g_pt_cfg.rec_pri},
	{"rec_mid",		8, &g_pt_cfg.rec_mid},
	//{"rec_intlv_gran",	3, &g_pt_cfg.rec_intlv_gran},
	{"cur_freq",		4, &g_pt_cfg.cur_freq},

	{"max_pattern_num",	28, &g_pt_cfg.max_pattern_num},
	{"trace_prd",		32, &g_pt_cfg.trace_prd},
	{"int_trigger",		16, &g_pt_cfg.int_trigger},
	{"trace_int_en",	1, &g_pt_cfg.trace_int_en},

	{"filter_ch",		4, &g_pt_cfg.filter_ch},
	{"filter_type_wr",	1, &g_pt_cfg.filter_type_wr},
	{"filter_type_rd",	1, &g_pt_cfg.filter_type_rd},
	{"filter_asi",		12, &g_pt_cfg.filter_asi},
	{"filter_mid0",		32, &g_pt_cfg.filter_mid[0]},
	{"filter_mid1",		32, &g_pt_cfg.filter_mid[1]},
	{"filter_mid2",		32, &g_pt_cfg.filter_mid[2]},
	{"filter_mid3",		32, &g_pt_cfg.filter_mid[3]},
	{"filter_top_addr",	16, &g_pt_cfg.filter_top_addr},
	{"filter_base_addr",	16, &g_pt_cfg.filter_base_addr},
};

#define PRIV_AUTH       (S_IRUSR|S_IWUSR|S_IRGRP)
static int dmsspt_debugfs_init(void)
{
	unsigned int i;
	struct dentry *pt_dir, *mode_cfg;

	pt_dir = debugfs_create_dir("pattern_trace", NULL);
	if (!pt_dir)
		return -ENOMEM;

	mode_cfg = debugfs_create_dir("mode_cfg", pt_dir);
	if (!mode_cfg)
		return -ENOMEM;

	for(i=0; i<ARRAY_SIZE(g_pt_op_nodes); i++)
	{
		debugfs_create_file(g_pt_op_nodes[i].name, PRIV_AUTH, pt_dir, \
			g_pt_op_nodes[i].name, &dmss_op_fops);
	}

	for(i=0; i<ARRAY_SIZE(g_pt_mod_cfg); i++)
	{
		if(g_pt_mod_cfg[i].bitwidth == 1)
			debugfs_create_bool(g_pt_mod_cfg[i].name, PRIV_AUTH, mode_cfg, g_pt_mod_cfg[i].val_p);
		else if(g_pt_mod_cfg[i].bitwidth <= 8)
			debugfs_create_x8(g_pt_mod_cfg[i].name, PRIV_AUTH, mode_cfg, g_pt_mod_cfg[i].val_p);
		else if(g_pt_mod_cfg[i].bitwidth <= 16)
			debugfs_create_x16(g_pt_mod_cfg[i].name, PRIV_AUTH, mode_cfg, g_pt_mod_cfg[i].val_p);
		else if(g_pt_mod_cfg[i].bitwidth <= 32)
			debugfs_create_x32(g_pt_mod_cfg[i].name, PRIV_AUTH, mode_cfg, g_pt_mod_cfg[i].val_p);
		else {
			pr_err("create debugfs node fail\n");
			return -EINVAL;
		}
	}
	return 0;
}
#else
static int dmsspt_debugfs_init(void)
{
	return 0;
}
#endif

static irqreturn_t dmsspt_interrupt(int irq, void *dev_id)
{
	unsigned long flags;
	int ret, is_stop;
	volatile SOC_DMSS_GLB_INT_STATUS_UNION *int_stat_ptr = DMSS_GLB_INT_STATUS_PTR(g_pt_devp->dmss_base);
	volatile SOC_DMSS_GLB_INT_CLEAR_UNION *int_clear_ptr = DMSS_GLB_INT_CLEAR_PTR(g_pt_devp->dmss_base);

	pr_info("%s@%llu\n", __func__, sched_clock());

	if(!int_stat_ptr->reg.trace_int) {
		pr_err("non pattern trace interrupt\n");
		WARN_ON(1);
		return IRQ_NONE;
	}

	spin_lock_irqsave(&g_pt_devp->lock, flags);

	is_stop = dmsspt_is_stop();
	dmsspt_get_stat();
	pr_info("is_stop=%d, cnt=%u\n", is_stop, g_pt_devp->save_cnt);
	ret = dmsspt_save_trace(&g_pt_devp->pt_info, &g_pt_devp->pt_stat, is_stop);
	g_pt_devp->save_cnt++;
	if(ret) {
		pr_err("dmsspt_save_trace fail, ret=%d!\n", ret);
		dmsspt_enable(0);
	}

	spin_unlock_irqrestore(&g_pt_devp->lock, flags);

#ifdef CONFIG_KIRIN970_DDR_AUTO_FSGT
	if(is_stop || ret) {
		(void)ddr_autofsgt_ctrl(DDR_AUTOFSGT_PROXY_CLIENT_DMSSPT, DDR_AUTOFSGT_LOGIC_EN);
	}
#endif

	int_clear_ptr->reg.trace_int_clear = 1;

	return IRQ_HANDLED;
}

static int dmsspt_probe(struct platform_device *pdev)
{
	int irq, ret = 0;
	struct device_node *np = pdev->dev.of_node;
	struct resource *res;

	pr_err("%s start\n", __func__);

	if (!check_himntn(HIMNTN_DMSSPT)) {
		pr_err("%s, HIMNTN_DMSSPT is closed!\n", __func__);
		return 0;
	}

	if (!np) {
		dev_err(&pdev->dev, "%s %d, no dev node\n", __func__, __LINE__);
		ret = -ENODEV;
		goto err;
	}

	g_pt_devp = devm_kzalloc(&pdev->dev, sizeof(struct dmsspt_devices), GFP_KERNEL);
	if (!g_pt_devp) {
		dev_warn(&pdev->dev, "Kzalloc failed\n");
		ret = -ENOMEM;
		goto err;
	}

	g_pt_devp->pt_region = get_dmsspt_buffer_region();
	if(!g_pt_devp->pt_region.base) {
		dev_warn(&pdev->dev, "Trace buffer reserved failed\n");
		ret = -ENOMEM;
		goto err;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM , 0);
	g_pt_devp->dmss_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(g_pt_devp->dmss_base)) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "fail to ioremap !\n");
		goto err;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		ret = -ENXIO;
		dev_err(&pdev->dev, "fail to get irq !\n");
		goto err;
	}

	ret = devm_request_irq(&pdev->dev, (unsigned int)irq, dmsspt_interrupt,
		(unsigned long)0, "hisi_dmsspt", pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "fail to request irq !\n");
		goto err;
	}
	g_pt_devp->irq = irq;

	dmsspt_get_intlv();
	dmsspt_debugfs_init();
	spin_lock_init(&g_pt_devp->lock);
	pr_err("%s success\n", __func__);
err:
	return ret;
}

static const struct of_device_id hisi_dmsspt_of_match[] = {
	{.compatible = "hisilicon,kirin970-dmsspt" },/*lint !e785*/
	{ }/*lint !e785*/
};
MODULE_DEVICE_TABLE(of, hisi_dmsspt_of_match);

static struct platform_driver hisi_dmsspt_driver = {
	.probe = dmsspt_probe,
	.driver = {
		.name  = "hisi-dmsspt",
		.of_match_table = of_match_ptr(hisi_dmsspt_of_match),
	},/*lint !e785*/
};/*lint !e785*/

/*lint -e528 -esym(528,*)*/
module_platform_driver(hisi_dmsspt_driver);/*lint   !e64*/
/*lint -e528 +esym(528,*)*/

/*lint -e753 -esym(753,*)*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("dmss pattern trace driver");
MODULE_AUTHOR("t00375831 <tanquanwen@huawei.com>");
/*lint -e753 +esym(753,*)*/

#endif
