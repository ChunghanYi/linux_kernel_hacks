/* **************** LDD:2.0 s_18/lab3_mmap.c **************** */
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
/*  Memory Mapping an Allocated Region
 *
 * Write a character driver that implements a mmap() entry point that
 * memory maps a kernel buffer, allocated dynamically (probably during
 * intialization).
 *
 * There should also be read() and write() entry points.
 *
 * Optionally, you may want to use an ioctl() command to tell user
 * space the size of the kernel buffer being memory mapped.  An
 * ioctl() command can be used to return to user-space the size of
 * the kernel buffer being memory mapped.
 *
 @*/

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/io.h>		/* for virt_to_phys() */

/* either of these (but not both) will work */
//#include "lab_char.h"
#include "lab_miscdev.h"

#define MMAP_DEV_CMD_GET_BUFSIZE 1	/* defines our IOCTL cmd */

static int mycdrv_mmap(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long pfn;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long len = vma->vm_end - vma->vm_start;

	if (offset >= ramdisk_size)
		return -EINVAL;
	if (len > (ramdisk_size - offset))
		return -EINVAL;
	pr_info("%s: mapping %ld bytes of ramdisk at offset %ld\n",
		__stringify(KBUILD_BASENAME), len, offset);

	/* need to get the pfn for remap_pfn_range -- either of these two
	   follwoing methods will work */

	/*    pfn = page_to_pfn (virt_to_page (ramdisk + offset)); */
	pfn = virt_to_phys(ramdisk + offset) >> PAGE_SHIFT;

	if (remap_pfn_range(vma, vma->vm_start, pfn, len, vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

/*
 *  mycdrv_unlocked_ioctl() --- give the user the value ramdisk_size
 */
static long
mycdrv_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned long tbs = ramdisk_size;
	void __user *ioargp = (void __user *)arg;

	switch (cmd) {
	default:
		return -EINVAL;

	case MMAP_DEV_CMD_GET_BUFSIZE:
		if (copy_to_user(ioargp, &tbs, sizeof(tbs)))
			return -EFAULT;
		return 0;
	}
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_generic_read,
	.write = mycdrv_generic_write,
	.mmap = mycdrv_mmap,
	.unlocked_ioctl = mycdrv_unlocked_ioctl,
	.llseek = mycdrv_generic_lseek,
};

module_init(my_generic_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Bill Kerr");
MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_18/lab3_mmap.c");
MODULE_LICENSE("GPL v2");
