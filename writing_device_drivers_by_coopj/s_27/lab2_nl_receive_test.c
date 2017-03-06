/* **************** LPD:1.0 s_27/lab2_nl_receive_test.c **************** */
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
 *  (RECEIVER APPLICATION MODULE)
 *
 * We give you a kernel module ( lab2_nl_sender.c) that sends messages
 * to a netlink socket to be monitored by a user application (
 * lab2_nl_receive_test.c).

 * Compile and test the two programs. Try sending multiple messages
 * through straightforward modifications.
 @*/
/*
 Netlink example that receives feedback from the kernel.
 Error handling was skipped in order to reduce code length.

 Copyright Dominic Duval <dduval@redhat.com> according to the terms
 of the GNU Public License.

 (minor changes introduced by J. Cooperstein, coop@coopj.com
  to work with newer kernels.)

*/
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#define MAX_PAYLOAD 1024
#define NL_EXAMPLE 19           /* was 19 */
#define NL_GROUP 1              /* was 1 */

int read_event (int sock)
{
    int ret;
    struct nlmsghdr *nlh;

    nlh = malloc (NLMSG_SPACE (MAX_PAYLOAD));
    memset (nlh, 0, NLMSG_SPACE (MAX_PAYLOAD));
    ret = recv (sock, (void *)nlh, NLMSG_SPACE (MAX_PAYLOAD), 0);

    printf ("Message size: %d , Message: %s\n", ret, (char *)NLMSG_DATA (nlh));

	if (nlh)
		free(nlh);

    return 0;
}

int main (int argc, char *argv[])
{
    struct sockaddr_nl addr;
    int nls;
    int rc;

    /* Set up the netlink socket */
    nls = socket (PF_NETLINK, SOCK_RAW, NL_EXAMPLE);
    printf ("nls=%d\n", nls);

    memset ((void *)&addr, 0, sizeof (addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid ();
    addr.nl_groups = NL_GROUP;
    rc = bind (nls, (struct sockaddr *)&addr, sizeof (addr));
    printf ("errno=%d\n", errno);
    perror ("bind");
    printf ("rc from bind = %d\n", rc);

    while (1)
        read_event (nls);

    return 0;
}
