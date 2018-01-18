#ifndef __PIMON_MNTN_H__
#define __PIMON_MNTN_H__

#define PATH_MAXLEN 128
#define FILE_NAME_MAXLEN 64
#define FILE_MAX_NUM 3 /*����ļ������������Ḳ��*/
#define PIMON_PATH "/data/hisi_logs/pimon/"
#define PIMON_PERFIX "pimon_data_"
#define PIMON_SUFFIX ".bin"
#define DIR_LIMIT 0770
#define FILE_LIMIT 0660
#define ROOT_UID 0
#define SYSTEM_GID 1000

/*���ڱ�ʶ���ݵ�ͷ��Ϣ������PC�����*/
#define PIMON_DATA_TAG_START (0xFEDCBA9876543210)

/*���ڱ�ʶ���ݵ�β��Ϣ������PC�����*/
#define PIMON_DATA_TAG_END (0x0123456789ABCDEF)

/*trigger_on��λ��������*/
#define PIMON_REG_BIT_MASK_TRIG 0x00000800
#define PIMON_REG_BIT_TRIG 11

/*channel_id��λ��������*/
#define PIMON_REG_BIT_MASK_CHANNEL 0x0000F000
#define PIMON_REG_BIT_CHANNEL 12

/*pi_dc_sel��λ��������*/
#define PIMON_REG_BIT_MASK_PIDCSEL 0x00000E00
#define PIMON_REG_BIT_PIDCSEL 9

/*vref_h��λ��������*/
#define PIMON_REG_BIT_MASK_VREF_H 0x000001C0
#define PIMON_REG_BIT_VREF_H 6

/*vref_l��λ��������*/
#define PIMON_REG_BIT_MASK_VREF_L 0x00000038
#define PIMON_REG_BIT_VREF_L 3

/*uv_threshold��λ��������*/
#define PIMON_REG_BIT_MASK_UV_TH 0x001F0000
#define PIMON_REG_BIT_UV_TH 16

/*ov_threshold��λ��������*/
#define PIMON_REG_BIT_MASK_OV_TH 0x0000001F
#define PIMON_REG_BIT_OV_TH 0

/*uv_filter_pionts��λ��������*/
#define PIMON_REG_BIT_MASK_UV_CNT 0xFFFF0000
#define PIMON_REG_BIT_UV_CNT 16

/*ov_filter_pionts��λ��������*/
#define PIMON_REG_BIT_MASK_OV_CNT 0x0000FFFF
#define PIMON_REG_BIT_OV_CNT 0

/*��ȡpimon_last_waddr_lo�Ĵ�����ǰѡ��block��λ��������*/
#define PIMON_REG_BIT_MASK_BLKS_SELECTED 0xF0000000
#define PIMON_REG_BIT_BLKS_SELECTED 28
#define PIMON_REG_BLK0_SELECTED 0x00000001
#define PIMON_REG_BLK1_SELECTED 0x00000002
#define PIMON_REG_BLK2_SELECTED 0x00000004
#define PIMON_REG_BLK3_SELECTED 0x00000008

/*��ȡpimon_last_waddr_lo�Ĵ������burstд��ַ��λ��������*/
#define PIMON_REG_BIT_MASK_LAST_WR_BASE 0x07F80000
#define PIMON_REG_BIT_LAST_WR_BASE 31
#define PIMON_REG_BIT_MASK_LAST_WR_OFFSET 0x0FFFFFFF
#define PIMON_REG_BIT_LAST_WR_OFFSET 3

/*��ȡpimon_int_start�Ĵ����ж�״̬������*/
#define PIMON_INT_BIT_MASK_FIFO_FULL 0x00000001
#define PIMON_INT_BIT_MASK_BLKS_ERR 0x00000002
#define PIMON_INT_BIT_MASK_BLK0_FULL 0x00000004
#define PIMON_INT_BIT_MASK_BLK1_FULL 0x00000008
#define PIMON_INT_BIT_MASK_BLK2_FULL 0x00000010
#define PIMON_INT_BIT_MASK_BLK3_FULL 0x00000020
#define PIMON_INT_BIT_MASK_AXI_WRERR 0x00000040
#define PIMON_INT_BIT_MASK_OV 0x00000080
#define PIMON_INT_BIT_MASK_UV 0x00000100

#define PIMON_INT_BITS_MASK_ERR (PIMON_INT_BIT_MASK_FIFO_FULL \
	| PIMON_INT_BIT_MASK_BLKS_ERR | PIMON_INT_BIT_MASK_AXI_WRERR)

#define PIMON_INT_BITS_MASK_BLK_FULL (PIMON_INT_BIT_MASK_BLK0_FULL \
	| PIMON_INT_BIT_MASK_BLK1_FULL | PIMON_INT_BIT_MASK_BLK2_FULL \
	| PIMON_INT_BIT_MASK_BLK3_FULL)

#define PIMON_INT_BITS_MASK_TRIG (PIMON_INT_BIT_MASK_OV | PIMON_INT_BIT_MASK_UV)

#define PIMON_INT_BITS_MASK_CLR (PIMON_INT_BIT_MASK_FIFO_FULL \
	| PIMON_INT_BIT_MASK_BLKS_ERR | PIMON_INT_BIT_MASK_BLK0_FULL \
	| PIMON_INT_BIT_MASK_BLK1_FULL | PIMON_INT_BIT_MASK_BLK2_FULL \
	| PIMON_INT_BIT_MASK_BLK3_FULL | PIMON_INT_BIT_MASK_AXI_WRERR \
	| PIMON_INT_BIT_MASK_OV | PIMON_INT_BIT_MASK_UV)

/*����blk��ʵ��ַ�����������뼰��λλ��*/
#define PIMON_REG_BIT_MASK_REAL_ADDR 0x07FFFFFF
#define PIMON_REG_BIT_REAL_ADDR 12
#define PIMON_REG_BIT_MASK_REAL_SIZE 0x0003FFFF
#define PIMON_REG_BIT_REAL_SIZE 12

/*�ر�FIFO����*/
#define PIMON_CLOSE_FIFO_MASK 0x00000002
/*�ر�trigger����*/
#define PIMON_CLOSE_TRIGGER_MASK 0x00000800

/*Ƭ�����ݻ��������־*/
#define PIMON_FRAG_BUF_DONE_BITS 0x00000100
/*Ƭ�����ݻ�������д��ַMASK*/
#define PIMON_FRAG_RAM_ADDR_MASK 0x000000FF
/*Ƭ�����ݻ����ڲ�ram��size*/
#define PIMON_INSIDE_RAM_SIZE 0x2000
/*Ƭ�����ݻ��浥λ256bit*/
#define PIMON_INSIDE_RAM_STEP 0x20

/*pimon�Ĵ�����ƫ����*/
#define PIMON_REG_OFFSET_GLB_STAT 0x40
#define PIMON_REG_OFFSET_INT_STAT 0x80
#define PIMON_REG_OFFSET_INIT_MASK 0x88
#define PIMON_REG_OFFSET_CTRL 0x100
#define PIMON_REG_OFFSET_LAST_WADDR_LO 0x134
#define PIMON_REG_OFFSET_FIFO_STAT 0x174
#define PIMON_REG_OFFSET_CTRL_2 0x204
#define PIMON_REG_OFFSET_FRAG_ADDR 0x20C
#define PIMON_INSIDE_RAM_ADDR 0x1000

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;

/*���ڱ�ʾpimon֧�ֵ�block�ı��*/
enum e_pimon_blk_index {
	PIMON_BLOCK_0 = 0,
	PIMON_BLOCK_1,
	PIMON_BLOCK_2,
	PIMON_BLOCK_3,
	PIMON_BLOCK_NUM_MAX
};

/*�������ֵ�ǰ���õ���block�ĵ�ַ���Ǵ�С*/
enum e_pimon_blk_info_id {
	PIMON_BLOCK_ADDR = 0,
	PIMON_BLOCK_SIZE
};

/*���ڶ���pimonά��ģ���״̬*/
enum e_pimon_status {
	PIMON_INIT = 0,
	PIMON_IDLE,
	PIMON_TESTING,
	PIMON_TEST_OVER,
	PIMON_ERROR_HANDLE,
	/*�ڴ�֮ǰ�����״̬*/
	PIMON_STATUS_INVALID
};

/*���ڶ���pimon��������*/
enum e_pimon_test_type {
	PIMON_TEST_CONT = 0, /*�������ݻ���ģʽ*/
	PIMON_TEST_MUL_CONT, /*Ƭ�����ݻ���ģʽ*/
	PIMON_TEST_TRIGGER, /*triggerģʽ*/
	PIMON_TEST_RST_REQ, /*��λ����ģʽ*/
	/*�ڴ�֮ǰ����²�������*/
	PIMON_TEST_MAX
};

/*���ڶ����û����ĵ�pimon�ļĴ���id������������������ͷ��Ϣ�еļĴ���*/
enum e_pimon_user_input_reg_id {
	PIMON_REG_INT_STAT = 0x80, /*PIMON�ж�״̬�Ĵ���*/
	PIMON_REG_CTRL = 0x100, /*PIMON���ƼĴ���*/
	PIMON_REG_ANA_CFG = 0x108, /*PIMONģ�����üĴ���*/
	PIMON_REG_ANA_SET = 0x10C, /*PIMONģ�����üĴ���*/
	PIMON_REG_THRESHOLD = 0x160, /*PIMON��ֵ�Ĵ���*/
	PIMON_REG_TRIG_CNT = 0x164, /*PIMON��ֵ���˼Ĵ���*/
	PIMON_REG_CTRL_2 = 0x204, /*PIMONģʽѡ��Ĵ���*/
	/*�ڴ�֮ǰ����¼Ĵ���id*/
	PIMON_REG_DEFINE_MAX = 0xFFF
};

/*�������ֵ�ǰ���õ��ǲ������͡��Ĵ��������ǼĴ�������ַ*/
enum e_pimon_set_type {
	PIMON_SET_TEST_TYPE = 0, /*���ò�������*/
	PIMON_SET_REG, /*���üĴ���*/
	PIMON_SET_BASE_ADDR, /*���üĴ�������ַ*/
	/*�ڴ�֮ǰ�������������id*/
	PIMON_SET_MAX
};

/*���ڱ�ʶ���ݵ�ͷ��Ϣ������PC�����*/
struct s_head_info {
	u64 tag_start;
	u16 channel_id;
	u16 pi_dc_sel;
	u16 vref_h;
	u16 vref_l;
	u16 pimon_trig;
	u16 uv_threshold;
	u16 uv_filter_pionts;
	u16 ov_threshold;
	u16 ov_filter_pionts;
	u16 reserved_one;
	u16 reserved_two;
	u16 reserved_three;
	u16 reserved_four;
	u16 reserved_five;
	u16 reserved_six;
	u16 reserved_serven;
};

/*���ڱ�ʶ���ݵ�β��Ϣ������PC�����*/
struct s_tail_info {
	u64 tag_end;
};

 /*���ڼ�¼block�ĵ�ַ�ʹ�С��Ϣ*/
struct s_pimon_block_info {
	u32 addr;
	u32 size;
};

 /*���ڼ�¼pimonά�������е�������*/
struct s_pimon_mgr_info {
	u32 irq; /*pimon irq num*/
	void __iomem *reg_base_addr; /*pimon�Ĵ�������ַ��ӳ���ַ*/
	struct s_pimon_block_info blocks[PIMON_BLOCK_NUM_MAX];
	enum e_pimon_status pimon_status_cur;
	enum e_pimon_test_type pimon_test_type_cur;
	struct s_head_info pimon_data_head;
	struct s_tail_info pimon_data_tail;
	u32 trigger_happened; /*��ʶ�Ƿ���trigger�ж�*/
	u32 cul_blk; /*pimon��ǰд��blk id������triggerģʽ*/
	u32 full_blk_num; /*Ƭ��ģʽ��triggerģʽ����blk���жϵĴ���*/
};

extern void *bbox_vmap(phys_addr_t, size_t);
void pimon_mntn_debug_show(void);
int pimon_mntn_set_block_addr(u32 id, u32 reg_offset, u32 reg_value);
int pimon_mntn_set_block_size(u32 id, u32 reg_offset, u32 reg_value);
int pimon_mntn_set_value(u32 id, u32 reg_offset, u32 reg_value);
int pimon_save_block_data(enum e_pimon_test_type test_type);
#endif
