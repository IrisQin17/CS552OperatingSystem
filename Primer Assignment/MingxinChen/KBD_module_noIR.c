#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");

/* attribute structures */
struct ioctl_KBD_WHILE {
    char key;
};
static struct proc_dir_entry *proc_entry;
static struct file_operations pseudo_dev_proc_operations;

#define IOCTL_CHAR_WHILE _IOW(0, 6, struct ioctl_KBD_WHILE)

/* functions */
static int pseudo_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
void enable_irq(int irq_num);
void disable_irq(int irq_num);


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

	return 0;
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
    return;
}


/***
 * ioctl() entry point...
 */
static inline unsigned char inb(unsigned short usPort) {
    unsigned char uch;
    asm volatile( "inb %1,%0" : "=a" (uch) : "Nd" (usPort) );
    return uch;
}

static inline void outb(unsigned char uch, unsigned short usPort) {
    asm volatile( "outb %0,%1" : : "a" (uch), "Nd" (usPort) );
}

char my_getchar ( void ) {
  char c;
  static char scancode[128] = "\0\e1234567890-=\177\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\000789-456+1230.\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

  while( !(inb( 0x64 ) & 0x1) || ( ( c = inb( 0x60 ) ) & 0x80 ) );
  return scancode[ (int)c ];

}


static int pseudo_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
    struct ioctl_KBD_WHILE ioc;

    my_printk("Enter pseudo_device_ioctl");
    switch (cmd){
        case IOCTL_CHAR_WHILE:
            disable_irq(1);
            ioc.key= my_getchar();         
            copy_to_user((struct ioctl_KBD_WHILE *)arg, &ioc,
                   sizeof(struct ioctl_KBD_WHILE));
            enable_irq(1);
    
        default:
            return -EINVAL;
            break;
    }
    return 0;
}

module_init(initialization_routine); 
module_exit(cleanup_routine); 