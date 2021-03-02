#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/init.h>			/* Needed for macros	*/
#include <linux/blkdev.h>
#include <linux/vmalloc.h>

struct ramdisk_dev {
	int size;	/*	size in sectors to form the disk	*/
	u8 *data;
	short users; /* How many users */
	short media_change; /* Flag a media change? */
	spinlock_t lock; /* For mutual exclusion */
	struct request_queue *queue; /* The device request queue */
	struct gendisk *gd; /* The gendisk structure */
};

static int ramdisk_major = 0;
struct ramdisk_dev *dev;
static unsigned int req = 0;

#define MAX_SECTORS 20000
#define SECT_SIZE	512
#define KERNEL_SECTOR_SHIFT 9
#define KERNEL_SECTOR_SIZE (1 << KERNEL_SECTOR_SHIFT)

static blk_qc_t ramdisk_req_fn(struct request_queue *q, struct bio *bio)
{
	/*	Fix the request queue handling	*/
	//struct ramdrv_dev *dev = q->queuedata;
	//int status;

	//status = ramdrv_xfer_bio(dev, bio);
	pr_emerg("%s: %d REQ served\n", __func__, ++req);
 	bio_endio(bio);

	return BLK_QC_T_NONE;
}

static int ramdisk_open(struct block_device *device, fmode_t mode)
{
	struct ramdisk_dev *dev = device->bd_disk->private_data;
	spin_lock(&dev->lock);
	dev->users++;
	spin_unlock(&dev->lock);
	pr_emerg("RAM disk opened\n");
	return 0;
}

static void ramdisk_release(struct gendisk *disk, fmode_t mode)
{
	struct ramdisk_dev *dev = disk->private_data;
	spin_lock(&dev->lock);
	dev->users--;
	spin_unlock(&dev->lock);
	pr_emerg("RAM disk closed\n");
}

static struct block_device_operations ramdisk_ops = {
	.owner = THIS_MODULE,
	.open = ramdisk_open,
	.release = ramdisk_release,
	.ioctl = NULL
};

static int __init init_ramdisk(void)
{
	int nsectors = MAX_SECTORS;
	int hardsect_size = KERNEL_SECTOR_SIZE;
	pr_emerg("Initializing RAM disk\n");

	dev = kmalloc(sizeof(struct ramdisk_dev), GFP_KERNEL);
	if (NULL == dev) {
		pr_emerg("kmalloc failed\n");
		return -ENOMEM;
	}
	memset(dev, 0, sizeof(struct ramdisk_dev));

	ramdisk_major = register_blkdev(ramdisk_major, "ramdisk");
	if (ramdisk_major <= 0) {
		pr_emerg("Error in getting the major number: %d\n", ramdisk_major);
		return -EBUSY;
	}
	
	spin_lock_init(&dev->lock);
	dev->size = nsectors * hardsect_size;
	dev->data = vmalloc(dev->size);
	if (NULL == dev->data)	{
		pr_emerg("Unable to allocate memory\n");
		return -ENOMEM;
	}

	dev->queue = blk_alloc_queue(GFP_KERNEL);
	blk_queue_make_request(dev->queue, ramdisk_req_fn);
	blk_queue_logical_block_size(dev->queue, hardsect_size);
	blk_queue_physical_block_size(dev->queue, hardsect_size);
	dev->queue->queuedata = dev;

	dev->gd = alloc_disk(1);
	if (NULL == dev->gd) {
        pr_emerg("Unable to allocate gendisk\n");
        return -ENOMEM;
	}
	pr_emerg("%s: %d\n", __func__, __LINE__);
	dev->gd->major = ramdisk_major;
	dev->gd->first_minor = 0;
	dev->gd->queue = dev->queue;
	dev->gd->fops = &ramdisk_ops;
	dev->gd->private_data = (void *)dev;
	snprintf (dev->gd->disk_name, 32, "ramdisk0");
	set_capacity(dev->gd, nsectors);
	add_disk(dev->gd);

	return 0;
}

static void __exit exit_ramdisk(void)
{
	pr_emerg("Exiting RAMDISK!\n");
	if (dev->gd) {
		del_gendisk(dev->gd);
		put_disk(dev->gd);
	}
	if (dev->queue) {
		blk_cleanup_queue(dev->queue);
	}
	if (dev->data) {
		vfree(dev->data);
	}
	kfree(dev);
	if (ramdisk_major > 0) {
		unregister_blkdev(ramdisk_major, "ramdisk");
	}
}

module_init(init_ramdisk);
module_exit(exit_ramdisk);

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("Vikas Siddhabhaktula"); 
MODULE_DESCRIPTION("Block device driver to create and use RAM disk"); 
MODULE_VERSION("1");
