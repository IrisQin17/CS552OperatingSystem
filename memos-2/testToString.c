#include <stdio.h>

typedef unsigned long unit_32;
typedef unsigned short unit_16;

void intToString(char* str, unit_32 data, unit_32 unit)
{
  	/*unit_32 temp = data;
  	unit_32 base = 1;
  	unit_32 quo = temp / base;
  	while(quo > 0)
  	{
    		base *= unit;
    		quo = temp / base;
  	}
  	printf("%d\n", base);
    int i;
  	for (i = 0; base > 0; i++)
  	{
    		temp = data / base;
        if(unit == 10) {
            str[i] = temp + '0';
        }
    		else {
            if(temp < 10) {
                str[i] = temp + '0';
            }
            else {
                str[i] = (temp - 10) + 'a'; 
            }
        }
        printf("%c", str[i]);
    		data -= temp * base;
    		base /= unit; 
  	}
	  str[i] = '\0';*/
	  
	if(data == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;	
	}
	
	static char revstr[32];
	int index = 0;
	while(data != 0) {
		unit_32 temp = data % unit;
		data = data / unit;
		if(unit == 10) {
            revstr[index] = temp + '0';
        }
    		else {
            if(temp < 10) {
                revstr[index] = temp + '0';
            }
            else {
                revstr[index] = (temp - 10) + 'a'; 
            }
        }
        index++;
	}
	
	int i;
	for(i = 0 ; i < index ; i++) {
		str[i] = revstr[index - i - 1];
	}
	str[index] = '\0';
	return;
}

int main() {
    unit_32 data = 21;
    static char str[10];
    intToString(str, data, 10);
    printf("%s", str);

    return 0;
}
