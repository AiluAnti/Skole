/** @file memory.c
 *  @brief Implements starting point for a memory hierarchy with caching and RAM.
 *  @see memory.h
 */

#include "memory.h"
#include "cache.h"
#include <stdio.h>

static unsigned long instr_count;
static unsigned long num_reads;

cache_t *L2;
cache_t *L1_data;
cache_t *L1_inst;

unsigned int L2_blocksize = 64;//bytes
unsigned int L2_setsize = 8;
unsigned int L2_cachesize = 262144;//bytes      262144;

unsigned int L1_i_blocksize = 4;//bytes
unsigned int L1_i_setsize = 1;
unsigned int L1_i_cachesize = 4;//bytes     32768

unsigned int L1_d_blocksize = 4;//bytes
unsigned int L1_d_setsize = 2;
unsigned int L1_d_cachesize = 8;//bytes     32768;



void
memory_init(void)
{
  /* Initialize memory subsystem here. */
  L2 = cache_create(L2_cachesize/(L2_setsize*L2_blocksize), L2_setsize, L2_blocksize);
  L1_data = cache_create(L1_d_cachesize/(L1_d_setsize*L1_d_blocksize), L1_d_setsize, L1_d_blocksize);
  L1_inst = cache_create(L1_i_cachesize/(L1_i_setsize*L1_i_blocksize), L1_i_setsize, L1_i_blocksize);
  num_reads = 0;
  instr_count = 0;
}

void
memory_fetch(unsigned int address, data_t *data)
{ 
  //FETCH FROM INSTRUCTION MEMORY
  //printf("memory: fetch 0x%08x\n", address);
  if (data)
    *data = (data_t) 0;


  int l1_has =0;
  int l2_has =0;

  l1_has = cache_read(address, L1_inst);
  if (!l1_has)
  {
    l2_has = cache_read(address, L2);
    if (!l2_has)
    {
      cache_write(address, L2);
    }
    cache_write(address, L1_inst);
  }

  
  instr_count++;
}

void
memory_read(unsigned int address, data_t *data)
{
  //READ FROM DATA MEMORY
  //printf("memory: read 0x%08x\n", address);
  if (data)
    *data = (data_t) 0;
  
  int l1_has=0; 
  int l2_has=0;

  l1_has = cache_read(address, L1_data);
  if (!l1_has)
  {
    l2_has = cache_read(address, L2);
    num_reads++;
    if (!l2_has)
    {
      cache_write(address, L2);
    }
    cache_write(address, L1_data);
  }


  


  instr_count++;
}

void
memory_write(unsigned int address, data_t *data)
{
  //printf("memory: write 0x%08x\n", address);
  //write-through
  cache_write(address, L1_data);
  cache_write(address, L2);




  instr_count++;
}
void print_cache_stats(cache_t *cache)
{
  int hits, miss, total;
  hits = cache->hits;
  miss = cache->miss;
  total= hits+miss;
  printf("===================================\n");
  printf("size(bytes):         | %d\n", cache->size*cache->block_size*cache->set_size);
  printf("linesize(bytes):     | %d\n", cache->block_size);
  printf("associativity:       | %d\n", cache->set_size);
  printf("-----------------------------------\n");
  printf("HITS:                | %d\n", hits);
  printf("MISSES:              | %d\n", miss);
  printf("TOTAL:               | %d\n", total);
  printf("-----------------------------------\n");
  printf("MISSRATE:            | %f\n", (float)miss/(float)(hits+miss));
  printf("===================================\n");
}

void
memory_finish(void)
{
  unsigned int hits = 0;
  unsigned int miss = 0;


  hits += L1_data->hits;
  miss += L1_data->miss;

  //fprintf(stdout, "Executed %lu instructions.\n", instr_count);
  //fprintf(stdout, "L2 processed %lu L1_data requests\n\n", num_reads);

  printf("               .::L1::.\n");
  print_cache_stats(L1_data);
  //printf("\n              .::L2::.\n");
  //print_cache_stats(L2);


  cache_destroy(L2);
  cache_destroy(L1_data);
  cache_destroy(L1_inst);
}
