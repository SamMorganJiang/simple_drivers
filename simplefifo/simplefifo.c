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
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/uaccess.h>

#define SIMPLEFIFO_NAME     "simplefifo"
#define SIMPLEFIFO_SIZE     0x1000
#define FIFO_CLEAR          0x1

static struct simplefifo_dev {
    struct miscdevice miscdev;
    unsigned char mem[SIMPLEFIFO_SIZE];
};

struct simplefifo_dev *sf;

static int simplefifo_open(struct inode *inode, struct file *filp) {
    filp->private_data = (void *)sf;
    return 0;
}

static int simplefifo_release(struct inode *inode, struct file *filp) {
    return 0;
}

static ssize_t simplefifo_read(struct file *filp, char __user *buf,
			       size_t count, loff_t *ppos)
{
    ssize_t ret = 0;
    struct simplefifo_dev *dev = filp->private_data;

    if (*ppos >= SIMPLEFIFO_SIZE)
        return 0;
    if (count > SIMPLEFIFO_SIZE - *ppos)
        count = SIMPLEFIFO_SIZE - *ppos;

    if (copy_to_user(buf, dev->mem + *ppos, count))
        return -EFAULT;
    else {
        *ppos += count;
        ret = count;
        pr_info(KERN_INFO "read %u bytes form %lu\n", count, *ppos);
    }

    return ret;
}

static ssize_t simplefifo_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *ppos)
{
    ssize_t ret = 0;
    struct simplefifo_dev *dev = filp->private_data;

    if (*ppos >= SIMPLEFIFO_SIZE)
        return 0;
    if (count > SIMPLEFIFO_SIZE - *ppos)
        count = SIMPLEFIFO_SIZE - *ppos;

    if (copy_from_user(dev->mem + *ppos, buf, count))
        return -EFAULT;
    else {
        *ppos += count;
        ret = count;
        pr_info(KERN_INFO "write %u bytes to %lu\n", count, *ppos);
    }

    return ret;
}

static long simplefifo_ioctl(struct file *filp, unsigned int cmd,
			     unsigned long arg)
{
    struct simplefifo_dev *dev = filp->private_data;

    switch (cmd) {
        case FIFO_CLEAR:
            memset(dev->mem, 0, SIMPLEFIFO_SIZE);
            pr_info(KERN_INFO "simplefifo is cleared\n");
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static loff_t simplefifo_llseek(struct file *filp, loff_t offset, int orig) {
    loff_t ret = 0;

    switch (orig) {
        case 0:
            if (offset < 0) {
                ret = -EINVAL;
                break;
            }
            if (offset > SIMPLEFIFO_SIZE) {
                ret = -EINVAL;
                break;
            }
            filp->f_pos = offset;
            ret = filp->f_pos;
            break;

        case 1:
            if (filp->f_pos + offset > SIMPLEFIFO_SIZE || filp->f_pos + offset < 0) {
                ret = -EINVAL;
                break;
            }
            filp->f_pos += offset;
            ret = filp->f_pos;
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
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

static int simplefifo_probe(struct platform_device *pdev) {
    int ret = 0;

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
