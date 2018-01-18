#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#ifdef CONFIG_HISI_MMC_MANUAL_BKOPS
#include <linux/blkdev.h>
#endif
/*lint -e421 -e715 -e747*/

#ifdef CONFIG_HISI_MMC_MANUAL_BKOPS
#define KERNEL_BUF_SIZE  4096
extern struct blk_bkops_ops mmc_bkops_ops;
static int mmc_bkops_stat_open(struct inode *inode, struct file *filp)
{
	struct mmc_card *card = inode->i_private;
	struct mmc_bkops_stats *bkops_stats_p = &(card->bkops_stats);
	char *buf;
	ssize_t n = 0;

	buf = (char *)kzalloc(KERNEL_BUF_SIZE, GFP_KERNEL); /*lint !e747*/
	if (!buf)
		return -ENOMEM;

	n += snprintf(buf, KERNEL_BUF_SIZE, "bkops idle start interval: %lu ms\n", mmc_bkops_ops.bkops_idle_start_interval_ms);

	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops count: %u\n", bkops_stats_p->bkops_count);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "hpi count: %u\n", bkops_stats_p->hpi_count);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops abort count: %u\n", bkops_stats_p->bkops_abort_count);

	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops non critical count: %u\n", bkops_stats_p->bkops_status[BKOPS_NON_CRITICAL]);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops perf impacted count: %u\n", bkops_stats_p->bkops_status[BKOPS_PERF_IMPACTED]);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops critical count: %u\n", bkops_stats_p->bkops_status[BKOPS_CRITICAL]);

	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops query count: %u\n", bkops_stats_p->bkops_query_count);

	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops query fail count: %u\n", bkops_stats_p->bkops_query_fail_count);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops start fail count: %u\n", bkops_stats_p->bkops_start_fail_count);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops stop fail count: %u\n", bkops_stats_p->bkops_stop_fail_count);

	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops max query time: %lluns\n", bkops_stats_p->bkops_max_query_time);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops average query time: %lluns\n", bkops_stats_p->bkops_avrg_query_time);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops max start time: %lluns\n", bkops_stats_p->bkops_max_start_time);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops average start time: %lluns\n", bkops_stats_p->bkops_avrg_start_time);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops max stop time: %lluns\n", bkops_stats_p->bkops_max_stop_time);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops average stop time: %lluns\n", bkops_stats_p->bkops_avrg_stop_time);

	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "max_bkops_duration: %ums\n", jiffies_to_msecs(bkops_stats_p->max_bkops_duration));
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops duration less than 100ms: %u\n", bkops_stats_p->bkops_dur[BKOPS_100MS]);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops duration less than 500ms: %u\n", bkops_stats_p->bkops_dur[BKOPS_500MS]);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops duration less than 1000ms: %u\n", bkops_stats_p->bkops_dur[BKOPS_1000MS]);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops duration less than 2000ms: %u\n", bkops_stats_p->bkops_dur[BKOPS_2000MS]);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops duration less than 5000ms: %u\n", bkops_stats_p->bkops_dur[BKOPS_5000MS]);
	n += snprintf(buf + n, KERNEL_BUF_SIZE - n, "bkops duration great than 5000ms: %u\n", bkops_stats_p->bkops_dur[BKOPS_FOR_AGES]);

	filp->private_data = buf;

	return 0;
}

static ssize_t mmc_bkops_stat_read(struct file *filp, char __user *ubuf,
				size_t cnt, loff_t *ppos)
{
	char *buf = filp->private_data;

	return simple_read_from_buffer(ubuf, cnt, ppos,
				       buf, strlen(buf));
}

static int mmc_bkops_stat_release(struct inode *inode, struct file *file)
{
	kfree(file->private_data);
	return 0;
} /*lint !e715*/

static const struct file_operations mmc_bkops_stat_fops = {
	.open		= mmc_bkops_stat_open,
	.read		= mmc_bkops_stat_read,
	.release	= mmc_bkops_stat_release,
}; /*lint !e785*/

static int hisi_mmc_add_bkops_debugfs(struct mmc_card *card, struct dentry	*bkops_root)
{
	struct dentry	*bkops_test_root = NULL;

	bkops_test_root = debugfs_create_dir("bkops_test_root", bkops_root);
	if (IS_ERR(bkops_test_root))
		goto err;

	if (!debugfs_create_bool("sim_bkops_start_fail", S_IRUSR | S_IWUSR, bkops_test_root, &(card->bkops_debug_ops.sim_bkops_start_fail)))
		goto err;
	if (!debugfs_create_bool("sim_bkops_stop_fail", S_IRUSR | S_IWUSR, bkops_test_root, &(card->bkops_debug_ops.sim_bkops_stop_fail)))
		goto err;
	if (!debugfs_create_bool("sim_bkops_query_fail", S_IRUSR | S_IWUSR, bkops_test_root, &(card->bkops_debug_ops.sim_bkops_query_fail)))
		goto err;
	if (!debugfs_create_bool("sim_critical_bkops", S_IRUSR | S_IWUSR, bkops_test_root, &(card->bkops_debug_ops.sim_critical_bkops)))
		goto err;
	if (!debugfs_create_bool("sim_bkops_abort", S_IRUSR | S_IWUSR, bkops_test_root, &(card->bkops_debug_ops.sim_bkops_abort)))
		goto err;
	if (!debugfs_create_u32("sim_bkops_stop_delay", S_IRUSR | S_IWUSR, bkops_test_root, &(card->bkops_debug_ops.sim_bkops_stop_delay)))
		goto err;
	if (!debugfs_create_bool("skip_bkops_stop", S_IRUSR | S_IWUSR, bkops_test_root, &(card->bkops_debug_ops.skip_bkops_stop)))
		goto err;
	if (!debugfs_create_bool("bypass_bkops", S_IRUSR | S_IWUSR, bkops_test_root, &(card->bkops_debug_ops.bypass_bkops)))
		goto err;

	return 0;
err:
	return -1;
}
#endif /* CONFIG_HISI_MMC_MANUAL_BKOPS */

int hisi_mmc_add_card_debugfs(struct mmc_card *card, struct dentry *root)
{
#ifdef CONFIG_HISI_MMC_MANUAL_BKOPS
	struct dentry	*bkops_root = NULL;

	if (card->ext_csd.man_bkops_en) {
		bkops_root = debugfs_create_dir("bkops_root", root);
		if (IS_ERR(bkops_root))
			return -1;

        if (!debugfs_create_file("bkops_stat", S_IRUSR | S_IWUSR, bkops_root, card, &mmc_bkops_stat_fops))
			return -1;

		if (!debugfs_create_ulong("idle_start_intrvl", S_IRUSR | S_IWUSR, bkops_root, &(mmc_bkops_ops.bkops_idle_start_interval_ms)))
			return -1;

		if (hisi_mmc_add_bkops_debugfs(card, bkops_root))
			return -1;
	}
#endif /* CONFIG_HISI_MMC_MANUAL_BKOPS */

#ifdef CONFIG_HISI_MMC_FLUSH_REDUCE
	if (!debugfs_create_u8("flush_reduce_en", S_IRUSR, root, &(card->host->mmc_flush_reduce_enable)))
		return -1;
#endif

	return 0;

}
/*lint +e421 +e715 +e747*/

