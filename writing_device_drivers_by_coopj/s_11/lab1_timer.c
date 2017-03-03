/* **************** LDD:2.0 s_11/lab1_timer.c **************** */
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
 * Kernel Timers from a Character Driver
 *
 * Write a driver that puts launches a kernel timer whenever a write
 * to the device takes place.
 *
 * Pass some data to the driver and have it print out.
 *
 * Have it print out the current->pid field when the timer function
 * is scheduled, and then again when the function is executed.
 *
 * You can use the same testing programs you used in the previous
 * exercise.
 @*/

#include <linux/module.h>
#include <linux/timer.h>

/* either of these (but not both) will work */
//#include "lab_char.h"
#include "lab_miscdev.h"

static struct timer_list my_timer;

static void my_timer_function(unsigned long ptr)
{
	pr_info("I am in my_timer_fun, jiffies = %ld\n", jiffies);
	pr_info(" I think my current task pid is %d\n", (int)current->pid);
	pr_info(" my data is: %d\n", (int)ptr);
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	static int len = 100;
	pr_info(" Entering the WRITE function\n");
	pr_info(" my current task pid is %d\n", (int)current->pid);
	init_timer(&my_timer);	/* intialize */
	my_timer.function = my_timer_function;
	my_timer.expires = jiffies + HZ;	/* one second delay */
	my_timer.data = len;
	add_timer(&my_timer);
	pr_info("Adding timer at jiffies = %ld\n", jiffies);
	len += 100;
	return mycdrv_generic_write(file, buf, lbuf, ppos);
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_generic_read,
	.write = mycdrv_write,
	.open = mycdrv_generic_open,
	.release = mycdrv_generic_release,
};

static void __exit my_exit(void)
{
	/* delete any running timers */
	pr_info("Deleted time,r rc = %d\n", del_timer_sync(&my_timer));
	my_generic_exit();
}

module_init(my_generic_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_11/lab1_timer.c");
MODULE_LICENSE("GPL v2");
