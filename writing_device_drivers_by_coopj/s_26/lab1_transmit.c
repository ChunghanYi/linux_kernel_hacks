/* **************** LDD:2.0 s_26/lab1_transmit.c **************** */
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
 * Building a Transmitting Network Driver
 *
 * Extend your stub network device driver to include a transmission
 * function, which means supplying a method for
 * ndo_start_xmit().
 *
 * While you are at it, you may want to add other entry points to see
 * how you may exercise them.
 *
 * Once again, you should be able to exercise it with:
 *
 *   insmod lab_network.ko
 *   ifconfig mynet0 up 192.168.3.197
 *   ping -I mynet0 localhost
 *       or
 *   ping -bI mynet0 192.168.3
 *
 * Make sure your chosen address is not being used by anything else.
 *
 @*/
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/init.h>

static struct net_device *dev;
static struct net_device_stats *stats;

void printline(unsigned char *data, int n){
	char line[256], entry[16];
	int j;
	strcpy(line,"");
	for (j=0; j < n; j++){
		sprintf(entry, " %2x", data[j]);
		strcat(line, entry);
	}
	pr_info("%s\n", line);
}

static int my_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int i;

	pr_info("my_start_xmit(%s)\n", dev->name);

#if 0
	dev->trans_start = jiffies;
#endif
	pr_info("Sending packet :\n");

	/* print out 16 bytes per line */

	for (i = 0; i < skb->len; i += 16) 
		printline(&skb->data[i], (skb->len-i)<16 ? skb->len -i : 16 );
	
	pr_info("\n");

	dev_kfree_skb(skb);
	++stats->tx_packets;

	return 0;
}

static int my_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	pr_info("my_do_ioctl(%s)\n", dev->name);
	return -1;
}

static struct net_device_stats *my_get_stats(struct net_device *dev)
{
	pr_info("my_get_stats(%s)\n", dev->name);
	return stats;
}

/*
 * This is where ifconfig comes down and tells us who we are, etc.
 * We can just ignore this.
 */
static int my_config(struct net_device *dev, struct ifmap *map)
{
	pr_info("my_config(%s)\n", dev->name);
	if (dev->flags & IFF_UP) {
		return -EBUSY;
	}
	return 0;
}

static int my_change_mtu(struct net_device *dev, int new_mtu)
{
	pr_info("my_change_mtu(%s)\n", dev->name);
	return -1;
}

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

static struct net_device_ops ndo = {
	.ndo_open = my_open,
	.ndo_stop = my_close,
	.ndo_start_xmit = my_start_xmit,
	.ndo_do_ioctl = my_do_ioctl,
	.ndo_get_stats = my_get_stats,
	.ndo_set_config = my_config,
	.ndo_change_mtu = my_change_mtu,
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
	dev->flags |= IFF_NOARP;
	stats = &dev->stats;

	/*
	 * Just for laughs, let's claim that we've seen 50 collisions.
	 */
	stats->collisions = 50;
}

static int __init my_init(void)
{
	pr_info("Loading transmitting network module:....");
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
	pr_info("Unloading transmitting network module\n\n");
	unregister_netdev(dev);
	free_netdev(dev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Bill Shubert");
MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_26/lab1_transmit.c");
MODULE_LICENSE("GPL v2");
