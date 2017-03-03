/* **************** LDD:2.0 s_13/lab3_ioctl_signal.c **************** */
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
 * Using ioctl's to send signals.
 *
 * It is sometimes desireable to send a signal to an application from
 * within the kernel.  The function for doing this is:
 *
 * int send_sig(int signal, struct task_struct *tsk, int priv);
 *
 * where signal is the signal to send, tsk points to the task
 * structure corresponding to the process to which the signal should
 * be sent, and priv indicates the privilege level (0 for user
 * applications, 1 for the kernel.)
 *
 * Write a character driver that has three ioctl commands:
 *
 *    Set the process ID to which signals should be sent.
 *    Set the signal which should be sent.
 *    Send the signal.
 *
 * Remember you'll have to use pid_task(find_vpid(pid), PIDTYPE_PID)
 * to connect the pid to the task structure it corresponds with.
 *
 * You'll also have to develop the sending program.
 *
 *    If given no arguments it should send SIGKILL to the current
 *    process.
 *
 *    If given one argument it should set the process ID to send signals to.
 *
 *    If given two arguments it should also set the signal.
 *
 @*/

#include <linux/module.h>

/* either of these (but not both) will work */
//#include "lab_char.h"
#include "lab_miscdev.h"

#define MYIOC_TYPE 'k'
#define MYIOC_SETPID   _IO(MYIOC_TYPE,1)
#define MYIOC_SETSIG   _IO(MYIOC_TYPE,2)
#define MYIOC_SENDSIG  _IO(MYIOC_TYPE,3)

static int sig_pid = 0;
static struct task_struct *sig_tsk = NULL;
static int sig_tosend = SIGKILL;

static inline long
mycdrv_unlocked_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	int retval;
	switch (cmd) {
	case MYIOC_SETPID:
		sig_pid = (int)arg;
		pr_info(
		       " Setting pid to send signals to, sigpid = %d\n",
		       sig_pid);
		/*        sig_tsk = find_task_by_vpid (sig_pid); */
		sig_tsk = pid_task(find_vpid(sig_pid), PIDTYPE_PID);
		break;
	case MYIOC_SETSIG:
		sig_tosend = (int)arg;
		pr_info(" Setting signal to send as: %d \n", sig_tosend);
		break;
	case MYIOC_SENDSIG:
		if (!sig_tsk) {
			pr_info(
			       "You haven't set the pid; using current\n");
			sig_tsk = current;
			sig_pid = (int)current->pid;
		}
		pr_info(" Sending signal %d to process ID %d\n",
			sig_tosend, sig_pid);
		retval = send_sig(sig_tosend, sig_tsk, 0);
		pr_info("retval = %d\n", retval);

		break;
	default:
		pr_info(" got invalid case, CMD=%d\n", cmd);
		return -EINVAL;
	}
	return 0;
}

static const struct file_operations mycdrv_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = mycdrv_unlocked_ioctl,
	.open = mycdrv_generic_open,
	.release = mycdrv_generic_release
};

module_init(my_generic_init);
module_exit(my_generic_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_13/lab3_ioctl_signal.c");
MODULE_LICENSE("GPL v2");
