.global stack

# Block Started by Symbol，通常是指用来存放程序中未初始化的全局变量的一块内存区域，属于静态内存分配
.bss 

# 对齐4kb？为什么
.align 0x1000

# 给stack留4kb的位置
# .comm 声明这个stack是一个名字，在做linkerscript的时候，会跟另外一个文件中的stack链接在一起，后面0x1000是stack的长度
.comm stack, 0x1000

# 数据段开始
.data

# 对齐4位？为什么
.align 0x4

gdt:
    # 第一个元组是空    null selector
    .long 0
    .long 0

    # GDT
    # base 0:15                        | limit 0:15
    # base 24:31 | flags | limit 16:19 | access byte | base 16:23
    # base地址是00000000，limit是FFFFF
    # Flags
    #   Gr | Sz | 0 | 0
    #   Granularity bit为1，表示是limit以4kB为单位
    #   Size bit是1，表示32位保护模式
    # access byte
    #   Pr | Privl | S | Ex | DC | RW | Ac
    #   Present bit是1，表示selector有效
    #   Privilege是00，表示ring level 0，是最高的kernel level
    #   Descriptor type是1
    #   Executable bit是1，表示可执行，0的话是data segment
    #   Direction bit/Conforming bit是0，因为是code segment，表示只能被当前ring privilege执行
    #   Readable bit/Writable bit是1，因为是code segment，表示可读
    #   Accessed bit是0，如果该段被使用的时候会设为1
    .long 0x0000FFFF
    .long 0x00CF9A00

    # Executable bit是0，表示是data segment
    # Readable bit/Writable bit是1，因为是data segment，表示可写
    .long 0x0000FFFF
    .long 0x00CF9200

# GDTR写入的信息
gdt_ptr:
    # 3个描述符，每个8bytes，3 * 8 - 1 = 23 = 0x17，GDT长度
    .short 0x17
    # GDT的基地址
    .long gdt
    # 总共48bit的内容

# test开始
.text
.global _start
.global system_finish
.global switch_to

_start:
    jmp real_start
    # multiboot header，memos的答案有写
    .align 4
    .long 0x1BADB002
    .long 0x00000003
    .long 0xE4524FFB

real_start:
    # 装载GDTR的信息
    lgdt gdt_ptr


    # GDT【null|cs|ds】 each one 8 bytes, so cs is 0x08
    # 跳到code selector的那个segment里面
    # 告诉CPU instructions的地址在1f，然后他的descriptor在08
    # cs寄存器有关
    ljmp $0x08 , $1f

1:
    # reload all segnment registers, ds = 16 = 0x10
    # 装载data descriptor指向的那个selector
    # 同上
    movw $0x10 , %ax
   
    movw %ax , %ds      # 16 bits register
    movw %ax , %ss
    movw %ax , %es      # es和fs和gs可以是null，每次初始化ds，ss和cs
    movw %ax , %fs
    movw %ax , %gs

    # 把esp 指向stack 的頂端，stack 的大小是1000所以才+1000
    # 当时预留的4kb的栈底 = 栈顶
    movl $stack+0x1000, %esp

    # ebx压栈的话，ebx就是multiboot information的address
    pushl %ebx

    call init

finish:
    # 處理器停止
    cli 
    hlt
system_finish:
    #thread 結束時，在這無窮迴圈
    jmp system_finish

