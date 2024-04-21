obj-m := hellomain.o

KERNEL_DIR=/lib/modules/$(shell uname -r)/build 

all: host

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

help:
	make -C $(KERNEL_DIR) M=$(PWD) help

host:
	make -C $(KERNEL_DIR) M=$(PWD) modules
