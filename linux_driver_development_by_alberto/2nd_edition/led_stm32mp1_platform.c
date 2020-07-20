/* ************ LDD4EP(2): led_stm32mp1_platform.c ************ */
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
#include <linux/fs.h> /* struct file_operations */
/* platform_driver_register(), platform_set_drvdata() */
#include <linux/platform_device.h>
#include <linux/io.h> /* devm_ioremap(), iowrite32() */
#include <linux/of.h> /* of_property_read_string() */
#include <linux/uaccess.h> /* copy_from_user(), copy_to_user() */
#include <linux/miscdevice.h> /* misc_register() */
#include <linux/clk.h>

//https://elixir.bootlin.com/linux/v4.19.108/source/drivers/pinctrl/stm32/pinctrl-stm32.c#L101

//Red LD6: PA13

//gpioa: gpio@50002000


/* Declare physical PA addresses */
#define GPIOA_MODER 0x50002000 // 0x00 offset -> 01: GP
#define GPIOA_OTYPER 0x50002004 // 0x04 offset -> 0: Output push-pull
#define GPIOA_PUPDR 0x5000200c  // 0x0C offset -> 01:Pull-up, 10:Pull down
#define GPIOA_BSRR 0x50002018 // 0x18 offset

/* Declare __iomem pointers that will keep virtual addresses */
static void __iomem *GPIOA_MODER_V;
static void __iomem *GPIOA_OTYPER_V;
static void __iomem *GPIOA_PUPDR_V;
static void __iomem *GPIOA_BSRR_V;

//Red LD6: PA13, Green LD5: PA14, and Blue LD8: PD11
#define GPIOA_MODER_BSRR_SET_Pos (13U)
#define GPIOA_MODER_BSRR_CLEAR_Pos (29U)
#define GPIOA_MODER_MODER13_Pos (26U)
#define GPIOA_MODER_MODER13_0 (0x1U << GPIOA_MODER_MODER13_Pos)
#define GPIOA_MODER_MODER13_1 (0x2U << GPIOA_MODER_MODER13_Pos)
#define GPIOA_PUPDR_PUPDR13_0 (0x1U << GPIOA_MODER_MODER13_Pos)
#define GPIOA_PUPDR_PUPDR13_1 (0x2U << GPIOA_MODER_MODER13_Pos)
//#define GPIOA_MODER_MODER13_1 (0x3U << GPIOA_MODER_MODER13_Pos)

#define GPIOA_OTYPER_OTYPER13_pos (13U)
#define GPIOA_OTYPER_OTYPER13_Msk (0x1U << GPIOA_OTYPER_OTYPER13_pos)

#define GPIOA_PA13_SET_BSRR_Mask (1U << GPIOA_MODER_BSRR_SET_Pos)
#define GPIOA_PA13_CLEAR_BSRR_Mask (1U << GPIOA_MODER_BSRR_CLEAR_Pos)

static int led_probe(struct platform_device *pdev)
{	
	int err;
	u32 GPIOA_MODER_write;
	u32 GPIOA_OTYPER_write;
	u32 GPIOA_PUPDR_write;
	struct clk *clk;

	pr_info("led_probe enter\n");

	clk = devm_clk_get(&pdev->dev, 0);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "failed to get clk (%ld)\n", PTR_ERR(clk));
		return PTR_ERR(clk);
	}

	err = clk_prepare(clk);
	if (err) {
		dev_err(&pdev->dev, "failed to prepare clk (%d)\n", err);
		return err;
	}

	clk_enable(clk);

	/* Get virtual addresses */
	GPIOA_MODER_V = devm_ioremap(&pdev->dev, GPIOA_MODER, sizeof(u32));
	GPIOA_OTYPER_V = devm_ioremap(&pdev->dev, GPIOA_OTYPER, sizeof(u32));
	GPIOA_PUPDR_V = devm_ioremap(&pdev->dev, GPIOA_PUPDR, sizeof(u32));
	GPIOA_BSRR_V = devm_ioremap(&pdev->dev, GPIOA_BSRR, sizeof(u32));

	/* set PA13 to GP output */
	GPIOA_MODER_write = readl_relaxed(GPIOA_MODER_V);
	GPIOA_MODER_write |= GPIOA_MODER_MODER13_0; 
	GPIOA_MODER_write &= ~(GPIOA_MODER_MODER13_1);

	writel_relaxed(GPIOA_MODER_write, GPIOA_MODER_V);

	/* set PA13 to PP (push-pull) configuration */
	GPIOA_OTYPER_write = ioread32(GPIOA_OTYPER_V);
	GPIOA_OTYPER_write &= ~(GPIOA_OTYPER_OTYPER13_Msk);

	writel_relaxed(GPIOA_OTYPER_write, GPIOA_OTYPER_V);

	/* set PA13 PU */
	GPIOA_PUPDR_write = readl_relaxed(GPIOA_PUPDR_V);
	GPIOA_PUPDR_write |= GPIOA_PUPDR_PUPDR13_0; 
	GPIOA_MODER_write &= ~(GPIOA_PUPDR_PUPDR13_1);

	writel_relaxed(GPIOA_PUPDR_write, GPIOA_PUPDR_V);

	/* set PA13 output to 0 */
	//iowrite32(GPIOA_PA13_CLEAR_BSRR_Mask, GPIOA_BSRR_V); // LED RED ON, output 0.
	writel_relaxed(GPIOA_PA13_CLEAR_BSRR_Mask, GPIOA_BSRR_V); // LED RED ON, output 0.

	clk_disable(clk);

	pr_info("leds_probe exit\n");

	return 0;
}

/* The remove() function is called 3 times once per led */
static int led_remove(struct platform_device *pdev)
{
	pr_info("leds_remove enter\n");
	pr_info("leds_remove exit\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,RGBleds"},
	{},
};
MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
	.probe = led_probe,
	.remove = led_remove,
	.driver = {
		.name = "RGBleds",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

/* Register our platform driver */
module_platform_driver(led_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a platform driver that turns on/off \
		three led devices");
