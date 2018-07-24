/*
 * a simple char device driver: simplefifo
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/cdev.h>
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
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/atomic.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#define SIMPLEFIFO_NAME     "simplefifo"
#define SIMPLEFIFO_SIZE     0x1000
#define FIFO_CLEAR          0x1

#define USE_MUTEX           1

static struct simplefifo_dev {

};

static int simplefifo_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int simplefifo_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t simplefifo_read(struct file *filp, char __user *buf,
				size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t simplefifo_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *ppos)
{
	return 0;
}


static long simplefifo_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	return 0;
}

static loff_t simplefifo_llseek(struct file *filp, loff_t offset, int orig)
{
	return 0;
}

static const struct file_operations simplefifo_fops = {
	.owner = THIS_MODULE,
	.open = simplefifo_open,
	.release = simplefifo_release,
	.read = simplefifo_read,
	.write = simplefifo_write,
	.unlocked_ioctl = simplefifo_ioctl,
	.llseek = simplefifo_llseek,
};

static int simplefifo_probe(struct platform_device *pdev)
{
	return 0;
}

static int simplefifo_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver simplefifo_drviver = {
	.driver = {
		.name   = SIMPLEFIFO_NAME,
		.owner  = THIS_MODULE,
	},
	.probe  = simplefifo_probe,
	.remove = simplefifo_remove,
};

module_platform_driver(simplefifo_drviver);

MODULE_AUTHOR("Sam <sam.morgan.jiang@outlook.com>");
MODULE_DESCRIPTION("Linux drivers learning");
MODULE_LICENSE("GPL v2");
