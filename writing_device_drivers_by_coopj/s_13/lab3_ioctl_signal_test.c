/* **************** LDD:2.0 s_13/lab3_ioctl_signal_test.c **************** */
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
 * Using ioctl's to send signals. (User level sending application.)
 @*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>

#define MYIOC_TYPE 'k'
#define MYIOC_SETPID   (int)_IO(MYIOC_TYPE,1)
#define MYIOC_SETSIG   (int)_IO(MYIOC_TYPE,2)
#define MYIOC_SENDSIG  (int)_IO(MYIOC_TYPE,3)
#define SIGDEFAULT SIGKILL

int main(int argc, char *argv[])
{
	int fd, rc;
	unsigned long pid, sig;
	char *nodename = "/dev/mycdrv";

	/* set up the message */
	pid = getpid();
	sig = SIGDEFAULT;

	if (argc > 1)
		pid = atoi(argv[1]);

	if (argc > 2)
		sig = atoi(argv[2]);

	if (argc > 3)
		nodename = argv[3];

	/* open the device node */

	fd = open(nodename, O_RDWR);
	printf(" I opened the device node, file descriptor = %d\n", fd);

	/* send the IOCTL to set the PID */

	rc = ioctl(fd, MYIOC_SETPID, pid);
	printf("\n\n rc from ioctl setting pid is = %d \n\n", rc);

	/* send the IOCTL to set the signal */

	rc = ioctl(fd, MYIOC_SETSIG, sig);
	printf("\n\n rc from ioctl setting signal is = %d \n\n", rc);

	/* send the IOCTL to send the signal */

	rc = ioctl(fd, MYIOC_SENDSIG, "anything");
	printf("\n\n rc from ioctl sending signal is = %d \n\n", rc);

	/* ok go home */
	close(fd);
	printf("\n\n FINISHED, TERMINATING NORMALLY\n");

	exit(0);
}
