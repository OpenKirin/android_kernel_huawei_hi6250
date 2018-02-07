#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sysfs.h>

#include "ics_debug.h"

#define CLASS_NAME "ics_debug"
#define UNMASK_LPM3 (0)
#define UNMASK_IOMCU (1)
#define UNMASK_ISP (2)
#define UNMASK_IVP (3)
#define UNMASK_HIFI (4)

#define PERI_OFFSET_PEREN0 (0x000)
#define PERI_OFFSET_PERDIS0 (0x004)
#define PERI_OFFSET_PEREN6 (0x410)
#define PERI_OFFSET_PERDIS6 (0x414)
#define PERI_OFFSET_PERRSTEN4 (0x090)
#define PERI_OFFSET_PERRSTDIS4 (0x094)
#define PERI_OFFSET_CLKDIV18 (0x0F0)
#define PERI_OFFSET_PERPWREN (0x150)
#define PERI_OFFSET_PERPWRDIS (0x154)
#define PERI_OFFSET_ISOEN (0x144)
#define PERI_OFFSET_ISODIS (0x148)
#define PERI_OFFSET_CLKDIV5 (0x0bc)
#define PERI_OFFSET_CLKDIV8 (0x0c8)
#define PERI_OFFSET_CLKDIV15 (0x0e4)

#define PERI_ISOEN_ICS_ISO_EN (0x00000100)
#define PERI_ISODIS_ICS_ISO_UN (0x00000100)
#define PERI_CLKDIV8_SEL_ICS_PLL2 (0xf0004000)
#define PERI_CLKDIV8_SEL_ICS_PLL0 (0xf0002000)
#define PERI_PERPWREN_ICSPWREN_EN (0x00000100)
#define PERI_PERPWRDIS_ICS_PWR_DIS (0x00000100)
#define PERI_PERRSTDIS4_IP_RST_MEDIA2 (0x00000001)
#define PERI_CLKDIV15_SEL_FREQ_DIV4_ICS (0x7e000600)
#define PERI_PERRSTEN4_IP_RST_MEDIA_CRG (0x00000002)
#define PERI_CLKDIV8_SEL_VCODECBUS_PLL2 (0x000f0004)
#define PERI_CLKDIV8_SEL_VCODECBUS_PLL0 (0x000f0002)
#define PERI_CLKDIV15_SEL_FREQ_DIV3_ICS (0x7e000400)
#define PERI_CLKDIV15_SEL_FREQ_DIV2_ICS (0x7e000200)
#define PERI_CLKDIV18_SC_GT_CLK_ICS__EN (0x40000000)
#define PERI_PERRSTEN4_IP_RST_MEDIA2_EN (0x00000001)
#define PERI_CLKDIV18_SC_GT_CLK_VCODEBUS_EN (0x01000000)
#define PERI_CLKDIV5_SET_FREQ_DIV4_VCODECBUS (0x003f0003)
#define PERI_CLKDIV5_SET_FREQ_DIV5_VCODECBUS (0x003f0004)
#define PERI_CLKDIV5_SET_FREQ_DIV8_VCODECBUS (0x003f0007)
#define PERI_PEREN0_GT_CLK_VCODECBUS2DDRC_EN (0x00000020)
#define PERI_PERRSTDIS4_IP_RST_MEDIA2_CRG_EN (0x00000002)
#define PERI_PERDIS0_GT_CLK_VCODECBUS2DDRC_UN (0x00000020)
#define PERI_CLKDIV18_SC_GT_CLK_ICS_OPEN_AND_EN (0x40004000)
#define PERI_CLKDIV18_SC_GT_CLK_VCODEBUS_OPEN_AND_EN (0x01000100)
#define PERI_PEREN6_GT_CLK_CFGBUS_MEDIA_AND_MEDIA2_EN (0x00010200)
#define PERI_PERDIS6_GT_CLK_CFGBUS_MEDIA_AND_MEDIA2_EN (0x00010200)

#define MEDIA2_OFFSET_PEREN0 (0x000)
#define MEDIA2_OFFSET_PERDIS0 (0x004)
#define MEDIA2_OFFSET_PERRSTEN0 (0x030)
#define MEDIA2_OFFSET_PERRSTDIS0 (0x034)

#define MEDIA2_PEREN0_GT_CLK_VCODEBUS_EN (0x00000200)
#define MEDIA2_PERDIS0_GT_CLK_VCODEBUS_UN (0x00000200)
#define MEDIA2_PEREN0_GT_CLK_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_EN (0x00000007)
#define MEDIA2_PERDIS0_GT_CLK_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_UN (0x00000007)
#define MEDIA2_PERRSTEN0_IP_RST_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_EN (0x00000038)
#define MEDIA2_PERRSTDIS0_IP_RST_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_UN (0x00000038)

#define PMCTRL_OFFSET_NOC_POWER_IDLEREQ_0 (0x380)
#define PMCTRL_OFFSET_NOC_POWER_IDLEACK_0 (0x384)
#define PMCTRL_OFFSET_NOC_POWER_IDLE_0 (0x388)

#define PMCTRL_NOC_POWER_IDLE_0_NOC_ICS_POWER_IDLE (0x200)
#define PMCTRL_NOC_POWER_IDLE_0_NOC_ICS_POWER_IDLE_UN (0x0)
#define PMCTRL_NOC_POWER_IDLE_0_NOC_ICS_POWER_IDLE_EN (0x200)
#define PMCTRL_NOC_POWER_IDLEACK_0_NOC_ICS_POWER_IDLEACK (0x200)
#define PMCTRL_NOC_POWER_IDLEACK_0_NOC_ICS_POWER_IDLEACK_UN (0x0)
#define PMCTRL_NOC_POWER_IDLEACK_0_NOC_ICS_POWER_IDLEACK_EN (0x200)
#define PMCTRL_NOC_POWER_IDLEACK_0_NOC_VCODEC_BUS_POWER_IDLEACK (0x10)
#define PMCTRL_NOC_POWER_IDLE_0_NOC_VCODEC_BUS_POWER_IDLE_STATUS (0x10)
#define PMCTRL_NOC_POWER_IDLEACK_0_NOC_VCODEC_BUS_POWER_IDLEACK_UN (0x0)
#define PMCTRL_NOC_POWER_IDLEREQ_0_NOC_ICS_POWER_IDLEREQ_EN (0x02000000)
#define PMCTRL_NOC_POWER_IDLEACK_0_NOC_VCODEC_BUS_POWER_IDLEACK_EN (0x10)
#define PMCTRL_NOC_POWER_IDLE_0_NOC_VCODEC_BUS_POWER_IDLE_STATUS_UN (0x0)
#define PMCTRL_NOC_POWER_IDLE_0_NOC_VCODEC_BUS_POWER_IDLE_STATUS_EN (0x10)
#define PMCTRL_NOC_POWER_IDLEREQ_0_NOC_ICS_POWER_IDLEREQ_REQ_AND_EN (0x02000200)
#define PMCTRL_NOC_NOC_POWER_IDLEREQ_0_NOC_VCODEC_BUS_POWER_IDLEREQ_EN (0x00100000)
#define PMCTRL_NOC_NOC_POWER_IDLEREQ_0_NOC_VCODEC_BUS_POWER_IDLEREQ_REQ_AND_EN (0x00100010)

#define CONFIG_REG_VIRT_ADDR_OFFSET_ICS_VERSION (0x40)
#define CONFIG_REG_VIRT_ADDR_OFFSET_ICS_BASE_ADDR (0x28)

#define ICS_IRQ_OFFSET_ICS_IRQ_MASK_LPMCU_FINISH (0x20)
#define ICS_IRQ_OFFSET_ICS_IRQ_MASK_IOMCU_FINISH (0x30)
#define ICS_IRQ_OFFSET_ICS_IRQ_MASK_ISPCPU_FINISH (0x40)
#define ICS_IRQ_OFFSET_ICS_IRQ_MASK_IVP_FINISH (0x50)
#define ICS_IRQ_OFFSET_ICS_IRQ_MASK_HIFI_FINISH (0x60)

static struct class  *ics_class;
struct cambricon_ipu_private *ics_adapter;

static uint32_t setclkrate = 0;
static uint32_t ipuopen = 0;
static uint32_t ipurelease = 0;
static uint32_t resetproc = 0;
static uint32_t ipurstcrtenv = 0;
static uint32_t ipurstdstenv = 0;
static uint32_t pusetreg = 0;
static uint32_t pdsetreg = 0;
static uint32_t rdcorereg = 0;
static uint32_t wrcorereg = 0;
static uint32_t wrregvbusclk = 0;
static uint32_t wrregcnnclk = 0;
static uint32_t wrreglmt = 0;

struct ics_test_iomap_addr {
	void __iomem *pmctrl_io_addr;
	void __iomem *pctrl_io_addr;
	void __iomem *sctrl_io_addr;
	void __iomem *media_io_addr;
	void __iomem *peri_io_addr;
};
struct ics_test_iomap_addr ics_test_iomap_addr;

unsigned long smmu_ttbr0_bk = 0;

static ssize_t setclkrate_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret = 0;
	ret = snprintf(buf, PAGE_SIZE, "setclkrate:0x%x!\n", setclkrate);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo clockRate>setclkrate\n");
	return ret;
}

static ssize_t setclkrate_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	int ret = 0;

	if (sscanf(buf, "%d", &setclkrate) != 1)
		return -EINVAL;
	printk(KERN_DEBUG"[%s]: setclkrate_store begin\n", __FUNCTION__);
	ret = call_ipu_clock_set_rate(ics_adapter->clock, setclkrate);
	ics_adapter->clock_start_rate = setclkrate;
	if (ret) {
		printk(KERN_ERR"[%s]: call_ipu_clock_start failed\n", __FUNCTION__);
		return -EINVAL;
	}
	printk(KERN_DEBUG"[%s]: setclkrate_store end\n", __FUNCTION__);

	return (ssize_t)size;
}

static ssize_t ipuopen_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret =0;
	ret = snprintf(buf, PAGE_SIZE, "ipuopen:0x%x!\n", ipuopen);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>ipuopen\n");
	return ret;
}

static ssize_t ipuopen_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	int ret = 0;

	printk(KERN_DEBUG"[%s]: ipuopen_store begin\n", __FUNCTION__);

	ret = call_regulator_ip_vipu_enable();
	if (ret) {
		printk(KERN_ERR"[%s]: call_regulator_ip_vipu_enable failed\n", __FUNCTION__);
		return -EINVAL;
	}

	printk(KERN_DEBUG"[%s]: call_regulator_ip_vipu_enable ok\n", __FUNCTION__);

	call_ipu_clock_start(ics_adapter->clock, ics_adapter->clock_start_rate);
	if (ret) {
		printk(KERN_ERR"[%s]: IPU clock start failed\n", __FUNCTION__);
		return -EINVAL;
	}
	printk(KERN_DEBUG"[%s]: call_ipu_clock_start ok\n", __FUNCTION__);

	call_ipu_smmu_init(ics_adapter->smmu_ttbr0,
		(unsigned long)ics_adapter->smmu_rw_err_phy_addr, ics_adapter->feature_tree.smmu_port_select);
	printk(KERN_DEBUG"[%s]: call_ipu_smmu_init ok\n", __FUNCTION__);

	call_ipu_interrupt_init();
	printk(KERN_DEBUG"[%s]: call_ipu_interrupt_init ok\n", __FUNCTION__);

	printk(KERN_DEBUG"[%s]: ipuopen_store end\n", __FUNCTION__);

	return (ssize_t)size;
}

static ssize_t ipurelease_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret =0;
	ret = snprintf(buf, PAGE_SIZE, "ipurelease:0x%x!\n", ipurelease);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>ipurelease\n");
	return ret;
}

static ssize_t ipurelease_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	int ret = 0;

	printk(KERN_DEBUG"[%s]: ipurelease_store begin\n", __FUNCTION__);

	call_ipu_clock_set_rate(ics_adapter->clock, ics_adapter->clock_stop_rate);
	printk(KERN_DEBUG"[%s]: call_ipu_clock_set_rate ok\n", __FUNCTION__);

	call_ipu_clock_stop(ics_adapter->clock);
	printk(KERN_DEBUG"[%s]: call_ipu_clock_stop ok\n", __FUNCTION__);

	ret = call_regulator_ip_vipu_disable();
	if (ret) {
		printk(KERN_ERR"[%s]: No IPU device!\n", __FUNCTION__);
		return -EBUSY;
	}
	printk(KERN_DEBUG"[%s]: call_regulator_ip_vipu_disable ok\n", __FUNCTION__);
	ics_adapter->ipu_open_count = 0;

	printk(KERN_DEBUG"[%s]: ipurelease_store end\n", __FUNCTION__);
	return (ssize_t)size;
}

static ssize_t resetproc_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret =0;
	ret = snprintf(buf, PAGE_SIZE, "resetproc:0x%x!\n", resetproc);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>resetproc\n");
	return ret;
}

static ssize_t resetproc_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	printk(KERN_DEBUG"[%s]: begin\n", __FUNCTION__);
	call_ipu_reset_proc((unsigned int)ics_adapter->va_addr);
	printk(KERN_DEBUG"[%s]: end\n", __FUNCTION__);

	return (ssize_t)size;
}

static ssize_t ipurstcrtenv_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret =0;
	ret = snprintf(buf, PAGE_SIZE, "ipu reset create environment:0x%x!\n", ipurstcrtenv);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>ipurstcrtenv (no need to pu ipu)\n");
	return ret;
}

static ssize_t ipurstcrtenv_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	printk(KERN_DEBUG"[%s]: begin\n", __FUNCTION__);
	smmu_ttbr0_bk = ics_adapter->smmu_ttbr0;
	ics_adapter->smmu_ttbr0 = smmu_ttbr0_bk & 0xffffffff;
	printk(KERN_DEBUG"[%s]: end\n", __FUNCTION__);

	return (ssize_t)size;
}

static ssize_t ipurstdstenv_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret =0;
	ret = snprintf(buf, PAGE_SIZE, "ipu reset destroy environment:0x%x!\n", ipurstdstenv);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>ipurstcrtenv (no need to pu ipu)\n");
	return ret;
}

static ssize_t ipurstdstenv_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	printk(KERN_DEBUG"[%s]: begin\n", __FUNCTION__);
	ics_adapter->smmu_ttbr0 = smmu_ttbr0_bk;
	printk(KERN_DEBUG"[%s]: end\n", __FUNCTION__);

	return (ssize_t)size;
}

static ssize_t pusetreg_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret = 0;
	ret = snprintf(buf, PAGE_SIZE, "pusetreg:0x%x!\n", pusetreg);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>pusetreg\n");
	return ret;
}

static ssize_t pusetreg_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	unsigned int read_value;

	printk(KERN_DEBUG"[%s]: peri_io_addr:%pK, media2_io_addr:%pK, pmctrl_io_addr=%pK\n", __FUNCTION__,
		ics_adapter->peri_io_addr, ics_adapter->media2_io_addr, ics_adapter->pmctrl_io_addr);

	//set_pu_media2_subsys
	printk(KERN_DEBUG"[%s]:meidia module unrst\n",__FUNCTION__);
	iowrite32(PERI_PERRSTDIS4_IP_RST_MEDIA2_CRG_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PERRSTDIS4));

	printk(KERN_DEBUG"[%s]:meidia module clk enable\n",__FUNCTION__);
	iowrite32(PERI_PEREN6_GT_CLK_CFGBUS_MEDIA_AND_MEDIA2_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PEREN6));
	udelay(1);

	printk(KERN_DEBUG"[%s]:meidia module clk disable\n",__FUNCTION__);
	iowrite32(PERI_PERDIS6_GT_CLK_CFGBUS_MEDIA_AND_MEDIA2_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PERDIS6));
	udelay(1);
	printk(KERN_DEBUG"[%s]:meidia module unrst\n",__FUNCTION__);

	iowrite32(PERI_PERRSTDIS4_IP_RST_MEDIA2, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PERRSTDIS4));
	printk(KERN_DEBUG"[%s]:meidia module clk enable\n",__FUNCTION__);

	iowrite32(PERI_PEREN6_GT_CLK_CFGBUS_MEDIA_AND_MEDIA2_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PEREN6));

	//set_pu_vcodec
	printk(KERN_DEBUG"[%s]:vcodec module clk enable\n",__FUNCTION__);
	iowrite32(PERI_CLKDIV18_SC_GT_CLK_VCODEBUS_OPEN_AND_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV18));
	iowrite32(MEDIA2_PEREN0_GT_CLK_VCODEBUS_EN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PEREN0));
	iowrite32(PERI_PEREN0_GT_CLK_VCODECBUS2DDRC_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PEREN0));
	udelay(1);

	printk(KERN_DEBUG"[%s]:vcodec module clk disable\n",__FUNCTION__);
	iowrite32(MEDIA2_PERDIS0_GT_CLK_VCODEBUS_UN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PERDIS0));
	iowrite32(PERI_PERDIS0_GT_CLK_VCODECBUS2DDRC_UN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PERDIS0));
	udelay(1);

	printk(KERN_DEBUG"[%s]:vcodec module clk enable\n",__FUNCTION__);
	iowrite32(MEDIA2_PEREN0_GT_CLK_VCODEBUS_EN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PEREN0));
	iowrite32(PERI_PEREN0_GT_CLK_VCODECBUS2DDRC_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PEREN0));

	printk(KERN_DEBUG"[%s]:vcodec bus idle clear\n",__FUNCTION__);
	iowrite32(PMCTRL_NOC_NOC_POWER_IDLEREQ_0_NOC_VCODEC_BUS_POWER_IDLEREQ_EN, (void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLEREQ_0));
	udelay(1);

	read_value = ioread32((void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLEACK_0));
	if ((PMCTRL_NOC_POWER_IDLEACK_0_NOC_VCODEC_BUS_POWER_IDLEACK & PMCTRL_NOC_POWER_IDLEACK_0_NOC_VCODEC_BUS_POWER_IDLEACK_UN) != 0x0) {
		printk(KERN_ERR"[%s]: pu_codec:no expect power idleack value:%d!\n",
			__FUNCTION__ , read_value);
		return -EINVAL;
	}
	udelay(1);

	read_value = ioread32((void *)((unsigned long)ics_adapter->pmctrl_io_addr + PMCTRL_OFFSET_NOC_POWER_IDLE_0));
	if ((read_value & PMCTRL_NOC_POWER_IDLE_0_NOC_VCODEC_BUS_POWER_IDLE_STATUS) != PMCTRL_NOC_POWER_IDLE_0_NOC_VCODEC_BUS_POWER_IDLE_STATUS_UN) {
		printk(KERN_ERR"[%s]: pu_codec:no expect power idle value:%d!\n",
			__FUNCTION__ , read_value);
		return -EINVAL;
	}

	//set_pu_ics
	printk(KERN_DEBUG"[%s]:ipu module mtcmos on\n",__FUNCTION__);
	iowrite32(PERI_PERPWREN_ICSPWREN_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PERPWREN));
	udelay(100);

	printk(KERN_DEBUG"[%s]:ipu module clk enable\n",__FUNCTION__);
	iowrite32(PERI_CLKDIV18_SC_GT_CLK_ICS_OPEN_AND_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV18));
	iowrite32(MEDIA2_PEREN0_GT_CLK_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_EN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PEREN0));
	udelay(1);

	printk(KERN_DEBUG"[%s]:ipu module clk disable\n",__FUNCTION__);
	iowrite32(MEDIA2_PERDIS0_GT_CLK_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_UN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PERDIS0));
	udelay(1);

	printk(KERN_DEBUG"[%s]:ipu module iso disable\n",__FUNCTION__);
	iowrite32(PERI_ISODIS_ICS_ISO_UN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_ISODIS));
	printk(KERN_DEBUG"[%s]:ipu module unrst\n",__FUNCTION__);
	iowrite32(MEDIA2_PERRSTDIS0_IP_RST_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_UN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PERRSTDIS0));
	udelay(1);

	printk(KERN_DEBUG"[%s]:ipu module clk enable\n",__FUNCTION__);
	iowrite32(MEDIA2_PEREN0_GT_CLK_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_EN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PEREN0));
	printk(KERN_DEBUG"[%s]:ipu bus idle clear\n",__FUNCTION__);
	iowrite32(PMCTRL_NOC_POWER_IDLEREQ_0_NOC_ICS_POWER_IDLEREQ_EN, (void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLEREQ_0));
	udelay(1);

	read_value = ioread32((void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLEACK_0));

	if ((read_value & PMCTRL_NOC_POWER_IDLEACK_0_NOC_ICS_POWER_IDLEACK) != PMCTRL_NOC_POWER_IDLEACK_0_NOC_ICS_POWER_IDLEACK_UN) {
		printk(KERN_ERR"[%s]: pu_ics:no expect power idleack value:%d!\n",
			__FUNCTION__ , read_value);
		return -EINVAL;
	}
	udelay(1);

	read_value = ioread32((void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLE_0));

	if ((read_value & PMCTRL_NOC_POWER_IDLE_0_NOC_ICS_POWER_IDLE) != PMCTRL_NOC_POWER_IDLE_0_NOC_ICS_POWER_IDLE_UN) {
		printk(KERN_ERR"[%s]: pu_ics:no expect power idle value:%d!\n",
			__FUNCTION__ , read_value);
		return -EINVAL;
	}

	return (ssize_t)size;
}

static ssize_t pdsetreg_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret = 0;
	ret = snprintf(buf, PAGE_SIZE, "pdsetreg:0x%x!\n", pdsetreg);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>pdsetreg\n");
	return ret;
}

static ssize_t pdsetreg_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	unsigned int read_value;

	//set_pd_ics
	printk(KERN_DEBUG"[%s]:ipu bus idle set\n",__FUNCTION__);
	iowrite32(PMCTRL_NOC_POWER_IDLEREQ_0_NOC_ICS_POWER_IDLEREQ_REQ_AND_EN, (void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLEREQ_0));
	udelay(1);

	read_value = ioread32((void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLEACK_0));
	if((read_value & PMCTRL_NOC_POWER_IDLEACK_0_NOC_ICS_POWER_IDLEACK) != PMCTRL_NOC_POWER_IDLEACK_0_NOC_ICS_POWER_IDLEACK_EN) {
		printk(KERN_ERR"[%s]: pd_ics:no expect power idleack value:%d!\n",
			__FUNCTION__ , read_value);
		return -EINVAL;
	}
	udelay(1);

	read_value = ioread32((void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLE_0));
	if((read_value & PMCTRL_NOC_POWER_IDLE_0_NOC_ICS_POWER_IDLE) != PMCTRL_NOC_POWER_IDLE_0_NOC_ICS_POWER_IDLE_EN) {
		printk(KERN_ERR"[%s]: pd_ics:no expect power idle value:%d!\n",
			__FUNCTION__ , read_value);
		return -EINVAL;
	}

	printk(KERN_DEBUG"[%s]:ipu module clk disable\n",__FUNCTION__);
	iowrite32(MEDIA2_PERDIS0_GT_CLK_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_UN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PERDIS0));
	udelay(1);

	printk(KERN_DEBUG"[%s]:ipu module rst\n",__FUNCTION__);
	iowrite32(MEDIA2_PERRSTEN0_IP_RST_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_EN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PERRSTEN0));
	udelay(1);

	printk(KERN_DEBUG"[%s]:ipu module clk disable\n",__FUNCTION__);
	iowrite32(MEDIA2_PEREN0_GT_CLK_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_EN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PEREN0));
	udelay(1);

	printk(KERN_DEBUG"[%s]:ipu module clk disable\n",__FUNCTION__);
	iowrite32(MEDIA2_PERDIS0_GT_CLK_ICS_AND_NOC_ICS_AND_NOC_ICS_CFG_UN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PERDIS0));
	iowrite32(PERI_CLKDIV18_SC_GT_CLK_ICS__EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV18));

	printk(KERN_DEBUG"[%s]:ipu module iso\n",__FUNCTION__);
	iowrite32(PERI_ISOEN_ICS_ISO_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_ISOEN));

	printk(KERN_DEBUG"[%s]:ipu module mtcmos off\n",__FUNCTION__);
	iowrite32(PERI_PERPWRDIS_ICS_PWR_DIS, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PERPWRDIS));

	//set_pd_vcodec
	printk(KERN_DEBUG"[%s]:vcodec bus idle set\n",__FUNCTION__);
	iowrite32(PMCTRL_NOC_NOC_POWER_IDLEREQ_0_NOC_VCODEC_BUS_POWER_IDLEREQ_REQ_AND_EN, (void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLEREQ_0));
	udelay(1);

	read_value = ioread32((void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLEACK_0));
	if((read_value & PMCTRL_NOC_POWER_IDLEACK_0_NOC_VCODEC_BUS_POWER_IDLEACK) != PMCTRL_NOC_POWER_IDLEACK_0_NOC_VCODEC_BUS_POWER_IDLEACK_EN) {
		printk(KERN_ERR"[%s]: pd_codec:no expect power idleack value:%d!\n",
			__FUNCTION__ , read_value);
		return -EINVAL;
	}
	udelay(1);

	read_value = ioread32((void *)((unsigned long)ics_adapter->pmctrl_io_addr+ PMCTRL_OFFSET_NOC_POWER_IDLE_0));
	if((read_value & PMCTRL_NOC_POWER_IDLE_0_NOC_VCODEC_BUS_POWER_IDLE_STATUS) != PMCTRL_NOC_POWER_IDLE_0_NOC_VCODEC_BUS_POWER_IDLE_STATUS_EN) {
		printk(KERN_ERR"[%s]: pd_codec:no expect power idle value:%d!\n",
			__FUNCTION__ , read_value);
		return -EINVAL;
	}

	printk(KERN_DEBUG"[%s]:vcodec module clk disable\n",__FUNCTION__);
	iowrite32(MEDIA2_PERDIS0_GT_CLK_VCODEBUS_UN, (void *)((unsigned long)ics_adapter->media2_io_addr + MEDIA2_OFFSET_PERDIS0));
	iowrite32(PERI_PERDIS0_GT_CLK_VCODECBUS2DDRC_UN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PERDIS0));
	iowrite32(PERI_CLKDIV18_SC_GT_CLK_VCODEBUS_EN, (void *)((unsigned long)ics_adapter->peri_io_addr+ PERI_OFFSET_CLKDIV18));

	//set_pu_media2_subsys
	printk(KERN_DEBUG"[%s]:media module rst\n",__FUNCTION__);
	iowrite32(PERI_PERRSTEN4_IP_RST_MEDIA2_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PERRSTEN4));

	printk(KERN_DEBUG"[%s]:media module clk disable\n",__FUNCTION__);
	iowrite32(PERI_PERDIS6_GT_CLK_CFGBUS_MEDIA_AND_MEDIA2_EN, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PERDIS6));

	printk(KERN_DEBUG"[%s]:media module unrst\n",__FUNCTION__);
	iowrite32(PERI_PERRSTEN4_IP_RST_MEDIA_CRG, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_PERRSTEN4));
	return (ssize_t)size;
}

static ssize_t rdcorereg_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret =0;
	ret = snprintf(buf, PAGE_SIZE, "rdcorereg:0x%x!\n", rdcorereg);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>rdcorereg\n");
	return ret;
}

static ssize_t rdcorereg_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	unsigned int read_value;
	read_value = ioread32((void *)((unsigned long)ics_adapter->config_reg_virt_addr + CONFIG_REG_VIRT_ADDR_OFFSET_ICS_VERSION));

	if (read_value != 0x44400a7c && read_value != 0x4440031b) {
		printk(KERN_ERR"[%s]: read_value error : 0x%x!\n", __FUNCTION__ , read_value);
		return -EINVAL;
	}

	return (ssize_t)size;
}

static ssize_t wrcorereg_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret =0;
	ret = snprintf(buf, PAGE_SIZE, "wrcorereg:0x%x!\n", wrcorereg);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>wrcorereg\n");
	return ret;
}

static ssize_t wrcorereg_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	unsigned int read_value;

	if (sscanf(buf, "0x%x", &wrcorereg) != 1)
		return -EINVAL;

	iowrite32(wrcorereg, (void *)((unsigned long)ics_adapter->config_reg_virt_addr + CONFIG_REG_VIRT_ADDR_OFFSET_ICS_BASE_ADDR));
	udelay(1);

	read_value = ioread32((void *)((unsigned long)ics_adapter->config_reg_virt_addr + CONFIG_REG_VIRT_ADDR_OFFSET_ICS_BASE_ADDR));

	if (read_value != wrcorereg) {
		printk(KERN_ERR"[%s]: read_value error : 0x%x!\n", __FUNCTION__ , read_value);
		return -EINVAL;
	}

	return (ssize_t)size;
}

static ssize_t wrregvbusclk_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret = 0;
	ret = snprintf(buf, PAGE_SIZE, "wrregvbusclk:0x%x!\n", wrregvbusclk);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>wrregvbusclk\n");
	return ret;
}

static ssize_t wrregvbusclk_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	if (sscanf(buf, "%d", &wrregvbusclk) != 1)
		return -EINVAL;

	if (wrregvbusclk != 480000000 && wrregvbusclk != 322000000 && wrregvbusclk != 207500000) {
		return -EINVAL;
	}

	if (wrregvbusclk == 480000000) {
		iowrite32(PERI_CLKDIV8_SEL_VCODECBUS_PLL2, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV8));
		iowrite32(PERI_CLKDIV5_SET_FREQ_DIV4_VCODECBUS, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV5));
	} else if(wrregvbusclk == 322000000){
		iowrite32(PERI_CLKDIV8_SEL_VCODECBUS_PLL0, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV8));
		iowrite32(PERI_CLKDIV5_SET_FREQ_DIV5_VCODECBUS, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV5));
	} else if(wrregvbusclk == 207500000){
		iowrite32(PERI_CLKDIV8_SEL_VCODECBUS_PLL0, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV8));
		iowrite32(PERI_CLKDIV5_SET_FREQ_DIV8_VCODECBUS, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV5));
	}

	return (ssize_t)size;
}

static ssize_t wrregcnnclk_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret = 0;
	ret = snprintf(buf, PAGE_SIZE, "wrregcnnclk:0x%x!\n", wrregcnnclk);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>wrregcnnclk\n");
	return ret;
}

static ssize_t wrregcnnclk_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	if (sscanf(buf, "%d", &wrregcnnclk) != 1)
		return -EINVAL;

	if (wrregcnnclk != 960000000 && wrregvbusclk != 640000000 && wrregvbusclk != 415000000) {
		return -EINVAL;
	}

	if (wrregcnnclk == 960000000) {
		iowrite32(PERI_CLKDIV8_SEL_ICS_PLL2, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV8));
		iowrite32(PERI_CLKDIV15_SEL_FREQ_DIV2_ICS, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV15));
	} else if (wrregcnnclk == 640000000) {
		iowrite32(PERI_CLKDIV8_SEL_ICS_PLL2, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV8));
		iowrite32(PERI_CLKDIV15_SEL_FREQ_DIV3_ICS, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV15));
	} else if (wrregcnnclk == 415000000) {
		iowrite32(PERI_CLKDIV8_SEL_ICS_PLL0, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV8));
		iowrite32(PERI_CLKDIV15_SEL_FREQ_DIV4_ICS, (void *)((unsigned long)ics_adapter->peri_io_addr + PERI_OFFSET_CLKDIV15));
	}
	return (ssize_t)size;
}

static ssize_t wrreglmt_show(struct class *class, struct class_attribute *attr,
				 char *buf)
{
	int ret = 0;
	ret = snprintf(buf, PAGE_SIZE, "wrreglmt:0x%x!\n", wrreglmt);
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "usage: echo param>wrreglmt_es\n");
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "wrreglmt = 0xf6b ,ics core:400M,vcodec bus:207.5M\n");
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "wrreglmt = 0xaaa,ics core:640M,vcodec bus:480M\n");
	ret += snprintf(buf+ret, (PAGE_SIZE-ret), "wrreglmt = 0xdd5 ,ics core:830M,vcodec bus:480M\n");
	return ret;
}

static ssize_t wrreglmt_store(struct class *class, struct class_attribute *attr,
				 const char *buf, size_t size)
{
	void __iomem *axi;
	unsigned int read_value = 0;
	if (sscanf(buf, "0x%x", &wrreglmt) != 1)
		return -EINVAL;
	if (wrreglmt != 0xf6b &&
		wrreglmt != 0xaaa &&
		wrreglmt != 0xdd5 ) {
		printk(KERN_ERR"[%s]: limiter:input error value:%d!\n",
			__FUNCTION__ , wrreglmt);
		return -EINVAL;
	}
	axi = ioremap((unsigned long)0xe8950000,(unsigned long)0xfff);
	iowrite32(0x1,(void *)((unsigned long)axi+0x0c));
	udelay(10);
	iowrite32(wrreglmt,(void *)((unsigned long)axi+0x10));
	udelay(10);
	iowrite32(0x40, (void *)((unsigned long)axi+0x14));
	read_value = ioread32((void *)((unsigned long)axi+ 0x10));
	printk(KERN_DEBUG"[%s]:limit value = 0x%x",__FUNCTION__, read_value);
	iounmap(axi);
	return (ssize_t)size;
}

static const struct class_attribute ics_attrs[] = {
	__ATTR(setclkrate,				0644, setclkrate_show,			setclkrate_store),
	__ATTR(ipuopen, 				0644, ipuopen_show, 			ipuopen_store),
	__ATTR(ipurelease,				0644, ipurelease_show,			ipurelease_store),
	__ATTR(resetproc,				0644, resetproc_show,			resetproc_store),
	__ATTR(ipurstcrtenv,			0644, ipurstcrtenv_show,		ipurstcrtenv_store),
	__ATTR(ipurstdstenv,			0644, ipurstdstenv_show,		ipurstdstenv_store),
	__ATTR(pusetreg,				0644, pusetreg_show,			pusetreg_store),
	__ATTR(pdsetreg,				0644, pdsetreg_show,			pdsetreg_store),
	__ATTR(rdcorereg,				0644, rdcorereg_show,			rdcorereg_store),
	__ATTR(wrcorereg,				0644, wrcorereg_show,			wrcorereg_store),
	__ATTR(wrregvbusclk,			0644, wrregvbusclk_show,		wrregvbusclk_store),
	__ATTR(wrregcnnclk, 			0644, wrregcnnclk_show, 		wrregcnnclk_store),
	__ATTR(wrreglmt,				0644, wrreglmt_show,			wrreglmt_store),
};

static int create_ics_attrs(struct class *class)
{
	unsigned int i = 0;
	int ret = 0;

	for (i = 0; i < (sizeof(ics_attrs)/sizeof(struct class_attribute)); i++) {
		ret = class_create_file(class, &ics_attrs[i]);
		if (ret < 0) {
			break;
		}
	}

	return ret;
}

static void remove_ics_attrs(struct class *class)
{
	unsigned int i = 0;
	for (i = 0; i < (sizeof(ics_attrs)/sizeof(struct class_attribute)); i++) {
		class_remove_file(class, &ics_attrs[i]);
	}
}

static int __init ics_debug_init(void) {
	int ret = 0;
	printk(KERN_ERR"[%s:%d], test begin\n", __FUNCTION__, __LINE__);

	ics_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(ics_class)) {
		printk(KERN_ERR"[%s:%d], class create error!\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	ret = create_ics_attrs(ics_class);
	if(ret < 0) {
		class_destroy(ics_class);
		printk(KERN_ERR"[%s:%d], create_ics_attrs error!\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	ics_adapter = get_ipu_adapter();

	return 0;

}

static void __exit ics_debug_exit(void) {
	printk(KERN_ERR"[%s:%d], test end\n", __FUNCTION__, __LINE__);
	remove_ics_attrs(ics_class);
	class_destroy(ics_class);

	return;
}

module_init(ics_debug_init);
module_exit(ics_debug_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon");

