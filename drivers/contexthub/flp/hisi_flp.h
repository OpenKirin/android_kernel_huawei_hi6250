/*
 * hisi flp driver.
 *
 * Copyright (C) 2015 huawei Ltd.
 * Author: lijiangxiong <lijiangxiong@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*ioctrl cmd type*/
#define FLP_TAG_FLP         0
#define FLP_TAG_GPS         1
#define FLP_TAG_AR          2

#define FLP_GPS_BATCHING_MAX_SIZE   50
#define FLP_GEOFENCE_MAX_NUM        48

#define FLP_IOCTL_CMD_MASK              (0xFFFF00FF)

#define FLP_IOCTL_TAG_MASK              (0xFFFFFF00)
#define FLP_IOCTL_TAG_FLP               (0x464C0000)
#define FLP_IOCTL_TAG_GPS               (0x464C0100)
#define FLP_IOCTL_TAG_AR                (0x464C0200)
#define FLP_IOCTL_TAG_COMMON            (0x464CFF00)

#define FLP_IOCTL_TYPE_MASK             (0xFFFF00F0)
#define FLP_IOCTL_TYPE_PDR              (0x464C0000)
#define FLP_IOCTL_TYPE_AR               (0x464C0010)
#define FLP_IOCTL_TYPE_GEOFENCE         (0x464C0020)
#define FLP_IOCTL_TYPE_BATCHING         (0x464C0030)
#define FLP_IOCTL_TYPE_ENV		(0x464C0040)
#define FLP_IOCTL_TYPE_COMMON           (0x464C00F0)

#define FLP_IOCTL_PDR_START(x)          (0x464C0000 + ((x) * 0x100) + 1)
#define FLP_IOCTL_PDR_STOP(x)           (0x464C0000 + ((x) * 0x100) + 2)
#define FLP_IOCTL_PDR_READ(x)           (0x464C0000 + ((x) * 0x100) + 3)
#define FLP_IOCTL_PDR_WRITE(x)          (0x464C0000 + ((x) * 0x100) + 4)
#define FLP_IOCTL_PDR_CONFIG(x)         (0x464C0000 + ((x) * 0x100) + 5)
#define FLP_IOCTL_PDR_UPDATE(x)         (0x464C0000 + ((x) * 0x100) + 6)
#define FLP_IOCTL_PDR_FLUSH(x)          (0x464C0000 + ((x) * 0x100) + 7)
#define FLP_IOCTL_PDR_STEP_CFG(x)       (0x464C0000 + ((x) * 0x100) + 8)

#define FLP_IOCTL_AR_START(x)           (0x464C0000 + ((x) * 0x100) + 0x11)
#define FLP_IOCTL_AR_STOP(x)            (0x464C0000 + ((x) * 0x100) + 0x12)
#define FLP_IOCTL_AR_READ(x)            (0x464C0000 + ((x) * 0x100) + 0x13)
#define FLP_IOCTL_AR_UPDATE(x)          (0x464C0000 + ((x) * 0x100) + 0x15)
#define FLP_IOCTL_AR_FLUSH(x)           (0x464C0000 + ((x) * 0x100) + 0x16)
#define FLP_IOCTL_AR_STATE(x)           (0x464C0000 + ((x) * 0x100) + 0x17)
#define FLP_IOCTL_AR_CONFIG(x)           (0x464C0000 + ((x) * 0x100) + 0x18)
#define FLP_IOCTL_AR_STATE_V2(x)           (0x464C0000 + ((x) * 0x100) + 0x1D)

#define FLP_IOCTL_ENV_CONFIG(x)           (0x464C0000 + ((x) * 0x100) + 0x41)
#define FLP_IOCTL_ENV_STOP(x)            (0x464C0000 + ((x) * 0x100) + 0x42)
#define FLP_IOCTL_ENV_READ(x)            (0x464C0000 + ((x) * 0x100) + 0x43)
#define FLP_IOCTL_ENV_FLUSH(x)           (0x464C0000 + ((x) * 0x100) + 0x46)
#define FLP_IOCTL_ENV_STATE(x)           (0x464C0000 + ((x) * 0x100) + 0x47)
#define FLP_IOCTL_ENV_INIT(x)		(0x464C0000 + ((x) * 0x100) + 0x48)
#define FLP_IOCTL_ENV_EXIT(x)		(0x464C0000 + ((x) * 0x100) + 0x49)


#define FLP_IOCTL_GEOFENCE_ADD                  (0x464C0000 + 0x21)
#define FLP_IOCTL_GEOFENCE_REMOVE               (0x464C0000 + 0x22)
#define FLP_IOCTL_GEOFENCE_MODIFY               (0x464C0000 + 0x23)

#define FLP_IOCTL_BATCHING_START                (0x464C0000 + 0x31)
#define FLP_IOCTL_BATCHING_STOP                 (0x464C0000 + 0x32)
#define FLP_IOCTL_BATCHING_UPDATE               (0x464C0000 + 0x33)
#define FLP_IOCTL_BATCHING_CLEANUP              (0x464C0000 + 0x34)
#define FLP_IOCTL_BATCHING_LASTLOCATION         (0x464C0000 + 0x35)
#define FLP_IOCTL_BATCHING_FLUSH                (0x464C0000 + 0x36)
#define FLP_IOCTL_BATCHING_INJECT               (0x464C0000 + 0x37)
#define FLP_IOCTL_BATCHING_GET_SIZE             (0x464C0000 + 0x38)
#define FLP_IOCTL_COMMON_HW_RESET               (0x464C0000 + 0x39)

/*common ioctrl*/
#define FLP_IOCTL_COMMON_GET_UTC            (0x464C0000 + 0xFFF1)
#define FLP_IOCTL_COMMON_SLEEP              (0x464C0000 + 0xFFF2)
#define FLP_IOCTL_COMMON_AWAKE_RET          (0x464C0000 + 0xFFF3)
#define FLP_IOCTL_COMMON_SETPID             (0x464C0000 + 0xFFF4)
#define FLP_IOCTL_COMMON_CLOSE_SERVICE      (0x464C0000 + 0xFFF5)
#define FLP_IOCTL_COMMON_HOLD_WAKELOCK      (0x464C0000 + 0xFFF6)
#define FLP_IOCTL_COMMON_RELEASE_WAKELOCK   (0x464C0000 + 0xFFF7)

enum {
    AR_ACTIVITY_VEHICLE         = 0x00,
    AR_ACTIVITY_RIDING          = 0x01,
    AR_ACTIVITY_WALK_SLOW       = 0x02,
    AR_ACTIVITY_RUN_FAST        = 0x03,
    AR_ACTIVITY_STATIONARY      = 0x04,
    AR_ACTIVITY_TILT            = 0x05,
    AR_ACTIVITY_END             = 0x10,
    AR_VE_BUS                   = 0x11, /* ��� */
    AR_VE_CAR                   = 0x12, /* С�� */
    AR_VE_METRO                 = 0x13, /* ���� */
    AR_VE_HIGH_SPEED_RAIL       = 0x14, /* ���� */
    AR_VE_AUTO                  = 0x15, /* ��·��ͨ */
    AR_VE_RAIL                  = 0x16, /* ��·��ͨ */
    AR_CLIMBING_MOUNT           = 0x17, /* ��ɽ*/
    AR_FAST_WALK                = 0x18, /* ����*/
    AR_UNKNOWN                  = 0x3F,
    AR_STATE_BUTT               = 0x40,
};

typedef enum ar_environment_type {
	AR_ENVIRONMENT_HOME,
	AR_ENVIRONMENT_OFFICE,
	AR_ENVIRONMENT_STATION,
	AR_ENVIRONMENT_TYPE_UNKNOWN,
	AR_ENVIRONMENT_END =0x20
} ar_environment_type_t;

#define CONTEXT_TYPE_MAX (0x40)/*MAX(AR_STATE_BUTT, AR_ENVIRONMENT_END)*/

enum {
	AR_STATE_ENTER = 1,
	AR_STATE_EXIT,
	AR_STATE_ENTER_EXIT = 3,
	AR_STATE_MAX
};

enum {
    FLP_SERVICE_RESET,
    FLP_GNSS_RESET,
    FLP_GNSS_RESUME,
    FLP_IOMCU_RESET,
};

typedef struct flp_pdr_data {
    unsigned long       msec ;
    unsigned int        step_count;
    int                 relative_position_x;
    int                 relative_position_y;
    short               velocity_x;
    short               velocity_y;
    unsigned int        migration_distance;
    unsigned int        absolute_altitude;
    unsigned short      absolute_bearing;
    unsigned short      reliability_flag;
}  __packed flp_pdr_data_t  ;

typedef struct compensate_data {
    unsigned int        compensate_step;
    int                 compensate_position_x;
    int                 compensate_position_y;
    unsigned int        compensate_distance;
} compensate_data_t ;

typedef struct  step_report {
    int data[12] ;
} step_report_t;

typedef struct buf_info {
    char            *buf;
    unsigned long   len;
} buf_info_t;

typedef struct pdr_start_config {
    unsigned int   report_interval;
    unsigned int   report_precise;
    unsigned int   report_count;
    unsigned int   report_times;   /*plan to remove */
} pdr_start_config_t;

/********************************************
            define AR struct
********************************************/
#define 	CONTEXT_PRIVATE_DATA_MAX (64)

typedef struct ar_packet_header{
    unsigned char tag;
    unsigned char  cmd;
    unsigned char  resp : 1;
    unsigned char  rsv : 3;
    unsigned char  core : 4;   /*AP set to zero*/
    unsigned char  partial_order;
    unsigned short tranid;
    unsigned short length;
}ar_packet_header_t;   /* compatable to pkt_header_t */

typedef struct ar_activity_event {
    unsigned int        event_type;         /*0:disable*/
    unsigned int        activity;
    unsigned long       msec ;
}  __packed ar_activity_event_t ;

typedef struct ar_activity_config {
    unsigned int        event_type;         /*0:disable*/
    unsigned int        activity;
}  __packed ar_activity_config_t ;

typedef struct ar_start_config {
    unsigned int            report_interval;
    ar_activity_config_t    activity_list[AR_STATE_BUTT];
} ar_start_config_t;

typedef struct ar_start_hal_config {
    unsigned int            report_interval;
    unsigned int            event_num;
    ar_activity_config_t    *pevent;
} ar_start_hal_config_t ;

typedef struct ar_start_mcu_config {
    unsigned int            event_num;
    unsigned int            report_interval;
    ar_activity_config_t    activity_list[AR_STATE_BUTT];
} ar_start_mcu_config_t;

typedef struct ar_context_cfg_header {
	unsigned int	event_type;
	unsigned int	context;
	unsigned int	len;
}__packed ar_context_cfg_header_t;
/*KERNEL&HAL*/

typedef struct context_config {
	ar_context_cfg_header_t head;
	char buf[CONTEXT_PRIVATE_DATA_MAX];
} __packed context_config_t;

typedef struct context_hal_config {
	unsigned int	report_interval;
	unsigned int	context_num;
	context_config_t *context_addr;
}__packed context_hal_config_t;

/*KERNEL-->HUB*/
typedef struct context_iomcu_cfg {
	unsigned int report_interval;
	unsigned int context_num;
	context_config_t context_list[CONTEXT_TYPE_MAX];
} context_iomcu_cfg_t;
#define STATE_KERNEL_BUF_MAX	(1024)
typedef struct context_dev_info {
	unsigned int	usr_cnt;
	unsigned int 	cfg_sz;
	context_iomcu_cfg_t   cfg;
	struct completion state_cplt;
	unsigned int state_buf_len;
	char state_buf[STATE_KERNEL_BUF_MAX];
}context_dev_info_t;

/*HUB-->KERNEL*/
#define GET_STATE_NUM_MAX (5)
typedef struct context_event {
	unsigned int event_type;
	unsigned int context;
	unsigned long long msec;/*long long :keep some with iomcu*/
	unsigned int buf_len;
	char buf[0];
} __packed context_event_t;

typedef struct {
	pkt_header_t pkt;
	unsigned int context_num;
	context_event_t context_event[0];
} __packed ar_data_req_t;

typedef struct {
	unsigned int context_num;
	context_event_t context_event[0];
} __packed ar_state_t;
/********************************************
      define Batching and Geofence struct
********************************************/
typedef struct {
    unsigned short		flags;
    double              latitude;
    double              longitude;
    double              altitude;
    float               speed;
    float               bearing;
    float               accuracy;
    long		        timestamp;
    unsigned int        sources_used;
} __packed iomcu_location;

typedef struct gps_batching_report{
    int num_locations;
    iomcu_location location[FLP_GPS_BATCHING_MAX_SIZE];
} __packed gps_batching_report_t;

/*Geofence event report*/
typedef struct geofencing_transition {
    int geofence_id;
    int transition;
    unsigned int sources_used;
    unsigned long timestamp;
    iomcu_location location;
} __packed geofencing_transition_t;

/*Geofence status report*/
typedef struct geofencing_monitor_status {
    unsigned char status;
    unsigned int source;
    iomcu_location last_location;
}__packed geofencing_monitor_status_t;

/*modify Geofence*/
typedef struct geofencing_option_info {
    unsigned char  virtual_id;
    unsigned char last_transition;
    unsigned char monitor_transitions;
    unsigned char reserved;
    int unknown_timer_ms;
    unsigned int sources_to_use;
}__packed  geofencing_option_info_t;

typedef struct {
double latitude;
double longitude;
double radius_m;
}__packed geofencing_circle_t;

typedef struct {
    unsigned char geofence_type;
    union {
        geofencing_circle_t circle;
    };
}__packed geofencing_data_t;

typedef struct geofencing_useful_data {
    geofencing_option_info_t  opt;
    geofencing_data_t info;
}__packed   geofencing_useful_data_t;

/*add Geofence*/
typedef struct geofencing_add_config {
    unsigned char  number_of_geofences;
    geofencing_useful_data_t geofences[FLP_GEOFENCE_MAX_NUM];
}__packed  geofencing_add_config_t;

/*remove Geofence*/
typedef struct geofencing_remove_config {
    unsigned char  number_of_geofences;
    unsigned char index_id[FLP_GEOFENCE_MAX_NUM];
}__packed  geofencing_remove_config_t;

typedef struct geofencing_hal_config {
    unsigned int  length;
    void *buf;
}__packed  geofencing_hal_config_t;

/*start or update batching cmd*/
typedef struct {
    double max_power_allocation_mW;
    unsigned int sources_to_use;
    unsigned int flags;
    long period_ns;
    float smallest_displacement_meters;
} FlpBatchOptions;

typedef struct batching_config {
    int  id;
    int  batching_distance;
    FlpBatchOptions opt;
} __packed  batching_config_t;

/********************************************
            define flp netlink
********************************************/
#define FLP_GENL_NAME                   "TASKFLP"
#define TASKFLP_GENL_VERSION            0x01

enum {
    FLP_GENL_ATTR_UNSPEC,
    FLP_GENL_ATTR_EVENT,
    __FLP_GENL_ATTR_MAX,
};
#define FLP_GENL_ATTR_MAX (__FLP_GENL_ATTR_MAX - 1)

enum {
    FLP_GENL_CMD_UNSPEC,
    FLP_GENL_CMD_PDR_DATA,
    FLP_GENL_CMD_AR_DATA,
    FLP_GENL_CMD_PDR_UNRELIABLE,
    FLP_GENL_CMD_NOTIFY_TIMEROUT,
    FLP_GENL_CMD_AWAKE_RET,
    FLP_GENL_CMD_GEOFENCE_TRANSITION,
    FLP_GENL_CMD_GEOFENCE_MONITOR,
    FLP_GENL_CMD_GNSS_LOCATION,
    FLP_GENL_CMD_IOMCU_RESET,
    FLP_GENL_CMD_ENV_DATA,
    __FLP_GENL_CMD_MAX,
};
#define FLP_GENL_CMD_MAX (__FLP_GENL_CMD_MAX - 1)


