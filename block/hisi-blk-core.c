#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/gfp.h>
#include <linux/blk-mq.h>

#include "blk.h"
#include "blk-mq.h"

/*lint -e438 -e529 -e730 -e747 -e732 -e737 -e826*/
#define BLK_BUSY_IDLE_HANDLE_LATENCY_WARNING_THRESHOLD_MS	1000U
#define BLK_BUSY_IDLE_HANDLE_LATENCY_LOG_WARNING_THRESHOLD_MS 10ULL

static struct workqueue_struct *blk_idle_notify_workqueue;

struct blk_lld_func* blk_get_lld(struct request_queue *q)
{
	if(q->mq_ops)
		return (q->tag_set ? &q->tag_set->lld_func : &q->lld_func);
	else
		return (q->queue_tags ? &q->queue_tags->lld_func : &q->lld_func);
}

struct request_queue* blk_get_queue_by_lld(struct blk_lld_func* lld)
{
	struct blk_queue_tag *tag;
	struct blk_mq_tag_set* tag_set;
	switch(lld->type){
		case BLK_LLD_QUEUE_BASE:
			return (struct request_queue *)(lld->data);
		case BLK_LLD_QUEUE_TAG_BASE:
			tag = (struct blk_queue_tag *)(lld->data);
			return list_last_entry(&tag->tag_list,struct request_queue,tag_set_list); /*lint !e826*/
		case BLK_LLD_TAGSET_BASE:
			tag_set = (struct blk_mq_tag_set*)(lld->data);
			return  list_last_entry(&tag_set->tag_list,struct request_queue,tag_set_list); /*lint !e826*/
	}
	return NULL;
}

static void blk_busy_idle_handler_latency_check_timer_expire(unsigned long data)
{
	struct blk_busy_idle_nb* notify_nb = (struct blk_busy_idle_nb*) data;

	printk(KERN_EMERG "<%s> %s notify block process time is more than %d ms \r\n", __func__,
			notify_nb->subscriber_name, BLK_BUSY_IDLE_HANDLE_LATENCY_WARNING_THRESHOLD_MS);
}

static int blk_busy_idle_notify_handler(struct notifier_block *nb, unsigned long val, void *v)
{
	enum blk_busy_idle_callback_return ret = BLK_BUSY_IDLE_HANDLE_NO_IO_TRIGGER;
	int handler_result = NOTIFY_DONE;
	ktime_t notify_start_ktime;
	struct blk_busy_idle_nb* notify_nb = container_of(nb, struct blk_busy_idle_nb, busy_idle_nb); /*lint !e826*/
	if(notify_nb->blk_busy_idle_notifier_callback == NULL){
		printk(KERN_EMERG "<%s> %s notify block illegal!callback = %pK \r\n", __func__,
			notify_nb->subscriber_name, notify_nb->blk_busy_idle_notifier_callback);
#ifdef CONFIG_HISI_DEBUG_FS
		BUG_ON(1); /*lint !e730*/
#else
		return NOTIFY_DONE;
#endif
	}
	notify_start_ktime = ktime_get();
	mod_timer(&notify_nb->busy_idle_handler_latency_check_timer,jiffies + msecs_to_jiffies(BLK_BUSY_IDLE_HANDLE_LATENCY_WARNING_THRESHOLD_MS));
	switch(val){
		case BLK_IDLE_NOTIFY:
			if(notify_nb->last_state == BLK_IO_BUSY){
				ret = notify_nb->blk_busy_idle_notifier_callback(notify_nb,BLK_IDLE_NOTIFY);
				notify_nb->last_state = BLK_IO_IDLE;
				if(ret == BLK_BUSY_IDLE_HANDLE_IO_TRIGGER){
					handler_result = NOTIFY_STOP;
					notify_nb->continue_trigger_io_count++;
				}
				else
					notify_nb->continue_trigger_io_count = 0;
			}
			break;
		case BLK_BUSY_NOTIFY:
			if(notify_nb->last_state == BLK_IO_IDLE){
				ret = notify_nb->blk_busy_idle_notifier_callback(notify_nb,BLK_BUSY_NOTIFY);
				notify_nb->last_state = BLK_IO_BUSY;
			}
			break;
		default:
#ifdef CONFIG_HISI_DEBUG_FS
			BUG_ON(1); /*lint !e730*/
#endif
			break;
	}
	del_timer_sync(&notify_nb->busy_idle_handler_latency_check_timer);
	if(ktime_after(ktime_get(),ktime_add_ms(notify_start_ktime, BLK_BUSY_IDLE_HANDLE_LATENCY_LOG_WARNING_THRESHOLD_MS)))
		printk(KERN_EMERG "<%s> %s blk_busy_idle_notifier_callback more than 10ms, actual %lld ms \r\n", __func__,notify_nb->subscriber_name,ktime_to_ms(ktime_sub(ktime_get(),notify_start_ktime))); /*lint !e446*/
	if(ret == BLK_BUSY_IDLE_HANDLE_ERR){
		printk(KERN_EMERG "<%s> %s blk_busy_idle_notifier_callback meets error! \r\n", __func__,notify_nb->subscriber_name);
#ifdef CONFIG_HISI_DEBUG_FS
		dump_stack();
#endif
	}
	return handler_result;
} /*lint !e715*/

int blk_busy_idle_event_register(struct request_queue *q, struct blk_busy_idle_nb* notify_nb)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	if(notify_nb->subscriber_name == NULL ||notify_nb->blk_busy_idle_notifier_callback == NULL){
		printk(KERN_EMERG "<%s> %s notify block register illegal!callback = %pK \r\n",
			__func__,notify_nb->subscriber_name, notify_nb->blk_busy_idle_notifier_callback);
#ifdef CONFIG_HISI_DEBUG_FS
		BUG_ON(1); /*lint !e730*/
#else
		dump_stack();
		return -EINVAL;
#endif
	}
	if(notify_nb->busy_idle_nb.notifier_call == blk_busy_idle_notify_handler){
		if(notify_nb->subscriber_source != lld ){
			printk(KERN_EMERG "<%s> %s notify block has been register to %pK, current lld is %pK\r\n",
			__func__,notify_nb->subscriber_name,notify_nb->subscriber_source,lld);
#ifdef CONFIG_HISI_DEBUG_FS
			BUG_ON(1);
#else
			dump_stack();
			return -EPERM;
#endif
		}
		return 0;
	}
	notify_nb->busy_idle_nb.notifier_call = blk_busy_idle_notify_handler;
	notify_nb->busy_idle_nb.priority = 0;
	init_timer(&notify_nb->busy_idle_handler_latency_check_timer);
	notify_nb->busy_idle_handler_latency_check_timer.data = (unsigned long)notify_nb;
	notify_nb->busy_idle_handler_latency_check_timer.function = blk_busy_idle_handler_latency_check_timer_expire;
	notify_nb->subscriber_source = lld;
	notify_nb->last_state = BLK_IO_IDLE;
	notify_nb->continue_trigger_io_count = 0;
	blocking_notifier_chain_register(&lld->blk_idle.blk_idle_event_subscribers, &notify_nb->busy_idle_nb);

	pr_info("%s registered to busy/idle module.\n", notify_nb->subscriber_name);
	return 0;
}

int blk_busy_idle_event_unregister(struct request_queue *q, struct blk_busy_idle_nb* notify_nb)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	if(notify_nb->busy_idle_nb.notifier_call != blk_busy_idle_notify_handler)
		return -EINVAL;
	if(notify_nb->subscriber_source != lld){
		printk(KERN_EMERG "<%s> %s notify block illegal!current lld is %pK, notify registered lld is %pK \r\n",
			__func__,notify_nb->subscriber_name, lld, notify_nb->subscriber_source);
#ifdef CONFIG_HISI_DEBUG_FS
		BUG_ON(1);
#else
		dump_stack();
		return -EINVAL;
#endif
	}
	blocking_notifier_chain_unregister(&lld->blk_idle.blk_idle_event_subscribers, &notify_nb->busy_idle_nb);
	notify_nb->subscriber_name = NULL;
	notify_nb->busy_idle_nb.notifier_call = NULL;
	notify_nb->subscriber_source = NULL;

	pr_info("%s unregistered from busy/idle module.\n", notify_nb->subscriber_name);
	return 0;
}

static void blk_idle_notify_work(struct work_struct* work)
{
	struct blk_idle_state *state = (struct blk_idle_state *)container_of((struct delayed_work *)work, struct blk_idle_state, idle_notify_worker);
	struct blk_lld_func* blk_lld = container_of(state, struct blk_lld_func, blk_idle);
	struct request_queue *q = blk_get_queue_by_lld(blk_lld);

	mutex_lock(&state->io_count_mutex);
	if((atomic_read(&state->io_count)==0) && (state->idle_state != BLK_IO_IDLE)) {
		state->idle_state = BLK_IO_IDLE;
		state->total_idle_count++;
		state->last_idle_ktime = ktime_get();
		if(!ktime_equal(state->last_busy_ktime,ktime_set(0,0)))
			state->total_busy_ktime = ktime_add(state->total_busy_ktime, ktime_sub(state->last_idle_ktime, state->last_busy_ktime));
		blocking_notifier_call_chain(&state->blk_idle_event_subscribers, (unsigned long)BLK_IDLE_NOTIFY, (void*)q);
	}
	mutex_unlock(&state->io_count_mutex);
}

void blk_idle_count(struct request_queue *q)
{
	struct blk_lld_func *lld = blk_get_lld(q);
	if(atomic_read(&lld->blk_idle.io_count)==0){
		if(lld->blk_idle.idle_notify_enable){
			printk(KERN_EMERG "<%s> io_count has been zero \r\n", __func__);
		#ifdef CONFIG_HISI_DEBUG_FS
			BUG_ON(1);
		#endif
		}
		return;
	}
	if(lld->blk_idle.idle_notify_enable){
		if(atomic_dec_return(&lld->blk_idle.io_count)==0)
			queue_delayed_work(blk_idle_notify_workqueue,&lld->blk_idle.idle_notify_worker, msecs_to_jiffies(lld->blk_idle.idle_notify_delay_ms));
	}
	else
		atomic_dec(&lld->blk_idle.io_count);
}

#ifdef CONFIG_HISI_DEBUG_FS
static void blk_busy_idle_update_dur(struct blk_idle_state *blk_idle)
{
	s64 blk_idle_dur_ms;

	if ( ktime_equal(blk_idle->last_idle_ktime, ktime_set(0,0)) ||
		ktime_equal(blk_idle->last_busy_ktime, ktime_set(0,0)) ) {
		return ;
	}

	blk_idle_dur_ms = ktime_to_ms(blk_idle->last_busy_ktime) - ktime_to_ms(blk_idle->last_idle_ktime);
	if (blk_idle_dur_ms > blk_idle->max_idle_dur) {
		blk_idle->max_idle_dur = blk_idle_dur_ms;
	}

	if (blk_idle_dur_ms < 100)
		blk_idle->blk_idle_dur[BLK_IDLE_100MS]++;
	else if (blk_idle_dur_ms < 500)
		blk_idle->blk_idle_dur[BLK_IDLE_500MS]++;
	else if (blk_idle_dur_ms < 1000)
		blk_idle->blk_idle_dur[BLK_IDLE_1000MS]++;
	else if (blk_idle_dur_ms < 2000)
		blk_idle->blk_idle_dur[BLK_IDLE_2000MS]++;
	else if (blk_idle_dur_ms < 4000)
		blk_idle->blk_idle_dur[BLK_IDLE_4000MS]++;
	else if (blk_idle_dur_ms < 6000)
		blk_idle->blk_idle_dur[BLK_IDLE_6000MS]++;
	else if (blk_idle_dur_ms < 8000)
		blk_idle->blk_idle_dur[BLK_IDLE_8000MS]++;
	else if (blk_idle_dur_ms < 10000)
		blk_idle->blk_idle_dur[BLK_IDLE_10000MS]++;
	else
		blk_idle->blk_idle_dur[BLK_IDLE_FOR_AGES]++;
}
#endif /* CONFIG_HISI_DEBUG_FS */

void blk_busy_count(struct request_queue *q)
{
	struct blk_lld_func *lld = blk_get_lld(q);

	if(lld->blk_idle.idle_notify_enable){
		mutex_lock(&lld->blk_idle.io_count_mutex);
		if(atomic_inc_return(&lld->blk_idle.io_count) == 1){
			cancel_delayed_work(&lld->blk_idle.idle_notify_worker);
			if(lld->blk_idle.idle_state == BLK_IO_IDLE){//Avoid twice busy event (The idle work may be canceled)
				lld->blk_idle.idle_state = BLK_IO_BUSY;
				lld->blk_idle.last_busy_ktime = ktime_get();
#ifdef CONFIG_HISI_DEBUG_FS
				blk_busy_idle_update_dur(&(lld->blk_idle));
#endif
				if(!ktime_equal(lld->blk_idle.last_idle_ktime,ktime_set(0,0)))
					lld->blk_idle.total_idle_ktime = ktime_add(lld->blk_idle.total_idle_ktime,ktime_sub(lld->blk_idle.last_busy_ktime, lld->blk_idle.last_idle_ktime));
				blocking_notifier_call_chain(&lld->blk_idle.blk_idle_event_subscribers,BLK_BUSY_NOTIFY,(void*)q);
			}
		}
		if(ktime_after(ktime_get(),ktime_add_ms(lld->blk_idle.last_busy_ktime, 3600000))){
		#ifdef CONFIG_HISI_DEBUG_FS
			BUG_ON(1);
		#else
			printk(KERN_EMERG "<%s> More than 1 hours not in IO IDLE!! \r\n", __func__);
		#endif
		}
		mutex_unlock(&lld->blk_idle.io_count_mutex);
	}
	else
		atomic_inc(&lld->blk_idle.io_count);
}

void blk_bio_in_count_set(struct request_queue *q, struct bio *bio)
{
#ifndef CONFIG_HISI_DEBUG_FS
	static DEFINE_RATELIMIT_STATE(bio_in_count_rs, (60 * HZ), 1);
#endif

	if(bio->bi_bdev == NULL)
		bio->q = q;
	if(bio->io_in_count & HISI_IO_IN_COUNT_WILL_BE_SEND_AGAIN)
		return;
	if(bio->io_in_count & HISI_IO_IN_COUNT_SET) {
#ifdef CONFIG_HISI_DEBUG_FS
		BUG_ON(1);
#else
		if (__ratelimit(&bio_in_count_rs)) {
			pr_err("Error, repeated bio\n");
			dump_stack();
		}
		return;
#endif
	}
	bio->io_in_count |= HISI_IO_IN_COUNT_SET;
	blk_busy_count(q);
}

bool blk_request_bio_in_count_check(struct request_queue *q, struct request *rq)
{
	struct bio *bio = rq->bio;
	if(bio){
		do{
			blk_bio_in_count_set(q,bio);
			bio = bio->bi_next;
		}while(bio);
		return true;
	}
	return false;
}

static void blk_end_rq(struct request *rq, int error)
{
	blk_idle_count(rq->q);
	if(rq->uplayer_end_io)
		rq->uplayer_end_io(rq,error);
}

void blk_execute_request_in_count_check(struct request_queue *q, struct request *rq,rq_end_io_fn *done)
{
	if(!blk_request_bio_in_count_check(q,rq)){
		blk_busy_count(q);
		rq->end_io = blk_end_rq;
		rq->uplayer_end_io = done;
	}
	else
		rq->end_io = done;
}

void blk_bio_endio_in_count_check(struct bio *bio)
{
	struct request_queue *q = bio->bi_bdev == NULL ? bio->q : bdev_get_queue(bio->bi_bdev);

	if(bio->io_in_count & (HISI_IO_IN_COUNT_SKIP_ENDIO | HISI_IO_IN_COUNT_DONE))
		return ;

	if(bio->io_in_count == 0){
		printk(KERN_EMERG "BLK_CORE: <%s> exception bio, type: %s(%lu) bio_addr: %pK\r\n",
			__func__, io_type_parse(bio->bi_rw), bio->bi_rw, bio);
#ifdef CONFIG_HISI_DEBUG_FS
		BUG_ON(1);
#endif
	}

	bio->io_in_count = HISI_IO_IN_COUNT_DONE;
	blk_idle_count(q);
}

static void blk_idle_state_init(struct blk_idle_state* blk_idle)
{
	blk_idle->total_busy_ktime = blk_idle->total_idle_ktime = ktime_set(0,0);
	blk_idle->last_busy_ktime = blk_idle->last_idle_ktime = ktime_set(0,0);
#ifdef CONFIG_HISI_DEBUG_FS
	memset(blk_idle->blk_idle_dur, 0, sizeof(blk_idle->blk_idle_dur));
	blk_idle->max_idle_dur = 0;
#endif
	blk_idle->total_idle_count = 0ULL;
	blk_idle->idle_notify_enable = 0;
	blk_idle->idle_notify_delay_ms = BLK_IO_IDLE_AVOID_JITTER_TIME;
	INIT_DELAYED_WORK(&blk_idle->idle_notify_worker, blk_idle_notify_work);
	atomic_set(&blk_idle->io_count, 0); /*lint !e1058*/
	mutex_init(&blk_idle->io_count_mutex);
	BLOCKING_INIT_NOTIFIER_HEAD(&blk_idle->blk_idle_event_subscribers);
	blk_idle->idle_state = BLK_IO_IDLE;
}

static void blk_lld_func_init(struct blk_lld_func* blk_lld, enum blk_lld_base type, void* data)
{
	blk_lld->feature_flag = 0;
	blk_lld->type = type;
	blk_lld->data = data;
	blk_idle_state_init(&blk_lld->blk_idle);
	blk_bkops_init(&blk_lld->blk_bkops);
}

void hisi_blk_allocated_queue_init(struct request_queue *q)
{
	blk_lld_func_init(&q->lld_func,BLK_LLD_QUEUE_BASE,(void*)q);
#ifdef CONFIG_HISI_BLK_FLUSH_REDUCE
	blk_queue_async_flush_init(q);
#endif
}

void hisi_blk_allocated_tags_init(struct blk_queue_tag *tags)
{
	mutex_init(&tags->tag_list_lock);
	INIT_LIST_HEAD(&tags->tag_list);
	blk_lld_func_init(&tags->lld_func,BLK_LLD_QUEUE_TAG_BASE,(void*)tags);
}

void blk_add_queue_tags(struct blk_queue_tag *tags,struct request_queue *q)
{
	mutex_lock(&tags->tag_list_lock);
	list_add_tail(&q->tag_set_list, &tags->tag_list);
	mutex_unlock(&tags->tag_list_lock);
}

void hisi_blk_mq_allocated_tagset_init(struct blk_mq_tag_set *set)
{
	blk_lld_func_init(&set->lld_func,BLK_LLD_TAGSET_BASE,(void*)set);
}

void hisi_blk_queue_register(struct request_queue *q, struct gendisk *disk)
{
	q->request_queue_disk = disk;
#ifdef CONFIG_HISI_DEBUG_FS
	printk(KERN_EMERG "<%s> request queue name = %s addr = 0x%llx\r\n", __func__,q->request_queue_disk->disk_name,(long long)q);
#endif
}

int __init hisi_blk_dev_init(void)
{
	blk_idle_notify_workqueue = alloc_workqueue("busy_idle_notify", WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);
	return hisi_blk_bkops_init();
}
/*lint +e438 +e529 +e730 +e732 +e737 +e747 +e826*/

