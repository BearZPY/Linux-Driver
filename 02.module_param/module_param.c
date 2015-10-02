#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

static char *whom = "Bear";
static int times = 1;
module_param(times,int,S_IRUGO);
module_param(whom,charp,S_IRUGO);
//module_param_array();

static int module_param_init(void)
{
	unsigned char i;
	for(i = 0 ; i < times ; i++ )
		printk(KERN_ALERT "module_param_init %d by %s \n\r",i,whom);
	return 0;
}

static void module_param_exit(void)
{
	printk("module_param_exit\n\r");
	//return 0;
}

module_init(module_param_init);
module_exit(module_param_exit);

MODULE_LICENSE("GPL");
