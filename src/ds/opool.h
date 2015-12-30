#ifndef OPOOL_HEADER
#define OPOOL_HEADER

#include "config.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct {
	size_t object_size;
	unsigned bucket_count;
	unsigned count;
	unsigned current_bucket;
	unsigned char buckets[];
} Opool;

typedef struct {
	size_t object_size;
	unsigned count;
	unsigned bucket_count;
	unsigned pool_count;
	unsigned pool_capacity;
	unsigned current_pool;
	unsigned pool_mem_size;
	Opool ** pools;
} Flexpool;

// Opool Functions

Opool * opool_init(void * memory, const size_t mem_size, const size_t object_size);

void * opool_alloc(Opool * pool);

void opool_free(Opool * pool, void * object);

unsigned opool_capacity(Opool * pool);

int opool_checkn(Opool * pool, unsigned n);

void * opool_peekn(Opool * pool, unsigned n);

int opool_getn(Opool * pool, void * object);

void opool_print(Opool * pool);

// Flexpool Functions

Flexpool * flexpool_init(Flexpool * pool, size_t osize, unsigned count_per_block);

void flexpool_deinit(Flexpool * pool);

void * flexpool_alloc(Flexpool * pool);

int flexpool_checkn(Flexpool * pool, unsigned n);

void * flexpool_peekn(Flexpool * pool, unsigned n);

int flexpool_getn(Flexpool * pool, void * object);

void flexpool_free(Flexpool * pool, void * object);

void flexpool_print(Flexpool * pool);

#endif
