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
#include <asm/uaccess.h>	


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
static struct device *chdev_device;
static unsigned char chdev_buff[4];

static void __exit chdev_exit(void);
static int __init chdev_init(void);
static int chdev_open(struct inode *inode, struct file *file);
static ssize_t chdev_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
static ssize_t chdev_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
static int chdev_close(struct inode *inode, struct file *file);
static void chdev_setup_cdev(struct cdev *temp,int index);

module_param(chdev_major,int,S_IRUGO);
module_init(chdev_init);
module_exit(chdev_exit);

MODULE_AUTHOR("Bear  965006619@qq.com");
MODULE_DESCRIPTION("prj 6");
MODULE_LICENSE("GPL");

struct file_operations chdev_fops =
{
	.owner = THIS_MODULE,
	.open = chdev_open,
	.read = chdev_read,
	.write = chdev_write,
	.release = chdev_close
};


static int chdev_open(struct inode *inode, struct file *file)
{
	PDEBUG("open\n");	
	return 0;
}

static ssize_t chdev_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	unsigned long ret;
	PDEBUG("read\n");	
	ret = copy_to_user(buff,chdev_buff,4);
	if( 0 > ret )
	{
		PDEBUG("copy fail %lu data\n",ret);
	}
	return 0;
}

static ssize_t chdev_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	unsigned long ret;
	PDEBUG("write");
	ret = copy_from_user(chdev_buff,buff,4);   
	if( 0 > ret )
	{ 
		PDEBUG("copy fail %lu data\n",ret);
	}
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
	int i;
	dev_t dev = 0;

	PDEBUG("init\n");	
	if(chdev_major)
	{
		dev = MKDEV(chdev_major,chdev_minor);
		ret = register_chrdev_region(dev,chdev_nr_devs,"chdev");
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

	PDEBUG("major %d \n",chdev_major);
	chdev_cdev = cdev_alloc();
	chdev_cdev->ops = &chdev_fops;
	chdev_cdev->owner = THIS_MODULE;
	for( i = 0 ; i < chdev_nr_devs ; i++ )
		chdev_setup_cdev(chdev_cdev,chdev_minor + i);	

	chdev_class = class_create(THIS_MODULE, "chdev");
	chdev_device = device_create(chdev_class, NULL, MKDEV(chdev_major, 0), NULL, "chdev");
 
	
	return 0;

ERR:
	chdev_exit();
	return 1;
}

static void __exit chdev_exit(void)
{
	dev_t dev = MKDEV(chdev_major,chdev_minor);
	printk("chdev_exit\n");	

	device_unregister(chdev_device);
	class_destroy(chdev_class);

	cdev_del(chdev_cdev);
	unregister_chrdev_region(dev,chdev_nr_devs);	
}


static void chdev_setup_cdev(struct cdev *temp,int index)
{
	dev_t devno;
	int ret;
	devno = MKDEV(chdev_major,index);
	PDEBUG("devno %u \n",devno);	
	PDEBUG("index %d \n",index);
	
	ret = cdev_add (chdev_cdev, devno, 1);	
	if(ret < 0)
	{
		printk("cdev add fail\n");	
	}
}


