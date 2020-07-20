/*
 * Copyright (C) 2018 Microchip Technology Inc.
 * Marek Sieranski <marek.sieranski@microchip.com>
 *
 * app-lantern - a program that interract with the lantern device driver.
 * It processes the command line and if the request is formally correct
 * it opens the lantern device and send approprite ioctl command to it 
 * If the device returns an answer, it writes it to the console and exits.
 * The application sets the color of the leds. The three lowest bits of
 * the numeric parameter carry the rgb color information, e.g. 4 refers
 * to the red., 3 would set cyan color.
 *
 * This program is free software, you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your discretion) any later version.
 */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "lantern.h"

#define ARRAY_SIZE(a)	(sizeof(a)/sizeof(*a))
#define NODEPATH	"/dev/lantern"

struct command {
	char *name;	/* command name */
	unsigned int cmd;	/* ioctl uniq id */
	unsigned int count;	/* number of parameters */
};
struct command commands[] = {
	{ "c", LANTERN_IOC_CLEAR, 2 },
	{ "r", LANTERN_IOC_READ,  2 },
	{ "w", LANTERN_IOC_WRITE, 3 },
	{ "t", LANTERN_IOC_TOGGLE, 2 },
};

void help()
{
	fprintf(stderr, "usage: app_lantern c | r | w <rgb> | t\n");
}

int main(int argc, char *argv[])
{
	int cmd, arg;
	int f;
	int j = 0;
	long int result;

	/* processing the command line */
	if (argc > 1) {
		for (j = 0; j < ARRAY_SIZE(commands); ++j) {
			if (strcmp(argv[1], commands[j].name) == 0)
				break;
		}
	}

	if (j >= ARRAY_SIZE(commands)) {
		fprintf(stderr, "Invalid command - %s\n", argv[1]);
		help();
		return -1;
	}

	if ( argc != commands[j].count ) {
		fprintf(stderr, "Invalid number of parameters - %d\n", argc);
		help();
		return -1;
	}

	if (j == 2)
		arg = atoi(argv[2]);

	/* communicating with the device driver */
	f = open(NODEPATH, 0);
	if (f < 0) {
		fprintf(stderr, "Failed to open the device\n");
		return -1;
	}

	result = ioctl(f, commands[j].cmd, (long int)arg);
	if (result < 0) {
		perror("ioctl failed");
	} else if (j == 1) {
		printf("lantern color = %d\n", (int)result);
		result = 0;
	}

	usleep(100000); /* 100 ms, allow printf to finish printing */
	close(f);

	return result;
}
