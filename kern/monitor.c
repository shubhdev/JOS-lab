// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line

struct StackFrame{
	uint32_t caller_ebp;
	uint32_t ret_addr;
	uint32_t param[5];
	//uint32_t param2;
	//uint32_t param3;
};
struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Get the stack trace",mon_backtrace},
	{"continue","If breakpoint, then can continue",mon_continue},
	{"si","single step through instruction",mon_single_step}
};
#define NCOMMANDS (sizeof(commands)/sizeof(commands[0]))

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}
void print_stackframe(uint32_t ebp,struct StackFrame * frame){
	cprintf("    ebp %08x  eip %08x  args:",ebp,frame->ret_addr);
	int i;
	for(i = 0; i < 5; i++) {
		//cprintf("%d ->",i);
		cprintf(" %08x",frame->param[i]);	
	}
	cprintf("\n");
}
void print_funcinfo(uint32_t eip){
	struct Eipdebuginfo info;
	uintptr_t addr = (uintptr_t)eip;
	//cprintf("-----\n");
	debuginfo_eip(addr,&info);
	//kern/monitor.c:101: mon_backtrace+15
	int offset = eip - (uint32_t)info.eip_fn_addr;
	//cprintf("%d\n",offset);
	cprintf("\t%s:%d: %.*s+%d\n",info.eip_file,info.eip_line,info.eip_fn_namelen,info.eip_fn_name,offset);
}

// A dummy function to get value of the eip register of a function. return the ret_address, which is the
//address of the instruction after the call instruction in the caller function
uint32_t __attribute__ ((noinline)) read_eip(){
	uint32_t curr_ebp = read_ebp();
	struct StackFrame * curr_frame = (struct StackFrame *)curr_ebp;
	uint32_t ret = curr_frame->ret_addr;
	return ret; 
}
int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	uint32_t curr_eip = read_eip();
	uint32_t curr_ebp = read_ebp(); //WRONG..Xpoints to addr,just below callers ebpX
	struct StackFrame * curr_frame = NULL; 
	cprintf("Stack backtrace:\n    current eip=%08x\n",curr_eip);
	while (curr_ebp != 0) {
		print_funcinfo(curr_eip);
		curr_frame = (struct StackFrame *)(curr_ebp);
		print_stackframe(curr_ebp,curr_frame);
		//cprintf("=====\n");
		curr_ebp = curr_frame->caller_ebp;
		curr_eip = curr_frame->ret_addr;
	}
	print_funcinfo(curr_eip);
	return 0;
}

int check_debug_trap(struct Trapframe *tf){
	if(!tf) {
		cprintf("Cannot continue.No trapframe found! This suggests did not reach here because of a breakpoint\n");
		return 0;
	}
	if(tf->tf_trapno != T_BRKPT && tf->tf_trapno != T_DEBUG){
		cprintf("Cannot continue, did not reach here via a breakpoint/debug trap!\n");
		return 0;
	}
	return 1;
}
int mon_continue(int argc, char **argv, struct Trapframe *tf){

	if(!check_debug_trap(tf)) return 0;
	//change the eflags register

	tf->tf_eflags &= ~FL_TF;
	//return -1 so loop breaks;
	return -1;
}
int mon_single_step(int argc, char **argv, struct Trapframe *tf){
	if(!check_debug_trap(tf)) return 0;
	tf->tf_eflags |= FL_TF;
	return -1;
}
/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);
	
	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
