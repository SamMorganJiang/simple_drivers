#ifndef __MISC_CTRL_H__
#define __MISC_CTRL_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/string.h>

#include "gpio.h"
#include "file.h"
#include "irq.h"
#include "input.h"
#include "pwm.h"


#define PREFIX "[normal_misc_drv]"

#define DEBUG_MISC_HW_K
#ifdef DEBUG_MISC_HW_K
#define KP_DBG(fmt, arg...)                   printk(PREFIX fmt, ##arg)
#define KP_INF(fmt, arg...)                   printk(PREFIX fmt, ##arg)
#define KP_ERR(fmt, arg...)                   printk(PREFIX fmt, ##arg)
#else
#define KP_DBG(fmt, arg...)
#define KP_INF(fmt, arg...)                   pr_info(PREFIX fmt, ##arg)
#define KP_ERR(fmt, arg...)                   pr_err(PREFIX fmt, ##arg)
#endif

#define DRV_NAME                        "normal_misc_drv"
#define CLASS_NAME                      "normal_drv"
#define INPUT_NAME                      "normal_input"
#define DRV_COMPATIBLE_MATCH            "normal, normal_misc_drv"

struct normal_drv_struct {
	struct GPIO               gpio;
	struct mutex              mutex;
	struct input_module       input;
	struct irq_module         irq;
	struct class              *class;
	struct platform_device    *pdev;
};

enum RETURN {
	RETURN_SUCCESS = 0,
	RETURN_ERROR   = -1
};

extern struct normal_drv_struct *v_global;

#endif
