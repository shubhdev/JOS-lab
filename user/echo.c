//prints command line input

#include <inc/lib.h>
#include <inc/stdio.h>

void umain(int argc , char** argv)
{
	int i;
	for(i=1;i<argc;i++)
	{
		cprintf(argv[i]);
		cprintf(" ");
	}
}