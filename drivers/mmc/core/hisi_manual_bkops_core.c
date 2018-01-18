#include <linux/blkdev.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include "mmc_ops.h"
#include "core.h"
#include "../card/queue.h"
/*lint -e730 -e747*/
static int mmc_bkops_status_query(struct request_queue *q, int *status)
{
	int err;
	struct mmc_queue *mq = q->queuedata;
	struct mmc_card *card = mq->card;
	struct mmc_bkops_stats *bkops_stats_p = &(card->bkops_stats);
	u64 start_time, stop_time;
	u64 time_interval;

	BUG_ON(!card);

	start_time = ktime_get_ns();
	mmc_get_card(card);
	if (mmc_card_suspended(card->host->card)) {
		err = -1;
		goto put_card;
	}

	bkops_stats_p->bkops_query_count++;
#ifdef CONFIG_HISI_DEBUG_FS
	if (card->bkops_debug_ops.bypass_bkops) {
		err = 0;
		*status = 0;
		goto put_card;
	}
#endif

	err = mmc_read_bkops_status(card);
#ifdef CONFIG_HISI_DEBUG_FS
	if (card->bkops_debug_ops.sim_critical_bkops) {
		card->ext_csd.raw_bkops_status = 2;
		card->bkops_debug_ops.sim_critical_bkops = 0;
	}
	if (card->bkops_debug_ops.sim_bkops_query_fail) {
		pr_err("simulate bkops query failure!\n");
		card->bkops_debug_ops.sim_bkops_query_fail = false;
		err = -1;
	}
#endif
	if (err) {
		pr_err("%s: Failed to read bkops status: %d\n",
		       mmc_hostname(card->host), err);
#ifdef CONFIG_HISI_DEBUG_FS
		BUG_ON(1);
#else
		bkops_stats_p->bkops_query_fail_count++;
		*status = 0;
		goto put_card;
#endif
	}

	*status = card->ext_csd.raw_bkops_status;
	bkops_stats_p->bkops_status[*status]++;

put_card:
	mmc_put_card(card);
	if (!err) {
		stop_time = ktime_get_ns();
		time_interval = stop_time - start_time;
		if ( time_interval > bkops_stats_p->bkops_max_query_time) {
			bkops_stats_p->bkops_max_query_time = time_interval;
		}
		bkops_stats_p->bkops_avrg_query_time =
		((bkops_stats_p->bkops_avrg_query_time * (bkops_stats_p->bkops_query_count - 1)) + time_interval) / bkops_stats_p->bkops_query_count;
	}

	/* always return sucess */
	return 0;
}

int mmc_start_bkops(struct mmc_card *card, bool from_exception)
{
	int err;
	u64 start_time;
	u64 stop_time;
	u64 time_interval;
	struct mmc_bkops_stats *bkops_stats_p = &(card->bkops_stats);

	start_time = ktime_get_ns();
	if (!card->ext_csd.man_bkops_en || mmc_card_doing_bkops(card))
		return 0;

	err = __mmc_switch(card, EXT_CSD_CMD_SET_NORMAL,
			EXT_CSD_BKOPS_START, 1, 0,
			false, true, false);
#ifdef CONFIG_HISI_DEBUG_FS
	if (card->bkops_debug_ops.sim_bkops_start_fail) {
		pr_err("simulate bkops start failure!\n");
		card->bkops_debug_ops.sim_bkops_start_fail = false;
		err = -1;
	}
#endif
	if (err) {
		pr_err("%s: Error %d starting bkops\n",
			mmc_hostname(card->host), err);
#ifdef CONFIG_HISI_DEBUG_FS
		BUG_ON(1);
#else
		card->bkops_stats.bkops_start_fail_count++;
		goto out;
#endif
	}
	mmc_card_set_doing_bkops(card);
	card->bkops_stats.bkops_count++;
	card->bkops_stats.bkops_start_time = jiffies;
	card->bkops_stats.bkops_ongoing = 1;
	pr_info("BKOPS started.\n");
	stop_time = ktime_get_ns();
	time_interval = stop_time - start_time;
	if (time_interval > bkops_stats_p->bkops_max_start_time)
		bkops_stats_p->bkops_max_start_time = time_interval;
	bkops_stats_p->bkops_avrg_start_time =
	((bkops_stats_p->bkops_avrg_start_time * (bkops_stats_p->bkops_count - 1)) + time_interval) / bkops_stats_p->bkops_count;

#ifdef CONFIG_HISI_DEBUG_FS
	if (card->bkops_debug_ops.sim_bkops_abort)
		mmc_stop_bkops(card);
#endif

#ifndef CONFIG_HISI_DEBUG_FS
out:
#endif
	return err;
} /*lint !e715*/

static void hisi_mmc_send_hpi(struct mmc_card *card)
{
	int err;
	int retry_count = 4;
#ifdef CONFIG_HISI_DEBUG_FS
	struct mmc_bkops_debug_ops *bkops_debug_ops_p = &(card->bkops_debug_ops);
#endif
	struct mmc_host *host = card->host;

#ifdef CONFIG_HISI_DEBUG_FS
	if (bkops_debug_ops_p->sim_bkops_stop_fail) {
		pr_err("simulate bkops stop failure!\n");
		err = -1;
		goto sim_bkops_stop_fail;
	}
#endif

	do {
		err = mmc_interrupt_hpi(card);
		if (!err || (err == -EINVAL))
			break;
	} while (retry_count--);
#ifdef CONFIG_HISI_DEBUG_FS
sim_bkops_stop_fail:
#endif
	if ((retry_count <= 0) || (err && (err != -EINVAL))) {
		pr_err("%s %d HPI failed! do hisi_mmc_reset\n", __func__, __LINE__);
#ifdef CONFIG_HISI_DEBUG_FS
		if (bkops_debug_ops_p->sim_bkops_stop_fail == 0)
			dump_stack();
#endif
		card->bkops_stats.bkops_stop_fail_count++;
		if (!(host->bus_ops->reset) || host->bus_ops->reset(host)) {
			pr_err("%s %d hisi_mmc_reset Failed!\n", __func__, __LINE__);
			dump_stack();
		}
	}
}

static void hisi_mmc_bkops_update_dur(struct mmc_bkops_stats *bkops_stats_p)
{
	unsigned long bkops_duration;
	unsigned int bkops_dur_msecs;

	bkops_duration = jiffies - bkops_stats_p->bkops_start_time;
	bkops_stats_p->hpi_count++;
	if (bkops_duration > bkops_stats_p->max_bkops_duration) {
		bkops_stats_p->max_bkops_duration = bkops_duration;
	}
	bkops_dur_msecs = jiffies_to_msecs(bkops_duration);
	if (bkops_dur_msecs < 100)
		bkops_stats_p->bkops_dur[BKOPS_100MS]++;
	else if (bkops_dur_msecs < 500)
		bkops_stats_p->bkops_dur[BKOPS_500MS]++;
	else if (bkops_dur_msecs < 1000)
		bkops_stats_p->bkops_dur[BKOPS_1000MS]++;
	else if (bkops_dur_msecs < 2000)
		bkops_stats_p->bkops_dur[BKOPS_2000MS]++;
	else if (bkops_dur_msecs < 5000)
		bkops_stats_p->bkops_dur[BKOPS_5000MS]++;
	else
		bkops_stats_p->bkops_dur[BKOPS_FOR_AGES]++;
}

/**
 *	mmc_stop_bkops - stop ongoing BKOPS
 *	@card: MMC card to check BKOPS
 *
 *	Send HPI command to stop ongoing background operations to
 *	allow rapid servicing of foreground operations, e.g. read/
 *	writes. Wait until the card comes out of the programming state
 *	to avoid errors in servicing read/write requests.
 */
int mmc_stop_bkops(struct mmc_card *card)
{
	u64 start_time;
	u64 stop_time;
	u64 time_interval;
	struct mmc_bkops_stats *bkops_stats_p = &(card->bkops_stats);
#ifdef CONFIG_HISI_DEBUG_FS
	struct mmc_bkops_debug_ops *bkops_debug_ops_p = &(card->bkops_debug_ops);
#endif
	struct mmc_host *host = card->host;

	start_time = ktime_get_ns();
	if (!card->ext_csd.man_bkops_en || !mmc_card_doing_bkops(card))
		return 0;

#ifdef CONFIG_HISI_DEBUG_FS
	if (bkops_debug_ops_p->skip_bkops_stop) {
		pr_err("stop bkops was skipped!\n");
		bkops_debug_ops_p->skip_bkops_stop = false;
		mmc_card_clr_doing_bkops(card);
		return 0;
	}

	if ( (host->ops->card_busy && host->ops->card_busy(host)) || card->bkops_debug_ops.sim_bkops_abort) {
		bkops_stats_p->bkops_abort_count++;
		pr_err("%s ongoing bkops aborted\n", __func__);
	}
#endif

	hisi_mmc_send_hpi(card);

	mmc_card_clr_doing_bkops(card);
	card->bkops_stats.bkops_ongoing = 0;
	/* update BKOPS stats */
	hisi_mmc_bkops_update_dur(bkops_stats_p);

#ifdef CONFIG_HISI_DEBUG_FS
	if (bkops_debug_ops_p->sim_bkops_stop_delay) {
		pr_err("simulate bkops stop delay %dms\n", bkops_debug_ops_p->sim_bkops_stop_delay);
		msleep(bkops_debug_ops_p->sim_bkops_stop_delay);
	}
#endif

	pr_info("BKOPS stoped.\n");
	stop_time = ktime_get_ns();
	time_interval = stop_time - start_time;
	if (time_interval > bkops_stats_p->bkops_max_stop_time)
		bkops_stats_p->bkops_max_stop_time = time_interval;
	bkops_stats_p->bkops_avrg_stop_time =
	((bkops_stats_p->bkops_avrg_stop_time * (bkops_stats_p->bkops_count - 1)) + time_interval) / bkops_stats_p->bkops_count;


	/* stop BKOPS can't fail, we always return 0 */
	return 0;
}

static int mmc_bkops_start_stop(struct request_queue *q, int start)
{
	int ret;
	struct mmc_queue *mq = q->queuedata;
	struct mmc_card *card = mq->card;

	BUG_ON(!card);
	mmc_get_card(card);
	if (mmc_card_suspended(card->host->card)) {
		ret = -1;
		goto put_card;
	}
	if (BLK_BKOPS_STOP == start) {
		ret = mmc_stop_bkops(card);
	} else if (BLK_BKOPS_START == start) {
		ret = mmc_start_bkops(card, (bool)false);
	} else {
		pr_err("%s Invalid start value: %d\n", __func__, start);
		ret = -1;
	}
put_card:
	mmc_put_card(card);

	/* always return success */
	return 0;
}

struct blk_bkops_ops mmc_bkops_ops = {
	.bkops_idle_start_interval_ms = 2000,
	.bkops_start_stop = mmc_bkops_start_stop,
	.bkops_status_query = mmc_bkops_status_query,
};

/*  first 9 Bytes of CID of Micron 2D eMMC that needs BKOPS
	0x13014e51334a393756
	0x13014e51334a393656
	0x13014e51334a393652
*/
#define MICRON_2D_CID_MASK_LEN	9
static u8 micron_mmc_bkops_cid1[MICRON_2D_CID_MASK_LEN] = {0x13, 0x01, 0x4e, 0x51, 0x33, 0x4a, 0x39, 0x37, 0x56};
static u8 micron_mmc_bkops_cid2[MICRON_2D_CID_MASK_LEN] = {0x13, 0x01, 0x4e, 0x51, 0x33, 0x4a, 0x39, 0x36, 0x56};
static u8 micron_mmc_bkops_cid3[MICRON_2D_CID_MASK_LEN] = {0x13, 0x01, 0x4e, 0x51, 0x33, 0x4a, 0x39, 0x36, 0x52};
bool hisi_mmc_is_bkops_needed(struct mmc_card *card)
{
	char converted_cid[12];

	/* convert ending */
	((int *)converted_cid)[0] = be32_to_cpu(card->raw_cid[0]);
	((int *)converted_cid)[1] = be32_to_cpu(card->raw_cid[1]);
	((int *)converted_cid)[2] = be32_to_cpu(card->raw_cid[2]);

	return !memcmp(converted_cid, micron_mmc_bkops_cid1, MICRON_2D_CID_MASK_LEN) ||
		   !memcmp(converted_cid, micron_mmc_bkops_cid2, MICRON_2D_CID_MASK_LEN) ||
		   !memcmp(converted_cid, micron_mmc_bkops_cid3, MICRON_2D_CID_MASK_LEN);
}

void hisi_mmc_enable_hpi_for_micron(struct mmc_card *card, bool *broken_hpi)
{
	if (hisi_mmc_is_bkops_needed(card))
		*broken_hpi = 0;
}
/*lint +e730 +e747*/

