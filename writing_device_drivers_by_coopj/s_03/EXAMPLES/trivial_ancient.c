/* **************** LDD:2.0 s_03/trivial_ancient.c **************** */
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
#include <linux/module.h>

int __init init_module(void)
{
	pr_info("Hello: init_module loaded at 0x%p\n", init_module);
	return 0;
}

void __exit cleanup_module(void)
{
	pr_info("Bye: cleanup_module loaded at 0x%p\n", cleanup_module);
}
