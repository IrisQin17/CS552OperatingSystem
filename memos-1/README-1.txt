MemOS-1 by MingXin Chen and Ji-Ying Zou

We created a virtual disk image and installing grub by following the instructions the professor West provided on the assignment website: https://www.cs.bu.edu/~richwest/cs552_spring_2019/assignments/memos/BOCHS-disk-image-HOWTO

With the above steps from 1-4, we now have an img file.

Now we can go to the directory where the memos-1 file located, and type in “make” command into the terminal.

After executing the $ make all call, we will have file “memos-1_test” , and we can execute it by typing “qemu-system-i386 pathto_memos-1_test -m 16”, and we boot from qemu successfully.

And we can write the “memos-1_test” to our img created earlier, by typing the dd command:
“dd if=memos-1_test of=pathto/img bs=440 count=1 conv=notrunc”
And then we can boot from qemu by using the command:
“qemu-system-i386 pathto/img -m 16” (p.s. 16 is memory size of MB.)

And open the vnc viewer by typing this command in another terminal:
"/root/vnc/opt/TigerVNC/bun/vncviewer :127.0.0.1 :5900"

After doing all these steps, we are able to see the message like this:
MemOS: Welcome *** System Memory is: 16 MB
Address range [xxxx : yyyy] status: zzzz
…(several lines)

