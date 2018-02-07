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

#include <linux/kernel.h>
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
#include "usbaudio_dsp_client.h"
#include "usbaudio_ioctl.h"

#include "usbaudio.h"
#include "card.h"
#include "helper.h"
#include "format.h"
#include "clock.h"
/*lint -e429 -e514 -e574*/
struct usbaudio_pcm_cfg {
	u64 formats;			/* ALSA format bits */
	unsigned int channels;		/* # channels */
	unsigned int rates;
};

u32 special_usb_ids[] = {
	USB_ID(0x041e, 0x3000),
	USB_ID(0x041e, 0x3020),
	USB_ID(0x10f5, 0x0200),
	USB_ID(0x0d8c, 0x0102),
	USB_ID(0x0ccd, 0x00b1),
	USB_ID(0x0dba, 0x3000),
	USB_ID(0x1235, 0x0010),
	USB_ID(0x1235, 0x0018),
	USB_ID(0x133e, 0x0815),
	USB_ID(0x17cc, 0x1000),
	USB_ID(0x17cc, 0x1010),
	USB_ID(0x17cc, 0x1020),
	USB_ID(0x0763, 0x2012),
	USB_ID(0x047f, 0xc010),
	USB_ID(0x18d1, 0x2d04),
	USB_ID(0x18d1, 0x2d05),
	USB_ID(0x0763, 0x0150),
	USB_ID(0x07fd, 0x0001),
	USB_ID(0x04fa, 0x4201),
	USB_ID(0x0a92, 0x0053),
	USB_ID(0x041e, 0x3020),
	USB_ID(0x0763, 0x2003),
	USB_ID(0x0763, 0x2001),
	USB_ID(0x0763, 0x2012),
	USB_ID(0x047f, 0x0ca1),
	USB_ID(0x077d, 0x07af),
};

/* find an input terminal descriptor (either UAC1 or UAC2) with the given
 * terminal id
 */
static void *
snd_usb_find_input_terminal_descriptor(struct usb_host_interface *ctrl_iface,
					       int terminal_id)
{
	struct uac2_input_terminal_descriptor *term = NULL;

	while ((term = snd_usb_find_csint_desc(ctrl_iface->extra,
					       ctrl_iface->extralen,
					       term, UAC_INPUT_TERMINAL))) {
		if (term->bTerminalID == terminal_id)
			return term;
	}

	return NULL;
}

static struct uac2_output_terminal_descriptor *
	snd_usb_find_output_terminal_descriptor(struct usb_host_interface *ctrl_iface,
						int terminal_id)
{
	struct uac2_output_terminal_descriptor *term = NULL;

	while ((term = snd_usb_find_csint_desc(ctrl_iface->extra,
					       ctrl_iface->extralen,
					       term, UAC_OUTPUT_TERMINAL))) {
		if (term->bTerminalID == terminal_id)
			return term;
	}

	return NULL;
}

static int parse_uac_endpoint_attributes(struct snd_usb_audio *chip,
					 struct usb_host_interface *alts,
					 int protocol, int iface_no)
{
	/* parsed with a v1 header here. that's ok as we only look at the
	 * header first which is the same for both versions */
	struct uac_iso_endpoint_descriptor *csep;
	struct usb_interface_descriptor *altsd = get_iface_desc(alts);
	int attributes = 0;

	csep = snd_usb_find_desc(alts->endpoint[0].extra, alts->endpoint[0].extralen, NULL, USB_DT_CS_ENDPOINT);

	/* Creamware Noah has this descriptor after the 2nd endpoint */
	if (!csep && altsd->bNumEndpoints >= 2)
		csep = snd_usb_find_desc(alts->endpoint[1].extra, alts->endpoint[1].extralen, NULL, USB_DT_CS_ENDPOINT);

	/*
	 * If we can't locate the USB_DT_CS_ENDPOINT descriptor in the extra
	 * bytes after the first endpoint, go search the entire interface.
	 * Some devices have it directly *before* the standard endpoint.
	 */
	if (!csep)
		csep = snd_usb_find_desc(alts->extra, alts->extralen, NULL, USB_DT_CS_ENDPOINT);

	if (!csep || csep->bLength < 7 ||
	    csep->bDescriptorSubtype != UAC_EP_GENERAL) {
		usb_audio_warn(chip,
			       "%u:%d : no or invalid class specific endpoint descriptor\n",
			       iface_no, altsd->bAlternateSetting);
		return 0;
	}

	if (protocol == UAC_VERSION_1) {
		attributes = csep->bmAttributes;
	} else {
		struct uac2_iso_endpoint_descriptor *csep2 =
			(struct uac2_iso_endpoint_descriptor *) csep;

		attributes = csep->bmAttributes & UAC_EP_CS_ATTR_FILL_MAX;

		/* emulate the endpoint attributes of a v1 device */
		if (csep2->bmControls & UAC2_CONTROL_PITCH)
			attributes |= UAC_EP_CS_ATTR_PITCH_CONTROL;
	}

	return attributes;
}

static int snd_usb_parse_audio_interface(struct snd_usb_audio *chip, int iface_no, struct list_head *fp_list)
{
	struct usb_device *dev;
	struct usb_interface *iface;
	struct usb_host_interface *alts;
	struct usb_interface_descriptor *altsd;
	int i, altno, stream;
	unsigned int format = 0, num_channels = 0;
	struct audioformat *fp = NULL;
	int num, protocol, clock = 0;
	struct uac_format_type_i_continuous_descriptor *fmt;
	unsigned int chconfig;

	dev = chip->dev;

	/* parse the interface's altsettings */
	iface = usb_ifnum_to_if(dev, iface_no);

	num = iface->num_altsetting;

	for (i = 0; i < num; i++) {
		alts = &iface->altsetting[i];
		altsd = get_iface_desc(alts);
		protocol = altsd->bInterfaceProtocol;
		/* skip invalid one */
		if (((altsd->bInterfaceClass != USB_CLASS_AUDIO ||
		      (altsd->bInterfaceSubClass != USB_SUBCLASS_AUDIOSTREAMING &&
		       altsd->bInterfaceSubClass != USB_SUBCLASS_VENDOR_SPEC)) &&
		     altsd->bInterfaceClass != USB_CLASS_VENDOR_SPEC) ||
		    altsd->bNumEndpoints < 1 ||
		    le16_to_cpu(get_endpoint(alts, 0)->wMaxPacketSize) == 0)
			continue;
		/* must be isochronous */
		if ((get_endpoint(alts, 0)->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) !=
		    USB_ENDPOINT_XFER_ISOC)
			continue;
		/* check direction */
		stream = (get_endpoint(alts, 0)->bEndpointAddress & USB_DIR_IN) ?
			SNDRV_PCM_STREAM_CAPTURE : SNDRV_PCM_STREAM_PLAYBACK;
		altno = altsd->bAlternateSetting;

		chconfig = 0;
		/* get audio formats */
		switch (protocol) {
		default:
			dev_dbg(&dev->dev, "%u:%d: unknown interface protocol %#02x, assuming v1\n",
				iface_no, altno, protocol);
			protocol = UAC_VERSION_1;
			/* fall through */

		case UAC_VERSION_1: {
			struct uac1_as_header_descriptor *as =
				snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL, UAC_AS_GENERAL);
			struct uac_input_terminal_descriptor *iterm;

			if (!as) {
				dev_err(&dev->dev,
					"%u:%d : UAC_AS_GENERAL descriptor not found\n",
					iface_no, altno);
				continue;
			}

			if (as->bLength < sizeof(*as)) {
				dev_err(&dev->dev,
					"%u:%d : invalid UAC_AS_GENERAL desc\n",
					iface_no, altno);
				continue;
			}

			format = le16_to_cpu(as->wFormatTag); /* remember the format value */

			iterm = snd_usb_find_input_terminal_descriptor(chip->ctrl_intf,
								       as->bTerminalLink);
			if (iterm) {
				num_channels = iterm->bNrChannels;
				chconfig = le16_to_cpu(iterm->wChannelConfig);
			}

			break;
		}

		case UAC_VERSION_2: {
			struct uac2_input_terminal_descriptor *input_term;
			struct uac2_output_terminal_descriptor *output_term;
			struct uac2_as_header_descriptor *as =
				snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL, UAC_AS_GENERAL);

			if (!as) {
				dev_err(&dev->dev,
					"%u:%d : UAC_AS_GENERAL descriptor not found\n",
					iface_no, altno);
				continue;
			}

			if (as->bLength < sizeof(*as)) {
				dev_err(&dev->dev,
					"%u:%d : invalid UAC_AS_GENERAL desc\n",
					iface_no, altno);
				continue;
			}

			num_channels = as->bNrChannels;
			format = le32_to_cpu(as->bmFormats);
			chconfig = le32_to_cpu(as->bmChannelConfig);

			/* lookup the terminal associated to this interface
			 * to extract the clock */
			input_term = snd_usb_find_input_terminal_descriptor(chip->ctrl_intf,
									    as->bTerminalLink);
			if (input_term) {
				clock = input_term->bCSourceID;
				if (!chconfig && (num_channels == input_term->bNrChannels))
					chconfig = le32_to_cpu(input_term->bmChannelConfig);
				break;
			}

			output_term = snd_usb_find_output_terminal_descriptor(chip->ctrl_intf,
									      as->bTerminalLink);
			if (output_term) {
				clock = output_term->bCSourceID;
				break;
			}

			dev_err(&dev->dev,
				"%u:%d : bogus bTerminalLink %d\n",
				iface_no, altno, as->bTerminalLink);
			continue;
		}
		}

		/* get format type */
		fmt = snd_usb_find_csint_desc(alts->extra, alts->extralen, NULL, UAC_FORMAT_TYPE);
		if (!fmt) {
			dev_err(&dev->dev,
				"%u:%d : no UAC_FORMAT_TYPE desc\n",
				iface_no, altno);
			continue;
		}
		if (((protocol == UAC_VERSION_1) && (fmt->bLength < 8)) ||
		    ((protocol == UAC_VERSION_2) && (fmt->bLength < 6))) {
			dev_err(&dev->dev,
				"%u:%d : invalid UAC_FORMAT_TYPE desc\n",
				iface_no, altno);
			continue;
		}

		/*
		 * Blue Microphones workaround: The last altsetting is identical
		 * with the previous one, except for a larger packet size, but
		 * is actually a mislabeled two-channel setting; ignore it.
		 */
		if (fmt->bNrChannels == 1 &&
		    fmt->bSubframeSize == 2 &&
		    altno == 2 && num == 3 &&
		    fp && fp->altsetting == 1 && fp->channels == 1 &&
		    fp->formats == SNDRV_PCM_FMTBIT_S16_LE &&
		    protocol == UAC_VERSION_1 &&
		    le16_to_cpu(get_endpoint(alts, 0)->wMaxPacketSize) ==
							fp->maxpacksize * 2)
			continue;

		fp = kzalloc(sizeof(*fp), GFP_KERNEL);
		if (! fp) {
			dev_err(&dev->dev, "cannot malloc\n");
			return -ENOMEM;
		}

		fp->iface = iface_no;
		fp->altsetting = altno;
		fp->altset_idx = i;
		fp->endpoint = get_endpoint(alts, 0)->bEndpointAddress;
		fp->ep_attr = get_endpoint(alts, 0)->bmAttributes;
		fp->datainterval = snd_usb_parse_datainterval(chip, alts);
		fp->protocol = protocol;
		fp->maxpacksize = le16_to_cpu(get_endpoint(alts, 0)->wMaxPacketSize);
		fp->channels = num_channels;
		if (snd_usb_get_speed(dev) == USB_SPEED_HIGH)
			fp->maxpacksize = (((fp->maxpacksize >> 11) & 3) + 1)
					* (fp->maxpacksize & 0x7ff);
		fp->attributes = parse_uac_endpoint_attributes(chip, alts, protocol, iface_no);
		fp->clock = clock;
		INIT_LIST_HEAD(&fp->list);

		/* ok, let's parse further... */
		if (snd_usb_parse_audio_format(chip, fp, format, fmt, stream) < 0) {
			kfree(fp->rate_table);
			kfree(fp);
			fp = NULL;
			continue;
		}

		/* Create chmap */
		if (fp->channels != num_channels)
			chconfig = 0;

		list_add_tail(&fp->list, fp_list);
	}

	return 0;
}

static int snd_usb_create_stream(struct snd_usb_audio *chip, int ctrlif, int interface, struct list_head *fp_list)
{
	struct usb_device *dev = chip->dev;
	struct usb_host_interface *alts;
	struct usb_interface_descriptor *altsd;
	struct usb_interface *iface = usb_ifnum_to_if(dev, interface);

	if (!iface) {
		dev_err(&dev->dev, "%u:%d : does not exist\n",
			ctrlif, interface);
		return -EINVAL;
	}

	alts = &iface->altsetting[0];
	altsd = get_iface_desc(alts);

	if (usb_interface_claimed(iface)) {
		dev_dbg(&dev->dev, "%d:%d: skipping, already claimed\n",
			ctrlif, interface);
		return -EINVAL;
	}

	if ((altsd->bInterfaceClass == USB_CLASS_AUDIO ||
	     altsd->bInterfaceClass == USB_CLASS_VENDOR_SPEC) &&
	    altsd->bInterfaceSubClass == USB_SUBCLASS_MIDISTREAMING) {
		return -EINVAL;
	}

	if ((altsd->bInterfaceClass != USB_CLASS_AUDIO &&
	     altsd->bInterfaceClass != USB_CLASS_VENDOR_SPEC) ||
	    altsd->bInterfaceSubClass != USB_SUBCLASS_AUDIOSTREAMING) {
		dev_dbg(&dev->dev,
			"%u:%d: skipping non-supported interface %d\n",
			ctrlif, interface, altsd->bInterfaceClass);
		/* skip non-supported classes */
		return -EINVAL;
	}

	if (snd_usb_get_speed(dev) == USB_SPEED_LOW) {
		dev_err(&dev->dev, "low speed audio streaming not supported\n");
		return -EINVAL;
	}

	return snd_usb_parse_audio_interface(chip, interface, fp_list);
}

/*
 * parse audio control descriptor and create pcm/midi streams
 */
static int snd_usb_create_streams(struct snd_usb_audio *chip, int ctrlif, struct list_head *fp_list)
{
	struct usb_device *dev = chip->dev;
	struct usb_host_interface *host_iface;
	struct usb_interface_descriptor *altsd;
	void *control_header;
	int i, protocol;

	/* find audiocontrol interface */
	host_iface = &usb_ifnum_to_if(dev, ctrlif)->altsetting[0];
	control_header = snd_usb_find_csint_desc(host_iface->extra,
						 host_iface->extralen,
						 NULL, UAC_HEADER);
	altsd = get_iface_desc(host_iface);
	protocol = altsd->bInterfaceProtocol;

	if (!control_header) {
		dev_err(&dev->dev, "cannot find UAC_HEADER\n");
		return -EINVAL;
	}

	switch (protocol) {
	default:
		dev_warn(&dev->dev,
			 "unknown interface protocol %#02x, assuming v1\n",
			 protocol);
		/* fall through */

	case UAC_VERSION_1: {
		struct uac1_ac_header_descriptor *h1 = control_header;

		if (!h1->bInCollection) {
			dev_info(&dev->dev, "skipping empty audio interface (v1)\n");
			return -EINVAL;
		}

		if (h1->bLength < sizeof(*h1) + h1->bInCollection) {
			dev_err(&dev->dev, "invalid UAC_HEADER (v1)\n");
			return -EINVAL;
		}

		for (i = 0; i < h1->bInCollection; i++)
			snd_usb_create_stream(chip, ctrlif, h1->baInterfaceNr[i], fp_list);

		break;
	}

	case UAC_VERSION_2: {
		struct usb_interface_assoc_descriptor *assoc =
			usb_ifnum_to_if(dev, ctrlif)->intf_assoc;

		if (!assoc) {
			/*
			 * Firmware writers cannot count to three.  So to find
			 * the IAD on the NuForce UDH-100, also check the next
			 * interface.
			 */
			struct usb_interface *iface =
				usb_ifnum_to_if(dev, ctrlif + 1);
			if (iface &&
			    iface->intf_assoc &&
			    iface->intf_assoc->bFunctionClass == USB_CLASS_AUDIO &&
			    iface->intf_assoc->bFunctionProtocol == UAC_VERSION_2)
				assoc = iface->intf_assoc;
		}

		if (!assoc) {
			dev_err(&dev->dev, "Audio class v2 interfaces need an interface association\n");
			return -EINVAL;
		}

		for (i = 0; i < assoc->bInterfaceCount; i++) {
			int intf = assoc->bFirstInterface + i;

			if (intf != ctrlif)
				snd_usb_create_stream(chip, ctrlif, intf, fp_list);
		}

		break;
	}
	}

	return 0;
}

bool hifi_samplerate_compare(struct audioformat *fp, struct usbaudio_pcm_cfg *pcm_cfg)
{
	int i,j;
	bool found = false;
	unsigned int hifi_playback_rates[] = {192000, 96000, 48000};
	unsigned int hifi_capture_rates[] = {96000, 48000};

	unsigned int max_rate = 48000;
	unsigned int min_rate = 96000;

	for (i = 0; i < fp->nr_rates; i++) {
		if (fp->endpoint & USB_DIR_IN) {
			for (j = 0; j < ARRAY_SIZE(hifi_capture_rates); j++) {
				if (hifi_capture_rates[j] == fp->rate_table[i]) {
					found = true;
					if (min_rate >= fp->rate_table[i]) {
						min_rate = fp->rate_table[i];
						pcm_cfg[SNDRV_PCM_STREAM_CAPTURE].rates = min_rate;
					}
				}
			}
		} else {
			for (j = 0; j < ARRAY_SIZE(hifi_playback_rates); j++) {
				if (hifi_playback_rates[j] == fp->rate_table[i]) {
					found = true;
					if (max_rate <= fp->rate_table[i]) {
						max_rate = fp->rate_table[i];
						pcm_cfg[SNDRV_PCM_STREAM_PLAYBACK].rates = max_rate;
					}
				}
			}
		}
	}

	return found;
}

bool hifi_formats_compare(struct audioformat *fp, struct usbaudio_pcm_cfg *pcm_cfg)
{
	int i;
	bool found = false;
	unsigned long long hifi_playback_formats[] = {SNDRV_PCM_FMTBIT_S32_LE, SNDRV_PCM_FMTBIT_S24_LE, SNDRV_PCM_FMTBIT_S24_3LE, SNDRV_PCM_FMTBIT_S16_LE};
	unsigned long long hifi_capture_formats[] = {SNDRV_PCM_FMTBIT_S16_LE};

	if (fp->endpoint & USB_DIR_IN) {
		for (i = 0; i < ARRAY_SIZE(hifi_capture_formats); i++) {
			if (fp->formats == hifi_capture_formats[i]) {
				found = true;
				pcm_cfg[SNDRV_PCM_STREAM_CAPTURE].formats = fp->formats;
			}
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(hifi_playback_formats); i++) {
			if (fp->formats == hifi_playback_formats[i]) {
				found = true;
				pcm_cfg[SNDRV_PCM_STREAM_PLAYBACK].formats = fp->formats;
			}
		}
	}

	return found;
}

bool hifi_channel_compare(struct audioformat *fp, struct usbaudio_pcm_cfg *pcm_cfg)
{
	int i;
	bool found = false;
	unsigned int hifi_playback_channels[] = {2};
	unsigned int hifi_capture_channels[] = {2, 1};

	if (fp->endpoint & USB_DIR_IN) {
		for (i = 0; i < ARRAY_SIZE(hifi_capture_channels); i++) {
			if (fp->channels == hifi_capture_channels[i]) {
				found = true;
				pcm_cfg[SNDRV_PCM_STREAM_CAPTURE].channels = fp->channels;
			}
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(hifi_playback_channels); i++) {
			if (fp->channels == hifi_playback_channels[i]) {
				found = true;
				pcm_cfg[SNDRV_PCM_STREAM_PLAYBACK].channels = fp->channels;
			}
		}
	}

	return found;
}

bool hifi_usbaudio_protocal_compare(struct usb_device *dev, struct audioformat *fp)
{
	struct usb_host_interface *alts;
	struct usb_interface_descriptor *altsd;
	struct usb_interface *iface;

	iface = usb_ifnum_to_if(dev, fp->iface);
	if (WARN_ON(!iface))
		return false;

	alts = &iface->altsetting[fp->altset_idx];
	altsd = get_iface_desc(alts);

	if (altsd->bNumEndpoints != 1)
		return false;

	if ((fp->protocol == UAC_VERSION_1 || fp->protocol == UAC_VERSION_2) &&
		(fp->fmt_type == UAC_FORMAT_TYPE_I) &&
		(fp->datainterval == 0))
		return true;

	return false;
}

snd_pcm_format_t pcm_fomat_convert(unsigned long long format)
{
	snd_pcm_format_t pcm_format = SNDRV_PCM_FORMAT_S16_LE;
	switch (format) {
	case SNDRV_PCM_FMTBIT_S16_LE:
		pcm_format = SNDRV_PCM_FORMAT_S16_LE;
		break;
	case SNDRV_PCM_FMTBIT_S24_LE:
		pcm_format = SNDRV_PCM_FORMAT_S24_LE;
		break;
	case SNDRV_PCM_FMTBIT_S24_3LE:
		pcm_format = SNDRV_PCM_FORMAT_S24_3LE;
		break;
	case SNDRV_PCM_FMTBIT_S32_LE:
		pcm_format = SNDRV_PCM_FORMAT_S32_LE;
		break;
	default:
		pr_err("format not support %lld \n", format);
		break;
	}

	return pcm_format;
}

bool match_hifi_format(struct usb_device *dev, struct list_head *fp_list, struct usbaudio_pcms *pcms)
{
	struct audioformat *fp, *n;
	struct usb_host_interface *alts;
	struct usb_interface_descriptor *altsd;
	struct usb_interface *iface;
	struct usb_endpoint_descriptor *epd;
	struct usbaudio_pcm_cfg pcm_cfg[2];
	int dir = 0;
	bool playback_match = false;
	bool capture_match = false;
	memset(pcms, 0, sizeof(*pcms));

	list_for_each_entry(fp, fp_list, list) {
		if (hifi_usbaudio_protocal_compare(dev, fp) &&
			hifi_samplerate_compare(fp, pcm_cfg) &&
			hifi_formats_compare(fp, pcm_cfg) &&
			hifi_channel_compare(fp, pcm_cfg)) {
			if (fp->endpoint & USB_DIR_IN) {
				capture_match = true;
				dir = SNDRV_PCM_STREAM_CAPTURE;
			} else {
				playback_match = true;
				dir = SNDRV_PCM_STREAM_PLAYBACK;
			}

			iface = usb_ifnum_to_if(dev, fp->iface);
			if (WARN_ON(!iface))
				return false;
			alts = &iface->altsetting[fp->altset_idx];
			altsd = get_iface_desc(alts);
			epd = get_endpoint(alts, 0);

			pcms->fmts[dir].formats = pcm_fomat_convert(fp->formats);
			pcms->fmts[dir].channels = fp->channels;
			pcms->fmts[dir].fmt_type = fp->fmt_type;
			pcms->fmts[dir].frame_size = fp->frame_size;
			pcms->fmts[dir].iface = fp->iface;
			pcms->fmts[dir].altsetting = fp->altsetting;
			pcms->fmts[dir].altset_idx = fp->altset_idx;
			pcms->fmts[dir].attributes = fp->attributes;
			pcms->fmts[dir].endpoint = fp->endpoint;
			pcms->fmts[dir].ep_attr = fp->ep_attr;
			pcms->fmts[dir].datainterval = fp->datainterval;
			pcms->fmts[dir].protocol = fp->protocol;
			pcms->fmts[dir].maxpacksize = fp->maxpacksize;
			pcms->fmts[dir].rates = pcm_cfg[dir].rates;
			pcms->fmts[dir].clock = fp->clock;

			pcms->ifdesc[dir].bLength = altsd->bLength;
			pcms->ifdesc[dir].bDescriptorType = altsd->bDescriptorType;
			pcms->ifdesc[dir].bInterfaceNumber = altsd->bInterfaceNumber;
			pcms->ifdesc[dir].bAlternateSetting = altsd->bAlternateSetting;
			pcms->ifdesc[dir].bNumEndpoints = altsd->bNumEndpoints;
			pcms->ifdesc[dir].bInterfaceClass = altsd->bInterfaceClass;
			pcms->ifdesc[dir].bInterfaceSubClass = altsd->bInterfaceSubClass;
			pcms->ifdesc[dir].bInterfaceProtocol = altsd->bInterfaceProtocol;
			pcms->ifdesc[dir].iInterface = altsd->iInterface;

			pcms->epdesc[dir].bLength = epd->bLength;
			pcms->epdesc[dir].bDescriptorType = epd->bDescriptorType;
			pcms->epdesc[dir].bEndpointAddress = epd->bEndpointAddress;
			pcms->epdesc[dir].bmAttributes = epd->bmAttributes;
			pcms->epdesc[dir].wMaxPacketSize = epd->wMaxPacketSize;
			pcms->epdesc[dir].bInterval = epd->bInterval;
			pcms->epdesc[dir].bRefresh = epd->bRefresh;
			pcms->epdesc[dir].bSynchAddress = epd->bSynchAddress;
		}
	}

	list_for_each_entry_safe(fp, n, fp_list, list) {
		kfree(fp->rate_table);
		kfree(fp->chmap);
		kfree(fp);
	}

	if (playback_match&capture_match) {
		return true;
	} else {
		return false;
	}
}

bool is_match_hifi_format(struct usb_device *dev, u32 usb_id, struct usb_host_interface *ctrl_intf, int ctrlif, struct usbaudio_pcms *pcms)
{
	struct snd_usb_audio chip;
	struct list_head fp_list;
	int ret;

	chip.dev = dev;
	chip.ctrl_intf = ctrl_intf;
	chip.usb_id = usb_id;

	INIT_LIST_HEAD(&fp_list);
	ret = snd_usb_create_streams(&chip, ctrlif, &fp_list);
	if (ret != 0)
		return false;

	return match_hifi_format(dev, &fp_list, pcms);
}

bool is_usbaudio_device(struct usb_device *udev)
{
	struct usb_host_config *config = NULL;
	int hid_intf_num = 0, audio_intf_num = 0, other_intf_num = 0;
	int nintf;
	int i;

	/* root hub */
	if (udev->parent == NULL)
		return false;
	/* not connect to roothub */
	if (udev->parent->parent != NULL)
		return false;

	config = udev->actconfig;
	if (!config) {
		pr_err("config is null\n");
		return false;
	}

	nintf = config->desc.bNumInterfaces;
	if ((nintf < 0) || (nintf > USB_MAXINTERFACES)) {
		pr_err("nintf invalid %d\n", nintf);
		return false;
	}

	for (i = 0; i < nintf; ++i) {
		struct usb_interface_cache *intfc;
		struct usb_host_interface *alt;

		intfc = config->intf_cache[i];
		alt = &intfc->altsetting[0];

		if (alt->desc.bInterfaceClass == USB_CLASS_AUDIO) {
			if (alt->desc.bInterfaceSubClass == USB_SUBCLASS_AUDIOCONTROL)
				audio_intf_num++;
		} else if (alt->desc.bInterfaceClass == USB_CLASS_HID)
			hid_intf_num++;
		else
			other_intf_num++;
	}

	pr_info("audio_intf_num %d, hid_intf_num %d, other_intf_num %d\n",
		audio_intf_num, hid_intf_num, other_intf_num);

	if ((audio_intf_num == 1) && (hid_intf_num <= 1) && (other_intf_num == 0)) {
		pr_info("[%s]to hisi_usb_start_hifi_usb\n", __func__);
		return true;
	}

	return false;
}

bool is_special_usbid(u32 usb_id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(special_usb_ids); i++) {
		if (usb_id == special_usb_ids[i])
			return true;
	}

	return false;
}

bool controller_switch(struct usb_device *dev, u32 usb_id, struct usb_host_interface *ctrl_intf, int ctrlif, struct usbaudio_pcms *pcms)
{
	if ((!hisi_usb_using_hifi_usb(dev)) && is_usbaudio_device(dev) && !is_special_usbid(usb_id) && is_match_hifi_format(dev, usb_id, ctrl_intf, ctrlif, pcms)) {
		if ( !hisi_usb_start_hifi_usb()) {
			return true;
		}
	}

	return false;
}

bool send_usbaudioinfo2hifi(struct snd_usb_audio *chip, struct usbaudio_pcms *pcms)
{
	int i = 0;
	if (hisi_usb_using_hifi_usb(chip->dev)) {
		if(0 == usbaudio_probe_msg(pcms)) {
			for (i = 0; i < 2; i++) {
				pr_err("-----------begin \n");
				pr_err("formats %lld channels %d fmt_type %d frame_size %d iface %d altsetting %d altset_idx %d attributes %d endpoint %d ep_attr %d datainterval %d protocol %d maxpacksize %d rates %d clock %d\n",
				pcms->fmts[i].formats,
				pcms->fmts[i].channels,
				pcms->fmts[i].fmt_type,
				pcms->fmts[i].frame_size,
				pcms->fmts[i].iface,
				pcms->fmts[i].altsetting,
				pcms->fmts[i].altset_idx,
				pcms->fmts[i].attributes,
				pcms->fmts[i].endpoint,
				pcms->fmts[i].ep_attr,
				pcms->fmts[i].datainterval,
				pcms->fmts[i].protocol,
				pcms->fmts[i].maxpacksize,
				pcms->fmts[i].rates,
				pcms->fmts[i].clock);

				pr_err("bLength %d bDescriptorType %d bInterfaceNumber %d bAlternateSetting %d bNumEndpoints %d bInterfaceClass  %d bInterfaceSubClass  %d bInterfaceProtocol %d iInterface %d\n",
				pcms->ifdesc[i].bLength,
				pcms->ifdesc[i].bDescriptorType,
				pcms->ifdesc[i].bInterfaceNumber,
				pcms->ifdesc[i].bAlternateSetting,
				pcms->ifdesc[i].bNumEndpoints,
				pcms->ifdesc[i].bInterfaceClass,
				pcms->ifdesc[i].bInterfaceSubClass,
				pcms->ifdesc[i].bInterfaceProtocol,
				pcms->ifdesc[i].iInterface);

				pr_err("bLength %d bDescriptorType %d bEndpointAddress %d bmAttributes %d wMaxPacketSize %d bInterval %d bRefresh %d bSynchAddress %d \n",
				pcms->epdesc[i].bLength,
				pcms->epdesc[i].bDescriptorType,
				pcms->epdesc[i].bEndpointAddress,
				pcms->epdesc[i].bmAttributes,
				pcms->epdesc[i].wMaxPacketSize,
				pcms->epdesc[i].bInterval,
				pcms->epdesc[i].bRefresh,
				pcms->epdesc[i].bSynchAddress);
				pr_err("-----------end \n");
			}

			return true;
		} else {
			pr_err("send 2 hifi fail \n");
			return false;
		}
	}

	return true;
}

static int usbaudio_set_interface(int direction, struct snd_usb_audio *chip, struct usbaudio_pcms *pcms, unsigned int running)
{
	int ret;
	struct usb_host_interface *alts;
	struct usb_interface *iface;
	struct usbaudio_formats *fmt;
	struct audioformat cur_audiofmt;

	fmt = &pcms->fmts[direction];
	pr_info("%s,direction %d running %d iface %d altsetting %d \n",__FUNCTION__, direction, running, fmt->iface, fmt->altsetting);
	if (running == START_STREAM) {
		cur_audiofmt.attributes = fmt->attributes;
		cur_audiofmt.altsetting = fmt->altsetting;
		cur_audiofmt.protocol = fmt->protocol;
		cur_audiofmt.clock = fmt->clock;
		iface = usb_ifnum_to_if(chip->dev, fmt->iface);
		alts = &iface->altsetting[fmt->altset_idx];
		ret = snd_usb_init_sample_rate(chip,
					       fmt->iface,
					       alts,
					       &cur_audiofmt,
					       fmt->rates);
		if (ret < 0) {
			pr_err("%d:%d: init_sample_rate failed (%d)\n", fmt->iface, fmt->rates, ret);
			return -EIO;
		}
	}

	ret = usb_set_interface(chip->dev, fmt->iface, 0);
	if (ret < 0) {
		pr_err("%d:%d: usb_set_interface failed (%d)\n", fmt->iface, 0, ret);
		return -EIO;
	}

	if (running == START_STREAM) {
		ret = usb_set_interface(chip->dev, fmt->iface, fmt->altsetting);
		if (ret < 0) {
			pr_err("%d:%d: usb_set_interface failed (%d)\n", fmt->iface, fmt->altsetting, ret);
			return -EIO;
		}
	}

	return ret;
}

int set_pipeout_interface(struct snd_usb_audio *chip, struct usbaudio_pcms *pcms, unsigned int running)
{
	return usbaudio_set_interface(SNDRV_PCM_STREAM_PLAYBACK, chip, pcms, running);
}

int set_pipein_interface(struct snd_usb_audio *chip, struct usbaudio_pcms *pcms, unsigned int running)
{
	return usbaudio_set_interface(SNDRV_PCM_STREAM_CAPTURE, chip, pcms, running);
}

bool controller_query(struct snd_usb_audio *chip)
{
	return hisi_usb_using_hifi_usb(chip->dev);
}

int disconnect(struct snd_usb_audio *chip)
{
	if (hisi_usb_using_hifi_usb(chip->dev))
		return usbaudio_disconnect_msg();
	else
		return 0;
}

