#include "x86.h"
#include "device.h"
#include <sys/syscall.h>


static int line=5;
static int col=0;

#define pattern 0x0c
void showCharInScreen(char ch){
	if(ch!='\n')
	{
		short b=(pattern<<8)+(int)(ch);
		int place=(line*80+col)*2;
		short* p=(short*)(0xb8000+place);
		*p=b;
		col++;
		if(col==80)
		{
			line++;
			col=0;
		}
	}
	else {
		line++;
		col=0;
	}
}

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
		            showCharInScreen(ch);		
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
