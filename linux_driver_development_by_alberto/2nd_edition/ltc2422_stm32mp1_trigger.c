/* ************ LDD4EP(2): ltc2422_stm32mp1_trigger.c ************ */
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
#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/iio/iio.h>
#include <linux/wait.h>

#define LTC2422_GPIO_NAME	"int"

struct ADC_data {
	struct gpio_desc *gpio;
	int irq;
	wait_queue_head_t wq_data_available;
	struct spi_device *spi;
	u8 buffer[4];
	bool conversion_done;
	struct mutex lock;
};

static irqreturn_t ltc2422_adc_interrupt(int irq, void *data)
{
	struct ADC_data *st = data;
	st->conversion_done = true;
	wake_up_interruptible(&st->wq_data_available);
	return IRQ_HANDLED;
}

static const struct iio_chan_spec ltc2422_channel[] = {
	{
		.type		= IIO_VOLTAGE,
		.indexed	= 1,
		.output		= 1,
		.channel	= 0,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	}
};

static int ltc2422_read_raw(struct iio_dev *indio_dev,
		struct iio_chan_spec const *chan, int *val, int *val2, long m)
{
	int ret;
	struct ADC_data *st = iio_priv(indio_dev);

	dev_info(&st->spi->dev, "Press MIKROBUS key to start conversion\n");

	switch (m) {
		case IIO_CHAN_INFO_RAW:
			mutex_lock(&st->lock);

			ret = wait_event_interruptible(st->wq_data_available, st->conversion_done);
			if (ret) {
				dev_err(&st->spi->dev, "Failed to request interrupt\n");
				return ret;
			}
			spi_read(st->spi, &st->buffer, 3);

			*val  = st->buffer[0] << 16;
			*val |= st->buffer[1] << 8;
			*val |= st->buffer[2];

			st->conversion_done = false;

			mutex_unlock(&st->lock);

			return IIO_VAL_INT;

		default:
			break;
	}
	return -EINVAL;
}

static const struct iio_info ltc2422_info = {
	.read_raw = &ltc2422_read_raw,
};

static int ltc2422_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct ADC_data *st;
	int ret;
	const struct spi_device_id *id;
	dev_info(&spi->dev, "my_probe() function is called.\n");

	/* get the id from the driver structure to use the name */
	id = spi_get_device_id(spi);

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if (indio_dev == NULL)
		return -ENOMEM;

	st = iio_priv(indio_dev); 
	st->spi = spi;
	spi_set_drvdata(spi, indio_dev);

	/* 
	 * you can also use
	 * devm_gpiod_get(&spi->dev, LTC2422_GPIO_NAME, GPIOD_IN);
	 */
	st->gpio = devm_gpiod_get_index(&spi->dev, LTC2422_GPIO_NAME, 0, GPIOD_IN);
	if (IS_ERR(st->gpio)) {
		dev_err(&spi->dev, "gpio get index failed\n");
		return PTR_ERR(st->gpio);
	}

	st->irq = gpiod_to_irq(st->gpio);
	if (st->irq < 0)
		return st->irq;
	dev_info(&spi->dev, "The IRQ number is: %d\n", st->irq);

	indio_dev->dev.parent = &spi->dev;
	indio_dev->channels = ltc2422_channel;
	indio_dev->info = &ltc2422_info;
	indio_dev->name = id->name;
	indio_dev->num_channels = 1;
	indio_dev->modes = INDIO_DIRECT_MODE;

	init_waitqueue_head(&st->wq_data_available);
	mutex_init(&st->lock);

	ret = devm_request_irq(&spi->dev, st->irq, ltc2422_adc_interrupt,
			IRQF_TRIGGER_FALLING, id->name, st);
	if (ret) {
		dev_err(&spi->dev, "failed to request interrupt %d (%d)", st->irq, ret);
		return ret;
	}
	ret = devm_iio_device_register(&spi->dev, indio_dev);
	if (ret < 0)
		return ret;

	st->conversion_done = false;

	return 0;
}

static int ltc2422_remove(struct spi_device *spi)
{
	dev_info(&spi->dev, "my_remove() function is called.\n");
	return 0;
}

static const struct of_device_id ltc2422_dt_ids[] = {
	{ .compatible = "arrow,ltc2422", },
	{ }
};
MODULE_DEVICE_TABLE(of, ltc2422_dt_ids);

static const struct spi_device_id ltc2422_id[] = {
	{ .name = "ltc2422", },
	{ }
};
MODULE_DEVICE_TABLE(spi, ltc2422_id);

static struct spi_driver ltc2422_driver = {
	.driver = {
		.name	= "ltc2422",
		.owner	= THIS_MODULE,
		.of_match_table = ltc2422_dt_ids,
	},
	.probe		= ltc2422_probe,
	.remove		= ltc2422_remove,
	.id_table	= ltc2422_id,
};

module_spi_driver(ltc2422_driver);

MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("LTC2422 DUAL ADC with triggering");
MODULE_LICENSE("GPL");
