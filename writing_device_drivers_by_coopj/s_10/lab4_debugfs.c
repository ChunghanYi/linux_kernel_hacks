/* **************** LDD:2.0 s_10/lab4_debugfs.c **************** */
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
 * Using debugfs.
 *
 * Write a module that creates entries in debugfs.
 *
 * Make sure the debugfs filesystem is mounted and you have support
 * for it in your kernel.
 *
 * First use one of the convenience functions to make just a simple
 * one variable entry under the root debugfs filesystem, of whatever
 * length you desire.
 *
 * Next create your own directory and put one or more entries in it.
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

static struct dentry *var32, *parent, *filen;
static u32 val = (u32) 888;
static u32 val32 = (u32) 777;
#define KS 32
static char kstring[KS];	/* should be less sloppy about overflows :) */

static ssize_t
my_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes;
	nbytes = sprintf(kstring, "%d\n", val);
	pr_info("d_inode = %p\n", parent->d_inode);
	return simple_read_from_buffer(buf, lbuf, ppos, kstring, nbytes);
}

static ssize_t
my_write(struct file *file, const char __user * buf, size_t lbuf, loff_t * ppos)
{
	int rc;
	int nbytes = lbuf;
	rc = copy_from_user(&kstring, buf, lbuf);
	sscanf(kstring, "%d", &val);
	pr_info("\n WRITING function, nbytes=%d, val=%d\n", nbytes, val);
	return nbytes;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = my_read,
	.write = my_write,
};

static int __init my_init(void)
{
	var32 = debugfs_create_u32("myname", S_IRUGO | S_IWUSR, NULL, &val32);
	parent = debugfs_create_dir("mydir", NULL);
	filen =
	    debugfs_create_file("filen", S_IRUGO | S_IWUSR, parent, NULL,
				&fops);
	pr_info("Hello: module loaded at 0x%p\n", my_init);
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("Bye: module unloaded from 0x%p\n", my_exit);
	if (filen)
		debugfs_remove(filen);
	if (parent)
		debugfs_remove(parent);
	if (var32)
		debugfs_remove(var32);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_10/lab4_debugfs.c");
MODULE_LICENSE("GPL v2");
