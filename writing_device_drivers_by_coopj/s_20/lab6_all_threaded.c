/* **************** LDD:2.0 s_20/lab6_all_threaded.c **************** */
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
 * Threaded Interrupt Handlers (all interrupts case)
 * 
 * If you are running a kernel version 2.6.30 or later, solve the
 * producer/consumer problem with a threaded interrupt handler.
 *
 * There are two types of solutions presented, one for just one shared
 * interrupt, one sharing them all, with the same delay parameter as
 * used in the earlier exercises.
 @*/

#include <linux/module.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30)
static int __init my_init(void)
{
	pr_warning("Threaded interrupts don't appear until 2.6.30\n");
	return -1;
}

module_init(my_init)
#else

#define THREADED_IRQ

#include "lab_all_interrupt.h"

static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	atomic_inc(&interrupts[irq]);
	atomic_inc(&nevents[irq]);
	mdelay(delay);
	return IRQ_WAKE_THREAD;
}

static irqreturn_t thread_fun(int irq, void *thr_arg)
{
	do {
		atomic_inc(&bhs[irq]);
	}
	while (!atomic_dec_and_test(&nevents[irq]));
	if (atomic_read(&bhs[irq]) != atomic_read(&interrupts[irq]))
		pr_info("irq=%5d,th=%6d bh=%6d\n", irq,
			atomic_read(&interrupts[irq]), atomic_read(&bhs[irq]));
	/* we return IRQ_NONE because we are just observing */
	return IRQ_NONE;
}

module_init(my_generic_init);
module_exit(my_generic_exit);
#endif

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_20/lab6_all_threaded.c");
MODULE_LICENSE("GPL v2");
