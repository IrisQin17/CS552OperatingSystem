MemOS-1 by MingXin Chen and Ji-Ying Zou

We created a virtual disk image and installing grub by following the instructions the professor West provided on the assignment website: https://www.cs.bu.edu/~richwest/cs552_spring_2019/assignments/memos/BOCHS-disk-image-HOWTO

After executing the $ make all call, we will have file “disk.img” , and we can execute it by typing “qemu-system-i386 pathto/disk.img -m 16”(p.s. 16 is memory size of MB.), and we boot from qemu successfully.

And open the vnc viewer by typing this command in another terminal:
"/root/vnc/opt/TigerVNC/bun/vncviewer :127.0.0.1 :5900"

After doing all these steps, we are able to see the message like this:
MemOS: Welcome *** System Memory is: 16 MB
Address range [xxxx : yyyy] status: zzzz
…(several lines)

