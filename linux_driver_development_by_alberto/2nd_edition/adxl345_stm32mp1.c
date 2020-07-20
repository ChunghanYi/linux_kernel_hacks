/* ************ LDD4EP(2): adxl345_stm32mp1.c ************ */
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

#include <linux/input.h>	
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/of_gpio.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>

#define ADXL345_CMD_MULTB	(1 << 6)
#define ADXL345_CMD_READ	(1 << 7)
#define ADXL345_WRITECMD(reg)	(reg & 0x3F)
#define ADXL345_READCMD(reg)	(ADXL345_CMD_READ | (reg & 0x3F))
#define ADXL345_READMB_CMD(reg) (ADXL345_CMD_READ | ADXL345_CMD_MULTB \
		| (reg & 0x3F))

/* ADXL345 Register Map */
#define DEVID		0x00	/* R   Device ID */
#define THRESH_TAP	0x1D	/* R/W Tap threshold */
#define DUR		0x21	/* R/W Tap duration */
#define TAP_AXES	0x2A	/* R/W Axis control for tap/double tap */
#define ACT_TAP_STATUS	0x2B	/* R   Source of tap/double tap */
#define BW_RATE		0x2C	/* R/W Data rate and power mode control */
#define POWER_CTL	0x2D	/* R/W Power saving features control */
#define INT_ENABLE	0x2E	/* R/W Interrupt enable control */
#define INT_MAP		0x2F	/* R/W Interrupt mapping control */
#define INT_SOURCE	0x30	/* R   Source of interrupts */
#define DATA_FORMAT	0x31	/* R/W Data format control */
#define DATAX0		0x32	/* R   X-Axis Data 0 */
#define DATAX1		0x33	/* R   X-Axis Data 1 */
#define DATAY0		0x34	/* R   Y-Axis Data 0 */
#define DATAY1		0x35	/* R   Y-Axis Data 1 */
#define DATAZ0		0x36	/* R   Z-Axis Data 0 */
#define DATAZ1		0x37	/* R   Z-Axis Data 1 */
#define FIFO_CTL	0x38	/* R/W FIFO control */

/* DEVIDs */
#define ID_ADXL345	0xE5

/* INT_ENABLE/INT_MAP/INT_SOURCE Bits */
#define SINGLE_TAP	(1 << 6)

/* TAP_AXES Bits */
#define TAP_X_EN	(1 << 2)
#define TAP_Y_EN	(1 << 1)
#define TAP_Z_EN	(1 << 0)

/* BW_RATE Bits */
#define LOW_POWER	(1 << 4)
#define RATE(x)		((x) & 0xF)

/* POWER_CTL Bits */
#define PCTL_MEASURE	(1 << 3)
#define PCTL_STANDBY	0X00

/* DATA_FORMAT Bits */
#define FULL_RES	(1 << 3)

/* FIFO_CTL Bits */
#define FIFO_MODE(x)	(((x) & 0x3) << 6)
#define FIFO_BYPASS	0
#define FIFO_FIFO	1
#define FIFO_STREAM	2
#define SAMPLES(x)	((x) & 0x1F)

/* FIFO_STATUS Bits */
#define ADXL_X_AXIS			0
#define ADXL_Y_AXIS			1
#define ADXL_Z_AXIS			2

#define ADXL345_GPIO_NAME		"int"

/* Macros to do SPI operations */
#define AC_READ(ac, reg)	((ac)->bops->read((ac)->dev, reg))
#define AC_WRITE(ac, reg, val)	((ac)->bops->write((ac)->dev, reg, val))

struct adxl345_bus_ops {
	u16 bustype;
	int (*read)(struct device *, unsigned char);
	int (*read_block)(struct device *, unsigned char, int, void *);
	int (*write)(struct device *, unsigned char, unsigned char);
};

struct axis_triple {
	int x;
	int y;
	int z;
};

struct adxl345_platform_data {
	/*
	 * low_power_mode:
	 * A '0' = Normal operation and a '1' = Reduced
	 * power operation with somewhat higher noise.
	 */

	u8 low_power_mode;

	/*
	 * tap_threshold:
	 * holds the threshold value for tap detection/interrupts.
	 * The data format is unsigned. The scale factor is 62.5 mg/LSB
	 * (i.e. 0xFF = +16 g). A zero value may result in undesirable
	 * behavior if Tap/Double Tap is enabled.
	 */

	u8 tap_threshold;

	/*
	 * tap_duration:
	 * is an unsigned time value representing the maximum
	 * time that an event must be above the tap_threshold threshold
	 * to qualify as a tap event. The scale factor is 625 us/LSB. A zero
	 * value will prevent Tap/Double Tap functions from working.
	 */

	u8 tap_duration;

	/*
	 * TAP_X/Y/Z Enable: Setting TAP_X, Y, or Z Enable enables X,
	 * Y, or Z participation in Tap detection. A '0' excludes the
	 * selected axis from participation in Tap detection.
	 * Setting the SUPPRESS bit suppresses Double Tap detection if
	 * acceleration greater than tap_threshold is present during the
	 * tap_latency period, i.e. after the first tap but before the
	 * opening of the second tap window.
	 */

#define ADXL_TAP_X_EN	(1 << 2)
#define ADXL_TAP_Y_EN	(1 << 1)
#define ADXL_TAP_Z_EN	(1 << 0)

	u8 tap_axis_control;

	/*
	 * data_rate:
	 * Selects device bandwidth and output data rate.
	 * RATE = 3200 Hz / (2^(15 - x)). Default value is 0x0A, or 100 Hz
	 * Output Data Rate. An Output Data Rate should be selected that
	 * is appropriate for the communication protocol and frequency
	 * selected. Selecting too high of an Output Data Rate with a low
	 * communication speed will result in samples being discarded.
	 */

	u8 data_rate;

	/*
	 * data_range:
	 * FULL_RES: When this bit is set with the device is
	 * in Full-Resolution Mode, where the output resolution increases
	 * with RANGE to maintain a 4 mg/LSB scale factor. When this
	 * bit is cleared the device is in 10-bit Mode and RANGE determine the
	 * maximum g-Range and scale factor.
	 */

#define ADXL_FULL_RES		(1 << 3)
#define ADXL_RANGE_PM_2g	0
#define ADXL_RANGE_PM_4g	1
#define ADXL_RANGE_PM_8g	2
#define ADXL_RANGE_PM_16g	3

	u8 data_range;

	/*
	 * A valid BTN or KEY Code; use tap_axis_control to disable
	 * event reporting
	 */

	u32 ev_code_tap[3];	/* EV_KEY {X-Axis, Y-Axis, Z-Axis} */

	/*
	 * fifo_mode:
	 * BYPASS The FIFO is bypassed
	 * FIFO   FIFO collects up to 32 values then stops collecting data
	 * STREAM FIFO holds the last 32 data values. Once full, the FIFO's
	 *        oldest data is lost as it is replaced with newer data
	 *
	 * DEFAULT should be FIFO_STREAM
	 */

	u8 fifo_mode;

	/*
	 * watermark:
	 * The Watermark feature can be used to reduce the interrupt load
	 * of the system. The FIFO fills up to the value stored in watermark
	 * [1..32] and then generates an interrupt.
	 * A '0' disables the watermark feature.
	 */

	u8 watermark;
};

/* Set initial adxl345 register values */
static const struct adxl345_platform_data adxl345_default_init = {
	.tap_threshold = 50,
	.tap_duration = 3,
	//.tap_axis_control = ADXL_TAP_X_EN | ADXL_TAP_Y_EN | ADXL_TAP_Z_EN,
	.tap_axis_control = ADXL_TAP_Z_EN,
	.data_rate = 8,
	.data_range = ADXL_FULL_RES,
	.ev_code_tap = {BTN_TOUCH, BTN_TOUCH, BTN_TOUCH}, /* EV_KEY {x,y,z} */
	//.fifo_mode = ADXL_FIFO_STREAM,
	.fifo_mode = FIFO_BYPASS,
	.watermark = 0,
};

/* Create private data structure */
struct adxl345 {
	struct gpio_desc *gpio;
	struct device *dev;
	struct input_dev *input;
	struct adxl345_platform_data pdata;
	struct axis_triple saved;
	u8 phys[32];
	int irq;
	u32 model;
	u32 int_mask;
	const struct adxl345_bus_ops *bops;
};

/* Get the adxl345 axis data */
static void adxl345_get_triple(struct adxl345 *ac, struct axis_triple *axis)
{
	__le16 buf[3];

	ac->bops->read_block(ac->dev, DATAX0, DATAZ1 - DATAX0 + 1, buf);

	ac->saved.x = sign_extend32(le16_to_cpu(buf[0]), 12);
	axis->x = ac->saved.x;

	ac->saved.y = sign_extend32(le16_to_cpu(buf[1]), 12);
	axis->y = ac->saved.y;

	ac->saved.z = sign_extend32(le16_to_cpu(buf[2]), 12);
	axis->z = ac->saved.z;
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
			input_report_key(ac->input, pdata->ev_code_tap[i], press);
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
	int int_stat, tap_stat;

	/*
	 * ACT_TAP_STATUS should be read before clearing the interrupt
	 * Avoid reading ACT_TAP_STATUS in case TAP detection is disabled
	 * Read the ACT_TAP_STATUS if any of the axis has been enabled
	 */
	if (pdata->tap_axis_control & (TAP_X_EN | TAP_Y_EN | TAP_Z_EN))
		tap_stat = AC_READ(ac, ACT_TAP_STATUS);
	else
		tap_stat = 0;

	/* Read the INT_SOURCE (0x30) register. The interrupt is cleared */
	int_stat = AC_READ(ac, INT_SOURCE);

	/*
	 * if the SINGLE_TAP event has occurred the axl345_do_tap function
	 * is called with the ACT_TAP_STATUS register as an argument
	 */
	if (int_stat & (SINGLE_TAP)) {
		dev_info(ac->dev, "single tap interrupt has occurred\n");
		adxl345_do_tap(ac, pdata, tap_stat);
	}

	input_sync(ac->input);

	return IRQ_HANDLED;
}

static ssize_t adxl345_rate_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct adxl345 *ac = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", RATE(ac->pdata.data_rate));
}

static ssize_t adxl345_rate_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct adxl345 *ac = dev_get_drvdata(dev);
	u8 val;
	int error;

	/* transform char array to u8 value */
	error = kstrtou8(buf, 10, &val);
	if (error)
		return error;

	/* 
	 * if I set ac->pdata.low_power_mode = 1
	 * then lower power mode but higher noise is selected
	 * getting LOW_POWER macro, by default ac->pdata.low_power_mode = 0
	 * RATE(val)sets to 0 the 4 upper u8 bits
	 */
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

	count = sprintf(buf, "(%d, %d, %d)\n", axis.x, axis.y, axis.z);

	return count;
}
static DEVICE_ATTR(read, S_IRUGO, adxl345_position_read, NULL);

static struct attribute *adxl345_attributes[] = {
	&dev_attr_rate.attr,
	&dev_attr_position.attr,
	&dev_attr_read.attr,
	NULL
};

static const struct attribute_group adxl345_attr_group = {
	.attrs = adxl345_attributes,
};

struct adxl345 *adxl345_probe(struct device *dev,
		const struct adxl345_bus_ops *bops)
{
	struct adxl345 *ac; /* declare our private structure */
	struct input_dev *input_dev;
	const struct adxl345_platform_data *pdata;
	int err;
	u8 revid;

	/* Allocate private structure*/
	ac = devm_kzalloc(dev, sizeof(*ac), GFP_KERNEL);
	if (!ac) {
		dev_err(dev, "Failed to allocate memory\n");
		err = -ENOMEM;
		goto err_out;
	}

	/* Allocate the input_dev structure */
	input_dev = devm_input_allocate_device(dev);
	if (!ac || !input_dev) {
		dev_err(dev, "failed to allocate input device\n");
		err = -ENOMEM;
		goto err_out;
	}

	/* Initialize our private structure */

	/*
	 * Store the previously initialized platform data
	 * in our private structure
	 */
	pdata = &adxl345_default_init;
	ac->pdata = *pdata;
	pdata = &ac->pdata;

	ac->input = input_dev;
	ac->dev = dev;

	/* Store the SPI operations in our private structure */
	ac->bops = bops;

	input_dev->name = "ADXL345 accelerometer";
	revid = AC_READ(ac, DEVID);
	dev_info(dev, "DEVID: %d\n", revid);

	if (revid == 0xE5) {
		dev_info(dev, "ADXL345 is found");
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

	/* 
	 * Set the different event types.
	 * EV_KEY type events, with BTN_TOUCH events code
	 * when the single tap interrupt is triggered
	 */
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(pdata->ev_code_tap[ADXL_X_AXIS], input_dev->keybit);
	__set_bit(pdata->ev_code_tap[ADXL_Y_AXIS], input_dev->keybit);
	__set_bit(pdata->ev_code_tap[ADXL_Z_AXIS], input_dev->keybit);

	/* 
	 * Check if any of the axis has been enabled
	 * and set the interrupt mask
	 * In this driver only SINGLE_TAP interrupt
	 */
	if (pdata->tap_axis_control & (TAP_X_EN | TAP_Y_EN | TAP_Z_EN))
		ac->int_mask |= SINGLE_TAP;

	ac->gpio = devm_gpiod_get_index(dev, ADXL345_GPIO_NAME, 0, GPIOD_IN);
	if (IS_ERR(ac->gpio)) {
		dev_err(dev, "gpio get index failed\n");
		err = PTR_ERR(ac->gpio); /* PTR_ERR return an int from a pointer */
		goto err_out;
	}

	ac->irq = gpiod_to_irq(ac->gpio);
	if (ac->irq < 0) {
		dev_err(dev, "gpio get irq failed\n");
		err = ac->irq;
		goto err_out;
	}
	dev_info(dev, "The IRQ number is: %d\n", ac->irq);

	/* Request threaded interrupt */
	/*err = devm_request_threaded_irq(input_dev->dev.parent, ac->irq, NULL,
	  adxl345_irq, IRQF_TRIGGER_HIGH | IRQF_ONESHOT, dev_name(dev), ac);
	  if (err)
	  goto err_out;*/

	err = devm_request_threaded_irq(input_dev->dev.parent, ac->irq, NULL,
			adxl345_irq, IRQF_ONESHOT | IRQF_TRIGGER_HIGH, dev_name(dev), ac);
	if (err)
		goto err_out;

	err = sysfs_create_group(&dev->kobj, &adxl345_attr_group);
	if (err)
		goto err_out;

	/* Register the input device to the input core */
	err = input_register_device(input_dev);
	if (err)
		goto err_remove_attr;

	/* Initialize the ADXL345 registers */

	/* Set the tap threshold and duration */
	AC_WRITE(ac, THRESH_TAP, pdata->tap_threshold);
	AC_WRITE(ac, DUR, pdata->tap_duration);

	/* set the axis where the tap will be detected */
	AC_WRITE(ac, TAP_AXES, pdata->tap_axis_control);

	/* set the data rate and the axis reading power
	 * mode, less or higher noise reducing power
	 */
	AC_WRITE(ac, BW_RATE, RATE(ac->pdata.data_rate) |
			(pdata->low_power_mode ? LOW_POWER : 0));

	/* 13-bit full resolution right justified */
	AC_WRITE(ac, DATA_FORMAT, pdata->data_range);

	/* Set the FIFO mode, no FIFO by default */
	AC_WRITE(ac, FIFO_CTL, FIFO_MODE(pdata->fifo_mode) |
			SAMPLES(pdata->watermark));

	/* Map all INTs to INT1 pin */
	AC_WRITE(ac, INT_MAP, 0);

	/* Enables interrupts */
	AC_WRITE(ac, INT_ENABLE, ac->int_mask);

	/* Set RUN mode */
	AC_WRITE(ac, POWER_CTL, PCTL_MEASURE);

	return ac;

err_remove_attr:
	sysfs_remove_group(&dev->kobj, &adxl345_attr_group);

	/* 
	 * this function returns a pointer
	 * to a struct ac or an err pointer
	 */
err_out:
	return ERR_PTR(err);
}

/* 
 * Write the address of the register
 * and read the value of it
 */
static int adxl345_spi_read(struct device *dev, unsigned char reg)
{
	struct spi_device *spi = to_spi_device(dev);
	u8 cmd;

	cmd = ADXL345_READCMD(reg);

	return spi_w8r8(spi, cmd);
}

/* 
 * Write 2 bytes, the address
 * of the register and the value
 */
static int adxl345_spi_write(struct device *dev,
		unsigned char reg, unsigned char val)
{
	struct spi_device *spi = to_spi_device(dev);
	u8 buf[2];

	buf[0] = ADXL345_WRITECMD(reg);
	buf[1] = val;

	return spi_write(spi, buf, sizeof(buf));
}

/* Read multiple registers */
static int adxl345_spi_read_block(struct device *dev,
		unsigned char reg, int count, void *buf)
{
	struct spi_device *spi = to_spi_device(dev);
	ssize_t status;

	/* Add MB flags to the reading */
	reg = ADXL345_READMB_CMD(reg);

	/* 
     * write byte stored in reg (address with MB)
	 * read count bytes (from successive addresses)
	 * and stores them to buf
	 */
	status = spi_write_then_read(spi, &reg, 1, buf, count);

	return (status < 0) ? status : 0;
}

static const struct adxl345_bus_ops adxl345_spi_bops = {
	.bustype	= BUS_SPI,
	.write		= adxl345_spi_write,
	.read		= adxl345_spi_read,
	.read_block	= adxl345_spi_read_block,
};

static int adxl345_spi_probe(struct spi_device *spi)
{
	struct adxl345 *ac;

	pr_info("accel_probe enter\n");

	/* send the spi operations */
	ac = adxl345_probe(&spi->dev, &adxl345_spi_bops);

	if (IS_ERR(ac))
		return PTR_ERR(ac);

	/* Attach the SPI device to the private structure */
	spi_set_drvdata(spi, ac);

	return 0;
}

static int adxl345_spi_remove(struct spi_device *spi)
{
	struct adxl345 *ac = spi_get_drvdata(spi);
	dev_info(ac->dev, "my_remove() function is called.\n");

	sysfs_remove_group(&ac->dev->kobj, &adxl345_attr_group);
	input_unregister_device(ac->input);
	AC_WRITE(ac, POWER_CTL, PCTL_STANDBY);

	dev_info(ac->dev, "unregistered accelerometer\n");
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
	.probe = adxl345_spi_probe,
	.remove = adxl345_spi_remove,
	.id_table = adxl345_id,
};

module_spi_driver(adxl345_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("ADXL345 Three-Axis Accelerometer SPI Bus Driver");
