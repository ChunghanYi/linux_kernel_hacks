/* **************** LDD:2.0 s_20/lab1_workqueue_shared.c **************** */
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
 * Deferred Functions (shared work queue solution)
 *
 * Write a driver that schedules a deferred function whenever a
 * write() to the device takes place.
 *
 * Pass some data to the driver and have it print out.
 *
 * Have it print out the current->pid field when the tasklet is
 * scheduled, and then again when the queued function is executed.
 *
 * Implement this using:
 *
 * tasklets
 *
 * work queues (using the system shared workqueue)
 *
 * work queues (using your own created workqueue)
 *
 * work queues (using your own created workqueue and the new API)
 *
 * You can use the same testing programs you used in the sleep
 * exercises.
 *
 * Try scheduling multiple deferred functions
 * and see if they come out in LIFO or FIFO order.  What happens if you
 * try to schedule the deferred function more than once?
 *
 @*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/workqueue.h>

/* either of these (but not both) will work */
//#include "lab_char.h"
#include "lab_miscdev.h"

static struct my_dat {
	atomic_t len;
	struct work_struct work;
} my_data;

static void w_fun(struct work_struct *w_arg)
{
	struct my_dat *data = container_of(w_arg, struct my_dat, work);
	pr_info("I am in w_fun, jiffies = %ld\n", jiffies);
	pr_info(" I think my current task pid is %d\n", (int)current->pid);
	pr_info(" my data is: %d\n", atomic_read(&data->len));
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	struct my_dat *data = (struct my_dat *)&my_data;
	pr_info(" Entering the WRITE function\n");
	pr_info(" my current task pid is %d\n", (int)current->pid);
	pr_info("about to schedule workqueue,  jiffies=%ld\n", jiffies);
	schedule_work(&data->work);
	pr_info(" i queued the task, jiffies=%ld\n", jiffies);
	atomic_add(100, &data->len);
	return lbuf;
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_generic_read,
	.write = mycdrv_write,
	.open = mycdrv_generic_open,
	.release = mycdrv_generic_release,
};

static int __init my_init(void)
{
	struct my_dat *data = (struct my_dat *)&my_data;
	atomic_set(&data->len, 100);
	INIT_WORK(&data->work, w_fun);
	return my_generic_init();
}

module_init(my_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_20/lab1_workqueue_shared.c");
MODULE_LICENSE("GPL v2");
