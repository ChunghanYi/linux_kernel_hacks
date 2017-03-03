/* **************** LDD:2.0 s_19/lab3_poll_test.c **************** */
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
 * User-Space Interrupt Handling (testing program)
 *
 * Adapt the character driver with polling to handle a shared
 * interrupt.
 *
 * The read method should sleep until events are available and then
 * deal with potentially multiple events.
 *
 * The information passed back by the read should include the number
 * of events.
 *
 * You'll need a testing program that opens the device node and then
 * sits on it with poll() until interrupts arrive.
 *
 * You can probably also implement a solution that does not involve
 * poll(), but just a blocking read.
 @*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>

int main(int argc, char *argv[])
{
	struct pollfd ufds[1];
	int timeout = 10000;	/* time out for poll */
	int fd, rc, nbytes = 32;
	char *buffer, *filename = "/dev/mycdrv";

	if (argc > 1)
		filename = argv[1];
	if (argc > 2)
		nbytes = atoi(argv[2]);
	buffer = malloc(nbytes);

	fd = open(filename, O_RDONLY);
	printf("opened file: %s,  with file descriptor = %d\n", filename, fd);

	ufds[0].fd = fd;
	ufds[0].events = POLLIN;

	for (;;) {

		if ((rc = poll(ufds, 1, timeout)) < 0) {
			perror("Failure in poll\n");
			exit(EXIT_FAILURE);
		}

		if (rc > 0) {

			printf(" poll returns %d, revents = 0x%03x", rc,
			       ufds[0].revents);

			if (ufds[0].revents & POLLIN) {
				rc = read(fd, buffer, nbytes);
				printf("reading %d bytes:%s\n", rc, buffer);
			} else {
				printf("POLLIN not set!\n");
			}

		} else {
			printf("poll timed out in %d milliseconds on %s.\n",
			       timeout, filename);
		}
	}
	close(fd);
	printf("Shutting down %s\n", argv[0]);
	exit(EXIT_SUCCESS);
}
