/* ************ LDD4EP: listing8-4: sdma.c ************ */
/*
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
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

