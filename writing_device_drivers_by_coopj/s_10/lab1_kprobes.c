/* **************** LDD:2.0 s_10/lab1_kprobes.c **************** */
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
 * Using kprobes
 *
 * Place a kprobe at an often executed place in the kernel.  A good
 * choice would be the do_fork() function, which is executed whenever
 * a child process is born.
 *
 * Put in simple handler functions.
 *
 * Test the module by loading it and running simple commands which
 * cause the probed instruction to execute, such as starting a new
 * shell with bash.
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>

#define PRCUR(t) pr_info("current->comm=%s, current->pid=%d\n", t->comm, t->pid);

static char *name = "do_fork";
module_param(name, charp, S_IRUGO);

static struct kprobe kp;

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	pr_info("pre_handler: p->addr=0x%p\n", p->addr);
	PRCUR(current);
	return 0;
}

static void handler_post(struct kprobe *p, struct pt_regs *regs,
			 unsigned long flags)
{
	pr_info("post_handler: p->addr=0x%p\n", p->addr);
	PRCUR(current);
}

static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	pr_info("fault_handler:p->addr=0x%p\n", p->addr);
	PRCUR(current);
	return 0;
}

static int __init my_init(void)
{
	/* set the handler functions */

	kp.pre_handler = handler_pre;
	kp.post_handler = handler_post;
	kp.fault_handler = handler_fault;
	kp.symbol_name = name;

	if (register_kprobe(&kp)) {
		pr_info("Failed to register kprobe, quitting\n");
		return -1;
	}

	pr_info("Hello: module loaded at 0x%p\n", my_init);

	return 0;
}

static void __exit my_exit(void)
{
	unregister_kprobe(&kp);
	pr_info("Bye: module unloaded from 0x%p\n", my_exit);
}

module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_10/lab1_kprobes.c");
MODULE_LICENSE("GPL v2");
