#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/syscore_ops.h>
#include <bsp_ipc.h>
#include <n_bsp_ipf.h>
#include <n_bsp_psam.h>
#include <bsp_slice.h>
#include <bsp_pm_om.h>
#include "ipf_balong.h"

extern ipf_ctx_t g_ipf_ctx;

const unsigned int ipf32_save_table[][2] =
{
    {HI_IPF_CTRL_OFFSET, 0xffffffff},
    {HI_IPF_GATE_OFFSET, 0xffffffff},
    {HI_IPF_DMA_CTRL1_OFFSET, 0xffffffff},
    {HI_IPF32_EF_BADDR_OFFSET, 0xffffffff},
    {HI_IPF32_CH0_CTRL_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_CTRL_OFFSET, 0xffffffff},		
    {HI_IPF32_CH0_BDQ_SIZE_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_BDQ_SIZE_OFFSET, 0xffffffff},
    {HI_IPF32_CH0_RDQ_SIZE_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_RDQ_SIZE_OFFSET, 0xffffffff},
    {HI_IPF32_CH0_BDQ_BADDR_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_BDQ_BADDR_OFFSET, 0xffffffff},
    {HI_IPF32_CH0_RDQ_BADDR_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_RDQ_BADDR_OFFSET, 0xffffffff},
    {HI_IPF32_CH0_ADQ0_BASE_OFFSET, 0xffffffff}, 
    {HI_IPF32_CH0_ADQ1_BASE_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_ADQ0_BASE_OFFSET, 0xffffffff},  
    {HI_IPF32_CH1_ADQ1_BASE_OFFSET, 0xffffffff}, 
    {HI_IPF_TIME_OUT_OFFSET, 0xffffffff},
    {HI_IPF_PKT_LEN_OFFSET, 0xffffffff},
	{HI_IPF32_CH0_ADQ_CTRL_OFFSET,			0xfffffff0 },
    {HI_IPF32_CH1_ADQ_CTRL_OFFSET,			0xfffffff0 },
    {HI_IPF32_CH0_ADQ0_RPTR_OFFSET, 0xffffffff}, 
    {HI_IPF32_CH0_ADQ1_RPTR_OFFSET, 0xffffffff}, 
    {HI_IPF32_CH1_ADQ0_RPTR_OFFSET, 0xffffffff}, 
    {HI_IPF32_CH1_ADQ1_RPTR_OFFSET, 0xffffffff}, 
    {HI_IPF32_CH0_ADQ0_WPTR_OFFSET, 0xffffffff}, 
    {HI_IPF32_CH0_ADQ1_WPTR_OFFSET, 0xffffffff}, 
    {HI_IPF32_CH1_ADQ0_WPTR_OFFSET, 0xffffffff}, 
    {HI_IPF32_CH1_ADQ1_WPTR_OFFSET, 0xffffffff}, 
    {HI_IPF32_CH0_ADQ_CTRL_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_ADQ_CTRL_OFFSET, 0xffffffff},
    {HI_IPF_INT_MASK0_OFFSET, 0xffffffff},
    {HI_IPF_INT_MASK1_OFFSET, 0xffffffff},
    {HI_IPF32_CH0_WDATA_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH0_RDATA_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH0_RD_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH0_BD_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH0_AD_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH0_CD_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH1_WDATA_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH1_RDATA_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH1_RD_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH1_BD_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH1_AD_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH1_CD_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_EF_FAMA_ATTR_OFFSET, 0xffffffff},
	{HI_IPF32_CH1_DIRECT_RDATA_FAMA_ATTR_OFFSET, 0xffffffff},
    {HI_IPF32_MST_REGION0_START_OFFSET, 0xffffffff},
	{HI_IPF32_MST_REGION0_END_OFFSET, 0xffffffff},
	{HI_IPF32_MST_REGION1_START_OFFSET, 0xffffffff},
	{HI_IPF32_MST_REGION1_END_OFFSET, 0xffffffff},
	{HI_IPF32_MST_REGION2_START_OFFSET, 0xffffffff},
	{HI_IPF32_MST_REGION2_END_OFFSET, 0xffffffff},
	{HI_IPF32_MST_REGION3_START_OFFSET, 0xffffffff},
	{HI_IPF32_MST_REGION3_END_OFFSET, 0xffffffff},
	{HI_IPF32_MST_DEFAULT_ADDR_OFFSET, 0xffffffff},
	{HI_IPF32_MST_SECCTRL_OFFSET, 0xffffffff},	
	{HI_IPF_CH_EN_OFFSET, 0xffffffff},
};

const unsigned int ipf64_save_table[][2] =
{
    {HI_IPF_CTRL_OFFSET, 0xffffffff},
    {HI_IPF_GATE_OFFSET, 0xffffffff},
    {HI_IPF_DMA_CTRL1_OFFSET, 0xffffffff},
    {HI_IPF_EF_BADDR_L_OFFSET, 0xffffffff},
    {HI_IPF_EF_BADDR_H_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_CTRL_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_CTRL_OFFSET, 0xffffffff},		
    {HI_IPF64_CH0_BDQ_SIZE_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_BDQ_SIZE_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_RDQ_SIZE_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_RDQ_SIZE_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_BDQ_BADDR_L_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_BDQ_BADDR_H_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_BDQ_BADDR_L_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_BDQ_BADDR_H_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_RDQ_BADDR_L_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_RDQ_BADDR_H_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_RDQ_BADDR_L_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_RDQ_BADDR_H_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_ADQ0_BASE_L_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_ADQ0_BASE_H_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_ADQ1_BASE_L_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_ADQ1_BASE_H_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_ADQ0_BASE_L_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_ADQ0_BASE_H_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_ADQ1_BASE_L_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_ADQ1_BASE_H_OFFSET, 0xffffffff},
    {HI_IPF_TIME_OUT_OFFSET, 0xffffffff},
    {HI_IPF_PKT_LEN_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_ADQ0_RPTR_OFFSET, 0xffffffff}, 
    {HI_IPF64_CH0_ADQ1_RPTR_OFFSET, 0xffffffff}, 
    {HI_IPF64_CH1_ADQ0_RPTR_OFFSET, 0xffffffff}, 
    {HI_IPF64_CH1_ADQ1_RPTR_OFFSET, 0xffffffff}, 
    {HI_IPF64_CH0_ADQ0_WPTR_OFFSET, 0xffffffff}, 
    {HI_IPF64_CH0_ADQ1_WPTR_OFFSET, 0xffffffff}, 
    {HI_IPF64_CH1_ADQ0_WPTR_OFFSET, 0xffffffff}, 
    {HI_IPF64_CH1_ADQ1_WPTR_OFFSET, 0xffffffff}, 
    {HI_IPF64_CH0_ADQ_CTRL_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_ADQ_CTRL_OFFSET, 0xffffffff},
    {HI_IPF_INT_MASK0_OFFSET, 0xffffffff},
    {HI_IPF_INT_MASK1_OFFSET, 0xffffffff},		
    {HI_IPF64_SEC_ATTR_OFFSET, 0xffffffff},
    {HI_IPF64_BURST_CFG_OFFSET, 0xffffffff},    
	{HI_IPF_CH_EN_OFFSET, 0xffffffff},
};

#if 1

static int _ipf32_reg_save(void)
{
    unsigned int i = 0;

    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    
    for(i=0; i<sizeof(ipf32_save_table)/sizeof(ipf32_save_table[0]); i++)
    {
       sm->reg_save[i] = ipf_readl(ipf32_save_table[i][0]);
    }
    
    return IPF_SUCCESS;
}

static int _ipf64_reg_save(void)
{
    unsigned int i = 0;

    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    
    for(i=0; i<sizeof(ipf64_save_table)/sizeof(ipf64_save_table[0]); i++)
    {
       sm->reg_save[i] = ipf_readl(ipf64_save_table[i][0]);
    }
    
    return IPF_SUCCESS;
}

static int _ipf32_reg_load(void)
{
    unsigned int i=0;
    
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    for(i=0; i<sizeof(ipf32_save_table)/sizeof(ipf32_save_table[0]); i++)
    {
       ipf_writel((sm->reg_save[i]&ipf32_save_table[i][1]), ipf32_save_table[i][0]);
    }

    return 0;
}

static int _ipf64_reg_load(void)
{
    unsigned int i=0;
    
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    for(i=0; i<sizeof(ipf64_save_table)/sizeof(ipf64_save_table[0]); i++)
    {
       ipf_writel((sm->reg_save[i]&ipf32_save_table[i][1]),ipf64_save_table[i][0]);
    }

    return 0;
}

/*
static int _ipf_filter_load(void)
{
    unsigned int i;
	filter_map* map;
    struct ipf_filter_handler* fh = g_ipf_ctx.filter_handler;
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    

    for(i=0;i<IPF_MODEM_MAX;i++)
    {
        map = fh->launched[i];
        while(map)
        {
            if(map->index>=fh->bf_num)
                break;
            fh->basic_write(map->index, (ipf_filter_node_s*)((unsigned char*)sm->filter+i*0x80));
            map = map->next;
        }
    }

	(void)sm;

    return 0;

}
*/
static int ipf32_pm_prepare_ex(struct device *pdev)
{
    HI_IPF_CH1_STATE_T state;

    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    /* check whether the downstream channel is idle*/
    state.u32 = ipf_readl(HI_IPF32_CH1_STATE_OFFSET);

    if(state.bits.dl_busy)
    {
		g_ipf_ctx.ipf_pm_busy++;
        return IPF_ERROR;
    }

	sm->init.status.acore = IPF_PWR_PREPARED;

    return IPF_SUCCESS;
}

static int ipf64_pm_prepare_ex(struct device *pdev)
{
    HI_IPF_CH1_STATE_T state;

    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    /* check whether the downstream channel is idle*/
    state.u32 = ipf_readl(HI_IPF64_CH1_STATE_OFFSET);

    if(state.bits.dl_busy)
    {
		g_ipf_ctx.ipf_pm_busy++;
        return IPF_ERROR;
    }
	
	sm->init.status.acore = IPF_PWR_PREPARED;

	(void)pdev;

    return IPF_SUCCESS;
}


static int ipf32_pm_suspend_ex(struct device *pdev)
{
    unsigned long ipf_flags = 0;
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
	
    spin_lock_irqsave(&g_ipf_ctx.filter_spinlock, ipf_flags);
    bsp_ipc_spin_lock(IPC_SEM_IPF_PWCTRL);
    if(sm->init.status.ccore==IPF_PWR_DOWN){   
        _ipf32_reg_save();
		sm->init.status.save = 1;
    }
    
    sm->init.status.acore=IPF_PWR_DOWN;
    cache_sync();
    bsp_ipc_spin_unlock(IPC_SEM_IPF_PWCTRL);
    spin_unlock_irqrestore(&g_ipf_ctx.filter_spinlock, ipf_flags);

    g_ipf_ctx.status->suspend++;

	(void)pdev;
	return IPF_SUCCESS;
}

static int ipf64_pm_suspend_ex(struct device *pdev)
{
    unsigned long ipf_flags = 0;
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
	
    spin_lock_irqsave(&g_ipf_ctx.filter_spinlock, ipf_flags);
    bsp_ipc_spin_lock(IPC_SEM_IPF_PWCTRL);
    if(sm->init.status.ccore==IPF_PWR_DOWN){   
        _ipf64_reg_save();
		sm->init.status.save = 1;
    }

    sm->init.status.acore=IPF_PWR_DOWN;
    cache_sync();
    bsp_ipc_spin_unlock(IPC_SEM_IPF_PWCTRL);
    spin_unlock_irqrestore(&g_ipf_ctx.filter_spinlock, ipf_flags);

    g_ipf_ctx.status->suspend++;
	(void)pdev;
	return IPF_SUCCESS;
}


static int ipf32_pm_resume_ex(struct device *pdev)
{
    unsigned int status;
	unsigned long ipf_flags = 0;
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
	
    spin_lock_irqsave(&g_ipf_ctx.filter_spinlock, ipf_flags);
    bsp_ipc_spin_lock(IPC_SEM_IPF_PWCTRL);
    if(sm->init.status.save){
        sm->init.status.save=0;
        _ipf32_reg_load();
    }
    sm->init.status.acore=0;
    cache_sync();
    bsp_ipc_spin_unlock(IPC_SEM_IPF_PWCTRL);
    spin_unlock_irqrestore(&g_ipf_ctx.filter_spinlock, ipf_flags);
    
	status = ipf_readl(HI_IPF_INT0_OFFSET); 
	if (status & (IPF_UL_RPT_INT0 \
					| IPF_UL_TIMEOUT_INT0 \
					| IPF_UL_ADQ0_EPTY_INT0 \
					| IPF_UL_ADQ1_EPTY_INT0)) {
		g_ipf_ctx.status->resume_with_intr++;
	}

    g_ipf_ctx.status->resume++;
	(void)pdev;
	return IPF_SUCCESS;
}

static int ipf64_pm_resume_ex(struct device *pdev)
{
    unsigned int status;
	unsigned long ipf_flags = 0;
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
	
    spin_lock_irqsave(&g_ipf_ctx.filter_spinlock, ipf_flags);
    bsp_ipc_spin_lock(IPC_SEM_IPF_PWCTRL);
    if(sm->init.status.save){
        sm->init.status.save=0;
        _ipf64_reg_load();
    }
    sm->init.status.acore=0;
    cache_sync();
    bsp_ipc_spin_unlock(IPC_SEM_IPF_PWCTRL);
    spin_unlock_irqrestore(&g_ipf_ctx.filter_spinlock, ipf_flags);
    
	status = ipf_readl(HI_IPF_INT0_OFFSET); 
	if (status & (IPF_UL_RPT_INT0 \
					| IPF_UL_TIMEOUT_INT0 \
					| IPF_UL_ADQ0_EPTY_INT0 \
					| IPF_UL_ADQ1_EPTY_INT0)) {
		g_ipf_ctx.status->resume_with_intr++;
	}

    g_ipf_ctx.status->resume++;
	(void)pdev;
	return IPF_SUCCESS;
}


struct dev_pm_ops ipf32_dev_pm_ops={

    .prepare = ipf32_pm_prepare_ex,
    .suspend_noirq = ipf32_pm_suspend_ex,
    .resume_noirq = ipf32_pm_resume_ex,

};

struct dev_pm_ops ipf64_dev_pm_ops={

    .prepare = ipf64_pm_prepare_ex,
    .suspend_noirq = ipf64_pm_suspend_ex,
    .resume_noirq = ipf64_pm_resume_ex,
};

#endif


