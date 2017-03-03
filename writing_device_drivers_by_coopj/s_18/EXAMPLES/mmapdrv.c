/* **************** LDD:2.0 s_18/mmapdrv.c **************** */
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
/* Sample Character Driver with mmap'ing */

#include <linux/module.h>
#include <linux/mm.h>

/* either of these (but not both) will work */
//#include "lab_char.h"
#include "lab_miscdev.h"

static int mycdrv_mmap(struct file *file, struct vm_area_struct *vma)
{
	pr_info("I entered the mmap function\n");
	if (remap_pfn_range(vma, vma->vm_start,
			    vma->vm_pgoff,
			    vma->vm_end - vma->vm_start, vma->vm_page_prot)) {
		return -EAGAIN;
	}

	return 0;
}

/* don't bother with open, release, read and write */

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.mmap = mycdrv_mmap,
};

module_init(my_generic_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("Sample Memory Map Driver Entry");
MODULE_LICENSE("GPL v2");
