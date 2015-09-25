#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/stat.h>
#include <linux/fcntl.h>

#define CHDEV_DEBUG
#undef PDEBUG             /* undef it, just in case */
#ifdef CHDEV_DEBUG
    #ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
	#define PDEBUG(fmt,args...)    printk(KERN_EMERG "chdev: " fmt,## args)
    #else
     /* This one for user space */
        #define PDEBUG(fmt, args…)     fprintf(stderr, fmt, ## args)
    #endif
#else
    #define PDEBUG(fmt, args…) /* not debugging: nothing */
#endif



static int chdev_major = 0;// major dev number
static int chdev_minor = 0;// minor dev number
static int chdev_nr_devs = 4;//  
static struct cdev *chdev_cdev;  
static struct class *chdev_class;
static struct device *chdev_device[4];


// insmod .ko afferent parameters
module_param(chdev_major,int,S_IRUGO);

static void __exit chdev_exit(void);
static int __init chdev_init(void);
static int chdev_open(struct inode *inode, struct file *file);
static ssize_t chdev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static int chdev_close(struct inode *inode, struct file *file);

struct file_operations chdev_fops =
{
	.owner = THIS_MODULE,
	.open = chdev_open,
	.read = chdev_read,
	.release = chdev_close
};


static int chdev_open(struct inode *inode, struct file *file)
{
	PDEBUG("open\n");	
	return 0;
}

static ssize_t chdev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	PDEBUG("read\n");	
	return 0;
}

static int chdev_close(struct inode *inode, struct file *file)
{
	PDEBUG("close\n");	
	return 0;
}

static int __init chdev_init(void)
{
	int ret;
	dev_t dev = 0;
	dev_t devno;

	PDEBUG("init\n");	
	if(chdev_major)
	{
		dev = MKDEV(chdev_major,chdev_minor);
		ret = register_chrdev_region(dev,chdev_nr_devs,"chdev");
		//ret = register_chrdev(chdev_major,"chdev",&chdev_fops); 
	}
	else
	{
		ret = alloc_chrdev_region(&dev,0,chdev_nr_devs,"chdev");
		chdev_major = MAJOR(dev);
	}
	if(ret < 0)
	{
		PDEBUG("can't register major number\n");	
		goto ERR;			
	}

	chdev_cdev = cdev_alloc();
	chdev_cdev->ops = &chdev_fops;
	chdev_cdev->owner = THIS_MODULE;
	//cdev_init(chdev_cdev,&chdev_fops);
	devno = MKDEV(chdev_major,chdev_minor);
	ret = cdev_add(chdev_cdev,devno,1);
	devno = MKDEV(chdev_major,chdev_minor+1);
	ret = cdev_add(chdev_cdev,devno,1);
	devno = MKDEV(chdev_major,chdev_minor+2);
	ret = cdev_add(chdev_cdev,devno,1);
	devno = MKDEV(chdev_major,chdev_minor+3);
	ret = cdev_add(chdev_cdev,devno,1);


	PDEBUG("major %d \n",chdev_major);
	PDEBUG("minor %d \n",chdev_minor);
	PDEBUG("devno %u \n",devno);	

	if(ret < 0)
	{
		PDEBUG("cdev_add fail %d ",ret );	
		goto ERR;			
	}

	chdev_class = class_create(THIS_MODULE, "chdev");
	chdev_device[0] = device_create(chdev_class, NULL, MKDEV(chdev_major, 0), NULL, "chdev0");
    chdev_device[1] = device_create(chdev_class, NULL, MKDEV(chdev_major, 1), NULL, "chdev1");
	chdev_device[2] = device_create(chdev_class, NULL, MKDEV(chdev_major, 2), NULL, "chdev2");
    chdev_device[3] = device_create(chdev_class, NULL, MKDEV(chdev_major, 3), NULL, "chdev3");
	return 0;

ERR:
	chdev_exit();
	return 1;
}

static void __exit chdev_exit(void)
{
	dev_t dev = MKDEV(chdev_major,chdev_minor);
	printk("chdev_exit\n");	

	device_unregister(chdev_device[0]);
    device_unregister(chdev_device[1]);
	device_unregister(chdev_device[2]);
    device_unregister(chdev_device[3]);
	
	class_destroy(chdev_class);

	cdev_del(chdev_cdev);
	unregister_chrdev_region(dev,chdev_nr_devs);	

}

module_init(chdev_init);
module_exit(chdev_exit);

MODULE_AUTHOR("Bear  965006619@qq.com");
MODULE_DESCRIPTION("5.class dev cdev");
MODULE_LICENSE("GPL");


