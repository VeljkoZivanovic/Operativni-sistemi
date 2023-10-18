#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
	return fork();
}

int
sys_exit(void)
{
	exit();
	return 0;  // not reached
}

int
sys_wait(void)
{
	return wait();
}

int
sys_kill(void)
{
	int pid;

	if(argint(0, &pid) < 0)
		return -1;
	return kill(pid);
}

int
sys_getpid(void)
{
	return myproc()->pid;
}

int
sys_sbrk(void)
{
	int addr;
	int n;

	if(argint(0, &n) < 0)
		return -1;
	addr = myproc()->sz;
	if(growproc(n) < 0)
		return -1;
	return addr;
}

int
sys_sleep(void)
{
	int n;
	uint ticks0;

	if(argint(0, &n) < 0)
		return -1;
	acquire(&tickslock);
	ticks0 = ticks;
	while(ticks - ticks0 < n){
		if(myproc()->killed){
			release(&tickslock);
			return -1;
		}
		sleep(&ticks, &tickslock);
	}
	release(&tickslock);
	return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
	uint xticks;

	acquire(&tickslock);
	xticks = ticks;
	release(&tickslock);
	return xticks;
}
int
sys_share_data(void)
{
	char *ime; 
	char *adresa;
	int size;
	
 	if (argstr(0, &ime) < 0) 
		return -1;

 	if (argint(2, &size) < 0) 
		return -1;

 	if(argptr(1,&adresa, size) < 0){
		return -1;
	}

 
 	struct proc* curproc = myproc();

	for(int i = 0; i < 10; i++)
		if (strncmp(curproc->shared[i].ime, ime, 10) == 0)
			return -2;

 	for (int pom = 0; pom < 10; pom++) {
 		if (curproc->shared[pom].size == 0) {
 			strncpy(curproc->shared[pom].ime, ime, 10);
 			curproc->shared[pom].adresa = adresa;
 			curproc->shared[pom].size = size;
 			return pom;
 		}
 	}
 
 	return -3;
}

int
sys_get_data(void)
{
 	char *ime;
 	int *adresa;
 
 	if (argstr(0, &ime) < 0) 
		return -1;

 	if (argptr(1, &adresa, sizeof(int*)) < 0) 
		return -1;
 
 	struct proc* curproc = myproc();

 	for (int i = 0; i < 10; i++) {
 		if (curproc->shared[i].size == 0) continue;

		if (strncmp(curproc->shared[i].ime, ime, 10) == 0) {
			*adresa = (uint)curproc->shared[i].adresa;
			return 0;
		}

 	}
 
 	return -2;
}
