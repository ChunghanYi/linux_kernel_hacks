/* **************** LDD:2.0 s_11/hrexamp.c **************** */
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
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>

static struct kt_data {
	struct hrtimer timer;
	ktime_t period;
} *data;

static enum hrtimer_restart ktfun(struct hrtimer *var)
{
	ktime_t now = var->base->get_time();
	pr_info("timer running at jiffies=%ld\n", jiffies);
	hrtimer_forward(var, now, data->period);
	return HRTIMER_RESTART;
}

static int __init my_init(void)
{
	data = kmalloc(sizeof(*data), GFP_KERNEL);
	data->period = ktime_set(1, 0);	/* short period, 1 second  */
	hrtimer_init(&data->timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
	data->timer.function = ktfun;
	hrtimer_start(&data->timer, data->period, HRTIMER_MODE_REL);

	return 0;
}

static void __exit my_exit(void)
{
	hrtimer_cancel(&data->timer);
	kfree(data);
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL v2");
