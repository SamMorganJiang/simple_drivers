/*
 * a simple char device driver: simplefifo
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#define SIMPLEFIFO_NAME     "simplefifo"
#define SIMPLEFIFO_SIZE     0x1000
#define FIFO_CLEAR          0x1

static struct simplefifo_dev {
    struct miscdevice miscdev;
    unsigned char mem[SIMPLEFIFO_SIZE];
};

struct simplefifo_dev *sf;

static int simplefifo_open(struct inode *inode, struct file *filp) {
    filp->private_data = sf;
    return 0;
}

static int simplefifo_release(struct inode *inode, struct file *filp) {
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

static const struct file_operations simplefifo_fops = {
	.owner = THIS_MODULE,
	.open = simplefifo_open,
	.release = simplefifo_release,
	.read = simplefifo_read,
	.write = simplefifo_write,
	.unlocked_ioctl = simplefifo_ioctl,
};

static int simplefifo_probe(struct platform_device *pdev) {
    int ret;

    sf = devm_kzalloc(&pdev->dev, sizeof(*sf), GFP_KERNEL);
    if (!sf)
        return -ENOMEM;

    sf->miscdev.minor = MISC_DYNAMIC_MINOR;
    sf->miscdev.name  = SIMPLEFIFO_NAME;
    sf->miscdev.fops  = &simplefifo_fops;

    ret = misc_register(&sf->miscdev);
    if (ret < 0)
        goto err;

    dev_info(&pdev->dev, "simplefifo probe ok!\n");
    return 0;
err:
    return ret;
}

static int simplefifo_remove(struct platform_device *pdev) {
    //struct simplefifo_dev *sf = platform_get_drvdata(pdev);

    misc_deregister(&sf->miscdev);

    dev_info(&pdev->dev, "simplefifo remove ok!\n");
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
