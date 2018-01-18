/*
 * hi6xxx_fpga_test.c  --  fpga test for hi6xxx
 *
 * Copyright (c) 2014 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "hi6555c_stub.h"
#include "hi6555c_debug.h"
#include "hi6xxx_fpga_test.h"

#define HI6XXX_FPGA_TEST_DIR      "hi6xxx_fpga"
#define HI6XXX_FPGA_TEST_FD       "hi6xxx_fpga_test"

struct reg_attr {
	char rw[1];
	unsigned int addr;
	unsigned int value;
};

struct hi6xxx_fpga_tester
{
	struct proc_dir_entry *fs_dir;
};

static struct hi6xxx_fpga_tester *fpga_tester = NULL;

static ssize_t do_test_write(struct file *file, const char __user *user_buf,
			size_t count, loff_t *ppos)
{
	struct reg_attr kern_buf;

	if (NULL == user_buf) {
		loge("input error: user buf is NULL\n");
		return -EFAULT;
	}

	if (count != sizeof(kern_buf)) {
		loge("count:%zu from user space exceed " \
			"sizeof(kern_buf):%zu\n", count, sizeof(kern_buf));
		return -EFAULT;
	}

	if (copy_from_user(&kern_buf, user_buf, count)) {
		loge("copy_from_user fail.\n");
		return -EFAULT;
	}

	/* reg addr will check in codec_reg_write/read */
	if (kern_buf.rw[0] == 'w') {
		codec_reg_write(kern_buf.addr, kern_buf.value);
	} else {
		loge("invalid input\n");
	}

	return sizeof(kern_buf);
}

static const struct file_operations do_test_fops = {
	.write = do_test_write,
};

void hi6xxx_fpga_test_init(void)
{
	fpga_tester = kzalloc(sizeof(*fpga_tester), GFP_KERNEL);
	if(!fpga_tester){
		loge("fpga tester malloc failed\n");
		return;
	}

	fpga_tester->fs_dir = proc_mkdir(HI6XXX_FPGA_TEST_DIR, NULL);
	if (!fpga_tester->fs_dir) {
		loge("create proc/hi6xxx_fpga dir failed\n");
		kfree(fpga_tester);
		return;
	}

	if (!proc_create(HI6XXX_FPGA_TEST_FD, S_IRUGO|S_IWUSR|S_IWGRP,
			fpga_tester->fs_dir, &do_test_fops)) {
		loge("failed to create hi6xxx_fpga_test node\n");
		remove_proc_entry(HI6XXX_FPGA_TEST_DIR, 0);
		kfree(fpga_tester);
		return;
	}

	logi("audio fpga test init success.\n");

	return;
}

void hi6xxx_fpga_test_deinit(void)
{
	if (fpga_tester) {
		remove_proc_entry(HI6XXX_FPGA_TEST_FD, fpga_tester->fs_dir);
		kfree(fpga_tester);
	}

	return;
}

