#ifndef __LINUX_USB31_TCA_H__
#define __LINUX_USB31_TCA_H__

typedef enum _tca_irq_type_e{
	TCA_IRQ_HPD_OUT = 0,
	TCA_IRQ_HPD_IN = 1,
	TCA_IRQ_SHORT = 2,
	TCA_IRQ_MAX_NUM
}TCA_IRQ_TYPE_E;

typedef enum _tca_device_type_e{
	TCA_CHARGER_CONNECT_EVENT = 0,
	TCA_CHARGER_DISCONNECT_EVENT,
	TCA_ID_FALL_EVENT,/*¡ä¨²¡À¨ªotg??2?¨¨?¨º??¨²*/
	TCA_ID_RISE_EVENT,
	TCA_DP_OUT,
	TCA_DP_IN,
	TCA_DEV_MAX
}TCA_DEV_TYPE_E;

typedef enum {
	TCPC_NC = 0,
	TCPC_USB31_CONNECTED = 1,
	TCPC_DP = 2,
	TCPC_USB31_AND_DP_2LINE = 3,
	TCPC_MUX_MODE_MAX
}TCPC_MUX_CTRL_TYPE;

typedef enum _typec_plug_orien_e{
	TYPEC_ORIEN_POSITIVE = 0,
	TYPEC_ORIEN_NEGATIVE = 1,
	TYPEC_ORIEN_MAX
}TYPEC_PLUG_ORIEN_E;

extern int  pd_event_notify(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode_type, TCA_DEV_TYPE_E dev_type, TYPEC_PLUG_ORIEN_E typec_orien);
#endif
