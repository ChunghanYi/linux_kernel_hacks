/* ************ LDD4EP(2): ltc3206_stm32mp1_led_class.c ************ */
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
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>

#define LED_NAME_LEN	32
#define CMD_RED_SHIFT	4
#define CMD_BLUE_SHIFT	4
#define CMD_GREEN_SHIFT	0
#define CMD_MAIN_SHIFT	4
#define CMD_SUB_SHIFT	0
#define EN_CS_SHIFT	(1 << 2)

/* set a led_device struct for each 5 led device */
struct led_device {
	u8 brightness;
	struct led_classdev cdev;
	struct led_priv *private;
};

/* 
 * store the global parameters shared for the 5 led devices
 * the parameters are updated after each led_control() call
 */
struct led_priv {
	u32 num_leds;
	u8 command[3];
	struct gpio_desc *display_cs;
	struct i2c_client *client;
};

/* function that writes to the I2C device */
static int ltc3206_led_write(struct i2c_client *client, u8 *command)
{
	int ret = i2c_master_send(client, command, 3);
	if (ret >= 0)
		return 0;
	return ret;
}

/* the sysfs functions */
static ssize_t sub_select(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	char *buffer;
	struct i2c_client *client;
	struct led_priv *private;

	buffer = (char *)buf;

	/* replace \n added from terminal with \0 */
	*(buffer+(count-1)) = '\0';

	client = to_i2c_client(dev);
	private = i2c_get_clientdata(client);

	private->command[0] |= EN_CS_SHIFT; /* set the 3d bit A2 */
	ltc3206_led_write(private->client, private->command);

	if (!strcmp(buffer, "on")) {
		gpiod_set_value(private->display_cs, 1); /* low */
		usleep_range(100, 200);
		gpiod_set_value(private->display_cs, 0); /* high */
	} else if (!strcmp(buffer, "off")) {
		gpiod_set_value(private->display_cs, 0); /* high */
		usleep_range(100, 200);
		gpiod_set_value(private->display_cs, 1); /* low */
	} else {
		dev_err(&client->dev, "Bad led value.\n");
		return -EINVAL;
	}

	return count;
}
static DEVICE_ATTR(sub, S_IWUSR, NULL, sub_select);

static ssize_t rgb_select(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	char *buffer;
	struct i2c_client *client = to_i2c_client(dev);
	struct led_priv *private = i2c_get_clientdata(client);
	buffer = (char *)buf;

	*(buffer+(count-1)) = '\0';

	private->command[0] &= ~(EN_CS_SHIFT); /* clear the 3d bit */

	ltc3206_led_write(private->client, private->command);

	if (!strcmp(buffer, "on")) {
		gpiod_set_value(private->display_cs, 1); /* low */
		usleep_range(100, 200);
		gpiod_set_value(private->display_cs, 0); /* high */
	} else if (!strcmp(buffer, "off")) {
		gpiod_set_value(private->display_cs, 0); /* high */
		usleep_range(100, 200);
		gpiod_set_value(private->display_cs, 1); /* low */
	} else {
		dev_err(&client->dev, "Bad led value.\n");
		return -EINVAL;
	}

	return count;
}
static DEVICE_ATTR(rgb, S_IWUSR, NULL, rgb_select);

static struct attribute *display_cs_attrs[] = {
	&dev_attr_rgb.attr,
	&dev_attr_sub.attr,
	NULL,
};

static struct attribute_group display_cs_group = {
	.name = "display_cs",
	.attrs = display_cs_attrs,
};

/* 
 * this is the function that is called for each led device
 * when writing the brightness file under each device
 * the global parameters are kept in the led_priv struct
 * that is pointed inside each led_device struct
 */
static int led_control(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	struct led_classdev *cdev;
	struct led_device *led;
	led = container_of(led_cdev, struct led_device, cdev);
	cdev = &led->cdev;
	led->brightness = value;

	dev_info(cdev->dev, "the subsystem is %s\n", cdev->name);

	if (value > 15 || value < 0)
		return -EINVAL;

	if (strcmp(cdev->name,"red") == 0) {
		led->private->command[0] &= 0x0F; /* clear the upper nibble */
		led->private->command[0] |= ((led->brightness << CMD_RED_SHIFT) & 0xF0);
	} else if (strcmp(cdev->name,"blue") == 0) {
		led->private->command[1] &= 0x0F; /* clear the upper nibble */
		led->private->command[1] |= ((led->brightness << CMD_BLUE_SHIFT) & 0xF0);
	} else if (strcmp(cdev->name,"green") == 0) {
		led->private->command[1] &= 0xF0; /* clear the lower nibble */
		led->private->command[1] |= ((led->brightness << CMD_GREEN_SHIFT) & 0x0F);
	} else if (strcmp(cdev->name,"main") == 0) {
		led->private->command[2] &= 0x0F; /* clear the upper nibble */
		led->private->command[2] |= ((led->brightness << CMD_MAIN_SHIFT) & 0xF0);
	} else if (strcmp(cdev->name,"sub") == 0) {
		led->private->command[2] &= 0xF0; //clear the lower nibble
		led->private->command[2] |= ((led->brightness << CMD_SUB_SHIFT) & 0x0F);
	} else
		dev_info(cdev->dev, "No display found\n");

	return ltc3206_led_write(led->private->client, led->private->command);
}

static int ltc3206_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int count, ret;
	u8 value[3];
	struct fwnode_handle *child;
	struct device *dev = &client->dev;
	struct led_priv *private;

	dev_info(dev, "platform_probe enter\n");

	/* 
	 * set blue led maximum value for i2c testing
	 * ENRGB must be set to VCC 
	 */
	value[0] = 0x00;
	value[1] = 0xF0;
	value[2] = 0x00;

	i2c_master_send(client, value, 3);

	dev_info(dev, "led BLUE is ON\n");

	count = device_get_child_node_count(dev);
	if (!count)
		return -ENODEV;

	dev_info(dev, "there are %d nodes\n", count);

	private = devm_kzalloc(dev, sizeof(*private), GFP_KERNEL);
	if (!private)
		return -ENOMEM;

	private->client = client;
	i2c_set_clientdata(client, private);

	private->display_cs = devm_gpiod_get(dev, NULL, GPIOD_ASIS);
	if (IS_ERR(private->display_cs)) {
		ret = PTR_ERR(private->display_cs);
		dev_err(dev, "Unable to claim gpio\n");
		return ret;
	}

	gpiod_direction_output(private->display_cs, 1);

	/* Register sysfs hooks */
	ret = sysfs_create_group(&client->dev.kobj, &display_cs_group);
	if (ret < 0) {
		dev_err(&client->dev, "couldn't register sysfs group\n");
		return ret;
	}

	/* parse all the child nodes */
	device_for_each_child_node(dev, child) {
		struct led_device *led_device;
		struct led_classdev *cdev;

		led_device = devm_kzalloc(dev, sizeof(*led_device), GFP_KERNEL);
		if (!led_device)
			return -ENOMEM;

		cdev = &led_device->cdev;
		led_device->private = private;

		fwnode_property_read_string(child, "label", &cdev->name);

		if (strcmp(cdev->name,"main") == 0) {
			led_device->cdev.brightness_set_blocking = led_control;
			ret = devm_led_classdev_register(dev, &led_device->cdev);
			if (ret)
				goto err;
			dev_info(cdev->dev, "the subsystem is %s and num is %d\n", 
					cdev->name, private->num_leds);
		} else if (strcmp(cdev->name,"sub") == 0) {
			led_device->cdev.brightness_set_blocking = led_control;
			ret = devm_led_classdev_register(dev, &led_device->cdev);
			if (ret)
				goto err;
			dev_info(cdev->dev, "the subsystem is %s and num is %d\n", 
					cdev->name, private->num_leds);
		} else if (strcmp(cdev->name,"red") == 0) {
			led_device->cdev.brightness_set_blocking = led_control;
			ret = devm_led_classdev_register(dev, &led_device->cdev);
			if (ret)
				goto err;
			dev_info(cdev->dev, "the subsystem is %s and num is %d\n", 
					cdev->name, private->num_leds);
		} else if (strcmp(cdev->name,"green") == 0) {
			led_device->cdev.brightness_set_blocking = led_control;
			ret = devm_led_classdev_register(dev, &led_device->cdev);
			if (ret)
				goto err;
			dev_info(cdev->dev, "the subsystem is %s and num is %d\n", 
					cdev->name, private->num_leds);
		} else if (strcmp(cdev->name,"blue") == 0) {
			led_device->cdev.brightness_set_blocking = led_control;
			ret = devm_led_classdev_register(dev, &led_device->cdev);
			if (ret)
				goto err;
			dev_info(cdev->dev, "the subsystem is %s and num is %d\n", 
					cdev->name, private->num_leds);
		} else {
			dev_err(dev, "Bad device tree value\n");
			return -EINVAL;
		}

		private->num_leds++;
	}

	dev_info(dev, "i am out of the device tree\n");
	dev_info(dev, "my_probe() function is exited.\n");
	return 0;

err:
	fwnode_handle_put(child);
	sysfs_remove_group(&client->dev.kobj, &display_cs_group);

	return ret;
}

static int ltc3206_remove(struct i2c_client *client)
{
	dev_info(&client->dev, "leds_remove enter\n");
	sysfs_remove_group(&client->dev.kobj, &display_cs_group);
	dev_info(&client->dev, "leds_remove exit\n");

	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,ltc3206"},
	{},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static const struct i2c_device_id ltc3206_id[] = {
	{ "ltc3206", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ltc3206_id);

static struct i2c_driver ltc3206_driver = {
	.probe = ltc3206_probe,
	.remove = ltc3206_remove,
	.id_table	= ltc3206_id,
	.driver = {
		.name = "ltc3206",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

module_i2c_driver(ltc3206_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a driver that controls the \
		ltc3206 I2C multidisplay device");
