#include "x86.h"
#include "device.h"
#include <sys/syscall.h>

void syscallHandle(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf) {
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */

	asm volatile("movw %%ax,%%es":: "a" (KSEL(SEG_KDATA)));
	asm volatile("movw %%ax,%%ds":: "a" (KSEL(SEG_KDATA)));
	switch(tf->irq) {
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:assert(0);
	}
}

unsigned line=1;
unsigned raw=1;

void putc(char ch){
	if(ch!='\n'){
		line--;raw--;
		int b=(0x0c<<8)+(int)(ch);
		int place=(line*80+raw)*2;
		int g=0xb8000+place;
		char* p;
		p=(char*)g;
		*p=b;
		raw+=1;
		if(raw==80){line+=1;raw=0;}
		line++;raw++;
		//while(1);
	}
	else {
		line+=1;raw=1;
	}
}




void syscallHandle(struct TrapFrame *tf) {
	switch(tf->eax) {
		/* TODO: Add more system calls. */
		case SYS_write:
		{
			if (tf->ebx == 1 || tf->ebx == 2)
			{
				char *buf=(void *)tf->ecx;
				int len=tf->edx;
           		char ch;
           		while (len--) 
           		 {
		           	ch=*(buf++);
		            if (ch == '\0') 
		            	break;
					extern void putChar(char ch);
		            putChar(ch);			
	            }
	        	tf->eax=len;
			}
			else
			{
				assert(0);
			}
			break;
		}
		default: assert(0);
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}
