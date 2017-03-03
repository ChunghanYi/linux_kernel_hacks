/* **************** LDD:2.0 s_14/x_busy.c **************** */
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
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/jiffies.h>

static int x_delay = 1;		/* the default delay */

static int
x_read_busy(char *buf, char **start, off_t offset, int len,
	    int *eof, void *unused)
{
	unsigned long j = jiffies + x_delay * HZ;

	while (time_before(jiffies, j))
		/* nothing */ ;
	*eof = 1;
	return sprintf(buf, "jiffies = %d\n", (int)jiffies);
}

static struct proc_dir_entry *x_proc_busy;

static int __init my_init(void)
{
	x_proc_busy = create_proc_entry("x_busy", 0, NULL);
	x_proc_busy->read_proc = x_read_busy;
	return 0;
}

static void __exit my_exit(void)
{
	if (x_proc_busy)
		remove_proc_entry("x_busy", NULL);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL v2");
