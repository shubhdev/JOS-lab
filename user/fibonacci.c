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
	long long int fib1=1 , fib2 =1;
	if(inp==1 || inp == 2)
	{
		cprintf("Fibonacci(%d) : %d\n" ,inp1 , 1);
		return;
	}
	for(i=2;i<inp;i++)
	{
		long long int temp = fib2;
		fib2 = fib2 + fib1;
		fib1 = temp;
	}
	cprintf("Fibonacci(%d) : %d\n" ,inp1 , fib2);
	
}
