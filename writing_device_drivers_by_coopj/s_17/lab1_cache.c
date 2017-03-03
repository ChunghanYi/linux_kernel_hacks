/* **************** LDD:2.0 s_17/lab1_cache.c **************** */
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
 * Memory caches
 *
 * Extend your chararcter driver to allocate the driver's
 * internal buffer by using your own memory cache.
 * Make sure you free any slabs you create.
 *
 * For extra credit create more than one object (perhaps every time
 * you do a read or write) and make sure you release them all before
 * destroying the cache.
 @*/

#include <linux/module.h>
#include "lab_char.h"

static int size = PAGE_SIZE;
static struct kmem_cache *my_cache;
module_param(size, int, S_IRUGO);

static int mycdrv_open(struct inode *inode, struct file *file)
{
	/* allocate a memory cache object */

	if (!(ramdisk = kmem_cache_alloc(my_cache, GFP_ATOMIC))) {
		pr_err(" failed to create a cache object\n");
		return -ENOMEM;
	}
	pr_info(" successfully created a cache object\n");
	return mycdrv_generic_open(inode, file);
}

static int mycdrv_release(struct inode *inode, struct file *file)
{
	/* destroy a memory cache object */
	kmem_cache_free(my_cache, ramdisk);
	pr_info("destroyed a memory cache object\n");
	pr_info(" closing character device: %s:\n\n", MYDEV_NAME);
	return 0;
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_generic_read,
	.write = mycdrv_generic_write,
	.open = mycdrv_open,
	.release = mycdrv_release
};

static int __init my_init(void)
{
	/* create a memory cache */

	if (size > (1024 * PAGE_SIZE)) {
		pr_info
		    (" size=%d is too large; you can't have more than 1024 pages!\n",
		     size);
		return -1;
	}

	if (!(my_cache = kmem_cache_create("mycache", size, 0,
					   SLAB_HWCACHE_ALIGN, NULL))) {
		pr_err("kmem_cache_create failed\n");
		return -ENOMEM;
	}
	pr_info("allocated memory cache correctly\n");
	ramdisk_size = size;

	if (alloc_chrdev_region(&first, 0, count, MYDEV_NAME) < 0) {
		pr_err("failed to allocate character device region\n");
		(void)kmem_cache_destroy(my_cache);
		return -1;
	}
	if (!(my_cdev = cdev_alloc())) {
		pr_err("cdev_alloc() failed\n");
		unregister_chrdev_region(first, count);
		(void)kmem_cache_destroy(my_cache);
		return -1;
	}
	cdev_init(my_cdev, &mycdrv_fops);
	if (cdev_add(my_cdev, first, count) < 0) {
		pr_err("cdev_add() failed\n");
		cdev_del(my_cdev);
		unregister_chrdev_region(first, count);
		(void)kmem_cache_destroy(my_cache);
		return -1;
	}
	foo_class = class_create(THIS_MODULE, "my_class");
	device_create(foo_class, NULL, first, NULL, "%s", "mycdrv");
	pr_info("\nSucceeded in registering character device %s\n", MYDEV_NAME);
	pr_info("Major number = %d, Minor number = %d\n", MAJOR(first),
		MINOR(first));

	return 0;
}

static void __exit my_exit(void)
{
	device_destroy(foo_class, first);
	class_destroy(foo_class);

	if (my_cdev)
		cdev_del(my_cdev);
	unregister_chrdev_region(first, count);
	pr_info("\ndevice unregistered\n");

	(void)kmem_cache_destroy(my_cache);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_17/lab1_cache.c");
MODULE_LICENSE("GPL v2");
