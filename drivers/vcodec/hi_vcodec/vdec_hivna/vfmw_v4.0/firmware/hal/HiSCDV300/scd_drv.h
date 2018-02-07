#ifndef __SCD_DRV_H__
#define __SCD_DRV_H__

#include "basedef.h"
#include "mem_manage.h"
#include "vfmw.h"

#define SCDDRV_OK                      (0)
#define SCDDRV_ERR                     (-1)

#define SCD_TIME_OUT_COUNT             (200)

#define REG_SCD_START                  (0x800)
#define REG_LIST_ADDRESS               (0x804)
#define REG_UP_ADDRESS                 (0x808)
#define REG_UP_LEN                     (0x80c)
#define REG_BUFFER_FIRST               (0x810)
#define REG_BUFFER_LAST                (0x814)
#define REG_BUFFER_INI                 (0x818)
#define REG_SCD_PROTOCOL               (0x820)

#define REG_DSP_SPS_MSG_ADDRESS        (0x828)
#define REG_DSP_PPS_MSG_ADDRESS        (0x82c)
#define REG_DVM_MSG_ADDRESS            (0x830)
#define REG_SED_TOP_ADDRESS            (0x834)
#define REG_CA_MN_ADDRESS              (0x838)

/* state registers */
#define REG_SCD_OVER                   (0x840)
#define REG_SCD_INT                    (0x844)
#define REG_SCD_NUM                    (0x84c)
#define REG_ROLL_ADDR                  (0x850)
#define REG_SRC_EATEN                  (0x854)

#define REG_SCD_SAFE_INT_MASK          (0x884)
#define REG_SCD_SAFE_INI_CLR           (0x888)
#define REG_SCD_NORM_INT_MASK          (0x81c)
#define REG_SCD_NORM_INI_CLR           (0x824)

#ifdef ENV_SOS_KERNEL
#define REG_SCD_INT_MASK               REG_SCD_SAFE_INT_MASK
#define REG_SCD_INI_CLR                REG_SCD_SAFE_INI_CLR
#else
#define REG_SCD_INT_MASK               REG_SCD_NORM_INT_MASK
#define REG_SCD_INI_CLR                REG_SCD_NORM_INI_CLR
#endif

#define REG_AVS_FLAG                   (0x0000)
#define REG_EMAR_ID                    (0x0004)
#define REG_VDH_SELRST                 (0x0008)
#define REG_VDH_ARBIT_CTRL_STATE       (0X0010)
#define REG_VDH_CK_GT                  (0x000c)
#define REG_DSP_WATCH_DOG              (0X0018)

typedef enum {
	FMW_OK          = 0,
	FMW_ERR_PARAM   = -1,
	FMW_ERR_NOMEM   = -2,
	FMW_ERR_NOTRDY  = -3,
	FMW_ERR_BUSY    = -4,
	FMW_ERR_RAWNULL = -5,
	FMW_ERR_SEGFULL = -6,
	FMW_ERR_SCD     = -7
} FMW_RETVAL_E;

typedef enum {
	SCDDRV_SLEEP_STAGE_NONE = 0,
	SCDDRV_SLEEP_STAGE_PREPARE,
	SCDDRV_SLEEP_STAGE_SLEEP
} SCDDRV_SLEEP_STAGE_E;

typedef enum {
	SCD_IDLE = 0,
	SCD_WORKING,
} SCD_STATE_E;

/* register operator */
#define RD_SCDREG(reg)       MEM_ReadPhyWord((gScdRegBaseAddr + reg))
#define WR_SCDREG(reg, dat)  MEM_WritePhyWord((gScdRegBaseAddr + reg),(dat))

#define FMW_ASSERT_RET( cond, ret )                     \
do{                                     \
	if (!(cond))                             \
		return (ret);                           \
} while (0)

/*######################################################
       struct defs.
 ######################################################*/

typedef struct {
	SINT8  ScdIntMask;
	SINT8  SliceCheckFlag;
	SINT8  ScdStart;
	UADDR  DownMsgPhyAddr;
	UADDR  UpMsgPhyAddr;
	SINT32 UpLen;
	UADDR  BufferFirst;
	UADDR  BufferLast;
	UADDR  BufferIni;
	SINT32 ScdProtocol;
	SINT32 ScdIniClr;
	UADDR  DspSpsMsgMemAddr;
	SINT32 DspSpsMsgMemSize;
	UADDR  DspPpsMsgMemAddr;
	SINT32 DspPpsMsgMemSize;
	UADDR  DvmMemAddr;
	SINT32 DvmMemSize;
	UADDR  DspSedTopMemAddr;
	SINT32 DspSedTopMemSize;
	UADDR  DspCaMnMemAddr;
	SINT32 DspCaMnMemSize;
	SINT32 ScdLowdlyEnable;
	SINT32 reg_avs_flag;

	SINT32 *pDownMsgVirAddr;
	SINT32 *pUpMsgVirAddr;
	SINT32 *pDspSpsMsgMemVirAddr;
	SINT32 *pDspPpsMsgMemVirAddr;
	SINT32 *pDvmMemVirAddr;
	SINT32 *pDspSedTopMemVirAddr;
	SINT32 *pDspCaMnMemVirAddr;
} SM_CTRLREG_S;

typedef struct {
	SINT8  ScdIntMask;
	SINT8  SliceCheckFlag;
	SINT8  ScdStart;
	UADDR  DownMsgPhyAddr;
	UADDR  UpMsgPhyAddr;
	SINT32 UpLen;
	UADDR  BufferFirst;
	UADDR  BufferLast;
	UADDR  BufferIni;
	SINT32 ScdProtocol;
	SINT32 ScdIniClr;
	UADDR  DspSpsMsgMemAddr;
	SINT32 DspSpsMsgMemSize;
	UADDR  DspPpsMsgMemAddr;
	SINT32 DspPpsMsgMemSize;
	UADDR  DvmMemAddr;
	SINT32 DvmMemSize;
	UADDR  DspSedTopMemAddr;
	SINT32 DspSedTopMemSize;
	UADDR  DspCaMnMemAddr;
	SINT32 DspCaMnMemSize;
	SINT32 ScdLowdlyEnable;
	SINT32 reg_avs_flag;

	compat_ulong_t pDownMsgVirAddr;
	compat_ulong_t pUpMsgVirAddr;
	compat_ulong_t pDspSpsMsgMemVirAddr;
	compat_ulong_t pDspPpsMsgMemVirAddr;
	compat_ulong_t pDvmMemVirAddr;
	compat_ulong_t pDspSedTopMemVirAddr;
	compat_ulong_t pDspCaMnMemVirAddr;
} COMPAT_SM_CTRLREG_S;

typedef struct {
	SINT32 Scdover;
	SINT32 ScdInt;
	SINT32 ShortScdNum;
	SINT32 ScdNum;
	UADDR ScdRollAddr;
	SINT32 SrcEaten;
} SM_STATEREG_S;

typedef struct {
	HI_S32 ScdProtocol;
	HI_S32 Scdover;
	HI_S32 ScdInt;
	HI_S32 ScdNum;
	HI_U32 ScdRollAddr;
	HI_S32 SrcEaten;
	HI_S32 UpLen;
} SCD_STATE_REG_S;

typedef enum hi_CONFIG_SCD_CMD {
	CONFIG_SCD_REG_CMD = 100,
} CONFIG_SCD_CMD;

typedef struct {
	CONFIG_SCD_CMD cmd;
	SINT32         eVidStd;
	UINT32         SResetFlag;
	UINT32         GlbResetFlag;
	SM_CTRLREG_S   SmCtrlReg;
} OMXSCD_REG_CFG_S;

typedef struct {
	CONFIG_SCD_CMD cmd;
	SINT32         eVidStd;
	COMPAT_SM_CTRLREG_S IoctlSmCtrlReg;
} COMPAT_OMXSCD_REG_CFG_S;

SINT32 SCDDRV_PrepareSleep(VOID);

SCDDRV_SLEEP_STAGE_E SCDDRV_GetSleepStage(VOID);
VOID SCDDRV_SetSleepStage(SCDDRV_SLEEP_STAGE_E sleepState);

VOID SCDDRV_ForceSleep(VOID);

VOID SCDDRV_ExitSleep(VOID);

SINT32 SCDDRV_ResetSCD(VOID);

SINT32 SCDDRV_WriteReg(SM_CTRLREG_S *pSmCtrlReg);

VOID SCDDRV_GetRegState(SCD_STATE_REG_S *pScdStateReg);

VOID SCDDRV_ISR(VOID);

VOID SCDDRV_init(VOID);

VOID SCDDRV_DeInit(VOID);

#ifdef ENV_ARMLINUX_KERNEL
SINT32 SCDDRV_IsScdIdle(VOID);
#endif
#endif
