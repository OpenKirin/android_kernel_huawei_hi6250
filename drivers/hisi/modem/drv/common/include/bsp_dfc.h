#ifndef __BSP_DFC_H__
#define __BSP_DFC_H__

#include <osl_types.h>
#include <bsp_sram.h>

#ifdef CONFIG_DFC

/* DFCͨ��id���� */
enum DFC_CHN_ID
{
	DFC_CHN_CCORE =0, 
	DFC_CHN_DSP = 1,
	DFC_CHN_DSP_1 = 2,
	DFC_CHN_DSP_2 = 3,
	DFC_CHN_DSP_3 = 4,
	DFC_CHN_DSP_4 = 5,
	DFC_CHN_DSP_5 = 6,
	DFC_CHN_DSP_6 = 7,
	DFC_CHN_DSP_7 = 8,
	DFC_CHN_DSP_8 = 9,
	DFC_CHN_ID_MAX
};

struct dfc_u64_addr{
    u32 addrL;
    u32 addrH;
};

union dfc_list_bd
{
    struct list_bd_bits
    {
        u32    user_info                 : 3; /* [2..0] user info*/
        u32    base_addrL              : 29; /* [31..3]  base_addrL*/
        u32    base_addrH             : 16; /* [47..32]  base_addrH*/
        u32    reserved                  : 16; /* [63..48] ���� */
    } bits;
    struct dfc_u64_addr union_stru;
};  

enum dfc_cpu_id
{
    DFC_TLDSP0 = 0,
    DFC_CCPU = 1,
    DFC_NXP = 2
};

typedef u32 (*int_cb_func)(u32 chn_id, u32 level);

/* DFC addr */
#ifndef DFC_LLRAM_BASE_ADDR
#define DFC_LLRAM_BASE_ADDR           (CCPU_DFC_ADDR)
#endif

#ifndef DFC_LLRAM_SIZE
#define DFC_LLRAM_SIZE                (CCPU_DFC_SIZE)
#endif

/*define*/
#define DFC_DSP0_SIZE          (SZ_4K)
#define DFC_DSP1_SIZE        (SZ_4K)
#define DFC_DSP2_SIZE           (SZ_4K)
#define DFC_DSP3_SIZE        (SZ_16K)
#define DFC_DSP4_SIZE        (SZ_4K)

/*****************************************************************************************************************************
*ͨ����С��ƫ����
******************************************************************************************************************************/
#define DFC_CHANNEL_OFFSET_BASE		    	(0x0)
#define DFC_CHANNEL_OFFSET_DSP0    (DFC_CHANNEL_OFFSET_BASE + DFC_DSP0_SIZE)
#define DFC_CHANNEL_OFFSET_DSP1    (DFC_CHANNEL_OFFSET_DSP0 + DFC_DSP0_SIZE)
#define DFC_CHANNEL_OFFSET_DSP2    (DFC_CHANNEL_OFFSET_DSP1 + DFC_DSP1_SIZE)
#define DFC_CHANNEL_OFFSET_DSP3    (DFC_CHANNEL_OFFSET_DSP2 + DFC_DSP2_SIZE)
#define DFC_CHANNEL_OFFSET_DSP4    (DFC_CHANNEL_OFFSET_DSP3 + DFC_DSP3_SIZE)

/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_startup
*
* ��������  : dfcͨ��������������ͨ��״̬������idle
*
* �������  :  ͨ��id
                         
* �������  : ��
*
* �� �� ֵ  :     OK         ����ɹ�
                              ERROR   ����ʧ��
*
* �޸ļ�¼  :     
*****************************************************************************/
s32 bsp_dfc_channel_startup(u32 chn_id);

/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_shutdown
*
* ��������  : dfcͨ��������������ͨ��״̬��Ϊidle
*
* �������  :  ͨ��id
                         
* �������  : ��
*
* �� �� ֵ  :     OK         �ɹ�
                              ERROR   ʧ��
*
* �޸ļ�¼  :   
*****************************************************************************/
s32 bsp_dfc_channel_shutdown(u32 chn_id);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_register_isr
*
* ��������  : ͨ���ж�ע��ص�
*
* �������  :  ͨ��id
                         
* �������  : ��
*
* �� �� ֵ  :     OK         �ɹ�
                              ERROR   ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
s32 bsp_dfc_channel_register_isr(u32 chn_id, int_cb_func pFunc);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_unregister_isr
*
* ��������  : ͨ���ж�ע���ص�
*
* �������  : ͨ��id
                         
* �������  : ��
*
* �� �� ֵ  :     OK         �ɹ�
                              ERROR   ʧ��
*
* �޸ļ�¼  
*****************************************************************************/
s32 bsp_dfc_channel_unregister_isr (u32 chn_id);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_get_state
*
* ��������  : ��ȡͨ��״̬
                             
*
* �������  :  ͨ��id
                         
* �������  : ��
*
* �� �� ֵ  :     ��
*
* �޸ļ�¼  :  
*****************************************************************************/
u32 bsp_dfc_channel_get_state (u32 chn_id);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_int_enable
*
* ��������  : dfcͨ���ж�ʹ��
*
* �������  :  ͨ��id
*                           ʹ���ж�״̬λ
                         
* �������  : ��
*
* �� �� ֵ  :     OK       �ɹ�
                              ERROR   ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
s32 bsp_dfc_channel_int_enable(u32 chn_id, u32 level);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_int_disable
*
* ��������  :dfcͨ���ж�ȥʹ��
*
* �������  :  ͨ��id
*                           ȥʹ���ж�״̬λ
                         
* �������  : ��
*
* �� �� ֵ  :     OK       �ɹ�
                              ERROR   ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
s32 bsp_dfc_channel_int_disable(u32 chn_id, u32 level);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_int_clr
*
* ��������  :dfcͨ���ж�д��
*
* �������  :  ͨ��id
*                           ȥʹ���ж�д��
                         
* �������  : ��
*
* �� �� ֵ  :     OK       �ɹ�
                              ERROR   ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
s32 bsp_dfc_channel_int_clr(u32 chn_id, u32 level);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_read_all
*
* ��������  : dfcͨ����ȡ��������
*
* �������  :  ͨ��id,
*                           ���ݶ�ȡĿ�ĵ�ַָ��void * dest
                         
* �������  : ��
*
* �� �� ֵ  :     OK       �ɹ�
                              ERROR   ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
s32 bsp_dfc_channel_read_all(u32 chn_id, void * dest );
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_read
*
* ��������  : dfcͨ��������
*
* �������  :  ͨ��id
*                           ��ȡ����Ŀ�ĵ�ַָ��void * dest
*                           ��ȡ���ݳ���
                         
* �������  : ��
*
* �� �� ֵ  :     OK       �ָ��ɹ�
                              ERROR   �ָ�ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
u32 bsp_dfc_channel_read(u32 chn_id, void * dest, u32 data_len);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_list_read
*
* ��������  : ��ʽ��������ͨ��������
*
* �������  :  ͨ��id
                         ��ʽ���ݹ�����
* �������  : ��
*
* �� �� ֵ  :     OK       �ָ��ɹ�
                              ERROR   �ָ�ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
s32 bsp_dfc_channel_list_read(u32 chn_id, union dfc_list_bd * list_rd);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_list_data_read
*
* ��������  : ��ʽ��������ͨ��������
*
* �������  :  ͨ��id
*                         ��ʽ���ݹ�����
*                         ��ȡ����Ŀ�ĵ�ַvoid* data_dest
* �������  : ��
*
* �� �� ֵ  :     OK       �ָ��ɹ�
                              ERROR   �ָ�ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
s32 bsp_dfc_channel_list_data_read(u32 chn_id, union dfc_list_bd * list_rd, void * data_dest);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_data_write
*
* ��������  : dfcͨ��д����
*
* �������  :  ͨ��id
*                         �޷�����������
* �������  : ��
*
* �� �� ֵ  :     OK       �ָ��ɹ�
                              ERROR   �ָ�ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
s32 bsp_dfc_channel_data_write(u32 chn_id, u32 data);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_list_write
*
* ��������  : ��ʽ��������ͨ��д����
*
* �������  :  ͨ��id
*                           ��ʽ���ݹ�����
* �������  : ��
*
* �� �� ֵ  :     OK       �ָ��ɹ�
                              ERROR   �ָ�ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
s32 bsp_dfc_channel_list_write(u32 chn_id, union dfc_list_bd bd_data);
/*****************************************************************************
* �� �� ��  :bsp_dfc_channel_write
*
* ��������  : dfcͨ��д����
*
* �������  :  ͨ��id
*                           ����Դ��ַ
*                           ���ݳ���
* �������  : ��
*
* �� �� ֵ  :     OK       �ָ��ɹ�
                              ERROR   �ָ�ʧ��
*
* �޸ļ�¼  :  
*****************************************************************************/
s32 bsp_dfc_channel_write(u32 chn_id, void * src, u32 data_len);


#else

#endif/*CONFIG_CCORE_PM*/

#endif
