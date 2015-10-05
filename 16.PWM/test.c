#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>   
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>  
#include "pwm.h"

void usage(char *argv[])
{
	printf("usage:\n");
	printf("%s | dev\n",argv[0]);
}

int main(int argc,char *argv[])
{
	int fd,ret,timer,freq,duty;
    int key_value[5];
	if(argc != 2)
	{
		usage(argv);
		exit(1);	
	}
	fd = open(argv[1],0);
	//fd = open("/dev/chdev",O_RDWR);
	if(fd < 0)
	{
		//printf("open failed. \n");
		perror("open fail");
		exit(1);
	}	
    while(1)
    {
		scanf("%d %d %d",&timer,&freq,&duty);
        switch(timer)
        {
        case 0:
            printf("case 0:\n");
            ioctl(fd,PWM_IOC_SET_TIME0_FREQ,freq);
            ioctl(fd,PWM_IOC_SET_TIME0_DUTY,duty);
            break;
        case 1:
            printf("case 1:\n");
            ioctl(fd,PWM_IOC_SET_TIME1_FREQ,freq);
            ioctl(fd,PWM_IOC_SET_TIME1_DUTY,duty);
            break;
        case 2:
            printf("case 2:\n");
            ioctl(fd,PWM_IOC_SET_TIME2_FREQ,freq);
            ioctl(fd,PWM_IOC_SET_TIME2_DUTY,duty);
            break;
        case 3:
            printf("case 3:\n");
            ioctl(fd,PWM_IOC_SET_TIME3_FREQ,freq);
            ioctl(fd,PWM_IOC_SET_TIME3_DUTY,duty);
            break;
        default:break;
        }
    }
    
	close(fd);	
}


