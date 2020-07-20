/* ************ LDD4EP(2): keyled_stm32mp1_class.c ************ */
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
#include <linux/property.h>
#include <linux/kthread.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/of_device.h>

#define LED_NAME_LEN		32
#define INT_NUMBER		2
static const char *HELLO_KEYS_NAME1 = "KEY1";
static const char *HELLO_KEYS_NAME2 = "KEY2";

/* Specific LED private structure */
struct led_device {
	char name[LED_NAME_LEN];
	struct gpio_desc *ledd; /* each LED gpio_desc */
	struct device *dev; 
	struct keyled_priv *private; /* pointer to the global private struct */
};

/* Global private structure */
struct keyled_priv {
	u32 num_leds;
	u8 led_flag;
	u8 task_flag;
	u32 period;
	spinlock_t period_lock;
	struct task_struct *task; 	/* kthread task_struct */
	struct class *led_class;  	/* the keyled class */
	struct device *dev;
	dev_t led_devt;		  	/* first device identifier */
	struct led_device *leds[];	/* pointers to each led private struct */
};

/* kthread function */
static int led_flash(void *data)
{
	unsigned long flags;
	u32 value = 0;
	u32 period;
	struct led_device *led_dev = data;

	dev_info(led_dev->dev, "Task started\n");
	dev_info(led_dev->dev, "I am inside the kthread\n");

	while (!kthread_should_stop()) {
		spin_lock_irqsave(&led_dev->private->period_lock, flags);
		period = led_dev->private->period;
		spin_unlock_irqrestore(&led_dev->private->period_lock, flags);
		value = !value;
		gpiod_set_value(led_dev->ledd, value);
		msleep(period/2);
	}
	gpiod_set_value(led_dev->ledd, 1); /* switch off the led */
	dev_info(led_dev->dev, "Task completed\n");

	return 0;
}

/*
 * sysfs methods
 */

/* switch on/of each led */
static ssize_t set_led_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int i;
	char *buffer = (char *)buf;
	struct led_device *led_count;
	struct led_device *led = dev_get_drvdata(dev);

	/* replace \n added from terminal with \0 */
	*(buffer+(count-1)) = '\0';

	if (led->private->task_flag == 1) {
		kthread_stop(led->private->task);
		led->private->task_flag = 0;
	}

	if(!strcmp(buffer, "on")) {
		if (led->private->led_flag == 1) { 
			for (i = 0; i < led->private->num_leds; i++) {
				led_count = led->private->leds[i];
				gpiod_set_value(led_count->ledd, 1); 
			}
			gpiod_set_value(led->ledd, 0); 
		} else { 
			gpiod_set_value(led->ledd, 0);
			led->private->led_flag = 1;
		}
	} else if (!strcmp(buffer, "off")) {
		gpiod_set_value(led->ledd, 1);
	} else {
		dev_info(led->dev, "Bad led value.\n");
		return -EINVAL;
	}

	return count;
}
static DEVICE_ATTR_WO(set_led);

/* blinking ON the specific LED running a kthread */
static ssize_t blink_on_led_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int i;
	char *buffer = (char *)buf;
	struct led_device *led_count;
	struct led_device *led = dev_get_drvdata(dev);

	/* replace \n added from terminal with \0 */
	*(buffer+(count-1)) = '\0';

	if (led->private->led_flag == 1) { 
		for (i = 0; i < led->private->num_leds; i++) {
			led_count = led->private->leds[i];
			gpiod_set_value(led_count->ledd, 1); 
		}
	}

	if(!strcmp(buffer, "on")) {
		if (led->private->task_flag == 0) {
			led->private->task = kthread_run(led_flash, led, "Led_flash_tread");
			if (IS_ERR(led->private->task)) {
				dev_info(led->dev, "Failed to create the task\n");
				return PTR_ERR(led->private->task);
			}
		} else
			return -EBUSY;
	} else {
		dev_info(led->dev, "Bad led value.\n");
		return -EINVAL;
	}

	led->private->task_flag = 1;

	dev_info(led->dev, "Blink_on_led exited\n");
	return count;
}
static DEVICE_ATTR_WO(blink_on_led);

/* switch off the blinking of any led */
static ssize_t blink_off_led_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int i;
	char *buffer = (char *)buf;
	struct led_device *led = dev_get_drvdata(dev);
	struct led_device *led_count;

	/* replace \n added from terminal with \0 */
	*(buffer+(count-1)) = '\0';

	if (!strcmp(buffer, "off")) {
		if (led->private->task_flag == 1) {
			kthread_stop(led->private->task);
			for (i = 0; i < led->private->num_leds; i++) {
				led_count = led->private->leds[i];
				gpiod_set_value(led_count->ledd, 1);
			}
		} else
			return 0;
	} else {
		dev_info(led->dev, "Bad led value.\n");
		return -EINVAL;
	}

	led->private->task_flag = 0;
	return count;
}
static DEVICE_ATTR_WO(blink_off_led);

/* set the blinking period */
static ssize_t set_period_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long flags;
	int ret, period;
	struct led_device *led = dev_get_drvdata(dev);
	dev_info(led->dev, "Enter set_period\n");

	ret = sscanf(buf, "%u", &period);
	if (ret < 1 || period < 10 || period > 10000) {
		dev_err(dev, "invalid value\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&led->private->period_lock, flags);
	led->private->period = period;
	spin_unlock_irqrestore(&led->private->period_lock, flags);

	dev_info(led->dev, "period is set\n");
	return count;
}
static DEVICE_ATTR_WO(set_period);

/* Declare the sysfs structures */
static struct attribute *led_attrs[] = {
	&dev_attr_set_led.attr,
	&dev_attr_blink_on_led.attr,
	&dev_attr_blink_off_led.attr,
	&dev_attr_set_period.attr,
	NULL,
};

static const struct attribute_group led_group = {
	.attrs = led_attrs,
};

static const struct attribute_group *led_groups[] = {
	&led_group,
	NULL,
};

/* 
 * Allocate space for the global private struct 
 * and the three local LED private structs
 */
static inline int sizeof_keyled_priv(int num_leds)
{
	return sizeof(struct keyled_priv) + (sizeof(struct led_device*) * num_leds);
}

/* First interrupt handler */
static irqreturn_t KEY_ISR1(int irq, void *data)
{
	struct keyled_priv *priv = data;

	dev_info(priv->dev, "interrupt KEY1 received. key: %s\n",
			HELLO_KEYS_NAME1);

	spin_lock(&priv->period_lock);
	priv->period = priv->period + 10;
	if ((priv->period < 10) || (priv->period > 10000))
		priv->period = 10;
	spin_unlock(&priv->period_lock);

	dev_info(priv->dev, "the led period is %d\n", priv->period);
	return IRQ_HANDLED;
}

/* Second interrupt handler */
static irqreturn_t KEY_ISR2(int irq, void *data)
{
	struct keyled_priv *priv = data;
	dev_info(priv->dev, "interrupt KEY2 received. key: %s\n",
			HELLO_KEYS_NAME2);

	spin_lock(&priv->period_lock);
	priv->period = priv->period - 10;
	if ((priv->period < 10) || (priv->period > 10000))
		priv->period = 10;
	spin_unlock(&priv->period_lock);

	dev_info(priv->dev, "the led period is %d\n", priv->period);
	return IRQ_HANDLED;
}

/* Create the LED devices under the sysfs keyled entry */
struct led_device *led_device_register(const char *name, int count,
		struct device *parent, dev_t led_devt, struct class *led_class)
{
	struct led_device *led;
	dev_t devt;
	int ret;

	/* First allocate a new led device */
	led = devm_kzalloc(parent, sizeof(struct led_device), GFP_KERNEL);
	if (!led)
		return ERR_PTR(-ENOMEM);

	/* Get the minor number of each device */
	devt = MKDEV(MAJOR(led_devt), count);

	/* Create the device and init the device's data */
	led->dev = device_create(led_class, parent, devt, led, "%s", name); 
	if (IS_ERR(led->dev)) {
		dev_err(led->dev, "unable to create device %s\n", name);
		ret = PTR_ERR(led->dev);
		return ERR_PTR(ret);
	}

	dev_info(led->dev, "the major number is %d\n", MAJOR(led_devt));
	dev_info(led->dev, "the minor number is %d\n", MINOR(devt));

	/* To recover later from each sysfs entry */
	dev_set_drvdata(led->dev, led);

	strncpy(led->name, name, LED_NAME_LEN);

	dev_info(led->dev, "led %s added\n", led->name);

	return led;
}

static int my_probe(struct platform_device *pdev)
{	
	int count, ret, i;
	unsigned int major;
	struct fwnode_handle *child;

	struct device *dev = &pdev->dev;
	struct keyled_priv *priv;

	dev_info(dev, "my_probe() function is called.\n");

	count = device_get_child_node_count(dev);
	if (!count)
		return -ENODEV;

	dev_info(dev, "there are %d nodes\n", count);

	/* Allocate all the private structures */
	priv = devm_kzalloc(dev, sizeof_keyled_priv(count-INT_NUMBER), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* Allocate 3 device numbers */
	alloc_chrdev_region(&priv->led_devt, 0, count-INT_NUMBER, "Keyled_class");
	major = MAJOR(priv->led_devt);
	dev_info(dev, "the major number is %d\n", major);

	/* Create the LED class */
	priv->led_class = class_create(THIS_MODULE, "keyled");
	if (!priv->led_class) {
		dev_info(dev, "failed to allocate class\n");
		return -ENOMEM;
	}

	/* Set attributes for this class */
	priv->led_class->dev_groups = led_groups;
	priv->dev = dev;

	spin_lock_init(&priv->period_lock);

	/* Parse all the DT nodes */
	device_for_each_child_node(dev, child) {
		int irq, flags;
		struct gpio_desc *keyd;
		const char *label_name, *colour_name, *trigger;
		struct led_device *new_led;

		fwnode_property_read_string(child, "label", &label_name);

		/* Parsing the DT LED nodes */
		if (strcmp(label_name,"led") == 0) {
			fwnode_property_read_string(child, "colour", &colour_name);

			/*
			 * Create led devices under keyled class
			 * priv->num_leds is 0 for the first iteration
			 * used to set the minor number of each device
			 * increased to the end of the iteration
			 */
			new_led = led_device_register(colour_name, priv->num_leds, dev,
					priv->led_devt, priv->led_class);
			if (!new_led) {

				fwnode_handle_put(child);
				ret = PTR_ERR(new_led);

				for (i = 0; i < priv->num_leds-1; i++) {
					device_destroy(priv->led_class, 
							MKDEV(MAJOR(priv->led_devt), i));
				}
				class_destroy(priv->led_class);
				return ret;
			}

			new_led->ledd = devm_fwnode_get_gpiod_from_child(dev, NULL, child, GPIOD_ASIS, colour_name);
			if (IS_ERR(new_led->ledd)) {
				fwnode_handle_put(child);
				ret = PTR_ERR(new_led->ledd);
				goto error;
			}
			new_led->private = priv;
			priv->leds[priv->num_leds] = new_led;
			priv->num_leds++;

			/* set direction to output */
			gpiod_direction_output(new_led->ledd, 1);

			/* set led state to off */
			gpiod_set_value(new_led->ledd, 1);

		} else if (strcmp(label_name,"KEY_1") == 0) { /* Parsing the interrupt nodes */
			keyd = devm_fwnode_get_gpiod_from_child(dev, NULL, child, GPIOD_ASIS, label_name);
			gpiod_direction_input(keyd);
			fwnode_property_read_string(child, "trigger", &trigger);
			if (strcmp(trigger, "falling") == 0)
				flags = IRQF_TRIGGER_FALLING;
			else if (strcmp(trigger, "rising") == 0)
				flags = IRQF_TRIGGER_RISING;
			else if (strcmp(trigger, "both") == 0)
				flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
			else
				return -EINVAL;

			irq = gpiod_to_irq(keyd);
			if (irq < 0)
				return irq;

			ret = devm_request_irq(dev, irq, KEY_ISR1, flags, "ISR1", priv);
			if (ret) {
				dev_err(dev, "Failed to request interrupt %d, error %d\n", 
						irq, ret);
				return ret;
			}
			dev_info(dev, "IRQ number: %d\n", irq);

		} else if (strcmp(label_name,"KEY_2") == 0) {
			keyd = devm_fwnode_get_gpiod_from_child(dev, NULL, child, GPIOD_ASIS, label_name);
			gpiod_direction_input(keyd);
			fwnode_property_read_string(child, "trigger", &trigger);
			if (strcmp(trigger, "falling") == 0)
				flags = IRQF_TRIGGER_FALLING;
			else if (strcmp(trigger, "rising") == 0)
				flags = IRQF_TRIGGER_RISING;
			else if (strcmp(trigger, "both") == 0)
				flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
			else
				return -EINVAL;

			irq = gpiod_to_irq(keyd);
			if (irq < 0)
				return irq;

			ret = devm_request_irq(dev, irq, KEY_ISR2, flags, "ISR2", priv);
			if (ret < 0) {
				dev_err(dev, "Failed to request interrupt %d, error %d\n", 
						irq, ret);
				goto error;
			}
			dev_info(dev, "IRQ number: %d\n", irq);

		} else {
			dev_info(dev, "Bad device tree value\n");
			ret = -EINVAL;
			goto error;
		}
	}

	dev_info(dev, "i am out of the device tree\n");

	/* reset period to 10 */
	priv->period = 10;

	dev_info(dev, "the led period is %d\n", priv->period);

	platform_set_drvdata(pdev, priv);

	dev_info(dev, "my_probe() function is exited.\n");

	return 0;

error:
	/* Unregister everything in case of errors */
	for (i = 0; i < priv->num_leds; i++)
		device_destroy(priv->led_class, MKDEV(MAJOR(priv->led_devt), i));
	class_destroy(priv->led_class);
	unregister_chrdev_region(priv->led_devt, priv->num_leds);
	return ret;
}

static int my_remove(struct platform_device *pdev)
{	
	int i;
	struct led_device *led_count;
	struct keyled_priv *priv = platform_get_drvdata(pdev);
	dev_info(&pdev->dev, "my_remove() function is called.\n");

	if (priv->task_flag == 1) {
		kthread_stop(priv->task);
		priv->task_flag = 0;
	}

	if (priv->led_flag == 1) {
		for (i = 0; i < priv->num_leds; i++) {
			led_count = priv->leds[i];
			gpiod_set_value(led_count->ledd, 1);
		}
	}

	for (i = 0; i < priv->num_leds; i++)
		device_destroy(priv->led_class, MKDEV(MAJOR(priv->led_devt), i));
	class_destroy(priv->led_class);
	unregister_chrdev_region(priv->led_devt, priv->num_leds);
	dev_info(&pdev->dev, "my_remove() function is exited.\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,ledpwm"},
	{},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "ledpwm",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a platform keyled_class driver that decreases \
		and increases the led flashing period");
