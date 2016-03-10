#ifndef UTIL_HEADER__
#define UTIL_HEADER__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ldmath.h"

#define uerr(msg) do { \
    fprintf(stderr, "Error in %s, line %d, %s: %s\n", __FILE__, __LINE__, __func__, msg); \
    exit(1); \
} while (0)

#define uassert(expr) do { \
    if (!(expr)) \
        uerr("Assertion failed."); \
} while (0)

const char * util_filename_ext(const char * path);

// Allocates some temprary data. Only one chunk of data at a time can be allocated like this.
void * uqmalloc(size_t size);

// realloc the memory from uqmalloc.
void * uqrealloc(size_t size);

// Frees data allocated with uqmalloc.
void uqfree();

// Slurped data musted be freed via free.
char * util_slurp(const char * path, long * length);

void util_spit(const char * path, const char * data, long length);

// Debug printing
void mat4_print(mat4 m);

#endif
