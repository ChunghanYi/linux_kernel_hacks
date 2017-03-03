/* **************** LDD:2.0 s_25/lab1_devices.c **************** */
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
 * Examining Network Devices
 *
 * All network devices are linked together in a list.  You can get a
 * pointer to the head of the list and then *walk through it using:
 *
 *   struct net_device *first_net_device (&init_net);
 *   struct net_device *next_net_device(struct net_device *dev);
 *
 * or even easier:
 *
 *   for_each_netdev(&init_net, dev) { ..... }
 *    
 * Write a module that works its way down the list and prints out
 * information about each driver.
    
 * This should include the name, any associated irq, and various other
 * parameters you may find interesting.
    
 * Try doing this with your previous simple network module loaded.
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>

static void printit(struct net_device *dev)
{
#if 0
	pr_info("name = %6s irq=%4d trans_start=%12lu last_rx=%12lu\n",
		dev->name, dev->irq, dev->trans_start, dev->last_rx);
#else
	pr_info("name = %6s irq=%4d last_rx=%12lu\n",
		dev->name, dev->irq, dev->last_rx);
#endif
}

static int __init my_init(void)
{
	struct net_device *dev;
	pr_info("Hello: module loaded at 0x%p\n", my_init);

	/* either of these methods will work */
	for_each_netdev(&init_net, dev)
	    printit(dev);
	/*
	   dev = first_net_device(&init_net);
	   while (dev) {
	   printit(dev);
	   dev = next_net_device(dev);
	   }
	 */

	return 0;
}

static void __exit my_exit(void)
{
	pr_info("Module Unloading\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_25/lab1_devices.c");
MODULE_LICENSE("GPL v2");
