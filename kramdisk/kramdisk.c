#include <linux/init.h>
#include <linux/module.h>

#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

#include <linux/spinlock.h>

MODULE_LICENSE("GPL");

#define MODULE_NAME "kramdisk"

#define MODULE_MINORS_K 1 // TODO #1: extend to generic count

#define NSECTORS 512 // TODO #3: add as parameter

#define SECTORSIZE 0 // TODO #2: need to support custom sector size
#define KERNEL_SECTOR_SIZE 512

static int major_num = 0;

struct kramdisk_dev {
	size_t size;
	char *data;
	spinlock_t lock;
	struct request_queue *queue;
	struct gendisk *gd;
};

static struct kramdisk_dev *dev;

static int kramdisk_getgeo(struct block_device *blk_dev, struct hd_geometry *geo) {
	size_t size;

	/* We have no real geometry, of course, so make something up. */
	// FIXME: not use global var
	size = dev->size;
	geo->cylinders = (size & ~0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
	
	return 0;
}

static void _kramdisk_request(...) {
	
}

static void kramdisk_request(struct request_queue *queue) {
	
}

static struct block_device_operations kramdisk_fops = {
	.owner = THIS_MODULE,
	.getgeo = kramdisk_getgeo
};

static int kramdisk_init(void) {
	int ret = 0;
	
	major_num = register_blkdev(major_num, MODULE_NAME);
	if (major_num <= 0) {
		printk(KERN_ALERT "Can't allocate major number\n");
		return -EBUSY;
	}
	printk(KERN_ALERT "majornum acquired\n");
	
	dev = kmalloc(sizeof(struct kramdisk_dev), GFP_KERNEL);
	if (!dev) {
		printk(KERN_ALERT "Can't allocate kmem for device\n");
		goto out_unregister;
	}
	printk(KERN_ALERT "mem allocated\n");
	
	memset(dev, 0, sizeof(struct kramdisk_dev));
	dev->size = NSECTORS * KERNEL_SECTOR_SIZE;
	dev->data = vmalloc(dev->size);
	if (!dev->data) {
		printk(KERN_ALERT "Can't allocate vmem for data\n");
		ret = -ENOMEM;
		goto out_vmem;
	}
	spin_lock_init(&dev->lock);
	
	dev->queue = blk_init_queue(kramdisk_request, &dev->lock);
	if (!dev->queue) {
		printk(KERN_ALERT "Can't allcoate memory for queue\n");
		ret = -ENOMEM;
		goto out_devqueue;
	}
	blk_queue_physical_block_size(dev->queue, PAGE_SIZE);
	blk_queue_logical_block_size(dev->queue, PAGE_SIZE);
	// To support custom hardware sectorsize
	// blk_queue_hardsect_size(dev->queue, SECTORSIZE);
	
	dev->gd = alloc_disk(MODULE_MINORS_K);
	if (!dev->gd) {
		printk(KERN_ALERT "Can't allocate disk\n");
		ret = -ENOMEM;
		goto out_devgd;
	}
	
	dev->gd->major = major_num;
	dev->gd->first_minor = 1; // TODO #1
	dev->gd->fops = &kramdisk_fops;
	dev->gd->queue = dev->queue;
	dev->gd->private_data = dev;
	snprintf(dev->gd->disk_name, 32, "%s%c", MODULE_NAME, 0); // TODO #1
	set_capacity(dev->gd, NSECTORS * KERNEL_SECTOR_SIZE /*(SECTORSIZE / KERNEL_SECTOR_SIZE)*/); // TODO #2
	
	// This should be last call in init function
	add_disk(dev->gd);
	printk(KERN_ALERT "kramdisk is initialized\n");
	
	return ret;
		
out_devgd:
	blk_cleanup_queue(dev->queue);
	
out_devqueue:
	vfree(dev->data);
	
out_vmem:
	kfree(dev);
	
out_unregister:
	unregister_blkdev(major_num, MODULE_NAME);
	return ret;
}

static void kramdisk_exit(void) {
	del_gendisk(dev->gd);
	put_disk(dev->gd);
	unregister_blkdev(major_num, MODULE_NAME);
	blk_cleanup_queue(dev->queue);
	vfree(dev->data);
	printk(KERN_ALERT "kramdisk is freed\n");
}

module_init(kramdisk_init);
module_exit(kramdisk_exit);
