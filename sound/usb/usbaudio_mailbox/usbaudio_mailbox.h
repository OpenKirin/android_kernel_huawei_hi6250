#ifndef __USBUDIO_MAILBOX_H
#define __USBUDIO_MAILBOX_H

enum IRQ_RT
{
	/* IRQ Not Handled as Other problem */
	IRQ_NH_OTHERS    = -5,
	/* IRQ Not Handled as Mailbox problem */
	IRQ_NH_MB     = -4,
	/* IRQ Not Handled as pcm MODE problem */
	IRQ_NH_MODE     = -3,
	/* IRQ Not Handled as TYPE problem */
	IRQ_NH_TYPE     = -2,
	/* IRQ Not Handled */
	IRQ_NH          = -1,
	/* IRQ HanDleD */
	IRQ_HDD         = 0,
	/* IRQ HanDleD related to PoinTeR */
	IRQ_HDD_PTR     = 1,
	/* IRQ HanDleD related to STATUS */
	IRQ_HDD_STATUS,
	/* IRQ HanDleD related to SIZE */
	IRQ_HDD_SIZE,
	/* IRQ HanDleD related to PoinTeR of Substream */
	IRQ_HDD_PTRS,
	/* IRQ HanDleD Error */
	IRQ_HDD_ERROR,
};
typedef enum IRQ_RT irq_rt_t;

typedef irq_rt_t (*irq_hdl_t)(void *, unsigned int);

#define USBAUDIO_CHN_COMMON   \
	unsigned short  msg_type;   \
	unsigned short  reserved;

enum USBAUDIO_CHN_MSG_TYPE {
	USBAUDIO_CHN_MSG_PROBE = 0xFF00,    /* acpu->dsp */
	USBAUDIO_CHN_MSG_PROBE_RCV = 0xFF01,    /* dsp->acpu */
	USBAUDIO_CHN_MSG_DISCONNECT = 0xFF02,    /* acpu->dsp */
	USBAUDIO_CHN_MSG_DISCONNECT_RCV = 0xFF03,    /* dsp->acpu */
};

struct usbaudio_epdesc {
	unsigned char bLength;
	unsigned char bDescriptorType;

	unsigned char bEndpointAddress;
	unsigned char bmAttributes;
	unsigned short wMaxPacketSize;
	unsigned char bInterval;

	/* NOTE:  these two are _only_ in audio endpoints. */
	/* use USB_DT_ENDPOINT*_SIZE in bLength, not sizeof. */
	unsigned char bRefresh;
	unsigned char bSynchAddress;
};

struct usbaudio_ifdesc {
	unsigned char bLength;
	unsigned char bDescriptorType;
	unsigned char bInterfaceNumber;
	unsigned char bAlternateSetting;
	unsigned char bNumEndpoints;
	unsigned char bInterfaceClass;
	unsigned char bInterfaceSubClass;
	unsigned char bInterfaceProtocol;
	unsigned char iInterface;
};

struct usbaudio_formats {
	unsigned long long formats;			/* ALSA format bits */
	unsigned int channels;		/* # channels */
	unsigned int fmt_type;		/* USB audio format type (1-3) */
	unsigned int frame_size;	/* samples per frame for non-audio */
	int iface;			/* interface number */
	unsigned char altsetting;	/* corresponding alternate setting */
	unsigned char altset_idx;	/* array index of altenate setting */
	unsigned char attributes;	/* corresponding attributes of cs endpoint */
	unsigned char endpoint;		/* endpoint */
	unsigned char ep_attr;		/* endpoint attributes */
	unsigned char datainterval;	/* log_2 of data packet interval */
	unsigned char protocol;		/* UAC_VERSION_1/2 */
	unsigned int maxpacksize;	/* max. packet size */
	unsigned int rates;
	unsigned char clock;		/* associated clock */
};

struct usbaudio_pcms {
	struct usbaudio_formats fmts[2];
	struct usbaudio_ifdesc ifdesc[2];
	struct usbaudio_epdesc epdesc[2];
};

struct usbaudio_probe_rcv_data {
	unsigned int ret_val;
};

struct usbaudio_disconnect_rcv_data {
	unsigned int ret_val;
};

struct usbaudio_rcv_msg {
	USBAUDIO_CHN_COMMON
	union {
		char data[8];
		struct usbaudio_probe_rcv_data probe_rcv_data;
		struct usbaudio_disconnect_rcv_data disconnect_rcv_data;
	};
};

struct usbaudio_disconnect_msg {
	USBAUDIO_CHN_COMMON
};

/* usbaudio dsp receive test message */
struct usbaudio_probe_msg {
	USBAUDIO_CHN_COMMON
	struct usbaudio_pcms pcms;
};

int usbaudio_mailbox_init(void);
int usbaudio_probe_msg(struct usbaudio_pcms *pcms);
int usbaudio_disconnect_msg(void);

#endif
