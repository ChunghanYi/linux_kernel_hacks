#ifndef __LINUX_INPUT_ADXL34X_H__
#define __LINUX_INPUT_ADXL34X_H__


#define MAX_SPI_FREQ_HZ		5000000
#define MAX_FREQ_NO_FIFODELAY	1500000
#define ADXL345_CMD_MULTB	(1 << 6)
#define ADXL345_CMD_READ	(1 << 7)
#define ADXL345_WRITECMD(reg)	(reg & 0x3F)
#define ADXL345_READCMD(reg)	(ADXL345_CMD_READ | (reg & 0x3F))
#define ADXL345_READMB_CMD(reg) (ADXL345_CMD_READ | ADXL345_CMD_MULTB \
					| (reg & 0x3F))

/* ADXL345 Register Map */
#define DEVID		0x00	/* R   Device ID */
#define THRESH_TAP	0x1D	/* R/W Tap threshold */
#define OFSX		0x1E	/* R/W X-axis offset */
#define OFSY		0x1F	/* R/W Y-axis offset */
#define OFSZ		0x20	/* R/W Z-axis offset */
#define DUR		0x21	/* R/W Tap duration */
#define LATENT		0x22	/* R/W Tap latency */
#define WINDOW		0x23	/* R/W Tap window */
#define THRESH_ACT	0x24	/* R/W Activity threshold */
#define THRESH_INACT	0x25	/* R/W Inactivity threshold */
#define TIME_INACT	0x26	/* R/W Inactivity time */
#define ACT_INACT_CTL	0x27	/* R/W Axis enable control for activity and */
				/* inactivity detection */
#define THRESH_FF	0x28	/* R/W Free-fall threshold */
#define TIME_FF		0x29	/* R/W Free-fall time */
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
#define FIFO_STATUS	0x39	/* R   FIFO status */


/* DEVIDs */
#define ID_ADXL345	0xE5

/* INT_ENABLE/INT_MAP/INT_SOURCE Bits */
#define DATA_READY	(1 << 7)
#define SINGLE_TAP	(1 << 6)
#define DOUBLE_TAP	(1 << 5)
#define ACTIVITY	(1 << 4)
#define INACTIVITY	(1 << 3)
#define FREE_FALL	(1 << 2)
#define WATERMARK	(1 << 1)
#define OVERRUN		(1 << 0)

/* ACT_INACT_CONTROL Bits */
#define ACT_ACDC	(1 << 7)
#define ACT_X_EN	(1 << 6)
#define ACT_Y_EN	(1 << 5)
#define ACT_Z_EN	(1 << 4)
#define INACT_ACDC	(1 << 3)
#define INACT_X_EN	(1 << 2)
#define INACT_Y_EN	(1 << 1)
#define INACT_Z_EN	(1 << 0)

/* TAP_AXES Bits */
#define SUPPRESS	(1 << 3)
#define TAP_X_EN	(1 << 2)
#define TAP_Y_EN	(1 << 1)
#define TAP_Z_EN	(1 << 0)

/* ACT_TAP_STATUS Bits */
#define ACT_X_SRC	(1 << 6)
#define ACT_Y_SRC	(1 << 5)
#define ACT_Z_SRC	(1 << 4)
#define ASLEEP		(1 << 3)
#define TAP_X_SRC	(1 << 2)
#define TAP_Y_SRC	(1 << 1)
#define TAP_Z_SRC	(1 << 0)

/* BW_RATE Bits */
#define LOW_POWER	(1 << 4)
#define RATE(x)		((x) & 0xF)

/* POWER_CTL Bits */
#define PCTL_LINK	(1 << 5)
#define PCTL_AUTO_SLEEP (1 << 4)
#define PCTL_MEASURE	(1 << 3)
#define PCTL_SLEEP	(1 << 2)
#define PCTL_WAKEUP(x)	((x) & 0x3)

/* DATA_FORMAT Bits */
#define SELF_TEST	(1 << 7)
#define SPI		(1 << 6)
#define INT_INVERT	(1 << 5)
#define FULL_RES	(1 << 3)
#define JUSTIFY		(1 << 2)
#define RANGE(x)	((x) & 0x3)
#define RANGE_PM_2g	0
#define RANGE_PM_4g	1
#define RANGE_PM_8g	2
#define RANGE_PM_16g	3

/*
 * Maximum value our axis may get in full res mode for the input device
 * (signed 13 bits)
 */
#define ADXL_FULLRES_MAX_VAL 4096

/*
 * Maximum value our axis may get in fixed res mode for the input device
 * (signed 10 bits)
 */
#define ADXL_FIXEDRES_MAX_VAL 512

/* FIFO_CTL Bits */
#define FIFO_MODE(x)	(((x) & 0x3) << 6)
#define FIFO_BYPASS	0
#define FIFO_FIFO	1
#define FIFO_STREAM	2
#define FIFO_TRIGGER	3
#define TRIGGER		(1 << 5)
#define SAMPLES(x)	((x) & 0x1F)

/* FIFO_STATUS Bits */
#define FIFO_TRIG	(1 << 7)
#define ENTRIES(x)	((x) & 0x3F)

#define ADXL_X_AXIS			0
#define ADXL_Y_AXIS			1
#define ADXL_Z_AXIS			2


struct adxl345_bus_ops {
	u16 bustype;
	int (*read)(struct device *, unsigned char);
	int (*read_block)(struct device *, unsigned char, int, void *);
	int (*write)(struct device *, unsigned char, unsigned char);
};

struct adxl345_platform_data {

	/*
	 * X,Y,Z Axis Offset:
	 * offer user offset adjustments in twoscompliment
	 * form with a scale factor of 15.6 mg/LSB (i.e. 0x7F = +2 g)
	 */

	s8 x_axis_offset;
	s8 y_axis_offset;
	s8 z_axis_offset;

	/*
	 * TAP_X/Y/Z Enable: Setting TAP_X, Y, or Z Enable enables X,
	 * Y, or Z participation in Tap detection. A '0' excludes the
	 * selected axis from participation in Tap detection.
	 * Setting the SUPPRESS bit suppresses Double Tap detection if
	 * acceleration greater than tap_threshold is present during the
	 * tap_latency period, i.e. after the first tap but before the
	 * opening of the second tap window.
	 */

#define ADXL_SUPPRESS	(1 << 3)
#define ADXL_TAP_X_EN	(1 << 2)
#define ADXL_TAP_Y_EN	(1 << 1)
#define ADXL_TAP_Z_EN	(1 << 0)

	u8 tap_axis_control;

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
	 * tap_latency:
	 * is an unsigned time value representing the wait time
	 * from the detection of a tap event to the opening of the time
	 * window tap_window for a possible second tap event. The scale
	 * factor is 1.25 ms/LSB. A zero value will disable the Double Tap
	 * function.
	 */

	u8 tap_latency;

	/*
	 * tap_window:
	 * is an unsigned time value representing the amount
	 * of time after the expiration of tap_latency during which a second
	 * tap can begin. The scale factor is 1.25 ms/LSB. A zero value will
	 * disable the Double Tap function.
	 */

	u8 tap_window;

	/*
	 * act_axis_control:
	 * X/Y/Z Enable: A '1' enables X, Y, or Z participation in activity
	 * or inactivity detection. A '0' excludes the selected axis from
	 * participation. If all of the axes are excluded, the function is
	 * disabled.
	 * AC/DC: A '0' = DC coupled operation and a '1' = AC coupled
	 * operation. In DC coupled operation, the current acceleration is
	 * compared with activity_threshold and inactivity_threshold directly
	 * to determine whether activity or inactivity is detected. In AC
	 * coupled operation for activity detection, the acceleration value
	 * at the start of activity detection is taken as a reference value.
	 * New samples of acceleration are then compared to this
	 * reference value and if the magnitude of the difference exceeds
	 * activity_threshold the device will trigger an activity interrupt. In
	 * AC coupled operation for inactivity detection, a reference value
	 * is used again for comparison and is updated whenever the
	 * device exceeds the inactivity threshold. Once the reference
	 * value is selected, the device compares the magnitude of the
	 * difference between the reference value and the current
	 * acceleration with inactivity_threshold. If the difference is below
	 * inactivity_threshold for a total of inactivity_time, the device is
	 * considered inactive and the inactivity interrupt is triggered.
	 */

#define ADXL_ACT_ACDC		(1 << 7)
#define ADXL_ACT_X_EN		(1 << 6)
#define ADXL_ACT_Y_EN		(1 << 5)
#define ADXL_ACT_Z_EN		(1 << 4)
#define ADXL_INACT_ACDC		(1 << 3)
#define ADXL_INACT_X_EN		(1 << 2)
#define ADXL_INACT_Y_EN		(1 << 1)
#define ADXL_INACT_Z_EN		(1 << 0)

	u8 act_axis_control;

	/*
	 * activity_threshold:
	 * holds the threshold value for activity detection.
	 * The data format is unsigned. The scale factor is
	 * 62.5 mg/LSB. A zero value may result in undesirable behavior if
	 * Activity interrupt is enabled.
	 */

	u8 activity_threshold;

	/*
	 * inactivity_threshold:
	 * holds the threshold value for inactivity
	 * detection. The data format is unsigned. The scale
	 * factor is 62.5 mg/LSB. A zero value may result in undesirable
	 * behavior if Inactivity interrupt is enabled.
	 */

	u8 inactivity_threshold;

	/*
	 * inactivity_time:
	 * is an unsigned time value representing the
	 * amount of time that acceleration must be below the value in
	 * inactivity_threshold for inactivity to be declared. The scale factor
	 * is 1 second/LSB. Unlike the other interrupt functions, which
	 * operate on unfiltered data, the inactivity function operates on the
	 * filtered output data. At least one output sample must be
	 * generated for the inactivity interrupt to be triggered. This will
	 * result in the function appearing un-responsive if the
	 * inactivity_time register is set with a value less than the time
	 * constant of the Output Data Rate. A zero value will result in an
	 * interrupt when the output data is below inactivity_threshold.
	 */

	u8 inactivity_time;

	/*
	 * free_fall_threshold:
	 * holds the threshold value for Free-Fall detection.
	 * The data format is unsigned. The root-sum-square(RSS) value
	 * of all axes is calculated and compared to the value in
	 * free_fall_threshold to determine if a free fall event may be
	 * occurring.  The scale factor is 62.5 mg/LSB. A zero value may
	 * result in undesirable behavior if Free-Fall interrupt is
	 * enabled. Values between 300 and 600 mg (0x05 to 0x09) are
	 * recommended.
	 */

	u8 free_fall_threshold;

	/*
	 * free_fall_time:
	 * is an unsigned time value representing the minimum
	 * time that the RSS value of all axes must be less than
	 * free_fall_threshold to generate a Free-Fall interrupt. The
	 * scale factor is 5 ms/LSB. A zero value may result in
	 * undesirable behavior if Free-Fall interrupt is enabled.
	 * Values between 100 to 350 ms (0x14 to 0x46) are recommended.
	 */

	u8 free_fall_time;

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
	 * low_power_mode:
	 * A '0' = Normal operation and a '1' = Reduced
	 * power operation with somewhat higher noise.
	 */

	u8 low_power_mode;

	/*
	 * power_mode:
	 * LINK: A '1' with both the activity and inactivity functions
	 * enabled will delay the start of the activity function until
	 * inactivity is detected. Once activity is detected, inactivity
	 * detection will begin and prevent the detection of activity. This
	 * bit serially links the activity and inactivity functions. When '0'
	 * the inactivity and activity functions are concurrent. Additional
	 * information can be found in the ADXL34x datasheet's Application
	 * section under Link Mode.
	 * AUTO_SLEEP: A '1' sets the ADXL34x to switch to Sleep Mode
	 * when inactivity (acceleration has been below inactivity_threshold
	 * for at least inactivity_time) is detected and the LINK bit is set.
	 * A '0' disables automatic switching to Sleep Mode. See the
	 * Sleep Bit section of the ADXL34x datasheet for more information.
	 */

#define ADXL_LINK	(1 << 5)
#define ADXL_AUTO_SLEEP	(1 << 4)

	u8 power_mode;

	/*
	 * fifo_mode:
	 * BYPASS The FIFO is bypassed
	 * FIFO   FIFO collects up to 32 values then stops collecting data
	 * STREAM FIFO holds the last 32 data values. Once full, the FIFO's
	 *        oldest data is lost as it is replaced with newer data
	 *
	 * DEFAULT should be ADXL_FIFO_STREAM
	 */

#define ADXL_FIFO_BYPASS	0
#define ADXL_FIFO_FIFO		1
#define ADXL_FIFO_STREAM	2

	u8 fifo_mode;

	/*
	 * watermark:
	 * The Watermark feature can be used to reduce the interrupt load
	 * of the system. The FIFO fills up to the value stored in watermark
	 * [1..32] and then generates an interrupt.
	 * A '0' disables the watermark feature.
	 */

	u8 watermark;

	/*
	 * When acceleration measurements are received from the ADXL34x
	 * events are sent to the event subsystem. The following settings
	 * select the event type and event code for new x, y and z axis data
	 * respectively.
	 */
	u32 ev_type;	/* EV_ABS or EV_REL */

	u32 ev_code_x;	/* ABS_X,Y,Z or REL_X,Y,Z */
	u32 ev_code_y;	/* ABS_X,Y,Z or REL_X,Y,Z */
	u32 ev_code_z;	/* ABS_X,Y,Z or REL_X,Y,Z */

	/*
	 * A valid BTN or KEY Code; use tap_axis_control to disable
	 * event reporting
	 */

	u32 ev_code_tap[3];	/* EV_KEY {X-Axis, Y-Axis, Z-Axis} */

	/*
	 * A valid BTN or KEY Code for Free-Fall or Activity enables
	 * input event reporting. A '0' disables the Free-Fall or
	 * Activity reporting.
	 */

	u32 ev_code_ff;	/* EV_KEY */
	u32 ev_code_act_inactivity;	/* EV_KEY */

	/*
	 * Use ADXL34x INT2 pin instead of INT1 pin for interrupt output
	 */
	u8 use_int2;

};
#endif
