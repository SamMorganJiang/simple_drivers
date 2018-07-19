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

static struct simplefifo_dev {
	struct file *filp;
	struct inode *inodp;
	struct miscdevice miscdev;
	unsigned char fifo[SIMPLEFIFO_SIZE];
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
	ssize_t ret = 0;
	struct simplefifo_dev *sf = container_of(filp->private_data,
		struct simplefifo_dev, miscdev);

	if (count < 0)
		return -EINVAL;
	if (*ppos >= SIMPLEFIFO_SIZE)
		return 0;
	if (count > SIMPLEFIFO_SIZE - *ppos)
		count = SIMPLEFIFO_SIZE - *ppos;

	if (copy_to_user(buf, sf->fifo + *ppos, count))
		ret = -EFAULT;
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
	struct simplefifo_dev *sf = container_of(filp->private_data,
		struct simplefifo_dev, miscdev);

	if (count < 0)
		return -EINVAL;
	if (*ppos >= SIMPLEFIFO_SIZE)
		return 0;
	if (count > SIMPLEFIFO_SIZE - *ppos)
		count = SIMPLEFIFO_SIZE - *ppos;

	if (copy_from_user(sf->fifo + *ppos, buf, count))
		ret = -EFAULT;
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
	struct simplefifo_dev *sf = container_of(filp->private_data,
		struct simplefifo_dev, miscdev);

	switch (cmd) {
		case FIFO_CLEAR:
			memset(sf->fifo, 0, SIMPLEFIFO_SIZE);
			pr_info(KERN_INFO "simplefifo is cleared\n");
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static loff_t simplefifo_llseek(struct file *filp, loff_t offset, int orig)
{
	loff_t ret = 0;

	switch (orig) {
		case 0:
			if (offset < 0 || offset > SIMPLEFIFO_SIZE) {
				ret = -EINVAL;
				break;
			}
			filp->f_pos = offset;
			ret = filp->f_pos;
			break;
		case 1:
			if (filp->f_pos + offset < 0 || filp->f_pos + offset > SIMPLEFIFO_SIZE) {
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

static int simplefifo_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct simplefifo_dev *sf;

	dev_info(&pdev->dev, "simplefifo probe start!\n");

	sf = devm_kzalloc(&pdev->dev, sizeof(*sf), GFP_KERNEL);
	if (!sf)
		return -ENOMEM;

	platform_set_drvdata(pdev, sf);

	sf->miscdev.minor = MISC_DYNAMIC_MINOR;
	sf->miscdev.name  = SIMPLEFIFO_NAME;
	sf->miscdev.fops  = &simplefifo_fops;

	ret = misc_register(&sf->miscdev);
	if (ret < 0)
		goto err;

	sf->filp = filp_open("/dev/simplefifo", O_RDONLY, 0);
	if (IS_ERR(sf->filp)) {
		pr_err(KERN_INFO "file open failed!\n");
		goto file_err;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
	sf->inodp = sf->filp->f_path.dentry->d_inode;
#else
	sf->inodp = sf->filp->f_dentry->d_inode;
#endif
	sf->inodp->i_mode |= S_IRWXUGO;
	filp_close(sf->filp, NULL);

	dev_info(&pdev->dev, "simplefifo probe end!\n");
	return ret;

file_err:
	misc_deregister(&sf->miscdev);
err:
	return ret;
}

static int simplefifo_remove(struct platform_device *pdev)
{
	struct simplefifo_dev *sf = platform_get_drvdata(pdev);

	misc_deregister(&sf->miscdev);

	dev_info(&pdev->dev, "simplefifo remove!\n");
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
