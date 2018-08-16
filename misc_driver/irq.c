/*
 * Vanzo Misc Device Driver --- irq subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#include "core.h"

static int current_irq_num;
static struct irq_module_name irq_namespace_list[VANZO_IRQ_NAME_NUM] = { NULL };

static void global_tasklet_handler(unsigned long data)
{
	int i;
	struct vanzo_drv_struct *v    = v_global;
	struct irq_module       *irqm = &v->irq;

	KP_INF("[%s] start\n", __func__);

	for (i = 0; i < VANZO_IRQ_NAME_NUM; i++) {
		if (current_irq_num == irqm->irq_num[i]) {
			switch (irqm->irq_state[i]) {
				case IRQ_STATE_UP:
					irq_set_irq_type(irqm->irq_num[i], IRQF_TRIGGER_HIGH);
					irqm->irq_state[i] = IRQ_STATE_DOWN;
					input_event_state_report(&v->input, input_event_list[i], KEY_STATE_DOWN);
					KP_DBG("[%s] irq(%d) state down\n", __func__, irqm->irq_num[i]);
					break;

				case IRQ_STATE_DOWN:
					irq_set_irq_type(irqm->irq_num[i], IRQF_TRIGGER_LOW);
					irqm->irq_state[i] = IRQ_STATE_UP;
					input_event_state_report(&v->input, input_event_list[i], KEY_STATE_UP);
					KP_DBG("[%s] irq(%d) state up\n", __func__, irqm->irq_num[i]);
					break;
			}

			enable_irq(irqm->irq_num[i]);
			break;
		}
	}
}

static DECLARE_TASKLET(global_tasklet, global_tasklet_handler, 0);

static irqreturn_t irq_handler(int irq, void *dev_id)
{
	current_irq_num = irq;
	tasklet_schedule(&global_tasklet);

	return IRQ_HANDLED;
}

static irqreturn_t key_f11_eint_handler(int irq, void *dev_id)
{
	disable_irq_nosync(irq);
	current_irq_num = irq;
	tasklet_schedule(&global_tasklet);

	return IRQ_HANDLED;
}

static irqreturn_t key_f12_eint_handler(int irq, void *dev_id)
{
	disable_irq_nosync(irq);
	current_irq_num = irq;
	tasklet_schedule(&global_tasklet);

	return IRQ_HANDLED;
}

static irqreturn_t (*handler[VANZO_IRQ_NAME_NUM])(int irq, void *dev_id) = {
	&key_f11_eint_handler,
	&key_f12_eint_handler,
};

enum RETURN irq_request_init(struct irq_module *irq)
{
	int i, deb;
	int ret = RETURN_SUCCESS;
	struct vanzo_drv_struct *v    = container_of(irq, struct vanzo_drv_struct, irq);
	struct irq_module_name  *name = irq->name = irq_namespace_list;

	KP_INF("[%s] start\n", __func__);

	irq->irq_node = of_find_compatible_node(NULL, NULL, VANZO_DRV_COMPATIBLE_MATCH);
	if (!irq->irq_node) {
		KP_ERR("[%s] null irq node!\n", __func__);
		ret = RETURN_ERROR;
		goto err;
	}

	for (i = 0; i < VANZO_IRQ_NAME_NUM; i++, name++) {
		/* Parse the irq number */
		if (irq->irq_num[i] = irq_of_parse_and_map(irq->irq_node, i))
			KP_DBG("[%s] irq(%d) num is %d\n", __func__, i, irq->irq_num[i]);
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
		} else {
			KP_DBG("[%s] irq(%d) name is %s\n", __func__, i, name->irq_name_space);
		}

		/* Parse the irq debounce */
		if (of_property_read_u32_index(irq->irq_node, "debounce", i, &deb)) {
			KP_ERR("[%s] irq(%d) debounce parse error!", __func__, i);
			ret = RETURN_ERROR;
			goto err;
		} else {
			KP_DBG("[%s] irq(%d) debounce is %d\n", __func__, i, deb);
			gpio_request(irq->irq_num[i], name->irq_name_space);
			gpio_set_debounce(irq->irq_num[i], deb);
		}

		irq->eint_handler[i] = handler[i];
#if 1
		if (request_irq(irq->irq_num[i],
				irq->eint_handler[i], IRQF_TRIGGER_LOW,
				name->irq_name_space, NULL)) {
#else
		if (request_irq(irq->irq_num[i],
				irq_handler, IRQF_TRIGGER_LOW,
				name->irq_name_space, NULL)) {
#endif
			KP_ERR("[%s] irq(%d) is not available\n", __func__, i);
			ret = RETURN_ERROR;
			goto err;
		}
	}

err:
	return ret;
}

