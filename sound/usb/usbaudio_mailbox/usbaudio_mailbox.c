#include "usbaudio_mailbox.h"
#include <linux/usb.h>
#include <linux/proc_fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#ifdef CONFIG_HIFI_MAILBOX
#include "drv_mailbox_cfg.h"
#endif
#ifdef CONFIG_HISI_DEBUG_FS
#include <linux/debugfs.h>
#endif
struct completion probe_msg_complete;
struct completion disconnect_msg_complete;
#define HIFI_USBAUDIO_MSG_TIMEOUT (2 * HZ)

int usbaudio_mailbox_send_data(void *pmsg_body, unsigned int msg_len, unsigned int msg_priority)
{
	unsigned int ret = 0;

	ret = DRV_MAILBOX_SENDMAIL(MAILBOX_MAILCODE_ACPU_TO_HIFI_USBAUDIO, pmsg_body, msg_len);
	if (MAILBOX_OK != ret) {
		pr_err("usbaudio channel send mail failed, ret=%d\n", ret);
	}

	return (int)ret;
}

static irq_rt_t usbaudio_mailbox_recv_isr(void *usr_para, void *mail_handle, unsigned int mail_len)
{
	struct usbaudio_rcv_msg rcv_msg;
	unsigned int ret = MAILBOX_OK;
	unsigned int mail_size = mail_len;
	memset(&rcv_msg, 0, sizeof(struct usbaudio_rcv_msg));

	ret = DRV_MAILBOX_READMAILDATA(mail_handle, (unsigned char*)&rcv_msg, &mail_size);
	if ((ret != MAILBOX_OK)
		|| (mail_size == 0)
		|| (mail_size > sizeof(struct usbaudio_rcv_msg))) {
		pr_err("Empty point or data length error! size: %d  ret:%d sizeof(struct usbaudio_mailbox_received_msg):%lu\n",
						mail_size, ret, sizeof(struct usbaudio_rcv_msg));
		return IRQ_NH_MB;
	}

	switch(rcv_msg.msg_type) {
		/* just support audio_format message only for now */
		case USBAUDIO_CHN_MSG_PROBE_RCV:
			pr_info("receive message: probe succ.\n");
			/* todo */
			complete(&probe_msg_complete);
			break;
		case USBAUDIO_CHN_MSG_DISCONNECT_RCV:
			pr_info("receive message: disconnect succ.\n");
			/* todo */
			complete(&disconnect_msg_complete);
			break;
		default:
			pr_err("msg_type 0x%x.\n", rcv_msg.msg_type);
			break;
	}

	return IRQ_HDD;
}

static int usbaudio_mailbox_isr_register(irq_hdl_t pisr)
{
	int ret = 0;
	unsigned int mailbox_ret = MAILBOX_OK;

	if (NULL == pisr) {
		pr_err("pisr==NULL!\n");
		ret = ERROR;
	} else {
		mailbox_ret = DRV_MAILBOX_REGISTERRECVFUNC(MAILBOX_MAILCODE_HIFI_TO_ACPU_USBAUDIO, (void *)pisr, NULL);/*lint !e611 */
		if (MAILBOX_OK != mailbox_ret) {
			ret = ERROR;
			pr_err("register isr for usbaudio channel failed, ret : %d,0x%x\n", ret, MAILBOX_MAILCODE_HIFI_TO_ACPU_USBAUDIO);
		}
	}

	return ret;
}

int usbaudio_probe_msg(struct usbaudio_pcms *pcms)
{
	int ret;
	int i;
	unsigned long retval;
	struct usbaudio_probe_msg probe_msg;

	pr_info("usbaudio_probe_msg\n");
	init_completion(&probe_msg_complete);

	probe_msg.msg_type = (unsigned short)USBAUDIO_CHN_MSG_PROBE;
	for (i=0; i<2; i++) {
		probe_msg.pcms.fmts[i].formats = pcms->fmts[i].formats;
		probe_msg.pcms.fmts[i].channels = pcms->fmts[i].channels;
		probe_msg.pcms.fmts[i].fmt_type = pcms->fmts[i].fmt_type;
		probe_msg.pcms.fmts[i].frame_size = pcms->fmts[i].frame_size;
		probe_msg.pcms.fmts[i].iface = pcms->fmts[i].iface;
		probe_msg.pcms.fmts[i].altsetting = pcms->fmts[i].altsetting;
		probe_msg.pcms.fmts[i].altset_idx = pcms->fmts[i].altset_idx;
		probe_msg.pcms.fmts[i].attributes = pcms->fmts[i].attributes;
		probe_msg.pcms.fmts[i].endpoint = pcms->fmts[i].endpoint;
		probe_msg.pcms.fmts[i].ep_attr = pcms->fmts[i].ep_attr;
		probe_msg.pcms.fmts[i].datainterval = pcms->fmts[i].datainterval;
		probe_msg.pcms.fmts[i].protocol = pcms->fmts[i].protocol;
		probe_msg.pcms.fmts[i].maxpacksize = pcms->fmts[i].maxpacksize;
		probe_msg.pcms.fmts[i].rates = pcms->fmts[i].rates;
		probe_msg.pcms.fmts[i].clock = pcms->fmts[i].clock;

		probe_msg.pcms.ifdesc[i].bLength = pcms->ifdesc[i].bLength;
		probe_msg.pcms.ifdesc[i].bDescriptorType = pcms->ifdesc[i].bDescriptorType;
		probe_msg.pcms.ifdesc[i].bInterfaceNumber = pcms->ifdesc[i].bInterfaceNumber;
		probe_msg.pcms.ifdesc[i].bAlternateSetting = pcms->ifdesc[i].bAlternateSetting;
		probe_msg.pcms.ifdesc[i].bNumEndpoints = pcms->ifdesc[i].bNumEndpoints;
		probe_msg.pcms.ifdesc[i].bInterfaceClass = pcms->ifdesc[i].bInterfaceClass;
		probe_msg.pcms.ifdesc[i].bInterfaceSubClass = pcms->ifdesc[i].bInterfaceSubClass;
		probe_msg.pcms.ifdesc[i].bInterfaceProtocol = pcms->ifdesc[i].bInterfaceProtocol;
		probe_msg.pcms.ifdesc[i].iInterface = pcms->ifdesc[i].iInterface;

		probe_msg.pcms.epdesc[i].bLength = pcms->epdesc[i].bLength;
		probe_msg.pcms.epdesc[i].bDescriptorType = pcms->epdesc[i].bDescriptorType;
		probe_msg.pcms.epdesc[i].bEndpointAddress = pcms->epdesc[i].bEndpointAddress;
		probe_msg.pcms.epdesc[i].bmAttributes = pcms->epdesc[i].bmAttributes;
		probe_msg.pcms.epdesc[i].wMaxPacketSize = pcms->epdesc[i].wMaxPacketSize;
		probe_msg.pcms.epdesc[i].bInterval = pcms->epdesc[i].bInterval;
		probe_msg.pcms.epdesc[i].bRefresh = pcms->epdesc[i].bRefresh;
		probe_msg.pcms.epdesc[i].bSynchAddress = pcms->epdesc[i].bSynchAddress;
	}

	ret = usbaudio_mailbox_send_data(&probe_msg, sizeof(struct usbaudio_probe_msg), 0);

	retval = wait_for_completion_timeout(&probe_msg_complete, HIFI_USBAUDIO_MSG_TIMEOUT);
	if (retval == 0) {
		pr_err("usbaudio probe msg send timeout\n");
		return -ETIME;
	}

	return ret;
}

int usbaudio_disconnect_msg(void)
{
	int ret;
	unsigned long retval;
	struct usbaudio_disconnect_msg disconnect_msg;

	pr_info("usbaudio_disconnect_msg\n");
	init_completion(&disconnect_msg_complete);
	disconnect_msg.msg_type = (unsigned short)USBAUDIO_CHN_MSG_DISCONNECT;
	ret = usbaudio_mailbox_send_data(&disconnect_msg, sizeof(disconnect_msg), 0);

	retval = wait_for_completion_timeout(&disconnect_msg_complete, HIFI_USBAUDIO_MSG_TIMEOUT);
	if (retval == 0) {
		pr_err("usbaudio disconnect msg send timeout\n");
		return -ETIME;
	}

	return ret;
}

int usbaudio_mailbox_init(void)
{
	int ret = 0;

	/* register usbaudio mailbox message isr */
	ret = usbaudio_mailbox_isr_register((void*)usbaudio_mailbox_recv_isr);/*lint !e611 */
	if (ret) {
		pr_err("usbaudio_mailbox_isr_register failed : %d\n", ret);
	}

	return ret;
}
