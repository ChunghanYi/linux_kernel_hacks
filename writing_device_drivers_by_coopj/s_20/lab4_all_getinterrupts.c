/* **************** LDD:2.0 s_20/lab4_all_getinterrupts.c **************** */
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
/* Sharing All Interrupts, Bottom Halves (get results application)
 *
 * Extend the solution to share all possible interrupts, and evaluate
 * the consumer/producer problem.
 *
 @*/
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DEATH(mess) { perror(mess); exit(errno); };

#define MAXIRQS 256
#define NB (MAXIRQS * sizeof(int))

int main(int argc, char *argv[])
{
	int fd, j;
	char *nodename = "/dev/mycdrv";
	int *interrupts = malloc(NB);
	int *bhs = malloc(NB);

	if (argc > 1)
		nodename = argv[1];

	if ((fd = open(nodename, O_RDONLY)) < 0)
		DEATH("opening device node");
	if (read(fd, interrupts, NB) != NB)
		DEATH("reading interrupts");
	if (read(fd, bhs, NB) != NB)
		DEATH("reading bhs");

	for (j = 0; j < MAXIRQS; j++)
		if (interrupts[j] > 0)
			printf(" %4d %10d%10d\n", j, interrupts[j], bhs[j]);
	exit(0);
}
