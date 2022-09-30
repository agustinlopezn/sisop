#include "malloc.h"

struct memory virtual_memory = { 0, 0, 0, 0 };

void
region_init(struct block *block)
{
	size_t min_region_size = block->size / MAX_REGIONS;
	for (size_t i = 0; i < MAX_REGIONS; i++) {
		block->regions[i].size = min_region_size;
		block->regions[i].ptr = block->ptr + i * min_region_size;
		block->regions[i].free = 1;
	}

	for (size_t i = 0; i < MAX_REGIONS - 1; i++) {
		block->regions[i].next = &block->regions[i + 1];
	}
}


void
block_init(struct block *block)
{
	block->available_size = block->size;
	block->ptr = mmap(NULL,
	                  block->size,
	                  PROT_READ | PROT_WRITE,
	                  MAP_PRIVATE | MAP_ANONYMOUS,
	                  -1,
	                  0);
	if (block->ptr == MAP_FAILED) {
		perror("block init");
		return;
	}
	block->initialized = 1;
	region_init(block);
}

void
memory_init()
{
	/* Sets 0 to indicate that's not initialized */
	for (int i = 0; i < MAX_BLOCKS; i++) {
		virtual_memory.small_blocks[i].size = SMALL_BLOCK_SIZE;
		virtual_memory.medium_blocks[i].size = MEDIUM_BLOCK_SIZE;
		virtual_memory.large_blocks[i].size = LARGE_BLOCK_SIZE;

		virtual_memory.small_blocks[i].initialized = 0;
		virtual_memory.medium_blocks[i].initialized = 0;
		virtual_memory.large_blocks[i].initialized = 0;
	}
	/* Updates references */
	for (int i = 0; i < MAX_BLOCKS - 1; i++) {
		virtual_memory.small_blocks[i].next =
		        &virtual_memory.small_blocks[i + 1];
		virtual_memory.medium_blocks[i].next =
		        &virtual_memory.medium_blocks[i + 1];
		virtual_memory.large_blocks[i].next =
		        &virtual_memory.large_blocks[i + 1];
	}

	/* Set th next of each block to the next bigger first block */
	virtual_memory.small_blocks[MAX_BLOCKS - 1].next =
	        virtual_memory.medium_blocks;
	virtual_memory.medium_blocks[MAX_BLOCKS - 1].next =
	        virtual_memory.large_blocks;
	virtual_memory.large_blocks[MAX_BLOCKS - 1].next = NULL;


	block_init(virtual_memory.small_blocks);
	virtual_memory.initialized = 1;
}


struct block *
block_find(size_t size)
{
	if (size <= SMALL_BLOCK_SIZE)
		return virtual_memory.small_blocks;
	if (size <= MEDIUM_BLOCK_SIZE)
		return virtual_memory.medium_blocks;
	return virtual_memory.large_blocks;
}


void *
_malloc(size_t size)
{
	if (!virtual_memory.initialized)
		memory_init();

	size = (size < MIN_ALLOC_SIZE) ? MIN_ALLOC_SIZE : size;

	if (size > LARGE_BLOCK_SIZE) {
		fprintf(stderr, "malloc: size too big\n");
		return NULL;
	}

	size_t contiguous_memory = 0;
	int region_count = 0;
	struct region *current;
	struct region *iterator;
	struct block *block = block_find(size);

	while (block) {
		if (!block->initialized)
			block_init(block);

		if (block->available_size >= size) {
			contiguous_memory = 0;
			region_count = 0;
			current = block->regions;
			iterator = block->regions;

			/* Searching regions to allocate *
			 * inside while (block) in case the memory isnt contiguous
			 * in that case keep searching on different blocks */
			while (iterator && contiguous_memory < size) {
				if (iterator->free) {
					contiguous_memory += iterator->size;
					region_count++;
				} else {
					contiguous_memory = 0;
					region_count = 0;
					current = iterator->next;
				}
				iterator = iterator->next;
			}
			/* Enough contiguous memory found */
			if (contiguous_memory >= size)
				break;
		}
		block = block->next;
	}

	if (!block) {
		fprintf(stderr, "malloc: not enough available memory\n");
		return NULL;
	}


	/* Mark the regions as used */
	iterator = current;
	for (int i = 0; i < region_count; i++) {
		iterator->free = 0;
		iterator->size = 0;
		iterator = iterator->next;
	}
	current->size = contiguous_memory;
	block->available_size -= contiguous_memory;

	return current->ptr;
}

void *
_calloc(size_t nmemb, size_t size)
{
	size_t total_size = nmemb * size;
	void *ptr = _malloc(total_size);
	if (!ptr)
		return NULL;
	memset(ptr, 0, total_size);
	return ptr;
}

void
_free(void *ptr)
{
	struct block *block = virtual_memory.small_blocks;
	size_t region_size, regions_to_free;


	while (block) {
		if (!block->initialized ||
		    !(block->regions[0].ptr <= ptr &&
		      block->regions[MAX_REGIONS - 1].ptr >=
		              ptr)) {  // optimization: ptr is not in this block
			block = block->next;
			continue;
		}
		for (size_t i = 0; i < MAX_REGIONS; i++) {
			if (block->regions[i].ptr == ptr) {
				if (block->regions[i].free) {
					fprintf(stderr, "free: double free\n");
					return;
				}
				region_size = (block->size / MAX_REGIONS);
				regions_to_free =
				        i +
				        (size_t) ceil(
				                (double) (block->regions[i].size /
				                          region_size));

				for (; i < regions_to_free; i++) {
					block->regions[i].free = 1;
					block->regions[i].size = region_size;
					block->available_size += region_size;
				}
				if (block->size == block->available_size) {
					if (munmap(block->ptr, block->size) < 0) {
						perror("free");
					} else {
						block->initialized = 0;
					}
				}
				return;
			}
		}
		block = block->next;
	}

	fprintf(stderr, "free: invalid pointer\n");
}
