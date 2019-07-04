.global switch_to

switch_to:

    # 如果esi是0的话就跳到1f，应该是boot.s里面的那个ljmp相关？
    # 例子：movsb es:di, ds:si，si是Source Index的意思
    # esi是用来防止被替换下来的thread的stack pointer的地址
    testl %esi,%esi
    jz 1f

	#save
    # 放所有的 flags register 和 general register
    pushfl
    pushal

    # 放 segment register
    pushw %ds
    pushw %es
    pushw %fs
    pushw %gs

    # 将esp的内容放到esi指向的地址中去，esp的内容就是stack pointer
    movl %esp, (%esi)

1:
   
    # 将edi中存储的地址指向的内容，放入esp，重新设定了stack pointer
    # 为什么要这样做: restore the next thread
    movl (%edi) , %esp

    popw %gs
    popw %fs
    popw %es
    popw %ds

    popal
    popfl

    ret
