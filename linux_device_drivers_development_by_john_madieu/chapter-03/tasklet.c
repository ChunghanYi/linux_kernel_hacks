/* ************ LDDD: chapter-03: tasklet.c ************ */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>    /* for tasklets api */

char tasklet_data[]="We use a string; but it could be pointer to a structure";

/* Tasklet handler, that just print the data */
void tasklet_function(unsigned long data)
{
	pr_info("%s\n", (char *)data);
	return;
}

DECLARE_TASKLET(my_tasklet, tasklet_function, (unsigned long) tasklet_data);

static int __init my_init(void)
{
	/* Schedule the handler */
	tasklet_schedule(&my_tasklet);
	pr_info("tasklet example\n");
	return 0;
}

void my_exit(void)
{
	/* Stop the tasklet before we exit */
	tasklet_kill(&my_tasklet);
	pr_info("tasklet example cleanup\n");
	return;
}

module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_LICENSE("GPL");
