#ifndef __DRV_VENC_EFL_H__
#define __DRV_VENC_EFL_H__

#include "hi_type.h"
#include "hi_drv_mem.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* µ÷ÊÔº¯Êý */
#define pos()  printk("***### %s: L%d\n", __FUNCTION__, __LINE__ )
#define INVAILD_CHN_FLAG   (-1)

enum {
	VEDU_H265	= 0,
	VEDU_H264   = 1
};

typedef struct {
	HI_U32    IpFree;       /* for channel control */
	HI_HANDLE CurrHandle;   /* used in ISR */
	HI_U32   *pRegBase;
	HI_VOID  *pChnLock;     /* lock ChnCtx[MAX_CHN] */
	HI_VOID  *pTask_Frame;  /* for both venc & omxvenc */
	HI_VOID  *pTask_Stream; /* juse for omxvenc */
	HI_U32    StopTask;
	HI_U32    TaskRunning;  /* to block Close IP */
	HI_U32    bReEncode;
} VeduEfl_IpCtx_S;

typedef struct {
	HI_U32 IsFPGA;
	HI_U32 VeduIrqNumNorm;
	HI_U32 VeduIrqNumProt;
	HI_U32 VeduIrqNumSafe;
	HI_U32 VencRegBaseAddr;
	HI_U32 VencRegRange;
	HI_U32 normalRate;
	HI_U32 highRate;
	HI_U32 lowRate;
	HI_U64 SmmuPageBaseAddr;
} VeduEfl_DTS_CONFIG_S;

typedef struct {
       HI_U64 RdAddr;
       HI_U64 WrAddr;
}VeduEfl_SMMU_ERR_RW_ADDR;

HI_S32	VENC_DRV_EflOpenVedu(HI_VOID);
HI_S32	VENC_DRV_EflCloseVedu(HI_VOID);
HI_S32	VENC_DRV_EflResumeVedu(HI_VOID);
HI_S32  VENC_DRV_EflSuspendVedu(HI_VOID);
HI_S32  VENC_SetDtsConfig(VeduEfl_DTS_CONFIG_S* info);

/*************************************************************************************/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif //__DRV_VENC_EFL_H__
