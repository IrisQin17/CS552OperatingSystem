rmmod ramdisk_module.sh

cd module
if [ -a ramdisk_module.ko ]; then
	make clean
fi
make
insmod ramdisk_module.ko

cd ../test
if [ -x ramdisk ]; then
	make clean
fi
make
./ramdisk
