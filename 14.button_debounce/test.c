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
	int fd,ret,i;
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
        ret = read(fd, key_value, sizeof(key_value));
        if (ret < 0) {
            printf("read err!\n");
            continue;
        } 

        for (i = 0; i < sizeof(key_value)/sizeof(key_value[0]); i++) {
            // 如果被按下的次数不为0，打印出来
            if (key_value[i])
                printf("K%d has been pressed %d times!\n", i+1, key_value[i]);
        }
    }
    
	close(fd);	
}


