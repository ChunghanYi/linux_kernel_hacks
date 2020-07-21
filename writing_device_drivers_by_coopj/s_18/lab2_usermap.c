/* **************** LDD:2.0 s_18/lab2_usermap.c **************** */
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
 * Mapping User Pages
 *
 * Use the character device driver, adapt it to use get_user_pages()
 * for the read() and write() entry points.
 *
 * An alternative soulution using <TB>get_user_pages_fast()</TB> is
 * also given.
 *
 * To properly exercise this you'll need to use a page-aligned utility
 * such as dd, or write page-aligned reading and writing programs.
 @*/

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/pagemap.h>

/* either of these (but not both) will work */
//#include "lab_char.h"
#include "lab_miscdev.h"

static ssize_t
mycdrv_rw(struct file *file, unsigned long buf, size_t lbuf, loff_t * ppos,
	  int rw)
{
	int j, nb, rc, npages;
	struct page **pages;
	char *uaddr, *ktmp;

	/* get the total number of pages of memory needed */
	npages = (lbuf - 1) / PAGE_SIZE + 1;

	/* allocate space for array of page structures */

	pr_info("npages = %d for %ld bytes\n", npages, (long)lbuf);
	if (!(pages = kmalloc(npages * sizeof(pages), GFP_KERNEL))) {
		pr_err("allocating pages failed\n");
		return -ENOMEM;
	}

	/* get the page structures, protect with a lock */

	down_read(&current->mm->mmap_sem);
#if 0
	rc = get_user_pages(current, current->mm,
			    (unsigned long)buf, npages, 1, 0, pages, NULL);
#else
	rc = get_user_pages((unsigned long)buf, npages, 1, pages, NULL);
#endif
	up_read(&current->mm->mmap_sem);
	pr_info(" I received %d pages from the user \n", rc);

	for (j = 0, nb = PAGE_SIZE, ktmp = ramdisk; j < npages;
	     j++, ktmp += PAGE_SIZE) {

		/* remap the page address; could also use page_address(page) */

		uaddr = kmap(pages[j]);

		if (j == npages - 1)
			nb = (lbuf - 1) % PAGE_SIZE + 1;

		/* do the actual i/o operation ; for now just to a kernel buffer */

		/* should put the switch outside the loop; for now live with it */

		switch (rw) {
		case 0:	/* read */
			memcpy(uaddr, ktmp, nb);
			//            pr_info(" kernel sees on a read:\n%s\n", uaddr);
			break;
		case 1:	/* write */
			memcpy(ktmp, uaddr, nb);
			//            pr_info(" kernel sees on a write:uaddr:\n%s\n", uaddr);
			break;
		default:	/* should never hit here */
			pr_err("Error in rw routine, rw=%d\n", rw);
			break;
		}

		/* release the page cache -- no surprises to apps */

		set_page_dirty_lock(pages[j]);
#if 0
		page_cache_release(pages[j]);	/* make sure it is released from cache */
#else
		__put_page(pages[j]);	/* make sure it is released from cache */
#endif

		/* release the remap; don't need to do if use page_address() */
		kunmap(pages[j]);
	}

	kfree(pages);

	pr_info("\n Leaving the READ  function, nbytes=%ld\n", (long)lbuf);
	return lbuf;
}

static ssize_t
mycdrv_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	int rw = 0;
	return mycdrv_rw(file, (unsigned long)buf, lbuf, ppos, rw);
}

static ssize_t
mycdrv_write(struct file *file, const char __user * buf, size_t lbuf,
	     loff_t * ppos)
{
	int rw = 1;
	return mycdrv_rw(file, (unsigned long)buf, lbuf, ppos, rw);
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.read = mycdrv_read,
	.write = mycdrv_write,
	.open = mycdrv_generic_open,
	.release = mycdrv_generic_release,
};

module_init(my_generic_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_18/lab2_usermap.c");
MODULE_LICENSE("GPL v2");
