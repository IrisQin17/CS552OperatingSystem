obj-m += ramdisk_module.o
ramdisk_module-objs := initial_module.o

all:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean