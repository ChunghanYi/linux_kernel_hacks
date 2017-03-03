/* **************** LDD:2.0 s_20/lab3_one_workqueue_dynamic.c **************** */
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
 * Producer/Consumer (workqueue Solution)
 *
 * You may have noticed that
 * you lost some bottom halves. This will happen when more than one
 * interrupt arrives before bottom halves are accomplished. For
 * instance, the same tasklet can only be queued up twice.
 *
 * Write a bottom half that can "catch up"; i.e., consume more than
 * one event when it is called, cleaning up the pending queue.  Do
 * this for at least one of the previous solutions.
 @*/

#include <linux/module.h>
#include "lab_one_interrupt.h"

static void w_fun(struct work_struct *w_arg)
{
	struct my_dat *data = container_of(w_arg, struct my_dat, work);

	atomic_inc(&counter_bh);
	pr_info("In BH: counter_th = %d, counter_bh = %d, jiffies=%ld, %ld\n",
		atomic_read(&counter_th), atomic_read(&counter_bh),
		data->jiffies, jiffies);
	kfree(data);
}

static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	struct my_dat *data =
	    (struct my_dat *)kmalloc(sizeof(struct my_dat), GFP_ATOMIC);
	data->jiffies = jiffies;

	INIT_WORK(&data->work, w_fun);
	atomic_inc(&counter_th);
	schedule_work(&data->work);
	mdelay(delay);		/* hoke up a delay to try to cause pileup */
	return IRQ_NONE;	/* we return IRQ_NONE because we are just observing */
}

module_init(my_generic_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_20/lab3_one_workqueue_dynamic.c");
MODULE_LICENSE("GPL v2");
