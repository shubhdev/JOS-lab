#include <inc/lib.h>

#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10

// struct cmd {
//   int type;
// };

// struct execcmd {
//   int type;
//   char *argv[MAXARGS];
//   char *eargv[MAXARGS];
// };
void runcmd(int util_id,int arg){
	
	if(sys_exec(util_id,arg) < 0)
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
				runcmd(3,0);
				break;
			}
	}
}
