#include <inc/lib.h>

// void readseg(envid_t guest,int load_addr, size_t memsz, size_t filesz, int fd,int offset){

// }
// static char data[512];
// static int 
// load_kernel(envid_t guest, char* path)
// {
// 	cprintf("%s\n",path);
// 	int fd = open(path,O_RDONLY);
// 	if(fd < 0){
// 		return -E_NOT_FOUND;	
// 	}
// 	if (readn(fd, data, sizeof(data)) != sizeof(data)) {
// 		close(fd);
// 		return -E_NOT_FOUND;
// 	}
// 	struct Elf *elfhdr = (struct Elf*)data;
// 	if (elfhdr->e_magic != ELF_MAGIC) {
// 		close(fd);
// 		return -E_NOT_EXEC;
// 	}
// 	struct Proghdr* ph = (struct Proghdr*) (data + elfhdr->e_phoff);
// 	struct Proghdr* eph = ph + elfhdr->e_phnum;
// 	int r = 0;
	
// 	cprintf("here\n");
// 	for (; ph < eph; ph++) {
//     	if (ph->p_type == ELF_PROG_LOAD) {
// 			// Call map_in_guest if needed.
// 			cprintf("Mapping kernel: %08x\n",ph->p_pa);
// 			r = readseg(guest, ph->p_pa, ph->p_memsz, fd, ph->p_filesz, ph->p_offset);
// 			if (r < 0) {
// 				close(fd);
// 				return -2;
// 			}
// 		}
// 	}
// 	close(fd);
// 	return r;
// }
void
umain(int argc, char **argv)
{
	int fd, n, r;
	char buf[512+1];

	binaryname = "icode";

	cprintf("icode startup\n");

	cprintf("icode: open /vmm.c\n");
	if ((fd = open("/vmm.c", O_RDONLY)) < 0)
		panic("icode: open /vmm.c: %e", fd);

	cprintf("icode: read /vmm.c\n");
	while ((n = read(fd, buf, sizeof buf-1)) > 0)
		sys_cputs(buf, n);

	cprintf("icode: close /vmm.c\n");
	close(fd);

}