/* ************ LDD4EP: listing0-1: ioctl_test.c ************ */
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

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
	/* First you need run "mknod /dev/mydev c 202 0" to create /dev/mydev */

	int my_dev = open("/dev/mydev", 0);

	if (my_dev < 0) {
		perror("Fail to open device file: /dev/mydev.");
	} else {
		ioctl(my_dev, 100, 110);   // cmd = 100, arg = 110.
		close(my_dev);
	}

	return 0;
}
