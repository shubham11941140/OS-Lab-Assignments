/*----------xv6 sync lab----------*/
#include "types.h"
#include "x86.h"
#include "defs.h"
#include "semaphore.h"

struct semaphore s[NSEM];

int sem_init(int index, int val)
{
	if(index<0 || index>=NSEM)
		return -1;
	initlock(&s[index].lk, "mylock");
	acquire(&s[index].lk);
	s[index].count = val;
	release(&s[index].lk);
	return 0;
}

int sem_up(int index)
{
	if(index<0 || index>=NSEM)
		return -1;
	acquire(&s[index].lk);
	s[index].count++;
	if(s[index].count >= 0)
	{
		wakeup_one(&s[index].chan);
	}
	release(&s[index].lk);
	return 0;
}

int sem_down(int index)
{
	if(index<0 || index>=NSEM)
		return -1;
	acquire(&s[index].lk);
	if(s[index].count == 0)
	{
		sleep(&s[index].chan, &s[index].lk);
	}
	s[index].count--;
	release(&s[index].lk);
	return 0;
}

/*----------xv6 sync lab end----------*/
