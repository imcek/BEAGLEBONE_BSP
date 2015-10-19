
obj-m:=bbnative_kernel.o

ARCH=arm 
CROSS_COMPILER=/home/recep/Desktop/beaglebone/gcc-linaro-arm-linux-gnueabihf-4.7-2013.03-20130313_linux/bin/arm-linux-gnueabihf-
KERN_SRC 	:=/home/recep/git/BEAGLEBONE_BSP/linux-3.14.26/
PWD:=$(shell pwd)

modules:
	make -C $(KERN_SRC) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILER) modules
	$(CROSS_COMPILER)gcc ../app.c -o ../app -g
	
install:
	make -C $(KERN_SRC) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(COMPILER) modules_install
	depmod -a #bagimli tum modulleri ekle

clean:
	make -C $(KERN_SRC) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(COMPILER) clean
	rm -f ../app
	
	
