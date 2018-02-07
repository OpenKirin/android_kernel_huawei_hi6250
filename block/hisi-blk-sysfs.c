#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/blktrace_api.h>
#include <linux/blk-mq.h>
#include <linux/blk-cgroup.h>

#include "blk.h"
#include "blk-mq.h"
#include "hisi-blk-mq.h"

/*lint -e421*/
extern unsigned char io_busy_idle_state;
extern unsigned char io_busy_idle_multi_nb_state[5];
extern enum blk_busy_idle_callback_return test_io_idle_notify_handler(struct blk_busy_idle_nb *nb, enum blk_idle_notify_state state);
extern enum blk_busy_idle_callback_return m_nb_test_io_idle_notify_handler(struct blk_busy_idle_nb *nb, enum blk_idle_notify_state state);
extern void blk_prepare_bkops_test(struct request_queue *q);

ssize_t hisi_queue_bkops_enable_show(struct request_queue *q, char *page)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	return snprintf(page, PAGE_SIZE, "%s \n", test_bit(BLK_BKOPS_ENABLE,&lld->blk_bkops.bkops_flag) ? "enable" : "disable");
}

ssize_t hisi_queue_bkops_enable_store(struct request_queue *q, const char *page, size_t count)
{
	/*lint -save -e747*/
	if (!strncmp(page, "enable", 6) )
		blk_queue_bkops_enable(q,1);
	else if (!strncmp(page, "disable", 7) )
		blk_queue_bkops_enable(q,0);
	/*lint -restore*/
	return (ssize_t)count;
}

ssize_t hisi_queue_busy_idle_enable_show(struct request_queue *q, char *page)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	return snprintf(page, PAGE_SIZE, "%s \n", lld->blk_idle.idle_notify_enable ? "enable" : "disable");
}

ssize_t hisi_queue_busy_idle_enable_store(struct request_queue *q, const char *page, size_t count)
{
	/*lint -save -e747*/
	if (!strncmp(page, "enable", 6) ){
		printk(KERN_EMERG "<%s> enable busy idle notify \r\n", q->request_queue_disk->disk_name);
		blk_queue_busy_idle_enable(q,1);
	}
	else if (!strncmp(page, "disable", 7) ) {
		printk(KERN_EMERG "<%s> disable busy idle notify \r\n", q->request_queue_disk->disk_name);
		blk_queue_busy_idle_enable(q,0);
	}
	/*lint -restore*/
	return (ssize_t)count;
}

static s64 hisi_ktime_to_common_time(ktime_t ktime, char* unit)
{
	s64 time = ktime_to_ms(ktime);
	if(time < 1000){
		unit[0] = 'm';
		unit[1] = 's';
		unit[2] = 0;
		goto exit;
	}
	unit[0] = 's';
	unit[1] = unit[2] = 0;
	time /= 1000;;
exit:
	return time;
}

ssize_t hisi_queue_busy_idle_statistic_store(struct request_queue *q, const char *page, size_t count)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	/*lint -save -e747*/
	if (!strncmp(page, "clear", 5) ){
		printk(KERN_EMERG "clear busy idle statistic on %s \r\n", q->request_queue_disk->disk_name);
		lld->blk_idle.total_busy_ktime= lld->blk_idle.total_idle_ktime = ktime_set(0,0);
		lld->blk_idle.total_idle_count = 0ULL;
	}
	/*lint -restore*/
	return (ssize_t)count;
}

ssize_t hisi_queue_busy_idle_statistic_show(struct request_queue *q, char *page)
{
	char unit_b[3];
	char unit_i[3];
	unsigned long offset = 0;
	struct blk_lld_func* lld = blk_get_lld(q);
	offset += snprintf(page,PAGE_SIZE,"Total Busy Time: %lld(%s)  Total Idle Time: %lld(%s)  \r\n",
		hisi_ktime_to_common_time(lld->blk_idle.total_busy_ktime,unit_b),unit_b,hisi_ktime_to_common_time(lld->blk_idle.total_idle_ktime,unit_i),unit_i);/*lint !e737*/
	offset += snprintf(page + offset,PAGE_SIZE,"Total Idle Count: %llu \r\n",lld->blk_idle.total_idle_count);/*lint !e737*/
	return (ssize_t)offset;
}

#define KERNEL_BUF_SIZE 4096
ssize_t hisi_queue_idle_dur_store(struct request_queue *q, const char *page, size_t count)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	/*lint -save -e747*/
	if (!strncmp(page, "clear", 5) ){
		printk(KERN_EMERG "clear idle duration statistic on %s \r\n", q->request_queue_disk->disk_name);
		memset(lld->blk_idle.blk_idle_dur, 0, sizeof(lld->blk_idle.blk_idle_dur));
		lld->blk_idle.max_idle_dur = 0;
	}
	/*lint -restore*/
	return (ssize_t)count;
}

ssize_t hisi_queue_idle_dur_show(struct request_queue *q, char *page)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	struct blk_idle_state *blk_idle = &(lld->blk_idle);
	ssize_t n = 0;

	n += snprintf(page, (size_t)KERNEL_BUF_SIZE, "block idle interval statistic:\n");
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "max_idle_duration: %lldms\n", blk_idle->max_idle_dur);
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "less than 100ms: %lld\n", blk_idle->blk_idle_dur[BLK_IDLE_100MS]);
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "less than 500ms: %lld\n", blk_idle->blk_idle_dur[BLK_IDLE_500MS]);
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "less than 1s: %lld\n", blk_idle->blk_idle_dur[BLK_IDLE_1000MS]);
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "less than 2s: %lld\n", blk_idle->blk_idle_dur[BLK_IDLE_2000MS]);
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "less than 4s: %lld\n", blk_idle->blk_idle_dur[BLK_IDLE_4000MS]);
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "less than 6s: %lld\n", blk_idle->blk_idle_dur[BLK_IDLE_6000MS]);
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "less than 8s: %lld\n", blk_idle->blk_idle_dur[BLK_IDLE_8000MS]);
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "less than 10s: %lld\n", blk_idle->blk_idle_dur[BLK_IDLE_10000MS]);
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "more than 10s: %lld\n", blk_idle->blk_idle_dur[BLK_IDLE_FOR_AGES]);
	n += snprintf(page + n, (size_t)(KERNEL_BUF_SIZE - n), "idle total counts: %llu\n", blk_idle->total_idle_count);

	return n;
}

static ssize_t
hisi_queue_var_store(unsigned long *var, const char *page, size_t count)
{
	int err;
	unsigned long v;

	err = kstrtoul(page, 0, &v);
	if (err || v > UINT_MAX)
		return -EINVAL;

	*var = v;

	return (ssize_t)count;
}

ssize_t hisi_queue_bkops_retry_show(struct request_queue *q, char *page)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long en_bkops_retry = lld->blk_bkops.en_bkops_retry;

	pr_err("bkops_retry_en: %lu\n", en_bkops_retry);
	return snprintf(page, PAGE_SIZE, "%lu\n", en_bkops_retry);
}

ssize_t hisi_queue_bkops_retry_store(struct request_queue *q, const char *page, size_t count)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long en_bkops_retry;
	ssize_t ret;

	if (count > 10) {
		pr_err("%s invalid count: %lu\n", __func__, count);
		return 0;
	}

	ret = hisi_queue_var_store(&en_bkops_retry, page, count);
	if (ret < 0)
		return 0;

	pr_err("bkops_retry_en: %lu\n", en_bkops_retry);

	lld->blk_bkops.en_bkops_retry = en_bkops_retry;

	return ret;
}

ssize_t hisi_queue_bkops_unconditional_query_show(struct request_queue *q, char *page)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long unconditional_query = lld->blk_bkops.unconditional_query;

	pr_err("unconditional_query: %lu\n", unconditional_query);
	return snprintf(page, PAGE_SIZE, "%lu\n", unconditional_query);
}

ssize_t hisi_queue_bkops_unconditional_query_store(struct request_queue *q, const char *page, size_t count)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long unconditional_query;
	ssize_t ret = hisi_queue_var_store(&unconditional_query, page, count);

	if (ret < 0)
		return 0;

	pr_err("unconditional_query: %lu\n", unconditional_query);

	lld->blk_bkops.unconditional_query = unconditional_query;

	return ret;
}

ssize_t hisi_queue_bkops_restart_interval_show(struct request_queue *q, char *page)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	struct blk_bkops_ops *bkops_ops_p = lld->blk_bkops.bkops_ops;

	if (!bkops_ops_p)
		return 0;
	pr_err("bkops_idle_start_interval_ms: %lu\n", bkops_ops_p->bkops_idle_start_interval_ms);
	return snprintf(page, PAGE_SIZE, "%lu\n", bkops_ops_p->bkops_idle_start_interval_ms);
}

ssize_t hisi_queue_bkops_restart_interval_store(struct request_queue *q, const char *page, size_t count)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	struct blk_bkops_ops *bkops_ops_p = lld->blk_bkops.bkops_ops;
	unsigned long bkops_idle_start_interval_ms;
	ssize_t ret = hisi_queue_var_store(&bkops_idle_start_interval_ms, page, count);

	if (!bkops_ops_p || ret < 0)
		return 0;

	pr_err("bkops_idle_restart_interval_ms: %lu\n", bkops_idle_start_interval_ms);
	bkops_ops_p->bkops_idle_start_interval_ms = bkops_idle_start_interval_ms;

	return ret;
}

ssize_t hisi_queue_bkops_max_check_interval_show(struct request_queue *q, char *page)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long max_check_interval = lld->blk_bkops.max_check_interval;

	pr_err("max_check_interval: %lu\n", max_check_interval);
	return snprintf(page, PAGE_SIZE, "%lu\n", max_check_interval);
}

ssize_t hisi_queue_bkops_max_check_interval_store(struct request_queue *q, const char *page, size_t count)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long max_check_interval;
	ssize_t ret = hisi_queue_var_store(&max_check_interval, page, count);

	if (ret < 0)
		return ret;

	pr_err("max_check_interval: %lu\n", max_check_interval);

	lld->blk_bkops.max_check_interval = max_check_interval;

	return ret;
}

ssize_t hisi_queue_bkops_max_discard_len_show(struct request_queue *q, char *page)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long max_discard_len = lld->blk_bkops.max_discard_len;

	pr_err("max_discard_len: 0x%lx\n", max_discard_len);
	return snprintf(page, PAGE_SIZE, "0x%lx\n", max_discard_len);
}

ssize_t hisi_queue_bkops_max_discard_len_store(struct request_queue *q, const char *page, size_t count)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long max_discard_len;
	ssize_t ret = hisi_queue_var_store(&max_discard_len, page, count);

	if (ret < 0)
		return ret;

	pr_err("bkops_discard_len: 0x%lx\n", max_discard_len);

	lld->blk_bkops.max_discard_len = max_discard_len;

	return ret;
}

ssize_t hisi_queue_bkops_max_write_len_show(struct request_queue *q, char *page)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long max_write_len = lld->blk_bkops.max_write_len;

	pr_err("max_write_len: 0x%lx\n", max_write_len);
	return snprintf(page, PAGE_SIZE, "0x%lx\n", max_write_len);
}

ssize_t hisi_queue_bkops_max_write_len_store(struct request_queue *q, const char *page, size_t count)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long max_write_len;
	ssize_t ret = hisi_queue_var_store(&max_write_len, page, count);

	if (ret < 0)
		return ret;

	pr_err("bkops_discard_len: 0x%lx\n", max_write_len);

	lld->blk_bkops.max_write_len = max_write_len;

	return ret;
}

ssize_t hisi_queue_bkops_test_enable_store(struct request_queue *q, const char *page, size_t count)
{
	/*lint -save -e747*/
	if (!strncmp(page, "enable", 6) ){
		printk(KERN_EMERG "<%s> enable bkops test \r\n", q->request_queue_disk->disk_name);
		blk_prepare_bkops_test(q);
		blk_queue_hw_bkops_support(q,1);
	}
	else if (!strncmp(page, "disable", 7) ) {
		printk(KERN_EMERG "<%s> disable bkops test \r\n", q->request_queue_disk->disk_name);
		blk_queue_hw_bkops_support(q,0);
	}
	/*lint -restore*/
	return (ssize_t)count;
}

#define HISI_BUSY_IDLE_TEST_NB_NAME	"blk_busy_idle_notify_test"
ssize_t hisi_queue_busy_idle_test_enable_store(struct request_queue *q, const char *page, size_t count)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	/*lint -save -e747*/
	if (!strncmp(page, "enable", 6) ){
		printk(KERN_EMERG "<%s> subscriber event \r\n", q->request_queue_disk->disk_name);
		lld->blk_idle.idle_notify_test_nb.subscriber_name = HISI_BUSY_IDLE_TEST_NB_NAME;
		lld->blk_idle.idle_notify_test_nb.blk_busy_idle_notifier_callback = test_io_idle_notify_handler;
		lld->blk_idle.idle_notify_test_nb.param_data = (void*)q;
		lld->blk_idle.idle_notify_test_nb.nb_flag = BLK_BUSY_IDLE_NB_FLAG_NOT_JOIN_POLL;
		blk_queue_busy_idle_event_subscriber(q,&lld->blk_idle.idle_notify_test_nb);
	}
	else if (!strncmp(page, "disable", 7) ) {
		printk(KERN_EMERG "<%s> unsubscriber event \r\n", q->request_queue_disk->disk_name);
		blk_queue_busy_idle_event_unsubscriber(q,&lld->blk_idle.idle_notify_test_nb);
		io_busy_idle_state = BLK_IO_IDLE;
	}
	/*lint -restore*/
	return (ssize_t)count;
}

ssize_t hisi_queue_busy_idle_multi_nb_test_enable_store(struct request_queue *q, const char *page, size_t count)
{
	struct blk_lld_func* lld = blk_get_lld(q);
	unsigned long long i;
	/*lint -save -e747*/
	if (!strncmp(page, "enable", 6) ){
		for(i=0;i<5;i++){
			lld->blk_idle.idle_notify_common_nb[i].subscriber_name = HISI_BUSY_IDLE_TEST_NB_NAME;
			lld->blk_idle.idle_notify_common_nb[i].blk_busy_idle_notifier_callback = m_nb_test_io_idle_notify_handler;
			lld->blk_idle.idle_notify_common_nb[i].param_data = (void*)i;
			lld->blk_idle.idle_notify_common_nb[i].nb_flag = 0UL;
			blk_queue_busy_idle_event_subscriber(q,&lld->blk_idle.idle_notify_common_nb[i]);
		}
	}
	else if (!strncmp(page, "disable", 7) ) {
		for(i=0;i<5;i++){
			blk_queue_busy_idle_event_unsubscriber(q,&lld->blk_idle.idle_notify_common_nb[i]);
			io_busy_idle_multi_nb_state[i] = BLK_IO_IDLE;
		}
	}
	/*lint -restore*/
	return (ssize_t)count;
}
/*lint +e421*/

