/* ************ LDD4EP: listing6-1: int_key.c ************ */
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
	intkey {
		compatible = "arrow,intkey";
		pinctrl-names = "default";
		pintctrl-0 = <&pinctrl_key>;
		gpios = <&gpio4 16 0>;
		interrupts = <16 0>;
		interrupt-parent = <&gpio4>;
	};

	pinctrl_key: key {
		fsl,pins = <
			MX6QDL_PAD_DI0_DISP_CLK__GPIO4_IO16     0x1b0b1
		>;
	};
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>	// platform_get_irq
#include <linux/uaccess.h>
#include <linux/of_gpio.h>	// of_get_gpio
#include <linux/interrupt.h>	// devm_request_irq
#include <linux/miscdevice.h>

static char *HELLO_KEYS_NAME = "SW5";
static int gpio_nb;
static int irq;

/* interrupt handler */
static irqreturn_t hello_keys_isr(int irq, void *dev_id)
{
	int val;

	pr_info("interrupt received. key: %s\n", HELLO_KEYS_NAME);
	val = gpio_get_value(gpio_nb);
	pr_info("Button state: 0x%08X\n", val);
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

static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.release = my_dev_close,
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

	pr_info("mydev: got minor %i\n", helloworld_miscdevice.minor);
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
MODULE_DESCRIPTION("This is a SW5 INT platform driver");
