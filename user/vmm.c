#include <inc/lib.h>
#include <inc/elf.h>

#define GUEST_ENTRY 0x7000
static int
map_guest(envid_t guest,uintptr_t guestpa, size_t memsz,int fd, size_t filesz,off_t fileoffset)
{
	int i,r=0;
	for (i = 0; i < memsz; i += PGSIZE) {
		if (i >= filesz) {
			// allocate a blank page
			if ((r = sys_page_alloc(guest, (void*) (guestpa + i), PTE_P|PTE_U|PTE_W)) < 0)
				return r;

		} else {
			// from file
			if ((r = sys_page_alloc(0, UTEMP, PTE_P|PTE_U|PTE_W)) < 0)
				return r;
			if ((r = seek(fd, fileoffset + i)) < 0)
				return r;
			if ((r = readn(fd, UTEMP, MIN(PGSIZE, filesz-i))) < 0)
				return r;
			if ((r = sys_page_map(0, UTEMP, guest, (void*) (guestpa + i), PTE_P|PTE_U|PTE_W)) < 0)
				panic("spawn: sys_page_map data: %e", r);
			sys_page_unmap(0, UTEMP);
		}
	}
	return 0;
}

static char buf[512];
int 
load_kernel(envid_t guest, char* path)
{
	cprintf("%s\n",path);
	int fd = open(path,O_RDONLY);
	if(fd < 0){
		return -E_NOT_FOUND;	
	}
	if (readn(fd, buf, sizeof(buf)) != sizeof(buf)) {
		close(fd);
		return -E_NOT_FOUND;
	}
	struct Elf *elfhdr = (struct Elf*)buf;
	if (elfhdr->e_magic != ELF_MAGIC) {
		close(fd);
		return -E_NOT_EXEC;
	}
	struct Proghdr *ph, *eph;

	ph = (struct Proghdr*) (buf + elfhdr->e_phoff);
	eph = ph + elfhdr->e_phnum;
	
	int r = 0;
	
	for (; ph < eph; ph++) {
    	if (ph->p_type == ELF_PROG_LOAD) {
			//cprintf("Mapping kernel: %08x\n",ph->p_pa);
			r = map_guest(guest, ph->p_pa, ph->p_memsz, fd, ph->p_filesz, ph->p_offset);
			if (r < 0) {
				close(fd);
				return -2;
			}
		}
	}
	close(fd);
	return 0;
}

int 
load_bootloader(envid_t guest, char *path){
	cprintf("%s\n",path);
	int fd = open(path,O_RDONLY);
	if(fd < 0){
		return -E_NOT_FOUND;	
	}
	if (readn(fd, buf, sizeof(buf)) != sizeof(buf)) {
		close(fd);
		return -E_NOT_FOUND;
	}
	int r = map_guest(guest,GUEST_ENTRY,512,fd,512,0);
	if(r < 0){
		close(fd);
		return -2;
	}
	return 0;	
}
void
umain(int argc, char **argv)
{
	int guest_id = sys_mkguest((void*)GUEST_ENTRY);
	
	cprintf("----%d\n",guest_id);
	
	// allocate 0-4MB to the guest
	int i,r;
	for(i = 0; i < 1024; i++){
		//i'th page
		if((r = sys_page_alloc(guest_id,(void*)(i*PGSIZE),PTE_P|PTE_U|PTE_W)) < 0){
			cprintf("allocate 4MB failed\n");
		}
	}	

	r = load_bootloader(guest_id,"/boot");
	if(r < 0) cprintf("%e bootloader copy failed\n",r);
	r = load_kernel(guest_id,"/kernel");
	
	if(r < 0) cprintf("%e bootloader copy failed\n",r);
	sys_env_set_status(guest_id,ENV_RUNNABLE);

}