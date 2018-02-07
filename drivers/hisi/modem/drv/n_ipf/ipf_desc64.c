#include "ipf_balong.h"
#include <hi_ipf.h>
#include <bsp_ddr.h>
#include <mdrv_ipf_comm.h>
#include <osl_malloc.h>
#include <linux/dma-mapping.h>

extern struct ipf_ctx g_ipf_ctx;

void ipf64_cd_en_set(void* bd_base, unsigned int index, unsigned short en)
{
	ipf64_bd_s* bd = (ipf64_bd_s*)bd_base;
	ipf_desc_attr_t* attrp = (ipf_desc_attr_t*)&bd[index].attribute;
	attrp->bits.cd_en = en;
	return;
}

int ipf64_cd_en_get(void* bd_base, unsigned int index){
	ipf64_bd_s* bd = (ipf64_bd_s*)bd_base;
	ipf_desc_attr_t* attrp = (ipf_desc_attr_t*)&bd[index].attribute;
	return attrp->bits.cd_en;
}

void ipf64_cd_clear(void* cd_base, unsigned int index)
{
	ipf64_cd_s* cd = (ipf64_cd_s*)cd_base;
	memset(&cd[index], 0, sizeof(ipf64_cd_s));
}

void ipf64_bd_s2h(IPF_CONFIG_PARAM_S* param, void* bd_base, unsigned int index)
{
	ipf64_bd_s* bd = (ipf64_bd_s*)bd_base;
	bd[index].attribute.u16 = 0;
    bd[index].attribute.bits.int_en		= param->int_en;
    bd[index].attribute.bits.fc_head	= param->fc_head;
	bd[index].attribute.bits.mode		= param->mode;
    bd[index].input_ptr.addr= (unsigned long)param->Data;
    bd[index].pkt_len		= param->u16Len;
    bd[index].user_field1	= param->u16UsrField1;
    bd[index].user_field2	= param->u32UsrField2;
    bd[index].user_field3	= param->u32UsrField3;
}

void ipf64_bd_h2s(IPF_CONFIG_PARAM_S* param, void* bd_base, unsigned int index)
{
	ipf64_bd_s* bd = (ipf64_bd_s*)bd_base;
	
    param->int_en	=	bd[index].attribute.bits.int_en;
    param->fc_head  =   bd[index].attribute.bits.fc_head;
	param->mode  	=   bd[index].attribute.bits.mode;
    param->Data		=	(modem_phy_addr)(unsigned long)bd[index].input_ptr.addr;
    param->u16Len		=	bd[index].pkt_len;
    param->u16UsrField1	=	bd[index].user_field1;
    param->u32UsrField2	=	bd[index].user_field2;
    param->u32UsrField3	=	bd[index].user_field3;
}

void ipf64_rd_h2s(IPF_RD_DESC_S* param, void* rd_base, unsigned int index)
{
	ipf64_rd_s* rd = (ipf64_rd_s*)rd_base;
	param->u16Attribute	= rd[index].attribute.u16;
	param->u16PktLen	= rd[index].pkt_len;
	param->u16Result	= rd[index].result;
	param->InPtr		= (modem_phy_addr)(unsigned long)rd[index].input_ptr.addr;
#ifdef CONFIG_PSAM
	param->OutPtr	= (modem_phy_addr)(unsigned long)rd[index].virt.ptr;
#else
	param->OutPtr	= (modem_phy_addr)(unsigned long)rd[index].output_ptr;
#endif
	param->u16UsrField1	= rd[index].user_field1;
	param->u32UsrField2	= rd[index].user_field2;
	param->u32UsrField3	= rd[index].user_field3;
	rd[index].attribute.bits.dl_direct_set?g_ipf_ctx.stax.direct_bd++:g_ipf_ctx.stax.indirect_bd++;
}

int ipf64_ad_s2h(IPF_AD_TYPE_E type, unsigned int n, IPF_AD_DESC_S * param)
{
	unsigned int i;
	unsigned int wptr;
	unsigned int offset;
	unsigned int size;
	ipf64_ad_s* ad;

	ad = 	(IPF_AD_0==type)?
			(ipf64_ad_s*)g_ipf_ctx.dl_info.pstIpfADQ0:
			(ipf64_ad_s*)g_ipf_ctx.dl_info.pstIpfADQ1;

	offset = (IPF_AD_0==type)?
			 HI_IPF64_CH1_ADQ0_WPTR_OFFSET:
			 HI_IPF64_CH1_ADQ1_WPTR_OFFSET;

	size = (IPF_AD_0==type)?
			 IPF_DLAD0_DESC_SIZE:
			 IPF_DLAD1_DESC_SIZE;

	/*读出写指针*/
	wptr = ipf_readl(offset);
	for(i=0; i < n; i++)
	{
		if(0 == param->OutPtr1)
		{
			pr_err("%s the %d short ad outptr is null\n", __func__, i);
			g_ipf_ctx.status->ad_out_ptr_null[type]++;
			return BSP_ERR_IPF_INVALID_PARA;
		}
		ad[wptr].output_ptr0.addr	= (unsigned long)(param[i].OutPtr0);
		ad[wptr].output_ptr1.addr	= (unsigned long)(param[i].OutPtr1);
		wptr = ((wptr + 1) < size)? (wptr + 1) : 0;
	}
	g_ipf_ctx.status->cfg_ad_cnt[type] += n;
	/* 更新AD0写指针*/
	ipf_writel(wptr, offset);

	(void)offset;
	
	return 0;
}

void ipf64_ad_h2s(IPF_AD_DESC_S* param, void* ad_base, unsigned int index)
{
	ipf64_ad_s* ad = (ipf64_ad_s*)ad_base;
	param->OutPtr0	= (modem_phy_addr)(unsigned long)ad[index].output_ptr0.bits.ptr;
	param->OutPtr1	= (modem_phy_addr)(unsigned long)ad[index].output_ptr1.bits.ptr;
}

unsigned int ipf64_last_get(void* cd_base, unsigned int index)
{
    ipf64_cd_s* cd = (ipf64_cd_s*)cd_base;
    return cd[index].cd_last;
}

void ipf64_config(void)
{
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    g_ipf_ctx.dma_mask = 0xffffffffffffffffULL;
    
    g_ipf_ctx.ul_info.pstIpfBDQ     = dmam_alloc_coherent(
                g_ipf_ctx.dev, 
                sizeof(ipf_ubd_u),
                &g_ipf_ctx.ul_info.pstIpfPhyBDQ,
                GFP_KERNEL);
    phy_addr_write(g_ipf_ctx.ul_info.pstIpfPhyBDQ, 
	                (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH0_BDQ_BADDR_H_OFFSET,
		            (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH0_BDQ_BADDR_L_OFFSET);
                
//    g_ipf_ctx.ul_info.pstIpfRDQ     = &sm->urd;
//    g_ipf_ctx.ul_info.pstIpfADQ0    = &sm->uad0;
//    g_ipf_ctx.ul_info.pstIpfADQ1    = &sm->uad1;
    g_ipf_ctx.ul_info.pu32IdleBd    = &sm->idle;

//    g_ipf_ctx.dl_info.pstIpfBDQ     = &sm->dbd;

	g_ipf_ctx.dl_info.pstIpfRDQ     = kmalloc(
				sizeof(ipf_drd_u)+128,	
				GFP_KERNEL | __GFP_DMA);

    g_ipf_ctx.dl_info.pstIpfRDQ = (void *)(((unsigned long)g_ipf_ctx.dl_info.pstIpfRDQ +32) & (unsigned long)(~31));

    g_ipf_ctx.dl_info.pstIpfPhyRDQ  = virt_to_phys(g_ipf_ctx.dl_info.pstIpfRDQ);
    pr_err("phy pstIpfPhyRDQ  %p\n", (void *)g_ipf_ctx.dl_info.pstIpfPhyRDQ);

    phy_addr_write(g_ipf_ctx.dl_info.pstIpfPhyRDQ,
                    (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH1_RDQ_BADDR_H_OFFSET,
	                (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH1_RDQ_BADDR_L_OFFSET);
                
#ifndef CONFIG_PSAM
    g_ipf_ctx.dl_info.pstIpfADQ0    = dmam_alloc_coherent(
                g_ipf_ctx.dev,
                sizeof(ipf_dad_u),
                &g_ipf_ctx.dl_info.pstIpfPhyADQ0,
                GFP_KERNEL);

    phy_addr_write(g_ipf_ctx.dl_info.pstIpfPhyADQ0,
                    (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH1_ADQ0_BASE_H_OFFSET,
	                (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH1_ADQ0_BASE_L_OFFSET);
                
    g_ipf_ctx.dl_info.pstIpfADQ1    = dmam_alloc_coherent(
                g_ipf_ctx.dev,
                sizeof(ipf_dad_u),
                &g_ipf_ctx.dl_info.pstIpfPhyADQ1,
                GFP_KERNEL);
                
    phy_addr_write(g_ipf_ctx.dl_info.pstIpfPhyADQ1,
                    (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH1_ADQ1_BASE_H_OFFSET,
	                (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH1_ADQ1_BASE_L_OFFSET);

#endif            
    g_ipf_ctx.dl_info.pstIpfCDQ     = &sm->dcd;
    g_ipf_ctx.dl_info.u32IpfCdRptr = &sm->dcd_rptr;
    *(g_ipf_ctx.dl_info.u32IpfCdRptr) = 0;
	return;
}

unsigned int ipf64_get_ulbd_num(void)
{
	HI_IPF_CH0_BDQ_DEPTH_T dq_depth;
	
	/* 计算空闲BD数量 */
		dq_depth.u32 = ipf_readl(HI_IPF64_CH0_BDQ_DEPTH_OFFSET);
		return IPF_ULBD_DESC_SIZE - (dq_depth.bits.ul_bdq_depth);
}

unsigned int ipf64_get_ulrd_num(void)
{
    HI_IPF_CH0_RDQ_DEPTH_T dq_depth;

    dq_depth.u32 = ipf_readl(HI_IPF64_CH0_RDQ_DEPTH_OFFSET);
    return dq_depth.bits.ul_rdq_depth;
}

void ipf64_config_bd(unsigned int u32Num, IPF_CONFIG_ULPARAM_S* pstUlParam)
{
	HI_IPF_CH0_BDQ_WPTR_T bdq_wptr;
	unsigned int u32BD;
    unsigned int i;
	
	/* è¯»å‡ºBDå†™æŒ‡é’ˆ,å°†u32BdqWpträ½œä¸ºä¸´æ—¶å†™æŒ‡é’ˆä½¿ç”¨ */
    bdq_wptr.u32 = ipf_readl(HI_IPF64_CH0_BDQ_WPTR_OFFSET);
    u32BD = bdq_wptr.bits.ul_bdq_wptr;
    for(i = 0; i < u32Num; i++)
    {
		g_ipf_ctx.desc->bd_s2h(&pstUlParam[i], g_ipf_ctx.ul_info.pstIpfBDQ, u32BD);
		g_ipf_ctx.desc->cd_en_set(g_ipf_ctx.ul_info.pstIpfBDQ, u32BD, ipf_disable);

        u32BD = ((u32BD + 1) < IPF_ULBD_DESC_SIZE)? (u32BD + 1) : 0;
    }

	g_ipf_ctx.status->cfg_bd_cnt += u32Num;

    /* æ›´æ–°BDå†™æŒ‡é’ˆ*/
    ipf_writel(u32BD, HI_IPF64_CH0_BDQ_WPTR_OFFSET);
}

void ipf64_get_dlrd(unsigned int* pu32Num, IPF_RD_DESC_S *pstRd)
{
    unsigned int u32RdqRptr;
    unsigned int u32RdqDepth;
    unsigned int u32Num;
    unsigned int i;
    unsigned int u32CdqRptr;
    HI_IPF_CH1_RDQ_DEPTH_T dq_depth;
    void *pBuff = NULL;

    /* 读取RD深度 */
    dq_depth.u32 = ipf_readl(HI_IPF64_CH1_RDQ_DEPTH_OFFSET);
    u32RdqDepth = dq_depth.bits.dl_rdq_depth;

	g_ipf_ctx.status->get_rd_times++;
	
    u32Num = (u32RdqDepth < *pu32Num)?u32RdqDepth:*pu32Num;
    if(0 == u32Num)
    {
        *pu32Num = 0;
        return;
    }

    u32RdqRptr = ipf_readl(HI_IPF64_CH1_RDQ_RPTR_OFFSET);
    for(i = 0; i < u32Num; i++)
    {
            pBuff = (void *)dma_map_single(g_ipf_ctx.dev, (void *)((char *)g_ipf_ctx.dl_info.pstIpfRDQ + u32RdqRptr*sizeof(ipf64_rd_s)),
                                                                    sizeof(ipf64_rd_s), DMA_FROM_DEVICE);
		g_ipf_ctx.desc->rd_h2s(&pstRd[i], g_ipf_ctx.dl_info.pstIpfRDQ, u32RdqRptr);
        if(ipf_enable == g_ipf_ctx.desc->cd_en_get(g_ipf_ctx.dl_info.pstIpfRDQ, u32RdqRptr)){
            /* 更新CD读指针 */
        	u32CdqRptr = ((unsigned long)SHD_DDR_P2V((void *)pstRd[i].InPtr) - (unsigned 
        	long)g_ipf_ctx.dl_info.pstIpfCDQ)/(unsigned long)sizeof(ipf64_cd_s);//lint !e712

            while(g_ipf_ctx.desc->cd_last_get(g_ipf_ctx.dl_info.pstIpfCDQ, u32CdqRptr) != 1)
            {
                /* 将释放的CD  清0 */
				//g_ipf_ctx.desc->cd_clear(g_ipf_ctx.dl_info.pstIpfCDQ, u32CdqRptr);
                u32CdqRptr = ((u32CdqRptr+1) < IPF_DLCD_DESC_SIZE)?(u32CdqRptr+1):0;
            }
            //g_ipf_ctx.desc->cd_clear(g_ipf_ctx.dl_info.pstIpfCDQ, u32CdqRptr);
            u32CdqRptr = ((u32CdqRptr+1) < IPF_DLCD_DESC_SIZE)?(u32CdqRptr+1):0;
            *(g_ipf_ctx.dl_info.u32IpfCdRptr) = u32CdqRptr;
        }
		//ipf_pm_print_packet((void *)pstRd[i].OutPtr, (size_t)pstRd[i].u16PktLen);
        /* 更新RD读指针 */
        u32RdqRptr = ((u32RdqRptr+1) < IPF_DLRD_DESC_SIZE)?(u32RdqRptr+1):0;
		pstRd[i].u16PktLen > (g_ipf_ctx.status->ad_thred)? g_ipf_ctx.status->get_rd_cnt[IPF_AD_1]++:\
						   		  					   g_ipf_ctx.status->get_rd_cnt[IPF_AD_0]++;

        g_ipf_ctx.status->rd_len_update += pstRd[i].u16PktLen;
            dma_unmap_single(g_ipf_ctx.dev, (dma_addr_t)pBuff, sizeof(ipf64_rd_s), DMA_FROM_DEVICE);
    }

//	ipf_rd_rate(g_ipf_ctx.status->rate_en, IPF_CHANNEL_DOWN);
    ipf_writel(u32RdqRptr, HI_IPF64_CH1_RDQ_RPTR_OFFSET);
    *pu32Num = u32Num;
}

unsigned int ipf64_get_dlrd_num(void)
{
    HI_IPF_CH1_RDQ_DEPTH_T dq_depth;

    /* 读取RD深度 */
    dq_depth.u32 = ipf_readl(HI_IPF64_CH1_RDQ_DEPTH_OFFSET);
	g_ipf_ctx.status->get_rd_num_times++;
    return dq_depth.bits.dl_rdq_depth;
}

struct ipf_desc_handler_s ipf_desc64_handler = {
    .name = "ipf64_desc",
	.config = ipf64_config,
	.cd_en_set = ipf64_cd_en_set,
	.cd_en_get = ipf64_cd_en_get,
	.cd_clear = ipf64_cd_clear,
	.bd_s2h = ipf64_bd_s2h,
	.bd_h2s = ipf64_bd_h2s,
	.rd_h2s = ipf64_rd_h2s,
	.ad_s2h = ipf64_ad_s2h,
	.ad_h2s = ipf64_ad_h2s,
	.cd_last_get = ipf64_last_get,
	.get_bd_num = ipf64_get_ulbd_num,
	.get_ulrd_num = ipf64_get_ulrd_num,
	.config_bd = ipf64_config_bd,
	.get_rd = ipf64_get_dlrd,
	.get_dlrd_num = ipf64_get_dlrd_num,
};

EXPORT_SYMBOL(ipf_desc64_handler);

