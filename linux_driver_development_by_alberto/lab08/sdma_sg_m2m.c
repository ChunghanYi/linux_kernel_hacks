/* ************ LDD4EP: listing8-2: sdma_sg_m2m.c ************ */
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
#include <linux/platform_data/dma-imx.h>
#include <linux/dmaengine.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

static dma_addr_t dma_dst;
static dma_addr_t dma_src;
static char *dma_dst_coherent;
static char *dma_src_coherent;
static unsigned int *wbuf, *wbuf2, *wbuf3;
static unsigned int *rbuf, *rbuf2, *rbuf3;

static struct dma_chan *dma_m2m_chan;

static struct completion dma_m2m_ok;

static struct scatterlist sg3[1], sg4[1];
static struct scatterlist sg[3], sg2[3];

#define SDMA_BUF_SIZE  (63*1024)

static bool dma_m2m_filter(struct dma_chan *chan, void *param)
{
	if (!imx_dma_is_general_purpose(chan))
		return false;
	chan->private = param;
	return true;
}

static int sdma_open(struct inode *inode, struct file *filp)
{
	pr_info("sdma open is called.\n");
	return 0;
}

static int sdma_release(struct inode *inode, struct file *filp)
{
	pr_info("sdma release is called.\n");
	return 0;
}

static void dma_sg_callback(void *data)
{
	unsigned int i;

	pr_info("%s\n finished SG DMA transaction", __func__);

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf+i) != *(wbuf+i)) {
			pr_info("buffer 1 copy failed!\n");
			return;
		}
	}
	pr_info("buffer 1 copy passed!\n");

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf2+i) != *(wbuf2+i)) {
			pr_info("buffer 2 copy failed!\n");
			return;
		}
	}
	pr_info("buffer 2 copy passed!\n");

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		if (*(rbuf3+i) != *(wbuf3+i)) {
			pr_info("buffer 3 copy failed!\n");
			return;
		}
	}
	pr_info("buffer 3 copy passed!\n");
	complete(&dma_m2m_ok);
}

static void dma_m2m_callback(void *data)
{
	pr_info("%s\n finished DMA coherent transaction" , __func__);

	if (*(dma_src_coherent) != *(dma_dst_coherent)) {
		pr_info("buffer copy failed!\n");
		return;
	}
	pr_info("buffer coherent sg copy passed!\n");
	pr_info("dma_src_coherent is %s\n", dma_src_coherent);
	pr_info("dma_dst_coherent is %s\n", dma_dst_coherent);
	complete(&dma_m2m_ok);
}

static ssize_t sdma_write(struct file *filp, const char __user *buf, size_t count,
		loff_t *offset)
{
	unsigned int *index1, *index2, *index3, i;
	struct dma_async_tx_descriptor *dma_m2m_desc;
	struct dma_device *dma_dev;
	dma_dev = dma_m2m_chan->device;

	pr_info("sdma_write is called.\n");

	index1 = wbuf;
	index2 = wbuf2;
	index3 = wbuf3;

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		*(index1 + i) = 0x12345678;
	}

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		*(index2 + i) = 0x87654321;
	}

	for (i=0; i<SDMA_BUF_SIZE/4; i++) {
		*(index3 + i) = 0xabcde012;
	}

	init_completion(&dma_m2m_ok);

	if (copy_from_user(dma_src_coherent, buf, count)) {
		return -EFAULT;
	}

	pr_info ("The string is %s\n", dma_src_coherent);

	sg_init_table(sg, 3);
	sg_set_buf(&sg[0], wbuf, SDMA_BUF_SIZE);
	sg_set_buf(&sg[1], wbuf2, SDMA_BUF_SIZE);
	sg_set_buf(&sg[2], wbuf3, SDMA_BUF_SIZE);
	dma_map_sg(dma_dev->dev, sg, 3, DMA_TO_DEVICE);

	sg_init_table(sg2, 3);
	sg_set_buf(&sg2[0], rbuf, SDMA_BUF_SIZE);
	sg_set_buf(&sg2[1], rbuf2, SDMA_BUF_SIZE);
	sg_set_buf(&sg2[2], rbuf3, SDMA_BUF_SIZE);
	dma_map_sg(dma_dev->dev, sg2, 3, DMA_FROM_DEVICE);

	sg_init_table(sg3, 1);
	sg_set_buf(sg3, dma_src_coherent, SDMA_BUF_SIZE);
	dma_map_sg(dma_dev->dev, sg3, 1, DMA_TO_DEVICE);
	sg_init_table(sg4, 1);
	sg_set_buf(sg4, dma_dst_coherent, SDMA_BUF_SIZE);
	dma_map_sg(dma_dev->dev, sg4, 1, DMA_FROM_DEVICE);

	dma_m2m_desc = dma_dev->device_prep_dma_sg(dma_m2m_chan,sg2, 3, sg, 3, 0);

	dma_m2m_desc->callback = dma_sg_callback;
	dmaengine_submit(dma_m2m_desc);
	dma_async_issue_pending(dma_m2m_chan);
	wait_for_completion(&dma_m2m_ok);
	dma_unmap_sg(dma_dev->dev, sg, 3, DMA_TO_DEVICE);
	dma_unmap_sg(dma_dev->dev, sg2, 3, DMA_FROM_DEVICE);

	reinit_completion(&dma_m2m_ok);
	dma_m2m_desc = dma_dev->device_prep_dma_sg(dma_m2m_chan, sg4, 1, sg3, 1, 0);

	dma_m2m_desc->callback = dma_m2m_callback;
	dmaengine_submit(dma_m2m_desc);
	dma_async_issue_pending(dma_m2m_chan);
	wait_for_completion(&dma_m2m_ok);
	dma_unmap_sg(dma_dev->dev, sg3, 1, DMA_TO_DEVICE);
	dma_unmap_sg(dma_dev->dev, sg4, 1, DMA_FROM_DEVICE);

	return count;
}

struct file_operations dma_fops = {
	open: sdma_open,
	release: sdma_release,
	write: sdma_write,
};

static struct miscdevice dma_miscdevice = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "sdma_test",
	.fops = &dma_fops,
};

static int my_probe(struct platform_device *pdev)
{
	int retval;
	dma_cap_mask_t dma_m2m_mask;
	struct imx_dma_data m2m_dma_data = {0};
	struct dma_slave_config dma_m2m_config = {0};

	pr_info("platform_probe enter\n");
	retval = misc_register(&dma_miscdevice);
	if (retval)
		return retval; 

	pr_info("mydev: got minor %i\n",dma_miscdevice.minor);

	wbuf = devm_kzalloc(&pdev->dev, SDMA_BUF_SIZE, GFP_KERNEL);
	if (!wbuf) {
		pr_info("error wbuf !\n");
		return -ENOMEM;
	}

	wbuf2 = devm_kzalloc(&pdev->dev, SDMA_BUF_SIZE, GFP_KERNEL);
	if (!wbuf2) {
		pr_info("error wbuf !\n");
		return -ENOMEM;
	}

	wbuf3 = devm_kzalloc(&pdev->dev, SDMA_BUF_SIZE, GFP_KERNEL);
	if (!wbuf3) {
		pr_info("error wbuf2 !\n");
		return -ENOMEM;
	}

	rbuf = devm_kzalloc(&pdev->dev, SDMA_BUF_SIZE, GFP_KERNEL);
	if (!rbuf) {
		pr_info("error rbuf !\n");
		return -ENOMEM;
	}

	rbuf2 = devm_kzalloc(&pdev->dev, SDMA_BUF_SIZE, GFP_KERNEL);
	if (!rbuf2) {
		pr_info("error rbuf2 !\n");
		return -ENOMEM;
	}

	rbuf3 = devm_kzalloc(&pdev->dev, SDMA_BUF_SIZE, GFP_KERNEL);
	if (!rbuf3) {
		pr_info("error rbuf2 !\n");
		return -ENOMEM;
	}

	dma_dst_coherent = dma_alloc_coherent(&pdev->dev, SDMA_BUF_SIZE,
			&dma_dst, GFP_DMA);
	if (dma_dst_coherent == NULL) {
		pr_err("dma_alloc_coherent failed\n");
		return -ENOMEM;
	}

	dma_src_coherent = dma_alloc_coherent(&pdev->dev, SDMA_BUF_SIZE,
			&dma_src, GFP_DMA);
	if (dma_src_coherent == NULL) {
		dma_free_coherent(&pdev->dev, SDMA_BUF_SIZE,
				dma_dst_coherent, dma_dst);
		pr_err("dma_alloc_coherent failed\n");
		return -ENOMEM;
	}

	dma_cap_zero(dma_m2m_mask);
	dma_cap_set(DMA_MEMCPY, dma_m2m_mask);
	m2m_dma_data.peripheral_type = IMX_DMATYPE_MEMORY;
	m2m_dma_data.priority = DMA_PRIO_HIGH;

	dma_m2m_chan = dma_request_channel(dma_m2m_mask, dma_m2m_filter, &m2m_dma_data);
	if (!dma_m2m_chan) {
		pr_err("Error opening the SDMA memory to memory channel\n");
		return -EINVAL;
	}

	dma_m2m_config.direction = DMA_MEM_TO_MEM;
	dma_m2m_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	dmaengine_slave_config(dma_m2m_chan, &dma_m2m_config);

	return 0;
}

static int __exit my_remove(struct platform_device *pdev)
{
	misc_deregister(&dma_miscdevice);
	dma_release_channel(dma_m2m_chan);
	dma_free_coherent(&pdev->dev, SDMA_BUF_SIZE,
			dma_dst_coherent, dma_dst);
	dma_free_coherent(&pdev->dev, SDMA_BUF_SIZE,
			dma_src_coherent, dma_src);
	pr_info("platform_remove exit\n");
	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,hellokeys"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.driver = {
		.name = "hellokeys",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

static int __init demo_init(void)
{
	int ret_val;

	ret_val = platform_driver_register(&my_platform_driver);
	if (ret_val != 0) {
		pr_err("platform value returned %d\n", ret_val);
		return ret_val;
	}

	return 0;
}

static void __exit demo_exit(void)
{
	platform_driver_unregister(&my_platform_driver);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a SDMA scatter/gather memory to memory driver");
