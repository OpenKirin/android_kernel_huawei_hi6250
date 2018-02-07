#ifndef __MDRV_MSP_SLEEP_COMMON_H__
#define __MDRV_MSP_SLEEP_COMMON_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include "mdrv_pm_common.h"

#define SLAVE_MODE_BUTT (PWC_COMM_MODE_NUMBER)

enum MasterMode_Enum
{
    MASTER_MODE_LTE = 0,
    MASTER_MODE_TDS = 1,
    MASTER_MODE_BUTT
};

enum TLSLEEP_RttForceAwake_Type
{
    TLSLEEP_MAILBOX = 0,
    TLSLEEP_OTHERS
};

typedef enum cpm_mode_e
{
    CPM_LTE0,
    CPM_LTE1,
    CPM_TDS,
    CPM_COMMON,
    BUTT_MODE,
    CPM_MAX = 10
}CPM_MODE_E;

typedef enum cpm_req_e
{
    IDLE_REQ = 0,
    BUSY_REQ
}CPM_REQ_E;

typedef enum cpm_phy_state_e
{
    PHY_LOW_POWER_STATE = 0,
    PHY_NORMAL_STATE 
}CPM_PHY_STATE_E;
 
struct MasterAwakeSlaveFlag
{
    unsigned int SlaveAwakeFlag[MASTER_MODE_BUTT][SLAVE_MODE_BUTT];
};

struct DspDvsFlag
{
    unsigned int DspDvsReq;            /*bit0-bit7:BBP��ѹ���� 01��ѹ 00��ѹ;bit8-bit15:BBE16��ѹ���� 01��ѹ 00��ѹ;
                                         bit16-bit23:�������к�(��BBE16����);bit24-bit31:��ѹ������Ч��־(0x55)*/
    unsigned int DspDvsReq_timestamp;   
    unsigned int DspDvsRespond;        /*bit0-bit7:BBP��ѹ״̬ 01��ѹ 00��ѹ;bit8-bit15:BBE16��ѹ״̬ 01��ѹ 00��ѹ;
                                         bit16-bit23:��ѹӦ�����к�(���ڵ�ѹ�������к�);bit24-bit31:��ѹӦ����Ч��־(0xAA)*/   
    unsigned int DspDvsRespond_timestamp;
};

typedef struct
{
    unsigned int ulLightSleepFlag;    /*DSP��˯ǳ˯�ı�־*/
    unsigned int ulTdsPreSfn;         /*DSP����������*/
    unsigned int ulRfTime;            /*RF�ȶ�ʱ����Ҫ1ms*/
    unsigned int dspState;            /*DSP��˯�߻����ѣ�0:˯ 1:��*/
    unsigned int dspTime;            	/*DSP��Ӧ��dspState���ʱ���*/ 
    unsigned int ulReserved[3];
}RTT_SLEEP_INFO_STRU;

struct sleep_state_req
{
    unsigned char mode_req[CPM_MAX];
    unsigned char core_req[2];
    unsigned char dsp_state[CPM_MAX];  
    unsigned char core_state[2];
};

struct mst_awake_slv
{
    unsigned char awake_flag[PWC_COMM_MODEM_2][SLAVE_MODE_BUTT];
};

struct harq_ddr_freq
{
    unsigned int ddr_freq;
};

typedef struct{
    RTT_SLEEP_INFO_STRU PstRttSleepInfo;     /* �����������ͨ���׵�ַƫ�Ʒ�ʽʹ�øýṹ���������Ҫ�����ýṹ�����ڲ���Ա˳����֪�������ͬ�� */
    struct DspDvsFlag DvsFlag;
    struct MasterAwakeSlaveFlag AwakeFlag;
#if (FEATURE_DSP_PM_SEPARATE_MODE == FEATURE_ON)
    struct sleep_state_req   state_req;
    struct mst_awake_slv     mst_awake_slv;
    struct harq_ddr_freq     req;
#endif
}SRAM_SOC_TLDSP_INTERACTION_STRU;

#ifdef __cplusplus
}
#endif
#endif
  
