/* **************** LDD:2.0 s_11/lab3_periodic_timer_new.c **************** */
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

/*
 * Tested under the linux kernel 4.19.94
 */

#include <linux/module.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/init.h>

static struct kt_data {
	unsigned long period;
	unsigned long start_time;	/* jiffies value when we first started the timer */
	unsigned long timer_start;	/* jiffies when timer was queued */
	unsigned long timer_end;	/* jiffies when timer is executed */
	struct timer_list timer;
} data;

static void ktfun(struct timer_list *t)
{
	struct kt_data *tdata = from_timer(tdata, t, timer);	
	pr_info("ktimer: period = %ld  elapsed = %ld\n",
		tdata->period, jiffies - tdata->start_time);
	/* resubmit */
	mod_timer(&tdata->timer, jiffies + tdata->period);
}

static int __init my_init(void)
{
	data.period = 2 * HZ;	/* short period,   2 secs */
	data.start_time = jiffies;
	timer_setup(&data.timer, ktfun, 0);

	mod_timer(&data.timer, jiffies + data.period);

	return 0;
}

static void __exit my_exit(void)
{
	/* delete any running timers */
	pr_info("Deleted time,r rc = %d\n", del_timer_sync(&data.timer));
	pr_info("Module successfully unloaded \n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_AUTHOR("Chunghan Yi");
MODULE_DESCRIPTION("LDD:2.0 s_11/lab3_periodic_timer_new.c");
MODULE_LICENSE("GPL v2");
