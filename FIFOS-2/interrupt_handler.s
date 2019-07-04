.global timerInterruptHandler_assembly
.align 	4

timerInterruptHandler_assembly:
	# 压栈所有的普通寄存器
	pushal
	# 压栈ds（16 bits的ds，但是stack是32 bits，所以先搬到eax）
	#save machine state
	movw %ds, %ax
	pushl %eax

	# 初始化ds，es，fs，gs
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	# movw $0x20, %al
	# outb %al, $0x20 
	call timerInterruptHandler

	# restore machine states
	popl %eax
	movw %ax, %gs
	movw %ax, %fs
	movw %ax, %es
	movw %ax, %ds

	popal

	iret