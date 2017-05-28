/* ************ LDD4EP: listing5-5: led_UIO_platform.c ************ */
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
	<kernel menuconfig>
	Device Drivers -->
		<*> Userspace I/O drivers  --->
			<*>   Userspace I/O platform driver with generic IRQ handling
			<*>   Userspace platform driver with generic irq and dynamic memory
*/
/*
	//<device tree>
	UIO {
		compatible = "arrow,UIO";
		reg = <0x020A4000 0x4000>;
		pinctrl-0 = <&pinctrl_led>;
	};

	pinctrl_led: ledgrp {
		fsl,pins = <
				MX6QDL_PAD_EIM_A25__GPIO5_IO02      0x1b0b1 // user led0
				MX6QDL_PAD_EIM_D28__GPIO3_IO28      0x1b0b1 // user led1
		>;
	};
*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/uio_driver.h>


static void __iomem *g_ioremap_addr;

static struct uio_info the_uio_info;

static int my_probe(struct platform_device *pdev)
{
	int ret_val;
	struct resource *r;
	struct device *dev = &pdev->dev;

	pr_info("platform_probe enter\n");

	/* get our first memory resource from device tree */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r) {
		pr_err("IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}
	pr_info("r->start = 0x%08lx\n", (unsigned long)r->start);
	pr_info("r->end = 0x%08lx\n", (unsigned long)r->end);

	/* ioremap our memory region and get virtual address */
	g_ioremap_addr = devm_ioremap(dev, r->start, resource_size(r));
	if (!g_ioremap_addr) {
		pr_err("ioremap failed \n");
		return -ENOMEM;
	}

	/* initialize uio_info struct uio_mem array */
	the_uio_info.name = "led_uio";
	the_uio_info.version = "1.0";
	the_uio_info.mem[0].memtype = UIO_MEM_PHYS;
	the_uio_info.mem[0].addr = r->start; /* physical address needed for the kernel user mapping */
	the_uio_info.mem[0].size = resource_size(r);
	the_uio_info.mem[0].name = "demo_uio_driver_hw_region";
	the_uio_info.mem[0].internal_addr = g_ioremap_addr; /* virtual address for internal driver use */

	/* register the uio device */
	ret_val = uio_register_device(&pdev->dev, &the_uio_info);
	if (ret_val != 0)
		pr_warn("Could not register device \"led_uio\"...");
	return 0;
}

static int __exit my_remove(struct platform_device *pdev)
{
	pr_info("platform_remove exit\n");

	uio_unregister_device(&the_uio_info);

	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,UIO"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "UIO",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a UIO platform driver that turns the LED on/off \
	without using system calls");
