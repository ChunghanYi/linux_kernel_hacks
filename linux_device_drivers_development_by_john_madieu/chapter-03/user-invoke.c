/* ************ LDDD: chapter-03: user-invoke.c ************ */
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
#include <linux/module.h>
#include <linux/workqueue.h> /* for work queue */
#include <linux/kmod.h>

static struct delayed_work initiate_shutdown_work;

static void delayed_shutdown(struct work_struct *work)
{
	char *cmd = "/sbin/shutdown";
	char *argv[] = {
		cmd,
		"-h",
		"now",
		NULL,
	};
	char *envp[] = {
		"HOME=/",
		"PATH=/sbin:/bin:/usr/sbin:/usr/bin",
		NULL,
	};

	call_usermodehelper(cmd, argv, envp, 0);
}

static int __init my_shutdown_init(void)
{
	INIT_DELAYED_WORK(&initiate_shutdown_work, delayed_shutdown);
	schedule_delayed_work(&initiate_shutdown_work, msecs_to_jiffies(200));
	return 0;
}

static void __exit my_shutdown_exit(void)
{
	return;
}

module_init(my_shutdown_init);
module_exit(my_shutdown_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
MODULE_DESCRIPTION("Simple module that trigger a delayed shut down");
