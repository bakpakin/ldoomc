#include "hash.h"
#include <stdlib.h>

// Main Hash function
uint64_t hash_hash(const char * key) {

    uint64_t hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + 5) + c;

    return hash;
}

HashTable * hash_init(HashTable * ht, unsigned capacity) {

    ht->count = 0;
    ht->capacity = capacity;

    // Use calloc to make sure the buckets are initially empty
    ht->buckets = calloc(capacity, sizeof(HashBucket));

    return ht;
}

void hash_deinit(HashTable * ht) {

    // Free all the buckets
    const HashBucket * b = hash_first(ht);
    while (b) {
        hash_dropbucket(ht, b);
        b = hash_next(ht, b);
    }

    free(ht->buckets);

}

const HashBucket * hash_first(const HashTable * ht) {
    return NULL;
}

const HashBucket * hash_next(const HashTable * ht, const HashBucket * bucket) {

    if (bucket->next)
        return bucket->next;
    else {

    }

    return NULL;
}

void hash_put(HashTable * ht, const char * key, void * value) {

}

void * hash_pop(HashTable * ht, const char * key) {
    return NULL;
}

void * hash_get(HashTable * ht, const char * key) {
    return NULL;
}

HashBucket * hash_bucket(HashTable * ht, const char * key) {
    return NULL;
}

void hash_dropbucket(HashTable * ht, const HashBucket * bucket) {

}
