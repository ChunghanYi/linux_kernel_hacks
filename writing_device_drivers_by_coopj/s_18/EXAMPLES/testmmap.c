/* **************** LDD:2.0 s_18/testmmap.c **************** */
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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

#define DEATH(mess) { perror(mess); exit(errno); }

int main(int argc, char **argv)
{
	int fd, size, rc, j;
	char *area, *tmp, *nodename = "/dev/mycdrv";
	char c[2] = "CX";

	if (argc > 1)
		nodename = argv[1];

	size = getpagesize();	/* use one page by default */
	if (argc > 2)
		size = atoi(argv[2]);

	printf(" Memory Mapping Node: %s, of size %d bytes\n", nodename, size);

	if ((fd = open(nodename, O_RDWR)) < 0)
		DEATH("problems opening the node ");

	area = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (area == MAP_FAILED)
		DEATH("error mmaping");

	/* can close the file now */

	close(fd);

	/* put the string repeatedly in the file */

	tmp = area;
	for (j = 0; j < size - 1; j += 2, tmp += 2)
		memcpy(tmp, &c, 2);

	/* just cat out the file to see if it worked */

	rc = write(STDOUT_FILENO, area, size);

	if (rc != size)
		DEATH("problems writing");

	exit(EXIT_SUCCESS);
}
