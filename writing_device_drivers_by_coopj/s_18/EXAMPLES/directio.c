/* **************** LDD:2.0 s_18/directio.c **************** */
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
 args: 1 = input file, 2 = output file, [3 = chunk size]

 usage: %s infile ofile
        %s infile ofile 512
 */

#define _GNU_SOURCE
#define SECTOR_SIZE 512

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char *buf;
	int fdin, fdout, rcr, rcw;
/* default chunk = 1 page */
	int size = getpagesize();
	if (argc > 3)
		size = atoi(argv[3]);

	/* open input file, must exist */
	fdin = open(argv[1], O_RDONLY);

	/* open output file, create or truncate */
	fdout = open(argv[2], O_DIRECT | O_RDWR | O_CREAT | O_TRUNC, 0666);

	/* use sector aligned memory region */
	/* buf = (char *)memalign (SECTOR_SIZE, size); */
	posix_memalign(&buf, SECTOR_SIZE, size);

	while ((rcr = read(fdin, buf, size)) > 0) {
		rcw = write(fdout, buf, rcr);
		printf("in = %d, out = %d\n", rcr, rcw);
		if (rcr != rcw)
			printf
			    ("Oops, BAD values -- not sector aligned perhaps\n");
	}
	close(fdin);
	close(fdout);
	exit(0);
}
