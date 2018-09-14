/*
 * Misc Device Driver
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include "core.h"

struct normal_drv_struct *v_global;

//=================================================================//
static int normal_misc_drv_probe(struct platform_device *pdev)
{
	int err = 0;

	KP_INF("[%s] start\n", __func__);

	v_global = devm_kzalloc(&pdev->dev, sizeof(*v_global), GFP_KERNEL);
	if (!v_global)
		return -ENOMEM;

	v_global->pdev = pdev;
	mutex_init(&v_global->mutex);

	err = gpio_state_init(&v_global->gpio);
	if (err)
		KP_ERR("[%s] gpio pinctrl init failed\n", __func__);

	err = input_device_init(&v_global->input);
	if (err)
		KP_ERR("[%s] input device init failed\n", __func__);

	err = irq_request_init(&v_global->irq);
	if (err)
		KP_ERR("[%s] irq request init failed\n", __func__);

	err = proc_file_init(v_global->class);
	if (err)
		KP_ERR("[%s] proc file init failed\n", __func__);

	KP_INF("[%s] end\n", __func__);
	return err;
}

//=================================================================//
static int normal_misc_drv_remove(struct platform_device *pdev)
{
	KP_INF("[%s]\n", __func__);

	return 0;
}

//=================================================================//
static const struct of_device_id normal_misc_drv_of_match[] = {
	{ .compatible = DRV_COMPATIBLE_MATCH, },
	{},
};

static struct platform_driver normal_misc_driver = {
	.probe  = normal_misc_drv_probe,
	.remove = normal_misc_drv_remove,
	.driver = {
		.owner          = THIS_MODULE,
		.name           = DRV_NAME,
		.of_match_table = normal_misc_drv_of_match,
	},
};

module_platform_driver(normal_misc_driver);

MODULE_AUTHOR("Sam <sam.morgan.jiang@outlook.com>");
MODULE_DESCRIPTION("Misc Device Driver");
MODULE_LICENSE("GPL v2");
