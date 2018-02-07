#include <linux/errno.h>
#include <linux/wakelock.h>
#include <linux/clk-provider.h>
#include "ipu_clock.h"

static int ipu_clock_get_dtsi_rate(struct device *dev, unsigned int *start_rate, unsigned int *stop_rate)
{
	int ret;

	/* read start clock rate from dtsi by "ics-clk-start-rate" */
	ret = of_property_read_u32(dev->of_node, "ics-clk-start-rate", start_rate); /*lint -save -e838*/
	if (ret) {
		printk(KERN_ERR"[%s]: get start rate failed, ret:%d\n", __func__, ret);
		return -ENOMEM;
	}

	/* read stop clock rate from dtsi by "ics-clk-stop-rate" */
	ret = of_property_read_u32(dev->of_node, "ics-clk-stop-rate", stop_rate); /*lint -save -e838*/
	if (ret) {
		printk(KERN_ERR"[%s]: get stop rate failed, ret:%d\n", __func__, ret);
		return -ENOMEM;
	}

	printk(KERN_DEBUG"[%s]: get clk rate done, start clk rate:%u, stop clk rate:%u\n",
		__func__, *start_rate, *stop_rate);

	return ret;
}

int ipu_clock_init(struct device *dev, struct clk **clock, unsigned int *start_rate, unsigned int *stop_rate)
{
    int ret;

	/* get clock of "clk-ics" from CLK API */
	*clock = devm_clk_get(dev, "clk-ics");

	if (!(*clock)) {
		printk(KERN_ERR"[%s]: get clock failed\n", __func__);
		return -ENODEV;
	}

	ret = ipu_clock_get_dtsi_rate(dev, start_rate, stop_rate);

	printk(KERN_DEBUG"[%s]: get clock done, ret is %d\n", __func__, ret);
	return ret;
}

int ipu_clock_start(struct clk *clock, unsigned int clock_rate)
{
	int ret;

	/* WARNING: clk_prepare_enable should NOT be called in interrupt because it contains mutex.
	   If needed in furture, use API: clk_prepare and clk_enable instead of clk_prepare_enable
	   in interrupt functions. */
	ret = clk_prepare_enable(clock);

	if (ret) {
		printk(KERN_ERR"[%s]: clk prepare enable failed,ret=%d\n", __func__, ret);
		return ret;
	}

	ret = ipu_clock_set_rate(clock, clock_rate);
	if (ret) {
		printk(KERN_ERR"[%s]: ipu_clock_set_rate failed,ret=%d\n", __func__, ret);
		return ret;
	}

	printk(KERN_DEBUG"[%s]: ipu_clock_start success to: %ld\n", __func__, clk_get_rate(clock));

	return 0;
}

int ipu_clock_set_rate(struct clk *clock, unsigned int clock_rate)
{
	int ret;

	/*WARNING: clk_prepare_enable should NOT be called in interrupt because it contains mutex. */
	ret = clk_set_rate(clock, (unsigned long)clock_rate);
	if (ret != 0) {
		printk(KERN_ERR"[%s]: set rate %d fail, ret:%d\n", __func__, clock_rate, ret);
		return ret;
	}

	printk(KERN_DEBUG"[%s]: set rate success to: %d\n", __func__, clock_rate);

	//TODO: DRV guoxiaodong will offer more clock info(PLLx, divide number...) in 2017/3/20, add more prints.

	return 0;
}

void ipu_clock_stop(struct clk *clock)
{
	clk_disable_unprepare(clock);

	printk(KERN_DEBUG"[%s]: ipu_clock_stop done\n", __func__);
}

