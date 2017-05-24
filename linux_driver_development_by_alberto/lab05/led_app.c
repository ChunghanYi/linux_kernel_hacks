/* ************ LDD4EP: listing5-4: led_app.c ************ */
/*
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_LENGHT 128

int main()
{
	int ret, fd;
	char sendstring[BUFFER_LENGHT];

	printf("Starting led example\n");
	fd = open("/dev/mydev", O_RDWR);
	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}

	do {
		printf("Enter led value: on, off, or exit :\n");
		scanf("%[^\n]%*c", sendstring);
		ret = write(fd, sendstring, strlen(sendstring));
		if (ret < 0) {
			perror("Failed to write the value to the led device");
			return errno;
		}
	} while (strncmp(sendstring, "exit", strlen(sendstring)));

	close(fd);
	printf("Application termined\n");
	return 0;
}
