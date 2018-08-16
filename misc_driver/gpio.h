/*
 * Vanzo Misc Device Driver --- gpio subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#ifndef __VANZO_MISC_GPIO_CTRL_H__
#define __VANZO_MISC_GPIO_CTRL_H__

enum GPIO_CTRL_STATE {
	GPIO_CTRL_STATE_NULL        = -1,

	GPIO_CTRL_STATE_OTG_DISABLE = 0,            /*  0  */
	GPIO_CTRL_STATE_OTG_ENABLE,                 /*  1  */

	GPIO_CTRL_STATE_LED_X_DISABLE,              /*  2  */
	GPIO_CTRL_STATE_LED_X_ENABLE,               /*  3  */

	GPIO_CTRL_STATE_LED_Y_DISABLE,              /*  4  */
	GPIO_CTRL_STATE_LED_Y_ENABLE,               /*  5  */

	GPIO_CTRL_STATE_FAN_1V8_DISABLE,            /*  6  */
	GPIO_CTRL_STATE_FAN_1V8_ENABLE,             /*  7  */

	GPIO_CTRL_STATE_FAN_5V_DISABLE,             /*  8  */
	GPIO_CTRL_STATE_FAN_5V_ENABLE,              /*  9  */

	GPIO_CTRL_STATE_FAN_PWM_DISABLE,            /*  10  */
	GPIO_CTRL_STATE_FAN_PWM_ENABLE,             /*  11  */

	GPIO_CTRL_STATE_MAX_NUM
};

enum DEVICE_OBJECT {
	DEVICE_NULL = -1,
	OTG_EN_CTL  = 0,   /* 0 */
	LED_X,             /* 1 */
	LED_Y,             /* 2 */
	FAN_1V8,           /* 3 */
	FAN_5V,            /* 4 */
	FAN_PWM,           /* 5 */

	DEVICE_MAX
};

enum GPIO_CTRL_LEVEL {
	PIN_STATE_LOW,
	PIN_STATE_HIGH
};

struct GPIO_PINCTRL {
	char *pin_lookup_names;
};

struct GPIO {
	struct pinctrl       *ppinctrl;
	struct pinctrl_state *ppinctrl_state[GPIO_CTRL_STATE_MAX_NUM];
};

enum RETURN gpio_state_init(struct GPIO *gpio);
enum RETURN gpio_state_set(struct GPIO *gpio,
			enum DEVICE_OBJECT dev_obj,
			enum GPIO_CTRL_LEVEL pin_level);

#endif
