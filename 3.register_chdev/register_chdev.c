#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/types.h>

static int chdev_major = 0;// major dev number
static int chdev_minor = 0;// minor dev number
static int chdev_nr_devs = 4;// 

module_param(chdev_major,int,S_IRUGO);

static void __exit chdev_exit(void);


static int __init chdev_init(void)
{
	int ret;
	dev_t dev = 0;
	printk("chdev init\n");	
	if(chdev_major)
	{
		dev = MKDEV(chdev_major,chdev_minor);
		ret = register_chrdev_region(dev,chdev_nr_devs,"chdev");
		//Segmentation fault
		//ret = register_chrdev(chdev_major,"chdev",NULL); 
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
	printk("chdev inited major %d \n",chdev_major);	
	return 0;

ERR:
	chdev_exit();
	return 1;
}

static void __exit chdev_exit(void)
{
	dev_t dev = MKDEV(chdev_major,chdev_minor);
	printk("chdev_exit\n");	
	unregister_chrdev_region(dev,chdev_nr_devs);
	//unregister_chrdev(chdev_major,"chdev");
}

module_init(chdev_init);
module_exit(chdev_exit);

MODULE_LICENSE("GPL");
