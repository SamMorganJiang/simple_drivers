/*
 * Vanzo Misc Device Driver --- irq subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#ifndef __VANZO_MISC_INTERRUPT_CTRL_H__
#define __VANZO_MISC_INTERRUPT_CTRL_H__

#define VANZO_IRQ_NAME_NUM        2

struct irq_module_name {
	char *irq_name_space;
};

struct irq_module {
	int                     irq_num[VANZO_IRQ_NAME_NUM];
	int                     irq_state[VANZO_IRQ_NAME_NUM];
	struct irq_module_name  *name;
	struct device_node      *irq_node;

	irqreturn_t (*eint_handler[VANZO_IRQ_NAME_NUM])(int irq, void *dev_id);
};

enum IRQ_STATE {
	IRQ_STATE_UP    = 0,
	IRQ_STATE_DOWN  = 1
};

enum RETURN irq_request_init(struct irq_module *irq);


#endif
