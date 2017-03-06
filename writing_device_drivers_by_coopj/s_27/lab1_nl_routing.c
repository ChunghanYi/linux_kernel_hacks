/* **************** LPD:1.0 s_27/lab1_nl_routing.c **************** */
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
 * Using Netlink to monitor routing changes.
 *
 * We give you a program which is capable of monitoring (through
 * netlink) routing table operations: lab1_nl_routing.c.  Compile and
 * execute.
 *
 * Add, delete, and modify routes on your system and see how the
 * program responds. If you stop and restart your network (with
 * service network restart) you should see some messages.  Or you can
 * try some commands such as
 *
 * route add -net 192.56.76.0 netmask 255.255.255.0 dev eth0
 * route add -host 123.213.221.231 eth0
 * route add -net 10.13.21.0 netmask 255.255.255.0 gw 192.168.1.254 eth0
 *
 * If you are ambitious, modify the code to obtain more information
 * about the requests.
 *
 @*/

/*
   Netlink example that uses routing table operations to get feedback
   from the kernel. Error handling code was skipped in order to reduce
   code length.

   Copyright Dominic Duval <dduval@redhat.com> according to the terms
   of the GNU Public License.
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#define MAX_PAYLOAD 1024

int read_event (int sock)
{
    int ret;
    struct nlmsghdr *nlh;

    nlh = malloc (NLMSG_SPACE (MAX_PAYLOAD));
    memset (nlh, 0, NLMSG_SPACE (MAX_PAYLOAD));
    ret = recv (sock, (void *)nlh, NLMSG_SPACE (MAX_PAYLOAD), 0);

    switch (nlh->nlmsg_type) {
    case RTM_NEWROUTE:
        printf ("NEWROUTE\n");
        break;
    case RTM_DELROUTE:
        printf ("DELROUTE\n");
        break;
    case RTM_GETROUTE:
        printf ("GETROUTE\n");
        break;
    default:
        printf ("Unknown type\n");
        break;
    }
    return 0;
}

int main (int argc, char *argv[])
{
    int nls = socket (PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    struct sockaddr_nl addr;

    memset ((void *)&addr, 0, sizeof (addr));

    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid ();
    addr.nl_groups = RTMGRP_IPV4_ROUTE;
    bind (nls, (struct sockaddr *)&addr, sizeof (addr));

    while (1)
        read_event (nls);
    return 0;
}
