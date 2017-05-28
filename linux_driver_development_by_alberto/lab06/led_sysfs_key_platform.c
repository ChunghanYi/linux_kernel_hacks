/* ************ LDD4EP: listing6-3: led_sysfs_key_platform.c ************ */
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
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>	//required for the GPIO functions
#include <linux/interrupt.h>	// devm_request_irq

static char *HELLO_KEYS_NAME = "SW5";
static int irq;
static struct platform_driver my_platform_driver;
static unsigned int gpioLED = 2;		// (gpio_bank - 1)*32 + gpio_bit = (1-1)*32 + 2
static unsigned int gpioButton = 5;		// (gpio_bank - 1)*32 + gpio_bit = (1-1)*32 + 5
static bool ledOn;


static irqreturn_t hello_keys_isr(int irq, void *dev_id)
{
	int val;

	pr_info("interrupt received. key: %s\n", HELLO_KEYS_NAME);
	val = gpio_get_value(gpioButton);
	pr_info("Button state: 0x%08X\n", val);
	ledOn = !ledOn;
	gpio_set_value(gpioLED, ledOn);

	return IRQ_HANDLED;
}

/* create the led_store function */
static ssize_t led_store(struct device_driver *driver, const char *buf, size_t count)
{
	int result;
	unsigned long value;
	char *led_on = "on";
	char *led_off = "off";

	result = kstrtoul(buf, 0, &value);

	/* we use count-1 as echo add \n to the terminal string */
	if (!strncmp(buf, led_on, count-1) | (value == 0x01)) {
		ledOn = 1;
		gpio_set_value(gpioLED, ledOn);
	} else if (!strncmp(buf, led_off, count-1) | (value == 0x00)) {
		ledOn = 0;
		gpio_set_value(gpioLED, ledOn);
	} else {
		pr_info("Bad led value.\n");
		return -EINVAL;
	}

	return count;
}

/* Include the DRIVER_ATTR macro registering a store function */
#if 0
DRIVER_ATTR(led, (S_IWUGO), NULL, led_store);
#else /* fixed by chunghan.yi@gmail.com, 05/19/2017 */
DRIVER_ATTR(led, 0600, NULL, led_store);
#endif

/* Turn on/off the led with led_app, use copy_from_user() */
static ssize_t my_dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char *led_on = "on"; 
	char *led_off = "off"; 
	unsigned char myled_value[10]; 

	pr_info("my_dev_write() is called.\n");

	if (copy_from_user(myled_value, buf, count))
		return -EFAULT;

	if (strncmp(led_on, myled_value, count) == 0) {
		ledOn = 1;
		gpio_set_value(gpioLED, ledOn);
	} else if (strncmp(led_off, myled_value, count) == 0) {
		ledOn = 0;
		gpio_set_value(gpioLED, ledOn);
	} else if(strncmp("exit", myled_value, count) == 0) {
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
	struct device *dev = &pdev->dev;

	pr_info("platform_probe enter\n");

	if (!gpio_is_valid(gpioLED)) {
		pr_info("Invalid led GPIO");
		return -ENODEV;
	}

	if (!gpio_is_valid(gpioButton)) {
		pr_info("Invalid button GPIO");
		return -ENODEV;
	}

	gpio_request(gpioLED, "sysfs");
	gpio_direction_output(gpioLED, 1);
	gpio_export(gpioLED, 0);

	gpio_request(gpioButton, "sysfs");
	gpio_direction_input(gpioButton);
	gpio_export(gpioButton, 0);

	/* create the sysfs entries */
	ret_val = driver_create_file(&my_platform_driver.driver, &driver_attr_led);
	if (ret_val != 0) {
		pr_err("failed to create sysfs entry");
		return ret_val;
	}

	irq = gpio_to_irq(gpioButton);
	if (irq < 0) {
		pr_err("irq is not available\n");
		return -EINVAL;
	}
	pr_info("IRQ_using_gpio_to_irq: %d\n", irq);

	ret_val = devm_request_irq(dev, irq, hello_keys_isr,
					IRQF_TRIGGER_FALLING, HELLO_KEYS_NAME, pdev->dev.of_node);

	if (ret_val) {
		pr_err("Failed to request interrupt %d, error %d\n", irq, ret_val);
		return ret_val;
	}

	ret_val = misc_register(&helloworld_miscdevice);
	if (ret_val != 0) {
		pr_err("could not register the misc device mydev");
		return ret_val;
	}
	pr_info("mydev: got minor %i\n", helloworld_miscdevice.minor);
	return 0;
}

static int __exit my_remove(struct platform_device *pdev)
{
	gpio_unexport(gpioLED);
	gpio_unexport(gpioButton);
	misc_deregister(&helloworld_miscdevice);
	driver_remove_file(&my_platform_driver.driver,
			&driver_attr_led);
	pr_info("platform_remove exit\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,intkey"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "intkey",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a platform driver that controls the LED \
	using a sysfs and an user application. It also manages GPIO interrupts");
