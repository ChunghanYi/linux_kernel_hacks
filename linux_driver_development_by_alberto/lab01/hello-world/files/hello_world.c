/* ************ LDD4EP: listing1-1: helloworld.c ************ */

#include <linux/module.h>

static int __init hello_init(void)
{
	pr_info("Hello world !\n");
	return 0;
}

static void __exit hello_exit(void)
{
	pr_info("Good bye !\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
