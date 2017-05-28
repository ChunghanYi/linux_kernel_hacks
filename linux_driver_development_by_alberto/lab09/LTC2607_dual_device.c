/* ************ LDD4EP: listing9-2: LTC2607_dual_device.c ************ */
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
#include <linux/iio/iio.h>

#define LTC2607_DRV_NAME "ltc2607"

struct ltc2607_device {
	struct i2c_client *client;
	struct iio_dev *indio_dev;
	char name[8]; 
};

static const struct iio_chan_spec ltc2607_channel[] = {
	{
		.type	= IIO_VOLTAGE,
		.indexed	= 1,
		.output	= 1,
		.channel	= 0,
		.info_mask_separate	= BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type	= IIO_VOLTAGE,
		.indexed	= 1,
		.output	= 1,
		.channel	= 1,
		.info_mask_separate	= BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type	= IIO_VOLTAGE,
		.indexed	= 1,
		.output	= 1,
		.channel	= 2,
		.info_mask_separate	= BIT(IIO_CHAN_INFO_RAW),
	}
};

static int ltc2607_set_value(struct iio_dev *indio_dev, int val, int channel)
{
	struct ltc2607_device *data = iio_device_get_drvdata(indio_dev);
	u8 outbuf[3];
	int ret;
	int chan;

	if (channel == 2)
		chan = 0x0F;
	else
		chan = channel;

	if (val >= (1 << 16) || val < 0)
		return -EINVAL;
	outbuf[0] = 0x30 | chan; // write and update DAC
	outbuf[1] = (val >> 8) & 0xff; // MSB byte of dac_code
	outbuf[2] = val & 0xff; // LSB byte of dac_code

	ret = i2c_master_send(data->client, outbuf, 3);
	if (ret < 0)
		return ret;
	else if (ret != 3)
		return -EIO;
	else
		return 0;
}

static int ltc2607_write_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan,
		int val, int val2, long mask)
{
	int ret;

	switch (mask) {
		case IIO_CHAN_INFO_RAW:
			ret = ltc2607_set_value(indio_dev, val, chan->channel);
			break;
		default:
			ret = -EINVAL;
			break;
	}

	return ret;
}

static const struct iio_info ltc2607_info = {
	.write_raw = ltc2607_write_raw,
	.driver_module = THIS_MODULE,
};

static int ltc2607_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	static int counter = 0;
	struct ltc2607_device *data;
	u8 inbuf[3];
	u8 command_byte;
	int err;
	dev_info(&client->dev, "DAC_probe()\n");

	command_byte = 0x30 | 0x00; /* Write and update register with value 0xFF*/
	inbuf[0] = command_byte;
	inbuf[1] = 0xFF;
	inbuf[2] = 0xFF;

	/* allocate private structure for new device */
	data = devm_kzalloc(&client->dev, sizeof(struct ltc2607_device), GFP_KERNEL);

	data->indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (data->indio_dev == NULL) 
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	data->client = client;
	iio_device_set_drvdata(data->indio_dev, data);
	sprintf(data->name, "DAC%02d", counter++);
	data->indio_dev->name = data->name;
	data->indio_dev->dev.parent = &client->dev;
	data->indio_dev->info = &ltc2607_info;
	data->indio_dev->channels = ltc2607_channel;
	data->indio_dev->num_channels = 3;
	data->indio_dev->modes = INDIO_DIRECT_MODE;

	err = i2c_master_send(client, inbuf, 3); /* write DAC value */
	if (err < 0) {
		dev_err(&client->dev, "failed to write DAC value");
		return err;
	}

	pr_info("the dac answer is: %x.\n", err);

	err = devm_iio_device_register(&client->dev, data->indio_dev);
	if (err)
		return err;

	dev_info(&client->dev, "ltc2607 DAC registered\n");

	return 0;
}

static int ltc2607_remove(struct i2c_client *client)
{
	dev_info(&client->dev, "DAC_remove()\n");
	return 0;
}

static const struct of_device_id dac_dt_ids[] = {
	{ .compatible = "arrow,ltc2607", },
	{ }
};

MODULE_DEVICE_TABLE(of, dac_dt_ids);

static const struct i2c_device_id ltc2607_id[] = {
	{ "ltc2607", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ltc2607_id);

static struct i2c_driver ltc2607_driver = {
	.driver = {
		.name	= LTC2607_DRV_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = dac_dt_ids,
	},
	.probe		= ltc2607_probe,
	.remove		= ltc2607_remove,
	.id_table	= ltc2607_id,
};
module_i2c_driver(ltc2607_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("LTC2607 16-bit DAC");
