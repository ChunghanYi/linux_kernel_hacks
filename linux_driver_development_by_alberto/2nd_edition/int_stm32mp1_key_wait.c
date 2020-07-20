/* ************ LDD4EP(2): int_stm32mp1_key_wait.c ************ */
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
#include <linux/wait.h> /* include wait queue */

#define MAX_KEY_STATES 256

static char *HELLO_KEYS_NAME = "PB_USER";
static char hello_keys_buf[MAX_KEY_STATES];
static int buf_rd, buf_wr;

struct key_priv {
	struct device *dev;
	struct gpio_desc *gpio;
	struct miscdevice int_miscdevice;
	wait_queue_head_t wq_data_available;
	int irq;
};

static irqreturn_t hello_keys_isr(int irq, void *data)
{
	int val;
	struct key_priv *priv = data;
	dev_info(priv->dev, "interrupt received. key: %s\n", HELLO_KEYS_NAME);

	val = gpiod_get_value(priv->gpio);
	dev_info(priv->dev, "Button state: 0x%08X\n", val);

	if (val == 1)
		hello_keys_buf[buf_wr++] = 'P'; 
	else
		hello_keys_buf[buf_wr++] = 'R';

	if (buf_wr >= MAX_KEY_STATES)
		buf_wr = 0;

	/* Wake up the process */
	wake_up_interruptible(&priv->wq_data_available);

	return IRQ_HANDLED;
}

static int my_dev_read(struct file *file, char __user *buff,
		size_t count, loff_t *off)
{
	int ret_val;
	char ch[2];
	struct key_priv *priv;

	priv = container_of(file->private_data, struct key_priv, int_miscdevice);

	dev_info(priv->dev, "mydev_read_file entered\n");

	/* 
	 * Sleep the process 
	 * The condition is checked each time the waitqueue is woken up
	 */
	ret_val = wait_event_interruptible(priv->wq_data_available, buf_wr != buf_rd);
	if (ret_val)	
		return ret_val;

	/* Send values to user application*/
	ch[0] = hello_keys_buf[buf_rd];
	ch[1] = '\n';
	if (copy_to_user(buff, &ch, 2))
		return -EFAULT;

	buf_rd++;
	if (buf_rd >= MAX_KEY_STATES)
		buf_rd = 0;
	*off+=1;
	return 2;
}

static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.read = my_dev_read,
};

static int my_probe(struct platform_device *pdev)
{	
	int ret_val;
	struct key_priv *priv;
	struct device *dev = &pdev->dev;

	dev_info(dev, "my_probe() function is called.\n");

	/* Allocate new structure representing device */
	priv = devm_kzalloc(dev, sizeof(struct key_priv), GFP_KERNEL);
	priv->dev = dev;

	platform_set_drvdata(pdev, priv);

	/* Init the wait queue head */
	init_waitqueue_head(&priv->wq_data_available);

	/* Get Linux IRQ number from device tree using 2 methods */
	priv->gpio = devm_gpiod_get(dev, NULL, GPIOD_IN);
	if (IS_ERR(priv->gpio)) {
		dev_err(dev, "gpio get failed\n");
		return PTR_ERR(priv->gpio);
	}
	priv->irq = gpiod_to_irq(priv->gpio);
	if (priv->irq < 0)
		return priv->irq;
	dev_info(dev, "The IRQ number is: %d\n", priv->irq);

	priv->irq = platform_get_irq(pdev, 0);
	if (priv->irq < 0) {
		dev_err(dev, "irq is not available\n");
		return priv->irq;
	}
	dev_info(dev, "IRQ_using_platform_get_irq: %d\n", priv->irq);

	ret_val = devm_request_irq(dev, priv->irq, hello_keys_isr,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			HELLO_KEYS_NAME, priv);
	if (ret_val) {
		dev_err(dev, "Failed to request interrupt %d, error %d\n", priv->irq, ret_val);
		return ret_val;
	}

	priv->int_miscdevice.name = "mydev";
	priv->int_miscdevice.minor = MISC_DYNAMIC_MINOR;
	priv->int_miscdevice.fops = &my_dev_fops;

	ret_val = misc_register(&priv->int_miscdevice);
	if (ret_val != 0) {
		dev_err(dev, "could not register the misc device mydev\n");
		return ret_val;
	}

	dev_info(dev, "my_probe() function is exited.\n");

	return 0;
}

static int my_remove(struct platform_device *pdev)
{	
	struct key_priv *priv = platform_get_drvdata(pdev);
	dev_info(&pdev->dev, "my_remove() function is called.\n");
	misc_deregister(&priv->int_miscdevice);
	dev_info(&pdev->dev, "my_remove() function is exited.\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,intkeywait"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "intkeywait",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a platform driver that sends to user space \
		the number of times we press the switch using INTs");
