/* **************** LDD:2.0 s_04/lab6_misc_with_header.c **************** */
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
 * Using the misc API (Using a header)
 *
 * Implement your fullest driver from above with the misc API.
 *    
 * Once again a second solution is given which includes the same
 * header file previously used.
 *
@*/

#include <linux/module.h>
#include "lab_miscdev.h"

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_generic_read,
	.write = mycdrv_generic_write,
	.open = mycdrv_generic_open,
	.release = mycdrv_generic_release,
	.llseek = mycdrv_generic_lseek,
};

module_init(my_generic_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_04/lab6_misc_with_header.c");
MODULE_LICENSE("GPL v2");
