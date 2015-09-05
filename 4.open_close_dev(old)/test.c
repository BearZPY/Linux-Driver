#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>   
#include <fcntl.h>
#include <stdlib.h>

void usage(char *argv[])
{
	printf("usage:\n");
	printf("%s | dev\n",argv[0]);
}

int main(int argc,char *argv[])
{
	int fd;
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
	close(fd);	
}


