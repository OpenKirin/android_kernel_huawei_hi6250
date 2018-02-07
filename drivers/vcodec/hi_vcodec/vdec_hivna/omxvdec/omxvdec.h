#ifndef __OMXVDEC_H__
#define __OMXVDEC_H__

#include "platform.h"
#include "drv_omxvdec.h"

#define OMXVDEC_VERSION	 		          (2017032300)
#define MAX_OPEN_COUNT                    (32)

#define OMX_ALWS                          (30)
#define OMX_FATAL                         (0)
#define OMX_ERR                           (1)
#define OMX_WARN                          (2)
#define OMX_INFO                          (3)
#define OMX_TRACE                         (4)
#define OMX_INBUF                         (5)
#define OMX_OUTBUF                        (6)
#define OMX_VPSS                          (7)
#define OMX_VER                           (8)
#define OMX_PTS                           (9)
#define OMX_MEM                           (10)

extern HI_U32 g_TraceOption;

#ifndef HI_ADVCA_FUNCTION_RELEASE
#define OmxPrint(flag, format,arg...) \
    do { \
        if (OMX_ALWS == flag || (0 != (g_TraceOption & (1 << flag)))) \
            printk(KERN_ALERT format, ## arg); \
    } while (0)
#else
#define OmxPrint(flag, format,arg...)    ({do{}while(0);0;})
#endif

typedef struct {
	HI_U32 open_count;
	atomic_t nor_chan_num;
	atomic_t sec_chan_num;
	struct semaphore omxvdec_mutex;
	struct semaphore vdec_mutex_scd;
	struct semaphore vdec_mutex_bpd;
	struct semaphore vdec_mutex_vdh;
	struct cdev cdev;
	struct device *device;
} OMXVDEC_ENTRY;

#endif
