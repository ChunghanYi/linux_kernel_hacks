/* **************** LDD:2.0 s_08/lab1_interrupt.c **************** */
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
 * Shared Interrupts
 *
 * Write a module that shares its IRQ with your network card.  You can
 * generate some network interrupts either by browsing or pinging.
 * (If you have trouble with the network driver, try using the mouse
 * interrupt.)
 *
 * Check /proc/interrupts while it is loaded.
 *
 * Have the module keep track of the number of times the interrupt
 * handler gets called.
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#define SHARED_IRQ 17
static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;
module_param(irq, int, S_IRUGO);

static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	irq_counter++;
	pr_info("In the ISR: counter = %d\n", irq_counter);
	return IRQ_NONE;	/* we return IRQ_NONE because we are just observing */
}

static int __init my_init(void)
{
	if (request_irq
	    (irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id)) {
		pr_info("Failed to reserve irq %d\n", irq);
		return -1;
	}
	pr_info("Successfully loading ISR handler\n");
	return 0;
}

static void __exit my_exit(void)
{
	synchronize_irq(irq);
	free_irq(irq, &my_dev_id);
	pr_info("Successfully unloading,  irq_counter = %d\n", irq_counter);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_08/lab1_interrupt.c");
MODULE_LICENSE("GPL v2");
