//prints command line input

#include <inc/lib.h>
#include <inc/stdio.h>

void umain(int argc , char** argv)
{
	int i;
	for(i=1;i<argc;i++)
	{	
		int len = strlen(argv[i]);
		if(len == 1) continue;
		if(argv[i][0] == '\"' && argv[i][len-1] == '\"'){
			argv[i] = argv[i]+1;
			len--;
			argv[i][len-1] = '\0';
		}
		//if(argv[i][strlen(argv[i])-1] == '\"')
	}
	for(i=1;i<argc;i++)
	{
		cprintf(argv[i]);
		cprintf(" ");
	}
	cprintf("\n");
}