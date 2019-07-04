#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");

/* attribute structures */
struct ioctl_KBD {
    char key;
};
static struct proc_dir_entry *proc_entry;
static struct file_operations pseudo_dev_proc_operations;

#define IOCTL_CHAR _IOW(0, 6, struct ioctl_KBD)

#define KBD_IRQ 1
#define KBD_STATUS 0x80
#define KBD_SCANCODE 0x60

DECLARE_WAIT_QUEUE_HEAD(wq);

static volatile int ev_press = 0;
static unsigned char KBD_key;

/* functions */
static int pseudo_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
irqreturn_t ir_getchar(int irq, void* dev);

static int __init initialization_routine(void) {
    printk("<1> Loading module\n");
    pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;
    /* Start create proc entry */
    proc_entry = create_proc_entry("ioctl_test", 0444, NULL);
    if(!proc_entry) {
        printk("<1> Error creating /proc entry.\n");
        return 1;
    }

    //proc_entry->owner = THIS_MODULE; <-- This is now deprecated
    proc_entry->proc_fops = &pseudo_dev_proc_operations;

    free_irq(1, NULL);
    int ret = request_irq(1, ir_getchar, IRQF_SHARED, "KBD_IRQ", (irqreturn_t*)ir_getchar);
	printk("Success loading? %d", ret);
	return ret;
}


void my_printk(char *string)
{
    struct tty_struct *my_tty;

    my_tty = current->signal->tty;

    if (my_tty != NULL) {
        (*my_tty->driver->ops->write)(my_tty, string, strlen(string));
        (*my_tty->driver->ops->write)(my_tty, "\015\012", 2);
    }
} 


static void __exit cleanup_routine(void) {

    printk("<1> Dumping module\n");
    remove_proc_entry("ioctl_test", NULL);
    free_irq(1, (irqreturn_t*) ir_getchar);
    return;
}


/***
 * ioctl() entry point...
 */
/*static inline unsigned char inb(unsigned short usPort) {
    unsigned char uch;
    asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
}

static inline void outb(unsigned char uch, unsigned short usPort) {
    asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
}*/

irqreturn_t ir_getchar(int irq, void* dev) {
    //my_printk("Enter ir_getchar\n");

    static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

    char c = inb(KBD_SCANCODE);
    char status = inb(0x64);
    
    if(! (c & KBD_STATUS)) {
        //my_printk("Enter KBD_KEY\n");
        KBD_key = scancode[(int)c];
        ev_press = 1;
        wake_up_interruptible(&wq);
    }

    return IRQ_RETVAL(IRQ_HANDLED);
}



static int pseudo_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    struct ioctl_KBD ioc;
    ev_press = 0;

    my_printk("Enter pseudo_device_ioctl");
    switch (cmd){
        case IOCTL_CHAR:
            wait_event_interruptible(wq, ev_press);
            ev_press = 0;
            ioc.key = KBD_key;
            printk("Get char: %c \n", KBD_key);
            copy_to_user((struct ioctl_KBD *)arg, &ioc, sizeof(struct ioctl_KBD));
            break;
    
        default:
            return -EINVAL;
            break;
    }
    return 0;
}

module_init(initialization_routine); 
module_exit(cleanup_routine); 