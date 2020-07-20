/* ************ LDD4EP(2): io_stm32mp1_expander.c ************ */
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
#include <linux/miscdevice.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/uaccess.h>

/* This structure will represent single device */
struct ioexp_dev {
	struct i2c_client *client;
	struct miscdevice ioexp_miscdevice;
	char name[8]; /* ioexpXX */
};

/* User is reading data from /dev/ioexpXX */
static ssize_t ioexp_read_file(struct file *file, char __user *userbuf,
		size_t count, loff_t *ppos)
{
	int expval, size;
	char buf[3];
	struct ioexp_dev *ioexp;

	ioexp = container_of(file->private_data, struct ioexp_dev, ioexp_miscdevice);

	/* read IO expander input to expval */
	expval = i2c_smbus_read_byte(ioexp->client);
	if (expval < 0)
		return -EFAULT;

	/* 
	 * converts expval in 2 characters (2bytes) + null value (1byte)
	 * The values converted are char values (FF) that match with the hex
	 * int(s32) value of the expval variable.
	 * if we want to get the int value again, we have to
	 * do Kstrtoul(). We convert 1 byte int value to
	 * 2 bytes char values. For instance 255 (1 int byte) = FF (2 char bytes).
	 */
	size = sprintf(buf, "%02x", expval);

	/* 
	 * replace NULL by \n. It is not needed to have the char array
	 * ended with \0 character.
	 */
	buf[size] = '\n';

	/* send size+1 to include the \n character */
	if (*ppos == 0) {
		if (copy_to_user(userbuf, buf, size+1)) {
			pr_info("Failed to return led_value to user space\n");
			return -EFAULT;
		}
		*ppos+=1;
		return size+1;
	}

	return 0;
}

/* Writing from the terminal command line, \n is added */
static ssize_t ioexp_write_file(struct file *file, const char __user *userbuf,
		size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	char buf[4];
	struct ioexp_dev *ioexp;

	ioexp = container_of(file->private_data, struct ioexp_dev, ioexp_miscdevice);

	dev_info(&ioexp->client->dev, 
			"ioexp_write_file entered on %s\n", ioexp->name);

	dev_info(&ioexp->client->dev,
			"we have written %zu characters\n", count); 

	if (copy_from_user(buf, userbuf, count)) {
		dev_err(&ioexp->client->dev, "Bad copied value\n");
		return -EFAULT;
	}

	buf[count-1] = '\0';

	/* convert the string to an unsigned long */
	ret = kstrtoul(buf, 0, &val);
	if (ret)
		return -EINVAL;

	dev_info(&ioexp->client->dev, "the value is %lu\n", val);

	ret = i2c_smbus_write_byte(ioexp->client, val);
	if (ret < 0)
		dev_err(&ioexp->client->dev, "the device is not found\n");

	dev_info(&ioexp->client->dev, 
			"ioexp_write_file exited on %s\n", ioexp->name);

	return count;
}

static const struct file_operations ioexp_fops = {
	.owner = THIS_MODULE,
	.read = ioexp_read_file,
	.write = ioexp_write_file,
};

static int ioexp_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	static int counter = 0;

	struct ioexp_dev *ioexp;

	/* Allocate new structure representing device */
	ioexp = devm_kzalloc(&client->dev, sizeof(struct ioexp_dev), GFP_KERNEL);

	/* Store pointer to the device-structure in bus device context */
	i2c_set_clientdata(client,ioexp);

	/* Store pointer to I2C device/client */
	ioexp->client = client;

	/* Initialize the misc device, ioexp incremented after each probe call */
	sprintf(ioexp->name, "ioexp%02d", counter++); 
	dev_info(&client->dev, 
			"ioexp_probe is entered on %s\n", ioexp->name);

	ioexp->ioexp_miscdevice.name = ioexp->name;
	ioexp->ioexp_miscdevice.minor = MISC_DYNAMIC_MINOR;
	ioexp->ioexp_miscdevice.fops = &ioexp_fops;

	/* Register misc device */
	return misc_register(&ioexp->ioexp_miscdevice);

	dev_info(&client->dev, 
			"ioexp_probe is exited on %s\n", ioexp->name);

	return 0;
}

static int ioexp_remove(struct i2c_client *client)
{
	struct ioexp_dev *ioexp;

	/* Get device structure from bus device context */	
	ioexp = i2c_get_clientdata(client);

	dev_info(&client->dev, 
			"ioexp_remove is entered on %s\n", ioexp->name);

	/* Deregister misc device */
	misc_deregister(&ioexp->ioexp_miscdevice);

	dev_info(&client->dev, 
			"ioexp_remove is exited on %s\n", ioexp->name);

	return 0;
}

static const struct of_device_id ioexp_dt_ids[] = {
	{ .compatible = "arrow,ioexp", },
	{ }
};
MODULE_DEVICE_TABLE(of, ioexp_dt_ids);

static const struct i2c_device_id i2c_ids[] = {
	{ .name = "ioexp", },
	{ }
};
MODULE_DEVICE_TABLE(i2c, i2c_ids);

static struct i2c_driver ioexp_driver = {
	.driver = {
		.name = "ioexp",
		.owner = THIS_MODULE,
		.of_match_table = ioexp_dt_ids,
	},
	.probe = ioexp_probe,
	.remove = ioexp_remove,
	.id_table = i2c_ids,
};

module_i2c_driver(ioexp_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a driver that controls several i2c IO expanders");
