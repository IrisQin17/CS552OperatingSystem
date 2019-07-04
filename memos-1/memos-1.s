.text
	.global _start
	.code16

#-----------------------------------------------------------------------
	.set 	segment_address, 	0x07c0
	.set 	stack_address, 		0x1000

	movw	$segment_address, %ax	# init ds, es and ss to 0x0000
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %ss
	

	xorw	%si, %si 				# clear si, di
	xorw	%di, %di 	

	movw	$stack_address, %ax		# init bp, sp to 0x1000
	movw	%ax, %bp
	movw	%ax, %sp

	call	printWelcomeMessage
	call 	getAddressMap
	call 	printAddressMap
	jmp		.

#-----------------------------------------------------------------------
printWelcomeMessage:
	mov 	$0x0, %dl 				# 8 row, 0 column
	mov 	$0x8, %dh
	leaw 	msg_welcome - 0x1000, %SI
	movw 	len_welcome - 0x1000, %CX
	call 	print_string

	call 	getMemorySize

	leaw 	msg_welcome_unit - 0x1000, %SI
	movw 	len_welcome_unit - 0x1000, %CX
	call 	print_string

	call 	newline
	ret	

msg_welcome: 	  .ascii "MemOS: Welcome *** System Memory is: 0x"
len_welcome: 	  .word  . - msg_welcome
msg_welcome_unit: .ascii " MB  "
len_welcome_unit: .word	 . - msg_welcome_unit

#-----------------------------------------------------------------------
getMemorySize:
	movw 	$0xE801, %ax		# int 15 E801 get memory size
	int 	$0x15
	addw 	$0x400, %ax			# ax add 1MB
	sall 	$6, %ebx	  		# bx mul 64
	addl 	%EAX, %ebx			# ax + bx
	sarl 	$10, %ebx			# convert KB to MB
	
	movl 	$msg_memory, %esi	
	movl	%ebx, (%esi) 		# put the answer to msg_memory
	call 	print_32 			# print the data in address of msg_memory

	ret

#-----------------------------------------------------------------------
	.set	SMAP_MAGIC,	0x534d4150	#magic number
	.set	MMARD_SIZE, 20			#memory map address range descriptor size

# eax for counter, es:si for buffer
getAddressMap:
	pushl	%ebp
	pushl	%esp

	movl	$0x0000e820, %eax 	# int15 e820 get memory address map
	xorl	%ebx, %ebx 			
	movl	$MMARD_SIZE, %ecx 	
	movl	$SMAP_MAGIC, %edx 	
								
	movl	$buffer, %edi 		# edi for return buffer
	xorw	%bp, %bp 			# bp for counter = 0	
	int 	$0x15

	jc		finish				# cf set on fail

	movl	$SMAP_MAGIC, %edx 	
	cmpl	%eax, %edx 			# eax should eq edx on success
	jne 	finish				

loop:
	test	%ebx, %ebx 			# ebx & ebx = 0 if finished
	je 		finish 			

	incw	%bp				 	# increase counter
	addl	$MMARD_SIZE, %edi 	# increase return address

	movl	$0x0000e820, %eax 	# reset
	movl	$MMARD_SIZE, %ecx 	
	int 	$0x15 				# next call
	jmp 	loop				# loop until ebx = 0

finish:
	xorl	%eax, %eax 			# reset
 	movw 	%bp, %ax 			# copy counter to ax
 	movl 	$buffer, %esi 		# copy address to esi
 	clc 						# clear cf
 	popl 	%esp
 	popl 	%ebp

 	ret

#-----------------------------------------------------------------------
newline:
	pushl	%eax
	pushl 	%ebx
	pushl 	%ecx

	movb 	$0x03, %ah 			# int10 0x3 read cursor position
	movb 	$0x00, %bh

	int 	$0x10 

	addb   	$0x01, %dh			# row ++
	xorb	%dl, %dl			# column = 0
	movb	$0x02, %ah			# int10 0x2 set cursor position

	int    	$0x10 

	popl 	%ecx
	popl 	%ebx
	popl 	%eax

	ret

#-----------------------------------------------------------------------
#ecx for length, si for address
print_string:
	lodsb
	or 		%AL, %AL
	jz 		return
	movb 	$0xE, %ah			# int10 0xE print string in the address of si
	int 	$0x10
	jmp 	print_string
return:
	ret


#-----------------------------------------------------------------------
#print one digit (lower 4 bits of al) in hex
print_hex_digit:
	
	pushl	%eax
	pushl 	%ebx

	andb	$0x0f, %al  		# mask higher 4 bits
	addb 	$0x30, %al 			# convert 0~9 to '0'~'9'
	cmpb 	$0x39, %al			# compare with '9'
	jle		do_print_hex		# if <= '9', print. else, convert to 'A'
	addb 	$0x07, %al

do_print_hex:
	movb	$0x0e, %ah 			# int10 0x3 display char and move cursor
	movb	$0x00, %bh 			# page number
	movb	$0x0c, %bl 			# color

	int   	$0x10 				

	popl 	%ebx
	popl 	%eax 

	ret

#-----------------------------------------------------------------------
#esi for the address
print_32:
	pushl	%ecx
	movl	$4, %ecx			# loop counter for 32 bits
	addl	$3, %esi			# first one byte on highest address
do_print_32:
	movb 	(%esi), %al 		# four bits on left
	shrb	$4, %al
	call 	print_hex_digit

	movb	(%esi), %al  		# four bits on right
	call 	print_hex_digit

	decl	%esi
	loop 	do_print_32

	addl	$5, %esi
	popl 	%ecx
	ret


#-----------------------------------------------------------------------
#es:si for begin address, eax for count
printAddressMap:
	pushl 	%esi

entry_loop:
	pushl	%EAX				# save counter

	pushl	%esi						
	leaw 	str_base - 0x1000, %si
	movw 	len_base - 0x1000, %CX
	call 	print_string		# print 'Address Range['
	popl	%esi

	movb 	$0x03, %ah 			# int10 0x3 read cursor position
	movb 	$0x00, %bh
	int 	$0x10

	call 	print_32 			# print base address

	movb $':', %AL
	movb $0xE, %AH
	int $0x10 					# int10 0xe print ':'

	# movb $'0', %AL
	# movb $0xE, %AH
	# int $0x10 					# int10 0xe print '0'

	# movb $'x', %AL
	# movb $0xE, %AH
	# int $0x10 					# int10 0xe print 'x'

	addl	$16, %esi 			# move to the base address of the next entry 
	call 	print_32 			# print end address
	
	pushl	%esi
	leaw 	str_type - 0x1000, %SI
	movw 	len_type - 0x1000, %CX
	call 	print_string 		# print '] State:'
	popl	%esi

	subl	$8, %esi 			# move back to the type of current entry
	movw	(%esi), %eax 		# read the type into eax

	pushl	%esi
	cmp 	$0x02, %eax 		# compare to 2
	je 		state2
	leaw 	str_state1 - 0x1000, %SI
	movw 	state1_len - 0x1000, %CX
	jmp 	state_con

state2: 	
	leaw 	str_state2 - 0x1000, %SI
	movw 	state2_len - 0x1000, %CX

state_con:
	call 	print_string 		# print the type
	popl	%esi

	addl	$4, %esi 			# move to the begining of the next entry

	call 	newline 			# call for a new line

	popl	%eax
	decl 	%eax
	cmpl 	$0, %eax 			# loop counter compare to 0
	jge		entry_loop

	popl	%esi

	ret

str_base: 	.ascii	"Address Range["
len_base: 	.word  	. - str_base

str_type: 	.ascii 	"] State: "
len_type: 	.word  	. - str_type

str_state1:	.ascii	"available"
state1_len:	.word	. - str_state1

str_state2:	.ascii	"reserved"
state2_len: .word	. - str_state2

#-----------------------------------------------------------------------
	.org	0x01fe				#510 Byte
	.byte	0x55				
	.byte	0xaa

buffer:
	.fill 	20 * MMARD_SIZE, 1 
	.set	buffer_len, . - buffer

msg_memory: .word 0
			.word 0