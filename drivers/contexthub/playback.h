/*
 * hisi playback driver.
 *
 * Copyright (C) 2017 huawei Ltd.
 * Author:lijiangxiong <lijingxiong@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define PLAYBACK_MAX_PATH_LEN   512
#define MAX_FILE_SIZE           (1<<24) /*16M*/
#define MAX_SUPPORT_TAG_COUNT   8

#define IOCTL_PLAYBACK_INIT     _IOW(0xB3, 0x01, unsigned int)
#define IOCTL_PLAYBACK_START    _IO(0xB3, 0x02)
#define IOCTL_PLAYBACK_STOP     _IO(0xB3, 0x03)
#define IOCTL_PLAYBACK_REPLAY_COMPLETE  _IOR(0xB3, 0x04, unsigned int)

enum {
    FUNCTION_OPEN   = (1<<0),
    FUNCTION_INIT   = (1<<1),
    FUNCTION_START  = (1<<2),
};

enum {
    BUFFER1_READY = 1,
    BUFFER2_READY,
    BUFFER_ALL_READY,
};

enum {
    COMPLETE_CLOSE = 0,
    COMPLETE_REPLAY_DONE,
};

typedef struct {
    unsigned short  tag_id;
    unsigned int    buf1_addr;
    unsigned int    buf2_addr;
    unsigned int    buf_size;
} __packed sm_info_t;

typedef struct {
    pkt_header_t    pkt;
    unsigned char   mode;   /*0:play 1:record*/
    sm_info_t       sm_info[];
} __packed sm_open_cmd_t;

typedef struct {
    unsigned short  tag_id;
    unsigned short  buf_status; //1: buffer1 ready 2: buffer2 ready 3: buffer1 and 2 ready
    unsigned int    data_size[2];
}  __packed data_status_t;

typedef struct {
    pkt_header_t    pkt;
    unsigned int    sub_cmd;
    data_status_t   data_status[];
} __packed  sm_data_ready_cmd_t;

typedef struct {
    unsigned short  tag_id;
    unsigned short  buf_status; //1: buffer1 ready 2: buffer2 ready 3: buffer1 and 2 ready
} __packed buf_status_t;

typedef struct {
    pkt_header_t    pkt;
    unsigned int    sub_cmd;
    buf_status_t    buf_status[];
} __packed sm_buf_ready_cmd_t;

typedef struct {
    unsigned short  tag_id;
    unsigned short  data_len;
    unsigned char   sensor_data[];
}  __packed sensor_data_format_t;

typedef struct {
    unsigned int    mode; //0表示数据回放，1表示数据录取
    char            *path;
} app_init_cmd_t;

typedef struct {
    unsigned short  tag_id;
    unsigned int    buf1_addr;
    unsigned int    buf2_addr;
    unsigned int    buf_size;
    unsigned int    file_offset;
} __packed playback_info_t;

typedef struct {
    unsigned int    status;
    unsigned int    current_mode;
    char            current_path[PLAYBACK_MAX_PATH_LEN];
    char            filename[PLAYBACK_MAX_PATH_LEN];
    unsigned int    current_count;
    unsigned int    phyAddr;
    unsigned int    size;
    void __iomem    *vaddr;
    struct mutex    mutex_lock;
    struct completion done;
    unsigned int    is_wait;
    unsigned int    complete_status;
    struct work_struct      work;
    unsigned int    data;
    playback_info_t *info;
} playback_dev_t;


