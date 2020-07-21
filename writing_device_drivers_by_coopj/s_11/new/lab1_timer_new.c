/* **************** LDD:2.0 s_11/lab1_timer_new.c **************** */
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

/*
 * Tested under the linux kernel 4.19.94
 */

#include <linux/module.h>
#include <linux/timer.h>


/* either of these (but not both) will work */
//#include "lab_char.h"
#include "lab_miscdev.h"

static struct wrapper {
	struct timer_list my_timer;
} wr;

void my_timer_function(struct timer_list *t)
{
	struct wrapper *wrdata = from_timer(wrdata, t, my_timer);
	pr_info("I am in my_timer_fun, jiffies = %ld\n", jiffies);
	pr_info(" I think my current task pid is %d\n", (int)current->pid);
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	static int len = 100;
	int retval;
	pr_info(" Entering the WRITE function\n");
	pr_info(" my current task pid is %d\n", (int)current->pid);
	timer_setup(&wr.my_timer, my_timer_function, 0);

	retval = mod_timer(&wr.my_timer, jiffies + HZ);
	if (retval)
		pr_info("Timer firing failed\n");
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
	pr_info("Deleted time,r rc = %d\n", del_timer_sync(&wr.my_timer));
	my_generic_exit();
}

module_init(my_generic_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_AUTHOR("Chunghan Yi");
MODULE_DESCRIPTION("LDD:2.0 s_11/lab1_timer_new.c");
MODULE_LICENSE("GPL v2");
