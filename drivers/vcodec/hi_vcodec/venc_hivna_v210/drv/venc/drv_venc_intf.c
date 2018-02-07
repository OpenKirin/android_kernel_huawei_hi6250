

#include <linux/device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>

#include "drv_venc_osal.h"
#include "drv_venc.h"
#include "venc_regulator.h"
#include "hal_venc.h"

#define PCTRL_PERI 0xE8A090A4
#define PCTRL_PERI_SATA0 (0xE8A090BC)
//#define SCTRL_CHIPID (0xFFF0AE00)
/*lint -e750 -e838 -e715*/
#ifndef VM_RESERVED /*for kernel up to 3.7.0 version*/
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

// cppcheck-suppress *
#define D_VENC_CHECK_DOWN_INTERRUPTIBLE_RET(Ret)\
	if (Ret != 0) {\
		HI_ERR_VENC("down_interruptible failed!\n");\
		return HI_FAILURE;\
	}\

/*============Deviece===============*/
typedef struct {
	dev_t dev;
	struct device* venc_device;
	struct device* venc_device_2;
	struct cdev cdev;
	struct class* venc_class;
}VENC_ENTRY;

#ifdef PLATFORM_KIRIN970
typedef enum {
	KIRIN_960,
	KIRIN_970_ES,
	KIRIN_970_CS,
}KIRIN_PLATFORM_E;
#endif
#ifdef PLATFORM_KIRIN660
typedef enum {
	KIRIN_660,
}KIRIN_PLATFORM_E;
#endif


struct semaphore g_VencMutex;

HI_U32 IpFree = 0;
HI_BOOL g_vencOpenFlag = HI_FALSE;
HI_BOOL g_vencDevDetected = HI_FALSE;

//VENC device open times
atomic_t g_VencCount = ATOMIC_INIT(0);

HI_S32 VENC_DRV_Resume(struct platform_device *pltdev);
HI_S32 VENC_DRV_Suspend(struct platform_device *pltdev, pm_message_t state);

static HI_S32  VENC_DRV_SetupCdev(VENC_ENTRY *venc, const struct file_operations *fops);
static HI_S32  VENC_DRV_CleanupCdev(VENC_ENTRY *venc);
static HI_S32  VENC_DRV_Probe(struct platform_device * pltdev);
static HI_S32  VENC_DRV_Remove(struct platform_device *pltdev);
static HI_VOID VENC_DRV_DeviceRelease1(struct device* dev);

extern VeduEfl_IpCtx_S   VeduIpCtx;
extern U_FUNC_VCPI_RAWINT  g_hw_done_type;
extern VEDU_OSAL_EVENT   g_hw_done_event;
extern HI_U32 gVencIsFPGA;

static HI_S32 venc_drv_waithwdone(U_FUNC_VCPI_RAWINT *hw_done_type)
{
	HI_S32 Ret = HI_FAILURE;

	Ret = VENC_DRV_OsalWaitEvent(&g_hw_done_event, ((1 == gVencIsFPGA) ? msecs_to_jiffies(1000000) : msecs_to_jiffies(1000)));/*lint !e712 !e747 */

	if (Ret != 0) {
		hw_done_type->u32 = 0;
		IpFree = 0;
		HI_ERR_VENC("%s,wait timeout ! Ret = %d\n", __func__, Ret);
		return Ret;
	}

	*hw_done_type = g_hw_done_type;
	return Ret;
}

static HI_S32  venc_drv_getvenccount(atomic_t  *venc_count)
{
	HiMemCpy((HI_VOID *)venc_count, (HI_VOID *)(&g_VencCount), sizeof(atomic_t));
	return HI_SUCCESS;
}

static HI_S32 venc_drv_register_info(VENC_REG_INFO_S *regcfginfo)
{
	HI_S32 Ret = HI_SUCCESS;
	CMD_TYPE cmd  = regcfginfo->cmd;
	switch (cmd) {
	case VENC_SET_CFGREG:
		if (regcfginfo->bResetReg == HI_TRUE)
		{
			Ret = VENC_HAL_ResetReg();
			if (Ret != HI_SUCCESS)
			{
				HI_ERR_VENC("%s, CMD_VENC_RESETREG Ret:%d\n", __func__, Ret);
				break;
			}
		}

		if (regcfginfo->bClkCfg == HI_TRUE)
		{
			Ret = Venc_SetClkRate(regcfginfo->clk_type);
			if (Ret != HI_SUCCESS) {
				HI_ERR_VENC("%s, CMD_VENC_SETRATE Ret:%d\n", __func__, Ret);
				break;
			}
		}

		VeduHal_CfgReg(regcfginfo);

		VENC_HAL_StartEncode((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */

		Ret = venc_drv_waithwdone(&regcfginfo->hw_done_type) ;

		if((Ret == HI_SUCCESS ) && (!regcfginfo->hw_done_type.bits.vcpi_rint_vedu_timeout))
		{
			VENC_HAL_Get_Reg_Venc(regcfginfo);
			HI_DBG_VENC("%s,  VENC_HAL_Get_Reg_Venc\n", __func__);
		}

		break;
	case VENC_SET_CFGREGSIMPLE:
		VeduHal_CfgRegSimple(regcfginfo);

		VENC_HAL_StartEncode((S_HEVC_AVC_REGS_TYPE*)(VeduIpCtx.pRegBase));/*lint !e826 */

		Ret = venc_drv_waithwdone(&regcfginfo->hw_done_type) ;

		if((Ret == HI_SUCCESS ) && (!regcfginfo->hw_done_type.bits.vcpi_rint_vedu_timeout))
		{
			VENC_HAL_Get_Reg_Venc(regcfginfo);
			HI_DBG_VENC("%s,  VENC_HAL_Get_Reg_Venc\n", __func__);
		}
		break;
	default:
		HI_ERR_VENC(" cmd type unknown:%d\n", cmd);
		Ret = HI_FAILURE;
		break;
	}
	return Ret;
}
static HI_S32 VENC_DRV_Open(struct inode *finode, struct file  *ffile)
{
	HI_S32 Ret = 0;

	HI_DBG_VENC("enter %s()\n", __func__);

	Ret =HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	if (Ret == 0) {
		HI_DBG_VENC("down_interruptible success!\n");
	}
	else {
		HI_FATAL_VENC("down_interruptible failed!\n");
		return HI_FAILURE;
	}

	if (atomic_inc_return(&g_VencCount) == 1) {
		Ret = VENC_DRV_BoardInit();
		if (Ret != HI_SUCCESS) {
			HI_FATAL_VENC("%s, VENC_DRV_BoardInit failed, ret=%d\n", __func__, Ret);
			atomic_dec(&g_VencCount);
			HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
			return HI_FAILURE;
		}
		Ret = VENC_DRV_EflOpenVedu();
		if (Ret != HI_SUCCESS) {
			HI_FATAL_VENC("%s, VeduEfl_OpenVedu failed, ret=%d\n", __func__, Ret);
			atomic_dec(&g_VencCount);
			VENC_DRV_BoardDeinit();
			HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
			return HI_FAILURE;
		}
		IpFree = 1;
	}

	g_vencOpenFlag = HI_TRUE;
	HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);

	HI_INFO_VENC("open venc device successfull!!\n");
	return HI_SUCCESS;
}

static HI_S32 VENC_DRV_Close(struct inode *finode, struct file  *ffile)
{
	HI_S32 Ret = 0;

	HI_DBG_VENC("enter %s()\n", __func__);

	HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	if (atomic_dec_and_test(&g_VencCount)) {
		Ret = VENC_DRV_EflCloseVedu();
		if (Ret != HI_SUCCESS) {
			HI_FATAL_VENC("%s, VeduEfl_CloseVedu failed, ret=%d\n", __func__, Ret);
			VENC_DRV_BoardDeinit();
			HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
			return HI_FAILURE;
		}

		VENC_DRV_BoardDeinit();
		IpFree = 0;
		g_vencOpenFlag = HI_FALSE;
	}

	HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);

	HI_INFO_VENC("close venc device success!!\n");
	return HI_SUCCESS;
}

HI_S32 VENC_DRV_Suspend(struct platform_device *pltdev, pm_message_t state)
{
	HI_S32 Ret = 0;
	HI_INFO_VENC("enter!\n");

	HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	IpFree = 0;
	if (!g_vencOpenFlag) {
		HI_INFO_VENC("venc device already suspend!\n");
		HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		return 0;
	}

	Ret = VENC_DRV_EflSuspendVedu();
	if (Ret != HI_SUCCESS) {
		HI_FATAL_VENC("%s, VeduEfl_CloseVedu failed, ret=%d\n", __func__, Ret);
		HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		return HI_FAILURE;
	}

	VENC_DRV_BoardDeinit();
	g_hw_done_event.flag = 0;
	HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);

	HI_INFO_VENC("exit! \n");
	return HI_SUCCESS;
}/*lint !e715*/

HI_S32 VENC_DRV_Resume(struct platform_device *pltdev)
{
	HI_S32 Ret = 0;

	HI_INFO_VENC("enter! \n");

	HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	if (!g_vencOpenFlag) {
		HI_INFO_VENC("venc device already resume!\n");
		HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		return 0;
	}
	Ret = VENC_DRV_BoardInit();
	if (Ret != HI_SUCCESS) {
		HI_FATAL_VENC("%s, VENC_DRV_BoardInit failed, ret=%d\n", __func__, Ret);
		atomic_dec(&g_VencCount);
		HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		return HI_FAILURE;
	}
	Ret = VENC_DRV_EflResumeVedu();
	if (Ret != HI_SUCCESS) {
		HI_FATAL_VENC("%s, VeduEfl_OpenVedu failed, ret=%d\n", __func__, Ret);
		atomic_dec(&g_VencCount);
		HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		return HI_FAILURE;
	}

	IpFree = 1;
	HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
	HI_INFO_VENC("exit! \n");
	return HI_SUCCESS;
}/*lint !e715*/

static long VENC_Ioctl(struct file *file, unsigned int ucmd, unsigned long uarg)
{
	HI_S32  Ret;
	HI_S32  cmd  = (HI_S32)ucmd;
	HI_VOID *arg = (HI_VOID *)uarg;
	atomic_t *venc_count = NULL;
	VENC_REG_INFO_S *regcfginfo =  NULL;

	/*Ret = HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	D_VENC_CHECK_DOWN_INTERRUPTIBLE_RET(Ret);*/

	if (IpFree == 0) {
		//HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		HI_ERR_VENC("%s, IpFree is 0!\n", __func__);
		return HI_FAILURE;
	}

	if (!arg) {
		//HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
		HI_ERR_VENC("%s, uarg is NULL!\n", __func__);
		return HI_FAILURE;
	}

	switch (cmd) {
	case CMD_VENC_GET_VENCCOUNT:/*lint !e30 */
		venc_count = (atomic_t *)arg;
		Ret = venc_drv_getvenccount(venc_count);
		HI_DBG_VENC("%s, CMD_VENC_GET_VENCCOUNT Ret:%d\n", __func__, Ret);
		break;
	case CMD_VENC_START_ENCODE:/*lint !e30 !e142*/
		VeduIpCtx.TaskRunning = 1;
		regcfginfo = (VENC_REG_INFO_S *)arg;
		Ret = venc_drv_register_info(regcfginfo);
		VeduIpCtx.TaskRunning = 0;
		HI_DBG_VENC("%s, CMD_VENC_CFGREGINFO Ret:%d\n", __func__, Ret);
		break;
	default:
		HI_ERR_VENC("venc cmd unknown:%x\n", ucmd);
		Ret = HI_FAILURE;
		break;
	}
	//HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
	return Ret;
}
static long VENC_DRV_Ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long Ret;

	Ret = HiVENC_DOWN_INTERRUPTIBLE(&g_VencMutex);
	if (Ret != 0)
	{
		HI_FATAL_VENC("down_interruptible failed!\n");
		return Ret;
	}
	Ret = (long)HI_DRV_UserCopy(file, cmd, arg, VENC_Ioctl);
	HiVENC_UP_INTERRUPTIBLE(&g_VencMutex);
	return Ret;
}

static struct file_operations VENC_FOPS =
{
	.owner          = THIS_MODULE,/*lint !e64 */
	.open           = VENC_DRV_Open,
	.unlocked_ioctl = VENC_DRV_Ioctl,
	.compat_ioctl   = VENC_DRV_Ioctl,  //用户态 ，内核态平台位宽不一致的时候，会调用
	.release        = VENC_DRV_Close,
	//.mmap         = VENC_DRV_MMap,   // reserve
};/*lint !e785 */

#ifdef PLATFORM_KIRIN970
static const struct of_device_id venc_of_match[] = {
	{ .compatible = "hisi,kirin970-venc", },/*lint !e785 */
	{ }/*lint !e785 */
};
#endif

#ifdef PLATFORM_KIRIN660
static const struct of_device_id venc_of_match[] = {
	{ .compatible = "hisi,kirin660-venc", },/*lint !e785 */
	{ }/*lint !e785 */
};
#endif

static struct platform_driver Venc_driver = {
	.probe   = VENC_DRV_Probe,
	.remove  = VENC_DRV_Remove,
	.suspend = VENC_DRV_Suspend,
	.resume  = VENC_DRV_Resume,
	.driver  = {
	.name    = "hi_venc",
	.owner   = THIS_MODULE,/*lint !e64 */
	.of_match_table = venc_of_match
	},/*lint !e785 */
};/*lint !e785 */

static struct platform_device Venc_device = {
	.name = "hi_venc",
	.id   = -1,
	.dev  = {
	.platform_data = NULL,
	.release   = VENC_DRV_DeviceRelease1,
	},/*lint !e785 */
};/*lint !e785 */

static HI_S32 VENC_DRV_SetupCdev(VENC_ENTRY *venc, const struct file_operations *fops)
{
	HI_S32 err = 0;

	HI_INFO_VENC("enter %s()\n", __func__);
	err = alloc_chrdev_region(&venc->dev, 0, 1, "hi_venc");

	HiMemSet((HI_VOID*)&(venc->cdev), 0, sizeof(struct cdev));
	cdev_init(&(venc->cdev), &VENC_FOPS);

	venc->cdev.owner = THIS_MODULE;/*lint !e64 */
	venc->cdev.ops = &VENC_FOPS;
	err = cdev_add(&(venc->cdev), venc->dev, 1);

	/*在/sys/class/目录下创建设备类别目录hi_venc*/
	venc->venc_class = class_create(THIS_MODULE, "hi_venc");/*lint !e64 */
	if (IS_ERR(venc->venc_class)) {
		err = PTR_ERR(venc->venc_class);/*lint !e712 */
		HI_ERR_VENC("Fail to create hi_venc class\n");
		return HI_FAILURE;/*lint !e438 */
	}

	/*在/dev/目录和/sys/class/hi_venc目录下分别创建设备文件hi_venc*/
	venc->venc_device = device_create(venc->venc_class, NULL, venc->dev, "%s", "hi_venc");
	if (IS_ERR(venc->venc_device)) {
		err = PTR_ERR(venc->venc_device);/*lint !e712 */
		HI_ERR_VENC("%s, Fail to create hi_venc device\n", __func__);
		return HI_FAILURE;/*lint !e438 */
	}

	HI_INFO_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;/*lint !e438 */
}/*lint !e550 */

static HI_S32 VENC_DRV_CleanupCdev(VENC_ENTRY *venc)
{
	/*销毁设备类别和设备*/
	if (venc->venc_class) {
		device_destroy(venc->venc_class,venc->dev);
		class_destroy(venc->venc_class);
	}

	cdev_del(&(venc->cdev));
	unregister_chrdev_region(venc->dev,1);

	return 0;
}

static HI_U32 VENC_DRV_Device_Idle(KIRIN_PLATFORM_E plt_frm)
{
	HI_S32 idle   = 0;
	HI_U32 *pctrl = HI_NULL;
	switch(plt_frm)
	{
#ifdef PLATFORM_KIRIN970
		case KIRIN_960 :
		case KIRIN_970_ES :
		{
			pctrl  = (HI_U32 *)ioremap(PCTRL_PERI, (unsigned long)4);/*lint !e747 */
			if(pctrl)
			{
				idle = readl(pctrl) & 0x4;/*lint !e50 !e64 */
			}
			break;
		}
		case KIRIN_970_CS :
		{
			pctrl  = (HI_U32 *)ioremap(PCTRL_PERI_SATA0, (unsigned long)4);/*lint !e747 */
			if(pctrl)
			{
				idle = readl(pctrl) & 0x40000;/*lint !e50 !e64 */ //b[18]
			}
			break;
		}
#endif
#ifdef PLATFORM_KIRIN660
		case KIRIN_660 :
		{
			pctrl  = (HI_U32 *)ioremap(PCTRL_PERI_SATA0, (unsigned long)4);/*lint !e747 */
			if(pctrl)
				idle = readl(pctrl) & 0x20000;/*lint !e50 !e64 */ //b[17]
			break;
		}
#endif
		default :
		{
			printk(KERN_ERR "unkown platform %d!\n", plt_frm);
			break;
		}
	};

	if (!pctrl)
	{
		printk(KERN_ERR "ioremap failed!\n");
		return HI_FALSE;
	}
	else
	{
		iounmap(pctrl);
	}

	return (HI_U32)((idle != 0) ? HI_TRUE : HI_FALSE);
}

static HI_S32 VENC_DRV_Probe(struct platform_device * pltdev)
{
	HI_S32 ret = HI_FAILURE;
	VENC_ENTRY *venc = HI_NULL;
#ifdef PLATFORM_KIRIN970
	HI_U32 fpga970_cs = 0;
	HI_U32 fpga970_es = 0;
#endif
#ifdef PLATFORM_KIRIN660
	HI_U32 fpga660 = 0;
#endif
	struct device *dev = &pltdev->dev;

	HI_INFO_VENC("%s, omxvenc prepare to probe\n", __func__);
	HiVENC_INIT_MUTEX(&g_VencMutex);

#ifdef PLATFORM_KIRIN970
	of_property_read_u32(dev->of_node, "fpga_flag_es", &fpga970_es);
	of_property_read_u32(dev->of_node, "fpga_flag_cs", &fpga970_cs);
	if(fpga970_es == 1) {
		printk(KERN_INFO "boston es fpga \n");
		if (VENC_DRV_Device_Idle(KIRIN_970_ES) == HI_FALSE) {
			printk(KERN_ERR "KIRIN_970_ES, venc is not exist \n");
			return HI_FAILURE;
		}
	}
	else if(fpga970_cs == 1) {
		printk(KERN_INFO "boston cs fpga\n");
		if (VENC_DRV_Device_Idle(KIRIN_970_CS) == HI_FALSE) {
			printk(KERN_ERR "KIRIN_970_CS, venc is not exist\n");
			return HI_FAILURE;
		}
	}
	else {
		printk(KERN_INFO "not boston cs/es fpga\n");
	}
#endif

#ifdef PLATFORM_KIRIN660
	of_property_read_u32(dev->of_node, "fpga_flag", &fpga660);
	if(fpga660 == 1)
	{
		printk(KERN_INFO "KIRIN_660 fpga\n");
		if (VENC_DRV_Device_Idle(KIRIN_660) == HI_FALSE) {
			printk(KERN_ERR "KIRIN_660, venc is not exist\n");
			return HI_FAILURE;
		}
	}
	else {
		printk(KERN_INFO "not KIRIN_660 fpga \n");
	}
#endif

	if (g_vencDevDetected) {
		HI_INFO_VENC("%s,  venc device detected already!\n", __func__);
		return HI_SUCCESS;
	}

	venc = HiMemVAlloc(sizeof(VENC_ENTRY));/*lint !e747 */
	if (!venc) {
		HI_ERR_VENC("%s call vmalloc failed!\n", __func__);
		return ret;
	}

	HiMemSet((HI_VOID *)venc, 0, sizeof(VENC_ENTRY));
	ret = VENC_DRV_SetupCdev(venc, &VENC_FOPS);
	if (ret < 0) {
		HI_ERR_VENC("%s call hivdec_setup_cdev failed!\n", __func__);
		goto free;
	}

	venc->venc_device_2 = &pltdev->dev;
	platform_set_drvdata(pltdev, venc);
	g_vencDevDetected = HI_TRUE;

	ret = Venc_Regulator_Init(pltdev);
	if (ret < 0) {
		HI_ERR_VENC("%s, Venc_Regulator_Init failed!\n", __func__);
		goto cleanup;
	}
	VENC_DRV_BoardInit();

	HI_INFO_VENC("%s, omxvenc probe ok.\n", __func__);
	return ret;

cleanup:
	VENC_DRV_CleanupCdev(venc);
free:
	HiMemVFree(venc);
	return ret;
}

static HI_S32 VENC_DRV_Remove(struct platform_device *pltdev)
{
	VENC_ENTRY *venc = NULL;
	HI_INFO_VENC("%s, omxvenc prepare to remove.\n", __func__);

	venc = platform_get_drvdata(pltdev);
	if (venc) {
		VENC_DRV_CleanupCdev(venc);
		Venc_Regulator_Deinit(pltdev);
	}
	else {
		HI_ERR_VENC("%s, call platform_get_drvdata err!\n", __func__);
	}

	platform_set_drvdata(pltdev,NULL);
	HiMemVFree(venc);
	g_vencDevDetected = HI_FALSE;

	HI_INFO_VENC("%s, remove omxvenc ok.\n", __func__);
	return 0;
}

static HI_VOID VENC_DRV_DeviceRelease1(struct device* dev)
{
	return;
}

HI_S32 __init VENC_DRV_ModInit(HI_VOID)
{
	HI_S32 ret = 0;
	HI_INFO_VENC("enter %s()\n", __func__);

	ret = platform_device_register(&Venc_device);
	if (ret < 0) {
		HI_ERR_VENC("%s call platform_device_register failed!\n", __func__);
		return ret;
	}

	ret = platform_driver_register(&Venc_driver);/*lint !e64 */
	if (ret < 0) {
		HI_ERR_VENC("%s call platform_driver_register failed!\n", __func__);
		goto exit;
	}
#ifdef USER_DISABLE_VENC_PROC
	VENC_DRV_MemProcAdd();
#endif
	HI_INFO_VENC("%s, success!!!\n", __func__);
#ifdef MODULE
	HI_INFO_VENC("Load hi_venc.ko success.\t(%s)\n", VERSION_STRING);
#endif
	HI_INFO_VENC("exit %s()\n", __func__);
	return HI_SUCCESS;
exit:
	platform_device_unregister(&Venc_device);
#ifdef MODULE
	HI_ERR_VENC("Load hi_venc.ko failed!!.\t(%s)\n", VERSION_STRING);
#endif
	return ret;
}

HI_VOID VENC_DRV_ModExit(HI_VOID)
{
	HI_INFO_VENC("enter %s()\n", __func__);
#ifdef USER_DISABLE_VENC_PROC
	VENC_DRV_MemProcDel();
#endif
	platform_driver_unregister(&Venc_driver);
	platform_device_unregister(&Venc_device);

	HI_INFO_VENC("exit %s()\n", __func__);
	return;
}
/*lint -e528*/
module_init(VENC_DRV_ModInit); /*lint !e528*/
module_exit(VENC_DRV_ModExit); /*lint !e528*/
/*lint -e753*/
MODULE_LICENSE("Dual BSD/GPL"); /*lint !e753*/

