#include <inc/lib.h>

// Waits until 'envid' exits.
int
wait(envid_t pid)
{
	const volatile struct Env *e;

	assert(pid != 0);

	e = &envs[ENVX(pid)];
	if(e->env_parent_id != thisenv->env_id)
		return -1;
	while (e->env_id == pid && e->env_status != ENV_FREE)
		sys_yield();
	return 0;
}