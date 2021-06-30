/*----------xv6 sync lab----------*/
#include "types.h"
#include "x86.h"
#include "spinlock.h"
#include "defs.h"
#include "barrier.h"

//define any variables needed here
int count, chan;
struct spinlock lk;


int barrier_init(int n)
{
	//to be done
	initlock(&lk, "mylock");
	acquire(&lk);
	count = n;
	chan = 1;
	release(&lk);
	return 0;
}

int barrier_check(void)
{
	//to be done
	acquire(&lk);
	count--;
	if(count==0)
	{
		wakeup(&chan);
	}
	while(count>0)
	{
		sleep(&chan, &lk);
	}
	release(&lk);
	return 0;
}

/*----------xv6 sync lock end----------*/
