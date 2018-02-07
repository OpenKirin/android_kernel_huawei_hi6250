#ifndef __VFMW_INTF_H__
#define __VFMW_INTF_H__
#include "vdm_drv.h"
#include "scd_drv.h"
#include "bitplane.h"

#define VCTRL_OK                0
#define VCTRL_ERR              -1
#define VCTRL_ERR_VDM_BUSY     -2

typedef struct hiDRV_MEM_S {
	MEM_RECORD_S stVdhReg;
	MEM_RECORD_S stCrgReg;
} DRV_MEM_S;

SINT32 VCTRL_OpenDrivers(VOID);

SINT32 VCTRL_OpenVfmw(VOID);

SINT32 VCTRL_CloseVfmw(VOID);

SINT32 VCTRL_VDMHal_Process(OMXVDH_REG_CFG_S *pVdmRegCfg, VDMHAL_BACKUP_S *pVdmRegState);

SINT32 VCTRL_SCDHal_Process(OMXSCD_REG_CFG_S *pScdRegCfg, SCD_STATE_REG_S *pScdStateReg);

SINT32 VCTRL_BPDHal_Process(OMXBPD_REG_S *pBpdReg);

SINT32 VCTRL_VDMHAL_IsRun(VOID);

VOID VCTRL_Suspend(VOID);

VOID VCTRL_Resume(VOID);

HI_BOOL VCTRL_Scen_Ident(VOID);

#endif
