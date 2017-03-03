/* **************** LDD:2.0 s_12/lab1_mutex1.c **************** */
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
 * Mutex Contention
 *
 *    
 * Write three simple modules where the second and third one use a
 * variable exported from the first one. The second and third one can
 * be identical; just give them different names.
 *     
 * Hint: You can use the macro __stringify(KBUILD_MODNAME) to print
 * out the module name.
 *     
 * You can implement this by making small modifications to your
 * results from the modules exercise.
 *     
 * The exported variable should be a mutex. Have the first module
 * initialize it in the unlocked state.
 *     
 * The second (third) module should attempt to lock the mutex and if
 * it is locked, either fail to load or hang until the mutex is
 * available; make sure you return the appropriate value from your
 * initialization function.
 *    
 * Make sure you release the mutex in your cleanup function.
 *     
 * Test by trying to load both modules simultaneously, and see if it
 * is possible. Make sure you can load one of the modules after the
 * other has been unloaded, to make sure you released the mutex
 * properly.
 @*/

#include <linux/module.h>
#include <linux/init.h>

DEFINE_MUTEX(my_mutex);
EXPORT_SYMBOL(my_mutex);

static int __init my_init(void)
{
	pr_info("\nInit mutex in unlocked state, count=%d:\n",
		atomic_read(&my_mutex.count));
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("\nExiting with  mutex having count=%d:\n",
		atomic_read(&my_mutex.count));
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Tatsuo Kawasaki");
MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_12/lab1_mutex1.c");
MODULE_LICENSE("GPL v2");
