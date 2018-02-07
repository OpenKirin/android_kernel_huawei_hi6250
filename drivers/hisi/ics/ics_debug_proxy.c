#include "ipu_clock.h"
#include "ipu_smmu_drv.h"
#include "ics_debug_proxy.h"

extern struct cambricon_ipu_private *adapter;
extern int regulator_ip_vipu_enable(void);
extern int regulator_ip_vipu_disable(void);
extern int ipu_reset_proc(unsigned int addr);
extern void ipu_interrupt_init(void);

int call_regulator_ip_vipu_enable(void)
{
	return regulator_ip_vipu_enable();
}
EXPORT_SYMBOL(call_regulator_ip_vipu_enable);

int call_regulator_ip_vipu_disable(void)
{
	return regulator_ip_vipu_disable();
}
EXPORT_SYMBOL(call_regulator_ip_vipu_disable);

int call_ipu_reset_proc(unsigned int addr)
{
	return ipu_reset_proc(addr);
}
EXPORT_SYMBOL(call_ipu_reset_proc);

void call_ipu_interrupt_init(void)
{
	ipu_interrupt_init();
}
EXPORT_SYMBOL(call_ipu_interrupt_init);

void * get_ipu_adapter(void)
{
	return adapter;
}
EXPORT_SYMBOL(get_ipu_adapter);

int call_ipu_clock_start(void *clock, unsigned int clock_rate)
{
	return ipu_clock_start(clock, clock_rate);
}
EXPORT_SYMBOL(call_ipu_clock_start);

int call_ipu_clock_set_rate(void *clock, unsigned int clock_rate)
{
	return ipu_clock_set_rate(clock, clock_rate);
}
EXPORT_SYMBOL(call_ipu_clock_set_rate);

void call_ipu_clock_stop(void *clock)
{
	ipu_clock_stop(clock);
}
EXPORT_SYMBOL(call_ipu_clock_stop);

int call_ipu_smmu_init(unsigned long ttbr0, unsigned long smmu_rw_err_phy_addr, bool port_sel)
{
	return ipu_smmu_init(ttbr0, smmu_rw_err_phy_addr, port_sel);
}
EXPORT_SYMBOL(call_ipu_smmu_init);

