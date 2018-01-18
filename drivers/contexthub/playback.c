/*
 * hisi playback driver.
 *
 * Copyright (C) 2015 huawei Ltd.
 * Author:lijiangxiong <lijingxiong@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/sort.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/completion.h>
#include <linux/string.h>
#include <soc_acpu_baseaddr_interface.h>
#include <soc_syscounter_interface.h>

#include <protocol.h>
#include <inputhub_route.h>
#include <inputhub_bridge.h>
#include <global_ddr_map.h>
#include <libhwsecurec/securec.h>
#include "playback.h"

#ifdef HISI_RESERVED_CH_BLOCK_SHMEM_PHYMEM_BASE
#define PLAYBACK_SHARE_MEM_BASEADDR     HISI_RESERVED_CH_BLOCK_SHMEM_PHYMEM_BASE
#else
#define PLAYBACK_SHARE_MEM_BASEADDR     (HISI_RESERVED_SENSORHUB_SHMEM_PHYMEM_BASE + 0X3C00)
#endif
#define PLAYBACK_SHARE_MEM_SIZE         (1<<16)

extern int inputhub_mcu_write_cmd_adapter(const void *buf, unsigned int length,
				   struct read_info *rd);
extern int register_mcu_event_notifier(int tag, int cmd,
				int (*notify) (const pkt_header_t *head));
extern int unregister_mcu_event_notifier(int tag, int cmd,
				  int (*notify) (const pkt_header_t *head));
playback_dev_t g_playback_dev;

/*********************************
     get dir total space
*********************************/
struct linux_dirent {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    char d_name[1];
};

static unsigned int atoi(char *s)
{
	char *p = s;
	char c;
	unsigned long ret = 0;

	if (s == NULL)
		return 0;
	while ((c = *p++) != '\0') {
		if ('0' <= c && c <= '9') {
			ret *= 10;
			ret += (unsigned long) ((unsigned char)c - '0');
			if (ret > U32_MAX)
				return 0;
		} else {
			break;
		}
	}
	return (unsigned int) ret;
}

void search_file_in_dir(const char *path, unsigned int *count, unsigned short tag[])
{
    mm_segment_t old_fs;
    long fd;
    long nread = 0;
    long bpos = 0;
    char *buf;
    struct linux_dirent *d;
    char d_type;
    char *pstr;
    unsigned int i = 0;

    if (NULL == path) {
        printk(KERN_ERR  "playback: path is NULL\n");
        return;
    }
    printk(KERN_ERR  "playback: %s enter\n", __func__);
    buf = (char *)kmalloc((size_t)PLAYBACK_MAX_PATH_LEN, GFP_KERNEL | __GFP_ZERO);
    if (buf == NULL) {
        printk(KERN_ERR "playback:fail to kmalloc\n");
        return;
    }
    old_fs = get_fs();
    set_fs(KERNEL_DS); /*lint !e501*/
    /* check path , if path isnt exist, return. */
    fd = sys_access(path, 0);
    if (0 != fd) {
        printk(KERN_ERR  "playback:dir doesn't exist %s\n", path);
        goto oper_over2;
    }

    fd = sys_open(path, O_RDONLY, 0660);
    if (fd < 0) {
        printk(KERN_ERR "playback:fail to open dir path %s,  fd %ld\n", path, fd);
        goto oper_over1;
    }

    nread = sys_getdents((unsigned int)fd, (struct linux_dirent *)buf,
    PLAYBACK_MAX_PATH_LEN);
    if (nread == -1) {
        printk(KERN_ERR "playback:fail to getdents \n");
        goto oper_over1;
    }

    for (bpos = 0; bpos < nread;) {
        d = (struct linux_dirent *)(buf + bpos);
        d_type = *(buf + bpos + d->d_reclen - 1);
        if (!strncmp((const char *)d->d_name, (const char *)"..", sizeof("..")) ||
            !strncmp((const char *)d->d_name, (const char *)".", sizeof("."))) {
            bpos += d->d_reclen;
            continue;
        }

        if (d_type != DT_DIR)  {
            pstr = strchr(d->d_name, '_');
            if (pstr != NULL) {
                tag[i] = (unsigned short)atoi(pstr + 1);
                printk(KERN_INFO "playback:file %s:%d\n", d->d_name, tag[i]);
                i ++ ;
                if (i >= MAX_SUPPORT_TAG_COUNT) {
                    printk(KERN_ERR "playback:Surpport tag count overflow\n");
                    goto oper_over1;
                }
            }
        }
        bpos += d->d_reclen;
    }

oper_over1:
    sys_close(fd);
oper_over2:
    kfree(buf);
    set_fs(old_fs);
    *count = i;
    return ;
}

static long save_data_to_file(char *name, char *databuf, size_t count)
{
    int flags;
    struct file *fp;
    mm_segment_t old_fs;
    loff_t file_size;
    long ret;
    long fd;

    if (!(databuf) || !(count)) {
        return 0;
    }
    printk(KERN_INFO "playback %s():name:%s count:%ld.\n", __func__, name, count);
    old_fs = get_fs();
    set_fs(KERNEL_DS); /*lint !e501*/

    fd = sys_access(g_playback_dev.current_path, 0);
    if (0 != fd) {
        fd  = sys_mkdir(g_playback_dev.current_path, 0666);
        if (fd < 0) {
            printk(KERN_ERR "playback %s():create dir err:%s.\n", __func__, g_playback_dev.current_path);
            set_fs(old_fs);
            return -1;
        }
    }

    flags = O_CREAT | O_RDWR | O_APPEND;
    fp = filp_open(name, flags, 0666);
    if (!fp || IS_ERR(fp)) {
        set_fs(old_fs);
        printk(KERN_ERR "playback:%s():create %s err.\n", __func__, name);
        return -1;
    }
    file_size = vfs_llseek(fp, 0L, SEEK_END);
    if (file_size > MAX_FILE_SIZE) {
        printk(KERN_ERR "%s():file size is overflow.\n", __func__);
    }

    ret = vfs_write(fp, databuf, count, &(fp->f_pos));
    if (ret != count) {
        printk(KERN_ERR "playback:%s():write file exception with ret %ld.\n", __func__,  ret);
    }

    filp_close(fp, NULL);
    set_fs(old_fs);
    return ret ;

}

static long read_data_form_file(char *name, char *databuf, unsigned int maxcount, size_t offset)
{
    int flags;
    struct file *fp;
    mm_segment_t old_fs;
    size_t size;
    char *data;
    long fd;
    unsigned int count = 0;
    long ret;

    if (!(databuf) || !(maxcount)) {
        return 0;
    }
    printk(KERN_INFO "playback %s():name:%s count:%d.\n", __func__, name, maxcount);
    old_fs = get_fs();
    set_fs(KERNEL_DS); /*lint !e501*/

    fd = sys_access(name, 0);
    if (0 != fd) {
        printk(KERN_ERR "playback:file not exist:%s.%ld\n", name, fd);
        set_fs(old_fs);
        return -1;
    }

    flags = O_RDONLY;
    fp = filp_open(name, flags, 0644);
    if (!fp || IS_ERR(fp)) {
        set_fs(old_fs);
        printk(KERN_ERR "playback:open file err.%s.%pK\n", name, fp);
        return -1;
    }

    if (vfs_llseek(fp, (loff_t)offset, SEEK_SET) < 0) {
        printk(KERN_ERR "playback:large than file size %ld\n", offset);
        goto err;
    }
    data = databuf;
    do {
        size = 4;
        if (count + size > maxcount) {
            break;
        }
        ret = vfs_read(fp, data, size, &fp->f_pos);
        if (ret != size) {
            printk(KERN_ERR "playback:%s():read file exception with ret %ld:%lld.\n", __func__, ret, fp->f_pos);
            goto err;
        }

        size = *(unsigned short *)(data + 2);
        if (count + size + 4  > maxcount) {
            break ;
        }
        data += 4;
        ret = vfs_read(fp, data, size, &fp->f_pos);
        if (ret != size) {
            printk(KERN_ERR "playback:%s():read file exception with ret %ld:%lld.\n", __func__, ret, fp->f_pos);
            goto err;
        }
        data += size;
        count += size + 4;
    } while (1);

err:
    filp_close(fp, NULL);
    set_fs(old_fs);
    return count ;

}

static int send_cmd_from_kernel(unsigned char cmd_tag, unsigned char cmd_type,
    unsigned int subtype, char  *buf, size_t count)
{
    char buffer[MAX_PKT_LENGTH] = {0};
    int ret = 0;
    unsigned int *subcmd = (unsigned int *)(((pkt_header_t *)buffer) + 1);

    /*init sending cmd*/
    ((pkt_header_t *)buffer)->tag = cmd_tag;
    ((pkt_header_t *)buffer)->resp = NO_RESP;
    ((pkt_header_t *)buffer)->cmd = cmd_type;
    printk(KERN_INFO "playback:%s : cmd_tag[0x%x] cmd[%d:0x%x]  count[%ld]\n",
        __func__, cmd_tag, cmd_type, subtype, count);
    print_hex_dump_bytes("playback:", DUMP_PREFIX_OFFSET, buf, count);

    if (CMD_CMN_CONFIG_REQ != cmd_type) {
        if ((count) && (count < (MAX_PKT_LENGTH - sizeof(pkt_header_t)))) {
            memcpy_s(buffer + sizeof(pkt_header_t),
                (MAX_PKT_LENGTH - sizeof(pkt_header_t)), buf, count);
        }
        ((pkt_header_t *)buffer)->length = (unsigned short)count;
        ret = inputhub_mcu_write_cmd_adapter(buffer, (unsigned int)(count + sizeof(pkt_header_t)), NULL);
    } else {
        *subcmd = subtype;
        if ((count) && (count < (MAX_PKT_LENGTH - sizeof(pkt_header_t) - 4))) {
            memcpy_s(buffer + sizeof(pkt_header_t) + 4,
                (MAX_PKT_LENGTH - sizeof(pkt_header_t) - 4), buf, count);
        }
        ((pkt_header_t *)buffer)->length = (unsigned short)(count + sizeof(unsigned int));
        ret = inputhub_mcu_write_cmd_adapter(buffer, (unsigned int)(count + sizeof(pkt_header_t) + 4), NULL);
    }
    if (ret) {
        printk(KERN_ERR "playback:%s error\n", __func__);
    }
    return ret;
}

static int send_cmd_from_kernel_response(unsigned char cmd_tag, unsigned char cmd_type,
    unsigned int subtype, char  *buf, size_t count, struct read_info *rd)
{
    char *buffer;
    int ret = 0;
    unsigned int *subcmd;
    if (NULL == rd) {
        printk(KERN_ERR "playback:%s error\n", __func__);
        return -EPERM;
    }
    buffer = rd->data;
    subcmd = (unsigned int *)(((pkt_header_t *)buffer) + 1);
    /*init sending cmd*/
    ((pkt_header_t *)buffer)->tag = cmd_tag;
    ((pkt_header_t *)buffer)->resp = RESP;
    ((pkt_header_t *)buffer)->cmd = cmd_type;
    printk(KERN_INFO "playback:%s : cmd_tag[0x%x] cmd[%d:0x%x]  count[%ld]\n",
    __func__, cmd_tag, cmd_type, subtype, count);
    print_hex_dump_bytes("playback:", DUMP_PREFIX_OFFSET, buf, count);

    if (CMD_CMN_CONFIG_REQ != cmd_type) {
        if ((count) && (count < (MAX_PKT_LENGTH - sizeof(pkt_header_t)))) {
            memcpy_s(buffer + sizeof(pkt_header_t),
            (MAX_PKT_LENGTH - sizeof(pkt_header_t)), buf, count);
        }
        ((pkt_header_t *)buffer)->length = (unsigned short)count;
        ret = inputhub_mcu_write_cmd_adapter(buffer, (unsigned int)(count + sizeof(pkt_header_t)), rd);
    } else {
        *subcmd = subtype;
        if ((count) && (count < (MAX_PKT_LENGTH - sizeof(pkt_header_t) - 4))) {
            memcpy_s(buffer + sizeof(pkt_header_t) + 4,
            (MAX_PKT_LENGTH - sizeof(pkt_header_t) - 4), buf, count);
        }
        ((pkt_header_t *)buffer)->length = (unsigned short)(count + sizeof(unsigned int));
        ret = inputhub_mcu_write_cmd_adapter(buffer, (unsigned int)(count + sizeof(pkt_header_t) + 4), rd);
    }

    if ((ret) || (rd->errno)) {
            pr_err("playback:[%s]error[%d][%d]\n", __func__, ret, rd->errno);
        return -EBUSY;
    }

    return 0;
}

static long hisi_playback_init_cmd(unsigned long arg)
{
    app_init_cmd_t init_cmd;
    app_init_cmd_t *argp = (app_init_cmd_t *)arg;
    unsigned int count;
    playback_info_t *pinfo;
    unsigned int size;
    unsigned short tag[MAX_SUPPORT_TAG_COUNT] = {0};
    char *pbuf;
    struct read_info rd;
    int ret;
    if (copy_from_user(&init_cmd, (void *)arg, (size_t)sizeof(app_init_cmd_t))) {
        printk(KERN_ERR "%s flp_ioctl copy_from_user error\n", __func__);
        return -EFAULT;
    }
    if (copy_from_user(g_playback_dev.current_path, (const void __user *)argp->path, (size_t)PLAYBACK_MAX_PATH_LEN)) {
        printk(KERN_ERR "%s flp_ioctl copy_from_user error\n", __func__);
        return -EFAULT;
    }
    if (PLAYBACK_MAX_PATH_LEN == strnlen(g_playback_dev.current_path, (size_t)PLAYBACK_MAX_PATH_LEN))  {
        printk(KERN_ERR "%s path in overflow\n", __func__);
        return -EFAULT;
    }
    if (init_cmd.mode) {
        g_playback_dev.current_count = MAX_SUPPORT_TAG_COUNT;
    } else {
        search_file_in_dir(g_playback_dev.current_path, &g_playback_dev.current_count, tag);
        if (!g_playback_dev.current_count) {
            printk(KERN_ERR "playback:%s no file\n", __func__);
            return -EFAULT;
        }
    }
    /*if dispose IOCTL_PLAYBACK_INIT cmd repeatly, just free memory that malloc in last time*/
    if (g_playback_dev.info != NULL) {
        kfree(g_playback_dev.info);
        printk(KERN_ERR "%s reinit without release\n", __func__);
    }
    pinfo = (playback_info_t *)kmalloc(g_playback_dev.current_count*sizeof(playback_info_t), GFP_KERNEL|__GFP_ZERO);
    if (!pinfo) {
        printk(KERN_ERR "%s no memory \n", __func__);
        return -ENOMEM;
    }
    pbuf = (char *)kmalloc((size_t)MAX_PKT_LENGTH, GFP_KERNEL|__GFP_ZERO);
    if (!pbuf) {
        printk(KERN_ERR "%s no memory \n", __func__);
        kfree(pinfo);
        return -ENOMEM;
    }
    g_playback_dev.info = pinfo;
    g_playback_dev.current_mode = init_cmd.mode;
    *pbuf = (char)init_cmd.mode;
    size = (PLAYBACK_SHARE_MEM_SIZE/g_playback_dev.current_count) & ~0x3ff ;
    for (count = 0; count < g_playback_dev.current_count; count++) {
        if (init_cmd.mode) {
            pinfo->tag_id = count;
        } else {
            pinfo->tag_id = tag[count];
        }

        pinfo->buf1_addr = PLAYBACK_SHARE_MEM_BASEADDR + count * size;
        pinfo->buf2_addr = PLAYBACK_SHARE_MEM_BASEADDR + count * size + (size >> 1);
        pinfo->buf_size = size >> 1;
        pinfo->file_offset = 0;
        memcpy_s((pbuf + 1 + count * sizeof(sm_info_t)),
            (MAX_PKT_LENGTH - 1 - count * sizeof(sm_info_t)), pinfo, sizeof(sm_info_t));
        pinfo ++ ;
    }
    ret = send_cmd_from_kernel_response(TAG_DATA_PLAYBACK, CMD_CMN_OPEN_REQ,
        0, (char *)pbuf, count * sizeof(sm_info_t) + 1, &rd);
    if (!ret) {
        g_playback_dev.status |= FUNCTION_INIT;
    }
    kfree (pbuf) ;
    return ret;
}

static long hisi_playback_start_cmd(void)
{
    unsigned int count;
    data_status_t *pbuf;
    buf_status_t *pstatus;
    playback_info_t *info = g_playback_dev.info;
    char buf[MAX_PKT_LENGTH] = {0};

    if (FUNCTION_START == (g_playback_dev.status & FUNCTION_START)) {
        printk(KERN_ERR "%s data playback start aready\n", __func__);
        return -EFAULT;
    }

    /*1:record 0:playback*/
    if (g_playback_dev.current_mode) {
        pstatus = (buf_status_t *)buf;
        for (count = 0; count < g_playback_dev.current_count; count++) {
            pstatus->tag_id = info->tag_id;
            pstatus->buf_status = BUFFER_ALL_READY;
            pstatus ++;
            info ++;
        }
        return send_cmd_from_kernel (TAG_DATA_PLAYBACK, CMD_CMN_CONFIG_REQ,
           CMD_DATA_PLAYBACK_BUF_READY_REQ, (char *)buf, count * sizeof(buf_status_t));
    } else {
        pbuf = (data_status_t *)buf;
        for (count = 0; count < g_playback_dev.current_count; count++) {
            pbuf->tag_id = info->tag_id;
            pbuf->buf_status = BUFFER_ALL_READY;
            snprintf_s(g_playback_dev.filename, PLAYBACK_MAX_PATH_LEN, (size_t)(PLAYBACK_MAX_PATH_LEN - 1),
                "%s/data_%d.bin", g_playback_dev.current_path, info->tag_id);
            if (PLAYBACK_MAX_PATH_LEN == strnlen(g_playback_dev.filename, (size_t)PLAYBACK_MAX_PATH_LEN))  {
                printk(KERN_ERR "%s path in overflow\n", __func__);
                return -EFAULT;
            }
            pbuf->data_size[0] = (unsigned int)read_data_form_file(g_playback_dev.filename,
                ((char *)g_playback_dev.vaddr + info->buf1_addr - g_playback_dev.phyAddr),
                info->buf_size, (size_t)info->file_offset);
            info->file_offset += pbuf->data_size[0];
            pbuf->data_size[1] = (unsigned int)read_data_form_file(g_playback_dev.filename,
                ((char *)g_playback_dev.vaddr + info->buf2_addr - g_playback_dev.phyAddr),
                info->buf_size, (size_t)info->file_offset);
            info->file_offset += pbuf->data_size[1];
            pbuf ++;
            info ++;
        }
        return send_cmd_from_kernel (TAG_DATA_PLAYBACK, CMD_CMN_CONFIG_REQ,
           CMD_DATA_PLAYBACK_DATA_READY_REQ, (char *)buf, (size_t)(count * sizeof(data_status_t)));
    }
}

static void hisi_playback_work(struct work_struct *wk)
{
    mutex_lock(&g_playback_dev.mutex_lock);
    if (IOCTL_PLAYBACK_START == g_playback_dev.data) {
        hisi_playback_start_cmd();
        g_playback_dev.status |= FUNCTION_START;
        g_playback_dev.data = 0;
    }
    mutex_unlock(&g_playback_dev.mutex_lock);
    return;
}

/*one times only one buf empty or full*/
static int handle_notify_from_mcu_playback(const pkt_header_t *head)
{
    data_status_t *pstatus = (data_status_t  *) (head + 1);
    playback_info_t *pinfo;
    char buf[MAX_PKT_LENGTH] = {0};
    data_status_t *pbuf = (data_status_t *)buf;
    unsigned int count;

    printk(KERN_ERR "%s mode:%d length:%d tag:%d\n", __func__, g_playback_dev.current_count,
        head->length, pstatus->tag_id);
    pinfo = g_playback_dev.info;
    for (count = 0; count < g_playback_dev.current_count; count ++) {
        if (pinfo->tag_id == pstatus->tag_id) {
            break;
        }
        pinfo ++;
    }

    if (count == g_playback_dev.current_count) {
        printk(KERN_ERR "%s tag_id error for mcu\n", __func__);
        return -EFAULT;
    }
    pbuf->tag_id = pstatus->tag_id;
    /*0:playback; handle playback*/
    /*if IOCTL_PLAYBACK_STOP, just close*/
    if (FUNCTION_START != (g_playback_dev.status & FUNCTION_START)) {
        printk(KERN_ERR "%s data playback stop aready\n", __func__);
        return -EFAULT;
    }
    if ((g_playback_dev.is_wait) &&
        ((!pstatus->data_size[0] && BUFFER1_READY == pstatus->buf_status) ||
        (!pstatus->data_size[1] && BUFFER2_READY == pstatus->buf_status))) {
        g_playback_dev.complete_status = COMPLETE_REPLAY_DONE;
        complete(&g_playback_dev.done);
    }
    snprintf_s(g_playback_dev.filename, PLAYBACK_MAX_PATH_LEN, (PLAYBACK_MAX_PATH_LEN - 1),
        "%s/data_%d.bin", g_playback_dev.current_path, pstatus->tag_id);
    if (PLAYBACK_MAX_PATH_LEN == strnlen(g_playback_dev.filename, (size_t)PLAYBACK_MAX_PATH_LEN))  {
        printk(KERN_ERR "%s path in overflow\n", __func__);
        return -EFAULT;
    }
    if (BUFFER1_READY == pstatus->buf_status) {
        count = (unsigned int)read_data_form_file(g_playback_dev.filename,
            ((char*)g_playback_dev.vaddr + pinfo->buf1_addr - g_playback_dev.phyAddr),
            pinfo->buf_size, (size_t)pinfo->file_offset);
        pbuf->data_size[0] = count;
        pinfo->file_offset += count;
        pbuf->buf_status = BUFFER1_READY;
    } else if (BUFFER2_READY == pstatus->buf_status) {
        count = (unsigned int)read_data_form_file(g_playback_dev.filename,
            ((char *)g_playback_dev.vaddr + pinfo->buf2_addr - g_playback_dev.phyAddr),
            pinfo->buf_size, (size_t)pinfo->file_offset);
        pbuf->data_size[1] = count;
        pinfo->file_offset += count;
        pbuf->buf_status = BUFFER2_READY;
    } else {
        printk(KERN_ERR "%s Buf Ready at the same time, not support\n", __func__);
        return -EFAULT;
    }
    return send_cmd_from_kernel (TAG_DATA_PLAYBACK, CMD_CMN_CONFIG_REQ,
            CMD_DATA_PLAYBACK_DATA_READY_REQ, (char *)pbuf, sizeof(data_status_t));
}

/*one times only one buf empty or full*/
static int handle_notify_from_mcu_record(const pkt_header_t *head)
{
    data_status_t *pstatus = (data_status_t  *) (head + 1);
    char *data;
    playback_info_t *pinfo;
    char buf[MAX_PKT_LENGTH] = {0};
    data_status_t *pbuf = (data_status_t *)buf;
    unsigned int count;

    printk(KERN_ERR "%s mode:%d length:%d tag:%d\n", __func__, g_playback_dev.current_count,
        head->length, pstatus->tag_id);
    pinfo = g_playback_dev.info;
    for (count = 0; count < g_playback_dev.current_count; count ++) {
        if (pinfo->tag_id == pstatus->tag_id) {
            break;
        }
        pinfo ++;
    }

    if (count == g_playback_dev.current_count) {
        printk(KERN_ERR "%s tag_id error for mcu\n", __func__);
        return -EFAULT;
    }
    pbuf->tag_id = pstatus->tag_id;
    /*1:record; handle record */
    if (BUFFER1_READY == pstatus->buf_status) {
        data = (char *)g_playback_dev.vaddr + pinfo->buf1_addr - g_playback_dev.phyAddr;
        pbuf->buf_status = BUFFER1_READY;
    } else if (BUFFER2_READY == pstatus->buf_status) {
        data = (char *)g_playback_dev.vaddr + pinfo->buf2_addr - g_playback_dev.phyAddr;
        pbuf->buf_status = BUFFER2_READY;
    } else {
        printk(KERN_ERR "%s Buf Ready at the same time, not support\n", __func__);
        return -EFAULT;
    }
    snprintf_s(g_playback_dev.filename, PLAYBACK_MAX_PATH_LEN, (PLAYBACK_MAX_PATH_LEN - 1),
        "%s/data_%d.bin", g_playback_dev.current_path, *((unsigned short *)data));
    if (PLAYBACK_MAX_PATH_LEN == strnlen(g_playback_dev.filename, (size_t)PLAYBACK_MAX_PATH_LEN))  {
        printk(KERN_ERR "%s path in overflow\n", __func__);
        return -EFAULT;
    }
    save_data_to_file(g_playback_dev.filename, data, (size_t)pinfo->buf_size);
    return send_cmd_from_kernel (TAG_DATA_PLAYBACK, CMD_CMN_CONFIG_REQ,
       CMD_DATA_PLAYBACK_BUF_READY_REQ, (char *)pbuf, sizeof(buf_status_t));
}

/*one times only one buf empty or full*/
static int hisi_playback_notify_from_mcu(const pkt_header_t *head)
{
    int ret;

    mutex_lock(&g_playback_dev.mutex_lock);

    /*if release, just return*/
    if (!g_playback_dev.info) {
        mutex_unlock(&g_playback_dev.mutex_lock);
        printk(KERN_ERR "%s dev have closed\n", __func__);
        return -EFAULT;
    }

    /*1:record 0:playback*/
    if (g_playback_dev.current_mode) {
        ret = handle_notify_from_mcu_record(head);
    } else {
        ret = handle_notify_from_mcu_playback(head);
    }
    mutex_unlock(&g_playback_dev.mutex_lock);
    return ret;

}

static long hisi_playback_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret = 0;
    printk(KERN_ERR "%s : 0X%x  \n", __func__, cmd);
    switch (cmd) {
        case IOCTL_PLAYBACK_INIT:
            mutex_lock(&g_playback_dev.mutex_lock);
            ret = hisi_playback_init_cmd(arg);
            mutex_unlock(&g_playback_dev.mutex_lock);
            break;
        case IOCTL_PLAYBACK_START:
            mutex_lock(&g_playback_dev.mutex_lock);
            if ((FUNCTION_INIT != (g_playback_dev.status & FUNCTION_INIT)) ||
                (FUNCTION_START == (g_playback_dev.status & FUNCTION_START))) {
                mutex_unlock(&g_playback_dev.mutex_lock);
                printk(KERN_ERR "%s data playback START aready\n", __func__);
                return -EFAULT;
            }
            g_playback_dev.data = IOCTL_PLAYBACK_START;
            queue_work(system_power_efficient_wq, &g_playback_dev.work);
            mutex_unlock(&g_playback_dev.mutex_lock);
            break;
        case IOCTL_PLAYBACK_STOP:
            mutex_lock(&g_playback_dev.mutex_lock);
            if (g_playback_dev.status & FUNCTION_START) {
                send_cmd_from_kernel (TAG_DATA_PLAYBACK, CMD_CMN_CLOSE_REQ, 0, NULL, 0UL);
                g_playback_dev.status &= ~FUNCTION_START;
            }
            mutex_unlock(&g_playback_dev.mutex_lock);
            break;
        case IOCTL_PLAYBACK_REPLAY_COMPLETE:
            g_playback_dev.is_wait = TRUE;
            wait_for_completion_interruptible(&g_playback_dev.done);
            mutex_lock(&g_playback_dev.mutex_lock);
            g_playback_dev.is_wait = FALSE;
            if (copy_to_user((void *)arg, &g_playback_dev.complete_status, (size_t)sizeof(int))) {
                mutex_unlock(&g_playback_dev.mutex_lock);
                printk(KERN_ERR "%s playback: copy_from_user error\n", __func__);
                return -EFAULT;
            }
            mutex_unlock(&g_playback_dev.mutex_lock);
            printk(KERN_ERR "%s  replay complete\n", __func__);
            break;
        default:
            printk(KERN_ERR "%s  cmd not surpport\n", __func__);
            return -EFAULT;
    }
    return ret;
}

static int hisi_playback_open(struct inode *inode, struct file *filp)
{
    mutex_lock(&g_playback_dev.mutex_lock);
    g_playback_dev.status = FUNCTION_OPEN;
    g_playback_dev.info = NULL;
    mutex_unlock(&g_playback_dev.mutex_lock);
    printk(KERN_ERR "%s %d: enter\n", __func__, __LINE__);
    return 0;
}

static int hisi_playback_release(struct inode *inode, struct file *file)
{
    mutex_lock(&g_playback_dev.mutex_lock);
    if(!g_playback_dev.status) {
        mutex_unlock(&g_playback_dev.mutex_lock);
        return 0;
    } else if (g_playback_dev.status & FUNCTION_START) {
        send_cmd_from_kernel (TAG_DATA_PLAYBACK, CMD_CMN_CLOSE_REQ, 0, NULL, 0UL);
    }
    g_playback_dev.status = 0;
    if (g_playback_dev.info) {
        kfree(g_playback_dev.info);
        g_playback_dev.info = NULL;
    }
    if (g_playback_dev.is_wait) {
        g_playback_dev.complete_status = COMPLETE_CLOSE;
        complete(&g_playback_dev.done);
    }
    printk(KERN_ERR "%s %d: close\n", __func__, __LINE__);
    mutex_unlock(&g_playback_dev.mutex_lock);
    return 0;
}

static const struct file_operations hisi_playback_fops = {
    .owner =          THIS_MODULE,
    .unlocked_ioctl = hisi_playback_ioctl,
    .open       =     hisi_playback_open,
    .release    =     hisi_playback_release,
};

/*******************************************************************************************
Description:   miscdevice to data playback function
*******************************************************************************************/
static struct miscdevice hisi_playback_miscdev =
{
    .minor =    MISC_DYNAMIC_MINOR,
    .name =     "playback",
    .fops =     &hisi_playback_fops,
    .mode =     0660,
};

static int generic_playback_probe(struct platform_device *pdev)
{
    int ret;
    char *status = NULL;
    struct device_node *np = pdev->dev.of_node;

    dev_info(&pdev->dev, "generic_playback_probe \n");
    ret = of_property_read_string(np, "status", (const char **)&status);
	if (!ret) {
        if (!strncmp("disabled", status, strlen("disabled"))) {
            printk(KERN_ERR "cannot register hisi playback driver, dts status setting is disabled\n");
            return -EPERM;
        }
    }
    ret = of_property_read_u32(np, "playback_mem_addr", &g_playback_dev.phyAddr);
    if (ret < 0) {
        g_playback_dev.phyAddr = PLAYBACK_SHARE_MEM_BASEADDR;
    }
    ret = of_property_read_u32(np, "playback_mem_size", &g_playback_dev.size);
    if (ret < 0) {
        g_playback_dev.size = PLAYBACK_SHARE_MEM_BASEADDR;
    }

    g_playback_dev.vaddr =
	    ioremap_wc((size_t)g_playback_dev.phyAddr, g_playback_dev.size);
	if (!g_playback_dev.vaddr) {
		pr_err("[%s] ioremap err\n", __func__);
		return -ENOMEM;
	}
    mutex_init(&g_playback_dev.mutex_lock);
    init_completion(&g_playback_dev.done);
    INIT_WORK(&g_playback_dev.work, hisi_playback_work);
    register_mcu_event_notifier(TAG_DATA_PLAYBACK, CMD_DATA_PLAYBACK_DATA_READY_RESP, hisi_playback_notify_from_mcu);
    register_mcu_event_notifier(TAG_DATA_PLAYBACK, CMD_DATA_PLAYBACK_BUF_READY_RESP, hisi_playback_notify_from_mcu);
    ret = misc_register(&hisi_playback_miscdev);
    if (ret != 0)    {
        printk(KERN_ERR "cannot register hisi playback err=%d\n", ret);
    }
    return ret;
}

static int generic_playback_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "%s %d: \n", __func__, __LINE__);
    unregister_mcu_event_notifier(TAG_DATA_PLAYBACK, CMD_DATA_PLAYBACK_DATA_READY_RESP, hisi_playback_notify_from_mcu);
    unregister_mcu_event_notifier(TAG_DATA_PLAYBACK, CMD_DATA_PLAYBACK_BUF_READY_RESP, hisi_playback_notify_from_mcu);
    misc_deregister(&hisi_playback_miscdev);
    cancel_work_sync(&g_playback_dev.work);
    iounmap(g_playback_dev.vaddr);
    return 0;
}

static const struct of_device_id generic_playback[] = {
    { .compatible = "hisilicon,Contexthub-playback" },
    {},
};
MODULE_DEVICE_TABLE(of, generic_playback);
static struct platform_driver generic_playback_platdrv = {
    .driver = {
        .name	= "hisi-playback",
        .owner	= THIS_MODULE,
        .of_match_table = of_match_ptr(generic_playback),
    },
    .probe		= generic_playback_probe,
    .remove     = generic_playback_remove,
};

static int __init hisi_playback_init(void)
{
    return platform_driver_register(&generic_playback_platdrv);
}

static void __exit hisi_playback_exit(void)
{
    platform_driver_unregister(&generic_playback_platdrv);
}

late_initcall_sync(hisi_playback_init);
module_exit(hisi_playback_exit);

MODULE_AUTHOR(" hisi<hisi@huawei.com>");
MODULE_DESCRIPTION("Generic huawei playback driver via DT");
MODULE_LICENSE("GPL");

