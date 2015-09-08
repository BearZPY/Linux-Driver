#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>   
#include <fcntl.h>
#include <stdlib.h>

unsigned char read_buff[20];
unsigned char write_buff[20];

void usage(char *argv[])
{
	printf("usage:\n");
	printf("%s | dev\n",argv[0]);
}

int main(int argc,char *argv[])
{
	int fd;
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
	printf("Read :\n");
	if(read(fd,read_buff,4))
	{
		perror("Read fail");
		goto ERR;
	}
	
	for(i = 0;i < 4; i++)
	{
		printf(" %x ",read_buff[i]);
	}
	printf("\n");
	
	write_buff[0] = 0x01;
	write_buff[1] = 0x02;
	write_buff[2] = 0x03;
	write_buff[3] = 0x04;
	printf("Write :\n");
	if(write(fd,write_buff,4))
	{
		perror("Read fail");
		goto ERR;
	}
	
	printf("Read :\n");
	if(read(fd,read_buff,4))
	{
		perror("Read fail");
		goto ERR;
	}
	
	for(i = 0;i < 4; i++)
	{
		printf(" %x ",read_buff[i]);
	}
	printf("\n");
ERR:
	close(fd);	
}


