/*
 * vdec driver interface
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */

#include <linux/clk.h>
#include <linux/of.h>
#include <linux/io.h>

#include "omxvdec.h"
#include "linux_kernel_osal.h"
#include "vfmw_dts.h"
#include "scd_drv.h"
#include "vdm_drv.h"
#include "bitplane.h"
#include "vfmw_intf.h"
/*lint -e774*/

#define PCTRL_PERI              (0xE8A090A4)
#define PCTRL_PERI_SATA0        (0xE8A090BC)

static HI_S32 gIsNormalInit = 0;

HI_U32 g_TraceOption = (1<<OMX_FATAL) + (1<<OMX_ERR);
VDEC_PROC_CMD pfun_vdec_proc_cmd        = NULL;
static HI_BOOL       gIsDeviceDetected  = HI_FALSE;
static struct class *g_OmxVdecClass     = HI_NULL;
static const HI_CHAR g_OmxVdecDrvName[] = OMXVDEC_NAME;
static dev_t         g_OmxVdecDevNum;
static OMXVDEC_ENTRY g_OmxVdecEntry;

#ifdef CONFIG_COMPAT //Modified for 64-bit platform
typedef enum {
	T_IOCTL_ARG,
	T_USER_BUFFER,
	T_BUF_MSG,
	T_SM_CONFIG,
	T_BUTT,
} COMPAT_TYPE_E;
#endif
typedef enum {
	KIRIN_960,
	KIRIN_970_ES,
	KIRIN_970_CS,
	KIRIN_980,
	KIRIN_660,
	KIRIN_BUTT,
} KIRIN_PLATFORM_E;

static HI_S32 omxvdec_vfmw_init(HI_VOID)
{
	return VCTRL_OpenVfmw();
}

static HI_VOID omxvdec_vfmw_deinit(HI_VOID)
{
	VCTRL_CloseVfmw();
}

static HI_S32 omxvdec_setup_cdev(OMXVDEC_ENTRY *omxvdec, const struct file_operations *fops)
{
	HI_S32 rc;
	struct device *dev;

	g_OmxVdecClass = class_create(THIS_MODULE, "omxvdec_class");
	if (IS_ERR(g_OmxVdecClass)) {
		rc = PTR_ERR(g_OmxVdecClass);
		g_OmxVdecClass = HI_NULL;
		OmxPrint(OMX_FATAL, "%s call class_create failed, rc : %d\n", __func__, rc);
		return rc;
	}

	rc = alloc_chrdev_region(&g_OmxVdecDevNum, 0, 1, "hisi video decoder");
	if (rc) {
		OmxPrint(OMX_FATAL, "%s call alloc_chrdev_region failed, rc : %d\n", __func__, rc);
		goto cls_destroy;
	}

	dev = device_create(g_OmxVdecClass, NULL, g_OmxVdecDevNum, NULL, OMXVDEC_NAME);
	if (IS_ERR(dev)) {
		rc = PTR_ERR(dev);
		OmxPrint(OMX_FATAL, "%s call device_create failed, rc : %d\n", __func__, rc);
		goto unregister_region;
	}

	cdev_init(&omxvdec->cdev, fops);
	omxvdec->cdev.owner = THIS_MODULE;
	omxvdec->cdev.ops = fops;
	rc = cdev_add(&omxvdec->cdev, g_OmxVdecDevNum, 1);
	if (rc < 0) {
		OmxPrint(OMX_FATAL, "%s call cdev_add failed, rc : %d\n", __func__, rc);
		goto dev_destroy;
	}

	return HI_SUCCESS;

dev_destroy:
	device_destroy(g_OmxVdecClass, g_OmxVdecDevNum);
unregister_region:
	unregister_chrdev_region(g_OmxVdecDevNum, 1);
cls_destroy:
	class_destroy(g_OmxVdecClass);
	g_OmxVdecClass = HI_NULL;

	return rc;
}

static HI_S32 omxvdec_cleanup_cdev(OMXVDEC_ENTRY *omxvdec)
{
	if (g_OmxVdecClass == HI_NULL) {
		OmxPrint(OMX_FATAL, "%s: invalid g_OmxVdecClass is NULL", __func__);
		return HI_FAILURE;
	} else {
		cdev_del(&omxvdec->cdev);
		device_destroy(g_OmxVdecClass, g_OmxVdecDevNum);
		unregister_chrdev_region(g_OmxVdecDevNum, 1);
		class_destroy(g_OmxVdecClass);
		return HI_SUCCESS;
	}
}

static HI_S32 omxvdec_open(struct inode *inode, struct file *fd)
{
	HI_S32 ret = -EBUSY;
	OMXVDEC_ENTRY *omxvdec = HI_NULL;

	omxvdec = container_of(inode->i_cdev, OMXVDEC_ENTRY, cdev);

	VDEC_DOWN_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);

	if (omxvdec->open_count < MAX_OPEN_COUNT) {
		omxvdec->open_count++;
		if (omxvdec->open_count == 1) {

			VDEC_Regulator_Enable();
			ret = omxvdec_vfmw_init();
			if (ret != HI_SUCCESS) {
				OmxPrint(OMX_FATAL, "%s : omxvdec_vfmw_init failed\n", __func__);
				goto error0;
			}
			gIsNormalInit = 1;
		}

		fd->private_data = omxvdec;
		ret = HI_SUCCESS;
	} else {
		printk(KERN_CRIT "%s open omxvdec instance too much\n", __func__);
		ret = -EBUSY;
	}

	VDEC_UP_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);

	return ret;

error0:
	VDEC_Regulator_Disable();
	omxvdec->open_count--;
	VDEC_UP_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);

	return ret;
}

static HI_S32 omxvdec_release(struct inode *inode, struct file *fd)
{
	OMXVDEC_ENTRY *omxvdec = HI_NULL;

	omxvdec = fd->private_data;
	if (omxvdec == HI_NULL) {
		printk(KERN_CRIT "%s: invalid omxvdec is null\n", __func__);
		return -EFAULT;
	}

	VDEC_DOWN_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);

	if (fd->private_data == HI_NULL) {
		OmxPrint(OMX_FATAL, "%s: invalid fd->private_data is null\n", __func__);
		VDEC_UP_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);
		return -EFAULT;
	}

	if (omxvdec->open_count > 0)
		omxvdec->open_count--;

	if (omxvdec->open_count == 0) {
		omxvdec_vfmw_deinit();
		VDEC_Regulator_Disable();
		gIsNormalInit = 0;
	}

	fd->private_data = HI_NULL;

	printk(KERN_INFO "exit %s , open_count : %d\n", __func__, omxvdec->open_count);

	VDEC_UP_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);

	return 0;
}

static long omxvdec_ioctl(struct file *fd, unsigned int code, unsigned long arg)
{
	HI_S32 ret;
	OMXVDEC_IOCTL_MSG  vdec_msg;
	HI_VOID           *u_arg   = (HI_VOID *)arg;
	OMXVDEC_ENTRY     *omxvdec = fd->private_data;

	CLK_RATE_E        clk_rate;
	VFMW_DTS_CONFIG_S dts_config;
	OMXSCD_REG_CFG_S  scd_reg_cfg;
	SCD_STATE_REG_S   scd_state_reg;
	OMXVDH_REG_CFG_S  vdm_reg_cfg;
	VDMHAL_BACKUP_S   vdm_state_reg;
	OMXBPD_REG_S      bpd_reg;
	HI_S32            vdm_is_run;

	if (!gIsNormalInit)
		return -EIO;

	if (VCTRL_Scen_Ident()) {
		OmxPrint(OMX_ALWS, "xxx mode\n");
		return -EIO;
	}

	if (copy_from_user(&vdec_msg, u_arg, sizeof(vdec_msg))) {
		OmxPrint(OMX_FATAL, "%s call copy_from_user failed \n", __func__);
		return -EFAULT;
	}

	switch (code) {
	case VDEC_IOCTL_GET_OPEN_COUNT:
		if (HI_NULL == omxvdec) {
			OmxPrint(OMX_FATAL, "%s: invalid omxvdec is null\n", __func__);
			return HI_FAILURE;
		}

		VDEC_DOWN_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);

		ret = copy_to_user(vdec_msg.out, &(omxvdec->open_count), sizeof(omxvdec->open_count));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_GET_OPEN_COUNT :  copy_to_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);
			return -EIO;
		}

		VDEC_UP_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);

		break;

	case VDEC_IOCTL_POWER_CTRL:
		break;

	case VDEC_IOCTL_SET_CLK_RATE:
		ret = copy_from_user(&clk_rate, vdec_msg.in, sizeof(clk_rate));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_SET_CLK_RATE : copy_from_user failed\n");
			return -EIO;
		}

		if (VDEC_Regulator_SetClkRate(clk_rate) != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_Regulator_SetClkRate faied\n");
			return HI_FAILURE;
		}
		break;

	case VDEC_IOCTL_GET_CLK_RATE:
		if (VDEC_Regulator_GetClkRate(&clk_rate) == HI_SUCCESS) {
			ret = copy_to_user(vdec_msg.out, &clk_rate, sizeof(clk_rate));
			if (ret != HI_SUCCESS) {
				OmxPrint(OMX_FATAL, "VDEC_IOCTL_GET_CLK_RATE: copy_to_user failed\n");
				return -EIO;
			}
		} else {
			OmxPrint(OMX_FATAL, "VDEC_Regulator_GetClkRate faied\n");
			return HI_FAILURE;
		}
		break;

	case VDEC_IOCTL_VDM_PROC:
		VDEC_DOWN_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
		ret = copy_from_user(&vdm_reg_cfg, vdec_msg.in, sizeof(vdm_reg_cfg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_VDM_PROC : copy_from_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
			return -EIO;
		}

		ret = VCTRL_VDMHal_Process(&vdm_reg_cfg, &vdm_state_reg);
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VCTRL_VDMHal_Process failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
			return -EIO;
		}

		ret = copy_to_user(vdec_msg.out, &vdm_state_reg, sizeof(vdm_state_reg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_VDM_PROC : copy_to_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
			return -EIO;
		}
		VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
		break;

	case VDEC_IOCTL_GET_VDM_HWSTATE:
		vdm_is_run = VCTRL_VDMHAL_IsRun();
		ret = copy_to_user(vdec_msg.out, &vdm_is_run, sizeof(vdm_is_run));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_GET_VDM_HWSTATE :copy_to_user failed\n");
			return -EIO;
		}
		break;

	case VDEC_IOCTL_GET_DTS_CONFIG:
		if (VFMW_GetDtsConfig(&dts_config) == VDEC_OK) {
			ret = copy_to_user(vdec_msg.out, &dts_config, sizeof(dts_config));
			if (ret != HI_SUCCESS) {
				OmxPrint(OMX_FATAL, "VDEC_IOCTL_GET_DTS_CONFIG : copy_to_user failed\n");
				return -EIO;
			}
		} else {
			OmxPrint(OMX_FATAL, "VFMW_GetDtsConfig failed\n");
			return VDEC_ERR;
		}
		break;

	case VDEC_IOCTL_SCD_PROC:
		VDEC_DOWN_INTERRUPTIBLE(&omxvdec->vdec_mutex_scd);
		ret = copy_from_user(&scd_reg_cfg, vdec_msg.in, sizeof(scd_reg_cfg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_SCD_PROC : copy_from_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_scd);
			return -EIO;
		}

		ret = VCTRL_SCDHal_Process(&scd_reg_cfg, &scd_state_reg);
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, " VCTRL_SCDHal_Process failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_scd);
			return -EIO;
		}

		ret = copy_to_user(vdec_msg.out, &scd_state_reg, sizeof(scd_state_reg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_SCD_PROC : copy_to_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_scd);
			return -EIO;
		}
		VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_scd);
		break;

	case VDEC_IOCTL_SET_BPD_PROC:
		VDEC_DOWN_INTERRUPTIBLE(&omxvdec->vdec_mutex_bpd);
		ret = copy_from_user(&bpd_reg, vdec_msg.in, sizeof(bpd_reg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_SET_BPD_PROC : copy_from_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_bpd);
			return -EIO;
		}

		ret = VCTRL_BPDHal_Process(&bpd_reg);
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VCTRL_BPDHal_Process failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_bpd);
			return -EIO;
		}

		ret = copy_to_user(vdec_msg.out, &bpd_reg, sizeof(bpd_reg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_SET_BPD_PROC :  copy_to_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_bpd);
			return -EIO;
		}
		VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_bpd);
		break;
	default:
		/* could not handle ioctl */
		OmxPrint(OMX_FATAL, "%s %d: ERROR cmd : %d is not supported\n", __func__, __LINE__, _IOC_NR(code));
		return -ENOTTY;
	}
	return 0;

}

#ifdef CONFIG_COMPAT   //Modified for 64-bit platform
static HI_S32 omxvdec_compat_get_data(COMPAT_TYPE_E eType, HI_VOID __user *pUser, HI_VOID *pData)
{
	HI_S32 ret = HI_SUCCESS;
	HI_S32 s32Data = 0;
	compat_ulong_t CompatData = 0;

	if (HI_NULL == pUser || HI_NULL == pData) {
		OmxPrint(OMX_FATAL, "%s: invalid param is null\n", __func__);
		return HI_FAILURE;
	}

	switch (eType) {
	case T_IOCTL_ARG:{
		COMPAT_IOCTL_MSG __user *pCompatMsg = pUser;
		OMXVDEC_IOCTL_MSG       *pIoctlMsg  = pData;

		ret |= get_user(s32Data, &(pCompatMsg->chan_num));
		pIoctlMsg->chan_num = s32Data;

		ret |= get_user(CompatData, &(pCompatMsg->in));
		pIoctlMsg->in   = (HI_VOID *) ((HI_VIRT_ADDR_T)CompatData);

		ret |= get_user(CompatData, &(pCompatMsg->out));
		pIoctlMsg->out = (HI_VOID *) ((HI_VIRT_ADDR_T)CompatData);
	}
		break;

	case T_USER_BUFFER: {
		COMPAT_BUF_DESC __user *pCompatBuf  = pUser;
		OMXVDEC_BUF_DESC       *pIoctlBuf   = pData;

		ret |= copy_from_user(pIoctlBuf, pCompatBuf, sizeof(*pCompatBuf));

		ret |= get_user(CompatData, &(pCompatBuf->bufferaddr));
		pIoctlBuf->bufferaddr = (HI_VOID *) ((HI_VIRT_ADDR_T)CompatData);

		ret |= get_user(CompatData, &(pCompatBuf->client_data));
		pIoctlBuf->client_data = (HI_VOID *) ((HI_VIRT_ADDR_T)CompatData);
	}
		break;

	case T_SM_CONFIG: {
		COMPAT_OMXSCD_REG_CFG_S __user *pCompatCfg = pUser;
		OMXSCD_REG_CFG_S               *pIoctlCfg  = pData;
		ret |= copy_from_user(pIoctlCfg, pCompatCfg, sizeof(*pCompatCfg));
		ret |= get_user(CompatData, &(pCompatCfg->IoctlSmCtrlReg.pDownMsgVirAddr));
		pIoctlCfg->SmCtrlReg.pDownMsgVirAddr = (SINT32 *) ((HI_VIRT_ADDR_T)CompatData);
		ret |= get_user(CompatData, &(pCompatCfg->IoctlSmCtrlReg.pUpMsgVirAddr));
		pIoctlCfg->SmCtrlReg.pUpMsgVirAddr = (SINT32 *) ((HI_VIRT_ADDR_T)CompatData);
		ret |= get_user(CompatData, &(pCompatCfg->IoctlSmCtrlReg.pDspSpsMsgMemVirAddr));
		pIoctlCfg->SmCtrlReg.pDspSpsMsgMemVirAddr = (SINT32 *) ((HI_VIRT_ADDR_T)CompatData);
		ret |= get_user(CompatData, &(pCompatCfg->IoctlSmCtrlReg.pDspPpsMsgMemVirAddr));
		pIoctlCfg->SmCtrlReg.pDspPpsMsgMemVirAddr = (SINT32 *) ((HI_VIRT_ADDR_T)CompatData);
		ret |= get_user(CompatData, &(pCompatCfg->IoctlSmCtrlReg.pDvmMemVirAddr));
		pIoctlCfg->SmCtrlReg.pDvmMemVirAddr = (SINT32 *) ((HI_VIRT_ADDR_T)CompatData);
		ret |= get_user(CompatData, &(pCompatCfg->IoctlSmCtrlReg.pDspSedTopMemVirAddr));
		pIoctlCfg->SmCtrlReg.pDspSedTopMemVirAddr = (SINT32 *) ((HI_VIRT_ADDR_T)CompatData);
		ret |= get_user(CompatData, &(pCompatCfg->IoctlSmCtrlReg.pDspCaMnMemVirAddr));
		pIoctlCfg->SmCtrlReg.pDspCaMnMemVirAddr = (SINT32 *) ((HI_VIRT_ADDR_T)CompatData);
	}
		break;

	default:
		OmxPrint(OMX_FATAL, "%s: unkown type %d\n", __func__, eType);
		ret = HI_FAILURE;
		break;
	}

	return ret;
}

static long omxvdec_compat_ioctl(struct file *fd, unsigned int code, unsigned long arg)
{
	HI_S32 ret;
	OMXVDEC_IOCTL_MSG  vdec_msg;
	OMXVDEC_ENTRY     *omxvdec = fd->private_data;

	CLK_RATE_E        clk_rate;
	VFMW_DTS_CONFIG_S dts_config;
	OMXSCD_REG_CFG_S  scd_reg_cfg;
	SCD_STATE_REG_S   scd_state_reg;
	OMXVDH_REG_CFG_S  vdm_reg_cfg;
	VDMHAL_BACKUP_S   vdm_state_reg;
	OMXBPD_REG_S      bpd_reg;
	HI_S32            vdm_is_run;

	if (!gIsNormalInit)
		return -EIO;

	if (VCTRL_Scen_Ident()) {
		OmxPrint(OMX_ALWS, "xxx mode\n");
		return -EIO;
	}

	ret = omxvdec_compat_get_data(T_IOCTL_ARG, compat_ptr(arg), &vdec_msg);
	if (ret != HI_SUCCESS) {
		OmxPrint(OMX_FATAL, "%s: T_IOCTL_MSG get data failed\n", __func__);
		return -EIO;
	}

	switch (code) {
	case VDEC_IOCTL_GET_OPEN_COUNT:
		if (HI_NULL == omxvdec) {
			OmxPrint(OMX_FATAL, "%s: invalid omxvdec is null\n", __func__);
			return HI_FAILURE;
		}

		VDEC_DOWN_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);

		ret = copy_to_user(vdec_msg.out, &(omxvdec->open_count), sizeof(omxvdec->open_count));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_GET_OPEN_COUNT :  copy_to_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);
			return -EIO;
		}

		VDEC_UP_INTERRUPTIBLE(&omxvdec->omxvdec_mutex);

		break;

	case VDEC_IOCTL_POWER_CTRL:
		break;

	case VDEC_IOCTL_SET_CLK_RATE:
		ret = copy_from_user(&clk_rate, vdec_msg.in, sizeof(clk_rate));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_SET_CLK_RATE : copy_from_user failed\n");
			return -EIO;
		}

		if (VDEC_Regulator_SetClkRate(clk_rate) != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_Regulator_SetClkRate faied\n");
			return HI_FAILURE;
		}
		break;

	case VDEC_IOCTL_GET_CLK_RATE:
		if (VDEC_Regulator_GetClkRate(&clk_rate) == HI_SUCCESS) {
			ret = copy_to_user(vdec_msg.out, &clk_rate, sizeof(clk_rate));
			if (ret != HI_SUCCESS) {
				OmxPrint(OMX_FATAL, "VDEC_IOCTL_GET_CLK_RATE: copy_to_user failed\n");
				return -EIO;
			}
		} else {
			OmxPrint(OMX_FATAL, "VDEC_Regulator_GetClkRate faied\n");
			return HI_FAILURE;
		}
		break;

	case VDEC_IOCTL_VDM_PROC:
		VDEC_DOWN_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
		ret = copy_from_user(&vdm_reg_cfg, vdec_msg.in, sizeof(vdm_reg_cfg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_VDM_PROC : copy_from_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
			return -EIO;
		}

		ret = VCTRL_VDMHal_Process(&vdm_reg_cfg, &vdm_state_reg);
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, " VCTRL_VDMHal_Process failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
			return -EIO;
		}

		ret = copy_to_user(vdec_msg.out, &vdm_state_reg, sizeof(vdm_state_reg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_VDM_PROC : copy_to_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
			return -EIO;
		}
		VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
		break;

	case VDEC_IOCTL_GET_VDM_HWSTATE:
		VDEC_DOWN_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
		vdm_is_run = VCTRL_VDMHAL_IsRun();
		ret = copy_to_user(vdec_msg.out, &vdm_is_run, sizeof(vdm_is_run));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_GET_VDM_HWSTATE copy_to_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
			return -EIO;
		}
		VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_vdh);
		break;

	case VDEC_IOCTL_GET_DTS_CONFIG:
		if (VFMW_GetDtsConfig(&dts_config) == VDEC_OK) {
			ret = copy_to_user(vdec_msg.out, &dts_config, sizeof(dts_config));
			if (ret != HI_SUCCESS) {
				OmxPrint(OMX_FATAL, "VDEC_IOCTL_GET_DTS_CONFIG : copy_to_user failed\n");
				return -EIO;
			}
		} else {
			OmxPrint(OMX_FATAL, "VFMW_GetDtsConfig failed\n");
			return VDEC_ERR;
		}
		break;

	case VDEC_IOCTL_SCD_PROC:
		VDEC_DOWN_INTERRUPTIBLE(&omxvdec->vdec_mutex_scd);
		ret = omxvdec_compat_get_data(T_SM_CONFIG, vdec_msg.in, &scd_reg_cfg);
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_SCD_PROC :  omxvdec_compat_get_data failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_scd);
			return -EIO;
		}

		ret = VCTRL_SCDHal_Process(&scd_reg_cfg, &scd_state_reg);
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "omxvdec_SMHal_Process failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_scd);
			return -EIO;
		}

		ret = copy_to_user(vdec_msg.out, &scd_state_reg, sizeof(scd_state_reg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_SCD_PROC : copy_to_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_scd);
			return -EIO;
		}
		VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_scd);
		break;

	case VDEC_IOCTL_SET_BPD_PROC:
		VDEC_DOWN_INTERRUPTIBLE(&omxvdec->vdec_mutex_bpd);
		ret = copy_from_user(&bpd_reg, vdec_msg.in, sizeof(bpd_reg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_SET_BPD_PROC : copy_from_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_bpd);
			return -EIO;
		}

		ret = VCTRL_BPDHal_Process(&bpd_reg);
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VCTRL_BPDHal_Process failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_bpd);
			return -EIO;
		}

		ret = copy_to_user(vdec_msg.out, &bpd_reg, sizeof(bpd_reg));
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "VDEC_IOCTL_SET_BPD_PROC :  copy_to_user failed\n");
			VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_bpd);
			return -EIO;
		}
		VDEC_UP_INTERRUPTIBLE(&omxvdec->vdec_mutex_bpd);
		break;
	default:
		/* could not handle ioctl */
		OmxPrint(OMX_FATAL, "%s %d: ERROR cmd : %d is not supported\n", __func__, __LINE__, _IOC_NR(code));
		return -ENOTTY;
	}
	return 0;
}
#endif

static const struct file_operations omxvdec_fops = {
	.owner          = THIS_MODULE,
	.open           = omxvdec_open,
	.unlocked_ioctl = omxvdec_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = omxvdec_compat_ioctl,
#endif
	.release        = omxvdec_release,
};

static HI_BOOL omxvdec_device_idle(KIRIN_PLATFORM_E plt_frm)
{
	HI_S32 idle   = 0;
	HI_U32 *pctrl = HI_NULL;

	switch(plt_frm) {
	case KIRIN_960 :
	case KIRIN_970_ES :
		pctrl  = (HI_U32 *)ioremap(PCTRL_PERI, 4);
		if (pctrl)
			idle = readl(pctrl) & 0x4;
		break;
	case KIRIN_970_CS :
		pctrl  = (HI_U32 *)ioremap(PCTRL_PERI_SATA0, 4);
		if (pctrl)
			idle = readl(pctrl) & 0x40000; //b[18]
		break;
	case KIRIN_660:
		pctrl  = (HI_U32 *)ioremap(PCTRL_PERI_SATA0, 4);
		if (pctrl)
			idle = readl(pctrl) & 0x20000; //b[17]
		break;
	default :
		printk(KERN_ERR "unkown platform %d!\n", plt_frm);
		break;
	};

	if (!pctrl) {
		printk(KERN_ERR "ioremap failed!\n");
		return HI_FALSE;
	}

	iounmap(pctrl);
	pctrl = HI_NULL;
	return ((idle != 0) ? HI_TRUE : HI_FALSE);
}

static HI_S32 omxvdec_probe(struct platform_device *pltdev)
{
	HI_S32 ret;

#ifdef PLATFORM_KIRIN970
	HI_U32 fpga_cs = 0;
	HI_U32 fpga_es = 0;
	struct device *dev = &pltdev->dev;

	of_property_read_u32(dev->of_node, "fpga_flag_es", &fpga_es);
	of_property_read_u32(dev->of_node, "fpga_flag_cs", &fpga_cs);
	if (fpga_es == 1) {
		printk(KERN_INFO "boston es fpga\n");
		if (omxvdec_device_idle(KIRIN_970_ES) == HI_FALSE) {
			printk(KERN_ERR "vdec is not exist\n");
			return HI_FAILURE;
		}
	} else if (fpga_cs == 1) {
		printk(KERN_INFO "boston cs fpga\n");
		if (omxvdec_device_idle(KIRIN_970_CS) == HI_FALSE) {
			printk(KERN_ERR "vdec is not exist\n");
			return HI_FAILURE;
		}
	} else {
		printk(KERN_INFO "not boston cs/es fpga\n");
	}
#endif

#ifdef PLATFORM_KIRIN660
		HI_U32 fpga = 0;
		struct device *dev = &pltdev->dev;

		of_property_read_u32(dev->of_node, "fpga_flag", &fpga);
		if(fpga == 1) {
			printk(KERN_INFO "KIRIN_660 fpga\n");
			if (omxvdec_device_idle(KIRIN_660) == HI_FALSE) {
				printk(KERN_ERR "vdec is not exist\n");
				return HI_FAILURE;
			}
		} else {
			printk(KERN_INFO "not KIRIN_660 fpga\n");
		}
#endif

	if (gIsDeviceDetected == HI_TRUE) {
		OmxPrint(OMX_INFO, "Already probe omxvdec\n");
		return 0;
	}

	platform_set_drvdata(pltdev, HI_NULL);

	memset(&g_OmxVdecEntry, 0, sizeof(OMXVDEC_ENTRY));
	VDEC_INIT_MUTEX(&g_OmxVdecEntry.omxvdec_mutex);
	VDEC_INIT_MUTEX(&g_OmxVdecEntry.vdec_mutex_scd);
	VDEC_INIT_MUTEX(&g_OmxVdecEntry.vdec_mutex_bpd);
	VDEC_INIT_MUTEX(&g_OmxVdecEntry.vdec_mutex_vdh);

	ret = omxvdec_setup_cdev(&g_OmxVdecEntry, &omxvdec_fops);
	if (ret < 0) {
		printk(KERN_CRIT "%s call omxvdec_setup_cdev failed\n", __func__);
		goto cleanup0;
	}

	ret = VDEC_Regulator_Probe(&pltdev->dev);
	if (ret != HI_SUCCESS) {
		printk(KERN_CRIT "%s call Regulator_Initialize failed\n", __func__);
		goto cleanup1;
	}

	g_OmxVdecEntry.device = &pltdev->dev;
	platform_set_drvdata(pltdev, &g_OmxVdecEntry);
	gIsDeviceDetected = HI_TRUE;

	return 0;

cleanup1:
	omxvdec_cleanup_cdev(&g_OmxVdecEntry);

cleanup0:
	return ret;
}

static HI_S32 omxvdec_remove(struct platform_device *pltdev)
{
	OMXVDEC_ENTRY *omxvdec = HI_NULL;

	omxvdec = platform_get_drvdata(pltdev);
	if (omxvdec != HI_NULL) {
		if (IS_ERR(omxvdec)) {
			OmxPrint(OMX_ERR, "call platform_get_drvdata err, errno : %ld\n", PTR_ERR(omxvdec));
		} else {
			omxvdec_cleanup_cdev(omxvdec);
			VDEC_Regulator_Remove(&pltdev->dev);
			platform_set_drvdata(pltdev, HI_NULL);
			gIsDeviceDetected = HI_FALSE;
		}
	}

	return 0;
}

static HI_S32 omxvdec_suspend(struct platform_device *pltdev, pm_message_t state)
{
	SINT32 ret;
	OmxPrint(OMX_ALWS, "%s enter\n", __func__);

	if (gIsNormalInit != 0)
		VCTRL_Suspend();

	ret = VDEC_Regulator_Disable();
	if (ret != HI_SUCCESS)
		OmxPrint(OMX_FATAL, "%s disable regulator failed\n", __func__);

	OmxPrint(OMX_ALWS, "%s success\n", __func__);

	return HI_SUCCESS;
}

static HI_S32 omxvdec_resume(struct platform_device *pltdev)
{
	SINT32 ret;
	OmxPrint(OMX_ALWS, "%s enter\n", __func__);

	if (gIsNormalInit != 0) {
		ret = VDEC_Regulator_Enable();
		if (ret != HI_SUCCESS) {
			OmxPrint(OMX_FATAL, "%s enable regulator failed\n", __func__);
			return HI_FAILURE;
		}
		VCTRL_Resume();
	}

	OmxPrint(OMX_ALWS, "%s success\n", __func__);

	return HI_SUCCESS;
}

static HI_VOID omxvdec_device_release(struct device *dev)
{
	return;
}

static struct platform_driver omxvdec_driver = {

	.probe   = omxvdec_probe,
	.remove  = omxvdec_remove,
	.suspend = omxvdec_suspend,
	.resume  = omxvdec_resume,
	.driver  = {
		.name  = (HI_PCHAR) g_OmxVdecDrvName,
		.owner = THIS_MODULE,
		.of_match_table = Hisi_Vdec_Match_Table
	},
};

static struct platform_device omxvdec_device = {

	.name = g_OmxVdecDrvName,
	.id   = -1,
	.dev  = {
		.platform_data = NULL,
		.release       = omxvdec_device_release,
	},
};

HI_S32 __init OMXVDEC_DRV_ModInit(HI_VOID)
{
	HI_S32 ret;

	ret = platform_device_register(&omxvdec_device);
	if (ret < 0) {
		OmxPrint(OMX_FATAL, "%s call platform_device_register failed\n", __func__);
		return ret;
	}

	ret = platform_driver_register(&omxvdec_driver);
	if (ret < 0) {
		OmxPrint(OMX_FATAL, "%s call platform_driver_register failed\n", __func__);
		goto exit;
	}
#ifdef MODULE
	OmxPrint(OMX_ALWS, "Load hi_omxvdec.ko :%d success\n", OMXVDEC_VERSION);
#endif

	return HI_SUCCESS;
exit:
	platform_device_unregister(&omxvdec_device);

	return ret;
}

HI_VOID __exit OMXVDEC_DRV_ModExit(HI_VOID)
{
	platform_driver_unregister(&omxvdec_driver);
	platform_device_unregister(&omxvdec_device);

#ifdef MODULE
	OmxPrint(OMX_ALWS, "Unload hi_omxvdec.ko : %d success\n", OMXVDEC_VERSION);
#endif

}

module_init(OMXVDEC_DRV_ModInit);
module_exit(OMXVDEC_DRV_ModExit);

MODULE_AUTHOR("gaoyajun@hisilicon.com");
MODULE_DESCRIPTION("vdec driver");
MODULE_LICENSE("GPL");
