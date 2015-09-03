#include <linux/init.h>
#include <linux/module.h>

static int first_drv_init(void)
{
	printk("first_drv_init\n\r");
	return 0;
}

static void first_drv_exit(void)
{
	printk("first_drv_exit\n\r");
	//return 0;
}

module_init(first_drv_init);
module_exit(first_drv_exit);

MODULE_LICENSE("Dual BSD/GPL");
