/*
 * a simple char device driver: simplefifo
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include <linux/module.h>
#include <linux/platform_device.h>

static struct platform_device *simplefifo_pdev;

static int __init simplefifo_dev_init(void) {
    int ret;

    simplefifo_pdev = platform_device_alloc("simplefifo", -1);
    if (!simplefifo_pdev)
        return -ENOMEM;

    ret = platform_device_add(simplefifo_pdev);
    if (ret) {
        platform_device_put(simplefifo_pdev);
        return ret;
    }

    return 0;
}

static void __exit simplefifo_dev_exit(void) {
    platform_device_unregister(simplefifo_pdev);
}

module_init(simplefifo_dev_init);
module_exit(simplefifo_dev_exit);

MODULE_AUTHOR("Sam <sam.morgan.jiang@outlook.com>");
MODULE_LICENSE("GPL v2");
