/* ************ LDD4EP: listing5-3: led_sysfs_platform.c ************ */
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

/*
	//<device tree>
	hellokeys {
		compatible = "arrow,hellokeys";
		reg = <0x020A4000 0x4000>;
		pinctrl-0 = <&pinctrl_led>;
	};

	pinctrl_led: ledgrp {
		fsl,pins = <
				MX6QDL_PAD_EIM_A25__GPIO5_IO02      0x1b0b1 // user led0
				MX6QDL_PAD_EIM_D28__GPIO3_IO28      0x1b0b1 // user led1
		>;
	};
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/uaccess.h>


static void __iomem *g_ioremap_addr;
static struct platform_driver my_platform_driver;

#define GPIO2_GDIR_offset	0x04
#if 0 /* GPIO1_02 */
#define GPIO_DIR_MASK		1<<2
#define GPIO_DATA_MASK		1<<2
#else /* GPIO03_28, ACTIVE_LOW */
#define GPIO_DIR_MASK		1<<28
#define GPIO_DATA_MASK		1<<28
#endif

/* create the led_store function */
static ssize_t led_store(struct device_driver *driver, const char *buf, size_t count)
{
	int result;
	unsigned long value;
	unsigned int temp;
	char *led_on = "on";
	char *led_off = "off";

	result = kstrtoul(buf, 0, &value);

	/* we use count-1 as echo add \n to the terminal string */
	if (!strncmp(buf, led_off, count-1) | (value == 0x01)) {
		temp = ioread32(g_ioremap_addr);
		temp = temp | GPIO_DATA_MASK;
		iowrite32(temp, g_ioremap_addr);
	} else if (!strncmp(buf, led_on, count-1) | (value == 0x00)) {
		temp = ioread32(g_ioremap_addr);
		temp = temp & ~(GPIO_DATA_MASK);
		iowrite32(temp, g_ioremap_addr);
	} else {
		pr_info("Bad led value.\n");
		return -EINVAL;
	}
	return count;
}

/* Include the DRIVER_ATTR macro registering a store function */
#if 0
DRIVER_ATTR(led, (S_IWUGO), NULL, led_store);
#else
DRIVER_ATTR(led, 0600, NULL, led_store);
#endif

/* Turn on/off the led with led_app, use copy_from_user() */
static ssize_t my_dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char *led_on = "on"; 
	char *led_off = "off"; 
	unsigned char myled_value[10]; 
	unsigned int temp;

	pr_info("my_dev_write() is called.\n");

	if (copy_from_user(myled_value, buf, count))
		return -EFAULT;

	if (strncmp(led_off, myled_value, count) == 0) {
		temp = ioread32(g_ioremap_addr);
		temp = temp | GPIO_DATA_MASK;
		iowrite32(temp, g_ioremap_addr);
	} else if (strncmp(led_on, myled_value, count) == 0) {
		temp = ioread32(g_ioremap_addr);
		temp = temp & ~(GPIO_DATA_MASK);
		iowrite32(temp, g_ioremap_addr);
	} else if (strncmp("exit", myled_value, count) == 0) {
		return 0;
	} else {
		pr_info("Bad value\n");
		return -EINVAL;
	}

	pr_info("my_dev_write() is exit.\n");
	return 0;
}

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

static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.write = my_dev_write,
	.release = my_dev_close,
	.unlocked_ioctl = my_dev_ioctl,
};

static struct miscdevice helloworld_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mydev",
	.fops = &my_dev_fops,
};

static int my_probe(struct platform_device *pdev)
{
	int ret_val;
	struct resource *r;
	struct device *dev = &pdev->dev;
	int i;
	unsigned int temp;

	pr_info("platform_probe enter\n");

	/* get our first memory resource from device tree */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r) {
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}
	pr_info("r->start = 0x%08lx\n", (unsigned long)r->start);
	pr_info("r->end = 0x%08lx\n", (unsigned long)r->end);

	/* ioremap our memory region */
	g_ioremap_addr = devm_ioremap(dev, r->start, resource_size(r));
	if (!g_ioremap_addr) {
		pr_err("ioremap failed \n");
		return -ENOMEM;
	}

	/* Set GPIO output direction */
	temp = ioread32(g_ioremap_addr + GPIO2_GDIR_offset);
	temp = temp | GPIO_DIR_MASK;
	iowrite32(temp, g_ioremap_addr + GPIO2_GDIR_offset);

	/* LED toggle test */
	for (i=0; i<4; i++) {
		temp = ioread32(g_ioremap_addr);
		temp = temp & ~(GPIO_DATA_MASK);
		iowrite32(temp, g_ioremap_addr);
		msleep(250);

		temp = temp | GPIO_DATA_MASK;
		iowrite32(temp, g_ioremap_addr);
		msleep(250);
	}

	/* create the sysfs entries */
	ret_val = driver_create_file(&my_platform_driver.driver, &driver_attr_led);
	if (ret_val != 0) {
		pr_err("failed to create sysfs entry");
		return ret_val;
	}

	ret_val = misc_register(&helloworld_miscdevice);
	if (ret_val != 0) {
		pr_err("could not register the misc device mydev");
		return ret_val;
	}
	pr_info("mydev: got minor %i\n",helloworld_miscdevice.minor);
	return 0;
}

static int __exit my_remove(struct platform_device *pdev)
{
	misc_deregister(&helloworld_miscdevice);
	driver_remove_file(&my_platform_driver.driver, &driver_attr_led);
	pr_info("platform_remove exit\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,hellokeys"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "hellokeys",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a platform driver that turns on/off \
	the LED using sysfs and an user application");
