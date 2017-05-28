/* ************ LDD4EP: listing9-6: LTC2422_dual_trigger_bottom.c ************ */
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
#include <linux/of_irq.h>
#include <linux/iio/iio.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

static char *HELLO_KEYS_NAME = "SW5";
static int gpio_nb;
static int irq;
static bool done;
static DECLARE_WAIT_QUEUE_HEAD(hello_keys_wait);

struct ADC_data {
	struct workqueue_struct *ADC_wq;
	struct work_struct work;
	struct spi_device *spi;
	uint8_t buffer[4];
	char name[8];
};

static irqreturn_t hello_keys_isr(int irq, void *data)
{
	struct iio_dev *indio_dev = data;
	struct ADC_data *job = iio_priv(indio_dev);

	pr_info("interrupt received. key: %s\n", HELLO_KEYS_NAME);

	/* recover ADC_data to enqueue work in the workqueue ADC_wq */
	queue_work(job->ADC_wq, &job->work);

	return IRQ_HANDLED;
}

static void ADC_trigger_int(struct work_struct *work_s)
{
	pr_info("ISR bottom done");
	done = true;
	wake_up(&hello_keys_wait);
}

static const struct iio_chan_spec ltc2422_channel[] = {
	{
		.type	= IIO_VOLTAGE,
		.indexed	= 1,
		.output	= 1,
		.channel	= 0,
		.info_mask_separate	= BIT(IIO_CHAN_INFO_RAW),
	}
};

static int ltc2422_read_raw(struct iio_dev *indio_dev,
		struct iio_chan_spec const *chan, int *val, int *val2, long m)
{
	int ret_val;
	struct ADC_data *st = iio_priv(indio_dev);

	pr_info("Press SW5 key to start conversion");

	switch (m) {
		case IIO_CHAN_INFO_RAW:
			ret_val = wait_event_interruptible(hello_keys_wait, done);
			if (ret_val) {
				pr_err("Failed to request interrupt\n");
				return ret_val;
			}
			spi_read(st->spi, &st->buffer, 3);

			*val  = st->buffer[0] << 16;
			*val |= st->buffer[1] << 8;
			*val |= st->buffer[2];

			pr_info("the value is %x\n", *val);
			done = false;
			return IIO_VAL_INT;

		default:
			break;
	}
	return -EINVAL;
}

static const struct iio_info ltc2422_info = {
	.read_raw = &ltc2422_read_raw,
	.driver_module = THIS_MODULE,
};

static int ltc2422_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct ADC_data *st;
	int ret_val;

	pr_info("my_probe() function is called.\n");
	done = false;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if (indio_dev == NULL)
		return -ENOMEM;

	st = iio_priv(indio_dev); 
	st->spi = spi;
	spi_set_drvdata(spi, indio_dev);

	indio_dev->dev.parent = &spi->dev;
	indio_dev->channels = ltc2422_channel;
	indio_dev->info = &ltc2422_info;
	sprintf(st->name, "LTC2422");
	indio_dev->name = st->name;
	indio_dev->num_channels = 1;
	indio_dev->modes = INDIO_DIRECT_MODE;

	gpio_nb = of_get_gpio(spi->dev.of_node, 0);
	pr_info("GPIO: %d\n", gpio_nb);
	irq = irq_of_parse_and_map(spi->dev.of_node, 0);
	pr_info("IRQ_parsing_devicetree: %d\n", irq);
	irq = gpio_to_irq(gpio_nb);
	pr_info("IRQ_using_gpio_to_irq: %d\n", irq);

	st->ADC_wq = alloc_workqueue("ADC_wq", 0, 0);
	INIT_WORK(&st->work, ADC_trigger_int);

	ret_val = devm_request_irq(indio_dev->dev.parent, irq,
					hello_keys_isr, IRQF_SHARED | IRQF_TRIGGER_RISING,
					HELLO_KEYS_NAME, indio_dev);
	if (ret_val) {
		pr_err("Failed to request interrupt %d, error %d\n",irq,ret_val);
		return ret_val;
	}
	ret_val = devm_iio_device_register(&spi->dev, indio_dev);
	if (ret_val < 0)
		return ret_val;

	return 0;
}

static int ltc2422_remove(struct spi_device *spi)
{
	struct iio_dev *indio_dev = spi_get_drvdata(spi);
	struct ADC_data *st = iio_priv(indio_dev);

	pr_info("my_remove() function is called.\n");
	flush_workqueue(st->ADC_wq);
	destroy_workqueue(st->ADC_wq);
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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("LTC2422 DUAL ADC with hardware triggering");
