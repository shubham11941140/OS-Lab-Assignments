#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
	//	Write the code for initializing your read-write lock.
	pthread_mutex_init(&rw->mutex, NULL);
	rw->read_count = 0;
	rw->writer_present = 0;
	rw->waiting_writer_count = 0;
	pthread_cond_init(&rw->reader_can_enter, NULL);
	pthread_cond_init(&rw->writer_can_enter, NULL);
}

void ReaderLock(struct read_write_lock * rw)
{
	//	Write the code for aquiring read-write lock by the reader.
	pthread_mutex_lock(& rw->mutex);
	while(rw->writer_present == 1 || rw->waiting_writer_count > 0)
	{
		pthread_cond_wait(& rw->reader_can_enter, & rw->mutex);
	}
	rw->read_count++;
	pthread_mutex_unlock(& rw->mutex);
}

void ReaderUnlock(struct read_write_lock * rw)
{
	//	Write the code for releasing read-write lock by the reader.
	pthread_mutex_lock(& rw->mutex);
	rw->read_count--;
	if (rw->read_count == 0)
	{
		pthread_cond_broadcast(& rw->writer_can_enter);
	}
	pthread_mutex_unlock(& rw->mutex);
}

void WriterLock(struct read_write_lock * rw)
{
	//	Write the code for aquiring read-write lock by the writer.
	pthread_mutex_lock(& rw->mutex);
	rw->waiting_writer_count++;
	while(rw->read_count > 0 || rw->writer_present==1)
	{
		pthread_cond_wait(& rw->writer_can_enter, & rw->mutex);
	}
	rw->waiting_writer_count--;
	rw->writer_present=1;
	pthread_mutex_unlock(& rw->mutex);
}

void WriterUnlock(struct read_write_lock * rw)
{
	//	Write the code for releasing read-write lock by the writer.
	pthread_mutex_lock(& rw->mutex);
	rw->writer_present = 0;
	if (rw->waiting_writer_count == 0)
	{
		pthread_cond_broadcast(& rw->reader_can_enter);
	}
	else
	{
		pthread_cond_signal(& rw->writer_can_enter);
	}
	pthread_mutex_unlock(& rw->mutex);
}
