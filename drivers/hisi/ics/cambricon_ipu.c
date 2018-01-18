/*
 * Generic driver for the cambricon ipu device.
 *
 * Copyright (C) 2016 Cambricon Limited
 *
 * Licensed under the GPL v2 or later.
 */

#include <linux/mfd/hisi_pmic.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/compiler.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/pm_runtime.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/input.h>
#include <linux/wakelock.h>
#include <linux/clk.h>
#include <linux/hwspinlock.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include "ipu_smmu_drv.h"
#include <linux/kfifo.h>
#include <linux/irq.h>
#include "ipu_clock.h"
#include <asm/io.h>
#include <linux/ion.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/time.h>


#include "cambricon_ipu.h"

#define COMP_CAMBRICON_IPU_DRV_NAME "hisilicon,cambricon-ipu"

/* ipu base address */
#define IPU_NAME	"ipu"
#define UNUSED_PARAMETER(x) ((void)(x))

/* configure registers info */
#define IPU_CONF_REG_BASE	(IPU_BASE_ADDRESS + 0x00000000)
#define IPU_CONF_REG_SIZE	(0x00100000)
/* instruction RAM info */
#define IPU_INST_RAM_BASE	(IPU_BASE_ADDRESS + 0x00100000)
#define IPU_INST_RAM_SIZE	(0x00100000)

/* ipu configure register offset */
#define IPU_START_REG (0x18)	/* IPU start up reg */
#define IPU_STATUS_REG (0x20)	/* IPU payload finish status reg */
#define IPU_BASE_ADDR_REG (0x28)	/* IPU access external DDR address */
#define IPU_SRAM_CTRL_REG (0x30)	/* IPU internal SRAM configure reg */

/* reserved DDR memory info */
#define DMA_BUFFER_START (0x20c00000)
#define DMA_BUFFER_SIZE	 (500 * 1024 * 1024)

/* Configuration Registers Operations: ioctl */
#define MAGIC_NUM	100
#define RDCONFIG_DWORD	_IOR(MAGIC_NUM, 3, unsigned int)
#define WRCONFIG_DWORD	_IOW(MAGIC_NUM, 6, unsigned int*)
#define SETCONFIG_MAP	_IOW(MAGIC_NUM, 7, unsigned int*)
#define SETCONFIG_UNMAP	_IOW(MAGIC_NUM, 8, unsigned int*)
#define IN_TASKQUEUE	_IOW(MAGIC_NUM, 9, unsigned int*)
#define SETCONFIG_RESET_STATISTIC  _IOW(MAGIC_NUM, 10, unsigned int)
#define SETCONFIG_REPORT_STATISTIC _IOW(MAGIC_NUM, 11, unsigned int*)
#define SETCONFIG_UPDATE_PTE	   _IOW(MAGIC_NUM, 12, unsigned int)
#define SETCONFIG_RESET_VIRT_ADDR  _IOW(MAGIC_NUM, 13, unsigned int)
#define IPU_IOC_MAXNR	(13)
#define ICS_IRQ_UNMASK_NO_SECURITY (0x00000000)
#define ICS_IRQ_MASK_NO_SECURITY (0x00000001)
#define ICS_IRQ_CLEAR_IRQ_NS (0x00000001)
#define ICS_SMMU_WR_ERR_BUFF_LEN (128)
#define ICS_NOC_BUS_CONFIG_QOS_TYPE (0x1)
#define ICS_NOC_BUS_CONFIG_FACTOR (0x0)
#define ICS_NOC_BUS_CONFIG_SATURATION (0x0)
#define CONFIG_NOC_POWER_IDLEREQ_0 (0x02000200)
#define CONFIG_NOC_POWER_IDLEACK_0_BIT9 (0X200)
#define CONFIG_NOC_POWER_IDLE_0_BIT9 (0X200)
#define CONFIG_PCTRL_PERI_STAT3_BIT22 (0x400000)
#define CONFIG_MEDIA2_REG_PERDIS0_ICS (0x7)
#define CONFIG_MEDIA2_REG_PEREN0_ICS (0x7)
#define CONFIG_MEDIA2_REG_PERRSTEN0_ICS (0x38)
#define CONFIG_SC_GT_CLK_ICS_DIS (0x40000000)
#define CONFIG_SC_GT_CLK_ICS_EN (0x40004000)
#define CONFIG_NOC_ICS_POWER_IDLEREQ_DIS (0x02000000)
#define TASKQUEUE_SIZE (4096)

/*check whether the parameter is effective*/
#define CHECK_TASK_TYPE(taskType) \
({	\
	unsigned int __ret = 0;	\
	if((taskType-IPU_TASK_ENUM_START)>(IPU_TASK_ENUM_END - IPU_TASK_ENUM_START - 1))		\
		__ret = 1;	\
	__ret;	\
})	\


/* global variables */
static unsigned int ipu_major = 0;
static unsigned int ipu_minor = 0;
struct cambricon_ipu_private *adapter = NULL;
static struct class *dev_class = NULL;
extern char ipu_wq_enable_str[];

/* define fasync queue */
static struct fasync_struct *ipu_async_queue;

/* FIFO_TaskElements which holds ipu tasks */
static DECLARE_KFIFO_PTR(FIFO_TaskElements, taskElement);
/* wait queue holds sync task which make user process sleep */
wait_queue_head_t sync_wq;

/* ipu char device ops declaration */
static int ipu_open(struct inode *inode, struct file *filp);
static int ipu_release(struct inode *inode, struct file *filp);
static ssize_t ipu_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static ssize_t ipu_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static int ipu_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#else
static long ipu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif
static long ipu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static loff_t ipu_llseek(struct file *filp, loff_t off, int whence);
static int ipu_fasync(int fd, struct file *filp, int on);
int regulator_ip_vipu_enable(void);
int regulator_ip_vipu_disable(void);

void setOffchipInstAddr(unsigned long addr) {
	/* write offchip inst addr to instram */
	uint32_t inst_addr = (uint32_t) (addr + 384);
	inst_addr = (inst_addr | 0x80000000) >> 1;
	iowrite8((inst_addr&0x000000ff)    , (void *)((unsigned long)adapter->inst_ram_virt_addr + 53));
	iowrite8((inst_addr&0x0000ff00)>>8 , (void *)((unsigned long)adapter->inst_ram_virt_addr + 54));
	iowrite8((inst_addr&0x00ff0000)>>16, (void *)((unsigned long)adapter->inst_ram_virt_addr + 55));
	iowrite8((inst_addr&0xff000000)>>24, (void *)((unsigned long)adapter->inst_ram_virt_addr + 56));
}

void start_ipu(void) {
	if(1!= ioread32((void *)((unsigned long)adapter->config_reg_virt_addr + IPU_START_REG))){
		taskElement head;
		if (kfifo_peek(&FIFO_TaskElements, &head)) {
			setOffchipInstAddr(head.offchipInstAddr);
			/* update pte */
			(void )ipu_smmu_pte_update();
			/* start ipu */
			iowrite32(0, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_START_REG));
			iowrite32(0, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_STATUS_REG));
			iowrite32(1, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_START_REG));
		}
	}
}
/* global variable declaration */
static const struct file_operations ipu_fops = {
	.owner			= THIS_MODULE,/*lint !e64*/
	.open			= ipu_open,
	.release		= ipu_release,
	.read			= ipu_read,
	.write			= ipu_write,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
	.ioctl			= ipu_ioctl,
#else
	.unlocked_ioctl = ipu_ioctl,
#endif
	.llseek 		= ipu_llseek,
	.fasync			= ipu_fasync,
};/*lint !e785*/
/* ipu platform device */
static struct platform_device cambricon_ipu_device = {
	.name	= "cambricon-ipu",
	.id	= -1,
};/*lint !e785*/

bool ipu_get_irq_offset (struct device *dev)
{
	struct irq_reg_offset *offset = &adapter->irq_reg_offset;
	struct device_node *node = of_find_node_by_name(dev->of_node, "ics_irq");
	if(!node) {
		printk(KERN_ERR"[%s]: find ics_irq node error\n", __func__);
		return false;
	}
	memset(offset, 0, sizeof(*offset));// coverity[secure_coding]
	of_property_read_u32(node, "ics-irq-base-addr", &offset->ics_irq_base_addr);
	of_property_read_u32(node, "ics-irq-mask-ns", &offset->ics_irq_mask_ns);
	of_property_read_u32(node, "ics-irq-clr-ns", &offset->ics_irq_clr_ns);

	return true;
}

bool ipu_get_reset_offset (struct device *dev)
{
	struct ics_noc_bus_reg_offset *ics_noc_bus = &adapter->ics_noc_bus_reg_offset;
	struct pmctrl_reg_offset *pmctrl = &adapter->pmctrl_reg_offset;
	struct pctrl_reg_offset *pctrl = &adapter->pctrl_reg_offset;
	struct media2_reg_offset *media2 = &adapter->media2_reg_offset;
	struct peri_reg_offset *peri = &adapter->peri_reg_offset;
	struct device_node *node = of_find_node_by_name(dev->of_node, "ics_noc_bus");

	if(!node) {
		printk(KERN_ERR"[%s]: find ics_noc_bus node error\n", __func__);
		return false;
	}
	memset(ics_noc_bus, 0, sizeof(*ics_noc_bus));
	of_property_read_u32(node, "base-addr", &ics_noc_bus->base_addr);
	of_property_read_u32(node, "qos-type", &ics_noc_bus->qos_type);
	of_property_read_u32(node, "factor", &ics_noc_bus->factor);
	of_property_read_u32(node, "saturation", &ics_noc_bus->saturation);

	node = of_find_node_by_name(dev->of_node, "pmctrl");
	if(!node) {
		printk(KERN_ERR"[%s]: find pmctrl node error\n", __func__);
		return false;
	}
	memset(pmctrl, 0, sizeof(*pmctrl));
	of_property_read_u32(node, "base-addr", &pmctrl->base_addr);
	of_property_read_u32(node, "noc-power-idle-req", &pmctrl->noc_power_idle_req);
	of_property_read_u32(node, "noc-power-idle-ack", &pmctrl->noc_power_idle_ack);
	of_property_read_u32(node, "noc-power-idle-stat", &pmctrl->noc_power_idle_stat);

	node = of_find_node_by_name(dev->of_node, "pctrl");
	if(!node) {
		printk(KERN_ERR"[%s]: find pctrl node error\n", __func__);
		return false;
	}
	memset(pctrl, 0, sizeof(*pctrl));
	of_property_read_u32(node, "base-addr", &pctrl->base_addr);
	of_property_read_u32(node, "peri-stat3", &pctrl->peri_stat3);

	node = of_find_node_by_name(dev->of_node, "media2");
	if(!node) {
		printk(KERN_ERR"[%s]: find media2 node error\n", __func__);
		return false;
	}
	memset(media2, 0, sizeof(*media2));
	of_property_read_u32(node, "base-addr", &media2->base_addr);
	of_property_read_u32(node, "peren0", &media2->peren0);
	of_property_read_u32(node, "perdis0", &media2->perdis0);
	of_property_read_u32(node, "perrsten0", &media2->perrsten0);
	of_property_read_u32(node, "perrstdis0", &media2->perrstdis0);

	node = of_find_node_by_name(dev->of_node, "peri");
	if(!node) {
		printk(KERN_ERR"[%s]: find peri node error\n", __func__);
		return false;
	}
	memset(peri, 0, sizeof(*peri));// coverity[secure_coding]
	of_property_read_u32(node, "base-addr", &peri->base_addr);
	of_property_read_u32(node, "clkdiv18", &peri->clkdiv18);

	return true;
}

bool ipu_get_feature_tree (struct device *dev)
{
	/* get platform information */
	const char *str;
	int ret = of_property_read_string(dev->of_node, "ics-platform", &str);

	if (ret) {
		printk(KERN_ERR"[%s]: fatal err, of_property_read_string fail\n", __func__);
		return false;
	}

	memset(&adapter->feature_tree, 0, sizeof(adapter->feature_tree));// coverity[secure_coding]
	if ((strncmp(str, "kirin970_es", strlen(str)) == 0)) {
		adapter->feature_tree.finish_irq_expand_ns = false;
		adapter->feature_tree.finish_irq_expand_p = false;
		adapter->feature_tree.finish_irq_expand_s = false;
		adapter->feature_tree.finish_irq_to_hifi = false;
		adapter->feature_tree.finish_irq_to_ivp = false;
		adapter->feature_tree.finish_irq_to_isp = false;
		adapter->feature_tree.finish_irq_to_lpm3 = false;
		adapter->feature_tree.finish_irq_to_iocmu = false;
		adapter->feature_tree.smmu_port_select = false;
		adapter->feature_tree.ipu_reset_when_in_error = false;
	} else if ((strncmp(str, "kirin970_cs", strlen(str)) == 0)) {
		adapter->feature_tree.finish_irq_expand_ns = true;
		adapter->feature_tree.finish_irq_expand_p = true;
		adapter->feature_tree.finish_irq_expand_s = true;
		adapter->feature_tree.finish_irq_to_hifi = true;
		adapter->feature_tree.finish_irq_to_ivp = true;
		adapter->feature_tree.finish_irq_to_isp = true;
		adapter->feature_tree.finish_irq_to_lpm3 = true;
		adapter->feature_tree.finish_irq_to_iocmu = true;
		adapter->feature_tree.smmu_port_select = false;
		adapter->feature_tree.ipu_reset_when_in_error = true;
	} else {
		printk(KERN_ERR"[%s]: fatal err, platform is unsupported\n", __func__);
		return false;
	}

	printk(KERN_DEBUG"[%s]: the platform is %s\n", __func__, str);

	return true;
}

int regulator_ip_vipu_enable(void)
{
	int ret;

	ret = regulator_is_enabled(adapter->vipu_ip);
	if (ret) {
		printk(KERN_ERR"[%s]:regulator_is_enabled: %d\n", __func__, ret);
		return -EBUSY;
	}

	ret = regulator_enable(adapter->vipu_ip);
	if (0 != ret) {
		printk(KERN_ERR"[%s]:Failed to enable: %d\n", __func__, ret);
		return ret;
	}
	printk(KERN_DEBUG"[%s]:success to enable: %d\n", __func__, ret);
	return 0;
}

int regulator_ip_vipu_disable(void)
{
	int ret;

	ret = regulator_disable(adapter->vipu_ip);
	if (ret != 0) {
		printk(KERN_ERR"[%s]:Failed to disable: %d\n", __func__, ret);
		return ret;
	}
	printk(KERN_DEBUG"[%s]:success to disable: %d\n", __func__, ret);
	return 0;
}

void ipu_reset_ioremap_addr(void)
{
	adapter->noc_bus_io_addr = ioremap((unsigned long)adapter->ics_noc_bus_reg_offset.base_addr, (unsigned long)0xff);
	adapter->pmctrl_io_addr = ioremap((unsigned long)adapter->pmctrl_reg_offset.base_addr, (unsigned long)0xfff);
	adapter->pctrl_io_addr = ioremap((unsigned long)adapter->pctrl_reg_offset.base_addr, (unsigned long)0xff);
	adapter->media2_io_addr = ioremap((unsigned long)adapter->media2_reg_offset.base_addr, (unsigned long)0xff);
	adapter->peri_io_addr = ioremap((unsigned long)adapter->peri_reg_offset.base_addr, (unsigned long)0xff);
}

void ipu_reset_unremap_addr(void)
{
	if (adapter->noc_bus_io_addr) {
		iounmap(adapter->noc_bus_io_addr);
	}

	if (adapter->pmctrl_io_addr) {
		iounmap(adapter->pmctrl_io_addr);
	}

	if (adapter->pctrl_io_addr) {
		iounmap(adapter->pctrl_io_addr);
	}

	if (adapter->media2_io_addr) {
		iounmap(adapter->media2_io_addr);
	}

	if (adapter->peri_io_addr) {
		iounmap(adapter->peri_io_addr);
	}
}

#ifdef CAMBRICON_IPU_IRQ
/* ipu interrupt init, including update ipu status, clear interrupt, and unmask interrupt */
void ipu_interrupt_init(void)
{
	unsigned long irq_io_addr = (unsigned long)adapter->ics_irq_io_addr;

	/* clear ipu status to unfinished */
	iowrite32(0, (void *)((unsigned long)adapter->config_reg_virt_addr + IPU_STATUS_REG));

	/* clear ns interrupt */
	if (adapter->feature_tree.finish_irq_expand_ns) {
		iowrite32(ICS_IRQ_CLEAR_IRQ_NS, (void *)(irq_io_addr + adapter->irq_reg_offset.ics_irq_clr_ns));
	}

	/* unmask interrupt */
	iowrite32(ICS_IRQ_UNMASK_NO_SECURITY, (void *)(irq_io_addr + adapter->irq_reg_offset.ics_irq_mask_ns));
}

/* to mask ipu interrupt and will not receive it */
void ipu_interrupt_deinit(void)
{
	iowrite32(ICS_IRQ_MASK_NO_SECURITY,
		(void *)((unsigned long)adapter->ics_irq_io_addr + adapter->irq_reg_offset.ics_irq_mask_ns));
}
#endif

int ipu_reset_proc(unsigned int addr)
{
	unsigned int loop_cnt = 0;
	int noc_power_idle_ack;
	int noc_power_idle_stat;
	int noc_peri_status;
	int noc_idle = 0;
	static bool ipu_reset_lock = false;

	if (ipu_reset_lock) {
		printk(KERN_DEBUG"[%s]: can not re-enter\n", __func__);
		return 0;
	}

	ipu_reset_lock = true;

#ifdef IPU_SMMU_ENABLE
	ipu_smmu_deinit();
#endif

#ifdef CAMBRICON_IPU_IRQ
	ipu_interrupt_deinit();
#endif

	printk(KERN_DEBUG"[%s]: ipu_recover start!\n", __func__);

	ipu_clock_set_rate(adapter->clock, adapter->clock_stop_rate);
	printk(KERN_DEBUG"[%s]: ipu_clock_set_rate ok\n", __FUNCTION__);

	/* config NOC register, to enter stream limited mode, stream is limited to 0 */
	iowrite32(ICS_NOC_BUS_CONFIG_QOS_TYPE, (void *)((unsigned long)adapter->noc_bus_io_addr + adapter->ics_noc_bus_reg_offset.qos_type));
	iowrite32(ICS_NOC_BUS_CONFIG_FACTOR, (void *)((unsigned long)adapter->noc_bus_io_addr + adapter->ics_noc_bus_reg_offset.factor));
	iowrite32(ICS_NOC_BUS_CONFIG_SATURATION, (void *)((unsigned long)adapter->noc_bus_io_addr + adapter->ics_noc_bus_reg_offset.saturation));

	ipu_smmu_override_prefetch_addr(addr);

	udelay(10);

	iowrite32(CONFIG_NOC_POWER_IDLEREQ_0, (void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_req));

	while(!noc_idle) {
		if (loop_cnt > 10) {
			printk(KERN_ERR"[%s]: loop timeout", __func__);
			break;
		}
		noc_power_idle_ack = ioread32((void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_ack)) & CONFIG_NOC_POWER_IDLEACK_0_BIT9;
		noc_power_idle_stat = ioread32((void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_stat)) & CONFIG_NOC_POWER_IDLE_0_BIT9;
		noc_peri_status = ioread32((void *)((unsigned long)adapter->pctrl_io_addr + adapter->pctrl_reg_offset.peri_stat3)) & CONFIG_PCTRL_PERI_STAT3_BIT22;
		noc_idle = noc_power_idle_ack && noc_power_idle_stat && noc_peri_status;
		printk(KERN_DEBUG"[%s]: noc_power_idle_ack:%d, noc_power_idle_stat:%d, noc_peri_status:%d\n",
			__func__, noc_power_idle_ack, noc_power_idle_stat, noc_peri_status);

		udelay(1);
		loop_cnt++;
	}

	iowrite32(CONFIG_MEDIA2_REG_PERDIS0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.perdis0));
	iowrite32(CONFIG_MEDIA2_REG_PERRSTEN0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.perrsten0));
	iowrite32(CONFIG_MEDIA2_REG_PEREN0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.peren0));
	iowrite32(CONFIG_MEDIA2_REG_PERDIS0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.perdis0));
	iowrite32(CONFIG_SC_GT_CLK_ICS_DIS, (void *)((unsigned long)adapter->peri_io_addr + adapter->peri_reg_offset.clkdiv18));

	/* restart ipu */
	iowrite32(CONFIG_SC_GT_CLK_ICS_EN, (void *)((unsigned long)adapter->peri_io_addr + adapter->peri_reg_offset.clkdiv18));
	iowrite32(CONFIG_MEDIA2_REG_PEREN0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.peren0));
	iowrite32(CONFIG_MEDIA2_REG_PERDIS0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.perdis0));
	iowrite32(CONFIG_MEDIA2_REG_PERRSTEN0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.perrstdis0));
	iowrite32(CONFIG_MEDIA2_REG_PEREN0_ICS, (void *)((unsigned long)adapter->media2_io_addr + adapter->media2_reg_offset.peren0));
	iowrite32(CONFIG_NOC_ICS_POWER_IDLEREQ_DIS, (void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_req));

	noc_power_idle_ack = ioread32((void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_ack)) & CONFIG_NOC_POWER_IDLEACK_0_BIT9;
	noc_power_idle_stat = ioread32((void *)((unsigned long)adapter->pmctrl_io_addr + adapter->pmctrl_reg_offset.noc_power_idle_stat)) & CONFIG_NOC_POWER_IDLE_0_BIT9;

	printk(KERN_DEBUG"[%s]: noc_power_idle_ack:%d, noc_power_idle_stat:%d\n", __func__, noc_power_idle_ack, noc_power_idle_stat);

	/* unmask irq*/
	iowrite32(ICS_IRQ_UNMASK_NO_SECURITY, (void *)((unsigned long)(adapter->ics_irq_io_addr) + adapter->irq_reg_offset.ics_irq_mask_ns));

	ipu_clock_set_rate(adapter->clock, adapter->clock_start_rate);

#ifdef IPU_SMMU_ENABLE
	ipu_smmu_init(adapter->smmu_ttbr0,
		(unsigned long)adapter->smmu_rw_err_phy_addr, adapter->feature_tree.smmu_port_select);
#endif

#ifdef CAMBRICON_IPU_IRQ
	ipu_interrupt_init();
#endif

	printk(KERN_ERR"[%s]: ipu_recover finished!\n", __func__);
	ipu_reset_lock = false;
	return 0;
}

#ifdef CAMBRICON_IPU_IRQ

static bool cambricon_ipu_workqueue(void *dev) {
	taskElement Element;
	struct timeval tv1, tv2;
	unsigned int elapse;
	struct cambricon_ipu_private * pdev = container_of((struct cdev *)dev, struct cambricon_ipu_private, cdev);
	/* ipu irq comes means the first element of FIFO_TaskElements is finished
		and this task must be compute task,
		so remove it from queue. */

	if (kfifo_get(&FIFO_TaskElements, &Element)) {
		printk(KERN_DEBUG "[%s]: Get task type is %d\n", __func__, Element.taskType);
		up(&(pdev->task_fifo_sem));
		if (CHECK_TASK_TYPE(Element.taskType)) {
			printk(KERN_ERR "[%s]: Get error type of first task from kfifo\n", __func__);
			return false;
		}
	} else {
		printk(KERN_ERR "[%s]: Get element from kfifo faild.\n", __func__);
		return false;
	}

	if (!kfifo_is_empty(&FIFO_TaskElements)) {
		if (kfifo_peek(&FIFO_TaskElements, &Element)) {/*to check next taskType*/

			if (CHECK_TASK_TYPE(Element.taskType)) {
				printk(KERN_ERR "[%s]: Get error type of next task from kfifo\n", __func__);
				return false;
			}
			do_gettimeofday(&tv1);
			/* handle sync task, until queue head is compute task or queue is empty */
			while (IPU_SYNC_TASK == Element.taskType) {
				/* wakeup user process */
				if (NULL != Element.ptaskFlag)
					*Element.ptaskFlag = IPU_TASK_FLAG_SYNC_DONE;
				wake_up_interruptible(&sync_wq);
				if (!kfifo_get(&FIFO_TaskElements,&Element))
					printk(KERN_ERR "[%s]: FIFO_TaskElements is empty\n", __func__);
				up(&(pdev->task_fifo_sem));
				/*to check next taskType*/
				if (!kfifo_peek(&FIFO_TaskElements, &Element)) {
					printk(KERN_DEBUG "[%s]: FIFO_TaskElements has been taken empty.\n", __func__);
					break;
				}

				if (CHECK_TASK_TYPE(Element.taskType)) {
					printk(KERN_ERR "[%s]: Get error sync task type from kfifo\n", __func__);
					return false;
				}
			}
			do_gettimeofday(&tv2);
			elapse = (tv2.tv_sec - tv1.tv_sec)*1000 + (tv2.tv_usec - tv1.tv_usec)/1000;
			printk(KERN_ERR"%s elapse=%d\n",__func__ , elapse);
		} else {
			printk(KERN_ERR "[%s]: Peek element from kfifo failed while fifo is not empty\n", __func__);
			return false;
		}
	}
	return true;
}

/*
 * A very tiny interrupt handler. It runs with interrupts disabled,
 * but there is possibility of conflicting with operating register
 * at the same time in two different CPUs. So we need to serialize
 * accesses to the chip with the ipu_lock spinlock.
 */
static irqreturn_t ipu_interrupt_handler(int irq, void *dev)
{
	unsigned long reg_virt_addr = (unsigned long)adapter->config_reg_virt_addr;

	(void)adapter->chrdev;

	if (dev == NULL) {
		printk(KERN_ERR"[%s]: no device\n", __func__);
		return IRQ_HANDLED;
	}

	if (ipu_smmu_interrupt_handler()) {

		if (adapter->feature_tree.ipu_reset_when_in_error) {
			ipu_reset_proc((unsigned int)adapter->va_addr);
		}
	} else {
		/* clear ipu finished status */
		iowrite32(0, (void *)(reg_virt_addr + IPU_STATUS_REG));
		if (adapter->feature_tree.finish_irq_expand_ns) {
			/* clear ipu finished status non-security irq, which is copied from ipu finished irq in non-security mode if unmask*/
			// fixme: add security and protected mode here in furture
			iowrite32(ICS_IRQ_CLEAR_IRQ_NS, (void *)((unsigned long)adapter->ics_irq_io_addr + adapter->irq_reg_offset.ics_irq_clr_ns));
		}
		if (!strncmp(ipu_wq_enable_str, "1", 1))  {
			if (!cambricon_ipu_workqueue(dev))
				return IRQ_HANDLED;
		}
	}

	if (ipu_async_queue) {
		printk(KERN_ERR"[%s]: ipu_async_queue", __func__);
		/* to awake process in user-mode(in libipu.so) */
		kill_fasync(&ipu_async_queue, SIGIO, POLL_IN);
	}

	/*Start next task*/
	if (!kfifo_is_empty(&FIFO_TaskElements)) {
		/* handle compute task */
		start_ipu();
	}

	printk(KERN_DEBUG"[%s]: IRQ_HANDLED is finished, irq=%d, IRQ_HANDLED=%d\n", __func__, irq, IRQ_HANDLED);

	return IRQ_HANDLED;
}
#endif

/* ipu char device ops function implementation, inode:node of file, filp: pointer of file */
static int ipu_open(struct inode *inode, struct file *filp)
{
	struct cambricon_ipu_private *dev;
	int ret;

	if (!inode || !filp){
		printk(KERN_ERR"[%s]: invalid input parameter !\n", __func__);
		return -EINVAL;
	}

	if (adapter->ipu_open_count >= 1) {
		printk(KERN_ERR"[%s]: IPU device has already been opened !\n", __func__);
		return -EBUSY;
	} else {
		printk(KERN_DEBUG"[%s]: IPU device open success!\n", __func__);
		adapter->ipu_open_count++;
	}

	/*ICS power on*/
	ret = regulator_ip_vipu_enable();
	if (ret) {
		printk(KERN_ERR"[%s]: regulator_ip_vipu_enable failed\n", __func__);
		return ret;
	}

	/* start ipu clock */
	ret = ipu_clock_start(adapter->clock, adapter->clock_start_rate);
	if (ret) {
		printk(KERN_ERR"[%s]: IPU clock start failed\n", __func__);
		return ret;
	}

#ifdef IPU_SMMU_ENABLE
	ipu_smmu_init(adapter->smmu_ttbr0,
		(unsigned long)adapter->smmu_rw_err_phy_addr, adapter->feature_tree.smmu_port_select);
#endif

#ifdef CAMBRICON_IPU_IRQ
	ipu_interrupt_init();
#endif

	/* find offset of "cdev" in "struct cambricon_ipu_private" */
	dev = container_of(inode->i_cdev, struct cambricon_ipu_private, cdev);/*lint !e826*/

	/* save dev info to "filp->private_data", it will be used later */
	filp->private_data = dev;
	printk(KERN_DEBUG"[%s]: success\n", __func__);

	return 0;
}

/* ipu device release and power down */
static int ipu_release(struct inode *inode, struct file *filp)
{
	struct cambricon_ipu_private *dev;
	int ret;
	struct irq_data *desc = irq_get_irq_data(adapter->irq);
	struct irq_common_data *d = desc ? desc->common : NULL;

	UNUSED_PARAMETER(inode);

	if (!inode || !filp){
		printk(KERN_ERR"[%s]: input parameter inode or filp is invalid !\n", __func__);
		return -EINVAL;
	}

	dev = filp->private_data;

	if (dev) {
		if (!strncmp(ipu_wq_enable_str, "1", 1))  {
			kfifo_reset(&FIFO_TaskElements);
			if (d && (d->state_use_accessors & IRQD_IRQ_DISABLED)) {
				enable_irq(adapter->irq);
			}
		} else {
			/* free ipu fasync resource */
			ipu_fasync(-1, filp, 0);
			printk(KERN_DEBUG"[%s]: ipu_fasync fail, %pK\n", __func__, filp);
		}
	} else {
		printk(KERN_ERR"[%s]: No IPU device!\n", __func__);
		return -EINVAL;
	}

	#ifdef IPU_SMMU_ENABLE
	ipu_smmu_deinit();
	#endif

	#ifdef CAMBRICON_IPU_IRQ
	ipu_interrupt_deinit();
	#endif

	/* Set clock rate to default(which should always be generated from PPLL0) */
	ipu_clock_set_rate(adapter->clock, adapter->clock_stop_rate);

	/* STOP ipu clock */
	ipu_clock_stop(adapter->clock);

	/*ICS power down*/
	ret = regulator_ip_vipu_disable();
	if (ret) {
		printk(KERN_ERR"[%s]: No IPU device!\n", __func__);
		return -EBUSY;
	}

	adapter->ipu_open_count = 0;

	printk(KERN_DEBUG"[%s]: ipu release succeed\n", __func__);

	return 0;
}

static int input_check(unsigned int cmd, unsigned long arg)
{
	bool ret;

	printk(KERN_DEBUG"[%s]: IOCTL cmd=%d/0x%x, arg ptr=%ld\n", __func__, cmd, cmd, arg);

	if (0 == adapter->ipu_open_count) {
		printk(KERN_ERR"[%s]: receiving IOCTL attack when ipu is not open, ignore\n", __func__);
		return -EINVAL;
	}

	if (cmd == RDCONFIG_DWORD) {
		if (arg > 0xfffff) {
			printk(KERN_ERR"[%s]: input parameter arg for read offset is invalid !\n", __func__);
			return -EINVAL;
		}
		return 0;
	}
	if (cmd == WRCONFIG_DWORD) {
		if (!arg) {
			printk(KERN_ERR"[%s]: input parameter arg for write is invalid !\n", __func__);
			return -EINVAL;
		}
		return 0;
	}

	ret = (cmd == SETCONFIG_MAP || cmd == SETCONFIG_REPORT_STATISTIC || cmd == SETCONFIG_UNMAP);
	if (ret) {
		if (!arg) {
			printk(KERN_ERR"[%s]: input parameter arg is invalid, cmd is %d\n", __func__ , cmd);
			return -EINVAL;
		}
		return 0;
	}

	if (cmd == SETCONFIG_RESET_VIRT_ADDR) {
		if (arg > 0x7fffffff) {
			printk(KERN_ERR"[%s]: input parameter arg for set reset virt_addr is invalid !\n", __func__);
			return -EINVAL;
		}
		return 0;
	}

	ret = (cmd == SETCONFIG_RESET_STATISTIC ||  cmd == SETCONFIG_UPDATE_PTE || cmd == IN_TASKQUEUE);

	if (ret) {
		return 0;
	}

	printk(KERN_ERR"[%s]: WRONG CMD 0x%x, ignore\n", __func__, cmd);
	return -EINVAL;
}

static int input_filp_check(struct file *filp, unsigned int cmd)
{
	struct cambricon_ipu_private *dev;
	if (!filp){
		printk(KERN_ERR"[%s]: input parameter filp is invalid !\n", __func__);
		return -EINVAL;
	}
	dev = filp->private_data;
	if (!dev) {
		printk(KERN_ERR"[%s]: No IPU device!\n", __func__);
		return -1;
	}

	if (!dev->config_reg_virt_addr) {
		printk(KERN_ERR"[%s]: reg_virt_addr is invalid!\n", __func__);
		return -1;
	}
	/* check whether cmd is valid */
	if (_IOC_TYPE(cmd) != MAGIC_NUM) {
		printk(KERN_ERR"[%s]: cmd is invalid!(not a MAGIC_NUM)\n", __func__);
		return -1;
	}

	if (_IOC_NR(cmd) > IPU_IOC_MAXNR) {
		printk(KERN_ERR"[%s]: cmd is invalid!( > IPU_IOC_MAXNR)\n", __func__);
		return -1;
	}

	printk(KERN_ERR"[%s]:cmd = %d", __func__, _IOC_NR(cmd));
	if(IN_TASKQUEUE == cmd){
		if(down_interruptible(&(dev->task_fifo_sem))){
			printk(KERN_DEBUG "[%s]Request task_fifo_sem failed\n", __func__);
			return -1;
		}
	}
	return 0;
}

static int ipu_process_workqueue(struct file *filp, unsigned long arg, long *ret_value)
{
	struct cambricon_ipu_private *dev;
	taskElement element;
	taskFlag_t taskFlag = IPU_TASK_FLAG_NONE;
	dev = filp->private_data;
	/*Caution : Already obtain task_fifo_sem !!*/
	disable_irq(adapter->irq);

	//printk(KERN_DEBUG "[%s]: taskType=%d taskId=%ll ptaskFlag=0x%pK \n", __func__, ((taskElement *)arg)->taskType,((taskElement *)arg)->taskId,((taskElement *)arg)->ptaskFlag);
	if (!copy_from_user(&element, (void __user *)arg, sizeof(element))) {
		element.ptaskFlag = NULL;/*Ensure point correct*/
		if (kfifo_is_empty(&FIFO_TaskElements)) {
			/*First task is SYNC while fifo is empty should return user thread at once*/
			if (element.taskType != IPU_SYNC_TASK){
				kfifo_put(&FIFO_TaskElements, element);
				start_ipu();
			} else {
				up(&(dev->task_fifo_sem));
			}
		} else {
			if (unlikely(IPU_SYNC_TASK == element.taskType)) {
				printk(KERN_DEBUG "[%s]: IPU task is IPU_SYNC_TASK\n", __func__);
				/*Set sync flag as wake up condition*/
				taskFlag = IPU_TASK_FLAG_SYNC_WAITING;
				element.ptaskFlag = &taskFlag;
				kfifo_put(&FIFO_TaskElements, element);
				enable_irq(adapter->irq);
				up(&(dev->config_reg_sem));
				/* sync task sleeping */
				wait_event_interruptible(sync_wq, IPU_TASK_FLAG_SYNC_DONE == taskFlag);
				return -1;
			} else {
				kfifo_put(&FIFO_TaskElements, element);
				printk(KERN_DEBUG "[%s]: FIFO_TaskElements inqueue success\n", __func__);
			}
		}
	} else {
		printk(KERN_DEBUG "[%s]: Copy_from_user failed\n", __func__);
		up(&(dev->task_fifo_sem));
		*ret_value = -EINVAL;
	}

	enable_irq(adapter->irq);
	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static int ipu_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#else
static long ipu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	long ret_value = 0;
	unsigned long reg_virt_addr;
	unsigned int read_value;
	struct cambricon_ipu_private *dev;
	struct sg_table *table;
	struct map_data map_data;
	struct ion_handle *hdl;
	struct smmu_statistic statistic;

	if (input_check(cmd, arg)) {
		printk(KERN_ERR"[%s]: input parameter arg is invalid !\n", __func__);
		return -EINVAL;
	}

	if (input_filp_check(filp, cmd)) {
		printk(KERN_ERR"[%s]: input parameter filp is invalid !\n", __func__);
		return -EINVAL;
	}

	dev = filp->private_data;
	reg_virt_addr = (unsigned long)dev->config_reg_virt_addr;

	/* a "mutex signal" to guarantee only one process is working */
	if (down_interruptible(&dev->config_reg_sem))
		return -ERESTARTSYS;

	if (cmd == RDCONFIG_DWORD) {
		/*unsigned int read_value;*/
		read_value = ioread32((void *)(reg_virt_addr + arg));
		printk(KERN_DEBUG"[%s]: Read CONFIG REG dword offset 0x%x, value is 0x%x\n", __func__, (unsigned int)arg, (unsigned int)read_value);
		ret_value = (long)read_value;
	}

	if (cmd == WRCONFIG_DWORD) {
		unsigned int *offset = (unsigned int *)arg;
		unsigned int *data = (unsigned int *)((unsigned long *)arg + 1);
		printk(KERN_DEBUG"[%s]: Write CONFIG REG dword offset 0x%x, value is 0x%x\n", __func__, (unsigned int) *offset, (unsigned int)*data);
		iowrite32(*data, (void *)(reg_virt_addr + *offset));
	}

	if (cmd == SETCONFIG_MAP) {
		if (copy_from_user(&map_data, (void __user *)arg, sizeof(map_data))) {
			printk(KERN_ERR"[%s]: copy arg failed!\n", __func__);
			ret_value = -EFAULT;
		} else {
			printk(KERN_DEBUG"[%s]: SETCONFIG_MAP copy from user SUCC\n", __func__);

			hdl = ion_import_dma_buf(ipu_ion_client, map_data.share_fd);// coverity[UNINIT]
			if (IS_ERR_OR_NULL(hdl)) {
				printk(KERN_ERR"[%s]: SETCONFIG_MAP hdl is error, which is: %pK\n", __func__, hdl);
				up(&(dev->config_reg_sem));
				return -EFAULT;
			}

			table = ion_sg_table(ipu_ion_client, hdl);
			if (IS_ERR_OR_NULL(table)) {
				printk(KERN_ERR"[%s]: SETCONFIG_MAP table is error, which is: %pK\n", __func__, table);
				up(&(dev->config_reg_sem));
				return -EFAULT;
			}

			ret_value = ipu_smmu_map(table->sgl, map_data.format.iova_size, &(map_data.format));
			if (copy_to_user((void __user *)arg, &map_data, sizeof(map_data))) {
				printk(KERN_ERR"[%s]: copy_to_user failed!\n", __func__);
				ret_value = -EFAULT;
			}

			ion_free(ipu_ion_client, hdl);
		}
	}

	if (cmd == SETCONFIG_UNMAP) {
		if (copy_from_user(&map_data, (void __user *)arg, sizeof(map_data))) {
			printk(KERN_ERR"[%s]: copy_from_user failed!\n", __func__);
			ret_value = -EFAULT;
		}
		printk(KERN_DEBUG"[%s]: SETCONFIG_UNMAP copy from user SUCC\n", __func__);
		ret_value = ipu_smmu_unmap(&(map_data.format));
	}
	if (cmd == IN_TASKQUEUE) {
		if (ipu_process_workqueue(filp, arg, &ret_value))
			goto _OUT;
	}
	if (cmd == SETCONFIG_RESET_VIRT_ADDR) {
		printk(KERN_DEBUG"[%s]: RECOVERY addr is 0x%x\n", __func__, (unsigned int)arg);
		adapter->va_addr = arg;
	}

	if (cmd == SETCONFIG_RESET_STATISTIC) {
		ret_value = ipu_smmu_reset_statistic();
	}

	if (cmd == SETCONFIG_REPORT_STATISTIC) {
		ret_value = ipu_smmu_report_statistic(&statistic);
		if (copy_to_user((void __user *)arg, &statistic, sizeof(statistic))) {
			printk(KERN_ERR"[%s]: ipu_smmu_report_statistic copy_to_user failed!\n", __func__);
			ret_value = -EFAULT;
		}
	}

	if (cmd == SETCONFIG_UPDATE_PTE) {
		ipu_smmu_pte_update();
	}

	up(&(dev->config_reg_sem));
_OUT:
	return ret_value;
}

/* this func is unused now, its utility is: CPU read info from IPU(it is invalid because hardward is unsupported) */
static ssize_t ipu_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned int ret_value = 0;
	struct cambricon_ipu_private *dev;
	if (!filp || !buf) {
		printk(KERN_ERR"[%s]: input parameter filp or buf is invalid!\n", __func__);
		return -EINVAL;
	}
	dev = filp->private_data;

	if (!dev) {
		printk(KERN_ERR"[%s]: No IPU device!\n", __func__);
		return -EINVAL;
	}

	if (down_interruptible(&dev->inst_ram_sem))
		return -ERESTARTSYS;

	if (count == 0 || count == 0xFFFFFFFF) {
		printk(KERN_ERR"[%s]: input parameter count is invalid!\n", __func__);
		goto out;
	}

	if (*f_pos > dev->inst_ram_size || *f_pos < 0) {
		printk(KERN_ERR"[%s]: Read file position out of range!\n", __func__);
		goto out;
	}
	if ((unsigned long)*f_pos + count > dev->inst_ram_size)
		count = dev->inst_ram_size - (unsigned long)*f_pos;

	if (dev->inst_ram_virt_addr) {
		if (copy_to_user(buf, (void*)((unsigned long)dev->inst_ram_virt_addr + (unsigned long)(*f_pos)), count)) {
			printk(KERN_ERR"[%s]: Copy data to user failed!\n", __func__);
			ret_value = EFAULT;
			goto out;
		}
	}

	ret_value = (unsigned int)count;
	printk(KERN_DEBUG"[%s]: read %d bytes\n", __func__, ret_value);

out:
	up(&(dev->inst_ram_sem));
	return ret_value;
}

/* CPU write inst to IPU SRAM */
static ssize_t ipu_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned int ret_value = 0;
	struct cambricon_ipu_private *dev;
	if (!filp || !buf) {
		printk(KERN_ERR"[%s]: input parameter filp or buf is invalid !\n", __func__);
		return -EINVAL;
	}
	dev = filp->private_data;

	if (!dev) {
		printk(KERN_ERR"[%s]: No IPU device!\n", __func__);
		return -EINVAL;
	}

	if(down_interruptible(&dev->inst_ram_sem))
		return -ERESTARTSYS;

	if (count == 0 || count == 0xFFFFFFFF) {
		printk(KERN_ERR"[%s]:input parameter count is invalid!\n", __func__);
		goto out;
	}
	if (*f_pos > dev->inst_ram_size || *f_pos < 0) {
		printk(KERN_ERR"[%s]: Write file position out of range!\n", __func__);
		goto out;
	}
	if((unsigned long)*f_pos + count > dev->inst_ram_size)
		count = dev->inst_ram_size - (unsigned long)*f_pos;

	if (dev->inst_ram_virt_addr) {
		if(copy_from_user((void *)((unsigned long)dev->inst_ram_virt_addr + (unsigned long)(*f_pos)), buf, count)) {
			printk(KERN_ERR"[%s]: Copy data from user failed!\n", __func__);
			ret_value = EFAULT;
			goto out;
		}
	}

	ret_value = (unsigned int)count;
	printk(KERN_DEBUG"[%s]: write %d bytes\n", __func__, ret_value);
out:
	/* release down_interruptible */
	up(&dev->inst_ram_sem);
	return ret_value;
}

/* set file position in IPU where CPU to write */
static loff_t ipu_llseek(struct file *filp, loff_t off, int whence)
{
	struct cambricon_ipu_private *dev;
	loff_t pos;
	if (!filp) {
		printk(KERN_ERR"[%s]: input parameter filp is invalid !\n", __func__);
		return -EINVAL;
	}
	dev = filp->private_data;
	if (!dev) {
		printk(KERN_ERR"[%s]: No IPU device!\n", __func__);
		return -EINVAL;
	}

	if(down_interruptible(&dev->llseek_sem))
		return -ERESTARTSYS;

	pos = filp->f_pos;
	switch (whence) {
		/* Set f_pos */
	case 0:
		pos = off;
		break;
		/* Move f_pos forward */
	case 1:
		pos += off;
		break;
		/* More */
	case 2:
		break;
		/* Default Operation */
	default:
		return -EINVAL;
	}

	if ((pos > dev->inst_ram_size) || (pos < 0)) {
		printk(KERN_ERR"[%s]: Move file position out of range!", __func__);
		return -EINVAL;
	}

	printk(KERN_DEBUG"[%s]: Move file position to %d", __func__, (unsigned int)pos);
	up(&dev->llseek_sem);
	return filp->f_pos = pos;
}

/* async notify user space application */
static int ipu_fasync(int fd, struct file *filp, int on)
{
	if (fasync_helper(fd, filp, on, &ipu_async_queue) >= 0)
		return 0;
	else
		return -EIO;
}

/* Allocate ipu chrdev region ("dev/ipu") */
static int cambricon_ipu_chrdev_region(dev_t *chrdev)
{
	int err = -1;

	/* Allocate char device region */
	if (ipu_major) {
		*chrdev = MKDEV(ipu_major, ipu_minor);
		err = register_chrdev_region(*chrdev, 1, IPU_NAME);
	} else {
		err = alloc_chrdev_region(chrdev, 0, 1, IPU_NAME);
	}
	if (err < 0) {
		printk(KERN_ERR"[%s]:alloc_chrdev_region fail!\n", __func__);
		return err;
	}

	ipu_major = MAJOR(*chrdev);
	ipu_minor = MINOR(*chrdev);

	return err;
}

/* probe() function for platform driver */
static int cambricon_ipu_probe(struct platform_device *pdev)
{
	int err;
	dev_t chrdev = 0;
	struct device *temp = NULL;
	struct resource *res, *res_cfg;
	unsigned long  size;

	dev_info(&pdev->dev, "[%s]: Initializing IPU device!\n", __func__);

	/* Allocate char device region */
	err = cambricon_ipu_chrdev_region(&chrdev);
	if (err < 0) {
		dev_err(&pdev->dev, "[%s]: Failed to allocate device ID for IPU!\n", __func__);
		goto fail;
	}

	//fixme: not add re-call restrict here, should make sure why cambricon_ipu_probe will be called twice?
	/* create and init queue FIFO_TaskElements */
	err = kfifo_alloc(&FIFO_TaskElements, TASKQUEUE_SIZE, GFP_KERNEL);
	if(err != 0) {
		printk(KERN_DEBUG"[%s]: Failed to allocate kfifo for FIFO_TaskElements!\n", __func__);
		return err;
	}
	/*init fifo of task elements*/
	printk(KERN_INFO "[%s]:IPU FIFO_TaskElements init success, queue size: %u\n", __func__, kfifo_size(&FIFO_TaskElements));

	/* init wait queue sync_wq */
	init_waitqueue_head(&sync_wq);

	/* Allocate cambricon_ipu_private struct and asigned to global variable adapter */
	adapter = kmalloc(sizeof(struct cambricon_ipu_private), GFP_KERNEL);
	if (!adapter) {
		err = -ENOMEM;
		dev_err(&pdev->dev, "[%s]: Failed to allocate memory for struct ipu!\n", __func__);
		goto unregister;
	}

	/* Initialize cambricon_ipu_private struct */
	memset(adapter, 0, sizeof(struct cambricon_ipu_private));
	adapter->name = IPU_NAME;
	adapter->irq = (unsigned int)platform_get_irq_byname(pdev, "ipu_dma_irq");
	dev_info(&pdev->dev, "[%s]: platform_get_irq_byname:%d\n", __func__, adapter->irq);

	/* get regulator */
	adapter->vipu_ip = devm_regulator_get(&pdev->dev, "vipu");
	if (IS_ERR(adapter->vipu_ip)) {
		dev_err(&pdev->dev, "[%s]:Couldn't get regulator ip! \n", __func__);
		goto cleanup;
	}
	dev_info(&pdev->dev, "[%s]:Get regulator ip succuss! dev-num-resouce:%d\n",
		__func__, pdev->num_resources);

	/* init mutex signals (down_interrupt/up) */
	sema_init(&(adapter->config_reg_sem), 1);
	sema_init(&(adapter->inst_ram_sem), 1);
	sema_init(&(adapter->llseek_sem), 1);
	sema_init(&(adapter->task_fifo_sem), TASKQUEUE_SIZE);

	/* ipu instruction ram resource, third input para is for the number of dts */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);

	if (res == NULL) {
		dev_err(&pdev->dev, "[%s]: failed to get instruction resource\n", __func__);
		err = -ENXIO;
		goto cleanup;
	}

	size = resource_size(res);
	adapter->inst_ram_phys_addr = res->start;
	adapter->inst_ram_size = (unsigned int)size;

	/* the region of request must be unused */
	adapter->inst_mem = request_mem_region(res->start, size, pdev->name);
	if (adapter->inst_mem == NULL) {
		dev_err(&pdev->dev, "[%s]: failed to get instruction memory region\n", __func__);
		err = -ENOENT;
		goto cleanup;
	}

	adapter->inst_ram_virt_addr = ioremap(res->start, size);
	if (adapter->inst_ram_virt_addr == NULL) {
		dev_err(&pdev->dev, "[%s]: ioremap() of instruction resource failed\n", __func__);
		err = -ENXIO;
		goto release_res_inst;
	}

	/* ipu configure registers resource */
	res_cfg = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res_cfg == NULL) {
		dev_err(&pdev->dev, "[%s]: failed to get configure registers resource\n", __func__);
		err = -ENXIO;
		goto unmap_ram;
	}

	size = resource_size(res_cfg);
	adapter->config_reg_phys_addr = res_cfg->start;
	adapter->config_reg_length = (unsigned int)size;
	adapter->cfg_mem = request_mem_region(res_cfg->start, size, pdev->name);
	if (adapter->cfg_mem == NULL) {
		dev_err(&pdev->dev, "[%s]: failed to get configure registers memory region\n", __func__);
		err = -ENOENT;
		goto unmap_ram;
	}

	adapter->config_reg_virt_addr = ioremap(res_cfg->start, size);
	if (adapter->config_reg_virt_addr == NULL) {
		dev_err(&pdev->dev, "[%s]: ioremap() of configure registers resource failed\n", __func__);
		err = -ENXIO;
		goto release_res_cfg;
	}

#ifdef CAMBRICON_IPU_IRQ
	/* request ipu irq */
	if (request_irq(adapter->irq, ipu_interrupt_handler, (unsigned long)IRQF_TRIGGER_HIGH, IPU_NAME, &(adapter->cdev))) {
		dev_err(&pdev->dev, "[%s]: IPU Require IRQ failed!\n", __func__);
		err = -EIO;
		goto free_irq;
	}
	dev_info(&pdev->dev, "[%s]: IPU Require IRQ Succeeded\n", __func__);
#endif

	/* Add ipu char device to system, udev can auto detect */
	cdev_init(&(adapter->cdev), &ipu_fops);
	adapter->cdev.owner = THIS_MODULE;/*lint !e64*/
	adapter->cdev.ops = &ipu_fops;
	err = cdev_add(&(adapter->cdev), chrdev, 1);
	if (err) {
		dev_err(&pdev->dev, "[%s]: Failed to Add IPU to system!\n", __func__);
		goto unmap_reg;
	}

	/* Create ipu class under /sys/class */
	dev_class = class_create(THIS_MODULE, IPU_NAME);/*lint !e64*/
	if (IS_ERR(dev_class)) {
		err = (int)PTR_ERR(dev_class);
		dev_err(&pdev->dev, "[%s]: Failed to create ipu class!\n", __func__);
		goto destroy_cdev;
	}
	/* Register ipu device in sysfs, and this will cause udev to create corresponding device node */
	temp = device_create(dev_class, NULL, chrdev, NULL, "%s", IPU_NAME);
	if (IS_ERR(temp)) {
		err = (int)PTR_ERR(temp);
		dev_err(&pdev->dev, "[%s]: Failed to mount IPU to /dev/ipu!\n", __func__);
		goto destroy_class;
	}

	if (!ipu_get_feature_tree(&pdev->dev)) {
		dev_err(&pdev->dev, "[%s]: fatal err, unknown feature tree\n", __func__);
		goto destroy_class;
	}

	if (!ipu_smmu_master_get_offset(&pdev->dev)) {
		dev_err(&pdev->dev, "[%s]: fatal err, ipu_smmu_master is unsupported\n", __func__);
		goto destroy_class;
	}

	if (!ipu_smmu_common_get_offset(&pdev->dev)) {
		dev_err(&pdev->dev, "[%s]: fatal err, ipu_smmu_common is unsupported\n", __func__);
		goto destroy_class;
	}

#ifdef CAMBRICON_IPU_IRQ
	if (!ipu_get_irq_offset(&pdev->dev)) {
		dev_err(&pdev->dev, "[%s]: fatal err, ipu irq is unsupported\n", __func__);
		goto destroy_class;
	}

	/* ioremap irq reg addr */
	adapter->ics_irq_io_addr = ioremap((unsigned long)adapter->irq_reg_offset.ics_irq_base_addr, (unsigned long)0xff);

#endif

	/* ioremap smmu reg addr */
	ipu_smmu_ioremap();

	/* ioremap reg addr for reset */
	if (adapter->feature_tree.ipu_reset_when_in_error) {
		if (!ipu_get_reset_offset(&pdev->dev)) {
			dev_err(&pdev->dev, "[%s]: fatal err, error reset reg offset\n", __func__);
			goto destroy_class;
		}

		ipu_reset_ioremap_addr();
	}

	/* init ipu clock */
	if (ipu_clock_init(&pdev->dev, &adapter->clock, &adapter->clock_start_rate, &adapter->clock_stop_rate)) {
		dev_err(&pdev->dev, "[%s]: Failed to init ipu clock\n", __func__);
		goto exit_error;
	}

#ifdef IPU_SMMU_ENABLE
	adapter->smmu_ttbr0 = ipu_get_smmu_base_phy(&pdev->dev);

	adapter->smmu_rw_err_phy_addr = kmalloc(ICS_SMMU_WR_ERR_BUFF_LEN, GFP_KERNEL);
	if (!adapter->smmu_rw_err_phy_addr) {
		err = -ENOMEM;
		dev_err(&pdev->dev,
			"[%s]: Failed to allocate memory for smmu read and write phy addr in error case\n", __func__);
		goto exit_error;
	}
#endif

	dev_info(&pdev->dev, "[%s]: Succeeded to initialize ipu device.\n", __func__);

	return 0;

exit_error:
#ifdef CAMBRICON_IPU_IRQ
free_irq:
	free_irq(adapter->irq, &(adapter->cdev));
#endif
destroy_class:
	class_destroy(dev_class);
destroy_cdev:
	cdev_del(&(adapter->cdev));
unmap_reg:
	iounmap(adapter->config_reg_virt_addr);
release_res_cfg:
	release_mem_region((unsigned long)res_cfg->start, size);
unmap_ram:
	iounmap(adapter->inst_ram_virt_addr);
release_res_inst:
	release_mem_region((unsigned long)res->start, size);
cleanup:
	kfree(adapter);
unregister:
	unregister_chrdev_region(chrdev, 1);
	kfifo_free(&FIFO_TaskElements);
fail:
	return err;
}

/* remove() function for platform driver */
static int __exit cambricon_ipu_remove(struct platform_device *pdev)
{
	dev_t chrdev;

	UNUSED_PARAMETER(pdev);

	/* device has two dev number, i.e. ipu_major, ipu_minor */
	chrdev = MKDEV(ipu_major, ipu_minor);

	dev_info(&pdev->dev, "[%s]: Destroying IPU device!\n", __func__);

	/* Destroy ipu class */
	if (dev_class) {
		device_destroy(dev_class, chrdev);
		class_destroy(dev_class);
	}

	/* Destroy ipu */
	if (adapter) {
		cdev_del(&(adapter->cdev));
		kfifo_free(&FIFO_TaskElements);
		iounmap(adapter->config_reg_virt_addr);
		iounmap(adapter->inst_ram_virt_addr);
		iounmap(adapter->ics_irq_io_addr);

		ipu_smmu_ioremap();

		if (adapter->feature_tree.ipu_reset_when_in_error) {
			ipu_reset_unremap_addr();
		}

		release_mem_region(adapter->inst_mem->start, resource_size(adapter->inst_mem));
		release_mem_region(adapter->cfg_mem->start, resource_size(adapter->cfg_mem));

#ifdef CAMBRICON_IPU_IRQ
		free_irq(adapter->irq, &(adapter->cdev));
#endif

#ifdef IPU_SMMU_ENABLE
		kfree(adapter->smmu_rw_err_phy_addr);
#endif

		kfree(adapter);
	}

	/* Unregister chrdev region */
	unregister_chrdev_region(chrdev, 1);

	dev_info(&pdev->dev, "[%s]: Succeeded to destroying IPU device.\n", __func__);

	return 0;
}
/*lint -e785*/
static const struct of_device_id cambricon_ipu_match_table[] = {
	{ .compatible = COMP_CAMBRICON_IPU_DRV_NAME, },
	{},
};

/* to find key word "cambricon-ipu" in dts, if failed, not load driver */
MODULE_DEVICE_TABLE(of, cambricon_ipu_match_table);
/* ipu platform drive */
static struct platform_driver cambricon_ipu_driver = {
	.driver	= {
		.name = "cambricon-ipu",
		.owner = THIS_MODULE,/*lint !e64*/
		.of_match_table = of_match_ptr(cambricon_ipu_match_table),
	},
	.probe	= cambricon_ipu_probe,
	.remove	= cambricon_ipu_remove,
};
/*lint +e785*/
/* ipu platform device and driver register */
static int __init cambricon_ipu_init(void)
{
	int ret;

	printk(KERN_DEBUG"[%s]: platform device and driver register!\n", __func__);
	ret = platform_driver_register(&cambricon_ipu_driver);/*lint !e64*/
	if (ret)
		return ret;

	ret = platform_device_register(&cambricon_ipu_device);
	if (ret)
		goto fail_platform_device;

	return 0;

fail_platform_device:
	platform_driver_unregister(&cambricon_ipu_driver);
	return ret;
}

/* ipu platform device and driver unregister */
static void __exit cambricon_ipu_exit(void)
{
	platform_device_unregister(&cambricon_ipu_device);
	platform_driver_unregister(&cambricon_ipu_driver);
}

/*lint -e753 -e528*/

module_init(cambricon_ipu_init);
module_exit(cambricon_ipu_exit);

MODULE_AUTHOR("Cambricon Limited");
MODULE_LICENSE("GPL");

