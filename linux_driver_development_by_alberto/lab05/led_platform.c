/* ************ LDD4EP: listing5-1: led_platform.c ************ */
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

/*
	//<device tree>
    hellokeys {
        compatible = "arrow,hellokeys";
        pinctrl-0 = <&pinctrl_led>;
    };

	...

	pinctrl_led: ledgrp {
		fsl,pins = <
			MX6QDL_PAD_EIM_A25__GPIO5_IO02      0x1b0b1 // user led0
			MX6QDL_PAD_EIM_D28__GPIO3_IO28      0x1b0b1 // user led1
		>;
	};
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>

/* Include header files for ioremap() and copy_to_user() */
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

/* Declare physical GPDAT, GPDIR and ENET_PAUR addresses */
static unsigned long GPDAT = 0x020A4000;	/* GPIO3_DR - GPIO data register */
static unsigned long GPDIR = 0x020A4004;	/* GPIO3_GDIR - GPIO direction register */
static unsigned long ENET_PAUR = 0x021880E8;	/* MAC address */

/* Declare __iomem pointers that will keep virtual addresses */
static void __iomem *GPDAT_V;
static void __iomem *GPDIR_V;
static void __iomem *ENET_PAUR_V;

#define ENET_ADDRESS_LOWER_LEN	8
#define GPIO_DIR_MASK			1<<28	/* 28th bit enabled <- GPIO3_28*/	
#define GPIO_DATA_MASK			1<<28	/* 28th bit enabled <- GPIO3_28 */	


static ssize_t my_dev_write(struct file *file, const char __user *buff, size_t count, loff_t *ppos)
{
	pr_info("my_dev_write() is called.\n");
	return 0;
}

static ssize_t my_dev_read(struct file *file, char __user *buff, size_t count, loff_t *ppos)
{
	char buf[32];
	size_t size;

	pr_info("my_dev_read() is called.\n");

	/* Read the MAC register and send to user */
	if ((*ppos == 0) && count > ENET_ADDRESS_LOWER_LEN) {
		size = sprintf(buf, "0x%08x", ioread32(ENET_PAUR_V)); 
		buf[size] = '\n';
		if (copy_to_user(buff, buf, size+1)) {
			return -EFAULT;
		}
		*ppos = ENET_ADDRESS_LOWER_LEN;
		return size+1;
	}
	return 0;
}

static int my_dev_open(struct inode *inode, struct file *file)
{
	pr_info("my_dev_open() is called.\n");
	return 0;
}

static int my_dev_close(struct inode *inode, struct file *file)
{
	pr_info("my_dev_close() is called.\n");
	return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	pr_info("my_dev_ioctl() is called. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

static const struct file_operations my_dev_fops = {
	.owner = THIS_MODULE,
	.open = my_dev_open,
	.write = my_dev_write,
	.read = my_dev_read,
	.release = my_dev_close,
	.unlocked_ioctl = my_dev_ioctl,
};

static struct miscdevice helloworld_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mydev",
	.fops = &my_dev_fops,
};

static int my_probe(struct platform_device *pdev)
{
	int retval;
	int i = 0;
	unsigned int temp;

	pr_info("platform_probe enter\n");

	/* Get virtual addresses */
	GPDAT_V = ioremap(GPDAT, sizeof(u32));
	GPDIR_V = ioremap(GPDIR, sizeof(u32));
	ENET_PAUR_V = ioremap(ENET_PAUR, sizeof(u32));

	pr_info("GPDAT_V virtual address: 0x%08lx\n", (unsigned long)GPDAT_V);
	pr_info("GPDIR_V virtual address: 0x%08lx\n", (unsigned long)GPDIR_V);
	pr_info("ENET_PAUR_V virtual address: 0x%08lx\n", (unsigned long)ENET_PAUR_V);

	/* Set GPIO1_IO_2 direction bit to output */
	temp = ioread32(GPDIR_V);
	temp = temp | (GPIO_DIR_MASK);
	iowrite32(temp, GPDIR_V);
	temp = ioread32(GPDIR_V);

	/* Toggle the led */
	for (i=0; i<5; i++) {
		temp = ioread32(GPDAT_V);
		temp = temp & ~(GPIO_DATA_MASK);
		pr_info("LED ON\n");
		iowrite32(temp, GPDAT_V);
		msleep(250);

		temp = ioread32(GPDAT_V);
		temp = temp | (GPIO_DATA_MASK);
		pr_info("LED OFF\n");
		iowrite32(temp, GPDAT_V);
		msleep(250);
	}

	retval = misc_register(&helloworld_miscdevice);
	if (retval)
		return retval; /* misc_register returns 0 if success */
	pr_info("mydev: got minor %i\n",helloworld_miscdevice.minor);
	return 0;
}

static int __exit my_remove(struct platform_device *pdev)
{
	misc_deregister(&helloworld_miscdevice);
	iounmap(GPDAT_V);
	iounmap(GPDIR_V);
	pr_info("platform_remove exit\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,hellokeys"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "hellokeys",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

static int __init demo_init(void)
{
	int ret_val;

	pr_info("demo_init enter\n");

	ret_val = platform_driver_register(&my_platform_driver);
	if (ret_val != 0) {
		pr_err("platform value returned %d\n", ret_val);
		return ret_val;
	}
	pr_info("demo_init exit\n");
	return 0;
}

static void __exit demo_exit(void)
{
	pr_info("demo_exit enter\n");

	platform_driver_unregister(&my_platform_driver);

	pr_info("demo_exit exit\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_AUTHOR("Chunghan Yi <chunghan.yi@gmail.com>");
MODULE_DESCRIPTION("This is a platform driver that turns on/off a led \
	when probing and read MAC address");
