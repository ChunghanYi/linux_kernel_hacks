/* **************** LDD:2.0 s_18/lab1_read_write.c **************** */
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
   Basic write program to test the read/write on the character driver
 @*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int length = 20, fd, rc;
	char *message, *nodename = "/dev/mycdrv";

	if (argc > 1)
		nodename = argv[1];

	if (argc > 2)
		length = atoi(argv[2]);

	/* set up the message */
	message = malloc(length);
	memset(message, 'x', length);
	message[length - 1] = '\0';	/* make sure it is null terminated */

	/* open the device node */

	fd = open(nodename, O_RDWR);
	printf(" I opened the device node, file descriptor = %d\n", fd);

	/* write to the device node */

	rc = write(fd, message, length);
	printf("return code from write = %d\n", rc);

	/* reset the message to null */

	memset(message, 0, length);

	/* go back to the beginning */

	lseek(fd, 0, SEEK_SET);

	/* read from the device node */

	rc = read(fd, message, length);
	printf("return code from read = %d\n", rc);
	printf(" the message was: %s\n", message);

	close(fd);
	exit(0);

}
