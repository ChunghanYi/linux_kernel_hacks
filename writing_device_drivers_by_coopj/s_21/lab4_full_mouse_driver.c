/* **************** LDD:2.0 s_21/lab4_full_mouse_driver.c **************** */
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
 * Copyright Jerry Cooperstein (GPLv2) (12/2002) (coop@linuxfoundation.org)
 * Extensive revisions Tatuso Kawasaki (redhat) 2008
 */
/*  Provided by Tatuso Kawasaki:

# Xorg configuration created by system-config-display

Section "ServerLayout"
        Identifier     "single head configuration"
        Screen      0  "Screen0" 0 0
        InputDevice    "Keyboard0" "CoreKeyboard"
EndSection

Section "InputDevice"
        Identifier  "Keyboard0"
        Driver      "kbd"
        Option      "XkbModel" "jp106"
        Option      "XkbLayout" "jp"
EndSection

Section "InputDevice"
        Identifier  "Mouse0"
        #Driver      "lab4_full_mouse_driver"
        Driver      "mouse"
        Option      "Device" "/dev/mymouse"
        Option      "Protocol" "microsoft"
        #Option    "SendCoreEvents"
        #Option    "Emulate3Buttons" "true"
EndSection

Section "Monitor"
        Identifier   "Monitor0"
        ModelName    "LCD Panel 1024x768"
 ### Comment all HorizSync and VertSync values to use DDC:
        HorizSync    31.5 - 48.5
        VertRefresh  40.0 - 70.0
        Option      "dpms"
EndSection

Section "Device"
        Identifier  "Videocard0"
        Driver      "mga"
EndSection

Section "Screen"
        Identifier "Screen0"
        Device     "Videocard0"
        Monitor    "Monitor0"
        DefaultDepth     16
        SubSection "Display"
                Viewport   0 0
                Depth     16
                Modes    "1024x768" "800x600" "640x480"
        EndSubSection
EndSection

@*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/ioctl.h>
#include <linux/serial_reg.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/sched.h>

#define MYDEV_NAME "mymouse"

/* Select Serial Port  */

#undef COM1
#undef COM2
#undef COM3
#undef COM4
#define  COM1

/* /dev/ttyS0  com1  */
#ifdef COM1
#define MOUSE_IRQ 4
#define IOSTART  0x03f8
#define IOEXTEND 0x08
#endif

/*  /dev/ttyS1 com2 */
#ifdef COM2
#define MOUSE_IRQ 3
#define IOSTART  0x02f8
#define IOEXTEND 0x08
#endif

/* /dev/ttyS2 com3 */
#ifdef COM3
#define MOUSE_IRQ 4
#define IOSTART  0x03e8
#define IOEXTEND 0x08
#endif

/* /dev/ttyS3 com4 */
#ifdef COM4
#define MOUSE_IRQ 3
#define IOSTART  0x02e8
#define IOEXTEND 0x08
#endif

static dev_t first;
static unsigned int count = 1;
static struct cdev *my_cdev;
static struct class *mymouse_class;

static int my_dev_id;

#define MY_NBYTES 3
#define FIRST_BYTE_MAGIC 0x40

static DECLARE_WAIT_QUEUE_HEAD(my_wait);
static struct {
	unsigned char bytes[5];
	atomic_t ready;
} my_state;

/* save pid for send SIGIO to Xserver */
static long user_pid;

/* send SIGIO to Xserver */
static void t_sendsig(unsigned long unsed)
{
	send_sig(SIGIO, pid_task(find_vpid(user_pid), PIDTYPE_PID), 1);
	/* send_sig (SIGIO, find_task_by_vpid (user_pid), 1); */
}

static DECLARE_TASKLET(usersig, t_sendsig, 0);

static int mymouse_open(struct inode *inode, struct file *file)
{
	pr_info(" OPENING my_mouse device: %s:", MYDEV_NAME);
	pr_info("  MAJOR number = %d, MINOR number = %d\n",
		imajor(inode), iminor(inode));
	atomic_set(&my_state.ready, 0);

	/* set Xserver pid */
	user_pid = current->tgid;
	return 0;
}

static int mymouse_release(struct inode *inode, struct file *file)
{
	pr_info(" RELEASING my_mouse device: %s:", MYDEV_NAME);
	pr_info("  MAJOR number = %d, MINOR number = %d\n",
		imajor(inode), iminor(inode));
	atomic_set(&my_state.ready, 0);

	if (user_pid) {
		tasklet_kill(&usersig);
		user_pid = 0;
	}
	return 0;
}

static ssize_t mymouse_read(struct file *file, char __user * buf, size_t lbuf,
			    loff_t * ppos)
{
	int r;
	/* int flags; */

	/* Must be a request for at least 3 bytes */
	if (lbuf < 3)
		return -EINVAL;

	/* go into a loop until a packet is ready, unless O_NODELAY is specified */

	while (!atomic_read(&my_state.ready)) {
		if (file->f_flags & O_NDELAY)
			return -EAGAIN;

		/* to sleep until a packet is ready */

		wait_event_interruptible(my_wait, atomic_read(&my_state.ready));

		/* Maybe a signal has arrived? */

		if (signal_pending(current))
			return -ERESTARTSYS;
	}

	/* disable this irq while processing */

	disable_irq(MOUSE_IRQ);

	/* just send the raw packet */
	if (copy_to_user(buf, my_state.bytes, 3))
		return -EFAULT;

	/* pad out the rest of the read request with zeros */

	if (lbuf > 3) {
		for (r = 3; r < lbuf; r++)
			put_user(0x00, buf + r);
	}

	/* Ok, we are ready to get a new packet  */

	atomic_set(&my_state.ready, 0);

	/* Let interrupts begin again for the mouse */

	enable_irq(MOUSE_IRQ);

	return lbuf;
}

static ssize_t
mymouse_write(struct file *file, const char __user * buf, size_t lbuf,
	      loff_t * ppos)
{
	/* in case we want to turn on a real write:
	   int r;
	   char byte;
	   pr_info(" Entering the WRITE function, shouldn't be here\n");
	   for (r = 3; r < lbuf; r++) {
	   get_user (byte, buf + r);
	   outb (byte, IOSTART + UART_TX);
	   }
	   return lbuf;
	 */

	/*
	 * Should not be here!
	 */

	return -EINVAL;
}

/* poll for mouse input */

static unsigned int mymouse_poll(struct file *file, poll_table * wait)
{
	poll_wait(file, &my_wait, wait);
	if (atomic_read(&my_state.ready))
		return POLLIN | POLLRDNORM;
	return 0;
}

static long
mymouse_unlocked_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	default:
		pr_info(" got unknown ioctl, CMD=%d\n", cmd);
		return -EINVAL;
	}
	return 0;
}

static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	static int numByteIn = 0;
	char byte;

	/* Check the LSR register; do we have data ready to read? */

	while (inb(IOSTART + UART_LSR) & UART_LSR_DR) {

		/*
		 * We have at least one byte waiting on the UART for us to read it.
		 */

		byte = inb(IOSTART + UART_RX);
		/*      pr_info("byte = %0x\n", byte); */

		/*
		   We accept this byte, *UNLESS* it is the first byte of a
		   my_mouse data packet and it doesn't match the protocol. This
		   lets us resync with the mouse if we somehow get "out of
		   step".
		 */

		if ((numByteIn > 0) || (byte & FIRST_BYTE_MAGIC)) {
			my_state.bytes[numByteIn++] = byte;
		}

		/*
		 * If we have five bytes, then we have a complete my_mouse message.
		 */

		if (numByteIn == MY_NBYTES) {
			atomic_set(&my_state.ready, 1);
			numByteIn = 0;
			wake_up_interruptible(&my_wait);

			/* schedule tasklset for SIGIO */
			if (user_pid)
				tasklet_schedule(&usersig);

		}
	}
	return IRQ_HANDLED;
}

#define BASE_BAUD ( 1843200 / 16 )
#define QUOT ( BASE_BAUD / 1200 )

static void mymouse_turnon(void)
{
	int quot = QUOT;
/* byte 0: Transmit Buffer */
	outb(0x0, IOSTART + UART_TX);
/* byte 1: Interrupt Enable Register */
	outb(UART_IER_RDI, IOSTART + UART_IER);
/* byte 2: FIFO Control Register */
	outb(UART_FCR_ENABLE_FIFO, IOSTART + UART_FCR);
/* byte 3: Line Control Register */
	outb(UART_LCR_WLEN7, IOSTART + UART_LCR);
/* byte 4: Modem Control Register */
	outb(UART_MCR_RTS | UART_MCR_OUT2, IOSTART + UART_MCR);

	/* set baud rate */
	outb(UART_LCR_WLEN7 | UART_LCR_DLAB, IOSTART + UART_LCR);
	outb(quot & 0xff, IOSTART + UART_DLL);	/* LS of divisor */
	outb(quot >> 8, IOSTART + UART_DLM);	/* MS of divisor */
	outb(UART_LCR_WLEN7, IOSTART + UART_LCR);
}

static const struct file_operations mymouse_fops = {
	.owner = THIS_MODULE,
	.read = mymouse_read,
	.write = mymouse_write,
	.open = mymouse_open,
	.release = mymouse_release,
	.unlocked_ioctl = mymouse_unlocked_ioctl,
	.poll = mymouse_poll,
};

static int __init my_init(void)
{
	if (request_irq
	    (MOUSE_IRQ, my_interrupt, IRQF_SHARED, "mymouse", &my_dev_id))
		return -1;
	pr_info(" my_mouse INTERRUPT %d successfully registered\n", MOUSE_IRQ);

	pr_info(" requesting the IO region from 0x%x to 0x%x\n",
		IOSTART, IOSTART + IOEXTEND);
	if (!request_region(IOSTART, IOEXTEND, MYDEV_NAME)) {
		pr_info
		    ("the IO REGION is busy, boldly going forward anyway ! \n");
		/*
		   pr_info("the IO REGION is busy, quitting\n");
		   synchronize_irq (MOUSE_IRQ);
		   free_irq (MOUSE_IRQ, &my_dev_id);
		   return -1
		 */
	}
#if 0
	first = MKDEV(my_major, my_minor);
	if (register_chrdev_region(first, count, MYDEV_NAME)) {
		pr_err("Failed registering\n");
		return -1;
	}
#else
	if (!(my_cdev = cdev_alloc())) {
		pr_err("failed to cdev_alloc\n");
		unregister_chrdev_region(first, count);
		return -1;
	}
	cdev_init(my_cdev, &mymouse_fops);
	cdev_add(my_cdev, first, count);
#endif

	if (alloc_chrdev_region(&first, 0, count, MYDEV_NAME) < 0) {
		pr_err("failed to allocate character device region\n");
		return -1;
	}
	if (!(my_cdev = cdev_alloc())) {
		pr_err("cdev_alloc() failed\n");
		unregister_chrdev_region(first, count);
		return -1;
	}
	cdev_init(my_cdev, &mymouse_fops);
	if (cdev_add(my_cdev, first, count) < 0) {
		pr_err("cdev_add() failed\n");
		cdev_del(my_cdev);
		unregister_chrdev_region(first, count);
		return -1;
	}

	mymouse_class = class_create(THIS_MODULE, "mymouse");
	device_create(mymouse_class, NULL, first, NULL, "%s", "mymouse");

	pr_info(" turning on the mouse\n");
	mymouse_turnon();
	atomic_set(&my_state.ready, 0);
	return 0;
}

static void __exit my_exit(void)
{
	device_destroy(mymouse_class, first);
	class_destroy(mymouse_class);
	unregister_chrdev_region(first, count);
	release_region(IOSTART, IOEXTEND);
	synchronize_irq(MOUSE_IRQ);
	free_irq(MOUSE_IRQ, &my_dev_id);
	pr_info(" releasing  the IO region from 0x%x to 0x%x\n",
		IOSTART, IOSTART + IOEXTEND);
	pr_info("removing the module\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Tatsuo Kawasaki");
MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_21/lab4_full_mouse_driver.c");
MODULE_LICENSE("GPL v2");
