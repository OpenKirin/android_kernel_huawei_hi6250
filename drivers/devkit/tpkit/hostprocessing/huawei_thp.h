/*
 * Huawei Touchscreen Driver
 *
 * Copyright (C) 2017 Huawei Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#ifndef _THP_H_
#define _THP_H_

/*
 * define THP_CHARGER_FB here to enable charger notify callback
 */
#define THP_CHARGER_FB
#if defined(THP_CHARGER_FB)
#include <linux/hisi/usb/hisi_usb.h>
#endif
#include <linux/amba/pl022.h>
#include <huawei_platform/log/hw_log.h>

#define THP_UNBLOCK		(5)
#define THP_TIMEOUT		(6)

#define THP_RESET_LOW	(0)
#define THP_RESET_HIGH	(1)

#define THP_IRQ_ENABLE 1
#define THP_IRQ_DISABLE 0

#define THP_IO_TYPE	 (0xB8)
#define THP_IOCTL_CMD_GET_FRAME	\
		_IOWR(THP_IO_TYPE, 0x01, struct thp_ioctl_get_frame_data)
#define THP_IOCTL_CMD_RESET	_IOW(THP_IO_TYPE, 0x02, u32)
#define THP_IOCTL_CMD_SET_TIMEOUT	_IOW(THP_IO_TYPE, 0x03, u32)
#define THP_IOCTL_CMD_SPI_SYNC	\
		_IOWR(THP_IO_TYPE, 0x04, struct thp_ioctl_spi_sync_data)
#define THP_IOCTL_CMD_FINISH_NOTIFY	_IO(THP_IO_TYPE, 0x05)

#define GPIO_LOW  (0)
#define GPIO_HIGH (1)
#define DUR_RESET_HIGH	(1)
#define DUR_RESET_LOW	(0)
#define WAITQ_WAIT	 (0)
#define WAITQ_WAKEUP (1)
#define THP_MAX_FRAME_SIZE (8*1024+16)
#define THP_DEFATULT_TIMEOUT_MS 200
#define THP_SPI_SPEED_DEFAULT (20 * 1000 * 1000)

#define THP_LIST_MAX_FRAMES			20

#define THP_PROJECT_ID_LEN 10

#define IS_TMO(t)                   ((t) == 0)

#define thp_do_time_delay(x) \
	do {		\
		if (x)	\
			msleep(x); \
	} while (0)

#define THP_DEV_COMPATIBLE "huawei,thp"
#define THP_SPI_DEV_NODE_NAME "thp_spi_dev"
#define THP_TIMING_NODE_NAME "thp_timing"

struct thp_ioctl_get_frame_data {
	char __user *buf;
	char __user *tv; /* struct timeval* */
	unsigned int size;
};

struct thp_ioctl_spi_sync_data {
	char __user *tx;
	char __user *rx;
	unsigned int size;
};

struct thp_frame {
	struct list_head list;
#ifdef THP_NOVA_ONLY
	u8 frame[NT_MAX_FRAME_SIZE];
#else
	u8 frame[THP_MAX_FRAME_SIZE];
#endif
	struct timeval tv;
};

#if defined(CONFIG_HUAWEI_DSM)
struct host_dsm_info {
	int constraints_SPI_status;
};
#endif

struct thp_device_ops {
	int (*init)(struct thp_device *tdev);
	int (*detect)(struct thp_device *tdev);
	int (*get_frame)(struct thp_device *tdev, char *buf, unsigned int len);
	int (*resume)(struct thp_device *tdev);
	int (*suspend)(struct thp_device *tdev);
	void (*exit)(struct thp_device *tdev);
};

struct thp_device {
	char *ic_name;
	struct thp_device_ops *ops;
	struct thp_core_data *thp_core;
	char *tx_buff;
	char *rx_buff;
};

struct thp_spi_config {
	u32 max_speed_hz;
	u16 mode;
	u8 bits_per_word;
	struct pl022_config_chip pl022_spi_config;
};

struct thp_timing_config {
	u32 boot_reset_hi_delay_ms;
	u32 boot_reset_low_delay_ms;
	u32 boot_reset_after_delay_ms;
	u32 resume_reset_after_delay_ms;
	u32 suspend_reset_after_delay_ms;
	u32 spi_sync_cs_hi_delay_ns;
	u32 spi_sync_cs_low_delay_ns;
};

struct thp_core_data {
	struct spi_device *sdev;
	struct thp_device *thp_dev;
	struct device_node *thp_node;
	struct notifier_block fb_notify;
	struct thp_frame frame_list;
	struct thp_spi_config spi_config;
	struct kobject *thp_obj;
	struct platform_device *ts_dev;
	struct mutex mutex_frame;
	struct mutex irq_mutex;
	struct mutex thp_mutex;
	atomic_t register_flag;
	int open_count;
	int frame_count;
	unsigned int frame_size;
	bool irq_enabled;
	int irq;
	unsigned int irq_flag;
	u8 frame_read_buf[THP_MAX_FRAME_SIZE];
	u8 frame_waitq_flag;
	wait_queue_head_t frame_waitq;
	u8 reset_flag;
	unsigned int timeout;
	int irq_gpio;
	int rst_gpio;
	int cs_gpio;
	bool suspended;
#if defined(CONFIG_HUAWEI_DSM)
	struct host_dsm_info dsm_info;
#endif
#if defined(THP_CHARGER_FB)
	int charger_state;
	struct notifier_block charger_detect_notify;
#endif
	char project_id[THP_PROJECT_ID_LEN + 1];
	const char *ic_name;
	const char *vendor_name;
	struct thp_timing_config timing_config;
	int tui_flag;
};

extern u8 g_thp_log_cfg;

#define HWLOG_TAG	THP
HWLOG_REGIST();
#define THP_LOG_INFO(x...)		_hwlog_info(HWLOG_TAG, ##x)
#define THP_LOG_ERR(x...)		_hwlog_err(HWLOG_TAG, ##x)
#define THP_LOG_DEBUG(x...)	\
	do { \
		if (g_thp_log_cfg)	\
			_hwlog_info(HWLOG_TAG, ##x);	\
	} while (0)
extern int hostprocessing_get_project_id(char *out);
int thp_register_dev(struct thp_device *dev);
int thp_parse_spi_config(struct device_node *spi_cfg_node,
					struct thp_core_data *cd);
struct thp_core_data *thp_get_core_data(void);
int thp_parse_timing_config(struct device_node *timing_cfg_node,
			struct thp_core_data *cd);
#endif /* _THP_H_ */

