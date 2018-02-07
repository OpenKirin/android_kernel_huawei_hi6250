#ifndef _HISI_USB_H_
#define _HISI_USB_H_

#include <linux/usb.h>

enum hisi_charger_type {
	CHARGER_TYPE_SDP = 0,		/* Standard Downstreame Port */
	CHARGER_TYPE_CDP,		/* Charging Downstreame Port */
	CHARGER_TYPE_DCP,		/* Dedicate Charging Port */
	CHARGER_TYPE_UNKNOWN,		/* non-standard */
	CHARGER_TYPE_NONE,		/* not connected */

	/* other messages */
	PLEASE_PROVIDE_POWER,		/* host mode, provide power */
};

/**
 * event types passed to hisi_usb_otg_event().
 */
enum otg_dev_event_type {
	CHARGER_CONNECT_EVENT = 0,
	CHARGER_DISCONNECT_EVENT,
	ID_FALL_EVENT,
	ID_RISE_EVENT,
	START_HIFI_USB,
	STOP_HIFI_USB,
	NONE_EVENT
};

#if defined(CONFIG_USB_SUSB_HDRC) || defined(CONFIG_USB_DWC3)
int hisi_charger_type_notifier_register(struct notifier_block *nb);
int hisi_charger_type_notifier_unregister(struct notifier_block *nb);
enum hisi_charger_type hisi_get_charger_type(void);

/**
 * hisi_usb_otg_event() - Queue a event to be processed.
 * @evnet_type: the event to be processed.
 *
 * The event will be added to tail of a queue, and processed in a work.
 *
 * Return: 0 means the event added sucessfully. others means event was rejected.
 */
int hisi_usb_otg_event(enum otg_dev_event_type event_type);

void hisi_usb_otg_bc_again(void);
int hisi_usb_otg_irq_notifier_register(struct notifier_block *nb);
int hisi_usb_otg_irq_notifier_unregister(struct notifier_block *nb);
#else
static inline int hisi_charger_type_notifier_register(
		struct notifier_block *nb){return 0;}
static inline int hisi_charger_type_notifier_unregister(
		struct notifier_block *nb){return 0;}
static inline enum hisi_charger_type hisi_get_charger_type(void)
{
	return CHARGER_TYPE_NONE;
}
static inline int hisi_usb_otg_event(enum otg_dev_event_type event_type)
{
	return 0;
}
static inline void hisi_usb_otg_bc_again(void)
{
}
int hisi_usb_otg_irq_notifier_register(
	struct notifier_block *nb){return 0;}
int hisi_usb_otg_irq_notifier_unregister(
	struct notifier_block *nb){return 0;}
#endif /* CONFIG_USB_SUSB_HDRC || CONFIG_USB_DWC3 */

static inline int hisi_usb_id_change(enum otg_dev_event_type event)
{
	if ((event == ID_FALL_EVENT) || (event == ID_RISE_EVENT))
		return hisi_usb_otg_event(event);
	else
		return 0;
}

/* ************************************************************************** */
#ifdef CONFIG_USB_PROXY_HCD
/**
 * switch to hifi usb.
 * return 0 means the request was accepted, others means rejected.
 */
int hisi_usb_start_hifi_usb(void);

/**
 * switch to AP usb host from hifi usb.
 */
void hisi_usb_stop_hifi_usb(void);

/**
 * Wether a usb_device using hifi usb.
 * return true means the usb device is using hifi usb, others means the usb
 * device is not using hifi usb.
 */
bool hisi_usb_using_hifi_usb(struct usb_device *udev);

/**
 * Start hifi usb, but will not configure the usb phy.
 * This function should used only by hisi usb.
 */
int start_hifi_usb(void);

/**
 * Stop the hifi usb, but will not configure the usb phy.
 * This function should used only by hisi usb.
 */
void stop_hifi_usb(void);

/**
 * Rest hifi usb, including reset hifi usb states and freeing its resources.
 * This function should used only by hisi usb when switch to poweroff mode.
 */
void reset_hifi_usb(void);
#else
static inline int hisi_usb_start_hifi_usb(void){return -1;}
static inline void hisi_usb_stop_hifi_usb(void){}
static inline bool hisi_usb_using_hifi_usb(struct usb_device *udev){return false;}
static inline int start_hifi_usb(void){return -1;}
static inline void stop_hifi_usb(void){}
static inline void reset_hifi_usb(void){}
#endif /* CONFIG_USB_PROXY_HCD */

#endif /* _HISI_USB_H_*/
