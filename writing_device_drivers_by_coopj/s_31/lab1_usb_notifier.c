/* **************** LDD:2.0 s_31/lab1_usb_notifier.c **************** */
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
 * Joining the USB Notifier Chain
 *
 * To register and unregister with the already existing notifier chain
 * for hot-plugging of USB devices, use the exported functions:
 *
 *     void usb_register_notify   (struct notifier_block *nb);
 *     void usb_unregister_notify (struct notifier_block *nb);
 *
 * You should be able to trigger events by plugging and unplugging a
 * USB device, such as a mouse, pendrive, or keyboard.
 *
 * Print out the event that triggers your callback function.  (Note
 * that definitions of events can be found in
 * /usr/src/linux/include/linux/usb.h.).
 @*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/usb.h>

static int my_notifier_call(struct notifier_block *b, unsigned long event,
			    void *data)
{
	pr_info("\nRECEIVING USB event = %ld\n", event);
	switch (event) {
	case USB_DEVICE_ADD:
		pr_info("Adding a USB device, event=USB_DEVICE_ADD\n");
		break;
	case USB_DEVICE_REMOVE:
		pr_info("Removing a USB device, event=USB_DEVICE_REMOVE\n");
		break;
	case USB_BUS_ADD:
		pr_info("Adding a USB bus, event=USB_BUS_ADD\n");
		break;
	case USB_BUS_REMOVE:
		pr_info("Removing a USB bus, event=USB_BUS_REMOVE\n");
		break;
	default:
		pr_info("Receiving an unknown USB event\n");
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block my_nh_block = {
	.notifier_call = my_notifier_call,
	.priority = 0,
};

static int __init my_init(void)
{
	usb_register_notify(&my_nh_block);
	pr_info("USB Notifier module successfully loaded\n");

	return 0;
}

static void __exit my_exit(void)
{
	usb_unregister_notify(&my_nh_block);
	pr_info("USB Notifier module successfully unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_31/lab1_usb_notifier.c");
MODULE_LICENSE("GPL v2");
