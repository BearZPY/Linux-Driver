#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>

struct timer_list timer;						//定义定时器结构体

//超时中断服务子函数
void timeout(unsigned long dat)
{
	printk("Time out!The data is %ld\n",dat);
}

static int __init timer_init(void)  
{
	init_timer(&timer);							//初始化定时器
	timer.data = 5;								//为了验证进入了中断
	timer.expires = jiffies + (5*HZ);			//设置超时时间
	timer.function = timeout;					//设置超时之后中断服务子程序入口
	add_timer(&timer); 							//启动定时器
	return 0;
}


static void __exit timer_exit(void)	
{
	del_timer(&timer);
	printk("exit\n");
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE("GPL");

