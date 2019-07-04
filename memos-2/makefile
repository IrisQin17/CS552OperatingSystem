all: memos-2

memos-2: boot.o memos-2.o
	ld -T memos-2.ld -o memos-2 boot.o memos-2.o

memos-2.o:
	gcc -m32 -fno-builtin -fno-stack-protector -fno-strict-aliasing -fno-delete-null-pointer-checks -nostdinc -I. -g -Wall -std=c99 -c memos-2.c -o memos-2.o 

boot.o:
	gcc -c boot.s -o boot.o

install: memos-2
	cp memos-2 /mnt/boot

clean:
	-rm *.o memos-2
