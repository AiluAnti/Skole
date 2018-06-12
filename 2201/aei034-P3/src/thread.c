/*
 * Implementation of locks and condition variables
 */

#include "common.h"
#include "util.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"

void lock_init(lock_t * l)
{
    /*
     * no need for critical section, it is callers responsibility to
     * make sure that locks are initialized only once
     */
    l->status = UNLOCKED;
    l->waiting = NULL;
}

/* Acquire lock without critical section (called within critical section) */
static void lock_acquire_helper(lock_t * l)
{
	// Check if locked 
	if (l->status == UNLOCKED)
	{
		l->status = LOCKED;
	}
	else
	{
		block(&l->waiting);
	}
}

void lock_acquire(lock_t * l)
{
	enter_critical();

	lock_acquire_helper(l);

	leave_critical();
}

static void lock_release_helper(lock_t * l)
{
	// Check if queue is empty
	if (l->waiting == NULL)
	{
		// Unblock the next element in the queue
		l->status = UNLOCKED;	
	}
	else
	{
		unblock(&l->waiting);
	}
}

void lock_release(lock_t * l)
{
	enter_critical();

	lock_release_helper(l);

	leave_critical();
}

/* condition functions */

void condition_init(condition_t * c)
{
	c->waiting = NULL;
}

/*
 * unlock m and block the thread (enqued on c), when unblocked acquire
 * lock m
 */
void condition_wait(lock_t * m, condition_t * c)
{
	enter_critical();

	lock_release_helper(m);
	block(&c->waiting);

	lock_acquire_helper(m);

	leave_critical();	
}

/* unblock first thread enqued on c */
void condition_signal(condition_t * c)
{
	enter_critical();
	if(c->waiting != NULL)
		unblock(&c->waiting);
	leave_critical();
}

// Unblock first queued item
static void condition_broadcast_helper(condition_t * c)
{
	while (c->waiting != NULL)
		unblock(&c->waiting);
}

/* unblock all threads enqued on c */
void condition_broadcast(condition_t * c)
{
	enter_critical();
	condition_broadcast_helper(c);
	leave_critical();
}

/* Semaphore functions. */
void semaphore_init(semaphore_t * s, int value)
{
	s->waiting = NULL;
	s->num = value;
}

void semaphore_up(semaphore_t * s)
{
	enter_critical();
	s->num++;
	if (s->waiting != NULL)
		unblock(&s->waiting);
	leave_critical();
}

void semaphore_down(semaphore_t * s)
{
	enter_critical();
	s->num--;
	if (s->num < 0)
		block(&s->waiting);
	leave_critical();
}

/*
 * Barrier functions
 * Note that to test these functions you must set NUM_THREADS
 * (kernel.h) to 9 and uncomment the start_addr lines in kernel.c.
 */
static void barrier_wait_helper(barrier_t * b)
{
	// Increment counter
	b->counter++;
	// If everyone has arrived
	if (b->counter == b->limit)
	{
		while (b->waiting != NULL)
			unblock(&b->waiting);
		b->counter = 0;
	}
	// If not, block until that happens
	else
		block(&b->waiting);
}

/* n = number of threads that waits at the barrier */
void barrier_init(barrier_t * b, int n)
{
	b->waiting = NULL;
	b->limit = n;
	b->counter = 0;
}

/* Wait at barrier until all n threads reach it */
void barrier_wait(barrier_t * b)
{
	enter_critical();
	barrier_wait_helper(b);
	leave_critical();
}

