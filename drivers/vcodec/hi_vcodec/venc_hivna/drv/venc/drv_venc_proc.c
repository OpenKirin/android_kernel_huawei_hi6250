/*
* Copyright 2004 - 2050, Hisilicon Tech. Co., Ltd.
* ALL RIGHTS RESERVED
* FileName   :  vi_proc.c
* Description:
*
*/

#include "hi_drv_mem.h"
#include "drv_venc.h"
#include "drv_venc_osal.h"

#include <linux/version.h>
#include <linux/proc_fs.h>

extern HI_S32 g_venc_node_num;
extern struct venc_mem_buf  g_venc_mem_node[MAX_KMALLOC_MEM_NODE];

static HI_S32  VENC_DRV_ProcRead(struct seq_file *p, HI_VOID *v);
static ssize_t VENC_DRV_ProcWrite(struct file *file, const char __user *buffer, size_t count, loff_t *ppos);
static HI_S32  VENC_DRV_MEM_ProcRead(struct seq_file *p, HI_VOID *v);

static HI_S32  VENC_DRV_ProcOpen(struct inode *inode, struct file *file)
{
	void* data = NULL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
	data = PDE(inode)->data;
#else
	data = PDE_DATA(inode); /*lint !e838 */
#endif
	if (!data)
		return single_open(file, VENC_DRV_MEM_ProcRead, data);
	else
		return single_open(file, VENC_DRV_ProcRead, data);
}

struct file_operations  VENC_PROC_FOPS = {
	.owner      = THIS_MODULE,/*lint !e64 */
	.open       = VENC_DRV_ProcOpen,
	.read       = seq_read,
	.write      = VENC_DRV_ProcWrite,
	.llseek     = seq_lseek,
	.release    = single_release,
};/*lint !e785 */

static HI_VOID VENC_DRV_ProcHelp(HI_VOID)
{
	HI_DRV_PROC_EchoHelper("------ VENC Proc Help ------\n");
	HI_DRV_PROC_EchoHelper("USAGE:echo [cmd] [para1] [para2] > /proc/msp/vencXX\n");
	HI_DRV_PROC_EchoHelper("cmd = set_print,   para1          set print level for Encode,(0x1:FATAL 0x3:Err 0x7: WARNING 0xf:INFO 0x1f:DEBUG)\n");
	HI_DRV_PROC_EchoHelper("cmd = save_yuv,    para1 = start  start to save the yuv data before Encode\n");
	HI_DRV_PROC_EchoHelper("cmd = save_yuv,    para1 = stop   stop to save the yuv data before Encode\n");
	HI_DRV_PROC_EchoHelper("cmd = save_stream, para1 = second save the streams after Encode for [para2] seconds\n");
	HI_DRV_PROC_EchoHelper("cmd = save_stream, para1 = frame  save the streams after Encode for [para2] frames\n");
	HI_DRV_PROC_EchoHelper("cmd = LowPowEn,    para1          if [para1]=0,means unable the low power control of algorithm\n");
	HI_DRV_PROC_EchoHelper("                                  if [para1]=0,means enable the low power control of algorithm\n");
}

static HI_S32 str2val(char *str, unsigned int *data)
{
	unsigned int i, d, dat, weight;

	dat = 0;
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
		i = 2;
		weight = 16;
	} else {
		i = 0;
		weight = 10;
	}

	for (; i < 10; i++) {
		if (str[i] < 0x20)
			break;
		else if (weight == 16 && str[i] >= 'a' && str[i] <= 'f')
			d = str[i] - 'a' + 10;/*lint !e732 !e834*/
		else if (weight == 16 && str[i] >= 'A' && str[i] <= 'F')
			d = str[i] - 'A' + 10;/*lint !e732 !e834*/
		else if (str[i] >= '0' && str[i] <= '9')
			d = str[i] - '0';/*lint !e732*/
		else
			return -1;

		dat = dat * weight + d;
	}

	*data = dat;

	return 0;
}

static HI_S32 VENC_DRV_MEM_ProcRead(struct seq_file *p, HI_VOID *v)
{
	HI_S32 i = 0;
	HI_U32 total_length = 0;

	PROC_PRINT(p, "================ MEM INFO ================\n\n");

	if (g_venc_node_num > 0) {
		PROC_PRINT(p, " %-10s%-10s\n\n", "PHYADDR", "SIZE");
		for (i = 0; i < MAX_KMALLOC_MEM_NODE; i++) {
			if (g_venc_mem_node[i].phys_addr!= 0) {
				PROC_PRINT(p, " %-10llu%-10d\n", g_venc_mem_node[i].phys_addr, g_venc_mem_node[i].size);
				total_length += g_venc_mem_node[i].size;
			}
		}
		PROC_PRINT(p, "\n");
		PROC_PRINT(p, "------------------------------------------\n\n");
	}

	PROC_PRINT(p, "%-10s:%d\n", " TotalNum",  g_venc_node_num);
	PROC_PRINT(p, "%-10s:%d\n", " TotalLength",  total_length);
	PROC_PRINT(p, "\n");
	PROC_PRINT(p, "==========================================\n\n");

	return 0;
}/*lint !e715*/

static HI_S32 VENC_DRV_ProcRead(struct seq_file *p, HI_VOID *v)
{
	/* VBR */
	PROC_PRINT(p,"\n");
	PROC_PRINT(p, "**********Real-time Statistics **********\n");

	return HI_SUCCESS;
}/*lint !e715*/

static ssize_t VENC_DRV_ProcWrite(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	HI_S32 i;
	HI_S32 j;
	HI_U32 parm;
	static HI_CHAR buf[256];
	static HI_CHAR str1[256];
	static HI_CHAR str2[256];
	static HI_CHAR str3[256];
	struct seq_file *q = file->private_data;

	HI_HANDLE hVenc = (HI_HANDLE)q->private;
	if (hVenc == 0)
		return 0;

	if (count >= sizeof(buf)) {
		HI_INFO_VENC("MMZ: your echo parameter string is too long!\n");
		return -EIO;
	}

	if (count >= 1) {
		HiMemSet(buf, 0, sizeof(buf));/*lint !e747*/
		if (copy_from_user(buf, buffer, count)) {
			HI_INFO_VENC("MMZ: copy_from_user failed!\n");
			return -EIO;
		}
		buf[count] = 0;

		/* dat1 */
		i = 0;
		j = 0;
		for (; i < count; i++) {/*lint !e737 !e574*/
			if (j == 0 && buf[i] == ' ')
				continue;
			if (buf[i] > ' ')
				str1[j++] = buf[i];
			if (j > 0 && buf[i] == ' ')
				break;
		}
		str1[j] = 0;

		/* dat2 */
		j = 0;
		for (; i < count; i++) {/*lint !e737 !e574*/
			if (j == 0 && buf[i] == ' ')
				continue;
			if (buf[i] > ' ')
				str2[j++] = buf[i];
			if (j > 0 && buf[i] == ' ')
				break;
		}
		str2[j] = 0;

		if (!HiStrNCmp(str1,"save_yuv",256)) {
			if (!HiStrNCmp(str2,"start",256)) // modfiy by xy 20161027
				;
			else if (!HiStrNCmp(str2,"stop",256))   // modfiy by xy 20161027
				;
			else
				VENC_DRV_ProcHelp();
        		} else if (!HiStrNCmp(str1,"save_stream",256)) {
			if (!HiStrNCmp(str2,"second",256)) {
				/*dat 3*/
				j = 0;
				for (; i < count; i++) {/*lint !e737 !e574*/
					if (j == 0 && buf[i] == ' ')
						continue;
					if (buf[i] > ' ')
						str3[j++] = buf[i];
					if (j > 0 && buf[i] == ' ')
						break;
				}
				str3[j] = 0;
				if (str2val(str3, &parm) != 0) {
					HI_ERR_VENC("error: echo cmd '%s' is worng!\n", buf);
					return HI_FAILURE;
				}

	               			if ( parm > 3600 ) {
					HI_ERR_VENC("error: not support save too large stream file!\n");
					return HI_FAILURE;
	                		}

				HI_DBG_VENC("now save stream %d second(to do)\n",parm);
		   	} else if (!HiStrNCmp(str2,"frame",256)) {
				/*dat 3*/
				j = 0;
				for (; i < count; i++) {/*lint !e737 !e574*/
					if (j==0 && buf[i]==' ')
						continue;
					if (buf[i] > ' ')
						str3[j++] = buf[i];
					if (j>0 && buf[i]==' ')
						break;
				}
				str3[j] = 0;

				if (str2val(str3, &parm) != 0) {
					HI_ERR_VENC("error: echo cmd '%s' is worng!\n", buf);
					return HI_FAILURE;
				}

				if (parm > 100000) {
					HI_ERR_VENC("error: not support save too large YUV file!\n");
					return HI_FAILURE;
				}

			} else {
				VENC_DRV_ProcHelp();
			}
		} else if (!HiStrNCmp(str1,"LowPowEn",256)) {
			HI_DBG_VENC("LowPowEn\n");

			if (!HiStrNCmp(str2,"0",256))
				;
			else if (!HiStrNCmp(str2,"1",256))
				;
			else
				VENC_DRV_ProcHelp();
		} else if (!HiStrNCmp(str1,"set_print",256)) {
			if(str2val(str2, &parm) != 0) {
				HI_ERR_VENC("error: echo cmd '%s' is worng!\n", buf);
				VENC_DRV_ProcHelp();
			} else {
				g_VencPrintEnable = parm;
				HI_DBG_VENC("set_print level:%d\n",parm);
			}
		} else {
			VENC_DRV_ProcHelp();
		}
	} else {
		VENC_DRV_ProcHelp();
	}

	return count;/*lint !e713*/

}/*lint !e715*/

#ifdef USER_DISABLE_VENC_PROC
HI_S32 VENC_DRV_MemProcAdd()
{
	struct proc_dir_entry* entry;

	entry = proc_create_data("venc_mem_info", 0, HI_NULL, &VENC_PROC_FOPS, NULL);
	if (!entry) {
		HI_ERR_VENC("proc create error\n");
		return -1;
	}

	HI_INFO_VENC("proc create success\n");
	return HI_SUCCESS;
}
#endif

#ifdef USER_DISABLE_VENC_PROC
HI_VOID VENC_DRV_MemProcDel()
{
	remove_proc_entry("venc_mem_info", NULL);
}
#endif

