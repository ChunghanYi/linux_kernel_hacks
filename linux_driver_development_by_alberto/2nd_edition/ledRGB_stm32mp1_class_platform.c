/* ************ LDD4EP(2): ledRGB_stm32mp1_class_platform.c ************ */
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
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/leds.h>
#include <linux/clk.h> 

/* Declare physical PA addresses offsets */
#define GPIOA_MODER_offset 0x00 /* 0x00 offset */
#define GPIOA_OTYPER_offset 0x04 /* 0x04 offset -> 0: Output push-pull */
#define GPIOA_PUPDR_offset 0x0c  /* 0x0C offset -> 01:Pull-up, 10:Pull down */
#define GPIOA_BSRR_offset 0x18 /* 0x18 offset */

/* Declare physical PD addresses offsets */
#define GPIOD_MODER_offset 0x00 /* 0x00 offset */
#define GPIOD_OTYPER_offset 0x04 /* 0x04 offset -> 0: Output push-pull */
#define GPIOD_PUPDR_offset 0x0c  /* 0x0C offset -> 01:Pull-up, 10:Pull down */
#define GPIOD_BSRR_offset 0x18 /* 0x18 offset */

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

/* Green LD5: PA14 */
#define GPIOA_MODER_BSRR14_SET_Pos (14U)
#define GPIOA_MODER_BSRR14_CLEAR_Pos (30U)
#define GPIOA_MODER_MODER14_Pos (28U)
#define GPIOA_MODER_MODER14_0 (0x1U << GPIOA_MODER_MODER14_Pos)
#define GPIOA_MODER_MODER14_1 (0x2U << GPIOA_MODER_MODER14_Pos)
#define GPIOA_PUPDR_PUPDR14_0 (0x1U << GPIOA_MODER_MODER14_Pos)
#define GPIOA_PUPDR_PUPDR14_1 (0x2U << GPIOA_MODER_MODER14_Pos)

#define GPIOA_OTYPER_OTYPER14_pos (14U)
#define GPIOA_OTYPER_OTYPER14_Msk (0x1U << GPIOA_OTYPER_OTYPER14_pos)

#define GPIOA_PA14_SET_BSRR_Mask (1U << GPIOA_MODER_BSRR14_SET_Pos)
#define GPIOA_PA14_CLEAR_BSRR_Mask (1U << GPIOA_MODER_BSRR14_CLEAR_Pos)

/* Blue LD8: PD11 */
#define GPIOD_MODER_BSRR11_SET_Pos (11U)
#define GPIOD_MODER_BSRR11_CLEAR_Pos (27U)
#define GPIOD_MODER_MODER11_Pos (22U)
#define GPIOD_MODER_MODER11_0 (0x1U << GPIOD_MODER_MODER11_Pos)
#define GPIOD_MODER_MODER11_1 (0x2U << GPIOD_MODER_MODER11_Pos)
#define GPIOD_PUPDR_PUPDR11_0 (0x1U << GPIOD_MODER_MODER11_Pos)
#define GPIOD_PUPDR_PUPDR11_1 (0x2U << GPIOD_MODER_MODER11_Pos)

#define GPIOD_OTYPER_OTYPER11_pos (11U)
#define GPIOD_OTYPER_OTYPER11_Msk (0x1U << GPIOD_OTYPER_OTYPER11_pos)

#define GPIOD_PD11_SET_BSRR_Mask (1U << GPIOD_MODER_BSRR11_SET_Pos)
#define GPIOD_PD11_CLEAR_BSRR_Mask (1U << GPIOD_MODER_BSRR11_CLEAR_Pos)

struct led_device {
	u32 led_mask_set; 
	u32 led_mask_clear;
	const char *led_name;
	void __iomem *base_gpioa;
	void __iomem *base_gpiod;
	struct clk *clk_gpioa;
	struct clk *clk_gpiod;
	struct led_classdev cdev;
};

static void led_control(struct led_classdev *led_cdev, enum led_brightness b)
{
	struct led_device *led = container_of(led_cdev, struct led_device, cdev);

	/* Enable the clocks to configure the GPIO registers */
	clk_enable(led->clk_gpioa);
	clk_enable(led->clk_gpiod);

	if (b != LED_OFF) {	/* LED ON */
		if (!strcmp(led->led_name, "ledred"))
			writel_relaxed(led->led_mask_set, led->base_gpioa + GPIOA_BSRR_offset);
		else if (!strcmp(led->led_name, "ledgreen"))
			writel_relaxed(led->led_mask_set, led->base_gpioa + GPIOA_BSRR_offset);
		else if (!strcmp(led->led_name, "ledblue"))
			writel_relaxed(led->led_mask_set, led->base_gpiod + GPIOD_BSRR_offset);
		else {
			pr_info("Bad value\n");
			return;
		}
	} else {
		if (!strcmp(led->led_name, "ledred"))
			writel_relaxed(led->led_mask_clear, led->base_gpioa + GPIOA_BSRR_offset);
		else if (!strcmp(led->led_name, "ledgreen"))
			writel_relaxed(led->led_mask_clear, led->base_gpioa + GPIOA_BSRR_offset);
		else if (!strcmp(led->led_name, "ledblue"))
			writel_relaxed(led->led_mask_clear, led->base_gpiod + GPIOD_BSRR_offset);
		else {
			pr_info("Bad value\n");
			return;
		}
	}

	/* Disable the clocks to configure the GPIO registers */
	clk_disable(led->clk_gpioa);
	clk_disable(led->clk_gpiod);
}

static int ledclass_probe(struct platform_device *pdev)
{
	void __iomem *g_ioremap_addr_gpioa;
	void __iomem *g_ioremap_addr_gpiod;
	struct device_node *child;
	struct resource *r_gpioa;
	struct resource *r_gpiod;
	struct clk *clk_gpioa;
	struct clk *clk_gpiod;
	u32 GPIOA_MODER_write, GPIOD_MODER_write;
	u32 GPIOA_OTYPER_write, GPIOD_OTYPER_write;
	u32 GPIOA_PUPDR_write, GPIOD_PUPDR_write;
	int ret_val, count;

	struct device *dev = &pdev->dev;

	dev_info(dev, "platform_probe enter\n");

	/* Get the clocks from the device tree and store them in the global structure */
	clk_gpioa = devm_clk_get(&pdev->dev, "GPIOA");
	if (IS_ERR(clk_gpioa)) {
		dev_err(&pdev->dev, "failed to get clk (%ld)\n", PTR_ERR(clk_gpioa));
		return PTR_ERR(clk_gpioa);
	}

	clk_gpiod = devm_clk_get(&pdev->dev, "GPIOD");
	if (IS_ERR(clk_gpiod)) {
		dev_err(&pdev->dev, "failed to get clk (%ld)\n", PTR_ERR(clk_gpiod));
		return PTR_ERR(clk_gpiod);
	}

	ret_val = clk_prepare(clk_gpioa);
	if (ret_val) {
		dev_err(&pdev->dev, "failed to prepare clk (%d)\n", ret_val);
		return ret_val;
	}

	ret_val = clk_prepare(clk_gpiod);
	if (ret_val) {
		dev_err(&pdev->dev, "failed to prepare clk (%d)\n", ret_val);
		return ret_val;
	}

	/* get our first memory resource from device tree */
	r_gpioa = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_gpioa) {
		dev_err(dev, "IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}
	dev_info(dev, "r->start = 0x%08lx\n", (long unsigned int)r_gpioa->start);
	dev_info(dev, "r->end = 0x%08lx\n", (long unsigned int)r_gpioa->end);

	/* get our second memory resource from device tree */
	r_gpiod = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!r_gpiod) {
		dev_err(dev, "IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}
	dev_info(dev, "r->start = 0x%08lx\n", (long unsigned int)r_gpiod->start);
	dev_info(dev, "r->end = 0x%08lx\n", (long unsigned int)r_gpiod->end);

	/* ioremap our memory region */
	g_ioremap_addr_gpioa = devm_ioremap(dev, r_gpioa->start, resource_size(r_gpioa));
	if (!g_ioremap_addr_gpioa) {
		dev_err(dev, "ioremap failed \n");
		return -ENOMEM;
	}

	/* ioremap our memory region */
	g_ioremap_addr_gpiod = devm_ioremap(dev, r_gpiod->start, resource_size(r_gpiod));
	if (!g_ioremap_addr_gpiod) {
		dev_err(dev, "ioremap failed \n");
		return -ENOMEM;
	}

	/* Enable the clocks to configure the GPIO registers */
	clk_enable(clk_gpioa);
	clk_enable(clk_gpiod);

	/* ensures that all leds are off when GPIOs are configured to GP output */
	writel_relaxed(GPIOA_PA13_SET_BSRR_Mask, (g_ioremap_addr_gpioa + GPIOA_BSRR_offset));
	writel_relaxed(GPIOA_PA14_SET_BSRR_Mask, (g_ioremap_addr_gpioa + GPIOA_BSRR_offset));
	writel_relaxed(GPIOD_PD11_CLEAR_BSRR_Mask, (g_ioremap_addr_gpiod + GPIOD_BSRR_offset));

	/* set PA13 to GP output */
	GPIOA_MODER_write = readl_relaxed(g_ioremap_addr_gpioa + GPIOA_MODER_offset);
	GPIOA_MODER_write |= GPIOA_MODER_MODER13_0; 
	GPIOA_MODER_write &= ~(GPIOA_MODER_MODER13_1);

	writel_relaxed(GPIOA_MODER_write, (g_ioremap_addr_gpioa + GPIOA_MODER_offset));

	/* set PA14 to GP output */
	GPIOA_MODER_write = readl_relaxed(g_ioremap_addr_gpioa + GPIOA_MODER_offset);
	GPIOA_MODER_write |= GPIOA_MODER_MODER14_0; 
	GPIOA_MODER_write &= ~(GPIOA_MODER_MODER14_1);

	writel_relaxed(GPIOA_MODER_write, (g_ioremap_addr_gpioa + GPIOA_MODER_offset));

	/* set PD11 to GP output */
	GPIOD_MODER_write = readl_relaxed(g_ioremap_addr_gpiod + GPIOD_MODER_offset);
	GPIOD_MODER_write |= GPIOD_MODER_MODER11_0; 
	GPIOD_MODER_write &= ~(GPIOD_MODER_MODER11_1);

	writel_relaxed(GPIOD_MODER_write, (g_ioremap_addr_gpiod + GPIOD_MODER_offset));

	/* set PA13 to PP (push-pull) configuration */
	GPIOA_OTYPER_write = readl_relaxed(g_ioremap_addr_gpioa + GPIOA_OTYPER_offset);
	GPIOA_OTYPER_write &= ~(GPIOA_OTYPER_OTYPER13_Msk);

	writel_relaxed(GPIOA_OTYPER_write, (g_ioremap_addr_gpioa + GPIOA_OTYPER_offset));

	/* set PA14 to PP (push-pull) configuration */
	GPIOA_OTYPER_write = readl_relaxed(g_ioremap_addr_gpioa + GPIOA_OTYPER_offset);
	GPIOA_OTYPER_write &= ~(GPIOA_OTYPER_OTYPER14_Msk);

	writel_relaxed(GPIOA_OTYPER_write, (g_ioremap_addr_gpioa + GPIOA_OTYPER_offset));

	/* set PD11 to PP (push-pull) configuration */
	GPIOD_OTYPER_write = readl_relaxed(g_ioremap_addr_gpiod + GPIOD_OTYPER_offset);
	GPIOD_OTYPER_write &= ~(GPIOD_OTYPER_OTYPER11_Msk);

	writel_relaxed(GPIOD_OTYPER_write, (g_ioremap_addr_gpiod + GPIOD_OTYPER_offset));

	/* set PA13 PU */
	GPIOA_PUPDR_write = readl_relaxed(g_ioremap_addr_gpioa + GPIOA_PUPDR_offset);
	GPIOA_PUPDR_write |= GPIOA_PUPDR_PUPDR13_0; 
	GPIOA_PUPDR_write &= ~(GPIOA_PUPDR_PUPDR13_1);

	writel_relaxed(GPIOA_PUPDR_write, (g_ioremap_addr_gpioa + GPIOA_PUPDR_offset));

	/* set PA14 PU */
	GPIOA_PUPDR_write = readl_relaxed(g_ioremap_addr_gpioa + GPIOA_PUPDR_offset);
	GPIOA_PUPDR_write |= GPIOA_PUPDR_PUPDR14_0; 
	GPIOA_PUPDR_write &= ~(GPIOA_PUPDR_PUPDR14_1);

	writel_relaxed(GPIOA_PUPDR_write, (g_ioremap_addr_gpioa + GPIOA_PUPDR_offset));

	/* set PD11 PD */
	GPIOD_PUPDR_write = readl_relaxed(g_ioremap_addr_gpiod + GPIOD_PUPDR_offset);
	GPIOD_PUPDR_write |= GPIOD_PUPDR_PUPDR11_1; 
	GPIOD_PUPDR_write &= ~(GPIOD_PUPDR_PUPDR11_0);	

	writel_relaxed(GPIOD_PUPDR_write, (g_ioremap_addr_gpiod + GPIOD_PUPDR_offset));

	/* Disable the clocks after configuring the registers */
	clk_disable(clk_gpioa);
	clk_disable(clk_gpiod);

	count = of_get_child_count(dev->of_node);
	if (!count)
		return -EINVAL;

	dev_info(dev, "there are %d nodes\n", count);

	for_each_child_of_node(dev->of_node, child) {
		struct led_device *led_device;
		struct led_classdev *cdev;
		led_device = devm_kzalloc(dev, sizeof(*led_device), GFP_KERNEL);
		if (!led_device)
			return -ENOMEM;

		cdev = &led_device->cdev;

		led_device->base_gpioa = g_ioremap_addr_gpioa;
		led_device->base_gpiod = g_ioremap_addr_gpiod;

		of_property_read_string(child, "label", &cdev->name);

		if (strcmp(cdev->name,"ledred") == 0) {
			led_device->led_mask_set = GPIOA_PA13_CLEAR_BSRR_Mask;
			led_device->led_mask_clear = GPIOA_PA13_SET_BSRR_Mask;
			led_device->led_name ="ledred";
			led_device->cdev.default_trigger = "heartbeat";
			led_device->clk_gpioa = clk_gpioa;
			led_device->clk_gpiod = clk_gpiod;
		} else if (strcmp(cdev->name,"ledgreen") == 0) {
			led_device->led_mask_set = GPIOA_PA14_CLEAR_BSRR_Mask;
			led_device->led_mask_clear = GPIOA_PA14_SET_BSRR_Mask;
			led_device->led_name ="ledgreen";
			led_device->clk_gpioa = clk_gpioa;
			led_device->clk_gpiod = clk_gpiod;
		} else if (strcmp(cdev->name,"ledblue") == 0) {
			led_device->led_mask_set = GPIOD_PD11_SET_BSRR_Mask;
			led_device->led_mask_clear = GPIOD_PD11_CLEAR_BSRR_Mask;
			led_device->led_name ="ledblue";
			led_device->clk_gpioa = clk_gpioa;
			led_device->clk_gpiod = clk_gpiod;
		} else {
			dev_info(dev, "Bad device tree value\n");
			return -EINVAL;
		}

		/* Disable timer trigger until led is on */
		led_device->cdev.brightness = LED_OFF;
		led_device->cdev.brightness_set = led_control;

		ret_val = devm_led_classdev_register(dev, &led_device->cdev);
		if (ret_val) {
			dev_err(dev, "failed to register the led %s\n", cdev->name);
			of_node_put(child);
			return ret_val;
		}
	}

	dev_info(dev, "platform_probe exit\n");

	return 0;
}

static int ledclass_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "platform_remove enter\n");
	dev_info(&pdev->dev, "platform_remove exit\n");

	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,RGBclassleds"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
	.probe = ledclass_probe,
	.remove = ledclass_remove,
	.driver = {
		.name = "RGBclassleds",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

/* Register our platform driver */
module_platform_driver(led_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a driver that turns on/off RGB leds \
		using the LED subsystem");
