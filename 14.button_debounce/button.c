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
#include <linux/proc_fs.h>	// /proc create_proc_read_entry
#include <linux/kdev_t.h>   //print_dev_t
#include <linux/errno.h>	// error codes
#include <linux/mutex.h>  	//mutex_init
#include <linux/spinlock.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/io.h>	//ioremap
#include <linux/platform_device.h> 
#include <linux/gpio.h> 

#include <asm/irq.h>
#include <asm/uaccess.h>	// copy_from_user

#include <mach/regs-gpio.h>   
#include <mach/hardware.h> 

#define BUTTON_DEBUG
#undef PDEBUG             /* undef it, just in case */
#ifdef BUTTON_DEBUG
    #ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
	#define PDEBUG(fmt,args...)    printk(KERN_EMERG "button: " fmt,## args)
    #else
     /* This one for user space */
        #define PDEBUG(fmt, args…)     fprintf(stderr, fmt, ## args)
    #endif
#else
    #define PDEBUG(fmt, args…) /* not debugging: nothing */
#endif


static int button_major = 0;// major dev number
static int button_minor = 0;// minor dev number
static int button_nr_devs = 5;//  
static struct cdev *button_cdev;  
static struct class *button_class;
static struct device *button_device[5];
static struct timer_list button_timer[5];
static unsigned int button_pin[5] = {S3C2410_GPF(0),S3C2410_GPF(1),S3C2410_GPF(2),
	S3C2410_GPF(3),S3C2410_GPF(4)};

static volatile int key_no[] = {0,1,2,3,4};
static volatile int key_value[] = {0,0,0,0,0};
static volatile int ev_press = 0;
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

module_param(button_major,int,S_IRUGO);

static void __exit button_exit(void);
static int __init button_init(void);
static int button_open(struct inode *inode, struct file *file);
static ssize_t button_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
static int button_close(struct inode *inode, struct file *file);
static void button_press_time(unsigned long dat);


struct file_operations button_fops =
{
	.owner = THIS_MODULE,
	.open = button_open,
	.read = button_read,
	.release = button_close
};

struct button_irq_desc {
    int irq;
    unsigned long flags;
    char *name;
};

/* 用来指定按键所用的外部中断引脚及中断触发方式, 名字 */
static struct button_irq_desc button_irqs [] = {
    {IRQ_EINT0, IRQF_TRIGGER_FALLING, "KEY1"}, /* K1 */
    {IRQ_EINT1, IRQF_TRIGGER_FALLING, "KEY2"}, /* K2 */
    {IRQ_EINT2, IRQF_TRIGGER_FALLING, "KEY3"}, /* K3 */
    {IRQ_EINT3, IRQF_TRIGGER_FALLING, "KEY4"}, /* K4 */
    {IRQ_EINT4, IRQF_TRIGGER_FALLING, "KEY5"}, /* K5 */
};

static void button_press_time(unsigned long dat)
{
	unsigned int pinval;
    PDEBUG("Timer ID is: %ld\r\n",dat);
	if(dat == -1)
		return ;
	pinval = s3c2410_gpio_getpin(button_pin[dat]); 
	PDEBUG("pinval: %d\r\n",pinval);
	if(pinval)
	{
		return ;
	}
	else
	{
		key_value[dat]++;
	}
    ev_press = 1;
    wake_up_interruptible(&button_waitq);
}
static irqreturn_t buttons_interrupt(int irq, void *dev_id)
{
    volatile int *des = (volatile int *) dev_id;
	int ret;
	
	PDEBUG("Dev ID is: %d\r\n",*des);
	button_timer[*des].data = *des;
	ret = mod_timer(&button_timer[*des],jiffies + (2*HZ/100));
//	if (ret != 1)
//		PDEBUG("mod_timer failed");
	return IRQ_RETVAL(IRQ_HANDLED);	
}

static int button_open(struct inode *inode, struct file *file)
{
	int err,i;
	//s3c2410_gpio_getpin(S3C2410_GPF4_EINT4);
    PDEBUG("%d\n",sizeof(button_irqs)/sizeof(button_irqs[0])); 
    for (i = 0;i < sizeof(button_irqs)/sizeof(button_irqs[0]); i++) 
    {
    	init_timer(&button_timer[i]);
		//button_timer[i].expires = jiffies +(2*HZ/100);
		button_timer[i].function = button_press_time;
		button_timer[i].data = -1;
		add_timer(&button_timer[i]);
        // 注册中断处理函数
        err = request_irq(button_irqs[i].irq, buttons_interrupt, button_irqs[i].flags, 
                          button_irqs[i].name, (void *)&key_no[i]);
        if (err)
            break;
    }

    if (err)
    {
        // 释放已经注册的中断
        i--;
        for (; i >= 0; i--)
        {
        	free_irq(button_irqs[i].irq, (void *)&key_no[i]);
			del_timer(&button_timer[i]);
        }
		
        return -EBUSY;
    }
	PDEBUG("open\n");	
	return 0;
}

static ssize_t button_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	ssize_t err;
    PDEBUG("read\n");	
    /* 如果ev_press等于0，休眠 */
    wait_event_interruptible(button_waitq, ev_press);

    /* 执行到这里时，ev_press等于1，将它清0 */
    ev_press = 0;

    /* 将按键状态复制给用户，并清0 */
    err = copy_to_user(buf,(const void *)key_value, count);
    memset((void *)key_value, 0, sizeof(key_value));

    return err ? -EFAULT : 0;
}

static int button_close(struct inode *inode, struct file *file)
{
    int i;
	PDEBUG("close\n");	
    for (i = 0; i < sizeof(button_irqs)/sizeof(button_irqs[0]); i++) 
    {
        // 释放已经注册的中断
        free_irq(button_irqs[i].irq, (void *)&key_no[i]);
		del_timer(&button_timer[i]);
    }
	return 0;
}

static int __init button_init(void)
{
	int ret;
    int i;
	dev_t devno;

	PDEBUG("init\n");	
	if(button_major)
	{
		devno = MKDEV(button_major,button_minor);
		ret = register_chrdev_region(devno,button_nr_devs,"button");
	}
	else
	{
		ret = alloc_chrdev_region(&devno,0,button_nr_devs,"button");
		button_major = MAJOR(devno);
	}
	if(ret < 0)
	{
		PDEBUG("can't register major number\n");	
		goto ERR;			
	}

	button_cdev = cdev_alloc();
	button_cdev->ops = &button_fops;
	button_cdev->owner = THIS_MODULE;

	button_class = class_create(THIS_MODULE, "button");
    
    for(i = 0; i < button_nr_devs; i++)
    {
        cdev_add(button_cdev,MKDEV(button_major, button_minor+i),1);
        button_device[i] = device_create(button_class, NULL, 
                            MKDEV(button_major, button_minor+i), 
                            NULL, "button%d",button_minor+i);
    }
    
	return 0;

ERR:
	button_exit();
	return 1;
}

static void __exit button_exit(void)
{
    int i;
	dev_t dev = MKDEV(button_major,button_minor);
	printk("button_exit\n");	

    for(i = 0; i < button_nr_devs; i++)
    {
        device_unregister(button_device[i]);
    }
    
	class_destroy(button_class);

	cdev_del(button_cdev);
	unregister_chrdev_region(dev,button_nr_devs);	
}

module_init(button_init);
module_exit(button_exit);

MODULE_AUTHOR("Bear  965006619@qq.com");
MODULE_DESCRIPTION("14.button ");
MODULE_LICENSE("GPL");







