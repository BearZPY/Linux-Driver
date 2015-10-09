#include <linux/init.h>
#include <linux/module.h>
#include <mach/regs-gpio.h>   
#include <mach/hardware.h> 
#include <linux/gpio.h> 
#include <linux/platform_device.h>



static int led_init(void);
static void led_exit(void);
static int led_probe(struct platform_device *pdev);
static int led_remove(struct platform_device *pdev);

struct platform_device led_device =
{
	.name = "help2416-led",
	.id = 99
};

struct platform_driver led_driver =
{
	.driver = 
	{
		.name = "help2416-led",
		.owner = THIS_MODULE
	},
	.probe = led_probe,
	.remove = led_remove
};

//s3c2410_gpio_cfgpin(S3C2410_GPB(0), S3C2410_GPIO_OUTPUT);
//s3c2410_gpio_setpin(S3C2410_GPB(0), 1); 
static int led_probe(struct platform_device *pdev) 
{
	return 0;
}

static int led_remove(struct platform_device *pdev) 
{
	return 0;
}
static int led_init(void)
{
	printk("led_platform_init\n\r");
	platform_device_register(&led_device); 
	return platform_driver_register(&led_driver);
}

static void led_exit(void)
{
	printk("led_platform_exit\n\r");
	platform_driver_unregister(&led_driver);
	platform_device_unregister(&led_device);
}


module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("prj 18 led_platform");

