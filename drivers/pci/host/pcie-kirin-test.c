/*
 * PCIe host controller driver for Kirin 960 SoCs
 *
 * Copyright (C) 2015 Huawei Electronics Co., Ltd.
 *		http://www.huawei.com
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*lint -e438 -e529 -e550 -e774 -e838 -e502 -e648 -e651 -e655  -esym(438,*) -esym(529,*) -esym(550,*) -esym(502,*) -esym(648,*) -esym(651,*) -esym(655,*) -esym(774,*) -esym(838,*) */

#include "pcie-kirin-common.h"
#include <linux/pci-aspm.h>
#include <asm/memory.h>
#include <linux/pci_regs.h>

#define WIFI_FIRMWARE_START 0x180000
#define WIFI_TEST_DATA_SIZE 0xc0000
#define TEST_MEM_SIZE 0x400000
#define TRANSFER_TIMES 50
#define PIPE_TX2RX_LOOPBK (0x1 << 0)
#define PIPE_RX2TX_LOOPBK (0x1 << 1)
#define MAX_RC_NUM 2
#define PCIE0_PHY_BASE 0xFC000000
#define PCIE_PHY_SIZE 0x80000

enum phy_lb_dir {
	TX2RX = 0,
	RX2TX = 1,
};

struct pcie_test_st {
	struct kirin_pcie *pcie;
	u32 is_ep_wifi;
	u64 rc_mem_addr;
	u64 ep_mem_addr;
	u32 wl_power;
};

struct pcie_test_st g_test_kirin_pcie[MAX_RC_NUM];

/**
 * check_pcie_on_work - Check if the parameters are valid and host is working.
 * @rc_id: Host ID;
 */
int check_pcie_on_work(u32 rc_id)
{
	struct pcie_test_st *test_kirin_pcie;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	if (!test_kirin_pcie->pcie) {
		PCIE_PR_ERR("PCIe%d is null", rc_id);
		return -EINVAL;
	}

	if (!test_kirin_pcie->pcie->is_power_on) {
		PCIE_PR_ERR("PCIe%d is power off ", rc_id);
		return -EINVAL;
	}

	return 0;
}

/**
 * show_trans_rate - Show data transmission rate.
 * @begin_time: The time before transmission;
 * @end_time: The time after transmission;
 * @size: The size of datas.
 */
long int show_trans_rate(u64 begin_time, u64 end_time, u64 size)
{
	u64 time_count = end_time - begin_time;
	long int rate = ((size * sizeof(char) * 8 * 100 * TRANSFER_TIMES)/time_count);

	PCIE_PR_INFO("Data size [%lld], time sapce [%lld]",
		(size * sizeof(char) * 8 * 100 * TRANSFER_TIMES), time_count);
	PCIE_PR_INFO("Time sapce [%lld]. Transferring Rate is :[%ld] Bit/s", time_count, rate);

	return rate;
}

/**
 * show_pci_dev_state - Show rc and ep dev state.
 * @rc_id: Host ID;
 */
void show_pci_dev_state(u32 rc_id)
{
	int pm;
	u32 reg_value;
	struct pcie_port *pp;
	struct pci_dev *ep_dev;
	struct pci_dev *rc_dev;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	ep_dev = test_kirin_pcie->pcie->ep_dev;
	rc_dev = test_kirin_pcie->pcie->rc_dev;
	pp = &(test_kirin_pcie->pcie->pp);

	if ((!rc_dev) || (!ep_dev)) {
		PCIE_PR_ERR("Failed to get Device");
		return;
	}

	pm = pci_find_capability(rc_dev, PCI_CAP_ID_PM);
	if (pm) {
		kirin_pcie_rd_own_conf(pp, pm + PCI_PM_CTRL, 4, &reg_value);
		PCIE_PR_INFO("RC: pci_pm_ctrl reg value = [0x%x]", reg_value & 0x3);
	}

	pm = pci_find_capability(ep_dev, PCI_CAP_ID_PM);
	if (pm) {
		pci_read_config_dword(ep_dev, pm + PCI_PM_CTRL, &reg_value);
		PCIE_PR_INFO("EP: pci_pm_ctrl reg value = [0x%x]", reg_value & 0x3);
	}
}

/**
 * disable_outbound_iatu - Disable outbound iatu region.
 * @rc_id: Host ID;
 * @index: Outbound region ID;
 */
void disable_outbound_iatu(u32 rc_id, int index)
{
	struct pcie_port *pp;
	void __iomem *dbi_base;
	unsigned int base_addr;
	struct kirin_pcie *pcie;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	pp = &(pcie->pp);
	dbi_base = pp->dbi_base;
	base_addr = pcie->dtsinfo.iatu_base_offset;

	if (base_addr != PCIE_ATU_VIEWPORT)
		base_addr += (index * 0x200);
	else
		kirin_pcie_writel_rc(pp, PCIE_ATU_REGION_OUTBOUND | (u32)index, dbi_base + PCIE_ATU_VIEWPORT);

	dbi_base += base_addr;

	kirin_pcie_writel_rc(pp, 0x0, dbi_base + PCIE_ATU_CR2);
}

/**
 * disable_inbound_iatu - Disable inbound iatu region.
 * @rc_id: Host ID;
 * @index: Inbound region ID;
 */
void disable_inbound_iatu(u32 rc_id, int index)
{
	struct pcie_port *pp;
	void __iomem *dbi_base;
	unsigned int base_addr;
	struct kirin_pcie *pcie;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	pp = &(pcie->pp);
	dbi_base = pp->dbi_base;
	base_addr = pcie->dtsinfo.iatu_base_offset;

	if (base_addr != PCIE_ATU_VIEWPORT)
		base_addr += (index * 0x200 + 0x100);
	else
		kirin_pcie_writel_rc(pp, PCIE_ATU_REGION_INBOUND | (u32)index, dbi_base + PCIE_ATU_VIEWPORT);

	dbi_base += base_addr;

	kirin_pcie_writel_rc(pp, 0x0, dbi_base + PCIE_ATU_CR2);
}


u64 g_dma_begin_time;
u64 g_dma_end_time;

struct dmatest_done {
	bool			done;
	wait_queue_head_t	*wait;
};

static void dmatest_callback(void *arg)
{
	struct dmatest_done *done = (struct dmatest_done *)arg;
	g_dma_end_time = hisi_getcurtime();
	PCIE_PR_INFO("Dmatest callback, begin_time = %llu,  end_time = %llu", g_dma_begin_time, g_dma_end_time);
	done->done = true;
	wake_up_all(done->wait);
}

/**
 * rc_read_ep_by_dma - RC read EP mem by dma.
 * @addr: RC mem address;
 * @size: Data size;
 * @rc_id: Host ID;
 */
int rc_read_ep_by_dma(u64 addr, u32 size, u32 rc_id)
{
	DECLARE_WAIT_QUEUE_HEAD_ONSTACK(done_wait);
	dma_cap_mask_t mask;
	struct dma_chan		*chan;
	struct dma_device	*dev;
	unsigned long flags;
	enum dma_status status;
	dma_cookie_t cookie;
	struct dmatest_done	done = { .wait = &done_wait };
	struct dma_async_tx_descriptor *tx;
	u64 rc_phys_addr, ep_in_rc_cpu_phys_addr;
	u64 wait_jiffies = msecs_to_jiffies(20000);
	struct pcie_port *pp;

	if (check_pcie_on_work(rc_id))
		return -1;

	pp = &g_test_kirin_pcie[rc_id].pcie->pp;

	rc_phys_addr = addr;
	ep_in_rc_cpu_phys_addr = pp->mem_base + TEST_BUS1_OFFSET;
	flags = (unsigned long)(DMA_CTRL_ACK | DMA_PREP_INTERRUPT);

	kirin_pcie_outbound_atu(rc_id, PCIE_ATU_REGION_INDEX0, PCIE_ATU_TYPE_MEM, ep_in_rc_cpu_phys_addr,
		TEST_BUS1_OFFSET, size);

	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);
	chan = dma_request_channel(mask, NULL, NULL);
	if (chan) {
		dev = chan->device;
		tx = dev->device_prep_dma_memcpy(chan, rc_phys_addr, ep_in_rc_cpu_phys_addr, size, flags);
		if (!tx) {
			PCIE_PR_ERR("Tx is null");
			goto release_dma_channel;
		}
		done.done = false;
		tx->callback = dmatest_callback;
		tx->callback_param = &done;
		cookie = tx->tx_submit(tx);

		if (dma_submit_error(cookie)) {
			PCIE_PR_ERR("DMA submit error");
			goto release_dma_channel;
		}
		g_dma_begin_time = hisi_getcurtime();
		dma_async_issue_pending(chan);
		wait_event_freezable_timeout(done_wait, done.done, wait_jiffies);
		status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
		if (status != DMA_COMPLETE) {
			PCIE_PR_ERR("DMA transfer fail, dmatest_error_type = %d", status);
			goto release_dma_channel;
		}
	} else {
		PCIE_PR_ERR("Failed to get dma channel");
		return -1;
	}
	dma_release_channel(chan);
	return 0;
release_dma_channel:
	dma_release_channel(chan);
	return -1;
}

/**
 * rc_write_ep_by_dma - RC write EP mem by dma.
 * @addr: RC mem address;
 * @size: Data size;
 * @rc_id: Host ID;
 */
int rc_write_ep_by_dma(u64 addr, u32 size, u32 rc_id)
{
	DECLARE_WAIT_QUEUE_HEAD_ONSTACK(done_wait);
	dma_cap_mask_t mask;
	struct dma_chan		*chan;
	struct dma_device	*dev;
	unsigned long flags;
	enum dma_status status;
	dma_cookie_t cookie;
	struct dmatest_done	done = { .wait = &done_wait };
	struct dma_async_tx_descriptor *rx;
	u64 rc_phys_addr, ep_in_rc_cpu_phys_addr;
	u64 wait_jiffies = msecs_to_jiffies(20000);
	struct pcie_port *pp;

	if (check_pcie_on_work(rc_id))
		return -1;

	pp = &g_test_kirin_pcie[rc_id].pcie->pp;

	rc_phys_addr = addr;
	ep_in_rc_cpu_phys_addr = pp->mem_base + TEST_BUS1_OFFSET;

	kirin_pcie_outbound_atu(rc_id, PCIE_ATU_REGION_INDEX0, PCIE_ATU_TYPE_MEM, ep_in_rc_cpu_phys_addr,
		TEST_BUS1_OFFSET, size);

	flags = (unsigned long)(DMA_CTRL_ACK | DMA_PREP_INTERRUPT);

	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);
	chan = dma_request_channel(mask, NULL, NULL);
	if (chan) {
		dev = chan->device;
		rx = dev->device_prep_dma_memcpy(chan, ep_in_rc_cpu_phys_addr, rc_phys_addr, size, flags);
		if (!rx) {
			PCIE_PR_ERR("Rx is null");
			goto release_dma_channel;
		}
		done.done = false;
		rx->callback = dmatest_callback;
		rx->callback_param = &done;
		cookie = rx->tx_submit(rx);

		if (dma_submit_error(cookie)) {
			PCIE_PR_ERR("DMA submit error");
			goto release_dma_channel;
		}
		g_dma_begin_time = hisi_getcurtime();
		dma_async_issue_pending(chan);
		wait_event_freezable_timeout(done_wait, done.done, wait_jiffies);
		status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
		if (status != DMA_COMPLETE) {
			PCIE_PR_ERR("DMA transfer fail, dmatest_error_type = %d", status);
			goto release_dma_channel;
		}
	} else {
		PCIE_PR_ERR("Failed to get dma channel");
		return -1;
	}
	dma_release_channel(chan);
	return 0;
release_dma_channel:
	dma_release_channel(chan);
	return -1;
}

static void calc_rate(char* description,    /* print description */
                      u32 size,       /* transfer's block size */
                      u64 start_time,       /* transfer's start time */
                      u64 end_time)         /* transfer's end time */
{
    /* HZ: system timer interrupt number per seconds */
	u64 cost_ns = end_time - start_time;

	u64 kbyte_size = size >> 10;
	u64 mbyte_size = size >> 20;
	u64 gbyte_size = size >> 30;

	u64 kbit_rate = (cost_ns) ? (kbyte_size * 8 * 1000000000) / (cost_ns) : 0;
	u64 mbit_rate = (cost_ns) ? (mbyte_size * 8 * 1000000000) / (cost_ns) : 0;
	u64 gbit_rate = (cost_ns) ? (mbyte_size * 8 * 1000000) / (cost_ns) : 0;

	PCIE_PR_INFO("%s total size: %llu MB(%llu GB)(%llu KB), cost times: %llu ns, rate:%llu Mb/s(%llu Gb/s)(%llu Kb/s)\n",
		description, mbyte_size, gbyte_size, kbyte_size, cost_ns, mbit_rate, gbit_rate, kbit_rate);
}

/**
 * dma_trans_rate - Calculate the dma rate of data transmision.
 * @rc_id: Host ID;
 * @direction: 0 -- RC read EP; 1 -- RC write EP;
 * @size: Data size;
 * @addr: RC mem address;
 */
int dma_trans_rate(u32 rc_id, int direction, u32 size, u64 addr)
{
	struct kirin_pcie *kirin_pcie;
	int ret = 0;
	void __iomem *dbi_base;

	g_dma_begin_time = 0;
	g_dma_end_time = 0;

	if (check_pcie_on_work(rc_id))
		return -1;

	kirin_pcie = &g_kirin_pcie[rc_id];

	dbi_base = ioremap(kirin_pcie->pp.mem_base, 0x2000);
	if (!dbi_base) {
		PCIE_PR_ERR("Failed to ioremap dbi_base");
		return -1;
	}

	config_enable_dbi(rc_id, 1);
	writel(0xfffffff, dbi_base + 0x1010);
	config_enable_dbi(rc_id, 0);

	switch (direction) {
		case 0: {
			ret = rc_read_ep_by_dma(addr, size, rc_id);
			if (ret) {
				PCIE_PR_ERR("RC read EP by DMA  fail");
				goto END;
			}
			calc_rate ("EP_TO_RC_BY_DMA rate :", size, g_dma_begin_time, g_dma_end_time);
			break;
		}
		case 1: {
			ret = rc_write_ep_by_dma(addr, size, rc_id);
			if (ret) {
				PCIE_PR_ERR("RC write EP by DMA  fail");
				goto END;
			}
			calc_rate ("RC_TO_EP_BY_DMA rate :", size, g_dma_begin_time, g_dma_end_time);
			break;
		}
		default:
			PCIE_PR_ERR("Invalid Param");
			goto END;
	}

END:
	iounmap((void __iomem *)dbi_base);
	return ret;
}

/**
 * rc_read_ep_cfg - RC read EP configuration.
 * @rc_id: Host ID;
 */
int rc_read_ep_cfg(u32 rc_id)
{
	unsigned int i;
	u64 ep_vaddr_in_cpu;
	u64 busdev;
	struct pci_dev *ep_dev;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	ep_dev = test_kirin_pcie->pcie->ep_dev;

	if (!ep_dev) {
		PCIE_PR_ERR("Failed to get EP device");
		return -1;
	}

	busdev = TEST_BUS1_OFFSET;
	ep_vaddr_in_cpu = pci_resource_start(ep_dev, 0);

	PCIE_PR_INFO("EP addr in cpu physical is [0x%llx]", ep_vaddr_in_cpu);
	kirin_pcie_outbound_atu(rc_id, PCIE_ATU_REGION_INDEX0, PCIE_ATU_TYPE_CFG0, ep_vaddr_in_cpu, busdev, 0x1000);
	ep_vaddr_in_cpu = (__force u64)ioremap(ep_vaddr_in_cpu, 0x1000);
	if (!ep_vaddr_in_cpu) {
		PCIE_PR_ERR("Failed to ioremap ep_vaddr_in_cpu");
		return -1;
	}
	PCIE_PR_INFO("EP addr in cpu virtual is [0x%llx]", ep_vaddr_in_cpu);

	config_enable_dbi(rc_id, DISABLE);
	for (i = 0; i < 0x100; i += 4)
		PCIE_PR_INFO("Wifi cfg register: offset[%d] = [0x%x]", i, readl((void *)ep_vaddr_in_cpu + i));

	iounmap((void __iomem *)ep_vaddr_in_cpu);
	return 0;
}

/**
 * rc_read_wifi_mem - RC read EP mem.
 * @rc_id: Host ID;
 */
int rc_read_wifi_mem(u32 rc_id, u32 size)
{
	unsigned int i;
	u32 bar2 = 0;
	int ret = 0;
	struct kirin_pcie *pcie;
	struct pcie_port *pp;
	struct pcie_test_st *test_kirin_pcie;
	void __iomem *loop_back_cmp;
	void __iomem *loop_back_src;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	pp = &(pcie->pp);

	loop_back_cmp = kmalloc(WIFI_TEST_DATA_SIZE, GFP_KERNEL);
	if (!loop_back_cmp) {
		PCIE_PR_ERR("Failed to alloc loop_back_cmp memory");
		return -1;
	}

	ret = pci_enable_device(pcie->ep_dev);
	if (ret) {
		PCIE_PR_ERR("Failed to enable ep_dev");
		goto TEST_FAIL_FREE;
	}
	pci_set_master(pcie->ep_dev);

	pci_read_config_dword(pcie->ep_dev, 0x18, &bar2);
	PCIE_PR_INFO("Bar2 is %x", bar2);
	loop_back_src = ioremap_nocache(pp->mem_base, 0x400000);
	if (!loop_back_src) {
		PCIE_PR_ERR("Failed to ioremap loop_back_src");
		ret = -1;
		goto TEST_FAIL_FREE;
	}
	kirin_pcie_outbound_atu(pcie->rc_id, PCIE_ATU_REGION_INDEX0, PCIE_ATU_TYPE_MEM,
				pp->mem_base, (bar2 & (~0x3)), 0x400000);

	if ((size <= 0) || (size > WIFI_TEST_DATA_SIZE))
		size = WIFI_TEST_DATA_SIZE;

	for (i = 0; i < size; i = i + 4) {
		writel((0x1234abcd + i), (loop_back_cmp + i));
		writel((0x1234abcd + i), (loop_back_src + i + WIFI_FIRMWARE_START));
	}
	if (memcmp((loop_back_src + WIFI_FIRMWARE_START), loop_back_cmp, size)) {
		PCIE_PR_ERR("RC read/write EP mem fail");
		ret = -1;
		goto TEST_FAIL_IOUNMAP;
	}
	PCIE_PR_INFO("RC read/write EP mem OK");

TEST_FAIL_IOUNMAP:
	iounmap(loop_back_src);
TEST_FAIL_FREE:
	kfree(loop_back_cmp);
	return ret;
}

/**
 * rc_read_ep_mem - RC read EP mem by cpu.
 * @rc_id: Host ID;
 * @size: Data size;
 * @index: Inbound region ID;
 */
int rc_read_ep_mem(u32 rc_id, unsigned int size, int index)
{
	int i;
	u64 cpu_addr;
	u64 begin_time;
	u64 end_time;
	int ret;
	char *temp_memcmp;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	if (!test_kirin_pcie->rc_mem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->rc_mem_addr is null");
		return -1;
	}

	cpu_addr = test_kirin_pcie->pcie->pp.mem_base + TEST_BUS1_OFFSET;

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	PCIE_PR_INFO("EP addr in cpu physical is [0x%llx], size is [0x%x]", cpu_addr, size);

	kirin_pcie_outbound_atu(rc_id, index, PCIE_ATU_TYPE_MEM, cpu_addr, TEST_BUS1_OFFSET, TEST_MEM_SIZE);

	cpu_addr = (__force u64)ioremap(cpu_addr, TEST_MEM_SIZE);
	if (!cpu_addr) {
		PCIE_PR_ERR("Failed to ioremap cpu addr");
		return -1;
	}

	temp_memcmp = (char *)kmalloc(TEST_MEM_SIZE, GFP_KERNEL);
	if (!temp_memcmp) {
		PCIE_PR_ERR("Failed to alloc temp_memcmp");
		iounmap((void __iomem *)cpu_addr);
		return -1;
	}
	memset(temp_memcmp, 0xff, TEST_MEM_SIZE);

	PCIE_PR_INFO("Reading from EP mem");
	begin_time = jiffies;

	for (i = 0; i < TRANSFER_TIMES; i++)
		memcpy((void *)test_kirin_pcie->rc_mem_addr, (void *)cpu_addr, size);

	end_time = jiffies;
	show_trans_rate(begin_time, end_time, size);

	if (memcmp((void *)test_kirin_pcie->rc_mem_addr, (void *)cpu_addr, size) != 0 ||
		memcmp((void *)test_kirin_pcie->rc_mem_addr, temp_memcmp, size) == 0)
		ret = -1;
	else
		ret = 0;

	iounmap((void __iomem *)cpu_addr);
	kfree(temp_memcmp);

	return ret;
}

/**
 * set_ep_mem_inbound - Set EP DDR mem inbound for loopback data transfer.
 * @rc_id: Host ID;
 * @index: Inbound region ID;
 */
int set_ep_mem_inbound(u32 rc_id, int index)
{
	u64 busdev, cpu_addr, temp_addr;
	struct pcie_test_st *test_kirin_pcie;
	int i;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	set_bme(rc_id, ENABLE);
	set_mse(rc_id, ENABLE);
	busdev = TEST_BUS1_OFFSET;

	if (!test_kirin_pcie->ep_mem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->ep_mem_addr is null");
		return -1;
	}

	temp_addr = virt_to_phys((void *)test_kirin_pcie->ep_mem_addr);
	cpu_addr = test_kirin_pcie->ep_mem_addr;

	PCIE_PR_INFO("Inbound pci_add [0x%llx] to cpu_addr[0x%llx]", busdev, temp_addr);
	kirin_pcie_inbound_atu(rc_id, index, PCIE_ATU_TYPE_MEM, temp_addr, busdev, TEST_MEM_SIZE);

	for (i = 0; i < TEST_MEM_SIZE; i+=4)
		writel(i, (void *)test_kirin_pcie->ep_mem_addr + i);

	return 0;
}

/**
 * read_ep_addr - Read EP MEM value.
 * @rc_id: Host ID;
 * @offset: The offset which you want to read;
 * @size: Data size;
 */
void read_ep_addr(u32 rc_id, u32 offset, u32 size)
{
	u32 i;
	u32 val;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	if (!test_kirin_pcie->ep_mem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->ep_mem_add is null");
		return;
	}

	for (i = 0; i < size; i += 4) {
		val = readl((void *)test_kirin_pcie->ep_mem_addr + offset + i);
		PCIE_PR_INFO("Offset[0x%x], value=[0x%x]", i + offset, val);
	}

	return;
}

/**
 * rc_write_ep_mem - RC write EP mem by cpu.
 * @rc_id: Host ID;
 * @size: Data size;
 * @index: Inbound region ID;
 */
int rc_write_ep_mem(u32 rc_id, unsigned int size, int index)
{
	u64 cpu_addr;
	u64 begin_time;
	u64 end_time;
	int i;
	int ret;
	char *temp_memcmp;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	if (!test_kirin_pcie->rc_mem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->rc_mem_addr is null");
		return -1;
	}

	memset((void *)test_kirin_pcie->rc_mem_addr, 0xef, TEST_MEM_SIZE);

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	cpu_addr = test_kirin_pcie->pcie->pp.mem_base + TEST_BUS1_OFFSET;

	kirin_pcie_outbound_atu(rc_id, index, PCIE_ATU_TYPE_MEM, cpu_addr, TEST_BUS1_OFFSET, TEST_MEM_SIZE);

	cpu_addr = (__force u64)ioremap_nocache(cpu_addr, TEST_MEM_SIZE);
	if (!cpu_addr) {
		PCIE_PR_ERR("Failed to ioremap cpu addr");
		return -1;
	}

	temp_memcmp = (char *)kmalloc(size, GFP_KERNEL);
	if (!temp_memcmp) {
		PCIE_PR_ERR("Failed to alloc temp_memcmp");
		iounmap((void __iomem *)cpu_addr);
		return -1;
	}
	memset(temp_memcmp, 0xff, size);

	PCIE_PR_INFO("Writing 0xef to EP mem");

	begin_time = jiffies;

	for (i = 0; i < TRANSFER_TIMES; i++)
		memcpy((void *)cpu_addr, (void *)test_kirin_pcie->rc_mem_addr, size);

	end_time = jiffies;

	show_trans_rate(begin_time, end_time, size);

	if (memcmp((void *)test_kirin_pcie->rc_mem_addr, (void *)cpu_addr, size) != 0 ||
		memcmp((void *)cpu_addr, temp_memcmp, size) == 0)
		ret = -1;
	else
		ret = 0;

	iounmap((void __iomem *)cpu_addr);
	kfree(temp_memcmp);
	return ret;
}

/**
 * data_trans_ok - Check if RC data EQU EP data.
 * @rc_id: Host ID;
 * @size: Data size;
 */
int data_trans_ok(u32 rc_id, unsigned int size)
{
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	if ((!test_kirin_pcie->ep_mem_addr) || (!test_kirin_pcie->rc_mem_addr))
		return -1;

	if (memcmp((void *)test_kirin_pcie->ep_mem_addr, (void *)test_kirin_pcie->rc_mem_addr, size) != 0)
		return -1;
	else
		return 0;
}

/**
 * set_rc_mem_inbound - Set RC DDR mem inbound for loopback data transfer.
 * @rc_id: Host ID;
 * @index: Inbound region ID;
 */
int set_rc_mem_inbound(u32 rc_id, int index)
{
	u64 busdev, temp_addr;
	int i;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	set_bme(rc_id, ENABLE);
	set_mse(rc_id, ENABLE);
	busdev = TEST_BUS0_OFFSET;

	if (!test_kirin_pcie->rc_mem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->rc_mem_addr is null");
		return -1;
	}

	temp_addr = virt_to_phys((void *)test_kirin_pcie->rc_mem_addr);

	PCIE_PR_INFO("Inbound pci_add [0x%llx] to cpu_addr[0x%llx]", busdev, temp_addr);
	kirin_pcie_inbound_atu(rc_id, index, PCIE_ATU_TYPE_MEM, temp_addr, busdev, TEST_MEM_SIZE);

	for (i = 0; i < TEST_MEM_SIZE; i+=4)
		writel(i, (void *)test_kirin_pcie->rc_mem_addr + i);

	return 0;
}

/**
 * read_rc_addr - Read RC MEM value.
 * @rc_id: Host ID;
 * @offset: The offset which you want to read;
 * @size: Data size;
 */
int read_rc_addr(u32 rc_id, u32 offset, u32 size)
{
	u32 i;
	u32 val;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	if (!test_kirin_pcie->rc_mem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->rc_mem_add is null");
		return -1;
	}

	for (i = 0; i < size; i += 4) {
		val = readl((void *)test_kirin_pcie->rc_mem_addr + offset + i);
		PCIE_PR_INFO("Offset[0x%x], value=[0x%x]", i + offset, val);
	}

	return 0;
}

/**
 * ep_read_rc_mem - EP read RC mem by cpu.
 * @rc_id: Host ID;
 * @size: Data size;
 * @index: Outbound region ID;
 */
int ep_read_rc_mem(u32 rc_id, unsigned int size, int index)
{
	u64 cpu_addr;
	u32 i;
	u64 begin_time;
	u64 end_time;
	int ret;
	char *temp_memcmp;
	struct resource *dbi;
	struct platform_device *pdev;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return -1;

	if (size > TEST_MEM_SIZE || size == 0)
		size = TEST_MEM_SIZE;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	if (!test_kirin_pcie->ep_mem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->ep_mem_addr is null");
		return -1;
	}

	pdev = to_platform_device(test_kirin_pcie->pcie->pp.dev);
	dbi = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	if (!dbi) {
		PCIE_PR_ERR("Failed to get dbi base");
		return -1;
	}

	cpu_addr = dbi->start + TEST_BUS0_OFFSET;
	kirin_pcie_outbound_atu(rc_id, index, PCIE_ATU_TYPE_MEM, cpu_addr, TEST_BUS0_OFFSET, TEST_MEM_SIZE);

	cpu_addr = (__force u64)ioremap(cpu_addr, TEST_MEM_SIZE);
	if (!cpu_addr) {
		PCIE_PR_ERR("Failed to ioremap cpu addr");
		return -1;
	}

	temp_memcmp = (char *)kmalloc(size, GFP_KERNEL);
	if (!temp_memcmp) {
		PCIE_PR_ERR("Failed to alloc temp_memcmp");
		iounmap((void __iomem *)cpu_addr);
		return -1;
	}
	memset(temp_memcmp, 0xff, size);
	memset((void *)test_kirin_pcie->ep_mem_addr, 0xbc, size);

	PCIE_PR_INFO("Reading from RX mem");
	begin_time = jiffies;
	for (i = 0; i < TRANSFER_TIMES; i++)
		memcpy((void *)test_kirin_pcie->ep_mem_addr, (void *)cpu_addr, size);
	end_time = jiffies;

	show_trans_rate(begin_time, end_time, size);

	if (memcmp((void *)test_kirin_pcie->ep_mem_addr, (void *)cpu_addr, size) != 0 ||
		memcmp((void *)test_kirin_pcie->ep_mem_addr, temp_memcmp, size) == 0)
		ret = -1;
	else
		ret = 0;

	iounmap((void __iomem *)cpu_addr);
	kfree(temp_memcmp);
	return ret;
}

/**
 * ep_write_rc_mem - EP write RC mem by cpu.
 * @rc_id: Host ID;
 * @size: Data size;
 * @index: Outbound region ID;
 */
int ep_write_rc_mem(u32 rc_id, unsigned int size, int index)
{
	u64 cpu_addr;
	u64 begin_time;
	u64 end_time;
	int i;
	int ret;
	char *temp_memcmp;
	struct resource *dbi;
	struct platform_device *pdev;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return -1;

	if (size > TEST_MEM_SIZE || size <= 0)
		size = TEST_MEM_SIZE;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	if (!test_kirin_pcie->ep_mem_addr) {
		PCIE_PR_ERR("The test_kirin_pcie->ep_mem_addr is null");
		return -1;
	}

	pdev = to_platform_device(test_kirin_pcie->pcie->pp.dev);
	dbi = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	if (!dbi) {
		PCIE_PR_ERR("Failed to get dbi base");
		return -1;
	}

	cpu_addr = dbi->start + TEST_BUS0_OFFSET;

	kirin_pcie_outbound_atu(rc_id, index, PCIE_ATU_TYPE_MEM, cpu_addr, TEST_BUS0_OFFSET, TEST_MEM_SIZE);

	cpu_addr = (__force u64)ioremap_nocache(cpu_addr, TEST_MEM_SIZE);
	if (!cpu_addr) {
		PCIE_PR_ERR("Failed to ioremap cpu addr");
		return -1;
	}

	temp_memcmp = (char *)kmalloc(size, GFP_KERNEL);
	if (!temp_memcmp) {
		PCIE_PR_ERR("Failed to alloc temp_memcmp");
		iounmap((void __iomem *)cpu_addr);
		return -1;
	}
	memset(temp_memcmp, 0xff, size);

	PCIE_PR_INFO("Writing 0xbc to RC mem");
	memset((void *)test_kirin_pcie->ep_mem_addr, 0xbc, size);

	begin_time = jiffies;
	for (i = 0; i < TRANSFER_TIMES; i++)
		memcpy((void *)cpu_addr, (void *)test_kirin_pcie->ep_mem_addr, size);
	end_time = jiffies;

	show_trans_rate(begin_time, end_time, size);
	if (memcmp((void *)test_kirin_pcie->ep_mem_addr, (void *)cpu_addr, size) != 0 ||
		memcmp((void *)cpu_addr, temp_memcmp, size) == 0)
		ret = -1;
	else
		ret = 0;

	iounmap((void __iomem *)cpu_addr);
	kfree(temp_memcmp);
	return ret;
}

/**
 * test_host_power_control - Power on host and scan bus.
 * @rc_id: Host ID;
 * @flag: RC power status;
 */
int test_host_power_control(u32 rc_id, u32 flag)
{
	struct pcie_port *pp;
	int ret = 0;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	pp = &(g_test_kirin_pcie[rc_id].pcie->pp);

	ret = kirin_pcie_power_on(pp, (enum rc_power_status )flag);

	return ret;
}

/**
 * test_compliance - Set compliance for test.
 * @rc_id: Host ID;
 * @entry: 0 -- exit compliance; others -- entry compliance;
 */
int test_compliance(u32 rc_id, u32 entry)
{
	u32 val = 0;
	u32 cap_pos;
	struct pcie_test_st *test_kirin_pcie;
	struct pcie_port *pp;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pp  = &(g_test_kirin_pcie[rc_id].pcie->pp);
	cap_pos = kirin_pcie_find_capability(pp, PCI_CAP_ID_EXP);
	if (!cap_pos)
		return -1;

	ltssm_enable(rc_id, DISABLE);
	kirin_pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, 4, &val);
	if (entry)
		val |= ENTER_COMPLIANCE;
	else
		val &= ~ENTER_COMPLIANCE;
	kirin_pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, 4, val);

	ltssm_enable(rc_id, ENABLE);

	show_link_state(rc_id);
	return 0;
}

/**
 *set_preset - set preset for gen3 test.
 * @rc_id: Host ID;
 * @preset: preset value;
 */
int set_preset(u32 rc_id, u32 preset)
{
	u32 val;
	u32 cap_pos;
	struct pcie_port *pp;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pp = &(test_kirin_pcie->pcie->pp);

	if (preset > 10) {
		PCIE_PR_ERR("Invalid Param");
		return -1;
	}

	ltssm_enable(rc_id, DISABLE);
	cap_pos = kirin_pcie_find_capability(pp, PCI_CAP_ID_EXP);
	if (!cap_pos) {
		PCIE_PR_ERR("Failed to get PCI_CAP_ID_EXP");
		return -1;
	}

	kirin_pcie_rd_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, 4, &val);
	val &= ~(0xf << 12);
	val |= (preset << 12);
	kirin_pcie_wr_own_conf(pp, (int)cap_pos + PCI_EXP_LNKCTL2, 4, val);

	ltssm_enable(rc_id, ENABLE);
	return 0;
}

/**
 * test_entry_loopback - Set entry loopabck for test.
 * @rc_id: Host ID;
 * @local: 0 -- remote loopback; others -- local loopback;
 */
void test_entry_loopback(u32 rc_id, u32 local)
{
	u32 val;
	struct pcie_port *pp;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pp = &(test_kirin_pcie->pcie->pp);

	kirin_pcie_rd_own_conf(pp, PORT_LINK_CTRL_REG, 4, &val);
	val |= (0x1 << 2);
	kirin_pcie_wr_own_conf(pp, PORT_LINK_CTRL_REG, 4, val);

	if (local) {
		kirin_pcie_rd_own_conf(pp, PORT_GEN3_CTRL_REG, 4, &val);
		val |= (0x1 << 16);
		kirin_pcie_wr_own_conf(pp, PORT_GEN3_CTRL_REG, 4, val);

		kirin_pcie_rd_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, &val);
		val |= (0x1 << 31);
		kirin_pcie_wr_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, val);
	}

}

/**
 * test_exit_loopback - Set exit loopabck.
 * @rc_id: Host ID;
 * @local: 0 -- remote loopback; others -- local loopback;
 */
void test_exit_loopback(u32 rc_id, u32 local)
{
	u32 val;
	struct pcie_port *pp;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pp = &(test_kirin_pcie->pcie->pp);

	kirin_pcie_rd_own_conf(pp, PORT_LINK_CTRL_REG, 4, &val);
	val &= ~(0x1 << 2);
	kirin_pcie_wr_own_conf(pp, PORT_LINK_CTRL_REG, 4, val);

	if (local) {
		kirin_pcie_rd_own_conf(pp, PORT_GEN3_CTRL_REG, 4, &val);
		val &= ~(0x1 << 16);
		kirin_pcie_wr_own_conf(pp, PORT_GEN3_CTRL_REG, 4, val);

		kirin_pcie_rd_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, &val);
		val &= ~(0x1 << 31);
		kirin_pcie_wr_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, val);
	}

}

/**
 * bypass_detect - Bypass detect link state.
 * @rc_id: Host ID;
 */
void bypass_detect(u32 rc_id)
{
	u32 val = 0;
	struct kirin_pcie *pcie;
	struct pcie_port *pp;

	if (check_pcie_on_work(rc_id))
		return;

	pcie = g_test_kirin_pcie[rc_id].pcie;
	pp = &pcie->pp;

	kirin_pcie_rd_own_conf(pp, PROT_FORCE_LINK_REG, 4, &val);
	val |= (0x1 << 17);
	kirin_pcie_wr_own_conf(pp, PROT_FORCE_LINK_REG, 4, val);
	udelay(10);

	kirin_pcie_rd_own_conf(pp, PROT_FORCE_LINK_REG, 4, &val);
	val |= (0x1 << 15);
	kirin_pcie_wr_own_conf(pp, PROT_FORCE_LINK_REG, 4, val);
}

/**
 * phy_internal_loopback - Set phy internal loopback for test.
 * @rc_id: Host ID;
 * @dir: 0 -- TX2RX; 1 -- RX2TX;
 */
int phy_internal_loopback(u32 rc_id, enum phy_lb_dir dir, enum link_speed gen)
{
	u32 val = 0;
	struct kirin_pcie *pcie;
	struct pcie_port *pp;
	void __iomem    *phy_base;

	if (check_pcie_on_work(rc_id))
		return -1;

	pcie = g_test_kirin_pcie[rc_id].pcie;
	pp = &pcie->pp;

	set_link_speed(rc_id, gen);

	phy_base = ioremap(PCIE0_PHY_BASE, PCIE_PHY_SIZE);
	if (!phy_base) {
		PCIE_PR_ERR("Failed to ioremap phy_base");
		return -1;
	}

	if (dir == TX2RX) {
		writel(0x5, phy_base + 0x4000);
	} else {
		writel(0x6, phy_base + 0x4000);
	}

	if (gen == GEN3) {
		kirin_pcie_rd_own_conf(pp, PORT_GEN3_CTRL_REG, 4, &val);
		val |= (0x1 << 16);
		kirin_pcie_wr_own_conf(pp, PORT_GEN3_CTRL_REG, 4, val);

		kirin_pcie_rd_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, &val);
		val |= (0x1 << 31);
		kirin_pcie_wr_own_conf(pp, PORT_PIPE_LOOPBACK_REG, 4, val);
	}

	iounmap((void __iomem *)phy_base);
	return 0;
}

/**
 * ep_triggle_intr - EP triggle MSI interrupt.
 * @rc_id: Host ID;
 */
void ep_triggle_intr(u32 rc_id)
{
	u32 value_temp;
	struct kirin_pcie *pcie;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	value_temp = kirin_elb_readl(pcie, 0x8);
	value_temp |= (0x1 << 26);
	kirin_elb_writel(pcie, value_temp, 0x8);
	udelay((unsigned long)2);

	value_temp &= ~(0x1 << 26);
	kirin_elb_writel(pcie, value_temp, 0x8);
	PCIE_PR_INFO("Read after write, ctl2 is %x", kirin_elb_readl(pcie, 0x8));
	PCIE_PR_INFO("ep_triggle_msi_intr ");
}

/**
 * ep_triggle_intA_intr - EP triggle INTa interrupt.
 * @rc_id: Host ID;
 */
void ep_triggle_intA_intr(u32 rc_id)
{
	u32 value_temp;
	struct kirin_pcie *pcie;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	value_temp = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	value_temp |= (0x1 << 5);
	kirin_elb_writel(pcie, value_temp, SOC_PCIECTRL_CTRL7_ADDR);

	value_temp = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	PCIE_PR_INFO("read after write, ctl7 is %x", value_temp);
	PCIE_PR_INFO("ep_triggle_inta_intr");
}

/**
 * ep_clr_intA_intr - EP clear INTa interrupt.
 * @rc_id: Host ID;
 */
void ep_clr_intA_intr(u32 rc_id)
{
	u32 value_temp;
	struct kirin_pcie *pcie;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	value_temp = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	value_temp &= ~(0x1 << 5);
	kirin_elb_writel(pcie, value_temp, SOC_PCIECTRL_CTRL7_ADDR);

	value_temp = kirin_elb_readl(pcie, SOC_PCIECTRL_CTRL7_ADDR);
	PCIE_PR_INFO("read after write, ctl7 is %x", value_temp);
	PCIE_PR_INFO("ep_clr_inta_intr");
}

/**
 * generate_msg - Generate MSG.
 * @rc_id: Host ID;
 * @msg_code: MSG code;
 */
void generate_msg(u32 rc_id, int msg_code)
{
	u64 cpu_addr;
	u32 val;
	struct resource *dbi;
	struct platform_device *pdev;
	unsigned int iatu_offset;
	struct pcie_port *pp;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];

	pp = &(test_kirin_pcie->pcie->pp);
	pdev = to_platform_device(pp->dev);
	dbi = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
	if (!dbi) {
		PCIE_PR_ERR("Failed to get dbi base");
		return;
	}

	cpu_addr = dbi->start;
	PCIE_PR_ERR("cpu_addr = 0x%llx", cpu_addr);

	iatu_offset = test_kirin_pcie->pcie->dtsinfo.iatu_base_offset;

	kirin_pcie_outbound_atu(rc_id, PCIE_ATU_REGION_INDEX0, PCIE_ATU_TYPE_MSG,
				cpu_addr, TEST_BUS1_OFFSET, TEST_BUS1_OFFSET);

	kirin_pcie_readl_rc(pp, pp->dbi_base+ iatu_offset + PCIE_ATU_CR2, &val);
	val |= (msg_code | 0x400000);
	kirin_pcie_writel_rc(pp, val, pp->dbi_base+ iatu_offset + PCIE_ATU_CR2);

	writel(0x0, (void *)pp->dbi_base);

}

/**
 * msg_triggle_clr - Triggle or clear MSG.
 * @rc_id: Host ID;
 * @offset: Register offset;
 * @bit: Register bit;
 */
void msg_triggle_clr(u32 rc_id, u32 offset, u32 bit)
{
	u32 value_temp;
	struct kirin_pcie *pcie;
	struct pcie_test_st *test_kirin_pcie;


	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;

	value_temp = kirin_elb_readl(pcie, offset);
	value_temp &= ~(0x1 << bit);
	kirin_elb_writel(pcie, value_temp, offset);
	value_temp |= (0x1 << bit);
	kirin_elb_writel(pcie, value_temp, offset);
}

/**
 * msg_received - Whether received MSG or not.
 * @rc_id: Host ID;
 * @offset: Register offset;
 * @bit: Register bit;
 */
int msg_received(u32 rc_id, u32 offset, u32 bit)
{
	u32 value_temp;
	struct kirin_pcie *pcie;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return -1;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;
	value_temp = kirin_elb_readl(pcie, offset);
	if (value_temp & (0x1 << bit)) 
		return 1;
	else 
		return 0;
}

/**
 * pcie_enable_msg_num - Enable MSG function,include LTR and OBFF.
 * @rc_id: Host ID;
 * @num: 13 -- OBFF; 10 -- LTR; others -- not msg function
 * @local: 0 -- Set EP dev; others -- Set RC dev;
 */
void pcie_enable_msg_num(u32 rc_id, int num, int local)
{
	u32 val;
	struct pci_dev *dev;
	struct kirin_pcie *pcie;
	struct pcie_test_st *test_kirin_pcie;

	if (check_pcie_on_work(rc_id))
		return;

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	pcie = test_kirin_pcie->pcie;

	if (local)
		dev = pcie->rc_dev;
	else
		dev = pcie->ep_dev;

	pcie_capability_read_dword(dev, PCI_EXP_DEVCTL2, &val);

	switch (num) {
	case 13:
		PCIE_PR_INFO("Enable obff message");
		val |= PCI_EXP_DEVCTL2_OBFF_MSGA_EN;
		pcie_capability_write_dword(dev, PCI_EXP_DEVCTL2, val);
		break;
	case 10:
		PCIE_PR_INFO("Enable LTR message");
		val |= PCI_EXP_DEVCTL2_LTR_EN;
		pcie_capability_write_dword(dev, PCI_EXP_DEVCTL2, val);
		break;
	default:
		PCIE_PR_INFO("Unsupport function");
		break;
	}

}

int kirin_pcie_test_init(u32 rc_id)
{
	struct device_node *np;
	struct kirin_pcie *pcie;
	struct pcie_test_st *test_kirin_pcie;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	pcie = &g_kirin_pcie[rc_id];
	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	test_kirin_pcie->pcie = pcie;
	test_kirin_pcie->rc_mem_addr = __get_free_pages(GFP_KERNEL, 10);
	test_kirin_pcie->ep_mem_addr = __get_free_pages(GFP_KERNEL, 10);
	if (!test_kirin_pcie->rc_mem_addr || !test_kirin_pcie->ep_mem_addr) {
		PCIE_PR_ERR("Failed to alloc ep or rc mem_space ");
		return -EINVAL;
	}

	np = pcie->pp.dev->of_node;
	if (np) {
		if (of_property_read_u32(np, "wl_power", &test_kirin_pcie->wl_power)) {
			PCIE_PR_ERR("Failed to get wl_power info");
			return -EINVAL;
		}
		PCIE_PR_INFO("WL Power On Number is [%d] ", test_kirin_pcie->wl_power);
	} else {
		PCIE_PR_INFO("Failed to get kirin-pcie node");
		return -EINVAL;
	}
	return 0;
}

int kirin_pcie_test_exist(u32 rc_id)
{
	struct pcie_test_st *test_kirin_pcie;

	if (rc_id >= g_rc_num) {
		PCIE_PR_ERR("There is no rc_id = %d", rc_id);
		return -EINVAL;
	}

	test_kirin_pcie = &g_test_kirin_pcie[rc_id];
	if (test_kirin_pcie->rc_mem_addr) {
		free_pages(test_kirin_pcie->rc_mem_addr, 10);
		test_kirin_pcie->rc_mem_addr = 0;
	}
	if (test_kirin_pcie->ep_mem_addr) {
		free_pages(test_kirin_pcie->ep_mem_addr, 10);
		test_kirin_pcie->ep_mem_addr = 0;
	}
	test_kirin_pcie->pcie = NULL;
	test_kirin_pcie->wl_power = 0;
	return 0;
}
/*lint -e438 -e529 -e550 -e502 -e648 -e651 -e655 -e774 -e838 +esym(438,*) +esym(529,*) +esym(550,*) +esym(502,*) +esym(648,*) +esym(651,*) +esym(655,*) +esym(774,*) +esym(838,*) */
MODULE_DESCRIPTION("Hisilicon Kirin pcie driver");
MODULE_LICENSE("GPL");


