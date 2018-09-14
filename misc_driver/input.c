/*
 * Misc Device Driver --- input subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/input.h>

#include "core.h"

//=================================================================//
static unsigned int input_event_list[EVENT_NUM] = {
	KEY_F11,
	KEY_F12,

	EVENT_END
};

//=================================================================//
enum RETURN input_device_init(struct input_module *input)
{
	int i;
	int ret = RETURN_SUCCESS;

	KP_INF("[%s] start\n", __func__);

	input->dev = input_allocate_device();
	if (!input->dev)
		return -ENOMEM;

	input->list = input_event_list;
	input->dev->name = INPUT_NAME;
	set_bit(EV_KEY, input->dev->evbit);

	for (i = 0; input_event_list[i] != EVENT_END; i++) {
		set_bit(input_event_list[i], input->dev->keybit);
		input_set_capability(input->dev, EV_KEY, input_event_list[i]);
	}

	ret = input_register_device(input->dev);
	if (ret)
		goto err;

	return ret;

err:
	input_free_device(input->dev);
	return RETURN_ERROR;
}

//=================================================================//
void input_event_state_report(struct input_module *input, unsigned int code, int value)
{
	input_report_key(input->dev, code, value);
	input_sync(input->dev);
}

