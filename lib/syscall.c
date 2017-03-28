#include "lib.h"
#include "types.h"
#include <sys/syscall.h>
#include <stdarg.h>

/*/*
 * io lib here
 * 库函数写在这
 */

#define BLOCK_SIZE 1024

static int ptr=0;
static char buff[BLOCK_SIZE];


int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;

	/* 内嵌汇编 保存 num, a1, a2, a3, a4, a5 至通用寄存器*/

	asm volatile("int $0x80"
					:"=a"(ret) 
					:"a"(num), 
					"b"(a1), 
					"c"(a2), 
					"d"(a3),
					"S"(a4),
					"D"(a5));
		
	return ret;
}

void addString2Buff(char *str)
{
	while(*str)
	{
		buff[ptr++]=*(str++);
	}
}


void addCh2Buff(char ch)
{
	buff[ptr++]=ch;
}

void addDec2Buff(int a)
{
	if(a<0)
	{
		if(a==-2147483648)
		{	
			addString2Buff("-2147483648");
			return;
		}
		addCh2Buff('-');
		addDec2Buff(-a);
		return ;
	}
	if(a==0)
		return ;
	addDec2Buff(a/10);
	buff[ptr++]=(a%10+'0');
}

void addHex2Buff(unsigned a)
{
	if(a==0)
		return ;
	addHex2Buff(a/16);
	a=a%16;
	if(a<10)
		buff[ptr++]=a+'0';
	else
		buff[ptr++]=a-10+'a';
}


void printf(char *format,...){
	char tempChar;
	int tempInt;
	char *tempString;



	va_list vp;
	va_start(vp,format);

	char*stepPointer=format;
	while(*stepPointer)
	{
		if(*stepPointer=='%')
		{
			stepPointer++;
			switch(*stepPointer)
			{

				case 'c':
					tempChar=(char)va_arg(vp,int);	 /* need a cast here since va_arg only	*/
                    addCh2Buff(tempChar);				/*takes fully promoted types 			*/
					stepPointer++;
					break;
				case 'd':
					tempInt=va_arg(vp,int);
					if(tempInt==0)
						addCh2Buff('0');
					else
						addDec2Buff(tempInt);
					stepPointer++;
					break;
				case 'x':
					tempInt=va_arg(vp,int);
					if(tempInt==0)
						addCh2Buff('0');
					else
						addHex2Buff(tempInt);
					stepPointer++;
					break;
				case 's':
					tempString = va_arg(vp, char*);
					addString2Buff(tempString);
					stepPointer++;
					break;
				case '%':
					addCh2Buff('%');
					stepPointer++;
					break;
			}
		}
		else
		{
			addCh2Buff(*(stepPointer++));
		}
	}
	va_end(vp);

	buff[ptr]='\0';
	syscall(SYS_write,1,(int32_t)buff,ptr,0,0);
	ptr=0;
}