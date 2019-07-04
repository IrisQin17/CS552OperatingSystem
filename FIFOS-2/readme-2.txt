FIFO-2 by Mingxin Chen and Ji-Ying Zou

We created 3 threads, called it thread[0], thread[1] and thread[2] and assigning them some jobs function1, function2, and function3 respectively which have the print function and system sleep (to make it run slower).
After initializing IDT, PIC, PIT and enabling the interrupt, the timer will generate a timer interrupt every 10ms. CPU will save the CPU status before sending the EOI and executing the handler function. And then we call the scheduling function which finds the next thread to run and do the context switch, after all of this, we restore the CPU status. 

We implemented the interrupt handler in this assignment by following the lab materials on the lab website and os.dev

We can run our FIFO-2 by entering the directory of the files and typing:
$ make all
$ qemu-system-i386 -kernel FIFO-2 -m 16  

