
#include "drv_venc_osal.h"
#include "hi_drv_mem.h"
#include <asm/uaccess.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define MAX_BUFFER_SIZE (10*1024)
HI_CHAR  *g_sbuf = NULL;
HI_S32  g_venc_node_num   = 0;

struct semaphore  g_VencMemSem;
struct venc_mem_buf g_venc_mem_node[MAX_KMALLOC_MEM_NODE];

HI_S32 DRV_MEM_INIT(HI_VOID)
{
        HiVENC_INIT_MUTEX(&g_VencMemSem);

	g_sbuf = HiMemVAlloc(MAX_BUFFER_SIZE);
	if (g_sbuf == NULL) {
		HI_ERR_VENC("%s call vmalloc failed!\n", __func__);
		return HI_FAILURE;
	}
	HiMemSet((HI_VOID *)g_sbuf, 0, MAX_BUFFER_SIZE);/*lint !e747 */
        /* Init kmalloc mem for register's VEDU_COMN1_REGS.COMN1_SMMU_ERR_RDADDRR modify by hyl 20161205*/
        memset((HI_VOID *)&g_venc_mem_node, 0, MAX_KMALLOC_MEM_NODE*sizeof(g_venc_mem_node[0]));
        g_venc_node_num = 0;

	return HI_SUCCESS;
}

HI_S32 DRV_MEM_EXIT(HI_VOID)
{
	HI_S32 i;

	/* Exit kfree mem for register's VEDU_COMN1_REGS.COMN1_SMMU_ERR_RDADDRR*/
	for (i = 0; i < MAX_KMALLOC_MEM_NODE; i++) {
		if (g_venc_mem_node[i].virt_addr != HI_NULL) {
			kfree(g_venc_mem_node[i].virt_addr);
			memset(&g_venc_mem_node[i], 0, sizeof(g_venc_mem_node[i]));/*lint !e866 */
		}
	}

	if (g_sbuf)
		HiMemVFree(g_sbuf);

	g_venc_node_num = 0;
	return HI_SUCCESS;
}

/* kalloc */
HI_S32 DRV_MEM_KAlloc(const HI_CHAR* bufName, const HI_CHAR *zone_name, MEM_BUFFER_S *psMBuf)
{
        HI_U32  i;
	 HI_VOID *virt_addr = HI_NULL;

        if (HiVENC_DOWN_INTERRUPTIBLE(&g_VencMemSem)) {
                HI_ERR_VENC("down_interruptible failed!\n");
                return HI_FAILURE;
        }

        if (psMBuf == HI_NULL || psMBuf->u32Size == 0) {
                HI_ERR_VENC("%s, failed to check arguments!!\n", __func__);
                goto err_exit;
        }

        for (i=0; i<MAX_KMALLOC_MEM_NODE; i++) {
                if ((0 == g_venc_mem_node[i].phys_addr) && (g_venc_mem_node[i].virt_addr == HI_NULL)) {
                        break;
            }
        }

        if (i >= MAX_KMALLOC_MEM_NODE) {
                HI_ERR_VENC("%s, FATAL: No free ion mem node!\n", __func__);
                goto err_exit;
        }

        virt_addr = kmalloc(psMBuf->u32Size, GFP_KERNEL | GFP_DMA);/*lint !e747*/
        if (IS_ERR_OR_NULL(virt_addr)) {
                HI_ERR_VENC("%s, call kzalloc failed, size : %d!\n", __func__, psMBuf->u32Size);
                goto err_exit;
        }

        psMBuf->pStartVirAddr   = virt_addr;
        psMBuf->u64StartPhyAddr = __pa(virt_addr);/*lint !e648 !e834 !e712*/

        snprintf(g_venc_mem_node[i].node_name, MAX_MEM_NAME_LEN, bufName);/*lint !e712 !e747 !e592*/
        g_venc_mem_node[i].node_name[MAX_MEM_NAME_LEN-1] = '\0';

        snprintf(g_venc_mem_node[i].zone_name, MAX_MEM_NAME_LEN, zone_name);/*lint !e712 !e747 !e592*/
        g_venc_mem_node[i].zone_name[MAX_MEM_NAME_LEN-1] = '\0';

        g_venc_mem_node[i].virt_addr = psMBuf->pStartVirAddr;
        g_venc_mem_node[i].phys_addr = psMBuf->u64StartPhyAddr;
        g_venc_mem_node[i].size      = psMBuf->u32Size;

        g_venc_node_num++;

        HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
        return HI_SUCCESS;

err_exit:
        HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
        return HI_FAILURE;/*lint !e429*/
}

/* kfree */
HI_S32 DRV_MEM_KFree(const MEM_BUFFER_S *psMBuf)
{
        HI_U32  i;

        if (HiVENC_DOWN_INTERRUPTIBLE(&g_VencMemSem)) {
                HI_ERR_VENC("down_interruptible failed!\n");
                return HI_FAILURE;
        }

        if (HI_NULL == psMBuf || psMBuf->pStartVirAddr == HI_NULL || psMBuf->u64StartPhyAddr == 0) {
                HI_ERR_VENC("%s, failed to check arguments!\n", __func__);
                goto err_exit;
        }

        for (i=0; i<MAX_KMALLOC_MEM_NODE; i++) {
                if ((psMBuf->u64StartPhyAddr == g_venc_mem_node[i].phys_addr) &&
			(psMBuf->pStartVirAddr == g_venc_mem_node[i].virt_addr))
                {
                        break;
                }
        }

        if(i >= MAX_KMALLOC_MEM_NODE) {
                HI_ERR_VENC("%s, FATAL: No free ion mem node!\n", __func__);
                goto err_exit;
        }

        kfree(g_venc_mem_node[i].virt_addr);
        memset(&g_venc_mem_node[i], 0, sizeof(g_venc_mem_node[i]));/*lint !e866 */
        g_venc_node_num = (g_venc_node_num > 0)? (g_venc_node_num-1): 0;

        HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
        return HI_SUCCESS;

err_exit:
        HiVENC_UP_INTERRUPTIBLE(&g_VencMemSem);
        return HI_FAILURE;
}

HI_S32 HI_DRV_UserCopy(struct file *file, HI_U32 cmd, unsigned long arg,
			long (*func)(struct file *file, HI_U32 cmd, unsigned long uarg))
{
	//HI_CHAR  sbuf[768];
	HI_VOID  *parg = NULL;
	HI_S32   err   = -EINVAL;

	/*  Copy arguments into temp kernel buffer  */
	switch (_IOC_DIR(cmd)) {
	case _IOC_NONE:
	case _IOC_READ:
	case _IOC_WRITE:
	case (_IOC_WRITE | _IOC_READ):
		//if (_IOC_SIZE(cmd) <= sizeof(sbuf)) {
		if (_IOC_SIZE(cmd) <= MAX_BUFFER_SIZE) {
			parg = g_sbuf;
		} else {
			HI_FATAL_VENC("_IOC_SIZE(cmd) is too long!!\n");
			goto out;
		}
		err = -EFAULT;
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			if (copy_from_user(parg, (void __user*)arg, _IOC_SIZE(cmd))) {/*lint !e747 */
				HI_FATAL_VENC("copy_from_user failed, when use ioctl, the para must be a address, cmd=0x%x\n", cmd);
				goto out;
			}
		}
		break;
	default:
		goto out;
	}

	/* call driver */
	err = func(file, cmd, (long)(parg)); /*lint !e732 !e712*/
	if (err == -ENOIOCTLCMD)
		err = -EINVAL;
	if (err < 0)
		goto out;

	/*  Copy results into user buffer  */
	switch (_IOC_DIR(cmd)) {
	case _IOC_READ:
	case (_IOC_WRITE | _IOC_READ):
		if (copy_to_user((void __user *)arg, parg, _IOC_SIZE(cmd))) {/*lint !e747 */
			HI_FATAL_VENC("copy_to_user failed, when use ioctl, the para must be a address, cmd=0x%x\n", cmd);
			err = -EFAULT;
		}
		break;
	default:
		goto out;
	}
out:
	return err;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */



