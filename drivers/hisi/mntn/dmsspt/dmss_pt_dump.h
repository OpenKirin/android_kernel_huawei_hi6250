
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
	TRACE_MODE0 = 0, //����ģʽ
	TRACE_MODE1 = 1, //����ģʽ
	TRACE_MODE_MAX,
}
INTLV_MODE;

typedef enum {
	DMSS_TRACE0 = 0, //Ӳ��traceģ��0
	DMSS_TRACE1 = 1, //Ӳ��traceģ��1
	DMSS_TRACE_MAX,
}
TRACE_MOD;

struct dmsspt_region {
	phys_addr_t			base;
	phys_addr_t			size;
};

struct pattern_trace_info {
	u32 intlv;  //��֯����
	u32 intlv_mode;  //��֯ģʽ
	u64 pt_wptr[DMSS_TRACE_MAX]; /*2��trace ���Ե�дָ�룬ACPU������Ԥ��buffer��ַ��
									������Ϊdmsspt_save_trace�ӿ���δ����Ϊ�����ַ��
									ȫ�ֱ���g_pt_wptr�д����ת�������������ַ*/
	u64 pt_rptr[DMSS_TRACE_MAX];/*ACPU�����������ַ*/
};

struct pattern_trace_stat {
	u64 trace_begin_time;  //trace��ʼʱ��
	u64 trace_end_time;  //trace����ʱ��
	u32 trace_cur_address[DMSS_TRACE_MAX];  //GLB_TRACE_STAT0;
	u32 trace_pattern_cnt[DMSS_TRACE_MAX]; //GLB_TRACE_STAT1;
	u32 trace_roll_cnt[DMSS_TRACE_MAX]; //GLB_TRACE_STAT2;
	u32 trace_unaligned_mode; //GLB_TRACE_CTRL1[28];
};

struct pattern_trace_header {
	u64 trace_begin_time;  //trace��ʼʱ��
	u64 pad0;
	u64 trace_end_time;  //trace����ʱ��
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
