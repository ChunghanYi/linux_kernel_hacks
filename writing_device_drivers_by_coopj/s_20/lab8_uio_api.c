/* **************** LDD:2.0 s_20/lab8_uio_api.c **************** */
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
 * The UIO API
 *
 * In order to write a user-space driver using the UIO API with a
 * small kernel stub driver you'll have to do the following:
 *
 * Allocate space for a uio_info structure, defined
 * in /usr/src/linux/include/linux/uio_driver.h:

 * struct uio_info - UIO device capabilities
 * @uio_dev:            the UIO device this info belongs to
 * @name:               device name
 * @version:            device driver version
 * @mem:                list of mappable memory regions, size==0 for end of list
 * @port:               list of port regions, size==0 for end of list
 * @irq:                interrupt number or UIO_IRQ_CUSTOM
 * @irq_flags:          flags for request_irq()
 * @priv:               optional private data
 * @handler:            the device's irq handler
 * @mmap:               mmap operation for this uio device
 * @open:               open operation for this uio device
 * @release:            release operation for this uio device
 * @irqcontrol:         disable/enable irqs when 0/1 is written to /dev/uioX
 *
 *struct uio_info {
 *        struct uio_device       *uio_dev;
 *        const char              *name;
 *        const char              *version;
 *        struct uio_mem          mem[MAX_UIO_MAPS];
 *        struct uio_port         port[MAX_UIO_PORT_REGIONS];
 *        long                    irq;
 *        unsigned long           irq_flags;
 *        void                    *priv;
 *        irqreturn_t (*handler)(int irq, struct uio_info *dev_info);
 *        int (*mmap)(struct uio_info *info, struct vm_area_struct *vma);
 *        int (*open)(struct uio_info *info, struct inode *inode);
 *        int (*release)(struct uio_info *info, struct inode *inode);
 *        int (*irqcontrol)(struct uio_info *info, s32 irq_on);
 *};
 *
 * You'll need to fill in entries for at least name, irq, irq_flags
 * and handler, which should return IRQ_HANDLED.
 *  
 * The structure should be register and unregistered with:
 *      
 * int uio_register_device(struct device *parent, struct uio_info *info);
 * void uio_unregister_device(struct uio_info *info);
 *
@*/
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/uio_driver.h>

static struct uio_info *info;
static struct device *dev;
static int irq = 1;
module_param(irq, int, S_IRUGO);

static void my_release(struct device *dev)
{
	pr_info("releasing my uio device\n");
}

static irqreturn_t my_handler(int irq, struct uio_info *dev_info)
{
	static int count = 0;
	pr_info("In UIO handler, count=%d\n", ++count);
	return IRQ_HANDLED;
}

static int __init my_init(void)
{
	dev = kzalloc(sizeof(struct device), GFP_KERNEL);
	dev_set_name(dev, "my_uio_device");
	dev->release = my_release;
	device_register(dev);

	info = kzalloc(sizeof(struct uio_info), GFP_KERNEL);
	info->name = "my_uio_device";
	info->version = "0.0.1";
	info->irq = irq;
	info->irq_flags = IRQF_SHARED;
	info->handler = my_handler;

	if (uio_register_device(dev, info) < 0) {
		device_unregister(dev);
		kfree(dev);
		kfree(info);
		pr_info("Failing to register uio device\n");
		return -1;
	}
	pr_info("Registered UIO handler for IRQ=%d\n", irq);
	return 0;
}

static void __exit my_exit(void)
{
	uio_unregister_device(info);
	device_unregister(dev);
	pr_info("Un-Registered UIO handler for IRQ=%d\n", irq);
	kfree(info);
	kfree(dev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_AUTHOR("Jerry Cooperstein");
MODULE_DESCRIPTION("LDD:2.0 s_20/lab8_uio_api.c");
MODULE_LICENSE("GPL v2");
