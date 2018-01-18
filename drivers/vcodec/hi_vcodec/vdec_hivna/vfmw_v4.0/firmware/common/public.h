#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include "basedef.h"
#include "vfmw.h"

#define  DEFAULT_PRINT_ENABLE   (0x0)
#define  DEFAULT_PRINT_DEVICE   (DEV_SCREEN)

typedef enum {
	DEV_SCREEN = 1,
	DEV_SYSLOG,
	DEV_FILE,
	DEV_MEM
} PRINT_DEVICE_TYPE;

typedef enum {
	PRN_FATAL = 0,
	PRN_ERROR,
	PRN_CTRL,
	PRN_VDMREG,

	PRN_DNMSG,
	PRN_RPMSG,
	PRN_UPMSG,
	PRN_STREAM,

	PRN_STR_HEAD,
	PRN_STR_TAIL,
	PRN_STR_BODY,
	PRN_IMAGE,

	PRN_QUEUE,
	PRN_REF,
	PRN_DPB,
	PRN_POC,

	PRN_MARK_MMCO,
	PRN_SEQ,
	PRN_PIC,
	PRN_SLICE,

	PRN_SEI,
	PRN_SE,
	PRN_DBG,
	PRN_BLOCK,

	PRN_SCD_REGMSG,
	PRN_SCD_STREAM,
	PRN_SCD_INFO,
	PRN_CRC,

	PRN_POST,
	PRN_PTS,
	PRN_DEC_MODE,
	PRN_FS,

	PRN_ALWS = 32
} PRINT_MSG_TYPE;

extern UINT32 g_PrintEnable;
extern UINT32 g_PrintDevice;
extern SINT32 g_TraceCtrl;

#define dprint_sos_kernel(type, fmt, arg...)                        \
do{                                                                 \
    if (((PRN_ALWS == type) || (0 != (g_PrintEnable & (1LL << type))) \
        && (DEV_SCREEN == g_PrintDevice)))                          \
    {                                                               \
        VFMW_OSAL_Print("S: ");                                     \
        VFMW_OSAL_Print(fmt, ##arg);                                \
    }                                                               \
}while(0)

#ifdef HI_ADVCA_FUNCTION_RELEASE
#define dprint(type, fmt, arg...)  vfmw_dprint_nothing()
#else

#ifdef ENV_SOS_KERNEL
#define dprint(type, fmt, arg...)  dprint_sos_kernel(type, fmt, ##arg)
#else
#define dprint(type, fmt, arg...)  dprint_linux_kernel(type, fmt, ##arg)
#endif

#endif

VOID vfmw_dprint_nothing(VOID);

#ifdef ENV_ARMLINUX_KERNEL
SINT32 dprint_linux_kernel(UINT32 type, const SINT8 * format, ...);
#endif

#endif
