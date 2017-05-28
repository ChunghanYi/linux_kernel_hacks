/* ************ LDD4EP: listing8-4: sdma.c ************ */
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
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>

#define SDMA_BUF_SIZE  (1024*63)

int main(void)
{
	char *virtaddr;
	char phrase[128];
	int my_dev = open("/dev/sdma_test", O_RDWR);

	if (my_dev < 0) {
		perror("Fail to open device file: /dev/sdma_test.");
	} else {
		printf("Enter phrase :\n");
		scanf("%[^\n]%*c", phrase);
		virtaddr = (char *)mmap(0, SDMA_BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, my_dev, 0);
		strcpy(virtaddr, phrase);
		ioctl(my_dev, 0, NULL);
		close(my_dev);
	}

	return 0;
}

