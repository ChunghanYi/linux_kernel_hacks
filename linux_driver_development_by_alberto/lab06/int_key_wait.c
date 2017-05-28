/* ************ LDD4EP: listing6-2: int_key_wait.c ************ */
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
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>

/* include wait queue and sched header */
#include <linux/wait.h>
#include <linux/sched.h>

#define MAX_KEY_STATES 10

static char *HELLO_KEYS_NAME = "SW5";
static char hello_keys_buf[MAX_KEY_STATES];
static int buf_rd, buf_wr;
static int gpio_nb;
static int irq;

/* declare wait queue */
static DECLARE_WAIT_QUEUE_HEAD(hello_keys_wait);

static int prev_value = 0x00000020;
static int next_value;

static irqreturn_t hello_keys_isr(int irq, void *dev_id)
{
	int val;

	pr_info("interrupt received. key: %s\n", HELLO_KEYS_NAME);
	val = gpio_get_value(gpio_nb);
	pr_info("Button state: 0x%08X\n", val);
	next_value = val;
	if (prev_value == next_value)
		return IRQ_HANDLED;

	if (val == 0)
		hello_keys_buf[buf_wr++] = 'P'; //read and increment buf_wr
	if (val == 0x0000020)
		hello_keys_buf[buf_wr++] = 'R';

	if (buf_wr >= MAX_KEY_STATES)
		buf_wr = 0;

	prev_value = next_value;

	/* Wake up the process */
	wake_up(&hello_keys_wait);

	return IRQ_HANDLED;
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

static int my_dev_read(struct file *file, char __user *buff, size_t count,
		loff_t *off)
{
	int ret_val;
	char ch[2];

	pr_info("my_dev_read() is called");
	/* Sleep the process */
	ret_val = wait_event_interruptible(hello_keys_wait, buf_wr != buf_rd);
	if (ret_val) {
		pr_err("Failed to request interrupt %d, error %d\n", irq, ret_val);
		return ret_val;
	}

	ch[0] = hello_keys_buf[buf_rd];
	ch[1] = '\n';

	if (copy_to_user(buff, &ch, 2))
		return -EFAULT;
	buf_rd++;
	if (buf_rd >= MAX_KEY_STATES)
		buf_rd = 0;
	return 2;
}

static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.read = my_dev_read,
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

	pr_info("my_probe() function is called.\n");

	/* Get int number from device tree in 2 ways */
	gpio_nb = of_get_gpio(pdev->dev.of_node, 0);
	if (gpio_nb < 0)
		return gpio_nb;
	pr_info("GPIO: %d\n", gpio_nb);

	irq = gpio_to_irq(gpio_nb);
	if (irq < 0) {
		pr_err("irq is not available\n");
		return -EINVAL;
	}
	pr_info("IRQ_using_gpio_to_irq: %d\n", irq);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("irq is not available\n");
		return -EINVAL;
	}
	pr_info("IRQ_using_platform_get_irq: %d\n", irq);

	/* Register the interrupt handler */
	ret_val = devm_request_irq(dev, irq, hello_keys_isr,
					IRQF_SHARED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
					HELLO_KEYS_NAME, pdev->dev.of_node);
	if (ret_val) {
		pr_err("Failed to request interrupt %d, error %d\n", irq, ret_val);
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
	pr_info("my_remove() function is called.\n");
	misc_deregister(&helloworld_miscdevice);
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
MODULE_DESCRIPTION("This is a platform driver that sends to user space \
	the number of times we press the switch using INT");
