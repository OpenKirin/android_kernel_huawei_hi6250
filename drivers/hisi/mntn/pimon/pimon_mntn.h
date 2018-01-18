#ifndef __PIMON_MNTN_H__
#define __PIMON_MNTN_H__

#define PATH_MAXLEN 128
#define FILE_NAME_MAXLEN 64
#define FILE_MAX_NUM 3 /*最大文件个数，超过会覆盖*/
#define PIMON_PATH "/data/hisi_logs/pimon/"
#define PIMON_PERFIX "pimon_data_"
#define PIMON_SUFFIX ".bin"
#define DIR_LIMIT 0770
#define FILE_LIMIT 0660
#define ROOT_UID 0
#define SYSTEM_GID 1000

/*用于标识数据的头信息，便于PC侧解析*/
#define PIMON_DATA_TAG_START (0xFEDCBA9876543210)

/*用于标识数据的尾信息，便于PC侧解析*/
#define PIMON_DATA_TAG_END (0x0123456789ABCDEF)

/*trigger_on的位数及掩码*/
#define PIMON_REG_BIT_MASK_TRIG 0x00000800
#define PIMON_REG_BIT_TRIG 11

/*channel_id的位数及掩码*/
#define PIMON_REG_BIT_MASK_CHANNEL 0x0000F000
#define PIMON_REG_BIT_CHANNEL 12

/*pi_dc_sel的位数及掩码*/
#define PIMON_REG_BIT_MASK_PIDCSEL 0x00000E00
#define PIMON_REG_BIT_PIDCSEL 9

/*vref_h的位数及掩码*/
#define PIMON_REG_BIT_MASK_VREF_H 0x000001C0
#define PIMON_REG_BIT_VREF_H 6

/*vref_l的位数及掩码*/
#define PIMON_REG_BIT_MASK_VREF_L 0x00000038
#define PIMON_REG_BIT_VREF_L 3

/*uv_threshold的位数及掩码*/
#define PIMON_REG_BIT_MASK_UV_TH 0x001F0000
#define PIMON_REG_BIT_UV_TH 16

/*ov_threshold的位数及掩码*/
#define PIMON_REG_BIT_MASK_OV_TH 0x0000001F
#define PIMON_REG_BIT_OV_TH 0

/*uv_filter_pionts的位数及掩码*/
#define PIMON_REG_BIT_MASK_UV_CNT 0xFFFF0000
#define PIMON_REG_BIT_UV_CNT 16

/*ov_filter_pionts的位数及掩码*/
#define PIMON_REG_BIT_MASK_OV_CNT 0x0000FFFF
#define PIMON_REG_BIT_OV_CNT 0

/*获取pimon_last_waddr_lo寄存器当前选中block的位数及掩码*/
#define PIMON_REG_BIT_MASK_BLKS_SELECTED 0xF0000000
#define PIMON_REG_BIT_BLKS_SELECTED 28
#define PIMON_REG_BLK0_SELECTED 0x00000001
#define PIMON_REG_BLK1_SELECTED 0x00000002
#define PIMON_REG_BLK2_SELECTED 0x00000004
#define PIMON_REG_BLK3_SELECTED 0x00000008

/*获取pimon_last_waddr_lo寄存器最后burst写地址的位数及掩码*/
#define PIMON_REG_BIT_MASK_LAST_WR_BASE 0x07F80000
#define PIMON_REG_BIT_LAST_WR_BASE 31
#define PIMON_REG_BIT_MASK_LAST_WR_OFFSET 0x0FFFFFFF
#define PIMON_REG_BIT_LAST_WR_OFFSET 3

/*获取pimon_int_start寄存器中断状态的掩码*/
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

/*计算blk真实地址和容量的掩码及移位位数*/
#define PIMON_REG_BIT_MASK_REAL_ADDR 0x07FFFFFF
#define PIMON_REG_BIT_REAL_ADDR 12
#define PIMON_REG_BIT_MASK_REAL_SIZE 0x0003FFFF
#define PIMON_REG_BIT_REAL_SIZE 12

/*关闭FIFO掩码*/
#define PIMON_CLOSE_FIFO_MASK 0x00000002
/*关闭trigger掩码*/
#define PIMON_CLOSE_TRIGGER_MASK 0x00000800

/*片段数据缓存结束标志*/
#define PIMON_FRAG_BUF_DONE_BITS 0x00000100
/*片段数据缓存最新写地址MASK*/
#define PIMON_FRAG_RAM_ADDR_MASK 0x000000FF
/*片段数据缓存内部ram的size*/
#define PIMON_INSIDE_RAM_SIZE 0x2000
/*片段数据缓存单位256bit*/
#define PIMON_INSIDE_RAM_STEP 0x20

/*pimon寄存器和偏移量*/
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

/*用于表示pimon支持的block的编号*/
enum e_pimon_blk_index {
	PIMON_BLOCK_0 = 0,
	PIMON_BLOCK_1,
	PIMON_BLOCK_2,
	PIMON_BLOCK_3,
	PIMON_BLOCK_NUM_MAX
};

/*用于区分当前设置的是block的地址还是大小*/
enum e_pimon_blk_info_id {
	PIMON_BLOCK_ADDR = 0,
	PIMON_BLOCK_SIZE
};

/*用于定义pimon维测模块的状态*/
enum e_pimon_status {
	PIMON_INIT = 0,
	PIMON_IDLE,
	PIMON_TESTING,
	PIMON_TEST_OVER,
	PIMON_ERROR_HANDLE,
	/*在此之前添加新状态*/
	PIMON_STATUS_INVALID
};

/*用于定义pimon测试类型*/
enum e_pimon_test_type {
	PIMON_TEST_CONT = 0, /*连续数据缓存模式*/
	PIMON_TEST_MUL_CONT, /*片段数据缓存模式*/
	PIMON_TEST_TRIGGER, /*trigger模式*/
	PIMON_TEST_RST_REQ, /*复位请求模式*/
	/*在此之前添加新测试类型*/
	PIMON_TEST_MAX
};

/*用于定义用户关心的pimon的寄存器id，包括但不限于数据头信息中的寄存器*/
enum e_pimon_user_input_reg_id {
	PIMON_REG_INT_STAT = 0x80, /*PIMON中断状态寄存器*/
	PIMON_REG_CTRL = 0x100, /*PIMON控制寄存器*/
	PIMON_REG_ANA_CFG = 0x108, /*PIMON模拟配置寄存器*/
	PIMON_REG_ANA_SET = 0x10C, /*PIMON模拟设置寄存器*/
	PIMON_REG_THRESHOLD = 0x160, /*PIMON阈值寄存器*/
	PIMON_REG_TRIG_CNT = 0x164, /*PIMON阈值过滤寄存器*/
	PIMON_REG_CTRL_2 = 0x204, /*PIMON模式选择寄存器*/
	/*在此之前添加新寄存器id*/
	PIMON_REG_DEFINE_MAX = 0xFFF
};

/*用于区分当前设置的是测试类型、寄存器、还是寄存器基地址*/
enum e_pimon_set_type {
	PIMON_SET_TEST_TYPE = 0, /*设置测试类型*/
	PIMON_SET_REG, /*设置寄存器*/
	PIMON_SET_BASE_ADDR, /*设置寄存器基地址*/
	/*在此之前添加新设置类型id*/
	PIMON_SET_MAX
};

/*用于标识数据的头信息，便于PC侧解析*/
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

/*用于标识数据的尾信息，便于PC侧解析*/
struct s_tail_info {
	u64 tag_end;
};

 /*用于记录block的地址和大小信息*/
struct s_pimon_block_info {
	u32 addr;
	u32 size;
};

 /*用于记录pimon维测框架运行的上下文*/
struct s_pimon_mgr_info {
	u32 irq; /*pimon irq num*/
	void __iomem *reg_base_addr; /*pimon寄存器基地址的映射地址*/
	struct s_pimon_block_info blocks[PIMON_BLOCK_NUM_MAX];
	enum e_pimon_status pimon_status_cur;
	enum e_pimon_test_type pimon_test_type_cur;
	struct s_head_info pimon_data_head;
	struct s_tail_info pimon_data_tail;
	u32 trigger_happened; /*标识是否发生trigger中断*/
	u32 cul_blk; /*pimon当前写的blk id，用于trigger模式*/
	u32 full_blk_num; /*片段模式、trigger模式发生blk满中断的次数*/
};

extern void *bbox_vmap(phys_addr_t, size_t);
void pimon_mntn_debug_show(void);
int pimon_mntn_set_block_addr(u32 id, u32 reg_offset, u32 reg_value);
int pimon_mntn_set_block_size(u32 id, u32 reg_offset, u32 reg_value);
int pimon_mntn_set_value(u32 id, u32 reg_offset, u32 reg_value);
int pimon_save_block_data(enum e_pimon_test_type test_type);
#endif
