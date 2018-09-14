/*
 * Misc Device Driver --- pwm subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#ifndef __MISC_PWM_CTRL_H__
#define __MISC_PWM_CTRL_H__

#include <mach/mt_pwm_hal.h>
#include <mt-plat/mt_pwm.h>

void pwm_state_set(int pwm_num, int state, int speed);


#endif
