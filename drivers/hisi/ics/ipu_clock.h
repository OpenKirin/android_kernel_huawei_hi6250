/* Module internals
 *
 * Copyright (C) 2017 Hisilicon, Inc. All Rights Reserved.
 * Written by fuying@Hisilicon
 *
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of
 * Hisilicon Inc. and its licensors, and are licensed to the recipient
 * under the terms of a separate license agreement.  They may be
 * adapted and modified by bona fide purchasers under the terms of the
 * separate license agreement for internal use, but no adapted or
 * modified version may be disclosed or distributed to third parties
 * in any manner, medium, or form, in whole or in part, without the
 * prior written consent of Hisilicon Inc.
 */

#ifndef _IPU_CLOCK_H
#define _IPU_CLOCK_H

#include <linux/clk.h>

extern int ipu_clock_init(struct device *dev, struct clk **clock, unsigned int *start_rate, unsigned int *stop_rate);
extern int ipu_clock_start(struct clk *clock, unsigned int clock_rate);
extern void ipu_clock_stop(struct clk *clock);
extern int ipu_clock_set_rate(struct clk *clock, unsigned int clock_rate);

#endif
