#include "support_function.h"

extern superblock *superblock_ptr;
extern my_inode *inode_ptr;

char * rd_pwd(void)
{
    FDT *FDT_t;
    FDT_t = get_FDT();
    return FDT_t->cur_path;
}

// we don't allow same name file if even they are different type
int create(char *pathname, int mode, char *type)
{
    int des_inode_index;
    FDT *FDT_t;
    char *parentfile[20];
    char *childfile;
    int i;         // for any loop
    directory_entry *directory_entry_t;
    char para[16];
    
    int temp;
    int *position;
    temp = -1;
    position = &temp;
    
    if (superblock_ptr -> freeBlock_number < 1 ||
        superblock_ptr -> freeInode_number < 1)
    {
        printk("Insufficient Memory!\n");
        return ERROR;
    }
    
    
    FDT_t = get_FDT();
    
    for (i = 0; i < 20; i++)
        parentfile[i] = NULL;
    
    
    des_inode_index = read_pathname(pathname, FDT_t, parentfile, &childfile);
    
    // check correctness of parentfile path and childfile
    // check whether parentfile exist in coresponding path. it must exist
    // otherwise it reture ERROR
    
    for (i = 0; parentfile[i] != NULL; i++)
        if ((des_inode_index = isExist(parentfile[i], des_inode_index, position)) == ERROR)
            return ERROR;
    
    // check whether child is legal, name lenth and whether exist the same name
    if (strlen(childfile) > 13)
        return ERROR;
    
    // if exist the same name file
    if (isExist(childfile, des_inode_index, position) != ERROR)
        return ERROR;
    
    // write into cur_path
    
    directory_entry_t = (directory_entry *)vmalloc(sizeof(uint8_t) * 16);
    
    strcpy(directory_entry_t -> filename, childfile);
    directory_entry_t -> inode_index = superblock_ptr -> first_freeInode;
    
    
    // if (!strcmp(type, reg))
    // {
    //     int len = strlen(childfile);

    //     char suffix[5] = ".txt";
    //     if (len + 5 <= 14)
    //         strcpy(directory_entry_t -> filename + len, suffix);
    //     else
    //         strcpy(directory_entry_t -> filename + 9, suffix);
    // }
    
    for (i = 0; i < 16; i++)
        *(para + i) = *((char *)directory_entry_t + i);
    
    *position = (int)(&inode_ptr[des_inode_index])->size;

    if (write_file(des_inode_index, position, para, 16, 'c') == ERROR)
    {
        vfree(directory_entry_t);
        return ERROR;
    }
    
    if (!strcmp(type, dir))
        allocate_inode(des_inode_index, mode, dir);
    else
        allocate_inode(des_inode_index, mode, reg);
    
    vfree(directory_entry_t);
    return SUCCESS;
    
}

int rd_creat(char *pathname, int mode)
{
    return create(pathname, mode, reg);
}

int rd_mkdir(char *pathname)
{
    return create(pathname, 1, dir);
}

int rd_open(char *pathname, int flag)
{
    int des_inode_index;
    int open_inode_index;
    FDT *FDT_t;
    char *parentfile[20];
    char *childfile;
    int i;         // for any loop
    my_inode *des_inode;
    int temp ;
    int *position;

    int fd;
    char temppath[1024];
    char *temppathp;
    char backup[1024];
    int b;
    int back;
    file_object *file_object_t;

    temp = -1;
    position = &temp;
    fd = 0;
    temppathp = temppath;
    b = 0;

    FDT_t = get_FDT();
    strcpy(backup, pathname);
    
    for (i = 0; i < 20; i++)
        parentfile[i] = NULL;
    
    des_inode_index = read_pathname(pathname, FDT_t, parentfile, &childfile);
    
    // (1) if the pathname does not exist
    for (i = 0; parentfile[i] != NULL; i++)
        if ((des_inode_index = isExist(parentfile[i], des_inode_index, position)) == ERROR)
            return ERROR;
    
    // check whether child is legal, name lenth and whether exist the same name
    if (strlen(childfile) > 13)
        return ERROR;
    
    // check whether the file need to open exist
    if ((open_inode_index = isExist(childfile, des_inode_index, position)) == ERROR)
        return ERROR;
    
    des_inode = &inode_ptr[open_inode_index];
    // (3) if you attempt to open an open file
    if (isOpen(open_inode_index, FDT_t) == ERROR)
        return ERROR;
    //check access right
    if (des_inode->mode == 0 && (flag == 1 || flag == 2))
        return ERROR;
    if (des_inode->mode == 2 && (flag == 0 || flag == 1))
        return ERROR;
    
    // load into FDT
    file_object_t = FDT_t -> file_object_list_head;
    if (!strcmp(des_inode -> type, dir)) {
        fd = 0;
    }
    else
    {
        while (file_object_t -> next != NULL){
            file_object_t = file_object_t -> next;
            fd++;
        }
        file_object_t -> next = (file_object *)vmalloc(sizeof(file_object));
        fd++;
        file_object_t = file_object_t -> next;
    }
    
    file_object_t -> inode_index = open_inode_index;
    file_object_t -> position = 0;
    file_object_t -> next = NULL;
    
    
    // build new path
    strcpy(temppathp, FDT_t->cur_path);
    temppathp += strlen(FDT_t->cur_path);
    if (strcmp(FDT_t->cur_path, "/"))
    {
        *temppathp = '/';
        temppathp++;
        *temppathp = '\0';
    }
    
    back = 0;
    while (strncasecmp(&backup[b], "../", 3) == SUCCESS)
    {
        back++;
        b += 3;
    }
    
    while (back > 0)
    {
        for (temppathp -= 2; *temppathp != '/'; temppathp--);
        back--;
        temppathp++;
    }
    
    if (strncasecmp(backup, "./", 2) == SUCCESS)
        b += 2;
    
    if (strncasecmp(backup, "/", 1) == SUCCESS)
    {
        b ++;
        temppathp = &temppath[1];
    }
    strcpy(temppathp, &backup[b]);
    vfree(FDT_t->cur_path);
    FDT_t->cur_path = (char *)vmalloc(strlen(temppath) + 1);
    strcpy(FDT_t->cur_path, temppath);
    
    return fd;
}

int rd_close(int fd)
{
    my_inode *close_inode;
    FDT *FDT_t;
    int i;
    file_object *file_object_t;
    file_object *fomer;

    FDT_t = get_FDT();
    file_object_t = FDT_t->file_object_list_head;
    fomer = NULL;
    if (fd == 0)
    {
        if (file_object_t->inode_index == 0)
            return ERROR;
        close_inode = &inode_ptr[file_object_t->inode_index];
        file_object_t->inode_index = close_inode->parent_index;
        file_object_t->position = 0;
        for (i = strlen(FDT_t->cur_path); i > 0; i--)
            if (FDT_t->cur_path[i] == '/')
                break;
        FDT_t->cur_path[i + 1] = '\0';
        return SUCCESS;
    }
    for (i = 0; i < fd; i++)
    {
        if (file_object_t == NULL)
            return ERROR;
        fomer = file_object_t;
        file_object_t = file_object_t->next;
    }
    fomer->next = file_object_t->next;
    vfree(file_object_t);
    return SUCCESS;
}

int rd_read(int fd, char *output, int num_bytes)
{
    FDT *FDT_t;
    file_object *file_object_t;
    my_inode *des_inode;
    char *readPointer;
    int i;
    int readBytes;
    char *tempoutput;
    int position;
    int remain;
    int iterationRead;
    
    FDT_t = get_FDT();
    file_object_t = FDT_t->file_object_list_head;
    tempoutput = output;
    i = 0;

    if (fd == 0)
        return ERROR;
    for (i = 0; i < fd; i++)
    {
        if (file_object_t == NULL)
            return ERROR;
        file_object_t = file_object_t->next;
    }
    if (file_object_t == NULL)
        return ERROR;
    
    des_inode = &inode_ptr[file_object_t->inode_index];
    
    if (des_inode->size == 0)
        return ERROR;
    if (des_inode->location[0] == NULL)
        return ERROR;
    
    position = file_object_t->position >= des_inode->size ? 0 : file_object_t->position;
    
    readBytes = (des_inode->size - position) < num_bytes ? (des_inode->size - position) : num_bytes;
    num_bytes = readBytes;
    *(output + num_bytes) = '\0';
    
    while (readBytes > 0)
    {
        readPointer = get_blockpointer(position, des_inode);
        if (readPointer == NULL)
            return ERROR;
        remain = block_size - position % block_size;
        iterationRead = readBytes < remain ? readBytes : remain;
        
        memcpy(tempoutput, readPointer, iterationRead);
        tempoutput += iterationRead;
        readBytes -= iterationRead;
        position += remain;
    }
    
    return num_bytes;
}

int rd_write(int fd, char *input, int num_bytes)
{
    FDT *FDT_t;
    int i;
    file_object *file_object_t;
    int ret;

    FDT_t = get_FDT();   
    file_object_t = FDT_t->file_object_list_head;

    if (fd == 0)
        return ERROR;
    for (i = 0; i < fd; i++)
    {
        if (file_object_t == NULL)
            return ERROR;
        file_object_t = file_object_t->next;
    }
    if (file_object_t == NULL)
        return ERROR;


    ret = write_file(file_object_t->inode_index, &(file_object_t->position), input, num_bytes, 'w');

    printk("write to file type: %s", (&inode_ptr[file_object_t->inode_index])->type);

    
    return ret;
}

int rd_lseek(int fd, int position)
{
    FDT *FDT_t;
    my_inode *des_inode;
    file_object *file_object_t;
    int size;
    int i;

    FDT_t = get_FDT();
    file_object_t = FDT_t->file_object_list_head;
    
    if (fd == 0)
        return ERROR;
    for (i = 0; i < fd; i++)
    {
        if (file_object_t == NULL)
            return ERROR;
        file_object_t = file_object_t->next;
    }
    if (file_object_t == NULL)
        return ERROR;
    
    des_inode = &inode_ptr[file_object_t->inode_index];
    size = des_inode->size;
    
    if (size < position)
        position = size;
    
    file_object_t->position = position;

    printk("lseek to position %d\n", file_object_t->position);


    return SUCCESS;
}

int rd_unlink(char *pathname)
{
    int des_inode_index;
    int delet_inode_index;
    FDT *FDT_t;
    char *parentfile[20];
    char *childfile;
    int i;         // for any loop
    my_inode *des_inode;
    int temp;
    int *position;

    temp = -1;
    position = &temp;

    printk("pathname %s\n", pathname);

    // (4) if you attempt to unlink the root directory file.
    if (!strcmp(pathname, "/"))
        return ERROR;

    
    FDT_t = get_FDT();
    
    for (i = 0; i < 20; i++)
        parentfile[i] = NULL;
    
    des_inode_index = read_pathname(pathname, FDT_t, parentfile, &childfile);
    
    for (i = 0; i < 20; i++)
        printk("parentfile[i] %s\n", parentfile[i]);

    printk("childfile %s", childfile);

    // (1) if the pathname does not exist
    for (i = 0; parentfile[i] != NULL; i++)
        if ((des_inode_index = isExist(parentfile[i], des_inode_index, position)) == ERROR)
            return ERROR;
    
    printk("a");
    // check whether child is legal, name lenth and whether exist the same name
    if (strlen(childfile) > 13)
        return ERROR;
    printk("b");
    // check whether the file need to delet exist
    if ((delet_inode_index = isExist(childfile, des_inode_index, position)) == ERROR)
        return ERROR;
    
    printk("c");
    des_inode = &inode_ptr[delet_inode_index];
    
    // (2) if you attempt to unlink a non-empty directory file,
    if (!strcmp(des_inode->type, dir) && des_inode->size > 0)
        return ERROR;
    
    printk("5");
    // (3) if you attempt to unlink an open file
    if (isOpen(delet_inode_index, FDT_t) == ERROR)
        return ERROR;
    
    printk("6");
    free_inode(childfile, delet_inode_index); // change the num of vfree inode and first available
    
    printk("7");
    return SUCCESS;
}

int rd_chmod(char *pathname, mode_t mode)
{
    int des_inode_index;
    my_inode *inode_t;
    FDT *FDT_t;
    char *parentfile[20];
    char *childfile;
    int i;         // for any loop
    int temp;
    int *position;
    
    temp = -1;
    position = &temp;
    
    FDT_t = get_FDT();
    for (i = 0; i < 20; i++)
        parentfile[i] = NULL;
    
    des_inode_index = read_pathname(pathname, FDT_t, parentfile, &childfile);
    printk("rd_chmod a %d\n", des_inode_index);

    for (i = 0; parentfile[i] != NULL; i++)
        if ((des_inode_index = isExist(parentfile[i], des_inode_index, position)) == ERROR)
            return ERROR;

    printk("rd_chmod b\n");

    if ((des_inode_index = isExist(childfile, des_inode_index,  position)) == ERROR) {
        printk("rd_chmod c %d\n", des_inode_index);
        return ERROR;
    }

        

    
    inode_t = &inode_ptr[des_inode_index];
    inode_t -> mode = mode;
    
    return SUCCESS;
}
