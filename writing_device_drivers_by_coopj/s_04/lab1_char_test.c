/* **************** LDD:2.0 s_04/lab1_char_test.c **************** */
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
   Basic read/write program
 @*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int length, fd1, fd2, rc;
	char *nodename = "/dev/mycdrv";
	char message[] = " *** TESTING CHAR/DRIVER ***\n";

	length = sizeof(message);

	if (argc > 1)
		nodename = argv[1];

	fd1 = open(nodename, O_RDWR);
	printf(" opened file descriptor first time  = %d\n", fd1);
	fd2 = open(nodename, O_RDWR);
	printf(" opened file descriptor second time  = %d\n", fd2);

	rc = write(fd1, message, length);
	printf("return code from write = %d on %d, message=%s\n", rc, fd1,
	       message);

	memset(message, 0, length);

	rc = read(fd2, message, length);
	printf("return code from read  = %d on %d, message=%s\n", rc, fd2,
	       message);

	close(fd1);
	exit(0);
}
