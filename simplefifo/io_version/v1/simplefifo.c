/*
 * a simple char device driver: simplefifo
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/version.h>
#include <linux/sched.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
#include <linux/sched/signal.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

#define SIMPLEFIFO_NAME     "simplefifo"
#define SIMPLEFIFO_SIZE     0x1000
#define FIFO_CLEAR          0x1

static struct simplefifo_dev {
	struct miscdevice miscdev;
	struct mutex mutex;
	struct inode *inodp;
	struct file *filp;
	struct fasync_struct *async_queue;
	wait_queue_head_t r_wait;
	wait_queue_head_t w_wait;
	unsigned int current_len;
	unsigned char fifo[SIMPLEFIFO_SIZE];
};

static int simplefifo_fasync(int fd, struct file *filp, int mode)
{
	struct simplefifo_dev *sf = container_of(filp->private_data,
		struct simplefifo_dev, miscdev);

	return fasync_helper(fd, filp, mode, &sf->async_queue);
}

static int simplefifo_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int simplefifo_release(struct inode *inode, struct file *filp)
{
	simplefifo_fasync(-1, filp, 0);
	return 0;
}

static ssize_t simplefifo_read(struct file *filp, char __user *buf,
				size_t count, loff_t *ppos)
{
	ssize_t ret = 0;
	struct simplefifo_dev *sf = container_of(filp->private_data,
		struct simplefifo_dev, miscdev);

	mutex_lock(&sf->mutex);
	DECLARE_WAITQUEUE(wq, current);
	add_wait_queue(&sf->r_wait, &wq);

	while (sf->current_len == 0) {
		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto mutex_release;
		}

		__set_current_state(TASK_INTERRUPTIBLE);
		mutex_unlock(&sf->mutex);

		schedule();
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			goto err;
		}

		mutex_lock(&sf->mutex);
	}

	if (count > sf->current_len)
		count = sf->current_len;

	if (copy_to_user(buf, sf->fifo, count)) {
		ret = -EFAULT;
		goto mutex_release;
	} else {
		memcpy(sf->fifo, sf->fifo + count, sf->current_len - count);
		sf->current_len -= count;
		pr_info(KERN_INFO "read %d bytes(s),current_len:%d\n",
			count, sf->current_len);

		wake_up_interruptible(&sf->w_wait);

		if (sf->async_queue) {
			kill_fasync(&sf->async_queue, SIGIO, POLL_OUT);
			pr_debug(KERN_DEBUG, "%s kill SIGIO POLL_OUT\n", __func__);
		}

		ret = count;
	}

mutex_release:
	mutex_unlock(&sf->mutex);
err:
	remove_wait_queue(&sf->r_wait, &wq);
	__set_current_state(TASK_RUNNING);
	return ret;
}

static ssize_t simplefifo_write(struct file *filp, const char __user *buf,
				size_t count, loff_t *ppos)
{
	ssize_t ret = 0;
	struct simplefifo_dev *sf = container_of(filp->private_data,
		struct simplefifo_dev, miscdev);

	mutex_lock(&sf->mutex);
	DECLARE_WAITQUEUE(wq, current);
	add_wait_queue(&sf->w_wait, &wq);

	while (sf->current_len == SIMPLEFIFO_SIZE) {
		if (filp->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			goto mutex_release;
		}

		__set_current_state(TASK_INTERRUPTIBLE);
		mutex_unlock(&sf->mutex);

		schedule();
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			goto err;
		}

		mutex_lock(&sf->mutex);
	}

	if (count > SIMPLEFIFO_SIZE - sf->current_len)
		count = SIMPLEFIFO_SIZE - sf->current_len;

	if (copy_from_user(sf->fifo + sf->current_len, buf, count)) {
		ret = -EFAULT;
		goto mutex_release;
	} else {
		sf->current_len += count;
		pr_info(KERN_INFO "written %d bytes(s),current_len:%d\n",
			count, sf->current_len);

		wake_up_interruptible(&sf->r_wait);

		if (sf->async_queue) {
			kill_fasync(&sf->async_queue, SIGIO, POLL_IN);
			pr_debug(KERN_DEBUG, "%s kill SIGIO POLL_IN\n", __func__);
		}

		ret = count;
	}

mutex_release:
	mutex_unlock(&sf->mutex);
err:
	remove_wait_queue(&sf->w_wait, &wq);
	__set_current_state(TASK_RUNNING);
	return ret;
}

static unsigned int simplefifo_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;
	struct simplefifo_dev *sf = container_of(filp->private_data,
		struct simplefifo_dev, miscdev);

	mutex_lock(&sf->mutex);

	poll_wait(filp, &sf->r_wait, wait);
	poll_wait(filp, &sf->w_wait, wait);

	if (sf->current_len != 0)
		mask |= POLLIN | POLLRDNORM;

	if (sf->current_len != 0)
		mask |= POLLOUT | POLLRDNORM;

	mutex_unlock(&sf->mutex);
	return mask;
}

static long simplefifo_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	struct simplefifo_dev *sf = container_of(filp->private_data,
		struct simplefifo_dev, miscdev);

	switch (cmd) {
		case FIFO_CLEAR:
			mutex_lock(&sf->mutex);
			sf->current_len = 0;
			memset(sf->fifo, 0, SIMPLEFIFO_SIZE);
			mutex_unlock(&sf->mutex);

			pr_info(KERN_INFO "simplefifo is set to zero\n");
			break;

		default:
			return -EINVAL;
	}

	return 0;
}

static const struct file_operations simplefifo_fops = {
	.owner          = THIS_MODULE,
	.open           = simplefifo_open,
	.release        = simplefifo_release,
	.read           = simplefifo_read,
	.write          = simplefifo_write,
	.poll           = simplefifo_poll,
	.unlocked_ioctl	= simplefifo_ioctl,
	.fasync         = simplefifo_fasync,
};

static int simplefifo_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct simplefifo_dev *sf;

	sf = devm_kzalloc(&pdev->dev, sizeof(*sf), GFP_KERNEL);
	if (!sf) {
		ret = -ENOMEM;
		goto err;
	}

	sf->miscdev.minor = MISC_DYNAMIC_MINOR;
	sf->miscdev.name  = SIMPLEFIFO_NAME;
	sf->miscdev.fops  = &simplefifo_fops;

	ret = misc_register(&sf->miscdev);
	if (ret < 0)
		goto err;

	mutex_init(&sf->mutex);
	platform_set_drvdata(pdev, sf);
	init_waitqueue_head(&sf->r_wait);
	init_waitqueue_head(&sf->w_wait);

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

	dev_info(&pdev->dev, "simplefifo probe!\n");
	return 0;
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
