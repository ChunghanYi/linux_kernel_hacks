/* **************** LDD:2.0 s_09/lab3_module2.c **************** */
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
 * Dynamic Module Loading  (module 2)
 *
 * Take your basic character driver from the previous exercise and
 * adapt it to use dynamic loading:
 *
 * Construct a trivial second module and have it dynamically loaded
 * during the character driver's open entry point. (Make sure the name
 * of the file that is requested is the same as the name of your
 * file.)
 *
 * Add a small function to your character driver and have it
 * referenced by the second module.
 *
 * Make sure you place your modules in a place where modprobe can find
 * them.  (Installing with the target modules_install will take care
 * of this for you.)
 *
 * You can use either cat or the main program from the character
 * driver lab to exercise your module.  What happens if you try to
 * request loading more than once?
 @*/

#include <linux/module.h>
#include <linux/init.h>

extern void mod_fun(void);

static int __init my_init(void)
{
	pr_info("Hello world from mymod\n");
	mod_fun();
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("Goodbye world from mymod\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_09/lab3_module2.c");
MODULE_LICENSE("GPL v2");
