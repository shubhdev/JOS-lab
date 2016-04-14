#include <inc/lib.h>


#define MAXARGS 10

void runcmd(int argc, char *argv){
	
	if(sys_exec(argc, char* argv) < 0)
		cprintf("FAILED!!!\n");
		return;
}
void
umain(int argc, char **argv)
{
	//cprintf("Yo!\n");
	char *buf;
	while (1) {
		buf = readline("$ ");

		if (buf != NULL)
			if(fork() == 0){
				runcmd(2,"fib 1");
				break;
			}
	}
}
