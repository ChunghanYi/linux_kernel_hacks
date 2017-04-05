/*
 * hs_cdsm.c
 * Cds Light sensor 
 *
 * -------------------------------------------------------
 * [FC-03]                                      [UDOO Neo]
 * VCC(3.3~5V)        ......................... 3.3V
 * D0(digital input)  ......................... GPIO 180
 * GND                ......................... GND
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>

static int hs_cdsm_d0_pin = 180;	/* UDOO Neo J4 pin 18 */
static int gpio_irq = 0;

static irqreturn_t light_interrupt_handler(int irq, void *data)
{
	int value;

	value = gpio_get_value(hs_cdsm_d0_pin);
	pr_info(KERN_INFO "Value of DO pin is [%d]\n", value);
	
	return IRQ_HANDLED;
}

static int __init hs_cdsm_init(void)
{
	int ret;

	ret = gpio_request_one(hs_cdsm_d0_pin, GPIOF_IN, "LINE");
	if (ret < 0) {
		pr_info("failed to request GPIO %d, error %d\n", hs_cdsm_d0_pin, ret);
		return -1;
	}

	ret = gpio_to_irq(hs_cdsm_d0_pin);
	if (ret < 0) {
		pr_info(KERN_ERR "failed to set gpio IRQ(%d)\n", ret);
		goto error;
	} else {
		gpio_irq = ret;
	}

#if 1
	ret = request_irq(gpio_irq, light_interrupt_handler,
					IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "hs-cdsm-light", NULL);
#else
	ret = request_irq(gpio_irq, light_interrupt_handler,
					IRQF_TRIGGER_HIGH, "hs-cdsm-light", NULL);
#endif
	if (ret) {
		pr_info(KERN_ERR "failed to request IRQ(%d)\n", ret);
		goto error;
	}

	return 0;
error:
	return -1;
}

static void __exit hs_cdsm_exit(void)
{  
	synchronize_irq(gpio_irq);
	free_irq(gpio_irq, NULL);
	gpio_free(hs_cdsm_d0_pin);
}

module_init(hs_cdsm_init);
module_exit(hs_cdsm_exit);

MODULE_AUTHOR("Chunghan Yi");
MODULE_DESCRIPTION("hs_cdsm light sensor driver");
MODULE_LICENSE("GPL v2");
