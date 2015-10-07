#include <linux/init.h>
#include <linux/module.h>
#include <mach/regs-gpio.h>   
#include <mach/hardware.h> 
#include <linux/gpio.h> 

static int first_drv_init(void)
{
	printk("first_drv_init\n\r");
	s3c2410_gpio_cfgpin(S3C2410_GPB(0), S3C2410_GPIO_OUTPUT);
	s3c2410_gpio_setpin(S3C2410_GPB(0), 1); 
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
