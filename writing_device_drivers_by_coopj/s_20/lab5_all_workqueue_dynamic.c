/* **************** LDD:2.0 s_20/lab5_all_workqueue_dynamic.c **************** */
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
 * Sharing All Interrupts, Bottom Halves, Producer/Consumer Problem
 * (workqueue solution)
 *
 * Find solutions for the producer/consumer problem for the previous
 * lab.
 @*/

#include <linux/module.h>
#include "lab_all_interrupt.h"

struct my_dat {
	int irq;
	struct work_struct work;
};

static void w_fun(struct work_struct *w_arg)
{
	struct my_dat *data = container_of(w_arg, struct my_dat, work);
	atomic_inc(&bhs[data->irq]);
	kfree(data);
}

static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	struct my_dat *data =
	    (struct my_dat *)kmalloc(sizeof(struct my_dat), GFP_ATOMIC);

	INIT_WORK(&data->work, w_fun);
	data->irq = irq;
	atomic_inc(&interrupts[data->irq]);
	schedule_work(&data->work);
	mdelay(delay);

	/* we return IRQ_NONE because we are just observing */
	return IRQ_NONE;
}

module_init(my_generic_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_20/lab5_all_workqueue_dynamic.c");
MODULE_LICENSE("GPL v2");
