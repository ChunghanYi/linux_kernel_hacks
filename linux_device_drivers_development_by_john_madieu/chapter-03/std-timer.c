/* ************ LDDD: chapter-03: std-timer.c ************ */
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static struct timer_list my_timer;

void my_timer_callback(unsigned long data)
{
	pr_info("%s called (%ld).\n", __FUNCTION__, jiffies);
}
#else
static struct my_data {
	struct timer_list my_timer;
} data;

static void my_timer_callback(struct timer_list *t)
{
	struct my_data *dat = from_timer(dat, t, my_timer);
	pr_info("%s called (%ld).\n", __FUNCTION__, jiffies);
}
#endif

static int __init my_init(void)
{
	int retval;
	pr_info("Timer module loaded\n");

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	setup_timer(&my_timer, my_timer_callback, 0);
#else
	timer_setup(&data.my_timer, my_timer_callback, 0);
#endif
	pr_info("Setup timer to fire in 300ms (%ld)\n", jiffies);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	retval = mod_timer(&my_timer, jiffies + msecs_to_jiffies(300));
#else
	retval = mod_timer(&data.my_timer, jiffies + msecs_to_jiffies(300));
#endif
	if (retval)
		pr_info("Timer firing failed\n");

	return 0;
}

static void my_exit(void)
{
	int retval;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	retval = del_timer(&my_timer);
#else
	retval = del_timer(&data.my_timer);
#endif
	if (retval)
		pr_info("The timer is still in use...\n");

	pr_info("Timer module unloaded\n");
	return;
}

module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_AUTHOR("Chunghan Yi <chunghan.yi@gmail.com>");
MODULE_DESCRIPTION("Standard timer example");
MODULE_LICENSE("GPL");
