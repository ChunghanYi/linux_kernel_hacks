/*
 * hc_sr04.c
 * Ultrasonic distance sensor driver for UDOO Neo board.
 *
 * ----------------------------------------------------
 * [HC-SR04]                               [UDOO Neo]
 * VCC(5V)      .......................... 5V
 * Trig(output) .......................... GPIO 181
 * Echo(input)  .......................... GPIO 180
 * GND          .......................... GND
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#define	GPIO_HIGH			1
#define	GPIO_LOW			0

#define	ECHO_VALID			1
#define	ECHO_UNVALID		0

#define HC_SR04_ECHO_PIN	180	/* UDOO Neo J4 pin 18 */
#define HC_SR04_TRIG_PIN	181	/* UDOO Neo J4 pin 19 */

static int gpio_irq = -1;
static int echo_valid_flag = 0;
static ktime_t echo_start;
static ktime_t echo_stop;

static ssize_t hc_sr04_value_show(struct class *class, struct class_attribute *attr,
	char *buf)
{  
	int counter;

	gpio_set_value(HC_SR04_TRIG_PIN, GPIO_HIGH);
	udelay(10);
	gpio_set_value(HC_SR04_TRIG_PIN, GPIO_LOW);  
	echo_valid_flag = ECHO_UNVALID;
	counter = 0;
	while (echo_valid_flag == ECHO_UNVALID) {  
		if (++counter > 23200) {  
			return sprintf(buf, "%d\n", -1);
		}
		udelay(1);
	}
	return sprintf(buf, "%lld\n", ktime_to_us(ktime_sub(echo_stop, echo_start)));
}

static ssize_t hc_sr04_value_store(struct class *class, struct class_attribute *attr,
	const char *buf, size_t len)
{  
	pr_info(KERN_INFO "Buffer len %d bytes\n", len);
	return len;
}  

static struct class_attribute hc_sr04_class_attrs[] = {
	__ATTR(value, S_IRUGO | S_IWUSR, hc_sr04_value_show, hc_sr04_value_store),
	__ATTR_NULL,
};

static struct class hc_sr04_class = {
	.name = "hc_sr04",
	.owner = THIS_MODULE,
	.class_attrs = hc_sr04_class_attrs,
};

static irqreturn_t echo_interrupt_handler(int irq, void *data)
{
	ktime_t time_value;  

	if (echo_valid_flag == ECHO_UNVALID) {
		time_value = ktime_get();
		if (gpio_get_value(HC_SR04_ECHO_PIN) == GPIO_HIGH) {
			echo_start = time_value;
		} else {
			echo_stop = time_value;
			echo_valid_flag = ECHO_VALID;
		}
	}
	return IRQ_HANDLED;
}

static int hc_sr04_init(void)
{
	int ret;

	if (class_register(&hc_sr04_class) < 0) {
		pr_info(KERN_ERR "Failed to register a class\n");
		goto error;
	}

	ret = gpio_request_one(HC_SR04_TRIG_PIN, GPIOF_DIR_OUT, "TRIG");
	if (ret < 0) {
		pr_info(KERN_ERR "Failed to request gpio for TRIG(%d)\n", ret);
		return -1;
	}

	ret = gpio_request_one(HC_SR04_ECHO_PIN, GPIOF_DIR_IN, "ECHO");
	if (ret < 0) {
		pr_info(KERN_ERR "Failed to request gpio for ECHO(%d)\n", ret);
		return -1;
	}

	ret = gpio_to_irq(HC_SR04_ECHO_PIN);
	if (ret < 0) {
		pr_info(KERN_ERR "Failed to set gpio IRQ(%d)\n", ret);
		goto error;
	} else {
		gpio_irq = ret;
	}

	ret = request_irq(gpio_irq, echo_interrupt_handler,
					IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "hc-sr04-echo", NULL);
	if (ret) {
		pr_info(KERN_ERR "Failed to request IRQ(%d)\n", ret);
		goto error;
	}

	return 0;
error:
	return -1;
}

static void hc_sr04_exit(void)
{  
	if (gpio_irq != -1)
		free_irq(gpio_irq, NULL);
	gpio_free(HC_SR04_TRIG_PIN);
	gpio_free(HC_SR04_ECHO_PIN);

	class_unregister(&hc_sr04_class);
}

module_init(hc_sr04_init);
module_exit(hc_sr04_exit);

MODULE_AUTHOR("Tushar Panda");
MODULE_AUTHOR("Chunghan Yi");
MODULE_DESCRIPTION("hc-sr04 ultrasonic distance sensor driver");
MODULE_LICENSE("GPL");
