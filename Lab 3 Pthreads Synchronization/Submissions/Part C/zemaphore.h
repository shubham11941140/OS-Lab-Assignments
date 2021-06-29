#include <pthread.h>
#include <signal.h>
#include <wait.h>
#include <sys/types.h>

typedef struct zemaphore 
{
    int val;
    pthread_cond_t cond;
    pthread_mutex_t lock;

} zem_t;

void zem_init(zem_t *, int);
void zem_up(zem_t *);
void zem_down(zem_t *);
