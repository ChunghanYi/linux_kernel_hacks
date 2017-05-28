/* ************ LDD4EP: listing3-4: helloworld_with_timing.c ************ */
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

#include <linux/module.h>
#include <linux/time.h>

static int num = 10;
static struct timeval start_time;

module_param(num, int, S_IRUGO);

static void say_hello(void)
{
	int i;

	for (i = 1; i <= num; i++)
		pr_info("[%d/%d] Hello!\n", i, num);
}

static int __init first_init(void)
{
	do_gettimeofday(&start_time);
	pr_info("Loading first!\n");
	say_hello();
	return 0;
}

static void __exit first_exit(void)
{
	struct timeval end_time;

	do_gettimeofday(&end_time);
	pr_info("Unloading module after %ld seconds\n",
			end_time.tv_sec - start_time.tv_sec);
	say_hello();
}

module_init(first_init);
module_exit(first_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a module that will print the time since it was loaded");
