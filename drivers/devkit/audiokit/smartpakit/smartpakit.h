/*
** =============================================================================
** Copyright (c) 2017 Huawei Device Co.Ltd
**
** This program is free software; you can redistribute it and/or modify it under
** the terms of the GNU General Public License as published by the Free Software
** Foundation; version 2.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
** FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
** Street, Fifth Floor, Boston, MA 02110-1301, USA.
**
**Author: wangping48@huawei.com
** =============================================================================
*/

#ifndef __SMARTPAKIT_H__
#define __SMARTPAKIT_H__

#include "smartpakit_defs.h"

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#define SMARTPAKIT_KFREE_OPS(p) \
do {\
	if (p != NULL) { \
		kfree(p); \
		p = NULL; \
	} \
} while(0)

#define SMARTPAKIT_I2C_ADDR_ARRAY_MAX    (0x80) // i2c addr max == 0x7f
#define SMARTPAKIT_INVALID_PA_INDEX      (0xff)

// reg val_bits
#define SMARTPAKIT_REG_VALUE_B8     (8)  // val_bits == 8
#define SMARTPAKIT_REG_VALUE_B16    (16) // val_bits == 16
#define SMARTPAKIT_REG_VALUE_B24    (24) // val_bits == 24
#define SMARTPAKIT_REG_VALUE_B32    (32) // val_bits == 32

// reg value mask by reg val_bits
#define SMARTPAKIT_REG_VALUE_M8     (0xFF)
#define SMARTPAKIT_REG_VALUE_M16    (0xFFFF)
#define SMARTPAKIT_REG_VALUE_M24    (0xFFFFFF)
#define SMARTPAKIT_REG_VALUE_M32    (0xFFFFFFFF)

typedef struct smartpakit_gpio_state {
	unsigned int state;
	unsigned int delay;
} smartpakit_gpio_state_t;

typedef struct smartpakit_gpio_sequence {
	unsigned int num;
	smartpakit_gpio_state_t *node;
} smartpakit_gpio_sequence_t;

typedef struct smartpakit_gpio_reset {
	int gpio;
	char gpio_name[SMARTPAKIT_NAME_MAX];
	smartpakit_gpio_sequence_t sequence;
} smartpakit_gpio_reset_t;

typedef enum smartpakit_reg_ctl_type {
	SMARTPAKIT_REG_CTL_TYPE_R = 0,  // read reg
	SMARTPAKIT_REG_CTL_TYPE_W,      // write reg
	SMARTPAKIT_REG_CTL_TYPE_DELAY,  // only time delay

	SMARTPAKIT_REG_CTL_TYPE_MAX,
} smartpakit_reg_ctl_type_t;

typedef struct smartpakit_reg_ctl {
	// one reg address or reg address_begin of a set of registers
	unsigned int addr;

	// ctl value
	// read:  reg number(num >= 1)
	// write: reg value
	// delay: time delay
	unsigned int value;

	// read ctl type, default 0(read)
	unsigned int ctl_type;
} smartpakit_reg_ctl_t;

typedef struct smartpakit_reg_ctl_sequence {
	unsigned int num;
	smartpakit_reg_ctl_t *regs;
} smartpakit_reg_ctl_sequence_t;

typedef struct smartpakit_gpio_irq {
	int gpio;
	int irq;
	unsigned int irqflags;
	bool need_reset;
	char gpio_name[SMARTPAKIT_NAME_MAX];
	char irq_name[SMARTPAKIT_NAME_MAX];

	smartpakit_reg_ctl_sequence_t *rw_sequence; // read or write reg sequence
} smartpakit_gpio_irq_t;

typedef struct smartpakit_regmap_cfg {
	// write reg or update_bits
	unsigned int value_mask;

	// regmap config
	int num_writeable;
	int num_unwriteable;
	int num_readable;
	int num_unreadable;
	int num_volatile;
	int num_unvolatile;
	int num_defaults;

	unsigned int *reg_writeable;
	unsigned int *reg_unwriteable;
	unsigned int *reg_readable;
	unsigned int *reg_unreadable;
	unsigned int *reg_volatile;
	unsigned int *reg_unvolatile;
	struct reg_default *reg_defaults;

	struct regmap_config cfg;

	// regmap
	struct regmap *regmap;
} smartpakit_regmap_cfg_t;

typedef struct smartpakit_i2c_priv {
	unsigned int chip_vendor;
	unsigned int chip_id;
	const char *chip_model;

	int probe_completed;

	// reset
	unsigned int reset_debounce_wait_time;
	smartpakit_gpio_reset_t *hw_reset;

	// irq
	smartpakit_gpio_irq_t *irq_handler;
	struct work_struct irq_handle_work;
	unsigned long irq_debounce_jiffies;
	struct delayed_work irq_debounce_work;

	// version regs
	smartpakit_reg_ctl_sequence_t *version_regs_sequence;

	// dump regs
	smartpakit_reg_ctl_sequence_t *dump_regs_sequence;

	// reg map config
	smartpakit_regmap_cfg_t *regmap_cfg;

	unsigned int i2c_pseudo_addr;
	void *priv_data;
	struct device *dev;
	struct i2c_client *i2c;
} smartpakit_i2c_priv_t;

typedef struct smartpakit_i2c_ioctl_ops {
	int (*hw_reset)(void *priv);
	int (*dump_regs)(void *priv);
	int (*read_regs)(void *priv, void __user *p);
	int (*write_regs)(void *priv, void __user *p, int compat_mode);
	int (*write_regs_all)(void *priv, void __user *p, int compat_mode);
	int (*i2c_read)(struct i2c_client *i2c, char *rx_data, int length);
	int (*i2c_write)(struct i2c_client *i2c, char *rx_data, int length);
} smartpakit_i2c_ioctl_ops_t;

// param node from hal_driver XXX.xml
typedef struct smartpakit_param_node {
	unsigned int index; // reg addr for smartpa, or gpio index for simple pa
	unsigned int mask;
	unsigned int value;
	unsigned int delay;
	unsigned int reserved;
} smartpakit_param_node_t;

typedef struct smartpakit_pa_ctl_sequence {
	unsigned int pa_ctl_num;
	unsigned int pa_ctl_index_max;
	unsigned int pa_ctl_index[SMARTPAKIT_PA_ID_MAX];
	unsigned int param_num;
	smartpakit_param_node_t *node;
} smartpakit_pa_ctl_sequence_t;

typedef struct smartpakit_switch_node {
	int gpio;
	int state;
	char name[SMARTPAKIT_NAME_MAX];
} smartpakit_switch_node_t;

typedef struct smartpakit_priv {
	unsigned int soc_platform;
	unsigned int algo_in;
	unsigned int algo_delay_time;
	unsigned int out_device;
	// support pa number(now, max == SMARTPAKIT_PA_ID_MAX)
	unsigned int pa_num;
	unsigned int i2c_num; // load successful i2c module

	// for simple pa(not smart pa)
	const char *chip_model;
	unsigned int switch_num;
	smartpakit_switch_node_t *switch_ctl;

	// smartpa resume regs sequence
	bool resume_sequence_permission_enable;
	smartpakit_pa_ctl_sequence_t resume_sequence;

	// misc device r/w settings
	bool misc_rw_permission_enable;
	bool misc_i2c_use_pseudo_addr;
	// Here, i2c_addr_to_pa_index array length is 128 bytes
	// use this array to speed the i2c_ops(r/w) up
	unsigned char i2c_addr_to_pa_index[SMARTPAKIT_I2C_ADDR_ARRAY_MAX];
	struct i2c_client *current_i2c_client;

	// for i2c ops
	smartpakit_i2c_priv_t *i2c_priv[SMARTPAKIT_PA_ID_MAX];
	smartpakit_i2c_ioctl_ops_t *ioctl_ops;

	struct mutex irq_handler_lock;
	struct mutex hw_reset_lock;
	struct mutex write_regs_lock;
} smartpakit_priv_t;

int  smartpakit_register_i2c_device(smartpakit_i2c_priv_t *i2c_priv);
int  smartpakit_deregister_i2c_device(smartpakit_i2c_priv_t *i2c_priv);
void smartpakit_register_ioctl_ops(smartpakit_i2c_ioctl_ops_t *ops);
int  smartpakit_parse_params(smartpakit_pa_ctl_sequence_t *sequence, void __user *arg, int compat_mode);

#endif // __SMARTPAKIT_H__

