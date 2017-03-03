/* **************** LDD:2.0 s_12/lab2_sem2.c **************** */
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
/* **************** LDD:2.0 s_12/lab2_sem3.c **************** */
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
 * Semaphore Contention
 *
 * second and third module to test semaphore contention.
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <asm/atomic.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/semaphore.h>

static char *modname = __stringify(KBUILD_BASENAME);

extern struct semaphore my_sem;

static int __init my_init(void)
{
	pr_info("Trying to load module %s\n", modname);
	pr_info("semaphore_count=%u\n", my_sem.count);

#if 0
	/* this branch should hang if not available */
	if (down_trylock(&my_sem)) {
		pr_info("Not loading %s; down_trylock() failed\n", modname);
		return -EBUSY;
	}
#else
	/* this branch will return with failure if not available */
	if (down_interruptible(&my_sem)) {
		pr_info("Not loading %s, interrupted by signal\n", modname);
		return -EBUSY;
	}
#endif

	pr_info("\nGrabbed semaphore in %s, ", modname);
	pr_info("semaphore_count=%u\n", my_sem.count);
	return 0;
}

static void __exit my_exit(void)
{
	up(&my_sem);
	pr_info("\nExiting semaphore in %s, ", modname);
	pr_info("semaphore_count=%u\n", my_sem.count);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_12/lab2_sem3.c");
MODULE_LICENSE("GPL v2");
