/* **************** LDD:2.0 s_20/mytasklet.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2012
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/init.h>

static void t_fun(unsigned long t_arg);

static struct simp {
	int i;
	int j;
} t_data;

static DECLARE_TASKLET(t_name, t_fun, (unsigned long)&t_data);

static int __init my_init(void)
{
	t_data.i = 100;
	t_data.j = 200;
	pr_info(" scheduling my tasklet, jiffies= %ld \n", jiffies);
	tasklet_schedule(&t_name);
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("\nHello: unloading module\n", cleanup_module);
}

static void t_fun(unsigned long t_arg)
{
	struct simp *datum;
	datum = (struct simp *)t_arg;
	pr_info("Entering t_fun, datum->i = %d, jiffies = %ld\n",
		datum->i, jiffies);
	pr_info("Entering t_fun, datum->j = %d, jiffies = %ld\n",
		datum->j, jiffies);
}

module_init(my_init);
module_exit(my_exit);
