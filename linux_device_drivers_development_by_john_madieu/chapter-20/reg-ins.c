/* ************ LDDD: chapter-20: reg-ins.c ************ */
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

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>

static struct platform_device *pdev;

static int __init fake_reg_add(void)
{
	int inst_id = 0;
	pdev = platform_device_alloc("regulator-dummy", inst_id);
	platform_device_add(pdev);
	pr_info("regulator-dummy added");
	return 0;
}

static void __exit fake_reg_put(void)
{
	pr_info("regulator-dummy removed");
	platform_device_put(pdev);
}

module_init(fake_reg_add);
module_exit(fake_reg_put);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
