#ifndef _INTERNAL_HISI_BLK_MQ_H_
#define _INTERNAL_HISI_BLK_MQ_H_

#include <linux/blkdev.h>
#include <linux/hisi-blk-mq.h>

#ifdef CONFIG_HISI_BLK_MQ

#define MQ_MERGE_MAX_SIZE 0x40000

static inline bool flush_insert(struct request_queue *q, struct bio *bio)
{
	if((bio->bi_rw & REQ_FLUSH)&&(bio->bi_iter.bi_size == 0)){
		if(atomic_read(&q->wio_after_flush_fua) == 0)
			return false;
	}
	return true;
}

static inline void flush_reducing_stats_update(struct request_queue *q,
		struct request *rq, struct request *processing_rq)
{
	if (processing_rq->cmd_flags & REQ_FLUSH) {
		atomic_set(&q->wio_after_flush_fua, 0);
	} else if ((processing_rq->cmd_type == REQ_TYPE_FS)
			   && (processing_rq->cmd_flags & REQ_WRITE) && rq->__data_len
			   && (atomic_read(&q->wio_after_flush_fua) == 0)) {
		atomic_set(&q->wio_after_flush_fua, 1);
	}
}

static inline void hisi_blk_mq_init(struct request_queue *q)
{
	atomic_set(&q->wio_after_flush_fua,0);
}

#else /* CONFIG_HISI_BLK_MQ */

static inline void hisi_blk_mq_init(struct request_queue *q){}

static inline bool flush_insert(struct request_queue *q, struct bio *bio)
{
	return true;
}

static inline void flush_reducing_stats_update(struct request_queue *q,
		struct request *rq, struct request *processing_rq) {}

#endif /* CONFIG_HISI_BLK_MQ */

#endif /* _INTERNAL_HISI_BLK_MQ_H_ */
