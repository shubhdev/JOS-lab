#include <inc/lib.h>

// Waits until 'envid' exits.

int
wait(envid_t pid)
{
	const volatile struct Env *e;

	

	e = &envs[ENVX(pid)];
	if(e->env_parent_id != thisenv->env_id || pid == 0)
		return -1;
	while (e->env_id == pid && e->env_status != ENV_FREE)
		sys_yield();
	return 0;
}
