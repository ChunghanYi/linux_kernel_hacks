/* **************** LDD:2.0 s_35/lab2_block_test.c **************** */
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
 * Block Driver testing program
 *
 * Write a program to read and write from the node, using the standard
 * unix I/O functions (open, read, write, close).  After loading the
 * module with insmod use this program to access the node.
 @*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define SIZE 2122
#define CHK  250

int main()
{
	int j, length, fd, rc;
	int vector[SIZE];
	off_t offset;

	length = sizeof(int) * SIZE;
	offset = sizeof(int) * CHK;

	fd = open("/dev/mybdrv", O_RDWR);

	for (j = 0; j < SIZE; j++) {
		vector[j] = j;
	}

	rc = write(fd, vector, length);
	printf("\n **** return code from write = %d\n", rc);

	rc = lseek(fd, offset, SEEK_SET);
	printf("\n **** retrun code from lseek(%d) = %d \n", (int)offset, rc);

	rc = read(fd, vector + CHK, sizeof(int));
	printf("\n **** retrun code from read vec[%d] = %d, vec[%d] = %d \n",
	       CHK, rc, CHK, vector[CHK]);

	close(fd);

	exit(0);

}
