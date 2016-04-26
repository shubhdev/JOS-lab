#include <inc/lib.h>

#include <inc/lib.h>

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