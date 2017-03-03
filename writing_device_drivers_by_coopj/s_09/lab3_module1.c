/* **************** LDD:2.0 s_09/lab3_module1.c **************** */
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
 * Dynamic Module Loading  (module 1)
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
 * them, (Installing with the target modules_install and then running
 * depmod -ae will take care of this for you.)
 *
 * Make sure you place your modules in a place where modprobe can find
 * them.  (Installing with the target modules_install will take care
 * of this for you.)
 *
 * You can use either cat or the main program from the character
 * driver lab to exercise your module.  What happens if you try to
 * request loading more than once?
 *
 @*/

#include <linux/module.h>

/* either of these (but not both) will work */

//#include "lab_char.h"
#include "lab_miscdev.h"

void mod_fun(void)
{
	pr_info(" VOILA! I got into mod_fun \n");
}

EXPORT_SYMBOL(mod_fun);
static int mycdrv_open(struct inode *inode, struct file *file)
{
	pr_info("/n rc from requesting module mod_fun is: %d\n",
		request_module("%s", "lab1_module2"));
	return mycdrv_generic_open(inode, file);
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_generic_read,
	.write = mycdrv_generic_write,
	.open = mycdrv_open,
	.release = mycdrv_generic_release,
};

module_init(my_generic_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_09/lab3_module1.c");
MODULE_LICENSE("GPL v2");
