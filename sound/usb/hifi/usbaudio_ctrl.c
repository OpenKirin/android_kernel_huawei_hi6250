/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/usb.h>
#include <linux/usb/audio.h>
#include <linux/usb/audio-v2.h>
#include <linux/slab.h>

#include <sound/control.h>
#include <sound/core.h>
#include <sound/info.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <linux/hisi/usb/hisi_usb.h>

#include "usbaudio.h"
#include "card.h"
#include "helper.h"
#include "format.h"
#include "clock.h"
#include "usbaudio_dsp_client.h"
#include "usbaudio_ioctl.h"

#define DTS_USBAUDIO_DSP_NAME "hisilicon,usbaudiodsp"

static DEFINE_MUTEX(connect_mutex);

struct usbaudio_dsp_client_ops {
	bool (*controller_switch)(struct usb_device *dev, u32 usb_id, struct usb_host_interface *ctrl_intf, int ctrlif, struct usbaudio_pcms *pcms);
	int (*disconnect)(struct snd_usb_audio *chip);
	int (*set_pipeout_interface)(struct snd_usb_audio *chip, struct usbaudio_pcms *pcms, unsigned int running);
	int (*set_pipein_interface)(struct snd_usb_audio *chip, struct usbaudio_pcms *pcms, unsigned int running);
	bool (*controller_query)(struct snd_usb_audio *chip);
	bool (*send_usbaudioinfo2hifi)(struct snd_usb_audio *chip, struct usbaudio_pcms *pcms);
};

struct usbaudio_dsp  {
	struct snd_usb_audio *chip;
	struct usbaudio_pcms pcms;
	struct usbaudio_dsp_client_ops *ops;
	unsigned int pipeout_running_flag;
	unsigned int pipein_running_flag;
};

static struct usbaudio_dsp_client_ops client_ops = {
	.controller_switch = controller_switch,
	.disconnect = disconnect,
	.set_pipeout_interface = set_pipeout_interface,
	.set_pipein_interface = set_pipein_interface,
	.controller_query = controller_query,
};

static struct usbaudio_dsp *usbaudio_hifi = NULL;

bool usbaudio_ctrl_controller_switch(struct usb_device *dev, u32 usb_id, struct usb_host_interface *ctrl_intf, int ctrlif)
{
	bool ret;
	mutex_lock(&connect_mutex);
	if (!usbaudio_hifi) {
		ret = false;
	} else {
		usbaudio_hifi->pipeout_running_flag = STOP_STREAM;
		usbaudio_hifi->pipein_running_flag = STOP_STREAM;
		ret = usbaudio_hifi->ops->controller_switch(dev, usb_id, ctrl_intf, ctrlif, &usbaudio_hifi->pcms);
	}
	mutex_unlock(&connect_mutex);

	return ret;
}

int usbaudio_ctrl_disconnect(void)
{
	int ret;
	mutex_lock(&connect_mutex);
	if (!usbaudio_hifi || !usbaudio_hifi->chip) {
		ret = 0;
	} else {
		ret = usbaudio_hifi->ops->disconnect(usbaudio_hifi->chip);
		usbaudio_hifi->chip = NULL;
	}
	mutex_unlock(&connect_mutex);

	return ret;
}

int usbaudio_ctrl_set_pipeout_interface(unsigned int running)
{
	int ret;
	mutex_lock(&connect_mutex);
	if (!usbaudio_hifi || !usbaudio_hifi->chip) {
		ret = 0;
	} else {
		if (running == usbaudio_hifi->pipeout_running_flag) {
			ret = 0;
		} else {
			usbaudio_hifi->pipeout_running_flag = running;
			ret = usbaudio_hifi->ops->set_pipeout_interface(usbaudio_hifi->chip, &usbaudio_hifi->pcms, running);
		}
	}
	mutex_unlock(&connect_mutex);

	return ret;
}

int usbaudio_ctrl_set_pipein_interface(unsigned int running)
{
	int ret;
	mutex_lock(&connect_mutex);
	if (!usbaudio_hifi || !usbaudio_hifi->chip) {
		ret = 0;
	} else {
		if (running == usbaudio_hifi->pipein_running_flag) {
			ret = 0;
		} else {
			usbaudio_hifi->pipein_running_flag = running;
			ret = usbaudio_hifi->ops->set_pipein_interface(usbaudio_hifi->chip, &usbaudio_hifi->pcms, running);
		}
	}
	mutex_unlock(&connect_mutex);

	return ret;
}

int usbaudio_ctrl_query_controller_location(void)
{
	int ret;
	mutex_lock(&connect_mutex);
	if (!usbaudio_hifi || !usbaudio_hifi->chip) {
		ret = ACPU_CONTROL;
	} else {
		if(usbaudio_hifi->ops->controller_query(usbaudio_hifi->chip))
			ret = DSP_CONTROL;
		else
			ret = ACPU_CONTROL;
	}
	mutex_unlock(&connect_mutex);

	return ret;
}

void usbaudio_ctrl_set_chip(struct snd_usb_audio *chip)
{
	mutex_lock(&connect_mutex);
	if (usbaudio_hifi) {
		usbaudio_hifi->chip = chip;
		send_usbaudioinfo2hifi(usbaudio_hifi->chip, &usbaudio_hifi->pcms);
	}
	mutex_unlock(&connect_mutex);
}

static int usbaudio_dsp_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	dev_info(dev, "probe begin");
	usbaudio_hifi = devm_kzalloc(dev, sizeof(*usbaudio_hifi), GFP_KERNEL);
	if (!usbaudio_hifi) {
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, usbaudio_hifi);
	usbaudio_hifi->ops = &client_ops;
	ret = usbaudio_mailbox_init();

	return ret;
}

static int usbaudio_dsp_remove(struct platform_device *pdev)
{
	usbaudio_hifi = NULL;
	return 0;
}

static const struct of_device_id usbaudio_dsp_match_table[] = {
	{
		.compatible = DTS_USBAUDIO_DSP_NAME,
		.data = NULL,
	},
	{ /* end */ }
};

static struct platform_driver usbaudio_dsp_driver = {
	.driver =
	{
		.name  = "usbaudio_dsp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(usbaudio_dsp_match_table),
	},
	.probe	= usbaudio_dsp_probe,
	.remove = usbaudio_dsp_remove,
};

static int __init usbaudio_dsp_init(void)
{
	return platform_driver_register(&usbaudio_dsp_driver);
}

static void __exit usbaudio_dsp_exit(void)
{
	platform_driver_unregister(&usbaudio_dsp_driver);
}

fs_initcall_sync(usbaudio_dsp_init);
module_exit(usbaudio_dsp_exit);

MODULE_DESCRIPTION("hisi usbaudio dsp driver");
MODULE_AUTHOR("guzhengming <guzhengming@hisilicon.com>");
MODULE_LICENSE("GPL");

