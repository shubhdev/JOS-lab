// program to cause a breakpoint trap

#include <inc/lib.h>

void
umain(int argc, char **argv)
{	
	cprintf("Break after this\n");
	asm volatile("int $3");
	cprintf("TESTING1\n");
	cprintf("TESTING2\nNow you can continue");
	int zero = 0;
	cprintf("1/0 is %08x!\n", 1/zero);
}

