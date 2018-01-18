#ifndef __VDM_DRV_HEADER__
#define __VDM_DRV_HEADER__
#include "vfmw.h"
#include "sysconfig.h"

#define VDH_IDLE                  (0)
#define VDH_BUSY                  (1)

#define VDM_TIME_OUT              (500)
#define VDM_FPGA_TIME_OUT         (500000)

#define VDMDRV_OK                 (0)
#define VDMDRV_ERR                (-1)

#define MSG_SLOT_SIZE             (256)
#define UP_MSG_SLOT_NUM           (2)
#define MAX_UP_MSG_SLICE_NUM      (200)

#define UP_MSG_SIZE               (MAX_UP_MSG_SLICE_NUM*4)
#define LUMA_HISTORGAM_NUM        (32)

typedef enum {
	VDH_STATE_REG   = 1,
	INT_STATE_REG   = 2,
	INT_MASK_REG    = 3,
	VCTRL_STATE_REG = 4,
} REG_ID_E;

typedef enum {
	VDM_IDLE_STATE     = 0,
	VDM_DECODE_STATE   = 1,
	VDM_REPAIR_STATE_0 = 2,
	VDM_REPAIR_STATE_1 = 3
} VDMDRV_STATEMACHINE_E;

typedef enum {
	VDMDRV_SLEEP_STAGE_NONE = 0,
	VDMDRV_SLEEP_STAGE_PREPARE,
	VDMDRV_SLEEP_STAGE_SLEEP
} VDMDRV_SLEEP_STAGE_E;

typedef enum {
	FIRST_REPAIR = 0,
	SECOND_REPAIR
} REPAIRTIME_S;

typedef enum hi_CONFIG_VDH_CMD {
	CONFIG_VDH_AfterDec_CMD = 200,
	CONFIG_VDH_ACTIVATEDEC_CMD
} CONFIG_VDH_CMD;

typedef struct {
	CONFIG_VDH_CMD vdh_cmd;
	UINT32 vdh_reset_flag;
	UINT32 GlbResetFlag;
	SINT32 VdhStartRepairFlag;
	SINT32 VdhStartHwDecFlag;
	SINT32 VdhAvsFlag;
	SINT32 VdhSelRst;
	SINT32 VdhBasicCfg0;
	SINT32 VdhBasicCfg1;
	SINT32 VdhAvmAddr;
	SINT32 VdhVamAddr;
	SINT32 VdhStreamBaseAddr;
	SINT32 VdhEmarId;
	SINT32 VdhSedTo;
	SINT32 VdhItransTo;
	SINT32 VdhPmvTo;
	SINT32 VdhPrcTo;
	SINT32 VdhRcnTo;
	SINT32 VdhDblkTo;
	SINT32 VdhPpfdTo;
	SINT32 VdhPartLevel;
	SINT32 VdhYstAddr;
	SINT32 VdhYstride;
	SINT32 VdhUvstride;//VREG_UVSTRIDE_1D
	SINT32 VdhDdrInterleaveMode;//VREG_DDR_INTERLEAVE_MODE
	SINT32 VdhCfgInfoAddr;//CFGINFO_ADDR
	SINT32 VdhUvoffset;
	SINT32 VdhHeadInfOffset;
	SINT32 VdhLineNumAddr;
	SINT32 VdhPpfdBufAddr;
	SINT32 VdhPpfdBufLen;
	SINT32 VdhRefPicType;
	SINT32 VdhFfAptEn;
	SINT32 VdhIntState;
	SINT32 VdhIntMask;
	SINT32 LowlyEnable;
	VDMDRV_STATEMACHINE_E VdmStateMachine;
	REPAIRTIME_S RepairTime;
	VID_STD_E VidStd;
	SINT32 PicStruct;
	SINT32 ValidGroupNum0;
	SINT32 ValidGroupNum1;
	SINT32 ErrRationAndRpStratageFlag;
	SINT32 IsMpeg4Nvop;
	SINT32 IsVc1Skpic;
	SINT32 IsVp6Nvop;
	SINT32 AvsFirstFld;
	SINT32 AvsSecondFld;
} OMXVDH_REG_CFG_S;

typedef struct {
	VDMDRV_STATEMACHINE_E VdmStateMachine;
	SINT32     ErrRatio;
	SINT32     ChanId;
	VID_STD_E  VidStd;
	UINT32     StartTime;
	UINT32     CurrTime;
	SINT32     VdmTimeOut;
	UINT32     LastWaitMoreStartTime;
	SINT32     ChanResetFlag;
	VOID       *pDecParam;
} VDMDRV_PARAM_S;

typedef struct {
	// vdm register base vir addr
	SINT32 *pVdmRegVirAddr;
	SINT32 *pPERICRGVirAddr;
	SINT32 *pBpdRegVirAddr;

	// vdm hal base addr
	UADDR HALMemBaseAddr;
	SINT32 HALMemSize;
	SINT32 VahbStride;

	/* message pool */
	UADDR MsgSlotAddr[256];
	SINT32 ValidMsgSlotNum;

	/* vlc code table */
	UADDR H264TabAddr;     /* 32 Kbyte */
	UADDR MPEG2TabAddr;    /* 32 Kbyte */
	UADDR MPEG4TabAddr;    /* 32 Kbyte */
	UADDR AVSTabAddr;      /* 32 Kbyte */
	UADDR VC1TabAddr;
	/* cabac table */
	UADDR H264MnAddr;
	/* nei info for vdh for hevc  */
	UADDR  sed_top_phy_addr;
	UADDR  pmv_top_phy_addr;
	UADDR  pmv_left_phy_addr;
	UADDR  rcn_top_phy_addr;
	UADDR  mn_phy_addr;
	UADDR  tile_segment_info_phy_addr;
	UADDR  dblk_left_phy_addr;
	UADDR  dblk_top_phy_addr;
	UADDR  sao_left_phy_addr;
	UADDR  sao_top_phy_addr;
	UADDR  ppfd_phy_addr;
	SINT32 ppfd_buf_len;

	/*nei info for vdh */
	UADDR SedTopAddr;    /* len = 64*4*x */
	UADDR PmvTopAddr;    /* len = 64*4*x */
	UADDR RcnTopAddr;    /* len = 64*4*x */
	UADDR ItransTopAddr;
	UADDR DblkTopAddr;
	UADDR PpfdBufAddr;
	UADDR PpfdBufLen;

	UADDR IntensityConvTabAddr;
	UADDR BitplaneInfoAddr;
	UADDR Vp6TabAddr;
	UADDR Vp8TabAddr;
	
	/* VP9 */
	SINT32    DblkLeftAddr;
	SINT32    Vp9ProbTabAddr;
	SINT32    Vp9ProbCntAddr;

	UINT8 *luma_2d_vir_addr;
	UADDR luma_2d_phy_addr;
	UINT8 *chrom_2d_vir_addr;
	UADDR chrom_2d_phy_addr;
} VDMHAL_HWMEM_S;

typedef struct {
	UINT32 Int_State_Reg;

	UINT32 BasicCfg1;
	UINT32 VdmState;
	UINT32 Mb0QpInCurrPic;
	UINT32 SwitchRounding;
	UINT32 SedSta;
	UINT32 SedEnd0;

	UINT32 DecCyclePerPic;
	UINT32 RdBdwidthPerPic;
	UINT32 WrBdWidthPerPic;
	UINT32 RdReqPerPic;
	UINT32 WrReqPerPic;
	UINT32 LumaSumHigh;
	UINT32 LumaSumLow;
	UINT32 LumaHistorgam[LUMA_HISTORGAM_NUM];
} VDMHAL_BACKUP_S;

typedef struct {
	VID_STD_E VidStd;
	UADDR     ImageAddr;
	UADDR     Image2DAddr;
	SINT32    VahbStride;
	UADDR     RefImageAddr;
	UADDR     CurrPmvAddr;
	SINT32    ImageWidth;
	SINT32    ImageHeight;
	SINT32    IsFrame;
	SINT32    ImageCSP;
	struct {
		SINT16 StartMbn;
		SINT16 EndMbn;
	} MbGroup[MAX_UP_MSG_SLICE_NUM];
	SINT32 ValidGroupNum;
	SINT32 rpr_ref_pic_type;

	SINT32 Compress_en;
	SINT32 Pic_type;
	SINT32 FullRepair;
	UINT32 level;
	UINT32 CtbSize;
	UINT32 tiles_enabled_flag;
	UINT32     Stride1D;
	UINT32     uvOffset;
	UINT32     uvstride_1d;
	UINT32     vdh_2d_en;
	UINT32     BitDepth;
	UINT32     head_stride;
	UINT32     src_head_ystaddr;
	UINT32     src_head_cstaddr;
	UINT32     dst_head_ystaddr;
	UINT32     dst_head_cstaddr;
} VDMHAL_REPAIR_PARAM_S;

typedef struct {
	UINT32 *pMb0QpInCurrPic;
	SINT32 VdhId;
} BACKUP_INFO_S;

extern VDMHAL_HWMEM_S g_HwMem[MAX_VDH_NUM];
#endif
