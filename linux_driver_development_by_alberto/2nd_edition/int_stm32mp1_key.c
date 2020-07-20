/* ************ LDD4EP(2): int_stm32mp1_key.c ************ */
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
#include <linux/platform_device.h> 	
#include <linux/interrupt.h> 		
#include <linux/gpio/consumer.h>
#include <linux/miscdevice.h>
#include <linux/of_device.h>

static char *HELLO_KEYS_NAME = "PB_KEY";

/* interrupt handler */
static irqreturn_t hello_keys_isr(int irq, void *data)
{
	struct device *dev = data;
	dev_info(dev, "interrupt received. key: %s\n", HELLO_KEYS_NAME);
	return IRQ_HANDLED;
}

static struct miscdevice helloworld_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mydev",
};

static int my_probe(struct platform_device *pdev)
{	
	int ret_val, irq;
	struct gpio_desc *gpio;
	struct device *dev = &pdev->dev;

	dev_info(dev, "my_probe() function is called.\n");

	/* First method to get the Linux IRQ number */
	gpio = devm_gpiod_get(dev, NULL, GPIOD_IN);
	if (IS_ERR(gpio)) {
		dev_err(dev, "gpio get failed\n");
		return PTR_ERR(gpio);
	}
	irq = gpiod_to_irq(gpio);
	if (irq < 0)
		return irq;
	dev_info(dev, "The IRQ number is: %d\n", irq);

	/* Second method to get the Linux IRQ number */
	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(dev, "irq is not available\n");
		return -EINVAL;
	}
	dev_info(dev, "IRQ_using_platform_get_irq: %d\n", irq);

	/* Allocate the interrupt line */
	ret_val = devm_request_irq(dev, irq, hello_keys_isr, 
					IRQF_TRIGGER_FALLING,
					HELLO_KEYS_NAME, dev);
	if (ret_val) {
		dev_err(dev, "Failed to request interrupt %d, error %d\n", irq, ret_val);
		return ret_val;
	}

	ret_val = misc_register(&helloworld_miscdevice);
	if (ret_val != 0) {
		dev_err(dev, "could not register the misc device mydev\n");
		return ret_val;
	}

	dev_info(dev, "mydev: got minor %i\n",helloworld_miscdevice.minor);
	dev_info(dev, "my_probe() function is exited.\n");

	return 0;
}

static int my_remove(struct platform_device *pdev)
{	
	dev_info(&pdev->dev, "my_remove() function is called.\n");
	misc_deregister(&helloworld_miscdevice);
	dev_info(&pdev->dev, "my_remove() function is exited.\n");
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
MODULE_DESCRIPTION("This is a button INT platform driver");
