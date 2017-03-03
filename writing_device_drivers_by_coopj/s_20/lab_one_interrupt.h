/* **************** LDD:2.0 s_20/lab_one_interrupt.h **************** */
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
#ifndef _LAB_ONE_INTERRUPT_H
#define _LAB_ONE INTERRUPT_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/slab.h>

#define SHARED_IRQ 19
static int irq = SHARED_IRQ;
module_param(irq, int, S_IRUGO);

/* default delay time in top half -- try 10 to get results */
static int delay = 0;
module_param(delay, int, S_IRUGO);

static atomic_t counter_bh, counter_th;

struct my_dat {
	unsigned long jiffies;	/* used for timestamp */
	struct tasklet_struct tsk;	/* used in dynamic tasklet solution */
	struct work_struct work;	/* used in dynamic workqueue solution */
};
static struct my_dat my_data;

static irqreturn_t my_interrupt(int irq, void *dev_id);
#ifdef THREADED_IRQ
static irqreturn_t thread_fun(int irq, void *thr_arg);
#endif

static int __init my_generic_init(void)
{
	atomic_set(&counter_bh, 0);
	atomic_set(&counter_th, 0);

	/* use my_data for dev_id */

#ifdef THREADED_IRQ
	if (request_threaded_irq(irq, my_interrupt, thread_fun, IRQF_SHARED,
				 "my_int", &my_data))
#else
	if (request_irq(irq, my_interrupt, IRQF_SHARED, "my_int", &my_data))
#endif
	{
		pr_warning("Failed to reserve irq %d\n", irq);
		return -1;
	}
	pr_info("successfully loaded\n");
	return 0;
}

static void __exit my_generic_exit(void)
{
	synchronize_irq(irq);
	free_irq(irq, &my_data);
	pr_info(" counter_th = %d,  counter_bh = %d\n",
		atomic_read(&counter_th), atomic_read(&counter_bh));
	pr_info("successfully unloaded\n");
}

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_20/lab_one_interrupt.h");
MODULE_LICENSE("GPL v2");

#endif
