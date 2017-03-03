/* **************** LDD:2.0 s_20/lab8_uio_api_test.c **************** */
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
 * The UIO API (test program)
 *
@*/
#include  <stdlib.h>
#include  <stdio.h>
#include  <unistd.h>
#include  <fcntl.h>

int main()
{
	int fd;
	unsigned long nint;
	if ((fd = open("/dev/uio0", O_RDONLY)) < 0) {
		perror("Failed to open /dev/uio0\n");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Started uio test driver.\n");
	while (read(fd, &nint, sizeof(nint)) >= 0)
		fprintf(stderr, "Interrupts: %ld\n", nint);
	exit(EXIT_SUCCESS);
}
