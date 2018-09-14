/*
 * Misc Device Driver --- irq subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include "core.h"

static int current_irq_num;
static struct irq_module_name irq_namespace_list[IRQ_NAME_NUM] = { NULL };

static void global_tasklet_handler(unsigned long data);
static DECLARE_TASKLET(global_tasklet, global_tasklet_handler, 0);

static void irq_state_handler(struct irq_module *irqm, int i)
{
	switch (irqm->irq_state[i]) {
		case IRQ_STATE_UP:
			irq_set_irq_type(irqm->irq_num[i], IRQF_TRIGGER_HIGH);
			irqm->irq_state[i] = IRQ_STATE_DOWN;
			IRQ_NOTIFY(irqm, i, KEY_STATE_DOWN);
			KP_DBG("[%s] irq(%d) state down\n", __func__, irqm->irq_num[i]);
			break;

		case IRQ_STATE_DOWN:
			irq_set_irq_type(irqm->irq_num[i], IRQF_TRIGGER_LOW);
			irqm->irq_state[i] = IRQ_STATE_UP;
			IRQ_NOTIFY(irqm, i, KEY_STATE_UP);
			KP_DBG("[%s] irq(%d) state up\n", __func__, irqm->irq_num[i]);
			break;

		default:
			KP_DBG("[%s] unknown irq state\n", __func__);
			break;
	}
}

static void global_tasklet_handler(unsigned long data)
{
	int i;
	struct irq_module       *irqm     = &v_global->irq;

	KP_INF("[%s] start\n", __func__);

	for (i = 0; i < IRQ_NAME_NUM; i++) {
		if (current_irq_num == irqm->irq_num[i]) {
			irq_state_handler(irqm, i);
			enable_irq(irqm->irq_num[i]);
			break;
		}
	}
}

static DECLARE_IRQ_HANDLER(f11);
static DECLARE_IRQ_HANDLER(f12);

static irqreturn_t (*handler[IRQ_NAME_NUM])(int irq, void *dev_id) = {
	&DECLARE_IRQ_HANDLER_NAME(f11),
	&DECLARE_IRQ_HANDLER_NAME(f12),
};

enum RETURN irq_request_init(struct irq_module *irq)
{
	int i, deb;
	int ret = RETURN_SUCCESS;
	struct irq_module_name  *name = irq->name = irq_namespace_list;

	KP_INF("[%s] start\n", __func__);

	irq->irq_node = of_find_compatible_node(NULL, NULL, DRV_COMPATIBLE_MATCH);
	if (!irq->irq_node) {
		KP_ERR("[%s] null irq node!\n", __func__);
		ret = RETURN_ERROR;
		goto err;
	}

	for (i = 0; i < IRQ_NAME_NUM; i++, name++) {
		/* Parse the irq number */
		if (irq->irq_num[i] = irq_of_parse_and_map(irq->irq_node, i))
			KP_DBG("[%s] irq(%d) number is %d\n", __func__, i, irq->irq_num[i]);
		else {
			KP_ERR("[%s] irq(%d) parse error!", __func__, i);
			ret = RETURN_ERROR;
			goto err;
		}

		/* Parse the irq name */
		if (of_property_read_string_index(irq->irq_node,
						"interrupt-names",
						i, &name->irq_name_space)) {
			KP_ERR("[%s] irq(%d) name parse error!", __func__, i);
			ret = RETURN_ERROR;
			goto err;
		} else
			KP_DBG("[%s] irq(%d) name is %s\n", __func__, i, name->irq_name_space);

		/* Parse the irq source */
		if (of_property_read_u32_index(irq->irq_node, "source", i, &irq->irq_source[i])) {
			KP_ERR("[%s] irq(%d) source parse error!\n", __func__, i);
			ret = RETURN_ERROR;
			goto err;
		} else
			KP_DBG("[%s] irq(%d) source is %d\n", __func__, i, irq->irq_source[i]);

		/* Parse the irq debounce */
		if (of_property_read_u32_index(irq->irq_node, "debounce", i, &deb)) {
			KP_ERR("[%s] irq(%d) debounce parse error!", __func__, i);
			ret = RETURN_ERROR;
			goto err;
		} else
			KP_DBG("[%s] irq(%d) debounce is %d\n", __func__, i, deb);

		/* Bind irq source and debounce */
		gpio_request(irq->irq_source[i], name->irq_name_space);
		gpio_set_debounce(irq->irq_source[i], deb);

		/* Allocate handler for each irq */
		irq->eint_handler[i] = handler[i];

		/* Request irq for device */
		if (request_irq(irq->irq_num[i],
				irq->eint_handler[i], IRQF_TRIGGER_LOW,
				name->irq_name_space, irq)) {
			KP_ERR("[%s] irq(%d) is not available\n", __func__, i);
			ret = RETURN_ERROR;
			goto err;
		}
	}

err:
	return ret;
}

