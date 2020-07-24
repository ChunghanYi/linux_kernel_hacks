/* ************ LDDD: chapter-11: kmalloc.c ************ */
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>

static void *ptr;

static int alloc_init(void)
{
	size_t size = 1024; /* allocate 1024 bytes */
	ptr = kmalloc(size,GFP_KERNEL);

	if (!ptr) {
		/* handle error */
		pr_err("memory allocation failed\n");
		return -ENOMEM;
	} else {
		pr_info("Memory allocated successfully\n");
	}

	return 0;
}

static void alloc_exit(void)
{
	kfree(ptr);
	pr_info("Memory freed\n");
}

module_init(alloc_init);
module_exit(alloc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Madieu <john.madieu@gmail.com>");
