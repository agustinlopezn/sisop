#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>

#define MAX_REGIONS                                                            \
	4096  // Blocks will always have 4096 regions, independent of the size of the block

#define MAX_BLOCKS 10
#define SMALL_BLOCK_SIZE 16384     // 16KB
#define MEDIUM_BLOCK_SIZE 1048576  // 1MB
#define LARGE_BLOCK_SIZE 33554432  // 32MB
#define MIN_ALLOC_SIZE                                                         \
	SMALL_BLOCK_SIZE / MAX_REGIONS  // 16KB / 4096 = 4 bytes (min region size)

struct region {
	size_t size;
	void *ptr;
	int free;
	struct region *next;
};

struct block {
	size_t size;
	size_t available_size;
	int initialized;
	void *ptr;
	struct region regions[MAX_REGIONS];
	struct block *next;
};

struct memory {
	int initialized;
	struct block small_blocks[MAX_BLOCKS];
	struct block medium_blocks[MAX_BLOCKS];
	struct block large_blocks[MAX_BLOCKS];
};

void *_malloc(size_t size);
void *_calloc(size_t nmemb, size_t size);
void _free(void *ptr);

extern struct memory virtual_memory;