/*
 * Vanzo Misc Device Driver --- file subsystem
 *
 * Copyright (C) 2018 Sam Morgan Jiang  (sam.morgan.jiang@outlook.com)
 *
 * Licensed under GPLv2 or later.
 */

#ifndef __VANZO_MISC_FILE_CTRL_H__
#define __VANZO_MISC_FILE_CTRL_H__

#define FILE_SET_VALUE_SIZE             3

struct file_set {
	unsigned char name;
	int value[FILE_SET_VALUE_SIZE];
};

enum RETURN proc_file_init(struct class *class);


#endif
