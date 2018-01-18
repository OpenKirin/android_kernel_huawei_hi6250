#ifndef __VDEC_FIRMWARE_H__
#define __VDEC_FIRMWARE_H__

#if defined(VFMW_EXTRA_TYPE_DEFINE)
#include "hi_type.h"
#endif

#define  VFMW_VERSION_NUM       (2017032400)

#if defined(VFMW_EXTRA_TYPE_DEFINE)
#define UINT64 HI_U64
#define SINT64 HI_S64
#define UINT32 HI_U32
#define SINT32 HI_S32
#define UINT16 HI_U16
#define SINT16 HI_S16
#define UINT8  HI_U8
#define SINT8  HI_S8
#define ULONG  HI_SIZE_T
#define UADDR  HI_U32

#ifndef VOID
#define VOID   HI_VOID
#endif

typedef const void CONSTVOID;
typedef unsigned   USIGN;
#else
typedef  unsigned long long UINT64;
typedef  long long          SINT64;
typedef  unsigned int       UINT32;
typedef  signed int         SINT32;
typedef  unsigned short     UINT16;
typedef  signed short       SINT16;
typedef  signed char        SINT8;
typedef  unsigned char      UINT8;
typedef  void               VOID;
typedef  unsigned int       UADDR;
typedef  const void         CONSTVOID;
typedef  unsigned           USIGN;
typedef  unsigned long      ULONG;
#endif

#ifndef NULL
#define NULL               0L
#endif

#if  defined(PRODUCT_STB) || defined(PRODUCT_DPT)
#define COMMAND_LINE_EXTRA
#endif

#ifdef HI_TVP_SUPPORT
#define  TVP_CHAN_NUM            (2)
#else
#define  TVP_CHAN_NUM            (0)
#endif

#ifdef ENV_SOS_KERNEL
#define  MAX_CHAN_NUM            (TVP_CHAN_NUM)
#else
#define  MAX_CHAN_NUM            (32)
#endif

#define MAX_FRAME_NUM           (32)

#define  VDEC_OK                (0)
#define  VDEC_ERR               (-1)
#define  VDEC_ERR_WRONG_PARAM   (-2)
#define  VDEC_ERR_NO_MEM        (-3)
#define  VDEC_ERR_VDEC_BUSY     (-4)
#define  VDEC_ERR_CHAN_FULL     (-5)
#define  VDEC_ERR_CHAN_RUN      (-6)
#define  VDEC_ERR_CHAN_STOP     (-7)
#define  VDEC_ERR_UNSUPPORT     (-8)
#define  VDEC_ERR_DEFAUT        (-10)

#define  VF_ERR_SYS             (-20)

typedef enum {
	VFMW_START_RESERVED = 0,
	VFMW_H264           = 0,
	VFMW_VC1,
	VFMW_MPEG4,
	VFMW_MPEG2,
	VFMW_H263,
	VFMW_DIVX3,
	VFMW_AVS,
	VFMW_JPEG,
	VFMW_REAL8 = 8,
	VFMW_REAL9 = 9,
	VFMW_VP6   = 10,
	VFMW_VP6F,
	VFMW_VP6A,
	VFMW_VP8,
	VFMW_VP9,
	VFMW_SORENSON,
	VFMW_MVC,
	VFMW_HEVC,
	VFMW_RAW,
	VFMW_USER,    /*## vfmw simply provide frame path. for external decoder, eg. mjpeg ## */
	VFMW_END_RESERVED
} VID_STD_E;

#define STD_START_RESERVED VFMW_START_RESERVED
#define STD_MPEG2          VFMW_MPEG2
#define STD_JPEG           VFMW_JPEG
#define STD_REAL8          VFMW_REAL8
#define STD_REAL9          VFMW_REAL9
#define STD_VP8            VFMW_VP8
#define STD_VP9            VFMW_VP9
#define STD_SORENSON       VFMW_SORENSON
#define STD_MVC            VFMW_MVC
#define STD_HEVC           VFMW_HEVC
#define STD_RAW            VFMW_RAW
#define STD_USER           VFMW_USER
#define STD_END_RESERVED   VFMW_END_RESERVED

/*memory type*/
typedef enum {
	MEM_ION = 0,    // ion default
	MEM_ION_CTG,    // ion contigeous
	MEM_CMA,        // kmalloc
	MEM_CMA_ZERO,    // kzalloc
} MEM_TYPE_E;

/* memroy description */
typedef struct {
	UINT8 IsSecure;
	MEM_TYPE_E MemType;
	UINT64 PhyAddr;
	UINT32 Length;
	UINT64 VirAddr;
} MEM_DESC_S;

typedef struct {
	UINT32 IsFPGA;
	UINT32 VdecIrqNumNorm;
	UINT32 VdecIrqNumProt;
	UINT32 VdecIrqNumSafe;
	UINT32 VdhRegBaseAddr;
	UINT32 VdhRegRange;
	UINT64 SmmuPageBaseAddr;
	UINT32 PERICRG_RegBaseAddr;
} VFMW_DTS_CONFIG_S;

#endif    // __VDEC_FIRMWARE_H__
