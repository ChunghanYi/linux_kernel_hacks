/* **************** LDD:2.0 s_12/lab2_sem1.c **************** */
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
 * Sempahore Contention
 *
 * Now do the same thing using semaphores instead of mutexes.
 *
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <asm/atomic.h>
#include <linux/version.h>
#include <linux/semaphore.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
DECLARE_MUTEX(my_sem);
#else
DEFINE_SEMAPHORE(my_sem);
#endif
EXPORT_SYMBOL(my_sem);

static int __init my_init(void)
{
	pr_info("\nInitializing semaphore, ");
	pr_info("semaphore_count=%u\n", my_sem.count);
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("\nExiting semaphore, ");
	pr_info("semaphore_count=%u\n", my_sem.count);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_12/lab2_sem1.c");
MODULE_LICENSE("GPL v2");
