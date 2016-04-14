//calculate factorial

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	//int i;
	//char* buf = argv[1];
	int inp = argc;
	int i=0;
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
	cprintf("Factorial(%d) : %d\n" ,inp , fact);
	
}
