/* **************** LDD:2.0 s_24/lab1_network.c **************** */
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
 * Building a Basic Network Driver Stub
 *
 * Write a basic network device driver.
 *
 * It should register itself upon loading, and unregister upon
 * removal.
 *
 * Supply minimal open() and stop() methods.
 *
 * You should be able to exercise it with:
 *
 *     insmod lab1_network.ko
 *     ifconfig mynet0 up 192.168.3.197
 *     ifconfig mynet0
 *
 * Make sure your chosen address is not being used by anything else.
 *
 * Warning: Depending on kernel version, your stub driver may crash if
 * you try to bring it up or ping it.  If you put in a trivial
 * transmit function, such as
 *
 * static int stub_start_xmit (struct sk_buff *skb, struct net_device *dev)
 * {
 *   dev_kfree_skb (skb);
 *   return 0;
 * }
 * this should avoid the problems.
 *
 @*/
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/init.h>

static struct net_device *dev;

static int my_open(struct net_device *dev)
{
	pr_info("Hit: my_open(%s)\n", dev->name);

	/* start up the transmission queue */

	netif_start_queue(dev);
	return 0;
}

static int my_close(struct net_device *dev)
{
	pr_info("Hit: my_close(%s)\n", dev->name);

	/* shutdown the transmission queue */

	netif_stop_queue(dev);
	return 0;
}

/* Note this method is only needed on some; without it
   module will fail upon removal or use. At any rate there is a memory
   leak whenever you try to send a packet through in any case*/

static int stub_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	dev_kfree_skb(skb);
	return 0;
}

static struct net_device_ops ndo = {
	.ndo_open = my_open,
	.ndo_stop = my_close,
	.ndo_start_xmit = stub_start_xmit,
};

static void my_setup(struct net_device *dev)
{
	int j;
	pr_info("my_setup(%s)\n", dev->name);

	/* Fill in the MAC address with a phoney */

	for (j = 0; j < ETH_ALEN; ++j) {
		dev->dev_addr[j] = (char)j;
	}

	ether_setup(dev);
	dev->netdev_ops = &ndo;
}

static int __init my_init(void)
{
	pr_info("Loading stub network module:....");
#if 0
	dev = alloc_netdev(0, "mynet%d", my_setup);
#else
	dev = alloc_netdev(0, "mynet%d", NET_NAME_UNKNOWN, my_setup);
#endif
	if (register_netdev(dev)) {
		pr_info(" Failed to register\n");
		free_netdev(dev);
		return -1;
	}
	pr_info("Succeeded in loading %s!\n\n", dev_name(&dev->dev));
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("Unloading stub network module\n\n");
	unregister_netdev(dev);
	free_netdev(dev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Bill Shubert");
MODULE_AUTHOR("Jerry Cooperstein");
MODULE_AUTHOR("Tatsuo Kawasaki");
MODULE_DESCRIPTION("LDD:2.0 s_24/lab1_network.c");
MODULE_LICENSE("GPL v2");
