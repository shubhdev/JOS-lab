//prints command line input

#include <inc/lib.h>
#include <inc/stdio.h>

void umain(int argc , char** argv)
{
	int i;
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] == '\"')
			argv[i] = argv[i]+1;
		if(argv[i][strlen(argv[i])-1] == '\"')
			argv[i][strlen(argv[i])-1] = '\0';
	}
	for(i=1;i<argc;i++)
	{
		cprintf(argv[i]);
		cprintf(" ");
	}
	cprintf("\n");
}