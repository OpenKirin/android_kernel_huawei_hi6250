#include <linux/module.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/workqueue.h>
#include <linux/of_irq.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>

#include "pimon_mntn.h"

static struct s_pimon_mgr_info g_pimon_mgr;
/*保存blk真实物理地址*/
static u64 blks_phys_addr[PIMON_BLOCK_NUM_MAX] = {0};
/*保存blk真实容量*/
static u32 blks_phys_size[PIMON_BLOCK_NUM_MAX] = {0};
/*保存blk映射地址*/
static void __iomem *blks_map_addr[PIMON_BLOCK_NUM_MAX] = {NULL};
/*延迟执行保存数据的工作和工作队列*/
static struct work_struct pimon_save_data_w;
static struct workqueue_struct *pimon_save_data_wq;
static u32 filename_num;

/********************************************************************
*Function:	pimon_mntn_get_irq_from_dts
*Description:	从dts中获取pimon中断号
*Input:		NA
*Output:	NA
*Return:	0:获取成功;1:获取失败
********************************************************************/
static int pimon_mntn_get_irq_from_dts(void)
{
	unsigned int irq;
	struct device_node *np;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,pimon_mntn");
	if (!np) {
		pr_err("[%s], cannot find pimon node in dts!\n",
			__func__);
		return 1;
	}

	irq = irq_of_parse_and_map(np, 0);
	if (!irq) {
		pr_err("%s: irq_of_parse_and_map failed, ret:%d\n",
			__func__, irq);
		return 1;
	}

	g_pimon_mgr.irq = irq;
	return 0;
}

/********************************************************************
*Function:	pimon_mntn_debug_show
*Description:	打印pimon头信息、block信息和寄存器信息
*Input:		NA
*Output:	NA
*Return:	NA
********************************************************************/
void pimon_mntn_debug_show(void)
{
	u32 i, reg_value;
	/*打印头信息*/
	pr_err("[%s]: The head info of pimon data:\n",
		__func__);
	pr_err("[%s]: tag_start=[0x%llx].\n",
		__func__, g_pimon_mgr.pimon_data_head.tag_start);
	pr_err("[%s]: vref_l=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.vref_l);
	pr_err("[%s]: vref_h=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.vref_h);
	pr_err("[%s]: pi_dc_sel=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.pi_dc_sel);
	pr_err("[%s]: channel_id=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.channel_id);
	pr_err("[%s]: ov_threshold=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.ov_threshold);
	pr_err("[%s]: uv_filter_pionts=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.uv_filter_pionts);
	pr_err("[%s]: uv_threshold=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.uv_threshold);
	pr_err("[%s]: pimon_trig=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.pimon_trig);
	pr_err("[%s]: reserved_one=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.reserved_one);
	pr_err("[%s]: reserved_two=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.reserved_two);
	pr_err("[%s]: reserved_three=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.reserved_three);
	pr_err("[%s]: ov_filter_pionts=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.ov_filter_pionts);
	pr_err("[%s]: reserved_four=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.reserved_four);
	pr_err("[%s]: reserved_five=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.reserved_five);
	pr_err("[%s]: reserved_six=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.reserved_six);
	pr_err("[%s]: reserved_serven=[0x%x].\n",
		__func__, g_pimon_mgr.pimon_data_head.reserved_serven);
	/*打印block信息*/
	for (i = 0; i < PIMON_BLOCK_NUM_MAX; i++) {
		pr_err("[%s]: pimon_block_info, block%d: addr=[0x%x], size=[0x%x]!\n",
			__func__, i, g_pimon_mgr.blocks[i].addr,
			g_pimon_mgr.blocks[i].size);
	}
	/*打印寄存器信息*/
	reg_value = readl_relaxed((u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_LAST_WADDR_LO);
	pr_err("[%s]: last_waddr_lo reg value=[0x%x].\n",
		__func__, reg_value);
	reg_value = readl_relaxed((u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_FIFO_STAT);
	pr_err("[%s]: fifo_stat reg value=[0x%x].\n",
		__func__, reg_value);
}

/********************************************************************
*Function:	pimon_mntn_get_blk_real_addr
*Description:	获取blk的真实物理地址
*Input:		blk_addr:块地址寄存器值
*Output:	NA
*Return:	blk真实物理地址
********************************************************************/
static u64 pimon_mntn_get_blk_real_addr(u32 blk_addr)
{
	u64 real_addr;

	real_addr = (u64)(blk_addr & PIMON_REG_BIT_MASK_REAL_ADDR);
	real_addr = real_addr << PIMON_REG_BIT_REAL_ADDR;
	return real_addr;
}

/********************************************************************
*Function:	pimon_mntn_get_blk_real_addr
*Description:	获取blk的真实容量
*Input:		blk_addr:块容量寄存器值
*Output:	NA
*Return:	blk真实容量
********************************************************************/
static u32 pimon_mntn_get_blk_real_size(u32 blk_size)
{
	u32 real_size;

	real_size = blk_size & PIMON_REG_BIT_MASK_REAL_SIZE;
	real_size = (real_size + 1) << PIMON_REG_BIT_REAL_SIZE;
	return real_size;
}

/********************************************************************
*Function:	pimon_mntn_set_block_addr
*Description:	设置block的addr
*Input:		id:block id
		reg_offset: 寄存器地址偏移
		reg_value:写入寄存器的值
*Output:	NA
*Return:	0:设置成功;1:设置失败
********************************************************************/
int pimon_mntn_set_block_addr(u32 id, u32 reg_offset, u32 reg_value)
{
	if (id >= PIMON_BLOCK_NUM_MAX) {
		pr_err("[%s]: blk id is invalid!\n",
			__func__);
		return 1;
	}
	/*写寄存器*/
	pr_err("[%s]: write reg_base_addr:0x%llx!reg_offset:0x%x\n",
		__func__, (u64)g_pimon_mgr.reg_base_addr, reg_offset);
	writel_relaxed(reg_value, (u8 __iomem *)g_pimon_mgr.reg_base_addr +
		reg_offset); /*lint !e792*/
	pr_err("[%s]: writel_relaxed success!\n",
		__func__);

	g_pimon_mgr.blocks[id].addr = reg_value;
	blks_phys_addr[id] = pimon_mntn_get_blk_real_addr(reg_value);
	return 0;
}

/********************************************************************
*Function:	pimon_mntn_set_block_size
*Description:	设置block的size
*Input:		id:block id
		reg_offset: 寄存器地址偏移
		reg_value:写入寄存器的值
*Output:	NA
*Return:	0:设置成功;1:设置失败
********************************************************************/
int pimon_mntn_set_block_size(u32 id, u32 reg_offset, u32 reg_value)
{
	if (id >= PIMON_BLOCK_NUM_MAX) {
		pr_err("[%s]: blk id is invalid!\n",
			__func__);
		return 1;
	}
	/*写寄存器*/
	pr_err("[%s]: write reg_base_addr:0x%llx!reg_offset:0x%x\n",
		__func__, (u64)g_pimon_mgr.reg_base_addr, reg_offset);
	writel_relaxed(reg_value, (u8 __iomem *)g_pimon_mgr.reg_base_addr +
		reg_offset); /*lint !e792*/
	pr_err("[%s]: writel_relaxed success!\n",
		__func__);

	g_pimon_mgr.blocks[id].size = reg_value;
	blks_phys_size[id] = pimon_mntn_get_blk_real_size(reg_value);
	/*映射blk地址,脚本控制先设置blk addr后设置blk size*/
	pr_err("[%s]: map blks_phys_addr:0x%llx!blks_phys_size:0x%x\n",
		__func__, blks_phys_addr[id], blks_phys_size[id]);

	if (pfn_valid(blks_phys_addr[id] >> PAGE_SHIFT)) {
		blks_map_addr[id]  = bbox_vmap(blks_phys_addr[id],
			(size_t)blks_phys_size[id]);
	} else {
		blks_map_addr[id]  = ioremap_wc(blks_phys_addr[id],
			(size_t)blks_phys_size[id]);
	}

	if (NULL == blks_map_addr[id]) {
		pr_err("[%s]: map blk%u addr failed!phys_addr:0x%llx, phys_size:0x%x.\n",
			__func__, id, blks_phys_addr[id], blks_phys_size[id]);
		return 1;
	}
	pr_err("[%s]: map blk%u success!map addr: 0x%llx;map size: 0x%x\n",
		__func__, id, (u64)blks_map_addr[id], blks_phys_size[id]);
	return 0;
}

/********************************************************************
*Function:	pimon_mntn_set_value
*Description:	设置测试类型、寄存器值、基地址
*Input:		id:设置类型id: 0测试类型;1寄存器值;2基地址
		value1:id取0表示测试类型id;
			id取1表示寄存器地址偏移
			id取2表示寄存器基地址
		value2:id取0为无效值;
			id取1表示写入寄存器的值
			id取2表示寄存器地址段的大小
*Output:	NA
*Return:	0:设置成功;1:设置失败
********************************************************************/
int pimon_mntn_set_value(u32 id, u32 value1, u32 value2)
{
	if (id >= PIMON_SET_MAX) {
		pr_err("[%s]: set_type id is invalid!id:%u.\n", __func__, id);
		return 1;
	}
	/*脚本控制先设置基地址，后设置各寄存器值*/
	switch (id) {
	case PIMON_SET_TEST_TYPE:
		if (value1 >= PIMON_TEST_MAX) {
			pr_err("[%s]: test_type id is invalid!id:%u.\n",
				__func__, value1);
			return 1;
		}
		g_pimon_mgr.pimon_test_type_cur =
			(enum e_pimon_test_type)value1;
		break;
	case PIMON_SET_REG:
		/*写寄存器*/
		pr_err("[%s]: write reg_base_addr:0x%llx!reg_offset:0x%x\n",
			__func__, (u64)g_pimon_mgr.reg_base_addr, value1);
		writel_relaxed(value2, (u8 __iomem *)g_pimon_mgr.reg_base_addr +
			value1); /*lint !e792*/
		pr_err("[%s]: writel_relaxed success!\n",
			__func__);
		switch (value1) {
		case PIMON_REG_CTRL: /*pimon控制寄存器*/
			/* 保存头信息的pimon_trig值*/
			g_pimon_mgr.pimon_data_head.pimon_trig =
				(u16)((value2 & PIMON_REG_BIT_MASK_TRIG) >>
				PIMON_REG_BIT_TRIG);
			break;
		case PIMON_REG_ANA_CFG:
			/*保存头信息的channel_id、pi_dc_sel值*/
			g_pimon_mgr.pimon_data_head.channel_id =
				(u16)((value2 & PIMON_REG_BIT_MASK_CHANNEL) >>
				PIMON_REG_BIT_CHANNEL);
			g_pimon_mgr.pimon_data_head.pi_dc_sel =
				(u16)((value2 & PIMON_REG_BIT_MASK_PIDCSEL) >>
				PIMON_REG_BIT_PIDCSEL);
			break;
		case PIMON_REG_ANA_SET:
			/*保存头信息的vref_h、vref_l值*/
			g_pimon_mgr.pimon_data_head.vref_h =
				(u16)((value2 & PIMON_REG_BIT_MASK_VREF_H) >>
				PIMON_REG_BIT_VREF_H);
			g_pimon_mgr.pimon_data_head.vref_l =
				(u16)((value2 & PIMON_REG_BIT_MASK_VREF_L) >>
				PIMON_REG_BIT_VREF_L);
			break;
		case PIMON_REG_THRESHOLD:
			/*保存头信息的uv_threshold、ov_threshold值*/
			g_pimon_mgr.pimon_data_head.uv_threshold =
				(u16)((value2 & PIMON_REG_BIT_MASK_UV_TH) >>
				PIMON_REG_BIT_UV_TH);
			g_pimon_mgr.pimon_data_head.ov_threshold =
				(u16)((value2 & PIMON_REG_BIT_MASK_OV_TH) >>
				PIMON_REG_BIT_OV_TH);
			break;
		case PIMON_REG_TRIG_CNT:
			/*保存头信息的uv_filter_pionts、ov_filter_pionts值*/
			g_pimon_mgr.pimon_data_head.uv_filter_pionts =
				(u16)((value2 & PIMON_REG_BIT_MASK_UV_CNT) >>
				PIMON_REG_BIT_UV_CNT);
			g_pimon_mgr.pimon_data_head.ov_filter_pionts =
				(u16)((value2 & PIMON_REG_BIT_MASK_OV_CNT) >>
				PIMON_REG_BIT_OV_CNT);
			break;
		default:
			break;
		}
		break;
	case PIMON_SET_BASE_ADDR:
		pr_err("[%s]: map reg_base_addr:0x%x!reg_base_size:0x%x\n",
			__func__, value1, value2);
		g_pimon_mgr.reg_base_addr = ioremap((u64)value1,
			(size_t)value2);
		if (NULL == g_pimon_mgr.reg_base_addr) {
			pr_err("[%s]: map reg_base_addr failed!\n",
				__func__);
			return 1;
		}
		pr_err("[%s]: map reg_base_addr success!\n",
			__func__);
		break;
	default:
		break;
	}

	return 0;
}

/********************************************************************
*Function:	pimon_mntn_close
*Description:	关闭FIFO、PIMON
*Input:		void
*Output:	NA
*Return:	0:创建成功;1:创建失败
********************************************************************/
static void pimon_mntn_close(void)
{
	u32 ret;

	pr_err("[%s]: close fifo pimon start!\n",
		__func__);
	ret = readl_relaxed((u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_CTRL);
	ret = ret & (~PIMON_CLOSE_FIFO_MASK);
	pr_err("[%s]: close fifo, value:0x%x!\n",
		__func__, ret);
	writel_relaxed(ret, (u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_CTRL); /*lint !e792*/
	ret = readl_relaxed((u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_GLB_STAT);
	pr_err("[%s]: glb_stat:0x%x\n",
		__func__, ret);
	pr_err("[%s]: close fifo success!\n",
		__func__);
}

/********************************************************************
*Function:	pimon_create_dir
*Description:	建立指定目录
*Input:		path:待创建目录名
		user:目录所有者id
		group:目录所有群组id
*Output:	NA
*Return:	0:创建成功; 1:创建失败
********************************************************************/
static int pimon_create_dir(char *path, uid_t user, gid_t group)
{
	int fd, ret;

	if (NULL == path) {
		pr_err("[%s]: path is null!\n",
			__func__);
		return 1;
	}

	fd = (int)sys_access(path, 0);
	if (0 != fd) {
		pr_err("[%s]: need create dir %s !\n",
			__func__, path);
		fd = (int)sys_mkdir(path, DIR_LIMIT);
		if (fd < 0) {
			pr_err("[%s]: create dir %s failed!ret:%d.\n",
				__func__, path, fd);
			return 1;
		}

		ret = (int)sys_chown((const char __user *)path, user, group);
		if (ret)
			pr_err("[%s]: chown %s uid [%d] gid [%d] failed!ret:%d.\n",
				__func__, path, user, group, ret);

		pr_err("[%s]: create dir %s successed!ret:%d.\n",
			__func__, path, fd);
	}

	return 0;
}

/********************************************************************
*Function:	pimon_create_dir_recursive
*Description:	递归建立指定目录
*Input:		path:待创建目录名
		user:目录所有者id
		group:目录所有群组id
*Output:	NA
*Return:	0:创建成功；1:创建失败
********************************************************************/
static int pimon_create_dir_recursive(char *path, uid_t user, gid_t group)
{
	char cur_path[PATH_MAXLEN+1] = {'\0'};
	int index = 0;
	int ret;

	if ('/' != *path) {
		pr_err("[%s]: path is invalid!path:%s.\n",
			__func__, path);
		return 1;
	}

	cur_path[index++] = *path++;
	while ('\0' != *path) {
		if ('/' == *path) {
			ret = pimon_create_dir(cur_path, user, group);
			if (ret) {
				pr_err("[%s]: pimon_create_dir failed!cur_path:%s.\n",
					__func__, cur_path);
				return 1;
			}
		}
		cur_path[index] = *path;
		path++;
		index++;
	}
	return 0;
}

/********************************************************************
*Function:	pimon_savedata2fs
*Description:	将数据保存至文件系统
*Input:		savepath:保存文件的路径
		filename: 保存文件名
		addr:数据起始地址
		size:数据大小
		is_append:标志位1:数据追加到文件尾 0:若文件存在删除原文件
*Output:	NA
*Return:	0:保存成功;1:保存失败
********************************************************************/
static int pimon_savedata2fs(char *savepath, char *filename, void *addr,
	u32 size, u32 is_append)
{
	int ret, flags;
	ssize_t length;
	struct file *fp;
	char path[PATH_MAXLEN + FILE_NAME_MAXLEN + 1] = {'\0'};

	if (NULL == savepath || NULL == filename || NULL == addr || size == 0) {
		pr_err("[%s]: invalid parameter!savepath:%p, filename:%p, addr:%p, size:0x%x.\n",
			__func__, savepath, filename, addr, size);
		return 1;
	}

	snprintf(path, (unsigned long)(PATH_MAXLEN + FILE_NAME_MAXLEN + 1),
		"%s/%s", savepath, filename);

	flags = O_CREAT | O_RDWR | (is_append ? O_APPEND : O_TRUNC);
	fp = filp_open(path, flags, FILE_LIMIT);
	if (IS_ERR(fp)) {
		pr_err("[%s]: open file %s err!\n",
			__func__, path);
		return 1;
	}

	vfs_llseek(fp, 0L, SEEK_END);
	length = vfs_write(fp, addr, (size_t)size, &(fp->f_pos));
	if (size != length) {
		pr_err("[%s]: write file %s exception!length:%lu.\n",
			__func__, path, length);
		goto write_fail;
	}
	vfs_fsync(fp, 0);
write_fail:
	filp_close(fp, NULL);
	/*根据权限要求，目录及子目录群组调整为root-system*/
	ret = (int)sys_chown((const char __user *)path, ROOT_UID, SYSTEM_GID);
	if (ret) {
		pr_err("[%s]: chown %s uid [%d] gid [%d] failed err [%d]!\n",
			__func__, path, ROOT_UID, SYSTEM_GID, ret);
	}

	return 0;
}

/********************************************************************
*Function:	pimon_mntn_get_cur_blk
*Description:	获取pimon寄存器LAST_WADDR_LO写blk的bit位
*Input:		NA
*Output:	NA
*Return:	LAST_WADDR_LO写blk的bit位
********************************************************************/
static u32 pimon_mntn_get_cur_blk(void)
{
	u32 reg_value = readl_relaxed((u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_LAST_WADDR_LO);

	reg_value = (reg_value & PIMON_REG_BIT_MASK_BLKS_SELECTED) >>
		PIMON_REG_BIT_BLKS_SELECTED;
	return reg_value;
}

/********************************************************************
*Function:	pimon_mntn_get_cur_blk
*Description:	将LAST_WADDR_LO写blk的bit位映射成blk索引
*Input:		reg_value:LAST_WADDR_LO写blk的bit位
*Output:	NA
*Return:	pimon当前写的bik id
********************************************************************/
static u32 pimon_mntn_blk_mark_to_id(u32 reg_value)
{
	u32	blk_index = 0;

	switch (reg_value) {
	case PIMON_REG_BLK0_SELECTED:
		blk_index = PIMON_BLOCK_0;
		break;
	case PIMON_REG_BLK1_SELECTED:
		blk_index = PIMON_BLOCK_1;
		break;
	case PIMON_REG_BLK2_SELECTED:
		blk_index = PIMON_BLOCK_2;
		break;
	case PIMON_REG_BLK3_SELECTED:
		blk_index = PIMON_BLOCK_3;
		break;
	default:
		break;
	}
	return blk_index;
}

/********************************************************************
*Function:	pimon_save_data_cont
*Description:	连续模式下保存数据
*Input:		filename:保存文件名
*Output:	NA
*Return:	0:保存成功;1:保存失败
********************************************************************/
static int pimon_save_data_cont(char *filename)
{
	int ret;
	u32 i;
	/*连续模式，将4个block数据保存*/
	for (i = 0; i < PIMON_BLOCK_NUM_MAX; i++) {
		ret = pimon_savedata2fs(PIMON_PATH, filename,
			blks_map_addr[i], blks_phys_size[i], 1);
		if (ret) {
			pr_err("[%s]: pimon_savedata2fs: save block%u data failed!\n",
				__func__, i);
			return 1;
		}
	}
	return 0;
}

/********************************************************************
*Function:	pimon_save_data_mul_cont
*Description:	片段缓存模式下保存数据
*Input:		filename:保存文件名
*Output:	NA
*Return:	0:保存成功;1:保存失败
********************************************************************/
static int pimon_save_data_mul_cont(char *filename)
{
	int ret;
	void __iomem *data_addr;
	u32 reg_value, frag_ram_addr, data_size;

	/*片段缓存模式，结合frag_ram_addr将内部ram数据保存*/
	mdelay(1000);
	reg_value = readl_relaxed((u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_FRAG_ADDR);
	if (0 == (reg_value & PIMON_FRAG_BUF_DONE_BITS)) {
		pr_err("[%s]:frag_buf is not done!\n", __func__);
		return 1;
	}

	frag_ram_addr = reg_value & PIMON_FRAG_RAM_ADDR_MASK;
	frag_ram_addr = (frag_ram_addr == PIMON_FRAG_RAM_ADDR_MASK) ?
		0 : (frag_ram_addr + 1);

	data_addr = g_pimon_mgr.reg_base_addr + PIMON_INSIDE_RAM_ADDR +
		frag_ram_addr * PIMON_INSIDE_RAM_STEP; /*lint !e679*/
	data_size = PIMON_INSIDE_RAM_SIZE -
		frag_ram_addr * PIMON_INSIDE_RAM_STEP;
	ret = pimon_savedata2fs(PIMON_PATH, filename,
			data_addr, data_size, 1);
	if (ret) {
		pr_err("[%s]: pimon_savedata2fs: save inside ram data failed!\n",
			__func__);
		return 1;
	}

	data_addr = g_pimon_mgr.reg_base_addr + PIMON_INSIDE_RAM_ADDR;
	data_size = frag_ram_addr * PIMON_INSIDE_RAM_STEP;
	ret = pimon_savedata2fs(PIMON_PATH, filename,
			data_addr, data_size, 1);
	if (ret) {
		pr_err("[%s]: pimon_savedata2fs: save inside ram data failed!\n",
			__func__);
		return 1;
	}

	return 0;
}

/********************************************************************
*Function:	pimon_save_data_trigger
*Description:	trigger模式下保存数据
*Input:		filename:保存文件名
*Output:	NA
*Return:	0:保存成功;1:保存失败
********************************************************************/
static int pimon_save_data_trigger(char *filename)
{
	int ret;
	u32 i, last_blk_id, break_blk_id;
	/*触发模式，将触发时所在的block和前后的block数据保存*/
	last_blk_id = pimon_mntn_blk_mark_to_id(g_pimon_mgr.cul_blk);
	if (g_pimon_mgr.full_blk_num > 2)
		i = (last_blk_id + 2) % PIMON_BLOCK_NUM_MAX;
	else
		i = PIMON_BLOCK_0;

	break_blk_id = (last_blk_id + 1) % PIMON_BLOCK_NUM_MAX;
	pr_err("[%s]: head_blk_id: %u; last_blk_id:%u;break_blk_id:%u!\n",
		__func__, i, last_blk_id, break_blk_id);
	while (i != break_blk_id) {
		ret = pimon_savedata2fs(PIMON_PATH, filename,
			blks_map_addr[i], blks_phys_size[i], 1);
		if (ret) {
			pr_err("[%s]: pimon_savedata2fs: save block%u data failed!\n",
				__func__, i);
			return 1;
		}
		i = (i + 1) % PIMON_BLOCK_NUM_MAX;
	}
	return 0;
}

/********************************************************************
*Function:	pimon_save_data
*Description:	将数据（不包括头信息和尾信息）保存至文件系统
*Input:		test_type:测试类型 filename:保存文件名
*Output:	NA
*Return:	0:保存成功;1:保存失败
********************************************************************/
static int pimon_save_data(enum e_pimon_test_type test_type, char *filename)
{
	int ret;

	if (test_type >= PIMON_TEST_RST_REQ) {
		pr_err("[%s]: test_type is invalid!\n",
			__func__);
		return 1;
	}
	switch (test_type) {
	case PIMON_TEST_CONT:
		ret = pimon_save_data_cont(filename);
		if (ret) {
			pr_err("[%s]: pimon_save_data_cont failed!\n",
				__func__);
			return 1;
		}
		break;
	case PIMON_TEST_MUL_CONT:
		ret = pimon_save_data_mul_cont(filename);
		if (ret) {
			pr_err("[%s]: pimon_save_data_mul_cont failed!\n",
				__func__);
			return 1;
		}
		break;
	case PIMON_TEST_TRIGGER:
		ret = pimon_save_data_trigger(filename);
		if (ret) {
			pr_err("[%s]: pimon_save_data_trigger failed!\n",
				__func__);
			return 1;
		}
		break;
	default:
		break;
	}

	return 0;
}

/********************************************************************
*Function:	pimon_save_block_data
*Description:	将头+数据+尾保存至文件系统
*Input:		test_type:测试类型
*Output:	NA
*Return:	0:保存成功;1:保存失败
********************************************************************/
int pimon_save_block_data(enum e_pimon_test_type test_type)
{
	char filename[FILE_NAME_MAXLEN + 1] = { '\0' };
	int ret;

	if (test_type >= PIMON_TEST_RST_REQ) {
		pr_err("[%s]: test_type is invalid!\n",
			__func__);
		return 1;
	}
	/*判断PIMON_PATH是否存在，如果不存在创建目录*/
	if (pimon_create_dir_recursive(PIMON_PATH, ROOT_UID, SYSTEM_GID)) {
		pr_err("[%s]: pimon_create_dir_recursive failed!\n",
			__func__);
		return 1;
	}

	snprintf(filename, (unsigned long)(FILE_NAME_MAXLEN + 1), "%s%u%s",
			PIMON_PERFIX, filename_num, PIMON_SUFFIX);
	filename_num = (filename_num + 1) % FILE_MAX_NUM;

	/*保存头信息*/
	ret = pimon_savedata2fs(PIMON_PATH, filename,
		(void *)&g_pimon_mgr.pimon_data_head,
		(u32)sizeof(struct s_head_info), 0);
	if (ret) {
		pr_err("[%s]: pimon_savedata2fs: save head info failed!\n",
			__func__);
		return 1;
	}

	/*保存数据*/
	ret = pimon_save_data(test_type, filename);
	if (ret) {
		pr_err("[%s]: pimon_savedata2fs: save block data failed!\n",
			__func__);
		return 1;
	}

	/*保存尾信息*/
	ret = pimon_savedata2fs(PIMON_PATH, filename,
		(void *)&g_pimon_mgr.pimon_data_tail,
		(u32)sizeof(struct s_tail_info), 1);
	if (ret) {
		pr_err("[%s]: pimon_savedata2fs: save tail info failed!\n",
			__func__);
		return 1;
	}

	/*辅助全局变量清0*/
	g_pimon_mgr.trigger_happened = 0;
	g_pimon_mgr.cul_blk = 0;
	g_pimon_mgr.full_blk_num = 0;
	return 0;
}

static void pimon_mntn_save_data_work_func(struct work_struct *work)
{
	int ret;
	u32 reg_value;

	pr_err("[%s]: pimon work start!\n",
		__func__);

	ret = pimon_save_block_data(g_pimon_mgr.pimon_test_type_cur);
	if (ret) {
		pr_err("[%s]: pimon_save_block_data failed!\n",
			__func__);
	}

	reg_value = readl_relaxed((u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_GLB_STAT);
	pr_err("[%s]: glb_stat:0x%x\n",
		__func__, reg_value);
	writel_relaxed(0x00000000, (u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_CTRL); /*lint !e792*/
	pr_err("[%s]: close pimon success!\n",
		__func__);

	if (g_pimon_mgr.pimon_test_type_cur == PIMON_TEST_MUL_CONT) {
		writel_relaxed(0x00000000,
			(u8 __iomem *)g_pimon_mgr.reg_base_addr +
			PIMON_REG_OFFSET_CTRL_2);
	}
	pr_err("[%s]: pimon work end!\n",
		__func__);
} /*lint !e715*/

static irqreturn_t pimon_mntn_irq_handler(int irq, void *data)
{
	/*judge the irq reason by reading reg*/
	u32 reg_value;

	reg_value = readl_relaxed((u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_INT_STAT);
	/*清中断*/
	writel_relaxed(PIMON_INT_BITS_MASK_CLR,
		(u8 __iomem *)g_pimon_mgr.reg_base_addr +
		PIMON_REG_OFFSET_INIT_MASK); /*lint !e792*/
	pr_err("[%s]: pimon irq handler!reg_value:0x%x\n",
		       __func__, reg_value);
	/*fifo_full_int,blks_err_int,axi_wrerr_int*/
	if ((reg_value & PIMON_INT_BITS_MASK_ERR) > 0) {
		/*关闭FIFO、PIMON*/
		pimon_mntn_close();
		pr_err("[%s]: pimon error!reg_value:0x%x\n",
			__func__, reg_value);
	}
	switch (g_pimon_mgr.pimon_test_type_cur) {
	case PIMON_TEST_CONT:
		/*连续模式只响应blk3_full*/
		if ((reg_value & PIMON_INT_BIT_MASK_BLK3_FULL) > 0) {
			/*关闭FIFO、PIMON*/
			pimon_mntn_close();
			queue_work(pimon_save_data_wq, &pimon_save_data_w);
			pr_err("[%s]: pimon cont test over!\n",
				__func__);
		}
		break;
	case PIMON_TEST_MUL_CONT:
		/*片段缓存模式*/
		if ((reg_value & PIMON_INT_BITS_MASK_TRIG) > 0) {
			/*关闭trigger_en位，只响应第一次trigger中断*/
			reg_value = readl_relaxed((u8 __iomem *)g_pimon_mgr.reg_base_addr +
				PIMON_REG_OFFSET_CTRL);
			reg_value = reg_value & (~PIMON_CLOSE_TRIGGER_MASK);
			writel_relaxed(reg_value, (u8 __iomem *)g_pimon_mgr.reg_base_addr +
				PIMON_REG_OFFSET_CTRL); /*lint !e792*/
			queue_work(pimon_save_data_wq, &pimon_save_data_w);
			pr_err("[%s]: pimon mul cont test over!\n",
				__func__);
		}
		break;
	case PIMON_TEST_TRIGGER:
		/*trigger模式响应uv、ov、blk_full*/
		if ((reg_value & PIMON_INT_BITS_MASK_BLK_FULL) > 0) {
			g_pimon_mgr.full_blk_num++;
			if (1 == g_pimon_mgr.trigger_happened) {
				if (g_pimon_mgr.cul_blk > 0) {
					/*关闭FIFO、PIMON*/
					pimon_mntn_close();
					queue_work(pimon_save_data_wq,
						&pimon_save_data_w);
					pr_err("[%s]: pimon trigger test over!\n",
						__func__);
					break;
				}
				g_pimon_mgr.cul_blk = pimon_mntn_get_cur_blk();
			}
		}
		if ((reg_value & PIMON_INT_BITS_MASK_TRIG) > 0) {
			/*关闭trigger_en位，只响应第一次trigger中断*/
			reg_value = readl_relaxed((u8 __iomem *)g_pimon_mgr.reg_base_addr +
				PIMON_REG_OFFSET_CTRL);
			reg_value = reg_value & (~PIMON_CLOSE_TRIGGER_MASK);
			writel_relaxed(reg_value, (u8 __iomem *)g_pimon_mgr.reg_base_addr +
				PIMON_REG_OFFSET_CTRL); /*lint !e792*/
			g_pimon_mgr.trigger_happened = 1;
		}
		break;
	default:
		break;
	}

	return IRQ_HANDLED;
} /*lint !e715*/

static int __init pimon_mntn_init(void)
{
	int ret;

	memset(&g_pimon_mgr, 0, sizeof(struct s_pimon_mgr_info));
	g_pimon_mgr.pimon_data_head.tag_start = PIMON_DATA_TAG_START;
	g_pimon_mgr.pimon_data_tail.tag_end = PIMON_DATA_TAG_END;
	filename_num = 0;

	pimon_save_data_wq = create_workqueue("pimon_save_data_wq");
	if (NULL == pimon_save_data_wq) {
		pr_err("[%s]: pimon_save_data_wq workqueue create failed!\n",
			__func__);
		return 1;
	}
	INIT_WORK(&pimon_save_data_w, pimon_mntn_save_data_work_func);

	/*register irq handler*/
	ret = pimon_mntn_get_irq_from_dts();
	if (0 != ret) {
		pr_err("[%s]: pimon_mntn_get_irq_from_dts failed!ret:%u\n",
			__func__, ret);
		return 1;
	}
	ret = request_irq(g_pimon_mgr.irq, pimon_mntn_irq_handler,
		0UL, "pimon irq handler", NULL);
	if (0 != ret) {
		pr_err("[%s]: request_irq failed! irq:%d, ret:%u.\n",
			__func__, g_pimon_mgr.irq, ret);
		return 1;
	}

	pr_err("[%s]: pimon_mntn_init success!\n",
		__func__);
	return 0;
}

static void __exit pimon_mntn_exit(void)
{
	u32 i;

	iounmap(g_pimon_mgr.reg_base_addr);
	for (i = 0; i < PIMON_BLOCK_NUM_MAX; i++) {
		if (pfn_valid(blks_phys_addr[i] >> PAGE_SHIFT))
			vunmap(blks_map_addr[i]);
		else
			iounmap(blks_map_addr[i]);
	}

	cancel_work_sync(&pimon_save_data_w);
	flush_workqueue(pimon_save_data_wq);
	destroy_workqueue(pimon_save_data_wq);

	free_irq(g_pimon_mgr.irq, NULL);
}
/*lint -e528 -esym(528,*)*/
module_init(pimon_mntn_init);
module_exit(pimon_mntn_exit);
/*lint -e528 +esym(528,*)*/
/*lint -e753 -esym(753,*)*/
MODULE_LICENSE("GPL");
/*lint -e753 +esym(753,*)*/
