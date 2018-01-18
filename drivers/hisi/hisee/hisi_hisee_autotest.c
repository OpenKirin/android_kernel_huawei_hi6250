#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/fd.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/of_platform.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/hisi/kirin_partition.h>
#include <linux/clk.h>
#include <linux/mm.h>
#include "hisi_hisee.h"
#include "hisi_hisee_fs.h"
#include "hisi_hisee_autotest.h"

/* part 1: channel test function */
#ifdef CONFIG_HISI_DEBUG_FS
/**
 *notes: echo command should add "new line" character(0xa) to the end of string.
 *so path should discard this character.
 */
static int hisee_test(char *path, phys_addr_t result_phy, size_t result_size)
{
	char *buff_virt;
	phys_addr_t buff_phy = 0;
	char fullname[MAX_PATH_NAME_LEN + 1] = { 0 };
	int fd;
	int i = 0;
	mm_segment_t fs;
	atf_message_header *p_message_header;
	int ret;
	int image_size;

	do {
		if (0xa == path[i] || 0x20 == path[i]) {
			break;
		}
		fullname[i] = path[i];
		i++;
	} while (i < MAX_PATH_NAME_LEN);
	if (i <= 0) {
		pr_err("%s() filename is invalid\n", __func__);
		set_errno_and_return(HISEE_CHANNEL_TEST_PATH_ABSENT_ERROR);
	}

	fs = get_fs();
	set_fs(KERNEL_DS);/*lint !e501*/
	fd = (int)sys_open(fullname, O_RDONLY, HISEE_FILESYS_DEFAULT_MODE);
	if (fd < 0) {
		pr_err("%s(): open %s failed, fd=%d\n", __func__, fullname, fd);
		set_fs(fs);
		set_errno_and_return(HISEE_CHANNEL_TEST_PATH_ABSENT_ERROR);
	}
	image_size = (int)sys_lseek((unsigned int)fd, (long)0, SEEK_END);
	if (image_size < 0) {
		pr_err("%s(): sys_lseek failed from set.\n", __func__);
		sys_close((unsigned int)fd);
		set_fs(fs);
		set_errno_and_return(HISEE_LSEEK_FILE_ERROR);
	}
	image_size += HISEE_ATF_MESSAGE_HEADER_LEN;
	pr_err("%s() file size is 0x%x\n", __func__, image_size);
	sys_close((unsigned int)fd);

	sys_unlink(TEST_SUCCESS_FILE);
	sys_unlink(TEST_FAIL_FILE);
	sys_unlink(TEST_RESULT_FILE);
	set_fs(fs);

	buff_virt = (void *)dma_alloc_coherent(g_hisee_data.cma_device, (unsigned long)ALIGN_UP_4KB(image_size),
											&buff_phy, GFP_KERNEL);
	if (buff_virt == NULL) {
		pr_err("%s(): dma_alloc_coherent failed\n", __func__);
		set_errno_and_return(HISEE_NO_RESOURCES);
	}
	memset(buff_virt, 0, (unsigned long)ALIGN_UP_4KB(image_size));
	p_message_header = (atf_message_header *)buff_virt;/*lint !e826*/
	set_message_header(p_message_header, CMD_HISEE_CHANNEL_TEST);
	p_message_header->test_result_phy = (unsigned int)result_phy;
	p_message_header->test_result_size = (unsigned int)result_size;

	ret = hisee_read_file(fullname, buff_virt + HISEE_ATF_MESSAGE_HEADER_LEN, 0UL, (size_t)(image_size - HISEE_ATF_MESSAGE_HEADER_LEN));/*lint !e571*/
	if (ret < HISEE_OK) {
		pr_err("%s(): hisee_read_file failed, ret=%d\n", __func__, ret);
		dma_free_coherent(g_hisee_data.cma_device, (unsigned long)ALIGN_UP_4KB(image_size), buff_virt, buff_phy);
		set_errno_and_return(ret);
	}

	ret = send_smc_process(p_message_header, buff_phy, (unsigned int)image_size,
							HISEE_ATF_GENERAL_TIMEOUT, CMD_HISEE_CHANNEL_TEST);

	fs = get_fs();
	set_fs(KERNEL_DS);/*lint !e501*/
	fd = (int)sys_mkdir(TEST_DIRECTORY_PATH, HISEE_FILESYS_DEFAULT_MODE);
	if (fd < 0 && (-EEXIST != fd)) {/*EEXIST(File exists), don't return error*/
		set_fs(fs);
		dma_free_coherent(g_hisee_data.cma_device, (unsigned long)ALIGN_UP_4KB(image_size), buff_virt, buff_phy);
		pr_err("create dir %s fail, ret: %d.\n", TEST_DIRECTORY_PATH, fd);
		return fd;
	}
	if (HISEE_OK == ret) {
		/* create file for test flag */
		pr_err("%s(): rcv result size is 0x%x\r\n", __func__, p_message_header->test_result_size);
		if ((g_hisee_data.channel_test_item_result.phy == p_message_header->test_result_phy) &&
			(g_hisee_data.channel_test_item_result.size >= (long)p_message_header->test_result_size)) {
			fd = (int)sys_open(TEST_RESULT_FILE, O_RDWR|O_CREAT, 0);
			if (fd < 0) {
				pr_err("sys_open %s fail, fd: %d.\n", TEST_RESULT_FILE, fd);
				ret = fd;
				goto error;
			}
			sys_write((unsigned int)fd, g_hisee_data.channel_test_item_result.buffer, (unsigned long)p_message_header->test_result_size);
			sys_close((unsigned int)fd);
			fd = (int)sys_open(TEST_SUCCESS_FILE, O_RDWR|O_CREAT, 0);
			if (fd < 0) {
				pr_err("sys_open %s fail, fd: %d.\n", TEST_SUCCESS_FILE, fd);
				ret = fd;
				goto error;
			}
			sys_close((unsigned int)fd);
			ret = HISEE_OK;
		} else {
			fd = (int)sys_open(TEST_FAIL_FILE, O_RDWR|O_CREAT, 0);
			if (fd < 0) {
				pr_err("sys_open %s fail, fd: %d.\n", TEST_FAIL_FILE, fd);
				ret = fd;
				goto error;
			}
			sys_close((unsigned int)fd);
			ret = HISEE_CHANNEL_TEST_WRITE_RESULT_ERROR;
		}
	} else {
		fd = (int)sys_open(TEST_FAIL_FILE, O_RDWR|O_CREAT, 0);
		if (fd < 0) {
			pr_err("sys_open %s fail, fd: %d.\n", TEST_FAIL_FILE, fd);
			ret = fd;
			goto error;
		}
		sys_close((unsigned int)fd);
		ret = HISEE_CHANNEL_TEST_WRITE_RESULT_ERROR;
	}

error:
	set_fs(fs);
	dma_free_coherent(g_hisee_data.cma_device, (unsigned long)ALIGN_UP_4KB(image_size), buff_virt, buff_phy);
	set_errno_and_return(ret);
}

static int channel_test_check_buffer_size(char *buff, int offset)
{
	int i, j, k, value;

	if (0 == strncmp(buff + offset, "result_size:0x", sizeof("result_size:0x") - 1)) {
		offset += (int)sizeof("result_size:0x") - 1;
		/* find last size char */
		i = 0;
		while (0x20 != buff[offset + i]) {/*lint !e679*/
			i++;
		}

		if (0 == i) {
			pr_err("result size is bad, use default size.\n");
			k = 0;
			g_hisee_data.channel_test_item_result.size = CHANNEL_TEST_RESULT_SIZE_DEFAULT;
		} else {
			g_hisee_data.channel_test_item_result.size = 0;
			k = i;
			i--;
			j = 0;
			while (i >= 0) {
				if ((buff[offset + i] >= '0') && (buff[offset + i] <= '9')) {/*lint !e679*/
					value = buff[offset + i] - 0x30;/*lint !e679*/
				} else if ((buff[offset + i] >= 'a') && (buff[offset + i] <= 'f')) {/*lint !e679*/
					value = (buff[offset + i] - 'a') + 0x10;/*lint !e679*/
				} else if ((buff[offset + i] >= 'A') && (buff[offset + i] <= 'F')) {/*lint !e679*/
					value = (buff[offset + i] - 'A') + 0x10;/*lint !e679*/
				} else {
					pr_err("result size is bad, use default size.\n");
					g_hisee_data.channel_test_item_result.size = TEST_RESULT_SIZE_DEFAULT;
					break;
				}
				g_hisee_data.channel_test_item_result.size += (value << (unsigned int)j);/*lint !e701*/
				i--;
				j += 4;
			}
		}
		offset += k;
	} else {
		g_hisee_data.channel_test_item_result.size = TEST_RESULT_SIZE_DEFAULT;
	}
	return offset;
}
#endif /*CONFIG_HISI_DEBUG_FS; hisee_channel_test_func inner functions*/

int hisee_channel_test_func(void *buf, int para)
{
#ifdef CONFIG_HISI_DEBUG_FS
	char *buff = buf;
	int ret;
	int offset = 0;

	if (NULL == buf) {
		pr_err("%s(): input buf is NULL.\n", __func__);
		set_errno_and_return(HISEE_NO_RESOURCES);
	}
	bypass_space_char();

	offset = channel_test_check_buffer_size(buff, offset);

	pr_err("result size is 0x%x.\n", g_hisee_data.channel_test_item_result.size);
	if (0 == g_hisee_data.channel_test_item_result.size) {
		pr_err("result size is bad.\r\n");
		set_errno_and_return(HISEE_CHANNEL_TEST_CMD_ERROR);
	}

	bypass_space_char();

	if (0 == buff[offset]) {
		pr_err("test file path is bad.\n");
		set_errno_and_return(HISEE_CHANNEL_TEST_CMD_ERROR);
	}

	if (NULL != g_hisee_data.channel_test_item_result.buffer) {
		dma_free_coherent(g_hisee_data.cma_device,
					(unsigned long)ALIGN_UP_4KB(g_hisee_data.channel_test_item_result.size),
					g_hisee_data.channel_test_item_result.buffer,
					g_hisee_data.channel_test_item_result.phy);
	}

	g_hisee_data.channel_test_item_result.buffer = (char *)dma_alloc_coherent(g_hisee_data.cma_device,
													(unsigned long)ALIGN_UP_4KB(g_hisee_data.channel_test_item_result.size),
													(dma_addr_t *)&g_hisee_data.channel_test_item_result.phy,
													GFP_KERNEL);
	if (NULL == g_hisee_data.channel_test_item_result.buffer) {
		pr_err("%s(): alloc 0x%x fail.\r\n", __func__, ALIGN_UP_4KB(g_hisee_data.channel_test_item_result.size));
		set_errno_and_return(HISEE_CHANNEL_TEST_RESULT_MALLOC_ERROR);
	}


    ret = hisee_test(buff + offset, g_hisee_data.channel_test_item_result.phy, (size_t)g_hisee_data.channel_test_item_result.size);
	if (HISEE_OK != ret) {
		pr_err("%s(): hisee_test fail, ret = %d\n", __func__, ret);
	}
	dma_free_coherent(g_hisee_data.cma_device,
				(unsigned long)ALIGN_UP_4KB(g_hisee_data.channel_test_item_result.size),
				g_hisee_data.channel_test_item_result.buffer,
				g_hisee_data.channel_test_item_result.phy);
	g_hisee_data.channel_test_item_result.buffer = NULL;
	g_hisee_data.channel_test_item_result.phy = 0;
	g_hisee_data.channel_test_item_result.size = 0;
	check_and_print_result();
	set_errno_and_return(ret);
#else
	int ret = HISEE_OK;
	check_and_print_result();
	set_errno_and_return(ret);
#endif
}/*lint !e715*/
