/* ************ LDDD: chapter-11: vmalloc.c ************ */
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

#include<linux/init.h>
#include<linux/module.h>
#include <linux/vmalloc.h>

static void *ptr;

static int my_vmalloc_init(void)
{
	unsigned long size = 8192;
	ptr = vmalloc(size);
	if (!ptr) {
		/* handle error */
		pr_err("memory allocation failed\n");
		return -ENOMEM;
	} else {
		pr_info("Memory allocated successfully\n");
	}
	return 0;
}

static void my_vmalloc_exit(void)
{
	vfree(ptr); //free the allocated memory
	pr_info("Memory freed\n");
}

module_init(my_vmalloc_init);
module_exit(my_vmalloc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("john Madieu <john.madieu@gmail.com>");
