
#include "Vedu_RegAll.h"

#ifndef __DRV_VENC_IOCTL_H__
#define __DRV_VENC_IOCTL_H__
#ifdef __cplusplus
#if __cplusplus
extern "C" {
 #endif
#endif

typedef enum
{
	VENC_SET_CFGREG = 100,
	VENC_SET_CFGREGSIMPLE
}CMD_TYPE;

typedef enum {
	CLK_RATE_LOW = 0,
	CLK_RATE_NORMAL,
	CLK_RATE_HIGH,
} VENC_CLK_TYPE;

typedef struct
{
	CMD_TYPE cmd;

	HI_BOOL bResetReg;
	HI_BOOL bClkCfg;
	HI_BOOL bFirstNal2Send;
	unsigned int   bSecureFlag;
	U_FUNC_VCPI_RAWINT    hw_done_type;
	S_HEVC_AVC_REGS_TYPE_CFG all_reg;
	VENC_CLK_TYPE clk_type;
}VENC_REG_INFO_S;

#define CMD_VENC_GET_VENCCOUNT         _IOR(IOC_TYPE_VENC, 0x31, atomic_t)
#define CMD_VENC_START_ENCODE          _IOWR(IOC_TYPE_VENC, 0x32, VENC_REG_INFO_S)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif //__HI_DRV_VENC_H__
