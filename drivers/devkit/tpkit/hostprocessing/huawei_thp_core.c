/*
 * Huawei Touchscreen Driver
 *
 * Copyright (C) 2017 Huawei Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include "huawei_thp.h"
#include "huawei_thp_mt_wrapper.h"
#include "huawei_thp_attr.h"

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

struct thp_core_data *g_thp_core;

#if defined(CONFIG_HUAWEI_DSM)
static struct dsm_dev dsm_thp = {
	.name = "dsm_thp",
	.device_name = "THP",
	.ic_name = "syn",
	.module_name = "NNN",
	.fops = NULL,
	.buff_size = 1024,
};

struct dsm_client *dsm_thp_dclient;

#endif

#define THP_DEVICE_NAME	"huawei_thp"
#define THP_MISC_DEVICE_NAME "thp"

struct thp_core_data *thp_get_core_data(void)
{
	return g_thp_core;
}

static void thp_wake_up_frame_waitq(struct thp_core_data *cd)
{
	cd->frame_waitq_flag = WAITQ_WAKEUP;
	wake_up_interruptible(&(cd->frame_waitq));
}

static int thp_wait_frame_waitq(struct thp_core_data *cd)
{
	int t;

	cd->frame_waitq_flag = WAITQ_WAIT;

	/* if not use timeout*/
	if (!cd->timeout) {
		t = wait_event_interruptible(cd->frame_waitq,
				(cd->frame_waitq_flag == WAITQ_WAKEUP));
		return 0;
	}

	/* if use timeout*/
	t = wait_event_interruptible_timeout(cd->frame_waitq,
			(cd->frame_waitq_flag == WAITQ_WAKEUP),
			msecs_to_jiffies(cd->timeout));
	if (!IS_TMO(t))
		return 0;


	THP_LOG_ERR("%s: wait frame timed out\n", __func__);

#if defined(CONFIG_HUAWEI_DSM)
	if (!dsm_client_ocuppy(dsm_thp_dclient)) {
		dsm_client_record(dsm_thp_dclient,
			"irq_gpio:%d\tvalue:%d.\n\
			reset_gpio:%d\tvalue:%d.\n\
			THP_status:%d.\n",
			cd->irq_gpio,
			gpio_get_value(cd->irq_gpio),
			cd->rst_gpio,
			gpio_get_value(cd->rst_gpio),
			ETIMEDOUT);
		dsm_client_notify(dsm_thp_dclient,
			DSM_TPHOSTPROCESSING_DEV_STATUS_ERROR_NO);
	}
#endif

	return -ETIMEDOUT;
}

static void thp_clear_frame_buffer(struct thp_core_data *cd)
{
	struct thp_frame *temp;
	struct list_head *pos, *n;

	if (list_empty(&cd->frame_list.list))
		return;

	list_for_each_safe(pos, n, &cd->frame_list.list) {
		temp = list_entry(pos, struct thp_frame, list);
		list_del(pos);
		kfree(temp);
	}

	cd->frame_count = 0;
}

static int thp_spi_transfer(struct thp_core_data *cd,
			char *tx_buf, char *rx_buf, unsigned int len)
{
	struct spi_message msg;
	struct spi_device *sdev = cd->sdev;

	struct spi_transfer xfer = {
		.tx_buf = tx_buf,
		.rx_buf = rx_buf,
		.len    = len,
	};

	if (cd->suspended)
		return 0;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);
	return spi_sync(sdev, &msg);
}

/*
 * If irq is disabled/enabled, can not disable/enable again
 * disable - status 0; enable - status not 0
 */
static void thp_set_irq_status(struct thp_core_data *cd, int status)
{
	mutex_lock(&cd->irq_mutex);
	if (cd->irq_enabled != !!status) {
		status ? enable_irq(cd->irq) : disable_irq(cd->irq);
		cd->irq_enabled = !!status;
		THP_LOG_INFO("%s: %s irq\n", __func__,
				status ? "enable" : "disable");
	}
	mutex_unlock(&cd->irq_mutex);
};

static int thp_fb_suspend(struct thp_core_data *cd)
{
	THP_LOG_INFO("%s: called\n", __func__);

	if (cd->suspended) {
		THP_LOG_INFO("%s: already suspended, return\n", __func__);
		return 0;
	}

	cd->suspended = true;

	/* wake up */
	thp_wake_up_frame_waitq(cd);

	/* empty list */
	mutex_lock(&cd->mutex_frame);
	thp_clear_frame_buffer(cd);
	mutex_unlock(&cd->mutex_frame);

	if (cd->open_count)
		thp_set_irq_status(cd, THP_IRQ_DISABLE);

	cd->thp_dev->ops->suspend(cd->thp_dev);

	return 0;
}

static int thp_fb_resume(struct thp_core_data *cd)
{
	THP_LOG_INFO("%s: called\n", __func__);

	if (!cd->suspended) {
		THP_LOG_INFO("%s: already resumed, return\n", __func__);
		return 0;
	}

	cd->thp_dev->ops->resume(cd->thp_dev);

	/*
	 * clear rawdata frame buffer list
	 */
	mutex_lock(&cd->mutex_frame);
	thp_clear_frame_buffer(cd);
	mutex_unlock(&cd->mutex_frame);

	if (cd->open_count)
		thp_set_irq_status(cd, THP_IRQ_ENABLE);

	cd->suspended = false;
	return 0;
}

static int thp_fb_notifier_callback(struct notifier_block *self,
					unsigned long event, void *data)
{
	struct fb_event *evdata = data;
	int *blank = evdata->data;
	struct thp_core_data *cd = thp_get_core_data();

	if (!evdata || !evdata->data) {
		THP_LOG_DEBUG("%s: input null\n", __func__);
		return 0;
	}

	if (event == FB_EARLY_EVENT_BLANK) {
		if (*blank == FB_BLANK_POWERDOWN) {
			THP_LOG_INFO("%s: fb early suspend\n", __func__);
			thp_fb_suspend(cd);
		} else if (*blank == FB_BLANK_UNBLANK) {
			THP_LOG_DEBUG("%s: fb early resume\n", __func__);
		}
	} else if (event == FB_EVENT_BLANK) {
		if (*blank == FB_BLANK_POWERDOWN) {
			THP_LOG_DEBUG("%s: fb suspend\n", __func__);
		} else if (*blank == FB_BLANK_UNBLANK) {
			THP_LOG_INFO("%s: fb resume\n", __func__);
			thp_fb_resume(cd);
		}
	}

	return 0;
}

static int thp_open(struct inode *inode, struct file *filp)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_frame *temp;
	struct list_head *pos, *n;

	THP_LOG_INFO("%s: called\n", __func__);

	mutex_lock(&cd->thp_mutex);
	if (cd->open_count) {
		THP_LOG_ERR("%s: dev have be opened\n", __func__);
		mutex_unlock(&cd->thp_mutex);
		return -EBUSY;
	}

	cd->open_count++;
	mutex_unlock(&cd->thp_mutex);

	cd->reset_flag = 0;

	cd->frame_size = THP_MAX_FRAME_SIZE;
#ifdef THP_NOVA_ONLY
	cd->frame_size = NT_MAX_FRAME_SIZE;
#endif
	cd->timeout = THP_DEFATULT_TIMEOUT_MS;

	thp_clear_frame_buffer(cd);

	return 0;
}

static int thp_release(struct inode *inode, struct file *filp)
{
	struct thp_core_data *cd = thp_get_core_data();

	THP_LOG_INFO("%s: called\n", __func__);

	mutex_lock(&cd->thp_mutex);
	cd->open_count--;
	if (cd->open_count < 0) {
		THP_LOG_ERR("%s: abnormal release\n", __func__);
		cd->open_count = 0;
	}
	mutex_unlock(&cd->thp_mutex);

	thp_wake_up_frame_waitq(cd);
	thp_set_irq_status(cd, THP_IRQ_DISABLE);

	return 0;
}


static long thp_ioctl_spi_sync(void __user *data)
{
	struct thp_core_data *cd = thp_get_core_data();
	int rc;
	u8 *tx_buf;
	u8 *rx_buf;
	struct thp_ioctl_spi_sync_data sync_data;

	THP_LOG_DEBUG("%s: called\n", __func__);

	if (cd->suspended)
		return 0;

	if (copy_from_user(&sync_data, data,
				sizeof(struct thp_ioctl_spi_sync_data))) {
		THP_LOG_ERR("Failed to copy_from_user().\n");
		return -EFAULT;
	}

	rx_buf = kzalloc(sync_data.size, GFP_KERNEL);
	tx_buf = kzalloc(sync_data.size, GFP_KERNEL);
	if (!rx_buf || !tx_buf) {
		THP_LOG_ERR("%s:buf request memory fail\n", __func__);
		goto exit;
	}

	rc = copy_from_user(tx_buf, sync_data.tx, sync_data.size);
	if (rc) {
		THP_LOG_ERR("%s:copy in buff fail\n", __func__);
		goto exit;
	}

	rc =  thp_spi_transfer(cd, tx_buf, rx_buf, sync_data.size);
	if (rc) {
		THP_LOG_ERR("%s: transfer error, ret = %d\n", __func__);
		goto exit;
	}

	if (sync_data.rx) {
		rc = copy_to_user(sync_data.rx, rx_buf, sync_data.size);
		if (rc) {
			THP_LOG_ERR("%s:copy out buff fail\n", __func__);
			goto exit;
		}
	}

exit:
	kfree(rx_buf);
	kfree(tx_buf);
	return rc;
}

static long thp_ioctl_finish_notify(unsigned long arg)
{
	THP_LOG_INFO("%s: called\n", __func__);
	return 0;
}

static long thp_ioctl_get_frame(unsigned long arg, unsigned int f_flag)
{
	long rc;
	struct thp_core_data *cd = thp_get_core_data();
	void __user *argp = (void __user *)arg;
	struct thp_ioctl_get_frame_data data;

	if (!arg) {
		THP_LOG_ERR("%s: input parameter null\n", __func__);
		return -EINVAL;
	}

	if (copy_from_user(&data, argp,
			sizeof(struct thp_ioctl_get_frame_data))) {
		THP_LOG_ERR("Failed to copy_from_user .\n");
		return -EFAULT;
	}

	if (data.buf == 0 || data.size == 0 ||
		data.size > THP_MAX_FRAME_SIZE || data.tv == 0) {
		THP_LOG_ERR("%s:input buf invalid\n", __func__);
		return -EINVAL;
	}

	if (data.size < cd->frame_size)
		cd->frame_size = data.size;

	thp_set_irq_status(cd, THP_IRQ_ENABLE);

	if (list_empty(&cd->frame_list.list) && !(f_flag & O_NONBLOCK)) {
		rc = thp_wait_frame_waitq(cd);
		if (rc)
			return rc;
	}

	mutex_lock(&cd->mutex_frame);

	if (!list_empty(&cd->frame_list.list)) {
		struct thp_frame *temp;

		temp = list_first_entry(&cd->frame_list.list,
				struct thp_frame, list);

		if (copy_to_user(data.buf, temp->frame, cd->frame_size)) {
			mutex_unlock(&cd->mutex_frame);
			THP_LOG_ERR("Failed to copy_to_user().\n");
			return -EFAULT;
		}

		if (copy_to_user(data.tv, &(temp->tv),
					sizeof(struct timeval))) {
			mutex_unlock(&cd->mutex_frame);
			THP_LOG_ERR("Failed to copy_to_user().\n");
			return -EFAULT;
		}

		list_del(&temp->list);
		kfree(temp);
		cd->frame_count--;
	} else {
		THP_LOG_ERR("%s:no frame\n", __func__);
		mutex_unlock(&cd->mutex_frame);
		return -ENODATA;
	}

	mutex_unlock(&cd->mutex_frame);

	return 0;
}

static long thp_ioctl_reset(unsigned long reset)
{
	struct thp_core_data *cd = thp_get_core_data();

	THP_LOG_INFO("%s:set reset status %d\n", __func__, reset);

	gpio_set_value(cd->rst_gpio, !!reset);

	cd->frame_waitq_flag = WAITQ_WAIT;
	cd->reset_flag = !!reset;

	return 0;
}

static long thp_ioctl_set_timeout(unsigned long arg)
{
	struct thp_core_data *ts = thp_get_core_data();
	unsigned int timeout_ms = arg;

	ts->timeout = timeout_ms;

	THP_LOG_DEBUG("%s:set timeout %d ms\n", __func__, timeout_ms);

	thp_wake_up_frame_waitq(ts);
	return 0;
}

static long thp_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	long ret;

	switch (cmd) {
	case THP_IOCTL_CMD_GET_FRAME:
		ret = thp_ioctl_get_frame(arg, filp->f_flags);
		break;

	case THP_IOCTL_CMD_RESET:
		ret = thp_ioctl_reset(arg);
		break;

	case THP_IOCTL_CMD_SET_TIMEOUT:
		ret = thp_ioctl_set_timeout(arg);
		break;

	case THP_IOCTL_CMD_SPI_SYNC:
		ret = thp_ioctl_spi_sync((void __user *)arg);
		break;

	case THP_IOCTL_CMD_FINISH_NOTIFY:
		ret = thp_ioctl_finish_notify(arg);
		break;

	default:
		THP_LOG_ERR("cmd unknown.\n");
		ret = 0;
	}

	return ret;
}

static const struct file_operations g_thp_fops = {
	.owner = THIS_MODULE,
	.open = thp_open,
	.release = thp_release,
	.unlocked_ioctl = thp_ioctl,
};

static struct miscdevice g_thp_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = THP_MISC_DEVICE_NAME,
	.fops = &g_thp_fops,
};

static void thp_copy_frame(struct thp_core_data *cd)
{
	struct thp_frame *temp;
	static int pre_frame_count = -1;

	mutex_lock(&(cd->mutex_frame));

	/* check for max limit */
	if (cd->frame_count >= THP_LIST_MAX_FRAMES) {
		if (cd->frame_count != pre_frame_count)
			THP_LOG_ERR("ts frame buferr full start\n");

		temp = list_first_entry(&cd->frame_list.list,
						struct thp_frame, list);
		list_del(&temp->list);
		kfree(temp);
		pre_frame_count = cd->frame_count;
		cd->frame_count--;
	} else if (pre_frame_count >= THP_LIST_MAX_FRAMES) {
		THP_LOG_ERR("%s:ts frame buf full exception restored\n",
					__func__);
		pre_frame_count = cd->frame_count;
	}

	temp = kzalloc(sizeof(struct thp_frame), GFP_KERNEL);
	if (!temp) {
		THP_LOG_ERR("%s:memory out\n", __func__);
		mutex_unlock(&(cd->mutex_frame));
		return;
	}

	memcpy(temp->frame, cd->frame_read_buf, cd->frame_size);
	do_gettimeofday(&(temp->tv));
	list_add_tail(&(temp->list), &(cd->frame_list.list));
	cd->frame_count++;
	mutex_unlock(&(cd->mutex_frame));
}

static irqreturn_t thp_irq_thread(int irq, void *dev_id)
{
	struct thp_core_data *cd = dev_id;
	u8 *read_buf = (u8 *)cd->frame_read_buf;
	int rc;

	if (cd->reset_flag)
		return IRQ_HANDLED;

	disable_irq_nosync(cd->irq);
	/* get frame */
	rc = cd->thp_dev->ops->get_frame(cd->thp_dev, read_buf, cd->frame_size);
	if (rc) {
		THP_LOG_ERR("%s: failed to read frame (%d)\n", __func__, rc);
		goto exit;
	}

	thp_copy_frame(cd);
	thp_wake_up_frame_waitq(cd);

exit:
	enable_irq(cd->irq);
	return IRQ_HANDLED;
}

static void thp_spi_cs_set(u32 control)
{
	int rc = 0;
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd) {
		THP_LOG_ERR("%s:no driver data", __func__);
		return;
	}

	if (control == SSP_CHIP_SELECT) {
		rc = gpio_direction_output(cd->cs_gpio, control);
		ndelay(cd->timing_config.spi_sync_cs_low_delay_ns);
	} else {
		rc = gpio_direction_output(cd->cs_gpio, control);
		ndelay(cd->timing_config.spi_sync_cs_hi_delay_ns);
	}

	if (rc < 0)
		THP_LOG_ERR("%s:fail to set gpio cs", __func__);

}

#if defined(THP_CHARGER_FB)
static int thp_charger_detect_notifier_callback(struct notifier_block *self,
					unsigned long event, void *data)
{
	struct thp_core_data *cd = thp_get_core_data();

	THP_LOG_INFO("%s called, charger type: %d\n", __func__, event);
	if (event == CHARGER_TYPE_NONE)
		cd->charger_state = 0;
	else
		cd->charger_state = 1;

	return 0;
}
#endif

#define THP_PROJECTID_LEN 9
#define THP_PROJECTID_PRODUCT_NAME_LEN 4
#define THP_PROJECTID_IC_NAME_LEN 2
#define THP_PROJECTID_VENDOR_NAME_LEN 3

struct thp_vendor {
	char *vendor_id;
	char *vendor_name;
};

static struct thp_vendor thp_vendor_table[] = {
	{"080", "jdi"},
	{"100", "lg"},
	{"160", "sharp"},
};
static int thp_projectid_to_vender_name(char *project_id,
				char **vendor_name)
{
	char temp_buf[THP_PROJECTID_LEN + 1] = {'0'};
	int i;

	strncpy(temp_buf, project_id + THP_PROJECTID_PRODUCT_NAME_LEN +
		THP_PROJECTID_IC_NAME_LEN, THP_PROJECTID_VENDOR_NAME_LEN);

	for (i = 0; i < ARRAY_SIZE(thp_vendor_table); i++) {
		if (!strncmp(thp_vendor_table[i].vendor_id, temp_buf,
			strlen(thp_vendor_table[i].vendor_id))) {
			*vendor_name = thp_vendor_table[i].vendor_name;
			return 0;
		}
	}

	return -ENODATA;
}

static int thp_init_chip_info(struct thp_core_data *cd)
{
	int rc = hostprocessing_get_project_id(cd->project_id);

	if (rc)
		THP_LOG_ERR("%s:get project id form LCD fail\n", __func__);
	else
		THP_LOG_INFO("%s:project id :%s\n", __func__, cd->project_id);

	cd->project_id[THP_PROJECT_ID_LEN] = '\0';

	rc = thp_projectid_to_vender_name(cd->project_id, &cd->vendor_name);
	if (rc)
		THP_LOG_INFO("%s:vendor name parse fail\n", __func__);

	cd->ic_name = cd->thp_dev->ic_name;

	return rc;
}

static int thp_setup_irq(struct thp_core_data *cd)
{
	int rc;
	int irq = gpio_to_irq(cd->irq_gpio);

	rc = request_threaded_irq(irq, NULL,
			thp_irq_thread,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT | cd->irq_flag,
			THP_DEVICE_NAME, cd);

	if (rc) {
		THP_LOG_ERR("%s: request irq fail\n", __func__);
		return rc;
	}
	mutex_lock(&cd->irq_mutex);
	disable_irq(irq);
	cd->irq_enabled = false;
	mutex_unlock(&cd->irq_mutex);
	THP_LOG_INFO("%s: disable irq\n", __func__);
	cd->irq = irq;

	return 0;
}

static int thp_setup_gpio(struct thp_core_data *cd)
{
	int rc;

	THP_LOG_ERR("%s: called\n", __func__);

	rc = gpio_request(cd->rst_gpio, "thp_reset");
	if (rc) {
		THP_LOG_ERR("%s:gpio_request(%d) failed\n", __func__,
				cd->rst_gpio);
		return rc;
	}

	rc = gpio_request(cd->cs_gpio, "thp_cs");
	if (rc) {
		THP_LOG_ERR("%s:gpio_request(%d) failed\n", __func__,
				cd->cs_gpio);
		gpio_free(cd->rst_gpio);
		return rc;
	}

	rc = gpio_request(cd->irq_gpio, "thp_int");
	if (rc) {
		THP_LOG_ERR("%s: irq gpio(%d) request failed\n", __func__,
				cd->irq_gpio);
		gpio_free(cd->rst_gpio);
		gpio_free(cd->cs_gpio);
		return rc;
	}

	gpio_direction_input(cd->irq_gpio);

	gpio_direction_output(cd->rst_gpio, GPIO_LOW);
	gpio_set_value(cd->rst_gpio, THP_RESET_LOW);
	thp_do_time_delay(cd->timing_config.boot_reset_low_delay_ms);

	gpio_set_value(cd->rst_gpio, THP_RESET_HIGH);
	thp_do_time_delay(cd->timing_config.boot_reset_hi_delay_ms);

	return 0;
}

static void thp_free_gpio(struct thp_core_data *ts)
{
	gpio_free(ts->irq_gpio);
	gpio_free(ts->cs_gpio);
	gpio_free(ts->rst_gpio);
}

static int thp_setup_spi(struct thp_core_data *cd)
{
	int rc;

	cd->spi_config.pl022_spi_config.cs_control = thp_spi_cs_set;
	cd->spi_config.pl022_spi_config.hierarchy = SSP_MASTER;

	if (!cd->spi_config.max_speed_hz)
		cd->spi_config.max_speed_hz = THP_SPI_SPEED_DEFAULT;
	if (!cd->spi_config.mode)
		cd->spi_config.mode = SPI_MODE_0;
	if (!cd->spi_config.bits_per_word)
		cd->spi_config.bits_per_word = 8;

	cd->sdev->mode = cd->spi_config.mode;
	cd->sdev->max_speed_hz = cd->spi_config.max_speed_hz;
	cd->sdev->bits_per_word = cd->spi_config.bits_per_word;
	cd->sdev->controller_data = &cd->spi_config.pl022_spi_config;

	rc = spi_setup(cd->sdev);
	if (rc) {
		THP_LOG_ERR("%s: spi setup fail\n", __func__);
		return rc;
	}

	return 0;

}

static int thp_core_init(struct thp_core_data *cd)
{
	int rc;

	/*step 1 : init mutex */
	mutex_init(&cd->mutex_frame);
	mutex_init(&cd->irq_mutex);
	mutex_init(&cd->thp_mutex);

	dev_set_drvdata(&cd->sdev->dev, cd);

#if defined(CONFIG_HUAWEI_DSM)
	dsm_thp_dclient = dsm_register_client(&dsm_thp);
#endif

	rc = thp_init_chip_info(cd);
	if (rc)
		THP_LOG_ERR("%s: chip info init fail\n", __func__);

	cd->fb_notify.notifier_call = thp_fb_notifier_callback;
	rc = fb_register_client(&cd->fb_notify);
	if (rc)	{
		THP_LOG_ERR("%s: failed to register fb_notifier: %d\n", rc);
		goto err_register_fb_notify;
	}

	rc = misc_register(&g_thp_misc_device);
	if (rc)	{
		THP_LOG_ERR("%s: failed to register misc device\n", __func__);
		goto err_register_misc;
	}

	rc = thp_mt_wrapper_init();
	if (rc) {
		THP_LOG_ERR("%s: failed to init input_mt_wrapper\n", __func__);
		goto err_init_wrapper;
	}

	rc = thp_init_sysfs(cd);
	if (rc) {
		THP_LOG_ERR("%s: failed to create sysfs\n", __func__);
		goto err_init_sysfs;
	}

	rc = thp_setup_irq(cd);
	if (rc) {
		THP_LOG_ERR("%s: failed to set up irq\n", __func__);
		goto err_register_misc;
	}

#if defined(THP_CHARGER_FB)
	cd->charger_detect_notify.notifier_call =
			thp_charger_detect_notifier_callback;
	rc = hisi_charger_type_notifier_register(&cd->charger_detect_notify);
	if (rc < 0) {
		THP_LOG_ERR("%s:charger notifier register failed\n", __func__);
		cd->charger_detect_notify.notifier_call = NULL;
	} else {
		THP_LOG_INFO("%s:charger notifier register succ\n", __func__);
		thp_charger_detect_notifier_callback(NULL,
					hisi_get_charger_type(), NULL);
	}
#endif

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_TOUCH_PANEL);
#endif

	atomic_set(&cd->register_flag, 1);
	return 0;

err_setip_irq:
	thp_sysfs_release(cd);
err_init_sysfs:
	thp_mt_wrapper_exit();
err_init_wrapper:
	misc_deregister(&g_thp_misc_device);
err_register_misc:
	fb_unregister_client(&cd->fb_notify);
err_register_fb_notify:
	mutex_destroy(&cd->mutex_frame);
	mutex_destroy(&cd->irq_mutex);
	mutex_destroy(&cd->thp_mutex);
	return rc;
}

int thp_register_dev(struct thp_device *dev)
{
	int rc;
	struct thp_core_data *cd = thp_get_core_data();

	THP_LOG_INFO("%s: called\n", __func__);
	if (!dev) {
		THP_LOG_ERR("%s: input dev null\n", __func__);
		return -EINVAL;
	}

	if (!cd) {
		THP_LOG_ERR("%s: core not inited\n", __func__);
		return -EINVAL;
	}

	if (atomic_read(&cd->register_flag)) {
		THP_LOG_ERR("%s: thp have registerd");
		return -ENODEV;
	}

	dev->thp_core = cd;
	cd->thp_dev = dev;

	rc = dev->ops->init(dev);
	if (rc) {
		THP_LOG_ERR("%s: dev init fail\n", __func__);
		goto dev_init_err;
	}

	rc = thp_setup_gpio(cd);
	if (rc) {
		THP_LOG_ERR("%s: spi dev init fail\n", __func__);
		goto dev_init_err;
	}

	rc = thp_setup_spi(cd);
	if (rc) {
		THP_LOG_ERR("%s: spi dev init fail\n", __func__);
		goto err;
	}

	rc = dev->ops->detect(dev);
	if (rc) {
		THP_LOG_ERR("%s: chip detect fail\n", __func__);
		goto err;
	}

	rc = thp_core_init(cd);
	if (rc) {
		THP_LOG_ERR("%s: core init\n", __func__);
		goto err;
	}

	return 0;
err:
	thp_free_gpio(cd);
dev_init_err:
	cd->thp_dev = 0;
	return rc;
}

int thp_parse_spi_config(struct device_node *spi_cfg_node,
			struct thp_core_data *cd)
{
	int rc;
	unsigned int value;
	struct thp_spi_config *spi_config;
	struct pl022_config_chip *pl022_spi_config;

	if (!spi_cfg_node || !cd) {
		THP_LOG_INFO("%s: input null\n", __func__);
		return -ENODEV;
	}

	spi_config = &cd->spi_config;
	pl022_spi_config = &cd->spi_config.pl022_spi_config;

	rc = of_property_read_u32(spi_cfg_node, "spi_max_frequency", &value);
	if (!rc) {
		spi_config->max_speed_hz = value;
		THP_LOG_INFO("%s:spi-max-frequency configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(spi_cfg_node, "spi_mode", &value);
	if (!rc) {
		spi_config->mode = value;
		THP_LOG_INFO("%s:spi-mode configed %d\n", __func__, value);
	}

	rc = of_property_read_u32(spi_cfg_node, "bits_per_word", &value);
	if (!rc) {
		spi_config->bits_per_word = value;
		THP_LOG_INFO("%s:spi-mode configed %d\n", __func__, value);
	}

	rc = of_property_read_u32(spi_cfg_node, "pl022,interface", &value);
	if (!rc) {
		pl022_spi_config->iface = value;
		THP_LOG_INFO("%s: pl022,interface parsed\n", __func__);
	}

	rc = of_property_read_u32(spi_cfg_node, "pl022,com-mode", &value);
	if (!rc) {
		pl022_spi_config->com_mode = value;
		THP_LOG_INFO("%s:com_mode parsed\n", __func__);
	}

	rc = of_property_read_u32(spi_cfg_node, "pl022,rx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->rx_lev_trig = value;
		THP_LOG_INFO("%s:rx-level-trig parsed\n", __func__);
	}

	rc = of_property_read_u32(spi_cfg_node, "pl022,tx-level-trig", &value);
	if (!rc) {
		pl022_spi_config->tx_lev_trig = value;
		THP_LOG_INFO("%s:tx-level-trig parsed\n", __func__);
	}

	rc = of_property_read_u32(spi_cfg_node, "pl022,ctrl-len", &value);
	if (!rc) {
		pl022_spi_config->ctrl_len = value;
		THP_LOG_INFO("%s:ctrl-len parsed\n", __func__);
	}

	rc = of_property_read_u32(spi_cfg_node, "pl022,wait-state", &value);
	if (!rc) {
		pl022_spi_config->wait_state = value;
		THP_LOG_INFO("%s:wait-state parsed\n", __func__);
	}

	rc = of_property_read_u32(spi_cfg_node, "pl022,duplex", &value);
	if (!rc) {
		pl022_spi_config->duplex = value;
		THP_LOG_INFO("%s:duplex parsed\n", __func__);
	}

	return 0;
}

int thp_parse_timing_config(struct device_node *timing_cfg_node,
			struct thp_core_data *cd)
{
	int rc;
	unsigned int value;
	struct thp_timing_config *timing = &cd->timing_config;

	if (!timing_cfg_node || !cd) {
		THP_LOG_INFO("%s: input null\n", __func__);
		return -ENODEV;
	}

	rc = of_property_read_u32(timing_cfg_node,
					"boot_reset_hi_delay_ms", &value);
	if (!rc) {
		timing->boot_reset_hi_delay_ms = value;
		THP_LOG_INFO("%s:boot_reset_hi_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"boot_reset_low_delay_ms", &value);
	if (!rc) {
		timing->boot_reset_low_delay_ms = value;
		THP_LOG_INFO("%s:boot_reset_low_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"boot_reset_after_delay_ms", &value);
	if (!rc) {
		timing->boot_reset_after_delay_ms = value;
		THP_LOG_INFO("%s:boot_reset_after_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"resume_reset_after_delay_ms", &value);
	if (!rc) {
		timing->resume_reset_after_delay_ms = value;
		THP_LOG_INFO("%s:resume_reset_after_delay_ms configed %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"suspend_reset_after_delay_ms", &value);
	if (!rc) {
		timing->suspend_reset_after_delay_ms = value;
		THP_LOG_INFO("%s:suspend_reset_after_delay configed_ms %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"spi_sync_cs_hi_delay_ns", &value);
	if (!rc) {
		timing->spi_sync_cs_hi_delay_ns = value;
		THP_LOG_INFO("%s:spi_sync_cs_hi_delay_ns configed_ms %d\n",
				__func__, value);
	}

	rc = of_property_read_u32(timing_cfg_node,
					"spi_sync_cs_low_delay_ns", &value);
	if (!rc) {
		timing->spi_sync_cs_low_delay_ns = value;
		THP_LOG_INFO("%s:spi_sync_cs_low_delay_ns configed_ms %d\n",
				__func__, value);
	}

	return 0;
}

static int thp_parse_config(struct thp_core_data *cd,
					struct device_node *thp_node)
{
	int rc;
	unsigned int value;
	struct device_node *spi_cfg_node;

	if (!thp_node) {
		THP_LOG_ERR("%s:thp not config in dts, exit\n", __func__);
		return -ENODEV;
	}

	rc = thp_parse_spi_config(thp_node, cd);
	if (rc) {
		THP_LOG_ERR("%s: spi config parse fail\n", __func__);
		return rc;
	}

	rc = thp_parse_timing_config(thp_node, cd);
	if (rc) {
		THP_LOG_ERR("%s: timing config parse fail\n", __func__);
		return rc;
	}

	rc = of_property_read_u32(thp_node, "irq_flag", &value);
	if (!rc) {
		cd->irq_flag = value;
		THP_LOG_INFO("%s:irq_flag parsed\n", __func__);
	}

	value = of_get_named_gpio(thp_node, "irq_gpio", 0);
	THP_LOG_INFO("irq gpio_ = %d\n", value);
	if (!gpio_is_valid(value)) {
		THP_LOG_ERR("%s: get irq_gpio failed\n", __func__);
		return rc;
	}
	cd->irq_gpio = value;

	value = of_get_named_gpio(thp_node, "rst_gpio", 0);
	THP_LOG_ERR("rst_gpio = %d\n", value);
	if (!gpio_is_valid(value)) {
		THP_LOG_ERR("%s: get rst_gpio failed\n", __func__);
		return rc;
	}
	cd->rst_gpio = value;

	value = of_get_named_gpio(thp_node, "cs_gpio", 0);
	THP_LOG_ERR("cs_gpio = %d\n", value);
	if (!gpio_is_valid(cd->cs_gpio)) {
		THP_LOG_ERR("%s: get cs_gpio failed\n", __func__);
		return rc;
	}
	cd->cs_gpio = value;

	cd->thp_node = thp_node;

	return 0;
}

static int thp_probe(struct spi_device *sdev)
{
	struct thp_core_data *thp_core;
	int rc;

	THP_LOG_INFO("%s: in\n", __func__);

	thp_core = kzalloc(sizeof(struct thp_core_data), GFP_KERNEL);
	if (!thp_core) {
		THP_LOG_ERR("%s: out of memory\n", __func__);
		return -ENOMEM;
	}

	rc = thp_parse_config(thp_core, sdev->dev.of_node);
	if (rc) {
		THP_LOG_ERR("%s: parse dts fail\n", __func__);
		kfree(thp_core);
		return -ENODEV;
	}

	atomic_set(&thp_core->register_flag, 0);
	INIT_LIST_HEAD(&thp_core->frame_list.list);
	init_waitqueue_head(&(thp_core->frame_waitq));

	g_thp_core = thp_core;
	g_thp_core->sdev = sdev;

	return 0;
}

static int thp_remove(struct spi_device *sdev)
{
	struct thp_core_data *cd = spi_get_drvdata(sdev);

	THP_LOG_INFO("%s: in\n", __func__);

	if (atomic_read(&cd->register_flag)) {
		thp_sysfs_release(cd);

#if defined(THP_CHARGER_FB)
		if (cd->charger_detect_notify.notifier_call)
			hisi_charger_type_notifier_unregister(
					&cd->charger_detect_notify);
#endif
		fb_unregister_client(&cd->fb_notify);
		misc_deregister(&g_thp_misc_device);
		mutex_destroy(&cd->mutex_frame);
		thp_mt_wrapper_exit();
	}
	kfree(cd);

	return 0;
}


static const struct of_device_id g_thp_psoc_match_table[] = {
	{.compatible = "huawei,thp",},
	{ },
};


static const struct spi_device_id g_thp_device_id[] = {
	{ THP_DEVICE_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(spi, g_thp_device_id);

static struct spi_driver g_thp_spi_driver = {
	.probe = thp_probe,
	.remove = thp_remove,
	.id_table = g_thp_device_id,
	.driver = {
		.name = THP_DEVICE_NAME,
		.owner = THIS_MODULE,
		.bus = &spi_bus_type,
		.of_match_table = g_thp_psoc_match_table,
	},
};

module_spi_driver(g_thp_spi_driver);

MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei THP Driver");
MODULE_LICENSE("GPL");

