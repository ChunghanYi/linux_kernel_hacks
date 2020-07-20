/* ************ LDD4EP(2): sdma_stm32mp1_mmap.c ************ */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/dma-mapping.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/dmaengine.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

struct dma_private {
	struct miscdevice dma_misc_device;
	struct device *dev;
	char *wbuf;
	char *rbuf;
	struct dma_chan *dma_m2m_chan;
	struct completion dma_m2m_ok;
	dma_addr_t dma_src;
	dma_addr_t dma_dst;
};

#define SDMA_BUF_SIZE  (1024*63)


static void dma_m2m_callback(void *data)
{
	struct dma_private *dma_priv = data;
	dev_info(dma_priv->dev, "%s\n finished DMA transaction" ,__func__);
	complete(&dma_priv->dma_m2m_ok);
}

static int sdma_open(struct inode *inode, struct file *file)
{
	struct dma_private *dma_priv;
	dma_priv = container_of(file->private_data, struct dma_private, dma_misc_device);

	dma_priv->wbuf = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if (!dma_priv->wbuf) {
		dev_err(dma_priv->dev, "error allocating wbuf !!\n");
		return -ENOMEM;
	}

	dma_priv->rbuf = kzalloc(SDMA_BUF_SIZE, GFP_DMA);
	if (!dma_priv->rbuf) {
		dev_err(dma_priv->dev, "error allocating rbuf !!\n");
		return -ENOMEM;
	}

	dma_priv->dma_src = dma_map_single(dma_priv->dev, dma_priv->wbuf,
			SDMA_BUF_SIZE, DMA_TO_DEVICE);

	return 0;
}

static long sdma_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct dma_async_tx_descriptor *dma_m2m_desc;
	struct dma_device *dma_dev;
	struct dma_private *dma_priv;
	dma_cookie_t cookie;

	dma_priv = container_of(file->private_data, struct dma_private, dma_misc_device);

	dma_dev = dma_priv->dma_m2m_chan->device;

	dma_priv->dma_src = dma_map_single(dma_priv->dev, dma_priv->wbuf,
			SDMA_BUF_SIZE, DMA_TO_DEVICE);
	dma_priv->dma_dst = dma_map_single(dma_priv->dev, dma_priv->rbuf,
			SDMA_BUF_SIZE, DMA_TO_DEVICE);

	dma_m2m_desc = dma_dev->device_prep_dma_memcpy(dma_priv->dma_m2m_chan,
			dma_priv->dma_dst,
			dma_priv->dma_src,
			SDMA_BUF_SIZE,
			DMA_CTRL_ACK | DMA_PREP_INTERRUPT);

	dev_info(dma_priv->dev, "successful descriptor obtained");

	dma_m2m_desc->callback = dma_m2m_callback;
	dma_m2m_desc->callback_param = dma_priv;
	init_completion(&dma_priv->dma_m2m_ok);

	cookie = dmaengine_submit(dma_m2m_desc);

	if (dma_submit_error(cookie)) {
		dev_err(dma_priv->dev, "Failed to submit DMA\n");
		return -EINVAL;
	};

	dma_async_issue_pending(dma_priv->dma_m2m_chan);
	wait_for_completion(&dma_priv->dma_m2m_ok);
	dma_async_is_tx_complete(dma_priv->dma_m2m_chan, cookie, NULL, NULL);

	dma_unmap_single(dma_priv->dev, dma_priv->dma_src,
			SDMA_BUF_SIZE, DMA_TO_DEVICE);
	dma_unmap_single(dma_priv->dev, dma_priv->dma_dst,
			SDMA_BUF_SIZE, DMA_TO_DEVICE);

	if (*(dma_priv->rbuf) != *(dma_priv->wbuf)) {
		dev_err(dma_priv->dev, "buffer copy failed!\n");
		return -EINVAL;
	}

	dev_info(dma_priv->dev, "buffer copy passed!\n");
	dev_info(dma_priv->dev, "wbuf is %s\n", dma_priv->wbuf);
	dev_info(dma_priv->dev, "rbuf is %s\n", dma_priv->rbuf);

	kfree(dma_priv->wbuf);
	kfree(dma_priv->rbuf);

	return 0;
}

static int sdma_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct dma_private *dma_priv;

	dma_priv = container_of(file->private_data, struct dma_private, dma_misc_device);

	if (remap_pfn_range(vma, vma->vm_start, dma_priv->dma_src >> PAGE_SHIFT,
				vma->vm_end - vma->vm_start, vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

struct file_operations dma_fops = {
	.owner 			= THIS_MODULE,
	.open  			= sdma_open,
	.unlocked_ioctl		= sdma_ioctl,
	.mmap		 	= sdma_mmap,
};

static int my_probe(struct platform_device *pdev)
{
	int retval;
	struct dma_private *dma_device;
	dma_cap_mask_t dma_m2m_mask;

	dev_info(&pdev->dev, "platform_probe enter\n");

	dma_device = devm_kzalloc(&pdev->dev, sizeof(struct dma_private), GFP_KERNEL);

	dma_device->dma_misc_device.minor = MISC_DYNAMIC_MINOR;
	dma_device->dma_misc_device.name = "sdma_test";
	dma_device->dma_misc_device.fops = &dma_fops;

	dma_device->dev = &pdev->dev;

	dma_cap_zero(dma_m2m_mask);
	dma_cap_set(DMA_MEMCPY, dma_m2m_mask);
	dma_device->dma_m2m_chan = dma_request_channel(dma_m2m_mask, 0, NULL);
	if (!dma_device->dma_m2m_chan) {
		dev_err(&pdev->dev, "Error opening the SDMA memory to memory channel\n");
		return -EINVAL;
	}

	retval = misc_register(&dma_device->dma_misc_device);
	if (retval)
		return retval;

	platform_set_drvdata(pdev, dma_device);

	dev_info(&pdev->dev, "platform_probe exit\n");

	return 0;
}

static int my_remove(struct platform_device *pdev)
{
	struct dma_private *dma_device = platform_get_drvdata(pdev);
	dev_info(&pdev->dev, "platform_remove enter\n");
	misc_deregister(&dma_device->dma_misc_device);
	dma_release_channel(dma_device->dma_m2m_chan);
	dev_info(&pdev->dev, "platform_remove exit\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,sdma_m2m"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "sdma_m2m",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

static int demo_init(void)
{
	int ret_val;
	pr_info("demo_init enter\n");

	ret_val = platform_driver_register(&my_platform_driver);
	if (ret_val != 0) {
		pr_err("platform value returned %d\n", ret_val);
		return ret_val;
	}
	pr_info("demo_init exit\n");
	return 0;
}

static void demo_exit(void)
{
	pr_info("demo_exit enter\n");
	platform_driver_unregister(&my_platform_driver);
	pr_info("demo_exit exit\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a SDMA mmap memory to memory driver");
