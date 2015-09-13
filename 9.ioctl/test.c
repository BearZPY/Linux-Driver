#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>   
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "chdev.h"

int fd;
unsigned char read_buff[20];
unsigned char write_buff[20];

void usage(char *argv[]);
void printfbuff(void);
void readbuff(void);
void writebuff(void);

int main(int argc,char *argv[])
{
	
	int i = 0;
	if(argc != 2)
	{
		usage(argv);
		exit(1);	
	}
	fd = open(argv[1],O_RDWR);
	if(fd < 0)
	{
		perror("open fail");
		exit(1);
	}	
	readbuff();
	printfbuff();
	writebuff();
	printfbuff();
	readbuff();
	printfbuff();

	printf("chdev ioctl ror \n");
	if(ioctl(fd,CHDEV_IOC_ROR,NULL) < 0)
		perror("ioctl");
	readbuff();
	printfbuff();
	
	printf("chdev ioctl rol \n");
	if(ioctl(fd,CHDEV_IOC_ROL,NULL) < 0)
		perror("ioctl");
	readbuff();
	printfbuff();
	
	printf("chdev ioctl reset \n");
	if(ioctl(fd,CHDEV_IOC_RESET,NULL) < 0)
		perror("ioctl");
	readbuff();
	printfbuff();

ERR:
	close(fd);	
}

void usage(char *argv[])
{
	printf("usage:\n");
	printf("%s | dev\n",argv[0]);
}

void printfbuff(void)
{
	int i;
	for(i = 0;i < 4; i++)
	{
		printf(" %x ",read_buff[i]);
	}
	printf("\n");
}

void readbuff(void)
{
	printf("Read :\n");
	if(read(fd,read_buff,4) < 0)
	{
		perror("Read fail");
		close(fd);
	}
}

void writebuff(void)
{
	write_buff[0] = 0x01;
	write_buff[1] = 0x02;
	write_buff[2] = 0x03;
	write_buff[3] = 0x04;
	printf("Write :\n");
	if(write(fd,write_buff,4) < 0)
	{
		perror("Read fail");
		close(fd);
	}
}
