obj-m		:= event_rotary.o 
KERN_SRC	:= /home/recep/git/BEAGLEBONE_BSP/linux-3.14.26/
PWD			:= $(shell pwd)

ARCH=arm 
CROSS_COMPILER=/home/recep/Desktop/beaglebone/gcc-linaro-arm-linux-gnueabihf-4.7-2013.03-20130313_linux/bin/arm-linux-gnueabihf-

modules:
	make -C $(KERN_SRC) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILER) modules

install:
	make -C $(KERN_SRC) M=$(PWD) modules_install
	depmod -a

clean:
	make -C $(KERN_SRC) M=$(PWD) clean