/* **************** LDD:2.0 s_18/lab4_relay_read.c **************** */
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
 * Using Relay Channels. (read()reading program)
 @*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	int fd, j, rc;
	char buf[64];
	char *fname = "/sys/kernel/debug/my_rc_file0";
	if (argc > 1)
		fname = argv[1];
	fd = open(fname, O_RDONLY);
	printf("opening %s, fd=%d\n", fname, fd);

	for (j = 1; j < 20; j++) {
		rc = read(fd, buf, 64);
		printf("rc=%d    %s::", rc, buf);
	}
	exit(0);
}
