/* **************** LDD:2.0 s_21/lab1_ioports.c **************** */
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
 * Accessing I/O Ports From User-Space
 *
 * Look at /proc/ioports to find a free I/O port region.  One
 * possibility to use the first parallel port, usually at 0x378, where
 * you should be able to write a 0 to the register at the base
 * address, and read the next port for status information.
 *
 * Try reading and writing to these ports by using two methods:
 *
 *     ioperm()
 *     /dev/port
 *
 @*/
#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <stdlib.h>
#include <fcntl.h>

#define PARPORT_BASE 0x378

/*
  In each method we will:
  1) Clear the data signal -- see parport_pc.h for register info
  2) Sleep for a millisecond
  3) Read the status port
*/

int do_ioperm(unsigned long addr, unsigned long nports)
{
	unsigned char zero = 0, readout = 0;

	if (ioperm(addr, nports, 1))
		return EXIT_FAILURE;

	printf("Writing: %6d  to  %lx\n", zero, addr);
	outb(zero, addr);

	usleep(1000);

	readout = inb(addr + 1);
	printf("Reading: %6d from %lx\n", readout, addr + 1);

	if (ioperm(addr, nports, 0))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int do_read_devport(unsigned long addr, unsigned long nports)
{
	unsigned char zero = 0, readout = 0;
	int fd;

	if ((fd = open("/dev/port", O_RDWR)) < 0)
		return EXIT_FAILURE;

	if (addr != lseek(fd, addr, SEEK_SET))
		return EXIT_FAILURE;

	printf("Writing: %6d  to  %lx\n", zero, addr);
	write(fd, &zero, 1);

	usleep(1000);

	read(fd, &readout, 1);
	printf("Reading: %6d from %lx\n", readout, addr + 1);
	close(fd);

	return EXIT_SUCCESS;

}

int main(int argc, char *argv[])
{
	unsigned long addr = PARPORT_BASE, nports = 2;

	if (argc > 1)
		addr = strtoul(argv[1], NULL, 0);
	if (argc > 2)
		nports = atoi(argv[2]);

	if (do_read_devport(addr, nports))
		fprintf(stderr, "reading /dev/port method failed");
	if (do_ioperm(addr, nports))
		fprintf(stderr, "ioperm method failed");

	return EXIT_SUCCESS;
}
