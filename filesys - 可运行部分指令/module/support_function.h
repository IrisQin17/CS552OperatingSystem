#include "data_structure.h"

FDT* FDT_list_head = NULL;

extern superblock *superblock_ptr;
extern my_inode *inode_ptr;
extern bitmap *bitmap_ptr;
extern void *space_ptr;



// whether the file is opened
int isOpen(int inode_index, FDT* FDT_ptr);
// whether file exits
int isExist(char* filename, int parent_index, int* directory_index);
// search directory entry in blocks pointed by location[]
int search(uint32_t* location[], int location_size, int* times, char* filename, int* directory_index);

// create a new FDT
FDT* create_FDT(void);
// get the FDT of current process
FDT* get_FDT(void);
// analyze the pathname into folders and file
int read_pathname(char* pathname, FDT* FDT_ptr, char* parentlist[20], char** filename);
// write the file or folder
int write_file(int inode_index, int* position, char* input_content, int input_size, char type);

// switch related address into physical address
uint32_t* switchAddress(my_inode* object_inode, int block_num, int block_offset);
// get the physical address of No.position block in inode
uint8_t* get_blockpointer(int position, my_inode* object_inode);

// allocate a bit of bitmap
int allocate_bitmap(int new_block);
// allocate a new block
uint8_t* allocate_block(my_inode* object_inode);
// allcate a new inode
int allocate_inode(int parent_inode_index, int mode, char type[4]);

// release all FDTs
int free_FDT(void);
// release the inode of the file or folder
int free_inode(char* filename, int inode_index);
// release the all of the block of the file or folder
int free_block(my_inode* object_inode);
// release the bitmap of the block pointed by location
int free_bitmap(uint32_t* location);
// release the directory entry of the parent folder of the file or folder
int free_parent_directory_entry(char* filename, int parent_inode_index);




int isOpen(int inode_index, FDT* FDT_ptr) {
	// arguments declarations
	file_object* fo;

	// iterate file object linked list in FDT
	fo = FDT_list_head->file_object_list_head;
	while(fo != NULL && fo->inode_index != inode_index) {
		fo = fo->next;
	}
	if(fo == NULL) return SUCCESS;
	else return ERROR;
}

int isExist(char* filename, int parent_index, int* directory_index) {
	// arguments declarations
	int parent_size;
	int child_num;
	int* times;
	int ans;
	uint32_t** single_indirect;
	uint32_t*** double_indirect;
	int i;

	printk("isExist1");

	// filename is too long
	if(strlen(filename) > 13) return ERROR;

		printk("isExist2");


	// parent folder has no child
	parent_size = (&inode_ptr[parent_index])->size;
	child_num = parent_size / 16;
	if(child_num <= 0) return ERROR;

		printk("isExist3\n");
		printk("child_num = %d\n",child_num);
		printk("filename = %s\n",filename);


	// the maximum search times
	times = &child_num;

	// search blocks pointed by direct pointer
	ans = search((uint32_t**)((&inode_ptr[parent_index])->location), 8, times, filename, directory_index);
	if(ans != ERROR) return ans;

	printk("isExist4");

	// unfound
	if(*times <= 0) return ERROR;

	// search blocks pointed by single-indirect pointer
	single_indirect = (uint32_t**)((&inode_ptr[parent_index])->location[8]);
	if(single_indirect == NULL) return ERROR;
	ans = search(single_indirect, 64, times, filename, directory_index);
	if(ans != ERROR) return ans;

	// unfound
	if(*times <= 0) return ERROR;

		printk("isExisT5");


	// search blocks pointed by double-indirect pointer
	double_indirect = (uint32_t***)((&inode_ptr[parent_index])->location[9]);
	if(double_indirect == NULL) return ERROR;
	// iterate all blocks of single-indirect pointer pointed by double-indirect pointer
	for(i = 0 ; i < 64 ; i++) {
		single_indirect = double_indirect[i];
		// unfound
		if(single_indirect == NULL) return ERROR;

		ans = search(single_indirect, 64, times, filename, directory_index);
		if(ans != ERROR) return ans;

		// unfound
		if(*times <= 0) return ERROR;
	}

		printk("isExist6");


	return ERROR;
}

int search(uint32_t* location[], int location_size, int* times, char* filename, int* directory_index) {
	// arguments declarations
 	directory_entry* entry;
 	int i, j;

	// iterate all blocks pointed by location[]
	// the number of direcotry entry searched should be smaller than *times
 	for(i = 0 ; i < location_size && *times > 0 ; i++) {
 		entry = (directory_entry*)location[i];
 		if(entry == NULL) return ERROR;

 		// iterate all 16 entries in one block
 		for(j = 0 ; j < 16 && *times > 0; j++) {
 			(*directory_index)++;
 			if((entry + j)->filename == NULL) {
 				return ERROR;
 			}

 			printk("%s", (entry + j)->filename);

 			// return the inode index of the file
 			if(strcmp(((entry + j)->filename), filename) == 0) {
 				return (entry + j)->inode_index;
 			}
 			(*times)--;
 		}
 	}
 	return ERROR;
}

FDT* create_FDT(void) {
	// arguments declarations
	FDT* new_FDT;

	// allocate a memory space to FDT
	new_FDT = (FDT*)vmalloc(sizeof(FDT));
	if(new_FDT == NULL) return NULL;

	// fill in FDT
	// cur_path is root folder, the first file object is root folder
	new_FDT->pid = current->pid;
	new_FDT->cur_path = (char*)vmalloc(2);	// 这里大神写的是3
	strcpy(new_FDT->cur_path, "/");
	new_FDT->next = NULL;

	new_FDT->file_object_list_head = (file_object*)vmalloc(sizeof(file_object));
	new_FDT->file_object_list_head->inode_index = 0;
	new_FDT->file_object_list_head->position = 0;
	new_FDT->file_object_list_head->next = NULL;

	return new_FDT;
}

FDT* get_FDT(void) {
	// arguments declarations
	int cpid;
	int ppid;
	FDT* cFDT_ptr;
	FDT* lFDT_ptr;
	FDT* pFDT_ptr;
	FDT* new_FDT;
	file_object* new_fo;
	file_object* p_fo;

	// the first process
	if(FDT_list_head == NULL) {
		FDT_list_head = create_FDT();
		if(FDT_list_head != NULL) return FDT_list_head;
		else return NULL;
	}

	// search a FDT with pid == current process id
	cpid = current->pid;
	ppid = current->parent->pid;

	cFDT_ptr = FDT_list_head;
	lFDT_ptr = NULL;
	pFDT_ptr = NULL;

	// iterate the linked list of FDT
	while(cFDT_ptr != NULL && cFDT_ptr->pid != cpid) {
		// find the FDT of parent process
		if(cFDT_ptr->pid == ppid) pFDT_ptr = cFDT_ptr;
		// record the last FDT
		lFDT_ptr = cFDT_ptr;
		cFDT_ptr = cFDT_ptr->next;
	}

	// 接下来的代码跟大神有一点不同

	// find the FDT of current process
	if(cFDT_ptr != NULL && cFDT_ptr->pid == cpid) {
		return cFDT_ptr;		
	} 

	// create a new FDT from FDT of parent process
	// no FDT of parent process
	if(pFDT_ptr == NULL) {
		lFDT_ptr->next = create_FDT();
		return lFDT_ptr->next;
	}
	// find the FDT of parent process
	else {
		new_FDT = (FDT*)vmalloc(sizeof(FDT));
		lFDT_ptr->next = new_FDT;

		// fill in the new FDT
		new_FDT->pid = cpid;
		new_FDT->cur_path = (char*)vmalloc(strlen(pFDT_ptr->cur_path) + 1);
		strcpy(new_FDT->cur_path, pFDT_ptr->cur_path);
		new_FDT->next = NULL;

		// copy the file object linked list
		new_FDT->file_object_list_head = (file_object*)vmalloc(sizeof(file_object));
		new_fo = new_FDT->file_object_list_head;
		p_fo = pFDT_ptr->file_object_list_head;

		while(p_fo->next != NULL) {
			*(new_fo) = *(p_fo);
			new_fo->next = (file_object*)vmalloc(sizeof(file_object));
			new_fo = new_fo->next;
			p_fo = p_fo->next;
		}
		*(new_fo) = *(p_fo);
		new_fo->next = NULL;

		return new_FDT;
	}
	return NULL;
}

// 这里改了蛮多，可能会有bug，debug的时候先看这里
int layer_check(char *pathname)
{
    char *p;
    int counter = 0;
    for (p = pathname; *p != '\0'; p++)
        if (*p == '/')
            counter++;
    return counter;
}
int read_pathname(char* pathname, FDT* FDT_ptr, char* parentlist[20], char** childfile) {
	char *pn = pathname;
    int back = 0;
    my_inode *des_inode;     // the destination inode
    int des_index;          // the index of destination inode
    des_index = FDT_ptr->file_object_list_head->inode_index;
    des_inode = &inode_ptr[des_index];
    int length;


    // adjust the destination inode

    while (strncasecmp(pn, "../", 3) == 0)
    {
        back++;
        pn +=3;
    }

    if (back > 0 && back > layer_check(FDT_ptr->cur_path))
        return ERROR;

    while (back > 0)
    {
        des_index = des_inode->parent_index;
        des_inode = &inode_ptr[des_index];
        back--;
    }

    if (strncasecmp(pn, "./", 2) == 0)
        pn += 2;

    if (strncasecmp(pn, "/", 1) == 0)
    {
        pn ++;
        des_index = 0;
    }

    // partition, home/share/as4->home\0share\0as4\0
    length = strlen(pn);
    int i, j, k;
    k = 0;
    for (i = 0, j = 0; i < length; i++)
    {
        if (pn[i] != '/')
            j++;
        else
        {
            parentlist[k] = &pn[i - j];
            parentlist[k][j] = '\0';
            j = 0;
            k++;
        }

    }

    *childfile = &pn[i - j];

    printk("!!!!!childfile %s\n", *childfile);

    return des_index;

	// // arguments declarations
	// char names[20][14];
	// int i = 0, name_num = 0, j = 0, fn_index = 0;
	// int first_index;

	// // cut the pathname with '/'
	// while(i < strlen(pathname)) {
	// 	if(i != 0 && pathname[i] == '/') {
	// 		names[name_num][j] = '\0';
	// 		name_num++;
	// 		j = 0;
	// 	}
	// 	else if(pathname[i] != '/'){
	// 		names[name_num][j] = pathname[i];
	// 		j++;
	// 		// set the start index of last filename
	// 		fn_index = fn_index + 1 == i ? fn_index : i;
	// 	}
	// 	i++;
	// }

	// // set filename
	// *filename = &pathname[fn_index];

	// // the first folder is root, start with "/"
	// first_index = -1;
	// if(strncasecmp(pathname, "/", 1) == 0) {
	// 	first_index = 0;
	// }
	// // the first folder is current folder, start with "../", "f1/", "./""
	// else {
	// 	first_index = FDT_list_head->file_object_list_head->inode_index;
	// }

	// // iterate all folder name
	// // don't support pathname like "f1/../f2/d1.txt"
	// for(i = 0, j = 0 ; i < name_num ; i++) {
	// 	// stay in current folder
	// 	if(strcmp(names[i], ".") == 0) {
	// 		continue;
	// 	}
	// 	// go back to parent folder
	// 	else if(strcmp(names[i], "..") == 0) {
	// 		// current folder is already root folder, error
	// 		if(first_index == 0) return ERROR;

	// 		first_index = (&inode_ptr[first_index])->parent_index;
	// 	}
	// 	else {
	// 		strcpy(parentlist[j], names[i]);
	// 		j++;
	// 	}
	// }

	// return first_index;
}

uint32_t* switchAddress(my_inode* object_inode, int block_num, int block_offset) {
	// arguments declarations
	uint32_t* add = NULL;
	uint32_t** single_indirect;
	uint32_t*** double_indirect;

	// in blocks pointed by direct ptr
	if(block_num < 8) {
		add = (uint32_t*)((char*)(object_inode->location[block_num]) + block_offset);
	}
	// in blocks pointed by single-indirect ptr
	else if(block_num < 72) {
		single_indirect = (uint32_t**)object_inode->location[8];

		if(single_indirect == NULL) return NULL;

		add = (uint32_t*)((char*)(single_indirect[block_num - 8]) + block_offset);
	}
	// in blocks pointed by double-indirect ptr
	else {
		double_indirect = (uint32_t***)object_inode->location[9];

		if(double_indirect == NULL) return NULL;

		add = (uint32_t*)((char*)(double_indirect[(block_num - 72) / 64][(block_num - 72) % 64]) + block_offset);
	}
	return add;
}

uint8_t* get_blockpointer(int position, my_inode* object_inode) {
	// arguments declarations
	int block_num;
	int block_offset;
	uint32_t** single_indirect;
	uint32_t*** double_indirect;
	uint8_t* ans;

	block_num = position / block_size;
	block_offset = position % block_size;

	// if position < 0
	if(position < 0) return NULL;;

	// pointed by direct pointer
	if(block_num < 8) {
		if(object_inode->location[block_num] == NULL) return NULL;

		ans = (uint8_t*)(object_inode->location[block_num]) + block_offset;
	}
	// pointed by single-indirect ptr
	else if(block_num < 72) {
		single_indirect = (uint32_t**)object_inode->location[8];

		if(single_indirect == NULL) return NULL;

		ans = (uint8_t*)(single_indirect[block_num - 8]) + block_offset;
	}
	// pointed by double-indirect ptr
	else {
		double_indirect = (uint32_t***)object_inode->location[9];

		if(double_indirect == NULL) return NULL;

		ans = (uint8_t*)(double_indirect[(block_num - 72) / 64][(block_num - 72) % 64]) + block_offset;
	}

	return ans;
}

int write_file(int inode_index, int* position, char* input_content, int input_size, char type) {
	// arguments declarations
	my_inode* object_inode;
	int block_num;
	int block_offset;
	uint8_t* w_ptr;
	int w_size;

	object_inode = &inode_ptr[inode_index];

	// try to write a directory
	if(type == 'w' && !strcmp(object_inode->type, dir)) {
		return ERROR;
	}

	printk("write_file1\n");

	// cannot allocate a new block
	if(object_inode->location[0] == NULL && allocate_block(object_inode) == NULL) {
		return ERROR;
	}

	printk("write_file2\n");

	// calculate the location to write
	block_num = *position / block_size;
	block_offset = *position % block_size;

	printk("write_file 2.5\n");

	w_ptr = (uint8_t*)switchAddress(object_inode, block_num, block_offset);

	// need a new block at the beginning
	if(w_ptr == NULL) w_ptr = allocate_block(object_inode);

	printk("write_file3\n");

	// test the input size
	w_size = 0;
	while(w_size < input_size) {
		if(block_offset == block_size) {
			w_ptr = allocate_block(object_inode);

			printk("write_file3.5\n");

			if(w_ptr == NULL) {
				break;
			}
			else {
				block_offset = 0;
				continue;
			}
		}
		// write one byte
		*w_ptr = *(input_content + w_size);
		w_ptr++;
		w_size++;
		block_offset++;
		(*position)++;

		printk("write_file4 %d\n", w_size);
	}

	// modify inode
	// if(object_inode->block_number > block_num) {

	// }
	object_inode->size += w_size;

	object_inode->block_number = object_inode->size / block_size;
	object_inode->last_block_offset = object_inode->size % block_size;

	printk("write_file5\n");

	return w_size;
}

int allocate_bitmap(int new_block) {
	int index_num;
	int index_offset;
	int temp;
	int i;
	int j;

	// the position of bit to modify
	index_num = new_block / 8;
	index_offset = new_block % 8;

	// modify the bit
	temp = 0x80;
	temp >>= index_offset;
	bitmap_ptr->index[index_num] |= temp;

	// get the next free bit, next free block
	for(i = index_num, j = 8; i < bitmap_sublock && j == 8 ; i++) {
		for(j = 0 ; j < 8 ; j++) {
			if(bitmap_ptr->index[i] && (0x80 >> j)) break;
		}
	}
	i--;

	new_block = i * 8 + j;
	if(new_block > allocatable_space_size) new_block = -1;

	// modify superblock
	superblock_ptr->freeBlock_number--;
	superblock_ptr->first_freeBlock = (uint16_t)new_block;

	return SUCCESS;

}

uint8_t* allocate_block(my_inode* object_inode) {
	// arguments declarations
	int block_num;
	int new_block;
	uint8_t* new_block_ptr1;
	uint8_t* new_block_ptr2;
	uint8_t* new_block_ptr3;
	uint32_t** single_indirect;
	uint32_t*** double_indirect;
	int double_index;
	int single_index;

	// the new block number
	block_num = object_inode->block_number + 1;

	// the file is too large
	if(block_num > max_file_size / block_size) return NULL;

	// no free block
	if(superblock_ptr->freeBlock_number < 1) return NULL;

	// need 2 blocks for a single-indirect pointer
	// because block_number == 0 means there is one block, its initial number is -1
	if(block_num == 8 && superblock_ptr->freeBlock_number < 2) return NULL;

	// need 3 blocks for a double-indirect pointer
	if(block_num == 72 && superblock_ptr->freeBlock_number < 3) return NULL;

	// get a new block, modify bitmap
	new_block = superblock_ptr->first_freeBlock;
	allocate_bitmap(new_block);

	// modify inode info
	object_inode->block_number++;
    object_inode->last_block_offset = 0;

	// modify the location ptr in inode
	if(block_num < 8) {
		new_block_ptr1 = (uint8_t*)space_ptr + new_block * block_size;
		object_inode->location[block_num] = new_block_ptr1;
		
		return new_block_ptr1;
	}
	else if(block_num < 72) {
		// the first block pointed by single-indirect pointer
		if(object_inode->location[8] == NULL) {
			// first new block for direct pointer pointed by single-indirect pointer
			new_block_ptr1 = (uint8_t*)space_ptr + new_block * block_size;
			single_indirect = (uint32_t**)new_block_ptr1;
			object_inode->location[8] = new_block_ptr1;

			// second new block for content
			new_block = superblock_ptr->first_freeBlock;
			allocate_bitmap(new_block);

			// copy the block pointer to the block pointed by single-indirect pointer
			new_block_ptr2 = (uint8_t*)space_ptr + new_block * block_size;
			*single_indirect = (uint32_t*)new_block_ptr2;

			return new_block_ptr2;
		}
		else {
			// copy the block pointer to block pointed by single-indirect pointer
			new_block_ptr1 = (uint8_t*)space_ptr + new_block * block_size;
			single_indirect = (uint32_t**)object_inode->location[8];
			single_indirect[block_num - 8] = (uint32_t*)new_block_ptr1;

			return new_block_ptr1;
		}
	}
	else {
		// the first block pointed by double-indirect pointer
		if(object_inode->location[9] ==NULL) {
			// first new block for single-indirect pointer of double-indirect pointer
			new_block_ptr1 = (uint8_t*)space_ptr + new_block * block_size;
			double_indirect = (uint32_t***)new_block_ptr1;
			object_inode->location[9] = new_block_ptr1;

			// second new block for direct pointer pointed by single-indirect pointer
			new_block = superblock_ptr->first_freeBlock;
			allocate_bitmap(new_block);
            
			new_block_ptr2 = (uint8_t*)space_ptr + new_block * block_size;
			*double_indirect = (uint32_t**)new_block_ptr2;

			// third new block for content
			new_block = superblock_ptr->first_freeBlock;
			allocate_bitmap(new_block);
			new_block_ptr3 = (uint8_t*)space_ptr + new_block * block_size;
			**double_indirect = (uint32_t*)new_block_ptr3;

			return new_block_ptr3;
		}
		else {
			double_indirect = (uint32_t***)object_inode->location[9];

			double_index = (block_num - 72) / 64;
			single_index = (block_num - 72) % 64;

			// the first block pointed by signle-indirect pointer 
			if(double_indirect[double_index] == NULL) {
				// first new block for direct pointer pointed by single-indirect pointer
				new_block_ptr1 = (uint8_t*)space_ptr + new_block * block_size;
				double_indirect[double_index] = (uint32_t**)new_block_ptr1;

				// second new block for content
				new_block = superblock_ptr->first_freeBlock;
				allocate_bitmap(new_block);
				new_block_ptr2 = (uint8_t*)space_ptr + new_block * block_size;
				double_indirect[double_index][0] = (uint32_t*)new_block_ptr2;

				return new_block_ptr2;
			}
			else {
				new_block_ptr1 = (uint8_t*)space_ptr + new_block * block_size;
				double_indirect[double_index][single_index] = (uint32_t*)new_block_ptr1;
				return new_block_ptr1;
			}
		}
	}
	return NULL;
}

int allocate_inode(int parent_inode_index, int mode, char type[4]) {
	// arguments declarations
	my_inode* new_inode;
	int i;
	
	// allocate a new inode
	new_inode = inode_ptr + superblock_ptr->first_freeInode;

	// modify inode content
	strcpy(new_inode->type, type);
	new_inode->size = 0;
	new_inode->protection = 0;
	new_inode->operation = 0x100;
	new_inode->parent_index = (uint16_t)parent_inode_index;
	new_inode->block_number = -1;
	new_inode->last_block_offset = 0;
    new_inode->mode = mode;

	// find a new first_freeInode
	i = superblock_ptr->first_freeInode + 1;
	for( ; i < inode_list_num ; i++) {
		if(inode_ptr[i].operation == 0) break;
	}
	// modify superblock
	superblock_ptr->first_freeInode = i;
	superblock_ptr->freeInode_number--;

	return SUCCESS;
}

int free_FDT() {
	// arguments declarations
	FDT* FDT_temp;
	file_object* temp;

	// iterate all FDTs
	while(FDT_list_head != NULL) {
		FDT_temp = FDT_list_head->next;
		// iterate all file objects
		while(FDT_list_head->file_object_list_head != NULL) {
			temp = FDT_list_head->file_object_list_head->next;
			vfree(FDT_list_head->file_object_list_head);
			FDT_list_head->file_object_list_head = temp;
		}

		vfree(FDT_list_head->cur_path);
		vfree(FDT_list_head);
		FDT_list_head = FDT_temp;
	}
	return SUCCESS;
}

int free_bitmap(uint32_t* location) {
	// arguments declarations
	int block_num;
	int bitmap_byte_num;
	int bitmap_bit_num;
	uint8_t* bitmap_location;
	int freeindex;

	// calculate the bit location on bitmap
	block_num = (int)(((uint8_t*)location - (uint8_t*)space_ptr) / block_size);
	bitmap_byte_num = block_num / 8;
	bitmap_bit_num = block_num % 8;

	// change bitmap
	bitmap_location = &(bitmap_ptr->index[bitmap_byte_num]);
	*bitmap_location = *bitmap_location | (0x08 ^ (0xff >> bitmap_bit_num));

	// modify superblock
	superblock_ptr->freeBlock_number++;
	freeindex = bitmap_byte_num * 8 + bitmap_bit_num;
	if(superblock_ptr->first_freeBlock > freeindex) {
		superblock_ptr->first_freeBlock = (uint16_t)freeindex;
	}

	return SUCCESS;
}

int free_inode(char* filename, int inode_index) {	
	// arguments declarations
	my_inode* object_inode;

	object_inode = &inode_ptr[inode_index];

	// free block
	free_block(object_inode);

	// free parent directory entry
	free_parent_directory_entry(filename, object_inode->parent_index);

	// modify inode
	strcpy(object_inode->type, nul);
	object_inode->size = 0;
	object_inode->protection = 0;
	object_inode->operation = 0x0000;
    object_inode->parent_index = 0;
    object_inode->block_number = 0;
    object_inode->last_block_offset = 0;
    object_inode->mode = -1;

    // modify superblock
    superblock_ptr->freeInode_number++;
    if(superblock_ptr->first_freeInode > inode_index) {
    	superblock_ptr->first_freeInode = (uint16_t)inode_index;
    }

    return SUCCESS;
}

int free_block(my_inode* object_inode) {
	// arguments declarations
	int i;
	int j;
	uint32_t** single_indirect;
	uint32_t*** double_indirect;

	// no file content
	if(object_inode->block_number < 0) {
		return SUCCESS;
	}

	// iterate all direct pointers
	for(i = 0 ; i <= 8 ; i++) {
		if(object_inode->location[i] == NULL) break;
		// set block content to zero
		memset(object_inode->location[i], 0, block_size);
		free_bitmap(object_inode->location[i]);
	}
	
	if(object_inode->block_number < 8) return SUCCESS;

	// iterate all single-indirect pointers
	single_indirect = (uint32_t**)(object_inode->location[8]);
	for(i = 0 ; i < 64 ; i++) {
		if(!single_indirect[i]) break;
		memset(single_indirect[i], 0, block_size);
		free_bitmap(single_indirect[i]);
	}

	if(object_inode->block_number < 72) return SUCCESS;

	// iterate all double-indirect pointers
	double_indirect = (uint32_t***)object_inode->location[9];
	for(i = 0 ; i < 64 ; i++) {
		if(!double_indirect[i]) break;
		for(j = 0 ; j < 64 ; j++) {
			if(!double_indirect[i][j]) break;
			memset(double_indirect[i][j], 0, block_size);
			free_bitmap(double_indirect[i][j]);
		}
	}

	return SUCCESS;
}

int free_parent_directory_entry(char* filename, int parent_inode_index) {
	// arguments declarations
	int directory_index;
	int* tempptr;
	my_inode* parent_inode;
	int block_num;
	int block_offset;
	int last_block_num;
	int last_block_offset;
	directory_entry* entry;
	directory_entry* lastentry;

	// test whether parent exits, change position to the directory entry index of file in parent
	directory_index = -1;
	tempptr = &directory_index;
	if(isExist(filename, parent_inode_index, tempptr) == ERROR) {
		return ERROR;
	}

	parent_inode = &inode_ptr[parent_inode_index];

	// the directory entry of the object file
	block_num = directory_index / 16;
	block_offset = (directory_index % 16)* 16;

	// the last directory entry
	last_block_num = parent_inode->block_number;
	last_block_offset = parent_inode->last_block_offset - 16;

	if(last_block_offset == 0) {
		
	}
	else {
		printk("last_block_offset %d", last_block_offset);

		// get the physical address of two entries
		entry = (directory_entry*)(switchAddress(parent_inode, block_num, block_offset));
		lastentry = (directory_entry*)(switchAddress(parent_inode, last_block_num, last_block_offset));
		
		// copy the last entry to the object file entry
		strcpy(entry->filename, lastentry->filename);
		entry->inode_index = lastentry->inode_index;

		printk("move %s\n", entry->filename);
	}

	parent_inode->last_block_offset -= 16;
	parent_inode->size -= 16;

	return SUCCESS;
}







