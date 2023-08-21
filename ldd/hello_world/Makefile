obj-m := hellomain.o


KERNEL_DIR=/home/atom-pi/astudy/kernel/linux

KERNEL_DIR_HOST=/lib/modules/6.1.45-v8-VarunPyasi+/build


all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

help:
	make -C $(KERNEL_DIR) M=$(PWD) help

host:
	make -C $(KERNEL_DIR_HOST) M=$(PWD) modules
