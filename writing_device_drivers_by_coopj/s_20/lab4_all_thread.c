/* **************** LDD:2.0 s_20/lab4_all_thread.c **************** */
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
 * Sharing All Interrupts, Bottom Halves (thread solution)
 *
 * Extend the solution to share all possible interrupts, and evaluate
 * the consumer/producer problem.
 *
 @*/
#include <linux/module.h>
#include "lab_all_interrupt.h"

static atomic_t cond;
static struct task_struct *tsk;

static DECLARE_WAIT_QUEUE_HEAD(wq);

static struct my_dat {
	int irq;
} my_data;

static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	struct my_dat *data = (struct my_dat *)&my_data;
	data->irq = irq;
	atomic_set(&cond, 1);
	atomic_inc(&interrupts[irq]);
	mdelay(delay);
	wake_up_interruptible(&wq);
	/* we return IRQ_NONE because we are just observing */
	return IRQ_NONE;
}

static int thr_fun(void *thr_arg)
{
	struct my_dat *data = (struct my_dat *)thr_arg;

	/* go into a loop and deal with events as they come */
	pr_info("Beginning the thread loop for name=thr_fun\n");

	do {
		int irq = data->irq;
		atomic_set(&cond, 0);
		wait_event_interruptible(wq, kthread_should_stop()
					 || atomic_read(&cond) == 1);
		if (atomic_read(&cond) == 1)
			atomic_inc(&bhs[irq]);
	} while (!kthread_should_stop());
	return 0;
}

static int __init my_init(void)
{
	atomic_set(&cond, 1);

	if (!(tsk = kthread_run(thr_fun, (void *)&my_data, "thr_fun"))) {
		pr_info("Failed to generate a kernel thread\n");
		return -1;
	}
	return my_generic_init();
}

static void __exit my_exit(void)
{
	kthread_stop(tsk);
	my_generic_exit();
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_20/lab4_all_thread.c");
MODULE_LICENSE("GPL v2");
