/* ************ LDD4EP: listing3-1: helloworld.c ************ */
/*
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 */

#include <linux/module.h>

static int __init hello_init(void)
{
	pr_info("Hello world init\n");
	return 0;
}

static void __exit hello_exit(void)
{
	pr_info("Hello world exit\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a print out Hello World module");
