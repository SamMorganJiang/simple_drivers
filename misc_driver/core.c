/*
 * Vanzo Misc Device Driver
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include "core.h"

struct vanzo_drv_struct *v_global;

//=================================================================//
static int vanzo_misc_drv_probe(struct platform_device *pdev)
{
	int err = 0;

	KP_INF("[%s] start\n", __func__);

	v_global = devm_kzalloc(&pdev->dev, sizeof(*v_global), GFP_KERNEL);
	if (!v_global)
		return -ENOMEM;

	v_global->pdev = pdev;
	mutex_init(&v_global->mutex);

	err = gpio_state_init(&v_global->gpio);
	if (RETURN_SUCCESS != err)
		KP_ERR("[%s] gpio pinctrl init failed\n", __func__);

	err = input_device_init(&v_global->input);
	if (RETURN_SUCCESS != err)
		KP_ERR("[%s] input device init failed\n", __func__);

	err = irq_request_init(&v_global->irq);
	if (RETURN_SUCCESS != err)
		KP_ERR("[%s] irq request init failed\n", __func__);

	err = proc_file_init(v_global->class);
	if (RETURN_SUCCESS != err)
		KP_ERR("[%s] proc file init failed\n", __func__);

	KP_INF("[%s] end\n", __func__);
	return err;
}

//=================================================================//
static int vanzo_misc_drv_remove(struct platform_device *pdev)
{
	KP_INF("[%s]\n", __func__);

	return 0;
}

//=================================================================//
static const struct of_device_id vanzo_misc_drv_of_match[] = {
	{ .compatible = VANZO_DRV_COMPATIBLE_MATCH, },
	{},
};

static struct platform_driver vanzo_misc_driver = {
	.probe  = vanzo_misc_drv_probe,
	.remove = vanzo_misc_drv_remove,
	.driver = {
		.owner          = THIS_MODULE,
		.name           = VANZO_DRV_NAME,
		.of_match_table = vanzo_misc_drv_of_match,
	},
};

module_platform_driver(vanzo_misc_driver);

MODULE_AUTHOR("Sam <sam.morgan.jiang@outlook.com>");
MODULE_DESCRIPTION("Vanzo Misc Device Driver");
MODULE_LICENSE("GPL v2");
