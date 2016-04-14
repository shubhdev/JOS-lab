#include <inc/lib.h>


#define MAXARGS 10

// struct cmd {
//   int type;
// };

// struct execcmd {
//   int type;
//   char *argv[MAXARGS];
//   char *eargv[MAXARGS];
// };

char cmd_line[1024];

int isWhiteSpace(char a)
{
	
	if(a==' ' || a=='\t' || a=='\n' || a=='\v' || a=='\r') return 1;
	return 0;
}

int parsecmd(char* cmd_inp)
{
	memset(cmd_line , 0 , 1024);
	int j=0;
	while(isWhiteSpace(cmd_inp[j]))
	{
		j++;
	}
	int cmd_len = strlen(cmd_inp),i=0,sz=0,blank_space =0;
	//empty input
	if(cmd_len == 0) return -1;
	int args = 1;
	for(i=j;i<cmd_len;i++)
	{
		if(isWhiteSpace(cmd_inp[i]))
		{
			blank_space=1;
			continue;
		}

		if(blank_space) 
		{
			cmd_line[sz++] = '\0';
			args +=1;
		}
		cmd_line[sz++] = cmd_inp[i];
		blank_space=0;

	}

	return args;
}

void runcmd(int argc, char *argv){

	if(sys_exec(argc,argv) < 0){
		cprintf("FAILED!!!\n");
		return;
	}
}
void
umain(int argc, char **argv)
{
	//cprintf("Yo!\n");
	char *buf;
	while (1) {
		buf = readline("$ ");

		if (buf[0])
			if(fork() == 0){
				int argc = parsecmd(buf);
				if(argc < 0){
					cprintf("Invalid arguments\n");
					return;
				}
				runcmd(argc,cmd_line);
				break;
			}
	}
}
