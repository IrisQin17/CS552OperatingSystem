FIFO-1 by Mingxin Chen and Ji-Ying Zou

We finished the FIFO by doing the following processes: 
First, creating 3 threads which called thread[0], thread[1] and thread[2] and assigning them some jobs function1, function2, and function3 respectively which have the print function and system sleep (to make it run slower).
 
Second, we called the scheduler to do the context switch and schedule the threads. We also implemented the yield function in this assignment. The thread will call the yield function and it will call the scheduler to schedule the next thread.

We can run the FIFO-1 by going to the directory the files located and typing the following:
$ Make all
$ qemu-system-i386 -kernel FIFO-1 -m 16

References:
Memos-2.rar

