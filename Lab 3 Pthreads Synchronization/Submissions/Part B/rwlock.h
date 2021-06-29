#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>

using namespace std;

struct read_write_lock
{
    int read_count;
    pthread_mutex_t mutex;
    pthread_cond_t reader_can_enter, writer_can_enter;
    int writer_present;
    int waiting_writer_count;
};

void InitalizeReadWriteLock(struct read_write_lock * rw);
void ReaderLock(struct read_write_lock * rw);
void ReaderUnlock(struct read_write_lock * rw);
void WriterLock(struct read_write_lock * rw);
void WriterUnlock(struct read_write_lock * rw);
