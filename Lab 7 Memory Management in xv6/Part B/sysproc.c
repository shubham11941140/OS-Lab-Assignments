#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_fork(void)
{
  	return fork();
}

int sys_exit(void)
{
	exit();
	return 0;  // not reached
}

int sys_wait(void)
{
  	return wait();
}

int sys_kill(void)
{
	int pid;

	if(argint(0, &pid) < 0)
		return -1;
	return kill(pid);
}

int sys_getpid(void)
{
  	return myproc()->pid;
}

int sys_sbrk(void)
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

int sys_sleep(void)
{
	int n;
	uint ticks0;

	if(argint(0, &n) < 0)
		return -1;
	acquire(&tickslock);
	ticks0 = ticks;
	while(ticks - ticks0 < n)
	{
		if(myproc()->killed)
		{
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
int sys_uptime(void)
{
	uint xticks;

	acquire(&tickslock);
	xticks = ticks;
	release(&tickslock);
	return xticks;
}


int sys_getNumFreePages(void)
{
  	return getNumFreePages();
}

int sys_getNumVirtPages(void)
{
	uint sz = myproc()->sz;
	int num_logical_pages = (sz / PGSIZE) + (sz%PGSIZE != 0);
	return num_logical_pages;
}

int sys_getNumPhysPages(void)
{
	pde_t *pgdir = myproc()->pgdir;
	int n = 1024; // 10 bits used to index into pdgir
	int phy_pgs = 0;
	for(int i=0; i<n/2; i++)
	{
		// user space pgdir
		pde_t *pde = &pgdir[i];
		if(*pde & PTE_P)
		{
			pte_t *pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
			for(int j=0; j<n; j++)
			{
				// iterate over all page table entries
				pte_t pte = pgtab[j];
				if(pte & PTE_P)
				{
					phy_pgs++;
				}
			}
		}
	}
	return phy_pgs;
}

int sys_getNumPTPages(void)
{
	pde_t *pgdir = myproc()->pgdir;
	int n = 1024; // 10 bits used to index into pdgir
	int ptpgs = 1;
	for(int i=0; i<n; i++)
	{
		pde_t pde = pgdir[i];
		if(pde & PTE_P)
		{
			ptpgs++;
		}
	}
	return ptpgs;
}
