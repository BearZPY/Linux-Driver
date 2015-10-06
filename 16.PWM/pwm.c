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
#include <plat/regs-timer.h>
//#include <asm/mach/time.h>
#include <linux/clk.h>

#include "pwm.h"

#define PWM_DEBUG
#undef PDEBUG             /* undef it, just in case */
#ifdef PWM_DEBUG
    #ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
	#define PDEBUG(fmt,args...)    printk(KERN_EMERG "PWM: " fmt,## args)
    #else
     /* This one for user space */
        #define PDEBUG(fmt, args鈥?     fprintf(stderr, fmt, ## args)
    #endif
#else
    #define PDEBUG(fmt, args鈥? /* not debugging: nothing */
#endif


static int PWM_major = 0;// major dev number
static int PWM_minor = 0;// minor dev number
static int PWM_nr_devs = 4;//  
static struct cdev *PWM_cdev;  
static struct class *PWM_class;
static struct device *PWM_device[4];


unsigned long tcon;
unsigned long tcfg0;
unsigned long tcfg1;
unsigned long tcnt[4];
unsigned long tcmp[4];


module_param(PWM_major,int,S_IRUGO);

static void __exit PWM_exit(void);
static int __init PWM_init(void);
static int PWM_open(struct inode *inode, struct file *file);
static int PWM_close(struct inode *inode, struct file *file);
static long PWM_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static void set_timer(unsigned int count,unsigned int dat,unsigned int arg);

struct file_operations PWM_fops =
{
	.owner = THIS_MODULE,
	.open = PWM_open,
	.unlocked_ioctl = PWM_ioctl,
	.release = PWM_close
};


static int PWM_open(struct inode *inode, struct file *file)
{
	struct clk *clk_p;
	unsigned long pclk;
	
	s3c2410_gpio_cfgpin(S3C2410_GPB(0), S3C2410_GPB0_TOUT0);
	s3c2410_gpio_cfgpin(S3C2410_GPB(1), S3C2410_GPB1_TOUT1);
	s3c2410_gpio_cfgpin(S3C2410_GPB(2), S3C2410_GPB2_TOUT2);
	s3c2410_gpio_cfgpin(S3C2410_GPB(3), S3C2410_GPB3_TOUT3);

	tcon = __raw_readl(S3C2410_TCON);
	tcfg0 = __raw_readl(S3C2410_TCFG0); 
	tcfg1 = __raw_readl(S3C2410_TCFG1); 
	
	//设置TCFG0寄存器，prescaler = 50
	//Timer input clock Frequency = PCLK / {prescaler value+1} / {divider value}
	tcfg0 &= ~S3C2410_TCFG_PRESCALER0_MASK; // S3C2410_TCFG_PRESCALER0_MASK 定时器 0 和1 的预分频值的掩码,清除TCFG[0~8]
    tcfg0 |= (100 - 1); // 设置预分频为 50
    tcfg0 &= ~S3C2410_TCFG_PRESCALER1_MASK; // S3C2410_TCFG_PRESCALER0_MASK 定时器 2 3 4 的预分频值的掩码,清除TCFG[0~8]
	tcfg0 |= (100 - 1) << 8; // 设置预分频为 50

	tcfg1 &= ~(S3C2410_TCFG1_MUX0_MASK | S3C2410_TCFG1_MUX1_MASK | S3C2410_TCFG1_MUX2_MASK
				| S3C2410_TCFG1_MUX3_MASK);
	tcfg1 |= S3C2410_TCFG1_MUX0_DIV16 | S3C2410_TCFG1_MUX1_DIV16 | S3C2410_TCFG1_MUX2_DIV16
				| S3C2410_TCFG1_MUX3_DIV16;
	
	__raw_writel(tcfg1, S3C2410_TCFG1); //把 tcfg1 的值写到分割寄存器 S3C2410_TCFG1 中
	__raw_writel(tcfg0, S3C2410_TCFG0); //把 tcfg0 的值写到预分频寄存器 S3C2410_TCFG0 中

	clk_p = clk_get(NULL, "pclk"); //得到 pclk
	pclk  = clk_get_rate(clk_p);
	PDEBUG("plck %ld \n",pclk);
	tcnt[0] = (pclk/100/16)/1;
	tcmp[0] = tcnt[0]/2;
    tcnt[3] = tcnt[2] = tcnt[1] = tcnt[0];
    tcmp[3] = tcmp[2] = tcmp[1] = tcmp[0];
	__raw_writel(tcnt[0], S3C2410_TCNTB(0)); __raw_writel(tcmp[0], S3C2410_TCMPB(0));//tcnt[0]  tcmp[0] 4000
	__raw_writel(tcnt[0], S3C2410_TCNTB(1)); __raw_writel(tcmp[0], S3C2410_TCMPB(1));
	__raw_writel(tcnt[0], S3C2410_TCNTB(2)); __raw_writel(tcmp[0], S3C2410_TCMPB(2));
	__raw_writel(tcnt[0], S3C2410_TCNTB(3)); __raw_writel(tcmp[0], S3C2410_TCMPB(3));

	tcon &= ~(S3C2410_TCON_T0DEADZONE | S3C2410_TCON_T0INVERT);
	tcon |= S3C2410_TCON_T0RELOAD | S3C2410_TCON_T0MANUALUPD |S3C2410_TCON_T0START;
	
	tcon &= ~(S3C2410_TCON_T1INVERT);
	tcon |= S3C2410_TCON_T1RELOAD | S3C2410_TCON_T1MANUALUPD |S3C2410_TCON_T1START;

	tcon &= ~(S3C2410_TCON_T2INVERT);
	tcon |= S3C2410_TCON_T2RELOAD | S3C2410_TCON_T2MANUALUPD |S3C2410_TCON_T2START;

	tcon &= ~(S3C2410_TCON_T3INVERT);
	tcon |= S3C2410_TCON_T3RELOAD | S3C2410_TCON_T3MANUALUPD |S3C2410_TCON_T3START;
	__raw_writel(tcon, S3C2410_TCON);

	tcon &= ~(S3C2410_TCON_T0MANUALUPD | S3C2410_TCON_T1MANUALUPD |
				S3C2410_TCON_T2MANUALUPD | S3C2410_TCON_T3MANUALUPD);
	__raw_writel(tcon, S3C2410_TCON);
	
	return 0;

}

static void set_timer(unsigned int count,unsigned int dat,unsigned int arg)
{
    struct clk *clk_p;
	unsigned long pclk;
    if(dat == 1)
    {
        clk_p = clk_get(NULL, "pclk"); //得到 pclk
        pclk  = clk_get_rate(clk_p);
        tcnt[count] = (pclk/100/16)/arg;
        __raw_writel(tcnt[count], S3C2410_TCNTB(count)); 
        __raw_writel(tcnt[count] >> 1, S3C2410_TCMPB(count));  
        return ;        
    }
    tcmp[count] = tcnt[count] * arg / 100;
    __raw_writel(tcmp[count], S3C2410_TCMPB(count)); 
    
    if(arg >= 100 || arg == 0)
    {
        switch(count)
        {
        case 0: if(!arg)tcon |= S3C2410_TCON_T0INVERT;
                else tcon &= ~(S3C2410_TCON_T0INVERT);
                tcon &= ~(S3C2410_TCON_T0RELOAD); __raw_writel(tcon, S3C2410_TCON);
                break;
        case 1: if(!arg)tcon |= S3C2410_TCON_T1INVERT;
                else tcon &= ~(S3C2410_TCON_T1INVERT);
                tcon &= ~(S3C2410_TCON_T1RELOAD); __raw_writel(tcon, S3C2410_TCON);
                break;
        case 2: if(!arg)tcon |= S3C2410_TCON_T2INVERT;
                else tcon &= ~(S3C2410_TCON_T2INVERT);
                tcon &= ~(S3C2410_TCON_T2RELOAD); __raw_writel(tcon, S3C2410_TCON);
                break;
        case 3: if(!arg)tcon |= S3C2410_TCON_T3INVERT;
                else tcon &= ~(S3C2410_TCON_T3INVERT);
                tcon &= ~(S3C2410_TCON_T3RELOAD); __raw_writel(tcon, S3C2410_TCON);
                break;
        default:break;
        }
    }
    else
    {
        switch(count)
        {
        case 0: tcon &= ~S3C2410_TCON_T0INVERT; 
                tcon |= S3C2410_TCON_T0RELOAD; __raw_writel(tcon, S3C2410_TCON);
                break;
        case 1: tcon &= ~S3C2410_TCON_T1INVERT; 
                tcon |= S3C2410_TCON_T1RELOAD; __raw_writel(tcon, S3C2410_TCON);
                break;
        case 2: tcon &= ~S3C2410_TCON_T2INVERT; 
                tcon |= S3C2410_TCON_T2RELOAD; __raw_writel(tcon, S3C2410_TCON);
                break;
        case 3: tcon &= ~S3C2410_TCON_T3INVERT; 
                tcon |= S3C2410_TCON_T3RELOAD; __raw_writel(tcon, S3C2410_TCON);
                break;
        default:break;
        } 
    }    
    tcon = __raw_readl(S3C2410_TCON);
    PDEBUG("tcon 0x%08lx \n",tcon);
}

static long PWM_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	long retval = 0;
	/*	 
	* extract the type and number bitfields, and don't decode	 
	* wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()	 
	*/	
	if (_IOC_TYPE(cmd) != PWM_IOC_MAGIC) return -ENOTTY;	
	if (_IOC_NR(cmd) > PWM_IOC_MAXNR) return -ENOTTY;	
	/*	 
	* the direction is a bitmask, and VERIFY_WRITE catches R/W	 
	* transfers. `Type' is user-oriented, while	 
	* access_ok is kernel-oriented, so the concept of "read" and	 
	* "write" is reversed	 */	
	if (_IOC_DIR(cmd) & _IOC_READ)		
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));	
	else if (_IOC_DIR(cmd) & _IOC_WRITE)		
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));	
	if (err) 
		return -EFAULT;		
	switch(cmd)	
	{	
	case PWM_IOC_RESET:	
		break;	
	case PWM_IOC_SET_TIME0_FREQ:set_timer(0,1,arg);
		break;	
	case PWM_IOC_SET_TIME0_DUTY:set_timer(0,0,arg);		
		break;	
	case PWM_IOC_SET_TIME1_FREQ:set_timer(1,1,arg);		
		break;	
	case PWM_IOC_SET_TIME1_DUTY:set_timer(1,0,arg);		
		break;		
	case PWM_IOC_SET_TIME2_FREQ:set_timer(2,1,arg);		
		break;	
	case PWM_IOC_SET_TIME2_DUTY:set_timer(2,0,arg);		
		break;		
	case PWM_IOC_SET_TIME3_FREQ:set_timer(3,1,arg);		
		break;	
	case PWM_IOC_SET_TIME3_DUTY:set_timer(3,0,arg);		
		break;		
	default:return -ENOTTY;	
	}	
	return retval;
}

static int PWM_close(struct inode *inode, struct file *file)
{
	s3c2410_gpio_cfgpin(S3C2410_GPB(0), S3C2410_GPIO_OUTPUT);
	s3c2410_gpio_setpin(S3C2410_GPB(0), 1); 
	s3c2410_gpio_cfgpin(S3C2410_GPB(1), S3C2410_GPIO_OUTPUT);
	s3c2410_gpio_setpin(S3C2410_GPB(1), 1); 
	s3c2410_gpio_cfgpin(S3C2410_GPB(2), S3C2410_GPIO_OUTPUT); 
	s3c2410_gpio_setpin(S3C2410_GPB(2), 1); 
	s3c2410_gpio_cfgpin(S3C2410_GPB(3), S3C2410_GPIO_OUTPUT); 
	s3c2410_gpio_setpin(S3C2410_GPB(3), 1); 
	return 0;
}

static int __init PWM_init(void)
{
	int ret;
    int i;
	dev_t devno;

	PDEBUG("init\n");	
	if(PWM_major)
	{
		devno = MKDEV(PWM_major,PWM_minor);
		ret = register_chrdev_region(devno,PWM_nr_devs,"PWM");
	}
	else
	{
		ret = alloc_chrdev_region(&devno,0,PWM_nr_devs,"PWM");
		PWM_major = MAJOR(devno);
	}
	if(ret < 0)
	{
		PDEBUG("can't register major number\n");	
		goto ERR;			
	}

	PWM_cdev = cdev_alloc();
	PWM_cdev->ops = &PWM_fops;
	PWM_cdev->owner = THIS_MODULE;

	PWM_class = class_create(THIS_MODULE, "PWM");
    
    for(i = 0; i < PWM_nr_devs; i++)
    {
        cdev_add(PWM_cdev,MKDEV(PWM_major, PWM_minor+i),1);
        PWM_device[i] = device_create(PWM_class, NULL, 
                            MKDEV(PWM_major, PWM_minor+i), 
                            NULL, "PWM%d",PWM_minor+i);
    }
    
	return 0;

ERR:
	PWM_exit();
	return 1;
}

static void __exit PWM_exit(void)
{
    int i;
	dev_t dev = MKDEV(PWM_major,PWM_minor);
	printk("PWM_exit\n");	

    for(i = 0; i < PWM_nr_devs; i++)
    {
        device_unregister(PWM_device[i]);
    }
    
	class_destroy(PWM_class);

	cdev_del(PWM_cdev);
	unregister_chrdev_region(dev,PWM_nr_devs);	
}

module_init(PWM_init);
module_exit(PWM_exit);

MODULE_AUTHOR("Bear  965006619@qq.com");
MODULE_DESCRIPTION("16.PWM ");
MODULE_LICENSE("GPL");







