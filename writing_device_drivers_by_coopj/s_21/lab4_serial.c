/* **************** LDD:2.0 s_21/lab4_serial.c **************** */
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
 * Serial Mouse Driver (See write up for long description :)
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

#define MY_IOC_TYPE 'k'
#define MY_IOC_Z _IO(MY_IOC_TYPE,1)
static int interrupt_number = 0, xpos = 0, ypos = 0;

static dev_t first;
static unsigned int count = 1;
static int my_major = 700, my_minor = 0;
static struct cdev *my_cdev;

static int my_dev_id;

static int mymouse_open(struct inode *inode, struct file *file)
{
	pr_info(" OPENING my_mouse device: %s:", MYDEV_NAME);
	pr_info("  MAJOR number = %d, MINOR number = %d\n",
		imajor(inode), iminor(inode));
	return 0;
}

static int mymouse_release(struct inode *inode, struct file *file)
{
	pr_info(" RELEASING my_mouse device: %s:", MYDEV_NAME);
	pr_info("  MAJOR number = %d, MINOR number = %d\n",
		imajor(inode), iminor(inode));
	return 0;
}

static ssize_t
mymouse_read(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	pr_info(" Entering the  READ function\n");

	if (lbuf < 2 * sizeof(int)) {
		pr_info
		    (" mymouse_read: buffer not long enough to store x and y\n");
		return -EFAULT;
	}
	pr_info("xpos=%d, ypos=%d\n", xpos, ypos);
	if (copy_to_user(buf, &xpos, sizeof(int)))
		return -EFAULT;
	if (copy_to_user(buf + sizeof(int), &ypos, sizeof(int)))
		return -EFAULT;
	return 2 * sizeof(int);
}

static ssize_t
mymouse_write(struct file *file, const char __user * buf, size_t lbuf,
	      loff_t * ppos)
{
	pr_info(" Entering the WRITE function\n");
	return lbuf;
}

static long
mymouse_unlocked_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case MY_IOC_Z:
		pr_info(" zeroing out x and y, which were x= %d, y= %d\n", xpos,
			ypos);
		xpos = 0;
		ypos = 0;
		break;
	default:
		pr_info(" got unknown ioctl, CMD=%d\n", cmd);
		return -EINVAL;
	}
	return 0;
}

static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	/*
	 * Note that using static values here is a VERY BAD IDEA. This means that we
	 * can only support one mouse at a time with this driver; otherwise their
	 * packets can get intermixed and confused! The right thing to do would be
	 * to have our byte_in buffer and nbytes_in value held in our device
	 * extension.
	 */
	static unsigned char byte_in[3];
	static int nbytes_in = 0;
	char byte, left, right;
	int dx, dy;

	interrupt_number++;

	/* Read a byte */

	byte = inb(IOSTART + UART_RX);

	/*
	 * We accept this byte, *UNLESS* it is the first byte of a mouse data
	 * packet and it doesn't match the protocol. This lets us resync with the
	 * mouse if we somehow get "out of step".
	 */
	if ((nbytes_in > 0) || (byte & 0x40)) {
		byte_in[nbytes_in++] = byte;
	}
	/*
	 * If we have three bytes, then we have a complete mouse message.
	 */
	if (nbytes_in == 3) {

		/* Change in X since last message */
		dx = (signed char)(((byte_in[0] & 0x03) << 6) |
				   (byte_in[1] & 0x3f));
		/* Change in Y since last message */
		dy = (signed char)(((byte_in[0] & 0x0c) << 4) |
				   (byte_in[2] & 0x3f));
		/* Is the left button pressed? */
		left = (byte_in[0] & 0x20 ? 'L' : ' ');
		/* Is the right button pressed? */
		right = (byte_in[0] & 0x10 ? 'R' : ' ');
		/* Increment x and y positions */
		xpos += dx;
		ypos += dy;
		pr_info("mymouse_int %d", interrupt_number);
		pr_info("  Data = %2x:%2x:%2x (buttons[%c%c]), dx=%d, dy=%d",
			byte_in[0], byte_in[1], byte_in[2], left, right, dx,
			dy);
		pr_info(" x=%d, y=%d\n", xpos, ypos);
		nbytes_in = 0;
	}
	/* we return IRQ_NONE because we are just observing */
	return IRQ_NONE;
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

	first = MKDEV(my_major, my_minor);
	if (register_chrdev_region(first, count, MYDEV_NAME)) {
		pr_err("Failed registering\n");
		return -1;
	}
	if (!(my_cdev = cdev_alloc())) {
		pr_err("failed to cdev_alloc\n");
		unregister_chrdev_region(first, count);
		return -1;
	}
	cdev_init(my_cdev, &mymouse_fops);
	cdev_add(my_cdev, first, count);

	pr_info(" turning on the mouse\n");
	mymouse_turnon();
	return 0;
}

static void __exit my_exit(void)
{
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

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_21/lab4_serial.c");
MODULE_LICENSE("GPL v2");
