//prints command line input

#include <inc/lib.h>
#include <inc/stdio.h>

void umain(int argc , char** argv)
{
	cprintf("In process pid: %d parent pid: %d\n",sys_getenvid(),thisenv->env_parent_id);
	int i;
	for(i=1;i<argc;i++)
	{
		cprintf(argv[i]);
		cprintf(" ");
	}
	cprintf("\n");
}