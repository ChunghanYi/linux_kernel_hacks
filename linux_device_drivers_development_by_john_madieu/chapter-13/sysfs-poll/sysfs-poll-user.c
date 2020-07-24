/* ************ LDDD: chapter-13: sysfs-poll-user.c ************ */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <poll.h>

#define TEST_SYSFS_TRIGGER  "/sys/hello/trigger"
#define TEST_SYSFS_NOTIFY   "/sys/hello/notify"

int main(int argc, char **argv)
{
	int cnt, notify_fd, trigger_fd, rv;
	char attrData[100];
	struct pollfd ufds[2];

	if ((notify_fd = open(TEST_SYSFS_NOTIFY, O_RDWR)) < 0) {
		perror("Unable to open notify");
		exit(1);
	}

	if ((trigger_fd = open(TEST_SYSFS_TRIGGER, O_RDWR)) < 0) {
		perror("Unable to open trigger");
		exit(1);
	}

	ufds[0].fd = notify_fd;
	ufds[0].events = POLLPRI|POLLERR;
	ufds[1].fd = trigger_fd;
	ufds[1].events = POLLPRI|POLLERR;

	cnt = read(notify_fd, attrData, 100);
	cnt = read(trigger_fd, attrData, 100);
	ufds[0].revents = 0;
	ufds[1].revents = 0;

#if 0
	if ((rv = poll( ufds, 2, 1000000)) < 0) {
#else
	if ((rv = poll( ufds, 2, 1000000)) < 0)
#endif
		perror("poll error");
	else if (rv == 0)
		printf("Timeout occurred!\n");
	else
		printf("triggered\n");

	printf("revents[0]: %08X\n", ufds[0].revents);
	printf("revents[1]: %08X\n", ufds[1].revents);
	close(trigger_fd);
	close(notify_fd);
}
