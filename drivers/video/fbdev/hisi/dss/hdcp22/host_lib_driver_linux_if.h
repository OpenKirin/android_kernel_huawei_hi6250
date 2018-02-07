/*-----------------------------------------------------------------------
// Copyright (c) 2017, Hisilicon Tech. Co., Ltd. All rights reserved.
//
// This software and the associated documentation are confidential and
// proprietary to Synopsys, Inc. Your use or disclosure of this software
// is subject to the terms and conditions of a written license agreement
// between you, or your company, and Synopsys, Inc.
//
// The entire notice above must be reproduced on all authorized copies.
//
//-----------------------------------------------------------------------
//
// Project:
//
// Host Library.
//
// Description:
//
// Linux kernel driver interface definitions.
//
//-----------------------------------------------------------------------*/

#ifndef _HOST_LIB_DRIVER_LINUX_IF_H_
#define _HOST_LIB_DRIVER_LINUX_IF_H_

#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h>
#endif

//#include "HostLibDriver.h"
#include "elliptic_std_def.h"
#include "elliptic_system_types.h"
#include "HostLibDriverErrors.h"

/**
 * \addtogroup HL_Driver
 * @{
 */
/** Host Library Driver instruct kernel driver to create dynamic memory. */
#define HL_DRIVER_ALLOCATE_DYNAMIC_MEM 0xffffffff
/** @}
 */
 

#define ESM_STATUS                          ELP_STATUS
// IOCTL commands
#define ESM_HLD_IOCTL_LOAD_CODE             1000
#define ESM_HLD_IOCTL_GET_CODE_PHYS_ADDR    1001
#define ESM_HLD_IOCTL_GET_DATA_PHYS_ADDR    1002
#define ESM_HLD_IOCTL_GET_DATA_SIZE         1003
#define ESM_HLD_IOCTL_HPI_READ              1004
#define ESM_HLD_IOCTL_HPI_WRITE             1005
#define ESM_HLD_IOCTL_DATA_READ             1006
#define ESM_HLD_IOCTL_DATA_WRITE            1007
#define ESM_HLD_IOCTL_DATA_SET              1008
#define ESM_HLD_IOCTL_ESM_OPEN              1009

#define ESM_HLD_IOCTL_DPTX_READ              1010
#define ESM_HLD_IOCTL_DPTX_WRITE             1011
#define ESM_HLD_IOCTL_ESM_START              1012

// ESM_HLD_IOCTL_LOAD_CODE
typedef struct
{
   uint8_t *code;
   uint32_t code_size;
   ESM_STATUS returned_status;
} esm_hld_ioctl_load_code;

// ESM_HLD_IOCTL_GET_CODE_PHYS_ADDR
typedef struct
{
   uint32_t returned_phys_addr;
   ESM_STATUS returned_status;
} esm_hld_ioctl_get_code_phys_addr;

// ESM_HLD_IOCTL_GET_DATA_PHYS_ADDR
typedef struct
{
   uint32_t returned_phys_addr;
   ESM_STATUS returned_status;
} esm_hld_ioctl_get_data_phys_addr;

// ESM_HLD_IOCTL_GET_DATA_SIZE
typedef struct
{
   uint32_t returned_data_size;
   ESM_STATUS returned_status;
} esm_hld_ioctl_get_data_size;

// ESM_HLD_IOCTL_HPI_READ
typedef struct
{
   uint32_t offset;
   uint32_t returned_data;
   ESM_STATUS returned_status;
} esm_hld_ioctl_hpi_read;

// ESM_HLD_IOCTL_HPI_WRITE
typedef struct
{
   uint32_t offset;
   uint32_t data;
   ESM_STATUS returned_status;
} esm_hld_ioctl_hpi_write;

// ESM_HLD_IOCTL_DATA_READ
typedef struct
{
   uint32_t offset;
   uint32_t nbytes;
   uint8_t *dest_buf;
   ESM_STATUS returned_status;
} esm_hld_ioctl_data_read;

// ESM_HLD_IOCTL_DATA_WRITE
typedef struct
{
   uint32_t offset;
   uint32_t nbytes;
   uint8_t *src_buf;
   ESM_STATUS returned_status;
} esm_hld_ioctl_data_write;

// ESM_HLD_IOCTL_DATA_SET
typedef struct
{
   uint32_t offset;
   uint32_t nbytes;
   uint8_t data;
   ESM_STATUS returned_status;
} esm_hld_ioctl_data_set;

// ESM_HLD_IOCTL_ESM_OPEN
typedef struct
{
   uint32_t hpi_base;
   uint32_t code_base;
   uint32_t code_size;
   uint32_t data_base;
   uint32_t data_size;
   ESM_STATUS returned_status;
} esm_hld_ioctl_esm_open;

typedef struct
{
   ESM_STATUS returned_status;
} esm_hld_ioctl_esm_start;

void esm_driver_enable(int en);

#endif // _HOST_LIB_DRIVER_LINUX_IF_H_
