KERN_DIR = /home/bear/workspace/s3c-linux.jyx

INSTALL_DIR = ./bin
EXP_INSTALL = install -m 755 
EXEC =  ./button.ko
CROSS = arm-linux-
CC = ${CROSS}gcc
OBJS = test.c
TESTC = ./test

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	$(EXP_INSTALL) $(EXEC) $(INSTALL_DIR)
	$(EXP_INSTALL) $(EXEC) /nfsroot 
	$(CC) -o $(TESTC) $(OBJS)
	$(EXP_INSTALL) $(TESTC) $(INSTALL_DIR)
	$(EXP_INSTALL) $(TESTC) /nfsroot 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order *~
	rm -rf *.o $(TESTC)

obj-m	+= button.o

