#ifndef __LINUX_SHMEM_H__
#define __LINUX_SHMEM_H__
#include "protocol.h"

#include <global_ddr_map.h>

#ifndef HISI_CONTEXTHUB_THERMAL_SHMEM_PHYMEM_BASE
#define HISI_CONTEXTHUB_THERMAL_SHMEM_PHYMEM_BASE      HISI_RESERVED_SENSORHUB_SHMEM_PHYMEM_BASE
#define HISI_CONTEXTHUB_THERMAL_SHMEM_PHYMEM_SIZE      (0x3C00)

/*loadmonitor*/
#define LOADMONITOR_PHYMEM_BASE                        (HISI_CONTEXTHUB_THERMAL_SHMEM_PHYMEM_BASE + HISI_CONTEXTHUB_THERMAL_SHMEM_PHYMEM_SIZE)
#define LOADMONITOR_PHYMEM_SIZE                        (0x1000)

#define HISI_RESERVED_CH_BLOCK_SHMEM_PHYMEM_BASE       (LOADMONITOR_PHYMEM_BASE + LOADMONITOR_PHYMEM_SIZE)
#define HISI_RESERVED_CH_BLOCK_SHMEM_PHYMEM_SIZE       (HISI_RESERVED_SENSORHUB_SHMEM_PHYMEM_BASE \
                                                           + HISI_RESERVED_SENSORHUB_SHMEM_PHYMEM_SIZE \
                                                           - HISI_RESERVED_CH_BLOCK_SHMEM_PHYMEM_BASE)
#endif

extern int shmem_notifier_register(obj_tag_t module_id,
	void (*notifier_call)(void __iomem *buf_addr, unsigned int buf_size));
extern int shmem_notifier_unregister(obj_tag_t module_id);
extern int shmem_send(obj_tag_t module_id, const void *usr_buf, unsigned int usr_buf_size);
extern int __init contexthub_shmem_init(void);
extern const pkt_header_t *shmempack(const char *buf, unsigned int length);

#endif
