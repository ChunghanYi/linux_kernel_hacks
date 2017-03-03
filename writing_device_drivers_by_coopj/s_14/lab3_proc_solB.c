/* **************** LDD:2.0 s_14/lab3_proc_solB.c **************** */
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
 * Making your own subdirectory in proc.
 *
 * Write a module that creates your own proc filesystem subdirectory
 * and creates at least two entries under it.

 * As in the first exercise, reading an entry should obtain a
 * parameter value, and writing it should reset it.

 * You may use the data element in the proc_dir_entry structure to use
 * the same callback functions for multiple entries.
 @*/

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>

#define NODE_DIR "my_proc_dir"
#define NODE_1 "param_1"
#define NODE_2 "param_2"

static int param_1 = 100, param_2 = 200;
static struct proc_dir_entry *my_proc_dir, *my_proc_1, *my_proc_2;
static struct file_operations my_file_ops1 = {
	.owner   = THIS_MODULE,
};
static struct file_operations my_file_ops2 = {
	.owner   = THIS_MODULE,
};

ssize_t
my_proc_read1(struct file *file, char __user *buffer,
		size_t count, loff_t *ppos)
{
	int nbytes;

    if (count < 4096)
        return 0;
    nbytes = sprintf(buffer, "%d\n", param_1);
    return nbytes;
}

ssize_t
my_proc_read2(struct file *file, char __user *buffer,
		size_t count, loff_t *ppos)
{
	int nbytes;

	if (count < 4096)
		return 0;
	nbytes = sprintf(buffer, "%d\n", param_2);
	return nbytes;
}

ssize_t
my_proc_write1(struct file *file, const char __user * buffer,
		size_t count, loff_t *data)
{
	char *str = kmalloc((size_t) count, GFP_KERNEL);

	if (copy_from_user(str, buffer, count)) {
		kfree(str);
		return -EFAULT;
	}

	sscanf(str, "%d", &param_1);
	pr_info("param_1 has been set to %d\n", param_1);
	kfree(str);
	return count;
}

ssize_t
my_proc_write2(struct file *file, const char __user * buffer,
		size_t count, loff_t *data)
{
	char *str = kmalloc((size_t) count, GFP_KERNEL);
	if (copy_from_user(str, buffer, count))
		return -EFAULT;
	sscanf(str, "%d", &param_2);
	pr_info("param_2 has been set to %d\n", param_2);
	kfree(str);
	return count;
}

static int __init my_init(void)
{
	my_proc_dir = proc_mkdir(NODE_DIR, NULL);
	if (!my_proc_dir) {
		pr_err("I failed to make %s\n", NODE_DIR);
		return -1;
	}
	pr_info("I created %s\n", NODE_DIR);

	my_file_ops1.read = my_proc_read1;
	my_file_ops1.write = my_proc_write1;
	my_proc_1 = proc_create(NODE_1, S_IRUGO | S_IWUSR, my_proc_dir, &my_file_ops1);
	if (!my_proc_1) {
		pr_err("I failed to make %s\n", NODE_1);
		remove_proc_entry(NODE_DIR, NULL);
		return -1;
	}
	pr_info("I created %s\n", NODE_1);

	my_file_ops2.read = my_proc_read2;
	my_file_ops2.write = my_proc_write2;
	my_proc_2 = proc_create(NODE_2, S_IRUGO | S_IWUSR, my_proc_dir, &my_file_ops2);
	if (!my_proc_2) {
		pr_err("I failed to make %s\n", NODE_2);
		remove_proc_entry(NODE_DIR, NULL);
		return -1;
	}
	pr_info("I created %s\n", NODE_2);
	return 0;
}

static void __exit my_exit(void)
{
	if (my_proc_1) {
		remove_proc_entry(NODE_1, my_proc_dir);
		pr_info("Removed %s\n", NODE_1);
	}
	if (my_proc_2) {
		remove_proc_entry(NODE_2, my_proc_dir);
		pr_info("Removed %s\n", NODE_2);
	}
	if (my_proc_dir) {
		remove_proc_entry(NODE_DIR, NULL);
		pr_info("Removed %s\n", NODE_DIR);
	}
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_AUTHOR("Chunghan Yi");
MODULE_DESCRIPTION("LDD:2.0 s_14/lab3_proc_solB.c");
MODULE_LICENSE("GPL v2");
