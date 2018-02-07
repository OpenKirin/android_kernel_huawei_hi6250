#ifndef _VDM_HAL_HEADER_
#define _VDM_HAL_HEADER_

#include "vdm_drv.h"

#define   VDMHAL_OK                    (0)
#define   VDMHAL_ERR                   (-1)
#define   MAX_IMG_WIDTH_IN_MB          (512)
#define   MAX_IMG_HALF_HEIGHT_IN_MB    (256)
#define   MAX_IMG_HEIGHT_IN_MB         (MAX_IMG_HALF_HEIGHT_IN_MB*2)
#define   MAX_HOR_SIZE                 (MAX_IMG_WIDTH_IN_MB*16)
#define   MAX_VER_SIZE                 (MAX_IMG_HEIGHT_IN_MB*16)
#define   MAX_MB_NUM_IN_PIC            (MAX_IMG_WIDTH_IN_MB*MAX_IMG_HEIGHT_IN_MB)

#define   MAX_SLICE_SLOT_NUM           (200)

#define   FIRST_REPAIR                 (0)
#define   SECOND_REPAIR                (1)
#define   ALIGN_LEN                    (128)

#define   DEFAULT_EMAR_ID_VALUE        (0x161f7)

/************************************************************************/
/*  Register read/write interface                                       */
/************************************************************************/
/* mfde register read/write */
#define RD_VREG( reg, dat, VdhId )               \
do {                    \
	if (VdhId < MAX_VDH_NUM)                \
		dat = *((volatile SINT32*)((SINT8*)g_HwMem[VdhId].pVdmRegVirAddr + reg)); \
	else                 \
		dprint(PRN_ALWS,"%s: RD_VREG but VdhId : %d is more than MAX_VDH_NUM : %d\n", __func__, VdhId, MAX_VDH_NUM); \
} while(0)

#define WR_VREG( reg, dat, VdhId )               \
do {                     \
	if (VdhId < MAX_VDH_NUM)                \
		*((volatile SINT32*)((SINT8*)g_HwMem[VdhId].pVdmRegVirAddr + reg)) = dat; \
	else                 \
		dprint(PRN_ALWS,"%s: WR_VREG but VdhId : %d is more than MAX_VDH_NUM : %d\n", __func__, VdhId, MAX_VDH_NUM); \
} while(0)

/* crg register read/write */
#define RD_CRG_VREG( reg, dat )               \
do {                    \
	dat = *((volatile SINT32*)((SINT8*)g_HwMem[0].pPERICRGVirAddr + reg)); \
} while(0)

#define WR_CRG_VREG( reg, dat )               \
do {                    \
	*((volatile SINT32*)((SINT8*)g_HwMem[0].pPERICRGVirAddr + reg)) = dat; \
} while(0)

/* bpd register read/write */
#define RD_BPD_VREG( reg, dat )               \
do {                    \
	dat = *((volatile SINT32*)((SINT8*)g_HwMem[0].pBpdRegVirAddr + reg)); \
} while(0)

#define WR_BPD_VREG( reg, dat )               \
do {                    \
	*((volatile SINT32*)((SINT8*)g_HwMem[0].pBpdRegVirAddr + reg)) = dat; \
} while(0)

/* message pool read/write */
#define RD_MSGWORD( vir_addr, dat )          \
do {                 \
	dat = *((volatile SINT32*)((SINT8*)vir_addr));                      \
} while(0)

#define WR_MSGWORD( vir_addr, dat )          \
do {                 \
	*((volatile SINT32*)((SINT8*)(vir_addr))) = dat;                      \
} while(0)

/* condition check */
#define VDMHAL_ASSERT_RET( cond, else_print )     \
do {               \
	if (!(cond)) {           \
		dprint(PRN_FATAL,"%s %d: %s\n", __func__, __LINE__,  else_print ); \
		return VDMHAL_ERR;          \
	}               \
} while(0)

#define VDMHAL_ASSERT( cond, else_print )      \
do {               \
	if (!(cond)) {           \
		dprint(PRN_FATAL,"%s: %s\n", __func__, else_print ); \
		return;             \
	}               \
}while(0)

#define    MPEG2_DUMMY_BITS     24    // 40

#define    SHORT_HEADER_ID       1
#define    NON_SHORT_HEADER_ID   2
#define RSHIFT(a,b) ( (a)>0 ? (((a) + ((1<<(b))>>1))>>(b)) : (((a) + ((1<<(b))>>1)-1)>>(b)))

/* Filter strength tables */
/* default strength specified by RV_Default_Deblocking_Strength (0) */
#define RV8_Default_Deblocking_Strength     0

/* Filter strength tables */
/* default strength specified by RV_Default_Deblocking_Strength (0) */
#define RV9_Default_Deblocking_Strength     0

#endif
