#include "module_interface.h"
#include "../test/data_structure.h"

static struct proc_dir_entry *proc_entry;
static struct file_operations ramdisk_operations;
superblock *superblock_ptr;
my_inode *inode_ptr;
bitmap *bitmap_ptr;
void *space_ptr;
static void *ramdisk;

static int ramdisk_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) {
	// handler of iotcl
	// check the module_opeartion.h for interfaces
    char current_path[100];
    char* pathname;
    int ret = ERROR;
    int size;
    int* fd;
    file_op_arguments* file_info;
    
    printk("enter ioctl %d", cmd);

    switch(cmd)
    {
        case IOCTL_PWD:
            strcpy(current_path, rd_pwd());
            ret = copy_to_user((char *)arg, current_path, sizeof(current_path));//1: user space destination address, 2: source address, 3: size. on success, zero
            return ret;
        case IOCTL_CREAT:
            size = sizeof(file_op_arguments);
            file_info = (file_op_arguments *)vmalloc(size);
            copy_from_user(file_info, (file_op_arguments *)arg, size);
            ret = rd_creat(file_info->pathname, file_info->mode);
            vfree(file_info);
            return ret;
        case IOCTL_MKDIR:
            size = sizeof(char) * (strlen((char *)arg) + 1);
            pathname = (char *)vmalloc(size);
            copy_from_user(pathname, (char *)arg, size);
            ret = rd_mkdir(pathname);
            vfree(pathname);
            return ret;
        case IOCTL_OPEN:
            size = sizeof(file_op_arguments);
            file_info = (file_op_arguments *)vmalloc(size);
            copy_from_user(file_info, (file_op_arguments *)arg, size);
            ret = rd_open(file_info->pathname, file_info->flag);
            vfree(file_info);
            return ret;
        case IOCTL_CLOSE:
            size = sizeof(int);
            fd = (int *)vmalloc(size);
            copy_from_user(fd, (int *)arg, size);
            ret = rd_close(*fd);
            vfree(fd);
            return ret;
        case IOCTL_READ:
            size = sizeof(file_op_arguments);
            file_info = (file_op_arguments *)vmalloc(size);
            copy_from_user(file_info, (file_op_arguments *)arg, size);
            ret = rd_read(file_info->fid, file_info->input_content, file_info->input_size);
            copy_to_user((file_op_arguments *)arg, file_info, size);
            vfree(file_info);
            return ret;
        case IOCTL_WRITE:
            size = sizeof(file_op_arguments);
            file_info = (file_op_arguments *)vmalloc(size);
            copy_from_user(file_info, (file_op_arguments *)arg, size);
            ret = rd_write(file_info->fid, file_info->input_content, file_info->input_size);
            vfree(file_info);
            return ret;
        case IOCTL_LSEEK:
            size = sizeof(file_op_arguments);
            file_info = (file_op_arguments *)vmalloc(size);
            copy_from_user(file_info, (file_op_arguments *)arg, size);
            ret = rd_lseek(file_info->fid, file_info->position);
            vfree(file_info);
            return ret;
        case IOCTL_UNLINK:
            printk("in iovtl_unlink1\n");
            size = sizeof(char) * (strlen((char *)arg) + 1);
            pathname = (char*)vmalloc(size);
            printk("in iovtl_unlink2\n");
            copy_from_user(pathname, (char *)arg, size);
            printk("in iovtl_unlink3\n");
            ret = rd_unlink(pathname);
            printk("in iovtl_unlink4\n");
            vfree(pathname);
            printk("in iovtl_unlink5\n");
            return ret;
        case IOCTL_CHMOD:
            size = sizeof(file_op_arguments);
            file_info = (file_op_arguments *)vmalloc(size);
            copy_from_user(file_info, (file_op_arguments *)arg, size);
            ret = rd_chmod(file_info->pathname, file_info->mode);
            vfree(file_info);
            return ret;
        default:
            break;
    }
    return ret;
}

int init_datastructure(void) {
    int i ;
 
    printk("initialize Ramdisk!\n");
    ramdisk = (void *) vmalloc(ramdisk_size);
    
    superblock_ptr = (superblock *) ramdisk;
    inode_ptr = (my_inode *) ((char *)ramdisk + block_size);
    bitmap_ptr = (bitmap *) ((char *)inode_ptr + block_size * inode_list_size);
    space_ptr = (void *) ((char *)bitmap_ptr + block_size * bitmap_size);
    
    memset((void *) ramdisk, 0, ramdisk_size);
    
	// initialize superblock
    superblock_ptr -> freeBlock_number = allocatable_space_size - 1;
    superblock_ptr -> freeInode_number = inode_list_num - 1;
    superblock_ptr -> first_freeBlock = 1;
    superblock_ptr -> first_freeInode = 1;
    // initialize bitmap
    bitmap_ptr->index[0] |= 0x80;
    for (i = 1; i < bitmap_sublock; i++)
        bitmap_ptr->index[i] &= 0;
    // initialize inode
    strcpy(inode_ptr->type, dir);
    inode_ptr->size = 0;
    inode_ptr->location[0] = space_ptr;
    inode_ptr->protection = 0;
    inode_ptr->operation = 0x100;
    inode_ptr->parent_index = -2;
    inode_ptr->block_number = 0;
    inode_ptr->last_block_offset = 0;
    
    return SUCCESS;
}

static int __init init_ramdisk(void) {
	// this function is finished
	printk("Loading ramdisk module\n");

	ramdisk_operations.ioctl = ramdisk_ioctl;
	proc_entry = create_proc_entry("Ramdisk", 0444, NULL);
    //extern struct proc_dir_entry *create_proc_entry(const char *name, mode_t mode,
    //struct proc_dir_entry *parent); 0444：user, group...permission

	if (!proc_entry)
	{
		printk("Error creating Ramdisk proc file \n");
		return ERROR;
	}

	proc_entry->proc_fops = &ramdisk_operations;

	if (init_datastructure() == ERROR)
	{	printk("Error initialize ramdisk\n");
		return ERROR;
	}
    
    // initialize it with the root directory"/"
	return SUCCESS;
}

static void __exit unload_ramdisk(void) {
	// destory FDT
    printk("Remove Ramdisk module\n");
    free_FDT();
    vfree(ramdisk);
    remove_proc_entry("Ramdisk", NULL);
}

module_init(init_ramdisk);
module_exit(unload_ramdisk);
