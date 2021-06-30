/*----------xv6 sync lab----------*/
#include "types.h"
#include "x86.h"
#include "uspinlock.h"

//TODO: define an array of NLOCK uspinlocks
uint locks[NLOCK];

int uspinlock_init(void)
{
	// Initialize all locks to unlocked state
	// To be done.
	for(int i=0; i<NLOCK; i++)
	{
		locks[i]=0;
	}
	return 0;
}

int uspinlock_acquire(int index)
{
	// To be done.
	while(xchg(&locks[index], 1)!=0)
	{
		;
	}
	return 0;
}

int uspinlock_release(int index)
{
	// To be done.
	locks[index]=0;
	return 0;
}

int uspinlock_holding(int index)
{
	// To be done.
	//Return 0 if lock is free, 1 if lock is held
	return locks[index];
	return 0;
}
/*----------xv6 sync lock end----------*/
