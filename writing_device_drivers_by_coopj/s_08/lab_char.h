/* **************** LDD:2.0 s_08/lab_char.h **************** */
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
 @*/
#ifndef _LAB_CHAR_H
#define _LAB_CHAR_H

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define MYDEV_NAME "mycdrv"

static char *ramdisk;
static size_t ramdisk_size = (16 * PAGE_SIZE);
static dev_t first;
static unsigned int count = 1;	/* number of dev_t needed */
static struct cdev *my_cdev;
static struct class *foo_class;

static const struct file_operations mycdrv_fops;

/* generic entry points */

static inline int mycdrv_generic_open(struct inode *inode, struct file *file)
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

static inline int mycdrv_generic_release(struct inode *inode, struct file *file)
{
	pr_info(" closing character device: %s:\n\n", MYDEV_NAME);
	return 0;
}

static inline ssize_t
mycdrv_generic_read(struct file *file, char __user * buf, size_t lbuf,
		    loff_t * ppos)
{
	int nbytes, maxbytes, bytes_to_do;
	maxbytes = ramdisk_size - *ppos;
	bytes_to_do = maxbytes > lbuf ? lbuf : maxbytes;
	if (bytes_to_do == 0)
		pr_warning("Reached end of the device on a read");

	nbytes = bytes_to_do - copy_to_user(buf, ramdisk + *ppos, bytes_to_do);
	*ppos += nbytes;
	pr_info("\n Leaving the   READ function, nbytes=%d, pos=%d\n",
		nbytes, (int)*ppos);
	return nbytes;
}

static inline ssize_t
mycdrv_generic_write(struct file *file, const char __user * buf, size_t lbuf,
		     loff_t * ppos)
{
	int nbytes, maxbytes, bytes_to_do;
	maxbytes = ramdisk_size - *ppos;
	bytes_to_do = maxbytes > lbuf ? lbuf : maxbytes;
	if (bytes_to_do == 0)
		pr_warning("Reached end of the device on a write");
	nbytes =
	    bytes_to_do - copy_from_user(ramdisk + *ppos, buf, bytes_to_do);
	*ppos += nbytes;
	pr_info("\n Leaving the   WRITE function, nbytes=%d, pos=%d\n",
		nbytes, (int)*ppos);
	return nbytes;
}

static inline loff_t mycdrv_generic_lseek(struct file *file, loff_t offset,
					  int orig)
{
	loff_t testpos;
	switch (orig) {
	case SEEK_SET:
		testpos = offset;
		break;
	case SEEK_CUR:
		testpos = file->f_pos + offset;
		break;
	case SEEK_END:
		testpos = ramdisk_size + offset;
		break;
	default:
		return -EINVAL;
	}
	testpos = testpos < ramdisk_size ? testpos : ramdisk_size;
	testpos = testpos >= 0 ? testpos : 0;
	file->f_pos = testpos;
	pr_info("Seeking to pos=%ld\n", (long)testpos);
	return testpos;
}

static inline int __init my_generic_init(void)
{
	if (alloc_chrdev_region(&first, 0, count, MYDEV_NAME) < 0) {
		pr_err("failed to allocate character device region\n");
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

	foo_class = class_create(THIS_MODULE, "my_class");
	device_create(foo_class, NULL, first, NULL, "%s", "mycdrv");
	pr_info("\nSucceeded in registering character device %s\n", MYDEV_NAME);
	pr_info("Major number = %d, Minor number = %d\n", MAJOR(first),
		MINOR(first));

	return 0;
}

static inline void __exit my_generic_exit(void)
{
	device_destroy(foo_class, first);
	class_destroy(foo_class);

	if (my_cdev)
		cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	kfree(ramdisk);
	pr_info("\ndevice unregistered\n");
}

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_08/lab_char.h");
MODULE_LICENSE("GPL v2");
#endif
