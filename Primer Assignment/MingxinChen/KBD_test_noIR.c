#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define IOCTL_CHAR_WHILE _IOW(0, 6, struct ioctl_KBD_WHILE)

int main () {
	struct ioctl_KBD_WHILE{
		char keys;
	};

	struct ioctl_KBD_WHILE ioc;

  	int fd = open ("/proc/ioctl_test", O_RDONLY);
  	int over = 1;

	while(over){
	  	ioctl (fd, IOCTL_CHAR_WHILE, &ioc);
	  	char getchar= ioc.keys;
	  	if (getchar=='\n') {
		  	printf("\n getchar!");
		  	over = 0;		  
	  	}
	  	else{
		  	printf("user program: %c\n",getchar);
	  	}
  	}
  	return 0;
}