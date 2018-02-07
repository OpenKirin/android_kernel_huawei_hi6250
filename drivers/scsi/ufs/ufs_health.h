#ifndef _UFS_HEALTH_H_
#define _UFS_HEALTH_H_

#include <linux/time.h>
/*
 * This part illustrates device health info -- vendor prop related info.
 */
#define DEVICE_HEALTH_VENDOR_PROP_INFO_OFFSET       5
#define DEVICE_HEALTH_VENDOR_PROP_INFO_SIZE_BYTE    32
#define DEVICE_HEALTH_VENDOR_PROP_INFO_ALIGN_SIZE   3

#define DEVICE_HEALTH_ALARM_COUNT   4
/* In ufs_get_device_info, the max model length is set to 32 without '\0',
   so round up to 36 here. */
#define MAX_MODEL_NAME_LEN          36
struct device_health_info_bigdata {
    uint16_t    exp_life_left;
    uint16_t    alarm_lvl;
    uint8_t     alarm_src;
    uint8_t     bPreEOLInfo;
    uint8_t     bDeviceLifeTimeEstA;
    uint8_t     bDeviceLifeTimeEstB;
    uint32_t    runtime_bad_blk_cnt;
    uint16_t    max_mlc_pec;
    uint16_t    min_mlc_pec;
    uint16_t    avg_mlc_pec;
    uint16_t    max_slc_pec;
    uint16_t    min_slc_pec;
    uint16_t    avg_slc_pec;
    uint32_t    cur_wrtn_data;
    uint32_t    used_capacity;
    uint32_t    write_ampl;
    uint32_t    used_days;
    uint32_t    used_life;
    uint16_t    manufacturer_id;
    uint16_t    manufacturer_date;
    char        product_name[MAX_MODEL_NAME_LEN];
};

enum {
    ALARM_NONE      = 0,
    ALARM_YELLOW    = 1,
    ALARM_ORANGE    = 2,
    ALARM_RED       = 3,
};

enum {
    PRE_EOL_INFO_UNDEFINED = 0,
    PRE_EOL_INFO_NORMAL = 1,
    PRE_EOL_INFO_WARNING = 2,
    PRE_EOL_INFO_CRITICAL = 3,
};

enum {
    DEVICE_LIFE_TIME_UNAVAILABLE = 0,
    DEVICE_LIFE_TIME_0_10_PERCENT = 1,
    DEVICE_LIFE_TIME_10_20_PERCENT = 2,
    DEVICE_LIFE_TIME_20_30_PERCENT = 3,
    DEVICE_LIFE_TIME_30_40_PERCENT = 4,
    DEVICE_LIFE_TIME_40_50_PERCENT = 5,
    DEVICE_LIFE_TIME_50_60_PERCENT = 6,
    DEVICE_LIFE_TIME_60_70_PERCENT = 7,
    DEVICE_LIFE_TIME_70_80_PERCENT = 8,
    DEVICE_LIFE_TIME_80_90_PERCENT = 9,
    DEVICE_LIFE_TIME_90_100_PERCENT = 10,
};

#define TLC_3D_EXPECTED_PEC     1500
#define MLC_2D_3D_EXPECTED_PEC  3000
#define SLC_EXPECTED_PEC        100000

struct device_life_log_struct {
    struct timeval   start_time;
    struct timeval   end_time;
    uint64_t         last_wrtn_data_100m;
};

#define SECONDS_TO_DAYS(seconds)    ((seconds)/86400)
#define DAYS_OF_YEAR                365
#define LIFE_TIME_EST(x)    ((x) * 10 - 5)
#define LIFE_TIME_CONVINCE_THRESHOLD_LOW    4
#define LIFE_TIME_CONVINCE_THRESHOLD_HIGH   6


#ifdef CONFIG_HUAWEI_UFS_HEALTH_INFO
int ufs_get_device_life_info(struct ufs_hba *hba);
void ufs_calc_standard_health_info(uint8_t *standard_health_info);
void ufs_set_product_name(char *product_name, uint32_t len);
void ufs_set_raw_capacity(uint64_t capacity);
int ufs_calc_remain_capacity(uint64_t *p_total_space, uint64_t *p_total_free_space);
int ufs_health_bigdata_output(unsigned char *buff, int len);
#else
int ufs_get_device_life_info(struct ufs_hba *hba) {return -1;}
void ufs_calc_standard_health_info(uint8_t *standard_health_info) {return;}
void ufs_set_product_name(char *product_name, uint32_t len) {return;}
void ufs_set_raw_capacity(uint64_t capacity) {return;}
int ufs_calc_remain_capacity(uint64_t *p_total_space, uint64_t *p_total_free_space) {return -1;}
int ufs_health_bigdata_output(unsigned char *buff, int len) {return -1;}
int dev_report_to_iotrace(unsigned char *buff, int len) {return 0;}
#endif

#endif
