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

#include "chdev.h"

static int chdev_major = 0;// major dev number
static int chdev_minor = 0;// minor dev number
static int chdev_nr_devs = 4;//  
static struct cdev *chdev_cdev;  
static struct class *chdev_class;
static struct device *chdev_device;
static struct mutex chdev_sem;
static unsigned char chdev_buff[CHDEV_BUFF_LEN];
//static spinlock_t chdev_spinlock = SPIN_LOCK_UNLOCKED;

static void __exit chdev_exit(void);
static int __init chdev_init(void);
static int chdev_open(struct inode *inode, struct file *file);
static ssize_t chdev_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
static int chdev_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data); 
static ssize_t chdev_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
static long chdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static int chdev_close(struct inode *inode, struct file *file);
static void chdev_setup_cdev(struct cdev *temp,int index);

module_param(chdev_major,int,S_IRUGO);
module_init(chdev_init);
module_exit(chdev_exit);

MODULE_AUTHOR("Bear  965006619@qq.com");
MODULE_DESCRIPTION("prj 9");
MODULE_LICENSE("GPL");

struct file_operations chdev_fops =
{
	.owner = THIS_MODULE,
	.open = chdev_open,
	.read = chdev_read,
	.write = chdev_write,
	.release = chdev_close,
	.unlocked_ioctl = chdev_ioctl
};


static int chdev_open(struct inode *inode, struct file *file)
{
	PDEBUG("open\n");	
	return 0;
}

static ssize_t chdev_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	ssize_t retval = 0;
	PDEBUG("read\n");	
	if (mutex_lock_interruptible(&chdev_sem))
	{
		PDEBUG("Mutex Lock\n");
		return -ERESTARTSYS;
	}
	
	if(count > CHDEV_BUFF_LEN)
	{
		PDEBUG("invalid parameter\n");
		retval = -EINVAL;
		goto out;
	}
	
	if(copy_to_user(buff,chdev_buff,count))
	{
		PDEBUG("copy fail data\n");
		retval = -EFAULT;
		goto out;
	}
	
	retval = count;
	
out:
	mutex_unlock(&chdev_sem);
	return retval;
}

static int chdev_read_proc(char *page, char **start, off_t offset, int count, int *eof, void *data) 
{
    int len = 0;
    PDEBUG("read /proc/chdev \r\n");
	len += sprintf(page + len, "chdev_buff[0]: %d\r\n",chdev_buff[0]);
	len += sprintf(page + len, "chdev_buff[1]: %d\r\n",chdev_buff[1]);
	len += sprintf(page + len, "chdev_buff[2]: %d\r\n",chdev_buff[2]);
	len += sprintf(page + len, "chdev_buff[3]: %d\r\n",chdev_buff[3]);
	
    *eof = 1;
    return len;
}

static ssize_t chdev_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */
	PDEBUG("write");
	if(mutex_lock_interruptible(&chdev_sem))
	{
		PDEBUG("Mutex Lock\n");
		return -ERESTARTSYS;
	}
	if(count > CHDEV_BUFF_LEN)
	{
		PDEBUG("invalid parameter\n");
		retval = -EINVAL;
		goto out;
	}
	if(copy_from_user(chdev_buff,buff,count))
	{ 
		PDEBUG("copy fail data\n");
		retval = -EFAULT;
		goto out;
	}
	retval = count;
	
out:
	mutex_unlock(&chdev_sem);
	return retval;
}

static long chdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	long retval = 0;
	unsigned char temp;
	PDEBUG("ioctl");
	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != CHDEV_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > CHDEV_IOC_MAXNR) return -ENOTTY;

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
	case CHDEV_IOC_RESET:
		//sprintf(chdev_buff,"%c%c%c%c",0,0,0,0);
		chdev_buff[0] = chdev_buff[1] = chdev_buff[2] = chdev_buff[3] = 0x00;
		break;
	case CHDEV_IOC_ROL:
		temp = chdev_buff[0];
		chdev_buff[0] = chdev_buff[1];
		chdev_buff[1] = chdev_buff[2];
		chdev_buff[2] = chdev_buff[3];
		chdev_buff[3] = temp;
		break;
	case CHDEV_IOC_ROR:
		temp = chdev_buff[3];
		chdev_buff[3] = chdev_buff[2];
		chdev_buff[2] = chdev_buff[1];
		chdev_buff[1] = chdev_buff[0];
		chdev_buff[0] = temp;
		break;
	default:
		return -ENOTTY;
	}
	return retval;
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
	
	PDEBUG("mutex create \n");
	mutex_init(&chdev_sem);
	
	PDEBUG("cdev create\n");
	chdev_cdev = cdev_alloc();
	chdev_cdev->ops = &chdev_fops;
	chdev_cdev->owner = THIS_MODULE;
	for( i = 0 ; i < chdev_nr_devs ; i++ )
		chdev_setup_cdev(chdev_cdev,chdev_minor + i);	
	
	PDEBUG("class create \n");
	chdev_class = class_create(THIS_MODULE, "chdev");
	PDEBUG("device create \n");
	chdev_device = device_create(chdev_class, NULL, MKDEV(chdev_major, 0), NULL, "chdev");
	PDEBUG("proc create \n");
	create_proc_read_entry("chdev",0/* default mode */,
						NULL /* parent dir */, chdev_read_proc,
						NULL /* client data */);

	return 0;

ERR:
	chdev_exit();
	return 1;
}

static void __exit chdev_exit(void)
{
	dev_t dev = MKDEV(chdev_major,chdev_minor);
	remove_proc_entry("chdev",NULL);
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
	
	ret = cdev_add (chdev_cdev, devno, 1);	
	if(ret < 0)
	{
		printk("cdev add fail\n");	
	}
}



