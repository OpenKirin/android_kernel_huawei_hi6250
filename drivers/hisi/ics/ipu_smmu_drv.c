#include <linux/errno.h>
#include <linux/hisi/hisi-iommu.h> //for struct iommu_domain_data
#include <linux/iommu.h> //for struct iommu_domain
#include <linux/mutex.h>
#include "ipu_smmu_drv.h"

#define SMMU_MSTR_DEBUG_CONFIG_WR (16)
#define SMMU_MSTR_DEBUG_CONFIG_CS (17)
#define SMMU_MSTR_SET_DEBUG_PORT ((1 << SMMU_MSTR_DEBUG_CONFIG_WR) | (1 << SMMU_MSTR_DEBUG_CONFIG_CS))
#define SMMU_MSTR_END_ACK_THRESHOLD (0x100)
#define SMMU_MSTR_INPUT_SEL_REGISTER (0x00000003)
#define SMMU_MSTR_ALL_STREAM_IS_END_ACK (0x0000000f)
#define SMMU_MSTR_GLB_BYPASS_NORMAL_MODE (0x00000000)
#define SMMU_MSTR_WDATA_BURST (0x00000010)
#define SMMU_MSTR_WR_VA_OUT_OF_128BYTE (0x00000008)
#define SMMU_MSTR_WR_VA_OUT_OF_BOUNDARY (0x00000004)
#define SMMU_MSTR_RD_VA_OUT_OF_128BYTE (0x00000002)
#define SMMU_MSTR_RD_VA_OUT_OF_BOUNDARY (0x00000001)
#define SMMU_MSTR_INTCLR_ALL (SMMU_MSTR_WDATA_BURST \
	| SMMU_MSTR_WR_VA_OUT_OF_128BYTE \
	| SMMU_MSTR_WR_VA_OUT_OF_BOUNDARY \
	| SMMU_MSTR_RD_VA_OUT_OF_128BYTE \
	| SMMU_MSTR_RD_VA_OUT_OF_BOUNDARY)

#define SMMU_MSTR_INTCLR_ALL_UNMASK (0x00000000)
#define SMMU_MSTR_INTCLR_ALL_MASK (0x0000001f)
#define SMMU_MSTR_SMRX_0_LEN (0x00004000)
#define SMMU_MSTR_SMRX_START_ALL_STREAM (0x0000000f)
#define SMMU_INTCLR_NS_PTW_NS_STAT (0x00000020)
#define SMMU_INTCLR_NS_PTW_INVALID_STAT (0x00000010)
#define SMMU_INTCLR_NS_PTW_TRANS_STAT (0x00000008)
#define SMMU_INTCLR_NS_TLBMISS_STAT (0x00000004)
#define SMMU_INTCLR_NS_EXT_STAT (0x00000002)
#define SMMU_INTCLR_NS_PERMIS_STAT (0x00000001)
#define SMMU_COMMON_INTCLR_NS_ALL_MASK (0x0000003f)
#define SMMU_COMMON_INTCLR_NS_ALL (SMMU_INTCLR_NS_PTW_NS_STAT \
	| SMMU_INTCLR_NS_PTW_INVALID_STAT \
	| SMMU_INTCLR_NS_PTW_TRANS_STAT \
	| SMMU_INTCLR_NS_TLBMISS_STAT \
	| SMMU_INTCLR_NS_EXT_STAT \
	| SMMU_INTCLR_NS_PERMIS_STAT)
#define SMMU_CACHE_ALL_LEVEL_INVALID_LEVEL1 (0x00000003)
#define SMMU_CACHE_ALL_LEVEL_VALID_LEVEL1 (0x00000002)
#define SMMU_OPREF_CTRL_CONFIG_DUMMY (0x1)


struct smmu_master_reg_offset {
	unsigned int smmu_mstr_base_addr;
	unsigned int smmu_mstr_glb_bypass;
	unsigned int smmu_mstr_end_ack;
	unsigned int smmu_mstr_smrx_start;
	unsigned int smmu_mstr_inpt_sel;
	unsigned int smmu_mstr_intmask;
	unsigned int smmu_mstr_intstat;
	unsigned int smmu_mstr_intclr;
	unsigned int smmu_mstr_dbg_port_in_0;
	unsigned int smmu_mstr_smrx_0[IPU_SMMU_TOTAL_STREAM_ID_NUMBER];
	unsigned int read_cmd_total_cnt[IPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_cmd_miss_cnt[IPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_data_total_cnt[IPU_SMMU_READ_STREAM_NUMBER];
	unsigned int read_cmd_case_cnt[IPU_SMMU_TAG_COMPARE_CASE_NUMBER];
	unsigned int read_cmd_trans_latency;
	unsigned int write_cmd_total_cnt;
	unsigned int write_cmd_miss_cnt;
	unsigned int write_data_total_cnt;
	unsigned int write_cmd_case_cnt[IPU_SMMU_TAG_COMPARE_CASE_NUMBER];
	unsigned int write_cmd_trans_latency;
};

struct smmu_common_reg_offset {
	unsigned int smmu_common_base_addr;
	unsigned int smmu_scr;
	unsigned int smmu_intmask_ns;
	unsigned int smmu_intstat_ns;
	unsigned int smmu_intclr_ns;
	unsigned int smmu_cb_ttbr0;
	unsigned int smmu_cb_ttbcr;
	unsigned int smmu_scachei_all;
	unsigned int smmu_addr_msb;
	unsigned int smmu_err_rdaddr;
	unsigned int smmu_err_wraddr;
	unsigned int smmu_fama_ctrl1_ns;
	unsigned int override_pref_addr;
	unsigned int cfg_override_original_pref_addr;
};

struct smmu_master_reg_offset smmu_master_reg_offset;
struct smmu_common_reg_offset smmu_common_reg_offset;
struct smmu_irq_count smmu_irq_count;

struct ion_client *ipu_ion_client = NULL;

static struct iommu_domain *ipu_smmu_domain = 0;
static struct gen_pool *ipu_iova_pool;

struct smmu_manager {
	void __iomem *master_io_addr;
	void __iomem *common_io_addr;
};

struct smmu_manager smmu_manager;

DEFINE_MUTEX(ipu_pool_mutex);/*lint !e651 !e708 !e570 !e64 !e785 */

void ipu_reg_bit_write_dword(
				unsigned long reg_addr,
				unsigned int start_bit,
				unsigned int end_bit,
				unsigned int content)
{
	unsigned int set_value;
	unsigned int reg_content;
	unsigned int tmp_mask;
	unsigned int tmp_bit;

	if ((end_bit < start_bit)
		|| (start_bit > 31)
		|| (end_bit > 31)) {
		printk(KERN_ERR"[%s]: error input: reg_addr=%lx,start_bit=%x,end_bit=%x,content=%x\n",
			__func__, (unsigned long)reg_addr, start_bit, end_bit, content);
		return;
	}
	set_value	   = content;
	set_value	   = set_value << start_bit;

	tmp_bit 	   = 31 - end_bit;
	tmp_mask	   = 0xffffffff << tmp_bit;
	tmp_mask	   = tmp_mask >> ( start_bit + tmp_bit);
	tmp_mask	   = tmp_mask << start_bit;

	reg_content    = (unsigned int) ioread32((void *)reg_addr);
	reg_content   &= (~tmp_mask);
	set_value	  &= tmp_mask;
	iowrite32((reg_content | set_value), (void *)reg_addr);
	printk(KERN_DEBUG"[%s]: reg_content=%d, set_value= %d\n", __func__, reg_content, set_value);
	return;
}

/* get ptr of iommu domain when probe */
static int ipu_enable_iommu(struct device *dev)
{
	int ret;

	if (!dev) {
		printk(KERN_DEBUG"[%s] dev is NULL\n", __func__);
		return -EIO;
	}

	if (!ipu_ion_client) {
		ipu_ion_client = hisi_ion_client_create("ipu-client");
		if (!ipu_ion_client) {
			printk(KERN_ERR"[%s] hisi_ion_client_create fail\n", __func__);
			return -ENODEV;
		}
	}

	if (ipu_smmu_domain) {
		printk(KERN_DEBUG"[%s] ipu_smmu_domain is not NULL\n", __func__);
		return 0;
	}

	printk(KERN_DEBUG"[%s] dev->bus is %lx\n", __func__, (unsigned long)dev->bus);

	if (!iommu_present(dev->bus)) {
		printk(KERN_DEBUG"[%s] iommu not found\n", __func__);
		return 0;
	}

	ipu_smmu_domain = iommu_domain_alloc(dev->bus);

	if (0 == ipu_smmu_domain) {
		printk(KERN_ERR"[%s] iommu_domain_alloc fail\n", __func__);
		return -EIO;
	}

	printk(KERN_DEBUG"[%s] iommu_domain_alloc success,ipu_smmu_domain=%lx\n",
		__func__, (unsigned long)ipu_smmu_domain);

	ret = iommu_attach_device(ipu_smmu_domain, dev);

	if (ret) {
		printk(KERN_ERR"[%s] iommu_attach_device fail, ret=%d\n", __func__, ret);
		iommu_domain_free(ipu_smmu_domain);
		return -EIO;
	}

	printk(KERN_DEBUG"[%s] successfully\n", __func__);
	return 0;
}

static struct gen_pool *iova_pool_setup(unsigned long start,
		unsigned long size, unsigned long align)
{
	struct gen_pool *pool;
	int ret;

	pool = gen_pool_create((int)order_base_2(align), -1);/*lint !e666 !e835 !e747 !e516 !e866 !e712 */
	if (!pool) {
		printk(KERN_ERR"[%s] Create gen pool failed!\n", __func__);
		return NULL;
	}
	/* iova start should not be 0, because return
	   0 when alloc iova is considered as error */
	if (!start)
		printk(KERN_ERR"[%s] iova start should not be 0!\n", __func__);

	ret = gen_pool_add(pool, start, size, -1);
	if (ret) {
		printk(KERN_ERR"[%s] Gen pool add failed, ret=%x\n", __func__, ret);
		gen_pool_destroy(pool);
		return NULL;
	}

	return pool;
}

unsigned long ipu_get_smmu_base_phy(struct device *dev)
{
	struct iommu_domain_data *domain_data = 0;

	if (ipu_enable_iommu(dev)) {
		printk(KERN_DEBUG"[%s] ipu_enable_iommu fail and cannot get TTBR\n", __func__);
		return 0;
	}

	domain_data = (struct iommu_domain_data *)(ipu_smmu_domain->priv); /*lint !e838*/
	if (0 == domain_data) {
		printk(KERN_DEBUG"domain_data is 0\n");
		return 0;
	}

	ipu_iova_pool = iova_pool_setup(domain_data->iova_start,
			domain_data->iova_size, domain_data->iova_align);

	return ((unsigned long)domain_data->phy_pgd_base);
}

static int ipu_smmu_mstr_init(bool port_sel)
{
	unsigned long io_mstr_base = (unsigned long)smmu_manager.master_io_addr;
	unsigned int stream_status;
	int cnt = 0;

	printk(KERN_DEBUG"[%s] check mstr end ack start\n", __func__);

	/* set input signal as "register" by config SMMU_MSTR_INPT_SEL */
	if (port_sel) {
		iowrite32(SMMU_MSTR_INPUT_SEL_REGISTER, (void *)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_inpt_sel));
	}

	/* polling by loop read SMMU_MSTR_END_ACK */
	do {
		cnt++;
		stream_status = ioread32((void *)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_end_ack));

		if (cnt > SMMU_MSTR_END_ACK_THRESHOLD) {
			printk(KERN_DEBUG"[%s] check SMMU MSTR END ACK loop overflow\n", __func__);
			break;
		}
	} while((stream_status & 0xf) != SMMU_MSTR_ALL_STREAM_IS_END_ACK);

	printk(KERN_DEBUG"[%s] check mstr end ack end\n", __func__);

	/* set SMMU-normal mode */
	iowrite32(SMMU_MSTR_GLB_BYPASS_NORMAL_MODE, (void *)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_glb_bypass));

	/* here can config clk:
	   for core_clk_en, hardware open, for low-power ctrl
	   for apb_clk_en,	software open, for debug (if want to read cache/ram status in RTL)
	   default value is OK, so NO need to config again */

	/* clean interrupt, and NOT mask all interrupts by config SMMU_MSTR_INTCLR and SMMU_MSTR_INTMASK */
	iowrite32(SMMU_MSTR_INTCLR_ALL, (void *)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_intclr));
	iowrite32(SMMU_MSTR_INTCLR_ALL_UNMASK, (void *)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_intmask));

	/********************************************************
	config stream by SMMU_MSTR_SMRX_0
	for a sid, includes:
	VA max and VA min for this stream-id r/w region;
	len and upwin (in 32k, if VA continue increase, will not decrease in 32k, upwin is 0)
	len should be iid/2, iid(index id) is 8, for iid, for example, if pingpong buffer, iid is 2

	00.b:weight
	01.b:input read
	10.b:output read
	11.b:output write

	.len = iid/2=4
	.upwin = 0(do not search in upwin)
	.bypass = 0(no bypass)

	for malloc and free, VA is not in a designated area, so can not set VA max and VA min
	*********************************************************/
	iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_smrx_0[0]));
	iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_smrx_0[1]));
	iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_smrx_0[2]));
	iowrite32(SMMU_MSTR_SMRX_0_LEN, (void *)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_smrx_0[3]));

	/* stream startup by config SMMU_MSTR_SMRX_START */
	iowrite32(SMMU_MSTR_SMRX_START_ALL_STREAM, (void *)(io_mstr_base + smmu_master_reg_offset.smmu_mstr_smrx_start));
	printk(KERN_DEBUG"[%s] smmu=%lx\n", __func__, io_mstr_base);
	return 0;

}

static int ipu_smmu_comm_init(unsigned long ttbr0, unsigned long smmu_rw_err_phy_addr)
{
	unsigned int low;
	unsigned int high;
	unsigned long io_comm_base = (unsigned long)smmu_manager.common_io_addr;

	/* set SMMU mode as normal */
	ipu_reg_bit_write_dword(io_comm_base + smmu_common_reg_offset.smmu_scr, 0, 0, 0);

	/* clear SMMU interrupt(SMMU_INTCLR_NS) */
	iowrite32(SMMU_COMMON_INTCLR_NS_ALL, (void *)(io_comm_base + smmu_common_reg_offset.smmu_intclr_ns));

	/* clear MASK of interrupt(SMMU_INTMASK_NS) */
	ipu_reg_bit_write_dword(io_comm_base + smmu_common_reg_offset.smmu_intmask_ns, 0, 5, 0);

	/* set stream status: SMMU normal(SMMU_SMRX_NS).
	default value is OK, NO need to config again */

	/* set SMMU Translation Table Base Register for Non-Secure Context Bank0(SMMU_CB_TTBR0) */
	low = (unsigned int)(ttbr0 & 0xffffffff);
	high = (unsigned int)((ttbr0 >> 32) & 0x7f);
	iowrite32(low, (void *)(io_comm_base + smmu_common_reg_offset.smmu_cb_ttbr0));
	iowrite32(high, (void *)(io_comm_base + smmu_common_reg_offset.smmu_fama_ctrl1_ns));

	/* set Descriptor select of the SMMU_CB_TTBR0 addressed region of Non-Secure Context Bank
	for 64bit system, select Long Descriptor -> 1(SMMU_CB_TTBCR.cb_ttbcr_des) */
	ipu_reg_bit_write_dword(io_comm_base + smmu_common_reg_offset.smmu_cb_ttbcr, 0, 0, 0x1);

	/* set SMMU read/write phy addr in TLB miss case */
	low = (unsigned int)(smmu_rw_err_phy_addr & 0xffffffff);
	high = (unsigned int)((smmu_rw_err_phy_addr >> 32) & 0x7f);
	iowrite32(low, (void *)(io_comm_base + smmu_common_reg_offset.smmu_err_rdaddr));
	ipu_reg_bit_write_dword(io_comm_base + smmu_common_reg_offset.smmu_addr_msb, 0, 6, high);

	iowrite32(low, (void *)(io_comm_base + smmu_common_reg_offset.smmu_err_wraddr));
	ipu_reg_bit_write_dword(io_comm_base + smmu_common_reg_offset.smmu_addr_msb, 7, 13, high);
	return 0;
}

int ipu_smmu_init(unsigned long ttbr0, unsigned long smmu_rw_err_phy_addr, bool port_sel)
{
	printk(KERN_DEBUG"[%s]: ipu_smmu_mstr_init start\n", __func__);
	if (ipu_smmu_mstr_init(port_sel)) {
		printk(KERN_ERR"[%s]: ipu_smmu_mstr_init fail\n", __func__);
		return -EINVAL;
	}

	printk(KERN_DEBUG"[%s]ttbr0=%lx\n", __func__, ttbr0);

	if (0 == ttbr0) {
		return -EINVAL;
	}

	printk(KERN_DEBUG"[%s]: ipu_smmu_comm_init start\n", __func__);
	if (ipu_smmu_comm_init(ttbr0, smmu_rw_err_phy_addr)) {
		printk(KERN_ERR"[%s]: ipu_smmu_comm_init fail\n", __func__);
		return -EINVAL;
	}

	printk(KERN_DEBUG"[%s]: ipu_smmu_init successfully\n", __func__);

	return 0;
}

void ipu_smmu_deinit(void)
{
	iowrite32(SMMU_MSTR_INTCLR_ALL_MASK,
		(void *)((unsigned long)smmu_manager.master_io_addr + smmu_master_reg_offset.smmu_mstr_intmask));
	iowrite32(SMMU_COMMON_INTCLR_NS_ALL_MASK,
		(void *)((unsigned long)smmu_manager.common_io_addr + smmu_common_reg_offset.smmu_intmask_ns));
}

static unsigned long ipu_alloc_iova(struct gen_pool *pool,
		unsigned long size)
{
	unsigned long iova;

	mutex_lock(&ipu_pool_mutex);

	iova = gen_pool_alloc(pool, size);
	if (!iova) {
		mutex_unlock(&ipu_pool_mutex);
		printk(KERN_ERR"[%s]: hisi iommu gen_pool_alloc failed! size = %lu\n", __func__, size);
		return 0;
	}
	mutex_unlock(&ipu_pool_mutex);
	return iova;
}

static void ipu_free_iova(struct gen_pool *pool,
		unsigned long iova, size_t size)
{
	mutex_lock(&ipu_pool_mutex);
	gen_pool_free(pool, iova, size);

	mutex_unlock(&ipu_pool_mutex);
}

size_t ipu_smmu_map(struct scatterlist *sgl, unsigned long size,
		 struct map_format *format)
{
	size_t ret;
	unsigned long phys_len;
	unsigned long iova_size;
	unsigned long iova_start;
	struct scatterlist *sg;

	(void)size;

	for (phys_len = 0, sg = sgl; sg; sg = sg_next(sg))
		phys_len += (unsigned long)ALIGN(sg->length, PAGE_SIZE);/*lint !e50 */

	iova_size = phys_len;
	iova_start = ipu_alloc_iova(ipu_iova_pool, iova_size);

	ret = iommu_map_sg(ipu_smmu_domain, iova_start, sgl,
			   (unsigned int)sg_nents(sgl), format->prot);

	if (ret != iova_size) {
		printk(KERN_ERR"[%s]: map failed!iova_start = %lx, iova_size = %lx\n",
				__func__, iova_start, iova_size);

		if (ipu_iova_pool) {
			ipu_free_iova(ipu_iova_pool, iova_start, iova_size);
		}

		return ret;
	}
	format->iova_start = iova_start;
	format->iova_size = iova_size;
	return 0;
}

long ipu_smmu_unmap(struct map_format *format)
{
	int ret;
	unsigned long unmaped_size;

	/* free iova */
	ret = addr_in_gen_pool(ipu_iova_pool, format->iova_start,
			format->iova_size);
	if(!ret) {
		printk(KERN_ERR"[%s]illegal para!!iova = %lx, size = %lx\n",
				__func__, format->iova_start, format->iova_size);
		return -EIO;
	}

	unmaped_size = iommu_unmap(ipu_smmu_domain,
			format->iova_start, format->iova_size);

	if (unmaped_size != format->iova_size) {
		printk(KERN_ERR"[%s]unmap failed!\n", __func__);
		return -EINVAL;
	}

	ipu_free_iova(ipu_iova_pool, format->iova_start, format->iova_size);

	return 0;
}

void ipu_smmu_ioremap(void)
{
	smmu_manager.master_io_addr = ioremap((unsigned long)smmu_master_reg_offset.smmu_mstr_base_addr, (unsigned long)0xffff);
	smmu_manager.common_io_addr = ioremap((unsigned long)smmu_common_reg_offset.smmu_common_base_addr, (unsigned long)0xffff);
}

void ipu_smmu_iounmap(void)
{
	iounmap(smmu_manager.master_io_addr);
	iounmap(smmu_manager.common_io_addr);
}

void ipu_smmu_override_prefetch_addr(unsigned long va_addr)
{
	iowrite32(va_addr, (void *)((unsigned long)smmu_manager.common_io_addr
		+ (unsigned long)smmu_common_reg_offset.override_pref_addr));
	iowrite32(SMMU_OPREF_CTRL_CONFIG_DUMMY, (void *)((unsigned long)smmu_manager.common_io_addr
		+ (unsigned long)smmu_common_reg_offset.cfg_override_original_pref_addr));

	printk(KERN_ERR"[%s] done\n", __func__);
}

bool ipu_smmu_interrupt_handler(void)
{
	unsigned int reg_smmu_mstr_status;
	unsigned int reg_smmu_comm_status;
	bool ret = false;
	unsigned long mstr_io_addr = (unsigned long)smmu_manager.master_io_addr;
	unsigned long comm_io_addr = (unsigned long)smmu_manager.common_io_addr;
	struct smmu_irq_count *irq_count = &smmu_irq_count;

	//fixme: if security/protect mode is needed, add code here
	reg_smmu_comm_status = ioread32((void *)(comm_io_addr + smmu_common_reg_offset.smmu_intstat_ns));
	reg_smmu_mstr_status = ioread32((void *)(mstr_io_addr + smmu_master_reg_offset.smmu_mstr_intstat));

	if (0 != reg_smmu_mstr_status) {
		ret = true;
		printk(KERN_ERR"[%s]: error, smmu common interrupt received: %x\n", __func__, reg_smmu_mstr_status);
		if (reg_smmu_mstr_status & SMMU_MSTR_WDATA_BURST) {
			irq_count->mstr_wdata_burst++;
		}
		if (reg_smmu_mstr_status & SMMU_MSTR_WR_VA_OUT_OF_128BYTE) {
			irq_count->mstr_wr_va_out_of_128byte++;
		}
		if (reg_smmu_mstr_status & SMMU_MSTR_WR_VA_OUT_OF_BOUNDARY) {
			irq_count->mstr_wr_va_out_of_boundary++;
		}
		if (reg_smmu_mstr_status & SMMU_MSTR_RD_VA_OUT_OF_128BYTE) {
			irq_count->mstr_rd_va_out_of_128byte++;
		}
		if (reg_smmu_mstr_status & SMMU_MSTR_RD_VA_OUT_OF_BOUNDARY) {
			irq_count->mstr_rd_va_out_of_boundary++;
		}
		/* clear smmu mstr interrupt */
		iowrite32(SMMU_MSTR_INTCLR_ALL, (void *)(mstr_io_addr + (unsigned long)smmu_master_reg_offset.smmu_mstr_intclr));
	}

	if (0 != reg_smmu_comm_status) {
		ret = true;
		printk(KERN_ERR"[%s]: error, smmu common interrupt received: %x\n", __func__, reg_smmu_comm_status);

		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PTW_NS_STAT) {
			/* When PTW transaction receive an page table whose ns bit is not match to the prefetch
			transaction, occur this fault. */
			irq_count->comm_ptw_ns_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PTW_INVALID_STAT) {
			/* When PTW transaction receive an invalid page table descriptor or access the invalid
			regoin between t0sz and t1sz in long descriptor mode, occur this fault.*/
			irq_count->comm_ptw_invalid_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PTW_TRANS_STAT) {
			/* When PTW transaciont receive an error response, occur this fault. */
			irq_count->comm_ptw_trans_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_TLBMISS_STAT) {
			/* When there is a TLB miss generated during the translation process, the mmu will record this. */
			irq_count->comm_tlbmiss_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_EXT_STAT) {
			/* When mmu receive an en error response the mmu will record this as a fault. */
			irq_count->comm_ext_stat++;
		}
		if (reg_smmu_comm_status & SMMU_INTCLR_NS_PERMIS_STAT) {
			/* When the input transaction¡¯s attributes doesn¡¯t match the attributes descripted in the page table,
			the mmu will raise a fault for this. */
			irq_count->comm_permis_stat++;
		}

		/* clear smmu interrupt */
		//fixme: if security/protect mode is needed, add code here
		iowrite32(SMMU_COMMON_INTCLR_NS_ALL, (void *)(comm_io_addr + (unsigned long)smmu_common_reg_offset.smmu_intclr_ns));
	}

	return ret;
}

unsigned int ipu_smmu_reset_statistic(void)
{
	int i;
	struct smmu_master_reg_offset *offset = &smmu_master_reg_offset;
	void *dbg_port_in = (void *)((unsigned long)smmu_manager.master_io_addr
		+ (unsigned long)offset->smmu_mstr_dbg_port_in_0);

	memset(&smmu_irq_count, 0, sizeof(smmu_irq_count));// coverity[secure_coding]

	/* clean read channel cmd-total-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < IPU_SMMU_READ_STREAM_NUMBER; i++) {
		iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->read_data_total_cnt[i]), dbg_port_in);
	}

	/* clean read channel cmd-miss-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < IPU_SMMU_READ_STREAM_NUMBER; i++) {
		iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->read_cmd_miss_cnt[i]), dbg_port_in);
	}

	/* clean read channel data-length-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < IPU_SMMU_READ_STREAM_NUMBER; i++) {
		iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->read_data_total_cnt[i]), dbg_port_in);
	}

	/* clean read channel tag-stat (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < IPU_SMMU_TAG_COMPARE_CASE_NUMBER; i++) {
		iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->read_cmd_case_cnt[i]), dbg_port_in);
	}

	/* clean read channel latency (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->read_cmd_trans_latency), dbg_port_in);

	/* clean write channel cmd-total-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_cmd_total_cnt), dbg_port_in);

	/* clean write channel cmd-miss-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_cmd_miss_cnt), dbg_port_in);

	/* clean write channel data-length-count (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_data_total_cnt), dbg_port_in);

	/* clean write channel tag-stat (by config SMMU_MSTR_DBG_PORT_IN_0) */
	for(i = 0; i < IPU_SMMU_TAG_COMPARE_CASE_NUMBER; i++) {
		iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_cmd_case_cnt[i]), dbg_port_in);
	}

	/* clean write channel latency (by config SMMU_MSTR_DBG_PORT_IN_0) */
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_cmd_trans_latency), dbg_port_in);
	iowrite32((SMMU_MSTR_SET_DEBUG_PORT + offset->write_cmd_trans_latency), dbg_port_in);

	return 0;
}

unsigned int ipu_smmu_report_statistic(struct smmu_statistic *statistic)
{
	struct smmu_master_reg_offset *offset = &smmu_master_reg_offset;
	unsigned long mstr_io_addr = (unsigned long)smmu_manager.master_io_addr;

	/* read channel cmd total count */
	statistic->read_stream_cmd_total[0] = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_total_cnt[0]));
	statistic->read_stream_cmd_total[1] = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_total_cnt[1]));
	statistic->read_stream_cmd_total[2] = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_total_cnt[2]));

	/* read channel cmd miss count */
	statistic->read_stream_cmd_miss[0] = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_miss_cnt[0]));
	statistic->read_stream_cmd_miss[1] = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_miss_cnt[1]));
	statistic->read_stream_cmd_miss[2] = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_miss_cnt[2]));

	/* read channel data total count */
	statistic->read_stream_data_total[0] = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_data_total_cnt[0]));
	statistic->read_stream_data_total[1] = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_data_total_cnt[1]));
	statistic->read_stream_data_total[2] = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_data_total_cnt[2]));

	/* read cmd miss/hit and latency */
	statistic->read_stream_cmd_miss_valid = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[0]));
	statistic->read_stream_cmd_miss_pending = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[1]));
	statistic->read_stream_cmd_hit_valid_not_slide_window = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[2]));
	statistic->read_stream_cmd_hit_valid_slide_window = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[3]));
	statistic->read_stream_cmd_hit_pending_not_slide_window = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[4]));
	statistic->read_stream_cmd_hit_pending_slide_window = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_case_cnt[5]));
	statistic->read_stream_cmd_latency = ioread32((void *)(mstr_io_addr + (unsigned long)offset->read_cmd_trans_latency));

	/* write channel cmd total count */
	statistic->write_stream_cmd_total = ioread32((void *)(mstr_io_addr + (unsigned long)offset->write_cmd_total_cnt));
	statistic->write_stream_cmd_miss = ioread32((void *)(mstr_io_addr + (unsigned long)offset->write_cmd_miss_cnt));
	statistic->write_stream_data_total = ioread32((void *)(mstr_io_addr + (unsigned long)offset->write_data_total_cnt));

	/* write cmd miss/hit and latency */
	statistic->write_stream_cmd_miss_valid = ioread32((void *)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[0]));
	statistic->write_stream_cmd_miss_pending = ioread32((void *)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[1]));
	statistic->write_stream_cmd_hit_valid_not_slide_window = ioread32((void *)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[2]));
	statistic->write_stream_cmd_hit_valid_slide_window = ioread32((void *)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[3]));
	statistic->write_stream_cmd_hit_pending_not_slide_window = ioread32((void *)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[4]));
	statistic->write_stream_cmd_hit_pending_slide_window = ioread32((void *)(mstr_io_addr + (unsigned long)offset->write_cmd_case_cnt[5]));
	statistic->write_stream_cmd_latency = ioread32((void *)(mstr_io_addr + (unsigned long)offset->write_cmd_trans_latency));

	memcpy(&statistic->smmu_irq_count, &smmu_irq_count, sizeof(smmu_irq_count));// coverity[secure_coding]

	return 0;
}

/* for online layer-by-layer mode, run once each op(i.e. conv, pooling, ReLU...), while online merge and offline mode
this func will only call once before run */
unsigned int ipu_smmu_pte_update(void)
{
	unsigned long mstr_io_addr = (unsigned long)smmu_manager.master_io_addr;
	unsigned long comm_io_addr = (unsigned long)smmu_manager.common_io_addr;

	iowrite32(SMMU_MSTR_SMRX_START_ALL_STREAM, (void *)(mstr_io_addr + smmu_master_reg_offset.smmu_mstr_smrx_start));

	/* update cache data to avoid this case: phy address across 8GB address-space */
	iowrite32(SMMU_CACHE_ALL_LEVEL_INVALID_LEVEL1, (void *)(comm_io_addr + smmu_common_reg_offset.smmu_scachei_all));
	iowrite32(SMMU_CACHE_ALL_LEVEL_VALID_LEVEL1, (void *)(comm_io_addr + smmu_common_reg_offset.smmu_scachei_all));

	return 0;
}

bool ipu_smmu_master_get_offset(struct device *dev)
{
	struct smmu_master_reg_offset *offset = &smmu_master_reg_offset;
	struct device_node *node = of_find_node_by_name(dev->of_node, "smmu_master");
	if(!node) {
		printk(KERN_ERR"[%s]: find smmu_master node error\n", __func__);
		return false;
	}

	memset(offset, 0, sizeof(*offset));// coverity[secure_coding]
	of_property_read_u32(node, "smmu-mstr-base-addr", &offset->smmu_mstr_base_addr);
	of_property_read_u32(node, "smmu-mstr-glb-bypass", &offset->smmu_mstr_glb_bypass);
	of_property_read_u32(node, "smmu-mstr-end-ack", &offset->smmu_mstr_end_ack);
	of_property_read_u32(node, "smmu-mstr-smrx-start", &offset->smmu_mstr_smrx_start);
	of_property_read_u32(node, "smmu-mstr-inpt-sel", &offset->smmu_mstr_inpt_sel);
	of_property_read_u32(node, "smmu-mstr-intmask", &offset->smmu_mstr_intmask);
	of_property_read_u32(node, "smmu-mstr-intstat", &offset->smmu_mstr_intstat);
	of_property_read_u32(node, "smmu-mstr-intclr", &offset->smmu_mstr_intclr);
	of_property_read_u32(node, "smmu-mstr-dbg-port-in-0", &offset->smmu_mstr_dbg_port_in_0);
	of_property_read_u32(node, "smmu-mstr-smrx-0-stream-0", &offset->smmu_mstr_smrx_0[0]);
	of_property_read_u32(node, "smmu-mstr-smrx-0-stream-1", &offset->smmu_mstr_smrx_0[1]);
	of_property_read_u32(node, "smmu-mstr-smrx-0-stream-2", &offset->smmu_mstr_smrx_0[2]);
	of_property_read_u32(node, "smmu-mstr-smrx-0-stream-3", &offset->smmu_mstr_smrx_0[3]);
	of_property_read_u32(node, "read-cmd-total-cnt-stream-0", &offset->read_cmd_total_cnt[0]);
	of_property_read_u32(node, "read-cmd-total-cnt-stream-1", &offset->read_cmd_total_cnt[1]);
	of_property_read_u32(node, "read-cmd-total-cnt-stream-2", &offset->read_cmd_total_cnt[2]);
	of_property_read_u32(node, "read-cmd-miss-cnt-stream-0", &offset->read_cmd_miss_cnt[0]);
	of_property_read_u32(node, "read-cmd-miss-cnt-stream-1", &offset->read_cmd_miss_cnt[1]);
	of_property_read_u32(node, "read-cmd-miss-cnt-stream-2", &offset->read_cmd_miss_cnt[2]);
	of_property_read_u32(node, "read-data-total-cnt-stream-0", &offset->read_data_total_cnt[0]);
	of_property_read_u32(node, "read-data-total-cnt-stream-1", &offset->read_data_total_cnt[1]);
	of_property_read_u32(node, "read-data-total-cnt-stream-2", &offset->read_data_total_cnt[2]);
	of_property_read_u32(node, "read-cmd-case-cnt-stream-0", &offset->read_cmd_case_cnt[0]);
	of_property_read_u32(node, "read-cmd-case-cnt-stream-1", &offset->read_cmd_case_cnt[1]);
	of_property_read_u32(node, "read-cmd-case-cnt-stream-2", &offset->read_cmd_case_cnt[2]);
	of_property_read_u32(node, "read-cmd-case-cnt-stream-3", &offset->read_cmd_case_cnt[3]);
	of_property_read_u32(node, "read-cmd-case-cnt-stream-4", &offset->read_cmd_case_cnt[4]);
	of_property_read_u32(node, "read-cmd-case-cnt-stream-5", &offset->read_cmd_case_cnt[5]);
	of_property_read_u32(node, "read-cmd-trans-latency", &offset->read_cmd_trans_latency);
	of_property_read_u32(node, "write-cmd-total-cnt", &offset->write_cmd_total_cnt);
	of_property_read_u32(node, "write-cmd-miss-cnt", &offset->write_cmd_miss_cnt);
	of_property_read_u32(node, "write-data-total-cnt", &offset->write_data_total_cnt);
	of_property_read_u32(node, "write-cmd-case-cnt-stream-0", &offset->write_cmd_case_cnt[0]);
	of_property_read_u32(node, "write-cmd-case-cnt-stream-1", &offset->write_cmd_case_cnt[1]);
	of_property_read_u32(node, "write-cmd-case-cnt-stream-2", &offset->write_cmd_case_cnt[2]);
	of_property_read_u32(node, "write-cmd-case-cnt-stream-3", &offset->write_cmd_case_cnt[3]);
	of_property_read_u32(node, "write-cmd-case-cnt-stream-4", &offset->write_cmd_case_cnt[4]);
	of_property_read_u32(node, "write-cmd-case-cnt-stream-5", &offset->write_cmd_case_cnt[5]);
	of_property_read_u32(node, "write-cmd-trans-latency", &offset->write_cmd_trans_latency);
	return true;
}

bool ipu_smmu_common_get_offset (struct device *dev)
{
	struct smmu_common_reg_offset *offset = &smmu_common_reg_offset;
	struct device_node *node = of_find_node_by_name(dev->of_node, "smmu_common");
	if(!node) {
		printk(KERN_ERR"[%s]: find smmu_common node error\n", __func__);
		return false;
	}

	memset(offset, 0, sizeof(smmu_common_reg_offset));// coverity[secure_coding]
	of_property_read_u32(node, "smmu-common-base-addr", &offset->smmu_common_base_addr);
	of_property_read_u32(node, "smmu-scr", &offset->smmu_scr);
	of_property_read_u32(node, "smmu-intmask-ns", &offset->smmu_intmask_ns);
	of_property_read_u32(node, "smmu-intstat-ns", &offset->smmu_intstat_ns);
	of_property_read_u32(node, "smmu-intclr-ns", &offset->smmu_intclr_ns);
	of_property_read_u32(node, "smmu-cb-ttbr0", &offset->smmu_cb_ttbr0);
	of_property_read_u32(node, "smmu-cb-ttbcr", &offset->smmu_cb_ttbcr);
	of_property_read_u32(node, "smmu-scachei-all", &offset->smmu_scachei_all);
	of_property_read_u32(node, "smmu-fama-ctrl1-ns", &offset->smmu_fama_ctrl1_ns);
	of_property_read_u32(node, "smmu-opref-addr", &offset->override_pref_addr);
	of_property_read_u32(node, "smmu-opref-ctrl", &offset->cfg_override_original_pref_addr);
	of_property_read_u32(node, "smmu-addr-msb", &offset->smmu_addr_msb);
	of_property_read_u32(node, "smmu-err-rdaddr", &offset->smmu_err_rdaddr);
	of_property_read_u32(node, "smmu-err-wraddr", &offset->smmu_err_wraddr);
	return true;
}

