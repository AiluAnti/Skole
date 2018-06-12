/*
 * memory.c
 * Note: 
 * There is no separate swap area. When a data page is swapped out, 
 * it is stored in the location it was loaded from in the process' 
 * image. This means it's impossible to start two processes from the 
 * same image without screwing up the running. It also means the 
 * disk image is read once. And that we cannot use the program disk.
 *
 * Best viewed with tabs set to 4 spaces.
 */

#include "common.h"
#include "kernel.h"
#include "scheduler.h"
#include "memory.h"
#include "thread.h"
#include "util.h"
#include "interrupt.h"
#include "tlb.h"
#include "usb/scsi.h"


// Function declarations
void make_kernel_page_directory(void);
void make_common_map(uint32_t * page_directory, int user);
void make_stack_map(uint32_t * page_directory, int user);
inline void table_map_present(uint32_t * table, uint32_t vaddr, uint32_t paddr, int user);
inline void directory_insert_table(uint32_t * directory, uint32_t vaddr, uint32_t * table, int user);
void create_user_map(uint32_t * page_directory, pcb_t *p, int user);
static uint32_t *allocate_page(uint32_t v_addr, uint32_t pinned, uint32_t *page_directory, pcb_t *p);
void init_mem_map(void);
static lock_t paging_lock;
static uint32_t next_mem;
static uint32_t *kernel_page_directory = NULL;
static uint32_t valid_accesses;
uint32_t eviction_count = 0;


/* This structure keeps track of values which are of the essence when executing an eviction */
struct mem_map {
  uint32_t  p_addr,
            v_addr,
            busy,
            pinned,
            swap_loc,
            swap_size,
            start_pc,
            *page_dir;
} __attribute__((packed));

typedef struct mem_map mem_map_t;
mem_map_t mem_map[PAGEABLE_PAGES];

inline uint32_t get_directory_index(uint32_t vaddr)
{
  return (vaddr & PAGE_DIRECTORY_MASK) >> PAGE_DIRECTORY_BITS;
}

inline uint32_t get_table_index(uint32_t vaddr)
{
  return (vaddr & PAGE_TABLE_MASK) >> PAGE_TABLE_BITS;
}

/* Initialize page directory for the kernel
 * Also setup the mem_map struct */
void init_memory(void)
{
  next_mem = MEM_START;
  lock_init(&paging_lock);
  init_mem_map();
  make_kernel_page_directory();
}

/* Initialize the values to be kept in mem_map struct */
void init_mem_map(void)
{
  int i;
  for (i = 0; i < PAGEABLE_PAGES; i++)
  {
    mem_map[i].p_addr = (i * PAGE_SIZE) + MEM_START;
    mem_map[i].v_addr = 0;
    mem_map[i].busy = 0;
    mem_map[i].pinned = 0;
    mem_map[i].swap_loc = 0;
    mem_map[i].swap_size = 0;
    mem_map[i].page_dir = NULL;
  }
}

/*
 * Sets up a page directory and page table for a new process or thread. 
 */
void setup_page_table(pcb_t * p)
{
  lock_acquire(&paging_lock);
  // If thread. All threads share the same dir
  if (p->is_thread)
  {
    // Kernel page directory should never be NULL here
    if(!kernel_page_directory)
      HALT("Kernel page directory is not set up!");

    // Assign p->page_dir to be the same as the kernel page dir assigned in the beginning of the code
    p->page_directory = kernel_page_directory;
  }
  // If proc
  else if (!p->is_thread)
  {
    // Create new dir for given proc
    p->page_directory = allocate_page(p->start_pc, 1, NULL, p);

    // Insert kernel page table in proc dir
    make_common_map(p->page_directory, 1);
    // Insert stack page table in proc dir
    make_stack_map(p->page_directory, 1);
    // Insert proc page table in proc dir
    create_user_map(p->page_directory, p, 1);
    
  }
  lock_release(&paging_lock);
}

void page_fault_handler(void)
{
  lock_acquire(&paging_lock);
  current_running->page_fault_count += 1;
  
  /* For convenience */
  uint32_t error_code = current_running->error_code;
  uint32_t fault_addr = current_running->fault_addr;
  uint32_t start_pc = current_running->start_pc;
  uint32_t *page_dir = current_running->page_directory;
  uint32_t swap_loc = current_running->swap_loc;
  uint32_t swap_size = current_running->swap_size;
  
  uint32_t idx = get_directory_index(fault_addr);
  uint32_t user = ~current_running->is_thread;
  uint32_t *page;
  uint32_t offset_bytes;
  uint32_t offset_sector;
  uint32_t img_loc;
  uint32_t num_sec_to_load;
  uint32_t *table_base_address;


  if (error_code % 2 == 0)
  {
    /* Check if fault addr is within available space of proc */
    if ((fault_addr >= start_pc) && (fault_addr < start_pc + swap_size * SECTOR_SIZE))
    {
      /* If this is true, we know the page needs to be loaded from disk */
      if(page_dir[idx] & PE_P)
      {
        /* Allocate a page */
        page = allocate_page(fault_addr, 0, page_dir, current_running); 
        /* Byte offset within proc (page aligned) */
        offset_bytes = (fault_addr & PE_BASE_ADDR_MASK) - start_pc;
        /* Sector offset within proc */
        offset_sector = (offset_bytes / SECTOR_SIZE);
        /* Fault loc on disk/usb */
        img_loc = swap_loc + offset_sector;
        /* Sectors requested capped at page size */
        num_sec_to_load = swap_size - offset_sector;

        if (num_sec_to_load > SECTORS_PER_PAGE)
          num_sec_to_load = SECTORS_PER_PAGE;

        /* Fill page with whatever is on disc */
        scsi_read(img_loc, num_sec_to_load, (char *)page);

        /* Make sure the base address is page aligned */
        table_base_address = (uint32_t *)(page_dir[idx] & PE_BASE_ADDR_MASK);

        /* Set the present bit */
        table_map_present(table_base_address, fault_addr, (uint32_t)page, user);
      }
      else
        HALT("\nPage table is not present!\n");
    }
    else
      HALT("\nTrying to access an invalid address\n");
  }
  else /* Error code is an odd number, which should not be able to happen as this is equal to a seg fault */
    HALT("\nTrying to access an address to which you do not have the privilege\n"); 
  lock_release(&paging_lock);
}


void make_common_map(uint32_t * page_directory, int user)
{
  uint32_t *page_table, addr;

  /* Page table for kernel */
  page_table = allocate_page(current_running->start_pc, 1, page_directory, current_running);

  /* Kernel is present */
  for (addr = 0; addr < (uint32_t) SCREEN_ADDR; addr += PAGE_SIZE)
    table_map_present(page_table, addr, addr, 0);

  /* VGA is present and available to use */
  table_map_present(page_table, (uint32_t) SCREEN_ADDR, (uint32_t) SCREEN_ADDR, user);

  /* Allocate rest of physical memory for the kernel */
  for (addr = MEM_START; addr < MAX_PHYSICAL_MEMORY; addr += PAGE_SIZE)
    table_map_present(page_table, addr, addr, 0);

  directory_insert_table(page_directory, 0, page_table, user);
}

/* This function creates a page table accessible for processes
 * this is where their pages will be located */
void create_user_map(uint32_t * page_directory, pcb_t *p, int user)
{
  uint32_t *page_table = allocate_page(p->start_pc, 1, page_directory, current_running);

  directory_insert_table(p->page_directory, p->start_pc, page_table, user);
}

/* Setup page table and stack pages for the processes
 * Each stack utilizes two pages  */
void make_stack_map(uint32_t * page_directory, int user)
{
  uint32_t *page_table;
  uint32_t *page;
  uint32_t *page2;

  /* Allocate memory for the page table  */
  page_table = allocate_page(PROCESS_STACK, 1, page_directory, current_running);

  /* Allocate memory for the pages */
  page = allocate_page(PROCESS_STACK, 1, page_directory, current_running);
  page2 = allocate_page(PROCESS_STACK - PAGE_SIZE, 1, page_directory, current_running);

  /* Mark the pages as present in the page table */
  table_map_present(page_table, PROCESS_STACK, (uint32_t)page, user);
  table_map_present(page_table, (uint32_t)PROCESS_STACK - PAGE_SIZE, (uint32_t)page2, user);
  
  /* Insert the page table into the given page directory */
  directory_insert_table(page_directory, PROCESS_STACK, page_table, user);  
  //HALT("MAKE STACK MAP");
}

void make_kernel_page_directory(void)
{
  kernel_page_directory = allocate_page(0, 1, current_running->page_directory, current_running);

  make_common_map(kernel_page_directory, 0);
}

static uint32_t *allocate_page(uint32_t v_addr, uint32_t pinned, uint32_t *page_directory, pcb_t *p)
{
  uint32_t *page;
  int i, k;
  uint32_t tidx;
  uint32_t idx;
  uint32_t *table_base_address;
  uint32_t offset_bytes;
  uint32_t offset_sector;
  uint32_t img_loc;
  uint32_t num_sec_to_load;
  uint32_t evicted;

  if (next_mem + PAGE_SIZE <= MAX_PHYSICAL_MEMORY)
  {
    page = (uint32_t *)next_mem;
    next_mem += PAGE_SIZE;
    //v_addr = v_addr & PE_BASE_ADDR_MASK;
    for (i = 0; i < 1024; page[i++] = 0);
    /* Page is ready to be returned */

    k = ((uint32_t)page - MEM_START) / PAGE_SIZE;

    mem_map[k].v_addr = v_addr;
    mem_map[k].pinned = pinned;
    mem_map[k].busy = 1;
    mem_map[k].p_addr = (uint32_t)page;
    mem_map[k].swap_loc = p->swap_loc;
    mem_map[k].swap_size = p->swap_size;
    mem_map[k].start_pc = p->start_pc;
    mem_map[k].page_dir = p->page_directory;

  }
  else
  {
    evicted = rand() % PAGEABLE_PAGES; /* Generates a random number between 0 and 32 */
    
    while (mem_map[evicted].pinned == 1) /* Make sure chosen page is not pinned*/
    {
      if (i > 1000)
        HALT("NO MORE PINNABLE PAGES!"); /* Not a certainty, although very likely */
      evicted = rand() % PAGEABLE_PAGES; /* Choose a random page to evict */
      i++;
    }

    tidx = get_table_index(mem_map[evicted].v_addr); /* Get table index of old vaddr */
    idx = get_directory_index(mem_map[evicted].v_addr); /* Get dir index of vaddr */
    table_base_address = (uint32_t *)(mem_map[evicted].page_dir[idx] & PE_BASE_ADDR_MASK); /* Get table base addr */
    table_base_address[tidx] &= ~PE_P; /* Set as not present */
    
    flush_tlb_entry(mem_map[evicted].v_addr); /* remove the evicted page from the tlb */

    offset_bytes = (mem_map[evicted].v_addr & PE_BASE_ADDR_MASK) - mem_map[evicted].start_pc;
    offset_sector = (offset_bytes / SECTOR_SIZE);
    img_loc = (mem_map[evicted].swap_loc + offset_sector);
    num_sec_to_load = (mem_map[evicted].swap_size - offset_sector);
    page = (uint32_t *)mem_map[evicted].p_addr;

    if (num_sec_to_load > SECTORS_PER_PAGE)
      num_sec_to_load = SECTORS_PER_PAGE;

    if ((uint32_t)table_base_address[tidx] & PE_D)
      scsi_write(img_loc, num_sec_to_load, (char *)page);

    /* Replace values in the evicted struct, to the values from the new page */
    mem_map[evicted].v_addr = v_addr;
    mem_map[evicted].pinned = pinned;
    mem_map[evicted].busy = 1;
    mem_map[evicted].p_addr = (uint32_t)page;
    mem_map[evicted].swap_loc = p->swap_loc;
    mem_map[evicted].swap_size = p->swap_size;
    mem_map[evicted].start_pc = p->start_pc;
    mem_map[evicted].page_dir = p->page_directory;

    for (i = 0; i < 1024; page[i++] = 0);
  }
  return page;
}

inline void table_map_present(uint32_t * table, uint32_t vaddr, uint32_t paddr, int user)
{
  int access = PE_RW | PE_P, index = get_table_index(vaddr);

  if (user)
    access |= PE_US;

  table[index] = (paddr & ~PAGE_MASK) | access;
}

inline void directory_insert_table(uint32_t * directory, uint32_t vaddr, uint32_t * table, int user)
{
  int access = PE_RW | PE_P, index = get_directory_index(vaddr);
  uint32_t taddr;

  if (user)
    access |= PE_US;

  taddr = (uint32_t) table;

  directory[index] = (taddr & ~PAGE_MASK) | access;
}
