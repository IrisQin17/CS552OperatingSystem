// module--------------------------------------------
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/utsname.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/string.h>
#include <asm/unistd.h>

// statistics
#define ramdisk_size 2097152    // 2MB, 8192 blocks
#define block_size 256            // in bytes
#define superblock_size 1        // in block
#define inode_list_size 256    // in block
#define inode_list_num 1024    // max number of file 256 blks * 256 bytes/ 64 bytes = 1024
#define bitmap_size 4            // in block
#define bitmap_sublock 1024    // bitmap sublock
#define allocatable_space_size 7931 // in block 8192-1-256-4 = 7931
#define max_file_size 1067008    // the maximum size of a file

// object type
#define dir "dir"
#define reg "reg"
#define nul "nul"

// info
#define ERROR -1
#define SUCCESS 0

// superblock
typedef struct superblock_t {
    uint16_t freeBlock_number;
    uint16_t freeInode_number;
    uint16_t first_freeBlock;
    uint16_t first_freeInode;
} superblock;

// inode
typedef struct my_inode_t {
    char type[4];            // 4 bytes
    uint32_t size;            // 4 bytes
    void *location[10];        // 40 bytes direct/indirect ptr
    uint16_t protection;    // permission 0-kernel 1-user
    uint16_t operation;        // opertions could do on the file r,w,e(execution)
    uint16_t parent_index;    // the parent dirctory inode index
    int block_number;       // block_number: 0-7 direct block if 8 means direct block full
    // 8-71 single_indirect block; 72-4176 double_indirect block
    int last_block_offset;  // last_block_offset: offset inside block
    uint16_t unused[4];// 8 bytes unused
    int mode;
    int flag;
} my_inode;

// directory entry
typedef struct directory_entry_t {
    char filename[14];        // 14 bytes for file entry
    uint16_t inode_index;    // 2 bytes for inode index
} directory_entry;

// bitmap
typedef struct bitmap_t {
    uint8_t index[bitmap_sublock];
} bitmap;

// file_object
typedef struct file_object_m {
    int inode_index;
    int position;
    struct file_object_m *next;
} file_object;

// file descriptor table
typedef struct FDT_m {
    int pid;
    file_object *file_object_list_head;
    char *cur_path;
    struct FDT_m *next;
} FDT;

