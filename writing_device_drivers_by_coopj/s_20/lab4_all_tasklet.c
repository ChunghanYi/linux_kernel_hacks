/* **************** LDD:2.0 s_20/lab4_all_tasklet.c **************** */
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
 * Sharing All Interrupts, Bottom Halves (tasklet solution)
 *
 * Extend the solution to share all possible interrupts, and evaluate
 * the consumer/producer problem.
 *
 @*/
#include <linux/module.h>
#include "lab_all_interrupt.h"

static struct my_dat {
	int irq;
} my_data;

static void t_fun(unsigned long t_arg)
{
	struct my_dat *data = (struct my_dat *)t_arg;
	atomic_inc(&bhs[data->irq]);
}

static DECLARE_TASKLET(t, t_fun, (unsigned long)&my_data);

static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	struct my_dat *data = (struct my_dat *)&my_data;
	data->irq = irq;
	atomic_inc(&interrupts[irq]);
	tasklet_schedule(&t);
	mdelay(delay);
	/* we return IRQ_NONE because we are just observing */
	return IRQ_NONE;
}

module_init(my_generic_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_20/lab4_all_tasklet.c");
MODULE_LICENSE("GPL v2");
