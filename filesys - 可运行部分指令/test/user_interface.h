#define USE_RAMDISK
#include "data_structure.h"
#define ReadOnly 0
#define ReadWrite 1
#define WriteOnly 2


// these function are called by test_file.c
// use iotcl in the functions
// return unique fid
int openedRandisk = ERROR;
int count = 0;

int check_pathname(char *pathname);
int rd_chmod(char* pathname, mode_t mode);

int openProcRandisk(void)
{
	if (openedRandisk != ERROR)
		return SUCCESS;

	openedRandisk = open("/proc/Ramdisk", O_RDWR);
	count++;
	if (openedRandisk == ERROR)
	{
		printf("Failed to open Ramdisk!\n");
		return ERROR;
	}
	return SUCCESS;
}

/*int CREAT(char* pathname, void permission) {

}*/

int mode_to_permission(int mode){
    
	if(mode == 292) //RD  (S_IRUSR | S_IRGRP | S_IROTH) = 444(oct) = 292(dec)
		return ReadOnly;
	else if (mode == 420) //RW  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) = 644(oct) = 420(dec)
		return ReadWrite;
	else //WR  (S_IWUSR | S_IRGRP | S_IROTH) = 244(oct) = 164(dec)
		return WriteOnly;

	return ERROR;
}

int rd_creat(char* pathname, mode_t mode) {

	int ret;
	int permission;

	file_op_arguments file_obj;
	file_obj.pathname = pathname;
	permission = mode_to_permission(mode);
	file_obj.mode = permission;
	
    //sprintf (pathname1,  " %d", mode);
    //fprintf (stderr, "creat: mode: %s \n",pathname1);
    // printf ("mode = %d",mode);

	if (openProcRandisk() == ERROR){

		return ERROR;
	}
    //printf ("openProcRandisk");
	if (check_pathname(pathname) == ERROR){

		return ERROR;
	}

    //printf ("check_pathname");
	if ((ret = ioctl(openedRandisk, IOCTL_CREAT, &file_obj)) == ERROR){

		return ERROR;
	}

    //printf ("ioctl");
    // printf ("ret = %d\n",ret);
	return ret;
}

// int MKDIR(char* pathname) {
	
// }

int rd_mkdir(char* pathname) {
	if (openProcRandisk() == ERROR)
		return ERROR;
	if (check_pathname(pathname) == ERROR)
		return ERROR;
	if (ioctl(openedRandisk, IOCTL_MKDIR, pathname) == ERROR)
		return ERROR;
	
    return SUCCESS;
}

// int UNLINK(char* pathname) {


// }

int rd_unlink(char* pathname) {
	if (openProcRandisk() == ERROR)
		return ERROR;

	if (check_pathname(pathname) == ERROR)
		return ERROR;
	if (ioctl(openedRandisk, IOCTL_UNLINK, pathname) == ERROR)
		return ERROR;
	
	return SUCCESS;
}


int rd_open(char* pathname, int flag) {

	int fid;
	int ret;
	int permission;
	fid = 0;

	file_op_arguments file_obj;
	file_obj.pathname = pathname;

	// printf("before flag = %d\n", flag);

	if(flag == 2)
		file_obj.flag = ReadWrite;
	else if(flag == 0)
		file_obj.flag = ReadOnly;
	else
		file_obj.flag = WriteOnly;

	// printf("after flag = %d\n", file_obj.flag);

	if (check_pathname(pathname) == ERROR)
		return ERROR;
	if ((fid=ioctl(openedRandisk, IOCTL_OPEN, &file_obj)) == ERROR)
		return ERROR;

	/*if ((ret=rd_chmod(pathname,mode)) == ERROR)
		return ERROR;*/

	return fid;
 }


int rd_write(int fid, char* input_content, int input_size) {

	file_op_arguments file_obj;

	file_obj.fid = fid;
	file_obj.input_size = input_size;
	file_obj.input_content = input_content;
	// printf("writting bytes %d\n", file_obj.input_size);	

	int ret;
	if (openProcRandisk() == ERROR)
		return ERROR;
	
	ret = ioctl(openedRandisk, IOCTL_WRITE, &file_obj);
	printf("%d words has been writen\n", ret);

	return ret;

 }

// position是从文件头开始算起的bytes偏移量
int rd_lseek(int fid, int position) {

 	file_op_arguments file_obj;
	file_obj.position = position;
	file_obj.fid = fid;
	int ret;
	ret = ioctl(openedRandisk, IOCTL_LSEEK, &file_obj);

	return ret;

}

// int READ(int fd, char* input_content, int input_size) {


// }

int rd_read(int fid, char* addr, int input_size) {

	file_op_arguments file_obj;
	file_obj.fid = fid;
	file_obj.input_size = input_size;
	file_obj.input_content = addr;

	if (openProcRandisk() == ERROR)
		return ERROR;

	// printf("reading bytes %d\n", file_obj.input_size);
	int ret;
	ret = ioctl(openedRandisk, IOCTL_READ, &file_obj);
	addr[input_size] = '\0';

	printf("%d words has been read\n", ret);
	// printf("%s\n", file_obj.input_content);

	return ret;

}


int rd_close(int fid) {
	int ret;
	if (openProcRandisk() == ERROR)
		return ERROR;
	if ((ret=ioctl(openedRandisk, IOCTL_CLOSE, &fid)) == ERROR)
		return ERROR;

	return ret;

}

int rd_chmod(char* pathname, mode_t mode) {

	int ret;
	int fid;
	int permission;
	fid = 0;

	file_op_arguments file_obj;
	file_obj.pathname = pathname;
	permission = mode_to_permission(mode);
	file_obj.mode = permission;

	if (openProcRandisk() == ERROR)
		return ERROR;

	if (check_pathname(pathname) == ERROR)
		return ERROR;

	if ((ret=ioctl(openedRandisk, IOCTL_CHMOD, &file_obj)) == ERROR)
		return ERROR;
	
	return ret;

}


int check_pathname(char *pathname)
{	// check the first character of filename
	// if there is extra space in the delete
	// if begin with non digit, letter, illegal
	char *p;

	//if pathname start with space, trim
	for (p = pathname; *p == ' '; p++);

	// if pathrame == empty, return ERROR
	if (*p == '\0')
		return ERROR;

	//if p != ./0-9 && Capatalized alphebet && decapatalized alphebet
	if ((*p < 46 || *p >57) && (*p <65 || *p > 90) && (*p < 97 || *p > 122))
		return ERROR;

	if (*p == '.')
	{
		//if ".xxx"
		if (*(p+1) != '.' && *(p+1) != '/')
			return ERROR;

		//if "..xxx"
		if (*(p+1) == '.' && *(p+2) != '/')
			return ERROR;
	}

	//pathname
	if (p != pathname)
		strcpy(pathname, p);

	//if pathname end with space, trim
	for (p = pathname + strlen(pathname); p > pathname; p--)
	{	
		if (*p == ' ')
			*p = '\0';

		//if p == ./0-9 && Capatalized alphebet && decapatalized alphebet
		if ((*p > 45 && *p < 58) || (*p > 64 && *p < 91) || (*p > 96 && *p < 123)) 
			break;
	}

	return SUCCESS;
}
