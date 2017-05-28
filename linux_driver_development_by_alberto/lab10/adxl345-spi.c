/* ************ LDD4EP: listing10-1: adxl345-spi.c ************ */
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

#include <linux/input.h>	/* Input devices */
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/types.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include "adxl34x.h"	// #define _ADXL34X_H_

#define AC_READ(ac, reg)	((ac)->bops->read((ac)->dev, reg))
#define AC_WRITE(ac, reg, val)	((ac)->bops->write((ac)->dev, reg, val))

static int gpio_nb;
static int irq;

struct axis_triple {
	int x;
	int y;
	int z;
};

/* Create private data structure */
struct adxl345 {
	struct device *dev;
	struct input_dev *input;
	struct adxl345_platform_data pdata;
	struct axis_triple swcal;
	struct axis_triple hwcal;
	struct axis_triple saved;
	char phys[32];
	bool fifo_delay;
	int irq;
	unsigned model;
	unsigned int_mask;
	const struct adxl345_bus_ops *bops;
};

/* Set initial adxl345 register values */
static const struct adxl345_platform_data adxl345_default_init = {
	.tap_threshold = 35,
	.tap_duration = 3,
	//.tap_axis_control = ADXL_TAP_X_EN | ADXL_TAP_Y_EN | ADXL_TAP_Z_EN,
	.tap_axis_control = ADXL_TAP_Z_EN,
	.data_rate = 8,
	.data_range = ADXL_FULL_RES,
	.ev_type = EV_ABS, // select the event type
	.ev_code_x = ABS_X,
	.ev_code_y = ABS_Y,
	.ev_code_z = ABS_Z,
	.ev_code_tap = {BTN_TOUCH, BTN_TOUCH, BTN_TOUCH}, /* EV_KEY {x,y,z} */
	//.fifo_mode = ADXL_FIFO_STREAM,
	.fifo_mode = FIFO_BYPASS,
	.watermark = 0,
};

/* Get the adxl345 axis data */
static void adxl345_get_triple(struct adxl345 *ac, struct axis_triple *axis)
{
	short buf[3];

	ac->bops->read_block(ac->dev, DATAX0, DATAZ1 - DATAX0 + 1, buf);

	ac->saved.x = (s32)buf[0];
	axis->x = ac->saved.x;

	ac->saved.y = (s32)buf[1];
	axis->y = ac->saved.y;

	ac->saved.z = (s32)buf[2];
	axis->z = ac->saved.z;
}

/*
 * this function is called inside the ISR when
 * there is a DATA_READY event
 */
static void adxl345_service_ev_fifo(struct adxl345 *ac)
{
	struct adxl345_platform_data *pdata = &ac->pdata;
	struct axis_triple axis;

	adxl345_get_triple(ac, &axis);

	input_event(ac->input, pdata->ev_type, pdata->ev_code_x,
			axis.x - ac->swcal.x);
	input_event(ac->input, pdata->ev_type, pdata->ev_code_y,
			axis.y - ac->swcal.y);
	input_event(ac->input, pdata->ev_type, pdata->ev_code_z,
			axis.z - ac->swcal.z);
}

/*
 * This function is called inside adxl34x_do_tap() in the ISR
 * when there is a SINGLE_TAP event. The function check
 * the ACT_TAP_STATUS (0x2B) TAP_X, TAP_Y, TAP_Z bits starting
 * for the TAP_X source bit. If the axis is involved in the event
 * there is a EV_KEY event
 */
static void adxl345_send_key_events(struct adxl345 *ac,
		struct adxl345_platform_data *pdata, int status, int press)
{
	int i;

	for (i = ADXL_X_AXIS; i <= ADXL_Z_AXIS; i++) {
		if (status & (1 << (ADXL_Z_AXIS - i)))
			input_report_key(ac->input,
					pdata->ev_code_tap[i], press);
	}
}

/* Function called in the ISR when there is a SINGLE_TAP event */
static void adxl345_do_tap(struct adxl345 *ac,
		struct adxl345_platform_data *pdata, int status)
{
	adxl345_send_key_events(ac, pdata, status, true);
	input_sync(ac->input);
	adxl345_send_key_events(ac, pdata, status, false);
}

/* Interrupt service routine */
static irqreturn_t adxl345_irq(int irq, void *handle)
{
	struct adxl345 *ac = handle;
	struct adxl345_platform_data *pdata = &ac->pdata;
	int int_stat, tap_stat, samples;

	/*
	 * ACT_TAP_STATUS should be read before clearing the interrupt
	 * Avoid reading ACT_TAP_STATUS in case TAP detection is disabled
	 */

	pr_info("interrupt entered \n");

	/* Read the ACT_TAP_STATUS if any of the axis has been enabled */
	if (pdata->tap_axis_control & (TAP_X_EN | TAP_Y_EN | TAP_Z_EN))
		tap_stat = AC_READ(ac, ACT_TAP_STATUS);
	else
		tap_stat = 0;

	/* Read the INT_SOURCE (0x30) register */
	int_stat = AC_READ(ac, INT_SOURCE);

	/*
	 * if the SINGLE_TAP event has occurred the axl345_do_tap function
	 * is called with the ACT_TAP_STATUS register as an argument
	 */
	if (int_stat & (SINGLE_TAP)) {
		adxl345_do_tap(ac, pdata, tap_stat);
	}

	/*
	 * Check if a DATA_READY or WATERMARK event has occurred.
	 * If any of the events have been set an EV_ABS event is generated
	 */
	if (int_stat & (DATA_READY | WATERMARK)) {
		if (pdata->fifo_mode)
			samples = ENTRIES(AC_READ(ac, FIFO_STATUS)) + 1;
		else
			samples = 1;

		for (; samples > 0; samples--) {
			adxl345_service_ev_fifo(ac);
			/*
			 * To ensure that the FIFO has
			 * completely popped, there must be at least 5 us between
			 * the end of reading the data registers, signified by the
			 * transition to register 0x38 from 0x37 or the CS pin
			 * going high, and the start of new reads of the FIFO or
			 * reading the FIFO_STATUS register. For SPI operation at
			 * 1.5 MHz or lower, the register addressing portion of the
			 * transmission is sufficient delay to ensure the FIFO has
			 * completely popped. It is necessary for SPI operation
			 * greater than 1.5 MHz to de-assert the CS pin to ensure a
			 * total of 5 us, which is at most 3.4 us at 5 MHz
			 * operation.
			 */
			if (ac->fifo_delay && (samples > 1))
				udelay(3);
		}
	}

	input_sync(ac->input);

	return IRQ_HANDLED;
}

static void __adxl345_enable(struct adxl345 *ac)
{
	AC_WRITE(ac, POWER_CTL, ac->pdata.power_mode | PCTL_MEASURE);
}

static ssize_t adxl345_calibrate_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct adxl345 *ac = dev_get_drvdata(dev);
	ssize_t count;

	count = sprintf(buf, "%d,%d,%d\n",
			ac->hwcal.x * 4 + ac->swcal.x,
			ac->hwcal.y * 4 + ac->swcal.y,
			ac->hwcal.z * 4 + ac->swcal.z);

	return count;
}

static ssize_t adxl345_calibrate_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct adxl345 *ac = dev_get_drvdata(dev);

	/*
	 * Hardware offset calibration has a resolution of 15.6 mg/LSB.
	 * We use HW calibration and handle the remaining bits in SW. (4mg/LSB)
	 */

	ac->hwcal.x -= (ac->saved.x / 4);
	ac->swcal.x = ac->saved.x % 4;

	ac->hwcal.y -= (ac->saved.y / 4);
	ac->swcal.y = ac->saved.y % 4;

	ac->hwcal.z -= (ac->saved.z / 4);
	ac->swcal.z = ac->saved.z % 4;

	AC_WRITE(ac, OFSX, (s8) ac->hwcal.x);
	AC_WRITE(ac, OFSY, (s8) ac->hwcal.y);
	AC_WRITE(ac, OFSZ, (s8) ac->hwcal.z);

	return count;
}

static DEVICE_ATTR(calibrate, 0664, adxl345_calibrate_show, adxl345_calibrate_store);

static ssize_t adxl345_rate_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct adxl345 *ac = dev_get_drvdata(dev);

	return sprintf(buf, "%u\n", RATE(ac->pdata.data_rate));
}

static ssize_t adxl345_rate_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct adxl345 *ac = dev_get_drvdata(dev);
	unsigned char val;
	int error;

	error = kstrtou8(buf, 10, &val);
	if (error)
		return error;

	ac->pdata.data_rate = RATE(val);
	AC_WRITE(ac, BW_RATE,
			ac->pdata.data_rate |
			(ac->pdata.low_power_mode ? LOW_POWER : 0));

	return count;
}

static DEVICE_ATTR(rate, 0664, adxl345_rate_show, adxl345_rate_store);

static ssize_t adxl345_position_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct adxl345 *ac = dev_get_drvdata(dev);
	ssize_t count;

	count = sprintf(buf, "(%d, %d, %d)\n",
			ac->saved.x, ac->saved.y, ac->saved.z);

	return count;
}

static DEVICE_ATTR(position, S_IRUGO, adxl345_position_show, NULL);

static ssize_t adxl345_position_read(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct axis_triple axis;
	ssize_t count;
	struct adxl345 *ac = dev_get_drvdata(dev);
	adxl345_get_triple(ac, &axis);

	count = sprintf(buf, "(%d, %d, %d)\n",
			axis.x - ac->swcal.x, axis.y - ac->swcal.y,
			axis.z - ac->swcal.z);

	return count;
}
static DEVICE_ATTR(read, S_IRUGO, adxl345_position_read, NULL);

static struct attribute *adxl345_attributes[] = {
	&dev_attr_calibrate.attr,
	&dev_attr_rate.attr,
	&dev_attr_position.attr,
	&dev_attr_read.attr,
	NULL
};

static const struct attribute_group adxl345_attr_group = {
	.attrs = adxl345_attributes,
};


static struct adxl345 *adxl345_probe(struct device *dev, int irq,
		bool fifo_delay_default,
		const struct adxl345_bus_ops *bops)
{
	struct adxl345 *ac;
	struct input_dev *input_dev;
	const struct adxl345_platform_data *pdata;
	int err, range, ret_val;
	unsigned char revid;

	/* Allocate private structure*/
	ac = devm_kzalloc(dev, sizeof(*ac), GFP_KERNEL);

	/* Allocate the input_dev structure */
	input_dev = devm_input_allocate_device(dev);
	if (!ac || !input_dev) {
		dev_err(dev, "Failed to allocate memory\n");
#if 0
		return -ENOMEM;
#else
		return ERR_PTR(-ENOMEM);
#endif
	}

	ac->fifo_delay = fifo_delay_default;

	/*
	 * Check if there is spi device platform data. If not get
	 * it from adxl345_default_init
	 */
	pdata = dev_get_platdata(dev);
	if (!pdata) {
		pr_info("No platform data: Using default initialization\n");
		pdata = &adxl345_default_init;
	}

	ac->pdata = *pdata;
	pdata = &ac->pdata;

	ac->input = input_dev;
	ac->dev = dev;
	ac->irq = irq;
	ac->bops = bops;

	input_dev->name = "ADXL345 accelerometer";
	revid = AC_READ(ac, DEVID);
	pr_info("DEVID: %d\n", revid);

	if (revid == 0xE5) {
		pr_info("ADXL345 is found");
	} else {
		dev_err(dev, "Failed to probe %s\n", input_dev->name);
		err = -ENODEV;
		goto err_out;
	}

	snprintf(ac->phys, sizeof(ac->phys), "%s/input0", dev_name(dev));

	/* Initialize the input device */
	input_dev->phys = ac->phys;
	input_dev->dev.parent = dev;
	input_dev->id.product = ac->model;
	input_dev->id.bustype = bops->bustype;

	/* Attach the input device and the private structure */
	input_set_drvdata(input_dev, ac);

	/* Set the different event types. EV_ABS by default */
	__set_bit(ac->pdata.ev_type, input_dev->evbit);

	if (ac->pdata.ev_type == EV_REL) {
		__set_bit(REL_X, input_dev->relbit);
		__set_bit(REL_Y, input_dev->relbit);
		__set_bit(REL_Z, input_dev->relbit);
	} else {
		/* EV_ABS */
		__set_bit(ABS_X, input_dev->absbit);
		__set_bit(ABS_Y, input_dev->absbit);
		__set_bit(ABS_Z, input_dev->absbit);

		if (pdata->data_range & FULL_RES)
			range = ADXL_FULLRES_MAX_VAL;	/* Signed 13-bit */
		else
			range = ADXL_FIXEDRES_MAX_VAL;	/* Signed 10-bit */

		input_set_abs_params(input_dev, ABS_X, -range, range, 3, 3);
		input_set_abs_params(input_dev, ABS_Y, -range, range, 3, 3);
		input_set_abs_params(input_dev, ABS_Z, -range, range, 3, 3);
	}

	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(pdata->ev_code_tap[ADXL_X_AXIS], input_dev->keybit);
	__set_bit(pdata->ev_code_tap[ADXL_Y_AXIS], input_dev->keybit);
	__set_bit(pdata->ev_code_tap[ADXL_Z_AXIS], input_dev->keybit);


	if (pdata->watermark) {
		ac->int_mask |= WATERMARK;
		if (!FIFO_MODE(pdata->fifo_mode))
			ac->pdata.fifo_mode |= FIFO_STREAM;
	}

	pr_info("%zu\n", pdata->tap_axis_control);
	if (pdata->tap_axis_control & (TAP_X_EN | TAP_Y_EN | TAP_Z_EN))
		ac->int_mask |= SINGLE_TAP;

	if (FIFO_MODE(pdata->fifo_mode) == FIFO_BYPASS)
		ac->fifo_delay = false;

	AC_WRITE(ac, POWER_CTL, 0);

	/* Request threaded interrupt */
	ret_val = devm_request_threaded_irq(input_dev->dev.parent, irq, NULL,
			adxl345_irq, IRQF_TRIGGER_HIGH | IRQF_ONESHOT, dev_name(dev), ac);
	if (ret_val) {
		pr_err("Failed to request interrupt %d, error %d\n", irq, ret_val);
#if 0
		return ret_val;
#else
		return ERR_PTR(ret_val);
#endif
	}

#if 0
	sysfs_create_group(&dev->kobj, &adxl345_attr_group);
#else
	if (sysfs_create_group(&dev->kobj, &adxl345_attr_group))
		pr_err("Failed to sysfs_create_group()\n");
#endif

	/* Register the input device to the input core */
	err = input_register_device(input_dev);
	if (err)
		goto err_remove_attr;

	/* Initialize the ADXL345 registers */
	AC_WRITE(ac, OFSX, pdata->x_axis_offset);
	ac->hwcal.x = pdata->x_axis_offset;
	AC_WRITE(ac, OFSY, pdata->y_axis_offset);
	ac->hwcal.y = pdata->y_axis_offset;
	AC_WRITE(ac, OFSZ, pdata->z_axis_offset);
	ac->hwcal.z = pdata->z_axis_offset;
	AC_WRITE(ac, THRESH_TAP, pdata->tap_threshold);
	AC_WRITE(ac, DUR, pdata->tap_duration);
	AC_WRITE(ac, TAP_AXES, pdata->tap_axis_control);
	AC_WRITE(ac, BW_RATE, RATE(ac->pdata.data_rate) |
			(pdata->low_power_mode ? LOW_POWER : 0));
	/* 13-bit full resolution right justified */
	AC_WRITE(ac, DATA_FORMAT, pdata->data_range);
	AC_WRITE(ac, FIFO_CTL, FIFO_MODE(pdata->fifo_mode) |
			SAMPLES(pdata->watermark));

	if (pdata->use_int2) {
		/* Map all INTs to INT2 pin */
		AC_WRITE(ac, INT_MAP, ac->int_mask | OVERRUN);
	} else {
		/* Map all INTs to INT1 pin */
		AC_WRITE(ac, INT_MAP, 0);
	}

	/* Enables interrupts */
	pr_info(" int_mask %zu\n", ac->int_mask);
	AC_WRITE(ac, INT_ENABLE, ac->int_mask);

	__adxl345_enable(ac);

	return ac;

err_remove_attr:
	sysfs_remove_group(&dev->kobj, &adxl345_attr_group);
err_out:
	return ERR_PTR(err);
}

static int adxl345_spi_read(struct device *dev, unsigned char reg)
{
	struct spi_device *spi = to_spi_device(dev);
	unsigned char cmd;

	cmd = ADXL345_READCMD(reg);

	return spi_w8r8(spi, cmd);
}

static int adxl345_spi_write(struct device *dev,
		unsigned char reg, unsigned char val)
{
	struct spi_device *spi = to_spi_device(dev);
	unsigned char buf[2];

	buf[0] = ADXL345_WRITECMD(reg);
	buf[1] = val;

	return spi_write(spi, buf, sizeof(buf));
}

static int adxl345_spi_read_block(struct device *dev,
		unsigned char reg, int count,
		void *buf)
{
	struct spi_device *spi = to_spi_device(dev);
	ssize_t status;

	reg = ADXL345_READMB_CMD(reg);
	status = spi_write_then_read(spi, &reg, 1, buf, count);

	return (status < 0) ? status : 0;
}

static const struct adxl345_bus_ops adxl345_spi_bops = {
	.bustype	= BUS_SPI,
	.write	= adxl345_spi_write,
	.read	= adxl345_spi_read,
	.read_block	= adxl345_spi_read_block,
};

static int adxl345_spi_probe(struct spi_device *spi)
{
	struct adxl345 *ac;

	gpio_nb = of_get_gpio(spi->dev.of_node, 0);
	pr_info("GPIO: %d\n", gpio_nb);
	irq = irq_of_parse_and_map(spi->dev.of_node, 0);
	pr_info("IRQ_parsing_devicetree: %d\n", irq);
	irq = gpio_to_irq(gpio_nb);
	pr_info("IRQ_using_gpio_to_irq: %d\n", irq);

	ac = adxl345_probe(&spi->dev, irq,
			spi->max_speed_hz > MAX_FREQ_NO_FIFODELAY, &adxl345_spi_bops);

	if (IS_ERR(ac))
		return PTR_ERR(ac);

	/* Attach the SPI device to the private structure */
	spi_set_drvdata(spi, ac);

	return 0;
}

static int adxl345_spi_remove(struct spi_device *spi)
{
	struct adxl345 *ac = spi_get_drvdata(spi);
	pr_info("my_remove() function is called.\n");
	sysfs_remove_group(&ac->dev->kobj, &adxl345_attr_group);
	input_unregister_device(ac->input);
	dev_dbg(ac->dev, "unregistered accelerometer\n");
	return 0;
}

static const struct of_device_id adxl345_dt_ids[] = {
	{ .compatible = "arrow,adxl345", },
	{ }
};
MODULE_DEVICE_TABLE(of, adxl345_dt_ids);

static const struct spi_device_id adxl345_id[] = {
	{ .name = "adxl345", },
	{ }
};
MODULE_DEVICE_TABLE(spi, adxl345_id);

static struct spi_driver adxl345_driver = {
	.driver = {
		.name = "adxl345",
		.owner = THIS_MODULE,
		.of_match_table = adxl345_dt_ids,
	},
	.probe   = adxl345_spi_probe,
	.remove  = adxl345_spi_remove,
	.id_table	= adxl345_id,
};

module_spi_driver(adxl345_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("ADXL345 Three-Axis Accelerometer SPI Bus Driver");
