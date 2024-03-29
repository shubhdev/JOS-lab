// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if(!(err & FEC_WR)){
		panic("PGFLT not W%d\n",err);
	}
	if(!(uvpt[PGNUM(addr)] & PTE_COW)){
		panic("not COW\n");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	r = sys_page_alloc(0,PFTEMP,PTE_P|PTE_U|PTE_W);
	if(r < 0){
		panic("sys_page_alloc : %e",r);
	}
	memmove((void *)PFTEMP,ROUNDDOWN(addr,PGSIZE),PGSIZE);
	r = sys_page_map(0,(void *)PFTEMP,0,ROUNDDOWN(addr,PGSIZE),PTE_P|PTE_U|PTE_W);
	if(r < 0){

		panic("syes_page_map : %e\n",r);
	}
	r = sys_page_unmap(0,(void *)PFTEMP);
	if(r < 0){
		panic("sys_page_unmap : %e\n",r);
	}
	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	// LAB 4: Your code here.
	void *addr = (void *)(pn*PGSIZE);
	if((uvpt[pn] & PTE_W) || (uvpt[pn] & PTE_COW)){
		r = sys_page_map(0,addr,envid,addr,PTE_U|PTE_P|PTE_COW);
		if(r < 0) return r;
		r = sys_page_map(0,addr,0,addr,PTE_U|PTE_P|PTE_COW);
		if(r < 0) return r;
	}
	else{
		r = sys_page_map(0,addr,envid,addr,PTE_U|PTE_P);
		if(r < 0)
			return r;
	}
	return 0;
	panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
extern void _pgfault_upcall(void);
envid_t
fork(void)
{
	// LAB 4: Your code here.
	int r;
	set_pgfault_handler(pgfault);
	envid_t id = sys_exofork();
	if(id < 0)
		return id;
	if(id == 0){
		// in child
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	int pgn;
	for(pgn = 0 ; pgn*PGSIZE < USTACKTOP ; pgn++){
		// check that the page is present (else sys_page_map has an error) and is user accessible
		void *addr = (void *)(pgn*PGSIZE);
		if( (uvpd[PDX(addr)] & PTE_P) && (uvpt[pgn] & PTE_P) && (uvpt[pgn] & PTE_U)){
			duppage(id,pgn);
		}
	}

	if( (r = sys_page_alloc(id,(void *)(UXSTACKTOP - PGSIZE),PTE_U|PTE_P|PTE_W)) < 0)
		return r;

	if( (r = sys_env_set_pgfault_upcall(id,_pgfault_upcall)) < 0)
		return r;
	if( (r = sys_env_set_status(id,ENV_RUNNABLE)) < 0)
		return r;
	
	return id;
	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
