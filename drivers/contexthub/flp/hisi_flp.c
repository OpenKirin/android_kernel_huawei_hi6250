/*
 * hisi flp driver.
 *
 * Copyright (C) 2015 huawei Ltd.
 * Author:lijiangxiong <lijingxiong@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/notifier.h>
#include <linux/syscalls.h>
#include <net/genetlink.h>
#include <linux/workqueue.h>
#include <linux/hisi/hisi_syscounter.h>
#include <soc_acpu_baseaddr_interface.h>
#include <soc_syscounter_interface.h>
#include <clocksource/arm_arch_timer.h>
#include <linux/wakelock.h>
#include "protocol.h"
#include "inputhub_route.h"
#include "inputhub_bridge.h"

#include "hisi_flp.h"
#include "hisi_softtimer.h"
#define CONTEXT_RESP_API

/*lint -e750 -esym(750,*) */
#define HISI_FLP_DEBUG              KERN_INFO
#define FLP_PDR_DATA	(0x1<<0)
#define FLP_AR_DATA	(0x1<<1)
#define FLP_BATCHING	(0x1<<2)
#define FLP_GEOFENCE	(0x1<<3)
#define FLP_ENVIRONMENT (0x1<<4)
#define PDR_INTERVAL_MIN	1000
#define PDR_INTERVAL_MXN	(3600*1000)
#define PDR_PRECISE_MIN	PDR_INTERVAL_MIN
#define PDR_PRECISE_MXN	60000
#define PDR_DATA_MAX_COUNT      300
#define PDR_DATA_MIX_COUNT      1
#define PDR_SINGLE_SEND_COUNT   48
#ifndef TRUE
#define  TRUE   1
#endif
/*lint +e750 +esym(750,*) */
/*lint -e750 -e754 -esym(754,*) */
typedef struct flp_ioctl_cmd{
    pkt_header_t    hd;
    unsigned int    sub_cmd;
} flp_ioctl_cmd_t;

typedef struct ar_ioctl_cmd{
    pkt_header_t    hd;
    unsigned char   core;
    unsigned char   rsv1;
    unsigned char   rsv2;
    unsigned char   sub_cmd;
} ar_ioctl_cmd_t;
/*lint +e750 +e754 +esym(754,*) */

typedef struct flp_device {
	struct list_head        list;
	pdr_start_config_t      pdr_config;
	context_dev_info_t	 ar_dev_info;
	context_dev_info_t env_dev_info;
	unsigned int            pdr_start_count;
	unsigned int            pdr_flush_config ;
	unsigned int            service_type ;
	struct mutex            lock;
	unsigned int            pdr_cycle;
	compensate_data_t       pdr_compensate;
	unsigned int            denial_sevice;
	long                    pdr_last_utc;
} flp_device_t;

typedef struct flp_data_buf{
    char                  *data_buf;
    unsigned int          buf_size;
    unsigned int          read_index;
    unsigned int          write_index;
    unsigned int          data_count;
} flp_data_buf_t;

typedef enum _ar_ver_e {
	AR_VER_0 = 0,
	AR_VER_1 = 1,
	AR_VER_2 = 2
}ar_ver_e;

typedef struct flp_port {
	struct list_head        list;
	unsigned int            port_type ;
	unsigned int            channel_type;
	struct softtimer_list   sleep_timer ;
	flp_data_buf_t          pdr_buf;
	flp_data_buf_t          ar_buf;
	flp_data_buf_t          env_buf;
	pdr_start_config_t      pdr_config;
	pdr_start_config_t      pdr_update_config;
	context_iomcu_cfg_t       ar_config;
#ifdef ENV_FEATURE
	context_iomcu_cfg_t       env_config;
#endif
#ifdef GEOFENCE_BATCH_FEATURE
	batching_config_t       gps_batching_config;
#endif
	unsigned int            rate ;          /*control get pdr data*/
	unsigned int            interval ;       /*control get pdr data*/
	unsigned int            nextid;         /*control get pdr data*/
	unsigned int            aquiredid;      /*control get pdr data*/
	unsigned int            pdr_flush_config  ;
	unsigned int            portid;
	unsigned long           total_count ;
	struct work_struct      work;
	unsigned int            work_para;
	unsigned int            need_awake;
	unsigned int            need_report;
	unsigned int flushing;
	ar_ver_e ar_ver;
	compensate_data_t       pdr_compensate;
	struct wake_lock        wlock;
	unsigned int            need_hold_wlock;
} flp_port_t;

extern int inputhub_mcu_write_cmd_adapter(const void *buf,
        unsigned int length, read_info_t *rd);
extern int inputhub_mcu_write_cmd_nolock(const void *buf, unsigned int length);
extern int register_mcu_event_notifier(int tag, int cmd,
        int (*notify)(const pkt_header_t *head));
extern int getSensorMcuMode(void);

#ifndef STUB_FOR_TEST
#define SEND_CMD_TO_SENSORHUB       inputhub_mcu_write_cmd_adapter
#define SEND_CMD_TO_SENSORHUB_NOLOCK  inputhub_mcu_write_cmd_nolock
#define PDR_REGISTER_CALLBACK       register_mcu_event_notifier
#else
#define SEND_CMD_TO_SENSORHUB
#define PDR_REGISTER_CALLBACK
#endif
#define FLP_DEBUG(message...) \
do { \
    if (g_flp_debug_level) { \
        printk(message); \
    } \
} while (0)

extern unsigned int g_flp_debug_level;
flp_device_t  g_flp_dev;
/*lint -e785 */
static struct genl_family flp_genl_family = {
    .id         = GENL_ID_GENERATE,
    .name       = FLP_GENL_NAME,
    .version    = TASKFLP_GENL_VERSION,
    .maxattr    = FLP_GENL_ATTR_MAX,
};
/*lint +e785 */

static int calc_GCD(unsigned int a, unsigned int b)
{
        unsigned int tmp;

        if (0 == a || 0 == b) {
                return 0;
        }

        /* Let a be the bigger one! */
        if (a < b) {
                tmp = a; a = b; b = tmp;
        }

        while ((a % b) != 0) {
                tmp = b;
                b = a % b;
                a = tmp;
        }
        return (int)b;
}

static int send_cmd_from_user(unsigned char cmd_tag, unsigned char cmd_type,
    unsigned int subtype, char __user *buf, size_t count)
{
    char buffer[MAX_PKT_LENGTH] = {0};
    /*init sending cmd*/
    ((pkt_header_t *)buffer)->tag = cmd_tag;
    ((pkt_header_t *)buffer)->resp = NO_RESP;
    ((pkt_header_t *)buffer)->cmd = cmd_type;
    printk(HISI_FLP_DEBUG "flp:%s : cmd_tag[0x%x] cmd[%d:0x%x]  count[%ld]\n",
        __func__, cmd_tag, cmd_type, subtype, count);

    if (CMD_CMN_CONFIG_REQ != cmd_type) {
	if (count >= (MAX_PKT_LENGTH - sizeof(pkt_header_t))){
		pr_err("[%s]pkt_header_t count[%ld]err\n",__func__,count);
		return -EIO;
	}

	if (count) {   /* [false alarm]:fortify */
		if (copy_from_user(buffer + sizeof(pkt_header_t), buf, count)) {
			printk(KERN_ERR "copy_to_user error line:%d\n", __LINE__);
			return -EIO;
		}
	}
	((pkt_header_t *)buffer)->length = (unsigned short)count;
	SEND_CMD_TO_SENSORHUB(buffer, (unsigned int)(count + sizeof(pkt_header_t)), NULL);
    } else {
	if (TAG_AR == cmd_tag) {
		((ar_ioctl_cmd_t *)buffer)->sub_cmd = (unsigned char)subtype;
	} else {
		((flp_ioctl_cmd_t *)buffer)->sub_cmd = subtype;
	}

	if (count >= (MAX_PKT_LENGTH - sizeof(flp_ioctl_cmd_t))){
		pr_err("[%s]flp_ioctl_cmd_t count[%ld]err\n",__func__,count);
		return -EIO;
	}

	if (count) {
		if (copy_from_user(buffer + sizeof(flp_ioctl_cmd_t), buf, count)) {
			printk(KERN_ERR "copy_to_user error line:%d\n", __LINE__);
			return -EIO;
		}
	}
	((pkt_header_t *)buffer)->length = (unsigned short)(count + sizeof(unsigned int));
	SEND_CMD_TO_SENSORHUB(buffer, (unsigned int)(count + sizeof(flp_ioctl_cmd_t)), NULL);
    }
    return 0;
}
/*lint -e655*/
static int send_cmd_from_kernel(unsigned char cmd_tag, unsigned char cmd_type,
    unsigned int subtype, char  *buf, size_t count)
{
	char buffer[MAX_PKT_LENGTH] = {0};
	int ret = 0;
	/*init sending cmd*/
	((pkt_header_t *)buffer)->tag = cmd_tag;
	((pkt_header_t *)buffer)->resp = NO_RESP;
	((pkt_header_t *)buffer)->cmd = cmd_type;
	printk(HISI_FLP_DEBUG "flp:%s : cmd_tag[0x%x] cmd[%d:0x%x]  count[%ld]\n",
		__func__, cmd_tag, cmd_type, subtype, count);

	if (CMD_CMN_CONFIG_REQ != cmd_type) {
		if (count >= (MAX_PKT_LENGTH - sizeof(pkt_header_t))){
			pr_err("[%s]pkt_header_t count[%ld]err\n",__func__,count);
			return -EIO;
		}

		if (count)
			memcpy(buffer + sizeof(pkt_header_t), buf, count);

		((pkt_header_t *)buffer)->length = (unsigned short)count;
		ret = SEND_CMD_TO_SENSORHUB(buffer, (unsigned int)(count + sizeof(pkt_header_t)), NULL);
	} else {
		if ((TAG_AR == cmd_tag) || (TAG_ENVIRONMENT == cmd_tag)) {
			((ar_ioctl_cmd_t *)buffer)->sub_cmd = (unsigned char)subtype;
		} else {
			((flp_ioctl_cmd_t *)buffer)->sub_cmd = subtype;
		}

		if (count >= (MAX_PKT_LENGTH - sizeof(flp_ioctl_cmd_t))){
			pr_err("[%s]flp_ioctl_cmd_t count[%ld]err\n",__func__,count);
			return -EIO;
		}

		if (count)
			memcpy(buffer + sizeof(flp_ioctl_cmd_t), buf, count);

		((pkt_header_t *)buffer)->length = (unsigned short)(count + sizeof(unsigned int));
		ret = SEND_CMD_TO_SENSORHUB(buffer, (unsigned int)(count + sizeof(flp_ioctl_cmd_t)), NULL);
	}

	if (ret)
		printk(KERN_ERR "%s error\n", __func__);

	return ret;
}

static int send_cmd_from_kernel_response(unsigned char cmd_tag, unsigned char cmd_type,
    unsigned int subtype, char  *buf, size_t count, struct read_info *rd)
{
	char *buffer;
	int ret = 0;
	if (NULL == rd) {
		printk(KERN_ERR "flp:%s error\n", __func__);
		return -EPERM;
	}
	buffer = rd->data;
	/*init sending cmd*/
	((pkt_header_t *)buffer)->tag = cmd_tag;
	((pkt_header_t *)buffer)->resp = RESP;
	((pkt_header_t *)buffer)->cmd = cmd_type;
	printk(HISI_FLP_DEBUG "flp:%s : cmd_tag[0x%x] cmd[%d:0x%x]  count[%ld]\n",
		__func__, cmd_tag, cmd_type, subtype, count);

	if (CMD_CMN_CONFIG_REQ != cmd_type) {
		if (count >= (MAX_PKT_LENGTH - sizeof(pkt_header_t))){
			pr_err("[%s]pkt_header_t count[%ld]err\n",__func__,count);
			return -EIO;
		}

		if (count)
			memcpy(buffer + sizeof(pkt_header_t), buf, count);

		((pkt_header_t *)buffer)->length = (unsigned short)count;
		ret = SEND_CMD_TO_SENSORHUB(buffer, (unsigned int)(count + sizeof(pkt_header_t)), rd);
	} else {
		if ((TAG_AR == cmd_tag) || (TAG_ENVIRONMENT == cmd_tag)) {
			((ar_ioctl_cmd_t *)buffer)->sub_cmd = (unsigned char)subtype;
		} else {
			((flp_ioctl_cmd_t *)buffer)->sub_cmd = subtype;
		}

		if (count >= (MAX_PKT_LENGTH - sizeof(flp_ioctl_cmd_t))) {
			pr_err("[%s]flp_ioctl_cmd_t count[%ld]err\n",__func__,count);
			return -EIO;
		}

		if (count)
			memcpy(buffer + sizeof(flp_ioctl_cmd_t), buf, count);

		((pkt_header_t *)buffer)->length = (unsigned short)(count + sizeof(unsigned int));
		ret = SEND_CMD_TO_SENSORHUB(buffer, (unsigned int)(count + sizeof(flp_ioctl_cmd_t)), rd);
	}
	if ((ret) || (rd->errno)) {
		pr_err("[%s]error[%d][%d]\n", __func__, ret, rd->errno);
		return -EBUSY;
	}
	return 0;
}

static int send_cmd_from_kernel_nolock(unsigned char cmd_tag, unsigned char cmd_type,
    unsigned int subtype, char  *buf, size_t count)
{
    char buffer[MAX_PKT_LENGTH] = {0};
    int ret = 0;
    /*init sending cmd*/
    ((pkt_header_t *)buffer)->tag = cmd_tag;
    ((pkt_header_t *)buffer)->resp = NO_RESP;
    ((pkt_header_t *)buffer)->cmd = cmd_type;
    printk(HISI_FLP_DEBUG "flp:%s : cmd_tag[0x%x] cmd[%d:0x%x]  count[%ld]\n",
        __func__, cmd_tag, cmd_type, subtype, count);

    if (CMD_CMN_CONFIG_REQ != cmd_type) {
	if (count >= (MAX_PKT_LENGTH - sizeof(pkt_header_t))) {
		pr_err("[%s]pkt_header_t count[%ld]err\n",__func__,count);
		return -EIO;
	}

	if (count)
		memcpy(buffer + sizeof(pkt_header_t), buf, (size_t)count);

	((pkt_header_t *)buffer)->length = (uint16_t)count;
	ret = SEND_CMD_TO_SENSORHUB_NOLOCK(buffer, (unsigned int)(count + sizeof(pkt_header_t)));
    } else {
        if ((TAG_AR == cmd_tag) || (TAG_ENVIRONMENT == cmd_tag)) {/*lint -e379*/
            ((ar_ioctl_cmd_t *)buffer)->sub_cmd = (unsigned char)subtype;
        } else {
            ((flp_ioctl_cmd_t *)buffer)->sub_cmd = subtype;
        }
	if (count >= (MAX_PKT_LENGTH - sizeof(flp_ioctl_cmd_t))) {
		pr_err("[%s]flp_ioctl_cmd_t count[%ld]err\n",__func__,count);
		return -EIO;
	}
	if (count)
		memcpy(buffer + sizeof(flp_ioctl_cmd_t), buf, (size_t)count);

        ((pkt_header_t *)buffer)->length = (uint16_t)(count + sizeof(unsigned int));
        ret = SEND_CMD_TO_SENSORHUB_NOLOCK(buffer, (unsigned int)(count + sizeof(flp_ioctl_cmd_t)));
    }
    if (ret) {
        printk(KERN_ERR "%s error\n", __func__);
    }
    return ret;
}
/*lint +e655*/
static int flp_genlink_checkin(flp_port_t *flp_port, unsigned int count, unsigned char cmd_type)
{
	if (!flp_port) {
		pr_err("[%s] flp_port NULL\n", __func__);
		return -EINVAL;
	}

	if(!flp_port->portid) {
		pr_err("[%s]no portid error\n", __func__);
		return -EBUSY;
	}

	if (((FLP_GENL_CMD_PDR_DATA == cmd_type) || (FLP_GENL_CMD_AR_DATA == cmd_type)
	||(FLP_GENL_CMD_ENV_DATA == cmd_type)) &&
	(!count)) {
		return -EFAULT;
	}

	return 0;

}
/*lint -e826 -e834 -e776*/
static int flp_generate_netlink_packet(flp_port_t *flp_port, char *buf,
        unsigned int count, unsigned char cmd_type)
{
	flp_data_buf_t *pdata = NULL ;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	void *msg_header;
	char *data ;
	int result;
	static unsigned int flp_event_seqnum = 0;

	result = flp_genlink_checkin(flp_port, count, cmd_type);
	if (result) {
		pr_err("[%s]flp_genlink_checkin[%d]\n", __func__, result);
		return result;
	}

	if (FLP_GENL_CMD_PDR_DATA == cmd_type) {
		pdata = &flp_port->pdr_buf;
	} else if (FLP_GENL_CMD_AR_DATA == cmd_type) {
		pdata = &flp_port->ar_buf;
	}else if (FLP_GENL_CMD_ENV_DATA == cmd_type) {
		pdata = &flp_port->env_buf;
	}

	skb = genlmsg_new((size_t)count, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	/* add the genetlink message header */
	msg_header = genlmsg_put(skb, 0, flp_event_seqnum++,
	&flp_genl_family, 0, cmd_type);
	if (!msg_header) {
		nlmsg_free(skb);
		return -ENOMEM;
	}

	/* fill the data */
	data = nla_reserve_nohdr(skb, (int)count);
	if (!data) {
		nlmsg_free(skb);
		return -EINVAL;
	}
	switch (cmd_type) {
	case FLP_GENL_CMD_PDR_DATA:
	case FLP_GENL_CMD_AR_DATA:
	case FLP_GENL_CMD_ENV_DATA:
		if (!pdata) {
			nlmsg_free(skb);
			printk(KERN_ERR "%s error\n", __func__);
			return -EINVAL;
		}
		if (count  > pdata->data_count) {
			count = pdata->data_count;
		}

		/*copy data to user buffer*/
		if ((pdata->read_index + count) >  pdata->buf_size) {
			memcpy(data, pdata->data_buf + pdata->read_index, (size_t)(pdata->buf_size - pdata->read_index));
			memcpy(data + pdata->buf_size - pdata->read_index,
			pdata->data_buf,
			(size_t)(count + pdata->read_index - pdata->buf_size));
		} else {
			memcpy(data, pdata->data_buf + pdata->read_index, (size_t)count);
		}
		pdata->read_index = (pdata->read_index + count)%pdata->buf_size;
		pdata->data_count -= count;
	break ;
	default:
		if (buf && count) {
			memcpy(data, buf, (size_t)count);
		}
	break ;
	};

	/*if aligned, just set real count*/
	nlh = (struct nlmsghdr *)((unsigned char *)msg_header - GENL_HDRLEN - NLMSG_HDRLEN);
	nlh->nlmsg_len = count + GENL_HDRLEN + NLMSG_HDRLEN;

	printk(HISI_FLP_DEBUG "%s 0x%x:%d:%d\n", __func__, flp_port->port_type, cmd_type, nlh->nlmsg_len);
	/* send unicast genetlink message */
	result = genlmsg_unicast(&init_net, skb, flp_port->portid);
	if (result) {
		printk(KERN_ERR "flp:Failed to send netlink event:%d", result);
	}

	return result;
}
/*lint -e845*/
static int  flp_pdr_stop_cmd(flp_port_t *flp_port, unsigned long arg)
{
	struct list_head    *pos;
	flp_port_t      *port;
	unsigned int delay = 0;

	printk(HISI_FLP_DEBUG "flp_stop_cmd pdr count[%d]--dalay[%d]\n", g_flp_dev.pdr_start_count, delay);
	if (!flp_port->pdr_buf.data_buf) {
		printk(KERN_ERR "Repeat stop is not permit \n");
		return -EPERM;
	}

	if (copy_from_user(&delay, (void *)arg, sizeof(unsigned int))) {
		printk(KERN_ERR "flp_ioctl copy_from_user error\n");
		return -EFAULT;
	}

	g_flp_dev.pdr_start_count-- ;

    flp_port->channel_type &= (~FLP_PDR_DATA);
    memset((void *)&flp_port->pdr_config, 0, sizeof(pdr_start_config_t));
    if (0 == g_flp_dev.pdr_start_count) {
        memset((void *)&g_flp_dev.pdr_config, 0, sizeof(pdr_start_config_t));
        delay = 0;
        send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ,
                CMD_FLP_PDR_STOP_REQ, (char *)&delay, sizeof(int));
        send_cmd_from_kernel(TAG_PDR, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
        g_flp_dev.pdr_cycle = 0;
        g_flp_dev.service_type &= ~FLP_PDR_DATA;
    } else if (g_flp_dev.pdr_start_count > 0) {
        list_for_each(pos, &g_flp_dev.list) {
            port = container_of(pos, flp_port_t, list);
            if ((port != flp_port) && (port->channel_type & FLP_PDR_DATA)) {
                memcpy((void *)&g_flp_dev.pdr_config, &port->pdr_config, sizeof(pdr_start_config_t));
                break;
            }
        }
        g_flp_dev.pdr_flush_config = FLP_IOCTL_PDR_STOP(0);
        send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ, CMD_FLP_PDR_FLUSH_REQ, NULL, (size_t)0);
        send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ, CMD_FLP_PDR_UPDATE_REQ,
             (char *)&g_flp_dev.pdr_config, sizeof(pdr_start_config_t));

    }

	kfree(flp_port->pdr_buf.data_buf);
	flp_port->pdr_buf.data_buf = NULL;
	return 0;
}
/*lint +e826 +e834 +e776 +e845*/
/*lint -e845*/

static int get_pdr_cfg(pdr_start_config_t *pdr_config, const char __user *buf, size_t len)
{
	if (len != sizeof(pdr_start_config_t)) {
		pr_err("[%s]len err [%lu]\n", __func__, len);
		return -EINVAL;
	}

	if (copy_from_user(pdr_config, buf, len)) {
		printk(KERN_ERR "flp_start_cmd copy_from_user error\n");
		return -EIO;
	}

	if (PDR_INTERVAL_MXN < pdr_config->report_interval ||PDR_INTERVAL_MIN > pdr_config->report_interval){
		pr_err("FLP[%s]interva err[%u]\n", __func__,pdr_config->report_interval);
		return -EINVAL;
	}

	if (PDR_DATA_MAX_COUNT < pdr_config->report_count || PDR_DATA_MIX_COUNT > pdr_config->report_count){
		pr_err("[%s]report_count err[%u]\n", __func__, pdr_config->report_count);
		return -EINVAL;
	}

	if (PDR_PRECISE_MXN < pdr_config->report_precise ||
		pdr_config->report_precise%PDR_PRECISE_MIN){
		pr_err("[%s]precise err[%u]\n", __func__, pdr_config->report_precise);
		return -EINVAL;
	}

	if (PDR_PRECISE_MIN > pdr_config->report_precise) {
		 pdr_config->report_precise = PDR_PRECISE_MIN;
	}

	if (pdr_config->report_interval <
		pdr_config->report_count * pdr_config->report_precise) {
		printk(KERN_ERR "flp_start_cmd error  line[%d]\n", __LINE__);
		return -EINVAL;
	}
	if (pdr_config->report_interval/pdr_config->report_precise >
		PDR_DATA_MAX_COUNT) {
		printk(KERN_ERR "flp_start_cmd error  line[%d]\n", __LINE__);
		return -EINVAL;
	}

	return 0;
}


/* why *cur use g_flp_dev.ar_dev_info.cfg, because cts open can be used directly.*/
static unsigned int  context_cfg_set_to_iomcu(unsigned int context_max,
	context_iomcu_cfg_t *cur, context_iomcu_cfg_t *old_set)
{
	unsigned int i;
	unsigned char *ultimate_data = (unsigned char *)cur->context_list;
	memset((void*)cur, 0 ,sizeof(context_iomcu_cfg_t));
	cur->report_interval = old_set->report_interval < 1?1:old_set->report_interval;
	for(i = 0;i < context_max; i++) {
		if(old_set->context_list[i].head.event_type) {
			memcpy((void*)ultimate_data, (void*)&old_set->context_list[i], (unsigned long)sizeof(ar_context_cfg_header_t));
			ultimate_data += sizeof(ar_context_cfg_header_t);
			if(old_set->context_list[i].head.len && old_set->context_list[i].head.len <= CONTEXT_PRIVATE_DATA_MAX) {
				memcpy((void*)ultimate_data, old_set->context_list[i].buf, (unsigned long)old_set->context_list[i].head.len);
				ultimate_data += old_set->context_list[i].head.len;
			}
			cur->context_num++;
		}
	}

	return (unsigned int)(ultimate_data - (unsigned char*)cur);
}

/*lint -e661 -e662 -e826*/
/*Multi-instance scenarios,different buff data can impact on business,
but the kernel does not pay attention to this matter,APP service deal with this thing*/
static void ar_multi_instance(flp_device_t*  flp_dev, context_iomcu_cfg_t *config)
{
	int count;
	struct list_head *pos;
	flp_port_t *port;
/*here is protected by g_flp_dev.lock in father function,is not a good method,
but too many global variables makes me confused, Fortunately most of processes in the flp driver is
a single configuration processes, we used g_flp_dev.lock just like BKL in kernel,
Must pay attention here ,if you want to call static function in new development.*/
	list_for_each(pos, &flp_dev->list) {
		port = container_of(pos, flp_port_t, list);
		if (port->channel_type & FLP_AR_DATA) {
			for (count= 0; count < AR_STATE_BUTT; count++) {
					config->context_list[count].head.event_type |=
					port->ar_config.context_list[count].head.event_type;
				}

			config->report_interval  =
			config->report_interval <= port->ar_config.report_interval ?
			config->report_interval:port->ar_config.report_interval;
		}
	}
}
/*lint -e715*/
static int ar_stop(unsigned int delay)
{
	int count;
	context_dev_info_t * devinfo = &g_flp_dev.ar_dev_info;

	if (!(FLP_AR_DATA & g_flp_dev.service_type)) {
		pr_info("[%s][%x]\n", __func__,g_flp_dev.service_type);
		return -EPERM;
	}

	if (devinfo->usr_cnt) {
		size_t len;
		context_iomcu_cfg_t *pcfg = (context_iomcu_cfg_t *)kzalloc(sizeof(context_iomcu_cfg_t), GFP_KERNEL);
		if (NULL == pcfg) {
			pr_err("[%s]kzalloc error\n", __func__);
			return -ENOMEM;
		}

		for (count = 0; count < AR_STATE_BUTT; count++) {
			pcfg->context_list[count].head.context = (unsigned int)count;
		}
		pcfg->report_interval = ~0;
		ar_multi_instance(&g_flp_dev, pcfg);
		len = context_cfg_set_to_iomcu(AR_STATE_BUTT, &devinfo->cfg, pcfg);
		kfree((void*)pcfg);
		if (0 == devinfo->cfg.context_num) {
			pr_err("[%s]context_cfg_set_to_iomcu context_num:0error\n", __func__);
			return -EINVAL;
		}
		send_cmd_from_kernel(TAG_AR, CMD_CMN_CONFIG_REQ, CMD_FLP_AR_START_REQ,
		(char *)&devinfo->cfg, len);
	} else {
		unsigned int delayed = 0;
		send_cmd_from_kernel(TAG_AR, CMD_CMN_CONFIG_REQ, CMD_FLP_AR_STOP_REQ, (char *)&delayed, sizeof(delayed));
		send_cmd_from_kernel(TAG_AR, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
		 g_flp_dev.service_type &= ~FLP_AR_DATA;
	}

	return 0;
}
/*lint +e661 +e662 +e826 +e715*/

static void data_buffer_exit(flp_data_buf_t *buf)
{
	if (buf->data_buf) {
		kfree((void*)buf->data_buf);
		buf->data_buf = NULL;
	}
}

static int  ar_stop_cmd(flp_port_t *flp_port, unsigned long arg)
{
	unsigned int delay = 0;
	int ret = 0;
	context_dev_info_t * devinfo = &g_flp_dev.ar_dev_info;

	mutex_lock(&g_flp_dev.lock);
	if (!(FLP_AR_DATA & flp_port->channel_type) || !(FLP_AR_DATA & g_flp_dev.service_type)) {
		pr_err("[%s]had stopped[%x][%x]\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		ret =  -EIO;
		goto AR_STOP_ERR;
	}

	if (copy_from_user(&delay, (void *)arg, sizeof(unsigned int))) {
		pr_err("[%s]delay copy_from_user error\n", __func__);
		ret =  -EFAULT;
		goto AR_STOP_ERR;
	}
	printk(HISI_FLP_DEBUG "[%s]delay[%u]\n", __func__, delay);
	memset((void *)&flp_port->ar_config, 0, sizeof(context_iomcu_cfg_t));
	devinfo->usr_cnt--;

	flp_port->channel_type &= (~FLP_AR_DATA);
	flp_port->ar_ver = AR_VER_0;
	ret = ar_stop(delay);
	if (ret) {
		pr_err("[%s][%d]ar_stop error\n", __func__, ret);
		ret = 0;/*Because the channel_type flag has been removed, the business has been closed for the application layer*/
	}

	data_buffer_exit(&flp_port->ar_buf);
AR_STOP_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}
#ifdef ENV_FEATURE
static int  env_stop_cmd(flp_port_t *flp_port, unsigned long arg)
{
	unsigned int delay = 0;
	int ret = 0;

	if (!(FLP_ENVIRONMENT & flp_port->channel_type) || !(FLP_ENVIRONMENT & g_flp_dev.service_type)) {
		pr_info("[%s][%x][%x]\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		return 0;
	}

	mutex_lock(&g_flp_dev.lock);
	if (copy_from_user(&delay, (void *)arg, sizeof(unsigned int))) {
		pr_err("[%s]delay copy_from_user error\n", __func__);
		ret =  -EFAULT;
		goto ENV_STOP_ERR;
	}

	memset((void *)&flp_port->env_config, 0, sizeof(context_iomcu_cfg_t));
	send_cmd_from_kernel(TAG_ENVIRONMENT, CMD_CMN_CONFIG_REQ, CMD_ENVIRONMENT_STOP_REQ, (char *)&delay, sizeof(int));
	mutex_unlock(&g_flp_dev.lock);
	return 0;
ENV_STOP_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}

static int  env_close_cmd(flp_port_t *flp_port)
{
	printk(HISI_FLP_DEBUG "[%s]\n", __func__);
	if (!(FLP_ENVIRONMENT & flp_port->channel_type) || !(FLP_ENVIRONMENT & g_flp_dev.service_type)) {
		pr_info("[%s][%x][%x]\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		return 0;
	}

	mutex_lock(&g_flp_dev.lock);
	memset((void *)&flp_port->env_config, 0, sizeof(context_iomcu_cfg_t));
	flp_port->channel_type &= (~FLP_ENVIRONMENT);
	send_cmd_from_kernel(TAG_ENVIRONMENT, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
	g_flp_dev.service_type &= ~FLP_ENVIRONMENT;
	data_buffer_exit(&flp_port->env_buf);
	mutex_unlock(&g_flp_dev.lock);
	return 0;
}
#endif
/*lint -e845*/
static int flp_set_port_tag(flp_port_t *flp_port, unsigned int cmd)
{
    switch (cmd & FLP_IOCTL_TAG_MASK) {
        case FLP_IOCTL_TAG_FLP:
            flp_port->port_type = FLP_TAG_FLP ;
            break ;
        case FLP_IOCTL_TAG_GPS:
            flp_port->port_type = FLP_TAG_GPS ;
            break ;
        case FLP_IOCTL_TAG_AR:
            flp_port->port_type = FLP_TAG_AR ;
            break ;
        default:
            return -EFAULT;
    }
	return 0 ;
}

static int  flp_pdr_start_cmd(flp_port_t *flp_port, const char __user *buf, size_t len, unsigned int cmd)
{
            int ret = 0;
            flp_set_port_tag(flp_port, cmd);
            if (flp_port->pdr_buf.data_buf) {
		printk(KERN_ERR "Restart is not permit \n");
		return -EPERM;
            }
            flp_port->total_count = 0;
            flp_port->pdr_buf.buf_size = sizeof(flp_pdr_data_t) * PDR_DATA_MAX_COUNT * 2;
            flp_port->pdr_buf.read_index = 0 ;
            flp_port->pdr_buf.write_index = 0;
            flp_port->pdr_buf.data_count = 0;
            flp_port->pdr_buf.data_buf = (char *) kmalloc((size_t)flp_port->pdr_buf.buf_size, GFP_KERNEL);
            if (!flp_port->pdr_buf.data_buf) {
		printk(KERN_ERR "flp_open no mem\n");
		return -ENOMEM;
            }

            if (!g_flp_dev.pdr_start_count) {
                g_flp_dev.pdr_last_utc = 0;
            }

	ret = get_pdr_cfg(&flp_port->pdr_config, buf, len);
	if (ret)
		return ret;

	/*differ app multi start*/
	if (g_flp_dev.pdr_start_count) {
		g_flp_dev.pdr_config.report_precise = PDR_PRECISE_MIN;
		g_flp_dev.pdr_config.report_interval =
		(unsigned int)calc_GCD(g_flp_dev.pdr_config.report_interval, flp_port->pdr_config.report_interval);
		g_flp_dev.pdr_config.report_count = PDR_DATA_MIX_COUNT;
		g_flp_dev.pdr_config.report_times = 0;

		flp_port->pdr_flush_config = TRUE;
		g_flp_dev.pdr_flush_config = FLP_IOCTL_PDR_START(0);
		flp_port->rate = flp_port->pdr_config.report_precise / g_flp_dev.pdr_config.report_precise ;
		flp_port->interval = flp_port->pdr_config.report_interval / g_flp_dev.pdr_config.report_precise ;
		send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ, CMD_FLP_PDR_FLUSH_REQ, NULL, (size_t)0);
		send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ, CMD_FLP_PDR_UPDATE_REQ,
		(char *)&g_flp_dev.pdr_config, sizeof(pdr_start_config_t));
	} else {
		struct read_info rd;
		memset((void*)&rd, 0,sizeof(struct read_info));
		memcpy(&g_flp_dev.pdr_config, &flp_port->pdr_config, sizeof(pdr_start_config_t));
		g_flp_dev.pdr_cycle = g_flp_dev.pdr_config.report_precise;
		g_flp_dev.pdr_config.report_times = 0;
		flp_port->rate = 1 ;
		flp_port->interval = flp_port->pdr_config.report_interval/g_flp_dev.pdr_config.report_precise ;
		ret = send_cmd_from_kernel_response(TAG_PDR, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0, &rd);
		if (ret) {
			pr_err("[%s]hub not support pdr[%d]\n", __func__, ret);
			goto PDR_START_ERR;
		}
		g_flp_dev.service_type |= FLP_PDR_DATA;
		send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ, CMD_FLP_PDR_START_REQ,
		(char *)&g_flp_dev.pdr_config, sizeof(pdr_start_config_t));
	}
	flp_port->nextid = 0;
	flp_port->aquiredid = 0;
	printk(KERN_ERR "flp[%u]:interval:%u,precise:%u,count:%u\n",
	flp_port->port_type, flp_port->pdr_config.report_interval,
	flp_port->pdr_config.report_precise, flp_port->pdr_config.report_count);
	g_flp_dev.pdr_start_count++ ;
	flp_port->channel_type |= FLP_PDR_DATA;
	return 0;
PDR_START_ERR:
	return ret;
}
/*lint +e845*/

/*lint -e838*/
static int data_buffer_init(flp_port_t *flp_port, flp_data_buf_t *buf, unsigned int cmd, unsigned int buf_sz)
{
	int ret = 0;
	ret = flp_set_port_tag(flp_port, cmd);
	if (ret) {
		pr_err("[%s] flp_set_port_tag err", __func__);
		return ret;
	}

	buf->buf_size = buf_sz;
	buf->read_index = 0 ;
	buf->write_index = 0;
	buf->data_count = 0;
	buf->data_buf = (char *)kzalloc((size_t)buf->buf_size, GFP_KERNEL);
	if (!buf->data_buf) {
		pr_err("[%s]data_buf kzalloc err\n", __func__);
		return -ENOMEM;
	}

	return ret;
}
/*lint +e838*/
/*devinfo->cfg.context_list :The order just like HAL layers,set of One app*/
/*config :flp_port->config,out parm, natural order ,Global group*/
static int create_flp_port_cfg(context_iomcu_cfg_t *config,
context_dev_info_t* devinfo, unsigned int context_max)
{
	unsigned int i;
	context_config_t  *plist = (context_config_t  *)devinfo->cfg.context_list;

	memset(config, 0, sizeof(context_iomcu_cfg_t));
	config->report_interval = devinfo->cfg.report_interval;
	for (i = 0; i < context_max; i++)
		config->context_list[i].head.context = i;

	for(i = 0; i < devinfo->cfg.context_num; i++,plist++) {
		if (plist->head.context >= context_max || plist->head.event_type >= AR_STATE_MAX ||plist->head.len > CONTEXT_PRIVATE_DATA_MAX) {
			pr_err("EPERM ERR c[%u]e[%u][%u]\n", plist->head.context,
			plist->head.event_type, plist->head.len);
			return -EPERM;
		}

		memcpy((void*)&config->context_list[plist->head.context], (void*)plist, sizeof(context_config_t));
		printk(HISI_FLP_DEBUG "c[%u]e[%u][%u]\n", plist->head.context,
			plist->head.event_type, plist->head.len);
	}
	return 0;
}
/*lint -e838 -e438*/
static int context_config(context_iomcu_cfg_t *config, context_dev_info_t* devinfo,
	unsigned long arg, unsigned int context_max)
{
	int ret = 0;
	context_hal_config_t hal_cfg_hdr;
	unsigned long usr_len;
	if (copy_from_user((void *)&hal_cfg_hdr, (void __user *)arg, sizeof(context_hal_config_t))) {
		pr_err("[%s] copy_from_user context_hal_config_t err\n", __func__);
		return -EIO;
	}

	if (0 == hal_cfg_hdr.context_num ||hal_cfg_hdr.context_num > context_max) {
		pr_err("[%s] num[%d]max[%d]copy_from_user context_num err\n", __func__,
			hal_cfg_hdr.context_num, context_max);
		return  -EINVAL;
	}

	memset((void*)&devinfo->cfg, 0, sizeof(context_iomcu_cfg_t));
	devinfo->cfg.report_interval = hal_cfg_hdr.report_interval;
	devinfo->cfg.context_num = hal_cfg_hdr.context_num;
	printk(HISI_FLP_DEBUG "[%s]interval[%u]\n", __func__, hal_cfg_hdr.report_interval);
	usr_len = sizeof(context_config_t) * hal_cfg_hdr.context_num;
	if (usr_len > sizeof(devinfo->cfg.context_list)) {
		pr_err("[%s] usr_len[%lu] bufsize[%lu]err\n", __func__,
			usr_len, sizeof(devinfo->cfg.context_list));
		return  -ENOMEM;
	}

	if (copy_from_user((void *)devinfo->cfg.context_list, (const void __user *)hal_cfg_hdr.context_addr, usr_len)) {
		pr_err("%s copy_from_user error context list\n", __func__);
		return -EDOM;
	}

	ret = create_flp_port_cfg(config, devinfo, context_max);
	if (ret)
		pr_err("%s create_flp_port_cfg error\n", __func__);
	return ret;
}
/*lint +e838 +e438*/
static int ar_config_to_iomcu(flp_port_t *flp_port)
{
	int ret = 0;
	context_dev_info_t* devinfo = &g_flp_dev.ar_dev_info;
	if (devinfo->usr_cnt) {
		context_iomcu_cfg_t *pcfg = (context_iomcu_cfg_t *)kzalloc(sizeof(context_iomcu_cfg_t), GFP_KERNEL);
		if (NULL == pcfg) {
			pr_err("[%s]kzalloc error\n", __func__);
			ret = -ENOMEM;
			goto CFG_IOMCU_FIN;
		}
		memcpy(pcfg, &flp_port->ar_config, sizeof(context_iomcu_cfg_t));
		ar_multi_instance(&g_flp_dev, pcfg);
		devinfo->cfg_sz = context_cfg_set_to_iomcu(AR_STATE_BUTT, &devinfo->cfg, pcfg);
		kfree((void*)pcfg);
		if (0 == devinfo->cfg.context_num) {
			pr_err("[%s]context_cfg_set_to_iomcu context_num:0error\n", __func__);
			ret = -ERANGE;
			goto CFG_IOMCU_FIN;
		}
		send_cmd_from_kernel(TAG_AR, CMD_CMN_CONFIG_REQ, CMD_FLP_AR_START_REQ,
		(char *)&devinfo->cfg, (size_t)devinfo->cfg_sz);
	} else {
		struct read_info rd;
		memset((void*)&rd, 0,sizeof(struct read_info));
		devinfo->cfg_sz = context_cfg_set_to_iomcu(AR_STATE_BUTT, &devinfo->cfg, &flp_port->ar_config);
		if (0 == devinfo->cfg.context_num) {
			pr_err("[%s]context_cfg_set_to_iomcu context_num:0error\n", __func__);
			ret = -EDOM;
			goto CFG_IOMCU_FIN;
		}

		ret = send_cmd_from_kernel_response(TAG_AR, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0, &rd);
		if (ret) {
			pr_err("[%s]hub not support context\n", __func__);
			goto CFG_IOMCU_FIN;
		}

		send_cmd_from_kernel(TAG_AR, CMD_CMN_CONFIG_REQ, CMD_FLP_AR_START_REQ,
		(char *)&devinfo->cfg, (size_t)devinfo->cfg_sz);
		g_flp_dev.service_type |= FLP_AR_DATA;
	}

	printk(HISI_FLP_DEBUG "[%s]interval[%u]\n", __func__, devinfo->cfg.report_interval);

	if (!(FLP_AR_DATA & flp_port->channel_type)) {
		devinfo->usr_cnt++;
		flp_port->channel_type |= FLP_AR_DATA;
	}

	return 0;
CFG_IOMCU_FIN:
	return ret;
}

static int ar_config_cmd(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	context_dev_info_t* devinfo = &g_flp_dev.ar_dev_info;

	if(AR_VER_1 == flp_port->ar_ver) {
		pr_err("[%s][%x] you are ver1\n", __func__, flp_port->ar_ver);
		return  -EDOM;
	}

	mutex_lock(&g_flp_dev.lock);
	if(!(FLP_AR_DATA & flp_port->channel_type)) {
		ret = data_buffer_init(flp_port, &flp_port->ar_buf, cmd,
			(unsigned int)(AR_STATE_BUTT * (sizeof(context_event_t) + CONTEXT_PRIVATE_DATA_MAX)));
		if (ret){
			pr_err("[%s]data_buffer_init err[%d]\n", __func__, ret);
			goto AR_CONFIG_ERR;
		}
	}

	ret = context_config(&flp_port->ar_config, devinfo, arg, AR_STATE_BUTT);
	if (ret) {
		pr_err("[%s]context_config err\n", __func__);
		goto AR_CONFIG_ERR;
	}

	ret = ar_config_to_iomcu(flp_port);
	if (ret) {
		pr_err("[%s]ar_config_to_iomcu err\n", __func__);
		goto AR_CONFIG_ERR;
	}

	flp_port->ar_ver = AR_VER_2;
	mutex_unlock(&g_flp_dev.lock);
	return 0;

AR_CONFIG_ERR:
	data_buffer_exit(&flp_port->ar_buf);
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}
/*lint -e838*/
#ifdef ENV_FEATURE
static int env_init_cmd(flp_port_t *flp_port, unsigned int cmd)
{
	int ret = 0;
	struct read_info rd;
	memset((void*)&rd, 0,sizeof(struct read_info));
	if (FLP_ENVIRONMENT & flp_port->channel_type || FLP_ENVIRONMENT & g_flp_dev.service_type) {/*Does not support multiple instances*/
		pr_err("[%s] [%x][%x]\n", __func__, flp_port->channel_type,  g_flp_dev.service_type);
		return -EINVAL;
	}

	mutex_lock(&g_flp_dev.lock);

	ret = data_buffer_init(flp_port, &flp_port->env_buf,cmd,
		(unsigned int)(AR_ENVIRONMENT_END * (sizeof(context_event_t) + CONTEXT_PRIVATE_DATA_MAX)));
	if (ret){
		pr_err("[%s]data_buffer_init err\n", __func__);
		goto ENV_INIT_ERR;
	}

	ret = send_cmd_from_kernel_response(TAG_ENVIRONMENT, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0, &rd);
	if (ret) {
		pr_err("[%s]hub not support env\n", __func__);
		goto ENV_INIT_ERR1;
	}
	flp_port->channel_type |= FLP_ENVIRONMENT;
	g_flp_dev.service_type |= FLP_ENVIRONMENT;
	mutex_unlock(&g_flp_dev.lock);
	return ret;
ENV_INIT_ERR1:
	data_buffer_exit(&flp_port->env_buf);
ENV_INIT_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}

static int env_config_cmd(flp_port_t *flp_port, unsigned long arg)
{
	int ret = 0;
	context_dev_info_t* devinfo = &g_flp_dev.env_dev_info;

	mutex_lock(&g_flp_dev.lock);

	if (!(FLP_ENVIRONMENT & flp_port->channel_type) || !(FLP_ENVIRONMENT & g_flp_dev.service_type)) {
		pr_err("[%s][%x][%x] err,you must init first\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		ret =  -EINVAL;
		goto ENV_CONFIG_ERR;
	}

	ret = context_config(&flp_port->env_config, devinfo, arg, AR_ENVIRONMENT_END);
	if (ret) {
		pr_err("[%s]context_config_update\n", __func__);
		ret = -ENODATA;
		goto ENV_CONFIG_ERR;
	}

	devinfo->cfg_sz = context_cfg_set_to_iomcu(AR_ENVIRONMENT_END, &devinfo->cfg, &flp_port->env_config);
	if (0 == devinfo->cfg.context_num) {
		pr_err("[%s]open with no context list\n", __func__);
		ret = -EPERM;
		goto ENV_CONFIG_ERR;
	}

	send_cmd_from_kernel(TAG_ENVIRONMENT, CMD_CMN_CONFIG_REQ, CMD_ENVIRONMENT_START_REQ,
	(char *)&devinfo->cfg, (size_t)devinfo->cfg_sz);
	printk(HISI_FLP_DEBUG "[%s]iomcu interval[%u]\n", __func__, devinfo->cfg.report_interval);
	mutex_unlock(&g_flp_dev.lock);
	return 0;
ENV_CONFIG_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}
/*lint +e838*/
#endif
/*lint -e845*/
static int  flp_pdr_update_cmd(flp_port_t *flp_port, const char __user *buf, size_t len)
{
	int ret;
	if (!(flp_port->channel_type & FLP_PDR_DATA)) {
		pr_err("FLP[%s] ERR: you must start first error\n", __func__);
		return -EINVAL;
	}

	ret = get_pdr_cfg(&flp_port->pdr_config, buf, len);
	if (ret)
		return ret;

    /*differ app multi start*/
    if (g_flp_dev.pdr_start_count > 1) {
        g_flp_dev.pdr_config.report_precise = PDR_PRECISE_MIN;
        g_flp_dev.pdr_config.report_interval =
            (unsigned int)calc_GCD(g_flp_dev.pdr_config.report_interval, flp_port->pdr_update_config.report_interval);
        g_flp_dev.pdr_config.report_count = PDR_DATA_MIX_COUNT;
        g_flp_dev.pdr_config.report_times = 0;
    }   else {
        memcpy(&g_flp_dev.pdr_config, &flp_port->pdr_update_config, sizeof(pdr_start_config_t));
        g_flp_dev.pdr_config.report_times = 0;
    }
    flp_port->pdr_flush_config = TRUE ;
    g_flp_dev.pdr_flush_config = FLP_IOCTL_PDR_UPDATE(0);
    send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ, CMD_FLP_PDR_FLUSH_REQ,
        NULL, (size_t)0);
    send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ, CMD_FLP_PDR_UPDATE_REQ,
        (char *)&g_flp_dev.pdr_config, sizeof(pdr_start_config_t));

    printk(KERN_ERR "flp[%u]:interval:%u,precise:%u,count:%u\n",
            flp_port->port_type, flp_port->pdr_update_config.report_interval,
            flp_port->pdr_update_config.report_precise, flp_port->pdr_update_config.report_count);
    return 0;
}
/*lint +e845*/

/*lint -e834 -e776*/
static void copy_data_to_buf(flp_data_buf_t *pdata, char *data,
        unsigned int len, unsigned int align)
{
    unsigned int deta;
    /*no enough space , just overwrite*/
    if ((pdata->data_count + len) > pdata->buf_size) {
        deta = pdata->data_count + len - pdata->buf_size;
        if (deta % align) {
            printk(KERN_ERR "copy_data_to_buf data not align\n");
            deta = (deta/align + 1)*align;
        }
        pdata->read_index = (pdata->read_index + deta)%pdata->buf_size;
    }
    /*copy data to flp pdr driver buffer*/
    if ((pdata->write_index + len) >  pdata->buf_size) {
        memcpy(pdata->data_buf + pdata->write_index , data, (size_t)(pdata->buf_size - pdata->write_index));
        memcpy(pdata->data_buf,
            data + pdata->buf_size - pdata->write_index,
            (size_t)(len + pdata->write_index - pdata->buf_size));
    } else {
        memcpy(pdata->data_buf + pdata->write_index, data , (size_t)len);
    }
    pdata->write_index = (pdata->write_index + len)%pdata->buf_size;
    pdata->data_count = (pdata->write_index - pdata->read_index + pdata->buf_size)%pdata->buf_size;
    /*if buf is full*/
    if (!pdata->data_count) {
        pdata->data_count = pdata->buf_size;
    }
}

static int get_pdr_notify_from_mcu(const pkt_header_t *head)
{
    flp_port_t *flp_port ;
    struct list_head    *pos;
    int *data = (int *) (head + 1);
    int ret = 0;

    if (CMD_FLP_PDR_UNRELIABLE_REQ == head->cmd) {
        mutex_lock(&g_flp_dev.lock);
        list_for_each(pos, &g_flp_dev.list) {
            flp_port = container_of(pos, flp_port_t, list);
            if (!(flp_port->channel_type&FLP_PDR_DATA)) {
                continue ;
            }

            if (*data < 2) {
                ret |= flp_generate_netlink_packet(flp_port, (char *)data,
                        (unsigned int)sizeof(int), FLP_GENL_CMD_PDR_UNRELIABLE);
            } else if ((2 == *data) && (flp_port->need_report)) {
                ret |= flp_generate_netlink_packet(flp_port, (char *)data,
                        (unsigned int)sizeof(int), FLP_GENL_CMD_PDR_UNRELIABLE);
            }
        }
        mutex_unlock(&g_flp_dev.lock);
    }
    return ret;
}
/*lint -e845 -e826*/
static void __get_pdr_data_from_mcu(flp_pdr_data_t *data, unsigned int count)
{
    flp_data_buf_t *pdata;
    flp_port_t *flp_port ;
    struct list_head    *pos;
    flp_pdr_data_t *pevent;
    flp_pdr_data_t *pinsert;
    int ret;

    /*pick up data from inputhub buf*/
    list_for_each(pos, &g_flp_dev.list) {
	flp_port = container_of(pos, flp_port_t, list);
	if (!(flp_port->channel_type&FLP_PDR_DATA))
		continue;

	if (0 == flp_port->pdr_config.report_count ||0 == g_flp_dev.pdr_config.report_precise ||0 == flp_port->rate) {
		pr_err("%s count [%u] precise [%u]rate[%u]error\n", __func__,
			flp_port->pdr_config.report_count, g_flp_dev.pdr_config.report_precise,
			flp_port->rate);
		continue;
	}

        /* if start pdr ever,just discard history data*/
        if ((FLP_IOCTL_PDR_START(0) == g_flp_dev.pdr_flush_config) &&
            (flp_port->pdr_flush_config)) {
            if (count < PDR_SINGLE_SEND_COUNT) {
                flp_port->pdr_flush_config = 0;
                memcpy(&flp_port->pdr_compensate, &g_flp_dev.pdr_compensate,
                    sizeof(compensate_data_t));
            }
            continue ;
        }
        pdata = &flp_port->pdr_buf ;
        while (flp_port->nextid < count) {
            pevent = data + flp_port->nextid;
            pinsert = (flp_pdr_data_t *)(pdata->data_buf + pdata->write_index);
            copy_data_to_buf(pdata, (char *)pevent,
                (unsigned int)sizeof(flp_pdr_data_t), (unsigned int)sizeof(flp_pdr_data_t));
            /*as multi port scene, need subtract original point*/
            if (flp_port->pdr_compensate.compensate_position_x ||
                flp_port->pdr_compensate.compensate_position_y ||
                flp_port->pdr_compensate.compensate_step) {
                pinsert->step_count -= flp_port->pdr_compensate.compensate_step;
                pinsert->relative_position_x -= flp_port->pdr_compensate.compensate_position_x;
                pinsert->relative_position_y -= flp_port->pdr_compensate.compensate_position_y;
                pinsert->migration_distance -= flp_port->pdr_compensate.compensate_distance;
            }
            flp_port->total_count++ ;
            flp_port->aquiredid++ ;
            if ((flp_port->interval ==  flp_port->pdr_config.report_count) ||
                (flp_port->aquiredid%flp_port->pdr_config.report_count)) {
                flp_port->nextid += flp_port->rate;
            } else {
                flp_port->nextid += flp_port->interval - (flp_port->pdr_config.report_count - 1) * flp_port->rate;
            }
        }


	flp_port->aquiredid = flp_port->aquiredid%flp_port->pdr_config.report_count;
        flp_port->nextid -= count ;
        /*if up to report condition , send packet to hal layer*/
        if ((0 == flp_port->total_count%flp_port->pdr_config.report_count) ||
            (flp_port->pdr_buf.data_count >=
            flp_port->pdr_config.report_count*sizeof(flp_pdr_data_t))) {
	ret = flp_generate_netlink_packet(flp_port, pdata->data_buf, pdata->data_count, FLP_GENL_CMD_PDR_DATA);
	if (ret){
		pr_err("%s netlink_packet error[%d]\n", __func__, ret);
		return;
	}
        }
        /*check if need update pickup parameter or not*/
        if ((g_flp_dev.pdr_flush_config) && (count < PDR_SINGLE_SEND_COUNT)) {
            if ((FLP_IOCTL_PDR_UPDATE(0) == g_flp_dev.pdr_flush_config) && (flp_port->pdr_flush_config)) {
                memcpy(&flp_port->pdr_config, &flp_port->pdr_update_config,
                    sizeof(pdr_start_config_t));
                flp_port->nextid = 0;
                flp_port->aquiredid = 0;
                flp_port->total_count = 0 ;
            } else {
	flp_port->nextid *=  (flp_port->pdr_config.report_precise/g_flp_dev.pdr_config.report_precise)/flp_port->rate;
            }

	flp_port->rate = flp_port->pdr_config.report_precise/g_flp_dev.pdr_config.report_precise ;
	flp_port->interval = flp_port->pdr_config.report_interval/g_flp_dev.pdr_config.report_precise ;
            /*as send update or flush command to the port, send the received packets immediately*/
            if (((FLP_IOCTL_PDR_UPDATE(0) == g_flp_dev.pdr_flush_config) ||
                (FLP_IOCTL_PDR_FLUSH(0) == g_flp_dev.pdr_flush_config)) &&
                (flp_port->pdr_flush_config)) {
		flp_port->pdr_flush_config = 0;
		ret = flp_generate_netlink_packet(flp_port, pdata->data_buf, pdata->data_count, FLP_GENL_CMD_PDR_DATA);
		if (ret){
			pr_err("%s netlink_packet error[%d]\n", __func__, ret);
			return;
		}
            }
        }
        printk(HISI_FLP_DEBUG "flp:%s port_type:%d: len:%d,%d\n", __func__,
                                        flp_port->port_type, flp_port->pdr_config.report_count, flp_port->pdr_buf.data_count);
    }
}

static int get_pdr_data_from_mcu(const pkt_header_t *head)
{
    flp_pdr_data_t *data = (flp_pdr_data_t *) (head + 1);
    unsigned int len = head->length;
    flp_pdr_data_t *pevent;
    unsigned int count;
    unsigned int j;

    /*check data lenghth is valid*/
    if (len%(sizeof(flp_pdr_data_t))) {
        printk(KERN_ERR "pkt len[%d] error\n", head->length);
        return -EFAULT;
    }

	mutex_lock(&g_flp_dev.lock);
	/*no port be opened ,just discard data*/
	if (list_empty(&g_flp_dev.list)) {
		printk(KERN_ERR "flp pdr no port be opened\n");
		mutex_unlock(&g_flp_dev.lock);
		return -EFAULT;
	}

    count = len/(sizeof(flp_pdr_data_t));
    pevent = data;
    /*start first time, get utc of start cmd */
    if (!g_flp_dev.pdr_last_utc) {
        /*lint -e647 -esym(647,*) */
        g_flp_dev.pdr_last_utc = ktime_get_real_seconds() - g_flp_dev.pdr_config.report_count * (g_flp_dev.pdr_cycle/1000);
        /*lint +e647 +esym(647,*) */
        FLP_DEBUG("flputc--first [%ld]\n", g_flp_dev.pdr_last_utc);
    }
    /*for support multi port,transfer one times; timer continue in one fifo transfer*/
    for (j = 0; j < count; j++) {
        pevent = data + j;
        g_flp_dev.pdr_last_utc += g_flp_dev.pdr_cycle/1000;
        FLP_DEBUG("flputc[%ld]: %ld\n", g_flp_dev.pdr_last_utc, pevent->msec);
        pevent->msec = (unsigned long)g_flp_dev.pdr_last_utc;
    }
    if ((j == count) && (count < PDR_SINGLE_SEND_COUNT)) {
        g_flp_dev.pdr_last_utc = ktime_get_real_seconds();
    }
    /*record last packet*/
    if(count > 0) {
        g_flp_dev.pdr_compensate.compensate_step = pevent->step_count;
        g_flp_dev.pdr_compensate.compensate_position_x = pevent->relative_position_x;
        g_flp_dev.pdr_compensate.compensate_position_y = pevent->relative_position_y;
        g_flp_dev.pdr_compensate.compensate_distance = pevent->migration_distance;
    }

    printk(HISI_FLP_DEBUG "flp:recv pkt len[%d]\n", len);

    __get_pdr_data_from_mcu(data, count);

	/*short package indecate history date sending complete*/
	if ((g_flp_dev.pdr_flush_config) && (count < PDR_SINGLE_SEND_COUNT)) {
		g_flp_dev.pdr_flush_config = 0;
		g_flp_dev.pdr_cycle = g_flp_dev.pdr_config.report_precise;
	}

	mutex_unlock(&g_flp_dev.lock);
	return (int)len;
}

static int get_ar_data_from_mcu(const pkt_header_t *head)
{
	unsigned int i, port_etype;
	flp_data_buf_t *pdata;
	const ar_data_req_t *pd = (const ar_data_req_t *)head;
	unsigned char *pcd;
	unsigned char *data_tail = (unsigned char *)head + sizeof(pkt_header_t) + head->length;
	context_event_t * pevent;
	int ret = 0;
	flp_port_t *flp_port;
	struct list_head *pos;

	if(0 == pd->context_num || AR_STATE_BUTT < pd->context_num) {
		pr_err("[%s] context_num [%d]err\n", __func__, pd->context_num);
		return -EBUSY;
	}

	mutex_lock(&g_flp_dev.lock);

	list_for_each(pos, &g_flp_dev.list) {
		flp_port = container_of(pos, flp_port_t, list);
		if (!(FLP_AR_DATA & flp_port->channel_type))
			continue;

		pdata = &flp_port->ar_buf;
		pcd = (unsigned char *)pd->context_event;
		for (i = 0; i < pd->context_num; i++) {
			if(pcd >= data_tail){
				pr_err("[%s] break[%d][%pK]\n", __func__, i, pcd);
				break;
			}
			pevent = (context_event_t *)pcd;
			if(AR_STATE_BUTT <= pevent->context) {
				pr_err("[%s]pevent->context[%d]too large\n", __func__, pevent->context);
				break;
			}
			port_etype = flp_port->ar_config.context_list[pevent->context].head.event_type;
			printk(HISI_FLP_DEBUG "[%s]num[%u]type[%u]ctt[%u]msec[%llu]len[%u][%u][%u][%u][%u]\n", __func__,
			pd->context_num, pevent->event_type, pevent->context, pevent->msec, pevent->buf_len,
			flp_port->port_type, flp_port->channel_type, flp_port->flushing,port_etype);
			if((port_etype & pevent->event_type) ||
				((FLP_AR_DATA & flp_port->flushing) && (!pevent->event_type))) {
				if (AR_VER_1 == flp_port->ar_ver) {
					copy_data_to_buf(pdata, (char *)pevent,
					(unsigned int)sizeof(ar_activity_event_t), (unsigned int)sizeof(ar_activity_event_t));
					printk(HISI_FLP_DEBUG"type[%u]ctx[%u]msec[%llu]len[%u]\n",
						pevent->event_type, pevent->context, pevent->msec, pevent->buf_len);
				}else if (pevent->buf_len <= CONTEXT_PRIVATE_DATA_MAX){
					copy_data_to_buf(pdata, (char *)pevent,
					(unsigned int)(sizeof(context_event_t) + pevent->buf_len), (unsigned int)1);
					printk(HISI_FLP_DEBUG"type[%u]ctx[%u]msec[%llu]len[%u]\n",
						pevent->event_type, pevent->context, pevent->msec, pevent->buf_len);
				}else {
					pr_err("[%s]type[%d]len[%d] private data too large\n", __func__,pevent->event_type,pevent->buf_len);
					break;
				}

				if(!pevent->event_type)
					flp_port->flushing &= ~FLP_AR_DATA;
			}

			pcd += sizeof(context_event_t) + pevent->buf_len;
		}

		flp_generate_netlink_packet(flp_port, pdata->data_buf,
		    pdata->data_count, FLP_GENL_CMD_AR_DATA);
	}

	mutex_unlock(&g_flp_dev.lock);
	return ret;
}

#ifdef ENV_FEATURE
static int get_env_data_from_mcu(const pkt_header_t *head)
{
	unsigned int i, port_etype;
	flp_data_buf_t *pdata;
	const ar_data_req_t *pd = (const ar_data_req_t *)head;
	unsigned char *pcd;
	unsigned char *data_tail = (unsigned char *)head + sizeof(pkt_header_t) + head->length;
	context_event_t * pevent;
	int ret = 0;
	flp_port_t *flp_port;
	struct list_head *pos;
	printk(HISI_FLP_DEBUG"[%s]\n", __func__);

	if(0 == pd->context_num || AR_ENVIRONMENT_END < pd->context_num) {
		pr_err("[%s] context_num [%d]err\n", __func__, pd->context_num);
		return -EBUSY;
	}

	printk(HISI_FLP_DEBUG"[%s]num[%d]\n", __func__, pd->context_num);

	mutex_lock(&g_flp_dev.lock);

	list_for_each(pos, &g_flp_dev.list) {
		flp_port = container_of(pos, flp_port_t, list);
		if (!(FLP_ENVIRONMENT & flp_port->channel_type))
			continue;

		pdata = &flp_port->env_buf;
		pcd = (unsigned char *)pd->context_event;
		for (i = 0; i < pd->context_num; i++) {
			if(pcd >= data_tail)
				break;
			pevent = (context_event_t *)pcd;
			if(AR_ENVIRONMENT_END <= pevent->context) {
				pr_err("[%s]pevent->context[%d]\n", __func__, pevent->context);
				break;
			}

			if (pevent->buf_len > CONTEXT_PRIVATE_DATA_MAX) {
				pr_err("[%s]type[%d]len[%d] private data too large\n", __func__,pevent->event_type,pevent->buf_len);
				break;
			}

			port_etype = flp_port->env_config.context_list[pevent->context].head.event_type;
			printk(HISI_FLP_DEBUG "[%s]num[%u]type[%u]ctt[%u]msec[%llu]len[%u][%u][%u][%u]\n", __func__,
			pd->context_num, pevent->event_type, pevent->context, pevent->msec, pevent->buf_len,
			flp_port->port_type, flp_port->channel_type,port_etype);
			if(port_etype & pevent->event_type)
				copy_data_to_buf(pdata, (char *)pevent,
					(unsigned int)(sizeof(context_event_t) + pevent->buf_len), (unsigned int)1);

			pcd += sizeof(context_event_t) + pevent->buf_len;
		}

		flp_generate_netlink_packet(flp_port, pdata->data_buf,
		    pdata->data_count, FLP_GENL_CMD_ENV_DATA);
		break;
	}

	mutex_unlock(&g_flp_dev.lock);
	return ret;
}
#endif
/*lint +e845 +e826*/
#ifndef CONTEXT_RESP_API
/*Must be reentrant*/
static int get_state_from_iomcu(const pkt_header_t *head, context_dev_info_t *devinfo)
{
	const ar_data_req_t *pd = (const ar_data_req_t *)head;
	unsigned int buflen_limit = (sizeof(context_event_t) + CONTEXT_PRIVATE_DATA_MAX)*GET_STATE_NUM_MAX + sizeof(unsigned int);
	printk(HISI_FLP_DEBUG"[%s]\n", __func__);
	if(0 == head->length || buflen_limit < head->length) {
		pr_err("[%s]length err[%d]buflen_limit[%d]\n", __func__, head->length, buflen_limit);
		return 0;
	}

	if (0 == pd->context_num || GET_STATE_NUM_MAX < pd->context_num) {
		pr_err("[%s]context_num err[%d]\n", __func__, pd->context_num);
		return 0;
	}

	if (list_empty(&g_flp_dev.list)) {
		pr_err("[%s] no app\n", __func__);
		return 0;
	}
	devinfo->state_buf_len = (head->length - sizeof(unsigned int));
	devinfo->state_buf_len = STATE_KERNEL_BUF_MAX < devinfo->state_buf_len?
		STATE_KERNEL_BUF_MAX:devinfo->state_buf_len;
	memcpy((void*)devinfo->state_buf, (void*)pd->context_event, devinfo->state_buf_len);
	complete(&devinfo->state_cplt);
	return 0;
}

static int get_ar_state_from_iomcu(const pkt_header_t *head)
{
	return get_state_from_iomcu(head, &g_flp_dev.ar_dev_info);
}

static int get_env_state_from_iomcu(const pkt_header_t *head)
{
	return get_state_from_iomcu(head, &g_flp_dev.env_dev_info);
}
#endif
/*lint -e826*/
#ifdef GEOFENCE_BATCH_FEATURE
static int get_common_data_from_mcu(const pkt_header_t *head)
{
    unsigned int len = head->length;
    char *data = (char  *) (head + 1);
    flp_port_t *flp_port ;
    struct list_head    *pos;

    printk(HISI_FLP_DEBUG "flp:%s cmd:%d: len:%d\n", __func__, head->cmd, len);
    mutex_lock(&g_flp_dev.lock);
    list_for_each(pos, &g_flp_dev.list) {
        flp_port = container_of(pos, flp_port_t, list);
        switch (head->cmd)  {
            case CMD_FLP_LOCATION_UPDATE_REQ:
                if ((FLP_TAG_FLP == flp_port->port_type) && (flp_port->channel_type&FLP_BATCHING)) {
                    flp_generate_netlink_packet(flp_port, data,
                        len, FLP_GENL_CMD_GNSS_LOCATION);
                }
                break;
            case CMD_FLP_GEOF_TRANSITION_REQ:
                if ((FLP_TAG_FLP == flp_port->port_type) && (flp_port->channel_type&FLP_GEOFENCE)) {
                    flp_generate_netlink_packet(flp_port, data,
                        len, FLP_GENL_CMD_GEOFENCE_TRANSITION);
                }
                break;
            case CMD_FLP_GEOF_MONITOR_STATUS_REQ:
                if ((FLP_TAG_FLP == flp_port->port_type) && (flp_port->channel_type&FLP_GEOFENCE)) {
                    flp_generate_netlink_packet(flp_port, data,
                        len, FLP_GENL_CMD_GEOFENCE_MONITOR);
                }
                break;
            case CMD_FLP_RESET_RESP:
                if (FLP_TAG_FLP == flp_port->port_type) {
                    flp_generate_netlink_packet(flp_port, data, (unsigned int)sizeof(unsigned int), FLP_GENL_CMD_IOMCU_RESET);
                }
                break;
            default:
                printk(KERN_ERR "flp:%s cmd[0x%x] error\n", __func__, head->cmd);
                mutex_unlock(&g_flp_dev.lock);
                return -EFAULT;
        }
    }
    mutex_unlock(&g_flp_dev.lock);
    return (int)len;
}
#endif

#ifdef CONFIG_IOM3_RECOVERY
static void  flp_service_recovery(void)
{
	flp_port_t *flp_port;
	unsigned int flag = 0;
#ifdef GEOFENCE_BATCH_FEATURE
	unsigned int response = FLP_IOMCU_RESET;
#endif
	struct list_head    *pos;
	list_for_each(pos, &g_flp_dev.list) {
		flp_port = container_of(pos, flp_port_t, list);
		if ((flp_port->channel_type&FLP_PDR_DATA) &&
		!(flag & FLP_PDR_DATA)) {
			send_cmd_from_kernel_nolock(TAG_PDR, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0);
			send_cmd_from_kernel_nolock(TAG_PDR, CMD_CMN_CONFIG_REQ, CMD_FLP_PDR_START_REQ,
			(char *)&g_flp_dev.pdr_config, sizeof(pdr_start_config_t));
			flag |= FLP_PDR_DATA;
		}
		if ((flp_port->channel_type & FLP_AR_DATA) &&
		!(flag & FLP_AR_DATA)) {
			send_cmd_from_kernel_nolock(TAG_AR, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0);
			send_cmd_from_kernel_nolock(TAG_AR, CMD_CMN_CONFIG_REQ, CMD_FLP_AR_START_REQ,
			(char *)&g_flp_dev.ar_dev_info.cfg, (size_t)g_flp_dev.ar_dev_info.cfg_sz);
			flag |= FLP_AR_DATA;
		}
#ifdef ENV_FEATURE
		if ((flp_port->channel_type & FLP_ENVIRONMENT) &&
		!(flag & FLP_ENVIRONMENT)) {
			send_cmd_from_kernel_nolock(TAG_ENVIRONMENT, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0);
			send_cmd_from_kernel_nolock(TAG_ENVIRONMENT, CMD_CMN_CONFIG_REQ, CMD_ENVIRONMENT_START_REQ,
			(char *)&g_flp_dev.env_dev_info.cfg, (size_t)g_flp_dev.env_dev_info.cfg_sz);
			flag |= FLP_ENVIRONMENT;
		}
#endif
#ifdef GEOFENCE_BATCH_FEATURE
		if ((FLP_TAG_FLP == flp_port->port_type) && ((flp_port->channel_type & FLP_BATCHING) ||
		(flp_port->channel_type & FLP_GEOFENCE))) {
			flp_generate_netlink_packet(flp_port, (char *)&response, (unsigned int)sizeof(unsigned int), FLP_GENL_CMD_IOMCU_RESET);
		}
#endif
	}
}
/*lint -e715*/
static int flp_notifier(struct notifier_block *nb,
            unsigned long action, void *data)
{
    switch (action) {
        case IOM3_RECOVERY_3RD_DOING:
            flp_service_recovery();
            break;
        default:
            printk(KERN_ERR "register_iom3_recovery_notifier err\n");
            break;
    }
    return 0;
}
/*lint +e715*/
extern int register_iom3_recovery_notifier(struct notifier_block *nb);
static struct notifier_block sensor_reboot_notify = {
    .notifier_call = flp_notifier,
    .priority = -1,
};
#endif

static void flp_timerout_work(struct work_struct *wk)
{
    flp_port_t *flp_port  = container_of(wk, flp_port_t, work);
    flp_generate_netlink_packet(flp_port, NULL, 0, (unsigned char)flp_port->work_para);
}
static void flp_sleep_timeout(unsigned long data)
{
    flp_port_t *flp_port = (flp_port_t *)data;
    printk(KERN_INFO "flp_sleep_timeout \n");
    if (flp_port) {
        flp_port->work_para = FLP_GENL_CMD_NOTIFY_TIMEROUT;
        queue_work(system_power_efficient_wq, &flp_port->work);
        if (flp_port->need_hold_wlock) {
            wake_lock_timeout(&flp_port->wlock, (long)(2 * HZ));
        }
    }
    return ;
}
void flp_port_resume(void)
{
    struct list_head    *pos;
    flp_port_t      *flp_port;

    mutex_lock(&g_flp_dev.lock);
    list_for_each(pos, &g_flp_dev.list) {
        flp_port = container_of(pos, flp_port_t, list);
        if (flp_port->need_awake) {
            flp_port->work_para = FLP_GENL_CMD_AWAKE_RET;
            queue_work(system_power_efficient_wq, &flp_port->work);
        }
    }
    mutex_unlock(&g_flp_dev.lock);
}

static bool flp_check_cmd(unsigned int cmd, int type)
{
	switch (type) {
	case FLP_PDR_DATA:
		if (((cmd & FLP_IOCTL_TAG_MASK) == FLP_IOCTL_TAG_FLP) ||
		((cmd & FLP_IOCTL_TAG_MASK) == FLP_IOCTL_TAG_GPS)) {
			return TRUE;
		}
	break ;
	case FLP_AR_DATA:
		if (((cmd & FLP_IOCTL_TAG_MASK) == FLP_IOCTL_TAG_FLP) ||
		((cmd & FLP_IOCTL_TAG_MASK) == FLP_IOCTL_TAG_GPS) ||
		((cmd & FLP_IOCTL_TAG_MASK) == FLP_IOCTL_TAG_AR)) {
			return TRUE;
		}
	break ;
#ifdef GEOFENCE_BATCH_FEATURE
	case FLP_GEOFENCE:
	case FLP_BATCHING:
		if ((cmd & FLP_IOCTL_TAG_MASK) == FLP_IOCTL_TAG_FLP) {
			return TRUE;
		}
	break ;
#endif
	case FLP_ENVIRONMENT:
		if ((cmd & FLP_IOCTL_TAG_MASK) == FLP_IOCTL_TAG_AR)
			return TRUE;
	break;
	default :
	break ;
	}
	return 0;
}
/*lint -e845 */
static int  flp_pdr_flush(flp_port_t *flp_port)
{
	if (!(flp_port->channel_type & FLP_PDR_DATA)) {
		pr_err("FLP[%s] ERR: you must start first error\n", __func__);
		return -EINVAL;
	}
	g_flp_dev.pdr_flush_config = FLP_IOCTL_PDR_FLUSH(0);
	flp_port->pdr_flush_config = TRUE;

	return send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ,
	CMD_FLP_PDR_FLUSH_REQ, NULL, 0);

}
/*lint +e845 */
static int flp_pdr_step(flp_port_t *flp_port, unsigned long arg)
 {
	if (!(flp_port->channel_type & FLP_PDR_DATA)) {
		pr_err("FLP[%s] ERR: you must start first error\n", __func__);
		return -EINVAL;
	}
	flp_port->need_report = TRUE;
	return send_cmd_from_user(TAG_PDR, CMD_CMN_CONFIG_REQ, CMD_FLP_PDR_STEPCFG_REQ, (char __user *)arg, sizeof(step_report_t));
}

/*lint -e845 -e747 -e712*/
static int flp_pdr_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	mutex_lock(&g_flp_dev.lock);
	switch (cmd & FLP_IOCTL_CMD_MASK) {
	case FLP_IOCTL_PDR_START(0):
		ret = flp_pdr_start_cmd(flp_port, (char __user *)arg, sizeof(pdr_start_config_t), cmd);
	break;
	case FLP_IOCTL_PDR_STOP(0):
		ret = flp_pdr_stop_cmd(flp_port, arg);
	break;
	case FLP_IOCTL_PDR_UPDATE(0):
		ret = flp_pdr_update_cmd(flp_port, (char __user *)arg, sizeof(pdr_start_config_t));
	break;
	case FLP_IOCTL_PDR_FLUSH(0):
		ret = flp_pdr_flush(flp_port);
	break;
	case FLP_IOCTL_PDR_STEP_CFG(0):
		ret = flp_pdr_step(flp_port, arg);
	break;
	default:
		printk(KERN_ERR "flp_pdr_ioctl input cmd[0x%x] error\n", cmd);
		mutex_unlock(&g_flp_dev.lock);
		return -EFAULT;
	}
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}
/*lint +e845 +e747 +e712*/
#ifdef CONTEXT_RESP_API
/*lint -e838*/
static int ar_state_v1_cmd(flp_port_t *flp_port, unsigned long arg)
{
	struct read_info rd;/*rd.data is only usr data*/
	int ret = 0;
	ar_state_t* date_report;
	memset((void*)&rd, 0,sizeof(struct read_info));
	if (!(FLP_AR_DATA & flp_port->channel_type) || !(FLP_AR_DATA & g_flp_dev.service_type)) {
		pr_err("[%s]you must first start[%x][%x]\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		return -EFAULT;
	}

	if(AR_VER_1 != flp_port->ar_ver) {
		pr_err("[%s][%x] you are not ver1\n", __func__, flp_port->ar_ver);
		return  -EDOM;
	}

	mutex_lock(&g_flp_dev.lock);

	ret = send_cmd_from_kernel_response(TAG_AR, CMD_CMN_CONFIG_REQ,
		CMD_FLP_AR_GET_STATE_REQ, NULL, (size_t)0, &rd);
	if (ret) {
		pr_err("[%s]send_cmd_from_kernel_response err[%d]\n", __func__, rd.errno);
		goto AR_STATE_ERR;
	}

	date_report = (ar_state_t*)rd.data;
	if (0 == date_report->context_num) {
		pr_err("[%s]context_num err[%d]\n", __func__, date_report->context_num);
		ret = -EINVAL;
		goto AR_STATE_ERR;
	}

	if (copy_to_user((void __user *)arg, (const void *)&date_report->context_event[0].context, sizeof(unsigned int))) {
		pr_err("[%s]copy_to_user ERR\n", __func__);
		ret = -ENOTDIR;
		goto AR_STATE_ERR;
	}

	printk(HISI_FLP_DEBUG "[%s]CONTEXT[0x%x]\n", __func__, date_report->context_event[0].context);

	mutex_unlock(&g_flp_dev.lock);
	return ret;
AR_STATE_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}
/*lint +e838*/
/*lint -e737*/
static int state_v2_cmd(struct read_info *rd, unsigned long arg)
{
	int ret = 0;
	unsigned long  usr_len;
	ar_state_t* date_report;
	unsigned int buflen_limit = (sizeof(context_event_t) + CONTEXT_PRIVATE_DATA_MAX)*GET_STATE_NUM_MAX;

	usr_len = rd->data_length - sizeof(unsigned int);
	if (0 == usr_len || buflen_limit < usr_len){
		pr_err("[%s]length err[%lu]buflen_limit[%u]\n", __func__, usr_len, buflen_limit);
		ret = -EINVAL;
		goto STATE_V2_ERR;
	}

	date_report = (ar_state_t*)rd->data;
	if (0 == date_report->context_num || GET_STATE_NUM_MAX < date_report->context_num) {
		pr_err("[%s]context_num err[%d]\n", __func__, date_report->context_num);
		ret = -EPERM;
		goto STATE_V2_ERR;
	}

	if (copy_to_user((void *)arg, date_report->context_event, usr_len)) {
		pr_err("[%s][STATE]copy_to_user ERR\n", __func__);
		ret = -EFAULT;
		goto STATE_V2_ERR;
	}

	return (int)usr_len;
STATE_V2_ERR:
	return ret;
}
/*lint +e737*/
static int ar_state_v2_cmd(flp_port_t *flp_port, unsigned long arg)
{
	int ret = 0;
	struct read_info rd;

	mutex_lock(&g_flp_dev.lock);
	if (!(FLP_AR_DATA & flp_port->channel_type) || !(FLP_AR_DATA & g_flp_dev.service_type)) {
		pr_err("[%s][%x][%x]\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		goto AR_STATE_V2_ERR;
	}

	if(AR_VER_1 == flp_port->ar_ver) {
		pr_err("[%s][%x] you are  ver1\n", __func__, flp_port->ar_ver);
		goto AR_STATE_V2_ERR;
	}

	memset((void*)&rd, 0,sizeof(struct read_info));
	ret = send_cmd_from_kernel_response(TAG_AR, CMD_CMN_CONFIG_REQ,
		CMD_FLP_AR_GET_STATE_REQ, NULL, (size_t)0, &rd);
	if (ret) {
		pr_err("[%s]send_cmd_from_kernel_response err[%d]\n", __func__, rd.errno);
		goto AR_STATE_V2_ERR;
	}

	ret = state_v2_cmd(&rd, arg);
	if (ret < 0) {
		pr_err("[%s]state_v2_cmd err[%d]\n", __func__, ret);
		goto AR_STATE_V2_ERR;
	}

	mutex_unlock(&g_flp_dev.lock);
	return ret;
AR_STATE_V2_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}
#ifdef ENV_FEATURE
static int env_state_cmd(flp_port_t *flp_port, unsigned long arg)
{
	struct read_info rd;
	int ret = 0;

	mutex_lock(&g_flp_dev.lock);

	if (!(FLP_ENVIRONMENT & flp_port->channel_type) || !(FLP_ENVIRONMENT & g_flp_dev.service_type)) {
		pr_err("[%s][%x][%x]\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		ret =  -EFAULT;
		goto ENV_STATE_ERR;
	}

	memset((void*)&rd, 0,sizeof(struct read_info));
	ret = send_cmd_from_kernel_response(TAG_ENVIRONMENT, CMD_CMN_CONFIG_REQ,
		CMD_ENVIRONMENT_GET_STATE_REQ, NULL, (size_t)0, &rd);
	if (ret) {
		pr_err("[%s]send_cmd_from_kernel_response err[%d]\n", __func__, rd.errno);
		goto ENV_STATE_ERR;
	}

	ret = state_v2_cmd(&rd, arg);
	if (ret < 0) {
		pr_err("[%s]state_v2_cmd err[%d]\n", __func__, ret);
		goto ENV_STATE_ERR;
	}

ENV_STATE_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}
#endif
#else

static int ar_state_cmd(flp_port_t *flp_port, ar_ver_e ar_ver, unsigned long arg)
{
	int ret = 0;
	context_dev_info_t *devinfo = &g_flp_dev.ar_dev_info;

	if (!(FLP_AR_DATA & flp_port->channel_type) || !(FLP_AR_DATA & g_flp_dev.service_type)) {
		pr_err("[%s]you must first start[%x][%x]\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		return -EFAULT;
	}

	mutex_lock(&g_flp_dev.lock);
	reinit_completion(&devinfo->state_cplt);
	/*send_cmd_from_kernel_response buf size limit 128bytes*/
	ret = send_cmd_from_kernel(TAG_AR, CMD_CMN_CONFIG_REQ,
		CMD_FLP_AR_STATE_REQ, NULL, (size_t)0);
	if (ret) {
		pr_err("[%s]send_cmd_from_kernel err[%d]\n", __func__, ret);
		goto AR_STATE_ERR;
	}

	if(!wait_for_completion_timeout(&devinfo->state_cplt, msecs_to_jiffies(2000))) {
		pr_err("[%s]wait_for_completion_timeout err\n", __func__);
		ret = -EACCES;
		goto AR_STATE_ERR;
	}

	if (AR_VER_1 == ar_ver) {
		context_event_t *event = (context_event_t *)devinfo->state_buf;
		if (copy_to_user((void __user *)arg, (const void *)&event[0].context, sizeof(unsigned int))) {
			pr_err("[%s]copy_to_user ERR\n", __func__);
			ret = -EIO;
			goto AR_STATE_ERR;
		}

		printk(HISI_FLP_DEBUG "[%s]CONTEXT[0x%x]\n", __func__, event[0].context);
		ret = 0;

	} else if (AR_VER_2 == ar_ver) {
		if (copy_to_user((void *)arg, devinfo->state_buf, (unsigned long)devinfo->state_buf_len)) {
			pr_err("[%s][STATE]copy_to_user ERR\n", __func__);
			ret = -EFAULT;
			goto AR_STATE_ERR;
		}

		ret = devinfo->state_buf_len;
	}

	mutex_unlock(&g_flp_dev.lock);
	return ret;
AR_STATE_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}

static int ar_state_v1_cmd(flp_port_t *flp_port, unsigned long arg)
{
	if(AR_VER_1 != flp_port->ar_ver) {
		pr_err("[%s][%x] you are not ver1\n", __func__, flp_port->ar_ver);
		return  -EDOM;
	}

	return ar_state_cmd(flp_port, 1, arg);
}

static int ar_state_v2_cmd(flp_port_t *flp_port, unsigned long arg)
{
	if(AR_VER_1 == flp_port->ar_ver) {
		pr_err("[%s][%x] you are  ver1\n", __func__, flp_port->ar_ver);
		return  -EDOM;
	}

	return ar_state_cmd(flp_port, 2, arg);
}
#ifdef ENV_FEATURE
static int env_state_cmd(flp_port_t *flp_port, unsigned long arg)
{
	int ret = 0;
	context_dev_info_t *devinfo = &g_flp_dev.env_dev_info;

	if (!(FLP_ENVIRONMENT & flp_port->channel_type) || !(FLP_ENVIRONMENT & g_flp_dev.service_type)) {
		pr_err("[%s][%x][%x]\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		return -EPERM;
	}

	mutex_lock(&g_flp_dev.lock);
	reinit_completion(&devinfo->state_cplt);
	/*send_cmd_from_kernel_response buf size limit 128bytes*/
	ret = send_cmd_from_kernel(TAG_ENVIRONMENT, CMD_CMN_CONFIG_REQ,
		CMD_ENVIRONMENT_GET_STATE_REQ, NULL, (size_t)0);
	if (ret) {
		pr_err("[%s]send_cmd_from_kernel err[%d]\n", __func__, ret);
		goto ENV_STATE_ERR;
	}

	if(!wait_for_completion_timeout(&devinfo->state_cplt, msecs_to_jiffies(2000))) {
		pr_err("[%s]wait_for_completion_timeout err\n", __func__);
		ret = -EFAULT;
		goto ENV_STATE_ERR;
	}

	if (copy_to_user((void *)arg, devinfo->state_buf, (unsigned long)devinfo->state_buf_len)) {
		pr_err("[%s][STATE]copy_to_user ERR\n", __func__);
		ret = -EACCES;
		goto ENV_STATE_ERR;
	}

	mutex_unlock(&g_flp_dev.lock);
	return devinfo->state_buf_len;

ENV_STATE_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}
#endif
#endif


static int ar_flush_cmd(flp_port_t *flp_port)
{
	int ret = 0;
	mutex_lock(&g_flp_dev.lock);
	if (!(FLP_AR_DATA & flp_port->channel_type) || !(FLP_AR_DATA & g_flp_dev.service_type)) {
		pr_err("[%s]you must first start[%x][%x]\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		ret = -EPERM;
		goto AR_FLUSH_ERR;
	}

	ret = send_cmd_from_kernel(TAG_AR, CMD_CMN_CONFIG_REQ,
		CMD_FLP_AR_FLUSH_REQ, NULL, (size_t)0);
	if (ret) {
		pr_err("[%s]send_cmd_from_kernel ERR[%d]\n", __func__, ret);
		ret = -EFAULT;
		goto AR_FLUSH_ERR;
	}

	flp_port->flushing |= FLP_AR_DATA;
AR_FLUSH_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}

static int  ar_config_v1(flp_port_t *flp_port, const char __user *buf, unsigned int cmd)
{
	unsigned int i;
	unsigned long usr_len;
	int ret = 0;
	ar_start_hal_config_t hal_config;
	ar_activity_config_t    activity_list[AR_STATE_BUTT];
	context_dev_info_t* devinfo = &g_flp_dev.ar_dev_info;

	mutex_lock(&g_flp_dev.lock);

	if(!(FLP_AR_DATA & flp_port->channel_type)) {
		ret = data_buffer_init(flp_port, &flp_port->ar_buf, cmd,
			(unsigned int)(AR_STATE_BUTT * (sizeof(context_event_t) + CONTEXT_PRIVATE_DATA_MAX)));
		if (ret){
			pr_err("[%s]data_buffer_init err\n", __func__);
			ret =  -ENOMEM;
			goto AR_CFG_V1_ERR0;
		}
	}

	if (copy_from_user((void *)&hal_config, buf, sizeof(ar_start_hal_config_t))){
		pr_err("%s copy_to_user error\n", __func__);
		ret  = -EIO;
		goto AR_CFG_V1_ERR;
	}

	if (0 == hal_config.event_num ||hal_config.event_num > AR_STATE_BUTT) {
		pr_err("[%s]event_num [%u]error\n", __func__, hal_config.event_num);
		ret  = -EINVAL;
		goto AR_CFG_V1_ERR;
	}

	usr_len = sizeof(ar_activity_config_t) * hal_config.event_num;
	if (usr_len > sizeof(activity_list)) {
		pr_err("[%s]usrlen[%lu] bufsize[%lu]error\n", __func__, usr_len, sizeof(activity_list));
		ret  = -EFAULT;
		goto AR_CFG_V1_ERR;
	}
	memset((void*)activity_list, 0, sizeof(activity_list));
	if (copy_from_user((void *)activity_list, (const void __user *)hal_config.pevent, usr_len)) {
			pr_err("%s copy_to_user error activity_list\n", __func__);
		ret = -EDOM;
		goto AR_CFG_V1_ERR;
	}

	memset((void*)&devinfo->cfg, 0, sizeof(context_iomcu_cfg_t));
	devinfo->cfg.report_interval = hal_config.report_interval;
	devinfo->cfg.context_num = hal_config.event_num;
	for(i = 0;i<hal_config.event_num;i++) {
		devinfo->cfg.context_list[i].head.context = activity_list[i].activity;
		devinfo->cfg.context_list[i].head.event_type = activity_list[i].event_type;
		devinfo->cfg.context_list[i].head.len = 0;
	}

	ret = create_flp_port_cfg(&flp_port->ar_config, devinfo, AR_STATE_BUTT);
	if (ret) {
		pr_err("%s create_flp_port_cfg error\n", __func__);
		ret = -EDOM;
		goto AR_CFG_V1_ERR;
	}

	ret = ar_config_to_iomcu(flp_port);
	if (ret) {
		pr_err("[%s]ar_config_to_iomcu err\n", __func__);
		goto AR_CFG_V1_ERR;
	}

	flp_port->ar_ver = AR_VER_1;
	mutex_unlock(&g_flp_dev.lock);
	return 0;
AR_CFG_V1_ERR:
	data_buffer_exit(&flp_port->ar_buf);
AR_CFG_V1_ERR0:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}

static int  ar_start_cmd(flp_port_t *flp_port, const char __user *buf, unsigned int cmd)
{
	if (FLP_AR_DATA & flp_port->channel_type) {
		pr_err("[%s]you had start[0x%x]\n", __func__, flp_port->channel_type);
		return  -EFAULT;
	}
	return ar_config_v1(flp_port, buf, cmd);
}

static int  ar_update_cmd(flp_port_t *flp_port, const char __user *buf, unsigned int cmd)
{
	if (!(FLP_AR_DATA & flp_port->channel_type) || !(FLP_AR_DATA & g_flp_dev.service_type)) {
		pr_err("[%s][%x][%x]you must start first\n", __func__, flp_port->channel_type, g_flp_dev.service_type);
		return  -EFAULT;
	}

	if(AR_VER_1 != flp_port->ar_ver) {
		pr_err("[%s][%x] you are not ver1\n", __func__, flp_port->ar_ver);
		return  -EDOM;
	}

	return ar_config_v1(flp_port, buf, cmd);
}

/*lint -e845*/
static int flp_ar_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch (cmd & FLP_IOCTL_CMD_MASK) {
/*HAL IOCTL VERSION1.0*/
	case FLP_IOCTL_AR_START(0):
		ret = ar_start_cmd(flp_port, (const char __user *)arg, cmd);
	break;
	case FLP_IOCTL_AR_STOP(0):
		ret = ar_stop_cmd(flp_port, arg);
	break;
	case FLP_IOCTL_AR_UPDATE(0):
		ret = ar_update_cmd(flp_port, (const char __user *)arg, cmd);
	break;
	case FLP_IOCTL_AR_FLUSH(0):
		ret = ar_flush_cmd(flp_port);
	break;
	case FLP_IOCTL_AR_STATE(0):
		ret = ar_state_v1_cmd(flp_port, arg);
	break;
/*HAL IOCTL VERSION2.0*/
	case FLP_IOCTL_AR_CONFIG(0):
		ret =  ar_config_cmd(flp_port, cmd, arg);
	break;
	case FLP_IOCTL_AR_STATE_V2(0):
		ret = ar_state_v2_cmd(flp_port, arg);
	break;
	default:
		printk(KERN_ERR "flp_ar_ioctrl input cmd[0x%x] error\n", cmd);
		return -EFAULT;
	}

	return ret;
}
#ifdef ENV_FEATURE
static int flp_env_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	if (!flp_check_cmd(cmd, FLP_ENVIRONMENT))
			return -EPERM;

	switch (cmd & FLP_IOCTL_CMD_MASK) {
	case FLP_IOCTL_ENV_INIT(0):
		ret = env_init_cmd(flp_port, cmd);
	break;
	case FLP_IOCTL_ENV_CONFIG(0):
		ret = env_config_cmd(flp_port, arg);
	break;
	case FLP_IOCTL_ENV_STOP(0):
		ret = env_stop_cmd(flp_port, arg);
	break;
	case FLP_IOCTL_ENV_STATE(0):
		ret = env_state_cmd(flp_port, arg);
	break;
	case FLP_IOCTL_ENV_EXIT(0):
		ret = env_close_cmd(flp_port);
	break;
	default:
		printk(KERN_ERR "flp_ar_ioctrl input cmd[0x%x] error\n", cmd);
		return -EFAULT;
	}
	return ret;
}
#endif
/*lint +e845*/
/*lint -e715*/
static int flp_common_ioctl_open_service(flp_port_t *flp_port)
{
#ifdef GEOFENCE_BATCH_FEATURE
	unsigned int response = FLP_IOMCU_RESET;
#endif
	struct read_info rd;
	int ret;
	memset((void*)&rd, 0,sizeof(struct read_info));
	pr_info("[%s]SER[%x]ARcnt[%d]\n",__func__, g_flp_dev.service_type, g_flp_dev.ar_dev_info.usr_cnt);
	if (g_flp_dev.service_type & FLP_AR_DATA && g_flp_dev.ar_dev_info.usr_cnt) {
		ret = send_cmd_from_kernel_response(TAG_AR, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0, &rd);
		if(0 == ret && g_flp_dev.ar_dev_info.cfg_sz && g_flp_dev.ar_dev_info.cfg.context_num)
			send_cmd_from_kernel(TAG_AR, CMD_CMN_CONFIG_REQ, CMD_FLP_AR_START_REQ,
			(char *)&g_flp_dev.ar_dev_info.cfg, (size_t)g_flp_dev.ar_dev_info.cfg_sz);
	}
#ifdef ENV_FEATURE
	if (g_flp_dev.service_type & FLP_ENVIRONMENT) {
		ret = send_cmd_from_kernel_response(TAG_ENVIRONMENT, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0, &rd);
		 if (0 == ret && g_flp_dev.env_dev_info.cfg_sz && g_flp_dev.env_dev_info.cfg.context_num)
			send_cmd_from_kernel(TAG_ENVIRONMENT, CMD_CMN_CONFIG_REQ, CMD_FLP_AR_START_REQ,
			(char *)&g_flp_dev.env_dev_info.cfg, (size_t)g_flp_dev.env_dev_info.cfg_sz);
	}
#endif
	if (g_flp_dev.service_type & FLP_PDR_DATA && g_flp_dev.pdr_start_count) {
		ret = send_cmd_from_kernel_response(TAG_PDR, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0, &rd);
		if (0 == ret)
			send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ, CMD_FLP_PDR_START_REQ,
			(char *)&g_flp_dev.pdr_config, (size_t)sizeof(pdr_start_config_t));

	}
#ifdef GEOFENCE_BATCH_FEATURE
	if (g_flp_dev.service_type & (FLP_BATCHING|FLP_GEOFENCE))
		flp_generate_netlink_packet(flp_port, (char *)&response, (unsigned int)sizeof(unsigned int), FLP_GENL_CMD_IOMCU_RESET);
#endif
	return 0;
}

static int flp_common_ioctl_close_service(flp_port_t *flp_port)
{
	unsigned int data = 0;
	pr_info("[%s]SER[%x]ARcnt[%d]\n",__func__, g_flp_dev.service_type, g_flp_dev.ar_dev_info.usr_cnt);
	if ((g_flp_dev.service_type & FLP_AR_DATA) && (g_flp_dev.ar_dev_info.usr_cnt)) {
		send_cmd_from_kernel(TAG_AR, CMD_CMN_CONFIG_REQ, CMD_FLP_AR_STOP_REQ,
			(char *)&data, sizeof(int));
		send_cmd_from_kernel(TAG_AR, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
	}
#ifdef ENV_FEATURE
	if (g_flp_dev.service_type & FLP_ENVIRONMENT) {
		send_cmd_from_kernel(TAG_ENVIRONMENT, CMD_CMN_CONFIG_REQ, CMD_ENVIRONMENT_STOP_REQ,
			(char *)&data, sizeof(int));
		send_cmd_from_kernel(TAG_ENVIRONMENT, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
	}
#endif
    /*if start pdr function ever*/
    if ((g_flp_dev.service_type & FLP_PDR_DATA) &&
            (g_flp_dev.pdr_start_count)) {
        send_cmd_from_kernel(TAG_PDR, CMD_CMN_CONFIG_REQ,
            CMD_FLP_PDR_STOP_REQ, (char *)&data, sizeof(int));
        send_cmd_from_kernel(TAG_PDR, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
    }
#ifdef GEOFENCE_BATCH_FEATURE
    if ((g_flp_dev.service_type & FLP_BATCHING) || (g_flp_dev.service_type & FLP_GEOFENCE)) {
        send_cmd_from_kernel(TAG_FLP, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
    }
#endif
    return 0;
}

static int flp_common_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
    unsigned int data = 0;
    int ret = 0;

    if (FLP_IOCTL_COMMON_RELEASE_WAKELOCK != cmd) {
        if (copy_from_user(&data, (void *)arg, sizeof(unsigned int))) {
            printk(KERN_ERR "flp_ioctl copy_from_user error[%d]\n", ret);
            return -EFAULT;
        }
    }

    switch (cmd) {
        case FLP_IOCTL_COMMON_SLEEP:
            printk(HISI_FLP_DEBUG "flp:start timer %d\n",  data);
            /*if timer is running just delete it ,then restart it*/
            hisi_softtimer_delete(&flp_port->sleep_timer);
            ret = hisi_softtimer_modify(&flp_port->sleep_timer, data);
            if (!ret)
                hisi_softtimer_add(&flp_port->sleep_timer);
            break ;
        case FLP_IOCTL_COMMON_AWAKE_RET:
            flp_port->need_awake = data;
            break ;
        case FLP_IOCTL_COMMON_SETPID:
            flp_port->portid = data;
            break ;
        case FLP_IOCTL_COMMON_CLOSE_SERVICE:
	mutex_lock(&g_flp_dev.lock);
	g_flp_dev.denial_sevice = data;
	printk(HISI_FLP_DEBUG "%s 0x%x\n", __func__, g_flp_dev.denial_sevice);
	if(g_flp_dev.denial_sevice)
		flp_common_ioctl_close_service(flp_port);
	else
		flp_common_ioctl_open_service(flp_port);
	mutex_unlock(&g_flp_dev.lock);
	break;
        case FLP_IOCTL_COMMON_HOLD_WAKELOCK:
            flp_port->need_hold_wlock = data;
            printk(HISI_FLP_DEBUG "%s 0x%x\n", __func__, flp_port->need_hold_wlock);
            break ;
        case FLP_IOCTL_COMMON_RELEASE_WAKELOCK:
            if (flp_port->need_hold_wlock) {
                wake_unlock(&flp_port->wlock);/*lint !e455*/
            }
            break;
        default:
            printk(KERN_ERR "flp_common_ioctl input cmd[0x%x] error\n", cmd);
            return -EFAULT;
    }
    return 0;
}
/*lint +e715*/
#ifdef GEOFENCE_BATCH_FEATURE
/*lint -e438*/
static int flp_ioctl_add_geofence(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
    geofencing_hal_config_t hal_config;
    char *cmd_data;
    geofencing_useful_data_t *pdata;
    unsigned int count;

    flp_set_port_tag(flp_port, cmd);
    send_cmd_from_kernel(TAG_FLP, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0);
    if (copy_from_user(&hal_config, (void *)arg, sizeof(geofencing_hal_config_t))) {
        printk(KERN_ERR "%s copy_from_user error\n", __func__);
        return -EFAULT;
    }
    if (hal_config.length> FLP_GEOFENCE_MAX_NUM || 0 == hal_config.length) {
        printk(KERN_ERR "flp geofence number overflow %d\n", hal_config.length);
        return -EFAULT;
    }
    cmd_data = (char *)kmalloc((size_t)hal_config.length, GFP_KERNEL);
    if (!cmd_data) {
        printk(KERN_ERR "%s kmalloc fail\n", __func__);
        return -ENOMEM;
    }
    if (copy_from_user(cmd_data, (const void __user *)hal_config.buf, (unsigned long)hal_config.length)) {
        printk(KERN_ERR "%s copy_from_user error\n", __func__);
        kfree(cmd_data);
        return -EFAULT;
    }
    /*packet size big than 128, so need split it*/
    pdata = (geofencing_useful_data_t *)cmd_data;
    count = hal_config.length/sizeof(geofencing_useful_data_t);
    do {
        if (count < 3) {
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_ADD_GEOF_REQ,
                    (char *)pdata, count * sizeof(geofencing_useful_data_t));
            break;
        } else {
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_ADD_GEOF_REQ,
                    (char *)pdata, 3 * sizeof(geofencing_useful_data_t));
            pdata += 3;
            count -= 3;
        }
    }while(count > 0);
    flp_port->channel_type |= FLP_GEOFENCE;
    g_flp_dev.service_type |= FLP_GEOFENCE;
    kfree(cmd_data);
    return 0;
}

static int flp_geofence_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
    geofencing_hal_config_t hal_config;
    geofencing_option_info_t modify_config;
    char *cmd_data;

    switch (cmd) {
        case FLP_IOCTL_GEOFENCE_ADD :
            flp_ioctl_add_geofence(flp_port, cmd, arg);
            break;
        case FLP_IOCTL_GEOFENCE_REMOVE :
            if (!(flp_port->channel_type & FLP_GEOFENCE)) {
                printk(KERN_ERR "%s not start \n", __func__);
                return -EPERM;
            }
            if (copy_from_user(&hal_config, (const void __user *)arg, sizeof(geofencing_hal_config_t))) {
                printk(KERN_ERR "%s copy_from_user error\n", __func__);
                return -EFAULT;
            }
            if (hal_config.length> FLP_GEOFENCE_MAX_NUM || 0 == hal_config.length) {
                printk(KERN_ERR "flp geofence number overflow %d\n", hal_config.length);
                return -EFAULT;
            }
            cmd_data = (char *)kmalloc((size_t)hal_config.length, GFP_KERNEL);
            if (!cmd_data) {
                printk(KERN_ERR "%s kmalloc fail\n", __func__);
                return -ENOMEM;
            }
            if (copy_from_user(cmd_data, (const void __user *)hal_config.buf, (unsigned long)hal_config.length)) {
                printk(KERN_ERR "%s copy_from_user error\n", __func__);
                kfree(cmd_data);
                return -EFAULT;
            }
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_REMOVE_GEOF_REQ,
                            (char *)cmd_data, (size_t)hal_config.length);
            kfree(cmd_data);
            break;
        case FLP_IOCTL_GEOFENCE_MODIFY :
            if (!(flp_port->channel_type & FLP_GEOFENCE)) {
                printk(KERN_ERR "%s not start \n", __func__);
                return -EPERM;
            }
            if (copy_from_user(&modify_config, (const void __user *)arg, sizeof(geofencing_option_info_t))) {
                printk(KERN_ERR "%s copy_from_user error\n", __func__);
                return -EFAULT;
            }
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_MODIFY_GEOF_REQ,
                (char *)&modify_config, sizeof(geofencing_option_info_t));
            break;
        default :
            printk(KERN_ERR "%s input cmd[0x%x] error\n", __func__, cmd);
            return -EFAULT;
    }
    return 0;
}
/*lint +e438*/
/*max complexiy must less than 15*/
static int __flp_location_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
    int data;
    iomcu_location inject_data;
    if (!(flp_port->channel_type & FLP_BATCHING)) {
        printk(KERN_ERR "%s not start \n", __func__);
        return -EPERM;
    }
    switch (cmd) {
        case FLP_IOCTL_BATCHING_STOP :
            if (copy_from_user(&data, (void *)arg, sizeof(int))) {
                printk(KERN_ERR "%s copy_from_user error\n", __func__);
                return -EFAULT;
            }
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_STOP_BATCHING_REQ, (char *)&data, sizeof(int));
            flp_port->channel_type &= ~FLP_BATCHING;
            g_flp_dev.service_type &= ~FLP_BATCHING;
            break ;
        case FLP_IOCTL_BATCHING_UPDATE :
            if (copy_from_user(&flp_port->gps_batching_config, (void *)arg, sizeof(batching_config_t))) {
                printk(KERN_ERR "%s copy_from_user error\n", __func__);
                return -EFAULT;
            }
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_UPDATE_BATCHING_REQ,
                (char *)&flp_port->gps_batching_config, sizeof(batching_config_t));
            break;
        case FLP_IOCTL_BATCHING_CLEANUP :
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
            flp_port->channel_type &= ~FLP_BATCHING;
            g_flp_dev.service_type &= ~(FLP_BATCHING | FLP_GEOFENCE);
            break;
        case FLP_IOCTL_BATCHING_LASTLOCATION :
            if (copy_from_user(&data, (void *)arg, sizeof(int))) {
                printk(KERN_ERR "%s copy_from_user error\n", __func__);
                return -EFAULT;
            }
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_GET_BATCHED_LOCATION_REQ, (char *)&data, sizeof(int));
            break ;
        case FLP_IOCTL_BATCHING_FLUSH :
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_FLUSH_LOCATION_REQ, NULL, (size_t)0);
            break ;
        case FLP_IOCTL_BATCHING_INJECT :            /*inject data needn't recovery*/
            if (copy_from_user(&inject_data, (void *)arg, sizeof(iomcu_location))) {
                printk(KERN_ERR "%s copy_from_user error\n", __func__);
                return -EFAULT;
            }
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_INJECT_LOCATION_REQ, (char *)&inject_data, sizeof(iomcu_location));
            break;
        case FLP_IOCTL_COMMON_HW_RESET :
            if (copy_from_user(&data, (void *)arg, sizeof(int))) {
                printk(KERN_ERR "%s copy_from_user error\n", __func__);
                return -EFAULT;
            }
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_RESET_REQ, (char *)&data, sizeof(int));
            break ;
	default:break;
    }
    return 0;
}

static int flp_location_ioctl(flp_port_t *flp_port, unsigned int cmd, unsigned long arg)
{
    struct read_info rd;

    switch (cmd) {
        case FLP_IOCTL_BATCHING_START :
            flp_set_port_tag(flp_port, cmd);
            if (!(flp_port->channel_type & FLP_BATCHING)) {
                send_cmd_from_kernel(TAG_FLP, CMD_CMN_OPEN_REQ, 0, NULL, (size_t)0);
            }
            if (copy_from_user(&flp_port->gps_batching_config, (void *)arg, sizeof(batching_config_t))) {
                printk(KERN_ERR "%s copy_from_user error\n", __func__);
                return -EFAULT;
            }
            send_cmd_from_kernel(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_START_BATCHING_REQ,
                (char *)&flp_port->gps_batching_config, sizeof(batching_config_t));
            flp_port->channel_type |= FLP_BATCHING;
            g_flp_dev.service_type |= FLP_BATCHING;
            break ;
        case FLP_IOCTL_BATCHING_STOP :
        case FLP_IOCTL_BATCHING_UPDATE :
        case FLP_IOCTL_BATCHING_CLEANUP :
        case FLP_IOCTL_BATCHING_LASTLOCATION :
        case FLP_IOCTL_BATCHING_FLUSH :
        case FLP_IOCTL_BATCHING_INJECT :
        case FLP_IOCTL_COMMON_HW_RESET :
            __flp_location_ioctl(flp_port, cmd, arg);
            break ;
        case FLP_IOCTL_BATCHING_GET_SIZE:
            if (!send_cmd_from_kernel_response(TAG_FLP, CMD_CMN_CONFIG_REQ, CMD_FLP_GET_BATCH_SIZE_REQ, NULL, (size_t)0, &rd)) {
                if (copy_to_user((void *)arg, rd.data, sizeof(unsigned int))) {
                    printk(KERN_ERR "%s copy_to_user error\n", __func__);
                    return -EFAULT;
                }
            }
            break;
        default :
            printk(KERN_ERR "%s input cmd[0x%x] error\n", __func__, cmd);
            return -EFAULT;
    }
    return 0;
}
#endif
/*lint -e732*/
static long flp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	flp_port_t *flp_port  = (flp_port_t *)file->private_data;
	if (!flp_port) {
		printk(KERN_ERR "flp_ioctl parameter error\n");
		return -EINVAL;
	}
	printk(HISI_FLP_DEBUG "[%s]cmd[0x%x]\n\n", __func__, cmd&0x0FFFF);
	mutex_lock(&g_flp_dev.lock);
	if ((g_flp_dev.denial_sevice) && (cmd != FLP_IOCTL_COMMON_CLOSE_SERVICE)) {
		mutex_unlock(&g_flp_dev.lock);
		return 0;
	}
	mutex_unlock(&g_flp_dev.lock);

	switch (cmd & FLP_IOCTL_TYPE_MASK) {
	case FLP_IOCTL_TYPE_PDR:
		if (!flp_check_cmd(cmd, FLP_PDR_DATA))
			return -EPERM;
		return (long)flp_pdr_ioctl(flp_port, cmd, arg);
	case FLP_IOCTL_TYPE_AR:
		if (!flp_check_cmd(cmd, FLP_AR_DATA))
			return -EPERM;
		return (long)flp_ar_ioctl(flp_port, cmd, arg);
#ifdef ENV_FEATURE
	case FLP_IOCTL_TYPE_ENV:
		return (long)flp_env_ioctl(flp_port, cmd, arg);
#endif
#ifdef GEOFENCE_BATCH_FEATURE
	case FLP_IOCTL_TYPE_GEOFENCE:
		if (!flp_check_cmd((int)cmd, FLP_GEOFENCE))
			return -EPERM;
		return (long)flp_geofence_ioctl(flp_port, cmd, arg);
	case FLP_IOCTL_TYPE_BATCHING:
		if (!flp_check_cmd((int)cmd, FLP_BATCHING))
			return -EPERM;
		return (long)flp_location_ioctl(flp_port, cmd, arg);
#endif
	case FLP_IOCTL_TYPE_COMMON:
		return (long)flp_common_ioctl(flp_port, cmd, arg);
	default:
		printk(KERN_ERR "flp_ioctl input cmd[0x%x] error\n", cmd);
		return -EFAULT;
	}
}
/*lint +e732*/
/*lint -e438*/
static int flp_open(struct inode *inode, struct file *filp)/*lint -e715*/
{
	int ret = 0;
	flp_port_t *flp_port;
	struct list_head *pos;
	int count = 0;

	mutex_lock(&g_flp_dev.lock);
	list_for_each(pos, &g_flp_dev.list) {
		count++;
	}

	if(count > 100) {
		pr_err("flp_open clinet limit\n");
		ret = -EACCES;
		goto FLP_OPEN_ERR;
	}

	flp_port  = (flp_port_t *) kmalloc(sizeof(flp_port_t), GFP_KERNEL|__GFP_ZERO);
	if (!flp_port) {
		printk(KERN_ERR "flp_open no mem\n");
		ret = -ENOMEM;
		goto FLP_OPEN_ERR;
	}
	INIT_LIST_HEAD(&flp_port->list);
	hisi_softtimer_create(&flp_port->sleep_timer,
	            flp_sleep_timeout, (unsigned long)flp_port, 0);
	INIT_WORK(&flp_port->work, flp_timerout_work);

	list_add_tail(&flp_port->list, &g_flp_dev.list);
	mutex_unlock(&g_flp_dev.lock);
	wake_lock_init(&flp_port->wlock, WAKE_LOCK_SUSPEND, "hisi_flp");
	filp->private_data = flp_port;
	printk(KERN_ERR "%s %d: enter\n", __func__, __LINE__);
	return 0;
FLP_OPEN_ERR:
	mutex_unlock(&g_flp_dev.lock);
	return ret;
}

static void __flp_release(flp_port_t *flp_port)
{
	hisi_softtimer_delete(&flp_port->sleep_timer);
	cancel_work_sync(&flp_port->work);

	data_buffer_exit(&flp_port->ar_buf);

	if (flp_port->pdr_buf.data_buf) {
		kfree(flp_port->pdr_buf.data_buf);
		flp_port->pdr_buf.data_buf = NULL;
	}
	wake_lock_destroy(&flp_port->wlock);
	kfree(flp_port);
	flp_port = NULL;
}

static int flp_release(struct inode *inode, struct file *file)/*lint -e715*/
{
	flp_port_t *flp_port  = (flp_port_t *)file->private_data;
	struct list_head    *pos;
	flp_port_t      *port;
	printk(HISI_FLP_DEBUG "[%s]\n", __func__);
	if (!flp_port) {
		printk(KERN_ERR "flp_close parameter error\n");
		return -EINVAL;
	}

	mutex_lock(&g_flp_dev.lock);
	list_del(&flp_port->list);

/*if andriod vm restart, apk doesnot send stop cmd,just adjust it*/
	g_flp_dev.ar_dev_info.usr_cnt = 0;
	g_flp_dev.pdr_start_count = 0;
	list_for_each(pos, &g_flp_dev.list) {
		port = container_of(pos, flp_port_t, list);
		if (port->channel_type & FLP_AR_DATA) {
			g_flp_dev.ar_dev_info.usr_cnt++ ;
		}

		if (port->channel_type & FLP_PDR_DATA) {
			g_flp_dev.pdr_start_count++ ;
		}
	}

	ar_stop(0);

	/*if start pdr function ever*/
	if ((g_flp_dev.service_type & FLP_PDR_DATA) &&
	(!g_flp_dev.pdr_start_count)) {
		send_cmd_from_kernel(TAG_PDR, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
		g_flp_dev.service_type &= ~FLP_PDR_DATA;
	}
	/*if start batching or Geofence function ever*/
	if ((g_flp_dev.service_type & FLP_BATCHING) || (g_flp_dev.service_type & FLP_GEOFENCE)) {
		send_cmd_from_kernel(TAG_FLP, CMD_CMN_CLOSE_REQ, 0, NULL, (size_t)0);
		g_flp_dev.service_type &= ~(FLP_BATCHING | FLP_GEOFENCE);
	}
#ifdef ENV_FEATURE
	env_close_cmd(flp_port);
#endif
	__flp_release(flp_port);
	file->private_data = NULL ;
	printk(KERN_ERR "%s pdr_count[%d]:service_type [%d] \n", __func__, g_flp_dev.pdr_start_count,
		g_flp_dev.service_type);
	mutex_unlock(&g_flp_dev.lock);
	return 0;
}
/*lint +e438*/
/*lint +e826*/
/*lint -e785 -e64*/
static const struct file_operations hisi_flp_fops = {
	.owner =          THIS_MODULE,
	.llseek =         no_llseek,
	.unlocked_ioctl = flp_ioctl,
	.open       =     flp_open,
	.release    =     flp_release,
};

/*******************************************************************************************
Description:   miscdevice to motion
*******************************************************************************************/
static struct miscdevice hisi_flp_miscdev =
{
    .minor =    MISC_DYNAMIC_MINOR,
    .name =     "flp",
    .fops =     &hisi_flp_fops,
};
/*lint +e785 +e64*/

/*******************************************************************************************
Function:       hisi_flp_register
Description:
Data Accessed:  no
Data Updated:   no
Input:          void
Output:        void
Return:        result of function, 0 successed, else false
*******************************************************************************************/
int  hisi_flp_register(void)
{
	int ret;

	memset((void*)&g_flp_dev, 0 ,sizeof(g_flp_dev));

	init_completion(&g_flp_dev.ar_dev_info.state_cplt);
	init_completion(&g_flp_dev.env_dev_info.state_cplt);

	ret = genl_register_family(&flp_genl_family);
	if (ret) {
		return ret ;
	}
	if (!getSensorMcuMode()) {
		printk(KERN_ERR "cannot register hisi flp %d\n", __LINE__);
		genl_unregister_family(&flp_genl_family);
		return -ENODEV;
	}
	INIT_LIST_HEAD(&g_flp_dev.list);
	PDR_REGISTER_CALLBACK(TAG_PDR, CMD_FLP_PDR_DATA_REQ, get_pdr_data_from_mcu);
	PDR_REGISTER_CALLBACK(TAG_PDR, CMD_FLP_PDR_UNRELIABLE_REQ, get_pdr_notify_from_mcu);
	PDR_REGISTER_CALLBACK(TAG_AR, CMD_FLP_AR_DATA_REQ, get_ar_data_from_mcu);
#ifndef CONTEXT_RESP_API
	PDR_REGISTER_CALLBACK(TAG_ENVIRONMENT, CMD_CMN_CONFIG_RESP, get_env_state_from_iomcu);
	PDR_REGISTER_CALLBACK(TAG_AR, CMD_CMN_CONFIG_RESP, get_ar_state_from_iomcu);
#endif
#ifdef ENV_FEATURE
	PDR_REGISTER_CALLBACK(TAG_ENVIRONMENT, CMD_ENVIRONMENT_DATA_REQ, get_env_data_from_mcu);
#endif
#ifdef GEOFENCE_BATCH_FEATURE
	PDR_REGISTER_CALLBACK(TAG_FLP, CMD_FLP_LOCATION_UPDATE_REQ, get_common_data_from_mcu);
	PDR_REGISTER_CALLBACK(TAG_FLP, CMD_FLP_GEOF_TRANSITION_REQ, get_common_data_from_mcu);
	PDR_REGISTER_CALLBACK(TAG_FLP, CMD_FLP_GEOF_MONITOR_STATUS_REQ, get_common_data_from_mcu);
#endif
#ifdef CONFIG_IOM3_RECOVERY
	register_iom3_recovery_notifier(&sensor_reboot_notify);
#endif
	mutex_init(&g_flp_dev.lock);

	ret = misc_register(&hisi_flp_miscdev);
	if (ret != 0)    {
		printk(KERN_ERR "cannot register hisi flp err=%d\n", ret);
		goto err;
	}
	printk(KERN_ERR "hisi_flp_register success\n");
	return 0;
err:
	unregister_mcu_event_notifier(TAG_PDR, CMD_FLP_PDR_DATA_REQ, get_pdr_data_from_mcu);
	unregister_mcu_event_notifier(TAG_PDR, CMD_FLP_PDR_UNRELIABLE_REQ, get_pdr_notify_from_mcu);
	unregister_mcu_event_notifier(TAG_AR, CMD_FLP_AR_DATA_REQ, get_ar_data_from_mcu);
#ifndef CONTEXT_RESP_API
	unregister_mcu_event_notifier(TAG_AR, CMD_CMN_CONFIG_RESP, get_ar_state_from_iomcu);
	unregister_mcu_event_notifier(TAG_ENVIRONMENT, CMD_CMN_CONFIG_RESP, get_env_state_from_iomcu);
#endif
#ifdef ENV_FEATURE
	unregister_mcu_event_notifier(TAG_ENVIRONMENT, CMD_ENVIRONMENT_DATA_REQ, get_env_data_from_mcu);
#endif
#ifdef GEOFENCE_BATCH_FEATURE
	unregister_mcu_event_notifier(TAG_FLP, CMD_FLP_LOCATION_UPDATE_REQ, get_common_data_from_mcu);
	unregister_mcu_event_notifier(TAG_FLP, CMD_FLP_GEOF_TRANSITION_REQ, get_common_data_from_mcu);
	unregister_mcu_event_notifier(TAG_FLP, CMD_FLP_GEOF_MONITOR_STATUS_REQ, get_common_data_from_mcu);
#endif
	genl_unregister_family(&flp_genl_family);
	return ret;
}

EXPORT_SYMBOL_GPL(hisi_flp_register);


/*******************************************************************************************
Function:       hisi_flp_unregister
Description:
Data Accessed:  no
Data Updated:   no
Input:          void
Output:        void
Return:        void
*******************************************************************************************/
 int hisi_flp_unregister(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
	int ret;
#endif
	ar_ioctl_cmd_t just_for_pclint;
	flp_ioctl_cmd_t just_for_pclint_flp;
	just_for_pclint.hd.cmd = 0;
	just_for_pclint.rsv1 = 0;
	just_for_pclint.rsv2 = 0;
	just_for_pclint.core = 0;
	just_for_pclint_flp.hd.cmd = 0;
	pr_debug("%u%u%u%u\n", just_for_pclint.hd.cmd,just_for_pclint.rsv1,just_for_pclint.rsv2,just_for_pclint.core);
	pr_debug("%u\n", just_for_pclint_flp.hd.cmd);
	unregister_mcu_event_notifier(TAG_PDR, CMD_FLP_PDR_DATA_REQ, get_pdr_data_from_mcu);
	unregister_mcu_event_notifier(TAG_PDR, CMD_FLP_PDR_UNRELIABLE_REQ, get_pdr_notify_from_mcu);
	unregister_mcu_event_notifier(TAG_AR, CMD_FLP_AR_DATA_REQ, get_ar_data_from_mcu);
#ifdef ENV_FEATURE
	unregister_mcu_event_notifier(TAG_ENVIRONMENT, CMD_ENVIRONMENT_DATA_REQ, get_env_data_from_mcu);
#endif
#ifndef CONTEXT_RESP_API
	unregister_mcu_event_notifier(TAG_ENVIRONMENT, CMD_CMN_CONFIG_RESP, get_env_state_from_iomcu);
	unregister_mcu_event_notifier(TAG_AR, CMD_CMN_CONFIG_RESP, get_ar_state_from_iomcu);
#endif
	genl_unregister_family(&flp_genl_family);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
	ret = misc_deregister(&hisi_flp_miscdev);
	printk(HISI_FLP_DEBUG "hisi_flp_unregister ret=%d\n", ret);
	return ret;
#else
	misc_deregister(&hisi_flp_miscdev);
	return 0;
#endif
}
EXPORT_SYMBOL_GPL(hisi_flp_unregister);
