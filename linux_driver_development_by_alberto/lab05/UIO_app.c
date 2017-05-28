/* ************ LDD4EP: listing5-6: UIO_app.c ************ */
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
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define BUFFER_LENGHT		128
#define GPIO2_GDIR_offset	0x04
#if 0 /* GPIOX_02 */
#define GPIO_DIR_MASK		1<<2
#define GPIO_DATA_MASK		1<<2
#else /* GPIOX_28 */
#define GPIO_DIR_MASK		1<<28
#define GPIO_DATA_MASK		1<<28
#endif
#define UIO_SIZE			"/sys/class/uio/uio0/maps/map0/size"

int main()
{
	int ret, devuio_fd;
	unsigned int uio_size;
	void *temp;
	void *demo_driver_map;
	char sendstring[BUFFER_LENGHT];
	char *led_on = "on";
	char *led_off = "off";
	char *Exit = "exit";

	printf("Starting led example\n");
	devuio_fd = open("/dev/uio0", O_RDWR | O_SYNC);
	if (devuio_fd < 0) {
		perror("Failed to open the device");
		exit(EXIT_FAILURE);
	}

	/* read the size that has to be mapped */
	FILE *size_fp = fopen(UIO_SIZE, "r");
	fscanf(size_fp, "0x%08X", &uio_size);
	fclose(size_fp);

	/* do the mapping */
	demo_driver_map = mmap(NULL, uio_size, PROT_READ|PROT_WRITE, MAP_SHARED, devuio_fd, 0);
	if (demo_driver_map == MAP_FAILED) {
		perror("devuio mmap");
		close(devuio_fd);
		exit(EXIT_FAILURE);
	}

	/* select output direction for GPIO pin */
	temp = demo_driver_map + GPIO2_GDIR_offset;
	*(int *)temp |= GPIO_DIR_MASK;

	/* control the LED */
	do {
		printf("Enter led value: on, off, or exit :\n");
		scanf("%[^\n]%*c", sendstring);
		if (strncmp(led_off, sendstring, 3) == 0) {
			temp = demo_driver_map;
			*(int *)temp |= GPIO_DATA_MASK;
		} else if (strncmp(led_on, sendstring, 2) == 0) {
			temp = demo_driver_map;
			*(int *)temp &= ~(GPIO_DATA_MASK);
		} else if (strncmp(Exit, sendstring, 4) == 0) {
			printf("Exit application\n");
		} else {
			printf("Bad value\n");
			return -EINVAL;
		}

	} while(strncmp(sendstring, "exit", strlen(sendstring)));

	ret = munmap(demo_driver_map, uio_size);
	if (ret < 0) {
		perror("devuio munmap");
		close(devuio_fd);
		exit(EXIT_FAILURE);
	}

	close(devuio_fd);
	printf("Application termined\n");
	exit(EXIT_SUCCESS);
}
