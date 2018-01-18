/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <bsp_pm_om.h>
#include <linux/syscore_ops.h>
#include <bsp_ipc.h>
#include "osl_cache.h"
#include <bsp_ddr.h>
#include <n_bsp_psam.h>
#include <n_bsp_ipf.h>
#include <ipf_balong.h>
#include <bsp_reset.h>
#include <bsp_vic.h>

#define IPF_VIC_LEVEL		5
#define DELAY_WAIT_CIPHER   2
ipf_ctx_t g_ipf_ctx;
struct dev_pm_ops ipf_dev_pm_ops;
extern struct dev_pm_ops ipf32_dev_pm_ops;
extern struct dev_pm_ops ipf64_dev_pm_ops;
extern struct ipf_desc_handler_s ipf_desc_handler;
extern struct ipf_desc_handler_s ipf_desc64_handler;

static void ipf_get_version(void)
{
	unsigned int ver;
	ver = ipf_readl(HI_IPF_VERSION_OFFSET);
	printk(" ipf version is 0x%x\n", ver);
	switch(ver){
	case IPF_VER_120A:
		g_ipf_ctx.ipf_version = IPF_VERSION_120a;
		break;
	case IPF_VER_130A:
		g_ipf_ctx.ipf_version = IPF_VERSION_130a;
		break;
	case IPF_VER_130B:
		g_ipf_ctx.ipf_version = IPF_VERSION_130b;
		break;
	case IPF_VER_140A:
		g_ipf_ctx.ipf_version = IPF_VERSION_140a;
		break;
    case IPF_VER_150A:
        g_ipf_ctx.ipf_version = IPF_VERSION_150a;
        break;
    case IPF_VER_160:
        g_ipf_ctx.ipf_version = IPF_VERSION_160;
        break;
	default:
		g_ipf_ctx.ipf_version = IPF_VERSION_0000;
		break;
	}
}

static int ipf_get_limit_addr(void)
{
    struct memblock_type *type;
	phys_addr_t mem_start;
	phys_addr_t mem_end;
    unsigned int i;

	type = &memblock.memory;
	if(0 == type->cnt){
		dev_err(g_ipf_ctx.dev, "ipf limit addr not get\n");
		g_ipf_ctx.limit_addr->block_err = 1;
		return BSP_ERR_IPF_INVALID_PARA;
	}

    mem_start = type->regions[0].base;
	mem_end = type->regions[type->cnt-1].base + type->regions[type->cnt-1].size;

    *(g_ipf_ctx.memblock_show) = type->cnt;
    for(i=0; i < type->cnt; i++){
        *(g_ipf_ctx.memblock_show + i + 1) = (unsigned int)(type->regions[i].base);
        if(mem_start > type->regions[i].base){
            mem_start = type->regions[i].base;
        }
        
        if(mem_end < (type->regions[i].base + type->regions[i].size)){
            mem_end = type->regions[i].base + type->regions[i].size;
        }
    }

    if(mem_end > (phys_addr_t)0xffffffff){
        mem_end = (phys_addr_t)0xffffffff;
    }
    
	if((MODEM_START_ADDR >= mem_start) && ((MODEM_START_ADDR + MODEM_START_SIZE) <= mem_end)){
		g_ipf_ctx.limit_addr->block_sum 		=2;
		g_ipf_ctx.limit_addr->block1_start 		=(unsigned int)mem_start;
		g_ipf_ctx.limit_addr->block1_end		=MODEM_START_ADDR;
		g_ipf_ctx.limit_addr->block2_start 		=MODEM_START_ADDR + MODEM_START_SIZE;
		g_ipf_ctx.limit_addr->block2_end		=(unsigned int)mem_end;
	}
	else{
		g_ipf_ctx.limit_addr->block_sum 		=1;
		g_ipf_ctx.limit_addr->block1_start 		=(unsigned int)mem_start;
		g_ipf_ctx.limit_addr->block1_end		=(unsigned int)mem_end;
	}

	return 0;
}

static void ipf_dl_job_done_cb(void){
	if(g_ipf_ctx.dl_info.pFnDlIntCb)
		g_ipf_ctx.dl_info.pFnDlIntCb();
}

static void ipf_dl_ads_empty_cb(void){
    if(g_ipf_ctx.dl_info.pAdqEmptyDlCb)
        g_ipf_ctx.dl_info.pAdqEmptyDlCb(IPF_EMPTY_ADQ0);
}

static void ipf_dl_adl_empty_cb(void){
    if(g_ipf_ctx.dl_info.pAdqEmptyDlCb)
        g_ipf_ctx.dl_info.pAdqEmptyDlCb(IPF_EMPTY_ADQ1);
}

static void ipf_ul_bd_empty_cb(void){
    if(g_ipf_ctx.ul_info.handle_bd_empty)
        g_ipf_ctx.ul_info.handle_bd_empty();
}

static void ipf_mst_sec_rd_err_cb(void){
//	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    disable_irq_nosync(g_ipf_ctx.irq);
	IPF_PRINT("ipf sec rd err\n");
//	system_error(DRV_ERRNO_IPF_OUT_REG, 0, 0, (char*)&sm->uad0, sizeof(ipf_uad_u)+sizeof(ipf_uad_u));
}

static void ipf_mst_sec_wr_err_cb(void){
//	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    disable_irq_nosync(g_ipf_ctx.irq);
	IPF_PRINT("ipf sec wr err\n");
//	system_error(DRV_ERRNO_IPF_OUT_REG, 0, 0, (char*)&sm->uad0, sizeof(ipf_uad_u)+sizeof(ipf_uad_u));
}

struct int_handler ipf_int_table[32] = {
		{"ul_rpt_int1",                 0,  NULL},                  /* [0..0] 上行结果上报中断1 */
		{"ul_timeout_int1",             0,  NULL},                  /* [1..1] 上行结果上报超时中断1 */
		{"ul_disable_end_int1",         0,  NULL},                  /* [2..2] 上行通道关闭完成中断1 */
		{"ul_idle_cfg_bd_int1",         0,  NULL},                  /* [3..3] 上行通道IDLE期间软件配置BD指示中断1 */
		{"ul_trail_cfg_bd_int1",        0,  NULL},                  /* [4..4] 上行通道关闭但BD没有处理完期间软件继续配置BD指示中断1 */
		{"ul_noidle_clrptr_int1",       0,  NULL},                  /* [5..5] 上行非IDLE态清指针指示中断1 */
		{"ul_rdq_downoverflow_int1",    0,  NULL},                  /* [6..6] 上行RDQ下溢中断1 */
		{"ul_bdq_upoverflow_int1",      0,  NULL},                  /* [7..7] 上行BDQ上溢中断1 */
		{"ul_rdq_full_int1",            0,  NULL},                  /* [8..8] 上行RDQ满中断1 */
		{"ul_bdq_epty_int1",            0,  ipf_ul_bd_empty_cb},  	/* [9..9] 上行BDQ空中断1 */
		{"ul_adq0_epty_int1",           0,  NULL},                  /* [10..10] 上行ADQ0空中断1 */
		{"ul_adq1_epty_int1",           0,  NULL},                  /* [11..11] 上行ADQ1空中断1 */
		{"reserved_1",                  0,  NULL},                  /* [15..12] 保留 */
		{"reserved_1",                  0,  NULL},                  /* [15..12] 保留 */
		{"reserved_1",                  0,  NULL},                  /* [15..12] 保留 */
		{"reserved_1",                  0,  NULL},                  /* [15..12] 保留 */
		{"dl_rpt_int1",                 0,  ipf_dl_job_done_cb},	/* [16..16] 下行结果上报中断1 */
		{"dl_timeout_int1",             0,  ipf_dl_job_done_cb},	/* [17..17] 下行结果上报超时中断1 */
		{"dl_disable_end_int1",         0,  NULL},                  /* [18..18] 下行通道关闭完成中断1 */
		{"dl_idle_cfg_bd_int1",         0,  NULL},                  /* [19..19] 下行通道IDLE期间软件配置BD指示中断1 */
		{"dl_trail_cfg_bd_int1",        0,  NULL},                  /* [20..20] 下行通道关闭但BD没有处理完期间软件继续配置BD指示中断1 */
		{"dl_noidle_clrptr_int1",       0,  NULL},                  /* [21..21] 下行非IDLE态清指针指示中断1 */
		{"dl_rdq_downoverflow_int1",    0,  NULL},                  /* [22..22] 下行RDQ下溢中断1 */
		{"dl_bdq_upoverflow_int1",      0,  NULL},                  /* [23..23] 下行BDQ上溢中断1 */
		{"dl_rdq_full_int1",            0,  ipf_dl_job_done_cb},	/* [24..24] 下行RDQ满中断1 */
		{"dl_bdq_epty_int1",            0,  NULL},                  /* [25..25] 下行BDQ空中断1 */
		{"dl_adq0_epty_int1",           0,  ipf_dl_ads_empty_cb},	/* [26..26] 下行ADQ0空中断1 */
		{"dl_adq1_epty_int1",           0,  ipf_dl_adl_empty_cb},	/* [27..27] 下行ADQ1空中断1 */
		{"ipf_mst_sec_wr_err_int1",     0,  ipf_mst_sec_wr_err_cb},	/* [28..28] IPF MST写操作地址溢出中断1。 */
		{"ipf_mst_sec_rd_err_int1",     0,  ipf_mst_sec_rd_err_cb},	/* [29..29] IPF MST读操作地址溢出中断1。 */
		{"reserved_0",                  0,  NULL},                  /* [31..30] 保留。 */
		{"reserved_0",                  0,  NULL},                  /* [31..30] 保留。 */
};

static irqreturn_t ipf_interrupt(int irq, void* dev)
{
	int bit;
	unsigned int reg;

	reg = ipf_readl(HI_IPF_INT1_OFFSET);
	ipf_writel(reg, HI_IPF_INT_STATE_OFFSET);

	for_each_set_bit(bit, (unsigned long*)&reg, 32)
	{
		ipf_int_table[bit].cnt++;
		if(ipf_int_table[bit].callback){
		    ipf_int_table[bit].callback();
		}
	}
	
    return IRQ_HANDLED;
}

static void ipf_ctx_init(void)
{
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
#if 0
    g_ipf_ctx.ul_info.pstIpfBDQ = &sm->ubd;
    g_ipf_ctx.ul_info.pstIpfRDQ = &sm->urd;
    g_ipf_ctx.ul_info.pstIpfADQ0 = &sm->uad0;
    g_ipf_ctx.ul_info.pstIpfADQ1 = &sm->uad1;
    g_ipf_ctx.ul_info.pu32IdleBd = &sm->idle;

    g_ipf_ctx.dl_info.pstIpfBDQ = &sm->dbd;
    g_ipf_ctx.dl_info.pstIpfRDQ = &sm->drd;
//    g_ipf_ctx.dl_info.pstIpfADQ0 = &sm->dad0;
//    g_ipf_ctx.dl_info.pstIpfADQ1 = &sm->dad1;
    g_ipf_ctx.dl_info.pstIpfCDQ = &sm->dcd;

    g_ipf_ctx.dl_info.pstIpfPhyBDQ = SHD_DDR_V2P((void *)&sm->dbd);
    g_ipf_ctx.dl_info.pstIpfPhyRDQ = SHD_DDR_V2P((void *)&sm->drd);
//    g_ipf_ctx.dl_info.pstIpfPhyADQ0 = SHD_DDR_V2P((void *)&sm->dad0);
//	g_ipf_ctx.dl_info.pstIpfPhyADQ1 = SHD_DDR_V2P((void *)&sm->dad1);
    g_ipf_ctx.dl_info.u32IpfCdRptr = &sm->dcd_rptr;
    *(g_ipf_ctx.dl_info.u32IpfCdRptr) = 0;
#endif
	g_ipf_ctx.share_mem = sm;
	g_ipf_ctx.desc->config();
}

struct ipf_desc_handler_s* ipf_get_desc_handler(unsigned int version)
{
	if(version>IPF_VERSION_150a)
        return &ipf_desc64_handler;
    else
        return &ipf_desc_handler;
}

int ipf_init(void)
{
	int ret;
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

//	memset(&sm->uad0, 0, sizeof(sm->uad0));
//    memset(&sm->uad1, 0, sizeof(sm->uad1));
//    memset(&sm->ubd, 0, sizeof(sm->ubd));
//    memset(&sm->drd, 0, sizeof(sm->drd));
//    memset(&sm->uad0, 0, sizeof(sm->uad0));
    memset(&sm->debug[0], 0, sizeof(sm->debug[0]));
    
	ipf_get_version();
	g_ipf_ctx.desc = ipf_get_desc_handler(g_ipf_ctx.ipf_version);
	
   	ipf_ctx_init();

    #ifndef CONFIG_IPF_PROPERTY_MBB
    spin_lock_init(&g_ipf_ctx.filter_spinlock);
    #endif

	g_ipf_ctx.irq_hd = ipf_int_table;
#ifndef IPF_USE_VIC
	ret = request_irq(g_ipf_ctx.irq, ipf_interrupt, IRQF_SHARED, "ipf", g_ipf_ctx.dev);
#else
	ret = bsp_vic_connect(IPF_VIC_LEVEL, (vicfuncptr)ipf_interrupt, 0);
#endif
	if(0 != ret) {
		dev_err(g_ipf_ctx.dev, "request irq failed\n");
	}

    sm->init.status.modem = (unsigned int)IPF_FORRESET_CONTROL_ALLOW;

    if(0 != mdrv_sysboot_register_reset_notify("IPF_BALONG",bsp_ipf_reset_ccore_cb, 0, DRV_RESET_CB_PIOR_IPF))
    {
    	dev_err(g_ipf_ctx.dev, "set modem reset call back func failed\n");
    }

	/*acore use first block,ccore use scnd block*/
	g_ipf_ctx.status = (struct ipf_debug*)&sm->debug[0] ;

	g_ipf_ctx.desc = ipf_get_desc_handler(g_ipf_ctx.ipf_version);
    g_ipf_ctx.status->init_ok = IPF_ACORE_INIT_SUCCESS;
    g_ipf_ctx.modem_status = IPF_FORRESET_CONTROL_ALLOW;
	g_ipf_ctx.ccore_rst_idle = 0;

#ifdef CONFIG_PSAM
	g_ipf_ctx.psam_para.desc_hd = g_ipf_ctx.desc;
	bsp_psam_set_ipf_para(&g_ipf_ctx.psam_para);
#endif

	dev_err(g_ipf_ctx.dev, "ipf init success\n");

	return 0;
}

void bsp_ipf_set_debug_para(void ** psam_get_debug)
{
	*(psam_get_debug) = (void *)(g_ipf_ctx.status);
}

int bsp_ipf_srest(void)
{
	unsigned int reg;
	int tmp_cnt = 10000;
	
	/*reset*/
	UPDATE1(reg,	HI_IPF_SRST,
			ipf_srst,	1);
	while(!ipf_readl(HI_IPF_SRST_STATE_OFFSET)){
		tmp_cnt--;
		if(!tmp_cnt){
			return -1;
		}
	}

	ipf_writel(IPF_INT_OPEN0, HI_IPF_INT_MASK0_OFFSET);
    ipf_writel(IPF_INT_OPEN1, HI_IPF_INT_MASK1_OFFSET);
	return 0;
}

int bsp_ipf_config_timeout(unsigned int u32Timeout)
{
	unsigned int reg;

    if((u32Timeout == 0) || (u32Timeout > 0xFFFF))
    {
    	dev_err(g_ipf_ctx.dev, "%s para invalid\n", __func__);
        return BSP_ERR_IPF_INVALID_PARA;
    }

	UPDATE2(reg,	HI_IPF_TIME_OUT,
			time_out_cfg,	u32Timeout,
			time_out_valid,	1);
    return IPF_SUCCESS;
}

int bsp_ipf_set_pktlen(unsigned int u32MaxLen, unsigned int u32MinLen)
{
	unsigned int reg;

    /* 参数检查 */
    if(u32MaxLen < u32MinLen)
    {
    	dev_err(g_ipf_ctx.dev, "%s para invalid\n", __func__);
        return BSP_ERR_IPF_INVALID_PARA;
    }

	UPDATE2(reg,HI_IPF_PKT_LEN,
			min_pkt_len,	u32MinLen,
			max_pkt_len,	u32MaxLen);
    return IPF_SUCCESS;
}

void bsp_set_init_status(int stat)
{
    g_ipf_ctx.status->init_ok = stat;    
}

int bsp_get_init_status(void)
{
    return g_ipf_ctx.status->init_ok;    
}

void mdrv_ipf_reinit_dlreg(void)
{
//	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
	
    ipf_ctx_init();
	
	g_ipf_ctx.desc->config();
	
#ifdef CONFIG_PSAM
	psam_reinit_regs();
#endif

    g_ipf_ctx.status->init_ok = IPF_ACORE_INIT_SUCCESS;

    dev_err(g_ipf_ctx.dev, "ipf dl register reinit success\n");

    return;

}
#ifndef CONFIG_PSAM
int bsp_ipf_get_used_dlad(IPF_AD_TYPE_E eAdType, unsigned int * pu32AdNum, IPF_AD_DESC_S * pstAdDesc)
{
	return IPF_SUCCESS;

}
#endif
int mdrv_ipf_get_used_dlad(IPF_AD_TYPE_E eAdType, unsigned int * pu32AdNum, IPF_AD_DESC_S * pstAdDesc)
{	
#ifdef CONFIG_PSAM
	return bsp_psam_get_used_dlad(eAdType, pu32AdNum, pstAdDesc);
#else
	return bsp_ipf_get_used_dlad(eAdType, pu32AdNum, pstAdDesc);
#endif
}

void bsp_ipf_set_control_flag_for_ccore_reset(IPF_FORRESET_CONTROL_E eResetFlag)
{
#ifndef CONFIG_IPF_PROPERTY_MBB
        struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
#endif
    /*设置标志，终止上行数传*/
    g_ipf_ctx.modem_status = eResetFlag;
#ifndef CONFIG_IPF_PROPERTY_MBB
    /*设置标志，终止下行数传*/
    sm->init.status.modem = eResetFlag;
#endif
    cache_sync();
}

int bsp_ipf_reset_ccore_cb(DRV_RESET_CB_MOMENT_E eparam, int userdata)
{
	unsigned int idle_cnt = 10;
	unsigned int time_out = 2000;
	unsigned int u32DlStateValue = 0;
	int psam_status = 0;

	(void)userdata;
	
	if(MDRV_RESET_CB_BEFORE == eparam)
	{
		bsp_ipf_set_control_flag_for_ccore_reset(IPF_FORRESET_CONTROL_FORBID);

		do{
			udelay(DELAY_WAIT_CIPHER);	//wait cipher idle
			if(g_ipf_ctx.ipf_version <  IPF_VERSION_160){
				u32DlStateValue = ipf_readl(HI_IPF32_CH1_STATE_OFFSET);
			}
			else{
				u32DlStateValue = ipf_readl(HI_IPF64_CH1_STATE_OFFSET);
			}
			psam_status = bsp_psam_idle();
			time_out--;
			if((u32DlStateValue == IPF_CHANNEL_STATE_IDLE) && psam_status){
				idle_cnt--;
			}else{
				idle_cnt = 10;
			}
		}while(idle_cnt && time_out);

		if(!idle_cnt){
			g_ipf_ctx.ccore_rst_idle = 1;
		}

		if (!time_out)
		{
			bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,
				"\r IPF dl channel on after bsp_ipf_reset_ccore_cb called \n");
		}
	}
	else if(MDRV_RESET_CB_AFTER == eparam)
	{
		g_ipf_ctx.ccore_rst_idle = 0;
		bsp_ipf_set_control_flag_for_ccore_reset(IPF_FORRESET_CONTROL_ALLOW);
		return IPF_SUCCESS;
	}
	else
	{
		/*under the requeset of yaoguocai*/
		return IPF_SUCCESS;
	}

	return IPF_SUCCESS;
}

#ifndef CONFIG_PSAM
int ipf_config_dl_ad(unsigned int u32AdType, unsigned int  u32AdNum, IPF_AD_DESC_S * pstAdDesc)
{
	unsigned int u32ADQwptr = 0;
	struct tagIPF_AD_DESC_S * pstADDesc = pstAdDesc;
	IPF_DL_S* dl_ad = &g_ipf_ctx.dl_info;
	unsigned int i;

	g_ipf_ctx.status->cfg_ad_times++;
	if(NULL == pstAdDesc || u32AdType >= IPF_AD_MAX)
	{
		pr_err("%s para invalid\n", __func__);
		g_ipf_ctx.status->invalid_para++;
		return BSP_ERR_IPF_INVALID_PARA;
	}

	/* 检查模块是否初始化 */
	if(g_ipf_ctx.status && (IPF_ACORE_INIT_SUCCESS != g_ipf_ctx.status->init_ok))
	{
		g_ipf_ctx.status->mdrv_called_not_init++;
		pr_err("%s ipf not init\n", __func__);
		return BSP_ERR_IPF_NOT_INIT;
	}

	if(u32AdNum >= IPF_DLAD0_DESC_SIZE)
	{
		pr_err("%s too much short ad num\n", __func__);
		g_ipf_ctx.status->invalid_para++;
		return BSP_ERR_IPF_INVALID_PARA;
	}

	return g_ipf_ctx.desc->ad_s2h(u32AdType, u32AdNum, pstAdDesc);
}
#endif
int ipf_register_wakeup_dlcb(BSP_IPF_WakeupDlCb pFnWakeupDl)
{
    /* 参数检查 */
    if(NULL == pFnWakeupDl)
    {
    	g_ipf_ctx.status->invalid_para++;
    	pr_err("%s invalid para\n", __func__);
        return BSP_ERR_IPF_INVALID_PARA;
    }
    g_ipf_ctx.dl_info.pFnDlIntCb = pFnWakeupDl;
    return IPF_SUCCESS;
}

int ipf_register_ul_bd_empty(ipf_bd_empty bd_handle)
{
   if(NULL == bd_handle)
    {
    	g_ipf_ctx.status->invalid_para++;
    	pr_err("%s invalid para\n", __func__);
        return BSP_ERR_IPF_INVALID_PARA;
    }
    g_ipf_ctx.ul_info.handle_bd_empty = bd_handle;
	return 0;
}
#ifndef CONFIG_PSAM
int ipf_register_adq_empty_dlcb(BSP_IPF_AdqEmptyDlCb pAdqEmptyDl)
{
    /* 参数检查 */
    if(NULL == pAdqEmptyDl)
    {
    	g_ipf_ctx.status->invalid_para++;
    	pr_err("%s invalid para\n", __func__);
        return IPF_ERROR;
    }
    g_ipf_ctx.dl_info.pAdqEmptyDlCb = pAdqEmptyDl;
    return IPF_SUCCESS;
}
#endif

unsigned int mdrv_ipf_get_ulbd_num(void)
{
    unsigned int u32IdleBd;

	g_ipf_ctx.status->get_bd_num_times++;

    u32IdleBd = g_ipf_ctx.desc->get_bd_num();
    *(g_ipf_ctx.ul_info.pu32IdleBd) = u32IdleBd;

    if(0 == u32IdleBd)
	{
		g_ipf_ctx.status->bd_full++;
	}
    return u32IdleBd;
}

unsigned int mdrv_ipf_get_ulrd_num(void)
{
    return g_ipf_ctx.desc->get_ulrd_num();
}

unsigned int mdrv_ipf_get_uldesc_num(void)
{
	return mdrv_ipf_get_ulbd_num();
}

int mdrv_ipf_config_ulbd(unsigned int u32Num, IPF_CONFIG_ULPARAM_S* pstUlPara)
{
	unsigned int i;
	
	g_ipf_ctx.status->cfg_bd_times++;

    /* 参数检查 */
    if((NULL == pstUlPara)||(0 == u32Num))
    {
    	g_ipf_ctx.status->invalid_para++;
    	pr_err("%s para invalid\n", __func__);
        return BSP_ERR_IPF_INVALID_PARA;
    }

    /* 检查模块是否初始化 */
    if(g_ipf_ctx.status && (IPF_ACORE_INIT_SUCCESS != g_ipf_ctx.status->init_ok))
    {
		g_ipf_ctx.status->mdrv_called_not_init++;
		pr_err("%s ipf not init\n", __func__);
        return BSP_ERR_IPF_NOT_INIT;
    }

    /* 检查Ccore是否上电*/
    if(IPF_FORRESET_CONTROL_FORBID <= g_ipf_ctx.modem_status)
    {
       	g_ipf_ctx.status->mdrv_called_not_init++;
        return BSP_ERR_IPF_CCORE_RESETTING;
    }

    for(i = 0; i < u32Num; i++)
    {
        if(0 == pstUlPara[i].u16Len)
        {
        	pr_err("%s the %d bdlen is zero drop packet\n", __func__, i);
			g_ipf_ctx.status->bd_len_zero_err++;
            return BSP_ERR_IPF_INVALID_PARA;
        }
    }

	/* æ£€æŸ¥Ccoreæ˜¯å¦ä¸Šç”µ*/
    if(IPF_FORRESET_CONTROL_FORBID <= g_ipf_ctx.modem_status)
    {
    	g_ipf_ctx.status->mdrv_called_not_init++;
        return BSP_ERR_IPF_CCORE_RESETTING;
    }

    g_ipf_ctx.desc->config_bd(u32Num, pstUlPara);

    return IPF_SUCCESS;
}
int mdrv_ipf_config_dlad(IPF_AD_TYPE_E eAdType, unsigned int u32AdNum, IPF_AD_DESC_S * pstAdDesc)
{
#ifdef CONFIG_PSAM
	return psam_config_dlad(eAdType, u32AdNum, pstAdDesc);
#else
	return ipf_config_dl_ad(eAdType, u32AdNum, pstAdDesc);
#endif
}

int mdrv_ipf_register_ops(struct mdrv_ipf_ops *ops)
{
	if(ops)
	{
		if(ipf_register_wakeup_dlcb(ops->rx_complete_cb)!= IPF_SUCCESS)
			return IPF_ERROR;
#ifdef CONFIG_PSAM
		if(psam_register_adq_empty_dlcb((adq_emtpy_cb_t)ops->adq_empty_cb)!= IPF_SUCCESS)
			return IPF_ERROR;
#else
		if(ipf_register_adq_empty_dlcb((BSP_IPF_AdqEmptyDlCb)ops->adq_empty_cb)!= IPF_SUCCESS)
			return IPF_ERROR;
#endif
	} else {
		pr_err("%s para invalid\n", __func__);
		g_ipf_ctx.status->invalid_para++;
		return IPF_ERROR;
	}
	return IPF_SUCCESS;
}

void mdrv_ipf_get_dlrd(unsigned int* pu32Num, IPF_RD_DESC_S *pstRd)
{
    g_ipf_ctx.desc->get_rd(pu32Num, pstRd);
}

int mdrv_ipf_get_dlad_num (unsigned int* pu32AD0Num, unsigned int* pu32AD1Num)
{
	g_ipf_ctx.status->get_ad_num_times++;
#ifdef CONFIG_PSAM
	return psam_get_dlad_num(pu32AD0Num, pu32AD1Num);
#else
	return ipf_get_dl_ad_num(pu32AD0Num, pu32AD1Num);
#endif
}

unsigned int mdrv_ipf_get_dlrd_num(void)
{
    return g_ipf_ctx.desc->get_dlrd_num();
}

static int ipf_probe(struct platform_device *pdev)
{
	struct resource	*regs;
	int ret;
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs)
		return -ENXIO;

    memset(&g_ipf_ctx, 0, sizeof(ipf_ctx_t));
	g_ipf_ctx.irq = platform_get_irq(pdev, 0);
	if (unlikely(g_ipf_ctx.irq == 0))
		return -ENXIO;

	g_ipf_ctx.regs = devm_ioremap_resource(&pdev->dev, regs);
	if (IS_ERR(g_ipf_ctx.regs))
		return PTR_ERR(g_ipf_ctx.regs);
	

	g_ipf_ctx.dev = &pdev->dev;
	g_ipf_ctx.dma_mask = 0xffffffffULL;
	g_ipf_ctx.dev->dma_mask = &g_ipf_ctx.dma_mask;

	g_ipf_ctx.clk = devm_clk_get(g_ipf_ctx.dev, "ipf_clk");
	if (IS_ERR(g_ipf_ctx.clk)) {
		dev_err(g_ipf_ctx.dev, "ipf clock not available\n");
		return -ENXIO;
	} else {
		ret = clk_prepare_enable(g_ipf_ctx.clk);
		if (ret) {
			dev_err(g_ipf_ctx.dev, "failed to enable ipf clock\n");
			return ret;
		}
	}

	g_ipf_ctx.limit_addr = (IPF_LIMIT_ADDR_S *)sm->trans_limit;
    g_ipf_ctx.memblock_show = (unsigned long *)sm->memlock;
	memset((void*)sm->trans_limit, 0x0, IPF_TRANS_LIMIT_SIZE);
    memset((void*)sm->memlock, 0x0, IPF_ADDR_MEMBLOCK_SIZE);

	if(ipf_get_limit_addr()){
		g_ipf_ctx.not_get_space++;
		dev_info(g_ipf_ctx.dev, "ipf addr limit func disable\n");
	}

	ipf_init();

	g_ipf_ctx.pm = (g_ipf_ctx.ipf_version < IPF_VERSION_160)?&ipf32_dev_pm_ops:&ipf64_dev_pm_ops;

	ipf_dev_pm_ops.prepare = g_ipf_ctx.pm->prepare;
	ipf_dev_pm_ops.suspend_noirq = g_ipf_ctx.pm->suspend_noirq;
	ipf_dev_pm_ops.resume_noirq = g_ipf_ctx.pm->resume_noirq;

	return 0;

}

static int ipf_remove(struct platform_device *pdev)
{
    return IPF_SUCCESS;
}

static const struct of_device_id ipf_match[] = {
	{ .compatible = "hisilicon,ipf_balong_app" },
	{},
};

static struct platform_driver ipf_pltfm_driver = {
	.probe		= ipf_probe,
	.remove		= ipf_remove,
	.driver		= {
		.name		= "ipf",
		.of_match_table	= ipf_match,
		.pm = &ipf_dev_pm_ops,
	},
};

EXPORT_SYMBOL(g_ipf_ctx);
EXPORT_SYMBOL(mdrv_ipf_config_dlad);
EXPORT_SYMBOL(mdrv_ipf_get_used_dlad);
EXPORT_SYMBOL(bsp_ipf_config_timeout);
EXPORT_SYMBOL(bsp_ipf_set_control_flag_for_ccore_reset);
EXPORT_SYMBOL(bsp_ipf_set_pktlen);
EXPORT_SYMBOL(mdrv_ipf_reinit_dlreg);
EXPORT_SYMBOL(mdrv_ipf_get_uldesc_num);
EXPORT_SYMBOL(mdrv_ipf_config_ulbd);
EXPORT_SYMBOL(mdrv_ipf_get_dlad_num);
EXPORT_SYMBOL(mdrv_ipf_get_ulbd_num);
EXPORT_SYMBOL(bsp_ipf_srest);
EXPORT_SYMBOL(ipf_init);
EXPORT_SYMBOL(mdrv_ipf_get_dlrd);
EXPORT_SYMBOL(mdrv_ipf_register_ops);
EXPORT_SYMBOL(ipf_register_wakeup_dlcb);


module_platform_driver(ipf_pltfm_driver);//lint !e64
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:ipf");

#ifdef __cplusplus
}
#endif

/*lint -restore*/

