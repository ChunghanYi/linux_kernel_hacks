/* **************** LDD:2.0 s_08/lab2_interrupt.c **************** */
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
 * Sharing All Interrupts  (Kernel Module)
 *
 * Extend the previous solution to construct a character driver that
 * shares every possible interrupt with already installed handlers.
 *
 * The highest interrupt number you have to consider will depend on
 * your kernel and platform; look at /proc/interrupts to ascertan what
 * is necessary.  You can also use the parameter nr_irqs but that is
 * likely to be much larger than the highest number you have active.
 *
 * Take particular care when you call free_irq() as it is very easy to
 * freeze your system if you are not careful.
 *
 * The character driver can be very simple; for instance if no open()
 * and release() methods are specified, success is the default.
 *
 * A read() on the device should return a brief report on the total
 * number of interrupts handled for each IRQ.
 *
 * To do this you'll also have to write a short application to
 * retrieve and print out the data.  (Don't forget to create the
 * device node before you run the application.)
 *
 @*/
#include <linux/module.h>
#include <linux/interrupt.h>

/* either of these (but not both) will work */

//#include "lab_char.h"
#include "lab_miscdev.h"

//#define MAXIRQS 256
#define MAXIRQS nr_irqs

#define NCOPY (MAXIRQS * sizeof(int))

static int *interrupts;

static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	interrupts[irq]++;
	return IRQ_NONE;	/* we return IRQ_NONE because we are just observing */
}

static void freeup_irqs(void)
{
	int irq;
	for (irq = 0; irq < MAXIRQS; irq++) {
		if (interrupts[irq] >= 0) {	/* if greater than 0, was able to share */
			synchronize_irq(irq);
			pr_info("Freeing IRQ= %4d, which had %10d events\n",
				irq, interrupts[irq]);
			free_irq(irq, interrupts);
		}
	}
}

static void get_irqs(void)
{
	int irq;
	interrupts = (int *)ramdisk;

	for (irq = 0; irq < MAXIRQS; irq++) {
		interrupts[irq] = -1;	/* set to -1 as a flag */
		if (!request_irq
		    (irq, my_interrupt, IRQF_SHARED, "my_int", interrupts)) {
			interrupts[irq] = 0;
			pr_info("Succeded in registering IRQ=%d\n", irq);
		}
	}
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_generic_read,
};

static int __init my_init(void)
{
	int rc = my_generic_init();
	if (!rc)
		get_irqs();
	return rc;
}

static void __exit my_exit(void)
{
	freeup_irqs();
	my_generic_exit();
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_08/lab2_interrupt.c");
MODULE_LICENSE("GPL v2");
