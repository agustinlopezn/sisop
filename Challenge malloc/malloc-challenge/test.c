#include <stdbool.h>
#include <stdio.h>

#include "malloc.h"

void
test_malloc_different_sizes()
{
	printf("\n\nTEST DIFFERENT MALLOC SIZES\n\n");
	void *small = _malloc(10);
	void *medium = _malloc(SMALL_BLOCK_SIZE + 1);
	void *large = _malloc(MEDIUM_BLOCK_SIZE + 1);

	printf("First small block is initialized: ---- %s\n",
	       virtual_memory.small_blocks[0].initialized == 1 ? "OK" : "FAIL");
	printf("First medium block is initialized: ---- %s\n",
	       virtual_memory.medium_blocks[0].initialized == 1 ? "OK" : "FAIL");
	printf("First big block is initialized: ---- %s\n",
	       virtual_memory.large_blocks[0].initialized == 1 ? "OK" : "FAIL");

	_free(small);
	_free(medium);
	_free(large);
}

void
test_malloc_and_free()
{
	void *pointers[10];
	size_t offset = 0, bytes_allocated = 0;
	bool success = true;

	for (size_t i = 0; i <= 10; i++) {
		if (i <= 5) {
			offset = 4;
		} else if (i <= 9) {
			offset = 8;
		} else {
			offset = 12;
		}
		pointers[i] = _malloc(i);
		if (i > 0) {
			success &= pointers[i] == (pointers[i - 1] + offset);
		}
		bytes_allocated += i;
	}

	printf("\n\nTEST SIMPLE MALLOCS AND FREES\n\n");
	printf("Multiple little mallocs are contiguous in memory: ---- %s\n",
	       success ? "OK" : "FAIL");
	printf("Freeing...\n");

	for (size_t i = 0; i <= 10; i++) {
		_free(pointers[i]);
	}
	bool are_free = true, size_is_restored = true, block_is_unitialized = true;
	for (int i = 0; i < bytes_allocated / 4; i++) {
		are_free &= virtual_memory.small_blocks[0].regions[i].free == 1;
		size_is_restored &=
		        virtual_memory.small_blocks[0].regions[i].size == 4;
	}
	block_is_unitialized = virtual_memory.small_blocks[0].initialized == 0;

	printf("The used sub-regions are free: ---- %s\n",
	       are_free ? "OK" : "FAIL");
	printf("The used sub-regions are size zero: ---- %s\n",
	       size_is_restored ? "OK" : "FAIL");
	printf("The used block was completely freed: ---- %s\n",
	       block_is_unitialized ? "OK" : "FAIL");
}

void
test_medium_malloc()
{
	char *foo = _malloc(SMALL_BLOCK_SIZE + 1);
	bool small_block_initialized = false;
	bool big_block_initialized = false;
	bool only_first_medium_block_initialized = true;

	for (int i = 0; i < MAX_BLOCKS; i++) {
		if (virtual_memory.small_blocks[i].initialized ||
		    virtual_memory.large_blocks[i].initialized) {
			small_block_initialized = true;
			big_block_initialized = true;
			break;
		}
		if (i == 0) {
			only_first_medium_block_initialized &=
			        virtual_memory.medium_blocks[i].initialized == 1;
		} else {
			only_first_medium_block_initialized &=
			        virtual_memory.medium_blocks[i].initialized == 0;
		}
	}

	printf("\n\nTEST MEDIUM MALLOCS AND FREES\n\n");
	printf("Allocating 16Kib+1 returns firts medium block pointer: ---- "
	       "%s\n",
	       virtual_memory.medium_blocks[0].regions[0].ptr == foo ? "OK"
	                                                             : "FAIL");
	printf("Causes to medium blocks to initialize: ---- %s\n",
	       virtual_memory.medium_blocks->regions[0].ptr ? "OK" : "FAIL");
	printf("Small blocks are not used: ---- %s\n",
	       !small_block_initialized ? "OK" : "FAIL");
	printf("Big blocks are not used: ---- %s\n",
	       !big_block_initialized ? "OK" : "FAIL");
	printf("Only first medium block is used: ---- %s\n",
	       only_first_medium_block_initialized ? "OK" : "FAIL");

	_free(foo);
}

void
test_large_malloc()
{
	char *foo = _malloc(MEDIUM_BLOCK_SIZE + 1);
	bool small_block_initialized = false;
	bool medium_block_initialized = false;
	bool only_first_large_block_initialized = true;

	for (int i = 0; i < MAX_BLOCKS; i++) {
		if (virtual_memory.small_blocks[i].initialized ||
		    virtual_memory.medium_blocks[i].initialized) {
			small_block_initialized = true;
			medium_block_initialized = true;
			break;
		}
		if (i == 0) {
			only_first_large_block_initialized &=
			        virtual_memory.large_blocks[i].initialized == 1;
		} else {
			only_first_large_block_initialized &=
			        virtual_memory.large_blocks[i].initialized == 0;
		}
	}

	printf("\n\nTEST LARGE MALLOCS AND FREES\n\n");
	printf("Allocating 1MiB+1 returns firts large block pointer: ---- "
	       "%s\n",
	       virtual_memory.large_blocks[0].regions[0].ptr == foo ? "OK"
	                                                            : "FAIL");
	printf("Causes to medium blocks to initialize: ---- %s\n",
	       virtual_memory.large_blocks->regions[0].ptr ? "OK" : "FAIL");
	printf("Small blocks are not used: ---- %s\n",
	       !small_block_initialized ? "OK" : "FAIL");
	printf("Big blocks are not used: ---- %s\n",
	       !medium_block_initialized ? "OK" : "FAIL");
	printf("Only first large block is used: ---- %s\n",
	       only_first_large_block_initialized ? "OK" : "FAIL");

	_free(foo);
}

void
test_malloc_errors()
{
	printf("\n\nTEST ERRORS AND BORDER CASES\n\n");
	bool small_blocks_initialized = true;
	bool medium_blocks_initialized = true;
	bool large_blocks_initialized = true;
	bool small_blocks_are_full = true;
	bool medium_blocks_are_full = true;
	bool large_blocks_are_full = true;
	void *small_pointers[MAX_REGIONS];
	void *medium_pointers[MAX_REGIONS];
	void *large_pointers[MAX_REGIONS];

	for (int i = 0; i < MAX_BLOCKS; i++) {
		small_pointers[i] = _malloc(SMALL_BLOCK_SIZE);
		medium_pointers[i] = _malloc(MEDIUM_BLOCK_SIZE);
		large_pointers[i] = _malloc(LARGE_BLOCK_SIZE);
	}

	for (int i = 0; i < MAX_BLOCKS; i++) {
		small_blocks_initialized &=
		        virtual_memory.small_blocks[i].initialized;
		medium_blocks_initialized &=
		        virtual_memory.medium_blocks[i].initialized;
		large_blocks_initialized &=
		        virtual_memory.large_blocks[i].initialized;
		for (int j = 0; j < MAX_REGIONS; j++) {
			small_blocks_are_full &=
			        virtual_memory.small_blocks[i].regions[j].free ==
			        0;
			medium_blocks_are_full &=
			        virtual_memory.medium_blocks[i].regions[j].free ==
			        0;
			large_blocks_are_full &=
			        virtual_memory.large_blocks[i].regions[j].free ==
			        0;
		}
	}

	printf("All small blocks and regions are used: ---- %s\n",
	       small_blocks_initialized && small_blocks_are_full ? "OK" : "FAIL");
	printf("All medium blocks and regions are used: ---- %s\n",
	       medium_blocks_initialized && medium_blocks_are_full ? "OK"
	                                                           : "FAIL");
	printf("All large blocks and regions are used: ---- %s\n",
	       large_blocks_initialized && large_blocks_are_full ? "OK" : "FAIL");
	printf("No more memory to alloc:\n");
	_malloc(1);

	for (int i = 0; i < MAX_BLOCKS; i++) {
		_free(small_pointers[i]);
		_free(medium_pointers[i]);
		_free(large_pointers[i]);
	}

	void *double_free = _malloc(4);
	void *p2 = _malloc(4);
	printf("Double free:\n");
	_free(double_free);
	_free(double_free);
	_free(p2);


	void *invalid_pointer = _malloc(4);
	printf("Invalid pointer:\n");
	_free(invalid_pointer + 1);
	_free(invalid_pointer);

	void *invalid_pointer_border = _malloc(4);
	printf("Invalid pointer border:\n");
	_free(invalid_pointer_border);
	_free(invalid_pointer_border);

	printf("Too large malloc:\n");
	_malloc(LARGE_BLOCK_SIZE + 1);
}

void
test_calloc()
{
	char *p1 = _calloc(sizeof(char), 12);
	bool calloc_initilize_in_zero = true;

	for (int i = 0; i < 12; i++) {
		calloc_initilize_in_zero &= p1[i] == 0;
	}

	printf("\n\nTEST CALLOC\n\n");
	printf("Calloc initialize in zero: ---- %s\n",
	       calloc_initilize_in_zero ? "OK" : "FAIL");

	_free(p1);
}

int
main()
{
	test_malloc_and_free();
	test_calloc();
	test_medium_malloc();
	test_large_malloc();
	test_malloc_different_sizes();
	test_malloc_errors();

	return 0;
}