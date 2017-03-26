void print(char *temp, short line){
	short i;
	line=160*line;
	asm volatile("mov %0, %%si" : : "a" (line) );
	for(i=0;i<80;i++){
		__asm(" movb $0x0c, %bh");
		asm volatile("mov %0, %%bl" : : "a" (temp[i]) );
		__asm(" movw %bx, %gs:(%esi)");
		__asm(" add $2, %si");
	}
}