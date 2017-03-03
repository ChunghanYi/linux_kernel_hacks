/* **************** LDD:2.0 s_11/lab5_hrtimer.c **************** */
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
 * High Resolution Timers
 *
 * Do the same things as in the first exercise, setting up two
 * periodic timers, but this time use the hrtimer interface.
 @*/

#include <linux/module.h>
#include <linux/kernel.h>	/* for container_of */
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/slab.h>

static struct my_data {
	ktime_t period;
	int period_in_secs;
	unsigned long start_time;	/* jiffies */
	struct hrtimer timer;
	char struct_id;		/* 'A' or 'B' */
} *data_array;			/* will kmalloc() an array of these */

/*
 *  my_timer_func -- the timer is embedded inside an instance of
 *    "struct my_data".  Use the "container_of" address-arithmetic
 *    macro to recover a pointer to the containing structure.
 */
static enum hrtimer_restart my_timer_func(struct hrtimer *var)
{
	struct my_data *dat = container_of(var, struct my_data, timer);
	ktime_t now;

	pr_info("%c: period = %d  elapsed = %ld\n",
		dat->struct_id, dat->period_in_secs, jiffies - dat->start_time);

	now = var->base->get_time();
	dat->start_time = jiffies;
	hrtimer_forward(var, now, dat->period);
	return HRTIMER_RESTART;
}

static int __init my_init(void)
{
	int i;
	struct my_data *d;

	data_array = kmalloc(2 * sizeof(struct my_data), GFP_KERNEL);

	for (d = data_array, i = 0; i < 2; i++, d++) {
		d->period_in_secs = (i == 0) ? 1 : 10;
		d->period = ktime_set(d->period_in_secs, 0);
		d->struct_id = 'A' + i;

		hrtimer_init(&d->timer, CLOCK_REALTIME, HRTIMER_MODE_REL);

		d->timer.function = my_timer_func;
		d->start_time = jiffies;

		hrtimer_start(&d->timer, d->period, HRTIMER_MODE_REL);
	}
	pr_info("Module loaded, two HRTimers started\n");

	return 0;
}

static void __exit my_exit(void)
{
	int i;
	struct my_data *d = data_array;

	for (i = 0; i < 2; i++, d++) {
		pr_info("deleted timer %c:  rc = %d\n",
			d->struct_id, hrtimer_cancel(&d->timer));
	}
	kfree(data_array);
	pr_info("Module unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_AUTHOR("Bill Kerr");
MODULE_DESCRIPTION("LDD:2.0 s_11/lab5_hrtimer.c");
MODULE_LICENSE("GPL v2");
