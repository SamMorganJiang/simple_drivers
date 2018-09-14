/*
 * Misc Device Driver --- pwm subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include "core.h"

void pwm_state_set(int pwm_num, int state, int speed)
{
	static struct pwm_spec_config pwm_setting;

	if (state) {
		pwm_setting.pwm_no                       = pwm_num;
		pwm_setting.mode                         = PWM_MODE_OLD;
		pwm_setting.clk_src                      = PWM_CLK_OLD_MODE_BLOCK;
		pwm_setting.PWM_MODE_OLD_REGS.THRESH     = speed; /* 4 ~ 9 */
		pwm_setting.clk_div                      = CLK_DIV128;
		pwm_setting.PWM_MODE_OLD_REGS.DATA_WIDTH = 20;
		pwm_set_spec_config(&pwm_setting);
	} else
		mt_pwm_disable(pwm_num, 1);
}

