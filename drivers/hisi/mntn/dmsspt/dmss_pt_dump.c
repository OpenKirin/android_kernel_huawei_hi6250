/************************************************************************

  版权所有 (C), 2001-2011, 华为技术有限公司

 *************************************************************************
 文 件 名		: dmss_pt_dump.c
 版 本 号		: 初稿
 作    者		: 蒋孝伟 j00207786
 生成日期	: 2016年10月18日
 最近修改	:
 功能描述	: 将pattern trace buffer中的数据不断保存到文件中;
				:
 修改历史	:
 1.日  期		: 2016年10月18日
    作  者		: 蒋孝伟 j00207786
    修改内容	: 创建文件

 *************************************************************************/

/*************************************************************************
  1 头文件包含
 *************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/rtc.h>
#include <linux/delay.h>
#include <linux/completion.h>

#ifdef CONFIG_OF_RESERVED_MEM
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_reserved_mem.h>
#endif

#include <linux/hisi/util.h>
#include <linux/hisi/rdr_pub.h>
#include <mntn_public_interface.h>
#include "dmss_pt_dump.h"
#include "rdr_inner.h"
#include "securec.h"


#define DATE_MAXLEN 14UL

static u8* g_trace_buffer_vaddr;
static u8* g_pt_rptr[DMSS_TRACE_MAX];
static int g_fs_init_flag;
static int g_is_tracing_dump_pt[DMSS_TRACE_MAX];
static int g_is_tracing_stop;
static u32 g_step;
static int g_is_overflow;
static struct pattern_trace_info g_pt_info;
static struct pattern_trace_stat g_pt_stat;
static struct semaphore* g_dump_pt_sem[DMSS_TRACE_MAX];
static struct file* pt_fp[DMSS_TRACE_MAX];
static char g_dmsspt_path[DMSS_TRACE_MAX][DMSSPT_PATH_MAXLEN];
static struct task_struct *dmsspt_main[DMSS_TRACE_MAX];

#define PT_SEM_INIT(pt_num) \
static DEFINE_SEMAPHORE(dump_trace##pt_num##_sem);\
	sema_init(&dump_trace##pt_num##_sem, 0);\
	g_dump_pt_sem[pt_num] = &dump_trace##pt_num##_sem

static DECLARE_COMPLETION(dmsspt_date);

struct pt_dumping_stat {
	u64 timestamp;
	int state; /*stop or tracing*/
	int pt_state[DMSS_TRACE_MAX];
	int is_overflow;
};
static struct pt_dumping_stat dstat;

static struct dmsspt_region pt_region;

#ifdef CONFIG_OF_RESERVED_MEM
/*动态预留内存相关模板代码*/
static void save_dynamic_alloc_area(phys_addr_t base, unsigned long len)
{
	struct dmsspt_region *reg = &pt_region;
	reg->base = base;
	reg->size = len;
}

static int dmsspt_reserve_area(struct reserved_mem *rmem)
{
	char *status;

	status = (char *)of_get_flat_dt_prop(rmem->fdt_node, "status", NULL);
	if (status && (strncmp(status, "ok", strlen("ok")) != 0))
		return 0;

	save_dynamic_alloc_area(rmem->base, rmem->size);
	return 0;
}
/*lint -e528 -esym({528}, RESERVEDMEM_OF_DECLARE)*/
RESERVEDMEM_OF_DECLARE(dmsspt, "dmsspt_trace_buffer", dmsspt_reserve_area); /*lint !e611*/
/*lint -e528 +esym({528}, RESERVEDMEM_OF_DECLARE)*/
#endif

/*************************************************************************
Function:		print_pt_buffer_info
Description:	打印trace buffer的信息；
Input:		NA
Return:		NA；
Author:		j00207786
*************************************************************************/
void print_pt_buffer_info(void)
{
	pr_err("%s: phy base addr[0x%llx] !\n", __func__, pt_region.base);
	pr_err("%s: virt base addr[0x%llx] !\n", __func__, (u64)g_trace_buffer_vaddr);
	pr_err("%s: buffer size[0x%llx] !\n", __func__, pt_region.size);
	pr_err("%s: g_pt_rptr[0][0x%llx] !\n", __func__, (u64)g_pt_rptr[0]);
	pr_err("%s: g_pt_rptr[1][0x%llx] !\n", __func__, (u64)g_pt_rptr[1]);
	pr_err("%s: g_pt_info.pt_wptr[0][0x%llx] !\n", __func__, g_pt_info.pt_wptr[0]);
	pr_err("%s: g_pt_info.pt_wptr[1][0x%llx] !\n", __func__, g_pt_info.pt_wptr[1]);
}

/*************************************************************************
Function:    		is_trace_overflow
Description: 		保存dmss pattern trace到文件的接口函数；
Constraint：		写指针移动的上报是按一定pattern数量的，
				所以不存在发生一周的卷绕后，才上报，
				并判断overflow，否则此接口返回结果失效；
Input:		new_writep：新上报的写指针；cur_writep：本次保存对应的写指针位置；
readp：		读指针位置；
Output:		NA
Return:		0：没有overflow； 1：发生overflow；
Author:			j00207786
*************************************************************************/
int is_trace_overflow(u64 new_writep, u64 cur_writep, u64 readp)
{
	if ((readp < cur_writep) && ((readp < new_writep) && (new_writep < cur_writep)) )
		goto oflw;

	if ((readp > cur_writep) && ((new_writep < cur_writep) || (new_writep > readp)) )
		goto oflw;

	return 0;
oflw:
	pr_err("%s: new_writep [0x%llx], cur_writep [0x%llx], readp [0x%llx]!\n",
		__func__, new_writep, cur_writep, readp);
	return 1;
}

/*************************************************************************
Function:		dmsspt_get_timestamp
Description:	获取年月日的时间戳；
Input:		NA
Return:		年月日小时分秒的字符串；
Author:		j00207786
*************************************************************************/
static char* dmsspt_get_timestamp(void)
{
	struct rtc_time tm;
	struct timeval tv;
	static char databuf[DATE_MAXLEN + 1];

	memset_s(databuf, sizeof(databuf), 0, DATE_MAXLEN + 1);

	memset_s(&tv, sizeof(struct timeval), 0, sizeof(struct timeval));
	memset_s(&tm, sizeof(struct rtc_time), 0, sizeof(struct rtc_time));
	do_gettimeofday(&tv);
	tv.tv_sec -= (__kernel_time_t)sys_tz.tz_minuteswest * 60;
	rtc_time_to_tm((unsigned long)tv.tv_sec, &tm);

	snprintf_s(databuf, DATE_MAXLEN + 1, DATE_MAXLEN, "%04d%02d%02d%02d%02d%02d",
		 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		 tm.tm_hour, tm.tm_min, tm.tm_sec);

	pr_err("%s: [%s] !\n", __func__, databuf);
	return databuf;
}

/*************************************************************************
Function:		v2p
Description:    trace buffer虚拟地址转物理地址；
Input:		vaddr: 虚拟地址；
Return:		非0:物理地址；0: 非法值；
Author:		j00207786
*************************************************************************/
static u64 v2p(u64 vaddr)
{
	if (!vaddr || !(g_trace_buffer_vaddr) || !(pt_region.base))
		return 0;

	if ((vaddr < (u64)g_trace_buffer_vaddr) || (vaddr > ((u64)g_trace_buffer_vaddr + pt_region.size)))
		return 0;
	return ((vaddr - (u64)g_trace_buffer_vaddr) + pt_region.base);
}

/*************************************************************************
Function:		p2v
Description:    trace buffer物理地址转虚拟地址；
Input:		paddr: 物理地址；
Return:		非0: 虚拟地址；0: 非法值；
Author:		j00207786
*************************************************************************/
static u64 p2v(u64 paddr)
{
	if (!(paddr) || !(g_trace_buffer_vaddr) || !(pt_region.base))
		return 0;

	if ((paddr < pt_region.base) || (paddr > (pt_region.base + pt_region.size)))
		return 0;
	return ((paddr -  pt_region.base) + (u64)g_trace_buffer_vaddr);
}

/*************************************************************************
Function:		dmsspt_is_finished
Description:    返回当前tracing完成状态的接口函数；
Input:		NA；
Return:		0: 未完成；1: tracing已完成；
Author:		j00207786
*************************************************************************/
int dmsspt_is_finished(void)
{
	pr_info("%s: timestamp [%llx], state [%d].\n", __func__, dstat.timestamp, dstat.state);
	pr_info("%s: dstat.pt_state0 [%d] dstat.pt_state1 [%d].\n", __func__, dstat.pt_state[0], dstat.pt_state[1]);
	if (!dstat.timestamp)
		return 1;
	return dstat.state;
}

/*************************************************************************
Function:		init_params
Description:    初始化trace记录参数；
Input:		pinfo：pt相关交织及buffer相关信息；pstat：pt状态信息；
Return:		0: 成功；-1: 失败；
Author:		j00207786
*************************************************************************/
static int init_params(struct pattern_trace_info* pinfo, struct pattern_trace_stat* pstat)
{
	u32 i;

	g_pt_info.intlv = pinfo->intlv;
	for (i = 0; i < DMSS_TRACE_MAX; i++) {
		if (pinfo->pt_rptr[i])
			g_pt_rptr[i] = (u8*)p2v(pinfo->pt_rptr[i]);
		else
			g_pt_rptr[i] = (u8*)((u64)g_trace_buffer_vaddr + ((u64)i * (u64)g_pt_info.intlv));

		if (!g_pt_rptr[i]) {
			pr_err("%s, g_pt_rptr[%u] is null!\n", __func__, i);
			return -1;
		}

		g_pt_info.pt_wptr[i] = p2v(pinfo->pt_wptr[i]);
		if (!g_pt_info.pt_wptr[i]) {
			pr_err("%s, g_pt_info.pt_wptr[%u] is null!\n", __func__, i);
			return -1;
		}
	}

	g_pt_info.intlv_mode = pinfo->intlv_mode;

	/*根据交织模式决定读取步长*/
	if (TRACE_MODE0 == g_pt_info.intlv_mode)
		g_step = 2 * g_pt_info.intlv;
	else if (TRACE_MODE1 == g_pt_info.intlv_mode)
		g_step = g_pt_info.intlv;
	else {
		pr_err("%s:g_pt_info.intlv_mode [%u] is invalid.\n",
			__func__, g_pt_info.intlv_mode);
		g_pt_info.intlv_mode = DMSSPT_NOINIT_MAGIC;
		return -1;
	}
	dstat.timestamp = pstat->trace_begin_time;
	dstat.state = 0;
	for (i = 0; i < DMSS_TRACE_MAX; i++) {
		dstat.pt_state[i] = 0;
	}
	pr_err("%s: dstat.pt_state0 [%d] dstat.pt_state1 [%d].\n",
				__func__, dstat.pt_state[0], dstat.pt_state[1]);
	dstat.is_overflow = 0;
	pr_err("%s:  initial pt params, begin time [%llu]!\n", __func__, pstat->trace_begin_time);
	return 0;
}

/*************************************************************************
Function:		check_wptr_range
Description:    判断写指针的范围合法性；
Input:		pinfo：pt相关交织及buffer相关信息；trace_num: trace模块编号；
Return:		0: 合法；-1: 非法；
Author:		j00207786
*************************************************************************/
static int check_wptr_range(struct pattern_trace_info* pinfo, int trace_num)
{
	if (pinfo->pt_wptr[trace_num] < pt_region.base ||
		pinfo->pt_wptr[trace_num] > (pt_region.base + pt_region.size)) {
		pr_err("%s: pinfo->pt_wptr[%d] 0x%llx is invalid!\n",
			__func__, trace_num, pinfo->pt_wptr[trace_num]);
		print_pt_buffer_info();
		return -1;
	}
	return 0;
}

/*************************************************************************
Function:		dmsspt_fs_init
Description:    初始化日志目录；
Input:		NA；
Return:		0: 成功； 非0: 失败；
Author:		j00207786
*************************************************************************/
static int dmsspt_fs_init(void)
{
	int ret;
	if (!g_fs_init_flag) {
		while (rdr_wait_partition("/data/lost+found", 1000) != 0)
				;
		ret = rdr_create_dir(DMSSPT_ROOT);
		if (ret) {
			pr_err("%s, create dir [%s] failed!\n", __func__, DMSSPT_ROOT);
			return ret;
		}
		g_fs_init_flag = DMSSPT_FS_INIT_MAGIC;
	}
	return 0;
}

/*************************************************************************
Function:		dmsspt_save_trace
Description:    保存dmss pattern trace到文件的接口函数；
Input:		pinfo：pt相关交织及buffer相关信息；pstat：pt状态信息；
            is_tracing_stop：trace是否停止；
Return:		0: 成功；-1: 失败；1: 上次trace保存还未结束
Author:		j00207786
*************************************************************************/
int dmsspt_save_trace(struct pattern_trace_info* pinfo, struct pattern_trace_stat* pstat, int is_tracing_stop)
{
	int i;
	int ret = 0;

	if (!g_fs_init_flag) {
		pr_err("%s, data file system hasn't been ready!\n", __func__);
		return -1;
	}

	if (IS_ERR_OR_NULL(pinfo) || IS_ERR_OR_NULL(pstat)) {
		pr_err("%s:pinfo or pstat is NULL.\n", __func__);
		return -1;
	}

	if (dstat.timestamp == pstat->trace_begin_time
		&& dstat.is_overflow) {
		pr_err("%s:  DMSS pattern trace is overflow!\n", __func__);
		return -1;
	}

	/*trace开始，首次初始化*/
	if (DMSSPT_NOINIT_MAGIC == g_pt_info.intlv_mode) {
		ret = init_params(pinfo, pstat);
		if (ret) {
			pr_err("%s:  init_params failed!\n", __func__);
			return ret;
		}
	}

	if (!dstat.state && (dstat.timestamp != pstat->trace_begin_time)) {
		pr_err("%s: last tracing [timestamp:%llu] hasn't been finished!\n",
			__func__, dstat.timestamp);
		return 1;
	}

	g_pt_stat = *pstat;

	for (i = 0; i < DMSS_TRACE_MAX; i++) {
		if (check_wptr_range(pinfo, i))
			return -1;

		if (is_trace_overflow(p2v(pinfo->pt_wptr[i]), g_pt_info.pt_wptr[i], (u64)g_pt_rptr[i])) {
			pr_err("%s:  DMSS pattern trace%d is overflow!\n", __func__, i);
			pr_err("%s:  new_writep[0x%llx], cur_writep[0x%llx], readp[0x%llx]",
				__func__, p2v(pinfo->pt_wptr[i]), g_pt_info.pt_wptr[i],
				(u64)g_pt_rptr[i]);
			g_is_overflow = 1;
			dstat.is_overflow = 1;
			ret = -1;
			break;
		}

		/*转换为虚拟地址*/
		g_pt_info.pt_wptr[i] = (u64)g_trace_buffer_vaddr + (pinfo->pt_wptr[i] - pt_region.base);
		if (!g_is_tracing_dump_pt[i])
			up(g_dump_pt_sem[i]);
	}

	g_is_tracing_stop = is_tracing_stop;
	return ret;
}

/*************************************************************************
Function:		waiting_for_tracing_finished
Description:    等待trace记录完成；
Input:		is_sync: 本接口是否同步等待到文件保存结束；
Return:		NA
Author:		j00207786
*************************************************************************/
static void waiting_for_tracing_finished(int is_sync)
{
	/*wait for tracing finished*/
	while (!dmsspt_is_finished() && is_sync)
		msleep(1000);
}

/*************************************************************************
Function:    	test_dmsspt_dump_trace
Description: 	测试trace保存函数，包括大数据量，buffer溢出，
			数据有效性及数据完整性；
Input:		buffer_size: trace buffer的大小，单位M；
			data_size: 决定数据量大小，单位M；
			write_velocity; 每秒写入的数据量，单位M；
			intlv: 交织粒度；intlv_mode: 交织模式；
			is_sync: 本接口是否同步等待到文件保存结束；
Return:		0：成功； -1：失败；
Author:		j00207786
*************************************************************************/
int test_dmsspt_dump_trace(u64 buffer_size, u64 data_size, u64 write_velocity, u32 intlv, u32 intlv_mode, int is_sync)
{
	int ret = 0;
	struct pattern_trace_stat pt_stat;
	struct pattern_trace_info  pt_info;
	u8 *pt0,*pt1;
	u8 poison = 0;
	int is_tracing_stop = 0;
	u8* pt_buffer_end;
	u64 buffer_old_size;
	u64 temp;

	if (!pt_region.base) {
		pr_err("%s, invalid params\n", __func__);
		return -1;
	}

	if (buffer_size > pt_region.size)
		buffer_size = pt_region.size;

	memset_s(&pt_stat, sizeof(struct pattern_trace_stat), 0, sizeof(struct pattern_trace_stat));
	memset_s(&pt_info, sizeof(struct pattern_trace_stat), 0, sizeof(struct pattern_trace_info));
	buffer_old_size = pt_region.size;
	pt_region.size = buffer_size * 1024 * 1024;
	pt_buffer_end = (u8*)pt_region.base + pt_region.size;
	memset_s(g_trace_buffer_vaddr, buffer_old_size, 0xff, buffer_old_size);
	memset_s(g_trace_buffer_vaddr, pt_region.size, 0x0, pt_region.size);

	pt_stat.trace_begin_time = sched_clock();
	pr_err("%s, trace_begin_time [%llu]\n", __func__, pt_stat.trace_begin_time);
	pr_err("%s, pt_buffer_end [0x%llx]\n", __func__, (u64)pt_buffer_end);

	pt_info.intlv = intlv;
	pt_info.intlv_mode = intlv_mode;
	pt_info.pt_wptr[0] = (u64)pt_region.base;
	pt_info.pt_wptr[1] = (u64)pt_region.base + pt_info.intlv;
	pt_info.pt_rptr[0] = 0;
	pt_info.pt_rptr[1] = 0;

	pt0 = (u8*)p2v(pt_info.pt_wptr[0]);
	pt1 = (u8*)p2v(pt_info.pt_wptr[1]);
	while (1) {
		pr_err("%s, buffer_base [0x%llx], wptr0 [0x%llx] wptr1 [0x%llx] \n",
			__func__, pt_region.base, pt_info.pt_wptr[0], pt_info.pt_wptr[1]);

		/*设置停止门限*/
		if (!data_size) {
			is_tracing_stop = 1;
			pt_stat.trace_end_time = sched_clock();
			pr_err("%s, trace_end_time [%llu]\n", __func__, pt_stat.trace_end_time);
		}

		/*trace0 buffer卷绕*/
		if (pt_info.pt_wptr[0] < (u64)v2p((u64)pt0)) {
			pt0 = g_trace_buffer_vaddr;
		}

		/*写入posion值*/
		while(pt_info.pt_wptr[0] > (u64)v2p((u64)pt0)) {
			memset_s(pt0, (u64)pt_info.intlv, poison, (u64)pt_info.intlv);
			memset_s(pt0 + pt_region.size/2, (u64)pt_info.intlv, poison, (u64)pt_info.intlv);
			temp = (u64)pt0;
			temp += ((u64)pt_info.intlv + (u64)pt_info.intlv);
			pt0 = (u8*)temp;
		}
		poison++;

		/*trace1 buffer卷绕*/
		if (pt_info.pt_wptr[1] < (u64)v2p((u64)pt1)) {
			pt1 = g_trace_buffer_vaddr + pt_info.intlv;
		}

		/*写入posion值*/
		while(pt_info.pt_wptr[1] > (u64)v2p((u64)pt1)) {
			memset_s(pt1, (u64)pt_info.intlv, poison, (u64)pt_info.intlv);
			memset_s(pt1 + pt_region.size/2, (u64)pt_info.intlv, poison, (u64)pt_info.intlv);
			temp = (u64)pt1;
			temp += (2UL * (u64)pt_info.intlv);
			pt1 = (u8*)temp;
		}
		poison++;

		pr_err("%s, dmsspt_save_trace\n", __func__);
		ret = dmsspt_save_trace(&pt_info, &pt_stat, is_tracing_stop);
		if (ret) {
			pr_err("%s():dmsspt_save_trace failed.\n", __func__);
			break;
		}

		if (is_tracing_stop) {
			pt_info.pt_wptr[0] = pt_region.base;
			pt_info.pt_wptr[1] = pt_region.base + pt_info.intlv;
			break;
		}
		msleep(1000);

		if (data_size > write_velocity) {
			pt_info.pt_wptr[0] += write_velocity * 1024 * 1024;
			pt_info.pt_wptr[1] += write_velocity * 1024 * 1024;
			data_size -= write_velocity;
		} else {
			pt_info.pt_wptr[0] += data_size * 1024 * 1024;
			pt_info.pt_wptr[1] += data_size * 1024 * 1024;
			data_size = 0;
		}

		pr_err("%s, pt_buffer_end [0x%llx], wptr0 [0x%llx] wptr1 [0x%llx] \n",
			__func__, (u64)pt_buffer_end, pt_info.pt_wptr[0], pt_info.pt_wptr[1]);
		/*处理写指针卷绕*/
		if (pt_info.pt_wptr[0] >= (u64)pt_buffer_end) {
			pt_info.pt_wptr[0] = (pt_info.pt_wptr[0] - (u64)pt_buffer_end) + (u64)pt_region.base;
		}
		if (pt_info.pt_wptr[1] >= (u64)pt_buffer_end) {
			pt_info.pt_wptr[1] = (pt_info.pt_wptr[1] - (u64)pt_buffer_end) + (u64)pt_region.base;
		}
		g_pt_info.pt_wptr[0] = p2v(pt_info.pt_wptr[0]);
		g_pt_info.pt_wptr[1] = p2v(pt_info.pt_wptr[1]);
	}

	waiting_for_tracing_finished(is_sync);
	pt_region.size = buffer_old_size;
	return ret;
}

/*************************************************************************
Function:    	dmsspt_save_trace_reentry_test
Description: 	测试trace保存不可重入；
Input:		NA
Return:		0：成功； -1：失败；
Author:		j00207786
*************************************************************************/
int dmsspt_save_trace_reentry_test(void)
{
	test_dmsspt_dump_trace(10UL, 20UL, 5UL, 256, 1, 0);/*u64 buffer_size, u64 data_size, u64 write_velocity, u32 intlv, u32 intlv_mode, int is_sync*/
	return test_dmsspt_dump_trace(10UL, 20UL, 5UL, 256, 1, 0);
}

/*************************************************************************
Function:    	dmsspt_update_pt_header
Description: 	更新dmss pattern trace的头信息到文件中；
Input:		trace_num: 对应的硬件trace模块编号；stat: 待更新的头信息；
Return:		0：成功； -1：失败；
Author:		j00207786
*************************************************************************/
static int dmsspt_update_pt_header(int trace_num, struct pattern_trace_stat *stat)
{
	struct pattern_trace_header pt_header;
	struct file* trace_fp;
	ssize_t ret;

	pr_err("%s:begin.\n", __func__);
	trace_fp = filp_open(g_dmsspt_path[trace_num], O_RDWR, (umode_t)DMSSPT_FILE_LIMIT);
	if (IS_ERR_OR_NULL(trace_fp)) {
		pr_err("%s, open file [%s] failed!\n", __func__, g_dmsspt_path[trace_num]);
		return -1;
	}

	vfs_llseek(trace_fp, 0L, SEEK_SET);
	pr_err("%s: finish pt_fp[%d] fpos [0x%llx].\n", __func__, trace_num, trace_fp->f_pos);
	pt_header.trace_begin_time = stat->trace_begin_time;
	pt_header.pad0 = 0;
	pt_header.trace_end_time = stat->trace_end_time;
	pt_header.pad1 = 0;
	pt_header.trace_cur_address = stat->trace_cur_address[trace_num];
	pt_header.trace_pattern_cnt = stat->trace_pattern_cnt[trace_num];
	pt_header.trace_roll_cnt = stat->trace_roll_cnt[trace_num];
	pt_header.trace_unaligned_mode = stat->trace_unaligned_mode;
	ret = vfs_write(trace_fp, (char*)&pt_header, sizeof(struct pattern_trace_header), &(trace_fp->f_pos));
	if (sizeof(struct pattern_trace_header) != (u64)ret) {
		pr_err("%s:write file %s exception with ret %ld.\n",
		       __func__,  trace_fp->f_path.dentry->d_iname, ret);
		filp_close(trace_fp, NULL);
		return -1;
	}
	vfs_fsync(trace_fp, 0);
	filp_close(trace_fp, NULL);
	pr_err("%s:end.\n", __func__);
	return 0;
}

/*************************************************************************
Function:		dmsspt_handle_overlap
Description:    读指针发生卷绕后的处理；
Input:		trace_num: trace模块编号；
Return:		NA
Author:		j00207786
*************************************************************************/
static void dmsspt_handle_overlap(int trace_num)
{
	u64 delta;
	u64 remainder;

	delta = (u64)g_trace_buffer_vaddr + pt_region.size - (u64)g_pt_rptr[trace_num];
	remainder = delta % g_step;

	/*读指针到buffer末尾是否不足DMSS_TRACE_MAX * g_step 或 g_step, 读指针为g_step的整数倍*/
	if ( !remainder && !delta) {
		g_pt_rptr[trace_num] = g_trace_buffer_vaddr + g_step;
		pr_err("%s:trace1 overlap has occured, trace_num [%d].\n", __func__, trace_num);
	} else if (!remainder && (delta == g_step)) {
		g_pt_rptr[trace_num] = g_trace_buffer_vaddr;
		pr_err("%s:trace0 overlap has occured, trace_num [%d].\n", __func__, trace_num);
	} else {
		if (0 == (g_pt_rptr[trace_num] - g_trace_buffer_vaddr) % g_step)
			g_pt_rptr[trace_num] += g_step;
	}
}

/*************************************************************************
Function:    		dmsspt_save_file
Description: 	保存dmss pattern trace到文件实体函数；
Input:		trace_num: 对应的硬件trace模块编号；
Output:		NA
Return:		0：成功； -1：失败；
Author:		j00207786
*************************************************************************/
static int dmsspt_save_file(struct file* trace_fp, int trace_num)
{
	int ret = 0;

	if (IS_ERR_OR_NULL(trace_fp)) {
		pr_err("%s:trace_fp is null.\n", __func__);
		return -1;
	}

	if (!g_step) {
		pr_err("%s:g_step is invalid zero.\n", __func__);
		return -1;
	}

	pr_err("%s: g_pt_rptr [0x%llx] g_pt_info.pt_wptr [0x%llx]\n",
		__func__, (u64)g_pt_rptr[trace_num], g_pt_info.pt_wptr[trace_num]);
	while (g_pt_rptr[trace_num] != (u8*)g_pt_info.pt_wptr[trace_num]) {
		if (g_is_overflow)
			goto finish;

		/*文件到达DMSSPT_FILE_MAX_LEN，新建文件继续存储，
		由于文件系统有文件大小限制*/
		if (trace_fp->f_pos >= (loff_t)DMSSPT_FILE_MAX_LEN) {
			pr_err("%s:dmsspt file reached the size of 0x%lx.\n", __func__, DMSSPT_FILE_MAX_LEN);
			return 0;
		}

		if (g_pt_rptr[trace_num] > (g_trace_buffer_vaddr + pt_region.size)) {
			pr_err("%s: rptr[%llx] overstep.  g_trace_buffer_vaddr [%llx]\n", __func__, (u64)g_pt_rptr[trace_num], (u64)g_trace_buffer_vaddr);
			ret  = -1;
			goto fail;
		}

		ret = (int)vfs_write(trace_fp, (char*)g_pt_rptr[trace_num], (u64)DMSSPT_UNIT_SIZE, &(trace_fp->f_pos));
		if (ret != DMSSPT_UNIT_SIZE) {
			pr_err("%s:write file %s exception with ret %d fpos [0x%llx]!!!\n",
			       __func__, trace_fp->f_path.dentry->d_iname, ret, trace_fp->f_pos);
			goto fail;
		}

		g_pt_rptr[trace_num] += DMSSPT_UNIT_SIZE;
		dmsspt_handle_overlap(trace_num);
	}

	if (g_is_tracing_stop)
		goto finish;
	else
		return 0;

finish:
	/*更新文件头信息*/
	ret = dmsspt_update_pt_header(trace_num, &g_pt_stat);
	pr_err("%s: tracing is finished!\n", __func__);
fail:
	pr_err("%s:ret [%d]!\n", __func__, ret);
	return ret;
}

/*************************************************************************
Function:    		dmsspt_open
Description: 	dmss pt trace文件打开函数；
Input:		trace_num: 对应的硬件trace模块编号；index: 打开文件的序号；
Return:		文件句柄；NULL: 为打开失败；
Author:			j00207786
*************************************************************************/
static struct file* dmsspt_open(int trace_num, int index)
{
	int flags;
	char path[DMSSPT_PATH_MAXLEN] = {'\0'};

	pr_err("%s: enter!\n", __func__);
	if (!index) {
		memset_s(g_dmsspt_path[trace_num], DMSSPT_PATH_MAXLEN - 1, 0, DMSSPT_PATH_MAXLEN - 1);
		g_dmsspt_path[trace_num][DMSSPT_PATH_MAXLEN - 1] = '\0';

		wait_for_completion(&dmsspt_date);
		snprintf_s(g_dmsspt_path[trace_num], DMSSPT_PATH_MAXLEN, DMSSPT_PATH_MAXLEN-1, "%s%d_%s", DMSSPT_ROOT"dmsspt", trace_num, dmsspt_get_timestamp());
		complete(&dmsspt_date);

		strncpy_s(path, DMSSPT_PATH_MAXLEN-1, g_dmsspt_path[trace_num], DMSSPT_PATH_MAXLEN-1);
	} else
		snprintf_s(path, DMSSPT_PATH_MAXLEN, DMSSPT_PATH_MAXLEN - 1, "%s.%d", g_dmsspt_path[trace_num], index);

	pr_err("%s:open file [%s]!\n", __func__, g_dmsspt_path[trace_num]);
	flags = O_CREAT | O_RDWR;
	return filp_open(path, flags, (umode_t)DMSSPT_FILE_LIMIT);
}

/*************************************************************************
Function:		dmsspt_thread_data_check_init
Description:    初始化线程参数及日志目录；
Input:		线程入参；
Return:		0: 成功； 非0: 失败；
Author:		j00207786
*************************************************************************/
static int dmsspt_thread_data_check_init(void *data)
{
	int trace_num;
	int ret;

	if ((u64)data > 0) {
		trace_num = (int)((u64)data-(u64)DMSSPT_NUM_OFFSET);
		if (trace_num >= DMSS_TRACE_MAX || trace_num < 0) {
			pr_err("%s, trace_num [%d] is invalid!\n", __func__, trace_num);
			return -1;
		}
	} else {
		pr_err("%s: para data must not be null.\n", __func__);
		return -1;
	}

	ret = dmsspt_fs_init();
	if (ret)
		return ret;

	return trace_num;
}

/*************************************************************************
Function:		dmsspt_finish
Description:    trace停止时的相关处理操作；
Input:		trace_num: trace模块编号；
Return:		NA；
Author:		j00207786
*************************************************************************/
static void dmsspt_finish(long trace_num)
{
	int finish = 1;/*trace停止后，文件是否保存完成标志*/
	int i;

	if (!IS_ERR_OR_NULL(pt_fp[trace_num])) {
		pr_err("%s: close pt fp.\n", __func__);
		vfs_fsync(pt_fp[trace_num], 0);
		filp_close(pt_fp[trace_num], NULL);

		/*相关数据清零处理；*/
		pt_fp[trace_num] = NULL;
		dstat.pt_state[trace_num] = 1;
	}

	/*等待2线程都保存完成*/
	for (i = 0; i < DMSS_TRACE_MAX; i++) {
		pr_err("%s: before while,trace_num [%d], pt_fp0 [%llx] pt_fp1 [%llx].\n", __func__, i, (u64)pt_fp[0], (u64)pt_fp[1]);
		pr_err("%s: trace_num [%d], dstat.pt_state0 [%d] dstat.pt_state1 [%d].\n",
				__func__, i, dstat.pt_state[0], dstat.pt_state[1]);
		if (1 != dstat.pt_state[i]) {
			pr_err("%s: wait for other threads finished,trace_num [%d], dstat.pt_state0 [%d] dstat.pt_state1 [%d].\n",
				__func__, i, dstat.pt_state[0], dstat.pt_state[1]);
			finish = 0;
			break;
		}
	}

	pr_err("%s: finish [%d].\n", __func__, finish);
	/*只有最后一个线程能走到这个流程，所以此分支内，
	不能有线程私有的数据处理，必须为全局性数据处理；*/
	if (finish) {
		g_is_tracing_stop = 0;
		g_is_overflow = 0;
		dstat.state = 1;
		g_pt_info.intlv = 0;
		g_pt_info.intlv_mode = DMSSPT_NOINIT_MAGIC;
		memset_s(&g_pt_stat, sizeof(struct pattern_trace_stat), 0, sizeof(struct pattern_trace_stat));
	}
}

/*************************************************************************
Function:    	dmsspt_file_init
Description: 	trace文件初始化函数；
Input:		trace_num：trace模块编号；index: trace文件编号；
Return:		非NULL：成功； NULL：失败；
Author:			j00207786
*************************************************************************/
static struct file* dmsspt_file_init(long trace_num, int* index)
{
	if (IS_ERR_OR_NULL(pt_fp[trace_num])) {
		pt_fp[trace_num] = dmsspt_open((int)trace_num, *index);
		if (IS_ERR_OR_NULL(pt_fp[trace_num])) {
			pr_err("%s:create file %s err.\n", __func__, g_dmsspt_path[trace_num]);
			return NULL;
		}

		(*index)++;
		if ((*index) > 1)
			pr_err("%s:index %d is too big.\n", __func__, *index);

		/*跳过文件头信息，这些信息是在trace结束时填充*/
		vfs_llseek(pt_fp[trace_num], sizeof(struct pattern_trace_header), SEEK_SET);
	}
	return pt_fp[trace_num];
}

/*************************************************************************
Function:    		dmss_pattern_trace_dump_thread
Description: 	保存trace的线程函数；
Input:		data：线程入参；
Return:		0：成功； 非0：失败；
Author:			j00207786
*************************************************************************/
static int dmss_pattern_trace_dump_thread (void *data)
{
	unsigned long jiffies_priv;
	int ret;
	long trace_num;
	int index = 0;/*trace文件序号*/

	trace_num = (long)dmsspt_thread_data_check_init(data);
	if (trace_num < 0)
		return -1;

	while (!kthread_should_stop()) {
		jiffies_priv = msecs_to_jiffies(DMSSPT_DUMP_TIMEOUT * (u32)MSEC_PER_SEC);
		if (down_timeout(g_dump_pt_sem[trace_num], (long)jiffies_priv)) {
			if (!g_is_tracing_stop || !pt_fp[trace_num])
				continue;
			pr_err("%s: dmss_pattern_trace_dump_thread %ld, g_is_tracing_stop [%d].\n",
				__func__, trace_num, g_is_tracing_stop);
		}

		if (DMSSPT_NOINIT_MAGIC == g_pt_info.intlv_mode)
			continue;

		g_is_tracing_dump_pt[trace_num] = 1;

		if (g_is_overflow) {
			goto save_fail;
		}

		if (!dmsspt_file_init(trace_num, &index))
			continue;

again:
		ret = dmsspt_save_file(pt_fp[trace_num], (int)trace_num);
		if (ret) {
			pr_err("%s:dmsspt_save_file err.\n", __func__);
			goto save_fail;
		}

		/*文件到达DMSSPT_FILE_MAX_LEN，新建文件继续存储，
		由于文件系统有文件大小限制*/
		if (pt_fp[trace_num]->f_pos >= (loff_t)DMSSPT_FILE_MAX_LEN &&
			g_pt_rptr[trace_num] != (u8*)g_pt_info.pt_wptr[trace_num]) {
			vfs_fsync(pt_fp[trace_num], 0);
			filp_close(pt_fp[trace_num], NULL);
			pt_fp[trace_num] = dmsspt_open((int)trace_num, index);
			if (IS_ERR_OR_NULL(pt_fp[trace_num])) {
				pr_err("%s:create file %s err.\n", __func__, g_dmsspt_path[trace_num]);
				continue;
			}
			index++;
			goto again;
		}

		if (g_is_tracing_stop || g_is_overflow) {
save_fail:
			index = 0;
			dmsspt_finish(trace_num);
		}
		g_is_tracing_dump_pt[trace_num] = 0;
	}
	return 0;
}

/*************************************************************************
Function:    	get_dmsspt_buffer_paddr
Description: 	获取dmss pt的预留内存地址和大小;
Input:		NA
Return:		struct dmsspt_region ,包含预留内存的地址和大小；
Author:		j00207786
*************************************************************************/
struct dmsspt_region  get_dmsspt_buffer_region(void)
{
	return pt_region;
}

/*************************************************************************
Function:    	dmsspt_init
Description: 	dmsspt的驱动初始化函数；
Input:		NA
Output:		NA
Return:		0：成功； 1：失败；
Author:		j00207786
*************************************************************************/
static int __init dmsspt_init (void)
{
	long i, j;

	if (!check_himntn(HIMNTN_DMSSPT)) {
		pr_err("%s, HIMNTN_DMSSPT is closed!\n", __func__);
		return 0;
	}

	/*预留内存地址映射；*/
	if (!pt_region.base) {
		pr_err("%s, dynamic mem reserved is not successfull!\n", __func__);
		return 0;
	}

	g_trace_buffer_vaddr = bbox_vmap(pt_region.base, pt_region.size);
	if (!g_trace_buffer_vaddr) {
		pr_err("%s, g_trace_buffer_vaddr ioremap failed!\n", __func__);
		return -1;
	}
	pr_err("%s, g_trace_buffer_vaddr [0x%pK] buffer size [0x%llx]!\n", __func__,
		g_trace_buffer_vaddr, pt_region.size);

	init_completion(&dmsspt_date);
	complete(&dmsspt_date);
	for (i = 0; i < DMSS_TRACE_MAX; i++) {
		PT_SEM_INIT(i); /*lint !e64 !e651 */
		pr_err("%s, create thread dmsspt_dump_thread%ld.\n",
		       __func__, i);
		dmsspt_main[i] = kthread_run(dmss_pattern_trace_dump_thread,
				(void*)(i+DMSSPT_NUM_OFFSET), "dmsspt_dthrd%ld", i);
		if (!dmsspt_main[i]) {
			pr_err("%s, create thread dmsspt_dump_thread failed.\n",
		       __func__);
			for (j = 0; j < DMSS_TRACE_MAX; j++) {
				if (dmsspt_main[j])
					kthread_stop(dmsspt_main[j]);
			}
			return -1;
		}
	}
	g_pt_info.intlv_mode = DMSSPT_NOINIT_MAGIC;

	pr_err("%s, success!\n", __func__);
	return 0;
}

/*************************************************************************
Function:		dmsspt_exit
Description:    dmsspt的驱动退出函数；
Input:		NA
Return:		NA
Author:			j00207786
*************************************************************************/
static void __exit dmsspt_exit(void)
{
	int i;

	for (i = 0; i < DMSS_TRACE_MAX; i++) {
		if (dmsspt_main[i])
			kthread_stop(dmsspt_main[i]);
	}
}

/*lint -e528 -esym(528,*)*/
core_initcall(dmsspt_init);
module_exit(dmsspt_exit);
/*lint -e528 +esym(528,*)*/

/*lint -e753 -esym(753,*)*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("j00207786 <j00207786@notesmail.huawei.com>");
/*lint -e753 +esym(753,*)*/

