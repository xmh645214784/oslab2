#include "x86.h"
#include "device.h"

#define INTERRUPT_GATE_32   0xE
#define TRAP_GATE_32        0xF

/* IDT表的内容 */
struct GateDescriptor idt[NR_IRQ]; // NR_IRQ=256, defined in x86/cpu.h

/* 初始化一个中断门(interrupt gate) */
static void setIntr(struct GateDescriptor *ptr, uint32_t selector, uint32_t offset, uint32_t dpl) {
	ptr->offset_15_0 = offset & 0xFFFF;
	ptr->segment = selector << 3;
	ptr->pad0 = 0;
	ptr->type = INTERRUPT_GATE_32;
	ptr->system = FALSE;
	ptr->privilege_level = dpl;
	ptr->present = TRUE;
	ptr->offset_31_16 = (offset >> 16) & 0xFFFF;
}

/* 初始化一个陷阱门(trap gate) */
static void setTrap(struct GateDescriptor *ptr, uint32_t selector, uint32_t offset, uint32_t dpl) {
	ptr->offset_15_0 = offset & 0xFFFF;
	ptr->segment = selector << 3;
	ptr->pad0 = 0;
	ptr->type = TRAP_GATE_32;
	ptr->system = FALSE;
	ptr->privilege_level = dpl;
	ptr->present = TRUE;
	ptr->offset_31_16 = (offset >> 16) & 0xFFFF;
}

/* 声明函数，这些函数在汇编代码里定义 */
void no0();
void no1();
void no2();
void no3();
void no4();
void no5();
void no6();
void no7();
void no8();
void no9();
void no10();
void no11();
void no12();
void no13();
void no14();
void irqEmpty();
void irqGProtectFault();
void irqSyscall();

void initIdt() {
	int i;
	/* 为了防止系统异常终止，所有irq都有处理函数(irqEmpty)。 */
	for (i = 0; i < NR_IRQ; i ++) {
		setTrap(idt + i, SEG_KCODE, (uint32_t)irqEmpty, DPL_KERN);
	}
	/*
	 * init your idt here
	 * 初始化 IDT 表, 为中断设置中断处理函数
	 */
	
	setTrap(idt + 0, SEG_KCODE, (uint32_t)no0, DPL_KERN);
	setTrap(idt + 1, SEG_KCODE, (uint32_t)no1, DPL_KERN);
	setTrap(idt + 2, SEG_KCODE, (uint32_t)no2, DPL_KERN);
	setTrap(idt + 3, SEG_KCODE, (uint32_t)no3, DPL_KERN);
	setTrap(idt + 4, SEG_KCODE, (uint32_t)no4, DPL_KERN);
	setTrap(idt + 5, SEG_KCODE, (uint32_t)no5, DPL_KERN);
	setTrap(idt + 6, SEG_KCODE, (uint32_t)no6, DPL_KERN);
	setTrap(idt + 7, SEG_KCODE, (uint32_t)no7, DPL_KERN);
	setTrap(idt + 8, SEG_KCODE, (uint32_t)no8, DPL_KERN);
	setTrap(idt + 9, SEG_KCODE, (uint32_t)no9, DPL_KERN);
	setTrap(idt + 10, SEG_KCODE, (uint32_t)no10, DPL_KERN);
	setTrap(idt + 11, SEG_KCODE, (uint32_t)no11, DPL_KERN);
	setTrap(idt + 12, SEG_KCODE, (uint32_t)no12, DPL_KERN);
	setTrap(idt + 13, SEG_KCODE, (uint32_t)no13, DPL_KERN);
	setTrap(idt + 14, SEG_KCODE, (uint32_t)no14, DPL_KERN);

	setTrap(idt + 0xd, SEG_KCODE, (uint32_t)irqGProtectFault, DPL_KERN);
	
	setIntr(idt + 0x80, SEG_KCODE, (uint32_t)irqSyscall, DPL_USER); // for int 0x80, interrupt vector is 0x80, Interruption is disabled

	/* 写入IDT */
	saveIdt(idt, sizeof(idt));
}
