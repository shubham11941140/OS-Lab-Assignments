#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock, pgflt_lock;
uint ticks;

void tvinit(void)
{
	int i;

	for(i = 0; i < 256; i++)
		SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
	SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

	initlock(&tickslock, "time");
	initlock(&pgflt_lock, "page_fault_lock");
}

void idtinit(void)
{
  	lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void trap(struct trapframe *tf)
{
	if(tf->trapno == T_SYSCALL)
	{
		if(myproc()->killed)
			exit();
		myproc()->tf = tf;
		syscall();
		if(myproc()->killed)
			exit();
		return;
	}

	if(tf->trapno == T_PGFLT)
	{
		uint va = rcr2();
		pde_t *pgdir = myproc()->pgdir;
		pte_t *pte, *pde, *pgtab;

		// obtain pte
		pde = &pgdir[PDX(va)];
		pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
		if((*pde & PTE_P) == 0)
			cprintf("invalid page table entry\n");
		pte = &pgtab[PTX(va)];

		acquire(&pgflt_lock);
		if((*pte & PTE_P) == 0)
		{
			cprintf("page not mapped to physical address\n");
		}
		else if((*pte & PTE_W) == 0)
		{
			// copy page
			uint pa = PTE_ADDR(*pte), perm = PTE_FLAGS(*pte);
			if(get_ref_cnt(pa) > 1)
			{
				if(copy_page(pgdir, va, pa, perm) == 0)
				{
					decr_ref_cnt(pa);
				}
				else
					cprintf("page copy failed\n");
			}
			else
			{
				// allow W permission on the page
				*pte |= PTE_W;
				lcr3(V2P(pgdir));
			}
			release(&pgflt_lock);
			return;
		}
		release(&pgflt_lock);
	}


	switch(tf->trapno)
	{
		case T_IRQ0 + IRQ_TIMER:
			if(cpuid() == 0)
			{
				acquire(&tickslock);
				ticks++;
				wakeup(&ticks);
				release(&tickslock);
			}
			lapiceoi();
			break;
		case T_IRQ0 + IRQ_IDE:
			ideintr();
			lapiceoi();
			break;
		case T_IRQ0 + IRQ_IDE+1:
			// Bochs generates spurious IDE1 interrupts.
			break;
		case T_IRQ0 + IRQ_KBD:
			kbdintr();
			lapiceoi();
			break;
		case T_IRQ0 + IRQ_COM1:
			uartintr();
			lapiceoi();
			break;
		case T_IRQ0 + 7:
		case T_IRQ0 + IRQ_SPURIOUS:
			cprintf("cpu%d: spurious interrupt at %x:%x\n", cpuid(), tf->cs, tf->eip);
			lapiceoi();
			break;

		//PAGEBREAK: 13
		default:
			if(myproc() == 0 || (tf->cs&3) == 0)
			{
				// In kernel, it must be our mistake.
				cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n", tf->trapno, cpuid(), tf->eip, rcr2());
				panic("trap");
			}
			// In user space, assume process misbehaved.
			cprintf("pid %d %s: trap %d err %d on cpu %d "
					"eip 0x%x addr 0x%x--kill proc\n", myproc()->pid, myproc()->name, tf->trapno, tf->err, cpuid(), tf->eip, rcr2());
			myproc()->killed = 1;
	}

	// Force process exit if it has been killed and is in user space.
	// (If it is still executing in the kernel, let it keep running
	// until it gets to the regular system call return.)
	if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
		exit();

	// Force process to give up CPU on clock tick.
	// If interrupts were on while locks held, would need to check nlock.
	if(myproc() && myproc()->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
		yield();

	// Check if the process has been killed since we yielded
	if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
		exit();
}
