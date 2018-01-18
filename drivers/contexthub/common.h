#ifndef __LINUX_CONTEXTHUB_COMMON_H__
#define __LINUX_CONTEXTHUB_COMMON_H__

#include <linux/of.h>
#include <linux/of_address.h>

#define HM_EN(n)		(0x10001 << (n))
#define HM_DIS(n)		(0x10000 << (n))
#define writel_mask(mask, data, addr)	/*lint -save -e717 */do {writel((((u32)readl(addr)) & (~((u32)(mask)))) | ((data) & (mask)), (addr));} while (0)/*lint -restore */

static inline unsigned int is_bits_clr(unsigned int  mask, volatile void __iomem *addr)
{
    return (((*(volatile unsigned int *) (addr)) & (mask)) == 0);
}

static inline unsigned int is_bits_set(unsigned int  mask, volatile void __iomem *addr)
{
	return (((*(volatile unsigned int*) (addr))&(mask)) == (mask));
}

static inline void set_bits(unsigned int  mask, volatile void __iomem *addr)
{
    (*(volatile unsigned int *) (addr)) |= mask;
}

static inline void clr_bits(unsigned int mask, volatile void __iomem *addr)
{
    (*(volatile unsigned int *) (addr)) &= ~(mask);
}

static inline int get_contexthub_dts_status(void)
{
	int len = 0;
	struct device_node* node = NULL;
	const char* status = NULL;
	node = of_find_compatible_node(NULL, NULL, "hisilicon,contexthub_status");
	if (node) {
		status = of_get_property(node, "status", &len);
		if(!status ) {
			pr_err("[%s]of_get_property status err\n", __func__);
			return -1;
		}

		if(strstr(status, "disabled")) {
			pr_info("[%s][disabled]\n", __func__);
			return -1;
		}
	}

	pr_info("[%s][enabled]\n", __func__);
	return 0;
}

#endif

