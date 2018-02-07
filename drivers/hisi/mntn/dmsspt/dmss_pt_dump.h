
#ifndef _DMSS_PT_H
#define _DMSS_PT_H

#define DMSSPT_UNIT_SIZE		128

#define DMSSPT_ROOT "/data/hisi_logs/running_trace/dmss_pt/"

#define DMSSPT_DUMP_TIMEOUT 	5
#define DMSSPT_DIR_LIMIT		0770
#define DMSSPT_FILE_LIMIT		0660
#define DMSSPT_PATH_MAXLEN         128UL
#define DMSSPT_NUM_OFFSET		1

#define DMSSPT_RT_PRIORITY		90

#define DMSSPT_FILE_MAX_LEN			0x40000000UL

#define DMSSPT_NOINIT_MAGIC		0xC5C5C5C5U
#define DMSSPT_FS_INIT_MAGIC		0x1FFFFFFF

typedef enum {
	TRACE_MODE0 = 0, //功耗模式
	TRACE_MODE1 = 1, //性能模式
	TRACE_MODE_MAX,
}
INTLV_MODE;

typedef enum {
	DMSS_TRACE0 = 0, //硬件trace模块0
	DMSS_TRACE1 = 1, //硬件trace模块1
	DMSS_TRACE_MAX,
}
TRACE_MOD;

struct dmsspt_region {
	phys_addr_t			base;
	phys_addr_t			size;
};

struct pattern_trace_info {
	u32 intlv;  //交织粒度
	u32 intlv_mode;  //交织模式
	u64 pt_wptr[DMSS_TRACE_MAX]; /*2个trace 各自的写指针，ACPU看到的预留buffer地址，
									其中作为dmsspt_save_trace接口入参传入的为物理地址，
									全局变量g_pt_wptr中存的是转换出来的虚拟地址*/
	u64 pt_rptr[DMSS_TRACE_MAX];/*ACPU看到的物理地址*/
};

struct pattern_trace_stat {
	u64 trace_begin_time;  //trace开始时间
	u64 trace_end_time;  //trace结束时间
	u32 trace_cur_address[DMSS_TRACE_MAX];  //GLB_TRACE_STAT0;
	u32 trace_pattern_cnt[DMSS_TRACE_MAX]; //GLB_TRACE_STAT1;
	u32 trace_roll_cnt[DMSS_TRACE_MAX]; //GLB_TRACE_STAT2;
	u32 trace_unaligned_mode; //GLB_TRACE_CTRL1[28];
};

struct pattern_trace_header {
	u64 trace_begin_time;  //trace开始时间
	u64 pad0;
	u64 trace_end_time;  //trace结束时间
	u64 pad1;
	u32 trace_cur_address;  //GLB_TRACE_STAT0;
	u32 trace_pattern_cnt; //GLB_TRACE_STAT1;
	u32 trace_roll_cnt; //GLB_TRACE_STAT2;
	u32 trace_unaligned_mode; //GLB_TRACE_CTRL1[28];
};

#ifdef CONFIG_HISI_DMSS_PT
int dmsspt_save_trace(struct pattern_trace_info* pinfo, struct pattern_trace_stat* pstat, int is_tracing_stop);
struct dmsspt_region  get_dmsspt_buffer_region(void);
int dmsspt_is_finished(void);
#else
static inline int dmsspt_save_trace(struct pattern_trace_info* pinfo, struct pattern_trace_stat* pstat, int is_tracing_stop)
{ return 0; }
static inline struct dmsspt_region  get_dmsspt_buffer_region(void)
{
	struct dmsspt_region region;
	region.base = 0;
	region.size = 0;
	return region;
}
#endif

#endif /*_DMSS_PT_H*/
