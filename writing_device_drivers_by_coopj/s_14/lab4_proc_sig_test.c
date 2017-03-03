/* **************** LDD:2.0 s_14/lab4_proc_sig_test.c **************** */
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
 * Using proc to send signals.
 *
 * User level sending application.
 *
 @*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>

void print_and_rewind(FILE * fp_pid, FILE * fp_tosend)
{
	unsigned long pid, sig;

	/* read current values and print */
	fscanf(fp_pid, "%lud", &pid);
	fscanf(fp_tosend, "%lud", &sig);
	printf("pid = %ld, signal = %ld\n\n", pid, sig);

	/* set back to the beginning */
	fseek(fp_pid, 0, SEEK_SET);
	fseek(fp_tosend, 0, SEEK_SET);
}

int main(int argc, char *argv[])
{
	FILE *fp_pid, *fp_tosend;
	unsigned long pid, sig;

	/* set up the values if on the command line */

	pid = getpid();
	sig = SIGKILL;

	if (argc > 1)
		pid = atoi(argv[1]);

	if (argc > 2)
		sig = atoi(argv[2]);

	/* open the proc files node */

	fp_pid = fopen("/proc/my_sig_dir/pid", "w+");
	fp_tosend = fopen("/proc/my_sig_dir/signal", "w+");
	printf
	    ("\n I opened /proc/my_sig_dir/pid and /proc/my_sig_dir/signal\n\n");

	printf(" \nValues read from /proc before doing anything:\n");
	print_and_rewind(fp_pid, fp_tosend);

	/* set the pid */

	fprintf(fp_pid, "%ld", pid);
	fseek(fp_pid, 0, SEEK_SET);

	printf(" \nValues read from /proc after setting pid:\n");
	print_and_rewind(fp_pid, fp_tosend);

	printf("\n Sending the signal:\n");
	fprintf(fp_tosend, "%ld", sig);
	fseek(fp_tosend, 0, SEEK_SET);

	printf(" \nValues read from /proc after sending signal:\n");
	print_and_rewind(fp_pid, fp_tosend);
	/* ok go home */
	fclose(fp_pid);
	fclose(fp_tosend);

	printf("\n\n FINISHED, TERMINATING NORMALLY\n");

	exit(0);
}
