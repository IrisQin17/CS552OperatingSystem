#include "multiboot.h"
#include "types.h"
#include "display.h"

// 总共三个线程
#define N 3
#define infinit 1000

// 每个线程一个静态stack区域
static uint32_t stack1[1024];
static uint32_t stack2[1024];
static uint32_t stack3[1024];

// TCB的结构
typedef struct {
  uint32_t *stackPointer;     // stack pointer
  uint32_t id;                // unique ID
  uint32_t (*function)();         // function address
  uint32_t finished;          // does the thread finish its job
  uint32_t assigned;              // is the TCB being assigned
  uint32_t priority;          // priority = 0
  uint32_t *bp;
}TCB;

// TCB的数组
TCB TCBArray[N];
// 当前运行的线程index初始化为-1
int currentThread = -1;

void scheduler() {
  // 从当前线程开始往后找，获取下一个要运行的线程的index
  uint32_t nextThread = infinit;
  for(uint32_t i = 1 ; i < N + 1 ; i++) {
    uint32_t thread = (currentThread + i) % N;
    // TCB block已经被assigned但是还没有finished
    if(TCBArray[thread].assigned == 1 && TCBArray[thread].finished == 0) {
      nextThread = thread;
      break;
    }
  }

  //terminal_functionId(nextThread);

  // 如果没有更多的线程，无限循环loop
  if(nextThread == infinit) {
    terminal_writestring("All threads done");
    __asm__ volatile("jmp system_finish");
  }

  // 当前运行的线程的index
  int temp = currentThread;
  // 修改当前运行线程为nextThread
  currentThread = nextThread;

  // context switch
  TCB *ptr = &TCBArray[nextThread];
  if(temp == -1) {
    // 第一次运行的时候，currentThread的TCB的address是0，表示没有store machine state，只有restore
    __asm__ volatile("call switch_to"::"S"(0), "D"(ptr));
  }
  else {
    TCB* cptr = &TCBArray[temp];
    __asm__ volatile("call switch_to"::"S"(cptr), "D"(ptr));
  }
}

// 线程yield
void functionYield() {
  scheduler();
}

// 线程结束
void functionFinish() {
  TCBArray[currentThread].finished = 1;
  TCBArray[currentThread].assigned = 0;
  scheduler();
}

// 拖延时间
void function_sleep() {
  int time = 80000000;
  while(time) {
    time -= 1;
  }
}

// function1 print <1>  3 times
void function1() {
  int times = 3;
  while(times) {
    terminal_writestring("<1>");
    function_sleep();
    times -= 1;
    functionYield();
  }
  terminal_writestring("thread 'function1' funished.");
  functionFinish();
}

// function2 print <2> 4 times
void function2() {
  int times = 4;
  while(times) {
    terminal_writestring("<2>");
    function_sleep();
    times -= 1;
    functionYield();
  }
  terminal_writestring("thread 'function2' funished.");
  functionFinish();
}

// function3 print <3> 5 times
void function3() {
  int times = 5;
  while(times) {
    terminal_writestring("<3>");
    function_sleep();
    times -= 1;
    functionYield();
  }
  terminal_writestring("thread 'function3' funished.");
  functionFinish();
}

// create a thread, write its TCB and put it into ready queue
uint32_t threadCreate(void* functionname, void* stack) {
  // 找到index of an unassigned TCB
  uint32_t id = 0;
  for(id = 0; id < N ; id ++) {
    if(TCBArray[id].assigned == 0) {
      break;
    }
  }
  // the number of ready thread is maxinum，已经没有要运行的线程
  if(id == N) {
    return 0;
  }

  // write TCB
  TCBArray[id].bp = (uint32_t *)stack;
  TCBArray[id].stackPointer = (uint32_t *)stack - 11;
  TCBArray[id].function = functionname;
  TCBArray[id].id = id;
  TCBArray[id].finished = 0;
  TCBArray[id].assigned = 1;
  TCBArray[id].priority = 0;

  // pushfl
  *((uint32_t *)stack) = (uint32_t)functionname;   // EIP   
  // FLAG里面的第1位是1，表示always 1 in EFLAGS
  // FLAG里面的第九位是0，表示disable interrupt
  *((uint32_t *)stack -  1) = 2;                // FLAGS 

  // pushal
  *((uint32_t *)stack -  2) = 0;                // EAX    
  *((uint32_t *)stack -  3) = 0;                // ECX  
  *((uint32_t *)stack -  4) = 0;                // EDX  
  *((uint32_t *)stack -  5) = 0;                // EBX  
  // ESP在push的过程中并不会被写入，所以不用担心
  *((uint32_t *)stack -  6) = 0;                // TEMP ESP 
  *((uint32_t *)stack -  7) = 0;                // EBP  
  *((uint32_t *)stack -  8) = 0;                // ESI  
  *((uint32_t *)stack -  9) = 0;                // EDI 

  // push ds, es, fs,gs 
  *(((uint16_t *)stack) -  20) = 0x10;    // DS
  *(((uint16_t *)stack) -  21) = 0x10;    // ES
  *(((uint16_t *)stack) -  22) = 0x10;    // FS
  *(((uint16_t *)stack) -  23) = 0x10;    // GS

  return 1;
}

// create three threads
void createThreads() {
  if(!threadCreate(function1, &stack1[1023])) {
    terminal_writestring("create thread 'function1' failed.");
  }
  else {
    terminal_writestring("thread 'function1' created.");
  }

  if(!threadCreate(function2, &stack2[1023])) {
    terminal_writestring("create thread 'function2' failed.");
  }
  else {
    terminal_writestring("thread 'function2' created.");
  }

  if(!threadCreate(function3, &stack3[1023])) {
    terminal_writestring("create thread 'function3' failed.");
  }
  else {
    terminal_writestring("thread 'function3' created.");
  }
}

void initTCBArray() {
  for(uint32_t i = 0 ; i < N ; i++) {
    // 每个TCB都unassigned和unfinished
    TCBArray[i].assigned = 0;
    TCBArray[i].finished = 1;
  }
}

void init(multiboot* pmb) {
  // display information
  terminal_initialize();
  terminal_writestring("FIFOS-1: Welcome to FIFOS-1 System!");
  terminal_writestring("                by Jiying & Mingxin");

  function_sleep();

  // init TCBArray
  initTCBArray();
  // create threads
  createThreads();
  // call the scheduler
  scheduler();
  return;
}