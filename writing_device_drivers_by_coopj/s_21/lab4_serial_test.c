/* **************** LDD:2.0 s_21/lab4_serial_test.c **************** */
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
   Serial Mouse Driver application
 @*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

#define MYIOC_TYPE 'k'
#define MYIOC_Z _IO(MYIOC_TYPE,1)

int main(int argc, char *argv[])
{
	int length, fd, rc;
	int buf[2] = { -1000, -1000 };
	ulong MYIOC;

	fd = open("/dev/mymouse", O_RDWR);
	printf(" opened file descriptor  = %d\n", fd);

	if (argc > 1) {
		if (!strcmp(argv[1], "z")) {
			MYIOC = MYIOC_Z;
			rc = ioctl(fd, MYIOC, "anything");
			printf("\n\n rc from ioctl = %d \n\n", rc);
		}
	}
	length = 2 * sizeof(int);
	rc = read(fd, buf, length);

	printf("return code from read  = %d on %d, x = %d, y = %d\n",
	       rc, fd, buf[0], buf[1]);
	close(fd);

	exit(0);
}
