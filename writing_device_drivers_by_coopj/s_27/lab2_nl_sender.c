/* **************** LPD:1.0 s_27/lab2_nl_sender.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2009
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
 *  Using Netlink to send kernel messages to an application.
 *  (SENDER MODULE)
 *
 * We give you a kernel module ( lab2_nl_sender.c) that sends messages
 * to a netlink socket to be monitored by a user application (
 * lab2_nl_receive_test.c).

 * Compile and test the two programs. Try sending multiple messages
 * through straightforward modifications.
 *
 @*/
/*
  Netlink demo that sends a message from kernel-space to user-space.

  Copyright Dominic Duval <dduval@redhat.com> according to the terms
  of the GNU Public License.

 (minor changes introduced by J. Cooperstein, coop@coopj.com
  to work with newer kernels.)

 (major changes introduced by Paul Drews, paul.drews@intel.com)

*/

#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/kthread.h>

#define MAX_PAYLOAD 1024
#define NETLINK_MESSAGE "This message originated from the kernel!"
#define NL_EXAMPLE 19           /* was 19 */
#define NL_GROUP 1              /* was 1 */

#define MSG_INTERVAL_SEC 5

#define seconds_to_jiffies(sec) (cputime_to_jiffies(secs_to_cputime(sec)))

static struct sock *nl_sk = NULL;
static struct task_struct *sender_thread = NULL;

static int my_sender_thread (void *data)
{
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh;

    do {
        skb = alloc_skb (NLMSG_SPACE (MAX_PAYLOAD), GFP_KERNEL);
        nlh = (struct nlmsghdr *)skb_put (skb, NLMSG_SPACE (MAX_PAYLOAD));
        nlh->nlmsg_len = NLMSG_SPACE (MAX_PAYLOAD);
        nlh->nlmsg_pid = 0;
        nlh->nlmsg_flags = 0;
        strcpy (NLMSG_DATA (nlh), NETLINK_MESSAGE);
#if 0
        NETLINK_CB (skb).pid = 0;
#endif
/*      NETLINK_CB (skb).dst_pid = 0;  removed in 2.6.20 */
        NETLINK_CB (skb).dst_group = NL_GROUP;
        netlink_broadcast (nl_sk, skb, 0, NL_GROUP, GFP_KERNEL);
        printk (KERN_INFO "my_sender_thread sent a message at jiffies=%ld\n",
                jiffies);
        set_current_state (TASK_INTERRUPTIBLE);
        schedule_timeout (seconds_to_jiffies (MSG_INTERVAL_SEC));
    } while (!kthread_should_stop ());

    return 0;

}	/* my_sender_thread */

static int __init my_init (void)
{
#if 0
    nl_sk = netlink_kernel_create (&init_net, NL_EXAMPLE, 0, NULL, NULL,
                                   THIS_MODULE);
#else
    nl_sk = netlink_kernel_create (&init_net, NL_EXAMPLE, NULL);
#endif

    if (nl_sk == NULL)
        return -1;

    sender_thread = kthread_run (my_sender_thread,
                                 NULL, "lab2_nl_sender_thread");
    if (IS_ERR (sender_thread)) {
        printk (KERN_INFO "Error %ld createing thread\n",
                PTR_ERR (sender_thread));
        sock_release (nl_sk->sk_socket);
        return -1;
    }

    printk (KERN_INFO "Adding netlink testing module\n");

    return 0;
}

static void __exit my_exit (void)
{
    printk (KERN_INFO "Removing netlink testing module\n");
    kthread_stop (sender_thread);
    sock_release (nl_sk->sk_socket);
}

module_exit (my_exit);
module_init (my_init);

MODULE_AUTHOR ("Dominic Duval");
MODULE_AUTHOR ("Paul Drews");
MODULE_AUTHOR ("Jerry Cooperstein");
MODULE_DESCRIPTION ("LPD:1.0 s_27/lab2_nl_sender.c");
MODULE_LICENSE ("GPL v2");
