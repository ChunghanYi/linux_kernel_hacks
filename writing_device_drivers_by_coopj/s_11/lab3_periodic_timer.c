/* **************** LDD:2.0 s_11/lab3_periodic_timer.c **************** */
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
/*
 * Periodic Kernel Timers
 *
 * Write a module that launches a periodic kernel timer function;
 * i.e., it should re-install itself.
 *
 @*/

#include <linux/module.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/init.h>

static struct timer_list timer;
static struct kt_data {
	unsigned long period;
	unsigned long start_time;	/* jiffies value when we first started the timer */
	unsigned long timer_start;	/* jiffies when timer was queued */
	unsigned long timer_end;	/* jiffies when timer is executed */
} data;

static void ktfun(unsigned long var)
{
	struct kt_data *tdata = (struct kt_data *)var;

	pr_info("ktimer: period = %ld  elapsed = %ld\n",
		tdata->period, jiffies - tdata->start_time);
	/* resubmit */
	mod_timer(&timer, tdata->period + jiffies);
}

static int __init my_init(void)
{

	data.period = 2 * HZ;	/* short period,   2 secs */

	init_timer(&timer);
	timer.function = ktfun;
	timer.data = (unsigned long)&data;
	timer.expires = jiffies + data.period;
	data.start_time = jiffies;
	add_timer(&timer);

	return 0;
}

static void __exit my_exit(void)
{
	/* delete any running timers */
	pr_info("Deleted time,r rc = %d\n", del_timer_sync(&timer));
	pr_info("Module successfully unloaded \n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_11/lab3_periodic_timer.c");
MODULE_LICENSE("GPL v2");
