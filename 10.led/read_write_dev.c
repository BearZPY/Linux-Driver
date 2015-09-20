#include <linux/init.h>		//module_init
#include <linux/module.h>	//MODULE_LICENSE
#include <linux/moduleparam.h>  //module_param
#include <linux/fs.h>		//register_chrdev_region	
#include <linux/types.h>	//MKDEV
#include <linux/cdev.h>     //cdev_alloc
#include <linux/device.h>   //device_create
#include <linux/stat.h>		//module_param S_IRUGO
#include <linux/fcntl.h>	//O_RDONLY
#include <linux/kernel.h>	//container_of(pointer, type, field);
#include <asm/uaccess.h>	// copy_from_user
#include <linux/proc_fs.h>	// /proc create_proc_read_entry
#include <linux/kdev_t.h>   //print_dev_t
#include <linux/errno.h>	// error codes
#include <linux/mutex.h>  	//mutex_init
#include <linux/spinlock.h>
#include <linux/ioctl.h>

#include <linux/io.h>	//ioremap
//#include <asm/atomic.h>
//#include <asm/unistd.h>

#include "led.h"

static int led_major = 0;// major dev number
static int led_minor = 0;// minor dev number
static int led_nr_devs = 4;//  
static struct cdev *led_cdev;  
static struct class *led_class;
static struct device *led_device;
static struct mutex led_sem;
static unsigned char led_buff[LED_BUFF_LEN];
volatile unsigned long *GPBCON, *GPBDAT;
//static spinlock_t led_spinlock = SPIN_LOCK_UNLOCKED;

static void __exit led_exit(void);
static int __init led_init(void);
static int led_open(struct inode *inode, struct file *file);
static ssize_t led_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
static int led_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data); 
static ssize_t led_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
static long led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int led_close(struct inode *inode, struct file *file);
static void led_setup_cdev(struct cdev *temp,int index);

module_param(led_major,int,S_IRUGO);
module_init(led_init);
module_exit(led_exit);

MODULE_AUTHOR("Bear  965006619@qq.com");
MODULE_DESCRIPTION("prj 10 LED");
MODULE_LICENSE("GPL");

struct file_operations led_fops =
{
	.owner = THIS_MODULE,
	.open = led_open,
	.read = led_read,
	.write = led_write,
	.release = led_close,
	.unlocked_ioctl = led_ioctl
};


static int led_open(struct inode *inode, struct file *file)
{
	PDEBUG("open\n");	
     (*GPBCON) |= 1<<2;
	return 0;
}

static ssize_t led_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	ssize_t retval = 0;
	unsigned long temp;
	PDEBUG("read\n");	
	if (mutex_lock_interruptible(&led_sem))
	{
		PDEBUG("Mutex Lock\n");
		return -ERESTARTSYS;
	}
	
	if(count > LED_BUFF_LEN)
	{
		PDEBUG("invalid parameter\n");
		retval = -EINVAL;
		goto out;
	}
	temp = *GPBDAT;
	temp &= 1<<1;
	temp = temp>>1; 
	led_buff[0] = temp;
	if(copy_to_user(buff,led_buff,count))
	{
		PDEBUG("copy fail data\n");
		retval = -EFAULT;
		goto out;
	}
	
	retval = count;
	
out:
	mutex_unlock(&led_sem);
	return retval;
}

static int led_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    int len = 0;
    PDEBUG("read /proc/led \r\n");
	len += sprintf(page + len, "led_buff[0]: %d\r\n",led_buff[0]);
	len += sprintf(page + len, "led_buff[1]: %d\r\n",led_buff[1]);
	len += sprintf(page + len, "led_buff[2]: %d\r\n",led_buff[2]);
	len += sprintf(page + len, "led_buff[3]: %d\r\n",led_buff[3]);
	
    *eof = 1;
    return len;
}

static ssize_t led_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */
	PDEBUG("write");
	if(mutex_lock_interruptible(&led_sem))
	{
		PDEBUG("Mutex Lock\n");
		return -ERESTARTSYS;
	}
	if(count > LED_BUFF_LEN)
	{
		PDEBUG("invalid parameter\n");
		retval = -EINVAL;
		goto out;
	}
	if(copy_from_user(led_buff,buff,count))
	{ 
		PDEBUG("copy fail data\n");
		retval = -EFAULT;
		goto out;
	}
	if(led_buff[0] == 0)
		*GPBDAT &= ~(1 << 1);
	else
		*GPBDAT |= 1 << 1;
	retval = count;

out:
	mutex_unlock(&led_sem);
	return retval;
}

static long led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	long retval = 0;
	unsigned char temp;
	PDEBUG("ioctl");
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != LED_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > LED_IOC_MAXNR) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;
	
	switch(cmd)
	{
	case LED_IOC_RESET:
		//sprintf(led_buff,"%c%c%c%c",0,0,0,0);
		led_buff[0] = led_buff[1] = led_buff[2] = led_buff[3] = 0x00;
		break;
	case LED_IOC_ROL:
		temp = led_buff[0];
		led_buff[0] = led_buff[1];
		led_buff[1] = led_buff[2];
		led_buff[2] = led_buff[3];
		led_buff[3] = temp;
		break;
	case LED_IOC_ROR:
		temp = led_buff[3];
		led_buff[3] = led_buff[2];
		led_buff[2] = led_buff[1];
		led_buff[1] = led_buff[0];
		led_buff[0] = temp;
		break;
	default:
		return -ENOTTY;
	}
	return retval;
}

static int led_close(struct inode *inode, struct file *file)
{
	PDEBUG("close\n");	
	return 0;
}

static int __init led_init(void)
{
	int ret;
	int i;
	dev_t dev = 0;

	PDEBUG("init\n");	
	if(led_major)
	{
		dev = MKDEV(led_major,led_minor);
		ret = register_chrdev_region(dev,led_nr_devs,"led");
	}
	else
	{
		ret = alloc_chrdev_region(&dev,0,led_nr_devs,"led");
		led_major = MAJOR(dev);
	}
	if(ret < 0)
	{
		printk("can't register major number\n");	
		goto ERR;			
	}

	PDEBUG("major %d \n",led_major);
	
	PDEBUG("mutex create \n");
	mutex_init(&led_sem);
	
	PDEBUG("cdev create\n");
	led_cdev = cdev_alloc();
	led_cdev->ops = &led_fops;
	led_cdev->owner = THIS_MODULE;
	for( i = 0 ; i < led_nr_devs ; i++ )
		led_setup_cdev(led_cdev,led_minor + i);	
	
	PDEBUG("class create \n");
	led_class = class_create(THIS_MODULE, "led");
	PDEBUG("device create \n");
	led_device = device_create(led_class, NULL, MKDEV(led_major, 0), NULL, "led");
	PDEBUG("proc create \n");
	create_proc_read_entry("led",0/* default mode */,
						NULL /* parent dir */, led_read_proc,
						NULL /* client data */);

	GPBCON = (volatile unsigned long*)ioremap(0x56000010,8);
    GPBDAT = GPBCON + 1;
	return 0;

ERR:
	led_exit();
	return 1;
}

static void __exit led_exit(void)
{
	dev_t dev = MKDEV(led_major,led_minor);
	iounmap(GPBCON);
	remove_proc_entry("led",NULL);
	printk("led_exit\n");	

	device_unregister(led_device);
	class_destroy(led_class);

	cdev_del(led_cdev);
	unregister_chrdev_region(dev,led_nr_devs);	
}


static void led_setup_cdev(struct cdev *temp,int index)
{
	dev_t devno;
	int ret;
	devno = MKDEV(led_major,index);
	
	ret = cdev_add (led_cdev, devno, 1);	
	if(ret < 0)
	{
		printk("cdev add fail\n");	
	}
}



