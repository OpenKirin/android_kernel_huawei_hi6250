/*
 * Huawei Touchscreen Driver
 *
 * Copyright (C) 2017 Huawei Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/byteorder.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include "huawei_thp.h"
#include "huawei_thp_attr.h"

#define SYSFS_PROPERTY_PATH	  "afe_properties"
#define SYSFS_TOUCH_PATH   "touchscreen"
#define SYSFS_PLAT_TOUCH_PATH	"huawei_touch"

u8 g_thp_log_cfg = 0;

static ssize_t thp_wake_up_enable_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	return count;
}

static ssize_t thp_wake_up_enable_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t thp_hostprocessing_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "hostprocessing\n");
}

static ssize_t thp_status_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	return snprintf(buf, PAGE_SIZE, "power : %d\ntui : %d\n",
			!cd->suspended, cd->tui_flag);
}

/*
 * If not config ic_name in dts, it will be "unknown"
 */
static ssize_t thp_chip_info_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	return snprintf(buf, PAGE_SIZE, "%s-%s-%s\n", cd->ic_name,
					cd->project_id, cd->vendor_name);
}

static ssize_t thp_loglevel_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	u8 new_level = g_thp_log_cfg ? 0 : 1;

	int len = snprintf(buf, PAGE_SIZE, "%d -> %d\n",
				g_thp_log_cfg, new_level);

	g_thp_log_cfg = new_level;

	return len;
}

#if defined(HOST_CHARGER_FB)
static ssize_t thp_host_charger_state_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct thp_core_data *ts = thp_get_core_data();

	THP_LOG_DEBUG("%s called\n", __func__);

	return snprintf(buf, 32, "%d\n", ts->charger_state);
}
static ssize_t thp_host_charger_state_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	struct thp_core_data *ts = thp_get_core_data();

	/*
	 * get value of charger status from first byte of buf
	 */
	unsigned int value = buf[0] - '0';

	THP_LOG_INFO("%s: input value is %d\n", __func__, value);

	ts->charger_state = value;

	return count;
}
#endif

struct device_attribute attr_thp_status = {
	.attr = {.name = "thp_status",
		 .mode = S_IRUGO },
	.show	= thp_status_show,
	.store	= NULL,
};

struct device_attribute attr_touch_chip_info = {
	.attr = {.name = "touch_chip_info",
		 .mode = S_IRUGO },
	.show	= thp_chip_info_show,
	.store	= NULL,
};
struct device_attribute attr_hostprocessing = {
	.attr = {.name = "hostprocessing",
		 .mode = S_IRUGO },
	.show	= thp_hostprocessing_show,
	.store	= NULL,
};
struct device_attribute attr_loglevel = {
	.attr = {.name = "loglevel",
		 .mode = S_IRUGO },
	.show	= thp_loglevel_show,
	.store	= NULL,
};

#if defined(HOST_CHARGER_FB)
struct device_attribute attr_charger_state = {
	.attr = {.name = "charger_state",
		 .mode = (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP) },
	.show	= thp_host_charger_state_show,
	.store	= thp_host_charger_state_store,
};
#endif

static struct attribute *thp_ts_attributes[] = {
	&attr_thp_status.attr,
	&attr_touch_chip_info.attr,
	&attr_hostprocessing.attr,
	&attr_loglevel.attr,
#if defined(HOST_CHARGER_FB)
	&attr_charger_state.attr,
#endif
	NULL,
};

static const struct attribute_group thp_ts_attr_group = {
	.attrs = thp_ts_attributes,
};

struct device_attribute attr_wake_up_enable = {
	.attr = {.name = "wake_up_enable",
		 .mode = (S_IRUGO | S_IWUSR | S_IWGRP) },
	.show	= thp_wake_up_enable_show,
	.store	= thp_wake_up_enable_store,
};

static struct attribute *thp_prop_attrs[] = {
	&attr_wake_up_enable.attr,
	NULL
};

static const struct attribute_group thp_ts_prop_attr_group = {
	.attrs = thp_prop_attrs,
};

int thp_init_sysfs(struct thp_core_data *cd)
{
	int rc;

	if (!cd) {
		THP_LOG_ERR("%s: core data null\n", __func__);
		return -EINVAL;
	}

	rc = sysfs_create_group(&cd->sdev->dev.kobj, &thp_ts_attr_group);
	if (rc) {
		THP_LOG_ERR("%s:can't create ts's sysfs\n", __func__);
		return rc;
	}

	rc = sysfs_create_link(NULL, &cd->sdev->dev.kobj, SYSFS_TOUCH_PATH);
	if (rc) {
		THP_LOG_ERR("%s: fail create link error = %d\n", __func__, rc);
		goto err_create_ts_sysfs;
	}

	/* add sys/afe_properties/ sysfs */
	cd->thp_obj = kobject_create_and_add(SYSFS_PROPERTY_PATH, NULL);
	if (!cd->thp_obj) {
		THP_LOG_ERR("%s:unable to create kobject\n", __func__);
		rc = -EINVAL;
		goto err_create_ts_sysfs;
	}
	rc = sysfs_create_group(cd->thp_obj, &thp_ts_prop_attr_group);
	if (rc) {
		THP_LOG_ERR("%s:failed to create attributes\n", __func__);
		goto err_create_ts_sysfs;
	}

	return 0;

err_create_ts_sysfs:
	sysfs_remove_group(&cd->sdev->dev.kobj, &thp_ts_attr_group);
	return rc;
}


void thp_sysfs_release(struct thp_core_data *cd)
{
	if (!cd) {
		THP_LOG_ERR("%s: core data null\n", __func__);
		return;
	}

	sysfs_remove_group(cd->thp_obj, &thp_ts_prop_attr_group);
	sysfs_remove_group(&cd->sdev->dev.kobj, &thp_ts_attr_group);
}


