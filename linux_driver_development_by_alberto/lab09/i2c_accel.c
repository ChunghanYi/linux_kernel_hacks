/* ************ LDD4EP: listing9-1: i2c_accel.c ************ */
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
#include <linux/i2c.h>
#include <linux/input-polldev.h>

/* create private structure */
struct ioaccel_dev {
	struct i2c_client *i2c_client;
	struct input_polled_dev *polled_input;
};

#define CRTL_REG1		0x2a
#define CRTL_REG1_ACTIVE	(1<<0)
#define OUT_X_MSB		0x1

/* poll function */
static void ioaccel_poll(struct input_polled_dev *pl_dev)
{
	struct ioaccel_dev *ioaccel = pl_dev->private;
	int val = 0;

	val = i2c_smbus_read_byte_data(ioaccel->i2c_client, OUT_X_MSB);

	if ((val > 0x10) && (val < 0x20)) {
		input_event(ioaccel->polled_input->input, EV_KEY, KEY_1, 1);
	} else {
		input_event(ioaccel->polled_input->input, EV_KEY, KEY_1, 0);
	}

	if ((val > 0x20) && (val < 0xe0)) {
		input_event(ioaccel->polled_input->input, EV_KEY, KEY_2, 1);
	} else {
		input_event(ioaccel->polled_input->input, EV_KEY, KEY_2, 0);
	}

	input_sync(ioaccel->polled_input->input);
}

static void ioaccel_open(struct input_polled_dev *pl_dev)
{

}

static int ioaccel_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	/* declare an instance of the private structure */
	struct ioaccel_dev * ioaccel;

	pr_info("my_probe() function is called.\n");

	/* allocate private structure for new device */
	ioaccel = devm_kzalloc(&client->dev, sizeof(struct ioaccel_dev), GFP_KERNEL);

	dev_info(&client->dev, "ioaccel_probe()\n");

	/* Associate client->dev with ioaccel private structure */
	i2c_set_clientdata(client,ioaccel);

	i2c_smbus_write_byte_data(client, CRTL_REG1, CRTL_REG1_ACTIVE);

	/* Allocate the struct input_polled_dev */
	ioaccel->polled_input = input_allocate_polled_device();

	/* Initialize polled input */
	ioaccel->i2c_client = client;
	ioaccel->polled_input->private = ioaccel;

	ioaccel->polled_input->poll_interval = 50;
	ioaccel->polled_input->poll = ioaccel_poll;
	ioaccel->polled_input->open = ioaccel_open;

	ioaccel->polled_input->input->dev.parent = &client->dev;

	ioaccel->polled_input->input->name = "IOACCEL keyboard";
	ioaccel->polled_input->input->id.bustype = BUS_I2C;

	/* Set event types */
	set_bit(EV_KEY, ioaccel->polled_input->input->evbit);
	set_bit(KEY_1, ioaccel->polled_input->input->keybit);
	set_bit(KEY_2, ioaccel->polled_input->input->keybit);

	/* Register the device, now the device is global until being unregistered */
	input_register_polled_device(ioaccel->polled_input);
	return 0;
}

static int ioaccel_remove(struct i2c_client *client)
{
	struct ioaccel_dev *ioaccel;

	ioaccel = i2c_get_clientdata(client);
	input_unregister_polled_device(ioaccel->polled_input);
	input_free_polled_device(ioaccel->polled_input);
	devm_kfree(&client->dev, ioaccel);
	dev_info(&client->dev, "ioaccel_remove()\n");
	return 0;
}

/* Add entries to device tree */
static const struct of_device_id ioaccel_dt_ids[] = {
	{ .compatible = "arrow,mma8451", },
	{ }
};
MODULE_DEVICE_TABLE(of, ioaccel_dt_ids);

static const struct i2c_device_id i2c_ids[] = {
	{ .name = "mma8451", },
	{ }
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

/* create struct i2c_driver */
static struct i2c_driver ioaccel_driver = {
	.driver = {
		.name = "mma8451",
		.owner = THIS_MODULE,
		.of_match_table = ioaccel_dt_ids,
	},
	.probe = ioaccel_probe,
	.remove = ioaccel_remove,
	.id_table = i2c_ids,
};

/* Register to i2c bus as a driver */
module_i2c_driver(ioaccel_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is an accel INPUT framework platform driver");
