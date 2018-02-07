#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/gfp.h>
#include <linux/blk-mq.h>
#include "blk.h"
#include "blk-mq.h"

static struct workqueue_struct *blk_bkops_workqueue;

static void blk_bkops_execution_work(struct work_struct *work)
{
	int bkops_status = 0;
	struct blk_bkops_func *bkops = container_of((struct delayed_work *)work, struct blk_bkops_func, bkops_execution_worker);
	struct blk_lld_func* lld = container_of(bkops, struct blk_lld_func, blk_bkops);
	struct request_queue *q = blk_get_queue_by_lld(lld);

	if (!test_bit(BLK_BKOPS_ENABLE,&lld->blk_bkops.bkops_flag)) {
		pr_err("%s %d\n", __func__, __LINE__);
		return;
	}
	if (lld->blk_bkops.bkops_ops->bkops_status_query &&
		(!lld->blk_bkops.bkops_ops->bkops_status_query(q,&bkops_status))&&(!bkops_status)){
		if (test_bit(BLK_BKOPS_START_STATUS,&lld->blk_bkops.bkops_flag))
			lld->blk_bkops.bkops_ops->bkops_start_stop(q,BLK_BKOPS_STOP);
		clear_bit(BLK_BKOPS_START_STATUS,&lld->blk_bkops.bkops_flag);
		clear_bit(BLK_BKOPS_ENABLE,&lld->blk_bkops.bkops_flag);
		//printk(KERN_EMERG "bkops auto stop \r\n");
		return;
	}
	if (!test_bit(BLK_BKOPS_START_STATUS,&lld->blk_bkops.bkops_flag)) {
		lld->blk_bkops.bkops_ops->bkops_start_stop(q,BLK_BKOPS_START);
	}
	set_bit(BLK_BKOPS_START_STATUS, &lld->blk_bkops.bkops_flag);
}

/*
 * we check BKOPS status when :
 * 	1. first boot
 *  2. every "max_check_interval" seconds.
 *  3. when data partition available capacity is less than BKOPS_CAP_LOW_THRESHOLD,
 *     check BKOPS status when every "max_write_len" bytes wrote to device.
 */
static int bkops_should_query(struct blk_bkops_func* blk_bkops)
{
	struct timespec tp;

#ifdef CONFIG_HISI_DEBUG_FS
	if (blk_bkops->unconditional_query)
		return 1;
#endif

	get_monotonic_boottime(&tp);
	if (!blk_bkops->last_bkops_time) {
		blk_bkops->last_bkops_time = tp.tv_sec;
		return 1;
	}

	if (tp.tv_sec - blk_bkops->last_bkops_time >= blk_bkops->max_check_interval) {
		blk_bkops->last_bkops_time = tp.tv_sec;
		return 1;
	}

	return 0;
}

static enum blk_busy_idle_callback_return bkops_io_busy_idle_notify_handler(struct blk_busy_idle_nb *nb, enum blk_idle_notify_state state)
{
	enum blk_busy_idle_callback_return ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
	struct blk_bkops_func* blk_bkops = container_of(nb, struct blk_bkops_func, bkops_nb);
	struct request_queue *q = (struct request_queue *)nb->param_data;


	if ((!test_bit(BLK_BKOPS_ENABLE,&blk_bkops->bkops_flag)) && (state == BLK_IDLE_NOTIFY) && bkops_should_query(blk_bkops))
		set_bit(BLK_BKOPS_ENABLE, &blk_bkops->bkops_flag);

	if (!test_bit(BLK_BKOPS_ENABLE,&blk_bkops->bkops_flag))
		return BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;

	switch(state){
		case BLK_IDLE_NOTIFY:
			queue_delayed_work(blk_bkops_workqueue,&blk_bkops->bkops_execution_worker,
				msecs_to_jiffies(blk_bkops->bkops_ops->bkops_idle_start_interval_ms));
			ret = BLK_BUSY_IDLE_HANDLE_IO_TRIGGER;
			break;
		case BLK_BUSY_NOTIFY:
			cancel_delayed_work_sync(&blk_bkops->bkops_execution_worker);
			if (test_bit(BLK_BKOPS_MANUAL_STOP_BEFORE_IO_BUSY,&blk_bkops->bkops_flag) && test_bit(BLK_BKOPS_START_STATUS,&blk_bkops->bkops_flag))
				blk_bkops->bkops_ops->bkops_start_stop(q,BLK_BKOPS_STOP);
			clear_bit(BLK_BKOPS_START_STATUS,&blk_bkops->bkops_flag);
			if (!(blk_bkops->en_bkops_retry)) {
				clear_bit(BLK_BKOPS_ENABLE, &blk_bkops->bkops_flag);
			}
			ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
			break;
	}
	return ret;
}

int blk_queue_bkops_enable(struct request_queue *q, int enable)
{
	int ret = 0;
	struct blk_lld_func* lld = blk_get_lld(q);

	if (lld->blk_bkops.bkops_ops == NULL ||
		lld->blk_bkops.bkops_ops->bkops_start_stop == NULL) {/* Current device support bkops or not */
		pr_err("current dev doesn't support bkops!\n");
		return 0;
	}

	if (test_bit(BLK_BKOPS_ENABLE, &lld->blk_bkops.bkops_flag) == enable)
		return 0;

	if (enable == 0)
		clear_bit(BLK_BKOPS_ENABLE, &lld->blk_bkops.bkops_flag);
	else {
		set_bit(BLK_BKOPS_ENABLE, &lld->blk_bkops.bkops_flag);
	}

	printk(KERN_EMERG "<%s>bkops enable = %d \r\n",__func__, test_bit(BLK_BKOPS_ENABLE, &lld->blk_bkops.bkops_flag));
	return ret;
}
EXPORT_SYMBOL(blk_queue_bkops_enable);

#define HISI_BLK_BKOPS_MODULE_NAME	"blk_bkops"
void blk_bkops_init(struct blk_bkops_func* bkops)
{
	bkops->bkops_flag = 0;
	INIT_DELAYED_WORK(&bkops->bkops_execution_worker, blk_bkops_execution_work);
	bkops->bkops_nb.subscriber_name = HISI_BLK_BKOPS_MODULE_NAME;
	bkops->bkops_nb.blk_busy_idle_notifier_callback = bkops_io_busy_idle_notify_handler;
	bkops->bkops_nb.busy_idle_nb.notifier_call = NULL;
}

int __init hisi_blk_bkops_init(void)
{
	blk_bkops_workqueue = alloc_workqueue("bkops_workqueue", WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);

	return 0;
}

