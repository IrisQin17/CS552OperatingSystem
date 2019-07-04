// process--------------------------------------------
// the structure used to call ioctl
typedef struct {
	char* filename;
	char *pathname;
	int fid;
	int position;
	char* input_content;
	int input_size;
	int flag;
	int mode;
} file_op_arguments;


# define SUCCESS 0
# define ERROR -1

// command code

# define type_number 500
# define IOCTL_PWD _IOR(type_number, 0, char *)
# define IOCTL_LS _IOR(type_number, 1,char **)
# define IOCTL_CREAT _IOWR(type_number, 2, file_op_arguments *)
# define IOCTL_MKDIR _IOWR(type_number, 3, char *)
# define IOCTL_UNLINK _IOWR(type_number, 4, char *)
# define IOCTL_WRITE _IOWR(type_number, 5, file_op_arguments*)
# define IOCTL_READ _IOWR(type_number, 6, file_op_arguments*)
# define IOCTL_READDIR _IOWR(type_number, 7, file_op_arguments*)
# define IOCTL_OPEN _IOWR(type_number, 8, file_op_arguments *)
# define IOCTL_CLOSE _IOWR(type_number, 9, int*)
# define IOCTL_LSEEK _IOWR(type_number, 10, file_op_arguments*)
# define IOCTL_CHMOD _IOWR(type_number, 11, file_op_arguments*)


