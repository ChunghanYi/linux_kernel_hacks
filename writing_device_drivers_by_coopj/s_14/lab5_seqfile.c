/* **************** LDD:2.0 s_14/lab5_seqfile.c **************** */
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
 * Using seq_file for the proc filesystem.
 *
 * Take the simple "x_busy" proc entry discussed earlier, and
 * re-implement it using the seq_file interface.
 *
 * As a parameter, input the number of lines to print out.
 *
 @*/

#include <linux/module.h>
#include <linux/sched.h>	/* Get "jiffies" from here */
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/seq_file.h>

static int items = 1;
static int x_delay = 1;
static unsigned long future;

static char const my_proc[] = { "x_busy" };

/* Sequential file iterator                                              */

static void *x_busy_seq_start(struct seq_file *sf, loff_t * pos)
{
	void *results;

	if (*pos < items) {
		future = jiffies + x_delay * HZ;
		while (time_before(jiffies, future)) ;
		results = (void *)&jiffies;
	} else {
		results = NULL;
	}
	return results;
}

static void *x_busy_seq_next(struct seq_file *sf, void *v, loff_t * pos)
{
	void *results;

	(*pos)++;
	if (*pos < items) {
		future = jiffies + x_delay * HZ;
		while (time_before(jiffies, future)) ;
		results = (void *)&jiffies;
	} else {
		results = NULL;
	}
	return results;
}

static void x_busy_seq_stop(struct seq_file *sf, void *v)
{
	/* Nothing to do here */
}

static int x_busy_seq_show(struct seq_file *sf, void *v	/* jiffies in disquise */
    )
{
	volatile unsigned long *const jp = (volatile unsigned long *)v;
	int results;

	seq_printf(sf, "jiffies = %lu.\n", *jp);
	results = 0;
	return results;
}

static struct seq_operations proc_x_busy_seq_ops = {
	.start = x_busy_seq_start,
	.next = x_busy_seq_next,
	.stop = x_busy_seq_stop,
	.show = x_busy_seq_show,
};

static int proc_x_busy_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &proc_x_busy_seq_ops);
}

static const struct file_operations proc_x_busy_operations = {
	.open = proc_x_busy_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

static struct proc_dir_entry *x_proc_busy;

static int __init my_init(void)
{
	int results;

	results = -1;
	do {
		x_proc_busy = proc_create(my_proc, 0, NULL, &proc_x_busy_operations);
		if (!x_proc_busy) {
			break;
		}
		results = 0;
	} while (0);
	return results;
}

static void __exit my_exit(void)
{
	if (x_proc_busy) {
		remove_proc_entry(my_proc, NULL);
	}
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Tommy Reynolds");
MODULE_AUTHOR("Jerry Cooperstein");
MODULE_AUTHOR("Chunghan Yi");
MODULE_DESCRIPTION("LDD:2.0 s_14/lab5_seqfile.c");
MODULE_LICENSE("GPL v2");
module_param(items, int, S_IRUGO);
MODULE_PARM_DESC(items, "How many items to simulate");
