/*  kernel.c
 */
#include "common.h"
#include "kernel.h"
#include "th.h"
#include "util.h"
#include "scheduler.h"

// Statically allocate some storage for the pcb's
pcb_t       pcb[NUM_TOTAL];
// Ready queue and pointer to currently running process
pcb_t       *current_running;

void (*proc_array[NUM_TOTAL])(void);

void stack_init(int i);
void pcb_init(void);

/* This is the entry point for the kernel.
 * - Initialize pcb entries
 * - set up stacks
 * - start first thread/process
 */
void _start(void)
{
    /* Declare entry_point as pointer to pointer to function returning void 
     * ENTRY_POINT is defined in kernel h as (void(**)()0xf00)
     */
    void (**entry_point) ()     = ENTRY_POINT;
    
    // load address of kernel_entry into memory location 0xf00
    *entry_point = kernel_entry;
    
    // Put threads/processes in the array
    proc_array[0] = clock_thread;
    proc_array[1] = thread2;
    proc_array[2] = thread3;
    proc_array[3] = mcpi_thread0;
    proc_array[4] = mcpi_thread1;
    proc_array[5] = mcpi_thread2;
    proc_array[6] = mcpi_thread3;
    proc_array[7] = PROC1_ADDR;
    proc_array[8] = PROC2_ADDR;

    clear_screen(0, 0, 80, 25);
    pcb_init();
    dispatch();

}

// Initialize stacks dependant on whether it's a thread or a process
void stack_init(int i)
{
    // If element is process
    if (pcb[i].is_thread == 0)
    {
        // Set up user stack
        pcb[i].u_sp = STACK_MIN + (STACK_SIZE * pcb[i].pid) + STACK_OFFSET;

    }
    // Set up kernel stack either way
    pcb[i].k_sp = STACK_MIN + (STACK_SIZE * pcb[i].pid) + STACK_OFFSET;
}


void pcb_init(void) {
    int i;

    // Looping through threads and processes
    for (i = 0; i < NUM_TOTAL; i++)
    {
        pcb[i].ip = proc_array[i];
        // Assign ID for each process or thread
        pcb[i].pid = i;
        // Set state
        pcb[i].state = STATUS_FIRST_TIME;
        // If first process/thread
        if (i == 0)
        {
            // Linking the PCB's in a circle
            pcb[i].previous = &pcb[NUM_TOTAL -1];
            pcb[i].next = &pcb[i+1];
            // Defining current running as the first in the array
            current_running = &pcb[i];
        }
        // If last element in the array
        else if (i == NUM_TOTAL - 1)
        {
            pcb[i].previous = &pcb[i-1];
            // Next elem will be the first in the array
            pcb[i].next = &pcb[0];
        }
        else
        {
            // Covering all other cases
            pcb[i].previous = &pcb[i-1];
            pcb[i].next = &pcb[i+1];
        }
        // If a thread is being initialized
        if (i < NUM_THREADS)
        {
            pcb[i].is_thread = 1;
        }
        else
        {
            // If a process is being initialized 
            pcb[i].is_thread = 0;
        }
        // Initialize stack both for process and thread
        stack_init(i);
    }
}

/*  Helper function for kernel_entry, in entry.S. Does the actual work
 *  of executing the specified syscall.
 */ 

 // Check if syscall is 0 or 1, defined as YIELD and EXIT, respectively 
void kernel_entry_helper(int fn) 
{
    if (fn == 0)
        yield();
    else if (fn == 1)
        exit();
}

