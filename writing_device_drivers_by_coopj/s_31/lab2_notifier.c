/* **************** LDD:2.0 s_31/lab2_notifier.c **************** */
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
 * Installing and Using a Notifier Chain
 *
 * Write a brief module that implements its own notifier chain.
 *
 * The module should register the chain upon insertion and
 * unregister upon removal.
 *
 * The callback function should be called at least twice, with
 * different event values, which should be printed out.
 *
 * You may want to make use of the data pointer, modifying the
 * contents in the callback function
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/notifier.h>

static BLOCKING_NOTIFIER_HEAD(my_nh);
static int my_notifier_call(struct notifier_block *b, unsigned long event,
			    void *data)
{
	long *c = (long *)data;
	*c += 100;
	pr_info("\n ..... I was called with event = %ld data=%ld\n", event, *c);
	return NOTIFY_OK;
}

static struct notifier_block my_nh_block = {
	.notifier_call = my_notifier_call,
	.priority = 0,
};

static long counter = 0;

static int __init my_init(void)
{
	int rc;
	if (blocking_notifier_chain_register(&my_nh, &my_nh_block)) {
		pr_info("Failed to register with notifier\n");
		return -1;
	}

	rc = blocking_notifier_call_chain(&my_nh, 1000, &counter);
	pr_info(" rc from call chain = %d\n", rc);
	pr_info("Notifier module successfully loaded\n");

	return 0;
}

static void __exit my_exit(void)
{
	int rc;
	rc = blocking_notifier_call_chain(&my_nh, -1000, &counter);
	pr_info(" rc from call chain = %d\n", rc);
	if (blocking_notifier_chain_unregister(&my_nh, &my_nh_block))
		pr_info(" Failed to unregister from notifier\n");
	pr_info("Notifier module successfully unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_31/lab2_notifier.c");
MODULE_LICENSE("GPL v2");
