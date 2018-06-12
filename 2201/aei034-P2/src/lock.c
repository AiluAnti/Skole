/* lock.c
 * 
 * Implementation of locks.
 */

#include "common.h"
#include "lock.h"
#include "scheduler.h"

// Initialize lock
void lock_init(lock_t *l)
{
	l->curr = NULL;
	l->state = UNLOCKED;
}

// Acquire lock/block if locked
void lock_acquire(lock_t *l)
{
	// Check if lock is unlocked 
	if (l->state == UNLOCKED)
	{
		// Lock it
		l->state = LOCKED;
	}
	else
	{
		// Block the element
		block(&l->curr);
	}

}

// Let go of the lock & unblock
void lock_release(lock_t *l)
{
	// Check if queue is empty
	if (l->curr == NULL)
	{
		// Unlock the lock
		l->state = UNLOCKED;
	}
	else
	{
		// Unblock the next element in the queue
		unblock(&l->curr);
	}
}
