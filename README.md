# Linux-Driver
----
####Linux-Driver-Example 

####Helper2416 

####Linux 3.2.50

####Ubuntu 12.04 32bit
----
####工程目录
1.first_drv  最简单的驱动

2.module_param insmod 传递参数

3.register_major 注册字符设备号

4.register_class_dev_old  不用cdev注册类设备

5.register_class_dev_cdev  用cdev注册类设备

6.read_write_dev  读写字符设备

7.proc_chdev  在/proc 目录下创建chdev文件 可用cat查看

8.mutex_lock 互斥锁

9.ioctl 互斥锁

10.led  LED驱动和测试程序

11.minor_device_no 次设备号

12.button 按键驱动

####更新历史
* 2015.9.2 添加 1.first_drv 
* 2015.9.3 添加 2.module_param 
* 2015.9.4 添加 3.register_major
* 2015.9.5 添加 4.open_close_dev(old)
* 2015.9.5 修改 部分目录名称
* 2015.9.5 添加 5.register_class_dev_cdev
* 2015.9.5 更新 4.register_class_dev_old 
* 2015.9.5 修改 description
* 2015.9.7 添加 6.read_write_dev
* 2015.9.8 添加 7.proc_chdev  
* 2015.9.12 添加 8.mutex_lock 
* 2015.9.13 添加 9.ioctl
* 2015.9.20 添加 10.led
* 2015.9.24 添加 11.minor_device_no
* 2015.9.25 添加 12.button

####相关链接及反馈

工程获取 ：  https://github.com/BearZPY/Linux-Driver.git

Bug反馈 ： 965006619@qq.com

----
