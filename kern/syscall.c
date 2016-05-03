/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/syscall.h>
#include <kern/console.h>
#include <kern/sched.h>
#include <kern/env.h>

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
static void
sys_cputs(const char *s, size_t len)
{
	// Check that the user has permission to read memory [s, s+len).
	// Destroy the environment if not.

	// LAB 3: Your code here.
	user_mem_assert(curenv,s,len,PTE_P|PTE_U);
	//cprintf("assert chill\n");
	// Print the string supplied by the user.
	cprintf("%.*s", len, s);
}

// Read a character from the system console without blocking.
// Returns the character, or 0 if there is no input waiting.
static int
sys_cgetc(void)
{
	return cons_getc();
}

// Returns the current environment's envid.
static envid_t
sys_getenvid(void)
{
	return curenv->env_id;
}

// Destroy a given environment (possibly the currently running environment).
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_destroy(envid_t envid)
{
	int r;
	struct Env *e;

	if ((r = envid2env(envid, &e, 1)) < 0)
		return r;
	env_destroy(e);
	return 0;
}

// Deschedule current environment and pick a different one to run.
static void
sys_yield(void)
{
	sched_yield();
}

// Allocate a new environment.
// Returns envid of new environment, or < 0 on error.  Errors are:
//	-E_NO_FREE_ENV if no free environment is available.
//	-E_NO_MEM on memory exhaustion.
static envid_t
sys_exofork(void)
{
	//cprintf("FORK called by %08x!\n",curenv->env_id);
	// Create the new environment with env_alloc(), from kern/env.c.
	// It should be left as env_alloc created it, except that
	// status is set to ENV_NOT_RUNNABLE, and the register set is copied
	// from the current environment -- but tweaked so sys_exofork
	// will appear to return 0.

	// LAB 4: Your code here.
	struct Env* child_env;
	int ret;
	if((ret = env_alloc(&child_env,curenv->env_id)) < 0)
		return ret;
	child_env->env_status = ENV_NOT_RUNNABLE;
	
	//set up the eip of the child
	//child_env->env_tf.tf_eip = curenv->env_tf.tf_eip;
	//child_env->env_tf.tf_regs = curenv->env_tf.tf_regs;
	child_env->env_tf = curenv->env_tf;
	child_env->env_tf.tf_regs.reg_eax = 0;

	return child_env->env_id;
	//panic("sys_exofork not implemented");
}
static envid_t
sys_mkguest(void *entry){
	
	struct Env* e;
	int r;
	if((r = env_alloc(&e,curenv->env_id)) < 0)
		return r;
	e->env_status = ENV_NOT_RUNNABLE;
	e->env_type = ENV_TYPE_GUEST;
	e->env_tf.tf_eip = (uintptr_t)entry;
	e->env_tf.tf_eflags &= ~FL_IF;
	return e->env_id;
}
// Set envid's env_status to status, which must be ENV_RUNNABLE
// or ENV_NOT_RUNNABLE.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if status is not a valid status for an environment.
static int
sys_env_set_status(envid_t envid, int status)
{
	// Hint: Use the 'envid2env' function from kern/env.c to translate an
	// envid to a struct Env.
	// You should set envid2env's third argument to 1, which will
	// check whether the current environment has permission to set
	// envid's status.

	// LAB 4: Your code here.
	if(status != ENV_RUNNABLE && status != ENV_NOT_RUNNABLE)
		return -E_INVAL;
	struct Env* env;
	int r;
	if((r = envid2env(envid,&env,1)) < 0)
		return r;
	env->env_status = status;
	return 0;
	//panic("sys_env_set_status not implemented");
}

// Set envid's trap frame to 'tf'.
// tf is modified to make sure that user environments always run at code
// protection level 3 (CPL 3) with interrupts enabled.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_trapframe(envid_t envid, struct Trapframe *tf)
{
	// LAB 5: Your code here.
	// Remember to check whether the user has supplied us with a good
	// address!
    struct Env *e;
    if (envid2env(envid, &e, 1) < 0)
        return -E_BAD_ENV;

    if ((tf->tf_eip >= UTOP)) 
        return -1;

    e->env_tf = *tf;
    e->env_tf.tf_eflags |= FL_IF;
    return 0;
}

// Set the page fault upcall for 'envid' by modifying the corresponding struct
// Env's 'env_pgfault_upcall' field.  When 'envid' causes a page fault, the
// kernel will push a fault record onto the exception stack, then branch to
// 'func'.
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
static int
sys_env_set_pgfault_upcall(envid_t envid, void *func)
{
	// LAB 4: Your code here.
	struct Env* e;
	int r;
	if((r = envid2env(envid,&e,1)) < 0)
		return r;
	e->env_pgfault_upcall = func;
	return 0;
	panic("sys_env_set_pgfault_upcall not implemented");
}
int valid_perm(int perm){
	if(perm & ~PTE_SYSCALL)
		return 0;
	if(!(perm & PTE_P) || !(perm & PTE_U))
		return 0;
	return 1;
}

// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of 'envid'.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a
// side effect.
//
// perm -- PTE_U | PTE_P must be set, PTE_AVAIL | PTE_W may or may not be set,
//         but no other bits may be set.  See PTE_SYSCALL in inc/mmu.h.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
//	-E_INVAL if perm is inappropriate (see above).
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
static int
sys_page_alloc(envid_t envid, void *va, int perm)
{
	// Hint: This function is a wrapper around page_alloc() and
	//   page_insert() from kern/pmap.c.
	//   Most of the new code you write should be to check the
	//   parameters for correctness.
	//   If page_insert() fails, remember to free the page you 
	//   allocated!

	// LAB 4: Your code here.
	struct Env* env;
	int r;
	if((r = envid2env(envid,&env,1)) < 0)
		return r;
	int _va = (int)va;
	if(_va%PGSIZE != 0 || _va >= UTOP){
		cprintf("Invalid va!\n");
		return -E_INVAL;
	}
	
	if(!valid_perm(perm)){
		cprintf("Invalid perm!\n");
		return -E_INVAL;
	}
	struct PageInfo* pp = page_alloc(ALLOC_ZERO);
	if(!pp)
		return -E_NO_MEM;
	if((r = page_insert(env->env_pgdir,pp,va,perm)) < 0){
		//free the allocated page
		page_free(pp);
		return r;
	}
	return 0;
	//panic("sys_page_alloc not implemented");
}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate any necessary page tables.
static int
sys_page_map(envid_t srcenvid, void *srcva,
	     envid_t dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//   page_insert() from kern/pmap.c.
	//   Again, most of the new code you write should be to check the
	//   parameters for correctness.
	//   Use the third argument to page_lookup() to
	//   check the current permissions on the page.

	// LAB 4: Your code here.
	int r;
	struct Env *srcenv, *dstenv;
	if((r = envid2env(srcenvid,&srcenv,1)) < 0)
		return r;
	if((r = envid2env(dstenvid,&dstenv,1)) < 0)
		return r;
	int _srcva = (int)srcva , _dstva = (int)dstva;
	if(_srcva >= UTOP || _srcva%PGSIZE != 0)
		return -E_INVAL;
	if(_dstva >= UTOP || _dstva%PGSIZE != 0)
		return -E_INVAL;
	pte_t *pte;
	struct PageInfo *pp = page_lookup(srcenv->env_pgdir,srcva,&pte);
	if(!pp){
		
		return -E_INVAL;
	}
	if(!valid_perm(perm)) {
		return -E_INVAL;
	}
	if( !(*pte & PTE_U) && (perm & PTE_W) > 0){

		return -E_INVAL;
	}
	// insert page into dest page table
	if((r = page_insert(dstenv->env_pgdir,pp,dstva,perm)) < 0){
		
		return r;
	}
	return 0;
	//panic("sys_page_map not implemented");
}

// Unmap the page of memory at 'va' in the address space of 'envid'.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
//	-E_INVAL if va >= UTOP, or va is not page-aligned.
static int
sys_page_unmap(envid_t envid, void *va)
{
	// Hint: This function is a wrapper around page_remove().
	// LAB 4: Your code here.
	struct Env* env;
	int r;
	if((r = envid2env(envid,&env,1)) < 0)
		return r;
	int _va = (int)va;
	if(_va%PGSIZE != 0 || _va >= UTOP)
		return -E_INVAL;
	page_remove(env->env_pgdir,va);
	return 0;
	//panic("sys_page_unmap not implemented");
}

// Try to send 'value' to the target env 'envid'.
// If srcva < UTOP, then also send page currently mapped at 'srcva',
// so that receiver gets a duplicate mapping of the same page.
//
// The send fails with a return value of -E_IPC_NOT_RECV if the
// target is not blocked, waiting for an IPC.
//
// The send also can fail for the other reasons listed below.
//
// Otherwise, the send succeeds, and the target's ipc fields are
// updated as follows:
//    env_ipc_recving is set to 0 to block future sends;
//    env_ipc_from is set to the sending envid;
//    env_ipc_value is set to the 'value' parameter;
//    env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
// The target environment is marked runnable again, returning 0
// from the paused sys_ipc_recv system call.  (Hint: does the
// sys_ipc_recv function ever actually return?)
//
// If the sender wants to send a page but the receiver isn't asking for one,
// then no page mapping is transferred, but no error occurs.
// The ipc only happens when no errors occur.
//
// Returns 0 on success, < 0 on error.
// Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist.
//		(No need to check permissions.)
//	-E_IPC_NOT_RECV if envid is not currently blocked in sys_ipc_recv,
//		or another environment managed to send first.
//	-E_INVAL if srcva < UTOP but srcva is not page-aligned.
//	-E_INVAL if srcva < UTOP and perm is inappropriate
//		(see sys_page_alloc).
//	-E_INVAL if srcva < UTOP but srcva is not mapped in the caller's
//		address space.
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in the
//		current environment's address space.
//	-E_NO_MEM if there's not enough memory to map srcva in envid's
//		address space.
static int
sys_ipc_try_send(envid_t envid, uint32_t value, void *srcva, unsigned perm)
{
	int r;
	struct Env* e;
	if((r = envid2env(envid,&e,0)) < 0)
		return r;
	if(!e->env_ipc_recving)
		return -E_IPC_NOT_RECV;
	uintptr_t _srcva = (uintptr_t)srcva;
	pte_t *pte;
	struct PageInfo *pp;
	if(_srcva < UTOP){
		if(_srcva%PGSIZE != 0)
			return -E_INVAL;
		if(!valid_perm(perm)) 
			return -E_INVAL;
		if((pp= page_lookup(curenv->env_pgdir,srcva,&pte)) == NULL)
			return -E_INVAL;
		if((perm & PTE_W) && !(*pte & PTE_W))
			return -E_INVAL;
		if(e->env_ipc_dstva < (void *)UTOP){
			if((r = page_insert(e->env_pgdir,pp,e->env_ipc_dstva,perm)) < 0)
				return r;
		}
		e->env_ipc_perm = perm;		// has to be done after every ipc call
	}	
	e->env_ipc_recving = 0;
	e->env_ipc_value = value;
	e->env_ipc_from = curenv->env_id;
	e->env_status = ENV_RUNNABLE;

	e->env_tf.tf_regs.reg_eax = 0;
	return 0;
	//panic("sys_ipc_try_send not implemented");
}

// Block until a value is ready.  Record that you want to receive
// using the env_ipc_recving and env_ipc_dstva fields of struct Env,
// mark yourself not runnable, and then give up the CPU.
//
// If 'dstva' is < UTOP, then you are willing to receive a page of data.
// 'dstva' is the virtual address at which the sent page should be mapped.
//
// This function only returns on error, but the system call will eventually
// return 0 on success.
// Return < 0 on error.  Errors are:
//	-E_INVAL if dstva < UTOP but dstva is not page-aligned.
static int
sys_ipc_recv(void *dstva)
{
	// LAB 4: Your code here.
	uintptr_t _dstva = (uintptr_t)dstva;
	if(_dstva < UTOP && _dstva%PGSIZE != 0){
		return -E_INVAL;
	}
	curenv->env_ipc_recving = true;
	curenv->env_ipc_dstva = dstva;
	curenv->env_status = ENV_NOT_RUNNABLE;
	sched_yield();
	//panic("sys_ipc_recv not implemented");
	return 0;
}
static uint8_t * get_binary(const char *name){
	uint8_t *binary;
	//cprintf("~~%s\n",name);
	//switch(util_id){
	if(strncmp(name,"fac",3) == 0){
		SET_ENV_BINARY(user_factorial,binary);
		return binary;	
	}
	if(strncmp(name,"fib",3) == 0){
		SET_ENV_BINARY(user_fibonacci,binary);
		return binary;	
	}
	if(strncmp(name,"echo",4) == 0){
		SET_ENV_BINARY(user_echo,binary);
		return binary;	
	}
	if(strncmp(name,"date",4) == 0){
		SET_ENV_BINARY(user_date,binary);
		return binary;	
	}
	if(strncmp(name,"cal",3) == 0){
		SET_ENV_BINARY(user_cal,binary);
		return binary;	
	}
	return 0;	
}
#define MAXLEN 1024
#define MAXARGS 10
static char argv_copy[MAXLEN];
// does not return unless an error
// returns -2 if invalid command, -E_INVAL on other errors
static int sys_exec(int argc, char *argv){
	if(!argv || argc <= 0)
		return -E_INVAL;
	uint8_t *bin = get_binary(argv);
	if(!bin)
		return -2;
	// TODO: add user mem assert to check that we can access argv till 1024
	// without entering kernel space

	envid_t parent_id = curenv->env_parent_id;
	envid_t own_id = curenv->env_id;
	// make a copy of the argv array before destroying the env
	memcpy(argv_copy,argv,MAXLEN);
	
	env_free(curenv);
	
	struct Env* e;
	int ret;
	if((ret = env_alloc(&e,parent_id)) < 0)
		return ret;
	e->env_id = own_id; 
	load_icode(e,bin);
	e->env_type = ENV_TYPE_USER;
	//cprintf("@@@@%s\n",argv_copy+4);
	// push argc and argv on the stack

	char *ustack = (char *)USTACKTOP;
	lcr3(PADDR(e->env_pgdir));
	
	int arg_offset[MAXARGS];
	int i = 0;
	int total_len = 0;
	char *arg = argv_copy;
	//cprintf("%s\n",argv_copy);
	while(i != argc){
		arg_offset[i] = total_len;
		int len = strlen(arg) + 1;	// including the null char
		total_len += len;
		arg += len;
		i++;
	}
	ustack -= total_len;
	memcpy(ustack,argv_copy,total_len*sizeof(char));
	//cprintf("^^^%s\n",ustack);
	uint32_t* ustacki = (uint32_t*)ustack;
	ustacki -= argc;
	for(i = 0; i < argc; i++){
		ustacki[i] = USTACKTOP - (total_len - arg_offset[i]);
	}
	ustacki -= 2;
	ustacki[0] = argc;
	ustacki[1] = (uintptr_t)(ustacki+2);
	lcr3(PADDR(kern_pgdir));
	e->env_tf.tf_esp = (uintptr_t)ustacki;
	env_run(e);
}
// Dispatches to the correct kernel function, passing the arguments.
int32_t
syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	// LAB 3: Your code here.

	//panic("syscall not implemented");

	switch (syscallno) {
		case SYS_cputs:
			sys_cputs((char*)a1,(size_t)a2);
			return 0;
		case SYS_cgetc:
			return sys_cgetc();
		case SYS_getenvid:
			return sys_getenvid();
		case SYS_env_destroy:
			return sys_env_destroy((envid_t)a1);
		case SYS_exofork:
			return sys_exofork();
		case SYS_env_set_status:
			return sys_env_set_status((envid_t)a1,a2);
		case SYS_page_alloc:
			return sys_page_alloc(a1,(void *)a2,a3);
		case SYS_page_map:
			return sys_page_map(a1,(void *)a2,a3,(void *)a4,a5);
		case SYS_page_unmap:
			return sys_page_unmap(a1,(void *)a2);
		case SYS_yield:
			sys_yield();
			return 0;
		case SYS_env_set_pgfault_upcall:
			return sys_env_set_pgfault_upcall(a1,(void *)a2);
		case SYS_ipc_recv:
			return sys_ipc_recv((void *)a1);
		case SYS_ipc_try_send:
			return sys_ipc_try_send((envid_t)a1,a2,(void *)a3,(unsigned)a4);
		case SYS_exec:
			return sys_exec(a1,(char*)a2);
		case SYS_mkguest:
			return sys_mkguest((void*)a1);
		default:
			return -E_INVAL;

	}
}

