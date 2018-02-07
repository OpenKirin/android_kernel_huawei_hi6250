#ifndef __USBAUDIO_IOCTL_H
#define __USBAUDIO_IOCTL_H

enum usbaudio_stream_type
{
	DOWNLINK_STREAM = 0,
	UPLINK_STREAM,
	TYPE_MAX
};

enum usbaudio_stream_status
{
	START_STREAM = 0,
	STOP_STREAM,
	STATUS_MAX
};

enum usbaudio_controller_status
{
	ACPU_CONTROL = 0,
	DSP_CONTROL,
	CONTROL_MAX
};

int usbaudio_ctrl_set_pipeout_interface(unsigned int running);
int usbaudio_ctrl_set_pipein_interface(unsigned int running);
int usbaudio_ctrl_query_controller_location(void);
#endif /* __USBAUDIO_IOCTL_H*/
