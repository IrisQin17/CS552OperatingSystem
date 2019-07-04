#include <multiboot.h>

// data type
typedef unsigned short unit_16;
typedef unsigned long unit_32;

// position to print char
unit_16 x = 0;
unit_16 y = 0;

// get the string length
unit_16 strlen(char* str) {
  unit_16 count;
  for (count = 0; str[count] != '\0'; count++);
  return count;
}

// print string on (x, y)
void printStr(char* str) {
    int size = strlen(str);
    for (int i = 0; i < size; i++) {
      // background color
      unit_16 color = (0 << 4) | (15 & 0x0F);
      // position
      unit_16* position = (unit_16*)0xB8000 + (y * 80 + x);
      x++;
      *position = str[i] | (color<<8);
    }
}

// convert int to decimal string or hex string
void toString(char* str, unit_32 data, unit_32 unit) {
	if(data == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;	
	}
	
  // convert each digit reversally
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
	
  // reverse the string and put '\0' at the end
	int i;
	for(i = 0 ; i < index ; i++) {
		str[i] = revstr[index - i - 1];
	}
	str[index] = '\0';
	return;
}

void init(multiboot_info_t* pmb) {
  // init data
  memory_map_t *mmap;
  unit_32 memsz = 0;
  static char memstr[10];
  unit_32 tmp0 = 0;
  static char tmp[1000];

  // init column and row
	x=0;
	y=4;

  // traverse the memory map
  for (mmap = (memory_map_t *) pmb->mmap_addr; (unsigned long) mmap < pmb->mmap_addr + pmb->mmap_length; mmap = (memory_map_t *) ((unsigned long) mmap + mmap->size + 4)) {   

    // add into memory size if it's available
    if (mmap->type == 1) {
      memsz += mmap->length_low;
    }
	
    // print the memory address map entry
    // // base address
		printStr("Address Range[");
    tmp0 = 0;
		tmp0 += mmap->base_addr_low;
		toString(tmp, tmp0,16);
		printStr(tmp);
	
    // // end address = base address + length
    printStr(":");
    unit_32 endAddr = mmap->base_addr_low + mmap->length_low;
    toString(memstr, endAddr,16);
    printStr(memstr);

    // // state
    printStr("] State: ");
    if(mmap->type == 1) {
      printStr("available");
    }
    else {
      printStr("reserved");
    }
    
    // row increase, set column to 0
    y++;
		x=0;
  }

  // convert memory size in B to MB
  memsz = (memsz >> 20) + 1;

  // print memory size
  toString(memstr, memsz,10);
  y=2;
  x=0;

  printStr("MemOS: Welcome MemOS_2 System memory is: ");
  printStr(memstr);
  printStr("MB");
}

