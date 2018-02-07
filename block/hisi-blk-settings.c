#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/gfp.h>
#include <linux/blk-mq.h>

#include "blk.h"
#include "blk-mq.h"

void blk_queue_bkops(struct request_queue *q, struct blk_bkops_ops *bkops_ops, unsigned long bkops_flag)
{
	struct blk_lld_func* lld = blk_get_lld(q);

	if (bkops_ops->bkops_idle_start_interval_ms < BKOPS_RESTART_INTERVAL) {
		pr_err("bkops_idle_start_interval_ms is less than %d, change it to default value!\n", BKOPS_RESTART_INTERVAL);
		bkops_ops->bkops_idle_start_interval_ms = BKOPS_RESTART_INTERVAL;
	}
	clear_bit(BLK_BKOPS_ENABLE,&bkops_flag);
	clear_bit(BLK_BKOPS_START_STATUS,&bkops_flag);
	lld->blk_bkops.bkops_ops = bkops_ops;
	lld->blk_bkops.bkops_flag = bkops_flag;
	lld->blk_bkops.unconditional_query = 0;
	lld->blk_bkops.en_bkops_retry = 0;
	lld->blk_bkops.max_check_interval = BKOPS_CHECK_INTERVAL;
	lld->blk_bkops.last_bkops_time = 0;
	lld->blk_bkops.max_discard_len = BKOPS_DEF_DISCARD_LEN;
	lld->blk_bkops.accumulated_discard = 0;
	lld->blk_bkops.max_write_len = BKOPS_DEF_WRITE_LEN;
	lld->blk_bkops.accumulated_write = 0;
	printk(KERN_EMERG "bkops_idle_start_interval_ms = %lu \r\n",lld->blk_bkops.bkops_ops->bkops_idle_start_interval_ms);
}
EXPORT_SYMBOL(blk_queue_bkops);

int blk_bd_bkops_enable(struct block_device *bdev, int enable)
{
	if (bdev == NULL ||bdev->bd_disk == NULL)
		return -EIO;
	return blk_queue_bkops_enable(bdev_get_queue(bdev),enable);
}
EXPORT_SYMBOL(blk_bd_bkops_enable);

int blk_queue_hw_bkops_support(struct request_queue *q, int support)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	lld->blk_bkops.bkops_nb.param_data = (void*)q;
	lld->blk_bkops.bkops_nb.nb_flag = 0UL;
	if(support)
		return blk_busy_idle_event_register(q, &lld->blk_bkops.bkops_nb);
	else
		return blk_busy_idle_event_unregister(q, &lld->blk_bkops.bkops_nb);
}
EXPORT_SYMBOL(blk_queue_hw_bkops_support);

int blk_busy_idle_event_subscriber(struct block_device*bi_bdev, struct blk_busy_idle_nb* notify_nb)
{
	return blk_busy_idle_event_register(bdev_get_queue(bi_bdev),notify_nb);
}
EXPORT_SYMBOL(blk_busy_idle_event_subscriber);

int blk_queue_busy_idle_event_subscriber(struct request_queue *q, struct blk_busy_idle_nb* notify_nb)
{
	return blk_busy_idle_event_register(q,notify_nb);
}
EXPORT_SYMBOL(blk_queue_busy_idle_event_subscriber);

int blk_busy_idle_event_unsubscriber(struct block_device	*bi_bdev, struct blk_busy_idle_nb* notify_nb)
{
	return blk_busy_idle_event_unregister(bdev_get_queue(bi_bdev),notify_nb);
}
EXPORT_SYMBOL(blk_busy_idle_event_unsubscriber);

int blk_queue_busy_idle_event_unsubscriber(struct request_queue *q, struct blk_busy_idle_nb* notify_nb)
{
	return blk_busy_idle_event_unregister(q,notify_nb);
}
EXPORT_SYMBOL(blk_queue_busy_idle_event_unsubscriber);

bool blk_busy_idle_enable_query(struct block_device	*bi_bdev)
{
	struct blk_lld_func* blk_lld = blk_get_lld(bdev_get_queue(bi_bdev));
	return blk_lld->blk_idle.idle_notify_enable ? true : false;
}
EXPORT_SYMBOL(blk_busy_idle_enable_query);

void blk_queue_busy_idle_enable(struct request_queue *q, int enable)
{
	struct blk_lld_func* blk_lld = blk_get_lld(q);
	blk_lld->blk_idle.idle_notify_enable = (unsigned char)enable;
}
EXPORT_SYMBOL(blk_queue_busy_idle_enable);

#ifdef CONFIG_HISI_BLK_FLUSH_REDUCE
void blk_direct_flush_register(struct request_queue *q,direct_flush_fn* direct_flush)
{
	if(q->direct_flush && direct_flush==NULL)
		blk_flush_reduced_queue_unregister(q);
	q->direct_flush = direct_flush;
	if(q->direct_flush)
		blk_flush_reduced_queue_register(q);
}
EXPORT_SYMBOL_GPL(blk_direct_flush_register);

void blk_flush_reduce(struct request_queue *q, unsigned char en)
{
	q->blk_flush_reduce = en ? 1 : 0;
}
EXPORT_SYMBOL_GPL(blk_flush_reduce);

void blk_flush_set_async( struct bio *bio)
{
	bio->bi_async_flush = 1;
}
EXPORT_SYMBOL_GPL(blk_flush_set_async);

int blk_flush_async_support(struct block_device	*bi_bdev)
{
	struct request_queue *q = bdev_get_queue(bi_bdev);
	return q->blk_flush_reduce;
}
EXPORT_SYMBOL_GPL(blk_flush_async_support);

int blk_queue_flush_async_support(struct request_queue *q)
{
	return q->blk_flush_reduce;
}
EXPORT_SYMBOL_GPL(blk_queue_flush_async_support);
#endif