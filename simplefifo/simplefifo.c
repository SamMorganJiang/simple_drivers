#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/platform_device.h>

static struct simplefifo_dev {

};

static int simplefifo_probe(struct platform_device *pdev) {

}

static int simplefifo_remove(struct platform_device *pdev) {

}

static struct platform_driver simplefifo_drviver = {
    .driver = {
        .name = "simplefifo",
        .owner = THIS_MODULE,
    },
    .probe = simplefifo_probe,
    .remove = simplefifo_remove,
};

module_platform_driver(simplefifo_drviver);

MODULE_AUTHOR("Sam <sam.morgan.jiang@outlook.com>");
MODULE_DESCRIPTION("Linux drivers learning");
MODULE_LICENSE("GPL");
