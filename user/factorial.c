//calculate factorial

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	// cprintf("argc: %d\n",argc-1);
	// int i;
	// for(i = 1; i < argc ;i++)
	// 	cprintf("%s\n",argv[i]);
	if(argc<=1)
	{
		cprintf("Enter a number\n");
		return;
	}
	int i;
	int inp = strtol(argv[1],NULL,10);
	int inp1 = inp;
	// while(buf[i]!= '\0')
	// {
	// 	inp = inp*10 + int(buf[i])-48;
	// }
	long long int fact = 1;
	while(inp!=0)
	{
		fact = fact*inp;
		inp--;			
	}
	cprintf("Factorial(%d) : %d\n" ,inp1 , fact);
	
}