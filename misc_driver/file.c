/*
 * Vanzo Misc Device Driver --- file subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/kernel.h>

#include "core.h"

static int otg_state;
static int led_x_state, led_y_state;
static int flash_x_state, flash_y_state;
static int fan_state, fan_speed;

extern int init_lm3643(void);
extern void unint_lm3643(void);
extern void open_lm3643_led1(void);
extern void close_lm3643_led1(void);
extern void open_lm3643_led2(void);
extern void close_lm3643_led2(void);

//=================================================================//
static ssize_t otg_ctrl_show(struct class *dev,
	struct class_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "otg state: %d\n", otg_state);
}

static ssize_t otg_ctrl_store(struct class *dev,
	struct class_attribute *attr, const char *buf, size_t count)
{
	struct vanzo_drv_struct *v = v_global;

	if (buf[0] == '0')
		gpio_state_set(&v->gpio, OTG_EN_CTL, PIN_STATE_LOW);
	else
		gpio_state_set(&v->gpio, OTG_EN_CTL, PIN_STATE_HIGH);

	return count;
}

//=================================================================//
static ssize_t led_ctrl_show(struct class *dev,
	struct class_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "x state: %d ; y state:%d\n", led_x_state, led_y_state);
}

static ssize_t led_ctrl_store(struct class *dev,
	struct class_attribute *attr, const char *buf, size_t count)
{
	struct vanzo_drv_struct *v = v_global;

	if (buf[0] == 'x') {
		if (buf[1] == '0') {
			gpio_state_set(&v->gpio, LED_X, PIN_STATE_LOW);
			led_x_state = 0;
		} else {
			gpio_state_set(&v->gpio, LED_X, PIN_STATE_HIGH);
			led_x_state = 1;
		}
	} else if (buf[0] == 'y') {
		if (buf[1] == '0') {
			gpio_state_set(&v->gpio, LED_Y, PIN_STATE_LOW);
			led_y_state = 0;
		} else {
			gpio_state_set(&v->gpio, LED_Y, PIN_STATE_HIGH);
			led_y_state = 1;
		}
	}

	return count;
}

//=================================================================//
static ssize_t flash_ctrl_show(struct class *dev,
	struct class_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "x state: %d ; y state:%d\n", flash_x_state, flash_y_state);
}

static ssize_t flash_ctrl_store(struct class *dev,
	struct class_attribute *attr, const char *buf, size_t count)
{
	if (buf[0] == 'x') {
		if (buf[1] == '0') {
			close_lm3643_led1();
			unint_lm3643();
			flash_x_state = 0;
		} else {
			init_lm3643();
			open_lm3643_led1();
			flash_x_state = 1;
		}
	} else if (buf[0] == 'y') {
		if (buf[1] == '0') {
			close_lm3643_led2();
			unint_lm3643();
			flash_y_state = 0;
		} else {
			init_lm3643();
			open_lm3643_led2();
			flash_y_state = 1;
		}
	}

	return count;
}

//=================================================================//
static ssize_t fan_ctrl_show(struct class *dev,
	struct class_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "state: %d; speed: %d\n", fan_state, fan_speed);
}

static ssize_t fan_ctrl_store(struct class *dev,
	struct class_attribute *attr, const char *buf, size_t count)
{
	int speed = buf[0] - '0';
	struct vanzo_drv_struct *v = v_global;

	if (buf[0] == '0') {
		gpio_state_set(&v->gpio, FAN_1V8, PIN_STATE_LOW);
		gpio_state_set(&v->gpio, FAN_5V,  PIN_STATE_LOW);
		gpio_state_set(&v->gpio, FAN_PWM, PIN_STATE_LOW);

		pwm_state_set(PWM1, 0, 0);

		fan_state = 0;
		fan_speed = 0;
	} else {
		gpio_state_set(&v->gpio, FAN_1V8, PIN_STATE_HIGH);
		gpio_state_set(&v->gpio, FAN_5V,  PIN_STATE_HIGH);
		gpio_state_set(&v->gpio, FAN_PWM, PIN_STATE_HIGH);

		pwm_state_set(PWM1, 1, speed);

		fan_state = 1;
		fan_speed = speed;
	}

	return count;
}

//=================================================================//
static struct class_attribute proc_file_attr[] = {
	__ATTR(otg,     0664, otg_ctrl_show,   otg_ctrl_store),
	__ATTR(led,     0664, led_ctrl_show,   led_ctrl_store),
	__ATTR(flash,   0664, flash_ctrl_show, flash_ctrl_store),
	__ATTR(fan,     0664, fan_ctrl_show,   fan_ctrl_store),
	__ATTR_NULL
};

//=================================================================//
static char *__ATTR_NAME(const struct class_attribute *obj)
{
	return (char *)(obj->attr.name);
}

//=================================================================//
enum RETURN proc_file_init(struct class *class)
{
	int err, i;

	KP_INF("[%s] start\n", __func__);

	class = class_create(THIS_MODULE, VANZO_CLASS_NAME);
	if (IS_ERR(class)) {
		KP_ERR("[%s] class_create failed\n", __func__);
		return RETURN_ERROR;
	}

	for (i = 0; NULL != __ATTR_NAME(&proc_file_attr[i]); i++) {
		err = class_create_file(class, &proc_file_attr[i]);
		if (0 != err) {
			KP_ERR("[%s] create %s class file failed\n",
				__func__, __ATTR_NAME(&proc_file_attr[i]));
			err = RETURN_ERROR;
		}
	}

	return err;
}

