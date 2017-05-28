/* ************ LDD4EP: listing4-4: helloworld_class_driver.c ************ */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define  DEVICE_NAME "mydev"
#define  CLASS_NAME  "hello_class"

dev_t dev_no, dev;
static int Major;
static struct cdev my_dev;
static struct class *helloClass = NULL; // class struct pointer
static struct device *helloDevice = NULL; // device struct pointer

static int my_dev_open(struct inode *inode, struct file *file)
{
	pr_info("my_dev_open() is called.\n");
	return 0;
}

static int my_dev_close(struct inode *inode, struct file *file)
{
	pr_info("my_dev_close() is called.\n");
	return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info("my_dev_ioctl() is called. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

/* declare a file_operations structure */
static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.release = my_dev_close,
	.unlocked_ioctl = my_dev_ioctl,
};

static int __init hello_init(void)
{
	int ret;

	pr_info("Hello world init\n");

	/* Allocate dynamically device numbers */
	ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		pr_info("Unable to allocate Mayor number \n");
		return ret;
	}
	Major = MAJOR(dev_no);
	dev = MKDEV(Major, 0);

	pr_info("Allocated correctly with major number %d\n", Major);

	/* Initialize the cdev structure and add it to the kernel space start */
	cdev_init(&my_dev, &my_dev_fops);
	ret = cdev_add(&my_dev, dev, 1);
	if (ret < 0) {
		unregister_chrdev_region(dev, 1);
		pr_info("Unable to add cdev\n");
		return ret;
	}

	/* Register the device class */
	helloClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(helloClass)) {
		unregister_chrdev_region(dev, 1);
		pr_info("Failed to register device class\n");
		return PTR_ERR(helloClass);
	}
	pr_info("device class registered correctly\n");

	/* Create a device node */
	helloDevice = device_create(helloClass, NULL, dev, NULL, DEVICE_NAME);
	if (IS_ERR(helloDevice)) {
		class_destroy(helloClass);
		unregister_chrdev_region(dev, 1);
		pr_info("Failed to create the device\n");
		return PTR_ERR(helloDevice);
	}
	pr_info("The device is created correctly\n");

	return 0;
}

static void __exit hello_exit(void)
{
	device_destroy(helloClass, dev);     // remove the device
	class_unregister(helloClass);        // unregister the device class
	class_destroy(helloClass);           // remove the device class
	unregister_chrdev_region(dev, 1);    // unregister the device numbers
	pr_info("Hello world with parameter exit\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a module that interacts with the ioctl system call");
