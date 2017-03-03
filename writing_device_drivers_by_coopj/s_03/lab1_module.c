/* **************** LDD:2.0 s_03/lab1_module.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2012
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
/*
 * Module parameters
 *
 * Write a module that can take an integer parameter when it is loaded
 * with insmod.  It should have a default value when none is
 * specified.
 *
 * Load it and unload it.  While the module is loaded, look at its
 * directory in /sys/module, and see if you can change the value of
 * the parameter you established.
 @*/

#include <linux/module.h>
#include <linux/init.h>

static int mod_param = 12;
module_param(mod_param, int, S_IRUGO | S_IWUSR);

static int __init my_init(void)
{
	pr_info("Loading module \n");
	pr_info(" mod_param = %d\n", mod_param);
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("Goodbye world from modfun \n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_03/lab1_module.c");
MODULE_LICENSE("GPL v2");
