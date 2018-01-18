#include "psam_balong.h"
#include "n_bsp_psam.h"

extern struct psam_device *g_psam_device;

u64 psam64_get_ad0_base(void)
{
	return phy_addr_read((unsigned char*)g_psam_device->regs + HI_PSAM64_ADQ0_BASE_H_OFFSET,
				(unsigned char*)g_psam_device->regs + HI_PSAM64_ADQ0_BASE_L_OFFSET);
}

u64 psam64_get_ad1_base(void)
{
	return phy_addr_read((unsigned char*)g_psam_device->regs + HI_PSAM64_ADQ1_BASE_H_OFFSET,
				(unsigned char*)g_psam_device->regs + HI_PSAM64_ADQ1_BASE_L_OFFSET);
}

void psam64_set_ad_base(void)
{
	phy_addr_write(g_psam_device->dma[0],
		                (unsigned char*)g_psam_device->regs + HI_PSAM64_ADQ0_BASE_H_OFFSET,
			            (unsigned char*)g_psam_device->regs + HI_PSAM64_ADQ0_BASE_L_OFFSET);
	phy_addr_write(g_psam_device->dma[1],
		                (unsigned char*)g_psam_device->regs + HI_PSAM64_ADQ1_BASE_H_OFFSET,
			            (unsigned char*)g_psam_device->regs + HI_PSAM64_ADQ1_BASE_L_OFFSET);
}

int psam64_config_ad(IPF_AD_TYPE_E type, unsigned int n, IPF_AD_DESC_S * param)
{
	unsigned int i;
	unsigned int wptr;
	unsigned int offset;
	unsigned int size;
	ipf64_ad_s* ad;

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
		ad[wptr].output_ptr0.bits.ptr	= (unsigned long long)(unsigned long)param[i].OutPtr0;
		ad[wptr].output_ptr1.bits.ptr	= (unsigned long long)(unsigned long)param[i].OutPtr1;
		wptr = ((wptr + 1) < size)? (wptr + 1) : 0;
	}

	psam_writel(wptr, offset);
	(void)offset;
	return 0;
}

struct psam_hal_handle psam64_hal = {
	.get_ad0_base = psam64_get_ad0_base,
	.get_ad1_base = psam64_get_ad1_base,
	.set_ad_base = psam64_set_ad_base,
	.config_ad = psam64_config_ad,
};

