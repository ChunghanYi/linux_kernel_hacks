/* **************** LDD:2.0 s_04/lab5_with_header.c **************** */
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
 * Dynamical Node Creation (II)
 *
 * Adapt the previous dynamic registration driver to use udev to
 * create the device node on the fly.
 *
 * A second solution is given which includes a header file
 * (lab_header.h) which will be used to simplify the coding in
 * subsequent character drivers we will write.
 *
 @*/

#include <linux/module.h>
#include "lab_char.h"

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
MODULE_DESCRIPTION("LDD:2.0 s_04/lab5_with_header.c");
MODULE_LICENSE("GPL v2");
