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

static int chdev_major = 200;// major dev number
//static int chdev_minor = 0;// minor dev number
static int chdev_nr_devs = 1;//  
static struct class *chdev_class;
static struct device *chdev_device;


// insmod .ko afferent parameters
module_param(chdev_major,int,S_IRUGO);

static void __exit chdev_exit(void);
static int __init chdev_init(void);
static int chdev_open(struct inode *inode, struct file *file);
static int chdev_close(struct inode *inode, struct file *file);

struct file_operations chdev_fops =
{
	.owner = THIS_MODULE,
	.open = chdev_open,
	.release = chdev_close
};


static int chdev_open(struct inode *inode, struct file *file)
{
	printk("chdev open\n");	
	return 0;
}

static int chdev_close(struct inode *inode, struct file *file)
{
	printk("chdev close\n");	
	return 0;
}

static int __init chdev_init(void)
{
	int ret;
	dev_t dev = 0;

	printk("chdev init\n");	
	if(chdev_major)
	{
		ret = register_chrdev(chdev_major,"chdev",&chdev_fops); 
	}
	else
	{
		ret = alloc_chrdev_region(&dev,0,chdev_nr_devs,"chdev");
		chdev_major = MAJOR(dev);
	}
	if(ret < 0)
	{
		printk("can't register major number\n");	
		goto ERR;			
	}

	chdev_class = class_create(THIS_MODULE, "chdev");
	chdev_device = device_create(chdev_class, NULL, MKDEV(chdev_major, 0), NULL, "chdev");
 
	printk("chdev inited major %d \n",chdev_major);	
	return 0;

ERR:
	chdev_exit();
	return 1;
}

static void __exit chdev_exit(void)
{
	printk("chdev_exit\n");	

	device_unregister(chdev_device);
	class_destroy(chdev_class);

	unregister_chrdev(chdev_major,"chdev");
}

module_init(chdev_init);
module_exit(chdev_exit);

MODULE_AUTHOR("Bear  965006619@qq.com");
MODULE_DESCRIPTION("4.open close dev");
MODULE_LICENSE("GPL");


