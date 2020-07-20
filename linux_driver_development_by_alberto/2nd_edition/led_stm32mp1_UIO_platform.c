/* ************ LDD4EP(2): led_stm32mp1_UIO_platform.c ************ */
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
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/uio_driver.h>
#include <linux/clk.h>

static struct uio_info the_uio_info;
static struct clk *clk;

/* Declare physical PA addresses offsets */
#define GPIOA_MODER_offset 0x00 /* 0x00 offset */
#define GPIOA_OTYPER_offset 0x04 /* 0x04 offset -> 0: Output push-pull */
#define GPIOA_PUPDR_offset 0x0c  /* 0x0C offset -> 01:Pull-up, 10:Pull down */
#define GPIOA_BSRR_offset 0x18 /* 0x18 offset */

/* Red LD6: PA13 */
#define GPIOA_MODER_BSRR13_SET_Pos (13U)
#define GPIOA_MODER_BSRR13_CLEAR_Pos (29U)
#define GPIOA_MODER_MODER13_Pos (26U)
#define GPIOA_MODER_MODER13_0 (0x1U << GPIOA_MODER_MODER13_Pos)
#define GPIOA_MODER_MODER13_1 (0x2U << GPIOA_MODER_MODER13_Pos)
#define GPIOA_PUPDR_PUPDR13_0 (0x1U << GPIOA_MODER_MODER13_Pos)
#define GPIOA_PUPDR_PUPDR13_1 (0x2U << GPIOA_MODER_MODER13_Pos)

#define GPIOA_OTYPER_OTYPER13_pos (13U)
#define GPIOA_OTYPER_OTYPER13_Msk (0x1U << GPIOA_OTYPER_OTYPER13_pos)

#define GPIOA_PA13_SET_BSRR_Mask (1U << GPIOA_MODER_BSRR13_SET_Pos)
#define GPIOA_PA13_CLEAR_BSRR_Mask (1U << GPIOA_MODER_BSRR13_CLEAR_Pos)

static int my_probe(struct platform_device *pdev)
{
	int ret_val;
	struct resource *r;
	void __iomem *g_ioremap_addr_gpioa;
	u32 GPIOA_MODER_write, GPIOA_OTYPER_write, GPIOA_PUPDR_write; 
	struct device *dev = &pdev->dev;

	dev_info(dev, "platform_probe enter\n");

	clk = devm_clk_get(&pdev->dev, 0);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "failed to get clk (%ld)\n", PTR_ERR(clk));
		return PTR_ERR(clk);
	}

	ret_val = clk_prepare(clk);
	if (ret_val) {
		dev_err(&pdev->dev, "failed to prepare clk (%d)\n", ret_val);
		return ret_val;
	}

	/* get our first memory resource from device tree */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r) {
		dev_err(dev, "IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}
	dev_info(dev, "r->start = 0x%08lx\n", (long unsigned int)r->start);
	dev_info(dev, "r->end = 0x%08lx\n", (long unsigned int)r->end);

	/* ioremap our memory region and get virtual address */
	g_ioremap_addr_gpioa = devm_ioremap(dev, r->start, resource_size(r));
	if (!g_ioremap_addr_gpioa) {
		dev_err(dev, "ioremap failed \n");
		return -ENOMEM;
	}

	/* Enable the clocks to configure the GPIO registers */
	clk_enable(clk);

	/* set PA13 to GP output */
	GPIOA_MODER_write = readl_relaxed(g_ioremap_addr_gpioa + GPIOA_MODER_offset);
	GPIOA_MODER_write |= GPIOA_MODER_MODER13_0; 
	GPIOA_MODER_write &= ~(GPIOA_MODER_MODER13_1);

	writel_relaxed(GPIOA_MODER_write, (g_ioremap_addr_gpioa + GPIOA_MODER_offset));

	/* set PA13 to PP (push-pull) configuration */
	GPIOA_OTYPER_write = readl_relaxed(g_ioremap_addr_gpioa + GPIOA_OTYPER_offset);
	GPIOA_OTYPER_write &= ~(GPIOA_OTYPER_OTYPER13_Msk);

	writel_relaxed(GPIOA_OTYPER_write, (g_ioremap_addr_gpioa + GPIOA_OTYPER_offset));

	/* set PA13 PU */
	GPIOA_PUPDR_write = readl_relaxed(g_ioremap_addr_gpioa + GPIOA_PUPDR_offset);
	GPIOA_PUPDR_write |= GPIOA_PUPDR_PUPDR13_0; 
	GPIOA_PUPDR_write &= ~(GPIOA_PUPDR_PUPDR13_1);

	writel_relaxed(GPIOA_PUPDR_write, (g_ioremap_addr_gpioa + GPIOA_PUPDR_offset));

	/* set ON RED led to test the driver */
	/* writel_relaxed(GPIOA_PA13_CLEAR_BSRR_Mask, (g_ioremap_addr_gpioa + GPIOA_BSRR_offset)); */

	/* initialize uio_info struct uio_mem array */
	the_uio_info.name = "led_uio";
	the_uio_info.version = "1.0";
	the_uio_info.mem[0].memtype = UIO_MEM_PHYS;
	the_uio_info.mem[0].addr = r->start; /* physical address needed for the kernel user mapping */
	the_uio_info.mem[0].size = resource_size(r);
	the_uio_info.mem[0].name = "demo_uio_driver_hw_region";
	the_uio_info.mem[0].internal_addr = g_ioremap_addr_gpioa; /* virtual address for internal driver use */

	/* register the uio device */
	ret_val = uio_register_device(&pdev->dev, &the_uio_info);
	if (ret_val != 0)
		dev_info(dev, "Could not register device \"led_uio\"...");
	return 0;
}

static int my_remove(struct platform_device *pdev)
{
	clk_disable(clk);
	uio_unregister_device(&the_uio_info);
	dev_info(&pdev->dev, "platform_remove exit\n");

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
