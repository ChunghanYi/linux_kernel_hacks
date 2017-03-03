/* **************** LDD:2.0 s_18/lab3_mmap_test.c **************** */
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
/*  Memory Mapping an Allocated Region -- testing program */

/*
 * Author Bill Kerr 8/2003
 * Modifications Jerry Cooperstein.2003-2008
 * LICENSE GPLv2
 @*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <malloc.h>

#define MMAP_DEV_CMD_GET_BUFSIZE 1	/* defines our IOCTL cmd */

void read_and_compare(int fd, char *read_buf, char *mmap_buf, unsigned long len)
{
	/* Read the file and compare with mmap_buf[] */

	if (read(fd, read_buf, len) != len) {
		fprintf(stderr, "read problem:  %s\n", strerror(errno));
		exit(1);
	}
	if (memcmp(read_buf, mmap_buf, len) != 0) {
		fprintf(stderr, "buffer miscompare\n");
		exit(1);
	}
}

int main(int argc, char **argv)
{
	unsigned long j, len;
	int fd;
	char *read_buf, *mmap_buf, *filename = "/dev/mycdrv";

	srandom(getpid());

	if (argc > 1)
		filename = argv[1];

	if ((fd = open(filename, O_RDWR)) < 0) {
		fprintf(stderr, "open of %s failed:  %s\n", filename,
			strerror(errno));
		exit(1);
	}
	/* have the driver tell us the buffer size */
	if (ioctl(fd, MMAP_DEV_CMD_GET_BUFSIZE, &len) < 0) {
		fprintf(stderr, "ioctl failed:  %s\n", strerror(errno));
		exit(1);
	}
	printf("driver's ioctl says buffer size is %ld\n", len);

	read_buf = malloc(len);
	mmap_buf = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (mmap_buf == (char *)MAP_FAILED) {
		fprintf(stderr, "mmap of %s failed:  %s\n", filename,
			strerror(errno));
		exit(1);
	}
	printf("mmap succeeded:  %p\n", mmap_buf);

	/* modify the mmaped buffer */
	for (j = 0; j < len; j++)
		*(mmap_buf + j) = (char)j;

	/* Read the file and compare with mmap_buf[] */
	read_and_compare(fd, read_buf, mmap_buf, len);
	printf("comparison of same data via read() and mmap() successful\n");

	/* Change one randomly chosen byte in the mmap region */

	j = random() % len;
	*(mmap_buf + j) = random() % j;

	/*  repeat the read-back comparison. */
	(void)lseek(fd, 0, SEEK_SET);
	read_and_compare(fd, read_buf, mmap_buf, len);
	printf
	    ("comparison of modified data via read() and mmap() successful\n");

	return 0;
}
