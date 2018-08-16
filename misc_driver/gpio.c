/*
 * Vanzo Misc Device Driver --- gpio subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/kernel.h>
#include <linux/pinctrl/pinctrl.h>

#include "core.h"

//=================================================================//
static struct GPIO_PINCTRL gpio_namespace_list[GPIO_CTRL_STATE_MAX_NUM] = {
	{"otg_disable"},
	{"otg_enable"},
	{"led_x_disable"},
	{"led_x_enable"},
	{"led_y_disable"},
	{"led_y_enable"},
	{"fan_1v8_disable"},
	{"fan_1v8_enable"},
	{"fan_5v_disable"},
	{"fan_5v_enable"},
	{"fan_pwm_disable"},
	{"fan_pwm_enable"},
	{NULL}
};

//=================================================================//
enum RETURN gpio_state_init(struct GPIO *gpio)
{
	int    i;
	enum   RETURN            ret            = RETURN_SUCCESS;
	struct GPIO              *pgpio         = gpio;
	struct GPIO_PINCTRL      *pgpio_pinctrl = gpio_namespace_list;
	struct vanzo_drv_struct  *v             = container_of(gpio, struct vanzo_drv_struct, gpio);

	pgpio->ppinctrl = devm_pinctrl_get(&v->pdev->dev);
	if (IS_ERR(pgpio->ppinctrl)) {
		KP_ERR("can not find devctrl\n");
		return RETURN_ERROR;
	}

	for (i = 0; i < GPIO_CTRL_STATE_MAX_NUM; i++, pgpio_pinctrl++) {
		if (pgpio_pinctrl->pin_lookup_names)
			pgpio->ppinctrl_state[i] =
				pinctrl_lookup_state(pgpio->ppinctrl, pgpio_pinctrl->pin_lookup_names);

		if (IS_ERR_OR_NULL(pgpio->ppinctrl_state[i])) {
			KP_ERR("%s, : pinctrl lookup err, %s\n",
				__func__, pgpio_pinctrl->pin_lookup_names);
			ret = RETURN_ERROR;
		}
	}

	return ret;
}

//=================================================================//
enum RETURN gpio_state_set(struct GPIO *gpio,
			enum DEVICE_OBJECT dev_obj,
			enum GPIO_CTRL_LEVEL pin_level)
{
	enum   RETURN        ret             = RETURN_SUCCESS;
	struct pinctrl_state *ppinctrl_state = NULL;
	struct GPIO          *pgpio          = gpio;

	if ((dev_obj >= DEVICE_MAX || dev_obj <= DEVICE_NULL) &&
		(pin_level >= GPIO_CTRL_STATE_MAX_NUM || pin_level <= GPIO_CTRL_STATE_NULL))
	{
		KP_ERR("[%s] pin num error : DEVICE_OBJECT(%d) GPIO_CTRL_LEVEL(%d)\n",
			__func__, dev_obj, pin_level);
		return RETURN_ERROR;
	}

	ppinctrl_state = pgpio->ppinctrl_state[dev_obj * 2 + pin_level];

	if (!IS_ERR_OR_NULL(ppinctrl_state))
		pinctrl_select_state(pgpio->ppinctrl, ppinctrl_state);
	else
		KP_ERR("%s, : pinctrl set err : %d-%d\n", __func__, dev_obj, pin_level);

	return RETURN_SUCCESS;
}

