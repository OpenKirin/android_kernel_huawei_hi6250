#include "psam_balong.h"
#include "n_bsp_psam.h"

extern struct psam_device *g_psam_device;

u64 psam32_get_ad0_base(void)
{
	return MDDR_FAMA_FROM(psam_readl(HI_PSAM32_ADQ0_BASE_OFFSET));//lint !e571
}

u64 psam32_get_ad1_base(void)
{
	return MDDR_FAMA_FROM(psam_readl(HI_PSAM32_ADQ1_BASE_OFFSET));//lint !e571
}

void psam32_set_ad_base(void)
{
	psam_writel(g_psam_device->dma[0], HI_PSAM32_ADQ0_BASE_OFFSET);
	psam_writel(g_psam_device->dma[1], HI_PSAM32_ADQ1_BASE_OFFSET);
}

int psam32_config_ad(IPF_AD_TYPE_E type, unsigned int n, IPF_AD_DESC_S * param)
{
	unsigned int i;
	unsigned int wptr;
	unsigned int offset;
	unsigned int size;
	ipf_ad_s* ad;

	ad = 	g_psam_device->desc[type];
			
	offset = (IPF_AD_0==type)?
			 HI_PSAM_ADQ0_WPTR_OFFSET:
			 HI_PSAM_ADQ1_WPTR_OFFSET;

	size = (IPF_AD_0==type)?
			 IPF_DLAD0_DESC_SIZE:
			 IPF_DLAD1_DESC_SIZE;

	wptr = psam_readl(offset);
	for(i=0; i < n; i++)
	{
		if(0 == param->OutPtr1)
		{
			return -1;
		}
		ad[wptr].u32OutPtr0	= ADDR_TO_U32(param[i].OutPtr0);
		ad[wptr].u32OutPtr1	= ADDR_TO_U32(param[i].OutPtr1);
		wptr = ((wptr + 1) < size)? (wptr + 1) : 0;
	}
	psam_writel(wptr, offset);

	(void)offset;
	return 0;
}


struct psam_hal_handle psam32_hal = {
	.get_ad0_base = psam32_get_ad0_base,
	.get_ad1_base = psam32_get_ad1_base,
	.set_ad_base = psam32_set_ad_base,
	.config_ad = psam32_config_ad,
};

