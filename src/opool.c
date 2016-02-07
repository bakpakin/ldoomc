#include "opool.h"
#include <stdio.h>
#include "util.h"

#define bbits(P, B, S) ((P)->buckets[(B) * (S)])
#define bsize(P) (8 * (P)->object_size + 1)

// A lookup table for for the most significant 0 bit in an 8 bit value.
static const unsigned char block_lookup[256] = {
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 8
};

typedef unsigned char byte;

static inline void * opool_mem_end(Opool * pool) {
	return pool->buckets + pool->bucket_count * bsize(pool);
}

static inline size_t opool_mem_for(size_t osize, unsigned count) {
	return (((count - 1) / 8) + 1) * (4 + 8 * osize);
}

/*
 * Prints a byte representing the populated cells of a pool.
 */
#define B_ON '*'
#define B_OFF '.'
#define B_T "%c"
static inline void print_pop_byte(byte b) {
	printf(B_T""B_T""B_T""B_T""B_T""B_T""B_T""B_T,
		b & 0x01 ? B_ON : B_OFF,
		b & 0x02 ? B_ON : B_OFF,
		b & 0x04 ? B_ON : B_OFF,
		b & 0x08 ? B_ON : B_OFF,
		b & 0x10 ? B_ON : B_OFF,
		b & 0x20 ? B_ON : B_OFF,
		b & 0x40 ? B_ON : B_OFF,
		b & 0x80 ? B_ON : B_OFF);
}
#undef B_T
#undef B_ON
#undef B_OFF

// Opool functions

Opool * opool_init(void * memory, size_t mem_size, size_t object_size) {
	size_t bucket_size = 1 + 8 * object_size;
	size_t bucket_mem = mem_size - sizeof(Opool);
	unsigned bucket_count = bucket_mem / bucket_size;

	//Initialize Head
	Opool * pool = (Opool *)memory;
	pool->object_size = object_size;
	pool->bucket_count = bucket_count;
	pool->current_bucket = 0;

	//Initialize buckets
    memset(&pool->buckets, 0, bucket_mem);

	return pool;
}

void * opool_alloc(Opool * pool) {
	size_t bucket_size = bsize(pool);
	unsigned bucket_count = pool->bucket_count;
	unsigned current_bucket = pool->current_bucket;
    byte * bits = pool->buckets + current_bucket * bucket_size;
	while (current_bucket < bucket_count && *bits == 255) {
		bits += bucket_size;
		current_bucket++;
	}

	if (current_bucket >= bucket_count) { // Run out of blocks
		return NULL;
	}

	pool->current_bucket = current_bucket;

	byte offset = block_lookup[*bits];
	*bits |= 1 << offset;

	pool->count++;

	return bits + 1 + offset * pool->object_size;
}

void opool_free(Opool * pool, void * object) {
	size_t bucket_size = bsize(pool);
	unsigned bucket_count = pool->bucket_count;

	ptrdiff_t diff = (byte *)object - pool->buckets;
	unsigned bucket_index = ((unsigned) diff) / bucket_size;
	byte offset = 1 << ((diff - (bucket_index * bucket_size)) / pool->object_size);

    if (bucket_index > 8 * bucket_count || !(offset & pool->buckets[bucket_size * bucket_index])) {
        uerr("Trying to free invalid pointer.");
    }

	pool->buckets[bucket_size * bucket_index] ^= offset;

	if (pool->current_bucket > bucket_index)
		pool->current_bucket = bucket_index;

	pool->count--;
}

unsigned opool_capacity(Opool * pool) {
	return 8 * pool->bucket_count;
}

int opool_checkn(Opool * pool, unsigned n) {
	if (n >= 8 * pool->bucket_count) {
		return 0;
	}
	unsigned bindex = n >> 3;
	byte offset = n - (bindex << 3);
	return pool->buckets[bsize(pool) * bindex] & (1 << offset) ? 1 : 0;
}

void * opool_peekn(Opool * pool, unsigned n) {
	if (n >= 8 * pool->bucket_count) {
		return NULL;
	}
	unsigned bindex = n >> 3;
	unsigned offset = n - (bindex << 3);
	return pool->buckets + (bsize(pool) * bindex) + pool->object_size * offset;
}

int opool_getn(Opool * pool, void * object) {
	size_t bucket_size = bsize(pool);
	unsigned bucket_count = pool->bucket_count;

	ptrdiff_t diff = (byte *)object - pool->buckets;
	unsigned bucket_index = ((unsigned) diff) / bucket_size;
	unsigned offset = (diff - (bucket_index * bucket_size)) / pool->object_size;
    int n = 8 * bucket_index + offset;
    if (n >= 8 * bucket_count) {
        return -1;
    }
    return n;
}

static void opool_print_trail(Opool * pool, char * trail) {
	size_t bucket_size = bsize(pool);
	unsigned bucket_count = pool->bucket_count;
	for (int i = 0; i < bucket_count * bucket_size; i += bucket_size) {
		byte b = pool->buckets[i];
		print_pop_byte(b);
	}
	printf("%s", trail);
}

void opool_print(Opool * pool) {
    opool_print_trail(pool, "\n");
}

// Flexpool functions

Flexpool * flexpool_init(Flexpool * pool, size_t osize, unsigned count_per_block) {
    pool->object_size = osize;
    pool->count = 0;
    pool->bucket_count = count_per_block;
    pool->pool_count = 0;
    pool->pool_capacity = 10;
    pool->current_pool = 0;
    pool->pools = calloc(10, sizeof(Opool *));
    pool->pool_mem_size = opool_mem_for(osize, count_per_block);
	return pool;
}

void flexpool_deinit(Flexpool * pool) {
    for (int i = 0; i < pool->pool_count; i++)
        free(pool->pools[i]);
	free(pool->pools);
}

static void flexpool_double_pools(Flexpool * pool) {
    pool->pool_capacity *= 2;
    pool->pools = realloc(pool->pools, pool->pool_capacity * sizeof(Opool *));
}

void * flexpool_alloc(Flexpool * pool) {
    Opool * o = pool->pools[pool->current_pool];
    void * ret = opool_alloc(o);
    while (!ret) {
        pool->current_pool++;
        if (pool->current_pool >= pool->pool_capacity) {
            flexpool_double_pools(pool);
        }
        if (pool->current_pool >= pool->pool_count) {
            o = malloc(pool->pool_mem_size);
            opool_init(o, pool->pool_mem_size, pool->object_size);
            pool->pools[pool->current_pool] = o;
        }
        o = pool->pools[pool->current_pool];
        ret = opool_alloc(o);
    }
    pool->count++;
    return ret;
}

void flexpool_free(Flexpool * pool, void * object) {
    int found = 0;
    for (int i = 0; i < pool->pool_count; i++) {
        Opool * o = pool->pools[i];
        if (object >= (void *) o->buckets && object < opool_mem_end(o)) {
            opool_free(o, object);
            found = 1;
            break;
        }
    }
    if (found)
        pool->count--;
}

void * flexpool_peekn(Flexpool * pool, unsigned n) {
    unsigned operpool = pool->bucket_count * 8;
    unsigned poolindex = n / operpool;
    unsigned on = n - operpool * poolindex;
    return opool_peekn(pool->pools[poolindex], on);
}

int flexpool_checkn(Flexpool * pool, unsigned n) {
    unsigned operpool = pool->bucket_count * 8;
    unsigned poolindex = n / operpool;
    unsigned on = n - operpool * poolindex;
    return opool_checkn(pool->pools[poolindex], on);
}

int flexpool_getn(Flexpool * pool, void * object) {
    int n = -1;
    unsigned operpool = pool->bucket_count * 8;
    for (int i = 0; i < pool->pool_count; i++) {
        Opool * o = pool->pools[i];
        if (object >= (void *) o->buckets && object < opool_mem_end(o)) {
            n = opool_getn(o, object) + i * operpool;
            break;
        }
    }
    return n;
}

void flexpool_print(Flexpool * pool) {
    printf("|");
    for (int i = 0; i < pool->pool_count; i++) {
        opool_print_trail(pool->pools[i], "|");
    }
}

#undef bbits
#undef bsize
