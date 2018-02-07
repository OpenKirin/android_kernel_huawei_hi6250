#include <linux/kthread.h>
#include <linux/delay.h>

#include "drv_venc_osal.h"
#include "hi_drv_mem.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*lint -e747 -e712 -e732 -e715 -e774 -e845 -e438 -e838*/
HI_U32  g_VencPrintEnable = 0xff;

char   *pszMsg[((HI_U8)VENC_ALW) + 1] = {"FATAL","ERR","WARN","IFO","DBG"}; /*lint !e785 */
HI_CHAR g_VencPrintMsg[1024];

static HI_VOID (*ptrVencCallBack)(HI_VOID);

static irqreturn_t VENC_DRV_OsalVencISR(HI_S32 Irq, HI_VOID* DevID)
{
	(*ptrVencCallBack)();
	return IRQ_HANDLED;
}

HI_S32 VENC_DRV_OsalIrqInit( HI_U32 Irq, HI_VOID (*ptrCallBack)(HI_VOID))
{
	HI_S32 ret = 0;

	if (Irq != 0) {
		ptrVencCallBack = ptrCallBack;
		ret = request_irq(Irq, VENC_DRV_OsalVencISR, 0, "DT_device", NULL);
	} else {
		HI_ERR_VENC("%s, params is invaild! Irq:%d  invalid_argument!\n", __func__, Irq);
		ret = HI_FAILURE;
	}

	if (ret == 0) {
		return HI_SUCCESS;
	} else {
		HI_ERR_VENC("%s, request_irq failed!\n", __func__);
		return HI_FAILURE;
	}
}

HI_VOID VENC_DRV_OsalIrqFree( HI_U32 Irq )
{
	free_irq(Irq, NULL);
}

HI_S32 VENC_DRV_OsalLockCreate( HI_VOID**phLock )
{
	spinlock_t *pLock = NULL;

	pLock = (spinlock_t *)vmalloc(sizeof(spinlock_t));

	if (!pLock) {
		HI_ERR_VENC("vmalloc failed!\n");
		return HI_FAILURE;
	}

	spin_lock_init( pLock );
	*phLock = pLock;

	return HI_SUCCESS;
}

HI_VOID VENC_DRV_OsalLockDestroy( HI_VOID* hLock )
{
	if (hLock) {
		vfree((HI_VOID *)hLock);
		//hLock = NULL;
	}
}

/************************************************************************/
/* ��ʼ���¼�                                                           */
/************************************************************************/
HI_S32 VENC_DRV_OsalInitEvent( VEDU_OSAL_EVENT *pEvent, HI_S32 InitVal )
{
	pEvent->flag = InitVal;
	init_waitqueue_head(&(pEvent->queue_head));
	return HI_SUCCESS;
}

/************************************************************************/
/* �����¼�����                                                             */
/************************************************************************/
HI_S32 VENC_DRV_OsalGiveEvent( VEDU_OSAL_EVENT *pEvent )
{
	pEvent->flag = 1;
	wake_up(&(pEvent->queue_head));
	return HI_SUCCESS;
}

/************************************************************************/
/* �ȴ��¼�                                                             */
/* �¼���������OSAL_OK����ʱ����OSAL_ERR ��condition������������ȴ�    */
/* �����ѷ��� 0 ����ʱ���ط�-1                                          */
/************************************************************************/
HI_S32 VENC_DRV_OsalWaitEvent( VEDU_OSAL_EVENT *pEvent, HI_U32 msWaitTime )
{
	HI_S32 l_ret = 0;

	if (msWaitTime != 0xffffffff) {
		l_ret = wait_event_interruptible_timeout((pEvent->queue_head), (pEvent->flag != 0), (msecs_to_jiffies(msWaitTime))); /*lint !e665 !e666 !e40 !e713 !e578*/
		pEvent->flag = 0;//(pEvent->flag>0)? (pEvent->flag-1): 0;
		return (l_ret != 0) ? HI_SUCCESS : HI_FAILURE;
	} else {
		l_ret = wait_event_interruptible((pEvent->queue_head), (pEvent->flag != 0));/*lint !e665 !e666 !e40 !e31 !e578*/
		pEvent->flag = 0;
		return (l_ret == 0) ? HI_SUCCESS : HI_FAILURE;
	}
}

HI_S32 HiMemCpy(HI_VOID * a_pHiDstMem, HI_VOID * a_pHiSrcMem, size_t a_Size)
{
	if ((!a_pHiDstMem) || (!a_pHiSrcMem)) {
		HI_ERR_VENC("%s, The input paratemter's pointer  is NULL \n", __func__);
		return HI_FAILURE;
	}

	memcpy((HI_VOID *)a_pHiDstMem, (HI_VOID *)a_pHiSrcMem, a_Size);
	return HI_SUCCESS;
}

HI_S32 HiMemSet(HI_VOID * a_pHiDstMem, HI_S32 a_Value, size_t a_Size)
{
	if (!a_pHiDstMem) {
		HI_ERR_VENC("%s, The input paratemter's pointer  is NULL \n", __func__);
		return HI_FAILURE;
	}

	if (a_pHiDstMem) {
		memset((HI_VOID *)a_pHiDstMem, a_Value, a_Size);
		return HI_SUCCESS;
	}

	return HI_FAILURE;
}

HI_VOID HiSleepMs(HI_U32 a_MilliSec)
{
	msleep(a_MilliSec);
}

HI_U32*  HiMmap(HI_U32 Addr ,HI_U32 Range)
{
	HI_U32 *res_addr = NULL;
	res_addr = (HI_U32 *)ioremap(Addr, Range);
	return res_addr;
}

HI_VOID HiMunmap(HI_U32 * pMemAddr)
{
	if (!pMemAddr) {
		HI_ERR_VENC("%s, The input paratemter's pointer  is NULL \n", __func__);
		return;
	}

	iounmap(pMemAddr);
	//pMemAddr = HI_NULL;
}

HI_S32 HiStrNCmp(const HI_PCHAR pStrName,const HI_PCHAR pDstName,HI_S32 nSize)
{
	HI_S32 ret = 0;
	if (pStrName && pDstName) {
		ret = strncmp(pStrName,pDstName,nSize);
		return ret;
	}

	return HI_FAILURE;
}

HI_VOID *HiMemVAlloc(HI_U32 nMemSize)
{
	HI_VOID * memaddr = NULL;
	if (nMemSize) {
		memaddr = vmalloc(nMemSize);
	}
	return memaddr;
 }

HI_VOID HiMemVFree(HI_VOID * pMemAddr)
{
	if (pMemAddr) {
		vfree((HI_VOID *)pMemAddr);
		//pMemAddr = NULL;
	}
}

HI_VOID HiVENC_INIT_MUTEX(HI_VOID* pSem)
{
	if (pSem) {
		sema_init((struct semaphore *)pSem, 1);
	}
}

HI_S32 HiVENC_DOWN_INTERRUPTIBLE(HI_VOID* pSem)
{
	HI_S32 Ret = -1;
	if (pSem) {
		Ret = down_interruptible((struct semaphore *)pSem);
		return  Ret;
	}
	return  Ret;
}

HI_VOID  HiVENC_UP_INTERRUPTIBLE(HI_VOID* pSem)
{
	if (pSem) {
		up((struct semaphore *)pSem);
	}
}

HI_VOID HI_PRINT(HI_U32 type,char *file, int line , char * function, HI_CHAR*  msg, ... )
{
	va_list args;
	HI_U32 uTotalChar;

	if ( ((1 << type) & g_VencPrintEnable) == 0 && (type != VENC_ALW) ) /*lint !e701 */
		return ;

	va_start(args, msg);

	uTotalChar = vsnprintf(g_VencPrintMsg, sizeof(g_VencPrintMsg), msg, args); //������args��msg����ת���γɸ�ʽ���ַ�������������ʾ���ַ�����
	g_VencPrintMsg[sizeof(g_VencPrintMsg) - 1] = '\0';

	va_end(args);

	if (uTotalChar <= 0 || uTotalChar >= 1023) /*lint !e775 */
		return;

	printk(KERN_ALERT "%s:<%d:%s>%s \n",pszMsg[type],line,function,g_VencPrintMsg);
	return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
