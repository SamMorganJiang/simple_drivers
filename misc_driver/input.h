/*
 * Vanzo Misc Device Driver --- input subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#ifndef __VANZO_MISC_INPUT_CTRL_H__
#define __VANZO_MISC_INPUT_CTRL_H__

#define EVENT_NUM            3
#define EVENT_END            -1

struct input_module {
	struct input_dev        *dev;
	unsigned int code[EVENT_NUM];
	unsigned int *list;
};

enum KEY_STATE {
	KEY_STATE_UP,
	KEY_STATE_DOWN
};

enum RETURN input_device_init(struct input_module *input);
void input_event_state_report(struct input_module *input, unsigned int code, int value);


#endif
