/* **************** LDD:2.0 s_20/lab2_one_tasklet.c **************** */
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
 * Shared Interrupts and Bottom Halves (tasklet solution)
 *
 * Write a module that shares its IRQ with your network card.  You can
 * generate some network interrupts either by browsing or pinging.
 *
 * Make it use a top half and a bottom half.
 *
 * Check /proc/interrupts while it is loaded.
 *
 * Have the module keep track of the number of times the interrupt's
 * halves are called.
 *
 * Implement the bottom half using:
 *
 * tasklets.
 *
 * work queues
 *
 * A background thread which you launch during the module's
 * initialization, which gets woken up anytime data is available.
 * Make sure you kill the thread when you unload the module, or it may
 * stay in a zombie state forever.
 *
 * For any method you use does, are the bottom and top halves called
 * an equal number of times?  If not why, and what can you do about
 * it?
 *
 * Note: the solutions introduce a delay parameter which can be set
 * when loading the module; this will introduce a delay of that many
 * milliseconds in the top half, which will provoke dropping even more
 * bottom halves, depending on the method used.x
 *
@*/

#include <linux/module.h>
#include "lab_one_interrupt.h"

static void t_fun(unsigned long t_arg)
{
	struct my_dat *data = (struct my_dat *)t_arg;
	atomic_inc(&counter_bh);
	pr_info(
	       "In BH: counter_th = %d, counter_bh = %d, jiffies=%ld, %ld\n",
	       atomic_read(&counter_th), atomic_read(&counter_bh),
	       data->jiffies, jiffies);
}

static DECLARE_TASKLET(t_name, t_fun, (unsigned long)&my_data);

/* initialize tasklet */
static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	struct my_dat *data = (struct my_dat *)dev_id;
	atomic_inc(&counter_th);
	data->jiffies = jiffies;
	tasklet_schedule(&t_name);
	mdelay(delay);		/* hoke up a delay to try to cause pileup */
	return IRQ_NONE;	/* we return IRQ_NONE because we are just observing */
}

module_init(my_generic_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_20/lab2_one_tasklet.c");
MODULE_LICENSE("GPL v2");
