#include "spinlock.h"

/*----------xv6 sync lab----------*/
#define NSEM 10

struct semaphore
{
    int count, chan;
    struct spinlock lk;
};


int sem_init(int, int);
int sem_up(int);
int sem_down(int);
/*----------xv6 sync lab end----------*/
