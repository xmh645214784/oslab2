#include "boot.h"
#include <string.h>
#include <stdlib.h>

#define SECTSIZE 512

#define ELF_START_POS SECTSIZE

#define assert(cond) \
	do\
	{\
		;\
	} while (0);

typedef unsigned uint32_t;
typedef unsigned short uint16_t;

uint32_t loader(char *buf);
void readseg(char *address, int count, int offset);


void bootMain(void) {
	char *buf = (char*)0x8000;
	
	/*read elf from disk*/
	// readseg((char*)buf, 4096, ELF_START_POS);
	readSect(buf,1);
	
	void (*entry)(void);
	entry=(void *)loader(buf);
	entry();
	/*never return*/
}

uint32_t loader(char *buf)
{
	ELFHeader *elf=(void *)buf;
	ProgramHeader *ph = NULL;
	uint16_t real_phnum=elf->phnum;
	if(elf->phnum==0xffffU)
	{
		assert(0);
	}
	for(int i=0;i<real_phnum;i++) 
	{
		ph=(void*)(buf+elf->phoff+i*elf->phentsize);
		if(ph->type == 1) /*PT_LOAD*/
		{
			uint32_t Offset=ph->off;
			uint32_t VirtAddr =ph->vaddr;//This is now physical addr.But should be VA
			int FileSiz=ph->filesz;
			int MemSize=ph->memsz;
			readseg((void*)(VirtAddr),FileSiz,ELF_START_POS+Offset);
			if(MemSize-FileSiz>0)
				for (char* i = (char*)VirtAddr + FileSiz; i < (char*)VirtAddr + MemSize; *i ++ = 0);
		}
	}
	volatile uint32_t entry = elf->entry;
	return entry;
}

void waitDisk(void) { // waiting for disk
	while((inByte(0x1F7) & 0xC0) != 0x40);
}

void readSect(void *dst, int offset) { // reading one sector of disk
	int i;
	waitDisk();
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}


/* 将磁盘offset位置的count字节数据读入物理地址pa */
void readseg(char *address, int count, int offset) 
{
	address -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; address < address + count; address += SECTSIZE, offset ++)
		readSect(address, offset);
}