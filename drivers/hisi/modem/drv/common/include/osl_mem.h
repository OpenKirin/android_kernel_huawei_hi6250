#ifndef __OSL_MEM_H
#define __OSL_MEM_H

#if defined(__OS_RTOSCK__)||defined(__OS_RTOSCK_SMP__)
#include <product_config.h>
#include <sre_buildef.h>
#include <sre_config.h>
/*
*���ڱ����Ҫ�ڲ���ϵͳ����֮ǰ�ĳ�ʼ����ģ��
*
*PS:һ������ǾͲ������޸Ĵ���ͱ���
*/
#define  __sys_entry_init   __attribute__((section(".text.entry.init")))

#ifdef CONFIG_CCPU_HAS_TCM
/*
*���ڱ�Ǵ����TCM�еĺ���������
*__tcmdata                ���ڱ�Ƿ�����DTCM�е�����
*__tcmfunc                ���ڱ�Ƿ�����ITCM�еĺ�����֧����ͨ��������
*__tcmlocalfunc          ���ڱ�Ƿ�����ITCM�еĺ�������֧����ͨ��������
*
*PS:�����TCM�еĺ���������Ҫ����δ��__tcmlocalfunc��ǵĺ���
*/
#define __tcmdata           __attribute__((section(".tcm.data")))

#define __tcmfunc  __attribute__((section(".tcm.text")))

#else
#define __tcmdata
#define __tcmfunc
#endif

#ifdef CONFIG_CCPU_HAS_LLRAM
/*
*���ڱ�Ǵ����LLRAM������
*__llramdata                ���ڱ�Ƿ�����LLRAM�е�����
*
*PS:LLRAM�в����ú�����ֻ��������
*/

#define __llramdata         __attribute__((section(".llram.data"))) 

/*֧��LLRAM�ռ䶯̬��ȡ*/
#define  LLRAM_MEM_POOL_INDEX          (OS_MEM_MAX_PT_NUM -1)     /*OS �ڴ������*/

/*LLRAM ����ӿ��ڲ�ʵ��ǿ��4�ֽڶ��룬ʧ��ΪNULL*/
void* osl_llram_malloc(unsigned int size);

/*LLRAM�ͷţ�ʧ��Ϊ�ǿ�*/
unsigned int osl_llram_free(void *free_addr);
#else
#define __llramdata

static inline void* osl_llram_malloc(unsigned int size){return NULL;}
static inline unsigned int osl_llram_free(void *free_addr){return (unsigned int)0;}
#endif
#else
#define __sys_entry_init
#define __tcmdata  
#define __tcmfunc       
#define __llramdata    
#endif
#endif
