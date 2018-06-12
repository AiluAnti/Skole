#ifndef CACHE_H
#define CACHE_H

typedef struct cache_block
{
	unsigned int valid;
	unsigned int dirty;
	unsigned int tag;
	unsigned int unused;
	unsigned long *data;
}cache_block_t;

typedef struct cache
{
	cache_block_t **array;
	unsigned int size;
	unsigned int block_size;
	unsigned int set_size;

	//HELPER VARIABLES
	unsigned int hits;
	unsigned int miss;
}cache_t;
unsigned int mung(unsigned int num);
cache_block_t *create_block(unsigned int block_size);
cache_t *cache_create(unsigned int array_size, unsigned int set_size, unsigned int block_size);



int cache_write(unsigned int address, cache_t *cache);
int cache_read(unsigned int address, cache_t *cache);
void cache_destroy(cache_t *cache);


#endif