.globl _start	
.globl stack
    .bss
    .align 0x1000
    .comm stack, 0x1000
    .data
    .text

_start:
    jmp real_start

    /* Multiboot header -- Safe to place this header in 1st page for GRUB */
    .align 4
    .long 0x1BADB002 /* Multiboot magic number */
    .long 0x00000003 /* Align modules to 4KB, req. mem size */
    .long 0xE4524FFB /* Checksum */

real_start:
    movl $stack+0x1000, %esp
    pushl %ebx

    call init
    
    cli
    hlt
loop:
    jmp loop
