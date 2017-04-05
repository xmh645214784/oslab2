#include "x86.h"
#include "device.h"

SegDesc gdt[NR_SEGMENTS];
TSS tss;

#define SECTSIZE 512
#define ELF_START_POS (201*SECTSIZE)

uint32_t loader(char *buf);
void readseg(char *address, int count, int offset);
void init_segment(void);

void waitDisk(void) {
	while((inByte(0x1F7) & 0xC0) != 0x40); 
}

void readSect(void *dst, int offset) {
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

void initSeg() {
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
	gdt[SEG_TSS].s = 0;
	setGdt(gdt, sizeof(gdt));

	/*
	 * 初始化TSS
	 */
	asm volatile("ltr %%ax":: "a" (KSEL(SEG_TSS)));

	/*设置正确的段寄存器*/
	tss.ss0=KSEL(SEG_KDATA);
	tss.esp0=0x200000;

	asm volatile(	"movw %%ax,%%es\n\t"
					"movw %%ax,%%ds\n\t"
					"movw %%ax,%%fs\n\t"
					"movw %%ax,%%ss    "
					:
					 : "a" (KSEL(SEG_KDATA)));
	lLdt(0);
	
}
void enterUserSpace(uint32_t entry) {
	/*
	 * Before enter user space 
	 * you should set the right segment registers here
	 * and use 'iret' to jump to ring3
	 */
	asm volatile(	"movw %%ax,%%es\n\t"
					"movw %%ax,%%fs\n\t"
					"movw %%ax,%%ds    "
					:
					: "a" ((SEG_UDATA<<3)|3));


	asm volatile("pushw %0		\n\t"
				 "pushl %1		\n\t"
				 "pushfl		\n\t"
				 "pushl %2		\n\t"
				 "pushl %3		    "
				 :
				 :"i"((SEG_UDATA<<3)|3),
				  "i"(0x400000), 			//the user stack top
				 					 		//at first I use 0xc0000000 and bad
				  "i"((SEG_UCODE<<3)|3),
				  "m"(entry)
				 );
	asm volatile("iret");
}

void loadUMain(void) {

	/*加载用户程序至内存*/
	char *buf=(char *)0x8000;
	/*read elf from disk*/
	readseg((char*)buf, SECTSIZE, ELF_START_POS);
	uint32_t entry=loader(buf);
	enterUserSpace(entry);
}

void readseg(char *address, int count, int offset) 
{
	char * dst=address+count;
	address -= offset % SECTSIZE;
	offset = (offset / SECTSIZE);
	while(address < dst)
	{
		readSect(address, offset);
		address += SECTSIZE;
		offset++;
	}
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