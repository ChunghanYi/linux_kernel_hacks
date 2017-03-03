/* **************** LDD:2.0 s_04/lab3_seek_test.c **************** */
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
 * Keeping track of file position. (Testing application)
 @*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int length = 20, position = 0, fd, rc;
	char *message, *nodename = "/dev/mycdrv";

	if (argc > 1)
		nodename = argv[1];

	if (argc > 2)
		position = atoi(argv[2]);

	if (argc > 3)
		length = atoi(argv[3]);

	/* set up the message */
	message = malloc(length);
	memset(message, 'x', length);
	message[length - 1] = '\0';	/* make sure it is null terminated */

	/* open the device node */

	fd = open(nodename, O_RDWR);
	printf(" I opened the device node, file descriptor = %d\n", fd);

	/* seek to position */

	rc = lseek(fd, position, SEEK_SET);
	printf("return code from lseek = %d\n", rc);

	/* write to the device node twice */

	rc = write(fd, message, length);
	printf("return code from write = %d\n", rc);
	rc = write(fd, message, length);
	printf("return code from write = %d\n", rc);

	/* reset the message to null */

	memset(message, 0, length);

	/* seek to position */

	rc = lseek(fd, position, SEEK_SET);
	printf("return code from lseek = %d\n", rc);

	/* read from the device node */

	rc = read(fd, message, length);
	printf("return code from read = %d\n", rc);
	printf(" the message was: %s\n", message);

	close(fd);
	exit(0);

}
