// buggy program - causes a divide by zero exception

#include <inc/lib.h>

int zero;

void
umain(int argc, char **argv)
{
	zero = 0;
	cprintf("1/0 is %d!\n", 69);
}

