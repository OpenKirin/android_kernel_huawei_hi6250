/*lint -e438 -e501 -e528 -e574 -e578 -e647 -e661 -e666 -e701 -e712 -e713 -e730 -e734 -e737 -e747 -e785 -e826 -e834 -e838*/
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/statfs.h>
#include <linux/vfs.h>
#include <linux/namei.h>
#include "ufs.h"
#include "ufshcd.h"
#include "ufs_health.h"
#include <iotrace/io_monitor.h>
#include <linux/bootdevice.h>

struct device_life_log_struct log_buf;
uint8_t vendor_health_info_buf[QUERY_DESC_HEALTH_MAX_SIZE - DEVICE_HEALTH_VENDOR_PROP_INFO_OFFSET];
uint8_t standard_health_info_buf[DEVICE_HEALTH_VENDOR_PROP_INFO_OFFSET];
struct device_health_info_bigdata big_data;
extern struct ufs_hba *hba_addr;
uint64_t ufs_raw_capacity;

static inline void modify_bits16(uint16_t *p_dest, uint8_t start_bit, uint8_t end_bit, uint16_t value)
{
    uint16_t bits_mask;
    WARN_ON((start_bit > end_bit) || (end_bit >= 16));
    bits_mask  = ((1 << (end_bit - start_bit + 1)) - 1) << start_bit;
    *p_dest &= (~bits_mask);
    *p_dest |= (value << start_bit);
}

static inline void modify_bit8(uint8_t *p_dest, uint8_t bit_pos, uint8_t value)
{
    uint8_t bit_mask;
    WARN_ON(bit_pos >= 8);
    bit_mask = 1 << bit_pos;
    *p_dest &= (~bit_mask);
    *p_dest |= (value << bit_pos);
}


int ufs_get_device_life_info(struct ufs_hba *hba)
{
	int err;
	uint8_t desc_buf[QUERY_DESC_HEALTH_MAX_SIZE];
    struct file *fp;
    uint64_t total_space_100m;
    uint64_t total_free_space_100m;
    mm_segment_t old_fs;
    loff_t pos;

    if (BOOT_DEVICE_UFS != get_bootdevice_type()) {
        printk("[HEALTH_ALERT]EMMC currently not enabled.\n");
        return 0;
    }
   /* /printk("[UFS_HEALTH]phase 0\n"); */
	err = ufshcd_read_device_health_desc(hba, desc_buf, QUERY_DESC_HEALTH_MAX_SIZE);
	if (err) {
		dev_err(hba->dev, "%s: Failed getting device health info\n", __func__);
	    return -1;
    }

    /* printk("Dumping raw health desc buffer...Length:%d bytes\n", QUERY_DESC_HEALTH_MAX_SIZE); */
    /* int i;                                                                                    */
    /* for (i=0; i<QUERY_DESC_HEALTH_MAX_SIZE; i++) {                                            */
    /*     printk("%02x", desc_buf[i]);                                                          */
    /* }                                                                                         */
    /* printk("\n");                                                                             */
    /* Avoid to clear product_name field, which is set during ufs_get_device_info in ufshcd_probe_hba. */
    memset((uint8_t*)&big_data, 0, sizeof(big_data) - sizeof(big_data.product_name));
    big_data.manufacturer_id = hba->manufacturer_id;
    big_data.manufacturer_date = hba->manufacturer_date;

    memset((uint8_t*)&log_buf, 0, sizeof(log_buf));
    fp = NULL;

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    fp = filp_open("/log/device_life.txt", O_RDWR | O_CREAT, 0644);
    if (IS_ERR(fp)) {
        printk("[UFS_HEALTH]Open/Create file error\n");
        return -1;
    }
    pos = 0;
    vfs_read(fp, (char*)&log_buf, sizeof(log_buf), &pos);

    printk("start_time:%ld, end_time:%ld, last_wrtn_data_100m:%llu\n", log_buf.start_time.tv_sec,
           log_buf.end_time.tv_sec, log_buf.last_wrtn_data_100m);
    filp_close(fp, NULL);
    set_fs(old_fs);


    /* /printk("[UFS_HEALTH]phase 1\n"); */
    memcpy(standard_health_info_buf, desc_buf, DEVICE_HEALTH_VENDOR_PROP_INFO_OFFSET);
    ufs_calc_standard_health_info(standard_health_info_buf);

    memcpy(vendor_health_info_buf, &desc_buf[DEVICE_HEALTH_VENDOR_PROP_INFO_OFFSET],
           QUERY_DESC_HEALTH_MAX_SIZE - DEVICE_HEALTH_VENDOR_PROP_INFO_OFFSET);

    /* This is for temporary remain capacity calcualtion. */
    /* When vendor health info ready, following 2 lines should be removed. */
    total_space_100m = 0;
    total_free_space_100m = 0;
    ufs_calc_remain_capacity(&total_space_100m, &total_free_space_100m);
    /* switch (hba->manufacturer_id) {                                                                  */
    /*     case UFS_VENDOR_SAMSUNG:                                                                     */
    /*         ufs_calc_samsung_health_info((struct samsung_device_health_info*)vendor_health_info_buf, */
    /*                                      standard_health_info_buf);                                  */
    /*         break;                                                                                   */
    /*     case UFS_VENDOR_HYNIX:                                                                       */
    /*         ufs_calc_hynix_health_info((struct hynix_device_health_info*)vendor_health_info_buf,     */
    /*                                    standard_health_info_buf);                                    */
    /*         break;                                                                                   */
    /*     case UFS_VENDOR_TOSHIBA:                                                                     */
    /*         ufs_calc_toshiba_health_info((struct toshiba_device_health_info*)vendor_health_info_buf, */
    /*                                      standard_health_info_buf);                                  */
    /*         break;                                                                                   */
    /*     default:                                                                                     */
    /*         dev_err(hba->dev, "unknown ufs manufacturer id\n");                                      */
    /*         break;                                                                                   */
    /* }                                                                                                */
    /* /printk("[UFS_HEALTH]phase 2\n"); */
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open("/log/device_life.txt", O_RDWR, 0644);
    if (IS_ERR(fp)) {
        printk("Open file error\n");
        return -1;
    }
    pos = 0;
    vfs_write(fp, (char*)&log_buf, sizeof(log_buf), &pos);

    filp_close(fp, NULL);
    set_fs(old_fs);

    return 0;
}

void ufs_calc_standard_health_info(uint8_t *standard_health_info)
{
	uint8_t pre_eol_info;
	uint8_t life_time_est_typ_a;
	uint8_t life_time_est_typ_b;
    struct timeval time;
    uint64_t cur_time_sec;
    uint64_t cur_time_days;
    uint32_t fx;

	pre_eol_info = standard_health_info[HEALTH_DEVICE_DESC_PARAM_PREEOL];
	life_time_est_typ_a = standard_health_info[HEALTH_DEVICE_DESC_PARAM_LIFETIMEA];
	life_time_est_typ_b = standard_health_info[HEALTH_DEVICE_DESC_PARAM_LIFETIMEB];

    printk("Calc standard health info: read pre_eol_info:%u, life_time_est_typ_a:%u, life_time_est_typ_b:%u\n",
           pre_eol_info, life_time_est_typ_a, life_time_est_typ_b);
    big_data.bPreEOLInfo = pre_eol_info;
    big_data.bDeviceLifeTimeEstA = life_time_est_typ_a;
    big_data.bDeviceLifeTimeEstB = life_time_est_typ_b;

    /* rtc_time tm; */

    do_gettimeofday(&time);
    cur_time_sec = (uint64_t)(time.tv_sec - log_buf.start_time.tv_sec);
    cur_time_days = SECONDS_TO_DAYS(cur_time_sec);
    printk("Get time. Start time:%ld, Current time:%ld, cur_time_sec:%llu, cur_time_days:%llu\n",
           log_buf.end_time.tv_sec, time.tv_sec, cur_time_sec, cur_time_days);
    memcpy(&log_buf.start_time, &log_buf.end_time, sizeof(struct timeval));
    memcpy(&log_buf.end_time, &time, sizeof(struct timeval));
    /* rtc_time_to_tm(local_time, &tm); */
    if (log_buf.start_time.tv_sec == 0) {
        big_data.used_days = 0;
    }
    else {
        big_data.used_days = cur_time_days;
    }

    if ((pre_eol_info == PRE_EOL_INFO_WARNING) && \
        (cur_time_days < 2 * DAYS_OF_YEAR)) {
        modify_bits16(&big_data.alarm_lvl, 0, 1, ALARM_YELLOW);
        modify_bit8(&big_data.alarm_src, 0, 1);
    }
    else if (pre_eol_info == PRE_EOL_INFO_CRITICAL) {
        if (cur_time_days < 2 * DAYS_OF_YEAR) {
            modify_bits16(&big_data.alarm_lvl, 0, 1, ALARM_ORANGE);
            modify_bit8(&big_data.alarm_src, 0, 1);
        }
        else if (cur_time_days < DAYS_OF_YEAR) {
            modify_bits16(&big_data.alarm_lvl, 0, 1, ALARM_RED);
            modify_bit8(&big_data.alarm_src, 0, 1);
        }
    }

    fx = LIFE_TIME_EST(life_time_est_typ_a);
    if ((100 / fx * cur_time_days < 2 * DAYS_OF_YEAR) && \
        ((life_time_est_typ_a >= LIFE_TIME_CONVINCE_THRESHOLD_LOW) || \
        (life_time_est_typ_b >= LIFE_TIME_CONVINCE_THRESHOLD_LOW))) {
            modify_bits16(&big_data.alarm_lvl, 2, 3, ALARM_YELLOW);
            modify_bit8(&big_data.alarm_src, 1, 1);
    }
    else if (100 / fx * cur_time_days < DAYS_OF_YEAR) {
        if ((life_time_est_typ_a >= LIFE_TIME_CONVINCE_THRESHOLD_HIGH) || \
            (life_time_est_typ_b >= LIFE_TIME_CONVINCE_THRESHOLD_HIGH)) {
                modify_bits16(&big_data.alarm_lvl, 2, 3, ALARM_RED);
                modify_bit8(&big_data.alarm_src, 1, 1);
        }
        else if ((life_time_est_typ_a >= LIFE_TIME_CONVINCE_THRESHOLD_LOW) || \
                (life_time_est_typ_b >= LIFE_TIME_CONVINCE_THRESHOLD_LOW)) {
                    modify_bits16(&big_data.alarm_lvl, 2, 3, ALARM_ORANGE);
                    modify_bit8(&big_data.alarm_src, 1, 1);
        }
    }
    printk("alarm level:%x, alarm source:%x\n", big_data.alarm_lvl, big_data.alarm_src);
}

void ufs_set_product_name(char *product_name, uint32_t len)
{
    uint32_t min_len;
    min_len = min_t(uint8_t, len, MAX_MODEL_NAME_LEN);

	strlcpy(big_data.product_name, product_name, min_len);
	big_data.product_name[min_len] = '\0';
}

void ufs_set_raw_capacity(uint64_t capacity)
{
    ufs_raw_capacity = capacity;
}

#define PARTITION_CNT           5
#define SECTOR_SIZE             512
#define FW_META_SIZE_100MB      3
int ufs_calc_remain_capacity(uint64_t *p_total_space, uint64_t *p_total_free_space)
{
    char p1[] = "/system";
    char p2[] = "/splash2";
    char p3[] = "/cust";
    char p4[] = "/data";
    char p5[] = "/cache";
    char *dev_name[PARTITION_CNT] = {
        p1,
        p2,
        p3,
        p4,
        p5,
    };
    int ret_val = 0;
    struct kstatfs st;
    struct path partition_path;
    int i, err;

    for (i = 0; i < PARTITION_CNT; i++) {
        err = kern_path(dev_name[i], LOOKUP_FOLLOW | LOOKUP_DIRECTORY, &partition_path);
        if (err) {
            printk("[UFS_HEALTH]Err parsing path dev_name[%d]:/%c%c%c%c\n", i, dev_name[i][1], dev_name[i][2], dev_name[i][3], dev_name[i][4]);
            ret_val = -1;
            continue;
        }
        err = vfs_statfs(&partition_path, &st);
        if (err) {
            ret_val = -1;
            printk("[UFS_HEALTH]Err getting statfs for dev_name[%d]:/%c%c%c%c\n", i, dev_name[i][1], dev_name[i][2], dev_name[i][3], dev_name[i][4]);
        }
        else {
            printk("st.f_blocks:%llu, st.f_bfree:%llu, st.f_bsize:%ld\n", st.f_blocks, st.f_bfree, st.f_bsize);
            *p_total_space += (uint64_t)st.f_blocks * st.f_bsize;
            *p_total_free_space += (uint64_t)st.f_bfree * st.f_bsize;
        }
    }
    /* Calculate size in 100MB */
    /* *p_total_space = *p_total_space / (1024 * 1024 * 100); */
    /* Use raw device capacity. */
    *p_total_space = (ufs_raw_capacity * SECTOR_SIZE) / (1024 * 1024 * 100);
    *p_total_free_space = *p_total_free_space / (1024 * 1024 * 100);
    big_data.used_capacity = *p_total_space - *p_total_free_space + FW_META_SIZE_100MB;
    printk("Calc remain capacity, total space:%llu, total free space:%llu\n", *p_total_space, *p_total_free_space);
    return ret_val;
}

#ifdef CONFIG_HUAWEI_UFS_HEALTH_INFO
static int ufs_health_get_log(unsigned char *buff, int len)
{
    int ret = 0;
    uint32_t big_data_size = sizeof(struct device_health_info_bigdata);

    ufs_get_device_life_info(hba_addr);

    if(len > big_data_size)
    {
        memcpy(buff, (uint8_t*)&big_data, big_data_size);
        ret = big_data_size;
    }

    return ret;
}
static int ufs_health_set_param(struct imonitor_eventobj *obj, unsigned char *buff)
{
    int ret = 0;
    struct device_health_info_bigdata *p_big_data;
    p_big_data = (struct device_health_info_bigdata *)buff;

    if (BOOT_DEVICE_UFS != get_bootdevice_type()) {
        return -1;
    }

    ret = ret | imonitor_set_param(obj, E914007000_ALL_LEFT_SPACE_INT, p_big_data->used_capacity);
    ret = ret | imonitor_set_param(obj, E914007000_BPREEOL_INFO_SMALLINT, p_big_data->bPreEOLInfo);
    ret = ret | imonitor_set_param(obj, E914007000_BDEVICELIFE_TIME_A_SMALLINT, p_big_data->bDeviceLifeTimeEstA);
    ret = ret | imonitor_set_param(obj, E914007000_BDEVICELIFE_TIME_B_SMALLINT, p_big_data->bDeviceLifeTimeEstB);
    ret = ret | imonitor_set_param(obj, E914007000_USED_DAYS_INT, p_big_data->used_days);
    ret = ret | imonitor_set_param(obj, E914007000_VENDOR_ID_INT, p_big_data->manufacturer_id);
    ret = ret | imonitor_set_param(obj, E914007000_PRODUCT_NAME_VARCHAR, (long)(p_big_data->product_name));

    return 0;
}
#ifdef CONFIG_HUAWEI_IO_TRACING
int dev_report_to_iotrace(unsigned char *buff, int len)
{
    struct dev_to_iotrace{
        uint16_t    manufacturer_id;
        uint16_t    manufacturer_date;
        char        product_name[MAX_MODEL_NAME_LEN];
    };
    struct dev_to_iotrace *dev = (struct dev_to_iotrace *)buff;
    int ret = sizeof(struct dev_to_iotrace);
    if(ret > len)
        return 0;

    dev->manufacturer_id = big_data.manufacturer_id;
    dev->manufacturer_date = big_data.manufacturer_date;
    memcpy(dev->product_name, big_data.product_name, MAX_MODEL_NAME_LEN);

    return ret;
}
EXPORT_SYMBOL(dev_report_to_iotrace);
#endif

static struct io_module_template ufs_health_io = {
    .mod_id = IO_MONITOR_UFS_HEALTH,
    .event_id = 914007000,
    .base_interval = 24 * 3600000, /*one day*/
    .ops = {
        .log_record = ufs_health_get_log,
        .log_set_param = ufs_health_set_param,
        .log_upload = NULL,
    },
};

static int __init ufs_health_init(void)
{
    /* Register DMD entity, etc. */
    io_monitor_mod_register(IO_MONITOR_UFS_HEALTH, &ufs_health_io);
    return 0;
}
/*lint +e438 +e501 +e528 +e574 +e578 +e647 +e661 +e666 +e701 +e712 +e713 +e730 +e734 +e737 +e747 +e785 +e826 +e834 +e838*/
/*lint -e528 -esym(528,*)*/
subsys_initcall(ufs_health_init);
/*lint -e528 +esym(528,*)*/
#endif
