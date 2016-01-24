#ifndef HASH_HEADER
#define HASH_HEADER

#include <stdint.h>

// This is a simple hash table implementation. It is not especially performant,
// but is easy to use and robust. Supports addition, deletion, and stateless iteration.

typedef struct HashBucket {

    unsigned key_size;
    char * key;
    void * value;

    struct HashBucket * next;

} HashBucket;

typedef struct {

    unsigned capacity;
    unsigned count;

    HashBucket * buckets;

} HashTable;

// Main Hash function
uint64_t hash_hash(const char * key);

HashTable * hash_init(HashTable * ht, unsigned capacity);

void hash_deinit(HashTable * ht);

const HashBucket * hash_first(const HashTable * ht);

const HashBucket * hash_next(const HashTable * ht, const HashBucket * bucket);

void hash_put(HashTable * ht, const char * key, void * value);

void * hash_pop(HashTable * ht, const char * key);

void * hash_get(HashTable * ht, const char * key);

HashBucket * hash_bucket(HashTable * ht, const char * key);

void hash_dropbucket(HashTable * ht, const HashBucket * bucket);

#endif
