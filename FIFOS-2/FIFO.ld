/*                    
 *  CS552 -- Intro to Operating Systems
 *
 *           The FIFOS Operating System 
 *
 *           In part taken from the Quest Operating System
 *  Copyright (C) 2005-2015  Richard West, Boston University
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

OUTPUT_FORMAT("elf32-i386")

OUTPUT_ARCH(i386)

ENTRY(_start)  /* 表示text section的第一个byte就是_start */

SECTIONS {
  /* Bootstrap physical section, at virtual and physical address 0x100000 */
  . = 0x100000 + SIZEOF_HEADERS; /* 从1MB开始*/
        /* 
        * When producing an ELF output file, if the linker script         * uses the SIZEOF_HEADERS builtin function, the linker must         * compute the number of program headers before it has           * determined all the section addresses and sizes.   
        */

  . = ALIGN(0x1000);    /* 限制模块大小最多为 4 KB */
  .text : { *(.multiboot .text) } /* code  */

  . = ALIGN(0x1000);
  .rodata : { *(.rodata) }      /* read-only data */  

  . = ALIGN(0x1000);
  .data : { *(.data) }    /* initialized data */

  . = ALIGN(0x1000);
  .bss : { *(.bss COMMON) } /* uninitialized data */

  . = ALIGN(0x1000);
  .eh_frame (NOLOAD) : { *(.eh_frame) } : NONE  /* 所有的Exception */  

/* 
*  NOLOAD directive will mark a section to not be loaded at run time. 
*  The linker will process the section normally, 
*  but will mark it so that a program loader will not load it into memory.
*/

/*
*  To prevent a section from being assigned to a segment when it would normally default to one, 
*  use :NONE.
*/

}
