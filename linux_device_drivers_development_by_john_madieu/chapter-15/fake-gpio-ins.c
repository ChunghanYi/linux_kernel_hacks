/* ************ LDDD: chapter-15: fake-gpio-ins.c ************ */
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

static int __init platform_dummy_char_add(void)
{
	int inst_id = 0; /* instance unique ID: base address would be a good choice */
	pdev = platform_device_alloc("fake-gpio-chip", inst_id);
	platform_device_add(pdev);
	pr_info("fake-gpio-chip added\n");
	return 0;
}

static void __exit fplatform_dummy_char_put(void)
{
	pr_info("fake-gpio-chip removed\n");
	platform_device_put(pdev);
}

module_init(platform_dummy_char_add);
module_exit(fplatform_dummy_char_put);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
