/*  scheduler.c
 */
#include "common.h"
#include "kernel.h"
#include "scheduler.h"
#include "util.h"

// Call scheduler to run the 'next' process
void yield(void) 
{
	current_running->state = STATUS_READY;
	scheduler_entry();
}


/* The scheduler picks the next job to run, and removes blocked and exited
 * processes from the ready queue, before it calls dispatch to start the
 * picked process.
 */
void scheduler(void)
{
	if(current_running->state == STATUS_READY || current_running->state == STATUS_EXITED)
	{
		// Run next in queue 
		current_running = current_running->next;
	}
	else if(current_running->state == STATUS_BLOCKED)
	{
		// Remove current_running from the ready queue
		current_running->next->previous = current_running->previous;
		current_running->previous->next = current_running->next;

		// Save current running
		pcb_t *tmp = current_running;
		current_running = current_running->next;
		// Nullify the pointers
		tmp->next = NULL;
		tmp->previous = NULL;
	}
	dispatch();
}

/* dispatch() does not restore gpr's it just pops down the kernel_stack,
 * and returns to whatever called scheduler (which happens to be scheduler_entry,
 * in entry.S).
 */
void dispatch(void)
{
	if (current_running->state == STATUS_FIRST_TIME)
	{
		if(current_running->is_thread == 1)
		{
			current_running->state = STATUS_READY;
			asm volatile 
			("movl %0, %%esp"
				:
				:"r" (current_running->k_sp)
				:
			);
		}	
		else
		{
			asm volatile 
			("movl %0, %%esp"
				:
				:"r" (current_running->u_sp)
				:
				);
		}
		asm volatile 
		("jmp %0"
			:
			:"r" (current_running->ip)
			:
		);
	}
}


/* Remove the current_running process from the linked list so it
 * will not be scheduled in the future
 */
void exit(void)
{
	// Change status of current running
	current_running->state = STATUS_EXITED;
	// Move current_running out of the 'ready' queue
	current_running->next->previous = current_running->previous;
	current_running->previous->next = current_running->next;
	// Call scheduler_entry to move on to next process/thread
	scheduler_entry();
}


/* 'q' is a pointer to the waiting list where current_running should be 
 * inserted
 */
void block(pcb_t **q) 
{
	// Change status of current running
	current_running->state = STATUS_BLOCKED;
	
	// If queue is empty
	if ((*q) == NULL)
	{
		(*q) = current_running;
	}
	else
	{
		// Loop through the queue
		pcb_t *tmp = (*q);
		while(tmp->next != NULL)
		{
			tmp = tmp->next;
		}
		// current running will be put at the end of the queue
		tmp->next = current_running;

	}
	// Schedule next process/thread
	scheduler_entry();
}

/* Must be called within a critical section. 
 * Unblocks the first process in the waiting queue (q), (*q) points to the 
 * first process/thread.
 */
void unblock(pcb_t **q) 
{
	// Save the currently first element in the queue
	pcb_t *tmp = (*q);

	// First item in the queue will now be the next element in the queue
	(*q) = (*q)->next;

	// Place what was had the lock behind in the ready queue
	tmp->next = current_running;
	tmp->previous = current_running->previous;

	// Reassign some pointers
	current_running->previous->next = tmp;
	current_running->previous = tmp;

	// Reassign status of newest element in ready queue
	tmp->state = STATUS_READY;
}




