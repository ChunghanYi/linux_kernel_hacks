/* **************** LDD:2.0 s_14/lab2_proc.c **************** */
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

* Using the /proc filesystem. (/proc/driver solution)
*
* Write a module that creates a /proc filesystem entry and can read
* and write to it.
*
* When you read from the entry, you should obtain the value of some
* parameter set in your module.
*
* When you write to the entry, you should modify that value, which
* should then be reflected in a subsequent read.
*
* Make sure you remove the entry when you unload your module.  What
* happens if you don't and you try to access the entry after the
* module has been removed?
*
* There are two different solutions given, one which creates the entry
* in the /proc directory, the other in /proc/driver.
@*/

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/seq_file.h>

#if 0
#define NODE "my_proc"
#else
#define NODE "driver/my_proc"
#endif

static int param = 100;
static struct proc_dir_entry *my_proc;
static struct file_operations my_file_ops = {
	.owner   = THIS_MODULE,
};

ssize_t
my_proc_read(struct file *file, char __user *buffer,
		size_t count, loff_t *ppos)
{
	int nbytes;

	if (count < 4096)
		return 0;
	nbytes = sprintf(buffer, "%d\n", param);
	return nbytes;
}

ssize_t
my_proc_write(struct file *file, const char __user * buffer,
	      size_t count, loff_t *ppos)
{
	char *str;
	str = kmalloc((size_t) count, GFP_KERNEL);
	if (copy_from_user(str, buffer, count)) {
		kfree(str);
		return -EFAULT;
	}
	sscanf(str, "%d", &param);
	pr_info("param has been set to %d\n", param);
	kfree(str);
	return count;
}

static int __init my_init(void)
{
	my_file_ops.read = my_proc_read;
	my_file_ops.write = my_proc_write;
	my_proc = proc_create(NODE, S_IRUGO | S_IWUSR, NULL, &my_file_ops);
	if (!my_proc) {
		pr_err("I failed to make %s\n", NODE);
		return -1;
	}
	pr_info("I created %s\n", NODE);
	return 0;
}

static void __exit my_exit(void)
{
	if (my_proc) {
		remove_proc_entry(NODE, NULL);
		pr_info("Removed %s\n", NODE);
	}
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_AUTHOR("Chunghan Yi");
MODULE_DESCRIPTION("LDD:2.0 s_14/lab2_proc.c");
MODULE_LICENSE("GPL v2");
