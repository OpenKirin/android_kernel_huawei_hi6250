#ifndef __ICS_DEBUG_PROXY_H__
#define __ICS_DEBUG_PROXY_H__

#include "cambricon_ipu.h"

int call_regulator_ip_vipu_enable(void);
int call_regulator_ip_vipu_disable(void);
int call_ipu_reset_proc(unsigned int addr);
void call_ipu_interrupt_init(void);
void * get_ipu_adapter(void);
int call_ipu_clock_start(void *clock, unsigned int clock_rate);
int call_ipu_clock_set_rate(void *clock, unsigned int clock_rate);
void call_ipu_clock_stop(void *clock);
int call_ipu_smmu_init(unsigned long ttbr0, unsigned long smmu_rw_err_phy_addr, bool port_sel);

#endif
