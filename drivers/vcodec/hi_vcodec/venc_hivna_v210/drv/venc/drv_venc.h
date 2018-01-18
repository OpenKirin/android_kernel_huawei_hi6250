#ifndef __DRV_VENC_H__
#define __DRV_VENC_H__

#include "drv_venc_efl.h"
#include "drv_venc_ioctl.h"
#include <linux/hisi/hisi-iommu.h>
#include <linux/iommu.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

extern HI_U32 b_Regular_down_flag;

HI_S32 VENC_DRV_BoardInit(HI_VOID);
HI_VOID VENC_DRV_BoardDeinit(HI_VOID);
HI_S32  VENC_DRV_MemProcAdd(HI_VOID);
HI_VOID VENC_DRV_MemProcDel(HI_VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif //__DRV_VENC_H__
