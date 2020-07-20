/* ************ LDD4EP(2): leds-stm32mp1.c ************ */
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

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>

struct lantern_platform_device {
	struct gpio_desc *red, *orange, *blue, *key;
	int irq;
	volatile int color;
	volatile int state;
};

static struct lantern_platform_device lpd;

/* a sole function that switches the leds on and off */
static void set_leds(void)
{
	int color;

	/* First lpd.state = 1, so color = lpd.color = 0x0111 */
	color = (lpd.state)? lpd.color: 0; 

	if (lpd.state == 1)
		gpiod_set_value(lpd.red, 0);
	else
		gpiod_set_value(lpd.red, 1);

	gpiod_set_value(lpd.orange, color & 2);
	gpiod_set_value(lpd.blue, color & 1);
}

/* 
 * interrupt service routine - it debounces a key, which has to be down
 * for a 1/10  of a second to be recognized
 */
static irqreturn_t lantern_isr(int irq, void *arg)
{
	static unsigned long last;
	unsigned long now;
	struct lantern_platform_device *lpd = arg;

	now = jiffies;
	if ((now - last) < (HZ/10))
		return IRQ_HANDLED;

	last = now;

	/* toggle, this is a condition, first time is true, so lpd->state = 1 */
	lpd->state = (lpd->state == 0);
	set_leds();

	return IRQ_HANDLED;
}

static int lantern_probe(struct platform_device *pdev)
{
	int rc;

	lpd.state = 0;
	lpd.color = 7; /* 0111 */

	dev_info(&pdev->dev, "led_stm32mp1 probe succeeded\n");

	/* switch off the three leds at the beginning */
	/* USER1: PA14, Red LD6: PA13, Orange LD7: PH7, and Blue LD8: PD11 */
	lpd.red = gpiod_get_index(&pdev->dev, "led", 0, GPIOD_OUT_HIGH); /* The LED is OFF */
	if (IS_ERR(lpd.red))
		goto fail1;
	lpd.orange = gpiod_get_index(&pdev->dev, "led", 1, GPIOD_OUT_LOW); /* The LED is OFF */
	//lpd.orange = gpiod_get_index(&pdev->dev, "led", 1, GPIOD_OUT_HIGH); /* The LED is ON */
	if (IS_ERR(lpd.orange))
		goto fail2;
	lpd.blue = gpiod_get_index(&pdev->dev, "led", 2, GPIOD_OUT_LOW); /* The LED is OFF */
	if (IS_ERR(lpd.blue))
		goto fail3;

	dev_info(&pdev->dev, "init leds\n");

	lpd.key = gpiod_get(&pdev->dev, "key", GPIOD_IN);
	if (IS_ERR(lpd.key))
		goto fail4;

	lpd.irq = gpiod_to_irq(lpd.key);
	if (lpd.irq < 0)
		goto fail5;

	//rc = request_irq(lpd.irq, lantern_isr,
	//IRQF_TRIGGER_RISING, "lantern_isr", &lpd);

	rc = request_irq(lpd.irq, lantern_isr,
			IRQF_TRIGGER_FALLING, "lantern_isr", &lpd);
	if (rc)
		goto fail5;

	return 0;

fail5:
	gpiod_put(lpd.key);
fail4:
	gpiod_put(lpd.blue);
fail3:
	gpiod_put(lpd.orange);
fail2:
	gpiod_put(lpd.red);
fail1:
	dev_err(&pdev->dev, "probe failed\n");
	return -1;
}

static int lantern_remove(struct platform_device *pdev)
{
	dev_notice(&pdev->dev, "remove called\n");
	free_irq(lpd.irq, &lpd);
	gpiod_put(lpd.key);
	gpiod_put(lpd.blue);
	gpiod_put(lpd.orange);
	gpiod_put(lpd.red);

	return 0;
}

static const struct of_device_id lantern_ids[] = {
	{ .compatible = "key-leds", },
	{ /* table end */ }
};

static struct platform_driver lantern_driver = {
	.probe		= lantern_probe,
	.remove		= lantern_remove,
	.suspend	= NULL,
	.resume		= NULL,
	.driver	= {
		.name	= "key-leds",
		.owner = THIS_MODULE,
		.of_match_table = lantern_ids,
	},
};

/* functions executed when a module is inserted and removed */
static int __init lantern_init(void)
{
	pr_notice("led_stm32mp1_init\n");
	return platform_driver_register(&lantern_driver);
}
module_init(lantern_init);

static void __exit lantern_exit(void)
{
	platform_driver_unregister(&lantern_driver);
	pr_notice("led_stm32mp1_exit\n");
}
module_exit(lantern_exit);

/* Information about this module */
MODULE_DESCRIPTION("LED - platform driver example");
MODULE_AUTHOR("Alberto Liberal");
MODULE_LICENSE("GPL");
