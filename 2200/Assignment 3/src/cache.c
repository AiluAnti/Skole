#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "cache.h"



unsigned int mung(unsigned int num)
{
	int i=0;
	while (num>1)
	{
		num = num/2;
		i++;
	}
	return i;
}


cache_t *cache_create(unsigned int array_size, unsigned int set_size, unsigned int block_size)
{
	int i, j;

	cache_t *cache = (cache_t*)malloc(sizeof(cache_t));
	if(!cache)
		return NULL;

	cache_block_t *block = NULL;

	cache->array = calloc(sizeof(cache_block_t*), array_size);
	for (i = 0; i<array_size; i++)
	{
		cache->array[i] = calloc(sizeof(cache_block_t), set_size);

		for (j=0; j<set_size;j++)
		{
			cache->array[i][j].valid = 0;
			cache->array[i][j].dirty = 0;
			cache->array[i][j].tag = 0;
			cache->array[i][j].unused = 0;
			cache->array[i][j].data = NULL;
			
		}
	}

	cache->size = array_size;
	cache->set_size = set_size;
	cache->block_size = block_size;


	cache->hits = 0;
	cache->miss = 0;

	return cache;
}
int cache_write(unsigned int address, cache_t *cache)
{
	unsigned int tag, index;
	int i;
	int isincache;
	int write_to_next_level = 0;

	cache_block_t *block;
	index = (address>>(unsigned int)mung(cache->block_size));
	tag = index>>((unsigned int)mung(cache->size));
	index = index % cache->size;
	//printf("Write-Index = %d\n", index);
	//printf("TAG = %d\n", tag);

	int n = 0;
	for(i = 0; i<cache->set_size; i++)
	{
		if (!cache->array[index][i].valid)
		{
			n = i;
			break;
		}
		if (cache->array[index][i].tag == tag)
		{
			n = i;
			break;
		}
		if (i!=n && cache->array[index][i].unused > cache->array[index][n].unused)
		{
			n = i;
		}
	}
	cache->array[index][n].valid = 1;
	cache->array[index][n].tag = tag;
	cache->array[index][n].unused = 0;
	cache->array[index][n].data = NULL;

	//return value relevant for unfinished implementation of writeback
	return write_to_next_level; 
}

int cache_read(unsigned int address, cache_t *cache)
{
	unsigned int tag, index;
	int isincache = 0;
	int i;
	index = (address>>(int)mung(cache->block_size));
	tag = index>>((int)mung(cache->size));
	index = index % cache->size;

	//printf("Read-Index = %d\n", index);
	//printf("TAG = %d\n", tag);
	int valid;
	cache_block_t *block;
	block = &cache->array[index][0];
	valid = block->valid;

	for (i=0; i<cache->set_size; i++)
	{
		block = &cache->array[index][i];
		valid = block->valid;
		if (block->valid == 1)
		{
			//printf("VALID BLOCK DETECTED, CHECK TAG\n");
			if (block->tag == tag)
			{
				block->unused = 0;
				isincache = 1;
				//printf("set-pos =  %d\n", i);
			}
			else
				block->unused++;
		}

	}	
	if (isincache == 1)
	{
		//printf("CACHE HIT\n");
		cache->hits +=1;
	}	
	else //if (valid==1) //-- UNCOMMENT TO ELIMINATE COUNTING OF COLD MISSES
	{
		//printf("CACHE MISS\n");
		cache->miss +=1;
	}

	return isincache;
}


void cache_destroy(cache_t *cache)
{
	int i, j;

	for (i = 0; i<cache->size; i++)
	{
		free(cache->array[i]);
	}

	free(cache->array);
	free(cache);

}
//TEST main function to verify correctness
/*
int main()
{
	cache_t *cache = cache_create(2048/4, 4, 64);
	cache_destroy(cache);
	return 1;
}*/