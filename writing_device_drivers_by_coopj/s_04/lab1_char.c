/* **************** LDD:2.0 s_04/lab1_char.c **************** */
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
 * Improving the Basic Character Driver
 *
 * Starting from sample_driver.c, extend it to:
 *
 * Keep track of the number of times it has been opened since loading,
 * and print out the counter every time the device is opened.
 *
 * Print the major and minor numbers when the device is opened.
 *
 * To exercise your driver, write a program to read (and/or write)
 * from the node, using the standard Unix I/O functions (open(),
 * read(), write(), close()).
 *
 * After loading the module with insmod use this program to access the
 * node.
 *
 * Track usage of the module by using lsmod (which is equivalent to
 * typing cat /proc/modules.)
 @*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>

#define MYDEV_NAME "mycdrv"

static char *ramdisk;
static size_t ramdisk_size = (16 * PAGE_SIZE);
static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
static struct cdev *my_cdev;

static int mycdrv_open(struct inode *inode, struct file *file)
{
	static int counter = 0;
	pr_info(" attempting to open device: %s:\n", MYDEV_NAME);
	pr_info(" MAJOR number = %d, MINOR number = %d\n",
		imajor(inode), iminor(inode));
	counter++;

	pr_info(" successfully open  device: %s:\n\n", MYDEV_NAME);
	pr_info("I have been opened  %d times since being loaded\n", counter);
	pr_info("ref=%d\n", (int)module_refcount(THIS_MODULE));

	return 0;
}

static int mycdrv_release(struct inode *inode, struct file *file)
{
	pr_info(" CLOSING device: %s:\n\n", MYDEV_NAME);
	return 0;
}

static ssize_t
mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	int nbytes = lbuf - copy_to_user(buf, ramdisk + *ppos, lbuf);
	*ppos += nbytes;
	pr_info("\n READING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	int nbytes = lbuf - copy_from_user(ramdisk + *ppos, buf, lbuf);
	*ppos += nbytes;
	pr_info("\n WRITING function, nbytes=%d, pos=%d\n", nbytes, (int)*ppos);
	return nbytes;
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.open = mycdrv_open,
	.release = mycdrv_release,
};

static int __init my_init(void)
{
	first = MKDEV(my_major, my_minor);
	if (register_chrdev_region(first, count, MYDEV_NAME) < 0) {
		pr_err("failed to register character device region\n");
		return -1;
	}

	if (!(my_cdev = cdev_alloc())) {
		pr_err("cdev_alloc() failed\n");
		unregister_chrdev_region(first, count);
		return -1;
	}

	cdev_init(my_cdev, &mycdrv_fops);
	ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);

	if (cdev_add(my_cdev, first, count) < 0) {
		pr_err("cdev_add() failed\n");
		cdev_del(my_cdev);
		unregister_chrdev_region(first, count);
		kfree(ramdisk);
		return -1;
	}

	pr_info("\nSucceeded in registering character device %s\n", MYDEV_NAME);
	return 0;
}

static void __exit my_exit(void)
{
	if (my_cdev)
		cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	kfree(ramdisk);
	pr_info("\ndevice unregistered\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_04/lab1_char.c");
MODULE_LICENSE("GPL v2");
