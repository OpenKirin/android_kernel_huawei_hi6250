#ifndef __PLATFORM_H__
#define	__PLATFORM_H__

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/seq_file.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#include "regulator.h"

typedef enum {
	MODULE_VFMW,
	MODULE_VPSS,
	MODULE_BPP,
	MODULE_BUTT,
} OMX_MODULE_E;

#define ATOMIC_INC(pAtm)                   \
do {                                       \
    atomic_inc(pAtm);                      \
}while(0)

#define ATOMIC_DEC(pAtm)                   \
do {                                       \
    if (atomic_read(pAtm) != 0)            \
    {                                      \
        atomic_dec(pAtm);                  \
    }                                      \
}while(0)

#define ATOMIC_READ(pAtm)   atomic_read(pAtm)

#define VDEC_ASSERT_RETURN(Condition)      \
do {                                       \
    if (Condition)                         \
    {                                      \
        printk(KERN_ALERT "%s %d assert return\n", __func__, __LINE__); \
        return HI_FAILURE;                 \
    }                                      \
}while(0)

#define VDEC_INIT_MUTEX(pSem)              \
do {                                       \
    sema_init(pSem, 1);                    \
}while(0)

#define VDEC_DOWN_INTERRUPTIBLE(pSem)      \
do {                                       \
    if(down_interruptible(pSem))           \
    {                                      \
        printk(KERN_CRIT "%s down_interruptible failed\n", __func__); \
        return HI_FAILURE;                 \
    }                                      \
}while(0)

#define VDEC_UP_INTERRUPTIBLE(pSem)        \
do {                                       \
    up(pSem);                              \
}while(0)

#define PROC_PRINT(arg...)  seq_printf(arg)

#endif
