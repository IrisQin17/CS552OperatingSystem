#include "types.h"
#include "display.h"

extern void scheduler();
extern void timerInterruptHandler_assembly();

typedef struct {
  uint16_t offsetLow;
  uint16_t selector;
  uint8_t  unused0;
  uint8_t  type_s_dpl_p;
  uint16_t offsetHigh;
} IDTEntry;

// interrupt的array，有33个interrupt，0-32号，0-31是固定的，剩下一个是我們加的timer的interrupt
static IDTEntry IDT[33];

// p是要写入的端口，v是data
// outb p, v
void outb (uint16_t p, uint8_t v) {
	// N: unsigned 8 bits int constant; d: edx
    __asm__ __volatile__("outb %0, %1"::"a"(v), "Nd"(p));
}

void systemInterruprHandler() {
  // 为什么不用EOI
  // outb(0x20 , 0x20);
  return;
}

void timerInterruptHandler() {
  //terminal_writestring("timerInterruptHandler");

  // 压栈所有的普通寄存器
  /*__asm__ __volatile__("pushal");
  // 压栈ds（16 bits的ds，但是stack是32 bits，所以先搬到eax）
  //__asm__ __volatile__("movl $0, %eax");
  __asm__ __volatile__("movw %ds , %ax");
  __asm__ __volatile__("pushl %eax");

  // 初始化ds，es，fs，gs
  __asm__ __volatile__("movw $0x10, %ax");
  __asm__ __volatile__("movw %ax, %ds");
  __asm__ __volatile__("movw %ax, %es");
  __asm__ __volatile__("movw %ax, %fs");
  __asm__ __volatile__("movw %ax, %gs");*/
  
  // PIC的command的I/O端口，0x20是EOI，end of interrupt的意思，就是告诉PIC已经handle了这个中断，可以继续接受中断信号
  outb(0x20 , 0x20);
  // scheduler
  scheduler();

  // restore machine states
  /*__asm__ __volatile__("popl %ebx");
  __asm__ __volatile__("movw %bx, %ds");
  __asm__ __volatile__("movw %bx, %es");
  __asm__ __volatile__("movw %bx, %fs");
  __asm__ __volatile__("movw %bx, %gs");
  __asm__ __volatile__("popal");
  __asm__ __volatile__("iret");*/
}

// 初始化IDT的内容
//https://wiki.osdev.org/Interrupt_Descriptor_Table
int initIDT() {
  // initialize IDT
  for(int i = 0 ; i < 32 ; i++) {
  	uint32_t handlerAddress = (uint32_t)systemInterruprHandler;
  	// offset 16-31
  	IDT[i].offsetLow = (uint16_t)(handlerAddress & 0xffff);

  	// selector是0x08，index|TI|RPL
	  //   index是1
	  // 	 TI是0代表GDT
	  //	 RPL是00代表多线程多进程multi-task
  	IDT[i].selector = 0x08;

  	// reserved 0
  	IDT[i].unused0 = 0;

  	// type是0xe代表80386 32-bit interrupt gate
  	// S是0代表是interrupt and trap gate
    // DPL = 0 means the handlers need to be executed at privilege level zero.
    // p = 1; Set to 0 for unused interrupts.

    //p DPL S Type
    //1 00  0 1110 = 8E
  	IDT[i].type_s_dpl_p = 0x8e;
  	
  	// offset 0-15
  	IDT[i].offsetHigh = (uint16_t)((handlerAddress >> 16) & 0xffff);
  }

  // initialize IDTR
  // 每一个IDT descriptor是8bytes
  uint16_t length = 8 * 33 - 1;
  uint32_t baseAddress = (uint32_t)(IDT);
  uint64_t IDTRegister = (uint64_t)baseAddress << 16 | length;

  // load IDTR
  __asm__ __volatile__ ("lidt %0"::"m"(IDTRegister)); 
  return 1;
}

// 初始化PIC的内容
// https://wiki.osdev.org/8259_PIC
int initPIC() {
  // master chip
  // 我们现在要用到master
  //When you enter protected mode, the first command you will need to give the two PICs is the initialise command  (code 0x11). This command makes the PIC wait for 3 extra "initialisation words" on the data port.
  outb (0x20, 0x11);
  // 我们在protected mode下用到IDT的offset是从32开始的，因为32之后才是自己定义的，前面32个都是系统处理的中断，不是programmable interrupt
  // 把IRQ0-IRQ7换成中断号32-，所以对于timer interrupt IRQ0而言，会变成IRQ32，所以把timer interrupt handler放在IDT[32]
  outb (0x21, 0x20);
  // 我们有用到slave，其实因为只有一个中断，完全可以不用的，连接的话能够处理16个中断，不连slave的话只能处理八个，不连的话这里写0x02
  outb (0x21, 0x04); 
  // environment set 系统是8086
  outb (0x21, 0x01); 

  // slave chip
  // 我们现在要用到slave
  outb (0xA0, 0x11);
  // 我们在protected mode下用到IDT的offset是从40开始的
  outb (0xA1, 0x28);
  // 我们后面没有slave了，所以是0x02，这里就是EOI，end of interrupt 
  outb (0xA1, 0x02); 
  // 系统是8086
  outb (0xA1, 0x01); 

  // unmask掩码的过程是会把0xfe跟收到的interrupt信号XOR，如果是1的话就会变成0，如果是0的话还是1
  // IRQ0是system timer，其他的都disable interrupt
  outb (0x21, 0xfe);
  // disable the slave PIC
  outb (0xA1, 0xff);
  return 1;
}


// 初始化PIT的内容
// https://wiki.osdev.org/Programmable_Interval_Timer
int initPIT() {
  uint32_t handlerAddress = (uint32_t)timerInterruptHandler;
  IDT[32].offsetLow = (uint16_t)(handlerAddress & 0xffff);
  IDT[32].selector = 0x08;
  IDT[32].unused0 = 0;
  IDT[32].type_s_dpl_p = 0x8e;
  IDT[32].offsetHigh = (uint16_t)((handlerAddress >> 16) & 0xffff);

  // 100 interrupts per second, 10ms
  uint16_t f = 1193180 / 100;

  // 0x43是写入PIT的command register的writeable端口
  // 0x34是0011 0100 
  //   00：channel 0
  //   11：access mode是lobyte/hibyte，下面会写入
  //   010：mode2 rate generator
  //   0：16-bit binary，如果是1的话是four-digit BCD
  outb(0x43, 0x34);

  uint8_t lobyte = (uint8_t)(f & 0xff);
  uint8_t hibyte = (uint8_t)((f >> 8) & 0xff);

  // 0x40是PIT的channel 0的data port（因为上面的access mode是lo/hi）
  outb(0x40, lobyte);
  outb(0x40, hibyte);

  return 1;
}
